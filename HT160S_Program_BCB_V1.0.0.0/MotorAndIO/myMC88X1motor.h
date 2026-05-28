//---------------------------------------------------------------------------
#ifndef myMC88X1motorH
#define myMC88X1motorH
//---------------------------------------------------------------------------
#include "HTMotor.h"
//---------------------------------------------------------------------------
class TMyMC88X1Motor: public HTMotor
{
private:
    unsigned char bAxisID;
    unsigned char bMotorStatus;
    unsigned char bLineAxisID;
    unsigned char bDoPort;
    bool bAxisIpBusy;
    bool bCardOpened;
    int OldSpeed;
    double dMC88X1Acc;
    int OldInitSpeed;
    int OldRate;

    bool Open_MC88X1Card();
    void Close_MC88X1Card();
    void SetMC88X1SoftLimit(int iPLimit, int iNLimit);
    void MC88X1SoftLimitEnable(bool bFlag);
    int  ReadMC88X1RealPos();
    int  ReadMC88X1EnCoderRealPos();
    virtual int SetCommand(int p);
    virtual int SetPosition(int p);
    bool MC88X1MotHome();
    void SetMC88X1MotPara();
    bool MotionDone();
    void SetEncodeDir(int iDir);
    void SetEncodeMultiple(int iMultiple);
    void ClearAxisAlarm();

protected:
    int iServoType;
    int iSetpType;
    bool HomeType90();
    void LevalHomeSensorConstDistance(int iPulse);
    int iTempPos;
    int iSearchHome;
    int iDelayReadCount;
    void TouchHomeSensorConstDistance(int Pulse);

public:
    __fastcall TMyMC88X1Motor(int Addr);
    virtual ~TMyMC88X1Motor();

    virtual void    Stop();
    virtual bool    JogP();
    virtual bool    JogN();
    virtual int     ReadPos();
    virtual bool    MoveTo(int Tar);
    virtual bool    HomeObject();
    virtual bool    HomeFlag(void);
    virtual bool    GetAlarm(void);
    virtual void    SetSpeed(unsigned int x);
    virtual void    SetInitSpeed(unsigned int x);
    virtual void    ScanMotorStatus(bool *Led);
    virtual void    SetServoAlarmOn(bool Value);
    virtual int     InitMotor(int IoAddress);
    virtual void    SetRange(unsigned int a);
    virtual void    SetAcc(double a);
    virtual void    SetRate(unsigned int a);
    virtual bool    ResetPos(int Pulse=0);
    virtual int     ReadEncoderPos();
    virtual void    HomeReset();
    virtual void    MotOutputOn(int iOutPort);
    virtual void    MotOutputOff(int iOutPort);
    virtual void    MotInputStatus(bool *bInputPort);
    virtual void    EnableTrigger(int iFlag, int iMode, long lValue);
    virtual bool    LinearAxisMoveTo(int iPortID[8], long lPos[8], bool bFlag);
    virtual bool    ReadStatus(DWORD offset, WORD *ReadData);
    virtual bool    WriteStatus(DWORD offset, WORD WriteData);
    virtual void    MotIpReset();
    virtual bool    IsMotorBusy();
    virtual void    MotorReset();
    virtual bool    IsMotorAlarm();
    virtual void    EnableSetInPosition(bool SetEnable);
    virtual void    SetPos(int pos);

    int iDelayCount;
    int iWaitCount;
    int iStepRange;
};
//---------------------------------------------------------------------------
#endif
