# 第 15 章　By Lot+Bin 分流模式

本章說明 HT160S 專屬的「By Lot+Bin」動態分流模式：依 (LotID, Bin) 組合在掃描時以先到先得（first-come-first-served）方式綁定到輸出料倉 Auto1~Auto6，之後僅讀取此綁定來放料。內容涵蓋此模式的用途、適用時機、逐一啟用各 Auto 的設定、Lot 執行中的硬體編輯鎖，以及開機繼承上一筆工單的提示。

> 配方（Recipe / WorkFile）整體資料模型與 Normal 模式的靜態 Bin->Auto 對應，請見第 6 章；Lot 的 2D/Bin 資料載入與 Lot 分頁操作，請見第 4 章。

---

## 15.1 兩種分流模式概觀

HT160S 把 CCD 分類結果（Bin）導向輸出料倉，提供兩種模式：

| 模式 | 行為 | 設定來源 |
| --- | --- | --- |
| Normal（預設） | 依靜態 `BinAreaMap.ini` 把每個 Bin 固定對到一個 Auto 區，全程不變 | 配方目錄 `BinAreaMap.ini` |
| By Lot+Bin（動態） | 掃描時依先到先得，把每組 (LotID, Bin) 綁定到下一個尚未被佔用且已啟用的 Auto，之後僅讀取此綁定放料 | `General.ini [SortMode]` + `system\LotBinBinding.ini` |

兩種模式共同的路由原則：**未知/未設定的 Bin 一律導向 Error Auto（`ErrorBinArea`），絕不 place-anywhere**，以維持每盤由上到下、左到右填滿的確定性。

### 何時使用 By Lot+Bin 模式

- 同一批工單中含多個 Lot，且需依「Lot 加 Bin」的組合動態分倉，而非固定 Bin->Auto 對應時。
- 各 Lot 的 Bin 分佈事先未知、希望依實際掃描到的組合自動佔用空閒 Auto 時。

> ⚠️ 注意：By Lot+Bin 模式不使用 Color 區做分流；分流目標僅限 Auto1~Auto6 與 Error Auto。

---

## 15.2 設定畫面位置

By Lot+Bin 的模式切換與各 Auto 啟用，位於 maintenance 的硬體安裝頁（`tsMaintHardware`）；Lot 編號與工單清單則在主畫面的 Lot 分頁。

![Lot 分頁與分流設定](screenshots/main-lot.png)
> 圖 15-1 主畫面 Lot 分頁與分流設定。（擷取方式：開啟主畫面，切換至 Lot 分頁；模式與 Auto 啟用勾選位於 maintenance 硬體安裝頁 `tsMaintHardware`）

---

## 15.3 相關控制項

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `chkUseLotBinMode` | checkbox | 切換分流模式 Normal <-> By Lot+Bin（寫入 `GeneralSetting.bUseLotBinSortMode` / `General.ini [SortMode] UseLotBinMode`）。OnClick 後僅以提示訊息建議重開軟體讓新模式乾淨生效，不強制；Lot 執行中被硬體編輯鎖鎖定 |
| `chkAutoEnable1`..`chkAutoEnable6` | checkbox | 逐一啟用/停用 Auto1~Auto6 輸出站（`GeneralSetting.bAutoEnabled[0..5]` / `[SortMode] AutoEnabled0..5`）。停用的 Auto 在綁定新 (LotID,Bin) 時被跳過；變更後建議重開軟體；Lot 執行中鎖定 |
| `chkSuckEnable1`..`chkSuckEnable4` | checkbox | 逐一啟用/停用 SortArm 四個吸嘴（`GeneralSetting.bSuckerEnabled[0..3]` / `[HardwareInstall] SuckerEnabled0..3`）。至少須保留一個啟用，取消最後一個時自動勾回並提示；即時生效不需重開；Lot 執行中鎖定 |
| `chkHardwareColorBinArea` | checkbox | 宣告是否安裝 Color 輸出區（`GeneralSetting.bColorBinAreaInstalled`），影響 `BinAreaMap` 中 Color 區是否可用；Lot 執行中鎖定 |
| `chkUseAMR` | checkbox | 啟用 AMR/AGV 模式（`GeneralSetting.bUseAMR`）；Lot 執行中鎖定 |
| `cbBinPanelType` | combobox | 選擇料倉顯示面板型號（`GeneralSetting.iBinDispPanelType`，0/1） |
| `edLoaderSafeDistance` | edit | 設定兩台 Loader-Y 車最小安全間距，以 mm 輸入經 `fQwertyKey` 鍵盤 clamp 325~650mm，YES/NO 確認後以 1/100mm 存入 `GeneralSetting.iLoaderYSafeDistance` |
| `pnlHardwareHeader` | panel | 硬體頁標題列；Lot 執行中顯示鎖定提示文字（`ApplyHardwareEditLock` 依 `MachineRun.bRunning` 切換） |
| `edLotNo` | edit | 主畫面輸入/顯示 Lot 編號；fresh start 時被清空 |

---

## 15.4 相關參數

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `GeneralSetting.bUseLotBinSortMode` | 預設 false；`[SortMode] UseLotBinMode` | 分流模式：false=Normal（靜態 `BinAreaMap`），true=By Lot+Bin（動態綁定） |
| `GeneralSetting.bAutoEnabled[0..5]` | 預設全 true；`[SortMode] AutoEnabled0..5` | Auto1~Auto6 是否參與分流（By Lot+Bin 綁定時跳過停用者） |
| `GeneralSetting.bSuckerEnabled[0..3]` | 預設全 true，至少一個須啟用；`[HardwareInstall] SuckerEnabled0..3` | SortArm 四個吸嘴啟用旗標（`FindPickCells` 跳過停用槽） |
| `GeneralSetting.bColorBinAreaInstalled` | 預設 false；`[HardwareInstall] ColorBinAreaInstalled` | 是否安裝 Color 輸出區（影響 `BinAreaMap` Color 區可用性） |
| `GeneralSetting.bUseAMR` | 預設 false；`[HardwareInstall] UseAMR` | 是否啟用 AMR/AGV 模式 |
| `GeneralSetting.iLoaderYSafeDistance` | 預設 10000(=100mm)，輸入 clamp 325~650mm；`[Safety] LoaderYSafeDistance` | 兩 Loader-Y 車最小安全間距，儲存單位 1/100mm |
| `BinAreaMap ErrorBinArea / [ErrorBinAreaMap]` | 預設 `HT160_DEFAULT_ERROR_BIN_AREA`；錯誤 Bin 可設 `Default` 沿用通用值 | 未知/未設定 Bin 與錯誤 Bin（2DScanFail/NoBinSetting）的溢位目標區 |
| `LotBinBinding.ini [LotBinBinding]` | `system\LotBinBinding.ini`；每次 `ResolveAuto` 新綁定即 `SaveToIni` | By Lot+Bin 動態綁定持久化：`Count` + `ItemN= LotID \x01 Bin \x01 AutoIndex` |

---

## 15.5 如何設定一筆 Lot+Bin 分流

以下步驟說明從啟用模式到首次產生綁定的完整流程。

1. 進入 maintenance 硬體安裝頁（`tsMaintHardware`）。
2. 勾選 `chkUseLotBinMode`，使 `bUseLotBinSortMode=true`。系統會提示建議重開軟體；依提示重新啟動軟體讓新模式乾淨生效。
3. 以 `chkAutoEnable1`..`chkAutoEnable6` 設定哪些 Auto 可用於分流。停用的 Auto 於綁定時會被跳過。變更後系統會提示建議重開軟體。
4. 確認至少保留一個吸嘴啟用（`chkSuckEnable1`..`chkSuckEnable4`）；若嘗試停用最後一個，系統會自動勾回並提示。
5. 回到主畫面，依第 4 章載入各 Lot 的 2D/Bin 資料（離線匯入或 WebAPI/SECS），使 `LotRegistry` 中項目數大於 0。
6. 按 Start。系統會做兩道前置檢查：
   - 是否已載入任何 2D 資料；未載入會擋下並提示 `No 2D data : load lot 2D/Bin data before Start !`。
   - By Lot+Bin 模式下是否已存在至少一筆綁定；若無會擋下並提示 `By Lot+Bin mode is ON but no binding is set. Set bindings first !`。
7. 開始運轉後，Loader CCD 掃描每顆 IC 時即依先到先得自動產生 (LotID, Bin)->Auto 綁定（詳見 15.6）；綁定一旦建立即寫入 `system\LotBinBinding.ini`。

> ⚠️ 注意：步驟 6 的綁定來自掃描。首次以全新工單啟動時，必須先載入 2D/Bin 資料並讓系統掃描出綁定，或先設定綁定，否則 Start 會被擋下。

---

## 15.6 執行期綁定與放料

啟用 By Lot+Bin 後，綁定在掃描時建立、放料時僅讀取：

1. Loader CCD 掃描每顆 IC：`LotRegistry.FindByCode2D(2D 碼)` 反查出所屬 `LotIndex` 與 `Bin`，寫入該料格（`SetTrayBin` / `SetTrayLot` / `SetTrayCode2D`）。
2. 若 `bUseLotBinSortMode` 為真：呼叫 `LotBinBinding.ResolveAuto(LotIndex, Bin)`。先查既有綁定；無則以先到先得選**最低 index、未被佔用且 `bAutoEnabled` 為真**的非 Error Auto；全被佔用時溢位到 Error Auto。新綁定立即 `AddObject` 並 `SaveToIni()` 持久化。
3. SortArm 放料時 `GetMappedAutoIndex` 只 READ 綁定（`FindAuto`），無任何配置副作用。`LotIndex<0` 或錯誤 Bin（2D 掃描失敗 / 無 Bin 設定）導向 Error Auto。
4. 綁定表持久化於 `system\LotBinBinding.ini [LotBinBinding]`（`Count` + `ItemN= LotID \x01 Bin \x01 AutoIndex`）。

> ⚠️ 互鎖（路由確定性）：未知/未設定 Bin 一律導向 `ErrorBinArea`，絕不 place-anywhere。
>
> ⚠️ 互鎖（2D 碼全域唯一）：`AddItemEx` 拒絕已被任一 Lot 擁有的重複 2D 碼（回傳 false，帶出 `DupExistingLot`），避免分流衝突。
>
> ⚠️ 互鎖（停用 Auto 跳過）：`ResolveAuto` 在綁定時跳過 `bAutoEnabled==false` 的 Auto；Error Auto 即使自身停用，仍保留為溢位目標。

---

## 15.7 硬體編輯鎖（Lot 執行中）

為避免運轉中改動路由破壞進行中的分流，系統在 Lot 執行時鎖定硬體頁的路由相關勾選框。

- 觸發條件：`MachineRun.bRunning==true`（Lot Start 設、Lot End 清）。
- 行為：`ApplyHardwareEditLock` 鎖定下列控制項，並在 `pnlHardwareHeader` 標題加註鎖定提示文字（locked - lot running, end lot to edit）。
  - `chkUseLotBinMode`
  - `chkAutoEnable1`..`chkAutoEnable6`
  - `chkSuckEnable1`..`chkSuckEnable4`
  - `chkHardwareColorBinArea`
  - `chkUseAMR`
- 解鎖：結束 Lot（未起 lot）後即解鎖，可重新編輯。

> ⚠️ 注意：Lot 執行中無法切換分流模式或改動 Auto 啟用。若需調整路由，請先結束目前 Lot。

---

## 15.8 開機繼承上一筆工單（inherit-record prompt）

開機若無 JSON 工單，系統會嘗試還原上次的手動 Lot 清單與動態綁定，並詢問是否繼承。

1. 開機以 `LoadLastLotList()` + `LotBinBinding.LoadFromIni()` 還原上次的手動 Lot 清單與 (Lot,Bin)->Auto 綁定。
2. 若有可還原內容（Lot 數或綁定數大於 0），彈出 YES/NO 詢問：`Inherit last work order ? (N lots, M bindings) Yes = resume, No = start fresh`。
3. 選 **Yes**：保留 registry 與綁定（等同中途重啟續跑），並以 `RecordProcess` 記錄 inherit。
4. 選 **No**：清空整筆工單（`LotRegistry.Clear` + `LotBinBinding.Clear` + 以空檔 `SaveToIni` + 清 `edLotNo` + `SaveLastLotList`），等同 Lot End 語意。

> ⚠️ 注意：無論選 Yes 或 No，由 `ReadLastDataIni` 載入的累計生產計數（`tRunData`）一律保留，不受工單繼承選擇影響。

---

## 15.9 開機與執行流程摘要

1. 開機 `InitialCosFunction`：載入付費旗標 -> `GeneralSetting.Load(General.ini)` -> `Config.Load` -> `RecipeManager.LoadLastRecipeName` + `EnsureCurrentRecipeDir` -> `BinAreaMap.LoadDefault`（讀目前配方的 `BinAreaMap.ini`）-> `TrayForm.Load(setup.ini)`。
2. 主畫面 `ReadLastDataIni` 載入累計統計（`system\lastdata.dat`）；還原 `LastLotList` 與 `LotBinBinding.ini` -> 若有內容則彈出 inherit-record 提示（Yes 續跑 / No 清空）。
3. 資料載入：每顆 IC 以 `LotRegistry.AddItem/AddItemEx(LotID,2D,Bin)` 註冊；2D 碼全域唯一，重複者回傳 false 並帶出 `DupExistingLot`，不覆蓋。
4. Start 前置檢查：`LotRegistry.GetItemCount()>0`；若 By Lot+Bin 模式則需綁定數大於 0。
5. 掃描階段（aLoader）：`FindByCode2D` 反查 `LotIndex`/`Bin` -> 寫料格 -> （By Lot+Bin）`ResolveAuto` 先到先得綁定 -> `OnSorted` 計數。
6. 放料階段（aSortArm）：`GetMappedAutoIndex` 依模式查 Auto；Normal=`GetAreaByBin`，LotBin=`FindAuto`（唯讀）。
7. 硬體頁變更 `chkUseLotBinMode` / `chkAutoEnable*`：提示建議重開軟體；變更於 `GeneralSetting` 即時記憶並 `Save()` 至 `General.ini`。

---

## 15.10 提示與互鎖訊息一覽

| 訊息 | 意義 | 處置 |
| --- | --- | --- |
| `No 2D data : load lot 2D/Bin data before Start !` | Start 前未載入任何 IC 的 2D/Bin 資料 | 先以離線匯入 / WebAPI / SECS 載入 Lot 2D/Bin 資料再 Start |
| `By Lot+Bin mode is ON but no binding is set. Set bindings first !` | By Lot+Bin 模式開啟但尚無任何綁定 | 先掃描產生綁定或設定綁定後再 Start |
| `Sort mode changed. Please restart the software so the new classification mode takes effect cleanly.` | 切換分流模式後的提醒（非強制） | 重新啟動軟體讓新模式乾淨生效 |
| `Auto enable changed. Please restart the software so the new Lot+Bin routing takes effect cleanly.` | 變更 Auto 啟用後的提醒（非強制） | 重新啟動軟體讓新路由乾淨生效 |
| `At least one nozzle must stay enabled.` | 嘗試停用最後一個吸嘴 | 系統自動勾回；保留至少一個啟用的吸嘴 |
| `Inherit last work order ? (N lots, M bindings) Yes = resume, No = start fresh` | 開機詢問是否繼承上次工單 | Yes 續跑保留綁定；No 清空整筆工單重新開始 |

---

## 15.11 待補項目

> 【待補：是否提供操作員手動指定/編輯特定 (Lot,Bin)->Auto 綁定的 UI。`ResolveAuto` 採「最低 index 先到先得」自動綁定，本批檔案未見手動編輯介面，需確認。】

> 【待補：當 Error 區設為 Color（非 Auto）時，By Lot+Bin 會 fallback 到最後一個 Auto 的邊界行為與實機意圖，需現場確認。】

> 【待補：配方「選單畫面」（若有獨立 form 供新增/複製/刪除/切換配方）對應的表單名稱與按鈕，本批檔案未確認；詳見第 6 章。】
