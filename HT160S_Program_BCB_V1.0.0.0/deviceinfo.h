//---------------------------------------------------------------------------
// TDeviceInfo : per-IC production trace CSV log under
//               D:\HT160S_Log\Production_Log (HT172 TDeviceInfo aligned)
// Base directory comes from HSys.LogRootDir (set at Init time).
// Singleton instance: g_DeviceInfo.
//---------------------------------------------------------------------------
#ifndef deviceinfoH
#define deviceinfoH
//---------------------------------------------------------------------------
#include <vcl.h>
#include <SyncObjs.hpp>
//AI(ht160s-prodlog) 20260716 : full type needed for the by-value m_dailyProd member below
#include "cCsvDailyLog.h"
//---------------------------------------------------------------------------

#define PROD_LOG_FIELD_COUNT    21

enum eICRecordField
{
    eStartTime = 0, eLoadX, eLoadY, eLoadTime, eLoadTrayID,
    eWhichArm, eSuckX, eSuckY, eWhichAuto,
    eBin, eOutTrayID, eUnloadX, eUnloadY, eUnloadTime, eErrorCode,
    // Trace code columns (backward-compat append)
    eTraceCode, eErrorType,
    //AI(ht160s-lotbin) 20260615 : per-IC Lot + 2D code (By Lot+Bin traceability;
    //appended so existing column positions are unchanged)
    eLotID, e2DCode,
    //AI(ht160s-ccd-manual2d) 20260626 : 1 = IC 2D operator hand-entered (appended; positions unchanged)
    eManual2D,
    //AI(ht160s-bin-passfail) 20260708 : per-IC PASS/FAIL vs operator Pass Bin (append; positions unchanged)
    ePassFail
};

//---------------------------------------------------------------------------
// Per-nozzle IC record
//---------------------------------------------------------------------------
struct SICRecord
{
    bool bActive;
    AnsiString sField[PROD_LOG_FIELD_COUNT];

    SICRecord() : bActive(false) {}
    void Clear()
    {
        bActive = false;
        for (int i = 0; i < PROD_LOG_FIELD_COUNT; i++)
            sField[i] = "";
    }
};

//---------------------------------------------------------------------------
class TDeviceInfo
{
private:
    TCriticalSection* m_pCS;
    AnsiString m_sBaseDir;
    AnsiString m_sCachedFilePath;
    AnsiString m_sStartTimeStr;     // Lot start "yyyy-mm-dd_hhnnss"
    AnsiString m_sLotID;

    SICRecord m_records[4];         // One per nozzle (SORT_ARM_NOZZLE_COUNT=4)

    //AI(ht160s-prodlog) 20260716 : ALSO mirror every production row into a per-DAY
    //aggregate CSV so one calendar day is one readable file. The per-lot files above
    //stay the untouched HT172-parity source of truth; this is a coexisting convenience
    //view with its own critical section + own day/month rollover (Production_Log/Daily).
    cCsvDailyLog m_dailyProd;

    AnsiString GetLogFilePath();
    AnsiString GetTitleLine();
    AnsiString GetDataLine(int iNozzle);
    void EnsureHeader(const AnsiString& sPath);
    void AppendLine(const AnsiString& sPath, const AnsiString& sLine);
    AnsiString NowTimeStr();

public:
    TDeviceInfo();
    ~TDeviceInfo();

    void Init();

    //AI(ht160s-prodlog) 20260716 : set the per-day aggregate log retention (called from
    //the ht160s.cpp boot block, same place as the other channels SetRetentionDays)
    void SetDailyRetentionDays(int nDays);

    // Called at lot start
    void OnLotStart(const AnsiString& sLotID, TDateTime dtStart);

    // Called at pick: fill input info per nozzle
    void AddInputInfo(int iNozzle, int iRow, int iCol,
                      const AnsiString& sTrayID);

    //AI(ht160s-lotbin) 20260615 : record this IC's owning Lot + 2D code on the
    //per-nozzle line (called at pick, alongside AddInputInfo).
    void AddIcIdentity(int iNozzle, const AnsiString& sLotID,
                       const AnsiString& sCode2D, bool bManual2D);

    // Called at classify: fill bin assignment per nozzle
    void AddBinInfo(int iNozzle, int iBinIndex, int iGradeCode);

    //AI(ht160s-bin-passfail) 20260708 : per-IC PASS/FAIL text; set before AddOutputInfo flush
    void AddPassFail(int iNozzle, const AnsiString& sPassFail);

    // trace code attach -- 0 = no trace
    void AddTraceInfo(int iNozzle, int iTraceCode);

    // Called at place: fill output info and save CSV line
    void AddOutputInfo(int iNozzle, const AnsiString& sAutoName,
                       const AnsiString& sOutputTray,
                       int iRow, int iCol);

    // Called for rejected ICs
    void SaveRejectRecord(int iNozzle, const AnsiString& sError);

    // Called at cycle done: clear all buffers
    void ClearBatch();
};

//---------------------------------------------------------------------------
extern TDeviceInfo g_DeviceInfo;
//---------------------------------------------------------------------------
#endif
