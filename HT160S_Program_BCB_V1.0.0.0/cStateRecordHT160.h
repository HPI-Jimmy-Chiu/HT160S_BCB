//---------------------------------------------------------------------------
// cStateRecordHT160 : triggered, packaged machine-state snapshot recorder.
//
// Complements cStepTrace (the continuous numeric step "movie"). On a manual
// trigger (the main-form "Store Hangup" button) it captures, for offline
// analysis:
//   - TaskHistory.csv  : per-module recent Task transitions (PRIMARY analysis)
//   - CurrentTasks.txt : current Task of every module + last change time
//   - MachineState.ini : RunMode / SystemStart / Recipe / Lot / live Tags
//   - Snapshot.ini     : trigger reason / time / version
//   - MachineConfig\   : full copy of system\ config + current recipe folder
// then compresses the whole folder into  D:\HT160S_StateRecord\<stamp>.zip .
//
// No FSM. SampleTasks() is a cheap non-blocking call made once per main cycle
// from DataModule1::DoAllProcess(); TriggerSnapshot() runs on the UI thread
// from the button click. Both run on the single VCL main thread.
//---------------------------------------------------------------------------
#ifndef cStateRecordHT160H
#define cStateRecordHT160H
//---------------------------------------------------------------------------
#include <vcl.h>
//---------------------------------------------------------------------------
#define SR_MAX_MODULE   16   // upper bound on UserMotion actions tracked
#define SR_MAX_HISTORY  30   // Task transitions kept per module (newest first)
//---------------------------------------------------------------------------
class cStateRecordHT160
{
private:
    struct TTaskSample
    {
        TDateTime Time;
        int       Task;
    };
    struct TModuleState
    {
        AnsiString  Name;
        int         LastTask;
        bool        bHasLast;
        TTaskSample Hist[SR_MAX_HISTORY];   // circular buffer
        int         HistHead;               // next write index
        int         HistCount;              // filled entries (<= SR_MAX_HISTORY)
    };

    TModuleState Modules[SR_MAX_MODULE];
    int          ModuleCount;
    bool         bInited;
    AnsiString   SaveRoot;                  // e.g. "D:\\HT160S_StateRecord\\"

    void       EnsureInited();
    void       PushSample(int ModuleIndex, int Task);
    AnsiString RunModeText(int Mode);
    AnsiString MakeStamp();
    AnsiString GetProjectRoot();
    AnsiString Get7ZipPath();
    bool       CopyOneFile(AnsiString Src, AnsiString Dst);
    int        CopyFolderFiles(AnsiString SrcDir, AnsiString DstDirWithSlash);
    bool       DeleteFolderRecursive(AnsiString Dir);
    bool       CompressFolder(AnsiString SrcDirWithSlash, AnsiString ZipPath);
    void       WriteSnapshotIni(AnsiString Path, AnsiString Reason, AnsiString Stamp);
    void       WriteTaskHistoryCsv(AnsiString Path);
    void       WriteCurrentTasksTxt(AnsiString Path);
    void       WriteMachineStateIni(AnsiString Path, AnsiString Reason, AnsiString Stamp);
    void       CaptureConfig(AnsiString DstRootWithSlash);
    //AI(ht160s-secsgem) 20260611 : package today's SECS/GEM log (gated by CosFunction.bUseSecsGem)
    void       CaptureSecsLog(AnsiString DstRootWithSlash);

public:
    cStateRecordHT160();
    ~cStateRecordHT160();
    void SetSaveFolder(AnsiString Folder);
    void SampleTasks();                     // call once per cycle (cheap)
    bool TriggerSnapshot(AnsiString Reason); // returns true if zip created
    AnsiString LastSnapshotZip;             // full path of the last created zip (for Explorer /select)
};
//---------------------------------------------------------------------------
extern cStateRecordHT160 *gStateRecord;
//---------------------------------------------------------------------------
#endif
