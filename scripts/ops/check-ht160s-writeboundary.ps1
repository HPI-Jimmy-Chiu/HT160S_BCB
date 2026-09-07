# check-ht160s-writeboundary.ps1
# PreToolUse write-boundary guard (WHITELIST model).
# Allows write-oriented tool calls ONLY when their target resolves under an
# allowed root (the HT160S_BCB project, the Claude state dir, and temp).
# Everything else - HT160S, HT160S -Original, HT172, HT160S_StateRecord, etc. -
# is denied. Reads are never blocked.

param(
    [string[]]$AllowedRoots
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

# Base whitelist is always computed (project root + Claude state + temp), so the
# env-derived roots are never lost. Any roots passed via -AllowedRoots are ADDED
# on top (e.g. the shared SECS simulator under D:\AI_Area\Tool), not substituted.
$baseRoots = @("D:\HT160S_BCB")
if ($env:USERPROFILE) { $baseRoots += (Join-Path $env:USERPROFILE ".claude") }
if ($env:TEMP) { $baseRoots += $env:TEMP }
if ($env:TMP)  { $baseRoots += $env:TMP }
# Normalize: when several roots are passed on one command line via -File, they may
# arrive as a single comma/semicolon-joined string instead of a real array. Split
# them back out so each root is matched individually (Windows paths never contain
# ',' or ';', so this split is safe).
if ($AllowedRoots) {
    $AllowedRoots = @($AllowedRoots |
        ForEach-Object { $_ -split '[;,]' } |
        ForEach-Object { $_.Trim() } |
        Where-Object { $_ })
}
if ($AllowedRoots -and $AllowedRoots.Count -gt 0) {
    $AllowedRoots = $baseRoots + $AllowedRoots
}
else {
    $AllowedRoots = $baseRoots
}

function New-DecisionPayload {
    param(
        [string]$Decision,
        [string]$Reason,
        [string]$SystemMessage = ""
    )

    $payload = @{
        hookSpecificOutput = @{
            hookEventName = "PreToolUse"
            permissionDecision = $Decision
        }
    }

    if ($Reason) {
        $payload.hookSpecificOutput.permissionDecisionReason = $Reason
    }

    if ($SystemMessage) {
        $payload.systemMessage = $SystemMessage
    }

    return ($payload | ConvertTo-Json -Depth 10 -Compress)
}

function Resolve-FullPathSafe {
    param(
        [string]$PathValue,
        [string]$Cwd
    )

    if (-not $PathValue) { return $null }
    $candidate = $PathValue.Trim()
    if (-not $candidate) { return $null }
    $candidate = $candidate.Replace("/", "\")

    try {
        if ([System.IO.Path]::IsPathRooted($candidate)) {
            return [System.IO.Path]::GetFullPath($candidate).TrimEnd("\")
        }
        if ($Cwd) {
            return [System.IO.Path]::GetFullPath((Join-Path $Cwd $candidate)).TrimEnd("\")
        }
    }
    catch {
        return $null
    }

    return $null
}

function Test-IsUnderRoot {
    param(
        [string]$FullPath,
        [string]$Root
    )

    if (-not $FullPath -or -not $Root) { return $false }

    try {
        $normalizedRoot = [System.IO.Path]::GetFullPath($Root).TrimEnd("\")
        $normalizedPath = [System.IO.Path]::GetFullPath($FullPath).TrimEnd("\")
        return ($normalizedPath.Equals($normalizedRoot, [System.StringComparison]::OrdinalIgnoreCase) -or
                $normalizedPath.StartsWith($normalizedRoot + "\", [System.StringComparison]::OrdinalIgnoreCase))
    }
    catch {
        return $false
    }
}

function Test-IsUnderAnyRoot {
    param(
        [string]$FullPath,
        [string[]]$Roots
    )

    foreach ($root in $Roots) {
        if (Test-IsUnderRoot -FullPath $FullPath -Root $root) {
            return $true
        }
    }
    return $false
}

function Add-StringValue {
    param(
        [System.Collections.ArrayList]$Collector,
        [object]$Value
    )

    if ($Value -is [string] -and $Value.Trim()) {
        [void]$Collector.Add($Value.Trim())
    }
}

function Extract-PatchPaths {
    param([string]$PatchText)

    $paths = New-Object System.Collections.ArrayList
    if (-not $PatchText) { return $paths }

    $patchMatches = [regex]::Matches($PatchText, "(?m)^\*\*\* (?:Add|Update|Delete) File: (.+?)(?: -> .+)?$")
    foreach ($m in $patchMatches) {
        [void]$paths.Add($m.Groups[1].Value.Trim())
    }
    return $paths
}

function Collect-ToolPaths {
    param(
        [object]$Node,
        [System.Collections.ArrayList]$Collector
    )

    if ($null -eq $Node) { return }
    if ($Node -is [string]) { return }

    if ($Node -is [System.Collections.IEnumerable] -and -not ($Node -is [string])) {
        foreach ($item in $Node) {
            Collect-ToolPaths -Node $item -Collector $Collector
        }
        return
    }

    $pathPropertyNames = @(
        "file_path", "filePath", "filePaths", "dirPath", "path", "paths",
        "targetPath", "sourcePath", "destinationPath", "old_path", "new_path",
        "notebook_path", "file"
    )

    foreach ($propertyName in $pathPropertyNames) {
        if ($Node.PSObject.Properties.Name -contains $propertyName) {
            $value = $Node.$propertyName
            if ($value -is [string]) {
                Add-StringValue -Collector $Collector -Value $value
            }
            elseif ($value -is [System.Collections.IEnumerable]) {
                foreach ($item in $value) {
                    Add-StringValue -Collector $Collector -Value $item
                }
            }
        }
    }

    if ($Node.PSObject.Properties.Name -contains "input" -and $Node.input -is [string]) {
        foreach ($path in (Extract-PatchPaths -PatchText $Node.input)) {
            Add-StringValue -Collector $Collector -Value $path
        }
    }

    foreach ($property in $Node.PSObject.Properties) {
        $value = $property.Value
        if ($value -is [pscustomobject]) {
            Collect-ToolPaths -Node $value -Collector $Collector
        }
        elseif ($value -is [System.Collections.IEnumerable] -and -not ($value -is [string])) {
            foreach ($item in $value) {
                if ($item -is [pscustomobject] -or ($item -is [System.Collections.IEnumerable] -and -not ($item -is [string]))) {
                    Collect-ToolPaths -Node $item -Collector $Collector
                }
            }
        }
    }
}

function Test-IsStructuredWriteTool {
    param(
        [string]$ToolName,
        [object]$ToolInput
    )

    if (-not $ToolName) { return $false }

    $lowerName = $ToolName.ToLowerInvariant()
    $writeSignals = @("apply_patch", "create", "edit", "write", "replace", "rename", "delete", "move", "mkdir", "notebook")
    foreach ($signal in $writeSignals) {
        if ($lowerName.Contains($signal)) { return $true }
    }

    if ($ToolInput -and $ToolInput.PSObject.Properties.Name -contains "input") {
        if ($ToolInput.input -is [string] -and $ToolInput.input -match "\*\*\* (Add|Update|Delete) File:") {
            return $true
        }
    }

    return $false
}

function Test-IsShellTool {
    param([string]$ToolName)

    if (-not $ToolName) { return $false }
    $lowerName = $ToolName.ToLowerInvariant()
    $shellSignals = @("bash", "powershell", "shell", "exec", "cmd", "terminal", "run_command")
    foreach ($signal in $shellSignals) {
        if ($lowerName.Contains($signal)) { return $true }
    }
    return $false
}

function Collect-TextValues {
    param(
        [object]$Node,
        [System.Collections.ArrayList]$Collector
    )

    if ($null -eq $Node) { return }

    if ($Node -is [string]) {
        Add-StringValue -Collector $Collector -Value $Node
        return
    }

    if ($Node -is [System.Collections.IEnumerable] -and -not ($Node -is [string])) {
        foreach ($item in $Node) {
            Collect-TextValues -Node $item -Collector $Collector
        }
        return
    }

    foreach ($property in $Node.PSObject.Properties) {
        $value = $property.Value
        if ($value -is [string]) {
            Add-StringValue -Collector $Collector -Value $value
        }
        elseif ($value -is [pscustomobject] -or ($value -is [System.Collections.IEnumerable] -and -not ($value -is [string]))) {
            Collect-TextValues -Node $value -Collector $Collector
        }
    }
}

# Returns the offending token (string) if a shell command writes outside the
# allowed roots, otherwise $null. Conservative: a line that carries a write
# verb AND references a foreign absolute path (or runs from a foreign cwd) is
# treated as an out-of-boundary write.
function Test-CommandWritesOutside {
    param(
        [string]$Text,
        [string]$Cwd,
        [string[]]$AllowedRoots
    )

    if (-not $Text) { return $null }

    # Write verbs, plus a file-write redirect: '>' or '>>' that is NOT a stderr
    # redirect (2>&1, 2>, >&) - excluded via a digit/'&' lookbehind and '&' lookahead.
    $writeCommandPattern = "(?i)\b(Set-Content|Add-Content|Out-File|New-Item|Remove-Item|Move-Item|Rename-Item|Copy-Item|Clear-Content|Set-ItemProperty|mkdir|md|rmdir|rd|del|erase|cp|copy|xcopy|robocopy|mv|rm|tee|touch|git\s+(checkout|reset|clean|restore|stash)|make(\.exe)?|bcc32(\.exe)?|dcc32(\.exe)?|ilink32(\.exe)?)\b|(?<![0-9&])>>?\s*(?!&)"
    $absPathPattern = "[A-Za-z]:\\[^\s`"'<>|;]+"

    $cwdFull = $null
    if ($Cwd) { $cwdFull = Resolve-FullPathSafe -PathValue $Cwd -Cwd $null }

    $lines = $Text -split "[`r`n]"
    foreach ($line in $lines) {
        if ($line -notmatch $writeCommandPattern) { continue }

        $absMatches = [regex]::Matches($line, $absPathPattern)
        foreach ($am in $absMatches) {
            $full = Resolve-FullPathSafe -PathValue $am.Value -Cwd $cwdFull
            if ($full -and -not (Test-IsUnderAnyRoot -FullPath $full -Roots $AllowedRoots)) {
                return $am.Value
            }
        }

        if ($cwdFull -and -not (Test-IsUnderAnyRoot -FullPath $cwdFull -Roots $AllowedRoots)) {
            return "(cwd) $Cwd"
        }
    }

    return $null
}

try {
    $stdinStream = [Console]::OpenStandardInput()
    $reader = New-Object System.IO.StreamReader($stdinStream, [System.Text.Encoding]::UTF8)
    $rawInput = $reader.ReadToEnd()
    $reader.Close()

    if (-not $rawInput) {
        Write-Output (New-DecisionPayload -Decision "allow" -Reason "")
        exit 0
    }

    $hookInput = $rawInput | ConvertFrom-Json
    $toolName = [string]$hookInput.tool_name
    $toolInput = $hookInput.tool_input
    $cwd = [string]$hookInput.cwd

    $allowedDisplay = ($AllowedRoots -join "; ")

    # 1) Structured file tools (Write / Edit / NotebookEdit / apply_patch ...):
    #    every target path must resolve under an allowed root.
    if (Test-IsStructuredWriteTool -ToolName $toolName -ToolInput $toolInput) {
        $candidatePaths = New-Object System.Collections.ArrayList
        Collect-ToolPaths -Node $toolInput -Collector $candidatePaths
        foreach ($candidatePath in @($candidatePaths | Select-Object -Unique)) {
            $fullPath = Resolve-FullPathSafe -PathValue $candidatePath -Cwd $cwd
            if ($fullPath -and -not (Test-IsUnderAnyRoot -FullPath $fullPath -Roots $AllowedRoots)) {
                Write-Output (New-DecisionPayload -Decision "deny" -Reason "Write blocked: '$candidatePath' is outside the writable boundary. Only these roots are writable: $allowedDisplay")
                exit 0
            }
        }
    }

    # 2) Shell tools (Bash / PowerShell): block write-verb lines that target a
    #    foreign absolute path or run from a foreign working directory.
    if (Test-IsShellTool -ToolName $toolName) {
        $textValues = New-Object System.Collections.ArrayList
        Collect-TextValues -Node $toolInput -Collector $textValues
        foreach ($text in @($textValues | Select-Object -Unique)) {
            $offender = Test-CommandWritesOutside -Text $text -Cwd $cwd -AllowedRoots $AllowedRoots
            if ($offender) {
                Write-Output (New-DecisionPayload -Decision "deny" -Reason "Shell write blocked: targets '$offender' outside the writable boundary. Only these roots are writable: $allowedDisplay")
                exit 0
            }
        }
    }

    Write-Output (New-DecisionPayload -Decision "allow" -Reason "")
    exit 0
}
catch {
    Write-Output (New-DecisionPayload -Decision "ask" -Reason "Write-boundary hook error - review before allowing the write." -SystemMessage $_.Exception.Message)
    exit 0
}
