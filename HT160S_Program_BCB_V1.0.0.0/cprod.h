//---------------------------------------------------------------------------
#ifndef cprodH
#define cprodH
//---------------------------------------------------------------------------
#include <vcl.h>
#include "cmydef.h"
#include "CosFunction.h"
//---------------------------------------------------------------------------
struct TRunData
{
    TDateTime StartTime;
    TDateTime LotEndTime;
    TDateTime AlarmTime;
    TDateTime HomeTime;
    TDateTime PauseTime;
    int BinICCnt[TEST_MAX_BIN];
    int TrayICCnt[eTrayCount];
    int LoaderIC;
    int TotalIC;
    int iPauseTime;
    int JamCount;
    int UPH;
    int iEjectionPinCT; // Ejection pin count
    double JamRate;
    int JamRateDenom;
    int iTotalQuantity;
    int iAutoQuantity;
    int iMagQuantity;

    TRunData()
    {
        Clear();
    }

    void Clear()
    {
        StartTime=Now();
        LotEndTime=StrToTime("00:00:00");
        PauseTime=StrToTime("00:00:00");
        AlarmTime=StrToTime("00:00:00");
        LoaderIC=0;
        TotalIC=0;
        iPauseTime=0;
        JamCount=0;
        UPH=0;
        ZeroMemory(BinICCnt, sizeof(BinICCnt));
        ZeroMemory(TrayICCnt, sizeof(TrayICCnt));
        iEjectionPinCT=0;
        JamRate=0.0;
        JamRateDenom=10000;
        iTotalQuantity=0;
        iAutoQuantity=0;
        iMagQuantity=0;
    }

    void SetAutoQuantity(int iValue){iAutoQuantity=iValue;}
    void SetMagQuantity(int iValue){iMagQuantity=iValue;}
    int GetTotalQuantity(){return iAutoQuantity+iMagQuantity;}
};
extern TRunData tRunData;

//AI(ht160s-uph) 20260707 : on-screen rolling per-tray UPH history feed. cprod owns the
// data (reuses TrayUphLog per-tray detection); main.cpp renders it into UPH_StringGrid.
// Newest row at index 0. Data/render split keeps cprod free of any fMain coupling.
#define UPH_ROW_MAX 10
struct TTrayUphRow
{
    AnsiString sStart;
    AnsiString sEnd;
    AnsiString sPause;
    int        iUph;
};
extern TTrayUphRow g_UphRecentRows[UPH_ROW_MAX];
extern int         g_UphRecentCount;
extern bool        g_UphRowsDirty;

extern int GetJamRateDenom();
//---------------------------------------------------------------------------
class TLatchCycleTime
{
private:
    int iCount;

public:
    TLatchCycleTime(){iCount=0;}
    int LatchCycleTime(bool Start=false);
};
extern TLatchCycleTime lctLoader;
//---------------------------------------------------------------------------
struct TFunction
{
    bool UseCCD;
};
extern TFunction tFunction;
//---------------------------------------------------------------------------
struct TSimulationData
{
    bool bRunSimulation;

    void Clear()
    {
        bRunSimulation=false;
    }
};
extern TSimulationData tSimuData;
//---------------------------------------------------------------------------
typedef struct
{
    int IO_Dev_Num[4];
    bool bInitialScan;

    void TMotionnetIO()
    {
        bInitialScan=false;
    }
}TMotionnetIO;
extern TMotionnetIO tMotionnetIO;
//---------------------------------------------------------------------------
typedef struct
{
    int iXItem;
    int iYItem;
    int iXPitch;
    int iYPitch;
    int iXEdge;
    int iYEdge;
    int iRotate;
    int iMethod;
}TRAY_DATA;
extern TRAY_DATA TrayDef;
//---------------------------------------------------------------------------
typedef struct
{
    int Year;
    int Month;
    int Day;
    int Hour;
    int Minute;
    int Second;
    int Reserved[5];
}DATE_INFO;
//---------------------------------------------------------------------------
typedef struct
{
    int  iCategData[TEST_MAX_BIN];
    int  IfError;
    bool bStackDefFailCate[eTrayCount];
    bool bCategoryFail[eTrayCount];
    int  iCategoryFailCountLimit[eTrayCount];
    int  iOpenBin;
}SYSTEM_BIN_SELECT;
extern SYSTEM_BIN_SELECT BinSelect[2];
//---------------------------------------------------------------------------
//AI(ht160s-password) 20260624 : legacy binary PASS_WORD USER struct removed.
//User accounts now live in THT160UserRoleManager (UserRoleManager.h),
//persisted as notepad-openable text in system\login.txt.
//---------------------------------------------------------------------------
typedef struct
{
    bool iTemperture;
    int  iSoakTime;
    bool OnOffLine;
    bool RealDummy;
    int  iLoad;
    int  iTotal;
    int  iAuto[3];
    int  iFix[3];
    char szRunStatus[24];
    int  iUPH;
    int  iTestCT[8];
    int  iHeadPass[8];
    int  iYieldChart[8][25];
    int  iYieldHour[25];
    int  iYieldMin[25];
    int  iUpdateInterval;
    int  iUpdateCount;
    int  iSystemStatus;
    bool bSiteEnable[8];
    int  iYieldChartByCount[8][25];
    int  iYieldCount[25];
    bool LotStart;
}RUN_INFO;
extern RUN_INFO RunInfo;
extern RUN_INFO RunInfo2;
//---------------------------------------------------------------------------
// MACHINE_RUN_INFO : single machine-level run state for the HT160 SORTER.
// Unlike HT172 (one Lot per run), HT160 holds many Lots at once and has only
// ONE machine finish.  Per-Lot detail lives in THT160LotRegistry (CosFunction);
// this struct is the distilled, single-instance machine summary.
typedef struct
{
    bool bRunning;             // machine is running a sort (HT172 LotStart)
    int  iSystemStatus;        // HALT / PAUSE / HOMING / RUNNING ...
    int  iUPH;                 // units per hour
    int  iTotalScanned;        // ICs whose 2D code was read
    int  iTotalSorted;         // ICs placed into a Bin (reverse-lookup hit)
    int  iUnknown2D;           // reverse-lookup miss -> Error Bin
    int  iActiveLotCount;      // Lots still being sorted
    int  iAreaCount[eTrayCount]; // sorted count per output area
    char szRunStatus[24];      // human-readable status text
    void Clear();
}MACHINE_RUN_INFO;
extern MACHINE_RUN_INFO MachineRun;
//---------------------------------------------------------------------------
bool WriteLastDataFile();
bool ReadLastDataFile();
bool WriteData(char *cFName,char *ptr,int size);
bool ReadData(char *cFName,char *ptr,int size);
bool CheckFileExist(char *cFName);
void ClearLastSet();
void SavePassword();
void ReadPassword();
void CheckLastData();
bool CheckFileCanAccess(char *cFName);
void ReadLastDataIni();
void WriteLastDataIni();
void UpdateAllParameter();
void CustomerFunctionSelect();
AnsiString GetTotalQuantityAutoPercent();
AnsiString GetTotalQuantityMagPercent();
//AI(ht160s-lot-reset) 20260706 : zero the per-run production counters at the start
//of a new work order (manual Lot Start button + SECS LOTSTART). Machine-total
//cumulative fields are left alone; only the per-lot display / throughput counts
//reset so the Auto Cnt display, UPH and SECS Scanned/Sorted represent THIS lot.
void ResetPerLotProductionCounters();
//AI(ht160s-uph) 20260706 : per-tray + per-lot UPH logging. Non-invasive observer
//(reads the public TAutoModule station status) + CSV under
//<LogRoot>\UPHLog\YYYY_MM\<LotID>__<ts>\. Total UPH uses the HT172 aggregate
//(GetCalculateUPH); per-tray UPH is diagnostic. See
//docs/plan/uph-suite-persistence-biniccnt-plan-20260706.md.
void TrayUphLog_OnLotStart(AnsiString LotID);
void TrayUphLog_Tick();
void TrayUphLog_OnLotEnd(AnsiString LotID, int TotalIC, int LotUPH);
void TrayUphLog_PruneOld();
//---------------------------------------------------------------------------
#endif