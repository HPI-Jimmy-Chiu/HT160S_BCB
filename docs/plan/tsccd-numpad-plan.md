# tsCCD move-to-cell + numpad row/col + foolproof, and SortArm retrofit — IMPLEMENTED

Status: implemented 2026-06-28 (build pending green). On-machine verification pending.
Related: `tsadvanced-teach-test-architecture` memory; tsTrayArm precedent (commit d95802d).

## Locked decisions (user, 2026-06-28)
1. **Drop Color** from the cell test. Color carries identity trays (no IC cell grid) and its CCD
   reads ONE 2D barcode at a fixed pose (aColor.cpp:756,880) — per-cell move is infeasible.
   tsCCD channel combo = **LoaderR, LoaderL only**. (A separate single-pose Color 2D read test
   could be added later if wanted.)
2. **Foolproof violation behavior**: when Col/Row is out of the tray-form range, **stop ALL Advanced
   tests, do NOT execute, then prompt with `ShowMyMessage`**. Applied to BOTH the CCD GO and the
   SortArm GO. (The numpad already clamps to [1,max]; the GO foolproof is the backstop for
   non-numpad entry / no-recipe edge cases.)
3. **Numpad upper limit = tray-form data** (`GetTrayXCount()` columns / `GetTrayYCount()` rows).

## Channel model (grounded)
- LoaderL → LoaderNo=1 (MLoaderY_1, left); LoaderR → LoaderNo=2 (MLoaderY_2, right) (main.cpp:1044).
- Top CCD X = shared `HSys.Mot.MTopCCDX` (Teach.LoaderCarFirstCCDXPosition); only the Loader Y differs per side.

## What was implemented
**aLoader.h/.cpp** (Phase 1):
- New public `CanMoveCcdToCell(LoaderNo,CellX,CellY,Err)` (mirror SortArm CanMoveSuckerToCell:
  range-check 0-based CellX∈[0,GetTrayXCount()), CellY∈[0,GetTrayYCount()), + soft-limit on
  MTopCCDX and the chosen MLoaderY).
- New public `MoveCcdToCell(LoaderNo,CellX,CellY,&Task)` — thin task-stepped wrapper over the
  existing private `MoveToCcdCell` (which commands+polls both axes and re-checks the shared-rail
  interlock `IsLoaderYMoveSafe` each tick; bad args → Task=900).
- Moved `GetTrayXCount()/GetTrayYCount()` from private → public (uteach needs them for the numpad limit).

**aSortArm.h**: moved `GetTrayXCount()/GetTrayYCount()` private → public (same reason). aSortArm.cpp
unchanged (CanMoveSuckerToCell already range-checks Col/Row at :1714-1723).

**uteach.h/.dfm/.cpp** (Phase 2-5):
- DFM: filled the user-added empty `tsCCD` tab with `gbCcd` (cbCcdChannel LoaderR/LoaderL,
  edCcdCol/edCcdRow, btnCcdGo) + lblCcdStatus. Wired `OnClick=edCcdColClick/edCcdRowClick`.
  Wired `OnClick=edSaColClick/edSaRowClick` onto the existing SortArm edSaCol/edSaRow.
- `EditCellWithNumpad(TEdit*,MaxValue,Caption)` — shared helper: `ShowQwertyKey(N_INTEGER, 0,
  CheckRange=true, 1, MaxValue, Caption)`. Reused by CCD + SortArm Col/Row edits.
- `StopAllAdvancedTests()` — StopSortArmTest+StopCarTest+StopTrayArmTest+StopCcdTest (foolproof path).
- CCD cluster: PopulateCcdCombos, ComboIndexToLoaderNo (0=LoaderR→2, 1=LoaderL→1), CheckCcdTestReady
  (MTopCCDX + chosen MLoaderY: SystemStart/EMG/enable/alarm/home), RunCcdTest (pump),
  StopCcdTest (stop MTopCCDX + both MLoaderY), SetCcdStatus, btnCcdGoClick.
- SortArm GO retrofit: btnSaGoClick CanMoveSuckerToCell-false branch now StopAllAdvancedTests +
  ShowMyMessage + return (was ShowMyOKMessageNoStop).
- Wiring: PopulateCcdCombos in BuildUI; RunCcdTest in tmrUpdate; StopCcdTest at all 3 stop sites
  (FormClose / EMG / btnStopClick).

## Interlocks honored
- Z (none for CCD). Shared Loader-Y rail: `MoveToCcdCell→MoveLoaderY→IsLoaderYMoveSafe` re-poll
  each tick (waits, never fails hard, if the other carriage blocks). Soft-limit inside
  MoveTopCcdX/MoveLoaderY + pre-checked in CanMoveCcdToCell. CheckCcdTestReady gates motor state.

## Modularization
- One numpad helper (EditCellWithNumpad) for all 4 cell edits (CCD col/row + SortArm col/row).
- Per-module `Can…ToCell`/`Move…ToCell(…,&Task)` shape (SortArm has it, Loader now gains it).
- uteach driver shape shared (combo→index→Can…→task-stepped Run…→3 stop sites), the tsTrayArm precedent.

## Build note (NOT mine)
The build initially failed on a pre-existing in-progress refactor: maintenance.cpp referenced
`GeneralSetting.bUseTrayDatumModel`, which the user had removed (replaced by ini-only
iSortArmXDatumBias/iSortArmYDatumBias, GeneralSetting.h:63-70). Removed the 2 orphaned maintenance.cpp
references to unblock; the dead `chkUseTrayDatumModel` checkbox remains in maintenance.dfm/.h for the
user to delete/repurpose. maintenance.cpp/GeneralSetting are the user's WIP and are NOT committed with
the CCD work.

## On-machine verification (user)
1. Teach LoaderCarFirstCCDXPosition + Loader1/2CarFirstCCDYPosition + tray-form XDivision/YDivision/pitch.
2. tsCCD: pick LoaderR/LoaderL, set Col/Row via numpad (capped at tray counts), GO → CCD lands on the cell.
3. Foolproof: a Col/Row beyond range (e.g. via no-recipe count=1, or pre-set text) → all tests stop +
   ShowMyMessage, no motion. Same for SortArm GO.
4. Numpad on SortArm edSaCol/edSaRow opens and clamps to the tray counts.
5. LoaderR/L orientation + per-side Y-encoder sign land on the correct physical cell.

## Open / deferred
- No-recipe edge: GetTrayXCount/YCount default to 1 → numpad caps at 1 (test needs a tray geometry first).
- Getter clamp inversion (GetTrayXCount clamps 1..50, GetTrayYCount 1..20 — inverted vs MAX_TRAY_X=20/
  MAX_TRAY_Y=50). Pre-existing; out of scope here. Setup UI enforces ≤20/≤50 so not hit in practice.
- Color single-pose 2D read test: deferred (separate feature).
