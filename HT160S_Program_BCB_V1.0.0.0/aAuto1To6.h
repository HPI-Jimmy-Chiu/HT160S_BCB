//---------------------------------------------------------------------------
#ifndef aAuto1To6H
#define aAuto1To6H
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "HTimer.h"
#include "MotorAndIO/MyMotor.h"   //AI(HT160S-Maintainer) 20260604 : TMyCar stacking-car container
//---------------------------------------------------------------------------
class TTrayMotor;
class TMyCylinder;
class TMySensor;
//---------------------------------------------------------------------------
//AI(general) 20260608 : Stage1 demand-API sentinel - "this Auto wants no tray".
//Distinct from eTrayKind values (0=Normal,1=Identity,2=Cover); mirrors the -1
//that GetNextTrayKindForAuto already returns when a car is full.
#define eTrayReqNone (-1)
//---------------------------------------------------------------------------
struct TAutoStationState
{
    bool bCarHasTray;
    bool bRearHasTray;
    bool bRearCanUse;
    bool bFrontHasTray;
    bool bFullIC;
    bool bCleanOutFinish;
};
//---------------------------------------------------------------------------
class TAutoModule
{
private:
    TAutoStationState State[6];
    int FeedTask;
    int DischargeTask;
    int CleanOutTask;
    int DischargeSubTask;     //AI(general) 20260617 : FrontRise sub-step (shared by DoDischargeTray + TestGoUpOnce)
    int TestUpTask;           //AI(general) 20260617 : Teach Advanced single-cylinder GoUp-once test
    HTimer TestDelay;         //AI(general) 20260617 : Teach Advanced GoUp-once settle delay
    int iFeedAuto;
    int iDischargeAuto;
    bool bCleanOutCheck[6];
    //AI(ht160s-agv) 20260615 : per-Auto AMR/AGV handoff lock. While set, GetTrayRequest
    //refuses new trays (TrayArm stops feeding this Auto) and ServiceCarFull defers the
    //operator full-car modal to the AGV handshake. Set when a full car is handed to the
    //AGV; cleared on AGV finish (ClearAmrCar) or a home/init.
    bool bAmrLocked[6];
    //AI(general) 20260608 : Stage0 fix for TrayArm back-and-forth. Latches a
    //TrayArm-delivered rear tray so RefreshAutoState() cannot erase the logical
    //handshake when the physical rear sensor reads OFF (offline / sim-data run).
    //Cleared when the Auto consumes the rear tray (DoFeedTray 7000), discharges,
    //or cleans out. True only after SetRearHasTrayFromTrayArm/NotifyTrayArmDelivered.
    bool bRearDeliveredPending[6];
    int RearKind[6];      //AI(HT160S-Maintainer) 20260605 : AMR kind of tray TrayArm placed at rear
    int WorkingKind[6];   //AI(HT160S-Maintainer) 20260605 : AMR kind of tray now at the sort working position
    AnsiString RearTrayID[6];     //AI(HT160S-Maintainer) 20260608 : 2D TrayID of the identity tray TrayArm placed at rear (from Color CCD)
    AnsiString WorkingTrayID[6];  //AI(HT160S-Maintainer) 20260608 : 2D TrayID of the tray now at the working position
    HTimer FeedDelay;
    HTimer DischargeDelay;
    HTimer CleanOutDelay;

    bool IsSoftSimulate();
    TTrayMotor *GetAutoMotor(int Index);
    TTrayMotor *GetAutoVMotor(int Index);
    int GetAutoFeedY(int Index);
    int GetAutoDischargeY(int Index);
    int GetAutoFirstSortY(int Index);
    TMyCylinder *GetPush(int Index);
    TMyCylinder *GetLean(int Index);
    TMyCylinder *GetFrontRise(int Index);
    TMySensor *GetInputHasTray(int Index);
    TMySensor *GetInputFullTray(int Index);
    TMySensor *GetInputEndSensor(int Index);
    TMySensor *GetOutputHasTray(int Index);
    TMySensor *GetOutputBottomHasTray(int Index);
    bool MoveAutoY(int Index, int Position);
    void RefreshAutoState();
    void CheckAutoTray();
    int FindFeedAuto();
    int FindDischargeAuto();
    bool DoFeedTray(int Flag);
    bool DoDischargeTray(int Flag);
    bool DoAllAutoCleanOut(int Flag);
    bool DoFrontRiseOnce(int Index, int &SubTask, HTimer &Delay);   //AI(general) 20260617 : shared single-cylinder FrontRise On->settle->Off
    //AI(HT160S-Maintainer) 20260612 : AMR output-car full service. Sim auto-clears
    //the full car; real machine alarms + operator confirm then clears; physical
    //InputFullTray sensor is the last line of defense (alarm until it reads OFF).
    void ServiceCarFull();

public:
    TAutoModule();
    void InitialFlag(bool bKeepMaterial=false);
    void DoAuto(int &Task);
    int FindEmptyRearForTrayArm();
    bool IsRearHasTray(int Index);
    void SetRearHasTrayFromTrayArm(int Index, bool bHasTray);
    bool IsAllCleanOutFinish();  //AI(HT160S-Maintainer) 20260602 : expose for csystem CheckCleanOutFinish
    //AI(HT160S-Maintainer) 20260604 : stacking-car data containers for Auto1~6 (AMR pack source).
    TMyCar Car[6];
    TMyCar *GetAutoCar(int Index);          // NULL if out of range
    //AI(ht160s-agv) 20260615 : AMR/AGV output-car-full test for the SECS AGVSupplement
    //  trigger. Real machine = the SnAutoX_InputFullTray sensor (IsOn); simulation =
    //  a logical tray threshold (AMR_FULL_TRAY_SIM). Distinct from TMyCar::IsFull()
    //  (MAX_TRAY_PER_CAR book-keeping cap) on purpose.
    bool IsOutputCarFullForAmr(int Index);
    void InitAutoCarStack(int Index);       // set tray[0]=identity, tray[1]=cover, rest=normal
    //AI(HT160S-Maintainer) 20260605 : AMR stack-order support.
    int GetNextTrayKindForAuto(int Index);          // eTrayKind needed next, -1 if car full
    void NotifyTrayArmDelivered(int Index, int Kind, AnsiString TrayID); // TrayArm placed a tray of Kind (and identity 2D TrayID) at rear
    bool IsReadyForSortArmPlace(int Index);         // SortArm may fill working tray (Normal only in AMR)
    //AI(general) 20260608 : Stage1 demand API (Auto pulls trays on demand).
    //GetTrayRequest returns the eTrayKind this Auto wants at its rear now, or
    //eTrayReqNone(-1) when it wants none. FindTrayRequestAuto returns the first
    //requesting Auto and sets OutKind, or -1 when no Auto currently wants a tray.
    int GetTrayRequest(int Index);
    int FindTrayRequestAuto(int &OutKind);
    //AI(ht160s-agv) 20260615 : E87/AGV output-car handoff support (SECS coordinator).
    void SetAmrLock(int Index, bool bLock);   // lock/unlock TrayArm feed + modal defer
    bool IsAmrLocked(int Index);
    bool IsDrainedForAmr(int Index);           // Ready : no working/rear/full tray left (all GoUp to car)
    bool IsAmrTaken(int Index);                // Finish : AGV removed the car (sim=true; real sensor TBD)
    void ClearAmrCar(int Index);               // AGV finish : empty the car + re-seed stack + unlock
    //AI(ht160s-state-record-analysis) 20260616 : read-only state + working-tray cell map
    //for the Store Hangup SortArmDecision.txt (why SortArm cannot place / Auto cannot discharge).
    int        GetStationCount();
    AnsiString DescribeStation(int Index);
    AnsiString GetWorkingTrayID(int Index);  //AI(ht160s-motion-view) 20260618 : 2D TrayID at working pos for Unload Auto-info ID panel

    bool TestGoUpOnce(int Index, int Flag);   //AI(general) 20260617 : Teach Advanced single-cylinder FrontRise GoUp once (no GoDown)
};
//---------------------------------------------------------------------------
extern TAutoModule *AutoModule;
void InitializeAutoModule();
void ShutdownAutoModule();
//---------------------------------------------------------------------------
#endif