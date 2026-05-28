//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "cmydef.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
int CUSTOMER_CODE=HT160S_DEFAULT_CUSTOMER_CODE;
int MotorPowerOnDelay=SERVER_MOTOR_POWER_ON_DELAY;
bool bMotorPowerState=false;
bool bMotorHomePowerOn=false;
bool fAllMotorHome=false;
bool SoftStart=false;
bool SoftStop=false;
const int REALLY=2;
const int HAS_TRAY=1;
const int DUMMY=0;
bool bHomeByStart=false;
//---------------------------------------------------------------------------