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
//AI(ht160s-empty-place-handshake) 20260730 : how close the EmptyY carrier encoder must be to a
//taught stop to count as PARKED (1/100mm). MoveTo's own arrival window is +/-5 card pulses,
//which at M03's GearRatio 0.9 is under 0.06mm, so 0.50mm is a generous band that still excludes
//every mid-travel reading between the two stops (feed 1.00mm / discharge 836.00mm).
static const int EMPTY_CARRIER_PARK_TOL=50;
//---------------------------------------------------------------------------
TEmptyModule::TEmptyModule()
{
    InitialFlag();
}
//---------------------------------------------------------------------------
void TEmptyModule::InitialFlag(bool bKeepMaterial)
{
    bAmrLocked=false;
    bWaitingAmrFeed=false;       //AI(ht160s-agv) Empty source-dry AMR wait latch
    AmrFeedWaitTimer.Clear();   //AI(ht160s-agv) Empty source-dry AMR wait timer
    RefillSimInfeed();
    Status=ES_IDLE;   //AI(ht160s-status) 20260703 : ladder-owned status reset
    FeedTask=1;
    FeedClampSub=0;
    GoDownTask=1;
    GoUpTask=1;
    bFrontHasTray=false;
    bRearHasTray=false;
    //AI(ht160s-home-resume-w1) 20260711 : keep-material HOME preserves the return
    //handshake (the sender TrayArm keeps Job/PlaceDest and re-signs on resume, but
    //keeping the receiver side too removes the one-scan race where a wiped bReturnTray
    //let the feed branch grab the rear first). bRearReturnInProgress is NOT kept :
    //a kept bReturnTray re-dispatches DoGoUpTray whose Flag==0 re-arms it, while a
    //stale true with no ladder running would block TrayArm picks forever.
    if(bKeepMaterial==false)
    {
        bReturnTray=false;
        bTrayXToEmptyFinish=false;
    }
    if(bKeepMaterial)
        RecordProcess("HOME-RESUME Empty: kept returnTray="+IntToStr(bReturnTray?1:0)+
            " xferFinish="+IntToStr(bTrayXToEmptyFinish?1:0));   //AI(ht160s-obsv-p0)
    bRearReturnInProgress=false;   //AI(ht160s-trayarm-empty-handoff) 20260701 : no rear-return in flight at init
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
//---------------------------------------------------------------------------
//AI(ht160s-home-resume-w3b2) 20260711 : HOME PARK haul-target for this carriage.
//Called by uHome at the case-1 PARK snapshot (module Task cursors are still intact
//there -- the InitialAllTask wipe only runs after ProcessMotorHome returns true).
//A mid-haul park must FINISH the interrupted transfer at re-acquire : re-clamping at
//a mid-rail position and resuming blind would let the restarted ladder stage a second
//tray and collide (EF-2). Direction from the ladder cursors; the clamp out-bits are
//the actual carry gate (checked on the uHome side).
//DoGoUpTray phase B hauls rear->front (target = feed Y); DoFeedTray hauls
//front->rear (target = discharge Y). Completion releases at the destination with
//the module's own order (Pop PushTray then Pop LeanOnTray); the rear sensor edge
//then re-births the tray on resume (zero-memory convergence).
int TEmptyModule::GetHomeHaulTargetY()
{
    if(GoUpTask>=3000 && GoUpTask<8000)
        return Teach.EmptyCarFeedTrayYPosition;
    if(FeedTask>=2000 && FeedTask<7000)
        return Teach.EmptyCarDischargeTrayYPosition;
    return -1;
}
//---------------------------------------------------------------------------
//AI(ht160s-home-resume-drain) 20260711 : W2 drain hook. Pump ONLY the pure-cylinder
//destack phases to their boundary (GoDown 100-600 -> stop at the 700 sensor/commit
//entry, EG-3; GoUp phase A 100-600 -> its own 600 commit lands at 1000 which is NOT
//pumped - 1000 decides into the motor haul). Idle/decision states (1/10) are never
//pumped (pumping them would START a new destack). The clamp/haul segments need no
//drain : PARK + haul-finish covers them. Cylinder reed timeouts can still raise the
//ladder's own alarm - accepted deviation, a stuck cylinder is a hardware fault.
bool TEmptyModule::HomeDrainTick()
{
    bool bDone=true;
    if(GoDownTask>=100 && GoDownTask<=600)
    {
        DoGoDownTray(1);
        if(GoDownTask>=100 && GoDownTask<=600)
            bDone=false;
    }
    if(GoUpTask>=100 && GoUpTask<=600)
    {
        DoGoUpTray(1);
        if(GoUpTask>=100 && GoUpTask<=600)
            bDone=false;
    }
    return bDone;
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
//the SystemStart pause/resume edges, alongside HSys.CynPtr[]/SortArmSuck (mirrors Color).
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

    //AI(ht160s-empty-place-handshake) 20260730 : the legacy cross-module TrayArm interlock was
    //REMOVED here (user decision). It read MTrayArmX's encoder plus C_TrayArmZ_Up and refused the
    //move while the head was not confirmed up and (TrayArmPos+500)>=TrayXArmToEmptyXPosition.
    //Three defects made it a liability rather than a protection:
    //  1) SILENT and unbounded - a bare return false, no alarm, no EventLog, no timeout, so a head
    //     left down parked the Empty ladder forever (the on-site "moved only one step").
    //  2) One-sided >= with no upper bound - a head down at ANY taught column (Loader 222.00 up to
    //     Color 1730.82) froze Empty Y although none of those can reach the Empty corridor.
    //  3) Blind at the one place that matters - MTrayArmX homes at 0 and the trip line was 11.37mm,
    //     so a Z-up loss at the HOME park (only 16.37mm from the Empty column) was never caught.
    //Protection now lives on the side that actually lowers into the shared space : TrayArm must pass
    //IsRearReadyForPlace() (clamps released + neither ladder mid-handoff + carrier parked) before it
    //goes down. Do NOT reintroduce a hardware peek here - extend that handshake predicate instead.

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
                Status=ES_RETURNING;   //AI(ht160s-status) 20260703
                DoGoUpTray(0);
                Task=3000;
                break;
            }

            if(bFrontHasTray==false && bLotFinish==false)
            {
                if(bAmrLocked)
                    break;   //AI(ht160s-agv) front GoDown suspended during AMR handoff
                Status=ES_DESTACK;   //AI(ht160s-status) 20260703
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
                //AI(amr-unmanned batch3) 20260721 : real AMR unmanned line -> the coordinator P2
                //supply CALL (CEID272) + WAR0962-on-timeout own Empty source-dry (align with Color,
                //which has NO supply-empty alarm). No operator MES1022 modal / no in-ladder wait.
                if(GeneralSetting.bUseAMR && IsSoftSimulate()==false)
                    break;
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
                Status=ES_FEEDING;   //AI(ht160s-status) 20260703 : carrier will own the rear
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
                //AI(amr-unmanned batch2) 20260721 : supply-stack FULL = "stop bringing", not
                //"collect". AMR (unmanned) : GoUp anyway (mechanically safe, user ruling), no
                //modal, no AGV call - drains as next production consumes it. non-AMR keeps modal.
                if(IsOutputCarFullForAmr() && GeneralSetting.bUseAMR==false)
                {
                    do
                    {
                        ShowMyError("MES1023", "Empty supply stack FULL (sensor) - remove stacked trays", &HSys.Sen.SnEmpty_InputFullTray, false, K_RETRY);
                    }
                    while(HSys.Sen.SnEmpty_InputFullTray.Enable==true && HSys.Sen.SnEmpty_InputFullTray.IsOn());
                }
                Status=ES_RETURNING;   //AI(ht160s-status) 20260703 : CleanOut drain GoUp
                DoGoUpTray(0);
                Task=3000;
            }
            break;

        case 1000:
            if(DoGoDownTray(1))
            {
                if(IsSoftSimulate() && iSimInfeedCount>0)
                    iSimInfeedCount--;   //AI(ht160s-agv) sim input drains 1/GoDown
                Status=(bRearHasTray ? ES_REAR_READY : ES_IDLE);   //AI(ht160s-status) 20260703
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
                //AI(ht160s-empty-return-latch) 20260731 : do NOT drop the interlock while the
                //return is still OWED. This park restarts the second haul via DoGoUpTray(1)
                //from the idle terminal, and ONLY DoGoUpTray(Flag==0) re-arms this flag, so
                //clearing it here left the whole second leg unprotected -- while
                //NotifyTrayXToEmptyFinish had already overwritten ES_RETURNING -> ES_REAR_READY.
                //TrayArm then passed IsRearReadyForPick and clamped the very tray the carrier
                //was re-clamping (onsite 20260730 16:58, alarm 600 on C_Empty_PushTray).
                //Safe to hold: the only readers are IsCleanOutFinish (already blocked by
                //bReturnTray on the same line) and ComputeRearPickReadyNoRefresh, which is
                //called ONLY from the TrayArm pick gate -- no Empty ladder reads it, and the
                //deposit this park waits for is gated by IsRearReadyForPlace, not by the pick
                //predicate. So there is no self-block.
                if(bReturnTray && bTrayXToEmptyFinish==false)
                    return;            //still owed the second leg : LEAVE the latch true
                bRearReturnInProgress=false;   //AI(ht160s-trayarm-empty-handoff) 20260701 : rear-return finished (tray back at front, rear cleared); lift the pick block
                //AI(ht160s-home-resume-cg4) 20260713 : Empty mirror of the Color CG-4
                //close-out blind-spot guard. NotifyTrayXToEmptyFinish sets bRearHasTray +
                //bTrayXToEmptyFinish together, but a late Notify can commit with the tray
                //still at rear (case 1000 already skipped the haul). On real hardware case
                //1000's RefreshStateFromSensors self-corrects; the sim / rear-sensor-disabled
                //config is where it strands and inflates the return count. Require the rear
                //cleared; else re-haul one more round.
                if(bReturnTray && bRearHasTray)
                {
                    DoGoUpTray(0);
                    return;
                }
                bReturnTray=false;
                Status=(bRearHasTray ? ES_REAR_READY : ES_IDLE);   //AI(ht160s-status) 20260703
                Task=1;
            }
            break;
        default:
            //AI(ht160s-ladder-guard) 20260703 : a state number with no matching case
            //(the 'number but no action' trap). Log it so a future dead-jump is a
            //diagnosable EventLog event, not a silent stall, and restart the ladder.
            LogLadderFault("Empty.DoEmpty", Task);
            Task=1;
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
            {
                FeedTask=13000;   //AI(ht160s-feeder-unify) 20260706 : idempotent re-entry -> single DONE terminal (case 13000)
                break;
            }
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
                    Status=ES_IDLE;   //AI(ht160s-status) 20260703 : feed aborted, nothing at rear
                    FeedTask=13000;
                }
            }
            else
            {
                bRearHasTray=true;
                Status=ES_REAR_READY;   //AI(ht160s-status) 20260703 : clamps popped (case5000/6000) + sensor confirmed
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
            //AI(ht160s-feeder-unify) 20260706 : single terminal - FeedTask returns to idle(1) HERE so
            //IsCleanOutFinish's "FeedTask==1" idle gate reads a completed feed as finished.
            FeedTask=1;
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
            {
                GoDownTask=99999;   //AI(ht160s-feeder-unify) 20260706 : idempotent re-entry -> single DONE terminal (Task returns to 1 there)
                break;
            }
            HSys.Cyn.C_Empty_FrontRiseTray_1.Reset();   //AI(ht160s-feeder-unify) 20260706 : one-shot re-arm before case 100 Push
            GoDownTask=100;
            break;

        case 100:
            //AI(ht160s-color-align-empty) 20260627 : dual destacker via PushCylinder/PopCylinder
            //(sim + Enable aware, alarm-on-timeout), replacing the old .On()/.IsOn()||sim ladder
            //that never timed out and hung in DUMMY. Same physical order as before.
            if(PushCylinder(HSys.Cyn.C_Empty_FrontRiseTray_1))
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.Reset();   //AI(ht160s-feeder-unify) 20260706 : one-shot re-arm before case 150 Push
                GoDownTask=150;
            }
            break;

        case 150:
            if(PushCylinder(HSys.Cyn.C_Empty_FrontRiseTray_2))
            {
                HSys.Cyn.C_Empty_FrontSeparateTray_1.Reset();   //AI(ht160s-feeder-unify) 20260706 : one-shot re-arm before case 200 Push
                GoDownTask=200;
            }
            break;

        case 200:
            //PRESERVED: Empty<->Loader front-separate interlock (Color is not in this pair).
            if(IsFrontSeparateBlockedBy(HSys.Cyn.C_Loader_FrontSeparateTray_1))
                break;   // interlock: wait while Loader front-separate is out
            if(PushCylinder(HSys.Cyn.C_Empty_FrontSeparateTray_1))
            {
                GoDownDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                GoDownDelay.On();
                HSys.Cyn.C_Empty_FrontRiseTray_2.Reset();   //AI(ht160s-feeder-unify) 20260706 : re-arm Rise_2 for its Pop at case 300
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
            {
                HSys.Cyn.C_Empty_FrontSeparateTray_1.Reset();   //AI(ht160s-feeder-unify) 20260706 : re-arm Separate for its Pop at case 400
                GoDownTask=400;
            }
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
            {
                HSys.Cyn.C_Empty_FrontRiseTray_1.Reset();   //AI(ht160s-feeder-unify) 20260706 : re-arm Rise_1 for its Pop at case 500
                GoDownTask=500;
            }
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
                GoDownTask=99999;   //AI(ht160s-feeder-unify) 20260706 : route to single DONE terminal
            }
            break;

        case 99999:
            //AI(ht160s-feeder-unify) 20260706 : single terminal - Task returns to idle(1) HERE so
            //IsCleanOutFinish's "GoDownTask==1" idle gate reads a completed drain as finished.
            GoDownTask=1;
            return true;
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
        //AI(ht160s-feeder-unify) 20260706 : GoUp converted from the old .On()/.IsOn()||sim and
        //.Pop()||sim idiom to PushCylinder/PopCylinder (sim + Enable aware, in-position confirm +
        //alarm-on-timeout), with a one-shot .Reset() re-arm before each poll and GoUpDelay settle -
        //mirrors DoGoDownTray so GoUp is no longer "too fast / not smooth". Physical order and the
        //Empty<->Loader front-separate interlock are unchanged.
        case 1:
            GoUpTask=10;
            break;

        case 10:
            //AI(ht160s-goup-hastray-gate) 20260708 : confirm a FRONT tray is present before the
            //raise. With an empty front rest the rise + Separate-pawl release (case 410) drops the
            //stack above by one pitch (loud bang). No front tray -> skip the front raise (case
            //100-600) and jump to rear handling (case 1000) so CleanOut drain and bReturnTray
            //rear-return still complete. Mirrors DoGoDownTray case 10; sensor SnEmpty_InputHasTray
            //-> bFrontHasTray. sim/dummy RefreshStateFromSensors early-returns and keeps the
            //caller-maintained latch, so sim is unaffected.
            RefreshStateFromSensors();
            if(bFrontHasTray==false)
            {
                GoUpTask=1000;   //no front tray -> rear-handling terminal only, no empty raise
                break;
            }
            GoUpTask=100;
            break;

        case 100:
            HSys.Cyn.C_Empty_FrontRiseTray_1.Reset();   //one-shot re-arm before Push
            GoUpTask=110;
            break;

        case 110:
            if(PushCylinder(HSys.Cyn.C_Empty_FrontRiseTray_1))   //extend Rise_1 (confirmed)
                GoUpTask=200;
            break;

        case 200:
            //PRESERVED: Empty<->Loader front-separate interlock.
            if(IsFrontSeparateBlockedBy(HSys.Cyn.C_Loader_FrontSeparateTray_1))
                break;   // interlock: wait while Loader front-separate is out
            HSys.Cyn.C_Empty_FrontSeparateTray_1.Reset();
            GoUpTask=210;
            break;

        case 210:
            if(PushCylinder(HSys.Cyn.C_Empty_FrontSeparateTray_1))   //extend Separate (confirmed)
            {
                GoUpDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                GoUpDelay.On();
                GoUpTask=300;
            }
            break;

        case 300:
            if(GoUpDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.Reset();
                GoUpTask=310;
            }
            break;

        case 310:
            if(PushCylinder(HSys.Cyn.C_Empty_FrontRiseTray_2))   //extend Rise_2 (confirmed)
                GoUpTask=400;
            break;

        case 400:
            HSys.Cyn.C_Empty_FrontSeparateTray_1.Reset();
            GoUpTask=410;
            break;

        case 410:
            if(PopCylinder(HSys.Cyn.C_Empty_FrontSeparateTray_1))   //retract Separate (confirmed)
            {
                GoUpDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                GoUpDelay.On();
                GoUpTask=500;
            }
            break;

        case 500:
            if(GoUpDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.Reset();
                GoUpTask=510;
            }
            break;

        case 510:
            if(PopCylinder(HSys.Cyn.C_Empty_FrontRiseTray_2))   //retract Rise_2 (confirmed)
            {
                HSys.Cyn.C_Empty_FrontRiseTray_1.Reset();
                GoUpTask=600;
            }
            break;

        case 600:
            if(PopCylinder(HSys.Cyn.C_Empty_FrontRiseTray_1))   //retract Rise_1 (confirmed)
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
            {
                HSys.Cyn.C_Empty_LeanOnTray.Reset();
                GoUpTask=3000;
            }
            break;

        case 3000:
            if(PushCylinder(HSys.Cyn.C_Empty_LeanOnTray))
            {
                HSys.Cyn.C_Empty_PushTray.Reset();
                GoUpTask=4000;
            }
            break;

        case 4000:
            if(PushCylinder(HSys.Cyn.C_Empty_PushTray))
                GoUpTask=5000;
            break;

        case 5000:
            if(MoveEmptyY(Teach.EmptyCarFeedTrayYPosition))
            {
                HSys.Cyn.C_Empty_PushTray.Reset();
                GoUpTask=6000;
            }
            break;

        case 6000:
            if(PopCylinder(HSys.Cyn.C_Empty_PushTray))
            {
                HSys.Cyn.C_Empty_LeanOnTray.Reset();
                GoUpTask=7000;
            }
            break;

        case 7000:
            if(PopCylinder(HSys.Cyn.C_Empty_LeanOnTray))
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
            //AI(ht160s-feeder-unify) 20260706 : single terminal - GoUpTask returns to idle(1) HERE so
            //IsCleanOutFinish's "GoUpTask==1" idle gate reads a completed GoUp drain as finished.
            GoUpTask=1;
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

    //AI(ht160s-feeder-unify) 20260706 : idiom aligned to production DoGoDownTray -
    //PushCylinder/PopCylinder (confirm + alarm) + one-shot .Reset() re-arm; settle profile
    //and the Empty<->Loader interlock unchanged. Teach-only (Advanced test tab).
    switch(TestDownTask)
    {
        case 1:
            HSys.Cyn.C_Empty_FrontRiseTray_1.Reset();
            TestDownTask=2000;
            break;

        case 2000:
            if(PushCylinder(HSys.Cyn.C_Empty_FrontRiseTray_1))
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.Reset();
                TestDownTask=3000;
            }
            break;

        case 3000:
            if(PushCylinder(HSys.Cyn.C_Empty_FrontRiseTray_2))
                TestDownTask=3500;
            break;

        case 3500:
            if(IsFrontSeparateBlockedBy(HSys.Cyn.C_Loader_FrontSeparateTray_1))
                break;   // interlock: wait while Loader front-separate is out
            HSys.Cyn.C_Empty_FrontSeparateTray_1.Reset();
            TestDownTask=3600;
            break;

        case 3600:
            if(PushCylinder(HSys.Cyn.C_Empty_FrontSeparateTray_1))
            {
                TestDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                TestDelay.On();
                HSys.Cyn.C_Empty_FrontRiseTray_2.Reset();
                TestDownTask=4000;
            }
            break;

        case 4000:
            if(TestDelay.Off())
            {
                if(PopCylinder(HSys.Cyn.C_Empty_FrontRiseTray_2))
                {
                    TestDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                    TestDelay.On();
                    TestDownTask=4100;
                }
            }
            break;

        case 4100:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontSeparateTray_1.Reset();
                TestDownTask=5000;
            }
            break;

        case 5000:
            if(PopCylinder(HSys.Cyn.C_Empty_FrontSeparateTray_1))
            {
                TestDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                TestDelay.On();
                HSys.Cyn.C_Empty_FrontRiseTray_1.Reset();
                TestDownTask=6000;
            }
            break;

        case 6000:
            if(TestDelay.Off())
                TestDownTask=6500;
            break;

        case 6500:
            if(PopCylinder(HSys.Cyn.C_Empty_FrontRiseTray_1))
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

    //AI(ht160s-feeder-unify) 20260706 : idiom aligned to production DoGoUpTray -
    //PushCylinder/PopCylinder (confirm + alarm) + one-shot .Reset() re-arm; settle profile
    //and the Empty<->Loader interlock unchanged. Teach-only (Advanced test tab).
    switch(TestUpTask)
    {
        case 1:
            HSys.Cyn.C_Empty_FrontRiseTray_1.Reset();
            TestUpTask=100;
            break;

        case 100:
            if(PushCylinder(HSys.Cyn.C_Empty_FrontRiseTray_1))
                TestUpTask=200;
            break;

        case 200:
            if(IsFrontSeparateBlockedBy(HSys.Cyn.C_Loader_FrontSeparateTray_1))
                break;   // interlock: wait while Loader front-separate is out
            HSys.Cyn.C_Empty_FrontSeparateTray_1.Reset();
            TestUpTask=210;
            break;

        case 210:
            if(PushCylinder(HSys.Cyn.C_Empty_FrontSeparateTray_1))
            {
                TestDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                TestDelay.On();
                TestUpTask=300;
            }
            break;

        case 300:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.Reset();
                TestUpTask=310;
            }
            break;

        case 310:
            if(PushCylinder(HSys.Cyn.C_Empty_FrontRiseTray_2))
                TestUpTask=400;
            break;

        case 400:
            HSys.Cyn.C_Empty_FrontSeparateTray_1.Reset();
            TestUpTask=410;
            break;

        case 410:
            if(PopCylinder(HSys.Cyn.C_Empty_FrontSeparateTray_1))
            {
                TestDelay.SetMS(GeneralSetting.iEmptyDestackSettleMs);
                TestDelay.On();
                TestUpTask=500;
            }
            break;

        case 500:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.Reset();
                TestUpTask=510;
            }
            break;

        case 510:
            if(PopCylinder(HSys.Cyn.C_Empty_FrontRiseTray_2))
            {
                HSys.Cyn.C_Empty_FrontRiseTray_1.Reset();
                TestUpTask=600;
            }
            break;

        case 600:
            if(PopCylinder(HSys.Cyn.C_Empty_FrontRiseTray_1))
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
    if(Status==ES_FEEDING || Status==ES_RETURNING || Status==ES_DESTACK)
        return false;   //AI(ht160s-status) 20260703 : status busy belt beside the cursor gate
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
//AI(ht160s-rearready-p0) 20260705 : the 4 readiness gates WITHOUT the sensor refresh --
//the single source of truth shared by IsRearReadyForPick() (which refreshes first) and
//DescribeState() (dump path must not refresh). Edit gates HERE ONLY : a hand-inlined
//copy in the dump already drifted from this predicate once this week -- never again.
bool TEmptyModule::ComputeRearPickReadyNoRefresh()
{
    //AI(ht160s-status) 20260703 : status BUSY gate. Deliberately negative-form (block while
    //a ladder actively owns the rear), NOT a positive Status==ES_REAR_READY requirement :
    //after a HOME the sensor re-latches a physically-present rear tray while InitialFlag
    //reset Status to ES_IDLE - requiring the positive state would strand that tray forever.
    //Positive readiness stays with the legacy 3 layers below (kept as safety belts).
    if(Status==ES_FEEDING || Status==ES_RETURNING)
        return false;
    if(bRearHasTray==false || bRearReturnInProgress)
        return false;
    if(FeedTask!=1 && FeedTask!=13000)
        return false;   //feed ladder mid-handoff : carrier still delivering the rear tray
    if(HSys.Cyn.C_Empty_LeanOnTray.GetOutBit() || HSys.Cyn.C_Empty_PushTray.GetOutBit())
        return false;   //transport clamps still actuated : the carrier still owns the tray
    return true;
}
//---------------------------------------------------------------------------
bool TEmptyModule::IsRearReadyForPick()
{
    RefreshStateFromSensors();
    return ComputeRearPickReadyNoRefresh();
}
//---------------------------------------------------------------------------
//AI(ht160s-empty-place-handshake) 20260730 : positive PARKED confirmation for the EmptyY carrier.
//The carrier is a two-stop shuttle, so an encoder anywhere other than a taught stop means it is
//mid-travel (or a move was aborted). The front/rear tray sensors are at FIXED lane positions and
//therefore cannot tell where the moving carrier is - that is exactly why a peer must not judge the
//station from the tray sensors alone. Pure read, no side effects.
bool TEmptyModule::IsCarrierParked()
{
    if(IsSoftSimulate())
        return true;
    if(HSys.Mot.MEmptyY==NULL)
        return false;
    int Pos=HSys.Mot.MEmptyY->ReadEncoderPos();
    int dFeed=Pos-Teach.EmptyCarFeedTrayYPosition;
    int dDisc=Pos-Teach.EmptyCarDischargeTrayYPosition;
    if(dFeed<0)
        dFeed=-dFeed;
    if(dDisc<0)
        dDisc=-dDisc;
    return (dFeed<=EMPTY_CARRIER_PARK_TOL || dDisc<=EMPTY_CARRIER_PARK_TOL);
}
//---------------------------------------------------------------------------
//AI(ht160s-empty-place-handshake) 20260730 : may a peer LOWER a head onto the Empty rear right now?
//The place-side counterpart of ComputeRearPickReadyNoRefresh, which has carried these gates since
//20260705 while the place side had only "rear sensor says empty" - strictly weaker, and blind to the
//two windows where the carrier is hauling a CLAMPED tray past the fixed rear sensor (DoFeedTray case
//4000 front->discharge, DoGoUpTray case 5000 discharge->front). In both the sensor reads OFF while a
//tray physically sits on the carrier, so a peer that trusted the sensor lowered onto that tray.
//The clamp out-bits are the decisive term : a tray riding the carrier is by definition a CLAMPED
//tray, whatever the carrier position or the fixed sensor says.
//Deliberately does NOT block on Status==ES_RETURNING : DoEmpty case 3000 parks with Status still
//ES_RETURNING while it waits for the peer's NotifyTrayXToEmptyFinish, so blocking on it would make
//both sides wait for each other forever. Motion is excluded by the ladder cursors + parked check.
//This asks only "is the rear safe to descend onto" - the caller still owns the rear-empty test.
bool TEmptyModule::IsRearReadyForPlace()
{
    if(Status==ES_FEEDING)
        return false;   //feed ladder owns the rear
    if(FeedTask!=1 && FeedTask!=13000)
        return false;   //feed ladder mid-handoff (idle=1, done=13000)
    if(GoUpTask!=1)
        return false;   //return/drain ladder mid-haul (single idle terminal is GoUpTask==1)
    if(HSys.Cyn.C_Empty_LeanOnTray.GetOutBit() || HSys.Cyn.C_Empty_PushTray.GetOutBit())
        return false;   //transport clamps still actuated : the carrier still owns a tray
    if(IsCarrierParked()==false)
        return false;   //carrier mid-travel : a moving carrier can still strike a lowered head
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
    if(bHasTray==false && Status==ES_REAR_READY)
        Status=ES_IDLE;   //AI(ht160s-status) 20260703 : TrayArm took the presented tray (only rectify the READY state, never a mid-ladder one)
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
    Status=ES_REAR_READY;   //AI(ht160s-status) 20260703 : returned tray parked at rear (re-enters supply pool); case-3000 completion re-confirms
}
//---------------------------------------------------------------------------
//AI(ht160s-status) 20260703 : eEmptyStatus display name (DescribeState / stbMain).
static const char *EmptyStatusName(int St)
{
    switch(St)
    {
        case ES_IDLE:       return "IDLE";
        case ES_DESTACK:    return "DESTACK";
        case ES_FEEDING:    return "FEEDING";
        case ES_REAR_READY: return "REAR_READY";
        case ES_RETURNING:  return "RETURNING";
    }
    return "?";
}
//---------------------------------------------------------------------------
int TEmptyModule::GetStatus()
{
    return Status;
}
//---------------------------------------------------------------------------
AnsiString TEmptyModule::DescribeState()
{
    //AI(ht160s-state-record-analysis) 20260622 : read-only inner-state dump for
    //FeederDecision.txt (latched members only; no RefreshStateFromSensors).
    AnsiString s;
    s  = "[Empty]\r\n";
    s += "  Status=" + AnsiString(EmptyStatusName(Status)) + "\r\n";
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
    //AI(ht160s-rearready-p0) 20260705 : rear-pick observability (report 5.2). Latched
    //members + commanded out-bits only -- do NOT call IsRearReadyForPick() here (it
    //refreshes sensors inside the dump path). RearPickReady comes from the SAME
    //no-refresh helper the predicate uses (single source of truth, no silent drift), so
    //a State Record can arbitrate "early pick" vs "stale block" and show WHICH gate
    //input disagreed (the diagnosability gap behind the onsite hand-revert).
    s += "  bRearReturnInProgress=" + IntToStr(bRearReturnInProgress ? 1 : 0)
       + "  RearLeanOut=" + IntToStr(HSys.Cyn.C_Empty_LeanOnTray.GetOutBit() ? 1 : 0)
       + "  RearPushOut=" + IntToStr(HSys.Cyn.C_Empty_PushTray.GetOutBit() ? 1 : 0)
       + "  RearPickReady=" + IntToStr(ComputeRearPickReadyNoRefresh() ? 1 : 0) + "\r\n";
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