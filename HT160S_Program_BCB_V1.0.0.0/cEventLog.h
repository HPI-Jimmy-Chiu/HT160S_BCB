//---------------------------------------------------------------------------
// cEventLog : daily alarm / event CSV log under D:\HT160S_Log\EventLog
// Now a thin subclass of cCsvDailyLog (shared infrastructure). Faithful port
// of the HT172 TFormSysTools::RecordAlarmMessage -> EventLog CSV path.
//---------------------------------------------------------------------------
#ifndef cEventLogH
#define cEventLogH
//---------------------------------------------------------------------------
#include <vcl.h>
#include "cCsvDailyLog.h"
//---------------------------------------------------------------------------

class cEventLog : public cCsvDailyLog
{
public:
    void Init();
    void Log(const AnsiString& sAlarmCode,
             const AnsiString& sMessage,
             const AnsiString& sErrorPart = "");
    void LogRecovery(const AnsiString& sRecovery,
                     int iPauseTimeSec,
                     const AnsiString& sAlarmCode,
                     const AnsiString& sMessage);
};

//---------------------------------------------------------------------------
extern cEventLog g_EventLog;
//---------------------------------------------------------------------------
#endif
