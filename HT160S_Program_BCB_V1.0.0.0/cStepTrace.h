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
#endif
