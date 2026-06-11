# check-ht160s-source-encoding.ps1
# Fails when HT160S BCB6 source files contain UTF-8 replacement bytes or BOMs.

param(
    [string]$ProjectRoot = ""
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

if (-not $ProjectRoot) {
    $repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
    $ProjectRoot = Join-Path $repoRoot "HT160S_Program_BCB_V1.0.0.0"
}

if (-not (Test-Path -LiteralPath $ProjectRoot)) {
    throw "ProjectRoot not found: $ProjectRoot"
}

$extensions = @(".cpp", ".h", ".c", ".dfm", ".rc", ".asm")
$badFiles = New-Object System.Collections.ArrayList

function Add-EncodingIssue {
    param(
        [string]$Path,
        [string]$Issue
    )

    [void]$badFiles.Add(("{0}: {1}" -f $Path, $Issue))
}

function Count-ReplacementBytes {
    param([byte[]]$Bytes)

    $count = 0
    for ($i = 0; $i -le $Bytes.Length - 3; $i++) {
        if ($Bytes[$i] -eq 0xEF -and $Bytes[$i + 1] -eq 0xBF -and $Bytes[$i + 2] -eq 0xBD) {
            $count++
        }
    }
    return $count
}

function Test-StartsWithUtf8Bom {
    param([byte[]]$Bytes)

    return ($Bytes.Length -ge 3 -and $Bytes[0] -eq 0xEF -and $Bytes[1] -eq 0xBB -and $Bytes[2] -eq 0xBF)
}

$files = Get-ChildItem -LiteralPath $ProjectRoot -Recurse -File | Where-Object {
    $extensions -contains $_.Extension.ToLowerInvariant()
}

foreach ($file in $files) {
    $bytes = [System.IO.File]::ReadAllBytes($file.FullName)
    $replacementCount = Count-ReplacementBytes -Bytes $bytes
    if ($replacementCount -gt 0) {
        Add-EncodingIssue -Path $file.FullName -Issue ("contains EF BF BD replacement bytes: {0}" -f $replacementCount)
    }

    if (Test-StartsWithUtf8Bom -Bytes $bytes) {
        Add-EncodingIssue -Path $file.FullName -Issue "contains UTF-8 BOM; BCB6 source must stay CP950/Big5 or ASCII"
    }
}

if ($badFiles.Count -gt 0) {
    $message = "HT160S source encoding check failed.`r`n" + ($badFiles -join "`r`n") + "`r`nUse CP950-safe byte editing for legacy source and prefer ASCII English comments."
    throw $message
}

Write-Output ("HT160S source encoding check passed: {0} files" -f $files.Count)