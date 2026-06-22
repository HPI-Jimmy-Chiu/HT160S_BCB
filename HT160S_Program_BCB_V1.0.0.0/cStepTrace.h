//---------------------------------------------------------------------------
// cStepTrace : numeric per-module step (Task) trace recorder (no FSM).
//
// Each module's "current step" is the numeric Task stored in its UserMotion
// action Tag (see database.cpp DoAllProcess: DoLoader(1, P->Tag)).  This tool
// snapshots all 7 action Tags + RunMode every cycle and writes one CSV row
// whenever any value changes, giving a continuous "step movie" of the machine
// without touching any control logic.
//
// Enable at runtime WITHOUT recompiling by creating the flag file:
//     D:\HT160S_Log\steptrace.on
// Output CSV (UTF-8) is written to:
//     D:\HT160S_Log\StepTrace\steptrace_YYYYMMDD_HHMMSS.csv
// Delete the flag file to stop; tracing re-checks the flag periodically.
//---------------------------------------------------------------------------
#ifndef cStepTraceH
#define cStepTraceH
//---------------------------------------------------------------------------
#include <vcl.h>
//---------------------------------------------------------------------------
// Call once per main cycle (e.g. end of DataModule1::DoAllProcess()).
// Cheap no-op when tracing is disabled.
void StepTraceTick();
//---------------------------------------------------------------------------
// Single-axis Home / motion task trace for Teach + Motor Test diagnosis.
// One-shot capture written through the shared cCsvDailyLog daily-CSV channel:
//     D:\HT160S_Log\MotorTaskLog\YYYYMMDD\MotorTask_YYYYMMDD.csv
// Active while a Teach / Motor Test session is open (call MotorTaskLogSetActive
// on form show/close) OR whenever the flag file D:\HT160S_Log\motortask.on
// exists. Cheap no-op when inactive. Each call appends one row; callers log on
// task transitions (not every tick) so the file stays a readable "step movie".
void MotorTaskLogSetActive(bool bActive);
bool MotorTaskLogActive();
void MotorTaskLog(const AnsiString& sSource, const AnsiString& sMotor,
                  const AnsiString& sEvent,  const AnsiString& sDetail);
//---------------------------------------------------------------------------
#endif
