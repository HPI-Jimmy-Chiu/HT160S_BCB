//---------------------------------------------------------------------------
#ifndef HTMotorH
#define HTMotorH
//---------------------------------------------------------------------------
#include <windows.h>
//---------------------------------------------------------------------------
#define ALM_MOTOR_MOVE 55555
typedef bool (*PF_CHECK)(void);
//---------------------------------------------------------------------------
enum {  iCwLed          =0,
        iHomeLed        =1,
        iCcwLed         =2,
        iEmgLed         =3,
        iAlarmLed       =4,
        iSoftcwLed      =5,
        iSoftccwLed     =6,
        iServoalarmLed  =7,
        iInposLed       =8,
        iZPhaseLed      =9,
        iServoOn        =10,
        iMotLedTotalCnt
     };

enum eMotionCardType{eMC8040A       =0,
                     ePCI885X       =1,
                     eMC88x1        =2,
                     eSMC           =3,
                     eMN200         =4,
                     ePLCbase       =5,
                     eMotionCardUnknown,
                     eMotionCardTypeTotal
                    };

enum eMotorKind{eMotor          =0,
                eLinerMotor     =1,
                eCylinderMotor  =2,
                eVoiceCoilMotor =3,
                eStepServo      =4,
                eYASKAWA        =5,
                eMotorKindTotal
               };
//---------------------------------------------------------------------------
class HTMotor
{
private:
protected:
    unsigned int    iSpeed;
    unsigned int    InitSpeed;
    unsigned int    Rate;
    unsigned int    Range;
    int             EncoderPos;
    int             iHomeObjectTask;
    double          dAcc;
    double          dDec;
    eMotorKind      MotorKind;
    eMotionCardType MotionCardType;

public:
    HTMotor();
    virtual ~HTMotor();

    unsigned int    Address;
    unsigned int    iBoardID;
    unsigned int    iPortID;
    unsigned int    HomeHighSpeed;
    unsigned int    HomeLowSpeed;
    unsigned int    JogHighSpeed;
    unsigned int    JogLowSpeed;
    bool            Enable;
    bool            Direction;
    bool            HomeDirection;
    bool            MotorType;
    bool            bSensorType;
    bool            bLimitLogic;
    bool            bIn1Logic;
    double          GearRatio;
    int             SoftLimitP;
    int             SoftLimitN;
    int             LastHomePos;
    int             EncoderType;
    bool            ServoAlarmOn;
    int             iMotNo;
    int             iHomeType;
    int             iEncodeMultiple;   // MC88X1 A/B encoder input multiplier config (set before InitMotor): 3=x4 default, 1=x1 (M20)
    int             iHomeStep;
    int             iHomeStepRange;
    bool            bNeedHome;
    bool            bThreadHome;
    bool            bHomePhaseTimeout;   // set by HomeType90 phase-B on leave/re-approach timeout
    PF_CHECK        MotorIdleSafeDoorCheck;

    unsigned int    ReadSpeed();
    unsigned int    ReadInitSpeed();
    unsigned int    ReadRate();
    unsigned int    ReadRange();
    double          ReadAcc();
    double          ReadDec();
    bool            ReadServoAlarmOn();
    virtual int     ReadEncoderPos();
    void            SetHomeobjectTask(int Task);
    void            SetMotNo(int No) { iMotNo=No; }
    void            SetMotorKind(eMotorKind Kind) { MotorKind=Kind; }
    eMotorKind      GetMotorKind() { return MotorKind; }
    void            SetMotionCardType(eMotionCardType Type) { MotionCardType=Type; }
    eMotionCardType GetMotionCardType() { return MotionCardType; }

    virtual void    Stop() {}
    virtual bool    JogP() { return false; }
    virtual bool    JogN() { return false; }
    virtual int     ReadPos() { return EncoderPos; }
    virtual bool    MoveTo(int Tar) { EncoderPos=Tar; return true; }
    virtual bool    MoveToPosShortDistance(int Tar) { return MoveTo(Tar); }
    virtual bool    HomeObject() { EncoderPos=0; return true; }
    virtual void    HomeReset() { iHomeObjectTask=1; }
    virtual bool    HomeFlag(void) { return true; }
    virtual bool    GetAlarm(void) { return false; }
    virtual void    SetSpeed(unsigned int x) { iSpeed=x; }
    virtual void    SetInitSpeed(unsigned int x) { InitSpeed=x; }
    virtual void    SetServoAlarmOn(bool Value) { ServoAlarmOn=Value; }
    virtual int     InitMotor(int IoAddress) { Address=IoAddress; return 0; }
    virtual void    SetRange(unsigned int a) { Range=a; }
    virtual void    SetRate(unsigned int a) { Rate=a; }
    virtual void    SetAcc(double a) { dAcc=a; }
    virtual void    SetDec(double a) { dDec=a; }
    virtual int     ReadRealPos() { return EncoderPos; }
    virtual int     ReadEnCoderRealPos() { return EncoderPos; }
    virtual int     SetCommand(int p) { EncoderPos=p; return 0; }
    virtual int     SetPosition(int p) { EncoderPos=p; return 0; }
    virtual void    SetServoOn(bool IsOn) { (void)IsOn; }
    virtual bool    ResetPos(int Pulse=0) { EncoderPos=Pulse; return true; }
    virtual void    SoftLimitEnable(bool bFlag) { (void)bFlag; }
    virtual void    MotOutputOn(int iOutPort) { (void)iOutPort; }
    virtual void    MotOutputOff(int iOutPort) { (void)iOutPort; }
    virtual void    MotInputStatus(bool *bInputPort) { (void)bInputPort; }
    virtual void    EnableTrigger(int iFlag, int iMode, long lValue) { (void)iFlag; (void)iMode; (void)lValue; }
    virtual void    ManualTestTrigger(bool bOn) { (void)bOn; }
    virtual bool    LinearAxisMoveTo(int iPortID[8], long lPos[8], bool bFlag) { (void)iPortID; (void)lPos; (void)bFlag; return false; }
    virtual void    ScanMotorStatus(bool *Led) { (void)Led; }
    virtual void    DecStop(void) {}
    virtual void    SetSoftLimit(int iPLimit, int iNLimit) { SoftLimitP=iPLimit; SoftLimitN=iNLimit; }
    virtual bool    ReadStatus(DWORD offset, WORD *ReadData) { (void)offset; if(ReadData!=NULL) *ReadData=0; return true; }
    virtual bool    WriteStatus(DWORD offset, WORD WriteData) { (void)offset; (void)WriteData; return true; }
    virtual void    MotIpReset() {}
    // AI(general) 20260617 : MC88X1 speed/accel range diagnostics. GetLastParaError
    // returns the last MC88X1PMotAxisParaSet return code (0=ok, 0x1000+ = a SV/DV/MDV/AC
    // param out of the card's range). VerifyHomeParaRange dry-runs the HOME-seek profile
    // and returns that code, then restores the running params. Default no-op for non-MC88X1.
    virtual DWORD   GetLastParaError(void) { return 0; }
    virtual DWORD   VerifyHomeParaRange(void) { return 0; }
    virtual bool    IsMotorBusy() { return false; }
    virtual void    MotorReset() {}
    virtual bool    IsMotorAlarm() { return false; }
    virtual void    EnableSetInPosition(bool SetEnable) { (void)SetEnable; }
    virtual void    SetPos(int pos) { SetCommand(pos); }

    bool            CheckArmPosInRange(int iNowPos, int iMin, int iMax);
    bool            CheckArmPosArrival(int iNowPos, int iDestination, int iTolerance);
};
//---------------------------------------------------------------------------
#endif