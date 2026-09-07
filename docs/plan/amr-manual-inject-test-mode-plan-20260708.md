# HT160S AMR 手動注入測試模式 — 設計文件（草案）

- 日期：2026-07-08
- 分支：`feat/iosetview-172-refactor`
- 狀態：**待審核**（審核通過才動程式碼；本文件不含任何 code edit）
- 相關：`tsMaintAmr` 改名 commit、`docs/AGV/HT160S_E87_AGV_Operation_Manual.md`、
  `docs/AGV/HT160S_E87_AGV_Communication_Draft_20260527.md`、
  `docs/plan/amr-manual-test-plan`（前一輪產出的驗收測試計畫）

---

## 0. 一句話

在「沒有真實 AMR、`SnAutoX_InputEnd` 取車感測尚未接線」的情況下，讓工程師用**面板按鈕**觸發完整的
AMR SECS 握手（P1–P3 上料、P4–P9 下料，含最後的取車/補料完成），而**送給 Server 的命令內容與
正式生產完全一致**——按鈕只 bypass「實體觸發時機 + sensor 判斷」，不碰命令內容。

已定決策（本輪確認）：
1. 面板位置：**AMR 維護頁 `tsMaintAmr`**
2. 注入顆粒度：**每個邊緣獨立按鈕**
3. 驗證位置：**兩側分工**（設備端輕量即時判讀 + 模擬器端深度協定驗證）
4. 下一步：**先出本設計文件審核**

---

## 1. 為什麼可行且乾淨（設計原理）

HT160S 的程式碼結構剛好把兩件事分在不同層：

- **感測判斷層**：每一個 AMR 事件，讀感測的動作只發生在**一個** predicate 函式裡，形狀都是
  `if(sim) return true; else return <讀感測>;`。
- **命令組裝層**：`PollAndCall` / `ServiceHandshake` 拿到 predicate 的 `true` 之後，才
  `BuildBitmap()` → `EventReport(0, CEID)`，而 SVID（`TrayCount`/`CarID`）是**當下讀真實車態**
  （`uAgvStation.cpp:227-228`：`TrayCount[si]=Car->iTrayCount; CarrierID[si]=Car->CarID;`）。

因此只要**按鈕只翻動 predicate 的回傳值**，下游的 bitmap、CEID 編號、SVID 片數、S2F42 HCACK 全部
走原本的生產程式碼、一個 byte 都不變。這正好滿足「只 bypass 觸發時機與 sensor 判斷、不影響命令內容」。

### 鐵律（實作時不可違反）

- ✅ 對：按鈕把 `IsAmrTaken(a)` 這一次回傳 `true`（一次性 latch，被 predicate 消費一次）。
- ❌ 錯：按鈕直接 `EventReport(0,274)`。這會跳過 `BuildBitmap`/SVID 快照/握手狀態機，內容就偏離生產。

**注入在「判斷層」，永遠不在「發訊層」。**

---

## 2. 注入點清單（權威表）

| 站 | 邊緣（事件語意） | 唯一 predicate（注入點） | 位置 | 真機讀什麼 | 建議預設 |
|---|---|---|---|---|---|
| P4–P9 Auto | 滿車 / 要車（→CEID272+FullCEID） | `TAutoModule::IsOutputCarFullForAmr` | `aAuto1To6.cpp:1237` | `SnAutoX_InputFullTray` ON | 可注入，或讓機台裝滿跑真 |
| P4–P9 Auto | 排空就緒（→CEID273） | `TAutoModule::IsDrainedForAmr` | `aAuto1To6.cpp:1281` | **非感測**：機台真堆完（`bCarHasTray`/殘料清/前缸home） | **預設跑真**（可選注入） |
| P4–P9 Auto | **取車完成（→CEID274）** | `TAutoModule::IsAmrTaken` | `aAuto1To6.cpp:1309` | `SnAutoX_InputEnd` OFF | **核心注入點** |
| P1 Loader | 缺料 / 要盤（→CEID272） | `TLoaderModule::IsInputShortageForAmr` | `aLoader.cpp:531` | `SnLoader_Inputend` OFF | 可注入，或讓料耗盡跑真 |
| P1 Loader | 就緒（→CEID273） | `TLoaderModule::IsReadyForAmrHandoff` | `aLoader.cpp:517` | 前缸 home（機台狀態） | **預設跑真** |
| P1 Loader | **補料完成（→CEID274）** | `TLoaderModule::IsInputHandoffFinishedForAmr` | `aLoader.cpp:540` | AMR 送到才滿足 | **核心注入點** |
| P2 Empty | 缺料 / 就緒 / 補料完成 | `TEmptyModule::IsInputShortageForAmr` / `IsReadyForAmrHandoff` / `IsInputHandoffFinishedForAmr` | `aEmpty.cpp:104 / 90 / 123` | 同上 | 同上 |
| P3 Color | 缺料 / 就緒 / 補料完成 | `TColorModule::IsInputShortageForAmr` / `IsReadyForAmrHandoff` / `IsInputHandoffFinishedForAmr` | `aColor.cpp:108 / 94 / 128` | 同上 | 同上 |

> 輸入站的自由函式 dispatch 在 `uAgvStation.cpp:43-63`（`InfeedShortage`/`InfeedReady`/`InfeedFinished`），
> 但注入要下在**模組 predicate 本體**，不是 dispatch 層（dispatch 只是轉呼叫）。

### 「最小注入」原則（本輪選的每邊緣獨立按鈕自然支援）

- **純協定 bench 測（筆電/Dummy）**：六個邊緣全用按鈕，最快驗協定。
- **真機驗收（無真 AMR）**：只按「真 AMR 才會滿足、且沒有替代訊號」的兩顆——
  **取車完成（`IsAmrTaken`）** 與 **補料完成（`IsInputHandoffFinishedForAmr`）**；
  滿車/缺料/排空/就緒讓機台跑真的。→ 最接近生產。
- 每邊緣獨立按鈕 = 測試員自選哪些跑真、哪些注入。

---

## 3. Latch 機制

一次性 latch（讀一次即清，模擬單一 sensor edge，避免持續 `true` 造成重複觸發）。

predicate 開頭插入（示意，ASCII，實作為 BCB6 風格）：

```
bool TAutoModule::IsAmrTaken(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return false;
    if(AmrInject.ConsumeTaken(Index))   // NEW : manual one-shot, test-mode only
        return true;
    if(IsSoftSimulate())
        return true;
    TMySensor *EndSensor=GetInputEndSensor(Index);
    return (EndSensor!=NULL && EndSensor->Enable==true && EndSensor->IsOff());
}
```

- 插在 **OOR guard 之後、最前面**，讓注入在 sim 與 real build 都生效（真機測試才是重點）。
- `Consume*` 內部先檢查「測試模式 ON」，OFF 時恆回 false → predicate 完全走原路，零行為變化。

### Latch 儲存位置（待決 Q1）

- **選項 A（推薦）集中式**：新增輕量物件 `AmrInject`（`uAmrInject.h/.cpp` 或掛在 `AgvCoord` 旁），
  持有 `bTaken[9]`/`bFull[9]`/`bShortage[3]`/`bFinished[3]`/`bDrained[9]`/`bReady[3]` + `Consume*()` +
  `bTestMode` + `Reset()`。各模組 predicate 查它。**改動集中、模組欄位不膨脹、清除一處搞定。**
- 選項 B 分散式：每個模組各加 latch 欄位。改動散、清除要多處，不建議。

---

## 4. 面板設計（`tsMaintAmr`）

現況：`tsMaintAmr` 只有 `memAmrStatus`（`Align=alClient`，滿版顯示 `DescribeAgvState()`）。
改為上下（或左右）分割：

```
+-------------------------------------------------------------+
| [!] AMR 測試模式：ON  (紅底橫幅，僅測試模式顯示)              |
+---------------------------+---------------------------------+
| memAmrStatus              | 注入按鈕區 (pnlAmrInject)        |
| (DescribeAgvState 即時)   |  上料: Loader/Empty/Color        |
|                           |    [要盤][就緒][補料完成]        |
|                           |  下料: Auto1..6                  |
|                           |    [滿車][排空][取車完成]        |
+---------------------------+---------------------------------+
| memAmrTx : 最近 N 筆 SECS 交易 (CEID/HCACK)，HCACK!=0 標紅    |
+-------------------------------------------------------------+
```

- 一個總開關：`chkAmrTestMode`（勾選才啟用注入 + 顯示紅色橫幅 `lblAmrTestBanner`）。
- 命名沿用慣例（`spb`/`btn` + `Amr` + 站 + 邊緣），例：
  `btnAmrInjAuto1Taken`、`btnAmrInjLoaderShortage`、`btnAmrInjColorFinish` …
- 新元件以「欄位放 `__published` 欄位區、事件 handler 放後段」加入 `maintenance.h`
  （遵循該檔頭部規約：fields-before-handlers、`__published` 本體不放註解）。
- 刷新：`RefreshAmrStatus()` 已掛在維修頁 refresh 迴圈（`maintenance.cpp:1339`）；
  `memAmrTx` 一併在此更新，並依 `chkAmrTestMode` 顯示/隱藏注入區與橫幅。
- **DFM 改動不開 BCB designer**（避免 designer save 誤刪元件）；以 byte-safe splice + 手動同步
  `.h __published`，參照既有 `chkUseAMR`/`memAmrStatus` 樣板。

---

## 5. 測試模式旗標 + 防呆

- **runtime 旗標，非 `SOFT_SIMULATE`**：`AmrInject.bTestMode`（或 `GeneralSetting` 一個 **不持久化**
  的 bool）。開機預設 OFF，**不寫入 ini**（避免殘留到真生產）。（待決 Q2）
- **只有測試模式 ON 時按鈕/latch 才作用**；OFF 時所有 `Consume*()` 恆回 false。
- **自動解除**：偵測以下任一事件即 `AmrInject.Reset()`（清 `bTestMode` + 全 latch）：
  - 進入正式生產 / LotStart
  - HOME / InitialFlag
  - RunMode 變更離開測試情境
- **螢幕紅色橫幅** `lblAmrTestBanner`：測試模式 ON 時常駐，避免有人忘了關。
- 進一步保護（可選）：測試模式僅在**非運轉**時可勾（比照 `chkUseAMR` 於 `MachineRun.bRunning` 鎖定）。

---

## 6. 通訊顯示 + 驗證（兩側分工）

### 設備端（`tsMaintAmr` 的 `memAmrTx`）— 輕量即時判讀
- 列最近 N 筆 AMR 相關 TX/RX：`S6F11 CEID=272/273/274`、`S2F42 HCACK=?`、`S9Fx`。
- **`HCACK != 0`（未知 CP=2 / 格式錯=1 / 其他）與 `S9Fx` 標紅**——這就是「Server 回傳/命令格式不正確」
  的第一線基礎判斷（機台端 S2F41 handler 本就會算 HCACK，見 `uHGemHT160.cpp:966` 一帶）。
- 資料來源：於 GEM log 既有 hook 點鏡射一份到 `memAmrTx`（唯讀、只加不改協定路徑）。

### 模擬器端（`D:\AI_Area\Tool\HT160S_SECS_Simulator`，Python，可自由改）— 深度協定驗證
- 欄位型別、list 結構、bitmap「恰一位為 1」、CEID/RPTID 對應等深度檢查。
- 這側是測試工具、寫邊界允許修改，適合放重驗證邏輯與情境腳本。

### 注入稽核
- 每次 `Consume*()` 命中 → 寫一行到 `FeederDecision.txt` 的 `[AMR coordinator]` 區塊與 `memAmrTx`，
  標記 `MANUAL-INJECT <edge> P<k>`。→ 驗收記錄能證明哪些邊緣是真感測、哪些是人工注入。

---

## 7. 與現有 sim bypass 的關係

- 既有 `if(IsSoftSimulate()) return true;` **保留不動**；真機 build 沒有它，manual latch 是真機唯一 bypass。
- 順序：manual latch 在 `IsSoftSimulate()` **之前** → sim 與 real 都能被按鈕覆蓋（真機測試必要）。
- 對 Dummy：原本 sim 是「自動秒過」，manual latch 讓你能**控制每個邊緣的時機**，逐步觀察握手。

---

## 8. 影響面 / 觸及檔案

| 檔案 | 改動 |
|---|---|
| `uAmrInject.h/.cpp`（新，選項 A） | latch 容器 + `Consume*/Reset/bTestMode` |
| `aAuto1To6.cpp`（`.h` 視情況） | 3 個 predicate 開頭各加一行 `Consume*` |
| `aLoader.cpp` / `aEmpty.cpp` / `aColor.cpp` | 各 3 個 predicate 開頭各加一行 |
| `maintenance.h/.cpp/.dfm` | AMR 面板：按鈕區 + `memAmrTx` + `chkAmrTestMode` + 橫幅 + handler |
| `cStateRecordHT160.cpp` | `[AMR coordinator]` 區塊加注入稽核行（可選） |
| GeneralSetting（若旗標放這） | 一個非持久化 runtime bool（待決 Q2） |

---

## 9. Build / 驗證計畫

- 每次改 predicate/面板：刪對應 `.obj` → 編譯；wiring 改 → full build（`scripts/ops/build-ht160s.ps1 -Clean`）。
- **真機 build gate 必跑**：注入點是 `IsSoftSimulate()`-guarded 的 shared core predicate →
  `MachineType.h` 註解掉 `#define SOFT_SIMULATE` → `-Full` → 確認 exit 0 → **還原 define** → 重建。
- 編碼檢查：`scripts/ops/check-ht160s-source-encoding.ps1`。
- DFM+`.h` 同步：不開 designer；byte-safe splice；`object Name` 數與 `*Name;` 數對齊。
- **功能驗證**（sim）：勾 `chkAmrTestMode` → 對 Auto1 依序按 `滿車`→（送 START_AGV）→`排空`→`取車完成`，
  確認 `memAmrStatus` 的 `hs` 依序 `CALLED→PREP→READY→IDLE`，且 SECS log 的 `CEID272/273/274` bitmap
  目標為 `P4`、SVID `TrayCount` 為當下真實片數（證明命令內容 = 生產）。

---

## 10. 待決問題

- **Q1** latch 集中式（推薦）vs 分散式？
- **Q2** 測試模式旗標是否持久化？（推薦否，每次開機 OFF）
- **Q3** `排空就緒`/`就緒`（273 前置）是否也給注入鈕？（建議給，但預設跑真）
- **Q4** `memAmrTx` 顯示筆數 N（建議 30–50）與資料 hook 點確認。
- **Q5** 按鈕最終命名與版面（上下分割 vs 左右分割）。
- **Q6** 測試模式是否比照 `chkUseAMR` 於運轉中鎖定不可勾？

---

## 11. 範圍外

- 不改實體 AMR 機構、不改客戶 EAP。
- 不接 `SnAutoX_InputEnd` 實體線（硬體工項）。本測試模式是「接線前的替代 + 協定驗證」，
  **不是永久方案**；正式生產前該感測仍須接妥並 `Enable`。
- 不改任何 SECS 命令內容 / CEID / SVID / HCACK 邏輯。

---

## 12. 實作結果與攻防裁判修正（20260708，已實作 + 建置驗證）

### 12.1 與原設計的關鍵差異：注入點改在「協調器呼叫點」而非「predicate 本體」
攻防裁判（5 攻擊者 / 三維度）指出：六個 `*ForAmr` predicate 被**非 AMR 的生產 / CleanOut 程式共用**
（不 gate `bUseAMR`），例如 `IsOutputCarFullForAmr` 於 `aAuto1To6.cpp:874/1017/1497`、
`IsInputShortageForAmr` 於 `aEmpty.cpp:266/308/330`。若把注入放在 predicate 本體（原 §3 設計），
注入會**波及生產側 ladder**（誤觸 CleanOut Full modal、source-dry MES 告警），且 `IsDrainedForAmr`
的殘料 interlock 會被繞過。

**最終實作**：注入改在 `TAgvCoordinator::PollAndCall` / `ServiceHandshake`（`uAgvStation.cpp`）
的呼叫點，與真 predicate `OR`：
```
bFull  = AutoModule->IsOutputCarFullForAmr(a) || AmrInject.AutoFull(a);
bShort = InfeedShortage(p)                    || AmrInject.InputShort(p);
if(IsDrainedForAmr(a)  || AmrInject.AutoDrained(a)) ... // 273
if(IsAmrTaken(a)       || AmrInject.AutoTaken(a))   ... // 274 (one-shot)
if(InfeedReady(p)      || AmrInject.InputReady(p))  ... // 273
if(InfeedFinished(p)   || AmrInject.InputFinish(p)) ... // 274 (one-shot)
```
predicate 本體維持原狀 → 生產/CleanOut 讀真 sensor，注入爆炸半徑僅限 SECS 握手。
命令內容仍由下游生產碼從真實車態組出（不受影響）。

### 12.2 攻防裁判裁決（20 候選）
- **已修**：
  - 生產滲漏（#4）：`MachineStart`（`csystem.cpp`）加 `AmrInject.Reset()` → 任何開機/開始生產清測試模式+latch。
  - SubString 1-based off-by-one（#9）：`uAmrInject.h` 改 `Length()-2399`。
  - `memAmrStatus` 每 300ms 重繪清選取（#19）：加變更守衛。
- **重構解決**（predicate 共用副作用，#1/#6/#10/#16/#20）：見 12.1。
- **忠實行為、非缺陷**（僅測試模式、且 HOME/Start/取消勾選可清）：sticky FULL 於 watchdog 逾時後
  重觸發（#14）、CALLED 無 watchdog（#15）。
- **本質限制 / 操作責任（已記錄）**：
  - 對「實體仍滿」的車注入 TAKEN → `ClearAmrCar` 清邏輯車，恢復堆疊（#11/#13）；本功能前提為**無真實滿車/bench**，僅測試模式、有紅色橫幅。
  - 入料站真 sensor 若 Enable 且空，FINISH 後真 `InfeedShortage` 會再觸發 272（#7）——忠實反映「站確實空」。
  - 純注入不驅動實體搬運：payload SVID 為空、離散 Unloadtray CEID 不發（#8）；驗證涵蓋 272/273/274 序列+gating+HCACK。
  - SOFT_SIMULATE 筆電下下游 predicate 恆真，無法逐鍵 step（#18）；逐鍵 step 為真機組態功能。
- **模擬器可測（#3）**：確認 YES——真機組態下 arm FULL→272→START_AGV→PREP→(真排空或 arm DRAIN)→273→arm TAKE→274，全程可由 SECS host 模擬器驅動。
- **可選加強（未做）**：主畫面全域「AMR TEST」指示（目前僅 AMR 分頁紅色橫幅 + 非持久 + Start/HOME/取消勾選清除）。

### 12.3 建置驗證
- Sim `-Clean` **exit 0**；編碼檢查通過（161 檔）。
- 真機組態 gate：`MachineType.h` 註解 `#define SOFT_SIMULATE` → `-Full` **exit 0** → 還原 define → 開發態重建。
