# align-monitor-view.ps1
# Re-aligns the HT160S "Motion View" (PanelMain6) column components in main.dfm
# to a uniform grid after manual designer drag has knocked them out of line.
#
# Layout model (display-only panels; zero machine-control impact):
#   Columns: Empty, Load, Auto1..Auto6, Color  (index 0..8)
#   TrayLeft = 137 + index * 150
#   ChannelLeft  = TrayLeft - 8   (left green bar)
#   ChannelRight = TrayLeft + 86  (right green bar)
#   Channel Top  = 52, Height 700
#   Tray Top     = 73 / 243 / 595 ; Load adds 432 (4th cell)
#   Label Top    = 775, Label Left = TrayLeft (column-centred)
#
# Excluded (driven at runtime / full-width rails): plTrayArm, palSortArm1,
#   plTrayArmName, plSortArmName, plCCDMotor, pnlLed.
#
# Big5 safety: edits main.dfm in binary mode (ASCII-only Left/Top tokens) so
# CP950 Chinese is never touched.

param(
    [string]$ProjectRoot = ""
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

if (-not $ProjectRoot) {
    $repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
    $ProjectRoot = Join-Path $repoRoot "HT160S_Program_BCB_V1.0.0.0"
}

$dfm = Join-Path $ProjectRoot "main.dfm"
if (-not (Test-Path -LiteralPath $dfm)) {
    throw "main.dfm not found: $dfm"
}

$python = Get-Command python -ErrorAction SilentlyContinue
if (-not $python) {
    throw "python not found on PATH (required for Big5-safe binary edit)."
}

$py = @'
# -*- coding: utf-8 -*-
import sys, re
dfm = sys.argv[1]

cols = ["plEmpty","plLoader","plAuto1","plAuto2","plAuto3",
        "plAuto4","plAuto5","plAuto6","plColor"]

items = []
for idx, c in enumerate(cols):
    tray = 137 + idx * 150
    items.append((c + "ChLeft",  tray - 8, 52))
    items.append((c + "ChRight", tray + 86, 52))
    items.append((c + "Tray1",   tray, 73))
    items.append((c + "Tray2",   tray, 243))
    if c == "plLoader":
        items.append((c + "Tray3", tray, 432))
        items.append((c + "Car",   tray, 595))
    elif c == "plEmpty":
        items.append((c + "Car",   tray, 595))
    else:
        items.append((c + "Tray3", tray, 595))
    items.append((c + "Label",   tray, 775))

with open(dfm, "rb") as f:
    data = f.read()

changes = 0
for name, L, T in items:
    hdr = re.compile(rb"object %s: T\w+\r?\n" % re.escape(name.encode("ascii")))
    hm = list(hdr.finditer(data))
    if len(hm) != 1:
        print("ABORT header %s count=%d" % (name, len(hm))); sys.exit(1)
    start = hm[0].end()
    mL = re.compile(rb"Left = (\d+)").search(data, start)
    mT = re.compile(rb"Top = (\d+)").search(data, start)
    if not mL or not mT:
        print("ABORT no Left/Top for %s" % name); sys.exit(1)
    oldL = int(mL.group(1)); oldT = int(mT.group(1))
    repls = [(mL.start(), mL.end(), b"Left = %d" % L),
             (mT.start(), mT.end(), b"Top = %d" % T)]
    for s, e, rep in sorted(repls, key=lambda x: -x[0]):
        data = data[:s] + rep + data[e:]
    if oldL != L or oldT != T:
        changes += 1
        print("  %-16s L %d->%d  T %d->%d" % (name, oldL, L, oldT, T))

with open(dfm, "wb") as f:
    f.write(data)
print("OK aligned %d objects, %d changed" % (len(items), changes))
'@

$tmp = Join-Path $env:TEMP "align-monitor-view.py"
Set-Content -LiteralPath $tmp -Value $py -Encoding ascii
try {
    & python $tmp $dfm
    if ($LASTEXITCODE -ne 0) {
        throw "alignment script failed (exit $LASTEXITCODE)"
    }
}
finally {
    Remove-Item -LiteralPath $tmp -ErrorAction SilentlyContinue
}
