#include "IncludeAllHeader.h"       //Dell 將.h統一,可加速build
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#include <IniFiles.hpp>
#pragma hdrstop

#include "aSortArm.h"
#include "aLoader.h"
#include "database.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "mymessbox.h"
#include "setup.h"
#include "uteach.h"
#include "deviceinfo.h"
#include "aAuto1To6.h"     //AI(HT160S-Maintainer) 20260605 : AMR SortArm fill gate
#include "GeneralSetting.h"  //AI(HT160S-Maintainer) 20260605 : GeneralSetting.bUseAMR
#include "MyLed.h"           //AI(ht172-to-ht160-porting) 20260609 : drive sucker LED on Motion View
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TSortArmModule *SortArmModule=NULL;
//---------------------------------------------------------------------------
static const int SORT_ARM_SUCKER_COUNT=4;
static const int SORT_ARM_AUTO_COUNT=6;
static const int SORT_ARM_SAFE_Z_POSITION=10;
static const int SUCK_HOME_LOST_MS=100;   //AI(HT160S-Maintainer) 20260622 : SortArmX suck-home loss debounce window (ms); time-based, not cycle-count
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
TSortArmModule::TSortArmModule()
{
    InitialFlag();
}
//---------------------------------------------------------------------------
void TSortArmModule::InitialFlag(bool bKeepMaterial)
{
    PickTask=1;
    PlaceTask=1;
    iActiveLoaderNo=0;
    iActiveAutoIndex=-1;
    iPlaceBaseX=0;
    iPlaceY=0;
    bCleanOutFinish=false;
    bOneCycleFinish=false;
    dwSuckHomeLostStart=0;
    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        //AI(HT160S-Maintainer) 20260612 : recoverable home with a sucked IC still on this
        //slot : keep the IC (TrayData/BinValue) and RE-ASSERT the vacuum so it cannot fall
        //during the re-home, dropping only the transient pick/place selection. The held IC
        //is placed to its Auto on resume (DoSortArm case 1 -> HasHoldingIC -> place).
        if(bKeepMaterial && Slot[SlotIndex].bHasIC)
        {
            TMySucker *Sucker=GetSucker(SlotIndex);
            Slot[SlotIndex].bCanPick=false;
            Slot[SlotIndex].bPlaceSelected=false;
            Slot[SlotIndex].PickX=0;
            Slot[SlotIndex].PickY=0;
            Slot[SlotIndex].PlaceX=0;
            Slot[SlotIndex].PlaceY=0;
            if(Sucker!=NULL && IsSoftSimulate()==false)
            {
                Sucker->Reset();   //clear sub-task only : does NOT touch the vacuum output
                Sucker->On();      //re-assert suck so the held IC cannot drop during home
            }
        }
        else
        {
            ClearSlot(SlotIndex);
        }
    }
    UpdateKitSuckState();
}
//---------------------------------------------------------------------------
void TSortArmModule::ClearSlot(int SlotIndex)
{
    TMySucker *Sucker=GetSucker(SlotIndex);

    if(SlotIndex<0 || SlotIndex>=SORT_ARM_SUCKER_COUNT)
        return;
    Slot[SlotIndex].bCanPick=false;
    Slot[SlotIndex].bHasIC=false;
    Slot[SlotIndex].bPlaceSelected=false;
    Slot[SlotIndex].PickX=0;
    Slot[SlotIndex].PickY=0;
    Slot[SlotIndex].PlaceX=0;
    Slot[SlotIndex].PlaceY=0;
    Slot[SlotIndex].TrayData=EMPTY_IC;
    Slot[SlotIndex].BinValue=0;
    Slot[SlotIndex].LotIndex=-1;       //AI(ht160s-lotbin) 20260615 : clear carried lot
    Slot[SlotIndex].Code2D="";         //AI(ht160s-lotbin) 20260615 : clear carried 2D code
    if(Sucker!=NULL)
    {
        Sucker->Item=EMPTY_IC;
        Sucker->Reset();
    }
}
//---------------------------------------------------------------------------
void TSortArmModule::ClearPickSelection()
{
    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
        Slot[SlotIndex].bCanPick=false;
}
//---------------------------------------------------------------------------
void TSortArmModule::ClearPlaceSelection()
{
    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        Slot[SlotIndex].bPlaceSelected=false;
        Slot[SlotIndex].PlaceX=0;
        Slot[SlotIndex].PlaceY=0;
    }
}
//---------------------------------------------------------------------------
void TSortArmModule::UpdateKitSuckState()
{
    bool bHasIC=HasHoldingIC();

    HSys.Suck.SortArmSuck.Has_SuckIC=bHasIC;
    HSys.Suck.SortArmSuck.SuckIC=bHasIC;
    HSys.Suck.SortArmSuck.TrayData=EMPTY_IC;

    //AI(ht172-to-ht160-porting) 20260609 : light each Motion View sucker LED
    //when that slot holds an IC (HT172 ledSortArm1ZA..ZF behavior). The LEDs
    //are wired to the slots via SortArmSuck.SetMyLed() in main.cpp
    //InitSimulateScreenBinding(). Display only - never gates motion/IO/vacuum.
    for(int LedSlot=0; LedSlot<SORT_ARM_SUCKER_COUNT; LedSlot++)
    {
        TMyLed *pSuckLed=HSys.Suck.SortArmSuck.Suck[0][LedSlot].pLed;
        if(pSuckLed!=NULL)
        {
            pSuckLed->TrueColor=clLime;
            pSuckLed->FalseColor=clSilver;
            pSuckLed->Value=Slot[LedSlot].bHasIC;
        }
    }

    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        if(Slot[SlotIndex].bHasIC)
        {
            HSys.Suck.SortArmSuck.TrayData=Slot[SlotIndex].TrayData;
            return;
        }
    }
}
//---------------------------------------------------------------------------
bool TSortArmModule::HasHoldingIC()
{
    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
        if(Slot[SlotIndex].bHasIC)
            return true;
    return false;
}
//---------------------------------------------------------------------------
bool TSortArmModule::IsSoftSimulate()
{
    #ifdef SOFT_SIMULATE
    return true;
    #else
    return (HSys.LastSet.iRealDummy==DUMMY);
    #endif
}
//---------------------------------------------------------------------------
bool TSortArmModule::IsPickableData(int Data)
{
    return (Data>UNCHECK_IC);
}
//---------------------------------------------------------------------------
TTrayMotor *TSortArmModule::GetLoaderMotor(int LoaderNo)
{
    if(LoaderNo==1)
        return HSys.Mot.MLoaderY_1;
    if(LoaderNo==2)
        return HSys.Mot.MLoaderY_2;
    return NULL;
}
//---------------------------------------------------------------------------
TTrayMotor *TSortArmModule::GetLoaderVMotor(int LoaderNo)
{
    if(LoaderNo==1)
        return HSys.VMot.MMLoaderY_1;
    if(LoaderNo==2)
        return HSys.VMot.MMLoaderY_2;
    return NULL;
}
//---------------------------------------------------------------------------
TTrayMotor *TSortArmModule::GetAutoMotor(int AutoIndex)
{
    switch(AutoIndex)
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
TTrayMotor *TSortArmModule::GetAutoVMotor(int AutoIndex)
{
    switch(AutoIndex)
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
TTrayMotor *TSortArmModule::GetSuckZMotor(int SlotIndex)
{
    switch(SlotIndex)
    {
        case 0: return HSys.Mot.MSuckZ_1;
        case 1: return HSys.Mot.MSuckZ_2;
        case 2: return HSys.Mot.MSuckZ_3;
        case 3: return HSys.Mot.MSuckZ_4;
    }
    return NULL;
}
//---------------------------------------------------------------------------
TMySucker *TSortArmModule::GetSucker(int SlotIndex)
{
    if(SlotIndex<0 || SlotIndex>=SORT_ARM_SUCKER_COUNT)
        return NULL;
    return &HSys.Suck.SortArmSuck.Suck[0][SlotIndex];
}
//---------------------------------------------------------------------------
int TSortArmModule::GetTrayXCount()
{
    //AI(HT160S-Maintainer) 20260608 : read recipe Tray geometry from the
    //in-memory TrayForm structure (single source), never the Setup form UI.
    return ClampIntValue(TrayForm.XDivision, 1, 50);
}
//---------------------------------------------------------------------------
int TSortArmModule::GetTrayYCount()
{
    return ClampIntValue(TrayForm.YDivision, 1, 20);
}
//---------------------------------------------------------------------------
double TSortArmModule::GetTrayXPitch()
{
    return TrayForm.XPitch;
}
//---------------------------------------------------------------------------
double TSortArmModule::GetTrayYPitch()
{
    return TrayForm.YPitch;
}
//---------------------------------------------------------------------------
int TSortArmModule::RoundPosition(double Value)
{
    if(Value>=0.0)
        return (int)(Value+0.5);
    return (int)(Value-0.5);
}
//---------------------------------------------------------------------------
int TSortArmModule::CalculatePitchPosition()
{
    double MaxPosition=(double)Teach.PitchArmXMaxPositoin;
    double MinPosition=(double)Teach.PitchArmXMinPositoin;
    double Pitch=GetTrayXPitch();
    double Scale;

    if(Pitch<1200.0)
        return 0;
    if(Teach.PitchArmXMinPositoin<=0 || Teach.PitchArmXMaxPositoin<=0)
        return 0;
    if(Teach.PitchArmXMinPositoin>Teach.PitchArmXMaxPositoin)
        return 0;

    Scale=(MaxPosition-MinPosition)/(4000.0-1200.0);
    return RoundPosition(MinPosition+Scale*(Pitch-1200.0));
}
//---------------------------------------------------------------------------
int TSortArmModule::GetLoaderSortX(int LoaderNo)
{
    if(LoaderNo==2)
        return Teach.SortArmToLoader2XPosition;
    return Teach.SortArmToLoader1XPosition;
}
//---------------------------------------------------------------------------
int TSortArmModule::GetLoaderFirstSortY(int LoaderNo)
{
    if(LoaderNo==2)
        return Teach.Loader2CarFirstSortYPosition;
    return Teach.Loader1CarFirstSortYPosition;
}
//---------------------------------------------------------------------------
int TSortArmModule::GetLoaderZPosition(int LoaderNo, int SlotIndex)
{
    if(LoaderNo==2)
    {
        switch(SlotIndex)
        {
            case 0: return Teach.SortArmToLoader_2_Z1Position;
            case 1: return Teach.SortArmToLoader_2_Z2Position;
            case 2: return Teach.SortArmToLoader_2_Z3Position;
            case 3: return Teach.SortArmToLoader_2_Z4Position;
        }
    }
    switch(SlotIndex)
    {
        case 0: return Teach.SortArmToLoader_1_Z1Position;
        case 1: return Teach.SortArmToLoader_1_Z2Position;
        case 2: return Teach.SortArmToLoader_1_Z3Position;
        case 3: return Teach.SortArmToLoader_1_Z4Position;
    }
    return 0;
}
//---------------------------------------------------------------------------
int TSortArmModule::GetAutoSortX(int AutoIndex)
{
    switch(AutoIndex)
    {
        case 0: return Teach.SortArmToAuto1XPosition;
        case 1: return Teach.SortArmToAuto2XPosition;
        case 2: return Teach.SortArmToAuto3XPosition;
        case 3: return Teach.SortArmToAuto4XPosition;
        case 4: return Teach.SortArmToAuto5XPosition;
        case 5: return Teach.SortArmToAuto6XPosition;
    }
    return 0;
}
//---------------------------------------------------------------------------
int TSortArmModule::GetAutoFirstSortY(int AutoIndex)
{
    switch(AutoIndex)
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
int TSortArmModule::GetAutoZPosition(int AutoIndex, int SlotIndex)
{
    switch(AutoIndex)
    {
        case 0:
            switch(SlotIndex)
            {
                case 0: return Teach.SortArmToAuto_1_Z1Position;
                case 1: return Teach.SortArmToAuto_1_Z2Position;
                case 2: return Teach.SortArmToAuto_1_Z3Position;
                case 3: return Teach.SortArmToAuto_1_Z4Position;
            }
            break;
        case 1:
            switch(SlotIndex)
            {
                case 0: return Teach.SortArmToAuto_2_Z1Position;
                case 1: return Teach.SortArmToAuto_2_Z2Position;
                case 2: return Teach.SortArmToAuto_2_Z3Position;
                case 3: return Teach.SortArmToAuto_2_Z4Position;
            }
            break;
        case 2:
            switch(SlotIndex)
            {
                case 0: return Teach.SortArmToAuto_3_Z1Position;
                case 1: return Teach.SortArmToAuto_3_Z2Position;
                case 2: return Teach.SortArmToAuto_3_Z3Position;
                case 3: return Teach.SortArmToAuto_3_Z4Position;
            }
            break;
        case 3:
            switch(SlotIndex)
            {
                case 0: return Teach.SortArmToAuto_4_Z1Position;
                case 1: return Teach.SortArmToAuto_4_Z2Position;
                case 2: return Teach.SortArmToAuto_4_Z3Position;
                case 3: return Teach.SortArmToAuto_4_Z4Position;
            }
            break;
        case 4:
            switch(SlotIndex)
            {
                case 0: return Teach.SortArmToAuto_5_Z1Position;
                case 1: return Teach.SortArmToAuto_5_Z2Position;
                case 2: return Teach.SortArmToAuto_5_Z3Position;
                case 3: return Teach.SortArmToAuto_5_Z4Position;
            }
            break;
        case 5:
            switch(SlotIndex)
            {
                case 0: return Teach.SortArmToAuto_6_Z1Position;
                case 1: return Teach.SortArmToAuto_6_Z2Position;
                case 2: return Teach.SortArmToAuto_6_Z3Position;
                case 3: return Teach.SortArmToAuto_6_Z4Position;
            }
            break;
    }
    return 0;
}
//---------------------------------------------------------------------------
bool TSortArmModule::AreAllSuckersHome()
{
    //AI(HT160S-Maintainer) 20260622 : the ONE canonical SortArm-move interlock, shared by the
    //production MoveSortArmX gate and the Motor Test / Teach pre-move checks so every screen
    //uses the SAME rule. The arm may move only when every ENABLED suck-Z sits on its Home
    //sensor RIGHT NOW (live Led[iHomeLed], not the sticky bHomeFlag). Sim / DUMMY has no card,
    //so ScanMotorStatus reports all LEDs off : short-circuit to true so the simulated / dry-run
    //arm is not blocked (the real machine reads the live sensor).
    if(IsSoftSimulate())
        return true;
    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        TTrayMotor *Motor=GetSuckZMotor(SlotIndex);
        if(Motor==NULL || Motor->GetEnable()==false)
            continue;
        Motor->ScanMotorStatus();
        if(Motor->Led[iHomeLed]==false)
            return false;
    }
    return true;
}
//---------------------------------------------------------------------------
bool TSortArmModule::MoveSortArmX(int Position)
{
    if(HSys.Mot.MSortingArmX==NULL)
        return false;
    if(HSys.Mot.MSortingArmX->CheckSoftLimit(Position)==false)
    {
        ShowMyMessage("Sorting Arm X motor will out of limit");
        return false;
    }
    //AI(HT160S-Maintainer) 20260622 : suck-nozzle home interlock (single rule via
    //AreAllSuckersHome). SortArmX may travel ONLY while every enabled suck-Z rests on its
    //Home sensor. Checked on EVERY call, so a nozzle knocked off home mid-travel (lost steps /
    //thrown off) is caught too, not just before the move starts. A short time-window debounce
    //(not a cycle count, given the ~1ms scan loop) rejects a single bad sensor read; on a
    //confirmed loss decel-stop all motion and raise the alarm (which also drops SystemStart).
    if(AreAllSuckersHome()==false)
    {
        HSys.Mot.MSortingArmX->Stop();   //hold the arm each tick (mode-0 decel stop)
        if(dwSuckHomeLostStart==0)
            dwSuckHomeLostStart=GetTickCount();
        else if((int)(GetTickCount()-dwSuckHomeLostStart)>=SUCK_HOME_LOST_MS)
        {
            dwSuckHomeLostStart=0;
            HSys.StopAllMotor();   //confirmed loss : real decel-stop ALL (DecStopAllMotor is a no-op on MC88X1)
            ShowSystemError("SortArm move blocked : a suck nozzle left its Home sensor (lost steps). Re-home the suckers.", K_RETRY);
        }
        return false;
    }
    dwSuckHomeLostStart=0;
    return HSys.Mot.MSortingArmX->MotorMove(Position);
}
//---------------------------------------------------------------------------
bool TSortArmModule::MoveLoaderY(int LoaderNo, int Position)
{
    TTrayMotor *Motor=GetLoaderMotor(LoaderNo);

    if(Motor==NULL)
        return false;
    if(Motor->CheckSoftLimit(Position)==false)
    {
        ShowMyMessage("Loader Y motor will out of limit");
        return false;
    }
    return Motor->MotorMove(Position);
}
//---------------------------------------------------------------------------
bool TSortArmModule::MoveAutoY(int AutoIndex, int Position)
{
    TTrayMotor *Motor=GetAutoMotor(AutoIndex);

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
bool TSortArmModule::MovePitchToTrayPitch()
{
    int Position=CalculatePitchPosition();

    if(HSys.Mot.MPitchX==NULL || Position==0)
        return true;
    if(HSys.Mot.MPitchX->CheckSoftLimit(Position)==false)
    {
        ShowMyMessage("Pitch X motor will out of limit");
        return false;
    }
    return HSys.Mot.MPitchX->MotorMove(Position);
}
//---------------------------------------------------------------------------
bool TSortArmModule::SortArmZToSafePos()
{
    bool bAllDone=true;

    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        TTrayMotor *Motor=GetSuckZMotor(SlotIndex);
        if(Motor==NULL || Motor->MotorMove(SORT_ARM_SAFE_Z_POSITION)==false)
            bAllDone=false;
    }
    return bAllDone;
}
//---------------------------------------------------------------------------
bool TSortArmModule::MoveToLoaderPick()
{
    int FirstSlot=-1;
    int BaseX;
    int XPosition;
    int YPosition;
    bool bXDone;
    bool bYDone;

    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        if(Slot[SlotIndex].bCanPick)
        {
            FirstSlot=SlotIndex;
            break;
        }
    }
    if(FirstSlot<0)
        return false;

    BaseX=Slot[FirstSlot].PickX-FirstSlot;
    if(BaseX<0)
        return false;

    XPosition=RoundPosition((double)GetLoaderSortX(iActiveLoaderNo)+((double)BaseX)*GetTrayXPitch());
    YPosition=RoundPosition((double)GetLoaderFirstSortY(iActiveLoaderNo)+((double)Slot[FirstSlot].PickY)*GetTrayYPitch());
    bXDone=MoveSortArmX(XPosition);
    bYDone=MoveLoaderY(iActiveLoaderNo, YPosition);
    return (bXDone && bYDone);
}
//---------------------------------------------------------------------------
bool TSortArmModule::MoveToAutoPlace()
{
    int XPosition=RoundPosition((double)GetAutoSortX(iActiveAutoIndex)+((double)iPlaceBaseX)*GetTrayXPitch());
    int YPosition=RoundPosition((double)GetAutoFirstSortY(iActiveAutoIndex)+((double)iPlaceY)*GetTrayYPitch());
    bool bXDone=MoveSortArmX(XPosition);
    bool bYDone=MoveAutoY(iActiveAutoIndex, YPosition);

    return (bXDone && bYDone);
}
//---------------------------------------------------------------------------
bool TSortArmModule::MovePickZDown()
{
    bool bAllDone=true;

    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        if(Slot[SlotIndex].bCanPick)
        {
            TTrayMotor *Motor=GetSuckZMotor(SlotIndex);
            if(Motor==NULL || Motor->MotorMove(GetLoaderZPosition(iActiveLoaderNo, SlotIndex))==false)
                bAllDone=false;
        }
    }
    return bAllDone;
}
//---------------------------------------------------------------------------
bool TSortArmModule::MovePlaceZDown()
{
    bool bAllDone=true;

    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        if(Slot[SlotIndex].bPlaceSelected)
        {
            TTrayMotor *Motor=GetSuckZMotor(SlotIndex);
            if(Motor==NULL || Motor->MotorMove(GetAutoZPosition(iActiveAutoIndex, SlotIndex))==false)
                bAllDone=false;
        }
    }
    return bAllDone;
}
//---------------------------------------------------------------------------
bool TSortArmModule::FindPickCells(int LoaderNo)
{
    TTrayMotor *TrayMotor=GetLoaderVMotor(LoaderNo);
    int XCount=GetTrayXCount();
    int YCount=GetTrayYCount();
    int FirstEnabled;

    ClearPickSelection();
    if(TrayMotor==NULL || TrayMotor->fHasTray==false)
        return false;

    //AI(ht160s-maintainer) 20260616 : per-nozzle enable. Find the first ENABLED sucker
    //and anchor it (not slot 0) to the anchor cell, so a disabled leading slot does not
    //deadlock (the anchor cell would otherwise always map to a slot that never picks).
    //The X geometry in MoveToLoaderPick uses BaseX=PickX-FirstSlot, so any starting slot
    //is handled. If every nozzle is disabled there is nothing to pick.
    FirstEnabled=-1;
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
    {
        if(GeneralSetting.bSuckerEnabled[s])
        {
            FirstEnabled=s;
            break;
        }
    }
    if(FirstEnabled<0)
        return false;

    //AI(ht160s-maintainer) 20260616 : the anchor cell column must be >= FirstEnabled.
    //With the leftmost nozzle(s) disabled the suckers physically cannot reach tray
    //columns 0..FirstEnabled-1 (BaseX would be negative and MoveToLoaderPick would
    //retry forever), so those edge cells are LEFT in the tray instead of stalling.
    for(int YIndex=0; YIndex<YCount; YIndex++)
    {
        for(int XIndex=FirstEnabled; XIndex<XCount; XIndex++)
        {
            if(IsPickableData(TrayMotor->Tray.Data[XIndex][YIndex]))
            {
                for(int SlotIndex=FirstEnabled; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
                {
                    int TrayX=XIndex+(SlotIndex-FirstEnabled);
                    if(GeneralSetting.bSuckerEnabled[SlotIndex]==false)
                        continue;
                    if(TrayX<XCount && IsPickableData(TrayMotor->Tray.Data[TrayX][YIndex]))
                    {
                        Slot[SlotIndex].bCanPick=true;
                        Slot[SlotIndex].PickX=TrayX;
                        Slot[SlotIndex].PickY=YIndex;
                        Slot[SlotIndex].TrayData=TrayMotor->Tray.Data[TrayX][YIndex];
                        //AI(HT160S-Maintainer) 20260604 : P4 capture 2D-looked-up bin for routing.
                        Slot[SlotIndex].BinValue=TrayMotor->GetTrayBin(TrayX, YIndex);
                        //AI(ht160s-lotbin) 20260615 : carry owning lot + 2D code for By Lot+Bin
                        //routing and Production_Log; harmless in Normal mode (unused).
                        Slot[SlotIndex].LotIndex=TrayMotor->GetTrayLot(TrayX, YIndex);
                        Slot[SlotIndex].Code2D=TrayMotor->GetTrayCode2D(TrayX, YIndex);
                    }
                }
                return true;
            }
        }
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260604 : P4 routing key. Prefer 2D-looked-up bin (iBin)
//when set (>0); otherwise fall back to legacy TrayData so behavior is unchanged
//while the 2D map is inert.
int TSortArmModule::GetSlotRoutingBin(int SlotIndex)
{
    if(SlotIndex<0 || SlotIndex>=SORT_ARM_SUCKER_COUNT)
        return EMPTY_IC;
    if(Slot[SlotIndex].BinValue>0)
        return Slot[SlotIndex].BinValue;
    return Slot[SlotIndex].TrayData;
}
//---------------------------------------------------------------------------
int TSortArmModule::GetMappedAutoIndex(int BinData, int LotIndex, bool &bFixedArea)
{
    int Area;

    bFixedArea=false;
    //AI(ht160s-lotbin) 20260615 : By Lot+Bin mode. The (Lot,Bin) pair was bound to
    //an Auto at CCD scan time (LotBinBinding.ResolveAuto), so here we only READ the
    //binding - no allocation side-effect during the per-slot/per-Auto place scan.
    //Error bins (2D fail / no bin setting) and ICs with no owning lot route to the
    //Error Auto. Color is not used for sorting in this mode (AMR identity tray only).
    if(GeneralSetting.bUseLotBinSortMode)
    {
        int AutoIndex;
        bFixedArea=true;
        if(LotIndex>=0 && BinAreaMap.IsErrorBin(BinData)==false)
        {
            AutoIndex=LotBinBinding.FindAuto(LotIndex, BinData);
            if(AutoIndex>=0)
                return AutoIndex;
        }
        Area=BinAreaMap.GetErrorBinArea();
        if(Area>=eHT160BinAreaAuto1 && Area<=eHT160BinAreaAuto6)
            return Area-eHT160BinAreaAuto1;
        return -1;
    }
    if(CosFunction.bUseBinAreaMap)
    {
        Area=BinAreaMap.GetAreaByBin(BinData);
        if(Area>=eHT160BinAreaAuto1 && Area<=eHT160BinAreaAuto6)
        {
            bFixedArea=true;
            return Area-eHT160BinAreaAuto1;
        }
        if(Area==eHT160BinAreaColor)
        {
            bFixedArea=true;
            return -1;
        }
        //AI(general) 20260609 : unconfigured / unknown bin -> Error bin area.
        //Never fall through to place-anywhere; keep placement deterministic so each
        //Auto tray fills top-to-bottom, left-to-right (matches old HT160 Direction=0).
        Area=BinAreaMap.GetErrorBinArea();
        bFixedArea=true;
        if(Area>=eHT160BinAreaAuto1 && Area<=eHT160BinAreaAuto6)
            return Area-eHT160BinAreaAuto1;
        return -1;
    }

    if(BinData>=eHT160BinAreaAuto1 && BinData<=eHT160BinAreaAuto6)
    {
        bFixedArea=true;
        return BinData-eHT160BinAreaAuto1;
    }
    return -1;
}
//---------------------------------------------------------------------------
bool TSortArmModule::CanPlaceSlotToAuto(int SlotIndex, int AutoIndex)
{
    bool bFixedArea=false;
    int MappedAutoIndex;

    if(SlotIndex<0 || SlotIndex>=SORT_ARM_SUCKER_COUNT || Slot[SlotIndex].bHasIC==false)
        return false;

    MappedAutoIndex=GetMappedAutoIndex(GetSlotRoutingBin(SlotIndex), Slot[SlotIndex].LotIndex, bFixedArea);
    if(MappedAutoIndex>=0)
        return (MappedAutoIndex==AutoIndex);
    return (bFixedArea==false);
}
//---------------------------------------------------------------------------
bool TSortArmModule::FindPlaceCells(int AutoIndex)
{
    //AI(HT160S-Maintainer) 20260605 : AMR gate. In AMR mode the first two trays of each
    //Auto stack are identity/cover trays that must NOT receive IC. Skip this Auto until
    //its working tray is a normal-kind tray. No effect in Normal mode (always ready).
    if(AutoModule!=NULL && AutoModule->IsReadyForSortArmPlace(AutoIndex)==false)
        return false;
    TTrayMotor *TrayMotor=GetAutoVMotor(AutoIndex);
    int XCount=GetTrayXCount();
    int YCount=GetTrayYCount();

    ClearPlaceSelection();
    if(TrayMotor==NULL || TrayMotor->fHasTray==false)
        return false;

    //AI(general) 20260609 : Option A compact fill. Always start from the FIRST empty
    //cell of the Auto tray (top-to-bottom, left-to-right) so cells fill strictly in
    //sequential 1,2,3... order with no gaps. The old algorithm pinned each sucker to
    //BaseX+SlotIndex; a sucked group split across bins/Auto areas then left column
    //holes (scattered placement). Here we only ever fill the next empty contiguous run.
    int TargetX=-1;
    int TargetY=-1;
    int FirstSlot=-1;
    int PlacedCount=0;

    for(int YIndex=0; YIndex<YCount && TargetY<0; YIndex++)
    {
        for(int XIndex=0; XIndex<XCount; XIndex++)
        {
            if(TrayMotor->Tray.Data[XIndex][YIndex]==EMPTY_IC)
            {
                TargetX=XIndex;
                TargetY=YIndex;
                break;
            }
        }
    }
    if(TargetY<0)
        return false;

    //AI(general) 20260609 : pick the lowest-index held sucker whose routing bin maps to
    //this Auto. Lowest slot maximises iPlaceBaseX (=TargetX-FirstSlot) and so minimises
    //any left-shift of the arm X axis.
    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        if(CanPlaceSlotToAuto(SlotIndex, AutoIndex))
        {
            FirstSlot=SlotIndex;
            break;
        }
    }
    if(FirstSlot<0)
        return false;

    //AI(general) 20260609 : extend a CONTIGUOUS sucker run (FirstSlot, FirstSlot+1, ...)
    //onto contiguous empty cells (TargetX, TargetX+1, ...). Stop at the first sucker that
    //does not route to this Auto, or the first non-empty / out-of-range cell, so the run
    //is always gap-free. Suckers are a fixed-pitch comb : one Z-down drops sucker s at
    //column (iPlaceBaseX+s), so a contiguous run lands on contiguous columns.
    for(int RunSlot=FirstSlot; RunSlot<SORT_ARM_SUCKER_COUNT; RunSlot++)
    {
        int TrayX=TargetX+(RunSlot-FirstSlot);

        if(CanPlaceSlotToAuto(RunSlot, AutoIndex)==false)
            break;
        if(TrayX>=XCount)
            break;
        if(TrayMotor->Tray.Data[TrayX][TargetY]!=EMPTY_IC)
            break;
        Slot[RunSlot].bPlaceSelected=true;
        Slot[RunSlot].PlaceX=TrayX;
        Slot[RunSlot].PlaceY=TargetY;
        PlacedCount++;
    }
    if(PlacedCount==0)
        return false;

    iPlaceBaseX=TargetX-FirstSlot;
    iPlaceY=TargetY;
    return true;
}
//---------------------------------------------------------------------------
bool TSortArmModule::SelectPlaceAuto()
{
    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        if(Slot[SlotIndex].bHasIC)
        {
            bool bFixedArea=false;
            int AutoIndex=GetMappedAutoIndex(GetSlotRoutingBin(SlotIndex), Slot[SlotIndex].LotIndex, bFixedArea);
            if(AutoIndex>=0 && FindPlaceCells(AutoIndex))
            {
                iActiveAutoIndex=AutoIndex;
                return true;
            }
        }
    }

    for(int AutoIndex=0; AutoIndex<SORT_ARM_AUTO_COUNT; AutoIndex++)
    {
        if(FindPlaceCells(AutoIndex))
        {
            iActiveAutoIndex=AutoIndex;
            return true;
        }
    }
    iActiveAutoIndex=-1;
    ClearPlaceSelection();
    return false;
}
//---------------------------------------------------------------------------
bool TSortArmModule::SuckSelectedSlots()
{
    bool bAllDone=true;

    if(IsSoftSimulate())
        return true;

    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        if(Slot[SlotIndex].bCanPick)
        {
            TMySucker *Sucker=GetSucker(SlotIndex);
            if(Sucker==NULL)
            {
                bAllDone=false;
            }
            else if(Sucker->Suck()==false)
            {
                bAllDone=false;
                if(Sucker->Error)
                {
                    ShowSuckError(*Sucker, 1, K_RETRY|K_SKIP, "SortArm Pick");
                    Sucker->Reset();
                }
            }
        }
    }
    return bAllDone;
}
//---------------------------------------------------------------------------
bool TSortArmModule::DestroySelectedSlots()
{
    bool bAllDone=true;

    if(IsSoftSimulate())
        return true;

    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        if(Slot[SlotIndex].bPlaceSelected)
        {
            TMySucker *Sucker=GetSucker(SlotIndex);
            if(Sucker==NULL)
            {
                bAllDone=false;
            }
            else if(Sucker->Destroy()==false)
            {
                bAllDone=false;
                if(Sucker->Error)
                {
                    ShowSuckError(*Sucker, 2, K_RETRY|K_SKIP, "SortArm Place");
                    Sucker->Reset();
                }
            }
        }
    }
    return bAllDone;
}
//---------------------------------------------------------------------------
void TSortArmModule::TransferPickDataFromLoader()
{
    TTrayMotor *TrayMotor=GetLoaderVMotor(iActiveLoaderNo);

    if(TrayMotor==NULL)
        return;
    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        if(Slot[SlotIndex].bCanPick)
        {
            TMySucker *Sucker=GetSucker(SlotIndex);
            TrayMotor->SetTraySingleData(Slot[SlotIndex].PickX, Slot[SlotIndex].PickY, EMPTY_IC);
            g_DeviceInfo.AddInputInfo(SlotIndex, Slot[SlotIndex].PickY, Slot[SlotIndex].PickX, "");
            //AI(ht160s-lotbin) 20260615 : record this IC's owning Lot + 2D code on the
            //production trace line. Empty for ICs picked without a 2D lookup (Normal mode
            //or pre-feature data) - the columns simply stay blank.
            {
                AnsiString sLotID="";
                TLotRunInfo *Lot=LotRegistry.GetLot(Slot[SlotIndex].LotIndex);
                if(Lot!=NULL)
                    sLotID=Lot->sLotID;
                g_DeviceInfo.AddIcIdentity(SlotIndex, sLotID, Slot[SlotIndex].Code2D);
            }
            Slot[SlotIndex].bHasIC=true;
            Slot[SlotIndex].bCanPick=false;
            if(Sucker!=NULL)
                Sucker->Item=Slot[SlotIndex].TrayData;
        }
    }
    UpdateKitSuckState();
}
//---------------------------------------------------------------------------
void TSortArmModule::TransferPlaceDataToAuto()
{
    TTrayMotor *TrayMotor=GetAutoVMotor(iActiveAutoIndex);

    if(TrayMotor==NULL)
        return;
    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        if(Slot[SlotIndex].bPlaceSelected)
        {
            TrayMotor->SetTraySingleData(Slot[SlotIndex].PlaceX, Slot[SlotIndex].PlaceY, HAS_OK_IC);
            //AI(ht160s-motion-view) 20260618 : per-Auto output IC count for the Unload
            //palAutoXXCnt display (HT172 ShowBinCount used tRunData.TrayICCnt). eAuto1=index 1.
            if(iActiveAutoIndex>=0 && iActiveAutoIndex<SORT_ARM_AUTO_COUNT)
                tRunData.TrayICCnt[iActiveAutoIndex+1]++;
            g_DeviceInfo.AddBinInfo(SlotIndex, iActiveAutoIndex, Slot[SlotIndex].TrayData);
            g_DeviceInfo.AddOutputInfo(SlotIndex, "Auto"+IntToStr(iActiveAutoIndex+1), "", Slot[SlotIndex].PlaceY, Slot[SlotIndex].PlaceX);
            ClearSlot(SlotIndex);
        }
    }
    UpdateKitSuckState();
}
//---------------------------------------------------------------------------
bool TSortArmModule::DoPickFromLoader(int Flag)
{
    if(Flag==0)
    {
        PickTask=1;
        return false;
    }

    switch(PickTask)
    {
        case 1:
            if(FindPickCells(iActiveLoaderNo)==false)
            {
                PickTask=1;
                return true;
            }
            // Take exclusive ownership of the Loader-Y axis before approaching.
            if(LoaderModule->AcquireSortOwner(iActiveLoaderNo)==false)
            {
                ClearPickSelection();
                return false;
            }
            PickTask=10;
            break;

        case 10:
            if(SortArmZToSafePos())
                PickTask=20;
            break;

        case 20:
            if(MovePitchToTrayPitch())
                PickTask=30;
            break;

        case 30:
            if(MoveToLoaderPick())
                PickTask=40;
            break;

        case 40:
            // Re-validate before the irreversible Z-down + suck: ownership must
            // still be held and the Loader-Y axis must be confirmed in position.
            if(LoaderModule->IsSortOwnerHeld(iActiveLoaderNo)==false)
            {
                PickTask=70;
                break;
            }
            if(MoveToLoaderPick()==false)
                break;
            PickTask=45;
            break;

        case 45:
            if(MovePickZDown())
                PickTask=50;
            break;

        case 50:
            if(SuckSelectedSlots())
            {
                TransferPickDataFromLoader();
                PickTask=60;
            }
            break;

        case 60:
            if(SortArmZToSafePos())
            {
                LoaderModule->ReleaseSortOwner(iActiveLoaderNo);
                PickTask=1;
                return true;
            }
            break;

        case 70:
            // Handshake lost before suck: lift Z safe, release axis, bail out.
            if(SortArmZToSafePos())
            {
                LoaderModule->ReleaseSortOwner(iActiveLoaderNo);
                ClearPickSelection();
                PickTask=1;
                return true;
            }
            break;

        default:
            PickTask=1;
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(general) 20260609 : diagnostic. Toggled by GeneralSetting.bShowSortArmPlaceCheck
//([Diagnostic] ShowSortArmPlaceCheck in system\General.ini, default OFF). When ON,
//pop a modal message comparing the CURRENT motor position (encoder) against the
//EXPECTED place position, just after the place XY move and before Z-down. NOTE: the
//modal pauses the auto flow on every placement, so this is a check mode : leave OFF
//for production.
void TSortArmModule::ShowPlaceDebugInfo()
{
    if(GeneralSetting.bShowSortArmPlaceCheck==false)
        return;
    if(iActiveAutoIndex<0)
        return;

    int ExpectX=RoundPosition((double)GetAutoSortX(iActiveAutoIndex)+((double)iPlaceBaseX)*GetTrayXPitch());
    int ExpectY=RoundPosition((double)GetAutoFirstSortY(iActiveAutoIndex)+((double)iPlaceY)*GetTrayYPitch());
    TTrayMotor *YMotor=GetAutoMotor(iActiveAutoIndex);
    int NowX=(HSys.Mot.MSortingArmX!=NULL)?HSys.Mot.MSortingArmX->ReadPos():0;
    int NowY=(YMotor!=NULL)?YMotor->ReadPos():0;
    AnsiString Msg;

    Msg ="SortArm Place Position Check\n";
    Msg+="Auto"+IntToStr(iActiveAutoIndex+1)+"  row="+IntToStr(iPlaceY)+"  col="+IntToStr(iPlaceBaseX)+"\n";
    Msg+="------------------------------\n";
    Msg+="Sorting Arm X\n";
    Msg+="  Now    = "+IntToStr(NowX)+"\n";
    Msg+="  Target = "+IntToStr(ExpectX)+"\n";
    Msg+="  Diff   = "+IntToStr(NowX-ExpectX)+"\n";
    Msg+="Auto"+IntToStr(iActiveAutoIndex+1)+" Y\n";
    Msg+="  Now    = "+IntToStr(NowY)+"\n";
    Msg+="  Target = "+IntToStr(ExpectY)+"\n";
    Msg+="  Diff   = "+IntToStr(NowY-ExpectY);
    ShowMyMessage(Msg);
}
//---------------------------------------------------------------------------
bool TSortArmModule::DoPlaceToAuto(int Flag)
{
    if(Flag==0)
    {
        PlaceTask=1;
        return false;
    }

    switch(PlaceTask)
    {
        case 1:
            if(SelectPlaceAuto()==false)
            {
                return false;
            }
            PlaceTask=10;
            break;

        case 10:
            if(SortArmZToSafePos())
                PlaceTask=20;
            break;

        case 20:
            if(MovePitchToTrayPitch())
                PlaceTask=30;
            break;

        case 30:
            if(MoveToAutoPlace())
                PlaceTask=35;
            break;

        case 35:
            //AI(general) 20260609 : optional diagnostic. When the flag is on, pause and
            //show actual vs expected motor positions before the irreversible Z-down.
            ShowPlaceDebugInfo();
            PlaceTask=40;
            break;

        case 40:
            if(MovePlaceZDown())
                PlaceTask=50;
            break;

        case 50:
            if(DestroySelectedSlots())
            {
                TransferPlaceDataToAuto();
                PlaceTask=60;
            }
            break;

        case 60:
            if(SortArmZToSafePos())
            {
                PlaceTask=1;
                return true;
            }
            break;

        default:
            PlaceTask=1;
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TSortArmModule::IsCleanOutFinish()
{
    return bCleanOutFinish;
}
//---------------------------------------------------------------------------
bool TSortArmModule::IsOneCycleFinish()
{
    //AI(HT160S-Maintainer) 20260605 : SortArm has placed its held IC and stopped.
    return bOneCycleFinish;
}
//---------------------------------------------------------------------------
//AI(ht160s-state-record-analysis) 20260612 : expose pick/place sub-task so the
//Store Hangup snapshot can record WHICH step the arm is in (e.g. 30 = moving XY,
//40 = Z-down). Top-level DoSortArm Task only shows 1/100/200.
int TSortArmModule::GetPickTask()
{
    return PickTask;
}
//---------------------------------------------------------------------------
int TSortArmModule::GetPlaceTask()
{
    return PlaceTask;
}
//---------------------------------------------------------------------------
//AI(ht160s-state-record-analysis) 20260616 : read-only dump of what the arm is
//holding and where each held IC routes, for the Store Hangup SortArmDecision.txt.
//Pairs with TAutoModule::DescribeStation so "SortArm frozen at place" can be traced
//to "slot held bin=X -> Auto Y, but Y has no contiguous EMPTY_IC run that fits".
//GetSlotRoutingBin / GetMappedAutoIndex are pure lookups (no allocation side-effect),
//so calling them here does NOT disturb the live place decision.
AnsiString TSortArmModule::DescribeHolding()
{
    AnsiString s;
    s  = "[SortArm]\r\n";
    s += "PickTask=" + IntToStr(PickTask) + "  PlaceTask=" + IntToStr(PlaceTask) + "\r\n";
    s += "ActiveLoaderNo=" + IntToStr(iActiveLoaderNo)
       + "  ActiveAutoIndex=" + IntToStr(iActiveAutoIndex) + "\r\n";

    int HoldCount=0;
    for(int i=0; i<SORT_ARM_SUCKER_COUNT; i++)
        if(Slot[i].bHasIC)
            HoldCount++;
    s += "HoldingIC=" + IntToStr(HoldCount) + " / " + IntToStr(SORT_ARM_SUCKER_COUNT) + "\r\n";

    for(int i=0; i<SORT_ARM_SUCKER_COUNT; i++)
    {
        s += "  Slot" + IntToStr(i) + ": hasIC=" + IntToStr(Slot[i].bHasIC ? 1 : 0);
        if(Slot[i].bHasIC)
        {
            int  RouteBin = GetSlotRoutingBin(i);
            bool bFixed   = false;
            int  Mapped   = GetMappedAutoIndex(RouteBin, Slot[i].LotIndex, bFixed);
            AnsiString Dest = (Mapped>=0) ? ("Auto"+IntToStr(Mapped+1)) : AnsiString("none(Color/Err)");
            s += "  TrayData=" + IntToStr(Slot[i].TrayData)
               + "  Bin="      + IntToStr(Slot[i].BinValue)
               + "  RouteBin=" + IntToStr(RouteBin)
               + "  LotIdx="   + IntToStr(Slot[i].LotIndex)
               + "  Code2D="   + Slot[i].Code2D
               + "  ->" + Dest;
        }
        s += "\r\n";
    }
    return s;
}
//---------------------------------------------------------------------------
void TSortArmModule::DoSortArm(int &Task)
{
    if(LoaderModule==NULL)
        return;

    switch(Task)
    {
        case 1:
            if(HasHoldingIC())
            {
                PlaceTask=1;
                Task=200;
                break;
            }

            //AI(HT160S-Maintainer) 20260605 : OneCycle place-before-stop. Any held IC
            //was sent to place above; in OneCycle do NOT start a new pick : declare
            //finish so csystem can stop (or resume a nested CleanOut).
            if(HSys.Sys.RunMode==Run_OneCycle)
            {
                bOneCycleFinish=true;
                break;
            }

            iActiveLoaderNo=LoaderModule->GetSortingLoaderNo();
            if(iActiveLoaderNo>0)
            {
                PickTask=1;
                Task=100;
                break;
            }
            //AI(HT160S-Maintainer) 20260605 : CleanOut cascade. Idle, no held IC and no
            //loader ready to sort. When both Loader sides have drained, SortArm has
            //nothing left to move : declare its own CleanOut finish.
            if(HSys.Sys.RunMode==Run_CleanOut &&
               LoaderModule->IsAllCleanOutFinish())
                bCleanOutFinish=true;
            break;

        case 100:
            if(DoPickFromLoader(1))
            {
                PlaceTask=1;
                Task=200;
            }
            break;

        case 200:
            if(HasHoldingIC()==false && PlaceTask<=1)   // no holding IC and place is idle : nothing to place, leave case 200
            {
                Task=1;
                break;
            }
            if(DoPlaceToAuto(1))
            {
                Task=HasHoldingIC()?200:1;
            }
            break;

        default:
            Task=1;
            break;
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-sortarm-flow) 20260617 : Teach Advanced single-nozzle point test.
//Move ONE sucker over ONE tray cell (Loader1/2 or Auto1..6) reusing the same
//geometry as MoveToLoaderPick/MoveToAutoPlace, generalised so any slot can be
//placed over any column : ArmX = baseSortX + (Col-Slot)*XPitch. Non-blocking;
//the caller (TfTeach) drives Task by its timer. Set Task=0 to start; returns
//true when finished. Z-safe-before-XY is mandatory, exactly like the pick flow.
//SlotIndex 0..3; Target 1=Loader1,2=Loader2,11..16=Auto1..6; Col/Row 0-based.
bool TSortArmModule::CanMoveSuckerToCell(int SlotIndex, int Target, int Col, int Row, AnsiString &Err)
{
    int LoaderNo=0;
    int AutoIndex=-1;
    int BaseSortX;
    int FirstSortY;
    int XPosition;
    int YPosition;
    TTrayMotor *YMotor;

    Err="";
    if(SlotIndex<0 || SlotIndex>=SORT_ARM_SUCKER_COUNT)
    {
        Err="Invalid sucker";
        return false;
    }
    if(Target==1 || Target==2)
        LoaderNo=Target;
    else if(Target>=11 && Target<=16)
        AutoIndex=Target-11;
    else
    {
        Err="Invalid target area";
        return false;
    }
    if(Col<0 || Col>=GetTrayXCount())
    {
        Err="Column out of tray range (1.."+IntToStr(GetTrayXCount())+")";
        return false;
    }
    if(Row<0 || Row>=GetTrayYCount())
    {
        Err="Row out of tray range (1.."+IntToStr(GetTrayYCount())+")";
        return false;
    }
    if((Col-SlotIndex)<0)
    {
        Err="Sucker cannot reach this column (Suck index > Column)";
        return false;
    }
    if(LoaderNo>0)
    {
        BaseSortX=GetLoaderSortX(LoaderNo);
        FirstSortY=GetLoaderFirstSortY(LoaderNo);
        YMotor=GetLoaderMotor(LoaderNo);
    }
    else
    {
        BaseSortX=GetAutoSortX(AutoIndex);
        FirstSortY=GetAutoFirstSortY(AutoIndex);
        YMotor=GetAutoMotor(AutoIndex);
    }
    XPosition=RoundPosition((double)BaseSortX+((double)(Col-SlotIndex))*GetTrayXPitch());
    YPosition=RoundPosition((double)FirstSortY+((double)Row)*GetTrayYPitch());
    if(HSys.Mot.MSortingArmX==NULL || HSys.Mot.MSortingArmX->CheckSoftLimit(XPosition)==false)
    {
        Err="Sorting Arm X target over soft limit";
        return false;
    }
    if(YMotor==NULL || YMotor->CheckSoftLimit(YPosition)==false)
    {
        Err="Target Y target over soft limit";
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
bool TSortArmModule::MoveSuckerToCell(int SlotIndex, int Target, int Col, int Row, bool bZDown, int &Task)
{
    int LoaderNo=0;
    int AutoIndex=-1;
    int BaseSortX;
    int FirstSortY;
    int XPosition;
    int YPosition;
    int ZPosition;
    bool bXDone;
    bool bYDone;
    TTrayMotor *ZMotor;

    if(SlotIndex<0 || SlotIndex>=SORT_ARM_SUCKER_COUNT)
    {
        Task=900;
        return true;
    }
    if(Target==1 || Target==2)
        LoaderNo=Target;
    else if(Target>=11 && Target<=16)
        AutoIndex=Target-11;
    else
    {
        Task=900;
        return true;
    }

    switch(Task)
    {
        case 0:
            Task=10;
            break;

        case 10:
            if(SortArmZToSafePos())
                Task=20;
            break;

        case 20:
            if(MovePitchToTrayPitch())
                Task=30;
            break;

        case 30:
            if(LoaderNo>0)
            {
                BaseSortX=GetLoaderSortX(LoaderNo);
                FirstSortY=GetLoaderFirstSortY(LoaderNo);
            }
            else
            {
                BaseSortX=GetAutoSortX(AutoIndex);
                FirstSortY=GetAutoFirstSortY(AutoIndex);
            }
            XPosition=RoundPosition((double)BaseSortX+((double)(Col-SlotIndex))*GetTrayXPitch());
            YPosition=RoundPosition((double)FirstSortY+((double)Row)*GetTrayYPitch());
            bXDone=MoveSortArmX(XPosition);
            if(LoaderNo>0)
                bYDone=MoveLoaderY(LoaderNo, YPosition);
            else
                bYDone=MoveAutoY(AutoIndex, YPosition);
            if(bXDone && bYDone)
                Task=bZDown?40:100;
            break;

        case 40:
            ZMotor=GetSuckZMotor(SlotIndex);
            if(ZMotor==NULL)
            {
                Task=100;
                break;
            }
            ZPosition=(LoaderNo>0)?GetLoaderZPosition(LoaderNo, SlotIndex):GetAutoZPosition(AutoIndex, SlotIndex);
            if(ZMotor->MotorMove(ZPosition))
                Task=100;
            break;

        case 100:
            return true;

        default:
            Task=900;
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
void InitializeSortArmModule()
{
    if(SortArmModule==NULL)
        SortArmModule=new TSortArmModule();
}
//---------------------------------------------------------------------------
void ShutdownSortArmModule()
{
    if(SortArmModule!=NULL)
    {
        delete SortArmModule;
        SortArmModule=NULL;
    }
}
//---------------------------------------------------------------------------