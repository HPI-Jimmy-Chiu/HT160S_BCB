# Suck2 四真空產生器模式（Suck2 Quad-Vacuum）作戰計畫

日期：2026-07-12
狀態：執行中（owner 2026-07-12 全部同意含 D1~D5）
Owner 定案來源：2026-07-12 對話問答（8 題全數回覆）

## 0. 執行狀態（每步完成即 commit，中斷後由此接續）

- [x] Step 1 GeneralSetting 欄位 bSuck2QuadVacuum（編譯過）
- [x] Step 2 TMySucker gang 機制（MyKitSuck.h/.cpp）（編譯過；Normal 路徑 iGangCount=0 不變）
- [x] Step 3 開機閂鎖（database.cpp gang 佈線；遮罩強制改放 GeneralSetting::Load()，見 3.3 修訂）（編譯過）
- [ ] Step 4 aSortArm 兩處 GetStatus 呼叫端替換（位元組安全）
- [ ] Step 5 maintenance UI（DFM+h+cpp）
- [ ] Step 6 setup.cpp PnP 頁鎖定
- [ ] Step 7 language.txt Big5 附加
- [ ] Step 8 全量驗證關卡（-Clean／真機建置閘／編碼檢查）
- [ ] Step 9 Sim 冒煙＋最終 commit

## 1. 需求（已定案）

新機型硬體：SortArm 只裝 **Suck2 一支吸嘴**（Suck1/3/4 物理上不裝、不用），
但 Suck2 由 **4 組真空產生器** 共同供真空——沿用現有 IO 表的 4 組迴路
（`Suck1~4_On` / `Suck1~4_Off` / `Suck1~4` sensor），配管全部接到 Suck2 吸嘴。

軟體行為（勾選模式後）：

| 動作 | 控制 | 判定 | 不同步時 |
|---|---|---|---|
| 吸料 Suck | 4 組真空同時 ON | 4 顆 sensor **全 ON** 才算吸到 | 原 pick 流程（latch→抬Z→自動 retry→`ShowSuckError` SUC0011） |
| 放料 Destroy | 4 組同時破真空 | 4 顆 sensor **全 OFF** 才算放掉 | 原 place 流程（modal `ShowSuckError` SUC0012） |
| 搬運持料監視 | — | **任一** sensor 失真空（100ms debounce）＝掉落 | 原 fall-down 流程（全機停＋SUC0013） |
| 放料後殘料 re-suck | 4 組同時再吸 | **任一** sensor ON＝殘料 | 原 Residue 流程（SUC0012, K_RETRY） |

- Alarm 訊息沿用原本 "Suck2 Sucker Error"（`SUCxxxx` 自由字串碼），不標示個別產生器。
- UI：`TfMaintenance` → Hardware Setup → `tsOption` 加一顆勾選框。
  文案 EN `Suck2 = 4 Vacuum Generators`／ZH `Suck2=4組真空產生器`。
- 存檔：`General.ini [HardwareInstall] Suck2QuadVacuum`，**重啟才生效**（比照 `rgSortMode` 警告樣板）。
- 「Suck2」＝ 模式下啟用 `chkSuckEnable2`（Nozzle2）作為唯一取放吸嘴。

## 2. 現況架構定位（調查結果，2026-07-12）

- 吸嘴物件：`TMySucker`（`MyKitSuck.h:36-97`）每支一組 `OnSw`（真空閥）/`OffSw`（破真空閥）/`Sensor`（真空檢知）。
  Kit：`HSys.Suck.SortArmSuck` 1x4（`database.cpp:1424-1427`），IO 名 `SuckN` / `SuckN_On` / `SuckN_Off` 綁定於
  `LoadSuckerParameterFromDataBase()`（`database.cpp:1429-1481`）。
- 吸放狀態機：`TMySucker::Suck()`（`MyKitSuck.cpp:114-178`）/`Destroy()`（`:180-246`）——只 latch `Error`，不發alarm；
  alarm 全在呼叫端 `aSortArm.cpp`（pick case 50/52/54 於 `1679-1767`；place `DestroySelectedSlots` `1368-1396`；
  掉落 `CheckHoldFallDown` `1506-1609`；殘料 `CheckPlaceResidue` `1268-1356`）。
- Alarm 發報：`ShowSuckError`（`note.cpp:873-881`）→ `SUC%03d%d`（Tag=1 → SUC0011/0012/0013）→ Note modal。
- 取放吸嘴遮罩：`GeneralSetting.bSuckerEnabled[4]`（`[HardwareInstall] SuckerEnabled0..3`），
  `aSortArm.cpp:880/898`（FindPickCells）每週期即時讀。UI 在 tsSortArm `chkSuckEnable1..4`
  （`maintenance.cpp:1960-1990`）與 Setup PnP 頁（`setup.cpp:450-582`）。
- 開機順序（重啟閂鎖點成立）：`database.cpp:761 InitialCosFunction()`（內含 `GeneralSetting.Load()`，
  `CosFunction.cpp:1838`）**先於** `database.cpp:795 LoadSuckerParameterFromDataBase()`。
- 重啟警告樣板：`rgSortModeClick`（`maintenance.cpp:1907-1920`，`ShowMyMessage` 提醒不強制）。
- tsOption 現況（`maintenance.dfm:1995-2045`）：`Panel5` 上只有 `cbBinPanelType`＋`cbCommType`＋
  `chkUseTrayDatumModel`（死控件）；Load/Save 於 `LoadHardwareSettings`（`maintenance.cpp:1084-1161`）/
  `SaveHardwareSettings`（`:1163-1215`）。
- iosetview：Suck 手動 V/D 走 `SuckerPtr->On()/Off()/OffSuck()/OffDestroy()`（`iosetview.cpp:1722-1762`）；
  LED `mlSuck1..4` 顯示各自 `Sensor.IsOn()`。
- HT172 無此模式可參考（kit 皆固定幾何，無多產生器選項）——本功能為 HT160S 新設計。

### 檔案編碼盤點（2026-07-12 實測）

| 檔案 | 非ASCII | 編輯手法 |
|---|---|---|
| MyKitSuck.h/.cpp、GeneralSetting.h/.cpp、database.cpp、maintenance.h/.cpp/.dfm、setup.cpp、iosetview.cpp | 0 | Edit 工具可用 |
| **aSortArm.cpp** | 7 bytes（第 1 行 Big5 註解） | **僅限位元組安全 PowerShell 編輯**；改後重掃非ASCII數須仍=7 |
| system\language.txt | Big5 | PowerShell 以 Big5 編碼附加 |

## 3. 設計

### 3.1 GeneralSetting（新欄位）

`bool bSuck2QuadVacuum;` 預設 false；`[HardwareInstall] Suck2QuadVacuum`。
只有**開機閂鎖值**驅動行為；UI 改值只寫 ini＋警告重啟。

### 3.2 TMySucker「群組（gang）」機制

- 新增成員：`TMySucker *pGang[MAX_SUB_SUCKER_ITEM]; int iGangCount;`（建構子歸零；`iGangCount==0` = Normal，行為 100% 不變）。
- 輸出扇出：`OnSuck/OffSuck/OnDestroy/OffDestroy` 當 `iGangCount>0` 時迴圈驅動所有成員的
  `OnSw`/`OffSw`（`On()/Off()/Normal()` 由上述組成，自動繼承扇出）。
- 判定 helper（Normal 模式即等於現行 `Sensor.IsOn()`）：
  - `SensorAllOn()`：quad＝所有成員（`Sensor.Enable` 者）全 ON；用於「吸到」判定。
  - `SensorAnyOn()`：quad＝任一成員 ON；用於「還沒放掉／殘料」判定。
  - `GetStatusAllOn()` / `GetStatusAnyOn()`：比照 `GetStatus()` 的非 REALLY 直接回 true 慣例，供 aSortArm 呼叫端替換。
- `Suck()` 判定點替換：Task1 `Sensor.IsOn()==false` → `SensorAllOn()==false`；Task50 `Sensor.IsOn()` → `SensorAllOn()`。
- `Destroy()` 判定點替換：Task1 `Sensor.IsOn()` → `SensorAnyOn()`；Task50 `Sensor.IsOn()==false` → `SensorAnyOn()==false`。
- 時基：以 master（Suck2）的 `OnAlarmTime/OffAlarmTime/OnDelayTime/OffDelayTime` 為準（現 4 組同為 5000/5000/30/10ms）。
- 暫停凍結（`csystem.cpp:1245-1279`）凍的是 4 支的 `Delay`，master 在內，不需改。

### 3.3 開機閂鎖（database.cpp）

`LoadSuckerParameterFromDataBase()` 尾端：若 `GeneralSetting.bSuck2QuadVacuum`：
`Suck[0][1]`（Suck2）設為 master：`pGang[0..3]=&Suck[0][0..3]`、`iGangCount=4`。

**執行時修訂（2026-07-12）**：遮罩強制不放 database（原設計），改放
`THT160GeneralSetting::Load()` 內——因維護頁每次開啟都重跑 `Load()`（maintenance.cpp:1086），
若只在開機強制一次會被後續 Load 洗掉。現在 quad=true 時每次 Load 都在記憶體強制
`bSuckerEnabled={F,T,F,F}`（不回寫 ini），防手改 ini 更徹底。

### 3.4 aSortArm.cpp 呼叫端替換（僅 2 處，位元組安全編輯）

- `CheckHoldFallDown`：持料判定 `Sucker->GetStatus()` → `GetStatusAllOn()`（任一失真空＝掉落）。
- `CheckPlaceResidue` Task 300：`Sucker->GetStatus()` → `GetStatusAnyOn()`（任一 ON＝殘料）。
- `SuckSelectedSlots` / `DestroySelectedSlots` / 各 alarm 呼叫**不動**——quad 模式下只有 slot 1 會被選取，
  alarm 自然就是 "Suck2 Sucker Error"。

### 3.5 maintenance UI

- DFM 手工加 `chkSuck2QuadVacuum: TCheckBox` 於 tsOption `Panel5`（Left 730, Top 8, Width 190，
  Caption ASCII `'Suck2 = 4 Vacuum Generators'`，`OnClick=chkSuck2QuadVacuumClick`）。
- `maintenance.h` `__published:` 加控件指標＋handler 宣告（form class body 內不加註解）。
- `maintenance.cpp`：
  - `LoadHardwareSettings`：反映勾選值＋依 quad 鎖定 `chkSuckEnable1..4`。
  - `SaveHardwareSettings`：回寫欄位。
  - handler（比照 `rgSortModeClick`）：`bLoadingHardwareSettings` guard → 寫欄位 →
    勾上時強制並**保存** `bSuckerEnabled={F,T,F,F}`＋同步 tsSortArm 勾選框顯示＋鎖定 →
    `GeneralSetting.Save()` → `RefreshHardwareSettingsStatus()` →
    `ShowMyMessage("Suck2 quad-vacuum mode changed. Please restart ...")`。
  - `ApplyHardwareEditLock` 納入新勾選框（Lot 運轉中鎖定）。
- `system\language.txt`（Big5）附加：
  `fMaintenance<TAB>chkSuck2QuadVacuum<TAB>Suck2 = 4 Vacuum Generators<TAB>Suck2=4組真空產生器`
  （執行時先核對現有行的分隔格式再照樣附加）。

### 3.6 Setup PnP 頁

`GeneralSetting.bSuck2QuadVacuum==true` 時：`rgPnpUseSuck`＋`grdSuckEnable` 鎖定
（`LoadSuckEnable` 設 disabled；`grdSuckEnableMouseUp`/`rgPnpUseSuckClick` 開頭 early-return）。

### 3.7 iosetview（**不改碼**，預設決策 D3）

- Suck2 的 V/D 鍵經 `On()/Off()` 自動扇出 4 組——符合直覺。
- Suck1/3/4 的 V/D 鍵保留單顆動作＝**逐顆產生器診斷工具**。
- `mlSuck1..4` LED 維持各自 sensor 顯示（查哪顆沒到位）。
- 群組鍵（Suck All/Destroy All）行為不變（quad 下效果冪等）。

## 4. 預設決策（owner 可否決）

| # | 決策 | 理由 |
|---|---|---|
| D1 | 勾選 quad 時**同步強制並保存** `SuckerEnabled=僅Nozzle2`，並鎖住兩處 Nozzle UI；**取消 quad 不自動還原**（operator 自行重勾） | 該機型 Suck1/3/4 物理不存在，殘留舊遮罩才是風險 |
| D2 | 閂鎖點＝`LoadSuckerParameterFromDataBase`，並在記憶體強制遮罩（防手改 ini 不一致） | 雙保險，改動最小 |
| D3 | iosetview 維持逐顆 V/D 與逐顆 LED（見 3.7） | 保留維修診斷能力；Suck2 鍵已自動連動 |
| D4 | 同步判定時基＝Suck2 的 OnAlarmTime/OffAlarmTime | 4 組現值相同（5000ms），單一時基最簡單 |
| D5 | 群組鍵 Suck All/Destroy All 不改 | quad 下冪等無害 |

## 5. 實作步驟（每步獨立可驗證）

> 每步 C++/DFM 改動後：刪除該檔 `.obj` → `scripts/ops/build-ht160s.ps1` 編譯，exit 0 才進下一步。

1. **GeneralSetting 欄位**：`bSuck2QuadVacuum` 宣告＋`SetDefault/Load/Save`（`[HardwareInstall] Suck2QuadVacuum`）。
   驗證：編譯過；手放 ini 鍵可讀（debug 或下一步一併驗）。
2. **TMySucker gang 機制**（MyKitSuck.h/.cpp）：成員、建構子歸零、四個輸出 primitive 扇出、
   `SensorAllOn/SensorAnyOn/GetStatusAllOn/GetStatusAnyOn`、`Suck()/Destroy()` 判定點替換。
   驗證：編譯過；**Normal 模式行為不變**（iGangCount=0 路徑逐點覆核 diff）。
3. **開機閂鎖**（database.cpp）：3.3 的 gang 佈線＋記憶體遮罩強制。
   驗證：編譯過；sim 下 ini 開 quad → 開機後僅 Nozzle2 取放（FindPickCells 行為）。
4. **aSortArm 兩處呼叫端替換**：`GetStatusAllOn`/`GetStatusAnyOn`。
   ⚠ 位元組安全 PowerShell 編輯；改後重掃非ASCII＝7 bytes（第 1 行）不變。
   驗證：編譯過＋位元組掃描。
5. **maintenance UI**：DFM 控件＋`.h` `__published`＋Load/Save＋handler＋`ApplyHardwareEditLock`＋Nozzle 勾選框鎖定。
   驗證：編譯過；sim 開 UI：勾選→跳重啟警告、ini 寫入、Nozzle1~4 變灰、Lot 運轉中鎖定。
6. **setup.cpp PnP 頁鎖定**。驗證：編譯過；quad 下 grid/radio 不可改。
7. **language.txt** Big5 附加一行。驗證：以 Big5 讀回核對；切 EN/ZH 勾選框文案正確。
8. **全量驗證關卡**：
   - `scripts/ops/build-ht160s.ps1 -Clean` exit 0。
   - 真機建置閘：`MachineType.h` 註解 `#define SOFT_SIMULATE` → `-Full` exit 0 → **還原** define → 重建（MyKitSuck/aSortArm 屬共用核心，必做）。
   - `scripts/ops/check-ht160s-source-encoding.ps1` 通過。
9. **Sim 冒煙**：quad ON——只有 Nozzle2 取放、iosetview Suck2 V 鍵 4 組輸出同動、Suck1/3/4 鍵單動；
   quad OFF——與現行為完全一致。通過後 scoped commit＋push
   （**不含**目前已存在的 `system/AlarmList.csv` 未關聯修改）。真機吸放/掉落/殘料驗證由 owner 執行。

## 6. 風險與注意

- `aSortArm.cpp` 第 1 行 Big5 註解：全程位元組安全編輯，Edit 工具禁用於此檔。
- `maintenance.dfm` 手工編輯後**不可**再用 BCB designer 開啟存檔（會剝除元件）。
- IO 表 `Suck4` sensor `InType=0` 與其他三顆（=1）極性不同——新機接線/IO_Table.csv 需先對齊，
  否則 AllOn/AnyOn 判定會誤報。上機前 owner 需確認。
- 假設新機 4 顆 sensor row 於 IO 表皆 `Enable=1`（判定 helper 只計 `Sensor.Enable` 者，
  若某顆被關掉會自動退化為 3 顆同步——此為容錯而非設計目標）。
- place case 50 成功後的「再吹氣 dwell」（`OnDestroy` 再開、case 70 `OffDestroy` 關）經扇出後為 4 組同吹／同關，行為一致。
- `TMySucker::CheckIsFallDown`／`eSuckIniOffErr/OnErr`／kit 級 `ShowSuckError` overload 皆無 runtime 呼叫者，不在本案範圍。
