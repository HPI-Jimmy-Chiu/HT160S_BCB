#include "IncludeAllHeader.h"       //Dell 將.h統一,可加速build
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "language.h"

#include "aEmpty.h"
#include "database.h"
#include "GeneralSetting.h"   //AI(ht160s-agv) 20260623 : iSimAmrMaxTray
#include "cmydef.h"
#include "mymessbox.h"
#include "uteach.h"
#include "aTrayArm.h"   //AI(cleanout) 20260701 : TrayArmModule->IsCleanOutFinish() gates the Empty CleanOut drain/finish
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TEmptyModule *EmptyModule=NULL;
//---------------------------------------------------------------------------
TEmptyModule::TEmptyModule()
{
    InitialFlag();
}
//---------------------------------------------------------------------------
void TEmptyModule::InitialFlag()
{
    bAmrLocked=false;
    bWaitingAmrFeed=false;       //AI(ht160s-agv) Empty source-dry AMR wait latch
    AmrFeedWaitTimer.Clear();   //AI(ht160s-agv) Empty source-dry AMR wait timer
    RefillSimInfeed();
    FeedTask=1;
    FeedClampSub=0;
    GoDownTask=1;
    GoUpTask=1;
    bFrontHasTray=false;
    bRearHasTray=false;
    bReturnTray=false;
    bRearReturnInProgress=false;   //AI(ht160s-trayarm-empty-handoff) 20260701 : no rear-return in flight at init
    bTrayXToEmptyFinish=false;
    bLotFinish=false;
    FrontSourceTray.Clear();   //AI(ht160s-tray-source) : no stale front grid across init/lot
    if(HSys.VMot.MMEmptyY!=NULL) HSys.VMot.MMEmptyY->ClearTray();   //AI(ht160s-tray-source) : rear tray lives on MEmptyY; hide grid on init
    FeedDelay.Clear();
    GoDownDelay.Clear();
    GoUpDelay.Clear();
    TestUpTask=1;
    TestDownTask=1;
    TestDelay.Clear();
}
//---------------------------------------------------------------------------
bool TEmptyModule::IsSoftSimulate()
{
    #ifdef SOFT_SIMULATE
    return true;
    #else
    return (HSys.LastSet.iRealDummy==DUMMY);
    #endif
}
//---------------------------------------------------------------------------
//AI(ht160s-actuator-timer) 20260627 : freeze/thaw this module's source-dry AMR wait
//timer (AmrFeedWaitTimer -> MES1022 on expiry) so a machine pause taken during the AMR
//refill wait is not charged against the wait budget -- no premature timeout alarm on
//resume. Called from csystem PauseActuatorTimeoutTimers/ReStartActuatorTimeoutTimers on
//the SystemStart pause/resume edges, alongside Cylinder[]/SortArmSuck (mirrors Color).
void TEmptyModule::PauseTimeoutTimers()
{
    AmrFeedWaitTimer.Pause();
}
//---------------------------------------------------------------------------
void TEmptyModule::ReStartTimeoutTimers()
{
    AmrFeedWaitTimer.ReStart();
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : AMR P2 (EmptyTray) handoff interface, mirrors TAutoModule.
void TEmptyModule::SetAmrLock(bool bLock)
{
    bAmrLocked=bLock;
}
//---------------------------------------------------------------------------
bool TEmptyModule::IsAmrLocked()
{
    return bAmrLocked;
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : Ready = front stacking cylinders back home (not commanded
//up); destack idle so the AGV may refill. Held stable by bAmrLocked (DoEmpty stops
//starting new destacks while locked).
bool TEmptyModule::IsReadyForAmrHandoff()
{
    //AI(ht160s-agv) 20260625 : SIM-ONLY bypass of the CEID273 READY cylinder-out-bit gate.
    //In SOFT_SIMULATE/DUMMY the front-destack out-bits may not settle false, latching the
    //AMR lock forever (no watchdog). Real-machine #else interlock below stays fully active.
    if(IsSoftSimulate())
        return true;
    return (HSys.Cyn.C_Empty_FrontRiseTray_1.GetOutBit()==false
            && HSys.Cyn.C_Empty_FrontRiseTray_2.GetOutBit()==false
            && HSys.Cyn.C_Empty_FrontSeparateTray_1.GetOutBit()==false);
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : shortage (call AGV). Sim drains iSimInfeedCount to 0; real
//reads SnEmpty_InputEnd (ON=has tray, OFF=empty). Disabled sensor -> no call.
bool TEmptyModule::IsInputShortageForAmr()
{
    if(IsSoftSimulate())
        return (iSimInfeedCount<=0);
    return (HSys.Sen.SnEmpty_InputEnd.Enable==true && HSys.Sen.SnEmpty_InputEnd.IsOff());
}
//---------------------------------------------------------------------------
//AI(cleanout) 20260703 : supply-stack Full verdict for the CleanOut GoUp/finish gate.
//Real machine reads SnEmpty_InputFullTray; sim returns false so a laptop CleanOut can
//never dead-lock on the InType=0 phantom-present read (no MN200 card).
bool TEmptyModule::IsOutputCarFullForAmr()
{
    if(IsSoftSimulate())
        return false;
    return (HSys.Sen.SnEmpty_InputFullTray.Enable==true && HSys.Sen.SnEmpty_InputFullTray.IsOn());
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : Finish = refill complete. Sim auto-completes (no sensor);
//real waits for SnEmpty_InputEnd to read a tray present (ON).
bool TEmptyModule::IsInputHandoffFinishedForAmr()
{
    if(IsSoftSimulate())
        return true;
    return (HSys.Sen.SnEmpty_InputEnd.Enable==true && HSys.Sen.SnEmpty_InputEnd.IsOn());
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : reset sim input-stack to configured max (AGV delivered a
//full magazine). Real machine ignores the count (sensor-driven).
void TEmptyModule::RefillSimInfeed()
{
    iSimInfeedCount=GeneralSetting.iSimAmrMaxTray[1];   // index 1 = Empty
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260624 : trays currently on the Empty supply car (PanelMain6 header).
//Sim drains per GoDown; real machine sensor-driven (count not maintained, reads max).
int TEmptyModule::GetCarTrayCount()
{
    return iSimInfeedCount;
}
//---------------------------------------------------------------------------
void TEmptyModule::RefreshStateFromSensors()
{
    //AI(HT160S-Maintainer) 20260622 : in SOFT_SIMULATE / real-machine DUMMY mode there are no
    //real trays, so DO NOT read the physical sensors here - the tray state is a LATCH owned by
    //the action sequence (DoFeedTray/DoGoDownTray set it when a tray is fed / comes down; the
    //TrayArm clears it via SetRearHasTray(false) when MotorY takes the tray away).
    //WHY reading sensors is wrong in sim/dummy: these HasTray inputs are wired InType=0
    //(active-low), and with no MN200 card TMyIo::IsOn() falls back to bOutValue(=false for a
    //never-written input), which InType=0 then inverts to TMySensor::IsOn()==true. So every
    //InType=0 input would read 'tray present' regardless of reality (the opposite of what
    //IOsetview shows on real hardware), wiping the latch and stalling the module. (Mirrors the
    //iRealDummy!=DUMMY gates on the sensor-miss checks in DoFeedTray/DoGoDownTray.)
    if(IsSoftSimulate())
        return;

    bool bHasRearSensor=false;
    bool bRearState=false;

    if(HSys.Sen.SnEmpty_InputHasTray.Enable==true)
        bFrontHasTray=HSys.Sen.SnEmpty_InputHasTray.IsOn();

    if(HSys.Sen.SnEmpty_OutputBottomHasTray.Enable==true)
    {
        bHasRearSensor=true;
        if(HSys.Sen.SnEmpty_OutputBottomHasTray.IsOn())
            bRearState=true;
    }

    if(bHasRearSensor)
    {
        //AI(ht160s-tray-source) : in REALLY mode a tray can latch present from the sensor
        //without the DoFeedTray birth running (tray already at rear on startup/recovery).
        //Birth the grid on the false->true edge so TrayArm always carries a born EMPTY_IC/Normal
        //grid, never a stale rear grid. (sim/dummy already returned above.)
        if(bRearHasTray==false && bRearState)
            BirthRearTray();
        else if(bRearHasTray && bRearState==false && HSys.VMot.MMEmptyY!=NULL)
            HSys.VMot.MMEmptyY->ClearTray();   //AI(ht160s-tray-source) : rear tray gone -> hide motor grid
        bRearHasTray=bRearState;
    }
}
//---------------------------------------------------------------------------
bool TEmptyModule::MoveEmptyY(int Position)
{
    if(HSys.Mot.MEmptyY==NULL)
        return false;
    if(HSys.Mot.MEmptyY->CheckSoftLimit(Position)==false)
    {
        ShowMotorLimitError(HSys.Mot.MEmptyY->AlarmName[eMotOverLimitErr], LangT("Empty Y motor will out of limit"), HSys.Mot.MEmptyY, Position);
        return false;
    }

    #ifndef SOFT_SIMULATE
    int TrayArmPos=0;
    if(HSys.Mot.MTrayArmX!=NULL)
        TrayArmPos=HSys.Mot.MTrayArmX->ReadEncoderPos();
    if(HSys.Cyn.C_TrayArmZ_Up.IsOn()==false &&
       (TrayArmPos+500)>=Teach.TrayXArmToEmptyXPosition)
    {
        return false;
    }
    #endif

    return HSys.Mot.MEmptyY->MotorMove(Position);
}
//---------------------------------------------------------------------------
void TEmptyModule::DoEmpty(int &Task)
{
    switch(Task)
    {
        case 1:
            Task=10;
            break;

        case 10:
            Task=100;
            break;

        case 100:
            //AI(ht160s-agv) 20260625 : NARROWED AMR lock (was 'if(bAmrLocked) break;'
            //which froze the WHOLE feed loop and starved TrayArm/SortArm downstream).
            //While AMR-locked we still SKIP the FRONT destack branches (bReturnTray
            //GoUpTray; bFrontHasTray==false GoDownTray; bLotFinish GoUpTray) so the AGV
            //may refill the front stack, but we ALLOW the REAR-feed branch (DoFeedTray)
            //to keep moving an already-staged front tray to the rear so downstream is
            //not starved. Mirrors Loader's narrow lock (aLoader.cpp case 100).
            RefreshStateFromSensors();
            //AI(cleanout) 20260701 : CleanOut drain phase. Once TrayArm has finished (Loader +
            //Auto drained, nothing more to recover/supply), stop feeding and GoUp every tray back
            //to the car via the existing bLotFinish drain path (it also gates off GoDown/feed
            //below). Before TrayArm finishes (produce phase) bLotFinish stays false so Empty keeps
            //supplying Autos normally; outside CleanOut it is always false.
            bLotFinish = (HSys.Sys.RunMode==Run_CleanOut &&
                          TrayArmModule!=NULL && TrayArmModule->IsCleanOutFinish());
            if(bReturnTray)
            {
                if(bAmrLocked)
                    break;   //AI(ht160s-agv) front GoUp suspended during AMR handoff
                DoGoUpTray(0);
                Task=3000;
                break;
            }

            if(bFrontHasTray==false && bLotFinish==false)
            {
                if(bAmrLocked)
                    break;   //AI(ht160s-agv) front GoDown suspended during AMR handoff
                DoGoDownTray(0);
                Task=1000;
                break;
            }

            //AI(ht160s-agv) source-dry : the supply magazine ran out (SnEmpty_InputEnd
            //OFF). In AMR mode wait iAmrFeedWaitSec for the AGV to refill before alarming;
            //the AGV refill is detected on a later cycle (IsInputHandoffFinishedForAmr).
            //Anchored on IsInputShortageForAmr (NOT bFrontHasTray) per the InputEnd rule.
            if(IsInputShortageForAmr() && HSys.Sys.RunMode!=Run_CleanOut)   //AI(cleanout) 20260701 : no Empty source-dry alarm during CleanOut
            {
                if(GeneralSetting.bUseAMR)
                {
                    if(bWaitingAmrFeed==false)
                    {
                        AmrFeedWaitTimer.SetMS(GeneralSetting.iAmrFeedWaitSec*1000);
                        AmrFeedWaitTimer.On();
                        bWaitingAmrFeed=true;
                        break;
                    }
                    if(AmrFeedWaitTimer.Off()==false)
                        break;   //AI(ht160s-agv) still waiting for the AGV to refill
                    bWaitingAmrFeed=false;
                    AmrFeedWaitTimer.Clear();
                }
                //AI(ht160s-agv) decision (b) : NEW machine-local code MES1022 (NOT a reuse
                //of MES1021). Distinct from Empty MES1021 (rear bottom-miss) and MES1024
                //(front miss) so operator + cloud/host can tell source-dry apart.
                //Registered in system/AlarmList.csv via CreateSystemAlarmCode (database.cpp); language pass pending.
                //AI(ht160s-maintainer) 20260701 : CLEAN OUT dropped from this alarm's
                //recovery set. Clean-out only applies when Loader itself is out of trays
                //(aLoader.cpp MES0920/MES0921); Empty source-dry is cleared by an
                //AGV/operator reload, not by draining the pipeline -- offering it here
                //previously did nothing on select (Ret was never checked for it).
                {
                    int Ret=ShowMyError("MES1022", LangT("Empty supply magazine empty"), &HSys.Sen.SnEmpty_InputEnd, true, K_RETRY);
                    if(Ret==K_RETRY)
                    {
                        //AI(ht160s-maintainer) 20260701 : no real magazine to reload in
                        //SOFT_SIMULATE/DUMMY, so a bare Retry would just re-hit this same
                        //shortage and loop the alarm forever. Refill the virtual car so
                        //the retried pass finds stock again, mirroring a real AGV/operator
                        //reload. Real hardware is untouched (IsSoftSimulate()==false there).
                        if(IsSoftSimulate())
                            RefillSimInfeed();
                        Task=1;   //AI(ht160s-agv) re-enter case 100; if AGV refilled, shortage clears
                    }
                }
                break;
            }
            //AI(ht160s-agv) refill arrived (or sim) : drop the latch so the next dry edge re-arms.
            if(bWaitingAmrFeed && IsInputShortageForAmr()==false)
            {
                bWaitingAmrFeed=false;
                AmrFeedWaitTimer.Clear();
            }

            if(bRearHasTray==false && bLotFinish==false)
            {
                DoFeedTray(0);   //AI(ht160s-agv) rear feed RUNS while AMR-locked (anti-starve)
                Task=2000;
                break;
            }

            if(bLotFinish && (bFrontHasTray || bRearHasTray))   //AI(cleanout) 20260701 : drain rear too, not only front
            {
                if(bAmrLocked)
                    break;   //AI(ht160s-agv) front GoUp suspended during AMR handoff
                //AI(cleanout) 20260703 : Full gate (user design). Never GoUp into a full
                //supply stack : hold the drain and ask the operator to empty it (AMR=0 ->
                //operator; the modal repeats until the Full sensor goes OFF, mirroring Auto
                //ServiceCarFull). IsOutputCarFullForAmr is sim-false, so this never blocks sim.
                if(IsOutputCarFullForAmr())
                {
                    do
                    {
                        ShowMyError("MES1023", "Empty supply stack FULL (sensor) - remove stacked trays", &HSys.Sen.SnEmpty_InputFullTray, false, K_RETRY);
                    }
                    while(HSys.Sen.SnEmpty_InputFullTray.Enable==true && HSys.Sen.SnEmpty_InputFullTray.IsOn());
                }
                DoGoUpTray(0);
                Task=3000;
            }
            break;

        case 1000:
            if(DoGoDownTray(1))
            {
                if(IsSoftSimulate() && iSimInfeedCount>0)
                    iSimInfeedCount--;   //AI(ht160s-agv) sim input drains 1/GoDown
                Task=1;
            }
            break;

        case 2000:
            if(DoFeedTray(1))
                Task=1;
            break;

        case 3000:
            if(DoGoUpTray(1))
            {
                bRearReturnInProgress=false;   //AI(ht160s-trayarm-empty-handoff) 20260701 : rear-return finished (tray back at front, rear cleared); lift the pick block
                if(bReturnTray && bTrayXToEmptyFinish==false)
                    return;
                bReturnTray=false;
                Task=1;
            }
            break;
    }
}
//---------------------------------------------------------------------------
bool TEmptyModule::DoFeedTray(int Flag)
{
    int Ret;

    if(Flag==0)
    {
        FeedTask=1;
        FeedClampSub=0;
        FeedDelay.Clear();
        //AI(HT160S-Maintainer) 20260623 : one-shot Reset clears stale Push() Task
        //  left by an aborted feed (alarm/skip). Do NOT Reset before each Push()
        //  poll -- that restarts the non-blocking state machine and hangs the step.
        HSys.Cyn.C_Empty_LeanOnTray.Reset();
        HSys.Cyn.C_Empty_PushTray.Reset();
        return true;
    }

    switch(FeedTask)
    {
        case 1:
            FeedTask=10;
            break;

        case 10:
            RefreshStateFromSensors();
            if(bRearHasTray)
                return true;
            FeedTask=1000;
            break;

        case 1000:
            if(MoveEmptyY(Teach.EmptyCarFeedTrayYPosition))
                FeedTask=2000;
            break;

        case 2000:
        {
            //AI(HT160S-Maintainer) 20260623 : standardized dual-cylinder clamp via
            //DoClampTray (lean-stop first, push last, settle+confirm). Alarm text
            //stays here so the display is module-specific.
            int Clamp=DoClampTray(HSys.Cyn.C_Empty_LeanOnTray, HSys.Cyn.C_Empty_PushTray,
                                  FeedClampSub, FeedDelay, IsSoftSimulate(), GeneralSetting.iEmptyFeedClampSettleMs);
            if(Clamp==1)
                FeedTask=4000;
            else if(Clamp==2)
            {
                Ret=ShowMyError("JAM1030", LangT("Empty Push Tray Miss"), &HSys.Cyn.C_Empty_PushTray.OnSensor, true, K_RETRY);
                if(Ret==K_RETRY)
                    FeedTask=1000;
            }
            break;
        }

        case 4000:
            if(MoveEmptyY(Teach.EmptyCarDischargeTrayYPosition))
            {
                bFrontHasTray=false;
                FeedTask=5000;
            }
            break;

        case 5000:
            if(HSys.Cyn.C_Empty_PushTray.Pop() || IsSoftSimulate())
                FeedTask=6000;
            break;

        case 6000:
            if(HSys.Cyn.C_Empty_LeanOnTray.Pop() || IsSoftSimulate())
                FeedTask=7000;
            break;

        case 7000:
            if(HSys.Sen.SnEmpty_OutputBottomHasTray.Enable==true &&
               HSys.Sen.SnEmpty_OutputBottomHasTray.IsOff() &&
               HSys.LastSet.iRealDummy!=DUMMY)
            {
                Ret=ShowMyError("MES1021", LangT("Bottom Empty Tray Is Miss Error"), &HSys.Sen.SnEmpty_OutputBottomHasTray, true, K_SKIP|K_RETRY);
                if(Ret==K_RETRY)
                    FeedTask=1;
                if(Ret==K_SKIP)
                {
                    bRearHasTray=false;
                    bFrontHasTray=false;
                    FeedTask=13000;
                }
            }
            else
            {
                bRearHasTray=true;
                if(HSys.VMot.MMEmptyY!=NULL)
                {
                    HSys.VMot.MMEmptyY->Tray.MoveFrom(FrontSourceTray);   //AI(ht160s-tray-source) : hand off front-born grid to rear motor (rule #1)
                    HSys.VMot.MMEmptyY->fHasTray=true;
                    HSys.VMot.MMEmptyY->Refresh();
                }
                FeedTask=13000;
            }
            break;

        case 13000:
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(ht160s-tray-source) : create the rear empty tray's per-cell grid at the 'has tray'
//latch. EMPTY_IC content, Kind=Normal, no TrayID. Hangs off the action latch (NOT a
//sensor read), so it advances in DUMMY/sim exactly like the bRearHasTray latch it follows.
void TEmptyModule::BirthRearTray()
{
    if(HSys.VMot.MMEmptyY!=NULL) HSys.VMot.MMEmptyY->InitNewTray(EMPTY_IC);   //AI(ht160s-tray-source) : born onto rear motor (grid shows + fHasTray)
}
//---------------------------------------------------------------------------
//AI(ht160s-tray-source) : born at DoGoDownTray front confirm (rule #1). Same EMPTY_IC/Normal
//content as the rear; handed off to MEmptyY->Tray when DoFeedTray moves front->rear.
void TEmptyModule::BirthFrontTray()
{
    FrontSourceTray.Birth(EMPTY_IC, eTrayKindNormal, "");
}
//---------------------------------------------------------------------------
//AI(ht160s-tray-source) : return-by-value deep copy of the rear tray grid for TrayArm.
TMyTray TEmptyModule::GetSourceTray()
{
    if(HSys.VMot.MMEmptyY!=NULL)
        return HSys.VMot.MMEmptyY->Tray;
    TMyTray empty;
    return empty;
}
//---------------------------------------------------------------------------
bool TEmptyModule::PushCylinder(TMyCylinder &Cyn)
{
    if(IsSoftSimulate())  return true;
    if(Cyn.Enable==false) return true;
    return Cyn.Push();
}
//---------------------------------------------------------------------------
bool TEmptyModule::PopCylinder(TMyCylinder &Cyn)
{
    if(IsSoftSimulate())  return true;
    if(Cyn.Enable==false) return true;
    return Cyn.Pop();
}
//---------------------------------------------------------------------------
bool TEmptyModule::DoGoDownTray(int Flag)
{
    int Ret;

    if(Flag==0)
    {
        GoDownTask=1;
        GoDownDelay.Clear();
        return true;
    }

    switch(GoDownTask)
    {
        case 1:
            GoDownTask=10;
            break;

        case 10:
            //AI(ht160s-color-align-empty) 20260627 : align to TColorModule::DoGoDownTray --
            //skip the destack if a front tray is already staged (idempotent re-entry).
            RefreshStateFromSensors();
            if(bFrontHasTray)
                return true;
            GoDownTask=100;
            break;

        case 100:
            //AI(ht160s-color-align-empty) 20260627 : dual destacker via PushCylinder/PopCylinder
            //(sim + Enable aware, alarm-on-timeout), replacing the old .On()/.IsOn()||sim ladder
            //that never timed out and hung in DUMMY. Same physical order as before.
            if(PushCylinder(HSys.Cyn.C_Empty_FrontRiseTray_1))
                GoDownTask=150;
            break;

        case 150:
            if(PushCylinder(HSys.Cyn.C_Empty_FrontRiseTray_2))
                GoDownTask=200;
            break;

        case 200:
            //PRESERVED: Empty<->Loader front-separate interlock (Color is not in this pair).
            if(IsFrontSeparateBlockedBy(HSys.Cyn.C_Loader_FrontSeparateTray_1))
                break;   // interlock: wait while Loader front-separate is out
            if(PushCylinder(HSys.Cyn.C_Empty_FrontSeparateTray_1))
            {
                GoDownDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                GoDownDelay.On();
                GoDownTask=300;
            }
            break;

        case 300:
            //AI(HT160S-Maintainer) 20260701 : gate on PopCylinder's own confirm (sensor +
            //alarm/timeout via TMyCylinder::Pop) instead of discarding its return and relying
            //solely on the fixed settle timer below. The old code advanced to case 350/400
            //(separation claw release) even if FrontRiseTray_2 never physically confirmed down --
            //a slow/marginal cylinder let the claw open before the stack was actually caught,
            //dropping the whole stack (reported noise on Empty godown).
            if(GoDownDelay.Off())
            {
                if(PopCylinder(HSys.Cyn.C_Empty_FrontRiseTray_2))
                {
                    GoDownDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                    GoDownDelay.On();
                    GoDownTask=350;
                }
            }
            break;

        case 350:
            if(GoDownDelay.Off())
                GoDownTask=400;
            break;

        case 400:
            if(PopCylinder(HSys.Cyn.C_Empty_FrontSeparateTray_1))
            {
                GoDownDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                GoDownDelay.On();
                GoDownTask=450;
            }
            break;

        case 450:
            if(GoDownDelay.Off())
                GoDownTask=500;
            break;

        case 500:
            if(PopCylinder(HSys.Cyn.C_Empty_FrontRiseTray_1))
            {
                GoDownDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                GoDownDelay.On();
                GoDownTask=600;
            }
            break;

        case 600:
            if(GoDownDelay.Off())
                GoDownTask=700;
            break;

        case 700:
            if(HSys.Sen.SnEmpty_InputHasTray.Enable==true &&
               HSys.Sen.SnEmpty_InputHasTray.IsOff() &&
               HSys.LastSet.iRealDummy!=DUMMY)
            {
                bFrontHasTray=false;
                Ret=ShowMyError("MES1024", LangT("Front Empty Tray Is Miss Error"), &HSys.Sen.SnEmpty_InputHasTray, true, K_RETRY);
                if(Ret==K_RETRY)
                    GoDownTask=1;
            }
            else
            {
                bFrontHasTray=true;
                BirthFrontTray();   //AI(ht160s-tray-source) : born at front confirm (rule #1)
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TEmptyModule::DoGoUpTray(int Flag)
{
    if(Flag==0)
    {
        bRearReturnInProgress=true;   //AI(ht160s-trayarm-empty-handoff) 20260701 : rear tray about to be re-clamped and hauled back to front; block TrayArm pick until this return completes
        GoUpTask=1;
        GoUpDelay.Clear();
        return true;
    }

    switch(GoUpTask)
    {
        case 1:
            GoUpTask=10;
            break;

        case 10:
            GoUpTask=100;
            break;

        case 100:
            HSys.Cyn.C_Empty_FrontRiseTray_1.On();
            GoUpTask=200;
            break;

        case 200:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                if(IsFrontSeparateBlockedBy(HSys.Cyn.C_Loader_FrontSeparateTray_1))
                    break;   // interlock: wait while Loader front-separate is out
                HSys.Cyn.C_Empty_FrontSeparateTray_1.On();
                GoUpDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                GoUpDelay.On();
                GoUpTask=300;
            }
            break;

        case 300:
            if(GoUpDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.On();
                GoUpTask=400;
            }
            break;

        case 400:
            if(HSys.Cyn.C_Empty_FrontRiseTray_2.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Empty_FrontSeparateTray_1.Off();
                GoUpDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                GoUpDelay.On();
                GoUpTask=500;
            }
            break;

        case 500:
            if(GoUpDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.Off();
                if(HSys.Cyn.C_Empty_FrontRiseTray_1.IsOn() || IsSoftSimulate())
                    GoUpTask=600;
            }
            break;

        case 600:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.Pop() || IsSoftSimulate())
            {
                bFrontHasTray=false;
                FrontSourceTray.Clear();   //AI(ht160s-tray-source) : front carrier pushed back to stack
                GoUpTask=1000;
            }
            break;

        case 1000:
            RefreshStateFromSensors();
            if(bRearHasTray)
                GoUpTask=2000;
            else
                GoUpTask=9000;
            break;

        case 2000:
            if(MoveEmptyY(Teach.EmptyCarDischargeTrayYPosition))
                GoUpTask=3000;
            break;

        case 3000:
            if(HSys.Cyn.C_Empty_LeanOnTray.Push() || IsSoftSimulate())
                GoUpTask=4000;
            break;

        case 4000:
            if(HSys.Cyn.C_Empty_PushTray.Push() || IsSoftSimulate())
                GoUpTask=5000;
            break;

        case 5000:
            if(MoveEmptyY(Teach.EmptyCarFeedTrayYPosition))
                GoUpTask=6000;
            break;

        case 6000:
            if(HSys.Cyn.C_Empty_PushTray.Pop() || IsSoftSimulate())
                GoUpTask=7000;
            break;

        case 7000:
            if(HSys.Cyn.C_Empty_LeanOnTray.Pop() || IsSoftSimulate())
            {
                bFrontHasTray=true;
                bRearHasTray=false;
                if(HSys.VMot.MMEmptyY!=NULL)
                {
                    FrontSourceTray.CopyFrom(HSys.VMot.MMEmptyY->Tray);   //AI(ht160s-tray-source) : rear motor tray pulled back to front holder
                    HSys.VMot.MMEmptyY->ClearTray();
                }
                GoUpTask=8000;
            }
            break;

        case 8000:
            GoUpTask=9000;
            break;

        case 9000:
            GoUpTask=10000;
            break;

        case 10000:
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(general) 20260617 : Teach Advanced destacker test. Cylinder-only versions of
//GoDown/GoUp (no Y-motor / push / lean) so the front destacker rise+separate can be
//exercised in isolation. GoDown mirrors DoGoDownTray; GoUp mirrors DoGoUpTray 100-600.
bool TEmptyModule::TestGoDownTray(int Flag)
{
    if(Flag==0)
    {
        TestDownTask=1;
        TestDelay.Clear();
        return true;
    }

    switch(TestDownTask)
    {
        case 1:
            HSys.Cyn.C_Empty_FrontRiseTray_1.On();
            TestDownTask=2000;
            break;

        case 2000:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.On();
                TestDownTask=3000;
            }
            break;

        case 3000:
            if(HSys.Cyn.C_Empty_FrontRiseTray_2.IsOn() || IsSoftSimulate())
            {
                if(IsFrontSeparateBlockedBy(HSys.Cyn.C_Loader_FrontSeparateTray_1))
                    break;   // interlock: wait while Loader front-separate is out
                HSys.Cyn.C_Empty_FrontSeparateTray_1.On();
                TestDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                TestDelay.On();
                TestDownTask=4000;
            }
            break;

        case 4000:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.Off();
                TestDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                TestDelay.On();
                TestDownTask=4100;
            }
            break;

        case 4100:
            if(TestDelay.Off())
                TestDownTask=5000;
            break;

        case 5000:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Empty_FrontSeparateTray_1.Off();
                TestDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                TestDelay.On();
                TestDownTask=6000;
            }
            break;

        case 6000:
            if(TestDelay.Off())
                TestDownTask=6500;
            break;

        case 6500:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.Pop() || IsSoftSimulate())
            {
                TestDownTask=1;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TEmptyModule::TestGoUpTray(int Flag)
{
    if(Flag==0)
    {
        TestUpTask=1;
        TestDelay.Clear();
        return true;
    }

    switch(TestUpTask)
    {
        case 1:
            HSys.Cyn.C_Empty_FrontRiseTray_1.On();
            TestUpTask=200;
            break;

        case 200:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                if(IsFrontSeparateBlockedBy(HSys.Cyn.C_Loader_FrontSeparateTray_1))
                    break;   // interlock: wait while Loader front-separate is out
                HSys.Cyn.C_Empty_FrontSeparateTray_1.On();
                TestDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                TestDelay.On();
                TestUpTask=300;
            }
            break;

        case 300:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.On();
                TestUpTask=400;
            }
            break;

        case 400:
            if(HSys.Cyn.C_Empty_FrontRiseTray_2.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Empty_FrontSeparateTray_1.Off();
                TestDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                TestDelay.On();
                TestUpTask=500;
            }
            break;

        case 500:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.Off();
                if(HSys.Cyn.C_Empty_FrontRiseTray_1.IsOn() || IsSoftSimulate())
                    TestUpTask=600;
            }
            break;

        case 600:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.Pop() || IsSoftSimulate())
            {
                TestUpTask=1;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TEmptyModule::IsFrontHasTray()
{
    RefreshStateFromSensors();
    return bFrontHasTray;
}
//---------------------------------------------------------------------------
bool TEmptyModule::IsRearHasTray()
{
    RefreshStateFromSensors();
    return bRearHasTray;
}
//---------------------------------------------------------------------------
bool TEmptyModule::IsCleanOutFinish()
{
    //AI(cleanout) 20260701 : Empty participates in CleanOut (was: no participation). It finishes
    //only after TrayArm has finished (so no more trays are recycled back to it), its flow path is
    //clear (no front/rear tray) and the front rise/separate cylinders are all home
    //(IsReadyForAmrHandoff checks exactly those out-bits; sim-true). Until then DoEmpty case 100
    //GoUp-drains every tray back to the car. Not consulted outside CleanOut -> return true there.
    if(HSys.Sys.RunMode!=Run_CleanOut)
        return true;
    if(TrayArmModule==NULL || TrayArmModule->IsCleanOutFinish()==false)
        return false;
    RefreshStateFromSensors();
    if(bFrontHasTray || bRearHasTray)
        return false;
    if(IsReadyForAmrHandoff()==false)
        return false;
    //AI(cleanout) 20260703 : idle gate. Do NOT report finished while a drain sub-ladder is still
    //stepping (Feed/GoDown/GoUp mid-flight) or a rear return is in progress, and only once the
    //software rear grid is cleared. Without this the predicate could go true the instant the
    //bFront/bRearHasTray latches read false while a GoUp was still running, so TrayArm's
    //drain-boundary divert saw Empty "finished" and its in-hand tray stranded (the reported
    //"TrayArm stops halfway" symptom).
    if(FeedTask!=1 || GoDownTask!=1 || GoUpTask!=1)
        return false;
    if(bReturnTray || bRearReturnInProgress)
        return false;
    if(HSys.VMot.MMEmptyY->fHasTray)
        return false;
    //AI(cleanout) 20260703 : Full gate - do not report finished while the supply stack is
    //full (a drain GoUp is still owed but paused for the operator to empty). Sim-false.
    if(IsOutputCarFullForAmr())
        return false;
    return true;
}
//---------------------------------------------------------------------------
//AI(ht160s-trayarm-empty-handoff) 20260701 : "safe for TrayArm to grab" predicate.
//IsRearHasTray() only means "a tray exists at the rear" -- ALSO true during the return-to-front
//DoGoUpTray window (the carrier re-clamps the rear tray and hauls it back).
//AI(cleanout) 20260703 : ON-MACHINE FAILURE FIX (verified interference at KYEC). The 20260701
//latch-only version reported ready too early : RefreshStateFromSensors follows the RAW rear
//sensor, which lights the moment the carrier ARRIVES at the discharge position (DoFeedTray
//case 4000) - while the transport clamps are still engaged and the carrier still owns the
//tray (cases 5000/6000 not yet run). TrayArm dove in during that window and collided. Sim
//never showed it (RefreshStateFromSensors early-outs; the latch is set at the correct step,
//case 7000). Fix = state-based readiness, three layers, no magic encoder numbers :
//  1) tray present and not being returned (existing latches),
//  2) the feed ladder is NOT mid-handoff (FeedTask parked at 1=idle or 13000=done),
//  3) both transport clamps are physically RELEASED (out-bits off).
//This replaces the on-site emptypos>70000 encoder workaround with the same protection
//expressed in states + cylinder confirmation.
bool TEmptyModule::IsRearReadyForPick()
{
    RefreshStateFromSensors();
    if(bRearHasTray==false || bRearReturnInProgress)
        return false;
    if(FeedTask!=1 && FeedTask!=13000)
        return false;   //feed ladder mid-handoff : carrier still delivering the rear tray
    if(HSys.Cyn.C_Empty_LeanOnTray.GetOutBit() || HSys.Cyn.C_Empty_PushTray.GetOutBit())
        return false;   //transport clamps still actuated : the carrier still owns the tray
    return true;
}
//---------------------------------------------------------------------------
bool TEmptyModule::IsReturnTrayRequested()
{
    return bReturnTray;
}
//---------------------------------------------------------------------------
void TEmptyModule::SetRearHasTray(bool bHasTray)
{
    bRearHasTray=bHasTray;
    if(bHasTray==false)
    {
        if(HSys.VMot.MMEmptyY!=NULL)
            HSys.VMot.MMEmptyY->ClearTray();   //AI(ht160s-tray-source) : TrayArm took rear tray -> clear motor grid + hide
    }
}
//---------------------------------------------------------------------------
void TEmptyModule::RequestReturnTray()
{
    bReturnTray=true;
    bTrayXToEmptyFinish=false;
}
//---------------------------------------------------------------------------
//AI(ht160s-divert) 20260703 : TrayArm diverted an in-flight Empty return to an Auto (mid-
//flight divert). Drop the return reservation so DoEmpty case 3000 stops parking for a
//deposit that will never come (its wait is bReturnTray && !bTrayXToEmptyFinish) and the
//case-100 destack/feed branches un-skip. bRearReturnInProgress is NOT touched : it is
//owned by DoGoUpTray and self-clears when that ladder completes.
void TEmptyModule::CancelReturnTray()
{
    bReturnTray=false;
}
//---------------------------------------------------------------------------
void TEmptyModule::NotifyTrayXToEmptyFinish()
{
    bTrayXToEmptyFinish=true;
    bRearHasTray=true;
}
//---------------------------------------------------------------------------
AnsiString TEmptyModule::DescribeState()
{
    //AI(ht160s-state-record-analysis) 20260622 : read-only inner-state dump for
    //FeederDecision.txt (latched members only; no RefreshStateFromSensors).
    AnsiString s;
    s  = "[Empty]\r\n";
    s += "  bFrontHasTray=" + IntToStr(bFrontHasTray ? 1 : 0)
       + "  bRearHasTray=" + IntToStr(bRearHasTray ? 1 : 0) + "\r\n";
    s += "  bReturnTray=" + IntToStr(bReturnTray ? 1 : 0)
       + "  bTrayXToEmptyFinish=" + IntToStr(bTrayXToEmptyFinish ? 1 : 0)
       + "  bLotFinish=" + IntToStr(bLotFinish ? 1 : 0)
       //AI(ht160s-state-record-analysis) 20260625 : AMR lock freezes DoEmpty case100 (no rear-tray feed) -> tray-supply-starve deadlock
       + "  bAmrLocked=" + IntToStr(bAmrLocked ? 1 : 0)
       //AI(ht160s-agv) source-dry AMR wait : latch armed + whether the wait timer expired
       + "  bWaitingAmrFeed=" + IntToStr(bWaitingAmrFeed ? 1 : 0)
       + "  AmrFeedWaitExpired=" + IntToStr(AmrFeedWaitTimer.Off() ? 1 : 0)
       + "  SoftSim=" + IntToStr(IsSoftSimulate() ? 1 : 0) + "\r\n";
    s += "  FeedTask=" + IntToStr(FeedTask)
       + "  GoDownTask=" + IntToStr(GoDownTask)
       + "  GoUpTask=" + IntToStr(GoUpTask) + "\r\n";
    //AI(ht160s-agv) 20260625 : expose the CEID273 READY gate so a State Record shows
    //whether the AMR handoff can ever clear the lock (ReadyForAmr) and the raw front
    //destack cylinder out-bits that gate it (real-machine path).
    s += "  ReadyForAmr=" + IntToStr(IsReadyForAmrHandoff() ? 1 : 0)
       + "  FrontRise1Out=" + IntToStr(HSys.Cyn.C_Empty_FrontRiseTray_1.GetOutBit() ? 1 : 0)
       + "  FrontRise2Out=" + IntToStr(HSys.Cyn.C_Empty_FrontRiseTray_2.GetOutBit() ? 1 : 0)
       + "  FrontSep1Out=" + IntToStr(HSys.Cyn.C_Empty_FrontSeparateTray_1.GetOutBit() ? 1 : 0) + "\r\n";
    return s;
}
//---------------------------------------------------------------------------
void InitializeEmptyModule()
{
    if(EmptyModule==NULL)
        EmptyModule=new TEmptyModule;
}
//---------------------------------------------------------------------------
void ShutdownEmptyModule()
{
    delete EmptyModule;
    EmptyModule=NULL;
}
//---------------------------------------------------------------------------