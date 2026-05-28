# build-ht160s.ps1
# Builds the HT160S BCB6 project and returns a non-zero exit code on failure.

param(
    [switch]$Clean,
    [string]$BCBRoot = "D:\ProgramFiles\Borland\CBuilder6"
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$projectRoot = Join-Path $repoRoot "HT160S_Program_BCB_V1.0.0.0"
$projectFile = Join-Path $projectRoot "ht160s.bpr"
$makeFile = Join-Path $projectRoot "ht160s.mak"
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
    $objRoot = Join-Path $repoRoot "Obj"
    if (-not (Test-Path -LiteralPath $objRoot)) {
        New-Item -ItemType Directory -Path $objRoot | Out-Null
    }

    if ($Clean) {
        Assert-StaleOutputsNotRunning

        $cleanFiles = @(
            "ht160s.obj", "main.obj", "database.obj", "uruncontrol.obj",
            "HTray.obj", "HTray.d", "HTMotor.obj", "MyMotor.obj", "mySMCmotor.obj", "myMN200motor.obj",
            "myMC88X1motor.obj", "AutomationServer.obj", $projectExeName,
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
