# Plan — 前分張器降盤統一（Color / Empty / Loader `DoGoDownTray`）

狀態：**PLAN ONLY — 等待使用者確認，尚未改任何程式碼。**
日期：2026-07-03
動機（使用者）：**Empty 動作「不順」、Color 卻「順」**。三站前分張機構完全相同，希望先把「軟體」做成一致，
之後若 Empty 仍不順，就能把問題**鎖定在機構或汽缸**，而非軟體差異。
參考：HT172（`D:\HT172\HT172_Program_V1.0.25.0_20260420`，唯讀）已用「共用底層汽缸原語 + 每站編排」的架構。
關聯：本案的 settle 常數同時被 [settle-time-config-panel-plan.md](settle-time-config-panel-plan.md) 追蹤（面板可調）；兩案不衝突，統一後面板改成調同一份 helper 的參數。

---

## 1. 目標與範圍

**主目標（一致性）**：讓 Color / Empty / Loader 的「分張一張、降到前緩存」動作走**同一份程式邏輯**，
差異只剩「哪三顆汽缸 / 哪顆確認 sensor / 哪個 alarm code / settle 值 / 是否互鎖」這些**資料**。
如此軟體行為可證明一致，把「不順」的變因收斂到機構/汽缸/氣壓。

**次目標（順帶修掉已知不一致）**：
- Loader 降盤仍用舊 `.On()/.IsOn()||sim` idiom（非正統，見記憶 `cylinder-idiom-pushcylinder-canonical`）。
- Loader 降盤 case4 未做「收回確認」——Color/Empty 已於 2026-07-01 修（掉整疊風險），Loader 還沒。
- Loader 降盤 settle 硬寫 `Delay.Set(10)`（=1000ms），面板/ini 調不到（`iLoaderDestackSettleMs` 只用在 Teach GoUp）。

**範圍內**：三站的「降盤 = 分張一張下來」序列（HT160S 稱 `DoGoDownTray` / `DoFrontDestackDown`），
以及其對應的 Teach 測試（`TestGoDownTray`）。
**範圍外（本案不動）**：`DoGoUpTray`（回盤上疊）、`DoFeedTray`（推到輸出）、TrayArm/SortArm/Auto。
但 GoUp 的 idiom 不一致（Empty GoUp 仍用 `.On()/.IsOn()||sim`）列入「後續」建議。

---

## 2. HT160S 現況：三方差異（已逐行確認）

| 面向 | Empty | Color | Loader |
|---|---|---|---|
| 函式 | `TEmptyModule::DoGoDownTray(int Flag)` [aEmpty.cpp:521](../../HT160S_Program_BCB_V1.0.0.0/aEmpty.cpp:521) | `TColorModule::DoGoDownTray(int Flag)` [aColor.cpp:443](../../HT160S_Program_BCB_V1.0.0.0/aColor.cpp:443) | `TLoaderModule::DoFrontDestackDown(int &SubTask, HTimer &Delay)` [aLoader.cpp:1824](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:1824) |
| 狀態變數 | member `GoDownTask`+`GoDownDelay` | member `GoDownTask`+`GoDownDelay` | 呼叫端傳參考（`State->DestackTask`/`FeedDelay`，因雙軌 Loader1/2 共機構） |
| 三顆汽缸 | `C_Empty_FrontRiseTray_1/_2`+`FrontSeparateTray_1` | `C_Color_FrontRiseTray_1/_2`+`FrontSeparateTray_1` | `C_Loader_FrontRiseTray_1/_2`+`FrontSeparateTray_1` |
| 汽缸 idiom | `PushCylinder/PopCylinder` 幫手（sim+Enable+逾時告警） | 同 Empty | **raw `.On()/.IsOn()‖sim` + case6 `.Pop()`**（舊 idiom） |
| settle 來源 | `SetMS(iEmptyDestackSettleMs)`；ini=**2000** | `SetMS(iColorDestackSettleMs)`；ini=**1000** | **硬寫 `Delay.Set(10)`=1000ms**；ini 值沒被降盤讀到 |
| settle 視窗數 | 4（分張後 / 收Rise2 / 收Separate / 收Rise1） | 4（同 Empty） | **1**（只在分張 ON 之後） |
| 收回確認 | case300 gate on `PopCylinder`（**已修**） | case300 同（**已修**） | **case4 只下 `.Off()` 不確認**（未修，掉疊風險殘留） |
| 分張互鎖 | case200 `IsFrontSeparateBlockedBy(Loader)` | **無** | case3 `IsFrontSeparateBlockedBy(Empty)` |
| 前置早退 | case10 `bFrontHasTray` 短路 | case1 `IsTraySupplyMode` 閘 + case10 短路 | **無**（短路在生產 case10 外部） |
| 到位確認+誕生盤 | **函式內** case700：`SnEmpty_InputHasTray`→**MES1024**→`BirthFrontTray()` | **函式內** case700：`SnColor_InputHasTray`→**MES1424**→`BirthFrontTray()` | **函式外**（生產 case9500）：`SnLoader_InputHasTray`→**JAM0913**→鑄 `fHasTray`/serial/kind |

**核心事實**：Color 與 Empty 的降盤階梯（case100–600）**除了「汽缸名 / settle 常數 / case200 互鎖」外，邏輯完全相同**。
Loader 則是不同世代寫法（傳參考、舊 idiom、單一 settle、外置確認）。

---

## 3. 「Empty 不順、Color 順」的軟體嫌疑點（診斷用）

同一份比對指出**三個純軟體差異**足以讓 Empty 感覺較不順（都可在不改機構下驗證）：

1. **分張互鎖（最可疑）**：Empty case200 會等 Loader 的 `C_Loader_FrontSeparateTray_1` 縮回才動；Color 無此等待。
   Loader 一忙，Empty 的降盤就會**卡在 case200**。此互鎖是**臨時機構對策**（`General.ini [Safety] FrontSeparateInterlock=1`，
   註解明載「set 0 after rework」）。→ 診斷步驟：暫時設 `FrontSeparateInterlock=0` 觀察 Empty 是否變順。
2. **settle 較久**：Empty `EmptyDestackSettleMs=2000` vs Color `1000`。四個 settle 視窗 → Empty 每張多花約 4 秒。
   → 診斷步驟：把 `EmptyDestackSettleMs` 暫改 1000 與 Color 對齊觀察。
3. **（次要）GoUp idiom 不一致**：Empty `DoGoUpTray` 仍用 raw `.On()/.IsOn()||sim`（[aEmpty.cpp:672-690](../../HT160S_Program_BCB_V1.0.0.0/aEmpty.cpp:672)），
   降盤已改幫手；上下行為不對稱。本案降盤統一後，建議 GoUp 一併對齊（列後續）。

> 若 (1)(2) 對齊後 Empty 仍不順 → 軟體已與 Color 一致，問題落在**機構/汽缸/氣壓**（分張爪、Rise 缸速度、真空、導引）。
> 這正是使用者要的「先一致、再鎖定」。本案的統一讓「軟體一致」成為**可證明**的前提，而不是逐站肉眼比對。

---

## 4. HT172 設計參考（已逐行確認；唯讀）

HT172 用 `TLoaderModule` 一個類別服務 Loader + Empty1 + Empty2 + Auto，關鍵三層：

1. **汽缸指標陣列**，以站別列舉為索引：
   `TMyCylinder *C_Up[eLoaderTotal]; *C_Middle[eLoaderTotal]; *C_Separatory[eLoaderTotal];`（[aLoader.h:203-205](file)）
   初始化把各站實體汽缸接進陣列（`C_Separatory[eEmpty1]=&HSys.Cyn...`, `C_Up[eEmpty2]=...`，aLoader.cpp:55-160）。
2. **共用底層汽缸原語**，只吃「站別索引」：
   `bool AutoCylinderUp(bool bReset, int iAuto)` / `AutoCylinderMiddle(...)` / `AutoCylinderLower(...)`（[aLoader.cpp:4424/4520/4582](file)）。
   一份實作被 **Loader(`iLoader`) / Empty1(`iEmpty1`) / Empty2(`iEmpty2`) / Auto(`iAuto`)** 共用（呼叫點：aLoader.cpp:1054/1425/1596/1889…）。
   原語內部：**每顆汽缸自帶時間與錯誤身分** — `CynUpDelay[i].SetMSAndOn(C_Up[i]->OnAlarmTime)`、
   `SetMSAndOn(C_Up[i]->OnDelayTime)`、失敗 `ShowSystemError(C_Up[i]->ErrorName[eOnNotOnErr], K_RETRY)`。
3. **每站編排** ladder（Loader ~1050-1170、Empty1 ~1420-1520、Empty2 ~1590-2180）呼叫上面原語、
   自理該站的 sensor / 車 / 盤身分。機構複雜度不同的站（Empty2/Loader 多 `C_Fixer`/`C_EdgePush`）就在編排層各自處理。

**對 HT160S 的啟示**：
- 172 的「共用原語 + 每站編排」正是要抄的骨架，但 HT160S 有**更好的條件**：三站前分張器**汽缸完全同構**
  （都恰好 Rise1/Rise2/Separate 三顆），不像 172 Empty1 vs Empty2 汽缸數不同。所以 HT160S 可以做到**比 172 更乾淨的單一原語**。
- HT160S 的 `TMyCylinder` **早已具備** 172 那套自帶時間/錯誤欄位：`OnAlarmTime/OffAlarmTime/OnDelayTime/OffDelayTime/OnAlarmCode/ErrorName[eCynErrTotal]/Delay/Enable`
  （[mycylin.h:21-68](../../HT160S_Program_BCB_V1.0.0.0/mycylin.h:21)），錯誤列舉 `eOnNotOnErr…` 也相同。缺的不是類別能力，是**降盤程式沒一致地使用它**。
- HT160S 也**早已有共用原語的慣例**：`DoClampTray(TMyCylinder &Lean, TMyCylinder &Push, int &SubTask, HTimer &Delay, bool bSoftSimulate, int SettleMs)`
  （[mycylin.h:82](../../HT160S_Program_BCB_V1.0.0.0/mycylin.h:82)）。本案只要新增一個**兄弟函式** `DoFrontDestackDown(...)`，
  完全沿用同一慣例——不需要引入 172 的類別陣列架構、也不違反 HT160 no-FSM 規則。

---

## 5. 統一設計提案（HT160 no-FSM 版）

### 5.1 新增共用原語（mirror `DoClampTray`）

在 `mycylin.h/.cpp`（與 `DoClampTray` 同檔）新增自由函式：

```cpp
// AI(destacker-unify) : shared front-destacker "separate one tray down" primitive.
// Cylinder-only (no Y/push/lean). Physical order fixed:
//   Rise1 up -> Rise2 up -> Separate on -> Rise2 down -> Separate off -> Rise1 down.
// SubTask/Delay caller-owned (init SubTask=1, Delay.Clear()). SettleMs = per-station settle.
// Interlock : if non-NULL, wait (return running) while Interlock->IsOn() (front-separate clash);
//             pass NULL for stations with no interlock (Color).
// Returns 0=running, 1=done(one tray separated). (Alarm-on-timeout handled by Push/Pop.)
int DoFrontDestackDown(TMyCylinder &Rise1, TMyCylinder &Rise2, TMyCylinder &Separate,
                       int &SubTask, HTimer &Delay, int SettleMs,
                       TMyCylinder *Interlock, bool bSoftSimulate);
```

實作即「Color/Empty 現行 case100–600 的正統版」：全程走 `Push()/Pop()`（sim+Enable+逾時告警內建於 TMyCylinder），
四個 settle 視窗，每次收回都 gate on `Pop()` 的確認（把 2026-07-01 的掉疊修正變成唯一實作）。
sim 由 `bSoftSimulate` 早退，不再靠 `||IsSoftSimulate()` 散落各步。

> 註：HT160 的 `PushCylinder/PopCylinder` 是各 module 的成員幫手（Empty/Color 有、Loader 沒有）。
> 共用原語直接用 `TMyCylinder::Push()/Pop()`（本就 Enable-aware、逾時告警）+ 傳入的 `bSoftSimulate`，
> 不依賴 module 幫手，三站一致。

### 5.2 三站改為呼叫原語

- **Color** `DoGoDownTray`：保留 case1（`IsTraySupplyMode`）、case10（`bFrontHasTray` 短路）、case700（`SnColor_InputHasTray`/MES1424/`BirthFrontTray`）**編排**；
  中段 100–600 換成 `DoFrontDestackDown(C_Color_FrontRiseTray_1, _2, FrontSeparateTray_1, GoDownTask?, GoDownDelay, iColorDestackSettleMs, NULL, IsSoftSimulate())`。
- **Empty** `DoGoDownTray`：同上，`Interlock=&C_Loader_FrontSeparateTray_1`、settle=`iEmptyDestackSettleMs`、sensor=`SnEmpty_InputHasTray`/MES1024。
- **Loader** `DoFrontDestackDown`：改成呼叫共用原語，`Interlock=&C_Empty_FrontSeparateTray_1`、settle=`iLoaderDestackSettleMs`（**改讀 ini，取代硬寫 `Set(10)`**）、`bSoftSimulate`；
  生產 case9500 的到位確認/鑄盤與 Teach `TestGoDownTray` 都不變（本來就呼叫 `DoFrontDestackDown`）。

> 編排層的差異（早退閘、確認 sensor、alarm code、盤身分誕生方式）**刻意保留為每站自理**——與 172「每站編排」一致，也符合模組各自持有身分邏輯的現況。

### 5.3 互鎖對稱化 + 可設定

- 現況：Empty↔Loader 互鎖，Color 無。統一後互鎖成為原語的一個參數（傳 `Interlock` 或 `NULL`）。
- 建議把互鎖是否生效仍讀 `General.ini [Safety] FrontSeparateInterlock`（原語內部檢查該旗標；旗標 0 時忽略 `Interlock`）——
  保留「rework 後可關」的既有設計，且三站行為由**同一段判斷**決定。

---

## 6. 分階段實作（每階段可獨立 build 驗證）

> 每階段完成後：刪對應 `.obj` → `scripts/ops/build-ht160s.ps1 -Clean`；動到 `mycylin.*`（共用/標頭）→ `-Full`。
> 觸及 `SOFT_SIMULATE` 共用碼 → 另跑真機組態驗證（`MachineType.h` 註解掉 `#define SOFT_SIMULATE`→`-Full`→復原）。
> 編碼：`mycylin.*` 為 legacy Big5，新增註解限 ASCII English；本 doc 為 UTF-8。

- **P0 — 落地共用原語（不接線）**：在 `mycylin.cpp` 新增 `DoFrontDestackDown(...)`，內容 = Color/Empty 現行 case100–600 的正統版。先不改三站呼叫。Build 綠。
- **P1 — Loader 接原語**：`DoFrontDestackDown`（Loader 版）改呼叫共用原語；settle 改讀 `iLoaderDestackSettleMs`；補上收回確認。
  生產+Teach 路徑不變介面。Build（含真機組態）。**上機驗證 Loader 降盤與 Teach 一致。**
- **P2 — Empty 接原語**：`DoGoDownTray` 中段換成呼叫原語（interlock 傳 Loader 分張缸）。case700 不動。Build。**上機驗證 Empty。**
- **P3 — Color 接原語**：`DoGoDownTray` 中段換成呼叫原語（interlock=NULL）。case1/case700 不動。Build。**上機驗證 Color。**
- **P4 — 互鎖對稱化**：把 `FrontSeparateInterlock` 旗標判斷收進原語；三站一致。Build + 上機。
- **後續（非本案）**：GoUp（`DoGoUpTray`）比照抽 `DoFrontDestackUp(...)` 共用原語，修正 Empty GoUp 的舊 idiom。

每階段都是「介面不變、實作收斂」，可單站回退。

---

## 7. 風險與相容

- **雙軌 Loader**：Loader1/2 共用機構、各自 `State->DestackTask`。原語吃 `int &SubTask` 傳參考，天生支援；Color/Empty 傳自己的 `GoDownTask`。無衝突。
- **settle 語意**：`Set(10)=1000ms`（`HTimer::Set(n)=n*100` [HTimer.cpp:14](../../HT160S_Program_BCB_V1.0.0.0/HTimer.cpp:14)）。改讀 `iLoaderDestackSettleMs`（ini 現值 1000）→ 行為等價，但變成可調。
- **收回確認**：Loader 補確認會讓「未確認就前進」的舊路徑消失（正向修 bug）；若某機台汽缸較慢，靠 `OnAlarmTime/OffAlarmTime` 逾時告警而非靜默前進。
- **Teach 測試**：Loader Teach 本就走 `DoFrontDestackDown`；Color/Empty 的 `TestGoDownTray` 亦可在後續改呼叫同原語（本案先不動，避免一次改太多）。
- **no-FSM / no-C++11**：自由函式 + `switch`，無 lambda/auto/enum class；不引入 172 的模組陣列架構。符合專案鐵律。

---

## 8. 開放問題（需使用者裁決）

1. **settle 值要不要三站統一成同一數字？** 為了診斷 Empty，建議測試期間三站都設 1000ms 對齊；量產是否各自保留可調（現況 Empty 2000 是刻意調慢？）由你決定。
2. **互鎖去留**：`FrontSeparateInterlock` 是臨時對策。若機構 rework 已完成可直接關（=0）；否則保留但納入原語參數。你要不要我在 P0 先做「暫時關互鎖 + Empty settle 對齊 Color」的最小驗證，確認這兩點就是「不順」主因？
3. **GoUp 是否納入本案**：目前列後續。若你要「上下行為都一致」，可把 P0 原語同時做 Up 版。
4. **是否先做純診斷、暫不重構**：可先只調 ini（互鎖 0 + Empty settle 1000）跑一輪；若證實 Empty 變順=軟體因素，再決定是否投入 P0–P4 重構。

---

## 附錄 A — 一致性檢查清單（統一後應成立）

- [ ] 三站降盤中段皆呼叫 `DoFrontDestackDown(...)`，無各自複製的 case100–600。
- [ ] 三站皆走 `TMyCylinder::Push()/Pop()`，無 `.On()/.IsOn()||sim` 殘留於降盤。
- [ ] 三站 settle 皆來自 `iXxxDestackSettleMs`（面板/ini 可調），無硬寫 `Set(10)`。
- [ ] 三站收回步驟皆 gate on `Pop()` 確認（無「未確認即前進」）。
- [ ] 互鎖由單一 `FrontSeparateInterlock` 判斷 + `Interlock` 參數決定，Color=NULL。
- [ ] 每站僅在編排層保留：早退閘、確認 sensor、alarm code、盤身分誕生——且清楚標註。
