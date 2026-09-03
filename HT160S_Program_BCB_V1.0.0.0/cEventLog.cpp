//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "cEventLog.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

cEventLog g_EventLog;

//---------------------------------------------------------------------------
void cEventLog::Init()
{
    InitLog("EventLog", "HT160S",
            "Date,Time,Recovery,PauseTime,Duplicate,AlarmCode,Message,ErrorPart");
}

//---------------------------------------------------------------------------
void cEventLog::Log(const AnsiString& sAlarmCode,
                    const AnsiString& sMessage,
                    const AnsiString& sErrorPart)
{
    // CSV: Date,Time,Recovery,PauseTime,Duplicate,AlarmCode,Message,ErrorPart
    TDateTime now = Now();
    AnsiString sDate = FormatDateTime("yyyy/mm/dd", now);
    AnsiString sTime = FormatDateTime("hh:nn:ss.zzz", now);   //AI(ht160s-obsv-p1) : ms resolution (resume bursts are sub-second)

    AnsiString sLine;
    sLine = sDate + "," + sTime + ",,"
            + "0,,"
            + sAlarmCode + ","
            + CsvQuote(sMessage) + ","
            + CsvQuote(sErrorPart);
    AppendLine(sLine);
}

//---------------------------------------------------------------------------
void cEventLog::LogRecovery(const AnsiString& sRecovery,
                            int iPauseTimeSec,
                            const AnsiString& sAlarmCode,
                            const AnsiString& sMessage)
{
    // CSV: Date,Time,Recovery,PauseTime,Duplicate,AlarmCode,Message,ErrorPart
    TDateTime now = Now();
    AnsiString sDate = FormatDateTime("yyyy/mm/dd", now);
    AnsiString sTime = FormatDateTime("hh:nn:ss.zzz", now);   //AI(ht160s-obsv-p1) : ms resolution

    AnsiString sLine;
    sLine = sDate + "," + sTime + ","
            + sRecovery + ","
            + IntToStr(iPauseTimeSec) + ",,"
            + sAlarmCode + ","
            + CsvQuote(sMessage) + ",";
    AppendLine(sLine);
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void LogLadderFault(const AnsiString& sLadder, int iState)
{
    //AI(ht160s-ladder-guard) 20260703 : a state cursor reached a value with no matching
    //case. Record it (WAR_LADDER) so the silent 'number with no action' stall becomes a
    //diagnosable EventLog line; the caller's default resets the cursor to restart.
    g_EventLog.Log("WAR_LADDER",
                   AnsiString("Unexpected ladder state ")+sLadder+" Task="+IntToStr(iState), "");
}
