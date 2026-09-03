# Clean Out Cascade Refactor Plan

Date: 2026-07-01
Owner: HT160S-Maintainer
Status: IMPLEMENTED + build-clean (sim -Full AND real-machine -Full both EXIT 0,
2026-07-01). On-machine verification by user pending.

## Implementation record (2026-07-01)

- Slice A Loader (aLoader.cpp, database.cpp): A1 removed the CleanOut feed block; A2 made
  case 9000 source-dry CleanOut-aware (no MES0920/MES0921 alarm); A3 phase-aware per-side
  finish guard (car dry + carriage empty + Y none); A4 IsAllCleanOutFinish adds front/rear/
  supply-car sensor gate + rear latch; A5 front untracked-residual operator alarm MES0922
  (registered in database.cpp CreateSystemAlarmCode SeedCode[15]).
- Slice B TrayArm (aTrayArm.cpp): IsCleanOutFinish is now a live predicate (Loader done &&
  Auto done && arm empty && Z-up), was always-true.
- Slice C Empty/Color (aEmpty.cpp/.h, aColor.cpp/.h): both now participate. Drain phase =
  RunMode==CleanOut && TrayArm->IsCleanOutFinish(); Empty reuses bLotFinish->GoUp (trigger
  widened to drain a rear tray too, MES1022 suppressed in CleanOut); Color gets a case-100
  drain branch reusing the case-1700 GoUp ladder. New IsCleanOutFinish() on each = TrayArm
  done + flow clear (no front/rear tray) + rise cylinders home (IsReadyForAmrHandoff);
  Color also gated on IsInstalled(). Edited byte-safe (Big5-preserving Python latin-1 splice).
- Slice D csystem (csystem.cpp): CheckCleanOutFinish ANDs in TrayArm/Empty/Color (+aTrayArm.h).

Sim testing note: IsContinuousFeed()==chkLoadTray->Checked, so to test CleanOut completion
in SOFT_SIMULATE the operator must UNCHECK "Load New Tray" (simulate the supply car empty),
matching the drain-the-car semantic.

---

(original design below)


## Motivation

On-machine test (2026-07-01) found the **Loader left an empty tray** after Clean
Out completed. This is not allowed: after Clean Out, the Loader **front feed
position, rear output position, and the carriage (fHasTray) must all be empty**
of trays AND IC.

Root cause: the per-side Clean-Out finish guard
(`aLoader.cpp` DoLoader top, ~line 934) declares a side finished when only the
**software carriage flag** `TrayMotor->fHasTray==false` (+ Y not owned). It never
checks the physical **rear** sensor (`SnLoader_OutputBottomHasTray` /
`bRearHasTray`) nor the **front** sensor (`SnLoader_InputHasTray`). A tray parked
at the rear waiting for TrayArm pickup => Loader wrongly reports finished => Clean
Out completes and the machine stops with a tray still in the pipeline.

Beyond that, the user asked to correct the whole cascade (per-part finish
semantics) so Clean Out truly empties the machine.

## User-confirmed scope decisions

1. **Loader supply car**: Clean Out must ALSO empty the input supply car
   (`SnLoader_Inputend`). During Clean Out the Loader keeps feeding + sorting the
   remaining input trays until the supply car is dry, THEN drains the pipeline.
   (i.e. "run everything already loaded to completion, then empty out and stop".)
2. **Untracked residual tray** (front sensor lit but both carriages report
   `fHasTray==false`, i.e. software cannot auto-discharge it): raise a Note alarm
   for the operator to remove it; do not finish until the sensor clears. A **rear**
   residual tray is still auto-recovered by TrayArm (no alarm).

## Corrected finish cascade (topological, no cycle)

```
Loader  ->  SortArm  ->  Auto1~6  ->  TrayArm  ->  Empty / Color
```

Overall end = Loader && SortArm && Auto && TrayArm && Empty && Color all
clean-out-finished AND no IC under the machine.

Dependency safety: Loader finish is driven by TrayArm's **action** (it keeps
picking the Loader rear and recycling), NOT by TrayArm's finish **flag**. The
finish-FLAG dependency graph is a strict order Loader < SortArm < Auto < TrayArm <
Empty/Color, so there is no circular wait.

Two phases inside Clean Out:
- **Produce phase** (until Loader.IsAllCleanOutFinish()): the whole machine runs
  normally - Loader feeds/sorts the remaining supply car, SortArm sorts into Auto,
  Auto discharges, TrayArm supplies empty trays from Empty/Color and recovers
  Loader-rear trays. Empty/Color KEEP SUPPLYING here (must not starve).
- **Drain phase** (after TrayArm.IsCleanOutFinish()): Empty/Color stop supplying
  and GoUp all trays back to their car; Auto has already run its physical
  clean-out discharge.

## Per-module changes

### A. Loader (`aLoader.cpp`, `aLoader.h`)

A1. Remove the blanket feed block so the supply car drains during Clean Out:
- `DoFeedTray` ~line 1072 `if(RunMode==Run_CleanOut) return false;` -> DELETE
  (feeding must continue until the car is dry).

A2. Source-dry during Clean Out must NOT alarm; it means "car drained, stop
feeding this side":
- case 3500 / case 9000 source-dry branches (MES0920 / MES0921): when
  `RunMode==Run_CleanOut` and source dry, skip the alarm and route the side to a
  benign idle so the finish guard can retire it. (Guard the ShowMyError calls with
  a `RunMode!=Run_CleanOut` condition, else set a "car drained" terminal.)

A3. Per-side finish guard (DoLoader top, ~line 934) becomes phase-aware:
- In Clean Out, a side is only "finished feeding" when the source is DRY
  (`SnLoader_Inputend` OFF in REALLY / `!IsContinuousFeed()` in sim) AND
  `fHasTray==false` AND Y not owned. If the source still has stock, DO NOT finish;
  fall through to the normal feed flow (re-feed).

A4. `IsAllCleanOutFinish()` (~line 657) gains the physical residual + car checks
(REALLY mode only; sim/DUMMY trusts the latches, mirroring RefreshStateFromSensors
early-out):
- both `Side[0/1].bCleanOutFinish`, AND
- front clear: `SnLoader_InputHasTray` OFF (Enable-gated), AND
- rear clear: `bRearHasTray==false` (and `SnLoader_OutputBottomHasTray` OFF), AND
- car dry: `SnLoader_Inputend` OFF (Enable-gated).

A5. Untracked front residual (front sensor lit, both carriages `fHasTray==false`,
no active feed): raise a Note (e.g. reuse ShowMyError with `SnLoader_InputHasTray`)
asking the operator to remove it; re-check on retry. Rear residual is NOT alarmed
(TrayArm recovers it).

### B. SortArm (`aSortArm.cpp`) - NO CHANGE

Existing finish (idle + no held IC + `Loader.IsAllCleanOutFinish()`) is correct;
it now inherits the stronger Loader gate automatically.

### C. Auto1~6 (`aAuto1To6.cpp`) - NO CHANGE

Existing physical discharge (`DoAllAutoCleanOut`) + `IsAllCleanOutFinish()` are
correct; gated on `SortArm.IsCleanOutFinish()`.

### D. TrayArm (`aTrayArm.cpp`, `aTrayArm.h`)

Replace the always-true `IsCleanOutFinish()` (line 79-85) with a computed flag.
In `DoTrayArm` idle (case 100, arm empty, `Job==TAJOB_NONE`):
```
if(RunMode==Run_CleanOut &&
   LoaderModule->IsAllCleanOutFinish() &&
   AutoModule->IsAllCleanOutFinish() &&
   HasTray()==false &&
   IsZUpAtPosition())
    bCleanOutFinish=true;
```
Reset `bCleanOutFinish=false` in `InitialFlag` (currently forced true). Mirrors the
SortArm pattern (aSortArm.cpp ~1651).

### E. Empty (`aEmpty.cpp`, `aEmpty.h`) - NEW participation

E1. Drain phase gate: at DoEmpty top, drain phase =
`RunMode==Run_CleanOut && TrayArmModule && TrayArmModule->IsCleanOutFinish()`.
While draining, set the existing `bLotFinish` path active (it already drives the
GoUp-to-car branch) so the module stops feeding/destacking and GoUp-drains.

E2. GoUp trigger must also drain a REAR tray, not only a front one:
- case 100 `if(bLotFinish && bFrontHasTray)` -> `if(bLotFinish && (bFrontHasTray || bRearHasTray))`.
  (DoGoUpTray already handles front->car then rear->front internally.)

E3. Suppress the source-dry MES1022 alarm during Clean Out.

E4. `IsCleanOutFinish()` (new, add to `aEmpty.h` public):
```
if(TrayArmModule && TrayArmModule->IsCleanOutFinish()==false) return false;
RefreshStateFromSensors();
if(bFrontHasTray || bRearHasTray) return false;
if(C_Empty_FrontRiseTray_1 on || C_Empty_FrontRiseTray_2 on) return false;
return true;
```

### F. Color (`aColor.cpp`, `aColor.h`) - NEW participation

F1. Not installed => `IsCleanOutFinish()` returns true (trivially done).
F2. Drain phase (same gate as Empty). Add a DoColor case-100 branch:
`if(drainPhase && (bFrontHasTray || bRearHasTray)) { DoGoUpTray(0); Task=1700; }`
placed after the bReturnTray branch, before the supply branches.
F3. `IsCleanOutFinish()` (new): installed gate + TrayArm done + `!bFrontHasTray &&
!bRearHasTray` + `C_Color_FrontRiseTray_1/_2` (+ FrontSeparate) off.

### G. csystem (`csystem.cpp`)

Extend `CheckCleanOutFinish()` (~line 1431) to AND in the new participants:
```
if(TrayArmModule && TrayArmModule->IsCleanOutFinish()==false) return false;
if(EmptyModule && EmptyModule->IsCleanOutFinish()==false) return false;
if(ColorModule && ColorModule->IsCleanOutFinish()==false) return false;
```
(Loader / SortArm / Auto / HasICUnderMachine checks stay.)

## Reset / entry correctness

`InitialAllTask` -> each module `InitialFlag()` resets `bCleanOutFinish`. It is
called at prior-Clean-Out completion (csystem ~1370) then ChangeRunMode(Run_Normal),
so a fresh Clean Out always starts with finish flags = false. TrayArm/Empty/Color
new flags must be reset to false in their `InitialFlag`.

## Implementation slices (build after each)

- Slice A: Loader (A1-A5) + build (`scripts/ops/build-ht160s.ps1 -Clean`, delete
  changed .obj first).
- Slice B: TrayArm (D) + build.
- Slice C: Empty + Color (E, F) + build.
- Slice D: csystem (G) + build. Then real-machine build gate: comment
  `#define SOFT_SIMULATE` in `MachineType.h`, `-Full`, expect exit 0, RESTORE
  define, rebuild. Encoding check.

Byte-safe editing: all edited regions are ASCII C++/English comments; still verify
no Big5 on edited lines (or use `scripts/ops/bcb6-bytesafe-edit.ps1`).

## On-machine verification (user)

1. Normal Clean Out from running: machine drains supply car, sorts remainder,
   empties Loader front/rear/carriage, Auto discharges, Empty/Color GoUp to car,
   ends back to Normal + stop. Loader has NO residual tray.
2. Rear-residual: a tray at Loader rear is recovered by TrayArm before finish.
3. Front untracked residual: Note alarm fires; clears after operator removes tray.
4. Color uninstalled: Clean Out still completes (Color trivially finished).
```

## Adversarial review (2026-07-01, 9-lens red-team + per-finding verify)

FIXED (commit after the checkpoint):
- CRITICAL: DoLoader finish guard preempted an in-flight rear discharge -
  `TrayMotor->fHasTray` is cleared at DoDischargeTray case 3000 but
  `bRearDischargeInProgress` only clears at case 4000. Retiring the side in that
  window stranded the flag true -> `IsRearReadyForPick()` never true -> TrayArm
  never recovers the rear tray -> Clean Out hangs with a tray at the Loader rear.
  Fix: finish guard also requires `bRearDischargeInProgress==false`. (Near-certain
  end-game trigger, so this was the most important fix.)
- HIGH: TrayArm could deliver an empty tray onto an Auto that already latched
  `bCleanOutFinish` (produce->drain boundary) -> tray stranded on the Auto rear
  and `Auto::IsAllCleanOutFinish()` (checks only `bCleanOutFinish`) still true, so
  Clean Out completes with a tray left. Fix: `GetTrayRequest` returns none once
  `SortArm.IsCleanOutFinish()` (drain phase) so no Auto requests a tray then.
- HIGH: MES0922 front-residual alarm was gated only by compile-time
  `#ifndef SOFT_SIMULATE`; a REAL build in DUMMY read the phantom InType=0 sensor
  and false-alarmed. Fix: also gate on runtime `IsSoftSimulate()==false`.
- MEDIUM: `Color::IsCleanOutFinish()` could hang in SortBin mode (drain lives in
  DoColor case 100, unreachable in SortBin). Fix: SortBin Color trivially finishes.

DEFERRED / accepted (follow-ups, not blocking):
- MEDIUM: a Color/Empty REAR tray that GoUp cannot physically clear (stuck-ON
  sensor, cylinders cycling fine) loops the drain forever with no timeout alarm.
  Add a drain-attempt watchdog -> operator alarm. (A stuck cylinder already
  self-alarms via Push/Pop timeout; only a stuck sensor loops silently.)
- ~~MEDIUM: recoverable full-machine HOME mid-Clean-Out reverted to Normal while
  `bCleanOut` stayed latched -> resumed NORMAL production.~~ FIXED: the post-home
  run-mode pick in ProcessMotion now resumes `Run_CleanOut` when `bCleanOut` is
  latched (bHomeByStart still wins for a Start-triggered home), mirroring the
  OneCycle-during-CleanOut resume. InitialAllTask(true) reset the finish flags so
  the cascade re-evaluates and finishes fast if already empty.
- ~~LOW: BUG-2 residual - a tray TrayArm already committed to an Auto during the
  produce phase can still be delivered in the drain window.~~ FIXED (two layers):
  (1) DoPlace case 1/10 in-flight divert - re-checks the drain boundary
  (SortArm.IsCleanOutFinish, same signal as the GetTrayRequest gate) on every
  tick while the tray is still in hand, and reroutes to the recycle destination
  (identity->Color, else Empty) via the DecidePlaceDestAfterPick contract; no
  Auto-side cleanup needed (rear flags are only written at case 4000).
  (2) DoAllAutoCleanOut case-7000 backstop - if any station still shows
  bRearHasTray/bRearDeliveredPending at the wipe (a deposit that was already
  mid-ladder at the boundary, or a delivered-not-yet-pulled rear tray), raise
  MES0923 (registered) so the operator removes the physical tray before the
  flags are wiped. Real machine only (runtime IsSoftSimulate gate).
- LOW: `SnLoader_Inputend` disabled -> guard treats car as dry (finish) so
  Clean Out drains the pipeline only, not the (unsensable) supply car. And
  Enabled-but-card-unbound (Input==NULL) makes IsOff()/IsOn() both false ->
  never dry -> hang. Both are misconfigurations; add an explicit handling if seen.
- LOW: Color drain reuses case-1700 so `iSimInfeedCount++/iReturnedCount++` fire
  per drained tray - arguably correct (trays return to the car) but inflates the
  returned-count. Cosmetic.
- LOW: Empty/Color `IsCleanOutFinish` do not explicitly check `bTrayReady` (a tray
  presented at the Color output); `bRearHasTray` should cover it - confirm on-machine.

Build after fixes: sim -Clean + real -Full both EXIT 0. Commits: 8548420
(cascade checkpoint) + 321dae9 (review-edge fixes).
```
