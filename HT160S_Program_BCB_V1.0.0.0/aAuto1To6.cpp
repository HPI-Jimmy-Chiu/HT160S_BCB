#include "IncludeAllHeader.h"       //Dell 將.h統一,可加速build
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop
#include "language.h"

#include "aAuto1To6.h"
#include "database.h"
#include "cmydef.h"
#include "mymessbox.h"
#include "uteach.h"
#include "SecsGem\uHGemEquipment.h"
#include "GeneralSetting.h"   //AI(HT160S-Maintainer) 20260605 : GeneralSetting.bUseAMR mode switch
#include "CosFunction.h"      //AI(ht160s-state-record-analysis) 20260616 : TrayForm recipe geometry for DescribeStation cell map
#include "aSortArm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TAutoModule *AutoModule=NULL;
//---------------------------------------------------------------------------
static const int AUTO_STATION_COUNT=6;
//AI(ht160s-agv) 20260615 : simulation output-car-full threshold (trays) for the AGV
//  AGVSupplement trigger. Real machine uses the SnAutoX_InputFullTray sensor instead.
static const int AMR_FULL_TRAY_SIM=10;
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260623 : IsSensorOnReady/IsCylinderOnReady moved to mycylin.h/.cpp (shared by Empty/Loader/Auto)
//---------------------------------------------------------------------------
static bool IsCylinderOffReady(TMyCylinder *Cylinder, bool bSoftSimulate)
{
    if(Cylinder==NULL)
        return false;
    if(bSoftSimulate)
        return true;
    if(Cylinder->OffSensor.Enable==false)
        return true;
    return Cylinder->OffSensor.IsOn();
}
//---------------------------------------------------------------------------
static bool AreAllFlagsOn(bool *Flag)
{
    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
        if(Flag[Index]==false)
            return false;
    return true;
}
//---------------------------------------------------------------------------
TAutoModule::TAutoModule()
{
    InitialFlag();
}
//---------------------------------------------------------------------------
void TAutoModule::InitialFlag(bool bKeepMaterial)
{
    TTrayMotor *TrayMotor=NULL;

    FeedTask=1;
    DischargeTask=1;
    CleanOutTask=1;
    iFeedAuto=-1;
    iDischargeAuto=-1;
    FeedDelay.Clear();
    DischargeDelay.Clear();
    CleanOutDelay.Clear();
    DischargeSubTask=1;
    TestUpTask=1;
    TestDelay.Clear();

    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
    {
        TrayMotor=GetAutoVMotor(Index);
        State[Index].bCarHasTray=(TrayMotor!=NULL && TrayMotor->fHasTray);
        State[Index].bCleanOutFinish=false;
        State[Index].bResidueClear=true;   //AI(ht160s-residue) 20260624 : clear place-residue gate on home/init
        bCleanOutCheck[Index]=false;
        bAmrLocked[Index]=false;   //AI(ht160s-agv) 20260615 : drop any AGV handoff lock on home/init
        //AI(HT160S-Maintainer) 20260612 : on a recoverable home keep the car stack + its
        //tray roles/2D identity so the Auto does not forget what it is holding. Only the
        //sensor-backed presence above and the cleanout transient flags are refreshed.
        if(bKeepMaterial)
            continue;
        State[Index].bRearHasTray=false;
        State[Index].bRearCanUse=false;
        State[Index].bFrontHasTray=false;
        State[Index].bFullIC=false;
        bRearDeliveredPending[Index]=false;  //AI(general) 20260608 : Stage0 latch reset
        RearKind[Index]=eTrayKindNormal;     //AI(HT160S-Maintainer) 20260605 : AMR reset
        WorkingKind[Index]=eTrayKindNormal;  //AI(HT160S-Maintainer) 20260605 : AMR reset
        RearTrayID[Index]="";                //AI(HT160S-Maintainer) 20260608 : AMR 2D TrayID reset
        WorkingTrayID[Index]="";             //AI(HT160S-Maintainer) 20260608 : AMR 2D TrayID reset
        RearGrid[Index].Clear();             //AI(ht160s-tray-source) : cleared rear => cleared staged grid (sensor re-latch must not feed a stale grid)
        //AI(HT160S-Maintainer) 20260604 : reset stacking-car data + roles
        Car[Index].Clear();
        InitAutoCarStack(Index);
    }
}
//---------------------------------------------------------------------------
bool TAutoModule::IsSoftSimulate()
{
    #ifdef SOFT_SIMULATE
    return true;
    #else
    return (HSys.LastSet.iRealDummy==DUMMY);
    #endif
}
//---------------------------------------------------------------------------
TTrayMotor *TAutoModule::GetAutoMotor(int Index)
{
    switch(Index)
    {
        case 0: return HSys.Mot.MAutoY_1;
        case 1: return HSys.Mot.MAutoY_2;
        case 2: return HSys.Mot.MAutoY_3;
        case 3: return HSys.Mot.MAutoY_4;
        case 4: return HSys.Mot.MAutoY_5;
        case 5: return HSys.Mot.MAutoY_6;
    }
    return NULL;
}
//---------------------------------------------------------------------------
TTrayMotor *TAutoModule::GetAutoVMotor(int Index)
{
    switch(Index)
    {
        case 0: return HSys.VMot.MMAutoY_1;
        case 1: return HSys.VMot.MMAutoY_2;
        case 2: return HSys.VMot.MMAutoY_3;
        case 3: return HSys.VMot.MMAutoY_4;
        case 4: return HSys.VMot.MMAutoY_5;
        case 5: return HSys.VMot.MMAutoY_6;
    }
    return NULL;
}
//---------------------------------------------------------------------------
int TAutoModule::GetAutoFeedY(int Index)
{
    switch(Index)
    {
        case 0: return Teach.Auto1CarFeedTrayYPosition;
        case 1: return Teach.Auto2CarFeedTrayYPosition;
        case 2: return Teach.Auto3CarFeedTrayYPosition;
        case 3: return Teach.Auto4CarFeedTrayYPosition;
        case 4: return Teach.Auto5CarFeedTrayYPosition;
        case 5: return Teach.Auto6CarFeedTrayYPosition;
    }
    return 0;
}
//---------------------------------------------------------------------------
int TAutoModule::GetAutoDischargeY(int Index)
{
    switch(Index)
    {
        case 0: return Teach.Auto1CarDischargeTrayYPosition;
        case 1: return Teach.Auto2CarDischargeTrayYPosition;
        case 2: return Teach.Auto3CarDischargeTrayYPosition;
        case 3: return Teach.Auto4CarDischargeTrayYPosition;
        case 4: return Teach.Auto5CarDischargeTrayYPosition;
        case 5: return Teach.Auto6CarDischargeTrayYPosition;
    }
    return 0;
}
//---------------------------------------------------------------------------
int TAutoModule::GetAutoFirstSortY(int Index)
{
    switch(Index)
    {
        case 0: return Teach.Auto1CarFirstSortYPosition;
        case 1: return Teach.Auto2CarFirstSortYPosition;
        case 2: return Teach.Auto3CarFirstSortYPosition;
        case 3: return Teach.Auto4CarFirstSortYPosition;
        case 4: return Teach.Auto5CarFirstSortYPosition;
        case 5: return Teach.Auto6CarFirstSortYPosition;
    }
    return 0;
}
//---------------------------------------------------------------------------
TMyCylinder *TAutoModule::GetPush(int Index)
{
    switch(Index)
    {
        case 0: return &HSys.Cyn.C_Auto1_PushTray;
        case 1: return &HSys.Cyn.C_Auto2_PushTray;
        case 2: return &HSys.Cyn.C_Auto3_PushTray;
        case 3: return &HSys.Cyn.C_Auto4_PushTray;
        case 4: return &HSys.Cyn.C_Auto5_PushTray;
        case 5: return &HSys.Cyn.C_Auto6_PushTray;
    }
    return NULL;
}
//---------------------------------------------------------------------------
TMyCylinder *TAutoModule::GetLean(int Index)
{
    switch(Index)
    {
        case 0: return &HSys.Cyn.C_Auto1_LeanOnTray;
        case 1: return &HSys.Cyn.C_Auto2_LeanOnTray;
        case 2: return &HSys.Cyn.C_Auto3_LeanOnTray;
        case 3: return &HSys.Cyn.C_Auto4_LeanOnTray;
        case 4: return &HSys.Cyn.C_Auto5_LeanOnTray;
        case 5: return &HSys.Cyn.C_Auto6_LeanOnTray;
    }
    return NULL;
}
//---------------------------------------------------------------------------
TMyCylinder *TAutoModule::GetFrontRise(int Index)
{
    switch(Index)
    {
        case 0: return &HSys.Cyn.C_Auto1_FrontRiseTray;
        case 1: return &HSys.Cyn.C_Auto2_FrontRiseTray;
        case 2: return &HSys.Cyn.C_Auto3_FrontRiseTray;
        case 3: return &HSys.Cyn.C_Auto4_FrontRiseTray;
        case 4: return &HSys.Cyn.C_Auto5_FrontRiseTray;
        case 5: return &HSys.Cyn.C_Auto6_FrontRiseTray;
    }
    return NULL;
}
//---------------------------------------------------------------------------
TMySensor *TAutoModule::GetInputHasTray(int Index)
{
    switch(Index)
    {
        case 0: return &HSys.Sen.SnAuto1_InputHasTray;
        case 1: return &HSys.Sen.SnAuto2_InputHasTray;
        case 2: return &HSys.Sen.SnAuto3_InputHasTray;
        case 3: return &HSys.Sen.SnAuto4_InputHasTray;
        case 4: return &HSys.Sen.SnAuto5_InputHasTray;
        case 5: return &HSys.Sen.SnAuto6_InputHasTray;
    }
    return NULL;
}
//---------------------------------------------------------------------------
TMySensor *TAutoModule::GetInputFullTray(int Index)
{
    switch(Index)
    {
        case 0: return &HSys.Sen.SnAuto1_InputFullTray;
        case 1: return &HSys.Sen.SnAuto2_InputFullTray;
        case 2: return &HSys.Sen.SnAuto3_InputFullTray;
        case 3: return &HSys.Sen.SnAuto4_InputFullTray;
        case 4: return &HSys.Sen.SnAuto5_InputFullTray;
        case 5: return &HSys.Sen.SnAuto6_InputFullTray;
    }
    return NULL;
}
//---------------------------------------------------------------------------
TMySensor *TAutoModule::GetInputEndSensor(int Index)
{
    switch(Index)
    {
        case 0: return &HSys.Sen.SnAuto1_InputEnd;
        case 1: return &HSys.Sen.SnAuto2_InputEnd;
        case 2: return &HSys.Sen.SnAuto3_InputEnd;
        case 3: return &HSys.Sen.SnAuto4_InputEnd;
        case 4: return &HSys.Sen.SnAuto5_InputEnd;
        case 5: return &HSys.Sen.SnAuto6_InputEnd;
    }
    return NULL;
}
//---------------------------------------------------------------------------
TMySensor *TAutoModule::GetOutputBottomHasTray(int Index)
{
    switch(Index)
    {
        case 0: return &HSys.Sen.SnAuto1_OutputBottomHasTray;
        case 1: return &HSys.Sen.SnAuto2_OutputBottomHasTray;
        case 2: return &HSys.Sen.SnAuto3_OutputBottomHasTray;
        case 3: return &HSys.Sen.SnAuto4_OutputBottomHasTray;
        case 4: return &HSys.Sen.SnAuto5_OutputBottomHasTray;
        case 5: return &HSys.Sen.SnAuto6_OutputBottomHasTray;
    }
    return NULL;
}
//---------------------------------------------------------------------------
bool TAutoModule::MoveAutoY(int Index, int Position)
{
    TTrayMotor *Motor=GetAutoMotor(Index);

    if(Motor==NULL)
        return false;
    if(Motor->CheckSoftLimit(Position)==false)
    {
        ShowMyMessage(LangT("Auto Y motor will out of limit"), Motor->SoftLimitDetail(Position));
        return false;
    }
    return Motor->MotorMove(Position);
}
//---------------------------------------------------------------------------
void TAutoModule::RefreshAutoState()
{
    TMySensor *InputSensor=NULL;
    TMySensor *InputFullSensor=NULL;
    TMySensor *BottomSensor=NULL;
    TTrayMotor *TrayMotor=NULL;
    bool bHasRearSensor;
    bool bRearState;

    if(IsSoftSimulate())
        return;

    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
    {
        TrayMotor=GetAutoVMotor(Index);
        if(TrayMotor!=NULL && TrayMotor->fHasTray)
            State[Index].bCarHasTray=true;

        InputSensor=GetInputHasTray(Index);
        if(InputSensor!=NULL && InputSensor->Enable==true)
            State[Index].bFrontHasTray=InputSensor->IsOn();

        InputFullSensor=GetInputFullTray(Index);
        if(InputFullSensor!=NULL && InputFullSensor->Enable==true && InputFullSensor->IsOn())
            State[Index].bFrontHasTray=true;

        BottomSensor=GetOutputBottomHasTray(Index);
        bHasRearSensor=false;
        bRearState=false;

        if(BottomSensor!=NULL && BottomSensor->Enable==true)
        {
            bHasRearSensor=true;
            if(BottomSensor->IsOn())
                bRearState=true;
        }
        //AI(general) 20260608 : Stage0 - a TrayArm-delivered rear tray stays
        //latched until the Auto consumes it; the physical rear sensor reading
        //OFF must not erase the logical handshake (offline / sim-data run).
        if(bHasRearSensor)
        {
            if(bRearState)
                State[Index].bRearHasTray=true;
            else if(bRearDeliveredPending[Index]==false)
                State[Index].bRearHasTray=false;
        }
    }
}
//---------------------------------------------------------------------------
void TAutoModule::CheckAutoTray()
{
    TTrayMotor *TrayMotor=NULL;

    RefreshAutoState();
    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
    {
        TrayMotor=GetAutoVMotor(Index);
        if(TrayMotor==NULL)
            continue;
        if(TrayMotor->fHasTray)
            State[Index].bCarHasTray=true;
        if(State[Index].bCarHasTray && TrayMotor->FullThisIC(HAS_OK_IC))
            State[Index].bFullIC=true;
    }
}
//---------------------------------------------------------------------------
int TAutoModule::FindFeedAuto()
{
    RefreshAutoState();
    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
        State[Index].bRearCanUse=false;
    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
    {
        if(State[Index].bCarHasTray==false)
        {
            State[Index].bRearCanUse=true;
            //AI(ht160s-tray-source) 20260625 : CHANGE1 collision-window fix. Feed only
            //after the TrayArm-delivered latch is set (bRearDeliveredPending, set at
            //DoPlace case4000 AFTER the Z-lift is confirmed UP). The raw rear sensor goes
            //ON the instant the tray lands (DoPlace case2000) while the TrayArm head is
            //still down; without the latch the Auto Y could start and collide. Keep
            //RefreshAutoState's sensor->bRearHasTray write untouched (still needed for
            //rear-occupied / TrayArm placement-avoidance).
            if(State[Index].bRearHasTray && bRearDeliveredPending[Index])
                return Index;
        }
    }
    return -1;
}
//---------------------------------------------------------------------------
int TAutoModule::FindDischargeAuto()
{
    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
    {
        //AI(ht160s-agv) 20260623 : skip an AMR-locked Auto - no new discharge during the
        //handoff (FrontRise stays home; any in-flight discharge still finishes via DoAuto).
        if(bAmrLocked[Index])
            continue;
        if(State[Index].bFullIC && State[Index].bResidueClear)
            return Index;
    }
    return -1;
}
//---------------------------------------------------------------------------
int TAutoModule::FindEmptyRearForTrayArm()
{
    RefreshAutoState();
    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
        if(State[Index].bCarHasTray==false && State[Index].bRearHasTray==false)
            return Index;
    return -1;
}
//---------------------------------------------------------------------------
bool TAutoModule::IsRearHasTray(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return false;
    RefreshAutoState();
    return State[Index].bRearHasTray;
}
//---------------------------------------------------------------------------
void TAutoModule::SetRearHasTrayFromTrayArm(int Index, bool bHasTray)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return;
    State[Index].bRearHasTray=bHasTray;
    State[Index].bRearCanUse=bHasTray;
    bRearDeliveredPending[Index]=bHasTray;  //AI(general) 20260608 : Stage0 latch
}
//---------------------------------------------------------------------------
bool TAutoModule::DoFeedTray(int Flag)
{
    TTrayMotor *TrayMotor=NULL;
    TMyCylinder *PushCylinder=NULL;
    TMyCylinder *LeanCylinder=NULL;
    TMySensor *BottomSensor=NULL;
    AnsiString ErrorText;
    int Ret;

    if(Flag==0)
    {
        FeedTask=1;
        iFeedAuto=-1;
        FeedDelay.Clear();
        return true;
    }

    switch(FeedTask)
    {
        case 1:
            FeedTask=100;
            break;

        case 100:
            iFeedAuto=FindFeedAuto();
            if(iFeedAuto<0)
                return true;
            FeedTask=200;
            break;

        case 200:
            //AI(ht160s-tray-source) 20260625 : CHANGE2 pre-move rear-sensor cross-check.
            //We reached here only because the TrayArm-delivered latch is set (CHANGE1),
            //so a tray MUST be at the Auto rear. Confirm the physical rear sensor reads
            //ON BEFORE any Y motion. IsSensorOnReady returns true in sim or when the
            //sensor is disabled (Enable==false), so sim/offline behavior is preserved.
            //Only OutputBottomHasTray is wired on this machine (OutputHasTray removed as
            //unused), so check just that one - same sensor as the post-Y check at case3000.
            //If the latch is set but that rear sensor sees no tray, ask the operator:
            //  Retry = re-read the sensor next cycle (after removing a stranded tray or
            //          fixing the sensor),
            //  Skip  = discard the staged rear data and abort this feed (TrayArm will
            //          re-supply on demand). NO Y motion on Skip.
            if(IsSoftSimulate()
               || IsSensorOnReady(GetOutputBottomHasTray(iFeedAuto)))
            {
                FeedTask=1000;
            }
            else
            {
                ErrorText.sprintf("Auto%d: rear tray data transferred but no-tray sensor. Remove any stranded tray; if no tray, check the rear tray sensor. Retry=recheck sensor, Skip=clear tray data", iFeedAuto+1);
                Ret=ShowMyError(AnsiString().sprintf("JAM%d11", 11+iFeedAuto), ErrorText, K_SKIP|K_RETRY);
                if(Ret==K_SKIP)
                {
                    //AI(ht160s-tray-source) 20260625 : clear the staged rear data so the
                    //next cycle does not re-feed a phantom tray. Mirrors the field reset
                    //in DoFeedTray case7000 / DoDischargeTray case1000 / cleanout case7000.
                    State[iFeedAuto].bRearHasTray=false;
                    State[iFeedAuto].bRearCanUse=false;
                    bRearDeliveredPending[iFeedAuto]=false;
                    RearGrid[iFeedAuto].Clear();
                    RearKind[iFeedAuto]=eTrayKindNormal;
                    RearTrayID[iFeedAuto]="";
                    return true;
                }
                //K_RETRY (or any other) : stay in case 200 and re-read next cycle.
            }
            break;

        case 1000:
            if(MoveAutoY(iFeedAuto, GetAutoFeedY(iFeedAuto)))
                FeedTask=3000;
            break;

        case 3000:
            BottomSensor=GetOutputBottomHasTray(iFeedAuto);
            if(IsSoftSimulate() || IsSensorOnReady(BottomSensor))
                FeedTask=4000;
            else
            {
                ErrorText.sprintf("Auto%d Feed Tray Miss", iFeedAuto+1);
                Ret=ShowMyError(AnsiString().sprintf("JAM%d70", 11+iFeedAuto), ErrorText, K_RETRY);
                if(Ret==K_RETRY)
                    FeedTask=1000;
            }
            break;

        case 4000:
            LeanCylinder=GetLean(iFeedAuto);
            if(LeanCylinder!=NULL && (LeanCylinder->Push() || IsSoftSimulate()))
                FeedTask=5000;
            break;

        case 5000:
            PushCylinder=GetPush(iFeedAuto);
            if(PushCylinder!=NULL && (PushCylinder->Push() || IsSoftSimulate()))
            {
                FeedDelay.Set(5);
                FeedDelay.On();
                FeedTask=5100;
            }
            break;

        case 5100:
            if(FeedDelay.Off())
            {
                PushCylinder=GetPush(iFeedAuto);
                if(IsCylinderOnReady(PushCylinder, IsSoftSimulate()))
                    FeedTask=6000;
                else
                    FeedTask=5200;
            }
            break;

        case 5200:
            PushCylinder=GetPush(iFeedAuto);
            if(PushCylinder!=NULL && (PushCylinder->Pop() || IsSoftSimulate()))
            {
                ErrorText.sprintf("Auto%d Push Tray Miss", iFeedAuto+1);
                Ret=ShowMyError(AnsiString().sprintf("JAM%d01", 11+iFeedAuto), ErrorText, K_RETRY);
                if(Ret==K_RETRY)
                    FeedTask=5000;
            }
            break;

        case 6000:
            if(MoveAutoY(iFeedAuto, GetAutoFirstSortY(iFeedAuto)))
                FeedTask=7000;
            break;

        case 7000:
            TrayMotor=GetAutoVMotor(iFeedAuto);
            if(TrayMotor!=NULL)
            {
                //AI(ht160s-tray-source) : receive the grid TrayArm staged at rear (born at
                //Empty/Color) instead of fabricating a fresh EMPTY_IC tray here. DoFeedTray
                //only PROMOTES rear->working; occupancy (fHasTray) is owned here. RearGrid is
                //default EMPTY_IC/Normal if a feed ever runs without a staged delivery.
                TrayMotor->Tray.CopyFrom(RearGrid[iFeedAuto]);
                TrayMotor->fHasTray=true;
                TrayMotor->Refresh();   //AI(ht160s-tray-source) : InitNewTray used to Refresh; keep MotionView in sync
            }
            State[iFeedAuto].bCarHasTray=true;
            State[iFeedAuto].bRearHasTray=false;
            State[iFeedAuto].bRearCanUse=false;
            State[iFeedAuto].bFullIC=false;
            bRearDeliveredPending[iFeedAuto]=false;  //AI(general) 20260608 : Stage0 rear tray consumed
            //AI(HT160S-Maintainer) 20260605 : AMR stack-order bookkeeping. Record the tray
            //pulled up to the working position into the output car. If it is an identity or
            //cover tray (cannot hold IC) flag it full immediately so it discharges to the
            //stack without SortArm. SortArm is gated separately by IsReadyForSortArmPlace.
            if(GeneralSetting.bUseAMR)
            {
                WorkingKind[iFeedAuto]=RearKind[iFeedAuto];
                WorkingTrayID[iFeedAuto]=RearTrayID[iFeedAuto];   //AI(HT160S-Maintainer) 20260608 : carry 2D TrayID to working tray
                int n=Car[iFeedAuto].iTrayCount;
                if(n>=0 && n<MAX_TRAY_PER_CAR)
                {
                    Car[iFeedAuto].Tray[n].SetKind((eTrayKind)WorkingKind[iFeedAuto]);
                    Car[iFeedAuto].Tray[n].TrayID=WorkingTrayID[iFeedAuto];   //AI(HT160S-Maintainer) 20260608 : stamp 2D TrayID onto stack tray
                    //AI(HT160S-Maintainer) 20260608 : the identity tray (stack tray[0]) carries
                    //the car/lot 2D code; copy it to the car so the whole stack is identified.
                    if(WorkingKind[iFeedAuto]==eTrayKindIdentity)
                        Car[iFeedAuto].CarID=WorkingTrayID[iFeedAuto];
                    Car[iFeedAuto].iTrayCount=n+1;
                }
                if(WorkingKind[iFeedAuto]!=eTrayKindNormal)
                    State[iFeedAuto].bFullIC=true;
            }
            else
                WorkingKind[iFeedAuto]=eTrayKindNormal;
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TAutoModule::DoDischargeTray(int Flag)
{
    TTrayMotor *TrayMotor=NULL;
    TMyCylinder *PushCylinder=NULL;
    TMyCylinder *LeanCylinder=NULL;
    int AutoCeid[6]={136, 137, 138, 140, 141, 142};
    int &Task = DischargeTask;
    if(Flag==0)
    {
        Task=1;
        iDischargeAuto=-1;
        DischargeDelay.Clear();
        return true;
    }

    switch(Task)
    {
        case 1:
            Task=100;
            break;

        case 100:
            iDischargeAuto=FindDischargeAuto();
            if(iDischargeAuto<0)
                return true;
            Task=1000;
            break;

        case 1000:
            if(MoveAutoY(iDischargeAuto, GetAutoDischargeY(iDischargeAuto)))
            {
                TrayMotor=GetAutoVMotor(iDischargeAuto);
                if(TrayMotor!=NULL)
                {
                    TrayMotor->ClearTray();   //AI(ht160s-tray-source) : Auto never self-fabricates a tray; ClearTray resets data+fHasTray=false+bHasCover=false (rule #4)
                }
                State[iDischargeAuto].bFullIC=false;
                State[iDischargeAuto].bResidueClear=true;   //AI(ht160s-residue) 20260624 : fresh tray on discharge
                State[iDischargeAuto].bCarHasTray=false;
                State[iDischargeAuto].bRearHasTray=false;
                bRearDeliveredPending[iDischargeAuto]=false;  //AI(general) 20260608 : Stage0 latch clear
                RearGrid[iDischargeAuto].Clear();   //AI(ht160s-tray-source) : cleared rear => cleared staged grid
                State[iDischargeAuto].bFrontHasTray=true;
                if(HGem!=NULL)
                    HGem->EventReport(1, AutoCeid[iDischargeAuto]);
                Task=3000;
            }
            break;

        case 3000:
            PushCylinder=GetPush(iDischargeAuto);
            if(PushCylinder!=NULL && (PushCylinder->Pop() || IsSoftSimulate()))
                Task=4000;
            break;

        case 4000:
            LeanCylinder=GetLean(iDischargeAuto);
            if(LeanCylinder!=NULL && (LeanCylinder->Pop() || IsSoftSimulate()))
                Task=5000;
            break;

        case 5000:
            if(MoveAutoY(iDischargeAuto, GetAutoFeedY(iDischargeAuto)))
            {
                DischargeDelay.Set(5);
                DischargeDelay.On();
                Task=6000;
            }
            break;

        case 6000:
            //AI(general) 20260617 : post-Y settle done; the FrontRise On->settle->Off now
            //lives in the shared DoFrontRiseOnce so Teach Advanced TestGoUpOnce drives the
            //identical cylinder action as this production discharge.
            if(DischargeDelay.Off())
            {
                DischargeSubTask=1;
                Task=6100;
            }
            break;

        case 6100:
            if(DoFrontRiseOnce(iDischargeAuto, DischargeSubTask, DischargeDelay))
                return true;
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TAutoModule::DoAllAutoCleanOut(int Flag)
{
    TTrayMotor *TrayMotor=NULL;
    TMyCylinder *Cylinder=NULL;

    if(Flag==0)
    {
        CleanOutTask=1;
        CleanOutDelay.Clear();
        for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
            bCleanOutCheck[Index]=false;
        return true;
    }

    switch(CleanOutTask)
    {
        case 1:
            CleanOutTask=100;
            break;

        case 100:
            for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
                bCleanOutCheck[Index]=false;
            CleanOutTask=1000;
            break;

        case 1000:
            for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
                if(bCleanOutCheck[Index]==false)
                    bCleanOutCheck[Index]=MoveAutoY(Index, GetAutoDischargeY(Index));
            if(AreAllFlagsOn(bCleanOutCheck))
            {
                for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
                    bCleanOutCheck[Index]=false;
                CleanOutTask=2000;
            }
            break;

        case 2000:
            for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
            {
                if(bCleanOutCheck[Index]==false)
                {
                    Cylinder=GetPush(Index);
                    if(Cylinder!=NULL)
                    {
                        Cylinder->Pop();
                        if(IsCylinderOffReady(Cylinder, IsSoftSimulate()))
                            bCleanOutCheck[Index]=true;
                    }
                }
            }
            if(AreAllFlagsOn(bCleanOutCheck))
            {
                for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
                    bCleanOutCheck[Index]=false;
                CleanOutTask=3000;
            }
            break;

        case 3000:
            for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
            {
                if(bCleanOutCheck[Index]==false)
                {
                    Cylinder=GetLean(Index);
                    if(Cylinder!=NULL)
                    {
                        Cylinder->Pop();
                        if(IsCylinderOffReady(Cylinder, IsSoftSimulate()))
                            bCleanOutCheck[Index]=true;
                    }
                }
            }
            if(AreAllFlagsOn(bCleanOutCheck))
            {
                for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
                    bCleanOutCheck[Index]=false;
                CleanOutTask=4000;
            }
            break;

        case 4000:
            for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
            {
                if(bCleanOutCheck[Index]==false)
                {
                    Cylinder=GetFrontRise(Index);
                    if(Cylinder!=NULL)
                    {
                        Cylinder->On();
                        if(IsCylinderOnReady(Cylinder, IsSoftSimulate()))
                            bCleanOutCheck[Index]=true;
                    }
                }
            }
            if(AreAllFlagsOn(bCleanOutCheck))
            {
                CleanOutDelay.Set(5);
                CleanOutDelay.On();
                for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
                    bCleanOutCheck[Index]=false;
                CleanOutTask=5000;
            }
            break;

        case 5000:
            if(CleanOutDelay.Off())
            {
                for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
                {
                    Cylinder=GetFrontRise(Index);
                    if(Cylinder!=NULL)
                        Cylinder->Off();
                    bCleanOutCheck[Index]=false;
                }
                CleanOutTask=6000;
            }
            break;

        case 6000:
            for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
                if(bCleanOutCheck[Index]==false)
                    bCleanOutCheck[Index]=MoveAutoY(Index, GetAutoFeedY(Index));
            if(AreAllFlagsOn(bCleanOutCheck))
                CleanOutTask=7000;
            break;

        case 7000:
            for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
            {
                State[Index].bCarHasTray=false;
                State[Index].bRearHasTray=false;
                State[Index].bRearCanUse=false;
                bRearDeliveredPending[Index]=false;  //AI(general) 20260608 : Stage0 latch clear
                RearGrid[Index].Clear();   //AI(ht160s-tray-source) : cleared rear => cleared staged grid
                State[Index].bFrontHasTray=false;
                State[Index].bFullIC=false;
                State[Index].bCleanOutFinish=true;
                TrayMotor=GetAutoVMotor(Index);
                if(TrayMotor!=NULL)
                {
                    TrayMotor->ClearTray();   //AI(ht160s-tray-source) : Auto never self-fabricates a tray; ClearTray resets data+fHasTray=false+bHasCover=false (rule #4)
                }
            }
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TAutoModule::IsAllCleanOutFinish()
{
    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
        if(State[Index].bCleanOutFinish==false)
            return false;
    return true;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260604 : stacking-car accessors for Auto1~6.
TMyCar *TAutoModule::GetAutoCar(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return NULL;
    return &Car[Index];
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260624 : trays stacked on the Auto output car (PanelMain6 header).
//Grows on DoFeedTray; maintained in both sim and real (book-keeping, not sensor).
int TAutoModule::GetCarTrayCount(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return 0;
    return Car[Index].iTrayCount;
}
//---------------------------------------------------------------------------
//AI(ht160s-motion-view) 20260618 : 2D TrayID now at the working position, for the
//Unload-area Auto-info ID panel (palAutoXXID). Empty until an identity tray is seen.
AnsiString TAutoModule::GetWorkingTrayID(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return "";
    return WorkingTrayID[Index];
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260604 : tag stack roles : tray[0]=identity,
//   tray[1]=cover, tray[2..]=normal work trays.
void TAutoModule::InitAutoCarStack(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return;
    for(int i=0; i<MAX_TRAY_PER_CAR; i++)
    {
        if(i==0)
            Car[Index].Tray[i].SetKind(eTrayKindIdentity);
        else if(i==1)
            Car[Index].Tray[i].SetKind(eTrayKindCover);
        else
            Car[Index].Tray[i].SetKind(eTrayKindNormal);
    }
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260605 : AMR next-kind query. Output stack is built
//   identity(0) -> cover(1) -> normal(2..). Returns the eTrayKind the next delivery
//   to this Auto must be, or -1 when the car is already full.
int TAutoModule::GetNextTrayKindForAuto(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return -1;
    int n=Car[Index].iTrayCount;
    if(n>=MAX_TRAY_PER_CAR)
        return -1;
    if(n==0)
        return eTrayKindIdentity;
    if(n==1)
        return eTrayKindCover;
    return eTrayKindNormal;
}
//---------------------------------------------------------------------------
//AI(general) 20260608 : Stage1 demand API - "Auto pulls trays on demand". Returns
//   the eTrayKind (0/1/2) this Auto wants delivered to its rear right now, or
//   eTrayReqNone(-1) when it wants none. An Auto wants a rear tray only when its
//   working car is free and its rear is neither occupied nor already promised by a
//   TrayArm delivery (the Stage0 pending latch). In AMR mode the kind follows the
//   stack order (identity->cover->normal) and is -1 once the car is full.
int TAutoModule::GetTrayRequest(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return eTrayReqNone;
    //AI(ht160s-agv) 20260615 : while a full car is being handed to the AGV, refuse new
    //trays so TrayArm stops feeding this Auto until the handoff finishes (ClearAmrCar).
    if(bAmrLocked[Index])
        return eTrayReqNone;
    RefreshAutoState();
    if(State[Index].bCarHasTray)
        return eTrayReqNone;                 // car still busy with a working tray
    if(State[Index].bRearHasTray || bRearDeliveredPending[Index])
        return eTrayReqNone;                 // rear already loaded or on the way
    if(GeneralSetting.bUseAMR)
        return GetNextTrayKindForAuto(Index); // 0/1/2, or -1 when the car is full
    return eTrayKindNormal;                  // Normal mode : always an empty work tray
}
//---------------------------------------------------------------------------
//AI(general) 20260608 : Stage1 demand finder. Scans Auto1~6 and returns the first
//   station that currently wants a tray, reporting the requested eTrayKind through
//   OutKind. Returns -1 (and OutKind=eTrayReqNone) when no Auto wants a tray, which
//   lets the TrayArm stay idle instead of shuttling needlessly.
int TAutoModule::FindTrayRequestAuto(int &OutKind)
{
    OutKind=eTrayReqNone;
    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
    {
        int Req=GetTrayRequest(Index);
        if(Req!=eTrayReqNone)
        {
            OutKind=Req;
            return Index;
        }
    }
    return -1;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260605 : TrayArm placed a tray of the given kind at the
//   Auto rear. Mark the rear occupied and remember the kind so DoFeedTray can route
//   identity/cover trays straight to discharge once pulled to the working position.
void TAutoModule::NotifyTrayArmDelivered(int Index, int Kind, AnsiString TrayID)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return;
    RearKind[Index]=Kind;
    RearTrayID[Index]=TrayID;            //AI(HT160S-Maintainer) 20260608 : carry Color 2D TrayID
    State[Index].bRearHasTray=true;
    State[Index].bRearCanUse=true;
    bRearDeliveredPending[Index]=true;  //AI(general) 20260608 : Stage0 latch
}
//---------------------------------------------------------------------------
//AI(ht160s-tray-source) : TrayArm hands the carried grid to the Auto rear staging slot.
//Called for BOTH AMR and Normal placements (just before NotifyTrayArmDelivered /
//SetRearHasTrayFromTrayArm). DoFeedTray case 7000 copies RearGrid into the working tray.
//Occupancy (fHasTray) is NOT set here, so FindFeedAuto can still feed this station.
void TAutoModule::StageRearGrid(int Index, const TMyTray &Grid)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return;
    RearGrid[Index].CopyFrom(Grid);
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260605 : SortArm fill gate. In Normal mode every working
//   tray may receive IC. In AMR mode only a normal-kind working tray may : identity
//   and cover trays carry no IC and must reach the stack untouched.
bool TAutoModule::IsReadyForSortArmPlace(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return false;
    if(GeneralSetting.bUseAMR==false)
        return true;
    return (WorkingKind[Index]==eTrayKindNormal);
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260615 : output-car-full test used by the SECS AGV coordinator to
//  raise AGVSupplement (CEID272). Simulation has no sensor, so it uses a logical tray
//  threshold; the real machine reads the per-Auto InputFullTray sensor (same source
//  ServiceCarFull treats as the physical last line of defense).
bool TAutoModule::IsOutputCarFullForAmr(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return false;
    if(IsSoftSimulate())
        return (Car[Index].iTrayCount >= GeneralSetting.iSimAmrMaxTray[3+Index]);
    TMySensor *FullSensor=GetInputFullTray(Index);
    return (FullSensor!=NULL && FullSensor->Enable==true && FullSensor->IsOn());
}
//---------------------------------------------------------------------------
void TAutoModule::SetPlaceResidueClear(int Index, bool bClear)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return;
    State[Index].bResidueClear=bClear;   //AI(ht160s-residue) 20260624 : SortArm place-residue result for the target Auto (gate discharge / AMR leave)
}
//---------------------------------------------------------------------------
void TAutoModule::SetAmrLock(int Index, bool bLock)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return;
    bAmrLocked[Index]=bLock;
}
//---------------------------------------------------------------------------
bool TAutoModule::IsAmrLocked(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return false;
    return bAmrLocked[Index];
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260615 : Ready (CEID273) condition. The Auto has stacked every tray
//  into the car - no working tray, no rear tray (none in transit), nothing left to
//  discharge. With the AMR lock on, TrayArm cannot feed it, so this state is stable.
bool TAutoModule::IsDrainedForAmr(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return false;
    //AI(ht160s-agv) 20260625 : SIM-ONLY bypass so the PREP->READY (CEID273) gate
    //cannot latch the AMR lock forever on a laptop run. MUST stay strictly inside
    //IsSoftSimulate(): on real hardware this gate also enforces bResidueClear (the
    //SortArm place-residue interlock) - an unconditional bypass would let the AGV
    //leave with un-verified IC residue. The real-hardware checks below stay active.
    if(IsSoftSimulate())
        return true;
    RefreshAutoState();
    //AI(ht160s-agv) 20260623 : Ready also needs the stacking FrontRise back home (not
    //commanded up) so the handoff starts with the front cylinder idle (user requirement).
    TMyCylinder *Rise=GetFrontRise(Index);
    bool bFrontHome=(Rise==NULL || Rise->GetOutBit()==false);
    return (!State[Index].bCarHasTray
            && !State[Index].bRearHasTray
            && !bRearDeliveredPending[Index]
            && !State[Index].bFullIC
            && State[Index].bResidueClear
            && bFrontHome);
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260615 : Finish (CEID274) condition. The AGV has removed the full car.
//  Simulation reports taken so the handshake is testable end-to-end; the real machine
//  needs a per-Auto "car taken" IO point (TBD) wired here - until then it returns false
//  and the handshake holds at Ready (production stays parked on that Auto).
bool TAutoModule::IsAmrTaken(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return false;
    if(IsSoftSimulate())
        return true;
    //AI(ht160s-agv) 20260623 : car-taken = SnAutoX_InputEnd reads no tray (ON=has tray).
    //Enable==false (sensor unwired) keeps this false -> holds at Ready (pre-sensor behavior).
    TMySensor *EndSensor=GetInputEndSensor(Index);
    return (EndSensor!=NULL && EndSensor->Enable==true && EndSensor->IsOff());
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260615 : AGV finish - empty the output car, re-seed the stack roles,
//  and release the lock so production resumes (mirrors ServiceCarFull's clear, without
//  the operator modal).
void TAutoModule::ClearAmrCar(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return;
    Car[Index].Clear();
    InitAutoCarStack(Index);
    bAmrLocked[Index]=false;
    State[Index].bResidueClear=true;   //AI(ht160s-residue) 20260624 : fresh car on AGV finish
}
//---------------------------------------------------------------------------
//AI(ht160s-state-record-analysis) 20260616 : read-only state + working-tray cell
//map for the Store Hangup SortArmDecision.txt. Lets an offline snapshot show WHY
//an Auto neither accepts a SortArm place (no contiguous EMPTY_IC run) nor discharges
//(discharge needs FullThisIC(HAS_OK_IC) = EVERY cell a good IC). Iterates only the
//recipe tray region clamped to the Data[] array bounds (mirrors MyMotor's
//GetTrayRealXCount/YCount), so an out-of-range recipe cannot overflow.
int TAutoModule::GetStationCount()
{
    return AUTO_STATION_COUNT;
}
//---------------------------------------------------------------------------
AnsiString TAutoModule::DescribeStation(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return "";

    RefreshAutoState();

    AnsiString s;
    s  = "[Auto" + IntToStr(Index+1) + "]\r\n";
    s += "  CarHasTray="  + IntToStr(State[Index].bCarHasTray ? 1 : 0)
       + "  RearHasTray=" + IntToStr(State[Index].bRearHasTray ? 1 : 0)
       + "  FullIC="      + IntToStr(State[Index].bFullIC ? 1 : 0)
       + "  RearPending=" + IntToStr(bRearDeliveredPending[Index] ? 1 : 0)
       + "  AmrLocked="   + IntToStr(bAmrLocked[Index] ? 1 : 0)
       + "  WorkingKind=" + IntToStr(WorkingKind[Index]) + "\r\n";

    TTrayMotor *V=GetAutoVMotor(Index);
    if(V==NULL)
    {
        s += "  (no working V motor)\r\n";
        return s;
    }
    if(V->fHasTray==false)
    {
        s += "  (working car empty - no tray)\r\n";
        return s;
    }

    int xEnd=TrayForm.XDivision;
    if(xEnd<1) xEnd=1;
    if(xEnd>MAX_TRAY_X) xEnd=MAX_TRAY_X;
    int yEnd=TrayForm.YDivision;
    if(yEnd<1) yEnd=1;
    if(yEnd>MAX_TRAY_Y) yEnd=MAX_TRAY_Y;

    int nEmpty=0, nOk=0, nUnchk=0, nOther=0;
    s += "  Cells (row=Y, col=X ; .=empty O=OK ?=uncheck #=other):\r\n";
    for(int y=0; y<yEnd; y++)
    {
        AnsiString Row="    ";
        for(int x=0; x<xEnd; x++)
        {
            int d=V->Tray.Data[x][y];
            if(d==EMPTY_IC)        { Row+="."; nEmpty++; }
            else if(d==HAS_OK_IC)  { Row+="O"; nOk++;    }
            else if(d==UNCHECK_IC) { Row+="?"; nUnchk++; }
            else                   { Row+="#"; nOther++; }
        }
        s += Row + "\r\n";
    }
    s += "  Counts: empty=" + IntToStr(nEmpty)
       + " ok="      + IntToStr(nOk)
       + " uncheck=" + IntToStr(nUnchk)
       + " other="   + IntToStr(nOther) + "\r\n";
    return s;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260612 : AMR output-car full handling. The car book-keeping
//(Car[].iTrayCount) only grows on DoFeedTray and is never reset by discharge/cleanout,
//so once it reaches MAX_TRAY_PER_CAR GetNextTrayKindForAuto()/GetTrayRequest() return -1
//forever and TrayArm stops feeding that Auto. This services a full car each idle scan :
//  - Simulation (IsSoftSimulate): no physical car/AMR, so logically recycle the full
//    car (stands in for the AMR car-swap) and keep running.
//  - Real machine : pop an alarm so the operator changes the car, and only clear the
//    data after the operator confirms.
//  - Last line of defense : the physical InputFullTray sensor. While it reads ON keep
//    alarming until the operator physically clears the stack (sensor OFF).
//Scoped to AMR + Run_Normal so Normal/CleanOut behavior is unchanged.
void TAutoModule::ServiceCarFull()
{
    if(GeneralSetting.bUseAMR==false)
        return;
    if(HSys.Sys.RunMode!=Run_Normal)
        return;

    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
    {
        //AI(ht160s-agv) 20260615 : when the SECS link is up, hand a full car to the AGV
        //(the CEID272/273/274 handshake clears it via ClearAmrCar) instead of popping the
        //operator full-car modal. While the host is disconnected this falls through to the
        //original modal + manual car change, so offline behavior is unchanged.
        if(HGem!=NULL && HGem->IsSelected())
            continue;

        bool bLogicalFull=(Car[Index].iTrayCount>=MAX_TRAY_PER_CAR);

        if(IsSoftSimulate())
        {
            if(bLogicalFull)
            {
                Car[Index].Clear();
                InitAutoCarStack(Index);
            }
            continue;
        }

        TMySensor *FullSensor=GetInputFullTray(Index);
        bool bSensorFull=(FullSensor!=NULL && FullSensor->Enable==true && FullSensor->IsOn());

        if(bSensorFull)
        {
            AnsiString ErrorText;
            ErrorText.sprintf("Auto%d output stack FULL (sensor) - remove finished trays", Index+1);
            do
            {
                ShowMyError(AnsiString().sprintf("MES%d20", 11+Index), ErrorText, K_RETRY);
                FullSensor=GetInputFullTray(Index);
            }
            while(FullSensor!=NULL && FullSensor->Enable==true && FullSensor->IsOn());
            Car[Index].Clear();
            InitAutoCarStack(Index);
        }
        else if(bLogicalFull)
        {
            AnsiString ErrorText;
            ErrorText.sprintf("Auto%d output car full (%d trays) - change car then confirm", Index+1, MAX_TRAY_PER_CAR);
            ShowMyError(AnsiString().sprintf("MES%d25", 11+Index), ErrorText, K_RETRY);
            Car[Index].Clear();
            InitAutoCarStack(Index);
        }
    }
}
//---------------------------------------------------------------------------
void TAutoModule::DoAuto(int &Task)
{
    if(HSys.Sys.RunMode==Run_CleanOut && IsAllCleanOutFinish())
        return;

    switch(Task)
    {
        case 1:
            Task=100;
            break;

        case 100:
            if(HSys.Sys.RunMode==Run_CleanOut &&
               SortArmModule->IsCleanOutFinish())
            {
                DoAllAutoCleanOut(0);
                Task=5000;
                break;
            }
            CheckAutoTray();
            ServiceCarFull();   //AI(HT160S-Maintainer) 20260612 : AMR car-full sim-clear / alarm / Full-sensor last line
            Task=1000;
            break;

        case 1000:
            if(FindFeedAuto()>=0)
            {
                DoFeedTray(0);
                Task=2000;
            }
            else
                Task=3000;
            break;

        case 2000:
            if(DoFeedTray(1))
                Task=3000;
            break;

        case 3000:
            CheckAutoTray();
            if(FindDischargeAuto()>=0)
            {
                DoDischargeTray(0);
                Task=4000;
            }
            else
                Task=1;
            break;

        case 4000:
            if(DoDischargeTray(1))
                Task=1;
            break;

        case 5000:
            if(DoAllAutoCleanOut(1))
                Task=1;
            break;
    }
}
//---------------------------------------------------------------------------
//AI(general) 20260617 : Teach Advanced single-cylinder GoUp-once test. Auto has only
//one FrontRiseTray cylinder per station (no _2 / no production Separate use), so the
//test just raises it, settles, then lowers : one GoUp, no GoDown. Mirrors the
//FrontRise On->settle->Off used in DoDischargeTray case 6000-7000.
//AI(general) 20260617 : shared single-cylinder FrontRise actuation (On->settle->Off).
//Extracted from DoDischargeTray case 6000-7000 so the Teach Advanced TestGoUpOnce drives
//the IDENTICAL cylinder action as the production discharge (no drift). Caller owns the
//SubTask + settle Delay. Returns true when the rise+lower has completed.
bool TAutoModule::DoFrontRiseOnce(int Index, int &SubTask, HTimer &Delay)
{
    TMyCylinder *Rise=GetFrontRise(Index);

    if(Rise==NULL)
    {
        SubTask=1;
        return true;
    }

    switch(SubTask)
    {
        case 1:
            Rise->On();
            if(IsCylinderOnReady(Rise, IsSoftSimulate()))
            {
                Delay.Set(5);
                Delay.On();
                SubTask=2;
            }
            break;

        case 2:
            if(Delay.Off())
            {
                Rise->Off();
                SubTask=1;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(general) 20260617 : Teach Advanced GoUp-once = the production FrontRise sub-action
//(shared DoFrontRiseOnce), so the standalone test == the Auto-run discharge rise.
bool TAutoModule::TestGoUpOnce(int Index, int Flag)
{
    if(Flag==0)
    {
        TestUpTask=1;
        TestDelay.Clear();
        return true;
    }
    return DoFrontRiseOnce(Index, TestUpTask, TestDelay);
}
//---------------------------------------------------------------------------
void InitializeAutoModule()
{
    if(AutoModule==NULL)
        AutoModule=new TAutoModule;
}
//---------------------------------------------------------------------------
void ShutdownAutoModule()
{
    delete AutoModule;
    AutoModule=NULL;
}
//---------------------------------------------------------------------------