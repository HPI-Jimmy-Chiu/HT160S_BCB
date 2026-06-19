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

//---------------------------------------------------------------------------
// Tray data grid dimensions (single source of truth for TMyTray::Data).
#define MAX_TRAY_Y 50
#define MAX_TRAY_X 20
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260604 : Tray role within a stacking car (TMyCar).
//   eTrayKindNormal   : normal work tray, may pick/place IC (default).
//   eTrayKindIdentity : identity tray, carries the stack's 2D ID (TrayID), no IC.
//   eTrayKindCover    : top cover tray, an empty tray that must NOT hold IC.
enum eTrayKind{eTrayKindNormal   =0,
               eTrayKindIdentity =1,
               eTrayKindCover    =2
              };
//---------------------------------------------------------------------------
class TMyTray
{
public:
    int Data[MAX_TRAY_X][MAX_TRAY_Y];   //AI(general) 20260601 : X-major Data[x][y], aligned to HT172
    int iBin[MAX_TRAY_X][MAX_TRAY_Y];
    //AI(ht160s-lotbin) 20260615 : By Lot+Bin mode carries the owning LotIndex and
    //the IC 2D code per cell (mirror iBin), so the (Lot,Bin) routing key and the
    //Production_Log Lot/2D columns survive the CCD->pick->place hand-off.
    int iLot[MAX_TRAY_X][MAX_TRAY_Y];
    AnsiString sCode2D[MAX_TRAY_X][MAX_TRAY_Y];
    AnsiString TrayID;
    eTrayKind Kind;   //AI(HT160S-Maintainer) 20260604 : tray role in stacking car

    TMyTray();
    void Clear();
    void SetAll(int data);
    bool HasIC();
    bool FullIC();
    bool HasThisIC(int data);
    bool FullThisIC(int data);
    //AI(HT160S-Maintainer) 20260601 : iBin sorting-bin grid helpers (mirror Data helpers)
    void ClearBin();
    void SetAllBin(int bin);
    void SetBin(int x, int y, int bin);
    int GetBin(int x, int y);
    //AI(ht160s-lotbin) 20260615 : LotIndex + 2D-code grid helpers (mirror iBin helpers)
    void ClearLotCode();
    void SetLot(int x, int y, int lot);
    int  GetLot(int x, int y);
    void SetCode2D(int x, int y, AnsiString code);
    AnsiString GetCode2D(int x, int y);
    //AI(HT160S-Maintainer) 20260604 : tray-kind helpers
    void SetKind(eTrayKind kind);
    eTrayKind GetKind();
    bool CanHoldIC();   // true only for eTrayKindNormal
};
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260604 : Stacking-car container (one level above TMyTray).
//   A car holds an ordered stack of trays:
//     Tray[0]   = identity tray (eTrayKindIdentity, carries CarID/2D),
//     Tray[1]   = top cover     (eTrayKindCover, no IC),
//     Tray[2..] = normal work trays (eTrayKindNormal).
//   Mainly used by Auto1~6; packed for AMR upload when AMR retrieves the car
//   (upload payload not designed yet).
#define MAX_TRAY_PER_CAR 100
//---------------------------------------------------------------------------
class TMyCar
{
public:
    AnsiString CarID;                  // stack identity (mirrors identity tray's TrayID)
    int iTrayCount;                    // number of valid trays currently in the car
    TMyTray Tray[MAX_TRAY_PER_CAR];

    TMyCar();
    void Clear();
    int GetTrayCount();
    TMyTray *GetTray(int index);       // NULL if out of range
    TMyTray *GetIdentityTray();        // first eTrayKindIdentity tray, else NULL
    bool IsFull();
    void PackForAmrUpload();           //AI(HT160S-Maintainer) 20260604 : payload TBD, stub for now
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
    DWORD dwHomeSensorWaitStart;   // GetTickCount stamp for the home-sensor confirm wait
    int SimulateSpeed;
    bool bShowSimulateCompoment;
    int Position;
    int EncoderPosition;
    bool bErrorMove;
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
    DWORD GetLastParaError(void);
    DWORD VerifyHomeParaRange(void);
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
    void SetHomeType(int Type);
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
    void SetTrayBin(int x, int y, int bin);   //AI(HT160S-Maintainer) 20260601 : write sorting bin for a cell
    int GetTrayBin(int x, int y);             //AI(HT160S-Maintainer) 20260601 : read sorting bin for a cell
    void SetTrayLot(int x, int y, int lot);   //AI(ht160s-lotbin) 20260615 : write owning LotIndex for a cell
    int  GetTrayLot(int x, int y);            //AI(ht160s-lotbin) 20260615 : read owning LotIndex for a cell
    void SetTrayCode2D(int x, int y, AnsiString code);  //AI(ht160s-lotbin) 20260615 : write IC 2D code for a cell
    AnsiString GetTrayCode2D(int x, int y);             //AI(ht160s-lotbin) 20260615 : read IC 2D code for a cell
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
    void UpdateTrayVisibleByHasTray();
};
//---------------------------------------------------------------------------
#endif