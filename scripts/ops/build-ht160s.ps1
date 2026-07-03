# build-ht160s.ps1
# Builds the HT160S BCB6 project and returns a non-zero exit code on failure.

param(
    [switch]$Clean,
    [switch]$Full,
    [string]$BCBRoot = "D:\ProgramFiles\Borland\CBuilder6"
)
# -Clean : delete a curated obj set, then build (fast; fine for source-only edits).
# -Full  : delete EVERY *.obj/*.d/*.tds under the project, then build. Use after a
#          shared-header STRUCT change (e.g. adding a member to TMyMotor) where the
#          curated -Clean list is insufficient -- every TU embedding the struct must
#          recompile. Implies the -Clean stale-output checks.

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$projectRoot = Join-Path $repoRoot "HT160S_Program_BCB_V1.0.0.0"
$projectFile = Join-Path $projectRoot "ht160s.bpr"
$makeFile = Join-Path $projectRoot "ht160s.mak"
$encodingCheck = Join-Path $PSScriptRoot "check-ht160s-source-encoding.ps1"
$formLint = Join-Path $PSScriptRoot "check-bcb-form-published.ps1"
$alarmCheck = Join-Path $PSScriptRoot "check-ht160s-alarm-registry.ps1"
$ladderCheck = Join-Path $PSScriptRoot "check-ladder-consistency.py"
$bpr2mak = Join-Path $BCBRoot "Bin\bpr2mak.exe"
$make = Join-Path $BCBRoot "Bin\make.exe"
$brcc32 = Join-Path $BCBRoot "Bin\brcc32.exe"

if (-not (Test-Path -LiteralPath $projectFile)) {
    throw "Project file not found: $projectFile"
}

if (-not (Test-Path -LiteralPath $bpr2mak)) {
    throw "bpr2mak.exe not found: $bpr2mak"
}

if (-not (Test-Path -LiteralPath $make)) {
    throw "make.exe not found: $make"
}

if (-not (Test-Path -LiteralPath $brcc32)) {
    throw "brcc32.exe not found: $brcc32"
}

if (-not (Test-Path -LiteralPath $encodingCheck)) {
    throw "Encoding check script not found: $encodingCheck"
}

if (-not (Test-Path -LiteralPath $formLint)) {
    throw "Form __published lint script not found: $formLint"
}

if (-not (Test-Path -LiteralPath $alarmCheck)) {
    throw "Alarm-registry check script not found: $alarmCheck"
}

$projectText = Get-Content -LiteralPath $projectFile -Raw
$projectMatch = [regex]::Match($projectText, '<PROJECT\s+value="([^"]+)"')
if (-not $projectMatch.Success) {
    throw "PROJECT output name not found in: $projectFile"
}
$projectExeName = $projectMatch.Groups[1].Value
$projectExePath = Join-Path $projectRoot $projectExeName
$staleOutputNames = @("ht160s.exe", "ht160s_bcb.exe")

function Test-OutputProcessRunning {
    param([string]$OutputPath)

    $targetPath = [System.IO.Path]::GetFullPath($OutputPath).ToLowerInvariant()
    $processName = [System.IO.Path]::GetFileNameWithoutExtension($OutputPath)
    $processes = @(Get-Process -Name $processName -ErrorAction SilentlyContinue)
    foreach ($process in $processes) {
        $processPath = ""
        try {
            $processPath = [System.IO.Path]::GetFullPath($process.MainModule.FileName).ToLowerInvariant()
        }
        catch {
            continue
        }

        if ($processPath -eq $targetPath -and -not $process.HasExited) {
            return $true
        }
    }

    return $false
}

function Assert-StaleOutputsNotRunning {
    foreach ($staleOutputName in $staleOutputNames) {
        if ($staleOutputName -ieq $projectExeName) {
            continue
        }

        $staleOutputPath = Join-Path $projectRoot $staleOutputName
        if (Test-OutputProcessRunning -OutputPath $staleOutputPath) {
            throw "Stale output $staleOutputName is still running from $staleOutputPath. Stop the old/debug process before building $projectExeName."
        }
    }
}

Push-Location $projectRoot
try {
    & $encodingCheck -ProjectRoot $projectRoot

    & $formLint -Path $projectRoot
    if ($LASTEXITCODE -ne 0) {
        throw "BCB form __published lint failed (fix the V1-V4 violations listed above)."
    }

    & $alarmCheck -ProjectRoot $projectRoot -FailOnViolation
    if ($LASTEXITCODE -ne 0) {
        throw "HT160S alarm-registry check failed (register the codes listed above in database.cpp CreateSystemAlarmCode)."
    }

    # AI(ht160s-ladder-guard) 20260703 : static "number but no action" gate. Fails the build
    # on a switch(Task) state cursor jumping to a value with no matching case AND no default:
    # (a silent dead-jump). python-based; if python is absent (some on-machine build boxes)
    # the gate is skipped with a warning so it never blocks a build that could otherwise run.
    $python = (Get-Command python -ErrorAction SilentlyContinue)
    if ($null -eq $python) { $python = (Get-Command py -ErrorAction SilentlyContinue) }
    if ($null -ne $python) {
        & $python.Source $ladderCheck
        if ($LASTEXITCODE -ne 0) {
            throw "HT160S ladder-consistency check failed (a state number with no matching case + no default; fix or add a default: LogLadderFault guard)."
        }
    } else {
        Write-Warning "python not found - skipping ladder-consistency gate (run scripts/ops/check-ladder-consistency.py manually)."
    }

    $objRoot = Join-Path $repoRoot "Obj"
    if (-not (Test-Path -LiteralPath $objRoot)) {
        New-Item -ItemType Directory -Path $objRoot | Out-Null
    }

    if ($Clean -or $Full) {
        Assert-StaleOutputsNotRunning

        if ($Full) {
            $fullObjs = Get-ChildItem -LiteralPath $projectRoot -Recurse -File -ErrorAction SilentlyContinue |
                Where-Object { @(".obj", ".d", ".tds") -contains $_.Extension.ToLowerInvariant() }
            if ($fullObjs) {
                Remove-Item -LiteralPath ($fullObjs | ForEach-Object { $_.FullName }) -ErrorAction SilentlyContinue
                Write-Output ("Full clean: removed {0} obj/d/tds file(s)." -f $fullObjs.Count)
            }
        }

        $cleanFiles = @(
            "ht160s.obj", "main.obj", "database.obj", "uruncontrol.obj",
            "HTray.obj", "HTray.d", "HTMotor.obj", "MyMotor.obj", "mySMCmotor.obj", "myMN200motor.obj",
            "myMC88X1motor.obj", "AutomationServer.obj", "MCUDisplayProtocol.obj", "MCUDisplay.obj", $projectExeName,
            ([System.IO.Path]::ChangeExtension($projectExeName, ".tds")),
            "..\Obj\ht160s.csm"
        )
        foreach ($staleOutputName in $staleOutputNames) {
            $cleanFiles += $staleOutputName
            $cleanFiles += [System.IO.Path]::ChangeExtension($staleOutputName, ".tds")
        }
        Remove-Item -LiteralPath $cleanFiles -ErrorAction SilentlyContinue
    }

    $rcFile = Join-Path $projectRoot "ht160s.rc"
    if (Test-Path -LiteralPath $rcFile) {
        & $brcc32 -fo"ht160s.res" "ht160s.rc"
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }
    }

    & $bpr2mak "ht160s.bpr"
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    if (-not (Test-Path -LiteralPath $makeFile)) {
        throw "Makefile was not generated: $makeFile"
    }

    & $make -f "ht160s.mak"
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    if (-not (Test-Path -LiteralPath $projectExeName)) {
        throw "Build completed but $projectExeName was not generated."
    }

    foreach ($staleOutputName in $staleOutputNames) {
        if ($staleOutputName -ieq $projectExeName) {
            continue
        }

        $staleOutputPath = Join-Path $projectRoot $staleOutputName
        Remove-Item -LiteralPath $staleOutputName, ([System.IO.Path]::ChangeExtension($staleOutputName, ".tds")) -ErrorAction SilentlyContinue
        if ((Test-Path -LiteralPath $staleOutputName) -and (Test-OutputProcessRunning -OutputPath $staleOutputPath)) {
            throw "Stale output $staleOutputName is still running from $staleOutputPath. Stop the old/debug process before building $projectExeName."
        }
        elseif (Test-Path -LiteralPath $staleOutputName) {
            Write-Warning "Stale output $staleOutputName still exists but is not running from this project path. Verify IDE Debug Run targets $projectExeName."
        }
    }
}
finally {
    Pop-Location
}
