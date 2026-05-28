# test-ioview-m1-readiness.ps1
# Read-only IOView M1 readiness checks for HT160S_BCB.

[CmdletBinding()]
param(
    [string]$IoTablePath = "",
    [string]$PadSourcePath = "",
    [string]$EvidenceRoot = "",
    [switch]$StrictPadCoverage
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$projectRoot = Join-Path $repoRoot "HT160S_Program_BCB_V1.0.0.0"

if (-not $IoTablePath) {
    $IoTablePath = Join-Path $repoRoot "system\IO_Table.csv"
}

if (-not $PadSourcePath) {
    $PadSourcePath = Join-Path $projectRoot "uPadInterface.cpp"
}

if (-not $EvidenceRoot) {
    $EvidenceRoot = Join-Path $repoRoot "logs\ioview-m1-readiness"
}

$knownTypes = @("Sensor", "Switch", "Cylinder", "Cylinder_On", "Cylinder_Off", "Sucker", "Sucker_On", "Sucker_Off")
$inputTypes = @("Sensor", "Cylinder_On", "Cylinder_Off", "Sucker")
$outputTypes = @("Switch", "Cylinder", "Sucker_On", "Sucker_Off")
$requiredColumns = @("IOType", "Alias", "Lane", "ModuleType", "IP", "Port", "Bit", "InType", "ISABase", "Enable", "OnAlarmTime", "OffAlarmTime", "OnDelayTime", "OffDelayTime", "Note")
$virtualOutputAliases = @("SwServerON")

Add-Type -AssemblyName Microsoft.VisualBasic

function Write-Header {
    param([string]$Text)
    Write-Host $Text -ForegroundColor Cyan
}

function Write-Ok {
    param([string]$Text)
    Write-Host "  [OK]    $Text" -ForegroundColor Green
}

function Write-WarnLine {
    param([string]$Text)
    Write-Host "  [WARN]  $Text" -ForegroundColor Yellow
}

function Write-ErrorLine {
    param([string]$Text)
    Write-Host "  [ERROR] $Text" -ForegroundColor Red
}

function Read-LinesCP950 {
    param([string]$Path)
    if (-not (Test-Path -LiteralPath $Path)) {
        throw "File not found: $Path"
    }

    $encoding = [System.Text.Encoding]::GetEncoding(950)
    return [System.IO.File]::ReadAllLines($Path, $encoding)
}

function Test-IntegerText {
    param(
        [string]$Value,
        [bool]$AllowBlank
    )

    $text = ($Value + "").Trim()
    if ($text -eq "") {
        return $AllowBlank
    }

    return ($text -match '^[+-]?\d+$')
}

function Test-IPCellText {
    param(
        [string]$Value,
        [bool]$AllowBlank
    )

    $text = ($Value + "").Trim().ToUpperInvariant()
    if ($text -eq "") {
        return $AllowBlank
    }

    return (($text -match '^[+-]?\d+$') -or ($text -match '^[A-Z]$'))
}

function Test-PortCellText {
    param(
        [string]$Value,
        [bool]$AllowBlank
    )

    $text = ($Value + "").Trim().ToUpperInvariant()
    if ($text -eq "") {
        return $AllowBlank
    }

    return (($text -match '^[+-]?\d+$') -or ($text -match '^0X[0-9A-F]+$'))
}

function Get-IoSide {
    param([string]$Type)
    if ($inputTypes -contains $Type) {
        return "Input"
    }
    if ($outputTypes -contains $Type) {
        return "Output"
    }
    return "Unknown"
}

function Convert-RowToLineObject {
    param(
        [object]$Row,
        [int]$LineNo
    )

    $Row | Add-Member -NotePropertyName LineNo -NotePropertyValue $LineNo -Force
    return $Row
}

function Read-IoTable {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        throw "File not found: $Path"
    }

    $encoding = [System.Text.Encoding]::GetEncoding(950)
    $parser = New-Object Microsoft.VisualBasic.FileIO.TextFieldParser($Path, $encoding)
    $parser.TextFieldType = [Microsoft.VisualBasic.FileIO.FieldType]::Delimited
    $parser.SetDelimiters(",")
    $parser.HasFieldsEnclosedInQuotes = $true

    $rows = @()
    try {
        if ($parser.EndOfData) {
            throw "IO table is empty: $Path"
        }

        $header = @($parser.ReadFields() | ForEach-Object { ($_ + "").Trim() })
        $missing = @($requiredColumns | Where-Object { $header -notcontains $_ })
        if ($missing.Count -gt 0) {
            throw "IO table missing required columns: $($missing -join ', ')"
        }

        $lineNo = 1
        while (-not $parser.EndOfData) {
            $fields = @($parser.ReadFields())
            $lineNo++
            if ($fields.Count -eq 0) {
                continue
            }

            $firstField = ($fields[0] + "").Trim()
            if ($firstField.StartsWith("//") -or $firstField.StartsWith("#") -or $firstField.StartsWith(";")) {
                continue
            }

            $hasText = $false
            foreach ($fieldValue in $fields) {
                if (($fieldValue + "").Trim() -ne "") {
                    $hasText = $true
                    break
                }
            }
            if (-not $hasText) {
                continue
            }

            $objectData = [ordered]@{}
            for ($index = 0; $index -lt $header.Count; $index++) {
                if ($index -lt $fields.Count) {
                    $objectData[$header[$index]] = $fields[$index]
                }
                else {
                    $objectData[$header[$index]] = ""
                }
            }
            $objectData["LineNo"] = $lineNo
            $rows += [pscustomobject]$objectData
        }
    }
    finally {
        $parser.Close()
    }

    if ($rows.Count -lt 1) {
        throw "IO table has no data rows: $Path"
    }

    return $rows
}

function Read-PadDefinitions {
    param([string]$Path)

    $items = @()
    if (-not (Test-Path -LiteralPath $Path)) {
        return $items
    }

    $lines = Read-LinesCP950 -Path $Path
    foreach ($line in $lines) {
        if ($line -match '^\s*\{"[^"]+",\s*"([^"]+)",\s*"([^"]+)",') {
            $items += [pscustomobject]@{
                PadName = $matches[1]
                InputName = $matches[2]
            }
        }
    }
    return $items
}

function Add-Issue {
    param(
        [System.Collections.ArrayList]$List,
        [string]$Code,
        [string]$Message,
        [int]$LineNo = 0
    )

    [void]$List.Add([pscustomobject]@{
        Code = $Code
        Message = $Message
        LineNo = $LineNo
    })
}

$sessionDir = Join-Path $EvidenceRoot (Get-Date -Format "yyyyMMdd-HHmmss")
New-Item -ItemType Directory -Path $sessionDir -Force | Out-Null

$errors = New-Object System.Collections.ArrayList
$warnings = New-Object System.Collections.ArrayList
$rows = @()
$padDefs = @()

Write-Host ""
Write-Header "======================================================================"
Write-Header " HT160S IOView M1 readiness check"
Write-Header "======================================================================"
Write-Host "  Repo root : $repoRoot"
Write-Host "  IO table  : $IoTablePath"
Write-Host "  Pad source: $PadSourcePath"
Write-Host "  Evidence  : $sessionDir"
Write-Host ""

try {
    $rows = @(Read-IoTable -Path $IoTablePath)
    $padDefs = @(Read-PadDefinitions -Path $PadSourcePath)
}
catch {
    Add-Issue -List $errors -Code "PARSE_ERROR" -Message $_.Exception.Message
}

if ($errors.Count -eq 0) {
    Write-Header "--- IO table structure ---"
    Write-Ok ("Rows parsed: {0}" -f $rows.Count)

    $aliasMap = @{}
    $rowMap = @{}
    $addressMap = @{}

    foreach ($row in $rows) {
        $type = ($row.IOType + "").Trim()
        $alias = ($row.Alias + "").Trim()
        $enable = ($row.Enable + "").Trim()
        $note = ($row.Note + "").Trim().ToUpperInvariant()
        $lineNo = [int]$row.LineNo

        if ($knownTypes -notcontains $type) {
            Add-Issue -List $errors -Code "UNKNOWN_TYPE" -LineNo $lineNo -Message ("Line {0}: unknown IOType '{1}' for alias '{2}'" -f $lineNo, $type, $alias)
        }

        if ($alias -eq "") {
            Add-Issue -List $errors -Code "MISSING_ALIAS" -LineNo $lineNo -Message ("Line {0}: Alias is required" -f $lineNo)
        }
        elseif ($aliasMap.ContainsKey($alias.ToUpperInvariant())) {
            Add-Issue -List $errors -Code "DUPLICATE_ALIAS" -LineNo $lineNo -Message ("Line {0}: duplicate Alias '{1}' first seen at line {2}" -f $lineNo, $alias, $aliasMap[$alias.ToUpperInvariant()])
        }
        else {
            $aliasMap[$alias.ToUpperInvariant()] = $lineNo
            $rowMap[$alias.ToUpperInvariant()] = $row
        }

        if (-not (Test-IntegerText -Value $enable -AllowBlank $false)) {
            Add-Issue -List $errors -Code "BAD_ENABLE" -LineNo $lineNo -Message ("Line {0}: Enable must be numeric for alias '{1}'" -f $lineNo, $alias)
        }

        if (-not (Test-IntegerText -Value $row.Lane -AllowBlank $true)) {
            Add-Issue -List $errors -Code "BAD_LANE" -LineNo $lineNo -Message ("Line {0}: Lane must be numeric for alias '{1}'" -f $lineNo, $alias)
        }
        if (-not (Test-IntegerText -Value $row.ModuleType -AllowBlank $true)) {
            Add-Issue -List $errors -Code "BAD_MODULE_TYPE" -LineNo $lineNo -Message ("Line {0}: ModuleType must be numeric for alias '{1}'" -f $lineNo, $alias)
        }
        if (-not (Test-IPCellText -Value $row.IP -AllowBlank $true)) {
            Add-Issue -List $errors -Code "BAD_IP" -LineNo $lineNo -Message ("Line {0}: IP must be numeric or A-Z for alias '{1}'" -f $lineNo, $alias)
        }
        if (-not (Test-PortCellText -Value $row.Port -AllowBlank $true)) {
            Add-Issue -List $errors -Code "BAD_PORT" -LineNo $lineNo -Message ("Line {0}: Port must be numeric or hex for alias '{1}'" -f $lineNo, $alias)
        }
        if (-not (Test-IntegerText -Value $row.Bit -AllowBlank $true)) {
            Add-Issue -List $errors -Code "BAD_BIT" -LineNo $lineNo -Message ("Line {0}: Bit must be numeric for alias '{1}'" -f $lineNo, $alias)
        }
        if (-not (Test-IntegerText -Value $row.InType -AllowBlank $true)) {
            Add-Issue -List $errors -Code "BAD_INTYPE" -LineNo $lineNo -Message ("Line {0}: InType must be numeric for alias '{1}'" -f $lineNo, $alias)
        }
        if (-not (Test-IntegerText -Value $row.ISABase -AllowBlank $true)) {
            Add-Issue -List $errors -Code "BAD_ISABASE" -LineNo $lineNo -Message ("Line {0}: ISABase must be numeric for alias '{1}'" -f $lineNo, $alias)
        }

        $enabled = $false
        if (Test-IntegerText -Value $enable -AllowBlank $false) {
            $enabled = ([int]$enable -ne 0)
        }

        if ($note -eq "COMM_PAD") {
            if ($enabled) {
                Add-Issue -List $errors -Code "PAD_COMM_ENABLED" -LineNo $lineNo -Message ("Line {0}: Pad communication alias '{1}' must stay disabled in IO_Table.csv" -f $lineNo, $alias)
            }

            foreach ($field in @("Lane", "IP", "Port", "Bit")) {
                if (($row.$field + "").Trim() -ne "") {
                    Add-Issue -List $warnings -Code "PAD_COMM_ADDRESS_PRESENT" -LineNo $lineNo -Message ("Line {0}: Pad communication alias '{1}' should not carry physical {2}" -f $lineNo, $alias, $field)
                    break
                }
            }
        }

        if ($enabled) {
            if ($virtualOutputAliases -contains $alias) {
                Add-Issue -List $warnings -Code "VIRTUAL_OUTPUT_UNMAPPED" -LineNo $lineNo -Message ("Line {0}: virtual output '{1}' is enabled without a physical IO address" -f $lineNo, $alias)
            }
            else {
                foreach ($field in @("Lane", "IP", "Port", "Bit")) {
                    if (($row.$field + "").Trim() -eq "") {
                        Add-Issue -List $errors -Code "ENABLED_ADDRESS_INCOMPLETE" -LineNo $lineNo -Message ("Line {0}: enabled IO '{1}' requires {2}" -f $lineNo, $alias, $field)
                    }
                }

                $side = Get-IoSide -Type $type
                if ($side -ne "Unknown") {
                    $addr = ("{0}|{1}|{2}|{3}|{4}" -f $side, $row.Lane, $row.IP, $row.Port, $row.Bit).ToUpperInvariant()
                    if ($addressMap.ContainsKey($addr)) {
                        Add-Issue -List $warnings -Code "DUPLICATE_ENABLED_ADDRESS" -LineNo $lineNo -Message ("Line {0}: {1} enabled address also used by line {2} ({3})" -f $lineNo, $side, $addressMap[$addr].LineNo, $addressMap[$addr].Alias)
                    }
                    else {
                        $addressMap[$addr] = [pscustomobject]@{ LineNo = $lineNo; Alias = $alias }
                    }
                }
            }
        }
    }

    if ($errors.Count -eq 0) {
        Write-Ok "Structural checks passed"
    }

    Write-Header "--- Pad alias coverage ---"
    if ($padDefs.Count -eq 0) {
        Add-Issue -List $warnings -Code "PAD_DEFS_NOT_FOUND" -Message "No PadButtonDefs parsed; Pad coverage was skipped"
    }
    else {
        Write-Ok ("Pad definitions parsed: {0}" -f $padDefs.Count)
        foreach ($pad in $padDefs) {
            $padNameKey = $pad.PadName.ToUpperInvariant()
            $inputNameKey = $pad.InputName.ToUpperInvariant()
            if (-not $aliasMap.ContainsKey($padNameKey)) {
                Add-Issue -List $warnings -Code "PAD_SWITCH_MISSING" -Message ("Pad switch alias missing in IO_Table.csv: {0}" -f $pad.PadName)
            }
            else {
                $padRow = $rowMap[$padNameKey]
                if (($padRow.Note + "").Trim().ToUpperInvariant() -ne "COMM_PAD") {
                    Add-Issue -List $warnings -Code "PAD_SWITCH_NOT_COMM" -LineNo ([int]$padRow.LineNo) -Message ("Line {0}: Pad switch alias '{1}' should be tagged COMM_PAD" -f $padRow.LineNo, $pad.PadName)
                }
                if (([int](($padRow.Enable + "0").Trim())) -ne 0) {
                    Add-Issue -List $warnings -Code "PAD_SWITCH_ENABLED" -LineNo ([int]$padRow.LineNo) -Message ("Line {0}: Pad switch alias '{1}' should be disabled physical IO" -f $padRow.LineNo, $pad.PadName)
                }
            }
            if (-not $aliasMap.ContainsKey($inputNameKey)) {
                Add-Issue -List $warnings -Code "PAD_SENSOR_MISSING" -Message ("Pad sensor alias missing in IO_Table.csv: {0}" -f $pad.InputName)
            }
            else {
                $padRow = $rowMap[$inputNameKey]
                if (($padRow.Note + "").Trim().ToUpperInvariant() -ne "COMM_PAD") {
                    Add-Issue -List $warnings -Code "PAD_SENSOR_NOT_COMM" -LineNo ([int]$padRow.LineNo) -Message ("Line {0}: Pad sensor alias '{1}' should be tagged COMM_PAD" -f $padRow.LineNo, $pad.InputName)
                }
                if (([int](($padRow.Enable + "0").Trim())) -ne 0) {
                    Add-Issue -List $warnings -Code "PAD_SENSOR_ENABLED" -LineNo ([int]$padRow.LineNo) -Message ("Line {0}: Pad sensor alias '{1}' should be disabled physical IO" -f $padRow.LineNo, $pad.InputName)
                }
            }
        }
    }

    if ($StrictPadCoverage) {
        foreach ($warning in @($warnings | Where-Object { $_.Code -like "PAD_*" })) {
            Add-Issue -List $errors -Code $warning.Code -Message $warning.Message -LineNo $warning.LineNo
        }
    }
}

Write-Host ""
Write-Header "--- Findings ---"
if ($errors.Count -eq 0) {
    Write-Ok "No blocking errors"
}
else {
    foreach ($errorItem in $errors) {
        Write-ErrorLine $errorItem.Message
    }
}

if ($warnings.Count -eq 0) {
    Write-Ok "No warnings"
}
else {
    foreach ($warningItem in $warnings) {
        Write-WarnLine $warningItem.Message
    }
}

$status = "Pass"
$exitCode = 0
if (@($errors | Where-Object { $_.Code -eq "PARSE_ERROR" }).Count -gt 0) {
    $status = "ParseError"
    $exitCode = 2
}
elseif ($errors.Count -gt 0) {
    $status = "Fail"
    $exitCode = 1
}

$result = [ordered]@{
    Status = $status
    Errors = $errors.Count
    Warnings = $warnings.Count
    StrictPadCoverage = [bool]$StrictPadCoverage
    IoTablePath = $IoTablePath
    PadSourcePath = $PadSourcePath
    RowCount = $rows.Count
    PadDefinitionCount = $padDefs.Count
    Findings = @($errors + $warnings)
}

$resultPath = Join-Path $sessionDir "result.json"
$result | ConvertTo-Json -Depth 6 | Out-File -LiteralPath $resultPath -Encoding UTF8

Write-Host ""
Write-Host "Status  : $status"
Write-Host "Evidence: $resultPath"

exit $exitCode