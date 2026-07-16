# 第 14 章　各模組運作流程

本章逐一拆解 HT160S 五個製程模組的運作流程：`Loader` 進料、`Color` 顏色（身份盤供應）、`Empty` 空盤、`SortArm` 分類臂（含 4 個吸嘴）／`TrayArm` 盤臂、`Auto1~6` 出料堆疊。每個模組以「功能 → 動作時序 → 互鎖與安全 → 操作員可見狀態」的順序說明，重點放在各模組之間的防撞互鎖與跨模組交握，這些是整機能否安全連續運轉的工程核心。

所有模組共通的設計原則：

- 皆為**純邏輯／控制模組**，多數無自身畫面，由主控迴圈 (`database.cpp ProcessMotion`) 以非阻塞 `switch(Task)` 程序式階梯逐拍呼叫，子動作回傳 `true` 表示完成。
- 不使用 FSM；所有等待以「回傳 false 讓上層輪詢重試」實作，不使用 `Sleep()`／modal 阻塞（警報彈窗除外）。
- 教導位置與編碼器位置以 **1/100mm** 為單位（100 units/mm）。
- 防撞互鎖（吸嘴回原點、Z 上位確認等）**只在編譯期 `SOFT_SIMULATE` 模擬版被略過**；實機 `DUMMY/HAS_TRAY/REALLY` 三種模式皆持續生效，因為 `DUMMY` 下馬達與氣缸仍會實際運動。

---

## 14.1　Loader 進料模組（TLoaderModule）

### 14.1.1　功能

進料模組負責雙側（`Loader1` 左／`Loader2` 右）來料盤的取放、頂部 CCD 掃描索引、與空盤排出。每一側透過共用的 **Loader-Y 軸**把盤子移到取盤位、用前置疊盤氣缸分出一盤、夾緊上料，再帶到 **Top CCD** 下方逐格掃描每顆 IC（讀 Bin 與 2D 條碼），結果寫入該側盤面資料供 `SortArm` 取料分類。CCD 掃描完成的一側標記為可被 `SortArm` 接管（`LS_READY_SORT`）。盤面全部排空後排到後方供 `TrayArm` 取走。

### 14.1.2　主畫面可見控制項

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `grpLoaderL` | groupbox | 左側 Loader（Loader1）盤面區塊標題（含格點顯示與盤號／分類 Bin 標籤） |
| `grpLoaderR` | groupbox | 右側 Loader（Loader2）盤面區塊標題 |
| `lblLoadCurrID_1` | panel/label | 左側 Loader 目前盤子的識別／盤號顯示 |
| `lblLoaderCarID` | panel/label | 右側 Loader 目前盤子的識別／盤號顯示 |
| `mtLoaderL` / `mtLoaderR` | grid (TTMyTray) | Tray Status 分頁左右盤面格點（預設 4×5），以 `SetSubHTrayPanel` 鏡射該 Loader 車道生產盤內容（每格 IC 掃描/分類狀態） |
| `mtLoaderLTrayWork` | grid (TTMyTray) | 左側 Loader-Y 移動中盤面（綁定 MLoaderY_1/MMLoaderY_1） |
| `mtLoaderRTrayWork` | grid (TTMyTray) | 右側 Loader-Y 移動中盤面（綁定 MLoaderY_2/MMLoaderY_2） |
| `chkLoadTray` | checkbox | 模擬/DUMMY 下決定空料時是否持續供盤（勾選＝視為永遠有盤；未勾＝回報 `Loader Tray Empty`）。實機不使用，由推盤氣缸 On 感測器判定有無盤 |

> 註（定案）：`lblLoadCurrID_1`／`lblLoaderCarID` 為靜態 Panel（Caption 固定 "Loader ID"），全程式**無任何寫入點**——HT172 靠 `SetIDPanel` 接線並掛 SVID 1080/1081，HT160S 移植時未接，屬**死 UI（保留）**；`lblLoadCurrBin_1` 元件在 HT160S 不存在（僅 DFM 有 "Current Sorting Bin :" 靜態文字）。HT160S 亦無 `mtWorkArea`/`mtSortRecv`（HT172 元件）——生產盤面＝`mtLoaderL/R`（鏡射），移動盤面＝`mtLoaderL/RTrayWork`（`BindMovingTrayPanel`）。左右對應：`LoaderNo==1`→Left（`grpLoaderL`）、`LoaderNo==2`→Right（`grpLoaderR`）。

### 14.1.3　動作時序

#### 單側自動進料循環（DoLoader，每側獨立）

狀態旗標序列：`LS_IDLE → LS_FEEDING → LS_CCD_SCAN → LS_READY_SORT →（SortArm 接管）LS_SORTING → LS_ToRear →` 排出後回 `LS_IDLE`。

1. **Task 100**：若 Loader-Y 上無盤（`fHasTray==false`）且對側不在 FEEDING/CCD_SCAN 且未被 AMR 鎖定 → 進 `LS_FEEDING` 呼叫 `DoFeedTray`（Task=1000）；若已有盤 → 進 `LS_CCD_SCAN` 呼叫 `DoCcdCheck`（Task=2000）。
2. **Task 1000**：`DoFeedTray` 完成後，若對側非 CCD_SCAN → 進 `LS_CCD_SCAN`（Task=2000）。
3. **Task 2000**：`DoCcdCheck` 逐格掃描完成 → Task=3000。
4. **Task 3000**：若 Y 軸未被 SortArm 佔用：盤面仍有非 EMPTY 的 IC → 標 `LS_READY_SORT` 等 SortArm 取；若全 EMPTY 且後方/對側可排出 → 進 `LS_ToRear` 呼叫 `DoDischargeTray`（Task=4000）。
5. **Task 4000**：`DoDischargeTray` 完成（空盤排到後方）→ Status=`LS_IDLE`，Task=1 重新循環。

#### 取盤上料（DoFeedTray）

1. `AcquireFrontOwner` 取得共用前置疊盤機台權（兩側互斥）。
2. `MoveLoaderY` 移到該側 `FeedTrayY` 取盤位。
3. `PushTray`、`LeanOnTray` 氣缸 Pop。
4. `DoFrontDestackDown` 分出一盤。
5. `LeanOnTray` Push、`PushTray` Push 夾緊盤子。
6. 確認有盤（模擬看 `chkLoadTray`；實機看 `PushTray` OnSensor）→ `fHasTray=true`、`PrepareTrayMap` 建立盤面（全格設 `UNCHECK_IC`）；無盤則跳 `Loader Tray Empty` 警報。
7. `ReleaseFrontOwner` 釋放疊盤機權，返回完成。

#### 前置疊盤分一盤（DoFrontDestackDown，生產與 Teach 測試共用）

1. `C_Loader_FrontRiseTray_1` On。
2. 等 RiseTray_1 到位 → `C_Loader_FrontRiseTray_2` On。
3. 等 RiseTray_2 到位 → `C_Loader_FrontSeparateTray_1` On（分張），起延時。
4. 延時到 → RiseTray_2 Off（放下最底一片）。
5. 等 RiseTray_1 到位 → `FrontSeparateTray_1` Off。
6. RiseTray_1 Pop 後 Off，返回完成。

#### 頂部 CCD 掃描與索引（DoCcdCheck）

1. 盤面仍有 `UNCHECK_IC` → 繼續掃；全掃完且對側未在等待則標 `LS_READY_SORT` 返回。
2. `FindNextCcdCell`：以 Y 列為外圈、X 為內圈**蛇行**（`bCcdLeftToRight` 每列端點翻轉）找下一個 UNCHECK 格。
3. `MoveToCcdCell`：把 Top CCD X 與 Loader-Y 移到該格（`FirstCCDX/Y` + `XPitch/YPitch`），設 100ms 安定延時。
4. `ReadTopCcdBin` 讀該格有無料（模擬/未啟用 CCD 回 `HAS_OK_IC`）；寫入盤面。若啟用 2D Bin Map 且為 `HAS_OK_IC`，觸發 Top CCD 拍照，設 3000ms 等待。
5. `ReadTopCcd2DCode` 取 2D 條碼，`LotRegistry.FindByCode2D` 反查 Lot 與 Bin，寫入該格 Bin/Lot/2D 碼；By Lot+Bin 模式則 `LotBinBinding.ResolveAuto` 綁定 Auto；查不到 → 警報。

> 註（定案）：`ReadTopCcdBin` 實機路徑（UseCCD 開、非模擬）確為 stub（`bOk=false` 回 `EMPTY_IC`，呼叫端跳 `WAR0330 Top CCD API not ready`，可 SKIP/RETRY/TRAY_END）。實機的 IC 有無/Bin 判定**不靠此函式**：真正來源是 `ReadTopCcd2DCode`（僅 REALLY＋`bUseTopCcd` 才實連 TopCcdSocket）取得 2D 碼後，由 `LotRegistry.FindByCode2D` 反查匯入的 2D/Bin 資料——即實機恆走 2D 路徑。

#### 空盤排出（DoDischargeTray）

1. 比較兩側 Loader-Y 編碼器位置，避免越過對側，且後排底部未佔用才繼續。
2. `IsRearOccupied` 後方有盤則等待。
3. `MoveLoaderY` 到該側 `DischargeTrayY`；實機若 `OutputBottom` 感測器仍顯示有 IC 則跳「請移除」警報；`PushTray`/`LeanOnTray` Reset。
4. `PushTray` Pop → `bRearHasTray=true`。
5. `LeanOnTray` Pop → `ClearTray` 清盤面。
6. `MoveLoaderY` 回 `FeedTrayY`，返回完成。

### 14.1.4　互鎖與安全

> ⚠️ 注意：**跨側 Loader-Y 安全距離互鎖**（`IsLoaderYMoveSafe`/`MoveLoaderY`）。兩側共用同一導軌，當對側夾著盤（`fHasTray`）且本側目標位置與對側編碼器位置間隔 `< GeneralSetting.iLoaderYSafeDistance`（預設 10000＝100mm）時，拒絕移動、回 false 等待。`MoveLoaderY` 是唯一的移動入口。`LoaderYSafeDistance<=0` 視為停用此互鎖。

> ⚠️ 注意：**SortArm Y 軸所有權互鎖**（`iYOwner=NONE/SORTARM`）。Loader 與 SortArm 不可同時動 Loader-Y。Task 3000 若 Y 被 SortArm 佔用則等待，由 `AcquireSortOwner`/`IsSortOwnerHeld`/`ReleaseSortOwner` 三段交握管理。

其他互鎖：

- **軟體極限**：`MoveLoaderY`/`MoveTopCcdX` 移動前 `CheckSoftLimit`，超界跳警報並拒動。
- **共用前置疊盤機互斥**：`AcquireFrontOwner`/`iFrontOwner`，同時只允許一側操作疊盤機。
- **對側忙碌互鎖**：一側 FEEDING/CCD_SCAN 時，對側 `DoFeedTray` 返回 false、不開始取盤。
- **排出前互鎖**：`DoDischargeTray` 須對側位置、後排底部（`IsOutputBottomOccupied`）與後方（`IsRearOccupied`）皆未佔用才排。
- **實機排出前殘料檢查**：`OutputBottom` 感測器顯示仍有 IC 時跳 `Loader Tray has IC, please remove`。
- **AMR 鎖**（`bAmrLocked`）：交握補料期間凍結前置疊盤/取盤。

### 14.1.5　AMR/AGV 補料交握

1. `IsInputShortageForAmr`：模擬看 `iSimInfeedCount<=0`；實機看 `SnLoader_Inputend` Off（空）→ 呼叫 AGV。
2. `SetAmrLock(true)`：交握期間凍結前置疊盤/上料（`DoLoader` case 100 略過）。
3. `IsReadyForAmrHandoff`：三顆前置疊盤氣缸輸出皆 Off（回原位）才可讓 AGV 補料。
4. `IsInputHandoffFinishedForAmr`：補料完成（實機 `SnLoader_Inputend` On）。
5. `RefillSimInfeed`：模擬補滿 `iSimInfeedCount = iSimAmrMaxTray[0]`。

### 14.1.6　設定

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `GeneralSetting.iLoaderYSafeDistance` | 預設 10000（=100mm）；`<=0` 停用 | 兩側 Loader-Y 車跨側最小安全間隔（1/100mm），`General.ini [Safety] LoaderYSafeDistance` |
| `GeneralSetting.iSimAmrMaxTray[0]` | 模擬補料用；index 0=Loader | 模擬 Loader 來料疊盤盤數（每次取盤遞減，0 缺料呼叫 AGV） |
| `GeneralSetting.iSortMode` | 0/1/2 | 分流模式；1=By Lot+Bin（2D 反查後 `ResolveAuto` 綁定 (Lot,Bin)→Auto）、2=By Lot+PassFail（綁定 (Lot,PASS/FAIL)→Auto，PASS/FAIL 由 Bin==Pass Bin 導出、掃描時凍結） |
| `CosFunction.bUse2DBinMap` | bool | 是否啟用 Top CCD 2D 條碼讀取與 Bin2DMap 反查（關閉只讀有無料） |
| `Teach.Loader1/2CarFeedTrayYPosition` | 教導值 | 各側 Loader-Y 取盤位（1/100mm） |
| `Teach.Loader1/2CarDischargeTrayYPosition` | 教導值 | 各側空盤排出位 |
| `Teach.Loader1/2CarFirstCCDYPosition` | 教導值 | 各側 CCD 掃描第一格 Y 起點 |
| `Teach.LoaderCarFirstCCDXPosition` | 教導值 | Top CCD X 掃描第一格起點（兩側共用） |
| `TrayForm.XDivision / YDivision` | X clamp 1..50；Y clamp 1..20 | 盤面格數（由 recipe 讀入） |
| `TrayForm.XPitch / YPitch` | recipe 值 (double) | 盤面格距，計算各格 CCD 位置用 |
| `HSys.LastSet.iRealDummy` | DUMMY/HAS_TRAY/REALLY | 運行層級，影響感測器/CCD/拍照是否走實機路徑 |

### 14.1.7　警報與排除

| 警報 | 意義 | 排除 |
| --- | --- | --- |
| `Loader Tray Empty` | 取盤時氣缸感測器/模擬判定無盤（來料疊盤已空） | K_RETRY 重取／K_TRAY_END 標該側盤盡／K_CLEAN_OUT 進排空模式 |
| `Loader Y motor will out of limit` | Loader-Y 目標超軟體極限 | 檢查教導位置/極限設定後重試 |
| `Top CCD X motor will out of limit` | Top CCD X 目標超軟體極限 | 檢查教導位置/極限設定 |
| `Top CCD Connect not ready` | REALLY 且啟用 2D Bin Map 時 Top CCD 連線未就緒 | K_RETRY 重試／K_SKIP 標 `HT160_BIN_ERROR_2D_SCAN_FAIL` |
| `Top CCD API not ready` | 讀該格有無料的 CCD Bin 結果失敗 | K_RETRY 重讀／K_SKIP 設 EMPTY_IC／K_TRAY_END 整盤 UNCHECK 改 EMPTY |
| `2D code not found in any lot : <code>` | 2D 條碼在所有 Lot 中反查不到 Bin | K_RETRY 重拍／K_SKIP 標 `NO_BIN_SETTING`、Lot=-1（導向 Error Auto） |
| `Top CCD 2D no response` | 3000ms 內 Top CCD 未回 2D 條碼 | K_RETRY 重拍／K_SKIP 標 `NO_BIN_SETTING` |
| `Loader Tray has IC,please remove` | 實機排出前底部感測器顯示盤上仍有 IC | K_RETRY 移除後重試／K_SKIP 略過排出 |

---

## 14.2　Color 顏色模組（TColorModule）

### 14.2.1　功能

顏色模組負責供應**身份盤（identity tray）**給後段 `TrayArm`。兩種模式：`TraySupply`（供盤，預設）與 `SortBin`（分 Bin，目前為空殼）。供盤時先在前方收/讀位置（`ColorRead2DYPosition`）由破棧氣缸分離一片盤並夾住，透過 **Color 2D CCD** 讀取盤上 2D `TrayID`，再用 `ColorY` 軸把載盤搬到後方取盤位置（`ColorTrayArmPickYPosition`）讓 `TrayArm` 夾取。實際供盤動作只有在收到 AMR/TrayArm 真正需求（`bSupplyRequested`）時才推到輸出位置並讀碼，避免提前呈現身份盤。

> Color 模組無自身 UI 畫面，由 `database.cpp ProcessMotion` 透過 `ColorModule->DoColor()` 驅動。

> ⚠️ 注意：**安裝閘**。`DoColor` 開頭若 `IsInstalled()==false`（`GeneralSetting.bColorBinAreaInstalled` 為 false）即直接返回，整個模組停用。

### 14.2.2　動作時序

ColorY 軸幾何：Y=前/後（相對操作者）、X=左/右、Z=上/下。供盤為**兩段式管線**（仿 Empty）：Stage1 先把一片盤分離到前方緩衝；Stage2 只有在有需求時才推到輸出讀碼。

#### TraySupply 供盤主流程（DoColor）

1. `RefreshStateFromSensors`；若為 SortBin 模式改走 `DoSortBin`。
2. case 100 決策：
   - 若 `bAmrLocked` → 暫停（凍結前段破棧）。
   - 若 `bTrayReady && bTrayPicked` → `DoReleaseTray` 釋放空位。
   - 若僅 `bTrayReady`（尚未被夾走）→ 回 idle 等 TrayArm 取走。
   - 若 `bFrontHasTray==false` 且（`bInputHasTray` 或模擬）→ `DoGoDownTray` 預先分離一片盤到前方緩衝。
   - 若前方已有暫存盤且 `bSupplyRequested==true` → `DoSupplyTray` 推到輸出並讀碼；否則回 idle。

#### 前方破棧分盤（DoGoDownTray）

1. 依序 Push `C_Color_FrontRiseTray_1`、`C_Color_FrontRiseTray_2`（升盤）。
2. Push `C_Color_FrontSeparateTray_1`（分盤），起 5 tick 延時。
3. 延時到 → Pop `FrontRiseTray_2`，再延時。
4. Pop `FrontSeparateTray_1`，再延時。
5. Pop `FrontRiseTray_1`，邏輯上設 `bFrontHasTray=true`（無前段感測器，僅邏輯暫存）並結束。

#### 供盤 + 讀碼 + 搬到後方（DoSupplyTray）

1. `MoveColorY(ColorRead2DYPosition)` 把載台移到前方收/讀位置。
2. `RefreshStateFromSensors`；若輸出已有盤直接走讀碼；若無前方暫存盤且非模擬，清 `bSupplyRequested` 並結束。
3. `DoClampTray` 標準雙缸夾盤（`C_Color_LeanOnTray` 靠 + `C_Color_PushTray` 推，SettleTicks=0 不做 push-on-sensor 確認）。
4. `RefreshStateFromSensors`；若 `SnColor_OutputBottomHasTray` 有效、輸出無盤且非 DUMMY → `Color supply tray is not ready` 報警；否則 `bFrontHasTray=false` 並啟動 `DoReadColor2D`。
5. 等待 `DoReadColor2D` 完成（填 `sTrayID2D`）。
6. `MoveColorY(ColorTrayArmPickYPosition)` 把盤搬到後方取盤位置；成功後 `bTrayReady=true`、`bTrayPicked=false`、`bSupplyRequested=false` 並結束。

#### Color 2D CCD 讀碼（DoReadColor2D）

1. 若 `SOFT_SIMULATE`／模擬 → 偽造 `sTrayID2D="COLOR2D_"+時間戳`；若 `CosFunction.bUseColorCcd==false` → 空碼結束；否則建立 socket 並連線。
2. `MTopCCDX_Color` 移到 `ColorRead2DXPosition`（含 `CheckSoftLimit`）。
3. socket 未連線 → `Color CCD connect not ready`（K_RETRY|K_SKIP）；連線則 `ColorCcdTriggerShot`，起 3000ms 延時。
4. 輪詢 `ColorCcdGetResult` 取碼填 `sTrayID2D` 後 `ColorCcdEndShot`；逾時則 `Color CCD 2D no response`（K_RETRY|K_SKIP）。

#### 釋放/清盤（DoReleaseTray）

依序 Pop `C_Color_PushTray`、`C_Color_LeanOnTray`、`C_Color_RearRiseTray`、`C_Color_FrontRiseTray_2`、`C_Color_FrontRiseTray_1`、`C_Color_FrontSeparateTray_1`，完成後清 `bTrayReady`/`bTrayPicked`/`bOutputHasTray` 並結束。

### 14.2.3　跨模組交握

**TrayArm 取盤交握**（由 `aTrayArm.cpp` 呼叫）：

1. TrayArm 需要盤時：若 `ColorModule->IsTrayReady()==false`，呼叫 `RequestSupplyTray()` 設 `bSupplyRequested=true` 觸發供盤。
2. 盤就緒被夾走時：`iDeliverTrayID=GetTrayID()` 取得 2D TrayID，並 `NotifyTrayPicked()` 設 `bTrayPicked=true`。

**AMR（P3 ColorTray）補料交握**（由 `uAgvStation.cpp` 呼叫）：

1. `IsInputShortageForAmr()`：模擬看 `iSimInfeedCount<=0`；實機看 `SnColor_InputEnd.IsOff()`。
2. `IsReadyForAmrHandoff()`：前段三顆升/分盤缸輸出位皆 false（在家）。
3. `SetAmrLock(true)` 凍結前段破棧。
4. `IsInputHandoffFinishedForAmr()`：模擬自動完成；實機等 `SnColor_InputEnd.IsOn()`。
5. `RefillSimInfeed()`：把 `iSimInfeedCount` 重設為 `GeneralSetting.iSimAmrMaxTray[2]`（僅模擬）。

### 14.2.4　互鎖與安全

> ⚠️ 注意：**AMR 鎖**。`DoColor` case 100 若 `bAmrLocked` 直接 break，補料期間凍結前方破棧，避免與 AMR 補料動作衝突。

> ⚠️ 注意：**供盤需求閘（just-in-time）**。`bTrayReady` 只由 `DoSupplyTray` 設、由 `DoReleaseTray` 清，且 `DoSupplyTray` 只在 `bSupplyRequested` 為真時觸發。確保身份盤只在 AMR/TrayArm 真正有需求時才呈現/掃描，不提前供盤。

其他互鎖：

- **ColorY 軟極限**：`MoveColorY` 先 `CheckSoftLimit`，超限 `ShowMyMessage("Color Y motor will out of limit")` 並回 true（不真的移動，但不卡住供盤梯）。
- **Color CCD X 軟極限**：`DoReadColor2D` `MTopCCDX_Color->CheckSoftLimit` 超限時跳過移動。
- **輸出盤確認**：`DoSupplyTray` 若 `SnColor_OutputBottomHasTray` 啟用、輸出無盤且非 DUMMY → `Color supply tray is not ready`（K_RETRY；DUMMY 不報，仿三層 RealDummy 檢查）。
- **CCD 連線/讀碼失敗**：以 K_RETRY|K_SKIP 處理，跳過時 2D 碼留空、供盤流程繼續。

### 14.2.5　設定

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `GeneralSetting.bColorBinAreaInstalled` | bool | Color 區是否安裝；false 時整個模組停用 |
| `iMode`（`eHT160ColorMode`） | 預設 TraySupply(1) | 模組模式：TraySupply 供盤 / SortBin(0) 分Bin |
| `Teach.ColorRead2DYPosition` | 教導值（1/100mm） | ColorY 前方收/讀 2D 位置 |
| `Teach.ColorTrayArmPickYPosition` | 教導值 | ColorY 後方 TrayArm 取盤位置 |
| `Teach.ColorRead2DXPosition` | 教導值 | Color 2D CCD 讀碼時 `MTopCCDX_Color` 的 X 位置 |
| `CosFunction.bUseColorCcd` | bool（`[ColorCCD] Enable`） | false 時跳過相機、2D 碼留空 |
| `GeneralSetting.iSimAmrMaxTray[2]` | int（index 2=Color） | 模擬 Color(P3) 進料堆最大盤數 |
| `ScanDelay 逾時` | 3000 ms | Color 2D CCD 一次拍攝/讀碼回應逾時 |
| `破棧/夾盤延時` | 各段 5 tick | 分盤動作各段安定延時 |

> 註（定案）：`iSupplyThreshold`（預設 100）與 `NotifyICPlaced`/`iICCount` 為**遺留碼**——全程式無任何呼叫點（`iICCount` 僅出現在 FeederDecision 除錯 dump），不影響供盤行為。

### 14.2.6　警報與排除

| 警報 | 意義 | 排除 |
| --- | --- | --- |
| `Color supply tray is not ready` | 推盤後輸出底部感測器未偵測到盤（非 DUMMY） | K_RETRY 重試（重跑供盤） |
| `Color CCD connect not ready` | Color 2D CCD socket 未連線/未就緒 | K_RETRY 重連／K_SKIP 跳過讀碼（2D 碼留空） |
| `Color CCD 2D no response` | 觸發拍攝後 3000ms 未取得結果 | K_RETRY 重拍／K_SKIP 跳過 |
| `Color Y motor will out of limit` | `MoveColorY` 目標超軟極限 | 檢查/修正 ColorRead2DYPosition 或 ColorTrayArmPickYPosition |
| `Color CCD X motor will out of limit` | `MTopCCDX_Color` 目標超軟極限 | 檢查/修正 ColorRead2DXPosition |

> 註（定案）：`DoSortBin`（Color 當 Bin 區收 IC）確為空殼（switch 僅 case 1 直接 return true）、`IsAcceptingIC()` 固定回 false——此功能**未實作，屬預留介面**；Color 站目前僅作身分/reject 盤供給，不收分選 IC。

---

## 14.3　Empty 空盤模組（TEmptyModule）

### 14.3.1　功能

空盤模組負責供應與處理空料盤。從前段堆疊（input stack）以雙缸頂升/分張機構逐片取下空盤（GoDown），再以斜靠+推送雙缸夾住、由 `MEmptyY` 軸搬移至放料位送入後段（Feed），供 `TrayArm` 取走。批次結束或需回收時可反向把後段料盤重新夾起、上推回前段堆疊（GoUp/Return）。

> **盤態鎖存（latch）約定**：盤子下來（GoDown 完成）＝有盤（`bFrontHasTray=true`）；`MotorY`/TrayArm 夾走＝沒盤（`SetRearHasTray(false)`）。模擬/DUMMY 下 `RefreshStateFromSensors` 直接 return，狀態完全由動作序列鎖存維護。本模組無自身 UI 畫面。

### 14.3.2　動作時序

#### DoEmpty 主排程（case 100 閒置決策優先序）

1. 若 `bAmrLocked` → 本回合不啟動任何新動作（凍結前段拆堆）。
2. `RefreshStateFromSensors` 後依優先序：
   - **Return**：若 `bReturnTray` → `DoGoUpTray`（回收/上推）。
   - **前段補盤**：若 `bFrontHasTray==false` 且 `bLotFinish==false` → `DoGoDownTray`（從堆疊取下一片）。
   - **送後段**：若 `bRearHasTray==false` 且 `bLotFinish==false` → `DoFeedTray`（前段料盤送後段）。
   - **批次結束回收**：若 `bLotFinish` 且 `bFrontHasTray` → `DoGoUpTray`（上推回收）。

#### 取下空盤（DoGoDownTray，前段拆堆）

1. `C_Empty_FrontRiseTray_1` On（頂升缸1 上）。
2. RiseTray_1 到位 → `C_Empty_FrontRiseTray_2` On（頂升缸2 上）。
3. RiseTray_2 到位 → `C_Empty_FrontSeparateTray_1` On（分張缸 出），起 5 延時。
4. 延時到 → `FrontRiseTray_2` Off（放下最底一片），再延時。
5. RiseTray_1 仍到位 → `FrontSeparateTray_1` Off（分張缸 收），再延時。
6. `FrontRiseTray_1` Pop（頂升缸1 復位）。
7. 檢查 `SnEmpty_InputHasTray`：Enable 且 IsOff 且非 DUMMY → `bFrontHasTray=false` 並 `Front Empty Tray Is Miss Error`；否則 `bFrontHasTray=true`、返回完成。

#### 送盤到後段（DoFeedTray，雙缸夾取+Y軸搬移）

1. 一次性 `C_Empty_LeanOnTray.Reset()`、`C_Empty_PushTray.Reset()` 清除中止留下的 Push() Task。
2. `RefreshStateFromSensors`；若 `bRearHasTray` 已有盤則 return（不需送）。
3. `MoveEmptyY(EmptyCarFeedTrayYPosition)` 移到上料位。
4. 共用 `DoClampTray`（SettleTicks=5）做雙缸夾盤：回 1（夾好）→ 繼續；回 2（推缸 Miss）→ `Empty Push Tray Miss`（K_RETRY）。
5. `MoveEmptyY(EmptyCarDischargeTrayYPosition)` 移到放料位，設 `bFrontHasTray=false`。
6. `C_Empty_PushTray.Pop()` → `C_Empty_LeanOnTray.Pop()`。
7. 檢查底部感測 `SnEmpty_OutputBottomHasTray`，設 `bRearHasTray`，返回完成。

> ⚠️ 注意：`DoFeedTray` 開頭的 `Reset()` 只能做一次。註解明確警告：**勿在每次 `Push()` 前 `Reset()`**，否則會重啟非阻塞狀態機而卡住。

#### 上推/回收料盤（DoGoUpTray，Return / 批次結束）

1. `FrontRiseTray_1` On → 到位後 `FrontSeparateTray_1` On（延時）。
2. `FrontRiseTray_2` On → 到位後 `FrontSeparateTray_1` Off（延時）。
3. `FrontRiseTray_2` Off → RiseTray_1 到位 → `FrontRiseTray_1` Pop，設 `bFrontHasTray=false`。
4. 若 `bRearHasTray`：`MoveEmptyY(Discharge)` → `LeanOnTray` Push → `PushTray` Push → `MoveEmptyY(Feed)` → `PushTray` Pop → `LeanOnTray` Pop，設 `bFrontHasTray=true`、清 rear/bottom 旗標，返回完成。

#### DoClampTray 雙缸夾盤共用子程序（mycylin.cpp）

回傳 0=進行中、1=夾好、2=推缸 Miss。動作固定為**斜靠缸先、推缸後**：

1. SubTask 0（lean-stop first）：`Lean.Push()` 成功 → 斜靠缸先到位。
2. SubTask 10（push last）：`Push.Push()` 成功；`SettleTicks<=0` 立即回 1，否則啟動 settle 延時。
3. SubTask 20（settle+confirm）：settle 後 `IsCylinderOnReady(Push)` 為真 → 回 1；否則退缸。
4. SubTask 30（miss-retract）：`Push.Pop()` → 回 2，由呼叫端 alarm/retry。Empty 模組以 SettleTicks=5 呼叫。

### 14.3.3　互鎖與安全

> ⚠️ 注意：**MoveEmptyY 防撞 TrayArm**（`#ifndef SOFT_SIMULATE`）。若 TrayArm Z 上升缸 `C_TrayArmZ_Up` 未到位（`IsOn()==false`）且 TrayArmX 編碼器位置（`TrayArmPos+500`）`>= Teach.TrayXArmToEmptyXPosition`，則 EmptyY 拒絕移動（return false），避免與 TrayArm 干涉。

其他互鎖：

- **MoveEmptyY 軟體極限**：`CheckSoftLimit==false` 時 `Empty Y motor will out of limit` 並拒動；`MEmptyY==NULL` 時直接 return false。
- **料盤遺失檢查加 DUMMY 條件**：`DoFeedTray`/`DoGoDownTray` 的感測檢查皆加 `iRealDummy!=DUMMY`（DUMMY 不讀實體感測，盤態由 latch 維護）。
- **RefreshStateFromSensors 在模擬/DUMMY 直接 return**：因 HasTray 輸入為 `InType=0` active-low，無 MN200 卡時會誤判成「有盤」而清掉 latch 卡住模組。
- **AMR 鎖**：`bAmrLocked` 為真時 `DoEmpty` case 100 直接 break，凍結前段拆堆。
- **DoClampTray 推缸到位確認**：settle 後若推缸 OnSensor 未到位則退缸並回 Miss，避免夾不到盤就搬移。

### 14.3.4　AMR 補料交接（P2）

1. `IsReadyForAmrHandoff`：前段三缸（RiseTray_1/_2 + SeparateTray_1）輸出皆 false（回原位）。
2. `SetAmrLock(true)` 期間 `DoEmpty` 不啟動新拆堆。
3. `IsInputShortageForAmr`：模擬 `iSimInfeedCount<=0`／實機 `SnEmpty_InputEnd` OFF → 觸發呼叫 AMR。
4. `IsInputHandoffFinishedForAmr`：模擬 always true／實機 `SnEmpty_InputEnd` ON。
5. `RefillSimInfeed` 把模擬輸入堆重置為 `GeneralSetting.iSimAmrMaxTray[1]`。

### 14.3.5　設定

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `Teach.EmptyCarFeedTrayYPosition` | 教導值（1/100mm） | EmptyY 軸「上料位置」（取盤/夾盤位） |
| `Teach.EmptyCarDischargeTrayYPosition` | 教導值 | EmptyY 軸「放料位置」（送料盤入後段位） |
| `Teach.TrayXArmToEmptyXPosition` | 教導值 | TrayArmX 進入 Empty 干涉區的 X 門檻，用於 MoveEmptyY 防撞判斷 |
| `GeneralSetting.iSimAmrMaxTray[1]` | 設定值（index 1=Empty） | 模擬 Empty 輸入堆滿匣盤數（每次 GoDown 消耗 1） |
| `DoClampTray SettleTicks` | Empty 用 5 | 雙缸夾盤後安定時間（HTimer ticks），>0 才做 settle+confirm |
| `HTimer Delay` | 5 ticks（寫死於程式） | 頂升/分張各步驟延時 |

### 14.3.6　警報與排除

| 警報 | 意義 | 排除 |
| --- | --- | --- |
| `Empty Push Tray Miss` | 送盤夾取時推缸未到位（DoClampTray 回 2） | K_RETRY：回 case 1000 重新移到上料位再夾 |
| `Bottom Empty Tray Is Miss Error` | 送盤完成後底部感測器讀不到料盤（非 DUMMY） | K_SKIP|K_RETRY：RETRY 重來；SKIP 清旗標跳過 |
| `Rear Empty Tray Is Miss Error` | SKIP 底盤後後段仍讀不到料盤（非 DUMMY） | K_RETRY：回 case 1 重來 |
| `Front Empty Tray Is Miss Error` | 拆堆完成後前段感測器讀不到料盤（非 DUMMY） | K_RETRY：回 case 1 重新拆堆 |
| `Empty Y motor will out of limit` | `MoveEmptyY` 目標超軟體極限 | 資訊提示，拒絕該次移動 |

> 註（定案）：`bLotFinish` 為 Empty 模組私有旗標，僅兩個寫入點：InitialTask 清 false；主 ladder case 100 每輪重算 `bLotFinish = (RunMode==Run_CleanOut && TrayArmModule->IsCleanOutFinish())`——即只在清機且 TrayArm 收尾完成時為 true（供 drain 分支判斷），與 Lot Start/End 流程無關。

---

## 14.4　SortArm 分類臂與 TrayArm 盤臂模組

### 14.4.1　功能

分類臂（`SortArmModule`）與盤臂（`TrayArmModule`）是兩個無自身畫面的機台控制模組，由 `csystem` 主流程以 `switch(Task)` 程序式階梯逐拍（非阻塞）驅動。

- **分類臂**：用一排吸嘴（4 個 Suck-Z）從 Loader 來料盤把 IC 逐顆吸取，依 Bin/Lot 路由放到 Auto1~6 出料盤。採**單顆取放（SINGLE-PICK）**：每一行程只取/放一顆，run-loop 每圈重呼 `FindPickCells` 逐顆把整盤排空。
- **盤臂**：單軸搬運臂，從 EmptyTray/Loader 後方/Color 取空盤，搬送並放到目標 Auto 後方（或回收到 EmptyTray）。

> ⚠️ 注意：兩臂都內建防撞互鎖。分類臂在 X 移動前須**全部吸嘴回原點**；盤臂在 X 移動前須 **Z 升降氣缸確認在上位**。這些互鎖只在編譯期 `SOFT_SIMULATE` 模擬版被略過，實機 `DUMMY/HAS_TRAY/REALLY` 三種模式皆持續生效（因 DUMMY 下馬達與氣缸仍實際運動，僅略過正確性感測器確認）。

### 14.4.2　SortArm 動作時序

#### 取放主迴圈（DoSortArm Task）

1. **Task 1（待命/派工）**：若手上已持有 IC → 去放料（PlaceTask=1, Task=200）；否則向 `LoaderModule` 詢問 `GetSortingLoaderNo` 取得待分類 Loader 編號（>0）→ 去取料（PickTask=1, Task=100）；`Run_CleanOut` 且兩側 Loader 皆排空 → 設 `bCleanOutFinish`。
2. **Task 100（取料）**：`DoPickFromLoader(1)` 完成後去放料。
3. **Task 200（放料）**：`DoPlaceToAuto(1)`；仍持料留在 200 繼續放，否則回 Task 1。

#### 取料子流程（DoPickFromLoader）

1. `FindPickCells` 掃整盤找第一顆可取格，選出能對到該欄的吸嘴；`AcquireSortOwner` 取得 Loader-Y 軸獨佔權，失敗則清選擇返回。
2. `SortArmZToSafePos` 全吸嘴升到安全 Z（`SORT_ARM_SAFE_Z_POSITION=10`）。
3. `MovePitchToTrayPitch` 把吸嘴間距軸（`MPitchX`）調到盤距。
4. `MoveToLoaderPick` 計算並移動 X/Y。
5. 再驗 `IsSortOwnerHeld`，失去則退出；否則 `MovePickZDown` 把選定吸嘴下降到 Loader 取料 Z。
6. `SuckSelectedSlots` 抽真空吸取，成功後把資料轉到吸嘴、清來料盤格、標 `bHasIC`。
7. `SortArmZToSafePos` 升回安全 Z → `ReleaseSortOwner` 釋放軸 → 完成。
8. （握手中斷退出）：升安全 Z、釋放軸、清取料選擇後返回。

#### 放料子流程（DoPlaceToAuto）

1. `SelectPlaceAuto`：依每顆吸嘴路由 Bin（`GetSlotRoutingBin`）經 `GetMappedAutoIndex` 對到 Auto，`FindPlaceCells` 找該 Auto 盤第一個空格並組出連續吸嘴 run。
2. `SortArmZToSafePos` 升安全 Z → `MovePitchToTrayPitch` 調間距 → `MoveToAutoPlace` 移到放料 X/Y。
3. `MovePlaceZDown` 選定吸嘴下降到 Auto 放料 Z。
4. `DestroySelectedSlots` 破真空放料，成功後寫入 Auto 盤格、累加 `TrayICCnt`、清吸嘴 slot。
5. `SortArmZToSafePos` 升回安全 Z → 完成。

### 14.4.3　TrayArm 動作時序

#### 搬運主迴圈（DoTrayArm Task）

`DecideJob` 派工優先序：1) Loader 後方滯留空盤回收（`TAJOB_LOADER_RECOVERY`）；2) AMR 模式依 Auto 需求堆疊 identity/cover/normal（`TAJOB_AMR_SUPPLY`）；3) Normal 模式 EmptyTray→Auto 補空盤（`TAJOB_EMPTYTRAY_TO_AUTO`）；皆為需求驅動。

1. **Task 100（待命/派工）**：若意外仍持盤且 `Job!=NONE`（中斷後可續）→ 續放；否則 `DecideJob` 派工，有工作則 `DoPick`（Task=1000）。
2. **Task 1000（取盤）**：`DoPick(1)` 完成後設 CARRYING；Loader-recovery 工作呼 `DecidePlaceDestAfterPick` 決定供 Auto 或回收 EmptyTray，其它為 AUTO；`DoPlace`（Task=2000）。
3. **Task 2000（放盤）**：`DoPlace(1)` 完成後回 IDLE、Job=NONE、Task=100。

#### 取盤子流程（DoPick）

1. `DoZUp` 升 Z（必須 up 感應器確認）。
2. `MoveTrayArmX` 移到取料 X（Loader-recovery=Loader 後方、AMR identity=Color、其餘=EmptyTray 後方）。
3. `DoZDown` 下降。
4. `C_TrayArm_FrontClamp` + `C_TrayArm_RearClamp` 兩夾爪 Push 夾住盤，啟動 `ArmDelay` 3 拍。
5. 等 `ArmDelay.Off` → `DoZUp` 升 Z。
6. 依工作別通知來源（Loader `NotifyTrayArmPickRearTray` / Color 取 TrayID 後 `NotifyTrayPicked` / EmptyTray `SetRearHasTray false`），標 `bHasTray=true`。

#### 放盤子流程（DoPlace / DoPlaceToEmpty）

1. `DoPlace` 開頭若 `PlaceDest==TAPLACE_EMPTY` 轉呼 `DoPlaceToEmpty`。
2. `DoZUp` 升 Z → `MoveTrayArmX` 移到目標 Auto X（回收路徑移到 EmptyTray X 並等後方清空才下放）。
3. `DoZDown` 下降 → 兩夾爪 Pop 放開（`ArmDelay` 3 拍）→ `DoZUp` 升 Z。
4. 通知目的端（AMR `NotifyTrayArmDelivered` 帶 kind+TrayID / Normal `SetRearHasTrayFromTrayArm` / 回收 `NotifyTrayXToEmptyFinish`），標 `bHasTray=false`。

### 14.4.4　互鎖與安全（防撞核心）

> ⚠️ 注意：**SortArm 全吸嘴回原點互鎖**（`AreAllSuckersHome()`）。`MoveSortArmX` 每次呼叫都檢查每個啟用的 Suck-Z 馬達當下 `Led[iHomeLed]==true`（**即時 Home 感應器，非黏滯 bHomeFlag**）；任一不在原點即不准 X 移動，避免吸嘴下伸時橫移撞料/撞框。

> ⚠️ 注意：**SortArm 失步去抖 + 急停**。確認非原點時先 `MSortingArmX->Stop()` 每拍夾停；首次掉失記 `dwSuckHomeLostStart`，超過 `SUCK_HOME_LOST_MS=100ms` 才判定真失步 → `HSys.StopAllMotor()` + `ShowSystemError(K_RETRY)`（連帶 SystemStart 下降）。註：`DecStopAllMotor` 在 MC88X1 為 no-op，故用 `StopAllMotor` 真正減速停。

> ⚠️ 注意：**TrayArm Z 上位互鎖**（`IsZUpAtPosition()`）。`MoveTrayArmX` 每次呼叫都檢查 `C_TrayArmZ_Up.IsOn()`；Z 升降未在上位即不准 X 橫移，避免頭/盤在低位時橫掃撞站。失步去抖同 SortArm（`dwZUpLostStart` / `TRAYARM_ZUP_LOST_MS=100ms` → StopAllMotor + ShowSystemError）。

> ⚠️ 注意：**TrayArm DoZUp 真上位確認**。`DoZUp` 須 `C_TrayArmZ_Up.Push()` 成功且 `IsZUpAtPosition()` 為真才回 true（避免 DUMMY 下 Push 立即回 true 就讓 X 在頭未實升時起步）；雙線圈 Z 升前先關 `C_TrayArmZ_Down`。

其他互鎖：

- **SortArm X 軸軟極限**：`MoveSortArmX` 先過 `CheckSoftLimit`，超限拒動。
- **SortArm 單取左緣 BaseX 下限**：`MoveToLoaderPick` 允許 BaseX 為負以取左緣欄，但不得低於 `-(SORT_ARM_SUCKER_COUNT-1)`，超過視為壞資料拒絕。
- **SortArm Loader-Y 雙車安全距**：`MoveLoaderY` 透過 `LoaderModule->IsLoaderYMoveSafe` 檢查共軌對向車安全距；不安全則靜默 return false 由 switch(Task) 重輪詢；DUMMY 下仍生效。
- **SortArm 軸獨佔握手**：取料前 `AcquireSortOwner`，下降吸取前 `IsSortOwnerHeld` 再驗；失去則升安全 Z + `ReleaseSortOwner` + 清選擇安全退出。
- **TrayArm X 軸軟極限**：`MoveTrayArmX` 先過 `CheckSoftLimit`，超限拒動。
- **防撞旁路範圍**：上述兩臂防撞互鎖只以編譯期 `#ifdef SOFT_SIMULATE` 略過；實機三模式皆生效。

### 14.4.5　設定

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `SORT_ARM_SUCKER_COUNT` | 4（常數） | 分類臂吸嘴（Suck-Z）數量 |
| `SORT_ARM_AUTO_COUNT` | 6（常數） | 出料 Auto 站數量 |
| `SORT_ARM_SAFE_Z_POSITION` | 10（常數，1/100mm） | 分類臂 Z 橫移前安全升起位置 |
| `SUCK_HOME_LOST_MS` | 100 ms | 分類臂 X 移動吸嘴脫離 Home 去抖窗 |
| `TRAYARM_ZUP_LOST_MS` | 100 ms | 盤臂 X 移動 Z 脫離上位去抖窗 |
| `GeneralSetting.bSuckerEnabled[s]` | per-sucker bool | 各吸嘴（0..3）是否啟用；影響取放選擇與互鎖檢查範圍 |
| `GeneralSetting.bUseAMR` | bool | AMR 模式開關，決定 TrayArm 派工策略 |
| `GeneralSetting.iSortMode` | 0/1/2 | 分流模式；動態模式（1 By Lot+Bin / 2 By Lot+PassFail）時 `GetMappedAutoIndex` 用綁定查表（Bin 或凍結的 PASS/FAIL 分類鍵） |
| `GeneralSetting.bShowSortArmPlaceCheck` | 預設 OFF | [診斷] 放料前彈出實際 vs 預期位置比對（生產應關） |
| `Teach.SortArmToLoader1/2XPosition` | 教導值 | 分類臂對 Loader1/2 取料 X 基準位 |
| `Teach.SortArmToLoader_1/2_Z1..Z4Position` | 教導值 | 分類臂在 Loader 各吸嘴下降 Z 位 |
| `Teach.SortArmToAuto1..6XPosition` | 教導值 | 分類臂對 Auto1..6 放料 X 基準位 |
| `Teach.SortArmToAuto_1..6_Z1..Z4Position` | 教導值 | 分類臂在各 Auto 各吸嘴下降 Z 位 |
| `Teach.TrayXArmToAuto1..6XPosition` | 教導值 | 盤臂對 Auto1..6 放盤 X 位 |
| `Teach.TrayXArmToColorXPosition` | 教導值 | 盤臂在 Color 站取 identity 盤 X 位（AMR） |
| `Teach.TrayXArmToLoaderXPosition` | 教導值 | 盤臂在 Loader 後方取回收空盤 X 位 |
| `Teach.TrayXArmToEmptyXPosition` | 教導值 | 盤臂在 EmptyTray 後方取/回收空盤 X 位 |

### 14.4.6　警報與排除

| 警報 | 意義 | 排除 |
| --- | --- | --- |
| `SortArm move blocked : a suck nozzle left its Home sensor (lost steps). Re-home the suckers.` | 某吸嘴 Z 離開 Home 並超過去抖窗，判定失步，已 StopAllMotor | K_RETRY；需重新 Home 吸嘴後重試（SystemStart 連帶下降） |
| `TrayArm move blocked : the Z lift left its UP sensor. Check the TrayArmZ up cylinder / air pressure.` | 盤臂 Z 升降氣缸離開上位並超過去抖窗，判定漏氣/下沉 | K_RETRY；檢查 TrayArmZ 上位氣缸與氣壓後重試 |
| `Sorting Arm X motor will out of limit` | 分類臂 X 目標超軟極限 | 修正教導位置/目標（拒動） |
| `Loader Y motor will out of limit` | Loader Y 目標超軟極限 | 修正教導位置 |
| `Auto Y motor will out of limit` | Auto Y 目標超軟極限 | 修正教導位置 |
| `Pitch X motor will out of limit` | 吸嘴間距軸目標超軟極限 | 修正設定 |
| `Tray Arm X motor will out of limit` | 盤臂 X 目標超軟極限 | 修正教導位置 |
| `ShowSuckError (SortArm Pick / Place)` | 吸取（Suck）或放料（Destroy）真空動作失敗 | K_RETRY|K_SKIP；隨後 `Sucker->Reset()` |

> 註：夾爪/升降實體 IO 點位（`system/IO_Table.csv`，格式 Lane/IP/Port/Bit）：`C_TrayArm_FrontClamp`＝0/0/2/6、`C_TrayArm_RearClamp`＝0/0/2/7（兩者 OnDelay/OffDelay 500/500）、`C_TrayArmZ_Up`＝0/0/2/5、`C_TrayArmZ_Down`＝0/0/2/4（各自 On 感測在 Port1 同 Bit）。SortArm 吸嘴為 `Suck1~Suck4`（Sucker，0/0/1/0~3；`SuckN_On/Off` 為 IP=W 的寫出點）。`iHomeLed`=1（`HTMotor.h` LED 索引 enum）。
>
> 【待補（現場）：`ArmDelay.Set(3)` 的 3 拍對應實際毫秒數取決於主迴圈實際週期（標稱約 1ms/拍，即約 3ms），列入現場動態驗證清單量測。】

---

## 14.5　Auto1~6 出料堆疊模組（TAutoModule）

### 14.5.1　功能

`TAutoModule` 管理 6 個出料堆疊站（Auto1~Auto6），純控制模組，無自身 UI 畫面。每站把分料後的盤子在「後方接收位（rear）」、「作業位（working/sort）」與「堆疊車（output car）」之間搬移：`TrayArm` 把空盤（或 AMR 模式的 identity/cover/normal 盤）放到後方，`DoFeedTray` 用 Lean/Push 氣缸把盤夾入並由 `MAutoY` 軸帶到作業位供 `SortArm` 放 IC，盤子滿料（FullIC）後 `DoDischargeTray` 以 FrontRise 氣缸把盤堆入堆疊車。

> **Bin→Auto 的路由判定不在本模組**，而由 `SortArm` 的 `GetMappedAutoIndex` 與 `BinAreaMap`/`LotBinBinding` 決定後，再把盤放入對應 Auto。

### 14.5.2　動作時序

#### 正常出料堆疊循環（DoAuto）

1. **case 1/100**：`CheckAutoTray` 更新各站狀態，AMR 模式呼叫 `ServiceCarFull` 處理滿車。
2. **case 1000/2000**：若 `FindFeedAuto` 找到「車內無盤且後方有盤」站 → `DoFeedTray` 把後方盤夾入並帶到作業位（FirstSortY）。
3. **case 3000/4000**：若 `FindDischargeAuto` 找到 FullIC 站 → `DoDischargeTray` 把滿盤堆入堆疊車。
4. **case 5000**：CleanOut 模式下執行 `DoAllAutoCleanOut` 清空全部站。

#### DoFeedTray 進盤上料（後方盤→作業位）

1. `FindFeedAuto` 選站（`bCarHasTray==false` 且 `bRearHasTray`）。
2. `MoveAutoY` 移到 FeedY；檢查 `OutputBottomHasTray` 確認到位，缺盤跳 `Auto%d Feed Tray Miss`。
3. Lean 氣缸 Push（靠盤）。
4. Push 氣缸 Push 推盤，延時 5 後確認 On；未到位 Pop 並跳 `Auto%d Push Tray Miss`。
5. `MoveAutoY` 移到 FirstSortY（作業位）。
6. 標記 `fHasTray`/`bCarHasTray=true`、清 rear 旗標；AMR 模式記錄 WorkingKind/2D TrayID，identity/cover 盤直接設 `bFullIC=true`。

#### DoDischargeTray 出盤堆疊（作業位滿盤→堆疊車）

1. `FindDischargeAuto` 選 FullIC 站（跳過 AMR-locked 站）。
2. `MoveAutoY` 移到 DischargeY，清車盤旗標、設 `bFrontHasTray=true`，並對 HGem 發 EventReport（CEID 對應 Auto1~6）。
3. Push 氣缸 Pop → Lean 氣缸 Pop。
4. `MoveAutoY` 移到 FeedY，延時 5。
5. `DoFrontRiseOnce` 把 FrontRise 氣缸 On→延時 5→Off，把盤堆入車。

#### DoAllAutoCleanOut 全站清盤（CleanOut 模式）

由 `SortArmModule->IsCleanOutFinish()` 先完成才進入。六站同步：MoveAutoY 到 DischargeY → Push Pop → Lean Pop → FrontRise On → 延時後 FrontRise Off → MoveAutoY 回 FeedY → 清旗標、TrayMotor 清盤、設 `bCleanOutFinish=true`。

### 14.5.3　跨模組與資料流

1. **盤子來源**：TrayArm 放盤到某 Auto 後方 → `NotifyTrayArmDelivered(Index,Kind,TrayID)`，設 `bRearHasTray` + `bRearDeliveredPending` 鎖存 + 記 RearKind/RearTrayID。
2. **需求拉取**：`GetTrayRequest` 回報該站想要的盤種（AMR 依堆疊順序 identity→cover→normal；Normal 模式恆為空工作盤）；`FindTrayRequestAuto` 回報第一個要盤的站給 TrayArm。
3. **Bin→Auto 路由（不在本模組）**：`SortArm.GetMappedAutoIndex` 依模式決定目標 Auto：(a) Lot+Bin 模式查 `LotBinBinding.FindAuto`；(b) BinAreaMap 模式查 `GetAreaByBin` 對應 `eHT160BinAreaAuto1..6`；(c) 皆查無 → ErrorBin 區的 Auto。
4. **主畫面 Unload 顯示**：`ShowUnloadAutoInfo` 每框顯示 Bin（`BinAreaMap.GetBinByArea` 反查或 LotBinBinding）、Lot、ID（`AutoModule->GetWorkingTrayID`）、Cnt（`tRunData.TrayICCnt[eAuto1+i]`）。

### 14.5.4　互鎖與安全

> ⚠️ 注意：**SortArm 放 IC 閘控**（`IsReadyForSortArmPlace`）。AMR 模式僅 `WorkingKind==Normal` 盤允許 SortArm 放 IC，保護 identity/cover 盤不受污染。

其他互鎖：

- **DoFeedTray 進盤確認**：須 `OutputBottomHasTray` 感測到位（或模擬）才繼續，否則 `Feed Tray Miss` 重試。
- **DoFeedTray 推盤確認**：Push 氣缸推盤後須 `IsCylinderOnReady` 確認到位，未到位 Pop 退回並 `Push Tray Miss` 重試。
- **MoveAutoY 軟體極限**：先 `CheckSoftLimit`，超出彈 `Auto Y motor will out of limit` 並拒動。
- **AMR-locked 站**：`FindDischargeAuto`/`GetTrayRequest` 對 `bAmrLocked` 站跳過，停止新出盤/餵料直到 `ClearAmrCar`。
- **ServiceCarFull 實機**：`InputFullTray` 感測 ON 時持續警告，須操作員實際清到感測 OFF 才清資料（最後一道防線）。
- **RefreshAutoState**：模擬/Dummy 直接 return；TrayArm 送來的後方盤以 `bRearDeliveredPending` 鎖存，實體感測讀 OFF 不會清掉邏輯握手。
- **DoAuto**：`RunMode==Run_CleanOut` 且全站清盤完成時直接 return 不動作。

### 14.5.5　AMR/AGV 滿車交接握手（E87 SECS）

1. 滿車判定 `IsOutputCarFullForAmr`：模擬用 `iSimAmrMaxTray` 門檻，實機讀 `InputFullTray` 感測。
2. `SetAmrLock` 鎖站：`GetTrayRequest` 拒收新盤、`FindDischargeAuto` 跳過。
3. `IsDrainedForAmr`（Ready/CEID273）：無作業盤/後方盤/滿盤且 FrontRise 在原位。
4. `IsAmrTaken`（Finish/CEID274）：實機讀 `InputEnd` 感測為 Off=車已被取走（模擬恆 true）。
5. `ClearAmrCar`：清空堆疊車、重建堆疊角色（identity/cover/normal）、解鎖。

> SECS 連線時 `ServiceCarFull` 把滿車交給 AGV 握手而非彈操作員 modal；離線則維持原本人工換車 modal。

### 14.5.6　操作員人工換車（ServiceCarFull，僅 AMR + Run_Normal + SECS 未連線）

1. 實機 `InputFullTray` 感測 ON：彈 `Auto%d output stack FULL (sensor) - remove finished trays`，操作員清空堆疊到感測 OFF 後清車資料。
2. 邏輯滿車（`iTrayCount>=MAX_TRAY_PER_CAR`）：彈 `Auto%d output car full (... trays) - change car then confirm`，確認後清車資料。
3. 模擬模式：邏輯滿車時自動 Clear+重建堆疊，不停機。

### 14.5.7　設定

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `GeneralSetting.bAutoEnabled[0..5]` | 預設全 true；存於 INI `[SortMode] AutoEnabled0..5` | 每站 Auto1~6 啟用旗標；關閉站在 Lot+Bin 路由與置盤掃描中被略過。改動後需重啟軟體。Error Auto 即使關閉仍作溢位目標 |
| `GeneralSetting.bUseAMR` | bool | AMR 堆疊模式總開關（堆疊順序、放料閘控、滿車服務、AGV 握手） |
| `GeneralSetting.iSortMode` | 0/1/2 | 動態路由模式（1 By Lot+Bin / 2 By Lot+PassFail，影響 Unload 面板顯示：PassFail 模式顯示 PASS/FAIL）；路由邏輯在 SortArm |
| `GeneralSetting.iSimAmrMaxTray[3+Index]` | 整數（盤），index 3..8 對應 Auto1~6 | 模擬各 Auto 堆疊車滿車門檻 |
| `Teach.Auto1..6CarFeedTrayYPosition` | 教導值（1/100mm） | 各站取盤高度 Y（DoFeedTray case 1000） |
| `Teach.Auto1..6CarDischargeTrayYPosition` | 教導值 | 各站出盤/堆疊 Y（DoDischargeTray case 1000） |
| `Teach.Auto1..6CarFirstSortYPosition` | 教導值 | 各站作業位（供 SortArm 放 IC）Y（DoFeedTray case 6000） |
| `MAX_TRAY_PER_CAR` | 外部定義常數 | 每台堆疊車最大盤數（滿車判定） |

### 14.5.8　警報與排除

| 警報 | 意義 | 排除 |
| --- | --- | --- |
| `Auto%d Feed Tray Miss` | 進盤後 `OutputBottomHasTray` 未感測到盤 | K_RETRY：回 case 1000 重新取盤 |
| `Auto%d Push Tray Miss` | Push 推盤氣缸未到位 | K_RETRY：回 case 5000 重推 |
| `Auto Y motor will out of limit` | `MAutoY` 目標超軟體極限 | 該次移動取消 |
| `Auto%d output stack FULL (sensor) - remove finished trays` | 實機 `InputFullTray` 感測 ON，堆疊已滿 | 操作員清空到感測 OFF；持續警告直到 OFF 才清資料 |
| `Auto%d output car full (%d trays) - change car then confirm` | 邏輯堆疊車盤數達 `MAX_TRAY_PER_CAR` | 操作員換車後 confirm；清車資料並重建堆疊角色 |
| `Auto enable changed. Please restart the software ...` | 操作員變更 per-Auto 啟用後的提示 | 重啟軟體讓 Lot+Bin 路由生效 |

> 註（定案）：`DoDischargeTray` 的 CEID 陣列 `{136,137,138,140,141,142}` 跳過 139，係因 **139 在 HT9045 CEID 空間已被占用**（`DoVisualSortLotStart`），HT160 為避免撞號刻意跳過（詳見第 12 章）。AMR 模式卸盤時另於 `iAmrDeviceCount[auto] += 工作盤CountIC()` 累計車上 IC 數供 SVID 38231-33/38240-42。maintenance 畫面 `chkAutoEnable1~6` 螢幕文字已由 DFM 確認為「**Auto1**」~「**Auto6**」（英文，無中文）。
>
> 【待補（現場）：實機「車已被取走」感測（`IsAmrTaken` 重用 `SnAutoX_InputEnd`，Lane0/IP2/Port1/Bit0~5）是否已最終接線——未接線（Enable==false）時 AMR 握手會停在 Ready。】
