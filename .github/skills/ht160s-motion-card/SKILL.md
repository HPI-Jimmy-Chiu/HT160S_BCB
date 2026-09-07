---
name: ht160s-motion-card
description: "Use when working on HT160S_BCB motor / motion-card code: TMyMC88X1Motor, MC88X1P_DLL, MC88X1PLazyLoad, HTMotor, TMyMotor wrapper, myMN200motor stub, Mot_Table.csv, motor startup open-card flow, Teach/uMotorTest jog, home (HomeType 0-8 / dormant HomeType90), HomeFlag, speed/accel/Range/Acc tuning, MC88X1PMotAxisParaSet range rejection. Triggers: MC88X1, TMyMC88X1Motor, InitMotor, Open_MC88X1Card, bCardOpened, motor jog fail, motor not moving, GearRatio, Mot_Table, SOFT_SIMULATE motor, HomeType, HomeType90, HomeFlag, MotionDone, AxisParaSet, SV/DV/MDV/AC, SCW, torque overload, home repeatability, IN3."
---

# HT160S Motion Card (MC88X1) Skill

## Authoritative reference (align all motion/home/speed changes to this)

`docs/MC88X1_Driver/MC88X1_technical-note.md` (built from the vendor manuals in
`docs/MC88X1_Driver/`). It is the source of truth for the speed model, parameter ranges,
home modes, status registers and the known-issue history. Before changing speed/home/
parameter behaviour, read it; after changing, update it. Do NOT re-derive these from memory.

### Speed/home model (the non-obvious invariants)

- Card speed = `FUNIT x SPEED_DATA`, `FUNIT = FCLK/(RANGE_DATA x 262144)`. `MC88X1PMotAxisParaSet(
  board,axis,ts,SV,DV,MDV,AC,AK)`: DV=run speed (jog/Cmove use this), MDV=max speed (the card
  derives the valid SV/DV/AC/SCW window FROM this), AC=accel (RATE-A, 1..4095), AK=SCW (S-curve).
- This port feeds the card `SV=InitSpeed*Range`, `DV=iSpeed*Range`, `MDV=JogHighSpeed*Range`,
  `AC=(DV-SV)/Acc` (Acc = Mot_Table Acc column, seconds). `Range`/`Acc` are Mot_Table columns.
- Out-of-range params make AxisParaSet RETURN an error (0x1000..0x10E9) and NOT apply -> the axis
  silently keeps stale params (this is why "HomeHighSpeed=200 ran SLOWER than 10"). The home
  window is sized by MDV, so a huge JogHighSpeed vs tiny HomeHighSpeed (wide ratio) gets the home
  speed/accel rejected. Lever to fix: lower JogHighSpeed or raise HomeHighSpeed; lower Acc raises
  accel. `Range` is RATIO-neutral (scales home and MDV together). Torque overload = accel too
  steep -> raise Acc (or lower Range/speed).
- Home: card HomeType is 0..8 (table in the note); **all axes use card-native type 7** (find Home
  -> leave -> re-enter -> hardware stop on IN3). Type 90 is a SOFTWARE pseudo-mode (`HomeType90()`),
  now dormant. Card-native home is latency-immune (sensor->stop in hardware) -> preferred as the
  software grows / many axes home together. Home sensor is always IN3 = `MC88X1PMotDI` bit 0x08.

## Purpose

Maintain the MC88X1 axis-card motor layer without re-deriving the whole call chain.
All 20 physical motors are MC88X1 (boards 0/1/2, 8 axes per board). MN200 motor class
is a stub (no card control) — the real MN200 work is IO (`myio.cpp`, `OpenMN200Card()`).

## File map

- `HT160S_Program_BCB_V1.0.0.0/MotorAndIO/myMC88X1motor.cpp/.h` — TMyMC88X1Motor (HTMotor subclass)
- `MotorAndIO/HTMotor.h/.cpp` — virtual base; check signatures here before adding overrides
- `MotorAndIO/MyMotor.cpp` — TMyMotor wrapper; `InitialMotorObject()` news the card class by CardModel string
- `MotorAndIO/MC88X1PLazyLoad.cpp` — runtime LoadLibrary("MC88X1P_DLL.dll") stubs; NO static .lib link
- `MotorAndIO/MC88X1P_DLL.h` — vendor API (function names contain vendor typo "Theorectical": keep as-is)
- `system/Mot_Table.csv` — per-motor config (Enable, GearRatio, CardModel, BoardID, Port...)
- `uMotorTest.cpp` — Teach/motor-test screen (jog at ~2921, save/reload at 1851/2099/2122)
- Machine-proven baseline: `D:\HT160S -Original 20260323\Code_V300A\Program_HT160S_20240806_20241111-SECSGem\myMC88X1motor.cpp` (READ-ONLY)

## Startup / open-card flow (verified 20260612)

1. `SYSTEM_MODULAR::LoadDataBase()` (database.cpp ~715) -> `OpenMN200Card()` -> `LoadMotorParameterFromDataBase()` (default Index=-1, bInitial=true).
2. `LoadMotorParameterFromDataBase` (database.cpp ~1475) per motor: `InitialMotorObject(addr)` (new TMyMC88X1Motor) -> `SetEnable(Data->iEnable)` (forced false under SOFT_SIMULATE) -> all parameter setters -> `if(bInitial) InitMotor(iAdder)`.
3. `TMyMC88X1Motor::InitMotor` opens the card (`Open_MC88X1Card`, per-board install flag + ref count) then configures axis registers. Since 20260612 it opens the card even when `Enable=false` (skipped under SOFT_SIMULATE) — MN200-style open-at-boot fix.
4. Address coding MC88X1: `addr = BoardID*10 + Port` (MN200/SYNTEK uses *100).

## Known traps (each cost a real debug session)

- **bInitial=false reload never calls InitMotor.** uMotorTest save/reload uses
  `LoadSingleMotorParameterFromDataBase(idx,false)`. A motor enabled at runtime relies on the
  card having been opened at boot. Do not gate `Open_MC88X1Card()` behind `Enable`.
- **SOFT_SIMULATE is defined in `MachineType.h`** (dev default). It forces every motor
  `SetEnable(false)` and skips card/ring opens. Machine builds MUST comment it out and full
  rebuild (`scripts/ops/build-ht160s.ps1 -Clean`).
- **Lazy DLL load fails silently.** If `MC88X1P_DLL.dll` is missing next to the EXE, every API
  returns ERROR_FILE_NOT_FOUND / FALSE with no dialog (legacy static link crashed loudly).
  On-machine "all motors dead" => check DLL presence first.
- **Pulse vs unit domain.** Card registers hold raw pulses; user positions are pulses*GearRatio.
  GearRatio values in Mot_Table are fractional (0.9, 0.5, 0.1, 0.04). Never compare positions by
  round-tripping through GearRatio — compare raw pulses (see MoveTo, fixed 20260612). Use
  `RoundPulseByGear()` for unit->pulse.
- **Direction negation lives INSIDE SetCommand/SetPosition in this port** (legacy SetCommand did
  not negate). Callers must NOT pre-negate (SetPos double-negation fixed 20260612).
- **Guard pattern:** every card call requires `Enable && bCardOpened`. JogP/JogN/MoveTo also call
  `MotorIdleSafeDoorCheck` and `MotionDone()` first; failures return false silently — when "motor
  does not move", check these guards before suspecting the card.

## Fixed 20260612 (do not regress)

1. `InitMotor`: open card at boot even when disabled (`#ifndef SOFT_SIMULATE`).
2. `MoveTo`: completion check reads `MC88X1PGetTheorecticalRegister` raw pulses and compares to
   TargetPulse (legacy-exact); the old unit-domain compare never matched for GearRatio 0.9/0.04.
3. `SetPos`: removed legacy pre-negation (SetCommand negates internally).

## Fixed 20260617 (do not regress)

1. `HomeFlag()`: ALL home types read the home sensor as `MC88X1PMotDI` bit 0x08 (IN3),
   matching `ScanMotorStatus`. The old non-90 path read `ReadStatus(0x08)` bit 0x0080 with
   inverted polarity -> card-native axes reported HomeFlag=0 at home -> false HOME_DONE_TIMEOUT.
2. Param-range diagnostics: `SetMC88X1MotPara` captures the AxisParaSet return code
   (`GetLastParaError`); `VerifyHomeParaRange` dry-runs the type-90 home profile and returns the
   card verdict (guarded to type-90 only). TMyMotor delegates; uMotorTest Save reports any
   rejected run/home params via `DecodeAxisParaError`. Touches HTMotor.h/MyMotor.*/myMC88X1motor.*/
   uMotorTest.cpp -> header struct change, build with `-Clean`.
3. All axes -> card-native HomeType 7 in `database.cpp` (MTopCCDX/MTopCCDX_Color were 90).
   `HomeType90()` kept dormant as a per-axis fallback.
4. Limit pre-escape added to `MC88X1MotHome` (cases 2-4): card-native home only drives
   HomeP0_Dir and will push HARDER into a hard limit it is pinned on (observed CW-limit crash).
   Now it jogs OFF any lit limit (JogN for CW, JogP for CCW) before issuing MC88X1PMotHome,
   timeout-bounded. CAVEAT: if an axis's home dir points back at the escaped limit with the home
   sensor right at the edge, escape may be insufficient -> keep that axis on type 90. So type 7
   vs 90 can be a PER-AXIS decision, not global.

## Rules

- Non-FSM, BCB6 only (no C++11), Big5 source preserved, new comments ASCII English.
- HomeType90 / MC88X1MotHome are `switch(Task)` polled state machines driven by repeated
  `HomeObject()` calls — keep non-blocking, no Sleep().
- After edits: delete `MotorAndIO\myMC88X1motor.obj`, run `scripts/ops/build-ht160s.ps1`;
  wiring/header changes -> `-Clean`.
- Compare against the Original-20260323 SECSGem baseline before changing motion semantics; it is
  the machine-proven reference.
