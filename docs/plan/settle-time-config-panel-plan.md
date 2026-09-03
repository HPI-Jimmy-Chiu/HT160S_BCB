# Plan — Operator-editable mechanism settle-time panel (tsLoaderUnloader)

Status: **IMPLEMENTED & COMMITTED (commit 4c56123, feat/iosetview-172-refactor) — 2026-07-15 verified: GeneralSetting members `iEmptyDestackSettleMs` / `iColorDestackSettleMs` / `iLoaderDestackSettleMs` / `iAutoPushConfirmSettleMs` / `iTrayArmClampSettleMs` etc. wired at ~230 call sites; `pnlSettleDelay` + `edSettle0..N` panel present in maintenance.h/.dfm/.cpp. On-machine verify pending. (This header previously read "PLAN ONLY — No code changed", which was stale.)**
Date: 2026-06-28
Origin: follow-up to the actuator-timer pause-freeze work. User wants the hard-coded
"expires-without-alarm" settle/step delays exposed for editing, defaults = current values,
on a new panel in the `tsLoaderUnloader` tab.

---

## 1. Goal & scope

Make the hard-coded settle/step dwell literals (the `Set(N)` calls that just advance a Task on
expiry, **never** raise an alarm) editable at runtime, persisted in machine config, defaults
pre-filled with today's values. New panel lives on `tsLoaderUnloader`.

In scope: pure settle dwells using a fixed `Set(N)` literal.
Out of scope (already config/recipe-driven, confirmed): SortArm `PnpSettle`, `BlowDwell`
(recipe `[PnP]`). Also excluded: Color `ScanDelay` 3000ms (a CCD-read **timeout that alarms**,
not a no-alarm settle).

Note on tab choice: `tsLoaderUnloader` is on the **Maintenance** form, not main. The params
span Empty/Color/Loader/Auto/TrayArm, so the panel title should read machine-wide, e.g.
"Mechanism Settle Times / 機構沉降時間", not "Loader only".

---

## 2. Parameters (consolidated; defaults = current literals)

`Set(N)` = N×100 ms. Multiple call sites with the same literal & mechanism collapse to ONE param.

| # | Param (GeneralSetting member) | Default | Sites | Module / role |
|---|---|---|---|---|
| 1 | `iEmptyDestackSettleMs` | **500** | aEmpty GoDown ×4 (505/515/529/543), GoUp ×2 (605/623), Test ×5 (741/751/766/810/828) | Empty front-destacker rise/separate settle |
| 2 | `iColorDestackSettleMs` | **500** | aColor GoDown ×4 (423/433/447/461), GoUp ×2 (538/556), Test ×5 (1142/1152/1167/1209/1227) | Color front-destacker settle (Empty mirror) |
| 3 | `iLoaderDestackSettleMs` | **1000** | aLoader Test ×2 (1743/1761) | Loader Teach destacker settle (distinct default) |
| 4 | `iAutoPushConfirmSettleMs` | **500** | aAuto1To6:541 | Auto feed post-push, before push-sensor confirm |
| 5 | `iAutoDischargePostYSettleMs` | **500** | aAuto1To6:684 | Auto discharge post-Y settle |
| 6 | `iAutoFrontRiseDwellMs` | **500** | aAuto1To6:1374 (shared by discharge + Teach GoUp-once) | Auto front-rise On→dwell→Off |
| 7 | `iAutoCleanOutRiseDwellMs` | **500** | aAuto1To6:807 | Auto clean-out front-rise dwell |
| 8 | `iTrayArmClampSettleMs` | **300** | aTrayArm:221 | TrayArm grab/release clamp settle (distinct default) |

Special cases (DoClampTray `SettleTicks` arg — NOT plain `Set(N)`; recommend handle separately):

| # | Param | Default | Site | Caveat |
|---|---|---|---|---|
| 9 | `iEmptyFeedClampSettleMs` | 500 | aEmpty:358 `DoClampTray(...,5)` | Helper does `Delay.Set(SettleTicks)` = ticks×100ms; **must stay ≥100ms** or the Push.OnSensor confirm is skipped. Would need `/100` at the call site (tick granularity). |
| 10 | `iColorFeedClampSettleMs` | 0 | aColor:733 `DoClampTray(...,0)` | `0` deliberately **disables** the inline confirm (Color uses rear sensor + MES1421 instead). Editing >0 changes Color's confirm design. **Recommend: leave hard-coded 0, exclude from panel.** |

→ Core editable set = **#1–#8** (8 params). #9 optional (tick-granularity nuance). #10 recommend exclude.

---

## 3. Storage design — GeneralSetting `[SettleDelay]` section

Mirror the existing `iLoaderYSafeDistance` idiom in `THT160GeneralSetting`:
- **Declare** 8–10 `int` members in `GeneralSetting.h` (next to `iLoaderYSafeDistance` ~line 76).
- **SetDefault** (`GeneralSetting.cpp` ~40/90): seed each to its current value (500/1000/300…).
- **Load**: `ReadInteger("SettleDelay","EmptyDestackSettleMs", default)` per param.
- **Save** (`GeneralSetting.cpp` ~139): `WriteInteger("SettleDelay", …)` per param.
- New INI section `[SettleDelay]` in the same machine-config .ini that holds `[Safety]`.

Tier = **machine config** (one set per machine), not per-Lot recipe — these are fixed mechanical
timings, parallel to `iLoaderYSafeDistance`/`iAmrFeedWaitSec`. (PnP delays stay in recipe; they
genuinely vary per product.)

---

## 4. Apply mechanism — inline `SetMS(GeneralSetting.iXxx)` (no new plumbing)

Replace each `XxxDelay.Set(5)` with `XxxDelay.SetMS(GeneralSetting.iXxx)`. This is the **existing
codebase pattern** — the AMR timers already arm via `AmrFeedWaitTimer.SetMS(GeneralSetting.iAmrFeedWaitSec*1000)`.
Modules already reference the `GeneralSetting` global, so:
- No setter/cache plumbing, no startup push needed.
- Value is read **live at each arm** → editing in the panel + `Save()` takes effect on the next
  cycle, no app restart.
- `Set(N)` (×100) becomes `SetMS(ms)` (raw ms) → finer granularity than the old 100ms step.

Per module: confirm/add `#include "GeneralSetting.h"` (aColor/aEmpty/aLoader/aAuto already use it;
**verify aTrayArm** — add include if missing). For #9, the DoClampTray call passes
`GeneralSetting.iEmptyFeedClampSettleMs/100` (ticks), clamped so result ≥1 tick.

---

## 5. UI design — new TPanel on tsLoaderUnloader

- Add a 6th `Align=alTop` `TPanel` below `pnlMachineIdentity` (existing panels stop at Top=370;
  ~506px free). Title label "Mechanism Settle Times (ms) / 機構沉降時間".
- 8 rows (2 columns × 4) — each row: `TLabel` (name) + ReadOnly `TEdit` (ms value).
- Edit pattern = **mirror `edLoaderSafeDistance`**: ReadOnly TEdit, `OnClick` →
  `fQwertyKey->ShowQwertyKey(edit, N_INTEGER, 0, true, MIN, MAX, title)` keypad → YES/NO confirm →
  write clamped value to `GeneralSetting.iXxx` → `GeneralSetting.Save()` → re-format text →
  `RefreshHardwareSettingsStatus()`.
- **Load**: populate the 8 edits inside `LoadHardwareSettings()` within the
  `bLoadingHardwareSettings=true/false` window (so the programmatic `->Text=` doesn't re-fire).
- **Guard**: every new `...Click` handler starts `if(bLoadingHardwareSettings) return;`.
- Proposed clamps: settle dwells **MIN 50 / MAX 5000 ms** (TrayArm/Empty-clamp MIN 100 to keep a
  real dwell/confirm); Loader Teach MIN 100 / MAX 5000. (Final clamps to confirm with you.)
- Bilingual labels via the existing LangT() pattern (project is bilingual).

---

## 6. Edit inventory (files touched)

- `GeneralSetting.h` / `GeneralSetting.cpp` — 8 new int members + SetDefault + Load + Save.
- `maintenance.dfm` — new TPanel + 8 TLabel + 8 TEdit objects inside tsLoaderUnloader (before its
  closing `end` at dfm:1308). **Hand-edit byte-safe, NOT the visual designer.**
- `maintenance.h` — 8 TEdit + labels/panel as `__published` **FIELDS** (before the first handler at
  ~h:299), and 8 new `...Click` handler decls **AFTER** the last handler (after edLoaderSafeDistanceClick
  h:331). No `//` comments inside the class body.
- `maintenance.cpp` — 8 `...Click` handlers (mirror edLoaderSafeDistanceClick) + 8 populate lines
  in `LoadHardwareSettings()`.
- `aEmpty.cpp` / `aColor.cpp` / `aLoader.cpp` / `aAuto1To6.cpp` / `aTrayArm.cpp` — replace the
  `Set(N)` literals with `SetMS(GeneralSetting.iXxx)` (and aTrayArm include if needed).

---

## 7. Phasing

- **P1 — storage**: add the 8 GeneralSetting members + default/load/save. Build -Full. (No behavior change yet.)
- **P2 — apply**: swap the `Set(N)` → `SetMS(GeneralSetting.iXxx)` at all sites. Build -Full + verify defaults equal old values (500/1000/300). Behavior identical at defaults.
- **P3 — UI**: add the panel + edits + handlers + load wiring. Build -Full.
- **P4 — verify**: sim build + real-machine (SOFT_SIMULATE off) build, encoding gate; confirm
  edit→Save→reflected on next cycle.
- (optional P5) #9 Empty clamp-settle if you want it; #10 left as-is.

---

## 8. Build / encoding / designer hazards (must observe)

- Struct change to `THT160GeneralSetting` → **-Full** build; shared-core → also verify
  SOFT_SIMULATE-off build, then restore.
- Byte-safe edits only (Big5 source; `scripts/ops/bcb6-bytesafe-edit.ps1` or Python/Latin1 splice).
  ASCII-only new comments. Run `check-ht160s-source-encoding.ps1`.
- maintenance form: **never open in the BCB visual designer** (strips components/decls);
  fields-before-handlers; no comments in class body; DFM event handlers must be `__published`;
  don't touch any `TALed` `Visible`.

---

## 9. Risks & safety

- **Too-low values**: operator could set a dwell to near-zero. Clamp MIN guards this; settle being
  short only risks a mechanical not-settled condition, but downstream confirms (cylinder/sensor)
  still catch real misses. Recommend conservative MIN (50–100ms).
- **No hang-up risk added**: these timers are NOT in any Pause accessor; making them configurable
  doesn't change that. (Same analysis as before.)
- **#10 Color clamp=0**: do NOT expose as a normal edit; changing it alters the confirm design.
- **Teach vs production collapse (#1/#2)**: editing the Empty/Color destacker settle also changes
  the Teach-test dwell (they share the param). Intended (Teach should mirror production) — confirm.

---

## 10. Decisions — RESOLVED (user, 2026-06-28)

1. **Scope**: **ALL 10 params (#1–#10), including #9 Empty clamp AND #10 Color clamp.**
2. **Collapse Teach into production** for Empty/Color (#1/#2): **YES** (one param each covers
   production GoDown/GoUp + Teach Test).
3. **Clamp ranges** (going with): settle dwells MIN 50 / MAX 5000 ms; TrayArm(#8) & Empty
   clamp(#9) & Loader(#3) MIN 100; **Color clamp (#10) MIN 0** (0 must remain selectable to keep
   today's no-inline-confirm behavior) / MAX 5000.
4. **Auto granularity**: keep #4/#5/#6/#7 as 4 separate params.
5. **Panel title**: "Mechanism Settle Times / 機構沉降時間".

### #10 Color clamp — special handling (because it is now in scope)
Default stays **0** = current behavior (no inline Push confirm; Color confirms via rear sensor +
MES1421). The call becomes `DoClampTray(..., GeneralSetting.iColorFeedClampSettleMs/100)`:
- `0` → SettleTicks 0 → identical to today.
- `>0` (operator-set) → ENABLES the inline Push.OnSensor confirm + Pop-on-miss that Color does
  not use today. The edit's on-screen hint MUST state: "0 = use rear sensor only (default);
  >0 = also do inline clamp confirm". This is an intentional operator capability, not a default change.

### Final editable set = 10 params
#1 iEmptyDestackSettleMs 500 · #2 iColorDestackSettleMs 500 · #3 iLoaderDestackSettleMs 1000 ·
#4 iAutoPushConfirmSettleMs 500 · #5 iAutoDischargePostYSettleMs 500 · #6 iAutoFrontRiseDwellMs 500 ·
#7 iAutoCleanOutRiseDwellMs 500 · #8 iTrayArmClampSettleMs 300 · #9 iEmptyFeedClampSettleMs 500 (÷100 ticks, ≥1) ·
#10 iColorFeedClampSettleMs 0 (÷100 ticks; 0 allowed).
UI = 10 rows (2 cols × 5). Status: **plan finalized — ready to implement P1→P4 on user go-ahead.**
