//---------------------------------------------------------------------------
// cSoterOutput : KYEC per-die "Soter" output CSV (one row per handled IC).
//
// Emits one row for every IC that carries a genuine 2D identity resolved in
// the 2D map, whether it ends placed OR rejected. Rows are buffered in memory
// (bucketed by owning lot) from Lot Start / production START until the lot ends,
// then written at lot end as ONE custom-named file PER KYEC lot:
//   {Date}_{Time}_KYEC-LFT_{Product}_{CustLot}_{KyecLot}_BI_{Substage}_{Sorter}_{Qty}.csv
// A single Lot End flush thus produces N files (one per distinct owning lot), and
// every file of that flush shares ONE batch timestamp ({Date}_{Time}) so the set is
// recognizable as one Lot End. Each file is written to TWO places :
//   1. archive : D:\HT160S_Log\SoterOutput\<yyyymm>\  - the machine's own permanent,
//      month-bucketed record; the customer never touches it. (<yyyymm> also from the
//      shared batch stamp, so a cross-midnight flush is not split across two folders.)
//   2. pickup  : [Soter] PickupDir in system\General.ini (default
//      D:\HT160S_Log\SoterPickup) - a FLAT, customer-facing hand-off folder. It is
//      CLEARED at every Lot Start / production START and then holds only the current
//      flush's files, so a customer-side fault that deletes/moves them cannot harm the
//      archive. The customer fetches from here after the SECS Lot End event (CEID 12),
//      which the manual Lot End path fires AFTER these files are on disk.
//
// Locked customer decisions (docs/plan/kyec-lot-identity-plan-20260722 ; KYEC dual lot
// identity, 1:N confirmed 2026-07-22):
//   - The machine keys everything on the KYEC lot (SECS/OSATLot). Kyec lot (col7) = the
//     owning KYEC lot (registry sLotID). Cust lot (col6) = the WebAPI response LOTID,
//     stored PER-IC (one KYEC lot can map to several customer lots, 1:N). Distinct values.
//     "" -> "NA".
//   - Substage (col5) + ProductCode (col4) are PER-IC (2D-map JSON group values); no host E87.
//   - SorterID (col10 + filename) = GeneralSetting.sSerialNo.
//   - Load Cover Tray ID (col8)  = incoming identity-tray 2D (Color side).
//   - Unload Cover Tray ID (col9)= flow-lane identity-tray 2D (Auto lane).
//   - Files are written once at lot end (LotEnd button OR host STOP / run-dry /
//     CleanOut-finish); OnLotEnd is idempotent so only the first caller writes.
//   - A lot that produced zero genuine-2D dies still writes ONE header-only file
//     (Qty=0) for the armed lot so the customer can tell "ran but 0 units" from a
//     missed hand-off.
//
// Distinct from g_DeviceInfo (Production_Log). Singleton: g_SoterOutput.
//---------------------------------------------------------------------------
#ifndef cSoterOutputH
#define cSoterOutputH
//---------------------------------------------------------------------------
#include <vcl.h>
#include <SyncObjs.hpp>
//---------------------------------------------------------------------------

#define SOTER_NOZZLE_COUNT 4

//---------------------------------------------------------------------------
// One buffered per-die row. Opened at pick (OpenRow), completed at place or
// reject (CommitPlaceRow / CommitRejectRow), all rows flushed once at lot end.
//---------------------------------------------------------------------------
struct TSoterRow
{
    bool       bActive;      // a pending row is open for this nozzle
    AnsiString sStartTime;   // pick time    "yyyy-mm-dd hh:nn:ss"
    AnsiString sFinishTime;  // place/reject time
    AnsiString sProductCode; // 2D map (per-die GetLot)
    AnsiString sSubstage;    // 2D map (per-die GetLot)
    AnsiString sCustLotID;   // col6 owning lot (2D-map LOTID); also file-split key
    AnsiString sKyecLotID;   // col7 KYEC batch id (SET_LOT_INFO); "" -> "NA"
    AnsiString sLoadTray;    // col8 incoming identity 2D (blank if not genuine)
    AnsiString sUnloadTray;  // col9 flow-lane identity 2D (place only; reject="")
    AnsiString s2DID;        // col11 Slot.Code2D
    AnsiString sRetestCode;  // col12 FindIcInfo
    int        iHbin;        // col13 FindIcInfo
    int        iSbin;        // col14 FindIcInfo
    AnsiString sDiePass;     // col15 FindIcInfo
    void Clear();
};

//---------------------------------------------------------------------------
class cSoterOutput
{
private:
    TCriticalSection* m_pCS;
    bool         m_bActive;       // armed between Lot Start and the LotEnd flush
    AnsiString   m_sArmCustLot;   // armed lot id = KYEC lot (zero-die fallback col7 identity)
    TSoterRow    m_pending[SOTER_NOZZLE_COUNT];
    //AI(ht160s-virtual2d) 20260808 : latched when any 2D code of the CURRENT flush window was
    //FABRICATED (cycled from the lot registry) instead of read from a die. Cleared by DoArm.
    //On 2026-08-06 the on-site "Enable Simulation" checkbox left three KYEC lots' CSVs carrying
    //registry codes in alphabetical order, FTP-published as genuine, and NOTHING recorded that
    //the codes were virtual - the file is bit-identical to a real run's. While latched, the
    //flush still writes the archive (engineering record) but skips BOTH customer channels
    //(FTP publish AND the pickup folder), and the latch event itself is EventLogged.
    bool         m_bVirtual2DSeen;
    TStringList* m_pLotBuckets;   // keyed by CustLot (case-sensitive), insertion-order; Objects=TSoterLotBucket*
    AnsiString   m_sBaseDir;      // HSys.LogRootDir + "\\SoterOutput" (archive)
    AnsiString   m_sPickupDir;    // customer hand-off folder (cleared each Lot Start)

    void DoArm(const AnsiString& sCustLot);   // assumes lock held
    void CommitRow(int iNozzle, const AnsiString& sUnloadTray); // assumes lock
    void FreeAllBuckets();                             // delete every heap bucket + clear list
    AnsiString GetTitleLine();
    AnsiString BuildDataLine(const TSoterRow& r, int iNo);
    AnsiString BuildFileName(const AnsiString& sProduct, const AnsiString& sCustLot,
                             const AnsiString& sKyecLot, const AnsiString& sSubstage,
                             int iQty, const AnsiString& sStamp);
    void WriteOneFile(const AnsiString& sArchDir, const AnsiString& sFileName,
                      TStringList* pDataLines,    // header+rows -> archive (+ pickup)
                      bool bAlsoPickup);          //AI(ht160s-virtual2d) 20260808 : false = archive only (virtual-2D window)
    void ClearPickupDir();                            // wipe the customer pickup folder (Lot Start)
    static AnsiString CsvField(const AnsiString& s);  // quote only if needed
    static AnsiString SafeToken(const AnsiString& s); // sanitize filename token
    static AnsiString NaIfBlank(const AnsiString& s); // "" -> "NA" (col7 / kyec token)

public:
    cSoterOutput();
    ~cSoterOutput();

    void Init();

    // Lot lifecycle. OnLotStart clears + arms a fresh buffer (new lot).
    // EnsureActive arms only if not already armed, so a Pause->resume does NOT
    // wipe an in-progress buffer. OnLotEnd writes the file once (if armed and
    // non-empty) then disarms; idempotent so the manual button AND any of the
    // automatic terminal paths may all call it and only the first writes.
    void OnLotStart(const AnsiString& sLotID);
    void EnsureActive(const AnsiString& sLotID);
    void OnLotEnd();

    // Pick: open a pending row for the nozzle. The caller passes only dies with
    // a genuine 2D identity (Code2D != "" and resolved in the 2D map); OpenRow
    // also no-ops on an empty Code2D as a backstop.
    void OpenRow(int iNozzle,
                 const AnsiString& sCustLot, const AnsiString& sKyecLot,
                 const AnsiString& sProductCode, const AnsiString& sSubstage,
                 const AnsiString& sCode2D, const AnsiString& sLoadTray,
                 const AnsiString& sRetestCode, int iHbin, int iSbin,
                 const AnsiString& sDiePass);

    // Place / reject: complete + buffer the pending row. Both no-op when no
    // pending row is open for the nozzle. Reject leaves col9 (Unload) blank.
    void CommitPlaceRow(int iNozzle, const AnsiString& sUnloadTray);
    void CommitRejectRow(int iNozzle);

    // Drop a pending row WITHOUT emitting (e.g. an IC dropped / abandoned after pick,
    // which is not recorded in Production_Log either). No-op when no pending row is
    // open for the nozzle. Prevents a stale row from being mis-committed by a later IC.
    void DiscardRow(int iNozzle);

    //AI(ht160s-virtual2d) 20260808 : the 2D reader just FABRICATED a code (virtual fallback)
    // instead of reading a die - taint the current flush window. sWhy names which fallback
    // arm fired (simulation checkbox / RealDummy tier / CCD disabled) and is EventLogged once
    // per window. See m_bVirtual2DSeen for the consequences. Called from the reader at the
    // moment of fabrication, NOT from a config check, so it cannot fire on an idle machine.
    void NoteVirtual2D(const AnsiString& sWhy);
};

//---------------------------------------------------------------------------
extern cSoterOutput g_SoterOutput;
//---------------------------------------------------------------------------
#endif
