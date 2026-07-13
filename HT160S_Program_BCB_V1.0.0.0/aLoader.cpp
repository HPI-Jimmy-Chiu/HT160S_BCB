#include "IncludeAllHeader.h"       //AI(HT160S-Maintainer) 20260609 : merged header to speed build
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#include <IniFiles.hpp>
#pragma hdrstop
#include "language.h"

#include "aLoader.h"
#include "database.h"
#include "cmydef.h"
#include "cprod.h"
#include "CosFunction.h"
#include "mymessbox.h"
#include "setup.h"
#include "uteach.h"
#include "TopCcdSocket.h"
#include "main.h"            //AI(HT160S-Maintainer) 20260609 : chkLoadTray on fMain
#include "GeneralSetting.h"   //AI(HT160S-Maintainer) 20260610 : LoaderYSafeDistance interlock
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TLoaderModule *LoaderModule=NULL;
//---------------------------------------------------------------------------
static int ClampIntValue(int Value, int MinValue, int MaxValue)
{
    if(Value<MinValue)
        return MinValue;
    if(Value>MaxValue)
        return MaxValue;
    return Value;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260608 : GetEditInt/GetEditDouble/ReadTrayForm* helpers
//removed - Tray geometry now comes from the in-memory TrayForm structure
//(CosFunction), not the Setup form UI nor a per-call setup.ini read.
//---------------------------------------------------------------------------
TLoaderModule::TLoaderModule()
{
    //AI(ht160s-rearready-p0) 20260705 : hard-zero the rear hold BEFORE InitialFlag --
    //InitialFlag now preserves a sensor-confirmed settled rear tray across runtime
    //resets, and must never "preserve" uninitialized ctor garbage on this first call.
    bRearHasTray=false;
    bRearDischargeInProgress=false;
    bRearReadyForPick=false;
    bRearResidualAlarmed=false;
    RearKind=eTrayKindNormal;
    InitialFlag();
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Loader-Y axis ownership tokens. The Loader-Y axis is shared between the
// Loader (feed/CCD) and the SortArm (pick). Only the current owner may move it.
static const int LOADER_Y_OWNER_NONE=0;
static const int LOADER_Y_OWNER_SORTARM=1;
//AI(HT160S-Maintainer) 20260605 : reserved TrayArm Loader-Y owner token for the
//rear-tray takeout handshake (LS_READY_TakeOut / LS_TakeOutIng). Not yet wired; kept
//so the owner model can grow to a third actor without touching existing call sites.
static const int LOADER_Y_OWNER_TRAYARM=2;
//---------------------------------------------------------------------------
void TLoaderModule::InitialFlag(bool bKeepMaterial)
{
    bAmrLocked=false;
    //AI(ht160s-home-resume-w1) 20260711 : keep-material HOME preserves the AMR car ledger
    //(host tray count + derived car total here, feed serial below) -- the physical car
    //stack does not change across a HOME, but zeroing these made GetFedTrayKind classify
    //the remaining cover/identity trays as Normal and broke the MES0921 cross-check.
    if(bKeepMaterial==false)
    {
        iSecsCarTrayCount=0;     //AI(ht160s-agv) 20260627 : no host count yet; RefillSimInfeed falls back to iSimAmrMaxTray
        RefillSimInfeed();
    }
    ResetSide(&Side[0]);
    ResetSide(&Side[1]);
    //AI(ht160s-rearready-p0) 20260705 : PRESERVE a settled rear tray across a runtime
    //reset (InitialAllTask on HOME / OneCycle finish / recovery). Wiping the published
    //latch while the tray physically stayed parked let the rear sensor re-latch
    //bRearHasTray with NO path back to bRearReadyForPick=true (its only setter is
    //DoDischargeTray case 4000, and a new discharge cannot start while the rear is
    //occupied) -> TAJOB_LOADER_RECOVERY pinned the TrayArm forever with no alarm.
    //Keep the whole rear hold (latch + Kind/ID/grid) when the settled tray is still
    //physically confirmed. A mid-discharge abort (latch still false) deliberately does
    //NOT preserve : that leftover is not known-safe to auto-pick, so it goes to the
    //MES0924 residual Note in DoLoader instead (operator removes it).
    bool bKeepRear = bRearReadyForPick && IsOutputBottomOccupied();
    if(bKeepRear==false)
    {
        bRearHasTray=false;
        bRearReadyForPick=false;      //AI(ht160s-rearready-state) 20260703 : rear not pickable until a discharge completes at case 4000
        RearKind=eTrayKindNormal;
        RearTrayID="";
        RearSourceTray.Clear();
    }
    bRearDischargeInProgress=false;   //AI(ht160s-trayarm-empty-handoff) 20260701 : no discharge settling in flight at init (all ladders reset to 1)
    bRearResidualAlarmed=false;       //AI(ht160s-rearready-p0) 20260705 : a new stranded episode after a reset may alarm again
    iFrontOwner=0;
    iTopCcdCount=0;
    iYOwner[0]=LOADER_Y_OWNER_NONE;
    iYOwner[1]=LOADER_Y_OWNER_NONE;
    SimuCcdCycleIndex=0;
    if(bKeepMaterial==false)
        iFeedSerial=0;        //AI(ht160s-tray-source) 20260625 : Phase 6 A.2 - reset feed counter (kept on keep-material HOME, see car-ledger note above)
    //AI(ht160s-rearready-p0) 20260705 : RearKind/RearTrayID/RearSourceTray moved into
    //the bKeepRear guard above -- wiping them while the tray stays parked would
    //misroute a preserved cover/identity tray as Normal.
    CurrentLotNumber="";
    TestUpTask=1;
    TestDownTask=1;
    TestDelay.Clear();
}
//---------------------------------------------------------------------------
//AI(ht160s-actuator-timer) 20260627 : freeze/thaw the per-side wall-clock timeout
//windows (CcdDelay Top-CCD scan timeout + FeedWaitTimer source-dry AMR wait MES0920) so a machine pause taken mid-scan is not
//charged against the timeout budget -- no false CCD-timeout on resume. Called from
//csystem PauseActuatorTimeoutTimers/ReStartActuatorTimeoutTimers on the SystemStart
//pause/resume edges, alongside Cylinder[]/SortArmSuck.
void TLoaderModule::PauseTimeoutTimers()
{
    Side[0].CcdDelay.Pause();
    Side[1].CcdDelay.Pause();
    Side[0].FeedWaitTimer.Pause();
    Side[1].FeedWaitTimer.Pause();
}
//---------------------------------------------------------------------------
void TLoaderModule::ReStartTimeoutTimers()
{
    Side[0].CcdDelay.ReStart();
    Side[1].CcdDelay.ReStart();
    Side[0].FeedWaitTimer.ReStart();
    Side[1].FeedWaitTimer.ReStart();
}
//---------------------------------------------------------------------------
void TLoaderModule::ResetSide(TLoaderSideState *State)
{
    if(State==NULL)
        return;
    State->FeedTask=1;
    State->CcdTask=1;
    State->DischargeTask=1;
    State->DestackTask=1;
    State->bTrayEmpty=false;
    State->bCcdLeftToRight=true;
    State->CcdX=0;
    State->CcdY=0;
    State->Status=LS_IDLE;
    State->bCleanOutFinish=false;
    State->FeedDelay.Clear();
    State->CcdDelay.Clear();
    State->bWaitingAmrFeed=false;   //AI(ht160s-agv) 20260626 : clear AMR feed deferral on side reset
    State->FeedWaitTimer.Clear();
}
//---------------------------------------------------------------------------
bool TLoaderModule::IsValidLoaderNo(int LoaderNo)
{
    return (LoaderNo==1 || LoaderNo==2);
}
//---------------------------------------------------------------------------
int TLoaderModule::GetSideIndex(int LoaderNo)
{
    if(LoaderNo==2)
        return 1;
    return 0;
}
//---------------------------------------------------------------------------
TLoaderSideState *TLoaderModule::GetSide(int LoaderNo)
{
    if(IsValidLoaderNo(LoaderNo)==false)
        return NULL;
    return &Side[GetSideIndex(LoaderNo)];
}
//---------------------------------------------------------------------------
TLoaderSideState *TLoaderModule::GetOtherSide(int LoaderNo)
{
    if(IsValidLoaderNo(LoaderNo)==false)
        return NULL;
    if(LoaderNo==1)
        return &Side[1];
    return &Side[0];
}
//---------------------------------------------------------------------------
int TLoaderModule::GetTrayXCount()
{
    //AI(HT160S-Maintainer) 20260608 : read recipe Tray geometry from the
    //in-memory TrayForm structure (single source), never the Setup form UI.
    return ClampIntValue(TrayForm.XDivision, 1, 50);
}
//---------------------------------------------------------------------------
int TLoaderModule::GetTrayYCount()
{
    return ClampIntValue(TrayForm.YDivision, 1, 20);
}
//---------------------------------------------------------------------------
double TLoaderModule::GetTrayXPitch()
{
    return TrayForm.XPitch*100.0;   //AI(ht160s-maintainer) 20260624 : mm to 1/100mm to match teach coords (same root cause as aSortArm; CCD cell scan pitch was 100x too small).
}
//---------------------------------------------------------------------------
double TLoaderModule::GetTrayYPitch()
{
    return TrayForm.YPitch*100.0;   //AI(ht160s-maintainer) 20260624 : mm to 1/100mm, see GetTrayXPitch.
}
//---------------------------------------------------------------------------
double TLoaderModule::GetTrayXStart()
{
    //AI(ht160s-maintainer) 20260624 : tray corner->first-IC offset X (mm->1/100mm), P2 HT172-align. Same TrayForm as SortArm => CCD shares origin.
    return TrayForm.XStart*100.0;
}
//---------------------------------------------------------------------------
double TLoaderModule::GetTrayYStart()
{
    return TrayForm.YStart*100.0;
}
//---------------------------------------------------------------------------
int TLoaderModule::GetLoaderFeedY(int LoaderNo)
{
    if(LoaderNo==2)
        return Teach.Loader2CarFeedTrayYPosition;
    return Teach.Loader1CarFeedTrayYPosition;
}
//---------------------------------------------------------------------------
int TLoaderModule::GetLoaderDischargeY(int LoaderNo)
{
    if(LoaderNo==2)
        return Teach.Loader2CarDischargeTrayYPosition;
    return Teach.Loader1CarDischargeTrayYPosition;
}
//---------------------------------------------------------------------------
int TLoaderModule::GetLoaderFirstCcdY(int LoaderNo)
{
    if(LoaderNo==2)
        return Teach.Loader2CarFirstCCDYPosition;
    return Teach.Loader1CarFirstCCDYPosition;
}
//---------------------------------------------------------------------------
int TLoaderModule::GetTopCcdFirstX()
{
    return Teach.LoaderCarFirstCCDXPosition;
}
//---------------------------------------------------------------------------
int TLoaderModule::RoundPosition(double Value)
{
    if(Value>=0.0)
        return (int)(Value+0.5);
    return (int)(Value-0.5);
}
//---------------------------------------------------------------------------
bool TLoaderModule::MoveLoaderY(int LoaderNo, int Position)
{
    TTrayMotor *Motor=NULL;

    if(LoaderNo==1)
        Motor=HSys.Mot.MLoaderY_1;
    else if(LoaderNo==2)
        Motor=HSys.Mot.MLoaderY_2;

    if(Motor==NULL)
        return false;
    //AI(HT160S-Maintainer) 20260610 : strict cross-side interlock. Refuse to move
    //this Loader-Y car when doing so would bring it within the configured safe
    //distance of the opposite car while the opposite car is clamping a tray.
    //Returning false (no modal) makes the caller poll/wait until the other car
    //clears, matching the non-blocking switch(Task) pattern.
    if(IsLoaderYMoveSafe(LoaderNo, Position)==false)
        return false;
    if(Motor->CheckSoftLimit(Position)==false)
    {
        ShowMotorLimitError(Motor->AlarmName[eMotOverLimitErr], LangT("Loader Y motor will out of limit"), Motor, Position);
        return false;
    }
    #ifndef SOFT_SIMULATE
    //AI(ht160s-trayarm-empty-handoff) 20260701 : TrayArm anti-collision, ported verbatim from
    //MoveEmptyY/MoveColorY (Loader was the only source lacking it). If the TrayArm is NOT raised
    //(Z-up off) and its X is at/near the Loader pickup X, block EITHER car's Y move so a shared-rail
    //carriage cannot slam into the lowered arm. Z-gated, so a Z-UP wait never blocks Loader Y (no
    //mutual deadlock). One MoveLoaderY serves LoaderNo 1/2, so this covers both cars.
    int TrayArmPos=0;
    if(HSys.Mot.MTrayArmX!=NULL)
        TrayArmPos=HSys.Mot.MTrayArmX->ReadEncoderPos();
    if(HSys.Cyn.C_TrayArmZ_Up.IsOn()==false &&
       (TrayArmPos+500)>=Teach.TrayXArmToLoaderXPosition)
    {
        return false;
    }
    #endif
    return Motor->MotorMove(Position);
}
//---------------------------------------------------------------------------
bool TLoaderModule::IsLoaderYMoveSafe(int LoaderNo, int Position, AnsiString *WhyBlocked)
{
    //AI(HT160S-Maintainer) 20260610 : framework for option C (opposite-side tray
    //clamped + minimum distance). The two Loader-Y cars share the same physical
    //rail. NOTE the two encoders read OPPOSITE signs for the same physical travel,
    //so the gap math abs()-normalizes each side to its physical magnitude first.
    //Rules :
    //  - Only the OTHER car holding a tray (fHasTray) is a collision risk; an empty
    //    car parked clear is ignored. (Empty parked cars are NOT yet protected - see
    //    the decouple/choreography follow-up.)
    //  - When both cars are loaded they cannot cross, so keep a FIXED order : require THIS
    //    car to end at least the safe distance on its OWN side of the OTHER car. A leading car
    //    may always advance further forward; only closing-in / crossing is blocked.
    //  - Safe distance is read from GeneralSetting.iLoaderYSafeDistance
    //    ([Safety] LoaderYSafeDistance in General.ini, default 10000).
    //  - When data cannot be evaluated (NULL motors) the move is allowed so this
    //    guard never silently freezes the machine on a missing object.
    int OtherNo;
    TTrayMotor *OtherMotor=NULL;
    TTrayMotor *OtherTray=NULL;
    int OtherPos;

    OtherNo=(LoaderNo==1) ? 2 : 1;
    if(OtherNo==1)
    {
        OtherMotor=HSys.Mot.MLoaderY_1;
        OtherTray=HSys.VMot.MMLoaderY_1;
    }
    else
    {
        OtherMotor=HSys.Mot.MLoaderY_2;
        OtherTray=HSys.VMot.MMLoaderY_2;
    }

    //AI(ht160s-loader) 20260707 (Option B, on-site) : a tray deposited on the rear REST
    //(awaiting TrayArm pickup) has NO carriage/encoder, so the two-car checks below are
    //blind to it. A loaded car SORTING at the top rows comes within SafeDist of that parked
    //tray (rest ~= discharge Y vs the top sort row) and the two overhanging trays clash.
    //Keep a loaded mover SafeDist away from the rest whenever the rear is occupied. Exempt
    //the discharging car itself (Status==LS_ToRear : it is the one placing at the rest and
    //its own retreat runs empty) else it would block its own landing. PeekRearOccupied() is
    //the NON-MUTATING physical-presence truth (sim: bRearHasTray latch, cleared by
    //NotifyTrayArmPickRearTray; real: live SnLoader_OutputBottomHasTray going OFF once the
    //TrayArm lifts the tray clear, latch fallback when the sensor is disabled).
    //RefreshRearState is deliberately NOT called here so the per-tick interlock and the
    //DescribeState state-record dump stay read-only (no latch writes from this path).
    {
        TTrayMotor *ThisTrayRear=(LoaderNo==1) ? HSys.VMot.MMLoaderY_1 : HSys.VMot.MMLoaderY_2;
        TLoaderSideState *ThisSideRear=GetSide(LoaderNo);
        bool bThisDischarging=(ThisSideRear!=NULL && ThisSideRear->Status==LS_ToRear);
        if(ThisTrayRear!=NULL && ThisTrayRear->fHasTray && bThisDischarging==false &&
           GeneralSetting.iLoaderYSafeDistance>0 && PeekRearOccupied())
        {
            int RestAbs=GetLoaderDischargeY(LoaderNo);
            if(RestAbs<0) RestAbs=-RestAbs;
            int TgtAbsRear=Position;
            if(TgtAbsRear<0) TgtAbsRear=-TgtAbsRear;
            int Diff=TgtAbsRear-RestAbs;
            if(Diff<0) Diff=-Diff;
            if(Diff<GeneralSetting.iLoaderYSafeDistance)
            {
                if(WhyBlocked!=NULL)
                    *WhyBlocked="rear-rest";
                return false;
            }
        }
    }

    if(OtherMotor==NULL || OtherTray==NULL)
        return true;

    //AI(HT160S-Maintainer) 20260610 : "opposite-side tray clamped" condition.
    //fHasTray is the logical tray-held state set true after the Push/Lean clamp
    //completes in DoFeedTray, so it is the available proxy for a clamped tray.
    //AI(ht160s-loader) 20260707 : the comment above is optimistic -- fHasTray is actually
    //minted LATE, at DoFeedTray case 9500 (confirm-then-mint), yet the tray is physically
    //destacked onto the carriage at case 4100 and clamped at 8200/8300. Through that
    //4100..9500 window the feeding car overhangs a tray while fHasTray is still false, so a
    //loaded OTHER car read "empty" here and was allowed to close in -> the two overhanging
    //trays clash (on-site 20260707 : LD1 clamp timing, LD2 hits LD1). Also treat a car that
    //is mid-FEED (Status==LS_FEEDING) as occupied so the collision backstop protects it for
    //the WHOLE feed, not only after the late mint. fHasTray still minted at 9500 (confirm-
    //then-mint invariant intact); the feeding car is stationary at its feed Y and completes
    //independently, so a blocked mover can never deadlock it.
    TLoaderSideState *OtherSide=GetOtherSide(LoaderNo);
    bool bOtherFeeding=(OtherSide!=NULL && OtherSide->Status==LS_FEEDING);
    if(OtherTray->fHasTray==false && bOtherFeeding==false)
        return true;

    //AI(ht160s-sortarm) 20260624 : collision happens ONLY when BOTH cars carry a tray
    //(operator-confirmed mechanical fact : the two carriages pass freely on their rails; it is
    //the two overhanging TRAYS that clash). So an empty car - crucially a just-discharged car
    //heading back to feed - may ALWAYS move past a loaded car. This is the core deadlock break :
    //empty Loader1 no longer gets trapped behind loaded Loader2, and two empty cars near the
    //home/origin zone no longer block each other. Restrict only when THIS car is loaded too.
    TTrayMotor *ThisTray=(LoaderNo==1) ? HSys.VMot.MMLoaderY_1 : HSys.VMot.MMLoaderY_2;
    if(ThisTray==NULL || ThisTray->fHasTray==false)
        return true;

    if(GeneralSetting.iLoaderYSafeDistance<=0)
        return true;

    //AI(ht160s-sortarm) 20260624 : ORDER-AWARE + sign-normalized interlock for the both-loaded
    // case. The two encoders read OPPOSITE signs for the same physical travel (Y1 +, Y2 -), so
    // abs()-normalize each to its physical magnitude first (rail is always >=0; manual negate, NOT
    // std::abs which this BCB6 unit lacks). The two loaded cars cannot cross, so they hold a fixed
    // order : enforce only that THIS car ends at least SafeDist on its OWN side of the OTHER car.
    //   - THIS car AHEAD : may move freely FURTHER ahead, only as far back as Other+SafeDist. So a
    //     leading car is NEVER blocked from advancing just because the trailing car sits close
    //     behind (that over-block was the bug in the earlier swept-interval form).
    //   - THIS car BEHIND : may move freely further back, only up to Other-SafeDist.
    // Lets the front car keep going forward AND forbids closing-in / crossing.
    TTrayMotor *ThisMotor=(LoaderNo==1) ? HSys.Mot.MLoaderY_1 : HSys.Mot.MLoaderY_2;
    int ThisCur=(ThisMotor!=NULL) ? ThisMotor->ReadEncoderPos() : Position;
    if(ThisCur<0)
        ThisCur=-ThisCur;
    int Tgt=Position;
    if(Tgt<0)
        Tgt=-Tgt;
    OtherPos=OtherMotor->ReadEncoderPos();
    if(OtherPos<0)
        OtherPos=-OtherPos;
    if(ThisCur>=OtherPos)
    {
        //this car leads : stay at least SafeDist ahead (forward moves always pass)
        if(Tgt>=OtherPos+GeneralSetting.iLoaderYSafeDistance)
            return true;
        if(WhyBlocked!=NULL)
            *WhyBlocked=(OtherTray->fHasTray==false) ? AnsiString("gap:other-feeding") : AnsiString("gap:both-loaded");
        return false;
    }
    //this car trails : stay at least SafeDist behind
    if(Tgt<=OtherPos-GeneralSetting.iLoaderYSafeDistance)
        return true;
    if(WhyBlocked!=NULL)
        *WhyBlocked=(OtherTray->fHasTray==false) ? AnsiString("gap:other-feeding") : AnsiString("gap:both-loaded");
    return false;
}
//---------------------------------------------------------------------------
bool TLoaderModule::MoveTopCcdX(int Position)
{
    if(HSys.Mot.MTopCCDX==NULL)
        return false;
    if(HSys.Mot.MTopCCDX->CheckSoftLimit(Position)==false)
    {
        ShowMotorLimitError(HSys.Mot.MTopCCDX->AlarmName[eMotOverLimitErr], LangT("Top CCD X motor will out of limit"), HSys.Mot.MTopCCDX, Position);
        return false;
    }
    return HSys.Mot.MTopCCDX->MotorMove(Position);
}
//---------------------------------------------------------------------------
bool TLoaderModule::MoveToCcdCell(int LoaderNo, int CellX, int CellY)
{
    //AI(ht160s-maintainer) 20260627 : CCD scan no longer applies the tray-datum model
    //(XStart/YStart bias). The TopCCD base teach (GetTopCcdFirstX/GetLoaderFirstCcdY) is
    //the first-cell scan position directly: pos = base + cell*pitch.
    int XPos=RoundPosition((double)GetTopCcdFirstX()+((double)CellX)*GetTrayXPitch());
    int YPos=RoundPosition((double)GetLoaderFirstCcdY(LoaderNo)+((double)CellY)*GetTrayYPitch());
    bool bXFlag=MoveTopCcdX(XPos);
    bool bYFlag=MoveLoaderY(LoaderNo, YPos);
    return (bXFlag && bYFlag);
}
//---------------------------------------------------------------------------
//AI(ht160s-ccd-teach-test) 20260628 : Teach Advanced CCD move-to-cell test support. Mirrors
//SortArm CanMoveSuckerToCell/MoveSuckerToCell. Drives the Top CCD over a chosen tray cell on
//LoaderR(=2)/LoaderL(=1) so RD can verify CCD-to-cell alignment from the Teach screen. Reuses the
//existing private MoveToCcdCell (commands+polls both axes and re-checks the shared-rail interlock
//each tick). Cells are 0-based here; the uteach caller converts 1-based UI by -1.
bool TLoaderModule::CanMoveCcdToCell(int LoaderNo, int CellX, int CellY, AnsiString &Err)
{
    int XPos;
    int YPos;
    TTrayMotor *Y;

    Err="";
    if(IsValidLoaderNo(LoaderNo)==false)
    {
        Err="Invalid Loader number";
        return false;
    }
    if(CellX<0 || CellX>=GetTrayXCount())
    {
        Err="Column out of tray range (1.."+IntToStr(GetTrayXCount())+")";
        return false;
    }
    if(CellY<0 || CellY>=GetTrayYCount())
    {
        Err="Row out of tray range (1.."+IntToStr(GetTrayYCount())+")";
        return false;
    }
    XPos=RoundPosition((double)GetTopCcdFirstX()+((double)CellX)*GetTrayXPitch());
    YPos=RoundPosition((double)GetLoaderFirstCcdY(LoaderNo)+((double)CellY)*GetTrayYPitch());
    if(HSys.Mot.MTopCCDX==NULL || HSys.Mot.MTopCCDX->CheckSoftLimit(XPos)==false)
    {
        Err="Top CCD X target over soft limit";
        return false;
    }
    Y=(LoaderNo==2) ? HSys.Mot.MLoaderY_2 : HSys.Mot.MLoaderY_1;
    if(Y==NULL || Y->CheckSoftLimit(YPos)==false)
    {
        Err="Loader Y target over soft limit";
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
bool TLoaderModule::MoveCcdToCell(int LoaderNo, int CellX, int CellY, int &Task)
{
    //AI(ht160s-ccd-teach-test) 20260628 : task-stepped wrapper. MoveToCcdCell commands then polls
    //the Top CCD X + the chosen Loader Y to in-position and re-checks IsLoaderYMoveSafe every tick
    //(waits, never fails hard, if the other carriage blocks the shared rail). Bad args -> Task=900,
    //finish with no motion (mirror SortArm). Caller validates first via CanMoveCcdToCell.
    if(IsValidLoaderNo(LoaderNo)==false ||
       CellX<0 || CellX>=GetTrayXCount() ||
       CellY<0 || CellY>=GetTrayYCount())
    {
        return true;   //AI(ht160s-ladder-guard) 20260703 : bad args, abort (was dead Task=900)
    }
    if(MoveToCcdCell(LoaderNo, CellX, CellY))
        return true;
    return false;
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : AMR P1 (Loader) handoff interface, mirrors TAutoModule.
void TLoaderModule::SetAmrLock(bool bLock)
{
    bAmrLocked=bLock;
}
//---------------------------------------------------------------------------
bool TLoaderModule::IsAmrLocked()
{
    return bAmrLocked;
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : Ready = front stacking cylinders back home (not commanded
//up); destack idle so the AGV may refill. Held stable by bAmrLocked.
bool TLoaderModule::IsReadyForAmrHandoff()
{
    //AI(ht160s-agv) 20260625 : sim defense-in-depth - in SOFT_SIMULATE the front
    //destacker out-bits are normally already false; assert ready so the PREP->READY
    //gate cannot latch the lock on a laptop run. Real hardware keeps the interlock.
    if(IsSoftSimulate())
        return true;
    return (HSys.Cyn.C_Loader_FrontRiseTray_1.GetOutBit()==false
            && HSys.Cyn.C_Loader_FrontRiseTray_2.GetOutBit()==false
            && HSys.Cyn.C_Loader_FrontSeparateTray_1.GetOutBit()==false);
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : shortage (call AGV). Sim drains iSimInfeedCount to 0; real
//reads SnLoader_Inputend (ON=has tray, OFF=empty). Disabled sensor -> no call.
bool TLoaderModule::IsInputShortageForAmr()
{
    if(IsSoftSimulate())
        return (iSimInfeedCount<=0);
    return (HSys.Sen.SnLoader_Inputend.Enable==true && HSys.Sen.SnLoader_Inputend.IsOff());
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : Finish = refill complete. Sim auto-completes (no sensor);
//real waits for SnLoader_Inputend to read a tray present (ON).
bool TLoaderModule::IsInputHandoffFinishedForAmr()
{
    if(IsSoftSimulate())
        return true;
    return (HSys.Sen.SnLoader_Inputend.Enable==true && HSys.Sen.SnLoader_Inputend.IsOn());
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : reset sim input-stack to configured max (AGV delivered a
//full magazine). Real machine ignores the count (sensor-driven).
void TLoaderModule::RefillSimInfeed()
{
    //AI(ht160s-agv) 20260627 : latch the FIXED magazine total for this car. When AMR is
    //on and the host declared a LoaderTrayCount (SECS S2F41 -> SetExpectedCarTrayCount),
    //that physical total (IC + cover + identity) is the source of truth for tray-kind
    //tagging and the count-vs-Inputend cross-check; otherwise fall back to the sim max.
    iCarTrayTotal = (GeneralSetting.bUseAMR && iSecsCarTrayCount>0)
                    ? iSecsCarTrayCount
                    : GeneralSetting.iSimAmrMaxTray[0];
    iSimInfeedCount=iCarTrayTotal;
    iFeedSerial=0;            //AI(ht160s-tray-source) 20260625 : Phase 6 A.2 - new car => restart feed serial
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260627 : the AGV coordinator calls this on car arrival (CEID274
//Finish) with the host-declared LoaderTrayCount captured from the preceding S2F41.
//Stored so the next RefillSimInfeed latches it as the fixed car total. 0 = host silent.
void TLoaderModule::SetExpectedCarTrayCount(int n)
{
    iSecsCarTrayCount = (n>0) ? n : 0;
}
//---------------------------------------------------------------------------
//AI(ht160s-tray-source) 20260625 : Phase 6 A.2 - D2 stack-position convention.
//No sensor reads the tray kind; software infers it from the feed order on the
//shared supply car. Confirmed convention: the identity tray is fed LAST, the top
//cover just before it, the rest are normal work trays.
//   feedSerial==total   => Identity
//   feedSerial==total-1 => Cover
//   else                => Normal
//Single source of truth for the kind-by-position rule.
eTrayKind TLoaderModule::GetFedTrayKind(int feedSerial, int total)
{
    //AI(ht160s-amr0-fix) 20260708 : identity/cover stack ordering is an AMR-supply
    //concept only. Under manual production (bUseAMR==false) iFeedSerial is not bounded
    //by a real magazine total (it resets only at init / via the AGV coordinator, which
    //is inert when AMR is off), so once it passes iSimAmrMaxTray[0] every fed tray would
    //be spuriously tagged Identity and misrouted back to Color in DecidePlaceDestAfterPick.
    //Force Normal so recovered empties recycle to the Empty pool / requesting Autos.
    if(GeneralSetting.bUseAMR==false)
        return eTrayKindNormal;
    if(feedSerial>=total)
        return eTrayKindIdentity;
    if(feedSerial==total-1)
        return eTrayKindCover;
    return eTrayKindNormal;
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260624 : trays currently on the shared Loader supply car, for the
//PanelMain6 Motion View header. Sim drains this per feed; the real machine is
//sensor-driven and does not maintain the count (reads the configured max).
int TLoaderModule::GetCarTrayCount()
{
    return iSimInfeedCount;
}
//---------------------------------------------------------------------------
bool TLoaderModule::IsSoftSimulate()
{
    #ifdef SOFT_SIMULATE
    return true;
    #else
    return (HSys.LastSet.iRealDummy==DUMMY);
    #endif
}
//---------------------------------------------------------------------------
bool TLoaderModule::IsContinuousFeed()
{
    //AI(HT160S-Maintainer) 20260609 : chkLoadTray gate for simulate/DUMMY feeding.
    //In simulate/DUMMY there is no physical "Load area has tray" sensor, so the
    //main-form chkLoadTray ("Load New Tray") checkbox decides what an empty feed
    //means : checked = pretend a tray is always present so the Loader keeps feeding
    //continuously; unchecked = report "Loader Tray Empty" so the run can stop.
    //NULL-guarded so early-boot calls default to continuous feed (prior behaviour).
    if(fMain!=NULL && fMain->chkLoadTray!=NULL)
        return fMain->chkLoadTray->Checked;
    return true;
}
//---------------------------------------------------------------------------
bool TLoaderModule::IsSupplyCarDry()
{
    //AI(ht160s-loader) 20260706 : "the supply car has no more trays to feed." Extracted from
    //DoLoader's CleanOut retire gate for readability (was an inline ?: + && expression).
    //Sim/DUMMY has no InputEnd card, so chkLoadTray (IsContinuousFeed) stands in for car stock.
    //Real: source InputEnd AND the input has-tray point must BOTH read empty (a disabled point
    //counts as empty) -- a side is NOT retired while a tray still sits at the input, so every
    //tray inside the machine is processed before CleanOut finishes.
    if(IsSoftSimulate())
        return (IsContinuousFeed()==false);

    bool bSourceEmpty = (HSys.Sen.SnLoader_Inputend.Enable==false    || HSys.Sen.SnLoader_Inputend.IsOff());
    bool bInputEmpty  = (HSys.Sen.SnLoader_InputHasTray.Enable==false || HSys.Sen.SnLoader_InputHasTray.IsOff());
    return bSourceEmpty && bInputEmpty;
}
//---------------------------------------------------------------------------
bool TLoaderModule::IsOutputBottomOccupied()
{
    if(IsSoftSimulate())
        return bRearHasTray;
    else if(HSys.Sen.SnLoader_OutputBottomHasTray.Enable==true)
        return HSys.Sen.SnLoader_OutputBottomHasTray.IsOn();
    return false;
}
//---------------------------------------------------------------------------
//AI(ht160s-loader) 20260708 : NON-MUTATING value-identical twin of IsRearOccupied().
//IsRearOccupied() calls RefreshRearState() first, which reads the rear sensor and can
//WRITE bRearHasTray / clear bRearReadyForPick+bRearResidualAlarmed -- side effects that
//must not run from the IsLoaderYMoveSafe interlock (polled every Loader/SortArm Y-move
//tick) nor from the DescribeState state-record dump (documented read-only; Empty's
//ComputeRearPickReadyNoRefresh is the same pattern). Value-equivalence per config :
//sim -> bRearHasTray latch (RefreshRearState is a sim no-op, so IsRearOccupied returns
//the latch too); real+sensor-enabled -> live sensor (exactly what the refresh would
//latch); real+sensor-disabled -> refresh leaves the latch unchanged -> latch here too.
//Keep in lockstep with RefreshRearState if its semantics ever change.
bool TLoaderModule::PeekRearOccupied()
{
    if(IsSoftSimulate())
        return bRearHasTray;
    if(HSys.Sen.SnLoader_OutputBottomHasTray.Enable==true)
        return HSys.Sen.SnLoader_OutputBottomHasTray.IsOn();
    return bRearHasTray;
}
//---------------------------------------------------------------------------
void TLoaderModule::RefreshRearState()
{
    bool bHasRearSensor=false;
    bool bSensorState=false;

    //AI(HT160S-Maintainer) 20260609 : in simulation/DUMMY there is no rear tray sensor,
    //so the rear-occupied state is driven purely by the latched bRearHasTray flag : set
    //true when DoDischargeTray parks an empty tray at the rear (case 2000), cleared by
    //NotifyTrayArmPickRearTray when the TrayArm picks it. Recomputing from the absent
    //sensor here would wipe that latch and the TrayArm would never see the discharged
    //tray. HAS_TRAY mode is NOT soft-simulate, so it still reads the real sensor below.
    if(IsSoftSimulate())
        return;

    if(HSys.Sen.SnLoader_OutputBottomHasTray.Enable==true)
    {
        bHasRearSensor=true;
        if(HSys.Sen.SnLoader_OutputBottomHasTray.IsOn())
            bSensorState=true;
    }

    if(bHasRearSensor)
    {
        bRearHasTray=bSensorState;
        if(bSensorState==false)
        {
            //AI(ht160s-rearready-p0) 20260705 : no tray = not pickable, ever. Clears a
            //stale TRUE latch left by a hand-removed tray (NotifyTrayArmPickRearTray
            //never ran) so the published state always matches the physical rear -- a
            //stale TRUE would otherwise satisfy IsRearReadyForPick through the NEXT
            //discharge's landing window (case 1000, clamps still ON). The case-100
            //re-arm in DoDischargeTray is the primary guard; this is the belt. The
            //empty rear also re-arms the MES0924 residual Note for a fresh episode.
            bRearReadyForPick=false;
            bRearResidualAlarmed=false;
        }
    }
}
//---------------------------------------------------------------------------
bool TLoaderModule::IsRearOccupied()
{
    RefreshRearState();
    return bRearHasTray;
}
//---------------------------------------------------------------------------
bool TLoaderModule::IsRearHasTray()
{
    return IsRearOccupied();
}
//---------------------------------------------------------------------------
//AI(ht160s-trayarm-empty-handoff) 20260701 / AI(ht160s-rearready-state) 20260703 REDESIGN :
//"safe for TrayArm to grab" predicate. The rear is pickable ONLY after DoDischargeTray case 4000
//success publishes bRearReadyForPick=true (carriage retreated to feed Y + Push/Lean clamps already
//Pop-confirmed at cases 2000/3000; TMyCylinder::Pop does not advance the ladder until the Off
//sensor confirms on a REALLY machine). The latch is false through cases 1000..3000 and re-armed
//false at discharge start, so it ALSO covers the on-machine interference (mirrors Empty commit
//564154c) where RefreshRearState re-latches bRearHasTray the instant the carriage LANDS the tray
//(case 1000, both clamps still ON) : bRearReadyForPick is not set until case 4000, so a bare
//bRearHasTray can no longer read as pickable. This replaces the old "block while DischargeTask in
//1000..4000" step-range gate, whose inclusive <=4000 terminal boundary stranded the settled tray
//(DischargeTask latches at 4000 after a completed discharge) -> deadlock. See the function body.
bool TLoaderModule::IsRearReadyForPick()
{
    //AI(ht160s-rearready-state) 20260703 : REDESIGN -- gate on the producer-published
    //bRearReadyForPick latch (set only at DoDischargeTray case 4000 success), NOT on the
    //DoDischargeTray step number. Removes the cross-module step-range coupling and the old
    //>=1000 && <=4000 terminal off-by-one : DischargeTask latches at 4000 after a completed
    //discharge, so the inclusive <=4000 blocked the settled+pickable state forever -> TrayArm
    //never picked -> rear stranded -> deadlock. IsRearOccupied() stays as an independent
    //sensor/occupancy confirmation belt (state = source of truth, sensor = confirm). Deliberately
    //NO clamp out-bit belt : the Push/Lean clamps legitimately hold a tray at the FRONT all
    //through feed/CCD/sort, so an unconditional out-bit test would false-block TrayArm.
    return IsRearOccupied() && bRearReadyForPick;
}
//---------------------------------------------------------------------------
bool TLoaderModule::IsLoaderReadyForSort(int LoaderNo)
{
    TLoaderSideState *State=GetSide(LoaderNo);
    TTrayMotor *TrayMotor=NULL;

    if(State==NULL)
        return false;
    // Status is the single source of truth for the handshake: a side may be
    // handed to the SortArm only after the Top CCD has scanned EVERY cell
    // (LS_READY). LS_FEEDING / LS_CCD_SCAN / LS_SORTING are not pickable.
    if(State->Status!=LS_READY_SORT)
        return false;

    if(LoaderNo==1)
        TrayMotor=HSys.VMot.MMLoaderY_1;
    else if(LoaderNo==2)
        TrayMotor=HSys.VMot.MMLoaderY_2;

    if(TrayMotor==NULL || TrayMotor->fHasTray==false)
        return false;
   return (State->Status==LS_READY_SORT);
}
//---------------------------------------------------------------------------
int TLoaderModule::GetSortingLoaderNo()
{
    if(IsLoaderReadyForSort(1))
        return 1;
    if(IsLoaderReadyForSort(2))
        return 2;
    return 0;
}
//---------------------------------------------------------------------------
int TLoaderModule::GetLoaderStatus(int LoaderNo)
{
    TLoaderSideState *State=GetSide(LoaderNo);
    if(State==NULL)
        return LS_IDLE;
    return State->Status;
}
//---------------------------------------------------------------------------
// SortArm requests exclusive use of the Loader-Y axis. Granted only when the
// side has finished CCD scanning (IsLoaderReadyForSort) and the axis is free.
bool TLoaderModule::AcquireSortOwner(int LoaderNo)
{
    TLoaderSideState *State=GetSide(LoaderNo);
    if(IsValidLoaderNo(LoaderNo)==false || State==NULL)
        return false;
    int idx=GetSideIndex(LoaderNo);
    if(iYOwner[idx]==LOADER_Y_OWNER_SORTARM)
        return true;
    if(iYOwner[idx]!=LOADER_Y_OWNER_NONE)
        return false;
    if(IsLoaderReadyForSort(LoaderNo)==false)
        return false;
    iYOwner[idx]=LOADER_Y_OWNER_SORTARM;
    State->Status=LS_SORTING;
    return true;
}
//---------------------------------------------------------------------------
// SortArm returns the Loader-Y axis to the Loader after it has lifted Z clear.
void TLoaderModule::ReleaseSortOwner(int LoaderNo)
{
    TLoaderSideState *State=GetSide(LoaderNo);
    int idx;

    if(IsValidLoaderNo(LoaderNo)==false || State==NULL)
        return;
    idx=GetSideIndex(LoaderNo);
    if(iYOwner[idx]==LOADER_Y_OWNER_SORTARM)
        iYOwner[idx]=LOADER_Y_OWNER_NONE;
    if(State->Status==LS_SORTING)
        State->Status=LS_ToRear;
}
//---------------------------------------------------------------------------
// Re-validate the handshake just before SortArm drops Z to suck. Returns true
// only while SortArm still owns the Loader-Y axis and the side is LS_SORTING.
bool TLoaderModule::IsSortOwnerHeld(int LoaderNo)
{
    TLoaderSideState *State=GetSide(LoaderNo);

    if(IsValidLoaderNo(LoaderNo)==false || State==NULL)
        return false;
    if(iYOwner[GetSideIndex(LoaderNo)]!=LOADER_Y_OWNER_SORTARM)
        return false;
    return (State->Status==LS_SORTING);
}
//---------------------------------------------------------------------------
bool TLoaderModule::IsAllCleanOutFinish()
{
    //AI(HT160S-Maintainer) 20260605 : both Loader sides have drained in CleanOut.
    if(Side[0].bCleanOutFinish==false || Side[1].bCleanOutFinish==false)
        return false;
    //AI(cleanout) 20260701 : physical residual + supply-car gate. The old check trusted
    //only the per-side software carriage flag and could finish with a tray still parked at
    //the shared front feed position / rear output / supply car (on-machine 2026-07-01 : a
    //Loader empty tray was left behind). REALLY mode now requires the shared front, rear and
    //supply-car sensors all clear so no tray is left in the pipeline; SOFT_SIMULATE/DUMMY has
    //no IO card (InType=0 inputs read present), so it trusts the action latches there and
    //skips the raw sensor read (mirrors RefreshStateFromSensors early-out). Enable-gated so an
    //uninstalled point never blocks. Rear (bRearHasTray) is cleared by TrayArm recovery.
    if(IsSoftSimulate()==false)
    {
        if(HSys.Sen.SnLoader_InputHasTray.Enable && HSys.Sen.SnLoader_InputHasTray.IsOn())
            return false;   //residual tray on the front feed position
        if(HSys.Sen.SnLoader_OutputBottomHasTray.Enable && HSys.Sen.SnLoader_OutputBottomHasTray.IsOn())
            return false;   //residual tray at the rear output position
        if(HSys.Sen.SnLoader_Inputend.Enable && HSys.Sen.SnLoader_Inputend.IsOn())
            return false;   //supply car still has stock (drain it before finishing)
    }
    if(bRearHasTray)
        return false;       //rear latch still set : TrayArm has not recovered it yet
    return true;
}
//---------------------------------------------------------------------------
void TLoaderModule::NotifyTrayArmPickRearTray()
{
    //AI(ht160s-tray-source) 20260625 : Phase 6 A.5 - rear tray taken by TrayArm;
    //the data has been transferred to the arm, so clear the rear hold too
    //(extends the Phase 1-5 "cleared rear => cleared grid" invariant).
    bRearHasTray=false;
    bRearReadyForPick=false;   //AI(ht160s-rearready-state) 20260703 : tray taken -> rear no longer pickable
    RearKind=eTrayKindNormal;
    RearTrayID="";
    RearSourceTray.Clear();
}
//---------------------------------------------------------------------------
//AI(ht160s-tray-source) 20260625 : Phase 6 A.5 - rear-tray accessors (return-by-value).
eTrayKind TLoaderModule::GetRearTrayKind()
{
    return RearKind;
}
//---------------------------------------------------------------------------
TMyTray TLoaderModule::GetRearSourceTray()
{
    return RearSourceTray;
}
//---------------------------------------------------------------------------
AnsiString TLoaderModule::GetRearTrayID()
{
    return RearTrayID;
}
//---------------------------------------------------------------------------
bool TLoaderModule::AcquireFrontOwner(int LoaderNo)
{
    if(iFrontOwner==0)
        iFrontOwner=LoaderNo;
    return (iFrontOwner==LoaderNo);
}
//---------------------------------------------------------------------------
void TLoaderModule::ReleaseFrontOwner(int LoaderNo)
{
    if(iFrontOwner==LoaderNo)
        iFrontOwner=0;
}
//---------------------------------------------------------------------------
void TLoaderModule::PrepareTrayMap(int LoaderNo)
{
    TTrayMotor *TrayMotor=NULL;
    int XCount=GetTrayXCount();
    int YCount=GetTrayYCount();

    if(LoaderNo==1)
        TrayMotor=HSys.VMot.MMLoaderY_1;
    else if(LoaderNo==2)
        TrayMotor=HSys.VMot.MMLoaderY_2;
    if(TrayMotor==NULL)
        return;

    TrayMotor->InitNewTray(EMPTY_IC);
    for(int YIndex=0; YIndex<YCount; YIndex++)
        for(int XIndex=0; XIndex<XCount; XIndex++)
            TrayMotor->SetTraySingleData(XIndex, YIndex, UNCHECK_IC);
}
//---------------------------------------------------------------------------
void TLoaderModule::ChangeActiveTrayData(int LoaderNo, int SourceData, int TargetData)
{
    TTrayMotor *TrayMotor=NULL;
    int XCount=GetTrayXCount();
    int YCount=GetTrayYCount();

    if(LoaderNo==1)
        TrayMotor=HSys.VMot.MMLoaderY_1;
    else if(LoaderNo==2)
        TrayMotor=HSys.VMot.MMLoaderY_2;
    if(TrayMotor==NULL)
        return;

    for(int YIndex=0; YIndex<YCount; YIndex++)
        for(int XIndex=0; XIndex<XCount; XIndex++)
            if(TrayMotor->Tray.Data[XIndex][YIndex]==SourceData)
                TrayMotor->SetTraySingleData(XIndex, YIndex, TargetData);
}
//---------------------------------------------------------------------------
bool TLoaderModule::HasActiveTrayData(int LoaderNo, int Data)
{
    TTrayMotor *TrayMotor=NULL;
    int XCount=GetTrayXCount();
    int YCount=GetTrayYCount();

    if(LoaderNo==1)
        TrayMotor=HSys.VMot.MMLoaderY_1;
    else if(LoaderNo==2)
        TrayMotor=HSys.VMot.MMLoaderY_2;
    if(TrayMotor==NULL)
        return false;

    for(int YIndex=0; YIndex<YCount; YIndex++)
        for(int XIndex=0; XIndex<XCount; XIndex++)
            if(TrayMotor->Tray.Data[XIndex][YIndex]==Data)
                return true;
    return false;
}
//---------------------------------------------------------------------------
//AI(ht160s-sortarm) 20260625 : robust "this side still has pickable ICs" predicate
//used by DoSortArm case 1 to KEEP picking the active side until its tray is fully
//drained (operator rule : never abandon a half-picked tray). Reads ONLY the carriage
//Tray grid, NOT State->Status, so it is immune to the per-batch LS_ToRear transient
//(ReleaseSortOwner sets LS_ToRear, then DoLoader case 3000 flips it back to
//LS_READY_SORT). Pickable = a cell holding a real IC (data > UNCHECK_IC, i.e.
//HAS_OK_IC/NG), matching FindPickCells/IsPickableData. Requires fHasTray so a
//discharged (cleared) side reports false and may be released.
bool TLoaderModule::HasPickableIC(int LoaderNo)
{
    TTrayMotor *TrayMotor=NULL;
    int XCount=GetTrayXCount();
    int YCount=GetTrayYCount();

    if(LoaderNo==1)
        TrayMotor=HSys.VMot.MMLoaderY_1;
    else if(LoaderNo==2)
        TrayMotor=HSys.VMot.MMLoaderY_2;
    if(TrayMotor==NULL || TrayMotor->fHasTray==false)
        return false;

    for(int YIndex=0; YIndex<YCount; YIndex++)
        for(int XIndex=0; XIndex<XCount; XIndex++)
            if(TrayMotor->Tray.Data[XIndex][YIndex]>UNCHECK_IC)
                return true;
    return false;
}
//---------------------------------------------------------------------------
bool TLoaderModule::ActiveTrayAllData(int LoaderNo, int Data)
{
    TTrayMotor *TrayMotor=NULL;
    int XCount=GetTrayXCount();
    int YCount=GetTrayYCount();

    if(LoaderNo==1)
        TrayMotor=HSys.VMot.MMLoaderY_1;
    else if(LoaderNo==2)
        TrayMotor=HSys.VMot.MMLoaderY_2;
    if(TrayMotor==NULL)
        return false;

    for(int YIndex=0; YIndex<YCount; YIndex++)
        for(int XIndex=0; XIndex<XCount; XIndex++)
            if(TrayMotor->Tray.Data[XIndex][YIndex]!=Data)
                return false;
    return true;
}
//---------------------------------------------------------------------------
bool TLoaderModule::FindNextCcdCell(int LoaderNo, int &CellX, int &CellY)
{
    TTrayMotor *TrayMotor=NULL;
    TLoaderSideState *State=GetSide(LoaderNo);
    int XCount=GetTrayXCount();
    int YCount=GetTrayYCount();
    int XStart;
    int XEnd;
    int XStep;

    if(LoaderNo==1)
        TrayMotor=HSys.VMot.MMLoaderY_1;
    else if(LoaderNo==2)
        TrayMotor=HSys.VMot.MMLoaderY_2;
    if(TrayMotor==NULL || State==NULL)
        return false;

    for(int YIndex=0; YIndex<YCount; YIndex++)
    {
        if(State->bCcdLeftToRight)
        {
            XStart=0;
            XEnd=XCount;
            XStep=1;
        }
        else
        {
            XStart=XCount-1;
            XEnd=-1;
            XStep=-1;
        }

        for(int XIndex=XStart; XIndex!=XEnd; XIndex+=XStep)
        {
            if(TrayMotor->Tray.Data[XIndex][YIndex]==UNCHECK_IC)
            {
                CellX=XIndex;
                CellY=YIndex;
                if((State->bCcdLeftToRight && XIndex==XCount-1) ||
                   (!State->bCcdLeftToRight && XIndex==0))
                    State->bCcdLeftToRight=!State->bCcdLeftToRight;
                return true;
            }
        }
    }
    return false;
}
//---------------------------------------------------------------------------
int TLoaderModule::ReadTopCcdBin(int LoaderNo, int CellX, int CellY, bool &bOk)
{
    (void)LoaderNo;
    (void)CellX;
    (void)CellY;
    bOk=true;
    if(tFunction.UseCCD==false || IsSoftSimulate() || tSimuData.bRunSimulation)
        return HAS_OK_IC;
    bOk=false;
    return EMPTY_IC;
}
//---------------------------------------------------------------------------
AnsiString TLoaderModule::ReadTopCcd2DCode(int LoaderNo, int CellX, int CellY, bool &bOk)
{
    //AI(HT160S-Maintainer) 20260604 : P3 active. Non-blocking poll of Top CCD socket.
    //Trigger is issued by DoCcdCheck before entering the poll state. Returns the 2D
    //code with bOk=true once a reply is buffered; otherwise bOk=false (keep waiting).
    AnsiString sCode="";
    (void)LoaderNo;
    (void)CellX;
    (void)CellY;
    bOk=false;
    //AI(HT160S-Maintainer) 20260608 : simulation path : no real Top CCD hardware.
    //Cycle through the virtual 2D codes that btnLoadSimuDataClick registered, so
    //every scanned cell gets a valid, registry-resolvable code (wraps around).
    if(tSimuData.bRunSimulation || HSys.LastSet.iRealDummy!=REALLY || CosFunction.bUseTopCcd==false)
    {
        int Total=LotRegistry.GetItemCount();
        if(Total>0)
        {
            if(SimuCcdCycleIndex<0 || SimuCcdCycleIndex>=Total)
                SimuCcdCycleIndex=0;
            sCode=LotRegistry.GetCode2DByIndex(SimuCcdCycleIndex);
            SimuCcdCycleIndex=(SimuCcdCycleIndex+1)%Total;
            bOk=true;
        }
        return sCode;
    }
    if(TopCcdSocket!=NULL)
    {
        TopCcdSocket->TopCcdPoll();
        if(TopCcdSocket->TopCcdGetResult(sCode))
            bOk=true;
    }
    return sCode;
}
//---------------------------------------------------------------------------
void TLoaderModule::SetCurrentLotNumber(AnsiString Lot)
{
    CurrentLotNumber=Lot;
}
//---------------------------------------------------------------------------
void TLoaderModule::DoLoader(int LoaderNo, int &Task)
{
    TLoaderSideState *State=GetSide(LoaderNo);
    TLoaderSideState *OtherState=GetOtherSide(LoaderNo);
    TTrayMotor *TrayMotor=NULL;

    if(State==NULL)
        return;
    if(LoaderNo==1)
        TrayMotor=HSys.VMot.MMLoaderY_1;
    else
        TrayMotor=HSys.VMot.MMLoaderY_2;
    if(TrayMotor==NULL)
        return;

    //AI(ht160s-rearready-p0) 20260705 : rear-leftover watchdog (Loader mirror of Color
    //MES1426). A rear tray with NO published readiness and NO discharge in flight can
    //never become pickable -- bRearReadyForPick's only setter is DoDischargeTray case
    //4000 and a new discharge cannot start while the rear is occupied -- yet the rear
    //sensor keeps re-latching bRearHasTray, so TAJOB_LOADER_RECOVERY pins the TrayArm
    //with no alarm (silent starvation). Reached only when InitialFlag could NOT
    //preserve the rear hold (cold start over a leftover tray / mid-discharge abort) :
    //the tray's Kind/ID are unknown then, so auto-collecting risks misrouting a
    //cover/identity tray into the normal pool -- require operator removal (unlike the
    //RETIRED front MES0922, which has the case-9500 self-collect path). Sensor-aware
    //overload prints the IO name; fires once per episode (bRearResidualAlarmed),
    //re-armed on the sensor-empty edge / reset. LoaderNo==1 = evaluate once per cycle.
    if(LoaderNo==1 &&
       HSys.LastSet.iRealDummy!=DUMMY &&
       bRearResidualAlarmed==false &&
       bRearDischargeInProgress==false &&
       bRearReadyForPick==false &&
       IsRearOccupied())
    {
        bRearResidualAlarmed=true;
        ShowMyError("MES0924", LangT("Loader rear has a leftover tray - please remove it"),
                    &HSys.Sen.SnLoader_OutputBottomHasTray, false, K_RETRY);
    }

    //AI(cleanout) 20260703 : the MES0922 front-residual manual-removal alarm that lived here is
    //GONE (user design : the machine collects every tray itself). A front tray stranded after
    //the supply car went dry is now self-collected by the DoFeedTray case-9000 CleanOut branch
    //(-> case 9500 confirm-then-mint), and IsAllCleanOutFinish keeps the side unfinished while
    //SnLoader_InputHasTray still reads a tray, so nothing retires early.
    //AI(cleanout) 20260701 : phase-aware CleanOut finish. The old guard retired a side the
    //instant its carriage flag was empty, ignoring the shared front/rear sensors and the supply
    //car -> CleanOut could finish with trays still in the pipeline and never drained the supply
    //car. User-confirmed semantics: keep feeding + sorting the remaining supply car until it is
    //DRY, THEN empty the pipeline. So a side is "finished feeding" only when the shared supply
    //car is dry (SnLoader_Inputend OFF) AND its carriage is empty AND it does not own the sort Y.
    //If the car still has stock, fall through to the normal feed/sort/discharge flow so it drains
    //(source-dry no longer alarms MES0920/MES0921 in CleanOut - see DoFeedTray case 9000).
    if(HSys.Sys.RunMode==Run_CleanOut &&
       iYOwner[GetSideIndex(LoaderNo)]==LOADER_Y_OWNER_NONE)
    {
        bool bSupplyCarDry = IsSupplyCarDry();
        //AI(cleanout) 20260701 : do NOT retire a side while its last tray's rear discharge is still
        //in flight. DoDischargeTray clears the carriage (fHasTray=false) at case 3000 but only
        //retreats the carriage + clears bRearDischargeInProgress at case 4000. Retiring here (Task=1;
        //return) between 3000 and 4000 would abandon the discharge, leaving bRearDischargeInProgress
        //latched true forever -> IsRearReadyForPick() never true -> TrayArm can never recover the
        //rear tray -> bRearHasTray never clears -> IsAllCleanOutFinish hangs. Let the discharge finish.
        if(TrayMotor->fHasTray==false && bSupplyCarDry && bRearDischargeInProgress==false)
        {
            State->bCleanOutFinish=true;
            State->Status=LS_IDLE;
            Task=1;
            return;
        }
        //supply car still has stock (or carriage still loaded) : do NOT finish; the switch
        //below re-runs the normal feed/sort/discharge flow so the car drains first.
    }

    if(State->bTrayEmpty)
        return;

    switch(Task)
    {
        case 1:
            Task=10;
            break;

        case 10:
            //AI(HT160S-Maintainer) 20260610 : CleanOut finish now handled by the
            //every-cycle guard at the top of DoLoader (the old transient check
            //here could never fire once a side left case 10).
            Task=100;
            break;

        case 100:
            if(TrayMotor->fHasTray==false)
            {
                if(bAmrLocked)   //AI(ht160s-agv) 20260623 : AMR handoff - no new front destack/feed
                    break;
                //AI(ht160s-sortarm) 20260624 : pipelined (NOT strict alternation) but hold off STARTING a
                //feed while the OTHER car is in the FRONT ZONE - either FEEDING (at the feed pos) or
                //CCD_SCAN (at the CCD pos). Those two front stations sit closer than SafeDist (feed ~1mm
                //<-> CCD ~131mm = 130mm < 325mm), so feeding into that window would only get blocked by
                //the IsLoaderYMoveSafe backstop anyway / risk a stall. Waiting until the other car leaves
                //the front zone (it advances to sort/discharge, >=427mm, clear of SafeDist from feed) lets
                //this car feed without contention. Mirrors DoFeedTray's own downstream FEEDING/CCD_SCAN
                //guard. Still pipelined : once the other car is READY_SORT/SORTING/ToRear/IDLE this car
                //feeds while the other runs its cycle - the gate does NOT block on the other car merely
                //holding a tray (that was the earlier strict-alternation fHasTray form, since dropped).
                //IsLoaderYMoveSafe stays the per-move collision backstop for the both-loaded case.
                //Operator confirmed on-machine 20260624 : the two cars run without interfering.
                if(OtherState->Status==LS_FEEDING ||
                   OtherState->Status==LS_CCD_SCAN)
                {
                    break;
                }
                else
                {
                    State->Status=LS_FEEDING;
                    DoFeedTray(LoaderNo, 0);
                    Task=1000;
                }
            }
            else
            {
                State->Status=LS_CCD_SCAN;
                DoCcdCheck(LoaderNo, 0);
                Task=2000;
            }
            break;

        case 1000:
            if(DoFeedTray(LoaderNo, 1))
            {
                if(IsSoftSimulate() && iSimInfeedCount>0)
                    iSimInfeedCount--;   //AI(ht160s-agv) sim input drains 1/feed
                if(OtherState->Status==LS_CCD_SCAN)
                {
                    break;
                }
                else
                {
                    State->Status=LS_CCD_SCAN;
                    DoCcdCheck(LoaderNo, 0);
                    Task=2000;
                }
            }
            break;

        case 2000:
            if(DoCcdCheck(LoaderNo, 1))
                Task=3000;
            break;

        case 3000:
            if(iYOwner[GetSideIndex(LoaderNo)]!=LOADER_Y_OWNER_NONE)
                break;
            if(ActiveTrayAllData(LoaderNo, EMPTY_IC)==false)
            {
                State->Status=LS_READY_SORT;
                break;
            }
            if(OtherState->Status==LS_ToRear ||
               IsOutputBottomOccupied())
            {
                break;
            }
            else
            {
                State->Status=LS_ToRear;
                DoDischargeTray(LoaderNo, 0);
                Task=4000;
                break;
            }
        case 4000:
            if(DoDischargeTray(LoaderNo, 1))
            {
                State->Status=LS_IDLE;
                Task=1;
            }
            break;
        default:
            //AI(ht160s-ladder-guard) 20260703 : a state number with no matching case
            //(the 'number but no action' trap). Log it so a future dead-jump is a
            //diagnosable EventLog event, not a silent stall, and restart the ladder.
            LogLadderFault("Loader.DoLoader", Task);
            Task=1;
            break;
    }
}
//---------------------------------------------------------------------------
bool TLoaderModule::DoFeedTray(int LoaderNo, int Flag)
{
    TLoaderSideState *State=GetSide(LoaderNo);
    TLoaderSideState *OtherState=GetOtherSide(LoaderNo);
    TMyCylinder *PushCylinder=NULL;
    TMyCylinder *LeanCylinder=NULL;
    TTrayMotor *TrayMotor=NULL;
    int Ret;

    if(State==NULL || OtherState==NULL)
        return false;
    if(Flag==0)
    {
        State->FeedTask=1;
        State->FeedDelay.Clear();
        State->bWaitingAmrFeed=false;   //AI(ht160s-agv) 20260626 : fresh feed attempt re-arms AMR deferral
        State->FeedWaitTimer.Clear();
        return true;
    }
    if(OtherState->Status==LS_FEEDING ||
       OtherState->Status==LS_CCD_SCAN)
        return false;
    //AI(cleanout) 20260701 : feeding is NO LONGER blocked in CleanOut (was: return false
    //here). User-confirmed semantics: drain the supply car by feeding + sorting the remaining
    //trays until SnLoader_Inputend is dry, then empty the pipeline. Source-dry is CleanOut-aware
    //in case 9000 (no MES0920/MES0921 alarm) and the DoLoader finish guard retires the side once
    //the car is dry + carriage empty.
    if(LoaderNo==1)
    {
        PushCylinder=&HSys.Cyn.C_Loader1_PushTray;
        LeanCylinder=&HSys.Cyn.C_Loader1_LeanOnTray;
        TrayMotor=HSys.VMot.MMLoaderY_1;
    }
    else
    {
        PushCylinder=&HSys.Cyn.C_Loader2_PushTray;
        LeanCylinder=&HSys.Cyn.C_Loader2_LeanOnTray;
        TrayMotor=HSys.VMot.MMLoaderY_2;
    }
    if(PushCylinder==NULL || LeanCylinder==NULL || TrayMotor==NULL)
        return false;

    switch(State->FeedTask)
    {
        case 1:
            State->FeedTask=10;
            break;

        case 10:
            if(TrayMotor->fHasTray)
            {
                State->FeedTask=1;   //AI(ht160s-feeder-unify) 20260706 : return true -> reset task to idle(1)
                return true;
            }
            //AI(ht160s-home-resume-lk1) 20260713 : orphan-tray self-collect on resume. A
            //HOME taken between the destack landing (case 8300 clamps the tray on the
            //carriage) and the identity mint (case 9500) leaves a tray PHYSICALLY on the
            //carriage while fHasTray was reset false by InitialFlag. uHome's fHasTray-keyed
            //removal hint cannot see it, and the destack path below (case 100 -> 4000) would
            //drop a SECOND tray onto it (clamps/cuts it + scatters IC). Route straight to the
            //confirm-then-mint case 9500 instead, reusing the CleanOut case-9000 self-collect
            //predicate. REAL + Enable gate only : sim/DUMMY InType=0 phantom-present would
            //mint ghost trays forever. On-machine verify : SnLoader_InputHasTray must read
            //the carriage tray at this HOME-Y (the case-9000 mirror proves the read at
            //feed-Y). A false miss is no worse than today (falls through to destack); a false
            //hit routes to 9500 = JAM0913 safe stop, never a collision.
            if(IsSoftSimulate()==false && TrayMotor->fHasTray==false &&
               HSys.Sen.SnLoader_InputHasTray.Enable && HSys.Sen.SnLoader_InputHasTray.IsOn())
            {
                State->FeedTask=9500;
                break;
            }
            State->FeedTask=100;
            break;

        case 100:
            if(AcquireFrontOwner(LoaderNo)==false)
                return false;
            State->FeedTask=1000;
            break;

        case 1000:
            if(MoveLoaderY(LoaderNo, GetLoaderFeedY(LoaderNo)))
            {
                if(IsSoftSimulate())
                    State->FeedTask=3500;   //AI(ht160s-loader) 20260627 : sim also source-dry gated (case 3500)
                else
                    State->FeedTask=2000;
            }
            break;

        case 2000:
            if(PushCylinder->Pop())
                State->FeedTask=3000;
            break;

        case 3000:
            if(LeanCylinder->Pop())
                State->FeedTask=3500;   //AI(ht160s-loader) 20260627 : source-dry pre-gate before destack
            break;

        case 3500:
            //AI(ht160s-loader) 20260627 : source-dry pre-gate BEFORE the front destacker
            //fires (item 2). Test the supply-car stock sensor SnLoader_Inputend here so a
            //dry source never wastes a godown cycle. HT160 has no HT172/HT9045 SelectHasTray;
            //the established source-dry truth source is SnLoader_Inputend (same sensor the
            //AMR shortage / MES0921 path uses). On 'has stock' proceed to the destack (4000);
            //on 'dry' route to case 9000 to REUSE the single existing tray-empty handling
            //(MES0920 + AMR feed-wait deferral) - no duplicated alarm. Enable gate: a disabled
            //sensor is treated as present/non-blocking; sim/DUMMY uses chkLoadTray, not the
            //real sensor (same idiom as the case-9000 Inputend gate).
            if(IsSoftSimulate()
                   ? IsContinuousFeed()
                   : (HSys.Sen.SnLoader_Inputend.Enable==false
                      || HSys.Sen.SnLoader_Inputend.IsOn()))
            {
                State->FeedTask=4000;   //source has stock -> destack one tray
            }
            else
            {
                State->FeedTask=9000;   //source dry -> reuse case 9000 tray-empty handling
            }
            break;

        case 4000:
            //AI(general) 20260617 : front-destacker separate-one-tray now lives in the
            //shared DoFrontDestackDown so the Teach Advanced TestGoDownTray exercises the
            //identical cylinder sequence as this production feed.
            State->DestackTask=1;
            State->FeedTask=4100;
            break;

        case 4100:
            if(DoFrontDestackDown(State->DestackTask, State->FeedDelay))
                State->FeedTask=8200;
            break;

        case 8200:
            if(LeanCylinder->Push())
                State->FeedTask=8300;
            break;

        case 8300:
            if(PushCylinder->Push())
                State->FeedTask=9000;
            break;

        case 9000:
            //AI(ht160s-agv) 20260627 : AMR-on tray-count vs Inputend cross-check. iCarTrayTotal
            //is the FIXED physical magazine total (SECS LoaderTrayCount = IC + cover + identity,
            //latched at car arrival). Once iFeedSerial has consumed the whole total the count
            //says the car is drained; if SnLoader_Inputend still reads a tray the count and the
            //hardware disagree -> abnormal, raise MES0921 rather than feed a tray the count says
            //is not there. count==0 + Inputend OFF is the NORMAL source-dry case, left to the
            //deferral/MES0920 else-branch below. Disabled sensor never false-fires (Enable gate).
            if(GeneralSetting.bUseAMR
               && iCarTrayTotal>0
               && (iCarTrayTotal - iFeedSerial)<=0
               && HSys.Sen.SnLoader_Inputend.Enable==true
               && HSys.Sen.SnLoader_Inputend.IsOn())
            {
                //AI(cleanout) 20260701 : draining the supply car in CleanOut - do NOT raise the
                //count-mismatch alarm; break and let the DoLoader finish guard retire the side.
                if(HSys.Sys.RunMode==Run_CleanOut)
                    break;
                Ret=ShowMyError("MES0921", LangT("Loader Tray Count Mismatch"), K_RETRY|K_CLEAN_OUT);
                if(Ret==K_RETRY)
                    State->FeedTask=1;
                if(Ret==K_CLEAN_OUT)
                {
                    HSys.Sys.RunMode=Run_CleanOut;
                    HSys.Sys.bCleanOut=true;
                    State->FeedTask=10000;
                }
                break;
            }
            //AI(HT160S-Maintainer) 20260609 : in simulate/DUMMY the chkLoadTray
            //checkbox decides : checked = treat the tray as present (feed forever),
            //unchecked = fall through to the "Loader Tray Empty" alarm.
            //AI(ht160s-agv) 20260627 : real presence now ANDs the supply-car InputEnd
            //(SnLoader_Inputend ON = car still has stock - the source-dry truth the AGV-call
            //path already uses) with the push-cylinder On sensor (a tray actually reached the
            //destacker - kept as the physical-arrival interlock, do NOT lower it). A disabled
            //sensor is treated as present so an uninstalled point never blocks the feed.
            if(IsSoftSimulate()
                   ? IsContinuousFeed()
                   : ((HSys.Sen.SnLoader_Inputend.Enable==false || HSys.Sen.SnLoader_Inputend.IsOn())
                      && (PushCylinder->OnSensor.Enable==false || PushCylinder->OnSensor.IsOn())))
            {
                State->bWaitingAmrFeed=false;   //AI(ht160s-agv) 20260626 : tray present (incl. AMR refill arriving during the deferral wait) - clear the wait
                State->FeedWaitTimer.Clear();
                //AI(ht160s-loader) 20260627 : tray reached the destacker (Inputend + push
                //sensor). Confirm it actually landed on the LoaderY carriage in case 9500
                //BEFORE minting the tray identity (HT172/HT9045 confirm-then-mint order) so a
                //lost tray (SnLoader_InputHasTray OFF) leaves no phantom fHasTray / serial.
                State->FeedTask=9500;
            }
            else
            {
                //AI(cleanout) 20260701 : supply car drained in CleanOut - do NOT raise the
                //MES0920 "Loader Tray Empty" alarm. Break so DoFeedTray idles at case 9000 and
                //the DoLoader phase-aware finish guard (car dry + carriage empty) retires the side.
                //AI(cleanout) 20260703 : self-collect a stranded FRONT tray (user design : the
                //machine collects every tray itself; the MES0922 manual-removal alarm is gone).
                //If the real front sensor still sees a tray while the carriage is empty, route
                //to case 9500 : the confirm-then-mint path gives it an identity so the normal
                //feed -> CCD -> SortArm suck -> discharge -> TrayArm chain drains it. REAL only :
                //the sim/DUMMY InType=0 phantom-present read would mint ghost trays forever
                //(this exact bug lived in the on-site copy's version of this branch).
                if(HSys.Sys.RunMode==Run_CleanOut)
                {
                    if(IsSoftSimulate()==false && TrayMotor->fHasTray==false &&
                       HSys.Sen.SnLoader_InputHasTray.Enable && HSys.Sen.SnLoader_InputHasTray.IsOn())
                        State->FeedTask=9500;
                    break;
                }
                //AI(ht160s-agv) 20260626 : AMR-aware feed deferral (port of HT9046
                //asendic_Loader.cpp:1943 600s wait). When AMR feeds the magazine, do
                //NOT alarm the operator the instant the push cylinder reads empty :
                //give the called AGV time to refill, and only fall through to MES0920
                //on timeout. State is per-side (HT9046's func-static is illegal here -
                //both Loader sides share this body). Tray arrival is handled by the
                //if-branch above on a later cycle (real push-cylinder sensor), which is
                //the ONLY valid cancel : IsInputHandoffFinishedForAmr is sim-true and
                //would defeat the wait. bUseAMR off keeps today's immediate alarm.
                if(GeneralSetting.bUseAMR)
                {
                    if(State->bWaitingAmrFeed==false)
                    {
                        State->FeedWaitTimer.SetMS(GeneralSetting.iAmrFeedWaitSec*1000);
                        State->FeedWaitTimer.On();
                        State->bWaitingAmrFeed=true;
                        break;
                    }
                    if(State->FeedWaitTimer.Off()==false)
                        break;
                    State->bWaitingAmrFeed=false;
                    State->FeedWaitTimer.Clear();
                }
                Ret=ShowMyError("MES0920", LangT("Loader Tray Empty"), &HSys.Sen.SnLoader_Inputend, true, K_RETRY|K_CLEAN_OUT);
                if(Ret==K_RETRY)
                    //AI(ht160s-loader) 20260706 : resume at carriage-confirm (9500), NOT case 1.
                    //Restarting the feed re-drives DoFrontDestackDown (rise1+rise2) on a tray already
                    //separated below -> clamps/cuts it + scatters IC. 9500 confirms the existing tray
                    //(SnLoader_InputHasTray) and carries it on; JAM0913 there is a safe stop.
                    State->FeedTask=9500;
                if(Ret==K_CLEAN_OUT)
                {
                    //AI(HT160S-Maintainer) 20260605 : operator chose CleanOut at the
                    //tray-empty alarm : enter CleanOut run mode + latch so the whole
                    //machine drains (resume CleanOut if OneCycle runs mid-drain).
                    //AI(ht160s-loader) 20260706 : route to 9500 (confirm + carry the tray physically
                    //present) instead of dead-ending at 10000, so CleanOut still drains that last tray.
                    HSys.Sys.RunMode=Run_CleanOut;
                    HSys.Sys.bCleanOut=true;
                    State->FeedTask=9500;
                }
            }
            break;

        case 9500:
            //AI(ht160s-loader) 20260627 : post-godown carriage confirm - first use of the
            //previously-dead SnLoader_InputHasTray (item 3). Mirrors HT9045 DoLoadNewICTray
            //case 400 (SnLoaderCarHasTray confirm after destack; JAM0913 K_RETRY|K_SKIP on the
            //tray-to-carriage timeout) and HT172 Empty1 case 420. RealDummy tiers + Enable
            //gate: sim/DUMMY and a disabled point pass through (never block the feed - same
            //idiom as the case-9000 Inputend gate); only REAL/HAS_TRAY with Enable==true &&
            //IsOff() raises the alarm. Tray identity is minted HERE (confirm-then-mint) so
            //SKIP/RETRY need no rollback.
            if(IsSoftSimulate()
               || HSys.Sen.SnLoader_InputHasTray.Enable==false
               || HSys.Sen.SnLoader_InputHasTray.IsOn())
            {
                TrayMotor->fHasTray=true;
                PrepareTrayMap(LoaderNo);
                //tag this fed tray's kind on the carriage Tray grid (born here, mirrors Color
                //BirthIdentityTray). Identity trays get a sim TrayID; real machine leaves it
                //blank (no 2D read at feed, D2) and Color re-reads/re-births the 2D on reuse.
                iFeedSerial++;
                {
                    eTrayKind kFed=GetFedTrayKind(iFeedSerial, iCarTrayTotal);
                    TrayMotor->Tray.SetKind(kFed);
                    if(kFed==eTrayKindIdentity)
                        TrayMotor->Tray.TrayID = IsSoftSimulate()
                            ? (AnsiString("LOAD2D_")+Now().FormatString("hhnnsszzz"))
                            : AnsiString("");
                    else
                        TrayMotor->Tray.TrayID = "";
                }
                State->FeedTask=10000;
                break;
            }
            //SnLoader_InputHasTray OFF (Enable==true, not sim/dummy) after the godown => the
            //destacked tray did not land on the carriage : tray lost or sensor fault. JAM0913
            //mirrors HT9045 (carriage has-tray timeout). SKIP finishes this feed with no tray
            //(nothing was minted); RETRY re-runs the whole feed (case 10 fHasTray short-circuit
            //stays false because the mint never ran).
            Ret=ShowMyError("JAM0913", LangT("Loader Tray Lost On Carriage"), &HSys.Sen.SnLoader_InputHasTray, true, K_SKIP|K_RETRY);
            if(Ret==K_SKIP)
                State->FeedTask=10000;
            if(Ret==K_RETRY)
                State->FeedTask=1;
            break;

        case 10000:
            ReleaseFrontOwner(LoaderNo);
            State->FeedTask=1;   //AI(ht160s-feeder-unify) 20260706 : return true -> reset task to idle(1)
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TLoaderModule::DoCcdCheck(int LoaderNo, int Flag)
{
    TLoaderSideState *State=GetSide(LoaderNo);
    TLoaderSideState *OtherState=GetOtherSide(LoaderNo);
    TTrayMotor *TrayMotor=NULL;
    bool bCcdOk=false;
    int BinData;
    int Ret;

    if(State==NULL || OtherState==NULL)
        return false;
    if(Flag==0)
    {
        State->CcdTask=1;
        State->CcdDelay.Clear();
        return true;
    }
    if(LoaderNo==1)
        TrayMotor=HSys.VMot.MMLoaderY_1;
    else
        TrayMotor=HSys.VMot.MMLoaderY_2;
    if(TrayMotor==NULL)
        return false;

    switch(State->CcdTask)
    {
        case 1:
            State->CcdTask=1000;
            break;
        case 1000:
            if(HasActiveTrayData(LoaderNo, UNCHECK_IC))
                State->CcdTask=2000;
            else
            {
                if(OtherState->Status==LS_READY_SORT ||
                   OtherState->Status==LS_SORTING)
                {
                    //dont move
                }
                else
                {
                    State->Status=LS_READY_SORT;
                    State->CcdTask=1;   //AI(ht160s-feeder-unify) 20260706 : return true -> reset task to idle(1)
                    return true;
                }
            }
            break;

        case 2000:
            if(FindNextCcdCell(LoaderNo, State->CcdX, State->CcdY))
                State->CcdTask=3000;
            else
                State->CcdTask=1000;
            break;

        case 3000:
            if(MoveToCcdCell(LoaderNo, State->CcdX, State->CcdY))
            {
                State->CcdDelay.SetMS(100);
                State->CcdDelay.On();
                State->CcdTask=4000;
            }
            break;

        case 4000:
            if(State->CcdDelay.Off())
                State->CcdTask=5000;
            break;

        case 5000:
            BinData=ReadTopCcdBin(LoaderNo, State->CcdX, State->CcdY, bCcdOk);
            if(bCcdOk)
            {
                iTopCcdCount++;
                TrayMotor->SetTraySingleData(State->CcdX, State->CcdY, BinData);
                //AI(HT160S-Maintainer) 20260604 : P3 active. Trigger Top CCD shot, then
                //poll for the 2D code in state 5500. Only when the flag is on AND the Top
                //CCD socket is connected; otherwise behaviour is unchanged (go idle).
                //AI(HT160S-Maintainer) 20260612 : GAP A fix - the connect condition was
                //reversed (it reported "not ready" while actually connected, blocking the
                //2D scan on real hardware). Only run the 2D path when the bin-map feature
                //is on and the IC is good; require a connected Top CCD for real hardware,
                //while still letting the NULL-socket simulation path advance to state 5500.
                if(CosFunction.bUse2DBinMap && BinData==HAS_OK_IC)
                {
                    if(HSys.LastSet.iRealDummy==REALLY &&
                       TopCcdSocket!=NULL &&
                       TopCcdSocket->IsTopCcdConnected()==false && CosFunction.bUseTopCcd &&
                       IsSoftSimulate()==false)
                    {
                        Ret=ShowSystemError("TopCCD_Connect", K_RETRY|K_SKIP);
                        if(Ret==K_SKIP)
                        {
                            TrayMotor->SetTrayBin(State->CcdX, State->CcdY, HT160_BIN_ERROR_2D_SCAN_FAIL);
                            State->CcdTask=1;
                        }
                        break;
                    }
                    //Guard NULL socket so the simulation path (no Top CCD hardware) can
                    //still advance to the 2D-code poll state. Real hardware triggers a shot.
                    if(HSys.LastSet.iRealDummy==REALLY && CosFunction.bUseTopCcd && TopCcdSocket!=NULL)
                        TopCcdSocket->TopCcdTriggerShot();
                    State->CcdDelay.SetMS(3000);
                    State->CcdDelay.On();
                    State->CcdTask=5500;
                }
                else
                    State->CcdTask=1;
            }
            else
            {
                Ret=ShowMyError("WAR0330", LangT("Top CCD API not ready"), K_SKIP|K_RETRY|K_TRAY_END);
                if(Ret==K_RETRY)
                    State->CcdTask=3000;
                if(Ret==K_SKIP)
                {
                    iTopCcdCount++;
                    TrayMotor->SetTraySingleData(State->CcdX, State->CcdY, EMPTY_IC);
                    State->CcdTask=1;
                }
                if(Ret==K_TRAY_END)
                {
                    ChangeActiveTrayData(LoaderNo, UNCHECK_IC, EMPTY_IC);
                    State->CcdTask=1;
                }
            }
            break;

        case 5500:
            //AI(HT160S-Maintainer) 20260604 : P3 active. Poll Top CCD 2D code, lookup
            //Bin via Bin2DMap. On lookup-miss or no-response, alarm + Note Retry/Skip:
            //Retry re-triggers a shot; Skip routes the IC as Error (no-bin-setting).
            {
                bool b2DOk=false;
                AnsiString sCode=ReadTopCcd2DCode(LoaderNo, State->CcdX, State->CcdY, b2DOk);
                if(b2DOk)
                {
                    //AI(HT160S-Maintainer) 20260604 : P3 multi-lot reverse lookup.
                    //The IC 2D code is globally unique across all loaded Lots, so the
                    //code alone resolves both the owning Lot and its Bin (no need to
                    //know the Lot first). Replaces the old single-lot forward Lookup.
                    int Bin=0;
                    AnsiString HitLot;
                    int HitLotIndex=-1;
                    MachineRun.iTotalScanned++;
                    if(LotRegistry.FindByCode2D(sCode, HitLot, Bin, HitLotIndex))
                    {
                        TrayMotor->SetTrayBin(State->CcdX, State->CcdY, Bin);
                        //AI(ht160s-lotbin) 20260615 : By Lot+Bin mode. Carry owning lot
                        //and 2D code on the cell. ICs are scanned in physical order, so
                        //ResolveAuto here binds each new (Lot,Bin) to the next free Auto
                        //first-come-first-served; placement later just reads the binding.
                        TrayMotor->SetTrayLot(State->CcdX, State->CcdY, HitLotIndex);
                        TrayMotor->SetTrayCode2D(State->CcdX, State->CcdY, sCode);
                        //AI(ht160s-lotpassfail) 20260709 : freeze the PASS/FAIL class at scan
                        //(mirror SetTrayBin) so routing + the PassFail log read the SAME class.
                        //Bind on the mode's key : Bin for Lot+Bin, PASS/FAIL(1/2) for Lot+PassFail
                        //(class 0 = error/off -> no binding; read path routes it to the Error Auto).
                        {
                            int PassClass=BinAreaMap.GetPassFailClass(Bin);
                            TrayMotor->SetTrayPassClass(State->CcdX, State->CcdY, PassClass);
                            if(GeneralSetting.IsLotBinSortMode())
                                LotBinBinding.ResolveAuto(HitLotIndex, Bin);
                            else if(GeneralSetting.IsLotPassFailSortMode() && PassClass>0)
                                LotBinBinding.ResolveAuto(HitLotIndex, PassClass);
                        }
                        LotRegistry.OnSorted(HitLotIndex, Bin);
                        //AI(ht160s-lot-reset) 20260706 : global per-Bin production count
                        //(HT172 parity: aSortArm/aMagArm bump BinICCnt[Bin] on sort). HT160
                        //resolves Bin here at scan (like iTotalSorted), so count it here.
                        if(Bin>=0 && Bin<TEST_MAX_BIN)
                            tRunData.BinICCnt[Bin]++;
                        MachineRun.iTotalSorted++;
                        if(TopCcdSocket!=NULL)
                            TopCcdSocket->TopCcdEndShot();   //AI(HT160S-Maintainer) 20260612 : align HT172 LOFF (GAP C)
                        State->CcdTask=1;
                    }
                    else
                    {
                        Ret=ShowMyError("WAR0475", LangT("2D code not found in any lot : ")+sCode, K_RETRY|K_SKIP|K_MANUAL_2D);
                        if(Ret==K_RETRY)
                        {
                            if(TopCcdSocket!=NULL)
                                TopCcdSocket->TopCcdTriggerShot();
                            State->CcdDelay.SetMS(3000);
                            State->CcdDelay.On();
                        }
                        else if(Ret==K_MANUAL_2D)
                            BindManual2D(State, TrayMotor);
                        else
                        {
                            MachineRun.iUnknown2D++;
                            TrayMotor->SetTrayBin(State->CcdX, State->CcdY, HT160_BIN_ERROR_NO_BIN_SETTING);
                            //AI(ht160s-lotbin) 20260615 : no owning lot -> route to Error Auto.
                            TrayMotor->SetTrayLot(State->CcdX, State->CcdY, -1);
                            TrayMotor->SetTrayCode2D(State->CcdX, State->CcdY, sCode);
                            if(TopCcdSocket!=NULL)
                                TopCcdSocket->TopCcdEndShot();   //AI(HT160S-Maintainer) 20260612 : align HT172 LOFF (GAP C)
                            State->CcdTask=1;
                        }
                    }
                }
                else if(State->CcdDelay.Off())
                {
                    Ret=ShowSystemError("TopCCD_2D", K_RETRY|K_SKIP|K_MANUAL_2D);
                    if(Ret==K_RETRY)
                    {
                        if(TopCcdSocket!=NULL)
                            TopCcdSocket->TopCcdTriggerShot();
                        State->CcdDelay.SetMS(3000);
                        State->CcdDelay.On();
                    }
                    else if(Ret==K_MANUAL_2D)
                        BindManual2D(State, TrayMotor);
                    else
                    {
                        TrayMotor->SetTrayBin(State->CcdX, State->CcdY, HT160_BIN_ERROR_NO_BIN_SETTING);
                        //AI(ht160s-lotbin) 20260615 : 2D no-response -> no owning lot -> Error Auto.
                        TrayMotor->SetTrayLot(State->CcdX, State->CcdY, -1);
                        TrayMotor->SetTrayCode2D(State->CcdX, State->CcdY, "");
                        if(TopCcdSocket!=NULL)
                            TopCcdSocket->TopCcdEndShot();   //AI(HT160S-Maintainer) 20260612 : align HT172 LOFF (GAP C)
                        State->CcdTask=1;
                    }
                }
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
void TLoaderModule::BindManual2D(TLoaderSideState *State, TTrayMotor *TrayMotor)
{
    //AI(ht160s-ccd-manual2d) : operator-supplied Top CCD 2D for one IC cell. Reuses the
    //scan-success bind path (Bin/Lot resolve + ResolveAuto + OnSorted + EndShot) so a
    //hand-entered code is treated like a real read, plus a Manual2D trace flag. On a
    //still-unknown code the operator keeps getting the prompt (Retry re-scans, Skip
    //routes the IC to Error) -- never silently dropped. This NEVER resumes the machine;
    //the operator presses Start to run it (operator boundary). Bounded by iGuard so a
    //stuck dialog can never spin forever -- every normal branch returns first.
    int iGuard;
    for(iGuard=0; iGuard<100; iGuard++)
    {
        AnsiString code=fNote->ManualText.Trim();
        int Bin=0;
        AnsiString HitLot;
        int HitLotIndex=-1;
        if(code!="" && LotRegistry.FindByCode2D(code, HitLot, Bin, HitLotIndex))
        {
            TrayMotor->SetTrayBin(State->CcdX, State->CcdY, Bin);
            TrayMotor->SetTrayLot(State->CcdX, State->CcdY, HitLotIndex);
            TrayMotor->SetTrayCode2D(State->CcdX, State->CcdY, code);
            TrayMotor->SetTrayManual2D(State->CcdX, State->CcdY, true);
            //AI(ht160s-lotpassfail) 20260709 : freeze class + bind on the mode key (see scan-success path).
            {
                int PassClass=BinAreaMap.GetPassFailClass(Bin);
                TrayMotor->SetTrayPassClass(State->CcdX, State->CcdY, PassClass);
                if(GeneralSetting.IsLotBinSortMode())
                    LotBinBinding.ResolveAuto(HitLotIndex, Bin);
                else if(GeneralSetting.IsLotPassFailSortMode() && PassClass>0)
                    LotBinBinding.ResolveAuto(HitLotIndex, PassClass);
            }
            LotRegistry.OnSorted(HitLotIndex, Bin);
            //AI(ht160s-lot-reset) 20260706 : per-Bin count on manual-2D sort too.
            if(Bin>=0 && Bin<TEST_MAX_BIN)
                tRunData.BinICCnt[Bin]++;
            MachineRun.iTotalSorted++;
            if(TopCcdSocket!=NULL)
                TopCcdSocket->TopCcdEndShot();
            State->CcdTask=1;
            return;
        }
        int Ret2=ShowMyError("WAR0475", LangT("2D code not found in any lot : ")+code, K_RETRY|K_SKIP|K_MANUAL_2D);
        if(Ret2==K_RETRY)
        {
            if(TopCcdSocket!=NULL)
                TopCcdSocket->TopCcdTriggerShot();
            State->CcdDelay.SetMS(3000);
            State->CcdDelay.On();
            return;
        }
        if(Ret2==K_SKIP)
        {
            MachineRun.iUnknown2D++;
            TrayMotor->SetTrayBin(State->CcdX, State->CcdY, HT160_BIN_ERROR_NO_BIN_SETTING);
            TrayMotor->SetTrayLot(State->CcdX, State->CcdY, -1);
            TrayMotor->SetTrayCode2D(State->CcdX, State->CcdY, code);
            if(TopCcdSocket!=NULL)
                TopCcdSocket->TopCcdEndShot();
            State->CcdTask=1;
            return;
        }
    }
    //Safety backstop (operator chose manual >100x without resolving): route to Error so
    //control always returns -- the loop can never run unbounded.
    MachineRun.iUnknown2D++;
    TrayMotor->SetTrayBin(State->CcdX, State->CcdY, HT160_BIN_ERROR_NO_BIN_SETTING);
    TrayMotor->SetTrayLot(State->CcdX, State->CcdY, -1);
    TrayMotor->SetTrayCode2D(State->CcdX, State->CcdY, fNote->ManualText.Trim());
    if(TopCcdSocket!=NULL)
        TopCcdSocket->TopCcdEndShot();
    State->CcdTask=1;
}
//---------------------------------------------------------------------------
bool TLoaderModule::DoDischargeTray(int LoaderNo, int Flag)
{
    TLoaderSideState *State=GetSide(LoaderNo);
    TTrayMotor *TrayMotor=NULL;
    TMyCylinder *PushCylinder=NULL;
    TMyCylinder *LeanCylinder=NULL;
    int Pos1=0;
    int Pos2=0;

    if(State==NULL)
        return false;
    int &Task = State->DischargeTask;
    if(Flag==0)
    {
        Task=1;
        return true;
    }

    if(LoaderNo==1)
    {
        TrayMotor=HSys.VMot.MMLoaderY_1;
        PushCylinder=&HSys.Cyn.C_Loader1_PushTray;
        LeanCylinder=&HSys.Cyn.C_Loader1_LeanOnTray;
    }
    else
    {
        TrayMotor=HSys.VMot.MMLoaderY_2;
        PushCylinder=&HSys.Cyn.C_Loader2_PushTray;
        LeanCylinder=&HSys.Cyn.C_Loader2_LeanOnTray;
    }
    if(TrayMotor==NULL || PushCylinder==NULL || LeanCylinder==NULL)
        return false;

    switch(Task)
    {
        case 1:
            Task=10;
            break;

        case 10:
            Pos1=HSys.Mot.MLoaderY_1->ReadEncoderPos();
            Pos2=HSys.Mot.MLoaderY_2->ReadEncoderPos();
            if(LoaderNo==1)
            {
                if(Pos2>Pos1 || IsOutputBottomOccupied())
                    return false;
            }
            else
            {
                if(Pos1>Pos2 || IsOutputBottomOccupied())
                    return false;
            }
            Task=100;
            break;

        case 100:
            if(IsRearOccupied())
                return false;
            //AI(ht160s-rearready-p0) 20260705 : commit point -- the carriage is about to
            //move rear-ward with the tray. Arm the in-flight flag and kill any stale
            //ready latch HERE, not at case 2000 : on a REALLY machine the rear sensor
            //re-latches bRearHasTray the moment the carriage LANDS at case 1000 --
            //BEFORE case 2000 runs -- so a latch left TRUE (hand-removed tray, Notify
            //never ran) would satisfy IsRearReadyForPick through the landing window
            //(clamps still ON = the on-machine early-pick interference class). Arming
            //bRearDischargeInProgress here also lets the DoLoader rear-residual check
            //tell "discharge in flight" from "stranded leftover" without peeking at
            //DischargeTask step numbers.
            bRearDischargeInProgress=true;
            bRearReadyForPick=false;
            Task=1000;
            break;

        case 1000:
            if(MoveLoaderY(LoaderNo, GetLoaderDischargeY(LoaderNo)))
            {
                if(IsSoftSimulate()==false &&
                   HSys.Sen.SnLoader_OutputBottomHasTray.Enable==true &&
                   HSys.Sen.SnLoader_OutputBottomHasTray.IsOn()==false)
                {
                    int ret=ShowMyError(LangT("Loader Tray has IC,please remove"), K_RETRY|K_SKIP);
                    if(ret==K_RETRY)
                    {
                        break;
                    }
                }
                PushCylinder->Reset();
                LeanCylinder->Reset();
                Task=2000;
            }
            break;
        case 2000:
            if(PushCylinder->Pop() || IsSoftSimulate())
            {
                bRearHasTray=true;   //AI(ht160s-rearready-p0) 20260705 : landing latch for sim/DUMMY (a REALLY machine re-latches this from the rear sensor at case-1000 landing); the in-flight + ready flags are armed earlier, at the case-100 commit
                Task=3000;
            }
            break;

        case 3000:
            if(LeanCylinder->Pop() || IsSoftSimulate())
            {
                //AI(ht160s-tray-source) 20260625 : Phase 6 A.4 - transfer the tray's
                //data into the module-level rear hold BEFORE ClearTray releases the
                //carriage (U3 transfer-chain relay; carriage is reused by the next feed).
                RearKind       = TrayMotor->Tray.GetKind();
                RearTrayID     = TrayMotor->Tray.TrayID;
                RearSourceTray = TrayMotor->Tray;
                TrayMotor->ClearTray();
                Task=4000;
            }
            break;

        case 4000:
            if(MoveLoaderY(LoaderNo, GetLoaderFeedY(LoaderNo)))
            {
                bRearDischargeInProgress=false;   //AI(ht160s-trayarm-empty-handoff) 20260701 : carriage retreated to feed Y; rear tray now settled + safe for TrayArm to pick
                bRearReadyForPick=true;   //AI(ht160s-rearready-state) 20260703 : PUBLISH readiness at the physically-safe instant (carriage retreated + clamps popped) -- the single set point the TrayArm pick gate reads
                Task=1;   //AI(ht160s-feeder-unify) 20260706 : return true -> reset task to idle(1) (Task = State->DischargeTask ref)
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(general) 20260617 : shared front-destacker "separate one tray down" sequence.
//Extracted verbatim from DoFeedTray case 4000-8100 so the Teach Advanced
//TestGoDownTray drives the IDENTICAL cylinders/steps as the production feed (no
//drift). Cylinder-only (no Y / push / lean); destacker cylinders are shared by both
//sides so no LoaderNo. Caller owns the SubTask + settle Delay. Returns true when done.
bool TLoaderModule::DoFrontDestackDown(int &SubTask, HTimer &Delay)
{
    //AI(ht160s-feeder-unify) 20260706 : converted from .On()/.IsOn()||sim + .Off()/.Pop() to the
    //cylinder's own confirmed .Push()/.Pop() (in-position confirm + alarm-on-timeout) with a
    //one-shot .Reset() re-arm before each poll - mirrors Empty/Color DoGoDownTray. TLoaderModule
    //has no PushCylinder wrapper (that name is a local TMyCylinder* here), so .Push()/.Pop() are
    //called directly (they are sim-safe via #ifdef and Enable-aware internally). SHARED by the
    //production DoFeedTray (case 3500) AND the Teach TestGoDownTray. Physical order, the single
    //Delay.Set(10) settle after Separate extend, and the Loader<->Empty interlock are unchanged.
    switch(SubTask)
    {
        case 1:
            HSys.Cyn.C_Loader_FrontRiseTray_1.Reset();
            SubTask=2;
            break;

        case 2:
            if(HSys.Cyn.C_Loader_FrontRiseTray_1.Push())   // extend Rise_1 (confirmed)
            {
                HSys.Cyn.C_Loader_FrontRiseTray_2.Reset();
                SubTask=3;
            }
            break;

        case 3:
            if(HSys.Cyn.C_Loader_FrontRiseTray_2.Push())   // extend Rise_2 (confirmed)
            {
                HSys.Cyn.C_Loader_FrontSeparateTray_1.Reset();
                SubTask=4;
            }
            break;

        case 4:
            //PRESERVED: Loader<->Empty front-separate interlock.
            if(IsFrontSeparateBlockedBy(HSys.Cyn.C_Empty_FrontSeparateTray_1))
                break;   // interlock: wait while Empty front-separate is out
            if(HSys.Cyn.C_Loader_FrontSeparateTray_1.Push())   // extend Separate (confirmed)
            {
                Delay.Set(10);
                Delay.On();
                HSys.Cyn.C_Loader_FrontRiseTray_2.Reset();
                SubTask=5;
            }
            break;

        case 5:
            if(Delay.Off())
            {
                if(HSys.Cyn.C_Loader_FrontRiseTray_2.Pop())   // retract Rise_2 (confirmed)
                {
                    HSys.Cyn.C_Loader_FrontSeparateTray_1.Reset();
                    SubTask=6;
                }
            }
            break;

        case 6:
            //AI(ht160s-feeder-unify) 20260706 : PRESERVED safety gate from the original case 5 -
            //do not release the separator claw unless Rise_1 still holds the stack (else the remaining
            //stack could drop). This Loader-specific gate has no GoDown/GoUp analog; keep it verbatim.
            if(HSys.Cyn.C_Loader_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                if(HSys.Cyn.C_Loader_FrontSeparateTray_1.Pop())   // retract Separate (confirmed)
                {
                    HSys.Cyn.C_Loader_FrontRiseTray_1.Reset();
                    SubTask=7;
                }
            }
            break;

        case 7:
            if(HSys.Cyn.C_Loader_FrontRiseTray_1.Pop())   // retract Rise_1 (confirmed)
            {
                SubTask=1;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(general) 20260617 : Teach Advanced GoDown test = the production destacker
//separate-one-tray sequence (shared DoFrontDestackDown), so test == Auto-run sub-action.
bool TLoaderModule::TestGoDownTray(int Flag)
{
    if(Flag==0)
    {
        TestDownTask=1;
        TestDelay.Clear();
        return true;
    }
    return DoFrontDestackDown(TestDownTask, TestDelay);
}
//---------------------------------------------------------------------------
//AI(general) 20260617 : Teach Advanced destacker test. Cylinder-only GoUp
//(return one tray up into the stack) mirrors Empty DoGoUpTray rise steps 100-600.
//AI(ht160s-home-resume-drain) 20260711 : W2 drain hook. Pump the pure-cylinder destack
//segment (FeedTask 4000/4100/8200/8300, incl. the DoFrontDestackDown sub-ladder) to the
//9000 ENTRY boundary (LK-3 arbitration : drain never executes 9000+ - modal + mint; an
//interrupted un-minted tray is closed by the resume-side self-adopt instead). Same-scan
//pumping with the Empty hook resolves the shared front-separate interlock (LK-5).
bool TLoaderModule::HomeDrainTick()
{
    bool bDone=true;
    for(int LoaderNo=1; LoaderNo<=2; LoaderNo++)
    {
        TLoaderSideState *S=GetSide(LoaderNo);
        if(S==NULL)
            continue;
        if(S->FeedTask==4000 || S->FeedTask==4100 || S->FeedTask==8200 || S->FeedTask==8300)
        {
            DoFeedTray(LoaderNo, 1);
            if(S->FeedTask==4000 || S->FeedTask==4100 || S->FeedTask==8200 || S->FeedTask==8300)
                bDone=false;
        }
    }
    return bDone;
}
//---------------------------------------------------------------------------
bool TLoaderModule::TestGoUpTray(int Flag)
{
    if(Flag==0)
    {
        TestUpTask=1;
        TestDelay.Clear();
        return true;
    }

    //AI(ht160s-feeder-unify) 20260706 : idiom aligned to DoFrontDestackDown - the cylinder's own
    //confirmed .Push()/.Pop() (TLoaderModule has no PushCylinder wrapper) + one-shot .Reset()
    //re-arm; settle profile and the Loader<->Empty interlock unchanged. Teach-only.
    switch(TestUpTask)
    {
        case 1:
            HSys.Cyn.C_Loader_FrontRiseTray_1.Reset();
            TestUpTask=100;
            break;

        case 100:
            if(HSys.Cyn.C_Loader_FrontRiseTray_1.Push())
                TestUpTask=200;
            break;

        case 200:
            if(IsFrontSeparateBlockedBy(HSys.Cyn.C_Empty_FrontSeparateTray_1))
                break;   // interlock: wait while Empty front-separate is out
            HSys.Cyn.C_Loader_FrontSeparateTray_1.Reset();
            TestUpTask=210;
            break;

        case 210:
            if(HSys.Cyn.C_Loader_FrontSeparateTray_1.Push())
            {
                TestDelay.SetMS(GeneralSetting.iLoaderDestackSettleMs);
                TestDelay.On();
                TestUpTask=300;
            }
            break;

        case 300:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Loader_FrontRiseTray_2.Reset();
                TestUpTask=310;
            }
            break;

        case 310:
            if(HSys.Cyn.C_Loader_FrontRiseTray_2.Push())
                TestUpTask=400;
            break;

        case 400:
            HSys.Cyn.C_Loader_FrontSeparateTray_1.Reset();
            TestUpTask=410;
            break;

        case 410:
            if(HSys.Cyn.C_Loader_FrontSeparateTray_1.Pop())
            {
                TestDelay.SetMS(GeneralSetting.iLoaderDestackSettleMs);
                TestDelay.On();
                TestUpTask=500;
            }
            break;

        case 500:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Loader_FrontRiseTray_2.Reset();
                TestUpTask=510;
            }
            break;

        case 510:
            if(HSys.Cyn.C_Loader_FrontRiseTray_2.Pop())
            {
                HSys.Cyn.C_Loader_FrontRiseTray_1.Reset();
                TestUpTask=600;
            }
            break;

        case 600:
            if(HSys.Cyn.C_Loader_FrontRiseTray_1.Pop())
            {
                TestUpTask=1;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(ht160s-state-record-analysis) 20260622 : eLoaderStatus -> short text for FeederDecision.txt.
static AnsiString SR_LoaderStatusText(int St)
{
    switch(St)
    {
        case LS_IDLE:       return "IDLE";
        case LS_FEEDING:    return "FEEDING";
        case LS_CCD_SCAN:   return "CCD_SCAN";
        case LS_READY_SORT: return "READY_SORT";
        case LS_SORTING:    return "SORTING";
        case LS_ToRear:     return "ToRear";
    }
    return "?" + IntToStr(St);
}
//---------------------------------------------------------------------------
AnsiString TLoaderModule::DescribeState()
{
    //AI(ht160s-state-record-analysis) 20260622 : read-only per-side inner-state for
    //FeederDecision.txt. AllEmpty/HasOK expose the discharge gate : a side stuck at
    //READY_SORT with HasOK=1 AllEmpty=0 is the stranded-cell pick/discharge deadlock.
    AnsiString s;
    s  = "[Loader]\r\n";
    s += "  bRearHasTray=" + IntToStr(bRearHasTray ? 1 : 0)
       + "  bRearDischargeInProgress=" + IntToStr(bRearDischargeInProgress ? 1 : 0)
       + "  bRearReadyForPick=" + IntToStr(bRearReadyForPick ? 1 : 0)
       + "  bRearResidualAlarmed=" + IntToStr(bRearResidualAlarmed ? 1 : 0)   //AI(ht160s-rearready-p0) 20260705 : MES0924 once-latch
       //AI(ht160s-rearready-state) 20260703 : verdict now reads the published latch (single source
       //of truth with IsRearReadyForPick). Do NOT call IsRearReadyForPick() here -- it would refresh
       //sensors inside the state-record dump path; bRearHasTray && bRearReadyForPick mirrors it.
       + "  RearPickReady=" + IntToStr((bRearHasTray && bRearReadyForPick) ? 1 : 0)
       + "  iFrontOwner=" + IntToStr(iFrontOwner)
       + "  iYOwner=[" + IntToStr(iYOwner[0]) + "," + IntToStr(iYOwner[1]) + "]"
       + "  iTopCcdCount=" + IntToStr(iTopCcdCount)
       + "  SoftSim=" + IntToStr(IsSoftSimulate() ? 1 : 0) + "\r\n";
    //AI(ht160s-tray-source) 20260625 : Phase 6 A.6 - rear-tray hold + feed serial.
    s += "  RearKind=" + IntToStr((int)RearKind)
       + "  RearTrayID=" + RearTrayID
       + "  iFeedSerial=" + IntToStr(iFeedSerial)
       + "  iCarTrayTotal=" + IntToStr(iCarTrayTotal)
       + "  iSecsCarTrayCount=" + IntToStr(iSecsCarTrayCount)
       + "  Remain=" + IntToStr(iCarTrayTotal - iFeedSerial) + "\r\n";
    for(int n=1; n<=2; n++)
    {
        TLoaderSideState *St = GetSide(n);
        if(St==NULL)
            continue;
        TTrayMotor *Tm = (n==1) ? HSys.VMot.MMLoaderY_1 : HSys.VMot.MMLoaderY_2;
        bool bHas = (Tm!=NULL && Tm->fHasTray);
        s += "  Side" + IntToStr(n) + ": Status=" + SR_LoaderStatusText(St->Status)
           + "  Feed=" + IntToStr(St->FeedTask)
           + "  Ccd=" + IntToStr(St->CcdTask)
           + "  Disc=" + IntToStr(St->DischargeTask)
           + "  Destack=" + IntToStr(St->DestackTask) + "\r\n";
        s += "         fHasTray=" + IntToStr(bHas ? 1 : 0)
           + "  TrayEmptyFlag=" + IntToStr(St->bTrayEmpty ? 1 : 0)
           + "  CleanOutFin=" + IntToStr(St->bCleanOutFinish ? 1 : 0)
           + "  AllEmpty=" + IntToStr(ActiveTrayAllData(n, EMPTY_IC) ? 1 : 0)
           + "  HasOK=" + IntToStr(HasActiveTrayData(n, HAS_OK_IC) ? 1 : 0)
           + "  HasUncheck=" + IntToStr(HasActiveTrayData(n, UNCHECK_IC) ? 1 : 0) + "\r\n";
    }
    //AI(ht160s-state-record-analysis) 20260624 : cross-side Loader-Y interlock geometry.
    //A hang where one side sits LS_ToRear while the other refuses to advance to READY_SORT is
    //almost always IsLoaderYMoveSafe rejecting a move because the two shared-rail cars are
    //closer than SafeDist. Record both car encoder positions, the live |gap|, the SafeDist
    //limit, and a per-side feed/discharge move verdict so the blocked move is visible in the
    //snapshot directly, instead of being hand-computed from tech.ini + General.ini after the fact.
    {
        int p1 = (HSys.Mot.MLoaderY_1!=NULL) ? HSys.Mot.MLoaderY_1->ReadEncoderPos() : 0;
        int p2 = (HSys.Mot.MLoaderY_2!=NULL) ? HSys.Mot.MLoaderY_2->ReadEncoderPos() : 0;
        int g = p1-p2;
        if(g<0)
            g=-g;
        s += "[LoaderY interlock]\r\n";
        s += "  SafeDist=" + IntToStr(GeneralSetting.iLoaderYSafeDistance)
           + "  Y1enc=" + IntToStr(p1)
           + "  Y2enc=" + IntToStr(p2)
           + "  |gap|=" + IntToStr(g) + "\r\n";
        for(int k=1; k<=2; k++)
        {
            int fy = GetLoaderFeedY(k);
            int dy = GetLoaderDischargeY(k);
            int sy = (k==1) ? Teach.Loader1CarFirstSortYPosition : Teach.Loader2CarFirstSortYPosition;   //AI(ht160s-state-record-analysis) 20260625 : first sort row Y (diagnostic only)
            //AI(ht160s-loader) 20260708 : log the computed verdict WITH the refusing rule
            //(rear-rest / gap:*) -- a bare BLOCK was three-way ambiguous in a hang snapshot.
            //Same six IsLoaderYMoveSafe calls as before (now side-effect-free via Peek).
            AnsiString wf="", wd="", ws="";
            bool okF=IsLoaderYMoveSafe(k, fy, &wf);
            bool okD=IsLoaderYMoveSafe(k, dy, &wd);
            bool okS=IsLoaderYMoveSafe(k, sy, &ws);
            s += "  Side" + IntToStr(k)
               + " ->feed(" + IntToStr(fy) + ")=" + (okF ? AnsiString("OK") : AnsiString("BLOCK(")+wf+AnsiString(")"))
               + "  ->disc(" + IntToStr(dy) + ")=" + (okD ? AnsiString("OK") : AnsiString("BLOCK(")+wd+AnsiString(")"))
               + "  ->sort(" + IntToStr(sy) + ")=" + (okS ? AnsiString("OK") : AnsiString("BLOCK(")+ws+AnsiString(")"))
               + "\r\n";
        }
    }
    return s;
}
//---------------------------------------------------------------------------
void InitializeLoaderModule()
{
    if(LoaderModule==NULL)
        LoaderModule=new TLoaderModule;
}
//---------------------------------------------------------------------------
void ShutdownLoaderModule()
{
    delete LoaderModule;
    LoaderModule=NULL;
}
//---------------------------------------------------------------------------
