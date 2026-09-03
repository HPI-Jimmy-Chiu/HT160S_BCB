# ALID Option D — 執行計畫（20260903）

- 規格權威：`docs/plan/alid-option-d-ratified-spec-20260902.md`（20260902 使用者逐項裁定，本文不重述規則）
- 昨晚成品（已自 scratchpad 收進 repo）：`docs/plan/alid-option-d-20260902/`
  - `alid_optiond_final_change_20260902.txt` — BCB6 變更檔，9 edits / 5 檔，逐行可貼
  - `gen-alid-catalog.py` — 產生器（`--selftest / --audit / --emit-* / --verify-table / --check`）
  - `HT160S_ALID_OptionD_mapping_amended_20260902.csv` — 532 筆新舊對照
  - `HT160S_ALID_Class9_FreeString_20260902.csv` — 47 支自由字串警報
  - `ALID_OptionD_doc_workbook_tooling_plan_20260902.md` — 文件／工作簿／模擬器逐格變更計畫
  - `ADVERSARIAL_EVIDENCE_20260902.md`、`GENERATOR_RUN_20260902.log` — 複驗證據
- 20260903 新增裁定：客戶需求定義為 **ALID ≤ 999,999,999（恆 9 碼）**，確定要改。信稿不處理。
- 客戶版工作簿：**`docs/SECS/SECS_GEM功能_Handler_20260903.xlsx`**（使用者 20260903 手動建立，
  目前與 0831 版 byte-identical，md5 `e0ff9e2c…`）。所有工作簿變更只進這一個檔。

---

## 0. 需求與現況（一句話）

| | 現況（雜湊） | 目標（Option D） |
|---|---|---|
| 位數 | 8 或 10，不固定 | **恆 9** |
| 註冊 485 筆 | 420 筆 8 碼、65 筆 10 碼 | 全部 9 碼 |
| 有號 int32 | 46/532 超過 2^31-1 | 0 筆（max 999,752,848） |
| 可反解 | 否 | 是（class 1-8） |
| 與 9046LS 目錄交集 | — | 0（窮舉） |
| 變動筆數 | — | 530/532（含全部 485 筆註冊碼） |

⚠ 客戶只要求 ≤ 9 碼；Option D 連 420 支已合規的 8 碼數字碼也一起換號，這是我方為「可反解、不撞 9046LS」
主動做的整併。對外文件與口頭說明都要講成「我方整併」，不是「依貴端要求只改 61 筆」。

---

## 1. 待裁定（動碼前）

| # | 事項 | 建議 | 影響 |
|---|---|---|---|
| **B2** | `database.cpp:1090` 既有 [ALARM-COLLISION] 守衛只測 `'4'/'5'/'6'`，是否擴成 `'4'..'8'` 並把 `OutputDebugString` 改走 `g_EventLog`（＝變更檔 E6b） | **擴，且併入修正 2 的同一份報告**。class 7/8 已是正式類別；現場沒有 debugger，OutputDebugString 等於不存在 | 改變 commit 內容（多 ~8 行），不影響 ALID 值 |
| D1 | 47 支自由字串是否**同批**登錄成正式 MES/WAR 碼 | 建議**分兩批**：本批只做 Option D（客戶已在等），登錄另案。但要在文件寫明「這 47 筆日後會再變一次」 | 同批做 = 工時 +1 天、客戶只換一次表；分批 = 客戶換兩次表 |
| D2 | `aLoader.cpp:2442` `ShowMyError(LangT(...))` 語言相依 ALID | 建議**順手修**（改三參數多載、登錄 MES 碼），它是唯一非確定性 ALID | 規格 §6 原列排除，需使用者點頭 |

B2 不裁，S2 不能切 commit；D1/D2 不裁不擋 S1-S3，但擋 S4（工作簿內容）。

---

## 2. 執行順序（相依鏈）

```
S0 保存成品(done) → S1 產生器入庫+加閘 → S2 韌體(E1-E9[+E6b]) → S3 建置閘 sim+real
   → S3b 模擬器 headless 對真韌體 round-trip → S4 工作簿 0903 → S5 md/html 兩本
   → S6 SECS 模擬器 → S7 記憶/.github → S8 退役雙胞胎 → S9 打包(_ZipFile.bat)
```
S8 是 S9 的**硬前置**（B5：否則 `*_manual.xlsx` 舊雜湊說法會一起被打包送出）。

| 步 | 內容 | 閘 / 驗收 | 工時 |
|---|---|---|---|
| S1 | `gen-alid-catalog.py` 移到 `scripts/ops/`；加 `--expect-csv-md5`（今日 `73e76f289844430582165095758c5126`）與**列數斷言 485**（B1：HEAD 的 `AlarmList.csv` 只有 484 列，缺 `WAR0963`，乾淨 checkout 未開機重生會送 484 列字典） | `--selftest` 0 失敗、`--audit` 0 違規、`--verify-table` 對 `mapping_amended` 0 不符 | 0.5 h |
| S2 | 照變更檔 E1-E9 套用（B2 裁「擴」則加 E6b）。E1 保留 `LegacyAlarmHash()`（class 9 payload 來源 + 一行回滾）。`uHGemHT160.cpp:3499` **不動**（純字串函式契約） | 新註解 ASCII；CRLF 保留（`git diff --stat` 不得出現整檔改寫） | 3.5 h |
| S3 | 刪 `UsecegemMainFrom.obj` `database.obj` → `-Full`（有 header 改動）。現行工作樹 `MachineType.h` 是 `//#define SOFT_SIMULATE`（真機），順序：真機 `-Full` exit 0 → 打開 define `-Full` exit 0 → 還原真機重編。**不要 commit define 的翻轉** | 兩態 exit 0；encoding check 166 檔 pass；`test-ht160s-startup.ps1` 後 EventLog 恰一行 `INF_ALID_AUDIT … 485 alarm code(s), 0 violation(s)`；`AlarmList.csv` byte-identical | 1.0 h |
| S3b | 筆電 headless host 對真韌體（記憶 `secs-headless-roundtrip-verify`）：S5F5/S5F7 全查 → 485 筆全 9 碼且逐筆等於對照表；觸發一支警報 → S5F1 ALID == 目錄 | 逐筆比對 0 不符 | 0.5 h |
| S4 | **工作簿 `Handler_20260903.xlsx`**（見 §3） | openpyxl 存檔後 PNG 兩張 md5 不變、Excel 目視、`--check-prose` 0 殘留 | 3.5 h |
| S5 | `HT160S_SECS_Comm_Examples.md`（:781-792, :805-810, :829, :1011-1013, :1075；`4045923824` 舊例改 MES0920）、`HT160S_SECS_Interface_Spec_20260727.md`（:345 錯句、:857-859）；兩檔 `.html` 依既有配方 byte-exact 重生 | grep 無 `3891410149`/`2^31`/`無號 32` 殘留（除歷史註記） | 2.5 h |
| S6 | `D:\AI_Area\Tool\HT160S_SECS_Simulator\code\`：`--emit-sim --in-place` 換 `ALID_CATALOG` 485 筆；`ALID_OVER_SIGNED` 守衛改為「任一 ALID 不是 9 碼即報警」；`ht160s_presets.py:1428`、`scenario_runner.py:1375`、docs 3 份「481 / 14 筆 / 無號」共 20 處；PyInstaller 重打包 | `--check` 磁碟同步 exit 0；對真韌體跑一次 `ALARMLIST` 情境 | 2.5 h |
| S7 | 記憶 `secs-alid-hash-encoding.md`（狀態改 SHIPPED）、`secs-workbook-single-file-0804.md`（SSOT 檔名 → 0903）；`docs/manual/D1-alarm-list.md:3`（481/33/14 → 485/36/15）；`docs/plan/cleanout-amr-collect-call-plan-20260901.md:552`（「ALID 不變」舊敘述） | — | 0.5 h |
| S8 | 四個 `*_manual.xlsx` 與 0831 版移出 `docs/SECS/` 交付範圍（移到 `docs/SECS/history/`），0903 成唯一交付檔 | `_ZipFile.bat` 打包清單只剩 0903 + 兩本 html | 0.3 h |
| S9 | 打包交付；**同一 maintenance window** 內韌體與字典一起切；現場 `General.ini` 已另有 T6/Linktest 手改項 | — | 0.5 h |

合計約 15.5 h（不含等機台）。D1 同批登錄 +1 天。

---

## 3. 工作簿 `SECS_GEM功能_Handler_20260903.xlsx` 逐格計畫

呈現慣例（自 0831 版讀出，必須沿用）：
- 「修訂說明」是**內部維護頁，客戶版不含**（功能 R32 自述）。客戶看得到的是 功能 / SVID / CEID / ECID / ALID 五張。
- 功能頁每格以 `[YYYYMMDD] 已實作…` 起頭，就地更正用 `〔YYYYMMDD 更正：…〕`，警示用 `⚠`，稱謂「貴端／本機／本方」，
  數字一律給精確筆數，並附「貴端請勿…／請改…」的行為指引。
- ALID 頁欄位：`ALID | 警報碼 AlarmCode | ALCD | 類別 | 目錄訊息 ALTX`，資料列後空一列，再接表尾註（長段落）。

| 工作表 | 位置 | 變更 |
|---|---|---|
| ALID | A2:E486 | 產生器 `--emit-xlsx --xlsx-base 0903` 重建 485 列（補 `MES1025 / MES1428 / MES1921 / WAR0963`），ALID 欄換 9 碼。**新增** F `Class`、G `Payload` 兩欄（解碼示範）；ALCD／類別／ALTX 欄不變 |
| ALID | A488（原 A484） | 全段改寫：刪雜湊規則、刪「14 筆超過 2^31-1／須無號解析」；改寫為 Class 表 + 解碼公式 + 「class 9 = 不在目錄、讀 ALTX 第一個 token」；保留 S5F1 ALCD 不帶類別、與 9046LS 差異兩段 |
| ALID | A489（原 A485） | 定點改：`3891410149` → `991410149`；其餘 SnFKCleanOut 語意段原文保留 |
| 功能 | E16（#14 S5F1） | 加 `〔20260903 變更〕` 段：ALID 改 class-banded 9 碼、值域、有號 int32 可存、S5F1 與目錄同號由構造成立；刪 481／14 筆／無號句 |
| 功能 | E18（#16 S5F5） | 481 → 485（含 4 筆補列說明）；刪「無號」句 |
| 功能 | E31（資料收集 #8） | 481 → 485 |
| 修訂說明 | R3-R6 | 修訂日期 2026-09-03；本檔 0903；前一版 0831；依據韌體 = S2 的 commit |
| 修訂說明 | R8 起 | 新增 「1. ALID 改 9 碼（Option D）」「2. 目錄 481 → 485」「3. 舊雜湊值停用、換表須整批」三項；「尚待貴端確認」加 D1（47 筆是否要先登錄只換一次表） |

**不做的事**：不新增工作表；不改 SVID/CEID/ECID 頁；不在客戶看得到的格子引用「修訂說明」頁。

---

## 4. 回滾

`ComputeAlarmAlid()` 第一個可執行敘述插入 `return LegacyAlarmHash(Code);` 一行 → S5F1 與 S5F6/S5F8 同時回到舊值，
`uHGemHT160.cpp` 不動、ALCD 不動、`AlarmList.csv` 本來就沒有 ALID。修正 2 自檢會如實報每列違規（上限 40+2 行，不擋開機）。
無法回滾的只有客戶 EAP 已吃進去的字典 —— 所以韌體與字典必須同窗切換。

---

## 5. 風險與已知不做

- 47 支自由字串留在 class 9、不進目錄；日後登錄會再變號（D1）。
- ALTX 超過 40 字元 447/485 筆（Data Item 規格上限），規模是 ALID 問題的 7 倍，另案。
- class 4-8 每類只有 10,000 個 payload；超過即落 class 9，由修正 2 抓。
- `note.cpp:1005` 在 map 不一致時給碼加 `-` 前綴 → S5F1 落 class 9 而目錄不落；只在內部 map 壞掉時可達，另案。

---

## 6. 執行紀錄

### 20260903 裁定
- **B2 = 擴 '4'..'8' 並改走 EventLog**（= 變更檔 E6b）；**D1 = 分批**（47 自由字串另案登錄）；**D2 = 順手修**。

### S1 完成（20260903）
- `scripts/ops/gen-alid-catalog.py` 入庫；新增 B1 資料來源閘：`EXPECTED_CSV_ROWS` / `EXPECTED_CSV_MD5` 常數（可用
  `--expect-csv-rows / --expect-csv-md5` 覆寫，`--allow-csv-drift` 降為警告），每次執行先驗。
  負向測試：錯 md5 → exit 1；HEAD 的 484 列副本 → exit 1。`--selftest` 0 失敗、`--audit` 0 違規、
  `--verify-table` 對昨晚 532 表 0 不符。
- **未掛進 `build-ht160s.ps1`**：`--check` 比對的是模擬器磁碟常數，S6 之前必然 stale 會擋建置；S6 完成後再掛。

### S2 完成（20260903）
- E1-E9 照變更檔逐字套用（`UsecegemMainFrom.cpp/.h`、`database.cpp/.h`、`ht160s.cpp`）。
- B2：`database.cpp` 守衛 `if(c0<'4' || c0>'8')`；命中時 `gAlidAuditFaults++` 並寫入 `gAlidAuditDetail`
  （受 `K_ALID_AUDIT_MAX_LINES` 上限，溢出計入 `gAlidAuditSuppressed`）；verdict latch 的五行 reset 移到
  `CreateSystemAlarmCode()` 開頭（`mapAlarmCodeList.clear()` 之後），守衛才能寫進同一份報告。
- D2：`aLoader.cpp` 改用 `ShowMyError("MES0926", LangT("Loader Tray has IC,please remove"), ...)`；
  `database.cpp` 以 MES0925 同款 standalone 區塊登錄 `MES0926`（訊息與 LangT key 逐位元相同，
  `language_phrases.txt:138` 中譯不受影響；9046LS 字典無 MES0926，不撞）。
  → 註冊碼 485 → **486**，ALID `300000926`；舊值 1671856712（雜湊）永不再送出。
- 全部新註解 ASCII；六檔 CRLF 保留、非 ASCII 位元 0。

### S3 完成（20260903）
| 建置 | 模式 | 結果 |
|---|---|---|
| A | 真機（`//#define SOFT_SIMULATE`，工作樹既有狀態）`-Full` | exit 0（唯一警告 W8004 csystem.cpp:1951 為既有） |
| B | 模擬（define 打開）`-Full` | exit 0 |
| smoke | `test-ht160s-startup.ps1 -StartupSeconds 20` | **Pass**（ProcessAliveAfterStartupWindow） |
| C | 還原真機（md5 `4fdc44ce…` byte-identical）`-Full` | exit 0 |
- EventLog `HT160S_2026_09_03.csv` 恰一行：`INF_ALID_AUDIT,"S5 ALID self-check : 486 alarm code(s), 0 violation(s)"`。
- `system/AlarmList.csv` 由開機重寫為 486 列（含 `MES0926`），md5 `b710227ca23356f178c7384bf2784e70`；
  產生器常數同步 bump；`--audit` 486/486、class {1:14, 2:15, 3:37, 4:234, 5:180, 6:6}、0 違規。
- 新交付對照表：`docs/plan/alid-option-d-20260902/HT160S_ALID_map_20260903.csv`（486 列，含 OldALID / Changed / DecodesBackTo）。
- alarm-registry 檢查：31 literals（MES0926 已被抓到並確認登錄）。

### 尚未做
- S3b 對真韌體 headless round-trip（需機台或真韌體在線）。
- S4 工作簿 0903、S5 兩本 md/html、S6 模擬器、S7 記憶/.github、S8 退役雙胞胎、S9 打包。
- ⚠ 對照表變成 486 列後，S4 表尾註／功能頁的「485」字樣要寫 **486**，`MES0926` 要列入「本次新增」。
