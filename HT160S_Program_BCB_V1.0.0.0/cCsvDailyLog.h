//---------------------------------------------------------------------------
// cCsvDailyLog : shared base for per-channel daily CSV logs.
// Owns the common infrastructure that cEventLog and cCommLog used to each
// copy: a per-instance critical section, monthly-folder + daily-file path
// rollover under HSys.LogRootDir, first-write header, append, and CSV quoting.
//   monthly folder (default): <LogRootDir>\<SubFolder>\YYYY_MM\<FilePrefix>_YYYY_MM_DD<ext>
//   daily folder            : <LogRootDir>\<SubFolder>\YYYYMMDD\<FilePrefix>_YYYYMMDD<ext>
// Subclasses (or direct users) call InitLog() once, then compose each data line
// (date/time + columns) themselves and call AppendLine().
// An empty header string means "no header row" (e.g. plain .log channels).
//---------------------------------------------------------------------------
#ifndef cCsvDailyLogH
#define cCsvDailyLogH
//---------------------------------------------------------------------------
#include <vcl.h>
#include <SyncObjs.hpp>
//---------------------------------------------------------------------------
class cCsvDailyLog
{
public:
    // Folder rollover granularity.
    enum eLogGranularity { lgMonthlyFolder, lgDailyFolder };

protected:
    TCriticalSection* m_pCS;
    AnsiString m_sBaseDir;      // LogRootDir + "\\" + SubFolder
    AnsiString m_sFilePrefix;   // daily file name prefix
    AnsiString m_sHeader;       // CSV header row ("" = no header)
    AnsiString m_sExt;          // file extension incl. dot, e.g. ".csv" / ".log"
    eLogGranularity m_eGran;    // folder rollover style
    AnsiString m_sLastFilePath; // cached daily path
    AnsiString m_sLastDate;     // "YYYY_MM_DD" of cached path (day-change detector)
    int m_nRetentionDays;       // keep this many days; 0 = keep forever (no prune)

    AnsiString GetLogFilePath();
    void EnsureHeader(const AnsiString& sPath);
    // Delete day/month sub-folders entirely older than the retention window.
    // No-op when retention is 0. Runs under the same lock as AppendLine.
    void PruneOldFolders();

public:
    cCsvDailyLog();
    virtual ~cCsvDailyLog();

    // Called once before logging (from a subclass Init() or directly).
    void InitLog(const AnsiString& sSubFolder,
                 const AnsiString& sFilePrefix,
                 const AnsiString& sHeader,
                 eLogGranularity eGran = lgMonthlyFolder,
                 const AnsiString& sExt = ".csv");

    // Set the retention window in days. Folders older than this are deleted on
    // the next day-change rollover and once immediately. 0 (default) keeps
    // everything. Call once after InitLog().
    void SetRetentionDays(int nDays);

    // Append one preformatted line (thread-safe; writes header on first call).
    void AppendLine(const AnsiString& sLine);

    // Wrap a free-form field in quotes and double any embedded quote so a
    // comma / quote cannot break the CSV column layout.
    static AnsiString CsvQuote(const AnsiString& sText);
};
//---------------------------------------------------------------------------
#endif
