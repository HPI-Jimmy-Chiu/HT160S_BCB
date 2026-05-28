//---------------------------------------------------------------------------
#include <vcl.h>
#include <string.h>
#include <stdlib.h>
#pragma hdrstop

#include "MyMotor.h"
#include "mySMCmotor.h"
#include "myMN200motor.h"
#include "myMC88X1motor.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#define DEFAULT_SIMULATE_SPEED 1000
//---------------------------------------------------------------------------
TMyTray::TMyTray()
{
    Clear();
}
//---------------------------------------------------------------------------
void TMyTray::Clear()
{
    for(int y=0; y<20; y++)
        for(int x=0; x<50; x++)
            Data[y][x]=0;
    TrayID="";
}
//---------------------------------------------------------------------------
void TMyTray::SetAll(int data)
{
    for(int y=0; y<20; y++)
        for(int x=0; x<50; x++)
            Data[y][x]=data;
}
//---------------------------------------------------------------------------
bool TMyTray::HasIC()
{
    for(int y=0; y<20; y++)
        for(int x=0; x<50; x++)
            if(Data[y][x]!=0)
                return true;
    return false;
}
//---------------------------------------------------------------------------
bool TMyTray::FullIC()
{
    for(int y=0; y<20; y++)
        for(int x=0; x<50; x++)
            if(Data[y][x]==0)
                return false;
    return true;
}
//---------------------------------------------------------------------------
bool TMyTray::HasThisIC(int data)
{
    for(int y=0; y<20; y++)
        for(int x=0; x<50; x++)
            if(Data[y][x]==data)
                return true;
    return false;
}
//---------------------------------------------------------------------------
bool TMyTray::FullThisIC(int data)
{
    for(int y=0; y<20; y++)
        for(int x=0; x<50; x++)
            if(Data[y][x]!=data)
                return false;
    return true;
}
//---------------------------------------------------------------------------
__fastcall TMyMotor::TMyMotor()
{
    speed=1000;
    Motor=new HTMotor;
    LockList=new TList;
    SimuCtrlList=new TList;
    HomeOrder=new TStringList;
    iHomeTask=1;
    bHomeFlag=false;
    bHomeFinish=false;
    SimulateSpeed=DEFAULT_SIMULATE_SPEED;
    Position=0;
    EncoderPosition=0;
    bErrorMove=false;
    OriginRate=0;
    OriginRange=0;
    bIsServoMotor=false;
    MoveCheckCallBack=NULL;
    Tag=-1;
    TargetPosition=0;
    iPersentSpeed=1;
    bQuickHome=false;
    memset(Led, 0, sizeof(Led));
}
//---------------------------------------------------------------------------
TMyMotor::~TMyMotor()
{
    ClearLock();
    delete LockList;
    delete SimuCtrlList;
    delete HomeOrder;
    delete Motor;
}
//---------------------------------------------------------------------------
void TMyMotor::SetHomeOrder(AnsiString OrderString)
{
    int iFlagPos;
    HomeOrder->Clear();
    while(OrderString.AnsiPos("|")!=0)
    {
        iFlagPos=OrderString.AnsiPos("|");
        OrderString.Delete(iFlagPos, 1);
        OrderString.Insert(",", iFlagPos);
    }
    HomeOrder->CommaText=OrderString;
}
//---------------------------------------------------------------------------
int TMyMotor::GetSpeed()
{
    if(Motor->Enable)
        return Motor->ReadSpeed();
    return speed;
}
//---------------------------------------------------------------------------
void TMyMotor::SetSpeed(unsigned int p)
{
    if(Motor->Enable)
        Motor->SetSpeed(p);
    else
        speed=p;
}
//---------------------------------------------------------------------------
int TMyMotor::GetPersentSpeed() { return iPersentSpeed; }
int TMyMotor::GetInitSpeed() { return Motor->ReadInitSpeed(); }
int TMyMotor::GetHomeHighSpeed() { return Motor->HomeHighSpeed; }
int TMyMotor::GetHomeLowSpeed() { return Motor->HomeLowSpeed; }
int TMyMotor::GetJogHighSpeed() { return Motor->JogHighSpeed; }
int TMyMotor::GetJogLowSpeed() { return Motor->JogLowSpeed; }
double TMyMotor::GetAcc() { return Motor->ReadAcc(); }
double TMyMotor::GetDec() { return Motor->ReadDec(); }
int TMyMotor::GetRange() { return Motor->ReadRange(); }
int TMyMotor::GetRate() { return Motor->ReadRate(); }
int TMyMotor::GetAddress() { return Motor->Address; }
int TMyMotor::GetSoftLimitP() { return Motor->SoftLimitP; }
int TMyMotor::GetSoftLimitN() { return Motor->SoftLimitN; }
int TMyMotor::GetLastHomePos() { return Motor->LastHomePos; }
double TMyMotor::GetGearRatio() { return Motor->GearRatio; }
bool TMyMotor::GetEnable() { return Motor->Enable; }
bool TMyMotor::GetDirection() { return Motor->Direction; }
bool TMyMotor::ReadServoAlarmOn() { return Motor->ReadServoAlarmOn(); }
//---------------------------------------------------------------------------
void TMyMotor::SetPersentSpeed(int persent, bool bSave)
{
    (void)bSave;
    if(persent<1)
        persent=1;
    if(persent>100)
        persent=100;
    iPersentSpeed=persent;
}
//---------------------------------------------------------------------------
void TMyMotor::SetInitSpeed(unsigned int x) { Motor->SetInitSpeed(x); }
void TMyMotor::SetRange(unsigned int a) { Motor->SetRange(a); }
void TMyMotor::SetRate(unsigned int a) { Motor->SetRate(a); }
void TMyMotor::SetAcc(double a) { Motor->SetAcc(a); }
void TMyMotor::SetDec(double a) { Motor->SetDec(a); }
void TMyMotor::SetHomeHighSpeed(unsigned int a) { Motor->HomeHighSpeed=a; }
void TMyMotor::SetHomeLowSpeed(unsigned int a) { Motor->HomeLowSpeed=a; }
void TMyMotor::SetJogHighSpeed(unsigned int a) { Motor->JogHighSpeed=a; }
void TMyMotor::SetJogLowSpeed(unsigned int a) { Motor->JogLowSpeed=a; }
void TMyMotor::SetDirection(bool Value) { Motor->Direction=Value; }
void TMyMotor::SetHomeDirection(bool Value) { Motor->HomeDirection=Value; }
void TMyMotor::SetGearRatio(double a) { Motor->GearRatio=a; }
void TMyMotor::SetSoftLimitN(int a) { Motor->SoftLimitN=a; }
void TMyMotor::SetSoftLimitP(int a) { Motor->SoftLimitP=a; }
void TMyMotor::SetServoAlarmOn(bool Value) { Motor->SetServoAlarmOn(Value); }
void TMyMotor::SetMotorType(bool Value) { Motor->MotorType=Value; }
void TMyMotor::SetSensorType(bool Value) { Motor->bSensorType=Value; }
void TMyMotor::SetEnable(bool Value) { Motor->Enable=Value; }
void TMyMotor::SetLimitLogic(bool logic) { Motor->bLimitLogic=logic; }
void TMyMotor::SetIn1Logic(bool logic) { Motor->bIn1Logic=logic; }
void TMyMotor::SetMotorKind(eMotorKind Kind) { Motor->SetMotorKind(Kind); }
eMotorKind TMyMotor::GetMotorKind() { return Motor->GetMotorKind(); }
void TMyMotor::SetMotionCardType(eMotionCardType Type) { Motor->SetMotionCardType(Type); }
eMotionCardType TMyMotor::GetMotionCardType() { return Motor->GetMotionCardType(); }
void TMyMotor::SetMotNo(int No) { Motor->SetMotNo(No); Tag=No; }
//---------------------------------------------------------------------------
void TMyMotor::InitialMotorObject(int addr)
{
    AnsiString Model=CardModel.UpperCase();
    bool bAddressAssigned=false;
    delete Motor;
    if(Model=="MC88X1" || Model=="MC88X1P")
    {
        Motor=new TMyMC88X1Motor(addr);
        bAddressAssigned=true;
    }
    else if(Model=="SMC")
        Motor=new TMySMCMotor(addr);
    else if(Model=="MN200")
        Motor=new TMyMN200Motor(addr);
    else
        Motor=new HTMotor;
    if(!bAddressAssigned)
        Motor->Address=addr;
}
//---------------------------------------------------------------------------
void TMyMotor::InitMotor(int IoAddress)
{
    Motor->InitMotor(IoAddress);
}
//---------------------------------------------------------------------------
bool TMyMotor::MotorMovePosition(int &Position, int iSpeed, int Target)
{
    (void)iSpeed;
    if(Motor->MoveTo(Target))
    {
        Position=Target;
        EncoderPosition=Target;
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TMyMotor::SimulateMotorMovePosition(int &Position, int iSpeed, int Tar)
{
    (void)iSpeed;
    Position=Tar;
    EncoderPosition=Tar;
    return true;
}
//---------------------------------------------------------------------------
bool TMyMotor::CheckSoftLimit(int p, bool bAlarm)
{
    (void)bAlarm;
    return (p<=Motor->SoftLimitP && p>=Motor->SoftLimitN);
}
//---------------------------------------------------------------------------
bool TMyMotor::MotorMoveSub(int p, bool bCheckLed)
{
    (void)bCheckLed;
    TargetPosition=p;
    bErrorMove=false;
    if(!CheckSoftLimit(p))
    {
        bErrorMove=true;
        return false;
    }
    if(MoveCheckCallBack!=NULL && MoveCheckCallBack()==false)
        return false;
    if(Motor->Enable)
        return MotorMovePosition(Position, GetSpeed(), p);
    return SimulateMotorMovePosition(Position, SimulateSpeed, p);
}
//---------------------------------------------------------------------------
bool TMyMotor::MotorMove(int p)
{
    return MotorMoveSub(p, true);
}
//---------------------------------------------------------------------------
int TMyMotor::MotorMove(int p, int PreDonePos, bool bJogP)
{
    (void)PreDonePos;
    (void)bJogP;
    return MotorMove(p)?1:0;
}
//---------------------------------------------------------------------------
bool TMyMotor::MotorMoveSKLED(int p)
{
    return MotorMoveSub(p, false);
}
//---------------------------------------------------------------------------
bool TMyMotor::MoveTo(int Tar) { return Motor->MoveTo(Tar); }
bool TMyMotor::MoveToPosShortDistance(int Tar) { return Motor->MoveToPosShortDistance(Tar); }
bool TMyMotor::HomeObject(void) { return Motor->HomeObject(); }
bool TMyMotor::GetAlarm(void) { return Motor->GetAlarm(); }
bool TMyMotor::JogP() { return Motor->JogP(); }
bool TMyMotor::JogN() { return Motor->JogN(); }
void TMyMotor::Stop() { Motor->Stop(); }
void TMyMotor::DecStop() { Motor->DecStop(); }
//---------------------------------------------------------------------------
void TMyMotor::InitHomeTask()
{
    iHomeTask=1;
    bHomeFlag=false;
    bHomeFinish=false;
}
//---------------------------------------------------------------------------
bool TMyMotor::Home(AnsiString &sErr)
{
    sErr="";
    if(Motor->Enable==false)
    {
        ResetPos(0);
        bHomeFlag=true;
        bHomeFinish=true;
        return true;
    }
    if(Motor->HomeObject() && Motor->HomeFlag())
    {
        bHomeFlag=true;
        bHomeFinish=true;
        Position=Motor->ReadPos();
        EncoderPosition=Motor->ReadEncoderPos();
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
int TMyMotor::ReadPos()
{
    if(Motor->Enable)
        Position=Motor->ReadPos();
    return Position;
}
//---------------------------------------------------------------------------
int TMyMotor::ReadEncoderPos()
{
    EncoderPosition=Motor->ReadEncoderPos();
    return EncoderPosition;
}
//---------------------------------------------------------------------------
void TMyMotor::ScanMotorStatus()
{
    Motor->ScanMotorStatus(Led);
}
//---------------------------------------------------------------------------
int TMyMotor::GetErrorIndex()
{
    if(Led[iAlarmLed] && Led[iServoalarmLed] && Led[iInposLed])
        return 0;
    else if(Led[iAlarmLed] && Led[iServoalarmLed])
        return 1;
    else if(Led[iCwLed] && Led[iAlarmLed])
        return 2;
    else if(Led[iCcwLed] && Led[iAlarmLed])
        return 3;
    else if(Led[iAlarmLed] && Led[iSoftcwLed])
        return 4;
    else if(Led[iAlarmLed] && Led[iSoftccwLed])
        return 5;
    else if(Led[iAlarmLed])
        return 6;
    else if(Led[iServoalarmLed])
        return 8;
    return 7;
}
//---------------------------------------------------------------------------
void TMyMotor::Lock(AnsiString MotorAlias, AnsiString FunctionName, int Task)
{
    MyLockList *p;
    for(int i=0; i<LockList->Count; i++)
    {
        p=(MyLockList *)LockList->Items[i];
        if(p->MotorAlias==MotorAlias && p->LockFunc==FunctionName)
            return;
    }
    p=new MyLockList;
    p->MotorAlias=MotorAlias;
    p->LockFunc=FunctionName;
    p->LockTask=Task;
    LockList->Add(p);
}
//---------------------------------------------------------------------------
void TMyMotor::UnLock(AnsiString MotorAlias, AnsiString FunctionName)
{
    MyLockList *p;
    for(int i=0; i<LockList->Count; i++)
    {
        p=(MyLockList *)LockList->Items[i];
        if(p->MotorAlias==MotorAlias && p->LockFunc==FunctionName)
        {
            delete p;
            LockList->Delete(i);
            return;
        }
    }
}
//---------------------------------------------------------------------------
void TMyMotor::ClearLock()
{
    MyLockList *p;
    for(int i=0; i<LockList->Count; i++)
    {
        p=(MyLockList *)LockList->Items[i];
        delete p;
    }
    LockList->Clear();
}
//---------------------------------------------------------------------------
int TMyMotor::GetLockCount()
{
    return LockList->Count;
}
//---------------------------------------------------------------------------
AnsiString TMyMotor::GetLockString(int Index)
{
    MyLockList *p;
    AnsiString S="";
    if(Index<LockList->Count && Index>=0)
    {
        p=(MyLockList *)LockList->Items[Index];
        S=p->MotorAlias+AnsiString("  ")+p->LockFunc+AnsiString("  ")+IntToStr(p->LockTask);
    }
    return S;
}
//---------------------------------------------------------------------------
bool TMyMotor::ResetPos(int p) { Position=p; EncoderPosition=p; return Motor->ResetPos(p); }
void TMyMotor::SetEncoderToCommand() { EncoderPosition=Position; }
void TMyMotor::SoftLimitEnable(bool bFlag) { Motor->SoftLimitEnable(bFlag); }
void TMyMotor::MotOutputOn(int iOutPort) { Motor->MotOutputOn(iOutPort); }
void TMyMotor::MotOutputOff(int iOutPort) { Motor->MotOutputOff(iOutPort); }
void TMyMotor::MotInputStatus(bool *bInputPort) { Motor->MotInputStatus(bInputPort); }
void TMyMotor::SetEncoderType(int a) { Motor->EncoderType=a; }
void TMyMotor::InitHomeTask_forSingleAxis() { InitHomeTask(); }
void TMyMotor::ServoOnOff(bool IsOn) { Motor->SetServoOn(IsOn); }
void TMyMotor::ServoOnResetPos() { ResetPos(Position); }
void TMyMotor::ClearPosition(int cmd) { ResetPos(cmd); }
void TMyMotor::EnableTrigger(int iFlag, int iMode, long lValue) { Motor->EnableTrigger(iFlag, iMode, lValue); }
void TMyMotor::ManualTestTrigger(bool bOn) { Motor->ManualTestTrigger(bOn); }
bool TMyMotor::ReadStatus(DWORD offset, WORD *ReadData) { return Motor->ReadStatus(offset, ReadData); }
void TMyMotor::UpdateSimulateCompomentPosition() {}
void TMyMotor::SetShowSimulateCompomentFlag(bool flag) { (void)flag; }
void TMyMotor::SetSimulateCompoment(TObject *PCtrl, TAnchorKind Alignment, int StartPos, int EndPos, int simuStartPos, int simuEndPos)
{
    (void)PCtrl;
    (void)Alignment;
    (void)StartPos;
    (void)EndPos;
    (void)simuStartPos;
    (void)simuEndPos;
}
//---------------------------------------------------------------------------
bool TMyMotor::LinearAxisMoveTo(TMyMotor* LineMotPtr[8], long lPos[8])
{
    int iPortID[8];
    for(int i=0; i<8; i++)
        iPortID[i]=(LineMotPtr[i]!=NULL)?LineMotPtr[i]->GetAddress():-1;
    return Motor->LinearAxisMoveTo(iPortID, lPos, true);
}
//---------------------------------------------------------------------------
int TMyMotor::CompareCommandPos(int iPos, int iGap)
{
    int iDiff=Position-iPos;
    if(iDiff<0)
        iDiff=-iDiff;
    return (iDiff<=iGap)?1:0;
}
//---------------------------------------------------------------------------
bool TMyMotor::CheckArmPosInRange(int iNowPos, int iMin, int iMax)
{
    return Motor->CheckArmPosInRange(iNowPos, iMin, iMax);
}
//---------------------------------------------------------------------------
bool TMyMotor::CheckArmPosArrival(int iNowPos, int iDestination, int iTolerance)
{
    return Motor->CheckArmPosArrival(iNowPos, iDestination, iTolerance);
}
//---------------------------------------------------------------------------
__fastcall TTrayMotor::TTrayMotor()
{
    fHTary=false;
    fPanel=false;
    fSubHTary=false;
    pHTray=NULL;
    pSubHTray=NULL;
    pPanel=NULL;
    fPanelID=false;
    pPalTrayID=NULL;
    fHasTray=false;
    bHasCover=false;
}
//---------------------------------------------------------------------------
TTrayMotor::~TTrayMotor()
{
}
//---------------------------------------------------------------------------
bool TTrayMotor::HasIC() { return Tray.HasIC(); }
bool TTrayMotor::HasRing() { return Tray.HasIC(); }
bool TTrayMotor::HasRealIC() { return Tray.HasIC(); }
bool TTrayMotor::RowYFullIC(int Y) { (void)Y; return Tray.FullIC(); }
bool TTrayMotor::FullIC() { return Tray.FullIC(); }
bool TTrayMotor::FullThisIC(int data) { return Tray.FullThisIC(data); }
bool TTrayMotor::HasThisIC(int data) { return Tray.HasThisIC(data); }
void TTrayMotor::SetTrayInfo(int iRow, int iCol) { (void)iRow; (void)iCol; }
void TTrayMotor::SetPTrayData(int x, int y, int iBin) { SetTraySingleData(x, y, iBin); }
void TTrayMotor::Refresh() {}
void TTrayMotor::SetIDPanel(TPanel *ptr) { pPalTrayID=ptr; }
void TTrayMotor::SetTrayPanel(TPanel *ptr) { pPanel=ptr; }
void TTrayMotor::SetHTrayPanel(TTMyTray *ptr) { pHTray=ptr; }
void TTrayMotor::SetSubHTrayPanel(TTMyTray *ptr) { pSubHTray=ptr; }
void TTrayMotor::CopyTrayFrom(int Index) { (void)Index; }
void TTrayMotor::MoveTrayFrom(int Index) { (void)Index; }
void TTrayMotor::SetTrayVisible(bool bVisible) { (void)bVisible; }
//---------------------------------------------------------------------------
void TTrayMotor::SetTraySingleData(int x, int y, int data)
{
    if(y>=0 && y<20 && x>=0 && x<50)
        Tray.Data[y][x]=data;
}
//---------------------------------------------------------------------------
void TTrayMotor::InitNewTray(int data)
{
    Tray.SetAll(data);
    fHasTray=true;
}
//---------------------------------------------------------------------------
void TTrayMotor::InitEmptyTray()
{
    Tray.Clear();
    fHasTray=true;
}
//---------------------------------------------------------------------------
void TTrayMotor::SetTray(int data, bool bWithCover)
{
    Tray.SetAll(data);
    fHasTray=true;
    bHasCover=bWithCover;
}
//---------------------------------------------------------------------------
void TTrayMotor::ClearTray()
{
    Tray.Clear();
    fHasTray=false;
    bHasCover=false;
}
//---------------------------------------------------------------------------
void TTrayMotor::SetTrayID(AnsiString ID)
{
    Tray.TrayID=ID;
}
//---------------------------------------------------------------------------