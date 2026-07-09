# 第 6 章　設定 (Config / Setup)

本章說明 HT160S 的「產品設定 (Product Setup)」畫面，以及與之相關的配方 (Recipe / WorkFile) 管理、料盤幾何 (Tray Form)、料區對 Bin 的對應 (Bin Area Map)，並涵蓋三種分流模式：靜態的 Normal 模式與動態的 By Lot+Bin、By Lot+PassFail 模式。設定畫面以「配方」為單位儲存，所有資料寫入 `data\<配方名>` 資料夾下的 `setup.ini`、`BinAreaMap.ini`、`manifest.ini`。

設定畫面 (`TfSetup`，Caption=`Product Setup`) 由左側選單列切換三個分頁：

- **Recipe**：配方管理（新增、複製另存、套用、刪除）。
- **Tray Form**：料盤幾何（起點、間距、分割數）。
- **Bin Setting**：料區 (Area) → Bin 編號對應表與錯誤料 (NG) 收集區。

![設定畫面](screenshots/screen-setup.png)
> 圖 6-1 Product Setup 設定畫面（左側選單列 Recipe / Tray Form / Bin Setting / Exit）。（擷取方式：由主畫面進入維護/設定入口開啟 Product Setup 畫面，預設顯示 Recipe 分頁。）

> ⚠️ 注意：設定畫面所有可見文字皆為英文。機台運轉中 (`SystemStart`) 時，`Use Recipe` 與 `Delete` 按鈕會被自動禁用；切換或刪除配方須先停機。

---

## 6.1 設定畫面導覽

### 6.1.1 左側選單與標題列

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `spbSetupRecipe` | speedbutton(選單) | 切換到 Recipe 配方管理分頁 (`tsSetupRecipe`) |
| `spbSetupTrayForm` | speedbutton(選單) | 切換到 Tray Form 料盤幾何分頁 (`tsSetupTrayForm`) |
| `spbSetupBinSetting` | speedbutton(選單) | 切換到 Bin Setting 料區對應分頁 (`tsSetupBinSetting`) |
| `spbSetupExit` | speedbutton(選單) | 關閉設定畫面；關閉時 `FormClose` 會自動存檔目前配方 (`SaveWorkFile`) |
| `pnlTitle` | panel(標籤) | 標題列，顯示目前作用分頁的 Caption |

操作步驟（切換分頁）：

1. 按左側選單 `Recipe` / `Tray Form` / `Bin Setting` 任一按鈕。
2. 畫面切換到對應分頁，標題列顯示該分頁名稱。
3. 按 `Exit` 關閉畫面（關閉時自動存檔）。

### 6.1.2 配方狀態欄

下列標籤位於畫面狀態區，僅供顯示目前配方與檔案狀態：

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `lblCurrentRecipeValue` | label | 顯示目前使用中的配方名稱 |
| `lblRecipeDirValue` | label | 顯示配方資料夾名稱/路徑 |
| `lblSetupFileValue` | label | 顯示 `setup.ini` 檔案路徑 |
| `lblBinAreaMapValue` | label | 顯示 `BinAreaMap.ini` 預設檔路徑 |
| `lblSetupFileStatusValue` | label | 顯示 `setup.ini` 狀態：Ready / Not Created |
| `lblManifestValue` | label | 顯示 `manifest.ini` 狀態：Ready / Not Created |
| `lblRecipeNote` | label | 提示文字：配方資料存於 `data\<配方名>` 之下 |

---

## 6.2 Recipe 配方管理

Recipe 分頁負責整套生產配方的新增、複製另存、套用與刪除。配方以 `data\<RecipeName>` 目錄為單位；目前使用中的配方名存於 `system\lastset.ini` 的 `[LastSet] RecipeName`（相容舊鍵 `cob_MainWorkFile`），開機時載入，預設為 `Default`。

### 6.2.1 控制項

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `lstRecipe` | listbox | 列出 `data` 根目錄下所有配方資料夾供選取；點選會把名稱帶入 `edRecipeName` |
| `edRecipeName` | edit | 輸入新配方名稱（供 Save As / New Blank 使用） |
| `spbRecipeSave` | speedbutton | 存檔目前配方（`setup.ini` + Tray Form + Bin Map + manifest），並驗證 Bin Map |
| `spbRecipeSaveAs` | speedbutton | 以 `edRecipeName` 名稱另存為新配方（複製現配方）；名稱重複或為空會擋下 |
| `spbRecipeUse` | speedbutton | 切換選取的配方為使用中配方並重新載入；運轉中 (`SystemStart`) 被禁用/擋下 |
| `spbRecipeNewBlank` | speedbutton | 以 `edRecipeName` 建立空白新配方，寫入預設 Tray Form 與空白 Bin Map |
| `spbRecipeDelete` | speedbutton | 刪除選取配方（Yes/No 確認）；不可刪目前使用中配方；運轉中被禁用/擋下 |
| `spbRecipeRefresh` | speedbutton | 重新掃描配方清單與狀態 |

> 配方名稱會經正規化處理：去除 `\ / : * ? " < > |` 等非法字元並以底線取代；空白名退回 `Default`。

### 6.2.2 建立全新空白配方 (New Blank)

1. 切到 Recipe 分頁。
2. 在 `Recipe Name` 欄輸入新配方名稱（不可空白）。
3. 按 `New Blank`；若名稱重複或建立失敗會提示。
4. 系統建立資料夾並寫入 `setup.ini`、`manifest.ini`、預設 Tray Form、空白 `BinAreaMap.ini`。
5. 清單刷新並選取新配方。

### 6.2.3 由現配方複製另存 (Save As)

1. 在 `Recipe Name` 欄輸入新名稱（不可空白、不可重複）。
2. 按 `Save As`。
3. 系統先存檔現配方，再複製為新配方，並於 `manifest.ini` 記錄來源配方。
4. 清單刷新並選取新配方，提示已另存。

### 6.2.4 套用配方 (Use Recipe)

1. 在 `Recipe List` 選取一個配方。
2. 按 `Use Recipe`。
3. 若機台運轉中 (`SystemStart`) 會被擋下並提示無法切換。
4. 存檔現配方後，將選取配方設為使用中並重新載入 (`OpenWorkFile`)。

### 6.2.5 刪除配方 (Delete)

1. 在 `Recipe List` 選取要刪的配方（不可為目前使用中）。
2. 按 `Delete`。
3. 若運轉中或選到使用中配方會被擋下。
4. 出現 Yes/No 確認對話 (`Delete recipe <name>?`)，選 `Yes` 才刪除。
5. 清單與狀態刷新。

> ⚠️ 注意：不可刪除目前使用中的配方；切換或刪除配方均須先停機。

---

## 6.3 Tray Form 料盤幾何

Tray Form 分頁定義料盤的座標起點、格位間距與分割數。每次變更會即時重繪料盤格位預覽；存檔（Recipe 分頁 `Save` 或關閉畫面）時寫入 `setup.ini` 的 `[TrayForm]` 區段，並呼叫 `TrayForm.Load` 即時更新記憶體幾何供 Loader/SortArm/Auto/Monitor 取用。

### 6.3.1 控制項

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `edXStart` | edit | 料盤 X 起點座標（浮點，預設 0.000） |
| `edXPitch` | edit | 料盤 X 方向間距（浮點，預設 1.000） |
| `edYStart` | edit | 料盤 Y 起點座標（浮點，預設 0.000） |
| `edYPitch` | edit | 料盤 Y 方向間距（浮點，預設 1.000） |
| `edXDivision` | edit | 料盤 X 方向格數（整數，1..`MAX_TRAY_X`=20） |
| `edYDivision` | edit | 料盤 Y 方向格數（整數，1..`MAX_TRAY_Y`=50） |
| `TMyTray1` | tray-preview | 依 X/Y Division 即時重繪料盤格位預覽，格內依序編號 |

### 6.3.2 參數

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `TrayForm.XStart` | float，預設 0.0 | 料盤 X 起點（`setup.ini [TrayForm]`） |
| `TrayForm.XPitch` | float，預設 1.0 | 料盤 X 間距 |
| `TrayForm.YStart` | float，預設 0.0 | 料盤 Y 起點 |
| `TrayForm.YPitch` | float，預設 1.0 | 料盤 Y 間距 |
| `TrayForm.XDivision` | int，1..20，預設 1 | 料盤 X 格數（上限 `MAX_TRAY_X`=20） |
| `TrayForm.YDivision` | int，1..50，預設 1 | 料盤 Y 格數（上限 `MAX_TRAY_Y`=50） |

> 【待補：`TrayForm.XStart/XPitch/YStart/YPitch` 的工程單位（mm 或 1/100mm）無法由設定畫面原始碼判定；設定畫面只做 `ReadFloat/WriteFloat`。實際單位需確認 TrayForm 結構與 Loader/SortArm 使用端。】

### 6.3.3 操作步驟

1. 切到 Tray Form 分頁。
2. 輸入 `X-Start` / `X-Pitch` / `Y-Start` / `Y-Pitch`（浮點）與 `X-Division` / `Y-Division`（整數）。
3. 每次變更會即時重繪料盤格位預覽。
4. 按 Recipe 分頁的 `Save`，或關閉畫面時，寫入 `setup.ini` 的 `[TrayForm]` 區段。
5. 存檔後 `TrayForm.Load` 即時更新記憶體幾何。

> ⚠️ 注意：`X-Division` / `Y-Division` 超過機台上限（X=20 / Y=50）時，值會被夾限並提示 `Tray division exceeds machine limit (X max=.., Y max=..). Value clamped.`，修正後才存。

---

## 6.4 Bin Setting 料區對應表

Bin Setting 分頁設定「料區 (Area) → Bin 編號」的對應表，以及錯誤料 (NG) 的收集料區。每列對應一個料區：Auto1..Auto6，若安裝色彩 Bin 區則再加 Color。料區清單末端是否含 Color 區，由 `GeneralSetting.bColorBinAreaInstalled` 決定（裝=到 `eHT160BinAreaColor`，未裝=到 `eHT160BinAreaAuto6`）；此畫面僅唯讀顯示，實際安裝旗標於 GeneralSetting 設定。

### 6.4.1 控制項

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `grdBinAreaMap` | grid | 料區→Bin 對應表（Area / Bin / Status / Note）；每列一個 Area，編輯 Bin 欄；驗證後顯示 OK/Empty/Duplicate/Invalid/Error 狀態與說明 |
| `cbbBinErrorArea` | combobox | 選擇錯誤料 (NG) 的收集料區；只列出已啟用的 Area，預設 `eHT160BinAreaAuto6` |
| `cbbPassBin` | combobox | 選擇視為 PASS 的 Bin：選項為 `0`（=關閉）+ 表格中各已設定的路由 Bin。存入配方 `BinAreaMap.ini [BinAreaMap] PassBin`。決定 Production_Log 的 PassFail 欄，且為 By Lot+PassFail 分流模式的分類依據（見第 15 章）。By Lot+PassFail 的 Lot 執行中變更會被拒絕 |
| `spbBinLoadMap` | speedbutton | 從預設 `BinAreaMap.ini` 重新載入對應表到表格 |
| `spbBinSaveMap` | speedbutton | 驗證並儲存 Bin 對應表（含 Error Bin 區）到預設 ini，顯示結果 |
| `spbBinValidate` | speedbutton | 驗證表格內容並顯示是否 OK（檢查範圍/重複/空白） |
| `spbBinClear` | speedbutton | 將所有列 Bin 清為 0（未指派） |
| `spbBinDefault` | speedbutton | 依列序填入預設 Bin 值（1..N）；Caption 隨料區數動態變為 `Default 1-<count>` |

Bin 分頁狀態欄：

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `lblBinRecipeValue` | label | 顯示目前配方名稱 |
| `lblBinFileValue` | label | 顯示 Bin 對應表 ini 檔路徑 |
| `lblBinMappedValue` | label | 顯示已對應數 / 總料區數（Count / N） |
| `lblBinColorValue` | label | 顯示色彩 Bin 區是否安裝：Installed / Not Installed |
| `lblBinMapStatusValue` | label | 顯示對應表檔案狀態與已對應數：Ready / Not Created（Count / N） |
| `lblBinPlaceholder` | label | 提示文字：Bin=0 代表該料區未指派 |

### 6.4.2 參數

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `BinAreaMap (Area->Bin)` | 每區 Bin 0..999，0=未指派 | 各料區對應的 Bin 編號（`BinAreaMap.ini`） |
| `ErrorBinArea` | 已啟用之 Area，預設 `eHT160BinAreaAuto6` | 錯誤料 (NG) 收集料區 |
| `BinAreaMap.PassBin` | 0（關閉）或某個已設定的 Bin；`[BinAreaMap] PassBin` | 視為 PASS 的 Bin。決定 Production_Log PassFail 欄；By Lot+PassFail 模式的分類/路由依據（該模式啟動前須 >0） |

> ⚠️ 注意：Bin 值範圍為 0..999（`HT160_BIN_AREA_NORMAL_MAX_BIN`=1000，1000 起為錯誤碼）。非數字或越界視為 `Invalid`；同一 Bin 指派到不同料區視為 `Duplicate`；驗證未通過時 `Save` 會被擋下。

### 6.4.3 操作步驟

1. 切到 Bin Setting 分頁。
2. 在表格 Bin 欄為每個料區（Auto1..Auto6，及 Color 若已安裝）填入 Bin 編號（0..999，0=未指派）。
3. 在 `Error Bin` 下拉選擇錯誤料收集料區。
4. 按 `Validate` 檢查（範圍/重複/空白），狀態欄顯示 OK/Empty/Duplicate/Invalid/Error。
5. 按 `Save Map` 驗證並存檔到 `BinAreaMap.ini`。
6. 視需要可用 `Load Current` 重新載入、`Clear All` 全清為 0、`Default 1-N` 依序填入預設值。
7. 若使用 By Lot+PassFail 分流，另以 `cbbPassBin` 選擇視為 PASS 的 Bin（存入同一 `BinAreaMap.ini`）。

> ⚠️ 注意：在動態分流模式（`GeneralSetting.iSortMode`=1 By Lot+Bin 或 =2 By Lot+PassFail）下，整張表的 Bin 欄禁止手動編輯（Auto↔Bin 由執行時動態綁定），此時只能修改 `Error Bin`；`cbbPassBin` 為獨立控制項，仍可設定（惟 By Lot+PassFail 的 Lot 執行中會被鎖）。

---

## 6.5 配方存取流程

設定畫面的載入/儲存流程如下，供理解各檔案何時被讀寫：

- **建構式**：`RegisterSetupPages` 建立 3 分頁與 Exit 按鈕對應表 → `LayoutSetupButtons` 排版選單按鈕 → `BindTrayFormEvents` 綁定 6 個 Tray 欄位的 OnChange/OnExit → `BuildBinSettingUI` 建表 → `SelectSetupPage(0)` 預設顯示 Recipe 分頁。
- **FormShow**：`OpenWorkFile`（載入最後配方 → 確保配方目錄 → 載入 Tray Form → 載入 Bin Map 到表格 → 刷新清單與狀態）→ 重新排版按鈕。
- **SaveWorkFile**：確保配方目錄 → 寫 `setup.ini` → 存 Tray Form（並 `TrayForm.Load` 刷新記憶體）→ 存 Bin Map（不跳訊息）→ 寫 `manifest.ini` → 記錄最後配方名。
- **UpdateRunStateLock**（每週期由 `UpdateRunControlFlag` 呼叫）：依 `SystemStart` 啟用/禁用 `Use Recipe` 與 `Delete` 按鈕，停機後自動恢復。
- **FormClose**：呼叫 `SaveWorkFile` 自動存檔目前配方。

配方相關設定參數：

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `RecipeName` | 由使用者輸入（正規化後使用） | 配方名稱（`setup.ini [Setup]`） |
| `RecipeDir / BinAreaMap` | `data\<配方名>\...` | 配方資料夾與 Bin 對應表路徑（`setup.ini [Setup]`） |
| `Manifest.SourceRecipe / LastUpdate` | Save As 時記錄來源；其餘空字串 | 配方來源與最後更新時間（`manifest.ini [Manifest]`） |
| `GeneralSetting.bColorBinAreaInstalled` | bool | 是否安裝色彩 Bin 區，決定料區表是否含 Color 列（此畫面唯讀顯示，於 GeneralSetting 設定） |
| `GeneralSetting.iSortMode` | 0/1/2 | 分流模式（0=Normal / 1=By Lot+Bin / 2=By Lot+PassFail；此畫面唯讀使用，於 maintenance 設定） |

---

## 6.6 分流模式：Normal / By Lot+Bin / By Lot+PassFail

HT160S 把分類結果 (Bin) 導向輸出料倉（Auto1~Auto6 / Color / Error 區）有三種模式：

- **Normal 模式（預設）**：依靜態 `BinAreaMap.ini`，每個 Bin 固定對到一個 Auto 區。
- **By Lot+Bin 模式**：在 CCD 掃描時依「先到先得 (first-come-first-served)」把每組 (LotID, Bin) 綁定到下一個尚未被佔用且已啟用的 Auto，之後僅讀取此綁定來放料。
- **By Lot+PassFail 模式**：掃描時以「Bin 是否等於 Pass Bin」把 IC 分為 PASS/FAIL，把每組 (LotID, PASS/FAIL) 先到先得綁定到 Auto；每 Lot 最多綁 2 個 Auto。分類在掃描當下凍結於料格，放料時只讀不重算。

模式切換與多數硬體/路由旗標的設定畫面位於 maintenance 硬體安裝頁 (`tsMaintHardware`)；By Lot+PassFail 所需的 Pass Bin 於 Bin Setting 頁 `cbbPassBin` 設定（見 6.4）。完整說明見第 15 章，本節一併概述。

![配方/產品設定](screenshots/screen-product.png)
> 圖 6-2 Product / 硬體安裝頁的分流與路由設定（模式切換、Auto/吸嘴啟用、安全距離等）。（擷取方式：進入 Maintenance → Hardware install setup 頁。）

### 6.6.1 硬體頁分流控制項

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `rgSortMode` | radiogroup（3 選項） | 切換分流模式 Normal / By Lot+Bin / By Lot+PassFail（`ItemIndex`→`GeneralSetting.iSortMode` 0/1/2；寫入 `General.ini [SortMode] Mode`）；變更後僅提醒重開軟體，不強制；lot 執行中被鎖定 |
| `chkAutoEnable1..6` | checkbox | 逐一啟用/停用 Auto1~Auto6 輸出站（`bAutoEnabled[0..5]` / `[SortMode] AutoEnabled0..5`）；停用者於綁定新分類鍵時被跳過；變更後提醒重開；lot 執行中鎖定 |
| `chkSuckEnable1..4` | checkbox | 逐一啟用/停用 SortArm 四個吸嘴（`bSuckerEnabled[0..3]` / `[HardwareInstall] SuckerEnabled0..3`）；至少須保留一個啟用，取消最後一個時自動勾回並提示；即時生效；lot 執行中鎖定 |
| `chkHardwareColorBinArea` | checkbox | 宣告是否安裝 Color 輸出區（`bColorBinAreaInstalled`）；lot 執行中鎖定 |
| `chkUseAMR` | checkbox | 啟用 AMR/AGV 模式（`bUseAMR`）；lot 執行中鎖定 |
| `cbBinPanelType` | combobox | 選擇料倉顯示面板型號（`iBinDispPanelType`，0/1） |
| `edLoaderSafeDistance` | edit | 設定兩台 Loader-Y 車最小安全間距；以 mm 輸入經 `fQwertyKey` 鍵盤 clamp 325~650mm，YES/NO 確認後以 1/100mm 存入 `iLoaderYSafeDistance` |
| `pnlHardwareHeader` | panel | 硬體頁標題列；lot 執行中顯示鎖定提示文字（`ApplyHardwareEditLock` 依 `MachineRun.bRunning` 切換） |
| `edLotNo` | edit | 主畫面輸入/顯示 Lot 編號；fresh start 時被清空 |

### 6.6.2 設定參數

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `GeneralSetting.iSortMode` | 預設 0；`[SortMode] Mode`（另同步寫 legacy `UseLotBinMode` 供舊版降級） | 分流模式：0=Normal（靜態 BinAreaMap），1=By Lot+Bin（動態綁定 Bin），2=By Lot+PassFail（動態綁定 PASS/FAIL）。載入時無 `Mode` 鍵則以 legacy `UseLotBinMode` 推導並 clamp |
| `BinAreaMap.PassBin` | 預設 0（關閉）；配方 `BinAreaMap.ini [BinAreaMap] PassBin` | 視為 PASS 的 Bin；By Lot+PassFail 模式的分類/路由依據（啟動前須 >0） |
| `GeneralSetting.bAutoEnabled[0..5]` | 預設全 true；`[SortMode] AutoEnabled0..5` | Auto1~Auto6 是否參與分流（綁定時跳過停用者） |
| `GeneralSetting.bSuckerEnabled[0..3]` | 預設全 true，至少一個須啟用；`[HardwareInstall] SuckerEnabled0..3` | SortArm 四個吸嘴啟用旗標（FindPickCells 跳過停用槽） |
| `GeneralSetting.bColorBinAreaInstalled` | 預設 false；`[HardwareInstall] ColorBinAreaInstalled` | 是否安裝 Color 輸出區（影響 BinAreaMap Color 區可用性） |
| `GeneralSetting.bUseAMR` | 預設 false；`[HardwareInstall] UseAMR` | 是否啟用 AMR/AGV 模式 |
| `GeneralSetting.iLoaderYSafeDistance` | 預設 10000（=100mm），輸入 clamp 325~650mm；`[Safety] LoaderYSafeDistance` | 兩 Loader-Y 車最小安全間距，儲存單位 1/100mm |
| `RecipeManager.CurrentRecipeName` | 預設 `Default`；`system\lastset.ini [LastSet] RecipeName`（舊鍵 `cob_MainWorkFile`） | 目前配方/WorkFile 名稱（`data\<name>` 目錄） |
| `BinAreaMap [BinAreaMap] Auto1..Auto6/Color` | 配方目錄 `BinAreaMap.ini`；值=Bin（>0 才設定） | Normal 模式各輸出區對應的 Bin 號 |
| `BinAreaMap ErrorBinArea / [ErrorBinAreaMap]` | 預設 `HT160_DEFAULT_ERROR_BIN_AREA`；錯誤 Bin 可設 `Default` 沿用通用值 | 未知/未設定 Bin 與錯誤 Bin (2DScanFail / NoBinSetting) 的溢位目標區 |
| `LotBinBinding.ini [LotBinBinding]` | `system\LotBinBinding.ini`；每次 `ResolveAuto` 新綁定即 `SaveToIni` | 動態綁定持久化：`Mode`（寫入時的分流模式）+ Count + ItemN= LotID \x01 鍵 \x01 AutoIndex。中間「鍵」在 By Lot+Bin 是 Bin、在 By Lot+PassFail 是 PASS/FAIL 分類碼(1/2)；載入時 `Mode` 與現行模式不符即整表捨棄 |

### 6.6.3 Normal 模式：設定 Bin 對 Auto 的靜態對應

1. 在 `BinAreaMap` 中為每個輸出區（Auto1~Auto6，Color 若已安裝）各指定一個 Bin 號，存於配方目錄的 `BinAreaMap.ini [BinAreaMap]` 區段（鍵=區名 Auto1.. / Color，值=Bin）。
2. 另設定 `ErrorBinArea`（`[BinAreaMap] ErrorBinArea`）作為未知/未設定 Bin 的溢位目標；可分別為 `2DScanFail`、`NoBinSetting` 兩種錯誤 Bin 指定區（`[ErrorBinAreaMap]`，值 `Default` 表沿用通用 `ErrorBinArea`）。
3. 執行期 aSortArm 的 `GetMappedAutoIndex` 以 `BinAreaMap.GetAreaByBin(Bin)` 把 Bin 轉成 Auto index；查不到者一律導向 `ErrorBinArea`，絕不 place-anywhere（維持每盤由上到下、左到右填滿）。

### 6.6.4 啟用動態分流模式（By Lot+Bin / By Lot+PassFail）

1. （僅 By Lot+PassFail 需要）先於 Bin Setting 頁以 `cbbPassBin` 設定視為 PASS 的 Bin（存入配方 `BinAreaMap.ini`）。
2. 於 maintenance 硬體頁 `rgSortMode` 選 By Lot+Bin 或 By Lot+PassFail（`iSortMode`=1 或 2），依提示重開軟體乾淨生效。
3. 以 `chkAutoEnable1..6` 設定哪些 Auto 可用於分流（停用者於綁定時被跳過）。By Lot+PassFail 每 Lot 用到 2 個 Auto，建議至少保留 2 個非 Error Auto。
4. 載入各 Lot 的 2D/Bin 資料（離線匯入或 WebAPI/SECS），使 `LotRegistry.GetItemCount()`>0。
5. 按 Start。全新工單以 0 綁定啟動是正常的——綁定只在掃描時產生（先前「Set bindings first」的啟動阻擋已於 2026-07-01 移除，因與動態模型矛盾）。By Lot+PassFail 另檢查 Pass Bin 是否 >0，未設定會擋下並提示 `By Lot+PassFail mode is ON but no Pass Bin is set. Set the Pass Bin on the Bin Setting page before Start !`。

### 6.6.5 動態模式執行期綁定與放料

1. Loader CCD 掃描每顆 IC：`LotRegistry.FindByCode2D`(2D 碼) 反查出所屬 LotIndex 與 Bin，寫入該料格（`SetTrayBin` / `SetTrayLot` / `SetTrayCode2D`）；同時以 `BinAreaMap.GetPassFailClass(Bin)` 計算 PASS(1)/FAIL(2)/0 並凍結於料格（`SetTrayPassClass`）。
2. 依模式綁定 `LotBinBinding.ResolveAuto(LotIndex, 鍵)`：By Lot+Bin 的鍵是 Bin；By Lot+PassFail 的鍵是 PASS/FAIL 分類碼（分類碼 0=錯誤/關閉時不綁定）。先查既有綁定，無則以「先到先得」選最低 index、未被佔用且 `bAutoEnabled` 為真的非 Error Auto；全被佔用時溢位到 Error Auto。新綁定立即 `AddObject` 並 `SaveToIni()`。
3. SortArm 放料時 `GetMappedAutoIndex` 只 READ 綁定（`FindAuto`）無配置副作用；By Lot+Bin 用該格 Bin、By Lot+PassFail 用該格凍結的分類碼查表。LotIndex<0、錯誤 Bin（2D 掃描失敗/無 Bin 設定）或分類碼 0 導向 Error Auto；動態模式不使用 Color 區做分流。
4. 綁定表持久化於 `system\LotBinBinding.ini [LotBinBinding]`（`Mode` + Count + ItemN= LotID \x01 鍵 \x01 AutoIndex）。

### 6.6.6 開機繼承上一筆工單 (inherit-record)

1. 開機若無 JSON 工單，系統以 `LoadLastLotList()` + `LotBinBinding.LoadFromIni()` 還原上次手動 Lot 清單與動態綁定。
2. 若有可還原內容（Lot 數或綁定數>0），以 Yes/No 對話彈出 `Inherit last work order ? (N lots, M bindings) Yes = resume, No = start fresh`。
3. 選 `Yes`：保留 registry 與 (Lot,Bin)→Auto 綁定（等同中途重啟續跑），`RecordProcess` 記錄 inherit。
4. 選 `No`：清空整筆工單（`LotRegistry.Clear` + `LotBinBinding.Clear`+`SaveToIni` 空檔 + 清 `edLotNo` + `SaveLastLotList`），等同 Lot End 語意；累計生產計數（`ReadLastDataIni`）一律保留。

---

## 6.7 互鎖與防呆 (Interlock / Poka-yoke)

> ⚠️ 注意：以下互鎖在運轉或設定錯誤時會擋下操作，請逐項確認。

- **運轉中禁止切換/刪除配方**：機台運轉中 (`HSys.Sys.SystemStart`=true) 禁止 `Use Recipe` / `Delete`；`UpdateRunStateLock` 視覺禁用按鈕，點擊處理另有 `ShowMyOKMessageNoStop` 備援擋下。
- **不可刪除目前使用中配方**：`spbRecipeDelete` 比對名稱擋下。
- **配方名不可空白/重複**：Save As / New Blank 名稱不可空白；名稱重複擋下。
- **Tray Division 夾限**：X/Y Division 夾限於 1..`MAX_TRAY_X`(20) / 1..`MAX_TRAY_Y`(50)，超出提示並夾限後才存。
- **Bin 值範圍**：0..999（1000 起為錯誤碼）；非數字或越界視為 `Invalid`；同一 Bin 指派到不同料區視為 `Duplicate`；驗證未通過時 `Save` 被擋下。
- **By Lot+Bin 鎖編輯**：By Lot+Bin 模式時，Bin 對應表的 Bin 欄不可被選取/編輯（Auto↔Bin 由執行時動態綁定）。
- **硬體編輯鎖 (ApplyHardwareEditLock)**：Lot 執行中 (`MachineRun.bRunning`) 鎖定 `rgSortMode` / `chkAutoEnable1..6` / `chkSuckEnable1..4` / `chkHardwareColorBinArea` / `chkUseAMR`，避免中途改路由破壞進行中的分流；重啟（未起 lot）即解鎖，標題加註 locked。（By Lot+PassFail 執行中另拒絕於 Bin Setting 頁變更 Pass Bin）
- **至少一個吸嘴啟用**：`chkSuckEnableClick` 偵測歸零時自動勾回並提示 `At least one nozzle must stay enabled.`。
- **2D 碼全域唯一**：`AddItemEx` 拒絕已被任一 Lot 擁有的重複 2D 碼（回傳 false，帶出 `DupExistingLot`），避免分流衝突。
- **Start 前置檢查（防呆）**：未載入任何 2D 資料 (`GetItemCount`<=0) 擋下 Start；By Lot+Bin 模式開啟但無任何綁定時擋下 Start。
- **路由確定性**：未知/未設定 Bin 一律導向 `ErrorBinArea`，絕不 place-anywhere；`ResolveAuto` 跳過被停用的 Auto，但 Error Auto 即使自身停用仍保留為溢位目標。

---

## 6.8 訊息與提示對照

| 訊息 | 意義 | 處置 |
| --- | --- | --- |
| `Tray division exceeds machine limit (X max=.., Y max=..). Value clamped.` | Tray Form X/Y Division 超過機台上限（X=20 / Y=50），值已被夾限 | 改成不超過上限的格數後重新儲存 |
| `Bin map setting has invalid rows.` | Bin 對應表存在無效列（範圍錯/重複），存檔被拒 | 修正標示為 Invalid/Duplicate 的列再存 |
| `Bin must be 0-999. Error code starts from 1000.` | 格內 Bin 值超出 0-999 範圍（1000 起為錯誤碼） | 輸入 0..999 的有效 Bin 值 |
| `Bin map setting is OK. / Bin map saved.` | 驗證通過 / 對應表已儲存（資訊提示） | 無 |
| `Please input new recipe name.` | Save As / New Blank 未輸入配方名 | 在 Recipe Name 欄輸入名稱 |
| `Recipe already exists.` | Save As 目標配方名已存在 | 改用其他名稱 |
| `Save As recipe failed. / Create recipe failed or recipe already exists. / Delete recipe failed.` | 配方複製/建立/刪除作業失敗 | 檢查名稱與資料夾權限後重試 |
| `Can not change recipe while machine is running. / Can not delete recipe while machine is running.` | 機台運轉中不可切換或刪除配方 | 停機後再操作 |
| `Recipe does not exist.` | Use Recipe 選取的配方不存在 | 重新整理清單後選有效配方 |
| `Can not delete current recipe.` | 嘗試刪除目前使用中配方 | 先切換到其他配方再刪除 |
| `Delete recipe <name>?` | 刪除配方的 Yes/No 確認 | 選 Yes 確認，No 取消 |
| `Recipe saved as <name>.` | 另存配方成功提示 | 無 |
| `No 2D data : load lot 2D/Bin data before Start !` | Start 前未載入任何 IC 的 2D/Bin 資料 | 先以離線匯入 / WebAPI / SECS 載入再 Start |
| `By Lot+PassFail mode is ON but no Pass Bin is set. Set the Pass Bin on the Bin Setting page before Start !` | By Lot+PassFail 模式開啟但 Pass Bin=0 | 先於 Bin Setting 頁以 `cbbPassBin` 設定 Pass Bin 再 Start |
| `Pass Bin is locked while a By Lot+PassFail lot is running. Finish the lot (Lot End) before changing it.` | Lot 執行中嘗試更改 Pass Bin | 先結束目前 Lot 再變更 |
| `Sort mode changed. Please restart the software so the new classification mode takes effect cleanly.` | 切換分流模式後的提醒（非強制） | 重新啟動軟體讓新模式乾淨生效 |
| `Auto enable changed. Please restart the software so the new Lot+Bin routing takes effect cleanly.` | 變更 Auto 啟用後的提醒（非強制） | 重新啟動軟體讓新路由乾淨生效 |
| `At least one nozzle must stay enabled.` | 嘗試停用最後一個吸嘴 | 系統自動勾回；保留至少一個啟用的吸嘴 |
| `Inherit last work order ? (N lots, M bindings) Yes = resume, No = start fresh` | 開機詢問是否繼承上次工單 | Yes 續跑保留綁定；No 清空整筆工單重新開始 |

---

## 6.9 設定檔位置摘要

| 檔案 | 位置 | 內容 |
| --- | --- | --- |
| `setup.ini` | `data\<配方名>\` | `[Setup]` 配方名/路徑、`[TrayForm]` 料盤幾何 |
| `BinAreaMap.ini` | `data\<配方名>\` | `[BinAreaMap]` Area→Bin、`[ErrorBinAreaMap]` 錯誤 Bin 溢位區 |
| `manifest.ini` | `data\<配方名>\` | `[Manifest]` 來源配方、最後更新時間 |
| `General.ini` | `system\`（GeneralSetting） | `[SortMode]`、`[HardwareInstall]`、`[Safety]` 等硬體/分流旗標 |
| `lastset.ini` | `system\` | `[LastSet] RecipeName` 目前使用中的配方名（舊鍵 `cob_MainWorkFile`） |
| `LotBinBinding.ini` | `system\` | `[LotBinBinding]` By Lot+Bin 動態綁定持久化 |
| `lastdata.dat` | `system\` | 累計生產統計 (`tRunData`) |

> 註：GeneralSetting（色彩 Bin / Lot+Bin 模式等）對設定畫面而言為唯讀使用；其實際設定畫面在 GeneralSetting / maintenance 硬體頁，而非 Product Setup 畫面。

---

## 6.10 待補事項

> 本章下列項目無法由現有來源規格確定，標記為待補，須由現場或跨檔確認後補上：

- 【待補】`TrayForm.XStart/XPitch/YStart/YPitch` 的工程單位（mm 或 1/100mm）：設定畫面只做 `ReadFloat/WriteFloat`，實際單位需看 TrayForm 結構與 Loader/SortArm 使用端確認（備忘錄記載位置類多為 1/100mm，但此處為配方幾何浮點值）。
- 【待補】各 Area（Auto1..Auto6, Color）對應實體出料站/料盒的物理位置，與 enum 編號間隙（`eHT160BinAreaAuto1`=3..`Auto6`=8, `Color`=9，1/2 用途）需由 `CosFunction.h`/機構定義確認，本畫面未說明。
- 【待補】畫面中文標籤：設定畫面可見文字皆為英文，未出現 Big5 中文標籤；硬體頁部分控制項標籤（`rgSortMode`、`chkAutoEnable*`、`chkSuckEnable*`、`chkHardwareColorBinArea`、`chkUseAMR`、`cbBinPanelType`、`edLotNo`）實際螢幕 Caption 需以 .dfm 或執行畫面確認。
- 【待補】`SYSTEM_BIN_SELECT BinSelect[2]`（`iCategData`/`bStackDefFailCate`/`bCategoryFail`/`iCategoryFailCountLimit`/`iOpenBin`）在 cprod 內僅宣告未見讀寫，其填值來源與是否仍在使用（可能為舊機型遺留）需確認。
- 【待補】Error Auto 的決定（`GetErrorBinArea` / `GetErrorAutoIndex`）依 `BinAreaMap.ErrorBinArea`；若 Error 區設為 Color（非 Auto）時 By Lot+Bin 會 fallback 到最後一個 Auto，此邊界行為的實機意圖需確認。
- 【待補】By Lot+Bin 採「最低 index 先到先得」綁定；是否提供操作員手動指定/編輯特定 (Lot,Bin)→Auto 綁定的 UI，未在現有來源確認。
- 【待補】是否存在獨立於 Product Setup 之外、供新增/複製/刪除/切換配方的另一配方選單畫面（form 名稱與按鈕）未在現有來源完全確認。
