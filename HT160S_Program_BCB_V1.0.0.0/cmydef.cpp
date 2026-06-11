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
const int EMPTY_IC=0;
const int UNCHECK_IC=1;
const int HAS_OK_IC=2;
bool bHomeByStart=false;
//AI(HT160S-Maintainer) 20260602 : HT172 0420 ProcessMotion lifecycle globals
bool bFirstRun=true;
bool bSortArmNeedHome=false;
bool bCalculatePauseTime=false;
TDateTime tUPH_PauseTime;
TDateTime tUPH_PauseStartTime;
//---------------------------------------------------------------------------