//---------------------------------------------------------------------------
#ifndef aTrayArmH
#define aTrayArmH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "HTimer.h"
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
    bool bTrayFeedFinish;
    HTimer ArmDelay;

    bool IsSoftSimulate();
    bool MoveTrayArmX(int Position);
    bool DoZUp();
    bool DoZDown();
    int DecideJob();
    bool DoPick(int Flag);
    bool DoPlace(int Flag);
    bool DoPlaceToEmpty(int Flag);     //AI(HT160S-Maintainer) 20260606 : recycle carried tray to EmptyTray rear
    void DecidePlaceDestAfterPick();   //AI(HT160S-Maintainer) 20260606 : Loader-recovery : choose supply Auto vs recycle Empty
    int GetPickSourceX();              //AI(HT160S-Maintainer) 20260606 : pick X for current job (Loader/Color/Empty)
    int GetAutoX(int Index);
    int GetColorX();                   //AI(HT160S-Maintainer) 20260605 : AMR identity-tray pickup X
    bool IsPickFromColor();            //AI(HT160S-Maintainer) 20260605 : this job picks from Color (identity)

public:
    TTrayArmModule();
    void InitialFlag(bool bKeepMaterial=false);
    void DoTrayArm(int &Task);
    bool HasTray();
    int GetStatus();
    bool IsCleanOutFinish();
    bool IsTrayFeedFinish();
};
//---------------------------------------------------------------------------
extern TTrayArmModule *TrayArmModule;
void InitializeTrayArmModule();
void ShutdownTrayArmModule();
//---------------------------------------------------------------------------
#endif