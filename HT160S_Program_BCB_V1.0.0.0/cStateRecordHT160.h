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
//   - MotionDetail.ini : per-motor cmd/enc/tgt + amplifier status + axis lock owner
//   - IoDetail.txt     : LIVE sweep of every cylinder / sensor / switch / sucker
//   - LotData.json     : lot registry + every 2D code with its Bin data
//   - FeederDecision.txt / SortArmDecision.txt : per-module latched decision state
//   - EventLog\ SecsLog\ WebApi log : the narrative that goes with the state
//   - ProductionLog\ SoterOutput\   : recent per-IC production rows (which 2D placed)
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
//AI(ht160s-obsv-p2) 20260806 : SLOW-ring dwell threshold. The plain history ring floods in an
//idle spin - on 2026-08-05 17:30 the Auto1/Color rings held ~100 ms of 1->100->1000->3000
//cycling and the module's last REAL action was gone. A second ring records only tasks that
//were actually DWELT IN (>= this many ms when left), so it survives minutes of idle spin.
//200 ms sits well between the spin period (one main cycle, ~10-20 ms) and the shortest real
//action (a motor move / settle delay, hundreds of ms). The pre-pick wait gate idles in
//exactly this spin shape by design, so the flood is now the NORMAL waiting posture -
//without the slow ring every gated wait would wipe the forensic history.
#define SR_SLOW_DWELL_MS 200
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
        TDateTime   WatchBase;              //AI(ht160s-obsv) 20260724 : stuck-clock base (last task change OR last production resume) - immune to Pause/Stop wall-clock inflation
        //AI(ht160s-obsv-p2) 20260806 : SLOW ring - only tasks dwelt in >= SR_SLOW_DWELL_MS
        //(entry = the task + the time it was ENTERED, pushed when it is LEFT). Survives the
        //idle-spin flood that wipes Hist[] within ~100 ms; see the define's note.
        TTaskSample Slow[SR_MAX_HISTORY];
        int         SlowHead;
        int         SlowCount;
    };

    TModuleState Modules[SR_MAX_MODULE];
    int          ModuleCount;
    bool         bInited;
    bool         bPrevRunGate;              //AI(ht160s-obsv) 20260724 : prev state of the production gate; rebase WatchBase on its false->true edge
    //AI(ht160s-obsv-p2) 20260806 : run-gate edge journal, dumped into CurrentTasks.txt +
    //MachineState.ini so a snapshot SELF-DESCRIBES whether the machine was running and for
    //how long. Both misread 2026-08-05 snapshots (16:14 / 17:30) needed an EventLog
    //cross-read to discover they were taken 9/14 ms after a resume that followed a >5 min
    //pause - with these fields that fact is on the first line of the dump.
    bool         bRunGateNow;
    TDateTime    tRunGateRise;              // 0 = never rose this run
    TDateTime    tRunGateFall;              // 0 = never fell this run
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
    //AI(ht160s-state-record-analysis) 20260801 : whole-registry IO photo - every cylinder
    //  (out bit + BOTH reed sensors + a derived verdict), every sensor (LIVE level, not a
    //  module latch), every switch, every sucker. Before this, a snapshot carried 5 of 39
    //  cylinder out-bits and no reed at all, and the ~18 sensor values it did carry were
    //  mostly module beliefs - which is why the 07-30 Loader investigation had to reach
    //  every IsSupplySourceDry() conclusion by elimination.
    void       WriteIoDetailTxt(AnsiString Path);
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
    //AI(ht160s-state-record) 20260807 : copy every *.csv in SrcDir whose last-write
    //  time is >= MinWrite (shared filter for the two production captures below).
    int        CopyCsvFilesSince(AnsiString SrcDir, AnsiString DstDirWithSlash, TDateTime MinWrite);
    //AI(ht160s-state-record) 20260807 : ship recent Production_Log output. LotData.json
    //  carries the LOADED work order only (per-lot sorted COUNTS, no per-IC produced
    //  flag) - WHICH 2D codes were actually produced is only answerable from the
    //  Production_Log rows (Code2D + Which Auto + Unload_Time), so package them.
    void       CaptureProductionLog(AnsiString DstRootWithSlash);
    //AI(ht160s-state-record) 20260807 : ship Soter (KYEC per-unit) CSVs flushed since
    //  yesterday, from the month-bucketed archive folder.
    void       CaptureSoterOutput(AnsiString DstRootWithSlash);
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
