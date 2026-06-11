---
name: ht160s-state-record-analysis
description: >
  Use when analyzing an HT160S_BCB "Store Hangup" state record snapshot to
  diagnose a hang/deadlock: decode per-module Task numbers, classify FROZEN vs
  CHURN modules, apply the SystemStart=0 timestamp caveat, and trace the
  SortArm-place / Auto-discharge threshold-mismatch deadlock. Also use when
  extending cStateRecordHT160 or the analyze-state-record.ps1 tool. Triggers:
  Store Hangup, state record, StateRecord, hang up, deadlock, TaskHistory.csv,
  MachineState.ini, Snapshot.ini, CurrentTasks.txt, cStateRecordHT160,
  SampleTasks, frozen module, idle churn, SelectPlaceAuto, DoPlaceToAuto,
  FindDischargeAuto, FullThisIC, analyze-state-record.
---

# HT160S State Record Analysis

How to read a HT160S_BCB **Store Hangup** snapshot and find why the machine
wedged. The machine is non-FSM: every module is a `switch(Task)` procedure in
`aXxx.cpp`. The state record samples each module's `Task` number over time.

## 1. Snapshot anatomy

Folder: `D:\HT160S_StateRecord\<yyyy-mm-dd hh_mm_ss>\` (written by
`cStateRecordHT160::TriggerSnapshot`, button "Store Hangup").

| File | Meaning |
|------|---------|
| `MachineState.ini` | `[System] RunMode/RunModeName/SystemStart/bCleanOut`, `[Recipe] Name`, `[Lot] LotNo`, `[Tasks] <module>=task` |
| `CurrentTasks.txt` | Module index table + current Task + last-change time |
| `TaskHistory.csv` | Per module, last 30 `(Time_k,Task_k)` pairs. **Time_0 is NEWEST.** A sample is recorded **only when Task changes** (`SampleTasks`). |
| `Snapshot.ini` | `TriggerReason` (Manual = operator pressed the button), `Time`, `Version` |
| `MachineConfig/` | Copied `system\` + active `recipe\` for reproduction |

Module order (index 0..6): **Empty, Loader1, Loader2, Auto1, TrayArm, SortArm,
Color** (the 7 entries in `UserMotion` action list).

## 2. The SystemStart=0 caveat (read this FIRST)

`TDataModule1::DoAllProcess()` (database.cpp) loops the module actions but:

```cpp
if(HSys.Sys.SystemStart==false) { HSys.DecStopAllMotor(); break; }  // breaks WHOLE loop
```

So when `SystemStart=0`, **no module Execute runs**, and `SampleTasks()` records
nothing new. Therefore in a snapshot with `SystemStart=0`:

- TaskHistory is **frozen at the last RUNNING moment**, not the snapshot time.
- The newest `Time_0` across modules = when the line last executed.
- Operator typically pressed Pause/Stop *after* noticing the hang, then pressed
  Store Hangup - so `Snapshot.Time` is later than the real stall.

Also each `actXxxExecute` wrapper gates on
`RunMode==Run_Normal|OneCycle|CleanOut|TrayFeed`. `RunMode=Home(1)` would also
stop modules.

## 3. Classify each module

- **CHURN (idle spin)**: >=10 Task changes packed into <200 ms. The module has
  no work and re-scans every MainProc tick (e.g. Auto cycles `1->100->1000->3000->1`,
  Color `1->10->100->1`). NOT a hang by itself - it just means "this module is
  idle and the thing that should feed it is stalled."
- **FROZEN**: last Task change is much older (seconds) than the globally-newest
  change. The module is stuck waiting on a condition that never becomes true.
- **A real stall** = FROZEN on a *cross-module handshake phase* (see §5) while
  other modules CHURN idle.

Key insight: a well-behaved idle module that genuinely rests at one Task also
looks "frozen" (no new samples). Use the per-module phase table to tell
"resting idle" (e.g. TrayArm 100 DecideJob=NONE) from "blocked mid-action"
(e.g. SortArm 200 mid-place).

## 4. Per-module Task -> phase map (HT160S_Program_BCB_V1.0.0.0)

Keep in sync with the `switch(Task)` in each source file.

**Empty** (`aEmpty.cpp DoEmpty`): 1/10 chain; 100 idle dispatch
(GoUp/GoDown/Feed decide); 1000 DoGoDownTray; 2000 DoFeedTray(front->rear pickup);
3000 DoGoUpTray; 7000 GoDown inner front-confirm.

**Loader1/Loader2** (`aLoader.cpp DoLoader`): 1/10 chain; 100 feed-or-CCD decide;
1000 DoFeedTray(front->Y car); 2000 DoCcdCheck(Top CCD); 3000 post-CCD
**LS_READY_SORT** wait SortArm pick / discharge gate; 4000 DoDischargeTray(empty
tray->rear).

**Auto1** (`aAuto1To6.cpp DoAuto`, Auto1 is the sampled representative of Auto1..6):
1 idle->100; 100 CheckAutoTray; 1000 FindFeedAuto decide; 2000 DoFeedTray(rear->car);
3000 CheckAutoTray + FindDischargeAuto decide; 4000 DoDischargeTray; 5000 CleanOut.

**TrayArm** (`aTrayArm.cpp DoTrayArm`): 1/10 chain; 100 idle (HasTray + DecideJob);
1000 DoPick; 2000 DoPlace.

**SortArm** (`aSortArm.cpp DoSortArm`): 1 idle pick-decide
(GetSortingLoaderNo); 100 DoPickFromLoader; 200 **DoPlaceToAuto** (SelectPlaceAuto
-> place held IC). PlaceTask sub-states 1/10/20/30/40/50/60.

**Color** (`aColor.cpp DoColor`): 1 idle->10; 10 Refresh + SortBin dispatch;
100 supply dispatch; 1000 DoSupplyTray; 1200 DoGoDownTray; 1500 DoReleaseTray;
2000 DoSortBin.

## 5. Known deadlock: SortArm-place / Auto-discharge THRESHOLD MISMATCH

**Signature**: SortArm FROZEN at Task=200; Auto* CHURN idle; TrayArm idle at 100;
Empty idle at 100; Loader1 FROZEN at 3000.

**Root cause** (FACT, source-verified):
- SortArm place needs `FindPlaceCells()` true = a contiguous run of `EMPTY_IC`
  cells fitting the held 4-sucker pattern (`aSortArm.cpp` ~607). When none fits
  in any Auto, `SelectPlaceAuto()` returns false -> `DoPlaceToAuto` stays
  `PlaceTask=1` -> SortArm wedges at 200 holding IC.
- Auto discharge needs `bFullIC` = `TrayMotor->FullThisIC(HAS_OK_IC)` = **every**
  cell a good IC (`aAuto1To6.cpp CheckAutoTray` ~357; `FindDischargeAuto` ~379).
- The "no place" threshold (SortArm) is LOOSER than the "must discharge"
  threshold (Auto). A working tray with **fragmented / wrong-bin / partial-last
  -column** empties is *neither placeable nor dischargeable* -> permanent stall.
- Cascade: car keeps its tray (`bCarHasTray=true`) -> `GetTrayRequest()=eTrayReqNone`
  (`aAuto1To6.cpp` ~849) -> `TrayArm DecideJob()=TAJOB_NONE` -> TrayArm idle ->
  Empty's ready rear tray never picked -> Loader1 LS_READY_SORT waits forever for
  SortArm to pick the rest of its IC.

**Reproduces** with `Lot=SIMU_LOT_A` (simulation bin data scatters ICs).

**Candidate fixes** (motion handshake = safety-critical, do NOT auto-apply;
get user decision):
- A) Auto discharges when the tray can no longer accept the pending held-IC
  pattern (not only at 100% HAS_OK_IC).
- B) SortArm single-sucker fallback: place into any remaining `EMPTY_IC` cell
  when the multi-sucker contiguous run fails.
- C) Watchdog: `SelectPlaceAuto` false for N s -> force target Auto discharge.

## 6. The tool

`scripts/ops/analyze-state-record.ps1` does §2-§5 automatically.

```powershell
# Newest snapshot under D:\HT160S_StateRecord
powershell -ExecutionPolicy Bypass -File .\scripts\ops\analyze-state-record.ps1

# A specific snapshot, write a report too
powershell -ExecutionPolicy Bypass -File .\scripts\ops\analyze-state-record.ps1 `
    -Path "D:\HT160S_StateRecord\2026-06-09 10_41_28" `
    -OutFile .\docs\hang\2026-06-09_10_41_28.txt
```

Read-only on the snapshot. `-FreezeSeconds` tunes the frozen threshold (default 5).
When you change a module's `switch(Task)` numbers, update both the `$TaskMaps`
in the tool and §4 here.

## 7. Analysis workflow (anti-hallucination)

1. Run the tool; note RunMode/SystemStart/Lot.
2. Apply the §2 caveat: trust the newest `Time_0`, not `Snapshot.Time`.
3. List FROZEN-on-handshake modules + CHURN-idle modules.
4. Pick the **most upstream blocked actor** (here SortArm place) and read its
   wait function in source **before** asserting a cause. Confirm `.h` + `.cpp`.
5. Walk the cascade (place -> discharge -> request -> deliver) to confirm the
   circular wait. Label FACT (read) vs INFER (derived).
6. Propose fixes; do NOT edit motion handshake code without user sign-off.
