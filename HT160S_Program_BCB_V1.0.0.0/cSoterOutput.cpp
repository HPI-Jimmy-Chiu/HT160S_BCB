//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "cSoterOutput.h"
#include "database.h"        // HSys.LogRootDir / HSys.CurrentDir
#include "GeneralSetting.h"  // GeneralSetting.sSerialNo
#include "cCsvDailyLog.h"    // cCsvDailyLog::CsvQuote (escape helper)
#include "uFtpUploadThread.h"// FtpUploadThd : Lot End FTP hand-off (enqueue only)
#include "cEventLog.h"       // g_EventLog : record an upload-skipped audit line
#include <FileCtrl.hpp>      // ForceDirectories
#include <IniFiles.hpp>      // TIniFile (read [Soter] PickupDir)
//---------------------------------------------------------------------------
#pragma package(smart_init)

cSoterOutput g_SoterOutput;

//---------------------------------------------------------------------------
//AI(ht160s-soter) 20260721 : per-lot output bucket. One bucket per distinct owning
// lot in the current Lot End flush : its completed CSV data rows plus the filename
// meta (Kyec lot / Product / Substage) captured from that lot's rows. Heap-allocated,
// held in m_pLotBuckets->Objects[], freed wholesale by FreeAllBuckets().
struct TSoterLotBucket
{
    AnsiString   sCustLot;
    AnsiString   sKyecLot;
    AnsiString   sProduct;
    AnsiString   sSubstage;
    TStringList* pLines;
    TSoterLotBucket() { pLines = new TStringList(); }
    ~TSoterLotBucket() { delete pLines; }
};

//---------------------------------------------------------------------------
void TSoterRow::Clear()
{
    bActive     = false;
    sStartTime  = "";
    sFinishTime = "";
    sProductCode= "";
    sSubstage   = "";
    sCustLotID  = "";
    sKyecLotID  = "";
    sLoadTray   = "";
    sUnloadTray = "";
    s2DID       = "";
    sRetestCode = "";
    iHbin       = 0;
    iSbin       = 0;
    sDiePass    = "";
}

//---------------------------------------------------------------------------
cSoterOutput::cSoterOutput()
    : m_pCS(NULL)
    , m_bActive(false)
    , m_sArmCustLot("")
    , m_bVirtual2DSeen(false)
    , m_pLotBuckets(NULL)
    , m_sBaseDir("")
    , m_sPickupDir("")
{
    for (int i = 0; i < SOTER_NOZZLE_COUNT; ++i)
        m_pending[i].Clear();
}

//---------------------------------------------------------------------------
cSoterOutput::~cSoterOutput()
{
    FreeAllBuckets();
    delete m_pLotBuckets;
    m_pLotBuckets = NULL;
    delete m_pCS;
    m_pCS = NULL;
}

//---------------------------------------------------------------------------
void cSoterOutput::Init()
{
    if (!m_pCS)
        m_pCS = new TCriticalSection();
    if (!m_pLotBuckets)
    {
        m_pLotBuckets = new TStringList();   // Objects[] hold TSoterLotBucket*; keyed by CustLot
        // Registry lot ids are case-SENSITIVE (AddLot / FindLotIndex compare byte-exact),
        // so the bucket key must be too : TStringList::IndexOf defaults to case-INsensitive,
        // which would collapse two case-only-distinct lots into one file. Not Sorted : files
        // are emitted in insertion (commit) order.
        m_pLotBuckets->CaseSensitive = true;
    }

    // Central log root constant (HSys.LogRootDir = "D:\\HT160S_Log")
    m_sBaseDir = HSys.LogRootDir + "\\SoterOutput";
    ForceDirectories(m_sBaseDir);

    // Customer pickup folder (KYEC fetch zone). Configurable via
    // system\General.ini [Soter] PickupDir; a blank/absent key falls back to a
    // sibling of the archive. Read once here (a per-machine commissioning path,
    // so a change takes effect on the next restart). Kept as a direct read so
    // this leaf feature does not alter the shared GeneralSetting class layout.
    m_sPickupDir = "";
    {
        AnsiString sRoot = HSys.CurrentDir;
        if (sRoot == "")
            sRoot = "..";
        AnsiString sIni = sRoot + "\\system\\General.ini";
        if (FileExists(sIni))
        {
            TIniFile* pIni = new TIniFile(sIni);
            try
            {
                m_sPickupDir = pIni->ReadString("Soter", "PickupDir", "").Trim();
            }
            __finally
            {
                delete pIni;
            }
        }
    }
    if (m_sPickupDir == "")
        m_sPickupDir = HSys.LogRootDir + "\\SoterPickup";
    ForceDirectories(m_sPickupDir);
}

//---------------------------------------------------------------------------
void cSoterOutput::FreeAllBuckets()
{
    if (m_pLotBuckets == NULL)
        return;
    for (int i = 0; i < m_pLotBuckets->Count; ++i)
    {
        TSoterLotBucket* b = (TSoterLotBucket*)m_pLotBuckets->Objects[i];
        delete b;
    }
    m_pLotBuckets->Clear();
}

//---------------------------------------------------------------------------
// Delete every FILE in the customer pickup folder (sub-folders are left alone).
// Called at Lot Start / production START so the hand-off folder holds only the
// lot about to run. The permanent archive under SoterOutput is untouched, so
// clearing here never loses the machine's own record.
void cSoterOutput::ClearPickupDir()
{
    if (m_sPickupDir == "")
        return;
    ForceDirectories(m_sPickupDir);

    TSearchRec sr;
    AnsiString sMask = m_sPickupDir + "\\*.*";
    if (FindFirst(sMask, faAnyFile, sr) == 0)
    {
        try
        {
            do
            {
                if ((sr.Attr & faDirectory) == 0)
                    DeleteFile(m_sPickupDir + "\\" + sr.Name);
            }
            while (FindNext(sr) == 0);
        }
        __finally
        {
            FindClose(sr);
        }
    }
}

//---------------------------------------------------------------------------
// Quote a field only when it contains a comma / quote / newline, so ordinary
// values stay bare (matching the customer sample) but a stray comma cannot
// break the column layout.
AnsiString cSoterOutput::CsvField(const AnsiString& s)
{
    if (s.Pos(",") > 0 || s.Pos("\"") > 0 || s.Pos("\n") > 0 || s.Pos("\r") > 0)
        return cCsvDailyLog::CsvQuote(s);
    return s;
}

//---------------------------------------------------------------------------
// "" -> "NA". Used for the Kyec lot (col7 + filename token) when SET_LOT_INFO
// did not supply a KYEC batch id. Per customer : missing data = "NA", and a lot
// cannot run without lot info, so NA is a should-not-happen marker.
AnsiString cSoterOutput::NaIfBlank(const AnsiString& s)
{
    return (s.Trim() == "") ? AnsiString("NA") : s;
}

//---------------------------------------------------------------------------
// Replace characters that are illegal in a Windows file name with '-' so the
// custom Soter file name (built from Product / Lot / Substage tokens) is always
// creatable by SaveToFile.
AnsiString cSoterOutput::SafeToken(const AnsiString& s)
{
    AnsiString r = s;
    const char* bad = "\\/:*?\"<>|";
    for (int i = 0; bad[i] != '\0'; ++i)
        r = StringReplace(r, AnsiString(bad[i]), "-", TReplaceFlags() << rfReplaceAll);
    // Windows rejects a name component that ends in a space or dot; strip both ends so a
    // token like "MT3781Q." cannot make the whole SaveToFile path uncreatable.
    r = r.Trim();
    while (r.Length() > 0 && (r[r.Length()] == '.' || r[r.Length()] == ' '))
        r = r.SubString(1, r.Length() - 1);
    while (r.Length() > 0 && (r[1] == '.' || r[1] == ' '))
        r = r.SubString(2, r.Length() - 1);
    return r;
}

//---------------------------------------------------------------------------
AnsiString cSoterOutput::GetTitleLine()
{
    return "No.,StartTime,FinishTime,ProductCode,Substage,Cust lot,Kyec Lot,"
           "Load Cover Tray ID,Unload Cover Tray ID,SorterID,2D ID,"
           "RetestCode,Hbin,Sbin,DiePass";
}

//---------------------------------------------------------------------------
AnsiString cSoterOutput::BuildDataLine(const TSoterRow& r, int iNo)
{
    AnsiString sLine;
    sLine  = IntToStr(iNo);                          // 1  No.
    sLine += "," + CsvField(r.sStartTime);           // 2  StartTime
    sLine += "," + CsvField(r.sFinishTime);          // 3  FinishTime
    sLine += "," + CsvField(r.sProductCode);         // 4  ProductCode
    sLine += "," + CsvField(r.sSubstage);            // 5  Substage
    sLine += "," + CsvField(r.sCustLotID);           // 6  Cust lot (owning lot / 2D-map LOTID)
    sLine += "," + CsvField(NaIfBlank(r.sKyecLotID));// 7  Kyec Lot (SET_LOT_INFO; NA if blank)
    sLine += "," + CsvField(r.sLoadTray);            // 8  Load Cover Tray ID
    sLine += "," + CsvField(r.sUnloadTray);          // 9  Unload Cover Tray ID
    sLine += "," + CsvField(GeneralSetting.sSerialNo);// 10 SorterID
    sLine += "," + CsvField(r.s2DID);                // 11 2D ID
    sLine += "," + CsvField(r.sRetestCode);          // 12 RetestCode
    sLine += "," + IntToStr(r.iHbin);                // 13 Hbin
    sLine += "," + IntToStr(r.iSbin);                // 14 Sbin
    sLine += "," + CsvField(r.sDiePass);             // 15 DiePass
    return sLine;
}

//---------------------------------------------------------------------------
// Build one file name from a lot's meta and the shared batch stamp. Product /
// Substage come from the lot's rows (not a global first-row latch), so a lot with
// dies always gets its own tokens; a zero-die fallback passes blank Product/Substage.
AnsiString cSoterOutput::BuildFileName(const AnsiString& sProduct, const AnsiString& sCustLot,
        const AnsiString& sKyecLot, const AnsiString& sSubstage, int iQty, const AnsiString& sStamp)
{
    AnsiString sName;
    sName  = sStamp;                                 // "yyyymmdd_hhnnss" (shared batch stamp)
    sName += "_KYEC-LFT";
    sName += "_" + SafeToken(sProduct);
    sName += "_" + SafeToken(sCustLot);              // CustomerLotNo
    sName += "_" + SafeToken(NaIfBlank(sKyecLot));   // KYECLotNo (NA if blank)
    sName += "_BI";
    sName += "_" + SafeToken(sSubstage);
    sName += "_" + SafeToken(GeneralSetting.sSerialNo);
    sName += "_" + IntToStr(iQty);
    sName += ".csv";
    return sName;
}

//---------------------------------------------------------------------------
// Guarantee a file name is unique within ONE Lot End flush. SafeToken is not
// injective (e.g. "L/1" and "L-1" both map to "L-1"; a trailing '.' is stripped),
// so two distinct owning-lot buckets could build the same name and the second
// SaveToFile would overwrite the first in BOTH archive and pickup, losing a lot's
// CSV. On collision, insert "_<n>" before the ".csv" so no data is ever lost.
// pSeen is case-INsensitive (Windows file names are), matching the filesystem.
static AnsiString UniqueFileNameInFlush(TStringList* pSeen, const AnsiString& sName)
{
    AnsiString sBase = sName;
    AnsiString sExt  = "";
    int iDot = sName.LastDelimiter(".");
    if (iDot > 0)
    {
        sBase = sName.SubString(1, iDot - 1);
        sExt  = sName.SubString(iDot, sName.Length() - iDot + 1);
    }
    AnsiString sCand = sName;
    int n = 1;
    while (pSeen->IndexOf(sCand) >= 0)
    {
        n++;
        sCand = sBase + "_" + IntToStr(n) + sExt;
    }
    pSeen->Add(sCand);
    return sCand;
}

//---------------------------------------------------------------------------
// Write header (+ optional data rows) to the month-bucketed archive and - when
// bAlsoPickup - the flat customer pickup folder. pDataLines == NULL -> header-only
// (zero-die file). bAlsoPickup=false is the virtual-2D window: the pickup folder is
// a CUSTOMER channel (their fetch script pulls it after CEID 12), so a file whose 2D
// codes were fabricated must not land there any more than it may be FTP-published.
void cSoterOutput::WriteOneFile(const AnsiString& sArchDir, const AnsiString& sFileName,
        TStringList* pDataLines, bool bAlsoPickup)
{
    TStringList* pOut = new TStringList();
    try
    {
        pOut->Add(GetTitleLine());
        if (pDataLines != NULL)
            pOut->AddStrings(pDataLines);

        // 1. permanent archive, month-bucketed (machine's own record)
        ForceDirectories(sArchDir);
        pOut->SaveToFile(sArchDir + "\\" + sFileName);

        // 2. customer pickup folder, flat (wiped at the next Lot Start)
        if (bAlsoPickup && m_sPickupDir != "")
        {
            ForceDirectories(m_sPickupDir);
            pOut->SaveToFile(m_sPickupDir + "\\" + sFileName);
        }
    }
    __finally
    {
        delete pOut;
    }
}

//---------------------------------------------------------------------------
void cSoterOutput::DoArm(const AnsiString& sCustLot)
{
    m_bActive     = true;
    m_sArmCustLot = sCustLot;
    //AI(ht160s-virtual2d) 20260808 : fresh flush window -> clear the virtual-2D taint. If the
    //reader is still fabricating, the very next scanned die re-latches it (per-die call), so a
    //window can only stay clean if every code in it really came off a die.
    m_bVirtual2DSeen = false;
    FreeAllBuckets();
    for (int i = 0; i < SOTER_NOZZLE_COUNT; ++i)
        m_pending[i].Clear();

    // Fresh lot : wipe the customer hand-off folder so it only ever holds the
    // flush now starting. (The permanent archive keeps every past lot.)
    ClearPickupDir();
}

//---------------------------------------------------------------------------
void cSoterOutput::OnLotStart(const AnsiString& sLotID)
{
    if (!m_pCS)
        return;
    m_pCS->Acquire();
    try
    {
        DoArm(sLotID);
    }
    __finally
    {
        m_pCS->Release();
    }
}

//---------------------------------------------------------------------------
void cSoterOutput::EnsureActive(const AnsiString& sLotID)
{
    if (!m_pCS)
        return;
    m_pCS->Acquire();
    try
    {
        if (!m_bActive)
            DoArm(sLotID);
    }
    __finally
    {
        m_pCS->Release();
    }
}

//---------------------------------------------------------------------------
void cSoterOutput::OnLotEnd()
{
    if (!m_pCS)
        return;
    m_pCS->Acquire();
    try
    {
        if (!m_bActive)
            return;   // already flushed by an earlier terminal path

        // Wrap the writes so a disk-full / bad-path failure cannot unwind into the
        // motion or SECS caller (OnLotEnd runs on the machine-control thread via the
        // CleanOut-finish path). On failure the lot CSV is lost but the machine is not
        // destabilised; do NOT pop a modal here (wrong thread). Buffer disarms below
        // regardless, so a failed write is not retried into the next lot's buffer.
        try
        {
            // One batch stamp for the whole flush : every per-lot file of this Lot End
            // shares {Date}_{Time}, and the archive month is taken from the same stamp
            // (a cross-midnight flush is not split across two month folders).
            TDateTime  dtBatch = Now();
            AnsiString sStamp  = FormatDateTime("yyyymmdd", dtBatch) + "_"
                               + FormatDateTime("hhnnss", dtBatch);
            AnsiString sArch   = m_sBaseDir + "\\" + FormatDateTime("yyyymm", dtBatch);

            // pSeen tracks file names emitted THIS flush so a SafeToken collision cannot
            // silently overwrite one lot's file (see UniqueFileNameInFlush). pPub groups the
            // written CSVs by KYEC lot for the FTP hand-off : each KYEC lot gets ONE /LotEnd/
            // flag listing all its CSVs, and ONE publish job that uploads every CSV BEFORE the
            // flag (the commit signal); name=KyecLot (case-sensitive), Objects=TStringList* of
            // that lot's written CSV leaf names. Both are declared NULL and allocated INSIDE
            // the guarded try so a throw during allocation is covered by the __finally.
            TStringList* pSeen = NULL;
            TStringList* pPub  = NULL;
            bool bUploadOn = (FtpUploadThd != NULL &&
                              FtpUploadThd->GetEnable() && FtpUploadThd->GetUploadReport());
            bool bNoKyecLogged = false;
            //AI(ht160s-virtual2d) 20260808 : a window that carried ANY fabricated 2D code is
            //archive-only. bVirtual is snapshotted here so the whole flush is judged once.
            bool bVirtual = m_bVirtual2DSeen;
            if (bVirtual)
                g_EventLog.Log("FTP_SKIP",
                    "Soter CSVs written to ARCHIVE ONLY (virtual 2D was used this run): "
                    "no FTP publish, no pickup copy", "");
            try
            {
                pSeen = new TStringList();
                pPub  = new TStringList();
                pPub->CaseSensitive = true;

                int nBuckets = (m_pLotBuckets != NULL) ? m_pLotBuckets->Count : 0;
                if (nBuckets == 0)
                {
                    // Zero genuine-2D dies : still emit ONE header-only file (Qty=0) for the
                    // armed lot so the customer can tell "ran but 0 units" from a missed
                    // hand-off. No rows -> Product/Substage/customer lot unknown.
                    //AI(ht160s-kyec) 20260722 : m_sArmCustLot is the armed KYEC lot -> col7
                    //(KYEC token). The customer-lot token (col6) is NA : no die resolved one.
                    AnsiString sFileName = BuildFileName("", "NA", m_sArmCustLot, "", 0, sStamp);
                    sFileName = UniqueFileNameInFlush(pSeen, sFileName);
                    WriteOneFile(sArch, sFileName, NULL, !bVirtual);
                    //AI(ht160s-kyec) 20260722 : a 0-die lot is archive/pickup only, no FTP publish
                    //(customer ruling). The KYEC lot IS known; there is simply nothing to publish.
                    if (bUploadOn && !bNoKyecLogged)
                    {
                        g_EventLog.Log("FTP_SKIP", "Soter 0-die lot (KYEC "+m_sArmCustLot+"): header-only file, no FTP publish", "");
                        bNoKyecLogged = true;
                    }
                }
                else
                {
                    for (int i = 0; i < nBuckets; ++i)
                    {
                        TSoterLotBucket* b = (TSoterLotBucket*)m_pLotBuckets->Objects[i];
                        if (b == NULL)
                            continue;
                        AnsiString sFileName = BuildFileName(b->sProduct, b->sCustLot, b->sKyecLot,
                                                             b->sSubstage, b->pLines->Count, sStamp);
                        sFileName = UniqueFileNameInFlush(pSeen, sFileName);
                        WriteOneFile(sArch, sFileName, b->pLines, !bVirtual);

                        //AI(ht160s-virtual2d) 20260808 : a tainted bucket never enters the
                        //publish grouping - the FTP_SKIP line above already said why, once.
                        if (bVirtual)
                            continue;
                        if (b->sKyecLot.Trim() != "")
                        {
                            int gi = pPub->IndexOf(b->sKyecLot);
                            TStringList* g;
                            if (gi < 0)
                            {
                                g = new TStringList();
                                pPub->AddObject(b->sKyecLot, (TObject*)g);
                            }
                            else
                            {
                                g = (TStringList*)pPub->Objects[gi];
                            }
                            g->Add(sFileName);   // CSV leaf name (already unique within the flush)
                        }
                        else if (bUploadOn && !bNoKyecLogged)
                        {
                            // A lot with dies but no KYEC batch id : cannot build /<KYLotNo>/,
                            // so skip its upload. The archive + pickup copy is still written.
                            g_EventLog.Log("FTP_SKIP",
                                AnsiString("Soter lot has no KYEC batch id, upload skipped (lot=")
                                + b->sCustLot + ")", "");
                            bNoKyecLogged = true;
                        }
                    }
                }

                // Publish phase : one flag + one publish job per KYEC lot (upload gated ON).
                if (bUploadOn)
                {
                    AnsiString sDateCode = FormatDateTime("yyyymmddhhnnss", dtBatch);
                    for (int gi = 0; gi < pPub->Count; ++gi)
                    {
                        AnsiString   sKyec = pPub->Strings[gi];
                        TStringList* g     = (TStringList*)pPub->Objects[gi];
                        if (g == NULL || g->Count == 0)
                            continue;

                        // Sanitize the KYEC lot ONCE and use it for BOTH the flag name and the
                        // remote folder (passed to EnqueueLotPublish) so the /LotEnd/ flag token
                        // always matches the /<KYLotNo>/ folder the worker creates. For real
                        // alphanumeric KYEC ids SafeToken is a no-op; this only diverges for an
                        // exotic id (e.g. a '/'), where the sanitized, creatable form is correct.
                        AnsiString sKyecSafe = SafeToken(sKyec);
                        // flag : <KYLotNo>_<DateCode>.txt ; content = one CSV leaf name per line
                        AnsiString sFlagName = UniqueFileNameInFlush(pSeen,
                            sKyecSafe + "_" + sDateCode + ".txt");
                        AnsiString sFlagPath = sArch + "\\" + sFlagName;

                        AnsiString   sCsvJoined = "";
                        TStringList* pFlag = new TStringList();
                        try
                        {
                            for (int k = 0; k < g->Count; ++k)
                            {
                                pFlag->Add(g->Strings[k]);
                                if (sCsvJoined != "")
                                    sCsvJoined += "\n";
                                sCsvJoined += sArch + "\\" + g->Strings[k];
                            }
                            pFlag->SaveToFile(sFlagPath);   // local flag copy (audit + upload source)
                        }
                        __finally
                        {
                            delete pFlag;
                        }

                        FtpUploadThd->EnqueueLotPublish(sKyecSafe, sCsvJoined, sFlagPath);
                    }
                }
            }
            __finally
            {
                if (pPub != NULL)
                {
                    for (int i = 0; i < pPub->Count; ++i)
                        delete (TStringList*)pPub->Objects[i];
                    delete pPub;
                }
                delete pSeen;
            }
        }
        catch (Exception&)
        {
            // Soter CSV write failed (disk full / path). Output for this lot is lost;
            // swallow so the caller is not disturbed.
        }
        m_bActive = false;   // disarm; buckets cleared on next OnLotStart (DoArm)
    }
    __finally
    {
        m_pCS->Release();
    }
}

//---------------------------------------------------------------------------
void cSoterOutput::OpenRow(int iNozzle,
                           const AnsiString& sCustLot,
                           const AnsiString& sKyecLot,
                           const AnsiString& sProductCode,
                           const AnsiString& sSubstage,
                           const AnsiString& sCode2D,
                           const AnsiString& sLoadTray,
                           const AnsiString& sRetestCode,
                           int iHbin, int iSbin,
                           const AnsiString& sDiePass)
{
    if (!m_pCS)
        return;
    if (iNozzle < 0 || iNozzle >= SOTER_NOZZLE_COUNT)
        return;
    m_pCS->Acquire();
    try
    {
        if (!m_bActive)
            return;
        if (sCode2D.Trim() == "")
            return;   // backstop: only dies with a genuine 2D identity

        TSoterRow& r = m_pending[iNozzle];
        r.Clear();
        r.bActive     = true;
        r.sStartTime  = FormatDateTime("yyyy-mm-dd hh:nn:ss", Now());
        r.sCustLotID  = sCustLot;
        r.sKyecLotID  = sKyecLot;
        r.sProductCode= sProductCode;
        r.sSubstage   = sSubstage;
        r.sLoadTray   = sLoadTray;
        r.s2DID       = sCode2D;
        r.sRetestCode = sRetestCode;
        r.iHbin       = iHbin;
        r.iSbin       = iSbin;
        r.sDiePass    = sDiePass;
    }
    __finally
    {
        m_pCS->Release();
    }
}

//---------------------------------------------------------------------------
// assumes lock held. Route the completed row into its owning lot's bucket (created
// on first row of that lot), capturing the bucket's filename meta from the row.
void cSoterOutput::CommitRow(int iNozzle, const AnsiString& sUnloadTray)
{
    if (iNozzle < 0 || iNozzle >= SOTER_NOZZLE_COUNT)
        return;
    TSoterRow& r = m_pending[iNozzle];
    if (!r.bActive)
        return;

    r.sFinishTime = FormatDateTime("yyyy-mm-dd hh:nn:ss", Now());
    r.sUnloadTray = sUnloadTray;

    if (m_pLotBuckets != NULL)
    {
        //AI(ht160s-kyec) 20260722 : file-split key = KYEC lot + customer lot (composite) so the
        //SAME customer LOTID under two different KYEC lots in one flush does NOT merge into one
        //CSV/folder. \x01 cannot appear in a lot id. Common 1:1/1:N case is unchanged (the KYEC
        //lot is constant across the flush, so the bucket count matches a custlot-only key).
        AnsiString sKey = r.sKyecLotID + "\x01" + r.sCustLotID;   // composite (KYEC, cust)
        int idx = m_pLotBuckets->IndexOf(sKey);
        TSoterLotBucket* b;
        if (idx < 0)
        {
            b = new TSoterLotBucket();
            b->sCustLot  = r.sCustLotID;
            b->sKyecLot  = r.sKyecLotID;
            b->sProduct  = r.sProductCode;
            b->sSubstage = r.sSubstage;
            m_pLotBuckets->AddObject(sKey, (TObject*)b);
        }
        else
        {
            b = (TSoterLotBucket*)m_pLotBuckets->Objects[idx];
            // First non-empty value wins : fill any meta the bucket still lacks.
            if (b->sKyecLot == "" && r.sKyecLotID != "")   b->sKyecLot  = r.sKyecLotID;
            if (b->sProduct == "" && r.sProductCode != "") b->sProduct  = r.sProductCode;
            if (b->sSubstage == "" && r.sSubstage != "")   b->sSubstage = r.sSubstage;
        }
        int iNo = b->pLines->Count + 1;   // per-file serial, 1-based
        b->pLines->Add(BuildDataLine(r, iNo));
    }
    r.Clear();
}

//---------------------------------------------------------------------------
void cSoterOutput::CommitPlaceRow(int iNozzle, const AnsiString& sUnloadTray)
{
    if (!m_pCS)
        return;
    m_pCS->Acquire();
    try
    {
        if (!m_bActive)
            return;
        CommitRow(iNozzle, sUnloadTray);
    }
    __finally
    {
        m_pCS->Release();
    }
}

//---------------------------------------------------------------------------
void cSoterOutput::CommitRejectRow(int iNozzle)
{
    if (!m_pCS)
        return;
    m_pCS->Acquire();
    try
    {
        if (!m_bActive)
            return;
        CommitRow(iNozzle, "");   // col9 (Unload) blank on reject
    }
    __finally
    {
        m_pCS->Release();
    }
}

//---------------------------------------------------------------------------
void cSoterOutput::DiscardRow(int iNozzle)
{
    if (!m_pCS)
        return;
    if (iNozzle < 0 || iNozzle >= SOTER_NOZZLE_COUNT)
        return;
    m_pCS->Acquire();
    try
    {
        m_pending[iNozzle].Clear();
    }
    __finally
    {
        m_pCS->Release();
    }
}

//---------------------------------------------------------------------------
void cSoterOutput::NoteVirtual2D(const AnsiString& sWhy)
{
    if (!m_pCS)
        return;
    m_pCS->Acquire();
    try
    {
        if (m_bVirtual2DSeen)
            return;   // already latched (and logged) for this flush window
        m_bVirtual2DSeen = true;
        //AI(ht160s-virtual2d) 20260808 : the one line that makes a virtual run findable
        //afterwards. On 2026-08-06 three KYEC lots' published CSVs carried registry codes in
        //alphabetical order and no artefact anywhere recorded that the 2D source was not the
        //CCD - it took a sort-by-StartTime analysis of the CSVs themselves to prove it.
        g_EventLog.Log("VIRTUAL_2D",
            "2D source is VIRTUAL (" + sWhy + ") - codes are cycled from the lot registry, "
            "not read from dies; this run's Soter CSVs stay in the archive only", "");
    }
    __finally
    {
        m_pCS->Release();
    }
}
//---------------------------------------------------------------------------
