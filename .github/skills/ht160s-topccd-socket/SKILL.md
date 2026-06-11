---
name: ht160s-topccd-socket
description: >-
  Use when working on the HT160S_BCB Top CCD 2D barcode camera client
  (THT160TopCcdSocket in TopCcdSocket.cpp/.h): raw non-blocking winsock client,
  LON trigger protocol, EnsureTopCcdSocketCreated global lifecycle, LoadConfig
  reading system\General.ini [TopCCD], maintenance-form Top CCD page wiring, and
  the aLoader ReadTopCcd2DCode integration. Triggers: TopCcdSocket,
  THT160TopCcdSocket, EnsureTopCcdSocketCreated, LoadConfig, SetEndpoint,
  TopCcdTriggerShot, TopCcdGetResult, TopCcdConnect, TopCcdPoll, LON, General.ini
  TopCCD, edTopCcdIP, LoadTopCcdSettings, ReadTopCcd2DCode.
---

# HT160S Top CCD Socket (2D Camera Client)

## 1. 目的

Top CCD 相機在 Loader 站拍照取得每顆 IC 的 2D code。HT160S 用一個
**獨立、非 FSM、非阻塞的 raw winsock client** 與相機溝通：送 `"LON"` 觸發拍照，
相機回傳 2D code 字串（trim 後存入 `sTopCcd2D`）。

由舊版 HT160S `TfSetup::ClientSocket1` 改寫而來（rename map 見 TopCcdSocket.h 檔頭）。

## 2. 核心物件與生命週期

| 元件 | 位置 | 說明 |
|------|------|------|
| `THT160TopCcdSocket` | `TopCcdSocket.cpp/.h` | 相機 client 類別 |
| `THT160TopCcdSocket *TopCcdSocket` | 全域指標，初值 `NULL` | 單例 |
| `void EnsureTopCcdSocketCreated()` | 自由函式 | `if(TopCcdSocket==NULL){ new + LoadConfig(); }` |

**開機建立時機（重要）**：`TopCcdSocket` 不在 WinMain 直接建立，而是由
`fMaintenance` 建構子呼叫 `EnsureTopCcdSocketCreated()`。因為 `ht160s.cpp` WinMain
在開機時 `Application->CreateForm(__classid(TfMaintenance), &fMaintenance)` 會把
維護表單一次建好，所以 socket 在開機就已建立、config 已讀入。

`HSys.CurrentDir`（= `".."`）在全域 `HSys` 建構時就設好，早於任何表單，所以
`LoadConfig()` 在 fMaintenance 建構子執行時讀 `system\General.ini` 路徑是正確的。

## 3. 設定（General.ini [TopCCD]）

`LoadConfig()` 讀 `HSys.CurrentDir + "\\system\\General.ini"` 的 `[TopCCD]` 區：

| Key | 預設 | 對應成員 |
|-----|------|---------|
| `Address` | `DEFAULT_TOPCCD_ADDRESS` (172.16.8.89) | `sCcdAddress` |
| `Port` | `DEFAULT_TOPCCD_PORT` (5001) | `iCcdPort`（範圍檢查 1..65535）|

- `SetEndpoint(addr, port)`：執行期覆寫（空字串 / 非法 port 會被忽略）。
- `GetAddress()` / `GetPort()`：讀回目前端點。

維護表單另有兩條讀寫路徑（注意 socket config 與 UI 欄位是**兩份**）：
- `TfMaintenance::LoadTopCcdSettings()` → 讀 General.ini 填 `edTopCcdIP/edTopCcdPort` **UI 欄位**。
- `TfMaintenance::SaveTopCcdSettings()` → 寫 General.ini，並 `SetEndpoint()` 同步 socket。

> 陷阱：建構子若只呼叫 `EnsureTopCcdSocketCreated()`（socket LoadConfig），
> 而**沒**呼叫 `LoadTopCcdSettings()`，則 socket 端點正確但維護頁 IP/Port 欄位
> 仍顯示 `BuildTopCcdPage` 的預設值，要等進頁 Reload 才同步。修法：建構子在
> `EnsureTopCcdSocketCreated()` 前補一行 `LoadTopCcdSettings();`。

## 4. 連線狀態機（TTopCcdState）

```
TOPCCD_IDLE(0) -> TOPCCD_CONNECTING(1) -> TOPCCD_CONNECTED(2)
```

非阻塞契約：**無 Sleep / busy-wait**。呼叫端 `TopCcdTriggerShot()` 後，在自己的
週期迴圈裡輪詢 `TopCcdGetResult()` 直到回 `true`。

## 5. 公開 API

| 方法 | 用途 |
|------|------|
| `LoadConfig()` | 讀 General.ini [TopCCD] |
| `SetEndpoint(addr,port)` | 覆寫端點 |
| `TopCcdConnect()` / `TopCcdDisconnect()` | 連線控制 |
| `IsTopCcdConnected()` | 是否已連線 |
| `TopCcdTriggerShot()` | 送 `"LON"`，清 read flag |
| `TopCcdGetResult(AnsiString &code)` | 收到回覆回 `true` + code |
| `TopCcdPoll()` | 驅動 socket 狀態機（connecting / receive）|
| `GetLastError()` | 最後錯誤字串 |

私有：`StartWinsock` / `CloseSocket` / `PollConnecting` / `PollReceive` /
`SendTopCcdCmd` / `SaveTopCcd2DLog`。

## 6. 維護表單 Top CCD 頁

`BuildTopCcdPage()` 建：`edTopCcdIP` / `edTopCcdPort` / `edTopCcdResult` /
`lblTopCcdStatusConn` / `lblTopCcdStatusError` / `memTopCcdLog`，按鈕：
`btnTopCcdSave`（SaveTopCcdSettings）/ `btnTopCcdReload`（LoadTopCcdSettings +
LoadConfig）/ `btnTopCcdConnect` / `btnTopCcdDisconnect` / `btnTopCcdShot`。
`RefreshTopCcdStatus()` 週期 `TopCcdPoll()` + 顯示連線/錯誤 + 取 `TopCcdGetResult`。
`GetTopCcdIniFileName()` → `RootPath\system\General.ini`。

## 7. Loader 整合（aLoader）

`TLoaderModule::ReadTopCcd2DCode(LoaderNo, CellX, CellY, bool &bOk)`：
- 模擬模式（`tSimuData.bRunSimulation`）→ 不碰硬體，從 `LotRegistry` 輪循虛擬碼
  （見 skill `ht160s-simulation-data`）。
- 正式模式 → `if(TopCcdSocket!=NULL) TopCcdSocket->TopCcdTriggerShot();` 後輪詢
  `TopCcdGetResult()`。`DoCcdCheck` state 5000 觸發、5500 取結果 →
  `LotRegistry.FindByCode2D()` 指派 Bin。

> state 5000 的 `TopCcdTriggerShot()` 必須用 `if(TopCcdSocket!=NULL)` 包住，
> 否則無硬體（模擬）時會 crash 或卡在 5000。

## 8. 注意事項

- 純 winsock（`<winsock.h>`），解構式 `WSACleanup()`；`sckTopCcd` 初值
  `INVALID_SOCKET`。
- 不要把 socket I/O 放進阻塞迴圈；維持「trigger 後輪詢」模式。
- 端點設定屬 **General tier**（出貨 + 硬體安裝），不要塞進 Recipe/Config。
