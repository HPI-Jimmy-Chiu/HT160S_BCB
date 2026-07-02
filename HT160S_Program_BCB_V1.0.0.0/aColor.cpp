#include "IncludeAllHeader.h"       //Dell 將.h統一,可加速build
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "language.h"

#include "aColor.h"
#include "database.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "GeneralSetting.h"
#include "uteach.h"
#include "ColorCcdSocket.h"
#include "aTrayArm.h"   //AI(cleanout) 20260701 : TrayArmModule->IsCleanOutFinish() gates the Color CleanOut drain/finish
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TColorModule *ColorModule=NULL;
//---------------------------------------------------------------------------
TColorModule::TColorModule()
{
    iMode=eHT160ColorModeTraySupply;
    iSupplyThreshold=100;
    InitialFlag();
}
//---------------------------------------------------------------------------
void TColorModule::InitialFlag()
{
    bAmrLocked=false;
    bWaitingAmrFeed=false;     //AI(ht160s-agv) 20260627 : clear Color source-dry AMR wait (P4)
    AmrFeedWaitTimer.Clear();  //AI(ht160s-agv) 20260627 : clear Color source-dry AMR wait timer (P4)
    RefillSimInfeed();
    FeedTask=1;
    FeedClampSub=0;
    SortBinTask=1;
    iICCount=0;
    bInputFullTray=false;
    bRearHasTray=false;
    bTrayReady=false;
    bSupplyRequested=false;
    bFrontHasTray=false;
    if(HSys.VMot.MMColorY!=NULL) HSys.VMot.MMColorY->ClearTray();   //AI(ht160s-tray-source) : hide Color grid on init
    FrontSourceTray.Clear();   //AI(ht160s-color-align-empty) : no stale front-born grid across init/lot
    ScanTask=1;
    GoDownTask=1;
    //AI(phase6-loader-recycle) 20260625 : Color receive-tray flow (mirrors Empty).
    GoUpTask=1;
    bReturnTray=false;
    bTrayXToEmptyFinish=false;
    iReturnedCount=0;
    GoUpDelay.Clear();
    sTrayID2D="";
    FeedDelay.Clear();
    GoDownDelay.Clear();
    ScanDelay.Clear();
    TestUpTask=1;
    TestDownTask=1;
    TestDelay.Clear();
}
//---------------------------------------------------------------------------
//AI(ht160s-actuator-timer) 20260627 : freeze/thaw this module's wall-clock timeout
//windows (ScanDelay CCD shot + AmrFeedWaitTimer source-dry AMR wait MES1421) so a machine pause taken mid-scan is not
//charged against the timeout budget -- no false scan-timeout on resume. Called from
//csystem PauseActuatorTimeoutTimers/ReStartActuatorTimeoutTimers on the SystemStart
//pause/resume edges, alongside Cylinder[]/SortArmSuck. Add future Color timeout
//timers here; csystem needs no change.
void TColorModule::PauseTimeoutTimers()
{
    ScanDelay.Pause();
    AmrFeedWaitTimer.Pause();
}
//---------------------------------------------------------------------------
void TColorModule::ReStartTimeoutTimers()
{
    ScanDelay.ReStart();
    AmrFeedWaitTimer.ReStart();
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : AMR P3 (ColorTray) handoff interface, mirrors TAutoModule.
void TColorModule::SetAmrLock(bool bLock)
{
    bAmrLocked=bLock;
}
//---------------------------------------------------------------------------
bool TColorModule::IsAmrLocked()
{
    return bAmrLocked;
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : Ready = front stacking cylinders back home (not commanded
//up); destack idle so the AGV may refill. Held stable by bAmrLocked.
bool TColorModule::IsReadyForAmrHandoff()
{
    //AI(ht160s-agv) 20260625 : sim defense-in-depth - in SOFT_SIMULATE the front
    //destacker out-bits are normally already false; assert ready so the PREP->READY
    //gate cannot latch the lock on a laptop run. Real hardware keeps the interlock.
    if(IsSoftSimulate())
        return true;
    return (HSys.Cyn.C_Color_FrontRiseTray_1.GetOutBit()==false
            && HSys.Cyn.C_Color_FrontRiseTray_2.GetOutBit()==false
            && HSys.Cyn.C_Color_FrontSeparateTray_1.GetOutBit()==false);
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : shortage (call AGV). Sim drains iSimInfeedCount to 0; real
//reads SnColor_InputEnd (ON=has tray, OFF=empty). Disabled sensor -> no call.
bool TColorModule::IsInputShortageForAmr()
{
    if(IsSoftSimulate())
        return (iSimInfeedCount<=0);
    return (HSys.Sen.SnColor_InputEnd.Enable==true && HSys.Sen.SnColor_InputEnd.IsOff());
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : Finish = refill complete. Sim auto-completes (no sensor);
//real waits for SnColor_InputEnd to read a tray present (ON).
bool TColorModule::IsInputHandoffFinishedForAmr()
{
    if(IsSoftSimulate())
        return true;
    return (HSys.Sen.SnColor_InputEnd.Enable==true && HSys.Sen.SnColor_InputEnd.IsOn());
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : reset sim input-stack to configured max (AGV delivered a
//full magazine). Real machine ignores the count (sensor-driven).
void TColorModule::RefillSimInfeed()
{
    iSimInfeedCount=GeneralSetting.iSimAmrMaxTray[2];
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260624 : trays currently on the Color supply car (PanelMain6 header).
//Sim drains per destack; real machine sensor-driven (count not maintained, reads max).
int TColorModule::GetCarTrayCount()
{
    return iSimInfeedCount;
}
//---------------------------------------------------------------------------
bool TColorModule::IsSoftSimulate()
{
    #ifdef SOFT_SIMULATE
    return true;
    #else
    return (HSys.LastSet.iRealDummy==DUMMY);
    #endif
}
//---------------------------------------------------------------------------
bool TColorModule::IsInstalled()
{
    return GeneralSetting.bColorBinAreaInstalled;
}
//---------------------------------------------------------------------------
void TColorModule::RefreshStateFromSensors()
{
    bool bHasInputSensor=false;
    bool bHasOutputSensor=false;
    bool bOutputState=false;

    if(IsInstalled()==false)
    {
        bInputFullTray=false;
        bFrontHasTray=false;
        bRearHasTray=false;
        bTrayReady=false;
        return;
    }

    //AI(ht160s-color-align-empty) 20260629 : MIRROR Empty RefreshStateFromSensors sim early-out.
    //In SOFT_SIMULATE / real-machine DUMMY there is no IO card, so InType=0 (active-low) HasTray
    //inputs all read present and clobber bRearHasTray=true; the DoFeedTray case 10 leftover-tray
    //guard then silently aborts every supply (MES1425 suppressed under DUMMY), so Color never
    //presents an identity tray and the AMR TrayArm/SortArm hang. In sim/dummy tray state is a
    //LATCH owned by the action ladders (DoGoDownTray->bFrontHasTray, DoFeedTray case7000->
    //bTrayReady, receive path->bRearHasTray).
    if(IsSoftSimulate())
        return;

    if(HSys.Sen.SnColor_InputHasTray.Enable==true)
    {
        bHasInputSensor=true;
        //AI(ht160s-color-align-empty) 20260627 : front-staged presence is SENSOR-driven,
        //mirroring TEmptyModule (bFrontHasTray=SnEmpty_InputHasTray.IsOn()). SnColor_InputHasTray
        //is the same hardware role as SnEmpty_InputHasTray, so bFrontHasTray tracks it directly
        //(was a logical-only latch -- the divergence from Empty).
        bFrontHasTray=HSys.Sen.SnColor_InputHasTray.IsOn();
    }

    if(HSys.Sen.SnColor_InputFullTray.Enable==true)
    {
        bInputFullTray=HSys.Sen.SnColor_InputFullTray.IsOn();
        if(bInputFullTray)
            bFrontHasTray=true;
    }
    else
        bInputFullTray=false;

    if(HSys.Sen.SnColor_OutputBottomHasTray.Enable==true)
    {
        bHasOutputSensor=true;
        if(HSys.Sen.SnColor_OutputBottomHasTray.IsOn())
            bOutputState=true;
    }

    if(HSys.Sen.SnColor_TrayPos1.Enable==true)
    {
        bHasOutputSensor=true;
        if(HSys.Sen.SnColor_TrayPos1.IsOn())
            bOutputState=true;
    }

    if(bHasOutputSensor)
        bRearHasTray=bOutputState;
    else if(IsSoftSimulate())
    {
        //AI(phase6-loader-recycle) 20260625 : while a receive (return) is in progress
        //bRearHasTray is the rear-handoff LATCH owned by the receive ladder (set by
        //NotifyTrayXToEmptyFinish, cleared by DoGoUpTray). Do NOT clobber it from the
        //sim bTrayReady fallback here, or the latch is wiped and the TrayArm deposit /
        //DoGoUpTray handshake breaks (Empty avoids this by returning early in sim).
        if(bReturnTray==false && bTrayXToEmptyFinish==false)
            bRearHasTray=bTrayReady;
    }

    if(bHasInputSensor==false && IsSoftSimulate())
        bFrontHasTray=true;

    //AI(general) 20260609 : Removed the sensor-driven bTrayReady latch. It set
    //bTrayReady=true from any output-sensor read and never cleared it (only
    //DoReleaseTray did), so IsTrayReady() stayed latched true and DoColor case 100
    //always folded back to idle, never reaching the supply ladder. bTrayReady is
    //now owned solely by the supply ladder (DoFeedTray case 7000 sets it,
    //DoReleaseTray clears it), mirroring Empty's just-in-time ready model.
}
//---------------------------------------------------------------------------
bool TColorModule::PushCylinder(TMyCylinder &Cyn)
{
    if(IsSoftSimulate())
        return true;
    if(Cyn.Enable==false)
        return true;
    return Cyn.Push();
}
//---------------------------------------------------------------------------
bool TColorModule::PopCylinder(TMyCylinder &Cyn)
{
    if(IsSoftSimulate())
        return true;
    if(Cyn.Enable==false)
        return true;
    return Cyn.Pop();
}
//---------------------------------------------------------------------------
void TColorModule::DoColor(int &Task)
{
    if(IsInstalled()==false)
    {
        Task=1;
        return;
    }

    switch(Task)
    {
        case 1:
            Task=10;
            break;

        case 10:
            RefreshStateFromSensors();
            if(IsSortBinMode())
            {
                DoSortBin(0);
                Task=2000;
                break;
            }
            Task=100;
            break;

        case 100:
            RefreshStateFromSensors();
            //AI(phase6-loader-recycle) 20260625 : single-MColorY arbitration (CRITICAL).
            //The receive (return) dispatch is placed BEFORE the supply godown/supply
            //branches. Under sim IsSoftSimulate() is always true, so the godown branch
            //below would otherwise win every idle pass and starve the return. Because
            //DoColor owns a single Task at a time, once we enter the return ladder
            //(Task=1700) no supply branch can run until DoGoUpTray completes, so the
            //return cannot deadlock against supply. The bReturnTray latch is held until
            //the deposit finishes (set in RequestReturnTray, cleared in case 1700).
            if(bReturnTray)
            {
                //AI(ht160s-agv) 20260625 : DoGoUpTray also raises C_Color_FrontRise*/Separate,
                //so hold it off while AMR-locked so the front stack stays home and the CEID273
                //READY gate (IsReadyForAmrHandoff) can clear. Return resumes after the lock clears.
                if(bAmrLocked)
                {
                    Task=1;
                    break;
                }
                DoGoUpTray(0);
                Task=1700;
                break;
            }
            //AI(cleanout) 20260701 : CleanOut drain phase. Once TrayArm has finished, Color stops
            //supplying/destacking and GoUp-drains every tray back to the car (reuses the receive
            //ladder at case 1700). Owns case 100 while draining so no GoDown/feed runs. Before
            //TrayArm finishes (produce phase) Color supplies normally; outside CleanOut it is a no-op.
            if(HSys.Sys.RunMode==Run_CleanOut &&
               TrayArmModule!=NULL && TrayArmModule->IsCleanOutFinish())
            {
                if(bFrontHasTray || bRearHasTray)
                {
                    DoGoUpTray(0);
                    Task=1700;
                }
                else
                    Task=1;
                break;
            }
            //AI(ht160s-color-align-empty) 20260627 : pickup release is now single-step in
            //NotifyTrayPicked (mirrors TEmptyModule SetRearHasTray(false)); no separate
            //DoReleaseTray pass. While a tray is presented at the rear, idle until it is picked.
            if(bTrayReady)
            {
                Task=1;
                break;
            }
            //AI(HT160S-Maintainer) 20260608 : two-stage supply like Empty. Stage 1
            //keeps the front buffer filled (separate one tray off the stack via
            //DoGoDownTray). Stage 2 pushes that staged tray to the output and reads
            //its 2D code, but only when an AMR supply was actually requested (or
            //simulating), so the identity tray is not presented / scanned early.
            if(bFrontHasTray==false && bReturnTray==false)
            {
                //AI(ht160s-agv) 20260625 : NARROW AMR lock. The FRONT-stack branches that
                //raise C_Color_FrontRise*/Separate (this DoGoDownTray, and the bReturnTray
                //DoGoUpTray above) are held off while the AGV refills the front stack, so
                //IsReadyForAmrHandoff stays true and the lock can clear. The downstream
                //branches (bTrayReady->DoReleaseTray, bSupplyRequested->DoFeedTray) keep
                //running so TrayArm/SortArm are not starved.
                if(bAmrLocked)
                {
                    Task=1;
                    break;
                }
                //AI(ht160s-color-align-empty) 20260626 : mirror TEmptyModule::DoEmpty
                //case 100 - always destack a front tray when the front buffer is empty
                //(no bInputHasTray pre-gate). Color == Empty + CCD : identity is stamped
                //by DoReadColor2D (real scan, or COLOR2D_ when CCD-off/HAS_TRAY/sim); a
                //truly empty real magazine is caught downstream by the output-sensor
                //MES1421 at DoFeedTray case 7000, so producing here can never silently hang.
                DoGoDownTray(0);
                Task=1200;
                break;
            }
            if(bSupplyRequested && bReturnTray==false)
            {
                DoFeedTray(0);
                Task=1000;
            }
            else
                Task=1;
            break;

        case 1000:
            if(DoFeedTray(1))
                Task=1;
            break;

        case 1200:
            if(DoGoDownTray(1))
            {
                if(IsSoftSimulate() && iSimInfeedCount>0)
                    iSimInfeedCount--;   //AI(ht160s-agv) sim input drains 1/GoDown
                Task=1;
            }
            break;

        case 1700:
            //AI(phase6-loader-recycle) 20260625 : Color receive ladder (mirrors
            //TEmptyModule::DoEmpty case 3000). Hold here until the TrayArm has finished
            //depositing the returned tray onto Color's rear (NotifyTrayXToEmptyFinish
            //sets bTrayXToEmptyFinish + bRearHasTray). Then DoGoUpTray stacks it back
            //onto the front car. On completion the returned identity tray re-enters the
            //supply pool (iSimInfeedCount++); a later DoFeedTray -> DoReadColor2D ->
            //StampReadIdentity2D reproduces Kind=Identity + TrayID (no new code).
            if(DoGoUpTray(1))
            {
                if(bReturnTray && bTrayXToEmptyFinish==false)
                    return;
                bReturnTray=false;
                iReturnedCount++;
                iSimInfeedCount++;
                Task=1;
            }
            break;

        case 2000:
            if(DoSortBin(1))
                Task=1;
            break;
    }
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260608 : separate one tray off the front stack and stage
//it at the front, mirroring TEmptyModule::DoGoDownTray. Sets bFrontHasTray on
//success so DoFeedTray can later push it to the output. Kept distinct so the
//front buffer can be pre-staged (Empty-style pipelining) while the AMR pull request
//is still pending. Color has no dedicated front-staging sensor, so staging is
//logical; physical presence is confirmed at the output (DoFeedTray case 7000).
bool TColorModule::DoGoDownTray(int Flag)
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
            if(IsTraySupplyMode()==false)
                return true;
            GoDownTask=10;
            break;

        case 10:
            RefreshStateFromSensors();
            if(bFrontHasTray)
                return true;
            //AI(ht160s-color-align-empty) 20260626 : no bInputHasTray pre-abort here
            //(mirrors TEmptyModule::DoGoDownTray case 10) - always destack; identity is
            //supplied by DoReadColor2D and a missing real tray is caught by MES1421.
            GoDownTask=100;
            break;

        case 100:
            //AI(HT160S-Maintainer) 20260608 : dual destacker, mirror Empty. Rise_1 up.
            if(PushCylinder(HSys.Cyn.C_Color_FrontRiseTray_1))
                GoDownTask=150;
            break;

        case 150:
            if(PushCylinder(HSys.Cyn.C_Color_FrontRiseTray_2))
                GoDownTask=200;
            break;

        case 200:
            if(PushCylinder(HSys.Cyn.C_Color_FrontSeparateTray_1))
            {
                GoDownDelay.SetMS(GeneralSetting.iColorDestackSettleMs);
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
            //dropping the whole stack (mirrors the Empty godown noise report; identical pattern).
            if(GoDownDelay.Off())
            {
                if(PopCylinder(HSys.Cyn.C_Color_FrontRiseTray_2))
                {
                    GoDownDelay.SetMS(GeneralSetting.iColorDestackSettleMs);
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
            if(PopCylinder(HSys.Cyn.C_Color_FrontSeparateTray_1))
            {
                GoDownDelay.SetMS(GeneralSetting.iColorDestackSettleMs);
                GoDownDelay.On();
                GoDownTask=450;
            }
            break;

        case 450:
            if(GoDownDelay.Off())
                GoDownTask=500;
            break;

        case 500:
            if(PopCylinder(HSys.Cyn.C_Color_FrontRiseTray_1))
            {
                GoDownDelay.SetMS(GeneralSetting.iColorDestackSettleMs);
                GoDownDelay.On();
                GoDownTask=600;
            }
            break;

        case 600:
            if(GoDownDelay.Off())
                GoDownTask=700;
            break;

        case 700:
            //AI(ht160s-color-align-empty) 20260627 : confirm the front-staged tray on
            //SnColor_InputHasTray, mirroring TEmptyModule::DoGoDownTray case 7000. Front
            //presence is now sensor-driven, so confirm here (after a settle) before declaring
            //success -- else RefreshStateFromSensors could read the not-yet-settled sensor as
            //empty and re-trigger a destack. Miss -> alarm + retry (Empty uses MES1024).
            RefreshStateFromSensors();
            if(HSys.Sen.SnColor_InputHasTray.Enable==true &&
               HSys.Sen.SnColor_InputHasTray.IsOff() &&
               HSys.LastSet.iRealDummy!=DUMMY)
            {
                bFrontHasTray=false;
                Ret=ShowMyError("MES1424", LangT("Color front supply tray is missing"), &HSys.Sen.SnColor_InputHasTray, true, K_RETRY);
                if(Ret==K_RETRY)
                    GoDownTask=1;
            }
            else
            {
                bFrontHasTray=true;
                BirthFrontTray();   //AI(ht160s-color-align-empty) : identity tray born at front (empty 2D; CCD updates later), mirror Empty
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(phase6-loader-recycle) 20260625 : stack a returned identity tray (sitting at the
//rear handoff position) back onto the front supply car, then re-stage the front. This
//is a near-verbatim port of TEmptyModule::DoGoUpTray (U4 : Empty and Color share the
//destacker mechanism). Cylinder names C_Empty_*->C_Color_*, MoveEmptyY->MoveColorY,
//Empty rear/front teach Y -> Color rear (ColorTrayArmPickYPosition) / front
//(ColorReceiveTrayYPosition; ColorRead2DYPosition is now the middle CCD scan Y). The
//rear-occupied latch is bRearHasTray (renamed from bOutputHasTray to match Empty; the
//output/read position is Color's rear handoff slot). The Color rear
//riser C_Color_RearRiseTray is intentionally NOT pushed here: its use as a receive
//stacking cylinder is mechanism-unconfirmed (plan section 0 item 2). The front
//destacker port is the working core; RearRiseTray is left out for now.
bool TColorModule::DoGoUpTray(int Flag)
{
    if(Flag==0)
    {
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
            HSys.Cyn.C_Color_FrontRiseTray_1.On();
            GoUpTask=200;
            break;

        case 200:
            if(HSys.Cyn.C_Color_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Color_FrontSeparateTray_1.On();
                GoUpDelay.SetMS(GeneralSetting.iColorDestackSettleMs);
                GoUpDelay.On();
                GoUpTask=300;
            }
            break;

        case 300:
            if(GoUpDelay.Off())
            {
                HSys.Cyn.C_Color_FrontRiseTray_2.On();
                GoUpTask=400;
            }
            break;

        case 400:
            if(HSys.Cyn.C_Color_FrontRiseTray_2.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Color_FrontSeparateTray_1.Off();
                GoUpDelay.SetMS(GeneralSetting.iColorDestackSettleMs);
                GoUpDelay.On();
                GoUpTask=500;
            }
            break;

        case 500:
            if(GoUpDelay.Off())
            {
                HSys.Cyn.C_Color_FrontRiseTray_2.Off();
                if(HSys.Cyn.C_Color_FrontRiseTray_1.IsOn() || IsSoftSimulate())
                    GoUpTask=600;
            }
            break;

        case 600:
            if(HSys.Cyn.C_Color_FrontRiseTray_1.Pop() || IsSoftSimulate())
            {
                bFrontHasTray=false;
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
            //AI(phase6-loader-recycle) 20260625 : carry the rear/output tray to the rear
            //pickup Y (Color's rear handoff Y) before leaning/pushing it onto the car.
            if(MoveColorY(Teach.ColorTrayArmPickYPosition))
                GoUpTask=3000;
            break;

        case 3000:
            if(HSys.Cyn.C_Color_LeanOnTray.Push() || IsSoftSimulate())
                GoUpTask=4000;
            break;

        case 4000:
            if(HSys.Cyn.C_Color_PushTray.Push() || IsSoftSimulate())
                GoUpTask=5000;
            break;

        case 5000:
            //AI(ht160s-color-3pos) 20260626 : return the carriage to the FRONT car Y. The
            //"front car" = the RECEIVE position (Teach.ColorReceiveTrayYPosition), not the
            //CCD scan Y. ColorRead2DYPosition was repurposed to the middle scan station.
            if(MoveColorY(Teach.ColorReceiveTrayYPosition))
                GoUpTask=6000;
            break;

        case 6000:
            if(HSys.Cyn.C_Color_PushTray.Pop() || IsSoftSimulate())
                GoUpTask=7000;
            break;

        case 7000:
            if(HSys.Cyn.C_Color_LeanOnTray.Pop() || IsSoftSimulate())
            {
                bFrontHasTray=true;
                bRearHasTray=false;
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
bool TColorModule::MoveColorY(int Position)
{
    //AI(HT160S-Maintainer) 20260622 : move the Color carriage in Y (front/back relative to the
    //operator; X=left/right, Z=up/down). Ported VERBATIM from MoveEmptyY (same mechanism; Color
    //only adds the middle CCD scan). Returns false on NULL / out-of-limit / TrayArm-collision so
    //the caller's ladder WAITS instead of advancing as if the move had completed.
    if(HSys.Mot.MColorY==NULL)
        return false;
    if(HSys.Mot.MColorY->CheckSoftLimit(Position)==false)
    {
        ShowMotorLimitError(HSys.Mot.MColorY->AlarmName[eMotOverLimitErr], LangT("Color Y motor will out of limit"), HSys.Mot.MColorY, Position);
        return false;
    }

    #ifndef SOFT_SIMULATE
    //AI(ht160s-color-align-empty) 20260627 : TrayArm anti-collision, ported verbatim from
    //MoveEmptyY. If the TrayArm is NOT raised (Z-up off) and its X is at/near the Color pickup X,
    //block the carriage Y move so the carriage cannot slam into the lowered arm.
    int TrayArmPos=0;
    if(HSys.Mot.MTrayArmX!=NULL)
        TrayArmPos=HSys.Mot.MTrayArmX->ReadEncoderPos();
    if(HSys.Cyn.C_TrayArmZ_Up.IsOn()==false &&
       (TrayArmPos+500)>=Teach.TrayXArmToColorXPosition)
    {
        return false;
    }
    #endif

    return HSys.Mot.MColorY->MotorMove(Position);
}
//---------------------------------------------------------------------------
bool TColorModule::MoveColorCcdX(int Position)
{
    //AI(ht160s-color-ccd-xy) 20260628 : move the Color CCD reader (stepper, X axis) to a taught
    //position. Mirrors TLoaderModule::MoveTopCcdX. Returns false on NULL / out-of-limit so the
    //caller's ladder WAITS instead of advancing as if the move had completed.
    if(HSys.Mot.MTopCCDX_Color==NULL)
        return false;
    if(HSys.Mot.MTopCCDX_Color->CheckSoftLimit(Position)==false)
    {
        ShowMotorLimitError(HSys.Mot.MTopCCDX_Color->AlarmName[eMotOverLimitErr], LangT("Color CCD X motor will out of limit"), HSys.Mot.MTopCCDX_Color, Position);
        return false;
    }
    return HSys.Mot.MTopCCDX_Color->MotorMove(Position);
}
//---------------------------------------------------------------------------
bool TColorModule::MoveColorCcdToScan()
{
    //AI(ht160s-color-ccd-xy) 20260628 : move the carriage Y and the CCD reader X to the taught
    //photo position TOGETHER (both commanded every call; returns true only when BOTH are in
    //position). Mirrors TLoaderModule::MoveToCcdCell (parallel X+Y). Shared by production
    //DoFeedTray case 3000 AND the Teach Advanced Color CCD photo test, so both move identically.
    //MoveColorY keeps its soft-limit + (real-build) TrayArm anti-collision guard.
    bool bYFlag=MoveColorY(Teach.ColorRead2DYPosition);
    bool bXFlag=MoveColorCcdX(Teach.ColorRead2DXPosition);
    return (bYFlag && bXFlag);
}
//---------------------------------------------------------------------------
bool TColorModule::DoFeedTray(int Flag)
{
    int Ret;

    if(Flag==0)
    {
        FeedTask=1;
        FeedClampSub=0;
        FeedDelay.Clear();
        //AI(ht160s-color-align-empty) 20260627 : one-shot Reset clears a stale Push() Task
        //left by an aborted supply (alarm/skip), mirroring TEmptyModule::DoFeedTray. Do NOT
        //Reset before each Push() poll -- that restarts the non-blocking machine and hangs.
        HSys.Cyn.C_Color_LeanOnTray.Reset();
        HSys.Cyn.C_Color_PushTray.Reset();
        return true;
    }

    switch(FeedTask)
    {
        case 1:
            if(IsTraySupplyMode()==false)
                return true;
            FeedTask=10;
            break;

        case 10:
            //AI(ht160s-color-align-empty) 20260627 : mirror TEmptyModule::DoFeedTray case 10 --
            //if the rear handoff slot is occupied, do not feed. In the supply context bTrayReady
            //is false here (DoColor idles on bTrayReady otherwise) and bReturnTray is false (the
            //return branch ran first), so a true bRearHasTray means a LEFTOVER tray stranded at
            //the rear on startup/recovery -- not pickable (pickup gate is bTrayReady) and not
            //re-stageable. Require the operator to remove it; the rear sensor clears bRearHasTray
            //when they do. Avoids the silent cold-start deadlock WITHOUT auto-presenting
            //(bRearHasTray is shared by the return/recycle path, so a sensor-backed pickup would
            //mis-fire). Once the sensor reads empty the normal supply proceeds.
            RefreshStateFromSensors();
            if(bRearHasTray)
            {
                if(HSys.LastSet.iRealDummy!=DUMMY)
                {
                    ShowMyError("MES1426", LangT("Color rear has a leftover tray - please remove it"), K_RETRY);
                    RefreshStateFromSensors();
                }
                if(bRearHasTray)
                    return true;
            }
            FeedTask=1000;
            break;

        case 1000:
            //AI(ht160s-color-align-empty) 20260627 : carriage to the FRONT receive Y
            //(Teach.ColorReceiveTrayYPosition = Empty's EmptyCarFeedTrayYPosition analog).
            if(MoveColorY(Teach.ColorReceiveTrayYPosition))
                FeedTask=2000;
            break;

        case 2000:
        {
            //AI(ht160s-color-align-empty) 20260627 : standardized dual-cylinder clamp via
            //DoClampTray (lean-stop first, push last), shared with Empty. SettleTicks=0
            //keeps Color's behavior; the rear output sensor at case 7000 verifies the tray.
            int Clamp=DoClampTray(HSys.Cyn.C_Color_LeanOnTray, HSys.Cyn.C_Color_PushTray,
                                  FeedClampSub, FeedDelay, IsSoftSimulate(), GeneralSetting.iColorFeedClampSettleMs);
            if(Clamp==1)
            {
                //AI(ht160s-color-align-empty) 20260627 : carriage clamped the front-staged tray
                //-> hand the front-born identity grid onto the carriage motor (mirror Empty
                //MoveFrom). The CCD photo step (case 3100) then only UPDATES its 2D TrayID.
                if(HSys.VMot.MMColorY!=NULL)
                {
                    HSys.VMot.MMColorY->Tray.MoveFrom(FrontSourceTray);
                    HSys.VMot.MMColorY->fHasTray=true;
                    HSys.VMot.MMColorY->Refresh();
                }
                FeedTask=3000;
            }
            else if(Clamp==2)
            {
                //AI(ht160s-color-align-empty) 20260628 : push miss -- helper already Popped
                //the push + reset FeedClampSub. Color-own code (NOT Empty JAM1030) + retry.
                Ret=ShowMyError("MES1422", LangT("Color Push Tray Miss"), &HSys.Cyn.C_Color_PushTray.OnSensor, true, K_RETRY);
                if(Ret==K_RETRY)
                    FeedTask=1000;
            }
            break;
        }

        //AI(ht160s-color-3pos) 20260627 : ===== CCD PHOTO STEP : the ONE difference vs
        //TEmptyModule::DoFeedTray. After GoDown+clamp, carry the tray through the MIDDLE scan
        //station and shoot the 2D identity. DoReadColor2D is an independent sub-ladder (its
        //own ScanTask) that moves the CCD-X reader + LON/read/LOFF and births the identity
        //tray. Tune the interleave / scan position here if the optics layout changes.
        case 3000:
            //AI(ht160s-color-ccd-xy) 20260628 : move carriage Y + CCD reader X TOGETHER via the
            //shared MoveColorCcdToScan (mirrors Loader MoveToCcdCell). DoReadColor2D case 10 then
            //re-asserts the CCD X position (a no-op once it is already there).
            if(MoveColorCcdToScan())
            {
                DoReadColor2D(0);
                FeedTask=3100;
            }
            break;

        case 3100:
            if(DoReadColor2D(1))
                FeedTask=4000;
            break;
        //AI(ht160s-color-3pos) 20260627 : ===== end CCD photo step =====

        case 4000:
            //AI(ht160s-color-align-empty) 20260627 : carry the clamped tray to the REAR pickup
            //Y (Empty's EmptyCarDischargeTrayYPosition analog). The front buffer is now empty
            //-> clear bFrontHasTray so DoColor re-stages via DoGoDownTray next cycle. (THIS is
            //the latch that was wrong before -- it stayed set and broke the 2nd pickup.)
            if(MoveColorY(Teach.ColorTrayArmPickYPosition))
            {
                bFrontHasTray=false;
                FeedTask=5000;
            }
            break;

        case 5000:
            //AI(ht160s-color-align-empty) 20260627 : release the carriage clamp at the rear
            //BEFORE the TrayArm picks (mirror Empty case 5000/6000). Without this the Color
            //cylinders still grip the tray when the TrayArm Z-downs + lifts -> collision.
            if(PopCylinder(HSys.Cyn.C_Color_PushTray))
                FeedTask=6000;
            break;

        case 6000:
            if(PopCylinder(HSys.Cyn.C_Color_LeanOnTray))
                FeedTask=7000;
            break;

        case 7000:
            //AI(ht160s-color-align-empty) 20260627 : confirm the tray actually reached the
            //rear/output (mirror Empty case 7000 / MES1021), then present it for TrayArm
            //pickup. bTrayReady is Color's pickable latch (Empty sets bRearHasTray).
            RefreshStateFromSensors();
            if(HSys.Sen.SnColor_OutputBottomHasTray.Enable==true &&
               HSys.Sen.SnColor_OutputBottomHasTray.IsOff() &&
               HSys.LastSet.iRealDummy!=DUMMY)
            {
                //AI(ht160s-agv) 20260627 : source-dry. In AMR mode wait iAmrFeedWaitSec for
                //the AGV to refill the supply magazine before raising the operator modal;
                //the happy path (refill in time) never reaches ShowMyError. Mirrors Empty/
                //Loader source-dry template. Armed ONLY under bUseAMR.
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
                        break;
                    bWaitingAmrFeed=false;
                    AmrFeedWaitTimer.Clear();
                }
                Ret=ShowMyError("MES1421", LangT("Color supply tray is not ready"), &HSys.Sen.SnColor_OutputBottomHasTray, true, K_RETRY);
                if(Ret==K_RETRY)
                    FeedTask=1;
            }
            else
            {
                bTrayReady=true;
                bSupplyRequested=false;
                bWaitingAmrFeed=false;     //AI(ht160s-agv) 20260627 : supply present -> end AMR wait (P4)
                AmrFeedWaitTimer.Clear();  //AI(ht160s-agv) 20260627 : clear AMR wait timer on success (P4)
                FeedTask=13000;
            }
            break;
        case 13000:
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260608 : Color identity-tray 2D barcode read sub-ladder.
//Mirrors the Loader Top CCD flow : move the reader (on the stepper, X axis) to the
//taught read position, LON to shoot, poll the socket for the code, LOFF to end.
bool TColorModule::DoReadColor2D(int Flag)
{
    int Ret;

    if(Flag==0)
    {
        ScanTask=1;
        ScanDelay.Clear();
        return true;
    }

    switch(ScanTask)
    {
        case 1:
            //AI(HT160S-Maintainer) 20260624 : per-CCD Enable. No real Color CCD scan
            //in a sim tier OR when disabled -> fabricate a TrayID so the AMR pull
            //flow keeps running. Enable=OFF now yields SIM data (was empty string).
            //DUMMY always sim; HAS_TRAY/REALLY real-scan when bUseColorCcd, else sim.
            if(IsSoftSimulate() || tSimuData.bRunSimulation || CosFunction.bUseColorCcd==false)
            {
                sTrayID2D=AnsiString("COLOR2D_")+Now().FormatString("hhnnsszzz");
                StampReadIdentity2D();   //AI(ht160s-tray-source) : born here (sim/disabled identity)
                return true;
            }
            EnsureColorCcdSocketCreated();
            if(ColorCcdSocket!=NULL)
                ColorCcdSocket->ColorCcdConnect();
            ScanTask=10;
            break;

        case 10:
            //Move the 2D reader to the taught read position over the identity tray.
            if(HSys.Mot.MTopCCDX_Color==NULL)
            {
                ScanTask=100;
                break;
            }
            if(HSys.Mot.MTopCCDX_Color->CheckSoftLimit(Teach.ColorRead2DXPosition)==false)
            {
                ShowMotorLimitError(HSys.Mot.MTopCCDX_Color->AlarmName[eMotOverLimitErr], LangT("Color CCD X motor will out of limit"), HSys.Mot.MTopCCDX_Color, Teach.ColorRead2DXPosition);
                ScanTask=100;
                break;
            }
            if(HSys.Mot.MTopCCDX_Color->MotorMove(Teach.ColorRead2DXPosition))
                ScanTask=100;
            break;

        case 100:
            if(ColorCcdSocket==NULL || ColorCcdSocket->IsColorCcdConnected()==false)
            {
                Ret=ShowSystemError("ColorCCD_Connect", K_RETRY|K_SKIP);
                if(Ret==K_SKIP)
                {
                    sTrayID2D="";
                    StampReadIdentity2D();   //AI(ht160s-tray-source) : born here (skip; identity tray, empty 2D)
                    return true;
                }
                EnsureColorCcdSocketCreated();
                if(ColorCcdSocket!=NULL)
                    ColorCcdSocket->ColorCcdConnect();
                break;
            }
            ColorCcdSocket->ColorCcdTriggerShot();   //LON : start shot
            ScanDelay.SetMS(3000);
            ScanDelay.On();
            ScanTask=200;
            break;

        case 200:
            {
                AnsiString sCode="";
                if(ColorCcdSocket!=NULL && ColorCcdSocket->ColorCcdGetResult(sCode))
                {
                    sTrayID2D=sCode;
                    if(ColorCcdSocket!=NULL)
                        ColorCcdSocket->ColorCcdEndShot();   //LOFF : end shot
                    StampReadIdentity2D();   //AI(ht160s-tray-source) : born here (real 2D read)
                    return true;
                }
                else if(ScanDelay.Off())
                {
                    if(ColorCcdSocket!=NULL)
                        ColorCcdSocket->ColorCcdEndShot();   //LOFF : end shot
                    Ret=ShowSystemError("ColorCCD_2D", K_RETRY|K_SKIP|K_MANUAL_2D);
                    if(Ret==K_RETRY)
                    {
                        if(ColorCcdSocket!=NULL)
                            ColorCcdSocket->ColorCcdTriggerShot();
                        ScanDelay.SetMS(3000);
                        ScanDelay.On();
                    }
                    else if(Ret==K_MANUAL_2D)
                    {
                        //AI(ht160s-ccd-manual2d) : operator hand-entered the tray identity 2D.
                        sTrayID2D=fNote->ManualText;
                        StampReadIdentity2D();
                        return true;
                    }
                    else
                    {
                        sTrayID2D="";
                        StampReadIdentity2D();   //AI(ht160s-tray-source) : born here (skip; identity tray, empty 2D)
                        return true;
                    }
                }
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(ht160s-color-align-empty) 20260627 : identity-tray BIRTH model, aligned to TEmptyModule.
//Born at the front staging point (BirthFrontTray, into FrontSourceTray), handed to the carriage
//motor at clamp (DoFeedTray case 2000, MoveFrom). Identity trays carry no IC so the grid is
//all-empty (Kind=Identity); the 2D code IS the identity (TrayID).
void TColorModule::BirthFrontTray()
{
    FrontSourceTray.Birth(EMPTY_IC, eTrayKindIdentity, "");
}
//---------------------------------------------------------------------------
//AI(ht160s-color-align-empty) 20260627 : the CCD read only UPDATES the carried tray's 2D TrayID
//(a data update, NOT a birth). Normal path: the grid was born at the front + handed to the
//carriage, so just set TrayID. Recovery/startup (tray at rear, no front birth): birth it here so
//the TrayArm always carries a valid identity grid.
void TColorModule::StampReadIdentity2D()
{
    if(HSys.VMot.MMColorY==NULL)
        return;
    if(HSys.VMot.MMColorY->fHasTray==false)
        HSys.VMot.MMColorY->Tray.Birth(EMPTY_IC, eTrayKindIdentity, sTrayID2D);
    else
        HSys.VMot.MMColorY->Tray.TrayID=sTrayID2D;
    HSys.VMot.MMColorY->fHasTray=true;
    HSys.VMot.MMColorY->Refresh();
}
//---------------------------------------------------------------------------
//AI(ht160s-tray-source) : return-by-value deep copy of the presented identity tray.
TMyTray TColorModule::GetSourceTray()
{
    if(HSys.VMot.MMColorY!=NULL)
        return HSys.VMot.MMColorY->Tray;
    TMyTray empty;
    return empty;
}
//---------------------------------------------------------------------------
bool TColorModule::DoSortBin(int Flag)
{
    if(Flag==0)
    {
        SortBinTask=1;
        return true;
    }

    switch(SortBinTask)
    {
        case 1:
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TColorModule::SetMode(int Mode)
{
    if(Mode!=eHT160ColorModeSortBin && Mode!=eHT160ColorModeTraySupply)
        return false;
    iMode=Mode;
    return true;
}
//---------------------------------------------------------------------------
int TColorModule::GetMode()
{
    return iMode;
}
//---------------------------------------------------------------------------
bool TColorModule::IsTraySupplyMode()
{
    return iMode==eHT160ColorModeTraySupply;
}
//---------------------------------------------------------------------------
bool TColorModule::IsSortBinMode()
{
    return iMode==eHT160ColorModeSortBin;
}
//---------------------------------------------------------------------------
bool TColorModule::IsTrayReady()
{
    RefreshStateFromSensors();
    return IsInstalled() && IsTraySupplyMode() && bTrayReady;
}
//---------------------------------------------------------------------------
bool TColorModule::IsAcceptingIC()
{
    return false;
}
//---------------------------------------------------------------------------
void TColorModule::RequestSupplyTray()
{
    if(IsInstalled() && IsTraySupplyMode())
        bSupplyRequested=true;
}
//---------------------------------------------------------------------------
void TColorModule::NotifyTrayPicked()
{
    //AI(ht160s-color-align-empty) 20260627 : single-step pickup release, mirroring TEmptyModule
    //(TrayArm picks -> SetRearHasTray(false)). The carriage clamp was already released inline in
    //DoFeedTray case 5000/6000, so just clear the rear-ready state + hide the grid; no separate
    //DoReleaseTray pass (that branch + case 1500 are removed from DoColor).
    bTrayReady=false;
    bRearHasTray=false;
    if(HSys.VMot.MMColorY!=NULL) HSys.VMot.MMColorY->ClearTray();
}
//---------------------------------------------------------------------------
//AI(phase6-loader-recycle) 20260625 : Color receive-tray contract, identical name and
//semantics to TEmptyModule's so TrayArm::DoPlaceToColor and DoPlaceToEmpty are the same
//shape. RequestReturnTray arms the receive ladder; the deposit handshake mirrors Empty.
void TColorModule::RequestReturnTray()
{
    bReturnTray=true;
    bTrayXToEmptyFinish=false;
}
//---------------------------------------------------------------------------
//AI(ht160s-color-align-empty) 20260627 : Color's rear-occupied latch is bRearHasTray (the
//output/read position = Color's rear handoff slot). TrayArm waits IsRearHasTray()==false
//before depositing, exactly as it waits on Empty's rear.
bool TColorModule::IsRearHasTray()
{
    RefreshStateFromSensors();
    return bRearHasTray;
}
//---------------------------------------------------------------------------
bool TColorModule::IsCleanOutFinish()
{
    //AI(cleanout) 20260701 : Color participates in CleanOut (was: no participation). Not installed
    //-> trivially finished. Otherwise it finishes only after TrayArm has finished (no more returns),
    //its flow path is clear (no front/rear tray) and the front rise/separate cylinders are all home
    //(IsReadyForAmrHandoff; sim-true). Until then DoColor case 100 GoUp-drains every tray to the car.
    if(IsInstalled()==false)
        return true;
    //AI(cleanout) 20260701 : the CleanOut drain (GoUp of front/rear trays) lives in DoColor case
    //100, which is UNREACHABLE in SortBin mode (case 10 branches to DoSortBin). A SortBin-mode
    //Color must not gate CleanOut on a tray it will never drain -> treat it as trivially finished.
    if(IsTraySupplyMode()==false)
        return true;
    if(HSys.Sys.RunMode!=Run_CleanOut)
        return true;
    if(TrayArmModule==NULL || TrayArmModule->IsCleanOutFinish()==false)
        return false;
    RefreshStateFromSensors();
    if(bFrontHasTray || bRearHasTray)
        return false;
    if(IsReadyForAmrHandoff()==false)
        return false;
    return true;
}
//---------------------------------------------------------------------------
//AI(phase6-loader-recycle) 20260625 : TrayArm finished depositing the returned tray onto
//Color's rear handoff slot. Mirrors Empty (sets the finish flag + marks rear occupied).
//This is the SOLE trigger that lets the receive ladder (case 1700) proceed; sim does NOT
//auto-advance it, matching Empty's single trigger point.
void TColorModule::NotifyTrayXToEmptyFinish()
{
    bTrayXToEmptyFinish=true;
    bRearHasTray=true;
}
//---------------------------------------------------------------------------
void TColorModule::NotifyICPlaced(int Count)
{
    if(Count>0)
        iICCount+=Count;
}
//---------------------------------------------------------------------------
void TColorModule::SetSupplyThreshold(int Count)
{
    if(Count<1)
        Count=1;
    iSupplyThreshold=Count;
}
//---------------------------------------------------------------------------
int TColorModule::GetSupplyThreshold()
{
    return iSupplyThreshold;
}
//---------------------------------------------------------------------------
int TColorModule::GetICCount()
{
    return iICCount;
}
//---------------------------------------------------------------------------
AnsiString TColorModule::GetTrayID()
{
    return sTrayID2D;
}
//---------------------------------------------------------------------------
//AI(general) 20260617 : Teach Advanced destacker test. Color has no production GoUp;
//these cylinder-only GoDown/GoUp drive the front destacker (FrontRiseTray_1/_2/Separate)
//in isolation, mirroring Empty's rise/separate choreography. No Y-motor / push / lean.
bool TColorModule::TestGoDownTray(int Flag)
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
            HSys.Cyn.C_Color_FrontRiseTray_1.On();
            TestDownTask=2000;
            break;

        case 2000:
            if(HSys.Cyn.C_Color_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Color_FrontRiseTray_2.On();
                TestDownTask=3000;
            }
            break;

        case 3000:
            if(HSys.Cyn.C_Color_FrontRiseTray_2.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Color_FrontSeparateTray_1.On();
                TestDelay.SetMS(GeneralSetting.iColorDestackSettleMs);
                TestDelay.On();
                TestDownTask=4000;
            }
            break;

        case 4000:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Color_FrontRiseTray_2.Off();
                TestDelay.SetMS(GeneralSetting.iColorDestackSettleMs);
                TestDelay.On();
                TestDownTask=4100;
            }
            break;

        case 4100:
            if(TestDelay.Off())
                TestDownTask=5000;
            break;

        case 5000:
            if(HSys.Cyn.C_Color_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Color_FrontSeparateTray_1.Off();
                TestDelay.SetMS(GeneralSetting.iColorDestackSettleMs);
                TestDelay.On();
                TestDownTask=6000;
            }
            break;

        case 6000:
            if(TestDelay.Off())
                TestDownTask=6500;
            break;

        case 6500:
            if(HSys.Cyn.C_Color_FrontRiseTray_1.Pop() || IsSoftSimulate())
            {
                TestDownTask=1;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TColorModule::TestGoUpTray(int Flag)
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
            HSys.Cyn.C_Color_FrontRiseTray_1.On();
            TestUpTask=200;
            break;

        case 200:
            if(HSys.Cyn.C_Color_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Color_FrontSeparateTray_1.On();
                TestDelay.SetMS(GeneralSetting.iColorDestackSettleMs);
                TestDelay.On();
                TestUpTask=300;
            }
            break;

        case 300:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Color_FrontRiseTray_2.On();
                TestUpTask=400;
            }
            break;

        case 400:
            if(HSys.Cyn.C_Color_FrontRiseTray_2.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Color_FrontSeparateTray_1.Off();
                TestDelay.SetMS(GeneralSetting.iColorDestackSettleMs);
                TestDelay.On();
                TestUpTask=500;
            }
            break;

        case 500:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Color_FrontRiseTray_2.Off();
                if(HSys.Cyn.C_Color_FrontRiseTray_1.IsOn() || IsSoftSimulate())
                    TestUpTask=600;
            }
            break;

        case 600:
            if(HSys.Cyn.C_Color_FrontRiseTray_1.Pop() || IsSoftSimulate())
            {
                TestUpTask=1;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
AnsiString TColorModule::DescribeState()
{
    //AI(ht160s-state-record-analysis) 20260622 : read-only inner-state dump for
    //FeederDecision.txt. Reads latched members directly (does NOT call
    //RefreshStateFromSensors, which clobbers the sim/dummy latch). The
    //bTrayReady/bSupplyRequested are first : a latched bTrayReady
    //with no pick is the Normal-mode (no AMR demand) idle-spin signature.
    AnsiString s;
    s  = "[Color]\r\n";
    s += "  bTrayReady=" + IntToStr(bTrayReady ? 1 : 0)
       + "  bSupplyRequested=" + IntToStr(bSupplyRequested ? 1 : 0)
       + "  bFrontHasTray=" + IntToStr(bFrontHasTray ? 1 : 0) + "\r\n";
    //AI(ht160s-agv) 20260627 : Color source-dry AMR wait latch + AMR lock (P4 State Record).
    s += "  bWaitingAmrFeed=" + IntToStr(bWaitingAmrFeed ? 1 : 0)
       + "  bAmrLocked=" + IntToStr(bAmrLocked ? 1 : 0) + "\r\n";
    s += "  Installed=" + IntToStr(IsInstalled() ? 1 : 0)
       + "  SoftSim=" + IntToStr(IsSoftSimulate() ? 1 : 0)
       + "  Mode=" + AnsiString(IsTraySupplyMode() ? "TraySupply" : (IsSortBinMode() ? "SortBin" : "?")) + "\r\n";
    s += "  bInputFullTray=" + IntToStr(bInputFullTray ? 1 : 0)
       + "  bRearHasTray=" + IntToStr(bRearHasTray ? 1 : 0) + "\r\n";
    s += "  FeedTask=" + IntToStr(FeedTask)
       + "  GoDownTask=" + IntToStr(GoDownTask)
       + "  GoUpTask=" + IntToStr(GoUpTask)
       + "  ScanTask=" + IntToStr(ScanTask)
       + "  SortBinTask=" + IntToStr(SortBinTask) + "\r\n";
    s += "  bReturnTray=" + IntToStr(bReturnTray ? 1 : 0)
       + "  bTrayXToEmptyFinish=" + IntToStr(bTrayXToEmptyFinish ? 1 : 0)
       + "  iReturnedCount=" + IntToStr(iReturnedCount) + "\r\n";
    s += "  iICCount=" + IntToStr(iICCount)
       + "  iSupplyThreshold=" + IntToStr(iSupplyThreshold)
       + "  TrayID2D=" + sTrayID2D + "\r\n";
    return s;
}
//---------------------------------------------------------------------------
void InitializeColorModule()
{
    if(ColorModule==NULL)
        ColorModule=new TColorModule;
}
//---------------------------------------------------------------------------
void ShutdownColorModule()
{
    delete ColorModule;
    ColorModule=NULL;
}
//---------------------------------------------------------------------------