//---------------------------------------------------------------------------
#ifndef aLoaderH
#define aLoaderH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "HTimer.h"
//---------------------------------------------------------------------------
// Loader per-side handshake status for the shared Loader-Y axis. SortArm uses
// this to know when a side has finished CCD scanning and the Y axis may be
// handed over for picking. See AcquireSortOwner/ReleaseSortOwner/IsSortOwnerHeld.
enum eLoaderStatus
{
    LS_IDLE=0,            // No tray / nothing to do
    LS_FEEDING=1,         // Feeding a tray, Loader owns the Y axis
    LS_CCD_SCAN=2,        // Top CCD scanning, Loader owns the Y axis
    LS_READY_SORT=3,      // CCD done, Y parked, available for SortArm to take
    LS_SORTING=4,         // Y axis ownership granted to SortArm
    LS_ToRear=5,          // Y axis will go to rear
};
//---------------------------------------------------------------------------
struct TLoaderSideState
{
    int FeedTask;
    int CcdTask;
    int DischargeTask;
    int DestackTask;        //AI(general) 20260617 : front-destacker sub-step (shared by DoFeedTray + TestGoDownTray)
    bool bTrayEmpty;
    bool bCcdLeftToRight;
    int CcdX;
    int CcdY;
    int Status;
    bool bCleanOutFinish;   //AI(HT160S-Maintainer) 20260605 : this side drained in CleanOut
    HTimer FeedDelay;
    HTimer CcdDelay;
};
//---------------------------------------------------------------------------
class TLoaderModule
{
private:
    TLoaderSideState Side[2];
    bool bRearHasTray;
    int iFrontOwner;
    int iTopCcdCount;
    int iYOwner[2];
    int TestUpTask;          //AI(general) 20260617 : Teach Advanced destacker test (GoUp)
    int TestDownTask;        //AI(general) 20260617 : Teach Advanced destacker test (GoDown)
    HTimer TestDelay;        //AI(general) 20260617 : Teach Advanced destacker test settle delay
    int SimuCcdCycleIndex;   // round-robin cursor over LotRegistry codes (simulation only)
    AnsiString CurrentLotNumber;
    bool bAmrLocked;          //AI(ht160s-agv) 20260623 : AMR handoff lock (freeze front destack)
    int iSimInfeedCount;      //AI(ht160s-agv) 20260623 : sim input-stack tray count (drains per destack)

    TLoaderSideState *GetSide(int LoaderNo);
    TLoaderSideState *GetOtherSide(int LoaderNo);
    int GetSideIndex(int LoaderNo);
    bool IsValidLoaderNo(int LoaderNo);

    void ResetSide(TLoaderSideState *State);
    void PrepareTrayMap(int LoaderNo);
    void ChangeActiveTrayData(int LoaderNo, int SourceData, int TargetData);
    bool HasActiveTrayData(int LoaderNo, int Data);
    bool ActiveTrayAllData(int LoaderNo, int Data);
    bool FindNextCcdCell(int LoaderNo, int &CellX, int &CellY);

    int GetTrayXCount();
    int GetTrayYCount();
    double GetTrayXPitch();
    double GetTrayYPitch();
    int GetLoaderFeedY(int LoaderNo);
    int GetLoaderDischargeY(int LoaderNo);
    int GetLoaderFirstCcdY(int LoaderNo);
    int GetTopCcdFirstX();
    int RoundPosition(double Value);

    bool MoveLoaderY(int LoaderNo, int Position);
    bool MoveTopCcdX(int Position);
    bool MoveToCcdCell(int LoaderNo, int CellX, int CellY);
    bool IsLoaderYMoveSafe(int LoaderNo, int Position);   //AI(HT160S-Maintainer) 20260610 : cross-side safe-distance interlock
    bool IsOutputBottomOccupied();
    bool IsRearOccupied();
    void RefreshRearState();
    bool AcquireFrontOwner(int LoaderNo);
    void ReleaseFrontOwner(int LoaderNo);
    bool IsSoftSimulate();
    bool IsContinuousFeed();   //AI(HT160S-Maintainer) 20260609 : chkLoadTray simulate-feed gate
    int ReadTopCcdBin(int LoaderNo, int CellX, int CellY, bool &bOk);
    AnsiString ReadTopCcd2DCode(int LoaderNo, int CellX, int CellY, bool &bOk);

    bool DoFeedTray(int LoaderNo, int Flag);
    bool DoCcdCheck(int LoaderNo, int Flag);
    bool DoDischargeTray(int LoaderNo, int Flag);
    bool DoFrontDestackDown(int &SubTask, HTimer &Delay);   //AI(general) 20260617 : shared front-destacker separate-one-tray sequence (cylinder-only)

public:
    TLoaderModule();
    void InitialFlag();
    void DoLoader(int LoaderNo, int &Task);
    AnsiString DescribeState();   //AI(ht160s-state-record-analysis) 20260622 : read-only per-side inner-state dump (FeederDecision.txt)
    bool IsRearHasTray();
    void SetCurrentLotNumber(AnsiString Lot);
    bool IsLoaderReadyForSort(int LoaderNo);
    int GetSortingLoaderNo();
    int GetLoaderStatus(int LoaderNo);
    bool AcquireSortOwner(int LoaderNo);
    void ReleaseSortOwner(int LoaderNo);
    bool IsSortOwnerHeld(int LoaderNo);
    void NotifyTrayArmPickRearTray();

    //AI(ht160s-agv) 20260623 : AMR handoff interface (mirrors TAutoModule).
    void SetAmrLock(bool bLock);
    bool IsAmrLocked();
    bool IsReadyForAmrHandoff();
    bool IsInputShortageForAmr();
    bool IsInputHandoffFinishedForAmr();
    void RefillSimInfeed();
    bool IsAllCleanOutFinish();   //AI(HT160S-Maintainer) 20260605 : both sides drained in CleanOut

    bool TestGoUpTray(int Flag);     //AI(general) 20260617 : Teach Advanced destacker test (cylinder-only GoUp; shared destacker, no LoaderNo)
    bool TestGoDownTray(int Flag);   //AI(general) 20260617 : Teach Advanced destacker test (cylinder-only GoDown, extracted from DoFeedTray)
};
//---------------------------------------------------------------------------
extern TLoaderModule *LoaderModule;
void InitializeLoaderModule();
void ShutdownLoaderModule();
//---------------------------------------------------------------------------
#endif