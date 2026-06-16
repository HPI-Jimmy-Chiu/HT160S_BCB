//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "myMC88X1motor.h"
#include "MC88X1P_DLL.h"
#include "..\MachineType.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//AI(general) 20260616 : default home type 7 (card-native MC88X1MotHome) like
//legacy HT160. The port hardcoded 90 and dropped the per-motor HomeType config
//load, which forced every axis through the manual HomeType90() seek/leave/touch
//routine; the suck-nozzle Z axes stalled in its phase-B leave-sensor loop and
//never homed in the Teach screen.
#define MC88X1_DEFAULT_HOME_TYPE       7
#define MC88X1_DEFAULT_HOME_STEP       5
#define MC88X1_DEFAULT_HOME_STEP_RANGE 100
#define MAX_MC88X1_CARD                16
//---------------------------------------------------------------------------
static bool MC88X1CardInstall[MAX_MC88X1_CARD]={false,false,false,false,
                                                false,false,false,false,
                                                false,false,false,false,
                                                false,false,false,false};
static int MC88X1CardRefCount[MAX_MC88X1_CARD]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//---------------------------------------------------------------------------
static long RoundPulseByGear(int Pulse, double GearRatio)
{
    if(GearRatio==0)
        return Pulse;
    if(Pulse>=0)
        return (long)(Pulse/GearRatio+0.5);
    return (long)(Pulse/GearRatio-0.5);
}
//---------------------------------------------------------------------------
__fastcall TMyMC88X1Motor::TMyMC88X1Motor(int Addr)
{
    int Board=(Addr>=0)?Addr/10:0;
    int Port =(Addr>=0)?Addr%10:0;
    Address=(Addr>=0)?Addr:0;
    iBoardID=(Board>=0 && Board<MAX_MC88X1_CARD)?Board:0;
    iPortID =(Port>=0 && Port<8)?Port:0;
    bAxisID=(unsigned char)(1<<iPortID);
    bMotorStatus=0;
    bLineAxisID=bAxisID;
    bDoPort=0;
    bAxisIpBusy=false;
    bCardOpened=false;
    OldSpeed=0;
    dMC88X1Acc=0;
    OldInitSpeed=0;
    OldRate=0;
    iServoType=4;
    iSetpType=1;
    iTempPos=0;
    iSearchHome=0;
    iDelayReadCount=0;
    iDelayCount=0;
    iWaitCount=0;
    iHomeType=MC88X1_DEFAULT_HOME_TYPE;
    iHomeStep=MC88X1_DEFAULT_HOME_STEP;
    iHomeStepRange=MC88X1_DEFAULT_HOME_STEP_RANGE;
    iStepRange=iHomeStepRange;
    MotorIdleSafeDoorCheck=NULL;
    SetMotionCardType(eMC88x1);
}
//---------------------------------------------------------------------------
TMyMC88X1Motor::~TMyMC88X1Motor()
{
    Close_MC88X1Card();
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::Open_MC88X1Card()
{
    if(bCardOpened)
        return true;
    if(iBoardID>=MAX_MC88X1_CARD)
        return false;
    if(MC88X1CardInstall[iBoardID]==false)
    {
        if(ERROR_SUCCESS!=MC88X1PMotDevOpen((BYTE)iBoardID))
            return false;
        MC88X1CardInstall[iBoardID]=true;
    }
    MC88X1CardRefCount[iBoardID]++;
    bCardOpened=true;
    return true;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::Close_MC88X1Card()
{
    if(!bCardOpened || iBoardID>=MAX_MC88X1_CARD)
        return;
    if(MC88X1CardRefCount[iBoardID]>0)
        MC88X1CardRefCount[iBoardID]--;
    if(MC88X1CardRefCount[iBoardID]==0 && MC88X1CardInstall[iBoardID])
    {
        MC88X1CardInstall[iBoardID]=false;
        MC88X1PMotDevClose((BYTE)iBoardID);
    }
    bCardOpened=false;
}
//---------------------------------------------------------------------------
int TMyMC88X1Motor::InitMotor(int IoAddress)
{
    int Board;
    int Port;

    Address=(IoAddress>=0)?IoAddress:0;
    Board=(IoAddress>=0)?IoAddress/10:-1;
    Port =(IoAddress>=0)?IoAddress%10:-1;
    if(Board<0 || Board>=MAX_MC88X1_CARD || Port<0 || Port>=8)
    {
        Enable=false;
        bAxisID=0;
        bLineAxisID=0;
        return 0;
    }
    iBoardID=Board;
    iPortID=Port;
    bAxisID=(unsigned char)(1<<iPortID);
    bLineAxisID=bAxisID;

    //AI(general) 20260612 : open the axis card at boot even when the motor is
    //disabled in Mot_Table, so a motor enabled later from the motor-test screen
    //(reload uses bInitial=false and never calls InitMotor again) still finds an
    //opened card. Same root cause as the MN200 open-at-boot fix. Legacy HT160
    //opened the card unconditionally in the constructor. Skipped in simulation.
    if(!Enable)
    {
#ifndef SOFT_SIMULATE
        Open_MC88X1Card();
#endif
        return 0;
    }
    if(Open_MC88X1Card()==false)
    {
        Enable=false;
        return 0;
    }

    SetMC88X1SoftLimit(SoftLimitP, SoftLimitN);
    MC88X1SoftLimitEnable(false);
    MC88X1PSetNLimitLogic((BYTE)iBoardID, bAxisID, (USHORT)bSensorType);
    MC88X1PSetPLimitLogic((BYTE)iBoardID, bAxisID, (USHORT)bSensorType);
    MC88X1PSetHomeLogic((BYTE)iBoardID, bAxisID, (USHORT)bSensorType);
    ResetPos(0);

    if(iHomeType!=90)
        MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeType, iHomeType);
    else
        MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeType, 8);

    if(ServoAlarmOn)
    {
        MC88X1PSetInposition((BYTE)iBoardID, bAxisID, 0x01, 0);
        MC88X1PSetServoAlarm((BYTE)iBoardID, bAxisID, 1, 1);
    }
    else
    {
        MC88X1PSetInposition((BYTE)iBoardID, bAxisID, 0x00, 0);
        MC88X1PSetServoAlarm((BYTE)iBoardID, bAxisID, 0, 0);
        MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeType, 8);
    }

    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeP0_Dir, HomeDirection);
    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeP0_Speed, HomeHighSpeed*Range);
    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeP1_Dir, !HomeDirection);
    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeP1_Speed, HomeLowSpeed*Range);
    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeOffset, 0);
    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeP2_Dir, HomeDirection);
    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeOffset_Speed, HomeLowSpeed*Range);
    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeLimitAlarm, 0);

    if(MotorType)
        MC88X1PSetPulseMode((BYTE)iBoardID, bAxisID, (BYTE)iServoType);
    else
        MC88X1PSetPulseMode((BYTE)iBoardID, bAxisID, (BYTE)iSetpType);

    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeClearPos, 0x00);
    SetSpeed(JogLowSpeed);
    SetEncodeDir(1);
    SetEncodeMultiple(3);
    if(IoAddress==13)
        SetEncodeMultiple(1);
    return 0;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetMC88X1SoftLimit(int iPLimit, int iNLimit)
{
    long LP;
    long LN;
    if(GearRatio==0)
        return;
    if(Direction)
    {
        LP=(long)(-iNLimit/GearRatio);
        LN=(long)(-iPLimit/GearRatio);
    }
    else
    {
        LP=(long)(iPLimit/GearRatio);
        LN=(long)(iNLimit/GearRatio);
    }
    if(Enable && bCardOpened)
    {
        MC88X1PSetCompNLimit((BYTE)iBoardID, bAxisID, LN);
        MC88X1PSetCompPLimit((BYTE)iBoardID, bAxisID, LP);
    }
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::MC88X1SoftLimitEnable(bool bFlag)
{
    if(Enable && bCardOpened)
        MC88X1PEnableCompLimit((BYTE)iBoardID, bAxisID, bFlag?0x01:0x00);
}
//---------------------------------------------------------------------------
int TMyMC88X1Motor::ReadMC88X1RealPos()
{
    long Pos=0;
    if(Enable && bCardOpened && ERROR_SUCCESS==MC88X1PGetTheorecticalRegister((BYTE)iBoardID, bAxisID, &Pos))
    {
        if(Direction)
            Pos=-Pos;
        return (int)(Pos*GearRatio);
    }
    return HTMotor::ReadPos();
}
//---------------------------------------------------------------------------
int TMyMC88X1Motor::ReadMC88X1EnCoderRealPos()
{
    long Pos=0;
    if(Enable && bCardOpened && ERROR_SUCCESS==MC88X1PGetPracticalRegister((BYTE)iBoardID, bAxisID, &Pos))
    {
        if(Direction==0)
            Pos=-Pos;
        return (int)(Pos*GearRatio);
    }
    return HTMotor::ReadEncoderPos();
}
//---------------------------------------------------------------------------
int TMyMC88X1Motor::SetCommand(int p)
{
    long Pos;
    HTMotor::SetCommand(p);
    if(Enable && bCardOpened && GearRatio!=0)
    {
        Pos=(long)(p/GearRatio);
        if(Direction)
            Pos=-Pos;
        MC88X1PSetTheorecticalRegister((BYTE)iBoardID, bAxisID, Pos);
    }
    return 0;
}
//---------------------------------------------------------------------------
int TMyMC88X1Motor::SetPosition(int p)
{
    long Pos;
    HTMotor::SetPosition(p);
    if(Enable && bCardOpened && GearRatio!=0)
    {
        Pos=(long)(p/GearRatio);
        if(Direction==0)
            Pos=-Pos;
        MC88X1PSetPracticalRegister((BYTE)iBoardID, bAxisID, Pos);
    }
    return 0;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::MotionDone()
{
    if(Enable && bCardOpened)
        return (ERROR_SUCCESS==MC88X1PMotAxisBusy((BYTE)iBoardID, bAxisID));
    return false;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetEncodeDir(int iDir)
{
    if(Enable && bCardOpened)
        MC88X1PSetEncoderDir((BYTE)iBoardID, bAxisID, (USHORT)iDir);
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetEncodeMultiple(int iMultiple)
{
    if(Enable && bCardOpened)
        MC88X1PSetEncoderMultiple((BYTE)iBoardID, bAxisID, (USHORT)iMultiple);
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::ClearAxisAlarm()
{
    LRESULT Status;
    if(!Enable || !bCardOpened)
        return;
    Status=MC88X1PMotAxisBusy((BYTE)iBoardID, bAxisID);
    if(Status==AxisIpBusy)
        MotIpReset();
    else if(Status==AxisHomeBusy || Status==AxisHomeErr)
        HomeReset();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::Stop()
{
    if(Enable && bCardOpened)
        MC88X1PMotStop((BYTE)iBoardID, bAxisID, 0);
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::JogP()
{
    if(!Enable || !bCardOpened)
        return false;
    if(MotorIdleSafeDoorCheck!=NULL && MotorIdleSafeDoorCheck()==true)
        return false;
    if(!MotionDone())
        return false;
    if(Direction)
        MC88X1PMotCmove((BYTE)iBoardID, bAxisID, bAxisID);
    else
        MC88X1PMotCmove((BYTE)iBoardID, bAxisID, 0x00);
    return true;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::JogN()
{
    if(!Enable || !bCardOpened)
        return false;
    if(MotorIdleSafeDoorCheck!=NULL && MotorIdleSafeDoorCheck()==true)
        return false;
    if(!MotionDone())
        return false;
    if(Direction)
        MC88X1PMotCmove((BYTE)iBoardID, bAxisID, 0x00);
    else
        MC88X1PMotCmove((BYTE)iBoardID, bAxisID, bAxisID);
    return true;
}
//---------------------------------------------------------------------------
int TMyMC88X1Motor::ReadPos()
{
    return ReadMC88X1RealPos();
}
//---------------------------------------------------------------------------
int TMyMC88X1Motor::ReadEncoderPos()
{
    return ReadMC88X1EnCoderRealPos();
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::MoveTo(int Tar)
{
    long Distance[8]={0,0,0,0,0,0,0,0};
    long TargetPulse;
    long NowPulse;
    if(MotorIdleSafeDoorCheck!=NULL && MotorIdleSafeDoorCheck()==true)
        return false;
    if(!Enable)
    {
        SetCommand(Tar);
        SetPosition(Tar);
        return true;
    }
    if(!bCardOpened || GearRatio==0 || iPortID>=8)
        return false;
    if(!CheckArmPosInRange(Tar, SoftLimitN, SoftLimitP))
        return false;
    if(!MotionDone())
        return false;
    TargetPulse=RoundPulseByGear(Tar, GearRatio);
    Distance[iPortID]=Direction?-TargetPulse:TargetPulse;
    MC88X1PMotPtp((BYTE)iBoardID, bAxisID, bAxisID,
                  Distance[0], Distance[1], Distance[2], Distance[3],
                  Distance[4], Distance[5], Distance[6], Distance[7]);
    //AI(general) 20260612 : compare raw card pulses like legacy HT160. The unit
    //domain compare (ReadPos()/GearRatio) truncates and never matches for the
    //fractional gear ratios in Mot_Table (0.9, 0.5, 0.1, 0.04), so the move task
    //would never report done.
    NowPulse=0;
    if(MotionDone() && ERROR_SUCCESS==MC88X1PGetTheorecticalRegister((BYTE)iBoardID, bAxisID, &NowPulse))
    {
        if(Direction)
            NowPulse=-NowPulse;
        if(NowPulse==TargetPulse)
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::HomeObject()
{
    if(MotorIdleSafeDoorCheck!=NULL && MotorIdleSafeDoorCheck()==true)
        return false;
    if(iHomeType==90 && iHomeStep>0)
        return HomeType90();
    return MC88X1MotHome();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::TouchHomeSensorConstDistance(int Pulse)
{
    DWORD Data;
    WORD Data1;
    WORD Data2;

    if(GearRatio!=0)
        Pulse=(int)(Pulse/GearRatio);
    if(Pulse==0)
        Pulse=1;
    Data=(DWORD)Pulse;
    Data1=(WORD)(Data>>16);
    Data2=(WORD)(Data & 0xFFFF);
    WriteStatus(0x00, Data1);
    WriteStatus(0x02, Data2);
    WriteStatus(0x04, 0x15);
    if(HomeDirection)
        WriteStatus(0x04, 0x21);
    else
        WriteStatus(0x04, 0x20);
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::LevalHomeSensorConstDistance(int iPulse)
{
    DWORD Data;
    WORD Data1;
    WORD Data2;

    if(GearRatio!=0)
        iPulse=(int)(iPulse/GearRatio);
    if(iPulse==0)
        iPulse=1;
    Data=(DWORD)iPulse;
    Data1=(WORD)(Data>>16);
    Data2=(WORD)(Data & 0xFFFF);
    WriteStatus(0x00, Data1);
    WriteStatus(0x02, Data2);
    WriteStatus(0x04, 0x15);
    if(HomeDirection)
        WriteStatus(0x04, 0x20);
    else
        WriteStatus(0x04, 0x21);
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::HomeType90()
{
    int &Task=iHomeObjectTask;
    unsigned int iAccPersent;
    unsigned int iAccMax;
    bool bScanLed[iMotLedTotalCnt];

    switch(Task)
    {
        case 1:
            if(!MotionDone())
            {
                ClearAxisAlarm();
                break;
            }
            iWaitCount=0;
            iDelayCount=0;
            iStepRange=iHomeStepRange;
            if(iStepRange<5)
                iStepRange=5;
            else if(iStepRange>500)
                iStepRange=500;
            SetCommand(0);
            SetPosition(0);
            OldSpeed=iSpeed;
            OldInitSpeed=InitSpeed;
            Stop();
            InitMotor(Address);
            MC88X1SoftLimitEnable(false);
            Task=10;
            break;
        case 10:
            iAccPersent=(HomeHighSpeed*Range)/65535;
            if(iAccPersent==0)
                iAccPersent=1;
            iAccMax=iAccPersent*8192000;
            MC88X1PMotAxisParaSet((BYTE)iBoardID, bAxisID, 0,
                                  (HomeLowSpeed*Range)/2,
                                  HomeLowSpeed*Range,
                                  HomeHighSpeed*Range,
                                  iAccMax, 100);
            SetInitSpeed(1);
            SetSpeed(HomeHighSpeed);
            iSearchHome=0;
            Task=11;
            break;
        case 11:
            ScanMotorStatus(bScanLed);
            if(bScanLed[iHomeLed])
            {
                Task=20;
                break;
            }
            else if(bScanLed[iCcwLed])
            {
                JogP();
                Task=12;
            }
            else if(bScanLed[iCwLed])
            {
                JogN();
                Task=12;
            }
            else
            {
                if(Direction)
                {
                    if(HomeDirection)
                        JogP();
                    else
                        JogN();
                }
                else
                {
                    if(HomeDirection)
                        JogN();
                    else
                        JogP();
                }
                Task=20;
            }
            break;
        case 12:
            ScanMotorStatus(bScanLed);
            if(bScanLed[iCcwLed]==false || bScanLed[iCwLed]==false)
                Task=20;
            break;
        case 20:
            ScanMotorStatus(bScanLed);
            if(bScanLed[iHomeLed])
            {
                Stop();
                Task=21;
            }
            else if(bScanLed[iCwLed] || bScanLed[iCcwLed])
            {
                Stop();
                if(iSearchHome>=1)
                    return false;
                iSearchHome++;
                Task=22;
            }
            break;
        case 21:
            if(MotionDone())
            {
                Task=100;
                iTempPos=ReadPos();
            }
            break;
        case 22:
            if(MotionDone())
            {
                iSearchHome++;
                Task=11;
            }
            break;
        case 100:
            if(MotionDone()==false)
                break;
            SetSpeed(HomeLowSpeed);
            LevalHomeSensorConstDistance(iStepRange);
            iWaitCount=0;
            Task=110;
        case 110:
            if(MotionDone()==false)
                break;
            if(iWaitCount<=10)
            {
                iWaitCount++;
                break;
            }
            iWaitCount=0;
            if(HomeFlag())
            {
                Task=100;
                break;
            }
            iStepRange=iStepRange/2;
            if(iStepRange<=0)
                iStepRange=1;
            Task=120;
        case 120:
            if(MotionDone()==false)
                break;
            iWaitCount=0;
            Task=130;
        case 130:
            if(MotionDone()==false)
                break;
            if(iWaitCount<=10)
            {
                iWaitCount++;
                break;
            }
            iWaitCount=0;
            if(HomeFlag())
            {
                if(iStepRange<=iHomeStep)
                {
                    if(iDelayCount>=0 && iDelayCount<3)
                    {
                        iDelayCount++;
                        break;
                    }
                    Task=200;
                }
                else
                {
                    iStepRange=iStepRange/2;
                    if(iStepRange<=0)
                        iStepRange=1;
                    Task=100;
                }
            }
            else
            {
                if(iStepRange<=iHomeStep)
                    TouchHomeSensorConstDistance(1);
                else
                    TouchHomeSensorConstDistance(iStepRange);
            }
            iDelayCount=0;
            break;
        case 200:
            if(HomeFlag()==false)
            {
                Task=100;
                return false;
            }
            LastHomePos=ReadMC88X1RealPos();
            SetCommand(0);
            SetPosition(0);
            SetMC88X1SoftLimit(SoftLimitP, SoftLimitN);
            MC88X1SoftLimitEnable(true);
            SetInitSpeed(OldInitSpeed);
            SetSpeed(OldSpeed);
            Task=1;
            return true;
        default:
            Task=1;
            return false;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::MC88X1MotHome()
{
    int &Task=iHomeObjectTask;
    unsigned int iAccPersent;
    unsigned int iAccMax;
    switch(Task)
    {
        case 1:
            if(!MotionDone())
            {
                ClearAxisAlarm();
                break;
            }
            ResetPos(0);
            OldSpeed=iSpeed;
            Stop();
            InitMotor(Address);
            MC88X1SoftLimitEnable(false);
            LastHomePos=0;
            Task=10;
            break;
        case 10:
            iAccPersent=(HomeHighSpeed*Range)/65535;
            if(iAccPersent==0)
                iAccPersent=1;
            iAccMax=iAccPersent*8192000;
            MC88X1PMotAxisParaSet((BYTE)iBoardID, bAxisID, 0,
                                  (HomeLowSpeed*Range)/2,
                                  HomeLowSpeed*Range,
                                  HomeHighSpeed*Range,
                                  iAccMax, 100);
            Task=11;
            break;
        case 11:
            if(ERROR_SUCCESS==MC88X1PMotHome((BYTE)iBoardID, bAxisID))
                Task=20;
            break;
        case 20:
            if(ERROR_SUCCESS==MC88X1PMotHomeStatus((BYTE)iBoardID, bAxisID))
                Task=30;
            break;
        case 30:
            SetPosition(0);
            Task=31;
            break;
        case 31:
            if(ReadMC88X1EnCoderRealPos()==0)
                Task=32;
            else
                Task=30;
            break;
        case 32:
            LastHomePos=ReadMC88X1RealPos();
            SetCommand(0);
            SetMC88X1SoftLimit(SoftLimitP, SoftLimitN);
            MC88X1SoftLimitEnable(true);
            SetSpeed(OldSpeed);
            Task=1;
            return true;
        default:
            Task=1;
            return false;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::HomeFlag(void)
{
    WORD Data2;
    bool Flag;
    if(!Enable || !bCardOpened)
        return true;
    if(iHomeType==90)
    {
        MC88X1PMotDI((BYTE)iBoardID, bAxisID, &bMotorStatus);
        Flag=(bMotorStatus & 0x08)!=0;
        return bSensorType?Flag:!Flag;
    }
    ReadStatus(0x08, &Data2);
    Flag=(Data2 & 0x0080)!=0;
    return bSensorType?!Flag:Flag;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::GetAlarm(void)
{
    if(!Enable || !ServoAlarmOn)
        return false;
    if(bCardOpened)
        MC88X1PMotDI((BYTE)iBoardID, bAxisID, &bMotorStatus);
    return (bMotorStatus & 0x80)!=0;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetMC88X1MotPara()
{
    double AccSec;
    unsigned int iAccPersent;
    unsigned int iAccMax;
    unsigned int iAccMin;
    unsigned int iChangSpeed[8]={1,1,1,1,1,1,1,1};
    if(!Enable || !bCardOpened || Range==0)
        return;
    if(MotionDone())
    {
        AccSec=(dAcc<=0)?0.1:dAcc;
        dMC88X1Acc=(iSpeed*Range-InitSpeed*Range)/AccSec;
        iAccPersent=(iSpeed*Range)/65535;
        if(iAccPersent==0)
            iAccPersent=1;
        iAccMin=iAccPersent*2001;
        iAccMax=iAccPersent*8192000;
        if(dMC88X1Acc<iAccMin)
            dMC88X1Acc=iAccMin;
        if(dMC88X1Acc>iAccMax)
            dMC88X1Acc=iAccMax;
        MC88X1PMotAxisParaSet((BYTE)iBoardID, bAxisID, 0,
                              InitSpeed*Range, iSpeed*Range,
                              JogHighSpeed*Range, (DWORD)dMC88X1Acc, 100);
    }
    else if(iPortID<8)
    {
        iChangSpeed[iPortID]=iSpeed*Range;
        MC88X1PMotChgDV((BYTE)iBoardID, bAxisID,
                        iChangSpeed[0], iChangSpeed[1], iChangSpeed[2], iChangSpeed[3],
                        iChangSpeed[4], iChangSpeed[5], iChangSpeed[6], iChangSpeed[7]);
    }
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetSpeed(unsigned int x)
{
    if(x>8191)
        x=8191;
    else if(x<1)
        x=1;
    if(x<InitSpeed)
        x=InitSpeed*2;
    if(x>8191)
        x=8191;
    iSpeed=x;
    SetMC88X1MotPara();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetInitSpeed(unsigned int x)
{
    InitSpeed=x;
    SetMC88X1MotPara();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::ScanMotorStatus(bool *Led)
{
    unsigned short MotionStatus;
    bool Flag;
    if(Led==NULL)
        return;
    for(int i=0; i<iMotLedTotalCnt; i++)
        Led[i]=false;
    if(!Enable || !bCardOpened)
        return;
    MC88X1PMotDI((BYTE)iBoardID, bAxisID, &bMotorStatus);
    Led[iInposLed]=(bMotorStatus & 0x40)!=0;
    Led[iAlarmLed]=ServoAlarmOn?((bMotorStatus & 0x80)!=0):false;
    Flag=(bMotorStatus & 0x08)!=0;
    Led[iHomeLed]=bSensorType?Flag:!Flag;
    Led[iEmgLed]=!(bMotorStatus & 0x02);
    Led[iZPhaseLed]=(bMotorStatus & 0x01)!=0;
    MC88X1PGetMotionInput((BYTE)iBoardID, bAxisID, &MotionStatus);
    Led[iCwLed]=(MotionStatus & 0x04)!=0;
    Led[iCcwLed]=(MotionStatus & 0x08)!=0;
    Led[iSoftcwLed]=(MotionStatus & 0x01)!=0;
    Led[iSoftccwLed]=(MotionStatus & 0x02)!=0;
    Led[iServoalarmLed]=(MotionStatus & 0x10)!=0;
    Led[iServoOn]=Enable;
    if(Led[iCwLed] || Led[iCcwLed])
        Led[iAlarmLed]=true;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetServoAlarmOn(bool Value)
{
    ServoAlarmOn=Value;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetRange(unsigned int a)
{
    if(a>1000)
        a=1000;
    if(a<1)
        a=1;
    Range=a;
    SetMC88X1MotPara();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetAcc(double a)
{
    dAcc=a;
    SetMC88X1MotPara();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetRate(unsigned int a)
{
    if(dAcc!=0)
        return;
    if(a<1)
        a=1;
    Rate=a;
    dAcc=(JogHighSpeed-InitSpeed)*Rate/8000000.0;
    SetMC88X1MotPara();
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::ResetPos(int Pulse)
{
    SetCommand(Pulse);
    SetPosition(Pulse);
    return true;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::HomeReset()
{
    if(Enable && bCardOpened)
        MC88X1PMotHomeReset((BYTE)iBoardID, bAxisID);
    HTMotor::HomeReset();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::MotOutputOn(int iOutPort)
{
    if(!Enable || !bCardOpened || iOutPort<4 || iOutPort>7)
        return;
    bDoPort=(unsigned char)(bDoPort | (1<<(iOutPort-4)));
    MC88X1PMotDO((BYTE)iBoardID, bAxisID, bDoPort);
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::MotOutputOff(int iOutPort)
{
    unsigned char bMask;
    if(!Enable || !bCardOpened || iOutPort<4 || iOutPort>7)
        return;
    bMask=(unsigned char)(1<<(iOutPort-4));
    bDoPort=(unsigned char)(bDoPort & (~bMask));
    MC88X1PMotDO((BYTE)iBoardID, bAxisID, bDoPort);
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::MotInputStatus(bool *bInputPort)
{
    if(bInputPort==NULL)
        return;
    bInputPort[0]=false;
    bInputPort[1]=false;
    if(Enable && bCardOpened)
    {
        MC88X1PMotDI((BYTE)iBoardID, bAxisID, &bMotorStatus);
        bInputPort[0]=!(bMotorStatus & 0x02);
        bInputPort[1]=!(bMotorStatus & 0x04);
    }
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::EnableTrigger(int iFlag, int iMode, long lValue)
{
    long lPitch;
    if(!Enable || !bCardOpened || GearRatio==0)
        return;
    if(iFlag)
    {
        lPitch=(long)(lValue/GearRatio);
        if(iMode==1)
            MC88X1PSetPichPulseMode((BYTE)iBoardID, bAxisID, 0x01);
        else if(iMode==2)
            MC88X1PSetPichPulseMode((BYTE)iBoardID, bAxisID, 0x02);
        else if(iMode==3)
            MC88X1PSetPichPulseMode((BYTE)iBoardID, bAxisID, 0x03);
        MC88X1PSetPichPulseCounter((BYTE)iBoardID, bAxisID, 0x00);
        MC88X1PSetPichData((BYTE)iBoardID, bAxisID, (WORD)lPitch);
    }
    else
        MC88X1PSetPichPulseMode((BYTE)iBoardID, bAxisID, 0x00);
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::LinearAxisMoveTo(int iPortID[8], long lPos[8], bool bFlag)
{
    if(MotorIdleSafeDoorCheck!=NULL && MotorIdleSafeDoorCheck()==true)
        return false;
    if(!Enable || !bCardOpened)
        return false;
    if(bFlag==false)
    {
        bLineAxisID=0;
        for(int i=0; i<8; i++)
        {
            if(iPortID[i]!=-1)
                bLineAxisID=(unsigned char)(bLineAxisID+(1<<iPortID[i]));
        }
        if(AxisIpBusy==MC88X1PMotAxisBusy((BYTE)iBoardID, bAxisID))
            MotIpReset();
        MC88X1PMotLine((BYTE)iBoardID, bLineAxisID, 255,
                       lPos[0], lPos[1], lPos[2], lPos[3],
                       lPos[4], lPos[5], lPos[6], lPos[7]);
    }
    else
    {
        if(ERROR_SUCCESS==MC88X1PMotAxisBusy((BYTE)iBoardID, bLineAxisID))
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::ReadStatus(DWORD offset, WORD *ReadData)
{
    DWORD Port;
    WORD Data=0;
    if(ReadData==NULL)
        return false;
    Port=(0x80)*(iPortID/4)+(0x10)*(iPortID%4);
    if(Enable && bCardOpened)
    {
        if(MC88X1PReadWord((BYTE)iBoardID, offset+Port, &Data)==FALSE)
            return false;
    }
    *ReadData=Data;
    return true;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::WriteStatus(DWORD offset, WORD WriteData)
{
    DWORD Port;
    Port=(0x80)*(iPortID/4)+(0x10)*(iPortID%4);
    if(Enable && bCardOpened)
        return MC88X1PWriteWord((BYTE)iBoardID, offset+Port, WriteData)!=FALSE;
    return true;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::MotIpReset()
{
    if(Enable && bCardOpened)
        MC88X1PMotIpReset((BYTE)iBoardID);
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::IsMotorBusy()
{
    return !MotionDone();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::MotorReset()
{
    if(Enable && bCardOpened)
        MC88X1PMotReset((BYTE)iBoardID);
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::IsMotorAlarm()
{
    if(!Enable || !bCardOpened)
        return false;
    MC88X1PMotDI((BYTE)iBoardID, bAxisID, &bMotorStatus);
    return (bMotorStatus & 0x80)!=0;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::EnableSetInPosition(bool SetEnable)
{
    if(!Enable || !bCardOpened)
        return;
    if(SetEnable)
        MC88X1PSetInposition((BYTE)iBoardID, bAxisID, 0x01, 0);
    else
        MC88X1PSetInposition((BYTE)iBoardID, bAxisID, 0x00, 0);
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetPos(int pos)
{
    //AI(general) 20260612 : SetCommand of this class already negates by Direction
    //internally (legacy SetCommand did not), so the legacy pre-negation here would
    //double-negate the command register and ReadPos would return -pos.
    SetCommand(pos);
    SetPosition(pos);
}
//---------------------------------------------------------------------------
