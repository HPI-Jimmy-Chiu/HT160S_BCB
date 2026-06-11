//---------------------------------------------------------------------------
// cSelfCheck : startup wiring self-check (no FSM).
// Verifies that all module pointers, the DataModule action list, and every
// UserMotion action OnExecute binding are wired before the machine runs.
// Mirrors the "[WIRING CHECK]" concept: name every "wire" at power-on so a
// missing binding aborts startup instead of failing silently mid-cycle.
//---------------------------------------------------------------------------
#ifndef cSelfCheckH
#define cSelfCheckH
//---------------------------------------------------------------------------
#include <vcl.h>
//---------------------------------------------------------------------------
// Runs the wiring self-check.
// Returns true when every checked binding is present.
// On return, Report contains one line per checked item ("OK" / "X NULL ..."),
// suitable for a message box or log.
bool ValidateWiring(AnsiString &Report);
//---------------------------------------------------------------------------
#endif
