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
// AI(ht160s-maintainer) 20260619 : SMC card SDK is not present in this build
// (no CSMC.lib / CSmc.h). This driver is a fail-safe stub: it must NOT pretend
// an SMC axis works. Jog reports failure, GetAlarm reports alarmed (so the
// kernel halts), HomeFlag never reports homed. All 20 axes use MC88X1 today.
bool TMySMCMotor::JogP() { return false; }
bool TMySMCMotor::JogN() { return false; }
bool TMySMCMotor::HomeObject() { return ResetPos(0); }
void TMySMCMotor::SetRange(unsigned int a) { HTMotor::SetRange(a); }
bool TMySMCMotor::GetAlarm(void) { return true; }   // fail-safe: treat as alarmed (no SMC SDK)
bool TMySMCMotor::HomeFlag(void) { return false; }  // fail-safe: never claim homed (no SMC SDK)
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