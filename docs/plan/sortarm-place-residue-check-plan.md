# SortArm Place 殘料檢查 Port Plan（綁目標 Auto、人工介入）

狀態：IMPLEMENTED（非阻塞/背景化版，sim+real build exit 0；待實機驗證）
> 注意：本文 §4.2、§6 的逐段 code 範例仍是初版「case 70 同步等待」寫法,保留作對照;
> 實際建置採 §4.1 的**非阻塞背景化**設計(case 60 立即 return、`CheckPlaceResidue` 於 `DoSortArm` 背景跑、pick case 45 gate)。
日期：2026-06-24
作者：Claude (ht160s-maintainer)
參考：HT172 `CheckSortArmDestroyActive`（`HT172_Program_V1.0.25.0_20260420/HT172_Module/aSortArm.cpp`）

---

## 1. 需求（使用者鎖定）

> SortArm 把 IC place 到 Auto1~6 後，吸嘴上升到最高點要檢查有無殘料；
> 確定「無殘料」且「Auto 盤已滿」，該 Auto 才能離開（卸盤 / AMR 取走）。

三個決策點（使用者已拍板）：

1. **gate 範圍**：殘料檢查結果**嚴格綁到「那一次 place 的目標 Auto」**（per-Auto，非全域）。
2. **檢查對象**：**只針對當下 place 動作實際放料的吸嘴/IC**（`bPlaceSelected` 的 slot），不是整支 4 嘴全檢。
3. **殘料處置**：照 HT172 做法 —— **alarm 請操作員把 IC 取下（人工介入確認）**，沿用 `eSuckDestroyErr` 流程。

---

## 2. 現況 vs HT172

### HT160S 現況（無殘料檢查）
- `DoPlaceToAuto`（`aSortArm.cpp:1170`）：case 50 `DestroySelectedSlots()` 破真空 + `TransferPlaceDataToAuto()` 寫資料 → case 60 `SortArmZToSafePos()` 吸嘴上升 → `return true`。**吸嘴上升後直接結束，無二次確認。**
- `TMySucker::Destroy()`（`MyKitSuck.cpp:180`）REALLY 模式只確認「破真空後**真空壓力消失**」（`Sensor.IsOn()==false`），這只證明破壞動作有效，**不等於 IC 真的離開吸嘴**。
- Auto 卸盤唯一 gate = `bFullIC`（`FindDischargeAuto` `aAuto1To6.cpp:391`）；AMR Ready = `IsDrainedForAmr`（`aAuto1To6.cpp:969`）也只看盤堆完 + `!bFullIC`。
- grep 確認 HT160 完全沒有 `CheckSortArmDestroyActive` / `bSortArmCheckDestroyACT` / `IsCheckSortArmDestroyActiveFinish`。

### HT172 殘料檢查機制（要參考的核心）
- 放料時設旗標（`aSortArm.cpp:873`）`bSortArmCheckDestroyACT[iR][iC]=true`。
- 背景每 cycle 跑 `CheckSortArmDestroyActive()`（`aSortArm.cpp:404`）：`OffDestroy()` → **`OnSuck()` 重吸真空** → 等 `dDestroyCheckTime` → 讀 `GetStatus()`：
  - `true`（重吸吸得到 = 吸嘴口被 IC 堵著 = 殘料）→ alarm `eDestoryErr`「請將 IC 取下」K_RETRY → 重試。
  - `false`（漏氣吸不到 = 空 = 無殘料）→ 完成、`Normal()`、清旗標。
  - DUMMY / HAS_TRAY 跳過。
- gate 點：HT172 擋在「**下次 pick 前**」（`IsCheckSortArmDestroyActiveFinish` `aSortArm.cpp:1730`）。

### 本質差異（殘料檢查的精髓）
**主動重吸一次**：吸得起真空 = 吸嘴口仍被 IC 堵著（殘料）；吸不起（漏氣）= 吸嘴是空的。比單純看「真空消失」更可靠，能抓到「真空已破但 IC 仍黏吸嘴」的漏網情況。

### HT160 與 HT172 落地差異（本案要新增的）
| 項目 | HT172 | HT160（本案） |
|------|-------|--------------|
| 檢查觸發 | place 後設旗標，背景持續檢查 | **吸嘴上升到最高點（place FSM case 60 之後）才檢查** |
| gate 點 | 下次 pick 前 | **Auto 卸盤 / 離開前** |
| gate 綁定 | 整支吸嘴 | **目標 Auto（per-Auto）** |
| 吸嘴數 | 8（R×C） | 4（Slot[4]，線性 slot） |

---

## 3. 移植可行性（高）

HT160 `TMySucker` 已具備全部所需 API：
| 用途 | HT160 API |
|------|-----------|
| 讀吸嘴真空狀態 | `GetStatus()` `MyKitSuck.cpp:69`（REALLY 回 `Sensor.IsOn()`；**非 REALLY 回 `true`** ⚠） |
| 重吸真空 | `OnSuck()` `MyKitSuck.cpp:76` |
| 破真空 | `OffDestroy()` `MyKitSuck.cpp:91` |
| 復歸 | `Normal()` `MyKitSuck.cpp:108` |
| alarm | `ShowSuckError(TMySucker&, CodeType, KCode, Region)` `note.h:151`；`eSuckDestroyErr` `MyKitSuck.h:16`，`_SuckDestroyErr` 已建於 `database.cpp:874` |
| 計時 | `HTimer`：`SetMS(int)` + `On()`，`Off()` 偵測到期（`HTimer.h`，**無 SetSec/SetSecAndOn**） |

⚠ **GetStatus 語意陷阱**：HT160 非 REALLY 回 `true`（與 HT172 DUMMY 回 false 相反）。對策：殘料檢查在 DUMMY/HAS_TRAY/SoftSimulate **一律跳過**（沒有真空 sensor，殘料檢查無意義），就不會踩到。與既有「sucker 真空相關只在 REALLY 做」一致。

缺口：`dDestroyCheckTime` 參數 HT160 沒有 → 先用常數（建議 300ms），未來可參數化。

---

## 4. 設計

### 4.1 資料流總覽（**非阻塞 / 背景化，對齊 HT172** — UPH 中性）

> 設計演進：初版把殘料檢查放在 place FSM 的 case 70 同步等待（吸嘴停在 Auto 上方等 ~600ms），
> 會直接拖慢節拍。**已改為 HT172 式非阻塞**：place 完吸嘴一上升就 return，殘料重吸驗證在
> SortArm 飛回 pick 的移動途中由背景跑完，pick 下降前才 gate。殘料檢查時間藏進移動時間，UPH 幾乎不受影響。

```
DoPlaceToAuto（非阻塞）:
  case 50: DestroySelectedSlots() 成功
           -> MarkResidueTargets()                              // 趁 bPlaceSelected 還在，標記本次放料 slot
           -> iResidueAutoIndex = iActiveAutoIndex              // 記住要回報的目標 Auto
           -> AutoModule->SetPlaceResidueClear(iActiveAutoIndex, false)  // 目標 Auto 殘料未清(pending)，擋住卸盤
           -> TransferPlaceDataToAuto()                         // 寫資料 + ClearSlot
           -> case 60   // 注意：此時「尚未」arm，吸嘴還在最低點，CheckPlaceResidue 不會動
  case 60: SortArmZToSafePos()（吸嘴到最高點）-> bResidueArmed=true; PlaceTask=1; return true
           // 關鍵：殘料檢查只在吸嘴上升到最高點(case 60)後才 ARM，
           // 絕不在 tray 上方/最低點重吸（否則會把剛放下的 IC 吸回來）；立即收工，不等檢查

DoSortArm（每 cycle 開頭，背景推進）:
  CheckPlaceResidue();   // if(!bResidueArmed) return;  -- 上升到最高點前完全不重吸
                         // armed 後對 bNeedResidueCheck 的 slot：破真空->重吸->等->讀 GetStatus
                         //   無殘料 -> 清旗標；全部清 -> SetPlaceResidueClear(iResidueAutoIndex,true) + disarm
                         //   有殘料 -> ShowSuckError(eSuckDestroyErr, K_RETRY) 人工取下後 retry
                         //   sim/DUMMY/HAS_TRAY -> 視為立即清 + 回報 Auto + disarm（無真空 sensor）

DoPickFromLoader:
  case 45: if(IsResidueCheckBusy()) break;   // 重吸未完成不下降，避免與 pick 取料吸氣衝突
           -> MovePickZDown -> case 50 SuckSelectedSlots()

Auto 卸盤 / 離開 gate:
  FindDischargeAuto():  State[i].bFullIC && State[i].bResidueClear
  IsDrainedForAmr(i):   ... && State[i].bResidueClear
```

### 4.2 SortArm 端（aSortArm.h / aSortArm.cpp）

新增 private 成員：
```cpp
bool   bNeedResidueCheck[4];   // 本次 place 放料的 slot（殘料檢查對象）
int    ResidueTask[4];         // per-slot 殘料檢查狀態機 (1/200/300)
HTimer ResidueDelay[4];        // per-slot 重吸等待計時
```
（`#include "HTimer.h"` 於 aSortArm.h，比照 aAuto1To6.h）

新增 private 方法：
```cpp
void MarkResidueTargets();     // for slot: if bPlaceSelected -> bNeedResidueCheck=true, ResidueTask=1
bool CheckPlaceResidue();      // 移植 HT172 CheckSortArmDestroyActive，per-slot、只跑 bNeedResidueCheck
```

`CheckPlaceResidue()` 狀態機（每呼叫推進一次，非阻塞）：
```cpp
bool TSortArmModule::CheckPlaceResidue()
{
    if(IsSoftSimulate())
        return true;                       // sim 無真空，視為已清
    bool bAllDone=true;
    for(int s=0; s<SORT_ARM_SUCKER_COUNT; s++)
    {
        if(bNeedResidueCheck[s]==false)
            continue;
        if(HSys.LastSet.iRealDummy!=REALLY) // DUMMY/HAS_TRAY 跳過（GetStatus 語意陷阱）
        {
            bNeedResidueCheck[s]=false;
            continue;
        }
        TMySucker *Sk=GetSucker(s);
        if(Sk==NULL){ bNeedResidueCheck[s]=false; continue; }
        switch(ResidueTask[s])
        {
            case 1:
                Sk->OffDestroy();
                ResidueDelay[s].SetMS(iDestroyCheckMS); ResidueDelay[s].On();
                ResidueTask[s]=200; bAllDone=false; break;
            case 200:
                Sk->OnSuck();                                   // 主動重吸
                ResidueDelay[s].SetMS(iDestroyCheckMS); ResidueDelay[s].On();
                ResidueTask[s]=300; bAllDone=false; break;
            case 300:
                if(ResidueDelay[s].Off())
                {
                    if(Sk->GetStatus())                          // 吸得到 = 殘料
                    {
                        ShowSuckError(*Sk, 2, K_RETRY, "SortArm Residue");  // 人工取下 IC
                        ResidueTask[s]=200;                      // retry 重吸
                        bAllDone=false;
                    }
                    else                                         // 無殘料
                    {
                        Sk->Normal();
                        bNeedResidueCheck[s]=false;
                        ResidueTask[s]=1;
                    }
                }
                else bAllDone=false;
                break;
            default: ResidueTask[s]=1; break;
        }
    }
    return bAllDone;
}
```

`DoPlaceToAuto` 改動（`aSortArm.cpp:1215-1229`）：
```cpp
case 50:
    if(DestroySelectedSlots())
    {
        MarkResidueTargets();                                   // 新增（在 Transfer/ClearSlot 之前）
        if(AutoModule!=NULL)
            AutoModule->SetPlaceResidueClear(iActiveAutoIndex, false);  // 新增
        TransferPlaceDataToAuto();
        PlaceTask=60;
    }
    break;
case 60:
    if(SortArmZToSafePos())
        PlaceTask=70;                                           // 改：原本直接 return true
    break;
case 70:                                                        // 新增
    if(CheckPlaceResidue())
    {
        if(AutoModule!=NULL)
            AutoModule->SetPlaceResidueClear(iActiveAutoIndex, true);
        PlaceTask=1;
        return true;
    }
    break;
```

`InitialFlag()` / `ClearPlaceSelection()`：殘料旗標歸零（`bNeedResidueCheck[*]=false; ResidueTask[*]=1; ResidueDelay[*].Clear();`），避免 reset/pause 後卡住。

### 4.3 Auto 端（aAuto1To6.h / aAuto1To6.cpp）

`TAutoStationState` 加欄位（`aAuto1To6.h:18`）：
```cpp
bool bResidueClear;   // 該 Auto「最後一次 place 殘料已確認清除」；預設 true
```

新增 public 介面（給 SortArm 推送結果）：
```cpp
void SetPlaceResidueClear(int Index, bool bClear);   // 邊界檢查 + 設 State[Index].bResidueClear
```

卸盤 gate：
- `FindDischargeAuto()`（`aAuto1To6.cpp:391`）：
  `if(State[Index].bFullIC && State[Index].bResidueClear) return Index;`
- `IsDrainedForAmr()`（`aAuto1To6.cpp:969`）：條件加 `&& State[Index].bResidueClear`。

預設值 / 重設（關鍵，避免卡死）：
- `TAutoModule()` ctor + `InitialFlag()`：`State[i].bResidueClear=true`。
- 卸盤完成（`DoDischargeTray` case 1000 清盤處 `aAuto1To6.cpp:600` 附近）：`State[i].bResidueClear=true`（新盤無殘料疑慮）。
- AMR `ClearAmrCar()`（`aAuto1To6.cpp:1004`）：`State[Index].bResidueClear=true`。

---

## 5. 時序驗證（為何不會卡 / 不會誤放）

| 步驟 | bFullIC | bResidueClear | 卸盤 gate |
|------|---------|---------------|-----------|
| 初始（空盤） | false | true | 不卸（bFullIC=false） |
| place 開始(case 50) | false→可能 true | **false(pending)** | **擋住**（即使填滿） |
| 吸嘴上升(case 60) | — | false | 擋住 |
| 殘料檢查通過(case 70) | true | **true** | **放行** ✓ |
| 殘料檢查失敗 | true | false | 擋住 + alarm 人工介入 → retry 通過後 true → 放行 |
| 卸盤完成 | false | true(重設) | 下一輪 |

中途 place 未填滿盤：bResidueClear 在 false↔true 間循環，但 bFullIC=false 不卸盤，無影響；最後填滿那次 place 的 case 70 通過後放行。

---

## 6. 檔案改動清單

| 檔案 | 改動 |
|------|------|
| `aSortArm.h` | `#include "HTimer.h"`；新增 `bNeedResidueCheck[4]` / `ResidueTask[4]` / `ResidueDelay[4]`；宣告 `MarkResidueTargets()` / `CheckPlaceResidue()`；常數 `iDestroyCheckMS`（或 `#define`） |
| `aSortArm.cpp` | `DoPlaceToAuto` case 50/60/70 改動；實作兩個新方法；ctor/`InitialFlag`/`ClearPlaceSelection` 歸零旗標 |
| `aAuto1To6.h` | `TAutoStationState::bResidueClear`；public `SetPlaceResidueClear()` |
| `aAuto1To6.cpp` | `FindDischargeAuto` + `IsDrainedForAmr` gate；`SetPlaceResidueClear` 實作；ctor/`InitialFlag`/卸盤完成/`ClearAmrCar` 重設 `bResidueClear=true` |

新增註解一律 ASCII English（BCB6 規範）。改 `.cpp/.h` 後刪對應 `.obj` 再編譯。

---

## 7. 分階段（每階段獨立編譯 exit 0）

- **P1 — Auto 端骨架**：加 `bResidueClear`（預設/重設 true）+ `SetPlaceResidueClear` + gate；此時 SortArm 還沒呼叫 setter，行為 == 現況（恆 true）。純結構，可先驗證不破壞既有流程。
- **P2 — SortArm 殘料檢查**：加旗標 + `MarkResidueTargets` + `CheckPlaceResidue` + place FSM case 50/60/70；接上 `SetPlaceResidueClear`。完整功能。
- **P3 — 驗證**：`build-ht160s.ps1 -Full`（結構改動）；`SOFT_SIMULATE` 開（sim 跳過殘料，流程不卡）；再關 `SOFT_SIMULATE` 跑 `-Full` 確認實機分支 exit 0 後還原。`ht160s-home-selftest` 回歸。
- **P4（選配）— 參數化**：`iDestroyCheckMS` 移到設定（teach/database），比照 HT172 `dDestroyCheckTime`。

---

## 8. 風險 / 待確認

- **UPH 影響（已解決）**：初版同步 case 70 會拉長 place cycle ~600ms。**已改背景化**（§4.1）：place 完立即 return，重吸驗證在 SortArm 飛回 pick 的移動途中由 `CheckPlaceResidue` 背景跑完，pick 下降前以 `IsResidueCheckBusy` gate（通常已完成，0 等待）。UPH 幾乎不受影響，對齊 HT172 非阻塞。新增 `iResidueAutoIndex` 記住要回報的目標 Auto；新增 `IsResidueCheckBusy()` 作 pick gate。
- **alarm KCode**：採 `K_RETRY`（強制處理，符合「確定無殘料才離開」）。是否要併 `K_SKIP`（誤判/取不下時的逃生門，但 SKIP=帶殘料放行）待確認 —— 預設不給 SKIP。
- **ShowSuckError CodeType**：沿用 Destroy 類的 `CodeType=2`（與 `DestroySelectedSlots` 一致）；需於實作時核對 `ShowSuckError` 內部 CodeType→訊息對應，確認顯示「Destroy/殘料」語意。
- **pause 中途**：`actuator-timer-pause-scoped` 記憶 —— 殘料 `ResidueDelay` 屬 SortArm actuator timer，pause 時是否需納入 `PauseActuatorTimeoutTimers`，避免 resume 後誤判到期。實作時對齊既有 sucker timer 處理。
- **HAS_TRAY**：本案在 HAS_TRAY 也跳過殘料檢查（GetStatus 語意陷阱）。HT172 的 Destroy 在 HAS_TRAY 有完整 sensor 檢查，但殘料的「重吸」需真實真空 → 仍只 REALLY 做，符合 `realdummy-three-tier-io-check`。

---

## 9. 驗證清單（交付前）
- [ ] `-Full` 編譯 exit 0（SOFT_SIMULATE 開）
- [ ] `-Full` 編譯 exit 0（SOFT_SIMULATE 關）後還原 define
- [ ] `check-ht160s-source-encoding.ps1` 通過
- [ ] `ht160s-home-selftest` 全軸 HOMED
- [ ] 使用者實機驗證：填滿一個 Auto → 確認殘料檢查發生在吸嘴最高點 → 無殘料才卸盤；人為留殘料 → alarm 人工介入
