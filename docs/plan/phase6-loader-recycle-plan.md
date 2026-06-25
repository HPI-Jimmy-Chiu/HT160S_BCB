# Phase 6 — Loader AMR 回收計畫（身分盤→Color、空盤/上蓋→Empty）

> 狀態：**DESIGN ONLY**（未動 code）。designer + 對抗式 reviewer，再依使用者 2026-06-25 三點更正修訂。
> 前置：Phase 1-5（盤來源交接）已 commit `b6e7580`。

## 鎖定決策（使用者）
- **D1 回收機制**：TrayArm 反向把 leftover 從 Loader rear 搬回模組（閉迴圈重用）：空盤+上蓋→Empty；身分盤→Color。不是 AGV 載走整車。
- **D2 Kind 來源**：固定堆疊位置慣例（無感測，軟體依進料順序推算）。✅ **身分盤=最後取到**（`feedSerial==total ⇒ Identity`）。
- **D3 上蓋**：上蓋當一般空盤回 Empty（Empty 不用改）。

## 2026-06-25 使用者三點更正（已併入）
- **U1**：計畫缺少「**Color 接收盤**」功能 → 列為 Sub-phase C 的第一級能力（收到的盤實體上是空盤，帶身分 2D）。
- **U2**：Color 接收盤**參考 Empty 收盤流程**（`RequestReturnTray` + `DoGoUpTray` 堆疊回供應車 + `NotifyTrayXToEmptyFinish`）。Color 與 Empty **同一組 destacker 缸**（`C_Color_FrontRiseTray_1/_2` + `FrontSeparateTray_1` + `PushTray`/`LeanOnTray`，見 `DoGoDownTray` :296），可忠實鏡像。原本我自創的單缸 push 作廢。
- **U3**：Loader 端是「**盤資料轉移到 TrayArm**」（盤被 TrayArm 收走），不是「Loader 清除盤資料」。`ClearTray` 是延伸/周邊問題。對齊 Phase 1-5「born-at-source、交接點轉移」原則。
- **U4（2026-06-25）**：**Empty 與 Color 是同一種機構**。Empty 供應空盤（含「不放 IC 的空盤」＝上蓋），Color 供應身分 Tray。⇒ Sub-phase C 的 Color 接收盤＝**把 Empty 的 `DoGoUpTray`/`RequestReturnTray`/`NotifyTrayXToEmptyFinish` 近乎逐字移植**（缸組相同、只換 `C_Empty_*`→`C_Color_*` 與 `MoveEmptyY`→`MoveColorY`、教導點 Empty→Color），最低風險。

**衍生範圍**：唯一全新「路由」是身分盤→Color；空盤/上蓋/用完 IC 盤走既有 `DoPlaceToEmpty`。Empty、AGV/SECS **不動**。

---

## 盤資料的「轉移鏈」（U3 的正確模型）

已驗證 Loader topology：`DoDischargeTray` case 100 `IsRearOccupied()` 擋的是**discharge**、不是 feed；一盤停到 rear 後，carriage 被釋放並**重用給下一盤**（`PrepareTrayMap` 會覆寫 `TrayMotor->Tray`）。所以盤資料是一條轉移鏈，每一步都是「移交」非「清除」：

```
carriage TrayMotor->Tray   (feed 時誕生 Kind/2D)
   │  discharge case 3000：實體盤從 carriage 移到 rear，資料轉移到 module 暫存
   ▼
module RearSourceTray       (代表「停在 rear 的盤」；carriage 隨即被下一盤重用)
   │  TrayArm pickup：盤被收走，資料轉移到手臂
   ▼
TrayArm ArmTray             (帶著 Kind/2D 去 Color 或 Empty)
```

`DoDischargeTray` 的 `ClearTray()`(:1326) 只是「**清掉 carriage 讓它接下一盤**」（延伸問題，U3）；盤資料在前一步已轉移到 `RearSourceTray`，沒有遺失。`RearSourceTray` 這個中繼**是必要的**，因為 carriage 會被下一盤覆寫（非可省略的 workaround，而是轉移鏈的一環）。

---

## ⚠ 動工前現場確認（plan §0）
1. **車組成**（剩餘確認）：最大 10 盤 = (a) 身分+上蓋+8IC（鏡像 Auto），或 (b) 8IC+空盤+身分(+上蓋)？因 `GetFedTrayKind` keyed off total 且只認 Identity，組成差異**不影響** helper，但影響 sim 計數顯示。
2. **`C_Color_RearRiseTray` 語意**：Color 多一支 Empty 沒有的後方升缸（目前全程只 Pop，[aColor.cpp:657](HT160S_Program_BCB_V1.0.0.0/aColor.cpp#L657)）。Color 接收主體鏡像 Empty 前堆疊器即可運作；此後方缸是否用於接收堆疊**待機構確認**（不確認就先不 Push 它）。
3. **Color return 教導點**：放盤 X/Y 幾何（新增獨立教導點、初值=pickup）。

---

## Sub-phase A — Loader 依位置標 Kind，資料隨盤轉移（可獨立 build；DescribeState 觀察）

**A.1 `aLoader.h`**：`#include "MotorAndIO/MyMotor.h"`。`TLoaderModule` 新增 private：`int iFeedSerial;`、`eTrayKind RearKind;`、`AnsiString RearTrayID;`、`TMyTray RearSourceTray;`（rear 暫存=轉移鏈中繼）。public：`GetRearTrayKind()/GetRearSourceTray()/GetRearTrayID()`（return-by-value，鏡像 `Empty/Color::GetSourceTray`）。→ struct/header 變更 = `-Full`。
> 註：Kind 在 feed 時寫**在 carriage 的 `Tray` grid 上**（與 Color `BirthIdentityTray` 一致），無需 per-side `FeedKind` 欄位——discharge 時直接從 `TrayMotor->Tray` 轉移到 `RearSourceTray`（含 Kind/TrayID）。

**A.2 `aLoader.cpp`**：`GetFedTrayKind(feedSerial,total)`（D2 慣例：`total⇒Identity, total-1⇒Cover, else Normal`，單一真相點）。`RefillSimInfeed`(:369)/`InitialFlag` 設 `iFeedSerial=0`。

**A.3 feed 時把 Kind 寫上 carriage grid**：`DoFeedTray` case 9000（`PrepareTrayMap` 後 :993）：
```
iFeedSerial++;
eTrayKind k = GetFedTrayKind(iFeedSerial, GetCarTrayCount());
TrayMotor->Tray.SetKind(k);
TrayMotor->Tray.TrayID = (k==eTrayKindIdentity) ? <sim:"LOAD2D_"+ts / 真機:""> : "";
```
（真機不讀 2D，D2；Color 重用時 `DoReadColor2D` 會重讀並重生 TrayID。）

**A.4 discharge 時把資料轉移到 rear 暫存（U3）**：`DoDischargeTray` case 3000，在 `ClearTray()`(:1326) **之前**：
```
RearKind       = TrayMotor->Tray.GetKind();
RearTrayID     = TrayMotor->Tray.TrayID;
RearSourceTray = TrayMotor->Tray;        // 整盤 grid 轉移到 rear 暫存
```
`ClearTray()` 維持不動（只是釋放 carriage 接下一盤）。註記：`bRearHasTray=true`(case 2000) 領先此轉移(case 3000) 一個 pass，安全——Kind 只在 TrayArm pickup(B.3) 才讀。

**A.5 accessor bodies + pickup 轉移後清除**：`GetRearTrayKind/GetRearSourceTray/GetRearTrayID` 回傳對應欄位。`NotifyTrayArmPickRearTray`(:548) 除 `bRearHasTray=false` 外加 `RearKind=Normal; RearTrayID=""; RearSourceTray.Clear();`（盤已轉移到手臂；延伸 Phase 1-5「cleared rear ⇒ cleared grid」不變量）。

**A.6 DescribeState**(:1497)：加 `RearKind/RearTrayID/iFeedSerial`（FeederDecision.txt 現場驗證 §0）。

**A.7 build**：`-Full` + SOFT_SIMULATE-off + 還原 + 編碼檢查 + selftest-home。A 無行為改變。

---

## Sub-phase B — TrayArm 依 Kind 轉移路由（身分→Color）（依賴 A）

**B.1 `aTrayArm.h`**：`eTrayArmPlaceDest`(:31) 加 `TAPLACE_COLOR`；private 加 `bool DoPlaceToColor(int Flag);`。→ `-Full`。

**B.2 DecideJob**(:211-216)：LOADER_RECOVERY 把硬編 `iDeliverKind=eTrayKindNormal`(:214) 改 `iDeliverKind=(int)LoaderModule->GetRearTrayKind(); iDeliverTrayID=LoaderModule->GetRearTrayID();`。

**B.3 DoPick** case 4000 Loader 分支(:348-355)：把 `ArmTray.Clear()` stub(:354) 改成**轉移**（copy 在 `NotifyTrayArmPickRearTray` 之前）：
```
ArmTray        = LoaderModule->GetRearSourceTray();   // 盤資料轉移到手臂(U3)
iDeliverTrayID = LoaderModule->GetRearTrayID();
LoaderModule->NotifyTrayArmPickRearTray();            // 之後才清 rear 暫存
```

**B.4 DecidePlaceDestAfterPick**(:463) 最前面加 Kind 閘：
```
if(iDeliverKind==eTrayKindIdentity){ PlaceDest=TAPLACE_COLOR; iAutoTarget=-1;
    if(ColorModule) ColorModule->RequestReturnTray();   // 同 Empty 收盤契約 (U2)
    return; }
```
Cover/Normal 落到既有 Auto-vs-Empty 邏輯（不變，D3）。

**B.5 DoPlace dispatch**(:397) 加 `if(PlaceDest==TAPLACE_COLOR) return DoPlaceToColor(Flag);`。
**`DoPlaceToColor`**＝**`DoPlaceToEmpty`(:503) 的逐行對應**，只換目標：MoveX→Color return X；case1000 `ColorModule->IsRearHasTray()==false`（等 Color 騰出 rear，同 Empty）；case5000 `ColorModule->NotifyTrayXToEmptyFinish()`（**沿用同名收盤契約**，U2）→ `ArmTray.Clear(); fHasTray=false`。`MoveTrayArmX`(:119) 已含 Z-up 互鎖。

**B.6 teach**：新增 `TrayXArmToColorReturnXPosition`（uteach，初值=`TrayXArmToColorXPosition`，可獨立教導；避免放盤撞盤）。→ `-Full`。

**B.7 build**：`-Full` + SOFT_SIMULATE-off + 還原；sim 強制 RearKind=Identity 驗 PlaceDest=COLOR、未走 Empty。

---

## Sub-phase C — Color 接收盤能力（U1，鏡像 Empty 收盤 U2）+ 閉迴圈重用 + 單一 MColorY 仲裁

> **Color 今天是純供應源、完全沒有接收盤能力。** 本 sub-phase 把 Empty 的收盤流程（`RequestReturnTray`/`DoGoUpTray`/`NotifyTrayXToEmptyFinish`）忠實移植到 Color，用 Color 同款 destacker 缸。**API 用與 Empty 相同的名字**，使 `DoPlaceToColor` 與 `DoPlaceToEmpty` 完全同形。

**C.1 `aColor.h`**：private 加 `int GoUpTask; bool bReturnTray; bool bTrayXToEmptyFinish; int iReturnedCount; HTimer GoUpDelay;`。public 加 `RequestReturnTray()`、`IsRearHasTray()`、`NotifyTrayXToEmptyFinish()`（簽章/語意比照 [aEmpty.cpp:791/797](HT160S_Program_BCB_V1.0.0.0/aEmpty.cpp#L791)）。→ header `-Full`。

**C.2 `aColor.cpp` DoGoUpTray（鏡像 Empty DoGoUpTray :493）**：用 Color 對應缸——`C_Color_FrontRiseTray_1`→`FrontSeparateTray_1`→`FrontRiseTray_2`→…→`Pop FrontRiseTray_1`（把 rear 盤堆回前供應車、騰出 rear），中間 `MoveColorY` 在 `ColorTrayArmPickYPosition`(rear)↔`ColorRead2DYPosition`(front) 之間，`PushTray`/`LeanOnTray` 轉移盤。收尾設 `bFrontHasTray=true; bRearHasTray=false;`（rear 騰空待手臂寄放）。**`C_Color_RearRiseTray` 僅在機構確認其為堆疊缸後才納入**（§0 確認 2）。

**C.3 `aColor.cpp` DoColor 接收分支（鏡像 Empty DoEmpty case 100/3000 :191-245）**：
```
case 100: ... if(bReturnTray){ DoGoUpTray(0); Task=<新>; break; }   // 接收優先，插在 godown 之前(見 C.4)
case <新>: if(DoGoUpTray(1)){ if(bReturnTray && bTrayXToEmptyFinish==false) return;  // hold 等手臂寄放
                              bReturnTray=false; iReturnedCount++; iSimInfeedCount++; Task=...; }   // 重入供應池(閉迴圈)
```
`RequestReturnTray()`：`bReturnTray=true; bTrayXToEmptyFinish=false;`。`NotifyTrayXToEmptyFinish()`：`bTrayXToEmptyFinish=true; bRearHasTray=true;`（手臂寄放完成、盤入 rear；**sim 也只由此觸發、不可 auto-advance**，比照 Empty 唯一觸發點）。

**C.4 單一 MColorY 仲裁（CRITICAL）**：接收 dispatch **必須插在 supply 的 godown 分支之前**，且 godown 條件加 `&& !bReturnTray`（否則 sim 下 `IsSoftSimulate()` 恆真使 godown 每 idle pass 搶走、接收餓死）。supply 分支(:255)加 `!(bReturnTray)` 守衛；在 dispatch 擋、需求 latch 不掉。單一 `Task` 擁有權 ⇒ 不死鎖。ASCII 註解寫明。

**C.5 teach**：新增 `ColorReturnYPosition`（初值=`ColorTrayArmPickYPosition`，可獨立教導）。

**C.6 閉迴圈重用 + DescribeState**：接收的身分盤入前供應堆（`iSimInfeedCount++`），下次 `DoSupplyTray`→`DoReadColor2D`→`BirthIdentityTray()`(:616) 自然重生 Kind=Identity+TrayID（用既有 birth，無新 code）。`DescribeState`(:928) 加 `bReturnTray/bTrayXToEmptyFinish/GoUpTask/iReturnedCount`。

**C.7 build**：`-Full` + SOFT_SIMULATE-off + 還原；sim 端到端：Loader RearKind=Identity → TrayArm PlaceDest=COLOR → Color DoGoUpTray 騰 rear → 手臂寄放 → NotifyTrayXToEmptyFinish → iReturnedCount++ → 後續 supply 重現身分盤；同時下 supply 不會 mid-return 啟動（仲裁）。

---

## Sub-phase D — Config / 診斷 / 未動模組
- 無新 config（組成由 `GetFedTrayKind`+既有 `iSimAmrMaxTray[0]` 衍生）。
- 診斷：Loader(A.6)+Color(C.6) DescribeState。
- **Empty `aEmpty.*`：不動**（D3）。**AGV/SECS：不動**（D1 閉迴圈）。

## Build/驗證通則
每 sub-phase：byte-safe（`bcb6-bytesafe-edit.ps1`/python latin1，ASCII 註解）→ `-Full` exit0 → SOFT_SIMULATE off `-Full` exit0 → 還原 → 編碼檢查 → selftest-home。無 C++11/FSM；alarm 用 ShowMyError/Note；互鎖只 bypass compile-time SOFT_SIMULATE。

## Reuse ledger
`GetNextTrayKindForAuto`→`GetFedTrayKind`；**`Empty::DoGoUpTray`+`RequestReturnTray`+`NotifyTrayXToEmptyFinish`→Color 接收盤(U2)**；`Color::DoGoDownTray` 缸組→Color `DoGoUpTray` 反向；`DoPlaceToEmpty`→`DoPlaceToColor`(同契約名)；`BirthIdentityTray`→閉迴圈重生；`Empty/Color::GetSourceTray`→`GetRearSourceTray`。
