//---------------------------------------------------------------------------
#ifndef HCylinderH
#define HCylinderH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include "htimer.h"
#include "hsensor.h"
#include "hswitcher.h"
#include "halarm.h"

typedef enum { CY_OFF=0,CY_ON,CY_ALARM } CYStatus;

typedef bool __fastcall (__closure *HCylinderEvent)(System::TObject* Sender);

//---------------------------------------------------------------------------
class MyTimer{
private:
	 unsigned int   Tick,LastTick,ms;
public:
	 bool           AutoReset;
	 __fastcall     MyTimer();
	 void           Restart(void);
	 void           SetOnDelayTime(unsigned int tms);
	 bool           On(void);
	 unsigned int   Time(unsigned int);
};

//---------------------------------------------------------------------------
class PACKAGE HCylinder : public TComponent
{
private:
    HTimer      OnTimer;
    HTimer      OffTimer;
    HTimer      OnAlarmTimer;
    HTimer      OffAlarmTimer;

protected:
    int         iAlarmNo;
    HSensor     *OnSensor;
    HSensor     *OffSensor;
    HSwitcher   *IOPort;
    HCylinderEvent  HOnCheck;
    HCylinderEvent  HOffCheck;
    AnsiString __fastcall GetOnSensorPort();
    AnsiString __fastcall GetOffSensorPort();
    AnsiString __fastcall GetCylinderPort();

    void __fastcall SetOnSensorPort(AnsiString s);
    void __fastcall SetOffSensorPort(AnsiString s);
    void __fastcall SetCylinderPort(AnsiString s);

public:
    HAlarm      *Alarm;
    unsigned int OnDelayTime,OffDelayTime,OnAlarmTime,OffAlarmTime;
    __fastcall  HCylinder(TComponent* Owner);

	void       TimerRestart();

	CYStatus   On(void);
	CYStatus   Off(void);
	bool       SensorOn(void);
    bool       SensorOff(void);
    bool       Status(void);  
    bool       bManualSet;

__published:
    __property  int AlarmNo = { read = iAlarmNo,write = iAlarmNo };
    __property  AnsiString OnSensorPort =
        { read = GetOnSensorPort,write = SetOnSensorPort };
    __property  AnsiString OffSensorPort =
        { read = GetOffSensorPort,write = SetOffSensorPort };
    __property  AnsiString CylinderPort =
        { read = GetCylinderPort,write = SetCylinderPort };
    __property  bool PManualSet =
        { read = bManualSet,write = bManualSet,default = false };
    __property  HCylinderEvent OnCheck =
        { read = HOnCheck,write = HOnCheck };
    __property  HCylinderEvent OffCheck =
        { read = HOffCheck,write = HOffCheck };
};
//---------------------------------------------------------------------------
#endif
