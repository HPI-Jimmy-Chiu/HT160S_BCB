//---------------------------------------------------------------------------
#ifndef aLoaderH
#define aLoaderH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "HTimer.h"
#include "MotorAndIO/MyMotor.h"   //AI(ht160s-tray-source) 20260625 : eTrayKind/TMyTray for rear-tray hold (Phase 6 A.1)
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
    //AI(ht160s-tray-source) 20260625 : Phase 6 A.1 - rear-tray hold (transfer-chain relay).
    //Kind is tagged on the carriage Tray grid at feed time; at discharge it is
    //transferred into this module-level hold before ClearTray releases the carriage.
    int iFeedSerial;          // 1-based feed counter on the shared supply car (sim count)
    eTrayKind RearKind;       // kind of the tray currently parked at rear
    AnsiString RearTrayID;    // identity 2D of the rear tray (identity trays only)
    TMyTray RearSourceTray;   // full grid of the rear tray (transfer-chain relay)

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
    double GetTrayXStart();   //AI(ht160s-maintainer) 20260624 : tray corner->first-IC offset (P2 HT172-align)
    double GetTrayYStart();
    int GetLoaderFeedY(int LoaderNo);
    int GetLoaderDischargeY(int LoaderNo);
    int GetLoaderFirstCcdY(int LoaderNo);
    int GetTopCcdFirstX();
    int RoundPosition(double Value);

    bool MoveLoaderY(int LoaderNo, int Position);
    bool MoveTopCcdX(int Position);
    bool MoveToCcdCell(int LoaderNo, int CellX, int CellY);
    bool IsOutputBottomOccupied();
    bool IsRearOccupied();
    void RefreshRearState();
    bool AcquireFrontOwner(int LoaderNo);
    void ReleaseFrontOwner(int LoaderNo);
    bool IsSoftSimulate();
    bool IsContinuousFeed();   //AI(HT160S-Maintainer) 20260609 : chkLoadTray simulate-feed gate
    int ReadTopCcdBin(int LoaderNo, int CellX, int CellY, bool &bOk);
    AnsiString ReadTopCcd2DCode(int LoaderNo, int CellX, int CellY, bool &bOk);
    void BindManual2D(TLoaderSideState *State, TTrayMotor *TrayMotor);   //AI(ht160s-ccd-manual2d) : operator manual Top CCD 2D bind loop

    bool DoFeedTray(int LoaderNo, int Flag);
    bool DoCcdCheck(int LoaderNo, int Flag);
    bool DoDischargeTray(int LoaderNo, int Flag);
    bool DoFrontDestackDown(int &SubTask, HTimer &Delay);   //AI(general) 20260617 : shared front-destacker separate-one-tray sequence (cylinder-only)
    eTrayKind GetFedTrayKind(int feedSerial, int total);   //AI(ht160s-tray-source) 20260625 : D2 stack-position convention, identity fed LAST (Phase 6 A.2)

public:
    TLoaderModule();
    void InitialFlag();
    void DoLoader(int LoaderNo, int &Task);
    AnsiString DescribeState();   //AI(ht160s-state-record-analysis) 20260622 : read-only per-side inner-state dump (FeederDecision.txt)
    bool IsRearHasTray();
    //AI(ht160s-tray-source) 20260625 : Phase 6 A.1 - rear-tray accessors (return-by-value,
    //mirror Empty/Color GetSourceTray). TrayArm reads these at pickup to route by Kind.
    eTrayKind GetRearTrayKind();
    TMyTray GetRearSourceTray();
    AnsiString GetRearTrayID();
    void SetCurrentLotNumber(AnsiString Lot);
    bool IsLoaderReadyForSort(int LoaderNo);
    bool HasPickableIC(int LoaderNo);                     //AI(ht160s-sortarm) 20260625 : tray-content "still has pickable ICs" predicate (LS_ToRear-transient-safe) for DoSortArm sticky-side commit
    bool IsLoaderYMoveSafe(int LoaderNo, int Position);   //AI(ht160s-sortarm) 20260624 : public so SortArm's shared-rail Loader-Y move reuses this canonical cross-side gap interlock (was private)
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
    int GetCarTrayCount();   //AI(ht160s-agv) 20260624 : sim input-stack tray count on the shared supply car (PanelMain6 Motion View header)
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