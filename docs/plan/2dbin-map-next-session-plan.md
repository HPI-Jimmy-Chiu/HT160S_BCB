# HT160S_BCB 2D 對帳本 — 後續作戰計畫（Top CCD 取碼 / Maintenance / P3 啟用）

作者：JimmyChiu
日期：2026/06/04
適用版本：`HT160S_Program_BCB_V1.0.0.0`（HT160S_BCB 主開發樹）
狀態：**待 Jimmy 確認**。確認後可直接複製本檔最末「新工作階段啟動提示」開新 session 施作。

---

## 0. 目前進度（已完成、已 build 驗證）

| 批次 | 內容 | 狀態 |
|------|------|------|
| Batch 1 | P1 iBin 配套（`TMyTray::iBin` + SetTrayBin/GetTrayBin）、P2 cJSON 移植、P2 `THT160Bin2DMap`/`Bin2DMap`/`bUse2DBinMap`、P5 文件 | ✅ Clean build |
| Batch 2 | P3 接線（`ReadTopCcd2DCode` stub + `CurrentLotNumber`/`SetCurrentLotNumber` + `DoCcdCheck` case 5000 旗標保護查表），**inert 不影響現狀** | ✅ Clean build |
| Batch 3 | **P4** SortArm 依 2D 查到的 bin 路由（`Slot.BinValue` + `GetSlotRoutingBin` → `BinAreaMap.GetAreaByBin`），**inert 不影響現狀** | ✅ Clean build |
| 工作包 A | **TopCCD socket 模組** `TopCcdSocket.cpp/.h`（raw winsock、非阻塞、非 FSM；改名落實；bpr 已接線），**未實例化 / inert** | ✅ Clean build 2026/6/4 11:13:58（ExitCode=0, EF BF BD=0）|
| 工作包 B | **TfMaintenance Top CCD 頁**（動態建頁，無新 DFM/form/bpr）：IP/Port 設定（General.ini `[TopCCD]`）、Connect/Disconnect/Save/Reload、手動 Trigger Shot (LON)、結果/狀態顯示、log memo、`EnsureTopCcdSocketCreated()` lazy 實例化、timer 輪詢 `RefreshTopCcdStatus`；**保留 disabled「Bottom CCD (reserved)」checkbox** | ✅ Clean build 2026/6/4 11:25:11（ExitCode=0, EF BF BD=0）|
| 工作包 C | **P3 啟用**：`ReadTopCcd2DCode` 改非阻塞輪詢（`TopCcdPoll`+`TopCcdGetResult`）；`DoCcdCheck` case 5000 重構為「觸發(LON)→新 case 5500 輪詢取碼」非阻塞兩段；查無對照/逾時 → `ShowMyError(K_RETRY\|K_SKIP)`（Retry 重拍、Skip 導 Error 1001）；`TfMain::Start` 接 `SetCurrentLotNumber(edLotNo->Text)`。安全閘：socket 未連線或旗標關 → 不進 5500，行為同改前 | ✅ Clean build 2026/6/4 11:38:28（ExitCode=0, EF BF BD=0）|

> P4 在 P3 未實際取碼前仍 **inert**（`iBin` 恆 0、SortArm 路由 fallback 回舊 `TrayData`）。P3 啟用條件：工程師在 Maintenance Top CCD 頁連上相機 **且** `bUse2DBinMap` 開啟。
>
> **剩硬體實機驗收**：放對照 JSON → 跑料 → IC 依 2D→Bin→Area 落到正確 Auto 區；查無 → Error 區。注意確認相機拍照模式（每格一拍 vs 整盤一拍）：目前 case 5000/5500 假設「每格一拍」（移動到該格後送 LON、輪詢該格結果）。若為整盤一拍需調整觸發呼叫點。

---

## 1. 舊 160 Top CCD 取碼 API（已查證，唯讀參考）

來源（唯讀，**禁止修改**）：`D:\HT160S -Original 20260323\Code_V300A\Program_HT160S_20240806_20241111-SECSGem`

| 元件 | 位置 | 角色 |
|------|------|------|
| `ClientSocket1`（`TClientSocket`）| `TfSetup`（setup.cpp/.h、setup.dfm）| **Top CCD** socket |
| `clntsckt_Bottom_CCD` | `TfSetup` | Bottom CCD（**本案不導入**）|
| `bCCD_Connect` | 全域/表單 | Top 連線狀態（`ClientSocket1Connect`/`Disconnect` 設定）|
| `SendCMD_CCD(AnsiString)` | `TfSetup` | 守 `bCCD_Connect` → `ClientSocket1->Socket->SendBuf`；命令 `"LON"` = 拍照 |
| `ClientSocket1Read` | `TfSetup` | 收碼：`bCCD_LON_Fin=true; sCCD_2D=ReceiveText().Trim(); Save_2D_Log(sCCD_2D)` |
| `SendCCDNoraml_CMD()` | aLoader | 送 `"LON"`，回 `bCCD_LON_Fin` |
| `CCD_Data_Return(sCCD_2D)` | aLoader | 把 2D 字串 → Bin（舊版用 checkbox / `sError_Bin`，本案改用 `Bin2DMap.Lookup`）|
| `btn_CCDConnectClick` / `btn_DisconnectClick` | `TfSetup` | 手動開/關連線；狀態燈 `pnl_CCDState` |
| `ChangePPID(sPPID)` | `TfSetup` | 送 `#LF,<ppid>@` |
| `ClientSocket1Error` | `TfSetup` | 連線錯誤訊息 |

確認結論：
- Top CCD 取碼 = **送 `"LON"` 拍照 → `ClientSocket1Read` 收回字串存 `sCCD_2D`**。✔ 與你描述一致。
- 舊碼是「送 LON 後立刻讀 `bCCD_LON_Fin`」，靠 Task FSM 的延遲步 + VCL 訊息泵收 socket。**移植到 HT160_BCB 必須改成非阻塞輪詢**（送命令 → 後續迴圈輪詢 `bOk`，不可 Sleep / busy-wait）。
- **不導入 `clntsckt_Bottom_CCD`**。✔

---

## 2. 作戰包（三個工作包，建議照順序）

### 工作包 A：Top CCD socket 移植 + 改名 TopCCD（不含 Bottom）

目標：在 HT160_BCB 建立一個 **TopCCD** 連線/收發層，提供「送 LON → 取得該次 2D 字串」非阻塞 API，供 P3 的 `ReadTopCcd2DCode` 呼叫。

- **改名規則**（為了好辨識，移植時一律改名）：
  - `ClientSocket1` → `sckTopCcd`
  - `SendCMD_CCD` → `SendTopCcdCmd`
  - `ClientSocket1Read` → `TopCcdSocketRead`
  - `bCCD_Connect` → `bTopCcdConnect`
  - `bCCD_LON_Fin` → `bTopCcdReadDone`
  - `sCCD_2D` → `sTopCcd2D`
  - `Save_2D_Log` 保留（或 `SaveTopCcd2DLog`）
- **承載位置（✅ 已定）**：新增獨立模組 `TopCcdSocket.cpp/.h`，**放專案根**（與 `AutomationServer.cpp` 同層——它是現存最接近的 raw winsock comm 先例；非 `MotorAndIO\`）。class `THT160TopCcdSocket` + 全域 `TopCcdSocket`（NULL，工作包 B/C 再實例化）。
  - 實作採 **raw winsock client**（非 VCL `TClientSocket`），非阻塞 connect（`select` 探測完成）+ 非阻塞 `recv` 輪詢，無 Sleep、無 FSM，純 ASCII 檔。
- **非阻塞 API 形狀（建議）**：
  ```cpp
  bool TopCcdConnect();              // 開 socket
  void TopCcdDisconnect();
  bool IsTopCcdConnected();
  void TopCcdTriggerShot();          // 送 "LON"，清 bTopCcdReadDone
  bool TopCcdGetResult(AnsiString &sCode); // 已收到回 true + sTopCcd2D；否則 false
  ```
- **限制**：BCB6（無 C++11）、AnsiString、`.cpp/.h` Big5 → **必用 Python latin1 binary 編輯**；不可 Sleep；新增 .cpp 要更新 `ht160s.bpr`（OBJFILES + FILELIST）。
- **驗收**：build clean；TopCCD 可連線、送 LON、收回字串（可先用手動驗證面板，見工作包 B）。

### 工作包 B：TfMaintenance 加 TopCCD 連線設定 + 手動送訊號驗證

目標：維修畫面可設定 TopCCD IP/Port、手動連線/斷線、手動送 `"LON"` 並顯示回傳字串（驗機/驗線用）。

- **參考**：舊 160 `maintenance.cpp` 的 `clntsckt_UseMES`（IP/Port 編輯框 + Open + SendBuf）是同型樣板；舊 setup 的 `btn_CCDConnectClick` 是連線樣板。
- **HT160_BCB 現況先查**：grep `TfMaintenance` / `maintenance.dfm` 是否已有 socket 設定區塊；ComPort/IP 設定慣例放哪（`config.ini` vs `General.ini` vs `machine_option.ini`）。依 `ht160s-config-tiers` 判定：**TopCCD 連線位址屬「出機 + 硬體安裝」層 → General/machine_option**，非 Recipe。
- **UI 元素（建議）**：IP 編輯框、Port 編輯框、Connect/Disconnect 鈕、狀態燈、「手動拍照」鈕（送 LON）、回傳字串顯示框。
- **限制**：`.dfm` 也是 Big5；新增 DFM 控件要同步 .h 宣告與 .dfm，且用 Big5 安全方式。`.cpp` 邏輯走工作包 A 的 TopCCD API。
- **驗收**：build clean；維修畫面手動連線→送 LON→看到相機回傳字串。

### 工作包 C：P3 啟用（接上真碼 + Lot 號 + 查無路由）

目標：把 inert 的 P3 變成真正運作。

1. **填 `TLoaderModule::ReadTopCcd2DCode(LoaderNo,CellX,CellY,bOk)`**（aLoader.cpp）：
   - 用工作包 A 的 `TopCcdTriggerShot` + `TopCcdGetResult` 非阻塞取該格 2D 字串。
   - 取得 → `bOk=true; return sCode;`；未就緒 → `bOk=false`（維持目前安全行為）。
   - 注意：`DoCcdCheck` case 5000 目前是「一次拍照取一格」的時序；若相機是「一次拍整盤」要調整呼叫點（開工先確認相機模式）。
2. **LotStart 呼叫 `SetCurrentLotNumber(asLotNo)`**：找 HT160_BCB 的開批流程（grep LotStart / 開批按鈕），在設定 Lot 後呼叫。
3. **查無對照（iBin=1001）路由決策（✅ 已定）**：查無對照時 **報警 + 出 Note 給操作員，提供 Retry / Skip 兩選項**：
   - **Retry** → 重新觸發 TopCCD 拍照取碼（`TopCcdTriggerShot` + 重新 `Bin2DMap.Lookup`），命中則正常路由。
   - **Skip** → 直接把該顆定義為 Error（`iBin=1001`），經 P4 `BinAreaMap.GetAreaByErrorBin` 導到 `ErrorBinArea`。
   - 不再採「靜默導 Error」；報警會打斷節奏但操作員有感、可人工補救。
- **驗收**：旗標 `bUse2DBinMap` 開啟下，放對照 JSON → 跑料 → IC 依 2D→Bin→Area 落到正確 Auto 區；查無 → Error 區。

---

## 3. 不做 / 邊界

- ❌ 不導入 `clntsckt_Bottom_CCD`（Bottom CCD）。
- ❌ 不改 `D:\HT172`、不改舊 160（`D:\HT160S -Original 20260323`）任何檔案（唯讀參考）。
- ❌ 不引入 FSM（`FSMRunner`/`*Step.h`/`*Table.cpp`/`*Exec.cpp`）；維持 HT160 procedural / `switch(Task)`。
- ❌ machine-control 路徑不用 Sleep / 阻塞迴圈。

## 4. 共同限制（每個工作包都適用）

- `.cpp/.h/.dfm` = Big5 → **只用 Python latin1 binary 編輯**，改完掃 `EF BF BD` 必為 0；不可用 VS Code 編輯器工具改這些檔。
- `.md` 文件 = UTF-8，可用編輯器工具。
- grep 本專案需 `includeIgnoredFiles: true`（`HT160S_Program_BCB_V1.0.0.0` 被 gitignore）。
- 改 `.cpp` 後先刪對應 `.obj` 再 build；新增 `.cpp` 要更新 `ht160s.bpr`（OBJFILES + FILELIST，.mak 自動產生）。
- Build：`cd D:\HT160S_BCB; powershell -ExecutionPolicy Bypass -File .\scripts\ops\build-ht160s.ps1`（全清加 `-Clean`）。
- 收尾刪掉臨時 script（如 `scripts\tmp_*.py`）。

---

## 5. 新工作階段啟動提示（複製這段開新 session）

```
延續 HT160S_BCB 2D 對帳本開發。Batch 1/2/3（P1/P2/P3 接線/P4）已完成並 build clean，
P3、P4 目前 inert。請依 docs\plan\2dbin-map-next-session-plan.md 施作【工作包 A：Top CCD
socket 移植 + 改名 TopCCD】（不導入 Bottom CCD）。

要求：
1. 先 grep HT160S_Program_BCB_V1.0.0.0（includeIgnoredFiles:true）確認是否已有可重用的
   socket 容器，決定 TopCCD 放既有表單或新模組 MotorAndIO\TopCcdSocket.cpp/.h。
2. 移植舊 160 ClientSocket1/SendCMD_CCD/ClientSocket1Read 的 Top CCD 收發邏輯，全部改名為
   sckTopCcd / SendTopCcdCmd / TopCcdSocketRead / bTopCcdConnect / bTopCcdReadDone / sTopCcd2D。
   提供非阻塞 API：TopCcdConnect / TopCcdDisconnect / IsTopCcdConnected / TopCcdTriggerShot /
   TopCcdGetResult。"LON"=拍照。
3. 限制：BCB6 無 C++11、AnsiString、Big5 檔案只用 Python latin1 binary 編輯、不可 Sleep、
   不引入 FSM、不改 D:\HT172 與舊 160。新增 .cpp 要更新 ht160s.bpr。
4. 改完掃 EF BF BD=0，刪對應 .obj 後跑 build-ht160s.ps1 驗證 clean，刪臨時 script。

工作包 B（TfMaintenance TopCCD 連線設定 + 手動送 LON 驗證）、工作包 C（P3 啟用：填
ReadTopCcd2DCode + LotStart 呼叫 SetCurrentLotNumber + 查無路由決策）排在 A 之後，
詳見計畫檔。
```
