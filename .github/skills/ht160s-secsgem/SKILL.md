---
name: ht160s-secsgem
description: >-
  Use when working on the HT160S_BCB SECS/GEM subsystem (HT160Gem in
  SecsGem/uHGemHT160.cpp/.h): the THGem HSMS-SS engine (active/passive socket,
  SECS-II codec, SV/EC/CEID/Report registry), HTGem base class + Dispatch,
  HT160Gem derived handler, TFSECS controller + global EventReport(), HSys.MyGem
  polymorphic pointer, SECS_EVENT 31-event table, the real SVID/ECID/CEID maps,
  S2F42 host commands (SET_LOT_INFO/PAUSE/ONLINE_*), General.ini [SECS] config,
  and the connection-mode (Select.req) interop facts.
  Triggers: HT160Gem, uHGemHT160, uHGemClass, uHGemEquipment, THGem, HTGem,
  TFSECS, UsecegemMainFrom, SECS_EVENT, EventReport, HSys.MyGem, AddSV, AddEC,
  AddCEID, AddReprot, Dispatch, SetSVDataPointer, SetCEIDContent, SetHsmsMode,
  StartCommunication, S2F42_Host_Command_Acknowledge, SET_LOT_INFO, bActiveMode,
  General.ini SECS, USE_SECS_GEM, GemInitial, SVID, CEID, ECID, S6F11, EAP, host.
---

# HT160S SECS/GEM (HT160Gem)

## 1. 目的與現況

HT160S 的 SECS/GEM 子系統位於 `HT160S_Program_BCB_V1.0.0.0/SecsGem/`，
由 HT172 的 SECS/GEM 架構（`uHGemHT172` / `HT172Gem`）移植，但 **HT160 重寫成
form-less `TComponent` 引擎（route B）**，不是整份搬 HT172 的 `TForm` 引擎。

> **現況（2026-06-11，已驗證 build green）：不再是 skeleton，是可端到端運作的引擎。**
> - HSMS-SS socket transport（active/passive）已實作：frame 組裝、Select/Linktest/
>   Separate 控制訊息自動回應、data message 分派。
> - SECS-II encode/decode codec 完整（由 HT172 codec 移植，去 GUI）。
> - SV/EC/CEID/Report **registry 引擎**（form-less `TList`）已實作並可序列化。
> - `S2F42` host command **SET_LOT_INFO / PAUSE / ONLINE_REMOTE / ONLINE_LOCAL** 已實作。
> - `EventReport` 真的會組 **S6F11** 封包送出（SELECTED 才送）。
> - GUI log/監控視窗 `TfSecsGemLog`（DFM，5 個 tab，含可編輯 [SECS] 設定）已做。
>
> 仍待補：alarm list 細節、recipe 上下傳（S7）、S14 tray 屬性的完整實作、real host 現場驗證。

移植脈絡（repo memory `ht160s-secsgem-porting.md` 有逐日紀錄）：
- 2026-05-24 先加 HT172-style skeleton，build verified。
- 命名對映：`uHGemHT172` → `uHGemHT160`、class `HT172Gem` → `HT160Gem`。
- 2026-06-10 ~ 06-11 逐步補：codec → HSMS transport → Dispatch → registry → SVID/ECID →
  S2F42 host commands → S1F11/F12、S2F15/F16 → GUI。每步刪 `.obj` 後 build green。
- `.bpr` / `.mak` 已把 `SecsGem`（含 `uHGemLogForm` DFM）納入 build。

## 2. 四層架構

| 層 | 類別 / 檔案 | 全域物件 | 角色 |
|----|------------|----------|------|
| 引擎 | `THGem`（`uHGemEquipment.cpp/.h`）| `THGem *HGem` | SECS 訊息引擎（TComponent）。SVID/ECID/CEID/Report 註冊、`EventReport`、訊息 head/data IO、`StringOut` log |
| 基底 | `HTGem`（`uHGemClass.cpp/.h`）| — | 所有 `SxFy_*` virtual handler 的基底，預設 `SendUnsupported()` |
| 機種 | `HT160Gem : HTGem`（`uHGemHT160.cpp/.h`）| `HSys.MyGem`（基底指標）| HT160S 專屬 override：AddSV/EC/Alarm/CEID/Report 與各 SxFy stub |
| 控制 | `TFSECS`（`UsecegemMainFrom.cpp/.h`）| `TFSECS *FSECS` | 初始化協調（`GemInitial` / `SECS_SETData` / `MySFCode`）+ 全域 `EventReport(Ceid)` |

型別常數：`HTypeStruct HType`（`uHGemEquipment.h`）提供 `ASCII_TYPE` / `INT_4_TYPE`
/ `BOOLEAN_TYPE` … 給 `SetSVDataPointer` 等使用。

```
EAP/Host  <--HSMS-->  THGem(HGem)  --分派-->  HSys.MyGem(HT160Gem) override SxFy
                          ^                         |
                          |  EventReport(1,Ceid)    | SetSVDataPointer / SetCEIDContent
                       TFSECS(FSECS) ---------------+
```

## 3. 建立與初始化時序

`ht160s.cpp` WinMain（約 L161-163）：

```cpp
HGem = new THGem(Application);              // 先建引擎
HSys.MyGem = new HT160Gem("HT160S", HGem); // 再建機種物件，傳入引擎指標
```

`TFSECS::GemInitial(HandlerType, SoftwareVersion)`（`UsecegemMainFrom.cpp`）
**從 `system\General.ini [SECS]` 讀端點**（與 `[ColorCCD]`/`[TopCCD]` 同 pattern），
不再 hard-code：

```cpp
// [SECS] 區段（TIniFile，預設值如下）
//   Enable     = 1            // 0 = 整個 GEM stack 關閉
//   Address    = 127.0.0.1    // ActiveMode=1 時才用（host IP）
//   Port       = 5098
//   DeviceID   = 0
//   ActiveMode = 0            // 0 = passive（equipment 監聽）, 1 = active（撥出當 host）
USE_SECS_GEM = (iEnable > 0) ? 1 : 0;
HGem->SetTimeFormat(1);
HGem->SetDefaultAddressAndPort(sAddress, sPort, sDeviceID);
HGem->SetReceipeDirectoryAndGlobalName("..\\data\\", "*.*", 0);
HGem->SetMachineTypeAndSoftwarseVer(HandlerType, SoftwareVersion);
SECS_SETData(HGem);          // AddSV/AddEC/AddAlarmList/AddCEID/AddReprot
HGem->SaveEventReportData();
HGem->Timer1->Enabled = true;
if(USE_SECS_GEM > 0) {
    HGem->SetHsmsMode(iActive != 0);  // 設 active/passive
    HGem->StartCommunication();        // listen（passive）或 connect（active）
}
bInitialed = true;
```

> repo 內 `system\General.ini` 目前 `ActiveMode=0`（HT160 passive listen on 5098）。
> 換設定後**要重啟**才生效（`GemInitial` 只在開機跑一次）。
> 維護畫面 `TfSecsGemLog` 的 Settings tab 可寫回 [SECS]，但一樣 write-only、需重啟。

`TFSECS::SECS_SETData` 透過 `HSys.MyGem` 多型呼叫 HT160Gem 的五個註冊函式。
`USE_SECS_GEM`（預設 `1`）為總開關；全域 `EventReport()` 在
`USE_SECS_GEM<=0 || HGem==NULL` 時直接 return。

## 3b. HSMS-SS 傳輸與連線模式（互通關鍵）

`THGem`（`uHGemEquipment.cpp/.h`）內建 `TClientSocket ClientSocket1`（active）與
`TServerSocket ServerSocket1`（passive，`stNonBlocking`），由 `bActiveMode` 決定用哪一個：

| `ActiveMode` | HT160 TCP 角色 | 對手（host/工具）角色 |
|---|---|---|
| `0`（repo 預設）| **passive**：`ServerSocket1` listen on Port | 工具當 **active client**，連進 HT160:Port |
| `1`（現場 160=Client）| **active**：`ClientSocket1` 撥到 Address:Port | 工具當 **passive server**，等 HT160 連入 |

**互通鐵律（已驗證 FACT，`uHGemEquipment.cpp` HandleControlMessage / ClientConnect）：
HT160 只「回應」Select.req，從不「主動發」Select.req。**
- `ClientConnect` → 只 `OnPeerConnected`（state=CONNECTED「awaiting Select」），**不送 Select.req**。
- `HandleControlMessage`：收到 `SELECT_REQ` → 回 `SELECT_RSP` 並設 `SELECTED`；
  收到 `SELECT_RSP` 只是「我們的 Select 被接受」（但沒有任何地方真的去發 Select.req）。

> 推論（INFER 基於上述 FACT）：`ActiveMode=1` 時 HT160 撥出後會卡在 CONNECTED、
> 不會自動進 SELECTED，除非**對手（host）送 Select.req**。所以無論 TCP 哪邊主動，
> **一定要由 host/測試工具送 Select.req**。要做端到端測試，最穩的是讓
> HT160 維持 `ActiveMode=0`（passive），由測試工具當 active client 連入並送 Select.req。

收送流程：`ReadFromPeer` → `RecvBuffer` 累積 → `ProcessReceiveBuffer`（4-byte MSB 長度
frame 組裝）→ 控制訊息走 `HandleControlMessage`、data 訊息走 `HandleDataMessage`
（取 S/F → `DecodeReceiveBody` → `GemLogic->Dispatch(S,F)`）。
送出：`SendLocalData` 在 `SELECTED` 時 `ActiveSocket->SendBuf(LocalBuffer, LocalLength_4)`。

`HTGem::Dispatch(S,F)`（`uHGemClass.cpp`）路由表（**偶數 F = host 回覆，log 後丟棄、不回 S9F3**）：
S1F1→S1F2、S1F3→S1F4、S1F11→S1F12、S1F13→S1F14、S2F13→S2F14、S2F15→S2F16、
S2F17→S2F18、S2F25→S2F26、S2F31→S2F32、**S2F41→S2F42**、S5F3→S5F4、S5F5→S5F6、
S7F1/3/5/17/19、S10F3/5、S14F1；未知 → S9F3。

## 3c. 連線重試（FACT 2026-06-12，已實作 build green）

> **`Timer1Timer` 看門狗已實作自動重連**（1 秒 tick，非阻塞、無 FSM）。

- `StartCommunication()` 開機跑一次並設 `bWantComm=true`；`StopCommunication()` 設 false
  （明確停止後看門狗不會立刻又把 socket 開回來）。
- `Timer1Timer`（`uHGemEquipment.cpp`）每秒：`bWantComm && iReconnectInterval>0 &&
  iHsmsState==NOTCONNECTED` 時倒數，歸零呼叫 `DoReconnectAttempt()` 後重設倒數；
  已 CONNECTED/SELECTED 則把倒數重新 arm 等下次斷線。
- `DoReconnectAttempt()`（try/catch 包覆）：**Active** 模式 `ClientSocket1->Active=false`→
  重設 Address/Port→`=true`（重撥）；**Passive** 模式 `if(!ServerSocket1->Active)` 才
  重新 `Active=true`（重新 listen，守 bind 失敗）。每次 `StringOut` 一行可在 Log tab 看到。
- 設定鍵 **`General.ini [SECS] ReconnectInterval`**（秒，預設 5，`0`=停用），
  `GemInitial` 讀入後 `HGem->SetReconnectInterval()`。
- 狀態存取：`GetReconnectStatusText()` 在未連線時回 `"retry in Ns (attempts M)"`、
  停用回 `"auto-reconnect off"`、已連線回空字串；GUI Connection tab 的 State 行會附帶顯示。
- 仍是「HT160 只回應 Select.req、不主動發」（見 §3b）——重連只負責把 TCP 接回來，
  進 SELECTED 仍需 host 送 Select.req。

### 斷線偵測（FACT 2026-06-11，已實作 build green）

之前關掉模擬器 host 後 160 仍顯示 SELECTED的兩個漏洞已補：
- **心跳（Linktest）**：SELECTED 時 `Timer1Timer` 每 `LinktestInterval` 秒主動送
  `Linktest.req`（`SendLinktestReq`，SessionID 0xFFFF + 自產 SystemBytes）；收到
  `Linktest.rsp` 清 `bAwaitLinktestRsp`。逆測「惄惄死掉」（RST / 砸 process / 半開）。
- **T6 逾時**：送出 Linktest.req 後 `T6Timeout` 秒內沒收到 rsp → `DropConnection()`
  關 socket + `OnPeerDisconnected()` → 看門狗重連。
- **錯誤事件降狀態**：`ClientError`/`ServerClientError` 收到錢誤（含 RST，不走
  OnDisconnect）且 `iHsmsState>=CONNECTED` 時 `DropConnection()`。
- 設定鍵：`General.ini [SECS] LinktestInterval`（秒，預設 10，0=關心跳）、
  `T6Timeout`（秒，預設 6，<=0 回退 6）。`GemInitial` 讀入後 `SetLinktestInterval`/`SetT6Timeout`。
- **溢位防護**：`iReconnectAttempts` 以 `< 1000000000` 護欄防長期無 host 運轉溢位；
  `uControlSystemByte` wrap 跳 0。

## 3d. 通訊內容磁碟記錄（FACT 2026-06-11，已實作 build green）

所有 `StringOut()` 進來的 SECS 通訊內容除了存記憶體 `LogList`（GUI 監看用），
另複製一份到 `LogFileBuffer`（**獨立緩衝**，避免 GUI `DrainLog()` 把要寫檔的行清掉），
由 `Timer1Timer` 每秒 `FlushSecsLogToFile()` 批次寫盤。
- 根目錄 = `HSys.LogRootDir`（`= "D:\HT160S_Log"`，與 EventLog 等共用），分層仿 HT172（每日一資料夾）。
- 資料夾：`D:\HT160S_Log\SECS_GEM\yyyy_mm_dd\`（`ForceDirectories`）。
- 文字記錄：`SECSGEM_TextLog_<hh>.txt`（每小時一檔，`fopen "a+"`）。
- 錯誤記錄：`SECSGEM_ErrLog.txt`（每日一檔）；`DropConnection()` 會呼叫 `SaveSecsErrToLog(Reason)`。
- 時間戳用 `DecodeDate`/`DecodeTime`+`sprintf`（**不要用 `FormatString`** — `/`/`:` 會被
  `DateSeparator`/`TimeSeparator` 取代造成 locale 陷阱）。
- 所有檔案 I/O 都 `try/catch` 包覆：寫盤失敗絕不干擾通訊。
- 設定鍵：`General.ini [SECS] LogToFile`（預設 1，`GemInitial` 讀入 → `HGem->SetLogToFile()`）。

## 3e. SECS 付費客戶功能卡關（FACT 2026-06-11，已實作 build green）

SECS/GEM 是**付費客戶功能**，由 `CosFunction.bUseSecsGem` 卡關（4-tier 的 CosFunction 層，
compile-time `switch(CUSTOMER_CODE)`，無 ini）。
- 預設：`InitialCosFunction()` 設 `false`（未購買關閉）；`FUNC_CC_HONPREC_QC()` 設 `true`
  （= `HT160S_DEFAULT_CUSTOMER_CODE`，所以開發版 SECS 開啟）。
- 卡關點：
  1. `UsecegemMainFrom.cpp GemInitial`：`if(!CosFunction.bUseSecsGem){ USE_SECS_GEM=0; return; }`
     → 整個 HSMS stack 不啟動（不開 socket、不建立心跳）。
  2. `main.cpp BuildFeatureStatusBadges`：`eMainFeatureSECS` badge 只有 `bUseSecsGem` 為真才
     接 `OnClick` 並顯示；否則 `BadgePanel->Visible=false`。`FeatureBadgeSecsClick` 進場防呆。
  3. `cStateRecordHT160 CaptureSecsLog`：未購買則不打包 SECS log。

## 3f. StateRecord 打包 SECS log（FACT 2026-06-11，已實作 build green）

`cStateRecordHT160::TriggerSnapshot` 在 `CaptureConfig` 之後、壓縮之前呼叫
`CaptureSecsLog(TempDir)`：
- 僅當 `CosFunction.bUseSecsGem` 為真才執行。
- 先 `HGem->FlushSecsLogToFile()`（NULL-guard + try/catch）把最新行寫盤，確保快照含當下。
- 複製 `D:\HT160S_Log\SECS_GEM\<今天 yyyy_mm_dd>` → 快照內 `SecsLog\<day>\`
  （`CopyFolderFiles`，`DirectoryExists` 護欄，無當日資料夾就略過）。
- `cStateRecordHT160.cpp` 為此 `#include "uHGemEquipment.h"`（取 `HGem` extern）。


## 4. SECS_EVENT 事件表（31 個）

`ETypeStruct SECS_EVENT`（`uHGemHT160.h`）是 1-based enum，搭配
`EventDescription[]`（`uHGemHT160.cpp` 建構子填入）。CEID = enum 值。

| # | 名稱 | 說明 |
|--:|------|------|
| 1 | HandlerStatus | Handler change status |
| 2 | RecipeChange | Recipe Change |
| 3 | ClearCount | Press Clear Count button |
| 4 | PressStartWithoutIC | Start（機內無 IC）|
| 5 | PressStartWithIC | Start（機內有 IC）|
| 6 | PressPause | Pause |
| 7 | PressHome | Home |
| 8 | PressOneCycle | One Cycle |
| 9 | PressCleanOut | Clean Out |
| 10 | PressTrayFeed | Tray Feed |
| 11 | PressLotStart | Lot Start |
| 12 | PressLotEnd | Lot End |
| 13 | PressExit | Exit |
| 14 | PressRetry | Retry |
| 15 | PressSkip | Skip |
| 16 | PressAlarmReset | Alarm Reset |
| 17 | ShowAlarm | Show Alarm |
| 18 | ReleaseAlarm | Release Alarm |
| 19 | ShowMessage | Show Message |
| 20 | ReleaseMessage | Release Message |
| 21 | ChangeUser | Switching User Level |
| 22 | EnterSetup | Enter Setup Page |
| 23 | EnterMaintenPage | Enter Maintenance Page |
| 24 | EnterIOPage | Enter I/O Page |
| 25 | EnterTeach | Enter Teach Page |
| 26 | EnterSECSPage | Enter SECS GEM Page |
| 27 | OneCycleOK | One Cycle Finish |
| 28 | CleanOutOK | Clean Out Finish |
| 29 | TrayFeedOK | Tray Feed Finish |
| 30 | TimeEvent | Time Event |
| 31 | RealDummy | Switching Real/Dummy Mode |

`SECS_EVENT.TotalEvent`（=32）為哨兵；`AddCEID()` 用
`for(i=HandlerStatus; i<TotalEvent; i++)` 註冊全部 31 個 CEID，每個掛 ReportID `1`。

## 5. SVID / ECID / Report（已實裝）

`HT160Gem::AddSV()` 註冊 13 個 SVID：**9045 共同帶 + HT160 自訂 1100+**。
設計原則（repo memory 2026-06-11）：**穩定位址且型別吻合的 scalar 才 direct-bind；
bool/enum/form-ptr/derived 一律 snapshot**（`RefreshSVData()` 在送事件前 / S1F4 開頭 snapshot）。

| SVID | Type | Name | 來源 |
|-----:|------|------|------|
| 1001 | ASCII | Machine Model | `HandlerPath`（direct）|
| 1003 | ASCII | Software Version | `svSoftwareVersion`（ctor "1.0.0.0"）|
| 1021 | INT4 | UPH | `svUPH` ← `tRunData.UPH` |
| 1027 | ASCII | System Time | `sSystemTime` ← `Now()` |
| 1100 | INT4 | RunMode | `HSys.Sys.RunMode`（0Normal/1Home/2OneCycle/3CleanOut/4TrayFeed）|
| 1101 | INT4 | SystemRunning | `HSys.Sys.SystemStart?1:0` |
| 1102 | INT4 | ControlState | `iControlState`（4=Local/5=Remote）|
| 1110 | INT4 | AlarmActive | `(fNote&&fNote->fShow)?1:0`（NULL-guard）|
| 1111 | INT4 | AlarmCode | `fNote->Code` |
| 1120 | INT4 | TotalIC | `tRunData.TotalIC` |
| 1121 | INT4 | TotalSorted | `MachineRun.iTotalSorted` |
| 1130 | INT4 | ActiveLotCount | `LotRegistry.GetLotCount()` |
| 1131 | ASCII | CurrentLotID | `LotRegistry.GetLot(0)->sLotID`（""若無）|

`AddReprot()`：ReportID `1` = 上表 13 個 SVID（`SVIDs[64]`），每個 CEID 都掛 report 1。

`HT160Gem::AddEC()` 註冊 ECID：
- `1501` ASCII "Recipe Name"（snapshot `ecRecipeName` ← `RecipeManager.GetCurrentRecipeName()`）
- `2011-2014` FT_8 Tray X/Y Start/Pitch（**direct-bind** `&TrayForm.XStart/XPitch/YStart/YPitch`）
- `2015-2016` INT_4 Tray X/Y Division（direct-bind `&TrayForm.XDivision/YDivision`）

> EC 寫入（S2F15/F16 或 GUI）只允許 2011-2016 tray-form；且 `SystemStart||HasICUnderMachine()`
> 時 busy 拒寫（EAC=2）。成功後 `TrayForm.Save(RecipeManager.GetCurrentRecipeName())` 持久化。
> `AddAlarmList()` 目前仍空。

## 6. SxFy handler 現況（已實作）

`HT160Gem` override 並實作（差不多都是 S1F4 pattern：讀 L,n 查詢 → emit 回覆）：

| Handler | 行為 |
|---------|------|
| `S1F4_SelectedStatusReply` | 讀 SVID 清單（n==0=all），回 SV 值 |
| `S1F12_StatusVariableNamelistReply` | 回 L,3{U4 SVID, A name, A unit} |
| `S2F14_EquipmentConstanData` | 讀 ECID 清單回 EC 值（S1F4 鏡像）|
| `S2F16_NewEquipmentConstantSendAcknowledge` | EC 寫入：busy→EAC=2；只 2011-2016 可寫，否則 EAC=1 |
| `S2F42_Host_Command_Acknowledge` | **已實作 host command**（見下）|
| `ProcessS14F1_GetAttrRequest` / `ProcessS14F2_GetAttrData` | tray 屬性（部分）|

### S2F42 host command（`L[2]{ A cmd, L[n]{ CP... } }` → `L[2]{ B HCACK, L[0] }`）

| cmd | 行為 | HCACK |
|-----|------|-------|
| **SET_LOT_INFO** | 讀內層 `L[n]{ A lotID }`；`LotRegistry.Clear()` → `AddLot(…, HT160_LOT_SOURCE_SECS)`；首筆回填 `fMain->edLotNo->Text`。cap `HT160_MAX_LOT=64` | 0=ok / 1=format / 2=param(n>64) / 4=busy |
| **PAUSE** | `HSys.Sys.SystemStart=false; SoftStop=true`（只複製 side-effect，無 RecordProcess/EventReport）| 0 |
| **ONLINE_REMOTE** / **ONLINE** | `iControlState=5` | 0 |
| **ONLINE_LOCAL** | `iControlState=4` | 0 |

> HCACK=4（busy）觸發條件 = `SystemStart || HasICUnderMachine()`——生產中 / 機內有 IC 不接受換 Lot。
> SET_LOT_INFO 對應 `main.cpp btnLoadSimuDataClick` 的 5 筆模擬 Lot（SIMU_LOT_A~E）。

未 override 者落 `HTGem` 基底 → `SendUnsupported` / S9F3 unrecognized。
`TFSECS::MySFCode(2,41)` 為 legacy 入口（也導到 S2F42），但**真正 inbound 走
`THGem::HandleDataMessage → GemLogic->Dispatch(S,F)`**。

## 7. 事件上報的兩種呼叫法

1. **全域自由函式**（最常用）— `void EventReport(unsigned Ceid)`（`UsecegemMainFrom`）：
   內部 `HGem->EventReport(1, Ceid)`（DataID 固定 1）。
   呼叫點（`main.cpp`）：`RecipeChange`(L922)、`ChangeUser`(L970)、`RealDummy`(L1052)、
   `PressHome`(L1162)、`PressOneCycle`(L1184)、`PressCleanOut`(L1195)、`PressPause`(L1213)。
   被註解未啟用：`PressStartWithIC` / `PressStartWithoutIC`(L1264-1266)、
   `maintenance.cpp` 的 `EnterIOPage`(L1179)。
2. **引擎直呼** — `HGem->EventReport(DataID, Ceid)`：
   例 `aAuto1To6.cpp` L590 `HGem->EventReport(1, AutoCeid[iDischargeAuto])`。

> `THGem::EventReport(dataID, ceid)` **已實作**：未 SELECTED 則 skip；否則先 `RefreshSVData()`
> 再走 CEID→Report→SV 組出 **S6F11** `L[3]{U4 DATAID, U4 CEID, L[a]{ L[2]{U4 RPTID, L[b]{<SV value>...}} }}` 送出。

## 8. 修改前必查（防雷）

- **不再是 skeleton，但仍需雙範圍 grep**：宣稱某 handler 行為前，先讀 `uHGemHT160.cpp`
  + `uHGemEquipment.cpp` 確認（`.h`+`.cpp` 雙範圍）。alarm list / S7 recipe / S14 tray 屬性
  仍可能是部分實作。
- **DataItemOut 陣陑**：數值 scalar 的 `DataItemOut` 第一參是**元素 count（=1）不是 byte length**；
  型別才決定 byte 數（List 長度才是元素數）。
- **不可開 FSM**：依 HT160 治理，SECS 擴充維持程序式 / VCL event / `switch(Task)`，
  不得引入 FSMRunner / *Step.h / *Table.cpp / *Exec.cpp。
- **編碼**：`SecsGem/*.cpp/.h` 可能含 Big5；編輯保留原編碼。
- **S2F42 安全**：PAUSE/ONLINE/SET_LOT_INFO 已實作；新增會驅動機構的 host command 前，
  必須先在 `S2F42_Host_Command_Acknowledge` 複製 `busy=SystemStart||HasICUnderMachine()` 拒寫護欄。
- **連線重試（已實作，見 §3c）**：`Timer1Timer` 每秒看門狗，`bWantComm &&
  ReconnectInterval>0 && NOTCONNECTED` 時 re-dial(active)/re-listen(passive)。
  改重連邏輯時記得 `StopCommunication` 會清 `bWantComm`，避免停了又自動開回來。
- **HT172 唯讀**：要對照 HT172 的 `HT172Gem` 完整實作時，只能讀 `D:\HT172\...\SecsGem\uHGemHT172.cpp`
  作參考，**絕不寫入 D:\HT172**。HT172 端 GEM 知識已封裝在 HT172 workspace 的
  `secsgem-protocol` skill。
- 編輯後依 build contract：刪對應 `.obj` → `scripts/ops/build-ht160s.ps1`（或單檔 make）驗證。

## 9. 從 HT172 補實作的工作流

1. 讀 HT172 `uHGemHT172.cpp` 對應 handler / SVID / CEID 作為**行為參考**（唯讀）。
2. 在 HT160 `uHGemHT160.cpp` 用現有 `THGem` API（`SetSVDataPointer` / `SetCEIDContent` /
   `SetReportIDContent` / `EventReport` / `DataItemOut` / `DataItemIn`）重寫，套 HT160S 號碼與旗標。
3. 新 host command：在 `S2F42_Host_Command_Acknowledge` 依現有 SET_LOT_INFO/PAUSE/ONLINE 型式擴充，
   驅動機構前務必加 `busy=SystemStart||HasICUnderMachine()` 拒寫護欄。
4. 對應 EAP/AGV 規格（見 `docs/reports` E87/AGV 草案）規劃 SVID/CEID/Report 號段。
5. 刪 `.obj` → 編譯驗證；更新 repo memory `ht160s-secsgem-porting.md`。

## 10. 端到端測試（配外部 host / 模擬工具）

- 測試工具位於 `D:\AI_Area\Tool\HT160S_SECS_Simulator\`（Python stdlib，独立於機台碼）。
- **推薦測試拓撲**：HT160 維持 `ActiveMode=0`（passive listen on 5098），工具當 **active client**
  連進來、送 Select.req → SELECTED → 送 S2F41 SET_LOT_INFO → 收 S2F42。原因見 §3b：
  **HT160 只回應 Select.req，不主動發**，所以 host 必須是發 Select.req 的一方。
- 若要模擬現場「160=Client」（`ActiveMode=1`），工具要當 **passive server** 並主動送 Select.req——
  但 HT160 active 端目前不發 Select.req，需留意此互通限制。
