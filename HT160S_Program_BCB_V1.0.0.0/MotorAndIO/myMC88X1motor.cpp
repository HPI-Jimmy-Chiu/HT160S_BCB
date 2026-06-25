//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "myMC88X1motor.h"
#include "MC88X1P_DLL.h"
#include "..\MachineType.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//AI(general) 20260617 : home type FIXED in code (Mot_Table HomeType column removed).
//Default is 7 (card-native MC88X1MotHome) because on-machine the manual HomeType90()
//port FAILS to home most axes (the phase-B seek/leave loop stalls). Per-axis overrides
//that genuinely need 90 are applied in code at load time (the SetHomeType block in
//database.cpp motor setup, keyed by Alias), NOT via an operator-settable CSV column.
#define MC88X1_DEFAULT_HOME_TYPE       7
#define MC88X1_DEFAULT_HOME_STEP       5
#define MC88X1_DEFAULT_HOME_STEP_RANGE 100
#define MAX_MC88X1_CARD                16
//AI(general) 20260617 : HomeType90 phase-B (continuous leave / slow re-approach) per-leg
//timeout. If the home sensor never changes state within this window the home FAILS
//instead of looping forever (the old fixed-distance binary search could hang).
#define HOME_PHASEB_TIMEOUT_MS         10000
//---------------------------------------------------------------------------
static bool MC88X1CardInstall[MAX_MC88X1_CARD]={false,false,false,false,
                                                false,false,false,false,
                                                false,false,false,false,
                                                false,false,false,false};
static int MC88X1CardRefCount[MAX_MC88X1_CARD]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//---------------------------------------------------------------------------
static long RoundPulseByGear(int Pulse, double GearRatio)
{
    if(GearRatio==0)
        return Pulse;
    if(Pulse>=0)
        return (long)(Pulse/GearRatio+0.5);
    return (long)(Pulse/GearRatio-0.5);
}
//---------------------------------------------------------------------------
__fastcall TMyMC88X1Motor::TMyMC88X1Motor(int Addr)
{
    int Board=(Addr>=0)?Addr/10:0;
    int Port =(Addr>=0)?Addr%10:0;
    Address=(Addr>=0)?Addr:0;
    iBoardID=(Board>=0 && Board<MAX_MC88X1_CARD)?Board:0;
    iPortID =(Port>=0 && Port<8)?Port:0;
    bAxisID=(unsigned char)(1<<iPortID);
    bMotorStatus=0;
    bLineAxisID=bAxisID;
    bDoPort=0;
    iServoOnDoPort=-1;        // not wired until SetServoOnDoConfig() is called per axis
    bServoOnActiveHigh=true;  // A6 SI polarity; confirm against wiring
    bAxisIpBusy=false;
    bCardOpened=false;
    OldSpeed=0;
    dMC88X1Acc=0;
    OldInitSpeed=0;
    OldRate=0;
    LastParaError=0;
    iServoType=4;
    iSetpType=1;
    iTempPos=0;
    iSearchHome=0;
    iDelayReadCount=0;
    iDelayCount=0;
    iWaitCount=0;
    iHomeType=MC88X1_DEFAULT_HOME_TYPE;
    iEncodeMultiple=3;   // A/B x4 default; database.cpp overrides per Alias (M20 -> 1) before InitMotor
    iEncodeDir=1;        // SetEncodeDir: 1=inverse default; database.cpp overrides per Alias (M05 -> 0) before InitMotor
    iHomeStep=MC88X1_DEFAULT_HOME_STEP;
    iHomeStepRange=MC88X1_DEFAULT_HOME_STEP_RANGE;
    iStepRange=iHomeStepRange;
    dwHomePhaseStart=0;
    MotorIdleSafeDoorCheck=NULL;
    SetMotionCardType(eMC88x1);
}
//---------------------------------------------------------------------------
TMyMC88X1Motor::~TMyMC88X1Motor()
{
    Close_MC88X1Card();
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::Open_MC88X1Card()
{
    if(bCardOpened)
        return true;
    if(iBoardID>=MAX_MC88X1_CARD)
        return false;
    if(MC88X1CardInstall[iBoardID]==false)
    {
        if(ERROR_SUCCESS!=MC88X1PMotDevOpen((BYTE)iBoardID))
            return false;
        MC88X1CardInstall[iBoardID]=true;
    }
    MC88X1CardRefCount[iBoardID]++;
    bCardOpened=true;
    return true;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::Close_MC88X1Card()
{
    if(!bCardOpened || iBoardID>=MAX_MC88X1_CARD)
        return;
    if(MC88X1CardRefCount[iBoardID]>0)
        MC88X1CardRefCount[iBoardID]--;
    if(MC88X1CardRefCount[iBoardID]==0 && MC88X1CardInstall[iBoardID])
    {
        MC88X1CardInstall[iBoardID]=false;
        MC88X1PMotDevClose((BYTE)iBoardID);
    }
    bCardOpened=false;
}
//---------------------------------------------------------------------------
int TMyMC88X1Motor::InitMotor(int IoAddress)
{
    int Board;
    int Port;

    Address=(IoAddress>=0)?IoAddress:0;
    Board=(IoAddress>=0)?IoAddress/10:-1;
    Port =(IoAddress>=0)?IoAddress%10:-1;
    if(Board<0 || Board>=MAX_MC88X1_CARD || Port<0 || Port>=8)
    {
        Enable=false;
        bAxisID=0;
        bLineAxisID=0;
        return 0;
    }
    iBoardID=Board;
    iPortID=Port;
    bAxisID=(unsigned char)(1<<iPortID);
    bLineAxisID=bAxisID;

    //AI(general) 20260612 : open the axis card at boot even when the motor is
    //disabled in Mot_Table, so a motor enabled later from the motor-test screen
    //(reload uses bInitial=false and never calls InitMotor again) still finds an
    //opened card. Same root cause as the MN200 open-at-boot fix. Legacy HT160
    //opened the card unconditionally in the constructor. Skipped in simulation.
    if(!Enable)
    {
#ifndef SOFT_SIMULATE
        Open_MC88X1Card();
#endif
        return 0;
    }
#ifdef SOFT_SIMULATE
    //AI(HT160S-Maintainer) 20260619 : SIMULATION deepening. No physical card is
    //present, so an enabled axis (Mot_Table Enable=1) must NOT be force-disabled by
    //the failed Open_MC88X1Card below : keep it enabled so it executes and LOGS a full
    //HOME just like the real machine (same TMyMotor::Home cases, same MotorTaskLog).
    //The card is never opened (bCardOpened stays false); every card-dependent read
    //already defaults to OK on !bCardOpened (e.g. HomeFlag()==true) and HomeObject()
    //reports done under sim. Only the logical position is zeroed here. The case FLOW
    //is identical to the real machine -- only the leaf card ops are replaced by defaults.
    ResetPos(0);
    return 0;
#endif
    if(Open_MC88X1Card()==false)
    {
        Enable=false;
        return 0;
    }

    SetMC88X1SoftLimit(SoftLimitP, SoftLimitN);
    MC88X1SoftLimitEnable(false);
    MC88X1PSetNLimitLogic((BYTE)iBoardID, bAxisID, (USHORT)bSensorType);
    MC88X1PSetPLimitLogic((BYTE)iBoardID, bAxisID, (USHORT)bSensorType);
    MC88X1PSetHomeLogic((BYTE)iBoardID, bAxisID, (USHORT)bSensorType);
    ResetPos(0);

    if(iHomeType!=90)
        MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeType, iHomeType);
    else
        MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeType, 8);

    if(ServoAlarmOn)
    {
        MC88X1PSetInposition((BYTE)iBoardID, bAxisID, 0x01, 0);
        MC88X1PSetServoAlarm((BYTE)iBoardID, bAxisID, 1, 1);
    }
    else
    {
        MC88X1PSetInposition((BYTE)iBoardID, bAxisID, 0x00, 0);
        MC88X1PSetServoAlarm((BYTE)iBoardID, bAxisID, 0, 0);
        MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeType, 8);
    }

    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeP0_Dir, HomeDirection);
    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeP0_Speed, HomeHighSpeed*Range);
    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeP1_Dir, !HomeDirection);
    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeP1_Speed, HomeLowSpeed*Range);
    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeOffset, 0);
    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeP2_Dir, HomeDirection);
    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeOffset_Speed, HomeLowSpeed*Range);
    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeLimitAlarm, 0);

    if(MotorType)
        MC88X1PSetPulseMode((BYTE)iBoardID, bAxisID, (BYTE)iServoType);
    else
        MC88X1PSetPulseMode((BYTE)iBoardID, bAxisID, (BYTE)iSetpType);

    MC88X1PMotWrReg((BYTE)iBoardID, bAxisID, HomeClearPos, 0x00);
    SetSpeed(JogLowSpeed);
    // Encoder count direction. Matches the proven same-card reference HT9045
    // (Motor/HTMC88X1Motor.cpp:116 SetEncoderDir(1), :508 ReadEnCoderRealPos if(Direction)).
    // NOTE: the old HT160 pair [SetEncodeDir(1) + ReadEncoder if(Direction==0)] is
    // algebraically identical to [SetEncodeDir(0) + if(Direction)] (a no-op refactor).
    // The real sign fix is dir=1 HERE plus the symmetric if(Direction) in
    // ReadMC88X1EnCoderRealPos, which together FLIP the encoder sign vs the old asymmetric form.
    // NOTE: encoder count direction is now PER-AXIS (HTMotor::iEncodeDir, set in
    // database.cpp before InitMotor). Default 1=inverse keeps the HT9045-aligned sign;
    // M05 MLoaderY_2 is overridden to 0=normal because its A6 OA/OB feedback phase is
    // wired opposite (its Encoder otherwise reads negated vs NowPos/command). The feedback
    // (practical) counter is monitor-only in pulse-train P mode, so this flips ONLY the
    // Encoder display sign -- command/positioning/soft-limit/home==0 are unaffected.
    // Manual P.51 MC88X1PSetEncoderDir: Value 0=normal, 1=inverse.
    SetEncodeDir(iEncodeDir);
    // A/B encoder input multiplier (MC88X1PSetEncoderMultiple value):
    //   0=CW/CCW, 1=A/B x1, 2=A/B x2, 3=A/B x4.
    // Every A6 drive now outputs Pr0.11=2500 pulses/rev on OA/OB, so x4 (value 3) gives
    // 2500*4=10000/rev to match the command resolution (Pr0.08=10000) -> NowPos and
    // Encoder track 1:1 on every axis. (M12 MTopCCDX / M20 MTopCCDX_Color once shipped
    // Pr0.11=10000 and read 4x; their drives were reset to 2500 to match the rest.)
    // iEncodeMultiple is set uniformly to 3 in database.cpp before InitMotor runs.
    SetEncodeMultiple(iEncodeMultiple);
    return 0;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetMC88X1SoftLimit(int iPLimit, int iNLimit)
{
    long LP;
    long LN;
    if(GearRatio==0)
        return;
    if(Direction)
    {
        LP=(long)(-iNLimit/GearRatio);
        LN=(long)(-iPLimit/GearRatio);
    }
    else
    {
        LP=(long)(iPLimit/GearRatio);
        LN=(long)(iNLimit/GearRatio);
    }
    if(Enable && bCardOpened)
    {
        MC88X1PSetCompNLimit((BYTE)iBoardID, bAxisID, LN);
        MC88X1PSetCompPLimit((BYTE)iBoardID, bAxisID, LP);
    }
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::MC88X1SoftLimitEnable(bool bFlag)
{
    if(Enable && bCardOpened)
        MC88X1PEnableCompLimit((BYTE)iBoardID, bAxisID, bFlag?0x01:0x00);
}
//---------------------------------------------------------------------------
int TMyMC88X1Motor::ReadMC88X1RealPos()
{
    long Pos=0;
    if(Enable && bCardOpened && ERROR_SUCCESS==MC88X1PGetTheorecticalRegister((BYTE)iBoardID, bAxisID, &Pos))
    {
        if(Direction)
            Pos=-Pos;
        return (int)(Pos*GearRatio);
    }
    return HTMotor::ReadPos();
}
//---------------------------------------------------------------------------
int TMyMC88X1Motor::ReadMC88X1EnCoderRealPos()
{
    long Pos=0;
    if(Enable && bCardOpened)
    {
        // Steppers (Mot_Table ServoAlarmOn=0, e.g. SuckZ M14-M17) have no physical encoder,
        // so report the command (theoretical) register as the Encoder value -- same as the
        // proven same-card reference HT9045 (HTMC88X1Motor.cpp:503 reads the theoretical
        // register when MotorType==Step_Motor). Servos read the practical (encoder) register.
        // NOTE: HT160's MotorType is loaded from 1P2P (true on every axis) so it is NOT a
        // stepper flag here; ServoAlarmOn is (==1 servo / ==0 stepper, see database.cpp).
        int iRet;
        if(!ServoAlarmOn)
            iRet=MC88X1PGetTheorecticalRegister((BYTE)iBoardID, bAxisID, &Pos);
        else
            iRet=MC88X1PGetPracticalRegister((BYTE)iBoardID, bAxisID, &Pos);
        if(ERROR_SUCCESS==iRet)
        {
            // Symmetric with ReadMC88X1RealPos and HT9045 (:508 ReadEnCoderRealPos if(Direction)).
            // Was if(Direction==0) (asymmetric); with SetEncodeDir(1) this flips the encoder sign.
            if(Direction)
                Pos=-Pos;
            return (int)(Pos*GearRatio);
        }
    }
    return HTMotor::ReadEncoderPos();
}
//---------------------------------------------------------------------------
int TMyMC88X1Motor::SetCommand(int p)
{
    long Pos;
    HTMotor::SetCommand(p);
    if(Enable && bCardOpened && GearRatio!=0)
    {
        Pos=(long)(p/GearRatio);
        if(Direction)
            Pos=-Pos;
        MC88X1PSetTheorecticalRegister((BYTE)iBoardID, bAxisID, Pos);
    }
    return 0;
}
//---------------------------------------------------------------------------
int TMyMC88X1Motor::SetPosition(int p)
{
    long Pos;
    HTMotor::SetPosition(p);
    if(Enable && bCardOpened && GearRatio!=0)
    {
        Pos=(long)(p/GearRatio);
        // AI 20260622 : symmetric with SetCommand (if(Direction)) and the read side
        // (ReadMC88X1RealPos / ReadMC88X1EnCoderRealPos both if(Direction)) and HT9045.
        // Was if(Direction==0): after a non-zero SetPos the Encoder read back inverted vs NowPos.
        if(Direction)
            Pos=-Pos;
        MC88X1PSetPracticalRegister((BYTE)iBoardID, bAxisID, Pos);
    }
    return 0;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::MotionDone()
{
#ifdef SOFT_SIMULATE
    //AI(HT160S-Maintainer) 20260619 : SIMULATION. No card -> report an enabled axis as
    //always idle/done so moves and home phases complete instantly (instant-complete sim).
    //Without this an enabled+card-less axis would wait forever on MotionDone()==false.
    if(Enable)
        return true;
#endif
    if(Enable && bCardOpened)
        return (ERROR_SUCCESS==MC88X1PMotAxisBusy((BYTE)iBoardID, bAxisID));
    return false;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetEncodeDir(int iDir)
{
    if(Enable && bCardOpened)
        MC88X1PSetEncoderDir((BYTE)iBoardID, bAxisID, (USHORT)iDir);
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetEncodeMultiple(int iMultiple)
{
    if(Enable && bCardOpened)
        MC88X1PSetEncoderMultiple((BYTE)iBoardID, bAxisID, (USHORT)iMultiple);
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::ClearAxisAlarm()
{
    LRESULT Status;
    if(!Enable || !bCardOpened)
        return;
    Status=MC88X1PMotAxisBusy((BYTE)iBoardID, bAxisID);
    if(Status==AxisIpBusy)
        MotIpReset();
    else if(Status==AxisHomeBusy || Status==AxisHomeErr)
        HomeReset();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::Stop()
{
    if(Enable && bCardOpened)
        MC88X1PMotStop((BYTE)iBoardID, bAxisID, 0);
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::DecStop()
{
    // Deceleration stop for this axis. MC88X1 manual p.98: MC88X1PMotStop's
    // byStopMode is a per-axis bit field; for the selected axis mode bit 0 =
    // decelerate-stop, bit 1 = immediate (sudden) stop. Passing 0 keeps this
    // axis bit clear -> decel stop (the same call Stop() already uses, so on
    // MC88X1 Stop() and DecStop() both decelerate; the card sudden stop,
    // mode = bAxisID, is deliberately not used here). This was the inherited
    // HTMotor no-op, which made HSys.DecStopAllMotor() -- the decel path on
    // every alarm (ShowNoteAlarm) and on SystemStart going false (DoAllProcess)
    // -- do NOTHING on real MC88X1 cards. Implementing it makes that safety
    // decel actually slow the hardware.
    if(Enable && bCardOpened)
        MC88X1PMotStop((BYTE)iBoardID, bAxisID, 0);
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::JogP()
{
    if(!Enable || !bCardOpened)
        return false;
    if(MotorIdleSafeDoorCheck!=NULL && MotorIdleSafeDoorCheck()==true)
        return false;
    // AI(general) 20260618 : Jog+ = Cmove CW on Direction=0 (Cmove 0), CCW on Direction=1
    // (Cmove bit; manual p.92 dir 0=CW,1=CCW). Jog direction is correct as written; the form
    // StartJog blocks jogging INTO a lit limit (spatial wiring: iCwLed=Jog+ end). CAVEAT
    // (MCD451 manual p.37): the card hard limit is per-pulse-direction (-LM stops CCW, +LM
    // stops CW), so on a Direction=1 axis the off-the-limit Jog IS the pulse the active limit
    // blocks -- escaping a lit limit can need the hard limit temporarily relaxed (under
    // review; not implemented here). The old Direction-blind Stop+MotIpReset recovery was
    // removed (it could not clear that block and mis-fired by Direction). Only clear a
    // still-busy IP before re-commanding.
    if(!MotionDone())
        ClearAxisAlarm();
    if(Direction)
        MC88X1PMotCmove((BYTE)iBoardID, bAxisID, bAxisID);
    else
        MC88X1PMotCmove((BYTE)iBoardID, bAxisID, 0x00);
    return true;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::JogN()
{
    if(!Enable || !bCardOpened)
        return false;
    if(MotorIdleSafeDoorCheck!=NULL && MotorIdleSafeDoorCheck()==true)
        return false;
    // AI(general) 20260618 : Jog- = Cmove CCW on Direction=0 (Cmove bit), CW on Direction=1
    // (Cmove 0). See JogP for the per-pulse-direction hard-limit caveat (MCD451 p.37) and why
    // the old Direction-blind Stop+MotIpReset recovery was removed. Only clear a still-busy IP.
    if(!MotionDone())
        ClearAxisAlarm();
    if(Direction)
        MC88X1PMotCmove((BYTE)iBoardID, bAxisID, 0x00);
    else
        MC88X1PMotCmove((BYTE)iBoardID, bAxisID, bAxisID);
    return true;
}
//---------------------------------------------------------------------------
int TMyMC88X1Motor::ReadPos()
{
    return ReadMC88X1RealPos();
}
//---------------------------------------------------------------------------
int TMyMC88X1Motor::ReadEncoderPos()
{
    return ReadMC88X1EnCoderRealPos();
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::MoveTo(int Tar)
{
    long Distance[8]={0,0,0,0,0,0,0,0};
    long TargetPulse;
    long NowPulse;
    if(MotorIdleSafeDoorCheck!=NULL && MotorIdleSafeDoorCheck()==true)
        return false;
    if(!Enable)
    {
        SetCommand(Tar);
        SetPosition(Tar);
        return true;
    }
#ifdef SOFT_SIMULATE
    //AI(HT160S-Maintainer) 20260619 : SIMULATION. No card -> complete the move instantly
    //(set command+position, report done) so an enabled axis simulates motion and logs the
    //same task/case flow as the real machine. The soft-limit range guard still applies.
    if(!CheckArmPosInRange(Tar, SoftLimitN, SoftLimitP))
        return false;
    SetCommand(Tar);
    SetPosition(Tar);
    return true;
#endif
    if(!bCardOpened || GearRatio==0 || iPortID>=8)
        return false;
    if(!CheckArmPosInRange(Tar, SoftLimitN, SoftLimitP))
        return false;
    if(!MotionDone())
        return false;
    TargetPulse=RoundPulseByGear(Tar, GearRatio);
    Distance[iPortID]=Direction?-TargetPulse:TargetPulse;
    MC88X1PMotPtp((BYTE)iBoardID, bAxisID, bAxisID,
                  Distance[0], Distance[1], Distance[2], Distance[3],
                  Distance[4], Distance[5], Distance[6], Distance[7]);
    //AI(general) 20260612 : compare raw card pulses like legacy HT160. The unit
    //domain compare (ReadPos()/GearRatio) truncates and never matches for the
    //fractional gear ratios in Mot_Table (0.9, 0.5, 0.1, 0.04), so the move task
    //would never report done.
    NowPulse=0;
    if(MotionDone() && ERROR_SUCCESS==MC88X1PGetTheorecticalRegister((BYTE)iBoardID, bAxisID, &NowPulse))
    {
        if(Direction)
            NowPulse=-NowPulse;
        if(NowPulse==TargetPulse)
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::HomeObject()
{
    if(MotorIdleSafeDoorCheck!=NULL && MotorIdleSafeDoorCheck()==true)
        return false;
#ifdef SOFT_SIMULATE
    //AI(HT160S-Maintainer) 20260619 : SIMULATION. With no card the real card-home
    //state machine (MC88X1MotHome) cannot step to completion, so report the inner card
    //home as instantly done. The higher-level TMyMotor::Home() still runs its full case
    //path (HOME_START -> HOME_CARD_DONE -> HOME_DONE) identically to the real machine;
    //HomeFlag() already returns true with no card so the case-200 sensor confirm passes.
    ResetPos(0);
    return true;
#endif
    if(iHomeType==90 && iHomeStep>0)
        return HomeType90();
    return MC88X1MotHome();
}
//---------------------------------------------------------------------------
// AI(general) 20260617 : RETIRED / DORMANT. As of 20260617 database.cpp sets ALL axes to
// card-native HomeType 7 (latency-immune hardware home), so HomeObject() no longer routes
// here (iHomeType==90 is unused at runtime). Kept as a per-axis FALLBACK: if an axis's
// mechanism ever needs the adaptive software seek (continuous leave / slow re-approach), or
// card-native home mis-behaves from a limit on that axis, flip it back to 90 in database.cpp.
// Its limit-escape idea (case 11 cw/ccw check) was ported into MC88X1MotHome (cases 2-4).
// Do NOT delete until type 7 is on-machine verified on every axis.
bool TMyMC88X1Motor::HomeType90()
{
    int &Task=iHomeObjectTask;
    unsigned int iAccPersent;
    unsigned int iAccMax;
    bool bScanLed[iMotLedTotalCnt];

    switch(Task)
    {
        case 1:
            if(!MotionDone())
            {
                ClearAxisAlarm();
                break;
            }
            iWaitCount=0;
            iDelayCount=0;
            bHomePhaseTimeout=false;
            iStepRange=iHomeStepRange;
            if(iStepRange<5)
                iStepRange=5;
            else if(iStepRange>500)
                iStepRange=500;
            SetCommand(0);
            SetPosition(0);
            OldSpeed=iSpeed;
            OldInitSpeed=InitSpeed;
            Stop();
            InitMotor(Address);
            MC88X1SoftLimitEnable(false);
            Task=10;
            break;
        case 10:
            iAccPersent=(HomeHighSpeed*Range)/65535;
            if(iAccPersent==0)
                iAccPersent=1;
            iAccMax=iAccPersent*8192000;
            MC88X1PMotAxisParaSet((BYTE)iBoardID, bAxisID, 0,
                                  (HomeLowSpeed*Range)/2,
                                  HomeLowSpeed*Range,
                                  HomeHighSpeed*Range,
                                  iAccMax, 100);
            SetInitSpeed(1);
            SetSpeed(HomeHighSpeed);
            iSearchHome=0;
            Task=11;
            break;
        case 11:
            ScanMotorStatus(bScanLed);
            if(bScanLed[iHomeLed])
            {
                Task=20;
                break;
            }
            else if(bScanLed[iCcwLed])
            {
                JogP();
                Task=12;
            }
            else if(bScanLed[iCwLed])
            {
                JogN();
                Task=12;
            }
            else
            {
                if(Direction)
                {
                    if(HomeDirection)
                        JogP();
                    else
                        JogN();
                }
                else
                {
                    if(HomeDirection)
                        JogN();
                    else
                        JogP();
                }
                Task=20;
            }
            break;
        case 12:
            ScanMotorStatus(bScanLed);
            if(bScanLed[iCcwLed]==false || bScanLed[iCwLed]==false)
                Task=20;
            break;
        case 20:
            ScanMotorStatus(bScanLed);
            if(bScanLed[iHomeLed])
            {
                Stop();
                Task=21;
            }
            else if(bScanLed[iCwLed] || bScanLed[iCcwLed])
            {
                Stop();
                if(iSearchHome>=1)
                    return false;
                iSearchHome++;
                Task=22;
            }
            break;
        case 21:
            if(MotionDone())
            {
                Task=100;
                iTempPos=ReadPos();
            }
            break;
        case 22:
            if(MotionDone())
            {
                iSearchHome++;
                Task=11;
            }
            break;
        // ---- phase-B (172 / original-HT160 MC88X1PosDirectHome style): continuous
        // reverse LEAVE then slow RE-APPROACH, bounded by HOME_PHASEB_TIMEOUT_MS. This
        // replaces the old fixed-distance binary search (LevalHomeSensorConstDistance),
        // which could spin case 100<->110 forever when the const step failed to clear the
        // sensor and was sensitive to HomeLowSpeed/GearRatio. The seek direction mirrors
        // case 11 (Direction?HomeDirection:!HomeDirection); LEAVE jogs opposite the seek
        // direction, RE-APPROACH jogs in the seek direction. The home sensor is read via
        // ScanMotorStatus (same path the seek phase uses) for consistency.
        case 100:
            // begin leaving the home sensor (jog AWAY from the seek direction)
            if(MotionDone()==false)
                break;
            SetSpeed(HomeLowSpeed);
            dwHomePhaseStart=GetTickCount();
            if(Direction?HomeDirection:!HomeDirection)
                JogN();
            else
                JogP();
            Task=110;
            break;
        case 110:
            // leaving: stop the moment the sensor releases; fail on timeout (never hang)
            ScanMotorStatus(bScanLed);
            if(bScanLed[iHomeLed]==false)
            {
                Stop();
                Task=120;
                break;
            }
            if((int)(GetTickCount()-dwHomePhaseStart)>=HOME_PHASEB_TIMEOUT_MS)
            {
                Stop();
                bHomePhaseTimeout=true;
                Task=1;
                return false;
            }
            break;
        case 120:
            // settle after the leave-stop, then begin the slow re-approach (jog back
            // IN the seek direction)
            if(MotionDone()==false)
                break;
            SetSpeed(HomeLowSpeed);
            dwHomePhaseStart=GetTickCount();
            if(Direction?HomeDirection:!HomeDirection)
                JogP();
            else
                JogN();
            Task=130;
            break;
        case 130:
            // re-approaching: stop the moment the sensor triggers; fail on timeout
            ScanMotorStatus(bScanLed);
            if(bScanLed[iHomeLed])
            {
                Stop();
                Task=200;
                break;
            }
            if((int)(GetTickCount()-dwHomePhaseStart)>=HOME_PHASEB_TIMEOUT_MS)
            {
                Stop();
                bHomePhaseTimeout=true;
                Task=1;
                return false;
            }
            break;
        case 200:
            if(MotionDone()==false)
                break;
            ScanMotorStatus(bScanLed);
            if(bScanLed[iHomeLed]==false)
            {
                Task=1;
                return false;
            }
            LastHomePos=ReadMC88X1RealPos();
            SetCommand(0);
            SetPosition(0);
            SetMC88X1SoftLimit(SoftLimitP, SoftLimitN);
            MC88X1SoftLimitEnable(true);
            SetInitSpeed(OldInitSpeed);
            SetSpeed(OldSpeed);
            Task=1;
            return true;
        default:
            Task=1;
            return false;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::MC88X1MotHome()
{
    int &Task=iHomeObjectTask;
    unsigned int iAccPersent;
    unsigned int iAccMax;
    bool bScanLed[iMotLedTotalCnt];
    switch(Task)
    {
        case 1:
            if(!MotionDone())
            {
                ClearAxisAlarm();
                break;
            }
            ResetPos(0);
            OldSpeed=iSpeed;
            Stop();
            InitMotor(Address);
            MC88X1SoftLimitEnable(false);
            LastHomePos=0;
            Task=2;
            break;
        // AI(general) 20260618 : limit pre-escape (Direction-INDEPENDENT, spatial convention).
        // Card-native MC88X1PMotHome only drives HomeP0_Dir; if the axis sits on the hard limit
        // in that direction it pushes harder in (observed MAutoY_6). Escape first: the limit
        // switches are wired by spatial convention, uniform across axes (Jog+ -end = CW/iCwLed,
        // Jog- -end = CCW/iCcwLed), so a lit iCwLed escapes via JogN, a lit iCcwLed via JogP.
        // (A Direction-aware variant was tried and reverted -- it was backwards on Direction=1
        // axes.) CAVEAT (MCD451 manual p.37): the card hard limit is per-PULSE-direction
        // (+LM stops CW, -LM stops CCW); on Direction=1 axes the escape Jog IS the pulse the
        // active limit blocks, so this Jog may not actually move until that card-level block is
        // addressed (see JogP/JogN). Bounded by HOME_PHASEB_TIMEOUT_MS; runs at JogLowSpeed.
        case 2:
            ScanMotorStatus(bScanLed);
            if(bScanLed[iCwLed])
            {
                JogN();
                dwHomePhaseStart=GetTickCount();
                Task=3;
            }
            else if(bScanLed[iCcwLed])
            {
                JogP();
                dwHomePhaseStart=GetTickCount();
                Task=3;
            }
            else
                Task=10;
            break;
        case 3:
            ScanMotorStatus(bScanLed);
            if(bScanLed[iCwLed]==false && bScanLed[iCcwLed]==false)
            {
                Stop();
                Task=4;
                break;
            }
            if((int)(GetTickCount()-dwHomePhaseStart)>=HOME_PHASEB_TIMEOUT_MS)
            {
                Stop();
                Task=1;
                return false;
            }
            break;
        case 4:
            if(MotionDone())
                Task=10;
            break;
        case 10:
            iAccPersent=(HomeHighSpeed*Range)/65535;
            if(iAccPersent==0)
                iAccPersent=1;
            iAccMax=iAccPersent*8192000;
            MC88X1PMotAxisParaSet((BYTE)iBoardID, bAxisID, 0,
                                  (HomeLowSpeed*Range)/2,
                                  HomeLowSpeed*Range,
                                  HomeHighSpeed*Range,
                                  iAccMax, 100);
            Task=11;
            break;
        case 11:
            if(ERROR_SUCCESS==MC88X1PMotHome((BYTE)iBoardID, bAxisID))
                Task=20;
            break;
        case 20:
            if(ERROR_SUCCESS==MC88X1PMotHomeStatus((BYTE)iBoardID, bAxisID))
                Task=30;
            break;
        case 30:
            SetPosition(0);
            iWaitCount=0;
            Task=31;
            break;
        case 31:
            // Best-effort encoder-zero confirmation after the card home completed.
            // The encoder often settles a few counts off zero, so do NOT spin here
            // forever (the legacy "else Task=30" looped endlessly and left the HOME
            // button stuck on "HOMING.."). Retry a bounded number of times, then
            // proceed so HomeObject() reliably reports done.
            if(ReadMC88X1EnCoderRealPos()==0 || iWaitCount>=10)
            {
                Task=32;
            }
            else
            {
                iWaitCount++;
                SetPosition(0);
            }
            break;
        case 32:
            LastHomePos=ReadMC88X1RealPos();
            SetCommand(0);
            SetMC88X1SoftLimit(SoftLimitP, SoftLimitN);
            MC88X1SoftLimitEnable(true);
            SetSpeed(OldSpeed);
            Task=1;
            return true;
        default:
            Task=1;
            return false;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::HomeFlag(void)
{
    bool Flag;
    if(!Enable || !bCardOpened)
        return true;
    //AI(general) 20260617 : home sensor is IN3 = MotDI bit 0x08 for ALL home types
    //(per MC88X1 manual: SetHomeLogic configures IN3; MotDI bit3 = IN3). The old
    //type-7/non-90 path read a different status word (ReadStatus 0x08 bit 0x0080) with
    //INVERTED polarity, so card-native(type 7) axes reported HomeFlag=0 at home and hit
    //a false HOME_DONE_TIMEOUT. Unify on the MotDI/IN3 read that matches ScanMotorStatus
    //and already worked for type 90.
    MC88X1PMotDI((BYTE)iBoardID, bAxisID, &bMotorStatus);
    Flag=(bMotorStatus & 0x08)!=0;
    return bSensorType?Flag:!Flag;
}
//---------------------------------------------------------------------------
DWORD TMyMC88X1Motor::GetLastParaError(void)
{
    return LastParaError;
}
//---------------------------------------------------------------------------
DWORD TMyMC88X1Motor::VerifyHomeParaRange(void)
{
    // AI(general) 20260617 : dry-run the HOME-seek parameter set the way HomeType90
    // case 10 + SetSpeed(HomeHighSpeed) build it (SV=1*Range, DV=HomeHighSpeed*Range,
    // MDV=JogHighSpeed*Range, AC=(DV-SV)/Acc clamped), read the card's range verdict,
    // then restore the running profile. Returns the AxisParaSet code (0=ok). The axis
    // is only re-parameterised (no motion is issued), safe while idle on the test screen.
    double AccSec;
    double dAccLocal;
    unsigned int iAccPersent;
    unsigned int iAccMin;
    unsigned int iAccMax;
    DWORD Sv;
    DWORD Dv;
    DWORD Mdv;
    DWORD Code;
    // home dry-run models the type-90 software-seek profile (SetInitSpeed(1)+
    // SetSpeed(HomeHighSpeed) -> MDV=JogHighSpeed*Range). For card-native type 7 the
    // home speeds come from the Home registers via MC88X1PMotHome, not this path, so
    // skip to avoid a misleading range warning.
    if(!Enable || !bCardOpened || Range==0 || iHomeType!=90)
        return 0;
    Sv =1*Range;
    Dv =HomeHighSpeed*Range;
    Mdv=JogHighSpeed*Range;
    AccSec=(dAcc<=0)?0.1:dAcc;
    dAccLocal=((double)Dv-(double)Sv)/AccSec;
    iAccPersent=Dv/65535;
    if(iAccPersent==0)
        iAccPersent=1;
    iAccMin=iAccPersent*2001;
    iAccMax=iAccPersent*8192000;
    if(dAccLocal<iAccMin)
        dAccLocal=iAccMin;
    if(dAccLocal>iAccMax)
        dAccLocal=iAccMax;
    Code=(DWORD)MC88X1PMotAxisParaSet((BYTE)iBoardID, bAxisID, 0,
                              Sv, Dv, Mdv, (DWORD)dAccLocal, 100);
    SetMC88X1MotPara();
    return Code;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::GetAlarm(void)
{
    if(!Enable || !ServoAlarmOn)
        return false;
    if(bCardOpened)
        MC88X1PMotDI((BYTE)iBoardID, bAxisID, &bMotorStatus);
    return (bMotorStatus & 0x80)!=0;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetMC88X1MotPara()
{
    double AccSec;
    unsigned int iAccPersent;
    unsigned int iAccMax;
    unsigned int iAccMin;
    unsigned int iChangSpeed[8]={1,1,1,1,1,1,1,1};
    if(!Enable || !bCardOpened || Range==0)
        return;
    if(MotionDone())
    {
        AccSec=(dAcc<=0)?0.1:dAcc;
        dMC88X1Acc=(iSpeed*Range-InitSpeed*Range)/AccSec;
        iAccPersent=(iSpeed*Range)/65535;
        if(iAccPersent==0)
            iAccPersent=1;
        iAccMin=iAccPersent*2001;
        iAccMax=iAccPersent*8192000;
        if(dMC88X1Acc<iAccMin)
            dMC88X1Acc=iAccMin;
        if(dMC88X1Acc>iAccMax)
            dMC88X1Acc=iAccMax;
        LastParaError=(DWORD)MC88X1PMotAxisParaSet((BYTE)iBoardID, bAxisID, 0,
                              InitSpeed*Range, iSpeed*Range,
                              JogHighSpeed*Range, (DWORD)dMC88X1Acc, 100);
    }
    else if(iPortID<8)
    {
        iChangSpeed[iPortID]=iSpeed*Range;
        MC88X1PMotChgDV((BYTE)iBoardID, bAxisID,
                        iChangSpeed[0], iChangSpeed[1], iChangSpeed[2], iChangSpeed[3],
                        iChangSpeed[4], iChangSpeed[5], iChangSpeed[6], iChangSpeed[7]);
    }
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetSpeed(unsigned int x)
{
    if(x>8191)
        x=8191;
    else if(x<1)
        x=1;
    if(x<InitSpeed)
        x=InitSpeed*2;
    if(x>8191)
        x=8191;
    iSpeed=x;
    SetMC88X1MotPara();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetInitSpeed(unsigned int x)
{
    InitSpeed=x;
    SetMC88X1MotPara();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::ScanMotorStatus(bool *Led)
{
    unsigned short MotionStatus;
    bool Flag;
    if(Led==NULL)
        return;
    for(int i=0; i<iMotLedTotalCnt; i++)
        Led[i]=false;
    if(!Enable || !bCardOpened)
        return;
    MC88X1PMotDI((BYTE)iBoardID, bAxisID, &bMotorStatus);
    Led[iInposLed]=(bMotorStatus & 0x40)!=0;
    Led[iAlarmLed]=ServoAlarmOn?((bMotorStatus & 0x80)!=0):false;
    Flag=(bMotorStatus & 0x08)!=0;
    Led[iHomeLed]=bSensorType?Flag:!Flag;
    Led[iEmgLed]=!(bMotorStatus & 0x02);
    Led[iZPhaseLed]=(bMotorStatus & 0x01)!=0;
    MC88X1PGetMotionInput((BYTE)iBoardID, bAxisID, &MotionStatus);
    Led[iCwLed]=(MotionStatus & 0x04)!=0;
    Led[iCcwLed]=(MotionStatus & 0x08)!=0;
    Led[iSoftcwLed]=(MotionStatus & 0x01)!=0;
    Led[iSoftccwLed]=(MotionStatus & 0x02)!=0;
    Led[iServoalarmLed]=(MotionStatus & 0x10)!=0;
    Led[iServoOn]=Enable;
    if(Led[iCwLed] || Led[iCcwLed])
        Led[iAlarmLed]=true;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetServoAlarmOn(bool Value)
{
    ServoAlarmOn=Value;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetRange(unsigned int a)
{
    if(a>1000)
        a=1000;
    if(a<1)
        a=1;
    Range=a;
    SetMC88X1MotPara();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetAcc(double a)
{
    dAcc=a;
    SetMC88X1MotPara();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetRate(unsigned int a)
{
    if(dAcc!=0)
        return;
    if(a<1)
        a=1;
    Rate=a;
    dAcc=(JogHighSpeed-InitSpeed)*Rate/8000000.0;
    SetMC88X1MotPara();
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::ResetPos(int Pulse)
{
    SetCommand(Pulse);
    SetPosition(Pulse);
    return true;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::HomeReset()
{
    if(Enable && bCardOpened)
        MC88X1PMotHomeReset((BYTE)iBoardID, bAxisID);
    HTMotor::HomeReset();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::MotOutputOn(int iOutPort)
{
    if(!Enable || !bCardOpened || iOutPort<4 || iOutPort>7)
        return;
    bDoPort=(unsigned char)(bDoPort | (1<<(iOutPort-4)));
    MC88X1PMotDO((BYTE)iBoardID, bAxisID, bDoPort);
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::MotOutputOff(int iOutPort)
{
    unsigned char bMask;
    if(!Enable || !bCardOpened || iOutPort<4 || iOutPort>7)
        return;
    bMask=(unsigned char)(1<<(iOutPort-4));
    bDoPort=(unsigned char)(bDoPort & (~bMask));
    MC88X1PMotDO((BYTE)iBoardID, bAxisID, bDoPort);
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetServoOnDoConfig(int iOutPort, bool bActiveHigh)
{
    iServoOnDoPort=iOutPort;
    bServoOnActiveHigh=bActiveHigh;
}
//---------------------------------------------------------------------------
// MC88X1/MCD451 has no dedicated servo-on command (unlike MN200 mn_servo_on);
// the only per-axis actuator is the general-purpose DO (MC88X1PMotDO OUT4..OUT7,
// iOutPort 4..7). iServoOnDoPort selects the OUT pin wired to this axis A6 SRV-ON
// input; -1 means "not wired", so SetServoOn stays a safe no-op until the field
// wiring is confirmed and SetServoOnDoConfig() is set per axis. bServoOnActiveHigh
// selects whether a HIGH or LOW DO bit asserts SRV-ON (A6 SI input polarity).
void TMyMC88X1Motor::SetServoOn(bool IsOn)
{
    bool bAssert;
    if(!Enable || !bCardOpened)
        return;
    if(iServoOnDoPort<4 || iServoOnDoPort>7)
        return;
    bAssert=bServoOnActiveHigh?IsOn:(!IsOn);
    if(bAssert)
        MotOutputOn(iServoOnDoPort);
    else
        MotOutputOff(iServoOnDoPort);
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::MotInputStatus(bool *bInputPort)
{
    if(bInputPort==NULL)
        return;
    bInputPort[0]=false;
    bInputPort[1]=false;
    if(Enable && bCardOpened)
    {
        MC88X1PMotDI((BYTE)iBoardID, bAxisID, &bMotorStatus);
        bInputPort[0]=!(bMotorStatus & 0x02);
        bInputPort[1]=!(bMotorStatus & 0x04);
    }
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::EnableTrigger(int iFlag, int iMode, long lValue)
{
    long lPitch;
    if(!Enable || !bCardOpened || GearRatio==0)
        return;
    if(iFlag)
    {
        lPitch=(long)(lValue/GearRatio);
        if(iMode==1)
            MC88X1PSetPichPulseMode((BYTE)iBoardID, bAxisID, 0x01);
        else if(iMode==2)
            MC88X1PSetPichPulseMode((BYTE)iBoardID, bAxisID, 0x02);
        else if(iMode==3)
            MC88X1PSetPichPulseMode((BYTE)iBoardID, bAxisID, 0x03);
        MC88X1PSetPichPulseCounter((BYTE)iBoardID, bAxisID, 0x00);
        MC88X1PSetPichData((BYTE)iBoardID, bAxisID, (WORD)lPitch);
    }
    else
        MC88X1PSetPichPulseMode((BYTE)iBoardID, bAxisID, 0x00);
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::LinearAxisMoveTo(int iPortID[8], long lPos[8], bool bFlag)
{
    if(MotorIdleSafeDoorCheck!=NULL && MotorIdleSafeDoorCheck()==true)
        return false;
    if(!Enable || !bCardOpened)
        return false;
    if(bFlag==false)
    {
        bLineAxisID=0;
        for(int i=0; i<8; i++)
        {
            if(iPortID[i]!=-1)
                bLineAxisID=(unsigned char)(bLineAxisID+(1<<iPortID[i]));
        }
        if(AxisIpBusy==MC88X1PMotAxisBusy((BYTE)iBoardID, bAxisID))
            MotIpReset();
        MC88X1PMotLine((BYTE)iBoardID, bLineAxisID, 255,
                       lPos[0], lPos[1], lPos[2], lPos[3],
                       lPos[4], lPos[5], lPos[6], lPos[7]);
    }
    else
    {
        if(ERROR_SUCCESS==MC88X1PMotAxisBusy((BYTE)iBoardID, bLineAxisID))
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::ReadStatus(DWORD offset, WORD *ReadData)
{
    DWORD Port;
    WORD Data=0;
    if(ReadData==NULL)
        return false;
    Port=(0x80)*(iPortID/4)+(0x10)*(iPortID%4);
    if(Enable && bCardOpened)
    {
        if(MC88X1PReadWord((BYTE)iBoardID, offset+Port, &Data)==FALSE)
            return false;
    }
    *ReadData=Data;
    return true;
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::WriteStatus(DWORD offset, WORD WriteData)
{
    DWORD Port;
    Port=(0x80)*(iPortID/4)+(0x10)*(iPortID%4);
    if(Enable && bCardOpened)
        return MC88X1PWriteWord((BYTE)iBoardID, offset+Port, WriteData)!=FALSE;
    return true;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::MotIpReset()
{
    if(Enable && bCardOpened)
        MC88X1PMotIpReset((BYTE)iBoardID);
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::IsMotorBusy()
{
    return !MotionDone();
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::MotorReset()
{
    if(Enable && bCardOpened)
        MC88X1PMotReset((BYTE)iBoardID);
}
//---------------------------------------------------------------------------
bool TMyMC88X1Motor::IsMotorAlarm()
{
    if(!Enable || !bCardOpened)
        return false;
    MC88X1PMotDI((BYTE)iBoardID, bAxisID, &bMotorStatus);
    return (bMotorStatus & 0x80)!=0;
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::EnableSetInPosition(bool SetEnable)
{
    if(!Enable || !bCardOpened)
        return;
    if(SetEnable)
        MC88X1PSetInposition((BYTE)iBoardID, bAxisID, 0x01, 0);
    else
        MC88X1PSetInposition((BYTE)iBoardID, bAxisID, 0x00, 0);
}
//---------------------------------------------------------------------------
void TMyMC88X1Motor::SetPos(int pos)
{
    //AI(general) 20260612 : SetCommand of this class already negates by Direction
    //internally (legacy SetCommand did not), so the legacy pre-negation here would
    //double-negate the command register and ReadPos would return -pos.
    SetCommand(pos);
    SetPosition(pos);
}
//---------------------------------------------------------------------------
