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
void DoSystemMessage();                  //AI(ht160s-maintainer) 20260627 : exposed so the modal Note Timer1 can re-drive tower light + buzzer + panel recovery LEDs while MainProc is suspended (HT172 note Timer1 -> DoSystemMessage parity)
//AI(HT160S-Maintainer) 20260622 : immediate FormShow buzzer kicks for the modal dialogs
//(MyMessageBox -> PlayMessageBuzzer, fNote -> PlayAlarmBuzzer). A modal ShowModal suspends
//MainProc, so the per-scan DoSystemMessage buzzer driver never runs while the dialog is up;
//these sound the same configured "Music Select" the instant the dialog appears.
void PlayMessageBuzzer();
void PlayAlarmBuzzer();
//AI(secs-kyec-rcmd4) 20260728 : SECS host panel override (S2F41 PP_SIGNALTOWER / PP_MUSIC).
//A latched host override of the per-RunState tower-lamp + buzzer table in DoSystemMessage.
//State is file-static in csystem.cpp and reachable ONLY through these accessors, so the
//SECS layer cannot touch the raw flags and every release path is greppable by name.
//Colour domain 0=off / 1=on / 2=blink; pass -1 for a colour the host did not name (keeps
//its previous value). Music class 1..4 -> SwMusic1..SwMusic4.
//ClearSecsPanelOverride() is the operator escape and releases BOTH at once.
void SetSecsTowerOverride(int Red, int Yellow, int Green);
void ClearSecsTowerOverride();
void SetSecsMusicOverride(int MusicClass);
void ClearSecsMusicOverride();
void ClearSecsPanelOverride();
bool IsSecsPanelOverrideActive();
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
//AI(machine-command-layer) 20260625 : single production-start gate (joins Pause/Stop/
//HomeAbort). Operator button, SECS START and the panel key all funnel through
//MachineStart so the lot/2D precondition gate + the SystemStart raise live in ONE place
//(manual == auto). Returns a result; caller maps it (operator -> ShowMyMessage,
//SECS -> HCACK). No modal here.
enum eMachineStartResult {
    msStarted,
    msRejNoContext,
    msRejBusy,
    msRejNotReady
};
eMachineStartResult MachineStart(eMachineTrigger trig, AnsiString &Reason);
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