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
//  Color and Auto1..6 - all nine agreed with their software has-tray flag), so this is ONE
//  shared test rather than nine bespoke ones.
//  Returns  1 = gripping CONFIRMED
//           0 = confirmed NOT gripping (commanded out, but the On reed is dark)
//          -1 = NO VERDICT (sim, cylinder or On sensor disabled, or not commanded out at all)
//  -1 means "this test says nothing" and every caller must treat it as such : a disabled
//  point is never evidence that a tray is missing.
int GetClampGripVerdict(TMyCylinder *Push, bool bSoftSimulate);
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
