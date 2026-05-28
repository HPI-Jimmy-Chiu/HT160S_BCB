# test-ht160s-startup.ps1
# Builds HT160S, launches the executable, and collects startup crash evidence.

param(
    [switch]$Clean,
    [int]$StartupSeconds = 10,
    [int]$MaxAttempts = 1,
    [string]$BCBRoot = "D:\ProgramFiles\Borland\CBuilder6",
    [string]$EvidenceRoot = "",
    [switch]$KeepProcessOnPass,
    [switch]$SkipAutomationProbe,
    [switch]$ProbeTopForms
)

Set-StrictMode -Version 2.0
$ErrorActionPreference = "Stop"

if ($StartupSeconds -lt 1) {
    throw "StartupSeconds must be >= 1."
}

if ($MaxAttempts -lt 1) {
    throw "MaxAttempts must be >= 1."
}

$repoRoot = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)
$projectRoot = Join-Path $repoRoot "HT160S_Program_BCB_V1.0.0.0"
$buildScript = Join-Path $PSScriptRoot "build-ht160s.ps1"
$projectFile = Join-Path $projectRoot "ht160s.bpr"

if (-not (Test-Path -LiteralPath $projectFile)) {
    throw "Project file not found: $projectFile"
}

$projectText = Get-Content -LiteralPath $projectFile -Raw
$projectMatch = [regex]::Match($projectText, '<PROJECT\s+value="([^"]+)"')
if (-not $projectMatch.Success) {
    throw "PROJECT output name not found in: $projectFile"
}

$exeName = $projectMatch.Groups[1].Value
$exePath = Join-Path $projectRoot $exeName
$staleExeNames = @("ht160s.exe", "ht160s_bcb.exe")

if (-not $EvidenceRoot) {
    $EvidenceRoot = Join-Path $repoRoot "logs\startup-smoke"
}

if (-not (Test-Path -LiteralPath $buildScript)) {
    throw "Build script not found: $buildScript"
}

function New-DirectoryIfMissing {
    param([string]$Path)

    if (-not (Test-Path -LiteralPath $Path)) {
        New-Item -ItemType Directory -Path $Path | Out-Null
    }
}

function Initialize-WindowCollector {
    $typeName = [System.Management.Automation.PSTypeName]"Win32WindowCollector"
    if ($typeName.Type -ne $null) {
        return $true
    }

    $source = @"
using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Text;

public class Win32WindowInfo
{
    public int ProcessId;
    public string Title;
    public bool Visible;
}

public class Win32WindowCollector
{
    private delegate bool EnumWindowsProc(IntPtr hWnd, IntPtr lParam);

    [DllImport("user32.dll")]
    private static extern bool EnumWindows(EnumWindowsProc enumProc, IntPtr lParam);

    [DllImport("user32.dll")]
    private static extern int GetWindowText(IntPtr hWnd, StringBuilder text, int count);

    [DllImport("user32.dll")]
    private static extern int GetWindowTextLength(IntPtr hWnd);

    [DllImport("user32.dll")]
    private static extern bool IsWindowVisible(IntPtr hWnd);

    [DllImport("user32.dll")]
    private static extern uint GetWindowThreadProcessId(IntPtr hWnd, out uint processId);

    public static List<Win32WindowInfo> GetWindows()
    {
        List<Win32WindowInfo> windows = new List<Win32WindowInfo>();

        EnumWindows(delegate(IntPtr hWnd, IntPtr lParam) {
            int length = GetWindowTextLength(hWnd);
            if (length <= 0) {
                return true;
            }

            StringBuilder builder = new StringBuilder(length + 1);
            GetWindowText(hWnd, builder, builder.Capacity);

            uint processId;
            GetWindowThreadProcessId(hWnd, out processId);

            Win32WindowInfo info = new Win32WindowInfo();
            info.ProcessId = (int)processId;
            info.Title = builder.ToString();
            info.Visible = IsWindowVisible(hWnd);
            windows.Add(info);
            return true;
        }, IntPtr.Zero);

        return windows;
    }
}
"@

    try {
        Add-Type -TypeDefinition $source -ErrorAction Stop
        return $true
    }
    catch {
        Write-Warning ("Window collector unavailable: " + $_.Exception.Message)
        return $false
    }
}

$script:WindowCollectorReady = Initialize-WindowCollector

function Save-WindowSnapshot {
    param(
        [string]$Path,
        [int]$TargetProcessId
    )

    $windows = @()
    if ($script:WindowCollectorReady) {
        $windows = @([Win32WindowCollector]::GetWindows() |
            Where-Object { $_.Visible -and $_.Title } |
            ForEach-Object {
                [pscustomobject]@{
                    ProcessId = $_.ProcessId
                    IsTarget  = ($_.ProcessId -eq $TargetProcessId)
                    Title     = $_.Title
                }
            })
    }

    $windows | Sort-Object ProcessId, Title | Export-Csv -LiteralPath $Path -NoTypeInformation -Encoding UTF8
    return $windows
}

function Collect-ApplicationEvents {
    param(
        [datetime]$StartTime,
        [datetime]$EndTime,
        [string]$Path
    )

    try {
        $events = @(Get-WinEvent -FilterHashtable @{
                LogName   = "Application"
                StartTime = $StartTime.AddSeconds(-2)
                EndTime   = $EndTime.AddSeconds(5)
            } -ErrorAction Stop |
            Where-Object {
                $_.ProviderName -match "Application Error|Windows Error Reporting|Application Hang" -or
                $_.Message -match "ht160s|Access violation|EAccessViolation|exception|Exception|Error"
            } |
            Select-Object TimeCreated, ProviderName, Id, LevelDisplayName, Message)

        if ($events.Count -gt 0) {
            $events | Format-List | Out-File -LiteralPath $Path -Encoding UTF8
        }
        else {
            "No matching Application event log entries." | Out-File -LiteralPath $Path -Encoding UTF8
        }

        $events
        return
    }
    catch {
        ("Failed to collect Application event log entries: " + $_.Exception.Message) |
            Out-File -LiteralPath $Path -Encoding UTF8
        return
    }
}

function Invoke-HT160Build {
    param([string]$AttemptDir)

    $buildLog = Join-Path $AttemptDir "build.log"
    $arguments = @("-ExecutionPolicy", "Bypass", "-File", $buildScript, "-BCBRoot", $BCBRoot)
    if ($Clean) {
        $arguments += "-Clean"
    }

    $output = & powershell.exe @arguments 2>&1
    $exitCode = $LASTEXITCODE
    $output | Out-File -LiteralPath $buildLog -Encoding UTF8

    $staleExeExists = @($staleExeNames | Where-Object { $_ -ine $exeName -and (Test-Path -LiteralPath (Join-Path $projectRoot $_)) })

    return [pscustomobject]@{
        ExitCode = $exitCode
        Success  = ($exitCode -eq 0 -and (Test-Path -LiteralPath $exePath))
        Log      = $buildLog
        ExeName  = $exeName
        StaleExeExists = $staleExeExists
    }
}

function Get-AutomationConfig {
    $config = [ordered]@{
        Enabled = $true
        Port = 16060
    }

    $iniPath = Join-Path $repoRoot "system\automation.ini"
    if (-not (Test-Path -LiteralPath $iniPath)) {
        return [pscustomobject]$config
    }

    $inAutomationSection = $false
    foreach ($line in Get-Content -LiteralPath $iniPath) {
        $text = $line.Trim()
        if (-not $text -or $text.StartsWith(";") -or $text.StartsWith("#")) {
            continue
        }

        if ($text -match '^\[(.+)\]$') {
            $inAutomationSection = ($matches[1] -ieq "Automation")
            continue
        }

        if (-not $inAutomationSection -or $text -notmatch '^([^=]+)=(.*)$') {
            continue
        }

        $key = $matches[1].Trim()
        $value = $matches[2].Trim()
        if ($key -ieq "Enabled") {
            $config.Enabled = ($value -notmatch '^(0|false|no|off)$')
        }
        elseif ($key -ieq "Port") {
            $portValue = 0
            if ([int]::TryParse($value, [ref]$portValue) -and $portValue -gt 0 -and $portValue -le 65535) {
                $config.Port = $portValue
            }
        }
    }

    return [pscustomobject]$config
}

function Test-AutomationSocket {
    param(
        [int]$Port,
        [string]$Path,
        [int]$TargetProcessId,
        [bool]$ProbeTopForms
    )

    $result = [ordered]@{
        Enabled = $true
        Port = $Port
        Success = $false
        Banner = ""
        Ping = ""
        Status = ""
        TopForms = ""
        Error = ""
    }

    $client = $null
    try {
        $client = New-Object System.Net.Sockets.TcpClient
        $async = $client.BeginConnect("127.0.0.1", $Port, $null, $null)
        if (-not $async.AsyncWaitHandle.WaitOne(3000)) {
            throw "Connect timeout"
        }
        $client.EndConnect($async)
        $stream = $client.GetStream()
        $stream.ReadTimeout = 3000
        $writer = [System.IO.StreamWriter]::new($stream, [System.Text.Encoding]::ASCII)
        $writer.NewLine = "`r`n"
        $writer.AutoFlush = $true
        $reader = [System.IO.StreamReader]::new($stream, [System.Text.Encoding]::ASCII)

        $result.Banner = $reader.ReadLine()
        $writer.WriteLine("PING")
        $result.Ping = $reader.ReadLine()
        $result.Success = ($result.Banner -match '^OK\|CONNECTED\|HT160S_BCB' -and $result.Ping -eq "OK|PONG|HT160S_BCB")
        if (-not $result.Success) {
            $result.Error = "Unexpected Automation reply"
        }

        if ($result.Success -and $ProbeTopForms) {
            $ready = $false
            $deadline = (Get-Date).AddSeconds(5)
            while ((Get-Date) -lt $deadline -and -not $ready) {
                $writer.WriteLine("GET_STATUS")
                $result.Status = $reader.ReadLine()
                if ($result.Status -match 'PROCESS_ID=([0-9]+)' -and [int]$matches[1] -ne $TargetProcessId) {
                    $result.Success = $false
                    $result.Error = "Automation socket belongs to another process"
                    break
                }
                $ready = ($result.Status -match 'MAIN_FORM_READY=1')
                if (-not $ready) {
                    Start-Sleep -Milliseconds 200
                }
            }

            if ($result.Error -eq "" -and -not $ready) {
                $result.Success = $false
                $result.Error = "Main form not ready for top form probe"
            }
        }

        if ($result.Success -and $ProbeTopForms) {
            $writer.WriteLine("SMOKE_TOP_FORMS")
            $result.TopForms = $reader.ReadLine()
            $result.Success = ($result.TopForms -match '^OK\|TOP_FORMS\|OPENED=')
            if (-not $result.Success) {
                $result.Error = "Top form probe failed"
            }
        }
    }
    catch {
        $result.Error = $_.Exception.Message
    }
    finally {
        if ($client -ne $null) {
            $client.Close()
        }
    }

    $resultObject = [pscustomobject]$result
    $resultObject | ConvertTo-Json -Depth 4 | Out-File -LiteralPath $Path -Encoding UTF8
    return $resultObject
}

function Stop-TestProcess {
    param(
        [System.Diagnostics.Process]$Process,
        [bool]$KeepAlive
    )

    if ($Process -eq $null) {
        return
    }

    $Process.Refresh()
    if ($Process.HasExited -or $KeepAlive) {
        return
    }

    try {
        [void]$Process.CloseMainWindow()
        if (-not $Process.WaitForExit(3000)) {
            $Process.Kill()
            [void]$Process.WaitForExit(5000)
        }
    }
    catch {
        try {
            if (-not $Process.HasExited) {
                $Process.Kill()
            }
        }
        catch {
        }
    }
}

function Invoke-StartupProbe {
    param([string]$AttemptDir)

    $runLog = Join-Path $AttemptDir "run.log"
    $windowBeforePath = Join-Path $AttemptDir "windows-before.csv"
    $windowAfterPath = Join-Path $AttemptDir "windows-after.csv"
    $eventLogPath = Join-Path $AttemptDir "application-events.txt"
    $matchedWindowPath = Join-Path $AttemptDir "matched-windows.csv"
    $processInfoPath = Join-Path $AttemptDir "process.json"
    $automationProbePath = Join-Path $AttemptDir "automation-probe.json"

    $startedAt = Get-Date
    $exceptionPattern = "Access violation|EAccessViolation|Debugger Exception|Exception Notification|raised exception|Application Error|Windows Error Reporting|Fatal|Runtime error"
    $windowsBefore = Save-WindowSnapshot -Path $windowBeforePath -TargetProcessId 0
    $baselineExceptionKeys = @($windowsBefore | Where-Object {
        $_.Title -match $exceptionPattern
    } | ForEach-Object {
        ("{0}|{1}" -f $_.ProcessId, $_.Title)
    })

    "Launching: $exePath" | Out-File -LiteralPath $runLog -Encoding UTF8
    "WorkingDirectory: $projectRoot" | Out-File -LiteralPath $runLog -Encoding UTF8 -Append
    "StartupSeconds: $StartupSeconds" | Out-File -LiteralPath $runLog -Encoding UTF8 -Append
    "ProbeTopForms: $ProbeTopForms" | Out-File -LiteralPath $runLog -Encoding UTF8 -Append
    $staleExeExists = @($staleExeNames | Where-Object { $_ -ine $exeName -and (Test-Path -LiteralPath (Join-Path $projectRoot $_)) })
    "StaleExeExists: $($staleExeExists -join ',')" | Out-File -LiteralPath $runLog -Encoding UTF8 -Append

    $process = $null
    $targetProcessId = 0
    $matchedWindows = @()
    $status = "Fail"
    $reason = "Unknown"

    try {
    $process = Start-Process -FilePath $exePath -WorkingDirectory $projectRoot -PassThru
    $targetProcessId = $process.Id

    $deadline = $startedAt.AddSeconds($StartupSeconds)
    while ((Get-Date) -lt $deadline) {
        $process.Refresh()
        $windows = Save-WindowSnapshot -Path $windowAfterPath -TargetProcessId $targetProcessId
        $matchedWindows = @($windows | Where-Object {
            $windowKey = ("{0}|{1}" -f $_.ProcessId, $_.Title)
            ($_.ProcessId -eq $targetProcessId -or $_.Title -match "ht160s|Debugger Exception") -and
            $_.Title -match $exceptionPattern -and
            ($baselineExceptionKeys -notcontains $windowKey)
        })

        if ($matchedWindows.Count -gt 0) {
            $status = "Fail"
            $reason = "ExceptionWindowDetected"
            break
        }

        if ($process.HasExited) {
            $status = "Fail"
            $reason = "ProcessExitedDuringStartup"
            break
        }

        Start-Sleep -Milliseconds 500
    }

    $endedAt = Get-Date
    $process.Refresh()

    if ($status -eq "Fail" -and $reason -eq "Unknown") {
        if (-not $process.HasExited) {
            $status = "Pass"
            $reason = "ProcessAliveAfterStartupWindow"
        }
    }

    $events = @(Collect-ApplicationEvents -StartTime $startedAt -EndTime $endedAt -Path $eventLogPath)
    if (@($events).Count -gt 0 -and $status -eq "Pass") {
        $status = "Fail"
        $reason = "ApplicationEventLogMatched"
    }

    $automationProbe = $null
    $automationConfig = Get-AutomationConfig
    if (-not $SkipAutomationProbe -and $automationConfig.Enabled -and $status -eq "Pass") {
        $automationProbe = Test-AutomationSocket -Port $automationConfig.Port -Path $automationProbePath -TargetProcessId $targetProcessId -ProbeTopForms:$ProbeTopForms
        if (-not $automationProbe.Success) {
            $status = "Fail"
            if ($ProbeTopForms -and $automationProbe.Error -eq "Top form probe failed") {
                $reason = "TopFormProbeFailed"
            }
            else {
                $reason = "AutomationSocketProbeFailed"
            }
        }
    }
    elseif (-not $automationConfig.Enabled) {
        [pscustomobject]@{ Enabled = $false; Port = $automationConfig.Port; Success = $true; Skipped = "Automation disabled by ini" } |
            ConvertTo-Json -Depth 4 | Out-File -LiteralPath $automationProbePath -Encoding UTF8
    }

    $matchedWindows | Export-Csv -LiteralPath $matchedWindowPath -NoTypeInformation -Encoding UTF8

    $exitCode = $null
    if ($process.HasExited) {
        $exitCode = $process.ExitCode
    }

    $processInfo = [ordered]@{
        ProcessId = $targetProcessId
        HasExited = $process.HasExited
        ExitCode = $exitCode
        StartedAt = $startedAt.ToString("s")
        EndedAt = $endedAt.ToString("s")
        Status = $status
        Reason = $reason
        ExeName = $exeName
        StaleExeExists = $staleExeExists
        AutomationProbe = $automationProbePath
    }
    $processInfo | ConvertTo-Json -Depth 4 | Out-File -LiteralPath $processInfoPath -Encoding UTF8

    "Status: $status" | Out-File -LiteralPath $runLog -Encoding UTF8 -Append
    "Reason: $reason" | Out-File -LiteralPath $runLog -Encoding UTF8 -Append

    return [pscustomobject]@{
        Status = $status
        Reason = $reason
        ProcessId = $targetProcessId
        RunLog = $runLog
        WindowBefore = $windowBeforePath
        WindowAfter = $windowAfterPath
        MatchedWindows = $matchedWindowPath
        EventLog = $eventLogPath
        AutomationProbe = $automationProbePath
        ProcessInfo = $processInfoPath
    }
    }
    finally {
        Stop-TestProcess -Process $process -KeepAlive:($KeepProcessOnPass -and $status -eq "Pass")
    }
}

New-DirectoryIfMissing -Path $EvidenceRoot
$sessionDir = Join-Path $EvidenceRoot (Get-Date -Format "yyyyMMdd-HHmmss")
New-DirectoryIfMissing -Path $sessionDir

$attemptResults = @()
$overallStatus = "Fail"
$overallReason = "NoPassingAttempt"

for ($attempt = 1; $attempt -le $MaxAttempts; $attempt++) {
    $attemptDir = Join-Path $sessionDir ("attempt-{0:00}" -f $attempt)
    New-DirectoryIfMissing -Path $attemptDir

    Write-Host ("Attempt {0}/{1}: build" -f $attempt, $MaxAttempts)
    $buildResult = Invoke-HT160Build -AttemptDir $attemptDir

    $runResult = $null
    if ($buildResult.Success) {
        Write-Host ("Attempt {0}/{1}: startup probe" -f $attempt, $MaxAttempts)
        $runResult = Invoke-StartupProbe -AttemptDir $attemptDir
    }

    $attemptResult = [ordered]@{
        Attempt = $attempt
        Build = $buildResult
        Run = $runResult
        EvidenceDir = $attemptDir
    }
    $attemptResults += $attemptResult

    $attemptResult | ConvertTo-Json -Depth 8 | Out-File -LiteralPath (Join-Path $attemptDir "result.json") -Encoding UTF8

    if (-not $buildResult.Success) {
        $overallStatus = "Fail"
        $overallReason = "BuildFailed"
        break
    }

    if ($runResult.Status -eq "Pass") {
        $overallStatus = "Pass"
        $overallReason = $runResult.Reason
        break
    }

    $overallStatus = "Fail"
    $overallReason = $runResult.Reason
}

$summary = [ordered]@{
    Status = $overallStatus
    Reason = $overallReason
    SessionDir = $sessionDir
    Attempts = $attemptResults
}

$summary | ConvertTo-Json -Depth 10 | Out-File -LiteralPath (Join-Path $sessionDir "summary.json") -Encoding UTF8

Write-Host ("Status: {0}" -f $overallStatus)
Write-Host ("Reason: {0}" -f $overallReason)
Write-Host ("Evidence: {0}" -f $sessionDir)

if ($overallStatus -eq "Pass") {
    exit 0
}

exit 1
