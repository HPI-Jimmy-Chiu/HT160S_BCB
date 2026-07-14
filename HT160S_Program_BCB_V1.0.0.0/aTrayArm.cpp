//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop
#include "language.h"

#include "aTrayArm.h"
#include "database.h"
#include "uteach.h"
#include "mymessbox.h"
#include "aEmpty.h"
#include "aAuto1To6.h"
#include "aColor.h"            //AI(HT160S-Maintainer) 20260605 : AMR identity-tray source
#include "aLoader.h"           //AI(HT160S-Maintainer) 20260606 : Loader rear empty-tray recovery source
#include "aSortArm.h"          //AI(cleanout) 20260701 : SortArmModule->IsCleanOutFinish() = drain-boundary signal for the DoPlace in-flight divert
#include "GeneralSetting.h"    //AI(HT160S-Maintainer) 20260605 : GeneralSetting.bUseAMR mode switch
#include "cStateRecordHT160.h" //AI(ht160s-rearready-p0) 20260705 : gStateRecord->TriggerSnapshot on blocked-pick watchdog expiry
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TTrayArmModule *TrayArmModule=NULL;
//---------------------------------------------------------------------------
static const int TRAYARM_ZUP_LOST_MS=100;   //AI(HT160S-Maintainer) 20260622 : TrayArm X-move Z-up loss debounce window (ms); time-based, not cycle-count
static const int TRAYARM_PICK_GATE_SCAN_GAP_MS=1500;   //AI(ht160s-rearready-p0) 20260705 : blocked-poll continuity gap (mirrors ION_FAN_SCAN_GAP_MS) -- a larger gap means the wall-clock span was NOT blocked run-time (modal Note froze MainProc, IO Set View open, machine stopped) and must re-arm, not charge, the watchdog window
static const int TRAYARM_PICK_GATE_ALARM_MS=60000;   //AI(ht160s-rearready-p0) 20260705 : blocked-pick watchdog window (ms). A normal rear handoff (Empty feed / Loader discharge remainder) finishes well under 30s; a full silent minute at the DoPick 1/10 gate means the readiness latch is stuck -- notify (report P1), never wait silently
static const int TRAYARM_PLACE_GATE_ALARM_MS=60000;   //AI(ht160s-home-resume-p0) 20260710 : blocked-place watchdog window (ms). A receiver GoUp that frees the rear finishes well under 30s; a full silent minute at the DoPlaceToEmpty/Color case-500 rear-clear wait means the return handshake is torn (post-HOME wipe) or the rear is stuck occupied -- notify, never wait silently. Shares TRAYARM_PICK_GATE_SCAN_GAP_MS for poll continuity.
//---------------------------------------------------------------------------
TTrayArmModule::TTrayArmModule()
{
    Status=TAS_IDLE;
    Job=TAJOB_NONE;
    PickTask=1;
    PlaceTask=1;
    iAutoTarget=-1;
    iDeliverKind=eTrayKindNormal;
    PlaceDest=TAPLACE_AUTO;
    bCleanOutFinish=true;
    bResiduePendingNotify=false;
    InitialFlag();
}
//---------------------------------------------------------------------------
void TTrayArmModule::InitialFlag(bool bKeepMaterial)
{
    bHasTray=false;
    if(HSys.VMot.MMTrayArmX!=NULL)
        bHasTray=HSys.VMot.MMTrayArmX->fHasTray;
    //AI(ht160s-home-residue) 20260708 : C1 follow-up to 05bc5c9. HOME now keeps the clamps
    //CLOSED when the physical clamp On sensors read held while the fHasTray latch desynced
    //to false (alarm inside the pick window between the clamp-close at DoLowerClampRaise
    //case 2000 and the latch set at DoPick case 4000). Without this block InitialFlag would
    //declare the arm empty and DoTrayArm would dispatch a fresh pick that Z-downs CLOSED-
    //JAWED onto an occupied source rear (double-stack jam). Adopt the sensor-held tray as
    //RESIDUE instead : fHasTray=true so the existing DoTrayArm case-100 residue guard idles
    //the arm, and force the non-keep branch below (a mid-pick job has a stale destination /
    //uncopied grid, so it must NOT resume as a valid carry). Both-On = high-certainty held
    //(operator-confirmed 20260707); DUMMY tier skipped (no material by definition, same gate
    //as the DoLoader rear-leftover watchdog). Un-adopt lives in DoTrayArm case 100.
    bool bAdoptedResidue=false;
#ifndef SOFT_SIMULATE
    if(bHasTray==false &&
       HSys.LastSet.iRealDummy!=DUMMY &&
       HSys.Cyn.C_TrayArm_FrontClamp.OnSensor.Enable && HSys.Cyn.C_TrayArm_FrontClamp.OnSensor.IsOn() &&
       HSys.Cyn.C_TrayArm_RearClamp.OnSensor.Enable  && HSys.Cyn.C_TrayArm_RearClamp.OnSensor.IsOn())
    {
        bHasTray=true;
        if(HSys.VMot.MMTrayArmX!=NULL)
            HSys.VMot.MMTrayArmX->fHasTray=true;
        bAdoptedResidue=true;
        bResiduePendingNotify=true;
    }
#endif
    PickTask=1;
    PlaceTask=1;
    bCleanOutFinish=true;
    ArmDelay.Clear();
    PickWaitTimer.Clear();   //AI(ht160s-rearready-p0) 20260705 : per-wait transient; also resets a Paused timer (HTimer trap)
    bPickWaitArmed=false;
    dwPickGateLastPollTick=0;
    PlaceWaitTimer.Clear();   //AI(ht160s-home-resume-p0) 20260710 : place-gate watchdog is per-wait transient (mirrors the pick trio)
    bPlaceWaitArmed=false;
    dwPlaceGateLastPollTick=0;
    dwZUpLostStart=0;
    //AI(HT160S-Maintainer) 20260612 : recoverable home while a tray is in hand. Keep the
    //delivery job + destination (Auto target, AMR kind, 2D TrayID, place dest) so the arm
    //resumes placing the SAME tray after home instead of losing where it must go. The
    //clamps are kept closed during the home (see uHome ProcessMotorHome) so the tray rides
    //up with the head and is never dropped. Only the transient pick/place sub-tasks above
    //are restarted.
    if(bKeepMaterial && bHasTray && bAdoptedResidue==false)
    {
        Status=TAS_CARRYING;
        return;
    }
    Status=TAS_IDLE;
    Job=TAJOB_NONE;
    iAutoTarget=-1;
    iDeliverKind=eTrayKindNormal;
    iDeliverTrayID="";
    if(HSys.VMot.MMTrayArmX!=NULL) HSys.VMot.MMTrayArmX->Tray.Clear();   //AI(ht160s-tray-source) : drop carried grid when material is not kept across home
    PlaceDest=TAPLACE_AUTO;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::HasTray()
{
    if(HSys.VMot.MMTrayArmX!=NULL)
        bHasTray=HSys.VMot.MMTrayArmX->fHasTray;
    return bHasTray;
}
//---------------------------------------------------------------------------
int TTrayArmModule::GetStatus()
{
    return Status;
}
//---------------------------------------------------------------------------
//AI(ht160s-rearready-p0) 20260705 : sub-task readouts for MotionDetail.ini (SortArm
//GetPickTask precedent) -- the top-level DoTrayArm Task is only 1/10/100/1000/2000,
//so the pick-gate wait (PickTask 1/10) was invisible in a State Record.
int TTrayArmModule::GetPickTask()
{
    return PickTask;
}
//---------------------------------------------------------------------------
int TTrayArmModule::GetPlaceTask()
{
    return PlaceTask;
}
//---------------------------------------------------------------------------
int TTrayArmModule::GetJob()
{
    return Job;
}
//---------------------------------------------------------------------------
//AI(ht160s-home-resume-drain) 20260711 : W2 drain hook. Finish an in-flight grab or
//deposit (PickTask/PlaceTask >= 1000 = the pure cylinder+data ladder; the positioning
//and rear-clear waits < 1000 are deliberately NOT pumped - they resume via the
//keep-material path). Status discriminates which cursor is live (both linger at 4000
//after completion, so raw ranges would double-commit). D4 : the open-clamp handoff is
//allowed inside the drain because no motor has moved yet. After a drained grab the
//Loader-recovery job still needs its destination decision (normally DoTrayArm case
//1000) - do it here so the resume heal re-signs the right receiver. A2 holds : the
//arm X was stationary at the station when the ladder was interrupted.
bool TTrayArmModule::HomeDrainTick()
{
    if(Status==TAS_PICKING && PickTask>=1000 && PickTask<=4000)
    {
        if(DoPick(1)==false)
            return false;
        Status=TAS_CARRYING;
        if(Job==TAJOB_LOADER_RECOVERY)
            DecidePlaceDestAfterPick();
        return true;
    }
    if((Status==TAS_CARRYING || Status==TAS_PLACING) && bHasTray &&
       PlaceTask>=1000 && PlaceTask<=4000)
    {
        if(DoPlace(1)==false)
            return false;
        Status=TAS_IDLE;
        Job=TAJOB_NONE;
    }
    return true;
}
//---------------------------------------------------------------------------
void TTrayArmModule::PauseTimeoutTimers()
{
    //AI(ht160s-rearready-p0) 20260705 : freeze the blocked-pick watchdog across a machine
    //pause so the paused span is not charged against the window (csystem
    //PauseActuatorTimeoutTimers edge; mirrors Loader FeedWaitTimer). ArmDelay is
    //deliberately NOT here : it is a short clamp-settle dwell re-armed via SetMS+On
    //WITHOUT Clear, and HTimer::On() does not reset Paused -- pausing it could leave
    //the production/Teach-test dwell Paused-stuck.
    PickWaitTimer.Pause();
    PlaceWaitTimer.Pause();   //AI(ht160s-home-resume-p0) 20260710 : same freeze rule as the pick watchdog
}
//---------------------------------------------------------------------------
void TTrayArmModule::ReStartTimeoutTimers()
{
    PickWaitTimer.ReStart();
    PlaceWaitTimer.ReStart();
}
//---------------------------------------------------------------------------
static AnsiString SR_TrayArmStatusText(int St)
{
    switch(St)
    {
        case TAS_IDLE:     return "IDLE";
        case TAS_PICKING:  return "PICKING";
        case TAS_CARRYING: return "CARRYING";
        case TAS_PLACING:  return "PLACING";
    }
    return "?" + IntToStr(St);
}
//---------------------------------------------------------------------------
static AnsiString SR_TrayArmJobText(int J)
{
    switch(J)
    {
        case TAJOB_NONE:              return "NONE";
        case TAJOB_EMPTYTRAY_TO_AUTO: return "EMPTY_TO_AUTO";
        case TAJOB_LOADER_RECOVERY:   return "LOADER_RECOVERY";
        case TAJOB_AMR_SUPPLY:        return "AMR_SUPPLY";
    }
    return "?" + IntToStr(J);
}
//---------------------------------------------------------------------------
AnsiString TTrayArmModule::DescribeState()
{
    //AI(ht160s-rearready-p0) 20260705 : read-only latched-state dump for FeederDecision.txt
    //(report 5.2 : the arm was invisible in a State Record -- a rear-ready gate wait could
    //not be told apart from an idle arm). Latched members only; no sensor refresh here.
    AnsiString s;
    s  = "[TrayArm]\r\n";
    s += "  Status=" + SR_TrayArmStatusText(Status)
       + "  Job=" + SR_TrayArmJobText(Job)
       + "  PickTask=" + IntToStr(PickTask)
       + "  PlaceTask=" + IntToStr(PlaceTask) + "\r\n";
    s += "  bHasTray=" + IntToStr(bHasTray ? 1 : 0)
       + "  PlaceDest=" + IntToStr(PlaceDest)
       + "  iDeliverKind=" + IntToStr(iDeliverKind)
       + "  iDeliverTrayID=" + iDeliverTrayID
       + "  iAutoTarget=" + IntToStr(iAutoTarget) + "\r\n";
    s += "  PickWaitArmed=" + IntToStr(bPickWaitArmed ? 1 : 0)
       + "  PlaceWaitArmed=" + IntToStr(bPlaceWaitArmed ? 1 : 0)
       + "  bCleanOutFinish=" + IntToStr(bCleanOutFinish ? 1 : 0)
       + "  SoftSim=" + IntToStr(IsSoftSimulate() ? 1 : 0) + "\r\n";
    return s;
}
//---------------------------------------------------------------------------
void TTrayArmModule::OnPickGateBlocked(AnsiString Source)
{
    //AI(ht160s-rearready-p0) 20260705 : blocked-pick watchdog (report P1; silent-stop-
    //must-notify). The DoPick 1/10 readiness gates wait unbounded and silently -- the
    //"machine looks healthy but does nothing" posture that made the field distrust the
    //predicate and hand-revert it. MES0920 pattern : arm a wall-clock window on the
    //first blocked cycle; if a full window elapses still blocked, snapshot the State
    //Record FIRST (FeederDecision.txt carries every gate input + this arm's posture),
    //then raise a Note naming the source module. The window re-arms only AFTER the
    //alarm, so the Note repeats at most once per window, never every cycle. Cleared on
    //gate pass / job reset (DoPick Flag==0) / InitialFlag; frozen across machine pause
    //via PauseTimeoutTimers.
    //AI(ht160s-rearready-p0) 20260705 : poll-continuity guard (ion-fan ION_FAN_SCAN_GAP_MS
    //precedent). This is polled once per MainProc pass while blocked; a gap larger than
    //the scan gap means the span was NOT blocked run-time -- a modal Note (possibly from
    //the very module causing the block) froze MainProc, IO Set View suspended it, or the
    //machine was stopped. Charging that span would pop a false MES1721 the moment the
    //operator restarts. Re-arm instead : the alarm can only ever fire late, never early.
    //(Makes the csystem Pause/ReStart freeze redundant-but-harmless for this timer.)
    unsigned int uNow=GetTickCount();
    bool bPollGap = (dwPickGateLastPollTick!=0 &&
                     (uNow-dwPickGateLastPollTick)>(unsigned int)TRAYARM_PICK_GATE_SCAN_GAP_MS);
    dwPickGateLastPollTick=uNow;
    if(bPickWaitArmed==false || bPollGap)
    {
        PickWaitTimer.Clear();   //HTimer trap : On() does not reset Paused -- Clear() first
        PickWaitTimer.SetMS(TRAYARM_PICK_GATE_ALARM_MS);
        PickWaitTimer.On();
        bPickWaitArmed=true;
        return;
    }
    if(PickWaitTimer.Off()==false)
        return;
    bPickWaitArmed=false;
    PickWaitTimer.Clear();
    dwPickGateLastPollTick=0;
    //AI(ht160s-rearready-p0) 20260705 : STOP FIRST, then snapshot. TriggerSnapshot does
    //multi-second synchronous IO on this control thread (config-tree copy + SECS day log
    //+ 7z, worst case ~60s in CompressFolder's WaitForSingleObject) -- running motors
    //would be unsupervised for that whole span. Everything the snapshot captures is
    //latched module state a decel stop does not disturb. ShowNoteAlarm repeats the stop
    //(idempotent).
    HSys.DecStopAllMotor();
    HSys.Sys.SystemStart=false;
    if(gStateRecord!=NULL)
        gStateRecord->TriggerSnapshot("TrayArmPickBlocked_" + Source);
    ShowMyError("MES1721", LangT("TrayArm pick blocked - rear source not ready") + " (" + Source + ")", K_RETRY);
}
//---------------------------------------------------------------------------
void TTrayArmModule::OnPlaceGateBlocked(AnsiString Dest)
{
    //AI(ht160s-home-resume-p0) 20260710 : blocked-place watchdog (silent-stop-must-notify;
    //exact mirror of OnPickGateBlocked). The DoPlaceToEmpty/DoPlaceToColor case-500
    //rear-clear waits were unbounded and silent -- after a mid-carry full-machine HOME
    //wiped the receiver's bReturnTray handshake the arm could pin here forever with no
    //alarm (the pick-side watchdog never covered the place side). Same MES0920 window
    //pattern, same poll-continuity re-arm rule, same stop-then-snapshot posture.
    unsigned int uNow=GetTickCount();
    bool bPollGap = (dwPlaceGateLastPollTick!=0 &&
                     (uNow-dwPlaceGateLastPollTick)>(unsigned int)TRAYARM_PICK_GATE_SCAN_GAP_MS);
    dwPlaceGateLastPollTick=uNow;
    if(bPlaceWaitArmed==false || bPollGap)
    {
        PlaceWaitTimer.Clear();   //HTimer trap : On() does not reset Paused -- Clear() first
        PlaceWaitTimer.SetMS(TRAYARM_PLACE_GATE_ALARM_MS);
        PlaceWaitTimer.On();
        bPlaceWaitArmed=true;
        return;
    }
    if(PlaceWaitTimer.Off()==false)
        return;
    ClearPlaceGateWatch();
    //AI(ht160s-home-resume-p0) 20260710 : STOP FIRST, then snapshot (TriggerSnapshot does
    //multi-second synchronous IO on this control thread; see OnPickGateBlocked rationale).
    HSys.DecStopAllMotor();
    HSys.Sys.SystemStart=false;
    if(gStateRecord!=NULL)
        gStateRecord->TriggerSnapshot("TrayArmPlaceBlocked_" + Dest);
    ShowMyError("MES1723", LangT("TrayArm place blocked - destination rear not free") + " (" + Dest + ")", K_RETRY);
}
//---------------------------------------------------------------------------
void TTrayArmModule::ClearPlaceGateWatch()
{
    PlaceWaitTimer.Clear();   //HTimer trap : also resets a Paused timer
    bPlaceWaitArmed=false;
    dwPlaceGateLastPollTick=0;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::IsCarriedTrayAlreadyDeposited()
{
    //AI(ht160s-home-resume-drain) 20260713 : adopt-as-delivered detector (TA-2/XS-1/XS-2).
    //A full-machine HOME can interrupt a deposit ladder AFTER DoLowerClampRaise case 2000
    //opened the jaws (tray released onto the destination rear) but BEFORE the case-4000
    //sign. InitialFlag(keepMaterial) then resumes with bHasTray still latched, so DoPlace
    //re-runs the deposit : the Auto path re-descends open-jawed over the placed tray, and
    //the Empty/Color case-100 heal re-sends RequestReturnTray (receiver GoUp-collects the
    //just-placed tray, then an empty-jaw deposit signs a phantom finish). Detect the window
    //: carry latch set, BOTH clamp On sensors read OFF (jaws physically open = released),
    //and the destination rear still shows the tray. REAL + non-DUMMY only (same gate as the
    //InitialFlag residue adopt and the uHome case-2 keep-clamps guard; sim/DUMMY have no
    //trustworthy reeds). INFERRED-safe, NO production precedent -- on-machine verify.
#ifdef SOFT_SIMULATE
    return false;
#else
    if(HSys.LastSet.iRealDummy==DUMMY)
        return false;
    if(bHasTray==false)
        return false;
    if(HSys.Cyn.C_TrayArm_FrontClamp.OnSensor.Enable==false ||
       HSys.Cyn.C_TrayArm_RearClamp.OnSensor.Enable==false)
        return false;
    if(HSys.Cyn.C_TrayArm_FrontClamp.OnSensor.IsOn() ||
       HSys.Cyn.C_TrayArm_RearClamp.OnSensor.IsOn())
        return false;   //a clamp still reads On = still gripping, not yet released
    if(PlaceDest==TAPLACE_AUTO)
        return (AutoModule!=NULL && iAutoTarget>=0 &&
                AutoModule->IsRearPlacedButUnsigned(iAutoTarget));
    if(PlaceDest==TAPLACE_EMPTY)
        return (EmptyModule!=NULL && EmptyModule->IsRearHasTray());
    if(PlaceDest==TAPLACE_COLOR)
        return (ColorModule!=NULL && ColorModule->IsRearHasTray());
    return false;
#endif
}
//---------------------------------------------------------------------------
bool TTrayArmModule::IsCleanOutFinish()
{
    //AI(cleanout) 20260701 : real CleanOut finish gate (was: always true). TrayArm still has
    //work while it is recovering empty trays off the Loader rear and recycling them to Empty/
    //Color, so it must NOT report finished until the upstream (Loader + Auto) has drained AND
    //the arm is empty (it has placed its tray to Empty/Color) AND the Z lift is up. Empty/Color
    //gate their own CleanOut drain+finish on this, so it is the cascade hinge (Loader -> SortArm
    //-> Auto -> TrayArm -> Empty/Color). Computed live (not a latch) so a fresh tray appearing at
    //the Loader rear correctly un-finishes it. Outside CleanOut the value is unused.
    if(HSys.Sys.RunMode!=Run_CleanOut)
        return bCleanOutFinish;
    if(LoaderModule==NULL || LoaderModule->IsAllCleanOutFinish()==false)
        return false;
    if(AutoModule==NULL || AutoModule->IsAllCleanOutFinish()==false)
        return false;
    if(HasTray())
        return false;              //arm still carries a tray to deliver/recycle
    if(Job!=TAJOB_NONE)
        return false;              //a delivery job is still in flight
    if(Status!=TAS_IDLE)
        return false;              //AI(ht160s-status) 20260703 : status says the arm is still working (belt beside Job)
    if(IsZUpAtPosition()==false)
        return false;              //Z lift not confirmed up
    return true;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::IsSoftSimulate()
{
    #ifdef SOFT_SIMULATE
    return true;
    #else
    return (HSys.LastSet.iRealDummy==DUMMY);
    #endif
}
//---------------------------------------------------------------------------
bool TTrayArmModule::IsZUpAtPosition()
{
    //AI(HT160S-Maintainer) 20260622 : the ONE canonical TrayArm X-move precondition - the Z lift
    //cylinder is confirmed at the UP position (up-sensor lit). Anti-collision is a HARD safety
    //law : it stays ACTIVE in real-machine DUMMY/HAS_TRAY/REALLY (in DUMMY the X motor and the Z
    //cylinder still move PHYSICALLY; DUMMY only skips the correctness sensor confirmations).
    //Bypass ONLY the SOFT_SIMULATE dev build, where there is no IO card and the sensor read is
    //meaningless (a runtime DUMMY bypass would wrongly disarm the interlock on the real machine).
    #ifdef SOFT_SIMULATE
    return true;
    #else
    return HSys.Cyn.C_TrayArmZ_Up.IsOn();
    #endif
}
//---------------------------------------------------------------------------
bool TTrayArmModule::MoveTrayArmX(int Position)
{
    if(HSys.Mot.MTrayArmX==NULL)
        return false;
    if(HSys.Mot.MTrayArmX->CheckSoftLimit(Position)==false)
    {
        ShowMotorLimitError(HSys.Mot.MTrayArmX->AlarmName[eMotOverLimitErr], LangT("Tray Arm X motor will out of limit"), HSys.Mot.MTrayArmX, Position);
        return false;
    }
    //AI(HT160S-Maintainer) 20260622 : Z-up lift interlock (single chokepoint via IsZUpAtPosition).
    //TrayArm X may traverse ONLY while the Z lift is confirmed UP, so the head/tray can never swing
    //across a station while lowered. Checked on EVERY call, so a head that drops off the up-sensor
    //mid-travel (air loss / cylinder sag) is caught too, not only before the move. A short
    //time-window debounce rejects a single bad read; on a confirmed loss decel-stop ALL motion and
    //raise the alarm (which also drops SystemStart, mirroring SortArm AreAllSuckersHome). DoZUp
    //already confirms the up-sensor before the first call in every run mode, so this never
    //false-trips waiting for the initial rise. Unreachable in SOFT_SIMULATE (IsZUpAtPosition true).
    if(IsZUpAtPosition()==false)
    {
        HSys.Mot.MTrayArmX->Stop();   //hold the arm each tick (mode-0 decel stop)
        if(dwZUpLostStart==0)
            dwZUpLostStart=GetTickCount();
        else if((int)(GetTickCount()-dwZUpLostStart)>=TRAYARM_ZUP_LOST_MS)
        {
            dwZUpLostStart=0;
            HSys.StopAllMotor();   //confirmed loss : real decel-stop ALL
            ShowSystemError("TrayArm move blocked : the Z lift left its UP sensor. Check the TrayArmZ up cylinder / air pressure.", K_RETRY);
        }
        return false;
    }
    dwZUpLostStart=0;
    return HSys.Mot.MTrayArmX->MotorMove(Position);
}
//---------------------------------------------------------------------------
bool TTrayArmModule::DoZUp()
{
    //AI(HT160S-Maintainer) 20260605 : dual-coil Z, drop the down coil before driving up.
    HSys.Cyn.C_TrayArmZ_Down.Off();
    //AI(HT160S-Maintainer) 20260622 : anti-collision hard safety - the X traverse that follows must
    //NEVER start with the head still lowered, so do not report Z-up done until the UP sensor really
    //confirms (IsZUpAtPosition). This is what the real-machine DUMMY mode needs : Push() returns
    //true immediately in DUMMY (its own sensor wait is skipped for the dry run) and also returns
    //true if the cylinder OnSensor.Enable flag is off, so the old "Push() || IsSoftSimulate()" let X
    //start before the head physically rose. In REALLY/HAS_TRAY Push() already waits for + times-out-
    //alarms on the sensor, so this never weakens the cylinder's own alarm. IsZUpAtPosition holds the
    //single SOFT_SIMULATE bypass (dev simulation only), so this line is one rule for every run mode.
    bool bPushed=HSys.Cyn.C_TrayArmZ_Up.Push();
    return (bPushed && IsZUpAtPosition());
}
//---------------------------------------------------------------------------
bool TTrayArmModule::DoZDown()
{
    HSys.Cyn.C_TrayArmZ_Up.Off();
    return (HSys.Cyn.C_TrayArmZ_Down.Push() || IsSoftSimulate());
}
//---------------------------------------------------------------------------
bool TTrayArmModule::DoMoveToStationZSafe(int X, int &Task)
{
    //AI(ht160s-trayarm-teach-test) 20260627 : shared "move the head to a station, Z safe"
    //primitive. Z-up (anti-collision) then X traverse to the station X. Task 1->10; returns
    //true once the arm is at X with the Z lift confirmed UP. The Z-up interlock and soft-limit
    //guard stay inside MoveTrayArmX. Caller enters at Task=1.
    switch(Task)
    {
        case 1:
            if(DoZUp())
                Task=10;
            break;

        case 10:
            if(MoveTrayArmX(X))
                return true;
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::DoLowerClampRaise(bool bGrab, int &Task)
{
    //AI(ht160s-trayarm-teach-test) 20260627 : shared "lower, actuate clamps, raise" primitive -
    //the physical grab/release choreography. Z-down, then push (grab) or pop (release) BOTH edge
    //clamps with the same 3-tick settle as before, then Z-up. Task 1000->2000->2100->3000; returns
    //true once raised. This is the ONE copy of the clamp choreography (DoPick/DoPlace/DoPlaceTo*
    //and the Teach test all call it), so a change here propagates everywhere. Caller enters at 1000.
    //CONTRACT : every caller's outer switch MUST group the case labels 1000/2000/2100/3000 onto this
    //helper (and 1/10 onto DoMoveToStationZSafe). Renumbering these internal Task values without
    //updating the callers' grouped labels would let a Task value escape the switch and silently hang.
    switch(Task)
    {
        case 1000:
            if(DoZDown())
                Task=2000;
            break;

        case 2000:
        {
            bool bClamp = bGrab
                ? (HSys.Cyn.C_TrayArm_FrontClamp.Push() && HSys.Cyn.C_TrayArm_RearClamp.Push())
                : (HSys.Cyn.C_TrayArm_FrontClamp.Pop()  && HSys.Cyn.C_TrayArm_RearClamp.Pop());
            if(bClamp || IsSoftSimulate())
            {
                ArmDelay.SetMS(GeneralSetting.iTrayArmClampSettleMs);
                ArmDelay.On();
                Task=2100;
            }
            break;
        }

        case 2100:
            if(ArmDelay.Off())
                Task=3000;
            break;

        case 3000:
            if(DoZUp())
                return true;
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
int TTrayArmModule::GetAutoX(int Index)
{
    switch(Index)
    {
        case 0: return Teach.TrayXArmToAuto1XPosition;
        case 1: return Teach.TrayXArmToAuto2XPosition;
        case 2: return Teach.TrayXArmToAuto3XPosition;
        case 3: return Teach.TrayXArmToAuto4XPosition;
        case 4: return Teach.TrayXArmToAuto5XPosition;
        case 5: return Teach.TrayXArmToAuto6XPosition;
    }
    return Teach.TrayXArmToAuto1XPosition;
}
//---------------------------------------------------------------------------
int TTrayArmModule::GetColorX()
{
    //AI(HT160S-Maintainer) 20260605 : AMR identity-tray pickup position at Color station.
    return Teach.TrayXArmToColorXPosition;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::IsPickFromColor()
{
    //AI(HT160S-Maintainer) 20260605 : only the AMR identity tray (stack bottom, carries
    //the 2D TrayID) is picked from Color. Cover and normal trays come from EmptyTray.
    return (Job==TAJOB_AMR_SUPPLY && iDeliverKind==eTrayKindIdentity);
}
//---------------------------------------------------------------------------
int TTrayArmModule::DecideJob()
{
    //AI(HT160S-Maintainer) 20260606 : Priority 1 : recover an empty tray stranded at the
    //Loader rear. The Loader only discharges a tray to its rear after its own gate proves
    //the tray has no IC (DoLoader case3000 rejects trays that still carry IC), so
    //IsRearHasTray()==true here means an EMPTY tray is waiting to be cleared. Clearing it
    //first (as the old 160 did) keeps the Loader free to feed the next source tray. The
    //destination (supply an Auto vs recycle to EmptyTray) is decided after the pick, in
    //DecidePlaceDestAfterPick(), so the arm reacts to the live Auto demand at that moment.
    if(LoaderModule!=NULL && LoaderModule->IsRearHasTray())
    {
        //AI(HT160S-Maintainer) 20260625 : carry the REAL kind/2D the Loader stamped on the
        //tray at feed (Phase 6 A : GetRearTrayKind/GetRearTrayID), so an identity tray is
        //routed back to Color while empty/cover trays keep the existing Empty path.
        int iRearKind=(int)LoaderModule->GetRearTrayKind();
        //AI(ht160s-agv-identity2d) 20260714 : PICK-TIME interlock (owner). A Loader IDENTITY tray
        //goes to Color to be scanned + uploaded (CEID275/SVID38204); do NOT pick it until Color is
        //FULLY idle, else the Color-rear handoff collides with a Color carriage move. Reserve Color
        //(RequestReadIdentityTray -> Color drains to idle, starts no new supply/destack) and, if it
        //is not idle yet, FALL THROUGH to the lower-priority jobs so TrayArm keeps servicing (incl.
        //picking Color's presented tray) -> Color can reach idle. Deadlock-safe: we do NOT return
        //TAJOB_NONE here (which would freeze TrayArm into a mutual wait with a Color that cannot idle).
        bool bDeferIdentity=false;
        if(iRearKind==eTrayKindIdentity && ColorModule!=NULL)
        {
            ColorModule->RequestReadIdentityTray();
            //AI(ht160s-agv-identity2d) 20260714 : defer in BOTH sim and real until Color is idle.
            //IsReadyToReceiveIdentity is sim-reachable (task/latch based, cylinder check skipped in
            //sim), so a headless intake still lands on an EMPTY carriage -- no case-1
            //carriageNotClear stall/log-spam. Deadlock-safe via the fall-through below.
            if(ColorModule->IsReadyToReceiveIdentity()==false)
                bDeferIdentity=true;
        }
        if(bDeferIdentity==false)
        {
            iAutoTarget=-1;
            iDeliverKind=iRearKind;
            iDeliverTrayID=LoaderModule->GetRearTrayID();
            return TAJOB_LOADER_RECOVERY;
        }
        //bDeferIdentity : Color still draining to idle -> fall through to the branches below so
        //TrayArm services other work this cycle; the Loader identity tray is retried next cycle.
    }

    //AI(HT160S-Maintainer) 20260605 : AMR mode builds each Auto output stack in a fixed
    //order : tray[0]=identity (picked from Color, carries the 2D TrayID, no IC),
    //tray[1]=cover (empty tray from EmptyTray, no IC), tray[2..]=normal work trays.
    //The needed kind for the next delivery is derived from the Auto car's tray count.
    if(GeneralSetting.bUseAMR)
    {
        if(AutoModule==NULL)
            return TAJOB_NONE;
        //AI(general) 20260608 : Stage1 demand-driven - ask the Auto stations which one
        //wants a tray and what kind. No requesting Auto -> the arm stays idle (no
        //needless shuttling). The needed kind still follows the AMR stack order.
        int kind=eTrayReqNone;
        int idx=AutoModule->FindTrayRequestAuto(kind);
        if(idx<0)
            return TAJOB_NONE;             //no Auto wants a tray, nothing to deliver
        if(kind==eTrayKindIdentity)
        {
            //Identity tray comes from Color. Only start when Color presents one at the
            //pickup; otherwise ask Color to supply and wait (no deadlock : Color refills).
            if(ColorModule==NULL)
                return TAJOB_NONE;
            if(ColorModule->IsTrayReady()==false)
            {
                ColorModule->RequestSupplyTray();
                return TAJOB_NONE;
            }
        }
        else
        {
            //Cover / normal trays come from the EmptyTray rear pickup.
            if(EmptyModule==NULL || EmptyModule->IsRearHasTray()==false)
                return TAJOB_NONE;
        }
        iAutoTarget=idx;
        iDeliverKind=kind;
        return TAJOB_AMR_SUPPLY;
    }

    //AI(HT160S-Maintainer) 20260605 : Supply path : deliver an empty tray from the
    //EmptyTray rear pickup position to an Auto station whose rear is free. APIs and
    //Teach coordinates for this path are complete, and it has no cross-module deadlock
    //(EmptyTray auto-refills its rear after each pick), so it is safe to dispatch.
    if(EmptyModule!=NULL && AutoModule!=NULL)
    {
        if(EmptyModule->IsRearHasTray())
        {
            //AI(general) 20260608 : Stage1 demand-driven - only fetch an empty tray when
            //an Auto actually wants one (Normal-mode request kind is always Normal).
            int kind=eTrayReqNone;
            int idx=AutoModule->FindTrayRequestAuto(kind);
            if(idx>=0)
            {
                iAutoTarget=idx;
                iDeliverKind=eTrayKindNormal;   //AI(ht160s-home-resume-w6) 20260711 : TA-3 - a plain Empty->Auto supply must not inherit a stale Identity kind (the CleanOut divert routes on it and would missend a plain tray to Color)
                return TAJOB_EMPTYTRAY_TO_AUTO;
            }
        }
    }

    //AI(HT160S-Maintainer) 20260606 : Loader empty-tray recovery is now dispatched as
    //Priority 1 at the top of this function (TAJOB_LOADER_RECOVERY). Its two missing
    //handshakes are now satisfied : (1) the Loader only parks EMPTY trays at its rear
    //(DoLoader rejects trays that still hold IC before discharge), so IsRearHasTray() here
    //is safe to pick; (2) recycling to EmptyTray uses the RequestReturnTray() ->
    //IsRearHasTray()==false -> NotifyTrayXToEmptyFinish() handshake, so EmptyTray first
    //goes up to free its rear before accepting the returned tray (no rear contention).
    return TAJOB_NONE;
}
//---------------------------------------------------------------------------
int TTrayArmModule::GetPickSourceX()
{
    //AI(HT160S-Maintainer) 20260606 : pick X depends on the source of the current job.
    //Loader recovery picks from the Loader rear; the AMR identity tray from Color; every
    //other job picks from the EmptyTray rear.
    if(Job==TAJOB_LOADER_RECOVERY)
        return Teach.TrayXArmToLoaderXPosition;
    if(IsPickFromColor())
        return GetColorX();
    return Teach.TrayXArmToEmptyXPosition;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::DoPick(int Flag)
{
    //AI(HT160S-Maintainer) 20260605 : pick an empty tray from the EmptyTray rear.
    //Z-safe before X, then Z-down, clamp the tray (front+rear clamps hold the same
    //tray on its front/rear edges), Z-up, then hand off the EmptyTray rear slot.
    //AI(ht160s-trayarm-teach-test) 20260627 : the physical grab motion is now the shared
    //DoMoveToStationZSafe + DoLowerClampRaise primitives (same as DoPlace and the Teach test);
    //only the rear-slot handoff (case 4000) stays here. Task progression is unchanged.
    if(Flag==0)
    {
        PickTask=1;
        ArmDelay.Clear();
        PickWaitTimer.Clear();   //AI(ht160s-rearready-p0) 20260705 : new job = new watchdog window
        bPickWaitArmed=false;
        dwPickGateLastPollTick=0;
        return true;
    }

    switch(PickTask)
    {
        case 1:
        case 10:
            if(DoMoveToStationZSafe(GetPickSourceX(), PickTask))
            {
                if(Job==TAJOB_EMPTYTRAY_TO_AUTO)
                {
                    //AI(ht160s-trayarm-empty-handoff) 20260701 : wait here (Z still UP) until the
                    //Empty rear tray is present AND not being returned by the carrier. Producer-owned
                    //readiness predicate replaces the magic-70000 encoder threshold. Deadlock-safe vs
                    //the MoveEmptyY symmetric guard because we hold Z-UP (that guard only blocks EmptyY
                    //while TrayArm Z is DOWN at the Empty X).
                    if(EmptyModule!=NULL && EmptyModule->IsRearReadyForPick()==false)
                    {
                        OnPickGateBlocked("Empty");   //AI(ht160s-rearready-p0) 20260705 : watchdog tick while blocked
                        break;
                    }
                }
                if(Job==TAJOB_AMR_SUPPLY && IsPickFromColor()==false)
                {
                    //AI(ht160s-trayarm-empty-handoff) 20260703 : AMR-mode cover/normal trays are
                    //also picked from the Empty rear (GetPickSourceX returns TrayXArmToEmptyXPosition
                    //whenever IsPickFromColor()==false), so they need the SAME Z-UP-wait gate as the
                    //TAJOB_EMPTYTRAY_TO_AUTO path above. Without it the AMR supply job dispatched only
                    //on the RAW IsRearHasTray() latch (DecideJob) and dove onto the Empty rear while
                    //the carrier was still delivering / the transport clamps were still engaged - the
                    //same collision class as the onsite issue-C that IsRearReadyForPick() fixed on the
                    //AMR=0 path. Identity trays come from Color (IsPickFromColor()==true) and are
                    //excluded here : their readiness is Color's own bTrayReady latch. Deadlock-safe :
                    //we hold Z-UP and MoveEmptyY only blocks EmptyY while TrayArm Z is DOWN at Empty X.
                    if(EmptyModule!=NULL && EmptyModule->IsRearReadyForPick()==false)
                    {
                        OnPickGateBlocked("Empty");   //AI(ht160s-rearready-p0) 20260705 : watchdog tick while blocked
                        break;
                    }
                }
                if(Job==TAJOB_LOADER_RECOVERY)
                {
                    //AI(ht160s-trayarm-empty-handoff) 20260701 : same Z-UP-wait gate for the Loader
                    //rear pick. bRearHasTray latches while the discharge carriage is still at
                    //discharge Y and clamps are releasing; IsRearReadyForPick() holds until the
                    //carriage has retreated to feed (case 4000). Deadlock-safe : MoveLoaderY's
                    //TrayArm guard (added 20260701) is Z-gated -- it only blocks Loader Y while the
                    //TrayArm Z is DOWN at the Loader X -- and this wait holds Z-UP, so the source Y
                    //is never blocked by the waiting arm.
                    if(LoaderModule!=NULL && LoaderModule->IsRearReadyForPick()==false)
                    {
                        OnPickGateBlocked("Loader");   //AI(ht160s-rearready-p0) 20260705 : watchdog tick while blocked
                        break;
                    }
                }
                PickWaitTimer.Clear();   //AI(ht160s-rearready-p0) 20260705 : gate passed -- close the watchdog window
                bPickWaitArmed=false;
                dwPickGateLastPollTick=0;
#ifndef SOFT_SIMULATE
                //AI(ht160s-home-resume-w6) 20260711 : TP-1 asymmetry guard. uHome keeps the
                //clamps closed on EITHER clamp On reed (conservative never-drop) while the
                //InitialFlag residue-adopt needs BOTH On (phantom-adopt safety) -- with
                //exactly one reed On the arm reaches a fresh pick closed-jawed holding a
                //tray the latch does not know about, and the grab ladder would Z-down
                //closed onto the occupied source rear (double-stack jam). Require both
                //reeds fully OPEN before any new grab; otherwise route to the existing
                //MES1722 removal flow instead of diving.
                if(HSys.LastSet.iRealDummy!=DUMMY &&
                   ((HSys.Cyn.C_TrayArm_FrontClamp.OnSensor.Enable && HSys.Cyn.C_TrayArm_FrontClamp.OnSensor.IsOn()) ||
                    (HSys.Cyn.C_TrayArm_RearClamp.OnSensor.Enable && HSys.Cyn.C_TrayArm_RearClamp.OnSensor.IsOn())))
                {
                    ShowMyError("MES1722", LangT("TrayArm holds an unidentified tray - open the clamps in Teach and remove it"), K_RETRY);
                    return false;
                }
#endif
                PickTask=1000;
            }
            break;

        case 1000:
        case 2000:
        case 2100:
        case 3000:
            if(DoLowerClampRaise(true, PickTask))
                PickTask=4000;
            break;

        case 4000:
            if(Job==TAJOB_LOADER_RECOVERY)
            {
                //AI(HT160S-Maintainer) 20260625 : transfer the rear tray data onto the arm
                //(U3 born-at-source/handoff). The copy MUST precede NotifyTrayArmPickRearTray,
                //which clears the Loader rear hold (RearSourceTray/RearKind/RearTrayID).
                if(LoaderModule!=NULL)
                {
                    if(HSys.VMot.MMTrayArmX!=NULL) HSys.VMot.MMTrayArmX->Tray.CopyFrom(LoaderModule->GetRearSourceTray());
                    iDeliverTrayID=LoaderModule->GetRearTrayID();
                    //AI(ht160s-rearready-p0) 20260705 : re-read the KIND at pick time too.
                    //DecideJob latched iDeliverKind at dispatch; a job pinned at the pick
                    //gate can resume on a LATER discharged tray (e.g. after the MES0924
                    //leftover was removed and production continued), and routing that
                    //tray by the stale kind would send an identity tray to Empty/Auto
                    //instead of back to Color.
                    iDeliverKind=(int)LoaderModule->GetRearTrayKind();
                    LoaderModule->NotifyTrayArmPickRearTray();
                }
            }
            else if(IsPickFromColor())
            {
                //AI(HT160S-Maintainer) 20260608 : carry the 2D TrayID that Color read off
                //the identity tray so it can be stamped onto the Auto stack on place.
                if(ColorModule!=NULL)
                {
                    iDeliverTrayID=ColorModule->GetTrayID();
                    if(HSys.VMot.MMTrayArmX!=NULL) HSys.VMot.MMTrayArmX->Tray.CopyFrom(ColorModule->GetSourceTray());   //AI(ht160s-tray-source) : carry identity-tray grid before release
                    ColorModule->NotifyTrayPicked();
                }
            }
            else
            {
                if(EmptyModule!=NULL)
                {
                    if(HSys.VMot.MMTrayArmX!=NULL) HSys.VMot.MMTrayArmX->Tray.CopyFrom(EmptyModule->GetSourceTray());   //AI(ht160s-tray-source) : carry EMPTY_IC/Normal grid before release
                    EmptyModule->SetRearHasTray(false);
                }
            }
            if(HSys.VMot.MMTrayArmX!=NULL)
                HSys.VMot.MMTrayArmX->fHasTray=true;
            bHasTray=true;
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::DoPlace(int Flag)
{
    //AI(HT160S-Maintainer) 20260605 : place the carried empty tray onto the target
    //Auto rear. Z-safe before X, Z-down, release clamps, Z-up, then mark the Auto rear
    //as filled so SortArm can sort IC into it.
    if(Flag==0)
    {
        PlaceTask=1;
        ArmDelay.Clear();
        ClearPlaceGateWatch();   //AI(ht160s-home-resume-p0) 20260710 : new place job = new watchdog window
        return true;
    }

    //AI(ht160s-home-resume-drain) 20260713 : adopt-as-delivered fast-forward (TA-2/XS-1/
    //XS-2). On a resume re-entering at PlaceTask==1, if the carried tray was already
    //deposited on its destination rear before a full-machine HOME (jaws open, rear
    //occupied+unsigned) skip the deposit ladder and jump to the case-4000 handoff commit --
    //do NOT re-descend open-jawed (Auto) nor let the case-100 heal re-collect it (Empty/
    //Color). This runs before the PlaceDest dispatch so DoPlaceToColor/DoPlaceToEmpty(1)
    //also enter their switch at case 4000.
    if(PlaceTask==1 && IsCarriedTrayAlreadyDeposited())
        PlaceTask=4000;

    //AI(HT160S-Maintainer) 20260606 : Loader-recovery jobs may instead recycle the tray
    //back to the EmptyTray rear when no Auto needs one. Dispatch to that path.
    if(PlaceDest==TAPLACE_COLOR)
        return DoPlaceToColor(Flag);
    if(PlaceDest==TAPLACE_EMPTY)
        return DoPlaceToEmpty(Flag);

    //AI(ht160s-trayarm-teach-test) 20260627 : the physical release motion is now the shared
    //DoMoveToStationZSafe + DoLowerClampRaise primitives (same as DoPick and the Teach test);
    //only the Auto rear staging/notify (case 4000) stays here. Task progression is unchanged.
    switch(PlaceTask)
    {
        case 1:
        case 10:
            //AI(cleanout) 20260701 : in-flight divert at the drain boundary. The Auto-side
            //GetTrayRequest drain gate only stops NEW dispatches; a delivery already committed
            //by DecideJob before SortArm finished would still land on an Auto that is switching
            //to (or has finished) its clean-out discharge - the physical tray would be stranded
            //on the Auto rear shelf (rear->car pull no longer runs after clean-out). Re-check
            //the same boundary signal (SortArm.IsCleanOutFinish, the exact gate GetTrayRequest
            //uses) on every tick while the tray is still IN HAND (PlaceTask 1/10 : Z up, clamps
            //closed, before the deposit ladder). On divert, reroute the carried tray to the
            //recycle destination with the same contract as DecidePlaceDestAfterPick (identity ->
            //Color, cover/normal -> Empty; RequestReturnTray first so the rear is freed). No
            //Auto-side cleanup is needed : bRearHasTray/bRearDeliveredPending/RearGrid are only
            //written at case 4000, which we have not reached. Once DoLowerClampRaise starts the
            //tray is being set down - that residual is caught by the DoAllAutoCleanOut case-7000
            //backstop alarm instead.
            if(HSys.Sys.RunMode==Run_CleanOut &&
               SortArmModule!=NULL && SortArmModule->IsCleanOutFinish())
            {
                if(iDeliverKind==eTrayKindIdentity)
                {
                    PlaceDest=TAPLACE_COLOR;
                    if(ColorModule!=NULL)
                        ColorModule->RequestReturnTray();
                }
                else
                {
                    PlaceDest=TAPLACE_EMPTY;
                    if(EmptyModule!=NULL)
                        EmptyModule->RequestReturnTray();
                }
                iAutoTarget=-1;
                PlaceTask=1;
                break;
            }
            if(DoMoveToStationZSafe(GetAutoX(iAutoTarget), PlaceTask))
            {
                //AI(ht160s-home-resume-drain) 20260713 : TP-4 Auto rear re-verify. Unlike
                //the DoPlaceToEmpty/DoPlaceToColor case-500 gate, the Auto path had NO
                //rear-clear check -- X-in-position fell straight into DoLowerClampRaise.
                //After a full-machine HOME the case-100 heal re-signs only EMPTY/COLOR, so
                //an Auto rear that re-latched occupied would be descended onto open-jawed
                //with no re-verify and no alarm. Tick the SAME MES1723 place watchdog. sim
                //keeps the latch model (bypass mirrors the case-500 IsSoftSimulate gate).
                //The TA-2 adopt fast-forward sets PlaceTask=4000 for an already-placed tray,
                //so the switch skips this case entirely (never blocked).
                if(IsSoftSimulate()==false && AutoModule!=NULL && iAutoTarget>=0 &&
                   AutoModule->IsRearHasTray(iAutoTarget))
                {
                    OnPlaceGateBlocked("Auto");
                    break;
                }
                ClearPlaceGateWatch();
                Status=TAS_PLACING;   //AI(ht160s-status) 20260703 : deposit ladder starts (Z will lower)
                PlaceTask=1000;
            }
            break;

        case 1000:
        case 2000:
        case 2100:
        case 3000:
            if(DoLowerClampRaise(false, PlaceTask))
                PlaceTask=4000;
            break;

        case 4000:
            if(AutoModule!=NULL && iAutoTarget>=0)
            {
                //AI(ht160s-tray-source) : hand the carried grid to the Auto rear staging
                //slot for BOTH AMR and Normal. The Auto copies RearGrid into the working
                //tray at DoFeedTray case 7000; tray occupancy (fHasTray) is owned THERE,
                //not here. Setting fHasTray now would flip bCarHasTray and starve FindFeedAuto.
                if(HSys.VMot.MMTrayArmX!=NULL)
                    AutoModule->StageRearGrid(iAutoTarget, HSys.VMot.MMTrayArmX->Tray);
                if(Job==TAJOB_AMR_SUPPLY)
                    //AI(HT160S-Maintainer) 20260605 : record the delivered tray's stack
                    //role so the Auto knows identity/cover trays must NOT receive IC.
                    //AI(HT160S-Maintainer) 20260608 : also pass the identity tray's 2D
                    //TrayID (empty for cover/normal) so the Auto car carries the stack ID.
                    AutoModule->NotifyTrayArmDelivered(iAutoTarget, iDeliverKind, iDeliverTrayID);
                else
                    AutoModule->SetRearHasTrayFromTrayArm(iAutoTarget, true);
                //AI(ht160s-agv-identity2d) 20260714 : the CEID275/SVID38204 upload MOVED to the
                //Color scan point (aColor::DoReadIdentityRetreat case 300) per the finalized plan.
                //The former upload hook here (21ecb0f, TrayArm->Auto delivery) is removed so 275 is
                //fired exactly once at the Loader-recovery intake scan, not double-fired here.
            }
            if(HSys.VMot.MMTrayArmX!=NULL) HSys.VMot.MMTrayArmX->Tray.Clear();   //AI(ht160s-tray-source) : arm is now empty
            if(HSys.VMot.MMTrayArmX!=NULL)
                HSys.VMot.MMTrayArmX->fHasTray=false;
            bHasTray=false;
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
void TTrayArmModule::DecidePlaceDestAfterPick()
{
    //AI(HT160S-Maintainer) 20260606 : called once the Loader empty tray is in hand. The
    //arm reacts to the live demand : if an Auto rear is free it supplies that Auto, else
    //it recycles the tray back into the EmptyTray supply pool. AMR stacks have a strict
    //identity/cover/normal order that is built only by the dedicated AMR supply job, so a
    //recovered plain tray is never injected mid-stack in AMR mode : it always recycles.
    //AI(HT160S-Maintainer) 20260625 : an identity tray (real Kind from the Loader, Phase 6 A)
    //is never an Auto supply : route it back to Color using the SAME return contract as Empty
    //(RequestReturnTray -> IsRearHasTray()==false -> NotifyTrayXToEmptyFinish). Cover/Normal
    //fall through to the existing Auto-vs-Empty logic below unchanged (D3).
    if(iDeliverKind==eTrayKindIdentity)
    {
        //AI(ht160s-agv-identity2d) 20260714 : a Loader-recovered identity tray goes to Color to be
        //SCANNED + uploaded (CEID275/SVID38204), NOT recycled. Arm the identity-intake contract
        //(RequestReadIdentityTray -> bReadIdentityPending) rather than the recycle return; the
        //pick-time interlock in DecideJob already reserved Color (idempotent). Color->IsReceivingIdentity()
        //then makes DoPlaceToColor use the Color-idle deposit gate instead of the plain rear-free gate.
        PlaceDest=TAPLACE_COLOR;
        iAutoTarget=-1;
        if(ColorModule!=NULL)
            ColorModule->RequestReadIdentityTray();
        return;
    }
    bool bSupplyAuto=false;
    if(GeneralSetting.bUseAMR==false && AutoModule!=NULL)
    {
        //AI(general) 20260608 : Stage2 demand-driven Loader recovery - use the same pull
        //source as DecideJob (FindTrayRequestAuto) instead of FindEmptyRearForTrayArm, so
        //the recovered Loader tray is only handed to an Auto that actually requests one,
        //and only when it wants a plain Normal empty tray (the recovered tray carries no
        //identity/cover role). This also respects the Stage0 pending latch, so a tray
        //already on its way is never double-targeted. Otherwise recycle to EmptyTray.
        int kind=eTrayReqNone;
        int idx=AutoModule->FindTrayRequestAuto(kind);
        if(idx>=0 && kind==eTrayKindNormal)
        {
            iAutoTarget=idx;
            bSupplyAuto=true;
        }
    }

    if(bSupplyAuto)
    {
        PlaceDest=TAPLACE_AUTO;
    }
    else
    {
        PlaceDest=TAPLACE_EMPTY;
        iAutoTarget=-1;
        //Ask EmptyTray to free its rear (it goes up, pushing any parked rear tray into
        //the car) so it can accept the returned tray. We then wait for the rear to clear.
        if(EmptyModule!=NULL)
            EmptyModule->RequestReturnTray();
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-divert) 20260703 : mid-flight divert (user efficiency request). While the arm
//is still CARRYING a recovered tray toward the Empty rear (X traverse case 1/10 or the
//case-500 wait; Z up, clamps closed), re-check the live Auto demand every tick : if an
//Auto now requests a plain Normal tray, deliver it there FIRST instead of parking it at
//Empty and re-picking it later. Mirrors the CleanOut drain-boundary divert shape (DoPlace
//case 1/10) in the opposite direction. FindTrayRequestAuto embeds the producer-side no-go
//gates (CleanOut drain boundary, AMR lock, pending latch, rear occupied); the guards here
//keep the divert out of CleanOut entirely and out of AMR mode (AMR stacks are built only
//by the dedicated supply job), and identity trays never reach this path (routed to Color
//at DecidePlaceDestAfterPick). On success the Empty return reservation is released via
//CancelReturnTray and the DoPlace dispatch re-enters the Auto ladder at case 1.
bool TTrayArmModule::TryDivertCarriedTrayToAuto()
{
    if(HSys.Sys.RunMode==Run_CleanOut)
        return false;
    if(GeneralSetting.bUseAMR)
        return false;
    if(iDeliverKind==eTrayKindIdentity)
        return false;
    if(AutoModule==NULL)
        return false;
    int kind=eTrayReqNone;
    int idx=AutoModule->FindTrayRequestAuto(kind);
    if(idx<0 || kind!=eTrayKindNormal)
        return false;
    if(EmptyModule!=NULL)
        EmptyModule->CancelReturnTray();
    ClearPlaceGateWatch();   //AI(ht160s-home-resume-p0) 20260710 : retargeted -- the Empty rear-clear wait (and its watchdog) is abandoned
    iAutoTarget=idx;
    PlaceDest=TAPLACE_AUTO;
    PlaceTask=1;
    return true;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::DoPlaceToEmpty(int Flag)
{
    //AI(HT160S-Maintainer) 20260606 : recycle the carried empty tray back to the
    //EmptyTray rear. Action sequence mirrors the old 160 DoTrayX_Change_Empty (Z-safe,
    //move to Empty X, wait rear clear, Z-down, release clamps, Z-up). The handshake is
    //strengthened over the old global-flag version : instead of writing shared flags,
    //this calls EmptyModule->RequestReturnTray() (done in DecidePlaceDestAfterPick) and
    //only places once IsRearHasTray()==false confirms EmptyTray has gone up and freed its
    //rear, then signals completion with NotifyTrayXToEmptyFinish(). Clamp/Z cylinders
    //self-alarm on a sensor timeout (TMyCylinder::Push/Pop), so a stuck move is reported
    //rather than silently hanging.
    if(Flag==0)
    {
        PlaceTask=1;
        ArmDelay.Clear();
        ClearPlaceGateWatch();   //AI(ht160s-home-resume-p0) 20260710 : new place job = new watchdog window
        return true;
    }

    //AI(ht160s-trayarm-teach-test) 20260627 : physical motion via the shared primitives. The
    //rear-clear wait stays HERE (case 500, between the X traverse and Z-down) - same order as
    //before - so the recycle handshake is unchanged; only the lower/release/raise choreography
    //is shared. Internal Task values renumbered (500 wait, 4000 notify) but behavior identical.
    switch(PlaceTask)
    {
        case 1:
        case 10:
            if(TryDivertCarriedTrayToAuto())   //AI(ht160s-divert) 20260703 : retarget while traversing (Z up, clamps closed)
                break;
            if(DoMoveToStationZSafe(Teach.TrayXArmToEmptyXPosition, PlaceTask))
                PlaceTask=500;
            break;

        case 500:
            if(TryDivertCarriedTrayToAuto())   //AI(ht160s-divert) 20260703 : retarget during the (possibly long) rear-clear wait
                break;
            //AI(ht160s-status) 20260703 : anti-collision (on-site status==1 intent, concern B).
            //Do NOT lower onto the Empty rear while its carrier is actively FEEDING a tray there
            //- during the feed motion bRearHasTray has not latched yet (set at DoFeedTray case
            //7000), so the IsRearHasTray()==false gate below alone would let TrayArm dive into
            //the arriving carrier. Real-machine only; ES_FEEDING clears when the feed completes.
            if(IsSoftSimulate()==false && EmptyModule!=NULL && EmptyModule->GetStatus()==ES_FEEDING)
            {
                OnPlaceGateBlocked("Empty");   //AI(ht160s-home-resume-p0) 20260710 : watchdog tick while blocked
                break;
            }
            //Wait until EmptyTray has raised and cleared its rear before depositing.
            if(EmptyModule==NULL || EmptyModule->IsRearHasTray()==false || IsSoftSimulate())
            {
                ClearPlaceGateWatch();   //AI(ht160s-home-resume-p0) 20260710 : gate passed -- close the watchdog window
                Status=TAS_PLACING;   //AI(ht160s-status) 20260703 : deposit ladder starts
                PlaceTask=1000;
            }
            else
                OnPlaceGateBlocked("Empty");   //AI(ht160s-home-resume-p0) 20260710 : watchdog tick while blocked
            break;

        case 1000:
        case 2000:
        case 2100:
        case 3000:
            if(DoLowerClampRaise(false, PlaceTask))
                PlaceTask=4000;
            break;

        case 4000:
            //Tell EmptyTray the returned tray is now parked at its rear (this also marks
            //the rear as having a tray, so it re-enters the supply pool).
            if(EmptyModule!=NULL)
                EmptyModule->NotifyTrayXToEmptyFinish();
            if(HSys.VMot.MMTrayArmX!=NULL) HSys.VMot.MMTrayArmX->Tray.Clear();   //AI(ht160s-tray-source) : arm empty after recycle-to-Empty (parity with Auto place path)
            if(HSys.VMot.MMTrayArmX!=NULL)
                HSys.VMot.MMTrayArmX->fHasTray=false;
            bHasTray=false;
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::DoPlaceToColor(int Flag)
{
    //AI(HT160S-Maintainer) 20260625 : return the carried identity tray to the Color rear.
    //Color and Empty are the SAME mechanism (U4), so this mirrors DoPlaceToEmpty exactly,
    //changing only the target X (Color return teach point) and the destination module. The
    //return handshake uses the SAME contract names as Empty : RequestReturnTray() (already
    //called in DecidePlaceDestAfterPick) makes Color go up and free its rear; this places
    //once IsRearHasTray()==false confirms the rear is clear, then signals completion with
    //NotifyTrayXToEmptyFinish(). Clamp/Z cylinders self-alarm on a sensor timeout, so a
    //stuck move is reported rather than silently hanging. MoveTrayArmX holds the Z-up
    //interlock.
    if(Flag==0)
    {
        PlaceTask=1;
        ArmDelay.Clear();
        ClearPlaceGateWatch();   //AI(ht160s-home-resume-p0) 20260710 : new place job = new watchdog window
        return true;
    }

    //AI(ht160s-trayarm-teach-test) 20260627 : physical motion via the shared primitives (mirrors
    //DoPlaceToEmpty). The rear-clear wait stays HERE (case 500) - same order as before; only the
    //lower/release/raise choreography is shared. Internal Task values renumbered, behavior identical.
    switch(PlaceTask)
    {
        case 1:
        case 10:
            //AI(ht160s-tray-source) : reuse the Color pickup X as the return deposit X (same Color
            //station). On-machine confirm whether a distinct deposit X is needed (tray-on-tray
            //clash); a separate teach point belongs with the uOffset teach rework, not Phase 6.
            if(DoMoveToStationZSafe(Teach.TrayXArmToColorXPosition, PlaceTask))
                PlaceTask=500;
            break;

        case 500:
            //AI(ht160s-status) 20260703 : anti-collision (mirrors Empty concern B).
            if(IsSoftSimulate()==false && ColorModule!=NULL && ColorModule->GetStatus()==CS_FEEDING)
            {
                OnPlaceGateBlocked("Color");   //AI(ht160s-home-resume-p0) 20260710 : watchdog tick while blocked
                break;
            }
            //AI(ht160s-agv-identity2d) 20260714 : CONDITIONAL deposit gate. If Color is running the
            //identity INTAKE contract (IsReceivingIdentity) the deposit must wait for Color FULLY idle
            //(IsReadyToReceiveIdentity) so the rear handoff cannot collide with a Color carriage move.
            //A recycle deposit (bReturnTray path, IsReceivingIdentity false) keeps the plain rear-free
            //gate -- recycle REQUIRES Color busy at case 1700, so applying the idle gate would deadlock
            //it. Sim escapes both. (Was: IsRearHasTray()==false || IsSoftSimulate().)
            if( ( (ColorModule!=NULL && ColorModule->IsReceivingIdentity())
                    ? (ColorModule!=NULL && ColorModule->IsReadyToReceiveIdentity())
                    : (ColorModule==NULL || ColorModule->IsRearHasTray()==false) )
                || IsSoftSimulate())
            {
                ClearPlaceGateWatch();   //AI(ht160s-home-resume-p0) 20260710 : gate passed -- close the watchdog window
                Status=TAS_PLACING;   //AI(ht160s-status) 20260703 : deposit ladder starts
                PlaceTask=1000;
            }
            else
                OnPlaceGateBlocked("Color");   //AI(ht160s-home-resume-p0) 20260710 : watchdog tick while blocked
            break;

        case 1000:
        case 2000:
        case 2100:
        case 3000:
            if(DoLowerClampRaise(false, PlaceTask))
                PlaceTask=4000;
            break;

        case 4000:
            //Tell Color the returned identity tray is now parked at its rear (this also
            //marks the rear as having a tray, so it re-enters the supply pool).
            if(ColorModule!=NULL)
                ColorModule->NotifyTrayXToEmptyFinish();
            if(HSys.VMot.MMTrayArmX!=NULL) HSys.VMot.MMTrayArmX->Tray.Clear();   //AI(ht160s-tray-source) : arm empty after return-to-Color (parity with Empty path)
            if(HSys.VMot.MMTrayArmX!=NULL)
                HSys.VMot.MMTrayArmX->fHasTray=false;
            bHasTray=false;
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
void TTrayArmModule::DoTrayArm(int &Task)
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
            HasTray();
            //AI(HT160S-Maintainer) 20260605 : idle. If a tray is unexpectedly still
            //held (e.g. abort residue) do not guess where to drop it : stay idle until
            //a teach/recovery flow exists. Only start a new job when arm is empty.
            if(bHasTray)
            {
                //AI(HT160S-Maintainer) 20260612 : EXCEPTION - if the tray was in hand for
                //a still-valid delivery job that survived a recoverable home (Job!=NONE,
                //destination already chosen), this is not unknown residue : resume placing
                //that same tray so production continues without losing/dropping it.
                if(Job!=TAJOB_NONE)
                {
                    Status=TAS_CARRYING;
                    //AI(ht160s-home-resume-p0) 20260710 : re-sign the return handshake torn by a
                    //full-machine HOME. InitialAllTask(true) keeps this arm's Job/PlaceDest, but
                    //Empty/Color InitialFlag take no bKeepMaterial and wiped bReturnTray, and this
                    //resume path skips DecidePlaceDestAfterPick, so the request was never re-sent :
                    //the receiver refills/keeps its rear while the case-500 rear-clear wait pins the
                    //arm forever (sender remembers the contract, receiver forgot it). RequestReturnTray
                    //is idempotent (sets bReturnTray, clears bTrayXToEmptyFinish) and also drives the
                    //receiver to GoUp-clear an occupied rear, so it heals both the wiped-handshake and
                    //the sensor-relatched-rear cases.
                    //AI(ht160s-home-resume-drain) 20260713 : XS-2 -- do NOT re-arm the
                    //return handshake if the carried tray was already deposited on the
                    //receiver rear before the HOME. Re-sending RequestReturnTray would make
                    //the receiver GoUp and COLLECT the just-placed tray, after which an
                    //empty-jaw deposit signs a phantom NotifyTrayXToEmptyFinish. The DoPlace
                    //adopt fast-forward instead signs the real (present) tray at case 4000.
                    if(IsCarriedTrayAlreadyDeposited()==false)
                    {
                        if(PlaceDest==TAPLACE_EMPTY && EmptyModule!=NULL)
                            EmptyModule->RequestReturnTray();
                        if(PlaceDest==TAPLACE_COLOR && ColorModule!=NULL)
                        {
                            //AI(ht160s-agv-identity2d) 20260714 : preserve an interrupted identity
                            //INTAKE (scan+upload) across HOME -- re-arm the intake contract, not the
                            //recycle, so CEID275/SVID38204 is not silently dropped for this tray.
                            if(iDeliverKind==eTrayKindIdentity)
                                ColorModule->RequestReadIdentityTray();
                            else
                                ColorModule->RequestReturnTray();
                        }
                    }
                    DoPlace(0);
                    Task=2000;
                    break;
                }
#ifndef SOFT_SIMULATE
                //AI(ht160s-home-residue) 20260708 : residue recovery. (1) Un-adopt : once the
                //operator opened the clamps and removed the tray (an enabled clamp On sensor
                //exists and none reads On), release the fHasTray latch so production resumes
                //WITHOUT an application restart -- covers both a sensor-adopted residue and a
                //legacy abort residue removed via Teach. (2) One-shot notify for the adopted
                //residue (silent-stop rule : an arm pinned holding an unidentified tray must
                //tell the operator; unknown Kind/ID means operator removal, MES0924 rationale).
                {
                    bool bFrontEn=HSys.Cyn.C_TrayArm_FrontClamp.OnSensor.Enable;
                    bool bRearEn =HSys.Cyn.C_TrayArm_RearClamp.OnSensor.Enable;
                    bool bAnyOn  =(bFrontEn && HSys.Cyn.C_TrayArm_FrontClamp.OnSensor.IsOn()) ||
                                  (bRearEn  && HSys.Cyn.C_TrayArm_RearClamp.OnSensor.IsOn());
                    if((bFrontEn || bRearEn) && bAnyOn==false)
                    {
                        if(HSys.VMot.MMTrayArmX!=NULL)
                            HSys.VMot.MMTrayArmX->fHasTray=false;
                        bHasTray=false;
                        bResiduePendingNotify=false;
                    }
                    else if(bResiduePendingNotify)
                    {
                        bResiduePendingNotify=false;
                        ShowMyError("MES1722", LangT("TrayArm holds an unidentified tray - open the clamps in Teach and remove it"), K_RETRY);
                    }
                }
#endif
                Status=TAS_IDLE;
                break;
            }
            Job=DecideJob();
            if(Job==TAJOB_NONE)
            {
                Status=TAS_IDLE;
                break;
            }
            Status=TAS_PICKING;
            DoPick(0);
            Task=1000;
            break;

        case 1000:
            //AI(ht160s-rearready-p0) 20260705 : abandon a Loader-recovery job whose
            //source was emptied while the arm still waits at the pick gate (PickTask
            //1/10, Z-UP, nothing grabbed yet). This is the MES0924 remedy path : the
            //operator removed the un-preservable leftover, so there is nothing to
            //recover -- without this the job stays latched (DecideJob is only re-run
            //from Task 100), the arm stays pinned here until the NEXT discharge, and
            //in Run_CleanOut (no next discharge ever comes) the cascade hangs silently
            //until a full HOME. Gate on PickTask<1000 : once the physical grab ladder
            //started, the tray leaves the rear through case 4000's own handoff, so a
            //sensor-empty read past that point is the pick itself, not a removal. A
            //transient sensor flicker only churns : DecideJob re-reads the sensor next
            //tick and re-dispatches, and DoPick re-enters through the same Z-safe move.
            if(Job==TAJOB_LOADER_RECOVERY && PickTask<1000 &&
               LoaderModule!=NULL && LoaderModule->IsRearHasTray()==false)
            {
                PickWaitTimer.Clear();   //AI(ht160s-rearready-p0) 20260705 : abandoned wait -- disarm so a State Record never shows an armed watchdog on an idle arm (next dispatch's DoPick(0) would reset it anyway)
                bPickWaitArmed=false;
                dwPickGateLastPollTick=0;
                Status=TAS_IDLE;
                Job=TAJOB_NONE;
                Task=100;
                break;
            }
            if(DoPick(1))
            {
                Status=TAS_CARRYING;
                //AI(HT160S-Maintainer) 20260606 : a Loader-recovery tray now in hand : decide
                //here whether to supply an Auto or recycle to EmptyTray. Supply jobs always
                //place to their pre-chosen Auto.
                if(Job==TAJOB_LOADER_RECOVERY)
                    DecidePlaceDestAfterPick();
                else
                    PlaceDest=TAPLACE_AUTO;
                DoPlace(0);
                Task=2000;
            }
            break;

        case 2000:
            if(DoPlace(1))
            {
                Status=TAS_IDLE;
                Job=TAJOB_NONE;
                Task=100;
            }
            break;
        default:
            //AI(ht160s-ladder-guard) 20260703 : a state number with no matching case
            //(the 'number but no action' trap). Log it so a future dead-jump is a
            //diagnosable EventLog event, not a silent stall, and restart the ladder.
            LogLadderFault("TrayArm.DoTrayArm", Task);
            Task=1;
            break;
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-trayarm-teach-test) 20260627 : Teach Advanced TrayArm test support below.
//Flat channel index (eTrayArmChannel) -> per-station handoff X teach point. Grab sources are
//Empty/Color/Loader; place targets are Auto1-6 plus recycle Empty/Color. Auto ids reuse the
//existing GetAutoX(0..5) resolver so the six Auto teach points are addressed by index.
int TTrayArmModule::GetChannelHandoffX(int Channel)
{
    switch(Channel)
    {
        case TACH_EMPTY:  return Teach.TrayXArmToEmptyXPosition;
        case TACH_COLOR:  return Teach.TrayXArmToColorXPosition;
        case TACH_LOADER: return Teach.TrayXArmToLoaderXPosition;
        case TACH_AUTO1:
        case TACH_AUTO2:
        case TACH_AUTO3:
        case TACH_AUTO4:
        case TACH_AUTO5:
        case TACH_AUTO6:
            return GetAutoX(Channel-TACH_AUTO1);
    }
    return Teach.TrayXArmToEmptyXPosition;
}
//---------------------------------------------------------------------------
AnsiString TTrayArmModule::GetChannelName(int Channel)
{
    switch(Channel)
    {
        case TACH_EMPTY:  return "Empty";
        case TACH_COLOR:  return "Color";
        case TACH_LOADER: return "Loader";
        case TACH_AUTO1:  return "Auto1";
        case TACH_AUTO2:  return "Auto2";
        case TACH_AUTO3:  return "Auto3";
        case TACH_AUTO4:  return "Auto4";
        case TACH_AUTO5:  return "Auto5";
        case TACH_AUTO6:  return "Auto6";
    }
    return "?";
}
//---------------------------------------------------------------------------
bool TTrayArmModule::ChannelPlaceClear(int Channel)
{
    //AI(ht160s-trayarm-teach-test) 20260627 : anti-clash gate - the place destination rear must
    //be clear before lowering/releasing onto it. Empty/Color expose IsRearHasTray(); Auto exposes
    //a per-index IsRearHasTray(Index), so block the place test when the target rear already holds a
    //tray (would otherwise Z-down and Pop the clamps onto the existing tray = tray-on-tray clash).
    switch(Channel)
    {
        case TACH_EMPTY:  return (EmptyModule==NULL || EmptyModule->IsRearHasTray()==false);
        case TACH_COLOR:  return (ColorModule==NULL || ColorModule->IsRearHasTray()==false);
        case TACH_AUTO1:
        case TACH_AUTO2:
        case TACH_AUTO3:
        case TACH_AUTO4:
        case TACH_AUTO5:
        case TACH_AUTO6:
            return (AutoModule==NULL || AutoModule->IsRearHasTray(Channel-TACH_AUTO1)==false);
    }
    return true;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::CanTestTrayArm(int Channel, bool bGrab, AnsiString &Err)
{
    //AI(ht160s-trayarm-teach-test) 20260627 : parametric gate for the Teach TrayArm test. The
    //hard Z-up-before-X interlock is enforced inside MoveTrayArmX on every move; require it true
    //up front too so the test refuses to start with the head lowered. Grab is allowed on an empty
    //source (pure dry-run); place is blocked when the destination rear already holds a tray.
    Err="";
    if(Channel<0 || Channel>=TACH_COUNT)
    {
        Err="Invalid channel index";
        return false;
    }
    bool bAuto=(Channel>=TACH_AUTO1 && Channel<=TACH_AUTO6);
    if(bGrab && bAuto)
    {
        Err="Auto stations are place targets, not grab sources";
        return false;
    }
    if(bGrab==false && Channel==TACH_LOADER)
    {
        Err="Loader is a grab source only (TrayArm never places into the Loader)";
        return false;
    }
    if(IsZUpAtPosition()==false)
    {
        Err="TrayArm Z lift is not at the UP position (Z-up interlock)";
        return false;
    }
    if(bGrab==false && ChannelPlaceClear(Channel)==false)
    {
        Err=GetChannelName(Channel)+" rear already holds a tray (clear it first)";
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::TestGrabFromChannel(int Channel, int &Task)
{
    //AI(ht160s-trayarm-teach-test) 20260627 : pure-motion grab dry-run. Move to the channel
    //handoff X, lower, clamp, raise - the SAME primitives production DoPick uses. Deliberately
    //does NOT call the source module's pick notify (NotifyTrayPicked / SetRearHasTray /
    //NotifyTrayArmPickRearTray) or transfer the tray grid, so it never mutates tray-tracking
    //state and can be run repeatedly. Caller inits Task=1.
    switch(Task)
    {
        case 1:
        case 10:
            if(DoMoveToStationZSafe(GetChannelHandoffX(Channel), Task))
                Task=1000;
            break;

        case 1000:
        case 2000:
        case 2100:
        case 3000:
            if(DoLowerClampRaise(true, Task))
                return true;
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::TestPlaceToChannel(int Channel, int &Task)
{
    //AI(ht160s-trayarm-teach-test) 20260627 : pure-motion place dry-run. Move to the channel
    //handoff X, lower, release, raise - the SAME primitives production DoPlace uses. Deliberately
    //does NOT call the destination module's deliver notify (StageRearGrid / NotifyTrayArm-
    //Delivered / SetRearHasTrayFromTrayArm / NotifyTrayXToEmptyFinish) or the recycle rear-clear
    //handshake, so it never mutates tray-tracking state and can be run repeatedly. Caller inits Task=1.
    switch(Task)
    {
        case 1:
        case 10:
            if(DoMoveToStationZSafe(GetChannelHandoffX(Channel), Task))
                Task=1000;
            break;

        case 1000:
        case 2000:
        case 2100:
        case 3000:
            if(DoLowerClampRaise(false, Task))
                return true;
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
void InitializeTrayArmModule()
{
    if(TrayArmModule==NULL)
        TrayArmModule=new TTrayArmModule;
}
//---------------------------------------------------------------------------
void ShutdownTrayArmModule()
{
    delete TrayArmModule;
    TrayArmModule=NULL;
}
//---------------------------------------------------------------------------