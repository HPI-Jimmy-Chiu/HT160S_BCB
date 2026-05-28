# HT172 Motor Table Establishment Analysis For HT160

Date: 2026-05-18
Target: D:\HT160S_BCB
Reference: D:\HT172\HT172_Program_V1.0.25.0_20260420 (read-only)

## Scope

This note records how HT172 establishes motor details from Mot_Table.csv and how the same model should be adapted into HT160S_BCB. HT172 is used only as a reference. Runtime data, motor quantities, and HT172 machine-specific motor items are not copied.

## Reference Files

- HT172 reference Mot_Table header: `D:\HT172\system\Mot_Table.csv`
- HT172 parser and loader: `D:\HT172\HT172_Program_V1.0.25.0_20260420\database.h`, `database.cpp`
- HT172 motor wrapper: `D:\HT172\HT172_Program_V1.0.25.0_20260420\MotorAndIO\mymotor.h`, `mymotor.cpp`
- HT160 target Mot_Table: `D:\HT160S_BCB\system\Mot_Table.csv`
- HT160 backup before migration: `D:\HT160S_BCB\system\Mot_Table_backup_20260518_before_172_format.csv`

## HT172 Motor Establishment Flow

```text
Mot_Table.csv
  -> TMOTNO::SetMOTTableNo(header)
  -> TMOTDATA(row)
  -> SYSTEM_MODULAR::LoadMotData()
  -> SYSTEM_MODULAR::LoadMotorParameterFromDataBase()
  -> TMyMotor::InitialMotorObject(address)
  -> concrete HTMotor subclass (SMC / MN200 / base HTMotor)
```

## HT172 Header Contract

HT172 uses a 29-column Mot_Table.csv header:

```csv
Motorname,Alias,Direction,GearRatio,HomeDirectior,HomeHighSpeed,HomeLowSpeed,InitSpeed,JogHighSpeed,JogLowSpeed,Rate,SoftLimitN,SoftLimitP,Enable,ServoAlarmOn,Range,1P2P,SensorType,SimulateSpeed,CardModel,BoardID,Port,Acc,Dec,MotorKind,FlushPanel,HomeOrder,LimitLogic,In1Logic
```

Important detail: the column is spelled `HomeDirectior` in HT172. Keep this spelling for format compatibility.

## TMOTNO Header Mapping

`TMOTNO::SetMOTTableNo()` does not rely only on hard-coded positions. It scans the header and records each field index by name. This makes the loader more tolerant of column order, as long as required names exist.

Required HT172 fields include:

| Field | Purpose |
| --- | --- |
| Motorname | Display/index label such as M00/M01 |
| Alias | Logical motor name used as the runtime lookup key |
| CardModel | Selects motor hardware class/path |
| BoardID, Port | Raw hardware address parts |
| Direction, HomeDirectior | Motion and home direction flags |
| GearRatio | Command-to-real-unit scaling |
| SoftLimitN, SoftLimitP | Software travel limits |
| Speed fields | Init/Home/Jog speed setup |
| Acc, Dec | Card-specific acceleration/deceleration source |
| MotorKind | HT172 motor type enum value; default 0 is normal motor |
| FlushPanel | UI/diagnostic grouping; can be blank for HT160 phase 1 |
| HomeOrder | Home sequencing metadata; can be blank for HT160 phase 1 |
| LimitLogic, In1Logic | Sensor polarity/logic flags |

## TMOTDATA Row Conversion

`TMOTDATA::TMOTDATA()` parses each row into typed fields. It also disables a row when critical enabled-row data is missing.

HT172 has card-specific fallback behavior:

| CardModel | HT172 Behavior |
| --- | --- |
| MC88X1 | Forces `iBoardID=0`; defaults `dAcc=1.0`, `dDec=1.0`, `iLimitLogic=0`, `iIn1Logic=0`, `iRange=10`, `iSimulateSpeed=1000` |
| MN200 | Later converts Acc/Dec values greater than 1 by dividing by 100.0 |
| SMC | Uses BoardID*10+Port address and raw Acc/Dec |
| SynTek | Uses MN200-style BoardID*100+Port address |

For HT160, existing MC88X1 rows already contain valid BoardID values. The target implementation should not copy HT172's forced `iBoardID=0` behavior because HT160 currently uses multiple MC88X1 boards.

## LoadMotData Behavior

`SYSTEM_MODULAR::LoadMotData()` loads the CSV file, parses the header with `TMOTNO`, creates `TMOTDATA` for each data row, and builds `mapMotTable` keyed by `Alias`. Duplicate aliases are treated as data errors.

The critical lookup key is `Alias`, not `Motorname`. Preserving HT160 aliases is therefore more important than preserving the M-number sequence.

## LoadMotorParameterFromDataBase Behavior

`SYSTEM_MODULAR::LoadMotorParameterFromDataBase()` maps `MotPtr[i]->Alias` to `HSys.MotTable` by alias and applies data to each `TMyMotor`:

1. Build address by CardModel.
2. Set `MotPtr[i]->CardModel`.
3. Create concrete motor object through `InitialMotorObject(iAdder)`.
4. Set enable state and motion card type.
5. Apply direction, home direction, gear ratio, speeds, soft limits, motor type, sensor type.
6. Apply Acc/Dec with card-specific conversion.
7. Apply range, init speed, home order, servo alarm flag, simulate speed, limit logic, and IN1 logic.
8. Call `InitMotor(iAdder)` when initial loading.

HT172 address rules:

| CardModel | Address Rule |
| --- | --- |
| SMC | `BoardID * 10 + Port` |
| MN200 / SynTek | `BoardID * 100 + Port` |
| MC88X1 | HT172 parser has partial MC88X1 special cases, but no HT172 concrete MC88X1 class in `TMyMotor::InitialMotorObject()` |

HT160 already added a MC88X1 branch in `TMyMotor::InitialMotorObject()`, so HT160 should use `BoardID * 10 + Port` for MC88X1 and let `TMyMC88X1Motor::InitMotor()` translate to MC88X1 hardware address.

## HT160 Mot_Table Conversion Decision

HT160 should adopt the HT172 29-column format exactly for `system/Mot_Table.csv`.

The old HT160 fields below will be removed from the CSV:

| Old HT160 Field | New Handling |
| --- | --- |
| MotorCardType | Replaced by `CardModel` and code-level `eMotionCardType` |
| HomeType | Hardcoded in `TMyMC88X1Motor` as 90 |
| HomeStep | Hardcoded in `TMyMC88X1Motor` as 5 |
| HomeStepRange | Hardcoded in `TMyMC88X1Motor` as 100 |

The following HT172 fields will be added for HT160 rows:

| New HT172 Field | HT160 Phase-1 Value |
| --- | --- |
| MotorKind | `0` |
| FlushPanel | blank |
| HomeOrder | blank |
| LimitLogic | `0` for MC88X1 |
| In1Logic | `0` for MC88X1 |

## HT160 Current 18-Motor Mapping

Current HT160 aliases should be preserved:

```text
MSortingArmX, MTrayArmX, MEmptyY, MLoaderY_1, MLoaderY_2,
MAutoY_1, MAutoY_2, MAutoY_3, MAutoY_4, MAutoY_5, MAutoY_6,
MTopCCDX, MBottomCCDY, MSuckZ_1, MSuckZ_2, MSuckZ_3, MSuckZ_4, MPitchX
```

All current HT160 rows are `CardModel=MC88X1` and all old HomeType/HomeStep/HomeStepRange values were confirmed as `90/5/100` before the CSV format migration.

## Risks And Guards

| Risk | Guard |
| --- | --- |
| Losing MC88X1 home behavior | Keep `90/5/100` hardcoded in `TMyMC88X1Motor` |
| Alias mismatch prevents parameter load | Preserve Alias values exactly |
| BoardID rule differs from HT172 MC88X1 special handling | HT160 loader should keep BoardID and use `BoardID*10+Port` for MC88X1 |
| CSV format validates but motor hardware not active | Build verification only proves compile/link; machine motion needs later guarded runtime test |
| HT172 machine-specific data contaminates HT160 | Do not copy HT172 row data or motor item counts |

## Recommended Next Steps

1. Convert `D:\HT160S_BCB\system\Mot_Table.csv` to HT172's 29-column header using current HT160 row values.
2. Preserve all HT160 aliases and motion values.
3. Add HT160 loader support for `TMOTNO`, `TMOTDATA`, `MotTable`, `mapMotTable`, and `LoadMotData()` when the motor database path is implemented.
4. Keep HT160 non-FSM and BCB6-compatible.
5. Run BCB6 clean build after any C++/project edit.
