//---------------------------------------------------------------------------

#include <vcl.h>
#include <stdlib.h>
#include <string.h>
#pragma hdrstop

#include "database.h"
#include "cmydef.h"
#include "myio_MN200.h"
#include "CosFunction.h"
#include "SecsGem\uHGemClass.h"
#pragma package(smart_init)
//---------------------------------------------------------------------------
SYSTEM_MODULAR HSys;
//---------------------------------------------------------------------------
static int FindMotColumn(TStringList *SL, const char *ColumnName)
{
    AnsiString Target=AnsiString(ColumnName).UpperCase();
    for(int i=0; i<SL->Count; i++)
    {
        AnsiString Name=SL->Strings[i].UpperCase();
        if(Name==Target)
            return i;
    }
    return -1;
}
//---------------------------------------------------------------------------
static AnsiString GetMotField(TStringList *SL, int Index)
{
    if(Index<0 || Index>=SL->Count)
        return "";
    return SL->Strings[Index];
}
//---------------------------------------------------------------------------
static int GetMotInt(TStringList *SL, int Index, int DefaultValue)
{
    AnsiString Value=GetMotField(SL, Index);
    if(Value==AnsiString(""))
        return DefaultValue;
    return atoi(Value.c_str());
}
//---------------------------------------------------------------------------
static double GetMotDouble(TStringList *SL, int Index, double DefaultValue)
{
    AnsiString Value=GetMotField(SL, Index);
    if(Value==AnsiString(""))
        return DefaultValue;
    return atof(Value.c_str());
}
//---------------------------------------------------------------------------
static int FindIOColumn(TStringList *SL, const char *ColumnName)
{
    AnsiString Target=AnsiString(ColumnName).UpperCase();
    for(int ColumnIndex=0; ColumnIndex<SL->Count; ColumnIndex++)
    {
        AnsiString Name=SL->Strings[ColumnIndex].UpperCase();
        if(Name==Target)
            return ColumnIndex;
    }
    return -1;
}
//---------------------------------------------------------------------------
static AnsiString GetIOField(TStringList *SL, int Index)
{
    if(Index<0 || Index>=SL->Count)
        return "";
    return SL->Strings[Index];
}
//---------------------------------------------------------------------------
static int GetIOInt(TStringList *SL, int Index, int DefaultValue)
{
    AnsiString Value=GetIOField(SL, Index);
    if(Value==AnsiString(""))
        return DefaultValue;
    return atoi(Value.c_str());
}
//---------------------------------------------------------------------------
static int ParseIOBase(AnsiString Value)
{
    AnsiString UpperValue=Value.UpperCase();
    if(Value==AnsiString("") || UpperValue==AnsiString("MN200") || UpperValue==AnsiString("SYNTEK"))
        return eMotionNet;
    return atoi(Value.c_str());
}
//---------------------------------------------------------------------------
static int ParseIOIP(AnsiString Value)
{
    AnsiString UpperValue=Value.UpperCase();
    if(Value==AnsiString(""))
        return -1;
    if(UpperValue.Length()==1 && UpperValue[1]>='A' && UpperValue[1]<='Z')
        return int(UpperValue[1]-'A')+10;
    return atoi(Value.c_str());
}
//---------------------------------------------------------------------------
static int ParseIOPort(AnsiString Value, int ISABase)
{
    if(Value==AnsiString(""))
        return -1;
    if(ISABase==eISABase || ISABase==ePCI1735U || ISABase==e_PLCbase)
        return (int)strtol(Value.c_str(), NULL, 16);
    return atoi(Value.c_str());
}
//---------------------------------------------------------------------------
static int GetNonNegativeIOTime(int Value)
{
    if(Value<0)
        return 0;
    return Value;
}
//---------------------------------------------------------------------------
static bool IsValidIOData(TIODATA *Data)
{
    return (Data!=NULL && Data->iEnable!=0 && Data->iLane>=0 && Data->iIP>=0 && Data->iPort>=0 && Data->iBit>=0);
}
//---------------------------------------------------------------------------
static TMyIo *CreateIOObject(TIODATA *Data)
{
    if(IsValidIOData(Data) && Data->iISABase==eMotionNet)
        return new TMyMN200_IO;
    return new TMyIo;
}
//---------------------------------------------------------------------------
static void SetIOObjectData(TMyIo *IOPtr, TIODATA *Data)
{
    if(IOPtr==NULL || Data==NULL)
        return;

    IOPtr->ISABase=Data->iISABase;
    IOPtr->iLane=Data->iLane;
    IOPtr->iIP=Data->iIP;
    IOPtr->iCard=Data->iLane*100+Data->iIP;
    IOPtr->iPort=Data->iPort;
    IOPtr->iBit=Data->iBit;
    IOPtr->iModuleType=Data->iModuleType;
    IOPtr->SetHint(Data->IOPos);
}
//---------------------------------------------------------------------------
static void SetupSwitchFromIO(TMySwitch *SwitchPtr, TIODATA *Data, AnsiString Name)
{
    if(SwitchPtr==NULL)
        return;

    delete SwitchPtr->Output;
    SwitchPtr->Output=new TMyIo;
    SwitchPtr->Name=Name;
    SwitchPtr->Enable=IsValidIOData(Data);
    SwitchPtr->EnableAtDataBase=SwitchPtr->Enable;
    if(Data==NULL)
        return;

    SwitchPtr->Output=CreateIOObject(Data);
    SwitchPtr->IOPos=Data->IOPos;
    SwitchPtr->Card=Data->iLane*100+Data->iIP;
    SwitchPtr->Port=Data->iPort;
    SwitchPtr->Bit=Data->iBit;
    SwitchPtr->Type=Data->iInType;
    SetIOObjectData(SwitchPtr->Output, Data);
}
//---------------------------------------------------------------------------
static void SetupSensorFromIO(TMySensor *SensorPtr, TIODATA *Data, AnsiString Name)
{
    if(SensorPtr==NULL)
        return;

    delete SensorPtr->Input;
    SensorPtr->Input=new TMyIo;
    SensorPtr->Name=Name;
    SensorPtr->Enable=IsValidIOData(Data);
    SensorPtr->EnableAtDataBase=SensorPtr->Enable;
    if(Data==NULL)
        return;

    SensorPtr->Input=CreateIOObject(Data);
    SensorPtr->IOPos=Data->IOPos;
    SensorPtr->Card=Data->iLane*100+Data->iIP;
    SensorPtr->Port=Data->iPort;
    SensorPtr->Bit=Data->iBit;
    SensorPtr->Type=Data->iInType;
    SetIOObjectData(SensorPtr->Input, Data);
}
//---------------------------------------------------------------------------
TIOTABLENO::TIOTABLENO()
{
    eioType             =0;
    eioAlias            =1;
    eioLane             =2;
    eioModuleType       =3;
    eioIP               =4;
    eioPort             =5;
    eioBit              =6;
    eioInType           =7;
    eioISABase          =8;
    eioEnable           =9;
    eioOnAlarmTime      =10;
    eioOffAlarmTime     =11;
    eioOnDelayTime      =12;
    eioOffDelayTime     =13;
    eioNote             =14;
    eioTotal            =15;
}
//---------------------------------------------------------------------------
int TIOTABLENO::SetIOTableNo(AnsiString Str)
{
    int Result=eioTotal;
    TStringList *SL=new TStringList();
    SL->CommaText=Str;

    eioType=FindIOColumn(SL, "IOType");
    if(eioType<0 && Result==eioTotal) Result=0;
    eioAlias=FindIOColumn(SL, "Alias");
    if(eioAlias<0 && Result==eioTotal) Result=1;
    eioLane=FindIOColumn(SL, "Lane");
    if(eioLane<0 && Result==eioTotal) Result=2;
    eioModuleType=FindIOColumn(SL, "ModuleType");
    if(eioModuleType<0 && Result==eioTotal) Result=3;
    eioIP=FindIOColumn(SL, "IP");
    if(eioIP<0 && Result==eioTotal) Result=4;
    eioPort=FindIOColumn(SL, "Port");
    if(eioPort<0 && Result==eioTotal) Result=5;
    eioBit=FindIOColumn(SL, "Bit");
    if(eioBit<0 && Result==eioTotal) Result=6;
    eioInType=FindIOColumn(SL, "InType");
    if(eioInType<0 && Result==eioTotal) Result=7;
    eioISABase=FindIOColumn(SL, "ISABase");
    if(eioISABase<0 && Result==eioTotal) Result=8;
    eioEnable=FindIOColumn(SL, "Enable");
    if(eioEnable<0 && Result==eioTotal) Result=9;
    eioOnAlarmTime=FindIOColumn(SL, "OnAlarmTime");
    if(eioOnAlarmTime<0 && Result==eioTotal) Result=10;
    eioOffAlarmTime=FindIOColumn(SL, "OffAlarmTime");
    if(eioOffAlarmTime<0 && Result==eioTotal) Result=11;
    eioOnDelayTime=FindIOColumn(SL, "OnDelayTime");
    if(eioOnDelayTime<0 && Result==eioTotal) Result=12;
    eioOffDelayTime=FindIOColumn(SL, "OffDelayTime");
    if(eioOffDelayTime<0 && Result==eioTotal) Result=13;
    eioNote=FindIOColumn(SL, "Note");
    if(eioNote<0 && Result==eioTotal) Result=14;

    delete SL;
    return Result;
}
//---------------------------------------------------------------------------
TIODATA::TIODATA(AnsiString Str)
{
    bool bHasNullData=false;
    TStringList *SL=new TStringList();
    SL->CommaText=Str;

    Tag             =(HSys.IOTable==NULL)?0:HSys.IOTable->Count;
    _CommaText      =Str;
    Type            =GetIOField(SL, HSys.IoNo.eioType);
    Alias           =GetIOField(SL, HSys.IoNo.eioAlias);
    sIP             =GetIOField(SL, HSys.IoNo.eioIP);
    iISABase        =ParseIOBase(GetIOField(SL, HSys.IoNo.eioISABase));
    iLane           =GetIOInt(SL, HSys.IoNo.eioLane, -1);
    iModuleType     =GetIOInt(SL, HSys.IoNo.eioModuleType, -1);
    iIP             =ParseIOIP(sIP);
    iPort           =ParseIOPort(GetIOField(SL, HSys.IoNo.eioPort), iISABase);
    iBit            =GetIOInt(SL, HSys.IoNo.eioBit, -1);
    iInType         =GetIOInt(SL, HSys.IoNo.eioInType, 0);
    iEnable         =GetIOInt(SL, HSys.IoNo.eioEnable, 0);
    iOnAlarmTime    =GetIOInt(SL, HSys.IoNo.eioOnAlarmTime, -1);
    iOffAlarmTime   =GetIOInt(SL, HSys.IoNo.eioOffAlarmTime, -1);
    iOnDelayTime    =GetIOInt(SL, HSys.IoNo.eioOnDelayTime, -1);
    iOffDelayTime   =GetIOInt(SL, HSys.IoNo.eioOffDelayTime, -1);
    bDelete         =false;

    if(iPort<0 || iBit<0 || Alias==AnsiString(""))
        bHasNullData=true;
    if(iISABase==eMotionNet && (iLane<0 || iIP<0))
        bHasNullData=true;

    if(iISABase==eISABase || iISABase==ePCI1735U || iISABase==e_PLCbase)
    {
        IOPos.sprintf("%s%s", GetIOField(SL, HSys.IoNo.eioPort), GetIOField(SL, HSys.IoNo.eioBit));
    }
    else
    {
        IOPos.sprintf("%s%s%s%s%s", GetIOField(SL, HSys.IoNo.eioLane),
                                    GetIOField(SL, HSys.IoNo.eioModuleType),
                                    GetIOField(SL, HSys.IoNo.eioIP),
                                    GetIOField(SL, HSys.IoNo.eioPort),
                                    GetIOField(SL, HSys.IoNo.eioBit));
    }

    if(bHasNullData)
        iEnable=0;

    delete SL;
}
//---------------------------------------------------------------------------
TMOTNO::TMOTNO()
{
    emotNo               =0;
    emotAlias            =1;
    emotDirection        =2;
    emotGearRatio        =3;
    emotHomeDirectior    =4;
    emotHomeHighSpeed    =5;
    emotHomeLowSpeed     =6;
    emotInitSpeed        =7;
    emotJogHighSpeed     =8;
    emotJogLowSpeed      =9;
    emotRate             =10;
    emotSoftLimitN       =11;
    emotSoftLimitP       =12;
    emotEnable           =13;
    emotServoAlarmOn     =14;
    emotRange            =15;
    emot1P2P             =16;
    emotSensorType       =17;
    emotSimulateSpeed    =18;
    emotCardModel        =19;
    emotBoardID          =20;
    emotPort             =21;
    emotAcc              =22;
    emotDec              =23;
    emotMotorKind        =24;
    emotFlushPanel       =25;
    emotHomeOrder        =26;
    emotLimitLogic       =27;
    emotIn1Logic         =28;
    emotTotal            =29;
}
//---------------------------------------------------------------------------
int TMOTNO::SetMOTTableNo(AnsiString Str)
{
    int Result=emotTotal;
    TStringList *SL=new TStringList();
    SL->CommaText=Str;

    emotNo=FindMotColumn(SL, "Motorname");
    if(emotNo<0 && Result==emotTotal) Result=0;
    emotAlias=FindMotColumn(SL, "Alias");
    if(emotAlias<0 && Result==emotTotal) Result=1;
    emotDirection=FindMotColumn(SL, "Direction");
    if(emotDirection<0 && Result==emotTotal) Result=2;
    emotGearRatio=FindMotColumn(SL, "GearRatio");
    if(emotGearRatio<0 && Result==emotTotal) Result=3;
    emotHomeDirectior=FindMotColumn(SL, "HomeDirectior");
    if(emotHomeDirectior<0 && Result==emotTotal) Result=4;
    emotHomeHighSpeed=FindMotColumn(SL, "HomeHighSpeed");
    if(emotHomeHighSpeed<0 && Result==emotTotal) Result=5;
    emotHomeLowSpeed=FindMotColumn(SL, "HomeLowSpeed");
    if(emotHomeLowSpeed<0 && Result==emotTotal) Result=6;
    emotInitSpeed=FindMotColumn(SL, "InitSpeed");
    if(emotInitSpeed<0 && Result==emotTotal) Result=7;
    emotJogHighSpeed=FindMotColumn(SL, "JogHighSpeed");
    if(emotJogHighSpeed<0 && Result==emotTotal) Result=8;
    emotJogLowSpeed=FindMotColumn(SL, "JogLowSpeed");
    if(emotJogLowSpeed<0 && Result==emotTotal) Result=9;
    emotRate=FindMotColumn(SL, "Rate");
    if(emotRate<0 && Result==emotTotal) Result=10;
    emotSoftLimitN=FindMotColumn(SL, "SoftLimitN");
    if(emotSoftLimitN<0 && Result==emotTotal) Result=11;
    emotSoftLimitP=FindMotColumn(SL, "SoftLimitP");
    if(emotSoftLimitP<0 && Result==emotTotal) Result=12;
    emotEnable=FindMotColumn(SL, "Enable");
    if(emotEnable<0 && Result==emotTotal) Result=13;
    emotServoAlarmOn=FindMotColumn(SL, "ServoAlarmOn");
    if(emotServoAlarmOn<0 && Result==emotTotal) Result=14;
    emotRange=FindMotColumn(SL, "Range");
    if(emotRange<0 && Result==emotTotal) Result=15;
    emot1P2P=FindMotColumn(SL, "1P2P");
    if(emot1P2P<0 && Result==emotTotal) Result=16;
    emotSensorType=FindMotColumn(SL, "SensorType");
    if(emotSensorType<0 && Result==emotTotal) Result=17;
    emotSimulateSpeed=FindMotColumn(SL, "SimulateSpeed");
    if(emotSimulateSpeed<0 && Result==emotTotal) Result=18;
    emotCardModel=FindMotColumn(SL, "CardModel");
    if(emotCardModel<0 && Result==emotTotal) Result=19;
    emotBoardID=FindMotColumn(SL, "BoardID");
    if(emotBoardID<0 && Result==emotTotal) Result=20;
    emotPort=FindMotColumn(SL, "Port");
    if(emotPort<0 && Result==emotTotal) Result=21;
    emotAcc=FindMotColumn(SL, "Acc");
    if(emotAcc<0 && Result==emotTotal) Result=22;
    emotDec=FindMotColumn(SL, "Dec");
    if(emotDec<0 && Result==emotTotal) Result=23;
    emotMotorKind=FindMotColumn(SL, "MotorKind");
    if(emotMotorKind<0 && Result==emotTotal) Result=24;
    emotFlushPanel=FindMotColumn(SL, "FlushPanel");
    if(emotFlushPanel<0 && Result==emotTotal) Result=25;
    emotHomeOrder=FindMotColumn(SL, "HomeOrder");
    if(emotHomeOrder<0 && Result==emotTotal) Result=26;
    emotLimitLogic=FindMotColumn(SL, "LimitLogic");
    if(emotLimitLogic<0 && Result==emotTotal) Result=27;
    emotIn1Logic=FindMotColumn(SL, "In1Logic");
    if(emotIn1Logic<0 && Result==emotTotal) Result=28;

    delete SL;
    return Result;
}
//---------------------------------------------------------------------------
TMOTDATA::TMOTDATA(AnsiString Str)
{
    bool bHasNullData=false;
    TStringList *SL=new TStringList();
    SL->CommaText=Str;

    _CommaText      =Str;
    No              =GetMotField(SL, HSys.MotNo.emotNo);
    Alias           =GetMotField(SL, HSys.MotNo.emotAlias);
    iEnable         =GetMotInt(SL, HSys.MotNo.emotEnable, 0);
    iBoardID        =GetMotInt(SL, HSys.MotNo.emotBoardID, -1);
    iPort           =GetMotInt(SL, HSys.MotNo.emotPort, -1);
    dGearRatio      =GetMotDouble(SL, HSys.MotNo.emotGearRatio, 1.0);
    iSoftLimitN     =GetMotInt(SL, HSys.MotNo.emotSoftLimitN, -999999);
    iSoftLimitP     =GetMotInt(SL, HSys.MotNo.emotSoftLimitP, 999999);
    CardModel       =GetMotField(SL, HSys.MotNo.emotCardModel).UpperCase();
    MotorKind       =GetMotInt(SL, HSys.MotNo.emotMotorKind, 0);
    FlushPanel      =GetMotField(SL, HSys.MotNo.emotFlushPanel);
    HomeOrder       =GetMotField(SL, HSys.MotNo.emotHomeOrder);
    iDirection      =GetMotInt(SL, HSys.MotNo.emotDirection, 0);
    iHomeDirectior  =GetMotInt(SL, HSys.MotNo.emotHomeDirectior, 0);
    dAcc            =GetMotDouble(SL, HSys.MotNo.emotAcc, 1.0);
    dDec            =GetMotDouble(SL, HSys.MotNo.emotDec, 1.0);
    iInitSpeed      =GetMotInt(SL, HSys.MotNo.emotInitSpeed, 100);
    iHomeHighSpeed  =GetMotInt(SL, HSys.MotNo.emotHomeHighSpeed, 100);
    iHomeLowSpeed   =GetMotInt(SL, HSys.MotNo.emotHomeLowSpeed, 100);
    iJogHighSpeed   =GetMotInt(SL, HSys.MotNo.emotJogHighSpeed, 100);
    iJogLowSpeed    =GetMotInt(SL, HSys.MotNo.emotJogLowSpeed, 100);
    iRange          =GetMotInt(SL, HSys.MotNo.emotRange, 1);
    iRate           =GetMotInt(SL, HSys.MotNo.emotRate, 1);
    iServoAlarmOn   =GetMotInt(SL, HSys.MotNo.emotServoAlarmOn, 0);
    i1P2P           =GetMotInt(SL, HSys.MotNo.emot1P2P, 0);
    iSensorType     =GetMotInt(SL, HSys.MotNo.emotSensorType, 0);
    iLimitLogic     =GetMotInt(SL, HSys.MotNo.emotLimitLogic, 0);
    iIn1Logic       =GetMotInt(SL, HSys.MotNo.emotIn1Logic, 0);
    iSimulateSpeed  =GetMotInt(SL, HSys.MotNo.emotSimulateSpeed, 10000);

    if(No==AnsiString("") || Alias==AnsiString("") || CardModel==AnsiString("") ||
       iBoardID<0 || iPort<0)
    {
        bHasNullData=true;
    }
    if(SL->Count<HSys.MotNo.emotTotal || bHasNullData)
        iEnable=0;

    delete SL;
}
//---------------------------------------------------------------------------
SYSTEM_MODULAR::SYSTEM_MODULAR()
{
    CurrentDir = "..";
    MotTablePath = CurrentDir + "\\system\\Mot_Table.csv";
    IoTablePath = CurrentDir + "\\system\\IO_Table.csv";
    LastSet.iLanguageCountry = 0;
    LastSet.iStartMode = 0;
    #ifdef SOFT_SIMULATE
        LastSet.iRealDummy = DUMMY;
    #else
        LastSet.iRealDummy = REALLY;
    #endif
    Sys.SystemStart = false;
    Sys.RunMode = Run_Normal;
    MotPtr = NULL;
    VMotPtr = NULL;
    SenPtr = NULL;
    CynPtr = NULL;
    SwPtr = NULL;
    SuckPtr = NULL;
    MyGem = NULL;
    MotTable = new TList;
    IOTable = new TList;
    iTotalMotor = 0;
    iTotalVMotor = 0;
    iTotalSensor = 0;
    iTotalCylinder = 0;
    iTotalSwitch = 0;
    iTotalSucker = 0;
    iTotalSubSucker = MAX_SUB_SUCKER_ITEM;
    memset(&Mot, 0, sizeof(Mot));
    memset(&VMot, 0, sizeof(VMot));
    Initial();
}
//---------------------------------------------------------------------------
SYSTEM_MODULAR::~SYSTEM_MODULAR()
{
    if(MyGem!=NULL)
    {
        delete MyGem;
        MyGem=NULL;
    }
    if(MotPtr!=NULL)
    {
        for(int i=iTotalMotor-1; i>=0; i--)
        {
            delete MotPtr[i];
            MotPtr[i]=NULL;
        }
    }
    if(VMotPtr!=NULL)
    {
        for(int i=iTotalVMotor-1; i>=0; i--)
        {
            delete VMotPtr[i];
            VMotPtr[i]=NULL;
        }
    }
    if(SenPtr!=NULL)
    {
        for(int i=iTotalSensor-1; i>=0; i--)
        {
            delete SenPtr[i].Input;
            SenPtr[i].Input=NULL;
        }
    }
    if(CynPtr!=NULL)
    {
        for(int i=iTotalCylinder-1; i>=0; i--)
        {
            delete CynPtr[i].Switch.Output;
            CynPtr[i].Switch.Output=NULL;
            delete CynPtr[i].OnSensor.Input;
            CynPtr[i].OnSensor.Input=NULL;
            delete CynPtr[i].OffSensor.Input;
            CynPtr[i].OffSensor.Input=NULL;
        }
    }
    if(SwPtr!=NULL)
    {
        for(int i=iTotalSwitch-1; i>=0; i--)
        {
            delete SwPtr[i].Output;
            SwPtr[i].Output=NULL;
        }
    }
    if(SuckPtr!=NULL)
    {
        for(int SuckerIndex=iTotalSucker-1; SuckerIndex>=0; SuckerIndex--)
        {
            for(int RowIndex=MAX_SUCKER_ROW-1; RowIndex>=0; RowIndex--)
            {
                for(int ColIndex=MAX_SUCKER_COL-1; ColIndex>=0; ColIndex--)
                {
                    delete SuckPtr[SuckerIndex].Suck[RowIndex][ColIndex].OnSw.Output;
                    SuckPtr[SuckerIndex].Suck[RowIndex][ColIndex].OnSw.Output=NULL;
                    delete SuckPtr[SuckerIndex].Suck[RowIndex][ColIndex].OffSw.Output;
                    SuckPtr[SuckerIndex].Suck[RowIndex][ColIndex].OffSw.Output=NULL;
                    delete SuckPtr[SuckerIndex].Suck[RowIndex][ColIndex].Sensor.Input;
                    SuckPtr[SuckerIndex].Suck[RowIndex][ColIndex].Sensor.Input=NULL;
                }
            }
        }
    }
    ClearIOTable();
    ClearMotTable();
    delete IOTable;
    delete MotTable;
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::Initial()
{
    InitialCosFunction();
    MotPtr=(TTrayMotor **)&Mot;
    VMotPtr=(TTrayMotor **)&VMot;
    SenPtr=(TMySensor *)&Sen;
    CynPtr=(TMyCylinder *)&Cyn;
    SwPtr=(TMySwitch *)&Sw;
    SuckPtr=(TMyKitSuck *)&Suck;
    iTotalMotor=sizeof(MOTOR_MODULAR)/sizeof(TTrayMotor *);
    for(int i=0; i<iTotalMotor; i++)
        MotPtr[i]=new TTrayMotor;
    iTotalVMotor=sizeof(VIRTUAL_MOTOR_MODULAR)/sizeof(TTrayMotor *);
    for(int i=0; i<iTotalVMotor; i++)
        VMotPtr[i]=new TTrayMotor;
    iTotalSensor=sizeof(SENSOR_MODULAR)/sizeof(TMySensor);
    iTotalCylinder=sizeof(CYLINDER_MODULAR)/sizeof(TMyCylinder);
    iTotalSwitch=sizeof(SWITCH_MODULAR)/sizeof(TMySwitch);
    iTotalSucker=sizeof(SUCKER_MODULAR)/sizeof(TMyKitSuck);
    iTotalSubSucker=MAX_SUB_SUCKER_ITEM;
    InitialMotorName();
    InitialVMotorName();
    InitialSensorName();
    InitialCylinderName();
    InitialSwitchName();
    InitialSuckerName();
    LoadIoData();
    LoadSensorParameterFromDataBase();
    LoadCylinderParameterFromDataBase();
    LoadSwitchParameterFromDataBase();
    LoadSuckerParameterFromDataBase();
    LoadMotorParameterFromDataBase();
    InitialVMotorParameter();
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::InitialSensorName()
{
    Sen.SnFKPowerOff.Name="SnFKPowerOff";
    Sen.SnFKPowerOn.Name="SnFKPowerOn";
    Sen.SnRKPowerOff.Name="SnRKPowerOff";
    Sen.SnRKPowerOn.Name="SnRKPowerOn";
    Sen.SnSafeLock.Name="SnSafeLock";
    Sen.SnMotorPower.Name="SnMotorPower";
    Sen.SnAirIsEnough.Name="SnAirIsEnough";
    Sen.SnIonFan_Balance.Name="SnIonFan_Balance";
    Sen.SnIonFan_Power.Name="SnIonFan_Power";
    Sen.SnEMG.Name="SnEMG";
    Sen.SnEMG_1.Name="SnEMG_1";
    Sen.SnEMG_2.Name="SnEMG_2";
    Sen.SnEMG_3.Name="SnEMG_3";
    Sen.SnEMG_4.Name="SnEMG_4";
    Sen.SnSafeDoorFront.Name="SnSafeDoorFront";
    Sen.SnSafeDoorRight.Name="SnSafeDoorRight";
    Sen.SnSafeDoorLeft.Name="SnSafeDoorLeft";
    Sen.SnSafeSlideDoorRight.Name="SnSafeSlideDoorRight";
    Sen.SnSafeSlideDoorLeft.Name="SnSafeSlideDoorLeft";
    Sen.SnSafeAuto6.Name="SnSafeAuto6";
    Sen.SnColor_InputHasTray.Name="SnColor_InputHasTray";
    Sen.SnColor_InputFullTray.Name="SnColor_InputFullTray";
    Sen.SnColor_TrayPos1.Name="SnColor_TrayPos1";
    Sen.SnColor_OutputBottomHasTray.Name="SnColor_OutputBottomHasTray";
    Sen.SnFrontPadActive.Name="SnFrontPadActive";
    Sen.SnFKReset.Name="SnFKReset";
    Sen.SnFKPause.Name="SnFKPause";
    Sen.SnFKHome.Name="SnFKHome";
    Sen.SnFKStart.Name="SnFKStart";
    Sen.SnFKOneCycle.Name="SnFKOneCycle";
    Sen.SnFKRetry.Name="SnFKRetry";
    Sen.SnFKSkip.Name="SnFKSkip";
    Sen.SnFKCleanOut.Name="SnFKCleanOut";
    Sen.SnFKTrayFeed.Name="SnFKTrayFeed";
    Sen.SnFKTrayEnd.Name="SnFKTrayEnd";
    Sen.SnFKAlarmReset.Name="SnFKAlarmReset";
    Sen.SnRearPadActive.Name="SnRearPadActive";
    Sen.SnRKReset.Name="SnRKReset";
    Sen.SnRKPause.Name="SnRKPause";
    Sen.SnRKHome.Name="SnRKHome";
    Sen.SnRKStart.Name="SnRKStart";
    Sen.SnRKOneCycle.Name="SnRKOneCycle";
    Sen.SnRKRetry.Name="SnRKRetry";
    Sen.SnRKSkip.Name="SnRKSkip";
    Sen.SnRKCleanOut.Name="SnRKCleanOut";
    Sen.SnRKTrayFeed.Name="SnRKTrayFeed";
    Sen.SnRKTray.Name="SnRKTray";
    Sen.SnRKTrayEnd.Name="SnRKTrayEnd";
    Sen.SnRKAlarmReset.Name="SnRKAlarmReset";
    Sen.SnRKManualStep.Name="SnRKManualStep";
    Sen.SnRKManualTStart.Name="SnRKManualTStart";
    Sen.SnRKSafeLock.Name="SnRKSafeLock";
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::LoadSensorParameterFromDataBase()
{
    if(SenPtr==NULL)
    {
        SenPtr=(TMySensor *)&Sen;
        iTotalSensor=sizeof(SENSOR_MODULAR)/sizeof(TMySensor);
    }

    for(int i=0; i<iTotalSensor; i++)
    {
        TMySensor *SensorData=&SenPtr[i];
        if(SensorData->Name==AnsiString(""))
            continue;

        SetupSensorFromIO(SensorData, FindIOData(SensorData->Name), SensorData->Name);
        SensorData->Tag=i;
    }
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::InitialMotorName()
{
    Mot.MSortingArmX ->Alias="MSortingArmX";
    Mot.MTrayArmX   ->Alias="MTrayArmX";
    Mot.MEmptyY     ->Alias="MEmptyY";
    Mot.MLoaderY_1  ->Alias="MLoaderY_1";
    Mot.MLoaderY_2  ->Alias="MLoaderY_2";
    Mot.MAutoY_1    ->Alias="MAutoY_1";
    Mot.MAutoY_2    ->Alias="MAutoY_2";
    Mot.MAutoY_3    ->Alias="MAutoY_3";
    Mot.MAutoY_4    ->Alias="MAutoY_4";
    Mot.MAutoY_5    ->Alias="MAutoY_5";
    Mot.MAutoY_6    ->Alias="MAutoY_6";
    Mot.MTopCCDX    ->Alias="MTopCCDX";
    Mot.MBottomCCDY ->Alias="MBottomCCDY";
    Mot.MSuckZ_1    ->Alias="MSuckZ_1";
    Mot.MSuckZ_2    ->Alias="MSuckZ_2";
    Mot.MSuckZ_3    ->Alias="MSuckZ_3";
    Mot.MSuckZ_4    ->Alias="MSuckZ_4";
    Mot.MPitchX     ->Alias="MPitchX";
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::StopAllMotor()
{
    if(MotPtr==NULL)
        return;
    for(int i=0; i<iTotalMotor; i++)
    {
        if(MotPtr[i]!=NULL && MotPtr[i]->GetEnable())
            MotPtr[i]->Stop();
    }
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::DecStopAllMotor()
{
    if(MotPtr==NULL)
        return;
    for(int i=0; i<iTotalMotor; i++)
    {
        if(MotPtr[i]!=NULL && MotPtr[i]->GetEnable())
            MotPtr[i]->DecStop();
    }
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::InitialVMotorName()
{
    VMot.MMSortingArmX->Alias="MMSortingArmX";
    VMot.MMTrayArmX   ->Alias="MMTrayArmX";
    VMot.MMEmptyY     ->Alias="MMEmptyY";
    VMot.MMLoaderY_1  ->Alias="MMLoaderY_1";
    VMot.MMLoaderY_2  ->Alias="MMLoaderY_2";
    VMot.MMAutoY_1    ->Alias="MMAutoY_1";
    VMot.MMAutoY_2    ->Alias="MMAutoY_2";
    VMot.MMAutoY_3    ->Alias="MMAutoY_3";
    VMot.MMAutoY_4    ->Alias="MMAutoY_4";
    VMot.MMAutoY_5    ->Alias="MMAutoY_5";
    VMot.MMAutoY_6    ->Alias="MMAutoY_6";
    VMot.MMSuck_1     ->Alias="MMSuck_1";
    VMot.MMSuck_2     ->Alias="MMSuck_2";
    VMot.MMSuck_3     ->Alias="MMSuck_3";
    VMot.MMSuck_4     ->Alias="MMSuck_4";
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::InitialVMotorParameter()
{
    for(int i=0; i<iTotalVMotor; i++)
    {
        if(VMotPtr[i]==NULL)
            continue;
        VMotPtr[i]->Tag=i;
    }
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::InitialCylinderName()
{
    Cyn.C_TrayArmZ_Up.CylinderName="C_TrayArmZ_Up";
    Cyn.C_TrayArmZ_Down.CylinderName="C_TrayArmZ_Down";
    Cyn.C_TrayArm_FrontClamp.CylinderName="C_TrayArm_FrontClamp";
    Cyn.C_TrayArm_RearClamp.CylinderName="C_TrayArm_RearClamp";

    Cyn.C_Empty_FrontRiseTray_1.CylinderName="C_Empty_FrontRiseTray_1";
    Cyn.C_Empty_FrontRiseTray_2.CylinderName="C_Empty_FrontRiseTray_2";
    Cyn.C_Empty_PushTray.CylinderName="C_Empty_PushTray";
    Cyn.C_Empty_LeanOnTray.CylinderName="C_Empty_LeanOnTray";
    Cyn.C_Empty_FrontSeparateTray_1.CylinderName="C_Empty_FrontSeparateTray_1";
    Cyn.C_Empty_RearRiseTray.CylinderName="C_Empty_RearRiseTray";
    Cyn.C_Empty_RearSeparateTray_1.CylinderName="C_Empty_RearSeparateTray_1";

    Cyn.C_Loader_FrontRiseTray_1.CylinderName="C_Loader_FrontRiseTray_1";
    Cyn.C_Loader_FrontRiseTray_2.CylinderName="C_Loader_FrontRiseTray_2";
    Cyn.C_Loader1_PushTray.CylinderName="C_Loader1_PushTray";
    Cyn.C_Loader2_PushTray.CylinderName="C_Loader2_PushTray";
    Cyn.C_Loader1_LeanOnTray.CylinderName="C_Loader1_LeanOnTray";
    Cyn.C_Loader2_LeanOnTray.CylinderName="C_Loader2_LeanOnTray";
    Cyn.C_Loader_FrontSeparateTray_1.CylinderName="C_Loader_FrontSeparateTray_1";
    Cyn.C_Loader_RearRiseTray.CylinderName="C_Loader_RearRiseTray";

    Cyn.C_Auto1_FrontRiseTray.CylinderName="C_Auto1_FrontRiseTray";
    Cyn.C_Auto1_PushTray.CylinderName="C_Auto1_PushTray";
    Cyn.C_Auto1_LeanOnTray.CylinderName="C_Auto1_LeanOnTray";
    Cyn.C_Auto1_RearRiseTray.CylinderName="C_Auto1_RearRiseTray";
    Cyn.C_Auto1_FrontSeparateTray_1.CylinderName="C_Auto1_FrontSeparateTray_1";

    Cyn.C_Auto2_FrontRiseTray.CylinderName="C_Auto2_FrontRiseTray";
    Cyn.C_Auto2_PushTray.CylinderName="C_Auto2_PushTray";
    Cyn.C_Auto2_LeanOnTray.CylinderName="C_Auto2_LeanOnTray";
    Cyn.C_Auto2_RearRiseTray.CylinderName="C_Auto2_RearRiseTray";
    Cyn.C_Auto2_FrontSeparateTray_1.CylinderName="C_Auto2_FrontSeparateTray_1";

    Cyn.C_Auto3_FrontRiseTray.CylinderName="C_Auto3_FrontRiseTray";
    Cyn.C_Auto3_PushTray.CylinderName="C_Auto3_PushTray";
    Cyn.C_Auto3_LeanOnTray.CylinderName="C_Auto3_LeanOnTray";
    Cyn.C_Auto3_RearRiseTray.CylinderName="C_Auto3_RearRiseTray";
    Cyn.C_Auto3_FrontSeparateTray_1.CylinderName="C_Auto3_FrontSeparateTray_1";

    Cyn.C_Auto4_FrontRiseTray.CylinderName="C_Auto4_FrontRiseTray";
    Cyn.C_Auto4_PushTray.CylinderName="C_Auto4_PushTray";
    Cyn.C_Auto4_LeanOnTray.CylinderName="C_Auto4_LeanOnTray";
    Cyn.C_Auto4_RearRiseTray.CylinderName="C_Auto4_RearRiseTray";
    Cyn.C_Auto4_FrontSeparateTray_1.CylinderName="C_Auto4_FrontSeparateTray_1";

    Cyn.C_Auto5_FrontRiseTray.CylinderName="C_Auto5_FrontRiseTray";
    Cyn.C_Auto5_PushTray.CylinderName="C_Auto5_PushTray";
    Cyn.C_Auto5_LeanOnTray.CylinderName="C_Auto5_LeanOnTray";
    Cyn.C_Auto5_RearRiseTray.CylinderName="C_Auto5_RearRiseTray";
    Cyn.C_Auto5_FrontSeparateTray_1.CylinderName="C_Auto5_FrontSeparateTray_1";

    Cyn.C_Auto6_FrontRiseTray.CylinderName="C_Auto6_FrontRiseTray";
    Cyn.C_Auto6_PushTray.CylinderName="C_Auto6_PushTray";
    Cyn.C_Auto6_LeanOnTray.CylinderName="C_Auto6_LeanOnTray";
    Cyn.C_Auto6_RearRiseTray.CylinderName="C_Auto6_RearRiseTray";
    Cyn.C_Auto6_FrontSeparateTray_1.CylinderName="C_Auto6_FrontSeparateTray_1";

    Cyn.C_Color_FrontRiseTray.CylinderName="C_Color_FrontRiseTray";
    Cyn.C_Color_PushTray.CylinderName="C_Color_PushTray";
    Cyn.C_Color_LeanOnTray.CylinderName="C_Color_LeanOnTray";
    Cyn.C_Color_RearRiseTray.CylinderName="C_Color_RearRiseTray";
    Cyn.C_Color_FrontSeparateTray_1.CylinderName="C_Color_FrontSeparateTray_1";
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::LoadCylinderParameterFromDataBase()
{
    if(CynPtr==NULL)
    {
        CynPtr=(TMyCylinder *)&Cyn;
        iTotalCylinder=sizeof(CYLINDER_MODULAR)/sizeof(TMyCylinder);
    }

    for(int i=0; i<iTotalCylinder; i++)
    {
        TMyCylinder *CynData=&CynPtr[i];
        if(CynData->CylinderName==AnsiString(""))
            continue;

        TIODATA *SwitchData=FindIOData(CynData->CylinderName);
        TIODATA *OnData=FindIOData(CynData->CylinderName+AnsiString("_On"));
        TIODATA *OffData=FindIOData(CynData->CylinderName+AnsiString("_Off"));

        SetupSwitchFromIO(&CynData->Switch, SwitchData, CynData->CylinderName);
        SetupSensorFromIO(&CynData->OnSensor, OnData, CynData->CylinderName+AnsiString("_On"));
        SetupSensorFromIO(&CynData->OffSensor, OffData, CynData->CylinderName+AnsiString("_Off"));

        CynData->OnSensorName=CynData->CylinderName+AnsiString("_On");
        CynData->OffSensorName=CynData->CylinderName+AnsiString("_Off");
        CynData->OnAlarmCode=i*100;
        CynData->OffAlarmCode=i*100+1;
        CynData->OnAlarmTime=GetNonNegativeIOTime((SwitchData==NULL)?0:SwitchData->iOnAlarmTime);
        CynData->OffAlarmTime=GetNonNegativeIOTime((SwitchData==NULL)?0:SwitchData->iOffAlarmTime);
        CynData->OnDelayTime=GetNonNegativeIOTime((SwitchData==NULL)?0:SwitchData->iOnDelayTime);
        CynData->OffDelayTime=GetNonNegativeIOTime((SwitchData==NULL)?0:SwitchData->iOffDelayTime);
        CynData->Enable=IsValidIOData(SwitchData);
        CynData->EnableAtDataBase=CynData->Enable;
        CynData->Tag=i;

        if(CynData->Enable==false)
        {
            CynData->Switch.Enable=false;
            CynData->OnSensor.Enable=false;
            CynData->OffSensor.Enable=false;
        }
    }
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::InitialSwitchName()
{
    Sw.SwFKPowerOff.Name="SwFKPowerOff";
    Sw.SwFKPowerOn.Name="SwFKPowerOn";
    Sw.SwFrontActiveLed.Name="SwFrontActiveLed";
    Sw.SwFKReset.Name="SwFKReset";
    Sw.SwFKPause.Name="SwFKPause";
    Sw.SwFKHome.Name="SwFKHome";
    Sw.SwFKStart.Name="SwFKStart";
    Sw.SwFKOneCycle.Name="SwFKOneCycle";
    Sw.SwFKRetry.Name="SwFKRetry";
    Sw.SwFKSkip.Name="SwFKSkip";
    Sw.SwFKCleanOut.Name="SwFKCleanOut";
    Sw.SwFKTrayFeed.Name="SwFKTrayFeed";
    Sw.SwFKTrayEnd.Name="SwFKTrayEnd";
    Sw.SwFKAlarmReset.Name="SwFKAlarmReset";
    Sw.SwRKPowerOff.Name="SwRKPowerOff";
    Sw.SwRKPowerOn.Name="SwRKPowerOn";
    Sw.SwRKReset.Name="SwRKReset";
    Sw.SwRKPause.Name="SwRKPause";
    Sw.SwRKHome.Name="SwRKHome";
    Sw.SwRKStart.Name="SwRKStart";
    Sw.SwRKOneCycle.Name="SwRKOneCycle";
    Sw.SwRKRetry.Name="SwRKRetry";
    Sw.SwRKSkip.Name="SwRKSkip";
    Sw.SwRKCleanOut.Name="SwRKCleanOut";
    Sw.SwRKTrayFeed.Name="SwRKTrayFeed";
    Sw.SwRKTrayEnd.Name="SwRKTrayEnd";
    Sw.SwRKAlarmReset.Name="SwRKAlarmReset";
    Sw.SwRKManualStep.Name="SwRKManualStep";
    Sw.SwRKManualTStart.Name="SwRKManualTStart";
    Sw.SwTowerRed.Name="SwTowerRed";
    Sw.SwTowerYellow.Name="SwTowerYellow";
    Sw.SwTowerGreen.Name="SwTowerGreen";
    Sw.SwMusic1.Name="SwMusic1";
    Sw.SwMusic2.Name="SwMusic2";
    Sw.SwMusic3.Name="SwMusic3";
    Sw.SwMusic4.Name="SwMusic4";
    Sw.SwMotorRelay.Name="SwMotorRelay";
    Sw.SwServerON.Name="SwServerON";
    Sw.SwRKSafeLock.Name="SwRKSafeLock";
    Sw.SwRearActiveLed.Name="SwRearActiveLed";
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::LoadSwitchParameterFromDataBase()
{
    if(SwPtr==NULL)
    {
        SwPtr=(TMySwitch *)&Sw;
        iTotalSwitch=sizeof(SWITCH_MODULAR)/sizeof(TMySwitch);
    }

    for(int i=0; i<iTotalSwitch; i++)
    {
        if(SwPtr[i].Name==AnsiString(""))
            continue;

        TIODATA *SwitchData=FindIOData(SwPtr[i].Name);
        SetupSwitchFromIO(&SwPtr[i], SwitchData, SwPtr[i].Name);
        SwPtr[i].Tag=i;
    }
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::InitialSuckerName()
{
    Suck.SortArmSuck.initMyKitSuck("Suck", "", 1, 4);
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::LoadSuckerParameterFromDataBase()
{
    if(SuckPtr==NULL)
    {
        SuckPtr=(TMyKitSuck *)&Suck;
        iTotalSucker=sizeof(SUCKER_MODULAR)/sizeof(TMyKitSuck);
    }

    for(int SuckerIndex=0; SuckerIndex<iTotalSucker; SuckerIndex++)
    {
        SuckPtr[SuckerIndex].Tag=SuckerIndex;
        for(int RowIndex=0; RowIndex<SuckPtr[SuckerIndex].MaxItemR; RowIndex++)
        {
            for(int ColIndex=0; ColIndex<SuckPtr[SuckerIndex].MaxItemC; ColIndex++)
            {
                TMySucker *SuckerData=&SuckPtr[SuckerIndex].Suck[RowIndex][ColIndex];
                if(SuckerData->SuckerName==AnsiString(""))
                    continue;

                TIODATA *SensorData=FindIOData(SuckerData->SuckerName);
                TIODATA *OnData=FindIOData(SuckerData->SuckerName+AnsiString("_On"));
                TIODATA *OffData=FindIOData(SuckerData->SuckerName+AnsiString("_Off"));

                SetupSensorFromIO(&SuckerData->Sensor, SensorData, SuckerData->SuckerName);
                SetupSwitchFromIO(&SuckerData->OnSw, OnData, SuckerData->SuckerName+AnsiString("_On"));
                SetupSwitchFromIO(&SuckerData->OffSw, OffData, SuckerData->SuckerName+AnsiString("_Off"));

                SuckerData->SensorName=SuckerData->SuckerName;
                SuckerData->OnPortName=SuckerData->SuckerName+AnsiString("_On");
                SuckerData->OffPortName=SuckerData->SuckerName+AnsiString("_Off");
                SuckerData->OnAlarmCode="0";
                SuckerData->OffAlarmCode="0";
                SuckerData->OnAlarmTime=GetNonNegativeIOTime((SensorData==NULL)?0:SensorData->iOnAlarmTime);
                SuckerData->OffAlarmTime=GetNonNegativeIOTime((SensorData==NULL)?0:SensorData->iOffAlarmTime);
                SuckerData->OnDelayTime=GetNonNegativeIOTime((SensorData==NULL)?0:SensorData->iOnDelayTime);
                SuckerData->OffDelayTime=GetNonNegativeIOTime((SensorData==NULL)?0:SensorData->iOffDelayTime);
                SuckerData->Enable=IsValidIOData(SensorData);
                SuckerData->EnableAtDataBase=SuckerData->Enable;

                if(SuckerData->Enable==false)
                {
                    SuckerData->OnDelayTime=0;
                    SuckerData->OffDelayTime=0;
                    SuckerData->Sensor.Enable=false;
                    SuckerData->OnSw.Enable=false;
                    SuckerData->OffSw.Enable=false;
                }
                else
                {
                    SuckerData->OnSw.Enable=SuckerData->OnSw.Enable && SuckerData->Enable;
                    SuckerData->OffSw.Enable=SuckerData->OffSw.Enable && SuckerData->Enable;
                    SuckerData->Sensor.Enable=SuckerData->Sensor.Enable && SuckerData->Enable;
                }
            }
        }
    }
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::ClearMotTable()
{
    if(MotTable==NULL)
        return;
    for(int i=MotTable->Count-1; i>=0; i--)
    {
        delete (TMOTDATA *)MotTable->Items[i];
        MotTable->Delete(i);
    }
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::ClearIOTable()
{
    if(IOTable==NULL)
        return;
    for(int i=IOTable->Count-1; i>=0; i--)
    {
        delete (TIODATA *)IOTable->Items[i];
        IOTable->Delete(i);
    }
}
//---------------------------------------------------------------------------
TMOTDATA *SYSTEM_MODULAR::FindMotData(AnsiString Alias)
{
    for(int i=0; i<MotTable->Count; i++)
    {
        TMOTDATA *Data=(TMOTDATA *)MotTable->Items[i];
        if(Data!=NULL && Data->Alias==Alias)
            return Data;
    }
    return NULL;
}
//---------------------------------------------------------------------------
TIODATA *SYSTEM_MODULAR::FindIOData(AnsiString Alias)
{
    for(int i=0; i<IOTable->Count; i++)
    {
        TIODATA *Data=(TIODATA *)IOTable->Items[i];
        if(Data!=NULL && Data->Alias==Alias)
            return Data;
    }
    return NULL;
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::LoadMotData()
{
    AnsiString Msg;
    ClearMotTable();
    if(!FileExists(MotTablePath))
    {
        Msg.sprintf("File %s is not exist!", MotTablePath);
        ShowMessage(Msg);
        return;
    }

    TStringList *StrList=new TStringList;
    try
    {
        StrList->LoadFromFile(MotTablePath);
        if(StrList->Count<=1)
        {
            Msg.sprintf("File %s data is lose!", MotTablePath);
            ShowMessage(Msg);
        }
        else
        {
            int Result=MotNo.SetMOTTableNo(StrList->Strings[0]);
            if(Result>=MotNo.emotTotal)
            {
                for(int i=1; i<StrList->Count; i++)
                {
                    if(StrList->Strings[i]==AnsiString(""))
                        continue;
                    TMOTDATA *Data=new TMOTDATA(StrList->Strings[i]);
                    if(Data->Alias!=AnsiString("") && FindMotData(Data->Alias)!=NULL)
                    {
                        Msg.sprintf("Motor %s alias is duplicated!", Data->Alias);
                        ShowMessage(Msg);
                    }
                    MotTable->Add(Data);
                }
            }
            else
            {
                Msg.sprintf("File %s data is mistake! (%d)", MotTablePath, Result);
                ShowMessage(Msg);
            }
        }
    }
    catch(...)
    {
        Msg.sprintf("File %s is opened by other software!", MotTablePath);
        ShowMessage(Msg);
    }
    delete StrList;
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::LoadIoData()
{
    AnsiString Msg;
    ClearIOTable();
    if(!FileExists(IoTablePath))
    {
        Msg.sprintf("File %s is not exist!", IoTablePath);
        ShowMessage(Msg);
        return;
    }

    TStringList *StrList=new TStringList;
    try
    {
        StrList->LoadFromFile(IoTablePath);
        if(StrList->Count<=1)
        {
            Msg.sprintf("File %s data is lose!", IoTablePath);
            ShowMessage(Msg);
        }
        else
        {
            int Result=IoNo.SetIOTableNo(StrList->Strings[0]);
            if(Result>=IoNo.eioTotal)
            {
                for(int RowIndex=1; RowIndex<StrList->Count; RowIndex++)
                {
                    AnsiString Line=StrList->Strings[RowIndex];
                    if(Line==AnsiString("") || Line.SubString(1, 2)==AnsiString("//"))
                        continue;
                    TIODATA *Data=new TIODATA(Line);
                    if(Data->Alias!=AnsiString("") && FindIOData(Data->Alias)!=NULL)
                    {
                        Msg.sprintf("IO %s alias is duplicated!", Data->Alias);
                        ShowMessage(Msg);
                    }
                    IOTable->Add(Data);
                }
            }
            else
            {
                Msg.sprintf("File %s data is mistake! (%d)", IoTablePath, Result);
                ShowMessage(Msg);
            }
        }
    }
    catch(...)
    {
        Msg.sprintf("File %s is opened by other software!", IoTablePath);
        ShowMessage(Msg);
    }
    delete StrList;
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::LoadSingleMotorParameterFromDataBase(int Index, bool bInitial)
{
    LoadMotorParameterFromDataBase(Index, bInitial);
}
//---------------------------------------------------------------------------
void SYSTEM_MODULAR::LoadMotorParameterFromDataBase(int Index, bool bInitial)
{
    AnsiString MotName;
    AnsiString CardModel;
    int iAdder;

    if(Index<-1 || Index>=iTotalMotor)
        return;

    LoadMotData();
    int iStart=(Index==-1)?0:Index;
    int iCount=(Index==-1)?iTotalMotor:Index+1;

    for(int i=iStart; i<iCount; i++)
    {
        TMOTDATA *Data=FindMotData(MotPtr[i]->Alias);
        if(Data!=NULL && Data->No!=AnsiString(""))
            MotName=Data->No;
        else
            MotName.sprintf("M%02d", i+1);

        MotPtr[i]->Number=MotName;
        MotPtr[i]->NumberAlias=AnsiString("[")+MotName+AnsiString("] ")+MotPtr[i]->Alias;
        MotPtr[i]->AlarmName[eMotPwrErr       ]=MotPtr[i]->Alias+"_MotPwrErr";
        MotPtr[i]->AlarmName[eMotTorqueErr    ]=MotPtr[i]->Alias+"_MotTorqueErr";
        MotPtr[i]->AlarmName[eMotCWOnErr      ]=MotPtr[i]->Alias+"_MotCWOnErr";
        MotPtr[i]->AlarmName[eMotCCWOnErr     ]=MotPtr[i]->Alias+"_MotCCWOnErr";
        MotPtr[i]->AlarmName[eMotSoftPErr     ]=MotPtr[i]->Alias+"_MotSoftPErr";
        MotPtr[i]->AlarmName[eMotSoftNErr     ]=MotPtr[i]->Alias+"_MotSoftNErr";
        MotPtr[i]->AlarmName[eMotPosErr       ]=MotPtr[i]->Alias+"_MotPosErr";
        MotPtr[i]->AlarmName[eMotUnDefErr     ]=MotPtr[i]->Alias+"_MotUnDefErr";
        MotPtr[i]->AlarmName[eMotOverLimitErr ]=MotPtr[i]->Alias+"_MotOverLimitErr";

        if(Data==NULL)
        {
            MotPtr[i]->SetEnable(false);
            MotPtr[i]->Tag=i;
            continue;
        }

        CardModel=Data->CardModel;
        MotPtr[i]->CardModel=CardModel;
        if(CardModel=="MC88X1" || CardModel=="MC88X1P" || CardModel=="SMC")
            iAdder=Data->iBoardID*10+Data->iPort;
        else if(CardModel=="MN200" || CardModel=="SYNTEK")
            iAdder=Data->iBoardID*100+Data->iPort;
        else
            iAdder=-1;

        if(bInitial)
        {
            MotPtr[i]->InitialMotorObject(iAdder);
            MotPtr[i]->SetMotNo(i);
        }

        #ifdef SOFT_SIMULATE
            MotPtr[i]->SetEnable(false);
        #else
            MotPtr[i]->SetEnable(Data->iEnable);
        #endif

        if(CardModel=="MC88X1" || CardModel=="MC88X1P")
            MotPtr[i]->SetMotionCardType(eMC88x1);
        else if(CardModel=="SMC")
            MotPtr[i]->SetMotionCardType(eSMC);
        else if(CardModel=="MN200" || CardModel=="SYNTEK")
            MotPtr[i]->SetMotionCardType(eMN200);
        else
            MotPtr[i]->SetMotionCardType(eMotionCardUnknown);

        MotPtr[i]->SetDirection((Data->iDirection==1)?true:false);
        MotPtr[i]->SetHomeDirection((Data->iHomeDirectior==1)?true:false);
        MotPtr[i]->SetGearRatio(Data->dGearRatio);
        MotPtr[i]->SetHomeHighSpeed(Data->iHomeHighSpeed);
        MotPtr[i]->SetHomeLowSpeed(Data->iHomeLowSpeed);
        MotPtr[i]->SetJogHighSpeed(Data->iJogHighSpeed);
        MotPtr[i]->SetJogLowSpeed(Data->iJogLowSpeed);
        MotPtr[i]->SetSoftLimitN(Data->iSoftLimitN);
        MotPtr[i]->SetSoftLimitP(Data->iSoftLimitP);
        MotPtr[i]->SetMotorType((Data->i1P2P==1)?true:false);
        MotPtr[i]->SetSensorType((Data->iSensorType==1)?true:false);
        MotPtr[i]->SetAcc(Data->dAcc);
        MotPtr[i]->SetDec(Data->dDec);
        MotPtr[i]->SetRate(Data->iRate);
        MotPtr[i]->SetRange(Data->iRange);
        MotPtr[i]->SetInitSpeed(Data->iInitSpeed);
        MotPtr[i]->SetHomeOrder(Data->HomeOrder);
        MotPtr[i]->SetServoAlarmOn((Data->iServoAlarmOn==1)?true:false);
        if(Data->MotorKind>=0 && Data->MotorKind<eMotorKindTotal)
            MotPtr[i]->SetMotorKind((eMotorKind)Data->MotorKind);
        else
            MotPtr[i]->SetMotorKind(eMotor);
        MotPtr[i]->FlushPanelName=Data->FlushPanel;
        MotPtr[i]->OriginRange=Data->iRange;
        MotPtr[i]->OriginRate=Data->iRate;
        MotPtr[i]->SimulateSpeed=Data->iSimulateSpeed;
        MotPtr[i]->bIsServoMotor=(Data->iServoAlarmOn==1)?true:false;
        MotPtr[i]->SetLimitLogic((Data->iLimitLogic==1)?true:false);
        MotPtr[i]->SetIn1Logic((Data->iIn1Logic==1)?true:false);
        MotPtr[i]->bHomeFlag=false;
        if(bInitial)
            MotPtr[i]->InitMotor(iAdder);
    }

    for(int i=0; i<iTotalMotor; i++)
    {
        if(MotPtr[i]==NULL)
            continue;
        MotPtr[i]->Tag=i;
        if(MotPtr[i]->GetEnable()==true)
            MotPtr[i]->Stop();
    }
}
//--------------------------------------------------------------------------- 
