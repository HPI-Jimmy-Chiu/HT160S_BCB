---
name: weekly-case-flow
description: >
  HT160S 端週報 / 客戶異常案件處理流程技能（Hub 模式，操作共用 Weekly_AI 工作區）。
  涵蓋 京元(KYEC) 等客戶異常 → 週報登錄 → Case 歸檔 → 看板總覽的鏈路。資料與工具在
  d:\Work-jimmychiu\document\WeeklyReport\Weekly_AI（HT9045/HT172/HT160S 共用）。
  適用關鍵字：客戶異常、log 歸檔、case、CASE-ID、週報新增、看板、case registry、京元、京元竹南、KYEC。
applyTo: "**/*"
---

# weekly-case-flow（HT160S 端）— Weekly_AI 異常案件處理流程

> 本 skill 是「HT160S 接入共用 Weekly_AI 週報系統」的 SOP。工具與資料**不在本 repo**，而在共用工作區
> `d:\Work-jimmychiu\document\WeeklyReport\Weekly_AI`（與 HT9045 / HT172 共用同一份 `weekly_data.json`）。
> 實際操作交給 `.claude\agents\weekly-report.md` 子代理，用 Bash `cd` 進去跑 Python。
>
> **與 HT9045 原版的差異（重要）**：
> - 機型固定 **HT160S**；主要客戶 **京元竹南 (KYEC_CHEN)**。
> - **建案粒度與 9045 相同（ADR-008，2026-09-15 起）**：每筆客訴 = 一列 weekly item + 一個 `CASE-KYEC_CHEN-YYYYMMDD-NNN` + 一個 case 資料夾。
>   京元 HT160S 已是**量產機**（現場 3 台）；原「機台開發」列已改名「HT160S 量產維護（非客訴開發紀錄）」，只收非客訴工作，**禁止**再把客訴 append 進去。
> - **`make_release_note.py` / `make_proposal.py` / `close_case.py` 為 HT9045 專用**，
>   寫死掃 `d:\HT9045\HT9011UC_Code_V*`，對 HT160S 會失敗——**HT160S 不呼叫這三支**。
>   安裝包走 `ht160s-installer` skill / NSIS updater。
> - **release note 已接（2026-09-17）**：原稿 `.md` 寫在 `D:\HT160S_BCB\docs\release\`，用
>   `scripts\ops\make-release-note-html.py --pdf` 轉成鴻勁品牌 HTML/PDF（客戶版紅 `#c0392b`、
>   廠內版藍 `#1f4e79`，CSS/logo 逐字取自 HT9045 模板），成品再複製到 case 的
>   `04_release\<CASE-ID>_{customer,internal}_zh-TW.{md,html,pdf}`。
> - **完整建案／結案 SOP**：`D:\HT160S_BCB\docs\ops\weekly\ht160s-case-sop.md`（含驗收清單與常見錯誤表）。

## 核心識別碼

| 識別 | 用途 | 變動性 |
|---|---|---|
| **`CASE-<EngCode>-YYYYMMDD-NNN`** | 長期主鍵，跨 session 溝通用（全域唯一） | 永久不變 |
| `weekly_data.json row=N` | 週報臨時索引 | 重整週報時可能變動 |
| `Customer/<folder>/CASE-*/` | 案件實體位置 | 永久 |

- **京元竹南 的 EngCode**：`KYEC_CHEN`（code 920；alt 921/922/924/925），對照表在 `tools/customer_code_map.json`。
- **2026-09-17 起，HT160S 的 CASE-ID 帶機型**：`CASE-HT160S_KYEC_CHEN-YYYYMMDD-NNN`。
  機型前綴由 weekly row 的 `machine` 欄自動產生（`tools/archive_issue.py` 的 `machine_prefix()`，只對 `HT160*` 生效，
  HT9045 / HT172 不受影響）。**必須用底線併進 EngCode 段**——全庫 11 個工具檔共用的正規式
  `CASE-(?:[A-Za-z][A-Za-z0-9_]*-)?\d{8}-\d{3}` 的 EngCode 段只准一段，破折號寫法會讓整套工具認不得。
  2026-09-17 之前的舊 case（`CASE-KYEC_CHEN-20260715-001`）不改名。
- **京元 case 資料夾命名**：`weekly_data.json` 的 customer 字串是「京元竹南」，而 `config.json.customers` 的 key 是「京元」
  （「京元竹南」只是 alias），且 `Customer\` 下已有空的 `KYEC\`。`archive_issue.py` 首次歸檔會依 customer 字串建
  `Customer\京元竹南\`（與 row 一致，`check_case_integrity` 才不 FAIL）。**首次歸檔前先向使用者確認命名**。

## 標準 SOP

### Step 0：每筆異常 = 新列 + 新 case（鐵律，ADR-008）
- 客戶回報一筆異常 → 新增一列 weekly item → 建一個 case。**不可**把它 append 到「HT160S 量產維護」列或任何不相干的既有 case。
- 例外：同客戶、同功能、同部署版本再回報 NG → 重啟該既有 case（加 action + 補 `01_intake/`），仍不新開。
- 「HT160S 量產維護（非客訴開發紀錄）」列（掛 `CASE-KYEC_CHEN-20260715-001`）只收非客訴工作：元件汰換、工具導入、技術債、風險評估、版本定錨。

### Step 1：登週報 + 歸檔
```
cd /d/Work-jimmychiu/document/WeeklyReport/Weekly_AI/tools
# 1) 新增 weekly 列：title=具體症狀（現象+條件+影響）；version=部署YYYYMMDD 或 待確認；--desc 詳細 / --brief 白話（雙層制，工具會擋技術字）
python update_report.py add --customer 京元竹南 --machine HT160S --version "部署20260909"     --title "<現象+條件+影響>" --desc "<客戶何時/哪台/什麼現象/附件>" --brief "<白話一句>"
# 2) 先重產 Excel 讓 row 重排定案，再取該列最終 row
python generate_report.py
python list_open.py
# 3) 用最終 row 建 case（HT160S 專屬欄位見下表；--serial 必填）
python archive_issue.py <row> <log_or_screenshot...> --desc "<短描述>" --expect-customer "京元竹南"     --category B --serial KYEC-02 --component SortArm --tags HT160S,SortArm,KYEC --severity P1
# 還沒檔案：先建殼（issue.md + 4 子夾），欄位同上
python archive_issue.py <row> --skeleton-only --desc "<短描述>" --expect-customer "京元竹南" [--category ... --serial ... --component ... --tags ... --severity ...]
```
- `--category`：`B`=軟體缺陷 `P`=現場/硬體/未定 `R`=客戶需求 `E`=改善 `Q`=詢問（依實況填，不要無腦預設 `P`）
- **順序鐵律**：先 `add` → 先 `generate_report.py` 讓 row 重排定案 → 再用最終 row 跑 `archive_issue.py`。
- 執行後建立 `Customer/京元竹南/<CASE>_<desc>/{01_intake,02_analysis,03_fix,04_release}`、寫 `issue.md`、在 `weekly_data.json` row 的 `notes` 寫 `case=CASE-...`。

#### HT160S 專屬欄位定義（ADR-008 摘要；區隔 HT9045 / HT172）

| 欄位 | 規則 |
|---|---|
| `machine` | 固定 `HT160S` |
| `title` | 具體症狀一句；**禁止**「機台開發」「功能驗證」等籠統標題 |
| `version`（回報時） | 程式版號寫死 1.0.0.0 無法識別組建 → 填**現場部署日期** `部署YYYYMMDD`（安裝包 `HT160S_Setup_*_<yyyyMMddHHmmss>_MC.exe` 的日期）；不明填 `待確認` |
| `version_number`（結案） | 交付的 NSIS 安裝包時間戳 `Setup_YYYYMMDDHHmmss_MC`；git short hash 記在 `03_fix/` |
| `--serial` | **必填**（京元現場 3 台）：京元機台編號；未給時填交機序 `KYEC-01` / `KYEC-02` / `KYEC-03`，不得留 `Default` |
| `--component` | 擇一：`Loader` `Empty` `Color` `TrayArm` `SortArm` `Auto` `AMR` `SECS` `WebAPI` `TopCCD` `ColorCCD` `BinDisplay` `Soter` `StateRecord` `HOME` `Panel` `Motion`（不得用 9045 的 InArm / OutArm / Shuttle / Index / HotPlate） |
| `--tags` | 必含 `HT160S` + 模組名 + `KYEC`；SECS / AMR 相關再加 `SECS` / `AMR` |
| `--severity` | `P1` 停機/掉料/錯分 `P2` 功能失效但有 workaround `P3` 顯示或紀錄 `P4` 詢問 |
| `01_intake/` 必附 | State Record zip（CurrentTasks / FeederDecision / SortArmDecision / TaskHistory / MachineState / SecsLog / WebApiLog）、客戶 EAP log（SECS 案）、截圖/影片；只有口述時建 `YYYYMMDD_<slug>_summary.md` |
| `02_analysis/` 首件 | 記錄「現場執行檔是否為本倉庫組建」判定（京元現場曾出現倉庫不存在的程式碼，commit 日期推論不可靠） |

完整規則：`Weekly_AI\Customer\_index\decisions\ADR-008-HT160S-weekly-case-management.md`

### Step 2：產看板
```
python case_registry.py                      # 全部
python case_registry.py --status open        # 只看未結
python case_registry.py --customer 京元竹南   # 只看京元
python case_registry.py --print
```
輸出：`Customer/_index/case-registry.md`

### Step 3：分析與修復過程
子夾用途：

| 夾 | 內容 |
|---|---|
| `01_intake/` | 客戶原始資料（log / 截圖 / State Record / 影片） |
| `02_analysis/` | 工程師分析、root cause doc、triage 筆記 |
| `03_fix/` | code diff、patch 摘要 |
| `04_release/` | 交付物（HT160S 走 ht160s-installer / NSIS，非 Weekly_AI release 工具） |

> 根因分析與改 code 由 **HT160S 開發 session 本體**（或 ht160s-maintainer）進行，遵守本 repo 的 no-FSM / Big5 / compile-gate 規則；weekly-report 子代理不改程式碼、不編譯。

### Step 4：進度更新 / 結案
- 客訴進度：`update_report.py update --search "<CASE-ID 或 title 片段>" --desc "<詳細>" --brief "<白話>" [--status in-progress|waiting]` → `generate_report.py`。**用 CASE-ID / title 搜，不記 row**；命中多筆用 `--index N` 指定（非互動 Bash 沒有 stdin，`input()` 會失敗）。
- 非客訴開發工作：同上，`--search "HT160S 量產維護"`。
- `--desc` 內避開結案關鍵字（結案、出貨、已提供安裝包、已提供版本、已修正並提供、已完成、驗證OK、已OK、已解決）——`update` 偵測到會拒絕（exit 2）；措辭改用「收尾」「交機」「已改好」。
- **結案（HT160S 不用 `close_case.py`**，內含 HT9045 專用 release note 會中止）五步：
  ```
  python update_report.py update --search "<CASE-ID>" --status done --allow-direct-done --desc "<詳細>" --brief "<白話>"
  python generate_report.py                                   # 重排 row
  python -c "import json;d=json.load(open('../weekly_data.json',encoding='utf-8'));print([i['row'] for i in d['items'] if i.get('case_id')=='<CASE-ID>'])"
  python archive_issue.py <最終row> --skeleton-only --expect-customer 京元竹南   # reuse 既有 case → issue.md status/resolved_date 同步
  python case_registry.py && python check_case_integrity.py
  ```
  NSIS 安裝包放 `04_release/installer/`、commit 清單放 `03_fix/`。安裝包 / release note 走 `ht160s-installer` skill / NSIS updater。

## 唯讀查詢（免確認）
`list_open.py`、`check_case_integrity.py`、`case_registry.py`、`update_report.py list|health`、`_customers.py "<客戶名>"`。

## 破壞性動作（先確認）
`archive_issue.py`（建實體夾 + 寫 notes）、`copy_next_week.py`（推進週期、覆寫共用 JSON）、`generate_report.py`（覆蓋 Excel）。**三機台共用資料，只動 HT160S 相關 row。**

## 編碼規範
- Weekly_AI 的 Python 工具一律 UTF-8 讀寫。
- 本 skill / commands / agent 為治理文件 → UTF-8（非本 repo BCB6 source 的 Big5 規則）。
- **絕不手動 Edit `weekly_data.json` 重寫整檔**；用 Python 工具或最小 append。

## 相關檔案（本 repo）
- 子代理：`.claude\agents\weekly-report.md`
- 指令：`.claude\commands\weekly-*.md`（help/status/update/case-intake/case-integrity/next-week）
- 寫入邊界：`.claude\settings.json` 的 `-AllowedRoots` 已含 Weekly_AI（改後需重啟 session 生效）
- 計畫：`docs\plan\weekly-caseflow-wire-into-ht160s-plan-20260715.md`
- 建案粒度 / HT160S 欄位 ADR：`d:\Work-jimmychiu\document\WeeklyReport\Weekly_AI\Customer\_index\decisions\ADR-008-HT160S-weekly-case-management.md`（2026-09-15）
