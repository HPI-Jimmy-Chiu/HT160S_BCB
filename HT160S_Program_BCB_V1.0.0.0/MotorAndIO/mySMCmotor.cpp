//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "mySMCmotor.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TMySMCMotor::TMySMCMotor(int Addr)
{
    Address=Addr;
    iBoardID=(Addr>=0)?Addr/10:0;
    iPortID =(Addr>=0)?Addr%10:0;
    SetMotionCardType(eSMC);
}
//---------------------------------------------------------------------------
TMySMCMotor::~TMySMCMotor()
{
}
//---------------------------------------------------------------------------
int TMySMCMotor::InitMotor(int IoAddress)
{
    Address=IoAddress;
    return 0;
}
//---------------------------------------------------------------------------
void TMySMCMotor::SetSpeed(unsigned int x) { HTMotor::SetSpeed(x); }
void TMySMCMotor::SetInitSpeed(unsigned int x) { HTMotor::SetInitSpeed(x); }
void TMySMCMotor::SetServoAlarmOn(bool Value) { HTMotor::SetServoAlarmOn(Value); }
void TMySMCMotor::SetAcc(double a) { HTMotor::SetAcc(a); }
void TMySMCMotor::SetDec(double a) { HTMotor::SetDec(a); }
int  TMySMCMotor::ReadPos() { return ReadEncoderPos(); }
void TMySMCMotor::Stop() {}
void TMySMCMotor::DecStop() {}
bool TMySMCMotor::JogP() { return true; }
bool TMySMCMotor::JogN() { return true; }
bool TMySMCMotor::HomeObject() { return ResetPos(0); }
void TMySMCMotor::SetRange(unsigned int a) { HTMotor::SetRange(a); }
bool TMySMCMotor::GetAlarm(void) { return false; }
bool TMySMCMotor::HomeFlag(void) { return true; }
bool TMySMCMotor::MoveTo(int Tar) { SetCommand(Tar); return true; }
bool TMySMCMotor::MoveToPosShortDistance(int Tar) { return MoveTo(Tar); }
bool TMySMCMotor::ResetPos(int Pulse) { SetCommand(Pulse); return true; }
//---------------------------------------------------------------------------
void TMySMCMotor::ScanMotorStatus(bool *Led)
{
    if(Led==NULL)
        return;
    for(int i=0; i<iMotLedTotalCnt; i++)
        Led[i]=false;
    Led[iInposLed]=true;
}
//---------------------------------------------------------------------------