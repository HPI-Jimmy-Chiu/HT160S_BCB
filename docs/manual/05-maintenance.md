# 第 05 章　維護畫面 (Maintenance)

維護畫面（Maintenance）是 HT160S 工程與維護用的總畫面。版面分為兩區：右側為功能選單列（按鈕由上而下切換各功能），左側為對應的分頁內容區。本畫面集中提供：號誌燈 (Tower Light) 與蜂鳴器音樂設定、硬體安裝設定、Bin Display (COM) 設定與手動測試、Top CCD / Color CCD / Lot WebAPI 連線與測試，以及開啟 IO 監看、Teach、Motor Test、Pad COM Port、SECS/GEM 等子畫面。

存檔行為分為兩類，務必區分：

- **即時套用**：部分設定（吸嘴啟用、Color CCD Enable、Loader 安全距離、Color bin / AMR 勾選等）按下後立即寫入設定，部分並立即存檔。
- **關閉時存檔**：多數號誌燈與硬體設定在按 **Exit** 關閉畫面時，一併寫入設定檔。
- **需重新啟動軟體**：分類模式 (Sort By Lot+Bin)、各 Auto 啟用變更後雖會即時寫入設定，但需重新啟動軟體才會乾淨生效（畫面會跳提示）。

![維護畫面](screenshots/screen-maintenance.png)
> 圖 5-1 維護畫面 (Maintenance)。（擷取方式：於主畫面進入維護/工具入口開啟 TfMaintenance；預設顯示第 0 頁。）

---

## 5.1 功能選單列

右側選單列的按鈕分為「切換分頁」與「開啟子畫面」兩種。子畫面類（IO Monitor / Teach / Motor Test / Pad COM Port）開啟後會獨佔輸入，且機台運轉時被停用。

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| Tower Light | 按鈕 | 切換到號誌燈與音樂設定分頁 |
| Password | 按鈕 | 切換到密碼分頁（目前為空白頁） |
| AMR | 按鈕 | 切換到 AMR 狀態分頁；分頁內的 AMR 握手狀態顯示框（唯讀）即時顯示 AMR/AGV 握手狀態 |
| Function Define | 按鈕 | 切換到功能定義分頁（含 General、Network 子頁，Network 內容隱藏） |
| Hardware Setup | 按鈕 | 切換到硬體安裝設定分頁（內含 Loader/Unloader、ErrorMag、Sort Arm、Option、Lot Info 子頁） |
| IO Monitor | 按鈕 | 開啟 IO 監看畫面（會獨佔輸入）；機台運轉時按鈕被停用且點擊無作用 |
| Teach | 按鈕 | 開啟 Teach 教導畫面（會獨佔輸入）；運轉時停用 |
| Motor Test | 按鈕 | 開啟馬達測試畫面（會獨佔輸入）；運轉時停用 |
| Pad COM Port | 按鈕 | 開啟 Pad/COM Port 畫面（會獨佔輸入）；運轉時停用 |
| Bin Display | 按鈕 | 切換到 Bin Display (COM) 設定與手動測試分頁 |
| Top CCD | 按鈕 | 切換到 Top CCD 連線設定與測試分頁 |
| Color CCD | 按鈕 | 切換到 Color CCD 連線設定與測試分頁 |
| Lot WebAPI | 按鈕 | 切換到 Lot WebAPI 設定與手動 Fetch 測試分頁 |
| SECS/GEM | 按鈕 | 開啟 SECS/GEM 監看視窗（非獨佔，可與主畫面並存）；運轉時仍可開啟 |
| Exit | 按鈕 | 關閉維護畫面；關閉時自動存檔（寫入號誌燈與各項硬體/裝置設定） |

> ⚠️ 注意：機台運轉中時，IO Monitor / Teach / Motor Test / Pad COM Port 四個按鈕會被停用（純視覺互鎖）；即使被點到也不會造成停機。**SECS/GEM 刻意不鎖**，運轉中仍可檢視。

### 開啟子畫面操作

1. 按 **IO Monitor** / **Teach** / **Motor Test** / **Pad COM Port** 開啟對應子畫面（會獨佔輸入）。
2. 按 **SECS/GEM** 開啟 SECS 監看視窗（非獨佔，可與主畫面並存）。

---

## 5.2 號誌燈與蜂鳴器音樂 (Tower Light)

按 **Tower Light** 進入號誌燈分頁。畫面為 6 個狀態列 × 3 個燈色的 LED 格陣列，每列右側對應一個蜂鳴器音樂選擇 (Music Select)，下方提供 Music 試聽鈕。

狀態列（列標題）：Running / Error-Jam / Pause / Message / Reserved（隱藏）/ Homing。燈色欄位：Green / Yellow / Red。

### 控制項

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| 號誌燈 LED 格（6 列狀態 × 3 色） | 圖示 | 點擊單格在 ON↔OFF 間切換（已移除 BLINK），即時更新顯示；畫面關閉時存入號誌燈設定 |
| 狀態列標題 | 標籤 | Running / Error-Jam / Pause / Message / Reserved / Homing（Reserved 列已隱藏不顯示） |
| 燈色欄標題 | 標籤 | Green / Yellow / Red |
| 各列 Music Select 選項組 | 選項組 | 各狀態列對應的蜂鳴器音樂選擇；存入號誌燈設定。Reserved（Heating）列的選項組已隱藏 |
| Music1~Music4 試聽鈕 | 按鈕 | 按下立即觸發對應蜂鳴器開/關，再按一次或切換其他鍵會關閉；即時動作、不存檔 |

### Music Select 選項

| 畫面項目 | 範圍/預設 | 說明 |
| --- | --- | --- |
| Music Select（各列音樂選擇） | 0=Mute，1~4=Music1~Music4；預設 0 | 各狀態列對應的蜂鳴器音樂，存於號誌燈設定。**實際何時依此設定發聲由執行時套用；本畫面只負責存值。** |
| 號誌燈 LED 狀態（6 列 × 3 色） | 每格 OFF / ON（BLINK 顯示為 ON）；預設值由系統給定 | 6 狀態列 × 3 色的 ON/OFF 狀態，存於號誌燈設定。BLINK 已當成 ON 顯示。 |

### 操作步驟

1. 按右側 **Tower Light** 進入號誌燈分頁。
2. 在 6 列（Running / Error-Jam / Pause / Message / Homing；Reserved 列隱藏）× 3 色（Green / Yellow / Red）的格上點擊，切換該色 ON/OFF。
3. 在每列右側 **Music Select** 選擇該狀態要播放的音樂（[0]Mute ~ [4]Music4）。
4. （可選）按 **Music1~Music4** 試聽，再按一次或切換其他鍵停止。
5. 按 **Exit** 關閉畫面；關閉時自動將號誌燈狀態與音樂選擇存入號誌燈設定。

> ⚠️ 注意：LED 格已取消 BLINK，僅在 ON/OFF 間切換。Music 試聽為即時動作，不寫入設定檔。

> 註：RadioGroup6 對應的狀態列（enum `LED_Heating`，畫面標題 "Reserved"，Visible=False）為**保留列**：HT172 有加熱器 Heating 狀態，HT160 無加熱器，`GetTowerLightRunState` 永遠不會回傳 Heating（蜂鳴 ladder 併入 Running）——設定值會被讀取但執行期永不套用。

---

## 5.3 隱藏的 Heating 列

號誌燈第 5 列（列標題顯示為 **Reserved**）在畫面上不顯示；其對應的 Music Select 同樣隱藏。此列在系統內部對應加熱器（Heating）狀態。

| 畫面項目 | 狀態 | 說明 |
| --- | --- | --- |
| Reserved 狀態列標題 | 隱藏 | 第 5 狀態列標題，畫面上不顯示 |
| Reserved 列的 Music Select | 隱藏 | 該列蜂鳴器音樂選擇，畫面上不顯示 |

> 註：Heating（enum `LED_Heating`）＝畫面 "Reserved" 列。HT160 無加熱器，此狀態執行期不可達，屬 HT172 沿革保留位（見 5.2 註）。

---

## 5.4 硬體安裝設定 (Hardware Setup)

按 **Hardware Setup** 進入硬體安裝分頁，內含 Loader/Unloader、ErrorMag、Sort Arm、Option、Lot Info 等子頁。

> ⚠️ 注意：Lot 運轉中時，Loader/Unloader 的 13 個硬體勾選框（Color bin / AMR / Lot+Bin / Auto1-6 / Nozzle1-4）會被鎖定，須結束 Lot 才能編輯；標題會顯示 `(locked - lot running, end lot to edit)`。

### 5.4.1 Loader/Unloader 子頁

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| Color bin area installed | 勾選 | 是否安裝 Color bin 區硬體；勾選即時生效，關閉畫面時存檔 |
| Use AMR | 勾選 | 是否使用 AMR；勾選即時生效，關閉畫面時存檔 |
| Auto1~Auto6 | 勾選 | 各 Auto 啟用（僅動態模式 By Lot+Bin / By Lot+PassFail 有效）；未勾選的 Auto 在綁定新分類鍵時被跳過。勾選即時生效，但會跳提示需重新啟動軟體才乾淨生效；運轉時被鎖定 |
| Sort Mode | 選項組（3 選項） | 切換分類模式：Normal（靜態 Bin→Auto 表）/ By Lot+Bin（動態綁定 Lot+Bin）/ By Lot+PassFail（動態綁定 Lot+PASS/FAIL，PASS/FAIL 由 Bin 是否等於 Pass Bin 導出）。切換即時生效並提示需重新啟動軟體；運轉時被鎖定。詳見第 15 章 |
| Loader safe distance | 輸入 | 兩台 Loader 車的最小間距（325~650 mm）；唯讀欄，點擊以螢幕鍵盤輸入，OK 後再以 YES/NO 確認才存（以 mm 顯示/輸入）並即時存檔；運轉時被鎖定 |

#### 相關參數

| 畫面項目 | 範圍/預設 | 說明 |
| --- | --- | --- |
| Color bin area installed | 勾/不勾 | 是否安裝 Color bin 區硬體 |
| Use AMR | 勾/不勾 | 是否使用 AMR |
| Sort Mode | Normal / By Lot+Bin / By Lot+PassFail；變更需重啟軟體 | 分類模式：Normal＝靜態 Bin→Auto、By Lot+Bin＝動態綁定、By Lot+PassFail＝動態綁定（依 Bin 是否等於 Pass Bin 分 PASS/FAIL） |
| Auto1~Auto6 | 各別勾選，預設皆勾選；變更需重啟 | Auto1~Auto6 各別啟用（僅動態模式有效） |
| Loader safe distance | 325.00~650.00 mm；預設顯示 100.00 | 兩台 Loader 車最小間距，畫面以 mm 顯示/輸入 |

#### 操作步驟

1. 按 **Hardware Setup** 進入硬體分頁，選 **Loader/Unloader** 子頁。
2. 視機台安裝勾選 **Color bin area installed** / **Use AMR**（即時寫入記憶體設定）。
3. （By Lot+Bin 模式）勾/取消 **Auto1~Auto6**；會提示需重新啟動軟體。
4. 點 **Loader safe distance** 欄，以螢幕鍵盤輸入 325~650 mm，OK 後確認 **YES** 即時存檔。
5. 勾選/取消等變更於畫面關閉時一併寫入設定檔。

> ⚠️ 注意：Loader 安全距離輸入經螢幕鍵盤限制在 325..650 mm，OK 後再以 YES/NO 確認才存；按 NO 則還原原值。

### 5.4.2 Sort Arm 子頁（吸嘴啟用）

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| Nozzle1~Nozzle4 | 勾選 | 各 SortArm 吸嘴啟用；未勾選者在取放時被跳過，可將壞吸嘴停用。勾選即時生效並存檔，每次取料循環即時讀取（免重啟）；至少需保留一個啟用，取消最後一個會自動勾回並提示；運轉時被鎖定 |

| 畫面項目 | 範圍/預設 | 說明 |
| --- | --- | --- |
| Nozzle1~Nozzle4 | 各別勾選，預設皆勾選；至少一個啟用；即時生效 | SortArm 吸嘴 Nozzle1~4 各別啟用 |

#### 操作步驟

1. 在 Hardware Setup 的 **Sort Arm** 子頁勾/取消 **Nozzle1~Nozzle4**。
2. 變更即時生效並存檔；取放循環即時讀取，免重啟。
3. 若取消最後一個啟用的吸嘴，會被自動勾回並提示至少保留一個。

> ⚠️ 注意：SortArm 吸嘴至少需保留一個啟用；取消最後一個會自動勾回並彈出提示。運轉時此區被鎖定。

### 5.4.3 Option 子頁

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| Bin 顯示面板型號下拉 | 下拉 | 選擇 Bin 顯示面板型號（LED (HT9046) / TFT (HT9011)）；於畫面關閉時存檔 |
| CommType | 勾選 | Option 分頁的 CommType 勾選框（保留項，勾選無作用） |

| 畫面項目 | 範圍/預設 | 說明 |
| --- | --- | --- |
| Bin 顯示面板型號下拉 | LED (HT9046) / TFT (HT9011)；預設 LED (HT9046) | Bin 顯示面板型號 |

> 註：cbCommType（Option 分頁 "CommType" 勾選框）在 DFM 中存在，但載入/存檔與其他程式均未引用——**目前無作用（保留元件）**，勾選不影響任何行為。

### 5.4.4 ErrorMag 子頁（錯誤 Bin 對應顯示）

ErrorMag 子頁為唯讀資訊，顯示兩個特殊錯誤 Bin 目前對應的料區名稱。

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| 1000 = 2D scan fail -> [區域] | 顯示 | 2D 掃描失敗對應的料區（唯讀） |
| 1001 = no bin setting -> [區域] | 顯示 | 無 Bin 設定對應的料區（唯讀） |

> 註：Function Define 分頁的 G[General]/N[Network] 子頁內容為空（Panel4 設 Visible=False）、tsMaintPassword 分頁為空白頁——皆為**保留頁（未實作）**，非故障。

---

## 5.5 Bin Display (COM) 設定與手動測試

按 **Bin Display** 進入分頁。上半為連線設定，下半為手動測試與操作記錄。

### 連線設定

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| Bin Display Installed | 勾選 | Bin Display 是否安裝；按 Save 時存檔並重啟 COM |
| COM Port | 輸入 | Bin Display COM 埠（預設 COM5）；按 Save 存檔 |
| Baud | 下拉 | Bin Display 鮑率（9600/19200/38400/57600/115200）；按 Save 存檔，非法值回退 9600 |
| Delay(s) | 輸入 | Bin Display 重連/延遲秒數（預設 5）；按 Save 存檔（最小 1） |
| Save | 按鈕 | 存 Bin Display 設定並重啟 COM（重設連線端點與標籤） |
| Reload | 按鈕 | 重新載入 Bin Display 設定 |
| Refresh Status | 按鈕 | 刷新 Bin Display 狀態列（安裝/COM/狀態/單元數） |

### 手動測試

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| Address | 輸入 | 目標 Bin 顯示單元位址（預設 0） |
| Text | 輸入 | 要顯示的 Bin 文字/碼（預設 9） |
| Color 下拉 | 下拉 | 手動測試色彩下拉 (GREEN / RED)（送出時實際以 Color Code 為準，此下拉無作用） |
| Color Code | 輸入 | 顏色代碼（Send Display / Send Light 實際使用，預設值 3） |
| Symbol Code | 勾選 | Symbol Code 勾選框（保留項，送出時未引用） |
| Send Display | 按鈕 | 對 Address 設定 Bin 文字+顏色，即時動作 |
| Send Code | 按鈕 | 對 Address 只設定 Bin 碼，即時動作 |
| Send Light | 按鈕 | 對 Address 只設定顏色，即時動作 |
| 操作記錄欄 | 顯示 | 唯讀操作記錄欄，保留最近 200 行 |

### 相關參數

| 畫面項目 | 範圍/預設 | 說明 |
| --- | --- | --- |
| Bin Display Installed | 勾/不勾 | 是否安裝 Bin Display |
| COM Port | 文字；預設 COM5 | Bin Display COM 埠 |
| Baud | 9600/19200/38400/57600/115200；預設 9600，非法回退 9600 | Bin Display 鮑率 |
| Delay(s) | 最小 1；預設 5 | Bin Display 重連/延遲秒數 |

### 操作步驟

1. 按 **Bin Display** 進入分頁。
2. 設定 **Bin Display Installed**、**COM Port**、**Baud**、**Delay(s)**，按 **Save** 存檔並重啟 COM。
3. 在手動測試區輸入 **Address** / **Text** / **Color Code**，按 **Send Display** / **Send Code** / **Send Light** 即時對單元送出。
4. 按 **Refresh Status** 刷新狀態。

> 註：cbbMCUColor（Color 下拉 GREEN/RED）在三個 Send 按鈕程式中均未被讀取，送色一律以 **Color Code（`edMCULightValue`）為準**——下拉**無作用（遺留元件）**。chkMCUCodeSymbol（"Symbol Code"）同樣未被 btnMCUSend* 引用，為**保留元件**。

---

## 5.6 Top CCD 設定與測試

按 **Top CCD** 進入分頁。

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| IP | 輸入 | Top CCD IP（預設 172.16.8.89）；按 Save 存檔 |
| Port | 輸入 | Top CCD Port（預設 5001）；按 Save 存檔 |
| Bottom CCD (reserved) | 勾選 | Bottom CCD（保留項，停用，僅佔位） |
| Save | 按鈕 | 存 Top CCD 端點並即時套用連線 |
| Reload | 按鈕 | 重新載入 Top CCD 端點並重設連線 |
| Connect | 按鈕 | 先存端點再連線 Top CCD（即時） |
| Disconnect | 按鈕 | 中斷 Top CCD 連線（即時） |
| Trigger Shot | 按鈕 | 手動觸發 Top CCD 拍照取 2D 結果（即時） |
| 結果碼顯示欄 | 顯示 | 顯示收到的 2D 碼（唯讀刷新） |
| 記錄欄 | 顯示 | 唯讀記錄欄，保留最近 200 行 |

| 畫面項目 | 範圍/預設 | 說明 |
| --- | --- | --- |
| Top CCD 連線端點（IP / Port） | 預設 172.16.8.89 : 5001；Port 1..65535，非法回退 5001 | Top CCD 連線端點 |

### 操作步驟

1. 按 **Top CCD** 進入分頁。
2. 輸入 **IP** / **Port**，按 **Save** 存端點。
3. 按 **Connect** / **Disconnect** 連線或中斷。
4. 按拍照鈕觸發一次取像，結果碼顯示於結果欄。

> 註：btnTopCcdShot 的 DFM Caption 已確認為「**Trigger Shot**」（byte-safe 讀取 maintenance.dfm）。

---

## 5.7 Color CCD 設定與測試

按 **Color CCD** 進入分頁。本頁的 **Enable Color CCD** 為即時動作。

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| IP | 輸入 | Color CCD IP（預設 172.16.8.100）；按 Save 存檔 |
| Port | 輸入 | Color CCD Port（預設 5000）；按 Save 存檔 |
| Enable Color CCD | 勾選 | 啟用 Color CCD；勾選即時存檔並依勾選連線/斷線 |
| Save | 按鈕 | 存 Color CCD 端點 + Enable 並套用連線 |
| Reload | 按鈕 | 重新載入 Color CCD 設定並重設連線 |
| Connect | 按鈕 | 先存端點再連線 Color CCD（即時） |
| Disconnect | 按鈕 | 中斷 Color CCD 連線（即時） |
| Trigger Shot | 按鈕 | 手動觸發 Color CCD 拍照；未連線時提示先 Connect |
| 結果碼顯示欄 | 顯示 | 顯示收到的 2D 碼（唯讀刷新） |
| 記錄欄 | 顯示 | 唯讀記錄欄，保留最近 200 行 |

| 畫面項目 | 範圍/預設 | 說明 |
| --- | --- | --- |
| Color CCD 連線端點與啟用（IP / Port / Enable） | 預設 172.16.8.100 : 5000，Enable 預設開啟 | Color CCD 連線端點與啟用 |

### 操作步驟

1. 按 **Color CCD** 進入分頁。
2. 輸入 **IP** / **Port**，可勾 **Enable Color CCD**（即時存檔並連/斷線）。
3. 按 **Save** 存端點，按 **Connect** / **Disconnect** 連線或中斷。
4. 按拍照鈕觸發一次取像，結果碼顯示於結果欄（未連線時會提示先 Connect）。

> ⚠️ 注意：勾選 **Enable Color CCD** 為即時動作，會立即存檔並依勾選連線/斷線。

> 註：btnColorCcdShot 的 DFM Caption 已確認為「**Trigger Shot**」（byte-safe 讀取 maintenance.dfm）。

---

## 5.8 Lot WebAPI 設定與手動 Fetch 測試

按 **Lot WebAPI** 進入分頁。

| 畫面項目 | 類型 | 功能 |
| --- | --- | --- |
| WebAPI Path | 輸入 | Lot WebAPI 基底 URL；按 Save / Reload 存檔/讀取 |
| Auto-pull | 勾選 | 是否在 Lot Start / SECS LOTSTART 時自動抓取（客戶 API 就緒前預設關閉）；按 Save 時存檔 |
| Save | 按鈕 | 存 Lot WebAPI URL + 自動抓取設定 |
| Reload | 按鈕 | 重新載入 Lot WebAPI 設定 |
| Lot ID | 輸入 | 手動測試用 Lot 名稱（預設 A5921.RCS.TEST99） |
| Fetch | 按鈕 | 先存目前 URL，再對輸入 Lot 發起一次抓取；空 Lot 或忙碌中會中止 |
| Result 欄 | 顯示 | 顯示 HTTP 狀態與回應內容 |
| 記錄欄 | 顯示 | 唯讀記錄欄，保留最近 200 行 |

| 畫面項目 | 範圍/預設 | 說明 |
| --- | --- | --- |
| WebAPI Path + Auto-pull | 空值回退 http://127.0.0.1:8160/lot/；自動抓取預設依設定 | Lot WebAPI 基底 URL 與是否自動抓取 |

### 操作步驟

1. 按 **Lot WebAPI** 進入分頁。
2. 輸入 **WebAPI Path**（基底 URL），（可選）勾 **Auto-pull**，按 **Save** 存檔。
3. 在手動測試區輸入 **Lot ID**，按 **Fetch** 發一次抓取。
4. 於 Result 欄查看 HTTP 狀態與回應內容。

> ⚠️ 注意：按 **Fetch** 後系統會在背景進行抓取；輸入 Lot 為空或目前正在抓取（忙碌中）時會中止。

---

## 5.9 互鎖與運轉鎖定總覽

本畫面為設定面，互鎖以「運轉 / Lot 編輯鎖」為主：

| 互鎖 | 觸發條件 | 行為 |
| --- | --- | --- |
| 子畫面按鈕鎖 | 機台運轉中 | 停用 IO Monitor / Teach / Motor Test / Pad COM Port 按鈕；即使被點到也不會停機 |
| SECS/GEM 不鎖 | — | 刻意不鎖，運轉中仍可檢視 |
| 硬體編輯鎖 | Lot 運轉中 | 鎖定 13 個硬體勾選框（Color bin / AMR / Lot+Bin / Auto1-6 / Nozzle1-4）；標題顯示 (locked - lot running, end lot to edit) |
| 吸嘴最少保留 | 取消最後一個啟用吸嘴 | 自動勾回並彈出提示，至少保留一個 |
| Loader 安全距離確認 | 輸入後 | 螢幕鍵盤限制 325..650 mm，OK 後再以 YES/NO 確認才存，否則還原原值 |

> ⚠️ 互鎖原則：防碰撞安全互鎖在正常運轉中一律生效、不會被旁路。本畫面主要為運轉 / Lot 編輯鎖。

---

## 5.10 提示訊息一覽

| 訊息 | 意義 | 處置 |
| --- | --- | --- |
| Sort mode changed. Please restart the software so the new classification mode takes effect cleanly. | 切換 Sort By Lot+Bin 模式後的提示 | 重新啟動軟體使新分類模式乾淨生效 |
| Auto enable changed. Please restart the software so the new Lot+Bin routing takes effect cleanly. | 變更 Auto1~6 啟用後的提示 | 重新啟動軟體使新 Lot+Bin 路由生效 |
| At least one nozzle must stay enabled. | 嘗試取消最後一個 SortArm 吸嘴時的提示；系統會自動勾回 | 至少保留一個吸嘴啟用 |
| Save Loader safe distance? | Loader 安全距離輸入後的 YES/NO 確認 | YES=存檔套用；NO=還原原值 |
| 1000 = 2D scan fail -> [area] | ErrorMag 頁顯示錯誤 Bin 1000（2D 掃描失敗）對應的料區（資訊，非警報） | 資訊顯示 |
| 1001 = no bin setting -> [area] | ErrorMag 頁顯示錯誤 Bin 1001（無 Bin 設定）對應的料區（資訊） | 資訊顯示 |

---

## 5.x 維護樞紐子畫面總覽

維護畫面（Maintenance）右側選單為各工程子畫面的進入點。下列截圖為各子畫面實機畫面；其中 Hardware Setup 詳見第 6/15 章、Top/Color CCD 與料倉顯示見第 14 章、Lot WebAPI 見第 12 章。

![Hardware Setup 硬體安裝設定（Color bin / Use AMR / 各 Auto 啟用 / Loader 安全距離）](screenshots/screen-hardware.png)
> 圖 5-1 Hardware Setup 硬體安裝設定（Color bin / Use AMR / 各 Auto 啟用 / Loader 安全距離）。（擷取方式：Maintenance 右側選單點選對應項目。）

![Password 權限/密碼](screenshots/screen-password.png)
> 圖 5-2 Password 權限/密碼。（擷取方式：Maintenance 右側選單點選對應項目。）

![Function Define 功能定義](screenshots/screen-funcdef.png)
> 圖 5-3 Function Define 功能定義。（擷取方式：Maintenance 右側選單點選對應項目。）

![Bin Display 料倉顯示面板設定](screenshots/screen-bindisplay.png)
> 圖 5-4 Bin Display 料倉顯示面板設定。（擷取方式：Maintenance 右側選單點選對應項目。）

![Top CCD 頂部 CCD 設定/觸發](screenshots/screen-topccd.png)
> 圖 5-5 Top CCD 頂部 CCD 設定/觸發。（擷取方式：Maintenance 右側選單點選對應項目。）

![Color CCD 顏色 CCD 設定/觸發](screenshots/screen-colorccd.png)
> 圖 5-6 Color CCD 顏色 CCD 設定/觸發。（擷取方式：Maintenance 右側選單點選對應項目。）

![Lot WebAPI Lot 2D/Bin 資料擷取](screenshots/screen-lotapi.png)
> 圖 5-7 Lot WebAPI Lot 2D/Bin 資料擷取。（擷取方式：Maintenance 右側選單點選對應項目。）

![Pad COM Port 操作面板通訊埠](screenshots/screen-padcom.png)
> 圖 5-8 Pad COM Port 操作面板通訊埠。（擷取方式：Maintenance 右側選單點選對應項目。）

