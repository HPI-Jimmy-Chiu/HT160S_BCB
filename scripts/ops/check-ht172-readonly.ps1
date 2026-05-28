# check-ht172-readonly.ps1
# Blocks write-oriented tool calls that target D:\HT172 during HT160S development.

param(
    [string[]]$ReadonlyRoots = @("D:\HT172")
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

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

    if (-not $PathValue) {
        return $null
    }

    $candidate = $PathValue.Trim()
    if (-not $candidate) {
        return $null
    }

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

    if (-not $FullPath -or -not $Root) {
        return $false
    }

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
    if (-not $PatchText) {
        return $paths
    }

    $matches = [regex]::Matches($PatchText, "(?m)^\*\*\* (?:Add|Update|Delete) File: (.+?)(?: -> .+)?$")
    foreach ($match in $matches) {
        [void]$paths.Add($match.Groups[1].Value.Trim())
    }

    return $paths
}

function Collect-ToolPaths {
    param(
        [object]$Node,
        [System.Collections.ArrayList]$Collector
    )

    if ($null -eq $Node) {
        return
    }

    if ($Node -is [string]) {
        return
    }

    if ($Node -is [System.Collections.IEnumerable] -and -not ($Node -is [string])) {
        foreach ($item in $Node) {
            Collect-ToolPaths -Node $item -Collector $Collector
        }
        return
    }

    $pathPropertyNames = @(
        "filePath", "filePaths", "dirPath", "path", "paths", "targetPath",
        "sourcePath", "destinationPath", "old_path", "new_path", "file"
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

function Collect-TextValues {
    param(
        [object]$Node,
        [System.Collections.ArrayList]$Collector
    )

    if ($null -eq $Node) {
        return
    }

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

function Test-IsWriteTool {
    param(
        [string]$ToolName,
        [object]$ToolInput
    )

    if (-not $ToolName) {
        return $false
    }

    $lowerName = $ToolName.ToLowerInvariant()
    $writeSignals = @("apply_patch", "create", "edit", "write", "replace", "rename", "delete", "move", "mkdir")

    foreach ($signal in $writeSignals) {
        if ($lowerName.Contains($signal)) {
            return $true
        }
    }

    if ($ToolInput -and $ToolInput.PSObject.Properties.Name -contains "input") {
        if ($ToolInput.input -is [string] -and $ToolInput.input -match "\*\*\* (Add|Update|Delete) File:") {
            return $true
        }
    }

    return $false
}

function Test-TextMentionsRoot {
    param(
        [string]$Text,
        [string]$Root
    )

    if (-not $Text -or -not $Root) {
        return $false
    }

    $normalizedText = $Text.Replace("/", "\")
    $normalizedRoot = $Root.Replace("/", "\").TrimEnd("\")
    return ($normalizedText.IndexOf($normalizedRoot, [System.StringComparison]::OrdinalIgnoreCase) -ge 0)
}

function Test-TextWritesReadonlyRoot {
    param(
        [string]$Text,
        [string]$Cwd
    )

    if (-not $Text) {
        return $false
    }

    $writeCommandPattern = "(?i)\b(Set-Content|Add-Content|Out-File|New-Item|Remove-Item|Move-Item|Rename-Item|Clear-Content|mkdir|md|rmdir|rd|del|erase|git\s+(checkout|reset|clean|restore)|make(\.exe)?|bcc32(\.exe)?)\b|>\s*"

    foreach ($root in $ReadonlyRoots) {
        $cwdUnderRoot = $false
        if ($Cwd) {
            $cwdUnderRoot = Test-IsUnderRoot -FullPath $Cwd -Root $root
        }

        if ($cwdUnderRoot -and $Text -match $writeCommandPattern) {
            return $true
        }

        $lines = $Text -split "[`r`n;]"
        foreach ($line in $lines) {
            if ((Test-TextMentionsRoot -Text $line -Root $root) -and ($line -match $writeCommandPattern)) {
                return $true
            }
        }
    }

    return $false
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

    $candidatePaths = New-Object System.Collections.ArrayList
    Collect-ToolPaths -Node $toolInput -Collector $candidatePaths

    if (Test-IsWriteTool -ToolName $toolName -ToolInput $toolInput) {
        foreach ($candidatePath in @($candidatePaths | Select-Object -Unique)) {
            $fullPath = Resolve-FullPathSafe -PathValue $candidatePath -Cwd $cwd
            foreach ($root in $ReadonlyRoots) {
                if (Test-IsUnderRoot -FullPath $fullPath -Root $root) {
                    Write-Output (New-DecisionPayload -Decision "deny" -Reason "HT172 is read-only for HT160S development: $candidatePath")
                    exit 0
                }
            }
        }
    }

    $textValues = New-Object System.Collections.ArrayList
    Collect-TextValues -Node $toolInput -Collector $textValues
    foreach ($text in @($textValues | Select-Object -Unique)) {
        if (Test-TextWritesReadonlyRoot -Text $text -Cwd $cwd) {
            Write-Output (New-DecisionPayload -Decision "deny" -Reason "Write-oriented command targets HT172, which is read-only for HT160S development.")
            exit 0
        }
    }

    Write-Output (New-DecisionPayload -Decision "allow" -Reason "")
    exit 0
}
catch {
    Write-Output (New-DecisionPayload -Decision "ask" -Reason "HT172 read-only hook error. Review before allowing write tools." -SystemMessage $_.Exception.Message)
    exit 0
}
