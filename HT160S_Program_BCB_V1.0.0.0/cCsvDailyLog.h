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

    AnsiString GetLogFilePath();
    void EnsureHeader(const AnsiString& sPath);

public:
    cCsvDailyLog();
    virtual ~cCsvDailyLog();

    // Called once before logging (from a subclass Init() or directly).
    void InitLog(const AnsiString& sSubFolder,
                 const AnsiString& sFilePrefix,
                 const AnsiString& sHeader,
                 eLogGranularity eGran = lgMonthlyFolder,
                 const AnsiString& sExt = ".csv");

    // Append one preformatted line (thread-safe; writes header on first call).
    void AppendLine(const AnsiString& sLine);

    // Wrap a free-form field in quotes and double any embedded quote so a
    // comma / quote cannot break the CSV column layout.
    static AnsiString CsvQuote(const AnsiString& sText);
};
//---------------------------------------------------------------------------
#endif
