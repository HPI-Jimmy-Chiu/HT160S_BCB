//---------------------------------------------------------------------------
#ifndef aLoaderH
#define aLoaderH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "HTimer.h"
#include "MotorAndIO/MyMotor.h"   //AI(ht160s-tray-source) 20260625 : eTrayKind/TMyTray for rear-tray hold (Phase 6 A.1)
#include "cCsvDailyLog.h"         //AI(ht160s-overcount-tripqueue) 20260721 : dedicated per-tray OverTrayRecycle CSV
//---------------------------------------------------------------------------
class TMySensor;   //AI(ht160s-clampgrip) 20260806 : fwd-decl for GetCarriagePushOnSensor (pointer return only)
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
    int DestackTask;        //AI(general) 20260617 : front-destacker sub-step for the PRODUCTION feed only. AI(20260731) : the Teach Advanced test drives the same DoFrontDestackDown() helper but through its OWN TestDownTask cursor - the FUNCTION is shared, this FIELD is not.
    bool bTrayEmpty;
    bool bCcdLeftToRight;
    int CcdX;
    int CcdY;
    int Status;
    bool bCleanOutFinish;   //AI(HT160S-Maintainer) 20260605 : this side drained in CleanOut
    HTimer FeedDelay;
    HTimer CcdDelay;
    bool bWaitingAmrFeed;     //AI(ht160s-agv) 20260626 : AMR feed deferral latch (per-side; HT9046 func-static illegal here, 2 sides share DoFeedTray)
    HTimer FeedWaitTimer;     //AI(ht160s-agv) 20260626 : AMR feed deferral countdown (wait for AMR refill before MES0920)
    bool bRise1Waiting;       //AI(ht160s-anti-ghost-d) 20260720 : case-10 rise1-not-retracted wait latch (mirrors bWaitingAmrFeed idiom)
    HTimer Rise1WaitTimer;    //AI(ht160s-anti-ghost-d) 20260720 : case-10 rise1-settle countdown before the named MES0925 Note
};
//---------------------------------------------------------------------------
//AI(ht160s-overcount-tripqueue) 20260721 : per-car feed trip. iTotal = physical
//magazine total for that car (host WORK count + firmware cover/identity header);
//iServed = trays already minted from it. A FIFO of these lets back-to-back cars
//(a 2nd car placed while the 1st still has stock) each keep their own cover/
//identity boundary -- the single-scalar iCarTrayTotal/iFeedSerial lost the 1st
//car's boundary when the 2nd car's CEID274 reset it. Generalises the HT9045
//now/latest two-slot rollover (uLotInfo.cpp:13056 / acatchtray.cpp:5470).
struct TTripEntry
{
    int iTotal;
    int iServed;
};
//---------------------------------------------------------------------------
class TLoaderModule
{
private:
    TLoaderSideState Side[2];
    bool bRearHasTray;
    bool bRearDischargeInProgress;   //AI(ht160s-trayarm-empty-handoff) 20260701 : discharge in flight -- carriage committed rear-ward / at discharge Y + clamps releasing (DoDischargeTray case 100..4000; armed at the case-100 commit since 20260705); rear NOT pickable yet even though bRearHasTray may already be set
    bool bRearReadyForPick;          //AI(ht160s-rearready-state) 20260703 : producer-published rear pickable latch. Set ONLY at DoDischargeTray case 4000 success; cleared on pick / re-armed at the case-100 discharge commit / force-cleared on the rear-sensor-empty edge (RefreshRearState); PRESERVED across InitialFlag when sensor-confirmed settled (ht160s-rearready-p0 20260705). IsRearReadyForPick() reads THIS, not Side[].DischargeTask (removes step-range coupling + the <=4000 terminal off-by-one strand).
    bool bRearResidualAlarmed;       //AI(ht160s-rearready-p0) 20260705 : once-per-episode latch for the MES0924 rear-leftover Note (DoLoader); re-armed when the rear sensor reads empty (RefreshRearState) or on reset
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
    int iSecsCarTrayCount;    //AI(ht160s-agv) 20260627 : LAST host-declared SECS LoaderTrayCount = WORK trays ONLY (dump/visibility only; 0 = host silent). Per-car total now lives in TripQueue, not here.
    //AI(ht160s-overcount-tripqueue) 20260721 : per-car feed trips (FIFO). Replaces the
    //single-scalar iCarTrayTotal/iFeedSerial (retired) so overlapping cars each keep
    //their own cover/identity boundary. Head = trip being consumed; iServed advances at
    //each mint; a trip is popped+freed when iServed reaches iTotal. Owned pointer:
    //new'd in the ctor, entries freed in the dtor / InitialFlag(non-keep-material).
    TList *TripQueue;
    bool   bTripSeen;         //AI(ht160s-overcount-tripqueue) 20260721 : a real trip has been enqueued this episode (over-count vs host-silent discriminator at mint)
    bool   bOverTrayLogged;   //AI(ht160s-overcount-tripqueue) 20260721 : once-per-episode INF_OVERTRAY EventLog latch (cleared on new trip / non-keep init)
    cCsvDailyLog OverTrayLog; //AI(ht160s-overcount-tripqueue) 20260721 : per-tray OverTrayRecycle CSV (D:\HT160S_Log\OverTrayRecycle\<YYYY_MM>\, retention-pruned)
    bool   bOverTrayLogInited;//AI(ht160s-overcount-tripqueue) 20260721 : lazy InitLog latch (LogRootDir must be set -> init on first over-tray, not in ctor)
    //AI(ht160s-tray-source) 20260625 : Phase 6 A.1 - rear-tray hold (transfer-chain relay).
    //Kind is tagged on the carriage Tray grid at feed time; at discharge it is
    //transferred into this module-level hold before ClearTray releases the carriage.
    eTrayKind RearKind;       // kind of the tray currently parked at rear
    AnsiString RearTrayID;    // identity 2D of the rear tray (identity trays only)
    TMyTray RearSourceTray;   // full grid of the rear tray (transfer-chain relay)

    TLoaderSideState *GetSide(int LoaderNo);
    TLoaderSideState *GetOtherSide(int LoaderNo);
    int GetSideIndex(int LoaderNo);
    bool IsValidLoaderNo(int LoaderNo);

    void ResetSide(TLoaderSideState *State);
    void PrepareTrayMap(int LoaderNo);
    bool HasActiveTrayData(int LoaderNo, int Data);
    bool ActiveTrayAllData(int LoaderNo, int Data);
    bool FindNextCcdCell(int LoaderNo, int &CellX, int &CellY);

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
    bool PeekRearOccupied();   //AI(ht160s-loader) 20260708 : non-mutating value-identical twin of IsRearOccupied() (no RefreshRearState) for the interlock + DescribeState dump paths (Empty ComputeRearPickReadyNoRefresh precedent)
    void RefreshRearState();
    bool AcquireFrontOwner(int LoaderNo);
    void ReleaseFrontOwner(int LoaderNo);
    bool IsSoftSimulate();
    bool IsContinuousFeed();   //AI(HT160S-Maintainer) 20260609 : chkLoadTray simulate-feed gate
    bool IsInputHasTrayTrustworthy();   //AI(ht160s-anti-ghost-d) 20260720 : SnLoader_InputHasTray valid only when front rise-1 is confirmed retracted
    bool IsSupplyCarDry();     //AI(ht160s-loader) 20260706 : supply car empty for this side (InputEnd + input HasTray both empty; sim=chkLoadTray) -- the CleanOut RETIRE gate
    bool IsSupplySourceDry();  //AI(ht160s-cleanout-amr) 20260731 : SOURCE ONLY (InputEnd; sim=chkLoadTray) -- the CleanOut ENTRY decision, must NOT be vetoed by a tray still at the input
    int ReadTopCcdBin(int LoaderNo, int CellX, int CellY, bool &bOk);
    AnsiString ReadTopCcd2DCode(int LoaderNo, int CellX, int CellY, bool &bOk);
    void BindManual2D(TLoaderSideState *State, TTrayMotor *TrayMotor);   //AI(ht160s-ccd-manual2d) : operator manual Top CCD 2D bind loop

    bool DoFeedTray(int LoaderNo, int Flag);
    bool DoCcdCheck(int LoaderNo, int Flag);
    bool DoDischargeTray(int LoaderNo, int Flag);
    bool DoFrontDestackDown(int &SubTask, HTimer &Delay);   //AI(general) 20260617 : shared front-destacker separate-one-tray sequence (cylinder-only)
    eTrayKind GetFedTrayKind(int feedSerial, int total);   //AI(ht160s-tray-source) 20260625 : D2 stack-position convention, identity fed LAST (Phase 6 A.2)
    void FlushTripsOnDry();   //AI(ht160s-overcount-tripqueue) 20260721 : source-dry reconcile -- close open trips (declared trays never delivered / car short) so the NEXT car's boundary stays clean
    //AI(ht160s-overcount-tripqueue) 20260721 : TLoaderModule owns a heap TList (TripQueue);
    //it is a singleton (one global via new). Make it non-copyable so a future by-value copy
    //cannot shallow-copy the pointer -> double-free. Declared, never defined (link error on misuse).
    TLoaderModule(const TLoaderModule&);
    TLoaderModule& operator=(const TLoaderModule&);

public:
    TLoaderModule();
    ~TLoaderModule();   //AI(ht160s-overcount-tripqueue) 20260721 : free TripQueue entries + the list
    void InitialFlag(bool bKeepMaterial=false);   //AI(ht160s-home-resume-w1) 20260711 : keep-material HOME preserves the AMR car ledger (host count / car totals / feed serial)
    void PauseTimeoutTimers();     //AI(ht160s-actuator-timer) 20260627 : freeze per-side CcdDelay timeout on machine pause
    void ReStartTimeoutTimers();   //AI(ht160s-actuator-timer) 20260627 : thaw them on resume (csystem actuator-timer enrollment)
    void DoLoader(int LoaderNo, int &Task);
    AnsiString DescribeState();   //AI(ht160s-state-record-analysis) 20260622 : read-only per-side inner-state dump (FeederDecision.txt)
    bool IsRearHasTray();
    bool IsRearReadyForPick();   //AI(ht160s-trayarm-empty-handoff) 20260701 : rear tray present AND discharge fully settled (carriage retreated); model-independent TrayArm pick gate (mirrors Empty)
    //AI(ht160s-tray-source) 20260625 : Phase 6 A.1 - rear-tray accessors (return-by-value,
    //mirror Empty/Color GetSourceTray). TrayArm reads these at pickup to route by Kind.
    eTrayKind GetRearTrayKind();
    TMyTray GetRearSourceTray();
    AnsiString GetRearTrayID();
    void SetCurrentLotNumber(AnsiString Lot);
    bool IsLoaderReadyForSort(int LoaderNo);
    bool HasPickableIC(int LoaderNo);                     //AI(ht160s-sortarm) 20260625 : tray-content "still has pickable ICs" predicate (LS_ToRear-transient-safe) for DoSortArm sticky-side commit
    void ChangeActiveTrayData(int LoaderNo, int SourceData, int TargetData);   //AI(ht160s-ktrayend) 20260709 : public for SortArm pick K_TRAY_END tray-end wipe (Loader owns tray data; mirrors WAR0330 CCD tray-end)
    //AI(ht160s-loader) 20260708 : optional WhyBlocked out-param (default arg ONLY here, not at
    //the definition) tags the refusing rule for the DescribeState dump : "rear-rest" /
    //"gap:other-feeding" / "gap:both-loaded". Production callers pass nothing.
    bool IsLoaderYMoveSafe(int LoaderNo, int Position, AnsiString *WhyBlocked=NULL);   //AI(ht160s-sortarm) 20260624 : public so SortArm's shared-rail Loader-Y move reuses this canonical cross-side gap interlock (was private)
    int GetSortingLoaderNo();
    int GetLoaderStatus(int LoaderNo);
    bool AcquireSortOwner(int LoaderNo);
    void ReleaseSortOwner(int LoaderNo);
    bool IsSortOwnerHeld(int LoaderNo);
    int  GetCarriageGripVerdict(int LoaderNo);        //AI(ht160s-clampgrip) 20260806 : physical PushTray grip verdict for the Y carriage (1=gripping 0=tray gone -1=no verdict); SortArm pick-Z boundary confirm
    TMySensor *GetCarriagePushOnSensor(int LoaderNo); //AI(ht160s-clampgrip) 20260806 : the grip reed, so the alarm screen can name the real IO point
    void NotifyTrayArmPickRearTray();

    //AI(ht160s-agv) 20260623 : AMR handoff interface (mirrors TAutoModule).
    void SetAmrLock(bool bLock);
    bool IsAmrLocked();
    bool IsReadyForAmrHandoff();
    bool IsInputShortageForAmr();
    bool IsInputHandoffFinishedForAmr();
    void RefillSimInfeed();
    void EnqueueTrip(int nWork);   //AI(ht160s-overcount-tripqueue) 20260721 : enqueue a per-car feed trip at CEID274 InfeedRefill; nWork<=0 => WRN_TRIP_NOCOUNT + skip (no enqueue)
    void SetExpectedCarTrayCount(int n);   //AI(ht160s-agv) 20260627 : coordinator latches the SECS LoaderTrayCount on car arrival (before RefillSimInfeed)
    int GetCarTrayCount();   //AI(ht160s-agv) 20260624 : sim input-stack tray count on the shared supply car (PanelMain6 Motion View header)
    bool IsAllCleanOutFinish();   //AI(HT160S-Maintainer) 20260605 : both sides drained in CleanOut

    bool HomeDrainTick();            //AI(ht160s-home-resume-drain) 20260711 : pump the destack cylinder segment (FeedTask 4000/4100/8200/8300) to the 9000 entry boundary
    bool TestGoUpTray(int Flag);     //AI(general) 20260617 : Teach Advanced destacker test (cylinder-only GoUp; shared destacker, no LoaderNo)
    bool TestGoDownTray(int Flag);   //AI(general) 20260617 : Teach Advanced destacker test (cylinder-only GoDown, extracted from DoFeedTray)
    bool CanMoveCcdToCell(int LoaderNo, int CellX, int CellY, AnsiString &Err);  //AI(ht160s-ccd-teach-test) 20260628 : Teach CCD move-to-cell foolproof gate (mirror SortArm CanMoveSuckerToCell); 0-based cells
    bool MoveCcdToCell(int LoaderNo, int CellX, int CellY, int &Task);           //AI(ht160s-ccd-teach-test) 20260628 : task-stepped CCD move to a tray cell (reuses private MoveToCcdCell)
    int GetTrayXCount();   //AI(ht160s-ccd-teach-test) 20260628 : public for Teach numpad upper limit (tray-form columns)
    int GetTrayYCount();   //AI(ht160s-ccd-teach-test) 20260628 : public for Teach numpad upper limit (tray-form rows)
};
//---------------------------------------------------------------------------
extern TLoaderModule *LoaderModule;
void InitializeLoaderModule();
void ShutdownLoaderModule();
//---------------------------------------------------------------------------
#endif