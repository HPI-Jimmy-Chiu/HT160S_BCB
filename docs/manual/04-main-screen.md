# 第 04 章　主畫面詳解

HT160S 開機後常駐於最上層的全螢幕視窗為主畫面。本章逐一說明主畫面下方分頁區的六個操作分頁：Main、Lot、Tray Status、Logs、Time Data、Map Tray，並完整記錄生產計數區（Total / Fail / Summary 與 Unload Auto1~6）、Recipe Name 與 User 欄位的內容與操作方式。

> 說明：主畫面最外層另有一個上層分頁，將整個畫面切分為「Main 主操作頁」與「MonitorView 監看頁」。MonitorView 的運轉控制（Home / Start / Pause / One Cycle / Clean Out）與即時監看分頁（Motion / Motor / Record / Other）屬運轉操作範疇，本章僅在生產計數與按鈕關聯處提及，詳細操作另見運轉相關章節。本章聚焦於下方分頁區的六個資訊／設定分頁與計數欄位。

---

## 4.1 Main 分頁

Main 分頁是 HT160S 的主操作頁面，集中了頂部功能列、生產計數區、Recipe Name 與 User 選擇、Real/Dummy 與 Start Mode 切換、SECS/SAFE/AMR 狀態徽章，以及三色塔燈與機台狀態文字。

![Main 分頁](screenshots/main-overview.png)
> 圖 4-1 Main 主操作分頁。（擷取方式：開機進入主畫面後，預設即停在此分頁。）

### 4.1.1 頂部功能列

頂部功能列為一排功能按鈕，用於開啟各設定／工具子畫面或切換監看頁。

| 畫面項目 | 類型 | 功能 |
|---|---|---|
| Language | 按鈕 | 開啟語言設定畫面。 |
| Product | 按鈕 | 開啟產品／配方設定畫面。 |
| Maintance | 按鈕 | 開啟維護畫面。 |
| Offset | 按鈕 | 開啟 Offset 偏移補正畫面。 |
| Speed | 按鈕 | 開啟速度設定畫面。 |
| Tools | 按鈕 | 開啟系統工具畫面。 |
| Message | 按鈕 | 開啟訊息／警報視窗。 |
| Monitor | 按鈕 | 切換至 MonitorView 監看分頁。 |
| Exit | 按鈕 | 關閉主視窗、離開程式。 |

> 註：頂部功能列實際螢幕文字已由截圖（`screenshots/main-overview.png`，模擬程式擷取、與實機 UI 相同）確認，由左至右為：**Language / Product / Maintance / Offset / Speed / Tools / Message / Monitor / Exit**（各鈕為圖示＋文字；「Maintance」為機上實際拼字）。

### 4.1.2 Recipe Name 與 User 欄位

| 畫面項目 | 類型 | 功能 |
|---|---|---|
| Recipe Name 配方切換下拉 | 下拉 | 選擇／顯示目前配方名稱（標籤為 Recipe Name）。變更時：空白、不存在或運轉中則還原並提示；有效則切換配方、存檔、重載 Tray 幾何與各子畫面（Setup / Teach / Maintenance / Offset）的配方、刷新監看格線並記錄。展開時會重整清單。 |
| User 權限下拉 | 下拉 | 切換操作者權限等級（標籤為 User），選項 Operation / Supervisor / Engineer / Honprec。Operation 直接降權免密碼；其餘等級真機會跳出 Login User ID / Login Password 輸入框，帳號驗證成功才切換（模擬版直接強制切換）。 |

> ⚠️ 注意：Recipe Name 在運轉中不可變更（提示「Can not change recipe while machine is running.」）；請先停機再變更配方。

> ⚠️ 注意：真機切換非 Operation 權限需輸入使用者 ID 與密碼；輸錯提示「User ID or password is incorrect.」並還原原等級。帳號管理於 Maintenance 帳號頁（需 Engineer 以上權限）。

### 4.1.3 生產計數區

生產計數區顯示上料、總下料與不良計數，並提供清除與彙總按鈕。

| 畫面項目 | 類型 | 功能 |
|---|---|---|
| Load 上料計數 | 顯示 | 上料計數標籤（Load）與數值，預設 0。 |
| Total 總下料計數 | 顯示 | 總下料計數標籤（Total）與數值。 |
| Fail 計數 | 顯示 | 不良／遺失計數標籤（Fail）；目前隱藏。 |
| Clear All 清除計數鈕 | 按鈕 | 停機時可用：按下先跳 YES/NO 確認，確認後清除本 Lot 生產計數（IC／Bin／Tray／UPH／Loader／Jam＋SECS 統計）、寫回存檔並清空 Product Info 面板；運轉中按下會提示先停機。 |
| Summary 彙總鈕 | 按鈕 | 產量彙總按鈕（Summary）。目前未綁定，為**保留鈕（未啟用）**。 |
| UPH／產能統計表 | 表格 | 主畫面 UPH／產能統計表格（顯示用）。 |

> 註：`btnClearCount`（Clear All）已於 2026-07 接上功能（見上表）；`sbPaperSummary`（Summary）經原始碼確認未綁定 `OnClick` 且無處理函式，為保留鈕。Fail 計數標籤 `lblloseCnt` 的 DFM `Visible=False`，目前隱藏不顯示（程式亦無寫入點）。

#### Unload Auto1~6 下料即時資訊

主畫面以六組面板顯示下料 Auto1~Auto6 的即時資訊。

| 畫面項目 | 類型 | 功能 |
|---|---|---|
| 下料 Auto1~6 即時資訊面板 | 表格 | Unload Auto1~Auto6 即時資訊（Bin / Lot / ID / Cnt）。Bin：By Lot+Bin 模式取綁定，否則取靜態 Bin→Auto 對應；Lot：Lot 號；ID：目前工作 Tray ID；Cnt：IC 計數。 |

### 4.1.4 Real/Dummy 與 Start Mode 切換

| 畫面項目 | 類型 | 功能 |
|---|---|---|
| Real/Dummy 切換框 | 按鈕 | 點擊循環切換運轉模式（Dummy → HasTray → Real → 回 Dummy）。更新圖示／文字、存檔並記錄。 |
| Real/Dummy 指示圖示 | 圖示 | 與切換框共用點擊；依目前模式顯示 Real 或 Dummy 圖示。 |
| 「Real/Dummy」區塊標題 | 標籤 | Real/Dummy 區塊標題。 |
| Start Mode 切換框 | 按鈕 | 點擊切換起動模式（Initial 初始起動 ↔ Continue 續做）。更新圖示／文字、存檔並記錄。 |
| Start Mode 指示圖示 | 圖示 | 與切換框共用點擊；依目前模式顯示 Initial 或 Continue 圖示。 |
| 「Start Mode」區塊標題 | 標籤 | Start Mode 區塊標題。 |

> ⚠️ 注意：Real/Dummy 與 Start Mode 僅在停機時可變更；運轉中點擊直接忽略。

### 4.1.5 狀態徽章與三色塔燈

| 畫面項目 | 類型 | 功能 |
|---|---|---|
| SECS 連線徽章 | 顯示 | SECS/GEM 連線狀態徽章。依連線狀態顯示 OFF(灰) / CONNECT(橄欖) / ONLINE(綠)；僅在 SECS/GEM 啟用時可見並可點擊開啟 SECS/GEM Log 視窗。 |
| SAFE 安全徽章 | 顯示 | 安全門／安全狀態徽章，預設 NORMAL(綠)，名稱固定 SAFE。 |
| AMR 徽章 | 顯示 | AMR/AGV 模式徽章。依 AMR 模式設定顯示 ON(綠) / OFF(灰)。 |
| 三色塔燈 | 顯示 | 三色塔燈，鏡射實體塔燈；依機台運轉狀態與設定亮燈，同時驅動實體塔燈與蜂鳴器。 |
| 機台狀態文字 | 顯示 | 機台狀態文字與顏色；狀態值見下方說明（HALT / INIT / HOMING / RUNNING / Clean Out / Tray Feed / One Cycle / LOCK / EMG / MOTOR OFF / SAFE DOOR / AIR / PAUSE）。 |

機台狀態文字為即時顯示，非彈窗，依下列邏輯設定：

- 程式未啟動 = INIT
- 已下 Start 且未歸原 = HOMING（綠）
- 清機模式 = Clean Out（黃）；供盤模式 = Tray Feed（黃）；單循環 = One Cycle（黃）；其餘運轉中 = RUNNING（綠）
- 停機時依序判定：安全鎖啟動=LOCK、緊急停止=EMG、馬達電源關閉=MOTOR OFF、安全門開啟=SAFE DOOR、氣壓不足=AIR、機台內仍有 IC=PAUSE，預設 HALT

> ⚠️ 注意：LOCK / EMG / MOTOR OFF / SAFE DOOR / AIR 等狀態為安全／電源／氣壓條件的顯示結果；排除對應條件後狀態會自行恢復。

> 註：`palMainStatus_En`（英文狀態面板）DFM `Visible=False` 預設隱藏；`SetMainStatus`（csystem.cpp）會同步寫入相同狀態文字，屬鏡像面板。SECS 徽章狀態 0/1/2 = HSMS OFF／CONNECT／ONLINE，由 `UpdateSecsFeatureBadge` 依 SECS 引擎連線狀態設定（詳見第 12 章）。

### 4.1.6 開始生產（Start）操作步驟

1. 於 Main 分頁確認 Real/Dummy 與 Start Mode（點對應切換框切換，需停機狀態）。
2. 在 Recipe Name 下拉選好配方。
3. 於 Lot 分頁輸入 Lot No. 並用 Add Lot / Lot Start 建立 Lot 與載入 2D/Bin 資料（見 4.2 節）。
4. 切到 MonitorView 頁，按 Start。
5. 系統檢查 LotID 與 Lot/2D 資料；若未歸原會先進入 Home 並於完成後自動接續生產。

> ⚠️ 注意：未輸入 Lot 或無 2D/Bin 資料會跳出提示並中止；運轉中無法更改 Recipe。Start 與 One Cycle 必須通過 Lot 資料檢查：LotID 不可空、至少一筆 Lot 且至少一筆 2D/Bin 記錄。

---

## 4.2 Lot 分頁

Lot 分頁用於管理生產批次（Lot），包含 Lot 編號輸入、Lot 清單、手動新增／編輯／移除，以及產品資訊表格。透過主畫面下方的分頁按鈕切換至此分頁。

![Lot 分頁](screenshots/main-lot.png)
> 圖 4-2 Lot 分頁。（擷取方式：於主畫面下方 log-menu 點選對應按鈕切換至 Lot 分頁。）

| 畫面項目 | 類型 | 功能 |
|---|---|---|
| Lot No. 輸入框 | 輸入 | Lot 編號輸入框（Lot No.）；為 Start / One Cycle 的必要條件（空白則被擋下並提示 Please Enter LotID）。 |
| Lot 清單 | 表格 | 手動 Lot 清單；雙擊列顯示該 Lot 的 2D 明細。提示：Tip: double-click a lot row to view its 2D detail. |
| Add Lot | 按鈕 | 在 Lot 手動編輯區新增 Lot 清單列（Add Lot）。 |
| Edit Lot | 按鈕 | 編輯 Lot 清單列（Edit Lot）。 |
| Remove Lot | 按鈕 | 移除 Lot 清單列（Remove Lot）。 |
| Lot Start | 按鈕 | 將清單 Lot 納入作用中 Lot 名冊，清除動態綁定並存檔，設機台工單為執行中、記錄作用中 Lot 數，啟動分類工單。 |
| Lot End | 按鈕 | 結束目前 Lot 工單。 |
| 產品資訊表（Product Info） | 表格 | 產品資訊表格（顯示用）。 |

### 4.2.1 建立 Lot 工單操作步驟

1. 於 Lot No. 輸入框輸入有效 Lot 編號。
2. 按 Add Lot 將該 Lot 加入清單；必要時用 Edit Lot / Remove Lot 修改清單。
3. 按 Lot Start 將清單 Lot 納入作用中名冊並啟動分類工單。
4. 如需查看某 Lot 的 2D 明細，雙擊清單列。
5. 工單結束時按 Lot End。

> ⚠️ 注意：Lot Start 時清單為空會提示「Please add at least one Lot to the list !」；請先用 Add Lot 加入 Lot。

---

## 4.3 Tray Status 分頁

Tray Status 分頁以視覺化方式呈現各區盤位狀態，包含 Loader 左右側與分類／工作區的盤位排版。由主畫面下方的 Tray Status 按鈕切換。

![Tray Status 分頁](screenshots/main-traystatus.png)
> 圖 4-3 Tray Status 分頁。（擷取方式：於主畫面下方 log-menu 點選 `spbTrayStatus` 按鈕切換至此分頁。）

本分頁含「Loader 2D Left」與「Loader 2D Right」兩組盤位面板（各含盤面格點）。

> 註（定案）：HT160S **沒有** HT172 的 `mtSortRecv`/`mtWorkArea` 元件。盤面顯示分兩層：(a) 本分頁 `mtLoaderL`/`mtLoaderR` 以 `SetSubHTrayPanel` **鏡射該 Loader 車道的生產盤內容**（Left=Loader1／`MMLoaderY_1`、Right=Loader2／`MMLoaderY_2`）；(b) Motion View 的 `mtLoaderLTrayWork`/`mtLoaderRTrayWork` 由 `BindMovingTrayPanel` 綁定（位置取實體馬達、內容取虛擬馬達），顯示**移動中盤面**。格數（預設 4×5）執行期由配方覆寫（`SyncMonitorTrayDivision`）。

> 註：`spbTrayStatus` 按鈕文字已確認為「Tray Status」（DFM Caption＋截圖 `main-overview.png`，圖示＋文字並列）；DFM Hint「Change Language」為複製貼上遺留，非實際功能。`OnClick` 切換至 `tsTrayStatus`。

---

## 4.4 Logs 分頁

Logs 分頁顯示系統日誌清單。由主畫面下方的 Logs 按鈕切換。

![Logs 分頁](screenshots/main-logs.png)
> 圖 4-4 Logs 分頁。（擷取方式：於主畫面下方 log-menu 點選 `apbLogs` 按鈕切換至此分頁。）

| 畫面項目 | 類型 | 功能 |
|---|---|---|
| 日誌清單 | 顯示 | 日誌列表。 |

> 註：`apbLogs` 按鈕文字已確認為「Logs」（DFM Caption＋截圖）；Hint「Change Language」為複製遺留。`OnClick` 切換至 `tsLogs`。

---

## 4.5 Time Data 分頁

Time Data 分頁顯示時間統計表格並提供儲存。由主畫面下方的 Time Data 按鈕切換。

![Time Data 分頁](screenshots/main-timedata.png)
> 圖 4-5 Time Data 分頁。（擷取方式：於主畫面下方 log-menu 點選 `sbTimeData` 按鈕切換至此分頁。）

| 畫面項目 | 類型 | 功能 |
|---|---|---|
| 時間資料表與存檔鈕 | 表格 | 時間資料統計表格與存檔面板。 |

> 註：`sbTimeData` 按鈕文字已確認為「Time Data」（DFM Caption＋截圖）；Hint「Change Language」為複製遺留。`OnClick` 切換至 `tsTimeData`。

---

## 4.6 Map Tray 分頁

Map Tray 分頁以文字方式顯示盤位配置圖。由主畫面下方的 Map Tray 按鈕切換。

![Map Tray 分頁](screenshots/main-maptray.png)
> 圖 4-6 Map Tray 分頁。（擷取方式：於主畫面下方 log-menu 點選 `btnTrayMap` 按鈕切換至此分頁。）

| 畫面項目 | 類型 | 功能 |
|---|---|---|
| Tray 配置圖顯示區 | 顯示 | Tray 配置圖文字顯示。 |

> 註：`btnTrayMap` 按鈕文字已確認為「Map Tray」（DFM Caption＋截圖）；Hint「Change Language」為複製遺留。`OnClick` 切換至 `tsMapTray`。

---

## 4.7 補充：相關設定參數

下表整理本章畫面所涉及的設定參數，供查閱。

| 設定項目 | 範圍/預設 | 說明 |
|---|---|---|
| 運轉模式（Real/Dummy） | Dummy / HasTray / Real | 運轉模式，影響三層 IO／感測檢查層級；由 Real/Dummy 切換框循環切換並存檔。 |
| 起動模式（Start Mode） | Initial / Continue | 起動模式（初始起動 / 續做）；由 Start Mode 切換框切換並存檔。 |
| Recipe Name | 配方清單中存在的名稱 | 由 Recipe Name 下拉選擇的作業配方名稱，變更後重載 Tray 幾何與各子畫面。 |
| User Role（使用者權限） | Operation / Supervisor / Engineer / Honprec | 操作者權限等級，由 User 下拉選擇。 |
| AMR 模式 | 開 / 關 | AMR/AGV 模式啟用旗標，驅動 AMR 徽章 ON/OFF（此畫面唯讀顯示）。 |
| SECS/GEM | 開 / 關 | SECS/GEM 付費功能旗標，決定 SECS 徽章是否顯示與可點擊（此畫面唯讀）。 |

> 說明：模擬相關參數與控制項（模擬啟用勾選、載入模擬資料、每站滿盤門檻表等）位於 Simulation 分頁，屬模擬／測試用途，將於模擬相關章節說明。

---

## 4.8 主畫面常見提示訊息

| 訊息 | 含義 | 排除方式 |
|---|---|---|
| Please Enter LotID ! | 啟動前未輸入 Lot 編號 | 於 Lot No. 輸入有效 Lot 編號 |
| No Lot data : add at least one Lot before Start ! | Lot 清單無任何 Lot | 先 Add Lot / Lot Start 建立 Lot |
| Recipe name cannot be empty. | Recipe Name 下拉留空 | 還原為目前配方；請選擇有效配方 |
| Can not change recipe while machine is running. | 運轉中嘗試變更配方 | 先停機再變更 |
| Recipe does not exist. | 輸入的配方名稱不存在 | 選擇清單中既有配方 |
| One Cycle is only allowed in Normal / Clean Out mode. | 非 Normal/Clean Out 模式下按 One Cycle | 切回 Normal/Clean Out 模式再執行 |
| State Record snapshot failed (check 7-Zip / disk). | 手動快照打包失敗 | 檢查 7-Zip 是否安裝、磁碟空間／路徑 `D:\HT160S_StateRecord\` |
| User password login is not available yet. | 真機切換非 Operation 權限尚未支援密碼登入 | 維持 Operation 等級（功能待實作） |
| Please add at least one Lot to the list ! | Lot Start 時清單為空 | 先用 Add Lot 加入 Lot |
