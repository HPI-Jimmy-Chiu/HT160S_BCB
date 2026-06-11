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
    int iFeedAuto;
    int iDischargeAuto;
    bool bCleanOutCheck[6];
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

public:
    TAutoModule();
    void InitialFlag();
    void DoAuto(int &Task);
    int FindEmptyRearForTrayArm();
    bool IsRearHasTray(int Index);
    void SetRearHasTrayFromTrayArm(int Index, bool bHasTray);
    bool IsAllCleanOutFinish();  //AI(HT160S-Maintainer) 20260602 : expose for csystem CheckCleanOutFinish
    //AI(HT160S-Maintainer) 20260604 : stacking-car data containers for Auto1~6 (AMR pack source).
    TMyCar Car[6];
    TMyCar *GetAutoCar(int Index);          // NULL if out of range
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
};
//---------------------------------------------------------------------------
extern TAutoModule *AutoModule;
void InitializeAutoModule();
void ShutdownAutoModule();
//---------------------------------------------------------------------------
#endif