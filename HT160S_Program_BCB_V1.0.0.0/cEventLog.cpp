//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "cEventLog.h"
#include "database.h"
#include <stdio.h>
#include <FileCtrl.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)

cEventLog g_EventLog;

//---------------------------------------------------------------------------
cEventLog::cEventLog()
    : m_pCS(NULL)
    , m_sBaseDir("")
    , m_sLastFilePath("")
    , m_sLastDate("")
{
}

//---------------------------------------------------------------------------
cEventLog::~cEventLog()
{
    delete m_pCS;
    m_pCS = NULL;
}

//---------------------------------------------------------------------------
void cEventLog::Init()
{
    if (!m_pCS)
        m_pCS = new TCriticalSection();

    // Central log root constant (HSys.LogRootDir = "D:\\HT160S_Log")
    m_sBaseDir = HSys.LogRootDir + "\\EventLog";

    // Ensure base directory exists
    ForceDirectories(m_sBaseDir);
}

//---------------------------------------------------------------------------
AnsiString cEventLog::GetLogFilePath()
{
    // Format: D:\HT160S_Log\EventLog\YYYY_MM\HT160S_YYYY_MM_DD.csv
    TDateTime now = Now();
    AnsiString sDate = FormatDateTime("yyyy_mm_dd", now);

    // Return cached path if same day
    if (sDate == m_sLastDate && !m_sLastFilePath.IsEmpty())
        return m_sLastFilePath;

    AnsiString sMonth = FormatDateTime("yyyy_mm", now);
    AnsiString sDir = m_sBaseDir + "\\" + sMonth;
    ForceDirectories(sDir);

    m_sLastFilePath = sDir + "\\HT160S_" + sDate + ".csv";
    m_sLastDate = sDate;
    return m_sLastFilePath;
}

//---------------------------------------------------------------------------
void cEventLog::EnsureHeader(const AnsiString& sPath)
{
    if (FileExists(sPath))
        return;

    // Write CSV header row on first write of the day
    FILE* fp = fopen(sPath.c_str(), "w");
    if (fp)
    {
        fprintf(fp, "Date,Time,Recovery,PauseTime,Duplicate,AlarmCode,Message,ErrorPart\n");
        fclose(fp);
    }
}

//---------------------------------------------------------------------------
void cEventLog::AppendLine(const AnsiString& sLine)
{
    if (!m_pCS)
        return;

    m_pCS->Acquire();
    try
    {
        AnsiString sPath = GetLogFilePath();
        EnsureHeader(sPath);

        FILE* fp = fopen(sPath.c_str(), "a");
        if (fp)
        {
            fprintf(fp, "%s\n", sLine.c_str());
            fclose(fp);
        }
    }
    __finally
    {
        m_pCS->Release();
    }
}

//---------------------------------------------------------------------------
void cEventLog::Log(const AnsiString& sAlarmCode,
                    const AnsiString& sMessage,
                    const AnsiString& sErrorPart)
{
    // CSV: Date,Time,Recovery,PauseTime,Duplicate,AlarmCode,Message,ErrorPart
    TDateTime now = Now();
    AnsiString sDate = FormatDateTime("yyyy", now) + "\\"
                     + FormatDateTime("mm", now)   + "\\"
                     + FormatDateTime("dd", now);
    AnsiString sTime = FormatDateTime("hh:nn:ss", now);

    AnsiString sLine;
    sLine = sDate + "," + sTime + ",,"
            + "0,,"
            + sAlarmCode + ","
            + sMessage + ","
            + sErrorPart;
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
    AnsiString sDate = FormatDateTime("yyyy", now) + "\\"
                     + FormatDateTime("mm", now)   + "\\"
                     + FormatDateTime("dd", now);
    AnsiString sTime = FormatDateTime("hh:nn:ss", now);

    AnsiString sLine;
    sLine = sDate + "," + sTime + ","
            + sRecovery + ","
            + IntToStr(iPauseTimeSec) + ",,"
            + sAlarmCode + ","
            + sMessage + ",";
    AppendLine(sLine);
}
//---------------------------------------------------------------------------
