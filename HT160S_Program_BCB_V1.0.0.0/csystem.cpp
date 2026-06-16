//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

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
#include "cprod.h"
#include "uHome.h"
#include "uspeed.h"                     //AI(HT160S-Maintainer) 20260602 : SetMotorSpeed / LoadMotorSpeedFromIni (Speed module port)
#include "note.h"                       //AI(HT160S-Maintainer) 20260603 : ShowSystemError for ProcessAlarm dispatch
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
void MainProc()
{
	static bool bProgramStart=false;

	ProcessStartMode();
	DoSystem();
	ProcessRunStatus(bProgramStart);

	if(fMain!=NULL)
		fMain->SetSimulateScreenStatus();

	if(fMain!=NULL)
		fMain->ShowMotorInfo();

	//AI(ht160s-lot-webapi) 20260612 : Stage 4 : drive any in-flight Lot WebAPI pull.
	// MainProc runs on the VCL main thread (TRunControl::Synchronize), so this is a
	// safe place to consume the async response and reproject the Lot list. No-op when idle.
	if(fMain!=NULL)
		fMain->PollLotDataWebApi();

	if(fiosetview!=NULL && fiosetview->Visible)
	{
		UpdateRunControlFlag();
		return;
	}

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
		if(DataModule1!=NULL && bSortArmNeedHome==false)
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
int IsIonFanAlarm()
{
#ifdef SOFT_SIMULATE
	return 0;
#else
	int Index=0;
	Index=GetSensorOffIndex(&HSys.Sen.SnIonFan_Power);
	if(Index>0) return Index;
	Index=GetSensorOffIndex(&HSys.Sen.SnIonFan_Balance);
	return Index;
#endif
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
		if(bHomePowerCycling==false &&
		   HSys.MotPtr[i]->Led[iAlarmLed] && HSys.MotPtr[i]->GetEnable() &&
		   HSys.MotPtr[i]->ReadServoAlarmOn())
		{
			HSys.MotPtr[i]->bHomeFlag=false;
			fAllMotorHome=false;
			HSys.Sys.SystemStart=false;
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

	bool bOn=false;
	bool bOff=false;

	if(HSys.Sen.SnFKPowerOn.Enable)
		bOn|=HSys.Sen.SnFKPowerOn.IsOn();
	if(HSys.Sen.SnRKPowerOn.Enable)
		bOn|=HSys.Sen.SnRKPowerOn.IsOn();
	if(HSys.Sen.SnFKPowerOff.Enable)
		bOff|=HSys.Sen.SnFKPowerOff.IsOn();
	if(HSys.Sen.SnRKPowerOff.Enable)
		bOff|=HSys.Sen.SnRKPowerOff.IsOn();

	if(bOff && bOn==false)
	{
		HSys.DecStopAllMotor();
		AllBreakLock();
		HSys.Sw.SwMotorRelay.Off();
		bMotorPowerState=false;
		HSys.Sys.SystemStart=false;
		fAllMotorHome=false;
	}
	else if(bMotorPowerState==false && bOn)
	{
		HSys.Sw.SwMotorRelay.On();
		MotorPowerOnDelay=SERVER_MOTOR_POWER_ON_DELAY;
		bMotorPowerState=true;
	}
#endif
}
//---------------------------------------------------------------------------
void ScanSystemSenser()
{
	int SafeDoor=IsSafeDoorOpen();
	int Emg=IsEMGPressed();
	int IonFan=IsIonFanAlarm();

	if(MotorPowerOnDelay==0)
	{
		if(Emg>0)
		{
			HSys.StopAllMotor();
			AllBreakLock();
			HSys.Sys.SystemStart=false;
			fAllMotorHome=false;
			MotorPowerOnDelay=SERVER_MOTOR_POWER_ON_DELAY;
		}
		else if(IsSystemPowerOff())
		{
			HSys.StopAllMotor();
			AllBreakLock();
			HSys.Sys.SystemStart=false;
			fAllMotorHome=false;
			MotorPowerOnDelay=SERVER_MOTOR_POWER_ON_DELAY;
		}
	}

	if(HSys.Sys.SystemStart)
	{
		if(SafeDoor>0)
		{
			HSys.StopAllMotor();
			HSys.Sys.SystemStart=false;
		}
		else if(Emg>0)
		{
			HSys.StopAllMotor();
			AllBreakLock();
			HSys.Sys.SystemStart=false;
			fAllMotorHome=false;
		}
		else if(IonFan>0)
		{
			HSys.DecStopAllMotor();
			HSys.Sys.SystemStart=false;
		}
		else if(IsAirCheck())
		{
			HSys.DecStopAllMotor();
			HSys.Sys.SystemStart=false;
		}
		else if(MotorPowerOnDelay>0)
		{
			HSys.Sys.SystemStart=false;
		}
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
}
//---------------------------------------------------------------------------
void DoSystem()
{
	ScanAllMotorStatus();
	ScanSystemSenser();
	DoSystemMessage();
	if(HSys.Sys.SystemStart==false)
		CheckMotorPowerShutDown();
	if(CountMotorPowerDelay()==false)
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
        }
        else if(HSys.Sys.RunMode==Run_TrayFeed)
        {
            Status="Tray Feed";
			Color=clYellow;
        }
        else if(HSys.Sys.RunMode==Run_OneCycle)
        {
            Status="One Cycle";
			Color=clYellow;
        }
		else
		{
			Status="RUNNING";
			Color=clGreen;
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
		}
	}
	bPrevSystemStart=HSys.Sys.SystemStart;

	if(HSys.Sys.SystemStart==false)
		return;

	//Resume edge: accumulate the elapsed pause once, on the cycle we resume.
	if(bCalculatePauseTime)
	{
		tUPH_PauseTime=tUPH_PauseTime+(Now()-tUPH_PauseStartTime);
		bCalculatePauseTime=false;
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
	if(LoaderModule!=NULL && LoaderModule->IsAllCleanOutFinish()==false)
		return false;
	if(SortArmModule!=NULL && SortArmModule->IsCleanOutFinish()==false)
		return false;
	if(AutoModule!=NULL && AutoModule->IsAllCleanOutFinish()==false)
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
}
//---------------------------------------------------------------------------