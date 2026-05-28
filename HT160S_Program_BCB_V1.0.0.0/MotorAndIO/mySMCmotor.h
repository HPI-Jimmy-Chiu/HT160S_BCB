//---------------------------------------------------------------------------
#ifndef mySMCmotorH
#define mySMCmotorH
//---------------------------------------------------------------------------
#include "HTMotor.h"
//---------------------------------------------------------------------------
class TMySMCMotor: public HTMotor
{
private:
protected:
public:
    __fastcall TMySMCMotor(int Addr);
    virtual ~TMySMCMotor();
    virtual int     InitMotor(int IoAddress);
    virtual void    SetSpeed(unsigned int x);
    virtual void    SetInitSpeed(unsigned int x);
    virtual void    SetServoAlarmOn(bool Value);
    virtual void    SetAcc(double a);
    virtual void    SetDec(double a);
    virtual int     ReadPos();
    virtual void    ScanMotorStatus(bool *Led);
    virtual bool    MoveTo(int Tar);
    virtual bool    MoveToPosShortDistance(int Tar);
    virtual void    Stop();
    virtual void    DecStop();
    virtual bool    JogP();
    virtual bool    JogN();
    virtual bool    HomeObject();
    virtual void    SetRange(unsigned int a);
    virtual bool    GetAlarm(void);
    virtual bool    HomeFlag(void);
    virtual bool    ResetPos(int Pulse=0);
};
//---------------------------------------------------------------------------
#endif