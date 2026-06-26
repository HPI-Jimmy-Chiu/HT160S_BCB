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
void TLoaderModule::InitialFlag()
{
    bAmrLocked=false;
    RefillSimInfeed();
    ResetSide(&Side[0]);
    ResetSide(&Side[1]);
    bRearHasTray=false;
    iFrontOwner=0;
    iTopCcdCount=0;
    iYOwner[0]=LOADER_Y_OWNER_NONE;
    iYOwner[1]=LOADER_Y_OWNER_NONE;
    SimuCcdCycleIndex=0;
    iFeedSerial=0;            //AI(ht160s-tray-source) 20260625 : Phase 6 A.2 - reset feed counter
    RearKind=eTrayKindNormal;
    RearTrayID="";
    RearSourceTray.Clear();
    CurrentLotNumber="";
    TestUpTask=1;
    TestDownTask=1;
    TestDelay.Clear();
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
static const int LOADER_X_DATUM_BIAS=-1000;   //AI(ht160s-maintainer) 20260624 : P2 datum bias, mirrors aSortArm
static const int LOADER_Y_DATUM_BIAS=-1000;
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
        ShowMyMessage(LangT("Loader Y motor will out of limit"), Motor->SoftLimitDetail(Position));
        return false;
    }
    return Motor->MotorMove(Position);
}
//---------------------------------------------------------------------------
bool TLoaderModule::IsLoaderYMoveSafe(int LoaderNo, int Position)
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

    if(OtherMotor==NULL || OtherTray==NULL)
        return true;

    //AI(HT160S-Maintainer) 20260610 : "opposite-side tray clamped" condition.
    //fHasTray is the logical tray-held state set true after the Push/Lean clamp
    //completes in DoFeedTray, so it is the available proxy for a clamped tray.
    if(OtherTray->fHasTray==false)
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
        return false;
    }
    //this car trails : stay at least SafeDist behind
    if(Tgt<=OtherPos-GeneralSetting.iLoaderYSafeDistance)
        return true;
    return false;
}
//---------------------------------------------------------------------------
bool TLoaderModule::MoveTopCcdX(int Position)
{
    if(HSys.Mot.MTopCCDX==NULL)
        return false;
    if(HSys.Mot.MTopCCDX->CheckSoftLimit(Position)==false)
    {
        ShowMyMessage(LangT("Top CCD X motor will out of limit"), HSys.Mot.MTopCCDX->SoftLimitDetail(Position));
        return false;
    }
    return HSys.Mot.MTopCCDX->MotorMove(Position);
}
//---------------------------------------------------------------------------
bool TLoaderModule::MoveToCcdCell(int LoaderNo, int CellX, int CellY)
{
    double DatumX=0.0;
    double DatumY=0.0;
    if(GeneralSetting.bUseTrayDatumModel)
    {
        DatumX=(double)LOADER_X_DATUM_BIAS+GetTrayXStart();
        DatumY=(double)LOADER_Y_DATUM_BIAS+GetTrayYStart();
    }
    int XPos=RoundPosition((double)GetTopCcdFirstX()+DatumX+((double)CellX)*GetTrayXPitch());
    int YPos=RoundPosition((double)GetLoaderFirstCcdY(LoaderNo)+DatumY+((double)CellY)*GetTrayYPitch());
    bool bXFlag=MoveTopCcdX(XPos);
    bool bYFlag=MoveLoaderY(LoaderNo, YPos);
    return (bXFlag && bYFlag);
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
    iSimInfeedCount=GeneralSetting.iSimAmrMaxTray[0];
    iFeedSerial=0;            //AI(ht160s-tray-source) 20260625 : Phase 6 A.2 - new car => restart feed serial
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
bool TLoaderModule::IsOutputBottomOccupied()
{
    if(IsSoftSimulate())
        return bRearHasTray;
    else if(HSys.Sen.SnLoader_OutputBottomHasTray.Enable==true)
        return HSys.Sen.SnLoader_OutputBottomHasTray.IsOn();
    return false;
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
        bRearHasTray=bSensorState;
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
    return (Side[0].bCleanOutFinish && Side[1].bCleanOutFinish);
}
//---------------------------------------------------------------------------
void TLoaderModule::NotifyTrayArmPickRearTray()
{
    //AI(ht160s-tray-source) 20260625 : Phase 6 A.5 - rear tray taken by TrayArm;
    //the data has been transferred to the arm, so clear the rear hold too
    //(extends the Phase 1-5 "cleared rear => cleared grid" invariant).
    bRearHasTray=false;
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

    //AI(HT160S-Maintainer) 20260610 : evaluate CleanOut finish EVERY cycle, not
    //only at the transient case 10. During CleanOut DoFeedTray is blocked, so a
    //side that was mid-feed (Task 1000) or waiting on the other side (Task 100)
    //never loops back to case 10 and the old check could never fire : CleanOut
    //hung forever (state record 2026-06-10 09_21_52 : Loader1=1000, Loader2=100
    //both frozen ~2 min). A side with no tray (and not owned by SortArm) has
    //nothing left to drain : declare it finished regardless of Task. A side still
    //holding a tray (fHasTray==true) keeps its normal sort/discharge flow and
    //finishes on a later cycle once the tray has drained. bTrayEmpty sides also
    //finish here because they have no tray to drain.
    if(HSys.Sys.RunMode==Run_CleanOut &&
       TrayMotor->fHasTray==false &&
       iYOwner[GetSideIndex(LoaderNo)]==LOADER_Y_OWNER_NONE)
    {
        State->bCleanOutFinish=true;
        State->Status=LS_IDLE;
        Task=1;
        return;
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
        return true;
    }
    if(OtherState->Status==LS_FEEDING ||
       OtherState->Status==LS_CCD_SCAN)
        return false;
    if(HSys.Sys.RunMode==Run_CleanOut)
        return false;
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
                return true;
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
                    State->FeedTask=4000;
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
                State->FeedTask=4000;
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
            //AI(HT160S-Maintainer) 20260609 : in simulate/DUMMY the chkLoadTray
            //checkbox decides : checked = treat the tray as present (feed forever),
            //unchecked = fall through to the "Loader Tray Empty" alarm. Real mode is
            //unchanged : the push-cylinder On sensor still governs tray presence.
            if(IsSoftSimulate()
                   ? IsContinuousFeed()
                   : (PushCylinder->OnSensor.Enable==false || PushCylinder->OnSensor.IsOn()))
            {
                TrayMotor->fHasTray=true;
                PrepareTrayMap(LoaderNo);
                //AI(ht160s-tray-source) 20260625 : Phase 6 A.3 - tag this fed tray's
                //kind on the carriage Tray grid (born here, mirrors Color BirthIdentityTray).
                //Identity trays get a sim TrayID; real machine leaves it blank (no 2D read at
                //feed, D2) and Color re-reads/re-births the 2D on reuse.
                iFeedSerial++;
                {
                    eTrayKind kFed=GetFedTrayKind(iFeedSerial, GetCarTrayCount());
                    TrayMotor->Tray.SetKind(kFed);
                    if(kFed==eTrayKindIdentity)
                        TrayMotor->Tray.TrayID = IsSoftSimulate()
                            ? (AnsiString("LOAD2D_")+Now().FormatString("hhnnsszzz"))
                            : AnsiString("");
                    else
                        TrayMotor->Tray.TrayID = "";
                }
                State->FeedTask=10000;
            }
            else
            {
                Ret=ShowMyError("MES0920", "Loader Tray Empty", K_RETRY|K_TRAY_END|K_CLEAN_OUT);
                if(Ret==K_RETRY)
                    State->FeedTask=1;
                if(Ret==K_TRAY_END)
                {
                    State->bTrayEmpty=true;
                    State->FeedTask=10000;
                }
                if(Ret==K_CLEAN_OUT)
                {
                    //AI(HT160S-Maintainer) 20260605 : operator chose CleanOut at the
                    //tray-empty alarm : enter CleanOut run mode + latch so the whole
                    //machine drains (resume CleanOut if OneCycle runs mid-drain).
                    HSys.Sys.RunMode=Run_CleanOut;
                    HSys.Sys.bCleanOut=true;
                    State->FeedTask=10000;
                }
            }
            break;

        case 10000:
            ReleaseFrontOwner(LoaderNo);
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
                Ret=ShowMyError("WAR0330", "Top CCD API not ready", K_SKIP|K_RETRY|K_TRAY_END);
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
                        if(GeneralSetting.bUseLotBinSortMode)
                            LotBinBinding.ResolveAuto(HitLotIndex, Bin);
                        LotRegistry.OnSorted(HitLotIndex, Bin);
                        MachineRun.iTotalSorted++;
                        if(TopCcdSocket!=NULL)
                            TopCcdSocket->TopCcdEndShot();   //AI(HT160S-Maintainer) 20260612 : align HT172 LOFF (GAP C)
                        State->CcdTask=1;
                    }
                    else
                    {
                        Ret=ShowMyError("WAR0475", "2D code not found in any lot : "+sCode, K_RETRY|K_SKIP|K_MANUAL_2D);
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
            if(GeneralSetting.bUseLotBinSortMode)
                LotBinBinding.ResolveAuto(HitLotIndex, Bin);
            LotRegistry.OnSorted(HitLotIndex, Bin);
            MachineRun.iTotalSorted++;
            if(TopCcdSocket!=NULL)
                TopCcdSocket->TopCcdEndShot();
            State->CcdTask=1;
            return;
        }
        int Ret2=ShowMyError("WAR0475", "2D code not found in any lot : "+code, K_RETRY|K_SKIP|K_MANUAL_2D);
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
            Task=1000;
            break;

        case 1000:
            if(MoveLoaderY(LoaderNo, GetLoaderDischargeY(LoaderNo)))
            {
                if(IsSoftSimulate()==false &&
                   HSys.Sen.SnLoader_OutputBottomHasTray.Enable==true &&
                   HSys.Sen.SnLoader_OutputBottomHasTray.IsOn()==false)
                {
                    int ret=ShowMyError("Loader Tray has IC,please remove", K_RETRY|K_SKIP);
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
                bRearHasTray=true;
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
                Task=5000;
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
    switch(SubTask)
    {
        case 1:
            HSys.Cyn.C_Loader_FrontRiseTray_1.On();
            SubTask=2;
            break;

        case 2:
            if(HSys.Cyn.C_Loader_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Loader_FrontRiseTray_2.On();
                SubTask=3;
            }
            break;

        case 3:
            if(HSys.Cyn.C_Loader_FrontRiseTray_2.IsOn() || IsSoftSimulate())
            {
                if(IsFrontSeparateBlockedBy(HSys.Cyn.C_Empty_FrontSeparateTray_1))
                    break;   // interlock: wait while Empty front-separate is out
                HSys.Cyn.C_Loader_FrontSeparateTray_1.On();
                Delay.Set(10);
                Delay.On();
                SubTask=4;
            }
            break;

        case 4:
            if(Delay.Off())
            {
                HSys.Cyn.C_Loader_FrontRiseTray_2.Off();
                SubTask=5;
            }
            break;

        case 5:
            if(HSys.Cyn.C_Loader_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Loader_FrontSeparateTray_1.Off();
                SubTask=6;
            }
            break;

        case 6:
            if(HSys.Cyn.C_Loader_FrontRiseTray_1.Pop())
            {
                HSys.Cyn.C_Loader_FrontRiseTray_1.Off();
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
bool TLoaderModule::TestGoUpTray(int Flag)
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
            HSys.Cyn.C_Loader_FrontRiseTray_1.On();
            TestUpTask=200;
            break;

        case 200:
            if(HSys.Cyn.C_Loader_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                if(IsFrontSeparateBlockedBy(HSys.Cyn.C_Empty_FrontSeparateTray_1))
                    break;   // interlock: wait while Empty front-separate is out
                HSys.Cyn.C_Loader_FrontSeparateTray_1.On();
                TestDelay.Set(10);
                TestDelay.On();
                TestUpTask=300;
            }
            break;

        case 300:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Loader_FrontRiseTray_2.On();
                TestUpTask=400;
            }
            break;

        case 400:
            if(HSys.Cyn.C_Loader_FrontRiseTray_2.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Loader_FrontSeparateTray_1.Off();
                TestDelay.Set(10);
                TestDelay.On();
                TestUpTask=500;
            }
            break;

        case 500:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Loader_FrontRiseTray_2.Off();
                if(HSys.Cyn.C_Loader_FrontRiseTray_1.IsOn() || IsSoftSimulate())
                    TestUpTask=600;
            }
            break;

        case 600:
            if(HSys.Cyn.C_Loader_FrontRiseTray_1.Pop() || IsSoftSimulate())
            {
                HSys.Cyn.C_Loader_FrontRiseTray_1.Off();
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
       + "  iFrontOwner=" + IntToStr(iFrontOwner)
       + "  iYOwner=[" + IntToStr(iYOwner[0]) + "," + IntToStr(iYOwner[1]) + "]"
       + "  iTopCcdCount=" + IntToStr(iTopCcdCount)
       + "  SoftSim=" + IntToStr(IsSoftSimulate() ? 1 : 0) + "\r\n";
    //AI(ht160s-tray-source) 20260625 : Phase 6 A.6 - rear-tray hold + feed serial.
    s += "  RearKind=" + IntToStr((int)RearKind)
       + "  RearTrayID=" + RearTrayID
       + "  iFeedSerial=" + IntToStr(iFeedSerial) + "\r\n";
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
            s += "  Side" + IntToStr(k)
               + " ->feed(" + IntToStr(fy) + ")=" + AnsiString(IsLoaderYMoveSafe(k, fy) ? "OK" : "BLOCK")
               + "  ->disc(" + IntToStr(dy) + ")=" + AnsiString(IsLoaderYMoveSafe(k, dy) ? "OK" : "BLOCK")
               + "  ->sort(" + IntToStr(sy) + ")=" + AnsiString(IsLoaderYMoveSafe(k, sy) ? "OK" : "BLOCK")
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
