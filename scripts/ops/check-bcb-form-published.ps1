<#
.SYNOPSIS
  Lint BCB6 VCL form headers (*.h) so the form designer never rejects event edits.

.DESCRIPTION
  The C++Builder 6 form designer streams the __published section to map components
  and events. It raises "Incorrect method declaration in class TXxx" (on an event
  double-click) whenever __published is mis-shaped, and the VCL streamer raises an
  EReadError "Invalid property value" at form construction when a DFM-wired event
  handler is not __published. This linter catches the three recurring shapes:

    V1 (EReadError)  : a handler referenced by the .dfm (OnXxx = Name) is declared
                       OUTSIDE __published (private/public). Must be __published.
    V2 (designer)    : a component field (Type *name;) is declared AFTER an event
                       method inside __published. ALL fields must precede ALL methods.
    V3 (designer)    : a comment line lives inside the __published section. Move it
                       above the class (the out-of-class notes-block convention).
    V4 (link error)  : a DFM-wired handler has NO declaration anywhere in the class.

  Classification is authoritative: a __fastcall method is a "DFM-wired handler"
  (category B, must be __published, grouped last) ONLY if its name appears as an
  event value in the matching .dfm. Handlers wired in code (e.g. in the ctor) are
  category C and may stay in private:/public:. Component fields (Type *name;) are
  category A and must lead __published.

  Read-only. Reports violations and exits non-zero if any are found, so it can gate
  builds next to check-ht160s-source-encoding.ps1. Bytes are read as Latin1 so legacy
  Big5 content is never decoded/mangled (only ASCII structure is parsed).

.PARAMETER Path
  One or more .h files or directories to scan. Default: the BCB program directory.

.EXAMPLE
  pwsh scripts/ops/check-bcb-form-published.ps1
  pwsh scripts/ops/check-bcb-form-published.ps1 -Path HT160S_Program_BCB_V1.0.0.0/main.h
#>
param(
    [string[]]$Path
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$progDir  = Join-Path $repoRoot 'HT160S_Program_BCB_V1.0.0.0'
if (-not $Path -or $Path.Count -eq 0) { $Path = @($progDir) }

# ---- collect target .h files ------------------------------------------------
$headers = New-Object System.Collections.Generic.List[string]
foreach ($p in $Path) {
    $full = if ([System.IO.Path]::IsPathRooted($p)) { $p } else { Join-Path (Get-Location) $p }
    if (Test-Path $full -PathType Container) {
        Get-ChildItem -Path $full -Filter *.h -File | ForEach-Object { $headers.Add($_.FullName) }
    } elseif (Test-Path $full -PathType Leaf) {
        $headers.Add((Resolve-Path $full).Path)
    } else {
        Write-Warning "Path not found: $p"
    }
}

function Read-LatinLines([string]$file) {
    $bytes = [System.IO.File]::ReadAllBytes($file)
    $text  = [System.Text.Encoding]::GetEncoding(28591).GetString($bytes)  # Latin1, byte-exact
    return ($text -replace "`r`n", "`n") -split "`n"
}

# Parse a .dfm for event assignments: "OnClick = sbExitClick" -> name set
function Get-DfmWiredHandlers([string]$dfm) {
    $set = New-Object System.Collections.Generic.HashSet[string]
    if (-not (Test-Path $dfm)) { return $set }
    foreach ($line in (Read-LatinLines $dfm)) {
        $m = [regex]::Match($line, '^\s*On[A-Za-z0-9]+\s*=\s*([A-Za-z_]\w*)\s*$')
        if ($m.Success) { [void]$set.Add($m.Groups[1].Value) }
    }
    return $set
}

$totalViolations = 0
$filesWithIssues = 0

foreach ($h in $headers) {
    $lines = Read-LatinLines $h
    $dfm   = [System.IO.Path]::ChangeExtension($h, '.dfm')
    $wired = Get-DfmWiredHandlers $dfm
    $rel   = if ($h.StartsWith($repoRoot, [StringComparison]::OrdinalIgnoreCase)) { $h.Substring($repoRoot.Length).TrimStart('\','/') } else { $h }

    $fileViol = New-Object System.Collections.Generic.List[string]

    $n = $lines.Count
    $i = 0
    while ($i -lt $n) {
        $m = [regex]::Match($lines[$i], '^\s*class\s+(\w+)\s*:\s*public\s+(TForm|TFrame)\b')
        if (-not $m.Success) { $i++; continue }
        $className = $m.Groups[1].Value

        # Walk the class body: track current section, scan to closing "};"
        $section = ''                 # '', published, private, protected, public
        $seenMethodInPublished = $false
        $methodSection = @{}          # handlerName -> section it was declared in
        $j = $i + 1
        while ($j -lt $n) {
            $line  = $lines[$j]
            $trim  = $line.Trim()
            if ($trim -eq '};') { break }   # end of this form class

            $sm = [regex]::Match($trim, '^(__published|private|protected|public)\s*:')
            if ($sm.Success) { $section = $sm.Groups[1].Value; $j++; continue }

            if ($trim -ne '') {
                $isComment = $trim.StartsWith('//') -or $trim.StartsWith('/*') -or $trim.StartsWith('*')
                $isMethod  = (-not $isComment) -and ($trim -match '\(')
                $isField   = (-not $isComment) -and (-not $isMethod) -and ($trim -match ';\s*$')

                if ($section -eq '__published') {
                    if ($isComment) {
                        $fileViol.Add(("  V3 [{0}:{1}] comment inside __published of {2}: {3}" -f $rel, ($j+1), $className, $trim))
                    } elseif ($isMethod) {
                        $seenMethodInPublished = $true
                    } elseif ($isField -and $seenMethodInPublished) {
                        $fileViol.Add(("  V2 [{0}:{1}] component field after an event method in {2}: {3}" -f $rel, ($j+1), $className, $trim))
                    }
                }

                # record handler-style method declarations by name + section
                $hm = [regex]::Match($trim, '__fastcall\s+(\w+)\s*\(')
                if ($hm.Success) { $methodSection[$hm.Groups[1].Value] = $section }
            }
            $j++
        }

        # V1 / V4: every DFM-wired handler must be declared, and in __published
        foreach ($name in $wired) {
            if ($methodSection.ContainsKey($name)) {
                if ($methodSection[$name] -ne '__published') {
                    $fileViol.Add(("  V1 [{0}] DFM-wired handler '{1}' is in '{2}:' but must be __published (EReadError risk)" -f $rel, $name, $methodSection[$name]))
                }
            } else {
                $fileViol.Add(("  V4 [{0}] DFM-wired handler '{1}' has no declaration in class {2}" -f $rel, $name, $className))
            }
        }

        $i = $j + 1
    }

    if ($fileViol.Count -gt 0) {
        $filesWithIssues++
        $totalViolations += $fileViol.Count
        Write-Host ("FAIL  {0}" -f $rel) -ForegroundColor Red
        foreach ($v in $fileViol) { Write-Host $v -ForegroundColor Yellow }
    }
}

Write-Host ''
if ($totalViolations -eq 0) {
    Write-Host ("OK  scanned {0} header(s); no __published shape violations." -f $headers.Count) -ForegroundColor Green
    exit 0
} else {
    Write-Host ("FAILED  {0} violation(s) in {1} file(s)." -f $totalViolations, $filesWithIssues) -ForegroundColor Red
    Write-Host "Fix: in __published keep ALL component fields first, ALL DFM-wired __fastcall handlers last, and no comments inside the class body (move notes above the class)." -ForegroundColor Cyan
    exit 1
}
