//---------------------------------------------------------------------------
// uFtpUploadThread.h
// Background FTP upload worker for HT160S (KYEC Sorter-log data hand-off).
//
// Single class owns EVERY FTP action (connect / create-dir / put / test / log).
// Callers only touch the public enqueue / config / snapshot methods; they never
// carry connection parameters (config lives in this class, loaded from
// system\General.ini [Ftp]) and they never block (fire-and-forget queue).
//
// Ported and evolved from HT9045 899 TFtpUploadThread
//   (D:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\FTPUpload\).
// Kept from the proven parent: job queue + two manual-reset events + critical
// section, AnsiString c_str() deep-copy into the queue (defeats cross-thread COW
// refcount race), WinINet PASSIVE FTP, in-worker retry with ::Sleep (a background
// thread may sleep safely), dedup by job key + in-flight key, and the golden rule
// -- Execute() NEVER calls Synchronize / VCL / MOT[] / Sen[]; results are handed
// to the main thread by a polled queue instead.
//
// Evolutions for HT160S:
//   1. Config is embedded (LoadConfig/SaveConfig on [Ftp]); enqueue takes no
//      connection parameters. Config is snapshotted under a lock when a job runs.
//   2. Job kinds: LOT_PUBLISH (create-dir + CSV + flag, the KYEC two-stage
//      hand-off), TEST_CONN, TEST_UPLOAD. The maintenance screen's test buttons
//      only enqueue -- the UI never touches WinINet.
//   3. Two result channels: an EventLog queue (LOT_PUBLISH ok + give-up, pumped
//      to EventLog by the main loop) and a bounded UI ring buffer (all outcomes,
//      read non-destructively by the maintenance screen).
//   4. Status snapshot (queue depth / in-flight / ok-fail tallies / last result).
//   5. Background log via a dedicated cCsvDailyLog instance (thread-safe append),
//      D:\HT160S_Log\FtpUpload\YYYYMMDD\FtpUpload_YYYYMMDD.log.
//---------------------------------------------------------------------------
#ifndef uFtpUploadThreadH
#define uFtpUploadThreadH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <SyncObjs.hpp>
#include <list>
#include "cCsvDailyLog.h"
//---------------------------------------------------------------------------
// Job kinds.
#define FTPJOB_LOT_PUBLISH  0   // create /<RemoteDir>/<KYLotNo>/, put CSV, put flag to /LotEnd/
#define FTPJOB_TEST_CONN    1   // connect + login only (maintenance test button)
#define FTPJOB_TEST_UPLOAD  2   // connect + put a small timestamp file to RemoteDir root
//---------------------------------------------------------------------------
// One unit of FTP work. AnsiString members are deep-copied on enqueue so the
// worker never shares a COW buffer with the calling thread.
struct TFtpUploadJob
{
    int        iJobKind;    // FTPJOB_*
    AnsiString sKyecLot;    // LOT_PUBLISH: remote KYLotNo sub-folder name
    AnsiString sCsvList;    // LOT_PUBLISH: newline-joined local CSV paths (all uploaded before flag)
    AnsiString sFlagLocal;  // LOT_PUBLISH: local flag-file path to upload (put LAST = commit signal)
    AnsiString sSorterId;   // TEST_UPLOAD: token for the temp file name (read on main thread)
};
//---------------------------------------------------------------------------
// Connection config snapshot handed to a job at execution time.
struct TFtpCfgSnapshot
{
    AnsiString sHost;
    int        iPort;
    AnsiString sUser;
    AnsiString sPwd;
    AnsiString sRemoteDir;  // normalized: leading + trailing '/'
    int        iTimeoutMs;
    int        iRetry;
};
//---------------------------------------------------------------------------
class TFtpUploadThread : public TThread
{
private:
    HANDLE eJob;    // manual-reset: work available
    HANDLE eEnd;    // manual-reset: shutdown requested
    bool   bEndThread;

    // --- queue (csQueue) ---
    TCriticalSection         *csQueue;
    std::list<TFtpUploadJob>  lstJobs;
    AnsiString                sInFlightKey;   // key of the job currently uploading ("" = none)

    // --- config (csCfg) ---
    TCriticalSection *csCfg;
    AnsiString        m_sHost;
    int               m_iPort;
    AnsiString        m_sUser;
    AnsiString        m_sPwd;
    AnsiString        m_sRemoteDir;
    bool              m_bEnable;        // gate for PRODUCTION uploads (not test jobs)
    bool              m_bUploadReport;  // upload the Soter report at Lot End
    int               m_iTimeoutMs;
    int               m_iRetry;

    // --- results (csResult) ---
    TCriticalSection      *csResult;
    std::list<AnsiString>  lstResults;    // EventLog queue (LOT_PUBLISH ok + give-up)
    std::list<AnsiString>  lstUi;         // bounded UI ring buffer (all outcomes)
    int                    m_iOkCount;
    int                    m_iFailCount;
    AnsiString             m_sLastResult;
    AnsiString             m_sLastError;

    cCsvDailyLog m_BgLog;   // background text log (own instance; init on main thread)

    // helpers
    void __fastcall WriteBgLog(const AnsiString &asMsg);
    void __fastcall PushEventResult(const AnsiString &asMsg);   // -> EventLog queue (thread-safe)
    void __fastcall PushUiResult(const AnsiString &asMsg, bool bOk); // -> UI ring + tallies
    bool __fastcall PopJob(TFtpUploadJob &job);
    static AnsiString __fastcall MakeJobKey(const TFtpUploadJob &job);
    void __fastcall GetCfgSnapshot(TFtpCfgSnapshot &c);
    static AnsiString __fastcall LeafOf(const AnsiString &sPath);
    static AnsiString __fastcall NormalizeRemoteDir(const AnsiString &sDir);

    // one job, dispatched by kind
    void __fastcall RunJob(const TFtpUploadJob &job);
    bool __fastcall DoPublishAttempt(const TFtpUploadJob &job, const TFtpCfgSnapshot &c,
                                     unsigned int &dwErr, AnsiString &asResp);
    bool __fastcall DoTestConn(const TFtpCfgSnapshot &c, unsigned int &dwErr);
    bool __fastcall DoTestUpload(const TFtpUploadJob &job, const TFtpCfgSnapshot &c,
                                 unsigned int &dwErr, AnsiString &asResp, AnsiString &asRemote);
    // create every path level of a remote directory (ignore already-exists)
    void __fastcall EnsureRemoteDir(void *hConn, const AnsiString &sFullDir);

protected:
    void __fastcall Execute();

public:
    __fastcall TFtpUploadThread(bool CreateSuspended);
    __fastcall ~TFtpUploadThread();

    // Config (system\General.ini [Ftp]); thread-safe.
    void __fastcall LoadConfig();
    void __fastcall SaveConfig();
    void __fastcall SetConfig(AnsiString sHost, int iPort, AnsiString sUser, AnsiString sPwd,
                              AnsiString sRemoteDir, bool bEnable, bool bUploadReport,
                              int iTimeoutMs, int iRetry);
    void __fastcall GetConfig(AnsiString &sHost, int &iPort, AnsiString &sUser, AnsiString &sPwd,
                              AnsiString &sRemoteDir, bool &bEnable, bool &bUploadReport,
                              int &iTimeoutMs, int &iRetry);
    bool __fastcall GetEnable();
    bool __fastcall GetUploadReport();

    // Enqueue (thread-safe, deep-copy + dedup). sCsvLocalList = newline-joined local CSV
    // paths; all are uploaded into /<RemoteDir>/<KyecLot>/ before the flag is put to /LotEnd/.
    void __fastcall EnqueueLotPublish(AnsiString sKyecLot, AnsiString sCsvLocalList, AnsiString sFlagLocal);
    void __fastcall EnqueueTestConn();
    void __fastcall EnqueueTestUpload(AnsiString sSorterId);

    // Request shutdown (sets flag + signals Execute out of its wait).
    void __fastcall EndThread();

    // Main-thread result pump (EventLog). Returns one queued message or false.
    bool __fastcall FetchResult(AnsiString &asMsg);

    // UI snapshots (non-destructive).
    AnsiString __fastcall GetStatusSnapshot();
    AnsiString __fastcall GetResultLog();
    AnsiString __fastcall GetLastError();
};
//---------------------------------------------------------------------------
// Single definition lives in uFtpUploadThread.cpp (HT160 convention, mirrors
// LotWebApiClient). Lazily created; call EnsureFtpUploadThreadCreated() once at
// startup and tear down in TfMain::FormClose.
extern TFtpUploadThread *FtpUploadThd;
void EnsureFtpUploadThreadCreated();
//---------------------------------------------------------------------------
#endif
