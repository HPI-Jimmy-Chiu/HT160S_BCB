//---------------------------------------------------------------------------
#ifndef aSortArmH
#define aSortArmH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "HTimer.h"   //AI(ht160s-residue) 20260624 : per-slot residue-check timers
//---------------------------------------------------------------------------
struct TSortArmSlotState
{
    bool bCanPick;
    bool bHasIC;
    bool bPlaceSelected;
    int PickX;
    int PickY;
    int PlaceX;
    int PlaceY;
    int TrayData;
    int BinValue;
    int LotIndex;        //AI(ht160s-lotbin) 20260615 : owning LotIndex (By Lot+Bin routing key)
    AnsiString Code2D;   //AI(ht160s-lotbin) 20260615 : IC 2D code (Production_Log trace)
    bool bManual2D;   //AI(ht160s-ccd-manual2d) : IC 2D was operator hand-entered (Production_Log Manual2D)
    int PassClass;       //AI(ht160s-lotpassfail) 20260709 : frozen PASS/FAIL class (By Lot+PassFail routing key)
};
//---------------------------------------------------------------------------
class TTrayMotor;
class TMySucker;
//---------------------------------------------------------------------------
//AI(ht160s-status) 20260703 : explicit module status (approved unified-status design,
//docs/plan/module-status-enum-design-20260703.md). Ladder-owned. SAS_RECOVERY is a
//DERIVED overlay (residue re-suck busy / pick-suck-error latched) computed in
//GetStatus(); the stored member only holds the sequential IDLE/PICKING/PLACING.
enum eSortArmStatus
{
    SAS_IDLE=0,
    SAS_PICKING,      //committed to a Loader side, DoPickFromLoader active
    SAS_PLACING,      //holding IC and/or DoPlaceToAuto active
    SAS_RECOVERY,     //residue re-suck / pick-suck-error recovery pending (derived)
};
//---------------------------------------------------------------------------
class TSortArmModule
{
private:
    int Status;   //AI(ht160s-status) 20260703 : eSortArmStatus (sequential part; see enum note)
    int PickTask;
    int PlaceTask;
    int iActiveLoaderNo;
    int iActiveAutoIndex;
    int iPlaceBaseX;
    int iPlaceY;
    bool bCleanOutFinish;   //AI(HT160S-Maintainer) 20260605 : SortArm drained in CleanOut
    bool bOneCycleFinish;   //AI(HT160S-Maintainer) 20260605 : SortArm placed held IC then stopped (OneCycle)
    TSortArmSlotState Slot[4];
    bool   bNeedResidueCheck[4];   //AI(ht160s-residue) 20260624 : slots that placed an IC this cycle (residue-check targets)
    int    ResidueTask[4];         //AI(ht160s-residue) 20260624 : per-slot residue sub-FSM (1/200/300)
    HTimer ResidueDelay[4];        //AI(ht160s-residue) 20260624 : per-slot re-suck settle timer
    int    iResidueAutoIndex;      //AI(ht160s-residue) 20260624 : Auto to report residue-clear to when bg check completes (-1 idle)
    bool   bResidueArmed;          //AI(ht160s-residue) 20260625 : residue check enabled only after nozzle lifted to top (place case 60)
    unsigned int dwSuckHomeLostStart;   //AI(HT160S-Maintainer) 20260622 : SortArmX suck-home loss debounce (GetTickCount of first loss; 0=clear)
    unsigned int dwHoldLostStart;   //AI(ht160s-falldown) 20260706 : held-IC vacuum-loss debounce (GetTickCount of first OFF; 0=clear)
    bool bMoveAborted;   //AI(ht160s-sortarm) 20260703 : teach abort of an in-flight MoveSuckerToCell (bail before the next axis move; no stray Y after a fault modal raised mid-case-30). Cleared at case 0 / InitialFlag.
    int  iPickRetryCount;   //AI(ht160s-pick-retry) 20260702 : failed pick strokes on the current cell (HT172 iRetryCT port)
    bool bPickSuckErr[4];   //AI(ht160s-pick-retry) 20260702 : per-slot latched pick suck error (nozzle parked until retry/skip)
    int iBaseSuckX;   //AI(ht160s-maintainer) 20260624 : 0-based datum sucker for absolute X (HT172 iBaseSuckX port); 1=suck2 (carriage-fixed nozzle), 0=legacy suck1
    //AI(ht160s-pnp) 20260626 : PnP tuning (SortArm only). Pick/Place Z-down settle dwell plus the
    //pre-lift blow-off dwell. dDestroyCheckTime drives the Task 1 blow dwell (default 0.3s=300ms);
    //loaded per recipe via TfSetup [PnP]. Per-nozzle enable is NOT here - it stays machine-level in
    //GeneralSetting.bSuckerEnabled[4]. bBlowSlot keeps blow ON through the lift; captured before ClearSlot.
    double dPickDelaySec;          //settle dwell after pick Z-down (sec); default 0
    double dPlaceDelaySec;         //settle dwell after place Z-down (sec); default 0
    double dDestroyCheckTime;      //blow-off dwell before Z-up (sec); default 0.3
    bool   bBlowSlot[4];           //placed slots that hold blow ON until the lift clears
    HTimer BlowDwell;              //pre-lift blow dwell timer (vacuum must break before lifting the IC)
    HTimer PnpSettle;              //pick/place Z-down settle dwell timer

    void ClearSlot(int SlotIndex);
    void ClearPickSelection();
    void ClearPlaceSelection();
    void UpdateKitSuckState();
    bool IsSoftSimulate();
    void StartPnpSettle(double Sec);   //AI(ht160s-pnp) 20260626 : arm pick/place Z-down settle dwell (0ms in sim)
    bool PnpSettleElapsed();           //AI(ht160s-pnp) 20260626 : true when the settle dwell completes
    bool IsPickableData(int Data);

    TTrayMotor *GetLoaderMotor(int LoaderNo);
    TTrayMotor *GetLoaderVMotor(int LoaderNo);
    TTrayMotor *GetAutoMotor(int AutoIndex);
    TTrayMotor *GetAutoVMotor(int AutoIndex);
    TTrayMotor *GetSuckZMotor(int SlotIndex);
    TMySucker *GetSucker(int SlotIndex);

    double GetTrayXPitch();
    double GetTrayYPitch();
    int RoundPosition(double Value);
    int GetSortArmCellX(int BaseSortX, int ColMinusSlot);   //AI(ht160s-maintainer) 20260624 : cell->arm X with datum-sucker (iBaseSuckX) offset
    double GetTrayXStart();   //AI(ht160s-maintainer) 20260624 : tray corner->first-IC offset X (P2 HT172-align)
    double GetTrayYStart();
    int CalculatePitchPosition();

    int GetLoaderSortX(int LoaderNo);
    int GetLoaderFirstSortY(int LoaderNo);
    int GetLoaderZPosition(int LoaderNo, int SlotIndex);
    int GetAutoSortX(int AutoIndex);
    int GetAutoFirstSortY(int AutoIndex);
    int GetAutoZPosition(int AutoIndex, int SlotIndex);

    bool MoveSortArmX(int Position);
    bool MoveLoaderY(int LoaderNo, int Position);
    bool MoveAutoY(int AutoIndex, int Position);
    bool MovePitchToTrayPitch();
    bool MoveToLoaderPick();
    bool MoveToAutoPlace();
    bool MovePickZDown();
    bool MovePlaceZDown();
    void ShowPlaceDebugInfo();   //AI(general) 20260609 : place position check (flag-gated)

    bool FindPickCells(int LoaderNo);
    bool SelectPlaceAuto();
    bool FindPlaceCells(int AutoIndex);
    int GetSlotRoutingBin(int SlotIndex);
    int GetMappedAutoIndex(int BinData, int LotIndex, int PassClass, bool &bFixedArea);
    bool CanPlaceSlotToAuto(int SlotIndex, int AutoIndex);

    bool SuckSelectedSlots();
    bool HasPickSuckError();      //AI(ht160s-pick-retry) 20260702 : any slot latched a pick suck error
    void ClearPickSuckErrors();   //AI(ht160s-pick-retry) 20260702 : clear latches + sucker Error for a fresh retry round
    void SkipErroredPickCells();  //AI(ht160s-pick-retry) 20260702 : K_SKIP - write failed cells off (EMPTY_IC) and drop them
    void MarkResidueTargets();    //AI(ht160s-residue) 20260624 : tag this place's slots before ClearSlot
    bool CheckPlaceResidue();     //AI(ht160s-residue) 20260624 : HT172 re-suck residue verify (REALLY only)
    bool IsResidueCheckBusy();    //AI(ht160s-residue) 20260624 : pick gate - true while any nozzle re-suck pending
    bool DestroySelectedSlots();
    void TransferPickDataFromLoader();
    void TransferPlaceDataToAuto();

    bool DoPickFromLoader(int Flag);
    bool DoPlaceToAuto(int Flag);
    bool CheckHoldFallDown(bool bAtPick);   //AI(ht160s-falldown) 20260706 : HT172 OutArmDeviceDropCheck port - held-IC transit vacuum drop monitor (bAtPick=at-Loader restore vs in-transit)

public:
    int GetTrayXCount();   //AI(ht160s-ccd-teach-test) 20260628 : public for Teach numpad upper limit (tray-form columns)
    int GetTrayYCount();   //AI(ht160s-ccd-teach-test) 20260628 : public for Teach numpad upper limit (tray-form rows)
    int GetSortArmCellY(int BaseSortY, int Row);   //AI(ht160s-maintainer) 20260624 : symmetric Y helper (P1 HT172-align); public for DoFeedTray cell-Y park (aAuto1To6)
    TSortArmModule();
    void InitialFlag(bool bKeepMaterial=false);
    void PauseTimeoutTimers();     //AI(ht160s-actuator-timer) 20260627 : freeze ResidueDelay[] timers on machine pause
    void ReStartTimeoutTimers();   //AI(ht160s-actuator-timer) 20260627 : thaw them on resume (csystem actuator-timer enrollment)
    void ApplyPnPDefaults();   //AI(ht160s-pnp) 20260626 : seed PnP scalar defaults (ctor only; recipe values survive re-home)
    void SetPnPParameters(double PickDelaySec, double PlaceDelaySec, double DestroyCheckSec);   //AI(ht160s-pnp) 20260626 : push recipe [PnP] values into the runtime model
    int  GetDestroyCheckMS();   //AI(ht160s-pnp) 20260626 : dDestroyCheckTime as ms for the blow dwell (floor 300 on <=0)
    bool AreAllSuckersHome();   //AI(HT160S-Maintainer) 20260622 : canonical SortArm-move suck-home interlock (live Led[iHomeLed])
    bool SortArmZToSafePos();   //AI(ht160s-sortarm) 20260703 : public for Teach All-Z-up button + not-home recovery (lift all suck-Z to SORT_ARM_SAFE_Z_POSITION)
    void AbortCurrentMove();   //AI(ht160s-sortarm) 20260703 : set abort flag so an in-flight MoveSuckerToCell bails before issuing the next axis move (teach StopSortArmTest)
    void DoSortArm(int &Task);
    bool HasHoldingIC();
    int GetStatus();   //AI(ht160s-status) 20260703 : eSortArmStatus with SAS_RECOVERY overlay
    bool IsCleanOutFinish();   //AI(HT160S-Maintainer) 20260605 : SortArm CleanOut finish
    bool IsOneCycleFinish();   //AI(HT160S-Maintainer) 20260605 : SortArm OneCycle finish
    int  GetPickTask();        //AI(ht160s-state-record-analysis) 20260612 : sub-task readout for Store Hangup snapshot
    int  GetPlaceTask();       //AI(ht160s-state-record-analysis) 20260612 : sub-task readout for Store Hangup snapshot
    AnsiString DescribeHolding();   //AI(ht160s-state-record-analysis) 20260616 : read-only held-IC + routing dump for SortArmDecision.txt
    int GetHeldTargetAutos(int *OutAutoList, int MaxCount);   //AI(ht160s-predictive-supply) 20260707 : held fixed-route target Autos in place-priority order (Sucker1..4); read-only, no AutoModule callback
    bool MoveSuckerToCell(int SlotIndex, int Target, int Col, int Row, bool bZDown, int &Task);   //AI(ht160s-sortarm-flow) 20260617 : Teach Advanced single-nozzle point test (Target 1=Loader1,2=Loader2,11..16=Auto1..6; Col/Row 0-based)
    bool CanMoveSuckerToCell(int SlotIndex, int Target, int Col, int Row, AnsiString &Err);        //AI(ht160s-sortarm-flow) 20260617 : pre-move validation for the point test
};
//---------------------------------------------------------------------------
extern TSortArmModule *SortArmModule;
void InitializeSortArmModule();
void ShutdownSortArmModule();
//---------------------------------------------------------------------------
#endif