//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "HTMotor.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
HTMotor::HTMotor()
{
    Address         =0;
    iBoardID        =0;
    iPortID         =0;
    iSpeed          =0;
    InitSpeed       =10;
    Rate            =100;
    Range           =20;
    EncoderPos      =0;
    iHomeObjectTask =0;
    dAcc            =0;
    dDec            =0;
    MotorKind       =eMotor;
    MotionCardType  =eMotionCardUnknown;
    HomeHighSpeed   =100;
    HomeLowSpeed    =100;
    JogHighSpeed    =100;
    JogLowSpeed     =100;
    Enable          =false;
    Direction       =true;
    HomeDirection   =true;
    MotorType       =true;
    bSensorType     =true;
    bLimitLogic     =false;
    bIn1Logic       =false;
    GearRatio       =1;
    SoftLimitP      =999999;
    SoftLimitN      =-999999;
    LastHomePos     =0;
    EncoderType     =0;
    ServoAlarmOn    =false;
    iMotNo          =-1;
    iHomeType       =0;
    iHomeStep       =0;
    iHomeStepRange  =0;
    bNeedHome       =false;
    bThreadHome     =false;
    MotorIdleSafeDoorCheck=NULL;
}
//---------------------------------------------------------------------------
HTMotor::~HTMotor()
{
}
//---------------------------------------------------------------------------
unsigned int HTMotor::ReadSpeed()
{
    return iSpeed;
}
//---------------------------------------------------------------------------
unsigned int HTMotor::ReadInitSpeed()
{
    return InitSpeed;
}
//---------------------------------------------------------------------------
unsigned int HTMotor::ReadRate()
{
    return Rate;
}
//---------------------------------------------------------------------------
unsigned int HTMotor::ReadRange()
{
    return Range;
}
//---------------------------------------------------------------------------
double HTMotor::ReadAcc()
{
    return dAcc;
}
//---------------------------------------------------------------------------
double HTMotor::ReadDec()
{
    return dDec;
}
//---------------------------------------------------------------------------
bool HTMotor::ReadServoAlarmOn()
{
    return ServoAlarmOn;
}
//---------------------------------------------------------------------------
int HTMotor::ReadEncoderPos()
{
    return EncoderPos;
}
//---------------------------------------------------------------------------
void HTMotor::SetHomeobjectTask(int Task)
{
    iHomeObjectTask=Task;
}
//---------------------------------------------------------------------------
bool HTMotor::CheckArmPosInRange(int iNowPos, int iMin, int iMax)
{
    int iTemp;
    if(iMin>iMax)
    {
        iTemp=iMax;
        iMax=iMin;
        iMin=iTemp;
    }
    return (iMin<=iNowPos && iNowPos<=iMax);
}
//---------------------------------------------------------------------------
bool HTMotor::CheckArmPosArrival(int iNowPos, int iDestination, int iTolerance)
{
    return CheckArmPosInRange(iNowPos, iDestination-iTolerance, iDestination+iTolerance);
}
//---------------------------------------------------------------------------