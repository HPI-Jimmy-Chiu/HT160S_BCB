//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include "language.h"

#include "csystem.h"
#include "cmydef.h"
#include "iosetview.h"
#include "main.h"
#include "maintenance.h"
#include "setup.h"                       //AI(poka-yoke) 20260616 : fSetup->UpdateRunStateLock from UpdateRunControlFlag
#include "uruncontrol.h"
#include "aAuto1To6.h"
#include "aLoader.h"
#include "aSortArm.h"
#include "aColor.h"   //AI(ht160s-actuator-timer) 20260627 : ColorModule->PauseTimeoutTimers (ScanDelay freeze on pause)
#include "aEmpty.h"   //AI(ht160s-actuator-timer) 20260627 : EmptyModule->PauseTimeoutTimers (AmrFeedWaitTimer freeze)
#include "aTrayArm.h"   //AI(cleanout) 20260701 : TrayArmModule->IsCleanOutFinish() in CheckCleanOutFinish
#include "cprod.h"
#include "uHome.h"
#include "uspeed.h"                     //AI(HT160S-Maintainer) 20260602 : SetMotorSpeed / LoadMotorSpeedFromIni (Speed module port)
#include "note.h"                       //AI(HT160S-Maintainer) 20260603 : ShowSystemError for ProcessAlarm dispatch
#include "mymessbox.h"                   //AI 20260622 : ShowMyMessage for the real-machine (#ifndef SOFT_SIMULATE) servo-alarm home-flag-reset notice in ScanAllMotorStatus
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
static int GetSensorOffIndex(TMySensor *Sensor)
{
	if(Sensor==NULL || Sensor->Enable==false)
		return 0;
	if(Sensor->IsOff())
		return Sensor->Tag;
	return 0;
}
//---------------------------------------------------------------------------
static void SetMainStatus(AnsiString StatusText, TColor Color)
{
	if(fMain==NULL)
		return;
	if(fMain->palMainStatus!=NULL)
	{
		fMain->palMainStatus->Font->Color=Color;
		fMain->palMainStatus->Caption=StatusText;
	}
	if(fMain->palMainStatus_En!=NULL)
		fMain->palMainStatus_En->Caption=StatusText;
}
//---------------------------------------------------------------------------
static void UpdateRunControlFlag()
{
	RunControlSystemStart=HSys.Sys.SystemStart;
	//AI(poka-yoke) 20260616 : disable run-locked screens while the machine runs
	//  so the operator cannot open them. No stop-the-machine popup is used; the
	//  forms self-heal (re-enable) once the machine stops. Guarded by Visible so
	//  we only touch a form that is actually on screen.
	if(fMaintenance!=NULL && fMaintenance->Visible)
		fMaintenance->UpdateRunStateLock();
	if(fSetup!=NULL && fSetup->Visible)
		fSetup->UpdateRunStateLock();
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260603 : central alarm consumer. Modules raise alarms via
//Alarm->Set(code) (non-blocking, returns immediately); MainProc drains the queue
//here and shows each one through the alarm-code map. mapAlarmContext carries the
//caller Func/Case string so the note remark field (Memo1) shows what triggered it.
void ProcessAlarm()
{
	TComponent *Comp=NULL;
	int iCode=0;
	AnsiString sCtx;
	std::map<int, AnsiString>::iterator it;

	if(Alarm==NULL)
		return;

	while(PopUpAlarm(&Comp, iCode))
	{
		it=HSys.mapAlarmContext.find(iCode);
		sCtx=(it!=HSys.mapAlarmContext.end())?it->second:AnsiString("");
		ShowSystemError(AnsiString(iCode), K_RETRY, 0, sCtx);
	}
	ClearAllAlarm();
	HSys.mapAlarmContext.clear();
}
//---------------------------------------------------------------------------
#ifdef SOFT_SIMULATE
//AI(HT160S-Maintainer) 20260619 : --selftest-home headless self-test state (sim only).
bool g_SelfTestHome=false;
int  g_SelfTestExitCode=1;   // default 1 = did not complete; phase-2 sets 0(pass)/2(timeout)
#endif
//---------------------------------------------------------------------------
void MainProc()
{
	static bool bProgramStart=false;

	//AI(ht160s-initflow) 20260624 : whole-machine init guard (ref HT9045 InitialOK)
	if(InitialOK==false)
		return;

	//AI(HT160S-Maintainer) 20260616 : match HT172 MainProc - while the IO Set View
	//(manual IO test) is open, suspend the whole machine spin so DoSystem()/
	//DoSystemMessage() do not re-drive outputs (tower lamp SwTowerRed/Yellow/Green,
	//etc.) and override a manual IO test. Spin resumes when the view closes.
	if(fiosetview!=NULL && fiosetview->Visible)
	{
		UpdateRunControlFlag();
		return;
	}

#ifdef SOFT_SIMULATE
	//AI(HT160S-Maintainer) 20260619 : --selftest-home headless self-test (sim only).
	//Launched with --selftest-home, auto-trigger ONE full-machine home via the normal
	//operator path (Run_Home + SystemStart + ArmMotorHome) so ProcessMotion homes it
	//with IDENTICAL cases, then terminate with an exit code. Drives the offline
	//build->run->judge workflow without a GUI click. Inert unless g_SelfTestHome.
	if(g_SelfTestHome)
	{
		static int s_stPhase=0;
		static DWORD s_stT0=0;
		if(s_stPhase==0)
		{
			s_stT0=GetTickCount();
			s_stPhase=1;
		}
		else if(s_stPhase==1)
		{
			//Wait for program start, then let it settle one beat before triggering.
			if(bProgramStart && (int)(GetTickCount()-s_stT0)>=1500)
			{
				ChangeRunMode(Run_Home);
				HSys.Sys.SystemStart=true;
				ArmMotorHome();
				s_stT0=GetTickCount();
				s_stPhase=2;
			}
		}
		else if(s_stPhase==2)
		{
			//Home done (fAllMotorHome) -> pass; else bounded timeout -> fail.
			if(fAllMotorHome || (int)(GetTickCount()-s_stT0)>=30000)
			{
				g_SelfTestExitCode = fAllMotorHome ? 0 : 2;
				s_stPhase=3;
				if(Application!=NULL)
					Application->Terminate();
			}
		}
	}
#endif

	//AI(HT160S-Maintainer) 20260617 : consume the physical operator-panel keys
	//(Start/Home/Pause/One Cycle/Clean Out) before ProcessStartMode so a key press
	//takes effect this same cycle. Suspended with the rest of the spin while the IO
	//Set View is open (early return above). fMain owns the dispatch because it calls
	//the screen button handlers.
	if(fMain!=NULL)
		fMain->ScanPanelKeys();

	ProcessStartMode();
	DoSystem();
	ProcessRunStatus(bProgramStart);

	if(fMain!=NULL)
		fMain->SetSimulateScreenStatus();

	if(fMain!=NULL)
		fMain->ShowMotorInfo();

	if(fMain!=NULL)
		fMain->ShowUnloadAutoInfo();   //AI(ht160s-motion-view) 20260618 : Unload Auto1~6 Bin/Lot/ID/Cnt

	//AI(ht160s-lot-webapi) 20260612 : Stage 4 : drive any in-flight Lot WebAPI pull.
	// MainProc runs on the VCL main thread (TRunControl::Synchronize), so this is a
	// safe place to consume the async response and reproject the Lot list. No-op when idle.
	if(fMain!=NULL)
		fMain->PollLotDataWebApi();

	if(bProgramStart==false)
		bProgramStart=DoInitialProgramStart();
	else
	{
		//AI(general) 20260601 : ProcessMotion must run every cycle so homing
		//completion advances RunMode to Run_Normal; the old "else ProcessMotion()"
		//branch was dead code (DataModule1 is never NULL after startup), which
		//left RunMode stuck at Run_Home and the module dispatch never ran.
		ProcessMotion();
		//AI(HT160S-Maintainer) 20260602 : while a SortArm single Z-home is in
		//progress, hold the module engine so it cannot fight the re-home.
		//AI(HT160S-Maintainer) 20260624 : ONLY run the production action engine while the
		//machine is actually running. DoAllProcess() DecStopAllMotor()s every cycle when
		//SystemStart==false (its top-of-loop guard), which decel-stopped a Motor-Test or
		//Teach MANUAL jog the instant it started -> "jog only moves one step". HT172's
		//MainProc runs its production engine only via ProcessMotion (idle-safe; it has no
		//extra per-cycle DoAllProcess), so gate HT160's the same way and let idle / manual
		//screens own the motors. The real pause/EMG/safety paths still DecStopAllMotor on
		//the SystemStart falling edge, so production stop behaviour is unchanged.
		if(DataModule1!=NULL && bSortArmNeedHome==false && HSys.Sys.SystemStart)
			DataModule1->DoAllProcess();
	}

	//AI(HT160S-Maintainer) 20260603 : drain any alarms raised during this cycle's
	//module processing and show them through the central dispatch.
	ProcessAlarm();

	UpdateRunControlFlag();
}
//---------------------------------------------------------------------------
int IsEMGPressed()
{
#ifdef SOFT_SIMULATE
	return 0;
#else
	int Index=0;
	Index=GetSensorOffIndex(&HSys.Sen.SnEMG);
	if(Index>0) return Index;
	Index=GetSensorOffIndex(&HSys.Sen.SnEMG_1);
	if(Index>0) return Index;
	Index=GetSensorOffIndex(&HSys.Sen.SnEMG_2);
	if(Index>0) return Index;
	Index=GetSensorOffIndex(&HSys.Sen.SnEMG_3);
	if(Index>0) return Index;
	Index=GetSensorOffIndex(&HSys.Sen.SnEMG_4);
	return Index;
#endif
}
//---------------------------------------------------------------------------
bool IsSystemPowerOff()
{
#ifdef SOFT_SIMULATE
	return false;
#else
	if(MotorPowerOnDelay>0)
		return false;
	return (HSys.Sen.SnMotorPower.Enable && HSys.Sen.SnMotorPower.IsOff());
#endif
}
//---------------------------------------------------------------------------
int IsSafeDoorOpen()
{
#ifdef SOFT_SIMULATE
	return 0;
#else
	int Index=0;
	Index=GetSensorOffIndex(&HSys.Sen.SnSafeDoorFront);
	if(Index>0) return Index;
	Index=GetSensorOffIndex(&HSys.Sen.SnSafeDoorRight);
	if(Index>0) return Index;
	Index=GetSensorOffIndex(&HSys.Sen.SnSafeDoorLeft);
	if(Index>0) return Index;
	Index=GetSensorOffIndex(&HSys.Sen.SnSafeSlideDoorRight);
	if(Index>0) return Index;
	Index=GetSensorOffIndex(&HSys.Sen.SnSafeSlideDoorLeft);
	if(Index>0) return Index;
	Index=GetSensorOffIndex(&HSys.Sen.SnSafeAuto6);
	return Index;
#endif
}
//---------------------------------------------------------------------------
bool IsSafeLock()
{
#ifdef SOFT_SIMULATE
	return false;
#else
	return (HSys.Sen.SnSafeLock.Enable && HSys.Sen.SnSafeLock.IsOff());
#endif
}
//---------------------------------------------------------------------------
//AI 20260622 : ion-fan alarm debounce, re-aligned to HT172 csystem.cpp IsIonFanAlarm
//(TIME based, not cycle-count). A sensor must read OFF CONTINUOUSLY for a timeout
//before it counts as a real alarm; any OK read re-arms it. The earlier cycle-COUNT
//port (10 consecutive scans, "old HT160S CheckIonFan") did NOT translate: MainProc
//runs on the ~1ms TRunControl thread loop, so 10 scans is tens of ms -- far too short.
//The ion fans share the motor-relay power, so a HOME motor power-cycle restarts them
//and they take SECONDS to spin back up to the OK signal; a sub-second window false-
//trips an "Ion Fan Alarm" mid-HOME (machine kept homing, operator saw the fan fine in
//the IO Set View). HT172 uses a LONGER window while homing (fans spinning up) and a
//short one once homed. We also (a) skip the check while motor power is still settling
//(MotorPowerOnDelay>0 : fans just re-energized) and (b) re-arm after any scan gap
//>ION_FAN_SCAN_GAP_MS (the IO Set View and modal alarms suspend MainProc, so do not
//charge that suspended time against the fan). Counting happens ONCE per cycle here;
//IsIonFanAlarm() (GetTowerLightRunState + ScanSystemSenser) only reads the latched tag.
static const DWORD ION_FAN_TIMEOUT_HOME_MS=20000;  // homing : fans spinning up after the power-cycle
static const DWORD ION_FAN_TIMEOUT_RUN_MS = 5000;  // homed  : steady state
static const DWORD ION_FAN_SCAN_GAP_MS    = 1500;  // re-arm if the scan was suspended longer than this
static DWORD g_dwIonFanPowerOffStart=0;    // tick of the first OFF read (0 = OK / not counting)
static DWORD g_dwIonFanBalanceOffStart=0;
static DWORD g_dwIonFanLastScan=0;
static int   g_iIonFanAlarmTag=0;
//---------------------------------------------------------------------------
static void UpdateIonFanDebounce()
{
#ifdef SOFT_SIMULATE
	g_iIonFanAlarmTag=0;
	return;
#else
	DWORD dwNow=GetTickCount();
	if(dwNow==0)
		dwNow=1;   // 0 is the "OK" sentinel; keep a real tick out of it

	//Re-arm after a scan gap (IO Set View / a modal suspended MainProc): the fan was
	//not being watched, so start its OFF window fresh rather than charge it the gap.
	if(g_dwIonFanLastScan==0 || (dwNow-g_dwIonFanLastScan)>ION_FAN_SCAN_GAP_MS)
	{
		g_dwIonFanPowerOffStart=0;
		g_dwIonFanBalanceOffStart=0;
		g_dwIonFanLastScan=dwNow;
		g_iIonFanAlarmTag=0;
		return;
	}
	g_dwIonFanLastScan=dwNow;

	//Fans just re-energized (motor power settling): do not watch them yet.
	if(MotorPowerOnDelay>0)
	{
		g_dwIonFanPowerOffStart=0;
		g_dwIonFanBalanceOffStart=0;
		g_iIonFanAlarmTag=0;
		return;
	}

	//HT172 : 20s while homing (fAllMotorHome==false, fans spinning up), 5s once homed.
	DWORD dwTimeout=fAllMotorHome?ION_FAN_TIMEOUT_RUN_MS:ION_FAN_TIMEOUT_HOME_MS;

	int iPowerTag  =GetSensorOffIndex(&HSys.Sen.SnIonFan_Power);
	int iBalanceTag=GetSensorOffIndex(&HSys.Sen.SnIonFan_Balance);

	if(iPowerTag>0)
	{
		if(g_dwIonFanPowerOffStart==0)
			g_dwIonFanPowerOffStart=dwNow;
	}
	else
		g_dwIonFanPowerOffStart=0;

	if(iBalanceTag>0)
	{
		if(g_dwIonFanBalanceOffStart==0)
			g_dwIonFanBalanceOffStart=dwNow;
	}
	else
		g_dwIonFanBalanceOffStart=0;

	if(g_dwIonFanPowerOffStart!=0 && (dwNow-g_dwIonFanPowerOffStart)>=dwTimeout)
		g_iIonFanAlarmTag=iPowerTag;
	else if(g_dwIonFanBalanceOffStart!=0 && (dwNow-g_dwIonFanBalanceOffStart)>=dwTimeout)
		g_iIonFanAlarmTag=iBalanceTag;
	else
		g_iIonFanAlarmTag=0;
#endif
}
//---------------------------------------------------------------------------
int IsIonFanAlarm()
{
	//Pure read of the debounced/latched ion-fan tag (0 = none). Counting happens
	//once per cycle in UpdateIonFanDebounce() so the several per-cycle callers agree.
	return g_iIonFanAlarmTag;
}
//---------------------------------------------------------------------------
bool IsAirCheck()
{
#ifdef SOFT_SIMULATE
	return false;
#else
	return (HSys.Sen.SnAirIsEnough.Enable && HSys.Sen.SnAirIsEnough.IsOff());
#endif
}
//---------------------------------------------------------------------------
static bool GetTowerLightBlinkPhase()
{
	static bool BlinkPhase=false;
	static DWORD LastTick=0;
	DWORD NowTick=GetTickCount();

	if(LastTick==0)
		LastTick=NowTick;
	if(NowTick-LastTick>=300)
	{
		LastTick=NowTick;
		BlinkPhase=!BlinkPhase;
	}
	return BlinkPhase;
}
//---------------------------------------------------------------------------
static int GetTowerLightRunState()
{
	if(IsEMGPressed()>0 || IsSystemPowerOff() || IsSafeLock() ||
	   IsSafeDoorOpen()>0 || IsAirCheck() || IsIonFanAlarm()>0)
		return LED_ErrJam;

	if(HSys.Sys.SystemStart)
	{
		if(fAllMotorHome==false)
			return LED_Homeing;
		return LED_Running;
	}

	return LED_Pause;
}
//---------------------------------------------------------------------------
void AllBreakLock()
{
	HSys.Sw.SwServerON.Off();
}
//---------------------------------------------------------------------------
bool AllBreakFree()
{
	return true;
}
//---------------------------------------------------------------------------
bool CountMotorPowerDelay()
{
#ifdef SOFT_SIMULATE
	MotorPowerOnDelay=0;
	return true;
#else
	static DWORD LastTick=0;
	DWORD NowTick=GetTickCount();

	if(MotorPowerOnDelay>0 && HSys.Sen.SnMotorPower.IsOff()==false && IsEMGPressed()==0)
	{
		if(LastTick==0)
			LastTick=NowTick;
		if(NowTick-LastTick>=1000)
		{
			LastTick=NowTick;
			MotorPowerOnDelay--;
		}
		if(MotorPowerOnDelay<=0)
			AllBreakFree();
	}
	else if(MotorPowerOnDelay<=0)
	{
		LastTick=0;
	}
	return (MotorPowerOnDelay<=0);
#endif
}
//---------------------------------------------------------------------------
void ScanAllMotorStatus()
{
	if(HSys.MotPtr==NULL)
		return;

	for(int i=0; i<HSys.iTotalMotor; i++)
	{
		if(HSys.MotPtr[i]==NULL)
			continue;
		HSys.MotPtr[i]->ScanMotorStatus();
		//AI(HT160S-Maintainer) 20260616 : during a HOME power-cycle recovery the
		//servo is intentionally de-energized (reads alarm); do not force a
		//re-home / SystemStart=false off that expected transient. The cycle
		//clears the latched alarm and re-homes the axis itself.
		//AI(HT160S-Maintainer) 20260624 : also skip the alarm check during a HOME
		//(RunMode==Run_Home). Per request: do NOT check/warn a motor alarm BEFORE the
		//home power-cycle. HOME deliberately cuts motor power then restores it; a latched
		//servo/OT alarm present at home start is EXPECTED and will be cleared by that
		//cycle, so warning here would fire prematurely (and drop SystemStart, aborting
		//the home before it can recover). The home engine owns alarm handling: it power-
		//cycles, restores, waits, and only THEN checks + warns (uHome ProcessMotorHome
		//case 20). bHomePowerCycling already covers the cut-power window; this also covers
		//the home-start window before the cycle begins.
		if(bHomePowerCycling==false && HSys.Sys.RunMode!=Run_Home &&
		   HSys.MotPtr[i]->Led[iAlarmLed] && HSys.MotPtr[i]->GetEnable() &&
		   HSys.MotPtr[i]->ReadServoAlarmOn())
		{
			//AI 20260619 : HT172-style limit guard (HT172 csystem.cpp ScanAllMotorStatus
			//Led[iCwLed]||Led[iCcwLed] -> return). During a full-machine HOME an axis
			//routinely sits on a CW/CCW limit; the limit FORCES Led[iAlarmLed] (and the
			//A6 amp raises an OT alarm) -- EXPECTED here, NOT a fatal servo fault. Without
			//this guard HOME randomly drops SystemStart whenever an axis touches a limit,
			//closing the HOME monitor while motors are still moving. Skip (do not kill)
			//when on a limit; a real off-limit servo alarm still stops the machine.
			if(HSys.MotPtr[i]->Led[iCwLed] || HSys.MotPtr[i]->Led[iCcwLed])
				continue;
			//AI 20260619 : NEVER drop SystemStart silently on a real (off-limit) servo
			//alarm -- avoiding a silent stop is an iron rule. Aligned with HT172 csystem.cpp
			//ScanAllMotorStatus: if the machine was running, raise the motor-error Note via
			//ShowMotorError (modal; it also DecStopAllMotor + SystemStart=false +
			//ChangeRunMode(Run_Home)); if already stopped, just clear the home flag (and tell
			//the operator once if it had been homed). Gating the popup on SystemStart stops it
			//re-firing every scan cycle (the servo alarm latches until power-cycle / re-home).
			//ShowModal would hang the headless --selftest-home (no UI pump), so the operator
			//notifications are compiled out under SOFT_SIMULATE; sim drivers never alarm.
			bool bWasHomed=fAllMotorHome;
			HSys.MotPtr[i]->bHomeFlag=false;
			fAllMotorHome=false;
			if(HSys.Sys.SystemStart)
			{
#ifndef SOFT_SIMULATE
				int iRef=HSys.MotPtr[i]->GetErrorIndex();
				if(iRef==9)
					iRef=6;
				AnsiString S=AnsiString().sprintf("%d%03d%1d",(int)eMotorAlarm,i,iRef);
				ShowMotorError(S,"ScanAllMotorStatus");
#endif
				HSys.Sys.SystemStart=false;
			}
#ifndef SOFT_SIMULATE
			else if(bWasHomed)
			{
				ShowMyMessage(AnsiString().sprintf(LangT("Motor %s home flag reset (servo alarm).").c_str(),
				              HSys.MotPtr[i]->NumberAlias.c_str()));
			}
#else
			(void)bWasHomed;
#endif
		}
	}
}
//---------------------------------------------------------------------------
void CheckMotorPowerShutDown()
{
#ifdef SOFT_SIMULATE
	bMotorPowerState=true;
	MotorPowerOnDelay=0;
	return;
#else
	//AI(HT160S-Maintainer) 20260616 : a HOME power-cycle recovery owns the motor
	//relay until it completes; do not let the power-button logic fight it.
	if(bHomePowerCycling)
		return;

	//AI(HT160S-Maintainer) 20260616 : the panel Power On/Off buttons arrive over
	//the RS232 Pad link. Before the panel has exchanged a valid frame those
	//sensors hold stale defaults and must not drive the relay. Until the Pad has
	//talked, auto-energize the motor relay (per request, no interlock) so power
	//is up at software start; once bPadEverCommunicated is set we fall through to
	//the normal panel-button logic below.
	if(bPadEverCommunicated==false)
	{
		if(bMotorPowerState==false)
		{
			HSys.Sw.SwMotorRelay.On();
			MotorPowerOnDelay=SERVER_MOTOR_POWER_ON_DELAY;
			bMotorPowerState=true;
			bLampPowerOn=true;
			bLampPowerOff=false;
		}
		return;
	}

	//AI(HT160S-Maintainer) 20260617 : the panel Power On/Off keys are Pad keys with
	//no IO-card mapping, so TMySensor::Enable is false (Enable=IsValidIOData). The
	//old if(.Enable) guards therefore skipped IsOn() entirely - the Pad scan buffer
	//was never read and Power Off never cut the relay. Read unconditionally like
	//HT172 csystem.cpp: IsOn() returns the live Pad-key state for a Pad key and a
	//safe false for any unmapped non-Pad sensor (Input==NULL). bKeyPowerOffPressed
	//(HT172) arms Power Off only after the relay is actually on, so a stale Pad
	//default in the first cycle cannot trip it.
	static bool bKeyPowerOffPressed=false;
	bool bOn=false;
	bool bOff=false;

	bOn|=HSys.Sen.SnFKPowerOn.IsOn();
	bOn|=HSys.Sen.SnRKPowerOn.IsOn();
	bOff|=HSys.Sen.SnFKPowerOff.IsOn();
	bOff|=HSys.Sen.SnRKPowerOff.IsOn();

	if(bOff && bOn==false)
	{
		if(bKeyPowerOffPressed)
		{
			HSys.DecStopAllMotor();
			AllBreakLock();
			HSys.Sw.SwMotorRelay.Off();
			bMotorPowerState=false;
			HSys.Sys.SystemStart=false;
			fAllMotorHome=false;
			bLampPowerOn=false;
			bLampPowerOff=true;
		}
	}
	else if(bMotorPowerState==false && bOn)
	{
		HSys.Sw.SwMotorRelay.On();
		MotorPowerOnDelay=SERVER_MOTOR_POWER_ON_DELAY;
		bMotorPowerState=true;
		bLampPowerOn=true;
		bLampPowerOff=false;
	}

	bKeyPowerOffPressed=bMotorPowerState;
#endif
}
//---------------------------------------------------------------------------
void ScanSystemSenser()
{
	UpdateIonFanDebounce();
	int SafeDoor=IsSafeDoorOpen();
	int Emg=IsEMGPressed();
	int IonFan=IsIonFanAlarm();

	if(MotorPowerOnDelay==0)
	{
		if(Emg>0)
		{
			HSys.StopAllMotor();
			AllBreakLock();
			//AI 20260619 : do not stop silently -- align with HT172 csystem.cpp
			//ScanSystemSenser, which raises ShowSystemError before dropping SystemStart.
			//Guarded by SystemStart so a held EMG does not re-pop the Note every scan cycle
			//(SystemStart is already false on the next pass).
			if(HSys.Sys.SystemStart)
			{
				RecordProcess("MACHINE STOP by emergency-stop");
				ShowSystemError(AnsiString("Emergency Stop"), K_RETRY);
			}
			HSys.Sys.SystemStart=false;
			fAllMotorHome=false;
			MotorPowerOnDelay=SERVER_MOTOR_POWER_ON_DELAY;
		}
		//AI(HT160S-Maintainer) 20260624 : align with HT172 csystem.cpp ScanSystemSenser --
		//SKIP the motor-power-off stop while the HOME monitor is shown (HT172 guards this
		//same branch with fHome->fShow==false). A full-machine HOME deliberately power-cycles
		//the motor relay to clear latched servo alarms (uHome ProcessMotorHome case 1->10->20);
		//without this guard the cut-power window trips a spurious "Motor Power Off" alarm AND
		//drops SystemStart, which freezes the SystemStart-gated home engine (ProcessMotion
		//only steps ProcessMotorHome while SystemStart). EMG / safety-door / air / ion-fan
		//stops in the if(SystemStart) block below stay UN-guarded -- a real fault still stops.
		else if(IsSystemPowerOff() && (fHome==NULL || fHome->IsShown()==false))
		{
			HSys.StopAllMotor();
			AllBreakLock();
			if(HSys.Sys.SystemStart)
			{
				RecordProcess("MACHINE STOP by system-power-off");
				ShowSystemError(AnsiString("Motor Power Off"), K_RETRY);
			}
			HSys.Sys.SystemStart=false;
			fAllMotorHome=false;
			MotorPowerOnDelay=SERVER_MOTOR_POWER_ON_DELAY;
		}
	}

	if(HSys.Sys.SystemStart)
	{
		if(SafeDoor>0)
		{
			//AI 20260619 : align with HT172 ScanSystemSenser -- notify before stopping
			//(iron rule: never stop the running machine silently).
			HSys.StopAllMotor();
			RecordProcess("MACHINE STOP by safety-door");
			ShowSystemError(AnsiString("Safety Door Open"), K_RETRY);
			HSys.Sys.SystemStart=false;
		}
		else if(Emg>0)
		{
			HSys.StopAllMotor();
			AllBreakLock();
			RecordProcess("MACHINE STOP by emergency-stop");
			ShowSystemError(AnsiString("Emergency Stop"), K_RETRY);
			HSys.Sys.SystemStart=false;
			fAllMotorHome=false;
		}
		//AI(home-realign) 20260626 : suppress ion-fan + air while the HOME monitor is shown,
		//same rationale as the motor-power rungs (618/671) and HT172's fShow guard (HT172
		//applies fShow only to motor-power). HT160 gates the home engine on SystemStart, so a
		//false ion/air drop here freezes ProcessMotorHome. EMG/Safe-Door stay UNGUARDED (real
		//fault must stop). Interim shim until Step-6 iHome decoupling (docs/plan/home-realign-rootcause-and-fix-plan.md).
		else if(IonFan>0 && HSys.LastSet.iRealDummy==REALLY && (fHome==NULL || fHome->IsShown()==false))
		{
			HSys.DecStopAllMotor();
			RecordProcess("MACHINE PAUSE by ion-fan-alarm");
			ShowSystemError(AnsiString("Ion Fan Alarm"), K_RETRY);
			HSys.Sys.SystemStart=false;
		}
		else if(IsAirCheck() && (fHome==NULL || fHome->IsShown()==false))
		{
			HSys.DecStopAllMotor();
			RecordProcess("MACHINE PAUSE by air-pressure-low");
			ShowSystemError(AnsiString("Air Pressure Low"), K_RETRY);
			HSys.Sys.SystemStart=false;
		}
		//AI(HT160S-Maintainer) 20260624 : do not drop SystemStart for the motor-power-on
		//settle delay while the HOME monitor is shown. HT160 gates the home engine on
		//SystemStart (HT172 does not), so dropping it mid-HOME freezes ProcessMotorHome.
		//The home power-cycle owns the settle via uHome HomePowerTimer; let HOME keep running.
		else if(MotorPowerOnDelay>0 && (fHome==NULL || fHome->IsShown()==false))
		{
			HSys.Sys.SystemStart=false;
		}
	}
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
//AI(ht160s-maintainer) 20260617 : front-panel version of HT172 DoPanelLamp().
//Pushes machine run-state (bLamp* flags) onto the front operator-panel button
//LEDs over the Pad RS232 link (SwFK* -> fPadInterface). Runs only once the
//motor power is up and the power-on delay has elapsed, matching HT172
//ckernel.cpp. Rear (SwRK*) lamps are omitted: this machine is front-panel only.
static void DoPanelLamp()
{
	//AI(HT160S-Maintainer) 20260624 : the Power On/Off lamps MUST always track the real
	//relay state. The old code drove them only inside the power-up branch, so once the
	//relay dropped (panel Power Off, fault, or a HOME power-cycle) the else branch left
	//SwFKPowerOn at its last (lit) value -- the Power On lamp stayed lit while the motor
	//relay was actually off (no motor power). Drive both lamps every cycle straight from
	//bMotorPowerState (the relay command) so the lamp cannot lie; bLampPowerOn is not used
	//here because other relay-off paths (HOME power-cycle) drop bMotorPowerState without
	//updating that flag. (HT172 ckernel.cpp has the same branch shape but its panel LEDs
	//lose power together with the relay; HT160's panel LEDs are powered independently.)
	HSys.Sw.SwFKPowerOn.OnOff(bMotorPowerState);
	HSys.Sw.SwFKPowerOff.OnOff(!bMotorPowerState);

	if(bMotorPowerState && MotorPowerOnDelay==0)
	{
		HSys.Sw.SwFKPause.OnOff(bLampPause);
		HSys.Sw.SwFKStart.OnOff(bLampStart);
		HSys.Sw.SwFKRetry.OnOff(bLampRetry);
		HSys.Sw.SwFKSkip.OnOff(bLampSkip);
		HSys.Sw.SwFKTrayEnd.OnOff(bLampTrayEnd);
		HSys.Sw.SwFKTrayFeed.OnOff(bLampTrayFeed);
		HSys.Sw.SwFKCleanOut.OnOff(bLampCleanOut);
		HSys.Sw.SwFKOneCycle.OnOff(bLampOneCycle);
	}
	else
	{
		HSys.Sw.SwFKPause.Off();
		HSys.Sw.SwFKStart.Off();
		HSys.Sw.SwFKRetry.Off();
		HSys.Sw.SwFKSkip.Off();
		HSys.Sw.SwFKTrayEnd.Off();
		HSys.Sw.SwFKTrayFeed.Off();
		HSys.Sw.SwFKCleanOut.Off();
		HSys.Sw.SwFKOneCycle.Off();
	}
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260622 : map the tower-light run-state to its tsMaintTowerLight
//"Music Select" (RadioGroup2-7, one per state row). 0=mute, 1..4 -> Music1..4. Feeds the
//per-state buzzer driver in DoSystemMessage (HT172 ShowRunLed parity).
static int GetMaintenanceMusicSelect(int RunState)
{
	if(fMaintenance==NULL)
		return 0;
	switch(RunState)
	{
		case LED_Running: return (fMaintenance->RadioGroup2!=NULL)?fMaintenance->RadioGroup2->ItemIndex:0;
		case LED_ErrJam:  return (fMaintenance->RadioGroup3!=NULL)?fMaintenance->RadioGroup3->ItemIndex:0;
		case LED_Pause:   return (fMaintenance->RadioGroup4!=NULL)?fMaintenance->RadioGroup4->ItemIndex:0;
		case LED_Message: return (fMaintenance->RadioGroup5!=NULL)?fMaintenance->RadioGroup5->ItemIndex:0;
		case LED_Heating: return (fMaintenance->RadioGroup6!=NULL)?fMaintenance->RadioGroup6->ItemIndex:0;
		case LED_Homeing: return (fMaintenance->RadioGroup7!=NULL)?fMaintenance->RadioGroup7->ItemIndex:0;
	}
	return 0;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260622 : drive the 4 SwMusic outputs for one "Music Select"
//(0=mute, 1..4 -> SwMusic1..4): the selected line ON, the rest OFF. Shared by the per-scan
//DoSystemMessage buzzer driver and the dialog FormShow kicks (PlayMessageBuzzer /
//PlayAlarmBuzzer) so a modal alarm that suspends MainProc still sounds the moment it shows.
static void DriveSystemMusic(int MusicSel)
{
	int Base;
	int m;
	int idx;

	if(HSys.SwPtr==NULL)
		return;
	Base=HSys.Sw.SwMusic1.Tag;
	for(m=0; m<4; m++)
	{
		idx=Base+m;
		if(idx<0 || idx>=HSys.iTotalSwitch)
			continue;
		HSys.SwPtr[idx].OnOff(MusicSel!=0 && (MusicSel-1)==m);
	}
}
//---------------------------------------------------------------------------
void DoSystemMessage()
{
	int RunState;
	bool BlinkPhase;
	bool GreenOn;
	bool YellowOn;
	bool RedOn;

	RunState=GetTowerLightRunState();
	BlinkPhase=GetTowerLightBlinkPhase();
	GreenOn=GetTowerLightConfigOutput(RunState, 0, BlinkPhase);
	YellowOn=GetTowerLightConfigOutput(RunState, 1, BlinkPhase);
	RedOn=GetTowerLightConfigOutput(RunState, 2, BlinkPhase);

	if(fMain!=NULL)
	{
		if(fMain->ledGreen!=NULL)
			fMain->ledGreen->Value=GreenOn;
		if(fMain->ledYellow!=NULL)
			fMain->ledYellow->Value=YellowOn;
		if(fMain->ledRed!=NULL)
			fMain->ledRed->Value=RedOn;
	}

	HSys.Sw.SwTowerGreen.OnOff(GreenOn);
	HSys.Sw.SwTowerYellow.OnOff(YellowOn);
	HSys.Sw.SwTowerRed.OnOff(RedOn);

	//AI(HT160S-Maintainer) 20260622 : buzzer = a LITERAL transcription of HT172 ckernel.cpp
	//ShowRunLed (lines 83-148). HT172 derives ONE RunState from a DIALOG ladder and plays
	//RadioGroupPtr[RunState]'s "Music Select". Here the tower LAMPS keep HT160's live-sensor
	//RunState (set above); ONLY the buzzer uses the HT172 dialog ladder via BuzzState. Ladder
	//in HT172's exact order: alarm Note -> message box -> HOME monitor -> running -> pause.
	//(HT172 also has a Heating rung; HT160 has no heater, so it collapses into Running. HT160's
	//TfHome exposes IsShown() rather than HT172's fHome->fShow.) ErrJam is reached ONLY via a
	//Note, never a raw EMG/door/air/ion-fan read, so a safety sensor that trips with no alarm
	//dialog lights the red lamp but the buzzer stays silent. Music drive is also literal HT172:
	//play row 'is'(=MusicSel), and for ErrJam honour the OFF BUZZER acknowledge (HT172
	//bAlarmBuzzer==false -> Off; HT160 fNote->IsBuzzerOff()). The message-box rung is gated by
	//its Off Buzzer latch (HT172 bOffBuzzer; bMessageAlarm is never set true in HT172, omitted).
	//Suppress only while the Maintenance screen owns the buzzer alone (its sbMusic test buttons).
	{
		bool bMaintAlone=(fMaintenance!=NULL && fMaintenance->Visible &&
			(fNote==NULL || fNote->fShow==false) && (MyMessageBox==NULL || MyMessageBox->fShow==false));

		int BuzzState;
		if(fNote!=NULL && fNote->fShow)
			BuzzState=LED_ErrJam;
		else if(MyMessageBox!=NULL && MyMessageBox->fShow && MyMessageBox->fBuzzerOff==false)
			BuzzState=LED_Message;
		else if(fHome!=NULL && fHome->IsShown())
			BuzzState=LED_Homeing;
		else if(HSys.Sys.SystemStart)
			BuzzState=LED_Running;
		else
			BuzzState=LED_Pause;

		if(bMaintAlone==false)
		{
			int MusicSel=GetMaintenanceMusicSelect(BuzzState);
			//ErrJam honours the OFF BUZZER acknowledge (HT172 bAlarmBuzzer==false -> Off).
			if(BuzzState==LED_ErrJam && fNote!=NULL && fNote->IsBuzzerOff())
				MusicSel=0;
			DriveSystemMusic(MusicSel);
		}
	}

	//AI(ht160s-maintainer) 20260617 : derive Start/Pause panel-lamp state from the
	//run flag (mirrors HT172 ShowRunLed), then push all panel lamps. HT160 has no
	//ShowRunLed, so this lives here, the sole per-scan system-message hook.
	//AI(HT160S-Maintainer) 20260701 : while an alarm Note is up, TfNote::FlushLabel owns
	//the Start/Pause panel lamps (HT172 invitation-blink: after the operator selects a
	//recovery key -- or on a KeyCode==0 info alarm -- it blinks Start/Pause to cue "press
	//Start to resume"). Do NOT overwrite them here during a Note. During a modal Note only
	//note.cpp Timer1 calls DoSystemMessage (MainProc is suspended); outside an alarm derive
	//Start/Pause from the run flag as before.
	if(fNote==NULL || fNote->fShow==false)
	{
		if(HSys.Sys.SystemStart)
		{
			bLampStart=true;
			bLampPause=false;
		}
		else
		{
			bLampStart=false;
			bLampPause=true;
		}
	}
	DoPanelLamp();
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260622 : FormShow buzzer kicks. A modal dialog (MyMessageBox /
//fNote ShowModal) suspends MainProc, so DoSystemMessage never runs to sound the buzzer
//while the dialog is up. The dialog calls these in FormShow to start the same "Music
//Select" the scan driver would, so the message/alarm is audible the instant it appears.
//The scan driver keeps it driven for non-modal cases; FormClose's CloseBuzzerOff stops it.
void PlayMessageBuzzer()
{
	DriveSystemMusic(GetMaintenanceMusicSelect(LED_Message));
}
//---------------------------------------------------------------------------
void PlayAlarmBuzzer()
{
	DriveSystemMusic(GetMaintenanceMusicSelect(LED_ErrJam));
}
//---------------------------------------------------------------------------
void DoSystem()
{
	ScanAllMotorStatus();
	ScanSystemSenser();
	DoSystemMessage();
	if(HSys.Sys.SystemStart==false)
		CheckMotorPowerShutDown();
	if(CountMotorPowerDelay()==false && (fHome==NULL || fHome->IsShown()==false))
		HSys.Sys.SystemStart=false;
	DoSystemMessage();
	RecordSafeDoorStates();
}
//---------------------------------------------------------------------------
bool CheckMotorHome()
{
#ifdef SOFT_SIMULATE
	return true;
#else
	if(HSys.MotPtr==NULL)
		return false;
	for(int i=0; i<HSys.iTotalMotor; i++)
	{
		if(HSys.MotPtr[i]!=NULL && HSys.MotPtr[i]->GetEnable() && HSys.MotPtr[i]->bHomeFlag==false)
		{
			fAllMotorHome=false;
			return false;
		}
	}
	return true;
#endif
}
//---------------------------------------------------------------------------
void ProcessStartMode()
{
	if(SoftStart)
	{
		SoftStart=false;
		SoftStop=false;
		bLampSkip=false;
		bLampRetry=false;
		bLampTrayEnd=false;
		SetMotorSpeed();                                                        //AI(HT160S-Maintainer) 20260602 : apply working speed once per START, before SystemStart guard (HT172 0420 csystem.cpp port). Must precede SystemStart=true.
		HSys.Sys.SystemStart=true;
	}
	else if(SoftStop)
	{
		HSys.DecStopAllMotor();
		SoftStop=false;
		HSys.Sys.SystemStart=false;
	}
}
//---------------------------------------------------------------------------
void ProcessRunStatus(bool bProgramStart)
{
	AnsiString Status="HALT";
	TColor Color=clRed;

	if(bProgramStart==false)
	{
		Status="INIT";
		Color=clRed;
	}
	else if(HSys.Sys.SystemStart)
	{
		if(fAllMotorHome==false)
		{
			Status="HOMING";
			Color=clGreen;
		}
        else if(HSys.Sys.RunMode==Run_CleanOut)
        {
            Status="Clean Out";
			Color=clYellow;
            bLampCleanOut=true;
        }
        else if(HSys.Sys.RunMode==Run_TrayFeed)
        {
            Status="Tray Feed";
			Color=clYellow;
            bLampTrayFeed=true;
        }
        else if(HSys.Sys.RunMode==Run_OneCycle)
        {
            Status="One Cycle";
			Color=clYellow;
            bLampOneCycle=true;
        }
		else
		{
			Status="RUNNING";
			Color=clGreen;
			bLampCleanOut=false;
			bLampOneCycle=false;
			bLampTrayFeed=false;
		}
	}
	else if(IsSafeLock())
	{
		Status="LOCK";
		Color=clRed;
	}
	else if(IsEMGPressed()>0)
	{
		Status="EMG";
		Color=clRed;
	}
	else if(IsSystemPowerOff())
	{
		Status="MOTOR OFF";
		Color=clRed;
	}
	else if(IsSafeDoorOpen()>0)
	{
		Status="SAFE DOOR";
		Color=clRed;
	}
	else if(IsAirCheck())
	{
		Status="AIR";
		Color=clRed;
	}
	else if(HasICUnderMachine())
	{
		Status="PAUSE";
		Color=clRed;
	}

	SetMainStatus(Status, Color);
}
//---------------------------------------------------------------------------
void InitialAllTask(bool bKeepMaterial)
{
	if(DataModule1!=NULL)
		DataModule1->InitialAllTask(bKeepMaterial);
}
//---------------------------------------------------------------------------
void InitialAllModule()
{
}
//---------------------------------------------------------------------------
bool DoInitialProgramStart()
{
	static int Task=1;

	switch(Task)
	{
		case 1:
			HSys.DecStopAllMotor();
			InitialAllTask();
			LoadMotorSpeedFromIni();                                            //AI(HT160S-Maintainer) 20260602 : load+apply per-motor speed baseline so iPersentSpeed never stays at the 1%% default (HT172 0420 Speed port)
			//AI(HT160S-Maintainer) 20260624 : at software startup, make sure the motor
			//relay is energized. If it was left off at boot the machine has no motor
			//power, and ScanSystemSenser's per-cycle IsSystemPowerOff() stop then kills
			//any manual Motor-Test jog/move (the "jog only moves one step" symptom) and
			//the Power On lamp/state are wrong. One-time, real-machine only (sim forces
			//power on in CheckMotorPowerShutDown). Mirrors HT172's startup energize.
#ifndef SOFT_SIMULATE
			if(bMotorPowerState==false)
			{
				HSys.Sw.SwMotorRelay.On();
				bMotorPowerState=true;
				MotorPowerOnDelay=SERVER_MOTOR_POWER_ON_DELAY;
				bLampPowerOn=true;
				bLampPowerOff=false;
				RecordProcess("Startup: motor relay was off -> auto-energized");
			}
#endif
			Task=200;
			break;
		case 200:
			if(DataModule1!=NULL && DataModule1->Timer1!=NULL)
				DataModule1->Timer1->Enabled=true;
			return true;
	}
	return false;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260602 : SortArm single Z-home driver (HT172 0420
//SortArm1ZHome equivalent, rewritten non-FSM). Re-homes the 4 SortArm suck-Z
//axes non-blocking; returns true once every enabled axis reports homed. While
//bSortArmNeedHome is set MainProc suspends DataModule1->DoAllProcess().
static bool DoSortArmZHome()
{
	TTrayMotor *Z[4];
	Z[0]=HSys.Mot.MSuckZ_1;
	Z[1]=HSys.Mot.MSuckZ_2;
	Z[2]=HSys.Mot.MSuckZ_3;
	Z[3]=HSys.Mot.MSuckZ_4;

	bool bAllHomed=true;
	for(int i=0; i<4; i++)
	{
		if(Z[i]==NULL || Z[i]->GetEnable()==false)
			continue;
		AnsiString sErr="";
		if(Z[i]->Home(sErr)==false)
			bAllHomed=false;
	}
	return bAllHomed;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260602 : re-arm a fresh full-machine home. The Home
//button leaves bHomeFlag set from the previous home, so without clearing them
//CheckMotorHome() reports done and the engine never runs. Clearing every
//motor's home flag forces ProcessMotion Layer 1 to drive ProcessMotorHome.
void ArmMotorHome()
{
	if(HSys.MotPtr!=NULL)
	{
		for(int i=0; i<HSys.iTotalMotor; i++)
			if(HSys.MotPtr[i]!=NULL)
				HSys.MotPtr[i]->InitHomeTask();
	}
	fHome->iHomeStep=1;
	fAllMotorHome=false;
}
//---------------------------------------------------------------------------
//AI 20260619 : machine run-state command layer. Modelled on HT172 TfMain::
//Start()/sbPauseClick() (each owns ONE transition + RecordProcess). Migration is
//incremental: the HOME-abort path is wired (uHome timer); Pause/Stop are ready for
//the next slice; the operator Start button (LotID/SECS/RunCheck) is deferred.
const char* MachineTriggerName(eMachineTrigger trig)
{
	switch(trig)
	{
		case trigOperator:   return "operator";
		case trigSafetyDoor: return "safety-door";
		case trigEmg:        return "emergency-stop";
		case trigServoAlarm: return "servo-alarm";
		case trigHomeStop:   return "home-stop";
		case trigSecsRemote: return "secs-remote";
		default:             return "system";
	}
}
//---------------------------------------------------------------------------
//AI(machine-command-layer) 20260625 : production-start choke point. The gate + arm
//sequence moved out of TfMain::Start so the operator button, the SECS START host command
//and the panel key all reach production through ONE path. The UI-coupled arm work is in
//TfMain::DoStartArm (main.cpp, where edLotNo/Loader/fHome are in scope); here we own the
//precondition gate and the trigger log. No ShowMyMessage : the caller surfaces a
//rejection (operator -> popup, SECS -> HCACK).
eMachineStartResult MachineStart(eMachineTrigger trig, AnsiString &Reason)
{
	if(fMain==NULL)
		return msRejNoContext;
	if(HSys.Sys.SystemStart!=false)
		return msRejBusy;
	if(fMain->CheckLotDataReady(Reason)==false)
		return msRejNotReady;
	RecordProcess(AnsiString("MACHINE START by ")+MachineTriggerName(trig));
	fMain->DoStartArm();
	return msStarted;
}
//---------------------------------------------------------------------------
//Graceful pause : decelerate-stop, leave the machine paused (home state kept).
void MachinePause(eMachineTrigger trig)
{
	if(HSys.Sys.SystemStart==false)
		return;
	RecordProcess(AnsiString("MACHINE PAUSE by ")+MachineTriggerName(trig));
	HSys.Sys.SystemStart=false;
	HSys.DecStopAllMotor();
	SoftStop=true;
}
//---------------------------------------------------------------------------
//Hard stop : immediate stop (EMG / fault); same state effect, no decel.
void MachineStop(eMachineTrigger trig)
{
	if(HSys.Sys.SystemStart)
		RecordProcess(AnsiString("MACHINE STOP by ")+MachineTriggerName(trig));
	HSys.Sys.SystemStart=false;
	HSys.StopAllMotor();
	SoftStop=true;
}
//---------------------------------------------------------------------------
//HOME abort/stop : stop motors + clear the home-done flag so a fresh full-machine
//home is required before running. Port of the HT172 ProcessMotorHome SystemStart==
//false guard (DecStopAllMotor then close). The View (TfHome) still owns Close().
void MachineHomeAbort(eMachineTrigger trig)
{
	RecordProcess(AnsiString("MACHINE HOME-ABORT by ")+MachineTriggerName(trig));
	HSys.DecStopAllMotor();
	HSys.Sys.SystemStart=false;
	fAllMotorHome=false;
	SoftStop=true;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260619 : HOME monitor lifecycle owner. Ported from HT172,
//where ProcessMotorHome's own SystemStart==false top guard tears the monitor down.
//HT160 cannot do that inside the engine because ProcessMotion early-returns on
//SystemStart==false BEFORE it steps the engine, so the close/abort DECISION lives
//here and is called from the TOP of ProcessMotion, ahead of that early-return. This
//makes TfHome::Timer1Timer display-only. No-op unless the monitor is shown, so the
//headless --selftest-home path (fHome never Show()n) is unaffected. Both this and
//Timer1Timer run on the VCL main thread, so shared flags are touched cooperatively.
static void ProcessHomeLifecycle()
{
	//AI(home-realign) 20260626 : transient-drop debounce. A single-cycle SystemStart drop
	//is a now-suppressed settle/interlock transient (see the fHome->IsShown() guards on the
	//ion/air rungs in ScanSystemSenser and the motor-power-settle drop in DoSystem), NOT an
	//operator abort. Require the drop to persist >=2 consecutive home cycles before tearing
	//the monitor down so a one-cycle transient never closes HOME. A real operator Pause
	//closes fHome directly (uHome ScanKey -> Abort); a real persistent fault still aborts.
	static int iAbortDebounce=0;
	if(fHome==NULL || fHome->IsShown()==false)
	{
		iAbortDebounce=0;
		return;
	}

	//ORDER IS LOAD-BEARING : check normal completion BEFORE the SystemStart-drop
	//abort. On a HOME-button home the post-home SoftStop drives SystemStart=false,
	//and the held-tray prompt (case200 ShowMyMessage) also leaves SystemStart=false;
	//checking completion first lets such a home CLOSE-as-finished instead of being
	//mis-aborted. Do NOT swap these two blocks.
	if(fAllMotorHome)
	{
		iAbortDebounce=0;
		fHome->RequestCloseFinished();
		return;
	}

	//SystemStart dropped / paused while the monitor is up. Only after a start was
	//actually observed (SeenStart) and NOT during a deliberate power-cycle
	//(bHomePowerCycling drops SystemStart on purpose). Route the machine side through
	//the single command layer, then ask the View to close.
	if(fHome->SeenStart() && HSys.Sys.SystemStart==false && bHomePowerCycling==false)
	{
		if(++iAbortDebounce>=2)
		{
			iAbortDebounce=0;
			MachineHomeAbort(trigHomeStop);
			fHome->RequestClose();
		}
	}
	else
	{
		iAbortDebounce=0;
	}
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260623 : freeze/thaw the ACTUATOR (cylinder + sucker)
//push/pop timeout windows across a machine pause. TMyCylinder::Push/Pop and
//TMySucker::Suck/Destroy arm a wall-clock HTimer (Delay) for OnAlarmTime/
//OffAlarmTime; that clock keeps running while the machine is paused, so a long
//pause taken mid-travel burns the alarm budget and false-alarms on resume even
//though the actuator never actually timed out. Pause the Delay on the SystemStart
//falling edge and ReStart it on the resume edge so only RUNNING time counts.
//Scope is DELIBERATE: cylinders + the 4 SortArmSuck suckers only, NOT a global
//HTimer registry. The Pad operator-panel link and bin-display COM HTimers are
//pumped by the free-running ComPort spin (no SystemStart gate) and MUST keep
//running while paused -- a blanket pause would deadlock their Spin state machines.
//(See the INTENTIONALLY-EMPTY stubs in HTimer.cpp.) Idle timers are unaffected:
//HTimer::Clear() (run by the next Push/Suck arm) resets Paused, so even a timer
//left Paused by a skipped resume edge (e.g. the bFirstRun lot-start re-init)
//self-heals on its next arm.
static void PauseActuatorTimeoutTimers()
{
	int i, r, c;
	for(i=0; i<MaxCylinderItem; i++)
		Cylinder[i].Delay.Pause();
	for(r=0; r<MAX_SUCKER_ROW; r++)
		for(c=0; c<MAX_SUCKER_COL; c++)
			HSys.Suck.SortArmSuck.Suck[r][c].Delay.Pause();
	//AI(ht160s-actuator-timer) 20260627 : extend the freeze to the feeder/sort modules'
	//own wall-clock timeout windows (Color ScanDelay+AmrFeed, Loader CcdDelay x2+FeedWait x2, SortArm
	//ResidueDelay[], Empty AmrFeed, Auto AmrFull[]) so a mid-op pause is not charged
	//budget either; each module owns its timer list behind a public accessor (add
	//future timeout timers there, not here). NULL-guarded like every csystem->module call.
	if(ColorModule!=NULL)
		ColorModule->PauseTimeoutTimers();
	if(LoaderModule!=NULL)
		LoaderModule->PauseTimeoutTimers();
	if(SortArmModule!=NULL)
		SortArmModule->PauseTimeoutTimers();
	if(EmptyModule!=NULL)
		EmptyModule->PauseTimeoutTimers();
	if(AutoModule!=NULL)
		AutoModule->PauseTimeoutTimers();
}
//---------------------------------------------------------------------------
static void ReStartActuatorTimeoutTimers()
{
	int i, r, c;
	for(i=0; i<MaxCylinderItem; i++)
		Cylinder[i].Delay.ReStart();
	for(r=0; r<MAX_SUCKER_ROW; r++)
		for(c=0; c<MAX_SUCKER_COL; c++)
			HSys.Suck.SortArmSuck.Suck[r][c].Delay.ReStart();
	if(ColorModule!=NULL)
		ColorModule->ReStartTimeoutTimers();
	if(LoaderModule!=NULL)
		LoaderModule->ReStartTimeoutTimers();
	if(SortArmModule!=NULL)
		SortArmModule->ReStartTimeoutTimers();
	if(EmptyModule!=NULL)
		EmptyModule->ReStartTimeoutTimers();
	if(AutoModule!=NULL)
		AutoModule->ReStartTimeoutTimers();
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260602 : ported HT172 0420 ProcessMotion lifecycle in
//HT160S procedural style (no FSM). Layers: UPH pause accounting -> home not done
//-> home-just-finished finalize -> SortArm single Z-home -> normal production
//with lot-start init and RunMode finish dispatch.
void ProcessMotion()
{
	//Pause stopwatch: SystemStart falling edge starts timing so the paused
	//interval is excluded from UPH. Covers operator SoftStop and any safety stop.
	static bool bPrevSystemStart=false;
	if(bPrevSystemStart && HSys.Sys.SystemStart==false)
	{
		if(bCalculatePauseTime==false)
		{
			tUPH_PauseStartTime=Now();
			bCalculatePauseTime=true;
			//AI(HT160S-Maintainer) 20260623 : freeze cylinder/sucker push-pop
			//timeout windows on this same pause one-shot so the paused interval
			//is not charged against OnAlarmTime/OffAlarmTime (no false alarm on
			//resume). Runs even while SystemStart==false (it is before the gate).
			PauseActuatorTimeoutTimers();
		}
	}
	bPrevSystemStart=HSys.Sys.SystemStart;

	//AI(HT160S-Maintainer) 20260619 : drive the HOME monitor lifecycle (close on
	//completion, abort+close on SystemStart-drop) BEFORE the SystemStart gate below,
	//so the monitor can tear down even when SystemStart has already gone false. HT172
	//does this inside the engine's top guard; HT160 cannot (the engine is not stepped
	//once SystemStart==false), so the owner lives here.
	ProcessHomeLifecycle();

	if(HSys.Sys.SystemStart==false)
		return;

	//Resume edge: accumulate the elapsed pause once, on the cycle we resume.
	if(bCalculatePauseTime)
	{
		tUPH_PauseTime=tUPH_PauseTime+(Now()-tUPH_PauseStartTime);
		bCalculatePauseTime=false;
		//AI(HT160S-Maintainer) 20260623 : thaw the actuator timeout windows on the
		//resume edge (paired with PauseActuatorTimeoutTimers on the falling edge)
		//so only running time counts. HTimer::ReStart() defers the paused span via
		//iPauseLen; idle timers are harmlessly cleared on their next arm.
		ReStartActuatorTimeoutTimers();
	}

	//Layer 1 : full-machine home not finished -> drive the home engine. The
	//engine raises the TrayArm Z + opens clamps, batch-homes the 4 sucker Z,
	//then batch-homes every XY axis. When all enabled axes report homed,
	//CheckMotorHome() turns true next cycle and Layer 2 finalizes.
    CheckMotorHome();
	//Layer 2 : home just completed this cycle -> finalize exactly once.
	if(fAllMotorHome==false)
	{
        bSortArmNeedHome=false;
		if(HSys.Sys.RunMode!=Run_Home)
			ChangeRunMode(Run_Home);
        if(fHome->ProcessMotorHome())
        {
            //AI(HT160S-Maintainer) 20260612 : a full-machine home triggered mid-production
            //(operator HOME or motor-anomaly recovery) must NOT forget the material the
            //machine physically still holds. Pass bKeepMaterial=true so SortArm keeps its
            //sucked IC (vacuum re-asserted), TrayArm keeps its gripped tray + delivery job,
            //and Auto keeps its car/stack identity. Production then resumes (or pauses then
            //resumes on next Start) without dropping or misrouting material.
            InitialAllTask(true);
            fAllMotorHome=true;
            ChangeRunMode(Run_Normal);                                          //20120102 Daver add
            SetMotorSpeed(true);                                                //AI(HT160S-Maintainer) 20260602 : re-apply working speed after home (HT172 0420 csystem/uhome port)
            if(bHomeByStart)
                bHomeByStart=false;
            else
                SoftStop=true;
        }
		return;
	}

	//Layer 3 : SortArm single Z-home request. Writer in SortArm fault path is
	//not yet wired; the flag stays dormant until a future module change sets it.
	if(bSortArmNeedHome)
	{
		if(DoSortArmZHome())
			bSortArmNeedHome=false;
		return;
	}

	//Layer 4 : normal production. Lot-start one-shot init, then RunMode finish.
	if(bFirstRun)
	{
		bFirstRun=false;
		tRunData.StartTime=Now();
		tUPH_PauseTime=StrToTime("00:00:00");
		bCalculatePauseTime=false;
	}

	if(HSys.Sys.RunMode==Run_CleanOut)
	{
		if(CheckCleanOutFinish())
		{
			tRunData.LotEndTime=Now();
			tRunData.UPH=GetCalculateUPH(tRunData.LotEndTime);
			InitialAllTask();
			HSys.Sys.bCleanOut=false;   //AI(HT160S-Maintainer) 20260605 : CleanOut fully done, drop nested latch
			//AI(HT160S-Maintainer) 20260612 : pop a "CleanOut finish" note (ref HT172
			//  ShowSystemError(SnFKCleanOut, K_SKIP|K_TRAY_FEED)). Operator picks SKIP
			//  (end : back to Normal + stop) or TRAY_FEED (drain remaining trays).
			//  NOTE: CheckAllTrayFeedFinish() is still a stub returning false, so the
			//  TRAY_FEED branch will not auto-complete until that is wired : SKIP is
			//  the only fully-working choice today.
			int retCleanOut=ShowSystemError(HSys.Sen.SnFKCleanOut.Name, K_SKIP, 0);
			// if(retCleanOut==K_TRAY_FEED)
			// {
			// 	CheckAllTrayFeedFinish(true);   // reset per-module TrayFeed finish flags
			// 	ChangeRunMode(Run_TrayFeed);
			// }
			// else
			{
				ChangeRunMode(Run_Normal);
				SoftStop=true;
			}
		}
	}
	else if(HSys.Sys.RunMode==Run_OneCycle)
	{
		if(CheckOneCycleFinish())
		{
			InitialAllTask();
			//AI(HT160S-Maintainer) 20260605 : place-before-stop done. If OneCycle was
			//launched mid-CleanOut, resume CleanOut and run it to completion (no stop :
			//nested-continuation latch); otherwise return to Normal and stop the machine.
			if(HSys.Sys.bCleanOut)
            {
                ChangeRunMode(Run_CleanOut);
                SoftStop=true;
            }
			else
			{
				ChangeRunMode(Run_Normal);
				SoftStop=true;
			}
		}
	}
	else if(HSys.Sys.RunMode==Run_TrayFeed)
	{
		if(CheckAllTrayFeedFinish())
		{
			InitialAllTask();
			ChangeRunMode(Run_Normal);
			bFirstRun=true;
			SoftStop=true;
		}
	}
}
//---------------------------------------------------------------------------
void DoTemptureControl()
{
}
//---------------------------------------------------------------------------
void AddNoNeedHomeSensorList()
{
}
//---------------------------------------------------------------------------
bool CheckCleanOutFinish()
{
	//AI(HT160S-Maintainer) 20260602 : CleanOut done = Auto stations cleaned AND no
	//IC left under the machine. Aggregates real module state so the RunMode revert
	//never fires instantly at CleanOut start (was a stub returning true).
	//AI(cleanout) 20260701 : full cascade Loader -> SortArm -> Auto -> TrayArm -> Empty/Color.
	//TrayArm/Empty/Color now participate (TrayArm empties its hand + Z-up; Empty/Color GoUp all
	//trays back to their car). Loader now also requires the front/rear/supply-car sensors clear
	//(on-machine 2026-07-01 : a Loader empty tray was left behind by the old carriage-flag-only check).
	if(LoaderModule!=NULL && LoaderModule->IsAllCleanOutFinish()==false)
		return false;
	if(SortArmModule!=NULL && SortArmModule->IsCleanOutFinish()==false)
		return false;
	if(AutoModule!=NULL && AutoModule->IsAllCleanOutFinish()==false)
		return false;
	if(TrayArmModule!=NULL && TrayArmModule->IsCleanOutFinish()==false)
		return false;
	if(EmptyModule!=NULL && EmptyModule->IsCleanOutFinish()==false)
		return false;
	if(ColorModule!=NULL && ColorModule->IsCleanOutFinish()==false)
		return false;
	if(HasICUnderMachineForCleanOut())
		return false;
	return true;
}
//---------------------------------------------------------------------------
bool CheckOneCycleFinish()
{
	//AI(HT160S-Maintainer) 20260605 : OneCycle done = SortArm has placed its held IC
	//and is idle (place-before-stop). OneCycle deliberately does NOT drain trays, so
	//IC may remain under the machine : do not gate on HasICUnderMachine here.
	if(SortArmModule!=NULL && SortArmModule->IsOneCycleFinish()==false)
		return false;
	return true;
}
//---------------------------------------------------------------------------
void CCDLoadJobs()
{
}
//---------------------------------------------------------------------------
bool HasICUnderMachine()
{
	if(HSys.VMotPtr==NULL)
		return false;
	for(int i=0; i<HSys.iTotalVMotor; i++)
	{
		if(HSys.VMotPtr[i]!=NULL && HSys.VMotPtr[i]->HasIC())
			return true;
	}
	return false;
}
//---------------------------------------------------------------------------
bool HasAutoICInMachine()
{
	return HasICUnderMachine();
}
//---------------------------------------------------------------------------
bool HasICUnderFrontMachine()
{
	return false;
}
//---------------------------------------------------------------------------
bool HasICUnderMachineForCleanOut()
{
	return HasICUnderMachine();
}
//---------------------------------------------------------------------------
bool CheckEmpty1TrayFeedFinish()
{
	return true;
}
//---------------------------------------------------------------------------
bool CheckAllTrayFeedFinish(bool reset)
{
	//AI(HT160S-Maintainer) 20260602 : no per-module TrayFeed finish flag exists yet,
	//so report not-finished to preserve current behavior. Wire real module finish
	//state here when modules expose it.
	return false;
}
//---------------------------------------------------------------------------
void ChangeRunMode(RunModeEnum RunMode)
{
	HSys.Sys.RunMode=RunMode;
}
//---------------------------------------------------------------------------
int GetCalculateUPH(TDateTime tEndTime)
{
	//AI(HT160S-Maintainer) 20260602 : HT172 0420 UPH excludes accumulated pause time.
	TDateTime tElapsed=tEndTime-tRunData.StartTime-tUPH_PauseTime;
	double dSeconds=tElapsed.operator double()*86400.0;
	if(dSeconds<=0.0)
		return 0;
	return (int)((double)tRunData.TotalIC*3600.0/dSeconds);
}
//---------------------------------------------------------------------------
void RecordSafeDoorStates()
{
	//AI(ht160s-maintainer) 20260619 : log idle-state safety-door open events
	//(rising edge) while the machine is stopped, using HT160's six named door
	//sensors. Running-state stop-on-door-open is already logged in
	//ScanSystemSenser; this adds idle/maintenance door-open audit records.
	static bool bClear=false;
	static bool bDoorOpen[6]={false,false,false,false,false,false};

	TMySensor *pDoor[6];
	pDoor[0]=&HSys.Sen.SnSafeDoorFront;
	pDoor[1]=&HSys.Sen.SnSafeDoorRight;
	pDoor[2]=&HSys.Sen.SnSafeDoorLeft;
	pDoor[3]=&HSys.Sen.SnSafeSlideDoorRight;
	pDoor[4]=&HSys.Sen.SnSafeSlideDoorLeft;
	pDoor[5]=&HSys.Sen.SnSafeAuto6;

	const char *pName[6];
	pName[0]="Safe Door Front is Opened";
	pName[1]="Safe Door Right is Opened";
	pName[2]="Safe Door Left is Opened";
	pName[3]="Safe Slide Door Right is Opened";
	pName[4]="Safe Slide Door Left is Opened";
	pName[5]="Safe Door Auto6 is Opened";

	if(HSys.Sys.SystemStart)
	{
		if(bClear==false)
		{
			for(int i=0; i<6; i++)
				bDoorOpen[i]=false;
			bClear=true;
		}
		return;
	}
	bClear=false;

	for(int i=0; i<6; i++)
	{
		if(pDoor[i]->Enable && pDoor[i]->IsOff() && bDoorOpen[i]==false)
		{
			RecordProcess(pName[i]);
			bDoorOpen[i]=true;
		}
	}
}
//---------------------------------------------------------------------------