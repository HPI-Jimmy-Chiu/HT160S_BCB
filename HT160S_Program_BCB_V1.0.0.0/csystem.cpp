//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "csystem.h"
#include "ComPort.h"
#include "cmydef.h"
#include "iosetview.h"
#include "main.h"
#include "maintenance.h"
#include "uruncontrol.h"
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
}
//---------------------------------------------------------------------------
void MainProc()
{
	static bool bProgramStart=false;

	ProcessStartMode();
	DoSystem();
	SpinComPort();
	ProcessRunStatus(bProgramStart);

	if(fiosetview!=NULL && fiosetview->Visible)
	{
		UpdateRunControlFlag();
		return;
	}

	if(bProgramStart==false)
		bProgramStart=DoInitialProgramStart();
	else
		ProcessMotion();

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
		if(HSys.MotPtr[i]->Led[iAlarmLed] && HSys.MotPtr[i]->GetEnable() &&
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
void InitialAllTask()
{
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
			Task=200;
			break;
		case 200:
			return true;
	}
	return false;
}
//---------------------------------------------------------------------------
void ProcessMotion()
{
	if(HSys.Sys.SystemStart)
	{
		if(CheckMotorHome()==false)
		{
			ChangeRunMode(Run_Home);
			return;
		}
		fAllMotorHome=true;
		ChangeRunMode(Run_Normal);
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
	return true;
}
//---------------------------------------------------------------------------
bool CheckOneCycleFinish()
{
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
	return true;
}
//---------------------------------------------------------------------------
void ChangeRunMode(RunModeEnum RunMode)
{
	HSys.Sys.RunMode=RunMode;
}
//---------------------------------------------------------------------------
int GetCalculateUPH(TDateTime tEndTime)
{
	return 0;
}
//---------------------------------------------------------------------------
void RecordSafeDoorStates()
{
}
//---------------------------------------------------------------------------