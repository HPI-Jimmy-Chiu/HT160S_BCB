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
void InitialAllTask();
void ScanAllMotorStatus();
void RecordSafeDoorStates();
//---------------------------------------------------------------------------
#endif