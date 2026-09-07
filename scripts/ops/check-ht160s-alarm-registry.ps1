# check-ht160s-alarm-registry.ps1
# Drift guard: flags alarm codes raised via ShowMyError / ShowMotorLimitError /
# ShowErrorMessage that are NOT registered in database.cpp CreateSystemAlarmCode().
#
# WHY: AlarmList.csv is a startup dump of mapAlarmCodeList, and (later slice) the SECS
# S5F5/S5F6 alarm list will iterate the same map. A free-string code raised at a call
# site but never registered is invisible in both -- silent catalog drift. This guard
# makes that drift visible (and, with -FailOnViolation, blocks the build).
#
# SCOPE: the hand-written MES/JAM/WAR code family raised through the free-string
# ShowMyError path. Structured generated families (CYL/SUC/SHT/MAG/COM/CCD/TNT/SYS via
# ShowCylinderError etc., and the numeric 4/5/6 device families) are a separate
# subsystem and are intentionally NOT checked here.
#
# Literal codes (e.g. "MES1022") must be present verbatim in database.cpp. sprintf
# families (e.g. "MES%d20") are matched by TEMPLATE -- the database.cpp registration
# must contain the same "MES%d20" format string (it expands the 11+Index loop). Codes
# built from pure runtime variables (no string literal) cannot be seen statically and
# are skipped.
#
# Big5-safe: reads source as Latin1 (1 byte = 1 char) so CP950 bytes are never decoded.

param(
    [string]$ProjectRoot = "",
    [switch]$FailOnViolation
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

$latin1 = [System.Text.Encoding]::GetEncoding(28591)
$dbPath = Join-Path $ProjectRoot "database.cpp"
if (-not (Test-Path -LiteralPath $dbPath)) {
    throw "database.cpp not found: $dbPath"
}

$dbText = $latin1.GetString([System.IO.File]::ReadAllBytes($dbPath))

$regLiteral = New-Object System.Collections.Generic.HashSet[string]
foreach ($m in [regex]::Matches($dbText, '"((?:MES|JAM|WAR)\d{3,5})"')) {
    [void]$regLiteral.Add($m.Groups[1].Value)
}
$regFamily = New-Object System.Collections.Generic.HashSet[string]
foreach ($m in [regex]::Matches($dbText, '"((?:MES|JAM|WAR)%d\d{2,3})"')) {
    [void]$regFamily.Add($m.Groups[1].Value)
}

$callRe = '(?:ShowMyError|ShowMotorLimitError|ShowErrorMessage)\s*\(\s*(?:AnsiString\(\)\.sprintf\(\s*)?"((?:MES|JAM|WAR)(?:%d)?\d{2,5})"'

$violations = New-Object System.Collections.ArrayList
$files = Get-ChildItem -LiteralPath $ProjectRoot -Recurse -File | Where-Object {
    $_.Extension -ieq ".cpp" -and $_.FullName -ne $dbPath
}

foreach ($f in $files) {
    $lines = ($latin1.GetString([System.IO.File]::ReadAllBytes($f.FullName))) -split "`n"
    for ($i = 0; $i -lt $lines.Count; $i++) {
        foreach ($m in [regex]::Matches($lines[$i], $callRe)) {
            $code = $m.Groups[1].Value
            if ($code -match '%d') {
                if (-not $regFamily.Contains($code)) {
                    [void]$violations.Add(("{0}:{1}  family {2} raised but not registered" -f $f.FullName, ($i + 1), $code))
                }
            }
            else {
                if (-not $regLiteral.Contains($code)) {
                    [void]$violations.Add(("{0}:{1}  code {2} raised but not registered" -f $f.FullName, ($i + 1), $code))
                }
            }
        }
    }
}

if ($violations.Count -gt 0) {
    Write-Host ("HT160S alarm-registry check: {0} unregistered alarm code(s):" -f $violations.Count) -ForegroundColor Yellow
    foreach ($v in $violations) {
        Write-Host ("  {0}" -f $v) -ForegroundColor Yellow
    }
    Write-Host "Register each in database.cpp CreateSystemAlarmCode() so AlarmList.csv (and the SECS list) stay complete." -ForegroundColor Yellow
    if ($FailOnViolation) {
        exit 1
    }
    Write-Host "(warn-only: pass -FailOnViolation to make this a hard build gate)" -ForegroundColor Yellow
    exit 0
}

Write-Output ("HT160S alarm-registry check passed: every raised MES/JAM/WAR code is registered ({0} literals, {1} families)." -f $regLiteral.Count, $regFamily.Count)
exit 0
