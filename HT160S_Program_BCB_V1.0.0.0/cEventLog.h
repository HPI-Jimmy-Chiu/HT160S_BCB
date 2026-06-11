//---------------------------------------------------------------------------
// cEventLog : daily alarm / event CSV log under D:\HT160S_Log\EventLog
// Base directory comes from HSys.LogRootDir (set at Init time).
//---------------------------------------------------------------------------
#ifndef cEventLogH
#define cEventLogH
//---------------------------------------------------------------------------
#include <vcl.h>
#include <SyncObjs.hpp>
//---------------------------------------------------------------------------

class cEventLog
{
private:
    TCriticalSection* m_pCS;
    AnsiString m_sBaseDir;      // e.g. "D:\\HT160S_Log\\EventLog"
    AnsiString m_sLastFilePath; // cached daily path
    AnsiString m_sLastDate;     // "YYYY_MM_DD" of cached path

    AnsiString GetLogFilePath();
    void EnsureHeader(const AnsiString& sPath);
    void AppendLine(const AnsiString& sLine);

public:
    cEventLog();
    ~cEventLog();

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
