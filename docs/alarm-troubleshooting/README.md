# HT160S 警報／警示 檢修對應手冊

本資料夾提供 HT160S 全機 **576 則**警報／警示的檢修對應手冊，主軸是
「**每個異常 = 某裝置未達預期狀態**」，逐則列出要檢查的 IO／汽缸／馬達／真空點的
**應該狀態（健康）→ 目前狀態（故障）**。

## 檔案

| 檔案 | 用途 |
|---|---|
| [`HT160S_Alarm_Troubleshooting.html`](HT160S_Alarm_Troubleshooting.html) | 互動版（可搜尋代碼／關鍵字、淺深色切換）。用瀏覽器開啟。 |
| [`HT160S_Alarm_Troubleshooting.md`](HT160S_Alarm_Troubleshooting.md) | Markdown 版（可在 GitHub / VS Code 檢視、可貼進其他文件）。 |
| `data/cyl_ref.json` | 55 汽缸的線圈／到位 sensor 位址、逾時、啟用狀態（由兩份 CSV 交叉比對）。 |
| `data/mot_ref.json` | 20 馬達序號↔代碼對照。 |
| `data/wf_out.json` | 60 條流程字串碼 + 馬達 9 錯誤別 + 真空 6 錯誤別的**逐則根因分析**（含觸發條件 file:line、裝置應該/目前狀態）。 |

## 涵蓋範圍

| 代碼族 | 數量 | 說明 |
|---|---|---|
| 汽缸 `4xxxx` | 55 × 6 = 330 | 到位逾時（`…3` 伸出失敗 / `…0` 縮回失敗為現行實際觸發碼） |
| 馬達 `5xxxx` | 20 × 9 = 180 | 伺服電源／扭力／CW-CCW 硬限／軟限／位置誤差 等 |
| 真空 `6000x` | 1 × 6 = 6 | 分類手臂吸嘴組 `SortArmSuck` |
| 流程 `JAM/MES/WAR` | 60 | 盤流／視覺／計數；Auto1~6 六站同模式，文件以 6 種樣式呈現 |

## 資料來源與可信度

- **汽缸／馬達位址表**：由 `system/AlarmList.csv`（開機時由程式 `mapAlarmCodeList` 匯出）與
  `system/IO_Table.csv` 以 **Alias 名稱**交叉比對產生，為機器實際載入值。
- **流程碼根因分析**：逐則回讀原始碼觸發點（`aLoader.cpp` / `aEmpty.cpp` / `aColor.cpp` /
  `aAuto1To6.cpp` / `aTrayArm.cpp` / `aSortArm.cpp` 及 `database.cpp` 註冊處），並經第二輪
  對抗式複驗。文件內每則標示驗證狀態：
  - ✅ **已驗證 (CONFIRMED)** — 觸發條件與裝置狀態與原始碼一致（17 則）。
  - ✳️ **已校正 (CORRECTED)** — 初稿有誤，複驗時已依原始碼修正（13 則）。

## 重要判讀提醒

- 手冊列出的位址格式 `L環號/IP/P埠/b位元` 對應 MotionNet `gMnGetPortBit`；
  但 **IOsetview hover 顯示的位址對 IP/Port 另有編碼轉換**，現場請以 **Alias 名稱**搜尋為準，位址僅作輔助。
- `InType` 極性（0=常開 / 1=常閉）務必對照後再判讀 sensor 的 ON/OFF 意義。
- 部分汽缸的到位 sensor 在 IO_Table 內為**停用**（例如 TrayArm 系列無縮回 sensor、
  分離爪系列無到位 sensor）；這些行程靠逾時但不發到位警報，別去找不存在的 sensor。

## 重新產生

`AlarmList.csv` / `IO_Table.csv` 變動後，汽缸／馬達表可用擷取腳本重建（比對 code↔sensor↔位址）；
流程碼根因分析源自一次性的原始碼分析工作流，其結果快取於 `data/wf_out.json`，
若模組觸發邏輯有大改，需重跑該分析以更新。生成日期：2026-07-13。
