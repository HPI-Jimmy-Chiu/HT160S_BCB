# build-installer.ps1
# Build the HT160S machine-deployment installer package.
#
# This machine ships SOURCE CODE ONLY -- the machine compiles it locally with
# C++Builder. NSIS just zips the program source folder. So this script does NOT
# compile and does NOT package the exe / DLLs.
#
# Its only job: apply the two "machine profile" source edits that are tedious to
# redo by hand on every release, package, then revert the workspace back to dev:
#   1. MachineType.h : comment out  #define SOFT_SIMULATE   (-> real hardware I/O)
#   2. csystem.cpp   : IsSafeDoorOpen() returns 0 unconditionally
#                      (safety door not yet physically installed on the machine)
#
# Flow: assert-clean -> backup -> patch -> NSIS package -> revert (in finally).
# Edits are byte-safe (ISO-8859-1 round-trip) so the legacy Big5/CP950 bytes in
# csystem.cpp are preserved exactly (verified byte-identical revert).
#
# Usage:
#   pwsh scripts\ops\build-installer.ps1                # patch -> package -> revert
#   pwsh scripts\ops\build-installer.ps1 -SkipPackage   # dry run: prove patch/revert is byte-clean
#   pwsh scripts\ops\build-installer.ps1 -KeepPatched   # leave source patched (no revert)

param(
    [string]$InstallerDir = "D:\AI_Area\ClassTool\HT160S_Installer",
    [switch]$SkipPackage,       # skip the NSIS packaging step (patch/revert test only)
    [switch]$KeepPatched        # do NOT revert source on exit (workspace stays in machine mode)
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

$repoRoot    = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$projectRoot = Join-Path $repoRoot "HT160S_Program_BCB_V1.0.0.0"
$machineType = Join-Path $projectRoot "MachineType.h"
$cSystem     = Join-Path $projectRoot "csystem.cpp"
$nsiFile     = Join-Path $InstallerDir "installer.nsi"

# Backups live OUTSIDE the packed folder ($projectRoot) so they are never
# shipped inside the installer.
$backupDir   = Join-Path $PSScriptRoot ".installer-tmp"
$mtBackup    = Join-Path $backupDir "MachineType.h.devbak"
$csBackup    = Join-Path $backupDir "csystem.cpp.devbak"

$MARKER      = "[installer]"   # patch marker used for idempotency / clean-state checks
$latin1      = [System.Text.Encoding]::GetEncoding(28591)   # ISO-8859-1: 1:1 byte map

function Read-FileText([string]$path) {
    return $latin1.GetString([System.IO.File]::ReadAllBytes($path))
}
function Write-FileText([string]$path, [string]$text) {
    [System.IO.File]::WriteAllBytes($path, $latin1.GetBytes($text))
}

# ---- validate inputs -------------------------------------------------------
foreach ($f in @($machineType, $cSystem)) {
    if (-not (Test-Path -LiteralPath $f)) { throw "Required file not found: $f" }
}

Write-Host "=== HT160S installer build (source-only) ===" -ForegroundColor Cyan
Write-Host "  project  : $projectRoot"
Write-Host "  installer: $InstallerDir"

# ---- 1) assert the source is in a CLEAN dev state before we touch it -------
# Guards against capturing an already-patched file as the 'dev' backup
# (e.g. a previous run was interrupted before revert).
$mtText0 = Read-FileText $machineType
$csText0 = Read-FileText $cSystem

if ($mtText0.Contains($MARKER) -or $csText0.Contains($MARKER)) {
    throw ("Source already carries an $MARKER patch marker. A previous run may have " +
           "been interrupted. Restore MachineType.h / csystem.cpp to the development " +
           "state (git checkout or your own backup) before running this script again.")
}
if (-not [regex]::IsMatch($mtText0, "(?m)^[ \t]*#define[ \t]+SOFT_SIMULATE\b")) {
    throw "Active '#define SOFT_SIMULATE' not found in MachineType.h (already commented?). Aborting to avoid a wrong patch."
}

# ---- 2) backup the two files (byte-exact) ----------------------------------
if (-not (Test-Path -LiteralPath $backupDir)) {
    New-Item -ItemType Directory -Path $backupDir | Out-Null
}
Copy-Item -LiteralPath $machineType -Destination $mtBackup -Force
Copy-Item -LiteralPath $cSystem     -Destination $csBackup -Force
Write-Host "  backed up dev source -> $backupDir"

$patched = $false
try {
    # ---- 3) apply machine-profile patches ----------------------------------

    # 3a) MachineType.h : comment out #define SOFT_SIMULATE
    $mtPattern = "(?m)^(?<i>[ \t]*)#define[ \t]+SOFT_SIMULATE(?<rest>[ \t]*)(?=\r|\n|$)"
    $mtText = [regex]::Replace($mtText0, $mtPattern,
        '${i}//#define SOFT_SIMULATE${rest}   // ' + $MARKER + ' real-machine build (real hardware I/O)')
    Write-FileText $machineType $mtText

    # 3b) csystem.cpp : IsSafeDoorOpen() returns 0 unconditionally
    $csPattern = "(?<sig>int[ \t]+IsSafeDoorOpen[ \t]*\([ \t]*\)[ \t]*\r?\n[ \t]*\{)(?<nl>\r?\n)"
    if (-not [regex]::IsMatch($csText0, $csPattern)) {
        throw "Could not locate 'int IsSafeDoorOpen() {' in csystem.cpp -- pattern needs updating."
    }
    $csText = [regex]::Replace($csText0, $csPattern,
        '${sig}${nl}' + "`treturn 0;   // " + $MARKER + ' safety door not yet installed' + '${nl}', 1)
    Write-FileText $cSystem $csText
    $patched = $true

    # verify patches landed
    $mtChk = Read-FileText $machineType
    $csChk = Read-FileText $cSystem
    if (-not [regex]::IsMatch($mtChk, "(?m)^[ \t]*//[ \t]*#define[ \t]+SOFT_SIMULATE")) {
        throw "Verification failed: SOFT_SIMULATE was not commented out."
    }
    if (-not $csChk.Contains("$MARKER safety door")) {
        throw "Verification failed: IsSafeDoorOpen() bypass not inserted."
    }
    Write-Host "  [patched] MachineType.h : SOFT_SIMULATE commented out" -ForegroundColor Yellow
    Write-Host "  [patched] csystem.cpp   : IsSafeDoorOpen() returns 0"  -ForegroundColor Yellow

    # ---- 4) NSIS package (source folder only) ------------------------------
    # Call makensis DIRECTLY (not the installer's build.bat): build.bat now
    # delegates to THIS script, so calling it here would recurse forever.
    if ($SkipPackage) {
        Write-Host "  [skip] NSIS packaging (-SkipPackage)" -ForegroundColor DarkGray
    } else {
        if (-not (Test-Path -LiteralPath $nsiFile)) { throw "installer.nsi not found: $nsiFile" }
        $makensis = "C:\Program Files (x86)\NSIS\makensis.exe"
        if (-not (Test-Path -LiteralPath $makensis)) { $makensis = "C:\Program Files\NSIS\makensis.exe" }
        if (-not (Test-Path -LiteralPath $makensis)) { throw "makensis.exe not found -- install NSIS." }
        Write-Host "  running NSIS ($makensis) ..." -ForegroundColor Cyan
        # makensis resolves relative paths (OutFile, tools\7zr.exe) against its CWD,
        # so run it from the installer dir -- same as the original build.bat did.
        Push-Location $InstallerDir
        try { & $makensis "installer.nsi" }
        finally { Pop-Location }
        if ($LASTEXITCODE -ne 0) { throw "NSIS packaging failed (makensis exit $LASTEXITCODE)." }
    }
}
finally {
    # ---- 5) revert source back to dev state --------------------------------
    if ($patched -and -not $KeepPatched) {
        Copy-Item -LiteralPath $mtBackup -Destination $machineType -Force
        Copy-Item -LiteralPath $csBackup -Destination $cSystem     -Force
        Remove-Item -LiteralPath $backupDir -Recurse -Force
        Write-Host "  reverted source to development (SOFT_SIMULATE) state." -ForegroundColor Green
    }
    elseif ($KeepPatched) {
        Write-Host "  [-KeepPatched] source left in MACHINE mode. Restore with:" -ForegroundColor Yellow
        Write-Host "    Copy-Item '$mtBackup' '$machineType' -Force" -ForegroundColor Yellow
        Write-Host "    Copy-Item '$csBackup' '$cSystem' -Force"     -ForegroundColor Yellow
    }
}

# ---- 6) report -------------------------------------------------------------
if (-not $SkipPackage) {
    $pkg = Get-ChildItem -LiteralPath (Join-Path $InstallerDir "build") -Filter "*.exe" -ErrorAction SilentlyContinue |
           Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($pkg) { Write-Host "`n[OK] installer package: $($pkg.FullName)" -ForegroundColor Green }
}
