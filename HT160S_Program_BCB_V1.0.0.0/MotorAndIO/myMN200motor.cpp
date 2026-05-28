//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "myMN200motor.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TMyMN200Motor::TMyMN200Motor(int Addr)
{
    Address=Addr;
    iBoardID=(Addr>=0)?Addr/100:0;
    iPortID =(Addr>=0)?Addr%100:0;
    SetMotionCardType(eMN200);
}
//---------------------------------------------------------------------------
TMyMN200Motor::~TMyMN200Motor()
{
}
//---------------------------------------------------------------------------
int TMyMN200Motor::InitMotor(int IoAddress)
{
    Address=IoAddress;
    return 0;
}
//---------------------------------------------------------------------------
void TMyMN200Motor::SetSpeed(unsigned int x) { HTMotor::SetSpeed(x); }
void TMyMN200Motor::SetInitSpeed(unsigned int x) { HTMotor::SetInitSpeed(x); }
void TMyMN200Motor::SetServoAlarmOn(bool Value) { HTMotor::SetServoAlarmOn(Value); }
void TMyMN200Motor::SetAcc(double a) { HTMotor::SetAcc(a); }
void TMyMN200Motor::SetDec(double a) { HTMotor::SetDec(a); }
int  TMyMN200Motor::ReadPos() { return ReadEncoderPos(); }
void TMyMN200Motor::Stop() {}
void TMyMN200Motor::DecStop() {}
bool TMyMN200Motor::JogP() { return true; }
bool TMyMN200Motor::JogN() { return true; }
bool TMyMN200Motor::HomeObject() { return ResetPos(0); }
void TMyMN200Motor::SetRange(unsigned int a) { HTMotor::SetRange(a); }
bool TMyMN200Motor::GetAlarm(void) { return false; }
bool TMyMN200Motor::HomeFlag(void) { return true; }
bool TMyMN200Motor::MoveTo(int Tar) { SetCommand(Tar); return true; }
bool TMyMN200Motor::ResetPos(int Pulse) { SetCommand(Pulse); return true; }
//---------------------------------------------------------------------------
void TMyMN200Motor::ScanMotorStatus(bool *Led)
{
    if(Led==NULL)
        return;
    for(int i=0; i<iMotLedTotalCnt; i++)
        Led[i]=false;
    Led[iInposLed]=true;
}
//---------------------------------------------------------------------------