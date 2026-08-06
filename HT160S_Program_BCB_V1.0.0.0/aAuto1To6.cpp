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
#include "SecsGem\uAgvStation.h"   //AI(ht160s-agv) : AgvCoord (AMR full-collect handshake + DeviceCount)
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
//AI(ht160s-actuator-timer) 20260627 : actuator-timer enrollment hook. Auto has no wall-
//clock timeout window to freeze on a machine pause since the AMR full-wait (AmrFullWaitTimer[])
//was removed -- the AGV full-collect handshake is governed by the coordinator's own free-
//running watchdog (iAgvTimeoutSec). Kept as an enrollment stub so csystem Pause/ReStart-
//ActuatorTimeoutTimers need no change and a future Auto timeout timer has a home.
void TAutoModule::PauseTimeoutTimers()
{
}
//---------------------------------------------------------------------------
void TAutoModule::ReStartTimeoutTimers()
{
}
//---------------------------------------------------------------------------
void TAutoModule::InitialFlag(bool bKeepMaterial)
{
    TTrayMotor *TrayMotor=NULL;

    //AI(auto-per-station) 20260802 : per-station ladder cursors reset together.
    for(int ResetIndex=0; ResetIndex<AUTO_STATION_COUNT; ResetIndex++)
    {
        StationTask[ResetIndex]=0;   //AI(auto-per-station) 20260802 : no ladder in flight
        FeedTask[ResetIndex]=1;
        DischargeTask[ResetIndex]=1;
        DischargeSubTask[ResetIndex]=1;
        FeedDelay[ResetIndex].Clear();
        DischargeDelay[ResetIndex].Clear();
    }
    CleanOutTask=1;
    iFeedAuto=-1;
    iDischargeAuto=-1;
    CleanOutDelay.Clear();
    TestUpTask=1;
    TestDelay.Clear();

    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
    {
        TrayMotor=GetAutoVMotor(Index);
        State[Index].bCarHasTray=(TrayMotor!=NULL && TrayMotor->fHasTray);
        State[Index].Status=(State[Index].bCarHasTray ? AS_SORTING : AS_IDLE);   //AI(ht160s-status) 20260703 : re-derive from held material (bKeepMaterial-safe)
        State[Index].bCleanOutFinish=false;
        bCleanOutCheck[Index]=false;
        bCleanOutResidualLogged[Index]=false;   //AI(cleanout) 20260706 : new episode, allow one residual log again
        bAmrLocked[Index]=false;   //AI(ht160s-agv) 20260615 : drop any AGV handoff lock on home/init
        //AI(HT160S-Maintainer) 20260612 : on a recoverable home keep the car stack + its
        //tray roles/2D identity so the Auto does not forget what it is holding. Only the
        //sensor-backed presence above and the cleanout transient flags are refreshed.
        if(bKeepMaterial)
            continue;
        //AI(ht160s-home-resume-w4) 20260711 : moved BELOW the keep-material early-out
        //(was unconditional, SR-1/AD-4) : a keep-material HOME must NOT silently PASS an
        //unfinished place-residue verify -- the gate survives and the SortArm side keeps
        //its pending list + re-arms, so the verify re-runs and reports the real verdict.
        //A full wipe (cold init) still opens the gate as before.
        State[Index].bResidueClear=true;   //AI(ht160s-residue) 20260624 : clear place-residue gate on cold init
        State[Index].bRearHasTray=false;
        State[Index].bRearCanUse=false;
        State[Index].bFrontHasTray=false;
        State[Index].bFullIC=false;
        bRearDeliveredPending[Index]=false;  //AI(general) 20260608 : Stage0 latch reset
        bDischargeTailPending[Index]=false;   //AI(ht160s-home-resume-drain) 20260713 : AD-1 latch cleared on COLD init only (below the keep-material early-out, so a recoverable HOME preserves an in-flight eject tail)
        RearKind[Index]=eTrayKindNormal;     //AI(HT160S-Maintainer) 20260605 : AMR reset
        WorkingKind[Index]=eTrayKindNormal;  //AI(HT160S-Maintainer) 20260605 : AMR reset
        RearTrayID[Index]="";                //AI(HT160S-Maintainer) 20260608 : AMR 2D TrayID reset
        WorkingTrayID[Index]="";             //AI(HT160S-Maintainer) 20260608 : AMR 2D TrayID reset
        RearGrid[Index].Clear();             //AI(ht160s-tray-source) : cleared rear => cleared staged grid (sensor re-latch must not feed a stale grid)
        //AI(HT160S-Maintainer) 20260604 : reset stacking-car data + roles
        Car[Index].Clear();
        InitAutoCarStack(Index);
    }
    if(bKeepMaterial)
    {
        AnsiString sKeep;
        for(int k=0; k<AUTO_STATION_COUNT; k++)
        {
            if(State[k].bCarHasTray || bRearDeliveredPending[k] || bDischargeTailPending[k])
                sKeep+=" A"+IntToStr(k+1)+"(car="+IntToStr(State[k].bCarHasTray?1:0)+
                    ",rearPend="+IntToStr(bRearDeliveredPending[k]?1:0)+
                    ",tail="+IntToStr(bDischargeTailPending[k]?1:0)+")";
        }
        if(sKeep=="")
            sKeep=" none";
        RecordProcess("HOME-RESUME Auto: kept"+sKeep);   //AI(ht160s-obsv-p0)
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-home-resume-drain) 20260711 : W2 drain hook (AF-2). A HOME landing in the
//FeedTask 6000/7000 window leaves a tray physically clamped on the car with fHasTray
//still false (software-blind -> resume JAM + Skip ghost tray). Case 7000 is a verified
//single-scan pure-data commit, so stand-in execute it (spec-sanctioned Task write; case
//6000 itself contains MoveAutoY and is never pumped). FeedTask=1 afterwards - the
//commit is NOT idempotent (car ledger increments), it must run exactly once.
bool TAutoModule::HomeDrainTick()
{
    //AI(auto-per-station) 20260802 : scan EVERY station, not just the one the serial lap
    //happened to pick. In per-station mode up to six feed commits and six eject tails can
    //be outstanding when HOME lands; the single-cursor version would have finished one and
    //silently abandoned the rest, leaving committed trays software-blind.
    bool bAllFeedCommitted=true;
    for(int DrainIndex=0; DrainIndex<AUTO_STATION_COUNT; DrainIndex++)
    {
        if(FeedTask[DrainIndex]!=6000 && FeedTask[DrainIndex]!=7000)
            continue;
        FeedTask[DrainIndex]=7000;
        if(DoFeedTray(DrainIndex, 1))
        {
            FeedTask[DrainIndex]=1;
            StationTask[DrainIndex]=0;
        }
        else
            bAllFeedCommitted=false;
    }
    if(bAllFeedCommitted==false)
        return false;
    //AI(ht160s-home-resume-drain) 20260713 : AD-1 discharge-tail latch. The eject tail
    //(DischargeTask 5000-6100 = MoveAutoY-to-feed + FrontRise) is a MOTOR phase a drain
    //cannot pump; the working tray was committed at DoDischargeTray case 1000 and the
    //clamps released at 3000/4000, so the tray sits free at the output with no live
    //discharge candidate (FindDischargeAuto needs bFullIC, cleared at 1000). Latch the
    //station; DoAuto case 3000 finishes the eject on resume (latch survives keep-material).
    //AI(auto-per-station) 20260802 : latch EVERY station sitting in the eject tail.
    for(int TailIndex=0; TailIndex<AUTO_STATION_COUNT; TailIndex++)
    {
        if(DischargeTask[TailIndex]!=5000 && DischargeTask[TailIndex]!=6000 &&
           DischargeTask[TailIndex]!=6100)
            continue;
        if(bDischargeTailPending[TailIndex]==false)
            RecordProcess("HOME-DRAIN Auto: discharge-tail latched (Auto"+IntToStr(TailIndex+1)+
                " DischargeTask="+IntToStr(DischargeTask[TailIndex])+")");   //AI(ht160s-obsv-p0) : latch edge only (drain ticks every scan)
        bDischargeTailPending[TailIndex]=true;
    }
    //AI(ht160s-home-resume-drain) 20260713 : AD-2 FrontRise convergence. uHome homes only
    //MAutoY and InitialFlag only resets the DischargeSubTask cursor, so a riser left On by
    //a HOME in the discharge / cleanout FrontRise dwell stays extended and collides on the
    //next GoUp index. Force every station's FrontRise Off and HOLD the drain open (return
    //false) until the Off reed confirms, so the risers are down BEFORE MAutoY homes. M2 :
    //lowering a static riser is safe; an air-pressure-alarm HOME skips the whole drain (D3)
    //so a coil is only commanded when air is trusted. Lowering is NOT gated by the D1 AMR
    //freeze (which freezes destacker RISE / separate only).
    bool bAllRiserOff=true;
    for(int i=0; i<AUTO_STATION_COUNT; i++)
    {
        TMyCylinder *Rise=GetFrontRise(i);
        if(Rise==NULL)
            continue;
        Rise->Off();
        if(IsCylinderOffReady(Rise, IsSoftSimulate())==false)
            bAllRiserOff=false;
    }
    if(bAllRiserOff==false)
        return false;
    return true;
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
        ShowMotorLimitError(Motor->AlarmName[eMotOverLimitErr], LangT("Auto Y motor will out of limit"), Motor, Position);
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
        {
            State[Index].bFullIC=true;
            State[Index].Status=AS_FULL;   //AI(ht160s-status) 20260703 : working tray full, wants discharge
        }
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
//AI(auto-per-station) 20260802 : the same test FindFeedAuto applies, WITHOUT its side
//effect. FindFeedAuto REWRITES State[*].bRearCanUse for all six stations, so it must not
//be used to re-validate one station mid-ladder. The caller is responsible for having
//refreshed state this pass (DoAuto case 1000 does, via FindFeedAuto).
bool TAutoModule::IsFeedEligible(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return false;
    if(State[Index].bCarHasTray)
        return false;
    return (State[Index].bRearHasTray && bRearDeliveredPending[Index]);
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
//AI(auto-per-station) 20260802 : FindDischargeAuto's three gates, asked of ONE station.
bool TAutoModule::IsDischargeEligible(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return false;
    if(bAmrLocked[Index])
        return false;
    return (State[Index].bFullIC && State[Index].bResidueClear);
}
//---------------------------------------------------------------------------
//AI(auto-per-station) 20260802 : THE per-station engine. Each of the six output stations
//owns its feed/discharge ladder and steps it once per call, so Auto2 no longer waits for
//Auto1 to finish - the stated requirement. Safe because the ladders are now parameterised
//by station index (nothing reads a shared cursor) and the six Y axes are mechanically
//independent: Mot_Table M06-M11 give MAutoY_1..6 their own MC88X1 axes with no shared rail
//and no ownership token, and DoAllAutoCleanOut has always commanded all six in one scan.
//
//Concurrency caps how many ladders may be IN FLIGHT at once. A station already running is
//never suspended - the cap only refuses to START a new one - so no ladder can be stranded
//mid-stroke by lowering the value. Returns true when every station is idle.
bool TAutoModule::ServiceStations(bool bNoNewJobs)
{
    int Index;
    int iInFlight=0;
    for(Index=0; Index<AUTO_STATION_COUNT; Index++)
        if(StationTask[Index]!=0)
            iInFlight++;
    bool bAllIdle=(iInFlight==0);

    int iCap=GeneralSetting.iAutoConcurrency;
    if(iCap>AUTO_STATION_COUNT)
        iCap=AUTO_STATION_COUNT;

    for(Index=0; Index<AUTO_STATION_COUNT; Index++)
    {
        switch(StationTask[Index])
        {
            case 0:
                if(bNoNewJobs)
                    break;
                //AI(ht160s-home-resume-drain) : AD-1 tail first. A HOME that landed in the
                //eject tail left a committed tray free at the output with bFullIC already
                //cleared, so IsDischargeEligible can never rediscover it - the latch is the
                //only way back. Per station here, where the legacy ladder could only ever
                //consume ONE tail per lap.
                if(bDischargeTailPending[Index])
                {
                    bDischargeTailPending[Index]=false;
                    RecordProcess("HOME-RESUME Auto: discharge-tail consumed (Auto"+IntToStr(Index+1)+") - re-enter eject at 5000");
                    DischargeTask[Index]=5000;
                    StationTask[Index]=4000;
                    iInFlight++;
                    break;
                }
                if(iInFlight>=iCap)
                    break;
                if(IsFeedEligible(Index))
                {
                    DoFeedTray(Index, 0);
                    StationTask[Index]=2000;
                    iInFlight++;
                }
                else if(IsDischargeEligible(Index))
                {
                    DoDischargeTray(Index, 0);
                    StationTask[Index]=4000;
                    iInFlight++;
                }
                break;

            case 2000:
                if(DoFeedTray(Index, 1))
                    StationTask[Index]=0;
                break;

            case 4000:
                if(DoDischargeTray(Index, 1))
                    StationTask[Index]=0;
                break;

            default:
                //AI(ht160s-ladder-guard) : number-without-action trap, same posture as the
                //module ladder - log it and re-idle rather than stall silently.
                LogLadderFault("Auto.Station"+IntToStr(Index+1), StationTask[Index]);
                StationTask[Index]=0;
                break;
        }
    }
    return bAllIdle;
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
bool TAutoModule::IsRearPlacedButUnsigned(int Index)
{
    //AI(ht160s-home-resume-drain) 20260713 : XS-1/TA-2 adopt-as-delivered discriminator.
    //True when the rear PHYSICALLY holds a tray (raw OutputBottomHasTray sensor, Enable-
    //gated) that was never signed as delivered (bRearDeliveredPending==false, set at
    //DoPlace case 4000 via Notify/SetRearHasTrayFromTrayArm, cleared on consume/discharge/
    //cold-init). That is the HOME-interrupted-deposit window : jaws opened (case 2000) but
    //the case-4000 sign never ran. IsRearHasTray() conflates raw-sensor OR latch, so it
    //cannot tell signed from unsigned; this can.
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return false;
    TMySensor *Sn=GetOutputBottomHasTray(Index);
    if(Sn==NULL || Sn->Enable==false)
        return false;
    return (Sn->IsOn() && bRearDeliveredPending[Index]==false);
}
//---------------------------------------------------------------------------
void TAutoModule::SetRearHasTrayFromTrayArm(int Index, bool bHasTray)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return;
    State[Index].bRearHasTray=bHasTray;
    State[Index].bRearCanUse=bHasTray;
    bRearDeliveredPending[Index]=bHasTray;  //AI(general) 20260608 : Stage0 latch
    //AI(cleanout) 20260706 : late-delivery self-heal. A tray delivered to this Auto rear AFTER
    //the station latched drain-done (Auto pumps before TrayArm, so a place can land one tick
    //after DoAllAutoCleanOut case 7000) would wedge IsAllCleanOutFinish() forever (it blocks on
    //bRearHasTray / bRearDeliveredPending) while DoAuto short-circuits on the latch, so the
    //case-7000 re-collect never re-runs. Drop this station's drain latch so the drain ladder
    //re-collects the late tray (self-heal) - also covers the rear-sensor-disabled silent case.
    if(bHasTray && HSys.Sys.RunMode==Run_CleanOut)
    {
        State[Index].bCleanOutFinish=false;
        bCleanOutResidualLogged[Index]=false;
    }
    if(bHasTray)
        State[Index].Status=AS_REAR_STAGED;  //AI(ht160s-status) 20260703 : Normal-mode delivery
    else if(State[Index].Status==AS_REAR_STAGED)
        State[Index].Status=AS_IDLE;         //AI(ht160s-status) 20260703 : staged latch withdrawn
}
//---------------------------------------------------------------------------
//AI(auto-per-station) 20260802 : Index names the station this ladder drives, replacing
//the module-shared Index that every case used to read. Selection happens once, in
//DoAuto case 1000; case 100 only re-validates that the chosen station is still eligible.
bool TAutoModule::DoFeedTray(int Index, int Flag)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return true;
    TTrayMotor *TrayMotor=NULL;
    TMyCylinder *PushCylinder=NULL;
    TMyCylinder *LeanCylinder=NULL;
    TMySensor *BottomSensor=NULL;
    AnsiString ErrorText;
    int Ret;

    if(Flag==0)
    {
        FeedTask[Index]=1;
        FeedDelay[Index].Clear();
        return true;
    }

    switch(FeedTask[Index])
    {
        case 1:
            FeedTask[Index]=100;
            break;

        case 100:
            //AI(auto-per-station) 20260802 : was Index=FindFeedAuto() here. The choice
            //is made by DoAuto case 1000 now; this re-validates it with the non-mutating
            //predicate. Behaviour note: if the chosen station stopped being eligible in
            //the one scan between selection and here, this abandons the lap (DoAuto
            //re-selects next pass, ~4 ms) where the old code would have silently switched
            //to whatever station FindFeedAuto returned instead. Strictly safer.
            if(IsFeedEligible(Index)==false)
                return true;
            State[Index].Status=AS_LOADING;   //AI(ht160s-status) 20260703 : ladder owns this station before any Y motion
            FeedTask[Index]=200;
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
               || IsSensorOnReady(GetOutputBottomHasTray(Index)))
            {
                FeedTask[Index]=1000;
            }
            else
            {
                ErrorText.sprintf("Auto%d: rear tray data transferred but no-tray sensor. Remove any stranded tray; if no tray, check the rear tray sensor. Retry=recheck sensor, Skip=clear tray data", Index+1);
                Ret=ShowMyError(AnsiString().sprintf("JAM%d11", 11+Index), ErrorText, GetOutputBottomHasTray(Index), true, K_SKIP|K_RETRY);
                if(Ret==K_SKIP)
                {
                    //AI(ht160s-tray-source) 20260625 : clear the staged rear data so the
                    //next cycle does not re-feed a phantom tray. Mirrors the field reset
                    //in DoFeedTray case7000 / DoDischargeTray case1000 / cleanout case7000.
                    State[Index].bRearHasTray=false;
                    State[Index].bRearCanUse=false;
                    bRearDeliveredPending[Index]=false;
                    RearGrid[Index].Clear();
                    RearKind[Index]=eTrayKindNormal;
                    RearTrayID[Index]="";
                    State[Index].Status=AS_IDLE;   //AI(ht160s-status) 20260703 : feed aborted, nothing staged
                    return true;
                }
                //K_RETRY (or any other) : stay in case 200 and re-read next cycle.
            }
            break;

        case 1000:
            if(MoveAutoY(Index, GetAutoFeedY(Index)))
                FeedTask[Index]=3000;
            break;

        case 3000:
            BottomSensor=GetOutputBottomHasTray(Index);
            if(IsSoftSimulate() || IsSensorOnReady(BottomSensor))
                FeedTask[Index]=4000;
            else
            {
                ErrorText.sprintf("Auto%d Feed Tray Miss", Index+1);
                Ret=ShowMyError(AnsiString().sprintf("WAR%d30", 11+Index), ErrorText, BottomSensor, true, K_RETRY);
                if(Ret==K_RETRY)
                    FeedTask[Index]=1000;
            }
            break;

        case 4000:
            LeanCylinder=GetLean(Index);
            if(LeanCylinder!=NULL && (LeanCylinder->Push() || IsSoftSimulate()))
                FeedTask[Index]=5000;
            break;

        case 5000:
            PushCylinder=GetPush(Index);
            if(PushCylinder!=NULL && (PushCylinder->Push() || IsSoftSimulate()))
            {
                FeedDelay[Index].SetMS(GeneralSetting.iAutoPushConfirmSettleMs);
                FeedDelay[Index].On();
                FeedTask[Index]=5100;
            }
            break;

        case 5100:
            if(FeedDelay[Index].Off())
            {
                PushCylinder=GetPush(Index);
                if(IsCylinderOnReady(PushCylinder, IsSoftSimulate()))
                    FeedTask[Index]=6000;
                else
                    FeedTask[Index]=5200;
            }
            break;

        case 5200:
            PushCylinder=GetPush(Index);
            if(PushCylinder!=NULL && (PushCylinder->Pop() || IsSoftSimulate()))
            {
                ErrorText.sprintf("Auto%d Push Tray Miss", Index+1);
                Ret=ShowMyError(AnsiString().sprintf("JAM%d02", 11+Index), ErrorText, &PushCylinder->OnSensor, true, K_RETRY);
                if(Ret==K_RETRY)
                    FeedTask[Index]=5000;
            }
            break;

        case 6000:
            //AI(ht160s-sortarm) 20260707 : park directly at the first-cell Y so the carrier
            //stops at cell(0,0) instead of overshooting to the raw taught FirstSortY datum and
            //being walked back by the SortArm's first place move. GetSortArmCellY folds in the
            //SortArm datum bias + tray YStart (same single source the place move uses). A freshly
            //fed work tray is all EMPTY_IC, so FindPlaceCells' top-to-bottom scan makes Row0 the
            //first placement. NULL-guard falls back to the raw FirstSortY; the SortArm still
            //issues the authoritative absolute place move, so a non-Row0 first cell only forgoes
            //the optimization -- it cannot mis-place.
            if(MoveAutoY(Index,
                         (SortArmModule!=NULL)
                             ? SortArmModule->GetSortArmCellY(GetAutoFirstSortY(Index), 0)
                             : GetAutoFirstSortY(Index)))
                FeedTask[Index]=7000;
            break;

        case 7000:
            TrayMotor=GetAutoVMotor(Index);
            if(TrayMotor!=NULL)
            {
                //AI(ht160s-tray-source) : receive the grid TrayArm staged at rear (born at
                //Empty/Color) instead of fabricating a fresh EMPTY_IC tray here. DoFeedTray
                //only PROMOTES rear->working; occupancy (fHasTray) is owned here. RearGrid is
                //default EMPTY_IC/Normal if a feed ever runs without a staged delivery.
                TrayMotor->Tray.CopyFrom(RearGrid[Index]);
                TrayMotor->fHasTray=true;
                TrayMotor->Refresh();   //AI(ht160s-tray-source) : InitNewTray used to Refresh; keep MotionView in sync
            }
            State[Index].bCarHasTray=true;
            State[Index].bRearHasTray=false;
            State[Index].bRearCanUse=false;
            State[Index].bFullIC=false;
            bRearDeliveredPending[Index]=false;  //AI(general) 20260608 : Stage0 rear tray consumed
            State[Index].Status=AS_SORTING;   //AI(ht160s-status) 20260703 : rear promoted to working
            //AI(HT160S-Maintainer) 20260605 : AMR stack-order bookkeeping. Record the tray
            //pulled up to the working position into the output car. If it is an identity or
            //cover tray (cannot hold IC) flag it full immediately so it discharges to the
            //stack without SortArm. SortArm is gated separately by IsReadyForSortArmPlace.
            if(GeneralSetting.bUseAMR)
            {
                WorkingKind[Index]=RearKind[Index];
                WorkingTrayID[Index]=RearTrayID[Index];   //AI(HT160S-Maintainer) 20260608 : carry 2D TrayID to working tray
                int n=Car[Index].iTrayCount;
                if(n>=0 && n<MAX_TRAY_PER_CAR)
                {
                    Car[Index].Tray[n].SetKind((eTrayKind)WorkingKind[Index]);
                    Car[Index].Tray[n].TrayID=WorkingTrayID[Index];   //AI(HT160S-Maintainer) 20260608 : stamp 2D TrayID onto stack tray
                    //AI(HT160S-Maintainer) 20260608 : the identity tray (stack tray[0]) carries
                    //the car/lot 2D code; copy it to the car so the whole stack is identified.
                    if(WorkingKind[Index]==eTrayKindIdentity)
                        Car[Index].CarID=WorkingTrayID[Index];
                    Car[Index].iTrayCount=n+1;
                }
                if(WorkingKind[Index]!=eTrayKindNormal)
                {
                    State[Index].bFullIC=true;
                    State[Index].Status=AS_FULL;   //AI(ht160s-status) 20260703 : identity/cover discharges without SortArm
                }
            }
            else
                WorkingKind[Index]=eTrayKindNormal;
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(auto-per-station) 20260802 : Index names the station, replacing shared Index.
bool TAutoModule::DoDischargeTray(int Index, int Flag)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return true;
    TTrayMotor *TrayMotor=NULL;
    TMyCylinder *PushCylinder=NULL;
    TMyCylinder *LeanCylinder=NULL;
    //AI(secs-ceid-align) 20260728 : Auto4-6 Unloadtray CEIDs realigned to HT9045
    // (145/146/147). The old 140/141/142 collided with 9045 CEID 140 Prepare Load
    // Tray and 141 GEM Control State Change, so a 9045-dictionary host misread them.
    // AI(secs-comment-truth) 20260805 : was "Unregistered on purpose: EventReport sends
    // S6F11 with an empty report list". No longer true - ab1b99e's 1-292 loop registers all
    // six of these ids like every other 9045 number, so they now ship the default Report 1
    // (which since the 20260804 alignment holds exactly one SV, 1027 System Time), not an
    // empty L[0]. 136/137/138 and 145/146/147 are HT9045's own Unloadtray numbers.
    int AutoCeid[6]={136, 137, 138, 145, 146, 147};
    int &Task = DischargeTask[Index];
    if(Flag==0)
    {
        Task=1;
        DischargeDelay[Index].Clear();
        return true;
    }

    switch(Task)
    {
        case 1:
            Task=100;
            break;

        case 100:
            //AI(auto-per-station) 20260802 : selection moved to DoAuto case 3000; this
            //re-validates against the same terms FindDischargeAuto uses.
            if(bAmrLocked[Index] ||
               State[Index].bFullIC==false || State[Index].bResidueClear==false)
                return true;
            State[Index].Status=AS_DISCHARGING;   //AI(ht160s-status) 20260703
            Task=1000;
            break;

        case 1000:
            if(MoveAutoY(Index, GetAutoDischargeY(Index)))
            {
                TrayMotor=GetAutoVMotor(Index);
                if(TrayMotor!=NULL)
                {
                    iAmrDeviceCount[Index]+=TrayMotor->Tray.CountIC();   //AI(ht160s-agv-devicecount) 20260713 : tally this tray's ICs into the car running total BEFORE ClearTray wipes it (Car.Tray grids are never filled, so the old car-sum was always 0)
                    TrayMotor->ClearTray();   //AI(ht160s-tray-source) : Auto never self-fabricates a tray; ClearTray resets data+fHasTray=false+bHasCover=false (rule #4)
                }
                State[Index].bFullIC=false;
                State[Index].bResidueClear=true;   //AI(ht160s-residue) 20260624 : fresh tray on discharge
                State[Index].bCarHasTray=false;
                State[Index].bRearHasTray=false;
                bRearDeliveredPending[Index]=false;  //AI(general) 20260608 : Stage0 latch clear
                RearGrid[Index].Clear();   //AI(ht160s-tray-source) : cleared rear => cleared staged grid
                State[Index].bFrontHasTray=true;
                if(HGem!=NULL)
                    HGem->EventReport(1, AutoCeid[Index]);
                Task=3000;
            }
            break;

        case 3000:
            PushCylinder=GetPush(Index);
            if(PushCylinder!=NULL && (PushCylinder->Pop() || IsSoftSimulate()))
                Task=4000;
            break;

        case 4000:
            LeanCylinder=GetLean(Index);
            if(LeanCylinder!=NULL && (LeanCylinder->Pop() || IsSoftSimulate()))
                Task=5000;
            break;

        case 5000:
            if(MoveAutoY(Index, GetAutoFeedY(Index)))
            {
                DischargeDelay[Index].SetMS(GeneralSetting.iAutoDischargePostYSettleMs);
                DischargeDelay[Index].On();
                Task=6000;
            }
            break;

        case 6000:
            //AI(general) 20260617 : post-Y settle done; the FrontRise On->settle->Off now
            //lives in the shared DoFrontRiseOnce so Teach Advanced TestGoUpOnce drives the
            //identical cylinder action as this production discharge.
            if(DischargeDelay[Index].Off())
            {
                DischargeSubTask[Index]=1;
                Task=6100;
            }
            break;

        case 6100:
            if(DoFrontRiseOnce(Index, DischargeSubTask[Index], DischargeDelay[Index]))
            {
                //AI(ht160s-status) 20260703 : user decision - the station reads IDLE only
                //after the FULL discharge tail (Y retreated + FrontRise pumped), later
                //than the legacy flag clears at case 1000. Readers flip in phase 5b.
                State[Index].Status=AS_IDLE;
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
            CleanOutTask=500;
            break;

        //AI(cleanout) 20260703 : REAR-COLLECT phase (user design : no manual residual
        //removal). A tray TrayArm delivered to an Auto rear that was never pulled in must be
        //collected by the Auto itself : reuse the normal DoFeedTray rear->working pull
        //(FindFeedAuto finds exactly the delivered-but-unconsumed stations : rear latch +
        //delivered latch + working empty). Loop one station at a time until none is left,
        //then run the GoUp ladder below, which stacks every working tray onto this Auto's
        //own output car.
        case 500:
            //AI(auto-per-station) 20260802 : select here (was inside DoFeedTray case 100).
            iFeedAuto=FindFeedAuto();
            DoFeedTray(iFeedAuto, 0);
            CleanOutTask=600;
            break;

        case 600:
            if(DoFeedTray(iFeedAuto, 1))
            {
                iFeedAuto=FindFeedAuto();
                if(iFeedAuto>=0)
                {
                    DoFeedTray(iFeedAuto, 0);   //another delivered rear tray : pull it in too
                    break;
                }
                CleanOutTask=100;
            }
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
                        //AI(cleanout) 20260703 : Full gate (user design). Never GoUp into a
                        //full output stack : hold this station and ask the operator to empty
                        //it (AMR=0 machine -> operator; the modal repeats until the Full
                        //sensor goes OFF, mirroring ServiceCarFull). Real machine only : the
                        //sim Car-count verdict is not driven by the clean-out GoUp and the
                        //modal re-reads the physical sensor, so sim skips this gate.
                        if(IsSoftSimulate()==false && IsOutputCarFullForAmr(Index))
                        {
                            //AI(amr-unmanned D4-3) 20260721 : full output car during CleanOut
                            //drain. AMR : do NOT pop the operator modal, do NOT GoUp into a
                            //full car (overloads the AGV gripper). Hold this station this tick;
                            //the coordinator CALLs the AGV (D4-2) -> collect (273/274 ClearAmrCar,
                            //Full -> OFF) -> next tick GoUp proceeds. AGV not responding -> WAR0962
                            //(W3/W4). Normally the car was already collected during the sorting
                            //phase (continuous full-CALL), so this hold is the rare AGV-slow edge.
                            if(GeneralSetting.bUseAMR)
                                continue;
                            TMySensor *FullSensor=GetInputFullTray(Index);
                            AnsiString ErrorText;
                            ErrorText.sprintf("Auto%d output stack FULL (sensor) - remove finished trays", Index+1);
                            do
                            {
                                ShowMyError(AnsiString().sprintf("MES%d20", 11+Index), ErrorText, FullSensor, false, K_RETRY);
                                FullSensor=GetInputFullTray(Index);
                            }
                            while(FullSensor!=NULL && FullSensor->Enable==true && FullSensor->IsOn());
                            Car[Index].Clear();
                            InitAutoCarStack(Index);
                        }
                        Cylinder->On();
                        if(IsCylinderOnReady(Cylinder, IsSoftSimulate()))
                            bCleanOutCheck[Index]=true;
                    }
                }
            }
            if(AreAllFlagsOn(bCleanOutCheck))
            {
                CleanOutDelay.SetMS(GeneralSetting.iAutoCleanOutRiseDwellMs);
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
            //AI(cleanout) 20260703 : late-delivery re-collect (replaces the MES0923 manual-
            //removal alarm; user design : the machine collects every tray itself). A TrayArm
            //delivery that landed AFTER the rear-collect phase (deposit ladder already
            //running at the drain boundary) is pulled in and stacked by looping back to the
            //collect phase. GetTrayRequest stops NEW dispatches once SortArm finished, so at
            //most one in-flight tray exists and the loop terminates.
            if(FindFeedAuto()>=0)
            {
                CleanOutTask=500;
                break;
            }
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
                State[Index].Status=AS_CLEANOUT_DONE;   //AI(ht160s-status) 20260703
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
//AI(cleanout) 20260706 : pure per-station drain latch. bCleanOutFinish is set in
//DoAllAutoCleanOut case 7000 (the drain ladder ran to completion). Used ONLY as the DoAuto
//stop-gate : once every station has drained, stop pumping the GoUp ladder. Kept latch-based
//on purpose so a lingering/flickering rear sensor cannot restart the whole ladder (thrash) -
//a physically stuck tray is handled by the residual watchdog + the live finish predicate.
bool TAutoModule::AllStationsDrainLatched()
{
    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
        if(State[Index].bCleanOutFinish==false)
            return false;
    return true;
}
//---------------------------------------------------------------------------
//AI(cleanout) 20260706 : LIVE clean-out finish predicate (was : return the pure latch loop,
//which could report finished with a tray still physically on a station because case 7000
//clears the software flags and latches true in the SAME block, with no sensor re-check).
//This is what csystem CheckCleanOutFinish + TrayArm consult; it re-computes every call so a
//tray that reappears mid-cleanout cancels finish. Conditions (cheap/upstream first) :
//  (1) upstream SortArm live-finished (Auto <- SortArm <- Loader ; no cycle) - was only
//      gated at DoAuto case 100 ENTRY, never re-checked once the latch was set.
//  (2) the drain ladder actually reached case 7000 (AllStationsDrainLatched).
//  (3) per-station : no software/virtual residual (both sim + real, mirrors Loader).
//  (4) per-station REAL-machine sensor re-check + Full gate (sim early-out mirrors
//      RefreshAutoState so a laptop with InType=0 phantom-present sensors still completes).
//NOTE : NO FeedTask/DischargeTask/CleanOutTask idle-gate here. Unlike Empty/Color, Auto's
//drain is a SEPARATE ladder (CleanOutTask) whose cursors legitimately rest at non-1 values
//after completion (CleanOutTask stays 7000 ; the rear-collect leaves FeedTask at 100/7000 -
//DoFeedTray only resets to 1 via Flag==0 at the NEXT cycle start). AllStationsDrainLatched
//is the correct "drain fully ran" proof for Auto; a FeedTask==1 gate would false-block forever.
bool TAutoModule::IsAllCleanOutFinish()
{
    if(HSys.Sys.RunMode!=Run_CleanOut)
        return AllStationsDrainLatched();
    if(SortArmModule==NULL || SortArmModule->IsCleanOutFinish()==false)
        return false;
    if(AllStationsDrainLatched()==false)
        return false;
    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
    {
        if(State[Index].bRearHasTray || State[Index].bRearCanUse || bRearDeliveredPending[Index])
            return false;
        if(State[Index].bCarHasTray || State[Index].bFrontHasTray || State[Index].bFullIC)
            return false;
        TTrayMotor *VMot=GetAutoVMotor(Index);
        if(VMot!=NULL && VMot->fHasTray)
            return false;
        if(IsSoftSimulate()==false)
        {
            TMySensor *Front=GetInputHasTray(Index);
            if(Front!=NULL && Front->Enable==true && Front->IsOn())
                return false;   //residual tray at this Auto front feed position
            TMySensor *Full=GetInputFullTray(Index);
            if(Full!=NULL && Full->Enable==true && Full->IsOn())
                return false;   //output stack still holds trays
            TMySensor *Rear=GetOutputBottomHasTray(Index);
            if(Rear!=NULL && Rear->Enable==true && Rear->IsOn())
                return false;   //residual tray at this Auto rear staging
            if(IsOutputCarFullForAmr(Index))
                return false;   //Full gate (mirrors Empty/Color) : a drain GoUp is still owed
        }
    }
    return true;
}
//---------------------------------------------------------------------------
//AI(cleanout) 20260706 : EventLog-only residual watchdog (user chose the lightweight variant :
//no modal, no timer, no GeneralSetting field). Called each tick from DoAuto ONLY while the
//drain ladder is latched-done. On the REAL machine, if a station still shows a physical tray
//(front / full / rear sensor ON) after the drain completed, log ONE line per episode (the
//bCleanOutResidualLogged latch, cleared in InitialFlag) tagged MES1123..MES1623 so it is
//per-station identifiable in D:\HT160S_Log\EventLog. Sim early-outs (sensors not read), so a
//laptop clean-out still completes without a phantom log.
void TAutoModule::ServiceCleanOutResidualWatchdog()
{
    if(IsSoftSimulate())
        return;
    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
    {
        if(bCleanOutResidualLogged[Index])
            continue;
        TMySensor *Front=GetInputHasTray(Index);
        TMySensor *Full =GetInputFullTray(Index);
        TMySensor *Rear =GetOutputBottomHasTray(Index);
        bool bFrontOn=(Front!=NULL && Front->Enable==true && Front->IsOn());
        bool bFullOn =(Full !=NULL && Full->Enable==true  && Full->IsOn());
        bool bRearOn =(Rear !=NULL && Rear->Enable==true  && Rear->IsOn());
        if(bFrontOn==false && bFullOn==false && bRearOn==false)
            continue;
        AnsiString Where;
        Where.sprintf("front=%d full=%d rear=%d", bFrontOn?1:0, bFullOn?1:0, bRearOn?1:0);
        g_EventLog.Log(AnsiString().sprintf("MES%d23", 11+Index),
                       AnsiString().sprintf("Auto%d clean-out residual tray after drain - remove it", Index+1),
                       Where);
        bCleanOutResidualLogged[Index]=true;
    }
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
//AI(ht160s-agv-devicecount) 20260713 : running IC total on the output car, accumulated
//per discharged tray from the working tray's CountIC (see DoDischargeTray). Source for
//the AGV DeviceCount SVID; replaces the always-0 TMyCar::GetTotalDeviceCount (the Car's
//Tray[] grids are never filled with placed-IC data, so summing them returned 0).
int TAutoModule::GetAmrDeviceCount(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return 0;
    return iAmrDeviceCount[Index];
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
    iAmrDeviceCount[Index]=0;   //AI(ht160s-agv-devicecount) 20260713 : car re-seed resets the running IC total (every car-clear path funnels here; a keep-material HOME skips InitAutoCarStack so its ledger is preserved)
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
    //AI(cleanout) 20260701 : once Clean Out has drained the Loader (SortArm finished), the Autos
    //are about to / already running their physical clean-out discharge - stop requesting trays so
    //TrayArm never delivers an empty tray onto an Auto that has latched bCleanOutFinish (which
    //would strand the tray on the rear and let Clean Out complete with a tray left behind).
    if(HSys.Sys.RunMode==Run_CleanOut && SortArmModule!=NULL && SortArmModule->IsCleanOutFinish())
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
    //AI(amr-unmanned D4-1) 20260721 : do NOT request the next tray while the output car is
    //FULL. Reuse IsOutputCarFullForAmr (the SAME verdict the AGV full-CALL / finish gate
    //use) so 'full' is single-sourced : sim = iSimAmrMaxTray count, REAL = Full sensor ONLY
    //(user ruling). Closes the user invariant's edge : after a GoUp fills the car, rear
    //stops pulling until the AGV collects (Full sensor -> OFF). GetNextTrayKindForAuto's
    //iTrayCount>=MAX_TRAY_PER_CAR check is KEPT as Car.Tray[] bounds protection, not 'full'.
    if(GeneralSetting.bUseAMR && IsOutputCarFullForAmr(Index))
        return eTrayReqNone;
    if(GeneralSetting.bUseAMR)
        return GetNextTrayKindForAuto(Index); // 0/1/2, or -1 when the car is full
    return eTrayKindNormal;                  // Normal mode : always an empty work tray
}
//---------------------------------------------------------------------------
//AI(general) 20260608 : Stage1 demand finder. Scans Auto1~6 and returns the first
//   station that currently wants a tray, reporting the requested eTrayKind through
//   OutKind. Returns -1 (and OutKind=eTrayReqNone) when no Auto wants a tray, which
//   lets the TrayArm stay idle instead of shuttling needlessly.
//   AI(ht160s-amr-divert) 20260719 : WantKind!=eTrayReqNone narrows both scans to Autos
//   requesting exactly that kind (TrayArm recovery divert wants Normal-only targets).
int TAutoModule::FindTrayRequestAuto(int &OutKind, int WantKind)
{
    OutKind=eTrayReqNone;
    //AI(ht160s-predictive-supply) 20260707 : Phase 1 (opt-in). Prefer an Auto that SortArm
    //is CURRENTLY holding a fixed-route IC for and that still wants a tray, so the Auto on
    //SortArm's critical path gets its empty tray before the plain lowest-index scan and the
    //held IC does not stall (silent SelectPlaceAuto / PlaceTask=1 freeze). We only REORDER
    //the set GetTrayRequest already approves : each candidate is re-checked with GetTrayRequest
    //(so occupied / pending / AMR-locked / CleanOut Autos are still refused) and OutKind
    //carries the Auto's OWN requested kind (never overridden), so AMR stack order stays
    //intact. Iteration order = SortArm sucker 0..3 = its own place order (SelectPlaceAuto).
    //Falls through to the unchanged lowest-index scan when off or when SortArm holds nothing
    //that maps to a requesting Auto.
    if(GeneralSetting.bUsePredictiveAutoSupply && SortArmModule!=NULL)
    {
        int Targets[4];   //max SortArm suckers (SORT_ARM_SUCKER_COUNT)
        int nTargets=SortArmModule->GetHeldTargetAutos(Targets, 4);
        for(int t=0; t<nTargets; t++)
        {
            int Req=GetTrayRequest(Targets[t]);
            if(Req!=eTrayReqNone && (WantKind==eTrayReqNone || Req==WantKind))
            {
                OutKind=Req;
                return Targets[t];
            }
        }
    }
    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
    {
        int Req=GetTrayRequest(Index);
        if(Req!=eTrayReqNone && (WantKind==eTrayReqNone || Req==WantKind))
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
    //AI(cleanout) 20260706 : late-delivery self-heal (see SetRearHasTrayFromTrayArm for the full
    //note) - re-open this station's drain so case-7000 re-collects a tray delivered after latch.
    if(HSys.Sys.RunMode==Run_CleanOut)
    {
        State[Index].bCleanOutFinish=false;
        bCleanOutResidualLogged[Index]=false;
    }
    State[Index].Status=AS_REAR_STAGED;  //AI(ht160s-status) 20260703 : single producer of the staged state
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
//AI(ht160s-clampgrip) 20260806 : PHYSICAL grip verdict for this station's working car, for the
//SortArm boundary confirm just before its irreversible place Z-down (on-site notes 7/9 : the
//working tray can jump out of the clamp while the software still shows CarHasTray). Public so
//SortArm can ask without reaching into HSys.Cyn itself - the cylinder lookup stays here, where
//GetPush already lives. Tri-state passthrough : 1=gripping, 0=tray gone, -1=no verdict.
//NOTE the deliberate asymmetry with the Color diaper : this only REPORTS. An Auto working tray
//holds placed ICs, so auto-clearing its grid would destroy the placed-IC record; the operator
//decides.
int TAutoModule::GetCarTrayGripVerdict(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return -1;
    return GetClampGripVerdict(GetPush(Index), IsSoftSimulate());
}
//---------------------------------------------------------------------------
//AI(ht160s-clampgrip) 20260806 : the grip reed, so SortArm's alarm screen can name the real IO
//point (same idiom as the JAM%d02 call site in DoFeedTray case 5200).
TMySensor *TAutoModule::GetCarTrayPushOnSensor(int Index)
{
    TMyCylinder *Push=GetPush(Index);

    if(Push==NULL)
        return NULL;
    return &Push->OnSensor;
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
//AI(ht160s-status) 20260703 : eAutoStatus display name (DescribeStation / stbMain).
static const char *AutoStatusName(int St)
{
    switch(St)
    {
        case AS_IDLE:          return "IDLE";
        case AS_REAR_STAGED:   return "REAR_STAGED";
        case AS_LOADING:       return "LOADING";
        case AS_SORTING:       return "SORTING";
        case AS_FULL:          return "FULL";
        case AS_DISCHARGING:   return "DISCHARGING";
        case AS_CLEANOUT_DONE: return "CLEANOUT_DONE";
    }
    return "?";
}
//---------------------------------------------------------------------------
int TAutoModule::GetStationStatus(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return AS_IDLE;
    return State[Index].Status;
}
//---------------------------------------------------------------------------
//AI(auto-obsv) 20260801 : the Auto module cursors. Unlike the Loader - whose per-side
//FeedTask/CcdTask/DischargeTask/DestackTask are already in FeederDecision.txt - none of
//these appeared in any State Record, even though ALL SIX stations share them. DoAuto is a
//strictly linear phase ladder (1 -> 100 -> 1000 -> 2000 feed -> 3000 -> 4000 discharge -> 1)
//in which feed and discharge are mutually exclusive and each phase serves exactly ONE
//station, chosen by FindFeedAuto / FindDischargeAuto. iFeedAuto and iDischargeAuto are
//therefore the answer to "which Auto is the module actually working on right now", which
//no snapshot could previously answer.
AnsiString TAutoModule::DescribeModule()
{
    AnsiString s;
    s  = "[AutoModule] (cursors SHARED by all six stations)\r\n";
    //AI(auto-per-station) 20260802 : feed/discharge cursors are per station now; the two
    //i*Auto values are dispatch-only (which station the serial lap picked).
    s += "  Concurrency="   + IntToStr(GeneralSetting.iAutoConcurrency)
       + "  CleanOutTask="  + IntToStr(CleanOutTask)
       + "  iFeedAuto="     + IntToStr(iFeedAuto)
       + "  iDischargeAuto="+ IntToStr(iDischargeAuto) + "\r\n";
    for(int DumpIndex=0; DumpIndex<AUTO_STATION_COUNT; DumpIndex++)
    {
        s += "  Auto" + IntToStr(DumpIndex+1)
           + " FeedTask="      + IntToStr(FeedTask[DumpIndex])
           + "  DischargeTask=" + IntToStr(DischargeTask[DumpIndex])
           + "  DischargeSub="  + IntToStr(DischargeSubTask[DumpIndex])
           + "  StationTask="   + IntToStr(StationTask[DumpIndex])
           + "  FeedElig="      + IntToStr(IsFeedEligible(DumpIndex) ? 1 : 0)
           + "  DischElig="     + IntToStr(IsDischargeEligible(DumpIndex) ? 1 : 0) + "\r\n";
    }
    return s;
}
//---------------------------------------------------------------------------
AnsiString TAutoModule::DescribeStation(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return "";

    RefreshAutoState();

    AnsiString s;
    s  = "[Auto" + IntToStr(Index+1) + "]\r\n";
    s += "  Status=" + AnsiString(AutoStatusName(State[Index].Status)) + "\r\n";
    s += "  CarHasTray="  + IntToStr(State[Index].bCarHasTray ? 1 : 0)
       + "  RearHasTray=" + IntToStr(State[Index].bRearHasTray ? 1 : 0)
       + "  FullIC="      + IntToStr(State[Index].bFullIC ? 1 : 0)
       + "  RearPending=" + IntToStr(bRearDeliveredPending[Index] ? 1 : 0)
       + "  AmrLocked="   + IntToStr(bAmrLocked[Index] ? 1 : 0)
       + "  WorkingKind=" + IntToStr(WorkingKind[Index]) + "\r\n";
    //AI(ht160s-obsv-p0) 20260720 : discharge-gate blockers - a post-resume "full forever"
    //station is diagnosable only if the gate inputs are in the dump.
    //AI(auto-obsv) 20260801 : the two inputs that decide whether this station is asking for
    //a tray at all. TrayReq is GetTrayRequest() itself - the exact value TrayArm dispatch
    //reads - so "why was TrayArm not sent here" becomes a one-line read instead of an
    //eight-gate reconstruction by elimination. CarTrays (Car[].iTrayCount) was dumped
    //NOWHERE in the tree, yet it is what selects identity vs cover vs normal for the next
    //tray, so it also decides whether an AMR-recovery divert could ever match this station.
    s += "  TrayReq="   + IntToStr(GetTrayRequest(Index))
       + "  CarTrays="  + IntToStr(Car[Index].iTrayCount) + "\r\n";
    s += "  ResidueClear=" + IntToStr(State[Index].bResidueClear ? 1 : 0)
       + "  DischTail="    + IntToStr(bDischargeTailPending[Index] ? 1 : 0)
       + "  RearKind="     + IntToStr(RearKind[Index])
       + "  RearID="       + RearTrayID[Index] + "\r\n";

    //AI(ht160s-agv) 20260627 : Auto-full dump (State Record gap analysis). Log the computed
    //full verdict + the raw InputFullTray sensor so a hang at ServiceCarFull is diagnosable
    //(computed-full-but-sensor-not vs sensor-full-but-not-cleared).
    {
        TMySensor *FullSensor=GetInputFullTray(Index);
        int iFullSn  = (FullSensor!=NULL && FullSensor->Enable==true && FullSensor->IsOn()) ? 1 : 0;
        int iFullVer = IsOutputCarFullForAmr(Index) ? 1 : 0;
        s += "  FullVerdict="  + IntToStr(iFullVer)
           + "  InputFullSn="  + IntToStr(iFullSn) + "\r\n";
    }

    //AI(cleanout) 20260706 : clean-out diagnostic (State Record observability). Log the drain
    //latch + raw front/rear sensors + the computed "this station blocks finish" verdict so a
    //clean-out that will not complete is diagnosable per station (log-computed-verdict rule).
    {
        TMySensor *FrontSn=GetInputHasTray(Index);
        TMySensor *RearSn =GetOutputBottomHasTray(Index);
        TTrayMotor *Vc=GetAutoVMotor(Index);
        int iFrontSn=(FrontSn!=NULL && FrontSn->Enable==true && FrontSn->IsOn()) ? 1 : 0;
        int iRearSn =(RearSn !=NULL && RearSn->Enable==true  && RearSn->IsOn())  ? 1 : 0;
        bool bBlocks = State[Index].bRearHasTray || State[Index].bRearCanUse || bRearDeliveredPending[Index]
                    || State[Index].bCarHasTray || State[Index].bFrontHasTray || State[Index].bFullIC
                    || (Vc!=NULL && Vc->fHasTray);
        if(IsSoftSimulate()==false && (iFrontSn || iRearSn))
            bBlocks=true;
        s += "  CleanOut: DrainLatch=" + IntToStr(State[Index].bCleanOutFinish ? 1 : 0)
           + "  FrontSn=" + IntToStr(iFrontSn)
           + "  RearSn="  + IntToStr(iRearSn)
           + "  ResidualLogged=" + IntToStr(bCleanOutResidualLogged[Index] ? 1 : 0)
           + "  BlocksFinish=" + IntToStr(bBlocks ? 1 : 0) + "\r\n";
    }

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
//Run_Normal in BOTH AMR and Normal(manual-transport, bUseAMR=0) : no AGV exists in
//Normal mode, so the InputFullTray sensor (last line) / logical-full fallback raise
//the SAME operator full modal directly, with no AGV wait.
void TAutoModule::ServiceCarFull()
{
    if(HSys.Sys.RunMode!=Run_Normal)
        return;

    for(int Index=0; Index<AUTO_STATION_COUNT; Index++)
    {
        //AI(ht160s-agv) 20260615 : when the SECS link is up, hand a full car to the AGV
        //(the CEID272/273/274 handshake clears it via ClearAmrCar) instead of popping the
        //operator full-car modal. While the host is disconnected this falls through to the
        //original modal + manual car change, so offline behavior is unchanged.
        //AI(ht160s-agv) 20260627 : AGV handshake in flight. Defer the operator modal while
        //the car is full AND the AMR lock is held; start the wait timer once. On expiry,
        //abort THIS Auto's handshake (so PollAndCall does not re-CALL) and fall through to
        //the existing held alarm below. Happy path (AGV takes the car before timeout) is
        //unchanged: bFull/bLocked clears, we continue without ever raising the modal.
        //AI(ht160s-amr0) 20260630 : AGV handoff is AMR-only; in Normal mode skip the
        //wait/defer and fall straight to the sensor / logical-full operator modal below.
        if(GeneralSetting.bUseAMR && HGem!=NULL && HGem->IsSelected())
        {
            //AI(amr-unmanned D4-4) 20260721 : link-up AMR -> the AGV full-collect handshake
            //owns the full car (PollAndCall CEID272 -> ServiceHandshake 273/274 -> ClearAmrCar;
            //its aging escalates to WAR0962 via W3/W4 if the AGV never responds). NO operator
            //modal here, and no separate per-Auto full-wait timer (the unified iAgvTimeoutSec
            //governs in the coordinator). Was: a local wait + hold-off then fall-through to
            //the modal. SECS link DOWN still falls through to the operator modal below (a
            //genuine comm-lost fault, distinct from AGV-slow).
            continue;
        }

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
                ShowMyError(AnsiString().sprintf("MES%d20", 11+Index), ErrorText, FullSensor, false, K_RETRY);
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
    if(HSys.Sys.RunMode==Run_CleanOut && AllStationsDrainLatched())
    {
        //AI(cleanout) 20260706 : drain ladder is done (stop pumping). Keep the STOP gate on
        //the pure latch so a lingering sensor cannot restart the whole GoUp ladder (thrash).
        //The residual watchdog logs (EventLog-only) if a station still shows a physical tray,
        //so a stuck-tray hang is diagnosable, not silent. The live IsAllCleanOutFinish()
        //(sensor + SortArm re-checked) is what csystem / TrayArm consult for true completion.
        ServiceCleanOutResidualWatchdog();
        return;
    }

    //AI(auto-per-station) 20260802 : per-station mode. The module ladder keeps ONLY the
    //once-per-cycle housekeeping and the CleanOut drain; the six feed/discharge ladders
    //run independently in ServiceStations(). Concurrency==0 falls through to the original
    //linear ladder below, bit for bit - that is the on-site rollback.
    if(GeneralSetting.iAutoConcurrency>0)
    {
        if(HSys.Sys.RunMode==Run_CleanOut)
        {
            //Hand over to the module-wide drain ONLY once every station ladder is idle.
            //DoAllAutoCleanOut drives all six Y axes and pops all six clamp pairs; letting
            //it start while a station is mid-transfer would command a conflicting Y target
            //and release a tray in flight. Start nothing new, drain what is running.
            if(ServiceStations(true)==false)
                return;
            //all idle : fall through to the legacy switch, whose case 100 owns CleanOut
        }
        else
        {
            //CheckAutoTray refreshes State[] from the sensors; FindFeedAuto is called for
            //its SIDE EFFECT only - it is the sole maintainer of State[*].bRearCanUse,
            //which IsDrainedForAmr and the CleanOut residual watchdog both read.
            CheckAutoTray();
            ServiceCarFull();
            FindFeedAuto();
            ServiceStations(false);
            Task=1;   //module ladder parks; the stations own the work now
            return;
        }
    }

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
            //AI(auto-per-station) 20260802 : the station is chosen HERE and carried in the
            //dispatch cursor for this lap; the ladder itself is parameterised. FindFeedAuto
            //is still the selector (and still refreshes State[*].bRearCanUse as before), so
            //selection order is unchanged.
            iFeedAuto=FindFeedAuto();
            if(iFeedAuto>=0)
            {
                DoFeedTray(iFeedAuto, 0);
                Task=2000;
            }
            else
                Task=3000;
            break;

        case 2000:
            if(DoFeedTray(iFeedAuto, 1))
                Task=3000;
            break;

        case 3000:
        {
            CheckAutoTray();
            //AI(ht160s-home-resume-drain) 20260713 : AD-1 discharge-tail resume. A HOME
            //that landed at DischargeTask 5000-6100 left a committed tray free at the
            //output with no live discharge candidate (bFullIC was cleared at case 1000, so
            //FindDischargeAuto skips it). Re-enter the eject phase at its START (case 5000
            //re-commands MoveAutoY-to-feed and arms a FRESH DischargeDelay) : a phase-start
            //re-entry, not a mid-step jump into a Clear'd timer. AD-2 forced every FrontRise
            //Off during the drain, so the 6100 pump starts from a known-down riser. The
            //latch survived the keep-material wipe; consume it here so the tail runs once.
            int iTail=-1;
            for(int i=0; i<AUTO_STATION_COUNT; i++)
                if(bDischargeTailPending[i])
                {
                    iTail=i;
                    break;
                }
            if(iTail>=0)
            {
                bDischargeTailPending[iTail]=false;
                RecordProcess("HOME-RESUME Auto: discharge-tail consumed (Auto"+IntToStr(iTail+1)+") - re-enter eject at 5000");   //AI(ht160s-obsv-p0)
                iDischargeAuto=iTail;
                DischargeTask[iTail]=5000;
                Task=4000;
                break;
            }
            //AI(auto-per-station) 20260802 : same shape as case 1000 - select here, pass in.
            iDischargeAuto=FindDischargeAuto();
            if(iDischargeAuto>=0)
            {
                DoDischargeTray(iDischargeAuto, 0);
                Task=4000;
            }
            else
                Task=1;
            break;
        }

        case 4000:
            if(DoDischargeTray(iDischargeAuto, 1))
                Task=1;
            break;

        case 5000:
            if(DoAllAutoCleanOut(1))
                Task=1;
            break;
        default:
            //AI(ht160s-ladder-guard) 20260703 : a state number with no matching case
            //(the 'number but no action' trap). Log it so a future dead-jump is a
            //diagnosable EventLog event, not a silent stall, and restart the ladder.
            LogLadderFault("Auto.DoAuto", Task);
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
//AI(ht160s-frontrise-pushpop) 20260804 : the RISE now goes through TMyCylinder::Push() instead
//of a raw On() plus an IsCylinderOnReady() poll. Root cause this fixes (on-site 20260803): the
//old case 1 drove Rise->On() every scan and then only read the on-reed - IsCylinderOnReady is a
//bare OnSensor.IsOn() (mycylin.cpp), no timer and no alarm - so a reed that never made froze
//this sub-task forever, silently. With [Auto] Concurrency=0 the six stations share ONE ladder
//whose case 4000 blocks on DoDischargeTray, so one stuck riser starves every station
//rear->working-car pull: on 20260803 Auto4 (named by SECS CEID=145 at 15:25:35.634, 1.5 s after
//the module entered Task 4000) held the shared rung for ~8 minutes and the only trace was the
//300 s StuckWatchdog. Push() carries the confirm + timeout + SetCylinderAlarm the class already
//provides (C_Auto*_FrontRiseTray _On reeds are Enable=1 with OnAlarmTime=5000 in the in-force
//IO_Table), so a dead reed now alarms in 5 s naming the cylinder instead of hanging the machine.
//case 1 is a one-shot Rise->Reset() arm state, the idiom every feeder module already uses in
//front of a confirmed stroke (aEmpty.cpp:1192/1255). It is REQUIRED, not decoration: Task is
//shared by Push/Pop and the 20260731 iLastDir re-arm only covers a direction CHANGE, so a stroke
//abandoned mid-flight would leave Task=50 with a live wall-clock Delay and the next SAME-direction
//entry would raise the timeout alarm on its first scan, zero grace. One-shot only - calling
//Reset() every scan would re-arm the budget every poll and the alarm could never fire at all.
//The LOWER half is deliberately left as the original fire-and-forget Rise->Off(). Routing it
//through Pop() would newly make six _Off reeds load-bearing on every discharged tray when the
//only code that has ever read them is the HOME drain (line 199) - and this machine failed that
//very confirm twice on 20260803 ("40040 Cylinder=C_Empty_FrontRiseTray_1 Func=Pop"). Add the Pop
//confirm only after all twelve C_Auto*_FrontRiseTray reeds have been exercised on the machine.
//KNOWN GAP, deliberately not fixed here: DoAllAutoCleanOut case 4000/5000 still drives the same
//six risers with raw On()/Off() + IsCylinderOnReady and waits for ALL SIX, so the identical
//silent hang survives on the end-of-lot path.
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
            Rise->Reset();
            SubTask=2;
            break;

        case 2:
            if(Rise->Push() || IsSoftSimulate())
            {
                Delay.SetMS(GeneralSetting.iAutoFrontRiseDwellMs);
                Delay.On();
                SubTask=3;
            }
            break;

        case 3:
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