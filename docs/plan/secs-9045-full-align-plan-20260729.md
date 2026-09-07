# HT-160S 全面對齊 HT9045 — 剩餘缺口分析與執行計畫

- 日期：2026-07-29
- 分支：`feat/iosetview-172-refactor`
- 起點 HEAD：`ab1b99e`（CEID 字典整份照抄 9045 已 SHIPPED）
- 決策前提：**客戶（京元）已明確拒絕先前「HT-160S 自有字典為權威」的規格，要求全面對齊 HT9045。**
  本文因此不再討論「該不該對齊」，只分析「怎麼對齊、風險在哪、順序如何」。

## 證據來源（全部唯讀）

| 來源 | 路徑 |
|---|---|
| 9045 韌體原始碼 | `D:\HT9045\HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP\SECSGEM\uHGemHT9045.cpp` |
| 9045 XML lot 解析 | `D:\HT9045\HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP\BarCode\BarcodeXML.cpp:416` |
| 9045 CEID 目錄（韌體傾印） | `D:\backup_version\HT9046\KYEC\20260626\EventReport_CEID.def`（292 列） |
| 9045 ReportID 目錄 | `D:\backup_version\HT9046\KYEC\20260626\EventReport_ReportID.def`（**只有 1 列**） |
| 京元現場 log 表面 | `docs\plan\secs-9045kyec-diff-20260728\kyec9045_surface.json`（2026-06-08 全日） |

---

## 1. 現況盤點

### 1.1 已對齊（不需再動）

| 層 | 狀態 |
|---|---|
| CEID 號碼＋名稱＋enum（1–275） | ✅ `ab1b99e` 整份照抄，含 9045 自己的錯字與 142/143/144 無別名 |
| AMR 事件 272/273/274/275 | ✅ 同號同義、`DataID=1`（`372c9ad`） |
| Auto Full 35/36/37/148/149/150 | ✅ 皆 9045 既有編號 |
| Auto Unloadtray 136–138/145–147 | ✅ 皆 9045 既有編號（`bf9d048`） |
| AMR 上下料 IC-count 契約 | ✅ 上料只換盤數／下料才給 per-Auto 件數，兩邊一致 |
| 站台 SVID P1–P6 段 | ✅ 38202–38207 / 38222–38227 / 38228–38233 / 38234–38236 與 9045 逐號相同 |
| 標準 GEM 訊息 | ✅ S1F15/16、S2F17/18、S2F25/26、S2F33/35/37、S6F15/16、S6F19/20、S10F3–F6、S125F1/F2 |
| RCMD（12 個） | ✅ PAUSE/START/STOP/LOTSTART/ONLINE_REMOTE/ONLINE_LOCAL/START_AGV/ONE_CYCLE/ENERGY_SAVING/PP_SIGNALTOWER/PP_MUSIC |

### 1.2 尚未對齊（本文範圍）

| 編號 | 項目 | 風險 |
|---|---|---|
| **R1** | 報表連結層 L2/L3（`CEID i → RPTID i`） | 🔴 高 |
| **R2** | `SET_LOT_INFO` 同名不同體（且是不同的 lot 模型） | 🔴 高 |
| **R3** | 規格書 §3.3 仍是舊 1–31 語意 → 對客戶不實 | 🔴 高（必做） |
| **R4** | HT-160S 專屬 RCMD：`HOME` / `CLEARCOUNT` / `ONLINE`(裸) | 🟡 中 |
| **R5** | SVID：66000 段、站台延伸段、host RPTID 502 缺 7/8 格 | 🟡 中 |
| **R6** | 9045 有而 HT-160S 沒有的 35 個 RCMD | 🟡 中 / 🟢 低（分級） |
| **R7** | CEID 已註冊但無發射點 | 🟢 低 |

---

## 2. R1 — 報表連結層（最高風險，但前提被推翻了）

### 2.1 現況

`HT160Gem::AddCEID()`（`uHGemHT160.cpp`）：所有 CEID → **RPTID 1**（report 1 = 13 個 SVID 的機台上下文）；
另外 272→{2,6}、273→{3}、274→{4,6}、275→{7}。

`HT160Gem::AddReprot()`：自建 report 1–7，其中
- report 1 = `{1001, 1003, 1021, 1027, 66000, 66001, 66002, 66010, 66011, 66020, 66021, 66030, 66031}`
- report 2/3/4 = 單一 P1–P9 bitmap（38219 / 38220 / 38221）
- report 5 = 九站 carrier ID／report 6 = 九站 tray+device count／report 7 = 身分盤 2D（38204）

### 2.2 9045 實際做法

- `HT9045Gem::AddCEID()`：`CEID i → RPTID i`。
- `HT9045Gem::AddReprot()`（`uHGemHT9045.cpp:441`）：**是一個 no-op bug** ——
  `for(i=DoStart; i<TotalEvent; i++)` 迴圈跑 275 次，但迴圈內永遠寫死 `SetReportIDContent(1, …)`，
  迴圈變數 `i` 完全沒用到。所以 9045 只定義 **一張** 報表：`RPTID 1 = {1027 System Time}`。
- 韌體傾印互相印證：`EventReport_ReportID.def` 全檔只有 `1 → 1027` 一列；
  `EventReport_CEID.def` 的 ReportID 欄 = CEID 自己。
- ⇒ **9045 出廠狀態：除 CEID 1 之外，每一個事件都送空 `L,0`。**

### 2.3 關鍵發現：AMR 資料是 host 自己 provision 的

先前的判斷是「照抄 L2 會撞爛 AMR 交握」。查京元現場 log 後，這個前提不成立：

| host 自己用 S2F33 定義的報表 | 內容 |
|---|---|
| RPTID 2000 | `{38202, 38205, 38206, 38207, 38219, 38220, 38221}` |
| RPTID 2001 | `{38222 … 38236}`（15 格） |
| RPTID 502 | `{3, 1006, 1007, 1011, 1501, 1513, 1517, 1518}` |

host 再用 S2F35 綁：`272→{502,2000,2001}`、`273→{502,2000}`、`274→{502,2000}`、`275→{502,2000}`。

**也就是說 9045 的 AMR 資料路徑 100% 由 host 提供，韌體一格都沒給。**

而 HT-160S 對這兩張 AMR 報表的 SVID 覆蓋率我逐格查過（`uAgvStation.cpp:104-112` 站台表）：

| host 報表 | HT-160S 覆蓋 |
|---|---|
| RPTID 2000（7 格） | ✅ **7/7 全有**（38202 Loader、38205/38206/38207 = AUTO1-3 carrier、38219/38220/38221 三張 bitmap） |
| RPTID 2001（15 格） | ✅ **15/15 全有**（38222-38227 tray、38228-38233 device、38234-38236 AUTO1-3 binset） |
| RPTID 502（8 格） | ❌ **1/8**（只有 1518）。缺 3、1006、1007、1011、1501、1513、1517 |

⇒ **L2 對齊對 AMR 是安全的**：只要 host 送同一份 S2F33/S2F35，兩張 AMR 報表在 HT-160S 會完整解出。
真正的缺口在 RPTID 502（機台上下文），那是 R5。

### 2.4 計畫

分三步，每步可獨立編譯與驗證。

**S1 — 把自建報表搬離低位號**
自建 report 1–7 目前佔用 RPTID 1–7，正是 `CEID i → RPTID i` 會踩到的號。
比照 SVID 66000 高位段的既有手法，把自建報表搬到 **RPTID 66001–66007**（僅改號，SVID 內容不動）。
同時把 `RPTID 1` 改成 9045 的定義 `{1027}`，讓 CEID 1 的行為與 9045 一致。

- 改點：`AddReprot()` 的 RPTID 號、`AddCEID()` 的連結號。
- 風險：低。報表號只在韌體內部與 host provision 時有意義，SVID 內容不變。
- 驗證：`S6F19 RPTID=66001` 應回原本 13 格；`S6F19 RPTID=1` 應回單格 1027。

**S2 — `AddCEID` 改為 `CEID i → RPTID i`**
這是決策點，兩個選項：

| 選項 | 行為 | 代價 |
|---|---|---|
| **A（完全對齊，建議）** | `CEID i → RPTID i`，未 provision 前除 CEID 1 全送空 `L,0` | host 沒 provision 就拿不到資料——但京元 host 確定會 provision（當天 37 張報表 / 34 條連結） |
| B（對齊＋加值） | `CEID i → {RPTID i, 66001}` | 未 provision 也有資料，但本體是 `L,2` 而 9045 是 `L,1`，**不是 byte 級對齊** |

建議 **A**。理由：客戶要求的是對齊；空 `L,0` 是 host 已經在處理的 9045 既有行為，
選 B 等於「主動多送 host 沒訂的東西」，反而製造新的不一致。

**S3 — AMR 交握改為依賴 host provision**
272–275 不再由韌體硬掛 report 2/3/4/6/7，與 9045 一致。
⚠️ **這一步在京元確認「host 會對 HT-160S 送同一份 S2F33/S2F35」之前不可上機。**
若 host 不送，AMR 事件會變成空本體，無人線資料斷掉。

- 緩解：S3 加一個 `General.ini` 開關（預設 = 舊行為），確認後再切。
  這樣 S1/S2 可以先進，S3 的切換不需要再改碼。

**驗證方式（三步共用）**：用 `D:\AI_Area\Tool\HT160S_SECS_Simulator` 重放京元 2026-06-08 的
S2F33/S2F35 序列，再逐 byte 比對 S6F11 本體與 9045 log。

---

## 3. R2 — `SET_LOT_INFO`：同名，但連 lot 模型都不同

這是我這輪新查到的，先前規格書把它誤列為「9045 無對應」。

### 3.1 兩邊實際形狀

| | HT-160S | HT9045 |
|---|---|---|
| 位置 | `uHGemHT160.cpp:687-729` | `uHGemHT9045.cpp:2183` |
| 本體 | `L,2{ A "SET_LOT_INFO", L,n{ A lotID … } }` | `L,2{ L,2{A "LOT_INFO", A <XML>}, L,2{A "DISPLAY", A <text>} }` |
| 模型 | **多 Lot 清單**，覆蓋式重登 | **單一 Lot ＋ 15 欄 metadata**，XML |
| 前置 gate | work-order 備份成功 | `HasICUnderMachine() \|\| SystemStart` → HCACK=4 |

9045 的 XML 欄位（`BarcodeXML.cpp:416` `ProcessLotInfo()`）：

```
CUSTOMER  INNER_LOT_ID  CUST_LOT_ID  CUSTOMER_DEVICE_GROUP  DEVICE_NAME
STAGE  STEP  REPORTCOUNT  TEMPERATURE  TESTER_ID  PROGRAM_NAME
TEST_BIN_NO  HANDLER_ID  CURR_QTY  OPERATOR_ID
```

### 3.2 為什麼這是高風險

host 若沿用 9045 的 body，HT-160S 讀第一個 item 時期望 ASCII `"SET_LOT_INFO"`，
實際拿到的是 LIST → 解析失敗。**同名不同體是最難查的一類**，因為 host 端看起來「命令有被接受」。

### 3.3 但這裡有一個重要機會

`INNER_LOT_ID` = 京元批（OSAT lot）、`CUST_LOT_ID` = 客戶批 —— 這正好是 HT-160S 的雙批號模型。
HT-160S 目前的客戶批是走 **WebAPI HTTP GET** 拉的。若實作 9045 的 XML 形狀，
**客戶批可以直接從 SECS 拿到**，WebAPI 從「唯一來源」降為「備援」。
`TEST_BIN_NO` 可能對映 Bin 綁定、`CURR_QTY` 對映批量，值得一併評估。

### 3.4 計畫（建議 C＋A 併行）

| 選項 | 說明 | 評估 |
|---|---|---|
| A | HT-160S 支援雙形狀（看第一個 item 是 ASCII 還是 LIST 分流） | 相容性最好，成本中 |
| B | 直接改成 9045 形狀，要求京元改 host | 最對齊，但破壞既有多-Lot 功能 |
| **C（建議）** | `SET_LOT_INFO` 讓給 9045 的 XML 形狀；HT-160S 原本的多-Lot 清單**改名** `SET_LOT_LIST`，在規格書標為 HT-160S 專屬擴充 | 最乾淨，名字不再撞 |

C 需要決定的子項：15 個 XML 欄位裡，HT-160S 要真正消費哪幾個？
最小可行 = `INNER_LOT_ID`（→ SECS lot）＋ `CUST_LOT_ID`（→ 客戶批）＋ `CURR_QTY`（→ 批量），
其餘先解析並記 log 但不消費（與 9045 對很多欄位的處理相同）。

⚠️ **需要向京元索取一份真實的 `SET_LOT_INFO` XML 樣本**才能寫測試 fixture。

---

## 4. R3 — 規格書 §3.3（必做，且是對客戶的 breaking change）

`docs/SECS/HT160S_SECS_Interface_Spec_20260727.md` §3.3 仍列舊的 1–31 語意
（`1 Handler change status`、`27 One Cycle Finish` …），與 `ab1b99e` 之後的韌體**完全不符**。
這是客戶已經拒絕的那份規格的核心段落。

計畫：
1. §3.3 整段換成 9045 字典（可從 `EventReport_CEID.def` 機械產生，避免手打）。
2. 每個 CEID 標註三態：**有發射點** / **註冊但惰性**（HT-160S 無該機構） / **9045 未具名**（142/143/144）。
3. 移除 §4.x「同一件事、不同號碼」對照表——對齊後已不存在。
4. §7.1 / §7.2 重寫（見 R4、R5）。
5. 正式發文通知京元：CEID 1–31 語意全部改變，host 端設定需同步。

順帶要修的既有錯誤（本輪查到，尚未修）：
- `SET_LOT_INFO` 被列「9045 無對應」→ 實際是同名不同體（R2）。
- `HOME` 被列在 §7.1「已對齊 HT9045」→ 9045 整棵 SECSGEM 樹 0 命中（R4）。
- §7.2「SVID 38237–38245 = Auto4/5/6 的 carrier/tray/device/bin-setting」→ **描述錯**。
  AUTO4-6 的 carrier 是 **38208/38209/38210**，不在 38237–38245 內。
  正確的專屬段是 **38208–38210 ＋ 38237–38245**（共 12 個號）。

---

## 5. R4 — HT-160S 專屬 RCMD 處置

| RCMD | 9045 狀態（已查證） | 建議 | 理由 |
|---|---|---|---|
| `ONLINE`（裸） | 只有 `ONLINE_REMOTE` / `ONLINE_LOCAL` | **保留** | 純別名，9045 host 不會送這個名字，零風險 |
| `HOME` | 整棵樹 **0 命中** | **保留＋改列 §7.2 專屬** | 9045 最近似的是 `RESET`（`fMain->Reset()`），語意不同。刪掉 HOME 會少一個有用的遠端功能而換不到對齊 |
| `CLEARCOUNT` | 無此 RCMD；只有 CEID 5 `ClearCount Pressed` 事件 | **保留**，並**另外補** `CLEAN_AUTO_SORT_COUNT` | 京元當天送過 `CLEAN_AUTO_SORT_COUNT` 2 次，那個才是 host 真的會用的。兩者語意不同，不可互相改名 |

---

## 6. R5 — SVID 對齊

| 項目 | 現況 | 計畫 |
|---|---|---|
| 66000–66039 段 | HT-160S sorter 專屬 | **維持**。已驗證 9045 最高 SVID = **65095**，66000 以上一個都沒有 ⇒ 這是「安全的不對齊」，硬塞 9045 號碼等於發明假 SVID |
| 38208–38210 / 38237–38245 | AUTO4-6 延伸 | **待 9045 SVID 目錄**。手上 dump 只有 CEID 與 ReportID 目錄，`EventReport_ReportID.def` 257 列完全不含 382xx ⇒ 無法證實或否證 9045 是否已定義這 12 個號。**需向京元索取 9045 的 SVID 目錄傾印** |
| **RPTID 502 缺 7/8 格** | 只有 1518 | 🔴 **這是本項最高價值的缺口**。502 被 host 綁在 **272/273/274/275 全部四個 AMR 事件**上，等於每一次 AMR 交握都有 7 格空值。需查 9045 的 3 / 1006 / 1007 / 1011 / 1501 / 1513 / 1517 各是什麼，逐格評估 HT-160S 有無等價資料 |

---

## 7. R6 — 9045 有而 HT-160S 沒有的 35 個 RCMD

已用 `uHGemHT9045.cpp` 逐一確認存在。分三級：

### 7.1 優先補（sorter 有對應機構，9045 實作極簡）

| RCMD | 9045 實作 | HT-160S 對應 |
|---|---|---|
| `CLEAN_OUT` | `BtnCleanOutClick()`；HCACK=0 | 有 Clean Out 功能，缺遠端命令 |
| `TRAY_FEED` | has-IC gate → `BtnTrayEndClick()` + SoftStart | 有 Tray Feed，但 `CheckAllTrayFeedFinish()` 是 stub 回 false（見 R7） |
| `RESET` | `fMain->Reset()`；HCACK=0 | 有等價復位 |
| `HALT` | `SoftStart=false`，gate 於 `bRCMDStart && bPhysicalStart` | 有等價停止 |
| `CLEAN_AUTO_SORT_COUNT` | — | 京元當天送 **2 次**，host 真的在用 |

> 這五個是最划算的一批：9045 的實作都只是呼叫既有按鈕 handler，HT-160S 的對應功能全部已存在。

### 7.2 需評估（有商務或流程意涵）

`START_LOT`、`STOP_LOT`、`CLEAR_LOT_INFO`、`LOTORDER`、`CLOSE_ONECYCLE`、`TRAY_MAP`、
`PP_PASSWORD`、`AUTHORITY_CHECK`、`PP_SELECT`、`INITIAL_START`、`REMOTE_START`、
`REMOTE_SAVE`、`REMOTE_UPDATE_PROGRAM`、`DOWNLOAD_RECIPE_BY_FTP`

> `STOP_LOT` 是這批裡唯一「無現場證據但有明顯營運不對稱」的——AMR 無人線的 Lot 收尾責任歸屬需與客戶確認。

### 7.3 判定 N/A（測試機專屬，sorter 無此機構）

`AUTO_RETEST`、`CONTINUE_RETEST_ART`、`CONTINUE_START_ART`、`CONTINUE_START_MRT`、
`INITIAL_START_ART`、`INITIAL_START_MRT`、`RETEST_MRT`、`SWITCH_TO_FT`、`SWITCH_TO_RT`、
`START_AQL`、`DEVTEMPOFFSETADJUST`、`TESTTEMPSETTING`、`EESUG_OFFSET`、`AUTOSITEMAP`、
`AUTO_CLEAN`、`YIELD_FAIL`

> 建議一律回 **HCACK=2**（認得命令、本機不執行），與 `ENERGY_SAVING` 的既有處理一致，
> 不可回 4（HCACK=4 是肯定回覆，會讓 host 無限等待完成事件）。

---

## 8. R7 — CEID 已註冊但無發射點

`ab1b99e` 已補 14 個發射點。剩餘：

| 項目 | 狀態 |
|---|---|
| CEID 49 `Tray Feed Finish` | `CheckAllTrayFeedFinish()` 是 stub 回 false ⇒ 永不發射。與 R6 的 `TRAY_FEED` 綁在一起做 |
| 9045 CEID 154–211 站別細粒度（58 個） | HT-160S 無此粒度偵測，維持惰性註冊 |
| CEID 156/160/164（剩一盤偵測） | HT-160S 無此偵測 |
| CEID 141/91/92/93 | 9045 該版本自己也沒有發射點可參照 |

---

## 9. 建議執行順序

| 階段 | 內容 | 阻塞條件 |
|---|---|---|
| **P0** | R3 規格書 §3.3 重寫 ＋ 四處既有錯誤修正 ＋ 發文通知京元 | 無。**應立刻做**——目前交付給客戶的文件與韌體不符 |
| **P1** | R6.1 五個 RCMD 補實作（`CLEAN_OUT`/`RESET`/`HALT`/`CLEAN_AUTO_SORT_COUNT`/`TRAY_FEED`＋CEID 49） | 無 |
| **P2** | R1 的 S1＋S2（報表搬高位段、`CEID i → RPTID i`） | 無（S3 另計） |
| **P3** | R2 `SET_LOT_INFO` 分流／改名 | 需京元提供 XML 樣本 |
| **P4** | R1 的 S3（AMR 改依賴 host provision） | 需京元確認 host 會送同一份 S2F33/S2F35 |
| **P5** | R5 RPTID 502 缺格補實作 | 需 9045 SVID 目錄傾印 |
| **P6** | R6.2 需評估的 14 個 RCMD | 需商務確認 |

## 10. 需要對外索取／確認（阻塞項集中列表）

1. **9045 的 SVID 目錄傾印** — 解 R5 的 38208–38210 / 38237–38245 歸屬，以及 RPTID 502 的 7 格語意。
2. **一份真實的 `SET_LOT_INFO` XML 樣本** — 解 R2，並作為測試 fixture。
3. **京元 host 是否會對 HT-160S 送與 9045 相同的 S2F33/S2F35** — 解 R1-S3。若不送，AMR 資料會斷。
4. **AMR 無人線的 Lot 收尾責任歸屬** — 解 `STOP_LOT`。
5. **CEID 1–31 語意變更的正式通知** — R3 的對外動作，host 端需同步改設定。

---

## 附註：本文未主張的事

- 不主張 renumber 66000 段 SVID（那會變成發明假資料）。
- 不主張刪除 `HOME` / `CLEARCOUNT` / `ONLINE`（換不到對齊，只少功能）。
- 不主張照抄 9045 的 `AddReprot()`（那是個 no-op bug，照抄 bug 不叫對齊）。
- R1-S3 之前的任何 AMR 行為變更都不建議上機。
