# HT160S 建案 / 結案 SOP

> 對象：京元竹南（KYEC_CHEN）HT160S 量產機 3 台。
> 工具與資料在共用工作區 `D:\Work-jimmychiu\document\WeeklyReport\Weekly_AI`（HT9045 / HT172 共用）。
> 本文是 HT160S 端的操作手冊；治理規則見 `Customer\_index\decisions\ADR-008-HT160S-weekly-case-management.md`。
> 最後更新：2026-09-17

---

## 0. 鐵律（先記這三條）

1. **每筆客訴 = 一列 weekly + 一個 case。** 不可把客訴 append 進「HT160S 量產維護」列或任何不相干的既有 case。
   例外：同客戶、同功能、同部署版本再回報 NG → 重啟該既有 case，不新開。
2. **順序不能顛倒**：`add` → `generate_report.py`（讓 row 重排定案）→ 才用最終 row 跑 `archive_issue.py`。
   `add` 當下印出的 Row 是原始索引，**重整後會變**（實例：add 印 Row 822，重整後是 row 2）。
3. **三機台共用同一份 `weekly_data.json`**，只動 HT160S 相關 row。**絕不手動 Edit 該檔**。

---

## 1. Case ID 格式（2026-09-17 起）

```
CASE-HT160S_KYEC_CHEN-YYYYMMDD-NNN
     ^^^^^^^^^^^^^^^^
     機型_客戶代號（EngCode 段，一段，底線連接）
```

- **機型前綴由 weekly row 的 `machine` 欄自動產生**，不用手打。
  實作在 `tools/archive_issue.py` 的 `machine_prefix()`，只對 `HT160*` 生效；HT9045 / HT172 行為完全不變。
- **為什麼用底線不用破折號**：全庫 11 個工具檔共用同一條正規式
  `CASE-(?:[A-Za-z][A-Za-z0-9_]*-)?\d{8}-\d{3}`，EngCode 段只准**一段**英數＋底線。
  `CASE-HT160S-KYEC_CHEN-...` 是兩段，11 個檔全部認不得，會連帶弄壞 HT9045 / HT172。
- **2026-09-17 之前的舊 case 不改名**（`CASE-KYEC_CHEN-20260715-001` 保持原樣），新案才帶機型。

---

## 2. 建案（客戶回報異常 / 提出需求）

### Step 1 — 新增 weekly 列

```bash
cd "D:/Work-jimmychiu/document/WeeklyReport/Weekly_AI/tools"
python update_report.py add \
  --customer 京元竹南 --machine HT160S --version "部署YYYYMMDD" \
  --title "<現象＋條件＋影響，一句具體症狀>" \
  --desc  "<客戶何時/哪台/什麼現象/附件>" \
  --brief "<白話一句，給主管看>"
```

- `--machine HT160S` **必填**，case-id 的機型前綴靠它。
- `--title` 禁用「機台開發」「功能驗證」這類籠統標題。
- `--version` 程式版號寫死 1.0.0.0 無法識別組建 → 填**現場部署日期** `部署YYYYMMDD`
  （安裝包 `HT160S_Setup_*_<yyyyMMddHHmmss>_MC.exe` 的日期）；不明填 `待確認`。
- `--desc` 內避開結案關鍵字（結案 / 出貨 / 已提供安裝包 / 已修正並提供 / 已完成 / 驗證OK / 已解決），
  工具偵測到會 exit 2。措辭改用「收尾」「交機」「已改好」。

### Step 2 — 重整讓 row 定案，再取最終 row

```bash
python generate_report.py
python list_open.py
```

### Step 3 — 建 case

```bash
python archive_issue.py <最終row> <log或截圖...> \
  --desc "<資料夾名後綴，短>" --expect-customer "京元竹南" \
  --category R --serial KYEC-02 --component Loader \
  --tags HT160S,Loader,KYEC --severity P1
```

還沒拿到檔案時先建殼：把 `<log或截圖...>` 換成 `--skeleton-only`。

#### HT160S 專屬欄位

| 欄位 | 規則 |
|---|---|
| `--category` | `B` 軟體缺陷 / `P` 現場硬體或未定 / `R` 客戶需求 / `E` 改善 / `Q` 詢問。依實況填，不要無腦 `P` |
| `--serial` | **必填**，京元現場 3 台：`KYEC-01` / `KYEC-02` / `KYEC-03`。不得留 `Default` |
| `--component` | `Loader` `Empty` `Color` `TrayArm` `SortArm` `Auto` `AMR` `SECS` `WebAPI` `TopCCD` `ColorCCD` `BinDisplay` `Soter` `StateRecord` `HOME` `Panel` `Motion`（不得用 9045 的 InArm / OutArm / Shuttle / Index / HotPlate） |
| `--tags` | 必含 `HT160S` ＋模組名＋`KYEC`；SECS / AMR 相關再加 |
| `--severity` | `P1` 停機/掉料/錯分 · `P2` 功能失效但有 workaround · `P3` 顯示或紀錄 · `P4` 詢問 |

### Step 4 — 放進場資料

| 夾 | 內容 |
|---|---|
| `01_intake/` | State Record zip（CurrentTasks / FeederDecision / SortArmDecision / TaskHistory / MachineState / SecsLog / WebApiLog）、客戶 EAP log（SECS 案）、截圖／影片。只有口述時建 `YYYYMMDD_<slug>_summary.md` |
| `02_analysis/` | **首件必記**：現場執行檔是否為本倉庫組建的判定（京元現場曾出現倉庫不存在的程式碼，commit 日期推論不可靠） |
| `03_fix/` | code diff、patch 摘要、commit 清單 |
| `04_release/` | 交付物（release note ＋ 安裝包，見 §4） |

---

## 3. 進行中更新

```bash
python update_report.py update --search "<CASE-ID 或 title 片段>" \
  --desc "<詳細>" --brief "<白話>" [--status in-progress|waiting]
python generate_report.py
```

用 **CASE-ID / title 搜，不要記 row**（row 會漂）。命中多筆用 `--index N`
（非互動 Bash 沒有 stdin，工具的 `input()` 會失敗）。

非客訴的開發工作 → `--search "HT160S 量產維護"`，那一列只收元件汰換、工具導入、技術債、風險評估、版本定錨。

---

## 4. Release note（鴻勁紅）

### 產生

release note 原稿 `.md` 寫在 repo `D:\HT160S_BCB\docs\release\`（受 git 版控，改動有歷史可追），
成品用本 repo 的產生器轉成帶品牌的 HTML / PDF：

```bash
cd D:/HT160S_BCB
python scripts/ops/make-release-note-html.py --pdf
```

| 版本 | 檔名樣式 | 配色 |
|---|---|---|
| 客戶版 | `*_customer_zh-TW.*` | **鴻勁紅 `#c0392b`** |
| 代理商版 | `*_distributor_*.*` | 鴻勁紅 `#c0392b` |
| 廠內版 | `*_internal_zh-TW.*` | 鴻勁藍 `#1f4e79`（Steven 規範：廠內＝藍） |

- 配色由檔名自動判定，要覆寫用 `--accent red|blue`。
- CSS／header／logo 逐字取自 HT9045 `tools/make_release_note.py`，logo 在 `docs/release/assets/honprec-logo.png`。
- PDF 走 headless Chrome／Edge，與 `scripts/ops/build-op-guide-pdf.py` 同一套。
- **不要用 Weekly_AI 的 `make_release_note.py`**：它寫死掃 `d:\HT9045\HT9011UC_Code_V*`，對 HT160S 會失敗。

### 歸位

成品（`.md` / `.html` / `.pdf`）複製到 case 的 `04_release/`，改名為 HT9045 慣例：

```
04_release/<CASE-ID>_customer_zh-TW.{md,html,pdf}
04_release/<CASE-ID>_internal_zh-TW.{md,html,pdf}
04_release/installer/<安裝包>.exe ＋ _交付說明.md
```

**對外交付一律以 case 內的為準**，repo 的只是來源原稿。

### 客戶版內容規則

- **一個 case 只講那個 case 的事。** 同版夾帶的其他改善不要混進客戶版
  （需要時另開 case、另出一份）。
- 不得承諾軟體做不到的事。例：`[Safety]` 只有開關 `CleanOutRefillGuard`，
  確認時間 `CLEANOUT_REFILL_CONFIRM_MS` 是 `aLoader.cpp:34` 的 `static const int`，**不可說能調**。
- 警報要寫清楚操作員怎麼解：有哪些按鍵、按了不做事會不會再跳。
- 若同時存在不可出貨的測試包，要明寫「請勿使用」並說明為什麼（例：自報版號與檔名不符）。

---

## 5. 結案

**HT160S 不用 `close_case.py`**——它內含 HT9045 專用的 release note 產生流程（寫死 `d:\HT9045\HT9011UC_Code_V*`），會中止。改走五步：

```bash
cd "D:/Work-jimmychiu/document/WeeklyReport/Weekly_AI/tools"

# 1) 週報標記完成
python update_report.py update --search "<CASE-ID>" --status done --allow-direct-done \
  --desc "<詳細>" --brief "<白話>"

# 2) 重排 row
python generate_report.py

# 3) 取該 case 的最終 row
python -c "import json;d=json.load(open('../weekly_data.json',encoding='utf-8'));print([i['row'] for i in d['items'] if i.get('case_id')=='<CASE-ID>'])"

# 4) reuse 既有 case，讓 issue.md 的 status / resolved_date 同步
python archive_issue.py <最終row> --skeleton-only --expect-customer 京元竹南

# 5) 重建看板 + 驗收
python case_registry.py && python check_case_integrity.py
```

結案前 `04_release/` 必須有：NSIS 安裝包（放 `04_release/installer/`）、對應的 release note 三件組、
`03_fix/` 的 commit 清單。安裝包走 `ht160s-installer` skill / NSIS updater 產出。

---

## 6. 驗收檢查清單

建案後、結案後各跑一次：

- [ ] `python check_case_integrity.py` → **`[FAIL] integrity mismatches` 是 `(none)`**
- [ ] 該 case 出現在 `[OK] linked weekly items`，customer 是 `京元竹南`
- [ ] 沒有針對**本 case** 的 `stale weekly_row pointer` WARN
      （有的話：重跑 `archive_issue.py <最終row> --skeleton-only --expect-customer 京元竹南` 同步）
- [ ] case-id 形如 `CASE-HT160S_KYEC_CHEN-YYYYMMDD-NNN`
- [ ] `issue.md` 的 serial / component / tags / severity / category 都不是預設值
- [ ] `01_intake/` 有實體證據或 `_summary.md`
- [ ] 結案時 `04_release/` 有安裝包與三件組 release note
- [ ] `python case_registry.py` 重建成功

---

## 7. 常見錯誤

| 症狀 | 原因 | 處置 |
|---|---|---|
| `--expect-customer` 不符而中止 | `generate_report.py` 已重排 row，add 回傳的 row 失效 | 用 `list_open.py` 重查當前 row |
| `stale weekly_row pointer` WARN | `issue.md` 記的 row 與重排後不同 | 重跑 `archive_issue.py <最終row> --skeleton-only --expect-customer 京元竹南` |
| `update` exit 2 | `--desc` 含結案關鍵字 | 改措辭，或確實要結案就走 §5 |
| case-id 沒帶 `HT160S_` | weekly row 的 `machine` 欄不是 HT160S | 修正該 row 的 machine 後重建 |
| 客戶收到純文字信 | 只產了 `.md` 沒跑產生器 | 跑 `make-release-note-html.py --pdf` |
| 中文在終端機是亂碼 | 主控台是 CP950 | 顯示問題，非資料損毀；要確認內容請直接讀檔 |

---

## 8. 唯讀查詢（免確認）

`list_open.py`、`check_case_integrity.py`、`case_registry.py`、`update_report.py list|health`、`_customers.py "<客戶名>"`

## 9. 破壞性動作（先確認）

`archive_issue.py`（建實體夾＋寫 notes）、`copy_next_week.py`（推進週期、覆寫共用 JSON）、`generate_report.py`（覆蓋 Excel）
