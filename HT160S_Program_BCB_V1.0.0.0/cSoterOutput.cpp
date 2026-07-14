//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "cSoterOutput.h"
#include "database.h"        // HSys.LogRootDir
#include "GeneralSetting.h"  // GeneralSetting.sSerialNo
#include "cCsvDailyLog.h"    // cCsvDailyLog::CsvQuote (escape helper)
#include <FileCtrl.hpp>      // ForceDirectories
//---------------------------------------------------------------------------
#pragma package(smart_init)

cSoterOutput g_SoterOutput;

//---------------------------------------------------------------------------
void TSoterRow::Clear()
{
    bActive     = false;
    sStartTime  = "";
    sFinishTime = "";
    sProductCode= "";
    sSubstage   = "";
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
    , m_sLotID("")
    , m_sFileProduct("")
    , m_sFileSubstage("")
    , m_pLines(NULL)
    , m_sBaseDir("")
{
    for (int i = 0; i < SOTER_NOZZLE_COUNT; ++i)
        m_pending[i].Clear();
}

//---------------------------------------------------------------------------
cSoterOutput::~cSoterOutput()
{
    delete m_pLines;
    m_pLines = NULL;
    delete m_pCS;
    m_pCS = NULL;
}

//---------------------------------------------------------------------------
void cSoterOutput::Init()
{
    if (!m_pCS)
        m_pCS = new TCriticalSection();
    if (!m_pLines)
        m_pLines = new TStringList();

    // Central log root constant (HSys.LogRootDir = "D:\\HT160S_Log")
    m_sBaseDir = HSys.LogRootDir + "\\SoterOutput";
    ForceDirectories(m_sBaseDir);
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
    sLine += "," + CsvField(m_sLotID);               // 6  Cust lot
    sLine += "," + CsvField(m_sLotID);               // 7  Kyec Lot (== col6)
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
// Resolved at lot end so Date/Time and Qty reflect the final file.
AnsiString cSoterOutput::BuildFileName()
{
    TDateTime dtNow = Now();
    AnsiString sName;
    sName  = FormatDateTime("yyyymmdd", dtNow);
    sName += "_" + FormatDateTime("hhnnss", dtNow);
    sName += "_KYEC-LFT";
    sName += "_" + SafeToken(m_sFileProduct);
    sName += "_" + SafeToken(m_sLotID);              // CustomerLotNo
    sName += "_" + SafeToken(m_sLotID);              // KYECLotNo (== CustomerLotNo)
    sName += "_BI";
    sName += "_" + SafeToken(m_sFileSubstage);
    sName += "_" + SafeToken(GeneralSetting.sSerialNo);
    sName += "_" + IntToStr(m_pLines ? m_pLines->Count : 0);
    sName += ".csv";
    return sName;
}

//---------------------------------------------------------------------------
void cSoterOutput::DoArm(const AnsiString& sLotID)
{
    m_bActive       = true;
    m_sLotID        = sLotID;
    m_sFileProduct  = "";
    m_sFileSubstage = "";
    if (m_pLines)
        m_pLines->Clear();
    for (int i = 0; i < SOTER_NOZZLE_COUNT; ++i)
        m_pending[i].Clear();
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
        if (m_pLines && m_pLines->Count > 0)
        {
            // Wrap the write so a disk-full / bad-path failure cannot unwind into the
            // motion or SECS caller (OnLotEnd runs on the machine-control thread via the
            // CleanOut-finish path). On failure the lot CSV is lost but the machine is not
            // destabilised; do NOT pop a modal here (wrong thread). Buffer disarms below
            // regardless, so a failed write is not retried into the next lot's buffer.
            try
            {
                AnsiString sDir = m_sBaseDir + "\\" + FormatDateTime("yyyymm", Now());
                ForceDirectories(sDir);

                TStringList* pOut = new TStringList();
                try
                {
                    pOut->Add(GetTitleLine());
                    pOut->AddStrings(m_pLines);
                    pOut->SaveToFile(sDir + "\\" + BuildFileName());
                }
                __finally
                {
                    delete pOut;
                }
            }
            catch (Exception&)
            {
                // Soter CSV write failed (disk full / path). Output for this lot is lost;
                // swallow so the caller is not disturbed.
            }
        }
        m_bActive = false;   // disarm; buffer cleared on next OnLotStart
    }
    __finally
    {
        m_pCS->Release();
    }
}

//---------------------------------------------------------------------------
void cSoterOutput::OpenRow(int iNozzle,
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
// assumes lock held
void cSoterOutput::CommitRow(int iNozzle, const AnsiString& sUnloadTray)
{
    if (iNozzle < 0 || iNozzle >= SOTER_NOZZLE_COUNT)
        return;
    TSoterRow& r = m_pending[iNozzle];
    if (!r.bActive)
        return;

    r.sFinishTime = FormatDateTime("yyyy-mm-dd hh:nn:ss", Now());
    r.sUnloadTray = sUnloadTray;

    // Latch the filename Product / Substage tokens from the first emitted row.
    if (m_sFileProduct == "" && r.sProductCode != "")
        m_sFileProduct = r.sProductCode;
    if (m_sFileSubstage == "" && r.sSubstage != "")
        m_sFileSubstage = r.sSubstage;

    if (m_pLines)
    {
        int iNo = m_pLines->Count + 1;
        m_pLines->Add(BuildDataLine(r, iNo));
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
