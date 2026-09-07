---
name: ht160s-loader-flow
description: "Use when working on the HT160S_BCB Loader module (TLoaderModule in aLoader.cpp/.h): tray feed, Top CCD scan, discharge to rear, dual-side coordination, front-owner arbitration, or Loader<->SortArm handshake. Triggers: TLoaderModule, DoLoader, Loader1, Loader2, LoadNewTray, Top CCD, IsLoaderReadyForSort, GetSortingLoaderNo, SnLoader_InputHasTray, front owner, rear tray, NotifyTrayArmPickRearTray."
---

# HT160S Loader Flow Skill

## Purpose

Maintain and extend the HT160S_BCB Loader module. The Loader feeds trays, runs the Top CCD scan, and discharges scanned trays toward the rear pickup area for SortArm/TrayArm. This is a **non-FSM** procedural `switch(Task)` module — do not convert it to FSMRunner.

## Scope

- Target source: `HT160S_Program_BCB_V1.0.0.0/aLoader.cpp`, `aLoader.h`
- Runtime wiring: `ht160s.cpp` (`InitializeLoaderModule()` / `ShutdownLoaderModule()`)
- Dispatch: `database.cpp` `actLoader1Execute` -> `DoLoader(1, P->Tag)`, `actLoader2Execute` -> `DoLoader(2, P->Tag)`
- Reference only: `D:\HT172\HT172_Program_V1.0.25.0_20260420` (read-only, do not edit)

## Architecture

- Global singleton `TLoaderModule *LoaderModule`.
- Two-side design: `Side[0]` = Loader1, `Side[1]` = Loader2, each holding its own `FeedTask` / `CcdTask` / `DischargeTask`.
- Public entry: `void DoLoader(int LoaderNo, int &Task)` — non-blocking, returns by reference Task. Called every scan from `database.cpp` action handlers.
- Three private sub-flows, each `switch(Task)` returning `true` when done:
  - `DoFeedTray(LoaderNo, Flag)` — feed a tray into position.
  - `DoCcdCheck(LoaderNo, Flag)` — Top CCD scan cell-by-cell.
  - `DoDischargeTray(LoaderNo, Flag)` — push scanned tray to rear/output.

## Hardware Binding (database.h/.cpp)

Sensors added for Loader:
- `SnLoader_InputHasTray`, `SnLoader_InputFullTray`, `SnLoader_Inputend`
- `SnLoader_TrayPos1`, `SnLoader_TrayPos2`
- `SnLoader_OutputBottomHasTray`

> **Spatial-truth warning (user memory 雷 #2):** `Input*` = Front stacking area (operator load side); `Output*`/rear = pickup area (TrayArm takes away). `Input/Output` are logical roles, NOT physical front/rear directions. Confirm against `machine-spatial-truth.md` before wiring new sensors.

## Cross-Module Handshake

- `IsLoaderReadyForSort(int LoaderNo)` — tells SortArm a Loader side has a scanned tray ready to pick.
- `GetSortingLoaderNo()` — which side SortArm should currently service.
- `NotifyTrayArmPickRearTray()` — signal that the rear tray was taken.
- `IsRearHasTray()` — rear occupancy guard.
- Front-owner arbitration: `AcquireFrontOwner` / `ReleaseFrontOwner` / `iFrontOwner` prevent both sides driving the shared front mechanism at once.

## Dual-Side Ordering / Old HT160 Reference

- There is no absolute "Loader L must always feed first" or "Loader R must always feed first" rule in old HT160. Runtime priority comes from the `UserMotion` action order plus mutual flags.
- HT160S_BCB `database.dfm` action order is `actLoader1` then `actLoader2`, so Loader1 naturally gets the first scan opportunity when both sides are equally idle.
- Old HT160 feed interlock:
  - Loader1 feed returns immediately while `bLoader_2_CCD_Busy == true`.
  - Loader2 feed returns immediately while `bLoader_1_CCD_Busy == true`.
  - Each side sets its own busy flag when it starts feeding; the flag is released after the Loader Y axis passes `SystemSetup.iLoaderCarSafePosition` during CCD motion, or at discharge cleanup.
- Old HT160 CCD-complete behavior: when a tray still has non-empty IC data after CCD scan, the Loader does not alarm or discharge. It sets the matching `Loaader_1_Sorting` / `Loaader_2_Sorting` readiness flag only when the other side is not already sorting, then waits for SortArm to pick until the tray becomes `FullThisIC(EMPTY_IC)`.
- Old HT160 rear discharge guards: before discharging, each side checks the other Loader Y encoder relation and `SnLoader_OutputBottomHasTray`; it also waits while `Loader_Rear_Has_Tray` is true. Do not replace this with a blind discharge or a tray-has-IC alarm.
- In HT160S_BCB, preserve the same semantics with status values: `LS_READY_SORT` means wait for SortArm, `LS_SORTING` means SortArm owns Loader Y, and only an all-`EMPTY_IC` tray may enter `LS_ToRear` / `DoDischargeTray`.

## Tray Map / CCD Behavior

- Tray map uses the in-memory active recipe `TrayForm`; clamps to `Data[20][50]`.
- Access tray cells as `Tray.Data[x][y]`; write via `SetTraySingleData(x, y, data)`.
- CCD result rules (current implementation):
  - `tFunction.UseCCD == false` OR simulate/dummy -> Top CCD cells become `HAS_OK_IC`.
  - `tFunction.UseCCD == true` -> module raises `Top CCD API not ready` until a real CCD adapter exists.
- Cell iteration: `FindNextCcdCell` walks the tray; `bCcdLeftToRight` controls scan direction.

## Common Regression: Premature Tray-Has-IC Alarm

- Symptom: Loader1 feeds/scans first in simulation, Loader2 has not fed yet, then the system raises `Loader Tray has IC,please remove`.
- Root pattern: `DoLoader()` advanced from CCD-complete directly to discharge/error while the tray still contained `HAS_OK_IC` cells. That is wrong; non-empty cells mean SortArm must pick, not that the operator must remove the tray.
- Correct rule: in `Task=3000`, if the active tray is not all `EMPTY_IC`, keep `State->Status=LS_READY_SORT` and wait. Start `DoDischargeTray()` only after SortArm has cleared all cells to `EMPTY_IC`.
- In soft simulation, do not let `SnLoader_OutputBottomHasTray` absence/false state raise the tray-has-IC alarm during discharge; hardware presence checks should be skipped by `IsSoftSimulate()`.

## Run-Mode / Simulate

- `IsSoftSimulate()` gates motor/sensor actuation. Do NOT use `bSoft_Simulate` as a substitute for runtime dummy semantics.
- Keep all motion non-blocking. No `Sleep()` or blocking loops in `DoLoader` paths.

## Rules

- Non-FSM only: keep `switch(Task)` + helper `Do*` style. No `FSMRunner`, `FSM_GOTO`, `*Step.h`, `*Table.cpp`, `*Exec.cpp`.
- Search HT160 first; read HT172 0420 only for behavior reference.
- Preserve BCB6 compatibility (no `auto`/`nullptr`/lambdas/range-for/`enum class`).
- Preserve Big5/CP950 encoding on `.cpp`/`.h`; never use VS Code UTF-8 edit tools on Big5 source — use Python binary mode or PowerShell CP950.

## Verify Contract

After any edit:

```powershell
cd D:\HT160S_BCB
powershell -ExecutionPolicy Bypass -File .\scripts\ops\check-ht160s-source-encoding.ps1
powershell -ExecutionPolicy Bypass -File .\scripts\ops\build-ht160s.ps1 -Clean
```

Delete the related `.obj` before single-file compile so stale objects cannot hide failures. When adding a new `.cpp` unit, update `.bpr`, `.mak`, and `ht160s.cpp` `USEUNIT()`.
