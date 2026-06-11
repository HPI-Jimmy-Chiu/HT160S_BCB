---
name: ht160s-simulation-data
description: >-
  Use when working on the HT160S_BCB offline simulation / virtual-IC test flow:
  tSimuData.bRunSimulation semantics, btnLoadSimuData seeding 5 lots x 20 unique
  2D codes into LotRegistry + sgLotList, Loader ReadTopCcd2DCode virtual-code
  cycling (SimuCcdCycleIndex / GetCode2DByIndex), and how simulation differs from
  IsSoftSimulate(). Triggers: tSimuData, bRunSimulation, btnLoadSimuData,
  SimuCcdCycleIndex, GetCode2DByIndex, ReadTopCcd2DCode, cbEnableSimulation,
  SIMU_LOT, 2D_Simu, IsSoftSimulate, virtual IC, simulation.
---

# HT160S Simulation / Virtual-IC Data Flow

## 1. 兩個獨立的「模擬」概念（別混用）

| 機制 | 旗標 / 函式 | 語意 |
|------|-------------|------|
| **執行期模擬資料** | `tSimuData.bRunSimulation` (runtime bool) | 「Loader 進來的盤要灌**虛擬 IC 資料**」做離線測試。**不是**硬體 bypass — 一般仍可走流程，只是 2D code 來自自建資料而非真相機。 |
| **編譯期軟模擬** | `IsSoftSimulate()` / `SOFT_SIMULATE` | 整機軟體模擬（無硬體）。`FormShow` 在此編譯旗標下自動勾 `cbEnableSimulation` 並 `btnLoadSimuData->Click()`。 |

> 不要把 `bRunSimulation` 拿來當「跳過 sensor / cylinder」的 bypass 條件。

## 2. 種子資料：btnLoadSimuDataClick（main.cpp）

按下後（或 SOFT_SIMULATE 開機自動觸發）：
1. `tSimuData.Clear(); tSimuData.bRunSimulation = cbEnableSimulation->Checked;`
2. `LotRegistry.Clear(); SetupLotListGrid();`
3. 建 **5 個 Lot** `SIMU_LOT_A..E`，每 Lot **20 個唯一 2D code**：
   - code = `"2D_Simu_" + CodeSeq`（1..100，全程不重複）
   - `Bin = ((CodeSeq-1) % 6) + 1`（Bin 在 1..6 輪流）
   - 同時寫入 `sgLotList`（UI 清單）與 `LotRegistry`（反查表）
4. `edLotNo->Text = "SIMU_LOT_A";`

結果：`LotRegistry.GetItemCount()` = 100、`GetLotCount()` = 5。

## 3. Loader 端輪循：ReadTopCcd2DCode（aLoader.cpp）

`TLoaderModule::ReadTopCcd2DCode(LoaderNo, CellX, CellY, bool &bOk)`：

```cpp
if(tSimuData.bRunSimulation)
{
    int Total = LotRegistry.GetItemCount();
    if(Total > 0)
    {
        if(SimuCcdCycleIndex < 0 || SimuCcdCycleIndex >= Total)
            SimuCcdCycleIndex = 0;
        sCode = LotRegistry.GetCode2DByIndex(SimuCcdCycleIndex);
        SimuCcdCycleIndex = (SimuCcdCycleIndex + 1) % Total;   // wrap
        bOk = true;
    }
    return sCode;                 // never touches TopCcdSocket
}
// real path: TopCcdSocket->TopCcdPoll() + TopCcdGetResult(sCode)
```

- `SimuCcdCycleIndex`：`TLoaderModule` 私有成員，於 `InitialFlag()` 歸 0。
- `THT160LotRegistry::GetCode2DByIndex(int Index)`（CosFunction.cpp）：回傳排序索引
  第 Index 個 2D code（`m_Code2DIndex->Strings[Index]`），越界回 `""`。
- 每呼叫一次回下一個 code，到 `Total` 後 wrap → 形成「虛擬 IC 輪循輸入」。

## 4. 下游：DoCcdCheck 指派 Bin

模擬與正式共用同一條 Bin 指派：
- state 5000：觸發拍照。`TopCcdTriggerShot()` 必須 `if(TopCcdSocket!=NULL)` 包住，
  模擬無硬體時直接進 5500。
- state 5500：`ReadTopCcd2DCode` 取 code → `LotRegistry.FindByCode2D(sCode, HitLot,
  Bin, HitLotIndex)` → `SetTrayBin` + `OnSorted`。

因為模擬 code 必在 registry 中，`FindByCode2D` 一定命中，Bin 即 §2 的
`((CodeSeq-1)%6)+1`。

## 5. 注意事項

- 模擬輪循依賴 `LotRegistry` 已被 `btnLoadSimuData` 種子化；若 registry 空
  （`GetItemCount()==0`），`ReadTopCcd2DCode` 回 `bOk=false`（保持等待）。
- `GetCheckResult`/`UseCCD==false`/`IsSoftSimulate()`/`bRunSimulation` 任一成立時，
  `GetTopCcdBin`-類檢查直接回 `HAS_OK_IC`（aLoader.cpp 上方函式），避免無硬體卡住。
- 模擬資料屬測試輔助，**不要**據此跳過 cylinder/sensor 真實動作。
