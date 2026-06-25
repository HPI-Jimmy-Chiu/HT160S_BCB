<div style="border-top: 10px solid #b00020; border-bottom: 1px solid #d9d9d9; padding: 18px 0 14px 0; margin-bottom: 22px;">
  <div style="color: #b00020; font-weight: 700; letter-spacing: 0;">鴻勁精密 Hon Precision</div>
  <h1 style="margin: 6px 0 4px 0; color: #2c3e50;">HT160S E87 / AGV 通訊規範草案</h1>
  <div style="color: #626262;">文件狀態：客戶確認版草案｜日期：2026/05/27｜範圍：通訊規格與流程確認，不含程式修改</div>
</div>

## 1. 文件目的

本文件整理 HT160S 導入 E87 / AGV 對接時，Handler、EAP、AGV 三方的通訊流程、Remote Command、Event Report、SVID 對應，以及 Auto1-Auto6 的延伸規則。

客戶前面提供的圖式，正式名稱建議使用：**泳道式時序圖**。英文可寫為 **Swimlane Sequence Diagram**；若放在規格文件標題，可寫為 **SECS/GEM E87 Carrier Handoff Swimlane Sequence Diagram**。

## 2. 通訊設計原則

| 原則 | 說明 |
|---|---|
| AGV 動作與機台 START 分離 | `START_AGV` 只代表 AGV 交接動作；`START` 才是開始或繼續生產。 |
| 補料與滿盤叫車使用事件通知 | Handler 用 `S6F11 AGVSupplement` 通知 EAP 需要 AGV 支援。 |
| Ready 與 Finish 分開回報 | Handler 機構到位後送 `AGVLDUnLDStatus`；Sensor 確認完成後送 `AGVLDUnLDFinish`。 |
| P 位元代表站點 | P1-P9 對應 HT160S 的 Loader、EmptyTray、ColorTray、Auto1-Auto6。 |
| 完成判定以 Sensor 為準 | 上料完成、下料完成，都以實際感測器狀態作為 Finish 條件。 |
| Auto4-Auto6 使用順移延伸 | Auto4-Auto6 的 Remote Command 與 P mapping 可由 Auto3 同模板延伸，SVID 則依既有區段避開已使用號碼。 |

## 3. HT160S 站點 P Mapping

| P index | HT160S 站點 | 方向 | 用途 |
|---:|---|---|---|
| P1 | Loader | AGV → Handler | Loader 上料 / device tray supply |
| P2 | EmptyTray | AGV → Handler | EmptyTray 放空盤 / empty tray supply |
| P3 | ColorTray | AGV → Handler | ColorTray 放身分 Tray / identity or reject tray supply |
| P4 | Auto1 | Handler → AGV | Auto1 滿盤取走 |
| P5 | Auto2 | Handler → AGV | Auto2 滿盤取走 |
| P6 | Auto3 | Handler → AGV | Auto3 滿盤取走 |
| P7 | Auto4 | Handler → AGV | Auto4 滿盤取走 |
| P8 | Auto5 | Handler → AGV | Auto5 滿盤取走 |
| P9 | Auto6 | Handler → AGV | Auto6 滿盤取走 |

P bitmap payload 建議格式：

```text
P1:1,P2:0,P3:0,P4:0,P5:0,P6:0,P7:0,P8:0,P9:0
```

單次 AGV 任務建議只開一個目標 P 位元。若客戶希望一筆事件同時派多站點，需另外確認 AGV 派車策略與 Handler 動作互鎖。

## 4. Remote Command 規格

### 4.1 START_AGV

`START_AGV` 用於通知 Handler 針對指定站點執行 AGV 交接準備。

| Command | CP Name | CP Value | HT160S 動作 |
|---|---|---|---|
| `START_AGV` | `Loader` | `Action` | 啟動 P1 Loader 上料準備 |
| `START_AGV` | `Empty` | `Action` | 啟動 P2 EmptyTray 補空盤準備 |
| `START_AGV` | `Color` | `Action` | 啟動 P3 ColorTray 補身分 Tray 準備 |
| `START_AGV` | `AUTO1` | `Action` | 啟動 P4 Auto1 滿盤交接準備 |
| `START_AGV` | `AUTO2` | `Action` | 啟動 P5 Auto2 滿盤交接準備 |
| `START_AGV` | `AUTO3` | `Action` | 啟動 P6 Auto3 滿盤交接準備 |
| `START_AGV` | `AUTO4` | `Action` | 啟動 P7 Auto4 滿盤交接準備 |
| `START_AGV` | `AUTO5` | `Action` | 啟動 P8 Auto5 滿盤交接準備 |
| `START_AGV` | `AUTO6` | `Action` | 啟動 P9 Auto6 滿盤交接準備 |
| `START_AGV` | `LoaderTrayCount` | integer | 設定 Loader 預計補入 Tray 數量 |

### 4.2 START

`START` 用於開始或繼續生產，不與 `START_AGV` 合併。建議 EAP 在收到對應站點的 `AGVLDUnLDFinish` 後，再依 Lot 狀態與機台狀態送出 `START`。

## 5. Event Report 規格

| CEID | Event Name | 對應 SVID | 說明 |
|---:|---|---|---|
| 272 | `AGVSupplement` | `38219 Supplement Bin` | Handler 通知 EAP 需要 AGV 支援，例如上料不足或 Auto 滿盤。 |
| 273 | `AGVLDUnLDStatus` | `38220 LD UnLD Check AGV` | Handler 機構已準備完成，AGV 可開始上料或下料。 |
| 274 | `AGVLDUnLDFinish` | `38221 LD UnLD Finish AGV` | Sensor 已確認上料或下料完成。 |
| 275 | `AGVLdID` | Carrier ID SVID | Handler 回報目前 Carrier ID。 |

範例 payload：

```text
Loader 缺料叫車：
CEID 272 AGVSupplement
SVID38219 = P1:1,P2:0,P3:0,P4:0,P5:0,P6:0,P7:0,P8:0,P9:0

Auto1 已準備可下料：
CEID 273 AGVLDUnLDStatus
SVID38220 = P1:0,P2:0,P3:0,P4:1,P5:0,P6:0,P7:0,P8:0,P9:0

Auto1 下料完成：
CEID 274 AGVLDUnLDFinish
SVID38221 = P1:0,P2:0,P3:0,P4:1,P5:0,P6:0,P7:0,P8:0,P9:0
```

## 6. SVID 對應表

設計原則：保留既有 SVID 用途，Auto4-Auto6 以新增欄位延伸。Tray Count、Device Count、Bin Setting 不互相插號，避免 host parser 混淆。

| SVID | Type | Name | 用途 |
|---:|---|---|---|
| 38202 | ASCII | `Load Port Carrier ID` | P1 Loader Carrier ID |
| 38203 | ASCII | `EmptyTray Carrier ID` | P2 EmptyTray Carrier ID，待客戶確認是否需要 |
| 38204 | ASCII | `ColorTray Carrier ID` | P3 ColorTray Carrier ID，待客戶確認是否需要 |
| 38205 | ASCII | `Auto1 carrier ID` | P4 Auto1 Carrier ID |
| 38206 | ASCII | `Auto2 carrier ID` | P5 Auto2 Carrier ID |
| 38207 | ASCII | `Auto3 carrier ID` | P6 Auto3 Carrier ID |
| 38208 | ASCII | `Auto4 carrier ID` | P7 Auto4 Carrier ID，Auto3 carrier ID 順移延伸 |
| 38209 | ASCII | `Auto5 carrier ID` | P8 Auto5 Carrier ID，Auto3 carrier ID 順移延伸 |
| 38210 | ASCII | `Auto6 carrier ID` | P9 Auto6 Carrier ID，Auto3 carrier ID 順移延伸 |
| 38219 | ASCII | `Supplement Bin` | `AGVSupplement` 的 P1-P9 bitmap |
| 38220 | ASCII | `LD UnLD Check AGV` | `AGVLDUnLDStatus` 的 P1-P9 bitmap |
| 38221 | ASCII | `LD UnLD Finish AGV` | `AGVLDUnLDFinish` 的 P1-P9 bitmap |
| 38222 | INT4 | `AMR Loader Tray Count` | P1 tray count |
| 38223 | INT4 | `AMR Empty Tray Count` | P2 tray count |
| 38224 | INT4 | `AMR Color Tray Count` | P3 tray count |
| 38225 | INT4 | `AMR Auto1 Tray Count` | P4 tray count |
| 38226 | INT4 | `AMR Auto2 Tray Count` | P5 tray count |
| 38227 | INT4 | `AMR Auto3 Tray Count` | P6 tray count |
| 38228 | INT4 | `AMR Loader Device Count` | P1 device count |
| 38229 | INT4 | `AMR Empty Device Count` | P2 device count |
| 38230 | INT4 | `AMR Color Device Count` | P3 device count |
| 38231 | INT4 | `AMR Auto1 Device Count` | P4 device count |
| 38232 | INT4 | `AMR Auto2 Device Count` | P5 device count |
| 38233 | INT4 | `AMR Auto3 Device Count` | P6 device count |
| 38234 | ASCII | `AMR Auto1 Bin Setting` | P4 bin setting |
| 38235 | ASCII | `AMR Auto2 Bin Setting` | P5 bin setting |
| 38236 | ASCII | `AMR Auto3 Bin Setting` | P6 bin setting |
| 38237 | INT4 | `AMR Auto4 Tray Count` | P7 tray count，新增 |
| 38238 | INT4 | `AMR Auto5 Tray Count` | P8 tray count，新增 |
| 38239 | INT4 | `AMR Auto6 Tray Count` | P9 tray count，新增 |
| 38240 | INT4 | `AMR Auto4 Device Count` | P7 device count，新增 |
| 38241 | INT4 | `AMR Auto5 Device Count` | P8 device count，新增 |
| 38242 | INT4 | `AMR Auto6 Device Count` | P9 device count，新增 |
| 38243 | ASCII | `AMR Auto4 Bin Setting` | P7 bin setting，新增 |
| 38244 | ASCII | `AMR Auto5 Bin Setting` | P8 bin setting，新增 |
| 38245 | ASCII | `AMR Auto6 Bin Setting` | P9 bin setting，新增 |

## 7. Auto4-Auto6 順移延伸評估

結論：**可以延伸，而且建議以 AutoNo / PIndex 資料表化處理，不建議手動複製三段 Auto3 程式。**

| 項目 | 順移方式 | 注意事項 |
|---|---|---|
| Remote Command | `AUTO3/Action` 模板延伸為 `AUTO4/Action`、`AUTO5/Action`、`AUTO6/Action` | Host CP name 增加 `AUTO4`、`AUTO5`、`AUTO6`。 |
| P mapping | Auto1=P4，Auto2=P5，Auto3=P6，因此 Auto4=P7、Auto5=P8、Auto6=P9 | 可用公式 `PIndex = AutoNo + 3`。 |
| Carrier ID SVID | Auto3 `38207` 順移到 Auto4 `38208`、Auto5 `38209`、Auto6 `38210` | 38203/38204 建議保留給 EmptyTray / ColorTray。 |
| Tray Count SVID | Auto4-Auto6 不可直接接在 38227 後面 | 38228-38236 已保留給 Device Count / Bin Setting，建議 Auto4-Auto6 Tray Count 使用 38237-38239。 |
| Device Count SVID | Auto3 Device Count 後新增 Auto4-Auto6 | 建議 38240-38242。 |
| Bin Setting SVID | Auto3 Bin Setting 後新增 Auto4-Auto6 | 建議 38243-38245。 |
| Handler 內部結構 | HT160S AutoBin 已是 6 站共用同一套動作模型 | 後續程式可用 `AutoNo=1..6` 的迴圈或表格做 dispatch。 |

工程備註：HT160S AutoBin 現有架構已具備六站共用模組概念，包含 `AUTO_BIN_COUNT = 6` 與 `g_AutoBin[0..5]`。因此 E87/AGV 實作時，Auto4-Auto6 主要是通訊 mapping、SVID 註冊、事件 payload 與站點 dispatch 的延伸，不需要重新設計 AutoBin 動作流程。

## 8. 泳道式時序圖

### 8.1 Loading Process

| Step | Handler | EAP | AGV |
|---:|---|---|---|
| 1 | Handler 狀態可接受上料，必要時保持 HALT | 讀取 Handler 狀態 |  |
| 2 |  | 下達 setup / recipe 相關作業，必要時執行 `DOWNLOAD_RECIPE_BY_FTP` |  |
| 3 |  | 指派 AGV 到 Loader 站 | 接收任務 |
| 4 |  |  | 抵達指定位置 |
| 5 | 接收 `START_AGV(Loader, Action)` | 送出 `S2F41 START_AGV` |  |
| 6 | 機構鎖定，到位後送 `AGVLDUnLDStatus`，SVID38220 P1=1 | 接收 Ready |  |
| 7 |  | 通知可上料 | 開始上料 |
| 8 | Sensor 確認上料完成，送 `AGVLDUnLDFinish`，SVID38221 P1=1 | 接收 Finish |  |
| 9 | 接收 `START` 後開始或繼續生產 | 視 Lot 狀態送出 `S2F41 START` | 離開 Handler station |

### 8.2 AutoBin Full Devices Process

| Step | Handler | EAP | AGV |
|---:|---|---|---|
| 1 | AutoX 滿盤，送 `AGVSupplement`，SVID38219 目標 P=1 | 接收滿盤需求 |  |
| 2 |  | 指派 AGV 到 AutoX | 接收任務 |
| 3 |  |  | 抵達指定 AutoX 位置 |
| 4 | 接收 `START_AGV(AUTOx, Action)` | 送出 `S2F41 START_AGV` |  |
| 5 | AutoX 機構到位，送 `AGVLDUnLDStatus`，SVID38220 目標 P=1 | 接收 Ready |  |
| 6 |  | 通知可下料 | 開始取走滿盤 |
| 7 | Sensor 確認 tray 被取走，送 `AGVLDUnLDFinish`，SVID38221 目標 P=1 | 接收 Finish |  |
| 8 | 可進入下一輪生產或等待補空盤 | 依流程送 `START` 或下一個任務 | 離開 Handler station |

### 8.3 Loader / EmptyTray / ColorTray Shortage Process

| Step | Handler | EAP | AGV |
|---:|---|---|---|
| 1 | 偵測 Loader / EmptyTray / ColorTray 需要補料，送 `AGVSupplement` | 接收補料需求 |  |
| 2 |  | 指派 AGV 到目標站點 | 接收任務 |
| 3 |  |  | 抵達指定位置 |
| 4 | 接收 `START_AGV(Loader/Empty/Color, Action)` | 送出 `S2F41 START_AGV` |  |
| 5 | 目標站機構到位，送 `AGVLDUnLDStatus` | 接收 Ready |  |
| 6 |  | 通知可上料 | 開始補料 |
| 7 | Sensor 確認補料完成，送 `AGVLDUnLDFinish` | 接收 Finish |  |
| 8 | 條件滿足後可繼續生產 | 視流程送 `START` | 離開 Handler station |

### 8.4 Finish Test / Unloading Process

| Step | Handler | EAP | AGV |
|---:|---|---|---|
| 1 | Finish Test，進入可交接狀態 | 讀取 Handler 狀態 |  |
| 2 |  | 指派 AGV 到出料站 | 接收任務 |
| 3 |  |  | 抵達指定位置 |
| 4 | 接收 `START_AGV(AUTOx, Action)` | 送出 `S2F41 START_AGV` |  |
| 5 | 出料站機構到位，送 `AGVLDUnLDStatus` | 接收 Ready |  |
| 6 |  | 通知可下料 | 開始取走 tray / devices |
| 7 | Sensor 確認取走完成，送 `AGVLDUnLDFinish` | 接收 Finish |  |
| 8 | 交接完成，維持或切回指定狀態 | 記錄任務完成 | 離開 Handler station |

## 9. 完成條件

| 動作 | Ready 條件 | Finish 條件 |
|---|---|---|
| Loader 上料 | Loader 交接機構已到位且可接受 AGV 動作 | Loader sensor 確認 tray / device 到位 |
| EmptyTray 補空盤 | EmptyTray 交接機構已到位且可接受 AGV 動作 | EmptyTray sensor 確認空盤到位 |
| ColorTray 補身分 Tray | ColorTray 交接機構已到位且可接受 AGV 動作 | ColorTray sensor 確認 tray 到位 |
| Auto1-Auto6 下料 | AutoX 交接機構已到位且可接受 AGV 動作 | AutoX sensor 確認 tray / device 已被取走 |
| 生產繼續 | Lot、Run mode、Alarm 狀態允許 | EAP 送 `START` 或 Handler 依約定流程恢復 |

## 10. 客戶確認項目

| 項目 | 建議 | 需要確認原因 |
|---|---|---|
| P1-P9 bitmap | 採用 | HT160S 需要涵蓋 Loader、EmptyTray、ColorTray、Auto1-Auto6。 |
| Auto4-Auto6 SVID | 採用 38237-38245 | 可避開已使用欄位，並保持 Tray Count / Device Count / Bin Setting 分區清楚。 |
| EmptyTray / ColorTray Carrier ID | 待確認 | 若客戶需要追蹤 P2/P3 Carrier，建議保留 38203/38204。 |
| `START_AGV` 與 `START` 是否分離 | 建議分離 | 可避免 AGV 交接尚未完成就啟動生產。 |
| 是否要求完整 SEMI E87 Carrier State Model | 待確認 | 本文件目前定義為 SECS/GEM event + SVID 對接流程；若需完整 carrier object state，需要另列規格。 |
| 單次事件是否允許多個 P=1 | 建議單站 | 對 AGV 派車與 Handler 互鎖最單純。 |

## 11. 結論

HT160S E87 / AGV 對接建議採用 P1-P9 站點 bitmap，並以 `AGVSupplement`、`START_AGV`、`AGVLDUnLDStatus`、`AGVLDUnLDFinish`、`START` 形成清楚的叫車、準備、完成與生產控制分工。

Auto4-Auto6 可由 Auto3 的通訊與站點動作模板順移延伸；但 SVID 配置需避開既有 Device Count / Bin Setting 區段，因此建議將 Auto4-Auto6 的 Tray Count、Device Count、Bin Setting 追加到 38237-38245。

---

> 以下第 12-15 節為 2026/06/25 補充規範。第 1-11 節維持原內容不變；本補充章節釐清交接 timeout 與防呆、模擬模式自動通過、`Action` CP 值語意，以及 State Record 記錄需求，避免 AGV 缺席或 host 中斷流程時造成生產硬鎖死。

## 12. 交接 Timeout 與 AGV 缺席防呆

### 12.1 問題背景

每一站的 AMR infeed lock（或 Auto 滿盤 lock）由 host 的 `S2F41 START_AGV` 進入 `AGV_PREP` 時設定，僅在 handshake 完成 `PREP → READY (CEID273) → FINISH (CEID274)` 後才釋放。若 AGV 缺席、中途放棄，或 host 送出 `START_AGV` 後一直沒有完成 handshake，該站的 lock 會被永久 latch（原本只有 HOME 才能清除），凍結對應 feeder 的進料迴圈，進而拖垮整條產線。因此規範雙方都必須有 timeout 與明確的取消轉移。

### 12.2 設備端 Timeout（Equipment-side，必備）

| 項目 | 規範 |
|---|---|
| 監看狀態 | `AGV_PREP`、`AGV_READY` 兩個 handshake 狀態各自計時。 |
| 最大停留時間 | 每個狀態設定一個 MAX dwell（建議以秒為單位，預設約 120 秒，需寬鬆到正常運送中的 AGV 不會被誤判）。 |
| 逾時動作 | 設備端自動：(1) 釋放該站 AMR lock；(2) 將該站 handshake 重置為 `AGV_IDLE`；(3) 重新 arm（下一輪 `PollAndCall` 可重新叫車）。 |
| 計時重置 | handshake 狀態一旦推進（PREP→READY 或 READY→IDLE）或回到 IDLE，計時歸零。僅在「狀態這個 tick 未推進」時累加，避免誤殺進行中的交接。 |
| 連線中斷 | link drop（HSMS 非 SELECTED）時，設備端立即釋放所有站 lock 並重置 handshake，由 operator fallback 接手（與 timeout 為兩條互補的釋放路徑）。 |

### 12.3 Host 端 Timeout（Host-side，建議）

| 項目 | 規範 |
|---|---|
| 監看對象 | host 送出 `START_AGV` 後，等待對應站點的 `AGVLDUnLDStatus (CEID273)` 與 `AGVLDUnLDFinish (CEID274)`。 |
| 逾時動作 | 在自訂時限內未收到 Ready / Finish，host 應放棄該筆 AGV 任務、重新派車或通報 operator，不可無限等待。 |
| 與設備端關係 | host timeout 與設備 timeout 互相獨立；任一端逾時都應能讓該站脫離鎖定，不依賴對方先動作。 |

### 12.4 取消轉移（Cancel Transition）

| 轉移 | 觸發 | 結果 |
|---|---|---|
| `PREP → IDLE` | 設備端 PREP dwell 逾時 / link drop | 釋放 lock、重新 arm；可重新叫車。 |
| `READY → IDLE` | 設備端 READY dwell 逾時 / link drop | 釋放 lock、重新 arm；視站別重新進入待補料或滿盤判定。 |
| host 主動取消 | host 端 timeout 或 operator 取消 | 建議 host 不再對該站期待 Finish；設備端最終仍由自身 timeout 釋放，確保即使 host 不通知也不會硬鎖。 |

> 設計準則：lock 的釋放絕不可只依賴「AGV 一定會完成 handshake」這個假設。設備端 timeout 是最後的安全網，任何缺席或中斷情境都必須能在有限時間內自動脫離鎖定。

## 13. 模擬 / 維修模式

| 模式 | Ready 條件 | Finish 條件 | 說明 |
|---|---|---|---|
| `SOFT_SIMULATE`（離線驗證） | 自動通過（AUTO-PASS） | 自動通過（AUTO-PASS） | 沒有實體感測器、沒有真實 AGV，故 sensor 判定的 Ready / Finish 在模擬下一律視為成立，讓 handshake 能完整跑完以供離線驗證。 |
| 真機（normal） | 依第 9 節：交接機構到位且可接受 AGV 動作 | 依第 9 節：對應 sensor 確認 tray / device 到位或被取走 | 完整 interlock 啟用。模擬旁路嚴格限制在 `SOFT_SIMULATE` 內，真機路徑不受影響。 |

> 重要：模擬旁路必須嚴格包在 `if(IsSoftSimulate()) return true;`（或 `#ifdef SOFT_SIMULATE`）之內，真機 `#else` interlock 必須完整保留、永不無條件旁路。尤其 Auto 站的「已排空可交接」判定在真機同時把關 SortArm 置料殘料 interlock（殘料未清不可放行 AGV）；該旁路只能 SIM-ONLY。

## 14. `START_AGV` 的 `Action` CP 值語意

現況：`START_AGV` 在設備端只用 `cpName`（站點名稱：`Loader` / `Empty` / `Color` / `AUTO1`..`AUTO6`）路由到對應站點，`Action` 這個 CP 值目前為裝飾性（設備端不解析其內容）。

雙方需擇一確認：

| 選項 | 定義 | 說明 |
|---|---|---|
| 選項 A：定義語意 | `Action = LOAD / UNLOAD / SUPPLY` | LOAD = 對 Handler 上料（P1 Loader）；SUPPLY = 補空盤 / 身分 Tray（P2 Empty、P3 Color）；UNLOAD = 取走滿盤（P4-P9 Auto）。若採用，設備端需驗證 `Action` 與站點方向是否一致，不一致則以 `HCACK` 拒絕。 |
| 選項 B：保留欄位 | `Action` 明確標記為 reserved | 設備端僅以 `cpName` 路由，`Action` 不解析、不驗證，保留供未來擴充。host 仍可帶值，但設備端忽略。 |

> 建議：在客戶尚未要求 LOAD/UNLOAD/SUPPLY 分流前，採選項 B（保留欄位），與目前實作一致；待需求明確再升級為選項 A 並加上方向驗證。第 4.1 節表格中的 `Action` 欄位語意以本節為準。

## 15. State Record / Logging 需求

為了讓離線分析能區分 PREP-stall、READY-stall 與 link-drop，設備端 State Record 必須記錄 AGV 協調器的 handshake 狀態，而非只記每模組的 lock。

| 記錄項目 | 內容 | 用途 |
|---|---|---|
| 每站 handshake 狀態 | P1-P9 各站的 `IDLE / CALLED / PREP / READY / FINISH` | 判斷卡在哪一個 handshake 階段（PREP vs READY）。 |
| 每站 lock 狀態 | P1-P9 各站的 AMR lock（0/1） | 對照 handshake，確認 lock 是否被孤立 latch。 |
| 每站 live ready 值 | P1-P9 各站的即時 ready 判定（Infeed `IsReadyForAmrHandoff` / Auto `IsDrainedForAmr`） | 判斷 Ready gate 為何未通過。 |
| SECS Selected 狀態 | HSMS 連線是否 SELECTED（0/1） | 區分 link-drop 與真正的 handshake stall。 |
| `bUseAMR` | AMR 功能是否啟用（0/1） | 確認協調器是否本應運作。 |

實作備註：以上資訊由 `TAgvCoordinator::DescribeAgvState()` 一次產出（header 含 `Selected` 與 `bUseAMR`，後接 P1-P9 各一行 `lock / hs / ready`），State Record 快照寫入器（`FeederDecision.txt` 的「AMR coordinator」區塊）與 AMR 維修畫面共用同一來源，不另開 getter。如此一來，「AMR lock 凍結某個 feeder」這類問題在離線快照即可直接看出卡點，無需上機重現。