//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "cCommLog.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

// One instance per serial channel. Init() is called once at startup.
cCommLog g_PadCommLog;
cCommLog g_BinDispCommLog;

//---------------------------------------------------------------------------
void cCommLog::Init(const AnsiString& sName)
{
    // sName is both the sub-folder and the daily file prefix.
    InitLog(sName, sName, "Date,Time,Action,Message");
}

//---------------------------------------------------------------------------
void cCommLog::Log(const AnsiString& sAction, const AnsiString& sMessage)
{
    // One line per call: Date,Time,Action,"Message".
    TDateTime now = Now();
    AnsiString sDate = FormatDateTime("yyyy/mm/dd", now);
    AnsiString sTime = FormatDateTime("hh:nn:ss.zzz", now);
    AnsiString sLine = sDate + "," + sTime + "," + sAction + "," + CsvQuote(sMessage);
    AppendLine(sLine);
}
//---------------------------------------------------------------------------
