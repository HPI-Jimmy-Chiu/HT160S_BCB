# HT160S SECS/GEM 通訊範例與格式參考手冊

> **文件用途**：本文件為 HT160S 與客戶端 Host/EAP（京元電子 KYEC）整合時的 **SECS 通訊範例與格式參考**。內容針對每一個 message family 結合「處理流程 + 實際 case log 節錄（逐字引用）+ 模擬輸入資料／格式 + 逐欄 FIELD TABLE（item / SECS-II type / 意義 / 範例值）」，供 host/EAP 整合工程師作為對接依據。本手冊可直接當作 **介面合約（integration contract）** 使用。
>
> **閱讀對象**：本手冊假設讀者是 **第一次接觸 HT160S** 的 host/EAP 整合工程師。為了讓沒看過本機台的人也能讀懂，文中會在容易混淆處補充「**為什麼**」的背景說明（例如：為什麼設備是被動端、為什麼 START 與 START_AGV 要分開、為什麼 resume-START 回 HCACK=4、為什麼本 run 沒有 Color cycle）。
>
> **本文件定位**：這是 **通訊範例／格式參考**，**非** 缺陷報告。所有回覆碼（含 HCACK=4、WinError 10038）都已驗證為「正常／設計行為」，文中會逐一解釋其用意。

| 項目 | 內容 |
|---|---|
| 來源 Run | State Record `2026-06-26 00_01_18`（已驗證的乾淨量產 lot run） |
| 時間窗 | Start Lot `2026/06/25 23:59:27` → Clean Out 結束 `00:01:11` |
| 文件日期 | 2026-06-26（**2026-07-13 修訂**、**2026-08-04 再修訂**：依現行韌體更新格式／行為描述，見下方「修訂說明」） |
| Conformance 結果 | 50/50 checkpoints PASS，0 real defects |
| Protocol Stack | HSMS-SS (SEMI E37) over TCP + SECS-II (SEMI E5) + GEM (SEMI E30) |
| 裝置版本 | HT160S 1.0.0.0 |
| 設備角色 | EQUIPMENT，PASSIVE（`ActiveMode=0`），等待 Host 連入 |
| Host 角色 | HOST/EAP（本範例為 SECS Host Simulator），ACTIVE，發出 Select.req |

> **修訂說明（2026-07-13）**：本手冊原始 case log 仍逐字保留 **2026-06-26 來源 run** 的實況（timestamp／sys／len 皆未更動）。但該 run 之後韌體有數項通訊層變更，為使本手冊繼續作為**現況介面合約**可用，凡「格式／行為描述」與現行程式碼不符處已就地修正，並以 **「自 2026-07 起…／本 run 為舊版故未涵蓋」** 之字樣標示；讀到此類字樣時，代表該項為 run 之後新增、歷史 log 不會出現，請以現行韌體行為為對接依據。主要變更集中在 **§3.4 AGV 握手**（CEID 272／274 加掛 Report 6 的 Tray/Device Count、LoaderTrayCount 改為 work-only）。
>
> **修訂說明（2026-08-04）**：依京元裁定「HT-160S 只發布 HT-90XX 家族 SVID 編號」，10 個 HT160 專屬 `66xxx` SVID 已下架，**預設 Report 1 縮編為只有 `1027 System Time` 一個 SV**（註冊 SVID **76 → 67**：下架 10 個、同時新增家族號 `1008`，故 76 − 10 + 1 = 67）。因 288 個 CEID 連結 Report 1，現行幾乎每一則 S6F11 都只帶 1 個 item；本手冊凡「Report 1＝13 個 SV／`len=144`」之敘述皆已就地標註為 **2026-08-04 前的歷史值**，逐字 log 節錄一律不動。**新增 §3.3.1** 給出實測的現行 S6F11 標準範例，並載明兩項客戶端後果（電力循環後退回 Report 1；`SVID 1006` 於 `RPTID 502` slot 1 改為逗號串接的多 lot 值）。
>
> **如何閱讀本手冊**：
> - 想快速掌握全貌 → 先看 **第 2 章** 的 Mermaid 序列圖（一張圖看完整個 lot run 的訊息往返）。
> - 想對接某一類訊息 → 直接跳到 **第 3 章** 對應小節（每節都有「用途 / 格式 / 實際 log / 欄位 FIELD TABLE」四段式結構）。
> - 想查回覆碼 → 看 **第 5 章** 的 ACK/HCACK 速查表。
> - 想實際重現 → 看 **第 7 章** 的逐步操作。
> - 看到不懂的縮寫 → 查 **附錄** 的名詞對照表（所有縮寫第一次出現時也會就地展開）。

---

## 1. 系統與通訊架構

本章先交代 HT160S 在通訊上的「身分」與「站位」：它說哪種協定、扮演什麼角色、跟誰連、log 寫在哪裡。理解這幾點後，後面每一則訊息的方向（誰送、誰收）就不會搞錯。

### 1.1 Protocol Stack（協定堆疊）

HT160S 採用半導體業界標準的三層 SECS/GEM 堆疊。由下而上分別負責「怎麼傳」「怎麼編碼」「代表什麼意思」：

- **傳輸層**：**HSMS-SS**（High-Speed SECS Message Services – Single Session，SEMI E37）over TCP/IP。負責建立並維持一條 TCP 連線通道。
- **編碼層**：**SECS-II**（SEMI Equipment Communications Standard part 2，SEMI E5）。定義訊息（Stream/Function，如 S2F41）的位元組編碼格式。
- **語意層**：**GEM**（Generic Equipment Model，SEMI E30）。定義設備行為的語意（事件上報、警報、Host Command 等）。

> **為什麼分三層？** 對 host 整合而言，這三層各有對應的對接點：HSMS-SS 決定「連線怎麼建（誰 Active、誰 Passive、用哪個 port）」；SECS-II 決定「body 怎麼拆解」；GEM 決定「收到某個 CEID／RCMD 該怎麼反應」。本手冊第 3 章即按此分層逐一說明。

### 1.2 連線拓撲（Topology）

| 角色 | 裝置 | 連線模式 | 說明 |
|---|---|---|---|
| EQUIPMENT | HT160S | **PASSIVE**（`ActiveMode=0`） | 監聽，等待 Host 連入；DeviceID=0。**已由現場實機確認**：2026-07-31 京元竹南 State Record 的 `system\General.ini [SECS]` 為 `ActiveMode=0`、`Port=6000`、`DeviceID=0` |
| HOST / EAP | SECS Host Simulator | **ACTIVE** | 主動送出 `Select.req` |

- **Port**：`5098`
- **DeviceID**：`0`（equipment 端）
- **連線方向**：本範例中模擬器於 `127.0.0.1:5098` 監聽（device=1），由設備（equipment）主動撥入（dial in）。

> **為什麼設備是 PASSIVE（被動）？** 在標準 GEM 模型中，設備（EQUIPMENT）被定位為「資源」，由 Host/EAP（廠端自動化系統，通常是工廠 MES 的派工端）主動發起連線與下命令；設備只負責回應與上報。因此在實際產線中，HT160S 開機後即進入 PASSIVE 監聽狀態、等待 Host 連入並送出 `Select.req`，**不會** 主動去連 Host。
>
> **本範例的特例**：因為本範例改用 SECS Host Simulator 當對端來重現流程，模擬器設定成監聽方（device=1），由設備這端撥入，所以下方設備 log 會看到 `mode=active`。這只是「重現環境的連線方向」與正式產線相反；**訊息層的角色（誰下命令、誰上報）完全相同**，不影響本手冊任何範例的對接邏輯。

### 1.3 Log 檔案位置

整合對接時，兩端 log 是最重要的比對依據（可用 timestamp 與 sys 序號交叉核對）：

| 端 | 路徑 |
|---|---|
| 設備端（Equipment） | `D:\HT160S_Log\SECS_GEM\YYYY_MM_DD\SECSGEM_TextLog_HH.txt`（每小時一檔） |
| Host 端（Simulator） | `secs_host_YYYYMMDD.log` |

---

## 2. 通訊流程總覽

在逐則拆解訊息之前，先用一張圖看完整個故事。一個完整 lot run 的訊息走向，是一條清楚的時間線：

**連線（connect）→ 設定 lot（set lot）→ 啟動生產（production）→ AGV 補料握手（AGV）→ 警報上報（alarm）→ 結束連線（teardown）**。

下圖即依此順序呈現 host ↔ equipment 的訊息序列，涵蓋：TCP+Select → SET_LOT_INFO → LOTSTART（含 WebAPI pull）→ START → 生產事件迴圈（S6F11；AGV 272→START_AGV→273→274→resume START）→ 警報 S5F1 → CLEAR_LOT_INFO 結批 → Separate。讀完此圖，再進第 3 章看每一步的 body 格式與實際 log，會更容易對位。

> 圖中的 `LOTSTART` **不帶 lot id**（2026-07-30 起；Lot 身分一律由 `SET_LOT_INFO` 設定），結批的
> `CLEAR_LOT_INFO` 亦為同日新增。第 3 章的逐則 log 取自變更前的舊 run，故其 body 仍帶 lot 清單——
> 以本圖與 3.2 的表格為現行合約。

```mermaid
sequenceDiagram
    participant H as HOST / EAP (ACTIVE)
    participant E as HT160S EQUIPMENT (PASSIVE)

    Note over H,E: HSMS-SS 連線建立
    E->>H: TCP connect (equipment dials in)
    H->>E: Select.req
    E->>H: Select.rsp (SELECTED)
    loop ~ every 10s, 全程
        H->>E: Linktest.req
        E->>H: Linktest.rsp
    end

    Note over H,E: Lot 設定與啟動 (S2F41 trio)
    H->>E: S2F41 SET_LOT_INFO (W=1) 疊加登記 Lot
    E->>H: S2F42 HCACK=0
    H->>E: S2F41 LOTSTART (W=1) 無參數 L[0]
    E->>H: S6F11 CEID=6 Lot Start
    H->>E: S6F12 ACKC6=0
    E-->>E: Lot WebAPI pull (2D/Bin reconcile, HTTP 200)
    E->>H: S2F42 HCACK=0
    H->>E: S2F41 START (W=1)
    E->>H: S2F42 HCACK=0 (machine idle -> started)

    Note over H,E: 生產事件上報
    loop 生產 Auto 卸盤 events
        E->>H: S6F11 EventReport CEID=136-138 / 145-147 (W=1)
        H->>E: S6F12 ACKC6=0
    end

    Note over H,E: 批次中途補拉 2D-Bin (可重複)
    H->>E: S2F41 LOTSTART (W=1) 批已開
    E-->>E: Lot WebAPI pull only (不做任何初始化)
    E->>H: S2F42 HCACK=0

    Note over H,E: AGV/AMR 握手 (每站一輪)
    E->>H: S6F11 CEID=272 AGVSupplement (target=Pn)
    H->>E: S2F41 START_AGV(station)
    E->>H: S2F42 HCACK=0
    E->>H: S6F11 CEID=273 AGVLDUnLDStatus (Ready)
    E->>H: S6F11 CEID=274 AGVLDUnLDFinish (Finish)
    H->>E: S2F41 START (resume)
    E->>H: S2F42 HCACK=4 (already running, honest interlock)

    Note over H,E: 警報上報
    E->>H: S5F1 Alarm ALCD=128 (SET)
    H->>E: S5F2 ACKC5=0
    E->>H: S5F1 Alarm ALCD=0 (CLEAR)
    H->>E: S5F2 ACKC5=0

    Note over H,E: 結批 (host 端 Lot End)
    H->>E: S2F41 CLEAR_LOT_INFO (W=1)
    E->>H: S6F11 CEID=8 Lot End
    H->>E: S6F12 ACKC6=0
    E->>H: S2F42 HCACK=0 (停機且機內無 IC)

    Note over H,E: 連線結束
    H->>E: Separate.req (host-side teardown)
```

> **看圖重點**：
> - 連線是 **設備撥入 → Host 送 Select.req**（對應 1.2 的方向說明）。
> - Lot 設定是 **三連發（trio）**：SET_LOT_INFO → LOTSTART → START，三者各有分工（見 3.2）。
>   `SET_LOT_INFO` 給身分（疊加）、`LOTSTART` 開批＋拉 2D/Bin（不帶身分、可重複）、`START` 才讓機構動。
>   完整生命週期還有第四支：`CLEAR_LOT_INFO` 結批（唯一會退掉 Lot 的命令）。
> - 每一輪 AGV 握手結尾的 resume `START` 回的是 **HCACK=4 而非 0**——這是 **正常的誠實互鎖**，不是錯誤（見 3.4）。
> - 結尾的 `Separate.req` 由 Host 端送出，且本範例中發生在設備已關閉之後（見 3.6）。

---

## 3. 逐訊息 範例與格式

本章是手冊的核心。每一小節對應一個 message family，並固定採用 **四段式結構**，方便對接時逐項查核：

1. **用途**：這類訊息在流程中扮演什麼角色。
2. **格式／body 結構**：依 SEMI E5 的 SECS-II 編碼，列出 body 的 L/A/B/U4 結構。
3. **實際 case log 節錄**：逐字引用來源 Run 的設備端與 Host 端 log。
4. **FIELD TABLE（欄位說明）**：逐欄列出 item／SECS-II type／意義／範例值，作為對接時的格式合約。

> **接地規則**：以下所有 case log 節錄均逐字引用自來源 Run，未更動 timestamp、ALID、CEID、byte length 或 sys number。FIELD TABLE 的「範例值」一律取自本 run 的實際 log 或 evidence；非 evidence 涵蓋者於該欄註明。

> **SECS-II 型別速記（FIELD TABLE 用）**：`L[n]` = List（n 個元素）；`A` = ASCII 字串；`B` = Binary（1 byte）；`U4` = 4-byte unsigned integer。

### 3.1 HSMS 連線（Select / Linktest / Separate）

**用途**：建立並維持 HSMS-SS 連線。這是所有後續 SECS-II 訊息的前提——沒有先 SELECTED，任何 S2F41／S6F11 都無從談起。設備為 PASSIVE，Host 為 ACTIVE 發起 `Select.req`；連線進入 SELECTED 後，雙方週期性 `Linktest` 確認鏈路仍活著；結束時送 `Separate.req`。

> **為什麼有 Linktest？** TCP 連線可能在沒有訊息往來時悄悄斷掉（例如網路設備逾時）。`Linktest` 是 HSMS 層的「心跳」，週期性確認鏈路存活，避免在真正要送訊息時才發現連線早已失效。

**HSMS 控制訊息（SType）**：這些是 HSMS 層的控制訊息，不屬於 SECS-II 的 Stream/Function；以 SType 數字區分。**注意：SType 是 HSMS header 內的單一 byte 欄位，本層訊息無 SECS-II body。**

| 控制訊息 | SType | 方向 | 說明 |
|---|---|---|---|
| Select.req | 1 | Host → Equipment | 建立 session |
| Select.rsp | 2 | Equipment → Host | 回應，進入 SELECTED |
| Linktest.req | 5 | 雙向 | 鏈路存活測試 |
| Linktest.rsp | 6 | 雙向 | 回應 |
| Separate.req | 9 | 任一端 | 結束 session |

**實際 case log 節錄（設備端 Equipment）**：

```
2026/06/25 23:59:08.603  [SECS] StartCommunication mode=active port=5098
2026/06/25 23:59:08.603  [SECS] peer connected (TCP up, awaiting Select)
2026/06/25 23:59:08.862  [SECS] Select.req -> Select.rsp (SELECTED)
```

**實際 case log 節錄（Host 端 Simulator）**：

```
[23:59:08] equipment connected: 127.0.0.1:50817
[23:59:08] TX  Select.req
[23:59:08] RX  Select.rsp
[23:59:08] HSMS SELECTED
[23:59:18] RX  Linktest.req -> Linktest.rsp        (then ~every 10s, all answered, whole run)
```

**FIELD TABLE（HSMS header 關鍵欄位）**：

| Item | 型別 | 意義 | 範例值（本 run） |
|---|---|---|---|
| `SType` | byte（HSMS header） | 控制訊息類型碼 | `1`(Select.req) / `2`(Select.rsp) / `5`(Linktest.req) / `6`(Linktest.rsp) / `9`(Separate.req) |
| `mode` | log 欄位 | 本範例連線模式（模擬器監聽、設備撥入）；正式產線設備為 PASSIVE | `active` |
| `port` | TCP | HSMS 監聽埠 | `5098` |
| 連線狀態 | HSMS state | 完成 Select 交握後進入可傳輸狀態（此後才能送 SECS-II 訊息） | `SELECTED` |
| Linktest 週期 | — | 連線建立後約每 10 秒一次，全程皆有回應 | `~every 10s, all answered` |

---

### 3.2 Lot 設定與啟動（S2F41 SET_LOT_INFO / LOTSTART / START + S2F42）

**用途**：這是 host 對設備下達 **Host Command（S2F41）** 的核心三連發（trio）。Host 透過 S2F41 依序設定 lot 清單、啟動 lot（觸發 WebAPI 2D/Bin 對帳）、最後啟動生產。設備一律以 **S2F42** 回覆，body 內含 **HCACK** 回覆碼。

> **為什麼分成三個命令、而且順序固定？** 三者刻意分工、職責不重疊，host 可分階段控制與檢查：
> - **SET_LOT_INFO** = 「告訴設備這批要做哪些 lot」（純資料登錄，**不會讓機構動作**）。
> - **LOTSTART** = 「開批 ＋ 叫設備去拉這些 lot 的 2D/Bin 對帳資料」（觸發 WebAPI pull，但**不會讓機構動作**）。
> - **START** = 「真正開始生產」（讓 motion 動起來）。
>
> 把「登錄資料」「拉對帳資料」「啟動 motion」拆開，host 可以在 START 之前先確認 lot 清單與 WebAPI 對帳都成功，再決定是否啟動，降低誤啟動風險。

> **⚠ 2026-07-30 語意變更（本章下方的 log 是變更前的舊 run，僅供 body 結構參考）**
> 依京元現場 HT9045 全日 log 校準後，trio 的分工改為：
> - **`SET_LOT_INFO`** 成為**唯一**的 Lot 資訊設定入口，且改為**疊加**（不再清空覆寫），同 lot id 只留一筆。
> - **`LOTSTART`** **不再攜帶也不再設定 lot 身分**（京元 host 實際送的就是空 list `L[0]`），一律回 `HCACK=0`、
>   可重複下達。**批未開**→執行整套開批初始化＋拉 2D/Bin；**批已開**→只拉 2D/Bin、不做任何初始化。
> - 新增 **`CLEAR_LOT_INFO`**（對齊 9045）＝ host 端結批，是唯一會退掉 lot 的命令。
> 詳見 `HT160S_SECS_Interface_Spec_20260727.md` §3.4。

> **⚠ 2026-08-20 `SET_LOT_INFO` body 形狀變更（重要，本章下方 5 個 lot 的舊 log 是變更前的形狀）**
> 依客戶（京元 EAP）提供的正式格式改為 **標準 CPNAME/CPVAL 名值對**，與 HT9045 的
> `SET_LOT_INFO` 同一個慣例：
> ```
> L[2]
>   A "SET_LOT_INFO"
>   L[1]                      <- 參數個數（不是批數）
>     L[2]
>       A "LOTID"             <- CPNAME
>       A "lot1,lot2"         <- CPVAL：多個 lot 用「逗號」串接
> ```
> - 內層 `L[n]` 數的是**參數個數**，**不是 lot 數**；一個 `LOTID` 參數即可宣告多個 lot。
> - 與回報方向對稱：`SVID 1006`（CEID 9001 report 8）本來就是「全部已登記 lot 的逗號串」。
> - **退役**：2026-07-21 我方自行提案、未經客戶確認的位置式配對
>   `L[2]{ A 客戶批, A 京元批 }`。它與真實封包**撞形狀**——2026-08-19 現場 host 送
>   `L[2]{"LOTID","NQ100CFAA1"}`，舊碼把它當 (客戶批, 京元批) 讀，於是登記出一個名字叫
>   `LOTID` 的 lot 還回 `HCACK=0`，兩邊都看不出異常。連帶退役的還有該配對專屬的
>   `HCACK=4`（換京元批號且機內有 IC）與「退掉舊 2D 明細」動作：確認後的模型下，宣告的
>   lot id 就是京元/OSAT 批（也就是打 WebAPI 用的 `OSATLot`），客戶批號改由 WebAPI 回應
>   逐顆帶（Soter col6），所以「同一 lot 換批號」在資料上已不存在。
> - **整包沒有任何可辨識參數**（例如只送 `DISPLAY`）→ 回 **`HCACK=2`** 並記 log，不再靜默回 0。
> - 相容保留：平鋪 `A "lotID"` 與 `L[1]{ A "lotID" }` 仍可用（我方模擬器的舊形狀），
>   兩者也一併做逗號切分，所以任何寫法行為一致。

**格式／body 結構（SEMI E5）**：

S2F41 通用結構（**RCMD** = Remote Command，命令名稱；**CPNAME/CPVAL** = 參數名稱/值）：
```
L[2]
  A "RCMD"
  L[n]
    L[2]
      A CPNAME
      A CPVAL
```

回覆 S2F42（**HCACK** = Host Command Acknowledge，回覆碼）：
```
L[2]
  B  HCACK
  L[0]
```

各 RCMD body：

| RCMD | body 結構 | 語意 |
|---|---|---|
| `SET_LOT_INFO` | **（2026-08-20 起，客戶正式格式）** `L[2]{ A "SET_LOT_INFO", L[n]{ L[2]{ A "LOTID", A "lot1,lot2,..." } } }`；相容保留平鋪 `L[n]{ A lotID }` 與 `L[1]{ A lotID }` | lot 清單。內層 `L[n]` 是**參數個數**，多批號在 `LOTID` 的值內用逗號串接（所有寫法都會做逗號切分、trim、封包內去重）。**2026-07-30 起疊加（additive）**：不再 Clear 既有 LotRegistry，同 lot id 留一筆（沿用原 slot 與計數）。`HCACK=2` = 空 list／item 形狀或型別錯／**整包沒有任何可辨識參數**（例：只送 `DISPLAY`）／合併後超出容量 64。不認識的 CPNAME 會被**忽略並記 log**（不整包退，9045 的 `LOT_INFO`+`DISPLAY` 就是多參數），只要同包內有有效 `LOTID` 就照收。**已退役**：位置式 `L[2]{ A custLot, A kyecLot }` 與其專屬 `HCACK=4` |
| `LOTSTART` | `L[2]{ A "LOTSTART", L[0] }`（京元現場實際形式）；亦可選帶 HT-160S 專屬 `L[2]{ A "SORTMODE", A "NORMAL"\|"WHITELIST" }` | **2026-07-30 起不再攜帶 lot 身分**（清單內的 lot id 會被忽略並記 log，lot 一律由 `SET_LOT_INFO` 設定）。一律回 `HCACK=0`、可重複下達。**批未開**→整套開批初始化（per-run 計數歸零、UPH log、Soter buffer、ProductInfo、(Lot,Bin)→Auto 綁定、SVID 1009 開批時刻、CEID 6）＋ `StartLotWebApiPullAll` 拉全部 lot 的 2D/Bin；**批已開**→只拉 2D/Bin、不做任何初始化。**不啟動 motion**（仍需 `START` / operator Start）。唯二非 0 回覆皆來自 `SORTMODE` pair（格式或值錯誤 2；批已開要換模式或 WhiteList 要重載檔案 4） |
| `CLEAR_LOT_INFO` | `L[2]{ A "CLEAR_LOT_INFO", L[0] }` | **2026-07-30 新增**，對齊 9045 = host 端結批（走面板 Lot End 同一條 `DoLotEndProcess`）。運轉中回 1、機內有 IC 回 2。`SET_LOT_INFO` 改疊加後，這是唯一會退掉 lot 的命令 |
| `START` | `L[2]{ A "START", L[0] }` | start/resume 生產；與 START_AGV 刻意分開（原因見 3.4）；於 CEID 274 Finish 後送出 |

> `SET_LOT_INFO` 由 `ht160s_presets._set_lot_info` 建構，預設 5 個 lot `SIMU_LOT_A..E`（2026-08-20 起送客戶形狀：5 個批號逗號串接在一個 `LOTID` 參數內；舊的平鋪形狀另存為 preset `S2F41 SET_LOT_INFO (flat)` 供回歸）；`LOTSTART` 由 `_lot_start` 建構；`START` 由 `_start` 建構。

**實際 case log 節錄（設備端 Equipment）**：

```
23:59:25.060  [SECS][RX] S2F41 decoded rc=1 items=22
23:59:25.062  [SECS][TX] S2F42 W=0 len=21 (sent)
23:59:25.062  [SECS] S2F42 cmd=SET_LOT_INFO HCACK=0 Lots=5
23:59:26.059  [SECS][RX] S2F41 decoded rc=1 items=10
23:59:26.077  [SECS] S2F42 cmd=LOTSTART HCACK=0 Lots=5
23:59:27.543  [SECS][RX] S2F41 decoded rc=1 items=7
23:59:27.598  [SECS] S2F42 cmd=START HCACK=0 Lots=5
```

**實際 case log 節錄（Host 端 Simulator）**：

```
[23:59:25] TX  S2F41W  (S2F41 SET_LOT_INFO)   ;  [23:59:25] RX  S2F42  (sys=20558)
[23:59:26] TX  S2F41W  (S2F41 LOTSTART)        ;  [23:59:26] RX  S2F42  (sys=20559)
[23:59:27] TX  S2F41W  (S2F41 START)           ;  [23:59:27] RX  S2F42  (sys=20560)
```

**FIELD TABLE — S2F41 Host Command（送出）**：

| Item | 型別 | 意義 | 範例值（本 run） |
|---|---|---|---|
| `RCMD` | A | Host 命令名稱 | `"SET_LOT_INFO"` / `"LOTSTART"` / `"START"` |
| lot list 元素 | A（於 `L[n]` 內） | 本 run 的舊形狀：一個 item 一個 lot ID（仍相容） | `"SIMU_LOT_A"` … `"SIMU_LOT_E"`（共 5 筆） |
| `CPNAME` | A | 參數名稱。**2026-08-20 起 `SET_LOT_INFO` 使用此形式**，唯一有作用的名稱是 `LOTID`（比對時 trim + 不分大小寫）；其他名稱忽略並記 log | `"LOTID"` |
| `CPVAL` | A | 參數值。`LOTID` 的值 = **逗號分隔的 lot 清單**（單批就是一個 lot id，不需逗號） | `"NQ8002ZAA1,NQ80030AA1"` |
| `W`-bit | header | 要求回覆 | `1`（S2F41W） |

**FIELD TABLE — S2F42 回覆（收到）**：

| Item | 型別 | 意義 | 範例值（本 run） |
|---|---|---|---|
| `HCACK` | B | Host Command 回覆碼 | `0`（SET_LOT_INFO / LOTSTART / START 全 OK；START 因機台 idle→started） |
| 第二元素 | `L[0]` | 空 list（無附加參數回覆） | `L[0]` |
| `W`-bit | header | 偶函數回覆，不再要求對方回覆 | `0`（W=0，交易到此結束） |
| `len` | bytes | S2F42 body 長度 | `21` |
| `sys` | header（模擬器交易序號） | 嚴格 +1，可核對無漏訊 | `20558`→`20559`→`20560` |
| `Lots` | log 欄位 | 此時 LotRegistry 內 lot 數（**是登錄表總數，不是這包宣告了幾批**——2026-08-19 現場就是被這個欄位誤導） | `5` |

#### 3.2.1 客戶形狀實測（2026-08-20，另一個 run）

上面的 case log 釘在 2026-06-26 的舊形狀。**客戶正式格式（CPNAME/CPVAL）改完後**於 2026-08-20
以 HSMS 對真韌體實測（19 項斷言全過），逐字節錄如下——**這是另一個 run，不要與上面的
`SIMU_LOT_A..E` log 混讀**：

```
11:15:04.957  [SECS][RX] S2F41 W=1 body:
<L[2]
  <A[12] "SET_LOT_INFO">
  <L[1]
    <L[2]
      <A[5] "LOTID">
      <A[5] "PRB_A">
    >
  >
>
11:15:04.970  [SECS] S2F42 cmd=SET_LOT_INFO HCACK=0 Lots=1

11:15:05.009  [SECS][RX] S2F41 W=1 body:
<L[2]
  <A[12] "SET_LOT_INFO">
  <L[1]
    <L[2]
      <A[5] "LOTID">
      <A[11] "PRB_B,PRB_C">      <- 一個參數、兩個 lot
    >
  >
>
11:15:05.014  [SECS] S2F42 cmd=SET_LOT_INFO HCACK=0 Lots=3

11:15:05.087  [SECS][RX] S1F3  <L[1] <U4[1] 1006>>
11:15:05.087  [SECS][TX] S1F4  <L[1] <A[17] "PRB_A,PRB_B,PRB_C">>   <- 回報方向同樣逗號串

11:15:05.117  [SECS][RX] S2F41 W=1 body:
<L[2]
  <A[12] "SET_LOT_INFO">
  <L[1]
    <L[2]
      <A[7] "DISPLAY">          <- 整包只有不認識的參數
      <A[9] "SOMETHING">
    >
  >
>
11:15:05.123  [SECS] S2F42 cmd=SET_LOT_INFO HCACK=2 Lots=3            <- 不再靜默回 0
```

同一時刻的機台 Process log（`EventLog\2026_08\HT160S_2026_08_20.csv`）：

```
11:15:04.957  SECS SET_LOT_INFO : 1 lot(s) declared - PRB_A
11:15:05.009  SECS SET_LOT_INFO : 2 lot(s) declared - PRB_B,PRB_C
11:15:05.117  SECS SET_LOT_INFO : CP 'DISPLAY' not modelled - ignored
11:15:05.120  SECS SET_LOT_INFO refused : no lot declared (unknown CP 'DISPLAY') - expected L[2]{ A "LOTID", A "lot1,lot2" }
```

> 同一個 run 也驗過：`LOTID` + 多餘的 `DISPLAY` 參數 → `HCACK=0` 且該 lot 照收；
> 平鋪 `A "lotID"` → `HCACK=0`；平鋪值內含逗號 → 一樣切成兩批；
> 空 `L[0]`、`LOTID` 值全為空白 → `HCACK=2`；封包內重複 lot id → 只登記一筆；
> 重送已存在的 lot → `HCACK=0` 且不會多出 slot（疊加、keep-existing）。

---

### 3.3 生產事件上報（S6F11 / S6F12 + CEID + DataID）

**用途**：生產過程中，設備會主動把物料移動等事件以 **S6F11 Event Report** 上報給 host；host 收到後以 **S6F12** 回 ACK。這是設備「告知 host 現在發生了什麼」的主要管道。

> **為什麼是設備主動上報、而非 host 來問？** 在 GEM 模型中，事件（Collection Event）由設備在事件發生的當下即時 push 給 host，host 不必輪詢。每個事件以 **CEID（Collection Event ID，事件識別碼）** 標明「發生了什麼事」。

**格式／body 結構**：

S6F11（**DataID** = 資料識別；**CEID** = 事件識別碼）：
```
L[3]
  U4 DataID
  U4 CEID
  L[reports]{ ... }
```
回覆 S6F12（**ACKC6** = Event Report ACK 回覆碼）：
```
B ACKC6
```
W-bit=1（要求回覆）。

**實際 case log 節錄（設備端 Equipment）**：

```
23:59:30.356  [SECS][TX] S6F11 EventReport DataID=1 CEID=136
23:59:30.356  [SECS][TX] S6F11 W=1 len=30 (sent)
23:59:30.361  [SECS][RX] S6F12 decoded rc=1 items=3
23:59:30.361  [SECS][RX] S6F12 reply ignored          ("reply ignored" = S6F12 is the even-function ACK; nothing further to do)
```

**實際 case log 節錄（Host 端 Simulator）**：

```
[23:59:30] RX  S6F11W  (sys=1)
[23:59:30]     -> auto S6F12 ACKC6=0
```

**「reply ignored」說明**：`S6F12` 為偶函數（even-function）ACK，設備收到後**無後續動作**，僅標記為 reply ignored，屬正常行為，**非錯誤**。（凡 even-function 回覆都是一筆交易的結尾，收到即代表對方已確認，不需再回。）

**FIELD TABLE — S6F11 Event Report（送出）**：

| Item | 型別 | 意義 | 範例值（本 run） |
|---|---|---|---|
| `DataID` | U4 | 資料識別；**對 host 無語意**，host 依 CEID 分派 | `1`（生產／Auto CEID）／ `0`（AGV CEID） |
| `CEID` | U4 | 事件識別碼 | `136`（Auto 卸盤）／`272`（AGV）／`35`（car full） |
| `L[reports]` | L[n] | report 串列；可為空（無 SV 註冊時 `L[0]`） | 本 run：空（136–142，len=30）／13 SV（CEID 35，len=144）。**自 2026-07-29 起卸盤事件亦帶 13 SV**；**但自 2026-08-04 起 Report 1 已縮編為只有 `1027 System Time` 一個 SV**（見 3.3.1），故現行韌體上述事件一律只帶 **1 個 item** |
| `W`-bit | header | 要求回覆 | `1`（S6F11W） |
| `len` | bytes | S6F11 body 長度 | `30`（EMPTY，136–142）／`144`（CEID 35，Report 1 於當時＝13 SV；**自 2026-08-04 起 Report 1 僅 1 個 SV，此類事件 body 已遠小於 144**）／`86`（CEID 272，**本 run 舊版**；**自 2026-07-13 起**，CEID 272／274 除原有 bitmap report 外另追加 report 6＝9 站 Tray Count＋9 站 Device Count 共 18 個 SV，body 長度已大於 86，本 run 早於此改動故仍顯示 86） |
| `sys` | header（模擬器序號） | S6F11 上報序號池 | `1`（本 run S6F11 sys 為 1..53） |

**FIELD TABLE — S6F12 回覆（收到）**：

| Item | 型別 | 意義 | 範例值（本 run） |
|---|---|---|---|
| `ACKC6` | B | Event Report ACK 回覆碼 | `0`（事件已收到） |
| 設備端處理 | — | 偶函數 ACK，收到即標記 reply ignored（正常） | `reply ignored` |

**DataID 說明**：**DataID 對 host 無語意**——host 是依 **CEID** 來分派處理邏輯，不看 DataID。設備內部用 DataID 區分事件來源，本 run 中：

- AGV CEIDs（272/273/274）使用 `DataID=0`
- 生產／Auto CEIDs（136–142、35）使用 `DataID=1`

> **整合提醒**：host 端請以 CEID 作為唯一的事件分派依據，不要依賴 DataID 的數值。

**本 run CEID 統計**：

| CEID | 次數 | 類別 | report body | 說明 |
|---|---|---|---|---|
| 136 | 13 | Auto 卸盤（unload-tray / discharge） | 本 run `len=30`（EMPTY body） | 當時未綁定任何 SV，故 body 為空。**自 2026-07-29 起（commit `ab1b99e`）改為攜帶 Report 1＝13 個 SV；再自 2026-08-04 起 Report 1 縮編為僅 `1027 System Time`，故現行只帶 1 個 SV**（見 3.3.1） |
| 137 | 7 | Auto 卸盤（unload-tray / discharge） | EMPTY（`len=30`） | 同上 |
| 138 | 4 | Auto 卸盤（unload-tray / discharge） | EMPTY（`len=30`） | 同上 |
| 140 | 4 | Auto 卸盤（unload-tray / discharge） | EMPTY（`len=30`） | 同上。**⚠ 此號已作廢**：Auto4 卸盤自 commit `bf9d048` 起改為 **145** |
| 141 | 3 | Auto 卸盤（unload-tray / discharge） | EMPTY（`len=30`） | 同上。**⚠ 此號的 Auto5 卸盤語意已作廢**：Auto5 卸盤自 commit `bf9d048` 起改為 **146**。**⚠⚠ 但 `141` 本身不是空號 —— 2026-08-03 起被重新指派為 `GEM Control State Change`**（控制狀態每次變更即發，2026-08-04 實測收到），請勿把現行的 141 當作卸盤事件解析 |
| 142 | 3 | Auto 卸盤（unload-tray / discharge） | EMPTY（`len=30`） | 同上。**⚠ 此號已作廢**：Auto6 卸盤自 commit `bf9d048` 起改為 **147** |
| 272 | 6 | AGV AGVSupplement | 本 run `len=86`（僅 Report 2＝SVID 38219 bitmap） | 見 3.4。**自 2026-07-13 起（commit d10b9be）272 額外掛載 Report 6（全 9 站 TrayCount＋DeviceCount 共 18 個 SVID），body 較本 run 增大；本 run 為舊版故僅含 bitmap** |
| 273 | 6 | AGV AGVLDUnLDStatus | Report 3（SVID 38220 bitmap） | 見 3.4。273 僅帶 StatusBitmap（Ready 階段尚未計數，故不掛 Report 6） |
| 274 | 6 | AGV AGVLDUnLDFinish | Report 4（SVID 38221 bitmap）；自 2026-07-13 起另含 Report 6 | 見 3.4。**自 2026-07-13 起（commit d10b9be）274 除 bitmap 外額外掛載 Report 6，關帳回報該站實際盤數／IC 數；本 run 為舊版故未涵蓋** |
| 35 | 1 | Auto1 "car full" edge | 本 run `len=144` | `AutoFullCeid[0]=35`；本 run 時 Report 1 註冊 13 個 SV。**自 2026-08-04 起 Report 1 僅 `1027 System Time` 1 個 SV**（見 3.3.1） |

> **為什麼本 run 中 136–142 的 report body 是空的（len=30）？** 這 6 個 CEID 是 **Auto 卸盤（unload-tray / discharge）事件**。當時它們在設計上 **刻意不註冊任何 report SV**（lightweight、與 HT9045 對齊），所以 S6F11 只攜帶 CEID、不攜帶資料欄位——這是 by design，不是漏帶資料。
>
> **⚠ 自 2026-07-29 起兩項都已改變（本 run 為舊版故未涵蓋）**：
>
> 1. **號碼改了**：Auto4/5/6 卸盤由 `140/141/142` 更正為 **`145/146/147`**（commit `bf9d048`），以符合 HT9045 `EventReport_CEID.def` 的實際編號。現行陣列為 `aAuto1To6.cpp` 的 `AutoCeid[6]={136,137,138,145,146,147}`。**本 run 出現的 140/141/142 為舊編號。****⚠ 但 `141` 這個號碼現在有新語意、且是活躍事件**：2026-08-03 起 `141` 被**重新指派**為 **GEM Control State Change**（控制狀態每次變更即發，`uHGemHT160.cpp` 的 `EventDescription[GemControlStateChange]="141 GEM Control State Change"`），2026-08-04 實測連線當下就收到 `CEID=141`。所以「不會再看到」只適用於 **140 與 142**；`141` 會看到，但它不再是 Auto5 卸盤。
> 2. **body 不再是空的**：CEID 字典整份改為 HT9045 逐字複本後（commit `ab1b99e`），這 6 個號碼與其他事件一樣註冊在 **Report 1**（13 個 SV），body 長度與 CEID 35 同級（`len=144`），不再是 `len=30` 的空報表。若 host 端已針對空 body 撰寫解析，請一併更新。
>    **⚠ 再更新（2026-08-04）**：Report 1 已依京元裁定縮編為 **只有 `1027 System Time`**，故這 6 個號碼現行僅帶 **1 個 SV**（body 遠小於 144），但仍**不是**空 body。詳見 3.3.1。
>
> **CEID 名稱**：現行完整名稱為 HT9045 原文 `Auto 1..6 Unloading tray`，見設備端 `SecsGem/uHGemHT160.h` 的 `ETypeStruct` 與 `SecsGem/uHGemHT160.cpp` 的 `EventDescription`。

#### 3.3.1 Report 1 縮編（2026-08-04）：現行 S6F11 標準範例

**變更內容**：京元裁定 HT-160S 只得發布 HT-90XX 家族的 SVID 編號，因此 10 個 HT160 專屬 `66xxx` SVID 全部下架，原本掛在 **Report 1** 的 13 個 SV（`1001`／`1003`／`1021`／`1027` ＋ 其中 9 個 `66xxx`）亦隨之縮編。**現行 Report 1 只有 `1027 System Time` 一個 SV**，與 HT9045 的 `AddReprot`（`ReportIDContent[]={1027}`）以及京元自家 HT-9046 持久化的 `EventReport_ReportID.def`（唯一一列：RPTID 1／Type 1／SVID 1027）**逐位元組一致**。註冊 SVID 總數 76 → 67。

**影響範圍**：292 個 CEID 中有 **288 個**連結 Report 1（例外只有 AMR 握手的 272／273／274／275，分別改連 report 2＋6／3／4＋6／7），故現行幾乎每一則 S6F11 都只帶 **1 個 item**；`S1F24 Event Namelist` 走同一條連結鏈，那 288 列的 VID 清單也由 13 個縮為 1 個。**Report 2–7（AMR bitmap、各站 Tray／Device Count、身分盤 2D）完全未變。**

**現行 S6F11 標準 body**（2026-08-04 以 headless HSMS host 對重建後的 `EXE\ht160s.exe` 實測，非推導）：

```
S6F11  L[3]
  <U4 DATAID>
  <U4 CEID>
  L[1]
    L[2]
      <U4 1>                              (RPTID = 1)
      L[1]
        <A[19] '2026/08/04 15:49:53'>     (SVID 1027 System Time)
```

實測涵蓋 CEID `27`／`141`／`93`／`6`／`8`，五者皆為上述形狀、皆只帶一個值。

**兩項客戶端必須知道的後果**：

1. **⚠ 每次電力循環後會退回 Report 1（本次變更唯一的實質回歸）**：`S2F35` 的「事件→報表」連結**只存在於本次 session、從不持久化**。因此每次重開機後，全部 288 個 id 都會退回連結 Report 1，直到 host 重跑 `S2F33` ＋ `S2F35` 為止。**本次變更前**，這段 fallback 空窗期至少還帶 13 個機台狀態 SV；**變更後只帶一個 timestamp**。若 host 依賴開機初期的事件 body 取得機台狀態，請改為連線後盡早重下 `S2F33`／`S2F35`，或改以 `S1F3` 輪詢。
2. **`SVID 1006 Lot ID` 是 host 持久化 `RPTID 502` 的第 1 個 slot**：`1006` 語意已改為「**所有已登錄 lot，以半形逗號（無空白）串接**」，故這個逗號串會直接出現在既有的 `RPTID 502` 事件報表內。**item 數與 A 型別皆未變**，但把 slot 1 當成單一 lot token 解析的 host **必須改成支援多 lot**。單 lot 時輸出與改版前逐位元組相同（例：`PROBE_LOT_A`）；三個 lot 時為 `PROBE_LOT_A,PROBE_LOT_B,PROBE_LOT_C`（依登錄順序）；登錄表為空時退回主畫面 lot 欄位文字（行為未變）；被 `RemoveLot` 釋放的空 slot 會跳過，故不會出現開頭或連續的逗號。

**下架的 10 個 SVID 與替代讀法（host 端請移除引用）**：

| 已下架 SVID | 原意義 | 替代方式 |
|---|---|---|
| `66000` | Run Mode | 改用家族 SVID **`1008` Run Mode**（A 型，恆為 `"0"`＝Normal；家族值域 `0:Normal; 1:RT; 2:EQC`）。機台的 TASK 模式（Home／OneCycle／CleanOut／TrayFeed）**不再有任何 SV 發布**，請讀 **`1011` Machine State** 或追蹤對應的 task CEID |
| `66001` | System Running | 無替代。運轉／停止請看 **`1011`** 的狀態文字與 Start／Pause 事件。⚠ **Start 有兩個號碼，兩個都要訂**：**CEID 1** 只在機內**無** IC 時發，機內**有** IC 時的 Start 是 **CEID 76**（暫停後續跑、警報解除後續跑都走 76 —— 產線上這才是常態）；停止側為 **CEID 2**。只訂 CEID 1 會漏掉絕大多數的重新起動。HT9045 同樣沒有這支 SV |
| `66002` | Control State | 無替代。**`4` GemControlState**（＋ **`9` PreviousGemControlState**）成為唯一發布的控制狀態，值域比照 HT9045：`1`=Off-Line／`2`=On-Line Local／`3`=On-Line Remote。機內仍保有自己的 GEM 標準 `1/4/5` 值域，但不再對外發布 |
| `66010` | Alarm Active | 無替代。警報一律走 **S5F1** 串流（目錄查 **S5F6**），與 HT9045 完全相同（見 3.5） |
| `66011` | Alarm Code | 同上 |
| `66020` | Total IC | 改讀 **`1101` Loader Count**（自 Loader 盤取走的 IC 數）與／或 **`1102` Output Total Count**（放入 bin 的 IC 數） |
| `66021` | Total Sorted | 改讀 **`1102` Output Total Count**——它本來就綁在同一個計數器（`MachineRun.iTotalSorted`），純屬去重 |
| `66030` | Active Lot Count | 由新的 `1006` 推導，但**必須先判空**：`1006` 為空字串 → **0 批**；否則 lot 數 =（逗號數 + 1）。⚠ 未開批時 `1006` 回主畫面 lot 欄位的文字（通常為空），此時機上其實是 0 批，直接算「逗號數 + 1」會把 0 批誤報成 1 批（已退役的 `66030` 在該狀態回 0） |
| `66031` | Current Lot ID | 由新的 `1006` 推導：第一個 lot = 第一個逗號前的字串 |
| `66032` | Sort Mode | 無替代。家族沒有 sort-mode SVID（唯一候選 `35530`「[I27] Manual sort mode」是 HT9045/46 的 BOOLEAN 選項）。**已知並接受的代價**：host 仍可用 `S2F41 LOTSTART` 的 `SORTMODE` pair **設定**模式，但**無法再用 `S1F3` 讀回** |

> **未註冊的 SVID 是「可偵測的空值」**：以 `S1F3` 查上述任一個已下架號碼，設備回的是**空 list `<L [0]>`**（位元組 `01 00`），且 list 長度與請求筆數一致；這與「已註冊但值為空」的 **`<A [0]>`**（位元組 `41 00`）**在線上可區分**。2026-08-04 已並排實測：`66031` → `L,0`，`1006` → `A,0`。
>
> **完整 SVID 目錄**：以 `S1F11 <L[0]>` 取回共 **67 個** SVID，十個下架號碼皆不在其中、`1008` 在其中：
> `3, 4, 9, 1001, 1002, 1003, 1006, 1007, 1008, 1009, 1011, 1021, 1027, 1101, 1102, 1103, 1104, 1105, 1259, 1260, 1261, 1501, 1517, 1518, 2758-2763, 37010, 38199-38207, 38219-38245`。
> 其他實測值：`1008` → `<A [1] '0'>`；`4` → `<U1 3>`；`1011` → `<A 'HALT'>`；`1101`／`1102` → `I4`；`1021` → `I4`；`1027` → 19 字元 timestamp。

---

### 3.4 AGV / AMR 握手（272 / 273 / 274 / 275 + START_AGV + resume START）

**用途**：當設備缺料或某個 Auto 工位車滿時，需要 **AGV/AMR（Automated Guided Vehicle / Autonomous Mobile Robot，自動物料搬運車）** 來補料或卸料。本小節描述設備與 host 之間完成一次 AGV 補給／卸載的完整事件握手：設備發出 CEID 事件，host 依預先設定的 swimlane（自動回應規則）回對應的 Host Command。

> **為什麼 AGV 要先 START_AGV、最後又要一個 resume START？這兩個 START 不一樣嗎？** 兩者是**刻意分開的不同命令**，職責不同：
> - **START_AGV(station)** = 「授權 AGV 對某一站（station）執行搬運動作」——只針對該站的 AGV 交接，不影響整機生產狀態。
> - **resume START** = 「在 AGV 交接完成後，恢復整機生產」——對應 3.2 的同一個 START 命令。
>
> 之所以分開，是因為 AGV 交接期間機台會凍結該站的機構動作（進料側的 GoDown 解疊、出料側的 GoUp 堆盤），待 CEID 274（Finish）確認搬運完成後才解凍。把「授權 AGV 動作」與「恢復整機生產」拆成兩個命令，host 才能精準控制兩件事的時序。
>
> **⚠ 但要理解 resume `START` 在 HT160S 實際上是「儀式性」的：設備並不等它。** 韌體在發出 CEID 274 的同一個 tick 就已自行解除該站的機構鎖並恢復生產（Auto 站另含清空該車）。因此 host 沒送 resume `START` 時機台**不會**卡住。這也是下面 HCACK=4 的成因——送到時機台早已在跑。

**AGV CEID 定義**：

> ⚠ **名稱更正（2026-08-03）**：下表與本章各段 log 摘錄中的 `AGVSupplement` / `AGVLDUnLDStatus` /
> `AGVLDUnLDFinish` / `AGVLdID` 是**舊的 CENAME**。與京元現場 HT-9046 機台自存的
> `EventReport_CEID.def` 逐筆核對後發現該機用的是 `AMR Supplement` / `AMR LDUnLD Status` /
> `AMR LDUnLD Finish` / `AMR LD ID`，本機韌體已於 2026-08-03 改為一致。
> **CEID 號碼、攜帶報表、發射時機、body 內容全部未變**，只有 `S1F24` 回覆的名稱字串改了，
> 因此本章所有既有的 wire 範例與 body dump 仍然完全有效。下表與 log 摘錄保留原字串，
> 因為它們是當時實際輸出的紀錄。

| CEID | 名稱（舊 CENAME／現為 AMR *） | 語意 |
|---|---|---|
| 272 | AGVSupplement → **AMR Supplement** | 呼叫 AGV：缺料／Auto full。SVID `38219` bitmap 標記目標站 |
| 273 | AGVLDUnLDStatus → **AMR LDUnLD Status** | Ready：機構就位 |
| 274 | AGVLDUnLDFinish | Finish：sensor 確認 load/unload 完成。除 SVID `38221` bitmap 外，自 2026-07-13 起（commit d10b9be）額外掛載 Report 6（全 9 站 TrayCount＋DeviceCount），關帳回報實際盤數／IC 數 |
| 275 | AGVLdID | 身分盤（identity/cover tray）2D 上傳。**自 2026-07-14 起（commit c389e3f）已實作發報**：AMR 身分盤由 Loader 退出後經 TrayArm 放到 Color 後段，由 Color 載台帶動、讀碼器移到讀取位掃描 2D，**讀到後立即**發 S6F11 CEID275，SVID `38202`（AMR Loader load-port Carrier ID）帶該 2D，走**專屬 report 7＝僅 `38202`**（非 9 站全帶的 report 5）。**⚠ 2026-08-05 起由 `38204` 改為 `38202`**，見下方說明。本 run（2026-06-26）早於此實作故未出現 |

> **關於 CEID 275（AGVLdID）**：275 是身分盤（identity/cover tray）2D 的上傳事件。**本 run（2026-06-26）並未發出 275**——當時韌體尚未實作發報，且本範例的 6 個 AGV cycle 只出現 272／273／274，故 275 **沒有對應的 evidence row 可引用**。
>
> **自 2026-07-14 起（commit c389e3f）275 已實作並會實際發報**。機構流程：身分盤由 **Loader** 退出，**TrayArm** 夾取後放到 **Color 後段**（`aTrayArm.cpp` `PlaceDest=TAPLACE_COLOR`）；接著由 **Color 載台**帶動盤子、**讀碼器沿 X 移到讀取位**掃描 2D（`aColor.cpp` `DoReadColor2D`）。讀到真實 2D 後**立即**發出 S6F11 CEID275，SVID `38202` 帶該 2D。要點：
> - **SVID 已於 2026-08-05 由 `38204` 改為 `38202`（韌體 `AMR_IDENTITY_CARRIER_INDEX` 由 2 改為 0）**，理由是對齊 HT9045。
>   **`38204` 現已下架、不再註冊**，host 端請移除引用（連同同批下架的 `38203`，見下方「已下架 SVID」）。
> - **這不是特例，是本來就對齊。**HT9045（HT9046LS V3.32.810_B01）雖然把 `38202` 命名為 *Load Port Carrier ID*，它**也不是在 Loader load port 讀碼**——同樣是把盤送到 buffer 站（Color 或 Empty，由 `TrayForm.iReaderPos` 決定）再用 Keyence 讀（`acatchtray.cpp` `InitialCoverTrayIDTask`）。9045 雖有 Loader 位置的讀碼器，但其資料**沒有綁任何 SVID**。所以「讀取裝置在 buffer 站、數值發布在 `38202`」是 9045 原本的做法，HT160S 與之一致。
> - 每次身分盤進料**只上報一次**單一 2D——用**專屬 report 7（僅 `38202`）**，不夾帶其餘各站的 carrier id；Auto 各站的 carrier id（Auto1-3＝`38205`/`38206`/`38207`，Auto4-6＝`38199`/`38200`/`38201`）改由 host 以 **S1F3 輪詢**取得（與 9045 相同：身分盤 2D 用事件、Auto carrier id 用輪詢）。
> - 作業員略過／讀取失敗（空 2D）者**不上報**（與 9045 一致，不送空碼）。
> - 真機關閉 Color CCD 時韌體產生的暫代碼（`COLOR2D_…`）**不上報**（避免假碼進 MES）。閘門是韌體的 `IsTrayID2DGenuine()`，非字串前綴比對。
>   **⚠ 例外：真機組建若由操作員勾選「模擬」（`bRunSimulation`），該暫代碼仍會上報。** host 端若看到 `COLOR2D_` 開頭的碼，代表現場勾了模擬，應視為無效碼。
> - 報表連結：韌體自帶 report 7 → CEID275。**若 host 以 S2F35 重新連結 CEID275 而未同時用 S2F33 定義所帶的 RPTID**，`38202` 會從該事件掉出去。目前韌體 `bStrictReportValidation` 預設為真，會以 `LRACK=0x05` 拒絕含未知 RPTID 的 S2F35，report 7 因而存活——**此保護是現行行為的前提，勿關閉**。
> - `EventReport` 於 HSMS **SELECTED** 才送（未連線時為 no-op）。
>
> host 端仍不應假設每個 run 都必有 275——僅在該 run 有身分盤補給且連線時才出現。

**START_AGV body 結構**：
```
L[2]
  A "START_AGV"
  L[n]
    L[2]{ A station, A verb }          verb = "Action" | "NA"
    [, L[2]{ A "LoaderTrayCount",  A n }]
    [, L[2]{ A "LoaderICCount",    A n }]
```
`station` ∈ `Loader` / `Empty` / `Color` / `AUTO1`..`AUTO6`（**大小寫不拘**，韌體以大寫比對）。

> **⚠ `verb` 是啟動閘，不是說明文字。** 只有字面 **`"Action"`** 會讓該站進入交接；**`"NA"`** 表示「為了向量完整而列出，不要動作」。這與 HT9045 相同（其 START_AGV 亦要求第二欄等於 `"Action"`）。京元現場 host 的實際用法是**每次送完整站點向量、其中恰好一站帶 `"Action"`、其餘全帶 `"NA"`**。
>
> 送出 `"Action"` / `"NA"` 以外的動詞（例如 `LOAD` / `UNLOAD`）：韌體**記錄 log 但不動作**，且 **`HCACK` 仍為 `0`**，保留給未來擴充的動詞集。

**⚠ SVID 異動摘要（host 端必讀）**

| SVID | 原意義 | 現況 | host 要做什麼 |
|---|---|---|---|
| `38204` | Color 站 carrier ID（曾用於 CEID 275 身分盤 2D） | **已下架**，不再註冊 | 移除引用；身分盤 2D 改讀 `38202` |
| `38203` | Empty 站 carrier ID | **已下架**，不再註冊（從未有值） | 移除引用 |
| `38208` / `38209` / `38210` | Auto4-6 carrier ID（HT160S 早期自創） | **已退役**，不再註冊，S1F3 回空 `L[0]` | 改用 `38199` / `38200` / `38201` |
| `38237` / `38238` / `38239` | AMR Auto4-6 Tray Count | **意義已變更**：現為 *Record Auto 1/2/3 Tray Count*（本工單累計出盤數，Lot Start 重置） | AMR 用途改讀 `38246` / `38247` / `38248`；否則會讀到看似合理的錯誤數字 |

> 上述號碼一律以 **HT9046LS V3.32.810_B01**（京元部署機型）為唯一對齊依據。810_B01 有的照抄（`38202` Load Port Carrier ID、`38205`-`38207` Auto1-3 Carrier ID、`38222`-`38236` AMR 段、`38237`-`38239` Record 段）；810_B01 沒有的則標示為 **HT160S 擴充**。
>
> **Auto4-6 相關的號碼全部是擴充號碼，不是家族號碼**：810_B01 是三站 Auto 機台，其 AMR 資料結構本身就只有六格（Loader/Empty/Color/Auto1/Auto2/Auto3），結構上放不下 Auto4-6，因此家族中不存在可照抄的號碼。擴充號碼包含 Carrier ID `38199`-`38201`、Tray Count `38246`-`38248`、Device Count `38240`-`38242`、Bin Setting `38243`-`38245`。四組皆已確認在 810_B01 未被使用。
>
> **`38223` / `38224` / `38229` / `38230`（Empty / Color 的 Tray Count 與 Device Count）雖然恆為 0，但刻意保留註冊** —— 它們是 810_B01 的家族號碼，而 810_B01 同樣沒有寫入者（該樹所有寫入都只落在 Auto1-3）。保留才是對齊。

**站點對照（P-Map，PIndex = AutoNo + 3）**：CEID 272 的 SVID `38219` 以 bitmap 標出目標站，站號 P1–P9 對照如下：

| P | 站點 |
|---|---|
| P1 | Loader |
| P2 | EmptyTray（**`START_AGV` 的 CP 名稱是 `Empty`**）|
| P3 | ColorTray（**`START_AGV` 的 CP 名稱是 `Color`**）|
| P4 | AUTO1 |
| P5 | AUTO2 |
| P6 | AUTO3 |
| P7 | AUTO4 |
| P8 | AUTO5 |
| P9 | AUTO6 |

**bitmap payload（ASCII）**：`"P1:0,P2:0,...,P4:1,...,P9:0"`。此為**示意格式**——bitmap 中被設為 `1` 的位置，**就是當次握手的目標站**（單一站 P=1，其餘皆 0）；這裡以 `P4=1`（一個 AUTO1 呼叫）為例說明格式。實際 `1` 落在哪一位，視該次的目標站而定（例如下方 CEID 272 FIELD TABLE 取 @00:00:13 的實際 cycle，目標站為 P1 Loader，故 `P1=1`）。

**Auto-AGV Host swimlane（host 端自動回應規則）**：

- 收到 `272`（AGVSupplement）→ 自動送 `START_AGV(station)`（授權該站搬運）
- 收到 `273`（Ready）→ 僅 log（等待機台 sensor 回報 Finish，不做動作）
- 收到 `274`（Finish）→ 自動送 `START`（resume，恢復整機生產）

**Color (P3) 的觸發條件**：與 P1 / P2 相同，是**進料源乾** sensor —— `SnColor_InputEnd` 讀 OFF（韌體 `TColorModule::IsInputShortageForAmr`）。
（**更正**：舊版本文件寫「僅在內部旗標 `bSupplyRequested` 成立時才補給」，那是 Color 站**送盤 branch** 的閘，不是叫車的閘，兩者不同層。）

> **為什麼本 run 沒有出現 P3（Color）的 AGV cycle？** 因為 Color 站採「**按需供給（demand-gated）**」：只有在機台真正提出需求時才呼叫 AGV。本範例是一段很短的 run，期間 Color 站並未產生供給需求，所以 **完全沒有 P3 cycle** 出現。這是 **預期行為**，不是漏掉或失敗——host 端不應預期每個 run 都一定看到 P3。

**實際 case log 節錄 — 完整 AGV cycle，站 P1 Loader @ 00:00:13（設備端 Equipment）**：

```
00:00:13.587  [SECS][TX] S6F11 EventReport DataID=0 CEID=272
00:00:13.587  [SECS][TX] S6F11 W=1 len=86 (sent)
00:00:13.596  [SECS][RX] S2F41 decoded rc=1 items=15
00:00:13.596  [SECS][TX] S2F42 W=0 len=21 (sent)
00:00:13.596  [SECS] S2F42 cmd=START_AGV HCACK=0 Lots=5
00:00:14.597  [SECS][TX] S6F11 EventReport DataID=0 CEID=273
00:00:15.593  [SECS][TX] S6F11 EventReport DataID=0 CEID=274
00:00:15.600  [SECS][RX] S2F41 decoded rc=1 items=7
00:00:15.601  [SECS] S2F42 cmd=START HCACK=4 Lots=5
```

**實際 case log 節錄（Host 端 Simulator）**：

```
[00:00:13] RX  S6F11W  (sys=23)
[00:00:13]     >> AGV AGVSupplement (CEID 272) target=P1 Loader
[00:00:13] TX  S2F41W  (S2F41 START_AGV)
[00:00:13]     >> auto START_AGV(Loader)
[00:00:13] RX  S2F42  (sys=20563)
[00:00:14] RX  S6F11W  (sys=24)
[00:00:14]     >> AGV AGVLDUnLDStatus (CEID 273) target=P1 Loader
[00:00:14]     >> AGV handoff at Loader; awaiting sensor Finish
[00:00:15] RX  S6F11W  (sys=25)
[00:00:15]     >> AGV AGVLDUnLDFinish (CEID 274) target=P1 Loader
[00:00:15] TX  S2F41W  (S2F41 START)
[00:00:15]     >> auto START (resume production)
[00:00:15] RX  S2F42  (sys=20564)
```

> **對照看圖**：上面這段 log 即第 2 章 Mermaid 圖中「AGV/AMR 握手」那一輪的完整實況——272 → START_AGV(HCACK=0) → 273 → 274 → resume START(HCACK=4)。

**FIELD TABLE — CEID 272 AGVSupplement（S6F11 送出）**：

| Item | 型別 | 意義 | 範例值（本 run） |
|---|---|---|---|
| `DataID` | U4 | AGV 事件資料識別 | `0` |
| `CEID` | U4 | 事件識別碼 | `272` |
| SVID `38219`（Report 2） | A（report SV） | bitmap 標記目標站（被設為 `1` 的位置即目標站，其餘 0） | `"P1:1,P2:0,...,P9:0"`（**依格式推得；非逐字 log 值**——本 run @00:00:13 的 log 僅記錄 `target=P1 Loader`，未逐字記錄 bitmap 字串，此處依「單一站 P=1」格式推得 `P1=1`） |
| Report 6（18 個 SVID） | I4×18 | **自 2026-07-13 起（commit d10b9be）新增**：全 9 站 TrayCount（`38222/38223/38224/38225/38226/38227/`**`38246/38247/38248`**）＋全 9 站 DeviceCount（`38228/38229/38230/38231/38232/38233/38240/38241/38242`），依 P1–P9 固定順序排列。**⚠ Auto4-6 TrayCount 已於 2026-08-10 由 `38237/38238/38239` 改為 `38246/38247/38248`**（原號在 HT9045 810_B01 被廠商定義為 *Record Auto 1/2/3 Tray Count*，語意不同，見下方「已改號 SVID」）。線上型別為 **I4**（非 U4）。Report 6 的 body 為位置式（依序只帶值），故照位置解析的 host 不受改號影響；以 SVID 對映／S1F3 輪詢／S2F33 佈署的 host 必須重新對映 | 本 run 為舊版故未涵蓋 |
| `len` | bytes | S6F11 body 長度 | 本 run `86`（舊版僅 Report 2）；自 2026-07-13 起因加掛 Report 6 而增大 |

**FIELD TABLE — START_AGV（S2F41 收到 / 由 host 送出）**：

| Item | 型別 | 意義 | 範例值（本 run） |
|---|---|---|---|
| `RCMD` | A | 命令名稱 | `"START_AGV"` |
| `station` | A | 目標站（CPNAME 位置） | `"Loader"` / `"Empty"` / `"Color"` / `"AUTO1".."AUTO6"` |
| `verb` | A | station 的動作值（CPVAL 位置）。**只有 `"Action"` 會啟動交接**；`"NA"`＝列出但不動作；其他動詞＝記 log 不動作 | `"Action"` / `"NA"` |
| `"LoaderTrayCount"` / `n` | A / A | 選用參數對：Loader 補料盤數。**此為 WORK（工作盤）數，不含 cover／identity 表頭盤**（9045 `iSECSSetTrayCount` 對齊，commit 111b976）；韌體再依 `General.ini [AMR] CoverTray0`＋`IdentityTray0` 補上表頭盤，得出實際 magazine 總盤數（`iCarTrayTotal = 工作盤數 + 表頭盤數`）。**⚠ 消耗即清（consume-once）**：此值只套用到緊接著的那一台車，之後歸零。下一台車若省略本參數，**不會**沿用上一台的數字 | （選用；僅 Loader 站帶） |
| `"LoaderICCount"` / `n` | A / A | 選用參數對：host 宣告的進料 IC 數。存放於 P1 Loader 的 device-count 欄位（SVID `38228`）供回讀。**latched，非 consume-once**，機構不消費此值 | （選用；僅 Loader 站帶） |
| 回覆 `HCACK` | B（S2F42） | 見下方值域表 | `0`（本 run 全部 6 次皆 0） |

**`START_AGV` 的 `HCACK` 值域（2026-08-17 起與 HT9045 對齊）**：

| HCACK | 何時 | 機台狀態 |
|---|---|---|
| `0` | 正常受理。**也包含**：CP 名稱不認得、verb 不是 `"Action"`／`"NA"` —— 這些 CP 被忽略但**不影響**同封包其他 CP | 帶 `"Action"` 的已知站已進入交接 |
| `1` | body 的 LIST 結構壞掉（CP pair 不是 list） | 整包未套用 |
| `2` | **AMR 功能關閉**（`General.ini [HardwareInstall] UseAMR=0`）或 **CP 清單為空** | 整包未套用 |

> **設計說明（2026-08-17 變更）**：先前韌體對「不認得的 CP 名稱」回 `HCACK=2`，但它是**就地**設值、迴圈繼續，因此排在該 CP **之前**、帶 `"Action"` 的站點**已經進入交接並鎖住機構**——host 卻依非 0 的回覆判定「整包被拒」而不派車，機構就被留在鎖定狀態。現已改為與 HT9045 一致：**不認得的 CP 忽略、`HCACK` 維持 `0`**。因此 `START_AGV` 的非 0 回覆現在只代表「整包完全沒有套用」，語意不再自相矛盾。
>
> 表中 `2` 的兩種情形都是**在逐 CP 處理之前**就整包退回，不會造成部分套用。

**FIELD TABLE — CEID 273 / 274（S6F11 送出）**：

| Item | 型別 | 意義 | 範例值（本 run） |
|---|---|---|---|
| `DataID` | U4 | AGV 事件資料識別 | `0` |
| `CEID` | U4 | 273=Ready（機構就位）／274=Finish（sensor 確認完成） | `273` / `274` |
| 273 report SV | A | Report 3＝StatusBitmap（SVID `38220`），bitmap 標記目標站 | `"P1:1,...,P9:0"` |
| 274 report SV | A ＋ I4×18 | Report 4＝FinishBitmap（SVID `38221`）；**自 2026-07-13 起（commit d10b9be）274 另加掛 Report 6**（全 9 站 TrayCount＋DeviceCount，SVID 同 272 之 Report 6，含 2026-08-10 的 Auto4-6 TrayCount 改號 `38246/38247/38248`），關帳回報該站實際盤數／IC 數（本 run 為舊版故未涵蓋） | bitmap＋counts |
| target | （log 標註） | 對應目標站 | `P1 Loader`（本 cycle） |

**本 run 的 6 個 AGV cycle（依目標站）**：

| 時間 | 目標站 |
|---|---|
| 23:59:47 | P2 Empty |
| 00:00:13 | P1 Loader |
| 00:00:18 | P2 Empty |
| 00:00:41 | P4 AUTO1 |
| 00:00:46 | P1 Loader |
| 00:00:51 | P2 Empty |

**CEID=35 雙重發送（與 272 同時 @ 00:00:41）**：當 Auto1 工位「車滿（car full）」的邊緣事件觸發時，設備會在同一時刻同時送出 272（呼叫 AGV）與 35（car-full 事件）。兩者目的不同：272 是叫車、35 是上報車滿狀態（本 run 攜帶 13 個 SV，故 len=144；**自 2026-08-04 起 Report 1 僅 1 個 SV**，見 3.3.1）。

> **離散 Auto-Full CEID 完整集合**：本 run 只有 Auto1 車滿，故僅出現 `35`；現行韌體的離散 Auto-Full CEID（9045 對齊）為 **Auto1–3＝`35`／`36`／`37`、Auto4–6＝`148`／`149`／`150`**（`uHGemHT160.cpp:273` 的 `AutoFullCeid[6]`；於 `uAgvStation.cpp` 車滿邊緣與對應 272 同時發出），皆掛 Report 1（**自 2026-08-04 起 Report 1 僅 `1027 System Time` 1 個 SV**；本 run 當時為 13 個 SV，見 3.3.1）。host 端應對這 6 個 CEID 一致處理，不應只認得 35。

```
00:00:41.586  [SECS][TX] S6F11 EventReport DataID=0 CEID=272
00:00:41.586  [SECS][TX] S6F11 W=1 len=86 (sent)
00:00:41.586  [SECS][TX] S6F11 EventReport DataID=1 CEID=35
00:00:41.586  [SECS][TX] S6F11 W=1 len=144 (sent)
```

**FIELD TABLE — CEID 35 Auto1 car-full（S6F11 送出）**：

| Item | 型別 | 意義 | 範例值（本 run） |
|---|---|---|---|
| `DataID` | U4 | 生產／Auto 事件資料識別 | `1` |
| `CEID` | U4 | 事件識別碼（`AutoFullCeid[0]=35`） | `35` |
| `L[reports]` | L[n] | report 串列。本 run 當時 Report 1 註冊 **13 個 SV**（與空 body 的 136–142 對比）；**自 2026-08-04 起 Report 1 僅 `1027 System Time`，故現行只帶 1 個 item**（見 3.3.1） | 13 SV（本 run） |
| `len` | bytes | S6F11 body 長度（本 run 13 SV → 144；**2026-08-04 後只有 1 個 SV，body 遠小於 144**） | `144`（本 run） |
| 觸發時機 | — | 與 CEID 272 同時刻雙發（叫車 + 上報車滿） | `@00:00:41`（272 + 35 同一 timestamp） |

**HCACK=0 vs HCACK=4 說明（關鍵，務必理解）**：

| 命令 | HCACK | 說明 |
|---|---|---|
| `START_AGV`（全部 6 次） | `0` | OK，正常接受 |
| resume `START`（全部 6 次） | `4` | **By design**。此時 `SystemStart` 已為 true → MachineStart() 回 `msRejBusy`，機台誠實回報「已在運行」（`uHGemHT160.cpp:883-889`，其中行 886 `case msRejBusy: HCACK = 4; break;`；`MachineStart()` 的 `msRejBusy` 條件為 `SystemStart!=false`，見 `csystem.cpp:1133-1134`）。這 **不是失敗**，而是誠實互鎖（honest interlock），非假成功（not a fake-success）。 |

> **為什麼 resume START 回 HCACK=4（busy）而不是 0？這算失敗嗎？** 不算失敗。HCACK=4 的定義是「busy（機台已在運行 **或** ICs 仍在機內）」。AGV 交接期間機台其實**並未真正停機**，整機生產狀態仍是運行中；因此當 host 在 274 之後送 resume START 時，機台「誠實地」回報「我本來就在跑了，這個 START 無需再啟動」（HCACK=4）。
>
> 這正是所謂「**誠實互鎖（honest interlock）**」：機台寧可如實回 busy，也不會假裝「啟動成功（HCACK=0）」來討好 host。**host 端收到 resume-START 的 HCACK=4 應視為正常結果**，代表生產持續中，無需重試或告警。

> **文件修正提醒（給有看過舊文件的讀者）**：模擬器舊版 `docs/SECS_MESSAGES.md` 仍寫「START_AGV/START not yet implemented → HCACK=1」，**該文字為 STALE（過時）**。實際設備兩者皆已實作，且本 run 的真實回覆碼為 **START_AGV=0、resume-START=4**（皆非 1）；請以本文件的實際行為為準。

---

### 3.5 警報上報（S5F1 / S5F2）

**用途**：當設備發生需通報的狀況（如缺盤、清場）時，以 **S5F1** 上報警報的 SET（成立）／CLEAR（解除）；host 以 **S5F2** 回 ACK。每個警報的「成立」與「解除」會成對出現（先 SET、後 CLEAR）。

**格式／body 結構**：

S5F1（**ALCD** = Alarm Code byte；**ALID** = Alarm ID；**ALTX** = 人類可讀 alarm 文字）：
```
L[3]
  B  ALCD
  U4 ALID
  A  ALTX
```
回覆 S5F2（**ACKC5** = Alarm Report ACK 回覆碼）：
```
B ACKC5
```
W-bit=1。

**ALCD（SEMI E5 標準）**：以 byte 的 bit7 表示 SET/CLEAR：

| ALCD | 值 | 意義 |
|---|---|---|
| SET | `0x80` = `128`（bit7） | 警報成立 |
| CLEAR | `0x00` = `0` | 警報解除 |

**ALID 推導**：本機的 ALID 是以 alarm-code **字串** 做 31-poly rolling hash 算出的 U4（`alid = alid*31u + (unsigned char)byte`，`UsecegemMainFrom.cpp:150-152`）。此演算法 stateless／deterministic（同一字串永遠算出同一 ALID）；真正人類可讀的代碼承載於 **ALTX** 欄位。已驗證（python 重算 byte-for-byte 相符）：

| ALTX（alarm code 字串） | ALID（U4） |
|---|---|
| `"Loader Tray Empty"` | `4045923824` |
| `"SnFKCleanOut"` | `3891410149` |

> **ALTX 的內容範圍（重要）**：實際 handler（`UsecegemMainFrom.cpp:154-156`）在 **沒有附加 Message 時** 設 `altx = Code`（即純 alarm code 字串），在 **有 Message 時** 設 `altx = Code + ' ' + Message`（即 code、一個空白、再接人類可讀訊息）。換言之，**ALTX 不保證恆等於 alarm code**——可能是 `code` 或 `code + 空白 + message`。本 run 觀察到的 ALTX 皆為純 alarm code（`"Loader Tray Empty"`／`"SnFKCleanOut"`，無附加 Message）。
>
> **整合建議**：host 端若要以 ALTX 辨識警報，請 **以開頭的 code token（第一個空白前的字串）作為比對鍵**，而非整段 ALTX 字串，以免日後附加 Message 時比對失敗。

> **為什麼 ALID 用字串 hash、而不是固定編號？host 該以什麼為準？** 因為 ALID 由字串確定性地算出，可避免維護一張人工編號表；但這也代表 **ALID 本身對 host 不直觀**。整合時建議 host 端 **以 ALTX 的 leading code token 作為警報的辨識依據**（ALID 用於去重／配對 SET↔CLEAR 即可），因為 code token 才是人類可讀、語意明確的代碼。
>
> **S5F5/F6 + S5F7/F8 警報目錄查詢（2026-07-15 更新，commit `4ae0cce`）**：`AddAlarmList()`／`S5F6_ListAlarmData()` 過去為空 stub、只回空 `L,0`。現已改為從 `mapAlarmCodeList`（同一份 alarm SSOT，也餵 AlarmList.csv 與 S5F1）**即時輸出完整警報目錄**：
>
> - **S5F5**（host「List Alarm Request」）→ 設備回 **S5F6**「List Alarm Data」。
> - **S5F7**（host「List Enabled Alarm Request」）→ 設備回 **S5F8**。本機所有警報恆為 enabled，故 enabled list = 完整目錄（內容與 S5F6 相同）。
> - body 格式：`L[n] { L[3] { B ALCD, U4 ALID, A ALTX } }`，每筆 = 一個警報定義。`ALID` 用與 S5F1 **相同**的 `ComputeAlarmAlid()`（故上報警報的 ALID 必等於目錄中同一警報的 ALID，可交叉對位）；`ALCD` = 警報分類（`AlarmType & 0x7F`；目錄為「定義列」故 bit7 SET/CLEAR = 0）；`ALTX` = `code`（有 message 時為 `code + 空白 + message`，同 S5F1）。
> - **已知取捨**：S5F5／S5F7 body 內的 ALID 過濾清單**未實作**——設備一律回完整目錄（對常見的 `L,0` = 查全部等價；host 若送特定 ALID 子集需自行過濾）。
> - S5F5→S5F6／S5F7→S5F8 的**實測 round-trip log 待以 `HT160S_SECS_Simulator` 補跑**（本節不偽造 log；現況為程式碼已實作並雙建置 EXIT 0，上機/host 驗證 pending）。

**實際 case log 節錄（設備端 Equipment）**：

```
00:01:00.388  [SECS][TX] S5F1 Alarm ALID=4045923824 ALCD=128   (SET   "Loader Tray Empty")
00:01:00.388  [SECS][TX] S5F1 W=1 len=62 (sent)
00:01:00.431  [SECS][RX] S5F2 decoded rc=1 items=3
00:01:04.499  [SECS][TX] S5F1 Alarm ALID=4045923824 ALCD=0     (CLEAR "Loader Tray Empty")
00:01:05.056  [SECS][TX] S5F1 Alarm ALID=3891410149 ALCD=128   (SET   "SnFKCleanOut")
00:01:11.861  [SECS][TX] S5F1 Alarm ALID=3891410149 ALCD=0     (CLEAR "SnFKCleanOut")
```

**實際 case log 節錄（Host 端 Simulator）**：

```
[00:01:00] RX  S5F1W (sys=54) -> auto S5F2 ACKC5=0
[00:01:04] RX  S5F1W (sys=55) -> auto S5F2 ACKC5=0
[00:01:05] RX  S5F1W (sys=56) -> auto S5F2 ACKC5=0
[00:01:11] RX  S5F1W (sys=57) -> auto S5F2 ACKC5=0
```

> **對照讀**：上面 4 筆即兩個警報的 SET/CLEAR 配對——`Loader Tray Empty`（00:01:00 SET → 00:01:04 CLEAR）與 `SnFKCleanOut`（00:01:05 SET → 00:01:11 CLEAR）；後者 CLEAR 即本 run 的 clean out 結束點。

**FIELD TABLE — S5F1 Alarm Report（送出）**：

| Item | 型別 | 意義 | 範例值（本 run） |
|---|---|---|---|
| `ALCD` | B | Alarm Code byte：bit7(0x80=128)=SET、0x00=CLEAR | `128`（SET）／`0`（CLEAR） |
| `ALID` | U4 | Alarm ID，由 ALTX 字串 31-poly hash 而來，deterministic | `4045923824`（Loader Tray Empty）／`3891410149`（SnFKCleanOut） |
| `ALTX` | A | Alarm Text：`code`，或 `code + 空白 + message`（有附加訊息時）；本 run 皆為純 code | `"Loader Tray Empty"`／`"SnFKCleanOut"`（本 run 無附加 message；host 比對請取 leading code token） |
| `W`-bit | header | 要求回覆 | `1`（S5F1W） |
| `len` | bytes | S5F1 body 長度 | `62` |
| `sys` | header（模擬器序號） | 與 S6F11 共用同一遞增序號池 | `54`..`57`（本 run 4 筆警報） |

**FIELD TABLE — S5F2 回覆（收到）**：

| Item | 型別 | 意義 | 範例值（本 run） |
|---|---|---|---|
| `ACKC5` | B | Alarm Report ACK 回覆碼 | `0`（警報已收到，本 run 4 筆皆 0） |

---

### 3.6 連線結束（Separate）

**用途**：結束 HSMS session。正常情況下由任一端送 `Separate.req` 即可拆除連線。

**實際 case log 節錄（Host 端 Simulator；屬 cosmetic，約 run 窗後 ~4 分鐘）**：

```
[00:05:09] connection closed
[00:05:11] TX  Separate.req
[00:05:11] send error: [WinError 10038] (socket no longer valid -- equipment app closed first)
[00:05:11] disconnected
```

**說明**：teardown 時的 `WinError 10038` 為 **host-side cosmetic（host 端外觀層、無實質影響）**：本範例中設備 app 先關閉、socket 已失效，模擬器才送 `Separate.req`，所以送出時報 10038（socket 已無效）。

**FIELD TABLE — Separate / teardown**：

| Item | 型別 | 意義 | 範例值（本 run） |
|---|---|---|---|
| `Separate.req` | SType=9（HSMS header） | 結束 session（任一端可送） | host 端送出 @00:05:11 |
| `WinError 10038` | OS socket error | host 端 cosmetic：設備先關、socket 已失效 | `socket no longer valid -- equipment app closed first` |

> **為什麼這個 error 不是通訊缺陷？** 因為它純粹是「關閉順序」造成的：設備先離線，host 才嘗試送 Separate，自然送不出去。連線本身在整個 run 期間運作正常（見第 6 章的完整性核對）。正式產線中若由 host 先送 Separate、設備再關閉，就不會出現此訊息。它不影響任何已完成的交易，屬正常收尾。

---

## 4. 模擬輸入資料

本章列出本範例中 host 端「餵給」設備的輸入資料，供 host/EAP 工程師理解資料格式並自行構造測試輸入。

### 4.1 Host 宣告的 Lot 清單

`ht160s_presets.DEFAULT_SIMU_LOTS`（即 SET_LOT_INFO 送出的預設 5 個 lot）：

```
SIMU_LOT_A, SIMU_LOT_B, SIMU_LOT_C, SIMU_LOT_D, SIMU_LOT_E
```

### 4.2 Lot WebAPI mock（2D/Bin reconcile）

當 host 送出 **LOTSTART** 時，設備會去 pull 這份資料做 2D/Bin 對帳（reconcile）。本範例以 mock 檔案模擬：

檔案：`D:\AI_Area\Tool\HT160S_SECS_Simulator\code\lot_webapi_data.json`

- Top-level key：`"2DIDHistory"`
- 每個 lot 欄位：`LOTID`、`Substage`、`ProductCode`、`ICIInfo[]{ QRCodeID, RetestCode, HBin, SBin, DiePass }`

**JSON 結構（shape）**：
```
{
  "2DIDHistory": [
    {
      "LOTID": "SIMU_LOT_A",
      "Substage": "BI1",
      "ProductCode": "SIMU/DEVICE-A",
      "ICIInfo": [
        { "QRCodeID": "...", "RetestCode": "R0", "HBin": "1", "SBin": "1", "DiePass": "1" },
        ...
      ]
    },
    ...
  ]
}
```

**ICIInfo 欄位 FIELD TABLE**：

| 欄位 | 型別 | 意義 |
|---|---|---|
| `QRCodeID` | string | 該顆 IC 的 2D/QR 碼 ID |
| `RetestCode` | string | 重測碼（如 R0 / R1） |
| `HBin` | string | Hardware Bin |
| `SBin` | string | Software Bin |
| `DiePass` | string | Die 通過判定（值為字串 "0"／"1"） |

**範例 row（SIMU_LOT_A，Substage `BI1`，ProductCode `SIMU/DEVICE-A`）**：

| QRCodeID | RetestCode | HBin | SBin | DiePass |
|---|---|---|---|---|
| SIMU_A_0001 | R0 | 1 | 1 | 1 |
| SIMU_A_0002 | R0 | 1 | 1 | 1 |
| SIMU_A_0003 | R0 | 2 | 2 | 1 |
| SIMU_A_0004 | R1 | 3 | 3 | 1 |
| SIMU_A_0005 | R0 | 4 | 5 | 0 |

> **真實客戶格式 lot（real-customer-format）**：此檔同時保留真實客戶格式 lot `A5921.RCS.TEST99` / `TEST88`（兩者 ProductCode 皆為同一組單一字串 `MT3781Q/ZAHJA32-ETTTT-H`，含長 QRCodeID 如 `MT3781Q-ZAHJA32-EMFMT-H_N8R124.LR_011`），供真實格式測試。host/EAP 對接時應同時驗證短格式（SIMU_*）與長格式（真實客戶）兩種 QRCodeID。

### 4.3 WebAPI endpoint

設備於 LOTSTART 時對每個 lot 發出的 HTTP 請求形式：

```
GET http://127.0.0.1:8160/lot/<LOTID>  ->  HTTP 200 JSON
```

**本 run 證據（回應大小）**：

| Lot | bytes |
|---|---|
| A | 951 |
| B | 633 |
| C | 792 |
| D | 474 |
| E | 633 |

> 23:59:26 全 5 lot HTTP=200、ok=1（對帳成功）。

### 4.4 S2F41 builder bodies（host 輸入格式）

S2F41 body 由 `ht160s_presets.py` 的 builders 建構，可作為 host 端構造輸入時的參照格式：

| Builder | RCMD | body |
|---|---|---|
| `_set_lot_info` | SET_LOT_INFO | `L[2]{ A "SET_LOT_INFO", L[1]{ L[2]{ A "LOTID", A "lot1,lot2,..." } } }`（2026-08-20 起＝客戶正式格式） |
| `_set_lot_info_flat` | SET_LOT_INFO | `L[2]{ A "SET_LOT_INFO", L[n]{ A lotID } }`（舊平鋪形狀，preset `S2F41 SET_LOT_INFO (flat)`，回歸用） |
| `_lot_start` | LOTSTART | `L[2]{ A "LOTSTART", L[n]{ A lotID } }` |
| `_start_agv` | START_AGV | `L[2]{ A "START_AGV", L[n]{ L[2]{A station, A "Action"} [, L[2]{A "LoaderTrayCount", A n}] } }` |
| `_start` | START | `L[2]{ A "START", L[0] }` |

---

## 5. ACK / HCACK 代碼速查表

本章彙整本手冊出現的所有回覆碼，方便對接時快速查詢「某個回覆碼代表什麼」。

**HCACK（S2F42，Host Command Acknowledge）**：

| 值 | 意義 | 本 run 出現 |
|---|---|---|
| 0 | OK | SET_LOT_INFO、LOTSTART、initial START(23:59:27)、START_AGV(×6) |
| 1 | command does not exist **或 body/list 格式錯誤**（unknown command 走此碼；亦為預設值，outer L[2] 解析失敗、內層 L[n] 格式錯誤、CP pair 非 list 皆回 1） | （本 run 未出現；舊文件誤稱 START_AGV/START 回 1，已更正） |
| 2 | cannot perform now / param | （本 run 未出現） |

> **⚠ `START_AGV` 有專屬的值域，勿套用上表推論**：自 2026-08-17 起，`START_AGV` 中**不認得的 CP 名稱不再回 `2`，而是忽略該 CP 並維持 `HCACK=0`**（與 HT9045 一致）。其 `2` 只保留給「AMR 功能關閉」與「CP 清單為空」兩種整包退回的情形。完整值域見 3.4。
| 4 | busy（machine already running **OR** ICs still inside） | resume-START(×6)，by design honest interlock（見 3.4） |

**其他 ACK 代碼**：

| 代碼 | 訊息 | 標準值域 | 本 run 值 | 說明 |
|---|---|---|---|---|
| ACKC5 | S5F2（Alarm ACK） | 0=accepted / >0=error | 0 | 警報已收到 |
| ACKC6 | S6F12（Event Report ACK） | 0=accepted / >0=error | 0 | 事件已收到 |
| COMMACK | S1F14（Establish Communications Request Acknowledge；S1F13→S1F14） | 0=accepted / 1=denied | `0`（見下註） | 現行韌體已實作 S1F13→S1F14：COMMACK 於程式中恆設為 `0`（always accept），body 為 `L[2]{ B:COMMACK=0, L[2]{ A:MDLN="HT-160S", A:SOFTREV="1.0.0.0" } }`（`uHGemHT160.cpp:1007-1023`，程式碼註記 20260625）。本來源 Run（2026-06-26）未發出 S1F13，故無逐字 log；此值係由程式碼確定，非臆造。 |
| EAC | S2F38（Enable/Disable Event Report ACK，Equipment Acknowledge Code） | 0=accepted / 1=denied / 2=at least one CEID 不存在 / 3=at least one CEID 已 enabled | — | 本文件來源 Run evidence 未涵蓋具體值，故僅列名不臆造 |

> 註：COMMACK / EAC 屬標準 GEM 代碼，其「標準值域」欄為 SEMI E5/E30 通用定義（供 host 對接參考）。其中 COMMACK 之數值已可由程式碼確定——現行韌體 S1F13→S1F14 已實作、COMMACK 恆為 `0`（見上表註，`uHGemHT160.cpp:1007-1023`）；EAC（S2F38）則因本來源 Run evidence 未提供實際數值，依接地規則「本 run 值」不臆造。

---

## 6. 完整通訊時間軸

本章把整個 run（從 `23:59:08` 連線到 `00:01:11` clean out 結束）壓縮成一張時間軸，並附上 end-to-end 完整性核對——這正是整合驗收時「如何證明訊息沒有漏、沒有重複」的方法。

| 時間 | 事件 | 重點 |
|---|---|---|
| 23:59:08.603 | TCP up，awaiting Select | 設備撥入 |
| 23:59:08.862 | Select.req → Select.rsp | SELECTED |
| 23:59:18 起 | Linktest 週期 | 約每 10s，全程皆答 |
| 23:59:25 | S2F41 SET_LOT_INFO | HCACK=0，Lots=5，sys=20558 |
| 23:59:26 | S2F41 LOTSTART | HCACK=0；WebAPI 5 lot HTTP=200 ok=1；sys=20559 |
| 23:59:27 | S2F41 START | HCACK=0（machine idle → started）；sys=20560 |
| 23:59:30 | S6F11 CEID=136 | S6F12 ACKC6=0；sys=1 |
| 23:59:47 | AGV cycle | P2 Empty |
| 00:00:13 | AGV cycle | P1 Loader（完整握手）；resume START HCACK=4 |
| 00:00:18 | AGV cycle | P2 Empty |
| 00:00:41 | AGV cycle + CEID 35 | P4 AUTO1；272(len=86)+35(len=144) 同時（兩個長度皆為 **2026-06-26 當時**的 body 大小；現行韌體 272 因加掛 report 6 而更大、35 因 Report 1 縮編為 1 個 SV 而更小，見 3.3.1） |
| 00:00:46 | AGV cycle | P1 Loader |
| 00:00:51 | AGV cycle | P2 Empty |
| 00:01:00 | S5F1 SET | Loader Tray Empty，ALID=4045923824，ALCD=128 |
| 00:01:04 | S5F1 CLEAR | Loader Tray Empty，ALCD=0 |
| 00:01:05 | S5F1 SET | SnFKCleanOut，ALID=3891410149，ALCD=128 |
| 00:01:11 | S5F1 CLEAR | SnFKCleanOut，ALCD=0（clean out 結束） |

**End-to-End 完整性（Integrity）**：以下計數可在兩端 log 交叉核對，是整合驗收的關鍵證據——序號連續、TX/RX 對等，即代表通訊無漏訊、無重送。

| 維度 | 計數 / 驗證 |
|---|---|
| S6F11（生產／AGV 事件） | 設備 TX **53** = 設備 RX S6F12 **53** = 模擬器 RX S6F11W sys **1..53**（連續，無 gap/dup） |
| S6F11 CEID 內訳 | 136×13 + 137×7 + 138×4 + 140×4 + 141×3 + 142×3（合計 34 Auto 卸盤）＋ 272×6 + 273×6 + 274×6（合計 18 AGV）＋ 35×1 = **53** |
| S2F41（RCMD 下達） | **15** RCMDs（3 trio + 6 START_AGV + 6 resume-START） |
| S2F42（HCACK 回覆） | sys **20558..20572** 嚴格 +1（共 15 筆，對應 15 個 RCMD） |
| 警報 S5F1 | **4** 筆 S5F1 ↔ 模擬器 sys **54..57** ↔ EventLog（00:01:00 Loader Tray Empty / 00:01:05 SnFKCleanOut / 00:01:11 PAUSE+doors） |
| S5F2（ACKC5 回覆） | 4 筆，全 `0` |
| WebAPI | 23:59:26 全 5 lot HTTP=200 ok=1（A=951 / B=633 / C=792 / D=474 / E=633 bytes） |

> **註**：S6F11 與 S5F1 共用同一遞增序號池（**1..57**）：前 53 為 S6F11（53 筆生產／AGV 事件），54..57 為 S5F1（4 筆警報）。S2F42 另有獨立序號池 20558..20572。兩條序號鏈各自連續、無 gap/dup，是無漏訊的證據。整合驗收時，host 端只要核對：(1) S6F11 TX 數 = S6F12 RX 數；(2) sys 序號連續無跳號；(3) 每筆 S5F1 都有對應的 S5F2 ACKC5=0，即可判定通訊完整。

---

## 7. 重現步驟（以 SECS Host Simulator 重現）

本章說明如何用 SECS Host Simulator 重現上述完整流程，供整合工程師在自己的環境中對接與驗證。步驟順序對應第 2 章的 Mermaid 圖。

1. **啟動模擬器（PASSIVE listen）**：模擬器於 `127.0.0.1:5098` 監聽（device=1），等待設備連入。
   > 提醒：本重現環境刻意讓模擬器當監聽方、設備撥入（與正式產線方向相反，見 1.2）；訊息層角色不變。
2. **設備撥入**：HT160S（EQUIPMENT，**本重現環境設 `ActiveMode=1`**，DeviceID=0）連入 port 5098，完成 `Select.req → Select.rsp`（SELECTED）；之後約每 10s Linktest 維持鏈路。
   > 2026-08-03 更正：此步原註明 `ActiveMode=0`，與「設備主動撥入」矛盾 —— `ActiveMode=0` 是設備**監聽**（正式產線用，現場實機即為 0，見 1.2），要讓設備撥入必須設 `ActiveMode=1`。§1.2 表格描述的是**正式產線**方向，本章是刻意反向的實驗室環境。
3. **送 Lot 設定 trio**：依序送（順序不可顛倒，理由見 3.2）
   - `S2F41 SET_LOT_INFO`（預設 `SIMU_LOT_A..E`）→ 期望 `S2F42 HCACK=0`
   - `S2F41 LOTSTART` → 觸發 Lot WebAPI pull（`GET http://127.0.0.1:8160/lot/<LOTID>`，期望 HTTP=200、ok=1）→ `HCACK=0`
   - `S2F41 START`（機台 idle）→ `HCACK=0`
4. **啟用 Auto-AGV swimlane**：開啟模擬器的自動 AGV 回應規則：
   - 收 `272` → 自動 `START_AGV(station)`（期望 `HCACK=0`）
   - 收 `273` → 僅 log
   - 收 `274` → 自動 `START` resume（期望 `HCACK=4`，因機台已在運行——這是正常的，見 3.4）
5. **觀察並核對**：
   - S6F11 生產事件（Auto 卸盤）→ 自動 S6F12 ACKC6=0。⚠ **現行韌體要看的號碼是 `136 / 137 / 138 / 145 / 146 / 147`** —— 本章其他各處出現的 `136–142` 是 2026-06-26 來源 run 的舊編號，Auto4/5/6 已於 commit `bf9d048` 由 `140/141/142` 改為 `145/146/147`（見 3.3 的說明），重現時不會再看到 **140 與 142**。⚠ **`141` 仍會出現，但語意不同** —— 2026-08-03 起 `141` 被重新指派為 `GEM Control State Change`（控制狀態每次變更即發），2026-08-04 實測連線當下即收到，詳見 3.3 的號碼說明。
   - 完整 AGV cycle（272→START_AGV→273→274→resume START）
   - CEID 35（Auto1 car full）與 272 同時刻雙發（**本 run 為 len=144 + len=86**；現行韌體 35 因 Report 1 只剩 1 個 SV 而遠小於 144、272 因加掛 report 6 而大於 86，故請以「兩者同時刻雙發」為驗收點，勿以位元組長度比對，見 3.3.1）
   - 警報 S5F1 SET/CLEAR（ALID hash、ALTX）→ 自動 S5F2 ACKC5=0
6. **驗收完整性**：比對設備端 `SECSGEM_TextLog_HH.txt` 與 host 端 `secs_host_YYYYMMDD.log`：
   - S6F11 TX 數 = S6F12 RX 數（本 run 為 53）；
   - sys 序號連續無跳號（S6F11+S5F1 池 1..57；S2F42 池 20558..20572）；
   - 每筆 S5F1 都有對應 S5F2 ACKC5=0。
   三者皆成立即代表通訊無漏訊（即第 6 章的完整性核對）。

---

## 附錄：名詞 / 縮寫

| 縮寫 | 全稱 / 說明 |
|---|---|
| HSMS-SS | High-Speed SECS Message Services – Single Session（SEMI E37）；over TCP，負責傳輸層 |
| SECS-II | SEMI Equipment Communications Standard part 2（SEMI E5）；負責訊息編碼 |
| GEM | Generic Equipment Model（SEMI E30）；定義設備行為語意 |
| EQUIPMENT / HOST | 設備端（此處為 HT160S）／主控端（廠端自動化系統 Host/EAP） |
| ACTIVE / PASSIVE | HSMS 連線角色：ACTIVE 主動發起 Select.req，PASSIVE 監聽等待連入 |
| SType | HSMS 控制訊息類型碼（Select=1/2、Linktest=5/6、Separate=9） |
| CEID | Collection Event ID；事件識別碼（如 136、272、35） |
| RPTID | Report ID；report 定義識別碼 |
| SVID / SV | Status Variable ID／Status Variable；狀態變數識別碼（如 38219）／狀態變數 |
| DataID | Data ID（S6F11 欄位）；對 host 無語意，host 依 CEID 分派 |
| ALID | Alarm ID；本機由 ALTX 字串 31-poly hash 而來的 U4 |
| ALCD | Alarm Code byte；bit7(0x80=128)=SET、0x00=CLEAR |
| ALTX | Alarm Text；`code` 或 `code + 空白 + message`（有附加訊息時）；建議 host 以 leading code token 辨識警報 |
| HCACK | Host Command Acknowledge（S2F42 回覆碼，0/1/2/4） |
| ACKC5 | Alarm Report ACK（S5F2 回覆碼） |
| ACKC6 | Event Report ACK（S6F12 回覆碼） |
| COMMACK | Establish Communications Request Acknowledge（S1F14 回覆碼；S1F13→S1F14，0=accepted/1=denied） |
| EAC | Equipment Acknowledge Code（S2F38 Enable/Disable Event Report ACK） |
| RCMD | Remote Command（S2F41 命令名稱）。現行設備 S2F41 dispatch 接受（2026-07-30 現況，共 18 支）：`SET_LOT_INFO`、`LOTSTART`、`CLEAR_LOT_INFO`、`START`、`START_AGV`、`STOP`、`PAUSE`、`HALT`、`HOME`、`ONE_CYCLE`、`CLEAN_OUT`、`CLEARCOUNT`、`CLEAN_AUTO_SORT_COUNT`、`ENERGY_SAVING`、`PP_SIGNALTOWER`、`PP_MUSIC`、`ONLINE_REMOTE`/`ONLINE`、`ONLINE_LOCAL`；未列於此者回 `HCACK=1`（unknown command）。本手冊來源 run 僅觸發 trio + START_AGV，其餘命令未於本 run 出現（`SecsGem\uHGemHT160.cpp` 的 `S2F42_Host_Command_Acknowledge`） |
| CPNAME / CPVAL | Command Parameter Name / Value（S2F41 參數名稱／值） |
| AGV / AMR | Automated Guided Vehicle / Autonomous Mobile Robot；自動物料搬運車 |
| EAP | Equipment Automation Program；客戶端 host 自動化程式 |
| W-bit | Wait-bit；訊息要求對方回覆（W=1）或不要求（W=0，偶函數回覆） |
| SECS-II 型別 | `L[n]`=List；`A`=ASCII；`B`=Binary(1 byte)；`U4`=4-byte unsigned int |
