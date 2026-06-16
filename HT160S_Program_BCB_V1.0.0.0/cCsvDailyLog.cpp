//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "cCsvDailyLog.h"
#include "database.h"
#include <stdio.h>
#include <FileCtrl.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)

//---------------------------------------------------------------------------
cCsvDailyLog::cCsvDailyLog()
    : m_pCS(NULL)
    , m_sBaseDir("")
    , m_sFilePrefix("")
    , m_sHeader("")
    , m_sExt(".csv")
    , m_eGran(lgMonthlyFolder)
    , m_sLastFilePath("")
    , m_sLastDate("")
{
}

//---------------------------------------------------------------------------
cCsvDailyLog::~cCsvDailyLog()
{
    delete m_pCS;
    m_pCS = NULL;
}

//---------------------------------------------------------------------------
void cCsvDailyLog::InitLog(const AnsiString& sSubFolder,
                           const AnsiString& sFilePrefix,
                           const AnsiString& sHeader,
                           eLogGranularity eGran,
                           const AnsiString& sExt)
{
    if (!m_pCS)
        m_pCS = new TCriticalSection();

    // Central log root constant (HSys.LogRootDir = "D:\\HT160S_Log")
    m_sBaseDir    = HSys.LogRootDir + "\\" + sSubFolder;
    m_sFilePrefix = sFilePrefix;
    m_sHeader     = sHeader;
    m_eGran       = eGran;
    m_sExt        = sExt;

    // Invalidate cached daily path (in case of re-init).
    m_sLastDate     = "";
    m_sLastFilePath = "";

    // Ensure base directory exists
    ForceDirectories(m_sBaseDir);
}

//---------------------------------------------------------------------------
AnsiString cCsvDailyLog::GetLogFilePath()
{
    // Monthly: <base>\YYYY_MM\<prefix>_YYYY_MM_DD<ext>
    // Daily  : <base>\YYYYMMDD\<prefix>_YYYYMMDD<ext>
    TDateTime now = Now();
    AnsiString sDayKey = FormatDateTime("yyyy_mm_dd", now);  // day-change detector

    // Return cached path if same day
    if (sDayKey == m_sLastDate && !m_sLastFilePath.IsEmpty())
        return m_sLastFilePath;

    AnsiString sDir;
    AnsiString sFileStamp;
    if (m_eGran == lgDailyFolder)
    {
        AnsiString sDay = FormatDateTime("yyyymmdd", now);
        sDir = m_sBaseDir + "\\" + sDay;
        sFileStamp = sDay;
    }
    else
    {
        sDir = m_sBaseDir + "\\" + FormatDateTime("yyyy_mm", now);
        sFileStamp = sDayKey;
    }
    ForceDirectories(sDir);

    m_sLastFilePath = sDir + "\\" + m_sFilePrefix + "_" + sFileStamp + m_sExt;
    m_sLastDate = sDayKey;
    return m_sLastFilePath;
}

//---------------------------------------------------------------------------
void cCsvDailyLog::EnsureHeader(const AnsiString& sPath)
{
    // Empty header -> plain channel (e.g. .log); the append fopen creates the file.
    if (m_sHeader.IsEmpty())
        return;
    if (FileExists(sPath))
        return;

    // Write CSV header row on first write of the day
    FILE* fp = fopen(sPath.c_str(), "w");
    if (fp)
    {
        fprintf(fp, "%s\n", m_sHeader.c_str());
        fclose(fp);
    }
}

//---------------------------------------------------------------------------
void cCsvDailyLog::AppendLine(const AnsiString& sLine)
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
AnsiString cCsvDailyLog::CsvQuote(const AnsiString& sText)
{
    AnsiString s = sText;
    s = StringReplace(s, "\"", "\"\"", TReplaceFlags() << rfReplaceAll);
    return AnsiString("\"") + s + AnsiString("\"");
}
//---------------------------------------------------------------------------
