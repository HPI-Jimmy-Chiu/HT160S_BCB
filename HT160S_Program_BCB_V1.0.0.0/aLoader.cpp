#include "IncludeAllHeader.h"       //AI(HT160S-Maintainer) 20260609 : merged header to speed build
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#include <IniFiles.hpp>
#pragma hdrstop

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
    ResetSide(&Side[0]);
    ResetSide(&Side[1]);
    bRearHasTray=false;
    iFrontOwner=0;
    iTopCcdCount=0;
    iYOwner[0]=LOADER_Y_OWNER_NONE;
    iYOwner[1]=LOADER_Y_OWNER_NONE;
    SimuCcdCycleIndex=0;
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
    return TrayForm.XPitch;
}
//---------------------------------------------------------------------------
double TLoaderModule::GetTrayYPitch()
{
    return TrayForm.YPitch;
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
        ShowMyMessage("Loader Y motor will out of limit");
        return false;
    }
    return Motor->MotorMove(Position);
}
//---------------------------------------------------------------------------
bool TLoaderModule::IsLoaderYMoveSafe(int LoaderNo, int Position)
{
    //AI(HT160S-Maintainer) 20260610 : framework for option C (opposite-side tray
    //clamped + minimum distance). The two Loader-Y cars share the same physical
    //rail, so their encoder positions are directly comparable (same units, as the
    //legacy 160 discharge interlock did). Rules :
    //  - Only the OTHER car holding a tray (fHasTray) is a collision risk; an empty
    //    car parked clear is ignored.
    //  - If the target position of THIS car would sit closer than the configured
    //    safe distance to the OTHER car's current encoder position, block the move.
    //  - Safe distance is read from GeneralSetting.iLoaderYSafeDistance
    //    ([Safety] LoaderYSafeDistance in General.ini, default 10000).
    //  - When data cannot be evaluated (NULL motors) the move is allowed so this
    //    guard never silently freezes the machine on a missing object.
    int OtherNo;
    TTrayMotor *OtherMotor=NULL;
    TTrayMotor *OtherTray=NULL;
    int OtherPos;
    int Gap;

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

    if(GeneralSetting.iLoaderYSafeDistance<=0)
        return true;

    OtherPos=OtherMotor->ReadEncoderPos();
    Gap=Position-OtherPos;
    if(Gap<0)
        Gap=-Gap;
    if(Gap<GeneralSetting.iLoaderYSafeDistance)
        return false;
    return true;
}
//---------------------------------------------------------------------------
bool TLoaderModule::MoveTopCcdX(int Position)
{
    if(HSys.Mot.MTopCCDX==NULL)
        return false;
    if(HSys.Mot.MTopCCDX->CheckSoftLimit(Position)==false)
    {
        ShowMyMessage("Top CCD X motor will out of limit");
        return false;
    }
    return HSys.Mot.MTopCCDX->MotorMove(Position);
}
//---------------------------------------------------------------------------
bool TLoaderModule::MoveToCcdCell(int LoaderNo, int CellX, int CellY)
{
    int XPos=RoundPosition((double)GetTopCcdFirstX()+((double)CellX)*GetTrayXPitch());
    int YPos=RoundPosition((double)GetLoaderFirstCcdY(LoaderNo)+((double)CellY)*GetTrayYPitch());
    bool bXFlag=MoveTopCcdX(XPos);
    bool bYFlag=MoveLoaderY(LoaderNo, YPos);
    return (bXFlag && bYFlag);
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

    if(HSys.Sen.SnLoader_OutputHasTray.Enable==true)
    {
        bHasRearSensor=true;
        if(HSys.Sen.SnLoader_OutputHasTray.IsOn())
            bSensorState=true;
    }
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
    bRearHasTray=false;
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
    if(tSimuData.bRunSimulation || HSys.LastSet.iRealDummy!=REALLY)
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
                if(OtherState->Status==LS_FEEDING)
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
                State->FeedTask=10000;
            }
            else
            {
                Ret=ShowMyError("Loader Tray Empty", K_RETRY|K_TRAY_END|K_CLEAN_OUT);
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
                   OtherState->Status==LS_SORTING ||
                   OtherState->Status==LS_ToRear)
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
                    if(HSys.LastSet.iRealDummy==REALLY && TopCcdSocket!=NULL && TopCcdSocket->IsTopCcdConnected()==false)
                    {
                        Ret=ShowMyError("Top CCD Connect not ready", K_RETRY|K_SKIP);
                        if(Ret==K_SKIP)
                        {
                            TrayMotor->SetTrayBin(State->CcdX, State->CcdY, HT160_BIN_ERROR_2D_SCAN_FAIL);
                            State->CcdTask=1;
                        }
                        break;
                    }
                    //Guard NULL socket so the simulation path (no Top CCD hardware) can
                    //still advance to the 2D-code poll state. Real hardware triggers a shot.
                    if(HSys.LastSet.iRealDummy==REALLY && TopCcdSocket!=NULL)
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
                Ret=ShowMyError("Top CCD API not ready", K_SKIP|K_RETRY|K_TRAY_END);
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
                        Ret=ShowMyError("2D code not found in any lot : "+sCode, K_RETRY|K_SKIP);
                        if(Ret==K_RETRY)
                        {
                            if(TopCcdSocket!=NULL)
                                TopCcdSocket->TopCcdTriggerShot();
                            State->CcdDelay.SetMS(3000);
                            State->CcdDelay.On();
                        }
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
                    Ret=ShowMyError("Top CCD 2D no response", K_RETRY|K_SKIP);
                    if(Ret==K_RETRY)
                    {
                        if(TopCcdSocket!=NULL)
                            TopCcdSocket->TopCcdTriggerShot();
                        State->CcdDelay.SetMS(3000);
                        State->CcdDelay.On();
                    }
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
