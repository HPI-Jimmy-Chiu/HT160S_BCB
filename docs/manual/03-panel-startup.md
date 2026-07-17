# 第 03 章　操作面板與開機啟動

本章說明 HT160S 主畫面的操作面板配置，以及從「首次開機」到「開始生產」的完整流程。內容涵蓋頂部工具列、Real/Dummy 與 Start Mode 切換、各狀態徽章 (SECS / SAFE / AMR) 與三色塔燈、全機歸原 (HOME) 監看畫面，以及 START / PAUSE / STOP / One Cycle / Clean Out 等運轉控制。發生故障時的處理與警示彈窗，請參閱第 13 章。

---

## 3.1 主畫面總覽

主畫面是開機後常駐的最上層全螢幕視窗，分成 **Main 主操作頁** 與 **MonitorView 監看頁**：

- **Main 頁**：頂部功能列、生產計數區、Recipe Name 與 User 選擇、Real/Dummy 與 Start Mode 切換、SECS/SAFE/AMR 狀態徽章、三色塔燈與機台狀態文字。
- **MonitorView 頁**：Home / Start / Pause / One Cycle / Clean Out / Hangup 等運轉控制按鈕，以及 Motion / Motor / Record / Other 等即時監看分頁。

畫面同時承載 Lot 管理、Tray Status、Logs、Time Data、Map of Tray、Simulation 等子分頁。

![主畫面與工具列](screenshots/main-overview.png)
> 圖 3-1 主畫面與工具列。（擷取方式：程式啟動後預設即停留於此畫面；若在 MonitorView，按 `btnMainShow` 回主畫面）

### 頂部工具列（Main 頁）

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| Language | 按鈕 | 開啟語言設定畫面 |
| Product | 按鈕 | 開啟產品/配方設定畫面 |
| Maintance | 按鈕 | 開啟維護畫面 |
| Offset | 按鈕 | 開啟 Offset 偏移補正畫面 |
| Speed | 按鈕 | 開啟速度設定畫面 |
| Tools | 按鈕 | 開啟系統工具畫面 |
| Message | 按鈕 | 開啟訊息/警報視窗 |
| Monitor | 按鈕 | 切換至 MonitorView 監看分頁 |
| Exit | 按鈕 | 關閉主視窗、離開程式 |

### MonitorView 頁分頁切換

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| Return | 按鈕 | 從 MonitorView 切回 Main 分頁 |
| Record | 按鈕 | 切換至 Record 分頁 |
| Motor View | 按鈕 | 切換至 Motor View 馬達狀態分頁 |
| Motion View | 按鈕 | 切換至 Motion View 動作模擬分頁 |
| Other | 按鈕 | 切換至 Other 分頁，含 Lock/IO/Suck 狀態 |

> 註：頂部功能列實際螢幕文字已由截圖（`screenshots/main-overview.png`）與 DFM Caption 確認：**Language / Product / Maintance / Offset / Speed / Tools / Message / Monitor / Exit**（圖示＋文字；「Maintance」為機上實際拼字）。DFM Hint「Change Language」為複製貼上遺留，非按鈕功能。

---

## 3.2 運轉控制按鈕

以下按鈕位於 MonitorView 頁，是日常運轉的核心操作。

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| Home | 按鈕 | 啟動全機歸原。非模擬時先彈出「Confirm home?」；確認後進入歸原監看畫面，完成後停機待命 |
| Start | 按鈕 | 啟動生產。先檢查 LotID 與 2D/Bin 資料，通過後開始逐顆生產；若機台未歸原則先自動歸原，完成後自動接續生產 |
| Pause | 按鈕 | 暫停運轉。觸發減速停止；保留歸原狀態，可再按 Start 續跑 |
| One Cycle | 按鈕 | 單循環。僅允許在 Normal / Clean Out 模式且 Lot 資料就緒；完成放料後回 Normal 並停機 |
| Clean Out | 按鈕 | 清機。僅在 Normal 模式生效，排出機台內殘料 |
| Store Hangup | 按鈕 | 手動觸發機台狀態快照，打包狀態資料成 zip 至 `D:\HT160S_StateRecord\`，成功後以檔案總管選取該 zip |

> ⚠️ 注意：Start 與 One Cycle 必須通過 Lot 資料檢查——LotID 不可空白、至少一筆 Lot、且至少一筆 2D/Bin 記錄；若為 By Lot+Bin 模式還須有綁定，否則中止啟動並彈出提示。

---

## 3.3 Real/Dummy 與 Start Mode 切換

主畫面設定區塊提供兩個運轉前置設定，兩者**僅在停機狀態可變更**，運轉中點擊直接忽略。

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| Real/Dummy 切換框 | 按鈕 | 點擊循環切換運轉模式：Dummy → HasTray → Real → 回 Dummy；更新圖示/文字並存檔記錄 |
| Real/Dummy 指示圖示 | 圖示 | 與切換框共用點擊；依目前模式顯示 Real 或 Dummy 圖示 |
| Start Mode 切換框 | 按鈕 | 點擊切換起動模式（Initial 初始起動 ↔ Continue 續做）；更新圖示/文字並存檔記錄 |
| Start Mode 指示圖示 | 圖示 | 與切換框共用點擊；依目前模式顯示 Initial 或 Continue 圖示 |
| 「Real/Dummy」區塊標題 | 標籤 | Real/Dummy 區塊標題 |
| 「Start Mode」區塊標題 | 標籤 | Start Mode 區塊標題 |

### 設定值

| 設定項目 | 範圍/預設 | 說明 |
| --- | --- | --- |
| 運轉模式（Real/Dummy） | Dummy / HasTray / Real，預設 Dummy | 運轉模式，影響三層 IO/感測檢查層級。離子風扇警報等實機檢查僅 Real 模式生效 |
| 起動模式（Start Mode） | Initial / Continue，預設 Initial | 起動模式 |

> 註（定案）：`iStartMode=Continue` **目前對執行流程無任何行為分支**——全原始碼追查結果：`Start()`／`DoStartArm()`／`ProcessStartMode()` 均不讀取 `iStartMode`，`CheckContinusStartIsReady()` 只剩被註解的呼叫（main.cpp:1968）且函式定義已不存在，SECS 端亦無鏡像；唯一對外出口是 Automation TCP `GET_STATUS` 的 `START_MODE=` 欄位。Initial/Continue 差異僅止於畫面圖示、ini 記錄與操作紀錄（RecordProcess）。

> 註（定案）：DUMMY / HAS_TRAY / REALLY 三模式差異——
> - **DUMMY**：料流全虛擬。源乾以勾選框判斷、盤在席/遺失感測檢查全跳過、氣缸到位感測不確認（馬達/氣缸仍實際動作）。
> - **HAS_TRAY**：走真實 IO——氣缸到位確認與 Empty/Color/Loader 的盤在席/遺失警報全部生效，但**真空吸嘴檢查跳過、Top CCD 2D 走模擬循環碼**（「真的搬盤，但不驗 IC 真值」）。
> - **REALLY**：全真——真空感測故障會報警、Top CCD 實體觸發＋輪詢、離子風扇連鎖生效。
> 詳見第 10 章 IO 自我測試（`iosetview.cpp` SelfTestTier 註解為此三層之權威定義）。

---

## 3.4 狀態徽章與三色塔燈

### 機台狀態文字

主畫面顯示一個即時機台狀態文字，同步對應紅/綠/黃色：

| 狀態 | 觸發條件（節錄） |
| --- | --- |
| INIT | 程式尚未啟動 |
| HOMING（綠） | 已下 Start 且尚未歸原 |
| Clean Out（黃） | 清機模式 |
| Tray Feed（黃） | 供盤模式 |
| One Cycle（黃） | 單循環模式 |
| RUNNING（綠） | 其餘運轉中狀態 |
| LOCK | 停機且安全鎖啟動 |
| EMG | 停機且緊急停止被按下 |
| MOTOR OFF | 停機且馬達電源關閉 |
| SAFE DOOR | 停機且安全門開啟 |
| AIR | 停機且氣壓不足 |
| PAUSE | 停機且機台內仍有 IC |
| HALT | 以上皆非的預設停機狀態 |

> ⚠️ 注意：LOCK / EMG / MOTOR OFF / SAFE DOOR / AIR 是安全/電源/氣壓條件的**顯示結果**，並非由主畫面直接控制的閉鎖。需排除對應條件後，狀態才會自行恢復；故障停機的處理與彈窗，詳見第 13 章。

### 三色塔燈

三色塔燈內含紅/黃/綠三顆燈號，鏡射實體塔燈；依機台運轉狀態與設定亮燈，並同步驅動實體塔燈與蜂鳴器。

### 功能徽章

| 畫面項目 | 類型 | 標籤/狀態 | 功能 |
| --- | --- | --- | --- |
| SECS 連線徽章 | 顯示 | SECS：OFF / CONNECT / ONLINE | SECS/GEM 連線徽章。依連線狀態顯示 OFF(灰)/CONNECT(橄欖)/ONLINE(綠)；僅在 SECS/GEM 啟用時可見並可點擊開啟 SECS/GEM Log 視窗 |
| SAFE 安全徽章 | 顯示 | SAFE：NORMAL | 安全狀態徽章，名稱固定 SAFE，預設 NORMAL(綠) |
| AMR 徽章 | 顯示 | AMR：ON / OFF | AMR/AGV 模式徽章。依 AMR 模式設定顯示 ON(綠)/OFF(灰) |

| 設定項目 | 範圍/預設 | 說明 |
| --- | --- | --- |
| AMR 模式 | 開 / 關 | AMR/AGV 模式啟用旗標，驅動 AMR 徽章 ON/OFF（此畫面唯讀顯示） |
| SECS/GEM | 開 / 關 | SECS/GEM 付費功能旗標，決定 SECS 徽章是否顯示與可點擊（此畫面唯讀） |

> 註：SECS 徽章狀態碼定義（`UpdateSecsFeatureBadge`，main.cpp）：0=**OFF**（未連線，灰）、1=**CONNECT**（TCP 已連但未 SELECTED，橄欖）、2=**ONLINE**（HSMS SELECTED，綠）；根源為 SECS 引擎 `iHsmsState`（`uHGemEquipment.cpp`），每秒刷新。

---

## 3.5 Recipe、User 與計數區

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| Recipe Name 配方切換下拉 | 下拉 | 選擇/顯示目前配方名稱（Recipe Name）。有效則切換配方、存檔並重載相關子畫面；空白/不存在/運轉中則還原並提示 |
| User 權限下拉 | 下拉 | 切換操作者權限等級，選項 Operation / Supervisor / Engineer / Honprec。Operation 直接降權免密碼；其餘等級真機會跳出 **Login User ID / Login Password** 輸入框，帳號驗證成功才切換，輸錯提示「User ID or password is incorrect.」（模擬版則直接強制切換） |
| Load 上料計數欄 | 顯示 | 上料計數值，預設 0 |
| Total 總下料計數欄 | 顯示 | 總下料計數值 |
| Fail 計數欄 | 顯示 | 不良/遺失計數（目前隱藏不顯示） |
| Clear All 清除計數鈕 | 按鈕 | 停機時可用，YES/NO 確認後清除本 Lot 生產計數並寫回存檔（運轉中提示先停機） |
| Summary 彙總鈕 | 按鈕 | 產量彙總按鈕：目前**保留鈕（未啟用）** |
| 下料 Auto1~6 即時資訊面板 | 表格 | 下料 Auto1~6 即時資訊（Bin / Lot / ID / Cnt） |

| 設定項目 | 範圍/預設 | 說明 |
| --- | --- | --- |
| Recipe Name | 配方清單中存在的名稱 | 由 Recipe Name 下拉選擇，變更後重載 Tray 幾何與各子畫面 |
| User Role（使用者權限） | Operation / Supervisor / Engineer / Honprec，預設 Operation | 操作者權限等級，由 User 下拉選擇 |

> ⚠️ 注意：運轉中無法變更 Recipe（提示「Can not change recipe while machine is running.」），須先停機。

> 註：`btnClearCount`（Clear All）已於 2026-07 接上功能（見上表）；`sbPaperSummary`（Summary）確認未綁定 OnClick、無處理函式，為保留鈕。

---

## 3.6 全機歸原 (HOME) 監看畫面

按下 Home（或由 Start 觸發歸原）後，顯示 Motor Home Monitor 歸原監看畫面。此畫面為**顯示專用**，關閉/中止的決策由核心歸原流程掌控。

![HOME 復歸監看畫面](screenshots/screen-home.png)
> 圖 3-2 HOME 復歸監看畫面。（擷取方式：在 MonitorView 按 `Home` 並於 `Confirm home?` 按 Yes，歸原期間即顯示此畫面）

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| 馬達狀態格 | 表格 | 每軸一列，含軸名 + 原點燈（綠=已歸原 / 黃=歸原中 / 紅=警報 / 灰=未使用）+ 即時位置；定時刷新 |
| 進度訊息框 | 顯示 | 底部訊息框，顯示「Starting home procedure....」、「Motor power cycle...」、「Home finished.」等進度訊息 |
| Abort Home 中止歸原鈕 | 按鈕 | 中止歸原：停止所有馬達、關閉視窗；機台仍須重新成功歸原才能運轉 |

| 設定項目 | 範圍/預設 | 說明 |
| --- | --- | --- |
| 歸原步序（內部狀態） | 預設 1 | 歸原引擎內部步序狀態；離開畫面時重置 |
| HOME 監控刷新週期 | 100 ms | 歸原監看畫面刷新週期（顯示專用） |

### 歸原動作序

歸原引擎依序執行：升 TrayArm Z 上缸並開夾爪（持盤時保持夾緊不掉盤）→ 釋放 Loader 後鉤 → 前擋 → 批次回 4 支吸嘴 Z → 確認 Z 上升後批次回所有 XY 軸 → 完成。

> ⚠️ 注意：批次回 XY 前必先升 TrayArm Z 上缸並確認，避免撞擊下方盤；Loader 兩車共軌，歸原前先放後鉤再放前擋以避免拖盤對撞。

歸原完成後切回 Normal 模式並重套工作速度：
- 由 Home 按鈕觸發：完成後停機待命，監控畫面顯示「Home finished.」並自動關閉。
- 由 Start 觸發：完成後自動接續生產，不停機。

---

## 3.7 首次開機到開始生產（標準流程）

1. **程式啟動**：程式啟動後載入馬達速度基準、啟動各模組，狀態列顯示 INIT。
2. **馬達電源**：若 Pad 操作面板尚未通訊，軟體啟動時自動上電；面板通訊後改由前/後面板 Power On/Off 鍵控制。
3. **登入**：於 User 權限下拉選擇權限。Operation 直接生效；Supervisor/Engineer/Honprec 在真機提示密碼登入尚未提供（僅模擬可強制套用）。權限變更會記錄並 SECS 回報。
4. **設定模式**：必要時以 Real/Dummy 切換框切 Real/HasTray/Dummy、以 Start Mode 切換框切 Initial/Continue（兩者皆僅停機時可改）。
5. **選擇配方**：於 Recipe Name 下拉選好作業配方。
6. **建立 Lot**：於 Lot 頁輸入 Lot No.，以 Add Lot / Lot Start 建立 Lot 並載入 2D/Bin 資料。
7. **全機歸原**：按 Home，於「Confirm home?」按 Yes，觀看歸原監看畫面，待各軸原點燈轉綠、顯示「Home finished.」後自動關閉。
8. **開始生產**：切到 MonitorView 頁，按 Start。系統驗證 LotID / Lot 數 / 2D 資料；若機台尚未歸原，Start 會先歸原並於完成後自動續跑生產。
9. **運轉中監控**：系統每週期監控馬達與感測器故障、更新三色塔燈/蜂鳴器與面板燈；各模組推動生產。

---

## 3.8 暫停、單循環、清機與快照

### 暫停 / 停止

1. **暫停**：按 Pause（或前/後面板 Pause 鍵）→ 下一週期減速停止；保留歸原狀態，再按 Start 可續跑。
2. **優雅暫停**：暫停時機台先減速停止（不急停），保留現場狀態。
3. **硬停止**：遇緊急停止／故障時機台立即停止（不減速）。

> ⚠️ 注意：暫停時系統會啟動暫停碼錶並凍結氣缸/吸嘴逾時計時，恢復時再解凍累加，避免暫停時間誤算入 UPH 或誤報逾時。

### 單循環 / 清機

1. **單循環**：於 Normal / Clean Out 模式按 One Cycle（同樣須通過 Lot/2D 資料檢查）。
2. **清機**：於 Normal 模式按 Clean Out，將機台內殘料排出；完成後彈出清機完成提示，操作員選 SKIP 結束回 Normal 並停機。

### 擷取機台狀態快照 (Hang up)

1. 按 Store Hangup。
2. 等待系統打包狀態快照 zip。
3. 成功後檔案總管自動開啟並選取 zip（位於 `D:\HT160S_StateRecord\`）；失敗會提示檢查 7-Zip / 磁碟。

---

## 3.9 啟動相關提示訊息

下表為本章操作可能遇到的提示訊息（含義與排除方式）。完整故障/警報處理請參閱第 13 章。

| 提示訊息 | 含義 | 排除方式 |
| --- | --- | --- |
| `Please Enter LotID !` | 啟動前未輸入 Lot 編號 | 於 Lot No. 輸入有效 Lot 編號 |
| `No Lot data : add at least one Lot before Start !` | Lot 清單無任何 Lot | 先 Add Lot / Lot Start 建立 Lot |
| `Recipe name cannot be empty.` | Recipe Name 下拉留空 | 還原為目前配方；選擇有效配方 |
| `Can not change recipe while machine is running.` | 運轉中嘗試變更配方 | 先停機再變更 |
| `Recipe does not exist.` | 輸入的配方名稱不存在 | 選擇清單中既有配方 |
| `One Cycle is only allowed in Normal / Clean Out mode.` | 非 Normal/Clean Out 模式按 One Cycle | 切回 Normal/Clean Out 模式再執行 |
| `State Record snapshot failed (check 7-Zip / disk).` | 手動快照打包失敗 | 檢查 7-Zip 是否安裝、磁碟空間/路徑 `D:\HT160S_StateRecord\` |
| `User ID or password is incorrect.` | 切換權限時帳號或密碼輸入錯誤 | 確認帳號/密碼後重試（帳號管理見 Maintenance 帳號頁，需 Engineer 以上權限編輯） |
| `Please add at least one Lot to the list !` | Lot Start 時清單為空 | 先用 Add Lot 加入 Lot |

> 註：`cbbUserSelect` 選項與 `pnStartMode`／`pnRealDummy` 標籤已由 byte-safe DFM 讀取確認**全為英文**（Operation/Supervisor/Engineer/Honprec；Initial/Continue；Dummy/HasTray/Real），main.dfm 無任何中文 Caption/Hint，與截圖一致。
