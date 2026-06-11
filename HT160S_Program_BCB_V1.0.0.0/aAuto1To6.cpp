#include "IncludeAllHeader.h"       //Dell 將.h統一,可加速build
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "aAuto1To6.h"
#include "database.h"
#include "cmydef.h"
#include "mymessbox.h"
#include "uteach.h"
#include "SecsGem\uHGemEquipment.h"
#include "GeneralSetting.h"   //AI(HT160S-Maintainer) 20260605 : GeneralSetting.bUseAMR mode switch
#include "aSortArm.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TAutoModule *AutoModule=NULL;
//---------------------------------------------------------------------------
static const int AUTO_STATION_COUNT=6;
//---------------------------------------------------------------------------
static bool IsSensorOnReady(TMySensor *Sensor)
{
    if(Sensor==NULL || Sensor->Enable==false)
        return true;
    return Sensor->IsOn();
}
//---------------------------------------------------------------------------
static bool IsCylinderOnReady(TMyCylinder *Cylinder, bool bSoftSimulate)
{
    if(Cylinder==NULL)
        return false;
    if(bSoftSimulate)
        return true;
    if(Cylinder->OnSensor.Enable==false)
        return true;
    return Cylinder->OnSensor.IsOn();
}
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
void TAutoModule::InitialFlag()
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

    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
    {
        TrayMotor=GetAutoVMotor(Index);
        State[Index].bCarHasTray=(TrayMotor!=NULL && TrayMotor->fHasTray);
        State[Index].bRearHasTray=false;
        State[Index].bRearCanUse=false;
        State[Index].bFrontHasTray=false;
        State[Index].bFullIC=false;
        State[Index].bCleanOutFinish=false;
        bCleanOutCheck[Index]=false;
        bRearDeliveredPending[Index]=false;  //AI(general) 20260608 : Stage0 latch reset
        RearKind[Index]=eTrayKindNormal;     //AI(HT160S-Maintainer) 20260605 : AMR reset
        WorkingKind[Index]=eTrayKindNormal;  //AI(HT160S-Maintainer) 20260605 : AMR reset
        RearTrayID[Index]="";                //AI(HT160S-Maintainer) 20260608 : AMR 2D TrayID reset
        WorkingTrayID[Index]="";             //AI(HT160S-Maintainer) 20260608 : AMR 2D TrayID reset
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
TMySensor *TAutoModule::GetOutputHasTray(int Index)
{
    switch(Index)
    {
        case 0: return &HSys.Sen.SnAuto1_OutputHasTray;
        case 1: return &HSys.Sen.SnAuto2_OutputHasTray;
        case 2: return &HSys.Sen.SnAuto3_OutputHasTray;
        case 3: return &HSys.Sen.SnAuto4_OutputHasTray;
        case 4: return &HSys.Sen.SnAuto5_OutputHasTray;
        case 5: return &HSys.Sen.SnAuto6_OutputHasTray;
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
        ShowMyMessage("Auto Y motor will out of limit");
        return false;
    }
    return Motor->MotorMove(Position);
}
//---------------------------------------------------------------------------
void TAutoModule::RefreshAutoState()
{
    TMySensor *InputSensor=NULL;
    TMySensor *InputFullSensor=NULL;
    TMySensor *OutputSensor=NULL;
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

        OutputSensor=GetOutputHasTray(Index);
        BottomSensor=GetOutputBottomHasTray(Index);
        bHasRearSensor=false;
        bRearState=false;

        if(OutputSensor!=NULL && OutputSensor->Enable==true)
        {
            bHasRearSensor=true;
            if(OutputSensor->IsOn())
                bRearState=true;
        }
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
            if(State[Index].bRearHasTray)
                return Index;
        }
    }
    return -1;
}
//---------------------------------------------------------------------------
int TAutoModule::FindDischargeAuto()
{
    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
        if(State[Index].bFullIC)
            return Index;
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
            FeedTask=1000;
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
                Ret=ShowMyError(ErrorText, K_RETRY);
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
                Ret=ShowMyError(ErrorText, K_RETRY);
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
                TrayMotor->fHasTray=true;
                TrayMotor->InitNewTray(EMPTY_IC);
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
    TMyCylinder *FrontRiseCylinder=NULL;
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
                    TrayMotor->InitNewTray(EMPTY_IC);
                    TrayMotor->fHasTray=false;
                }
                State[iDischargeAuto].bFullIC=false;
                State[iDischargeAuto].bCarHasTray=false;
                State[iDischargeAuto].bRearHasTray=false;
                bRearDeliveredPending[iDischargeAuto]=false;  //AI(general) 20260608 : Stage0 latch clear
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
            if(DischargeDelay.Off())
            {
                FrontRiseCylinder=GetFrontRise(iDischargeAuto);
                if(FrontRiseCylinder!=NULL)
                    FrontRiseCylinder->On();
                if(IsCylinderOnReady(FrontRiseCylinder, IsSoftSimulate()))
                {
                    DischargeDelay.Set(5);
                    DischargeDelay.On();
                    Task=7000;
                }
            }
            break;

        case 7000:
            if(DischargeDelay.Off())
            {
                FrontRiseCylinder=GetFrontRise(iDischargeAuto);
                if(FrontRiseCylinder!=NULL)
                    FrontRiseCylinder->Off();
                return true;
            }
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
                State[Index].bFrontHasTray=false;
                State[Index].bFullIC=false;
                State[Index].bCleanOutFinish=true;
                TrayMotor=GetAutoVMotor(Index);
                if(TrayMotor!=NULL)
                {
                    TrayMotor->InitNewTray(EMPTY_IC);
                    TrayMotor->fHasTray=false;
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