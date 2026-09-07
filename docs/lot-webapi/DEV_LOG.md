# HT160S Lot WebAPI 功能 — 開發紀錄

> 目的：HT160S Handler 一次用一個 Lot 名稱，透過 WebAPI (HTTP GET) 取得該 Lot 對應的
> 2D / Bin 資料，灌入既有 `THT160LotRegistry` 反查機制供分選使用。
> 本檔為長期維護紀錄，討論本功能時可快速回顧背景與決策。

---

## 1. 背景與現況 (FACT，2026/6/11 實地查證)

### HT160 端通訊能力
- `THT160AutomationServer` (`AutomationServer.cpp/.h`)：raw winsock **TCP server**（被動監聽
  16060，ASCII 行協議 CRLF）。方向相反（等別人連入），**不可直接用於主動拉取**。
- `THT160TopCcdSocket` (`TopCcdSocket.cpp/.h`)：raw winsock **client**（非阻塞輪詢、無
  Sleep / 無 FSM）。**HTTP client 以此為範本**。
- `cJSON` 已移植進 `Include\`，可解析 JSON。

### 既有 2D/Bin 資料層 (已建好)
- `THT160Bin2DMap`（forward：LotNumber+Code2D → Bin）。
- `THT160LotRegistry` / `TLotRunInfo`（reverse：2D → LotID/Bin，支援多 Lot）。
- 既有內部 JSON 格式：`{"Maps":[{"LotNumber","SourceMachine","Items":[{"Code2D","Bin"}]}]}`，
  來源 `<CurrentDir>\HT160S_LotInfo\yyyymm\dd\*.json`。

### 客戶真實 JSON 格式 (待 WebAPI 對接)
```json
{ "2DIDHistory": [
  { "LOTID": "...", "Substage": "...", "ProductCode": "...",
    "ICIInfo": [
      { "QRCodeID": "...", "RetestCode": "R0", "HBin": "1", "SBin": "1", "DiePass": "1" }
    ] } ] }
```
- 根 `2DIDHistory`；Lot 層 `LOTID / Substage / ProductCode`；IC 層
  `QRCodeID / RetestCode / HBin / SBin / DiePass`。
- HBin / SBin 皆為**字串**。

---

## 2. 已定案決策 (user 2026/6/11)

| 項目 | 決策 |
|------|------|
| 分選 Bin | **用 SBin**（HBin 仍儲存備用） |
| 模擬器架構 | WebAPI 獨立檔 `lot_webapi_server.py`，透過 bat 啟動 |
| 拉取時機 | (a) LotStart 自動拉 (b) Maintenance 手動鈕；自動拉走 SECS S2F42 LOTSTART |
| SECS 自動拉參考 | HT9045 899 版 `HT9045Gem::S2F42_Host_Command_Acknowledge` 的 `else if(S.AnsiPos("LOTSTART")==1)` |
| 風險注意 | 手動流程的彈出式異常訊息**不得阻塞 SECS 通訊**（自動拉路徑須無 modal dialog） |
| 額外欄位 | Substage / ProductCode / RetestCode / DiePass **全部儲存備用**；Clear 時全部清乾淨 |
| 真實 API 規格 | 未定案，先用模擬器；WebAPI 路徑由 TfMaintenance `edWebapiPath` 設定（需存讀） |
| WebAPI Log | 需有，結構參考 SECS / Top CCD 2D log（依日期檔，append） |

---

## 3. 作戰計畫 6 階段

| 階段 | 內容 | 狀態 |
|------|------|------|
| 0 | 規格確認 | ✅ |
| 1 | 模擬器 WebAPI server (`lot_webapi_server.py`) | ✅ |
| 2 | HT160 HTTP client 模組 + edWebapiPath 存讀 + WebAPI log | ✅ |
| 3 | 客戶 JSON 結構擴充（TLotRunInfo + per-IC 欄位；相容 2DIDHistory；SBin 當分選 Bin） | ✅ |
| 4 | 接機台流程（btnLotStart 手動 + SECS S2F42 LOTSTART 自動，非阻塞） | ✅ |
| 5 | 實機/真實 API 驗收 | 進行中 (2026/6/24 客戶端點 + 回傳格式已確認相容，零程式改動；待實機 Fetch 驗證 — 見第 9 節) |

---

## 4. 階段 1 完成內容 (2026/6/11)

- 新檔 `D:\AI_Area\Tool\HT160S_SECS_Simulator\code\lot_webapi_server.py`
  - `LotDataStore`（thread-safe，LOTID→Lot 物件索引，精確後不分大小寫比對）
  - `LotWebApiServer`（`ThreadingHTTPServer` + daemon thread）
  - Endpoints：`/health`、`/lots`、`/lot/<LOTID>`（回 `{"2DIDHistory":[lot]}` 或 404）、`/reload`
  - 預設 port 8160
- 資料檔 `lot_webapi_data.json`（客戶 2DIDHistory 格式，可編輯，內含樣本 Lot）
- `secs_host_simulator.py` 接線：`import LotWebApiServer`(optional)、GUI「Start/Stop WebAPI」鈕、
  `--webapi` / `--webapi-port`(default 8160)、CLI 也支援。獨立 thread/port，不碰 HSMS。
- bat：`start_lot_webapi.bat`（獨立啟動 WebAPI server）。
- 驗證：4 endpoint 實測 OK；`py_compile` 兩檔通過。

---

## 5. 階段 2 內容 (2026/6/11 完成)

### 新增模組 `LotWebApiClient.cpp/.h`（HT160S_Program_BCB_V1.0.0.0）
- class `THT160LotWebApiClient`，raw winsock **非阻塞 HTTP/1.0 GET client**，以
  `TopCcdSocket.cpp` 為範本（無 Sleep / 無 FSM / 無 busy-wait）。
- 狀態 enum `TLotWebApiState`：IDLE / CONNECTING / SENDING / RECEIVING / DONE / FAILED。
- 公開 API：`LoadConfig` / `SaveConfig` / `SetBaseUrl` / `GetBaseUrl` /
  `StartLotRequest(LotID)` / `GetResult(Body&,bOk&,HttpStatus&)` / `Poll` / `IsBusy` /
  `Cancel` / `GetLastError` / `GetCurrentLot`。
- 內部：`ParseBaseUrl`（拆 host:port + path，預設 port 80）、`UrlEncodeLot`（百分比編碼）、
  `BeginConnect` / `PollConnecting`(select) / `PollSending`(partial-send safe) /
  `PollReceiving`(recv 到 peer FIN) / `FinishResponse`（解析 status line + CRLFCRLF/LFLF
  切 header/body，status 200 且有 body → `bRequestOk=true`）/ `IsTimedOut`
  （用 `Delta.operator double()*86400.0` 避開 BCB6 TDateTime cast ambiguity）。
- 逾時：`LOTWEBAPI_TIMEOUT_SEC=8`。預設 BaseUrl=`http://127.0.0.1:8160/lot/`。
- 全域單例：`extern THT160LotWebApiClient *LotWebApiClient; void EnsureLotWebApiClientCreated();`。

### 設定 (ship + 安裝層)
- `system\General.ini` `[LotWebApi] BaseUrl`，由 `LoadConfig`/`SaveConfig` 存讀。

### WebAPI Log
- `SaveWebApiLog()` → `<CurrentDir>\WebAPI_Logs\yyyy_mm_dd_log.txt`（依日期 append，
  結構參考 Top CCD 2D log）。

### Maintenance 介面接線（`maintenance.cpp/.h`）
- 新增 maintenance 頁 **Lot WebAPI**（`spbMaintLotApi` / `tsMaintLotApi`），由
  `BuildLotWebApiPage()` 動態建立；於 `RegisterMaintenancePages()` PageDefs 註冊
  （TopCcd 與 SECS 之間）。
- **URL 欄位重用既有 DFM `edWebapiPath`**（user 預先拉在 Function Define → Network 子頁，
  `pgcFunctionDef/tsNetwork/Panel4`）。**未重複宣告**，避免 E2238 multiple declaration。
- Lot WebAPI 頁內容：URL echo 標籤 (`lblLotApiUrl`)、Save / Reload 鈕（寫/讀
  `[LotWebApi] BaseUrl`）、測試 Lot 輸入 (`edLotApiTestLot`)、Fetch 鈕、狀態列
  (`lblLotApiStatus` / `lblLotApiError`)、結果 Memo (`memLotApiResult`)、Log Memo
  (`memLotApiLog`)。
- 非阻塞流程：`btnLotApiFetchClick` 呼 `StartLotRequest`，設 `bLotApiResultPending=true`；
  既有 `tmrTowerLightBlinkTimer`（300ms）新增 `RefreshLotWebApiStatus()` 輪詢
  `GetResult`，完成時填結果 Memo + log（**一次性消費**，無 modal dialog）。
- ctor：NULL-init 新成員、`BuildLotWebApiPage()`、`LoadLotWebApiSettings()` +
  `EnsureLotWebApiClientCreated()`。
- 重要：DFM 控制項 `edWebapiPath` **不可在 ctor 設 NULL**（會清掉 VCL streaming 指標）。
### 專案接線 (`ht160s.bpr`)
- OBJFILES 加 `LotWebApiClient.obj`（接在 `TopCcdSocket.obj ColorCcdSocket.obj` 後）。
- FILELIST 加 `<FILE FILENAME="LotWebApiClient.cpp" ... UNITNAME="LotWebApiClient"
  CONTAINERID="CCompiler".../>`。

### Build 驗證
- `scripts\ops\build-ht160s.ps1 -Clean`：encoding check 153 檔通過、**0 編譯/連結錯誤**、
  `EXE\HT160S.exe` 重新連結成功、`LotWebApiClient.obj` 產出並連入。
- 已知踩雷修正：初次建置因重複宣告 `edWebapiPath`（DFM 已有）→ E2238 連鎖到
  ht160s/main/csystem/maintenance；移除重複宣告 + 改用 DFM 欄位後全清。


---

## 6. 階段 3 內容 (2026/6/11 完成)

> 目標：擴充資料層儲存所有客戶備用欄位、相容客戶 `2DIDHistory` JSON 格式，並以
> SBin 作為分選 routing Bin（HBin 仍儲存備用）。全部編輯在 ASCII 檔（`CosFunction.h/.cpp`），
> 用編輯器工具安全處理。Clean build 0 錯誤。

### `CosFunction.h`
- 新增分選來源常數：`#define HT160_SORT_BIN_SOURCE_SBIN 0` / `HT160_SORT_BIN_SOURCE_HBIN 1`。
- `HT160S_CUSTOMER_FUNCTION` 加 `int iSortBinSource;`（控制 routing Bin 取 SBin 或 HBin）。
- `TLotRunInfo` 加備用欄位 `AnsiString sSubstage; AnsiString sProductCode;`，`Clear()` 一併清空。
- 新增 per-IC 備用結構 `TLotIcInfo`：`sCode2D / sLotID / iBin / iHBin / iSBin /
  sRetestCode / sDiePass`。
- `THT160LotRegistry` 加：private `TStringList *m_Code2DInfo;`（與 `m_Code2DIndex`
  平行的備用記錄表，Objects 持 `TLotIcInfo*` 堆指標）、private `void FreeAllIcInfo();`；
  public `AddItemEx(LotID,Code2D,Bin,HBin,SBin,RetestCode,DiePass,DupExistingLot&)` 與
  `FindIcInfo(Code2D, TLotIcInfo&)`。

### `CosFunction.cpp`
- ctor：`m_Code2DInfo` 建為 `Sorted=true / Duplicates=dupIgnore / CaseSensitive=true`。
- dtor：`FreeAllIcInfo(); delete m_Code2DInfo; delete m_Code2DIndex;`（先釋放堆物件再刪表）。
- `FreeAllIcInfo()`：逐筆 `delete (TLotIcInfo*)Objects[i]` 再 `Clear()`。
- `Clear()`：在 `m_Code2DIndex->Clear()` 後呼 `FreeAllIcInfo()` → **Clear 時全部清乾淨**
  （含 per-IC 堆物件，無記憶體洩漏）。
- `RemoveLot()`：除清反查 index，新增迴圈刪除 + 釋放該 Lot 名下所有 `TLotIcInfo`。
- `AddItem()`：改為委派 `AddItemEx(...,Bin,Bin,Bin,"","",...)`（舊 Maps/offline 路徑無備用
  欄位，HBin/SBin 鏡射 Bin）。
- `AddItemEx()`：2D code 全域唯一性檢查（重複回 false + 帶出既有 Lot 名）；成功才
  同步寫入 `m_Code2DIndex`（packed LotIndex+routingBin）與 `m_Code2DInfo`（堆 `TLotIcInfo`），
  `iPlanQty++`。
- `FindIcInfo()`：由 2D code 反查備用記錄，複製到 out 參數。
- `LoadFromJsonFile()`：在既有 `Maps` 區塊後新增 **`2DIDHistory`** 解析區塊——
  逐 Lot 讀 `LOTID / Substage / ProductCode`（寫入 `TLotRunInfo`），逐 IC 讀
  `QRCodeID / RetestCode / HBin / SBin / DiePass`，HBin/SBin 字串以 `StrToIntDef(...,0)`
  轉 int，依 `CosFunction.iSortBinSource` 取 routingBin（預設 SBin），呼 `AddItemEx`；
  重複碼比照 Maps 區塊設 `bHasDuplicate` / `FirstDupCode`。兩格式共用同一檔可並存。
- `InitialCosFunction()`：加 `CosFunction.iSortBinSource=HT160_SORT_BIN_SOURCE_SBIN;`（預設 SBin）。

### Build 驗證
- `scripts\ops\build-ht160s.ps1 -Clean`：encoding check 153 檔通過、**0 編譯/連結錯誤**、
  `EXE\ht160s.exe` 於 14:29 重新連結成功。
- 修正：一次 `multi_replace` 誤把 `LoadLatest` 函式簽章吞掉，已補回 `//---` 分隔 +
  簽章行；重建確認無誤。

---

## 7. 階段 4 內容 (2026/6/11 完成)

> 目標：把 WebAPI 拉取接進機台流程——(a) 手動 LotStart、(b) SECS S2F42 LOTSTART
> 自動拉。全程非阻塞、無 modal dialog（自動路徑跑在 HSMS/VCL 接收緒，彈窗會卡死
> SECS 通訊）。所有編輯都在 ASCII 檔，編輯器工具安全。Clean build 0 錯誤，
> `EXE\ht160s.exe` 於 17:05 重新連結成功。

### 資料層字串直載 (`CosFunction.h/.cpp`)
- 新增 `THT160LotRegistry::LoadFromJsonString(Json, bHasDuplicate&, FirstDupCode&)`：
  把原 `LoadFromJsonFile` 的 cJSON 解析整段抽出（同時相容 `Maps` 與 `2DIDHistory`）。
- `LoadFromJsonFile` 改為「讀檔 → 呼 `LoadFromJsonString`」薄包裝。WebAPI 回應 body
  是 JSON 字串（非檔案），可直接灌入 registry。

### HTTP client 自動拉開關 (`LotWebApiClient.h/.cpp`)
- 新增 private `bool bUsePull`（ctor 預設 false）、public `GetUsePull` / `SetUsePull`。
- `LoadConfig` / `SaveConfig` 存讀 `[LotWebApi] UsePull`（`ReadBool` / `WriteBool`）。
- **預設關閉**：真實客戶 API 接好前，機台流程不會嘗試連 127.0.0.1:8160（避免無謂連線）。

### 機台流程接線 (`main.h/main.cpp`)
- 新成員 `bool bLotApiPullActive; AnsiString sLotApiPullLot;`（ctor NULL/false 初始化）。
- `RequestLotDataFromWebApi(LotID)`：`EnsureLotWebApiClientCreated` → 若 client busy 則
  跳過（不堆疊請求）→ `StartLotRequest`，設 `bLotApiPullActive=true`。**全程 RecordProcess
  記錄、無 modal**（SECS 路徑共用）。
- `PollLotDataWebApi()`：每個 MainProc cycle 驅動；若有 in-flight 拉取，`Poll()` +
  `GetResult()`，完成時若成功 `LotRegistry.LoadFromJsonString(Body,...)` →
  `RefreshLotListFromRegistry()`，否則記 log。一次性消費、無 modal。
- `csystem.cpp MainProc()`：在 `ShowMotorInfo()` 後加 `if(fMain!=NULL)
  fMain->PollLotDataWebApi();`（MainProc 走 `TRunControl::Synchronize`，VCL 主緒安全）。
- `btnLotStartClick`：建好 Lot 清單後，若 `LotWebApiClient->GetUsePull()` 才呼
  `RequestLotDataFromWebApi(FirstLot)`（gated、非阻塞）。

### SECS S2F42 LOTSTART 分支 (`SecsGem/uHGemHT160.cpp`)
- 在 `ONLINE_LOCAL` 與 unknown-command else 之間新增 `else if(S.AnsiPos("LOTSTART")==1)`。
- 讀 inner L[n] 的 ASCII Lot id（鏡射 SET_LOT_INFO 讀法，但**additive 不 Clear**）；
  producing/IC-inside → HCACK=4 busy；空清單 → HCACK=2。
- 成功時：`fMain->edLotNo->Text=FirstLot` → `RefreshLotListFromRegistry()` →
  `RequestLotDataFromWebApi(FirstLot)`（async、無 modal）。
- **保守安全決策**：LOTSTART 只設定 active lot + 觸發資料拉取，**不自動啟動機台運動**；
  運動啟動仍由操作員 Start 鈕把關（safety-critical，不從 SECS 自動按 Start）。

### Maintenance UsePull 開關 (`maintenance.cpp/.h`)
- Lot WebAPI 頁新增 `chkLotApiUsePull` 勾選框（ctor NULL-init、`BuildLotWebApiPage` 建立）。
- `LoadLotWebApiSettings` 讀回勾選狀態、`SaveLotWebApiSettings` 一併 `SetUsePull` 並存檔。

### Build 驗證
- `scripts\ops\build-ht160s.ps1 -Clean`：encoding check 153 檔通過、**0 編譯/連結錯誤**、
  `EXE\ht160s.exe` 於 17:05 重新連結成功。

---

## 8. 風險與待確認

- 真實客戶 WebAPI 的 URL / port / 方法 / 認證尚未定案 → 先模擬。
- SECS 自動拉取必須走「無 modal dialog」路徑，避免阻塞 HSMS 執行緒。
- 階段 5（待辦）：實機 / 真實 API 驗收（需客戶 URL / port / 方法 / 認證）。
- UsePull 預設 OFF；實機接好客戶 API 後，於 Maintenance Lot WebAPI 頁勾選 + Save 啟用。

---

## 9. 階段 5 — 真實 API 對接 (2026/6/24 客戶端點確認)

> 客戶提供真實 WebAPI 端點與一份回傳樣本。經逐欄位核對，回傳即既有已支援的
> `2DIDHistory` 格式，**解析器零改動**即可端到端運作。本節記錄端點、設定方式與
> 實機驗證清單。

### 9.1 真實端點 (客戶提供)
```
http://192.168.11.18:7825/api/GetBISummaryByLot?OSATLot=<LotID>
```
- IP 直連 (內網 192.168.11.18，port 7825)，`inet_addr` 可直接解析，無需 DNS。
- 範例 `?OSATLot=內批` 的「內批」為**佔位字**，實際填真實 Lot 號 (樣本為純 ASCII
  `A5921.RCS.TEST99`)。

### 9.2 設定方式 (Maintenance → Lot WebAPI 頁，operator config，非程式改動)
- 本機請求模型為 `BaseUrl + UrlEncodeLot(LotID)`：LotID **接在 BaseUrl 尾端**。
- 故 `[LotWebApi] BaseUrl` (edWebapiPath) 要設成**結尾停在 `?OSATLot=` 的前綴**：
  ```
  http://192.168.11.18:7825/api/GetBISummaryByLot?OSATLot=
  ```
  **不可**把含 Lot 值的完整網址整串貼入 (會變成 `...OSATLot=內批<再接一次LotID>`)。
- `ParseBaseUrl` 把 host 後整段 (含 `?OSATLot=`) 當 path 送出，GET 行正確；
  Lot 號 ASCII 經 `UrlEncodeLot` 原樣輸出。最終：
  `GET /api/GetBISummaryByLot?OSATLot=A5921.RCS.TEST99 HTTP/1.0`。

### 9.3 回傳格式確認 = `2DIDHistory` (既有支援，零 parser 改動)
客戶樣本與 `THT160LotRegistry::LoadFromJsonString` 的 `2DIDHistory` 解析區塊
(`CosFunction.cpp` ~1262-1341) 逐欄位吻合：

| 客戶欄位 | 程式讀取 | 用途 |
|------|------|------|
| `2DIDHistory`[] / `LOTID` | 根陣列 / `AddLot` | Lot |
| `Substage` / `ProductCode` | TLotRunInfo 備用欄位 | 儲存 |
| `ICIInfo`[] / `QRCodeID` | IC 陣列 / 2D 反查 key | 分選 key |
| `HBin` / `SBin` (字串) | `StrToIntDef` 轉 int | routing Bin |
| `RetestCode` / `DiePass` | 備用欄位 | 儲存 |

- routing Bin **預設取 `HBin`** (2026/6/24 user 指示，`InitialCosFunction()` 改為
  `iSortBinSource=HT160_SORT_BIN_SOURCE_HBIN`，CosFunction.cpp:1659)；SBin 仍解析儲存
  備用。原 6/11 決策為 SBin (見第 2 節)，已由此覆寫。無 ini / per-customer 覆寫，改此
  一行即全域切換。build EXIT=0。

### 9.4 實機 Fetch 驗證清單 (在機台跑，開發筆電無法連客戶內網)
1. Maintenance → Lot WebAPI：`edWebapiPath` 填 9.2 前綴版，按 **Save**。
2. 測試 Lot 欄填真實 Lot 號，按 **Fetch** (手動、無 modal、不需開 UsePull)。
3. 結果 Memo 應顯示 HTTP 200 + JSON body；確認 Lot 清單反查表有灌入。
4. 仍需在實機確認的殘留點：
   - **HTTP 版本**：本機送 HTTP/1.0 + `Connection: close`，靠 peer FIN 判定收完。
     若 server 強回 HTTP/1.1 **chunked** 編碼，目前 parser 不會 de-chunk → body 含
     chunk size 行、cJSON 解析失敗。需以 Fetch 實測確認 server 回 1.0/非 chunked。
   - **嚴格 JSON**：cJSON 不接受**尾逗號** (樣本圖中 `"DiePass":"1",}` 的逗號為人工
     示意)。確認 server 實際輸出為合法 JSON。
   - **中文 Lot 編碼**：`UrlEncodeLot` 是對 Big5 bytes 做百分比編碼；目前真實 Lot 為
     ASCII 故無影響。若日後出現含中文 Lot 號且 server 期待 UTF-8，需另行調整。
5. 驗證 OK 後，於同頁勾選 **UsePull** + Save，啟用 LotStart / SECS S2F42 LOTSTART 自動拉。

### 9.5 文件同步修正 (程式已超前 DEV_LOG 之處)
- **整批拉取 + 重試**：階段 4 原記「只拉第一個 Lot」；現行 `main.cpp` 已是整批掃描
  (`StartLotWebApiPullAll` / `StartNextLotApiPull`)，逐 Lot 拉、含 cursor，單 Lot 失敗
  重試 `LOT_API_MAX_RETRY=3` 次才跳過，不靜默漏 Lot。手動 LotStart 與 SECS LOTSTART
  共用此掃描。
- **WebAPI Log 路徑**：第 5 節舊記 `WebAPI_Logs\yyyy_mm_dd_log.txt`；現已改用
  `cCsvDailyLog` → `WebAPI\YYYYMMDD\WebAPI_YYYYMMDD.log` (會進 State Record 快照)。
