# 第 02 章　系統概觀與機構

本章為機台的整體導覽（orientation），說明 HT160S 是什麼、由哪些主要機構模組組成，以及料盤從進料到出料的整體物料流向。各模組的詳細操作、教導值與警報處理，請參閱第 14 章（各模組詳述）；本章僅做總覽，不深入單一模組。

## 2.1 HT160S 是什麼

HT160S 是一台 **料盤分類／搬運機（tray sorting / handling machine）**。它把來料盤上的每一顆 IC 透過頂部 CCD 逐格掃描（讀取每格的有無料、Bin 與 2D 條碼），依 Bin／Lot 路由規則，由分類臂（SortArm）的吸嘴逐顆吸取，放到對應的出料堆疊站（Auto1~6）；空盤則由盤臂（TrayArm）與空盤模組（Empty）負責供應與回收。整機由主畫面（TfMain）監看與操作，內建 SECS/GEM 與 AMR/AGV 補料／取車交握能力。

![主畫面總覽](screenshots/main-overview.png)
> 圖 2-1 HT160S 主畫面總覽，顯示頂部功能列、生產計數、Real/Dummy 與 Start Mode、SECS/SAFE/AMR 狀態徽章、三色塔燈與機台狀態文字。（擷取方式：開機後即進入此常駐全螢幕視窗 `fMain`；其分頁 `pgcMain` 預設停在 Main 主操作頁 `tsMain`。）

## 2.2 主要機構模組

HT160S 由以下機構模組構成。多數機構模組為「純邏輯／純控制模組」，無自身畫面，由主控迴圈（`database.cpp` 的 `ProcessMotion`）以非阻塞 `switch(Task)` 程序式階梯逐拍驅動；其運轉狀態與計數投影到主畫面顯示。

| 模組 | 程式類別 | 角色 |
| --- | --- | --- |
| Loader 進料模組 | `TLoaderModule` | 雙側（Loader1 左／Loader2 右）來料盤的取放、頂部 CCD 掃描與索引、空盤後排排出 |
| Color 顏色模組 | `TColorModule` | 供應「身份盤（identity tray）」給後段 TrayArm（預設 TraySupply 供盤模式），含 Color 2D CCD 讀碼 |
| Empty 空盤模組 | `TEmptyModule` | 從前段堆疊逐片取下空盤、搬移送入後段供 TrayArm 取走；批次結束或回收時反向上推 |
| SortArm 分類臂 + 吸嘴 | `SortArmModule` | 以一排 4 個吸嘴（Suck-Z）從 Loader 來料盤逐顆吸取 IC，依 Bin/Lot 放到 Auto1~6 |
| TrayArm 盤臂 | `TrayArmModule` | 單軸搬運臂，從 EmptyTray／Loader 後方／Color 取空盤或身份盤，搬送並放到目標 Auto 後方或回收 |
| Auto1~6 出料堆疊 | `TAutoModule` | 6 個出料堆疊站，把分料後滿料的盤子堆入堆疊車（output car），含 CleanOut 與 AMR 滿車交接 |
| Top CCD 頂部相機 | （Loader / Color 模組共用視覺） | Loader 逐格讀 Bin/2D 條碼；Color 端讀身份盤 2D TrayID |

> ⚠️ 注意：Bin → Auto 的路由判定**不在** Auto 模組內，而是由 SortArm 的 `GetMappedAutoIndex` 搭配 `BinAreaMap` / `LotBinBinding` 決定後，再把盤放入對應 Auto。

### 2.2.1 Loader 進料模組（TLoaderModule）

雙側（Loader1 左 / Loader2 右）共用同一條 Loader-Y 物理導軌、共用前置疊盤機台與後排排出口。每側透過 Loader-Y 軸把盤子移到取盤位、用前置疊盤氣缸分出一盤、夾緊上料，再帶到 Top CCD 下方逐格掃描每顆 IC（讀 Bin 與 2D 條碼），把結果寫入該側盤面資料供 SortArm 取料分類。CCD 掃描完成的一側標記為 `LS_READY_SORT` 供 SortArm 接管；盤面全部排空後排到後方供 TrayArm 取走。

主畫面上的盤面區塊：

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `grpLoaderL` | groupbox | 主畫面左側 Loader（Loader1）盤面區塊標題（2D Left），含格點顯示與盤號/Bin 標籤 |
| `grpLoaderR` | groupbox | 主畫面右側 Loader（Loader2）盤面區塊標題（2D Right），含格點顯示與盤號標籤 |
| `mtWorkArea` | grid (TTMyTray) | 左側 Loader 盤面格點（4x5），顯示每格 IC 掃描/分類狀態（UNCHECK/EMPTY/HAS_OK/錯誤碼） |
| `mtSortRecv` | grid (TTMyTray) | 右側 Loader 盤面格點（4x5）狀態顯示 |

### 2.2.2 Color 顏色模組（TColorModule）

負責供應身份盤給 TrayArm。預設為 TraySupply（供盤）模式：先在前方收/讀位置（`ColorRead2DYPosition`）由破棧氣缸分離一片盤並夾住，透過 Color 2D CCD 讀取盤上的 2D TrayID，再用 ColorY 軸把載盤搬到後方取盤位置（`ColorTrayArmPickYPosition`）讓 TrayArm 夾取。實際供盤只在收到 AMR/TrayArm 真正需求（`bSupplyRequested`）時才推到輸出並讀碼，避免提前呈現身份盤。座標慣例：Y = 前/後、X = 左/右、Z = 上/下。

> ⚠️ 注意：Color 區受安裝閘控制——`GeneralSetting.bColorBinAreaInstalled` 為 `false`（`IsInstalled()==false`）時整個模組停用。

### 2.2.3 Empty 空盤模組（TEmptyModule）

從前段堆疊以雙缸頂升/分張機構逐片取下空盤（GoDown），再以斜靠 + 推送雙缸夾住料盤、由 MEmptyY 軸搬移至放料位送入後段（Feed），供 TrayArm 取走。批次結束或需要回收時，可反向把後段料盤重新夾起、上推回前段堆疊（GoUp/Return）。盤態以動作序列為主的旗標鎖存（latch）：盤子下來＝有盤，MotorY/TrayArm 夾走＝沒盤。

### 2.2.4 SortArm 分類臂與吸嘴

用一排 4 個吸嘴（`SORT_ARM_SUCKER_COUNT` = 4，Suck-Z）從 Loader 來料盤逐顆吸取 IC（單顆取放 SINGLE-PICK，每行程只取/放一顆），依路由 Bin 放到 Auto1~6（`SORT_ARM_AUTO_COUNT` = 6）。橫移前先升到安全 Z 位置（`SORT_ARM_SAFE_Z_POSITION` = 10）。

> ⚠️ 注意：分類臂 X 軸（`MoveSortArmX`）每拍都先過 `AreAllSuckersHome()`——任一啟用的 Suck-Z 不在 Home 感應器即不准移動，避免吸嘴下伸時橫移撞料/撞框。此防撞互鎖只在編譯期 `SOFT_SIMULATE` 模擬版被略過，實機 DUMMY/HAS_TRAY/REALLY 三種模式皆持續生效。

### 2.2.5 TrayArm 盤臂

單軸搬運臂，依 `DecideJob` 派工優先序取盤搬運：1) Loader 後方滯留空盤回收；2) AMR 模式依 Auto 需求堆疊 identity/cover/normal 盤；3) Normal 模式 EmptyTray → Auto 補空盤。取盤來源依工作別不同：Loader 後方、Color（取 identity 盤）、或 EmptyTray 後方。

> ⚠️ 注意：盤臂 X 軸（`MoveTrayArmX`）每拍都先過 `IsZUpAtPosition()`——Z 升降氣缸須在上位（`C_TrayArmZ_Up` On）才准橫移，避免頭/盤在低位時橫掃撞站。同樣只略過編譯期 `SOFT_SIMULATE`，實機三模式皆生效。

### 2.2.6 Auto1~6 出料堆疊模組（TAutoModule）

管理 6 個出料堆疊站。每站把盤子在「後方接收位（rear）」、「作業位（working/sort）」與「堆疊車（output car）」之間搬移：TrayArm 把空盤（或 AMR 模式的 identity/cover/normal 盤）放到後方，`DoFeedTray` 用 Lean/Push 氣缸把盤夾入並由 MAutoY 軸帶到作業位供 SortArm 放 IC，盤子滿料（FullIC）後 `DoDischargeTray` 以 FrontRise 氣缸把盤堆入堆疊車。同時負責整機 CleanOut 清盤與 AMR/AGV 滿車交接握手，並輸出每站工作盤的 2D TrayID 與計數供主畫面 Unload 區顯示。

主畫面下料資訊面板：

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `palAuto01..06` Bin/ID/Cnt + `plLotNumberAuto1..6` | grid | 下料 Auto1~6 即時資訊（Bin / Lot / ID / Cnt），由 `ShowUnloadAutoInfo` 填入 |

## 2.3 整體物料流向

料盤的整體流程如下（以 ASCII 流向圖呈現）：

```
                      [ 來料堆疊 (input stack) ]
                                |
                                v
      +-----------------------------------------------+
      |  Loader (Loader1 左 / Loader2 右, 共用 Y 導軌)  |
      |   1. 前置疊盤分出一盤 -> 夾緊上料               |
      |   2. Top CCD 逐格掃描 (Bin + 2D 條碼)          |
      |   3. 2D 反查 Lot/Bin, 寫入盤面資料             |
      +-----------------------------------------------+
                                |  (LS_READY_SORT)
                                v
            +-----------------------------------+
            |  SortArm (4 吸嘴, 逐顆 SINGLE-PICK) |
            |   依路由 Bin/Lot -> GetMappedAuto  |
            +-----------------------------------+
                                |
                                v
       +--------------------------------------------------+
       |   Auto1   Auto2   Auto3   Auto4   Auto5   Auto6   |
       |   (依 Bin/Lot 分流, 作業位收 IC -> 滿盤堆入車)      |
       +--------------------------------------------------+
                                |  (FullIC)
                                v
                    [ 堆疊車 output car -> 滿車交接 ]

   空盤供應路徑:
   [ Empty 空盤堆疊 ] --GoDown/Feed--> 後段 --> TrayArm --> Auto 後方接收位
   [ Color 身份盤 ]  --(AMR 需求)-->  讀 2D TrayID --> TrayArm --> Auto 後方
```

整體可分為三條主線：

1. **掃描線（讀料）**：來料盤進 Loader → 前置疊盤分一盤、夾緊上料 → Top CCD 逐格掃描讀 Bin 與 2D 條碼 → 2D 反查 Lot/Bin 寫入盤面 → 標記 `LS_READY_SORT`。
2. **分類線（搬料）**：SortArm 向 Loader 取得 Y 軸獨佔權後逐顆吸取 IC → 依路由 Bin/Lot 由 `GetMappedAutoIndex` 對到 Auto → 放到該 Auto 作業盤 → 盤滿（FullIC）後 Auto 模組把盤堆入堆疊車。
3. **空盤線（供盤/回收）**：Empty 從堆疊取空盤送後段，或 Color 供身份盤；TrayArm 搬到目標 Auto 後方接收位；空盤/滿盤的後送與回收由 TrayArm 與各模組以 Notify/Request 介面交握。

Loader 內部狀態旗標序列：`LS_IDLE → LS_FEEDING → LS_CCD_SCAN → LS_READY_SORT →（SortArm 接管）LS_SORTING → LS_ToRear →` 排出後回 `LS_IDLE`。

## 2.4 跨模組協同與安全原則

整機運轉建立在「需求驅動」與「防撞互鎖」兩大原則上：

1. **需求驅動（just-in-time）**：Color 供盤只在 `bSupplyRequested` 為真時才推到輸出讀碼；TrayArm 依 `DecideJob` / `FindTrayRequestAuto` 按 Auto 實際需求派工；Auto 各站以 `GetTrayRequest` 回報目前想要的盤種。避免提前呈現/搬運盤子。
2. **共用機構互鎖**：Loader 雙側共用 Y 導軌、共用前置疊盤機台與後排出口，故有跨側安全距離互鎖（`IsLoaderYMoveSafe`，預設 `GeneralSetting.iLoaderYSafeDistance` = 10000 = 100mm）、共用疊盤機互斥（`AcquireFrontOwner`）、以及 SortArm 對 Loader-Y 的軸獨佔握手（`AcquireSortOwner` / `IsSortOwnerHeld`）。
3. **防撞硬安全法則**：SortArm 全吸嘴回原點互鎖、TrayArm Z 上位互鎖，皆只略過編譯期 `SOFT_SIMULATE`；實機 DUMMY/HAS_TRAY/REALLY 三模式一律生效（因 DUMMY 下馬達與氣缸仍實際運動，僅略過正確性感應器確認）。
4. **AMR/AGV 交握**：Loader/Empty/Color 提供補料 Ready/Shortage/Finish 介面（交握期間以 `bAmrLocked` 凍結前段破棧/取盤）；Auto 提供滿車 Drained/Taken 交握（E87 SECS 協調），SECS 連線時把滿車交給 AGV，離線則維持人工換車。

> ⚠️ 注意：所有等待採非阻塞設計（返回 false 由上層輪詢重試），機台控制路徑不使用 `Sleep()` 或 modal 阻塞（警報彈窗除外）。

## 2.5 運轉模式與層級設定（總覽）

下列為與整機運轉相關的關鍵設定，操作細節見後續各章。

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `iRealDummy`（`HSys.LastSet.iRealDummy`） | DUMMY / HAS_TRAY / REALLY | 運轉模式，影響感測器/CCD/拍照是否走實機路徑（三層 IO 檢查層級）；於主畫面 `pnRealDummy` 切換，僅停機可改 |
| `iStartMode` | 0=Initial / 1=Continue | 起動模式：初始起動或續做；於 `pnStartMode` 切換，僅停機可改 |
| `GeneralSetting.bUseAMR` | true/false | AMR/AGV 堆疊模式總開關，決定堆疊順序 identity→cover→normal、滿車服務與 AGV 握手 |
| `GeneralSetting.bUseLotBinSortMode` | true/false | By Lot+Bin 動態路由模式（2D 反查後 `ResolveAuto` 動態綁定 (Lot,Bin)→Auto） |
| `CosFunction.bUse2DBinMap` | true/false | 是否啟用 Top CCD 2D 條碼讀取與 Bin2DMap 反查（關閉則只讀有無料的 Bin） |
| `CosFunction.bUseColorCcd` | true/false | `[ColorCCD] Enable`；false 時 Color 跳過相機、2D 碼留空，供盤照常進行 |
| `CosFunction.bUseSecsGem` | true/false | SECS/GEM 功能旗標，決定 SECS 徽章是否顯示與可點擊 |
| `GeneralSetting.iLoaderYSafeDistance` | 預設 10000（=100mm）；≤0 停用 | 兩側 Loader-Y 車跨側最小安全間隔（編碼器單位，1/100mm） |

> 註：位置/教導值單位為 1/100mm（100 units/mm）；mm 與設定值換算為 ×100 / ÷100。

## 2.6 待補項目

> 【待補：以下為本章導覽涉及、需現場/後續章節確認之項目】

- 主畫面 `mtWorkArea`/`mtSortRecv`（4x5 盤面格點）與 `mtLoaderLTrayWork`/`mtLoaderRTrayWork`（移動盤面）的對應關係，以及哪個顯示生產中盤面、哪個顯示移動盤面，需與 `main.cpp` 顯示綁定一併確認。
- 「Loader 2D Left/Right」文字標籤與物理 Loader1/Loader2（左/右）的程式綁定（`LoaderNo` 與左右標籤對應）未明示，需現場確認。
- 主畫面頂部功能列與監看選單多採點陣圖示（DFM Hint 多統一為「Change Language」），實際螢幕圖示/標籤文字需以現場截圖確認。
- 堆疊順序 identity/cover/normal 的盤種定義（`eTrayKindIdentity/Cover/Normal`）與 `MAX_TRAY_PER_CAR` 實際數值定義在外部標頭，需另行對照。
- 各模組氣缸/感測器的實際 IO 點位與中文標籤須對照 IO_Table / 機構表確認（本章僅引用識別字）。
