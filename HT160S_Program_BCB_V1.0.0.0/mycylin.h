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
extern class TMyCylinder Cylinder[MaxCylinderItem];
void InitialCylinderName();
//---------------------------------------------------------------------------
#endif
