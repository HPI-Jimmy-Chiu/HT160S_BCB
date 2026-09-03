//---------------------------------------------------------------------------
// uFtpUploadThread.cpp
// Background FTP upload worker for HT160S. See uFtpUploadThread.h for the design.
// Ported and evolved from HT9045 899 TFtpUploadThread.
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <windows.h>
#include <wininet.h>
#include <stdio.h>
#include <IniFiles.hpp>
#include <FileCtrl.hpp>

#include "uFtpUploadThread.h"
#include "database.h"        // HSys.CurrentDir / HSys.LogRootDir
#include "GeneralSetting.h"  // GeneralSetting.iLogRetentionEventDays
#pragma package(smart_init)
// WinINet import library (wininet.lib) is linked via the project (ht160s.bpr /
// ht160s.mak LIBRARIES), the same way ws2_32.lib is -- not via #pragma comment.
//---------------------------------------------------------------------------
// Config defaults (KYEC Sorter-log). Enable / UploadReport ship OFF: the machine
// never uploads until the maintenance screen has verified the link.
static const char  *FTP_DEF_HOST       = "192.168.11.11";
static const int    FTP_DEF_PORT       = 21;
static const char  *FTP_DEF_USER       = "Sorter-log";
static const char  *FTP_DEF_PWD        = "Kyec20260720";
static const char  *FTP_DEF_REMOTEDIR  = "/Sorter-log/";
static const int    FTP_DEF_TIMEOUT_MS = 5000;
static const int    FTP_DEF_RETRY      = 2;
static const int    FTP_UI_RING_MAX    = 50;
static const DWORD  FTP_RETRY_SLEEP_MS = 2000;
//---------------------------------------------------------------------------
// Single global definition (HT160 convention, mirrors LotWebApiClient).
TFtpUploadThread *FtpUploadThd = NULL;
//---------------------------------------------------------------------------
void EnsureFtpUploadThreadCreated()
{
    if(FtpUploadThd == NULL)
    {
        // Create suspended, load config on the main thread, then start the worker
        // so it never runs with default config before the INI is read.
        FtpUploadThd = new TFtpUploadThread(true);
        FtpUploadThd->LoadConfig();
        FtpUploadThd->Resume();
    }
}
//---------------------------------------------------------------------------
// Read the FTP server's last response text (for diagnostic logging).
static AnsiString FtpLastResp()
{
    char  sz[512];
    DWORD len = sizeof(sz) - 1;
    DWORD e   = 0;
    sz[0] = 0;
    if(InternetGetLastResponseInfo(&e, sz, &len))
    {
        if(len >= sizeof(sz)) len = sizeof(sz) - 1;
        sz[len] = 0;
        return AnsiString(sz);
    }
    return AnsiString("");
}
//---------------------------------------------------------------------------
__fastcall TFtpUploadThread::TFtpUploadThread(bool CreateSuspended)
    : TThread(CreateSuspended)
{
    eJob = CreateEvent(NULL, true, false, NULL);   // manual-reset: work available
    eEnd = CreateEvent(NULL, true, false, NULL);   // manual-reset: shutdown
    bEndThread = false;

    csQueue  = new TCriticalSection();
    csCfg    = new TCriticalSection();
    csResult = new TCriticalSection();

    sInFlightKey = "";

    m_sHost         = FTP_DEF_HOST;
    m_iPort         = FTP_DEF_PORT;
    m_sUser         = FTP_DEF_USER;
    m_sPwd          = FTP_DEF_PWD;
    m_sRemoteDir    = NormalizeRemoteDir(FTP_DEF_REMOTEDIR);
    m_bEnable       = false;
    m_bUploadReport = false;
    m_iTimeoutMs    = FTP_DEF_TIMEOUT_MS;
    m_iRetry        = FTP_DEF_RETRY;

    m_iOkCount   = 0;
    m_iFailCount = 0;
    m_sLastResult= "";
    m_sLastError = "";

    // Background text log: dedicated instance, daily folder, no header (.log).
    // Same folder layout as EventLog/WebAPI. Init on the main thread (before the
    // worker resumes) so only AppendLine runs from the worker (single writer).
    m_BgLog.InitLog("FtpUpload", "FtpUpload", "", cCsvDailyLog::lgDailyFolder, ".log");
    m_BgLog.SetRetentionDays(GeneralSetting.iLogRetentionEventDays);
}
//---------------------------------------------------------------------------
__fastcall TFtpUploadThread::~TFtpUploadThread()
{
    try
    {
        if(eJob) { CloseHandle(eJob); eJob = NULL; }
        if(eEnd) { CloseHandle(eEnd); eEnd = NULL; }
        if(csQueue)  { delete csQueue;  csQueue  = NULL; }
        if(csCfg)    { delete csCfg;    csCfg    = NULL; }
        if(csResult) { delete csResult; csResult = NULL; }
    }
    catch(...)
    {
        // never throw out of a destructor; never call VCL here
    }
}
//---------------------------------------------------------------------------
// Ensure a remote dir path ends and begins with '/'. Blank -> "/".
AnsiString __fastcall TFtpUploadThread::NormalizeRemoteDir(const AnsiString &sDir)
{
    AnsiString s = sDir.Trim();
    if(s == "")
        return AnsiString("/");
    if(s[1] != '/')
        s = AnsiString("/") + s;
    if(s[s.Length()] != '/')
        s = s + "/";
    return s;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TFtpUploadThread::LeafOf(const AnsiString &sPath)
{
    int i = sPath.LastDelimiter("\\/");
    if(i > 0 && i < sPath.Length())
        return sPath.SubString(i + 1, sPath.Length() - i);
    return sPath;
}
//---------------------------------------------------------------------------
void __fastcall TFtpUploadThread::LoadConfig()
{
    AnsiString sPath = HSys.CurrentDir + AnsiString("\\system\\General.ini");
    TIniFile *pIni = new TIniFile(sPath);
    csCfg->Acquire();
    try
    {
        m_sHost         = AnsiString(pIni->ReadString ("Ftp", "Host",         FTP_DEF_HOST).c_str());
        m_iPort         =            pIni->ReadInteger("Ftp", "Port",         FTP_DEF_PORT);
        m_sUser         = AnsiString(pIni->ReadString ("Ftp", "User",         FTP_DEF_USER).c_str());
        m_sPwd          = AnsiString(pIni->ReadString ("Ftp", "Password",     FTP_DEF_PWD).c_str());
        m_sRemoteDir    = NormalizeRemoteDir(pIni->ReadString("Ftp", "RemoteDir", FTP_DEF_REMOTEDIR));
        m_bEnable       =            pIni->ReadBool   ("Ftp", "Enable",       false);
        m_bUploadReport =            pIni->ReadBool   ("Ftp", "UploadReport", false);
        m_iTimeoutMs    =            pIni->ReadInteger("Ftp", "TimeoutMs",    FTP_DEF_TIMEOUT_MS);
        m_iRetry        =            pIni->ReadInteger("Ftp", "Retry",        FTP_DEF_RETRY);
        if(m_iPort <= 0)      m_iPort = FTP_DEF_PORT;
        if(m_iTimeoutMs <= 0) m_iTimeoutMs = FTP_DEF_TIMEOUT_MS;
        if(m_iRetry < 0)      m_iRetry = 0;
    }
    __finally
    {
        csCfg->Release();
        delete pIni;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFtpUploadThread::SaveConfig()
{
    AnsiString sPath = HSys.CurrentDir + AnsiString("\\system\\General.ini");
    ForceDirectories(ExtractFilePath(sPath));
    TIniFile *pIni = new TIniFile(sPath);
    csCfg->Acquire();
    try
    {
        pIni->WriteString ("Ftp", "Host",         m_sHost);
        pIni->WriteInteger("Ftp", "Port",         m_iPort);
        pIni->WriteString ("Ftp", "User",         m_sUser);
        pIni->WriteString ("Ftp", "Password",     m_sPwd);
        pIni->WriteString ("Ftp", "RemoteDir",    m_sRemoteDir);
        pIni->WriteBool   ("Ftp", "Enable",       m_bEnable);
        pIni->WriteBool   ("Ftp", "UploadReport", m_bUploadReport);
        pIni->WriteInteger("Ftp", "TimeoutMs",    m_iTimeoutMs);
        pIni->WriteInteger("Ftp", "Retry",        m_iRetry);
    }
    __finally
    {
        csCfg->Release();
        delete pIni;
    }
}
//---------------------------------------------------------------------------
void __fastcall TFtpUploadThread::SetConfig(AnsiString sHost, int iPort, AnsiString sUser,
        AnsiString sPwd, AnsiString sRemoteDir, bool bEnable, bool bUploadReport,
        int iTimeoutMs, int iRetry)
{
    csCfg->Acquire();
    try
    {
        m_sHost         = AnsiString(sHost.c_str());
        m_iPort         = (iPort > 0) ? iPort : FTP_DEF_PORT;
        m_sUser         = AnsiString(sUser.c_str());
        m_sPwd          = AnsiString(sPwd.c_str());
        m_sRemoteDir    = NormalizeRemoteDir(sRemoteDir);
        m_bEnable       = bEnable;
        m_bUploadReport = bUploadReport;
        m_iTimeoutMs    = (iTimeoutMs > 0) ? iTimeoutMs : FTP_DEF_TIMEOUT_MS;
        m_iRetry        = (iRetry >= 0) ? iRetry : 0;
    }
    __finally
    {
        csCfg->Release();
    }
}
//---------------------------------------------------------------------------
void __fastcall TFtpUploadThread::GetConfig(AnsiString &sHost, int &iPort, AnsiString &sUser,
        AnsiString &sPwd, AnsiString &sRemoteDir, bool &bEnable, bool &bUploadReport,
        int &iTimeoutMs, int &iRetry)
{
    csCfg->Acquire();
    try
    {
        sHost         = AnsiString(m_sHost.c_str());
        iPort         = m_iPort;
        sUser         = AnsiString(m_sUser.c_str());
        sPwd          = AnsiString(m_sPwd.c_str());
        sRemoteDir    = AnsiString(m_sRemoteDir.c_str());
        bEnable       = m_bEnable;
        bUploadReport = m_bUploadReport;
        iTimeoutMs    = m_iTimeoutMs;
        iRetry        = m_iRetry;
    }
    __finally
    {
        csCfg->Release();
    }
}
//---------------------------------------------------------------------------
bool __fastcall TFtpUploadThread::GetEnable()
{
    bool b;
    csCfg->Acquire();
    try { b = m_bEnable; }
    __finally { csCfg->Release(); }
    return b;
}
//---------------------------------------------------------------------------
bool __fastcall TFtpUploadThread::GetUploadReport()
{
    bool b;
    csCfg->Acquire();
    try { b = m_bUploadReport; }
    __finally { csCfg->Release(); }
    return b;
}
//---------------------------------------------------------------------------
void __fastcall TFtpUploadThread::GetCfgSnapshot(TFtpCfgSnapshot &c)
{
    csCfg->Acquire();
    try
    {
        c.sHost      = AnsiString(m_sHost.c_str());
        c.iPort      = m_iPort;
        c.sUser      = AnsiString(m_sUser.c_str());
        c.sPwd       = AnsiString(m_sPwd.c_str());
        c.sRemoteDir = AnsiString(m_sRemoteDir.c_str());
        c.iTimeoutMs = m_iTimeoutMs;
        c.iRetry     = m_iRetry;
    }
    __finally
    {
        csCfg->Release();
    }
}
//---------------------------------------------------------------------------
void __fastcall TFtpUploadThread::WriteBgLog(const AnsiString &asMsg)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    AnsiString line;
    line.sprintf("[%04d-%02d-%02d %02d:%02d:%02d] %s",
                 st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
                 asMsg.c_str());
    m_BgLog.AppendLine(line);   // cCsvDailyLog append is critical-section guarded
}
//---------------------------------------------------------------------------
// EventLog queue (LOT_PUBLISH ok + give-up). Deep-copy the message so the main
// thread never shares this worker's AnsiString buffer.
void __fastcall TFtpUploadThread::PushEventResult(const AnsiString &asMsg)
{
    AnsiString asCopy = AnsiString(asMsg.c_str());
    csResult->Acquire();
    try { lstResults.push_back(asCopy); }
    __finally { csResult->Release(); }
}
//---------------------------------------------------------------------------
// UI ring (all outcomes) + tallies + last-result. Bounded to FTP_UI_RING_MAX.
void __fastcall TFtpUploadThread::PushUiResult(const AnsiString &asMsg, bool bOk)
{
    SYSTEMTIME st;
    GetLocalTime(&st);
    AnsiString asLine;
    asLine.sprintf("[%02d:%02d:%02d] %s", st.wHour, st.wMinute, st.wSecond, asMsg.c_str());
    AnsiString asCopy = AnsiString(asLine.c_str());

    csResult->Acquire();
    try
    {
        lstUi.push_back(asCopy);
        while((int)lstUi.size() > FTP_UI_RING_MAX)
            lstUi.pop_front();
        if(bOk) m_iOkCount++;
        else    m_iFailCount++;
        m_sLastResult = AnsiString(asMsg.c_str());
    }
    __finally
    {
        csResult->Release();
    }
}
//---------------------------------------------------------------------------
bool __fastcall TFtpUploadThread::FetchResult(AnsiString &asMsg)
{
    bool bHas = false;
    csResult->Acquire();
    try
    {
        if(!lstResults.empty())
        {
            asMsg = AnsiString(lstResults.front().c_str());
            lstResults.pop_front();
            bHas = true;
        }
    }
    __finally
    {
        csResult->Release();
    }
    return bHas;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TFtpUploadThread::GetResultLog()
{
    AnsiString s = "";
    csResult->Acquire();
    try
    {
        for(std::list<AnsiString>::iterator it = lstUi.begin(); it != lstUi.end(); ++it)
        {
            if(s != "") s += "\r\n";
            s += *it;
        }
    }
    __finally
    {
        csResult->Release();
    }
    return AnsiString(s.c_str());
}
//---------------------------------------------------------------------------
AnsiString __fastcall TFtpUploadThread::GetLastError()
{
    AnsiString s;
    csResult->Acquire();
    try { s = AnsiString(m_sLastError.c_str()); }
    __finally { csResult->Release(); }
    return s;
}
//---------------------------------------------------------------------------
AnsiString __fastcall TFtpUploadThread::GetStatusSnapshot()
{
    int  iDepth;
    AnsiString sInFlight;
    csQueue->Acquire();
    try
    {
        iDepth = (int)lstJobs.size();
        sInFlight = AnsiString(sInFlightKey.c_str());
    }
    __finally { csQueue->Release(); }

    int iOk, iFail;
    AnsiString sLast;
    csResult->Acquire();
    try
    {
        iOk = m_iOkCount;
        iFail = m_iFailCount;
        sLast = AnsiString(m_sLastResult.c_str());
    }
    __finally { csResult->Release(); }

    AnsiString s;
    s.sprintf("queue=%d inflight=%s ok=%d fail=%d",
              iDepth,
              (sInFlight == "" ? "-" : sInFlight.c_str()),
              iOk, iFail);
    if(sLast != "")
        s += AnsiString(" | last: ") + sLast;
    return AnsiString(s.c_str());
}
//---------------------------------------------------------------------------
AnsiString __fastcall TFtpUploadThread::MakeJobKey(const TFtpUploadJob &job)
{
    AnsiString k;
    k.sprintf("%d|%s|%s", job.iJobKind, job.sKyecLot.c_str(), job.sCsvList.c_str());
    return k;
}
//---------------------------------------------------------------------------
void __fastcall TFtpUploadThread::EnqueueLotPublish(AnsiString sKyecLot, AnsiString sCsvLocalList,
        AnsiString sFlagLocal)
{
    TFtpUploadJob job;
    job.iJobKind   = FTPJOB_LOT_PUBLISH;
    job.sKyecLot   = AnsiString(sKyecLot.c_str());
    job.sCsvList   = AnsiString(sCsvLocalList.c_str());
    job.sFlagLocal = AnsiString(sFlagLocal.c_str());
    job.sSorterId  = "";

    AnsiString asKey = MakeJobKey(job);
    bool bDup = false;
    csQueue->Acquire();
    try
    {
        if(asKey != "" && sInFlightKey == asKey)
        {
            bDup = true;
        }
        else
        {
            for(std::list<TFtpUploadJob>::iterator it = lstJobs.begin(); it != lstJobs.end(); ++it)
            {
                if(MakeJobKey(*it) == asKey) { bDup = true; break; }
            }
        }
        if(!bDup)
            lstJobs.push_back(job);
    }
    __finally
    {
        csQueue->Release();
    }

    if(bDup)
    {
        AnsiString l;
        l.sprintf("DEDUP skip: lot=%s already queued/in-flight",
                  job.sKyecLot.c_str());
        WriteBgLog(l);
        return;
    }
    SetEvent(eJob);
}
//---------------------------------------------------------------------------
void __fastcall TFtpUploadThread::EnqueueTestConn()
{
    TFtpUploadJob job;
    job.iJobKind  = FTPJOB_TEST_CONN;
    job.sKyecLot  = "";
    job.sCsvList  = "";
    job.sFlagLocal= "";
    job.sSorterId = "";
    csQueue->Acquire();
    try { lstJobs.push_back(job); }
    __finally { csQueue->Release(); }
    SetEvent(eJob);
}
//---------------------------------------------------------------------------
void __fastcall TFtpUploadThread::EnqueueTestUpload(AnsiString sSorterId)
{
    TFtpUploadJob job;
    job.iJobKind  = FTPJOB_TEST_UPLOAD;
    job.sKyecLot  = "";
    job.sCsvList  = "";
    job.sFlagLocal= "";
    job.sSorterId = AnsiString(sSorterId.c_str());
    csQueue->Acquire();
    try { lstJobs.push_back(job); }
    __finally { csQueue->Release(); }
    SetEvent(eJob);
}
//---------------------------------------------------------------------------
bool __fastcall TFtpUploadThread::PopJob(TFtpUploadJob &job)
{
    bool bHas = false;
    csQueue->Acquire();
    try
    {
        if(!lstJobs.empty())
        {
            job = lstJobs.front();
            lstJobs.pop_front();
            bHas = true;
            sInFlightKey = MakeJobKey(job);
        }
    }
    __finally
    {
        csQueue->Release();
    }
    return bHas;
}
//---------------------------------------------------------------------------
// Create every level of a remote directory. FtpCreateDirectory returns FALSE for
// an already-existing level; that is ignored (idempotent).
void __fastcall TFtpUploadThread::EnsureRemoteDir(void *hConn, const AnsiString &sFullDir)
{
    AnsiString acc = "";
    AnsiString seg = "";
    int n = sFullDir.Length();
    for(int i = 1; i <= n; i++)
    {
        char ch = sFullDir[i];
        if(ch == '/')
        {
            if(seg != "")
            {
                acc = acc + "/" + seg;
                FtpCreateDirectory((HINTERNET)hConn, acc.c_str());
                seg = "";
            }
        }
        else
        {
            seg += ch;
        }
    }
    if(seg != "")
    {
        acc = acc + "/" + seg;
        FtpCreateDirectory((HINTERNET)hConn, acc.c_str());
    }
}
//---------------------------------------------------------------------------
// One LOT_PUBLISH attempt: connect, create dir, put CSV, put flag (flag LAST so
// KY CIM only sees the commit signal after the payload is complete).
bool __fastcall TFtpUploadThread::DoPublishAttempt(const TFtpUploadJob &job,
        const TFtpCfgSnapshot &c, unsigned int &dwErr, AnsiString &asResp)
{
    bool bOk = false;
    dwErr = 0;
    asResp = "";

    HINTERNET hSession = InternetOpen("HT160S_FtpUpload", INTERNET_OPEN_TYPE_PRECONFIG,
                                      NULL, NULL, 0);
    if(hSession == NULL)
    {
        dwErr = ::GetLastError();
        return false;
    }
    DWORD dwTimeout = (DWORD)c.iTimeoutMs;
    InternetSetOption(hSession, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeout, sizeof(dwTimeout));

    HINTERNET hConn = InternetConnect(hSession, c.sHost.c_str(), (INTERNET_PORT)c.iPort,
                                      c.sUser.c_str(), c.sPwd.c_str(),
                                      INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);
    if(hConn == NULL)
    {
        dwErr = ::GetLastError();
        InternetCloseHandle(hSession);
        return false;
    }

    // hConn + hSession are open; a __finally guarantees both close even if an allocation
    // throws below (no handle leak, and the throw cannot escape without cleanup).
    try
    {
        EnsureRemoteDir(hConn, c.sRemoteDir + job.sKyecLot);

        // Upload every CSV of this lot FIRST; the flag (commit signal) goes LAST so KY CIM
        // never sees the flag before the payload is complete. A single failed CSV aborts the
        // whole attempt (all-or-nothing) so the flag is never orphaned.
        bool bAllCsv = true;
        int  iPut    = 0;
        TStringList *pCsv = new TStringList();
        try
        {
            pCsv->Text = job.sCsvList;   // one local CSV path per line
            for(int i = 0; i < pCsv->Count; i++)
            {
                if(bEndThread)   // shutdown mid-attempt : abort BEFORE the flag so an
                {                // incomplete set is never committed (re-sent next Lot End)
                    bAllCsv = false;
                    break;
                }
                AnsiString sLocal = pCsv->Strings[i].Trim();
                if(sLocal == "")
                    continue;
                AnsiString sRemote = c.sRemoteDir + job.sKyecLot + "/" + LeafOf(sLocal);
                if(!FtpPutFile(hConn, sLocal.c_str(), sRemote.c_str(), FTP_TRANSFER_TYPE_BINARY, 0))
                {
                    dwErr = ::GetLastError();
                    asResp = FtpLastResp();
                    bAllCsv = false;
                    break;
                }
                iPut++;
            }
        }
        __finally
        {
            delete pCsv;
        }

        // Put the flag only when every CSV uploaded AND at least one CSV was actually put
        // (a zero-payload flag would falsely tell KY CIM to collect an empty folder), and
        // not while shutting down.
        if(bAllCsv && iPut > 0 && !bEndThread)
        {
            EnsureRemoteDir(hConn, c.sRemoteDir + "LotEnd");
            AnsiString sFlagRemote = c.sRemoteDir + "LotEnd/" + LeafOf(job.sFlagLocal);
            if(FtpPutFile(hConn, job.sFlagLocal.c_str(), sFlagRemote.c_str(),
                          FTP_TRANSFER_TYPE_BINARY, 0))
            {
                bOk = true;
            }
            else
            {
                dwErr = ::GetLastError();
                asResp = FtpLastResp();
            }
        }
    }
    __finally
    {
        InternetCloseHandle(hConn);
        InternetCloseHandle(hSession);
    }
    return bOk;
}
//---------------------------------------------------------------------------
bool __fastcall TFtpUploadThread::DoTestConn(const TFtpCfgSnapshot &c, unsigned int &dwErr)
{
    dwErr = 0;
    HINTERNET hSession = InternetOpen("HT160S_FtpUpload", INTERNET_OPEN_TYPE_PRECONFIG,
                                      NULL, NULL, 0);
    if(hSession == NULL) { dwErr = ::GetLastError(); return false; }
    DWORD dwTimeout = (DWORD)c.iTimeoutMs;
    InternetSetOption(hSession, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeout, sizeof(dwTimeout));

    HINTERNET hConn = InternetConnect(hSession, c.sHost.c_str(), (INTERNET_PORT)c.iPort,
                                      c.sUser.c_str(), c.sPwd.c_str(),
                                      INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);
    bool bOk = (hConn != NULL);
    if(!bOk) dwErr = ::GetLastError();
    if(hConn) InternetCloseHandle(hConn);
    InternetCloseHandle(hSession);
    return bOk;
}
//---------------------------------------------------------------------------
// Write a small local temp file then upload it to RemoteDir root, verifying that
// the account can actually write. The file is left in place for the customer to
// confirm receipt.
bool __fastcall TFtpUploadThread::DoTestUpload(const TFtpUploadJob &job,
        const TFtpCfgSnapshot &c, unsigned int &dwErr, AnsiString &asResp, AnsiString &asRemote)
{
    dwErr = 0;
    asResp = "";

    SYSTEMTIME st;
    GetLocalTime(&st);
    AnsiString sSorter = (job.sSorterId.Trim() == "") ? AnsiString("NA") : job.sSorterId;
    AnsiString sLeaf;
    sLeaf.sprintf("TEST_%s_%04d%02d%02d_%02d%02d%02d.txt",
                  sSorter.c_str(), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond);

    AnsiString sLocalDir = HSys.LogRootDir + "\\FtpUpload";
    ForceDirectories(sLocalDir);
    AnsiString sLocal = sLocalDir + "\\" + sLeaf;

    FILE *fp = fopen(sLocal.c_str(), "w");
    if(fp)
    {
        fprintf(fp, "HT160S FTP test upload %04d-%02d-%02d %02d:%02d:%02d sorter=%s\n",
                st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, sSorter.c_str());
        fclose(fp);
    }
    else
    {
        dwErr = (unsigned int)::GetLastError();
        asResp = "cannot create local temp file";
        return false;
    }

    HINTERNET hSession = InternetOpen("HT160S_FtpUpload", INTERNET_OPEN_TYPE_PRECONFIG,
                                      NULL, NULL, 0);
    if(hSession == NULL) { dwErr = ::GetLastError(); return false; }
    DWORD dwTimeout = (DWORD)c.iTimeoutMs;
    InternetSetOption(hSession, INTERNET_OPTION_CONNECT_TIMEOUT, &dwTimeout, sizeof(dwTimeout));

    HINTERNET hConn = InternetConnect(hSession, c.sHost.c_str(), (INTERNET_PORT)c.iPort,
                                      c.sUser.c_str(), c.sPwd.c_str(),
                                      INTERNET_SERVICE_FTP, INTERNET_FLAG_PASSIVE, 0);
    if(hConn == NULL)
    {
        dwErr = ::GetLastError();
        InternetCloseHandle(hSession);
        return false;
    }

    asRemote = c.sRemoteDir + sLeaf;
    bool bOk = FtpPutFile(hConn, sLocal.c_str(), asRemote.c_str(), FTP_TRANSFER_TYPE_BINARY, 0) ? true : false;
    if(!bOk)
    {
        dwErr = ::GetLastError();
        asResp = FtpLastResp();
    }
    InternetCloseHandle(hConn);
    InternetCloseHandle(hSession);
    return bOk;
}
//---------------------------------------------------------------------------
void __fastcall TFtpUploadThread::RunJob(const TFtpUploadJob &job)
{
    TFtpCfgSnapshot c;
    GetCfgSnapshot(c);

    DWORD dwStart = GetTickCount();

    if(job.iJobKind == FTPJOB_LOT_PUBLISH)
    {
        int attemptsLeft = c.iRetry + 1;
        if(attemptsLeft < 1) attemptsLeft = 1;
        int  attempt = 0;
        bool bOk = false;
        unsigned int uErr = 0;
        AnsiString asResp;

        // count CSVs in the job (for the log/result message only)
        int iFileCount = 0;
        {
            TStringList *pC = new TStringList();
            try
            {
                pC->Text = job.sCsvList;
                for(int i = 0; i < pC->Count; i++)
                    if(pC->Strings[i].Trim() != "")
                        iFileCount++;
            }
            __finally { delete pC; }
        }

        while(attemptsLeft > 0 && !bEndThread)
        {
            attempt++;
            uErr = 0;
            asResp = "";
            bOk = DoPublishAttempt(job, c, uErr, asResp);
            if(bOk) break;
            attemptsLeft--;
            if(attemptsLeft > 0 && !bEndThread)
            {
                AnsiString r;
                r.sprintf("PUBLISH FAIL (attempt %d) lot=%s files=%d flag=%s err=%u resp=%s ; will retry",
                          attempt, job.sKyecLot.c_str(), iFileCount, LeafOf(job.sFlagLocal).c_str(),
                          uErr, asResp.c_str());
                WriteBgLog(r);
                ::Sleep(FTP_RETRY_SLEEP_MS);
            }
        }

        DWORD dwCost = GetTickCount() - dwStart;
        AnsiString msg;
        if(bOk)
        {
            msg.sprintf("PUBLISH OK lot=%s files=%d flag=%s attempts=%d cost=%ums",
                        job.sKyecLot.c_str(), iFileCount,
                        LeafOf(job.sFlagLocal).c_str(), attempt, (unsigned)dwCost);
            WriteBgLog(msg);
            PushUiResult(msg, true);
            PushEventResult(AnsiString("FTP report upload OK: ") + msg);
        }
        else
        {
            msg.sprintf("PUBLISH GIVEUP lot=%s files=%d flag=%s err=%u resp=%s attempts=%d cost=%ums",
                        job.sKyecLot.c_str(), iFileCount, LeafOf(job.sFlagLocal).c_str(),
                        uErr, asResp.c_str(), attempt, (unsigned)dwCost);
            WriteBgLog(msg);
            PushUiResult(msg, false);
            PushEventResult(AnsiString("FTP report upload give up: ") + msg);
            csResult->Acquire();
            try { m_sLastError = AnsiString(msg.c_str()); }
            __finally { csResult->Release(); }
        }
    }
    else if(job.iJobKind == FTPJOB_TEST_CONN)
    {
        unsigned int uErr = 0;
        bool bOk = DoTestConn(c, uErr);
        AnsiString msg;
        if(bOk)
            msg.sprintf("TEST CONN OK host=%s port=%d user=%s", c.sHost.c_str(), c.iPort, c.sUser.c_str());
        else
            msg.sprintf("TEST CONN FAIL host=%s port=%d err=%u", c.sHost.c_str(), c.iPort, uErr);
        WriteBgLog(msg);
        PushUiResult(msg, bOk);
        if(!bOk)
        {
            csResult->Acquire();
            try { m_sLastError = AnsiString(msg.c_str()); }
            __finally { csResult->Release(); }
        }
    }
    else if(job.iJobKind == FTPJOB_TEST_UPLOAD)
    {
        unsigned int uErr = 0;
        AnsiString asResp, asRemote;
        bool bOk = DoTestUpload(job, c, uErr, asResp, asRemote);
        AnsiString msg;
        if(bOk)
            msg.sprintf("TEST UPLOAD OK remote=%s", asRemote.c_str());
        else
            msg.sprintf("TEST UPLOAD FAIL err=%u resp=%s", uErr, asResp.c_str());
        WriteBgLog(msg);
        PushUiResult(msg, bOk);
        if(!bOk)
        {
            csResult->Acquire();
            try { m_sLastError = AnsiString(msg.c_str()); }
            __finally { csResult->Release(); }
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall TFtpUploadThread::Execute()
{
    WriteBgLog("FtpUploadThread started.");

    HANDLE hWait[2];
    hWait[0] = eJob;
    hWait[1] = eEnd;

    do
    {
        WaitForMultipleObjects(2, hWait, false, INFINITE);
        if(bEndThread)
            break;

        // Reset eJob before draining so a new Enqueue during this drain is not lost
        // (Enqueue SetEvents after push; reset here can only precede that).
        ResetEvent(eJob);

        TFtpUploadJob job;
        while(PopJob(job))
        {
            if(bEndThread)
                break;
            // Golden rule : a job must NEVER kill the worker. Swallow any exception (e.g.
            // OOM from an allocation deep in a publish) so Execute keeps draining; the
            // in-flight key is still cleared below so the queue never wedges.
            try { RunJob(job); }
            catch(...) { try { WriteBgLog("RunJob threw; job dropped, worker survives."); } catch(...) {} }

            csQueue->Acquire();
            try { sInFlightKey = ""; }
            __finally { csQueue->Release(); }
        }
    }
    while(!bEndThread);

    WriteBgLog("FtpUploadThread stopped.");
}
//---------------------------------------------------------------------------
void __fastcall TFtpUploadThread::EndThread()
{
    bEndThread = true;
    SetEvent(eEnd);
    SetEvent(eJob);
}
//---------------------------------------------------------------------------
