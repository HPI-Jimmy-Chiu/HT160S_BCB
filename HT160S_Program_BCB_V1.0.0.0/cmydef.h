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
extern int MotorPowerOnDelay;
extern bool bMotorPowerState;
extern bool bMotorHomePowerOn;
extern bool fAllMotorHome;
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