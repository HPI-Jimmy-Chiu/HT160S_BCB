//---------------------------------------------------------------------------
#ifndef aEmptyH
#define aEmptyH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "HTimer.h"
#include "MotorAndIO/MyMotor.h"   //AI(ht160s-tray-source) : TMyTray FrontSourceTray holder; rear tray on MEmptyY
//---------------------------------------------------------------------------
class TMyCylinder;   //AI(ht160s-color-align-empty) : fwd-decl for PushCylinder/PopCylinder helpers (mirror aColor.h)
//---------------------------------------------------------------------------
class TEmptyModule
{
private:
    int FeedTask;
    int FeedClampSub;   //AI(HT160S-Maintainer) 20260623 : DoClampTray sub-state for DoFeedTray
    int GoDownTask;
    int GoUpTask;
    int TestUpTask;
    int TestDownTask;
    HTimer TestDelay;
    bool bFrontHasTray;
    bool bRearHasTray;
    bool bReturnTray;
    bool bRearReturnInProgress;   //AI(ht160s-trayarm-empty-handoff) 20260701 : DoGoUpTray active (carrier re-clamping/returning the rear tray); rear NOT pickable even if bRearHasTray still true
    bool bTrayXToEmptyFinish;
    bool bLotFinish;
    TMyTray FrontSourceTray;  //AI(ht160s-tray-source) : FRONT staging holder only (rule #1); REAR tray lives on HSys.Mot.MEmptyY->Tray (parity with Loader, drives MotionView)
    bool bAmrLocked;          //AI(ht160s-agv) 20260623 : AMR P2 handoff lock (freeze front destack)
    int iSimInfeedCount;      //AI(ht160s-agv) 20260623 : sim input-stack tray count (drains per GoDown)
    bool bWaitingAmrFeed;     //AI(ht160s-agv) Empty source-dry AMR wait latch
    HTimer AmrFeedWaitTimer;  //AI(ht160s-agv) Empty source-dry AMR wait timer
    HTimer FeedDelay;
    HTimer GoDownDelay;
    HTimer GoUpDelay;

    bool IsSoftSimulate();
    void RefreshStateFromSensors();
    void BirthFrontTray();    //AI(ht160s-tray-source) : born at DoGoDownTray front confirm (rule #1)
    void BirthRearTray();     //AI(ht160s-tray-source) : direct rear birth for REALLY-mode startup/recovery re-latch
    bool MoveEmptyY(int Position);
    bool PushCylinder(TMyCylinder &Cyn);   //AI(ht160s-color-align-empty) : sim/Enable-aware cylinder push (mirror aColor; alarm-on-timeout via Push())
    bool PopCylinder(TMyCylinder &Cyn);    //AI(ht160s-color-align-empty) : sim/Enable-aware cylinder pop
    bool DoFeedTray(int Flag);
    bool DoGoDownTray(int Flag);
    bool DoGoUpTray(int Flag);

public:
    TEmptyModule();
    void InitialFlag();
    void PauseTimeoutTimers();     //AI(ht160s-actuator-timer) 20260627 : freeze AmrFeedWaitTimer (source-dry AMR wait) on machine pause
    void ReStartTimeoutTimers();   //AI(ht160s-actuator-timer) 20260627 : thaw it on resume (csystem actuator-timer enrollment)
    void DoEmpty(int &Task);
    AnsiString DescribeState();   //AI(ht160s-state-record-analysis) 20260622 : read-only inner-state dump (FeederDecision.txt)

    bool IsFrontHasTray();
    bool IsRearHasTray();
    bool IsCleanOutFinish();   //AI(cleanout) 20260701 : Empty CleanOut-drain finish (TrayArm done + flow clear + rise cylinders home)
    bool IsRearReadyForPick();   //AI(ht160s-trayarm-empty-handoff) 20260701 : present AND not being returned; model-independent successor to the TrayArm magic-position gate
    bool IsReturnTrayRequested();
    void SetRearHasTray(bool bHasTray);
    TMyTray GetSourceTray();   //AI(ht160s-tray-source) : return-by-value deep copy of the rear tray grid for TrayArm
    void RequestReturnTray();
    void NotifyTrayXToEmptyFinish();

    //AI(ht160s-agv) 20260623 : AMR P2 (EmptyTray) handoff interface (mirrors TAutoModule).
    void SetAmrLock(bool bLock);
    bool IsAmrLocked();
    bool IsReadyForAmrHandoff();
    bool IsInputShortageForAmr();
    bool IsInputHandoffFinishedForAmr();
    bool IsOutputCarFullForAmr();   //AI(cleanout) 20260703 : SnEmpty_InputFullTray verdict (sim-false) - CleanOut GoUp/finish Full gate
    void RefillSimInfeed();
    int GetCarTrayCount();   //AI(ht160s-agv) 20260624 : sim input-stack tray count on the supply car (PanelMain6 Motion View header)

    bool TestGoUpTray(int Flag);     //AI(general) 20260617 : Teach Advanced destacker test (cylinder-only GoUp, mirrors DoGoUpTray rise steps)
    bool TestGoDownTray(int Flag);   //AI(general) 20260617 : Teach Advanced destacker test (cylinder-only GoDown, mirrors DoGoDownTray)
};
//---------------------------------------------------------------------------
extern TEmptyModule *EmptyModule;
void InitializeEmptyModule();
void ShutdownEmptyModule();
//---------------------------------------------------------------------------
#endif