# HT160S 計數器/長時間生產審計 — 死碼清理 + 誤導註解 TODO 計畫

- 日期：2026-07-14
- 觸發：使用者關切「長時間、多數量生產下避免 (a) 整數溢位、(b) Lot Start 初始化涵蓋、(c) 記憶體/磁碟無界成長」，以 `tRunData.iAutoSkipCount` 為例。
- 方法：10-agent 對抗式 workflow（TRunData 計數器 / 其他全域計數器 / RAM 成長 / 磁碟 log / Lot-init+SECS 五線，各 investigate+verify），全部行號已人工複核。
- 狀態：**本 session 未改任何 code**（使用者決定先記錄）。此文件是待辦清單。

> 本文件為 UTF-8。實作時務必遵守 build gate：改 `.cpp/.h` 先刪對應 `.obj` 再編；`cprod.h` 是**共用標頭**，動它要 `scripts/ops/build-ht160s.ps1 -Full`（layout 變更需全體 TU 重編）。改 Big5 legacy 檔勿用會破壞編碼的工具。

---

## 0. 審計總結論（先講重點，避免之後重審）

- **整數溢位：實質無風險。** 所有「活的」計數器都在 `ResetPerLotProductionCounters()` 每 Lot 歸零，上限＝單一 Lot 產量，遠不到 int32（2,147,483,647）。最高量的 `TotalIC` 要單一 Lot 連跑 ~6 年不下 Lot End 才會滿。`iAutoSkipCount` 因每 Lot 歸零而安全。
- **RAM：有界。** work-order 結束即 `LotRegistry.Clear()/LotBinBinding.Clear()`（`main.cpp:2452-2456`）；產線熱路徑（`aLoader OnSorted`）不新增 RAM；其餘皆固定陣列或 hard-cap ring buffer。
- **真正的長時間風險＝磁碟 log 無界成長**（見 §3，本次未實作）。
- 以下 §1 死碼、§2 誤導註解為使用者本次指定要「記錄」的兩塊；§3 為本次未採納但應保留的功能性 TODO。

---

## 1. 死碼清理 TODO（宣告了但從未被寫入/累加）

清理動作二選一：**刪除**（含其 decl/init/reset 全部參照），或若打算未來實作則**明確標註 `// NOT IMPLEMENTED`**。優先刪除以免誤導。

### 1a. `TRunData` 死欄位 — `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\cprod.h` / `cprod.cpp`

| 欄位 | 參照（絕對路徑省略前綴 `...\cprod`） | 狀態 |
|---|---|---|
| `LoaderIC` | `.h:18` decl、`.h:42` Clear=0、`.cpp:145` reset=0 | 無任何 `++`／`+=`，恆為 0 |
| `iPauseTime` | `.h:20` decl、`.h:44` Clear=0 | 從未寫入；且不在 per-lot reset（因死碼故無害） |
| `JamCount` | `.h:21` decl、`.h:45` Clear=0、`.cpp:146` reset=0 | 無 `++`；HT172 jam 追蹤未移植 |
| `iEjectionPinCT` | `.h:23` decl+誤導註解、`.h:49` Clear=0 | 無 `++`；HT172 tester 遺留（見 §2-1） |
| `iTotalQuantity` | `.h:27` decl、`.h:53` Clear=0 | 從未寫入**且從未讀取**（`GetTotalQuantity()` 用 `iAuto+iMag`） |
| `iAutoQuantity` / `iMagQuantity` | `.h:28-29` decl、`.h:54-55` Clear、setter `.h:58-59` | setter `SetAutoQuantity/SetMagQuantity` **從未被呼叫** → 恆 0；被 `GetTotalQuantity()`/`GetTotalQuantityAutoPercent()` 讀（永遠得 0）。半死：讀但不寫。決定要不要接。 |
| `JamRate` (double) | `.h:25` decl、`.h:51` Clear=0.0 | Clear 以外從無 producer。（`JamRateDenom` 是活的常數 10000，**保留**） |

### 1b. `MACHINE_RUN_INFO` 死欄位 — `...\cprod.h` / `cprod.cpp`

| 欄位 | 參照 | 狀態 |
|---|---|---|
| `iAreaCount[eTrayCount]` | `.h:201` decl+誤導註解、`.cpp:43` Clear=0、`.cpp:139` reset=0 | 只有 `=0`、**無 `++`**（verify 已更正另一 agent「sort 時累加」的錯誤說法） |
| `iSystemStatus` | `.h:196` | 審計標為未維護 — **移除前請再 grep 確認**（各線略有分歧） |
| `iUPH` | `.h:197` | 同上，未維護 — **移除前再確認** |
| `iActiveLotCount` | `.h:198` | **可能是活的**（來源 `THT160LotRegistry::m_LotCount`，`CosFunction.cpp:1012`）— 勿誤刪，先確認 |

### 1c. `RUN_INFO RunInfo / RunInfo2` 整組 — HT172 tester 遺留死碼

- struct 定義 `...\cprod.h:160-186`；物件定義 `...\cprod.cpp:29-30`；extern `...\cprod.h:185-186`。
- 全專案**零寫入點**（含 yield 陣列 `iYieldChart[8][25]`、`iYieldHour[25]`、`iYieldMin[25]`、`iYieldCount[25]`、`iYieldChartByCount[8][25]`、`iTestCT[8]`、`iHeadPass[8]`、`iLoad`、`iTotal`、`iAuto[3]`、`iFix[3]`、`iUPH`、`iUpdateCount` 等）。
- 動作：整組刪除（連同任何殘留 include）。這是最大的 HT172 遺留死碼。

### 1d. `TLatchCycleTime lctLoader` — 死類別

- class `...\cprod.h:81-90`；實例 `...\cprod.cpp:18`；method 定義 `...\cprod.cpp:57`。
- `LatchCycleTime()` **全庫無呼叫點** → 其內部 `iCount++` 為死碼。
- 動作：刪除整個類別 + `lctLoader` 實例。

### 1e. `cStepTrace s_TickNo` — 唯一「永不歸零的全域 int」（非死碼，屬 hardening）

- `...\cStepTrace.cpp:20`（`static int s_TickNo=0`）、`:136`（`s_TickNo++`）、`:159`（寫入 row）。
- StepTrace 為 opt-in 診斷；理論上連續開啟多年才會 wrap。優先度低。
- 動作（可選）：改 `unsigned`，或在 trace 檔（重）開時 reset。

---

## 2. 誤導註解逐條記錄（內容 + 為何誤導 + 建議修正）

> 使用者要求「誤導註解內容條列出來也記錄」。以下為**目前原文**與問題。

1. **`...\cprod.h:23`**
   - 原文：`int iEjectionPinCT; // Ejection pin count`
   - 問題：暗示是一個被維護的計數；實際**從未 increment**。且「ejection pin」是 HT172 **tester** 概念，HT160 是**分選機**，語意也不符。
   - 建議：刪除該欄位；若保留則改 `// HT172-legacy, NOT IMPLEMENTED in HT160S`。

2. **`...\cprod.cpp:127-131`**（`ResetPerLotProductionCounters` 檔頭註解）
   - 原文：`... Leaves configured quantities (iAutoQuantity/iMagQuantity), the lifetime ejection-pin counter and run-state fields (bRunning/iActiveLotCount) to callers.`
   - 問題：把 `iEjectionPinCT` 講成「lifetime 計數器交給 caller 維護」，但**沒有任何 caller 維護它**。
   - 建議：移除「lifetime ejection-pin counter」字樣（隨 §1a 刪欄位一起改）。

3. **`...\maintenance.cpp:2043`**（`chkSortArmAutoSkipClick` 檔頭；來自 auto-skip 功能 commit 9f7a74b）
   - 原文：`... read live by aSortArm each pick, page locked mid-lot.`
   - 問題：「page locked mid-lot」不實 — `chkSortArmAutoSkip` **未**加入 `ApplyHardwareEditLock` 的 `Locked[]` 陣列，量產中仍可勾動（與 `GeneralSetting.h` 的「read live 免重啟」設計其實一致）。
   - 建議：把「page locked mid-lot」改為「editable mid-lot (read live)」；或若真要鎖，把 `chkSortArmAutoSkip` 加進 `Locked[]` 並 bump 迴圈上界。（行為決策待定。）

4. **`...\aSortArm.cpp:1490`**（`TotalIC` 累加點註解）
   - 原文：`//unit so tRunData.TotalIC (SECS SVID 1120/66020) and the derived UPH are`
   - 問題：SVID **1120 從未註冊**（`SetSVDataPointer` 只註冊 66020）；stale 參照。
   - 建議：改為僅 `SVID 66020`。

5. **`...\cprod.h:201`**（`MACHINE_RUN_INFO.iAreaCount`）
   - 原文：`int iAreaCount[eTrayCount]; // sorted count per output area`
   - 問題：暗示是活的 per-area 計數；實際**從未 increment**（見 §1b）。
   - 建議：隨欄位刪除，或標 `// NOT IMPLEMENTED`。

6. **`...\cprod.h:196-197`**（`MACHINE_RUN_INFO.iSystemStatus` / `iUPH`）
   - 問題：註解暗示為活的執行狀態/UPH；審計標為未維護。
   - 建議：**移除前再 grep 確認**後，隨欄位刪除或標註。

---

## 3. 本次未採納、但應保留的功能性 TODO（分析已完成）

> 使用者本輪只要求記錄 §1/§2；以下為審計發現、但這次決定**不實作**的項目，一併記錄以免遺失。動這些前請重新確認共用工作樹（別的 session WIP）。

### 3a. 🔴 HIGH — `Production_Log` 沒有保留/清除策略
- `...\deviceinfo.cpp`：每顆 IC（含 **auto-skip reject**）一行；`:247` `AppendLine`（AddOutputInfo）、`:266`（SaveRejectRecord）。新檔 per lot（`:65`），groupby `<yyyymm>` 月資料夾，**從不 prune**。
- 量級：~240k IC/日 ≈ 21.6 MB/日 ≈ **7.9 GB/年**，永久累積。
- 直接連動：auto-skip 功能（commit 9f7a74b）每次 skip 就往這裡寫一行 → 「要有紀錄」與「長時間不爆」匯集於此。
- 建議：新增 `[Log] ProdLogRetention*Days`（如 365），比照 `cCsvDailyLog::PruneFolderTree` 清 `<yyyymm>`；開機 + day-rollover 各跑一次。

### 3b. SECS LOTSTART 與手動 lot-init 不對齊（對應使用者 Q2）
- 手動 `btnLotStartClick`（`...\main.cpp:2203`）會做 `LotBinBinding.Clear()/SaveToIni()` + 設 `MachineRun.bRunning/iActiveLotCount`；SECS LOTSTART（`...\SecsGem\uHGemHT160.cpp:859`）**沒有**。
- 風險：host 觸發的 lot 繼承殘留 (Lot,Bin) 綁定、run-state 不同步。
- 建議：抽共用 `DoLotStartInit()`，兩條路徑都呼叫。

### 3c. 其他無 prune 的磁碟 sink（verify 補抓）
- Top/Color CCD `2D_Logs`：`...\TopCcdSocket.cpp:364`（per 2D 讀）、`...\ColorCcdSocket.cpp:362`。
- MyComm bin-display CSV：`...\MyComm.cpp:364`（`fopen("a")`，EnableLog 預設 true）。
- AutomationServer startup log：`...\AutomationServer.cpp:75`（單一檔永久附加）。
- SECS_GEM text/err log：`...\SecsGem\uHGemEquipment.cpp:1211`（SECS file-log 開時）。
- 建議：各自套 `PruneFolderTree` 或改走 `cCsvDailyLog`。

**⚠ 本次稽核漏抓的一項（20260901 補登，已修）**
- **State Record 快照本體**：`D:\HT160S_StateRecord\*.zip`。本節當時只登記了 `_ldj_trace.txt`（見 3e），
  **沒有登記 zip 本身**，而那才是量級最大的一項：`TriggerSnapshot()` 完全沒有 retention / 去重 / 上限，
  且每顆快照都重新複製「當天全部」的 SECS + Production + Soter + EventLog，**同一天內是二次成長**。
  20260831 現場實測：第一台一天 8 顆、第二台 6 顆（`HomeResumeDone`×5 / `StuckWatchdog`×3 等）。
  另有 `CompressFolder` 失敗時刻意保留的**未壓縮資料夾**——客戶機沒裝 7-Zip 就從第一天開始累積。
- **已修（20260901）**：新增 `[LogRetention] StateRecordDays`（預設 90，0=永久保留），
  `cStateRecordHT160::PurgeOldSnapshots()` 同時清 zip 與殘留資料夾；
  只刪符合 `YYYY-MM-DD HH_MM_SS` 戳記的檔名（工程師手動改名的檔案永不觸碰）。
  **刻意避開 3d 的缺陷**：開機掃一次之外，**每次快照後再掃一次**，所以永不重開機的機台也會清。
  同時補上 7-Zip 缺失的開機警告與失敗原因（見下方 3f）。

### 3f. 7-Zip 缺失沒有任何提示（20260901 補登，已修）
- `CompressFolder` 找不到 7z.exe 就直接回 false，而 `TriggerSnapshot` **只在成功時刪暫存資料夾**，
  所以沒裝 7-Zip 的機台每顆快照都留一個未壓縮資料夾（比 zip 大 3–10 倍），且 EventLog 只寫 `FAILED` 不說原因。
- **已修（20260901）**：`EnsureInited` 開機檢查一次並寫一行 WARNING；
  `SNAPSHOT ... FAILED` 現在會分辨「7-Zip 未安裝」與「這一次壓縮失敗（磁碟滿 / 逾時）」。
- **仍未修**：`CompressFolder` 的 `WaitForSingleObject(..., 60000)` 逾時後**不會 `TerminateProcess`**，
  會留下孤兒 7z 行程。已知，未處理。

### 3d. cCsvDailyLog 家族 prune 僅開機時跑
- EventLog/UPHLog/PadLog/… 有 90/180/365 天保留，但 prune 在**啟動時**執行 → 永不重開機的機台不會清。
- 建議：改成 day-rollover（或 Lot End）也觸發 prune。

### 3e. 暫時碼 `_ldj_trace.txt`（**屬別的除錯任務、勿由本任務刪**）
- `...\cStateRecordHT160.cpp:27-39` 的 `SR_Trace`（`AI(diag-av) TEMP ... REMOVE after fix`）永久附加到 `D:\HT160S_StateRecord\_ldj_trace.txt`。
- 這是 WriteLotDataJson access-violation 追查用的臨時 breadcrumb。**待該 AV 修好後由該任務移除**，此處僅登記。

---

## 4. 已驗證安全（免再審）

- 每 Lot 歸零、overflow-safe：`TotalIC`、`iAutoSkipCount`、`BinICCnt[]`、`TrayICCnt[]`、`UPH`（計算值）、`MachineRun.iTotalScanned/iTotalSorted/iUnknown2D`。
- LotRegistry：`iSortedQty`（per-lot）、`m_LotCount`（綁 [64] slot）、`iBinCount[]`（固定陣列），皆有界。
- SECS SystemByte 產生器 unsigned（定義好的 wrap）或有守衛；reconnect 計數 capped 1e9。
- RAM 容器：`g_UphRecentRows[UPH_ROW_MAX]`（hard-cap）、State-Record TaskHistory、SECS log list（dedup-guarded）、deviceinfo `m_records[4]`（固定）— 全部有界。

---

## 5. 建議實作順序（若日後動工）

1. §3a Production_Log 保留（最高值，連動 auto-skip）。
2. §3b SECS lot-init 對齊（正確性）。
3. §1 死碼刪除 + §2 註解修正（同一批，純 hygiene；因動 `cprod.h` 共用標頭 → `-Full`）。
4. §3c/§3d 其餘磁碟 sink prune。
5. §1e cStepTrace wrap-safe（最低）。
