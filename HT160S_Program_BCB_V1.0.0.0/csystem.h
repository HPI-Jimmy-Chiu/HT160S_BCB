//---------------------------------------------------------------------------
#ifndef csystemH
#define csystemH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include "database.h"
#include "halarm.h"                       //AI(HT160S-Maintainer) 20260603 : central alarm dispatch (HAlarm raise-hand queue)
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260603 : global central alarm object, aligned with HT172 main.cpp
extern HAlarm *Alarm;
//---------------------------------------------------------------------------
void MainProc();
void ProcessAlarm();                     //AI(HT160S-Maintainer) 20260603 : drain HAlarm queue and dispatch via ShowSystemError
void AddNoNeedHomeSensorList();
void DoTemptureControl();
void ProcessStartMode();
void DoSystem();
//AI(HT160S-Maintainer) 20260622 : immediate FormShow buzzer kicks for the modal dialogs
//(MyMessageBox -> PlayMessageBuzzer, fNote -> PlayAlarmBuzzer). A modal ShowModal suspends
//MainProc, so the per-scan DoSystemMessage buzzer driver never runs while the dialog is up;
//these sound the same configured "Music Select" the instant the dialog appears.
void PlayMessageBuzzer();
void PlayAlarmBuzzer();
void ProcessRunStatus(bool bProgramStart=false);
void ProcessMotion();
bool DoInitialProgramStart();
bool CheckMotorHome();
void ArmMotorHome();
bool CountMotorPowerDelay();
int IsEMGPressed();
int IsSafeDoorOpen();
int IsIonFanAlarm();
bool IsSafeLock();
bool IsSystemPowerOff();
bool IsAirCheck();
bool AllBreakFree();
void AllBreakLock();
void InitialAllModule();
bool CheckCleanOutFinish();
bool CheckOneCycleFinish();
void CCDLoadJobs();
bool HasICUnderMachine();
bool HasICUnderFrontMachine();
bool HasICUnderMachineForCleanOut();
bool CheckEmpty1TrayFeedFinish();
bool CheckAllTrayFeedFinish(bool reset=false);
void ChangeRunMode(RunModeEnum RunMode);
int GetCalculateUPH(TDateTime tEndTime);
bool HasAutoICInMachine();
void InitialAllTask(bool bKeepMaterial=false);
void ScanAllMotorStatus();
void RecordSafeDoorStates();
//AI 20260619 : machine run-state command layer (see csystem.cpp). Pass the
//trigger source so every Pause/Stop/home-abort is logged with WHO did it.
//Single choke point for HSys.Sys.SystemStart + its mandatory motor-stop, so the
//invariant "SystemStart=false always stops the motors" holds in ONE place.
enum eMachineTrigger {
    trigOperator,
    trigSafetyDoor,
    trigEmg,
    trigServoAlarm,
    trigHomeStop,
    trigSecsRemote,
    trigSystem
};
const char* MachineTriggerName(eMachineTrigger trig);
void MachinePause(eMachineTrigger trig);
void MachineStop(eMachineTrigger trig);
void MachineHomeAbort(eMachineTrigger trig);
#ifdef SOFT_SIMULATE
//AI(HT160S-Maintainer) 20260619 : --selftest-home headless self-test (sim only).
//g_SelfTestHome is set in WinMain from the --selftest-home command-line arg; MainProc
//then auto-runs ONE full-machine home and sets g_SelfTestExitCode (0=home completed,
//2=timeout) before Application->Terminate(). WinMain returns that code.
extern bool g_SelfTestHome;
extern int  g_SelfTestExitCode;
#endif
//---------------------------------------------------------------------------
#endif