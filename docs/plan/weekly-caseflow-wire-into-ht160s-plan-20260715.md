# 把 weekly-case-flow 週報系統接到 HT160S_BCB — 計畫

- 日期：2026-07-15
- 分支：`feat/iosetview-172-refactor`
- 目標：讓 HT160S_BCB 也能像 HT9045 / HT172 一樣，把「京元竹南 (KYEC) / HT160S」的開發進度更新進共用週報，並把京元現場異常依客戶別歸檔、留排查紀錄。

---

## 0. 現況（已查證）

**週報系統實體（單一資料源，共用）**
- 根目錄：`D:\Work-jimmychiu\document\WeeklyReport\Weekly_AI\`
- 真相檔：`weekly_data.json`（JSON 是真相，Excel 是產出）
- 工具：`tools\*.py`（archive_issue / case_registry / update_report / generate_report / copy_next_week / close_case / build_lessons / make_release_note / make_proposal …）
- 客戶案件：`Customer\<客戶>\<CASE-ID>_<desc>\{01_intake,02_analysis,03_fix,04_release}\`
- 設定：`config.json`（owner=邱健銘 / RD5）

**京元竹南 / HT160S 已存在的資料**
- `weekly_data.json` row 4：`customer=京元竹南, machine=HT160S, title=機台開發, status=in-progress, is_active_this_week=true`；
  actions：③2026-07-06「dummy run 測試 / Clean out 改成全部空盤退出，開發驗證中」②2026-04-01「開發中」①2026-03-23「新需求，資料蒐集中」。
- `tools\customer_code_map.json`：`京元竹南 → KYEC_CHEN (code 920, alt 921/922/924/925)` → CASE-ID = `CASE-KYEC_CHEN-YYYYMMDD-NNN`。
- `Customer\KYEC\`：**存在但空的**（只有 .gitkeep）。

**HT9045 的接法（Hub 模式，要照抄的範本）**
- `.claude\agents\weekly-report.md`：子代理，宣告 `WEEKLY_AI_ROOT`，用 Bash `cd` 進 Weekly_AI 跑 Python。
- `.claude\agents\case-coordinator.md`：鐵律「絕不手動編 weekly_data.json，一律走 Python 工具」。
- `.claude\commands\`：`update-weekly` / `weekly-case-intake` / `weekly-case-integrity` / `weekly-status` / `weekly-next-week` / `weekly-help`。
- `.claude\skills\weekly-case-flow\SKILL.md`（+ assets\honprec-logo.png）。
- HT9045 **沒有** write-boundary hook，所以能自由寫 Weekly_AI。

**HT160S_BCB 的差異（要處理的關卡）**
- 有 write-boundary hook（`.claude\settings.json` → `scripts\ops\check-ht160s-writeboundary.ps1 -AllowedRoots …`）。
- hook 對 shell 只攔「寫入動詞」(cp/mv/rm/Set-Content/`>`/make/bcc32…)；`python x.py` 不含寫入動詞 → **目前不會被攔**。
  會被攔的是：① 直接用 Edit/Write 改 `weekly_data.json`；② 用 `cp` 複製證據檔到 `Customer\…\01_intake\`。
- 依「整套接進來」的意圖，正解是把 Weekly_AI 明確加進 `-AllowedRoots`，而非依賴上述漏洞。

---

## 1. 決策點（需你拍板）

| # | 議題 | 建議 |
|---|------|------|
| D1 | **寫入邊界**：是否把 `D:\Work-jimmychiu\document\WeeklyReport\Weekly_AI` 加進 `.claude\settings.json` 的 `-AllowedRoots` | **建議加**（否則證據檔複製 / 直接編 JSON 會被擋）。這是唯一動到「安全設定」的一步，須你明確同意。 |
| D2 | **客戶資料夾命名**：京元 case 要建在 `Customer\京元竹南\`（工具預設會用 weekly 的 customer 字串）還是沿用既有空的 `Customer\KYEC\` | 建議首次歸檔時**用工具預設 `京元竹南`**（與 weekly row 一致、`check_case_integrity` 才不會 FAIL）；空的 `KYEC\` 可留著或由你刪。若要用 `KYEC`，需在 Weekly_AI `config.json` 把 `京元竹南` 設為獨立 key —— 那是改外部 store，需 D1 通過。 |
| D3 | **footprint**：完整對齊 HT9045（agent+6 指令+skill）還是精簡（agent+核心指令） | 建議**完整對齊**，但 release-note 相關**先不接**（見 D4）。 |
| D4 | **release note**：`make_release_note.py` 寫死掃 `d:\HT9045\HT9011UC_Code_V*`，HT160S 不適用 | 建議**本次不接** release-note；HT160S 出貨走既有 `ht160s-installer` / NSIS updater。skill 內對應段落標「HT9045 專用、HT160S 待適配」。 |

---

## 2. 要建立的檔案（全部在 `D:\HT160S_BCB\`，可寫）

1. `.claude\agents\weekly-report.md`
   - 由 HT9045 版移植。改：機型固定敘述為 **HT160S**；主要客戶 **京元竹南 (KYEC_CHEN)**；`WEEKLY_AI_ROOT` 不變（共用 store）。
   - 保留：破壞性動作先確認鐵律、工具表、核心流程 A–D。
   - 加：D2 命名對齊備註；release-note 走 HT160S installer（不呼叫 make_release_note）。
2. `.claude\commands\weekly-help.md` / `weekly-status.md` / `update-weekly.md` / `weekly-case-intake.md` / `weekly-case-integrity.md` / `weekly-next-week.md`
   - 由 HT9045 版移植；把「客戶鎖定」防呆的反例從 HT9045 語境改成「勿把 HT9045/HT172 的客戶帶進來；本 repo 合法客戶＝京元竹南/KYEC」。
3. `.claude\skills\weekly-case-flow\SKILL.md`
   - 由 HT9045 版移植；release-note / 版本偵測段落標記為 HT9045 專用（HT160S 待適配）。不複製 logo（release-note 未接，用不到）。
4. `.claude\settings.json`（**D1 通過才改**）
   - 在 hook `command` 的 `-AllowedRoots` 尾端加入 `'D:\Work-jimmychiu\document\WeeklyReport\Weekly_AI'`。
5. `CLAUDE.md`
   - Authoritative sources 加一行指向 weekly-case-flow skill / Weekly_AI store。
6. 記憶：新增一則 project memory（週報接線位置、京元竹南=KYEC_CHEN、Weekly_AI 路徑、release-note 未接）。

---

## 3. 執行順序

1. 你確認 D1–D4。
2. 建 agent / commands / skill（純新增檔，可寫、可逆）。
3. D1 通過 → 改 `.claude\settings.json` 的 `-AllowedRoots`。
   - ⚠ **注意**：hook 的 `-AllowedRoots` 是啟動時讀入 settings.json；本 session 可能仍用舊值，**須重啟 Claude Code session 才生效**。本 session 內 `python 工具` 仍可跑（不含寫入動詞），但 `cp` 證據檔 / 直接 Edit JSON 可能要重啟後才放行。
4. 連線驗證（唯讀，不改資料）：
   `cd /d/Work-jimmychiu/document/WeeklyReport/Weekly_AI/tools && python case_registry.py --customer 京元竹南 --print`
   與 `python list_open.py`，確認能讀到 row 4。
5. 更新 CLAUDE.md + 記憶。
6. commit + push（只含 HT160S_BCB 內的新增/修改；Weekly_AI 是外部 store 不進本 repo）。

---

## 4. 不做 / 界線

- 不改 HT172、不改 HT9045、不把 Weekly_AI 內容複製進本 repo。
- 不動 `weekly_data.json` 既有資料（除非你明確要更新 row 4 進度）。
- 不接 release-note 自動產生（D4）。
- 京元現場異常 case 的實際歸檔，等你給實際案件（或同意用 onsite-kyec-reconciliation 的議題建 case）再做。
