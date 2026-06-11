<#
.SYNOPSIS
    HT160S State Record (Store Hangup) analyzer.

.DESCRIPTION
    Parses a HT160S_StateRecord snapshot folder produced by cStateRecordHT160
    (the "Store Hangup" button) and produces a hang/deadlock diagnosis:
      - Decodes every module's current Task number into a human-readable phase.
      - Classifies each module as FROZEN (stuck at one Task) or CHURNING
        (idle spin: many Task changes in a few ms).
      - Applies the SystemStart=0 caveat: when SystemStart is false, DoAllProcess
        breaks the whole action loop, so TaskHistory is frozen at the last
        *running* moment - the snapshot timestamp is NOT when modules last ran.
      - Flags the known SortArm-place / Auto-discharge threshold-mismatch
        deadlock and other cross-module stall patterns.

    Read-only. Never writes into the snapshot folder. Optional -OutFile writes a
    UTF-8 report next to wherever you choose.

.PARAMETER Path
    Path to a single state-record folder (the one that contains
    MachineState.ini / TaskHistory.csv / CurrentTasks.txt / Snapshot.ini).
    If omitted, the newest folder under -Root is analyzed.

.PARAMETER Root
    Root that holds dated snapshot folders. Default D:\HT160S_StateRecord.

.PARAMETER FreezeSeconds
    A module whose last Task change is older than this many seconds before the
    newest change seen across all modules is reported as FROZEN. Default 5.

.PARAMETER OutFile
    Optional path to write the text report (UTF-8). Console always prints it.

.EXAMPLE
    powershell -ExecutionPolicy Bypass -File .\scripts\ops\analyze-state-record.ps1 `
        -Path "D:\HT160S_StateRecord\2026-06-09 10_41_28"

.EXAMPLE
    # Analyze the newest snapshot under the default root
    powershell -ExecutionPolicy Bypass -File .\scripts\ops\analyze-state-record.ps1
#>
[CmdletBinding()]
param(
    [string]$Path,
    [string]$Root = 'D:\HT160S_StateRecord',
    [double]$FreezeSeconds = 5.0,
    [string]$OutFile
)

$ErrorActionPreference = 'Stop'

# ---------------------------------------------------------------------------
# Per-module Task -> phase decoder. Mirrors the switch(Task) dispatch in each
# aXxx.cpp DoXxx() as of HT160S_Program_BCB_V1.0.0.0 (non-FSM procedural).
# Keep in sync with the source if Task numbers change.
# ---------------------------------------------------------------------------
$TaskMaps = @{
    'Empty' = @{
        1='idle->10'; 10='->100'; 100='idle dispatch (GoUp/GoDown/Feed decide)';
        1000='DoGoDownTray (separate front tray)'; 2000='DoFeedTray (front->rear pickup)';
        3000='DoGoUpTray (return/lot-finish stack up)'; 7000='GoDown inner (front confirm)'
    }
    'Loader1' = @{
        1='idle->10'; 10='->100 (CleanOut drain check)'; 100='feed-or-CCD decide';
        1000='DoFeedTray (front->Y car)'; 2000='DoCcdCheck (Top CCD 2D scan)';
        3000='post-CCD: LS_READY_SORT wait SortArm pick / discharge gate';
        4000='DoDischargeTray (empty tray -> rear)'
    }
    'Loader2' = @{
        1='idle->10'; 10='->100 (CleanOut drain check)'; 100='feed-or-CCD decide';
        1000='DoFeedTray (front->Y car)'; 2000='DoCcdCheck (Top CCD 2D scan)';
        3000='post-CCD: LS_READY_SORT wait SortArm pick / discharge gate';
        4000='DoDischargeTray (empty tray -> rear)'
    }
    'Auto1' = @{
        1='idle->100'; 100='CheckAutoTray ->1000'; 1000='FindFeedAuto decide (feed/none)';
        2000='DoFeedTray (rear tray -> car)'; 3000='CheckAutoTray + FindDischargeAuto decide';
        4000='DoDischargeTray (full tray out)'; 5000='DoAllAutoCleanOut'
    }
    'TrayArm' = @{
        1='idle->10'; 10='->100'; 100='idle: HasTray + DecideJob';
        1000='DoPick (source rear)'; 2000='DoPlace (-> Auto / EmptyTray)'
    }
    'SortArm' = @{
        1='idle: pick decide (GetSortingLoaderNo)'; 100='DoPickFromLoader';
        200='DoPlaceToAuto (SelectPlaceAuto -> place held IC)'
    }
    'Color' = @{
        1='idle->10'; 10='RefreshState + SortBin dispatch'; 100='supply dispatch (GoDown/Supply/Release)';
        1000='DoSupplyTray (front->output + 2D read)'; 1200='DoGoDownTray (separate front tray)';
        1500='DoReleaseTray (release picked tray)'; 2000='DoSortBin'
    }
}

# Phases that mean "actively waiting on a cross-module handshake" (a freeze here
# is a real stall, not benign idle).
$StallPhase = @{
    'Loader1' = @(3000, 2000)
    'Loader2' = @(3000, 2000)
    'SortArm' = @(100, 200)
    'TrayArm' = @(1000, 2000)
    'Auto1'   = @(2000, 4000)
    'Empty'   = @(1000, 2000, 3000)
    'Color'   = @(1000, 1200, 1500)
}

function Resolve-SnapshotFolder {
    param([string]$Path, [string]$Root)
    if ($Path) {
        if (-not (Test-Path -LiteralPath $Path)) { throw "Path not found: $Path" }
        return (Get-Item -LiteralPath $Path).FullName
    }
    if (-not (Test-Path -LiteralPath $Root)) { throw "Root not found: $Root" }
    $newest = Get-ChildItem -LiteralPath $Root -Directory |
        Where-Object { Test-Path (Join-Path $_.FullName 'MachineState.ini') } |
        Sort-Object Name -Descending | Select-Object -First 1
    if (-not $newest) { throw "No snapshot folder with MachineState.ini under $Root" }
    return $newest.FullName
}

function Read-IniValue {
    param([string]$File, [string]$Section, [string]$Key)
    if (-not (Test-Path -LiteralPath $File)) { return $null }
    $cur = ''
    foreach ($line in Get-Content -LiteralPath $File) {
        $t = $line.Trim()
        if ($t -match '^\[(.+)\]$') { $cur = $Matches[1]; continue }
        if ($cur -eq $Section -and $t -match '^([^=]+)=(.*)$') {
            if ($Matches[1].Trim() -eq $Key) { return $Matches[2].Trim() }
        }
    }
    return $null
}

function Get-TaskPhase {
    param([string]$Module, [int]$Task)
    $map = $TaskMaps[$Module]
    if ($map -and $map.ContainsKey($Task)) { return $map[$Task] }
    return "(unknown task $Task)"
}

function Parse-TimeOfDay {
    param([string]$s)
    # Format hh:nn:ss.zzz -> TimeSpan since midnight.
    if ($s -match '^(\d{1,2}):(\d{2}):(\d{2})\.(\d{1,3})$') {
        return [TimeSpan]::FromMilliseconds(
            ([int]$Matches[1])*3600000 + ([int]$Matches[2])*60000 +
            ([int]$Matches[3])*1000 + [int]$Matches[4])
    }
    return $null
}

# ---------------------------------------------------------------------------
$folder = Resolve-SnapshotFolder -Path $Path -Root $Root
$stateIni = Join-Path $folder 'MachineState.ini'
$histCsv  = Join-Path $folder 'TaskHistory.csv'
$snapIni  = Join-Path $folder 'Snapshot.ini'

$sb = New-Object System.Text.StringBuilder
function Emit { param([string]$s='') [void]$sb.AppendLine($s) }

Emit '============================================================'
Emit ' HT160S State Record Analysis'
Emit '============================================================'
Emit "Folder      : $folder"

$runMode   = Read-IniValue $stateIni 'System' 'RunModeName'
$runModeN  = Read-IniValue $stateIni 'System' 'RunMode'
$sysStart  = Read-IniValue $stateIni 'System' 'SystemStart'
$cleanOut  = Read-IniValue $stateIni 'System' 'bCleanOut'
$recipe    = Read-IniValue $stateIni 'Recipe' 'Name'
$lot       = Read-IniValue $stateIni 'Lot' 'LotNo'
$trigger   = Read-IniValue $snapIni  'Snapshot' 'TriggerReason'
$snapTime  = Read-IniValue $snapIni  'Snapshot' 'Time'

Emit "Snapshot    : $snapTime  (trigger=$trigger)"
Emit "RunMode     : $runMode ($runModeN)   SystemStart=$sysStart   bCleanOut=$cleanOut"
Emit "Recipe      : $recipe    Lot: $lot"
Emit ''

# --- Parse TaskHistory.csv into per-module transition lists -----------------
if (-not (Test-Path -LiteralPath $histCsv)) { throw "Missing TaskHistory.csv in $folder" }
$rows = Import-Csv -LiteralPath $histCsv
$modules = @()
$globalNewest = [TimeSpan]::Zero

foreach ($r in $rows) {
    $name = $r.Module
    $trans = @()
    for ($k = 0; $k -lt 30; $k++) {
        $tk = $r.("Time_$k"); $vk = $r.("Task_$k")
        if ([string]::IsNullOrWhiteSpace($tk) -or [string]::IsNullOrWhiteSpace($vk)) { continue }
        $ts = Parse-TimeOfDay $tk
        if ($null -eq $ts) { continue }
        $trans += [pscustomobject]@{ Time = $ts; TimeStr = $tk; Task = [int]$vk }
    }
    if ($trans.Count -eq 0) { continue }
    # Time_0 is newest.
    $newest = $trans[0]
    if ($newest.Time -gt $globalNewest) { $globalNewest = $newest.Time }
    $modules += [pscustomobject]@{
        Name = $name; Trans = $trans; Newest = $newest
        SpanMs = [int]($newest.Time - $trans[$trans.Count-1].Time).TotalMilliseconds
        Count = $trans.Count
    }
}

# --- Classify ---------------------------------------------------------------
Emit '------------------------------------------------------------'
Emit ' Per-module state'
Emit '------------------------------------------------------------'
$frozen = @()
$churn  = @()
foreach ($m in $modules) {
    $ageSec = [math]::Round(($globalNewest - $m.Newest.Time).TotalSeconds, 3)
    $phase  = Get-TaskPhase $m.Name $m.Newest.Task
    # CHURNING: >= 10 transitions packed inside < 200 ms.
    $isChurn = ($m.Count -ge 10 -and $m.SpanMs -ge 0 -and $m.SpanMs -lt 200)
    # FROZEN: last change is older than FreezeSeconds vs the globally-newest change.
    $isFrozen = ($ageSec -ge $FreezeSeconds)

    $tag = 'active'
    if ($isChurn)  { $tag = 'CHURN (idle spin)'; $churn += $m }
    if ($isFrozen) { $tag = "FROZEN ${ageSec}s"; $frozen += $m }

    Emit ("{0,-9} Task={1,-5} {2,-50} [{3}]" -f $m.Name, $m.Newest.Task, $phase, $tag)
    Emit ("          last change {0}   ({1} samples over {2} ms)" -f $m.Newest.TimeStr, $m.Count, $m.SpanMs)
}
Emit ''

# --- SystemStart caveat -----------------------------------------------------
Emit '------------------------------------------------------------'
Emit ' Interpretation'
Emit '------------------------------------------------------------'
if ($sysStart -eq '0') {
    Emit 'SystemStart=0 at capture. DoAllProcess() breaks the whole module loop'
    Emit 'when SystemStart==false (database.cpp), so NO module runs at capture time.'
    Emit 'TaskHistory therefore freezes at the LAST RUNNING moment - the newest'
    Emit 'change timestamp is when modules last executed, NOT the snapshot time.'
    Emit ''
}

# --- Deadlock heuristics ----------------------------------------------------
Emit '------------------------------------------------------------'
Emit ' Stall / deadlock signals'
Emit '------------------------------------------------------------'
$signal = $false

# A real stall = modules FROZEN on a cross-module handshake phase, while others churn idle.
$frozenStall = @()
foreach ($m in $frozen) {
    $sp = $StallPhase[$m.Name]
    if ($sp -and ($sp -contains $m.Newest.Task)) { $frozenStall += $m }
}

if ($frozenStall.Count -gt 0) {
    $signal = $true
    Emit 'Modules frozen on a cross-module handshake phase (genuine stall):'
    foreach ($m in $frozenStall) {
        Emit ("  - {0} stuck at Task={1}: {2}" -f $m.Name, $m.Newest.Task, (Get-TaskPhase $m.Name $m.Newest.Task))
    }
    Emit ''
}

# Known signature: SortArm frozen at 200 (DoPlaceToAuto) + Auto* churning idle.
$sa = $modules | Where-Object { $_.Name -eq 'SortArm' } | Select-Object -First 1
$autoChurn = $churn | Where-Object { $_.Name -like 'Auto*' }
if ($sa -and $sa.Newest.Task -eq 200 -and ($frozen | Where-Object {$_.Name -eq 'SortArm'}) -and $autoChurn) {
    $signal = $true
    Emit '*** SIGNATURE: SortArm-place / Auto-discharge THRESHOLD-MISMATCH deadlock ***'
    Emit '  SortArm is FROZEN at Task=200 (DoPlaceToAuto). PlaceTask sits at 1 because'
    Emit '  SelectPlaceAuto() returns false: no Auto working tray exposes a contiguous'
    Emit '  EMPTY_IC run that fits the held 4-sucker pattern.'
    Emit '  Meanwhile Auto* idle-churn: FindFeedAuto<0 (car already has a tray) and'
    Emit '  FindDischargeAuto<0 because bFullIC requires FullThisIC(HAS_OK_IC) = EVERY'
    Emit '  cell a good IC. A tray with fragmented / wrong-bin / partial empties is'
    Emit '  neither placeable (SortArm) nor dischargeable (Auto) -> the line wedges.'
    Emit '  Downstream: GetTrayRequest=None (car busy) -> TrayArm DecideJob=NONE (idle),'
    Emit '  Empty holds a ready rear tray nobody picks, Loader1 LS_READY_SORT waits for'
    Emit '  SortArm to pick the rest of its IC.'
    Emit ''
    Emit '  Candidate fixes (NOT auto-applied - motion handshake = safety-critical):'
    Emit '    A) Auto discharge when the working tray can no longer accept the pending'
    Emit '       held-IC pattern (not only when 100% full of HAS_OK_IC).'
    Emit '    B) SortArm single-sucker fallback place into any remaining EMPTY_IC cell'
    Emit '       when the multi-sucker contiguous run fails.'
    Emit '    C) Watchdog: SelectPlaceAuto false for N s -> force target Auto discharge.'
    Emit ''
}

# Known signature: Color idle-churning at the dispatch head (Task 1/10/100) and
# never advancing to a supply phase (1000/1200). This is the fingerprint of the
# bTrayReady "latch" bug: RefreshStateFromSensors used to set bTrayReady=true from
# any output-sensor read and never cleared it, so DoColor case 100 folded straight
# back to idle (and IsTrayReady() stayed stuck true for TrayArm). Color supplies the
# AMR identity tray, so a wedged Color also starves the TrayArm identity-tray job.
$colorChurn = $churn | Where-Object { $_.Name -eq 'Color' -and ($_.Newest.Task -in 1,10,100) }
if ($colorChurn) {
    $signal = $true
    Emit '*** SIGNATURE: Color supply never starts (idle-churn at Task 1/10/100) ***'
    Emit '  Color is spinning at its dispatch head and never reaches a supply phase'
    Emit '  (1000 DoSupplyTray / 1200 DoGoDownTray). Suspect a stuck-true bTrayReady:'
    Emit '  if DoColor case 100 sees bTrayReady it returns to idle without supplying,'
    Emit '  and IsTrayReady() reports a tray that is not actually presented.'
    Emit ''
    Emit '  Check (aColor.cpp):'
    Emit '    1) RefreshStateFromSensors must NOT latch bTrayReady=true from a sensor.'
    Emit '       bTrayReady is owned by DoSupplyTray (case 900 set) + DoReleaseTray (clear).'
    Emit '    2) Confirm an AMR identity-tray pull actually calls RequestSupplyTray().'
    Emit '    3) Mirror Empty: ready follows the supply ladder just-in-time, not a latch.'
    Emit ''
}

if (-not $signal) {
    Emit 'No known deadlock signature matched. If the machine was perceived hung,'
    Emit 'inspect the FROZEN modules above and trace their wait condition in source.'
    Emit ''
}

Emit '============================================================'

$report = $sb.ToString()
Write-Output $report

if ($OutFile) {
    $dir = Split-Path -Parent $OutFile
    if ($dir -and -not (Test-Path -LiteralPath $dir)) { New-Item -ItemType Directory -Force -Path $dir | Out-Null }
    [System.IO.File]::WriteAllText($OutFile, $report, (New-Object System.Text.UTF8Encoding $false))
    Write-Host "Report written: $OutFile"
}
