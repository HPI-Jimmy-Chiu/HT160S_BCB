---
name: ht160s-installer
description: >-
  Use when building / producing the HT160S machine-deployment installer package
  (the NSIS Setup.exe), or when the user asks to "package", "make the installer",
  or ship the program to the machine. Triggers: 打包, 做安裝包, 安裝包, 出機台版,
  installer, NSIS, makensis, build.bat, installer.nsi, Setup.exe, deploy to machine,
  SOFT_SIMULATE off, safety-door bypass. Covers the patch -> package -> revert
  workflow.
---

# HT160S Installer / Machine-Deployment Packaging

The installer turns the program **source folder** into a single
`HT160S_Setup_<ver>.exe` that, when run on the machine, **backs up the existing
version to `D:\backup` (7z) and then deploys the new source files**.

## Key facts

- **Source-only deployment.** The package ships the program source folder
  (`D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0`). The **machine compiles it
  locally** with C++Builder. So packaging does **NOT** compile here, and does
  **NOT** include the exe/DLLs (those live in the sibling `D:\HT160S_BCB\EXE\`
  and are intentionally out of scope — the machine builds its own exe from the
  deployed source).
- **NSIS does not compile** — `installer.nsi` only zips the source folder. That
  is exactly what is wanted here.
- **The pain this solves.** Before every machine release the source must be
  switched from simulation to real-machine mode by hand. This script automates
  those edits, packages, then reverts so the dev workspace stays in simulation.

## One command

```powershell
pwsh D:\HT160S_BCB\scripts\ops\build-installer.ps1
```

What it does (in order):
1. Asserts the source is in a clean **dev** state (active `#define SOFT_SIMULATE`,
   no leftover `[installer]` marker). Refuses otherwise — prevents a crashed
   prior run from being captured as the "dev" backup.
2. Backs up `MachineType.h` + `csystem.cpp` byte-exact to
   `scripts\ops\.installer-tmp\` (**outside** the packed folder, so backups are
   never shipped).
3. Applies the **machine-profile patches** (byte-safe ISO-8859-1 round-trip, so
   the legacy Big5 bytes in `csystem.cpp` are preserved exactly — verified
   byte-identical revert):
   - `MachineType.h`: comment out `#define SOFT_SIMULATE` → real hardware I/O.
   - `csystem.cpp`: `IsSafeDoorOpen()` gets an unconditional `return 0;` at the
     top (safety door not yet physically installed on the machine).
4. Runs NSIS directly (`makensis installer.nsi`) to zip the source folder. (It
   calls makensis, **not** `build.bat`, because `build.bat` delegates back to
   this script — calling it would recurse.)
5. **Reverts** `MachineType.h` + `csystem.cpp` from backup in a `finally` block
   (runs even if packaging fails), so the workspace returns to simulation mode
   and the dev source is never left dirty.

Output: `D:\AI_Area\ClassTool\HT160S_Installer\build\HT160S_Setup_<ver>.exe`.

## Flags

| Flag | Effect |
|------|--------|
| (none) | Full flow: patch → package → revert. |
| `-SkipPackage` | Dry run: prove the patch/revert is byte-clean (no NSIS). |
| `-KeepPatched` | Leave source in machine mode after the run (prints restore commands). |
| `-InstallerDir <path>` | Override the NSIS installer folder. |

## Entry point: build.bat delegates here

The user's habit is to double-click `build.bat` in the installer folder. It is a
thin wrapper that calls this script:
`powershell -ExecutionPolicy Bypass -File D:\HT160S_BCB\scripts\ops\build-installer.ps1`.
So a normal double-click now does patch → package → revert automatically.

## Locations (do not confuse)

- Installer project (NSIS): `D:\AI_Area\ClassTool\HT160S_Installer\`
  (`installer.nsi`, `build.bat`, `tools\7zr.exe`, `README.md`). This tree is
  **outside the write boundary** — do not edit files there with the file tools.
  Editing `build.bat` / `installer.nsi` requires the user to paste the change.
  The editable packaging logic lives in the writable
  `scripts\ops\build-installer.ps1`.
- Packaging orchestrator (writable): `D:\HT160S_BCB\scripts\ops\build-installer.ps1`.

## Changing the version

`APP_VERSION` is hard-coded near the top of `installer.nsi` (outside the write
boundary, so the user edits it). It flows into the output filename and title.

## Adding a new machine-profile patch

If a future "machine vs simulation" difference must be baked at packaging time
(not a runtime toggle), add it to the patch section of `build-installer.ps1`:
back up the file, apply a byte-safe `[regex]::Replace` keyed off a unique
`[installer]` marker (for idempotency + the clean-state assertion), and verify
it landed. Prefer the existing `#ifdef SOFT_SIMULATE` mechanism when the
difference is purely sim-vs-real I/O; only patch when there is no compile flag.
