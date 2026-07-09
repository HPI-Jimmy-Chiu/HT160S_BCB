//---------------------------------------------------------------------------
#include <vcl.h>
#include <string.h>
#include <stdlib.h>
#pragma hdrstop

#include "MyMotor.h"
#include "mySMCmotor.h"
#include "myMN200motor.h"
#include "myMC88X1motor.h"
#include "HTray.h"
#include "CosFunction.h"   //AI(general) 20260609 : recipe TrayForm geometry (real tray Col/Row)
#include "cStepTrace.h"    //AI(general) 20260617 : MotorTaskLog home/limit diagnosis trace
//---------------------------------------------------------------------------
#pragma package(smart_init)
#define DEFAULT_SIMULATE_SPEED 1000
// Bounded wait (ms) for the home sensor (HomeFlag) to confirm after the card
// reports home complete; on timeout HOME commits best-effort so it cannot stick.
#define HOME_SENSOR_CONFIRM_MS 5000
//---------------------------------------------------------------------------
//AI(general) 20260609 : The real tray region comes from the active recipe
//geometry (TrayForm.XDivision/YDivision), NOT the MAX_TRAY_* array bounds.
//A 3x3 tray only fills Data[0..2][0..2]; scanning the full 20x50 array made
//FullThisIC()/FullIC() never report true, so Auto full-tray discharge never
//triggered. Clamp to the array bounds so an out-of-range recipe can't overflow.
static int GetTrayRealXCount()
{
    int x=TrayForm.XDivision;
    if(x<1) x=1;
    if(x>MAX_TRAY_X) x=MAX_TRAY_X;
    return x;
}
//---------------------------------------------------------------------------
static int GetTrayRealYCount()
{
    int y=TrayForm.YDivision;
    if(y<1) y=1;
    if(y>MAX_TRAY_Y) y=MAX_TRAY_Y;
    return y;
}
//---------------------------------------------------------------------------
TMyTray::TMyTray()
{
    Clear();
}
//---------------------------------------------------------------------------
void TMyTray::Clear()
{
    for(int y=0; y<MAX_TRAY_Y; y++)
        for(int x=0; x<MAX_TRAY_X; x++)
        {
            Data[x][y]=0;
            iBin[x][y]=0;   //AI(HT160S-Maintainer) 20260601 : 0 = bin not assigned yet
            iLot[x][y]=-1;  //AI(ht160s-lotbin) 20260615 : -1 = no owning lot yet
            sCode2D[x][y]="";
            bManual2D[x][y]=false;
            iPassClass[x][y]=0;  //AI(ht160s-lotpassfail) 20260709 : 0 = no PASS/FAIL class yet
        }
    TrayID="";
    Kind=eTrayKindNormal;   //AI(HT160S-Maintainer) 20260604 : default role = normal work tray
}
//---------------------------------------------------------------------------
void TMyTray::SetAll(int data)
{
    for(int y=0; y<MAX_TRAY_Y; y++)
        for(int x=0; x<MAX_TRAY_X; x++)
            Data[x][y]=data;
}
//---------------------------------------------------------------------------
//AI(ht160s-tray-source) : unified grid birth. Clear() resets all grids + TrayID + Kind=Normal;
//then stamp fill/kind/id. Birth(EMPTY_IC, Normal, empty) equals Clear(). Used by Empty/Color births.
void TMyTray::Birth(int data, eTrayKind kind, AnsiString id)
{
    Clear();
    SetAll(data);
    Kind=kind;
    TrayID=id;
}
//---------------------------------------------------------------------------
//AI(ht160s-tray-source) : deep copy (default member-wise; arrays element-wise, AnsiString refcount).
void TMyTray::CopyFrom(const TMyTray &src)
{
    *this = src;
}
//---------------------------------------------------------------------------
//AI(ht160s-tray-source) : move = copy then clear the source (mirrors HT172 MoveTrayFrom).
void TMyTray::MoveFrom(TMyTray &src)
{
    *this = src;
    src.Clear();
}
//---------------------------------------------------------------------------
bool TMyTray::HasIC()
{
    int xEnd=GetTrayRealXCount();
    int yEnd=GetTrayRealYCount();
    for(int y=0; y<yEnd; y++)
        for(int x=0; x<xEnd; x++)
            if(Data[x][y]!=0)
                return true;
    return false;
}
//---------------------------------------------------------------------------
bool TMyTray::FullIC()
{
    int xEnd=GetTrayRealXCount();
    int yEnd=GetTrayRealYCount();
    for(int y=0; y<yEnd; y++)
        for(int x=0; x<xEnd; x++)
            if(Data[x][y]==0)
                return false;
    return true;
}
//---------------------------------------------------------------------------
bool TMyTray::HasThisIC(int data)
{
    int xEnd=GetTrayRealXCount();
    int yEnd=GetTrayRealYCount();
    for(int y=0; y<yEnd; y++)
        for(int x=0; x<xEnd; x++)
            if(Data[x][y]==data)
                return true;
    return false;
}
//---------------------------------------------------------------------------
bool TMyTray::FullThisIC(int data)
{
    int xEnd=GetTrayRealXCount();
    int yEnd=GetTrayRealYCount();
    for(int y=0; y<yEnd; y++)
        for(int x=0; x<xEnd; x++)
            if(Data[x][y]!=data)
                return false;
    return true;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260601 : iBin sorting-bin grid helpers (mirror Data helpers)
void TMyTray::ClearBin()
{
    for(int y=0; y<MAX_TRAY_Y; y++)
        for(int x=0; x<MAX_TRAY_X; x++)
            iBin[x][y]=0;
}
//---------------------------------------------------------------------------
void TMyTray::SetAllBin(int bin)
{
    for(int y=0; y<MAX_TRAY_Y; y++)
        for(int x=0; x<MAX_TRAY_X; x++)
            iBin[x][y]=bin;
}
//---------------------------------------------------------------------------
void TMyTray::SetBin(int x, int y, int bin)
{
    if(x<0 || x>=MAX_TRAY_X || y<0 || y>=MAX_TRAY_Y)
        return;
    iBin[x][y]=bin;
}
//---------------------------------------------------------------------------
int TMyTray::GetBin(int x, int y)
{
    if(x<0 || x>=MAX_TRAY_X || y<0 || y>=MAX_TRAY_Y)
        return 0;
    return iBin[x][y];
}
//---------------------------------------------------------------------------
//AI(ht160s-lotbin) 20260615 : LotIndex + 2D-code grid helpers (mirror iBin helpers)
void TMyTray::ClearLotCode()
{
    for(int y=0; y<MAX_TRAY_Y; y++)
        for(int x=0; x<MAX_TRAY_X; x++)
        {
            iLot[x][y]=-1;
            sCode2D[x][y]="";
            bManual2D[x][y]=false;
            iPassClass[x][y]=0;  //AI(ht160s-lotpassfail) 20260709 : clear frozen PASS/FAIL class
        }
}
//---------------------------------------------------------------------------
void TMyTray::SetLot(int x, int y, int lot)
{
    if(x<0 || x>=MAX_TRAY_X || y<0 || y>=MAX_TRAY_Y)
        return;
    iLot[x][y]=lot;
}
//---------------------------------------------------------------------------
int TMyTray::GetLot(int x, int y)
{
    if(x<0 || x>=MAX_TRAY_X || y<0 || y>=MAX_TRAY_Y)
        return -1;
    return iLot[x][y];
}
//---------------------------------------------------------------------------
void TMyTray::SetCode2D(int x, int y, AnsiString code)
{
    if(x<0 || x>=MAX_TRAY_X || y<0 || y>=MAX_TRAY_Y)
        return;
    sCode2D[x][y]=code;
}
//---------------------------------------------------------------------------
AnsiString TMyTray::GetCode2D(int x, int y)
{
    if(x<0 || x>=MAX_TRAY_X || y<0 || y>=MAX_TRAY_Y)
        return "";
    return sCode2D[x][y];
}
//---------------------------------------------------------------------------
//AI(ht160s-lotpassfail) 20260709 : per-cell PASS/FAIL class accessors (mirror SetBin/GetBin)
void TMyTray::SetPassClass(int x, int y, int c)
{
    if(x<0 || x>=MAX_TRAY_X || y<0 || y>=MAX_TRAY_Y)
        return;
    iPassClass[x][y]=c;
}
//---------------------------------------------------------------------------
int TMyTray::GetPassClass(int x, int y)
{
    if(x<0 || x>=MAX_TRAY_X || y<0 || y>=MAX_TRAY_Y)
        return 0;
    return iPassClass[x][y];
}
//---------------------------------------------------------------------------
void TMyTray::SetManual2D(int x, int y, bool b)
{
    if(x<0 || x>=MAX_TRAY_X || y<0 || y>=MAX_TRAY_Y)
        return;
    bManual2D[x][y]=b;
}
//---------------------------------------------------------------------------
bool TMyTray::GetManual2D(int x, int y)
{
    if(x<0 || x>=MAX_TRAY_X || y<0 || y>=MAX_TRAY_Y)
        return false;
    return bManual2D[x][y];
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260604 : tray-kind helpers
void TMyTray::SetKind(eTrayKind kind)
{
    Kind=kind;
}
//---------------------------------------------------------------------------
eTrayKind TMyTray::GetKind()
{
    return Kind;
}
//---------------------------------------------------------------------------
bool TMyTray::CanHoldIC()
{
    return (Kind==eTrayKindNormal);   // identity / cover trays must not hold IC
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260604 : TMyCar stacking-car container
TMyCar::TMyCar()
{
    Clear();
}
//---------------------------------------------------------------------------
void TMyCar::Clear()
{
    CarID="";
    iTrayCount=0;
    for(int i=0; i<MAX_TRAY_PER_CAR; i++)
        Tray[i].Clear();
}
//---------------------------------------------------------------------------
int TMyCar::GetTrayCount()
{
    return iTrayCount;
}
//---------------------------------------------------------------------------
TMyTray *TMyCar::GetTray(int index)
{
    if(index<0 || index>=MAX_TRAY_PER_CAR)
        return NULL;
    return &Tray[index];
}
//---------------------------------------------------------------------------
TMyTray *TMyCar::GetIdentityTray()
{
    for(int i=0; i<MAX_TRAY_PER_CAR; i++)
        if(Tray[i].GetKind()==eTrayKindIdentity)
            return &Tray[i];
    return NULL;
}
//---------------------------------------------------------------------------
bool TMyCar::IsFull()
{
    return (iTrayCount>=MAX_TRAY_PER_CAR);
}
//---------------------------------------------------------------------------
void TMyCar::PackForAmrUpload()
{
    //AI(HT160S-Maintainer) 20260604 : AMR upload payload not designed yet; stub.
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
    bShowSimulateCompoment=false;
    Position=0;
    EncoderPosition=0;
    bErrorMove=false;
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
    //AI(HT160S-Maintainer) 20260602 : HT172 0420 SetPersentSpeed port. Was a
    //  dead stub that only stored iPersentSpeed; the percentage never reached
    //  the motor. Now convert percent of JogHighSpeed into a raw speed and push
    //  it to the motor register (same model as HT172 mymotor.cpp SetPersentSpeed).
    //  bSave=false applies a temporary speed (e.g. transient slow-down) without
    //  overwriting the saved working percentage.
    if(persent>100)
        persent=100;
    else if(persent<1)
        persent=1;

    if(bSave)
        iPersentSpeed=persent;

    if(Motor->Enable)
    {
        int s=Motor->JogHighSpeed*persent/100;
        if(s<1)
            s=1;
        SetSpeed(s);
    }
    else
    {
        int s=SimulateSpeed*persent/100;
        if(s<1)
            s=1;
        speed=s;
    }
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
// MC88X1 home mode is fixed in code (TMyMC88X1Motor MC88X1_DEFAULT_HOME_TYPE = 90);
// the former per-motor Mot_Table HomeType column was removed. This setter is retained
// for completeness but is no longer called from the table-load path.
void TMyMotor::SetHomeType(int Type) { if(Motor!=NULL) Motor->iHomeType=Type; }
void TMyMotor::SetEncodeMultiple(int m) { if(Motor!=NULL) Motor->iEncodeMultiple=m; }
void TMyMotor::SetEncodeDir(int d) { if(Motor!=NULL) Motor->iEncodeDir=d; }
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
//AI(ht160s-maintainer) 20260625 : build the explicit numeric detail line for an
//out-of-limit popup. Units are 1/100mm (same as teach / encoder), so an engineer can
//read the requested target against the configured soft-limit band directly.
AnsiString TMyMotor::SoftLimitDetail(int p)
{
    return AnsiString().sprintf("target=%d  now=%d  soft limit N=%d ~ P=%d  (unit:1/100mm)",
        p, Position, GetSoftLimitN(), GetSoftLimitP());
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
    bool bRet;
    if(Motor->Enable)
        bRet=MotorMovePosition(Position, GetSpeed(), p);
    else
        bRet=SimulateMotorMovePosition(Position, SimulateSpeed, p);
    UpdateSimulateCompomentPosition();
    return bRet;
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
bool TMyMotor::JogP()
{
    MotorTaskLog("Motor", Alias, "JOG+",
        AnsiString("alarm=")+(Led[iAlarmLed]?"1":"0")+AnsiString(" cw=")+(Led[iCwLed]?"1":"0")+AnsiString(" ccw=")+(Led[iCcwLed]?"1":"0"));
    return Motor->JogP();
}
bool TMyMotor::JogN()
{
    MotorTaskLog("Motor", Alias, "JOG-",
        AnsiString("alarm=")+(Led[iAlarmLed]?"1":"0")+AnsiString(" cw=")+(Led[iCwLed]?"1":"0")+AnsiString(" ccw=")+(Led[iCcwLed]?"1":"0"));
    return Motor->JogN();
}
void TMyMotor::Stop() { Motor->Stop(); }
//---------------------------------------------------------------------------
// AI(general) 20260617 : delegate MC88X1 speed/accel range diagnostics to the inner
// card object (private member), so the Motor Test screen can report the card's verdict.
DWORD TMyMotor::GetLastParaError(void)
{
    return (Motor!=NULL)?Motor->GetLastParaError():0;
}
DWORD TMyMotor::VerifyHomeParaRange(void)
{
    return (Motor!=NULL)?Motor->VerifyHomeParaRange():0;
}
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
    // 172-style 3-state latched home. The old single-line form
    //   if(Motor->HomeObject() && Motor->HomeFlag()) ...
    // required BOTH true in the SAME tick. The inner card HomeObject() returns
    // true exactly once on completion and then re-arms its own task to step 1,
    // so if the home sensor (HomeFlag) was not on at that instant the completion
    // was lost and the next tick re-ran the whole home -> "HOME never stops /
    // keeps repeating". Here we LATCH the card completion (case 100), then confirm
    // the sensor separately with a bounded wait (case 200), matching HT172.
    sErr="";
    int &Task=iHomeTask;
    switch(Task)
    {
        case 1:
            if(Motor->Enable==false)
            {
                ResetPos(0);
                bHomeFlag=true;
                bHomeFinish=true;
                MotorTaskLog("Motor", Alias, "HOME_DONE", "disabled axis -> pos reset 0");
                return true;
            }
            // AI(general) 20260617 : log the REAL servo-alarm bit (GetAlarm, MotDI 0x80) plus
            // limit/encoder at HOME start. Note Led[iAlarmLed] is force-set true on a limit by
            // ScanMotorStatus, so it is NOT a reliable alarm indicator here -- GetAlarm() is.
            // A latched servo alarm (set when an OT trips the amp) cannot be cleared in software
            // (MC88X1 SetServoOn is a no-op) -> Cmove is accepted but the axis does NOT move
            // (encoder stays put). Recovery needs a SwMotorRelay Off->On power-cycle.
            Motor->ScanMotorStatus(Led);
            MotorTaskLog("Motor", Alias, "HOME_START",
                AnsiString("alarm=")+(Motor->GetAlarm()?"1":"0")+AnsiString(" cw=")+(Led[iCwLed]?"1":"0")
                +AnsiString(" ccw=")+(Led[iCcwLed]?"1":"0")+AnsiString(" enc=")+IntToStr(Motor->ReadEncoderPos())
                +AnsiString(" -> drive card home"));
            // AI(HT160S-Maintainer) 20260622 : re-init the INNER motion-card home state
            // machine (iHomeObjectTask) on every home entry, HT172-aligned. A full-machine
            // HOME cancelled mid-sequence leaves the card-home Task frozen at a polling step;
            // the re-arm path resets only the wrapper (InitHomeTask), so HomeObject() would
            // resume that stale step, never re-issue MC88X1PMotHome, and the batch never
            // converges (fHome hangs, no motion). HomeReset() also issues MC88X1PMotHomeReset
            // to clear any aborted card home. Single-axis paths already do this via
            // InitHomeTask_forSingleAxis; this covers the full-machine batch too.
            Motor->HomeReset();
            Task=100;
            break;
        case 100:
            // Poll the inner card home state machine to completion. Do NOT also test
            // HomeFlag here, and do NOT keep calling HomeObject() after it reports
            // done (it would restart the sequence). Latch by advancing to case 200.
            if(Motor->HomeObject())
            {
                dwHomeSensorWaitStart=GetTickCount();
                MotorTaskLog("Motor", Alias, "HOME_CARD_DONE",
                    AnsiString("HomeFlag=")+(Motor->HomeFlag()?"1":"0")+AnsiString(" -> confirm sensor"));
                Task=200;
            }
            else if(Motor->bHomePhaseTimeout)
            {
                // HomeType90 phase-B could not change the home-sensor state within the
                // per-leg timeout; the inner routine reset itself and will re-seek. Log
                // once per timeout so a misdirected/faulty home sensor is visible instead
                // of a silent retry loop.
                Motor->bHomePhaseTimeout=false;
                MotorTaskLog("Motor", Alias, "HOME_PHASEB_TIMEOUT",
                    "home sensor did not change within phase-B timeout; re-seeking");
            }
            break;
        case 200:
            // Confirm the home sensor like HT172, but bounded: a sensor that never
            // reports on must not leave HOME stuck. On timeout commit best-effort
            // (the card already confirmed home in case 100) and log the warning.
            if(Motor->HomeFlag())
            {
                bHomeFlag=true;
                bHomeFinish=true;
                Position=Motor->ReadPos();
                EncoderPosition=Motor->ReadEncoderPos();
                MotorTaskLog("Motor", Alias, "HOME_DONE",
                    AnsiString("pos=")+IntToStr(Position)+AnsiString(" enc=")+IntToStr(EncoderPosition));
                Task=1;
                return true;
            }
            else if((int)(GetTickCount()-dwHomeSensorWaitStart)>=HOME_SENSOR_CONFIRM_MS)
            {
                bHomeFlag=true;
                bHomeFinish=true;
                Position=Motor->ReadPos();
                EncoderPosition=Motor->ReadEncoderPos();
                MotorTaskLog("Motor", Alias, "HOME_DONE_TIMEOUT",
                    AnsiString("HomeFlag not on within timeout; committed pos=")+IntToStr(Position));
                Task=1;
                return true;
            }
            break;
        default:
            Task=1;
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
int TMyMotor::ReadPos()
{
    // Match HT172: read command AND encoder together when enabled; when the axis is
    // disabled, mirror the command onto the encoder so a powered-off motor shows a
    // consistent Now Position / Encoder pair instead of a stale feedback value.
    // (For an ENABLED servo the two legitimately differ by the following error --
    // HT172 shows the same gap; this does not force them equal while powered.)
    if(Motor->Enable)
    {
        Position=Motor->ReadPos();
        EncoderPosition=Motor->ReadEncoderPos();
    }
    else
        EncoderPosition=Position;
    UpdateSimulateCompomentPosition();
    return Position;
}
//---------------------------------------------------------------------------
int TMyMotor::ReadEncoderPos()
{
    // Align to HT172: refresh command AND encoder together from one driver transaction,
    // so a paired (Position,EncoderPosition) snapshot is consistent no matter which
    // accessor the caller used last.
    if(Motor->Enable)
    {
        Position=Motor->ReadPos();
        EncoderPosition=Motor->ReadEncoderPos();
    }
    else
        EncoderPosition=Position;
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
void TMyMotor::InitHomeTask_forSingleAxis()
{
    // Single-axis (Teach / MotorTest) re-arm. Beyond clearing the wrapper home
    // flags, reset the inner motion-card home state machine (iHomeObjectTask)
    // back to step 1. Without this, a previously interrupted home (e.g. STOP
    // pressed mid-sequence) leaves the inner Task stuck at a polling step, so the
    // next HOME press issues no new home command and the axis appears dead.
    InitHomeTask();
    if(Motor!=NULL)
        Motor->HomeReset();
}
void TMyMotor::ServoOnOff(bool IsOn) { Motor->SetServoOn(IsOn); }
// AI 20260622 : snap command(NowPos) to the encoder(feedback), HT172-aligned. After a
// servo-off (EMG) hand-move the practical/encoder register tracked the motion but the
// command register stayed frozen; ResetPos(ReadEncoderPos()) re-aligns both to the real
// position. Was ResetPos(Position) (snapped to the STALE command -> effectively a no-op).
void TMyMotor::ServoOnResetPos() { ResetPos(ReadEncoderPos()); }
void TMyMotor::ClearPosition(int cmd) { ResetPos(cmd); }
void TMyMotor::EnableTrigger(int iFlag, int iMode, long lValue) { Motor->EnableTrigger(iFlag, iMode, lValue); }
void TMyMotor::ManualTestTrigger(bool bOn) { Motor->ManualTestTrigger(bOn); }
bool TMyMotor::ReadStatus(DWORD offset, WORD *ReadData) { return Motor->ReadStatus(offset, ReadData); }
//---------------------------------------------------------------------------
// Motion View motor-position visualization (ported from HT172 / HT160S V300A).
// GetScale equivalent: scale = (refStart-refEnd) / (factStart-factEnd).
static double CalcSimulateScale(int refStart, int refEnd, int factStart, int factEnd)
{
    double denom = (double)(factStart - factEnd);
    if(denom == 0.0)
        return 1.0;
    return (double)(refStart - refEnd) / denom;
}
//---------------------------------------------------------------------------
void TMyMotor::UpdateSimulateCompomentPosition()
{
    int i, ScreenPos;
    if(bShowSimulateCompoment==false)
        return;
    MyMotorSimulateList *P;
    for(i=0; i<SimuCtrlList->Count; i++)
    {
        P=(MyMotorSimulateList *)SimuCtrlList->Items[i];
        if(P->PWinCtrl==NULL)
            continue;
        ScreenPos=(int)(P->Scale*(Position-P->FactStart))+P->RefStart;
        if(P->bUpDownMove)
            P->PWinCtrl->Top=ScreenPos;
        else
            P->PWinCtrl->Left=ScreenPos;
    }
}
//---------------------------------------------------------------------------
void TMyMotor::SetShowSimulateCompomentFlag(bool flag)
{
    bShowSimulateCompoment=flag;
}
//---------------------------------------------------------------------------
// Bind a VCL control to this motor for position visualization.
// Alignment akLeft/akRight => vertical (Top) move; otherwise horizontal (Left) move.
void TMyMotor::SetSimulateCompoment(TObject *PCtrl, TAnchorKind Alignment, int StartPos, int EndPos, int simuStartPos, int simuEndPos)
{
    int i;
    MyMotorSimulateList *TempP, *P;
    bool bUpDown = (Alignment==akLeft || Alignment==akRight);

    TWinControl *PTempWinCtrl = dynamic_cast<TWinControl *>(PCtrl);
    if(PTempWinCtrl!=NULL)
        PTempWinCtrl->DoubleBuffered=true;

    TControl *PWinCtrl = dynamic_cast<TControl *>(PCtrl);
    if(PWinCtrl==NULL)
        return;

    for(i=0; i<SimuCtrlList->Count; i++)
    {
        TempP=(MyMotorSimulateList *)SimuCtrlList->Items[i];
        if(TempP->PWinCtrl==PWinCtrl)
        {
            TempP->FactStart=StartPos;
            TempP->FactEnd=EndPos;
            TempP->RefStart=simuStartPos;
            TempP->RefEnd=simuEndPos;
            TempP->bUpDownMove=bUpDown;
            TempP->Scale=CalcSimulateScale(TempP->RefStart, TempP->RefEnd, TempP->FactStart, TempP->FactEnd);
            return;
        }
    }
    P=new MyMotorSimulateList;
    P->PWinCtrl=PWinCtrl;
    P->FactStart=StartPos;
    P->FactEnd=EndPos;
    P->RefStart=simuStartPos;
    P->RefEnd=simuEndPos;
    P->bUpDownMove=bUpDown;
    P->Scale=CalcSimulateScale(P->RefStart, P->RefEnd, P->FactStart, P->FactEnd);
    SimuCtrlList->Add((void *)P);
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
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) : init a default index->color map for a bound tray panel.
static void InitTrayColorMap(TTMyTray *p)
{
    if(p==NULL) return;
    p->SetColorMap(0,  clWhite);    //EMPTY_IC   : empty pocket
    p->SetColorMap(1,  clSkyBlue);  //UNCHECK_IC : IC present, not yet checked
    p->SetColorMap(2,  clLime);     //HAS_OK_IC  : good / passed
    p->SetColorMap(3,  clRed);
    p->SetColorMap(4,  clYellow);
    p->SetColorMap(5,  clAqua);
    p->SetColorMap(6,  clFuchsia);
    p->SetColorMap(7,  clBlue);
    p->SetColorMap(8,  clGreen);
    p->SetColorMap(9,  clOlive);
    p->SetColorMap(10, clTeal);
    p->SetColorMap(11, clPurple);
    p->SetColorMap(12, clMaroon);
    p->SetColorMap(13, clNavy);
    p->SetColorMap(14, clGray);
    p->SetColorMap(15, clSilver);
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) : repaint all bound cells from Tray.Data. Grid dims come
//from the bound panel (HT160 TMyTray has no XItem/YItem), clamped to MAX_TRAY_*.
void TTrayMotor::Refresh()
{
    UpdateTrayVisibleByHasTray();
    if(fHTary && pHTray!=NULL)
    {
        int nx=pHTray->XItem, ny=pHTray->YItem;
        if(nx>MAX_TRAY_X) nx=MAX_TRAY_X;
        if(ny>MAX_TRAY_Y) ny=MAX_TRAY_Y;
        for(int x=0; x<nx; x++)
            for(int y=0; y<ny; y++)
                pHTray->SetCellColorIndex(x, y, Tray.Data[x][y]);
    }
    if(fSubHTary && pSubHTray!=NULL)
    {
        int nx=pSubHTray->XItem, ny=pSubHTray->YItem;
        if(nx>MAX_TRAY_X) nx=MAX_TRAY_X;
        if(ny>MAX_TRAY_Y) ny=MAX_TRAY_Y;
        for(int x=0; x<nx; x++)
            for(int y=0; y<ny; y++)
                pSubHTray->SetCellColorIndex(x, y, Tray.Data[x][y]);
    }
}
void TTrayMotor::SetIDPanel(TPanel *ptr) { fPanelID=(ptr!=NULL); pPalTrayID=ptr; }
void TTrayMotor::SetTrayPanel(TPanel *ptr) { fPanel=(ptr!=NULL); pPanel=ptr; }
void TTrayMotor::SetHTrayPanel(TTMyTray *ptr) { fHTary=(ptr!=NULL); pHTray=ptr; InitTrayColorMap(ptr); UpdateTrayVisibleByHasTray(); }
void TTrayMotor::SetSubHTrayPanel(TTMyTray *ptr) { fSubHTary=(ptr!=NULL); pSubHTray=ptr; InitTrayColorMap(ptr); UpdateTrayVisibleByHasTray(); }
void TTrayMotor::CopyTrayFrom(int Index) { (void)Index; }   //AI(ht160s-tray-source) : index form unused (no VMotPtr map here); use the pointer overload
void TTrayMotor::MoveTrayFrom(int Index) { (void)Index; }
//AI(ht160s-tray-source) : motor-level copy (mirrors HT172): receive a grid + own occupancy/display.
void TTrayMotor::CopyTrayFrom(TTrayMotor *MotPtr)
{
    if(MotPtr==NULL) return;
    Tray.CopyFrom(MotPtr->Tray);
    fHasTray=true;
    Refresh();
}
//AI(ht160s-tray-source) : motor-level move = copy + clear source.
void TTrayMotor::MoveTrayFrom(TTrayMotor *MotPtr)
{
    if(MotPtr==NULL) return;
    CopyTrayFrom(MotPtr);
    MotPtr->ClearTray();
}
void TTrayMotor::SetTrayVisible(bool bVisible)
{
    if(fHTary    && pHTray    !=NULL) pHTray->Visible=bVisible;
    if(fSubHTary && pSubHTray !=NULL) pSubHTray->Visible=bVisible;
    if(fPanel    && pPanel    !=NULL) pPanel->Visible=bVisible;
    if(fPanelID  && pPalTrayID!=NULL) pPalTrayID->Visible=bVisible;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) : sync the tray CONTENT grid visibility with fHasTray.
//A freshly fed/empty car has no tray -> hide the cell grid; once a tray is
//received (fHasTray==true) show it. Position carrier panel is NOT touched here
//(it stays visible to show the Y position). Display only - no machine risk.
//The != guard avoids redundant repaints each frame.
void TTrayMotor::UpdateTrayVisibleByHasTray()
{
    if(fHTary    && pHTray    !=NULL && pHTray->Visible   !=fHasTray) pHTray->Visible=fHasTray;
    if(fSubHTary && pSubHTray !=NULL && pSubHTray->Visible!=fHasTray) pSubHTray->Visible=fHasTray;
}
//---------------------------------------------------------------------------
void TTrayMotor::SetTraySingleData(int x, int y, int data)
{
    if(y>=0 && y<MAX_TRAY_Y && x>=0 && x<MAX_TRAY_X)
        Tray.Data[x][y]=data;
    if(fHTary    && pHTray    !=NULL) pHTray->SetCellColorIndex(x, y, data);
    if(fSubHTary && pSubHTray !=NULL) pSubHTray->SetCellColorIndex(x, y, data);
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260601 : sorting-bin accessors (no display side effect)
void TTrayMotor::SetTrayBin(int x, int y, int bin)
{
    Tray.SetBin(x, y, bin);
}
//---------------------------------------------------------------------------
int TTrayMotor::GetTrayBin(int x, int y)
{
    return Tray.GetBin(x, y);
}
//---------------------------------------------------------------------------
//AI(ht160s-lotbin) 20260615 : LotIndex + 2D-code cell accessors (By Lot+Bin mode)
void TTrayMotor::SetTrayLot(int x, int y, int lot)
{
    Tray.SetLot(x, y, lot);
}
//---------------------------------------------------------------------------
int TTrayMotor::GetTrayLot(int x, int y)
{
    return Tray.GetLot(x, y);
}
//---------------------------------------------------------------------------
void TTrayMotor::SetTrayCode2D(int x, int y, AnsiString code)
{
    Tray.SetCode2D(x, y, code);
}
//---------------------------------------------------------------------------
AnsiString TTrayMotor::GetTrayCode2D(int x, int y)
{
    return Tray.GetCode2D(x, y);
}
//---------------------------------------------------------------------------
//AI(ht160s-lotpassfail) 20260709 : frozen PASS/FAIL class cell accessors (By Lot+PassFail mode)
void TTrayMotor::SetTrayPassClass(int x, int y, int c)
{
    Tray.SetPassClass(x, y, c);
}
//---------------------------------------------------------------------------
int TTrayMotor::GetTrayPassClass(int x, int y)
{
    return Tray.GetPassClass(x, y);
}
//---------------------------------------------------------------------------
void TTrayMotor::SetTrayManual2D(int x, int y, bool b)
{
    Tray.SetManual2D(x, y, b);
}
//---------------------------------------------------------------------------
bool TTrayMotor::GetTrayManual2D(int x, int y)
{
    return Tray.GetManual2D(x, y);
}
//---------------------------------------------------------------------------
void TTrayMotor::InitNewTray(int data)
{
    Tray.Birth(data, eTrayKindNormal, "");   //AI(ht160s-tray-source) : unified birth (callers re-tag Kind/TrayID, e.g. Loader)
    fHasTray=true;
    Refresh();
}
//---------------------------------------------------------------------------
void TTrayMotor::InitEmptyTray()
{
    Tray.Clear();
    fHasTray=true;
    Refresh();
}
//---------------------------------------------------------------------------
void TTrayMotor::SetTray(int data, bool bWithCover)
{
    Tray.SetAll(data);
    Tray.ClearBin();   //AI(HT160S-Maintainer) 20260601 : reset bin assignment with tray content
    fHasTray=true;
    bHasCover=bWithCover;
    Refresh();
}
//---------------------------------------------------------------------------
void TTrayMotor::ClearTray()
{
    Tray.Clear();
    fHasTray=false;
    bHasCover=false;
    Refresh();
}
//---------------------------------------------------------------------------
void TTrayMotor::SetTrayID(AnsiString ID)
{
    Tray.TrayID=ID;
}
//---------------------------------------------------------------------------