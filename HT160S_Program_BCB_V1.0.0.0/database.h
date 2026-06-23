//---------------------------------------------------------------------------
#ifndef databaseH
#define databaseH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <ActnList.hpp>
#include <ExtCtrls.hpp>
#include "MachineType.h"
#include "MotorAndIO/MyMotor.h"
#include "mycylin.h"
#include "MyKitSuck.h"
#include <map>
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260603 : alarm-code map framework, aligned with HT172 (mapNameToAlarm / mapAlarmCodeList)
//Alarm type enum, value-aligned with HT172 IncludeAllHeader.h eAlarmType.
enum eAlarmType{eJamErr        =0,
                eMessageErr    =1,
                eFunErr        =2,
                eSystemMess    =3,
                eCynAlarm      =4,
                eMotorAlarm    =5,
                eSuckAlarm     =6,
                eRecordProcess =7,
                eOther         =8,
                eAlatmTypeTotal
               };
//---------------------------------------------------------------------------
//Alarm code record, shape-aligned with HT172 MyAlarmCodeStruct (bilingual fields).
//HT160 currently fills the Chinese fields with English text to keep the source ASCII;
//part-name wording may differ from HT172, but the framework is the same.
class MyAlarmCodeStruct
{
public:
    AnsiString AlarmCode;
    int        AlarmType;
    AnsiString E_ErrMessage;
    AnsiString C_ErrMessage;
    AnsiString E_Description;
    AnsiString C_Description;
    AnsiString FlushPanelName;

    MyAlarmCodeStruct()
    {
        AlarmType=0;
    }
    MyAlarmCodeStruct(AnsiString aCode, int aType, AnsiString aEMsg, AnsiString aCMsg, AnsiString aEDesc, AnsiString aCDesc, AnsiString aPanel)
    {
        AlarmCode=aCode;
        AlarmType=aType;
        E_ErrMessage=aEMsg;
        C_ErrMessage=aCMsg;
        E_Description=aEDesc;
        C_Description=aCDesc;
        FlushPanelName=aPanel;
    }
    //AI(HT160S-Maintainer) 20260609 : CSV row for AlarmList.csv (ported from HT172 0420
    //MyAlarmCodeStruct::CommaText). Text fields are double-quoted so embedded commas
    //(e.g. "...position Error,Home and restart") do not break column alignment.
    AnsiString CommaText()
    {
        return AlarmCode + AnsiString(",") +
               AnsiString(AlarmType) + AnsiString(",") +
               AnsiString("\"") + E_ErrMessage  + AnsiString("\",") +
               AnsiString("\"") + C_ErrMessage  + AnsiString("\",") +
               AnsiString("\"") + E_Description + AnsiString("\",") +
               AnsiString("\"") + C_Description + AnsiString("\"");
    }
};
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
    TTrayMotor *MColorY;
    TTrayMotor *MTopCCDX_Color;
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
    Run_TrayFeed    =4
};
//---------------------------------------------------------------------------
typedef struct SYSTEM_STATUS_STRUCT
{
    bool SystemStart;
    RunModeEnum RunMode;
    bool bCleanOut;   //AI(HT160S-Maintainer) 20260605 : nested-continuation latch (resume CleanOut after a mid-drain OneCycle)
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
    TMySensor SnEmpty_InputHasTray;
    TMySensor SnEmpty_InputFullTray;
    TMySensor SnEmpty_TrayPos1;
    TMySensor SnEmpty_TrayPos2;
    TMySensor SnEmpty_OutputHasTray;
    TMySensor SnEmpty_OutputBottomHasTray;
    TMySensor SnEmpty_InputEnd;
    TMySensor SnLoader_InputHasTray;
    TMySensor SnLoader_InputFullTray;
    TMySensor SnLoader_TrayPos1;
    TMySensor SnLoader_TrayPos2;
    TMySensor SnLoader_OutputHasTray;
    TMySensor SnLoader_OutputBottomHasTray;
    TMySensor SnLoader_Inputend;
    TMySensor SnAuto1_InputHasTray;
    TMySensor SnAuto1_InputFullTray;
    TMySensor SnAuto1_InputEnd;
    TMySensor SnAuto1_OutputHasTray;
    TMySensor SnAuto1_OutputBottomHasTray;
    TMySensor SnAuto1_TrayPos1;
    TMySensor SnAuto1_TrayPos2;
    TMySensor SnAuto2_InputHasTray;
    TMySensor SnAuto2_InputFullTray;
    TMySensor SnAuto2_InputEnd;
    TMySensor SnAuto2_OutputHasTray;
    TMySensor SnAuto2_OutputBottomHasTray;
    TMySensor SnAuto2_TrayPos1;
    TMySensor SnAuto2_TrayPos2;
    TMySensor SnAuto3_InputHasTray;
    TMySensor SnAuto3_InputFullTray;
    TMySensor SnAuto3_InputEnd;
    TMySensor SnAuto3_OutputHasTray;
    TMySensor SnAuto3_OutputBottomHasTray;
    TMySensor SnAuto3_TrayPos1;
    TMySensor SnAuto3_TrayPos2;
    TMySensor SnAuto4_InputHasTray;
    TMySensor SnAuto4_InputFullTray;
    TMySensor SnAuto4_InputEnd;
    TMySensor SnAuto4_OutputHasTray;
    TMySensor SnAuto4_OutputBottomHasTray;
    TMySensor SnAuto4_TrayPos1;
    TMySensor SnAuto4_TrayPos2;
    TMySensor SnAuto5_InputHasTray;
    TMySensor SnAuto5_InputFullTray;
    TMySensor SnAuto5_InputEnd;
    TMySensor SnAuto5_OutputHasTray;
    TMySensor SnAuto5_OutputBottomHasTray;
    TMySensor SnAuto5_TrayPos1;
    TMySensor SnAuto5_TrayPos2;
    TMySensor SnAuto6_InputHasTray;
    TMySensor SnAuto6_InputFullTray;
    TMySensor SnAuto6_InputEnd;
    TMySensor SnAuto6_OutputHasTray;
    TMySensor SnAuto6_OutputBottomHasTray;
    TMySensor SnAuto6_TrayPos1;
    TMySensor SnAuto6_TrayPos2;
    TMySensor SnColor_InputHasTray;
    TMySensor SnColor_InputFullTray;
    TMySensor SnColor_TrayPos1;
    TMySensor SnColor_OutputBottomHasTray;
    TMySensor SnColor_InputEnd;
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

    TMyCylinder C_Color_FrontRiseTray_1;
    TMyCylinder C_Color_FrontRiseTray_2;
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
class TDataModule1 : public TDataModule
{
__published:
    TActionList *UserActionList;
    TAction *InitialMotorName;
    TAction *SpecificSetupForMotorParameter;
    TAction *InitialCylinderName;
    TAction *SpecificSetupForCylinderParameter;
    TAction *InitialSensorName;
    TAction *SpecificSetupForSensorParameter;
    TAction *InitialSwitchName;
    TAction *SpecificSetupForSwitchParameter;
    TAction *InitialSuckerName;
    TAction *SpecificSetupForSuckerParameter;
    TAction *Initial_IO_Setup;
    TActionList *UserMotion;
    TTimer *Timer1;
    TAction *actEmpty;
    TAction *actLoader1;
    TAction *actLoader2;
    TAction *actAuto1to6;
    TAction *actTrayArm;
    TAction *actSortArm;
    TAction *actColor;
    void __fastcall InitialMotorNameExecute(TObject *Sender);
    void __fastcall InitialCylinderNameExecute(TObject *Sender);
    void __fastcall InitialSensorNameExecute(TObject *Sender);
    void __fastcall InitialSwitchNameExecute(TObject *Sender);
    void __fastcall InitialSuckerNameExecute(TObject *Sender);
    void __fastcall Timer1Timer(TObject *Sender);
    void __fastcall actEmptyExecute(TObject *Sender);
    void __fastcall actLoader1Execute(TObject *Sender);
    void __fastcall actLoader2Execute(TObject *Sender);
    void __fastcall actAuto1to6Execute(TObject *Sender);
    void __fastcall actTrayArmExecute(TObject *Sender);
    void __fastcall actSortArmExecute(TObject *Sender);
    void __fastcall actColorExecute(TObject *Sender);

public:
    __fastcall TDataModule1(TComponent* Owner);
    void InitialAllTask(bool bKeepMaterial=false);
    void DoAllProcess();
};
//---------------------------------------------------------------------------
extern PACKAGE TDataModule1 *DataModule1;
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
class TMyBinDispCtrl;   //AI(ht160s-maintainer) 20260615 : Bin display controller (MyBinDisp.h)
//-------------------------------------------------------------------------
class SYSTEM_MODULAR
{
public:
    AnsiString CurrentDir;
    AnsiString LogRootDir;
    AnsiString MotTablePath;
    AnsiString IoTablePath;
    AnsiString AlarmTablePath;   //AI(HT160S-Maintainer) 20260609 : AlarmList.csv (all machine alarms, built at startup)
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
    TMyBinDispCtrl *BinDisCtrl;   //AI(ht160s-maintainer) 20260615 : LED bin display (HSys-owned, created in database.cpp)
    int iTotalMotor;
    int iTotalVMotor;
    int iTotalSensor;
    int iTotalCylinder;
    int iTotalSwitch;
    int iTotalSucker;
    int iTotalSubSucker;

    //AI(HT160S-Maintainer) 20260603 : alarm-code lookup maps, aligned with HT172
    std::map<AnsiString, AnsiString>        mapNameToAlarm;
    std::map<AnsiString, MyAlarmCodeStruct> mapAlarmCodeList;
    std::map<AnsiString, MyAlarmCodeStruct>::iterator IterAlarmCodeList;
    //AI(HT160S-Maintainer) 20260603 : alarm-code -> caller context (Func/Case), shown in note remark
    std::map<int, AnsiString>               mapAlarmContext;

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
    void CreateSystemAlarmCode();
};
//---------------------------------------------------------------------------
extern SYSTEM_MODULAR HSys;
//---------------------------------------------------------------------------
#endif
