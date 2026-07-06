# UPH suite + lastdata persistence + BinICCnt — plan (2026-07-06)

Follow-up to commit `6eafc8f` (per-lot counter reset + TotalIC wiring). Implements
the user-approved items, referencing HT172 (`D:\HT172\HT172_Program_V1.0.25.0_20260420`).

## Decisions (user-confirmed)
- **Total UPH = HT172 aggregate** `TotalIC / (elapsed - pause) hours` (`GetCalculateUPH`,
  csystem.cpp:1549). NOT arithmetic mean of per-tray UPHs (that inflates on small/fast
  trays: 100.99 real vs 150 mean). The genuinely consistent average is time-weighted,
  which equals the aggregate.
- **Per-tray UPH** = that tray's IC / that tray's active time; logged for diagnostics.
- **Log layout = month bucket + folder-per-lot**, aged by `PruneFolderTree`.

## Hard constraint : aAuto1To6.cpp is contended
A concurrent session holds uncommitted edits to `aAuto1To6.cpp/.h`
(`docs/plan/auto1to6-cleanout-finish-live-predicate-plan-20260706.md`). Shared worktree
=> we must NOT edit aAuto1To6 (a `git commit --only` there would grab their work).
=> Per-tray timing uses a **non-invasive observer** that only READS the public
`TAutoModule` API (`GetStationCount/GetStationStatus/GetWorkingTrayID`) + globals.

## HT172 reference
- `GetCalculateUPH` (csystem.cpp:1186): `TotalIC / dHour`, `dHour=(end-Start-Pause)*24`.
- Lot End (csystem.cpp:880-894): compute UPH -> grid -> CSV
  `D:\HT-172_Log\UPHLog\YYYY_MM\{id}_YYYY_MM_DD.csv` -> `RecordProcess("End of Lot: ...")`.
- BinICCnt (aSortArm.cpp:874 / aMagArm.cpp:1130): `BinICCnt[Bin]++; TrayICCnt[t]++; TotalIC++`
  together at the place point.
- lastdata: read at startup (main.cpp:317), write at shutdown (main.cpp:578), Clear Count
  (main.cpp:2063 + Auto172.cpp:184), IO edits (iosetview.cpp:1917). Lot Start clears via
  `spbClearLoadCountClick` (main.cpp:2732).

## HT160 implementation

### Item 8 — BinICCnt
`aLoader.cpp` case 5500 success path (~:1645, next to `LotRegistry.OnSorted(HitLotIndex,Bin)`
which already does per-lot `iBinCount[Bin]++`): add
`if(Bin>=0 && Bin<TEST_MAX_BIN) tRunData.BinICCnt[Bin]++;`. Scan-time (like iTotalSorted),
a deliberate HT160 adaptation vs 172's place-time. Zeroed by ResetPerLotProductionCounters.

### Item 3+5 — per-tray + lot UPH logger (in cprod.cpp/.h, NO new project file)
Free functions (avoid ht160s.bpr edit): `TrayUphLog_OnLotStart/Tick/OnLotEnd/PruneOld`.
- Observer state per Auto: prevStatus, trayStartTime, trayStartPause(=tUPH_PauseTime),
  trayStartIC(=TrayICCnt[eAuto1+i]), trayID.
- Tick (every MainProc cycle): rising into `AS_SORTING` opens a window; leaving `AS_SORTING`
  closes it -> if trayIC>0 append row + count.
- Files under `<LogRoot>\UPHLog\YYYY_MM\{LotID}__YYYYMMDD_HHNNSS\`:
  `tray_uph.csv` (DataTime,LotID,Auto,TrayID,TrayIC,TrayStart,TrayEnd,DurationSec,TrayUPH)
  `lot_summary.csv` (LotEndTime,LotID,TotalIC,TotalUPH,TrayCount).
- Prune whole month buckets via `cCsvDailyLog::PruneFolderTree(root, iLogRetentionUPHLogDays)`.

### Item 2 — Lot End total UPH to EventLog
`btnLotEndClick` (main.cpp:2159): before clearing, `tRunData.UPH=GetCalculateUPH(Now())`,
`RecordProcess("End of Lot: Lot=..., TotalIC=..., UPH=...")`, `TrayUphLog_OnLotEnd(...)`.

### Item 7 — lastdata persistence + Clear Count
- `WriteLastDataIni()` at `FormClose` (main.cpp:616) and at each Lot End.
- SECS `CLEARCOUNT` host command (uHGemHT160.cpp remote dispatch) ->
  `ResetPerLotProductionCounters()` + `WriteLastDataIni()` (busy if SystemStart).
- (ReadLastDataIni already runs at startup.) Persists CURRENT-lot counts across power
  cycle; Lot Start / Clear Count zero them. Note: tUPH_PauseTime is a separate global,
  not persisted (pause across power loss is a corner case, deferred).
- GeneralSetting: `iLogRetentionUPHLogDays` (default 180, [LogRetention] UPHLogDays).

### Wiring points
- btnLotStartClick + SECS LOTSTART: `TrayUphLog_OnLotStart(FirstLot)` beside the reset.
- csystem.cpp:177 (after PollLotDataWebApi): `TrayUphLog_Tick()`.
- RestoreLastWorkOrder startup prune (main.cpp:2867): `TrayUphLog_PruneOld()`.

## Build + commit
- `-Full` (header + new logic). Encoding check. Real-machine `#ifndef SOFT_SIMULATE` not
  touched here.
- `git commit --only` the exact files (NEVER aAuto1To6.*). Push.
