# HT160S Soter 分類報表 — 客戶抓取路徑與時機說明

> 對象：KYEC（京元）資料抓取端
> 版本：草稿 v0.2（2026-08-08）— 待設備商確認上機驗證後定版
> 設備：HT160S IC 分類機（Sorter）
>
> **v0.2 變更**：①新增 §6「虛擬 2D 防護」——測試／模擬情境產生的報表不再進入交付資料夾（已實裝）；
> ②新增 §7「修訂提案」——寫掉（write-off）列的標示方式與 Qty 語意，**待京元確認後才會實裝**。

---

## 1. 目的

HT160S 於**每一個 Lot 結束**時，會將該 Lot 的**逐顆分類結果**輸出成一份 CSV 報表，放在機台本機的一個**專屬交付資料夾**，供 KYEC 端軟體自行抓取。

為避免抓取端軟體異常影響機台自身紀錄，機台採「**雙資料夾**」設計：

- **交付資料夾（本文件對象）**：只放「當前 Lot」一份檔，供客戶抓取。每次 **Start Lot 會清空**。
- **封存資料夾（機台內部）**：機台永久保存每一個 Lot 的報表，客戶不需存取；即使交付資料夾的檔被誤刪，仍可向設備商調閱。

---

## 2. 抓取路徑

| 項目 | 值 |
|---|---|
| 交付資料夾（預設） | `D:\HT160S_Log\SoterPickup\` |
| 可設定 | `系統\General.ini` 的 `[Soter] PickupDir`（留空＝用上列預設；修改後需重啟軟體生效）|
| 檔案型態 | UTF‑8 純文字 CSV，逗號分隔，含標題列 |
| 一個 Lot | 一個檔（平放，無子資料夾）|

---

## 3. 檔名規則

```
{Date}_{Time}_KYEC-LFT_{ProductCode}_{CustomerLotNo}_{KYECLotNo}_BI_{Substage}_{SorterID}_{Qty}.csv
```

範例：

```
20260317_165040_KYEC-LFT_MT3781Q-ZAHJA32-ETTTT-H_A5921.RCS.TEST99_NQ4000NAA1_BI_BI1_XX-01_30.csv
```

| 欄位 | 說明 | 來源 |
|---|---|---|
| Date | 檔案產生日期 `YYYYMMDD`（≈Lot End 時間）| 機台自產 |
| Time | 檔案產生時間 `HHMMSS` | 機台自產 |
| `KYEC-LFT` | 固定字串 | 固定 |
| ProductCode | 產品型號 | 2D Map |
| CustomerLotNo | 客戶批號 | Start Lot 輸入的 Lot |
| KYECLotNo | 京元批號（＝CustomerLotNo）| Start Lot 輸入的 Lot |
| `BI` | 固定字串 | 固定 |
| Substage | 站點 | 2D Map |
| SorterID | Sorter 編號 | 機台設定（General.ini `[MachineIdentity] SerialNo`）|
| Qty | 本檔資料列總數（＝本 Lot 逐顆列數）| 機台自產 |

> 檔名中若含 Windows 不允許的字元（`\ / : * ? " < > |`）會被替換為 `-`。

---

## 4. CSV 內容（15 欄，逐顆一列）

標題列（第一列，固定）：

```
No.,StartTime,FinishTime,ProductCode,Substage,Cust lot,Kyec Lot,Load Cover Tray ID,Unload Cover Tray ID,SorterID,2D ID,RetestCode,Hbin,Sbin,DiePass
```

| # | 欄位 | 說明 |
|---|---|---|
| 1 | No. | 流水號，本檔由 1 起 |
| 2 | StartTime | 該顆 pick 時間 `YYYY-MM-DD HH:NN:SS` |
| 3 | FinishTime | 該顆 place／reject 時間 |
| 4 | ProductCode | 產品型號（2D Map）|
| 5 | Substage | 站點（2D Map）|
| 6 | Cust lot | 客戶批號 |
| 7 | Kyec Lot | 京元批號（＝Cust lot）|
| 8 | Load Cover Tray ID | 進料身分盤 2D |
| 9 | Unload Cover Tray ID | 出料流道身分盤 2D（reject 該列留空）|
| 10 | SorterID | Sorter 編號 |
| 11 | 2D ID | 該顆 2D 碼 |
| 12 | RetestCode | 正/重測碼（2D Map 上游透傳，如 R0）|
| 13 | Hbin | Hardware bin（2D Map）|
| 14 | Sbin | Software bin（2D Map）|
| 15 | DiePass | Die pass（2D Map）|

---

## 5. 抓取時機與流程

1. 機台按下 **Start Lot** → **清空**交付資料夾。
2. 生產進行中，機台逐顆在記憶體累積分類結果。
3. 按下 **Lot End** → 機台**先產生報表檔**寫入交付（與封存）資料夾，**然後才送出 SECS Lot End 事件（S6F11，CEID 12）**。
4. KYEC 端**收到 SECS Lot End（CEID 12）事件後**，即可到交付資料夾抓取該檔（此時檔案已完整落地）。

> **順序保證**：報表檔一定在「SECS Lot End 事件送出之前」就已完整寫入磁碟，故收到事件即可安全抓取，不會抓到寫一半的檔。

---

## 6. 抓取視窗與注意事項

- **抓取視窗**：從「收到 SECS Lot End」起，到「下一次 Start Lot」為止。請於此視窗內完成抓取；下一次 Start Lot 會清空交付資料夾。
- **漏抓可補**：即使錯過視窗，機台封存資料夾仍永久保有該檔，可向設備商調閱。
- **零產出 Lot**：若某 Lot 完全沒有有效 2D 產出（0 顆），仍會產生一個**只有標題列、無資料列**的檔（`Qty=0`），代表「有跑但 0 顆」，並非漏傳。
- **CleanOut（排空）**：CleanOut 收尾**不**送 SECS Lot End 事件，因此不觸發抓取（依需求約定）。
- **SorterID 為空**：若機台尚未於設定填入 Sorter 編號，檔名與第 10 欄的 SorterID 會是空白；請於設備商調機時填入。
- **虛擬 2D 防護（2026-08-08 起）**：機台在測試／模擬情境下（例如工程模擬模式、CCD 停用）產生的
  2D 碼並非實際讀取自 IC。此種 run 的報表**只寫入機台封存資料夾，不會出現在交付資料夾、也不會
  FTP 上傳** —— 交付通道上的每一份檔，其 2D 碼保證來自實際讀取。若某次 Lot End 後交付資料夾沒有
  出現檔案，而機台 EventLog 有 `VIRTUAL_2D` 紀錄，即屬此情況（測試 run，非漏傳）。

---

## 7. 修訂提案（v0.2，待京元確認後實裝）

### 7.1 背景：寫掉（write-off）列目前無法與正常列區分

機台在「自動跳過取料失敗」（auto-skip）模式下，會把吸不起來的 IC **寫掉**：該顆實體仍留在
進料 Tray 上，並未被分類到任何出料流道。依 v0.1 規格，這種顆仍會在報表中佔一列
（§4 第 9 欄註明「reject 該列留空」），且檔名 Qty 計入該列。

實務問題（2026-08-06 實測資料）：在**尚未使用身分盤**的現況下，正常列的第 9 欄同樣是空白，
因此寫掉列與正常列**逐欄完全相同**，唯一線索是 `StartTime == FinishTime`（弱訊號）。
當日一份實際檔案：10 顆真實分類 + 3 列寫掉 → 檔名標 `_13`，與實際交付數不符，
且抓取端無法辨認哪 3 列未實際產出。

### 7.2 提案內容

| 項目 | 現行（v0.1） | 提案 |
|---|---|---|
| 寫掉列第 9 欄（Unload Cover Tray ID） | 留空 | 固定字串 **`SKIPPED`** |
| 檔名 Qty | 列數（含寫掉列） | **不變**（仍為列數；寫掉列已可由第 9 欄辨認，抓取端可自行對帳） |
| 其他 14 欄 | — | 不變（第 2/3 欄時間戳相同的特徵仍保留） |

選 `SKIPPED` 而非 `NA` 的原因：第 7 欄已用 `NA` 表示「無值」，若第 9 欄也用 `NA` 將與
「正常分類、但該流道未使用身分盤」的空值情境混淆；`SKIPPED` 為不可能出現的身分盤 2D 值，
語意唯一。

### 7.3 請京元確認

1. 抓取端 parser 對第 9 欄出現 `SKIPPED` 字串是否相容（或希望改用其他哨兵值）？
2. Qty 語意維持「列數」是否可接受？（替代方案：Qty 只計實際交付數，但將造成 Qty ≠ 列數。）
3. 是否希望寫掉列**完全不出現**在交付檔？（機台端封存與 Production_Log 仍會保留完整紀錄。）

確認前，機台行為維持 v0.1（寫掉列留空、Qty 計列數）。

---

*本文件描述 HT160S 端的輸出行為與交付方式；SECS 事件對接細節依雙方 SECS/GEM 介面文件為準。*
