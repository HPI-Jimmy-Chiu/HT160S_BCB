---
name: ht160s-motion-view-tray-display
description: "Use when working on the HT160S_BCB Motion View (Monitor) moving-tray display: position carrier vs content grid, fHasTray-driven tray-grid visibility, Loader full-tray vs Auto empty-tray-then-fill semantics, per-frame visibility sync. Triggers: Motion View, Monitor tray display, pl*TrayWork, mt*TrayWork, UpdateTrayVisibleByHasTray, fHasTray, SetSimulateScreenStatus, BindMovingTrayPanel, TTrayMotor Refresh, SetHTrayPanel, VMotPtr, tray grid visible."
---

# HT160S Motion View Tray Display Skill

## Purpose

Maintain and extend the HT160S_BCB **Motion View** (Monitor) moving-tray display. Each station column shows one moving tray whose **position** comes from the physical Y motor and whose **content** (per-cell IC color) comes from a virtual tray motor. This is **display-only** VCL code — never let it drive machine motion, vacuum, or sensors. HT160S_BCB is **non-FSM**: keep procedural / VCL-event / `switch(Task)` style.

## Scope

- Target source:
  - `HT160S_Program_BCB_V1.0.0.0/MotorAndIO/MyMotor.cpp`, `MyMotor.h` (`TTrayMotor` rendering)
  - `HT160S_Program_BCB_V1.0.0.0/main.cpp`, `main.h` (binding + per-frame refresh)
  - `HT160S_Program_BCB_V1.0.0.0/main.dfm` (`pl*TrayWork` panels + `mt*TrayWork` grids)
- Data producers (do not edit for display work, read for truth):
  - `aLoader.cpp` (`DoFeedTray` / `DoDischargeTray`)
  - `aAuto1To6.cpp` (`DoFeedTray` / `DoDischargeTray`, `GetAutoVMotor`)
- Reference only: `D:\HT172\HT172_Program_V1.0.25.0_20260420` (read-only, do not edit)
- SPEC behavior anchor: repo memory `ht160s-spec-machine-fundamentals.md`; layout/porting history `ht160s-motion-view-monitor.md`.

## Two-Layer Model (the core distinction)

Each station column binds **one moving tray** built from two cooperating objects on the same VCL panel (see `BindMovingTrayPanel` in `main.cpp`):

| Layer | DFM name pattern | Object | Bound via | Driven by | Meaning |
|-------|------------------|--------|-----------|-----------|---------|
| **Position carrier** | `pl*TrayWork` (TPanel) | physical `HSys.Mot.M*Y` (`TTrayMotor`) | `SetSimulateCompoment(panel, akLeft, softN, softP, 73, 595)` | real Y motor position | Where the tray physically is (Rear top=73 .. Car top=595, Work≈243 / CCD≈432 interpolated). **Always visible** when motor exists. |
| **Content grid** | `mt*TrayWork` (TTMyTray) | virtual `HSys.VMot.MM*Y` (`TTrayMotor`) | `SetHTrayPanel(tray)` | `fHasTray` + `Tray.Data[x][y]` | The per-cell IC content. **Only meaningful when a tray is actually present.** |

Key rule: **position carrier `pl*` stays visible; content grid `mt*` is shown only when `fHasTray==true`.** The same panel can be both position-bound and content-bound because `TTrayMotor` extends `TMyMotor` (which owns `SetSimulateCompoment`).

## fHasTray-Driven Content Visibility

`mt*` (the `pHTray` / `pSubHTray` TTMyTray grid) visibility follows `TTrayMotor::fHasTray`:

- `fHasTray == false` → no tray on the carrier → **hide** the content grid (`pHTray->Visible=false`). The `pl*` carrier and its label stay visible.
- `fHasTray == true` → tray present → **show** the content grid, painted from `Tray.Data[x][y]` via the color map.

### Implementation (already in code)

`TTrayMotor::UpdateTrayVisibleByHasTray()` (in `MyMotor.cpp`):

```cpp
void TTrayMotor::UpdateTrayVisibleByHasTray()
{
    if(fHTary    && pHTray    !=NULL && pHTray->Visible   !=fHasTray) pHTray->Visible=fHasTray;
    if(fSubHTary && pSubHTray !=NULL && pSubHTray->Visible!=fHasTray) pSubHTray->Visible=fHasTray;
}
```

It toggles **only** `pHTray` / `pSubHTray` (the `mt*` grids). It must **never** touch `pPanel` / `pPalTrayID` or the `pl*` position carrier — those stay visible.

Three call sites:
1. **`SetHTrayPanel` / `SetSubHTrayPanel`** — call at bind time. Since a fresh `TTrayMotor` has `fHasTray=false`, the grid starts hidden (correct startup state: position carrier shown, content empty/hidden).
2. **`Refresh()`** — first line, so every data-change repaint also re-syncs visibility.
3. **Per-frame in Motion View** — `TfMain::SetSimulateScreenStatus()` loops all virtual motors while the page is active:
   ```cpp
   if(HSys.VMotPtr != NULL)
       for(int i = 0; i < HSys.iTotalVMotor; i++)
           if(HSys.VMotPtr[i] != NULL)
               HSys.VMotPtr[i]->UpdateTrayVisibleByHasTray();
   ```

### Why the per-frame sync is mandatory (the pitfall)

Some data paths set `fHasTray` **directly** without going through `ClearTray()` / `Refresh()`:

- `aAuto1To6.cpp` `DoDischargeTray` case 1000: `TrayMotor->InitNewTray(EMPTY_IC); TrayMotor->fHasTray=false;` — sets the flag false but does **not** call `ClearTray()`, so no `Refresh()` fires and the grid would stay visible.

Without the per-frame VMot loop, the content grid would linger after the tray leaves. The loop guarantees visibility converges to `fHasTray` every frame while Motion View is open. (Cost is gated: `SetSimulateScreenStatus` only runs the loop when `pgcMain->ActivePage==tsMonitorView && pgcMonitor->ActivePage==TabSheet10`.)

## Color Map (content cells)

`InitTrayColorMap` (file-scope static in `MyMotor.cpp`), index from `Tray.Data[x][y]`:

| Index | Constant | Color |
|-------|----------|-------|
| 0 | `EMPTY_IC` | `clWhite` (cell, no IC) |
| 1 | `UNCHECK_IC` | `clSkyBlue` (IC present, not yet tested) |
| 2 | `HAS_OK_IC` | `clLime` (good IC) |
| 3..15 | bin colors | per-bin distinct colors |

## Station Fill Semantics vs SPEC

This is the behavior the display must faithfully reflect (cross-ref `ht160s-spec-machine-fundamentals.md`):

| Station | On tray arrival | Initial content | Then |
|---------|-----------------|-----------------|------|
| **Loader L / R** | Operator feeds a **production** tray from the Front/stack side. `aLoader.cpp` `DoFeedTray` case 9000: `fHasTray=true; PrepareTrayMap(...)` → `InitNewTray(EMPTY_IC)` then cells set `UNCHECK_IC`. | **Full tray** — every cell has IC data (`UNCHECK_IC`, sky-blue). | SortArm picks ICs out; on discharge `DoDischargeTray` case 3000 calls `ClearTray()` (`fHasTray=false`). |
| **Auto1..6** | Receives an **empty** tray from the **Rear** (delivered by TrayArm). `aAuto1To6.cpp` `DoFeedTray` case 7000: `fHasTray=true; InitNewTray(EMPTY_IC)`. | **Empty tray** — all cells `EMPTY_IC` (white), no IC yet. | SortArm places ICs in one-by-one (cells become OK/bin colors) until the tray fills; discharge sets `fHasTray=false` directly (see pitfall above). |

SPEC anchors (do not contradict): TrayArm delivers empty trays `EmptyTray -> Bin/Auto`; SortArm places ICs; **Scheduler owns routing/classification**. The Monitor view is a passive mirror of `fHasTray` + `Tray.Data` — it must never become a control input.

## Ownership / Safety Rules

- **Display-only.** Never gate motion, vacuum, sensors, or interlocks on Motion View state.
- Toggle visibility through `UpdateTrayVisibleByHasTray()` only; do not hand-set `mt*->Visible` elsewhere.
- Never touch the `pl*` position carrier or `pPanel`/`pPalTrayID` visibility from the content-visibility path.
- All binding and per-frame access must be `NULL`-guarded (`VMotPtr`, each element, `pHTray`/`pSubHTray`).
- `fHasTray` is **station-owned** truth (Loader/Auto modules write it). The display reads it; it must not write it back.
- Keep edits ASCII-in-context to preserve Big5 (CP950) encoding of `.cpp`/`.h`/`.dfm`.

## Known Wiring Facts

- `BindMovingTrayPanel(PosMot, ContentMot, Panel, Tray)` in `main.cpp`: `PosMot->SetSimulateCompoment(Panel, akLeft, softN, softP, 73, 595)` + `ContentMot->SetHTrayPanel(Tray)`.
- Bound columns: `MAutoY_1..6`/`MMAutoY_1..6` → `plAuto1..6TrayWork`/`mtAuto1..6TrayWork`; `MEmptyY`/`MMEmptyY` → `plEmptyTrayWork`/`mtEmptyTrayWork`; `MLoaderY_1`/`MMLoaderY_1` → `plLoaderLTrayWork`/`mtLoaderLTrayWork`; `MLoaderY_2`/`MMLoaderY_2` → `plLoaderRTrayWork`/`mtLoaderRTrayWork`.
- `MMTrayArmX` has no dedicated tray-grid panel → not bound.
- Color column carrier `plColorTrayWork` IS position-bound to physical `MColorY` (front↔rear; same softN/softP −100..83800 as Empty) via `BindMovingTrayPanel(MColorY, NULL, plColorTrayWork, NULL)`. It has **no virtual content motor** (`MMColorY` does not exist) because Color presents identity/dummy trays, not IC content — so `mtColorTrayWork` is hidden once in code at bind time and the column shows the gray carrier only, mirroring Empty (whose `aEmpty` likewise never fills `MMEmptyY` content). `plColorTrayWork` DFM `Visible` was flipped False→True to expose it. (Earlier note "Color has no Y motor → static" is obsolete: `MColorY` now exists in `Mot_Table` M19.)
- Tray grid cell counts come from the bound panel's `pHTray->XItem/YItem`, clamped to `MAX_TRAY_X/Y` (50/20).
```
