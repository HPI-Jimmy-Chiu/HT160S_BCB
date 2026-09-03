//---------------------------------------------------------------------------
// MyComm.h
//
// Self-built serial-port component intended to replace the closed-source
// SPComm "TComm" component.  Design goals:
//
//   1. Source-level, API-compatible drop-in: same property / method / event
//      names and the same OnReceiveData closure signature as SPComm::TComm,
//      so existing call sites need minimal edits.
//
//   2. Full physical-layer diagnostics that SPComm hides: the effective DCB at
//      open time, every TX/RX chunk in hex with millisecond timestamps, and
//      line errors reported by ClearCommError (framing / parity / overrun ...).
//
//   3. Deterministic threading: by default the OnReceiveData event is
//      marshalled back to the main VCL thread (Synchronize) so existing
//      handlers that touch the UI / shared flags are thread-safe.  This is the
//      single most important behaviour to get right for stability.
//
// Logging is BUFFERED (see MyComm.cpp): lines accumulate in memory and are
// written to disk in batches, so high-volume traffic never blocks the RX hot
// path.  Files follow the house EventLog layout with daily rotation.
//
// To allow this class to COEXIST with SPComm during a phased migration, all
// enum / type names are unique (TCommParity / TCommByteSize / TCommStopBits)
// and do NOT clash with the SPComm globals (TParity / None / _8 / _1).
//
// Encoding: ASCII only - all comments in English (see ENCODING_RULES.md).
//---------------------------------------------------------------------------
#ifndef MyCommH
#define MyCommH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <SysUtils.hpp>
#include <windows.h>
//---------------------------------------------------------------------------

// Parity values - the integer values map directly onto the Win32 DCB.Parity
// field (NOPARITY=0 .. SPACEPARITY=4), so no translation table is needed.
enum TCommParity { cpNone = 0, cpOdd = 1, cpEven = 2, cpMark = 3, cpSpace = 4 };

// Data bits - value is the literal bit count for DCB.ByteSize.
enum TCommByteSize { cbs5 = 5, cbs6 = 6, cbs7 = 7, cbs8 = 8 };

// Stop bits.
enum TCommStopBits { csb1 = 0, csb1_5 = 1, csb2 = 2 };

// OnReceiveData closure - identical signature to SPComm::TComm::OnReceiveData
// so existing handlers, e.g.
//     void __fastcall TMyBinDispCtrl::CommBinReceiveData(TObject*, Pointer, WORD)
// bind to it without any change.
typedef void __fastcall (__closure *TCommReceiveDataEvent)
    (System::TObject* Sender, void* Buffer, Word BufferLength);

// Optional log sink - fired (on the main thread only) for diagnostic lines so
// the caller can mirror them into a Memo.  The file log is the source of truth.
typedef void __fastcall (__closure *TCommLogEvent)
    (System::TObject* Sender, System::AnsiString Line);

class TMyComm;   // forward declaration

//---------------------------------------------------------------------------
// Internal worker thread.
//
// Blocks efficiently on WaitCommEvent(EV_RXCHAR) (no CPU spin), then drains the
// whole input queue and hands each chunk to the owner.  Not used directly.
//---------------------------------------------------------------------------
class TCommReadThread : public TThread
{
private:
    TMyComm*  FOwner;
    char*     FChunk;       // pointer to current chunk (valid during Synchronize)
    Word      FChunkLen;    // length of current chunk
    void __fastcall DoFireReceive();   // executes on the main thread
protected:
    void __fastcall Execute();
public:
    __fastcall TCommReadThread(TMyComm* AOwner);
};

//---------------------------------------------------------------------------
// TMyComm - the SPComm-compatible serial port.
//---------------------------------------------------------------------------
class TMyComm : public TComponent
{
    friend class TCommReadThread;
private:
    HANDLE              FHandle;        // INVALID_HANDLE_VALUE when closed
    HANDLE              FStopEvent;     // manual-reset; signals the read loop to quit
    TCommReadThread*    FReadThread;
    CRITICAL_SECTION    FWriteLock;     // serialises WriteCommData callers
    CRITICAL_SECTION    FLogLock;       // serialises log buffer + file access
    bool                FConnected;

    // Buffered logging.  Lines accumulate in FLogBuf and are flushed to disk in
    // batches (every FLOG_FLUSH_LINES lines or FLOG_FLUSH_MS ms - see the .cpp)
    // so disk I/O never sits in the RX hot path.  FlushLog() forces a write.
    AnsiString          FLogBuf;
    int                 FLogPending;
    DWORD               FLastFlushTick;

    void      __fastcall ApplyDcb();                         // configure the port
    void      __fastcall LogLine(const char* Category,
                                 const AnsiString& Text,
                                 bool fireEvent = true);
    void      __fastcall FlushLocked();                      // assumes FLogLock held
    AnsiString __fastcall GetCurrentLogPath();               // dated path or LogFileName
    AnsiString __fastcall DescribeCommError(DWORD e);        // CE_* flags -> text
    void      __fastcall FireReceive(char* Data, Word Len);  // log RX + raise event
    static AnsiString    BytesToHex(const char* Data, int Len);
    AnsiString __fastcall NormalizeName(AnsiString Name);    // "COM12" -> "\\.\COM12"

public:
    __fastcall          TMyComm(TComponent* Owner);
    __fastcall virtual ~TMyComm();

    // --- Configuration (set BEFORE StartComm; same names as SPComm) ----------
    AnsiString      CommName;          // e.g. "\\\\.\\COM12" or "COM12"
    int             BaudRate;          // e.g. 115200
    TCommParity     Parity;            // default cpNone
    bool            ParityCheck;       // default false
    TCommByteSize   ByteSize;          // default cbs8
    TCommStopBits   StopBits;          // default csb1

    // --- Events --------------------------------------------------------------
    TCommReceiveDataEvent OnReceiveData;   // same role as SPComm OnReceiveData
    TCommLogEvent         OnLog;           // optional diagnostic mirror

    // --- Diagnostics ---------------------------------------------------------
    bool            EnableLog;         // master switch (default true)
    // Logging destination.  Preferred: set LogDir (+ optional LogTag) for the
    // EventLog-style layout  <LogDir>\YYYY_MM\<LogTag>_YYYY_MM_DD.csv  with
    // daily rotation.  If LogDir is empty, the single file LogFileName is used
    // (handy for the standalone tester).
    AnsiString      LogDir;            // base directory; enables dated rotation
    AnsiString      LogTag;            // filename prefix (e.g. "Store" / "Mag")
    AnsiString      LogFileName;       // single-file fallback when LogDir is ""
    bool            SyncReceive;       // true (default): OnReceiveData on main
                                       // thread; false: on worker (SPComm-like)

    // --- API (same names / roles as SPComm::TComm) ---------------------------
    void __fastcall StartComm();                              // throws on failure
    void __fastcall StopComm();
    int  __fastcall WriteCommData(const char* Buffer, int Length);
    void __fastcall FlushLog();                               // force buffered lines to disk

    __property bool Connected = { read = FConnected };
};
//---------------------------------------------------------------------------
#endif
