//---------------------------------------------------------------------------
#ifndef csystemH
#define csystemH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include "database.h"
//---------------------------------------------------------------------------
void MainProc();
void AddNoNeedHomeSensorList();
void DoTemptureControl();
void ProcessStartMode();
void DoSystem();
void ProcessRunStatus(bool bProgramStart=false);
void ProcessMotion();
bool DoInitialProgramStart();
bool CheckMotorHome();
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