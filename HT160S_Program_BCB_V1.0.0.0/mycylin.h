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
