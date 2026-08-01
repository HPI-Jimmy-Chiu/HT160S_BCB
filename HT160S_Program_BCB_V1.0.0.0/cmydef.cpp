//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "cmydef.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
int CUSTOMER_CODE=HT160S_DEFAULT_CUSTOMER_CODE;
//AI(ht160s-statusbar) 20260624 : version string (status-bar panel 0) + machine
//identity globals (status-bar panels 1-3). asModel seeded HT160S; asHandlerID/
//asSerialNo default empty and are overwritten by UpdateMachineIdentity() from
//GeneralSetting once it is loaded.
AnsiString MainVersion="HT160S 1.0.0.0";
AnsiString asModel="HT160S";
AnsiString asHandlerID="";
AnsiString asSerialNo="";
int MotorPowerOnDelay=SERVER_MOTOR_POWER_ON_DELAY;
bool bMotorPowerState=false;
//AI(ht160s-maintainer) 20260617 : panel-LED run-state flags (DoPanelLamp).
bool bLampPowerOff=false;
bool bLampPowerOn=false;
bool bLampPause=false;
bool bLampStart=false;
bool bLampRetry=false;
bool bLampSkip=false;
bool bLampTrayEnd=false;
bool bLampTrayFeed=false;
bool bLampCleanOut=false;
bool bLampOneCycle=false;
bool bMotorHomePowerOn=false;
bool fAllMotorHome=false;
//AI(ht160s-initflow) 20260624 : whole-machine init-complete flag (ref HT9045 InitialOK).
//false until WinMain startup finishes; background timers / MainProc no-op while false.
bool InitialOK=false;
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
bool bHomePowerCycling=false;
bool bPadEverCommunicated=false;
bool bCalculatePauseTime=false;
TDateTime tUPH_PauseTime;
TDateTime tUPH_PauseStartTime;
//AI(secs-onsite0731) 20260801 : UPH numerator baseline. tRunData.TotalIC is deliberately
//KEPT across power cycles (main.cpp ReadLastDataIni, "user choice") while tRunData.StartTime
//is re-stamped on the first production cycle of every session (csystem.cpp bFirstRun block).
//Dividing one by the other is meaningless : on 2026-07-31 SVID 1021 reported 2026 UPH for a
//window in which 17 units were placed (truth 99, i.e. 20x over). This latches TotalIC at the
//same instant StartTime is re-stamped so the ratio covers ONE window. Lives here beside
//bFirstRun/tUPH_PauseTime because both csystem.cpp and main.cpp need it - a file-scope static
//in csystem.cpp would not link.
int g_iUphBaseIC=0;
//AI(secs-onsite0731) 20260801 : mirror of the main-screen status caption for SVID 1011
//"Machine State" (9045 binds its SVID 1011 straight to fMain->palMainStatus). Written at the
//single writer SetMainStatus() so the SECS serialize path never reads a VCL property, and so
//it keeps working if palMainStatus is renamed or the bilingual palMainStatus_En diverges.
AnsiString g_sMachineStateText="";
//---------------------------------------------------------------------------