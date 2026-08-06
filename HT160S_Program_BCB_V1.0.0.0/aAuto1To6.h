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
//AI(ht160s-status) 20260703 : explicit per-station status (approved unified-status
//design, docs/plan/module-status-enum-design-20260703.md). Ladder-owned; SHADOW phase
//5a : written everywhere, read only by logging/UI until the flag readers are flipped
//one-per-commit after on-machine soak (5b). AS_IDLE is only set after the FULL
//discharge tail (case 6100, user decision) - stricter than the legacy flag clears.
enum eAutoStatus
{
    AS_IDLE=0,
    AS_REAR_STAGED,    //TrayArm delivered to the rear shelf; awaiting pull-in
    AS_LOADING,        //this station is iFeedAuto with DoFeedTray in flight
    AS_SORTING,        //working tray at sort position accepting IC
    AS_FULL,           //working tray full (or identity/cover in AMR); wants discharge
    AS_DISCHARGING,    //this station is iDischargeAuto with DoDischargeTray in flight
    AS_CLEANOUT_DONE,  //station drained + latched finished in CleanOut
};
//---------------------------------------------------------------------------
struct TAutoStationState
{
    int  Status;       //AI(ht160s-status) 20260703 : eAutoStatus (see enum note)
    bool bCarHasTray;
    bool bRearHasTray;
    bool bRearCanUse;
    bool bFrontHasTray;
    bool bFullIC;
    bool bCleanOutFinish;
    bool bResidueClear;   //AI(ht160s-residue) 20260624 : place-residue cleared on target Auto (gate discharge / AMR leave)
};
//---------------------------------------------------------------------------
class TAutoModule
{
private:
    TAutoStationState State[6];
    //AI(auto-per-station) 20260802 : the feed / discharge ladder cursors are now PER STATION.
    //They used to be module scalars, which meant every station-keyed read inside DoFeedTray /
    //DoDischargeTray went through the shared iFeedAuto / iDischargeAuto. That was only safe
    //because DoAuto is a linear phase ladder that serves exactly one station at a time - an
    //accidental property, and the thing that made DoFeedTray case 7000 able to stamp the WRONG
    //Car[] Kind / TrayID / CarID / iTrayCount (and therefore the wrong SECS DeviceCount on
    //CEID 272/273/274) the moment a second station ever overlapped. Parameterising the ladders
    //by station index removes that failure mode structurally.
    //Parallel [6] arrays rather than members of TAutoStationState, matching the convention this
    //module already uses for bAmrLocked[6] / RearKind[6] / RearGrid[6] (that struct holds only
    //the sensor booleans RefreshAutoState rewrites every pass).
    //AI(auto-per-station) 20260802 : per-station ladder PHASE, for the concurrent mode.
    //0 = this station has no ladder in flight; 2000 = feed running; 4000 = discharge
    //running. Mirrors the module ladder's own phase numbers so a State Record reads the
    //same either way. Stays 0 throughout while [Auto] Concurrency = 0 (legacy mode).
    int StationTask[6];
    int FeedTask[6];
    int DischargeTask[6];
    int DischargeSubTask[6];  //AI(general) 20260617 : FrontRise sub-step (per station since 20260802)
    int CleanOutTask;         //module-wide : DoAllAutoCleanOut is a six-station lockstep drain
    int TestUpTask;           //AI(general) 20260617 : Teach Advanced single-cylinder GoUp-once test
    HTimer TestDelay;         //AI(general) 20260617 : Teach Advanced GoUp-once settle delay
    //AI(auto-per-station) 20260802 : DISPATCH cursors only - they record which station the
    //serial DoAuto ladder picked for this lap. NOTHING inside DoFeedTray / DoDischargeTray
    //reads them any more; those take the station as a parameter. Keep it that way: a ladder
    //that reads a shared cursor is exactly the bug this change removed.
    int iFeedAuto;
    int iDischargeAuto;
    bool bCleanOutCheck[6];
    //AI(cleanout) 20260706 : per-episode log-once latch for the EventLog-only residual
    //watchdog (drain latched but a station still shows a physical tray). Reset in InitialFlag.
    bool bCleanOutResidualLogged[6];
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
    bool bDischargeTailPending[6];   //AI(ht160s-home-resume-drain) 20260713 : AD-1 - HOME landed in the discharge eject tail (DischargeTask 5000-6100); finish MoveAutoY-to-feed + FrontRise on resume (cleared on COLD init only)
    int RearKind[6];      //AI(HT160S-Maintainer) 20260605 : AMR kind of tray TrayArm placed at rear
    int WorkingKind[6];   //AI(HT160S-Maintainer) 20260605 : AMR kind of tray now at the sort working position
    int iAmrDeviceCount[6];   //AI(ht160s-agv-devicecount) 20260713 : per-Auto running IC total for the SECS DeviceCount SVID. Tallied at discharge from the populated working tray (Car.Tray grids are never filled with IC data); reset in InitAutoCarStack, so a keep-material HOME (which skips it) preserves the count.
    AnsiString RearTrayID[6];     //AI(HT160S-Maintainer) 20260608 : 2D TrayID of the identity tray TrayArm placed at rear (from Color CCD)
    AnsiString WorkingTrayID[6];  //AI(HT160S-Maintainer) 20260608 : 2D TrayID of the tray now at the working position
    TMyTray RearGrid[6];          //AI(ht160s-tray-source) : per-cell grid TrayArm staged at rear (copied into working tray at DoFeedTray c7000)
    HTimer FeedDelay[6];        //AI(auto-per-station) 20260802 : per-station settle timer
    HTimer DischargeDelay[6];   //AI(auto-per-station) 20260802 : per-station settle timer
    HTimer CleanOutDelay;       //module-wide, with CleanOutTask

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
    TMySensor *GetOutputBottomHasTray(int Index);
    bool MoveAutoY(int Index, int Position);
    void RefreshAutoState();
    void CheckAutoTray();
    int FindFeedAuto();
    int FindDischargeAuto();
    //AI(auto-per-station) 20260802 : Index is the station this ladder drives. Every field it
    //touches is keyed on Index, so two stations can never stamp each other's Car[].
    bool DoFeedTray(int Index, int Flag);
    bool DoDischargeTray(int Index, int Flag);
    //AI(auto-per-station) 20260802 : non-mutating per-station feed predicate. FindFeedAuto()
    //cannot be used for a re-validation because it REWRITES State[*].bRearCanUse for all six
    //stations as a side effect; this asks the same question without touching anything.
    bool IsFeedEligible(int Index);
    //AI(auto-per-station) 20260802 : the FindDischargeAuto terms for ONE station, without
    //the first-match scan. Same three gates, so per-station mode admits exactly the
    //stations the legacy scan would have admitted - just without making them queue.
    bool IsDischargeEligible(int Index);
    //AI(auto-per-station) 20260802 : step every station's own ladder once. Returns true
    //when NO station has a ladder in flight - that is the hand-over condition CleanOut
    //waits on. bNoNewJobs starts nothing new and only drains what is already running.
    bool ServiceStations(bool bNoNewJobs);
    bool DoAllAutoCleanOut(int Flag);
    bool AllStationsDrainLatched();                 //AI(cleanout) 20260706 : pure per-station drain latch (DoAuto stop-gate)
    void ServiceCleanOutResidualWatchdog();         //AI(cleanout) 20260706 : EventLog-only residual notice (log-once per episode)
    bool DoFrontRiseOnce(int Index, int &SubTask, HTimer &Delay);   //AI(general) 20260617 : shared single-cylinder FrontRise On->settle->Off
    //AI(HT160S-Maintainer) 20260612 : AMR output-car full service. Sim auto-clears
    //the full car; real machine alarms + operator confirm then clears; physical
    //InputFullTray sensor is the last line of defense (alarm until it reads OFF).
    void ServiceCarFull();

public:
    TAutoModule();
    void InitialFlag(bool bKeepMaterial=false);
    bool HomeDrainTick();            //AI(ht160s-home-resume-drain) 20260711 : stand-in execute the FeedTask 6000/7000 single-scan commit (AF-2) so the clamped tray is never software-blind
    void PauseTimeoutTimers();     //AI(ht160s-actuator-timer) 20260627 : actuator-timer enrollment stub (Auto has no wall-clock timeout window since the AmrFullWaitTimer removal)
    void ReStartTimeoutTimers();   //AI(ht160s-actuator-timer) 20260627 : enrollment stub (see PauseTimeoutTimers)
    void DoAuto(int &Task);
    int FindEmptyRearForTrayArm();
    bool IsRearHasTray(int Index);
    bool IsRearPlacedButUnsigned(int Index);   //AI(ht160s-home-resume-drain) 20260713 : XS-1/TA-2 - rear physically holds a tray (raw sensor) NOT signed as delivered (bRearDeliveredPending==false); TrayArm adopt-as-delivered discriminator
    void SetRearHasTrayFromTrayArm(int Index, bool bHasTray);
    void SetPlaceResidueClear(int Index, bool bClear);   //AI(ht160s-residue) 20260624 : SortArm reports place-residue result for the target Auto
    bool IsAllCleanOutFinish();  //AI(HT160S-Maintainer) 20260602 : expose for csystem CheckCleanOutFinish
    //AI(HT160S-Maintainer) 20260604 : stacking-car data containers for Auto1~6 (AMR pack source).
    TMyCar Car[6];
    TMyCar *GetAutoCar(int Index);          // NULL if out of range
    int GetCarTrayCount(int Index);         //AI(ht160s-agv) 20260624 : trays stacked on the output car (PanelMain6 Motion View header); 0 if out of range
    int GetAmrDeviceCount(int Index);       //AI(ht160s-agv-devicecount) 20260713 : running IC total on the output car (SECS DeviceCount SVID source); 0 if out of range
    //AI(ht160s-agv) 20260615 : AMR/AGV output-car-full test for the SECS AGVSupplement
    //  trigger. Real machine = the SnAutoX_InputFullTray sensor (IsOn); simulation =
    //  a logical tray threshold (AMR_FULL_TRAY_SIM). Distinct from TMyCar::IsFull()
    //  (MAX_TRAY_PER_CAR book-keeping cap) on purpose.
    bool IsOutputCarFullForAmr(int Index);
    void InitAutoCarStack(int Index);       // set tray[0]=identity, tray[1]=cover, rest=normal
    //AI(HT160S-Maintainer) 20260605 : AMR stack-order support.
    int GetNextTrayKindForAuto(int Index);          // eTrayKind needed next, -1 if car full
    void NotifyTrayArmDelivered(int Index, int Kind, AnsiString TrayID); // TrayArm placed a tray of Kind (and identity 2D TrayID) at rear
    void StageRearGrid(int Index, const TMyTray &Grid);   //AI(ht160s-tray-source) : TrayArm hands the carried grid to the rear staging slot (AMR + Normal)
    bool IsReadyForSortArmPlace(int Index);
    int  GetCarTrayGripVerdict(int Index);   //AI(ht160s-clampgrip) 20260806 : physical PushTray grip verdict for the working car (1=gripping 0=tray gone -1=no verdict); SortArm place-Z boundary confirm
    TMySensor *GetCarTrayPushOnSensor(int Index);   //AI(ht160s-clampgrip) 20260806 : the grip reed, so the alarm screen can name the real IO point         // SortArm may fill working tray (Normal only in AMR)
    //AI(general) 20260608 : Stage1 demand API (Auto pulls trays on demand).
    //GetTrayRequest returns the eTrayKind this Auto wants at its rear now, or
    //eTrayReqNone(-1) when it wants none. FindTrayRequestAuto returns the first
    //requesting Auto and sets OutKind, or -1 when no Auto currently wants a tray.
    int GetTrayRequest(int Index);
    //AI(ht160s-amr-divert) 20260719 : WantKind (default eTrayReqNone=any) narrows the
    //scan to Autos requesting exactly that kind - used by the TrayArm recovery divert
    //to target only Autos wanting a plain Normal tray. Default = legacy behavior.
    int FindTrayRequestAuto(int &OutKind, int WantKind=eTrayReqNone);
    int GetStationStatus(int Index);   //AI(ht160s-status) 20260703 : eAutoStatus for stbMain display / peers
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
    AnsiString DescribeModule();   //AI(auto-obsv) 20260801 : the six-station SHARED cursors (dumped nowhere before)
    AnsiString GetWorkingTrayID(int Index);  //AI(ht160s-motion-view) 20260618 : 2D TrayID at working pos for Unload Auto-info ID panel

    bool TestGoUpOnce(int Index, int Flag);   //AI(general) 20260617 : Teach Advanced single-cylinder FrontRise GoUp once (no GoDown)
};
//---------------------------------------------------------------------------
extern TAutoModule *AutoModule;
void InitializeAutoModule();
void ShutdownAutoModule();
//---------------------------------------------------------------------------
#endif