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
        bool        bStuckFired;            //AI(ht160s-obsv-p1) : one auto-snapshot per stuck episode
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
    //AI(ht160s-lot-webapi) 20260612 : full Lot work order as JSON (lot names +
    //  every 2D code with Bin/HBin/SBin/RetestCode/DiePass), so a snapshot carries
    //  the actual sort data, not just the [LotList] counts in MachineState.ini.
    void       WriteLotDataJson(AnsiString Path, AnsiString Reason, AnsiString Stamp);
    //AI(ht160s-state-record-analysis) 20260612 : motor positions + SortArm sub-task +
    //  sucker vacuum, so "sucker not raised during move" is diagnosable from a snapshot.
    void       WriteMotionDetailIni(AnsiString Path);
    //AI(ht160s-state-record-analysis) 20260616 : SortArm held-IC routing + per-Auto
    //  working-tray cell map, so the place/discharge threshold-mismatch deadlock is
    //  diagnosable offline (which cell blocks the held pattern) without re-running config.
    void       WriteSortArmDecisionTxt(AnsiString Path);
    //AI(ht160s-state-record-analysis) 20260622 : Color/Empty/Loader inner-state + config gates -> FeederDecision.txt (triggered-only)
    void       WriteFeederDecisionTxt(AnsiString Path);
    void       CaptureConfig(AnsiString DstRootWithSlash);
    //AI(ht160s-secsgem) 20260611 : package today's SECS/GEM log (gated by CosFunction.bUseSecsGem)
    void       CaptureSecsLog(AnsiString DstRootWithSlash);
    //AI(ht160s-lot-webapi) 20260612 : package today's Lot WebAPI log if one exists
    //  (the log only exists when a pull actually ran, i.e. the feature was in use).
    void       CaptureWebApiLog(AnsiString DstRootWithSlash);
    //AI(ht160s-obsv-p1) 20260720 : ship today's+yesterday's EventLog CSV inside the zip
    void       CaptureEventLog(AnsiString DstRootWithSlash);
    //AI(ht160s-obsv-p1) 20260720 : live consumer of the StuckMs computation (auto snapshot)
    void       CheckStuckWatchdog();

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
