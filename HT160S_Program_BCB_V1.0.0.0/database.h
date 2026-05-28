//---------------------------------------------------------------------------
#ifndef databaseH
#define databaseH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "MachineType.h"
#include "MotorAndIO/MyMotor.h"
#include "mycylin.h"
#include "MyKitSuck.h"
//---------------------------------------------------------------------------
enum eIOType{eMotionNet=0,
             eISABase  =1,
             ePCI1735U =2,
             ePCI1203  =3,
             e_PLCbase  =4
             };
//---------------------------------------------------------------------------
typedef struct TIODATA
{
    int Tag;
    AnsiString _CommaText;
    AnsiString Type;
    AnsiString Alias;
    AnsiString IOPos;
    AnsiString sIP;
    int iLane;
    int iModuleType;
    int iIP;
    int iPort;
    int iBit;
    int iInType;
    int iISABase;
    int iEnable;
    int iOnAlarmTime;
    int iOffAlarmTime;
    int iOnDelayTime;
    int iOffDelayTime;
    bool bDelete;

    TIODATA(AnsiString Str=AnsiString(""));
}TIODATA;
//---------------------------------------------------------------------------
typedef struct TIOTABLENO
{
    int eioType;
    int eioAlias;
    int eioLane;
    int eioModuleType;
    int eioIP;
    int eioPort;
    int eioBit;
    int eioInType;
    int eioISABase;
    int eioEnable;
    int eioOnAlarmTime;
    int eioOffAlarmTime;
    int eioOnDelayTime;
    int eioOffDelayTime;
    int eioNote;
    int eioTotal;

    TIOTABLENO();
    int SetIOTableNo(AnsiString Str);
}TIOTABLENO;
//---------------------------------------------------------------------------
typedef struct TMOTDATA
{
    AnsiString _CommaText;
    AnsiString No;
    AnsiString Alias;
    int iEnable;
    int iBoardID;
    int iPort;
    double dGearRatio;
    int iSoftLimitN;
    int iSoftLimitP;
    AnsiString CardModel;
    int MotorKind;
    AnsiString FlushPanel;
    AnsiString HomeOrder;
    int iDirection;
    int iHomeDirectior;
    double dAcc;
    double dDec;
    int iInitSpeed;
    int iHomeHighSpeed;
    int iHomeLowSpeed;
    int iJogHighSpeed;
    int iJogLowSpeed;
    int iRange;
    int iRate;
    int iServoAlarmOn;
    int i1P2P;
    int iSensorType;
    int iLimitLogic;
    int iIn1Logic;
    int iSimulateSpeed;

    TMOTDATA(AnsiString Str=AnsiString(""));
}TMOTDATA;
//---------------------------------------------------------------------------
typedef struct TMOTNO
{
    int emotNo;
    int emotAlias;
    int emotDirection;
    int emotGearRatio;
    int emotHomeDirectior;
    int emotHomeHighSpeed;
    int emotHomeLowSpeed;
    int emotInitSpeed;
    int emotJogHighSpeed;
    int emotJogLowSpeed;
    int emotRate;
    int emotSoftLimitN;
    int emotSoftLimitP;
    int emotEnable;
    int emotServoAlarmOn;
    int emotRange;
    int emot1P2P;
    int emotSensorType;
    int emotSimulateSpeed;
    int emotCardModel;
    int emotBoardID;
    int emotPort;
    int emotAcc;
    int emotDec;
    int emotMotorKind;
    int emotFlushPanel;
    int emotHomeOrder;
    int emotLimitLogic;
    int emotIn1Logic;
    int emotTotal;

    TMOTNO();
    int SetMOTTableNo(AnsiString Str);
}TMOTNO;
//---------------------------------------------------------------------------
typedef struct MOTOR_MODULAR_STRUCT
{
    TTrayMotor *MSortingArmX;
    TTrayMotor *MTrayArmX;
    TTrayMotor *MEmptyY;
    TTrayMotor *MLoaderY_1;
    TTrayMotor *MLoaderY_2;
    TTrayMotor *MAutoY_1;
    TTrayMotor *MAutoY_2;
    TTrayMotor *MAutoY_3;
    TTrayMotor *MAutoY_4;
    TTrayMotor *MAutoY_5;
    TTrayMotor *MAutoY_6;
    TTrayMotor *MTopCCDX;
    TTrayMotor *MBottomCCDY;
    TTrayMotor *MSuckZ_1;
    TTrayMotor *MSuckZ_2;
    TTrayMotor *MSuckZ_3;
    TTrayMotor *MSuckZ_4;
    TTrayMotor *MPitchX;
}MOTOR_MODULAR;
//---------------------------------------------------------------------------
typedef struct VIRTUAL_MOTOR_MODULAR_STRUCT
{
    TTrayMotor *MMSortingArmX;
    TTrayMotor *MMTrayArmX;
    TTrayMotor *MMEmptyY;

    TTrayMotor *MMLoaderY_1;
    TTrayMotor *MMLoaderY_2;

    TTrayMotor *MMAutoY_1;
    TTrayMotor *MMAutoY_2;
    TTrayMotor *MMAutoY_3;
    TTrayMotor *MMAutoY_4;
    TTrayMotor *MMAutoY_5;
    TTrayMotor *MMAutoY_6;

    TTrayMotor *MMSuck_1;
    TTrayMotor *MMSuck_2;
    TTrayMotor *MMSuck_3;
    TTrayMotor *MMSuck_4;
}VIRTUAL_MOTOR_MODULAR;
//---------------------------------------------------------------------------
enum RunModeEnum
{
    Run_Normal      =0,
    Run_Home        =1,
    Run_OneCycle    =2,
    Run_CleanOut    =3,
    Run_TrayFeed    =4,
    Run_Scan        =5,
    Run_Check       =6
};
//---------------------------------------------------------------------------
typedef struct SYSTEM_STATUS_STRUCT
{
    bool SystemStart;
    RunModeEnum RunMode;
}SYSTEM_STATUS;
//---------------------------------------------------------------------------
typedef struct SENSOR_MODULAR_STRUCT
{
    TMySensor SnFKPowerOff;
    TMySensor SnFKPowerOn;
    TMySensor SnRKPowerOff;
    TMySensor SnRKPowerOn;
    TMySensor SnSafeLock;
    TMySensor SnMotorPower;
    TMySensor SnAirIsEnough;
    TMySensor SnIonFan_Balance;
    TMySensor SnIonFan_Power;
    TMySensor SnEMG;
    TMySensor SnEMG_1;
    TMySensor SnEMG_2;
    TMySensor SnEMG_3;
    TMySensor SnEMG_4;
    TMySensor SnSafeDoorFront;
    TMySensor SnSafeDoorRight;
    TMySensor SnSafeDoorLeft;
    TMySensor SnSafeSlideDoorRight;
    TMySensor SnSafeSlideDoorLeft;
    TMySensor SnSafeAuto6;
    TMySensor SnColor_InputHasTray;
    TMySensor SnColor_InputFullTray;
    TMySensor SnColor_TrayPos1;
    TMySensor SnColor_OutputBottomHasTray;
    TMySensor SnFrontPadActive;
    TMySensor SnFKReset;
    TMySensor SnFKPause;
    TMySensor SnFKHome;
    TMySensor SnFKStart;
    TMySensor SnFKOneCycle;
    TMySensor SnFKRetry;
    TMySensor SnFKSkip;
    TMySensor SnFKCleanOut;
    TMySensor SnFKTrayFeed;
    TMySensor SnFKTrayEnd;
    TMySensor SnFKAlarmReset;
    TMySensor SnRearPadActive;
    TMySensor SnRKReset;
    TMySensor SnRKPause;
    TMySensor SnRKHome;
    TMySensor SnRKStart;
    TMySensor SnRKOneCycle;
    TMySensor SnRKRetry;
    TMySensor SnRKSkip;
    TMySensor SnRKCleanOut;
    TMySensor SnRKTrayFeed;
    TMySensor SnRKTray;
    TMySensor SnRKTrayEnd;
    TMySensor SnRKAlarmReset;
    TMySensor SnRKManualStep;
    TMySensor SnRKManualTStart;
    TMySensor SnRKSafeLock;
}SENSOR_MODULAR;
//---------------------------------------------------------------------------
typedef struct CYLINDER_MODULAR_STRUCR
{
    TMyCylinder C_TrayArmZ_Up;
    TMyCylinder C_TrayArmZ_Down;
    TMyCylinder C_TrayArm_FrontClamp;
    TMyCylinder C_TrayArm_RearClamp;

    TMyCylinder C_Empty_FrontRiseTray_1;
    TMyCylinder C_Empty_FrontRiseTray_2;
    TMyCylinder C_Empty_PushTray;
    TMyCylinder C_Empty_LeanOnTray;
    TMyCylinder C_Empty_FrontSeparateTray_1;
    TMyCylinder C_Empty_RearRiseTray;
    TMyCylinder C_Empty_RearSeparateTray_1;

    TMyCylinder C_Loader_FrontRiseTray_1;
    TMyCylinder C_Loader_FrontRiseTray_2;
    TMyCylinder C_Loader1_PushTray;
    TMyCylinder C_Loader2_PushTray;
    TMyCylinder C_Loader1_LeanOnTray;
    TMyCylinder C_Loader2_LeanOnTray;
    TMyCylinder C_Loader_FrontSeparateTray_1;
    TMyCylinder C_Loader_RearRiseTray;

    TMyCylinder C_Auto1_FrontRiseTray;
    TMyCylinder C_Auto1_PushTray;
    TMyCylinder C_Auto1_LeanOnTray;
    TMyCylinder C_Auto1_RearRiseTray;
    TMyCylinder C_Auto1_FrontSeparateTray_1;

    TMyCylinder C_Auto2_FrontRiseTray;
    TMyCylinder C_Auto2_PushTray;
    TMyCylinder C_Auto2_LeanOnTray;
    TMyCylinder C_Auto2_RearRiseTray;
    TMyCylinder C_Auto2_FrontSeparateTray_1;

    TMyCylinder C_Auto3_FrontRiseTray;
    TMyCylinder C_Auto3_PushTray;
    TMyCylinder C_Auto3_LeanOnTray;
    TMyCylinder C_Auto3_RearRiseTray;
    TMyCylinder C_Auto3_FrontSeparateTray_1;

    TMyCylinder C_Auto4_FrontRiseTray;
    TMyCylinder C_Auto4_PushTray;
    TMyCylinder C_Auto4_LeanOnTray;
    TMyCylinder C_Auto4_RearRiseTray;
    TMyCylinder C_Auto4_FrontSeparateTray_1;

    TMyCylinder C_Auto5_FrontRiseTray;
    TMyCylinder C_Auto5_PushTray;
    TMyCylinder C_Auto5_LeanOnTray;
    TMyCylinder C_Auto5_RearRiseTray;
    TMyCylinder C_Auto5_FrontSeparateTray_1;

    TMyCylinder C_Auto6_FrontRiseTray;
    TMyCylinder C_Auto6_PushTray;
    TMyCylinder C_Auto6_LeanOnTray;
    TMyCylinder C_Auto6_RearRiseTray;
    TMyCylinder C_Auto6_FrontSeparateTray_1;

    TMyCylinder C_Color_FrontRiseTray;
    TMyCylinder C_Color_PushTray;
    TMyCylinder C_Color_LeanOnTray;
    TMyCylinder C_Color_RearRiseTray;
    TMyCylinder C_Color_FrontSeparateTray_1;
}CYLINDER_MODULAR;
//---------------------------------------------------------------------------
typedef struct SWITCH_MODULAR_STRUCR
{
    TMySwitch SwFKPowerOff;
    TMySwitch SwFKPowerOn;
    TMySwitch SwFrontActiveLed;
    TMySwitch SwFKReset;
    TMySwitch SwFKPause;
    TMySwitch SwFKHome;
    TMySwitch SwFKStart;
    TMySwitch SwFKOneCycle;
    TMySwitch SwFKRetry;
    TMySwitch SwFKSkip;
    TMySwitch SwFKCleanOut;
    TMySwitch SwFKTrayFeed;
    TMySwitch SwFKTrayEnd;
    TMySwitch SwFKAlarmReset;
    TMySwitch SwRKPowerOff;
    TMySwitch SwRKPowerOn;
    TMySwitch SwRKReset;
    TMySwitch SwRKPause;
    TMySwitch SwRKHome;
    TMySwitch SwRKStart;
    TMySwitch SwRKOneCycle;
    TMySwitch SwRKRetry;
    TMySwitch SwRKSkip;
    TMySwitch SwRKCleanOut;
    TMySwitch SwRKTrayFeed;
    TMySwitch SwRKTrayEnd;
    TMySwitch SwRKAlarmReset;
    TMySwitch SwRKManualStep;
    TMySwitch SwRKManualTStart;
    TMySwitch SwTowerRed;
    TMySwitch SwTowerYellow;
    TMySwitch SwTowerGreen;
    TMySwitch SwMusic1;
    TMySwitch SwMusic2;
    TMySwitch SwMusic3;
    TMySwitch SwMusic4;
    TMySwitch SwMotorRelay;
    TMySwitch SwServerON;
    TMySwitch SwRKSafeLock;
    TMySwitch SwRearActiveLed;
}SWITCH_MODULAR;
//---------------------------------------------------------------------------
typedef struct SUCKER_MODULAR_STRUCR
{
    TMyKitSuck SortArmSuck;
}SUCKER_MODULAR;
//---------------------------------------------------------------------------
class HTGem;
//---------------------------------------------------------------------------
typedef struct
{
    int  iLanguageCountry;
    int  iTemperature;
    int  iStartMode;
    int  iRealDummy;
    int SoakTime;                      // 23
    int JamSoakTime;                   // 24
    int InitialWaitTime;               // 25
    int CollingTime;                   // 26
    bool bPasswordCanInputByMouse;
    int  iJamCount[3];
    int  iErrorCount[10];                                                       // ben 20110721 //
    int  iTransferPos;
    int  iShuttlePos;
    int  iAutoPos;
    bool bCynStatus[200];
    bool bSwStatus[100];
    int iMaxContactCount;
} LAST_GENERAL_SET;
//-------------------------------------------------------------------------
class SYSTEM_MODULAR
{
public:
    AnsiString CurrentDir;
    AnsiString MotTablePath;
    AnsiString IoTablePath;
    TMOTNO MotNo;
    TIOTABLENO IoNo;
    LAST_GENERAL_SET LastSet;
    SYSTEM_STATUS Sys;
    MOTOR_MODULAR Mot;
    VIRTUAL_MOTOR_MODULAR VMot;
    SENSOR_MODULAR Sen;
    CYLINDER_MODULAR Cyn;
    SWITCH_MODULAR Sw;
    SUCKER_MODULAR Suck;
    TTrayMotor **MotPtr;
    TTrayMotor **VMotPtr;
    TMySensor *SenPtr;
    TMyCylinder *CynPtr;
    TMySwitch *SwPtr;
    TMyKitSuck *SuckPtr;
    TList *MotTable;
    TList *IOTable;
    HTGem *MyGem;
    int iTotalMotor;
    int iTotalVMotor;
    int iTotalSensor;
    int iTotalCylinder;
    int iTotalSwitch;
    int iTotalSucker;
    int iTotalSubSucker;

    SYSTEM_MODULAR();
    ~SYSTEM_MODULAR();
    void Initial();
    void InitialMotorName();
    void InitialVMotorName();
    void InitialVMotorParameter();
    void InitialSensorName();
    void InitialCylinderName();
    void InitialSwitchName();
    void InitialSuckerName();
    void LoadIoData();
    void LoadMotData();
    void LoadSensorParameterFromDataBase();
    void LoadCylinderParameterFromDataBase();
    void LoadSwitchParameterFromDataBase();
    void LoadSuckerParameterFromDataBase();
    void ClearIOTable();
    void ClearMotTable();
    TIODATA *FindIOData(AnsiString Alias);
    TMOTDATA *FindMotData(AnsiString Alias);
    void LoadSingleMotorParameterFromDataBase(int Index, bool bInitial=true);
    void LoadMotorParameterFromDataBase(int Index=-1, bool bInitial=true);
    void StopAllMotor();
    void DecStopAllMotor();
};
//---------------------------------------------------------------------------
extern SYSTEM_MODULAR HSys;
//---------------------------------------------------------------------------
#endif
