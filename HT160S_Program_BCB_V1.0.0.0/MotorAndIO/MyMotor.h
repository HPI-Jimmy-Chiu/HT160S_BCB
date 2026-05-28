//---------------------------------------------------------------------------
#ifndef MyMotorH
#define MyMotorH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <ExtCtrls.hpp>
#include "HTMotor.h"
//---------------------------------------------------------------------------
enum eMotError{eMotPwrErr       =0,
               eMotTorqueErr    =1,
               eMotCWOnErr      =2,
               eMotCCWOnErr     =3,
               eMotSoftPErr     =4,
               eMotSoftNErr     =5,
               eMotPosErr       =6,
               eMotUnDefErr     =7,
               eMotOverLimitErr =8,
               eMotErrTotal
              };
//---------------------------------------------------------------------------
typedef struct MyMotorSimulateStruct
{
    TControl *PWinCtrl;
    int RefStart;
    int RefEnd;
    int FactStart;
    int FactEnd;
    double Scale;
    bool bUpDownMove;
}MyMotorSimulateList;

typedef struct MyLockStruct
{
    AnsiString MotorAlias;
    AnsiString LockFunc;
    int LockTask;
}MyLockList;
//---------------------------------------------------------------------------
class TTMyTray;

class TMyTray
{
public:
    int Data[20][50];
    AnsiString TrayID;

    TMyTray();
    void Clear();
    void SetAll(int data);
    bool HasIC();
    bool FullIC();
    bool HasThisIC(int data);
    bool FullThisIC(int data);
};
//---------------------------------------------------------------------------
class TMyMotor
{
private:
    int speed;
    HTMotor *Motor;
    bool MotorMoveSub(int p, bool bCheckLed);
    bool MotorMovePosition(int &Position, int iSpeed, int Target);
    bool SimulateMotorMovePosition(int &Position, int iSpeed, int Tar);

protected:
    TList *LockList;
    TList *SimuCtrlList;
    int iHomeTask;
    bool MoveTo(int Tar);
    bool MoveToPosShortDistance(int Tar);
    bool HomeObject(void);
    bool GetAlarm(void);

public:
    __fastcall TMyMotor();
    virtual ~TMyMotor();

    bool Led[iMotLedTotalCnt];
    bool bHomeFlag;
    bool bHomeFinish;
    int SimulateSpeed;
    int Position;
    int EncoderPosition;
    bool bErrorMove;
    int OriginRate;
    int OriginRange;
    bool bIsServoMotor;
    bool (*MoveCheckCallBack)();
    int Tag;
    int TargetPosition;
    AnsiString Alias;
    AnsiString Number;
    AnsiString NumberAlias;
    AnsiString AlarmName[eMotErrTotal];
    bool bQuickHome;
    AnsiString CardModel;
    int iPersentSpeed;
    TStringList *HomeOrder;
    AnsiString FlushPanelName;

    void InitMotor(int IoAddress);
    bool JogP();
    bool JogN();
    void Stop();
    bool MotorMove(int p);
    int MotorMove(int p, int PreDonePos, bool bJogP);
    bool MotorMoveSKLED(int p);
    bool Home(AnsiString &sErr);
    void InitHomeTask();
    int ReadPos();
    int ReadEncoderPos();
    void ScanMotorStatus();

    int GetInitSpeed();
    int GetPersentSpeed();
    int GetSpeed();
    int GetRange();
    int GetRate();
    double GetAcc();
    double GetDec();
    int GetHomeHighSpeed();
    int GetHomeLowSpeed();
    int GetJogHighSpeed();
    int GetJogLowSpeed();
    int GetAddress();
    int GetErrorIndex();
    int GetSoftLimitP();
    int GetSoftLimitN();
    int GetLastHomePos();
    double GetGearRatio();
    bool GetEnable();
    bool GetDirection();
    bool ReadServoAlarmOn();

    void SetPersentSpeed(int persent, bool bSave=true);
    void SetSpeed(unsigned int p);
    void SetInitSpeed(unsigned int x);
    void SetRange(unsigned int a);
    void SetRate(unsigned int a);
    void SetAcc(double a);
    void SetDec(double a);
    void SetHomeHighSpeed(unsigned int a);
    void SetHomeLowSpeed(unsigned int a);
    void SetJogHighSpeed(unsigned int a);
    void SetJogLowSpeed(unsigned int a);
    void SetDirection(bool Value);
    void SetHomeDirection(bool Value);
    void SetGearRatio(double a);
    void SetSoftLimitN(int a);
    void SetSoftLimitP(int a);
    void SetServoAlarmOn(bool Value);
    void SetMotorType(bool Value);
    void SetSensorType(bool Value);
    void SetEnable(bool Value);
    void SetLimitLogic(bool logic);
    void SetIn1Logic(bool logic);
    void SetMotorKind(eMotorKind Kind);
    eMotorKind GetMotorKind();
    void SetMotionCardType(eMotionCardType Type);
    eMotionCardType GetMotionCardType();
    void SetHomeOrder(AnsiString OrderString);
    void SetMotNo(int No);
    void InitialMotorObject(int addr);

    void UpdateSimulateCompomentPosition();
    void SetShowSimulateCompomentFlag(bool flag);
    void SetSimulateCompoment(TObject *PCtrl, TAnchorKind Alignment, int StartPos, int EndPos, int simuStartPos, int simuEndPos);
    void Lock(AnsiString MotorAlias, AnsiString FunctionName, int Task);
    void UnLock(AnsiString MotorAlias, AnsiString FunctionName);
    void ClearLock();
    int GetLockCount();
    AnsiString GetLockString(int Index);
    bool ResetPos(int p);
    void SetEncoderToCommand();
    void SoftLimitEnable(bool bFlag);
    void MotOutputOn(int iOutPort);
    void MotOutputOff(int iOutPort);
    void MotInputStatus(bool *bInputPort);
    bool LinearAxisMoveTo(TMyMotor* LineMotPtr[8], long lPos[8]);
    void SetEncoderType(int a);
    void InitHomeTask_forSingleAxis();
    void DecStop();
    void ServoOnOff(bool IsOn);
    void ServoOnResetPos();
    void ClearPosition(int cmd);
    void EnableTrigger(int iFlag, int iMode, long lValue);
    void ManualTestTrigger(bool bOn);
    bool ReadStatus(DWORD offset, WORD *ReadData);
    int CompareCommandPos(int iPos, int iGap);
    bool CheckArmPosInRange(int iNowPos, int iMin, int iMax);
    bool CheckArmPosArrival(int iNowPos, int iDestination, int iTolerance);
    bool CheckSoftLimit(int p, bool bAlarm=true);
};
//---------------------------------------------------------------------------
class TTrayMotor: public TMyMotor
{
private:
protected:
    bool fHTary;
    bool fPanel;
    bool fSubHTary;
    TTMyTray *pHTray;
    TTMyTray *pSubHTray;
    TPanel *pPanel;
    bool fPanelID;
    TPanel *pPalTrayID;

public:
    __fastcall TTrayMotor();
    virtual ~TTrayMotor();
    bool fHasTray;
    bool bHasCover;
    TMyTray Tray;

    bool HasIC();
    bool HasRing();
    bool HasRealIC();
    bool RowYFullIC(int Y);
    bool FullIC();
    bool FullThisIC(int data);
    bool HasThisIC(int data);
    void SetTrayInfo(int iRow, int iCol);
    void SetPTrayData(int x, int y, int iBin);
    void SetTraySingleData(int x, int y, int data);
    void Refresh();
    void InitNewTray(int data);
    void InitEmptyTray();
    void SetIDPanel(TPanel *ptr);
    void SetTrayPanel(TPanel *ptr);
    void SetHTrayPanel(TTMyTray *ptr);
    void SetSubHTrayPanel(TTMyTray *ptr);
    void CopyTrayFrom(int Index);
    void MoveTrayFrom(int Index);
    void SetTray(int data, bool bWithCover=false);
    void ClearTray();
    void SetTrayID(AnsiString ID);
    void SetTrayVisible(bool bVisible);
};
//---------------------------------------------------------------------------
#endif