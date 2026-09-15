---
description: "京元（或其他 HT160S 客戶）新回報/出貨後重啟異常：建立或重啟 case、登週報、歸檔，並回報異常資料存放路徑（01_intake）。"
argument-hint: "例如：京元 反應 HT160S 異常，如附圖"
---

> 交給 **weekly-report** 子代理執行（Task 工具）。建 case / 歸檔屬破壞性動作，執行前先說明再做。

請用工具優先方式處理客戶新回報或重啟異常。操作共用 Weekly_AI 工作區（根目錄 `d:\Work-jimmychiu\document\WeeklyReport\Weekly_AI`）。

使用者輸入：$ARGUMENTS

## 必守原則
1. `weekly_data.json` 是唯一週報資料來源（HT9045/HT172/HT160S 共用）。
2. CASE 長期主鍵是 `(customer, case_id, path)`；`row` 只當目前定位。
3. 同客戶、同功能、同出貨版本回報 NG → 優先**重啟既有 case**，不直接新開。
4. 原始客訴資料放 `01_intake/`；推論與 root cause 假設放 `02_analysis/`。
5. **最後完成時，必須回報「放異常資料的資料夾」的完整絕對路徑**（該 case 的 `01_intake/`，以 `d:\...` 開頭），讓使用者可直接開啟並把原始截圖/log/State Record 放入。
6. **客戶名稱鎖定**：只用使用者原話逐字的客戶名稱。本 repo 的 git log/branch 可能出現 HT9045/HT172 的客戶（力成PTI、甬矽…）——**那是不相關的上下文，不可帶入**。HT160S 端主客戶為「京元竹南」。委派時 prompt 裡只能出現 Step 0 解析出的精確客戶名稱。
7. **每筆異常 = 一列 weekly item + 一個 case（Weekly_AI ADR-008，2026-09-15 起）**。京元 HT160S 已是量產機；「京元竹南 / HT160S / HT160S 量產維護（非客訴開發紀錄）」列**不是** case、不可 reuse、不可 append 客訴。
8. **HT160S 專屬欄位必填**（區隔 9045/172）：`--serial`（京元機台編號 / 交機序 `KYEC-0N`，不得 Default）、`--component`（HT160S 模組字彙：Loader/Empty/Color/TrayArm/SortArm/Auto/AMR/SECS/WebAPI/TopCCD/ColorCCD/BinDisplay/Soter/StateRecord/HOME/Panel/Motion）、`--tags HT160S,<模組>,KYEC`、`--category` B/P/R/E/Q、`add --version "部署YYYYMMDD"|"待確認"`。規則見 `weekly-case-flow` skill「HT160S 專屬欄位定義」。

## 流程
0. 客戶名稱解析（精確比對，不可省略）：`cd tools && python _customers.py "<使用者原話客戶名稱逐字>"` → 取得 `RESOLVED` 正式名稱才能往下；`UNKNOWN` 先跟使用者確認，不可自行猜測代換。
1. 解析：機型（HT160S）、版本、問題、日期、附件描述（客戶名稱已在 Step 0 鎖定）。
2. 查既有：`weekly_data.json`（僅用 Step 0 鎖定的正式客戶名稱 + HT160S + 關鍵字/版本/CASE-ID）與 `Customer/<客戶>/`。
3. **先說明**將新增或重啟哪一筆、要建哪個 case 資料夾（京元首次歸檔會建 `Customer/京元竹南/`，先與使用者確認命名），經我同意後：
   ```
   cd /d/Work-jimmychiu/document/WeeklyReport/Weekly_AI/tools
   # 1) 新增 weekly 列（title=現象+條件+影響；--desc 詳細 / --brief 白話，雙層制）
   python update_report.py add --customer 京元竹南 --machine HT160S --version "部署YYYYMMDD" --title "<title>" --desc "<詳細>" --brief "<白話>"
   # 2) 先 generate_report.py 定案 row → list_open.py 取最終 row（順序鐵律）
   python generate_report.py && python list_open.py
   # 3) 用最終 row 建 case（HT160S 專屬欄位必填）
   # 有原始檔：
   python archive_issue.py <最終row> "<source_file>" --desc "<短描述>" --expect-customer "京元竹南" --category B --serial KYEC-0N --component <模組> --tags HT160S,<模組>,KYEC --severity P1
   # 尚無檔案（只有圖片描述）：
   python archive_issue.py <最終row> --skeleton-only --desc "<短描述>" --expect-customer "京元竹南" --category B --serial KYEC-0N --component <模組> --tags HT160S,<模組>,KYEC --severity P1
   ```
4. 重產 Excel：`python generate_report.py`
5. 只有圖片描述時，建立 `01_intake/YYYYMMDD_<slug>_summary.md` 摘要，提醒我把原始截圖/log/State Record 放同夾。
6. 驗證：`python list_open.py`、`python check_case_integrity.py`

## 回覆必含

| 欄位 | 內容 |
|---|---|
| Row | weekly_data.json row |
| Case | CASE-ID（京元＝`CASE-KYEC_CHEN-YYYYMMDD-NNN`） |
| Case 路徑 | `Customer/<客戶>/<CASE>/` |
| 異常資料放置路徑（絕對路徑，**必填**） | `d:\Work-jimmychiu\document\WeeklyReport\Weekly_AI\Customer\<客戶>\<CASE>\01_intake\` |
| 分析筆記路徑 | `Customer/<客戶>/<CASE>/02_analysis/` |
| 狀態 | new / in-progress / done |
| 驗證 | `check_case_integrity.py exit=<N>` |

若 `check_case_integrity.py` 出現 `[FAIL]`，不可宣稱完成，需回報實際 fail reason。

> ✅ 收尾規則：回覆最後一行必須明確給出「異常資料放置路徑」的**絕對路徑**（`01_intake/` 完整 `d:\...` 路徑）。

> 後續我把異常檔放進 `01_intake/` 並說「開始分析」→ 交回 **HT160S 開發 session 本體**做根因分析（不另設分析子代理）。
