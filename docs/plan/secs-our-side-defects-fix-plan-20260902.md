# 我方缺陷修復計畫 — 20260902 京元 Auto3 AMR 逾時案

- 建立 / 更新：2026-09-02（已納入 forensics workflow `wf_6b0c1069-270` 5 agents 攻防複驗結果）
- 來源事件：`WAR0962 AGV/AMR handshake timeout - AGV did not respond : Auto3 (P6)` @ 2026/09/02 17:32:08
- 證據：`D:\HT160S_StateRecord\2026-09-02 17_33_20\`（機台側）
  + `D:\HT160S_StateRecord\EAP.Main.GEM.Log-20260902.log`（客戶側，只涵蓋 00:00–15:50:22）
- 相關記憶：`auto3-amr-timeout-20260902`、`amr-20260902-rulings`、`secs-startagv-no-reply-causes`

---

## 裁定摘要

**根因（HIGH 信心，三個檢查點各自獨立複驗）**
機台在 `17:23:55.610` 正確發出 `S6F11 CEID 272`（len=406、`(sent)`），`SVID 38219` 讀
`P6:1` = Auto3 叫車，客戶 EAP 在 **57 ms** 後回了 `S6F12 ACKC6=0x00`。
客戶 EAP 此後從未送出唯一能推進交握的 `S2F41 RCMD START_AGV` 帶 `AUTO3="Action"` —
**因為它的 AMR 排程器完全沒有 Auto 站的派車路徑**（全天 20 筆 `START_AGV` 皆為
`Loader="Action"` + 每個 `AUTOn="NA"`）。P6 停在 `AGV_CALLED` 直到我方 300 秒看門狗跳 `WAR0962`。

**但責任分配只有 MEDIUM 信心，原因見下面三個「不可對外主張」。**

---

## 🚫 三個絕對不可寫進客戶信的主張（會被反打）

### 1. 不可主張「Action 是閘門，你們沒送就是沒照規格」
**我方自己交付的文件說相反的話。**
`docs\AGV\HT160S_E87_AGV_Communication_Draft_20260527.md` §14 原文：

> `Action` 這個 CP 值目前為裝飾性(設備端不解析其內容)
> （建議選項 B）`Action` 不解析、不驗證,保留供未來擴充
> **第 4.1 節表格中的 `Action` 欄位語意以本節為準**

§14 明文**推翻**了 §4.1 那張要求 `AUTO3=Action` 的表格，並告訴整合者「路由只看 cpName」。
照 §14 開發的 EAP 工程師**從來沒被告知 Action 是閘門**。
→ 這是全案最大的公平性問題。信裡必須以「規格文件不一致，我方已確認並將修正」的姿態陳述。

### 2. 不可主張「你們超過 300 秒沒回應」
300 秒這個值與 `WAR0962` 的升級，**在任何交付給客戶的文件裡都不存在**
（exhaustive grep：只出現在內部 `docs\plan\amr-unmanned-alarm-reflow-plan-20260721.md:52/:72`
與原始碼 `GeneralSetting.cpp` 預設 300、`General.ini:109 AgvTimeoutSec=300`、
`uAgvStation.cpp:696-706`、`csystem.cpp:183`）。
更糟：**權威樹 HT9046LS 810_B01 在 Auto 路徑上根本沒有逾時** —
發出 AGVSupplement 後進 `Task=1380`，只要 `bUnLoaderActionFlag[Pos]` 還在就
`fMain->Pause()` 並記 "AMR Full Tray Wait for AGV"，**無限等待、不跳警報**
（`asendic_Auto.cpp:1702-1710`）。9045 的 `WAR0962` 只用在 **Loader 供料**路徑，且等 **600 秒**。
→ 300 秒逾時 + B 類停機警報是 **HT160S 自己發明的**。

### 3. 不可主張「你們不回應 Linktest」
客戶自己的 EAP log：`Receive Control Message :Linktest_req` **3501** 次 /
`Sent Control Message :Linktest_rsp` **3501** 次 = **100% 回覆**。
（forensics agent 的 rank-3 條目主張客戶不回 Linktest — 那是因為它只拿到機台側資料，
沒有客戶 EAP log。以客戶 log 為準。）
正確說法：我方 211 次 Linktest 未在 **6 秒**內收到回覆；客戶已收到的 3501 筆全數回覆
→ 那 211 筆是在傳輸層或 EAP 訊息泵阻塞時遺失。**我方 `T6Timeout=6` 也過短。**

---

## ✅ 已釘死、不再是疑問的事

| 原疑問 | 裁定 |
|---|---|
| W7 `SVID 2001` 第 7 元素 = `10` 是哪一站？ | **是 `SVID 38228` = AMR **Loader** Device Count（index [0]）**，不是 Auto3。客戶 `LoaderICCount` CP 被永久 latch。**已由 `5047c08` 修掉，但現場韌體還沒有這個修正。** |
| W4 `P6(83s)` 與 300 秒是否矛盾？ | **不矛盾，不是 bug。** 83 = `LinkLostAge[5]`，P6 在 HSMS **斷線期間**持鎖的 tick 數（`17:25:45.608`→`17:27:08.611` = 83.003s，`uAgvStation.cpp:278-279`）。它屬於 `WAR0963` 的逃生計數（上限也是 300s），**不是** 300 秒交握窗的一部分。 |
| 逾時窗公不公平？ | **公平且寬鬆。** 客戶實得 **493 秒**牆鐘時間對 300 秒設定。斷線期間 `ServiceHandshake` early-return（:620），`ServiceLinkLostHold` 把 `ShortageDebounce` 歸零（:250），無雙重計數。 |
| `CEID 273` 發 20 次 / `274` 只 7 次，有 13 次交接沒完成？ | **沒有缺漏。** 20 筆 `273` 與 20 筆 `START_AGV` **1:1 對應**（每次重送 `BeginPrep` 把站打回 PREP，下一 tick 重發 273），7 個交接群組**每一個都以 274 結尾**。且**全天每一筆 273/274 都是 P1 Loader 位元 — 今天沒有任何 Auto 站到過 273/274。** |
| 16:23:34 `P5`（Auto2）為何沒跳 WAR0962？ | **機台自己撤回了叫車**（sensor 轉 off → `uAgvStation.cpp:540-545` 靜默回 IDLE）。不是斷線運氣。證據：`16:24:11.610` 那次斷線的 EventLog **沒有** "link lost while P5 holds an AMR lock" 那行（P6 在自己斷線後 1.0 秒就有）。 |
| 「logs 16/17 沒有 S2F41」 | **錯。** `_16.txt` 90 行、`_17.txt` 48 行。客戶確實有送 S2F41，只是**從來不是 START_AGV**。`15:27:11` 之後的 RCMD 全清單：`PP_SIGNALTOWER` ×31、`PP_MUSIC` ×31、`START` ×2（其中一筆 HCACK=2）、`LOTSTART` ×1。 |
| 11 筆漏 `AUTO6`（不是 8 筆） | **不是缺陷。** `HT160S_SECS_Comm_Examples.md:574-582` 定義 body 為 L[n] 的站/動詞對，未強制九站完整向量；我方 parser 只跑實際存在的對。 |
| `LoaderTrayCount=4` / `LoaderICCount=10` 全天固定 | **不是缺陷。** `LoaderTrayCount` 是 consume-once、只作用於緊接的下一台車，**每台車重送才是正確用法**。`LoaderICCount` 是 host 宣告的 latch 值，機構不消費。 |
| CP 名稱大小寫混用 | **不是缺陷。** 文件明載大小寫不拘（`Comm_Examples.md:582`），韌體兩邊都轉大寫比對（`uHGemHT160.cpp:2762-2763`），全天零 "unknown CP"。 |
| 現場韌體版本 | **⚠ 現場韌體早於今天的原始碼。** `b5a838c`（Loader 不叫車）與 `5047c08`（38228）**都還沒上機**。證據：`FeederDecision.txt:49` = `P1 Loader: lock=0 hs=CALLED`（新分支會強制 IDLE），且全天 10 筆送達的 CEID 272 有 6 筆是 P1。 |

---

## 我方缺陷清單（forensics 複驗後）

### V1【HIGH／會面前必須有答案】Auto AMR 叫車沒有「機台正在生產」閘門
`PollAndCall` 只擋 `bUseAMR`、`AutoModule!=NULL`、HSMS SELECTED、
`RunMode!=Run_Normal && RunMode!=Run_CleanOut`（`uAgvStation.cpp:436-490`）。
**沒有 `SystemStart` 檢查、沒有 `RunGate` 檢查、沒有 lot-open 檢查、沒有「這條流道真的累積了東西」檢查。**
叫車當下機台狀態：`SystemStart=0`、`RunGate=0`（15:33:25 落下，110 分鐘前）、
`ActiveLot=""`、`LotCount=0`、七個模組 CurTask 全停在 1。
我方仍在 `17:23:55` 發 `CEID 272` + `CEID 37`，300 秒後升級成 B 類停機警報 —
**而同一個封包的 `RPTID 502` 第 3 欄我們自己回報 `HALT`。**
物理觸發是真的（`IoDetail.txt:94` `SnAuto3_InputFullTray` Live=1、`FullVerdict=1`），
所以叫車不是誤報；但在一台我們同時宣告 HALT、無 lot 的機器上叫車又停機，**客戶可以合理地釘我們**。

**修法**：`PollAndCall` 的 Auto 分支加生產閘（`SystemStart` / `RunGate` / lot-open 至少其一），
或至少「非生產狀態下只叫車、不升級成警報」。需裁定閘門的確切組合。

### V2【HIGH／這是不能指控客戶逾時的原因】300 秒與 WAR0962 升級完全沒有交付文件
見上面「不可對外主張 #2」。
**修法**（二選一，需裁定）
- **甲**：對齊權威樹 — Auto 路徑取消逾時警報，改為 `Pause()` + 持續提示，無限等待。
- **乙**：保留逾時但（a）寫進交付規格、（b）把預設值拉到 600 秒對齊 9045 的 Loader 值、
  （c）非生產狀態下不升級成 B 類。

### V3【降級為 MEDIUM／已查明真因，見文末「V3 追加調查」】Auto3 叫車封包帶 `TrayCount=0 / DeviceCount=0 / CarrierID=""`

> ⚠ **20260902 追加調查結論**：這個 0 **不是** Lot End 清帳造成的，而是**盤子由人手放上去的**，
> 機台從原理上就無法計數。真正該修的是 V1 的生產閘。詳見本文件最後一節。
現場封包實測（`SECSGEM_TextLog_17.txt` 17:23:55.611 body）：
`RPTID 2001` 位置 6 = `SVID 38227`（Auto3 Tray Count）= `<I4[1] 0>`；
位置 12 = `SVID 38233`（Auto3 Device Count）= `<I4[1] 0>`；
`RPTID 2000` 位置 4 = `SVID 38207`（Auto3 Carrier ID）= `<A[0] "">`。

**因果**：`IsOutputCarFullForAmr()` 在真機上是**純 sensor 讀取**（`aAuto1To6.cpp:1575-1583`
`FullSensor->IsOn()`），與車帳完全解耦。而車帳（`Car[].iTrayCount`、`iAmrDeviceCount[]`、
`Car[].CarID`）被 `InitialFlag()` 冷分支 `Car[Index].Clear(); InitAutoCarStack(Index);`
（`aAuto1To6.cpp:120-121`）清空，該分支由 `InitialAllTask()` 在 Lot End / CleanOut 完成時執行
（`csystem.cpp:1929`、`:1937`）。15:33:25 lot 結束 → 帳歸零，**盤還在 Auto3 上，滿盤 sensor 還 ON**。

**這違反我方自己的契約**（記憶 `amr-updown-ic-count-contract`：下料要提供 per-Auto-car IC 件數）
→ **它剝奪了我們主張「我們問得完整正確」的權利。**

**修法**：Lot End 清帳時不可讓實體堆疊與帳面脫鉤 — 要嘛不清有實體盤的流道，
要嘛叫車前由 sensor/實際堆疊重建帳面。需裁定。

### V4【MEDIUM】被抑制的 CEID 272 仍然上鎖、latch AGV_CALLED、啟動 300 秒看門狗
`THGem::EventReport` 是 void，兩種情況 early-return（未 SELECTED `:436-441`、
CEID 被 host 停用 `:445-450`）。`PollAndCall` **完全不看結果**：`SetAmrLock` → `EventReport(1,272)`
→ 無條件 `Handshake[si]=AGV_CALLED`（`uAgvStation.cpp:523-538`），而 CALL 是 IDLE→CALLED 邊緣的
**一次性**事件，**永不重送**。
→ 叫車可以靜默遺失、我方照樣跳警報。**今天真的發生了**：`14:42:35.615` 一筆 CEID 272 被
`suppressed (CEID disabled by host)` 吃掉。

**修法（使用者已裁定 = B + C）**
- **B** 只讓 AMR 生命線 CEID（**272 / 273 / 274**）豁免 host 的全域 `S2F37` 關閉，其餘照 GEM。
- **C** 被抑制時不再無聲：寫 EventLog + 跳通知（符合記憶 `silent-stop-must-notify`）。
- 併同建議：`EventReport` 改為回傳送出結果，`PollAndCall` 在送不出去時**不要** latch `AGV_CALLED`，
  或加重送機制。

### V5【MEDIUM／客戶可合理反駁，別把它講成純客戶問題】AGV 看門狗每次重連從零重算
`ServiceLinkLostHold` 每個斷線 tick 把九站的 `ShortageDebounce[si]` 歸零
（`uAgvStation.cpp:250`，理由註解在 `:239-242`：計數器量的是「AGV 沒回應」，我方離線時無從得知），
且 `ServiceHandshake` 未 SELECTED 就整個 early-return（`:620`）。
設計本身站得住腳，副作用是**這個警報的可靠度取決於客戶的網路**。

### V6【MEDIUM】`T6Timeout=6` 過短，今日 211 次由我方主動斷線
`uHGemEquipment.cpp:85` 預設 6，`UsecegemMainFrom.cpp:72` 讀 `General.ini`。
`:234-247` 每 1 秒 tick（`Timer1->Interval=1000`，`:37`）遞減，歸零即
`DropConnection("Linktest T6 timeout (peer not responding)")`（`:242`）。

實測我方 `S6F11`→`S6F12` 往返：p50 **0.010s** / p90 **0.301s** / p99 **8.109s** / max **9.132s**，
**1.59% 超過 6 秒**。今日 213 次 drop（211 Linktest + 2 socket error）；431 筆 peer-disconnected
有 426 筆在我方 drop 後 2 秒內 = **98.8% 是我方斷的**；客戶 log 覆蓋窗內我方 188 次 drop 有
**186 次客戶 socket 事前完全正常**。

**修法**（純設定，不改碼、不重編）

    [SECS]
    LinktestInterval=30    ; 10 -> 30
    T6Timeout=30           ; 6 -> 30，對實測最差 9.13s 有 3.3 倍餘裕

**注意：不要寫 `T6Timeout=0`** — `SetT6Timeout()`（`:2109-2112`）夾制
`(Seconds <= 0) ? 6 : Seconds`，寫 0 會靜默變回 6。

### V7【LOW-MEDIUM】`SVID 38220` / `38221` 從不清除，封包帶兩個過期 bitmap
三個 bitmap 只在各自事件前賦值，且只在 `Reset()` 清除，而 `Reset()` **沒有任何 runtime 呼叫者**
（`uAgvStation.cpp:526`/`:662`/`:681`；`Reset` 在 `:166-200`）。
現場封包 `17:23:55.611`：欄 5（38219）= `P6:1` **正確且即時**，
欄 6（38220）與欄 7（38221）**都還是 `P1:1`** — 分別殘留自 `15:27:10.607` 的 Loader CEID 273
與 `15:28:07.615` 的 CEID 274，**過期 116 分鐘**。9045 會清。
客戶的 `RPTID 2000` 把三個 bitmap 並排帶在每個 AMR 事件上，所以這在客戶眼裡很難看。

**修法**：事件送出後清除對應 bitmap，或給 `Reset()` 一個 runtime 呼叫點。

### V8【LOW】撤回叫車完全靜默，沒有任何 CEID 告知 host 請求已取消
`uAgvStation.cpp:540-545`（infeed 孿生在 `:601-605`）在 `bFull` 轉 false 且仍 `AGV_CALLED` 時
釋放鎖、回 `AGV_IDLE`，**不發任何事件**。
今天 Auto2 就是這樣：`16:23:34.608/.614` 發了 `CEID 272 (P5:1)` + `CEID 36`，sensor 轉 off，
我方靜默撤回（快照證據：`SortArmDecision.txt [Auto2] FullVerdict=0 InputFullSn=0 CarTrays=0`、
`FeederDecision.txt:53 P5 AUTO2: lock=0 hs=IDLE`）。
**客戶手上還握著一個我們早已撤銷的活請求** — 若他們真的派了車，車會白跑。
加上 CALL 永不重送，**漏接單一 272 的 host 不會收到任何提醒**。

**修法**：加一個「叫車取消」CEID，或至少重發帶全零 bitmap 的 272。需裁定（新 CEID 要走私有段 9001-9099）。

### V9【LOW】重複 `START_AGV` 把已完成的交接往後推
`BeginPrep` 無條件 `Handshake[i]=AGV_PREP`（`uAgvStation.cpp:811`），
所以每次重送都把站打回 PREP、下一 tick 重發 273，延後 Finish。
14:53 那串 7 筆造成 7 個 273 對 1 次實體交接，Finish 直到 14:54:16 才來（比最後一筆晚 57 秒）。
**修法**：對已在 PREP 或 READY 的站，重複請求改為忽略並記一行 log，不重啟。

### V10【已結案】`LoaderICCount` 恆定值
依 `amr-20260902-rulings` 第 1 項（SHIPPED `5047c08`）已不再寫任何 SVID。
**但現場韌體還沒有這個修正**，所以本次 log 仍看得到 `38228=10`。

---

## 執行順序

| # | 項目 | 類型 | 需裁定 |
|---|---|---|---|
| 1 | **修正 E87 §14** 的「Action 裝飾性」錯誤敘述，並與 §4.1 對齊 | 文件 | 否（必修） |
| 2 | V6 `T6Timeout=30` / `LinktestInterval=30` | 設定 | 否（數值可調） |
| 3 | V4 B+C：272/273/274 豁免 `S2F37` 全域關閉 + 抑制時通知 | 改碼 | **已裁定 B+C** |
| 4 | V3 Lot End 清帳與實體堆疊脫鉤 | 改碼 | **是**（清帳策略） |
| 5 | V1 Auto 叫車加生產閘 | 改碼 | **是**（閘門組合） |
| 6 | V2 300 秒逾時：甲（取消）或乙（文件化+600s+非生產不升級） | 改碼+文件 | **是** |
| 7 | V7 事件送出後清 bitmap | 改碼 | 否 |
| 8 | V9 `BeginPrep` idempotent guard | 改碼 | 否 |
| 9 | V8 叫車取消事件 | 改碼 | **是**（要不要新 CEID） |
| 10 | V5 看門狗跨斷線續計（若 V2 選乙） | 改碼 | 視 V2 |

半開連線 `OnPeerConnected` 未先 tear-down 一事（前一版的 W2）仍成立且應併入第 2 項之前處理：
`uHGemEquipment.cpp:2264` 無條件 `ActiveSocket = Socket;`，不先呼叫 `OnPeerDisconnected()`，
半開連線被新連線靜默取代時 `GemLogic->OnCommunicationLost()` 不會跑 →
`PP_SIGNALTOWER`/`PP_MUSIC` 面板覆寫與 AMR 鎖被孤立（依 `:2286-2293` 註解）。

建置閘：每項 C++ 改動後刪對應 `.obj` 再編譯；SECS 核心改動 → `scripts/ops/build-ht160s.ps1 -Clean`
＋真機建置驗證（註解 `SOFT_SIMULATE` 跑 `-Full`，確認 exit 0 後**還原** define 重編）。

---

## 只有客戶 log 能回答的問題（隨信索取）

1. EAP **應用層**是否真的收到並解析了 `17:23:55.610` 的 `CEID 272`？
   （`S6F12` 是 SECS driver 產生的，不代表排程器看到了。請提供 `17:23:55`–`17:32:39` 的 EAP log。）
2. EAP 是否有**任何**程式路徑能送出 `Loader` 以外的 `START_AGV` 站名？
3. 2026-09-02 是否真的有派車到 Auto3？（線上跡證顯示沒有。若有，代表 EAP 走 out-of-band 派車。）
4. EAP 是否有「不派車給回報 `SVID 1011 = HALT` 的機台」的政策？
   （他們今天讀了 1011 共 197 次，整段相關窗口都被回 `HALT`，連 CEID 272 封包的
   `RPTID 502` 第 3 欄也是 `HALT`。若有此政策，不回應是**刻意**的。）
5. 為何 EAP 無法在 6 秒內回 `Linktest.req`（211 次）？訊息泵被 DB/UI 卡住？防火牆丟 SType=5？
6. 為何每次重連都重做 provisioning（今天 207 次）而不是每次設備電源循環一次？
7. `AUTO6` CP 間歇缺席（11/20）是否刻意？
8. **他們的 EAP 整合者是照我方哪一份文件開發的？**（直接決定第 1 項怎麼陳述。）
9. **Auto 滿車 AMR 收貨交握在這個站點曾經端到端成功過嗎？**
   （兩天證據裡沒有一次成功實例：20 Ready + 7 Finish 全是 Loader；兩次 WAR0962（09-01 的 P4、
   09-02 的 P6）都發生在非生產狀態的機台上。若從未成功過，這要當成**功能未驗收**處理，不是異常。）

---

# V3 追加調查（20260902，使用者交辦「查清楚」）

## 一、`iTrayCount` 的真實語意（原始碼確認）

唯一遞增點在 **`TAutoModule::DoFeedTray()` case 7000**（`aAuto1To6.cpp:826-877`，函式起點 `:677`），
且**只在 `GeneralSetting.bUseAMR` 為真時**：

    if(GeneralSetting.bUseAMR)
    {
        int n=Car[Index].iTrayCount;
        if(n>=0 && n<MAX_TRAY_PER_CAR)
        {
            Car[Index].Tray[n].SetKind(...);
            Car[Index].Tray[n].TrayID=WorkingTrayID[Index];
            if(WorkingKind[Index]==eTrayKindIdentity) Car[Index].CarID=WorkingTrayID[Index];
            Car[Index].iTrayCount=n+1;
        }
    }

**語意 = 「後方盤被提升到工作位」的次數（feed-time counter）**，不是堆疊佔用計數。

→ **我先前的擔憂（CleanOut GoUp 路徑繞過計數器）不成立。** 計數發生在 feed 時，
在 CleanOut 把工作盤堆下去之前；`:1891` 註解也明載「only grows on DoFeedTray and is never reset
by discharge/cleanout」。每片盤要到工作位都得經過 `DoFeedTray`，所以帳在機台自己餵料時是對的。

## 二、但真因不是清帳 — 是人手放料（雙快照 + EventLog 交叉確認）

| 時間 | `SnAuto3_InputEnd` | `SnAuto3_InputFullTray` | `CarTrays` | `AmrLocked` | `RunGate` |
|---|---|---|---|---|---|
| **15:37:21** 快照 | **1**（有盤） | **0**（未滿） | 0 | 0 | 0 |
| **17:33:20** 快照 | **1** | **1**（滿） | 0 | 1 | 0 |

其餘五個 Auto 在 17:33 全部 `InputEnd=0` + `InputFullTray=0` — **只有 Auto3 有料。**

**決定性證據 — EventLog `HT160S_2026_09_02.csv`：**

    2026/09/02,15:33:25.523  "LOT END pressed"
    2026/09/02,15:33:25.528  "End of Lot: Lot=NQ8002ZAA1, TotalIC=15, UPH=177"
    2026/09/02,15:37:21.850  "SNAPSHOT Manual ok ...15_37_21.zip"
    2026/09/02,15:38:27.692  "Safe Slide Door Left is Opened"      <-- 有人進機台
    ...（1 小時 47 分鐘完全沒有任何事件）...
    2026/09/02,17:25:45.608  "AGV: HSMS link lost while P6 holds an AMR lock ..."

`RunGate` 自 15:33:25 落下後**再也沒有升起**（15:37 快照 `MsSinceRunGateFall=235815`；
17:33 快照 `=7195467`），`SystemStart=0`、`LotCount=0`、七個模組 CurTask 全停在 1。
**機台在這段期間沒有生產任何東西，不可能自己把盤放上 Auto3。**

**因果鏈（已定案）**

1. `15:33:25` Lot End → 帳被冷清（`InitialAllTask()` → `InitialFlag` 的
   `Car[Index].Clear(); InitAutoCarStack(Index);`，`aAuto1To6.cpp:120-121`）；
   殘留盤留在 Auto3 進料端（`InputEnd=1`、`InputFullTray=0`）。
   ※ 這是**設計行為**，`csystem.cpp:1922-1927` 註解明載「the output cars hold the sorted product
   for operator/AMR removal exactly as a manual Lot End leaves them today」。
2. `15:38:27` 左側安全滑門被打開 → 操作員進入。
3. 之後約 1 小時 45 分鐘內，**操作員把盤堆到 Auto3** → `SnAuto3_InputFullTray` 轉 ON。
4. `17:23:55` `PollAndCall` 讀到滿盤 sensor（`IsOutputCarFullForAmr` 在真機是**純 sensor 讀取**，
   `aAuto1To6.cpp:1575-1583`），發 `CEID 272` + `CEID 37`。此時帳當然是 0 —
   **人手放的料，任何軟體帳與任何 sensor 都無法計數。**
5. `17:32:08` `WAR0962`。

## 三、結論與修法（修正先前的建議）

- ❌ **「lot 轉換時若 `InputEnd` 為 ON 就不清帳」對本次事件沒有幫助。**
  盤不是機台餵的，帳裡從來沒有這些盤。此修法仍可保留為次要一致性改善，但**不是本案的解**。
- ❌ **「由 sensor 重建帳面」不可能** — 每個 Auto 只有兩個布林 sensor（`InputEnd`、`InputFullTray`），
  無盤數計數硬體。
- ❌ **`fHasTray` 不能當仲裁** — 純軟體 latch（`aColor.h:80-81` 明文「no sensor stands behind」），
  快照顯示它在叫車當下已是 false。
- ✅ **真正的解是 V1 的生產閘**（使用者已裁定：非生產狀態可叫車但不升級成 B 類警報）。
  機台在一台停機、無 lot、料由人手放置的狀態下叫車並停機，這是本案唯一該由我方負責的行為。
- ✅ **並且要誠實對待數字**：帳無法描述實體時不應聲稱 0。至少在 `CEID 272` 的 log 裡
  同時記下「帳三元組」與「sensor 三元組」，讓不一致當場可見（見下方測試 log）。

## 四、發現的 log 缺口（這次分析被卡住的原因）

| 缺口 | 影響 |
|---|---|
| `SnAutoN_InputFullTray` / `InputEnd` **沒有任何邊緣 log**（全樹 grep 零筆） | 「sensor 何時轉 ON」只能靠兩份快照夾逼，無法定時 |
| `Car[].Clear()` / `InitAutoCarStack()` **沒有 log** | 無法知道清帳當下丟掉了多少盤數 |
| `iTrayCount` 遞增**沒有 log** | 帳沒有歷史，只有快照的當下值 |
| `CEID 272` 發射時**沒有把帳與 sensor 並排記下** | 不一致要靠事後翻 SML body 才看得到 |
| 安全門只有 "is Opened"，**沒有 "is Closed"**，期間也沒有任何操作員動作記錄 | 只能推斷「有人進去」，無法知道做了什麼 |

## 五、建議加入的測試 log（最小、走既有 `RecordProcess()` → EventLog → 隨快照打包）

全部用 `RecordProcess()`，不新增檔案、不新增執行緒、不阻塞控制路徑；新註解 ASCII 英文。

1. **Auto 滿盤／有盤 sensor 邊緣**（`aAuto1To6.cpp`，每站一組 latch 比對前值）

       RecordProcess("AUTO3 SN EDGE: InputFullTray 0->1  InputEnd=1 InputHasTray=0"
                     "  CarTrays=0 CarHasTray=0 fHasTray=0"
                     "  RunMode=Normal SystemStart=0 RunGate=0 Lot=");

   → 這一行單獨就能回答本次全部疑問（何時轉 ON、當下機台在不在生產、帳是多少）。

2. **清帳點**（`InitialFlag` 冷分支與 `ClearAmrCar`）

       RecordProcess("AUTO3 CAR LEDGER WIPE: reason=LotEnd  discarding CarTrays=8 CarID=..."
                     "  InputEnd=1 InputFullTray=0");

   → 直接證明清帳有沒有丟掉真實盤數，以及當下實體還有沒有料。

3. **`iTrayCount` 遞增**（`DoFeedTray` case 7000，`bUseAMR` 分支內）

       RecordProcess("AUTO3 CAR FEED: CarTrays 7->8 kind=Normal id=...");

4. **`CEID 272` 發射時把兩組真值並排**（`uAgvStation.cpp` `PollAndCall` Auto 分支）

       RecordProcess("AGV CALL P6 (Auto3): ledger[trays=0 dev=0 id=\"\"]"
                     "  sensor[end=1 full=1 hasTray=0]  MISMATCH"
                     "  RunMode=Normal SystemStart=0 RunGate=0 Lot=");

   → 不一致當場可見，不必再翻 SML body；也讓「該不該叫車」的判斷有據可查。

5. **安全門關閉事件**（補上對稱的 "is Closed"），讓「有人進機台多久」可量。

**驗收方式**：加完後不需改變任何行為，等下次現場再產生一份 State Record，
上述 5 條 log 就能把「人手放料 vs sensor 誤動作 vs 清帳丟數」三者一次分離。
若下次數據顯示 sensor 在無人進入的情況下自行轉 ON，那就是 sensor 故障，屬另案
（參考 `lane-sensor-mechanism-quirks` 已記錄的假亮家族）。
