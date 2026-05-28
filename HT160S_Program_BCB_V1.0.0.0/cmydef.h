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
extern bool bHomeByStart;
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