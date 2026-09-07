---
name: weekly-case-flow
description: >
  HT172 端週報 / 客戶異常案件處理流程技能（Hub 模式，操作共用 Weekly_AI 工作區）。
  涵蓋 力成PTI 等客戶異常 → 週報登錄 → Case 歸檔 → 看板總覽的鏈路。資料與工具在
  d:\Work-jimmychiu\document\WeeklyReport\Weekly_AI（HT9045/HT172/HT160S 共用）。
  適用關鍵字：客戶異常、log 歸檔、case、CASE-ID、週報新增、看板、case registry、力成PTI。
applyTo: "**/*"
---

# weekly-case-flow（HT172 端）— Weekly_AI 異常案件處理流程

> 本 skill 是「HT172 接入共用 Weekly_AI 週報系統」的 SOP。工具與資料**不在本 repo**，而在共用工作區
> `d:\Work-jimmychiu\document\WeeklyReport\Weekly_AI`（與 HT9045 / HT160S 共用同一份 `weekly_data.json`）。
> 實際操作交給 `.claude\agents\weekly-report.md` 子代理，用 Bash `cd` 進去跑 Python。
>
> **與 HT9045 原版的差異（重要）**：
> - 機型固定 **HT172**；多客戶（週報現況以 **力成 / 力成PTI** 為主，另有台星科、京元竹南等）。
> - **release note / proposal / close_case 工具為 HT9045 專用**，寫死掃 `d:\HT9045\HT9011UC_Code_V*`，對 HT172 會失敗——**HT172 不使用**。出貨走 `D:\HT172_Updater_NSIS`。
> - HT172 為 **Big5 legacy source 且有 FSM**（FSM 參考專案）；改 code 交回 HT172 開發 session 本體，遵守 Big5 + FSM 慣例。
> - HT172 `.claude\settings.json` 只有 `guard-big5.ps1`（編碼防護，非路徑邊界）→ **可自由寫 Weekly_AI，無需 settings 改動**；但**切勿用 Edit 改 Big5 檔**（一律走 python 工具）。

## 核心識別碼

| 識別 | 用途 | 變動性 |
|---|---|---|
| **`CASE-<EngCode>-YYYYMMDD-NNN`** | 長期主鍵，跨 session 溝通用（全域唯一） | 永久不變 |
| `weekly_data.json row=N` | 週報臨時索引 | 重整週報時可能變動 |
| `Customer/<folder>/CASE-*/` | 案件實體位置 | 永久 |

- EngCode 取自 `tools/customer_code_map.json`（依客戶對應官方 `CC_*` 代號）。查無代號則 fallback 舊格式並提醒補對照表。
- **case 資料夾命名**：`archive_issue.py` 依 weekly 的 customer 字串解析資料夾（先查 `config.json.customers` key，其次 alias / 代號）。首次歸檔新客戶前先確認命名。

## 標準 SOP

### Step 1：登週報 + 歸檔
```
cd /d/Work-jimmychiu/document/WeeklyReport/Weekly_AI/tools
# 已有檔案：建 case 並複製到 01_intake/
python archive_issue.py <row> <log_or_screenshot...> --desc "<短描述>" --expect-customer "<客戶>" \
    [--category P|R|B|E|Q] [--serial <機台序號>] [--tags <t1,t2>] [--component <模組>] [--severity P2]
# 還沒檔案：先建殼（issue.md + 4 子夾）
python archive_issue.py <row> --skeleton-only --desc "<短描述>" --expect-customer "<客戶>" [...]
```
- **順序鐵律**：先新增 weekly item → 先 `generate_report.py` 讓 row 重排定案 → 再用最終 row 跑 `archive_issue.py`。
- 執行後建立 `Customer/<客戶>/<CASE>_<desc>/{01_intake,02_analysis,03_fix,04_release}`、寫 `issue.md`、在 `weekly_data.json` row 的 `notes` 寫 `case=CASE-...`。

### Step 2：產看板
```
python case_registry.py                      # 全部
python case_registry.py --status open        # 只看未結
python case_registry.py --customer 力成PTI    # 只看力成PTI
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
| `04_release/` | 交付物（HT172 走 D:\HT172_Updater_NSIS，非 Weekly_AI release 工具） |

> 根因分析與改 code 由 **HT172 開發 session 本體**進行，遵守 HT172 的 Big5 編碼與 FSM 慣例；weekly-report 子代理不改程式碼、不編譯。

### Step 4：進度更新 / 結案
- 進度：`update_report.py update --search "<客戶/關鍵字>" --desc "<描述>" --status in-progress` → `generate_report.py` 重產 Excel。
- **結案（HT172）**：用 `update_report.py` 把該 row `status=done` + `generate_report.py`。**不要跑 `close_case.py`**（內含 HT9045 專用 release-note，會失敗中止）。安裝包 / release note 走 `D:\HT172_Updater_NSIS`。

## 唯讀查詢（免確認）
`list_open.py`、`check_case_integrity.py`、`case_registry.py`、`update_report.py list|health`、`_customers.py "<客戶名>"`。

## 破壞性動作（先確認）
`archive_issue.py`（建實體夾 + 寫 notes）、`copy_next_week.py`（推進週期、覆寫共用 JSON）、`generate_report.py`（覆蓋 Excel）。**三機台共用資料，只動 HT172 相關 row。**

## 編碼規範
- Weekly_AI 的 Python 工具一律 UTF-8 讀寫。
- 本 skill / commands / agent 為治理文件 → UTF-8。
- **絕不手動 Edit `weekly_data.json` 重寫整檔**；用 Python 工具或最小 append。HT172 的 Big5 guard 會擋 Edit 非 UTF-8 檔，勿用 Edit 碰 Big5 樣板。

## 相關檔案（HT172 repo）
- 子代理：`.claude\agents\weekly-report.md`
- 指令：`.claude\commands\weekly-*.md`（help/status/update/case-intake/case-integrity/next-week）
- 寫入邊界：HT172 無路徑寫入邊界（只有 guard-big5），可直接寫 Weekly_AI
- 來源：由 HT160S_BCB `docs\plan\ht172-weekly-sync\` 套件套用
