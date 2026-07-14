//---------------------------------------------------------------------------
// cSoterOutput : KYEC per-die "Soter" output CSV (one row per handled IC).
//
// Emits one row for every IC that carries a genuine 2D identity resolved in
// the 2D map, whether it ends placed OR rejected. Rows are buffered in memory
// from Lot Start / production START until the lot ends, then written ONCE as a
// single custom-named file:
//   {Date}_{Time}_KYEC-LFT_{Product}_{CustLot}_{KyecLot}_BI_{Substage}_{Sorter}_{Qty}.csv
// under  D:\HT160S_Log\SoterOutput\<yyyymm>\ .
//
// Locked customer decisions (docs/plan/soter-output-csv-gap-analysis-20260714):
//   - Cust lot (col6) == Kyec lot (col7) == the Lot given at Lot Start.
//   - Substage (col5) comes from the 2D-map JSON (no host E87).
//   - SorterID (col10 + filename) = GeneralSetting.sSerialNo.
//   - Load Cover Tray ID (col8)  = incoming identity-tray 2D (Color side).
//   - Unload Cover Tray ID (col9)= flow-lane identity-tray 2D (Auto lane).
//   - File is written once at lot end (LotEnd button OR host STOP / run-dry /
//     CleanOut-finish); OnLotEnd is idempotent so only the first caller writes.
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
    AnsiString   m_sLotID;        // Cust lot == Kyec lot (Start-lot); filename token
    AnsiString   m_sFileProduct;  // filename ProductCode (first emitted row)
    AnsiString   m_sFileSubstage; // filename Substage    (first emitted row)
    TSoterRow    m_pending[SOTER_NOZZLE_COUNT];
    TStringList* m_pLines;        // completed CSV data rows (lot-scoped)
    AnsiString   m_sBaseDir;      // HSys.LogRootDir + "\\SoterOutput"

    void DoArm(const AnsiString& sLotID);   // assumes lock held
    void CommitRow(int iNozzle, const AnsiString& sUnloadTray); // assumes lock
    AnsiString GetTitleLine();
    AnsiString BuildDataLine(const TSoterRow& r, int iNo);
    AnsiString BuildFileName();
    static AnsiString CsvField(const AnsiString& s);  // quote only if needed
    static AnsiString SafeToken(const AnsiString& s); // sanitize filename token

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
};

//---------------------------------------------------------------------------
extern cSoterOutput g_SoterOutput;
//---------------------------------------------------------------------------
#endif
