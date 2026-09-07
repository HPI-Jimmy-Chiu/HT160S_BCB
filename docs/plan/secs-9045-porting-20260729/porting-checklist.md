# HT9045 -> HT160S SECS 移植清單 (2026-07-29)

## 出處與方法 / Provenance

- **建立日期**：2026-07-29
- **HT9045 原始碼（唯讀）**：`D:\HT9045\HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP\SECSGEM\`
  - `uHGemEquipment.cpp:8977-9102` — 訊息 dispatch（`Remote.MessageID_S/_F` if-else 鏈）
  - `uHGemHT9045.cpp:1046-3137` — `HT9045Gem::S2F42_Host_Command_Acknowledge()`（RCMD 鏈）
  - `uHGemClass.cpp` — 共用基底 handler（S100F4 / S101F2/F4/F6/F8 / S103F12 / S125F2 …）
  - `uHGemHT9045.h:304-327` — HT9045 實際 override 的 handler 清單（用來區分「有分支」與「空 base stub」）
- **HT9045 CEID 目錄**：`D:\backup_version\HT9046\KYEC\20260626\EventReport_CEID.def`
- **京元現場 log**：`D:\backup_version\HT9046\KYEC\20260626\2026_06_08\SECSGEM_TextLog_00..19.txt`（20 檔，2026-06-08 全日，雙向完整 SML body dump）
- **HT160S 實作（本專案，branch `feat/iosetview-172-refactor`，含未 commit 之工作樹變更）**：
  - `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uHGemClass.cpp` — `HTGem::Dispatch`
  - `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uHGemHT160.cpp` — SV/EC/CEID/Report/RCMD/handler override
  - `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uHGemHT160.h` — `SECS_EVENT` 事件枚舉
  - `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uHGemEquipment.cpp` — `EmitEventReportBody` / `EmitIndividualReport`
- **客戶規格書（對客戶為權威）**：`D:\HT160S_BCB\docs\SECS\HT160S_SECS_Interface_Spec_20260727.md`
- **前置文件**：`D:\HT160S_BCB\docs\plan\secs-9045kyec-vs-160-cmd-diff-20260728.md`、`D:\HT160S_BCB\docs\plan\secs-pathA-3way-crossverify-20260727.md`、`D:\HT160S_BCB\docs\plan\secs-9045kyec-diff-20260728\`（僅作提示，本文所依賴之數字全部重新推導）

> **方法說明（務必先讀）**
> 「9045 有 / HT160S 沒有」一律**比原始碼**（兩邊 dispatch 表與 RCMD 鏈逐條對照）；
> 「京元當天送出幾次」一律**比 log**（本次以自寫的遞迴 SML parser 重新計數，未沿用先前 JSON）。
> 兩者**交集**才是真正會在現場踩到的缺口；只在原始碼有差、host 從未使用者，屬於能力差異而非現場風險。
> 本次重新計數之總量（可稽核）：log 中 `[SxFy]` 記錄共 **2394** 筆；host 主動送出的 primary 共 **15 種**；
> 機台送出 `S6F11` **645** 筆（另有 **10987** 筆因 CEID 被 host disable 而中止，其中 CEID 80 佔 10962 筆）；
> `S2F41` 共 **82** 筆、**10** 種 RCMD；CEID 目錄 **292** 個編號、其中 **286** 個具名（未具名者為 74/75/79/142/143/144）。

---

## 0. 一句話結論

20260728 那一批工作把「**現場 log 已證實 host 會送、而 HT160S 不回或誤動作**」的項目**全部清空**了。
本清單剩下的都是「9045 有能力、京元 host 當天沒用到」的項目——它們是**能力差異**，不是現場故障。
唯一還帶現場證據的殘留有兩處：
1. **CEID 層 6 個可行動缺口** —— host 以 `S2F35` 明確訂閱的 34 個 9045 CEID 中，有 **2 個**（48 Change EC、54 UPH Record End）HT160S 完全沒有等價事件；另有 **4 個**當天實際上線 5–9 次、HT160S 同樣無事件可發（123 Safe Door On Off、124 Save Recipe、66 Load Tray Finish、53 UPH Record Start）。
2. **RCMD `CLEAN_AUTO_SORT_COUNT`** —— 當天送 2 次，HT160S 現在回 HCACK=1;已有 `CLEARCOUNT` 可作別名。

> **一個必須先知道的行為前提**：HT160S 的 `HTGem::S9F3_Unrecognized_Stream_Function_Type()`（`uHGemClass.cpp:265-268`）**只寫 log、不上線**。
> 因此在 HT160S 上，「**沒有 dispatch case**」與「**有 case 但 handler 是空殼**」對 host 而言結果**完全一樣：零 byte、T3 逾時**。
> （9045 也只在 `CUSTOMER_CODE==CC_ASE_KaohSiung_K3` 時才真送 S9F3，對京元同樣是靜默，兩邊在這點上一致。）
> 這使得下方 A-1「有 case 的空殼」與 A-2「完全沒 case」在**現場風險上等價**，只在維護語意上不同。

---

## A. Message 層（SxFy）

### A-0. 本次已完成（20260728，工作樹已 build clean，尚未 commit）

| 訊息 | 9045 用途 | 京元當天送出次數 | HT160S 現況 | 判定 |
|---|---|---|---|---|
| `S6F15` -> `S6F16` | Event Report Request：host 指定 CEID 主動拉取事件報表 | **3** | 已實作。`uHGemClass.cpp:114` dispatch -> `HT160Gem::S6F16_EventReportData()`（`uHGemHT160.cpp:1739`）-> `THGem::EmitEventReportBody()`（`uHGemEquipment.cpp`）。未定義 CEID 回 `L,3{DATAID,CEID,L,0}` | 已實作(20260728) |
| `S6F19` -> `S6F20` | Individual Report Request：host 指定 RPTID 主動拉取報表值 | **8** | 已實作。`uHGemClass.cpp:115` -> `HT160Gem::S6F20_IndividualReportData()`（`uHGemHT160.cpp:1790`）-> `THGem::EmitIndividualReport()`。長度由**即時**登錄表取（現場實證 RPTID 506 曾由 5 變 6） | 已實作(20260728) |
| `S10F3` -> `S10F4` | Terminal Display, Single：host 推單段文字到機台 | **1** | 已實作。`HT160Gem::S10F4_...`（`uHGemHT160.cpp:1877`）回 `B ACKC10=0`。**刻意不彈窗**（handler 跑在 HSMS 接收執行緒；本機訊息視窗皆 ShowModal 且會停所有馬達） | 已實作(20260728) |
| `S10F5` -> `S10F6` | Terminal Display, Multi-block | **7** | 已實作。`uHGemHT160.cpp:1924`。同上不彈窗 | 已實作(20260728) |
| `S125F1` -> `S125F2` | Enable/Disable EC Data Send（京元私有 stream）：host 指定要回報變更的 ECID 集合 | **6** | 已實作。`uHGemClass.cpp:149` -> `HT160Gem::S125F2_...`（`uHGemHT160.cpp:1972`）。**每個 request 只回一次**；9045 是每個 ECID 各回一次（log 中 6 個 request 對 **138** 個 secondary，且因 `InitLocalHead` 沿用 request SystemByte，那些回覆的 transaction id 全部相同 = 協定缺陷，刻意不沿用） | 已實作(20260728) |

> 這 5 組是 2026-06-08 現場 log 唯一證實「host 主動送、HT160S 先前完全不回」的訊息。全部補完後，**message 層已無帶現場證據的缺口**。

### A-1. HT160S 有 dispatch case、但 handler 是空殼（不上線，等同不回覆）

| 訊息 | 9045 用途 | 京元當天送出次數 | HT160S 現況 | 判定 | 理由 / 成本 |
|---|---|---|---|---|---|
| `S7F1` | Process Program Load Inquire -> `S7F2` PPGNT（配方上傳前的容量詢問） | **0** | dispatch 存在（`uHGemClass.cpp:121`）-> `HT160Gem::S7F2_ProcessProgramLoadGrant()`（`uHGemHT160.cpp:2031`）只 `StringOut`，回傳 1，**不上線** | 暫不處理 | 配方走本地/FTP，非 SECS（規格書 §5 已宣告）。但**有 case 卻不回**比沒 case 更容易誤判為「已支援」——若不做，建議把 case 移掉或改回最小 PPGNT=1（拒絕授權）。成本：小 |
| `S7F3` | Process Program Send -> `S7F4` ACKC7 | **0** | 同上，`uHGemHT160.cpp:2038` | 暫不處理 | 同上；此路徑是 host 把配方寫進機台，不做才是安全的預設 |
| `S7F5` | Process Program Request -> `S7F6` PP body | **0** | 同上，`uHGemHT160.cpp:2044` | 暫不處理 | 同上 |
| `S7F17` | Delete Process Program -> `S7F18` ACKC7 | **0** | dispatch 存在（`uHGemClass.cpp:124`）-> **基底** `HTGem::S7F18_...`（`uHGemClass.cpp:226`）`SendUnsupported`，HT160Gem 無 override | 暫不處理 | 遠端刪配方。即使客戶要求，也應先有權限層設計（見 §E） |
| `S7F19` | Current EPPID Request -> `S7F20` 現行配方 ID 清單 | **0** | dispatch 存在（`uHGemClass.cpp:125`）-> 基底 `uHGemClass.cpp:227` `SendUnsupported` | 待客戶確認 | Path-A 定稿即標「確認是否 intentional」。若 host 真會輪詢，回一個空 `L,0` 就能止住 T3，成本：小。當天 0 次，無現場證據 |
| `S14F1` | （HT160S 自有；9045 走 `S14F3`） | **0** | dispatch 存在（`uHGemClass.cpp:140`）-> `HT160Gem::ProcessS14F1_GetAttrRequest()`（`uHGemHT160.cpp:2064`）只 `StringOut` | 暫不處理 | 規格書 §5 已列「尚未支援」，但 dispatch 存在而不回覆；建議與 S7 群一併決定「移掉 case」或「回最小 ack」。成本：小 |

### A-2. 9045 有處理的 host primary、HT160S 完全沒有 dispatch case

| 訊息 | 9045 用途 | 京元當天送出次數 | HT160S 現況 | 判定 | 理由 / 成本 |
|---|---|---|---|---|---|
| `S1F23` | Collection Event Namelist Request -> `S1F24` 回 CEID/名稱/關聯 SVID 目錄（`uHGemEquipment.cpp:9014`） | **0** | S1 switch 只有 F=1/3/11/13/15/17（`uHGemClass.cpp:76-81`）-> 落到 log-only S9F3 | 待客戶確認 | GEM 自我描述訊息。HT160S 的 CEID 目錄已在規格書 §3.3 逐條明列，host 不必線上問。若客戶 EAP 要自動 discovery，可由 `AddCEID()` 的登錄表直接組回覆。成本：小 |
| `S2F23` | Trace Initialize Send -> `S2F24` TIAACK，開週期性 trace（`uHGemEquipment.cpp:9020`） | **0** | 無 case -> 靜默 | 暫不處理 | 京元 host 以事件報表取代 trace，證據明確：當天 `S2F33` 122 次、`S2F35` 121 次、`S2F37` 9 次，`S2F23` 0 次。Path-A 亦列低優先 |
| `S2F29` | Equipment Constant Namelist Request -> `S2F30`（ECID/名稱/MIN/MAX/DEF/UNITS） | **0** | 無 case -> 靜默 | 待客戶確認 | 是 `S2F15` 的標準搭配，但**現場證據反向**：host 當天寫了 24 次 `S2F15`（ECID 0/1/1006/1007/16000/16002/16023/16026/35011/37007）卻從未問 `S2F29`，顯示它用自己的字典表。HT160S 已有完整 EC 表（`HT160Gem::AddEC`），要做成本小 |
| `S2F43` | Reset Spooling Streams and Functions -> `S2F44`（`uHGemEquipment.cpp:9035`） | **0** | 無 case -> 靜默 | 暫不處理 | HT160S **未實作 spooling**，沒有 spool 佇列可 reset。做了也只能回一個空 ack |
| `S6F17` | Annotated Event Report Request -> `S6F18`（帶名稱欄位的 S6F15） | **0** | 無 case -> 靜默。`uHGemClass.cpp:107-110` 的註解已明確記載「刻意不處理」 | 待客戶確認 | 與已完成的 `S6F15/F16` 同族，只多帶名稱欄位，可沿用 `EmitEventReportBody()`。當天 host 用 F15 不用 F17，無現場證據。成本：小 |
| `S6F23` | Request Spooled Data -> `S6F24`（`uHGemEquipment.cpp:9048`） | **0** | 無 case -> 靜默 | 暫不處理 | 同 `S2F43`：無 spooling |
| `S14F3` | Device ID Bin Map Send：host 直接把「2DID -> Bin code」對照表推給機台（`uHGemEquipment.cpp:9068` -> `HT9045Gem::S14F4_Get2DID_BinCode()` @ `uHGemHT9045.cpp:4811`，含 XML 解析，且 `HasICUnderMachine()`/`SystemStart` 時回 HCACK=4） | **0** | 無 case -> 靜默（HT160S 走的是自有的 `S14F1`，兩邊 S14 不相容） | 待客戶確認 | **不是能力缺口**：HT160S 已有兩條 2D->Bin 資料路徑——Lot WebAPI（`LotWebApiClient.cpp`，per-Lot HTTP GET）與檔案匯入（`main.cpp` `btn2DImport`，JSON/CSV hybrid）。是否再開一條 SECS 路徑是客戶/介面決策。成本：大（需 XML/清單模型 + 與現有 registry 的優先權規則） |
| `S100F3` | Report All Alarm Request -> `S100F4`：傾印整份警報目錄（ALID / ALTX / enable flag，`uHGemClass.cpp:2418`）。**9045 私有 stream** | **0** | 無 case -> 靜默 | 暫不處理 | 功能與已實作的標準 `S5F5/S5F6` **完全重疊**，且 HT160S 的 S5F6 是由 `mapAlarmCodeList` / `AlarmList.csv` SSOT **即時**產生（約 480+ 碼）。做 S100F3 等於做第二套同樣的東西 |
| `S101F1` / `S101F3` | 上傳機台**現行配方檔內容**（逐行 ASCII；F1 對應 `SV_70_ReceipeStruct==1`，F3 對應 `==2`。`uHGemClass.cpp:2438` / `:2466`）。**9045 私有 stream** | **0** / **0** | 無 case -> 靜默 | 暫不處理 | Sorter 的「配方」是 tray 幾何 + 分選綁定（`.tech` / `.ofs` / `LotBinBinding.ini`），不是 tester 的 PP 檔；沒有對應的「逐行 ASCII 配方」可放。規格書 §5 已宣告 S7 系列不支援，此私有 stream 同理 |
| `S101F5` / `S101F7` | host **下載檔案寫入機台磁碟**。F7 帶 `PATH` + `FILE` + chunk 序號/總數，寫到 `CurrentDirectory\PATH\FILE`（`uHGemClass.cpp:2580` `S101F8_StoreHostUploadFile`，含 `mkdir` + `fopen("wb"/"ab+")`）。**9045 私有 stream** | **0** / **0** | 無 case -> 靜默 | **不建議**（見 §E） | 路徑與檔名完全由 host 指定，等於開放遠端寫檔 |
| `S103F11` | Status Variable Namelist Request **with Value** -> `S103F12`（`uHGemClass.cpp:3536`）。**9045 私有 stream** | **0** | 無 case -> 靜默 | 待客戶確認 | HT160S 已有 `S1F11/F12`（namelist）與 `S1F3/F4`（值），合併版屬便利性。成本：小（可重用 `DataItemOutSVValue`） |
| `S110F2` / `S110F6` / `S110F8` / `S120F2` | **皆為 even function**：9045 主動送 `S110F1/F5/F7`、`S120F1` 問 host（客戶名單 / 配方資訊 / 配方設定檔），這幾條是**回覆半**。ASEM 專屬 | **0** | HT160S 的 `Dispatch` 依設計丟棄所有 even F（`uHGemClass.cpp:61-70`，只 log「reply ignored」） | **N/A（非 host primary）** | 前一版報告把這幾條列進「只有 9045」是**統計假象**——它們不是 host 主動送的訊息。而且 9045 自己的 `S110F6_ListCustomerName` / `S110F8_ListReceipeInformation` / `S120F2_ListReceipeSetupFile` 在 `uHGemHT9045.h` **未被 override**，仍是空的 base `{}`，即 9045 收到也不做事 |
| `S125F3` | Level Setting Change Request -> `S125F4`：host 依 `LSID` 改**權限層級**（`HT9045Gem::S125F4_...` @ `uHGemHT9045.cpp:5133`，`SetECValue(LSID,&LEVEL)` + `fSecurity->SetLevelSet()`）。**9045 私有 stream** | **0** | 無 case -> 靜默 | **不建議**（見 §E） | 遠端改機台權限層級 |

### A-3. 私有 stream 總結（一句話）

| Stream | 9045 上承載什麼 | Sorter 有沒有東西可放 |
|---|---|---|
| `S100` | 全警報目錄傾印 | **有，但已被標準 S5F5/F6 覆蓋** —— 不必再做一套 |
| `S101` | 配方檔內容雙向搬運（F1/F3 上傳、F5/F7 下載寫檔） | **沒有可對應的「逐行 ASCII 配方檔」**；下載方向另有安全問題 |
| `S103` | SV namelist 加值版 | 有（SV 表已存在），純便利性 |
| `S110` / `S120` | 機台向 host 問客戶名單 / 配方資訊（ASEM 客戶專屬，9045 自身 handler 為空） | **沒有** —— 且 9045 本身也沒在用 |
| `S125` | F1/F2 = EC 變更回報訂閱（**已實作**）；F3/F4 = 遠端改權限層級 | F1/F2 有；F3/F4 **不建議** |

---

## B. RCMD 層（S2F41）

### B-0. 本次已完成（20260728）

| RCMD | 9045 行為（引原始碼） | 京元當天次數 | HT160S 現況 | 判定 |
|---|---|---|---|---|
| `ONE_CYCLE` | `uHGemHT9045.cpp:1101`：`bSECSOneCycleComm=true` + `fMain->BtnOneCycleClick()`，**無條件 HCACK=0** | **11** | `uHGemHT160.cpp:1233`：走 `TfMain::OneCycleCore()`（去掉兩個 `ShowMyMessage` modal，避免卡住 HSMS 接收執行緒）。**據實回 HCACK**（0 已啟動 / 4 機台未運轉或已在 OneCycle / 2 模式或 Lot 不符）。刻意偏離 9045 | 已實作(20260728) |
| `ENERGY_SAVING` | `uHGemHT9045.cpp:2857`：加熱器 / ATC 省電切換；京元機台當天 **23/23 全部回 HCACK=2**（走「Function Disable」分支） | **23** | `uHGemHT160.cpp:1285`：**刻意的 HCACK=2 拒絕**。HT160S 無加熱器 / 無 ATC / 無馬達電流待機切斷，沒有省電狀態可進出；驗證 `STATE` 語法後拒絕，不謊報成功。與京元自家 9045 現行回覆逐 byte 相同 | 已實作(20260728) |
| `PP_SIGNALTOWER` | `uHGemHT9045.cpp:1837`：CP = `RED`/`GREEN`/`YELLOW`，值 0/1/2 | **15** | `uHGemHT160.cpp:1344`：**閂鎖式 host 覆寫**。空 list = 解除；未知 CP 或值超出 0..2 整包 HCACK=2；警報 Note 顯示中或機台自身安全異常（`RunState=LED_ErrJam`）時暫停覆寫，避免遮蔽機台紅燈；值 2（閃）依本機慣例呈現為恆亮 | 已實作(20260728) |
| `PP_MUSIC` | `uHGemHT9045.cpp:1808`：單一 pair，CP 名稱為空字串 `A[0]` | **15** | `uHGemHT160.cpp:1417`：同為閂鎖式覆寫，值 1..4，超出回 HCACK=2。CP 名稱讀取後丟棄（京元送空字串） | 已實作(20260728) |

> 覆寫的解除路徑（規格書 §3.4 已對客戶宣告）：面板 ALARM RESET、警報/訊息視窗的 OFF BUZZER、維修畫面 SECS/GEM 分頁的 Release Host Override、`S1F15` 轉 OFF-LINE、以及 **HSMS 斷線**（新增 `HTGem::OnCommunicationLost()` 虛擬函式，`uHGemClass.h:43-47` / `uHGemHT160.cpp:1589`），避免 host 消失後閂鎖無人可解。

### RCMD 三集合現況（重新推導）

| 分類 | 數量 | 內容 |
|---|---|---|
| 兩邊都有 | **12** | `ENERGY_SAVING` `LOTSTART` `ONE_CYCLE` `ONLINE_LOCAL` `ONLINE_REMOTE` `PAUSE` `PP_MUSIC` `PP_SIGNALTOWER` `SET_LOT_INFO` `START` `START_AGV` `STOP` |
| 只有 HT160S | **3** | `CLEARCOUNT` `HOME` `ONLINE`（`ONLINE_REMOTE` 的裸別名） |
| 只有 9045 | **35** | 見 B1~B4（原始碼共擷到 37 個 token，其中 `EESUG_Offest`/`EESUG_OFFSET` 與 `PP_SELECT`/`PP-SELECT` 各為同一命令的兩種拼法，故不同命令為 35 個。20260728 前為 39，減去上表 4 個） |

京元 host 當天實際送出 **10** 種 RCMD（共 82 筆）：
`ENERGY_SAVING`(23) `PP_SIGNALTOWER`(15) `PP_MUSIC`(15) `ONE_CYCLE`(11) `START`(7) `START_AGV`(4) `LOTSTART`(3) `CLEAN_AUTO_SORT_COUNT`(2) `INITIAL_START_ART`(1) `SWITCH_TO_FT`(1)

`START_AGV` 三缺陷已於 `f7f3939` 修正（現場 payload 見 `SECSGEM_TextLog_15.txt:504`：`Loader=Action`，其餘 8 站 `=NA`，另帶 `LoaderTrayCount=1` `LoaderICCount=3`）：
① 接受 `LoaderICCount`（原本落入「未知 CP」-> 整包 HCACK=2）；
② 只在 `值=="Action"` 才 `BeginPrep()`（對齊 `uHGemHT9045.cpp:1676-1709`；原本不看值，會把 8 站全部鎖住）；
③ `bUseAMR==false` 時明確 HCACK=2 拒絕（`uHGemHT160.cpp:1102` 起的迴圈）。

---

### B1. 京元當天**真的送過**、HT160S 仍無分支（3 項）

| RCMD | 9045 行為（引原始碼） | 京元當天次數 | HT160S 對應功能 | 判定 | 說明 |
|---|---|---|---|---|---|
| `CLEAN_AUTO_SORT_COUNT` | `uHGemHT9045.cpp:1145`：機內有 IC 時 HCACK=2；否則寫一筆 `RecordProcess("Clear AUTO Sorting Count")` 稽核行（含各 Bin 計數、Jam 數、UPH、MTBF/MUBF）後清 Auto 分選計數 | **2** | `TfMain::btnClearCountClick`（`main.cpp:879`），已由 HT160S 自有 RCMD `CLEARCOUNT` 驅動（`uHGemHT160.cpp:887`） | **建議實作** | **唯一還帶現場證據的 RCMD 缺口**。host 送了 2 次，HT160S 現在會回 HCACK=1（命令不認得）。最小成本作法：在既有 `CLEARCOUNT` 分支加上 `CLEAN_AUTO_SORT_COUNT` 別名 + 機內有 IC 時回 HCACK=4。成本：**小** |
| `INITIAL_START_ART` | `uHGemHT9045.cpp:1383`：`HasICUnderMachine()==false` 才 `SetRunStartMode(rsmInitial_ART)`，並依 `TrayForm.bEnableAMR` 決定 `bSameSetupFileNoDownload` | **1** | 無 | **N/A(測試機專屬)** | ART = Auto Retest Tray，測試機的重測流程起始模式；Sorter 沒有 run-start-mode 概念，也沒有重測 |
| `SWITCH_TO_FT` | `uHGemHT9045.cpp:1465`：`iSecsGemSwitchFTRT=1` + `fMain->FTClick()`，HCACK 由 FTClick 回傳（2/3/4 各代表不同拒絕原因） | **1** | 無 | **N/A(測試機專屬)** | FT/RT = Final Test / Retest 測試程式切換；Sorter 不執行測試 |

### B2. Sorter **有對應功能**但未開 RCMD（14 項）

| RCMD | 9045 行為（引原始碼） | 京元當天次數 | HT160S 對應功能（file:line） | 判定 | 說明 |
|---|---|---|---|---|---|
| `CLEAN_OUT` | `uHGemHT9045.cpp:1460`：`fMain->BtnCleanOutClick()`，無條件 HCACK=0 | 0 | `TfMain::sbCleanOut1Click`（`main.cpp:1913`）；完成時已由 `EmitCleanOutOK()`（`main.h:496`）發 CEID 28 | 待客戶確認 | 存在一個**不對稱**：HT160S 已經會發「Clean Out 完成」事件，卻沒有任何遠端「開始 Clean Out」的命令。但 HT160S 的主自動路徑是**進料源乾 sensor** 觸發（`csystem.cpp` CleanOut 流程），不靠 host；且 host 當天 0 次。成本：小 |
| `RESET` | `uHGemHT9045.cpp:1108`：`fMain->Reset()` + `RecordProcess`，HCACK=0 | 0 | 面板 ALARM RESET -> `SECS_EVENT.PressAlarmReset`（`main.cpp:2152`、`note.cpp:705`） | 待客戶確認 | 注意：9045 的 `RESET` **不會**解 AMR 鎖（該路徑零呼叫），所以移植它並不能解決任何握手死結。成本：小 |
| `HALT` | `uHGemHT9045.cpp:1794`：僅在 `bRCMDStart && bPhysicalStart` 時 `SoftStart=false` + HCACK=0，否則 HCACK=2 | 0 | `PAUSE`（`uHGemHT160.cpp:876`）、`STOP`（`:1204`）皆已實作 | 暫不處理 | 功能與已實作的 `PAUSE`/`STOP` **重疊**，且 9045 版本被 `bRCMDStart`+`bPhysicalStart` 兩個旗標鎖住，多數情況直接回 HCACK=2 |
| `CLOSE_ONECYCLE` | `uHGemHT9045.cpp:2762`：**只有**當 `fNote->fShow` 且錯誤碼為 `MES1640` 時才 `SoftStop=true; SoftStart=false; fNote->Close()`；否則 HCACK=1 | 0 | `fNote`（`note.cpp`）+ `ONE_CYCLE`（`uHGemHT160.cpp:1233`）已實作 | 暫不處理 | 這是「從 SECS 執行緒關掉一個 modal 視窗」。HT160S 的訊息/警報視窗一律 `ShowModal` 且會**停所有馬達**，這正是 `S10F4` 決定不彈窗的同一個執行緒風險（`uHGemHT160.h:128-129` 註解）。要做必須先設計跨執行緒的關窗請求機制。成本：中，風險：高 |
| `START_LOT` | `uHGemHT9045.cpp:1537`：keyed CP（`DEVICEID` / `OPERATORID` / `RUN_MODE`），Onsemi 專屬 | 0 | `LOTSTART`（`uHGemHT160.cpp:920`，累加式）與 `SET_LOT_INFO`（`:729`，覆蓋式）已實作 | 暫不處理 | 京元 host 用的是 `LOTSTART`（3 次）不是 `START_LOT`（0 次）；且 CP 集為 Onsemi 專屬。已被現有兩個命令覆蓋 |
| `STOP_LOT` | `uHGemHT9045.cpp:2516`：解析 keyed CP 後結束 Lot | 0 | `TfMain::btnLotEndClick`（`main.cpp:2679`）+ `DoLotEndProcess()`（`main.h:495`）；完成時發 CEID 12（`main.cpp:2737`） | 待客戶確認 | **明顯不對稱**：host 可以遠端「開 Lot」（`LOTSTART`）但**不能遠端「收 Lot」**。而 Soter CSV、per-Lot 統計、UPH 凍結都掛在 Lot End。AMR 無人線若無操作員在場，這條會變成營運問題。當天 0 次故無現場證據，但這是本節**最值得問客戶**的一條。成本：中（需與在途 WebAPI pull 取消邏輯一起處理，見 `fix/lot-end-inflight-webapi-cancel`） |
| `CLEAR_LOT_INFO` | `uHGemHT9045.cpp:2431`：`SystemStart` 時 HCACK=1、機內有 IC 時 HCACK=2；否則清 ART lot info + 清 2DID 清單 | 0 | `LotRegistry.Clear()` + `ArchiveDiscardedWorkOrder()`（`main.cpp:1399/2457/2758`；SECS 路徑見 `uHGemHT160.cpp:834`） | 待客戶確認 | **目前沒有任何遠端清單方式**：`SET_LOT_INFO` 收到空 list 是回 **HCACK=2**（`uHGemHT160.cpp:737`），不會清空。若客戶要遠端清工單，必須新開命令。成本：小（可重用既有 archive + Clear 路徑，並沿用 9045 的 idle/無 IC 前置條件） |
| `TRAY_FEED` | `uHGemHT9045.cpp:1367`：機內有 IC 時 HCACK=2；否則 `fMain->BtnTrayEndClick()` + `SoftStart=true` | 0 | `Run_TrayFeed` 模式骨架存在（`csystem.cpp:1720-1729`）、事件 `SECS_EVENT.PressTrayFeed`(10)/`TrayFeedOK`(29) 已宣告 | **暫不處理** | **具體阻擋原因**：`CheckAllTrayFeedFinish()` 目前是回 `false` 的 stub（`csystem.cpp:1812-1818`），且進入 `Run_TrayFeed` 的路徑被註解掉（`csystem.cpp:1673-1677`）。現在開這個 RCMD 會讓機台**永久卡在 `Run_TrayFeed`**。要做必須先補模組級 TrayFeed 完成旗標。成本：中 |
| `LOTORDER` | `uHGemHT9045.cpp:2455`：SPIL lot 優先順序調整 | 0 | Lot 清單有順序（`btnAddLot`/`btnEditLot`/`btnRemoveLot`，`main.cpp:3725` 附近） | 待客戶確認 | 需先定義「順序」在 HT160S 多-Lot 模型中的語意（現行為登記順序）。成本：中 |
| `TRAY_MAP` | `uHGemHT9045.cpp:2512`：**分支主體只有 `HCACK=0;`** —— AOSH 1600LT 用的空殼 | 0 | `btnTrayMap`（`main.h:218`） | 暫不處理 | 移植它等於**移植一個空殼**：9045 收到後什麼都沒做。除非客戶說明期望行為，否則不做 |
| `PP_SELECT` / `PP-SELECT` | `uHGemHT9045.cpp:1926`：依 `CUSTOMER_CODE` 分歧的配方選擇（ASE 高雄 / ONSEMI 各一套解析） | 0 | 配方切換已可經 **ECID 1501**（`S2F15` 寫入）達成，規格書 §3.2/§7.1 已宣告 | 暫不處理 | host 已有一條**已實作、已寫進規格書**的替代路徑。再開一條 RCMD 會出現兩個真源 |
| `REMOTE_START` | `uHGemHT9045.cpp:1725`：`SystemStart==false` 時 `fMain->Start()`（AMR 開啟時另要求 `RunInfo.bLotStart`），否則 HCACK=1 | 0 | `START`（`uHGemHT160.cpp:1081`）已實作，行為相同 | 暫不處理 | 與已實作的 `START` **功能重複**；9045 保留兩個是歷史包袱 |
| `START_AQL` | `uHGemHT9045.cpp:1601`：需 `HasICUnderMachine()==false` 且 `IniConfig.bI52_bAQLSortMode==true`，CP 帶 `BIN` | 0 | HT160S 有 3 種分選模式（By Lot+Bin / By Lot+PassFail / WhiteList overlay），**無 AQL 抽樣模式** | 待客戶確認 | AQL 抽樣在 Sorter 上概念可成立，但 HT160S 沒有這個模式；要做是新功能不是移植。成本：大 |
| `AUTHORITY_CHECK` | `uHGemHT9045.cpp:2088`：`IniConfig.bN07_EnableEmployeeIdCheak` 為真時，關掉 barcode/password/MyMessageBox 視窗，然後解析 `Action` / `Message` CP —— host 回覆「這個員工可不可以操作」 | 0 | `THT160UserRoleManager`（`UserRoleManager.cpp:93 Login` / `:194 AddOrUpdateUser` / `:234 CheckPassword`）有帳號/層級，但**無員工 ID barcode 閘門** | 待客戶確認 | 需先有「刷員工證才能操作」的機構/流程需求。且 9045 版本會從 SECS 執行緒關 modal（同 `CLOSE_ONECYCLE` 的風險）。成本：大 |

### B3. 測試機專屬 —— N/A（14 項）

| RCMD | 9045 行為（引原始碼） | 京元當天次數 | 判定 | 為什麼 Sorter 沒有對應 |
|---|---|---|---|---|
| `AUTOSITEMAP` | `uHGemHT9045.cpp:1333`：機內無 IC 才 `SetRunStartMode(rsmAutoSiteMap)` | 0 | N/A(測試機專屬) | Site map = 測試座位圖，Sorter 無測試站位 |
| `AUTO_RETEST` | `:1349`：`DoAutoRetest(true)` + `rsmAutoRetest` + `SoftStart=true` | 0 | N/A(測試機專屬) | 重測流程，Sorter 不測試 |
| `RETEST_MRT` | `:1270` | 0 | N/A(測試機專屬) | MRT = Multi Retest Tray，重測盤流程 |
| `INITIAL_START_MRT` | `:1285` | 0 | N/A(測試機專屬) | 同上 |
| `CONTINUE_START_MRT` | `:1306` | 0 | N/A(測試機專屬) | 同上 |
| `CONTINUE_START_ART` | `:1129`：多處 `HasIC()` 檢查後 `SetRunStartMode(rsmContinuStart_ART)` | 0 | N/A(測試機專屬) | ART 續跑 |
| `CONTINUE_RETEST_ART` | `:1114` | 0 | N/A(測試機專屬) | ART 續重測 |
| `INITIAL_START` | `:1400`：`SetRunStartMode(rsmInitialStart)` | 0 | N/A(測試機專屬) | 依附 tester 的 run-start-mode 狀態機，Sorter 無此模型 |
| `SWITCH_TO_RT` | `:1494`：`iSecsGemSwitchFTRT=2` + `fMain->RTClick()` | 0 | N/A(測試機專屬) | 測試程式切換 |
| `TESTTEMPSETTING` | `:2777`：需 `ATC_SYSTEM==eNewATCSystem`，`FormHS->TESTTEMPSETTING(iTemp)` 改 ATC 溫度 | 0 | N/A(測試機專屬) | HT160S 無 ATC、無加熱器（`DoTemptureControl()` 為空 stub） |
| `DEVTEMPOFFSETADJUST` | `:2289`：Qualcomm 溫度 offset（需 3 個 CP） | 0 | N/A(測試機專屬) | 同上，無溫控 |
| `EESUG_OFFSET` / `EESUG_Offest` | `:2567`：需 `CosFunction.bUseEESUGOffsetFunction`，解析 HandlerNo/SetupFile/OffsetUnit/ParameterName/EESUG/SUGMTBF/Max/Min | 0 | N/A(測試機專屬) | EESUG = 測試機接觸壓/壽命補償參數族 |
| `AUTO_CLEAN` | `:1881`：需 `bEnableAutoCleanFunction && iAutoClean_Function && !bRunAutoClean`，`fShowBinSelect->btnAutoCleanClick()` | 0 | N/A(測試機專屬) | Auto Clean = 測試座清針，Sorter 無測試座 |
| `YIELD_FAIL` | `:3006`：**限 `CUSTOMER_CODE==CC_KYEC_LEE`**，`iSECSGEM_ConsecutiveFailureAlarm=1`（連續測試不良告警） | 0 | N/A(測試機專屬) | 依據是「測試連續失敗」。注意這是**京元專屬**命令，理論上京元 host 可能送；但 HT160S 不測試、無良率概念，且當天 0 次 |

### B4. 危險 / 不建議開放（4 項）—— 詳見 §E

| RCMD | 9045 行為（引原始碼） | 京元當天次數 | 判定 | 風險 |
|---|---|---|---|---|
| `REMOTE_UPDATE_PROGRAM` | `uHGemHT9045.cpp:3027`：解析 CP `FILE` / `PATH`，組成 `PATH\FILE` 後 **`WinExec(S.c_str(), SW_HIDE)`** | 0 | **不建議** | host 可指定任意路徑並**隱藏執行**。等同遠端任意程式執行 |
| `DOWNLOAD_RECIPE_BY_FTP` | `:1421`：CP `Setup_File` -> `fFTPClient->bControlBySECSGEM=true` + `fFTPClient->ShowFTPModal(0)` | 0 | **不建議** | ① 從 SECS 接收執行緒開 modal；② 由 host 觸發外部 FTP 下載並覆蓋機台設定檔 |
| `PP_PASSWORD` | `:1895`：讀 name + password 存入 `asSECSGEMChangeName`/`asSECSGEMChangePassword`，然後 `fMain->ChangePassword()` | 0 | **不建議** | 遠端變更機台帳號密碼 |
| `REMOTE_SAVE` | `:1322`（且 `:1915` 有重複的第二個分支）：**分支主體只設 HCACK**（機內有 IC 或運轉中 -> 4，否則 0），沒有任何動作 | 0 | **不建議 / 暫不處理** | 9045 本身就是空動作 + 重複分支（死碼）。名稱暗示「遠端存檔」，語意未定義，不應憑名稱猜實作 |

---

## C. CEID 層（概要）

> 完整逐號矩陣見 [`./ceid-matrix.md`](./ceid-matrix.md)。本節只放可以據以行動的摘要。

### C-0. 量級（本次重新計數）

| 項目 | 數量 |
|---|---|
| 9045 韌體 CEID 目錄（`EventReport_CEID.def`） | **292** 個編號，其中 **286** 個具名（未具名：74 / 75 / 79 / 142 / 143 / 144） |
| HT160S 可發射的 CEID 編號 | **47** = 已註冊 **41**（1–31 共 31 個 + 35/36/37 + 148/149/150 + 272–275，見 `uHGemHT160.cpp:273-316` 與 `uHGemHT160.h:7-44`）＋ 有發射但未註冊 **6**（136/137/138 + 145/146/147，`aAuto1To6.cpp:747`，`bf9d048` 已由 140/141/142 改正） |
| host 以 `S2F35` **明確綁定報表**（＝真訂閱）的 CEID | **34** |
| host 以 `S2F37` 明確 **disable** 的 CEID | **1**（CEID 80） |
| 當天實際上線的 `S6F11` | **645** 筆 / **46** 個不同 CEID |
| 因 CEID 被 disable 而中止發送的 `S6F11` | **10987** 筆 / 7 個 CEID（80 佔 10962；14/27/44/55/93/141 的中止**全部落在 host 自己 disable-all 的重設定視窗內**，例如 `SECSGEM_TextLog_13.txt:2589` CEED=0x00 -> `:5102` CEED=0x01 -> `:5174` 單獨 disable 80，一天重跑 3 次，非常態訂閱決策） |

### C-1. host 明確訂閱的 34 個 CEID，依主題歸類

| 主題 | 9045 CEID（訂閱數） | HT160S 能不能發（**按語意**，不按編號） | 判定 |
|---|---|---|---|
| **上下料握手 / AMR** | 272 AMR Supplement、273 AMR LDUnLD Status、274 AMR LDUnLD Finish、275 AMR LD ID（4） | **全部能**，且**同號同義**（`uHGemHT160.cpp:299-302`）。當天實際上線 2/4/4/2 筆 | **已對齊，無動作** |
| **操作員按鍵 / 機台狀態** | 1 Start Pressed、76 Start Pressed HasIC、27 Change Machine State、73 Mymessbox OK、9 Switch Real Dummy Mode（5） | **全部有語意等價事件**，但**編號不同**：1/76 -> HT160S 4/5、27 -> HT160S 1、73 -> HT160S 18/20、9 -> HT160S 31 | **不移植；屬編號碰撞**（見 C-3） |
| **頁面 / 設定切換** | 15 Switch Setup File、19 Enter Offset Page、14 Switch StartMode、26 Get Test Result（4） | 15 -> HT160S 2 RecipeChange ✔；19 -> HT160S 25 EnterTeach ✔；14 = tester run-start-mode ✘；26 = 測試結果 ✘ | 15/19 屬編號碰撞；**14 / 26 = N/A(測試機專屬)**。（若客戶想監看**分選模式**切換，HT160S 已有 SVID 66032 可查但無事件 -> 可評估） |
| **清機 / 供盤收尾** | 42 Clean Out Finish、49 Tray Feed Finish、50 Auto Clean Finish（3） | 42 -> HT160S 28 CleanOutOK ✔（編號碰撞）；49 -> HT160S 29 TrayFeedOK **已宣告但實際發不出來**（`CheckAllTrayFeedFinish()` 為 stub，`csystem.cpp:1812`）；50 = 測試座清針 ✘ | 42 編號碰撞；**49 = 真缺口（需先修 TrayFeed 完成判定）**；50 = N/A(測試機專屬) |
| **測試流程** | 10 Switch Tester Online、13 Switch Temperature Mode、34 Auto Clean Start、44 Site On Off、45 Arm On Off、58 Ready for ART、59 ART Receive Tray OK、61 RT Finish、63 FT Finish（9） | 全部無對應 | **N/A(測試機專屬)** —— 皆源自測試流程（tester 連線、溫度模式、測試座、ART/FT/RT）；Sorter 無測試 |
| **溫度 / EESUG 校正** | 125 EESUG Offest Select、126 EESUG Offest Modify（2） | 無 | **N/A(測試機專屬)** —— EESUG 為測試機接觸補償參數 |
| **省電** | 212 Energy Saving Start、213 Energy Saving End（2） | 無 | **N/A** —— HT160S 無省電子系統；與 `ENERGY_SAVING` 固定回 HCACK=2 的判定一致 |
| **稽核 / 統計** | 48 Change EC、54 UPH Record End、70 Barcode Reader Enter、78 Jam Skip IC Count、80 Read Now Handler Data（5） | 48 ✘；54 ✘（HT160S 有 UPH 值 `tRunData.UPH` 但無事件）；70 ✘（2D 讀取走 TopCCD/WebAPI）；78 部分（有 15 PressSkip 但不帶數量）；80 -> HT160S 30 TimeEvent 概念相近 | **48 建議實作**（host 綁的 RPTID 504 = SVID 20001/20002/20003，正是 Path-A 標為 partial 的「EC 變更稽核族」，而 host 當天真的寫了 24 次 EC）；**54 建議評估**；70 / 78 待客戶確認；**80 不要移植**（host 主動 disable 它，見 §E） |

小計：4 + 5 + 4 + 3 + 9 + 2 + 2 + 5 = **34** ✔

### C-2. 只有「當天上線過」但未被綁報表、且 HT160S 完全沒有等價事件的（可行動者）

| 9045 CEID | 意義 | 當天上線次數 | HT160S 現況 | 判定 |
|---|---|---|---|---|
| 123 | Safe Door On Off | **9** | 有六個安全門 sensor 與**現成的上緣偵測器** `RecordSafeDoorStates()`（`csystem.cpp:1821-1875`，目前只寫 log），但無 CEID | **建議實作**（小）—— 安全相關、host 當天收了 9 次、掛勾已存在 |
| 124 | Save Recipe | **8** | `RecipeManager.SaveLastRecipeName()`（`setup.cpp:98` / `:1234`、`main.cpp:1475`） | 建議評估（小） |
| 66 | Load Tray Finish | **5** | Loader 進盤完成有機構狀態但無事件；AMR 段只有 272–275 的車級握手，沒有 per-tray 進盤完成 | 建議評估（中）—— AMR 無人線的可見度 |
| 53 | UPH Record Start | **5** | 有 UPH 值無事件（與 C-1 的 54 成對） | 建議評估（中） |
| 47 | Change HandlerSpeed | **2** | 有速度頁（`uspeed.cpp`）無事件 | 暫不處理（低價值） |
| 20 / 22 / 23 | Enter Speed / Message / Debug Page | 1 / 2 / 9 | HT160S 的頁面事件只到 22 EnterSetup / 23 EnterMaintenPage / 24 EnterIOPage / 25 EnterTeach / 26 EnterSECSPage，沒有 Speed / Message / Debug | 暫不處理（低價值） |
| 67 / 250 / 55 / 60 | Tray Test Finish / START Auto contact height / Initial ART Start / ART Receive Tray START | 6 / 1 / 1 / 1 | 無 | **N/A(測試機專屬)** |
| 41 / 2 / 3 / 4 / 6 / 25 / 28 / 17 / 18 / 21 | One Cycle Finish / Pause / OneCycle / CleanOut / Lot Start / Home / Retry Pressed / Enter Tool / Enter Maintenance / Enter IO Page | 3 / 24 / 5 / 6 / 2 / 4 / 9 / 5 / 1 / 3 | **全部有 HT160S 等價事件**（27 / 6 / 8 / 9 / 11 / 7 / 14 / 22 / 23 / 24） | 不移植；屬編號碰撞（C-3） |
| 136 / 137 | Auto 1 / 2 Unloading tray | 2 / 3 | HT160S 同號同義（`aAuto1To6.cpp:747`） | 已對齊 |

### C-3. 編號碰撞（CEID 1–31）—— 立場不變

- 兩邊都存在的編號中，**語意一致者僅有 AMR 272–275、Auto Full 35/36/37/148/149/150、Auto Unloadtray 136–138/145–147**；`1–31` 整段**同號不同義**。
- 殺傷力最大的幾筆（9045 當天上線次數）：**27** Change Machine State（**406**，全日最高）vs HT160S 27 = One Cycle Finish；**2** Pause Pressed（24）vs HT160S 2 = Recipe Change；**14** Switch StartMode（16）vs HT160S 14 = Press Retry；**26** Get Test Result（12）vs HT160S 26 = Enter SECS Page。
- **這是商務決策，不是工程決策。** 維持既有立場：**不單方面 renumber**。客戶規格書 §7.2 已聲明「HT-160S 的 CEID 1–31 為本機自有語意，host 綁報表時請依本規格 §3.3」。
- 現場減災事實（來自 Path-A 定稿）：京元 host 是用 `S2F35` 把**自己的 RPTID** 綁到它要的 CEID 上，不吃機台的 CEID 語意表；因此碰撞是 discovery-gated 風險，而不是當下就會壞的東西。
- 可量化的風險：host 一天綁 34 個 CEID、收 645 筆 S6F11。若 host 沿用 9045 字典，HT160S 送出的 CEID 1–31 會被**當成 9045 的意義解讀** —— 例如 HT160S 的「One Cycle 完成」(27) 會被讀成「Change Machine State」。**不會斷線、不會 T3，但會產出錯誤的 MES 記錄。**

### C-4. CEID 層建議動作總結

| 動作 | CEID | 依據 |
|---|---|---|
| 建議實作 | 安全門 On/Off（對應 9045 123）、EC 變更（對應 9045 48，含 SVID 20001–20003） | host 訂閱 / 當天上線 + 掛勾已存在 |
| 建議評估 | Load Tray Finish（66）、Save Recipe（124）、UPH Record Start/End（53/54）、修好 TrayFeed 完成判定讓已宣告的 CEID 29 真的能發 | 當天上線 5–8 次；TrayFeed 為既有宣告未生效 |
| 待客戶確認 | Barcode Reader Enter（70）、Jam Skip IC Count 帶數量（78）、分選模式切換事件 | 有訂閱但 HT160S 資料來源不同（WebAPI/CCD） |
| 不移植 | Read Now Handler Data（80）、全部測試流程 CEID、省電 CEID（212/213） | host 主動 disable 80；其餘無 sorter 對應 |
| 不動 | CEID 1–31 編號 | 商務決策 |

---

## D. 優先順序建議

### P0 —— **空的**

**明確聲明：目前沒有 P0 項目。**
20260728 那一批工作（5 組 message + 4 個 RCMD + Auto4–6 CEID renumber `bf9d048` + `START_AGV` 三缺陷 `f7f3939`）把**所有帶現場證據的 host 直接受害項目**都清掉了：
- host 送了不回覆的 5 組訊息 -> 全部會回覆（A-0）
- host 送了會被拒的 4 個 RCMD -> `ONE_CYCLE`/`PP_SIGNALTOWER`/`PP_MUSIC` 會執行，`ENERGY_SAVING` 回覆與京元自家 9045 逐 byte 相同
- `START_AGV` 帶 `LoaderICCount` 會整包 HCACK=2 -> 已修；`NA` 站會被誤鎖 -> 已修
- Auto4–6 Unloadtray 撞到 9045 的 140/141 -> 已改為 145/146/147

> **前提**：以上皆為工作樹未 commit 之變更，且驗證止於「編譯 + 模擬」。**上機驗證仍未完成**，這是目前唯一的實質風險，不是新的移植項。

### P1 —— 有現場證據、成本小

| # | 項目 | 證據 | 規模 |
|---|---|---|---|
| 1 | RCMD `CLEAN_AUTO_SORT_COUNT`：在既有 `CLEARCOUNT` 分支加別名 + 機內有 IC 回 HCACK=4 | **京元 host 當天送 2 次**，HT160S 現在回 HCACK=1 —— 這是 message/RCMD 層唯一殘留的現場證據缺口 | 小 |
| 2 | CEID：安全門 On/Off 事件（對應 9045 123） | 當天上線 **9 次**；HT160S 已有六門 sensor 與現成上緣偵測 `RecordSafeDoorStates()`（`csystem.cpp:1821`），只差發事件 | 小 |
| 3 | CEID + SVID：EC 變更稽核（對應 9045 48 + SVID 20001/20002/20003） | host **以 `S2F35` 綁 CEID 48 到 RPTID 504**，而 RPTID 504 的內容正是 `S2F33` 定義的 SVID 20001/20002/20003；同一天 host 寫了 **24 次 `S2F15`**。Path-A 亦把 20001–3 列為 partial | 小 |
| 4 | 修正客戶規格書兩處與程式碼不符之處（見「未決事項」） | 規格書 §2 稱會送 `S9F3`，但 `uHGemClass.cpp:265` 只寫 log；§7.2 稱 CEID 148/149/150 為 HT-160S 專屬，但 9045 目錄裡它們就是 Auto 4/5/6 Full，且 `AddCEID` 註解自己寫「9045-aligned」 | 小 |

### P2 —— 有現場流量或明顯不對稱，成本中

| # | 項目 | 證據 | 規模 |
|---|---|---|---|
| 5 | 修好 `CheckAllTrayFeedFinish()` 並打通 `Run_TrayFeed`，讓已宣告的 CEID 29 / `TrayFeedOK` 真的能發；之後才談 RCMD `TRAY_FEED` | HT160S 已宣告事件卻發不出來（`csystem.cpp:1812` stub + `:1673` 註解掉的入口）；9045 對應 CEID 49 被 host 訂閱 | 中 |
| 6 | 遠端 Lot 收尾：RCMD `STOP_LOT`（+ 視需要 `CLEAR_LOT_INFO`） | **不對稱**：host 能遠端開 Lot（`LOTSTART`，當天 3 次）但不能遠端收 Lot；Soter CSV / UPH 凍結 / per-Lot 統計全掛在 Lot End。`SET_LOT_INFO` 空 list 是 HCACK=2（`uHGemHT160.cpp:737`），沒有替代路徑 | 中 |
| 7 | CEID：Load Tray Finish（66）、Save Recipe（124）、UPH Record Start/End（53/54） | 當天上線 5 / 8 / 5 次 | 中 |

### P3 —— 需與客戶確認，或屬 discovery-gated

| # | 項目 | 證據 | 規模 |
|---|---|---|---|
| 8 | CEID 1–31 編號碰撞的最終處置 | host 一天綁 34 個 CEID、收 645 筆 S6F11；碰撞會產出錯誤 MES 語意但不斷線。**商務決策** | 大（若 renumber） |
| 9 | `S6F17/F18`、`S1F23/F24`、`S103F11/F12` | 皆為已實作能力的「加值/自我描述」變體，host 當天 0 次 | 小～中 |
| 10 | `S14F3/F4`（2DID -> Bin map over SECS） | HT160S 已有 WebAPI + 檔案匯入兩條路；是否再開 SECS 路徑為客戶決策 | 大 |
| 11 | `S7F19/F20` 回空 PPID 清單、`S7F1/F3/F5/F17` 與 `S14F1` 的「有 case 卻不回」清理 | 目前 host 若送會 T3；但當天 0 次。清理方向：移掉 case 或回最小 ack，二者擇一並寫入規格書 | 小 |
| 12 | `S2F29/F30` EC namelist | host 當天 0 次（它有自己的字典表）；成本小，屬完整度 | 小 |

---

## E. 不建議移植（本節的目的是保護機台）

### E-1. 會讓 host 取得機台控制權或寫入權

| 項目 | 9045 做什麼 | 為什麼不做 |
|---|---|---|
| RCMD `REMOTE_UPDATE_PROGRAM` | `uHGemHT9045.cpp:3027`：CP `PATH` + `FILE` -> `WinExec(PATH\FILE, SW_HIDE)` | **遠端隱藏執行任意程式**。HT160S 的版本更新已有專屬通道（NSIS exe-only updater），不需要也不應該經 SECS |
| RCMD `DOWNLOAD_RECIPE_BY_FTP` | `:1421`：`fFTPClient->ShowFTPModal(0)` 並覆蓋 Setup File | ① 從 HSMS 接收執行緒開 **modal**（本機 modal 會停所有馬達，這是 `S10F4` 決定不彈窗的同一個理由）；② host 觸發外部 FTP 覆蓋機台設定檔，沒有任何本地確認 |
| RCMD `PP_PASSWORD` | `:1895`：`fMain->ChangePassword()` | **遠端變更帳號密碼**。HT160S 的 `THT160UserRoleManager` 是本機權限真源，不應由網路端改寫 |
| `S101F5/F6`、`S101F7/F8` | `uHGemClass.cpp:2580`：`mkdir(CurrentDirectory\PATH)` + `fopen(...,"wb"/"ab+")`，路徑與檔名**由 host 給** | **遠端任意路徑寫檔**。沒有白名單、沒有大小限制 |
| `S125F3/F4` | `uHGemHT9045.cpp:5133`：`SetECValue(LSID,&LEVEL)` + `fSecurity->SetLevelSet()` | **遠端改權限層級**。等於讓 host 決定誰能進維修畫面 |

### E-2. 9045 本身就是空殼 / 死碼 —— 移植它等於移植 bug

| 項目 | 事實 |
|---|---|
| RCMD `TRAY_MAP` | `uHGemHT9045.cpp:2512` 分支主體**只有 `HCACK=0;`**。收到後什麼都不做 |
| RCMD `REMOTE_SAVE` | `:1322` 與 `:1915` **兩個重複分支**，主體都只設 HCACK，沒有動作 |
| `S110F6` / `S110F8` / `S120F2` handler | 在 `uHGemHT9045.h:304-327` **未被 override**，仍是 `uHGemClass.h` 裡的空 `{}`。9045 收到也不做事 |
| 9045 `S125F2` 的「每個 ECID 各回一次」 | `uHGemClass.cpp:2627` 在解析迴圈內逐 ECID `LocalAcknowledge(125,2,...)`；現場 6 個 request 對 **138** 個 secondary，且因 `InitLocalHead` 沿用 request SystemByte，這些回覆的 **transaction id 全部相同** —— 是協定缺陷。HT160S 刻意每 request 只回一次 |

### E-3. 與已實作功能重複

| 項目 | 已有的替代 |
|---|---|
| RCMD `HALT` | `PAUSE`（`uHGemHT160.cpp:876`）/ `STOP`（`:1204`）；且 9045 的 HALT 被 `bRCMDStart`+`bPhysicalStart` 鎖住，多數情況只回 HCACK=2 |
| RCMD `REMOTE_START` | `START`（`uHGemHT160.cpp:1081`），行為相同 |
| RCMD `START_LOT` | `LOTSTART`（累加）+ `SET_LOT_INFO`（覆蓋），且 9045 版 CP 集為 Onsemi 專屬 |
| RCMD `PP_SELECT` / `PP-SELECT` | 配方切換已可經 **ECID 1501** + `S2F15`（規格書 §3.2 / §7.1 已宣告）。再開一條會出現兩個真源 |
| `S100F3/F4` | 標準 `S5F5/S5F6`，且 HT160S 由 `mapAlarmCodeList` / `AlarmList.csv` SSOT 即時產生（約 480+ 碼） |
| `S103F11/F12` | `S1F11/F12`（namelist）+ `S1F3/F4`（值） |

### E-4. host 自己已經拒絕的

| 項目 | 事實 |
|---|---|
| CEID 80 `Read Now Handler Data` | 9045 每約 5 秒觸發一次；京元 host 在**每一次**重設定 session 都用 `S2F37 CEED=0x00 L,1{80}` 單獨把它關掉（`SECSGEM_TextLog_13.txt:5174`、`_14.txt:2929`、`_16.txt:4813`），全日累計 **10962** 筆被中止。做了只會被關掉，並增加 645 -> 11607 的訊息量 |

### E-5. 測試機專屬 —— 不要憑名稱硬湊對應

`AUTOSITEMAP` `AUTO_RETEST` `RETEST_MRT` `INITIAL_START_MRT` `CONTINUE_START_MRT` `CONTINUE_START_ART` `CONTINUE_RETEST_ART` `INITIAL_START` `INITIAL_START_ART` `SWITCH_TO_FT` `SWITCH_TO_RT` `TESTTEMPSETTING` `DEVTEMPOFFSETADJUST` `EESUG_OFFSET` `AUTO_CLEAN` `YIELD_FAIL`
以及 CEID 10 / 13 / 26 / 34 / 44 / 45 / 50 / 55 / 58 / 59 / 60 / 61 / 63 / 67 / 125 / 126 / 250 —— 全部根植於測試流程（測試座位圖、ART/MRT/FT/RT、溫度與接觸補償、良率、清針）。
HT160S 是 **sorter**，沒有測試站、沒有溫控、沒有良率。**不要為了「完整度」造一個假的對應** —— 一個回 HCACK=0 卻什麼都不做的分支，比一個誠實的 HCACK=1 更危險，因為 host 會相信它成功了。

### E-6. 不要單方面 renumber CEID 1–31

編號屬客戶介面契約。維持既有結論：規格書宣告 HT160S 語意為權威，實際處置與客戶確認後再動。

---

## 未決事項 / 需要人確認

1. **規格書與程式碼不符（兩處，本文未修改任何檔案）**
   - `docs\SECS\HT160S_SECS_Interface_Spec_20260727.md` §2「Stream 6 / 9」列 `S9F3 | Unrecognized Stream/Function | E→H | B[10] MHEAD | 收到未實作的 primary 時送`，但 `uHGemClass.cpp:265-268` 的 handler **只寫 log、不上線**。實際行為是完全靜默 -> host T3。
   - 同文 §7.2 把 **CEID 148 / 149 / 150** 列為「HT-160S 專屬（9045 無對應）」，但 `EventReport_CEID.def` 裡 148/149/150 就是 `Auto 4 / 5 / 6 Full`，且 `uHGemHT160.cpp:304-315` 的註解自己寫「9045-aligned」。§7.2 該列需修正。
2. **CEID 74 / 75 / 79 / 142 / 143 / 144** 在 9045 目錄中有編號但**未具名**（alias 欄為空或只有數字）。意義**未知**，不做任何推測。若客戶要用到這些編號需向 9045 端索取定義。
3. **RPTID 994–999 / 804 / 900 / 2000 / 2001** 出現在 host 的 `S2F33` 定義中，但未被任何 `S2F35` 綁到 CEID（2000/2001 除外，綁在 272–275）。用途未知。
4. **`STOP_LOT` / 遠端 Lot 收尾** 是本清單中唯一「無現場證據但有明顯營運不對稱」的項目，需與客戶確認 AMR 無人線的 Lot 收尾責任歸屬。
5. **`YIELD_FAIL`** 在 9045 上是 `CUSTOMER_CODE==CC_KYEC_LEE` 專屬命令（京元專用）。當天 0 次，但由於它是京元專屬，建議向客戶確認其 host 是否在其他機型上會送。
6. 全部 20260728 變更**尚未上機驗證**（僅編譯 + 模擬 clean）。
