# ALID Option D — 文件／工作簿／工具變更計畫（20260902）

權威規格：`D:\HT160S_BCB\docs\plan\alid-option-d-ratified-spec-20260902.md`（含修正 1 / 1b / 2 / A5）。
本文件只講「除了 `ComputeAlarmAlid()` 那一個函式以外，還要動哪些檔案的哪一格」。
**本輪為設計與規劃，未修改任何專案檔、未建置、未 commit。所有產出都在 scratchpad。**

實測基準（本輪自行重跑，非引用）：
`system\AlarmList.csv` = 485 列（486 行含標頭，CP950、無 BOM、零重複碼），
class 分布 `{1:14, 2:15, 3:36, 4:234, 5:180, 6:6}`，零筆 class 9，
ALID 值域 `100000913`（`JAM0913`）～ `600060005`（`60005`），
485/485 恰 9 碼、485/485 反解回原碼、**0 筆超過 2^31-1**、485 筆全部與舊雜湊不同。

---

## 0. 硬性前置與紅線

| 項目 | 內容 |
|---|---|
| 不可先做 | **韌體 `ComputeAlarmAlid()` 尚未改、尚未上機驗證之前，不得把 0902 工作簿或任何新 ALID 表交付客戶。** 表一送出，客戶 EAP 會照新號建表，機台仍送舊雜湊 → 全部警報變「未知 ALID」。 |
| 不可先做 | 修正 1b 的「unit 01-09 上限 100 支」是**命名規範變更**，需與警報碼命名的維護者同步後才寫進客戶文件（本計畫已備妥文字，但需owner點頭）。 |
| Excel 鎖檔 | `docs\SECS\~$SECS_GEM功能_Handler_20260831.xlsx` 存在（2026-09-02 14:55，Jimmychiu）→ **0831 底本目前開在 Excel 裡**。動手前務必關閉 Excel，否則 md5 可能在你讀完後被 Excel 回寫而變。 |
| 底本 md5 | `SECS_GEM功能_Handler_20260831.xlsx` = `e0ff9e2c182dc6521fd706a2db00abda`（本輪實測相符）。**開檔前先比 md5**：同目錄另有 `SECS_GEM功能_Handler_20260729.xlsx`、`..._20260802.xlsx`、`..._20260804.xlsx`、`HandlerV1_20260804/20260813.xlsx`，且 `.claude\worktrees\friendly-vaughan-11d76a\docs\SECS\` 下有一整份**同名 0729 舊複本**樹（`git worktree` 造成）——這就是「同名 0729 複本」的來源，md5 閘門是唯一可靠的防呆。 |
| 寫入邊界 | 可寫：`D:\HT160S_BCB`、`D:\AI_Area\Tool\HT160S_SECS_Simulator`。`D:\HT9045` / `D:\HT172` 唯讀。 |
| 規格書內部不一致（已依指示回報，未擅改） | 規格書 §4 容量表 class 1/2/3 仍寫 1,000,000 槽（修正 1 的上限）。**修正 1b 之後可達 payload 只有 0..99,999 = 100,000 槽**，小 10 倍。規則本身一致，只有容量表過期；§3 的壓力測試數字（3,371,021 字串／變動 530 筆）亦為修正前的歷史值。**§7 要寫進工作簿的容量數字必須用 100,000，不是 1,000,000。** |

---

## 1. 產生器（第一優先，其他每一步都吃它的輸出）

| 項目 | 值 |
|---|---|
| 本輪產出（scratchpad） | `C:\Users\jimmychiu\AppData\Local\Temp\claude\D--HT160S-BCB\b63d382a-3a44-4023-ab09-e15ed9877b6e\scratchpad\p4\gen-alid-catalog.py` |
| 行數／md5 | 631 行 / `e96e41edb1675c4cc72142b0bffe3a4c`（UTF-8、LF、無 BOM、`-W error::SyntaxWarning` 乾淨） |
| 建議入庫位置 | **`D:\HT160S_BCB\scripts\ops\gen-alid-catalog.py`**（同目錄已有 `check-ladder-consistency.py`、`md-manual-to-docx.py` 兩支 python，慣例一致；python 3.8+，唯一外部相依 `openpyxl`，只有 `--emit-xlsx` / `--check-prose(xlsx)` 用得到） |
| 執行紀錄 | `...\scratchpad\p4\GENERATOR_RUN_20260902.log`（本輪全模式一次跑完，exit 0） |

### 1.1 資料流（單一真源）

```
firmware 開機 dump  ->  system\AlarmList.csv  (485 列, CP950)
                              |
   gen-alid-catalog.py  ------+-----------------------------------------------
                              |                     |               |
                    ALID 工作表 7 欄            ALID_CATALOG      HT160S_ALID_map
                    (--emit-sheet-csv /          485 tuples        _20260902.csv
                     --emit-xlsx)              (--emit-sim)      (--emit-map-csv)
```
`AlarmList.csv` 本身由 `database.cpp:1108-1119` 在開機時傾印，**今天磁碟上的那份已經是 485 列**（含要補進工作簿的 4 筆），所以不需要為了產表先重跑機台。

### 1.2 指令

```powershell
# 0) 規則自檢（不讀 CSV 也能跑；20 個裁定範例 + 35 萬字串結構掃描）
python scripts\ops\gen-alid-catalog.py --selftest
# 1) 修正 2 的離線孿生：恰 9 碼 / 非 class 9 / 全表唯一 / 可反解
python scripts\ops\gen-alid-catalog.py --audit
# 2) 工作簿 ALID 工作表列（CSV，UTF-8-BOM 給 Excel 直接吃）
python scripts\ops\gen-alid-catalog.py --emit-sheet-csv build\ALID_sheet_20260902.csv
# 3) 直接由 0831 底本生出 0902 工作簿（只動 ALID 工作表；散文格仍需手改）
python scripts\ops\gen-alid-catalog.py `
    --emit-xlsx "docs\SECS\SECS_GEM功能_Handler_20260902.xlsx" `
    --xlsx-base "docs\SECS\SECS_GEM功能_Handler_20260831.xlsx" `
    --expect-base-md5 e0ff9e2c182dc6521fd706a2db00abda
python scripts\ops\gen-alid-catalog.py --verify-png `
    "docs\SECS\SECS_GEM功能_Handler_20260831.xlsx" "docs\SECS\SECS_GEM功能_Handler_20260902.xlsx"
# 4) 模擬器（先 dry run 看 diff，再 --in-place）
python scripts\ops\gen-alid-catalog.py --emit-sim
python scripts\ops\gen-alid-catalog.py --emit-sim --in-place
# 5) 過期敘述獵捕（工作簿 + md/html + 模擬器全都吃）
python scripts\ops\gen-alid-catalog.py --check-prose docs\SECS\*.md docs\SECS\*.html "docs\SECS\SECS_GEM功能_Handler_20260902.xlsx"
# 6) CI：磁碟上的模擬器是否還與 CSV 同步（stale 就 exit 1）
python scripts\ops\gen-alid-catalog.py --check
# 7) 與外部產出的 old->new 對照表逐列交叉驗證
python scripts\ops\gen-alid-catalog.py --verify-table <mapping.csv>
```

### 1.3 本輪驗證結果（都在 `GENERATOR_RUN_20260902.log`）

* `--selftest`：20 個裁定範例**全對**（含 A5 的 `70000→700070000`、`80000→800080000`；
  1b 的 `MES01421→987714039`、`MES001421→928189725`、`JAM913→980936293`、`WAR09120→952507070`；
  `MES0000→300000000`；`SnFKCleanOut→991410149`）。
  結構掃描：class 4..8 全 50,000 個 5 碼數字碼 → 50,000 distinct、全 9 碼、反解 100% 相符；
  class 1/2/3 全 300,000 個標準形 → 300,000 distinct、零降級、反解 100% 相符；
  11 個必須降級的字串全部落到 class 9。**failures = 0。**
* `--audit`：485 列／485 distinct／全 9 碼／485 筆反解相符／**0 筆 > 2^31-1**／違規 0。
  順帶量到 `ALTX > 40 字元 = 447 筆（最長 82）`——**另案，本包不修**。
* `--verify-table` 對 p3 那份權威表：**485 列比對，0 不符、0 缺、0 多**（class / payload / NewALID / OldALID 四欄全對）。
* `--emit-sheet-csv` / `--emit-map-csv` / `--emit-sim-block`：各 486 / 486 / 487 行；
  md5 `797c9218bd49b738081fd069986bfb29` / `12018413594cbf54793faeaba9b38ffc` / `3706cd5bc38a10a525da0e09a2580753`。
* `--emit-xlsx` 乾跑（輸出在 scratchpad，未動 repo）：ALID 工作表 485 列（2..486）、表尾註移到 `A488`/`A489`、
  `--verify-png` 兩張 PNG md5 `297302cc48f8cb168239958cbac6b2ff` / `fc4f464d16ddd13bc7b0dbd0e2424cfe` **逐位元相同**。
* `--emit-sim` 乾跑：找到並替換 `ALID_CATALOG`（485 tuples）與 `ALARM_CATALOG_ROWS=485`，無 anchor 失配。

### 1.4 產生器內部與 BCB6 移植的對應（BCB6 端請照抄這三段）

```
canonical_tail(tail)   ->  修正 1b：len==4 && v<10000  或  len==5 && v>=10000
classify(bytes)        ->  shape A（恰 5 碼數字、首字 4..8）優先，再 shape B（3 大寫 + 全數字尾）
decode(alid)           ->  class 1/2/3 補零寬度 = (payload < 10000 ? 4 : 5)
```
`AnsiString` 是 **1-based**（`Code[1]..Code[Length()]`），移植時索引全部 +1；不得用 C++11。

### 1.5 CI／防再度手改

1. 在 `scripts\ops\gen-alid-catalog.py` 檔頭已寫明「本檔必須與 `UsecegemMainFrom.cpp:167-177` 同步」。
2. 在 `secs_host_simulator.py` 的 `ALID_CATALOG` 上方加一行
   `# GENERATED by scripts/ops/gen-alid-catalog.py --emit-sim --in-place ; DO NOT HAND-EDIT`。
3. 建議把 `--check` 掛進既有的 pre-commit／`scripts\ops\` 檢查群（與 `check-ht160s-source-encoding.ps1` 同一輪跑），
   在 `AlarmList.csv` 變動而模擬器沒跟上時 exit 1。

---

## 2. (a) `SECS_GEM功能_Handler_20260902.xlsx`（由 0831 底本產生）

### 2.0 前置

1. 關閉 Excel（消掉 `~$SECS_GEM功能_Handler_20260831.xlsx`）。
2. `md5sum` 底本 = `e0ff9e2c182dc6521fd706a2db00abda`（產生器 `--expect-base-md5` 會再擋一次）。
3. **先複製成新檔名再改**：`docs\SECS\SECS_GEM功能_Handler_20260902.xlsx`。0831 一律不覆寫，退為歷史。
4. openpyxl 只讀底本、寫新檔（產生器就是這樣做的）。

### 2.1 openpyxl 存檔的實測副作用（本輪逐格量測，全部可接受，但要知道）

| 現象 | 實測 | 判定 |
|---|---|---|
| 兩張內嵌 PNG | `xl/media/image1.png`/`image2.png` md5 **不變**；drawing rel 由 `sheet1.xml.rels` 移到 `sheet2.xml.rels` | **無害**：底本是 Excel 依「建立順序」編號（`sheet1.xml` 就是「功能」頁），openpyxl 改依「頁籤順序」編號（`sheet1`=修訂說明、`sheet2`=功能），圖仍掛在「功能」頁同一位置 |
| 未動的工作表 | 「功能」0 格差異、SVID 0、CEID 0；**「修訂說明」4 格、ECID 5 格**差異 | **無害**：全部是 `''`（空字串）→ `None`，openpyxl 丟掉空的 inline string，畫面無差別 |
| 合併儲存格 | 底本 `ALID!A484:E484` → 新檔 `ALID!A488:G488`（隨新欄寬到 G） | 預期 |
| 凍結窗格 | 底本 `ALID` 頁是 `A269`（明顯是誤操作留下的）→ 新檔設為 `A2` | **順手修正**，請在修訂說明裡記一句 |
| 欄寬 | `insert_cols` 只搬儲存格不搬 `column_dimensions`，產生器已重新指定 7 欄寬 A14/B9/C15/D18/E8/F16/G86 | 已處理 |

### 2.2 `ALID` 工作表 — 由產生器重建（`--emit-xlsx`）

| 位置 | 動作 |
|---|---|
| `ALID!B:C` | **插入兩欄**（原 B..E 右移成 D..G） |
| `ALID!B1` | `號段 Class`（沿用 `A1` 的表頭樣式：Arial 粗體、底色 `FF1F6FEB`、置中、wrap） |
| `ALID!C1` | `號段內碼 Payload`（同上） |
| `ALID!A1` | `ALID` 不變 |
| `ALID!D1` / `E1` / `F1` / `G1` | 原 `警報碼 AlarmCode` / `ALCD` / `類別` / `目錄訊息 ALTX (...)` 原文不變（只是位移） |
| `ALID!A2:G486` | **485 列全部重寫**（原 481 列 → 485 列）。欄序＝ ALID, Class, Payload, AlarmCode, ALCD, 類別, ALTX。排序沿用原表的「碼字串字典序」＝ S5F6 回覆順序 |
| 新增 4 列的落點 | `A443` `MES1025`→`300001025`（前 `MES1024`、後 `MES1120`）；`A461` `MES1428`→`300001428`（前 `MES1427`、後 `MES1520`）；`A471` `MES1921`→`300001921`（前 `MES1723`、後 `WAR0154`）；`A477` `WAR0963`→`200000963`（前 `WAR0962`、後 `WAR0970`） |
| ⚠ 4 列的 ALCD | 四筆的 `AlarmType` 都是 **1** → `ALCD=1`、類別欄 `Message 訊息`。注意 `WAR0963` 因此與其他 13 支 `WAR` 碼（ALCD=8 `Other 其他警示`）**不同類別**，這是警報表的既有資料，不是打錯 |
| `ALID!A487` | 空白間隔列（原 `A483`） |
| `ALID!A488` | 原 `A484` 表尾註第 1 段 → **全段改寫**（實測含 `雜湊` ×3、`無號` ×1、`2^31` ×1、`3184282107` ×1、`2054803979` ×1、`2502907625` ×1、`481` ×2），見 §2.3 |
| `ALID!A489` | 原 `A485` 表尾註第 2 段 → **定點修改**（實測含 `481` ×2、`3891410149` ×1），見 §2.4 |
| 凍結窗格 | `A2` |

### 2.3 `ALID!A488`（原 `A484`）— 全段改寫草稿（繁中）

> 註(2026-09-02)：本表為 S5F5 → S5F6 全查回覆的完整警報目錄，共 **485 筆**，順序與回覆完全相同；
> S5F7 → S5F8「已啟用清單」亦為同一份 485 筆(本機無 per-ALID 啟用表，所有警報恆為啟用，故 S5F3 的停用請求會被受理但不生效)。
> **⚠⚠ 本版起 ALID 的編碼規則全面更換，請整批換表 —— 舊版的「警報碼字串 31 進位雜湊」規則自本版作廢。**
> 新規則：`ALID = 號段(Class) × 100,000,000 + 號段內碼(Payload)`，**恆為 9 位十進位數字**，仍以 `UINT_4` 送出。
> 號段（＝ALID 首位數字，本表 B 欄）：`1`=JAM、`2`=WAR、`3`=MES —— Payload ＝三字母之後的數字尾(恆為 4 位或 5 位)；
> `4`=汽缸、`5`=馬達、`6`=真空、`7`=紀錄流程、`8`=其他 —— 碼本身恰 5 位純數字，Payload ＝**整個碼值**；
> `9`=尚未登錄成警報碼的自由字串警報 —— Payload 為舊雜湊折成 8 位，**語意＝「此 ALID 不在本目錄內」**；`0` 永不出現。
> **貴端反解**：`class = ALID / 100000000`；`payload = ALID % 100000000`；
> class 1/2/3 → 前綴字母 + payload（payload < 10000 補零至 4 位，否則 5 位）；class 4-8 → payload 補零至 5 位即為警報碼；
> class 9 → 不在目錄，請取 ALTX 開頭的第一個 token 當代碼顯示。
> 例：`40000`→`400040000`、`50198`→`500050198`、`60005`→`600060005`、`JAM0913`→`100000913`、
> `WAR0963`→`200000963`、`WAR16120`→`200016120`、`MES0920`→`300000920`、`MES1421`→`300001421`。
> **⚠ 本目錄 485 筆的值域為 `100000913` ～ `600060005`，規則上限 `999999999`，全部落在有號 32 位元範圍內 ——
> 舊版「其中 14 筆超過 2^31-1(最大 3184282107)，請以無號 32 位元解析」的警告自本版起完全作廢，
> 請一併移除貴端為此加的特別處理。**
> ⚠ 命名規範（影響日後新增碼）：JAM/WAR/MES 的數字尾只接受「4 位」或「5 位且數值 ≥ 10000」兩種標準形；
> 因此 unit 01-09 只能寫成 `0UNN`（每個 unit 上限 100 支），`0UNNN`(例 `WAR09120`) 為非法字串、會落到 class 9 而無法反解；
> unit 10-99 可用 `UUNN`(100 支)或 `UUNNN`(1000 支)。號段容量：class 1/2/3 各 100,000 槽、class 4-8 各 10,000 槽。
> ⚠⚠ 最容易誤判的一點：**S5F1 事件的 ALCD 不帶類別** —— 警報發生固定送 0x80、解除固定送 0x00，低 7 位恆為 0；
> 要分類請以 ALID 對本表查詢(本表 ALCD 欄才是類別值本身)，或直接讀 ALID 的首位數字(B 欄 Class)。
> **這一點與 HT-9046LS 不同**：9046 的 S5F1 送的是「類別 + 0x80」，沿用 9046「減 0x80 取類別」的解析會把本機警報全部誤判為類別 0(Jam)，請勿沿用。
> ⚠ 另有兩類警報會上報 S5F1 但 **ALID 為 class 9、不在本目錄內**，host 端請預留「class 9 → 顯示 ALTX 原文」的處理路徑、勿視為通訊錯誤：
> (1) 吸嘴碼族 `SUC<3 位索引><1 位錯誤類型>` 共 12 個(分選臂 4 支吸嘴 × 取料真空失敗／未脫離／途中掉料) ——
> `SUC` 不是本規則的三個前綴之一，故一律走 class 9(例 `SUC0011` → `945192897`)，其 ALTX 為「<警報碼> <吸嘴名> Sucker Error」，可直接顯示給操作員；
> (2) 尚未登錄成警報碼的自由字串警報(安全門、緊急停止、馬達電源、離子風扇、氣壓、各軸 `<軸別名>_MotOverLimitErr` 超程、
> 以及分選臂／TrayArm 的英文長句)，其 Payload 會隨字串修訂而改變(例 `SnFKCleanOut` → `991410149`、`Safety Door Open` → `945115854`)。
> ⚠ 其中「`Loader Tray has IC,please remove`」(逗號後沒有空格)是以介面語言的翻譯結果當警報碼，
> 同一支警報在英文與中文介面的 ALID 不同：英文介面 class 9 = `954803979`；中文介面則是譯文 `Loader 盤有 IC，請移除` 之 Big5(CP950) 位元組經同一套折算的結果，
> ⚠ 該值會隨 `system\language_phrases.txt` 的譯文修訂而改變，請勿在 host 端寫死。
> ⚠ 這 47 支自由字串警報**目前未登錄**，日後若正式編成 MES/WAR 碼，其 ALID **會再變一次**(從 class 9 變成 class 2/3)，屆時本方會另行通知。

### 2.4 `ALID!A489`（原 `A485`）— 定點修改（其餘原文保留）

| 原文片段 | 改為 |
|---|---|
| `本機目前已登錄的 481 筆` | `本機目前已登錄的 485 筆` |
| `ALID=3891410149 / ALTX="SnFKCleanOut SnFKCleanOut"` | `ALID=991410149(class 9) / ALTX="SnFKCleanOut SnFKCleanOut"` |
| `該號碼不在本目錄的 481 筆之內` | `該號碼為 class 9、不在本目錄的 485 筆之內` |
| 其餘三項 ALTX 差異敘述 | **不動**（S5F1 vs 目錄 ALTX 的三項差異與編碼無關，仍然正確） |

### 2.5 `功能` 工作表 — 4 段散文格

| 儲存格 | 動作 | 理由（本輪 `--check-prose` 實測命中） |
|---|---|---|
| `功能!E16`（No 14 / S5F1 Alarm Report Send，1713 字） | **全段改寫**，草稿見 §2.6 | 命中（實測逐字計數）：`雜湊` ×5、`無號` ×1、`2^31` ×1、`3184282107` ×1、`2054803979` ×1、`481` ×1 |
| `功能!E18`（No 16 / S5F5 List Alarms Request，624 字） | 定點：**全格共 6 處 `481`** 全改 `485`；`與本機 system\AlarmList.csv 的 481 筆逐筆相同` → `485 筆`；末句「請併同『ALID』工作表表頭第 4 段…」→ 改指「表尾註」並補一句「本表新增『號段 Class』『號段內碼 Payload』兩欄，可直接用於反解自我驗證」 | 命中：`481` ×6（實測逐字計數） |
| `功能!E31`（資料收集 No 8 / 機台Alarm回傳，260 字） | 定點：`目前 481 筆` → `目前 485 筆`；括號內「(第 1 版寫的 576 筆為 2026-07-20 移除 16 支未安裝汽缸之前的數字)」保留，後面補「(2026-09-02 補進機台已有而表上缺的 4 筆：MES1025 / MES1428 / MES1921 / WAR0963)」 | 命中：`481` ×1 |
| `功能!E17`（No 15 / S5F3） | **不動** | 只提到「ALED 與 ALID 清單會被忽略」，與編碼無關 |

### 2.6 `功能!E16` — 全段改寫草稿（繁中）

> [20260729] 已實作。警報發生／解除各送一次；ALCD bit7 0x80=set / 0x00=clear。
> **[20260902 改版：ALID 編碼全面更換，舊的 31 進位字串雜湊規則作廢，請整批換表]**
> ALID 改為「號段 + 碼值」的固定 **9 位**十進位數字，仍以 `UINT_4` 送出：
> `ALID = Class × 100,000,000 + Payload`。
> Class（首位數字）：`1`=JAM、`2`=WAR、`3`=MES(Payload ＝三字母後的數字尾，恆為 4 位或 5 位)；
> `4`=汽缸、`5`=馬達、`6`=真空、`7`=紀錄流程、`8`=其他(碼本身恰 5 位純數字，Payload ＝整個碼值)；
> `9`=尚未登錄的自由字串警報(Payload 為舊雜湊折 8 位，語意＝**此 ALID 不在 S5F6/S5F8 目錄內**)；`0` 永不出現。
> 反解：`class = ALID / 100000000`、`payload = ALID % 100000000`；class 1/2/3 → 前綴 + payload(< 10000 補零至 4 位，否則 5 位)；
> class 4-8 → payload 補零至 5 位即為警報碼；class 9 → 不在目錄，請讀 ALTX 的第一個 token。
> 例：`40000`→`400040000`、`50000`→`500050000`、`60000`→`600060000`、
> `JAM0913`→`100000913`、`WAR0963`→`200000963`、`WAR16120`→`200016120`、`MES0920`→`300000920`、`MES1421`→`300001421`。
> **⚠ 目錄 485 筆的值域 `100000913`～`600060005`、規則上限 `999999999`，兩端都在有號 32 位元內 ——
> 前一版「其中 14 筆超過 2^31-1(最大 3184282107)，host 端請以無號 32 位元解析」的警告自本版**完全作廢**，請移除相關特別處理。**
> S5F1 與 S5F6/S5F8 共用同一個產生函式，故同一支警報在事件與目錄中的 ALID 恆等。
> 完整 485 筆已列於本檔「ALID」工作表(第 6 張)，並新增「號段 Class」「號段內碼 Payload」兩欄供反解對照。
> ⚠ 例外(請務必看「ALID」工作表表尾註)：兩類警報的 ALID 為 class 9、不在目錄內 ——
> (1) 吸嘴真空家族以字串碼 `SUC<吸嘴索引><錯誤別>` 上報(共 12 個；`SUC` 不是本規則的三個前綴，故走 class 9，目錄裡的 60000-60005 實機不會送)；
> (2) 尚未登錄成警報碼的自由字串警報(安全門／緊急停止／馬達電源／氣壓／離子風扇、各軸 `<軸別>_MotOverLimitErr` 超程、部分英文長句)，
> 其 Payload 會隨字串修訂而改變，其中一支還隨介面語言改變。host 端請保留「class 9 就顯示 ALTX 原文」的 fallback。
> ⚠ 命名規範：JAM/WAR/MES 的數字尾只接受 4 位、或 5 位且 ≥ 10000；unit 01-09 因此每個 unit 上限 100 支(`0UNN`)，
> `WAR09120` 這種寫法非法(會落 class 9)，需改用 unit ≥ 10 的編號。
> ⚠ 另請注意 S5F1 的 ALCD 不帶類別(發生 0x80／解除 0x00，低 7 位恆為 0)，要分類請以 ALID 查「ALID」工作表或直接讀首位數字。
> ——以下 [20260805] 三項 ALTX 特性維持原文，僅把語言相依那一支的實測值由 `2054803979` 改為 `954803979`(class 9)。

### 2.7 `修訂說明` 工作表

| 儲存格 | 現值 | 新值 |
|---|---|---|
| `B3` | `2026-08-31（2026-09-01 措辭補充…）` | `2026-09-02` |
| `B4` | `SECS_GEM功能_Handler_20260831.xlsx｜本方只維護這一個檔案…` | `SECS_GEM功能_Handler_20260902.xlsx｜本方只維護這一個檔案。舊日期檔名（20260729 / 20260802 / 20260804 / HandlerV1_20260810 / HandlerV1_20260813 / **20260831**）留作歷史，不再更新。` |
| `B5` | `SECS_GEM功能_HandlerV1_20260813.xlsx` | `SECS_GEM功能_Handler_20260831.xlsx` |
| `B6` | `... commit 7f7b391` | 改成含 `ComputeAlarmAlid()` Option D 的那個 commit（**待韌體改完才填**，先留 `TBD`） |
| `A8`/`B8` | 0831 的第 1 項 | **`1. ⚠⚠ ALID 編碼全面更換（破壞性）`** ／ 規則、反解、值域、無號警告作廢、S5F1 與目錄同號的說明；明寫「舊表全部作廢，請整批換表」與「換表時點須與韌體版本對齊」 |
| `A9`/`B9` | 0831 第 2 項 | **`2. ALID 目錄 481 → 485 筆`** ／ 補進機台已有而表上缺的 `MES1025` / `MES1428` / `MES1921` / `WAR0963`（四筆 ALCD=1 Message）；說明「舊表少 4 筆是本方漏更新，不是機台改版」 |
| `A10`/`B10` | 0831 第 3 項 | **`3. ALID 工作表新增 Class / Payload 兩欄`** ／ 供貴端反解自我驗證；順帶修正舊檔誤設在 `A269` 的凍結窗格 |
| `A11`/`B11` | 0831 第 4 項 | **`4. 「須以無號 32 位元解析」警告作廢`** ／ 新編碼上限 `999999999`，全部在有號 32 位元內；請移除為此加的特別處理 |
| `A12`/`B12` | 0831 第 5 項 | **`5. 警報碼命名規範（影響日後新增碼）`** ／ 4 位或 5 位(≥10000)標準形；unit 01-09 上限 100 支；`WAR09120` 非法 |
| `A13`/`B13` | `尚待貴端確認 Open items` | 保留標題，內容改為：① 換表時點與韌體版本的對齊方式；② 47 支 class 9 自由字串日後若正式編碼，ALID 會再變一次，是否要一次做完再交表；③ ALTX 超過 40 字元（447/485，最長 82）本方另案處理，貴端 EAP 目前如何截斷 |

### 2.8 完工驗收（工作簿）

```powershell
python scripts\ops\gen-alid-catalog.py --check-prose "docs\SECS\SECS_GEM功能_Handler_20260902.xlsx"   # 必須 0 hit
python scripts\ops\gen-alid-catalog.py --verify-png "docs\SECS\SECS_GEM功能_Handler_20260831.xlsx" "docs\SECS\SECS_GEM功能_Handler_20260902.xlsx"
```
另以 openpyxl 逐格 diff 新舊檔，確認除 `ALID`、`功能!E16/E18/E31`、`修訂說明` 以外零差異
（本輪已驗：未動的頁只會出現 `''`→`None` 的空字串差異，SVID/CEID/功能 為 0 格）。
最後**用 Excel 開一次**、目視 ALID 頁前後 5 列與兩張 PNG，再存檔（讓 Excel 重寫成它自己的格式），
交付前用 `docs\SECS\_ZipFile.bat`（7z + 密碼 `BVL-3766`、`-mhe=on`）打包。

---

## 3. (b) `docs\SECS\HT160S_SECS_Comm_Examples.md` / `.html`

`.md` 是真源，`.html` 由 python-markdown 重生（見 §4.2 配方）。本文件釘在 2026-06-26 的逐字 case log，
**依既有維護規則：不得偽造新 log**——log 行只加註「該 run 為舊編碼」，不改數字。

| 行 | 現況 | 動作 |
|---|---|---|
| `:781` | `ALID 推導`：31-poly rolling hash + 引用 `UsecegemMainFrom.cpp:150-152` | **改寫整段**為 Option D 規則（照 §2.6 精簡版），並把出處改為 `UsecegemMainFrom.cpp:167-177`（`ComputeAlarmAlid()`，宣告在 `.h:37`）。加一句「本機唯一產生者，兩個呼叫點：`:187`(S5F1) 與 `uHGemHT160.cpp:3499`(S5F6/S5F8)，故事件與目錄同號」 |
| `:783-786` | 兩列對照表 `"Loader Tray Empty"`=`4045923824`、`"SnFKCleanOut"`=`3891410149` | **換成新表**：`MES0920`→`300000920`(class 3；⚠ 2026-06-26 那次 run 送的是自由字串 `"Loader Tray Empty"`，該訊息現已登錄成 `MES0920`)、`SnFKCleanOut`→`991410149`(class 9)、`40000`→`400040000`(class 4)、`WAR16120`→`200016120`(class 2 五位尾數)。表頭加 `Class` / `Payload` 兩欄 |
| `:788` | `ALTX 的內容範圍` blockquote，引用 `:154-156` | 保留內容，行號改為現行 `UsecegemMainFrom.cpp` 對應行（改韌體時一併確認） |
| `:793-795` | `為什麼 ALID 用字串 hash、而不是固定編號？` blockquote | **整段改寫**：理由已變。新文＝「ALID 現在是可反解的號段編碼，host 可直接由 ALID 還原警報碼；class 9 才是『不在目錄、請讀 ALTX 開頭 token』」。並保留「以 ALID 對表、不要以 ALTX 字串比對」的建議 |
| `:805-810` | 設備端 log 4 行（`4045923824`、`3891410149`） | **不改 log**。在 code fence 之後加一行註記：「⚠ 本段為 2026-06-26 舊編碼(31 進位雜湊)之實測 log；自 Option D 韌體起，同兩支警報的 ALID 分別為 `300000920`(該訊息已登錄成 `MES0920`)與 `991410149`」 |
| `:829` | FIELD TABLE 的 `ALID` 列：`由 ALTX 字串 31-poly hash 而來, deterministic` + 兩個舊值 | 改為「U4，`Class × 1e8 + Payload` 的 9 位號段編碼，可反解；本 run 的值為舊編碼」並附新舊對照 |
| `:1011-1013` | 時間軸表 3 行帶舊 ALID | 同 `:805-810` 處理：表格下方加一行「舊編碼」註記，數字不動 |
| `:1075` | 名詞表 `ALID｜本機由 ALTX 字串 31-poly hash 而來的 U4` | 改為「Alarm ID；`Class × 100,000,000 + Payload` 的 9 位 U4，可由 ALID 反解警報碼；class 9 ＝不在 S5F6 目錄」 |
| `.html` | `:1420`、`:1431`、`:1435`、`:1440`、`:1453`、`:1456-1458`、`:1489-1490`、`:1929`、`:1939`、`:2073` | **不手改**，改完 `.md` 後整檔重生（§4.2） |

---

## 4. (c) `docs\SECS\HT160S_SECS_Interface_Spec_20260727.md` / `.html`

| 行 | 現況 | 動作 |
|---|---|---|
| `:8`、`:13`、`:279`、`:374` | 引用 `SECS_GEM功能_Handler_20260831.xlsx` 為唯一權威 | 4 處全改 `SECS_GEM功能_Handler_20260902.xlsx`（`:8` 另把「四張表」維持，因 ALID 頁欄數變但張數不變） |
| `:345` | `66011 Alarm Code｜無替代,同上——警報碼即 S5F1 的 ALID` | **改寫**：`警報碼即 ALID` 在 Option D 下**不成立**。新文：「**無替代**。警報碼**可由 ALID 反解**但不等於 ALID：`class = ALID/1e8`、`payload = ALID%1e8`，class 1/2/3 → 前綴 + payload(4 或 5 位補零)、class 4-8 → payload 補零至 5 位；class 9 表示該警報未登錄，請讀 ALTX 開頭 token。詳見 §3.5 與工作簿『ALID』頁」 |
| `:855-859`（§3.5 警報/Alarms） | `約 480+ 碼`、只講欄位與 ALTX | **改寫整節**：① 目錄 **485 筆**（非「約 480+」）；② 完整 ALID 規則表（class 0-9 + 反解四行）；③ 值域 `100000913`～`600060005`、上限 `999999999`、**有號 32 位元相容**；④ class 9 語意與兩類例外（`SUC` 12 個 + 47 支自由字串）；⑤ 命名規範（4/5 位標準形、unit 01-09 上限 100）；⑥ 指向工作簿 ALID 頁的 Class/Payload 欄 |
| `:904-905` | HT-90XX 的 `One cycle finish` = ALID `316001640`，HT-160S 不發 | 保留，並補一句互斥性保證：「該號碼的第 2-3 位不是 `00`(payload 16,001,640 > 99,999)，本機 Option D **結構上不可能產生**，故不會與貴端 9046LS 字典撞號」 |
| `.html` `:1005`、`:2258` | 對應 `:345` 與 `:857` | 重生（§4.2） |

### 4.2 `.html` 重生配方（兩本共用，已驗 byte-exact）

1. `python-markdown`，extensions = `tables, fenced_code, toc, sane_lists, attr_list`，
   **`output_format="xhtml"`**（html5 會產 `<hr>` 造成假 diff）。
2. 外殼**逐字沿用目標 `.html` 自己的**：切 `^(.*?<body>)(\r?\n?)` 當 prefix、從 `</body>` 起當 suffix，
   並保留 `<body>` 之後原有的分隔字元（commit 版是 `<body><h1`，工作區版是 `<body>\n<h1`）。
3. `Comm_Examples` 另需後處理：把 ```mermaid fenced block 轉成 `<pre class="mermaid">` 並 unescape；
   其外殼含 HONTECH `<header>` + 紅色 CSS + mermaid CDN + `<footer>`。
4. **必做驗證**：先用 `git show HEAD:<md>` 重生 → 與 `git show HEAD:<html>` `cmp` 出 byte-exact，
   才動真檔（記憶 `secs-comm-examples-doc-maintenance` 記錄過一次「EXACT MATCH」是假訊號）。
5. 產生器不在 repo（歷史上寫在 scratchpad）。**建議這次順手把它入庫**成
   `scripts\ops\build-secs-html.py`，否則下次又要重寫一遍。

---

## 5. (d) 模擬器 `D:\AI_Area\Tool\HT160S_SECS_Simulator\code\`

### 5.1 `secs_host_simulator.py`（2791 行）

| 行 | 內容 | 動作 |
|---|---|---|
| `:669-690` | ALID 目錄檔頭註解（中英雙段）：`481 筆`、31 進位雜湊、`3184282107`、無號 32 位元 | **整塊改寫**：Option D 規則 + class 語意 + 「值域全在有號 32 位元內」；來源改為 **`system\AlarmList.csv` 經 `scripts/ops/gen-alid-catalog.py --emit-sim` 產生**（不再寫「由 V1_ALID.csv 手抄」）；加 `# GENERATED -- DO NOT HAND-EDIT` |
| `:694-1176` | `ALID_CATALOG` 481 tuples | `--emit-sim --in-place` 重生為 **485 tuples**（本輪 dry run 已驗 anchor 命中） |
| `:1178-1182` | 衍生表 `ALID_NAMES` / `ALID_ALCD` / `ALID_OVER_SIGNED` / `ALID_MAX` | 保留前兩張；**刪除 `ALID_OVER_SIGNED`**（9 碼上限下恆為空清單，留著只會誤導）；`ALID_MAX` 保留；**新增** `alid_class(a)` / `alid_payload(a)` / `alid_decode(a)` 三個小函式（照 §1.4 的 `decode()`） |
| `:1257-1268` | `_alid_gloss()`：未知號碼印「不在 481 筆目錄內」 | 改為先印 class/payload 反解：`ALID 300000920 = class 3 MES / payload 920 -> MES0920 [catalog ALCD=1 Message]`；未知號碼時，若 class 1-8 印「不在 485 筆目錄內，但可反解為 <code> —— 機台送了一個未登錄的號碼，請查警報表」；class 9 印「class 9 ＝依設計不在目錄，請讀 ALTX 開頭 token（SUC 家族或自由字串警報）」 |
| `:1271-1286` | `_alid_sign_line()`：負值／無號 32 位元自檢 | **整支換掉**成 `_alid_shape_line()`：檢查 ① 每個 ALID 恰 9 位（`100000000..999999999`）② class 不為 0 ③ 統計本次出現的 class 分布 ④ 目錄內的號碼不應為 class 9。負值檢查可留一行（真的收到負值＝某端存成有號 I4，仍是缺陷），但**不得再宣稱「目錄有 14 筆超過 2^31-1」** |
| `:1304` | S5F1 加註呼叫 `_alid_sign_line([alid])` | 改呼叫 `_alid_shape_line` |
| `:1309-1349` | `_annotate_alarm_list()`：S5F6/S5F8 逐列對帳 | `481` → `485`（註解與訊息）；`_alid_sign_line(seen)` → `_alid_shape_line(seen)`；「筆數不符」訊息補一句「或韌體尚未換到 Option D（收到 8/10 位數的 ALID 即是舊韌體）」——**這是換版期間最有用的一行** |
| `:1376` | `_first_ids()` 註解 `481 筆全印會洗版` | → `485 筆` |
| `:1401-1402` | `annotate_known_ids()` docstring `與本地 481 筆目錄對帳` | → `485` |
| `:1128` | `(3184282107, 0, "JAM1611 ...")` | 屬 `ALID_CATALOG` 資料列，由產生器重寫成 `(100001611, 0, ...)`，**不手改** |
| `:1180` | `ALID_OVER_SIGNED` 上方註解 | 隨 `:1178-1182` 一起刪 |

### 5.2 其他模擬器檔

| 檔案:行 | 動作 |
|---|---|
| `ht160s_presets.py:1084` | S5F3 preset 的 `expect`：`（481 筆）` → `（485 筆）` |
| `ht160s_presets.py:1424-1431` | S5F5 preset：`n = 【481】` → `【485】`（`:1425`）；`與機台 system\AlarmList.csv 逐筆相同；同一版韌體恆為 481` → `485`（`:1426`）；**刪掉 `:1428-1429` 整段「ALID 必須以無號 32 位元解析…最大 3184282107」**，換成「⚠ ALID 恆為 9 位；請以 `class = ALID/1e8` 反解並與 D 欄警報碼對照」 |
| `ht160s_presets.py:1435-1439` | S5F7 preset：兩處 `481` → `485` |
| `scenario_runner.py:266` | `ALARM_CATALOG_ROWS = 481` → `485`（產生器 `--emit-sim` 自動改，已驗） |
| `scenario_runner.py:1368-1379` | 情境步驟 `manual` 文字：刪掉 `:1375-1377` 的 unsigned-32 段，換成「Write the row count down (485). ALSO: every ALID must be exactly 9 digits; decode class = ALID/100000000 and check it against the code in the ALTX leading token.」 |
| `docs\SECS_MESSAGES.md:78,79,179,377,378` | 5 處 `481` → `485`；`:78` 的「ALID 須以**無號 32 位元**解析」→「ALID 恆為 9 位，`class×1e8+payload` 可反解」 |
| `docs\SECS_MESSAGES.md:396` | 整段 blockquote 改寫（含 `無號 32 位元`、`2^31-1`、`3184282107`、`481`） |
| `docs\README.md:101,145,270,271,305,321,461` | 6 處 `481` → `485`；`:271` 的「並提醒 ALID 須以無號 32 位元解析」→「並印出 class/payload 反解與 9 位長度檢查」 |
| `docs\COMM_OPERATIONS_REFERENCE.md:44` | **不動**（`無號整數（1/2/4 bytes）` 講的是 U1/U2/U4 型別，`--check-prose` 的誤命中） |
| `code\catalog_audit.py` | **不動**（只驗 SVID/ECID/CEID）。可選：日後加 ALID 一節，本包不做 |
| `build.ps1` / `dist\` | 文字改完後重新 PyInstaller 打包（`ALID_CATALOG` 是常數，打包才會生效）。`build\` 與 `dist\` 是產出物，不需人工改 |

---

## 6. (e) 記憶與 `.github`

### 6.1 `memory\secs-alid-hash-encoding.md`（lead 已改寫）— 本輪逐句核對結果

**一致（無需再動）**：權威指向 `docs\plan\alid-option-d-ratified-spec-20260902.md`；
`ComputeAlarmAlid()` 位置 `:167-177`；規則四行（含 **A5 的 4/5/6/7/8**）；
範例 `40000/MES1421/JAM0913/WAR16120/SnFKCleanOut` 五個值我逐一重算**全對**；
`min 100000913 / max 999752848`、有號 int32 相容；修正 1/1b/2 三段（含「不可用拒絕前導零」與 unit 01-09 上限 100）；
9046LS 互斥性依據；「只有一個發射點、`uHGemHT160.cpp:3499` 不需改」。

**建議補三句（都是本輪新確認的事實，不是改判）**：
1. 修正 1b 之下，**修正 1 永遠不會觸發**（尾數被限成 4 或 5 位 → payload ≤ 99,999）；BCB6 端仍保留當防禦性守衛，但別寫成「活規則」，也別寫會期待它觸發的測試。
2. class 7/8 目前**沒有任何產生器會鑄出**（`database.cpp:832/887/919` 只產 `%d%03d%1d` 的 4/5/6 家族）→ **可反解但不可達**，不要對客戶說 7/8 已在用。
3. 容量：class 1/2/3 各 **100,000** 槽（不是規格書 §4 寫的 1,000,000）；class 4-8 各 10,000 槽。
4. 新增一行工具指標：`scripts\ops\gen-alid-catalog.py` 是 ALID 表的唯一產生器（`--audit` 是修正 2 的離線孿生）。

`memory\secs-workbook-single-file-0804.md` 需把 SSOT 由 `Handler_20260831.xlsx` 更新為 `Handler_20260902.xlsx`
（並保留「動手前先 md5 比對底本」那句，本輪再次證明有用）。

### 6.2 `.github` — 實測**沒有任何 ALID 規則敘述**

`grep -rn "ALID" .github/ .claude/skills/` 只命中兩處與規則無關的行
（`ht160s-state-record-analysis/SKILL.md:179` 提到 `AlarmReport(Code,...)` 會發 S5F1 ALID；
`ht160s-topccd-socket/SKILL.md:110` 是 `INVALID_SOCKET` 的假命中）。
`.github/skills/ht160s-secsgem/SKILL.md` 完全沒提 ALID 或工作簿檔名（`:282` 只說 `AddAlarmList()` 目前仍空，那句本身已過期）。
→ **`.github` 不需要為 Option D 改任何規則文字**；建議只加一句指標到 `ht160s-secsgem/SKILL.md`：
「ALID 規則與產生器：`docs/plan/alid-option-d-ratified-spec-20260902.md` + `scripts/ops/gen-alid-catalog.py`」，
順手修掉 `:282` 那句過期敘述（`AddAlarmList()` 早已實作，見 `uHGemHT160.cpp:3482` `EmitAlarmCatalog()`）。

---

## 7. 孿生檔 `SECS_GEM功能_Handler_20260831_manual.xlsx` 的處置（建議：退為歷史，不產 0902 版）

**它是什麼（本輪實測）**：0831 主檔的**客戶精簡版**——同樣的號碼，但拿掉全部長註解欄與內部頁：
5 張表（**無「修訂說明」頁**）、`功能!E` 全欄清空（`E16` 長度 0 vs 主檔 1713）、SVID 少 `E`/`F` 兩欄註解、
CEID 少 `D`/`E` 欄、ECID 少表尾註兩列。ALID 頁**與主檔幾乎逐格相同**（2424 格相同、只有 `E1` 表頭較短），
**因此它同樣帶著 `ALID!A484` 的舊雜湊敘述與 `481 筆`**。

**現況事實**：`git ls-files` 顯示所有 `*_manual.xlsx`（0804 / 0810 / 0813 / 0831）**全部未入庫**（untracked），
是本機手工產物；`_ZipFile.bat`（7z + 密碼 `BVL-3766`）也未入庫。裁剪動作沒有腳本。

**建議（待 owner 裁定）**：
1. **採單一工作簿政策：不再產 `_manual` 孿生**。0831_manual 與其他三份 `_manual` 一併移進
   `docs\SECS\history\`（或加 `_HISTORY_` 前綴），避免交付時誤抓到帶舊雜湊敘述的那份。
2. 若客戶確實要精簡版，**不要再手工裁**：在產生器加 `--strip-notes`（複製 0902 → 刪 `修訂說明` 頁、
   清 `功能!E`、SVID `E:F`、CEID `D:E`、ECID 表尾註，ALID 頁保留），這樣孿生永遠不會與主檔漂移。
   工時約 1.5 h，只有在 owner 說「客戶要精簡版」時才做。
3. **絕不可**只更新主檔而讓 0831_manual 留在同一個交付資料夾——那是客戶拿錯表的最大風險點。

---

## 8. 順序、相依與工時

### 8.1 相依鏈

```
S1 產生器入庫  ──►  S2 韌體改碼(ComputeAlarmAlid + 修正 2)  ──►  S3 雙建置 + 上機驗證
      │                                                              │
      ├──►  S4 工作簿 0902(表+散文+修訂說明)  ◄── 表由 S1 產出        │
      ├──►  S5 兩本 md/html                                          │
      ├──►  S6 模擬器 + 其 docs                                      │
      └──►  S7 記憶/.github 指標                                     │
                                                                     ▼
                                          S8 交付客戶(工作簿 + 換表時點)  ← 必須在 S3 之後
```
**S4/S5/S6 可以先做完並 commit**（它們描述的是即將上線的韌體），但 **S8 交付必須在 S3 上機驗證通過之後**，
且交付信要寫明「請於本方韌體版本 X 上線後同步換表」。

### 8.2 各步工時（含驗證，不含等機台）

| 步 | 內容 | 工時 |
|---|---|---|
| S1 | 產生器入庫 + `--check` 掛進檢查群 + 一輪全模式跑過 | 0.5 h（腳本已寫好並驗過） |
| S2 | `ComputeAlarmAlid()` 改寫 + `LegacyAlarmHash()` + 註解更新 + 修正 2 自檢（與 `database.cpp:1077-1101` 既有守衛整合、**並把它的 `'4','5','6'` 首字檢查放寬到 `'4'..'8'`**、通道升級為 `g_EventLog.Log()`） | 2.5 h |
| S3 | 刪 `.obj` → `-Clean`；再註解掉 `SOFT_SIMULATE` 跑 `-Full` exit 0 → 還原 define 重編 | 1.0 h |
| S4 | 工作簿：產生器出表 + 4 段散文改寫 + `A488`/`A489` 表尾註 + 修訂說明 6 列 + Excel 目視 + PNG/prose 驗收 | 3.5 h（散文是主要成本） |
| S5 | `Comm_Examples.md`（8 處）+ `Interface_Spec.md`（6 處）+ 兩檔 `.html` byte-exact 重生與驗證 | 2.5 h |
| S6 | 模擬器：`--emit-sim --in-place` + 檔頭/gloss/shape-line/對帳 5 段手改 + presets/runner/3 份 docs 共 20 處計數 + PyInstaller 重打包 | 2.5 h |
| S7 | `memory` 兩則（`secs-alid-hash-encoding` 補 4 句、`secs-workbook-single-file-0804` 換 SSOT 檔名）+ `.github/skills/ht160s-secsgem` 一行指標 | 0.5 h |
| S8 | 客戶交付：`_ZipFile.bat` 打包 + 換表說明信（可沿用 p3 的 `HT160S_ALID_Letter_KYEC_20260902.md` 草稿，需按 1b/A5 校訂） | 1.0 h |
| — | **合計** | **約 14 h（含 S2/S3 的韌體 3.5 h；純文件／工具 10.5 h）** |

### 8.3 不可提前的事項（紅線再述）

1. **S8 不得早於 S3。** 表先出去 = 客戶照新號建表、機台仍送舊號。
2. **孿生 `_manual` 的處置（§7）需 owner 先裁定**，再動檔案位置。
3. **命名規範（unit 01-09 上限 100）寫進客戶文件前需 owner 確認**，因為它約束的是本方日後的編碼自由度。
4. `ALTX > 40 字元`（447/485、最長 82）與 `aLoader.cpp:2442` 語言相依警報碼**都不在本包**，
   但兩者都會在客戶讀新表時被問到，**交付信要主動提一句「另案處理中」**。

---

## 9. 附帶發現（本輪新量到，非規格變更）

1. `ALID` 工作表的凍結窗格在 0831 檔是 `A269`（誤操作殘留）→ 新檔設 `A2`。
2. `WAR0963` 的 `AlarmType=1` → ALCD 1 / 類別「Message 訊息」，與其他 13 支 `WAR`（ALCD 8）不同，是既有資料。
3. `SUC` 12 支吸嘴碼在 Option D 走 **class 9 的 `unknown_prefix` 路徑**（例 `SUC0011` → `945192897`），
   不是 class 6；工作簿裡的 `60000-60005` 六筆實機不會送 —— 舊表就是這樣寫的，新表要沿用這個說明。
4. openpyxl 會把空字串儲存格變成 `None`（本輪只影響「修訂說明」4 格、ECID 5 格，畫面無差別）。
5. 兩張內嵌 PNG 在 openpyxl 存檔後 md5 不變，但 drawing 的 rel 由 `sheet1.xml.rels` 移到 `sheet2.xml.rels`
   ——只是 worksheet part 編號改依頁籤順序，圖仍在「功能」頁，**不是掉圖**。
