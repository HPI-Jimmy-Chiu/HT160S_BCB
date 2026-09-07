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
> - **release note / proposal 工具（`make_release_note.py` / `make_proposal.py` / `close_case.py`）為 HT9045 專用**，
>   寫死掃 `d:\HT9045\HT9011UC_Code_V*`，對 HT160S 會失敗——**HT160S 不使用**。出貨走 `ht160s-installer` skill / NSIS updater。
> - 本 skill 不內建 logo / CSS 模板（release-note 未接，用不到）。

## 核心識別碼

| 識別 | 用途 | 變動性 |
|---|---|---|
| **`CASE-<EngCode>-YYYYMMDD-NNN`** | 長期主鍵，跨 session 溝通用（全域唯一） | 永久不變 |
| `weekly_data.json row=N` | 週報臨時索引 | 重整週報時可能變動 |
| `Customer/<folder>/CASE-*/` | 案件實體位置 | 永久 |

- **京元竹南 的 EngCode**：`KYEC_CHEN`（code 920；alt 921/922/924/925），對照表在 `tools/customer_code_map.json`。
  → CASE-ID = `CASE-KYEC_CHEN-YYYYMMDD-NNN`。
- **京元 case 資料夾命名**：`weekly_data.json` 的 customer 字串是「京元竹南」，而 `config.json.customers` 的 key 是「京元」
  （「京元竹南」只是 alias），且 `Customer\` 下已有空的 `KYEC\`。`archive_issue.py` 首次歸檔會依 customer 字串建
  `Customer\京元竹南\`（與 row 一致，`check_case_integrity` 才不 FAIL）。**首次歸檔前先向使用者確認命名**。

## 標準 SOP

### Step 1：登週報 + 歸檔
```
cd /d/Work-jimmychiu/document/WeeklyReport/Weekly_AI/tools
# 已有檔案：建 case 並複製到 01_intake/
python archive_issue.py <row> <log_or_screenshot...> --desc "<短描述>" --expect-customer "京元竹南" \
    [--category P|R|B|E|Q] [--serial <機台序號>] [--tags <t1,t2>] [--component <模組>] [--severity P2]
# 還沒檔案：先建殼（issue.md + 4 子夾）
python archive_issue.py <row> --skeleton-only --desc "<短描述>" --expect-customer "京元竹南" [...]
```
- `--category`：`P`=Problem `R`=Request `B`=Bug `E`=Enhancement `Q`=Question（預設 P）
- **順序鐵律**：先新增 weekly item → 先 `generate_report.py` 讓 row 重排定案 → 再用最終 row 跑 `archive_issue.py`。
- 執行後建立 `Customer/<客戶>/<CASE>_<desc>/{01_intake,02_analysis,03_fix,04_release}`、寫 `issue.md`、在 `weekly_data.json` row 的 `notes` 寫 `case=CASE-...`。

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
- 進度：`update_report.py`（或最小 Edit 加 action）→ `generate_report.py` 重產 Excel。
- **結案（HT160S）**：用 `update_report.py` 把該 row `status=done` + `generate_report.py`。**不要跑 `close_case.py`**（內含 HT9045 專用 release-note，會失敗中止）。安裝包 / release note 走 `ht160s-installer` skill / NSIS updater。

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
