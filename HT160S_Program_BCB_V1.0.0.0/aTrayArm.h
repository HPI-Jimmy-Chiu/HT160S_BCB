//---------------------------------------------------------------------------
#ifndef aTrayArmH
#define aTrayArmH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "HTimer.h"
#include "MotorAndIO/MyMotor.h"   //AI(ht160s-tray-source) : MyMotor.h for MMTrayArmX->Tray (arm grid lives on the motor)
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260605 : TrayArm single-axis transport arm. Picks an
//empty tray and places it onto a target station, following the LoaderModule->
//DoLoader procedural ladder pattern (no FSM). All steps are non-blocking.
//---------------------------------------------------------------------------
enum eTrayArmStatus
{
    TAS_IDLE=0,
    TAS_PICKING,
    TAS_CARRYING,
    TAS_PLACING,
};
//---------------------------------------------------------------------------
enum eTrayArmJob
{
    TAJOB_NONE=0,
    TAJOB_EMPTYTRAY_TO_AUTO,            //supply empty tray : EmptyTray rear -> Auto rear (Normal mode)
    TAJOB_LOADER_RECOVERY,             //AI(HT160S-Maintainer) 20260606 : recover empty tray stranded at Loader rear, then supply an Auto OR recycle to EmptyTray
    TAJOB_AMR_SUPPLY,                   //AMR mode : build Auto stack identity/cover/normal in order
};
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260606 : where a carried tray is placed. Decided up-front for
//supply jobs, decided after pick for Loader-recovery jobs (supply Auto vs recycle Empty).
enum eTrayArmPlaceDest
{
    TAPLACE_AUTO=0,                    //place onto a target Auto rear (supply)
    TAPLACE_EMPTY,                     //recycle back to the EmptyTray rear
    TAPLACE_COLOR,                     //AI(HT160S-Maintainer) 20260625 : return identity tray to Color (same contract as Empty)
};
//---------------------------------------------------------------------------
//AI(ht160s-trayarm-teach-test) 20260627 : flat channel index for the Teach Advanced
//TrayArm test tab. One id per TrayArm handoff station, so a single test function takes a
//different index (grab sources Empty/Color/Loader; place targets Auto1-6 + recycle Empty/
//Color). Maps to the Teach.TrayXArmTo*XPosition fields via GetChannelHandoffX. Auto ids are
//contiguous so (Channel-TACH_AUTO1) is the 0-based Auto index for GetAutoX.
enum eTrayArmChannel
{
    TACH_EMPTY=0,
    TACH_COLOR,
    TACH_LOADER,
    TACH_AUTO1,
    TACH_AUTO2,
    TACH_AUTO3,
    TACH_AUTO4,
    TACH_AUTO5,
    TACH_AUTO6,
    TACH_COUNT,
};
//---------------------------------------------------------------------------
class TTrayArmModule
{
private:
    bool bHasTray;
    int Status;
    int Job;
    int PickTask;
    int PlaceTask;
    int iAutoTarget;
    int iDeliverKind;                  //AI(HT160S-Maintainer) 20260605 : eTrayKind being delivered this job (AMR)
    AnsiString iDeliverTrayID;         //AI(HT160S-Maintainer) 20260608 : 2D TrayID carried from Color for the identity tray
    int PlaceDest;                     //AI(HT160S-Maintainer) 20260606 : eTrayArmPlaceDest for the current carry
    bool bCleanOutFinish;
    HTimer ArmDelay;
    unsigned int dwZUpLostStart;       //AI(HT160S-Maintainer) 20260622 : TrayArm X-move Z-up loss debounce (GetTickCount of first loss; 0=clear)

    bool IsSoftSimulate();
    bool MoveTrayArmX(int Position);
    bool DoZUp();
    bool DoZDown();
    int DecideJob();
    bool DoPick(int Flag);
    bool DoPlace(int Flag);
    bool DoPlaceToEmpty(int Flag);     //AI(HT160S-Maintainer) 20260606 : recycle carried tray to EmptyTray rear
    bool DoPlaceToColor(int Flag);     //AI(HT160S-Maintainer) 20260625 : return identity tray to Color rear (mirrors DoPlaceToEmpty)
    void DecidePlaceDestAfterPick();   //AI(HT160S-Maintainer) 20260606 : Loader-recovery : choose supply Auto vs recycle Empty
    int GetPickSourceX();              //AI(HT160S-Maintainer) 20260606 : pick X for current job (Loader/Color/Empty)
    int GetAutoX(int Index);
    int GetColorX();                   //AI(HT160S-Maintainer) 20260605 : AMR identity-tray pickup X
    bool IsPickFromColor();            //AI(HT160S-Maintainer) 20260605 : this job picks from Color (identity)
    //AI(ht160s-trayarm-teach-test) 20260627 : shared physical motion primitives. Production
    //(DoPick/DoPlace/DoPlaceToEmpty/DoPlaceToColor) AND the Teach Advanced test compose these,
    //so the grab/release choreography lives in ONE place (single source of truth).
    bool DoMoveToStationZSafe(int X, int &Task);   //Z-up then X to station (Task 1,10)
    bool DoLowerClampRaise(bool bGrab, int &Task); //Z-down, push(grab)/pop(release) clamps+dwell, Z-up (Task 1000..3000)
    int GetChannelHandoffX(int Channel);           //eTrayArmChannel -> Teach.TrayXArmTo*XPosition
    bool ChannelPlaceClear(int Channel);           //place destination rear is clear (anti-clash gate)
    AnsiString GetChannelName(int Channel);        //display name for status/alarm text

public:
    TTrayArmModule();
    void InitialFlag(bool bKeepMaterial=false);
    void DoTrayArm(int &Task);
    bool HasTray();
    int GetStatus();
    bool IsCleanOutFinish();
    //AI(HT160S-Maintainer) 20260624 : public so Motor Test / Teach manual screens can gate TrayArm X
    //move/jog on the SAME Z-up interlock as production (mirrors public SortArm AreAllSuckersHome).
    //Pure sensor read, no side effects; returns true under SOFT_SIMULATE (dev build, no IO card).
    bool IsZUpAtPosition();            //canonical TrayArm X-move interlock (Z lift up-sensor lit)
    //AI(ht160s-trayarm-teach-test) 20260627 : Teach Advanced TrayArm test entry points. Pure
    //motion dry-run (NO peer-module tray-tracking mutation), task-stepped by the caller like
    //SortArm MoveSuckerToCell : caller inits Task=1 then calls each tick until it returns true.
    //They reuse the SAME DoMoveToStationZSafe/DoLowerClampRaise primitives as production.
    bool TestGrabFromChannel(int Channel, int &Task);   //grab a tray from Empty/Color/Loader
    bool TestPlaceToChannel(int Channel, int &Task);    //place a tray to Auto1-6 / recycle Empty/Color
    bool CanTestTrayArm(int Channel, bool bGrab, AnsiString &Err);  //parametric ready/anti-clash gate
};
//---------------------------------------------------------------------------
extern TTrayArmModule *TrayArmModule;
void InitializeTrayArmModule();
void ShutdownTrayArmModule();
//---------------------------------------------------------------------------
#endif