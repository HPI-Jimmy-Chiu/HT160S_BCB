//---------------------------------------------------------------------------
#ifndef mycylinH
#define mycylinH
//---------------------------------------------------------------------------
#include <Controls.hpp>
#include "HTimer.h"
#include "myswitch.h"
#include "mysensor.h"
//---------------------------------------------------------------------------
#define MaxCylinderItem 200
//---------------------------------------------------------------------------
enum eCynMotion{eOffNotOnErr    =0,
                eOffNotOffErr   =1,
                eOffIsOnErr     =2,
                eOnNotOnErr     =3,
                eOnNotOffErr    =4,
                eOnIsOnErr      =5,
                eCynErrTotal
               };
//---------------------------------------------------------------------------
class TMyCylinder
{
private:
    int Task;
    int iLastDir;   //AI(ht160s-cylinder-dir) 20260731 : 0=none 1=push 2=pop. Task is SHARED by Push() and Pop(); a direction change must re-arm it or the confirm+watchdog are skipped.
    int OnOff;
    int iOnLeft;
    int iOnTop;
    int iOffLeft;
    int iOffTop;

protected:
    TControl *PTempWinCtrl;

public:
    __fastcall TMyCylinder();

    TMySensor OnSensor;
    TMySensor OffSensor;
    TMySwitch Switch;

    AnsiString CylinderName;
    AnsiString OnSensorName;
    AnsiString OffSensorName;
    AnsiString FlushPanelName;
    AnsiString ErrorName[eCynErrTotal];
    HTimer Delay;
    bool Enable;
    bool EnableAtDataBase;
    bool bInitialOk;
    int OnAlarmCode;
    int OffAlarmCode;
    int OnAlarmTime;
    int OffAlarmTime;
    int OnDelayTime;
    int OffDelayTime;
    int Tag;
    //AI(ht160s-clamp-geom) 20260916 : is this one of the ten MotorY tray clamps
    //(the C_*_PushTray cylinders) ? Set by database.cpp LoadCylinderParameterFromDataBase.
    //TrayArm's two clamps are excluded : they have no _Off reed at all
    //(system/IO_Table.csv:212-213, addresses blank and Enable=0).
    bool bTrayClamp;
    //AI(ht160s-clamp-geom) 20260916 : carriage identity in uHome.cpp HomeParkCarriage
    //order (0/1=Loader1/2, 2..7=Auto1..6, 8=Empty, 9=Color). -1 = not a tray clamp.
    //Range-checked by IsClampNewGeometry so a cylinder outside the ten can never take
    //the new reading; kept as a stable identity for diagnostics and future per-car work.
    int iClampGeomIdx;
    //AI(clamp-review-B) 20260916 : did this stroke ever see the piston ON the retracted
    //seat ? A NEW-geometry clamp confirms by DEPARTURE, so a seat reed stuck dark would
    //otherwise read as "already departed" in the same scan the coil is energised - the
    //cylinder would be reported as clamped, and holding a tray, without moving at all.
    bool bSeatConfirmed;

    bool Push();
    bool Pop();
    void On();
    void Off();
    bool Reset();
    bool IsOn();
    bool IsOff();
    bool GetOutBit();
    void SetSimulateCompoment(TObject *PCtrl, TAnchorKind Alignment, int simuStart, int simuEnd);
    void UpdateSimulateCompomentPosition(bool bFlag);
};
//---------------------------------------------------------------------------
void InitialCylinderName();
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260623 : shared cylinder/sensor "ready" predicates,
//  promoted from aAuto1To6.cpp so Empty/Loader/Auto share one definition.
//  bSoftSimulate forces ready (sim build has no real IO).
bool IsSensorOnReady(TMySensor *Sensor);
bool IsCylinderOnReady(TMyCylinder *Cylinder, bool bSoftSimulate);
//AI(ht160s-clampgrip) 20260806 : "is this clamp REALLY gripping a tray right now ?"
//  On-site 2026-08-05 : a tray jumped out of a Loader-Y carriage clamp while the software
//  still believed the carriage held it. User's rule, and the mechanism behind it : the
//  PushTray On reed is the ground truth for a gripped tray. The cylinder stops against the
//  tray edge, so the reed only lights with a tray between the hook and the stop; if the tray
//  escapes, the cylinder over-travels and the reed goes dark while the out-bit stays on.
//  Every tray carriage on the machine has a C_*_PushTray whose On sensor is Enable=1 in the
//  in-force IO_Table (verified against the 2026-08-05 IoDetail sweep for Loader1/2, Empty,
//  Color and Auto1..6 - all ten agreed with their software has-tray flag), so this WAS ONE
//  shared test rather than nine bespoke ones.
//  Returns  1 = a tray IS in the clamp
//           0 = confirmed NOT holding a tray
//          -1 = NO VERDICT (sim, cylinder or reed disabled, or not commanded out at all)
//  -1 means "this test says nothing" and every caller must treat it as such : a disabled
//  point is never evidence that a tray is missing.
//AI(ht160s-clamp-geom) 20260916 : RENAMED from GetClampGripVerdict, deliberately.
//  The 2026-09 mechanical rework INVERTS the reed reading on a reworked carriage, and
//  every reader tests only ==0 / !=0, so the compiler could not have caught a missed
//  call site. The rename forces each one to be looked at. Geometry per carriage :
//    OLD : _On lit = a tray (hook stopped on the tray edge) ; dark = over-travelled, empty
//    NEW : _On lit = EMPTY (piston reached the end stop) ; both reeds dark = a tray
//  Which reading applies comes from GeneralSetting.bTwoBandClamp, ONE switch covering
//  all ten clamps; its default 0 keeps the OLD reading on every carriage.
int GetTrayClampVerdict(TMyCylinder *Push, bool bSoftSimulate);
//AI(ht160s-clamp-geom) 20260916 : is this carriage on the reworked reed geometry ?
//  False for every unknown index, so a point we cannot place always answers "old".
bool IsClampNewGeometry(int iGeomIdx);
//AI(ht160s-clamp-geom) 20260916 : "has the piston LEFT the retracted seat ?" - the only
//  positive motion evidence a NEW-geometry tray clamp still has, because a loaded clamp
//  lights neither reed. Guard shape copied from IsCylinderOnReady : NULL -> false, a
//  point we cannot read -> true (a disabled reed must never block a ladder).
bool IsTrayClampDeparted(TMyCylinder *Cylinder);
//AI(HT160S-Maintainer) 20260623 : standardized dual-cylinder tray clamp
//  (lean-stop first, push last). SettleTicks>0 adds settle+OnSensor confirm
//  +Pop-on-miss; ==0 skips it. Returns 0=running, 1=clamped, 2=push miss.
//  SubTask caller-owned (init 0); Delay caller-owned settle timer.
int DoClampTray(TMyCylinder &Lean, TMyCylinder &Push, int &SubTask,
                HTimer &Delay, bool bSoftSimulate, int SettleMs);
//AI(ht160s-maintainer) 20260625 : short-term mechanical interlock. The Empty and
//  Loader front separate-tray cylinders clash if both extend at once. Returns true
//  when MY side must wait because the OTHER side's output is commanded out. Gated by
//  General.ini [Safety] FrontSeparateInterlock (default on); set 0 after rework.
bool IsFrontSeparateBlockedBy(TMyCylinder &Other);
//---------------------------------------------------------------------------
#endif
