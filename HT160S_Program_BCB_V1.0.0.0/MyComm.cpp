//---------------------------------------------------------------------------
// MyComm.cpp
//
// Implementation of TMyComm - see MyComm.h for the rationale.
//
// Threading / shutdown model (the tricky part, documented for maintainers):
//   * The worker issues an OVERLAPPED WaitCommEvent(EV_RXCHAR) and then blocks
//     in WaitForMultipleObjects({wait-event, FStopEvent}).  It NEVER spins.
//   * StopComm() signals FStopEvent and calls SetCommMask(handle,0).  Either is
//     enough to release the worker; both are used for belt-and-suspenders.
//   * RX delivery uses TThread::Synchronize, so OnReceiveData runs on the main
//     VCL thread.  StopComm() calls WaitFor() from the main thread, which pumps
//     the synchronize queue internally - so there is no Synchronize/WaitFor
//     deadlock.
//
// Logging model (high volume safe):
//   * LogLine() only appends to an in-memory buffer under FLogLock - cheap.
//   * The buffer is flushed to disk in batches (FLOG_FLUSH_LINES lines or
//     FLOG_FLUSH_MS ms), so the RX path is not stalled by disk I/O per line.
//   * Files use the house EventLog layout with daily rotation:
//        <LogDir>\YYYY_MM\<LogTag>_YYYY_MM_DD.csv
//     The dated name is recomputed on every flush, so it rolls over at midnight.
//   * FlushLog() forces an immediate write - StateRecord calls it before taking
//     a snapshot, and StopComm()/the destructor call it on shutdown.
//
// Encoding: ASCII only - all comments in English (see ENCODING_RULES.md).
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <stdio.h>
#include "MyComm.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

// Max bytes handed to OnReceiveData in a single call.  Kept below 1024 so the
// existing Bin-display handler's "if (BufferLength >= 1024) return;" guard never
// drops a chunk.  The full byte total is still recorded in the RX log line.
static const int RECEIVE_CHUNK = 512;

// Local read staging buffer size (per drain iteration).
static const int READ_BUF_SIZE = 4096;

// Log flush policy: write to disk once this many lines are buffered, or once
// this many milliseconds have elapsed since the last flush, whichever first.
static const int   FLOG_FLUSH_LINES = 50;
static const DWORD FLOG_FLUSH_MS    = 1000;

//===========================================================================
// TMyComm
//===========================================================================
__fastcall TMyComm::TMyComm(TComponent* Owner)
    : TComponent(Owner)
{
    FHandle      = INVALID_HANDLE_VALUE;
    FReadThread  = NULL;
    FConnected   = false;

    // SPComm-compatible defaults.
    BaudRate     = 9600;
    Parity       = cpNone;
    ParityCheck  = false;
    ByteSize     = cbs8;
    StopBits     = csb1;

    OnReceiveData = NULL;
    OnLog         = NULL;

    EnableLog    = true;
    LogDir       = "";
    LogTag       = "";
    LogFileName  = "";
    SyncReceive  = true;

    FLogBuf        = "";
    FLogPending    = 0;
    FLastFlushTick = GetTickCount();

    InitializeCriticalSection(&FWriteLock);
    InitializeCriticalSection(&FLogLock);
    FStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);   // manual-reset, unset
}
//---------------------------------------------------------------------------
__fastcall TMyComm::~TMyComm()
{
    StopComm();                          // also flushes the log
    if (FStopEvent)
        CloseHandle(FStopEvent);
    DeleteCriticalSection(&FWriteLock);
    DeleteCriticalSection(&FLogLock);
}
//---------------------------------------------------------------------------
AnsiString __fastcall TMyComm::NormalizeName(AnsiString Name)
{
    // Accept a bare "COMxx" and turn it into the "\\.\COMxx" device path that
    // is required for COM10 and above.  Leave anything already prefixed alone.
    AnsiString up = Name.UpperCase();
    if (up.SubString(1, 3) == "COM")
        return "\\\\.\\" + Name;
    return Name;
}
//---------------------------------------------------------------------------
void __fastcall TMyComm::ApplyDcb()
{
    DCB dcb;
    ZeroMemory(&dcb, sizeof(dcb));
    dcb.DCBlength = sizeof(dcb);

    if (!GetCommState(FHandle, &dcb))
        throw Exception("GetCommState failed for " + CommName);

    dcb.BaudRate = (DWORD)BaudRate;
    dcb.ByteSize = (BYTE)ByteSize;       // 5..8
    dcb.Parity   = (BYTE)Parity;         // 0..4 == NOPARITY..SPACEPARITY

    switch (StopBits)
    {
        case csb1:   dcb.StopBits = ONESTOPBIT;   break;
        case csb1_5: dcb.StopBits = ONE5STOPBITS; break;
        case csb2:   dcb.StopBits = TWOSTOPBITS;  break;
        default:     dcb.StopBits = ONESTOPBIT;   break;
    }

    dcb.fBinary          = TRUE;
    dcb.fParity          = ParityCheck ? TRUE : FALSE;
    // No flow control by default (matches how SPComm was used here).  DTR / RTS
    // are held enabled so devices that rely on those lines still operate.
    dcb.fOutxCtsFlow     = FALSE;
    dcb.fOutxDsrFlow     = FALSE;
    dcb.fDsrSensitivity  = FALSE;
    dcb.fDtrControl      = DTR_CONTROL_ENABLE;
    dcb.fRtsControl      = RTS_CONTROL_ENABLE;
    dcb.fOutX            = FALSE;
    dcb.fInX             = FALSE;
    dcb.fNull            = FALSE;         // keep NUL bytes (binary protocol)
    dcb.fAbortOnError    = FALSE;

    if (!SetCommState(FHandle, &dcb))
        throw Exception("SetCommState failed for " + CommName);

    // Log the EFFECTIVE settings - exactly what SPComm never let you see.
    AnsiString d;
    d.sprintf("%s Baud=%u Data=%d Parity=%d Stop=%d ParityCheck=%d "
              "Dtr=ON Rts=ON Flow=NONE",
              CommName.c_str(), (unsigned)dcb.BaudRate, (int)dcb.ByteSize,
              (int)dcb.Parity, (int)dcb.StopBits, (int)dcb.fParity);
    LogLine("OPEN", d, true);
}
//---------------------------------------------------------------------------
void __fastcall TMyComm::StartComm()
{
    if (FConnected)
        return;                          // idempotent - already open

    FLastFlushTick = GetTickCount();     // reset the log-flush timer

    AnsiString name = NormalizeName(CommName);

    FHandle = CreateFileA(name.c_str(),
                          GENERIC_READ | GENERIC_WRITE,
                          0,             // serial ports cannot be shared
                          NULL,
                          OPEN_EXISTING,
                          FILE_FLAG_OVERLAPPED,
                          NULL);
    if (FHandle == INVALID_HANDLE_VALUE)
    {
        DWORD gle = GetLastError();
        AnsiString msg = "Open " + name + " failed, GLE=" + IntToStr((int)gle);
        LogLine("ERR", msg, true);
        FlushLog();                      // make sure the failure reason hits disk
        throw Exception(msg);
    }

    SetupComm(FHandle, 8192, 8192);
    PurgeComm(FHandle,
              PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);

    // Reads are driven by WaitCommEvent + cbInQue, so ReadFile must return what
    // is queued without waiting.  Writes get a 1 s safety cap.
    COMMTIMEOUTS to;
    ZeroMemory(&to, sizeof(to));
    to.ReadIntervalTimeout         = MAXDWORD;
    to.ReadTotalTimeoutMultiplier  = 0;
    to.ReadTotalTimeoutConstant    = 0;
    to.WriteTotalTimeoutMultiplier = 0;
    to.WriteTotalTimeoutConstant   = 1000;
    SetCommTimeouts(FHandle, &to);

    try
    {
        ApplyDcb();
    }
    catch (...)
    {
        CloseHandle(FHandle);
        FHandle = INVALID_HANDLE_VALUE;
        throw;
    }

    ResetEvent(FStopEvent);
    FConnected  = true;
    FReadThread = new TCommReadThread(this);   // starts itself (see ctor)
}
//---------------------------------------------------------------------------
void __fastcall TMyComm::StopComm()
{
    if (!FConnected && FReadThread == NULL && FHandle == INVALID_HANDLE_VALUE)
    {
        FlushLog();                      // nothing open, but flush any backlog
        return;
    }

    // Release the worker: signal stop, and force any pending WaitCommEvent to
    // return immediately by clearing the comm mask.
    SetEvent(FStopEvent);
    if (FHandle != INVALID_HANDLE_VALUE)
    {
        SetCommMask(FHandle, 0);
        CancelIo(FHandle);
    }

    if (FReadThread)
    {
        FReadThread->Terminate();
        FReadThread->WaitFor();          // pumps Synchronize queue on main thread
        delete FReadThread;
        FReadThread = NULL;
    }

    if (FHandle != INVALID_HANDLE_VALUE)
    {
        PurgeComm(FHandle,
                  PURGE_RXCLEAR | PURGE_TXCLEAR | PURGE_RXABORT | PURGE_TXABORT);
        CloseHandle(FHandle);
        FHandle = INVALID_HANDLE_VALUE;
    }

    bool wasConnected = FConnected;
    FConnected = false;
    if (wasConnected)
        LogLine("CLOSE", CommName, true);
    FlushLog();                          // persist the CLOSE line + any backlog
}
//---------------------------------------------------------------------------
int __fastcall TMyComm::WriteCommData(const char* Buffer, int Length)
{
    if (!FConnected || FHandle == INVALID_HANDLE_VALUE || Length <= 0)
        return 0;

    int total = 0;
    EnterCriticalSection(&FWriteLock);

    OVERLAPPED ov;
    ZeroMemory(&ov, sizeof(ov));
    ov.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    try
    {
        DWORD nWritten = 0;
        BOOL ok = WriteFile(FHandle, Buffer, (DWORD)Length, &nWritten, &ov);
        if (!ok)
        {
            if (GetLastError() == ERROR_IO_PENDING)
            {
                if (GetOverlappedResult(FHandle, &ov, &nWritten, TRUE))
                    total = (int)nWritten;
                else
                    LogLine("ERR", "WriteFile overlapped failed", true);
            }
            else
            {
                LogLine("ERR", "WriteFile failed, GLE=" +
                        IntToStr((int)GetLastError()), true);
            }
        }
        else
        {
            total = (int)nWritten;
        }

        if (total > 0)
            LogLine("TX", BytesToHex(Buffer, total), true);
    }
    __finally
    {
        CloseHandle(ov.hEvent);
        LeaveCriticalSection(&FWriteLock);
    }
    return total;
}
//---------------------------------------------------------------------------
void __fastcall TMyComm::FireReceive(char* Data, Word Len)
{
    // When SyncReceive is true this runs on the main thread (via Synchronize);
    // otherwise it runs on the worker thread.
    if (EnableLog)
        LogLine("RX", BytesToHex(Data, (int)Len), true);
    if (OnReceiveData)
        OnReceiveData(this, (void*)Data, Len);
}
//---------------------------------------------------------------------------
AnsiString __fastcall TMyComm::DescribeCommError(DWORD e)
{
    AnsiString s = "CommError:";
    if (e & CE_RXOVER)   s += " RXOVER";    // input buffer overflow
    if (e & CE_OVERRUN)  s += " OVERRUN";   // hardware char overrun
    if (e & CE_RXPARITY) s += " PARITY";    // parity error
    if (e & CE_FRAME)    s += " FRAME";     // framing error
    if (e & CE_BREAK)    s += " BREAK";     // break detected
    if (e & CE_TXFULL)   s += " TXFULL";    // output buffer full
    AnsiString h;
    h.sprintf(" (0x%X)", (unsigned)e);
    return s + h;
}
//---------------------------------------------------------------------------
AnsiString TMyComm::BytesToHex(const char* Data, int Len)
{
    AnsiString s = "";
    for (int i = 0; i < Len; i++)
    {
        AnsiString b;
        b.sprintf("%02X ", (unsigned char)Data[i]);
        s += b;
    }
    return s.Trim();
}
//---------------------------------------------------------------------------
AnsiString __fastcall TMyComm::GetCurrentLogPath()
{
    // Preferred dated layout when LogDir is set; otherwise the single file.
    if (LogDir.Length() > 0)
    {
        SYSTEMTIME st;
        GetLocalTime(&st);
        AnsiString tag = (LogTag.Length() > 0) ? LogTag : AnsiString("Comm");
        AnsiString p;
        p.sprintf("%s\\%04d_%02d\\%s_%04d_%02d_%02d.csv",
                  LogDir.c_str(), st.wYear, st.wMonth, tag.c_str(),
                  st.wYear, st.wMonth, st.wDay);
        return p;
    }
    return LogFileName;
}
//---------------------------------------------------------------------------
void __fastcall TMyComm::FlushLocked()
{
    // Caller MUST hold FLogLock.
    if (FLogPending == 0 || FLogBuf.Length() == 0)
        return;

    AnsiString path = GetCurrentLogPath();
    if (path.Length() == 0)
    {
        FLogBuf = "";                    // nowhere to write - drop the backlog
        FLogPending = 0;
        return;
    }

    ForceDirectories(ExtractFilePath(path));
    bool isNew = !FileExists(path);

    FILE *fp = fopen(path.c_str(), "a");
    if (fp != NULL)
    {
        if (isNew)
            fputs("Date, Time, Category, Data\n", fp);
        fputs(FLogBuf.c_str(), fp);
        fclose(fp);
    }
    FLogBuf = "";
    FLogPending = 0;
    FLastFlushTick = GetTickCount();
}
//---------------------------------------------------------------------------
void __fastcall TMyComm::FlushLog()
{
    EnterCriticalSection(&FLogLock);
    try
    {
        FlushLocked();
    }
    __finally
    {
        LeaveCriticalSection(&FLogLock);
    }
}
//---------------------------------------------------------------------------
void __fastcall TMyComm::LogLine(const char* Category, const AnsiString& Text,
                                 bool fireEvent)
{
    if (!EnableLog)
        return;

    SYSTEMTIME st;
    GetLocalTime(&st);

    AnsiString line;
    line.sprintf("%04d-%02d-%02d, %02d:%02d:%02d.%03d, %s, %s",
                 st.wYear, st.wMonth, st.wDay,
                 st.wHour, st.wMinute, st.wSecond, st.wMilliseconds,
                 Category, Text.c_str());

    // Append to the in-memory buffer (fast) and flush only in batches, so the
    // RX worker path is never blocked by disk I/O on every single line.
    EnterCriticalSection(&FLogLock);
    try
    {
        FLogBuf += line;
        FLogBuf += "\n";
        FLogPending++;
        if (FLogPending >= FLOG_FLUSH_LINES ||
            (DWORD)(GetTickCount() - FLastFlushTick) >= FLOG_FLUSH_MS)
            FlushLocked();
    }
    __finally
    {
        LeaveCriticalSection(&FLogLock);
    }

    // OnLog event: only raised when the caller is on the main thread (RX path
    // with SyncReceive, plus open/close/TX), so the handler may touch the UI.
    if (fireEvent && OnLog)
        OnLog(this, line);
}

//===========================================================================
// TCommReadThread
//===========================================================================
__fastcall TCommReadThread::TCommReadThread(TMyComm* AOwner)
    : TThread(true)            // create SUSPENDED so FOwner is set before Execute
{
    FOwner    = AOwner;
    FChunk    = NULL;
    FChunkLen = 0;
    FreeOnTerminate = false;
    Resume();                  // now safe to run
}
//---------------------------------------------------------------------------
void __fastcall TCommReadThread::DoFireReceive()
{
    // Runs on the main VCL thread.
    FOwner->FireReceive(FChunk, FChunkLen);
}
//---------------------------------------------------------------------------
void __fastcall TCommReadThread::Execute()
{
    char buf[READ_BUF_SIZE];

    OVERLAPPED ovWait;
    ZeroMemory(&ovWait, sizeof(ovWait));
    ovWait.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    OVERLAPPED ovRead;
    ZeroMemory(&ovRead, sizeof(ovRead));
    ovRead.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);

    SetCommMask(FOwner->FHandle, EV_RXCHAR);

    while (!Terminated)
    {
        DWORD evtMask = 0;
        ResetEvent(ovWait.hEvent);

        BOOL ok = WaitCommEvent(FOwner->FHandle, &evtMask, &ovWait);
        if (!ok)
        {
            DWORD err = GetLastError();
            if (err == ERROR_IO_PENDING)
            {
                HANDLE h2[2];
                h2[0] = ovWait.hEvent;
                h2[1] = FOwner->FStopEvent;
                DWORD w = WaitForMultipleObjects(2, h2, FALSE, INFINITE);
                if (w != WAIT_OBJECT_0)        // stop signalled (or wait failed)
                {
                    CancelIo(FOwner->FHandle);
                    break;
                }
                DWORD dummy = 0;
                if (!GetOverlappedResult(FOwner->FHandle, &ovWait, &dummy, FALSE))
                {
                    FOwner->LogLine("CERR", "WaitCommEvent overlapped failed",
                                    false);
                    continue;
                }
            }
            else
            {
                FOwner->LogLine("CERR",
                    "WaitCommEvent failed, GLE=" + IntToStr((int)err), false);
                break;
            }
        }

        if (Terminated)
            break;

        // Drain every byte currently queued, reporting any line errors first.
        for (;;)
        {
            DWORD commErr = 0;
            COMSTAT cs;
            ZeroMemory(&cs, sizeof(cs));
            if (!ClearCommError(FOwner->FHandle, &commErr, &cs))
                break;
            if (commErr != 0)
                FOwner->LogLine("CERR", FOwner->DescribeCommError(commErr),
                                false);

            DWORD inQ = cs.cbInQue;
            if (inQ == 0)
                break;

            DWORD toRead = inQ;
            if (toRead > (DWORD)READ_BUF_SIZE)
                toRead = READ_BUF_SIZE;

            ResetEvent(ovRead.hEvent);
            DWORD nRead = 0;
            BOOL rok = ReadFile(FOwner->FHandle, buf, toRead, &nRead, &ovRead);
            if (!rok)
            {
                if (GetLastError() == ERROR_IO_PENDING)
                {
                    HANDLE h3[2];
                    h3[0] = ovRead.hEvent;
                    h3[1] = FOwner->FStopEvent;
                    DWORD w = WaitForMultipleObjects(2, h3, FALSE, INFINITE);
                    if (w != WAIT_OBJECT_0)
                    {
                        CancelIo(FOwner->FHandle);
                        break;
                    }
                    if (!GetOverlappedResult(FOwner->FHandle, &ovRead,
                                             &nRead, FALSE))
                        break;
                }
                else
                {
                    FOwner->LogLine("CERR",
                        "ReadFile failed, GLE=" + IntToStr((int)GetLastError()),
                        false);
                    break;
                }
            }

            // Hand the data over in <= RECEIVE_CHUNK pieces.
            DWORD off = 0;
            while (off < nRead && !Terminated)
            {
                DWORD piece = nRead - off;
                if (piece > (DWORD)RECEIVE_CHUNK)
                    piece = RECEIVE_CHUNK;

                FChunk    = buf + off;
                FChunkLen = (Word)piece;

                if (FOwner->SyncReceive)
                    Synchronize(DoFireReceive);     // -> main thread
                else
                    FOwner->FireReceive(FChunk, FChunkLen);

                off += piece;
            }

            if (Terminated)
                break;
        }
    }

    CloseHandle(ovWait.hEvent);
    CloseHandle(ovRead.hEvent);
}
//---------------------------------------------------------------------------
