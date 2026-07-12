//---------------------------------------------------------------------------
// uHome.cpp - Full machine HOME monitor form for HT160S
// AI(HT160S-Maintainer) 20260602 : Ported in HT172 TfHome style.
//   Display-only motor grid (name + TALed home LED + live position).
//   The Home button on the main form triggers the existing Run_Home
//   engine and shows this monitor non-modally; the engine drives the
//   actual homing while the timer here updates LEDs/positions. When all
//   motors are homed (fAllMotorHome) the monitor auto-closes. The bottom
//   "Abort Home" button stops all motors and closes.
//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop
#include "language.h"

#include "uHome.h"
#include "database.h"
#include "cmydef.h"
#include "csystem.h"
#include "aTrayArm.h"   //AI(HT160S-Maintainer) 20260612 : TrayArmModule->HasTray() guard for held-tray home
#include "aLoader.h"    //AI(HT160S-Maintainer) 20260617 : LoaderModule + held-tray safe-home clamp release
#include "aEmpty.h"     //AI(ht160s-home-resume-w3b2) 20260711 : Empty carriage PARK haul-target
#include "aColor.h"     //AI(ht160s-home-resume-w3b2) 20260711 : Color carriage PARK haul-target
#include "GeneralSetting.h"   //AI(ht160s-home-resume-w3c) 20260711 : iHomeReacquireOffsetCnt
#include "aAuto1To6.h"   //AI(ht160s-home-resume-drain) 20260711 : Auto drain hook (AF-2 commit stand-in)
#include "aSortArm.h"    //AI(ht160s-home-resume-drain) 20260711 : SP-1 vacuum reconciliation + blow-off
#include "uAgvStation.h" //AI(ht160s-home-resume-drain) 20260711 : D1 - freeze destacker drains at non-IDLE AMR stations
#pragma package(smart_init)
#pragma link "ALed"
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfHome *fHome;
//---------------------------------------------------------------------------
static const int iRowCount = 14;
//---------------------------------------------------------------------------
__fastcall TfHome::TfHome(TComponent* Owner)
    : TForm(Owner)
{
    iHomeStep=1;
    fShow=false;
    fSeenStart=false;
    fGridBuilt=false;
    for(int i=0;i<HOME_MOTOR_MAX;i++)
    {
        LabelMotorName[i]=NULL;
        LedPtr[i]=NULL;
        EditMotorPos[i]=NULL;
    }
    if(ComponentState.Contains(csDesigning))
        return;
    BuildMotorGrid();
}
//---------------------------------------------------------------------------
// Build the motor grid dynamically (HT172 layout: 14 rows per column,
// 300px column stride; label / LED / position edit per motor).
//---------------------------------------------------------------------------
void TfHome::BuildMotorGrid()
{
    const int LedPitch  = 150;
    const int EditPitch = 185;
    int i;

    if(fGridBuilt)
        return;
    if(HSys.MotPtr==NULL)
        return;

    for(i=0;i<HSys.iTotalMotor && i<HOME_MOTOR_MAX;i++)
    {
        if(HSys.MotPtr[i]==NULL)
            continue;

        LabelMotorName[i]=new TLabel(this);
        LabelMotorName[i]->Parent=Panel1;
        LabelMotorName[i]->Left=5+300*(i/iRowCount);
        LabelMotorName[i]->Top =8+ 30*(i%iRowCount);
        LabelMotorName[i]->Caption=HSys.MotPtr[i]->NumberAlias;
        LabelMotorName[i]->Font->Size=10;

        LedPtr[i]=new TALed(this);
        LedPtr[i]->Parent=Panel1;
        LedPtr[i]->Left=LedPitch+300*(i/iRowCount);
        LedPtr[i]->Top =8+ 30*(i%iRowCount);
        LedPtr[i]->LEDStyle=LEDSqLarge;
        LedPtr[i]->Blink=false;
        LedPtr[i]->FalseColor=clSilver;
        LedPtr[i]->TrueColor=clLime;

        EditMotorPos[i]=new TEdit(this);
        EditMotorPos[i]->Parent=Panel1;
        EditMotorPos[i]->Left=EditPitch+300*(i/iRowCount);
        EditMotorPos[i]->Top =8+ 30*(i%iRowCount);
        EditMotorPos[i]->Width=95;
        EditMotorPos[i]->Font->Size=10;
        EditMotorPos[i]->ReadOnly=true;
    }
    fGridBuilt=true;
}
//---------------------------------------------------------------------------
void TfHome::ShowLed(int index, eHomeLedColor attr)
{
    if(index<0 || index>=HOME_MOTOR_MAX)
        return;
    if(LedPtr[index]==NULL)
        return;

    if(attr==eHomeUnuse)
    {
        LedPtr[index]->Value=false;
    }
    else
    {
        if(attr==eHomeOk)
            LedPtr[index]->TrueColor=clLime;
        else if(attr==eHomeError)
            LedPtr[index]->TrueColor=clRed;
        else if(attr==eHomeBusy)
            LedPtr[index]->TrueColor=clYellow;
        LedPtr[index]->Value=true;
    }
}
//---------------------------------------------------------------------------
void TfHome::ShowMotorHomePos(int i)
{
    if(i<0 || i>=HOME_MOTOR_MAX)
        return;
    if(HSys.MotPtr==NULL || HSys.MotPtr[i]==NULL)
        return;

    if(HSys.MotPtr[i]->GetEnable())
    {
        EditMotorPos[i]->Text=HSys.MotPtr[i]->ReadPos();
        if(HSys.MotPtr[i]->bHomeFlag)
            ShowLed(i, eHomeOk);
        else
            ShowLed(i, eHomeBusy);
    }
    else
    {
        EditMotorPos[i]->Text=0;
        ShowLed(i, eHomeUnuse);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfHome::FormShow(TObject *Sender)
{
    BuildMotorGrid();
    Top=0;
    fShow=true;
    fSeenStart=false;
}
//---------------------------------------------------------------------------
void __fastcall TfHome::FormClose(TObject *Sender, TCloseAction &Action)
{
    iHomeStep=1;
    fShow=false;
    bHomePowerCycling=false;
}
//---------------------------------------------------------------------------
// Abort Home: stop all motors and close. fAllMotorHome stays false so the
// machine still requires a successful home before running.
//---------------------------------------------------------------------------
void __fastcall TfHome::SpeedButton1Click(TObject *Sender)
{
    HSys.StopAllMotor();
    fAllMotorHome=false;
    bHomePowerCycling=false;
    SoftStop=true;
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TfHome::ScanKey()
{
    //AI(HT160S-Maintainer) 20260619 : HT172 uhome.cpp TfHome::ScanKey port. HT160
    //sensors carry no Tag, so read SnFKPause/SnRKPause directly and rising-edge
    //latch so a held key fires once. Physical PAUSE aborts the HOME monitor via
    //the existing Abort-Home handler (StopAllMotor + Close).
    static bool bWasPause=false;
    bool bPause=HSys.Sen.SnFKPause.IsOn() || HSys.Sen.SnRKPause.IsOn();
    if(bPause && bWasPause==false)
        SpeedButton1Click(this);
    bWasPause=bPause;
}
//---------------------------------------------------------------------------
void __fastcall TfHome::Timer1Timer(TObject *Sender)
{
    int i;
    if(fShow==false)
        return;

    //AI(HT160S-Maintainer) 20260619 : physical PAUSE (front or rear) aborts the
    //HOME monitor (HT172 TfHome::ScanKey port). May Close() the form, so re-check
    //fShow before touching the LED/position widgets below.
    ScanKey();
    if(fShow==false)
        return;

    for(i=0;i<HSys.iTotalMotor && i<HOME_MOTOR_MAX;i++)
        ShowMotorHomePos(i);

    //AI(HT160S-Maintainer) 20260619 : display-only now (HT172 TfHome::Timer1Timer
    //parity). The window close/abort DECISION moved to the kernel home-lifecycle
    //owner (csystem ProcessHomeLifecycle), which runs every tick even when
    //SystemStart==false; this timer only refreshes LEDs/positions and latches
    //fSeenStart for that owner to read.
    if(HSys.Sys.SystemStart)
        fSeenStart=true;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260619 : View teardown hooks called by the kernel home-
//lifecycle owner (csystem ProcessHomeLifecycle). The DECISION of WHEN to close
//(normal completion / SystemStart-drop abort) lives in the kernel so Timer1Timer
//is display-only (HT172 parity); the View still physically owns the window Close()
//and the completion message. IsShown()/SeenStart() expose the private fShow/
//fSeenStart latches to that kernel owner (read-only).
bool TfHome::IsShown() const
{
    return fShow;
}
//---------------------------------------------------------------------------
bool TfHome::SeenStart() const
{
    return fSeenStart;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260624 : latch "a HOME start was commanded" at the moment
//the start paths set SystemStart=true (sbHome1Click / START-on-unhomed), instead of
//waiting for the 100ms monitor Timer1 to sample SystemStart==true. A safety fault can
//drop SystemStart inside that 100ms window, so Timer1 never sees true, fSeenStart stays
//false, and the kernel SeenStart-guarded abort (ProcessHomeLifecycle) never closes the
//monitor -> it sits "shown but never running, never closing". Latching here closes it
//via the existing guarded kernel path (fAllMotorHome-first, bHomePowerCycling-skip,
//MachineHomeAbort). Timer1 own latch stays a backup; the timer is NOT a closer.
void TfHome::MarkSeenStart()
{
    fSeenStart=true;
}
//---------------------------------------------------------------------------
void TfHome::RequestClose()
{
    if(fShow==false)
        return;
    Close();
}
//---------------------------------------------------------------------------
void TfHome::RequestCloseFinished()
{
    if(fShow==false)
        return;
    lstHomeMsg->Items->Insert(0, "Home finished.");
    Close();
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260612 : true when a motor can be treated as already homed
//and skipped during a full-machine home. ONLY servo axes qualify : a servo retains its
//absolute position, so once homed (bHomeFlag) and currently free of any drive alarm
//(Led[iAlarmLed]==false) it does not need to physically re-home. A live alarm means the
//position is no longer trustworthy, so the axis must re-home. STEPPER axes always return
//false : with no position memory they must re-home on every full-machine home (this is
//why the 4 sucker Z and TrayArm Z, which must rise on every home, are never skipped).
//Led[] is refreshed each cycle by ScanAllMotorStatus() in DoSystem() (runs before
//ProcessMotion/ProcessMotorHome in MainProc), and that same scan clears bHomeFlag on a
//live servo alarm, so an alarmed servo can never satisfy this test.
static bool IsHomedServoSkippable(TTrayMotor *m)
{
	if(m==NULL)
		return false;
	return (m->bIsServoMotor && m->bHomeFlag && (m->Led[iAlarmLed]==false));
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260618 : SIMULATION-ONLY home trace. Writes a fresh
//HomeTrace.log (EXE folder) each home round so the full-machine HOME sequence
//can be reviewed offline. Compiled ONLY under SOFT_SIMULATE -> ZERO footprint on
//a production build. Each line is flushed to disk immediately so a hang mid-round
//still leaves a partial trace. bReset=true truncates (called once at round start,
//case 1) so the file never grows across rounds.
#ifdef SOFT_SIMULATE
static void SimHomeTrace(AnsiString line, bool bReset)
{
	static TStringList *pTrace=NULL;
	if(pTrace==NULL)
		pTrace=new TStringList();
	if(bReset)
		pTrace->Clear();
	pTrace->Add(FormatDateTime("hh:nn:ss:zzz", Now())+"  "+line);
	try
	{
		pTrace->SaveToFile(ExtractFilePath(Application->ExeName)+"HomeTrace.log");
	}
	catch(...)
	{
	}
}
#endif
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260602 : full-machine motor home engine (HT172 0420
//ProcessMotorHome equivalent, rewritten non-FSM / no FSMRunner). Drives every
//enabled axis home in a safe order: raise the TrayArm Z cylinder and open the
//clamps FIRST so the TrayArm head clears any tray below, then batch-home the 4
//sucker Z axes, and only after the TrayArm Z is confirmed up batch-home all XY
//axes (TrayArm X included). Mirrors same-machine V300A DoTrayXHome case 1000
//where Z-up = C_TrayArmZ_Up.On() verified by IsOn()==true (NOT Pop, which would
//drop Z and crash into the tray below). Non-blocking: returns false while busy.
//AI(ht160s-home-resume-w3b) 20260711 : carriage table for PARK & RE-ACQUIRE.
//idx 0/1 = Loader1/2, idx 2..7 = Auto1..6. Same clamp semantics on every carriage :
//PushTray = front stopper, LeanOnTray = rear hook (owner order : release rear then
//front, engage front then rear). idx 8=Empty, 9=Color (W3b-2) : their carry gate is
//the clamp out-bits (their VMot fHasTray means a tray at the REAR SEAT, not on the
//car) and a mid-haul park must finish the interrupted transfer to the module-owned
//haul target (GetHomeHaulTargetY) before releasing there.
static void HomeParkCarriage(int idx, TTrayMotor **Mot, TTrayMotor **VMot,
                             TMyCylinder **Front, TMyCylinder **Rear, AnsiString *Name)
{
	*Mot=NULL; *VMot=NULL; *Front=NULL; *Rear=NULL; *Name="";
	switch(idx)
	{
		case 0: *Mot=HSys.Mot.MLoaderY_1; *VMot=HSys.VMot.MMLoaderY_1; *Front=&HSys.Cyn.C_Loader1_PushTray; *Rear=&HSys.Cyn.C_Loader1_LeanOnTray; *Name="Loader1"; break;
		case 1: *Mot=HSys.Mot.MLoaderY_2; *VMot=HSys.VMot.MMLoaderY_2; *Front=&HSys.Cyn.C_Loader2_PushTray; *Rear=&HSys.Cyn.C_Loader2_LeanOnTray; *Name="Loader2"; break;
		case 2: *Mot=HSys.Mot.MAutoY_1; *VMot=HSys.VMot.MMAutoY_1; *Front=&HSys.Cyn.C_Auto1_PushTray; *Rear=&HSys.Cyn.C_Auto1_LeanOnTray; *Name="Auto1"; break;
		case 3: *Mot=HSys.Mot.MAutoY_2; *VMot=HSys.VMot.MMAutoY_2; *Front=&HSys.Cyn.C_Auto2_PushTray; *Rear=&HSys.Cyn.C_Auto2_LeanOnTray; *Name="Auto2"; break;
		case 4: *Mot=HSys.Mot.MAutoY_3; *VMot=HSys.VMot.MMAutoY_3; *Front=&HSys.Cyn.C_Auto3_PushTray; *Rear=&HSys.Cyn.C_Auto3_LeanOnTray; *Name="Auto3"; break;
		case 5: *Mot=HSys.Mot.MAutoY_4; *VMot=HSys.VMot.MMAutoY_4; *Front=&HSys.Cyn.C_Auto4_PushTray; *Rear=&HSys.Cyn.C_Auto4_LeanOnTray; *Name="Auto4"; break;
		case 6: *Mot=HSys.Mot.MAutoY_5; *VMot=HSys.VMot.MMAutoY_5; *Front=&HSys.Cyn.C_Auto5_PushTray; *Rear=&HSys.Cyn.C_Auto5_LeanOnTray; *Name="Auto5"; break;
		case 7: *Mot=HSys.Mot.MAutoY_6; *VMot=HSys.VMot.MMAutoY_6; *Front=&HSys.Cyn.C_Auto6_PushTray; *Rear=&HSys.Cyn.C_Auto6_LeanOnTray; *Name="Auto6"; break;
		case 8: *Mot=HSys.Mot.MEmptyY; *VMot=HSys.VMot.MMEmptyY; *Front=&HSys.Cyn.C_Empty_PushTray; *Rear=&HSys.Cyn.C_Empty_LeanOnTray; *Name="Empty"; break;
		case 9: *Mot=HSys.Mot.MColorY; *VMot=HSys.VMot.MMColorY; *Front=&HSys.Cyn.C_Color_PushTray; *Rear=&HSys.Cyn.C_Color_LeanOnTray; *Name="Color"; break;
	}
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260612 : "home only the not-yet-homed". Steppers (no position
//memory) always re-home; an already-homed, non-alarmed servo keeps its position and is
//skipped (see IsHomedServoSkippable). The TrayArm-Z-up + sucker-Z home safety actions
//are unaffected because those axes/cylinders are steppers/cylinders, never skipped.
bool TfHome::ProcessMotorHome()
{
//#ifdef SOFT_SIMULATE
//	return true;
//#else
	AnsiString sErr="";
    int &iMotorHomeTask=iHomeStep;
	static HTimer HomePowerTimer;
	//AI(ht160s-home-resume-w3b) 20260711 : carriage PARK & RE-ACQUIRE state (five-step
	//HOME material protocol steps 2+4). W3a=Loader cars, W3b=+Auto1-6 working trays.
	//Snapshot at case 1 (per-axis alarm still visible there, A4), clamps released at
	//case 50/60, re-acquire at case 300 after all axes homed.
	static const int HOME_PARK_N=10;
	static bool bCarParked[10]={false,false,false,false,false,false,false,false,false,false};
	static int  iCarParkPos[10]={0,0,0,0,0,0,0,0,0,0};
	//W3b-2 : haul target per parked carriage (-1 = plain re-acquire at the park spot;
	//>=0 = after re-clamping, FINISH the interrupted Empty/Color transfer to this Y and
	//release there with the module order Pop PushTray -> Pop LeanOnTray).
	static int  iCarHaulTarget[10]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	//W3b-2 : snapshot-time carry that could NOT be parked (A4 alarm / no haul target) ->
	//case-200 operator-removal fallback keys off this, not off fHasTray (Empty/Color
	//fHasTray = rear-seat tray, must never be cleared by the fallback).
	static bool bCarUnparkedCarry[10]={false,false,false,false,false,false,false,false,false,false};
	static int  iReacqCar=0;
	static int  iReacqStep=0;
	//AI(ht160s-home-resume-drain) 20260711 : W2 drain stage state. Phase 0=arm, 1=pump,
	//2=done/skipped. Reset on every new round (see the iPrevHomeStep edge below).
	static int iHomeDrainPhase=2;
	static int iPrevHomeStep=-999;
	static HTimer HomeDrainTimer;
#ifdef SOFT_SIMULATE
	//Trace gate: log only on the FIRST entry of each case so a case that polls
	//(returns false for many cycles) is written ONCE, not every tick. The clean
	//per-round sequence becomes 1 -> 2 -> 50 -> 60 -> 100 -> 200 -> DONE.
	static int iSimLastStep=-999;
	bool bSimNewStep=(iMotorHomeTask!=iSimLastStep);
	iSimLastStep=iMotorHomeTask;
#endif
	//AI(ht160s-home-resume-drain) 20260711 : new-round edge (ArmMotorHome re-enters at
	//step 1 from a non-1 step) -> re-arm the drain stage. A mid-drain abort re-entering
	//at 1->1 keeps the running phase/timer (drain simply continues or times out).
	if(iMotorHomeTask==1 && iPrevHomeStep!=1)
		iHomeDrainPhase=0;
	iPrevHomeStep=iMotorHomeTask;
	switch(iMotorHomeTask)
	{
		case 1:
		{
#ifdef SOFT_SIMULATE
			if(bSimNewStep)
				SimHomeTrace("==== HOME ROUND START ====  case1: power-cycle gate (SIM forces skip) -> case2", true);
#endif
			//AI(ht160s-home-resume-drain) 20260711 : W2 drain stage (five-step protocol step 1,
			//spec docs/plan/home-resume-drain-spec.md). BEFORE any power cycle / motor command,
			//pump the in-flight pure-cylinder/data ladders to their phase boundary in the same
			//scan (cross-module interlocks self-resolve) : Empty/Color/Loader destackers,
			//TrayArm grab/deposit completion (D4), Auto 6000/7000 commit stand-in (AF-2), and
			//the SP-1 vacuum reconciliation (Z still down -> vacuum off + blow, D2). Cause gate
			//per D3 : only an air-pressure alarm skips the drain (EMG cuts motor power, not
			//air). D1 : destacker hooks are frozen at stations with a non-IDLE AMR handshake.
			//On timeout the round proceeds - PARK/removal are the fallback for whatever did
			//not converge.
			if(iHomeDrainPhase==0)
			{
				if(IsAirCheck())
				{
					RecordProcess("Home: drain skipped (air-pressure alarm)");
					iHomeDrainPhase=2;
				}
				else
				{
					HomeDrainTimer.Clear();
					HomeDrainTimer.SetMS(GeneralSetting.iHomeDrainTimeoutSec*1000);
					HomeDrainTimer.On();
					RecordProcess("Home: drain stage start");
					iHomeDrainPhase=1;
				}
			}
			if(iHomeDrainPhase==1)
			{
				bool bAllDrained=true;
				if(LoaderModule!=NULL && AgvCoord.Handshake[0]==AGV_IDLE &&
				   LoaderModule->HomeDrainTick()==false)
					bAllDrained=false;
				if(EmptyModule!=NULL && AgvCoord.Handshake[1]==AGV_IDLE &&
				   EmptyModule->HomeDrainTick()==false)
					bAllDrained=false;
				if(ColorModule!=NULL && AgvCoord.Handshake[2]==AGV_IDLE &&
				   ColorModule->HomeDrainTick()==false)
					bAllDrained=false;
				if(AutoModule!=NULL && AutoModule->HomeDrainTick()==false)
					bAllDrained=false;
				if(TrayArmModule!=NULL && TrayArmModule->HomeDrainTick()==false)
					bAllDrained=false;
				if(SortArmModule!=NULL && SortArmModule->HomeDrainTick()==false)
					bAllDrained=false;
				if(bAllDrained)
				{
					RecordProcess("Home: drain stage done");
					iHomeDrainPhase=2;
				}
				else if(HomeDrainTimer.Off())
				{
					RecordProcess("Home: drain stage TIMEOUT - fallback to park/removal");
					iHomeDrainPhase=2;
				}
				else
					return false;
			}
			//AI(ht160s-home-resume-w3b2) 20260711 : PARK snapshot (protocol step 2) for all
			//park-capable carriages (Loader1/2 + Auto1-6 + Empty + Color). Taken at case 1 =
			//the single tick per round BEFORE any power cycle, so a live per-axis servo alarm
			//is still visible (A4 : an alarmed axis' position is untrusted -> no park, the
			//carriage falls into the case-200 operator-removal fallback). Loader/Auto carry =
			//VMot fHasTray; Empty/Color carry = clamp out-bits (mid-haul grip; their fHasTray
			//is the rear-seat tray) and additionally needs a module haul target so the
			//re-acquire can FINISH the interrupted transfer (EF-2). Case 50/60 release every
			//clamp unconditionally (A1 : both Off -> Y motion does not drag the parked tray).
			for(int pk=0; pk<HOME_PARK_N; pk++)
			{
				TTrayMotor *PMot; TTrayMotor *PV; TMyCylinder *PF; TMyCylinder *PR; AnsiString PName;
				HomeParkCarriage(pk, &PMot, &PV, &PF, &PR, &PName);
				//aborted-round guard : a still-pending park means the carriage may already have
				//moved (tray left at the OLD position) -- keep the latched position/target, never
				//re-snapshot from the current (possibly homed) encoder value.
				if(bCarParked[pk])
				{
					if(pk>=8 || (PV!=NULL && PV->fHasTray))
						continue;
				}
				bCarParked[pk]=false;
				iCarParkPos[pk]=0;
				iCarHaulTarget[pk]=-1;
				bCarUnparkedCarry[pk]=false;
				if(PMot==NULL || PV==NULL || PF==NULL || PR==NULL)
					continue;
				bool bCarry=false;
				int iHaul=-1;
				if(pk>=8)
				{
					if(PF->GetOutBit() || PR->GetOutBit())
						bCarry=true;
					if(pk==8 && EmptyModule!=NULL)
						iHaul=EmptyModule->GetHomeHaulTargetY();
					if(pk==9 && ColorModule!=NULL)
						iHaul=ColorModule->GetHomeHaulTargetY();
				}
				else
					bCarry=PV->fHasTray;
				if(bCarry==false)
					continue;
				if(PMot->GetEnable()==false || PMot->bIsServoMotor==false ||
				   PMot->Led[iAlarmLed] || (pk>=8 && iHaul<0))
				{
					bCarUnparkedCarry[pk]=true;
					RecordProcess("Home: "+PName+" carries a tray but cannot be parked (alarm/no-target)");
					continue;
				}
				iCarParkPos[pk]=PMot->ReadEncoderPos();
				iCarHaulTarget[pk]=(pk>=8)?iHaul:-1;
				bCarParked[pk]=true;
				RecordProcess("Home: PARK "+PName+" tray at Y="+IntToStr(iCarParkPos[pk]));
			}
			//AI(HT160S-Maintainer) 20260616 : power-cycle gate (compromise port
			//of the old-160 home Off->On). A latched servo-amp alarm cannot be
			//cleared in software (MC88X1 SetServoOn is a no-op), so the ONLY way
			//to re-energize and clear it is a physical SwMotorRelay Off->On. We
			//run that ONLY when motor power was lost OR an enabled servo is
			//alarmed; a clean operator HOME (power on, no alarm) skips it and
			//keeps the fast selective home. SOFT_SIMULATE always skips.
			bool bNeedPowerCycle=false;
			if(bMotorPowerState==false)
				bNeedPowerCycle=true;
			if(HSys.MotPtr!=NULL)
			{
				for(int i=0; i<HSys.iTotalMotor; i++)
				{
					TTrayMotor *m=HSys.MotPtr[i];
					if(m==NULL || m->GetEnable()==false)
						continue;
					if(m->bIsServoMotor && m->ReadServoAlarmOn() && m->Led[iAlarmLed])
					{
						bNeedPowerCycle=true;
						break;
					}
				}
			}
#ifdef SOFT_SIMULATE
			bNeedPowerCycle=false;
#endif
			if(bNeedPowerCycle)
			{
				if(lstHomeMsg!=NULL)
					lstHomeMsg->Items->Insert(0, "Motor power cycle (servo alarm recovery)....");
				RecordProcess("Home: motor power-cycle to clear latched servo alarm");
				bHomePowerCycling=true;
				AllBreakLock();
				HSys.Sw.SwMotorRelay.Off();
				bMotorPowerState=false;
				bMotorHomePowerOn=false;
				HomePowerTimer.Set(50);
				HomePowerTimer.On();
				iMotorHomeTask=10;
				return false;
			}
			iMotorHomeTask=2;
			return false;
		}
		case 10:
#ifdef SOFT_SIMULATE
			if(bSimNewStep)
				SimHomeTrace("case10: servo discharge wait -> re-energize motor power, servo-on -> case20", false);
#endif
			//Servo discharge complete -> re-energize motor power and re-assert
			//servo-on for every alarm-monitored axis, then settle.
			if(HomePowerTimer.Off())
			{
				HSys.Sw.SwMotorRelay.On();
				bMotorPowerState=true;
				if(HSys.MotPtr!=NULL)
				{
					for(int i=0; i<HSys.iTotalMotor; i++)
					{
						if(HSys.MotPtr[i]!=NULL && HSys.MotPtr[i]->ReadServoAlarmOn())
							HSys.MotPtr[i]->ServoOnOff(true);
					}
				}
				HomePowerTimer.Set(30);
				HomePowerTimer.On();
				iMotorHomeTask=20;
			}
			return false;
		case 20:
#ifdef SOFT_SIMULATE
			if(bSimNewStep)
				SimHomeTrace("case20: servo-on settle -> release brakes, end power-cycle -> case2", false);
#endif
			//Servo-on settle complete -> release brakes, end the power-cycle and
			//fall into the normal home (the previously-alarmed axis re-homes
			//because ScanAllMotorStatus cleared its bHomeFlag).
			//AI(HT160S-Maintainer) 20260624 : settle on the HomePowerTimer alone (HT172 uhome.cpp
			//ResetOKDeleyTime parity). The motor-power-off stop in ScanSystemSenser is now SKIPPED
			//while the HOME monitor is shown (see csystem.cpp), so it no longer sets MotorPowerOnDelay
			//during a home power-cycle -- the old && MotorPowerOnDelay==0 wait is gone. case 10s
			//Set(30)=3s servo-on settle (after the case 1 Set(50)=5s discharge) lets the A6 amps
			//re-energize before case 2 commands motion, and SystemStart now stays true throughout
			//so the SystemStart-gated home engine keeps stepping (no freeze, no spurious alarm).
			if(HomePowerTimer.Off())
			{
				AllBreakFree();
				bMotorHomePowerOn=true;
				bHomePowerCycling=false;
				if(lstHomeMsg!=NULL)
					lstHomeMsg->Items->Insert(0, "Motor power restored.");
				//AI 20260619 : the motor relay cuts MAIN motor power only, NOT the A6 drive
				//CONTROL power (L1C/L2C), so a latched drive alarm can survive this power-cycle
				//(confirmed in the field: only a MAIN-breaker cycle clears it). If an enabled
				//servo STILL reads its ALM line (and is not merely sitting on a CW/CCW limit),
				//do NOT loop the recovery silently -- case 1 would just power-cycle again every
				//round. Tell the operator exactly what to do and abort: SystemStart=false stops
				//ProcessMotion stepping the engine (no re-loop) and ProcessHomeLifecycle then
				//closes the monitor. Compiled out under SOFT_SIMULATE (ShowModal would hang the
				//--selftest-home; sim drivers never raise an alarm anyway).
#ifndef SOFT_SIMULATE
				{
					AnsiString sStuck="";
					if(HSys.MotPtr!=NULL)
					{
						for(int i=0; i<HSys.iTotalMotor; i++)
						{
							TTrayMotor *m=HSys.MotPtr[i];
							if(m==NULL || m->GetEnable()==false || m->bIsServoMotor==false)
								continue;
							m->ScanMotorStatus();
							if(m->ReadServoAlarmOn() && m->Led[iAlarmLed] &&
							   m->Led[iCwLed]==false && m->Led[iCcwLed]==false)
								sStuck+=m->NumberAlias+" ";
						}
					}
					if(sStuck!="")
					{
						RecordProcess(AnsiString("Home: servo alarm NOT cleared by motor power-cycle : ")+sStuck);
						if(lstHomeMsg!=NULL)
							lstHomeMsg->Items->Insert(0, AnsiString("Servo alarm NOT cleared : ")+sStuck+"- cycle MAIN power (breaker).");
						ShowMyMessage(Format(LangT("Servo alarm not cleared by motor power-cycle : %s. The motor relay does not cut the drive control power, so HOME cannot clear it. Please cycle the machine MAIN power (breaker) to reset the drive, or remove the drive fault, then HOME again."), ARRAYOFCONST((sStuck))));
						fAllMotorHome=false;
						HSys.Sys.SystemStart=false;
						iMotorHomeTask=1;
						return false;
					}
				}
#endif
				iMotorHomeTask=2;
			}
			return false;
		case 2:
			//Arm a fresh home: clear stale home flags so re-home actually runs.
			//AI(HT160S-Maintainer) 20260612 : but do NOT clear the flag of an
			//already-homed, non-alarmed servo : it keeps its position and is skipped
			//in the home batches below, so re-arming it would force a needless re-home.
			if(HSys.MotPtr!=NULL)
			{
				for(int i=0; i<HSys.iTotalMotor; i++)
				{
					if(HSys.MotPtr[i]==NULL)
						continue;
					if(IsHomedServoSkippable(HSys.MotPtr[i]))
						continue;
					HSys.MotPtr[i]->InitHomeTask();
				}
			}
			//Interference guard: raise TrayArm Z + open clamps before any X home.
			HSys.Cyn.C_TrayArmZ_Up.On();
			HSys.Cyn.C_TrayArmZ_Down.Off();
			//AI(HT160S-Maintainer) 20260612 : if the TrayArm is carrying a tray, keep the
			//clamps CLOSED through the home so the tray is not dropped. Z is raised first,
			//so the head+tray clears any tray below before X moves : opening the clamps is
			//only needed to release an empty/unknown head. The held tray is then placed on
			//resume (see TTrayArmModule::InitialFlag bKeepMaterial / DoTrayArm case 100).
			{
				//AI(ht160s-home) 20260707 : on-site -- an alarm mid-handoff can desync the fHasTray
				//software latch (HasTray()) from the physical grip, so HOME opened the TrayArm clamps
				//and DROPPED a held tray. Also honor the physical clamp On sensors : keep the clamps
				//CLOSED when EITHER enabled clamp On sensor reads On (conservative never-drop : the
				//clamps actuate only as a lockstep pair, aTrayArm DoLowerClampRaise, so a single
				//dead-off reed must not defeat this guard; the both-On AND form stays the high-
				//certainty signal used by the InitialFlag residue adopt -- two predicates on purpose).
				//Open ONLY when neither the software latch NOR any enabled sensor says held.
				//Real-machine only (sim keeps latch-only : no physical desync there).
				bool bTrayArmClampHeld=false;
#ifndef SOFT_SIMULATE
				if((HSys.Cyn.C_TrayArm_FrontClamp.OnSensor.Enable && HSys.Cyn.C_TrayArm_FrontClamp.OnSensor.IsOn()) ||
				   (HSys.Cyn.C_TrayArm_RearClamp.OnSensor.Enable  && HSys.Cyn.C_TrayArm_RearClamp.OnSensor.IsOn()))
					bTrayArmClampHeld=true;
#endif
				if((TrayArmModule==NULL || TrayArmModule->HasTray()==false) && bTrayArmClampHeld==false)
				{
					HSys.Cyn.C_TrayArm_FrontClamp.Off();
					HSys.Cyn.C_TrayArm_RearClamp.Off();
				}
			}
#ifdef SOFT_SIMULATE
			if(bSimNewStep)
			{
				AnsiString sRun="", sSkipServo="", sDisabled="";
				if(HSys.MotPtr!=NULL)
				{
					for(int i=0; i<HSys.iTotalMotor; i++)
					{
						TTrayMotor *m=HSys.MotPtr[i];
						if(m==NULL)
							continue;
						//Match the home batch gate (case100/200): disabled axes are
						//skipped (never run), already-homed servos are skipped (keep
						//position), everything else physically re-homes this round.
						if(m->GetEnable()==false)
							sDisabled+=m->NumberAlias+" ";
						else if(IsHomedServoSkippable(m))
							sSkipServo+=m->NumberAlias+" ";
						else
							sRun+=m->NumberAlias+" ";
					}
				}
				bool bHeldTray=(TrayArmModule!=NULL && TrayArmModule->HasTray());
				SimHomeTrace(AnsiString("case2: arm home. TrayArmZ UP, TrayArm clamps ")+
					(bHeldTray?"KEPT CLOSED (holding tray)":"OPENED")+" -> case50", false);
				SimHomeTrace("       WILL HOME (enabled): "+sRun, false);
				SimHomeTrace("       skipped(already-homed servo): "+sSkipServo, false);
				SimHomeTrace("       NOT run (disabled): "+sDisabled, false);
			}
#endif
			iMotorHomeTask=50;
			return false;
		case 50:
			//AI(HT160S-Maintainer) 20260617 : Loader held-tray SAFE HOME. The two Loader
			//cars (L=Loader1, R=Loader2) share one Loader-Y rail; homing while a car still
			//clamps a tray would drag that tray into the other car (cars colliding/jamming).
			//So release BOTH cars' tray clamps BEFORE any Loader-Y motion (case 200 homes
			//MLoaderY_1/_2). Release ORDER per machine spec : rear hook (LeanOnTray) FIRST,
			//then front stopper (PushTray). Pop() opens the clamp and returns true once the
			//Off sensor confirms (returns true immediately under SOFT_SIMULATE).
			{
#ifdef SOFT_SIMULATE
				if(bSimNewStep)
					SimHomeTrace("case50: release Loader rear hooks (Loader1/2 LeanOnTray) -> case60 when both off", false);
#endif
				//AI(ht160s-home-resume-w3b) 20260711 : release EVERY park-capable carriage's
				//rear hook (Loader1/2 + Auto1-6). A1 : with both clamps Off the Y home motion
				//does not drag a tray; an empty carriage's clamp cycling is harmless (the feed
				//ladders re-engage on the next grab).
				bool bAllRear=true;
				for(int pk=0; pk<HOME_PARK_N; pk++)
				{
					TTrayMotor *PMot; TTrayMotor *PV; TMyCylinder *PF; TMyCylinder *PR; AnsiString PName;
					HomeParkCarriage(pk, &PMot, &PV, &PF, &PR, &PName);
					if(PR==NULL)
						continue;
					if(PR->Pop()==false)
						bAllRear=false;
				}
				if(bAllRear)
					iMotorHomeTask=60;
			}
			return false;
		case 60:
			//Front stopper (PushTray) released only after the rear hook is fully open.
			{
#ifdef SOFT_SIMULATE
				if(bSimNewStep)
					SimHomeTrace("case60: release Loader front stoppers (Loader1/2 PushTray) -> case100 when both off", false);
#endif
				bool bAllFront=true;
				for(int pk=0; pk<HOME_PARK_N; pk++)
				{
					TTrayMotor *PMot; TTrayMotor *PV; TMyCylinder *PF; TMyCylinder *PR; AnsiString PName;
					HomeParkCarriage(pk, &PMot, &PV, &PF, &PR, &PName);
					if(PF==NULL)
						continue;
					if(PF->Pop()==false)
						bAllFront=false;
				}
				if(bAllFront)
					iMotorHomeTask=100;
			}
			return false;
		case 100:
		{
			//Hold TrayArm Z raised; batch-home the 4 sucker Z axes in parallel.
			HSys.Cyn.C_TrayArmZ_Up.On();
			HSys.Cyn.C_TrayArmZ_Down.Off();
			TTrayMotor *Z[4];
			Z[0]=HSys.Mot.MSuckZ_1; Z[1]=HSys.Mot.MSuckZ_2;
			Z[2]=HSys.Mot.MSuckZ_3; Z[3]=HSys.Mot.MSuckZ_4;
#ifdef SOFT_SIMULATE
			if(bSimNewStep)
			{
				AnsiString s="";
				for(int i=0;i<4;i++)
				{
					if(Z[i]==NULL)
						continue;
					s+=Z[i]->NumberAlias+(Z[i]->GetEnable()?"":"(disabled)")+" ";
				}
				SimHomeTrace("case100: batch-home 4 sucker Z [ "+s+"] -> case200 when all done", false);
			}
#endif
			bool bZHomed=true;
			for(int i=0; i<4; i++)
			{
				if(Z[i]==NULL || Z[i]->GetEnable()==false)
					continue;
				//AI(HT160S-Maintainer) 20260612 : sucker Z are steppers (never skipped),
				//but apply the same servo-skip guard generically for config safety.
				if(IsHomedServoSkippable(Z[i]))
					continue;
				//AI(HT160S-Maintainer) 20260619 : HT172 batch-home gate (uhome.cpp
				//PushStoreArmHome case100). TMyMotor::Home() returns true only on the
				//tick it completes, then resets its inner task to 1, so calling it again
				//after completion physically RE-HOMES the axis. Servos are protected by
				//IsHomedServoSkippable, but STEPPERS (these sucker Z) are not -> in a
				//multi-axis batch the axes that finish first get re-homed every tick
				//while waiting for the slowest, oscillating near home (0<->1) and the
				//batch never converges. Latch on bHomeFinish (set by Home(), cleared by
				//InitHomeTask in case 2) so each axis homes exactly once per round.
				if(Z[i]->bHomeFinish)
					continue;
				if(Z[i]->Home(sErr)==false)
					bZHomed=false;
			}
            if(bZHomed)
			{
				//AI(ht160s-home-resume-drain) 20260711 : SuckZ batch homed = Z back at safe ->
				//stop the SP-1 reconciliation blow now (owner D2 : blow off only after Z-safe).
				if(SortArmModule!=NULL)
					SortArmModule->HomeDrainBlowOff();
				//AI(ht160s-suck2-quad) 20260712 : braces restore the bZHomed gate - the blow-off
				//insert had stolen this if-body, advancing to case 200 before SuckZ finished
				//(selftest M14-M17 NOThomed; real-machine XY-vs-SuckZ collision hazard).
				iMotorHomeTask=200;
			}
			return false;
		}
		case 200:
		{
			//TrayArm Z confirmed up -> batch-home all XY axes (TrayArm X safe now).
            TTrayMotor *XY[16];
			XY[0]=HSys.Mot.MSortingArmX; XY[1]=HSys.Mot.MTrayArmX;
			XY[2]=HSys.Mot.MEmptyY;      XY[3]=HSys.Mot.MLoaderY_1;
			XY[4]=HSys.Mot.MLoaderY_2;   XY[5]=HSys.Mot.MAutoY_1;
			XY[6]=HSys.Mot.MAutoY_2;     XY[7]=HSys.Mot.MAutoY_3;
			XY[8]=HSys.Mot.MAutoY_4;     XY[9]=HSys.Mot.MAutoY_5;
			XY[10]=HSys.Mot.MAutoY_6;    XY[11]=HSys.Mot.MTopCCDX;
			XY[12]=HSys.Mot.MBottomCCDY; XY[13]=HSys.Mot.MPitchX;
            XY[14]=HSys.Mot.MColorY;     XY[15]=HSys.Mot.MTopCCDX_Color;
#ifdef SOFT_SIMULATE
			if(bSimNewStep)
				SimHomeTrace("case200: batch-home all XY axes (TrayArm X incl) -> DONE when all homed", false);
#endif
			bool bAllHomed=true;
            for(int i=0; i<16; i++)
			{
				if(XY[i]==NULL || XY[i]->GetEnable()==false)
					continue;
				//AI(HT160S-Maintainer) 20260612 : skip an already-homed, non-alarmed
				//servo (keeps absolute position); steppers + alarmed/never-homed servos
				//still physically re-home here.
				if(IsHomedServoSkippable(XY[i]))
					continue;
				//AI(HT160S-Maintainer) 20260619 : same HT172 batch-home gate as case100 --
				//a stepper XY axis would otherwise re-home every tick while waiting for
				//the rest of the batch (Home() resets its task after returning true).
				//Latch on bHomeFinish so each axis homes exactly once per round.
				if(XY[i]->bHomeFinish)
					continue;
				if(XY[i]->Home(sErr)==false)
					bAllHomed=false;
			}
			if(bAllHomed)
			{
				//AI(HT160S-Maintainer) 20260617 : Loader held-tray exception. The clamps were
				//opened during this home (case 50/60), so any tray a Loader car still logically
				//holds is now loose/unreferenced on the car. Require the operator to physically
				//remove ALL Loader trays, then clear both cars' tray identity + map (ClearTray
				//also drops fHasTray). The loader feed Task is re-initialized to "request a new
				//tray from scratch" by the InitialAllTask(true) the home caller runs right after
				//this returns true (see ProcessMotion Layer 2 in csystem.cpp).
				//AI(ht160s-home-resume-w3b2) 20260711 : forced removal is ONLY the fallback for a
				//carriage that was carrying but could NOT be parked (own servo alarm at case 1 A4,
				//or an Empty/Color carry with no resolvable haul target). Keyed on the snapshot
				//flag, NOT on fHasTray : Empty/Color fHasTray is the rear-seat tray and must not
				//be cleared here. ClearTray applies to Loader/Auto carriages only.
				bool bUnparked=false;
				for(int pk=0; pk<HOME_PARK_N; pk++)
				{
					if(bCarUnparkedCarry[pk])
					{
						TTrayMotor *PMot; TTrayMotor *PV; TMyCylinder *PF; TMyCylinder *PR; AnsiString PName;
						HomeParkCarriage(pk, &PMot, &PV, &PF, &PR, &PName);
						bUnparked=true;
						RecordProcess("Home: unparked "+PName+" tray -> operator removal");
					}
				}
				if(bUnparked)
				{
					ShowMyMessage(LangT("A tray carriage still holds a tray that could not be parked. Please MANUALLY REMOVE it, then press OK."));
					for(int pk2=0; pk2<HOME_PARK_N; pk2++)
					{
						if(bCarUnparkedCarry[pk2]==false)
							continue;
						bCarUnparkedCarry[pk2]=false;
						if(pk2>=8)
							continue;   //Empty/Color : rail tray removed by hand; rear-seat fHasTray untouched
						TTrayMotor *PMot; TTrayMotor *PV; TMyCylinder *PF; TMyCylinder *PR; AnsiString PName;
						HomeParkCarriage(pk2, &PMot, &PV, &PF, &PR, &PName);
						if(PV!=NULL && PV->fHasTray)
							PV->ClearTray();
					}
				}
				{
					bool bAnyParked=false;
					for(int pk3=0; pk3<HOME_PARK_N; pk3++)
					{
						if(bCarParked[pk3])
							bAnyParked=true;
					}
					if(bAnyParked)
					{
						RecordProcess("Home: axes homed, parked tray(s) -> re-acquire");
#ifdef SOFT_SIMULATE
						SimHomeTrace("case200: axes homed, parked tray(s) -> case300 re-acquire", false);
#endif
						iReacqCar=0;
						iReacqStep=0;
						iMotorHomeTask=300;
						return false;
					}
				}
#ifdef SOFT_SIMULATE
				//Final per-round summary: enumerate EVERY motor's home status so the
				//offline reviewer can confirm a complete round physically homed all axes.
				SimHomeTrace("---- ALL MOTOR HOME STATUS (round done) ----", false);
				if(HSys.MotPtr!=NULL)
				{
					for(int i=0; i<HSys.iTotalMotor; i++)
					{
						TTrayMotor *m=HSys.MotPtr[i];
						if(m==NULL)
							continue;
						AnsiString s=m->NumberAlias;
						s+=m->GetEnable()?" EN":" --";
						s+=m->bHomeFlag?" HOMED":" NOThomed";
						s+=m->bIsServoMotor?" servo":" stepper";
						if(m->Led[iAlarmLed])
							s+=" ALARM";
						SimHomeTrace("   "+s, false);
					}
				}
				SimHomeTrace("==== HOME ROUND DONE (case200 -> return true) ====", false);
#endif
				iMotorHomeTask=1;
				return true;
			}
			return false;
		}
		case 300:
		{
			//AI(ht160s-home-resume-w3b2) 20260711 : RE-ACQUIRE (protocol step 4), sequential
			//per parked carriage. Y returns to the memorized park position + offset, raises
			//the front stopper FIRST (clear of the tray edge so it cannot flip the tray up),
			//then the rear hook squeezes. Then : Loader/Auto creep back to the exact park
			//position (production geometry restored); Empty/Color instead FINISH the
			//interrupted haul to the module-owned target Y and release there with the module
			//order (Pop PushTray -> Pop LeanOnTray) -- the destination sensors then let the
			//restarted ladders re-derive with zero memory (Empty rear edge re-births; a Color
			//feed haul lands as an MES1426 leftover by design, never presented unscanned).
			//Safe: all axes are homed (TrayArm X parked, Z up per case 2), motors still at
			//home speed; Push()/Pop() self-alarm on reed timeout; MotorMove respects limits.
#ifdef SOFT_SIMULATE
			if(bSimNewStep)
				SimHomeTrace("case300: re-acquire parked tray(s) -> DONE when re-clamped/hauled", false);
#endif
			while(iReacqCar<HOME_PARK_N && bCarParked[iReacqCar]==false)
				iReacqCar++;
			if(iReacqCar>=HOME_PARK_N)
			{
				RecordProcess("Home: re-acquire done, all parked trays re-clamped/hauled");
#ifdef SOFT_SIMULATE
				SimHomeTrace("==== HOME ROUND DONE (case300 re-acquire done) ====", false);
#endif
				iMotorHomeTask=1;
				return true;
			}
			{
				TTrayMotor *PMot; TTrayMotor *PV; TMyCylinder *PF; TMyCylinder *PR; AnsiString PName;
				HomeParkCarriage(iReacqCar, &PMot, &PV, &PF, &PR, &PName);
				if(PMot==NULL || PF==NULL || PR==NULL)
				{
					bCarParked[iReacqCar]=false;
					return false;
				}
				switch(iReacqStep)
				{
					case 0:   //approach : park position + offset (stopper clear of the tray edge)
						//approach offset : GeneralSetting.iHomeReacquireOffsetCnt (1/100mm; sign per carriage
						//MUST be verified on-machine -- LD-4 : the Loader-Y pair runs opposite signs)
						if(PMot->MotorMove(iCarParkPos[iReacqCar]+GeneralSetting.iHomeReacquireOffsetCnt))
							iReacqStep=1;
						break;
					case 1:   //front stopper first (never raise it AT the tray edge)
						if(PF->Push())
							iReacqStep=2;
						break;
					case 2:   //rear hook squeezes the tray against the stopper
						if(PR->Push())
							iReacqStep=(iCarHaulTarget[iReacqCar]>=0)?4:3;
						break;
					case 3:   //Loader/Auto : creep back to the exact park position
						if(PMot->MotorMove(iCarParkPos[iReacqCar]))
						{
							RecordProcess("Home: RE-ACQUIRE "+PName+" tray at Y="+IntToStr(iCarParkPos[iReacqCar]));
							bCarParked[iReacqCar]=false;
							iReacqCar++;
							iReacqStep=0;
						}
						break;
					case 4:   //Empty/Color : finish the interrupted haul to the module target
						if(PMot->MotorMove(iCarHaulTarget[iReacqCar]))
							iReacqStep=5;
						break;
					case 5:   //release at the destination : module order Pop PushTray first
						if(PF->Pop())
							iReacqStep=6;
						break;
					case 6:   //...then Pop LeanOnTray; destination sensors take over on resume
						if(PR->Pop())
						{
							RecordProcess("Home: HAUL-FINISH "+PName+" tray to Y="+IntToStr(iCarHaulTarget[iReacqCar]));
							iCarHaulTarget[iReacqCar]=-1;
							bCarParked[iReacqCar]=false;
							iReacqCar++;
							iReacqStep=0;
						}
						break;
					default:
						iReacqStep=0;
						break;
				}
			}
			return false;
		}
	}
	iMotorHomeTask=1;
	return false;
}
//---------------------------------------------------------------------------
