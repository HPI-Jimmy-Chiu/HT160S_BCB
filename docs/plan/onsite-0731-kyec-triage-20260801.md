# HT-160S 京元(KYEC)竹南 2026-07-31 現場測試 — 根因分析與修復計畫

- 日期：2026-08-01
- 分支：`feat/iosetview-172-refactor`（起點 HEAD `3103d8d`）
- 現場資料：`D:\HT160S_StateRecord\` 六份 07-31 State Record zip + 操作員手寫 8 條筆記 `20260731.txt`
- 現場韌體：`0ba540a` + 兩處本機修改（沿用 07-30 triage 的判定），程式於 `14:30:02` 與 `16:13:33` 各啟動一次
- 方法：5 個平行 finder（SECS 傳輸 / SVID 對齊 / Empty+TrayArm / 顯示 / State Record）→ 逐條對抗式複驗 → 本文
- 覆蓋：53 條發現，38 條完成複驗（7 CONFIRMED、30 PARTIAL、1 REFUTED），15 條 State Record 項因 API 網路錯誤未複驗，已於文中標示

---

## 1. 摘要

現場八條筆記裡，**六條已定位到確切的程式行**，一條（`lot number dot`）證據不足無法定位，一條（`off line remove client`）需要京元一句話確認語意才能動工。

真正的重點不是任何單一缺陷，而是**兩個系統性落差**：

1. **SECS 資料面幾乎是空的。** 京元 host 當天用 33 張 S2F33 報表訂了 **389 個 SVID**，HT-160S 只答得出 **27 個（6.9%）**。最痛的是 `RPTID 502`（機台上下文）8 格只填 1 格，而 host 把它綁在 **CEID 27（Change Machine State，當天實際上線最高頻事件）** 以及全部四個 AMR 事件上。這份 log 同時解掉了 07-29 對齊計畫卡住的外部索取項——我們現在有客戶完整的 SVID 需求清單。
2. **生產計數器的生命週期是壞的。** `TotalIC` 跨電源累積、`StartTime` 每次開機重設，兩者相除得到的 UPH 在 07-31 被觀測到**高估 20 倍**（實際 99，回報 2026），而且這個數字 host 有訂閱（`RPTID 508` → SVID 1021）。`Load`/`Total` 兩個面板則是**從 HT172 移植時只搬了 DFM、沒搬賦值程式碼**，永遠顯示設計期字面值 `0`。

使用者對 Empty/TrayArm 的兩個觀察 **都正確**，但兩者都是效率議題、不是故障，且都有不能直覺硬拆的陷阱（見 §5）。

---

## 2. 對操作員 8 條筆記的逐條裁定

| # | 筆記原文 | 判定 | 根因一句話 | 修復動作 | 嚴重度 |
|---|---|---|---|---|---|
| 1 | SECS passive unconnect record | **已定位** | passive 模式下重連看門狗的 socket 動作被 `!Active` 擋掉是 no-op，但 `StringOut()` 在 guard 外面，照樣每 60 秒印一行 | 把 log 移進 guard；補上邊緣觸發的斷線/連線記錄 | 中 |
| 2 | off line remove client | **部分／待確認** | HT-160S 的 HSMS 引擎**完全沒有 Separate.req 送出路徑**（只有接收）。但 9045 的「OffLine 30 秒自動斷線」複驗證實是 **CC_KYEC_LEE(邏輯廠) 專屬、且觸發條件是 comm-establish 卡住，不是 host OFF-LINE** | 先補 `StopCommunication()` 的 graceful Separate（零風險）；30 秒自動斷線需京元確認語意後再做 | 低（現場未造成故障） |
| 3 | svid 1011 machine status | **已定位** | HT-160S 從未註冊 SVID 1011；資料源 `fMain->palMainStatus` 一直都在 | 註冊 1011 → 鏡射 `SetMainStatus()` 的字串 | 高 |
| 4 | Lot number 1006 | **已定位** | SVID 1006 = Lot ID（本次由 9045 的 S2F15 線上證據確認，不再需要客戶確認）；HT-160S 只把批號放在自有的 66031 | 新增 `svActiveLot` 綁 1006，**不動** 66031 的既有語意 | 高 |
| 5 | lot number dot | **無法定位** | 六種候選機制已排除，剩兩種無法從現有素材判定 | 需要現場截圖或該筆批號原文（見 §10） | — |
| 6 | jam rate | **已定位** | HT-160S 根本沒有 jam rate：`JamCount`/`JamRate` 欄位從 HT172 搬過來了，**計數點、公式、顯示頁三層都沒搬**；`fData` 是空殼 Form | 需先與京元敲定定義（HT172 的 per-IC ppm vs 9045 的 per-tray「1 in N」兩者不同） | 中 |
| 7 | Load and total number is zero | **已定位** | 兩個獨立疏漏：(a) DFM 面板搬了但 HT172 `ShowBinCount()` 的兩行賦值沒搬；(b) `tRunData.LoaderIC` 全樹**沒有任何 `++`** | 補 `LoaderIC++` + 補渲染；`Total` 的渲染必須等 #8 的批次歸零一起 | 高 |
| 8 | UPH number computed 3 number | **已定位** | 分子 `TotalIC` 跨電源保留、分母 `StartTime` 每次開機重設 → 比值無意義 | 加 UPH 基準線 `g_iUphBaseIC`，分子改用差值 | 高 |

---

## 3. SECS 傳輸層（筆記 1、2）

### 3.1 【SECS-T03】passive 重連看門狗是純 no-op，只負責洗版　嚴重度：中

`SecsGem/uHGemEquipment.cpp:275-283`：

```
if(ServerSocket1 != NULL && !ServerSocket1->Active)   // <- 開機後永遠 false
{
    ServerSocket1->Port   = port;
    ServerSocket1->Active = true;
}
S.sprintf("[SECS] reconnect #%d (passive listen :%d)", iReconnectAttempts, port);
StringOut(S);                                          // <- 在 guard 外面
```

`ServerSocket1->Active` 開機時在 `StartCommunication()`（`:1784`）設 true 後**再也沒有人清掉**：全樹只有五個寫入點（`:101` ctor、`:170` dtor、`:278` 本身、`:1784`、`:1799` 的 `StopCommunication()` 而後者**零呼叫者**）。`DropConnection()` 的 socket 重置被 `if(bActiveMode && ...)` 擋住（`:1986-1989`），`OnPeerDisconnected()`（`:2005-2023`）完全不碰 socket。所以 `:277-278` 開機後不可達，`DoReconnectAttempt()` 在 passive 模式的唯一副作用就是計數器加一 + 印一行。

實測（`2026-07-31 16_38_37` 的 `SecsLog\2026_07_31\`）：

- 檔案 `00`–`13` 各 59 行、**59 行全是 reconnect、0 行真實 SECS 內容**
- 當天第一行 `00:00:40.161 reconnect #351`，跑到 `14:18:44.162 reconnect #1195`，**連續 845 行、14 小時 18 分零內容**
- 全日 9200 行 / 286,477 bytes，其中 883 行 / 61,950 bytes（21.6%）是這個

**複驗修正了我最初的推論**：這**不是** 17×4KB 的 ring buffer，沒有任何內容被沖掉。`FlushSecsLogToFile()`（`:1885-1912`）是**按當前小時**開檔、`fopen("a+")` 純附加、無上限無輪替（檔案 15 長到 156,758 bytes）。代價是訊噪比、磁碟與 zip 體積，不是資料遺失。

決定性證據（複驗補上的，比 finder 原本的更硬）：`14:56:53.921` host 送 Separate → `14:57:41.168` 48 秒後 listener **直接 accept 了一條全新 TCP 連線**，而重連計數器從 `#12`(14:42:14) 直接跳到 `#13`(15:00:30)——**中間一次看門狗都沒動作**。這證明 listen socket 一直是綁著的，重新 listen 本來就沒必要。

9045 對照：idle listening 時**什麼都不印**。它的 passive body 在 `D:\HT9045\HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP\SECSGEM\uHGemEquipment.cpp:3548-3598`（`DoOpenCommuncation()` 的 else 分支），同樣用 `if(ServerSocket1->Active==false)` 當 guard，且**零 `StringOut`**；狀態顯示是邊緣觸發（`:4854`/`:4873`）。

**修復（三段，(a)+(c) 必做，(b) 選配）**

- (a) 把 passive 分支的 `StringOut(S)` 移進 `if(...)` 內，只在真的重開 listener 時印。active 分支維持每次撥號都印（那裡是真的嘗試）。
- (c) **必做，不是 nice-to-have**：補回可見性。目前這行雖然吵，卻是**唯一**能證明「連線斷了多久」的記錄——本次分析就是靠計數器連續性證明了 14h18m 的斷線。所以要補：`CONNECTED→NOTCONNECTED` 邊緣印一行、沿用既有的 `peer connected`（`:2002`），外加一個**低頻**（30–60 分鐘一次，不是 60 秒）的斷線中心跳，讓長時間離線仍留下時間軸。
- (b) 選配：`Timer1Timer` 於 `:242` 前加 early-out 並**不遞增** `iReconnectAttempts`（沒有嘗試就不該計數）。若採用，`iReconnectCountdown = iReconnectInterval;` 的重新裝填必須保留。
- 文字正名：passive 模式印 `reconnect #N` 是誤導（根本沒有 retry），改成 `listening :6000 (no host)`。

### 3.2 【SECS-T04 / T08 / T11 / T12】四個已 CONFIRMED 的小缺陷

| 代號 | 內容 | 嚴重度 |
|---|---|---|
| SECS-T04 | `iReconnectAttempts` 連線成功後**從不歸零**，畫面上的「attempts N」作為健康指標無意義 | 低 |
| SECS-T08 | 每個 header-only 訊息（S1F15/S1F17 依標準本來就零長度本體）都印一行假的 `[SML parse error rc=-1]`，讓乾淨的交握在現場看起來像壞掉 | 低 |
| SECS-T11 | 收到 `Separate.req` 時 `peer disconnected` 印兩次、link-lost teardown 跑兩次 | 低 |
| SECS-T12 | host 只送 Select 沒送 S1F13，HT-160S 照樣完整交易——communication-established 沒有被追蹤 | 低 |

### 3.3 【SECS-T02】關機/重啟沒有 Separate，16:13 之後 147 筆訊息全數丟失　嚴重度：高

```
16:13:25.777  [SECS][TX] S6F11 CEID=24 (Exit Pressed)  W=1 len=144 (sent)
16:13:25.785  [SECS][RX] S6F12 dispatch                 <- host 有 ack，link 健康
16:13:34.053  [SECS] StartCommunication mode=passive port=6000   <- 8.3 秒後重啟
   ... 之後到 16:38:33.498 為止，每一筆送出都是 "skipped (not selected)"
```

**修正後的損失量**（複驗用計算機重算過，finder 的 147 拆錯）：`16:13:34` 之後被抑制 **147 筆 = 141 筆 S6F11 + 6 筆 S5F1**（3 個警報的 set+clear：ALID 1671881696 / 3184277272 / 3184278233）。丟掉的事件包含 CEID 8 Lot End ×1、CEID 1 Start ×5、CEID 2 Pause ×5、CEID 53/54 UPH Record Start/End ×20/×13、CEID 66 Load Tray Finish ×15、CEID 136-138/145-147 Auto-N Unloadtray ×26、CEID 27 ×24。全日共 241 筆（1 / 83 / 0 / 10 / 147）。

程式面：`main.cpp:709` 送 CEID 24 後直接 `Close()`；socket 只在 `~THGem`（`uHGemEquipment.cpp:169-170`）被硬拔，**沒有任何 HSMS 層的道別**。`StopCommunication()`（`:1793-1806`）是死碼。

9045 的作法（`D:\HT9045\...\SECSGEM\uHGemEquipment.cpp:7899-7915`）在**同一個 CEID 24** 上：`DoSeparate(); ServerSocket1->Close(); ClientSocket1->Close(); ... Close();`。

**修復（複驗修正過的 5 點）**

1. 不要新造 sender，直接 clone `THGem::SendLinktestReq`（`uHGemEquipment.cpp:1946-1972`）：14 byte frame、`buf[9]=HSMS_STYPE_SEPARATE_REQ`(=9)、新的 `uControlSystemByte`、`SendBuf` 包 try/catch，**但不要 arm `bAwaitLinktestRsp`/`iT6Countdown`**（Separate 無回覆）。
   ⚠ SessionID **不可**照抄 Linktest 的 `buf[4]=buf[5]=0xFF`——那對 Linktest 才正確；Separate 要帶設定的 `DeviceID`（`uHGemEquipment.h:150`，已用於 `:1479`）。
2. `main.cpp` **不能**直接呼叫 `THGem`。它進 SECS 的唯一途徑是 `SecsGem/UsecegemMainFrom.cpp:137-143` 的自由函式 `EventReport()`；照樣加一個同樣有 `USE_SECS_GEM<=0 || HGem==NULL` 雙 guard 的 wrapper。
3. **必須冪等**。`sbExitClick` 的 `Close()`（`main.cpp:710`）會跑 `FormClose`，兩處都呼叫會重複觸發；關掉 `ActiveSocket` 會同步引發 `seDisconnect` → `OnPeerDisconnected` → `HT160Gem::OnCommunicationLost`（`uHGemHT160.cpp:2348-2356`），那裡會寫 EventLog + fNote，沒有 early-return 每次結束都會多一行。若放在 `FormClose`，必須放在**最前面**（在 `MyThread->Terminate()/WaitFor()` 和 `delete Alarm` 之前）。
4. 直接把它**併進既有的 `StopCommunication()`** 而不是新增函式——它已經做了 `bWantComm=false`、兩個 `Active=false`、`ActiveSocket=NULL`、狀態重置，只缺 Separate 送出與 `FlushSecsLogToFile()`，而且死碼終於有了呼叫者。`bWantComm=false` 是關鍵：`Timer1Timer` 的 guard 在 `:206`，少了它看門狗會在程式關閉途中重開 listener 並印 `reconnect #1`。
5. 順序無需延遲：`SendLocalData→SendBuf` 是同步的，TCP 保序，CEID-24 的 bytes 一定先於 Separate。

### 3.4 【SECS-T01】「off line remove client」— 複驗推翻了直覺解法　嚴重度：低（待客戶確認）

現場確實發生過：

```
14:45:29.721  S1F18 ONLINE acknowledged  (control state -> Online-Remote 5)
14:45:40.193  S1F16 OFF-LINE acknowledged (control state -> Off-Line 1)
   ... HT-160S 繼續交易：S2F15 14:47:33、S2F33 14:47:57、S6F11 14:51:06/08/14 全部 (sent)
14:56:53.921  [SECS] Separate.req -> closing connection    <- 11分14秒後，host 自己斷的
```

**但複驗推翻了「照抄 9045」這條路**：

- 9045 那段 30 秒自動 Separate（`uHGemEquipment.cpp:4893-4917`）的觸發條件是 `GEMCommunicatingState->Caption == "1:OffLine"`，而那個 caption **不是 GEM control state**——它由 `bConnect` 驅動（`:4926-4956`），是 E30 的 COMMUNICATING 狀態（S1F13/F14）。非 KYEC 客戶同一個位置顯示的是 `4:Enable`/`1:Disable`；`1:OffLine` 只是 `CC_KYEC_LEE` 的**改字**（`:4949`）。真正的 control state 顯示在另一個 `GemPanelControlState`（`:4963-4988`）。
- 9045 **自己的 S1F15 handler 跟 HT-160S 一樣什麼都不做**：`uHGemClass.cpp:402-414` 回 OFLACK=0 後呼叫 `OffLine()`，而 `uHGemEquipment.cpp:5802-5806` 的 `OffLine()` 只是 `bOnLine=false; bStartOnLine=false;`——不碰 `bConnect`，所以那個 30 秒計時器在 host OFF-LINE 時**證明不會觸發**。同樣序列下 9045 的行為會和 HT-160S 一模一樣。
- 而且該區塊 gated 在 `CUSTOMER_CODE==CC_KYEC_LEE`（921 = 京元**邏輯**廠）。`D:\HT9045\...\MachineType.h:283-288` 列出六個不同的 KYEC 代碼，**沒有一個是竹南**。

另外複驗也排除了另一條直覺路線：「OFF-LINE 就不該發事件」。9045 唯一的 offline 不上報 gate 在 `uHGemEquipment.cpp:7866`，條件是 `CUSTOMER_CODE==CC_TFME_CHINA`，**不是 KYEC**。所以 HT-160S 目前在這點上**已經與 9045-for-KYEC 對齊，不要去「修」它**。

真正成立的事實只有一條：**HT-160S 的 HSMS 引擎是 receive-only，全樹沒有任何 Separate.req 送出路徑**（`uHGemEquipment.h:55` 定義了 SType，`uHGemEquipment.cpp:2132-2139` 是唯一使用處，且是 RX case）。9045 有 `DoSeparate()` 並在三處呼叫，都不是 host OFF-LINE：(a) KYEC_LEE 的 comm-establish 卡住 30 秒、(b) ini 關閉 SECS/GEM 功能、(c) CEID 24 離開程式。

**修復**：先做 §3.3 的 `StopCommunication()` graceful Separate（同時涵蓋 9045 的 (b) 與 (c)，零風險）。**不要**把任何行為綁在 `iControlState` 上——複驗查證它是 write-only 的 SVID 鏡射，唯一讀者是 `uHGemHT160.cpp:376` 的 SVID 66002 註冊。30 秒自動斷線需要京元竹南先確認語意（見 §10）。

---

## 4. SVID / 報表對齊（筆記 3、4、5）

### 4.1 客戶契約：host 當天實際下的報表定義

從 07-31 的 log 機械解析（解析器與原始 CSV 存於 `docs/plan/onsite-0731-kyec-secs/`）：

- **33 張 S2F33 報表定義**、**31 條 S2F35 事件綁定**、**389 個相異 SVID**
- HT-160S 註冊 52 個 SVID，與 host 需求的交集 **27 個 → 覆蓋率 6.9%**

這份資料**解掉了 `docs/plan/secs-9045-full-align-plan-20260729.md` §10 第 1 項的外部阻塞**（「需向京元索取 9045 的 SVID 目錄傾印」）——現在我們有客戶自己送過來的完整需求清單，比目錄更權威。

高價值報表（依 host 綁定的 CEID 數量排序）：

| RPTID | 格數 | SVID | 綁在哪些 CEID | HT-160S 覆蓋 |
|---|---|---|---|---|
| **502** | 8 | `1006 1007 1011 3 1501 1517 1518 1513` | **27**, 9, 10, 13, 14, 15, 19, 26, 34, 42, 44, 45, 48, 49, 50, 73, 76, 80, **272, 273, 274, 275** | **1/8**（只有 1518） |
| **501** | 12 | `1101…1108 16296…16299` | 1, 26, 42, 49, 76 | **0/12** |
| 2000 | 7 | `38202 38205 38206 38207 38219 38220 38221` | 272, 273, 274, 275 | **7/7** ✅ |
| 2001 | 15 | `38222…38236` | 272 | **15/15** ✅ |
| 508 | 6 | `1021 1028 1009 1023 1024 1025` | 1, 26, 42, 49, 76 | 1/6（1021 UPH，但值是錯的，見 §6.1） |
| 505 | 179 | （測試機參數大宗） | 1, 42, 49, 76 | 0/179 |
| 509 | 38 | `4900 1519 1520 1051…1082 1601…1604` | 1, 13, 42, 49, 76, 80 | 0/38 |

**RPTID 502 是投資報酬率最高的一格**：host 把它綁在 22 個 CEID 上，包含當天實際上線最高頻的 CEID 27。

### 4.2 決定性證據：host 的 S2F35 是「取代」不是「疊加」

`15:03:17.110` 的 CEID 27 本體是 `<L[1] <502 <L[8] L0 L0 L0 L0 L0 L0 <I4[1] 2> L0>>>`——**韌體預設的 report 1 不見了**。也就是說 host 完成 provision 之後，CEID 27 從「13 格有值的韌體上下文」變成「1 格有值」。這是真正的退化形狀，也是補 502 的最強理由。

`S6F16` 回 CEID 1 時同樣是 9 張報表、每一格 `L[0]`。

### 4.3 RPTID 502 逐格裁定

| 槽 | SVID | 9045 語意 | HT-160S 現況 | 裁定 | 動作 |
|---|---|---|---|---|---|
| 1 | **1006** | **Lot ID**（ECID，`uHGemHT9045_EC.cpp:57` → `fLotInfo->edtSysLotID`） | 只有自有 66031 | **B：有資料沒曝露** | 新增 `svActiveLot` 綁 1006 |
| 2 | **1007** | Operator ID | 2026-06-24 起有 UserRoleManager | **B** | 綁登入者 ID |
| 3 | **1011** | **Machine State**（`uHGemHT9045_SV.cpp:71` → `fMain->palMainStatus`） | `palMainStatus` 完全等價 | **B** | 鏡射 `SetMainStatus()` 的字串 |
| 4 | 3 | GemClock | 有 1027 但**格式不同**，不能直接別名 | B（需轉格式） | 依 9045 `uHGemEquipment.cpp:6359` 格式 |
| 5 | 1501 | Setup File（recipe） | 存在於 EC namespace，但只在 host 送 S2F13 時刷新 | B | 改為 serialize 時取值 |
| 6 | 1517 | Start Mode | 9045 值域**12 值**，HT-160S 的 RunMode 不能直接對應 | 需客戶確認對照表 | 待確認 |
| 7 | 1518 | Real/Dummy | ✅ 已註冊（`uHGemHT160.cpp:365`） | — | — |
| 8 | 1513 | Tester On/Off | HT-160S 是 sorter，**無測試機構** | **C：機構差異，合法的「不做」** | 回空即可 |

**1006 的語意已由線上證據定案，不再需要問客戶。** 9045 的 06-08 log 裡有三筆 `[S2F15] New Equipment Constant Send` 帶 `<U4[1] 1006> <A[10] "LQ50SIJAG2">`（`D:\backup_version\HT9046\KYEC\20260626\2026_06_08\SECSGEM_TextLog_15.txt:2902-2911` 等）。S2F15 依定義只定址 equipment constant，所以 1006 就是 ECID「Lot ID」，值是批號字串。這直接結掉 `docs/plan/secs-9045-porting-20260729/svid-ownership.md:105` 留下的待確認項。
（`SECSGEM.cpp:725` 那個 `"Site Ag Socket ID"` 的競爭註冊是**死碼**：該檔不在 `HT9045.bpr` 的六個 SECSGEM 來源裡，且它 `#include "SECSGEM.h"` 而全樹沒有這個標頭，根本無法編譯。）

### 4.4 RPTID 501：這正是筆記第 7 條的 SECS 面

`docs/plan/secs-9045-porting-20260729/svid-ownership.md:108-109`：SVID **1101 = Loader Count** 對應 `tRunData.LoaderIC`、**1102 = Output Total Count** 對應 `MachineRun.iTotalSorted`（已有的 66021，純別名）。

也就是說 **`LoaderIC` 沒有 `++` 這件事，同時餓死了螢幕面板和 host 報表兩邊**（§6.2）。這也是為什麼 `docs/plan/counter-audit-cleanup-plan-20260714.md:29` 建議「優先刪除以免誤導」的判斷必須**推翻**：死掉的 DFM 面板 + host 的 RPTID 501 一起證明 `LoaderIC` 是**未完成的移植**，不是死碼。

### 4.5 實作注意（複驗查證過的 BCB6 細節）

- **不可**照抄 9045 把 `TPanel*` 直接餵給 `SetSVDataPointer`。HT-160S 的 `THGem::SetSVDataPointer`（`uHGemEquipment.cpp:952`）只有 `void*` 一個多載，而 `DataItemOutSVItem`（`:653-668`）會把 ASCII 的 `Ptr` 直接 cast 成 `AnsiString*`——傳 `TPanel*` 是野指標讀取。9045 有四個多載含 `TObject*` 版本（`uHGemEquipment.h:335-338`），HT-160S 沒有。
- 1011 的更好作法：不要在 `RefreshSVData()` 讀 VCL 屬性，而是在唯一寫入點 `csystem.cpp:49`（`SetMainStatus()` 內，值本來就是 `AnsiString`）鏡射到一個 file-scope `AnsiString`，再綁那個。少一次 serialize 路徑上的 VCL 讀取，且 `palMainStatus` 改名或雙語 `palMainStatus_En`（`main.h:163`）分歧時仍正確。
- 1006 **不要**把 `svCurrentLot` 改指向 `ActiveLotID()`。`svCurrentLot` 是韌體 report 1 的第 12 格（`uHGemHT160.cpp:618`），那張報表的 13 格形狀已對客戶凍結、語意「first registered lot」已寫進規格書。要新增獨立成員 `svActiveLot`。
- **大小寫問題（需與京元確認）**：HT-160S 寫 `RUNNING`/`HOMING`，9045 寫 `Running`/`Homing`（`D:\HT9045\...\cmydef.cpp:5021-5023`）。host 若做字串比對會炸。但注意 `sMacStatus[30]` **不是** 9045 面板 caption 的完整值域——線上還出現過 `RT1 ART`、`FT ART`，所以參考清單要取 2026-06-08 的實際 log，不是那個陣列。HT-160S 專屬狀態（`Clean Out`/`Tray Feed`/`One Cycle`/`SAFE DOOR`/`AIR`/`MOTOR OFF`）需要客戶點頭；9045 說 `Power Off` 的地方 HT-160S 說 `MOTOR OFF`。
- 註解修正：`uHGemHT160.cpp:354` 把 1010-1190 稱作「count region」是錯的——9045 用 1010 = Machine Pre State、1011 = Machine State。就是這句註解會讓下一個人以為 1011 是刻意跳過的。

### 4.6 其他 SECS 發現

| 代號 | 內容 | 判定 |
|---|---|---|
| SECS-14 | host 綁了 RPTID 506/800/801 卻**從未定義**它們——當天的 provision 相對 06-08 基準是不完整的 | PARTIAL；需向京元確認是否漏送 |
| SECS-15 | AMR 的兩張報表（2000 / 2001）是唯一 100% 對齊的部分，07-31 在 HT-160S 自己的線上得到證實 → **07-29 計畫被阻塞的 S3 步驟現在解除風險** | PARTIAL（正面） |
| SECS-16 | 「host 唯一一次 EC 寫入被 machine-idle gate 擋掉」 | **REFUTED** |
| SECS-T05 | 沒有 T7 等價物：連上但不 Select 的 peer 會永久佔住 link | PARTIAL（潛在） |
| SECS-T07 | `Deselect.req` 未處理，不回 `Deselect.rsp` 且維持 SELECTED，讓 host 走到 T6 | PARTIAL |

---

## 5. Empty / TrayArm 動作 —— 直接回答使用者的兩個問題

### 5.1 問題 A：「TrayArm 放空盤到 Empty 後，會 rear→front 再 front→rear 才被夾走」

## ✅ **確認，而且比你描述的更絕對——沒有任何一條路徑會把放下的盤留在 rear。**

程式路徑（`aEmpty.cpp`）：

1. `DoEmpty` case 100 看到 `bReturnTray` → `DoGoUpTray(0)`、`Task=3000`（`:305-312`）
2. case 3000 呼叫 `DoGoUpTray(1)`；在回收還沒完成時執行 `if(bReturnTray && bTrayXToEmptyFinish==false) return;`（`:445-446`）——**這是個裸 `return`，`Task` 不變**。因為 `Task` 是持久化的 action Tag（`database.cpp:1816` `EmptyModule->DoEmpty(P->Tag)`），下一個 MainProc 掃描會**重新進入 case 3000，並從 idle terminal `GoUpTask==1` 把整個梯形圖再跑一遍**
3. `DoGoUpTray` case 1000 只要 `bRearHasTray` 就無條件轉 2000（`:889-895`），而 2000–7000 就是 `MoveEmptyY(Discharge) → Lean → Push → MoveEmptyY(Feed) → Pop → Pop → bFrontHasTray=true; bRearHasTray=false`（`:897-946`）
4. TrayArm 放盤時呼叫 `NotifyTrayXToEmptyFinish`（`aTrayArm.cpp:1165-1174` → `aEmpty.cpp:1330-1335`），它會設 `bTrayXToEmptyFinish=true` **且 `bRearHasTray=true`**——所以剛放下的盤正好落在一個「rear→front 搬運器」的中途，下一個掃描就被搬到前面
5. case 3000 完成後，case 100 的供料分支（`:382-388` → `DoFeedTray`）又把**同一個盤**拖回 rear

複驗補了一刀：就算運氣好、放盤發生在 GoUp 已過 case 1000 之後，`:455-459` 的 CG-4 守衛（`if(bReturnTray && bRearHasTray){ DoGoUpTray(0); return; }`）還會**重新啟動這趟搬運**。

**實測代價**（`TaskHistory.csv`，複驗逐筆重算過）：

| 區間 | 時間 |
|---|---|
| Empty Task=3000 全程 `16:37:41.413 → 16:37:56.202` | 14.789 s |
| 其中放盤後的 rear→front（TrayArm 於 `16:37:51.008` 結束 place） | **5.194 s** |
| 緊接的 front→rear 重新供料 `16:37:56.212 → 16:37:59.559` | **3.347 s** |
| **可省合計** | **8.541 s / 每個回收盤**，外加 3 × 835 mm 行程與一組多餘的彈匣 restack+destack |

**為什麼機構上會這樣**：Empty 站是**單一雙停位穿梭車**，前停位在彈匣下、後停位在 TrayArm 柱下。**唯一的升降/分離機構在前面**（`C_Empty_FrontRiseTray_1/2` + `C_Empty_FrontSeparateTray_1`，`aEmpty.cpp:656-761` / `:810-887`），後面只有兩個搬運夾爪（`database.h:374-378` 只宣告五個 Empty 汽缸，確無後段堆疊機構）。所以程式把「把盤還給 Empty」定義成「放回供料側（前面）」。

**但複驗修正了一個關鍵歸因**：第一段（把已備妥的前盤推回彈匣，再把後盤搬到前面）是**機構必需的**——前停位必須空出來才能接後面的盤，後面才空得出來接放料。**可省的只有**：(a) park-spin 把剛搬到前面的盤又推回彈匣、(b) 放盤後的那趟搬運、(c) 重新供料，以及它們逼出來的那次多餘 GoDown。

**修復方向（`TEmptyModule::DoEmpty` 內，`aEmpty.cpp:430-464`）**：把「用重跑梯形圖來原地等待」的寫法換成專用等待狀態 `case 3500`，只輪詢、不呼叫 `DoGoUpTray`。三個出口：

- (a) `bTrayXToEmptyFinish` → **原地接受**：呼叫既有 helper `BirthRearTray()`（`aEmpty.cpp:591-594`，**不要**用裸的 `MMEmptyY->InitNewTray()`）、清 `bRearReturnInProgress`、`Status=ES_REAR_READY`、`Task=1`
- (b) `bReturnTray==false`（TrayArm 中途改道，`CancelReturnTray` `:1325-1328`）→ 落回正常完成路徑。**這條出口不可省，否則會死鎖**
- (c) `RunMode==Run_CleanOut` 時**逐字保留舊行為**——那時盤真的必須回到彈匣

⚠ **三個陷阱**（複驗特別點名，直覺解法會出事）：

1. **`c80c7ea` 的不變量必須保住。** 它在 `:446` 的 `return` 讓 `bRearReturnInProgress` **保持 true**。如果 EMPTY-01 的實作是「一看到 `bTrayXToEmptyFinish` 就跳出 case 3000」（也就是繞過 `:447`），這個 latch 永遠不會被清、`GoUpTask` 可能停在非 1 的值——那會**同時**讓 `ComputeRearPickReadyNoRefresh`（`:1234`）和 `IsRearReadyForPlace`（`:1288`）永久為 false，TrayArm 從此既不能從 Empty rear 夾也不能放：**完全餓死，比原本的競態更糟**。改動必須做在 `if(DoGoUpTray(1))` 的 body 裡面、`GoUpTask` 已回到 idle terminal 的位置，並且要走到 `:447`。
2. **CG-4 守衛（`:448-459`）要改寫不是刪除**——它的後置條件從「rear 已清空」變成「rear 剛好只放著那個被放下的盤」，CleanOut 路徑要保留原語意。
3. **`EMPTY-03`：後盤的軟體身分（`MMEmptyY->fHasTray` + grid）目前是靠這趟往返「順便」修好的。** 直接刪掉搬運而不做明確的 adopt，會弄壞 `IsCleanOutFinish` 與 Motion View。（真機上 `RefreshStateFromSensors` 會在後段 sensor 的 false→true 邊緣自己 `BirthRearTray()`，所以主要影響 SOFT_SIMULATE/DUMMY——但不要宣稱真機上不需要。）

⚠ **排程建議**：`c80c7ea` 是**當天早上 10:20 才為了 alarm 600（`C_Empty_PushTray` 撞料）加固過的路徑**，而模擬跑不出來（`IsCarrierParked` sim 恆真、`IsRearReadyForPlace` sim 被繞過、夾爪確認 sim 被短路）。這是**上機驗證型**的優化，不要混進 07-31 的修復批次。

同時，複驗**推翻**了 EMPTY-02 的標題主張（「移掉這趟就消滅整個失效類別」）：`DoFeedTray` case 4000 每一次供料都會帶著夾住的盤橫越 rear（`:509-570`）、第一段回收也跑同樣的 2000-7000、CleanOut 仍跑完整搬運。`c80c7ea` 是這個類別的**正確結構性修法**，不是「紙糊」，不可以那樣描述。

### 5.2 問題 B：「TrayArm 每次都從 Loader 夾起空盤放到 Empty，才放到 Auto」

## ✅ **確認——但這是設定造成的，不是程式沒有直達路徑。**

直達的 Loader→Auto 路徑**存在，而且有兩處**，但都被同一組旗標關掉：

```
aTrayArm.cpp:1007-1008
bool bMaySupplyAuto = (GeneralSetting.bUseAMR==false)
                   || (GeneralSetting.bUseAmrRecoveryDivert && iDeliverKind==eTrayKindNormal);

aTrayArm.cpp:1067-1068   // 飛行中改道
if(GeneralSetting.bUseAMR && GeneralSetting.bUseAmrRecoveryDivert==false) return false;
```

現場設定（六份 `FeederDecision.txt` 全部一致，`General.ini:3` / `:50`）：`UseAMR=1`、`UseAmrRecoveryDivert=0` → `bMaySupplyAuto = false || (false && x) = false` → 落到 `:1036-1044`：`PlaceDest=TAPLACE_EMPTY; iAutoTarget=-1; EmptyModule->RequestReturnTray();`。

`PlaceDest=1` 的解碼：`aTrayArm.h:33-35` `TAPLACE_AUTO=0 / TAPLACE_EMPTY=1 / TAPLACE_COLOR=2` → **1 = 回收到 Empty rear**，與現場快照吻合。

**但你的敘述有兩個例外要記住**：

- **`bUseAMR=0`（一般生產模式）時 `bMaySupplyAuto` 為真**，只要有 Auto 在要盤，手臂就直接供給，不會繞 Empty。
- **回收到的「身分盤」根本不去 Empty**——它會被送去 Color 做 2D 掃描（`aTrayArm.cpp:988-1000`）。

**修復（設定即可，不需重編譯）**：開啟 `UseAmrRecoveryDivert`。但要誠實說三件事：

1. **不要在程式執行中手改 `General.ini`**——`GeneralSetting.cpp:301` 存檔時會覆寫回去，`:203` 只在載入時讀。用維護畫面既有的 checkbox（`maintenance.dfm:1162` `chkUseAmrRecoveryDivert` → `maintenance.cpp:2262-2268`）。
2. **這會是這個功能的第一次上機**。`docs/plan/amr-recovery-divert-plan-20260719.md` §6 的上機驗證項（堆疊順序 identity→cover→normal、車上盤數/Report6 SECS 帳、無多餘 Empty GoUp、生產中 HOME 續產且手上有盤、快速 flag-OFF 回退）**一項都還沒執行過**。在京元量產爬坡期打開它是一次試驗，不是安全的設定微調——要同時把 flag-OFF 回退步驟交給操作員。
3. **它取代不了 §5.1**。改道是**機會性**的：`FindTrayRequestAuto`（`aAuto1To6.cpp:1266-1303`）只會挑「車上已堆好 identity+cover、rear 空且無 pending、未被 AMR 鎖、不在 CleanOut 排空、出料車未滿、**且工作車上沒有盤**（`:1243-1244`）」的 Auto。條件不成立時盤仍然停 Empty，§5.1 的往返照樣發生。

### 5.3 EMPTY-05：抄捷徑前必須尊重的干涉

單一 TrayArm 在整段 Empty 回收期間被綁住（實測 14.75 s），而回收的前段抬升會透過共用的 front-separate 互鎖與 Loader 的 destacker 串行化。

---

## 6. 顯示問題（筆記 5、6、7、8）

### 6.1 【DISP-02】UPH：分子跨電源、分母每次開機重設　嚴重度：高（原評 blocker，複驗降級）

```
csystem.cpp:1968-1976  GetCalculateUPH()
tElapsed = tEndTime - tRunData.StartTime - tUPH_PauseTime;
return (int)((double)tRunData.TotalIC * 3600.0 / dSeconds);
```

- 分子 `tRunData.TotalIC`：開機時從磁碟還原（`main.cpp:3907-3909`，註解明寫「cumulative production stats and are KEPT even on a fresh start (user choice)」）
- 分母的 epoch `tRunData.StartTime`：在**每個 session 的第一個生產週期**被無條件重新打上 `Now()`（`csystem.cpp:1760-1765` 的 `bFirstRun` 區塊），而**同一個區塊不碰 `TotalIC`**

線上證據（S6F11 report 1，SVID 1021/66020/66021）：

```
14:43:02.905  UPH=5131  TotalIC=317  TotalSorted=4
14:51:08.773  UPH=4801  TotalIC=317  TotalSorted=4
15:03:31.245  UPH=4495  TotalIC=317  TotalSorted=4
15:07:57.326  UPH=2313  TotalIC=329  TotalSorted=15
15:09:54.349  UPH=2305  TotalIC=330  TotalSorted=15
```

**只放了 13 顆 IC，UPH 掉了 55%** ——分子凍結、分母增長的典型曲線。`TotalSorted=4`（session-only，不持久化）對上 `TotalIC=317`，那 313 的落差就是陳舊的結轉。

`lastdata.dat`（4120 bytes 純 struct blit，複驗自行從 `cprod.h` 推導 offset 驗證過）決定性一筆：`16_17_31` 的 `StartTime` 已重打成 `07-31 14:36:33`，而 `TotalIC` 仍從 07-30 的批次結束扛著 316 過來。

算術（計算機驗證）：`16:29:29` 那筆 Lot End 報 UPH=2026，`347*3600/2026 = 616.6 s` 生產時間對上 717 s 牆鐘；但該區間只放了 `347-330 = 17` 顆，真值是 `17*3600/616.6 = 99` UPH——**高估 20.4 倍**。`14:43:02` 那筆更誇張：1 顆 IC，真值約 16，回報 5131。

**客戶確實在消費這個數字**：`15:02:49.811` host 定義 `RPTID 508 = {1021,1028,1009,1023,1024,1025}`，`15:02:50.652` 把 CEID 1 綁上含 508 的 12 張報表。

**修復（複驗修正後為三個必要點）**

1. **`cmydef.cpp:46-52` 新增 `int g_iUphBaseIC = 0;`**（緊鄰 `bFirstRun`/`tUPH_PauseTime`），`cmydef.h:66` 附近加 `extern`。
   ⚠ finder 原提的 `static int` 在 `csystem.cpp` 檔案範圍是 internal linkage，而它自己又要求從 `cprod.cpp` 設定它——**那樣會連結錯誤**。
2. 在 `csystem.cpp:1760-1765` 的 **同一個 `bFirstRun` 區塊**內快照 `g_iUphBaseIC = tRunData.TotalIC;`。放這裡會自動涵蓋 `csystem.cpp:1869` 那個潛在的第二觸發點（`Run_TrayFeed` 結束時也設 `bFirstRun=true` 卻不歸零 `TotalIC`，目前因 `CheckAllTrayFeedFinish()` 恆回 false 而不可達）。
3. `GetCalculateUPH()`（`csystem.cpp:1971-1975`）分子改為 `(tRunData.TotalIC - g_iUphBaseIC)`，下限箝位到 0。
4. **`main.cpp:966` 的暖機守衛必須一起改**：`if(iUphMinN>0 && tRunData.TotalIC<iUphMinN)` → 用差值。維持原樣的話，這個防尖峰守衛會被陳舊的累積值永久滿足——也就是在它本來要防的情境下失效（現場 `General.ini [UPH] MinSampleIC=0`，317 輕鬆越過）。**這條是必要的，不是選配。**

不需要改的：`main.cpp:976` 的「UPH Time」格已經算 `Now()-StartTime-PauseTime`，與分母同構，本次引入的是**分子**基準線，那裡沒有東西要套。`cprod.cpp:135` 是選配（它已經把 `TotalIC=0` 且 `bFirstRun=true`，基準線會自然變 0）。

**不要**用「開機時把 `TotalIC` 歸零」來解——那會靜默推翻 `main.cpp:3907-3909` 記錄在案的使用者決定，並且會把 host 讀的 SVID 66020 一起歸零。

（相鄰但另案：`main.cpp:2910` 的 Lot End 記錄與 `uHGemHT160.cpp:381` 對 66020 的標示「total IC processed this lot/run」也因為同一個缺漏而不實——`16:29:29` 那批實際只放 17 顆卻記 347。修 UPH 不會修好這個標示語意。）

### 6.2 【DISP-01】Load / Total 兩個面板從來沒有人寫過　嚴重度：高（螢幕單獨看是中，加上 SECS 才是高）

- DFM 面板存在：`main.dfm:1348 palloadingCount` / `:1364 palUnloadingCount`，`Caption = '0'` 是**設計期字面值**
- 全樹對這兩個元件的參照只有 `main.h:199-200` 的宣告，**沒有任何 `->Caption=`**；`git log --all -S "palloadingCount->Caption"` 空
- `tRunData.LoaderIC` 只有三個接觸點：`cprod.h:18` 宣告、`cprod.h:42` 歸零、`cprod.cpp:147` 每批重置——**全樹沒有 `++`**
- 六份現場 `lastdata.dat` 全部 `LoaderIC=0`

HT172 原版（移植來源）：`D:\HT172\HT172_Program_V1.0.25.0_20260420\main.cpp:2438-2439` 在 `ShowBinCount()` 尾端做兩行賦值；分子在 `HT172_Module\aSortArm.cpp:2351` 的 `AddLoadingCount()` 裡 `tRunData.LoaderIC++`。**DFM 搬了，這兩處都沒搬。**

複驗補的一致性檢查很有說服力：現場 `TrayICCnt[1..6] = 99+28+100+41+67+12 = 347 = TotalIC`，**放料側的計數鏈是健康的，缺的就只有取料側的分子**。

**修復**

1. **分子**：在 `TSortArmModule::TransferPickDataFromLoader()`（`aSortArm.cpp:1525`）的 `if(Slot[SlotIndex].bCanPick)` 分支（`:1533`）內、緊鄰既有的 `g_DeviceInfo.AddInputInfo(...)`（`:1537`）加 `tRunData.LoaderIC++;`。複驗查證過**不會重複計數**：三個呼叫點（`:1884` 乾淨吸取、`:1945` K_SKIP、`:1960` K_TRAY_END）互斥且都以 `PickTask=60` 收尾，retry 分支不呼叫，而 `SkipErroredPickCells`（`:1358-1376`）對每個失敗槽跑 `ClearSlot` 使 `bCanPick` 掉落。
2. **渲染**：`ShowUnloadAutoInfo()` 能動但語意不對（那是 Auto1-6 的區塊，Load/Total 在 `Panel7` 下）。改用新的 6 行 `TfMain::ShowBinCntInfo()`，從 `csystem.cpp:266` 同一區塊呼叫；或併進 `ShowProductInfo()`（`main.cpp:932`）。沿用 `main.cpp:1108-1111` 的「值變才寫」慣用法，並用顯式 `IntToStr()`。
3. **順序有依賴**：`TotalIC` 目前跨批跨電源，**先接渲染會比現在的誠實 `0` 更糟**（開機後會拿陳舊值冒充本批）。分子（步驟 1）沒有這個依賴，可以先上；渲染要等 §6.4 的批次歸零。
4. 同一列還有：`lblloseCnt`/`palloseCnt`（Fail）`Visible=False`、`sbPaperSummary`（Summary）**無 OnClick 也無 handler**。四個控制項死了三個（`btnClearCount` 是活的，`main.cpp:899`）。

**要推翻的既有結論**：`docs/plan/counter-audit-cleanup-plan-20260714.md:29` 把 `LoaderIC` 列進「死碼清理 TODO」並建議刪除。本次的死 DFM 面板 + host 的 RPTID 501 一起證明它是未完成的移植，應**實作而非刪除**。附帶論證：`cprod.h` 是共用 header，刪欄位會改變 layout 並**靜默作廢**現有的 `lastdata.dat`（raw `sizeof(tRunData)` blit）；實作則零 layout 變更。

### 6.3 【DISP-06 / DISP-03】UPH「3 個數字」的另一種讀法

`DISP-06`：主畫面上**同時有三個定義不同的 UPH 數字**，而其中的「Avg UPH」是專案自己的計畫文件否決過的算術平均。若操作員的「3 number」指的是「三個數字」而不是「三位數」，這才是答案。兩種讀法都已記錄，需要一張截圖才能定案。

`DISP-03`：停在 VCL modal 裡的時間被算成 UPH 分母的生產時間（暫停碼錶只在 `ProcessMotion` 內跑，而 modal 會把它擋住）。這會讓警報多的日子 UPH 偏**低**，是次要成因。

### 6.4 【DISP-04】Lot End 不歸零任何生產計數器　嚴重度：高（未複驗，agent 網路錯誤）

`TfMain::DoLotEndProcess()`（`main.cpp:2874-2985`）清了 lot 資料（`LotRegistry.Clear()` `:2967`、`m_sActiveLot=""` `:2973`、`edLotNo->Text=""` `:2975`），但**從不呼叫 `ResetPerLotProductionCounters()`**（`cprod.cpp:135`）。那個 reset 只有三個呼叫者：`main.cpp:912`（Clear All 按鈕）、`main.cpp:2475`（`LotStartCore`）、`uHGemHT160.cpp:1505`（SECS CLEARCOUNT）。

現場證據：

```
07/30 14:57:27.865  "End of Lot: Lot=SIMU_LOT_A, TotalIC=88"
07/30 14:59:30.987  "End of Lot: Lot=,           TotalIC=88"   <- 兩分鐘後仍是 88
07/30 15:09:24.786  "End of Lot: Lot=SIMU_LOT_A, TotalIC=103"  <- 88 + 15 新的
```

而且兩天的 EventLog 裡**沒有任何一行 `LOT START`**、SECS log 裡**沒有 CEID 5/6**——唯一會歸零的路徑兩天都沒被觸發過。

**修復**：在 `DoLotEndProcess()` 的記錄區塊之後（`TrayUphLog_OnLotEnd` `:2911`、`g_SoterOutput.OnLotEnd()` `:2912`、`FreezeProductInfoAtLotEnd()` `:2913` 之後）、`WriteLastDataIni()`（`:2914`）之前插入 `ResetPerLotProductionCounters();`。

⚠ **這是對京元可見的行為改變，必須先確認**：該函式同時會設 `bFirstRun=true`（`cprod.cpp:152`）並歸零 `MachineRun.iTotalScanned/iTotalSorted/iUnknown2D` 與 `BinICCnt`，也就是 **SVID 66021 與 Auto 的 Cnt 面板在 Lot End 會掉到 0**。這是正確的 per-lot 語意（跟 Lot Start 現在做的一樣），但要先講。

### 6.5 【DISP-05】jam rate 在 HT-160S 根本不存在　嚴重度：中

三層都缺：

- 欄位在：`cprod.h:21 JamCount`、`:25 JamRate`、`:26 JamRateDenom`。`JamCount` 只被賦值 0（`cprod.cpp:148`），`JamRate` 除了 ctor **沒有生產者**，`GetJamRateDenom()`（`cprod.cpp:54-57`）**零呼叫者**
- 顯示沒有：全部 `.dfm` 搜 `Jam` 只有 `maintenance.dfm:618 'Error/Jam'`（塔燈標籤）。HT172 承載這個統計的 `fData` 在 HT-160S 是**空殼**（`data.dfm` 393 bytes 零子元件、`data.cpp` 528 bytes 只有 ctor）
- 兩台參考機的定義**不一樣**：HT172 是 per-IC ppm（`main.cpp:1922`，jams/TotalIC × 10000）；HT9045 是 per-tray 的 MUBF「1 / N unit」（`cObserver.cpp:2326-2356`，分母是 `SendCT[1]` 每餵一盤加一）

**複驗推翻了 finder 的兩個前提**（實作前必須知道）：

- ❌「`ShowJamError()` 零呼叫者 → 14 個 JAM 碼不可達」是**錯的**。JAM 碼今天就在發：現場 `HT160S_2026_07_31.csv` 有 `16:31:04.113 JAM1102 "Auto1 Push Tray Miss"` 與 `16:31:40.145 JAM1202`。它們走 `ShowMyError`（`aAuto1To6.cpp:665`、`aLoader.cpp:1947`、`aEmpty.cpp:524`）直達 `ShowNoteAlarm`。`ShowJamError` 只是個沒人用的便利包裝，**不要動它**。
- ❌「HT172 的字串規則會永遠回報 0」也是**錯的**——07-31 就會是 2。而且 HT9045（客戶熟悉的那台）用的正是同一個字串測試（`note.cpp:277` `CodeBuffer.Pos("JAM")>0`）。

**修復（需先與京元敲定定義）**

1. **分子**：唯一咽喉點是 `note.cpp:791` 的 `ShowNoteAlarm(...)`。兩個要刻意決定的點：(a) `:794-800` 有個 modal-busy 的提前 return，要決定計在它**之前**（機台實際發生的每一次）還是之後（操作員看見的）——建議之前，並寫進註解；(b) 補上 9045 的 dummy gate（`note.cpp:277` gated 在 `iRealDummy==REALLY`），HT-160S 有同一個欄位（已曝露為 SVID 1518）。若走 registry 分類路線，**必須用 `find()` 不能用 `operator[]`**——`MyAlarmCodeStruct()` 預設 ctor 的 `AlarmType=0` 就是 `eJamErr`，查不到的 key 會靜默自我分類成 jam。
2. **公式**：放進 `ShowProductInfo()` 既有的 `if(HSys.Sys.SystemStart && bFirstRun==false && tRunData.TotalIC>0)`（`main.cpp:956`），該守衛已排除零分母。**不要**照抄 HT172 的第二個渲染（`main.cpp:3141` 寫死 `/3000` 而 `JamRateDenom` 是 10000，它自己兩處就打架）。
3. **顯示**：⚠ 直接調高 `sgProductInfo.Height` **無效**——它是 `Align = alLeft`（`main.dfm:2303`），VCL 執行期會用父層 client height 覆寫。要調的是 `Panel1.Height`（`main.dfm:2294`，目前 122，`Align = alBottom`）。
4. **更好的選項**（若京元要的是 per-lot）：9045 自己的 per-lot jam rate 根本不用即時計數器——`RUN_INFO::SaveJamRateByLot`（`D:\HT9045\...\cprod.cpp:703`）是**重新掃事件記錄**、找第 3 欄含 `JAM` 的列。HT-160S 已經在寫等價的每日 EventLog CSV（第 6 欄就是代碼）。這條路**不需要新增計數點、不動 `cprod.h`、不動 DFM 幾何**。

### 6.6 【DISP-07】Lot End 之後批號變空白，且可以無批號重新開始生產

現場真的記錄過 `"End of Lot: Lot="`（空批號），下游資料夾退回 `"NA"`。這是「lot number」相關最有證據的 UI 缺陷，但**不是**筆記第 5 條字面上的「dot」。

---

## 7. 「lot number dot」為什麼結不了案

`SECS-03` 與 `DISP-08` 兩個角度都做了，結論一致：**六種候選機制已排除，兩種留著，證據不足以判定**。

其中一個仍成立的候選是 `SECS-04`：**未供應的 SVID 我們編碼成空 LIST `<L[0]>`，而 HT9045 送的是零長度 ASCII `<A[0] "">`**。host 端若把 `L[0]` 轉字串，很可能印出一個佔位符號。這值得修（也是對齊項），但**不能宣稱它就是那個 dot**。

需要什麼才能結案：見 §10。

---

## 8. State Record 擴充計畫（使用者項目 6）

> ⚠ 本節 15 條發現因 API 網路錯誤未經對抗式複驗。內容為 finder 產出，實作前每條的 file:line 需再確認一次。

### 8.1 現況盤點

| 產出 | 內容 | 寫入者 |
|---|---|---|
| `Snapshot.ini` | 觸發原因/時間/版本 | `cStateRecordHT160.cpp:1024` |
| `TaskHistory.csv` | 各模組 Task 變遷環形記錄 | `:1026` |
| `CurrentTasks.txt` | 七個模組的當下 Task + StuckMs | `:1028` |
| `MachineState.ini` | 系統/Recipe/Lot/Tasks/StuckMs | `:1030` |
| `LotData.json` | Lot registry + 每顆 2D | `:1032`（`:562-698`） |
| `MotionDetail.ini` | **20 軸 cmd/enc/tgt/home/err** + SortArm/TrayArm + Suckers | `:1034`（`:712-798`） |
| `FeederDecision.txt` / `SortArmDecision.txt` | 各模組 `DescribeState()` 傾印 | 模組自身 |
| `EventLog\` `SecsLog\` `MachineConfig\` | 整份複製 | `:944-967` 等 |

**所以「馬達狀態」已經部分有了**——`MotionDetail.ini` 已經傾印 20 軸的 cmd/enc/tgt/home/err。缺的是**放大器/狀態層**。

### 8.2 缺口（每一條都用本週真實查不出來的問題來證成）

| 代號 | 缺口 | 本週因此查不出來的問題 |
|---|---|---|
| **SREC-01** | **汽缸 87% 缺席**：登錄表有 39 個（`database.h:367-417`），只傾印 5 個 out-bit（全是 Empty 模組，`aEmpty.cpp:1384-1393`），**簧片 sensor 一個都沒有**。`IO_Table.csv` 有 39 個 `Cylinder_On` + 39 個 `Cylinder_Off` 沒人讀 | alarm 600 / 40023 這類「汽缸沒到位」無法區分「真的沒動」與「簧片壞了」 |
| **SREC-02** | **sensor 99 個只傾印約 18 個，而且 feeder 那些是 latch 不是實讀**。`aColor.cpp:1983-1987` 原始碼自己寫明「Reads latched members directly (does NOT call RefreshStateFromSensors)」 | 已記錄在 `onsite-0730-kyec-triage-20260731.md:326`（D9-1）：「`FeederDecision.txt` 不傾印 `SnLoader_Inputend` / `SnLoader_InputHasTray`，本次調查關於 `IsSupplySourceDry()` 的每一個結論都是靠排除法推出來的。**這是最大的缺口**」 |
| **SREC-03** | `[Suckers] vac=` 是**恆零的死欄位**：`TMySucker::Status` 全樹只有一個寫入者（ctor 設 false）和一個讀者（這行傾印）。真空 sensor 從未傾印 | 2026-07-23 的「噴嘴到位但建不了真空 → SUC0011」——要分辨「沒真空」與「沒 IC」就差這一個實讀值 |
| **SREC-04** | `err=` 只是**軟極限拒絕旗標**（`bErrorMove` 只在 `CheckSoftLimit` 拒絕目標時為真）。**latched 伺服警報停機時它讀 0**，看起來像「馬達健康」 | 現場 20 軸 `err=0` 而 `SystemStart=0`——快照分不出「操作員按停」和「伺服警報停機」 |
| **SREC-05** | `home` 只有一個 bit，home-done 與 home-in-progress 無法區分；**移動的發起者從不記錄**（`GetLockCount()/GetLockString()` 明明記著哪個函式/哪個 Task 佔用了軸） | 現場 `MSortingArmX cmd=125404 tgt=21009`（差 1.04 m）——分不出是移動中、被拒絕、還是被搶佔的梯形圖丟下的 |
| **SREC-06** | **沒有滾動 IO 變化軌跡**：閃爍或間歇性死掉的 sensor 從一張靜態照片結構上就無法診斷 | 已記錄在 `onsite-0730-kyec-triage-20260731.md:391`（Q9）：`SnLoader_Inputend` 在 14:48-14:50 以 8-40 秒週期跳動而機台沒動 |
| **SREC-07** | 揮發性資料取樣**晚了好幾秒**——motion 傾印排第六，在 `LotData.json`（13.5 KB、曾經會當機）之後 | StuckWatchdog 與手動按鈕觸發時**不會先減速停止**，位置讀值相對觸發瞬間是陳舊的 |
| **SREC-11** | 一個臨時的 access-violation 麵包屑追蹤器（`SR_Trace`，自我標註「REMOVE after fix」）仍在每次快照跑 16 個呼叫點，附加到**快照資料夾外**的無上限檔案 `D:\HT160S_StateRecord\_ldj_trace.txt` | 純粹是成本，分析者永遠看不到 |

### 8.3 執行緒答案（SREC-08）

**只有一個執行緒。** `TRunControl::Execute`（`uruncontrol.cpp:24-49`）只做 `Synchronize(ThreadProcess)`，所以 `MainProc → DoSystem → DoAllProcess → SampleTasks → TriggerSnapshot` 全部在 VCL 主執行緒上。手動按鈕、`HomeResumeDone`、`HomeDrainTimeout`、TrayArm 的兩個觸發點也都是同一條。`uFtpUploadThread.h:16` 明載「Execute() NEVER calls Synchronize / VCL / MOT[] / Sen[]」，`MyComm` 的 RX 走 `Synchronize` 回主執行緒。

⇒ **全 IO 掃描不需要任何鎖，也不可能競態。** 而且成本（§8.4）不到快照在同一條執行緒上已經花掉的 0.1%——快照現在就已經有 `WaitForSingleObject(Pi.hProcess, 60000)`（`cStateRecordHT160.cpp:344`）加約 200 次檔案複製。「快照期間馬達無人監督」是既有狀況，本工作不製造也不惡化它（`aTrayArm.cpp:269-274` 原始碼已經寫明這件事）。

### 8.4 成本預算（SREC-09）

- `IO_Table.csv`：183 個輸入點定義、138 個 `Enable=1`，分佈在 **22 個相異 (Lane,IP,Port) byte**；88 個輸出點全部從 RAM 讀（`Switch.OutValue`），**零卡片流量**
- 138 次讀 ≈ 機台正常穩態 8 個 MainProc cycle 的量（`ScanSystemSenser` 每 cycle 已經發約 16 次，cycle 約 2-4 ms）→ 數十毫秒
- 檔案：`IoDetail.txt` ≈ 15 kB（zip 內約 2 kB）；`MotionDetail.ini` 1,410 → 約 4.5 kB。**不含 trace 時約 +3% 快照體積**

### 8.5 實作計畫

**Slice 1（建議先做，自給自足，一次 `-Clean`）**

1. `cStateRecordHT160.h:78` 附近私有區宣告 `void WriteIoDetailTxt(AnsiString Path);`
2. 在 `cStateRecordHT160.cpp` 實作，**逐字模仿 `WriteCurrentTasksTxt`（`:400-446`）**：`fopen("wb")`、固定寬度 `AnsiString` 列、`fwrite/fflush/fclose`。
   ⚠ **不要用 `TIniFile`**——`WriteString` 是 `WritePrivateProfileString`，每個 key 一次整檔讀改寫；180 個 key 就是 180 次檔案重寫。
   四個區段：
   - `[Cylinders]`：`Idx | Name | En | Out(GetOutBit()) | SnOn | SnOff | OnTmo/OffTmo | OnDly/OffDly | Alm(OnAlarmCode/OffAlarmCode) | Verdict`。**Verdict 在傾印時導出、不新增機台狀態**：`OUT_OK` / `IN_OK` / `UNCONFIRMED`(commanded out 但兩簧片皆 0) / `CONTRADICT`(兩簧片皆 1) / `MISMATCH`。印出警報碼讓操作員報的號碼直接對到列（碼是 `4<idx:03d><err:1d>`，`database.cpp:829,850-851`，例如 `40020` = 第 2 個汽缸 Pop 逾時）
   - `[Sensors]`：`Idx | Name | En | Typ | Addr(Lane/IP/Port/Bit) | Live`。**Live 必須用 `IsOn()`**，理由與 `iosetview.cpp:1587` 改用 `Sensor.IsOn()` 完全相同（`GetStatus()` 系列在 `iRealDummy != REALLY` 時無條件回 true，會畫出一片假綠）。**不要**呼叫任何模組 helper（`IsRearReadyForPick`/`RefreshStateFromSensors`）——`aEmpty.cpp:1377-1382` 已經明文禁止在傾印路徑這樣做，因為它們會寫模組 latch。`TMySensor::IsOn()` 唯一的寫入是 `S->iStatus`，全樹只寫不讀，掃描是惰性的。Pad 按鍵 sensor 走 `fPadInterface->ProcessScanKey`，是純陣列讀取（`uPadInterface.cpp:682-692`），**不會吃掉操作員按鍵**
   - `[Switches]`：40 筆，純 RAM
   - `[Suckers]`：取代現行死掉的 `vac=`，改印 `OnBit | OffBit | Vac(Sensor.IsOn()) | SensorName | Has_SuckIC`
3. 擴充 `WriteMotionDetailIni`（`:712-798`）的 per-motor 值字串（**同一個 key、更長的值**，任何以 `, ` 切分找 `cmd=` 的既有解析器不受影響）：加 `en=`、`led=`（11 字元 0/1，順序依 `MotorAndIO/HTMotor.h:10-21` 的 CW/HOME/CCW/EMG/ALM/SCW/SCCW/SVALM/INPOS/Z/SVON）、`erridx=`、`svalm=`、`spd=`、`pct=`、`slim=`、`hfin=`、`lastHome=`、`locks=`。**`Led[]` 零額外成本**——`csystem.cpp:553-562` 的 `ScanAllMotorStatus()` 每個 MainProc cycle 已經刷新全部 20 軸。把誤導的 `err=` 正名為 `limrej=`（或保留 `err=` 並新增），並修 `:700-711` 與 `:707-708` 的錯誤註解。新增只在 `GetLockCount()>0` 時才寫的 `[MotorLocks]` 區段（正常為零，檔案不會變大）
   ⚠ **不要**呼叫 `M->MoveCheckCallBack()` 去模仿 9045 的 `fCanMove` 欄——那是任意函式指標（`MyMotor.h:163`），從傾印路徑呼叫是副作用風險；只印它是否為 NULL
4. **重排 `TriggerSnapshot`**（SREC-07）：所有揮發性讀取先跑（`Snapshot.ini` → `MotionDetail.ini` → 新的 `IoDetail.txt`），之後才是 `TaskHistory`/`CurrentTasks`/`MachineState`/`LotData`/decision 傾印/設定複製。安全，因為後移的全是 latch 或磁碟資料，而且執行緒被擋住、模組梯形圖不會前進。揮發性檔案各自打上 `hh:nn:ss.zzz` 取樣時間，讓分析者看得出離觸發多遠
5. 更新 `cStateRecordHT160.h:6-12` 過期的產出清單

**建置閘**：刪 `cStateRecordHT160.obj` → `scripts/ops/build-ht160s.ps1 -Clean`。本模組外沒有 header 變動，嚴格說不需要 full build；**但仍要跑真機建置閘**（註解掉 `MachineType.h` 的 `#define SOFT_SIMULATE`、`-Full`、確認 exit 0、還原、重建），因為新程式碼主要是模擬版永遠不會走到的實 IO 讀取。

**Slice 2（`IoTrace`，SREC-06）— 另一個 commit**

建議做，但三個硬性約束：(a) **opt-in**：`General.ini [Diag] IoTraceMs` 預設 0 = 完全關閉；(b) **節流**：從 `SampleTasks()`（`:169-192`，已經是每 MainProc cycle 的 hook）後面掛 `GetTickCount` 閘，預設 50 ms。**逐 cycle 掃描不可接受**——MainProc 每 2-4 ms 一次，138 次 bit 讀會對環網加上約 40k reads/s，那是真正的機台控制路徑成本；(c) **byte 批次**：用 `TMyIo::IOInputByte(port)`（`myio.h:35`）讀 22 個相異 port，而不是 138 次 per-bit 呼叫。
⚠ byte 模式必須**手動重新套用每點極性**（`TMySensor::IsOn` 除非 `Type==1` 否則反相），且**必須排除 pad 按鍵 sensor**（它們根本不碰卡片）。儲存用固定 static ring（`struct TIoEdge { DWORD Tick; short Idx; char Kind; char Level; }`，2000 筆 = 16 kB），**無 heap 無 STL**。只記錄變遷。

**Slice 3（`mycylin.h` accessor + `dwLastCmdTick`，SREC-10）— 按需**

要印「這個汽缸停在 Task=50 且 watchdog 已 arm」需要三個目前是 private 的成員。`mycylin.h` 透過 `database.h` 觸及全樹 → **full build**。⚠ `mycylin.cpp` 第 1 行有 7 個 non-ASCII (Big5) byte，**必須 byte-safe 編修，不能用 Edit 工具**。Slice 1 的 Verdict 欄已經涵蓋現場問過的問題，所以這一片可以等。

**順帶清掉**：`SR_Trace`（SREC-11）——刪掉或用 `General.ini` 旗標關掉，否則每加一個 writer 就會再邀請一個永久麵包屑。

### 8.6 檔案編碼

`cStateRecordHT160.cpp` 與 `.h` **零 non-ASCII byte**，Edit 工具安全。`aEmpty.cpp`（第 1 行）、`mycylin.cpp`（第 1 行）含 Big5，必須 byte-safe 編修。

---

## 9. 修復順序建議

| # | 項目 | 為何這個順序 | 風險 | 需上機驗證 | 外部阻塞 |
|---|---|---|---|---|---|
| 1 | **SECS-T03** passive log 洗版（(a)+(c)） | 現場筆記第 1 條；純 log，零機台行為；而且它讓後續每一次現場取證都變得可用 | 極低 | 否 | 無 |
| 2 | **SECS-01/02** SVID 1011 + 1006 | 筆記第 3、4 條；資料現成、9045 有明確前例；直接補上 host 綁在 22 個 CEID 上的報表 | 低 | 否（SECS 模擬器可驗） | 大小寫需京元確認 |
| 3 | **DISP-02** UPH 基準線（含 `main.cpp:966` 守衛） | 筆記第 8 條；客戶有訂閱（RPTID 508），目前高估 20 倍 | 低 | 否 | 無 |
| 4 | **DISP-01 步驟 1** `LoaderIC++` | 筆記第 7 條的分子；同時餵螢幕與 host 的 RPTID 501；**無依賴，可先上** | 低 | 是（計數正確性） | 無 |
| 5 | **SREC Slice 1** IO/汽缸/sensor/馬達傾印 | 讓之後每一次現場分析都變快；本身不改機台行為 | 低 | 是（實 IO 讀取，sim 走不到） | 無 |
| 6 | **SECS-T02** `StopCommunication()` graceful Separate | 一天丟 241 筆訊息；修法收斂在 transport + 關機路徑 | 中（冪等性是關鍵，見 §3.3-3） | 是 | 無 |
| 7 | **RPTID 502 其餘格** 1007 / 1501 / 3 | 補完 host 最高價值的那張報表 | 低 | 否 | 1517 的值域對照需京元 |
| 8 | **DISP-04** Lot End 歸零 + **DISP-01 步驟 2** 渲染 | 兩者綁在一起：先歸零才敢顯示 Total | 中 | 是 | **對京元可見的行為改變，需先告知** |
| 9 | **DISP-05** jam rate | 定義未定，不宜先寫碼 | 中 | 是 | **需京元敲定 per-IC vs per-tray** |
| 10 | **EMPTY-01** Empty 往返優化 | 純效率；踩的是當天早上才加固過的路徑，模擬驗不出來 | **高** | **是（必須）** | 無，但要排獨立批次 |
| 11 | **EMPTY-04** 開 `UseAmrRecoveryDivert` | 設定即可，但等於該功能首次上機 | 中 | **是（首次）** | 需備妥 flag-OFF 回退 |
| 12 | **SECS-T01** 30 秒 OffLine 自動斷線 | 9045 前例已被推翻，語意未定 | — | — | **需京元確認語意** |

---

## 10. 未解與待確認

### 對京元的確認清單

1. **「off line remove client」到底指什麼？** 這是五個字的手寫筆記，log 裡沒有對應的故障。表面相似的 9045 功能觸發條件完全不同（comm-establish 卡住，不是 host OFF-LINE），而且是為另一個廠區（`CC_KYEC_LEE` 921 京元邏輯）開的，不是竹南。**寫任何一行程式之前需要一句話確認。**
2. **「lot number dot」的現場畫面或該筆批號原文。** 需要：一張截圖，或當時的批號字串。若批號本身含 `.`，附上該字串即可定案。
3. **SVID 1011 的字串值域。** HT-160S 用大寫（`RUNNING`/`HOMING`），9045 用混合大小寫（`Running`/`Homing`），host 若做字串比對會炸。另外 HT-160S 專屬狀態（`Clean Out`/`Tray Feed`/`One Cycle`/`SAFE DOOR`/`AIR`/`MOTOR OFF`）需要對照；9045 說 `Power Off` 的地方 HT-160S 說 `MOTOR OFF`。
4. **SVID 1517 Start Mode 的值域對照。** 9045 觀測到 12 個值，HT-160S 的 RunMode 無法直接對應。
5. **jam rate 的定義**：HT172 的 per-IC ppm（jams/TotalIC × 10000），還是 HT9045 的 per-tray MUBF「1 / N unit」？兩者算出來的數字完全不同。
6. **RPTID 506 / 800 / 801**：host 綁了卻沒定義，當天的 provision 相對 06-08 基準不完整——是漏送還是刻意？
7. **Lot End 歸零是可見的行為改變**（SVID 66021 與 Auto 的 Cnt 面板會在 Lot End 掉到 0）。上線前告知。

### 內部待決

8. **`docs/plan/counter-audit-cleanup-plan-20260714.md` 的兩條建議要推翻**：`:29`（`LoaderIC` 刪除）與 `:31/:35`（`JamCount`/`JamRate` 刪除）。本次證據顯示它們是**未完成的移植**不是死碼，且刪除會改變 `cprod.h` layout 並靜默作廢現有的 `lastdata.dat`。需與該文作者確認。
9. **`SECS-04`：未供應的 SVID 應該回 `<L[0]>` 還是 9045 的 `<A[0] "">`？** 這是對齊項，也是「lot number dot」尚存的候選之一。
10. **07-31 當天 KYEC 的連線至少斷了連續 14 小時 18 分**（計數器 `#351`→`#1195` 無斷點，而且午夜前已經跑到 #351）。這件事本身是否符合預期，本次分析沒有回答，不應被埋掉。

---

## 11. 已排除（不要重新調查）

| 代號 | 主張 | 為什麼被推翻 |
|---|---|---|
| SECS-16 | 「host 當天唯一一次 EC 寫入被 machine-idle gate 擋掉」 | **REFUTED** |
| SECS-T01 | 「HT9045 KYEC 在 OFF-LINE 後 30 秒會斷線，照抄即可」 | 觸發條件是 `bConnect`（E30 communicating），不是 control state；9045 自己的 S1F15 handler 一樣什麼都不做；且 gated 在 `CC_KYEC_LEE`（邏輯廠）非竹南 |
| SECS-T01 | 「OFF-LINE 時不該再發 S6F11，應該加 gate」 | 9045 唯一的 offline 不上報 gate 是 `CC_TFME_CHINA` 專屬，**不是 KYEC**。HT-160S 目前已與 9045-for-KYEC 對齊，**不要去改** |
| EMPTY-02 | 「移掉第二趟搬運就消滅整個撞料失效類別」 | `DoFeedTray` case 4000 每次供料都帶盤橫越 rear、第一段回收跑同樣的 cases、CleanOut 仍跑完整搬運。`c80c7ea` 是正確的結構性修法，不是紙糊 |
| DISP-05 | 「`ShowJamError()` 零呼叫者 → 14 個 JAM 碼不可達」 | JAM1102/JAM1202 在 07-31 16:31 實際發生過，走 `ShowMyError` 直達 |
| DISP-05 | 「HT172 的 `AnsiPos('J')` 規則在 HT-160S 會永遠回 0」 | 07-31 會是 2；且 HT9045 用的正是同一個字串測試 |
| SECS-T03 | 「log 是 17×4KB ring，真實內容被沖掉」 | 是**每小時一檔、純附加、無上限**，沒有任何內容遺失；代價是訊噪比與體積 |
| DISP-01 | 「渲染 Total 就能解決筆記第 7 條」 | `TotalIC` 跨批跨電源，先接渲染會**比現在的誠實 0 更糟** |

---

## 附錄：證據檔案

| 檔案 | 內容 |
|---|---|
| `docs/plan/onsite-0731-kyec-secs/host_reports_S2F33.csv` | host 的 33 張報表定義（RPTID → SVID 清單） |
| `docs/plan/onsite-0731-kyec-secs/host_links_S2F35.csv` | host 的 31 條事件綁定（CEID → RPTID 清單） |
| `docs/plan/onsite-0731-kyec-secs/host_svids.txt` | 389 個相異 SVID |
| `docs/plan/onsite-0731-kyec-secs/s1f3_polls.json` | 8 次 S1F3 詢問與我方回覆 |
| `docs/plan/onsite-0731-kyec-secs/secs_message_samples.txt` | 26 種訊息各一則樣本 |
| `docs/plan/onsite-0731-kyec-secs/parse_secs.py` | 可重跑的 SML 解析器 |

---

## 12. 2026-08-01 追加查證（4 題，66 個 agent 全數複驗）

### 12.1 S1F15 / S1F17 能不能拒絕？（9045 查證）

| | 9045 能拒絕？ | 機制 |
|---|---|---|
| **S1F15 OFF-LINE** | **不能，永遠回 0** | 全樹只有一個 S1F16 實作，acknowledge byte 是**字面 0 且從未被重新賦值**。沒有 checkbox、沒有 ini key、沒有密碼閘、沒有 `CUSTOMER_CODE` #ifdef 碰它 |
| **S1F17 ON-LINE** | **可以** | `uHGemClass.cpp:426-442`：`if(bOnLine) Command=2; else if(GemCheckBoxAcceptHostOnlineRequest->Checked){...Command=0;} else Command=1;` |

9045 的那個 checkbox：`GemCheckBoxAcceptHostOnlineRequest`，Caption `'Accept OnLine Req'`（`SECSGEM\uHGemEquipment.dfm:291-297`，TabSheet3 'Normal'），由 `TFSECS::GemSBSetupClick` → `HGem->ShowModal()` 開啟，無密碼閘。持久化於 `SYSTEM\secs_gem.ini [GEM] AcceptHostOnlineRequest`，**預設 true**（出廠即「總是接受」）。

**但 ONLACK 不是「我現在不接受命令」**：它是**儲存的操作員偏好**，不是任何即時機台狀態。平台為「機台條件拒絕」保留的程式化掛鉤 `THGem::SetCanAcceptHostOnLineRequest()`（`uHGemEquipment.cpp:5825-5827`）是**空函式**、零呼叫者。

**更關鍵**：9045 上 GEM control state 對任何 KYEC 廠區**不 gate 任何 host 命令**——S2F41 RCMD 在 off-line 與 on-line 下被同等執行。唯一的 control-state gate 是 `CC_TFME_CHINA` 專屬。HT160S 的 `iControlState` 也是 write-only（唯一讀者是 SVID 66002 註冊），**這半邊本來就已經對齊**。

**建議：兩個拒絕都不要做。**
- OFLACK 拒絕：沒有合法拒絕碼（⚠ 此點為 DERIVED，未讀 SEMI E5 原文，對客戶引用前需查證）；會拿掉 OFF-LINE 目前唯一有用的效果（釋放塔燈/蜂鳴器的 host latch），那是操作員的 SECS 側逃生口；且規格書已對客戶承諾無條件回 0。
- ONLACK 拒絕：**更危險**。07-31 線上 S1F17 是 host provisioning 爆發的**第一個訊息**，拒絕它會中止整批報表定義，SVID 覆蓋率會比現在的 6.9% 更低。

**真正的缺口是「HT160S 的 OFF-LINE 是裝飾性的」**——要修是讓 OFF-LINE **做事**，不是拒絕它。做什麼仍需 §10 第 1 項的客戶確認。

### 12.2 SECS 閒置紀錄政策（已改，commit 0ade92d）

9045 在 passive listening 無 host 時**輸出零行 log**（`DoOpenCommuncation` passive 分支 `uHGemEquipment.cpp:3548-3598` 零 `StringOut`，該區域無 `CUSTOMER_CODE` guard，非廠區專屬），KYEC 整日現場 log 亦印證。HT160S 七條斷線/連線路徑**每一條都已有邊緣行**，週期行對任何一條都不是必要的。

已移除 30 分鐘心跳。保留**每日每 process run 一行**標記，理由單一且具體：`FlushSecsLogToFile` 對空 buffer early-out，所以全日無 SECS 流量的日子**根本不會建立當日資料夾**，`CaptureSecsLog` 於是在 State Record 裡**完全不放 SecsLog 資料夾**——與「SECS logging 壞了」無法區分。07-31 00:00-14:19 正是此例。

**代價（已寫進原始碼註解）**：斷線邊緣落在**前一天**的中斷，無法再單靠此檔測量時長。同一 session 內起訖的中斷仍精確（兩個邊緣時間戳相減）。**此項刻意推翻 §3.1(c) 標為「必做」的低頻心跳**，取捨已記錄。

### 12.3 DoAuto 拆成六個 per-Auto action？—— 不建議

**Loader 的類比不成立。** Loader 是真正的雙線道到底：`DoLoader(int LoaderNo, int &Task)`（`aLoader.h:149`）吃明確的線道號，每一條子梯形圖都住在 `TLoaderSideState Side[2]`（`aLoader.h:23-41,:59`），擁有 per-side 的 FeedTask/CcdTask/DischargeTask/DestackTask 與 per-side HTimer。

**Auto 沒有等價物**：`TAutoStationState`（`aAuto1To6.h:34-44`）**完全沒有 task 成員**；模組持有的是**各一份** FeedTask、DischargeTask、CleanOutTask、DischargeSubTask、iFeedAuto、iDischargeAuto、FeedDelay、DischargeDelay（`aAuto1To6.h:50-57,:80-82`）。

`DoAuto()` 既不 round-robin 也不每次迭代六台：它是嚴格線性的相位梯形圖 `1→100→1000→(2000 feed)→3000→(4000 discharge)→1`（`aAuto1To6.cpp:1666-1753`），**feed 與 discharge 在時間上互斥**，每個相位由 first-match 掃描（`FindFeedAuto` / `FindDischargeAuto`）挑**恰好一台**。所以那個單一 Tag **確實序列化了六台**——而那個序列化正是目前讓共用游標安全的唯一原因。

**只拆 Tag 不拆游標 = 注入 bug**：兩個並行 feed 會在 `DoFeedTray` case 7000（`aAuto1To6.cpp:713-723`，蓋 tray Kind + 2D TrayID + `Car[].CarID`）互相蓋錯 AMR 出料車身分與 SECS DeviceCount——在「擁有出料站的模組」上造成靜默資料損毀。

其他反對理由：真正的節流點是**單一 TrayArm 與單一 SortArm**（六台 Auto 硬體本就獨立，但供應者只有一組）；而且拆完 State Record 會變成**六份灌爆的 task history 取代一份**，診斷變差不是變好。

**已改做真正划算的那一手**（commit `0ade92d`）：把 Auto 的共用游標與 per-station `TrayReq=` / `CarTrays=` 傾印出來。

### 12.4 「TrayArm 夾了空盤、Auto 缺盤卻沒被叫去放」—— 不是 task 不及時

**H1（延遲）在結構上就不成立，H2（設定 + 優先權）成立。** 三個獨立量測：

1. **當下根本沒有需求**：16:38:37 那格六台 Auto **全部 `CarHasTray=1`**，被 `aAuto1To6.cpp:1243` 擋掉，其餘七道閘全過。沒有任何一台在要盤。
2. **就算有需求也沒用**：現場 `UseAMR=1` + `UseAmrRecoveryDivert=0` 讓 `bMaySupplyAuto` 恆 false（`aTrayArm.cpp:1007-1008`），目的地在 `DecidePlaceDestAfterPick` 被**無條件**寫成 `TAPLACE_EMPTY`（`:1038`）——`FindTrayRequestAuto` **根本不會被呼叫**。飛行中改道也被同一組旗標在 `:1067-1068` 直接 return false。
3. **派工延遲是毫秒級**：Auto 梯形圖平均 **4.07 ms** 換一次 Task（最大間隔 19 ms）；TrayArm 十趟工作有八趟在閒置後 **≤14 ms** 就派到工作。決定性反證：`16:38:08.912→16:38:20.346` 有 **11.434 秒** TrayArm 完全閒置、Empty rear 有盤、`DecideJob` 約被重算 **2,789 次**卻一個工作都派不出來——真有 Auto 要盤，這 11.4 秒早就抓到了。

**而且需求是單調的**：`GetTrayRequest` 不由任何 task step「舉起」，它是**純狀態函式**；需求一旦升起就不會自己消失（只能被送盤消掉，或被 AMR 鎖／車滿 sensor／CleanOut 關掉）。這在結構上否定了「需求在兩次決策點之間閃現又被錯過」。

**但現象描述是對的，機制是優先權排隊**：`DecideJob` 的 Priority 1（`aTrayArm.cpp:565`）永遠先做 Loader 回收，且**沒有任何飢餓保護**。一趟 Loader 回收獨佔手臂實測 **20.937 s / 20.734 s**，一趟供料只要 **9.4-11.6 s**。所以「手臂拿著空盤、Auto 缺盤、卻不送過去」在現場看起來就是不及時，程式上卻是被設定鎖死的路徑。

**順帶查到（非本題主線）**：`General.ini [AMR] CoverTray3..8 / IdentityTray3..8` 的 per-Auto 設定**不會**影響 Auto 的堆疊需求——`GetNextTrayKindForAuto` 是硬編的 identity→cover→normal。

**待確認**：使用者實際觀察的時間點。乾淨的生產窗口只有 `16:35:47-16:38:33`；若指的是 14:36-14:51，那段被 40013/40023/40033 三個 TrayArm 汽缸警報主導，節拍不可用、結論可能不同。

**待人裁決**：`DecideJob` Priority 1 要不要讓路。`:565` 的原始理由是「先清 Loader rear 讓 Loader 能繼續進料」，改優先權會反向壓 Loader 吞吐。**建議先開 `UseAmrRecoveryDivert` 觀察一輪，不要同時動優先權**，否則兩個變數混在一起無法歸因。
