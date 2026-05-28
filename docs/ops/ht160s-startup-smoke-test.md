# HT160S Startup Smoke Test

## 目的

`scripts/ops/test-ht160s-startup.ps1` 用來補足「編譯成功但軟體開不起來」這一段驗證缺口。

它會執行：

1. 編譯 HT160S。
2. 讀取 `ht160s.bpr` 的 `<PROJECT value="...">`，啟動目前指定的 BCB 版執行檔，例如 `HT_160s.exe`。
3. 在指定秒數內監控是否有例外視窗或程式提早結束。
4. 透過 Automation socket 執行 `PING` 探測。
5. 選擇性執行首頁 top form 探測，逐一建立並短暫開啟 Language / Product / Maintance / Offset / Speed / Tools / Message。
6. 蒐集視窗快照、Windows Application event log、process 狀態與 build/run log。
7. 產出每次嘗試的 evidence folder。

## 執行方式

```powershell
cd D:\HT160S_BCB
powershell -ExecutionPolicy Bypass -File .\scripts\ops\test-ht160s-startup.ps1 -Clean -StartupSeconds 10
```

包含首頁 top form 探測：

```powershell
powershell -ExecutionPolicy Bypass -File .\scripts\ops\test-ht160s-startup.ps1 -Clean -StartupSeconds 10 -ProbeTopForms
```

常用參數：

- `-Clean`：先用既有 `build-ht160s.ps1 -Clean` 編譯。
- `-StartupSeconds 10`：啟動後觀察 10 秒，程式仍存活且未偵測到異常視窗才算通過。
- `-MaxAttempts 3`：重複執行最多 3 次。這只會重跑測試，不會自動修改 source。
- `-KeepProcessOnPass`：通過時保留程式，不自動關閉。預設會關閉或 kill 測試啟動的 process。
- `-ProbeTopForms`：啟動通過後透過 Automation socket 呼叫 `SMOKE_TOP_FORMS`，檢查首頁 top 區 lazy-created forms 能否建立與開啟。

## 輸出

每次執行會建立：

```text
logs/startup-smoke/YYYYMMDD-HHMMSS/
```

主要檔案：

- `summary.json`：整體 Pass/Fail、失敗原因、各 attempt 路徑。
- `attempt-xx/build.log`：編譯輸出。
- `attempt-xx/run.log`：啟動測試摘要。
- `attempt-xx/process.json`：process id、exit code、狀態。
- `attempt-xx/automation-probe.json`：Automation socket `PING` 探測結果；使用 `-ProbeTopForms` 時也會包含 top form probe 回覆。
- `attempt-xx/windows-before.csv` / `windows-after.csv`：測試前後可見視窗快照。
- `attempt-xx/matched-windows.csv`：命中例外關鍵字的視窗。
- `attempt-xx/application-events.txt`：相關 Windows Application event log。

## 自動修正迴圈

這個腳本只做「可重複驗證與蒐證」，不直接改 source。實際迴圈由 AI agent 或工程師執行：

1. 跑 startup smoke test。
2. 讀 `summary.json` 與 attempt evidence。
3. 依 evidence 修改 HT160 source。
4. 重跑編譯與 startup smoke test。
5. 重複直到 `Status = Pass`。

這樣可以避免腳本在沒有工程判斷時亂改機台控制程式。
