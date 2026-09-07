<#
.SYNOPSIS
  Apply the HT172 weekly-case-flow bundle into D:\HT172\.claude\.

.DESCRIPTION
  Copies the prepared HT172-flavored agent + weekly-* commands + weekly-case-flow
  skill from this bundle (docs\plan\ht172-weekly-sync\claude\) into the HT172
  project's .claude\ directory. ADD-ONLY: it never touches HT172's existing
  settings.json or hooks\guard-big5.ps1.

  Run this in the HT172 context (or anywhere; it targets -Ht172Root). It is the
  USER's action that writes into HT172 - the HT160S_BCB session cannot and does
  not write there itself.

.PARAMETER Ht172Root
  HT172 project root. Default: D:\HT172

.PARAMETER DryRun
  Show what would be copied without writing.

.EXAMPLE
  powershell -NoProfile -ExecutionPolicy Bypass -File .\apply-ht172-weekly.ps1 -DryRun
  powershell -NoProfile -ExecutionPolicy Bypass -File .\apply-ht172-weekly.ps1
#>
param(
    [string]$Ht172Root = 'D:\HT172',
    [switch]$DryRun
)

$ErrorActionPreference = 'Stop'
$srcClaude = Join-Path $PSScriptRoot 'claude'
$dstClaude = Join-Path $Ht172Root '.claude'

if (-not (Test-Path $srcClaude)) { throw "Source bundle not found: $srcClaude" }
if (-not (Test-Path $Ht172Root)) { throw "HT172 root not found: $Ht172Root" }

# Files to apply, relative to the 'claude' dir.
$files = @(
    'agents\weekly-report.md',
    'commands\weekly-help.md',
    'commands\update-weekly.md',
    'commands\weekly-status.md',
    'commands\weekly-case-intake.md',
    'commands\weekly-case-integrity.md',
    'commands\weekly-next-week.md',
    'skills\weekly-case-flow\SKILL.md'
)

Write-Host "== HT172 weekly-case-flow apply ==" -ForegroundColor Cyan
Write-Host "  source: $srcClaude"
Write-Host "  target: $dstClaude"
if ($DryRun) { Write-Host "  MODE:   DRY-RUN (no files written)" -ForegroundColor Yellow }
Write-Host ""

$copied = 0
$overwritten = 0
foreach ($rel in $files) {
    $src = Join-Path $srcClaude $rel
    $dst = Join-Path $dstClaude $rel
    if (-not (Test-Path $src)) { Write-Host "  [MISS] source missing: $rel" -ForegroundColor Red; continue }

    $exists = Test-Path $dst
    $tag = if ($exists) { 'OVERWRITE' } else { 'NEW' }
    Write-Host ("  [{0,-9}] {1}" -f $tag, $rel)

    if (-not $DryRun) {
        $dstDir = Split-Path $dst -Parent
        if (-not (Test-Path $dstDir)) { New-Item -ItemType Directory -Force -Path $dstDir | Out-Null }
        Copy-Item -LiteralPath $src -Destination $dst -Force
        if ($exists) { $overwritten++ } else { $copied++ }
    }
}

Write-Host ""
if ($DryRun) {
    Write-Host "Dry-run complete. Re-run without -DryRun to apply." -ForegroundColor Yellow
} else {
    Write-Host ("Applied. new=$copied overwritten=$overwritten") -ForegroundColor Green
    Write-Host "NOTE: restart the HT172 Claude Code session so the new agent/commands load." -ForegroundColor Yellow
    Write-Host "      HT172 settings.json / guard-big5.ps1 were NOT modified (not needed)." -ForegroundColor DarkGray
}
