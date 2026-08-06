#include "IncludeAllHeader.h"       //Dell ±N.h²Î¤@,¥i¥[³tbuild
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
#include "cSoterOutput.h"
#include "aColor.h"
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
static const int FALLDOWN_LOST_MS=100;   //AI(ht160s-falldown) 20260706 : held-IC vacuum-loss confirm window (ms); reject a single noisy read (mirrors SUCK_HOME_LOST_MS)
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
//resume edges, alongside HSys.CynPtr[]/SortArmSuck.
void TSortArmModule::PauseTimeoutTimers()
{
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
        ResidueDelay[s].Pause();
    //AI(ht160s-prepick) 20260806 : re-arm the pre-pick wait on the pause edge. User ruling:
    //"machine paused or alarmed -> the count must restart, so it cannot cry wolf". csystem
    //calls this on the SystemStart FALLING edge, and every Note alarm drops SystemStart, so
    //this single hook covers both pause and alarm. This is the same defect the State Record
    //stuck watchdog had - it measured wall clock across a pause and fired on resume.
    dwPrePickWaitStart=0;
}
//---------------------------------------------------------------------------
void TSortArmModule::ReStartTimeoutTimers()
{
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
        ResidueDelay[s].ReStart();
    //AI(ht160s-prepick) 20260806 : and again on the resume edge, so the operator always gets a
    //FULL fresh window of actual running before MES1921 can fire.
    dwPrePickWaitStart=0;
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
    dwHoldLostStart=0;   //AI(ht160s-falldown) 20260706 : reset held-IC vacuum-loss debounce
    dwZDownGuardStart=0;   //AI(bcb6-172align) 20260723 : fresh SuckZ down-move guard on home/init
    bMoveAborted=false;   //AI(ht160s-sortarm) 20260703 : no pending in-flight move abort on home/init
    iPickRetryCount=0;   //AI(ht160s-pick-retry) 20260702 : fresh pick-retry budget on home/init
    //AI(ht160s-prepick) 20260806 : fresh pre-pick wait budget + no stale published target on
    //home/init. bPrePickBypassOnce is a one-shot operator escape and must never survive a HOME.
    dwPrePickWaitStart=0;
    iPrePickWantedAuto=-1;
    bPrePickBypassOnce=false;
    //AI(ht160s-home-resume-w4) 20260711 : keep-material HOME preserves an UNFINISHED
    //place-residue verify (SR-2). The Auto-side bResidueClear=false gate now also
    //survives a keep-material HOME (aAuto1To6 InitialFlag), so wiping the pending list
    //here would leave that gate closed forever with no owner to re-open it. Keep the
    //pending set + report target, restart each ladder at 1, and RE-ARM : uHome case 100
    //homes the SuckZ axes to the top, which satisfies the "never re-suck near a tray"
    //arming precondition, so the background verify re-runs and reports the verdict.
    bool bKeepResidue=false;
    if(bKeepMaterial)
    {
        for(int r=0; r<SORT_ARM_SUCKER_COUNT; r++)
        {
            if(bNeedResidueCheck[r])
                bKeepResidue=true;
        }
    }
    if(bKeepResidue==false)
    {
        iResidueAutoIndex=-1;   //AI(ht160s-residue) 20260624 : no pending residue report
        bResidueArmed=false;   //AI(ht160s-residue) 20260625 : disarm; armed at place case 60
    }
    else
        bResidueArmed=true;    //AI(ht160s-home-resume-w4) : re-arm the surviving verify (SuckZ at top after case 100)
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)   //AI(ht160s-residue) 20260624 : reset residue-check state on home/init
    {
        if(bKeepResidue==false)
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
            //AI(ht160s-home-resume-w4) 20260711 : PickX/PickY are KEPT for a held IC
            //(SP-2) -- the falldown monitor's bAtPick source-cell restore needs the
            //origin cell to write a dropped IC back to the source grid after a resumed
            //home; zeroing them degraded every post-home drop to SKIP-only.
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
    if(bKeepMaterial)
    {
        int iKeptIC=0;
        for(int k=0; k<SORT_ARM_SUCKER_COUNT; k++)
        {
            if(Slot[k].bHasIC)
                iKeptIC++;
        }
        RecordProcess("HOME-RESUME SortArm: kept heldIC="+IntToStr(iKeptIC)+
            " residuePending="+IntToStr(bKeepResidue?1:0)+" reArmed="+IntToStr(bResidueArmed?1:0));   //AI(ht160s-obsv-p0)
    }
    UpdateKitSuckState();
}
//---------------------------------------------------------------------------
//AI(ht160s-home-resume-drain) 20260711 : SP-1 vacuum reconciliation (owner D2). A HOME
//landing in the suck stroke (vacuum ON, bHasIC not yet committed) leaves an ownerless
//IC on the nozzle : InitialFlag's ClearSlot never touches the vacuum output, so the IC
//rides up invisible and the next pick crushes it. While the Z is STILL DOWN in the
//pocket (drain runs before any motor homing) cut the vacuum AND blow (D2 : once vacuum
//is established only positive pressure releases the IC - cutting vacuum alone leaves
//it stuck on the nozzle), so the IC settles back into its source cell (grid data was
//never transferred -> it is simply re-picked on resume). The blow stays ON through the
//SuckZ homing and is stopped at the uHome case-100 completion (Z back at safe, D2).
//Idempotent : once the vacuum output is off the slot no longer matches. Real tier only.
static bool s_bHomeDrainBlow[SORT_ARM_SUCKER_COUNT]={false,false,false,false};
//---------------------------------------------------------------------------
bool TSortArmModule::HomeDrainTick()
{
#ifndef SOFT_SIMULATE
    if(HSys.LastSet.iRealDummy!=DUMMY)
    {
        for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
        {
            TMySucker *Sucker=GetSucker(s);
            if(Sucker==NULL)
                continue;
            if(Sucker->GetOnBit() && Slot[s].bHasIC==false)
            {
                Sucker->Off();
                Sucker->OnDestroy();
                s_bHomeDrainBlow[s]=true;
                RecordProcess("Home: drain vacuum-reconcile sucker "+IntToStr(s+1)+" (ownerless IC released into its cell)");
            }
        }
    }
#endif
    return true;
}
//---------------------------------------------------------------------------
void TSortArmModule::HomeDrainBlowOff()
{
#ifndef SOFT_SIMULATE
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
    {
        if(s_bHomeDrainBlow[s]==false)
            continue;
        TMySucker *Sucker=GetSucker(s);
        if(Sucker!=NULL)
            Sucker->OffDestroy();
        s_bHomeDrainBlow[s]=false;
    }
#endif
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
    Slot[SlotIndex].PassClass=0;       //AI(ht160s-lotpassfail) 20260709 : clear frozen PASS/FAIL class
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
void TSortArmModule::AbortCurrentMove()
{
    //AI(ht160s-sortarm) 20260703 : mark the in-flight MoveSuckerToCell to bail. Set by the teach
    //StopSortArmTest (incl. the central fault guard / EMG). MoveSuckerToCell case 30 checks this
    //AFTER the X move, so a fault modal raised inside MoveSortArmX does not fall through to the Y
    //move once the operator dismisses it. Cleared at case 0 (fresh run) and InitialFlag.
    bMoveAborted=true;
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
bool TSortArmModule::GuardSuckZDown(bool bPick)
{
    //AI(bcb6-172align) 20260723 : open-loop SuckZ (MC88X1, no encoder) down-move safety, ported
    //from HT172 aSortArm.cpp:643. A pick/place Z-down that never reports done (card MotionDone
    //latched busy, lost steps, or a stuck/failed Home sensor) used to hang PickTask=45 / PlaceTask=40
    //forever and deadlock the whole machine (only the 300s watchdog noticed). Give the down-move a
    //bounded window (GeneralSetting.iSortArmZMoveGuardMs); if it is still not done AND a commanded-down
    //nozzle is STILL on its Home sensor (never left the top), stop all motion and raise an operator
    //alarm instead of a silent hang. Time-based debounce like MoveSortArmX. Returns true on fault.
    //AI(sortarm-vacuum) 20260724 : SCOPE - guards "Z-down never reports done" (axis stuck / never
    //reaches teach Z). Does NOT cover "nozzle reached Z but vacuum will not seal" - that is the
    //pick-suck path (case 50 -> SUC0011) and was the 20260723 on-machine event; this guard
    //correctly did NOT fire there (case 45 had already passed). Originally named after the wrong
    //root cause; kept as a distinct, real safety net for the Z-not-done failure.
#ifdef SOFT_SIMULATE
    (void)bPick;
    return false;   //no card in sim : MoveTo completes instantly so this is never reached
#else
    int GuardMs=GeneralSetting.iSortArmZMoveGuardMs;
    if(GuardMs<1000)
        GuardMs=1000;
    if(dwZDownGuardStart==0)
    {
        dwZDownGuardStart=GetTickCount();
        return false;
    }
    if((int)(GetTickCount()-dwZDownGuardStart)<GuardMs)
        return false;
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
    {
        bool bCommandedDown;
        if(bPick)
            bCommandedDown=Slot[s].bCanPick;
        else
            bCommandedDown=Slot[s].bPlaceSelected;
        if(bCommandedDown==false)
            continue;
        TTrayMotor *Motor=GetSuckZMotor(s);
        if(Motor==NULL || Motor->GetEnable()==false)
            continue;
        Motor->ScanMotorStatus();
        if(Motor->Led[iHomeLed])
        {
            dwZDownGuardStart=0;
            HSys.StopAllMotor();
            ShowSystemError("SortArm SuckZ "+IntToStr(s+1)+" down but Home sensor still ON : step loss / card busy / Home sensor fail. Re-home the suckers.", K_RETRY);
            return true;
        }
    }
    dwZDownGuardStart=0;
    HSys.StopAllMotor();
    ShowSystemError("SortArm SuckZ down did not complete within timeout. Check the suck-Z axis and MC88X1 card.", K_RETRY);
    return true;
#endif
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
                Slot[PickSlot].PassClass=TrayMotor->GetTrayPassClass(XIndex, YIndex);   //AI(ht160s-lotpassfail) 20260709
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
int TSortArmModule::GetMappedAutoIndex(int BinData, int LotIndex, int PassClass, bool &bFixedArea)
{
    int Area;

    bFixedArea=false;
    //AI(ht160s-lotbin) 20260615 : By Lot+Bin mode. The (Lot,Bin) pair was bound to
    //an Auto at CCD scan time (LotBinBinding.ResolveAuto), so here we only READ the
    //binding - no allocation side-effect during the per-slot/per-Auto place scan.
    //Error bins (2D fail / no bin setting) and ICs with no owning lot route to the
    //Error Auto. Color is not used for sorting in this mode (AMR identity tray only).
    if(GeneralSetting.IsLotBinSortMode())
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
    //AI(ht160s-lotpassfail) 20260709 : By Lot+PassFail mode. The PASS/FAIL class was
    //frozen at CCD scan (Slot.PassClass, passed in via the PassClass arg) and bound to an
    //Auto by LotBinBinding.ResolveAuto; here we only READ it. PassClass 0 (error bin) and
    //ICs with no owning lot route to the Error Auto - same as Lot+Bin.
    if(GeneralSetting.IsLotPassFailSortMode())
    {
        int AutoIndex;
        bFixedArea=true;
        if(LotIndex>=0 && PassClass>0)
        {
            AutoIndex=LotBinBinding.FindAuto(LotIndex, PassClass);
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

    MappedAutoIndex=GetMappedAutoIndex(GetSlotRoutingBin(SlotIndex), Slot[SlotIndex].LotIndex, Slot[SlotIndex].PassClass, bFixedArea);
    if(MappedAutoIndex>=0)
        return (MappedAutoIndex==AutoIndex);
    return (bFixedArea==false);
}
//---------------------------------------------------------------------------
int TSortArmModule::GetHeldTargetAutos(int *OutAutoList, int MaxCount)
{
    //AI(ht160s-predictive-supply) 20260707 : list the Auto indices the currently-held ICs
    //are routed to, in the SAME order SelectPlaceAuto serves them (sucker 0..3, fixed-route
    //ICs only). Lets TrayArm feed the empty Auto SortArm needs next FIRST, so a held IC does
    //not stall place. Pure read-only : reuses GetMappedAutoIndex/GetSlotRoutingBin, no
    //AutoModule callback, never changes routing. May list one Auto twice (two suckers, one
    //Auto); the caller takes the first entry whose Auto currently wants a tray. Returns count.
    int Count=0;

    if(OutAutoList==NULL || MaxCount<=0)
        return 0;

    for(int SlotIndex=0; SlotIndex<SORT_ARM_SUCKER_COUNT && Count<MaxCount; SlotIndex++)
    {
        if(Slot[SlotIndex].bHasIC==false)
            continue;
        bool bFixedArea=false;
        int AutoIndex=GetMappedAutoIndex(GetSlotRoutingBin(SlotIndex), Slot[SlotIndex].LotIndex, Slot[SlotIndex].PassClass, bFixedArea);
        if(AutoIndex>=0)
            OutAutoList[Count++]=AutoIndex;
    }
    return Count;
}
//---------------------------------------------------------------------------
//AI(ht160s-prepick) 20260806 : "can this Auto physically accept one more IC RIGHT NOW ?"
//This is the user's "Auto ?€?‰ç›¤" test, and it is deliberately NOT FindPlaceCells : that one
//has side effects (ClearPlaceSelection, iPlaceBaseX/iPlaceY) and gates every slot on
//bHasIC - which is false before the suck, so it would always answer "no" at pick time.
//Three conditions, all required :
//  1. IsReadyForSortArmPlace  - in AMR mode an identity/cover working tray takes no IC
//  2. TrayMotor->fHasTray     - the working car really has a tray  <- the note-7 rule
//  3. at least one EMPTY_IC cell left in that tray
bool TSortArmModule::IsAutoReadyToReceive(int AutoIndex)
{
    TTrayMotor *TrayMotor;
    int XCount, YCount;

    if(AutoIndex<0 || AutoIndex>=SORT_ARM_AUTO_COUNT)
        return false;
    if(AutoModule!=NULL && AutoModule->IsReadyForSortArmPlace(AutoIndex)==false)
        return false;
    TrayMotor=GetAutoVMotor(AutoIndex);
    if(TrayMotor==NULL || TrayMotor->fHasTray==false)
        return false;
    XCount=GetTrayXCount();
    YCount=GetTrayYCount();
    for(int Y=0; Y<YCount; Y++)
    {
        for(int X=0; X<XCount; X++)
        {
            if(TrayMotor->Tray.Data[X][Y]==EMPTY_IC)
                return true;
        }
    }
    return false;   //tray full
}
//---------------------------------------------------------------------------
//AI(ht160s-prepick) 20260806 : where will the cell FindPickCells just selected go ?
//The destination is fully knowable BEFORE the suck : FindPickCells already stamps BinValue /
//LotIndex / PassClass / TrayData into the slot, which is everything GetMappedAutoIndex needs,
//and the (Lot,Bin) binding was made back at Top-CCD scan time. Reads bCanPick (the pre-suck
//selection flag), NOT bHasIC.
//Returns the Auto index, or -1 = free routing (any Auto with room), or -2 = nothing SortArm
//can place (routed to Color, or no cell selected).
int TSortArmModule::GetSelectedPickTargetAuto()
{
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
    {
        bool bFixedArea=false;
        int AutoIndex;

        if(Slot[s].bCanPick==false)
            continue;
        AutoIndex=GetMappedAutoIndex(GetSlotRoutingBin(s), Slot[s].LotIndex, Slot[s].PassClass, bFixedArea);
        if(AutoIndex>=0)
            return AutoIndex;
        if(bFixedArea)
            return -2;   //fixed route that is not an Auto (Color / unmapped) - not ours to gate
        return -1;       //free routing
    }
    return -2;           //nothing selected
}
//---------------------------------------------------------------------------
//AI(ht160s-prepick) 20260806 : must the pick be HELD ? Also latches iPrePickWantedAuto so the
//Auto feeder can serve the blocking station first (without that the gate never opens and the
//line livelocks - see FindTrayRequestAuto).
//Run-mode policy is load-bearing : in CleanOut / OneCycle the machine must DRAIN. Holding the
//pick there would leave pickable ICs in the Loader for ever, IsAllCleanOutFinish would never go
//true and CleanOut would hang - exactly the shape of the 2026-08-05 Color hang. So the gate is
//Normal-production only; CleanOut keeps the old pick-then-wait behaviour.
bool TSortArmModule::IsPrePickBlocked()
{
    int Target;

    iPrePickWantedAuto=-1;
    if(GeneralSetting.iSortArmPrePickAutoWaitSec<0)
        return false;                       //gate disabled by config
    if(HSys.Sys.RunMode!=Run_Normal)
        return false;                       //CleanOut / OneCycle must drain
    if(bPrePickBypassOnce)
    {
        bPrePickBypassOnce=false;           //operator SKIPped at MES1921 : let this one through
        return false;
    }
    Target=GetSelectedPickTargetAuto();
    if(Target==-2)
        return false;                       //not an Auto-routed IC; leave the old path alone
    if(Target==-1)
    {
        //free routing (no BinAreaMap fixed area) : any Auto with room will do
        for(int i=0; i<SORT_ARM_AUTO_COUNT; i++)
        {
            if(IsAutoReadyToReceive(i))
                return false;
        }
        return true;
    }
    if(IsAutoReadyToReceive(Target))
        return false;
    iPrePickWantedAuto=Target;
    return true;
}
//---------------------------------------------------------------------------
int TSortArmModule::GetPrePickWantedAuto()
{
    return iPrePickWantedAuto;
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
            int AutoIndex=GetMappedAutoIndex(GetSlotRoutingBin(SlotIndex), Slot[SlotIndex].LotIndex, Slot[SlotIndex].PassClass, bFixedArea);
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
void TSortArmModule::RecordAutoSkippedCells()
{
    //AI(ht160s-autoskip) 20260714 : one Production_Log "Reject" line (Which Auto="Reject",
    //Error log="AutoSkip") per pick-vacuum errored cell the AutoSkipOnPickFail opt-in writes
    //off, plus a running tRunData.iAutoSkipCount and an EventLog PROCESS audit line. MUST run
    //BEFORE SkipErroredPickCells: that helper ClearSlot()s each errored nozzle (bCanPick=0,
    //PickX/PickY/Lot/Code2D wiped), and the normal TransferPickDataFromLoader->AddInputInfo
    //path (the only setter of SICRecord.bActive) is gated on bCanPick, so a skipped slot would
    //never open a record and SaveRejectRecord would early-out. Open the record here while the
    //FindPickCells identity is still live, then flush an immediate reject (the cell is never placed).
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
    {
        if(bPickSuckErr[s]==false)
            continue;

        g_DeviceInfo.AddInputInfo(s, Slot[s].PickY, Slot[s].PickX, "");

        AnsiString sLotID="";
        TLotRunInfo *Lot=LotRegistry.GetLot(Slot[s].LotIndex);
        if(Lot!=NULL)
            sLotID=Lot->sLotID;
        g_DeviceInfo.AddIcIdentity(s, sLotID, Slot[s].Code2D, Slot[s].bManual2D);

        int iTrace2D=0;
        if(Slot[s].Code2D=="")
            iTrace2D=999;
        else if(Slot[s].LotIndex<0)
            iTrace2D = GeneralSetting.IsWhiteListSortMode() ? 1005 : 1000;   //AI(ht160s-whitelist) 20260716 : code read OK but no owning lot -> WhiteList = not-in-list reject (1005), else generic NoMap (1000)
        g_DeviceInfo.AddTraceInfo(s, iTrace2D);

        g_DeviceInfo.SaveRejectRecord(s, "AutoSkip");
        //AI(ht160s-soter) 20260714 : emit a Soter reject row for a genuine-2D die
        //written off on pick-vacuum failure. OpenRow is mandatory here : the pick-time
        //OpenRow is bypassed for errored cells, so open then commit the row now.
        {
            TLotIcInfo SoterIc;
            if(Slot[s].Code2D!="" && LotRegistry.FindIcInfo(Slot[s].Code2D, SoterIc))
            {
                AnsiString sSoterLoad="";
                if(ColorModule!=NULL && ColorModule->IsTrayID2DGenuine())
                    sSoterLoad=ColorModule->GetTrayID();
                g_SoterOutput.OpenRow(s, SoterIc.sCustLotID, (Lot!=NULL ? Lot->sLotID : AnsiString("")), SoterIc.sProductCode, SoterIc.sSubstage,
                    Slot[s].Code2D, sSoterLoad,
                    SoterIc.sRetestCode, SoterIc.iHBin, SoterIc.iSBin, SoterIc.sDiePass);
                g_SoterOutput.CommitRejectRow(s);
            }
        }

        tRunData.iAutoSkipCount++;
        RecordProcess("SortArm auto-skip cell (Loader"+IntToStr(iActiveLoaderNo)
            +" X="+IntToStr(Slot[s].PickX)+" Y="+IntToStr(Slot[s].PickY)
            +") total="+IntToStr(tRunData.iAutoSkipCount));
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
            RecordProcess("SortArm residue verify: slot "+IntToStr(s)+" skipped (non-REALLY tier)");   //AI(ht160s-obsv-p2)
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
                    if(Sucker->GetStatusAnyOn())
                    {
                        ShowSuckError(*Sucker, 2, K_RETRY, "SortArm Residue");
                        ResidueTask[s]=200;
                        bAllDone=false;
                    }
                    else
                    {
                        Sucker->Normal();
                        RecordProcess("SortArm residue verify: slot "+IntToStr(s)+" CLEAR");   //AI(ht160s-obsv-p0)
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
            //AI(secs-onsite0731) 20260801 : KYEC on-site note 7 "Load and total number is
            //zero". tRunData.LoaderIC was declared, cleared and persisted but NEVER
            //incremented anywhere in the tree, so the main-screen "Load" panel and the
            //host's RPTID 501 slot 1 (SVID 1101 Loader Count) were both starved. This is
            //the structural twin of HT172 MySortArmParameter::AddLoadingCount (HT172
            //aSortArm.cpp:2351), whose DFM panels were ported here but whose two
            //assignments and this increment were not. Counted once per IC actually lifted
            //off a Loader tray: bCanPick is dropped by SkipErroredPickCells/ClearSlot for
            //every failed slot, and the three callers of this function are mutually
            //exclusive, so a retry cannot double-count.
            tRunData.LoaderIC++;
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
                    iTrace2D = GeneralSetting.IsWhiteListSortMode() ? 1005 : 1000;   //AI(ht160s-whitelist) 20260716 : code read OK but no owning lot -> WhiteList = not-in-list reject (1005), else generic NoMap (1000)
                g_DeviceInfo.AddTraceInfo(SlotIndex, iTrace2D);
                //AI(ht160s-soter) 20260714 : open a Soter per-die output row at pick, only for
                //a die with a genuine 2D identity resolved in the 2D map. Snapshot the fields
                //now so a lot unload cannot lose them before the single LotEnd flush.
                {
                    TLotIcInfo SoterIc;
                    if(Slot[SlotIndex].Code2D!="" && LotRegistry.FindIcInfo(Slot[SlotIndex].Code2D, SoterIc))
                    {
                        AnsiString sSoterLoad="";
                        if(ColorModule!=NULL && ColorModule->IsTrayID2DGenuine())
                            sSoterLoad=ColorModule->GetTrayID();
                        g_SoterOutput.OpenRow(SlotIndex, SoterIc.sCustLotID, (Lot!=NULL ? Lot->sLotID : AnsiString("")), SoterIc.sProductCode, SoterIc.sSubstage,
                            Slot[SlotIndex].Code2D, sSoterLoad,
                            SoterIc.sRetestCode, SoterIc.iHBin, SoterIc.iSBin, SoterIc.sDiePass);
                    }
                }
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
            //AI(ht160s-lot-reset) 20260706 : count each placed OK IC as one processed
            //unit so tRunData.TotalIC (no SVID since 66020 was retired 20260804) and the derived UPH are
            //non-zero (previously TotalIC had NO increment site anywhere -> read 0).
            tRunData.TotalIC++;
            //AI(secs-1102-placepoint) 20260805 : SVID 1102 Output Total Count. Customer ruling -
            // align to HT9045 / HT172: the total only increments once the nozzle has actually
            // PLACED the IC into the Auto area. It used to be bumped in aLoader at CCD-scan time
            // inside the 2D reverse-lookup HIT branch, which meant (a) an IC whose 2D could not be
            // read, or was not in any lot, was still physically sorted into the Error Auto but never
            // counted, and (b) a lot with no 2D table at all (LotRegistry empty) left 1102 pinned at
            // 0 while the machine was visibly producing. Counting here also makes 1102 exactly the
            // sum of 1103-1105 / 1259-1261, because TrayICCnt[] is bumped on the next line from the
            // same event.
            MachineRun.iTotalSorted++;
            //AI(ht160s-motion-view) 20260618 : per-Auto output IC count for the Unload
            //palAutoXXCnt display (HT172 ShowBinCount used tRunData.TrayICCnt). eAuto1=index 1.
            if(iActiveAutoIndex>=0 && iActiveAutoIndex<SORT_ARM_AUTO_COUNT)
                tRunData.TrayICCnt[iActiveAutoIndex+1]++;
            g_DeviceInfo.AddBinInfo(SlotIndex, iActiveAutoIndex, Slot[SlotIndex].TrayData);
            {   //AI(ht160s-bin-passfail) 20260708 : per-IC PASS/FAIL from the customer DiePass.
                //AI(ht160s-lotpassfail) 20260709 : read the class FROZEN at CCD scan (Slot.PassClass)
                //so the logged result matches the class the IC was actually routed on; 0 -> blank.
                int PassClassVal=Slot[SlotIndex].PassClass;
                AnsiString PassFailText="";
                //AI(bcb6-ternary) 20260723 : nested ?: yielding AnsiString miscompiles in BCB6 (crash); use if/else
                if(PassClassVal==1)
                    PassFailText=AnsiString("PASS");
                else if(PassClassVal==2)
                    PassFailText=AnsiString("FAIL");
                g_DeviceInfo.AddPassFail(SlotIndex, PassFailText);
            }
            //AI(ht160s-lotpassfail) 20260709 : By Lot+PassFail overflow trace. When every
            //non-Error Auto is taken, ResolveAuto binds a PASS/FAIL bucket to the Error Auto,
            //so a valid (PassClass>0) product physically lands there mixed with 2D read-fail
            //ICs. The customer accepts the overflow; record it in Production_Log (TraceCode
            //1004 = PFOverflow) with NO operator Note. Non-overflow products keep their own
            //Auto and stay trace 0. Only class>0 can hit this (scan-fail ICs are PassClass 0).
            if(GeneralSetting.IsLotPassFailSortMode() && Slot[SlotIndex].PassClass>0)
            {
                int ErrArea=BinAreaMap.GetErrorBinArea();
                int ErrAuto=(ErrArea>=eHT160BinAreaAuto1 && ErrArea<=eHT160BinAreaAuto6)?(ErrArea-eHT160BinAreaAuto1):(SORT_ARM_AUTO_COUNT-1);
                if(iActiveAutoIndex==ErrAuto)
                    g_DeviceInfo.AddTraceInfo(SlotIndex, 1004);
            }
            g_DeviceInfo.AddOutputInfo(SlotIndex, "Auto"+IntToStr(iActiveAutoIndex+1), "", Slot[SlotIndex].PlaceY, Slot[SlotIndex].PlaceX);
            //AI(ht160s-soter) 20260714 : complete + buffer this placed IC row. col9 Unload
            //Cover Tray = the Auto flow-lane identity tray 2D. Must run BEFORE ClearSlot
            //below (which wipes the slot); the pending Soter row is keyed by nozzle.
            {
                AnsiString sSoterUnload="";
                if(AutoModule!=NULL)
                    sSoterUnload=AutoModule->GetWorkingTrayID(iActiveAutoIndex);
                g_SoterOutput.CommitPlaceRow(SlotIndex, sSoterUnload);
            }
            ClearSlot(SlotIndex);
        }
    }
    UpdateKitSuckState();
}
//---------------------------------------------------------------------------
//AI(ht160s-falldown) 20260706 : HT172 OutArmDeviceDropCheck / CheckIsFallDown port.
//HT160S verified sucker vacuum ONLY once (at suck, SuckSelectedSlots case 50) and never
//again, so an IC that dropped after a "successful" suck went unnoticed : no alarm, no SKIP.
//Mirrors HT172 : while the arm carries picked ICs from the Loader to the Auto (post-suck,
//Z at/going to safe, BEFORE the place Z-down), re-read each holding nozzle's LIVE vacuum. A
//holding nozzle (bHasIC) whose vacuum reads OFF has dropped its IC. bAtPick decides what to
//do with the Loader source cell (already marked EMPTY_IC at pick by TransferPickDataFromLoader):
//  bAtPick==true  (DoPickFromLoader case 60, still parked at the Loader) : the IC fell back
//                 at the Loader tray -> RESTORE the source cell to its original data so DoLoader
//                 case 3000 sees ActiveTrayAllData(EMPTY_IC)==false and keeps the side
//                 LS_READY_SORT instead of discharging the tray to the rear WITH the IC (the
//                 exact reported symptom). ReleaseSortOwner is NOT called (caller breaks), so
//                 the discharge is blocked while the alarm is up as well.
//  bAtPick==false (DoPlaceToAuto transit) : the IC was lifted clear of the Loader and is lost
//                 in the travel path; the Loader cell is legitimately empty -> NOT restored.
//Either way : real decel-stop ALL motion, write the SortArm slot off (never a phantom placed
//OK / counted), and alarm with the dropped cell identity. A single OFF read is NOT trusted : it
//must persist FALLDOWN_LOST_MS (time-window debounce, same idiom as the suck-home interlock) so
//scan-rate sensor jitter cannot false-stop the machine. REALLY-only (sim/DUMMY have no real
//vacuum); the Sensor.Enable gate skips an uninstalled point.
bool TSortArmModule::CheckHoldFallDown(bool bAtPick)
{
    int iFirstDrop=-1;
    int iDropRow=0;
    int iDropCol=0;
    AnsiString sDrop2D="";
    bool bAnyOff=false;

    if(HSys.LastSet.iRealDummy!=REALLY)
        return false;
    //AI(ht160s-simfalldrop) 20260713 : a SOFT_SIMULATE laptop build can still run with
    //iRealDummy==REALLY (to exercise real fault paths / SIM 2D injection), so the REALLY
    //guard above does NOT stop this on a laptop. There is no real vacuum sensor there, so
    //the held-nozzle re-read below reads OFF and false-fires SUC0013 "Suck2 Sucker Error"
    //(SortArm IC Dropped In Transit). IsSoftSimulate() is compile-time true under
    //SOFT_SIMULATE; the real build keeps full detection.
    if(IsSoftSimulate())
        return false;

    //any holding nozzle currently reading vacuum-OFF ? (raw scan)
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
    {
        if(Slot[s].bHasIC==false)
            continue;
        TMySucker *Sucker=GetSucker(s);
        if(Sucker==NULL || Sucker->Enable==false || Sucker->Sensor.Enable==false)
            continue;
        if(Sucker->GetStatusAllOn()==false)
        {
            bAnyOff=true;
            break;
        }
    }

    //time-window debounce (mirror the suck-home interlock) : a single bad read is rejected; a
    //real drop holds the sensor OFF continuously, so it survives the FALLDOWN_LOST_MS window.
    if(bAnyOff==false)
    {
        dwHoldLostStart=0;
        return false;
    }
    if(dwHoldLostStart==0)
    {
        dwHoldLostStart=GetTickCount();
        return false;
    }
    if((int)(GetTickCount()-dwHoldLostStart)<FALLDOWN_LOST_MS)
        return false;
    dwHoldLostStart=0;

    //confirmed drop. Identity scan (first still-OFF held nozzle) for the breadcrumb only; do
    //NOT mutate yet so the modal can offer a real choice before the Loader source cell is set.
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
    {
        if(Slot[s].bHasIC==false)
            continue;
        TMySucker *Sucker=GetSucker(s);
        if(Sucker==NULL || Sucker->Enable==false || Sucker->Sensor.Enable==false)
            continue;
        if(Sucker->GetStatusAllOn()==false)
        {
            iFirstDrop=s;
            iDropRow=Slot[s].PickY;
            iDropCol=Slot[s].PickX;
            sDrop2D=Slot[s].Code2D;
            break;
        }
    }

    if(iFirstDrop<0)
        return false;   //every OFF nozzle recovered within the window : rejected glitch

    //AI(jamrate) 20260805 : the AddJamCount used to sit HERE, before the alarm. It has moved
    //BELOW the ShowSuckError call - user ruling: a jam is counted ONLY when SUC0013 is really
    //raised, no other situation. Counting here also counted the DropSucker==NULL path, where
    //no alarm is shown at all.
    HSys.StopAllMotor();   //MC88X1 : real decel-stop ALL (DecStopAllMotor is a no-op here)

    //breadcrumb : dropped cell tray Row/Col (+2D) into the alarm region so it is shown + logged
    //(HT160S alarms persist to the EventLog), mirroring HT172 RecordProcess.
    AnsiString Part="SortArm IC Dropped ";
    Part+=(bAtPick?"At Pick R":"In Transit R")+IntToStr(iDropRow)+" C"+IntToStr(iDropCol);
    if(sDrop2D!="")
        Part+=" 2D="+sDrop2D;

    //Honor the operator key (do NOT ignore it). At a PICK drop the IC fell at the Loader, so the
    //operator gets a real choice : K_RETRY keeps the IC in the tray (restore the source cell ->
    //re-pick, never discharge with it); K_SKIP writes it EMPTY_IC (abandon; the tray may then
    //discharge - the operator's call, mirrors the pick-retry SkipErroredPickCells + HT172
    //SkipError(NULL_IC)). A TRANSIT drop has no recoverable source (IC lost in the travel path),
    //so it is SKIP-only : acknowledge the loss.
    int iKey=K_SKIP;
    TMySucker *DropSucker=GetSucker(iFirstDrop);
    if(DropSucker!=NULL)
    {
        //AI(jamrate) 20260805 : ShowSuckError CodeType 3 is what raises SUC0013 "Suck2 Sucker
        //Error | SortArm IC Dropped ...". Count the jam HERE and nowhere else : the customer's
        //definition is "the vacuum was established, Z rose, the vacuum was gone, a Note alarm
        //fired -> that is jam+1, nothing else counts". Placed BEFORE the modal blocks so the
        //figure is already committed if the operator walks away from the dialog, and inside the
        //DropSucker!=NULL branch so a nozzle we cannot alarm on is not counted either.
        AddJamCount(bAtPick?AnsiString("SortArm IC dropped at pick"):AnsiString("SortArm IC dropped in transit"));
        iKey=ShowSuckError(*DropSucker, 3, bAtPick?(K_RETRY|K_SKIP):K_SKIP, Part);   //CodeType 3 = drop (pick=1, place/residue=2)
    }

    //dispose every still-OFF held nozzle per the choice, then write the SortArm slot off
    TTrayMotor *LoaderTray=(bAtPick?GetLoaderVMotor(iActiveLoaderNo):NULL);
    for(int s2=0; s2<SORT_ARM_SUCKER_COUNT; s2++)
    {
        if(Slot[s2].bHasIC==false)
            continue;
        TMySucker *Sk=GetSucker(s2);
        if(Sk==NULL || Sk->Enable==false || Sk->Sensor.Enable==false)
            continue;
        if(Sk->GetStatusAllOn()==false)
        {
            if(bAtPick && LoaderTray!=NULL)   //RETRY: restore (keep IC in tray) ; SKIP: abandon (EMPTY_IC)
                LoaderTray->SetTraySingleData(Slot[s2].PickX, Slot[s2].PickY, (iKey==K_SKIP)?EMPTY_IC:Slot[s2].TrayData);
            Sk->Normal();     //drop the now-meaningless vacuum/blow outputs on this nozzle
            ClearSlot(s2);    //write the SortArm slot off : not held, not place-selected, never counted
            //AI(ht160s-soter) 20260714 : drop the leaked Soter pending row for this abandoned
            //nozzle. A fall-down is not recorded in Production_Log, so emit no Soter row either;
            //this also stops a later non-genuine IC on this nozzle being mis-committed as placed.
            g_SoterOutput.DiscardRow(s2);
        }
    }
    UpdateKitSuckState();
    return true;
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
            dwZDownGuardStart=0;   //AI(bcb6-172align) 20260723 : fresh pick cycle -> reset SuckZ down-move guard
            //AI(ht160s-residue) 20260727 : F5 orphan-flag guard. A fresh pick begins only
            //when the place machine is idle, so a bNeedResidueCheck bit still set while
            //bResidueArmed==false is orphaned (place interrupted between case50 and case70).
            //CheckPlaceResidue's armed==false early-out never clears it, so it would latch
            //IsResidueCheckBusy() true and deadlock the pick Z-down gate at case45 (empty tray
            //included). Clear it here, GUARDED by bResidueArmed==false so a LIVE background
            //verify is never abandoned, and reopen the target Auto discharge gate place case50 shut.
            if(bResidueArmed==false)
            {
                for(int rs=0; rs<SORT_ARM_SUCKER_COUNT; rs++)
                {
                    bNeedResidueCheck[rs]=false;
                    ResidueTask[rs]=1;
                }
                if(iResidueAutoIndex>=0 && AutoModule!=NULL)
                {
                    AutoModule->SetPlaceResidueClear(iResidueAutoIndex, true);
                    iResidueAutoIndex=-1;
                }
            }
            if(FindPickCells(iActiveLoaderNo)==false)
            {
                dwPrePickWaitStart=0;   //AI(ht160s-prepick) 20260806 : nothing to pick -> not waiting
                iPrePickWantedAuto=-1;
                PickTask=1;
                return true;
            }
            //AI(ht160s-prepick) 20260806 : THE PRE-PICK GATE (on-site note 7 : "need check auto
            //has tray, then sortarm pick up ic"). Never suck an IC we cannot immediately place.
            //Before this, SortArm committed to the IC and only THEN called SelectPlaceAuto - and
            //that has no timeout, no alarm and no fallback for a fixed-route IC (its second loop
            //re-tests every Auto, but CanPlaceSlotToAuto answers false for all of them once the
            //route is fixed). A destination Auto with no tray therefore left the arm holding the
            //IC at PlaceTask=1 for ever, with the Loader tray pinned behind it.
            //POSITION IS LOAD-BEARING : this sits AFTER FindPickCells (the target is only
            //knowable once a cell is selected) and BEFORE AcquireSortOwner. Gating after the
            //acquire would make SortArm wait while HOLDING the shared Loader-Y rail, blocking the
            //Loader and TrayArm - i.e. trading one stall for a worse one.
            if(IsPrePickBlocked())
            {
                if(dwPrePickWaitStart==0)
                {
                    dwPrePickWaitStart=GetTickCount();
                }
                else if(GeneralSetting.iSortArmPrePickAutoWaitSec>0 &&
                        (int)(GetTickCount()-dwPrePickWaitStart) >
                            GeneralSetting.iSortArmPrePickAutoWaitSec*1000)
                {
                    AnsiString WaitMsg;
                    int RetWait;

                    if(iPrePickWantedAuto>=0)
                        WaitMsg.sprintf("SortArm is waiting for Auto%d to receive a tray before it may pick", iPrePickWantedAuto+1);
                    else
                        WaitMsg="SortArm is waiting : no Auto can receive an IC before it may pick";
                    RecordProcess("PREPICK "+WaitMsg+" (held >"+
                        IntToStr(GeneralSetting.iSortArmPrePickAutoWaitSec)+"s)");
                    dwPrePickWaitStart=0;   //re-arm a FULL window before this can alarm again
                    RetWait=ShowMyError("MES1921", WaitMsg, K_RETRY|K_SKIP);
                    if(RetWait==K_SKIP)
                    {
                        //escape hatch : pick anyway this once (pre-20260806 behaviour). The IC
                        //will be held until a home appears - which is the operator's call.
                        bPrePickBypassOnce=true;
                        RecordProcess("PREPICK operator SKIP : one pick allowed through the gate");
                    }
                }
                ClearPickSelection();
                PickTask=1;
                return true;   //back to idle WITHOUT taking the rail : Loader/TrayArm stay free
            }
            dwPrePickWaitStart=0;
            iPrePickWantedAuto=-1;
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
            //AI(ht160s-clampgrip) 20260806 : BOUNDARY CONFIRM before the irreversible pick
            //Z-down (on-site notes 1/9, 2026-08-05 : a tray suddenly jumped out of a Loader-Y
            //carriage clamp). The user's rule : the PushTray cylinder must be ON for "has tray"
            //to mean anything. Without this the software still read fHasTray=1 / HasOK=1 and the
            //nozzle would have driven down onto an empty carriage.
            //Placed HERE rather than as a continuous poll across the Y move : between clamp and
            //release there are legitimate windows where the reed changes (the feed ladder runs
            //its own clamp confirm), and a naive poll would false-fire in them. This is the one
            //moment where "is a tray really under me" must hold, and the answer is cheap.
            //Verdict -1 (sim / point disabled / clamp retracted) proceeds unchanged : a point we
            //cannot read is never evidence that a tray is missing.
            if(LoaderModule->GetCarriageGripVerdict(iActiveLoaderNo)==0)
            {
                int RetGrip;
                RecordProcess("SortArm pick BLOCKED : Loader"+IntToStr(iActiveLoaderNo)+
                    " carriage PushTray grip reed dark with tray data present - tray lost off the carriage");
                RetGrip=ShowMyError("JAM0913", LangT("Loader Tray Lost On Carriage"),
                                    LoaderModule->GetCarriagePushOnSensor(iActiveLoaderNo), true,
                                    K_RETRY|K_TRAY_END);
                if(RetGrip==K_TRAY_END)
                {
                    //operator confirms the tray is gone : wipe this side's remaining tray data so
                    //HasPickableIC goes false and the Loader discharges instead of offering the
                    //side again (the same wipe the pick-fail K_TRAY_END uses at case 54).
                    LoaderModule->ChangeActiveTrayData(iActiveLoaderNo, HAS_OK_IC, EMPTY_IC);
                    LoaderModule->ChangeActiveTrayData(iActiveLoaderNo, UNCHECK_IC, EMPTY_IC);
                    ClearPickSelection();
                    PickTask=70;
                }
                break;   //K_RETRY : hold at case 40 and re-check; never Z-down on this verdict
            }
            PickTask=45;
            break;

        case 45:
            if(IsResidueCheckBusy())                                     //AI(ht160s-residue) 20260624 : prior place re-suck must finish before pick suck
                break;
            if(MovePickZDown())
            {
                dwZDownGuardStart=0;                                     //AI(bcb6-172align) 20260723 : Z-down done, disarm guard
                StartPnpSettle(dPickDelaySec);                           //AI(ht160s-pnp) 20260626 : let the nozzle settle on the IC before suck
                PickTask=47;
            }
            else
                GuardSuckZDown(true);                                    //AI(bcb6-172align) 20260723 : bounded Home cross-check; alarm not silent deadlock
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
                    //AI(ht160s-pick-retry) 20260702 : lift Z to safe FIRST - both the silent
                    //auto-retry and the operator alarm happen with the nozzle parked up,
                    //never pressed on the IC (HT172 case 500/350).
                    //AI(jamrate) 20260805 : the AddJamCount that used to sit HERE is REMOVED
                    //(on-site note 3 "suck or read no ic is not jam type", user ruling: a jam
                    //is ONLY a vacuum that was ESTABLISHED and then LOST, and it always raises
                    //a Note alarm). This branch is a failed suck STROKE - the vacuum never
                    //built at all, which most often just means the cell had no IC in it - and
                    //it is followed by a SILENT auto-retry (case 52) with no operator-visible
                    //alarm at all. On the 2026-08-05 shift it inflated the figure from 4 real
                    //jams to 15 : EventLog JAM #2,3,4,6,7,8,9,10,11,13 were all this one line
                    //and not one of them carried an alarm row. The count now lives at its ONE
                    //remaining source, CheckHeldICFallDown, which raises SUC0013 before it
                    //counts. iPickRetryCount / the case-54 recovery modal are unchanged.
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

            //AI(ht160s-autoskip) 20260714 : opt-in [SortArm] AutoSkipOnPickFail. ON -> write the
            //cell off automatically with no operator modal (log + count each skip); OFF -> the
            //operator recovery modal decides RETRY / SKIP / TRAY_END (unchanged behaviour).
            if(GeneralSetting.bSortArmAutoSkipOnPickFail)
            {
                iKey=K_SKIP;
            }
            else
            {
                for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
                {
                    if(bPickSuckErr[s])
                    {
                        TMySucker *Sucker=GetSucker(s);
                        if(Sucker!=NULL)
                            iKey=ShowSuckError(*Sucker, 1, K_RETRY|K_SKIP|K_TRAY_END, "SortArm Pick");
                        break;
                    }
                }
            }
            iPickRetryCount=0;
            if(iKey==K_SKIP)
            {
                if(GeneralSetting.bSortArmAutoSkipOnPickFail)
                    RecordAutoSkippedCells();   //AI(ht160s-autoskip) 20260714 : reject-log + count, auto path only (before ClearSlot wipe)
                SkipErroredPickCells();
                TransferPickDataFromLoader();
                PickTask=60;
            }
            else if(iKey==K_TRAY_END)
            {
                //AI(ht160s-ktrayend) 20260709 : operator declares THIS Loader tray done from the
                //pick-vacuum error. A suck miss does NOT prove the cell is empty (nozzle wear,
                //vacuum build fail or wrong pick height read the same), so ending the tray is the
                //operator's call, not automatic. Place the ICs already held on the nozzles (case 60),
                //write the failed cells off, then wipe the WHOLE remaining tray - both CCD-passed
                //HAS_OK_IC and un-scanned UNCHECK_IC - to EMPTY_IC so HasPickableIC/FindNextCcdCell
                //go false and the Loader ladder discharges the emptied tray to rear (HT172 K_TRAY_END
                //= InitNewTray(NULL_IC) + still place the held ICs). Held ICs live on Slot[].bHasIC,
                //not the tray grid, so clearing Data leaves them untouched.
                SkipErroredPickCells();
                TransferPickDataFromLoader();
                LoaderModule->ChangeActiveTrayData(iActiveLoaderNo, HAS_OK_IC, EMPTY_IC);
                LoaderModule->ChangeActiveTrayData(iActiveLoaderNo, UNCHECK_IC, EMPTY_IC);
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
                //AI(ht160s-falldown) 20260706 : confirm the just-picked ICs survived the lift to
                //safe before handing back the Loader-Y / declaring the batch picked. A drop here
                //stops+alarms+writes the slot off (HT172 OutArmDeviceDropCheck at the transit start).
                if(CheckHoldFallDown(true))
                    break;
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

    //AI(ht160s-falldown) 20260706 : HT172 OutArmDeviceDropCheck port -- continuous held-IC
    //vacuum drop monitor across the transit to Auto (Z at/going to safe, BEFORE the place
    //Z-down at case 40). NOT gated during Z-down/destroy/blow (>=40) where vacuum legitimately
    //changes. A drop stops ALL motion + alarms + writes the slot off (never a phantom placed OK).
    if(PlaceTask>=10 && PlaceTask<=35)
    {
        if(CheckHoldFallDown(false))
            return false;
    }

    switch(PlaceTask)
    {
        case 1:
            dwZDownGuardStart=0;   //AI(bcb6-172align) 20260723 : fresh place cycle -> reset SuckZ down-move guard
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
            if(MoveToAutoPlace()==false)
                break;
            //AI(ht160s-clampgrip) 20260806 : BOUNDARY CONFIRM before the irreversible place
            //Z-down - the Auto half of the same on-site note ("need check auto has tray, then
            //sortarm pick up ic"). Proof that the destination car really still grips its working
            //tray, taken at the last moment before the IC is released.
            //HELD, not dropped : the nozzle still carries the IC, so the only safe action is to
            //stay here and re-check. Deliberately does NOT clear the Auto software state the way
            //the Color diaper does - an Auto working tray holds already-placed ICs, so wiping its
            //grid would destroy the placed-IC record. The operator decides.
            if(AutoModule!=NULL && AutoModule->GetCarTrayGripVerdict(iActiveAutoIndex)==0)
            {
                AnsiString ErrGrip;
                ErrGrip.sprintf("Auto%d Push Tray Miss - working tray lost from the car before place", iActiveAutoIndex+1);
                RecordProcess("SortArm place BLOCKED : "+ErrGrip);
                ShowMyError(AnsiString().sprintf("JAM%d02", 11+iActiveAutoIndex), ErrGrip,
                            AutoModule->GetCarTrayPushOnSensor(iActiveAutoIndex), true, K_RETRY);
                break;   //hold at case 30 : never release an IC into a car with no tray
            }
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
                dwZDownGuardStart=0;                                     //AI(bcb6-172align) 20260723 : Z-down done, disarm guard
                StartPnpSettle(dPlaceDelaySec);                          //AI(ht160s-pnp) 20260626 : settle at the place position before releasing the IC
                PlaceTask=45;
            }
            else
                GuardSuckZDown(false);                                   //AI(bcb6-172align) 20260723 : bounded Home cross-check; alarm not silent deadlock
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
//AI 20260721 : re-arm the one-shot OneCycle finish latch WITHOUT the old
//InitialAllTask full per-module reset. OneCycle finish now freezes all modules
//(pause-like) and clears only this latch, so a later OneCycle press is not a
//stale-true instant no-op. Mirrors HT172 (clears its OneCycle latch, no reset).
//bOneCycleFinish is set only in DoSortArm case 1 (OneCycle, idle, no held IC).
void TSortArmModule::ClearOneCycleFinish()
{
    bOneCycleFinish=false;
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

    //AI(ht160s-prepick) 20260806 : pre-pick gate visibility. Without this a machine that is
    //quietly refusing to pick (because the destination Auto has no tray) looks identical to an
    //idle one in the dump - PickTask=1, no held IC, nothing obviously wrong.
    {
        int WaitMs=0;
        if(dwPrePickWaitStart!=0)
            WaitMs=(int)(GetTickCount()-dwPrePickWaitStart);
        s += "PrePickWantedAuto=" + IntToStr(iPrePickWantedAuto)
           + "  PrePickWaitMs=" + IntToStr(WaitMs)
           + "  PrePickBudgetSec=" + IntToStr(GeneralSetting.iSortArmPrePickAutoWaitSec)
           + "  PrePickBypassOnce=" + IntToStr(bPrePickBypassOnce ? 1 : 0) + "\r\n";
    }

    //AI(ht160s-obsv-p0) 20260720 : residue-verify chain visibility. The Auto discharge gate
    //waits on bResidueClear; without these fields a post-resume "station full forever" hang
    //shows no blocker in the dump.
    {
        AnsiString resSlots;
        for(int r=0; r<SORT_ARM_SUCKER_COUNT; r++)
            if(bNeedResidueCheck[r])
                resSlots += (resSlots.IsEmpty() ? AnsiString("") : AnsiString(",")) + IntToStr(r);
        s += "ResiduePending=" + (resSlots.IsEmpty() ? AnsiString("none") : resSlots)
           + "  ResidueArmed=" + IntToStr(bResidueArmed ? 1 : 0)
           + "  ResidueAutoIndex=" + IntToStr(iResidueAutoIndex) + "\r\n";
    }

    for(int i=0; i<SORT_ARM_SUCKER_COUNT; i++)
    {
        s += "  Slot" + IntToStr(i) + ": hasIC=" + IntToStr(Slot[i].bHasIC ? 1 : 0);
        if(Slot[i].bHasIC)
        {
            int  RouteBin = GetSlotRoutingBin(i);
            bool bFixed   = false;
            int  Mapped   = GetMappedAutoIndex(RouteBin, Slot[i].LotIndex, Slot[i].PassClass, bFixed);
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
            bMoveAborted=false;   //AI(ht160s-sortarm) 20260703 : fresh run - clear any stale abort from a prior stopped test
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
            if(bMoveAborted)   //AI(ht160s-sortarm) 20260703 : a fault modal was raised inside MoveSortArmX and the timer guard stopped the test mid-modal; bail BEFORE the Y move (no stray motion after the operator acknowledges the alarm)
                return false;
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