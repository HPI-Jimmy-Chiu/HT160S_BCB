---
name: ht160s-sortarm-flow
description: "Use when working on the HT160S_BCB SortArm module (TSortArmModule in aSortArm.cpp/.h): 4-slot pick from Loader, place to Auto1..6, sucker grouping, Z-safe motion, pitch matching, Bin-to-Auto routing. Triggers: TSortArmModule, DoSortArm, SortArm pick place, 4 sucker, SortArmSuck, MMAutoY, MMLoaderY, GetMappedAutoIndex, BinAreaMap, HasHoldingIC."
---

# HT160S SortArm Flow Skill

## Purpose

Maintain and extend the HT160S_BCB SortArm module. SortArm picks ICs from a Loader sorting tray (4 grouped suckers) and places them into Auto area trays (Auto1..6) by Bin-to-Auto mapping. This is a **non-FSM** procedural `switch(Task)` module — do not convert it to FSMRunner.

## Scope

- Target source: `HT160S_Program_BCB_V1.0.0.0/aSortArm.cpp`, `aSortArm.h`
- Runtime wiring: `ht160s.cpp` (`InitializeSortArmModule()` / `ShutdownSortArmModule()`); `database.cpp` includes `aSortArm.h`, resets `SortArmModule` in `InitialAllTask()`, `actSortArmExecute` -> `DoSortArm(P->Tag)`
- Reference only: `D:\HT172\HT172_Program_V1.0.25.0_20260420` (read-only, do not edit)

## Architecture

- Global singleton `TSortArmModule *SortArmModule`.
- Public entry: `void DoSortArm(int &Task)` — non-blocking, returns by reference Task.
- 4 sucker slots: `TSortArmSlotState Slot[4]` (bCanPick / bHasIC / bPlaceSelected / pick XY / place XY / TrayData).
- Two private sub-flows, each `switch(Task)` returning `true` when done:
  - `DoPickFromLoader(Flag)` — select pickable cells, Z-down, suck, transfer tray data.
  - `DoPlaceToAuto(Flag)` — select Auto area, find place cells, Z-down, destroy vacuum, transfer data.
- `HasHoldingIC()` — public guard reporting any slot currently holds an IC (used for OneCycle/stop safety).

## Motion / Hardware

- Motors (via `TTrayMotor`): `GetLoaderMotor` / `GetAutoMotor` (Y), plus virtual motors, `GetSuckZMotor(SlotIndex)` per slot Z.
- Suckers: `GetSucker(SlotIndex)` -> grouped `HSys.Suck.SortArmSuck.Suck[0][0..3]`.
- Motion helpers: `MoveSortArmX`, `MoveLoaderY`, `MoveAutoY`, `MovePitchToTrayPitch`, `SortArmZToSafePos`, `MoveToLoaderPick`, `MoveToAutoPlace`, `MovePickZDown`, `MovePlaceZDown`.
- **Z-safe-before-XY**: always `SortArmZToSafePos()` before any X/Y move. This rule is mandatory.
- Pitch matching: `CalculatePitchPosition` / `MovePitchToTrayPitch` align the 4 suckers to tray pitch before pick/place.

## Bin-to-Auto Routing

- `GetMappedAutoIndex(BinData, bool &bFixedArea)` — resolve which Auto area a Bin maps to; supports BinAreaMap with direct-Auto fallback.
- `SelectPlaceAuto` / `FindPlaceCells(AutoIndex)` / `CanPlaceSlotToAuto(SlotIndex, AutoIndex)` — place planning.
- `FindPickCells(LoaderNo)` / `IsPickableData(Data)` — pick planning.

## Tray Data

- Access tray cells as `Tray.Data[y][x]`; write via `SetTraySingleData(x, y, data)`.
- `TransferPickDataFromLoader` / `TransferPlaceDataToAuto` move tray-cell ownership during pick/place.
- Phase A scope: CCD result comes from Loader Top CCD tray data; Bottom CCD / TestIF / Offset not ported.

## Run-Mode / Simulate

- `IsSoftSimulate()` gates motor/sucker actuation. `UpdateKitSuckState()` keeps grouped sucker state consistent.
- Suck/Destroy must respect runtime dummy semantics — do not build/break vacuum when there is no IC. Do not use `bSoft_Simulate` as a substitute for runtime dummy mode.
- Non-blocking only: no `Sleep()` or blocking loops in `DoSortArm` paths.

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
