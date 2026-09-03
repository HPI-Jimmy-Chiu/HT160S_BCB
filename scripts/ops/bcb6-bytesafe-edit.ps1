# bcb6-bytesafe-edit.ps1
# Byte-exact find/replace for legacy BCB6 (C++Builder 6) source files.
#
# WHY: editors/IO that re-encode through UTF-8 turn non-UTF-8 Big5/CP950 bytes
# (e.g. Chinese enum comments in cmydef.h) into U+FFFD (EF BF BD) replacement
# bytes -- silent corruption. This helper reads/writes RAW bytes only; it never
# decodes or re-encodes the file, so existing Big5/CP950 bytes are preserved.
#
# PATH-AGNOSTIC by design: targets any BCB6 source tree (HT160S_BCB, HT9045 899,
# HT172, ...). It edits exactly the byte run you anchor and leaves everything
# else untouched.
#
# USAGE
#   # ASCII anchors (most common) -- strings are taken as Latin1 (1 char = 1 byte):
#   bcb6-bytesafe-edit.ps1 -Path X.cpp -Search "//old comment" -Replace "//new comment"
#
#   # Anchor/replacement that contains Big5 Chinese -> use hex (safe for any byte):
#   bcb6-bytesafe-edit.ps1 -Path X.h -SearchHex "A4A4 A4E5" -ReplaceHex "2F2F 6E6577"
#
#   # Inspect first, write nothing:
#   bcb6-bytesafe-edit.ps1 -Path X.cpp -Search "anchor" -DryRun
#
#   # Delete a run (empty replacement):
#   bcb6-bytesafe-edit.ps1 -Path X.cpp -Search "junk" -Replace ""
#
# DEFAULTS / SAFETY
#   * Requires EXACTLY ONE match unless -All or -Count is given (like the Edit tool).
#   * Writes <Path>.bak before changing (suppress with -NoBackup).
#   * Aborts WITHOUT writing if the result would add EF BF BD bytes or a UTF-8 BOM.
#   * Preserves the file's bytes verbatim outside the matched run (EOL untouched).

[CmdletBinding(DefaultParameterSetName = "Text")]
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [string]$Path,

    [Parameter(ParameterSetName = "Text")]
    [AllowEmptyString()]
    [string]$Search,

    [Parameter(ParameterSetName = "Text")]
    [AllowEmptyString()]
    [string]$Replace,

    [Parameter(ParameterSetName = "Hex")]
    [string]$SearchHex,

    [Parameter(ParameterSetName = "Hex")]
    [AllowEmptyString()]
    [string]$ReplaceHex = "",

    [switch]$All,
    [int]$Count = 0,
    [switch]$NoBackup,
    [switch]$DryRun
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

# --- 1252/Latin1 round-trips all 256 byte values 1:1 -------------------------
$latin1 = [System.Text.Encoding]::GetEncoding(28591)

function ConvertFrom-Hex {
    param([string]$Hex)
    $clean = ($Hex -replace '0[xX]', '') -replace '[^0-9A-Fa-f]', ''
    if ($clean.Length % 2 -ne 0) {
        throw "Hex string has an odd number of digits: '$Hex'"
    }
    $bytes = New-Object byte[] ($clean.Length / 2)
    for ($i = 0; $i -lt $bytes.Length; $i++) {
        $bytes[$i] = [Convert]::ToByte($clean.Substring($i * 2, 2), 16)
    }
    return ,$bytes
}

function Get-ByteMatchOffsets {
    param([byte[]]$Haystack, [byte[]]$Needle)
    $offsets = New-Object System.Collections.ArrayList
    if ($Needle.Length -eq 0) { return ,$offsets }
    $last = $Haystack.Length - $Needle.Length
    $i = 0
    while ($i -le $last) {
        $match = $true
        for ($j = 0; $j -lt $Needle.Length; $j++) {
            if ($Haystack[$i + $j] -ne $Needle[$j]) { $match = $false; break }
        }
        if ($match) {
            [void]$offsets.Add($i)
            $i += $Needle.Length   # non-overlapping
        }
        else {
            $i++
        }
    }
    return ,$offsets
}

function Count-Replacement {
    param([byte[]]$Bytes)
    $c = 0
    for ($i = 0; $i -le $Bytes.Length - 3; $i++) {
        if ($Bytes[$i] -eq 0xEF -and $Bytes[$i + 1] -eq 0xBF -and $Bytes[$i + 2] -eq 0xBD) { $c++ }
    }
    return $c
}

function Test-Utf8Bom {
    param([byte[]]$Bytes)
    return ($Bytes.Length -ge 3 -and $Bytes[0] -eq 0xEF -and $Bytes[1] -eq 0xBB -and $Bytes[2] -eq 0xBF)
}

if (-not (Test-Path -LiteralPath $Path)) {
    throw "Target file not found: $Path"
}

# --- resolve search / replace bytes per parameter set ------------------------
if ($PSCmdlet.ParameterSetName -eq "Hex") {
    if (-not $SearchHex) { throw "-SearchHex must be a non-empty hex string." }
    $searchBytes  = ConvertFrom-Hex $SearchHex
    $replaceBytes = ConvertFrom-Hex $ReplaceHex
}
else {
    if ($null -eq $Search -or $Search.Length -eq 0) {
        throw "-Search must be a non-empty string (use -SearchHex for binary anchors)."
    }
    $searchBytes  = $latin1.GetBytes($Search)
    $replaceBytes = $latin1.GetBytes([string]$Replace)
}

$original = [System.IO.File]::ReadAllBytes((Resolve-Path -LiteralPath $Path).ProviderPath)
$offsets  = Get-ByteMatchOffsets -Haystack $original -Needle $searchBytes
$found    = $offsets.Count

Write-Output ("Matches: {0}  (search={1} bytes, replace={2} bytes, file={3} bytes)" -f `
    $found, $searchBytes.Length, $replaceBytes.Length, $original.Length)

if ($found -eq 0) {
    throw "No match found. Anchor not present (check exact spacing/EOL, or use -SearchHex)."
}

# --- occurrence-count policy -------------------------------------------------
if ($Count -gt 0) {
    if ($found -ne $Count) { throw "Expected exactly $Count match(es) but found $found." }
}
elseif (-not $All) {
    if ($found -gt 1) {
        throw "Found $found matches but expected exactly 1. Make the anchor unique, or pass -All / -Count $found."
    }
}

if ($DryRun) {
    $preview = ($offsets | ForEach-Object { $_ }) -join ", "
    Write-Output "DryRun: no file written. Offsets: $preview"
    return
}

# --- splice (non-overlapping, left to right) ---------------------------------
$out = New-Object System.Collections.Generic.List[byte]
$cursor = 0
foreach ($off in $offsets) {
    for ($k = $cursor; $k -lt $off; $k++) { $out.Add($original[$k]) }
    foreach ($b in $replaceBytes) { $out.Add($b) }
    $cursor = $off + $searchBytes.Length
}
for ($k = $cursor; $k -lt $original.Length; $k++) { $out.Add($original[$k]) }
$result = $out.ToArray()

# --- corruption guards -------------------------------------------------------
$beforeRepl = Count-Replacement -Bytes $original
$afterRepl  = Count-Replacement -Bytes $result
if ($afterRepl -gt $beforeRepl) {
    throw ("Aborted: result would add EF BF BD replacement bytes ({0} -> {1}). Nothing written." -f $beforeRepl, $afterRepl)
}
if ((-not (Test-Utf8Bom -Bytes $original)) -and (Test-Utf8Bom -Bytes $result)) {
    throw "Aborted: result would introduce a UTF-8 BOM. Nothing written."
}

# --- backup + write ----------------------------------------------------------
$full = (Resolve-Path -LiteralPath $Path).ProviderPath
if (-not $NoBackup) {
    $bak = "$full.bak"
    [System.IO.File]::WriteAllBytes($bak, $original)
    Write-Output "Backup: $bak"
}
[System.IO.File]::WriteAllBytes($full, $result)
Write-Output ("Wrote {0}: {1} -> {2} bytes, {3} occurrence(s) replaced." -f $full, $original.Length, $result.Length, $found)
