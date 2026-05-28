//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "HCylinder.h"
#pragma package(smart_init)

AnsiString __fastcall HCylinder::GetOnSensorPort()   { return OnSensor->IOPort; }
AnsiString __fastcall HCylinder::GetOffSensorPort()  { return OffSensor->IOPort; };
AnsiString __fastcall HCylinder::GetCylinderPort()   { return IOPort->IOPort; };
void __fastcall HCylinder::SetOnSensorPort(AnsiString s)  { OnSensor->IOPort = s; };
void __fastcall HCylinder::SetOffSensorPort(AnsiString s) { OffSensor->IOPort = s; };
void __fastcall HCylinder::SetCylinderPort(AnsiString s)  { IOPort->IOPort = s; };
//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//
static inline void ValidCtrCheck(HCylinder *)
{
    new HCylinder(NULL);
}

//---------------------------------------------------------------------------
//  Cylinder建構子
//---------------------------------------------------------------------------
__fastcall HCylinder::HCylinder(TComponent* Owner)
    : TComponent(Owner)
{
    Alarm = new HAlarm(this);
    OnSensor = new HSensor(this);
    OffSensor = new HSensor(this);
    IOPort = new HSwitcher(this);
    
    iAlarmNo = 0;

    OnDelayTime  =
    OffDelayTime = 0;
    OnAlarmTime  =
    OffAlarmTime = 6789;
    TimerRestart();
}

//---------------------------------------------------------------------------
namespace Hcylinder
{
    void __fastcall PACKAGE Register()
    {
        TComponentClass classes[1] = {__classid(HCylinder)};
        RegisterComponents("HungKai", classes, 0);
    }
}

//---------------------------------------------------------------------------
//  計時器重置
//---------------------------------------------------------------------------
void HCylinder::TimerRestart()
{
    OnTimer.Clear();
    OnTimer.Set(OnDelayTime);

    OffTimer.Clear();
    OffTimer.Set(OffDelayTime);

    OnAlarmTimer.Clear();
    OnAlarmTimer.Set(OnAlarmTime);

    OffAlarmTimer.Clear();
    OffAlarmTimer.Set(OffAlarmTime);
}

//---------------------------------------------------------------------------
//  汽缸動作
//---------------------------------------------------------------------------
CYStatus HCylinder::On(void)
{
    OnTimer.Set(OnDelayTime);
    OffTimer.Set(OffDelayTime);
    OnAlarmTimer.Set(OnAlarmTime);
    OffAlarmTimer.Set(OffAlarmTime);

    *IOPort = ON;

    if( bManualSet )
        return CY_ON;

    if( SensorOn() )
    {
        OnAlarmTimer.Clear();
        OnTimer.On();
        if(OnTimer.Off())
            return CY_ON;
        else
            return CY_OFF;
    }
    else
        OnTimer.Clear();

    if(OnAlarmTime)
    {
        OnAlarmTimer.On();    
        if(OnAlarmTimer.Off())
        {
            Alarm->Set(AlarmNo);
            return CY_ALARM;
        }
    }
    return CY_OFF;
}
//---------------------------------------------------------------------------
//  汽缸收回
//---------------------------------------------------------------------------
CYStatus HCylinder::Off(void)
{
    OnTimer.Set(OnDelayTime);
    OffTimer.Set(OffDelayTime);
    OnAlarmTimer.Set(OnAlarmTime);
    OffAlarmTimer.Set(OffAlarmTime);

    *IOPort = OFF;

    if( bManualSet )
        return CY_ON;

    if (SensorOff())
    {
        OffAlarmTimer.Clear();
        OffTimer.On();
        if(OffTimer.Off())
            return CY_ON;
        else
            return CY_OFF;
    }
    else
        OffTimer.Clear();

    if (OffAlarmTime)
    {
        OffAlarmTimer.On();
        if (OffAlarmTimer.Off())
        {
            Alarm->Set(AlarmNo);
            return CY_ALARM;
        }
    }
    return CY_OFF;
}
//---------------------------------------------------------------------------
//  檢查Sensor是否ON
//---------------------------------------------------------------------------
bool HCylinder::SensorOn()
{
    if (HOnCheck != NULL)
        return HOnCheck(this);

    if (OnSensor->IOPort != "")
        return *OnSensor == ON;

    else if (OffSensor->IOPort != "")
        return *OffSensor == OFF;

    return true;
}
//---------------------------------------------------------------------------
//  檢查Sensor 是否OFF
//-----------------------------------------------------------------------
bool HCylinder::SensorOff()
{
    if (HOffCheck != NULL)
        return HOffCheck(this);

    if (OffSensor->IOPort != "")
        return *OffSensor == ON;

    else if (OnSensor->IOPort != "")
        return *OnSensor == OFF;

    return true;
}
//-----------------------------------------------------------------------
bool HCylinder::Status(void)
{
    return *IOPort==ON;
}
