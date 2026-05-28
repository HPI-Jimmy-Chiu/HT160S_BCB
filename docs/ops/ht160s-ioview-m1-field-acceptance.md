# HT160S IOView M1 Field Acceptance

## Purpose

This checklist closes M1: IOView as a safe IO verification platform.

M1 is not a full Pad-to-machine-control rollout. It verifies that IOView can be used to inspect IO mapping, edit `IO_Table.csv` safely, manually toggle outputs under guarded conditions, and observe Pad display synchronization without wiring Pad keys into production actions. Pad panel aliases are communication-backed `COMM_PAD` rows, not physical IO addresses.

## Scope

Included:

- Maintenance -> IO Monitor opens IOView.
- IO table editor can search, modify, save, back up, and reload `system\IO_Table.csv`.
- IO map memo reports duplicate addresses, unbound UI components, disabled IO, and manual output log.
- Manual output is blocked while the machine is running, while safe door is open, or when Manual output is unchecked.
- Legacy `TBtnPanel` output requires General IO mode and Manual output.
- If PadInterface is visible, matching Pad switch display status is synchronized from IOView output state only.

Excluded from M1:

- Macro/Marco in IOView.
- Full HT172 ComPort Bin/StepTray/MotorTorque import.
- Pad Start/Home/Pause/Reset machine-action routing.
- Full `TMyMN200_IO` object model import.
- Any HT172 or archived old160 write operation.

## Pre-Check

Run the read-only readiness check before touching hardware:

```powershell
cd D:\HT160S_BCB
powershell -ExecutionPolicy Bypass -File .\scripts\ops\test-ioview-m1-readiness.ps1
```

Expected base result:

- Exit code `0`: IO table has no structural error. Warnings may still exist and should be reviewed.
- Exit code `1`: IO table has structural errors or strict Pad coverage failed.
- Exit code `2`: script could not parse required input.

Known non-blocking warning:

- `SwServerON` is treated as a virtual output alias. It may be enabled without a physical IO address and will be reported as a warning, not a blocking error.
- Pad aliases from `uPadInterface.cpp` should be tagged `COMM_PAD`, stay disabled as physical IO, and carry no Lane/IP/Port/Bit address.

Evidence is written to:

```text
logs/ioview-m1-readiness/YYYYMMDD-HHMMSS/result.json
```

Use strict Pad coverage only when the panel alias list is expected to be complete:

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\ops\test-ioview-m1-readiness.ps1 -StrictPadCoverage
```

## Build And Startup Gate

After any C++/DFM/project-file change:

```powershell
cd D:\HT160S_BCB
powershell -ExecutionPolicy Bypass -File .\scripts\ops\build-ht160s.ps1 -Clean
powershell -ExecutionPolicy Bypass -File .\scripts\ops\test-ht160s-startup.ps1 -Clean -StartupSeconds 8 -MaxAttempts 1 -ProbeTopForms
```

Pass criteria:

- BCB6 build completes without error.
- Startup smoke status is `Pass`.
- No `ht160s.exe` is left running after the smoke test unless intentionally kept.

## Field Acceptance Checklist

### 1. Open IOView

Steps:

1. Start HT160S.
2. Open Maintenance.
3. Click IO Monitor.

Pass criteria:

- IOView opens without exception.
- Sensors, Cylinders, Switches, Suckers, and IO Table tabs can be selected.
- Summary text and selected item text update normally.

### 2. IO Table Editor

Steps:

1. Open IO Table tab.
2. Filter by Sensor, Switch, Cylinder, and Sucker.
3. Search a known alias such as `SnFKPowerOff` or `SwFKPowerOff`.
4. Modify a harmless Note field and save.
5. Confirm backup creation under `system\backup`.

Pass criteria:

- Save is blocked if required fields are invalid.
- Save creates `IO_Table_yyyymmdd_hhnnss.csv` backup.
- Save reloads IO data and refreshes IOView.
- Existing table columns do not shift after reload.

### 3. IO Map Diagnostics

Steps:

1. Click Refresh.
2. Review `MemoIOMap`.
3. Save Input Map and Output Map.

Pass criteria:

- Duplicate addresses, unbound UI aliases, disabled IO count, and manual output log are visible.
- Pad communication aliases appear as `Pad COM` / `COMM`, not as missing physical addresses.
- Output files are written under `io_map\InputMap.csv` and `io_map\OutputMap.csv`.
- Any warning is understandable enough to map back to `IO_Table.csv`.

### 4. Manual Output Gate

Steps:

1. Leave Manual output unchecked and try ON/OFF/Destroy.
2. Open safe door and try enabling Manual output.
3. Start machine running state if safe to simulate, then try Manual output.
4. Return to stopped and safe-door-closed state, then enable Manual output.

Pass criteria:

- Manual output is blocked when unchecked.
- Manual output is blocked while safe door is open.
- Manual output is blocked while `HSys.Sys.SystemStart` is true.
- Header buttons are enabled only for selected output targets.

### 5. Manual Output Operation

Steps:

1. Select a known low-risk switch output.
2. Click ON and OFF.
3. Review IOView LED/button state.
4. Review `MemoIOMap` manual output log.

Pass criteria:

- Output toggles only under allowed gate conditions.
- Result is logged as `OK`, `BLOCK`, `NO_SELECTION`, or `UNBOUND_OR_DISABLED`.
- UI refresh reflects the new output state.

### 6. Pad Display Sync

Steps:

1. Open IOView Pad button to show PadInterface.
2. Select or click a Pad-related switch alias, such as `SwFKPause` or `SwRKAlarmReset`.
3. Toggle it from IOView.
4. Observe PadInterface display state.

Pass criteria:

- Matching Pad UI button/LED state follows IOView output state.
- No Pad key machine action is triggered by this display sync.
- PadInterface remains usable after IOView is closed and reopened.

## Sign-Off Notes

Record the following when M1 is validated on hardware:

- Date/time.
- Machine serial or bench identifier.
- IO table readiness evidence folder.
- Startup smoke evidence folder.
- Manual output aliases tested.
- Pad aliases tested.
- Any warnings intentionally accepted.
