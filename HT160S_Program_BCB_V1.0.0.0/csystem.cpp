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
#include "cSoterOutput.h"
#include "GeneralSetting.h"   //AI(ht160s-overcount-tripqueue D6) 20260721 : GeneralSetting.bUseAMR gates the CleanOut auto-Lot-End
#include "SecsGem/uAgvStation.h"   //AI(amr-unmanned W4) 20260721 : AgvCoord.TimeoutPending -> WAR0962 in the main loop
#include "SecsGem/uHGemClass.h"   //AI(secs-e30-gate) 20260803 : HTGem control-state hooks for the SecsGetControlState bridge
#include "uAmrInject.h"   //AI(ht160s-agv) 20260708 : clear AMR manual-inject test mode on machine start
#include "uHome.h"
#include "uspeed.h"                     //AI(HT160S-Maintainer) 20260602 : SetMotorSpeed / LoadMotorSpeedFromIni (Speed module port)
#include "note.h"
#include "cStateRecordHT160.h"   //AI(ht160s-obsv-p0) 20260720 : gStateRecord post-resume baseline snapshot                       //AI(HT160S-Maintainer) 20260603 : ShowSystemError for ProcessAlarm dispatch
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

	//AI(secs-onsite0731) 20260801 : mirror for SVID 1011 "Machine State". HT9045 binds its
	//SVID 1011 directly to fMain->palMainStatus (uHGemHT9045_SV.cpp:71) via a TObject*
	//overload of SetSVDataPointer that HT160S does not have - its only overload takes void*
	//and DataItemOutSVItem casts an ASCII Ptr straight to AnsiString*, so handing it a
	//TPanel* would be a wild read. Mirroring here instead keeps the SECS serialize path off
	//VCL entirely, and survives a rename of palMainStatus or a divergence of the bilingual
	//palMainStatus_En. Value domain: HALT/INIT/HOMING/Clean Out/Tray Feed/One Cycle/RUNNING/
	//LOCK/EMG/MOTOR OFF/SAFE DOOR/AIR/PAUSE (see ProcessRunStatus below).
	g_sMachineStateText=StatusText;

	//AI(secs-ceid-align9045) 20260729 : CEID 27 "Change Machine State". HT9045 main.cpp:18473
	//compares its cached sMachineState against palMainStatus->Caption on every scan and reports
	//on the edge; this is the same test at HT160S's single status writer. The first call after
	//boot moves "" -> the real status and therefore reports once, which is what HT9045 does too
	//(its sMachineState also starts empty). EmitRunStatusChange self-gates on USE_SECS_GEM +
	//HSMS SELECTED, so nothing goes out before the link is up.
	static AnsiString sLastSecsStatus="";
	if(sLastSecsStatus!=StatusText)
	{
		sLastSecsStatus=StatusText;
		fMain->EmitRunStatusChange();
	}
}
//---------------------------------------------------------------------------
//AI(secs-ceid-align9045) 20260729 : CEID 123 "Safe Door On Off". Port of HT9045
//csystem.cpp:2765-2777, which reports on a safety-door edge in EITHER direction, running or
//not. Deliberately separate from RecordSafeDoorStates below, which only logs RISING edges
//while the machine is STOPPED - the host wants production-time transitions and closes too.
//
//Tracked as one aggregate open/closed boolean off IsSafeDoorOpen() rather than HT9045's
//per-door edges, for three reasons : (a) the S6F11 for this CEID carries report 1 (machine
//context SVs) and has no door-id field, so per-door granularity is invisible to the host
//anyway; (b) it reuses the single existing definition of "a door is open" instead of forking
//a second sensor list that could drift from it; (c) IsSafeDoorOpen() already returns 0 under
//SOFT_SIMULATE, so the simulator cannot emit phantom door events and this needs no #ifdef of
//its own (which would rot, since dev builds keep SOFT_SIMULATE on).
static void ReportSafeDoorChangeToSecs()
{
	static bool bSeeded=false;
	static bool bWasOpen=false;

	bool bOpen=(IsSafeDoorOpen()>0);

	//First pass only seeds the baseline, so booting with a door already open is not
	//reported as a transition.
	if(bSeeded==false)
	{
		bSeeded=true;
		bWasOpen=bOpen;
		return;
	}
	if(bOpen!=bWasOpen)
	{
		bWasOpen=bOpen;
		if(fMain!=NULL)
			fMain->EmitSafeDoorChange();
	}
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
//AI(amr-unmanned W4) 20260721 : consume the AGV coordinator's per-station handshake-
//timeout latch in the MAIN control loop (safe modal context; TAgvCoordinator runs on the
//SECS 1s timer and must NOT pop modals there). Fires WAR0962 (B-class) once per timed-out
//station; the only key is K_RETRY -> RetryStation re-CALLs the AGV. AMR-mode only. P1
//(Loader/si=0) is never latched by the coordinator (its supply timeout is the S4 source-
//dry auto-CleanOut, Q3 ruling), so this loop starts at si=1 (Empty/Color supply + Auto
//collect). Manual recovery (operator empties car / refills) self-heals via the sensor
//paths without ever reaching here.
static void ServiceAgvTimeoutAlarm()
{
	if(GeneralSetting.bUseAMR==false)
		return;
	//AI(amr-unmanned W4-fix) 20260722 : freeze during HOME, mirroring the SECS-timer
	//coordinator's home-freeze (uAgvStation ServiceHandshake). Without this a TimeoutPending
	//latched just before a HOME could pop WAR0962 (ShowNoteAlarm DecStopAllMotor +
	//SystemStart=false) mid-homing and abort the just-armed sequence. Consume the latch only
	//after HOME completes (the pending flag survives; it fires on the next non-HOME tick).
	if(HSys.Sys.RunMode==Run_Home || fAllMotorHome==false)
		return;
	for(int si=1; si<AGV_STATION_COUNT; si++)
	{
		if(AgvCoord.TimeoutPending[si]==0)
			continue;
		AnsiString Where;
		if(si==1)      Where="Empty (P2)";
		else if(si==2) Where="Color (P3)";
		else           Where="Auto"+IntToStr(si-2)+" (P"+IntToStr(si+1)+")";
		ShowMyError("WAR0962", LangT("AGV/AMR handshake timeout - AGV did not respond")+" : "+Where, K_RETRY);
		AgvCoord.RetryStation(si);   // clears pending + station->IDLE so PollAndCall re-CALLs
	}
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
	//AI(secs-audit-fix) 20260729 : an armed SECS panel override used to be left FROZEN here - this
	//early return happens before fMain->ScanPanelKeys() and before DoSystem()->DoSystemMessage(),
	//so nothing re-drives the SwMusic outputs and they simply RETAIN their last written value, and
	//BOTH escapes are dead in that state (the panel ALARM RESET rung lives inside the skipped
	//ScanPanelKeys; the Maintenance Release button is on another form). The release is deliberately
	//NOT done here : this branch runs every scan, so it would fight a host that re-arms mid-session
	//and would drive SwMusic1..4 - which are manual test buttons inside that very view - Off under
	//the engineer's fingers. It is done ONCE on the open edge in Tfiosetview::FormShow, before
	//BackupOutputData() takes the output snapshot, so the snapshot cannot capture (and the
	//on-close RestoreOutputData cannot resurrect) a host-driven buzzer level after the flag that
	//makes the escapes work has already been cleared.
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
	if(fMain!=NULL)
		fMain->ShowProductInfo();      //AI(ht160s-uph) 20260707 : live Lot/UPH product-info grid + live UPH cell
	if(fMain!=NULL)
		fMain->ShowBinCntInfo();       //AI(loadtotal) 20260801 : Load / Total panels above the Bin column
	if(fMain!=NULL)
		fMain->ShowTrayUphHistory();   //AI(ht160s-uph) 20260707 : rolling per-tray UPH history grid

	//AI(ht160s-lot-webapi) 20260612 : Stage 4 : drive any in-flight Lot WebAPI pull.
	// MainProc runs on the VCL main thread (TRunControl::Synchronize), so this is a
	// safe place to consume the async response and reproject the Lot list. No-op when idle.
	if(fMain!=NULL)
		fMain->PollLotDataWebApi();

	//AI(ht160s-ftp) 20260721 : drain background FTP upload results into the EventLog.
	//Same safe main-thread spot as the WebAPI poll; cheap no-op when nothing uploaded.
	if(fMain!=NULL)
		fMain->PollFtpUploadResults();

	//AI(ht160s-uph) 20260706 : advance the per-tray UPH observer (reads Auto status).
	TrayUphLog_Tick();

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

	//AI(amr-unmanned W4) 20260721 : pop WAR0962 for any AGV handshake that timed out this
	//cycle (main-loop modal context; latched by the SECS-timer coordinator). Before
	//ProcessAlarm so it shares the same drain tick.
	ServiceAgvTimeoutAlarm();

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
//AI(secs-kyec-rcmd4) 20260728 : SECS host panel override (S2F41 PP_SIGNALTOWER / PP_MUSIC).
//Modelled on HT9045 SECS_GEM_PPSIGNALTOWER_CONTROL_flag / SECS_GEM_PPMUSIC_CONTROL_flag.
//KYEC fires the pair ~0.3s apart (0.256-0.361 s, PP_MUSIC first) to call an operator to the
//machine, then releases it with an EMPTY parameter list a few seconds later.
//AI(secs-msggap-fix) 20260729 : two field readings corrected after recounting the 20 KYEC logs.
//The trigger is a GENERIC host attention annunciator, not specifically the S10F5 "tester is
//IDLE, priority lot waiting" text - only 3 of the 6 SET bursts follow that one; the others
//follow "has no schedule.", "MES_Status Changed toSetUp", and an S10F3 ART-abort. And two of
//the nine CLEARs land 0.3 s after the equipment's OWN alarm event report, so the host does use
//this channel around alarm conditions too - the suppression gates below stand on the safety
//argument alone. The "longest armed span 1723.8 s" figure is the longest CONTINUOUS armed
//interval (15:02:41.838 -> 15:31:25.593) and contains three SETs and one release; the longest
//single SET->CLEAR pair is 1131.9 s. Either way the latch outlives any operator's patience,
//which is the only thing that argument needs.
//Colour domain 0=off / 1=on / 2=blink; music class 1..4 -> SwMusic1..SwMusic4 (database.h).
static bool s_bSecsTowerOverride=false;
static int  s_iSecsTowerRed=0;
static int  s_iSecsTowerYellow=0;
static int  s_iSecsTowerGreen=0;
static bool s_bSecsMusicOverride=false;
static int  s_iSecsMusicClass=0;
//---------------------------------------------------------------------------
static bool SecsTowerLampOn(int State, bool BlinkPhase)
{
	if(State==1)
		return true;
	if(State==2)
		return true;   //AI(secs-kyec-rcmd4-fix) 20260728 : host value 2 is "blink", but HT160S tower must NOT blink (user decision) -> render BLINK as STEADY ON, exactly like GetTowerLightConfigOutput (maintenance.cpp:284-285). Returning BlinkPhase here would strobe the three physical tower outputs. BlinkPhase is kept in the signature for call-site symmetry with GetTowerLightConfigOutput. //AI(secs-msggap-fix) 20260729 : recount - 5 of the 6 KYEC SET packets carry 2 on every colour (not ALL of them; 19:07:50.526 is RED=2 GREEN=0 YELLOW=0), so blink-valued packets are still the normal case and this mapping is still the hot path, but never assume the three colours share a value. The resulting operator-visible deviation (9045 blinks all three, HT160S lights all three steady) is declared in docs/SECS/HT160S_SECS_Interface_Spec_20260727.md 3.4.
	return false;
}
//---------------------------------------------------------------------------
void SetSecsTowerOverride(int Red, int Yellow, int Green)
{
	//A colour the host did not name keeps its previous value : HT9045 writes only the CP
	//names present in the packet. Callers pass -1 for "not specified".
	if(Red>=0)
		s_iSecsTowerRed=Red;
	if(Yellow>=0)
		s_iSecsTowerYellow=Yellow;
	if(Green>=0)
		s_iSecsTowerGreen=Green;
	s_bSecsTowerOverride=true;
}
//---------------------------------------------------------------------------
void ClearSecsTowerOverride()
{
	//AI(secs-kyec-rcmd4-fix) 20260728 : also drop the remembered colours. SetSecsTowerOverride
	//treats -1 as "keep previous", so leaving them set meant a later SET naming only GREEN
	//silently resurrected the RED value from a cleared, unrelated override.
	s_bSecsTowerOverride=false;
	s_iSecsTowerRed=0;
	s_iSecsTowerYellow=0;
	s_iSecsTowerGreen=0;
}
//---------------------------------------------------------------------------
void SetSecsMusicOverride(int MusicClass)
{
	//The S2F42 branch already validates 1..4; clamp again so no future caller can reproduce
	//the HT9045 bug SW[SwMusic1+CLASS-1].On() with no range check, where CLASS=0 drives the
	//switch immediately BEFORE SwMusic1.
	if(MusicClass<1 || MusicClass>4)
		return;
	s_iSecsMusicClass=MusicClass;
	s_bSecsMusicOverride=true;
}
//---------------------------------------------------------------------------
void ClearSecsMusicOverride()
{
	//AI(secs-msggap-fix) 20260729 : dropping the flag is NOT enough - it must also silence the
	//switch. SwMusic1..4 are latching myswitch outputs whose only scan-time driver is
	//DriveSystemMusic(), and that call sits inside the if(bMaintAlone==false) block in
	//DoSystemMessage. bMaintAlone means "fMaintenance visible with no dialog up", i.e. it is TRUE
	//exactly when the operator is looking at the Maintenance "Release Host Override" button - so
	//nothing rewrites the switch, it keeps its last written value, and the buzzer went on
	//sounding after a release that reported success. The same freeze applies to the host's own
	//PP_MUSIC empty-list release, to S1F16 OFF-LINE and to link-lost, and in those three cases
	//RefreshSecsOverrideStatus() then greys the Release button out because the latch is already
	//clear - buzzer still sounding, screen escape disabled, handler early-returns. Silencing here
	//covers all six release paths at once instead of patching each caller.
	//Only when it WAS armed : 5 of KYEC's 9 CLEARs arrive with nothing armed, and a no-op clear
	//must stay a no-op rather than reach in and mute a machine-owned buzzer.
	//Safe against the machine's own alarm tone : whenever the machine (not Maintenance) owns the
	//panel, DriveSystemMusic re-drives the correct MusicSel on the next 10 ms tick.
	bool bWasArmed=s_bSecsMusicOverride;
	s_bSecsMusicOverride=false;
	s_iSecsMusicClass=0;
	if(bWasArmed)
		CloseBuzzerOff();
}
//---------------------------------------------------------------------------
void ClearSecsPanelOverride()
{
	//Operator escape : releases BOTH overrides at once, matching HT9045 which clears the pair
	//together from the ALARM RESET key and from message-box acknowledge. HT160 callers :
	//TfMain::ScanPanelKeys (panel ALARM RESET - the ONLY escape when no dialog is up),
	//TfNote::BtnOffBuzzerClick, TfNote::ScanKey, TMyMessageBox::btnOffBuzzerClick,
	//TMyMessageBox::ScanKey, TfMaintenance::btnSecsOverrideReleaseClick (screen escape),
	//the IO-Set-View suspend rung in ProcessRunStatus, HT160Gem::S1F16_OFFLINEAcknowledge and
	//HT160Gem::OnCommunicationLost.
	//NOT ported : HT9045 also clears the tower flag from four unrelated RCMD branches that
	//happen to receive an empty parameter list (START_LOT / START_AGV / AUTHORITY_CHECK /
	//EESUG_Offest). Those are copy-paste artifacts, and KYEC does send START_AGV - porting
	//them would let an unrelated host command silently cancel a tower override.
	//AI(secs-msggap-fix) 20260729 : precision on the 9045 side - those four RCMD sites clear the
	//TOWER flag ONLY, they leave the music flag armed (which is its own defect over there), and
	//9045's genuine PAIR-clear surface is FOUR sites, not two : the ALARM RESET key rung in main,
	//TfNote's off-buzzer, and TWO in its message box (the off-buzzer button AND a separate
	//alarm-reset panel click). That 4-site shape is exactly why HT160 needs both a key-scan rung
	//AND a button handler on each dialog rather than one of the two.
	ClearSecsTowerOverride();
	//AI(secs-msggap-fix) 20260729 : delegate instead of clearing the two music fields inline, so
	//the buzzer-silencing in ClearSecsMusicOverride() cannot be bypassed by this path (which is
	//the one every operator escape actually calls).
	ClearSecsMusicOverride();
}
//---------------------------------------------------------------------------
bool IsSecsPanelOverrideActive()
{
	return (s_bSecsTowerOverride || s_bSecsMusicOverride);
}
//---------------------------------------------------------------------------
//AI(secs-e30-gate) 20260803 : GEM control-state bridge for the maintenance SECS tab. Calls through
//the HTGem* base (HSys.MyGem) exactly as note.cpp does for ReportSkipICCount, so no UI unit has to
//include the SECS headers. NULL-safe : on a build with SECS disabled MyGem does not exist and the
//tab simply shows nothing selectable.
void SecsOperatorSetControlState(int iGemStdState)
{
	if(HSys.MyGem!=NULL)
		HSys.MyGem->OperatorSetControlState(iGemStdState);
}
//---------------------------------------------------------------------------
int SecsGetControlState()
{
	return (HSys.MyGem!=NULL) ? HSys.MyGem->GetControlState() : 0;
}
//---------------------------------------------------------------------------
AnsiString SecsDescribeControlState()
{
	return (HSys.MyGem!=NULL) ? HSys.MyGem->DescribeControlState() : AnsiString("");
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

	//AI(secs-kyec-rcmd4) 20260728 : host PP_SIGNALTOWER override replaces the per-RunState
	//lamp table. One insertion covers BOTH the on-screen fMain->led* below and the physical
	//SwTowerGreen/Yellow/Red outputs, because HT160 drives both from these same three bools.
	//DELIBERATE DEVIATION from HT9045 : suppressed while an alarm Note is up, so a host
	//"come load a lot" flash can never mask the machine's own red alarm lamp. This matters
	//because BOTH modal dialogs re-drive DoSystemMessage() every 10 ms while MainProc is
	//suspended (note.cpp Timer1, mymessbox.cpp Timer1), and HT160 writes the tower outputs
	//unconditionally with no blink-phase early-out.
	//AI(secs-kyec-rcmd4-fix) 20260728 : the fNote gate ALONE is not enough and assuming
	//"machine red lamp <=> an alarm Note is up" is wrong - see the buzzer comment below, which
	//says so explicitly. RunState reaches LED_ErrJam straight off the live safety sensors in
	//GetTowerLightRunState() (EMG / power off / safe lock / safe door / air / ion fan) with NO
	//dialog involved. Without the RunState test a host override armed before an operator opens
	//a safety door would keep driving green while the door is open, and a host RED=0 would take
	//the physical red output dark during EMG. Safety-derived red always wins over the host.
	//AI(secs-audit-fix) 20260729 : TMyMessageBox added to the gate, matching the buzzer override
	//70 lines below (which already tests both dialogs). TMyMessageBox is a first-class alarm
	//surface on HT160 - ShowMyMessage() does DecStopAllMotor() + SystemStart=false and then
	//ShowModal, and its Timer1 re-drives DoSystemMessage() every 10 ms while MainProc is
	//suspended - so a host lamp override survived a popup that had just STOPPED the machine.
	//The RunState!=LED_ErrJam test does not cover it either : GetTowerLightRunState() only
	//reaches LED_ErrJam for EMG / power off / safe lock / safe door / air / ion fan, so an
	//application-level alarm box with no safety sensor tripped yields LED_Pause.
	//bFormShowNoStop EXCLUDED on purpose : ShowMyOKMessageNoStop / ShowMyMessage_Run /
	//ShowMyMessageBox_YES_NO (mymessbox.cpp:164/176/193) set it, and those boxes deliberately do
	//NOT stop the machine (the DecStopAllMotor block is gated on bFormShowNoStop==false,
	//mymessbox.cpp:449). They are ordinary confirmations - "Confirm home?", "Confirm Clean Out?",
	//"want to restore?" - so the machine-stopped justification above does not apply to them.
	//Suppressing for those would take a host-armed RED output DARK for as long as an unrelated
	//confirmation sits unanswered, and a YES/NO box has no OFF BUZZER escape of its own
	//(btnOffBuzzer hidden, fScanPanel=false at mymessbox.cpp:190-192). Only a real stopping box
	//suppresses the override. NOTE the resulting asymmetry with the buzzer gate below, which
	//suppresses for these boxes too - that is pre-existing behaviour and left alone.
	if(s_bSecsTowerOverride && RunState!=LED_ErrJam &&
		(fNote==NULL || fNote->fShow==false) &&
		(MyMessageBox==NULL || MyMessageBox->fShow==false || MyMessageBox->bFormShowNoStop))
	{
		GreenOn=SecsTowerLampOn(s_iSecsTowerGreen, BlinkPhase);
		YellowOn=SecsTowerLampOn(s_iSecsTowerYellow, BlinkPhase);
		RedOn=SecsTowerLampOn(s_iSecsTowerRed, BlinkPhase);
	}

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
			//AI(secs-kyec-rcmd4) 20260728 : host PP_MUSIC override replaces the per-RunState
			//buzzer selection. Placement is load-bearing in two directions :
			//  INSIDE this bMaintAlone==false block, so while the Maintenance screen owns the
			//  panel alone its sbMusic test buttons (which drive HSys.SwPtr[idx].On() directly)
			//  are never fought by a host override; and
			//  OUTSIDE DriveSystemMusic(), which PlayMessageBuzzer / PlayAlarmBuzzer share -
			//  those are dialog FormShow kicks and must not inherit it.
			//Suppressed during an alarm Note for the same reason as the tower override above :
			//a stale host override must not re-sound an alarm the operator already silenced.
			//AI(secs-kyec-rcmd4-fix) 20260728 : TMyMessageBox is the OTHER surface that owns the
			//buzzer (it sets BuzzState=LED_Message above and pumps DoSystemMessage every 10 ms,
			//mymessbox.cpp Timer1). Without this second gate a host override replaced the message
			//tone and bypassed the box's own fBuzzerOff acknowledge, so the operator could not
			//silence it. Gate on both dialogs, matching the bMaintAlone test.
			if(s_bSecsMusicOverride && (fNote==NULL || fNote->fShow==false) &&
				(MyMessageBox==NULL || MyMessageBox->fShow==false))
				MusicSel=s_iSecsMusicClass;
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
	if(fMain!=NULL)
		fMain->RefreshModuleStatusGrid();   //AI(ht160s-status) 20260703 : Module Status sheet (throttled + visibility-gated inside)
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
	ReportSafeDoorChangeToSecs();   //AI(secs-ceid-align9045) 20260729 : CEID123 door-edge report (both directions, running or not)
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
	AmrInject.Reset();   //AI(ht160s-agv) 20260708 : any machine start clears AMR manual-inject test mode + latches (no leak into a real run)
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
	//AI(secs-kyec-rcmd4-fix) 20260728 : discard a pending One Cycle arm on a HARD stop.
	//Run_OneCycle is a latch that only the OneCycle-finish dispatcher clears, and that
	//dispatcher sits behind ProcessMotion's SystemStart==false early-out - so an arm that
	//never reached SortArm's idle rung would survive the stop, make every later host
	//ONE_CYCLE answer HCACK=4 "already armed", and silently turn the operator's next Start
	//into place-one-then-stop. A hard stop is an abort, so the arm is discarded with it.
	//DELIBERATELY NOT done in MachinePause : a pause is resumable and the operator expects
	//the armed cycle to finish when they resume.
	//AI(secs-msggap-fix) 20260729 : TWO defects in the discard as first written.
	//(1) NEVER discard while bCleanOut is latched. OneCycleCore deliberately accepts
	//    Run_CleanOut, so during a drain Run_OneCycle is the CARRIER of that drain : the
	//    finish dispatcher below restores Run_CleanOut from bCleanOut and continues WITHOUT
	//    stopping. Discarding the mode left bCleanOut=true with RunMode==Run_Normal, and
	//    ProcessMotion Layer 4 has no Run_Normal branch for a drain - so the drain was
	//    silently abandoned, EmitCleanOutOK and the AMR DoLotEndProcess never ran, the batch
	//    never ended, and a later operator HOME would see the still-latched bCleanOut and drag
	//    the machine back into Clean Out + SoftStop out of nowhere. Keep the arm in that case;
	//    the drain owns it.
	//(2) Discarding the MODE without clearing SortArm's bOneCycleFinish left half the state
	//    behind. That latch is set by SortArm one scan before DoAllProcess consumes it, and
	//    the only clearer is the dispatcher behind the SystemStart==false gate - so a stop in
	//    that window kept it true, and the next accepted ONE_CYCLE on a running machine would
	//    read the stale true and immediately emit an S6F11 CEID 41 for a cycle that never ran,
	//    plus a spurious SoftStop. Clear both halves together or neither.
	//The keep-vs-discard POLICY for the plain (non-drain) case is untouched here : that is the
	//behaviour decision recorded at the ProcessMotion falling-edge rung below, which keeps the
	//arm on the other 15 stop paths. This only makes the one path that discards do it wholly
	//and never do it to a drain.
	if(HSys.Sys.RunMode==Run_OneCycle && HSys.Sys.bCleanOut==false)
	{
		if(SortArmModule!=NULL)
			SortArmModule->ClearOneCycleFinish();
		RecordProcess("MACHINE STOP : pending One Cycle arm discarded");
		ChangeRunMode(Run_Normal);
	}
	else if(HSys.Sys.RunMode==Run_OneCycle)
		RecordProcess("MACHINE STOP : One Cycle arm KEPT (nested Clean Out drain still pending)");
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
	//AI(ht160s-pause-cylinder) 20260731 : this loop used to walk the GLOBAL Cylinder[] array
	//(mycylin.cpp:13). That array is never initialised and is referenced NOWHERE else in the
	//program - every real cylinder lives in HSys.Cyn, reached through CynPtr (database.cpp:767
	//CynPtr=(TMyCylinder *)&Cyn). So the pause froze 200 dead objects while every real cylinder
	//Delay kept running: a stroke parked mid-confirm (Task 50 waiting on a reed, or Task 101 in
	//its settle delay) either alarmed with zero grace on resume, or reported done off a timer
	//that expired while the machine stood still. The sucker loop below was always correct.
	if(HSys.CynPtr!=NULL)
	{
		for(i=0; i<HSys.iTotalCylinder; i++)
			HSys.CynPtr[i].Delay.Pause();
	}
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
	if(TrayArmModule!=NULL)
		TrayArmModule->PauseTimeoutTimers();   //AI(ht160s-rearready-p0) 20260705 : blocked-pick watchdog window
}
//---------------------------------------------------------------------------
static void ReStartActuatorTimeoutTimers()
{
	int i, r, c;
	//AI(ht160s-pause-cylinder) 20260731 : resume twin of PauseActuatorTimeoutTimers - see the
	//comment there for why the global Cylinder[] array was the wrong target.
	if(HSys.CynPtr!=NULL)
	{
		for(i=0; i<HSys.iTotalCylinder; i++)
			HSys.CynPtr[i].Delay.ReStart();
	}
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
	if(TrayArmModule!=NULL)
		TrayArmModule->ReStartTimeoutTimers();   //AI(ht160s-rearready-p0) 20260705 : blocked-pick watchdog window
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
		//AI(secs-audit-fix) 20260729 : make a surviving One Cycle arm diagnosable. MachineStop()
		//discards the arm, but MachineStop has exactly ONE caller in the tree (the S2F41 "STOP"
		//RCMD), so every other way the machine stops - alarm Note, TMyMessageBox, servo alarm,
		//panel POWER OFF, TrayArm fault, Lot End - leaves RunMode==Run_OneCycle latched on a
		//stopped machine. Nothing clears it there: ProcessMotion returns at the SystemStart gate
		//below so the OneCycle-finish dispatcher never runs, and DoStartArm only resets RunMode on
		//the UNHOMED branch. The host side is now truthful regardless (OneCycleCore answers
		//ocRejStopped -> HCACK=2 for any stopped machine, ahead of the already-armed test), but
		//the operator's NEXT Start still becomes place-one-then-stop. Whether that arm SHOULD
		//survive a fault stop is a machine-behaviour decision, not a code cleanup - keeping it is
		//safer for a deliberate single-step, discarding it is more predictable for a resume - so
		//this only records the fact and leaves the behaviour as-is.
		if(HSys.Sys.RunMode==Run_OneCycle)
			RecordProcess("MACHINE STOPPED with a pending One Cycle arm : arm KEPT (next Start runs one cycle then stops)");
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
            //AI(cleanout) 20260701 : pick the post-home run mode. A Start-triggered home
            //(bHomeByStart) resumes production. A home taken mid-Clean-Out (bCleanOut latched)
            //resumes Clean Out - mirroring the OneCycle-during-CleanOut resume in
            //CheckOneCycleFinish below - so the drain is carried to completion instead of
            //silently reverting to a Normal production run (was: unconditional Run_Normal).
            //InitialAllTask(true) reset the per-module clean-out finish flags, so the cascade
            //re-evaluates and finishes fast if the pipeline is already empty.
            bool bByStart=bHomeByStart;   //AI(ht160s-obsv-p0) : captured before the branch consumes it
            if(bHomeByStart)
            {
                ChangeRunMode(Run_Normal);                                      //20120102 Daver add
                bHomeByStart=false;
            }
            else if(HSys.Sys.bCleanOut)
            {
                ChangeRunMode(Run_CleanOut);
                SoftStop=true;
            }
            else
            {
                ChangeRunMode(Run_Normal);
                SoftStop=true;
            }
            //AI(ht160s-obsv-p0) 20260720 : resume finalize breadcrumb + post-resume baseline
            //snapshot. The 30-slot TaskHistory ring wraps within the resume burst, so the only
            //reconstructable baseline of "what the machine believed right after resume" is
            //captured HERE, before production restarts.
            RecordProcess(AnsiString("HOME-RESUME finalize: mode=")+
                (HSys.Sys.RunMode==Run_CleanOut?AnsiString("CleanOut"):AnsiString("Normal"))+
                " byStart="+IntToStr(bByStart?1:0));
            if(gStateRecord!=NULL)
                gStateRecord->TriggerSnapshot("HomeResumeDone");
            SetMotorSpeed(true);                                                //AI(HT160S-Maintainer) 20260602 : re-apply working speed after home (HT172 0420 csystem/uhome port)
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
		//AI(secs-onsite0731) 20260801 : latch the UPH numerator at the SAME instant the
		//denominator epoch is re-stamped. tRunData.TotalIC survives power cycles by design
		//(main.cpp ReadLastDataIni, "cumulative ... KEPT even on a fresh start (user choice)")
		//while StartTime does not, so without this the ratio mixes two different epochs - on
		//2026-07-31 UPH decayed 5131 -> 2305 while only 13 ICs were placed, and the 16:29 Lot
		//End reported 2026 against a true 99. Deliberately placed INSIDE this block, not in
		//the boot path, so the Run_TrayFeed finish below (which also sets bFirstRun=true
		//without zeroing TotalIC) is covered by the same latch.
		g_iUphBaseIC=tRunData.TotalIC;
	}

	if(HSys.Sys.RunMode==Run_CleanOut)
	{
		if(CheckCleanOutFinish())
		{
			//AI(ht160s-overcount-tripqueue D3/D6) 20260721 : emit CEID42 CleanOutFinish first
			//(host sees Clean Out done before Lot End), in BOTH modes (self-gates on SELECTED).
			if(fMain!=NULL) fMain->EmitCleanOutOK();
			if(GeneralSetting.bUseAMR)
			{
				//AI(secs-comment-truth) 20260805 : was "CEID12" - wrong since the CEID dictionary was
				//realigned to HT9045 (ab1b99e). Lot End is CEID 8; 12 is Switch Engineer Mode.
				//AMR mode : run the FULL Lot End automatically (no operator modal) -- CEID 8 +
				//UPH/Soter/lastdata + WhiteList revert + ArchiveWorkOrder + LotRegistry.Clear +
				//LotBinBinding.Clear, all inside DoLotEndProcess (reads live lot data, so call it
				//BEFORE InitialAllTask). Investigation confirmed CleanOut-finish is reachable
				//without an AMR car-unload, so no per-Auto unload handshake is required here (that
				//D4 automation -- AMR physically taking the output cars -- is a separate future step;
				//the output cars hold the sorted product for operator/AMR removal exactly as a manual
				//Lot End leaves them today). Non-AMR keeps the existing modal path below (D6).
				HSys.Sys.bCleanOut=false;
				if(fMain!=NULL) fMain->DoLotEndProcess();
				InitialAllTask();
			}
			else
			{
				tRunData.LotEndTime=Now();
				tRunData.UPH=GetCalculateUPH(tRunData.LotEndTime);
				if(fMain!=NULL) fMain->FreezeProductInfoAtLotEnd();
				g_SoterOutput.OnLotEnd();
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
			}
			ChangeRunMode(Run_Normal);
			SoftStop=true;
		}
	}
	else if(HSys.Sys.RunMode==Run_OneCycle)
	{
		if(CheckOneCycleFinish())
		{
			//AI(secs-kyec-rcmd4) 20260728 : emit S6F11 "One Cycle Finish" FIRST, mirroring the
			//CleanOut-finish path above (EmitCleanOutOK). The event was declared and registered
			//but sent from NOWHERE, so a host that drove S2F41 ONE_CYCLE and got HCACK=0 had no
			//way to learn the cycle actually finished. KYEC's HT9045 emits its equivalent and did
			//so 3x on 2026-06-08 - HCACK was 0 in all eleven ONE_CYCLE cases, so the host can
			//distinguish a real cycle ONLY by this finish event.
			//AI(secs-comment-truth) 20260805 : this comment said CEID 27 throughout. That was true
			//only before ab1b99e made HT160's CEID dictionary a verbatim copy of HT9045's: One Cycle
			//Finish is now CEID 41 (SECS_EVENT.OneCycleFinish), the SAME number 9045 uses, and 27 is
			//Change Machine State in BOTH dictionaries. See the caveat note below.
			//AI(secs-msggap-fix) 20260729 : recount of that correlation - the 11 host commands split
			//3 ACCEPTED / 8 SWALLOWED (not 2/9), and the three CEID 41s do NOT map 1:1 onto the
			//accepted commands : one (18:51:58) follows a LOCAL OneCycle press with the nearest host
			//command 9 min 43 s earlier, and one accepted command (19:00:19) armed and was then
			//stranded by a HALT 0.85 s later, so it never produced a 41 at all. That stranded arm is
			//the field evidence behind the SECS stale-arm guard in TfMain::OneCycleCore and the
			//discard in MachineStop.
			//AI(secs-comment-truth) 20260805 : caveat (1) is RESOLVED and has been deleted - it said
			//"27 means Change Machine State in KYEC's 9045 dictionary, so a host provisioned from
			//that dictionary MISREADS this event". Since ab1b99e we emit 41, the number 9045 itself
			//uses, so there is nothing to misread. ONE HOST-SIDE CAVEAT remains on CEID 41:
			//9045 also emits an S5F1 (ALID 316001640
			//"One cycle finish") alongside CEID 41 and HT160 does not, so a host keying off that
			//ALID sees nothing. It is declared in
			//docs/SECS/HT160S_SECS_Interface_Spec_20260727.md as customer-confirmation items.
			//EventReport self-gates on USE_SECS_GEM + HSMS SELECTED.
			if(fMain!=NULL) fMain->EmitOneCycleOK();
			//AI 20260721 : OneCycle finish. Freeze all modules (pause-like) and only re-arm
			//the SortArm one-shot finish latch -- was InitialAllTask(), a full per-module reset
			//(HOME-resume machinery : cursor + material wipe + AGV reassert) too heavy for a
			//single-cycle pause, and it wiped material OneCycle deliberately leaves under the
			//machine. HT172 likewise clears only its OneCycle latch here. Clearing the latch is
			//mandatory : else a later OneCycle press reads a stale true and finishes instantly.
			if(SortArmModule!=NULL)
				SortArmModule->ClearOneCycleFinish();
			//If OneCycle was launched mid-CleanOut, resume CleanOut and run to completion
			//WITHOUT stopping (no SoftStop : the nested-continuation intent the old copied
			//SoftStop=true contradicted); otherwise return to Normal and stop.
			if(HSys.Sys.bCleanOut)
            {
                ChangeRunMode(Run_CleanOut);
                RecordProcess("ONE CYCLE finish: resume CleanOut (no stop)");
            }
			else
			{
				ChangeRunMode(Run_Normal);
				RecordProcess("ONE CYCLE finish: back to Normal, stop");
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
	//AI(secs-onsite0731) 20260801 : count only the units placed since the epoch that
	//tRunData.StartTime marks (see the bFirstRun block in ProcessMotion). TotalIC itself
	//stays cumulative - the customer's HT9045 is cumulative too - so the baseline is
	//subtracted here rather than zeroing the counter.
	//AI(secs-66xxx-retire) 20260804 : TotalIC no longer has an SVID of its own (66020 retired);
	//it now feeds only the screen and the UPH derived here. The host reads 1101 / 1102.
	int iWindowIC=tRunData.TotalIC-g_iUphBaseIC;
	if(iWindowIC<0)
		iWindowIC=0;
	return (int)((double)iWindowIC*3600.0/dSeconds);
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