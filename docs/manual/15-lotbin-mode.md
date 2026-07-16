# 第 15 章　動態分流模式（By Lot+Bin / By Lot+PassFail）

本章說明 HT160S 專屬的兩種「動態分流」模式，兩者都在掃描時以先到先得（first-come-first-served）方式把分類鍵綁定到輸出料倉 Auto1~Auto6，之後僅讀取此綁定放料：

- **By Lot+Bin**：分類鍵 = (LotID, Bin)，一個 Lot 可綁多個 Auto（依不同 Bin）。
- **By Lot+PassFail**：分類鍵 = (LotID, PASS/FAIL)，PASS/FAIL 由「Bin 是否等於設定的 Pass Bin」導出，一個 Lot 最多綁 2 個 Auto（PASS 一個、FAIL 一個）。

內容涵蓋兩模式的用途、適用時機、逐一啟用各 Auto 的設定、Lot 執行中的硬體編輯鎖，以及開機繼承上一筆工單的提示。

> 配方（Recipe / WorkFile）整體資料模型與 Normal 模式的靜態 Bin->Auto 對應、以及 Pass Bin（`cbbPassBin`）的設定，請見第 6 章；Lot 的 2D/Bin 資料載入與 Lot 分頁操作，請見第 4 章。

---

## 15.1 三種分流模式概觀

HT160S 把 CCD 分類結果（Bin）導向輸出料倉，提供三種模式（互斥，於 maintenance 硬體頁的 `rgSortMode` 三選一）：

| 模式 | `iSortMode` | 行為 | 設定來源 |
| --- | --- | --- | --- |
| Normal（預設） | 0 | 依靜態 `BinAreaMap.ini` 把每個 Bin 固定對到一個 Auto 區，全程不變 | 配方目錄 `BinAreaMap.ini` |
| By Lot+Bin（動態） | 1 | 掃描時依先到先得，把每組 (LotID, Bin) 綁定到下一個尚未被佔用且已啟用的 Auto，之後僅讀取此綁定放料 | `General.ini [SortMode]` + `system\LotBinBinding.ini` |
| By Lot+PassFail（動態） | 2 | 掃描時把每顆 IC 分為 PASS（Bin==Pass Bin）或 FAIL（其他），依先到先得把每組 (LotID, PASS/FAIL) 綁定到 Auto，之後僅讀取此綁定放料 | `General.ini [SortMode]` + 配方 `BinAreaMap.ini [BinAreaMap] PassBin` + `system\LotBinBinding.ini` |

三種模式共同的路由原則：**未知/未設定的 Bin 一律導向 Error Auto（`ErrorBinArea`），絕不 place-anywhere**，以維持每盤由上到下、左到右填滿的確定性。

### 何時使用動態模式

- **By Lot+Bin**：同一批工單含多個 Lot，且需依「Lot 加 Bin」的組合動態分倉，而非固定 Bin->Auto 對應；各 Lot 的 Bin 分佈事先未知、希望依實際掃描到的組合自動佔用空閒 Auto 時。
- **By Lot+PassFail**：只需把每個 Lot 的良品（Pass Bin）與非良品分成兩倉時。每 Lot 最多佔 2 個 Auto，適合良/不良二分的收料需求。

> ⚠️ 注意：動態模式不使用 Color 區做分流；分流目標僅限 Auto1~Auto6 與 Error Auto。

---

## 15.2 設定畫面位置

分流模式切換與各 Auto 啟用，位於 maintenance 的硬體安裝頁（`tsMaintHardware`）的 `rgSortMode` 選擇框；By Lot+PassFail 所需的 **Pass Bin** 則在 setup 的 Bin Setting 頁以 `cbbPassBin` 設定（見第 6 章）。Lot 編號與工單清單則在主畫面的 Lot 分頁。

![Lot 分頁與分流設定](screenshots/main-lot.png)
> 圖 15-1 主畫面 Lot 分頁與分流設定。（擷取方式：開啟主畫面，切換至 Lot 分頁；模式選擇位於 maintenance 硬體安裝頁 `tsMaintHardware` 的 `rgSortMode`；Pass Bin 位於 setup Bin Setting 頁）

---

## 15.3 相關控制項

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `rgSortMode` | radiogroup（3 選項） | 選擇分流模式：Normal / By Lot+Bin / By Lot+PassFail（`ItemIndex` 對應 `GeneralSetting.iSortMode` 0/1/2，寫入 `General.ini [SortMode] Mode`）。OnClick 後僅以提示訊息建議重開軟體讓新模式乾淨生效，不強制；Lot 執行中被硬體編輯鎖鎖定 |
| `cbbPassBin` | combobox（setup Bin Setting 頁） | 選擇哪個 Bin 視為 PASS（"0"=關閉 + 各已設定的路由 Bin）。存入配方 `BinAreaMap.ini [BinAreaMap] PassBin`。By Lot+PassFail 模式的分類即以此為準；詳見第 6 章 |
| `chkAutoEnable1`..`chkAutoEnable6` | checkbox | 逐一啟用/停用 Auto1~Auto6 輸出站（`GeneralSetting.bAutoEnabled[0..5]` / `[SortMode] AutoEnabled0..5`）。停用的 Auto 在綁定新分類鍵時被跳過；變更後建議重開軟體；Lot 執行中鎖定 |
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
| `GeneralSetting.iSortMode` | 預設 0（Normal）；`[SortMode] Mode` | 分流模式：0=Normal（靜態 `BinAreaMap`），1=By Lot+Bin，2=By Lot+PassFail。以 `IsLotBinSortMode()`/`IsLotPassFailSortMode()`/`IsDynamicBindingMode()` 判讀 |
| `[SortMode] UseLotBinMode`（legacy） | 由程式維護，不需手動改 | 向後相容鍵：載入時若無 `Mode` 鍵則以此推導（true→1）；存檔時同步寫 `(iSortMode==1)`，供舊版 exe 降級讀取（Normal 與 PassFail 都對映為 Normal） |
| `BinAreaMap.PassBin` | 預設 0（關閉）；配方 `BinAreaMap.ini [BinAreaMap] PassBin` | 視為 PASS 的 Bin 編號。0=關閉（Production_Log PassFail 欄留白）。By Lot+PassFail 模式下為分類/路由依據，且啟動前必須 >0 |
| `GeneralSetting.bAutoEnabled[0..5]` | 預設全 true；`[SortMode] AutoEnabled0..5` | Auto1~Auto6 是否參與分流（動態綁定時跳過停用者） |
| `GeneralSetting.bSuckerEnabled[0..3]` | 預設全 true，至少一個須啟用；`[HardwareInstall] SuckerEnabled0..3` | SortArm 四個吸嘴啟用旗標（`FindPickCells` 跳過停用槽） |
| `GeneralSetting.bColorBinAreaInstalled` | 預設 false；`[HardwareInstall] ColorBinAreaInstalled` | 是否安裝 Color 輸出區（影響 `BinAreaMap` Color 區可用性） |
| `GeneralSetting.bUseAMR` | 預設 false；`[HardwareInstall] UseAMR` | 是否啟用 AMR/AGV 模式 |
| `GeneralSetting.iLoaderYSafeDistance` | 預設 10000(=100mm)，輸入 clamp 325~650mm；`[Safety] LoaderYSafeDistance` | 兩 Loader-Y 車最小安全間距，儲存單位 1/100mm |
| `BinAreaMap ErrorBinArea / [ErrorBinAreaMap]` | 預設 `HT160_DEFAULT_ERROR_BIN_AREA`；錯誤 Bin 可設 `Default` 沿用通用值 | 未知/未設定 Bin 與錯誤 Bin（2DScanFail/NoBinSetting）的溢位目標區 |
| `LotBinBinding.ini [LotBinBinding]` | `system\LotBinBinding.ini`；每次 `ResolveAuto` 新綁定即 `SaveToIni` | 動態綁定持久化：`Mode`（寫入時的分流模式）+ `Count` + `ItemN= LotID \x01 鍵 \x01 AutoIndex`。中間「鍵」在 By Lot+Bin 是 Bin、在 By Lot+PassFail 是 PASS/FAIL 分類碼（1/2）。載入時若 `Mode` 與目前模式不符即整表捨棄（避免鍵語意被誤讀） |

---

## 15.5 如何設定一筆 By Lot+Bin 分流

1. 進入 maintenance 硬體安裝頁（`tsMaintHardware`）。
2. 於 `rgSortMode` 選 **By Lot+Bin**（`iSortMode=1`）。系統會提示建議重開軟體；依提示重新啟動軟體讓新模式乾淨生效。
3. 以 `chkAutoEnable1`..`chkAutoEnable6` 設定哪些 Auto 可用於分流。停用的 Auto 於綁定時會被跳過。變更後系統會提示建議重開軟體。
4. 確認至少保留一個吸嘴啟用（`chkSuckEnable1`..`chkSuckEnable4`）；若嘗試停用最後一個，系統會自動勾回並提示。
5. 回到主畫面，依第 4 章載入各 Lot 的 2D/Bin 資料（離線匯入或 WebAPI/SECS），使 `LotRegistry` 中項目數大於 0。
6. 按 Start。系統會檢查 Lot 編號、是否已載入任何 2D 資料、以及目前 Lot 是否有可路由的 2D/Bin；未通過會擋下並提示（見 15.11）。
7. 開始運轉後，Loader CCD 掃描每顆 IC 時即依先到先得自動產生 (LotID, Bin)->Auto 綁定（詳見 15.7）；綁定一旦建立即寫入 `system\LotBinBinding.ini`。

> ℹ️ 說明：全新工單以 0 綁定啟動是正常的——綁定只在掃描時產生（先前「Set bindings first」的啟動阻擋已於 2026-07-01 移除，因它與動態模型矛盾）。

---

## 15.6 如何設定一筆 By Lot+PassFail 分流

By Lot+PassFail 把每顆 IC 依「是否為 Pass Bin」分成 PASS / FAIL 兩類，各自綁一個 Auto。

1. 先在 setup 的 Bin Setting 頁設定 **Pass Bin**：`cbbPassBin` 選擇哪個 Bin 視為 PASS（見第 6 章），存檔寫入配方 `BinAreaMap.ini`。
2. 進入 maintenance 硬體安裝頁，於 `rgSortMode` 選 **By Lot+PassFail**（`iSortMode=2`），依提示重開軟體。
3. 以 `chkAutoEnable*` 設定可用 Auto（每 Lot 最多用到 2 個，建議至少保留 2 個非 Error Auto 啟用）。
4. 載入各 Lot 的 2D/Bin 資料（同 15.5 步驟 5）。
5. 按 Start。除了 15.5 的一般前置檢查外，**若 Pass Bin 未設定（=0），啟動會被擋下**並提示 `By Lot+PassFail mode is ON but no Pass Bin is set...`——因為此時每顆都會被判為 FAIL 而擠進單一 Auto。
6. 運轉後：CCD 掃描每顆 IC 時，以 `Bin==Pass Bin` 判定 PASS(1)/FAIL(2)，**在掃描當下凍結**此分類到料格（不在放料時重算），並依先到先得綁定 (LotID, PASS/FAIL)->Auto。

> ⚠️ 互鎖（Pass Bin 執行中鎖定）：By Lot+PassFail 的 Lot 一旦開始（已產生綁定），於 Bin Setting 頁存檔時會**拒絕變更 Pass Bin** 並提示，以免重新分割進行中的 Lot、造成 PASS/FAIL 混料。需調整請先結束 Lot（Lot End）。
>
> ⚠️ 說明（分類凍結）：PASS/FAIL 由可變的 Pass Bin 導出，與 Bin（IC 內生、不變）不同，因此在掃描當下凍結於料格（`iPassClass`，比照 `iBin`/`iLot`/`sCode2D` 的搬運），放料端只讀不重算，確保掃描時與放料時算出同一個桶。
>
> ⚠️ 說明（Production_Log 同步）：`Production_Log` 的 `PassFail` 欄改讀此凍結分類，因此紀錄的 PASS/FAIL 與 IC 實際落點一致。

---

## 15.7 執行期綁定與放料

啟用動態模式後，綁定在掃描時建立、放料時僅讀取：

1. Loader CCD 掃描每顆 IC：`LotRegistry.FindByCode2D(2D 碼)` 反查出所屬 `LotIndex` 與 `Bin`，寫入該料格（`SetTrayBin` / `SetTrayLot` / `SetTrayCode2D`）。同時以 `BinAreaMap.GetPassFailClass(Bin)` 計算 PASS/FAIL 分類並凍結（`SetTrayPassClass`）。
2. 依模式綁定：
   - **By Lot+Bin**：`LotBinBinding.ResolveAuto(LotIndex, Bin)`。
   - **By Lot+PassFail**：分類碼 >0（非錯誤/非關閉）時 `LotBinBinding.ResolveAuto(LotIndex, 分類碼)`；分類碼 0（錯誤 Bin 或 Pass Bin 關閉）則不綁定，放料時導向 Error Auto。
   - 先查既有綁定；無則以先到先得選**最低 index、未被佔用且 `bAutoEnabled` 為真**的非 Error Auto；全被佔用時溢位到 Error Auto。新綁定立即 `AddObject` 並 `SaveToIni()` 持久化。
3. SortArm 放料時 `GetMappedAutoIndex` 只 READ 綁定（`FindAuto`），無任何配置副作用。By Lot+Bin 用該格的 Bin、By Lot+PassFail 用該格凍結的 PASS/FAIL 分類查表。`LotIndex<0`、錯誤 Bin（2D 掃描失敗 / 無 Bin 設定）、或分類碼 0 一律導向 Error Auto。
4. 綁定表持久化於 `system\LotBinBinding.ini [LotBinBinding]`（`Mode` + `Count` + `ItemN= LotID \x01 鍵 \x01 AutoIndex`）。

> ⚠️ 互鎖（路由確定性）：未知/未設定 Bin 一律導向 `ErrorBinArea`，絕不 place-anywhere。
>
> ⚠️ 互鎖（2D 碼全域唯一）：`AddItemEx` 拒絕已被任一 Lot 擁有的重複 2D 碼（回傳 false，帶出 `DupExistingLot`），避免分流衝突。
>
> ⚠️ 互鎖（停用 Auto 跳過）：`ResolveAuto` 在綁定時跳過 `bAutoEnabled==false` 的 Auto；Error Auto 即使自身停用，仍保留為溢位目標。
>
> ⚠️ 互鎖（跨模式綁定檔）：`LotBinBinding.ini` 帶有寫入時的 `Mode` 標頭；載入時若與目前模式不符即整表捨棄，避免 Bin 與 PASS/FAIL 分類碼被互相誤讀。

---

## 15.8 硬體編輯鎖（Lot 執行中）

為避免運轉中改動路由破壞進行中的分流，系統在 Lot 執行時鎖定硬體頁的路由相關控制項。

- 觸發條件：`MachineRun.bRunning==true`（Lot Start 設、Lot End 清）。
- 行為：`ApplyHardwareEditLock` 鎖定下列控制項，並在 `pnlHardwareHeader` 標題加註鎖定提示文字（locked - lot running, end lot to edit）。
  - `rgSortMode`（分流模式選擇框）
  - `chkAutoEnable1`..`chkAutoEnable6`
  - `chkSuckEnable1`..`chkSuckEnable4`
  - `chkHardwareColorBinArea`
  - `chkUseAMR`
- 另外，By Lot+PassFail 執行中，setup Bin Setting 頁的 **Pass Bin** 變更也會被拒絕（見 15.6）。
- 解鎖：結束 Lot（未起 lot）後即解鎖，可重新編輯。

> ⚠️ 注意：Lot 執行中無法切換分流模式或改動 Auto 啟用。若需調整路由，請先結束目前 Lot。

---

## 15.9 開機繼承上一筆工單（inherit-record prompt）

開機若無 JSON 工單，系統會嘗試還原上次的手動 Lot 清單與動態綁定，並詢問是否繼承。

1. 開機以 `LoadLastLotList()` + `LotBinBinding.LoadFromIni()` 還原上次的手動 Lot 清單與綁定（載入時會依 `Mode` 標頭檢查模式一致性，見 15.7）。
2. 若有可還原內容（Lot 數或綁定數大於 0），彈出 YES/NO 詢問：`Inherit last work order ? (N lots, M bindings) Yes = resume, No = start fresh`。
3. 選 **Yes**：保留 registry 與綁定（等同中途重啟續跑），並以 `RecordProcess` 記錄 inherit。
4. 選 **No**：清空整筆工單（`LotRegistry.Clear` + `LotBinBinding.Clear` + 以空檔 `SaveToIni` + 清 `edLotNo` + `SaveLastLotList`），等同 Lot End 語意。

> ⚠️ 注意：無論選 Yes 或 No，由 `ReadLastDataIni` 載入的累計生產計數（`tRunData`）一律保留，不受工單繼承選擇影響。

---

## 15.10 開機與執行流程摘要

1. 開機 `InitialCosFunction`：載入付費旗標 -> `GeneralSetting.Load(General.ini)`（讀 `[SortMode] Mode`，無則以 legacy `UseLotBinMode` 推導並 clamp）-> `Config.Load` -> `RecipeManager.LoadLastRecipeName` + `EnsureCurrentRecipeDir` -> `BinAreaMap.LoadDefault`（讀目前配方的 `BinAreaMap.ini`，含 `PassBin`）-> `TrayForm.Load(setup.ini)`。
2. 主畫面 `ReadLastDataIni` 載入累計統計（`system\lastdata.dat`）；還原 `LastLotList` 與 `LotBinBinding.ini` -> 若有內容則彈出 inherit-record 提示（Yes 續跑 / No 清空）。
3. 資料載入：每顆 IC 以 `LotRegistry.AddItem/AddItemEx(LotID,2D,Bin)` 註冊；2D 碼全域唯一，重複者回傳 false 並帶出 `DupExistingLot`，不覆蓋。
4. Start 前置檢查（`CheckLotDataReady`）：需有 Lot 編號、有已載入的 2D 資料、且目前 Lot 有可路由的 2D/Bin；By Lot+PassFail 模式另需 `PassBin>0`。
5. 掃描階段（aLoader）：`FindByCode2D` 反查 `LotIndex`/`Bin` -> 寫料格 + 凍結 PASS/FAIL 分類 -> 依模式 `ResolveAuto`（Bin 或分類碼）先到先得綁定 -> `OnSorted` 計數。
6. 放料階段（aSortArm）：`GetMappedAutoIndex` 依模式查 Auto；Normal=`GetAreaByBin`，By Lot+Bin=`FindAuto(Bin)`（唯讀），By Lot+PassFail=`FindAuto(凍結分類碼)`（唯讀）。
7. 硬體頁變更 `rgSortMode` / `chkAutoEnable*`：提示建議重開軟體；變更於 `GeneralSetting` 即時記憶並 `Save()` 至 `General.ini`。

---

## 15.11 提示與互鎖訊息一覽

| 訊息 | 意義 | 處置 |
| --- | --- | --- |
| `Please Enter LotID !` | Start 前未輸入 Lot 編號 | 先於主畫面輸入 Lot 編號 |
| `No 2D data : load lot 2D/Bin data before Start !` | Start 前未載入任何 IC 的 2D/Bin 資料 | 先以離線匯入 / WebAPI / SECS 載入 Lot 2D/Bin 資料再 Start |
| `No 2D data for lot <LotID> : load this lot's 2D/Bin before Start !` | 目前作用 Lot 本身無 2D 資料 | 載入該 Lot 的 2D/Bin 資料再 Start |
| `By Lot+PassFail mode is ON but no Pass Bin is set. Set the Pass Bin on the Bin Setting page before Start !` | By Lot+PassFail 模式開啟但 Pass Bin=0 | 先於 setup Bin Setting 頁以 `cbbPassBin` 設定 Pass Bin 再 Start |
| `Pass Bin is locked while a By Lot+PassFail lot is running. Finish the lot (Lot End) before changing it.` | Lot 執行中嘗試更改 Pass Bin | 先結束目前 Lot 再變更 Pass Bin |
| `Sort mode changed. Please restart the software so the new classification mode takes effect cleanly.` | 切換分流模式後的提醒（非強制） | 重新啟動軟體讓新模式乾淨生效 |
| `Auto enable changed. Please restart the software so the new Lot+Bin routing takes effect cleanly.` | 變更 Auto 啟用後的提醒（非強制） | 重新啟動軟體讓新路由乾淨生效 |
| `At least one nozzle must stay enabled.` | 嘗試停用最後一個吸嘴 | 系統自動勾回；保留至少一個啟用的吸嘴 |
| `Inherit last work order ? (N lots, M bindings) Yes = resume, No = start fresh` | 開機詢問是否繼承上次工單 | Yes 續跑保留綁定；No 清空整筆工單重新開始 |

---

## 15.12 補充定案與備註

> 註（定案）：**不提供**手動指定/編輯單筆 (Lot,Bin)→Auto 或 (Lot,PASS/FAIL)→Auto 綁定的 UI。綁定唯一產生點是 Loader CCD 掃碼路徑的 `LotBinBinding.ResolveAuto`（最低 index 先到先得）；UI 端僅讀取顯示（主畫面 Auto 標籤、State Record dump、SECS BinSetting），僅工單清除時整組 Clear。

> ℹ️ By Lot+PassFail 多 Lot 併行時，可用非 Error Auto 少於「2 × 併行 Lot 數」會使某個 PASS/FAIL 桶溢位到 Error Auto。客戶接受此溢位行為（不跳 Note、不擋料），但溢位的每顆合法 PASS/FAIL 產品會於 `Production_Log` 以 `TraceCode=1004`、`ErrorType=PFOverflow` 記錄（PassFail 欄仍為 PASS/FAIL、Which Auto 為 Error Auto），以便追溯混入 Error Auto 的合格品。注意：此處 FAIL 是「合法產品等級（Bin）」，與 2D 讀碼失敗的 Error（走 Error Auto、無 PassFail 記錄）是兩回事。

> 註（定案）：Error 區設為非 Auto（如 Color）時，動態模式 fallback 由 `GetErrorAutoIndex`（CosFunction.cpp）處理：註解明寫 "Error area is not an Auto (e.g. Color) : fall back to the last Auto"，回傳 **Auto6**（index 5）；`ResolveAuto` 滿載溢流亦用同一 Error Auto。此為程式定義行為；若現場希望 Error 走 Color 實體站，需另開發（Color 收 IC 的 `DoSortBin` 目前為未實作之預留介面，見第 14 章）。

> 註（定案）：**無獨立配方選單 form**。配方新增/複製/刪除/切換全在 Product Setup 畫面（`setup.cpp`：`lstRecipe`＋`edRecipeName`，禁刪現用配方），另主畫面 `cb_WorkFile` 可直接切換配方；後端為 `THT160RecipeManager`（`data\<配方>\setup.ini`）。
