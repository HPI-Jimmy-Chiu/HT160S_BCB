#include "IncludeAllHeader.h"       //Dell 將.h統一,可加速build
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#include <IniFiles.hpp>
#pragma hdrstop
#include "language.h"

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
//AI(ht160s-maintainer) 20260624 : datum (base) sucker index, 0-based. The taught station base
//X (GetLoaderSortX/GetAutoSortX) is the absolute X at which THIS sucker sits over tray column
//0; every other sucker fans out from it via the MPitchX comb, so it is the only nozzle whose
//position does not move with tray pitch. This machine bolts suck2 to the X carriage => index
//1. 0 reproduces the legacy suck1-datum behavior. Port of HT172 iBaseSuckX (1-based =1).
static const int SORT_ARM_BASE_SUCKER_INDEX=1;
//AI(ht160s-maintainer) 20260627 : P2 HT172-align datum bias = calibration base -> tray top-left
//corner, then corner->first-cell (+XStart/+YStart): X=base+XBias+XStart, Y=base+YBias+YStart.
//XBias/YBias are per-machine commissioning values in General.ini [SortArm] XDatumBias/YDatumBias
//(default -1000/-1000, 1/100mm) read via GeneralSetting. Always applied; the old UseTrayDatumModel
//gate and the compile-time SORT_ARM_X/Y_DATUM_BIAS constants were removed.
static const int SORT_ARM_AUTO_COUNT=6;
static const int iDestroyCheckMS=300;   //AI(ht160s-residue) 20260624 : re-suck settle (ms) for place residue check
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
    //AI(ht160s-maintainer) 20260624 : datum sucker for absolute SortArm X (HT172 iBaseSuckX
    //port). Set once at construction - machine build attribute, not a per-run flag. suck2=1.
    iBaseSuckX=SORT_ARM_BASE_SUCKER_INDEX;
    ApplyPnPDefaults();   //AI(ht160s-pnp) 20260626 : seed PnP scalars before any recipe load
    InitialFlag();
}
//---------------------------------------------------------------------------
void TSortArmModule::ApplyPnPDefaults()
{
    //AI(ht160s-pnp) 20260626 : safe defaults used at construction and whenever a recipe has no
    //[PnP] section. dDestroyCheckTime defaults to 0.3s (=300ms) so the Task 1 pre-lift blow dwell
    //is active out of the box; pick/place settle default to 0 (no behavior change until tuned).
    dPickDelaySec=0.0;
    dPlaceDelaySec=0.0;
    dDestroyCheckTime=0.3;
}
//---------------------------------------------------------------------------
void TSortArmModule::SetPnPParameters(double PickDelaySec, double PlaceDelaySec, double DestroyCheckSec)
{
    //AI(ht160s-pnp) 20260626 : runtime push from TfSetup::ApplyPnPToSortArm. Clamp negatives.
    if(PickDelaySec<0.0)
        PickDelaySec=0.0;
    if(PlaceDelaySec<0.0)
        PlaceDelaySec=0.0;
    if(DestroyCheckSec<0.0)
        DestroyCheckSec=0.0;
    dPickDelaySec=PickDelaySec;
    dPlaceDelaySec=PlaceDelaySec;
    dDestroyCheckTime=DestroyCheckSec;
}
//---------------------------------------------------------------------------
int TSortArmModule::GetDestroyCheckMS()
{
    //AI(ht160s-pnp) 20260626 : seconds -> ms for the pre-lift blow dwell. A 0/negative recipe value
    //falls back to 300ms so the safety dwell can never be disabled by a bad recipe.
    int ms=(int)(dDestroyCheckTime*1000.0);
    if(ms<=0)
        ms=300;
    return ms;
}
//---------------------------------------------------------------------------
void TSortArmModule::StartPnpSettle(double Sec)
{
    //AI(ht160s-pnp) 20260626 : arm the pick/place Z-down settle dwell. Sim uses 0ms so the sim
    //cycle stays fast and deterministic (no physical settle to wait for).
    int ms;

    PnpSettle.Clear();
    ms=(int)(Sec*1000.0);
    if(ms<0)
        ms=0;
    if(IsSoftSimulate())
        ms=0;
    PnpSettle.SetMS(ms);
    PnpSettle.On();
}
//---------------------------------------------------------------------------
bool TSortArmModule::PnpSettleElapsed()
{
    //AI(ht160s-pnp) 20260626 : true once the settle dwell completes (HTimer::Off is true at 0ms).
    return PnpSettle.Off();
}
//---------------------------------------------------------------------------
//AI(ht160s-actuator-timer) 20260627 : freeze/thaw the per-slot residue-check wall-clock
//timers (ResidueDelay[]) so a machine pause taken during a place residue re-suck wait is
//not charged against the settle budget -- no false residue verdict on resume. Called from
//csystem PauseActuatorTimeoutTimers/ReStartActuatorTimeoutTimers on the SystemStart pause/
//resume edges, alongside Cylinder[]/SortArmSuck.
void TSortArmModule::PauseTimeoutTimers()
{
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
        ResidueDelay[s].Pause();
}
//---------------------------------------------------------------------------
void TSortArmModule::ReStartTimeoutTimers()
{
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
        ResidueDelay[s].ReStart();
}
//---------------------------------------------------------------------------
void TSortArmModule::InitialFlag(bool bKeepMaterial)
{
    Status=SAS_IDLE;   //AI(ht160s-status) 20260703 : held ICs (bKeepMaterial) re-commit to SAS_PLACING on the next DoSortArm tick
    PickTask=1;
    PlaceTask=1;
    iActiveLoaderNo=0;
    iActiveAutoIndex=-1;
    iPlaceBaseX=0;
    iPlaceY=0;
    bCleanOutFinish=false;
    bOneCycleFinish=false;
    dwSuckHomeLostStart=0;
    iPickRetryCount=0;   //AI(ht160s-pick-retry) 20260702 : fresh pick-retry budget on home/init
    iResidueAutoIndex=-1;   //AI(ht160s-residue) 20260624 : no pending residue report
    bResidueArmed=false;   //AI(ht160s-residue) 20260625 : disarm; armed at place case 60
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)   //AI(ht160s-residue) 20260624 : reset residue-check state on home/init
    {
        bNeedResidueCheck[s]=false;
        ResidueTask[s]=1;
        ResidueDelay[s].Clear();
        //AI(ht160s-pnp) 20260626 : an abort/home during the place hold-through-lift window (case50..70)
        //leaves blow latched ON (ClearSlot->Reset() never touches OffSw). Physically clear it here, BEFORE
        //dropping the flag, mirroring HT172's terminal Normal(). Real machine only; sucker must exist.
        if(bBlowSlot[s] && IsSoftSimulate()==false)
        {
            TMySucker *BlowSucker=GetSucker(s);
            if(BlowSucker!=NULL)
                BlowSucker->OffDestroy();
        }
        bBlowSlot[s]=false;          //AI(ht160s-pnp) 20260626 : clear pending blow-off targets on home/init
        bPickSuckErr[s]=false;       //AI(ht160s-pick-retry) 20260702 : drop pick-error latches on home/init
    }
    BlowDwell.Clear();               //AI(ht160s-pnp) 20260626 : reset pre-lift blow dwell
    PnpSettle.Clear();               //AI(ht160s-pnp) 20260626 : reset pick/place settle dwell
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
    Slot[SlotIndex].bManual2D=false;   //AI(ht160s-ccd-manual2d) : clear manual-2D flag
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
    return TrayForm.XPitch*100.0;   //AI(ht160s-maintainer) 20260624 : tray pitch is mm (setup.ini) but teach coords are 1/100mm; convert so cell-position math is unit-consistent. HT172 multiplies by 100 at load (aSortArm.cpp:208); port dropped it so pitch was 100x too small.
}
//---------------------------------------------------------------------------
double TSortArmModule::GetTrayYPitch()
{
    return TrayForm.YPitch*100.0;   //AI(ht160s-maintainer) 20260624 : mm to 1/100mm, see GetTrayXPitch.
}
//---------------------------------------------------------------------------
double TSortArmModule::GetTrayXStart()
{
    //AI(ht160s-maintainer) 20260624 : tray top-left corner -> first IC offset, X (mm->1/100mm). P2 HT172-align.
    return TrayForm.XStart*100.0;
}
//---------------------------------------------------------------------------
double TSortArmModule::GetTrayYStart()
{
    return TrayForm.YStart*100.0;
}
//---------------------------------------------------------------------------
int TSortArmModule::RoundPosition(double Value)
{
    if(Value>=0.0)
        return (int)(Value+0.5);
    return (int)(Value-0.5);
}
//---------------------------------------------------------------------------
//AI(ht160s-maintainer) 20260624 : single source for SortArm cell -> arm X. ColMinusSlot is the
//raw comb offset (trayColumn - activeSuckerIndex). Add iBaseSuckX so the taught base X refers
//to the datum sucker (suck2), not suck0:  X = baseX + (Col - Slot + iBaseSuckX)*pitch.
//iBaseSuckX==0 reproduces the legacy formula; travel is still clamped by MoveSortArmX.
int TSortArmModule::GetSortArmCellX(int BaseSortX, int ColMinusSlot)
{
    double Datum=(double)GeneralSetting.iSortArmXDatumBias+GetTrayXStart();
    return RoundPosition((double)BaseSortX+Datum+
        ((double)(ColMinusSlot+iBaseSuckX))*GetTrayXPitch());
}
//---------------------------------------------------------------------------
int TSortArmModule::GetSortArmCellY(int BaseSortY, int Row)
{
    //AI(ht160s-maintainer) 20260624 : symmetric cell->arm Y helper (P1 of HT172-align).
    //Single source for SortArm Y so the former inline sites (pick/place/teach/debug) no
    //longer drift. Datum bias (General.ini [SortArm] YDatumBias) always applied.
    double Datum=(double)GeneralSetting.iSortArmYDatumBias+GetTrayYStart();
    return RoundPosition((double)BaseSortY+Datum+((double)Row)*GetTrayYPitch());
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
    //sensor RIGHT NOW (live Led[iHomeLed], not the sticky bHomeFlag).
    //AI(HT160S-Maintainer) 20260622 : anti-collision is a HARD safety law - keep it ACTIVE in
    //real-machine DUMMY/HAS_TRAY/REALLY. In DUMMY the motors and cylinders STILL move physically
    //(DUMMY only skips correctness sensor confirmation), so the arm can still crash; the interlock
    //must read the live Home sensor in every run mode. Bypass ONLY the SOFT_SIMULATE dev build,
    //where there is no card so ScanMotorStatus reports all LEDs off (a runtime DUMMY bypass would
    //wrongly disarm the interlock on the real machine).
    #ifdef SOFT_SIMULATE
    return true;
    #else
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
    #endif
}
//---------------------------------------------------------------------------
bool TSortArmModule::MoveSortArmX(int Position)
{
    if(HSys.Mot.MSortingArmX==NULL)
        return false;
    if(HSys.Mot.MSortingArmX->CheckSoftLimit(Position)==false)
    {
        ShowMotorLimitError("WAR0154", "Sorting Arm X motor will out of limit", HSys.Mot.MSortingArmX->SoftLimitDetail(Position));
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
    //AI(ht160s-sortarm) 20260624 : two-car gap interlock. This SortArm-initiated move drives
    //the SAME shared-rail Loader-Y car as TLoaderModule::MoveLoaderY, so it must honor the
    //operator-configured safe distance ([Safety] LoaderYSafeDistance) against the opposite car
    //too. Route through the single canonical TLoaderModule check rather than duplicating it.
    //Silent return false (no modal) mirrors the aLoader path : the switch(Task) caller re-polls
    //until the opposite car clears. NULL-guarded so a missing module never freezes the move.
    //The check runs in DUMMY (motors physically move there); it is not runtime-bypassed.
    if(LoaderModule!=NULL && LoaderModule->IsLoaderYMoveSafe(LoaderNo, Position)==false)
        return false;
    if(Motor->CheckSoftLimit(Position)==false)
    {
        ShowMotorLimitError(Motor->AlarmName[eMotOverLimitErr], LangT("Loader Y motor will out of limit"), Motor, Position);
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
        ShowMotorLimitError(Motor->AlarmName[eMotOverLimitErr], LangT("Auto Y motor will out of limit"), Motor, Position);
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
        ShowMotorLimitError(HSys.Mot.MPitchX->AlarmName[eMotOverLimitErr], LangT("Pitch X motor will out of limit"), HSys.Mot.MPitchX, Position);
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
    //AI(ht160s-maintainer) 20260623 : SINGLE-PICK left-edge reach. With a leading site
    //closed, the chosen sucker must overhang the tray's left frame to reach the left-edge
    //columns, so BaseX is allowed to go negative - but only by at most
    //(SORT_ARM_SUCKER_COUNT-1) pitches, the deepest a valid single-pick selection can
    //produce (anchor sucker over column 0). The X motion layer still enforces the real
    //soft travel limit; anything past this bound is bad data and is rejected.
    if(BaseX<-(SORT_ARM_SUCKER_COUNT-1))
        return false;

    XPosition=GetSortArmCellX(GetLoaderSortX(iActiveLoaderNo), BaseX);
    YPosition=GetSortArmCellY(GetLoaderFirstSortY(iActiveLoaderNo), Slot[FirstSlot].PickY);
    bXDone=MoveSortArmX(XPosition);
    bYDone=MoveLoaderY(iActiveLoaderNo, YPosition);
    return (bXDone && bYDone);
}
//---------------------------------------------------------------------------
bool TSortArmModule::MoveToAutoPlace()
{
    int XPosition=GetSortArmCellX(GetAutoSortX(iActiveAutoIndex), iPlaceBaseX);
    int YPosition=GetSortArmCellY(GetAutoFirstSortY(iActiveAutoIndex), iPlaceY);
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

    //AI(ht160s-maintainer) 20260623 : SINGLE-PICK (user requirement: one IC per stroke).
    //Borrowed from HT9045 Find_InArm_Single - raster the WHOLE tray and return the first
    //pickable cell; the run-loop re-calls FindPickCells each cycle to drain the tray one
    //IC at a time. To CLEAR THE TRAY EVEN WITH A LEADING SITE CLOSED, scan from column 0
    //and choose the active sucker by a HT172-style floating anchor: the largest ENABLED
    //sucker whose index <= the cell column (smallest non-negative BaseX). For left-edge
    //columns that no enabled sucker can reach without overhang, fall back to the lowest
    //enabled sucker (FirstEnabled); MoveToLoaderPick then travels left (negative BaseX,
    //leading nozzle overhanging the tray frame) so those edge ICs are still picked.
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

    for(int YIndex=0; YIndex<YCount; YIndex++)
    {
        for(int XIndex=0; XIndex<XCount; XIndex++)
        {
            if(IsPickableData(TrayMotor->Tray.Data[XIndex][YIndex]))
            {
                int PickSlot=FirstEnabled;
                for(int k=0; k<SORT_ARM_SUCKER_COUNT && k<=XIndex; k++)
                {
                    if(GeneralSetting.bSuckerEnabled[k])
                        PickSlot=k;
                }
                Slot[PickSlot].bCanPick=true;
                Slot[PickSlot].PickX=XIndex;
                Slot[PickSlot].PickY=YIndex;
                Slot[PickSlot].TrayData=TrayMotor->Tray.Data[XIndex][YIndex];
                //AI(HT160S-Maintainer) 20260604 : P4 capture 2D-looked-up bin for routing.
                Slot[PickSlot].BinValue=TrayMotor->GetTrayBin(XIndex, YIndex);
                //AI(ht160s-lotbin) 20260615 : carry owning lot + 2D code for By Lot+Bin
                //routing and Production_Log; harmless in Normal mode (unused).
                Slot[PickSlot].LotIndex=TrayMotor->GetTrayLot(XIndex, YIndex);
                Slot[PickSlot].Code2D=TrayMotor->GetTrayCode2D(XIndex, YIndex);
                Slot[PickSlot].bManual2D=TrayMotor->GetTrayManual2D(XIndex, YIndex);
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
    //AI(ht160s-pick-retry) 20260702 : suck failure no longer alarms here with the nozzle
    //pressed on the IC. A failed slot is LATCHED (bPickSuckErr) and parked; once every
    //selected slot has finished (success or latched error) the function reports done and
    //DoPickFromLoader case 50/52/54 lifts Z to safe, auto-retries the same cell
    //(HT172 aSortArm case 300->500 shape) and only then raises the operator alarm.
    bool bAllDone=true;

    if(IsSoftSimulate())
        return true;

    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT; SlotIndex++)
    {
        if(Slot[SlotIndex].bCanPick && bPickSuckErr[SlotIndex]==false)
        {
            TMySucker *Sucker=GetSucker(SlotIndex);
            if(Sucker==NULL)
            {
                bAllDone=false;
            }
            else if(Sucker->Suck()==false)
            {
                if(Sucker->Error)
                {
                    bPickSuckErr[SlotIndex]=true;
                    Sucker->Reset();
                }
                else
                {
                    bAllDone=false;
                }
            }
        }
    }
    return bAllDone;
}
//---------------------------------------------------------------------------
bool TSortArmModule::HasPickSuckError()
{
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
    {
        if(bPickSuckErr[s])
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(ht160s-status) 20260703 : eSortArmStatus with the SAS_RECOVERY overlay derived
//from the live recovery predicates (user decision : recovery is a 4th state).
int TSortArmModule::GetStatus()
{
    if(IsResidueCheckBusy() || HasPickSuckError())
        return SAS_RECOVERY;
    return Status;
}
//---------------------------------------------------------------------------
void TSortArmModule::ClearPickSuckErrors()
{
    //AI(ht160s-pick-retry) 20260702 : arm a fresh suck round on the latched slots. The pick
    //selection (bCanPick/PickX/PickY) is kept so the retry re-approaches the SAME tray cell.
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
    {
        if(bPickSuckErr[s])
        {
            TMySucker *Sucker=GetSucker(s);
            bPickSuckErr[s]=false;
            if(Sucker!=NULL)
            {
                Sucker->Error=false;
                Sucker->Reset();
            }
        }
    }
}
//---------------------------------------------------------------------------
void TSortArmModule::SkipErroredPickCells()
{
    //AI(ht160s-pick-retry) 20260702 : operator chose SKIP. Write each failed cell off as
    //EMPTY_IC (HT172 case 350 K_SKIP marks it NULL_IC): the IC physically stays in the tray
    //but FindPickCells never re-finds it. ClearSlot drops bCanPick so the follow-up
    //TransferPickDataFromLoader ignores the slot; successfully-held slots are untouched.
    TTrayMotor *TrayMotor=GetLoaderVMotor(iActiveLoaderNo);

    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
    {
        if(bPickSuckErr[s])
        {
            if(TrayMotor!=NULL && Slot[s].bCanPick)
                TrayMotor->SetTraySingleData(Slot[s].PickX, Slot[s].PickY, EMPTY_IC);
            ClearSlot(s);
            bPickSuckErr[s]=false;
        }
    }
}
//---------------------------------------------------------------------------
void TSortArmModule::MarkResidueTargets()
{
    //AI(ht160s-residue) 20260624 : remember which slots actually placed an IC this
    //cycle so CheckPlaceResidue re-sucks only those nozzles. Called after
    //DestroySelectedSlots succeeds and BEFORE TransferPlaceDataToAuto (ClearSlot).
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
    {
        if(Slot[s].bPlaceSelected)
        {
            bNeedResidueCheck[s]=true;
            ResidueTask[s]=1;
        }
    }
}
//---------------------------------------------------------------------------
bool TSortArmModule::CheckPlaceResidue()
{
    //AI(ht160s-residue) 20260625 : HT172 CheckSortArmDestroyActive port, non-blocking.
    //Runs every DoSortArm cycle but ONLY after the nozzle lifted to the top (bResidueArmed
    //set in place case 60), so a re-suck never happens near the tray and cannot pull a
    //just-placed IC back. Per armed nozzle: break vacuum, RE-SUCK, wait, read status.
    //ON after re-suck = IC still plugging the nozzle (residue) -> alarm, operator removes
    //IC, retry. OFF = nozzle empty (clear). Real vacuum sensor only; DUMMY/HAS_TRAY/sim skip.
    if(bResidueArmed==false)        //not lifted to top yet : nothing armed to verify
        return true;
    if(IsSoftSimulate())
    {
        for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
            bNeedResidueCheck[s]=false;
        if(iResidueAutoIndex>=0 && AutoModule!=NULL)
        {
            AutoModule->SetPlaceResidueClear(iResidueAutoIndex, true);
            iResidueAutoIndex=-1;
        }
        bResidueArmed=false;
        return true;
    }
    bool bAllDone=true;
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
    {
        if(bNeedResidueCheck[s]==false)
            continue;
        if(HSys.LastSet.iRealDummy!=REALLY)   //no real vacuum sensor off REALLY
        {
            bNeedResidueCheck[s]=false;
            continue;
        }
        TMySucker *Sucker=GetSucker(s);
        if(Sucker==NULL)
        {
            bNeedResidueCheck[s]=false;
            continue;
        }
        switch(ResidueTask[s])
        {
            case 1:
                Sucker->OffDestroy();
                ResidueDelay[s].SetMS(iDestroyCheckMS);
                ResidueDelay[s].On();
                ResidueTask[s]=200;
                bAllDone=false;
                break;
            case 200:
                Sucker->OnSuck();
                ResidueDelay[s].SetMS(iDestroyCheckMS);
                ResidueDelay[s].On();
                ResidueTask[s]=300;
                bAllDone=false;
                break;
            case 300:
                if(ResidueDelay[s].Off())
                {
                    if(Sucker->GetStatus())
                    {
                        ShowSuckError(*Sucker, 2, K_RETRY, "SortArm Residue");
                        ResidueTask[s]=200;
                        bAllDone=false;
                    }
                    else
                    {
                        Sucker->Normal();
                        bNeedResidueCheck[s]=false;
                        ResidueTask[s]=1;
                    }
                }
                else
                    bAllDone=false;
                break;
            default:
                ResidueTask[s]=1;
                break;
        }
    }
    if(bAllDone)
    {
        if(iResidueAutoIndex>=0 && AutoModule!=NULL)   //all placed nozzles clear -> let target Auto leave
        {
            AutoModule->SetPlaceResidueClear(iResidueAutoIndex, true);
            iResidueAutoIndex=-1;
        }
        bResidueArmed=false;
    }
    return bAllDone;
}
//---------------------------------------------------------------------------
bool TSortArmModule::IsResidueCheckBusy()
{
    //AI(ht160s-residue) 20260624 : true while any just-placed nozzle is still being
    //re-suck verified. Pick Z-down gates on this so re-suck never overlaps pick suck.
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
        if(bNeedResidueCheck[s])
            return true;
    return false;
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
                g_DeviceInfo.AddIcIdentity(SlotIndex, sLotID, Slot[SlotIndex].Code2D, Slot[SlotIndex].bManual2D);
                //AI(ht160s-ccd-manual2d) 20260626 : revive TraceCode/ErrorType columns from the CCD
                //scan outcome. Code2D empty = 2D never read (ScanFail 999); code present but no
                //owning lot resolved = NoMap 1000. Normal reads leave trace 0 (columns blank).
                int iTrace2D=0;
                if(Slot[SlotIndex].Code2D=="")
                    iTrace2D=999;
                else if(Slot[SlotIndex].LotIndex<0)
                    iTrace2D=1000;
                g_DeviceInfo.AddTraceInfo(SlotIndex, iTrace2D);
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
            if(IsResidueCheckBusy())                                     //AI(ht160s-residue) 20260624 : prior place re-suck must finish before pick suck
                break;
            if(MovePickZDown())
            {
                StartPnpSettle(dPickDelaySec);                           //AI(ht160s-pnp) 20260626 : let the nozzle settle on the IC before suck
                PickTask=47;
            }
            break;

        case 47:
            if(PnpSettleElapsed())                                       //AI(ht160s-pnp) 20260626 : pick settle dwell done -> suck
                PickTask=50;
            break;

        case 50:
            if(SuckSelectedSlots())
            {
                if(HasPickSuckError())
                {
                    //AI(ht160s-pick-retry) 20260702 : count the failed stroke and lift Z to
                    //safe FIRST - both the silent auto-retry and the operator alarm happen
                    //with the nozzle parked up, never pressed on the IC (HT172 case 500/350).
                    iPickRetryCount++;
                    PickTask=52;
                }
                else
                {
                    TransferPickDataFromLoader();
                    iPickRetryCount=0;
                    PickTask=60;
                }
            }
            break;

        case 52:
            //AI(ht160s-pick-retry) 20260702 : Z up to safe, then a silent auto-retry of the
            //SAME cell while budget lasts ([SortArm] PickRetryCount, default 3 = HT172's
            //hardcoded iRetryCT>3). Re-entering case 40 re-validates Loader-Y ownership and
            //position before Z goes down again. Loader-Y ownership is kept for the retry.
            if(SortArmZToSafePos())
            {
                if(iPickRetryCount<=GeneralSetting.iSortArmPickRetryCount)
                {
                    ClearPickSuckErrors();
                    PickTask=40;
                }
                else
                {
                    PickTask=54;
                }
            }
            break;

        case 54:
        {
            //AI(ht160s-pick-retry) 20260702 : retries exhausted - modal alarm with Z parked
            //at safe. RETRY grants a fresh retry round on the same cell (HT172 rezeroes
            //iRetryCT); SKIP writes the failed cells off and carries on with whatever the
            //other nozzles picked. The operator key is finally honored (it used to be
            //discarded, making SKIP behave like RETRY).
            int iKey=K_RETRY;

            for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
            {
                if(bPickSuckErr[s])
                {
                    TMySucker *Sucker=GetSucker(s);
                    if(Sucker!=NULL)
                        iKey=ShowSuckError(*Sucker, 1, K_RETRY|K_SKIP, "SortArm Pick");
                    break;
                }
            }
            iPickRetryCount=0;
            if(iKey==K_SKIP)
            {
                SkipErroredPickCells();
                TransferPickDataFromLoader();
                PickTask=60;
            }
            else
            {
                ClearPickSuckErrors();
                PickTask=40;
            }
            break;
        }

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
                ClearPickSuckErrors();   //AI(ht160s-pick-retry) 20260702 : drop latches with the abandoned selection
                iPickRetryCount=0;
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

    int ExpectX=GetSortArmCellX(GetAutoSortX(iActiveAutoIndex), iPlaceBaseX);
    int ExpectY=GetSortArmCellY(GetAutoFirstSortY(iActiveAutoIndex), iPlaceY);
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
            {
                StartPnpSettle(dPlaceDelaySec);                          //AI(ht160s-pnp) 20260626 : settle at the place position before releasing the IC
                PlaceTask=45;
            }
            break;

        case 45:
            if(PnpSettleElapsed())                                       //AI(ht160s-pnp) 20260626 : place settle dwell done -> release
                PlaceTask=50;
            break;

        case 50:
            if(DestroySelectedSlots())
            {
                MarkResidueTargets();                                    //AI(ht160s-residue) 20260624 : tag placed slots before ClearSlot
                iResidueAutoIndex=iActiveAutoIndex;       //AI(ht160s-residue) 20260624 : report to this Auto when bg check ends
                if(AutoModule!=NULL)
                    AutoModule->SetPlaceResidueClear(iActiveAutoIndex, false);
                //AI(ht160s-pnp) 20260626 : SAFETY (Task 1) - Destroy() turns blow OFF the instant it
                //returns (OffDelayTime default 0). Capture the placed nozzles (BEFORE TransferPlaceDataToAuto
                //clears bPlaceSelected), RE-ASSERT blow ON, dwell in case55 BEFORE the Z-up, and keep blow ON
                //through the lift (off only in case70) so the seal is fully broken before the IC is lifted.
                for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
                    bBlowSlot[s]=Slot[s].bPlaceSelected;
                if(IsSoftSimulate()==false)
                {
                    for(int s2=0; s2<SORT_ARM_SUCKER_COUNT; s2++)
                    {
                        if(bBlowSlot[s2])
                        {
                            TMySucker *Sucker=GetSucker(s2);
                            if(Sucker!=NULL)
                                Sucker->OnDestroy();                     //blow ON (positive pressure); vacuum already OFF in Destroy()
                        }
                    }
                    BlowDwell.Clear();
                    BlowDwell.SetMS(GetDestroyCheckMS());
                    BlowDwell.On();
                }
                TransferPlaceDataToAuto();
                PlaceTask=55;
            }
            break;

        case 55:
            //AI(ht160s-pnp) 20260626 : blow-on dwell. Vacuum is off, blow is on; wait so positive
            //pressure fully breaks the IC-nozzle seal BEFORE the lift. Sim has no real air -> skip.
            if(IsSoftSimulate()==false && BlowDwell.Off()==false)
                break;
            PlaceTask=60;
            break;

        case 60:
            if(SortArmZToSafePos())                                      //AI(ht160s-residue) 20260625 : nozzle reached top -> ARM residue check (never re-suck near tray); verify runs in background
                PlaceTask=70;
            break;

        case 70:
            {
                //AI(ht160s-pnp) 20260626 : lift has cleared the place position. Only now turn blow OFF on
                //the placed nozzles, then ARM the background residue check exactly as before and finish.
                //(bResidueArmed moved here from old case60 so arming still happens on cycle completion.)
                if(IsSoftSimulate()==false)
                {
                    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
                    {
                        if(bBlowSlot[s])
                        {
                            TMySucker *Sucker=GetSucker(s);
                            if(Sucker!=NULL)
                                Sucker->OffDestroy();                    //blow OFF after the IC has cleared
                        }
                    }
                }
                for(int s2=0; s2<SORT_ARM_SUCKER_COUNT; s2++)
                    bBlowSlot[s2]=false;
                bResidueArmed=true;                                      //AI(ht160s-residue) 20260625 : nozzle at top -> arm residue check (bg verify)
                PlaceTask=1;
                return true;
            }

        default:
            PlaceTask=1;
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TSortArmModule::IsCleanOutFinish()
{
    //AI(cleanout) 20260703 : live-computed (was: return the one-shot bCleanOutFinish latch set in
    //DoSortArm case 1, which could read stale-true if a late Loader IC or a background residue
    //re-suck re-committed the arm AFTER the latch was set). During CleanOut, SortArm is finished
    //only when Loader (both L+R) is fully cleaned, it holds no IC, both sub-machines are parked,
    //and no residue/pick-error recovery is pending. Outside CleanOut the flag is returned as-is.
    if(HSys.Sys.RunMode!=Run_CleanOut)
        return bCleanOutFinish;
    if(LoaderModule==NULL || LoaderModule->IsAllCleanOutFinish()==false)
        return false;
    if(HasHoldingIC())
        return false;
    if(PickTask!=1 || PlaceTask!=1)
        return false;
    if(IsResidueCheckBusy() || HasPickSuckError())
        return false;
    if(Status!=SAS_IDLE)
        return false;   //AI(ht160s-status) 20260703 : status busy belt beside the cursor checks
    //AI(cleanout) 20260703 : user-confirmed cascade condition - SortArm idle INCLUDES every
    //enabled suck-Z nozzle parked UP on its Home sensor (live read; sim-true). A nozzle left
    //down is not idle even with all sub-machines parked.
    if(AreAllSuckersHome()==false)
        return false;
    return true;
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

    //AI(ht160s-pick-retry) 20260703 : surface the suck-retry state so a hang parked at
    //PickTask=52 (auto-retry) or 54 (modal) shows WHICH slot latched and HOW MANY retries
    //were burned against the budget - PickTask alone cannot explain the stall offline.
    {
        AnsiString errSlots;
        for(int e=0; e<SORT_ARM_SUCKER_COUNT; e++)
            if(bPickSuckErr[e])
                errSlots += (errSlots.IsEmpty() ? AnsiString("") : AnsiString(",")) + IntToStr(e);
        s += "PickRetry=" + IntToStr(iPickRetryCount)
           + " / " + IntToStr(GeneralSetting.iSortArmPickRetryCount)
           + "  PickSuckErrSlots=" + (errSlots.IsEmpty() ? AnsiString("none") : errSlots) + "\r\n";
    }

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

    CheckPlaceResidue();   //AI(ht160s-residue) 20260624 : background re-suck verify (HT172 non-blocking); pick/discharge gate on result

    switch(Task)
    {
        case 1:
            Status=SAS_IDLE;   //AI(ht160s-status) 20260703 : decision hub; re-committed below when there is work
            if(HasHoldingIC())
            {
                Status=SAS_PLACING;
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

            //AI(ht160s-sortarm) 20260625 : STICKY side-commit (operator rule : NEVER abandon a
            //half-picked Loader tray). A tray needing multiple pick batches drops SortArm back to
            //idle (Task=1) between batches; the old code RE-SELECTED via GetSortingLoaderNo every
            //idle cycle and GetSortingLoaderNo always prefers side 1, so with both sides READY_SORT
            //it would switch to side 1 mid-tray, park the abandoned loaded car in the sort zone, and
            //deadlock the rail (PickTask stuck at 30; State Record 2026-06-25 22_29_43). Fix : while
            //the LAST/CURRENT active side still has pickable ICs, KEEP that same side and do NOT call
            //GetSortingLoaderNo. HasPickableIC reads ONLY the carriage tray grid (not State->Status),
            //so it stays true across the per-batch ReleaseSortOwner LS_ToRear->LS_READY_SORT transient.
            //Only once the active side's tray is fully drained (no pickable IC -> it discharges and
            //leaves the rail) do we pick a new side. Net : one tray fully sorted at a time; the
            //non-active car is never left parked mid-sort-zone (SortArm only moves a side into the
            //sort zone while committed to picking it), so no explicit non-active "hold out of sort
            //zone" guard is needed - SortArm is the sole mover of a Loader-Y into the sort zone, and
            //IsLoaderYMoveSafe remains the per-move collision backstop.
            if(iActiveLoaderNo>0 && LoaderModule->HasPickableIC(iActiveLoaderNo))
            {
                //keep the committed side (re-acquired inside DoPickFromLoader::AcquireSortOwner)
            }
            else
            {
                iActiveLoaderNo=LoaderModule->GetSortingLoaderNo();
            }
            if(iActiveLoaderNo>0)
            {
                Status=SAS_PICKING;   //AI(ht160s-status) 20260703
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
                Status=SAS_PLACING;   //AI(ht160s-status) 20260703 : batch picked, go place
                PlaceTask=1;
                Task=200;
            }
            break;

        case 200:
            if(HasHoldingIC()==false && PlaceTask<=1)   // no holding IC and place is idle : nothing to place, leave case 200
            {
                Status=SAS_IDLE;   //AI(ht160s-status) 20260703
                Task=1;
                break;
            }
            if(DoPlaceToAuto(1))
            {
                if(HasHoldingIC())
                    Task=200;
                else
                {
                    Status=SAS_IDLE;   //AI(ht160s-status) 20260703 : all held ICs placed
                    Task=1;
                }
            }
            break;

        default:
            LogLadderFault("SortArm.DoSortArm", Task);   //AI(ht160s-ladder-guard) 20260703
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
    //AI(ht160s-maintainer) 20260626 : reach check must match GetSortArmCellX comb offset
    //(Col-SlotIndex+iBaseSuckX). Datum sucker = suck2 (iBaseSuckX=1) reaches Col 0,
    //e.g. suck2 -> Cell(1,1); legacy (Col-SlotIndex)<0 wrongly blocked it.
    if((Col-SlotIndex+iBaseSuckX)<0)
    {
        Err="Sucker cannot reach this column";
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
    XPosition=GetSortArmCellX(BaseSortX, Col-SlotIndex);
    YPosition=GetSortArmCellY(FirstSortY, Row);
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
        return true;   //AI(ht160s-ladder-guard) 20260703 : bad args, abort (was dead Task=900)
    }
    if(Target==1 || Target==2)
        LoaderNo=Target;
    else if(Target>=11 && Target<=16)
        AutoIndex=Target-11;
    else
    {
        return true;   //AI(ht160s-ladder-guard) 20260703 : bad args, abort (was dead Task=900)
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
            XPosition=GetSortArmCellX(BaseSortX, Col-SlotIndex);
            YPosition=GetSortArmCellY(FirstSortY, Row);
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
            LogLadderFault("SortArm.MoveSuckerToCell", Task);   //AI(ht160s-ladder-guard) 20260703 (was dead Task=900)
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