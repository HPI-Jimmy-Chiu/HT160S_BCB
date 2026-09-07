# TrayArm Teach-Test Tab (tsTrayArm) — Locked Implementation Plan

Status: PLAN (no edits yet). Created 2026-06-27.
Owner decisions captured (see §0). Build gate per `.github/instructions/ht160s-development.instructions.md`.

Related memory: `tsadvanced-teach-test-architecture`, `trayarm-zup-move-interlock`,
`cylinder-idiom-pushcylinder-canonical`, `color-supply-mirrors-empty-feed`,
`bcb6-dfm-events-must-be-published`, `bcb6-no-comments-in-form-class-body`.

## 0. Locked decisions (from user 2026-06-27)

1. **Sync = single code path.** Extract the physical grab/place choreography of
   `DoPick`/`DoPlace` into shared private helpers `DoGrabMotion`/`DoReleaseMotion`;
   BOTH production and the new test call them. If the motion/interlock changes once,
   the test inherits it — no parallel clone.
2. **Test semantics = pure motion dry-run.** The test sequences only the physical
   primitives. It does NOT call peer-module state mutations
   (`NotifyTrayPicked` / `StageRearGrid` / `SetRearHasTray*` / `NotifyTrayArm*` /
   `NotifyTrayXToEmptyFinish`). Repeatable; never corrupts born-at-source identity.
3. **Place targets = Auto1-6 + recycle Empty/Color.** Grab sources = Empty/Color/Loader.

## 1. Channel index scheme (the "same function, different index")

Flat channel id space (units of all X = 1/100mm):

| id | Channel | Handoff X (Teach field, uteach.h) | Grab src? | Place dst? |
|----|---------|-----------------------------------|-----------|------------|
| 0  | Empty   | `TrayXArmToEmptyXPosition` (:48)  | yes       | yes (recycle) |
| 1  | Color   | `TrayXArmToColorXPosition` (:50)  | yes       | yes (recycle) |
| 2  | Loader  | `TrayXArmToLoaderXPosition` (:49) | yes       | no (pick-only) |
| 3  | Auto1   | `TrayXArmToAuto1XPosition` (:55)  | no        | yes |
| 4  | Auto2   | `TrayXArmToAuto2XPosition` (:56)  | no        | yes |
| 5  | Auto3   | `TrayXArmToAuto3XPosition` (:57)  | no        | yes |
| 6  | Auto4   | `TrayXArmToAuto4XPosition` (:58)  | no        | yes |
| 7  | Auto5   | `TrayXArmToAuto5XPosition` (:59)  | no        | yes |
| 8  | Auto6   | `TrayXArmToAuto6XPosition` (:60)  | no        | yes |

Grab combo = {0,1,2}. Place combo = {3,4,5,6,7,8,0,1}.

Adapter (free functions in aTrayArm.cpp, procedural, no virtual base):
```
int  GetChannelHandoffX(int Channel);   // switch -> the 9 Teach.TrayXArmTo*XPosition
bool ChannelGrabReady(int Channel);     // Empty IsRearHasTray / Color IsTrayReady / Loader IsRearHasTray
bool ChannelPlaceClear(int Channel);    // destination rear NOT occupied (anti-clash); Auto via Auto ready
const char* GetChannelName(int Channel);
```
Normalizes singleton (Empty/Color) vs `Side[2]` (Loader) vs `State[6]` (Auto) addressing
inside the switch. `GetAutoX(0..5)` already exists (aTrayArm.cpp:176) — reuse for ids 3..8.

## 2. aTrayArm.h/.cpp — refactor + public test API (Phase 0)

### 2.1 Extract shared physical helpers (private)
```
bool DoGrabMotion(int X, int &Task);    // Z-up -> MoveTrayArmX(X) -> Z-down -> push BOTH clamps + settle -> Z-up
bool DoReleaseMotion(int X, int &Task); // Z-up -> MoveTrayArmX(X) -> Z-down -> pop  BOTH clamps + settle -> Z-up
```
These are the PHYSICAL choreography ONLY (no peer notify, no grid transfer).
Source today: `DoPick` cases 1/10/1000/2000/2100/3000 (aTrayArm.cpp:303) and
`DoPlace` cases 1/10/1000/2000/3000 (aTrayArm.cpp:392).

### 2.2 Route production through the helpers (keep state mutations OUTSIDE)
- `DoPick`  -> `DoGrabMotion(GetPickSourceX(), PickTask)` then its case4000 keeps the
  grid CopyFrom + iDeliverTrayID + per-source notify (Loader/Color/Empty) + fHasTray set.
- `DoPlace` (Auto path) -> `DoReleaseMotion(GetAutoX(iAutoTarget), PlaceTask)` then case4000
  keeps `StageRearGrid` + AMR/normal notify + clear Tray.
- `DoPlaceToEmpty` / `DoPlaceToColor` -> keep their rear-clear WAIT
  (`wait IsRearHasTray()==false`) as a step BEFORE `DoReleaseMotion(channelX, Task)`, then keep
  `NotifyTrayXToEmptyFinish`. The wait stays in the recycle wrapper, NOT in the shared helper,
  so the helper is purely physical (decision #2).

CRITICAL: behavior of production must be byte-for-byte equivalent after refactor.
Verify via the home self-test + sim run.

### 2.3 Public test API
```
bool TestGrabFromChannel(int Channel, int &Task);  // -> DoGrabMotion(GetChannelHandoffX(Channel), Task)
bool TestPlaceToChannel (int Channel, int &Task);  // -> DoReleaseMotion(GetChannelHandoffX(Channel), Task)
bool CanTestTrayArm(int Channel, bool bGrab, AnsiString &Err); // parametric gate
```
- Pure dry-run: NO peer-module state calls.
- `CanTestTrayArm`: mirror SortArm's `CanMoveSuckerToCell` shape +
  require `IsZUpAtPosition()` before any X move; for grab optionally check `ChannelGrabReady`
  (warn-not-block, since dry-run may run on empty clamp); for place require `ChannelPlaceClear`
  (anti-clash — do NOT lower+release onto an occupied rear).

OPEN (resolve during Phase 0, default = safe):
- Place-to-Color reuses `TrayXArmToColorXPosition` as deposit X (aTrayArm.cpp:628 flags a
  possible on-machine tray-on-tray clash). Test uses the same X; flag in status label.
- Grab dry-run on empty clamp: allowed (warn only), since no tray-tracking mutation.

## 3. uteach UI (Phases 1-3) — model on tsSortArm/tsChannel

### DFM (tsTrayArm, uteach.dfm:996) — back up uteach.dfm + uteach.h first
- `gbTaGrab` "TrayArm Grab Test": `cbTaGrabChannel` (Empty/Color/Loader),
  `btnTaGrab` "GO (Grab Tray)" OnClick=btnTaGrabClick, `lblTaStatus`.
- `gbTaPlace` "TrayArm Place Test": `cbTaPlaceChannel` (Auto1..6/Empty/Color),
  `btnTaPlace` "GO (Place Tray)" OnClick=btnTaPlaceClick.
- No jog (single-axis jog already lives in palMotorControl, gated by CheckCanTeachMove
  which already enforces the TrayArm Z-up interlock, uteach.cpp:843-849).

### uteach.h — __published, fields before handlers, NO comments in class body
- controls + 2 OnClick decls in `__published` (after tsTrayArm, uteach.h:251).
- private state after iAutoIndex (uteach.h:303): `bTaTestRunning`, `iTaTask`, `iTaChannel`, `bTaIsGrab`.
- private method decls: `CheckTrayArmTestReady`, `RunTrayArmTest`, `StopTrayArmTest`,
  `SetTaStatus`, `PopulateTrayArmCombos`.

### uteach.cpp
- `btnTaGrabClick` / `btnTaPlaceClick`: read combo -> CheckTrayArmTestReady ->
  CanTestTrayArm -> set bTaIsGrab/iTaChannel/iTaTask=0/bTaTestRunning=true.
- `RunTrayArmTest`: each tick call TestGrabFromChannel/TestPlaceToChannel(iTaChannel, iTaTask);
  clear flag on completion. Add the call at tmrUpdateTimer (uteach.cpp:1041).
- `StopTrayArmTest`: clear flag + MTrayArmX->Stop(). Add to all THREE stop sites:
  FormClose (uteach.cpp:114), EMG path (uteach.cpp:1000), btnStopClick (uteach.cpp:1195).
- `PopulateTrayArmCombos`: seed combos from BuildUI.
- `CheckTrayArmTestReady`: replicate CheckSortArmTestReady gate (uteach.cpp:1249) for MTrayArmX
  + require IsZUpAtPosition() true.

## 4. Phases (each build-verifiable)

- **Phase 0** — aTrayArm refactor (DoGrabMotion/DoReleaseMotion) + adapter + public test API.
  Delete aTrayArm.obj; build `-Full` (method/struct change). Run `ht160s-home-selftest`
  (regression: every Mot_Table axis HOMED, no production behavior change). Sim run sanity. EXIT 0.
- **Phase 1** — DFM tsTrayArm controls (back up dfm+h, re-inject from HEAD if designer strips).
- **Phase 2** — uteach.h decls (__published controls/handlers + private state/methods).
- **Phase 3** — uteach.cpp handlers + tmrUpdateTimer pump + 3 stop sites + combo seeding.
  Delete uteach.obj; build `-Clean`. EXIT 0.
- **Phase 4** — Real-machine build gate: comment out `#define SOFT_SIMULATE` in MachineType.h,
  `-Full`, confirm EXIT 0 (proves the #ifndef Z-up interlock branch still compiles), RESTORE define,
  rebuild. Run `scripts/ops/check-ht160s-source-encoding.ps1`.
- **Phase 5 (future)** — migrate existing tsChannel Empty/Color/Loader tests + SortArm cbToArea
  onto the same GetChannelHandoffX index adapter (full modularization). Address Auto GoDown /
  index-base asymmetry. Deferred until Phase 0-4 verified on-machine.

## 5. Interlocks that MUST be honored (non-negotiable)
- Z-up before X — inherited automatically because the test calls `MoveTrayArmX`
  (Z-up debounce + StopAllMotor on confirmed loss, aTrayArm.cpp:137-149). DO NOT bypass.
- Soft-limit — inside MoveTrayArmX (aTrayArm.cpp:124-128).
- SOFT_SIMULATE: IsZUpAtPosition returns true ONLY under compile-time #ifdef (aTrayArm.cpp:113-117),
  NOT a runtime DUMMY bypass. Do NOT add new `||IsSoftSimulate()` shortcuts on the Z-up confirm.
- Place anti-clash: require destination rear clear before Z-down+release.

## 6. Files touched (all under D:\HT160S_BCB, writable)
- aTrayArm.h / aTrayArm.cpp (helpers + adapter + public test API + production re-route)
- uteach.dfm (tsTrayArm controls)
- uteach.h (__published + private state/methods)
- uteach.cpp (handlers, pump @1041, 3 stop sites @114/1000/1195, combos in BuildUI)
