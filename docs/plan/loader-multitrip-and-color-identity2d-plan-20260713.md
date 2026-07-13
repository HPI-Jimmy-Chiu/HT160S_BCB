# Loader 多趟 FIFO 佇列 + Color 讀身分盤 2D/SECS 流程 — 設計（待確認，未實作）

> 2026-07-13。依使用者流程定義 + 9045 AutoUP 對照（[[ht9045-loader-amr-traykind-model]]）+ HT160S 現況盤點（workflow wf_c04febf1，opus-4-8，對抗驗證）。
> **狀態：設計提案，尚未動任何程式碼。** 待 owner 確認決策點後才寫實作步驟。
> 關聯 TODO#4（進料側對齊）、TODO#5（身分盤 2D）、TODO#1（sensor 驅動 CleanOut）。

---

## 現況確認（對抗驗證通過的事實）

- Loader 每趟盤數目前是**單一 scalar**：`iCarTrayTotal`/`iSecsCarTrayCount`/`iFeedSerial`（`aLoader.h:57-63`），無任何 queue。`AgvCoord.TrayCount[]` 是**站別索引 P-1，不是趟別**。
- 每趟 count 只在 **CEID274 完成緣**灌進模組：`uAgvStation.cpp:83-84 InfeedRefill(0)` → `SetExpectedCarTrayCount` + `RefillSimInfeed`；後者**覆寫** `iCarTrayTotal` 並把 `iFeedSerial` 歸零（`aLoader.cpp:563-567`）→ 這就是「丟趟」缺陷。
- `GetFedTrayKind(iFeedSerial, iCarTrayTotal)` 唯一呼叫點 `aLoader.cpp:1561`；規則 serial≥total→Identity、==total-1→Cover、否則 Normal。
- 超盤 cross-check 現在報 **MES0921**（`aLoader.cpp:1436-1446`）。
- Color：單一載台馬達 `MColorY`；三個 teach Y — `ColorReceiveTrayYPosition`(前/堆疊下方，收料)、`ColorRead2DYPosition`(中間掃描站)、`ColorTrayArmPickYPosition`(後/交握)。2D reader 另有步進 `MTopCCDX_Color`；`MoveColorCcdToScan()`(`aColor.cpp:907`) 同時驅 Y+X 到掃描位。
- Color 2D API：`ColorCcdTriggerShot`/`ColorCcdGetResult`→`sTrayID2D`/`ColorCcdEndShot`，封裝在 `DoReadColor2D(int Flag)`（`aColor.cpp:1112`，逾時走 K_RETRY|K_SKIP|K_MANUAL_2D）。身分打印在 `StampReadIdentity2D`(`:1241`)+`BirthFrontTray`(`:1232`)。
- **CEID275 "AGVLdID" 已註冊但休眠**（`uHGemHT160.cpp:266`，掛 report 5 = 九站 CarrierID 38202-38210），**從未被 fire**；Loader/Empty/Color 的 `CarrierID[]` 只在 reset 設 `""`（Auto 站才有寫入）。Loader carrier id SVID = **38202**（與 9045 相同）。
- ⚠ 驗證者提醒：使用者流程「rear → 2D reader → front」是**現有進料路徑（front→middle→rear）的反向**，無現成序列可照抄，只有個別 move + GoUp 防撞閘可重用。

---

## 設計 A — Loader 多趟 FIFO 佇列（超越 9045 單槽）

### A.1 資料結構【定案：無上限，用動態 TList 佇列】
```
struct TTripEntry { int iTotal; int iServed; };   // iTotal=該趟總盤數(work+cover+identity)
TList *TripQueue;   // FIFO of TTripEntry* ; enqueue=Add(尾), dequeue=Delete(0)+free(頭)
```
用 `TList`（程式其他處如 CEIDList/ReportList/LockList 皆此慣例，BCB6 idiomatic）→ **無固定上限、無「佇列滿」情形**。當前消耗中的趟 = 隊首 `TripQueue->Items[0]`，就地更新其 `iServed`，rollover 時 `Delete(0)`。`iSecsCarTrayCount` 續作 host 單筆暫存。淘汰 `iCarTrayTotal`/`iFeedSerial`（每趟角色移入隊首 entry）。

**為何無上限是安全的（白話）**：佇列只裝「已送到、還沒吃完的趟」；盤一張張被消耗，該趟吃完就 `Delete(0)` 釋放。所以一個 lot 總共來幾千盤都沒關係——佇列任何時刻只反映「當下堆在 Loader 上、尚未消化的那幾趟」，那受 Loader 料倉實體容量限制（本就有 sensor 管），不會隨 lot 總量無限長大。enqueue 只在每趟到料發生一次（很少），不是 hot loop，動態配置無疑慮。

### A.2 入列 = CEID274 完成緣（每趟剛好一次）
`uAgvStation.cpp:84` 的 `RefillSimInfeed()` 換成 `LoaderModule->EnqueueTrip(iSecsCarTrayCount)`：
```
TTripEntry *e = new TTripEntry;
e->iTotal  = (n>0)? n : iSimAmrMaxTray[0];
e->iServed = 0;
TripQueue->Add(e);          // 尾插；無上限、無「滿」
```

### A.3 出列 = 消耗滿 rollover
原 mint 點 `aLoader.cpp:1559`（`iFeedSerial++`）改推進隊首趟：
```
TTripEntry *h = (TTripEntry*)TripQueue->Items[0];
h->iServed++;
if (h->iServed >= h->iTotal) { delete h; TripQueue->Delete(0); }   // 該趟吃完→釋放隊首
```
→ Trip1 的 cover/identity 邊界在 Trip1 消耗完才結算，Trip2/3 已入列也不影響。

### A.4 GetFedTrayKind 改鍵於隊首趟
```
TTripEntry *h = (TTripEntry*)TripQueue->Items[0];   // 佇列非空時
kind = GetFedTrayKind(h->iServed + 1 /*1-based serial*/, h->iTotal);
```
`GetFedTrayKind` 內部不變。各趟自帶 `iTotal` → 各趟自己的 cover+identity 邊界；Trip1 identity 在其邊界仍判 Identity→送 Color。

### A.5 溢量 / 源乾分支（case 9000 改寫，取代 MES0921）
```
if (bUseAMR && TripQueue->Count == 0) {           // 佇列空
    if (SnLoader_Inputend ON) {                   // 溢量：host 少報、實體多送
        RecycleExtraToEmpty();                    // 當空盤走既有 rear→TrayArm→Empty
        g_EventLog.Log("INF_OVERTRAY", msg, "");  // EventLog 一筆
        LogOverTrayRecycle(...);                   // 專屬檔 OverTrayRecycle\<YYYY_MM>\...(見 A-Q2)
        // 不報警
    } else { StartAutoCleanOut(); }               // Inputend OFF = 乾 → 自動 CleanOut
}
```

### A.6 觸點清單
`aLoader.h:57-63`、`aLoader.cpp`（`RefillSimInfeed`557 / mint 1559 / case9000 1436 / `InitialFlag` 69-72,102-103 keep-material 保留整個 TripQueue）、`uAgvStation.cpp:83-84`、新增 `EnqueueTrip`/`RecycleExtraToEmpty`/`WriteDatedTripLog`。

### A.7 決策紀錄
- **A-Q1【定案：無上限，已核實安全】** 用動態 TList，無「佇列滿」情形（見 A.1 白話說明）。**不設「總盤數 100 上限」**：100 是輸出 Auto 車容量 `MAX_TRAY_PER_CAR`（MyMotor.h:113，只 size `TMyCar::Tray[100]`＝輸出堆疊車，`aAuto1To6` 判滿用），**與進料完全無關**。進料四個計數器 `iSimInfeedCount/iSecsCarTrayCount/iCarTrayTotal/iFeedSerial`（aLoader.h:57-63）全是 `int`、**從不當陣列索引**（只做加減/比較），超過 100/1000 都不會壞、不會靜默毀損（workflow wf_06848e52 核實）。硬設 100 反而讓機台跑到第 100 盤就停。現場 AMR 隨機時間、不限量送盤 → 動態佇列正確承接。
- **A-Q2【定案：log 規格如下】** 溢量「送 Empty」＝把該盤標空盤走既有 rear→TrayArm→Empty，已接受（使用者：客戶流程風險，我方只需記錄）。記錄兩處：
  1. **EventLog 一筆**：`g_EventLog.Log(sCode, sMsg, sErrorPart)`（`cEventLog.h:17-19`，全域 `g_EventLog` 已於 `ht160s.cpp:252` Init）。用獨立代碼（如 `INF_OVERTRAY` 或 `MES09xx`）放 AlarmCode 欄，與既有 EventLog 列一起分類。
  2. **專屬檔（比照 EventLog＝月資料夾分類）**：`cCsvDailyLog` 預設 `lgMonthlyFolder`，路徑 `D:\HT160S_Log\OverTrayRecycle\<YYYY_MM>\OverTrayRecycle_<YYYY_MM_DD>.csv`（根 `HSys.LogRootDir` database.cpp:641；組裝 cCsvDailyLog.cpp:59-93）。以 `InitLog("OverTrayRecycle","HT160S",<header>)` 建（預設月夾，仿 `cEventLog.cpp:14`）+ `AppendLine(...)`；`SetRetentionDays(n)` 自動清舊月夾避免膨脹。
     - Header/欄位（逗號分隔，自由字串走 `CsvQuote`）：`Date,Time,Station,TrayKind,TrayID_2D,Reason,Destination`。時戳仿 `cEventLog::Log` 的 `yyyy/mm/dd` + `hh:nn:ss`。最低要求時戳+事件；其餘欄位皆現成、一併寫。
- **A-Q3** keep-material HOME 保留整個 TripQueue（不清空、不 free）；非 keep-material 才清空 free。

---

## 設計 B — Color 讀身分盤 2D → SECS 上傳 → 退回前方（防撞）

新增子階梯 `DoReadIdentityRetreat(int &Task)`，過程式 `switch(Task)`，比照 `DoFeedTray/DoGoUpTray`。

### B.1 派工（`DoColor case 100`，優先於 `bReturnTray`）
```
if (bReadIdentityPending && IsRearHasTray()) { DoReadIdentityRetreat(0); Task=1800; break; }
```
新增 `case 1800`（仿 1700）：`DoReadIdentityRetreat(1)`；完成清旗標、`Task=1`。
新增旗標 `bReadIdentityPending` + `RequestReadIdentityTray()`（仿 `RequestReturnTray`）；沿用 `NotifyTrayXToEmptyFinish` 當 rear 收到。

### B.2 逐 case
| case | 動作 |
|---|---|
| 0 | Flag==0 重置 → Task=1 |
| 1 | `RefreshStateFromSensors`；確認 `bRearHasTray`；夾持 → 100 |
| 100 | `MoveColorCcdToScan()`（`MColorY`→`ColorRead2DYPosition` ＋ CCD-X→`ColorRead2DXPosition`；=使用者說的「入料掃描位置」）→ 200 |
| 200 | `DoReadColor2D(0)` 重置 → 300 |
| 300 | `DoReadColor2D(1)` 讀碼→`sTrayID2D`+`StampReadIdentity2D`；逾時走既有 K_RETRY/K_SKIP/K_MANUAL_2D → 400 |
| 400 | **SECS 上傳**：`AgvCoord.CarrierID[?] = sTrayID2D`；`Gem->EventReport(0, 275)` → 500 |
| 500 | `DoGoUpTray(0)` 重置 → 600 |
| 600 | `DoGoUpTray(1)`；**防撞用其 case 10 前方 has-tray 閘**（`aColor.cpp:693-708`：`bFrontHasTray` 為 true 才先升柱讓位，false 則直接進料避免掉節撞擊）；完成清 `bReadIdentityPending`、Task=0 |

### B.3 與既有互鎖
- 防撞完全由 `DoGoUpTray case 10` 提供，不另寫。
- 與 `bReturnTray`(1700) 路徑互斥（新分支優先），避免同一後方 Tray 被兩路消費。
- 落點 = `ColorReceiveTrayYPosition`（前車、堆疊區下方）。

### B.4 待 owner 確認 / 實作細節
- **B-Q1【定案：用 Color 站 SVID 38204；★客戶暫時決議，未來可能反悔改回 9045 的 Loader 38202】** case 400 寫 `AgvCoord.CarrierID[2]`（Color=P3，SVID 38204）+ `Gem->EventReport(0, 275)`。
  - ★**備註（客戶暫定，預留反悔）**：客戶暫時要用實際讀取站 Color 38204；未來可能反悔改回 9045 編號 Loader 38202。→ 實作時把「站別 index / SVID」做成**單一改動點**（一個具名常數或 GeneralSetting，例如 `AMR_IDENTITY_CARRIER_INDEX=2`），將來切回 38202 只改一處（index 2→0），不散落。
  - HT160S 早已把 9045 的 CEID275 "AGVLdID" 與 carrier-id SVID 38202-38210 **原樣移植**（`uHGemHT160.cpp:266` 註冊、`uAgvStation.cpp:102-110`），編號完全相同、無須改號，只是目前休眠（從未 fire、Loader/Empty/Color CarrierID 未填）。此 case 400 是第一個真正 fire 275 的點。
  - ⚠ report 形狀：HT160S 是**靜態 report**（鎖定決策 [[secs-9045-vs-160-diff]]），CEID275 綁 report 5 = **一次送全部九站** CarrierID(38202-38210)；9045 用 host 動態 report，其 275 可能單送。若 KYEC host 要求單 SVID，需另註冊單-SVID report 給 275，否則 host 端 config 對齊 HT160 靜態 report。host 整合項，非阻礙。
- **B-Q2【定案：依建議執行】** 實作時處理反向路徑（rear→middle→front）的幾何對位：case 600 前先 `MoveColorY(ColorTrayArmPickYPosition)` 對齊 `DoGoUpTray` 的後方入口，或把 GoUp 尾段抽成共用 retreat helper（實作時擇一，以不改動 GoUp 既有語意為原則）。
- **B-Q3** 確認「入料掃描位置」= `ColorRead2DYPosition`（中間掃描站），與進料時 Color 掃 2D 同位（feed-in 落點是 `ColorReceiveTrayYPosition` 前方，兩者不同）。
- **B-Q4** 身分打印已存在（`StampReadIdentity2D`+`BirthFrontTray`），勿重複 birth。

---

## 相關一致性規則 — Unloader 上傳盤數也 work-only（TODO#6）
SECS「盤數 = work-only」不只進料側，**Unloader(Auto 輸出)上傳的盤數也只能是 work 盤數**，排除 cover+identity。
現況(commit 4681261 Report 6)：`uAgvStation.cpp:233/:327` `TrayCount[si]=Car->iTrayCount` **含** identity+cover → 多報。
修正：新增 `TMyCar::GetWorkTrayCount()`(數 `eTrayKindNormal` 盤)取代 `iTrayCount` 當 SECS TrayCount 來源；`DeviceCount` 不動(IC 數本就 work-only)。動 `MyMotor.h` 共用 header → 需 `-Full` build。

---

## 檔案（絕對路徑，皆在 `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\`）
`aLoader.cpp` / `aLoader.h` / `aColor.cpp` / `uteach.h` / `SecsGem\uAgvStation.cpp` / `SecsGem\uAgvStation.h` / `SecsGem\uHGemHT160.cpp` / `SecsGem\uHGemEquipment.cpp`
