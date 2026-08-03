# HT160S AMR 單動測試 · 模擬驗證 SOP

文件狀態：操作手冊（v1）｜日期：2026/07/20｜適用版本：`HT160S_Program_BCB_V1.0.0.0`
範圍：用「機台端 AMR 手動注入測試模式」＋「上位機 SECS 模擬器」兩個畫面，**在沒有真實 AMR、取車感測未接線**的情況下，逐一單動驗證整條 E87/AGV SECS 握手。
相關文件：`HT160S_E87_AGV_Operation_Manual.md`（AGV 對接手冊）、`HT160S_E87_AGV_Communication_Draft_20260527.md`（通訊規格）、`docs/plan/amr-manual-inject-test-mode-plan-20260708.md`（注入設計原理）、`D:\AI_Area\Tool\HT160S_SECS_Simulator\docs\操作說明書.md`（模擬器操作）。

---

## 0. 這份 SOP 涵蓋的兩個畫面

| # | 畫面 | 在哪裡 | 角色 |
|---|------|--------|------|
| ① | **機台 AMR 測試模式面板** | HT160S 主程式 → **維護（Maintenance）** → 右側 **AMR** 分頁 | 用面板按鈕**注入**每一個 AMR 事件邊緣（缺料 / 滿盤 / 排空 / 取車 / 補料完成），bypass 實體 sensor 與觸發時機 |
| ② | **SECS/GEM Host 模擬器** | `D:\AI_Area\Tool\HT160S_SECS_Simulator\code\secs_host_simulator.py` | 扮演上位機（EAP / 雲端），收設備事件（CEID 272-275）、回命令（`START_AGV` / `START`） |

> **一句話**：畫面①按鈕只翻動「感測判斷層」的回傳值，下游的 bitmap / CEID 編號 / SVID 片數 / S2F42 HCACK 全部走**原本生產程式碼、一個 byte 都不變**。所以你在模擬器（畫面②）看到的封包，和真實生產送出的**完全一致**。

### 0.1 卡住流程 vs 完整流程（一眼對照）

> 這張圖對照「**未 HOME → 只跑到一半**」（＝2026/07/20 `SECSGEM_TextLog_11.txt` 實際觀察到的情況）與「**先 HOME → 完整跑完**」。差別只在**測試前有沒有先 HOME**（見 §2 第 5 條、§10）。

```
   ✗ 未 HOME（卡住 = 你這次的 log）          ✓ 先 HOME（完整）
   ───────────────────────────────        ───────────────────────────────
   （機台 fAllMotorHome = false）           （先按 Home 鈕，homed = true）

   機台 → S6F11 CEID272  叫車  (Full/Short)  機台 → S6F11 CEID272  叫車  (Full/Short)
   Host → S2F41 START_AGV                    Host → S2F41 START_AGV
   機台 → S2F42 HCACK=0      hs=PREP          機台 → S2F42 HCACK=0      hs=PREP
            │                                         │  (按 Drain / Ready)
            ▼                                         ▼
     ✗✗ 就停在這裡                            機台 → S6F11 CEID273  Ready   hs=READY
   （ServiceHandshake:324                              │  (按 Take / Finish)
     fAllMotorHome==false                              ▼
     → 直接 return，                         機台 → S6F11 CEID274  Finish  hs=IDLE
     273/274 永遠不送）                       Host → S2F41 START
                                             機台 → S2F42 HCACK=0   → 續跑生產
```

| | 未 HOME（卡住） | 先 HOME（完整） |
|---|---|---|
| CEID272 叫車 | ✅ 有 | ✅ 有 |
| START_AGV → HCACK=0 | ✅ 有（`hs=PREP`） | ✅ 有（`hs=PREP`） |
| **CEID273 Ready** | ❌ **無** | ✅ 有（`hs=READY`） |
| **CEID274 Finish** | ❌ **無** | ✅ 有（`hs=IDLE`） |
| 續跑 START | ❌ 無 | ✅ 有 |
| 卡點 | 停在 `hs=PREP` | 走完回 `hs=IDLE` |

---

## 1. 名詞與角色

- **Handler = HT160S**（設備端，Equipment）。
- **Host / EAP = 模擬器**（上位機端；本地模擬器或客戶雲端）。
- **CEID 272/273/274**：設備主動上報的三個 AGV 事件 = 叫車 / Ready / Finish。
- **注入（Inject）**：測試模式下，用面板按鈕讓某個 AMR predicate「這一次回傳 true」，等同真實 sensor 觸發。
- **level（電位）latch** vs **edge（邊緣）one-shot**：見 §4.3。

---

## 2. 前置條件（缺一不可）

| # | 條件 | 如何確認 | 沒滿足會怎樣 |
|---|------|----------|--------------|
| 1 | **SECS/GEM 已啟用**（`CosFunction.bUseSecsGem`）| 主畫面出現 **SECS** badge | 整個 GEM 堆疊不啟動 |
| 2 | **AMR 模式開啟**（`GeneralSetting.bUseAMR=1`）| 主畫面 **AMR** badge = 綠色 ON；面板左側狀態列 `bUseAMR=1` | `PollAndCall` 直接 return，按鈕全無效 |
| 3 | **HSMS 已 SELECTED（連線就緒）** | 面板左側狀態列 `Selected=1`；模擬器右上 **● selected**（綠） | 設備不會送任何 CEID |
| 4 | **機台在 Normal（正常）生產模式**（`RunMode=Run_Normal`） | 主畫面模式選擇＝Normal（模擬器 `S1F3` 回覆 `Run Mode=0`） | **`PollAndCall` 第 237 行 `if(RunMode!=Run_Normal) return;` → 叫車 CEID272 不會送**（Full / Short 按了沒反應）。※ 只需選 Normal 模式，**不必**真的按 Start 跑料 |
| 5 | **★ 機台已 HOME 完成**（`fAllMotorHome==true`） | 主畫面狀態非 `HOMING`；各軸 HOME 燈綠（`fAllMotorHome && bHomeFinish`）；State Record 顯示已 homed | **`ServiceHandshake` 第 324 行 `if(RunMode==Run_Home \|\| fAllMotorHome==false) return;` → CEID273 / 274 永遠不送，握手卡在 PREP**（會出現「只有 272、沒有 273/274」）。**這是最容易漏的一條** |
| 6 | **模擬器 Automation ＝ ON** | 連線列 **Automation ✓** | 設備上報的 S6F11 不會被回 ACK（測試仍可看到 TX，但不完整） |

> **★★ 操作順序鐵律（務必照做）**：**先 HOME → 再勾測試模式 → 再按注入**。原因：HOME 完成（`csystem.cpp:1352-1361`：`InitialAllTask(true)` 內含 `AmrInject.Reset()`）與按 **Start**（`MachineStart` 亦 `AmrInject.Reset()`）**都會清掉測試模式與所有 latch**。所以若你「先勾測試模式再 HOME」，HOME 會把測試模式關掉，之後按注入全無效；若「根本沒 HOME」，273/274 被閘門擋死。
>
> **重點**：滿足第 4 點但仍「不要按 Start 跑真實生產」——測試模式 banner 也是這個意思（見 §4.4 安全）。RunMode=Normal 只是**模式選擇**，不是「正在跑料」；HOME 則用 **Home 鈕**完成（不要用 Start 去 HOME，Start 會清測試模式）。

---

## 3. 連線拓撲與角色方向（重要，勿接反）

截圖的預設組態：**模擬器 = Passive（監聽）**、**HT160S = Active（主動撥入）**。
（模擬器原始碼 `secs_host_simulator.py:590` 註解明載：*Passive default ON — the HT160S equipment is the active side that dials in, so the host simulator should listen by default*。）

```
[General.ini] Enable=1  ActiveMode=1  Address=<模擬器IP>  Port=5098  DeviceID=1
        HT160S (Equipment, Active dial-out) ───► 模擬器 (Host, Passive listen :5098)
                         └── HSMS-SS / SECS-II，Select 由設備連入後由模擬器完成
```

- 兩端必須**一 Active、一 Passive**（不可同 Active 或同 Passive）。
- 若要反過來（HT160=Passive、模擬器=Active）：模擬器連線列**取消 Passive 勾選**，HT160 `General.ini [SECS] ActiveMode=0`。協議與訊息完全相同，只差連線方向。
- 同機測試用 `127.0.0.1`；跨機把 `Address`/`Host` 改成對方 IP，防火牆放行 5098。

> 註：舊版 `HT160S_E87_AGV_Operation_Manual.md §3.1` 把方向寫成「HT160 passive / 模擬器 active」，與目前模擬器預設**相反**。以本 SOP（對齊截圖與原始碼）為準。

---

## 4. 畫面① — 機台 AMR 測試模式面板（逐元件說明）

進入路徑：**維護** → 右側導覽 **AMR** 鈕。

### 4.1 版面

```
┌───────────────────────────────── AMR ─────────────────────────────────┐
│ [左：即時狀態列]              ☑ Enable AMR manual-inject test mode ...   │
│  Selected=1 bUseAMR=1        ** AMR TEST MODE ON - injection active ... **│
│  P1 Loader: lock=1 hs=PREP    ┌ Supply P1-P3 : arm one-shot inject ─────┐│
│  P2 Empty:  lock=0 hs=IDLE    │ Loader  [Loader Short][Loader Ready][…F]││
│  P3 Color:  lock=0 hs=IDLE    │ Empty   [Empty Short ][Empty Ready ][…F]││
│  P4 AUTO1:  lock=1 hs=PREP    │ Color   [Color Short ][Color Ready ][…F]││
│  ... P5-P9 AUTO2-6            └─────────────────────────────────────────┘│
│  AmrInject: testMode=1        ┌ Discharge P4-P9 (Auto1-6) : arm … ──────┐│
│    armed[ A1D A1T P1S P1F ]   │ Auto1   [A1 Full][A1 Drain][A1 Take]    ││
│                               │ ... Auto6 [A6 Full][A6 Drain][A6 Take]  ││
│ [下：注入 log memo]           └─────────────────────────────────────────┘│
│  == AMR TEST MODE ON ==                                                  │
│  Loader SHORTAGE=1 / Auto1 FULL=1 / Auto1 TAKEN armed ...                │
└─────────────────────────────────────────────────────────────────────────┘
```

### 4.2 各元件

| 元件 | 說明 |
|------|------|
| ☑ **Enable AMR manual-inject test mode (bypass sensor edge only)** | 主開關。勾＝進入測試模式（`AmrInject.SetTestMode(true)`）；取消＝離開並**清掉所有 latch** |
| 🔴 **AMR TEST MODE ON - injection active, do NOT run real production** | 測試模式紅色警示條，只在測試模式亮 |
| **左側即時狀態列**（`DescribeAgvState()`）| 每秒刷新。`Selected=` HSMS 是否就緒、`bUseAMR=`、每站 `Pn 站名: lock=<鎖> hs=<握手狀態> ready=<就緒> bins=[...]`；最後一行 `AmrInject: testMode=1 armed[...]` 列出目前 armed 的注入 |
| **Supply P1-P3** 群組 | 上料（進料補盤）三站：Loader / Empty / Color，各 3 顆按鈕 Short / Ready / Finish |
| **Discharge P4-P9** 群組 | 下料（滿車取走）六站：Auto1-6，各 3 顆按鈕 Full / Drain / Take |
| **下方注入 log**（`memAmrTx`）| 顯示 `AmrInject.GetLog()`：每次按鈕、每次注入被消費（`... injected (274)`）、被拒的 S2F41 都會記一行 |

> **注意**：群組標題寫「arm one-shot inject」，但其實只有 **Finish / Take** 是真正 one-shot；**Short / Ready / Full / Drain 是 level 開關**（見 §4.3）。

### 4.3 按鈕 → 事件 → SECS 對照（權威表）

| 群組 | 按鈕 | 內部 edge | latch 型態 | OR 進的 predicate | 觸發的 SECS 事件 |
|------|------|-----------|-----------|-------------------|------------------|
| Supply（上料） | **Short** | SHORTAGE | level（每按 toggle） | `IsInputShortageForAmr` | **CEID272** 叫車（缺料補盤） |
| Supply | **Ready** | READY | level | `IsReadyForAmrHandoff` | **CEID273** Ready |
| Supply | **Finish** | FINISH | **edge（one-shot）** | `IsInputHandoffFinishedForAmr` | **CEID274** Finish → 解鎖＋補滿 |
| Discharge（下料） | **Full** | FULL | level | `IsOutputCarFullForAmr` | **CEID272** 叫車（滿盤）＋離散滿盤碼 |
| Discharge | **Drain** | DRAINED | level | `IsDrainedForAmr` | **CEID273** Ready |
| Discharge | **Take** | TAKEN | **edge（one-shot）** | `IsAmrTaken` | **CEID274** Finish → 清車解鎖 |

**離散滿盤預告碼（下料 Full 附帶）**：Auto1-3 → CEID **35/36/37**，Auto4-6 → CEID **148/149/150**（與 CEID272 一起送，供 9045 型 host 先收到離散 Full 訊號）。所以在模擬器看到 `CEID 35=?` 就是 Auto1 的滿盤預告，屬正常。

### 4.4 latch 語意與安全（務必理解）

- **level（sticky）**：FULL / SHORTAGE / DRAINED / READY。每按一次 on↔off toggle，維持 asserted，讀取**不消費**；cycle 完成（Take / Finish）時自動清掉。
- **edge（one-shot）**：TAKEN / FINISH。按下 = armed；協調器下一個 tick 消費一次 → 送 CEID274 → **清掉整站 cycle**（連 FULL/DRAINED 一起歸零），避免立刻又叫車 272。
- **★ 一次注入 = 一輪 cycle（已修，commit `40a343d`）**：協調器在送出 CEID274 時會呼叫 `AmrInject.ClearAutoCycle/ClearInputCycle` 清掉該站注入的 level latch（FULL/SHORTAGE 等），面板 log 顯示 `Auto1 cycle cleared (274)` / `P1 cycle cleared (274)`。所以**按一次 Short/Full → 跑完 272→273→274 一輪就自動停**，要再測就再按一次。
  - **背景（為何要修）**：`40a343d` 之前，sim 下 273/274 硬回 true 會短路掉 inject 的清除（`InfeedFinished(p) || AmrInject.InputFinish(p)`，uAgvStation.cpp:396 / Auto 同理），level latch 清不掉 → 每秒 `272→273→274` **無限重叫**。若你在**舊 build** 看到此循環，先 rebuild；臨時要停可：再按一次該 level 鈕、取消測試模式、或取消模擬器 Auto-AGV。真機一直都正常（finish 靠 sensor，inject 會被正常消費而清 latch）。
- **注入只在協調器端 OR-進 predicate**：`bFull = IsOutputCarFullForAmr(a) || AmrInject.AutoFull(a)`。predicate 本體不變，所以**生產 / Clean-Out 等其它消費者只看到真實 sensor**，注入的爆炸半徑僅限 SECS 握手。
- **左側 `ready=` 顯示真實 predicate、非注入值**：按了 Drain/Ready，`ready=` 不一定變——**以 log 與模擬器收到的 CEID 為準**。
- **安全**：`bTestMode` 預設 OFF、**永不持久化**（每次開程式都是 OFF）；任何 **HOME/init**（`InitialAllTask`）、**機台 Start**（`MachineStart`）、或**取消勾選**都會清掉測試模式與所有 latch。OFF 時所有讀取/消費都 no-op。**測試模式期間請勿跑真實生產。**

### 4.5 需不需要觸碰 sensor？（含 SOFT_SIMULATE）— 重要

**單動測試的設計目的就是「不必碰任何 sensor」**——注入按鈕直接取代 sensor 邊緣（面板勾選字樣即 *bypass sensor edge only*）。這在**真機**與 **SOFT_SIMULATE** 皆成立。

各邊緣的真機 / sim 行為（已對程式碼查證）：

| 邊緣（CEID） | 真機（SOFT_SIMULATE OFF）讀什麼 | SOFT_SIMULATE ON | 用注入按鈕時 |
|---|---|---|---|
| Full / Short（272） | Auto：`SnAutoX_InputFullTray` sensor｜Loader：`SnLoader_Inputend` | Auto：比 `iSimAmrMaxTray` 盤數門檻（非 sensor）｜Loader：`iSimInfeedCount<=0`（非 sensor） | 按鈕直接 OR 成 true |
| Drain / Ready（273） | Auto：車態 State 旗標＋前缸 out-bit（非 raw sensor）｜Loader：前缸 out-bit | **硬回 true**（完全不讀） | 按鈕直接 OR 成 true |
| Take / Finish（274） | Auto：`SnAutoX_InputEnd` sensor（**尚未接線→恆回 false，卡在 Ready**）｜Loader：`SnLoader_Inputend` ON | **硬回 true**（完全不讀） | 按鈕直接 OR 成 true |

**結論（回應 sensor 問題）：**
1. **真機**：想跑完整條而不碰 sensor → 用**注入按鈕**（尤其 Take/Finish，因為 `SnAutoX_InputEnd` 還沒接線，不注入就會永遠停在 Ready）。這正是本測試模式存在的理由。
2. **SOFT_SIMULATE**：273/274 的 predicate **本來就硬回 true、根本不讀 sensor**；272 讀的是模擬盤數（也非實體 sensor）。所以 **sim 下你完全不會碰到任何實體 sensor**。（甚至不注入，homed 之後 273/274 也會自動完成——注入是讓你能「明確、逐步」地驗每一站。）
3. **但**：以上都**不能免除「先 HOME」**。HOME 不是 sensor 問題，是 `ServiceHandshake` 的 `fAllMotorHome` 閘門（§2 第 5 條）。**sensor 可以不碰，HOME 一定要做。**

---

## 5. 畫面② — SECS/GEM Host 模擬器（逐元件說明）

啟動：`cd D:\AI_Area\Tool\HT160S_SECS_Simulator\code` → 雙擊 `start.bat`（或 `python secs_host_simulator.py`）。

### 5.1 連線列

| 欄位 / 鈕 | 說明 |
|-----------|------|
| **Host / Port / Device** | `127.0.0.1` / `5098` / `1`（Passive 時為本機綁定位址；Device 要與 160 DeviceID 一致） |
| **Passive ✓** | 勾＝模擬器監聽，等 160 連入（預設；見 §3） |
| **Automation ✓** | 勾＝對 160 主動送來的訊息自動回預設資料（S6F11→S6F12、S5F1→S5F2、S1F1→S1F2）。**測試時務必開** |
| **Auto-AGV ✓** | 勾＝模擬器自動扮演 EAP 跑完泳道：收 **CEID272** 自動回 `START_AGV(該站)`；收 **CEID274** 自動回 `START`。**不勾則需手動按**（見 §8） |
| **Connect / Disconnect** | 開始監聽 / 中止 |
| **Linktest** | 手動送一次心跳 |
| **● selected** | 綠燈＝HSMS 已 SELECTED（連線就緒） |

### 5.2 Params 列（依 preset 帶入，多餘忽略）

`rcmd`（遠端命令字）、`svid`/`alid`/`aled`/`ecid`/`ecval`（查詢/設定用）、`datetime`（校時）、**`agv`（AGV 目標站：Loader/Empty/Color/AUTO1..6）**、`traycount`（選填）、`lots`（Lot 清單，逗號分隔）。

> 手動測 AMR 時，只有 **`agv`** 欄要填（給 `S2F41 START_AGV` 用），其餘留空。

### 5.3 GEM Commands 按鈕（本 SOP 會用到的）

| 按鈕 | 用途 |
|------|------|
| **S1F13 Establish Comm** | 建立 GEM 通訊（連上後先按一次） |
| **S1F3 Status Request** | 讀狀態變數（可確認 Run Mode / System Running） |
| **S2F41 START_AGV** | （手動模式）指派 AGV 到 `agv` 欄站點 |
| **S2F41 START** | （手動模式）續跑生產（收到 CEID274 後才送） |
| 其餘（SET_LOT_INFO / LOTSTART / EC Set …） | Lot / 常數作業，非本 AMR 單動 SOP 範圍 |

---

## 6. 驗證 SOP A — 下料（Auto 滿車取走，P4-P9）

**情境**：Auto1 輸出車滿 → 叫車 → AGV 取走 → 續跑。以 **Auto-AGV = ON** 全自動回應，操作員只按畫面①三顆鈕。

| 步 | 你在畫面①做 | 畫面①應出現（log / 狀態列） | 畫面②模擬器應出現 |
|---:|-------------|------------------------------|--------------------|
| **0a** | **先 HOME 機台**（Home 鈕），等 HOMING 結束、各軸 HOME 燈綠 | 狀態非 HOMING；`fAllMotorHome==true`（否則後面 273/274 永遠不出） | — |
| 0b | 確認 RunMode=Normal、AMR ON、模擬器 ● selected | 狀態列 `Selected=1 bUseAMR=1` | ● selected（綠） |
| 0c | **HOME 之後**才勾 **Enable ... test mode** | banner 亮紅；log `== AMR TEST MODE ON ==`；狀態列 `AmrInject: testMode=1 armed[ ]` | — |
| 1 | 按 **A1 Full** | log `Auto1 FULL=1`；狀態列 `armed[ A1F ]` → 隨即 `P4 AUTO1: lock=1 hs=CALLED` | `RX S6F11 CEID=272`（＋`CEID 35`）→ 自動 `TX S6F12`、`TX S2F41 START_AGV(AUTO1)` |
| 2 | （等 START_AGV 回來） | 狀態列 `P4 AUTO1: lock=1 hs=PREP` | `RX S2F42 HCACK=0` |
| 3 | 按 **A1 Drain** | log `Auto1 DRAINED=1` | `RX S6F11 CEID=273`（Ready）→ 自動 `TX S6F12` |
| 4 | 按 **A1 Take** | log `Auto1 TAKEN armed` → 下一 tick `Auto1 TAKEN injected (274)`；狀態列 `P4 AUTO1: lock=0 hs=IDLE`、A1 latch 清空 | `RX S6F11 CEID=274`（Finish）→ 自動 `TX S6F12`、`TX S2F41 START` |
| 5 | （等 START 回來） | `RX S2F41 START` → `TX S2F42 HCACK=0`（見機台 SECS log） | `RX S2F42 HCACK=0` |

**通過準則**：模擬器依序收到 **272 → 273 → 274**，站點 bitmap 皆指向 **P4**；每步 HCACK=0；最後 Auto1 `lock=0 hs=IDLE`。

其它 Auto：把 A1 換成 A2..A6（對應 P5..P9）即可。

---

## 7. 驗證 SOP B — 上料（缺料補盤，P1-P3）

**情境**：Loader 缺料 → 叫車補盤 → 補料完成。同樣 **Auto-AGV = ON**。

| 步 | 你在畫面①做 | 畫面①應出現 | 畫面②模擬器應出現 |
|---:|-------------|-------------|--------------------|
| **0** | **先 HOME**（Home 鈕）等 homed → **再**勾 **Enable ... test mode** | `fAllMotorHome==true`；banner 亮；`== AMR TEST MODE ON ==` | ● selected（綠） |
| 1 | 按 **Loader Short** | log `Loader SHORTAGE=1`；`P1 Loader: hs=CALLED` | `RX S6F11 CEID=272`（P1）→ `TX S6F12`、`TX S2F41 START_AGV(Loader)` |
| 2 | （等 START_AGV 回來） | `P1 Loader: lock=1 hs=PREP` | `RX S2F42 HCACK=0` |
| 3 | 按 **Loader Ready** | log `Loader READY=1` | `RX S6F11 CEID=273`（Ready） |
| 4 | 按 **Loader Finish** | log `Loader FINISH armed` → `P1 FINISH injected (274)`；`P1 Loader: lock=0 hs=IDLE`、latch 清空 | `RX S6F11 CEID=274`（Finish）→ `TX S2F41 START` |

**通過準則**：模擬器收到 **272 → 273 → 274**，bitmap 指向 **P1**（Empty=P2、Color=P3）；HCACK=0；最後該站 `lock=0 hs=IDLE`。

Empty / Color：把 Loader 換成 Empty / Color 三顆鈕。

---

## 8. 手動逐步模式（Auto-AGV = OFF）

若要一步一步親自送命令（不讓模擬器自動回）：連線列**取消 Auto-AGV 勾選**（Automation 仍保持 ON 以自動回 ACK）。此時泳道由你手動補：

**下料（Auto1）**：
1. 畫面① **A1 Full** → 模擬器收 **CEID272**。
2. 畫面②在 `agv` 欄填 `AUTO1`，按 **S2F41 START_AGV** → 機台 `hs=PREP`。
3. 畫面① **A1 Drain** → 模擬器收 **CEID273**。
4. 畫面① **A1 Take** → 模擬器收 **CEID274**。
5. 畫面② 按 **S2F41 START** → 機台 `HCACK=0`，續跑。

**上料（Loader）**：把上面換成 Loader Short / `agv=Loader` START_AGV / Loader Ready / Loader Finish（補料完成後同樣可按 START 續跑）。

> 順序不可顛倒：必須 **Full/Short（272）→ START_AGV（PREP）→ Drain/Ready（273）→ Take/Finish（274）**。CEID273 只在收到 START_AGV（進入 PREP）後才會送。

---

## 9. 驗證檢查表（看到這些 = 對接成功）

- [ ] 面板左側 `Selected=1 bUseAMR=1`，模擬器 **● selected**（綠）。
- [ ] 勾測試模式後 banner 亮、log `== AMR TEST MODE ON ==`。
- [ ] 按 **Full/Short** → 模擬器 `RX S6F11 CEID=272`，bitmap 站點正確（下料 P4-P9 / 上料 P1-P3）。
- [ ] （Auto-AGV）模擬器自動 `TX S2F41 START_AGV`，機台 `RX ... HCACK=0`、`hs=PREP`、該站鎖定。
- [ ] 按 **Drain/Ready** → 模擬器 `RX S6F11 CEID=273`（Ready）。
- [ ] 按 **Take/Finish** → 模擬器 `RX S6F11 CEID=274`（Finish），機台清車/補滿、`lock=0 hs=IDLE`、latch 自動清空。
- [ ] （Auto-AGV）模擬器收 274 後自動 `TX S2F41 START`，機台 `HCACK=0`。
- [ ] 取消勾選測試模式 → banner 消失、`armed[]` 清空；HOME 或 Start 亦會自動清。

---

## 10. 疑難排解

| 症狀 | 可能原因 / 處理 |
|------|------------------|
| 按 **Full/Short** 完全沒送 CEID272 | ① 不在 **Normal 模式**（`RunMode!=Run_Normal`，`PollAndCall` 直接 return）② `bUseAMR=0` ③ 未 SELECTED（`Selected=0`）。對照 §2 前置條件 |
| **★ 只出現 CEID272，之後永遠沒有 273 / 274（握手卡在 `hs=PREP`）** | **最常見：機台未 HOME（`fAllMotorHome==false`）→ `ServiceHandshake` 第 324 行 freeze 閘門直接 return，273/274 完全不送**（送 272 的 `PollAndCall` 不受此閘門管，才會只有前半）。**處置：先 HOME 機台**（Home 鈕，等 homed）**再重測**。※ SOFT_SIMULATE 下 273/274 的 predicate 硬回 true，所以 sim 卡在 PREP 幾乎必然是「沒 HOME」。次要可能：先勾了測試模式又去 HOME/Start，把測試模式清掉了（見下一列）|
| 按 **Drain/Ready** 沒出 CEID273 | ① 機台未 HOME（同上，最優先查）② 尚未收到 `START_AGV`（未進 PREP，先確認第 2 步 HCACK=0）。順序見 §8 |
| 模擬器沒自動回 `START_AGV` | 連線列 **Auto-AGV** 沒勾（改手動 §8，或勾起來） |
| 模擬器收到 CEID 但無 ACK | **Automation** 沒勾 |
| 按 **Drain/Ready** 沒出 CEID273 | 尚未收到 `START_AGV`（未進 PREP）；先確認第 2 步 HCACK=0。順序見 §8 |
| 左側 `ready=` 按了 Drain 沒變 | **正常**：`ready=` 顯示真實 predicate，不反映注入。以 log / 模擬器 CEID 為準 |
| 面板按鈕全部無效 | 測試模式沒勾（`testMode=0`）；或 `bUseAMR=0` |
| 一勾測試模式又馬上被取消 | 剛做過 HOME/init 或按了 Start（`MachineStart`/`InitialAllTask` 會清測試模式）——重新勾一次，測試期間別按 Start |
| 看到 `CEID 35 / 36 / 37 / 148 / 149 / 150` | **正常**：下料 Full 附帶的離散滿盤預告碼（§4.3） |
| **★ 模擬器 `272→273→274` 不斷重複、不會停**（sim build） | **握手本身正確**。此循環是 `40a343d` 之前的舊 bug（sim 下注入 level latch 不自動清，§4.4）。**已修：現在按一次 Short/Full 跑完一輪就自動停**（log 見 `... cycle cleared (274)`）。若仍看到 → 你在舊 build，rebuild 即可；臨時要停：再按一次 Short/Full、取消測試模式、或取消 Auto-AGV。真機無此問題 |
| **`START` 回 `HCACK=2`（每個 274 之後）** | **正常（bench 無 Lot）**：`START` 前置檢查 `CheckLotDataReady`＝false（沒載 Lot / 2D 資料）→ HCACK=2。純握手測試不需要 START 成功；要成功需先 `SET_LOT_INFO`/`LOTSTART`+WebAPI 載入 Lot。與 AGV 叫車循環無關（循環由缺料觸發，非 START） |
| 連不上（一直 disconnected）| 兩端角色接反（都 Active 或都 Passive）；Port/防火牆；`General.ini [SECS] ActiveMode` 與模擬器 Passive 要相對（§3） |

---

## 11. 附錄

### 11.1 站點 P mapping（`PIndex = AutoNo + 3`）

| P | 站點 | P | 站點 | P | 站點 |
|--:|------|--:|------|--:|------|
| P1 | Loader | P4 | AUTO1 | P7 | AUTO4 |
| P2 | EmptyTray | P5 | AUTO2 | P8 | AUTO5 |
| P3 | ColorTray | P6 | AUTO3 | P9 | AUTO6 |

### 11.2 CEID / SVID 對照

| CEID | 事件 | 載 SVID | 意義 |
|-----:|------|---------|------|
| 272 | AGVSupplement | 38219 | 叫車：缺料(P1-P3) 或 Auto 滿盤(P4-P9)，bitmap 標目標站 |
| 273 | AGVLDUnLDStatus | 38220 | Ready：機構到位 / Auto 已排空 |
| 274 | AGVLDUnLDFinish | 38221 | Finish：上/下料完成 |
| 275 | AGVLdID | 38202-38207 + 38199-38201 | Carrier ID（不自動上報；host 以 S1F3 讀） |
| 35/36/37、148/149/150 | Auto Full 離散預告 | — | 下料 Full 附帶（Auto1-6） |

### 11.3 程式位置（維護參考）

| 功能 | 檔案 |
|------|------|
| 注入器 `TAmrInject`（latch 語意 / 消費點） | `SecsGem/uAmrInject.h` |
| 面板 UI（`chkAmrTestMode` / `BuildAmrInjectPanel` / 按鈕 handler / 狀態列刷新） | `maintenance.cpp`、`maintenance.h`、`maintenance.dfm` |
| 注入 OR-進 predicate 的消費點（`PollAndCall` / `ServiceHandshake` / `DescribeAgvState`） | `SecsGem/uAgvStation.cpp` |
| Normal 模式閘門（`if(RunMode!=Run_Normal) return;`） | `SecsGem/uAgvStation.cpp:237` |
| S2F41 `START_AGV` / `START` 受理 | `SecsGem/uHGemHT160.cpp` |
| 測試模式清除點（HOME/init、機台 Start） | `TDataModule1::InitialAllTask`、`csystem.cpp::MachineStart` |
| 模擬器主程式 / 連線角色 | `D:\AI_Area\Tool\HT160S_SECS_Simulator\code\secs_host_simulator.py` |

### 11.4 設計原理與限制

- 注入只 bypass「觸發時機＋sensor 判斷」，**不碰命令內容**——送給上位機的封包與正式生產一致（設計原理見 `docs/plan/amr-manual-inject-test-mode-plan-20260708.md`）。
- **真機限制**：真實 Finish 需「車輛取走」IO 點（`SnAutoX_InputEnd`）配線；配線前真機握手會停在 Ready，本測試模式正是用來在配線前先驗完整條協定。
- 上機 real host round-trip（真實 HCACK/CEID/EC-persist）驗證仍為待辦（需硬體）。

---

*本 SOP 對齊截圖與原始碼（`uAmrInject.h` / `maintenance.cpp` / `uAgvStation.cpp` / `secs_host_simulator.py`）撰寫；行為若與程式衝突以程式為準。*
