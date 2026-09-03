# 盤來源 / 交接重構計畫（Phases 1–5）

> 狀態：**已實作 + 三組 build 全 exit 0**（sim / 真機 SOFT_SIMULATE-off / 還原 sim），2026-06-24。等使用者上機驗證。
> 範圍鎖定：Phase 1–5（Empty / Color → TrayArm → Auto 的盤資料來源與交接）。Phase 6（Loader AMR 身分盤/上蓋回收）規格附於文末，**本次不做**。
>
> **實作時 2 點與原計畫的差異（已決定）**：
> 1. 用 `TAutoModule::StageRearGrid(Index, Grid)` 暫存槽交接（AMR+Normal 統一），TrayArm 完全不碰 Auto VMotor → 因此 `GetRearVMotor` 不需要、未新增。
> 2. Color K_SKIP（讀不到 2D）仍 `BirthIdentityTray()`（Kind=Identity, TrayID=""）。深究後：路由真實來源是 scalar `iDeliverKind/iDeliverTrayID`，grid 的 Kind 為 inert（不參與 CarID/SortArm 路由），且 Color 實體上只供應身分盤，標 Identity 才正確。reviewer 擔心的空 CarID 是既有 scalar 行為、與本 grid 改動無關。

## 核心原則

盤資料 `TMyTray` 隨實體載具移動：在**來源 latch 誕生**（Empty 有盤 / Color 2D-read）→ 確認夾取時**深拷貝進手臂** → 確認放下時**交接進 Auto**。
傳遞機制一律用 compiler 預設 member-wise 賦值 `dest.Tray = src.Tray`（深拷貝 4 個 grid + TrayID + Kind；`AnsiString` 為 refcount，安全；`Car[].Tray[n]=` 已在 aAuto1To6.cpp:546 用同樣方式）。**不**動 `CopyTrayFrom`/`MoveTrayFrom` 空殼。

## 鎖定的設計決策

| 決策 | 內容 |
|---|---|
| A | TrayArm 自帶 `TMyTray ArmTray`，盤資料跟著手臂走 |
| B | 搬完整 per-cell grid（空盤/身分盤即使全空也整盤複製，統一介面） |
| C | 本次只做 1–5；Phase 6（Loader 回收）後續另開 |
| D | 「確認夾取/放下」沿用現有 clamp Push/Pop + Z-up sensor，不加盤在位 sensor |

---

## (a) TrayArm：新增 `TMyTray ArmTray`

- **aTrayArm.h**：頂部 `#include "MotorAndIO/MyMotor.h"`（by-value 成員需完整型別，forward decl 不足）；`private:` 與既有 `iDeliverKind`/`iDeliverTrayID` 同區加 `TMyTray ArmTray;`。
- **aTrayArm.cpp `InitialFlag(bKeepMaterial)`**：
  - keep 分支（早 return，行 52–56）：**不動** `ArmTray`，與既有 Kind/TrayID 保留邏輯一致（夾爪夾著、盤隨手臂升起，grid 必須保留）。
  - 非 keep 分支（清除區，行 57–62）：在 `iDeliverTrayID="";` 之後加 `ArmTray.Clear();`，避免殘留 grid 漏到下一次 place。

## (b) Empty：來源 `TMyTray`，在「有盤」latch 誕生（EMPTY_IC, Kind=Normal）

- **aEmpty.h**：`#include "MotorAndIO/MyMotor.h"`；`private: TMyTray SourceTray;` + `private: void BirthRearTray();`；`public: TMyTray GetSourceTray();`。
- **aEmpty.cpp**：抽 `BirthRearTray()`（`SetAll(EMPTY_IC)` + `ClearBin()` + `ClearLotCode()` + `SetKind(eTrayKindNormal)` + `TrayID=""`），在 **DoFeedTray 兩個 rear latch** 各呼叫一次：case 7000（行 ~329 `bRearHasTray=true`）與 case 12000（行 ~350）。
- **DUMMY 容忍**：誕生掛在 `else`（非 alarm）latch 分支＝`iRealDummy==DUMMY` / sensor 停用時會走的路徑，因此 DUMMY 下照常前進。**不**碰 `RefreshStateFromSensors`（sim 下 early-return、sensor 驅動；grid 只掛 latch）。
- 前段 `DoGoDownTray` 設的是 `bFrontHasTray`（前置暫存盤，非 TrayArm 取的盤）→ **不在此誕生**。
- `TMyTray TEmptyModule::GetSourceTray(){ return SourceTray; }`（return-by-value = 深拷貝）。

## (c) Color：來源 `TMyTray`，在 2D-read 誕生（空 grid + Kind=Identity + TrayID=sTrayID2D）

- **aColor.h**：`#include`；`private: TMyTray SourceTray;` + `private: void BirthIdentityTray();`；`public: TMyTray GetSourceTray();`。
- **aColor.cpp `DoReadColor2D`**：
  - `BirthIdentityTray()`＝`SetAll(EMPTY_IC)` + `ClearBin/LotCode` + `SetKind(eTrayKindIdentity)` + `TrayID=sTrayID2D`。
  - 在**真實讀取成功**兩處 `return true;` 前呼叫：sim/disabled（行 ~529，sTrayID2D 於 528 已設）、real scan（行 ~582，sTrayID2D 於 579 設）。
  - **K_SKIP 兩處**（行 ~561 / ~599，`sTrayID2D=""`）：**不要**生成 `Kind=Identity`（避免空身分→`CarID=""`，見 aAuto1To6:550）。改生成 `Kind=Normal, TrayID=""`（良性空盤）或不生成（擇一明確處理）。

## (d) 夾取拷貝（DoPick c4000）/ 放下交接（DoPlace c4000）

- **DoPick c4000**（行 346–372）：
  - Color 分支：在 `NotifyTrayPicked()`（行 361）**之前**加 `ArmTray = ColorModule->GetSourceTray();`（趁 Color 仍持有）。
  - Empty 分支：在 `SetRearHasTray(false)`（行 367）**之前**加 `ArmTray = EmptyModule->GetSourceTray();`。
  - Loader 分支：本次 out-of-scope，加 `//TODO Phase 6` 註記即可，不新增 Loader plumbing。
- **DoPlace c4000**（行 431–446）—— ⚠ **已依 reviewer 修正（BLOCKER 1）**：
  - **不可**在 place 時對 Auto VMotor 設 `fHasTray=true`。Auto 每站只有一個 `TTrayMotor`（GetAutoVMotor），「rear / working」是同一載具的兩個 Y 位；`FindFeedAuto` 的 gate 是 `bCarHasTray==false && bRearHasTray`，而 `bCarHasTray` 每 cycle 由 `TrayMotor->fHasTray` 重算。若 place 就把 `fHasTray=true`，該站被永久跳過 → **進料死鎖**（DUMMY/selftest 抓不到，量產第一盤就掛）。
  - 採**暫存槽**方案：在 `TAutoModule` 加 `TMyTray RearGrid[6]`（與既有 `RearKind`/`RearTrayID` staging 同模式）。`NotifyTrayArmDelivered` 增一個帶 grid 的多載/參數，把 `ArmTray` 存進 `RearGrid[Index]`。place 端**不碰** VMotor `fHasTray`。最後 `ArmTray.Clear();`。

## (e) aAuto1To6.cpp:527 —— 移除自捏、改為接收

DoFeedTray case 7000（行 523–559）把
```
TrayMotor->fHasTray=true;
TrayMotor->InitNewTray(EMPTY_IC);   // 移除：自捏全空 grid、丟棄交接內容
```
改為
```
TrayMotor->Tray = RearGrid[iFeedAuto];   // 接收交接來的 grid（rear→working 提升）
TrayMotor->fHasTray=true;                // 載具佔用由 feed 擁有（沿用既有時機）
TrayMotor->Refresh();                    // InitNewTray 原本內含 Refresh，移除後要自己補
```
- 行 530 以下的 bool/scalar bookkeeping（`bCarHasTray`/`bRearCanUse`/`bFullIC`、`WorkingKind=RearKind`、`WorkingTrayID=RearTrayID`、Car-stack、`IsReadyForSortArmPlace`）**全部不變**——Kind/TrayID 路由仍以 scalar 為真實來源。
- **保留不動**：discharge 的 `InitNewTray`（行 597）、cleanout（行 792）——皆為離場 reset。

## (f) NotifyTrayArmDelivered / SetRearHasTrayFromTrayArm

- Kind/TrayID 仍走 **scalar** 路徑（Car-stack/`bFullIC`/`IsReadyForSortArmPlace` 靠它），grid 內的 Kind/TrayID 為**冗餘但一致**（Color 同時設 grid 與 scalar，建構上必然相等）。
- `NotifyTrayArmDelivered` 增加把 `ArmTray` 存入 `RearGrid[Index]` 的能力（BLOCKER 1 修正）。`SetRearHasTrayFromTrayArm`（Normal bool-only）行為不變，但 Normal 路徑也需要一個對應的 grid staging（見下「待確認」）。

## (g) 已知範圍限制（reviewer 標註，本次可接受）

- **Car-stack 仍只 stamp scalar、未複製 grid**（aAuto1To6:543–552）。Phase 1–5 盤皆 EMPTY_IC，無 IC 資料遺失 → 可接受；但「全程整盤搬運」之說在此步其實未達成，**明確記為待辦**（將來 Normal IC 盤入堆時補 `Car[].Tray[n]=*TrayMotor;`）。

## (h) Build / 驗證

- **Big5 安全**：受影響檔皆含 Big5 註解 → **不可用 Edit 工具**；用 `scripts/ops/bcb6-bytesafe-edit.ps1`（優先）或 python latin1 splice。新註解一律 ASCII English、無 BOM。
- **強制 `-Full`**：三個 class 新增 `TMyTray` value 成員改變物件 layout（每個 grid 20×50、四組、約數十 KB），波及所有 include 這些 header 的 TU。`-Clean` 會留下 layout 不符的舊 .obj → 靜默損壞。用 `scripts/ops/build-ht160s.ps1 -Full`，要 exit 0。
- **真機 build**：誕生點落在 `IsSoftSimulate()`/`iRealDummy` 附近 → 註解掉 `MachineType.h` 的 `#define SOFT_SIMULATE`，`-Full` 跑一次確認 exit 0，**還原** define 再 build。
- 編碼檢查：`scripts/ops/check-ht160s-source-encoding.ps1`（不得有 `EF BF BD`/UTF-8 BOM）。
- 迴歸（選配）：`ht160s-home-selftest`（驗 bKeepMaterial 帶盤過 HOME）。
- **成本更正**：非「零成本」——每次 pickup/place 多一次數十 KB 的 struct 深拷貝（含 1000 個 AnsiString refcount）。pickup/place 不頻繁，可接受。

## 修改檔案總表

| 檔案 | 變更 |
|---|---|
| aTrayArm.h | include MyMotor.h；`TMyTray ArmTray;` |
| aTrayArm.cpp | InitialFlag 非keep 加 `ArmTray.Clear()`；DoPick c4000 Empty/Color 取 `GetSourceTray()`；DoPlace c4000 存 `RearGrid[]`（**不設 fHasTray**）+ `ArmTray.Clear()` |
| aEmpty.h/.cpp | `SourceTray` + `BirthRearTray()`（兩個 rear latch）+ `GetSourceTray()` |
| aColor.h/.cpp | `SourceTray` + `BirthIdentityTray()`（真實讀取兩處；K_SKIP 用 Normal/不生）+ `GetSourceTray()` |
| aAuto1To6.h/.cpp | `TMyTray RearGrid[6]` + NotifyTrayArmDelivered 收 grid；c7000:527 改接收 `RearGrid[]`+`fHasTray=true`+`Refresh()`；**保留 :597/:792** |

**不動**：Loader `InitNewTray`(:579)、Loader→TrayArm 回收(Phase 6)、`CopyTrayFrom/MoveTrayFrom` 空殼、scalar 路由、Empty/Color `RefreshStateFromSensors`。

## 待你/編輯時確認的點

1. **Normal（非 AMR）路徑的 grid staging**：`SetRearHasTrayFromTrayArm` 是 bool-only，是否也要把 `ArmTray` 存進 `RearGrid[]`（否則 Normal 模式 c7000 沒 grid 可接收，需 fallback 成 `InitNewTray(EMPTY_IC)`）。
2. Auto rear VMotor 的取得方式：優先在 `TAutoModule` 加 `GetRearVMotor(int)` 委派既有 `GetAutoVMotor(int)`，index→VMotor 對應留在 Auto 單一來源。

---

## 附錄：Phase 6 規格（Loader AMR 回收，本次不做）

Loader 進料車與 Auto 同構：`身分Tray + 上蓋 + IC盤(複數)`。以最大 10 盤為例：

| 盤 | 內容 | 回收去向 |
|---|---|---|
| 8 盤 | IC 盤 | 進機消化 |
| 第 9 盤 | 空盤 | **退回 Empty** |
| 上蓋 | 上蓋 | **退回 Empty** |
| 第 10 盤 | 身分 Tray | **退回 Color** |

需新增（目前 Loader 完全沒有、需自 Auto 反向移植）：辨識車內每盤 Kind、依 Kind 路由回收。方向與現況相反（Color 目前是供應源、`DoSortBin` 為空殼）。
