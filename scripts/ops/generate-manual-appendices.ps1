# generate-manual-appendices.ps1
# Regenerate docs/manual appendix tables from the machine config CSVs:
#   B1-io-table.md    <- system/IO_Table.csv
#   C1-motor-table.md <- system/Mot_Table.csv
#   D1-alarm-list.md  <- system/AlarmList.csv
# Output is UTF-8 (no BOM). Re-run whenever the CSVs change.
param(
    [string]$RepoRoot = "D:\HT160S_BCB"
)
$ErrorActionPreference = "Stop"
$SystemDir = Join-Path $RepoRoot "system"
$ManualDir = Join-Path $RepoRoot "docs\manual"
$Utf8NoBom = New-Object System.Text.UTF8Encoding($false)
$Today = "2026-07-16"

function Escape-MdCell([string]$s) {
    if ($null -eq $s) { return "" }
    $t = $s -replace '\|', '\|'
    $t = $t -replace '\\r\\n', '；'          # literal \r\n sequences in alarm CSV
    $t = $t -replace "`r`n", '；' -replace "`r", '；' -replace "`n", '；'
    return $t.Trim()
}

# ---------- C1 : motor table ----------
$mot = Import-Csv (Join-Path $SystemDir "Mot_Table.csv")
$c1 = New-Object System.Collections.Generic.List[string]
$c1.Add("# 附錄 C　軸（馬達）對照表")
$c1.Add("")
$c1.Add("資料來源：``system/Mot_Table.csv``（機台設定檔，$Today 重生；最終值以機台 State Record 內 system 副本為準）。")
$c1.Add("")
$c1.Add("| 馬達 | Alias 軸名 | 啟用 | 方向 | 原點方向 | 回原高速 | 回原低速 | 原點順序 | 軟極限 N | 軟極限 P | 初速 | 加速 | 減速 | 範圍 | 卡/板/埠 |")
$c1.Add("| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |")
foreach ($m in $mot) {
    if ([string]::IsNullOrWhiteSpace($m.Motorname)) { continue }
    $en = if ($m.Enable -eq "1") { "是" } else { "否(停用)" }
    $ho = if ([string]::IsNullOrWhiteSpace($m.HomeOrder)) { "-" } else { $m.HomeOrder }
    $c1.Add(("| {0} | {1} | {2} | {3} | {4} | {5} | {6} | {7} | {8} | {9} | {10} | {11} | {12} | {13} | {14}/{15}/{16} |" -f `
        $m.Motorname, $m.Alias, $en, $m.Direction, $m.HomeDirectior, $m.HomeHighSpeed, $m.HomeLowSpeed, $ho, `
        $m.SoftLimitN, $m.SoftLimitP, $m.InitSpeed, $m.Acc, $m.Dec, $m.Range, $m.CardModel, $m.BoardID, $m.Port))
}
$c1.Add("")
$c1.Add("> 註：位置/極限單位為 1/100mm（100 units = 1mm）。M13 (MBottomCCDY)、M18 (MPitchX) 目前 Enable=0（停用），回原點時不參與，屬正常設計。``HomeOrder`` 欄目前全空白——回原順序由程式內建順序決定，非本 CSV 控制。加速/減速（Acc/Dec）僅能於本 CSV 修改，無 UI 畫面可調（見第 9 章 9.6）。")
[System.IO.File]::WriteAllText((Join-Path $ManualDir "C1-motor-table.md"), ($c1 -join "`n") + "`n", $Utf8NoBom)
Write-Host ("C1 done: {0} axes" -f $mot.Count)

# ---------- B1 : IO table ----------
$io = Import-Csv (Join-Path $SystemDir "IO_Table.csv")
$b1 = New-Object System.Collections.Generic.List[string]
$b1.Add("# 附錄 B　全機 I/O 對照表")
$b1.Add("")
$b1.Add("資料來源：``system/IO_Table.csv``（機台設定檔，$Today 重生）。位址格式 Lane/IP/Port/Bit（IP=W 為寫出型點位）。IO_Table.csv 無中文標籤欄，畫面顯示名稱由程式以「前綴＋Alias」慣例產生。最終位址請以機台 State Record 內 MachineConfig\system 副本核對（repo 工作副本與機台副本可能 drift）。")
$b1.Add("")
$groupOrder = New-Object System.Collections.Generic.List[string]
foreach ($row in $io) {
    if (-not $groupOrder.Contains($row.IOType)) { [void]$groupOrder.Add($row.IOType) }
}
foreach ($g in $groupOrder) {
    $rows = @($io | Where-Object { $_.IOType -eq $g })
    $b1.Add(("## {0}（{1} 點）" -f $g, $rows.Count))
    $b1.Add("| Alias | Lane | IP | Port | Bit | InType | 啟用 | OnDelay | OffDelay | Note |")
    $b1.Add("| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |")
    foreach ($r in $rows) {
        $en = if ($r.Enable -eq "1") { "是" } else { "否" }
        $b1.Add(("| {0} | {1} | {2} | {3} | {4} | {5} | {6} | {7} | {8} | {9} |" -f `
            (Escape-MdCell $r.Alias), $r.Lane, $r.IP, $r.Port, $r.Bit, $r.InType, $en, $r.OnDelayTime, $r.OffDelayTime, (Escape-MdCell $r.Note)))
    }
    $b1.Add("")
}
[System.IO.File]::WriteAllText((Join-Path $ManualDir "B1-io-table.md"), ($b1 -join "`n") + "`n", $Utf8NoBom)
Write-Host ("B1 done: {0} points in {1} groups" -f $io.Count, $groupOrder.Count)

# ---------- D1 : alarm list ----------
$al = Import-Csv (Join-Path $SystemDir "AlarmList.csv")
$numeric = @($al | Where-Object { $_.AlarmCode -match '^\d+$' })
$jam = @($al | Where-Object { $_.AlarmCode -like 'JAM*' })
$mes = @($al | Where-Object { $_.AlarmCode -like 'MES*' })
$war = @($al | Where-Object { $_.AlarmCode -like 'WAR*' })
$d1 = New-Object System.Collections.Generic.List[string]
$d1.Add("# 附錄 D　警報碼一覽")
$d1.Add("")
$d1.Add(("資料來源：``system/AlarmList.csv``（開機由 ``CreateSystemAlarmCode()`` 依警報登錄表 ``mapAlarmCodeList``（SSOT）產生，$Today 重生）。共 {0} 筆＝動態數字碼 {1}（氣缸/馬達/吸嘴）＋ JAM {2} ＋ MES {3} ＋ WAR {4}。" -f $al.Count, $numeric.Count, $jam.Count, $mes.Count, $war.Count))
$d1.Add("")
$d1.Add("> 註（2026-07-16 查核）：中文欄 ``C_ErrMessage``/``C_Description`` 目前仍為英文（與 E_ 欄相同）；中文化為待辦工作。下表顯示英文訊息與排除說明。操作員排除手冊另見 ``docs/alarm-troubleshooting/``。")
$d1.Add("")
$d1.Add("| 警報碼 | 類型 | 訊息 (EN) | 原因/排除 (EN) |")
$d1.Add("| --- | --- | --- | --- |")
foreach ($a in $al) {
    $d1.Add(("| {0} | {1} | {2} | {3} |" -f `
        (Escape-MdCell $a.AlarmCode), (Escape-MdCell $a.AlarmType), (Escape-MdCell $a.E_ErrMessage), (Escape-MdCell $a.E_Description)))
}
[System.IO.File]::WriteAllText((Join-Path $ManualDir "D1-alarm-list.md"), ($d1 -join "`n") + "`n", $Utf8NoBom)
Write-Host ("D1 done: {0} alarms" -f $al.Count)
