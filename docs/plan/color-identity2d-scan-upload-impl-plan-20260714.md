# Color 掃身分盤 2D → 掃描後立即上傳 SECS(CEID275/SVID38204) — 實作計畫（定稿待審，未實作）

> 2026-07-14。**狀態：定稿設計，尚未動任何程式碼、未 build、未 commit。**
> 依 owner 2026-07-14 逐點確認 + 既有 [Design B](loader-multitrip-and-color-identity2d-plan-20260713.md)
> + 三輪 audit/攻防（現況 wf_adf92b9f、9045 對照、計畫驗證 wf_a1718b87）。
> **計畫驗證裁決：原稿「不可照現狀實作」——2 個 blocker 死鎖已於本版關閉（見 §1.5）。**

---

## 0. 定稿決策（owner 2026-07-14）

| # | 決策 |
|---|---|
| 來源 | 身分盤來自 **Loader/AMR 彈匣**，經 TrayArm 送 **Color 後方**才掃描 |
| 上傳時機 | **Color 掃描後立即上傳**（非 TrayArm 送 Auto 時） |
| 上傳範圍 | **只有「Loader 回收身分盤+讀 2D」上傳**；Color 供給讀 2D 不傳；每張 **intake 只傳一次** |
| Auto 滿倉 | **不發 event**；身分 2D 在 SVID `38205-38210`（`Car->CarID`）供 **S1F3 輪詢**——對齊 9045（poll-only）。⚠ 待驗 `Car->CarID` 有填真實 2D（比照 DeviceCount 曾恆 0 風險）；且 CarrierID[3..8] 只在 `PollAndCall`(bUseAMR && SELECTED && Run_Normal) 刷新，CleanOut/HOME 期間 S1F3 讀到的是上次 Normal 快照 |
| **D1 SVID** | **38204**（Color P3 = `AgvStation` index **2** = `CarrierID[2]`）。★**單一改動點 = 常數 `AMR_IDENTITY_CARRIER_INDEX=2`**；**report 7 的 SVID 必須由此常數導出**：`rLdId[0]=AgvStation[AMR_IDENTITY_CARRIER_INDEX].SvidCarrierID`（否則 index 與 report SVID 是兩處、改回 38202 要改兩地——驗證發現） |
| D2 report | CEID275 = 專屬單-SVID report 7 = [由上述常數導出的 SVID]（9045 是 host 動態、HT160S 靜態故寫死；只送剛掃的那張） |
| clamp 鐵律 | 夾 = **前擋(`LeanOnTray`) → 後勾(`PushTray`)**；放 = **先鬆後勾(`Pop PushTray`) → 再鬆前擋(`Pop LeanOnTray`)**（全專案通用 [[doclamptray-dual-cylinder-helper]]） |

---

## 1. 觸發範圍（只改 site 2；並帶「intake vs 回收」判別旗標）

身分盤送 Color 有 **3 觸點**，**只有 site (2) 改走掃+上傳**；(1)(3) 維持原回收：

| 觸點 | 位置 | 處置 |
|---|---|---|
| **(2) Loader 回收 intake** | `aTrayArm.cpp:955-960`（`DecidePlaceDestAfterPick`，`TAJOB_LOADER_RECOVERY`→identity→Color） | ✅ 走**身分 intake 契約**：`RequestReadIdentityTray()` + pick-time 互鎖 + `DoReadIdentityRetreat` |
| (1) CleanOut 清場轉向 | `aTrayArm.cpp:857-861` | ❌ 不動（`RequestReturnTray`，回收，不上傳） |
| (3) HOME 續產 re-send | `aTrayArm.cpp:1227-1228`（heal 跳過 DecidePlaceDestAfterPick） | ❌ 不動（`RequestReturnTray`） |

> **判別旗標（blocker① 修正）**：`DoPlaceToColor`（`aTrayArm.cpp:828`）是 intake 與回收**共用的單一梯**，`PlaceDest==TAPLACE_COLOR` 無法區分。TrayArm 於 site (2) 取工時設 **`bDeliverIsIdentityIntake=true`**（其餘 Color 放盤為 false），供下方 case-500 gate 分流。

---

## 1.5 兩階段 pick-time 互鎖（owner 更正②）＋死鎖防線（計畫驗證關閉）

**需求（owner）**：TrayArm **去 Loader 夾身分盤之前**，就要先讓 Color 收工進 idle，否則衝突（承諾原子性＋rear 交接動作干涉）。

**新增 `bool TColorModule::IsReadyToReceiveIdentity()`**：
```
DoColor idle (Task 1/100) && FeedTask==1 && GoDownTask==1 && GoUpTask==1 && ReadIdentityTask==1
  && Status 非 CS_FEEDING/CS_DESTACK/CS_RETURNING
  && MMColorY->fHasTray==false && bRearHasTray==false
  && IsReadyForAmrHandoff() && 前擋/後勾缸放開 && bAmrLocked==false
```
**新增 `bool TColorModule::IsReceivingIdentity()`**（= `bReadIdentityPending`，供 TrayArm 分流 case-500 gate）。

**握手（deadlock-safe）**：
1. **DecideJob（site 2，取工前）**：若有 Loader 身分盤待回收且目的 Color → `Color->RequestReadIdentityTray()`（set `bReadIdentityPending`＝預約）。
   - **Color 尚未 idle → 不取這趟**（回傳讓 TrayArm 做別的工作，尤其**先把 Color presenting 的盤取走送 Auto**），下 cycle 再判。
   - **Color idle（`IsReadyToReceiveIdentity()`）→ 才取工**（`bDeliverIsIdentityIntake=true`，去 Loader 夾）。
2. **Color 收到 `bReadIdentityPending`**：`DoColor case 100` 仲裁**最前面**加 idle-return（**blocker②/flow 修正**）：
   ```
   if(bReadIdentityPending){ if(IsRearHasTray()){ DoReadIdentityRetreat(0); Task=1800; } else Task=1; break; }
   ```
   且 destack(:421)/supply(:445) 兩 gate 各加 **`&& bReadIdentityPending==false`** → 預約當下就停開新任務、排空 idle。
3. **TrayArm 放盤 gate（`DoPlaceToColor` case ~500，blocker① 修正）**：**條件式**——
   ```
   bReady = bDeliverIsIdentityIntake ? Color->IsReadyToReceiveIdentity()
                                     : (Color->IsRearHasTray()==false);
   if(bReady || IsSoftSimulate()) 放盤;   // 保留 sim escape（major 修正）
   ```
   回收路徑**維持** `IsRearHasTray()==false`（回收本就要 Color 忙進 1700，不能套 idle-gate）。
4. 放盤 → `NotifyTrayXToEmptyFinish`（set `bRearHasTray`）→ Color 跑 `DoReadIdentityRetreat`。

> **死鎖防線總結**：TrayArm 等 Color idle 期間**持續服務其他工作（含取走 Color presenting 盤）**；「等 idle」只延後這趟身分盤，不凍結 TrayArm。回收路徑與 idle-gate 分流。單執行緒 `DoAllProcess`，靠旗標協調、防的是跨 cycle 實體動作。

---

## 2. `TColorModule::DoReadIdentityRetreat(int &Task)` 流程

| case | 動作 |
|---|---|
| **1** | 確認 `bRearHasTray` → `MoveColorY(ColorTrayArmPickYPosition)` → **前置：載台無盤(`fHasTray==false`)、前擋/後勾放開**（否則 → 走 else：`ShowMyError` 提示+retry，不 silent stall，**major/minor 修正**）→ 夾（前擋`LeanOnTray`→後勾`PushTray`）→ **`bRearHasTray=false; MMColorY->fHasTray=true`**（鏡射 GoUp case7000，讓二次接收能 re-arm，**major 修正**） |
| **100** | `MoveColorCcdToScan()`（載台 Y→中間掃描站 + CCD-X） |
| **200** | `DoReadColor2D` 讀 2D（`StampReadIdentity2D`；已 birth 勿重 birth） |
| **300** | **上傳**：`if(IsTrayID2DGenuine() && sTrayID2D!="") AgvCoord.ReportLoaderIdentity(HGem, AMR_IDENTITY_CARRIER_INDEX, sTrayID2D)` → CEID275/SVID38204 |
| **400** | `RefreshStateFromSensors`；**front 有盤 → 先 `RaiseFrontStackClear()`（升柱把原盤往上送、清出 rest、防兩盤相撞）**；rest 清空後 **`MoveColorY(ColorReceiveTrayYPosition)` + 放夾（先鬆後勾→再鬆前擋）+ 存放（`bFrontHasTray=true`、**重生 `FrontSourceTray` 為 Identity**＝B-4）** + 清 `bReadIdentityPending`/finish latch 完成 |

**GoUp helper（major 修正）**：`RaiseFrontStackClear()` = **抽出** `DoGoUpTray` 前段升柱（Rise_1/Separate/Rise_2 及退回）為**自結束 helper**，**不可** range-poke `DoGoUpTray`（其 case 600 尾跳 `GoUpTask=1000` 會誤觸「抓後方」段）。`DoGoUpTray` 既有語意不變。

（**無 case 500-600**；升柱/放盤收進 case 400）

---

## 3. 調和 `21ecb0f`（前置，必做）

| `21ecb0f` 現狀 | 處置 |
|---|---|
| CEID275 hook 在 `aTrayArm.cpp:931-933`，guard `Job==TAJOB_AMR_SUPPLY`（Color→**Auto** 交付） | **移除**。註：這是**遷移+收窄**（上傳點從 AMR-supply→Auto 改到 Loader-recovery Color intake），非在 3 個回收 site 間去重（**minor 修正措辭**） |
| `aTrayArm.cpp:17-18` 兩個 SecsGem includes（hook 是 `AgvCoord`/`HGem` 唯一用處） | **一併移除**（否則 dangling，**minor 修正**） |
| SVID 38202 / report 7 = [38202] | 改為由 `AMR_IDENTITY_CARRIER_INDEX` 導出（→38204） |
| `ReportLoaderIdentity(Gem,id2D)` | 改簽名吃 index：`ReportLoaderIdentity(Gem, idx, id2D)` → `CarrierID[idx]=id2D` |
| `IsTrayID2DGenuine()` | 保留（case 300 上傳前 gate） |
| **`aColor.cpp` 需新增** `#include "SecsGem\uHGemEquipment.h"` + `"SecsGem\uAgvStation.h"`（IncludeAllHeader.h 不含，比照 21ecb0f 對 aTrayArm 的做法，**minor 修正**） |

---

## 4. 實作步驟（排序、可驗收）— **尚未執行**

0. 移除 21ecb0f：`aTrayArm.cpp` case-4000 hook + :17-18 includes。
1. 常數 `AMR_IDENTITY_CARRIER_INDEX=2`；`ReportLoaderIdentity` 改吃 index。
2. `uHGemHT160.cpp` report 7 SVID 由常數導出。
3. `aColor.h`：宣告 `DoReadIdentityRetreat`/`RequestReadIdentityTray`/`IsReadyToReceiveIdentity`/`IsReceivingIdentity`、旗標 `bReadIdentityPending`、sub-task `ReadIdentityTask`。加 `aColor.cpp` 兩個 SecsGem includes。
4. `aColor.cpp`（Big5 byte-safe）：`DoReadIdentityRetreat`(case1/100/200/300/400 含 latch 清、else 失敗路、B-4 重生 Kind)、`RaiseFrontStackClear` helper、`DoColor` 派工（`bReadIdentityPending` 最前 idle-return + destack/supply gate 加 `&& bReadIdentityPending==false`）、case 1800、`RequestReadIdentityTray`/`IsReadyToReceiveIdentity`/`IsReceivingIdentity`。
5. `aTrayArm.cpp`：site (2) `RequestReturnTray`→`RequestReadIdentityTray` + `bDeliverIsIdentityIntake` 旗標 + DecideJob pick-time 互鎖（Color idle 才取工）；`DoPlaceToColor` case-500 **條件式 gate**（intake→`IsReadyToReceiveIdentity()||sim`；回收→`IsRearHasTray()==false`）。sites (1)(3) 不動。
6. **建置閘門**：`build-ht160s.ps1 -Full`(sim) EXIT0；`SOFT_SIMULATE` off real `-Full` EXIT0；`git checkout MachineType.h`；再 `-Full`(sim)；encoding check。
7. **headless sim 斷言**：一趟 intake+互鎖能跑完（防 sim hang，major 修正）。
8. **攻防裁判複驗**（共用梯死鎖已關？livelock？sim？二次接收？GoUp helper？供料 starvation？clamp 鐵律/防撞/上傳一次/不誤傳/Auto poll-only；每 finding 敵對駁斥）。
9. **上機驗證（owner）**：case 400 rear→中→front 夾/移/升柱防撞幾何、CEID275/SVID38204 交握、`Car->CarID` 填真實 2D。

---

## 5. 觸點檔案（`D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\`）
`aColor.cpp`(Big5)/`aColor.h`/`aTrayArm.cpp`/`SecsGem\uHGemHT160.cpp`/`SecsGem\uAgvStation.cpp`/`SecsGem\uAgvStation.h`

## 6. 風險 / 待驗
- **共用梯**：intake/回收分流靠 `bDeliverIsIdentityIntake` + 條件 gate，實作務必兩路徑都測（否則回收死鎖）。
- **供料短暫停**：預約→排空期間 Color 暫停供料（SortArm/Auto 等一個 intake 窗）；intake 快、可接受，但釋放要即時。
- **HOME 續產**：resume heal 用 `RequestReturnTray` 重送（跳過 DecidePlaceDestAfterPick）→ 中斷的 intake 在 HOME 後會走回收路徑（不上傳）；需確認可接受或補處理。
- **離機無法驗**：case 400 反向路徑升柱防撞；步驟 6-8 只保證 compile+邏輯+攻防，機構正確性 owner 首次上機留意。
- **外部引用**：9045 `asendic_Auto.cpp:546-560` 屬 9045 樹、此處無法核（poll-only parity 由 HT160S 側 report5-pollable/report7-event 佐證，成立）。
