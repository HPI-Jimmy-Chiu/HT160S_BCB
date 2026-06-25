//---------------------------------------------------------------------------
#ifndef cmydefH
#define cmydefH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "MachineType.h"
//---------------------------------------------------------------------------
#define SERVER_MOTOR_POWER_ON_DELAY 10
#define TEST_MAX_BIN 999
//---------------------------------------------------------------------------
extern int CUSTOMER_CODE;
//AI(ht160s-statusbar) 20260624 : version + machine-identity globals (HT172 parity).
//MainVersion is the single source for the status-bar version panel; asModel/
//asHandlerID/asSerialNo mirror HT172 cmydef and are copied from GeneralSetting by
//UpdateMachineIdentity(). asModel is seeded "HT160S" (NOT "HT172").
extern AnsiString MainVersion;
extern AnsiString asModel;
extern AnsiString asHandlerID;
extern AnsiString asSerialNo;
//AI(ht160s-statusbar) 20260624 : copy GeneralSetting identity into the as* globals
//and write stbMain panels 1-3 (HT172 MyFunctionB::Update analog). Safe to call any
//time after GeneralSetting.Load() and fMain creation; NULL-guards the form/panels.
void UpdateMachineIdentity();
extern int MotorPowerOnDelay;
extern bool bMotorPowerState;
//AI(ht160s-maintainer) 20260617 : panel-LED run-state flags (DoPanelLamp);
//front-panel only port of HT172 bLamp* set.
extern bool bLampPowerOff;
extern bool bLampPowerOn;
extern bool bLampPause;
extern bool bLampStart;
extern bool bLampRetry;
extern bool bLampSkip;
extern bool bLampTrayEnd;
extern bool bLampTrayFeed;
extern bool bLampCleanOut;
extern bool bLampOneCycle;
extern bool bMotorHomePowerOn;
extern bool fAllMotorHome;
extern bool InitialOK;                 //AI(ht160s-initflow) 20260624 : whole-machine init-complete (ref HT9045)
void UpdateInitProgress(int iPercent);  //AI(ht160s-initflow) 20260624 : startup splash progress (def ht160s.cpp; called from HSys.Initial)
extern bool SoftStart;
extern bool SoftStop;
extern const int REALLY;
extern const int HAS_TRAY;
extern const int DUMMY;
extern const int EMPTY_IC;
extern const int UNCHECK_IC;
extern const int HAS_OK_IC;
extern bool bHomeByStart;
//AI(HT160S-Maintainer) 20260602 : HT172 0420 ProcessMotion lifecycle globals
extern bool bFirstRun;
extern bool bSortArmNeedHome;
//AI(HT160S-Maintainer) 20260616 : true while uHome ProcessMotorHome runs a
//motor-power Off->On recovery (clears latched servo-amp alarms). Suppresses
//CheckMotorPowerShutDown relay control + ScanAllMotorStatus alarm-forcing +
//Home-monitor auto-close so the power-cycle owns SwMotorRelay until done.
extern bool bHomePowerCycling;
//AI(HT160S-Maintainer) 20260616 : set true once the RS232 Pad panel has sent a
//valid status frame. Until then the panel Power On/Off sensors are stale
//defaults; CheckMotorPowerShutDown auto-energizes the motor relay (no interlock,
//per request) and only defers to the panel signals after the Pad has talked.
extern bool bPadEverCommunicated;
extern bool bCalculatePauseTime;
extern TDateTime tUPH_PauseTime;
extern TDateTime tUPH_PauseStartTime;
//---------------------------------------------------------------------------
enum eTrayName                  //使用的Harware Bin數量與名稱
{
    eBinNotUse      = 0,     //Bin未使用的位置
    eAuto1          = 1,
    eAuto2          = 2,
    eAuto3          = 3,
    eAuto4          = 4,
    eAuto5          = 5,
    eAuto6          = 6,
    eColor          = 7,
    eTrayCount
};
enum eSystemTime{stStartTime=0,
                 stPauseTime=1,
                 stPowerOn=2,
                 stProductTime=3,
                 stJamTime=4,
                 stContactTest=5,
                 stHomeTime=6,
                 stMTBA=7,
                 stTotalCnt};
//---------------------------------------------------------------------------
#endif