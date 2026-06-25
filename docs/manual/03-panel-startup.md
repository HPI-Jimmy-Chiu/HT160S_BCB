# 第 03 章　操作面板與開機啟動

本章說明 HT160S 主畫面 (TfMain / fMain) 的操作面板配置，以及從「首次開機」到「開始生產」的完整流程。內容涵蓋頂部工具列、Real/Dummy 與 Start Mode 切換、各狀態徽章 (SECS / SAFE / AMR) 與三色塔燈、全機歸原 (HOME) 監看畫面，以及 START / PAUSE / STOP / One Cycle / Clean Out 等運轉控制。發生故障時的處理與警示彈窗，請參閱第 13 章。

---

## 3.1 主畫面總覽

主畫面是開機後常駐的最上層全螢幕視窗。最外層為 `pgcMain` 分頁，分成 **Main 主操作頁** (`tsMain`) 與 **MonitorView 監看頁** (`tsMonitorView`)：

- **Main 頁**：頂部功能列、生產計數區、`Recipe Name` 與 `User` 選擇、Real/Dummy 與 Start Mode 切換、SECS/SAFE/AMR 狀態徽章、三色塔燈與機台狀態文字。
- **MonitorView 頁**：`Home / Start / Pause / One Cycle / Clean Out / Hangup` 等運轉控制按鈕，以及 Motion / Motor / Record / Other 等即時監看分頁。

畫面同時承載 Lot 管理、Tray Status、Logs、Time Data、Map of Tray、Simulation 等子分頁。

![主畫面與工具列](screenshots/main-overview.png)
> 圖 3-1 主畫面與工具列。（擷取方式：程式啟動後預設即停留於此畫面；若在 MonitorView，按 `btnMainShow` 回主畫面）

### 頂部工具列（Main 頁）

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `sbLaguage` | button | 開啟語言設定畫面 (`ShowTopForm(fLan)`)；以點陣圖示呈現，提示文字為 Change Language |
| `sbProduct` | button | 開啟產品/配方設定畫面 `fSetup` |
| `sbMaintance` | button | 開啟維護畫面 `fMaintenance` |
| `sbOffset` | button | 開啟 Offset 偏移補正畫面 `fOffset` |
| `sbSpeed` | button | 開啟速度設定畫面 `fSpeed` |
| `sbTool` | button | 開啟系統工具畫面 `FormSysTools` |
| `sbMessage` | button | 開啟訊息/警報視窗 `fNote` |
| `sbMonitor` | button | 切換至 MonitorView 監看分頁 (`tsMonitorView`) |
| `sbExit` | button | 關閉主視窗離開程式；`FormClose` 會先終止 `MyThread` 工作緒並釋放 Alarm 物件 |

### MonitorView 頁分頁切換

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `btnMainShow` | button | 從 MonitorView 切回 Main 分頁 (`tsMain`) |
| `sbRecord` | button | 切換 `pgcMonitor` 至 Record 分頁 (`TabRecord`) |
| `sbMotorView` | button | 切換至 Motor View 馬達狀態分頁 (`tsMotorView`) |
| `sbMotionView` | button | 切換至 Motion View 動作模擬分頁 (`tsMotionView`) |
| `sbOther` | button | 切換至 Other 分頁 (`TabOther`)，含 Lock/IO/Suck 狀態 |

> 【待補：頂部功能列與 MonitorView 選單多數採點陣圖示，且 DFM 內 Hint 統一為 Change Language；表中標籤名稱依處理函式語意推得，確切螢幕圖示文字待現場截圖確認。】

---

## 3.2 運轉控制按鈕

以下按鈕位於 MonitorView 頁，是日常運轉的核心操作。

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `sbHome1` | button | 啟動全機歸原。非模擬時先彈出 `Confirm home?`；確認後清空 Home 監看清單、顯示 `fHome`、進入 `Run_Home`、`SystemStart=true`、`ArmMotorHome()` 重置歸原旗標、`bHomeByStart=false`（完成後停機待命） |
| `sbStart1` | button | 啟動生產。先以 `CheckLotDataReady` 檢查 LotID 與 2D/Bin 資料，通過後 `SystemStart=true`、`SoftStart=true`、開始 per-IC 生產；若機台未歸原 (`fAllMotorHome==false`) 則以 `bHomeByStart=true` 先歸原，完成後自動接續生產 |
| `sbPause1` | button | 暫停運轉。若 `SystemStart` 為真則 `SystemStart=false`、`SoftStop=true` 觸發減速停止；保留歸原狀態，可再按 Start 續跑 |
| `sbOneCycle1` | button | 單循環。僅允許在 `Run_Normal` / `Run_CleanOut` 模式且 Lot 資料就緒；切到 `Run_OneCycle`，完成放料後回 Normal 並停機 |
| `sbCleanOut1` | button | 清機。僅在 `Run_Normal` 時生效，設 `bCleanOut=true` 切到 `Run_CleanOut`，排出機台內殘料 |
| `sbStoreHangup` | button | 手動觸發 State Record 快照 (`TriggerSnapshot(Manual)`)，打包 TaskHistory/MachineState/config 成 zip 至 `D:\HT160S_StateRecord\`，成功後以檔總管選取該 zip |

> ⚠️ 注意：`Start` 與 `One Cycle` 必須通過 `CheckLotDataReady` 檢查——LotID 不可空白、`LotRegistry` 至少一筆 Lot、且至少一筆 2D/Bin 記錄；若為 By Lot+Bin 模式還須有綁定，否則中止啟動並彈出提示。

---

## 3.3 Real/Dummy 與 Start Mode 切換

主畫面 `pnlSetting` 區塊提供兩個運轉前置設定，兩者**僅在停機狀態 (`SystemStart==false`) 可變更**，運轉中點擊直接忽略。

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `pnRealDummy` | button | 點擊循環切換 `iRealDummy`：`DUMMY → HAS_TRAY → REALLY → 回 DUMMY`；更新圖示/文字、寫入 ini、記錄並 SECS 回報 (`EventReport(RealDummy)`)；Caption 對應 Dummy/HasTray/Real |
| `sbRealIcon` | button | 與 `pnRealDummy` 共用點擊事件；依 `iRealDummy` 載入 `picture\status\real.bmp` 或 `dummy.bmp` |
| `pnStartMode` | button | 點擊切換 `iStartMode`（0=Initial 初始起動 ↔ 1=Continue 續做）；更新圖示/文字、寫入 ini、記錄 |
| `sbStartIcon` | button | 與 `pnStartMode` 共用點擊事件；依 `iStartMode` 載入 `initial.bmp` 或 `continue.bmp` |
| `lblRunMode` | label | `Real/Dummy` 區塊標題標籤 |
| `lbStartMode` | label | `Start Mode` 區塊標題標籤 |

### 設定值

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `iRealDummy` | DUMMY(0) / HAS_TRAY(1) / REALLY(2)，預設 DUMMY | 運轉模式，影響三層 IO/感測檢查層級；存於 INI `[System] RealDummy`。離子風扇警報等實機檢查僅 REALLY 模式生效 |
| `iStartMode` | 0=Initial / 1=Continue，預設 0 | 起動模式；存於 INI `[System] iStartMode` |

> 【待補：`iStartMode=Continue` 在 `Start()` 流程中的實際差異行為。`CheckContinusStartIsReady()` 已被註解停用，目前 Initial 與 Continue 在啟動路徑上看不出程式分支差異，其語意是否僅供記錄/SECS 待現場確認。】

> 【待補：DUMMY / HAS_TRAY / REALLY 三模式對機台實際 IO/動作的差異，本章僅見 REALLY 啟用離子風扇檢查；其餘差異需查各模組與三層 IO 檢查文件確認。】

---

## 3.4 狀態徽章與三色塔燈

### 機台狀態文字 (`palMainStatus`)

主畫面顯示一個即時機台狀態文字，由 `ProcessRunStatus` 判定並以 `SetMainStatus` 寫入，同步對應紅/綠/黃色：

| 狀態 | 觸發條件（節錄） |
| --- | --- |
| `INIT` | 程式尚未啟動 |
| `HOMING`（綠） | `SystemStart` 且尚未歸原 |
| `Clean Out`（黃） | `Run_CleanOut` |
| `Tray Feed`（黃） | `Run_TrayFeed` |
| `One Cycle`（黃） | `Run_OneCycle` |
| `RUNNING`（綠） | 其餘運轉中狀態 |
| `LOCK` | 停機且 `IsSafeLock` |
| `EMG` | 停機且 `IsEMGPressed` |
| `MOTOR OFF` | 停機且 `IsSystemPowerOff` |
| `SAFE DOOR` | 停機且 `IsSafeDoorOpen` |
| `AIR` | 停機且 `IsAirCheck` |
| `PAUSE` | 停機且 `HasICUnderMachine` |
| `HALT` | 以上皆非的預設停機狀態 |

> ⚠️ 注意：`LOCK / EMG / MOTOR OFF / SAFE DOOR / AIR` 是安全/電源/氣壓條件的**顯示結果**，並非由主畫面直接控制的閉鎖。需排除對應條件後，狀態才會自行恢復；故障停機的處理與彈窗，詳見第 13 章。

### 三色塔燈 (`pnlLight`)

`pnlLight` 內含 `ledRed` / `ledYellow` / `ledGreen` 三顆 TALed，鏡射實體塔燈。由 `DoSystemMessage` 依 `GetTowerLightRunState` / Config 設定亮燈，並同步驅動實體 `SwTowerRed/Yellow/Green` 與蜂鳴器。

### 功能徽章

| 控制項 | 類型 | 標籤/狀態 | 功能 |
| --- | --- | --- | --- |
| `pnlFeatureBadge1` | led | `SECS : OFF / CONNECT / ONLINE` | SECS/GEM 連線徽章。`UpdateSecsFeatureBadge` 依 HSMS 狀態設 OFF(灰)/CONNECT(橄欖)/ONLINE(綠)；僅在 `bUseSecsGem` 啟用時可見並可點擊開啟 SECS/GEM Log 視窗 |
| `pnlFeatureBadge2` | led | `SAFE : NORMAL` | 安全狀態徽章，名稱固定 SAFE，預設 NORMAL(綠) |
| `pnlFeatureBadge3` | led | `AMR : ON / OFF` | AMR/AGV 模式徽章。`UpdateAmrFeatureBadge` 依 `GeneralSetting.bUseAMR` 顯示 ON(綠)/OFF(灰) |

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `GeneralSetting.bUseAMR` | true/false | AMR/AGV 模式啟用旗標，驅動 AMR 徽章 ON/OFF（此畫面唯讀顯示） |
| `CosFunction.bUseSecsGem` | true/false | SECS/GEM 付費功能旗標，決定 SECS 徽章是否顯示與可點擊（此畫面唯讀） |

> 【待補：SECS 徽章狀態碼 (0/1/2) 對應的實際 HSMS 連線語意，來源在 SECS 引擎，本章僅做顯示對應。】

---

## 3.5 Recipe、User 與計數區

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `cb_WorkFile` | combobox | 選擇/顯示目前配方名稱 (`Recipe Name`)。有效則 `SetCurrentRecipeName`、存檔、重載 TrayForm 與各子畫面 WorkFile、刷新監看格線並記錄；空白/不存在/運轉中則還原並提示 |
| `cbbUserSelect` | combobox | 切換操作者權限等級，選項 `Operation / Supervisor / Engineer / Honprec`。Operation 直接降權；其餘等級在 `SOFT_SIMULATE` 下強制切換，真機則提示尚未支援密碼登入並還原 |
| `palloadingCount` | label | `Load` 上料計數值面板（預設 0） |
| `palUnloadingCount` | label | `Total` 總下料計數值面板 |
| `palloseCnt` | label | `Fail` 不良/遺失計數（`lblloseCnt` Visible=False，目前隱藏） |
| `btnClearCount` | button | `Clear All` 清除計數按鈕 |
| `sbPaperSummary` | button | `Summary` 產量彙總按鈕 |
| `palAuto01..06` + `plLotNumberAuto1..6` | grid | 下料 Auto1~6 即時資訊面板（Bin / Lot / ID / Cnt），由 `ShowUnloadAutoInfo` 填入 |

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| Recipe Name | 配方清單中存在的名稱 | 由 `cb_WorkFile` 選擇，變更後重載 Tray 幾何與各子畫面 |
| User Role | Operation / Supervisor / Engineer / Honprec，預設 Operation | 操作者權限等級，由 `cbbUserSelect` 選擇 |

> ⚠️ 注意：運轉中無法變更 Recipe（提示 `Can not change recipe while machine is running.`），須先停機。

> 【待補：`btnClearCount` (Clear All) 與 `sbPaperSummary` (Summary) 在 main.dfm 中未綁定 OnClick，main.h 亦無對應處理函式宣告，實際功能/是否啟用待現場確認（疑為待接線或保留按鈕）。】

---

## 3.6 全機歸原 (HOME) 監看畫面

按下 `sbHome1`（或由 `Start` 觸發歸原）後，顯示 Motor Home Monitor (`fHome`)。此畫面為**顯示專用**，關閉/中止的決策由核心 `ProcessHomeLifecycle` 掌控（執行於 `ProcessMotion` 最前、`SystemStart` gate 之前）。

![HOME 復歸監看畫面](screenshots/screen-home.png)
> 圖 3-2 HOME 復歸監看畫面。（擷取方式：在 MonitorView 按 `Home` 並於 `Confirm home?` 按 Yes，歸原期間即顯示此畫面）

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `fHome.Panel1_grid` | grid | 動態建立的馬達格（`BuildMotorGrid`）：每軸一列含軸名 (`NumberAlias`) + TALed 原點燈（綠=已歸原 / 黃=歸原中 / 紅=警報 / 灰=未使用）+ 即時位置 `EditMotorPos`；每 100ms 由 `Timer1` 刷新 |
| `fHome.lstHomeMsg` | grid | 底部訊息框，顯示 `Starting home procedure....`、`Motor power cycle...`、`Home finished.` 等進度訊息 |
| `fHome.SpeedButton1` | button | `Abort Home` 中止歸原：`StopAllMotor()`、`fAllMotorHome=false`、`bHomePowerCycling=false`、`SoftStop=true`、關閉視窗；機台仍須重新成功歸原才能運轉 |

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `iHomeStep` (fHome) | 預設 1 | 歸原引擎內部步序狀態（1/10/20/2/50/60/100/200）；FormClose 重置為 1 |
| `Timer1.Interval` (fHome) | 100 ms | HOME 監控刷新週期（顯示專用） |

### 歸原動作序（`ProcessMotorHome`）

歸原引擎依序執行：升 TrayArm Z 上缸並開夾爪（持盤時保持夾緊不掉盤）→ 釋放 Loader 後鉤 → 前擋 → 批次回 4 支吸嘴 Z → 確認 Z 上升後批次回所有 XY 軸 → 完成。

> ⚠️ 注意：批次回 XY 前必先升 TrayArm Z 上缸並確認，避免撞擊下方盤；Loader 兩車共軌，歸原前先放後鉤再放前擋以避免拖盤對撞。

歸原完成後 `fAllMotorHome=true`，切回 `Run_Normal` 並重套工作速度：
- 由 `Home` 按鈕觸發（`bHomeByStart=false`）：完成後停機待命，監控畫面顯示 `Home finished.` 並自動關閉。
- 由 `Start` 觸發（`bHomeByStart=true`）：完成後自動接續生產，不停機。

---

## 3.7 首次開機到開始生產（標準流程）

1. **程式啟動**：背景執行緒 `TRunControl` 以約 1ms 週期呼叫 `MainProc()`。首次經 `DoInitialProgramStart` 載入馬達速度基準、啟動模組 Timer，狀態列顯示 `INIT`。
2. **馬達電源**：若 Pad 操作面板尚未通訊 (`bPadEverCommunicated==false`)，軟體啟動時自動 `SwMotorRelay.On` 上電；面板通訊後改由前/後面板 Power On/Off 鍵控制 (`CheckMotorPowerShutDown`)。
3. **登入**：於 `cbbUserSelect` 選擇權限。Operation 直接生效；Supervisor/Engineer/Honprec 在真機提示密碼登入尚未提供（僅模擬可強制套用）。權限變更會記錄並 SECS 回報 `ChangeUser`。
4. **設定模式**：必要時以 `pnRealDummy` 切 Real/HasTray/Dummy、以 `pnStartMode` 切 Initial/Continue（兩者皆僅 `SystemStart==false` 可改）。
5. **選擇配方**：於 `Recipe Name` (`cb_WorkFile`) 選好作業配方。
6. **建立 Lot**：於 Lot 頁輸入 `Lot No.` (`edLotNo`)，以 `Add Lot` / `Lot Start` 建立 Lot 並載入 2D/Bin 資料。
7. **全機歸原**：按 `Home`，於 `Confirm home?` 按 Yes，觀看 Motor Home Monitor，待各軸原點燈轉綠、顯示 `Home finished.` 後自動關閉。
8. **開始生產**：切到 MonitorView 頁，按 `Start`。系統以 `CheckLotDataReady` 驗證 LotID / Lot 數 / 2D 資料；若機台尚未歸原，`Start` 會先歸原並於完成後自動續跑生產。
9. **運轉中監控**：每週期 `ScanAllMotorStatus` + `ScanSystemSenser` 監控故障；`DoSystemMessage` 更新三色塔燈/蜂鳴器與面板燈；模組推動生產。

---

## 3.8 暫停、單循環、清機與快照

### 暫停 / 停止

1. **暫停**：按 `Pause`（或前/後面板 Pause 鍵）→ `SystemStart=false` + `SoftStop=true`，下一週期減速停止；保留歸原狀態，再按 `Start` 可續跑。
2. **指令層 MachinePause**：記錄 `MACHINE PAUSE by <trigger>` 後 `SystemStart=false`、`DecStopAllMotor`、`SoftStop=true`（優雅減速）。
3. **指令層 MachineStop**：硬停——`SystemStart=false`、`StopAllMotor`（不減速）、`SoftStop=true`，用於 EMG/故障。

> ⚠️ 注意：`SystemStart` 下降緣會啟動暫停碼錶並凍結 cylinder/sucker 逾時窗 (`PauseActuatorTimeoutTimers`)，恢復緣再解凍累加，避免暫停時間誤算入 UPH 或誤報逾時。

### 單循環 / 清機

1. **單循環**：於 `Run_Normal` / `Run_CleanOut` 模式按 `One Cycle`（同樣須通過 Lot/2D 資料檢查）。
2. **清機**：於 `Run_Normal` 模式按 `Clean Out`，將機台內殘料排出；完成提示 `CleanOut finish note`，操作員選 SKIP 結束回 Normal 並停機。

### 擷取機台狀態快照 (Hang up)

1. 按 `sbStoreHangup`。
2. 等待 `gStateRecord` 打包 zip。
3. 成功後檔總管自動開啟並選取 zip（位於 `D:\HT160S_StateRecord\`）；失敗會提示檢查 7-Zip / 磁碟。

---

## 3.9 啟動相關提示訊息

下表為本章操作可能遇到的提示訊息（含義與排除方式）。完整故障/警報處理請參閱第 13 章。

| 提示訊息 | 含義 | 排除方式 |
| --- | --- | --- |
| `Please Enter LotID !` | 啟動前未輸入 Lot 編號 | 於 `Lot No.` (`edLotNo`) 輸入有效 Lot 編號 |
| `No Lot data : add at least one Lot before Start !` | `LotRegistry` 無任何 Lot | 先 `Add Lot` / `Lot Start` 建立 Lot |
| `Recipe name cannot be empty.` | Recipe Name 下拉留空 | 還原為目前配方；選擇有效配方 |
| `Can not change recipe while machine is running.` | 運轉中嘗試變更配方 | 先停機再變更 |
| `Recipe does not exist.` | 輸入的配方名稱不存在 | 選擇清單中既有配方 |
| `One Cycle is only allowed in Normal / Clean Out mode.` | 非 Normal/Clean Out 模式按 One Cycle | 切回 Normal/Clean Out 模式再執行 |
| `State Record snapshot failed (check 7-Zip / disk).` | 手動快照打包失敗 | 檢查 7-Zip 是否安裝、磁碟空間/路徑 `D:\HT160S_StateRecord\` |
| `User password login is not available yet.` | 真機切換非 Operation 權限尚未支援密碼登入 | 維持 Operation 等級（功能待實作） |
| `Please add at least one Lot to the list !` | Lot Start 時清單為空 | 先用 `Add Lot` 加入 Lot |

> 【待補：原始碼中 `cbbUserSelect` 選項、`pnStartMode` / `pnRealDummy` 的實際螢幕中文標籤因 Big5 編碼未直接讀出，UI 文字以程式設定的英文 Caption 為準（Initial/Continue/Real/Dummy）。】
