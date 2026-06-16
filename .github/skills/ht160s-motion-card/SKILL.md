---
name: ht160s-motion-card
description: "Use when working on HT160S_BCB motor / motion-card code: TMyMC88X1Motor, MC88X1P_DLL, MC88X1PLazyLoad, HTMotor, TMyMotor wrapper, myMN200motor stub, Mot_Table.csv, motor startup open-card flow, Teach/uMotorTest jog, HomeType90. Triggers: MC88X1, TMyMC88X1Motor, InitMotor, Open_MC88X1Card, bCardOpened, motor jog fail, motor not moving, GearRatio, Mot_Table, SOFT_SIMULATE motor, HomeType90, MotionDone."
---

# HT160S Motion Card (MC88X1) Skill

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

## Rules

- Non-FSM, BCB6 only (no C++11), Big5 source preserved, new comments ASCII English.
- HomeType90 / MC88X1MotHome are `switch(Task)` polled state machines driven by repeated
  `HomeObject()` calls — keep non-blocking, no Sleep().
- After edits: delete `MotorAndIO\myMC88X1motor.obj`, run `scripts/ops/build-ht160s.ps1`;
  wiring/header changes -> `-Clean`.
- Compare against the Original-20260323 SECSGem baseline before changing motion semantics; it is
  the machine-proven reference.
