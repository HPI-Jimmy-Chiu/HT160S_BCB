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
    , m_nRetentionDays(0)
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

    // A new day (or first write of the session) just rolled over: drop any
    // sub-folders that have aged past the retention window. Cheap no-op when
    // retention is disabled.
    PruneOldFolders();
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
void cCsvDailyLog::SetRetentionDays(int nDays)
{
    m_nRetentionDays = (nDays > 0) ? nDays : 0;
    // Clean up immediately so a freshly-configured retention takes effect even
    // if this channel does not log again for a while (e.g. MotorTaskLog).
    if (m_nRetentionDays > 0 && !m_sBaseDir.IsEmpty())
        PruneOldFolders();
}

//---------------------------------------------------------------------------
// Parse a log sub-folder name back to the LAST calendar day it can contain, so
// a folder is only deleted once its entire span is past the cutoff.
//   daily   folder "YYYYMMDD" -> that day
//   monthly folder "YYYY_MM"  -> last day of that month
// Returns false for any unrecognized name (those are left untouched, by design).
static bool ParseFolderEndDate(const AnsiString& sName, TDateTime& dtOut)
{
    int nLen = sName.Length();
    int i;

    // Daily: 8 digits.
    if (nLen == 8)
    {
        for (i = 1; i <= 8; i++)
            if (sName[i] < '0' || sName[i] > '9')
                return false;
        int y = atoi(sName.SubString(1, 4).c_str());
        int m = atoi(sName.SubString(5, 2).c_str());
        int d = atoi(sName.SubString(7, 2).c_str());
        try { dtOut = EncodeDate(y, m, d); }
        catch (...) { return false; }
        return true;
    }

    // Monthly: "YYYY_MM".
    if (nLen == 7 && sName[5] == '_')
    {
        for (i = 1; i <= 7; i++)
            if (i != 5 && (sName[i] < '0' || sName[i] > '9'))
                return false;
        int y = atoi(sName.SubString(1, 4).c_str());
        int m = atoi(sName.SubString(6, 2).c_str());
        // Last day of the month = first day of next month - 1.
        int ny = (m == 12) ? y + 1 : y;
        int nm = (m == 12) ? 1 : m + 1;
        try { dtOut = EncodeDate(ny, nm, 1) - 1; }
        catch (...) { return false; }
        return true;
    }

    return false;
}

//---------------------------------------------------------------------------
// Recursively delete a folder and its contents. Log folders are normally flat
// (only daily files), but recurse defensively.
static void DeleteFolderRecursive(const AnsiString& sDir)
{
    TSearchRec sr;
    if (FindFirst(sDir + "\\*", faAnyFile, sr) == 0)
    {
        do
        {
            if (sr.Name == "." || sr.Name == "..")
                continue;
            AnsiString sFull = sDir + "\\" + sr.Name;
            if (sr.Attr & faDirectory)
                DeleteFolderRecursive(sFull);
            else
                DeleteFile(sFull);
        }
        while (FindNext(sr) == 0);
        FindClose(sr);
    }
    RemoveDir(sDir);
}

//---------------------------------------------------------------------------
void cCsvDailyLog::PruneOldFolders()
{
    if (m_nRetentionDays <= 0 || m_sBaseDir.IsEmpty())
        return;
    PruneFolderTree(m_sBaseDir, m_nRetentionDays);
}

//---------------------------------------------------------------------------
// Static reuse of the folder-aging policy for any base dir (e.g. the LotStory
// Discarded work-order backups, which write their own JSON files but want the
// same yyyy_mm folder retention). Unrecognized folder names are left untouched.
void cCsvDailyLog::PruneFolderTree(const AnsiString& sBaseDir, int nRetentionDays)
{
    if (nRetentionDays <= 0 || sBaseDir.IsEmpty())
        return;

    TDateTime dtCutoff = Date() - nRetentionDays;

    TSearchRec sr;
    if (FindFirst(sBaseDir + "\\*", faDirectory, sr) != 0)
        return;
    do
    {
        if ((sr.Attr & faDirectory) == 0)
            continue;
        if (sr.Name == "." || sr.Name == "..")
            continue;

        TDateTime dtFolderEnd;
        if (ParseFolderEndDate(sr.Name, dtFolderEnd) == false)
            continue;   // unrecognized name -> never delete (safety)

        if (dtFolderEnd < dtCutoff)
            DeleteFolderRecursive(sBaseDir + "\\" + sr.Name);
    }
    while (FindNext(sr) == 0);
    FindClose(sr);
}

//---------------------------------------------------------------------------
AnsiString cCsvDailyLog::CsvQuote(const AnsiString& sText)
{
    AnsiString s = sText;
    s = StringReplace(s, "\"", "\"\"", TReplaceFlags() << rfReplaceAll);
    return AnsiString("\"") + s + AnsiString("\"");
}
//---------------------------------------------------------------------------
