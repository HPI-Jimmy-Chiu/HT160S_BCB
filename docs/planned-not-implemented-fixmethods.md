# HT160S 未實作 / 半實作項目修法說明（Planned-but-Not-Implemented Fix Methods）

> 本文件僅為「修法說明（規劃）」彙整。**不修改任何原始碼**；HT172 為唯讀參考。所有程式識別字與 `file:line` 一律保留 ASCII，敘述以繁體中文呈現。
> 編碼規則：BCB6 舊有 `.cpp/.h/.dfm` 為 Big5/CP950，新增註解一律 ASCII English；本文件本身為 UTF-8（無 BOM）。
> 建置驗證：每次改動後先刪除對應 `.obj` 再編譯（`scripts/ops/build-ht160s.ps1 -Clean`，結構/標頭變更用 `-Full`），並跑 `scripts/ops/check-ht160s-source-encoding.ps1`。

---

## 摘要

本文件展開「仍待補（B 類）」各項目的修法細節，共 **38** 列（含同根因不同視角之重複稽核項），涵蓋 6 大類：`stub`、`gap-hardware`、`orphan-ui`、`dead-config`、`unhandled-state`、`todo`。

重點結論：

- **多數項目卡在「決策」而非「程式」**：TrayFeed 排空、AGV 取車感測點、SECS 警報/事件/Recipe 目錄等都需先確認是否在 HT160 產品範圍內。
- **TrayFeed（Run_TrayFeed）家族（F1/F5/F2/F6 多筆重複描述同一子系統）**：目前 `CheckAllTrayFeedFinish()` 永遠回傳 false 且進入點被註解掉，整條路徑「安全且休眠」。切勿只解註解進入點 —— 會造成機台永久卡在 TrayFeed 模式（永不回 Normal）。HT160 無 Magazine 子系統，無法 1:1 移植 HT172。
- **AGV 取車（IsAmrTaken，F4/F1/F2/F11 重複描述同一點）**：軟體掛點只有一行，但需先定義實體感測點（或改走 SECS 交握）。
- **已修正但稽核快照過期者**：`RecordSafeDoorStates()`（F4-safedoor 與 F4-csystem-RecordSafeDoorStates）其實已在 `csystem.cpp:1165-1210` 實作完成；稽核讀到的是舊備份檔。
- **最快安全收斂（low risk + S effort + 無 blocker）**：刪除確認為死碼的項目 —— `OriginRange/OriginRate`（無決策）、`IOInputLongByte` 死樁、`note.cpp` 7 個相容空殼、`aColor` IC 計數叢集、`RejectCCDfail/UseHitCylinder/HitRetry/UsePreAlignment` dead-config、`iBinDispColor[9]` dead-config、`HomeOrder` 寫入無讀取等。

---

## 總表

| id | category | title | effort | risk | confidence | 仍需決策? |
|---|---|---|---|---|---|---|
| F1-csystem-CheckAllTrayFeedFinish | stub | CheckAllTrayFeedFinish() 硬回 false，阻擋 Run_TrayFeed 自動完成 | M | low | 88 | 是（產品範圍：是否做 TrayFeed 排空） |
| F5-csystem-CheckAllTrayFeedFinish-stub | stub | 同上（重複稽核項，含進入點解註解風險） | S | low | 93 | 是 |
| F3-MyMotor-PackForAmrUpload-stub | stub | TMyCar::PackForAmrUpload 空樁；AMR upload payload 未設計 | S | low | 88 | 是（payload schema / IC 計數來源） |
| F6-uHGemEquipment-SetAlamData | stub | THGem::SetAlamData() 空（SECS 警報目錄 setter no-op） | M | low | 88 | 是（SECS 警報目錄是否在範圍） |
| F11-uHGemHT160-S7F6-recipe | stub | S7 recipe 上下傳處理器停用（log-only 骨架） | L | medium | 87 | 是（傳輸方向 / PPID 對應 / 里程碑） |
| F24-uHGemEquipment-IsEnableEvent | stub | THGem::IsEnableEvent() 硬回 true（事件啟用過濾遺失） | M | medium | 88 | 是（S2F37/S2F38 是否在範圍） |
| F4-csystem-RecordSafeDoorStates-empty | stub | RecordSafeDoorStates 空但每週期被呼叫（已修正，稽核過期） | S | low | 93 | 否（僅 log 政策確認） |
| F2-gem-SaveEventReportData-placeholder | stub | SaveEventReportData() 為 log placeholder（不持久化事件報表） | S | low | 93 | 是（事件報表持久化是否在範圍） |
| F4-aAuto1To6-IsAmrTaken | gap-hardware | IsAmrTaken() 實機硬回 false（AGV 取車 IO 未接） | S | low | 88 | 是（硬體/IO/極性） |
| F2-aAuto1To6-IsAmrTaken-TBD-comment | gap-hardware | 每 Auto「取車」IO 點設計註解標 TBD | S | medium | 95 | 是 |
| F11-aAuto1To6h-IsAmrTaken-decl-TBD | gap-hardware | IsAmrTaken 宣告處註明感測點 TBD | S | medium | — | 是 |
| F2-tapefeed-ringcatch-cassette-teach | gap-hardware | tape-feed/ring-catch/cassette 僅剩死的 teach 位置欄位 | — | — | — | 是（確認永久移除 vs 未來機種） |
| F18-aColor-IsAcceptingIC | orphan-ui | TColorModule::IsAcceptingIC() 硬回 false 且無呼叫者 | S | low | 93 | 是（叢集是否保留） |
| F1-systools-emptyform | orphan-ui | FormSysTools（System Tools 頁）空殼，接在主畫面 Tools 鈕 | S | low | 95 | 是（Tools 頁是否會回來） |
| ORPH-iosetview-spbTerminalProgram | orphan-ui | 「Terminal Program」按鈕 OnClick 空（關程式動作被樁掉） | S | medium | 90 | 是（是否保留 in-app 離開路徑） |
| ORPH-iosetview-ComboBox1Change | orphan-ui | ComboBox1（IOTool loop-time）OnChange 空 no-op | S | low | 93 | 是（移除 vs 重移植 blink 測試） |
| ORPH-iosetview-sbEnableIOChang | orphan-ui | 「Enable IO Change」按鈕 OnClick 空 no-op | S | low | 93 | 是（移除/隱藏/不處理） |
| F21-note-compat-empties | dead-config | note.cpp 7 個舊源相容空殼未被呼叫 | S | low | 93 | 是（清除 vs 保留相容性） |
| F22-myio-IOInputLongByte | dead-config | TMyIo::IOInputLongByte() 回 0（繼承死樁，無呼叫） | S | low | 96 | 否（僅次要範圍確認） |
| DC-tFunction-RejectCCDfail | dead-config | Config.ini [Function] RejectCCDfail 讀寫但無消費 | S | low | 96 | 是（功能是否要做） |
| DC-tFunction-UseHitCylinder | dead-config | [Function] UseHitCylinder 讀寫但無消費 | S | low | 95 | 是 |
| DC-tFunction-HitRetry | dead-config | [Function] HitRetry 讀寫但無消費 | S | low | 96 | 是 |
| DC-tFunction-UsePreAlignment | dead-config | [Function] UsePreAlignment 讀寫但無消費 | S | low | 96 | 是 |
| DC-GeneralSetting-iBinDispColor | dead-config | [BinDisplay] Color0..8（iBinDispColor[9]）讀寫但無讀取 | S | low | 95 | 是（刪除 vs 接成每單元色） |
| DC-MotTable-HomeOrder | dead-config | Mot_Table HomeOrder 欄解析進唯寫 TStringList | S | low | 95 | 是（A/B/C/D；HT172 是活的順序相依） |
| DC-Motor-OriginRangeRate | dead-config | Motor OriginRange/OriginRate 由 Mot_Table 餵入但無讀取 | S | low | 98 | 否 |
| F2-runmode-Run_TrayFeed | unhandled-state | RunModeEnum Run_TrayFeed 無活躍生產者（唯一 setter 被註解） | L | high | 95 | 是 |
| F5-trayfeed-finish-stub | unhandled-state | TrayFeed 撤離半實作（CheckAllTrayFeedFinish 樁 + 註解分支） | S | low | 90 | 是 |
| F6-aTrayArm-IsTrayFeedFinish-incomplete | unhandled-state | TrayArm TrayFeed 撤離依賴尚未存在的 Loader/EmptyTray 交握 | M | medium | 88 | 是 |
| F4-safedoor-state-record | unhandled-state | RecordSafeDoorStates 已實作於 1165-1210（稽核快照過期） | S | low | 96 | 否 |
| F8-csystem-bSortArmNeedHome-not-wired | unhandled-state | SortArm 單 Z 回原點請求層存在，但設旗標的生產者未接 | S | medium | 82 | 是（是否採 HT172 自動回復 + 偵測訊號） |
| F3-cmydef-eSystemTime | unhandled-state | 整個 eSystemTime enum 定義但全程式無引用 | M/S | low | — | 是（時間統計子系統是否在範圍） |
| F4-uHome-eHomeError | unhandled-state | eHomeError LED 狀態被 ShowLed 繪製但從不產生 | S | low | 93 | 是（紅燈 UX vs 刪死狀態） |
| F5-trayarm-TAS_PLACING | unhandled-state | TAS_PLACING 定義但從不產生；Status 唯寫 | S | low | — | 是（telemetry 是否上路線圖） |
| F7-motor-eMotorKind-nondefault | unhandled-state | eMotorKind 非預設值可由 config 產生但無分支 | A:S/B:M | A:low/B:med | — | 是（是否有非標準馬達） |
| F8-motioncard-nonproduced | unhandled-state | eMotionCardType eMC8040A/ePCI885X/ePLCbase 從不產生 | S | low | 93 | 是（是否支援 SMC/MN200 卡） |
| F7-aLoader-TrayArmTakeout-not-wired | dead-config | LOADER_Y_OWNER_TRAYARM 保留但取料交握未接 | S | low | 96 | 是（路線圖） |
| F1-aAuto1To6-IsAmrTaken-TODO | todo | IsAmrTaken AGV 取車感測未接（同 F4） | S | low | 93 | 是 |
| F10-MyBinDisp-P3-bMemo-echo-TODO | todo | bMemo 設定時 bin-display log 未回顯到 ComPort bin memo | M | medium | 90 | 是（是否要新 UI） |

> 註：稽核來源含多筆「同一根因、不同視角」的重複描述（IsAmrTaken 4 筆、TrayFeed 5 筆、RecordSafeDoorStates 2 筆）。下方依 category 分節展開；重複者合併在主項目下交叉註明。`F7-aLoader-TrayArmTakeout-not-wired` 稽核標 dead-config，置於 dead-config 節。

---

## stub 類

### F1-csystem-CheckAllTrayFeedFinish / F5-csystem-CheckAllTrayFeedFinish-stub（合併）

**位置**：`HT160S_Program_BCB_V1.0.0.0/csystem.cpp:1142-1148`（稽核標的 line:1097 已過期，該行現為 `CheckOneCycleFinish()`）；宣告 `csystem.h:40`。相關進入點 `csystem.cpp:1027-1037`，消費者 `csystem.cpp:1060-1069`，`CheckEmpty1TrayFeedFinish()` `csystem.cpp:1137-1140`。

**現況**：`bool CheckAllTrayFeedFinish(bool reset){ return false; }` 永遠回 false 且忽略 `reset`。`CheckEmpty1TrayFeedFinish()` 為 `return true;` 樁。進入 Run_TrayFeed 的唯一路徑（CleanOut 完成後 `if(retCleanOut==K_TRAY_FEED){...}`）整段被註解；`ShowSystemError` 只提供 `K_SKIP`，操作員選不到 TrayFeed。因此模式目前不可達、樁無害。HT172 版（`csystem.cpp:1176-1184`）聚合 `AllMagArmTrayFeedFinish() & AllMagTrayFeedFinish() & CheckEmpty1TrayFeedFinish() & TrayArmPara->bTrayArmTrayFeedFinish`，前二屬 HT172 Magazine 子系統，HT160 沒有。HT160 已有 `TTrayArmModule`（global `TrayArmModule`，`IsTrayFeedFinish()` `aTrayArm.cpp:82-89` 目前恆為 true）。對應排空模組為 `LoaderModule`/`EmptyModule`/`AutoModule`/`SortArmModule`。`CheckCleanOutFinish()`（`csystem.cpp:1080-1094`）已示範 null-guard 聚合樣式。`K_TRAY_FEED`（0x0004）已存在 `note.h:15` 且已接入鍵盤（`note.cpp:321,347,402`）。

**修法步驟**：
1. **決策閘**：先確認 HT160 是否需要 CleanOut→TrayFeed「排空剩餘空盤」功能。
2. 若否（建議，零工）：保留樁 + 註解進入點，把 `csystem.cpp:1024-1026` 註解改成「TrayFeed 為 HT160 刻意停用/won't-do 模式（無 Magazine 子系統）」。結束。
3. 若是（Option B）：
   - TrayArm：把 `IsTrayFeedFinish()` 改成真實 idle/empty 條件（如 `return (Status==TAS_IDLE && bHasTray==false)`），`InitialFlag()` 保留重置。
   - Loader：`aLoader.h` 公有區（`IsAllCleanOutFinish` 附近 105 行）加 `bool IsTrayFeedFinish();`，`aLoader.cpp` ~430 行實作（後段無空盤可撤即 true）。
   - EmptyTray + Auto：`aEmpty.h/.cpp` 與 `aAuto1To6.h/.cpp` 各加 `IsTrayFeedFinish()`（Auto 用 `FindTrayRequestAuto(kind)<0` 判斷無請求）。
   - 重寫 `CheckAllTrayFeedFinish()`：仿 `CheckCleanOutFinish()` 的 null-guard AND 鏈；`reset==true` 時清各模組 finish 旗標後 return false。
   - `CheckEmpty1TrayFeedFinish()` 可選給真值（file-scope `bool bEmpty1TrayFeedFinish`）或維持 true 並文件化。
   - 重啟進入點：`csystem.cpp:1027` 改成提供 `K_SKIP|K_TRAY_FEED`，解註解 `1028-1033`；可加 `EventReport`/完成 note 仿 HT172 `csystem.cpp:948-956`。
4. 同步 `csystem.cpp:1024-1026` 的 ASCII 註解。

**要動的檔**：`csystem.cpp`、`aTrayArm.cpp`、`aLoader.cpp/.h`、`aEmpty.cpp/.h`、`aAuto1To6.cpp/.h`。

**build 驗證**：Option A 只改註解 → 刪 `csystem.obj` 跑 `-Clean`。Option B 標頭新增公有方法（無新資料成員，但標頭散佈廣）→ 建議 `-Full`；再跑 encoding check。

**風險/工數**：low / M（Option B 多檔）。

**仍需決策**：HT160 是否要 CleanOut→TrayFeed 排空（Option A 維持刻意停用 vs Option B 跨 Loader/SortArm/Auto/TrayArm 實作 per-module finish + 重新定義「tray feed finished」於 HT160 硬體流程）。Option A 為合法 no-op。**警告（來自 F5 重複項）**：切勿只解註解進入點而不做 step 3 —— 否則 `CheckAllTrayFeedFinish()` 恆 false 會讓機台永久卡在 Run_TrayFeed。

---

### F3-MyMotor-PackForAmrUpload-stub

**位置**：`HT160S_Program_BCB_V1.0.0.0/MotorAndIO/MyMotor.cpp:235-238`（空樁，僅註解 "AMR upload payload not designed yet; stub."）；宣告 `MyMotor.h:115`。全樹 0 呼叫者；HT172 無對應（`TMyCar` 為 HT160-original）。

**現況**：`TAgvCoordinator` 已有 `int DeviceCount[AGV_STATION_COUNT]`（`uAgvStation.h:49`，`Reset()` 歸零於 `uAgvStation.cpp:53`），且已註冊為 SVID `SvidDeviceCnt`（38202-band，`uHGemHT160.cpp:137`，INT_4，unit "pcs"）。`PollAndCall`（`uAgvStation.cpp:129-136`）已刷 `TrayCount[si]=Car->iTrayCount`、`CarrierID[si]=Car->CarID`，但 `DeviceCount[si]` 釘在 0（行 132-133 註解明說）。`TMyTray` 無 IC 計數 accessor，但 `HasIC()/FullIC()`（`MyMotor.cpp:68-88`）已用 active-recipe bounds（`GetTrayRealXCount()/GetTrayRealYCount()`）走訪 `Data[x][y]!=0`。`TMyCar` 有 `iTrayCount/GetTrayCount()/GetTray(index)`、`CanHoldIC()`（`MyMotor.cpp:191-194`，僅 `eTrayKindNormal` 為 true）。

**修法步驟**：
1. **決策閘**：先確認 SVID 38202-band（`SvidDeviceCnt`）契約 —— 是「車內 IC 總數」（下方 minimal slice）或 per-tray/per-bin 明細？後者需擴大為結構化 payload。
2. `MyMotor.h` 在 `TMyTray` 公有區（`FullThisIC` 後）加 `int CountIC();`；`MyMotor.cpp` 實作以同樣 bounds 走訪 `Data[x][y]!=0` 計數。
3. `MyMotor.h` 在 `TMyCar` 公有區（`IsFull()` 後）加 `int GetDeviceCount();`；`MyMotor.cpp` 實作走訪 `Tray[0..iTrayCount-1]`、跳過 `CanHoldIC()==false`、累加 `CountIC()`。
4. `uAgvStation.cpp` `PollAndCall` 的 `if(Car!=NULL)` 區塊（行 130-136）加 `DeviceCount[si]=Car->GetDeviceCount();`，並更新行 132-133 過期註解。
5. 處理死方法 `PackForAmrUpload`：建議直接刪除 `MyMotor.cpp:235-238` 與宣告 `MyMotor.h:115`（值已由 `GetDeviceCount()` 提供）；或保留供日後結構化 payload。
6. 不可改 HT172（唯讀、無對應）。

**要動的檔**：`MyMotor.h`、`MyMotor.cpp`、`SecsGem/uAgvStation.cpp`。

**build 驗證**：`MyMotor.h` class 變更且被廣泛引用 → `-Full`（刪 `MyMotor.obj`、`uAgvStation.obj`）；再跑 encoding check。

**風險/工數**：low / S。

**仍需決策**：SVID 38202-band 契約（總數 vs per-tray 明細）。資料模型不阻擋（`TMyTray.Data[x][y]` 已可算）。無硬體相依。

---

### F6-uHGemEquipment-SetAlamData

**位置**：`HT160S_Program_BCB_V1.0.0.0/SecsGem/uHGemEquipment.cpp:656-658`（`SetAlamData` 空）；`ReadAlamData` `660-662`、`WriteAlamData` `664-666` 皆空。呼叫端 `HT160Gem::AddAlarmList()`（`uHGemHT160.cpp:232-234`）亦空（啟動時被 `UsecegemMainFrom.cpp:113` 呼叫）。`S5F6_ListAlarmData()`（`uHGemHT160.cpp:870-881`）刻意回空 `L,0`。

**現況（稽核 needsDecision 已過期修正）**：HT160 **已有**警報碼來源 —— `database.h:31-61` 定義 `MyAlarmCodeStruct`，`database.h:593-594` 宣告 `std::map<AnsiString,MyAlarmCodeStruct> mapAlarmCodeList` + `IterAlarmCodeList`，由 `CreateSystemAlarmCode()`（`database.cpp:755-884`）從 cylinder/motor/sucker 碼填充並已 dump 到 `system\AlarmList.csv`。HT160 無 StringGrid（THGem 無表單；SV/EC/CEID 皆用 heap-struct TList）。`THGem::CurrentDirectory = HSys.CurrentDir+"\\SECS"`（`uHGemEquipment.cpp:23`）為 HT172 `SecsGemPath` 的 HT160 對應。

**修法步驟**：
1. **範圍閘**：僅在 SECS 警報目錄（S5F5/S5F6，可選 S5F1）被明確列入當期里程碑時才動；空樁是內部一致的延後骨架（已避免 T3 timeout）。
2. `uHGemEquipment.h` 加 `struct TGemAlarmItem { unsigned __int64 ALID; int AlarmClass; AnsiString AlarmCode; AnsiString ALTX; AnsiString Position; int Enable; };`，THGem 私有加 `TList *AlarmList;` + `TGemAlarmItem *FindAlarmItem(unsigned __int64 ALID);`；ctor 初始化、dtor 清除。
3. 實作 `SetAlamData(...)`：寫入 TList（移植 HT172 `uHGemEquipment.cpp:6249` 但改 TList，`ALID=_atoi64(AlarmCode)`）。
4. 實作 `WriteAlamData()`：移植 HT172 `6303`，輸出到 `CurrentDirectory+"\\SYSTEM\\AlarmData.def"`，try/catch。
5. 實作 `ReadAlamData()`：移植 HT172 `6257`，若不存在則 return；僅在需持久化 Enable 編輯時才必要。
6. 實作 `HT160Gem::AddAlarmList()`：移植 HT172 `uHGemHT172.cpp:362-386`，迭代 `HSys.mapAlarmCodeList` 逐碼 `SetAlamData(...)`，最後 `WriteAlamData()`。
7. 實作 `S5F6_ListAlarmData()`：讀入 S5F5 ALID 清單，逐一 `FindAlarmItem`，輸出 `L,3 { B, INT_8 ALID, ASCII ALTX }`，找不到回 HT172 "Unknown Alarm Code" fallback。
8. 可選 `ReportAlarm()`（S5F1）—— 屬更大、獨立工作項，建議延後。
9. 實作後刪 3 個 `.obj`（`uHGemEquipment.obj`、`uHGemHT160.obj`、`uHGemClass.obj`）全建，跑 encoding check。

**要動的檔**：`SecsGem/uHGemEquipment.h`、`SecsGem/uHGemEquipment.cpp`、`SecsGem/uHGemHT160.cpp`。

**build 驗證**：標頭 struct/成員變更改變 THGem layout → `-Full`；encoding check 須過。

**風險/工數**：low / M。

**仍需決策**：(1) 警報目錄是否列入當期 SECS 里程碑；(2) [已解決] 用 `mapAlarmCodeList`，但須確認它即為權威 SECS 警報目錄（目前涵蓋 cylinder/motor/sucker，若需 system/sensor 碼須先加入 `database.cpp`）；(3) 是否需要 `AlarmData.def` 持久化（僅在 Enable 編輯需跨重啟時才有意義）；(4) S5F1 reporting 是否在範圍（建議獨立工作項）。

---

### F11-uHGemHT160-S7F6-recipe

**位置**：`HT160S_Program_BCB_V1.0.0.0/SecsGem/uHGemHT160.cpp` —— `S7F2_ProcessProgramLoadGrant()` `883`、`S7F4_ProcessProgramAcknowledge()` `890`、`S7F6_ProcessProgramData()` `896`、`S7F6_ProcessProgramData(AnsiString)` `902`，皆 log-only 樁；宣告 `uHGemHT160.h:94-97`。Dispatch 已接（`uHGemClass.cpp:100-108`）。

**現況**：四個 S7 處理器只 StringOut "disabled for HT160 framework skeleton"，不讀訊息、不解析 recipe、不回覆 → host S7F1/F3/F5 只得到 log 行（W-bit 會 T3 timeout）。S7F18/S7F20 未在 `uHGemHT160.h` override，落到 base `SendUnsupported`。HT172 參考 body 在 `uHGemHT172.cpp:1362/1411/1520/1557`。**三個硬性不相容**：(1) `S9F7_IllegalData` 與 `LocalAcknowledge` 在 HT160 不存在（HT160 只有 `S9F3_Unrecognized_Stream_Function_Type`，`uHGemClass.cpp:232`）；(2) Recipe 模型是「資料夾式」非 flat `.ini`（`CosFunction.cpp:63-89,135`：`GetDataRootPath()=<CurrentDir>\\data`、recipe 是含 `setup.ini`/`BinAreaMap.ini` 的目錄，`RecipeExists()=DirectoryExists()`）；(3) HT160 無 `memoPPBody`。`SKILL.md:35` 把「recipe 上下傳(S7)」列在「仍待補」，與「real host 現場驗證」同組（屬延後里程碑）。

**修法步驟**：
1. **決策閘（STEP 0）**：先答三問 —— 傳輸方向（equip→host 上傳 S7F5/F6 / host→equip 下載 S7F1/F2/F3/F4 / 兩者）、PPID↔recipe 資料夾對應與磁碟格式、S7 是否在當期里程碑（`SKILL.md:35`）。資料夾 vs flat-ini 不相容讓磁碟對應成為 load-bearing。
2. **STEP 1（補缺失原語）**：在 `uHGemClass.h/.cpp` 加 `S9F7_IllegalData(AnsiString)`（仿 `S9F3` `uHGemClass.cpp:232`，真送 S9F7 或至少 StringOut），與替代 `LocalAcknowledge(7,4,0)` 的 `ProcessProgramAck` helper（`InitLocalHead(7,4,0)+DataItemOut(1,BINARY_TYPE,&ACKC7)+SendLocalData()`）。
3. **STEP 2（S7F2）**：移植 `uHGemHT172.cpp:1362-1409`，讀 PPID，算 HCACK（`HSys.Sys.SystemStart`→6 / PPID 空→7 / `HasICUnderMachine()`→9 / else 0），可用手動掃描非法字元 `/\\*?:"<>|` 取代 TRegExpr（HCACK=8），回 `InitLocalHead(7,2,0)+DataItemOut(1,BINARY_TYPE,&HCACK)`。
4. **STEP 3（S7F6 兩版）**：移植 `1520-1606`，改用 `RecipeManager`（`RecipeExists`/`GetRecipeFileName`）解析（依 STEP-0 對應），缺檔呼叫 `S9F7_IllegalData` 後 return；載入 TStringList，CRLF escape，回 `L,2{A PPID, A body}`；不可用 `memoPPBody`。
5. **STEP 4（S7F4）**：移植 `1411-1518`，讀 `L,2{PPID, PPBODY}`，逐字節 CRLF-unescape（HT172 `1452-1497` 純 C 迴圈可照抄），`RecipeManager` 解目的路徑 + `ForceDirectories`，SaveToFile，回 ack（ACKC7=0）。
6. **STEP 5（S7F18/S7F20）**：在 `uHGemHT160.h:97` 後加兩個 virtual + 實作；S7F18 回 ack；S7F20 列舉 recipe 資料夾或回空 `L,0`。
7. **STEP 6**：標頭簽名同步（S7F2 回 int，S7F4/S7F6 void）；新增原語宣告加入 `uHGemClass.h`。
8. **STEP 7**：用 `D:\AI_Area\Tool\HT160S_SECS_Simulator` 驗 S7F1→F2、S7F5→F6、S7F3→F4，確認無 T3 timeout、缺 PPID 回 S9F7。

**要動的檔**：`SecsGem/uHGemHT160.cpp`、`SecsGem/uHGemHT160.h`、`SecsGem/uHGemClass.cpp`、`SecsGem/uHGemClass.h`。

**build 驗證**：S7F18/S7F20 新增 virtual = vtable 變更 → 刪 `uHGemHT160.obj`、`uHGemClass.obj`（必要時 `uHGemEquipment.obj`）後 `-Clean`（必要時 `-Full`）；encoding check 須過。

**風險/工數**：medium / L。

**仍需決策**：傳輸方向、PPID↔資料夾對應與格式、S7 是否在里程碑（皆來自 needsDecision，已由原始碼證實為真）。**工程阻擋（非決策）**：`S9F7_IllegalData`/`LocalAcknowledge` 不存在須先補；確認 TRegExpr 是否可連結（否則用手動掃描）。硬體：編譯不需；round-trip 驗證需 real host 或模擬器。

---

### F24-uHGemEquipment-IsEnableEvent

**位置**：`HT160S_Program_BCB_V1.0.0.0/SecsGem/uHGemEquipment.cpp:353-356`（`IsEnableEvent` 僅 `return true;`）；宣告 `uHGemEquipment.h:244`。

**現況**：死的（全 SecsGem 樹無 caller）。`EventReport`（`uHGemEquipment.cpp:307-351`）不查 `IsEnableEvent`，僅 gate `iHsmsState!=HSMS_STATE_SELECTED`(316) 後無條件送每個 CEID 的 S6F11 → per-event 啟用過濾真的遺失。`TGemCEIDItem`（`uHGemEquipment.h:96-102`）無 `Enabled` 旗標。S2F37 解碼所需 API 已存在（`DataItemIn`、`GetDataItemLenAndType(AndDelete)`、`BOOLEAN_TYPE`/`BINARY_TYPE`）。Dispatch（`uHGemClass.cpp:51-125`）case 2 無 `case 37`；`S2F38_...Acknowledge`（`uHGemClass.cpp:167`）是 `SendUnsupported("S2F38")` 樁 → host S2F37 落到 S9F3。HT172 參考：`IsEnableEvent` `7688`、`EventReport` 包 `if(IsEnableEvent(...))` `7707`、`EnableDisableEventReport` `8568`、S2F38 解析 `uHGemClass.cpp:1488`。HT172 找不到 CEID 回 false；建議 HT160 預設 `Enabled=true` 以保今日 report-all 行為。

**修法步驟**：
1. **決策閘**：選 A（完整 S2F37/S2F38 移植）/ B（保 `return true` + ASCII TODO，安全且小）/ C（刪死樁）。本線若無 host 送 S2F37，建議 B。
2. **STEP 1（struct 旗標）**：`uHGemEquipment.h` `TGemCEIDItem` 加 `bool Enabled;`；`SetCEIDContent`（`uHGemEquipment.cpp:681-686` new-item 區）設 `p->Enabled=true;`。
3. **STEP 2（真過濾）**：`IsEnableEvent` 改 `TGemCEIDItem *Ce=FindCEIDItem(iCeid); return (Ce!=NULL)?Ce->Enabled:true;`。
4. **STEP 3（gate report）**：`EventReport` 在 `HSMS_STATE_SELECTED` 檢查後、`RefreshSVData` 前插 `if(!IsEnableEvent(iDataID, iCeid)) { StringOut("[SECS][TX] S6F11 skipped (CEID disabled by host)"); return; }`。
5. **STEP 4（apply + ack）**：加 `THGem::ApplyEventEnable(bool CEED, int slen, unsigned *CEID)`（移植 HT172 `8568`：slen==0 全設、否則逐 CEID `FindCEIDItem` 設 `Enabled=CEED`）與 `THGem::SendS2F38(unsigned char ErrCode)`（`InitLocalHead(2,38,0)+DataItemOut(1,BINARY_TYPE,&ErrCode)+SendLocalData()`）。
6. **STEP 5（解析 + dispatch）**：實作 `S2F38_EnableDisableEventReportAcknowledge`（移植 HT172 `1488` 解 `L[2]{ BOOLEAN CEED, L{CEID} }`，缺 CEID 回 0x01），dispatch case 2 加 `case 37: S2F38_...(); return;`。
7. **STEP 6**：ack helper/parser 歸屬 —— `SendS2F38`/`ApplyEventEnable` 放 THGem（equipment 側），由 HTGem 透過 `HGemPtr` 呼叫；先確認 HT160 `HGemPtr` 成員名。

**要動的檔**：`SecsGem/uHGemEquipment.h`、`SecsGem/uHGemEquipment.cpp`、`SecsGem/uHGemClass.h`、`SecsGem/uHGemClass.cpp`。

**build 驗證**：`TGemCEIDItem` struct layout 變更 → 刪 `uHGemEquipment.obj`、`uHGemClass.obj`（+ `uHGemHT160.obj` 若觸及）後 `-Full`；encoding check；Big5 鄰行用 byte-safe editor。

**風險/工數**：medium / M。

**仍需決策**：(1) A/B/C —— 僅在 host 整合規格要求 S2F37/S2F38 才做 A，否則 B；(2) CEED-空清單=全部啟/停、未配置 CEID 預設 `Enabled=true`；(3) 持久化（HT172 存 `EventReport_CEID.def`，HT160 `SaveEventReportData` 是 placeholder —— enable 狀態是否須跨重啟）；(4) `HGemPtr` 成員名須先確認。無硬體，可用模擬器驗。

---

### F4-csystem-RecordSafeDoorStates-empty（合併 F4-safedoor-state-record —— 已修正）

**位置**：稽核標的 `csystem.cpp:1120`/`951` 已過期；真實實作在 `csystem.cpp:1165-1210`（20260619），呼叫端 `DoSystem()` `csystem.cpp:652`。

**現況（已解決，稽核快照過期）**：`RecordSafeDoorStates()` **已完整實作**，符合稽核所述 minimal port：`static bool bClear` + `static bool bDoorOpen[6]`；`SystemStart` 時清一次 return；停機時逐一檢查 6 個離散門感測（`SnSafeDoorFront/Right/Left/SlideDoorRight/SlideDoorLeft/SafeAuto6`）`Enable && IsOff()`，false→true 沿觸發 `RecordProcess(...)` 並 latch。正確丟棄 HT172 PLC-safety branch（`Enable_PLCSafety_IO/bPLCIOEffect/bIOPowered` 在 HT160 不存在）。符號型別皆確認（`TMySensor::Enable` `mysensor.h:28`、`IsOff()` `mysensor.h:35`、`RecordProcess(AnsiString)` `note.h:163`）。舊備份 `csystem.cpp.bak_unloadinfo_20260618`（line 960）仍含舊空 body —— 該備份**不編譯**，是稽核誤判來源。

**修法步驟**：
1. 無需原始碼編輯 —— 重讀 `csystem.cpp:1165-1210` 確認已實作。
2. （驗證）確認呼叫點 live：`DoSystem()` `csystem.cpp:652`。
3. （驗證）確認符號可用。
4. （可選清理，獨立於本項）移除過期非編譯備份 `csystem.cpp.bak_unloadinfo_20260618`。
5. 由於與其他未提交變更並存，可刪 `csystem.obj` 跑確認建置。
6. 結案為 already-fixed。

**要動的檔**：`csystem.cpp`（唯讀驗證，已實作）；可選清理過期備份。

**build 驗證**：刪 `csystem.obj` → `-Clean` 確認編譯 clean。不套用任何編輯。

**風險/工數**：low / S。

**仍需決策**：無阻擋。兩個 needsDecision 已由實作 code 解決（採離散門感測、丟棄 PLC-safety branch）。`RecordProcess` 為 sink，與鄰近 `ScanSystemSenser` MACHINE STOP/PAUSE record 一致。唯一 cosmetic 偏差：`pName[]` 直接存完整訊息字串而非 `Name`+sprintf，行為等價且更清楚。

---

### F2-gem-SaveEventReportData-placeholder

**位置**：`HT160S_Program_BCB_V1.0.0.0/SecsGem/uHGemEquipment.cpp:302-305`（`SaveEventReportData()` 僅 `StringOut("[SECS] SaveEventReportData placeholder")`）；配對 `ReadEventReportData()` `668-670` 為空 `{}`；宣告 `uHGemEquipment.h:242,282`。2 個呼叫端：`uHGemHT160.cpp:300`、`UsecegemMainFrom.cpp:91`。

**現況**：登錄資料在 heap TList（`ReportList`/`CEIDList`，`uHGemEquipment.h:90-102`），僅由 `SECS_SETData` 程式碼填充、每次開機重建。Host 報表定義（S2F33/35/37）不被接受（S2F34/36/38 皆 `SendUnsupported`，`uHGemClass.cpp:165-167`），故無任何 host 可變更內容需持久化。HT172 版（`uHGemEquipment.cpp:8604-8619`/`8146-8173`）序列化 GUI TStringGrid 到 `SecsGemPath\\SYSTEM\\*.def`；HT160 無表單、無 grid、無 `SecsGemPath`（SECS 目錄為 `sRecipeDirectory='..\\data\\'`）。

**修法步驟**：
1. **決策閘**：選 A（維持 by-design no-op）/ B（移除誤導樁 + 2 呼叫端 + placeholder log）/ C（完整 save+load）。建議 A 或 B。
2. **Option B（建議）**：刪 `uHGemHT160.cpp:300`、`UsecegemMainFrom.cpp:91` 兩呼叫，刪 `uHGemEquipment.cpp:302-305` 與空 `668-670`，刪宣告 `242,282`。執行期行為不變。
3. **Option C（僅當 host 定義報表在範圍）**：用 `sRecipeDirectory` 組檔名 `EventReport_ReportID.def`/`EventReport_CEID.def`；`SaveEventReportData()` 迭代 TList 以 tab 格式輸出；`ReadEventReportData()` 反向解析並用既有 `SetReportIDContent()/SetCEIDContent()` 重填；決定載入順序（開機 `SECS_SETData` 後載入才能讓磁碟覆蓋預設）。C 須先移植 S2F33/35/37 receive handler 才有意義。

**要動的檔**：`SecsGem/uHGemEquipment.cpp/.h`、`SecsGem/uHGemHT160.cpp`、`SecsGem/UsecegemMainFrom.cpp`。

**build 驗證**：刪各觸及 `.obj` 後 `-Clean`；若標頭宣告變更用 `-Full`；encoding check。

**風險/工數**：low / S。

**仍需決策**：事件報表持久化是否在範圍。今日登錄全程式碼定義、開機重建，host 無法變更，故無物可存。C 僅在 S2F33/35/37 移植後才有價值。若選 C 須確認 `..\data\` 目錄存在且可寫。

---

## gap-hardware 類

### F4-aAuto1To6-IsAmrTaken / F1-aAuto1To6-IsAmrTaken-TODO / F2-aAuto1To6-IsAmrTaken-TBD-comment / F11-aAuto1To6h-IsAmrTaken-decl-TBD（合併 —— 同一掛點）

**位置**：`HT160S_Program_BCB_V1.0.0.0/aAuto1To6.cpp:971-978`（`IsAmrTaken(int Index)`，OOR guard→false；`if(IsSoftSimulate()) return true;`；行 977 `return false; // TODO`）；設計註解 `967-970`；宣告 `aAuto1To6.h:118`。消費者 `SecsGem/uAgvStation.cpp:207`（AGV_READY 分支，true 時 fire CEID274 + `ClearAmrCar(a)` `aAuto1To6.cpp:983-990` + 設 AGV_IDLE）。

**現況**：實機硬回 false → 交握永久卡在 AGV_READY，CEID274 永不觸發、產線鎖死該 Auto。下游全已完成（`ClearAmrCar` 已實作）。可仿的 accessor 樣式已存在：`GetInputFullTray`（`aAuto1To6.cpp:246-258`，switch 回 `&HSys.Sen.SnAutoX_InputFullTray`），消費於 `IsOutputCarFullForAmr`（`929-937`）。正確型別是 `TMySensor`（非稽核 fixSummary 誤寫的 `THTSensor`），慣用判斷 `Sn!=NULL && Sn->Enable==true && Sn->IsOn()`。感測點綁定鏈：`database.h` `SENSOR_MODULAR` struct（`296-331`）→ `database.cpp` `InitialSensorName()`（`926-961`）→ `LoadSensorParameterFromDataBase()`（`997-1014`）依 `.Name` 對 `system/IO_Table.csv` 自動綁。HT172 **無**取車/AMR 感測（`uAgvStation` 為 HT160-original）。

**修法步驟（Option A：專用 per-Auto 取車感測，最乾淨）**：
1. **決策閘**：選來源 —— a=新 per-Auto 取車感測（6 點）/ b=反相沿用既有 `SnAutoX_OutputHasTray`（無新線，誤觸風險）/ c=AGV 端 SECS 訊息驅動 Finish（`IsAmrTaken` 維持 sim-only，改在 `uAgvStation` AGV_READY 分支處理）。
2. `IO_Table.csv` 加 6 列 `SnAuto1_CarTaken..SnAuto6_CarTaken`（沿用既有 Auto 感測欄位佈局，如 row 47 `Sensor,SnAuto1_InputFullTray,0,0,3,0,5,0,0,1,...`；實際 MN200 位址待硬體圖；位址未定前 `Enable=0`，等同今日 false）。
3. `database.h` `SENSOR_MODULAR` 在 `~331` 加 6 個 `TMySensor SnAutoX_CarTaken`。
4. `database.cpp` `InitialSensorName()` `~961` 後加 6 行 `.Name` 字串（須精確等於 CSV alias）。
5. `aAuto1To6.cpp` 仿 `GetInputFullTray` 加 `GetCarTakenSensor(int Index)`（switch 0..5），宣告於 `aAuto1To6.h` `~70`。
6. `aAuto1To6.cpp:977` 改為 `TMySensor *Sn=GetCarTakenSensor(Index); return (Sn!=NULL && Sn->Enable==true && Sn->IsOn());`（保留上方 `IsSoftSimulate()` 短路）；極性依電氣規格（`IsOn()`=已取走，否則改 `!Sn->IsOn()`）。
7. 更新 `aAuto1To6.h:118` 與 `967-970` 註解去除 TBD（ASCII）。
8. （可選）無感測機台不被永久卡：加守衛的操作員手動 clear（`bUseAMR && AGV_READY` 下呼叫 `ClearAmrCar` + 重置 `Handshake[si]=AGV_IDLE`）。
9. 若選 c：跳過感測步驟，改在 `uAgvStation` AGV_READY 分支處理 AGV-finish SECS 訊息直接呼叫 `ClearAmrCar(a)`。

**要動的檔**：`aAuto1To6.cpp`、`aAuto1To6.h`、`database.h`、`database.cpp`、`system/IO_Table.csv`（Option c 改 `SecsGem/` + `uAgvStation.cpp`，`IsAmrTaken` 維持 sim-only）。

**build 驗證**：Option a 改 `database.h` struct → `-Full`；再 encoding check。Option c 無 `database.h` 變更 → 刪 `aAuto1To6.obj` 用 `-Clean`。改 Big5 檔用 `scripts/ops/bcb6-bytesafe-edit.ps1`，勿用 Edit。

**風險/工數**：low / S（F2/F11 視角標 medium，因實機誤極性會誤觸 CEID274+ClearAmrCar）。

**仍需決策**：(1) 哪個訊號代表 AGV 已取走滿車（a 新感測 / b 反相 OutputHasTray / c SECS 交握）；(2) 極性與 debounce；(3) 6 點位址配置；(4) 是否加 interim 手動 clear。Code 可先以 `Enable=0` 列合併，決策只阻擋「讓讀取上線」而非骨架。位址/極性未定前，維持今日 sim-true/real-false 為正確安全停泊。

---

### F2-tapefeed-ringcatch-cassette-teach

**位置**：`HT160S_Program_BCB_V1.0.0.0/uteach.h:22-43`（19 個死的 tape-feed/ring-catch/cassette teach struct 成員）。

**現況**：稽核 `method` 為 null（未深查），但 fixSummary 指出這 19 個成員（`LoadCassetteFirstRingToLoadArmZPosition` … `TapeIn_WaitTapeVacuumDistance`）從未 `AddTeachItem`-wired、從未持久化、從未讀取。證據強烈顯示 HT160 刻意無 tape-feed/ring-catch/cassette 機構（所有驅動模組已移除、無馬達接線、無 task code）。另有 `iosetview` 內惰性的 tape/ring IO-view 分頁（`ts_IOTapeLoadUnload/ts_IOTapeShuttle/ts_IORingCatch` 與對應按鈕 `sbIOTapeShuttle/sbIOTapeLoadUnload/sbIORingCatchArm`）屬另一獨立、較重的 DFM/.h/.cpp 清理。

**修法步驟**：
1. **決策閘**：確認是「真的能力缺口需重建」或「確定永久移除的死碼待清」。證據傾向後者。
2. 若確認移除（Option 1）：用 `scripts/ops/bcb6-bytesafe-edit.ps1` 刪 `uteach.h:22-43` 那 19 個成員（單一標頭、單一 struct；無 `.cpp` 讀者，無 tech.ini key 曾寫入故無遷移）。
3. （可選後續，獨立 finding）一併移除 `iosetview` 惰性 tape/ring IO-view 分頁與按鈕。
4. 若未來機種需要（Option 2）：保留欄位 + 加 ASCII TODO 註解，視為延後。
5. 零風險替代：保留欄位 + 加 ASCII `DEAD - HT160 has no tape-feed/ring-catch/cassette mechanism` 註解。

**要動的檔**：`uteach.h`（可選擴及 `iosetview.dfm/.h/.cpp`）。

**build 驗證**：刪 `uteach.obj` 後編譯（無其他 TU 引用這些成員，`-Full` 非必要）；改 Big5 用 byte-safe editor；encoding check。

**風險/工數**：未評（屬 cosmetic 死碼移除）。

**仍需決策**：是真缺口待重實作，或確認永久移除待清理。建議 Option 1（移除）除非產品擁有者確認此機構在路線圖上。

---

## orphan-ui 類

### F18-aColor-IsAcceptingIC

**位置**：`HT160S_Program_BCB_V1.0.0.0/aColor.cpp:627-630`（`bool TColorModule::IsAcceptingIC(){ return false; }`）；宣告 `aColor.h:66`。全樹除自身宣告/定義外 0 呼叫者。

**現況**：屬一個完全孤立的 IC-accounting 叢集（皆無外部呼叫者）：`IsAcceptingIC`（`.h:66/.cpp:627`）、`NotifyICPlaced(int)`（`.h:69/.cpp:644`）、`GetICCount()`（`.h:72/.cpp:662`）、`SetSupplyThreshold(int)`（`.h:70/.cpp:650`）、`GetSupplyThreshold()`（`.h:71/.cpp:657`），及僅自寫的欄位 `iICCount`（`.h:29`，init 0 於 `.cpp:31`）、`iSupplyThreshold`（`.h:28`，init 100 於 `.cpp:22`）。純 scaffolding。**HT172 無對應**（`TColorModule` 為 HT160-original），故 Option B 無參考設計可移植。兄弟方法 `IsTrayReady/RequestSupplyTray/NotifyTrayPicked/GetTrayID` 有真實接線，**不可動**。

**修法步驟（建議 Option A，擴及整個孤兒叢集）**：
1. `aColor.h` 刪 5 個方法宣告（行 66、69、70、71、72）。
2. `aColor.h` 刪 2 個僅該叢集用的欄位（行 28 `iSupplyThreshold`、行 29 `iICCount`）。
3. `aColor.cpp` 刪 5 個定義（含 banner 註解）：`IsAcceptingIC`(627-630)、`NotifyICPlaced`(644-648)、`SetSupplyThreshold`(650-655)、`GetSupplyThreshold`(657-660)、`GetICCount`(662-665)；移除多餘 `//----` 分隔。
4. `aColor.cpp` ctor/init 移除 `iSupplyThreshold=100;`（行 22）與 `iICCount=0;`（行 31）。
5. grep 驗證 `IsAcceptingIC|NotifyICPlaced|GetICCount|SetSupplyThreshold|GetSupplyThreshold|iICCount|iSupplyThreshold` 全樹 0 殘留。**勿動** `IsTrayReady/RequestSupplyTray/NotifyTrayPicked/GetTrayID`。
6. 替代（僅改旗標符號）：只刪 `IsAcceptingIC` 宣告+定義（不建議，留 4 個孤兒）。
7. Option B（不建議）：無 HT172 可移植；若日後復活，實作 `return IsInstalled() && IsTraySupplyMode() && bTrayPicked==false && iICCount<iSupplyThreshold;` 並在 sortarm/tray placement 加真實呼叫者。

**要動的檔**：`aColor.h`、`aColor.cpp`。

**build 驗證**：移除 2 個私有欄位改 struct → `-Clean`（必要時 `-Full`）；link clean 證明確無呼叫者；encoding check。改 Big5 用 byte-safe editor。

**風險/工數**：low / S。

**仍需決策**：Color IC-accounting 叢集是「計畫但延後」或「廢棄 scaffolding」。證據（0 呼叫者 + HT172 無對應）指向廢棄 → 建議 Option A。唯一阻擋是 owner 確認 Color 供盤線不會出「IC 置入已呈現盤」功能。

---

### F1-systools-emptyform

**位置**：`HT160S_Program_BCB_V1.0.0.0/systools.cpp:9-14`（空 ctor）；`systools.h:10-16` 空 class。接線：`main.dfm:766-908`（`sbTool: TSpeedButton`，Caption 'Tools'，`OnClick=sbToolClick`）、`main.h:106/371`、`main.cpp:554-557`（`ShowTopForm(FormSysTools, sbTool)` → 顯示空白頁）、`main.cpp:27` include、`main.cpp:1302-1307` SmokeShow probe、`ht160s.cpp:17/88/136`、`ht160s.bpr:9/29/91`。

**現況**：確認空殼。HT172 `FormSysTools` 3411 行內部多已移至 `SYSTEM_MODULAR/database.cpp`；無 HT160 呼叫者需要它。其餘只剩 comment-only 參考（`uspeed.cpp:15`、`uspeed.h:16`、`cEventLog.h:4`，無編譯相依）。

**修法步驟**：
1. **決策閘**：Tools 頁是否會回來？永久樁→Option 2 全移除；可能再填→Option 1 隱藏。
2. **Option 1（建議，可逆）**：在 `main.dfm` `sbTool` 區塊加 `Visible = False`；或更安全 —— 在 `TfMain::FormShow/FormCreate` 末尾加 `sbTool->Visible = false;`（避免重存 DFM）。
3. **Option 2（全移除）**：刪 `main.dfm:766-908` 整個 `sbTool` 區塊、`main.h:106/371`、`main.cpp:554-557` handler、`main.cpp:1302-1307` probe、`main.cpp:27` include、`ht160s.cpp:17/88/136`、`ht160s.bpr:9/29/91`，刪 `systools.cpp/.h/.dfm`；comment-only refs 不需動。
4. **編輯注意**：`main.dfm` 為 Big5；勿用 plain Edit、勿開 BCB designer（會 strip 元件 + CRLF→LF）；用 `scripts/ops/bcb6-bytesafe-edit.ps1`，先備份 `main.dfm`+`main.h`。
5. 驗證：sim 啟動確認 Tools 鈕消失/隱形，按下原位不再開空白頁；Option 1 self-test `Tools:create/Tools:show` 仍過；Option 2 確認 self-test 不再參考 Tools。

**要動的檔**：`main.dfm`、`main.cpp`、`main.h`、`ht160s.cpp`、`ht160s.bpr`、`systools.cpp/.h/.dfm`。

**build 驗證**：Option 1（僅 `main.cpp` 或 `main.dfm`）→ 刪 `main.obj` 跑 `-Clean`。Option 2（改 BPR + USEFORM/CreateForm 接線）→ `-Full`。encoding check 須過。

**風險/工數**：low / S。

**仍需決策**：Tools 頁是否會回來。建議 Option 1 init-side（最小、可逆、不重存 DFM）。次要注意：Big5 DFM 須 byte-safe 編輯；Option 2 動 BPR + 表單註冊須 full build。

---

### ORPH-iosetview-spbTerminalProgram

**位置**：`HT160S_Program_BCB_V1.0.0.0/iosetview.cpp:2162-2165`（`spbTerminalProgramClick` 空樁）；宣告 `iosetview.h:95`；`iosetview.dfm:633`（`Visible=False` `776`，`OnClick` `777`）。

**現況**：按鈕存在但接空樁且隱藏（無害死碼）。`systools.cpp`（HT160）無 `TerminalProgram()`、無 `MyThread` 成員。HT172 行為：`Application->MessageBox` 確認 → `fMaintenance->SaveData()` + `FormSysTools->TerminalProgram()`（`Suspend()`+`Terminate()`）。HT160 真實 teardown 是 `MyThread`（`TRunControl`，`uruncontrol.h:17`），`main.cpp:575-578` 已用 `Terminate()`+`WaitFor()`；HT160 無 `MyPad232Thread`、無 `fMaintenance->SaveData()`（只有 granular `Save*` `maintenance.h:329-361`）。`mymessbox.h` 已透過 `IncludeAllHeader.h` 可用；`uruncontrol.h` 須明確 include 到觸 `MyThread` 的 `.cpp`。

**修法步驟**：
1. **決策閘**：是否需 in-app 關程式離開路徑？A=退役（移除孤兒）/ B=還原。
2. **Option A（退役）**：byte-safe 刪 `iosetview.dfm:633-778` 整個 `spbTerminalProgram` 區塊（含 OnClick），刪 `iosetview.h:95` 宣告，刪 `iosetview.cpp:2161-2165` 樁。勿開 IDE designer。
3. **Option B（還原，適配 HT160）**：
   - `systools.h` 加 `void TerminalProgram();`；`systools.cpp` `#include "uruncontrol.h"` 後實作 `if(MyThread!=NULL){ MyThread->Terminate(); MyThread->WaitFor(); } Application->Terminate();`（用 HT160 的 Terminate+WaitFor，**非** HT172 Suspend，避免硬體 mid-motion）。
   - `iosetview.cpp:2162` 改用 `if(ShowMyMessageBox_YES_NO("Close program now?")==TMyMessageBox::msgrtnYES){ FormSysTools->TerminalProgram(); }`（依專案規則禁用 VCL MessageBox，仿 `iosetview.cpp:2201`）；必要時 `#include "systools.h"`。
   - 存檔：HT160 無單一 `SaveData()`；與 owner 確認是否需呼叫具體 `Save*`（null guard）。
   - 可見性：HT172 在 code 設 `Visible=true`；找 HT160 維護/工程模式分支才設可見，否則維持隱藏並文件化。

**要動的檔**：`iosetview.cpp`、`iosetview.h`、`iosetview.dfm`（Option B 另加 `systools.cpp/.h`）。

**build 驗證**：Option A 刪 `iosetview.obj`；Option B 另刪 `systools.obj` 並因標頭變更建議 `-Full`；encoding check；DFM 用 byte-safe editor。

**風險/工數**：medium / S。

**仍需決策**：(1) 是否需 in-app 離開；(2) 在 HT160 安全門 + 單 MyThread 模型下從此頁關程式是否可接受（須 gate 在無料/維護檢查）；(3) 可見性政策；(4) 是否需 save-on-exit 及哪些 `Save*`。

---

### ORPH-iosetview-ComboBox1Change

**位置**：`HT160S_Program_BCB_V1.0.0.0/iosetview.cpp:1953-1956`（`ComboBox1Change` 空 no-op）。相關空樁 `sbEnableIOChangClick` `1994-1997`。

**現況**：loop-time 下拉與 "Enable IO Change" 鈕皆 no-op，組成孤兒 UI 對。`iosetview.h` 宣告 `ComboBox1`(L48)、`ComboBox1Change`(L82)、`sbEnableIOChangClick`(L88)。DFM `pn_IOTool3` 內含 `ComboBox1`(9591-9614)、`sbEnableIOChang`(9549-9564)、`lb_IOToolOB1/OB2`、`cbToolBit0..15`、`ed_OutPort_1`。`Timer1Timer`（`2167-2175`）為重構後的 live-view 刷新（無 TimeTick/TimeCT/blink）。HT172 blink 功能（`ComboBox1Change` `2044-2066` + `Timer1Timer/ScanIOTable` `661/705/708-798`）整個 substrate（CheckBox1-32、port edits、ALeds、loop toggle）在 HT160 DFM **完全不存在**（2026-06-13 重構刻意以 live alias-bound BtnPanel 取代 raw byte-level IO）。

**修法步驟（建議 Direction A：移除死碼）**：
1. **決策閘**：A=刪死 UI（單點、低風險、符合重構意圖）/ B=重移植 HT172 blink 測試（多元件多檔，不建議）。
2. **STEP A1**（`iosetview.cpp`）：刪空 handler `ComboBox1Change`(1953-1956) 與 `sbEnableIOChangClick`(1994-1997)（含 `//---` 分隔）；`Timer1Timer` 不動。
3. **STEP A2**（`iosetview.h`）：刪宣告 `ComboBox1Change`(L82)、`sbEnableIOChangClick`(L88)、`TComboBox *ComboBox1;`(L48)；若一併刪 DFM 物件則同步刪 `ed_OutPort_1`(L46)、`cbToolBit0`(L47)。保持 `__published` 無註解規則。
4. **STEP A3**（`iosetview.dfm`）：byte-safe 刪 `ComboBox1`(9591-9614)、`sbEnableIOChang`(9549-9564)、`lb_IOToolOB1`(9565-9577)、`lb_IOToolOB2`(9578-9590)、`ed_OutPort_1`(8869-8881)、`cbToolBit0..15`(8921-9019, 9316-9414)；若 `pn_IOTool3` 空可一併刪。
5. **STEP A4（關鍵）**：`cbToolBit1..15` 等在 DFM 但 `.h` 無對應宣告 —— 用 byte-safe editor，**勿開 designer**（會 strip 其他元件 + CRLF→LF）；先備份 dfm+h。
6. **STEP A5**：全專案 grep 確認無其他引用後再刪。

**要動的檔**：`iosetview.cpp`、`iosetview.h`、`iosetview.dfm`。

**build 驗證**：刪 `iosetview.obj`（+ IDE backup），DFM/.h 結構變更 → 建議 `-Full`；encoding check；smoke test 確認表單仍 stream、live BtnPanel 仍可切換、loop 下拉/鈕已消失。

**風險/工數**：low / S。

**仍需決策**：A（刪）vs B（重移植）。證據強烈傾向 A（blink substrate 完全不存在）。DFM 編輯（任一方向）須 byte-safe，勿用 designer。

---

### ORPH-iosetview-sbEnableIOChang

**位置**：`HT160S_Program_BCB_V1.0.0.0/iosetview.cpp:1994-1997`（`sbEnableIOChangClick` 空 no-op）；宣告 `iosetview.h:88`；DFM `9549-9564`（父 `pn_IOTool3` `9541`）。

**現況**：真 no-op。IO commit 已由獨立路徑處理：`sbUpdateClick`(2129) → `SaveIoTableFromGrid()`(836) → `HSys.LoadIoData()`(898)，不需 edit-lock。原 HT160-Original handler 的相依（`ed_OutPort_2`、`ed_InPort_1/2`、`sbStopMacro`、`myIO[]`）在 HT160S_BCB **不存在** → 還原是錯的修法。同 `pn_IOTool3` 的 `ComboBox1` handler 亦 no-op，整面板皆死 UI。DFM 另有無 `.h` 宣告的孤兒元件（`ed_InPort_1`/`ed_OutPort_2`/`ed_InPort_2`）→ 表單脆弱，勿開 designer。

**修法步驟**：
1. **決策閘**：A=移除 / B=`Visible=False` 隱藏（最小 churn，建議）/ C=WontFix；及是否一併清整個 `pn_IOTool3`。
2. **Option B（建議）**：byte-safe 在 `sbEnableIOChang` 區塊加 `Visible = False`（插在 9554/9555 間），`.h`/`.cpp` 不動以保 streaming 一致；可選在 `.cpp` body 加 ASCII 註解說明 commit 已由 `SaveIoTableFromGrid` 處理。
3. **Option A（移除單鈕）**：byte-safe 刪 DFM `9549-9564`（含 `OnClick` `9563`）、`.h:88` 宣告、`.cpp:1993-1996` 樁。
4. **Option A-WIDE**：另刪 `ComboBox1`/`lb_IOToolOB1/OB2` 與其空 handler，`pn_IOTool3` 空則整面板刪。
5. 不從 HT172 移植任何東西（無對應）。改後跑 encoding check。

**要動的檔**：`iosetview.dfm`、`iosetview.h`、`iosetview.cpp`。

**build 驗證**：刪 `iosetview.obj` → `-Clean`（Option A/A-WIDE 動 `__published` 指標欄位，`-Clean` 足夠，謹慎可 `-Full`）；encoding check；確認表單執行期無 EReadError。

**風險/工數**：low / S。

**仍需決策**：A/B/C 與是否擴及 `pn_IOTool3`。B 最低結構風險。所有 DFM 編輯走 byte-safe，勿用 designer。

---

## dead-config 類

### F7-aLoader-TrayArmTakeout-not-wired

**位置**：`HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:51`（`static const int LOADER_Y_OWNER_TRAYARM=2;`）；保留註解 `aLoader.cpp:48-50`（稽核標 line:49 略差）。

**現況**：前向相容保留 token，非缺陷。Loader-Y ownership 3 值模型：`LOADER_Y_OWNER_NONE=0`(46)、`LOADER_Y_OWNER_SORTARM=1`(47)、`LOADER_Y_OWNER_TRAYARM=2`(51)。`LOADER_Y_OWNER_TRAYARM` 全專案僅 1 次出現（自身定義），0 讀者/寫者。NONE/SORTARM 活躍使用（`AcquireSortOwner/ReleaseSortOwner` `391-412`、`IsSortOwnerHeld` `425`、feed/pick gating `659/727`）。註解提及的 `LS_READY_TakeOut/LS_TakeOutIng` 兩狀態**不存在**（`eLoaderStatus` `aLoader.h:11-19` 只到 `LS_ToRear=5`）。零執行期成本（編譯期常數）。HT172 無對應 token。

**修法步驟**：
1. **決策閘**：rear-tray takeout 交握是否近期路線圖？是→Option A（保留，不動）。需清死 config→Option B。
2. **Option A（建議，預設）**：`aLoader.cpp:46-51` 不動（註解已準確、常數零成本、保留 3rd actor 設計目標）。無建置。
3. **Option B（僅當須清死 config）**：byte-safe 刪 `aLoader.cpp:51` 整行；修剪/刪 `48-50` 保留註解（移除 `LS_READY_TakeOut/LS_TakeOutIng` 字樣）；保 ASCII 新註解。
4. **勿動** `46-47`（NONE/SORTARM 活躍互鎖）。
5. **勿加** `LS_READY_TakeOut/LS_TakeOutIng` 到 enum（除非真做 takeout 功能，否則造成半成品狀態機）。
6. 未來真做時：重加 token、加兩狀態、加 TrayArm acquire/release（仿 `AcquireSortOwner`，procedural、無 FSM）。

**要動的檔**：`aLoader.cpp`（Option B only；A 不動）。

**build 驗證**：Option A 無建。Option B 刪 `aLoader.obj` → `-Clean`（無 struct/layout 變更，`-Full` 非必要）；encoding check。即使 ASCII 行也用 byte-safe editor（檔為 Big5）。

**風險/工數**：low / S。

**仍需決策**：rear-tray takeout 是否近期路線圖（保留 Option A）或現在清死 config（Option B）。無硬體無 HT172 移植。

---

### F22-myio-IOInputLongByte

**位置**：`HT160S_Program_BCB_V1.0.0.0/myio.cpp:446-449`（`int TMyIo::IOInputLongByte(int port){ return 0; }`）；宣告 `myio.h:36`。

**現況**：死的、未呼叫的樁。三樹 grep（排除 docs）：HT160S_BCB 僅定義+宣告，0 呼叫；HT172 是 free function 且本身亦死樁（real body 註解掉、回 0、無呼叫）；HT160S-Original `myio_ISA.cpp:233` 有真 ISA `inportl` body 但屬 legacy ISA 路徑，HT160S_BCB 是 MN200/MotionNet，不適用。無物可移植。

**修法步驟**：
1. `myio.cpp` 刪 446-449 四行；保留 `IOInputByte`（~444）與 `IOByteOut`（~451）間恰一條分隔。
2. `myio.h` 刪 `int IOInputLongByte(int port);`（行 36）；`IOInputByte`(35) 與 `IOByteOut`(37) 相鄰即正確。
3. 不從 HT172 移植（其版亦死樁）。
4. （編碼安全）兩處皆 ASCII，但用 `scripts/ops/bcb6-bytesafe-edit.ps1` 避免周邊 Big5 被重編碼。
5. 替代（僅當未來真要 32-bit MN200 long-word 讀取）：保留方法但把 `return 0` 換成明確 ASCII TODO body。建議移除。

**要動的檔**：`myio.cpp`、`myio.h`。

**build 驗證**：`myio.h` 移除 class 成員（結構性變更）→ 因 `myio.h` 被廣泛 include，建議 `-Full`；encoding check 須無 `EF BF BD`/BOM；不得有 `IOInputLongByte` undefined 警告。

**風險/工數**：low / S。

**仍需決策**：刪除（建議，HT160 無 long-word IO 消費者且為 MN200-based）或保留為未來 placeholder（保留則須明確 TODO）。確認無 32-bit MN200 讀取路線圖即移除。

---

### F21-note-compat-empties

**位置**：`HT160S_Program_BCB_V1.0.0.0/note.cpp` —— `CheckCodeIsExist`(564-567 回 false)、`LevelRecordProcess`(820-822 空)、`CheckAlarmIsShow`(835-838 回 true)、`SetShowAlarmLocation`(840-842 空)、`SetShowSuckerLocation`(844-846 空)、`ShowImageTrayFuntion`(848-850 空)、`GetRefrenceCode`(852-855 回 "No Code")。宣告 `note.h`：`CheckCodeIsExist`(L137，public 非 __published，非 DFM-wired)、free functions L164/167-171。

**現況**：7 個皆空/瑣碎樁，全樹 grep 僅出現在定義+宣告，**0 呼叫者**，確認死碼。HT172 對應原較豐富，HT160 刻意縮為 no-op 並改走 `g_EventLog`，無物可移植。`GetRefrenceCode` 的 "No Code" 是與 HT172 `mapNameToAlarm` 查找的潛在分歧。`scripts/ops/bcb6-bytesafe-edit.ps1` 存在；`note.obj`(224680 bytes) 須先刪。**勿動** `LevelProcessErrMessage()`（亦空樁但不在本 7 符號清單）。

**修法步驟**：
1. **決策閘**：僅在更廣的 `note.cpp` 死碼清掃在範圍時才動；否則維持（或僅 step 8 註解）。預設建議移除。
2. byte-safe 備份 `note.cpp`/`note.h` 到 scratchpad。
3. `note.h` 用 byte-safe editor 刪 L137 `CheckCodeIsExist` 宣告（保留 L136/138）。
4. `note.h` 刪 6 個 free-function 原型（L164、167、168、169、170、171）；保留 `RecordProcess`(163)、`SearchMessage`(165)、`RecordAlarmMessagePassTime`(166)、`extern DWORD RecordHappenTime;`(172)。
5. `note.cpp` 刪 `CheckCodeIsExist`(564-567) + 一條相鄰 `//---`，保留 `LevelProcessErrMessage`(560-562)、`ProcessErrMessage`(569+)。
6. `note.cpp` 刪 6 個 free-function 定義（820-822、835-838、840-842、844-846、848-850、852-855）與分隔；保留 `RecordProcess`、`SearchMessage`(824-827)、`RecordAlarmMessagePassTime`(829-833)。
7. 跑 encoding check 確認無 `EF BF BD`/BOM。
8. 替代（保留）：原處不動，僅在 `GetRefrenceCode` 上方加 ASCII 註解說明它是未用相容樁且與 HT172 `mapNameToAlarm` 分歧。
9. 刪 `note.obj`。
10. `-Clean` 編譯，clean link 證明無隱藏引用。

**要動的檔**：`note.cpp`、`note.h`。

**build 驗證**：刪 `note.obj` → `-Clean`；clean link 證明 7 符號無隱藏引用；encoding check。無需執行期測試（無呼叫者）。

**風險/工數**：low / S。

**仍需決策**：移除 vs 保留為舊源/HT172 API parity shim（皆無 live 影響）。建議僅在更廣死碼清掃在範圍時移除。`GetRefrenceCode` "No Code" 分歧已在此記錄供未來重建真查找。

---

### DC-tFunction-RejectCCDfail

**位置**：`HT160S_Program_BCB_V1.0.0.0/cprod.h:77`（`bool RejectCCDfail;`）；`Config.cpp` SetDefault(29)、Load(46)、Save(61)。

**現況**：確認死碼，僅 4 行（1 宣告 + 3 config），0 消費。HT172 亦僅宣告不消費；相關 `iCCDFailBinTray`（HT172 `cmydef.h:400`）本身也死。HT160 CCD 路徑是樁：`ReadTopCcdBin`(`aLoader.cpp:580`) 只回 `HAS_OK_IC`/`EMPTY_IC`，從不產生 FAIL/NG → 無 CCD pass/fail 判定可供旗標 gate。Option B 無錨點、無參考可移植。

**修法步驟**：
1. **決策閘**：CCD-fail rejection 是否為 HT160S 功能？是=Option B（淨新工作，需 spec）/ 否=Option A。
2. **Option A**：`cprod.h` 刪 L77；`Config.cpp` 刪 29/46/61 三行。config.ini 殘留 key 被 TIniFile 無害忽略。
3. **Option B（僅確認要做）**：B1 先讓 `ReadTopCcdBin` 真產生 FAIL verdict；B2 在 `CcdTask` state 5000（`aLoader.cpp:966`）加 `if(tFunction.RejectCCDfail && verdict==FAIL){ SetTrayBin(...reject-bin...); record yield; }` 用既有 bin 路徑；B3 procedural/AnsiString/ASCII/no-C++11；B4 須先有 reject bin 與 status 碼 spec。

**要動的檔**：`cprod.h`、`Config.cpp`。

**build 驗證**：`TFunction` struct size 變更 → 建議 `-Full`（per memory 結構變更）；encoding check。

**風險/工數**：low / S。

**仍需決策**：CCD-fail rejection 是否要做。否→Option A trivial。是→三未指定項（無參考可移植）：CCD FAIL 偵測來源待建；reject bin/tray 對應；yield 計數器。建議 Option A 除非明確要求。

---

### DC-tFunction-UseHitCylinder

**位置**：`HT160S_Program_BCB_V1.0.0.0/cprod.h:78`（`bool UseHitCylinder;`，配對 `HitRetry` int L79）；`Config.cpp` SetDefault(30)、Load(47)、Save(62)。

**現況**：死 config round-trip，0 消費。唯一 live 成員是 `UseCCD`（`aLoader.cpp:586` 讀一次）。`Config.Save` 從不被呼叫且 config.ini 不在磁碟 → Load 在 FileExists guard(42) 早退，值維持 default false。HT172 亦宣告不消費，無物可移植。

**修法步驟**：
1. **決策閘**：HitCylinder 是計畫功能（B）或廢棄（A）？建議 A。
2. **Option A**：`cprod.h` 刪 L78 `UseHitCylinder` 與配對 L79 `HitRetry`（可選一併刪 L77/L80 兄弟）；保留 L76 `UseCCD`。`Config.cpp` 刪 SetDefault 30/31、Load 47/48、Save 62/63（可選含 29/32、46/49、61/64）。
3. **Option B（僅當硬體在路線圖）**：保留宣告 + Config 行，在 `aLoader.cpp:586` `UseCCD` 檢查附近加 retry step gate（`tFunction.UseHitCylinder`、最多 `tFunction.HitRetry` 次）。無 HT172 可移植 = 淨新邏輯，需硬體 spec。

**要動的檔**：`cprod.h`、`Config.cpp`。

**build 驗證**：`TFunction` layout 變更（被 `IncludeAllHeader.h` 全域可見）→ `-Full`（刪所有 obj 含 `Config.obj`/`cprod` 依賴/`aLoader.obj`）；改 Big5 `cprod.h` 用 byte-safe editor；encoding check。

**風險/工數**：low / S。

**仍需決策**：(1) HitCylinder 計畫 vs 廢棄；(2) Option A 窄範圍（UseHitCylinder+HitRetry）或全 [Function] 清理（含 RejectCCDfail/UsePreAlignment，稽核建議一起）；(3) Option B 需硬體/流程 spec。建議 A。

---

### DC-tFunction-HitRetry

**位置**：`HT160S_Program_BCB_V1.0.0.0/cprod.h:79`（`int HitRetry;`）；`Config.cpp` SetDefault(31)、Load(48)、Save(63)。

**現況**：grep 確認死 config，與 `UseHitCylinder` 配對。HT172 亦未消費。

**修法步驟**：
1. **決策閘**：確認 hit-cylinder retry 不在路線圖（否則 Option B 淨新 spec）。
2. **Option A**：`cprod.h` 刪 L79（可選含 L77/78/80 兄弟，保留 `UseCCD`）；`Config.cpp` 刪 31（可選 29/30/32）、48（46/47/49）、63（61/62/64），保留 28/45/60。
3. 殘留 [Function] HitRetry ini key 被 TIniFile 忽略，無遷移。純減法、無行為變更、無 FSM、無 C++11、無新註解。

**要動的檔**：`cprod.h`、`Config.cpp`。

**build 驗證**：`TFunction` 結構變更 → 建議 `-Full`；encoding check（稽核此項 buildVerify/blockers 欄位為佔位 "test"，以此處為準）。

**風險/工數**：low / S。

**仍需決策**：與 `UseHitCylinder` 同步（HitRetry 應與其一起移動）。建議與 `UseHitCylinder` 同一 pass 處理。

---

### DC-tFunction-UsePreAlignment

**位置**：`HT160S_Program_BCB_V1.0.0.0/cprod.h:80`（`bool UsePreAlignment;`）；`Config.cpp` SetDefault(32)、Load(49)、Save(64)。

**現況**：確認死 config，僅 4 行，0 消費。空的維護分頁 `tsMaintFunctionDef`（`maintenance.cpp:1255` 接入頁切換表，宣告 `maintenance.h:131`）綁定 0 控制項。HT172 亦僅宣告不消費、無 PreAlign 邏輯 → 無物可移植。兩編輯區皆純 ASCII。

**修法步驟**：
1. **決策閘**：保留或移除。預設建議移除（HT172 亦死宣告、無 consumer 可接）。
2. **Option A minimal**：`cprod.h` 刪 L80；`Config.cpp` 刪 32/49/64。
3. **Option A broader（僅當確認非路線圖）**：另刪 `RejectCCDfail`/`UseHitCylinder`/`HitRetry`，使 `TFunction` 僅留 `UseCCD`（獨立 opt-in pass）。
4. 用 `scripts/ops/bcb6-bytesafe-edit.ps1` 保 byte-safe。
5. 不動空 `tsMaintFunctionDef` 分頁（留空無害，移除分頁屬另一 UI 變更）。
6. 無物從 HT172 移植。

**要動的檔**：`cprod.h`、`Config.cpp`。

**build 驗證**：`cprod.h` 被廣泛 include 的 struct 變更 → `-Full`（per memory）；或最小化：刪 `Config.obj` → `-Clean`；encoding check。

**風險/工數**：low / S。

**仍需決策**：保留 vs 移除。HT172 亦死宣告無 consumer → 「保留並建功能」屬淨新開發，需真實產品/硬體需求（無證據支持）。3 兄弟清理需同樣確認。

---

### DC-GeneralSetting-iBinDispColor

**位置**：`HT160S_Program_BCB_V1.0.0.0/GeneralSetting.h:93`（`int iBinDispColor[9];`）；`GeneralSetting.cpp` SetDefault(50)、Load(84)、Save(115)。相關過期註解 `ComPort.cpp:164-165`。

**現況**：確認死 config，僅寫不讀（全 4 site）。真實消費者 `TfComPort::ApplyBinDisplayConfig()`（`ComPort.cpp:172-188`）只讀 `GeneralSetting.sBinDispText[i]`（live twin，`184`）並在執行期算色 `Color=((i+1)==ErrorArea)?BIN_COLOR_RED:BIN_COLOR_GREEN`（`185`）。`ComPort.cpp:164-165` 註解誤稱該陣列「留作 manual-test」（實際 manual-test 讀 `edMCULightValue` UI 欄位，非此陣列）。`GeneralSetting.h` 被 ~18 個 TU include。

**修法步驟**：
1. **決策閘**：A=刪死陣列（建議）/ B=接回成每單元 base color（ApplyBinDisplayConfig 用 `iBinDispColor[i]` 取代硬碼 GREEN，且需新維護 UI grid，今日不存在）。
2. **Option A**：`GeneralSetting.h:93` 刪該行（保留 `sBinDispText[9]` L92）；更新 `89-91` 註解去除 per-unit color；`GeneralSetting.cpp` SetDefault 刪 `iBinDispColor[i]=3;`(50)、Load 刪 ReadInteger(84)、Save 刪 WriteInteger(115)（各保留 sBinDispText）；`ComPort.cpp:164-165` 修正過期註解（移除「manual-test 用」子句，改述色由 error/non-error 狀態算出）。
3. 殘留 General.ini `[BinDisplay] Color0..8` 被忽略，無遷移。

**要動的檔**：`GeneralSetting.h`、`GeneralSetting.cpp`、`ComPort.cpp`。

**build 驗證**：移除標頭 struct 欄位 → 全 ~18 依賴須重編，**勿**只刪 obj；走 `-Clean`（不夠則 `-Full`）；encoding check；BCB6 須實際編譯成功才宣稱 clean。

**風險/工數**：low / S（但移除標頭欄位非單檔事件，須走 `-Clean`/`-Full`）。

**仍需決策**：A（刪）vs B（接回每單元色 + 新維護 UI grid）。B 僅在未來 spec 要每單元可設色才值得。無硬體需求。

---

### DC-MotTable-HomeOrder

**位置**：`HT160S_Program_BCB_V1.0.0.0/MotorAndIO/MyMotor.h:162`（`TStringList *HomeOrder;`）；`SetHomeOrder` 宣告 `MyMotor.h:228`、ctor `MyMotor.cpp:246`、dtor `271`、body `275-286`；`database.cpp:1574` 唯一呼叫；CSV 管線 `database.cpp:450/517-518/546`、`database.h:139/189`；Motor Test grid 鏡像 `uMotorTest.cpp:74/538/545/832/1093/1871`。

**現況**：`HomeOrder` TStringList 真正唯寫（配置、清除、賦值、釋放，**從不讀取**）。**根因**：這是不完整移植 —— HT172 中 `HomeOrder` 是 live 的（`uhome.cpp:1412-1435` 讀 `HomeOrder->Count`/`Strings[j]` 做 per-axis 回原點相依 gating），HT160 `uHome.cpp` 0 引用，該相依功能在 no-FSM 重寫時被丟掉、只留解析/儲存半邊。執行期無害。

**修法步驟**：
1. **決策閘**：A=移除唯寫鏡像 / B=全鏈移除（含 CSV/struct/grid，注意 positional enum）/ C=保留 + 文件化 / D=完成移植（讓 HOME 尊重 HomeOrder）。建議 A 或 C；B 僅在 schema 清掃；D 僅在 ops 真需順序回原點。
2. **Option A（建議 minimal）**：`MyMotor.h` 刪 162、228；`MyMotor.cpp` 刪 246、271、`SetHomeOrder` body 275-286；`database.cpp` 刪 1574。保留 450/517/518/546 + `TMOTDATA.HomeOrder` + grid 欄（CSV 仍解析、grid 唯讀顯示，無害）。
3. **Option B**：另刪 `database.cpp:450/517-518/546`、`database.h:139/189`、`uMotorTest.cpp:74/538/545/832/1093/1871`。注意 `eMpHomeOrder`/`emotHomeOrder` 為 positional，移除改變隱含欄序，須驗 grid 渲染與 `:518` fallback 鏈。
4. **Option C**：`MyMotor.h:162`、`database.cpp:1574` 加 ASCII 註解說明死碼根因。零風險。
5. **Option D（僅需順序回原點）**：保留解析/儲存，在 HT160 `uHome.cpp` 加 HomeOrder gate（移植 HT172 `1412-1435` 但 procedural、無 FSM、無 lambda/range-for/auto），需多 pass/retry 結構讓延後馬達後續再試。屬 M-L，超出 dead-config 範圍。

**要動的檔**：`MyMotor.h`、`MyMotor.cpp`、`database.cpp`（B 另 `database.h`/`uMotorTest.cpp`；D 另 `uHome.cpp`；可選 `system/Mot_Table.csv`）。

**build 驗證**：A/B 改 class layout → 刪 `MyMotor.obj`/`database.obj`（B 另 `uMotorTest.obj`）後 `-Full`；C 僅註解 → `-Clean`。改 Big5 用 byte-safe editor；encoding check。

**風險/工數**：low / S。

**仍需決策**：A/B/C/D。重要脈絡：HomeOrder 在 HT172 是真實回原點順序相依功能、僅半移植入 HT160；若 ops 要依相依順序回原點，正解是 Option D（移植 consumer）而非刪除。刪除前須與 owner 確認順序回原點是否為必要行為。

---

### DC-Motor-OriginRangeRate

**位置**：`HT160S_Program_BCB_V1.0.0.0/MotorAndIO/MyMotor.h:149-150`（`int OriginRate; int OriginRange;`）；ctor init `MyMotor.cpp:255-256`；DB 寫入 `database.cpp:1581-1582`。

**現況**：確認死 —— 唯寫 shadow 欄位，0 讀者（全樹 grep 僅 3 site×2）。HT172 同樣 4 occurrence（2 宣告 + 2 寫）亦 0 讀者；註解描述的 baseline-restore 功能兩邊都從未實作。live range/rate 路徑（`GetRange/GetRate`/`SetRange/SetRate`，`MyMotor.cpp:311-312,355-356`）完全獨立、不參考 Origin*。**決策已由證據解決**：HT160 與 HT172 皆無 mid-operation 變更 range/rate 再 restore 的機制。

**修法步驟**：
1. `MyMotor.h` 刪 149-150（用 byte-safe editor，因標頭含 Big5）。
2. `MyMotor.cpp` ctor 刪 255-256（保留 `bErrorMove=false`(254)、`bIsServoMotor=false`(257)）。
3. `database.cpp` 刪 1581-1582（保留 live `SetRate/SetRange` 1571-1572 與 `SimulateSpeed` 1583）。
4. **勿動** `GetRange/GetRate/SetRange/SetRate`。
5. grep 確認 `OriginRange`/`OriginRate` 全樹 0 殘留。

**要動的檔**：`MyMotor.h`、`MyMotor.cpp`、`database.cpp`。

**build 驗證**：標頭 class layout 變更 → 刪 `MyMotor.obj`、`database.obj` 後 `-Clean`（不在 curated set 則 `-Full`）；encoding check；不得有 `OriginRange/OriginRate` undefined symbol（證明無隱藏讀者）。

**風險/工數**：low / S。

**仍需決策**：**無**。A vs B 已由證據解決（HT160/HT172 皆無 reader/restore 機制）。僅操作注意：標頭含 Big5 用 byte-safe editor，標頭變更刪兩 obj 再建。

---

## unhandled-state 類

### F2-runmode-Run_TrayFeed / F5-trayfeed-finish-stub / F6-aTrayArm-IsTrayFeedFinish-incomplete（合併 —— 同一 TrayFeed 子系統）

**位置**：`database.h:250`（`Run_TrayFeed=4`）；唯一 setter `csystem.cpp:1028-1032`（註解掉）；消費 drain loop `csystem.cpp:1060-1069`（live 但不可達）；gate `csystem.cpp:1142-1148`（`CheckAllTrayFeedFinish` 恆 false）；`CheckEmpty1TrayFeedFinish` `1137-1140`（恆 true，orphan）；`TTrayArmModule::IsTrayFeedFinish()` `aTrayArm.cpp:82-89`（恆 true，ctor 29 + InitialFlag 41 硬設，無 caller）。

**現況**：整條路徑「安全且休眠」。今日無 live defect —— 沒有任何 code 設 `RunMode=Run_TrayFeed`。但若進入 Run_TrayFeed，`CheckAllTrayFeedFinish()` 恆 false 會讓 revert loop 永不完成 → 機台永久卡死（永不回 Normal、永不停）。HT172 `CheckAllTrayFeedFinish`（`csystem.cpp:1176-1184`）AND 四旗標，前二屬 Magazine/MagArm 子系統，HT160 **沒有**（模組僅 aTrayArm/aEmpty/aColor/aLoader/aSortArm/aAuto1To6）。HT160 模組**皆未**暴露真實 TrayFeed-finish accessor（Empty 只有 `IsRearHasTray()`、Loader 有 `IsRearHasTray()`+`IsAllCleanOutFinish()`、Auto 有 `IsAllCleanOutFinish()`+`FindTrayRequestAuto()`）。其餘消費者（lamp `csystem.cpp:717-735` / status / note K_TRAY_FEED `note.cpp:321/347/402` / SECS SV 66000 `uHGemHT160.cpp:107` / TrayFeedOK）皆休眠死碼。HT172 排空靠 Magazine/MagArm（HT160 無），且有真 drain task driver `DoLoaderTrayToStorage`（HT172 `aTrayArm.cpp:286+`），HT160 無。

**修法步驟**：
1. **決策閘（主阻擋）**：HT160 機構（單 TrayArm 搬空盤、無 Magazine/MagArm）是否需要 CleanOut→TrayFeed 排空？
2. **Option A（do nothing，建議預設）**：不改 code（現況安全且刻意）。可選清掉 dead scaffolding（`database.h:250` enum、lamp、note keys）但低價值且破壞 HT172 parity，建議保留。
3. **Option B（移除陷阱，推薦的清理路徑）**：
   - B1 `csystem.cpp ~1021-1037` 刪註解掉的 TRAY_FEED 分支(1028-1033)，修剪註解，保留 K_SKIP-only 與無條件 `ChangeRunMode(Run_Normal); SoftStop=true;`。
   - B2 刪 live-but-unreachable `else if(RunMode==Run_TrayFeed){...}`(1060-1069)。
   - B3 刪 `CheckEmpty1TrayFeedFinish()`(1137-1140) 與 `CheckAllTrayFeedFinish(bool)`(1142-1148) 及其 `csystem.h:39/40` 宣告。
   - B4 `aTrayArm.cpp` 刪 `IsTrayFeedFinish()`(82-89) 與 `bTrayFeedFinish=true;`(29,41)；`aTrayArm.h` 刪欄位(49) 與宣告(73)。
   - B5 **保留** `Run_TrayFeed` enum(database.h:250) 與 label maps（K_TRAY_FEED 為通用 Note/lamp key，移除會破壞無關 note 渲染）。
4. **Option C（defer）**：不改 code，僅在 `csystem.cpp:1027` 加 TODO，避免後人「好心」解註解。
5. **Option (full port)（僅當功能確認需要，LARGE，blocked）**：先定義 HT160 EmptyTray-recovery 交握（`aTrayArm.cpp:84-88` 註明尚不存在）；為 TrayArm/Empty/Loader 加真實 finish 旗標與 Run_TrayFeed drain job（procedural switch(Task)，無 FSM）；重寫 `CheckAllTrayFeedFinish` AND 真旗標（丟 Mag/MagArm 項）；**最後**才解註解進入點並改 `K_SKIP|K_TRAY_FEED`，並確認 `K_TRAY_FEED` 在 HT160 cmydef/mymessbox 存在。

**要動的檔**：Option A 無；Option B/C：`csystem.cpp`、`csystem.h`、`aTrayArm.cpp`、`aTrayArm.h`；full port 另 `aLoader.cpp/.h`、`aEmpty.cpp/.h`。

**build 驗證**：Option A 無建。Option B 移除 `csystem.h` 簽名 + `aTrayArm.h` 欄位（結構性）→ 刪 `csystem.obj`/`aTrayArm.obj` 後 `-Clean`（標頭被廣泛 include 則 `-Full`）；確認 link clean（無 `CheckAllTrayFeedFinish`/`CheckEmpty1TrayFeedFinish`/`IsTrayFeedFinish` unresolved）；encoding check。

**風險/工數**：F2-runmode 視角 high / L；F5/F6 視角 low-medium / S-M（取決於 Option）。

**仍需決策**：HT160 是否需 TrayFeed 排空。否→Option A/B/C。是→full port，blocked 在未定義的 EmptyTray-recovery 交握 spec 與實體排空目標（無 Magazine 餵料）。**硬約束**：切勿只解註解 `csystem.cpp:1028-1033` —— 會卡死機台，比今日 K_SKIP-only 更糟。

---

### F8-csystem-bSortArmNeedHome-not-wired

**位置**：旗標 `cmydef.cpp:36`/`cmydef.h:39`；consumer driver `DoSortArmZHome()` `csystem.cpp:806`；dispatch Layer 3 `csystem.cpp:995-1002`；hold gate `csystem.cpp:181`；full-home reset `csystem.cpp:972`。`aSortArm.cpp` 無任何 writer/retry/引用。

**現況**：consumer 半已完整、正確；**只缺 producer**（設 `bSortArmNeedHome=true` 的 writer）。旗標永遠 false，Layer 3 + `DoSortArmZHome()` 死/休眠。HT172 producer 是 `MoveSortArmToAutoSafe()`（`aSortArm.cpp:2244`），移所有 MSortArmZ 到 safe 後讀 `Led[iHomeLed]`，用 `static iRetryCount>100` debounce 才設旗標。HT160 結構對應是 `SortArmZToSafePos()`（`aSortArm.cpp:474`，已移 4 個 MSuckZ_n 到 `SORT_ARM_SAFE_Z_POSITION`=10，且為每次 pick/place 子週期首步）。API parity 確認：`TTrayMotor` 有 `ScanMotorStatus()`、`Led[]`、`GetEnable()`，`iHomeLed=1`。`SORT_ARM_SUCKER_COUNT=4`。

**修法步驟**：
1. **決策閘**：確認 HT160 SortArm 是否採 HT172 週期性 suck-Z 失原點自動回復；否則改走刪死碼替代。
2. 在 `SortArmZToSafePos()`（`aSortArm.cpp:474`）內加偵測器（HT160 對應 HT172 `MoveSortArmToAutoSafe()`，勿新建該函式）。
3. 僅在 `bAllDone==true`（所有 enabled Z 到 safe 並穩定）時取樣：`static int iZHomeLostRetry=0;`，迴圈 `GetSuckZMotor(SlotIndex)` 跳過 NULL/`GetEnable()==false`，`ScanMotorStatus()` 後 `if(Motor->Led[iHomeLed]==false) bLostHome=true;`。
4. debounce/latch：`if(bLostHome){ if(iZHomeLostRetry>100){ iZHomeLostRetry=0; bSortArmNeedHome=true; } else iZHomeLostRetry++; } else iZHomeLostRetry=0;`。**勿**另呼叫 SortArm1ZHome（HT160 無此 uHome 方法；`DoSortArmZHome()` 即 re-home，由旗標驅動）。整塊用 `#ifndef SOFT_SIMULATE`...`#endif` 包住（sim 無真實 home LED），確保 selftest 不誤觸。
5. 若編譯報 `bSortArmNeedHome` 未宣告，在 `aSortArm.cpp` 頂加 `#include "cmydef.h"`。
6. **勿動** `csystem.cpp`（consumer 已完整）。
7. 風格：procedural、無 FSM、ASCII 註解、無 C++11、無 Sleep。
8. **替代（決策=否）**：改在 `csystem.cpp` 刪 Layer 3(995-1002)、`DoSortArmZHome()`(801-824)、簡化 hold gate(181)，移除誤導死碼。

**要動的檔**：`aSortArm.cpp`（主）；可選 `aSortArm.h`；替代路徑 `csystem.cpp`。

**build 驗證**：刪 `aSortArm.obj`（替代另 `csystem.obj`）後 `-Clean`（無結構變更）；encoding check；可選跑 `ht160s-home-selftest` 確認 `#ifndef SOFT_SIMULATE` guard 在 sim 仍 inert。

**風險/工數**：medium / S。

**仍需決策**：(1) HT160 是否採 HT172 週期性失原點自動回復；(2) 偵測訊號與門檻 —— 須確認 `Led[iHomeLed]` 是 MSuckZ_n 正確訊號（有些 HT160 軸用 `bHomeFlag`），及 100 取樣門檻是否符合 cadence。風險：錯訊號會 silent never-trigger 或誤觸把模組反覆拉進單 Z-home。實體驗證後才在 `#ifndef SOFT_SIMULATE` 外啟用。

---

### F3-cmydef-eSystemTime

**位置**：`HT160S_Program_BCB_V1.0.0.0/cmydef.h:66`（`enum eSystemTime { stStartTime..stMTBA, stTotalCnt }`）。

**現況**：稽核 `method` 為 null（未深查）。fixSummary 指出整個 enum 定義但全程式無引用。HT172 對應有 `long SystemTimeRecord[stTotalCnt]` 資料層 + 累積 + ini 持久化 + `sgTimeData/tsTimeData/btSaveTimeData` UI；HT160 只留 enum（編譯無害、執行期無效）。HT160S 似已有 `sgTimeData/tsTimeData/btSaveTimeData` 控制項但未綁定。

**修法步驟**：
1. **決策閘**：per-category 系統時間/MTBA 統計子系統是否要存在於 HT160S？
2. **Option 1（PORT）**：在 `cprod.h`/`database.h` 的 `LAST_GENERAL_SET` struct 加 `long SystemTimeRecord[stTotalCnt];`（可選名稱陣列），`ClearLastSet()`/`database.cpp` init 歸零，在主 `ProcessMotion`/run loop 逐狀態累積（移植 `main.cpp:1998-2008` MTBA 邏輯，HT172 FSM 改 HT160 procedural switch(Task)），存/讀 ini（原用 `lastdata.ini` `[TimeData]`），綁定既有 `sgTimeData/tsTimeData/btSaveTimeData` UI。多檔。
3. **Option 2（DROP）**：刪 `cmydef.h:66-74` 死 enum；移除前須確認 `tsTimeData` UI 分頁亦為刻意死或將 repurpose。

**要動的檔**：Option 1 多檔（`cprod.h`/`database.h`/`database.cpp`/`main.cpp` + UI 綁定）；Option 2 `cmydef.h`（+ 視 UI 分頁決定）。

**build 驗證**：Option 1 struct 變更 → `-Full`；Option 2 enum 在標頭 → `-Full` 較安全；encoding check。改 Big5 用 byte-safe editor。

**風險/工數**：M（Option 1）/ S（Option 2）；low risk。

**仍需決策**：per-category 系統時間/MTBA 統計是否在範圍。enum 本身無害（編譯過、無執行期效應），屬產品/範圍決策而非正確性 bug。

---

### F4-uHome-eHomeError

**位置**：`HT160S_Program_BCB_V1.0.0.0/uHome.h:46`（`enum eHomeLedColor { eHomeUnuse=0, eHomeOk=1, eHomeError=2, eHomeBusy=3 }`）；`ShowLed()` `uHome.cpp:92-113`（`eHomeError`→`clRed` `107-108`）；`ShowMotorHomePos()` `uHome.cpp:115-135`（enabled 馬達僅 `bHomeFlag?eHomeOk:eHomeBusy`，行 125-128）。

**現況**：`eHomeError` + `clRed` 分支可達但無 caller 傳入 —— 全樹只發 `eHomeUnuse/eHomeOk/eHomeBusy`，回原點失敗永遠停留黃（busy）、不亮紅。live 警報訊號已存在：`Led[iAlarmLed]`（`iAlarmLed=4`，`HTMotor.h:14`，由 `ScanAllMotorStatus` 每週期刷新，本檔 `247/328/422/656` 已用）。HT172 `ShowMotorHomePos` 純位置文字、不發 LED，亦從未接紅錯誤態 → 屬 HT160 latent enum/branch，非 172 回歸。

**修法步驟（互斥，只做一個）**：
1. **決策閘**：A=接紅 LED（新 UX）/ B=刪死 enum 值 + clRed 分支（移除不可達態異味）。
2. **Option A**（`uHome.cpp` `ShowMotorHomePos` enabled 分支 122-129）：三向取代 —— `if(bHomeFlag) ShowLed(i,eHomeOk); else if(HSys.MotPtr[i]->Led[iAlarmLed]) ShowLed(i,eHomeError); else ShowLed(i,eHomeBusy);`。加 ASCII 註解。無新符號/標頭/DFM 變更。
3. （A 子決策）limit-vs-alarm：觸發 CW/CCW limit 也會 force `Led[iAlarmLed]=true`（`MyMotor.cpp:646-648`），若紅燈只應代表真故障，須加 gate `&& Led[iCwLed]==false && Led[iCcwLed]==false`（仿 `uHome.cpp:422-423`）。
4. **Option B**：`uHome.h:46` 移除 `eHomeError`（重編號 `eHomeBusy=2`）或保留編號刪該值；`uHome.cpp:107-108` 刪 `else if(attr==eHomeError) ... clRed;` 分支。零行為變更。確認無 code 存 raw int。

**要動的檔**：`uHome.cpp`（Option A）；`uHome.h`（Option B only）。

**build 驗證**：刪 `uHome.obj`；A 為 body-only → `-Clean`；B 動標頭 enum → `-Full`；encoding check（檔含 Big5 註解，用 byte-safe editor）；可選 `ht160s-home-selftest`（顯示變更不影響 sim pass/fail）。

**風險/工數**：low / S。

**仍需決策**：每軸紅色 home-error LED 是否為期望 UX（整機故障已由全域 Note/alarm 呈現，HT172 本身亦未接紅態）。是→Option A；否→Option B。A 子決策：僅 limit 上的軸是否該顯紅。純 UX/產品決策，無硬體需求。**不做兩個**。

---

### F5-trayarm-TAS_PLACING

**位置**：`HT160S_Program_BCB_V1.0.0.0/aTrayArm.h:17`（`eTrayArmStatus` 含 `TAS_PLACING`）；`DoTrayArm()` `aTrayArm.cpp`（case 1000 設 `Status=TAS_CARRYING` 約 563；case 100 home-resume 約 541-542）。

**現況**：稽核 `method` 為 null。fixSummary 指出 `TAS_PLACING` 定義但從不產生；`Status` 唯寫、`GetStatus()` 從無 caller。屬 cosmetic/telemetry 正確性 —— 因 `GetStatus()` 無消費者、`Status` 不用於控制，改動不影響機台行為。

**修法步驟**：
1. **決策閘**：TrayArm status 是真實 telemetry/UI/SECS hook 或死樣板？
2. **Option A（additive，若未來有 status 顯示/SECS）**：在 place phase 設 `Status=TAS_PLACING` —— case 1000 `DoPick(1)` 成功且 `PlaceDest` 決定後、`DoPlace(0)` 前（取代 `Status=TAS_CARRYING` `563`，或 CARRYING 標 transit、PLACING 標實際 place phase）；case 100 home-resume 分支（541-542）`DoPlace(0)` 前亦鏡像設定。inert 直到接 `GetStatus()` consumer（今日無）。
3. **Option B（subtractive，若無 status reporting）**：刪 `TAS_PLACING`、唯寫 `Status` 欄位、從無呼叫的 `GetStatus()`（單實例 arm 不需內部 status，不像雙側 LoaderModule）。

**要動的檔**：`aTrayArm.cpp`、`aTrayArm.h`。

**build 驗證**：刪 `aTrayArm.obj`；Option B 動標頭 enum/欄位 → `-Full`；Option A body-only → `-Clean`；encoding check。

**風險/工數**：low / S。

**仍需決策**：TrayArm telemetry 是否上路線圖。A（補完 enum，inert）vs B（刪未用 status + GetStatus）。皆為乾淨單點。

---

### F7-motor-eMotorKind-nondefault

**位置**：`HT160S_Program_BCB_V1.0.0.0/MotorAndIO/HTMotor.h:35`（`eMotorKind` 非預設值 `eLinerMotor/eCylinderMotor/eVoiceCoilMotor/eStepServo/eYASKAWA`）。

**現況**：稽核 `method` 為 null。fixSummary：這些值可由 config 產生但從不被 branch；HT160 無驅動路徑處理。`database.cpp:1576` 存值但 inert。HT172 在 base class 有兩處 branch（`VoiceCoilMotorHome()` 與 move-completion alarm 的 `eStepServo` 豁免），HT160 未移植。

**修法步驟**：
1. **決策閘**：是否有 HT160S 機台在任一軸跑非標準馬達（linear/cylinder/voice-coil/step-servo/Yaskawa）？需看現場 `Mot_Table.csv`（機台端，不在 repo）與硬體 BOM。
2. **Option A（範圍防禦，建議若單一馬達種類）**：保留 enum + loader，但讓 loader 主動拒絕非 `eMotor` 種類 —— `database.cpp:1576` 若 `Data->MotorKind!=eMotor` 則 log Note/EventLog warning 並 force `SetMotorKind(eMotor)`，使缺失行為 fail-loud。單點低風險。
3. **Option B（移植行為，僅當須驅動 voice-coil/step-servo）**：移植 HT172 兩 branch 點到 `MyMotor.cpp` —— (i) 加 `VoiceCoilMotorHome()` 與 `eVoiceCoilMotor` dispatch 於 `Home()`(`MyMotor.cpp:510`) 頂端；(ii) move-completion alarm 加 `GetMotorKind()!=eStepServo` guard；必要時移植 `mySMCmotor.cpp` kind branch。多點、medium/high 風險（觸 home+alarm+move-completion 控制路徑），須 procedural 重寫。

**要動的檔**：Option A `database.cpp`；Option B `MyMotor.cpp`（+ 視需要 `mySMCmotor.cpp`/`HTMotor.h`）。

**build 驗證**：Option A → `-Clean`；Option B 觸控制路徑 → 全 home/alarm 回歸（`ht160s-home-selftest`）；encoding check。

**風險/工數**：未評（A 低 / B medium-high）。

**仍需決策**：是否有任一 HT160S 機台跑非標準馬達種類。否→Option A（loader 拒絕/警告）；是→Option B（移植 per-kind dispatch + step-servo 豁免）。需現場 `Mot_Table.csv` 與硬體 BOM。

---

### F8-motioncard-nonproduced

**位置**：`HT160S_Program_BCB_V1.0.0.0/MotorAndIO/HTMotor.h:24`（`eMotionCardType eMC8040A/ePCI885X/ePLCbase`）；`MotionCardType` 欄位寫於 `database.cpp:1549-1556`、`myMC88X1motor.cpp:72`、`myMN200motor.cpp:14`、`mySMCmotor.cpp:14`；init `HTMotor.cpp:23`；accessor `MyMotor.h:226-227`、`MyMotor.cpp:380-381`、`HTMotor.h:56/103/104`。

**現況**：欄位唯寫、無 consumer、全 MC88X1。三個 enum 值從不產生。真正可動項其實是「唯寫欄位 + 未移植的 HT172 SMC/MN200 servo-on gate」（HT172 `mymotor.cpp:1638,1657`），而非三個 vestigial enum 值（HT172 中亦 vestigial）。

**修法步驟**：
1. **決策閘**：HT160 是否會驅動 SMC 或 MN200 motion 卡（vs 目前 Mot_Table 全 MC88X1）？
2. **Option A（否，建議）**：把 `MotionCardType` 視為死 —— 刪唯寫欄位 + `Set/GetMotionCardType` accessor（`HTMotor.h:56/103/104`、`MyMotor.h:226/227`、`MyMotor.cpp:380/381`、`SetMotionCardType` 呼叫 `database.cpp:1549-1556`/`myMC88X1motor.cpp:72`/`myMN200motor.cpp:14`/`mySMCmotor.cpp:14`、init `HTMotor.cpp:23`），**或**保留並加 vestigial 註解。三個未產生 enum 值留著（無害 legacy，與 HT172 一致）。
3. **Option B（是）**：移植 HT172 consumer —— 在 HT160 `MyMotor` ServoOnOff/ServoOnResetPos 對應加 `GetMotionCardType()==eSMC||eMN200` gate，使欄位變 live。

**要動的檔**：`HTMotor.h`、`MyMotor.cpp`、`database.cpp`（A 另多檔 accessor 移除；B 同 consumer 檔）。

**build 驗證**：A → `-Full`（標頭欄位移除）；B → `-Clean`。

**風險/工數**：low / S。

**仍需決策**：是否用 SMC/MN200 卡。稽核 confirmedCurrentState 標「No」。否→欄位真死，移除或註解（三 enum 值留為無害 legacy）；是→真正缺口是未移植的 SMC/MN200 servo-on gate，應移植 consumer 使欄位 live。`eMC8040A/ePCI885X/ePLCbase` 三值並非問題核心（HT172 中亦 vestigial）。

---

### F4-safedoor-state-record（已合併至上方 stub 節 F4-csystem-RecordSafeDoorStates-empty）

兩筆稽核描述同一函式，已在 stub 節合併展開（現況：已於 `csystem.cpp:1165-1210` 實作完成，稽核快照過期）。此處僅交叉指向，避免重複。

---

## todo 類

### F1-aAuto1To6-IsAmrTaken-TODO（已合併至 gap-hardware 節 IsAmrTaken）

此筆與 `F4-aAuto1To6-IsAmrTaken` 為同一掛點（`aAuto1To6.cpp:977`），已在 gap-hardware 節合併展開。此處僅交叉指向。

---

### F10-MyBinDisp-P3-bMemo-echo-TODO

**位置**：`HT160S_Program_BCB_V1.0.0.0/MyBinDisp.cpp:387-404`（`LogBinDisplay`）；行 403 `// P3 TODO: when bMemo, also echo to the ComPort bin memo.`（`bMemo` 未用）；`AddBinDisplayLog`(406-409) 以 `false` 呼叫。宣告 `MyBinDisp.h:28/119`。

**現況**：`LogBinDisplay` 建 `asLine`、append `slBinDispLog`(cap 500)、依 `GeneralSetting.bBinDispLogVerbose` 或非例行 frame 寫 `g_BinDispCommLog.Log`，但 `bMemo` 無螢幕回顯。`MyBinDisp.cpp` **未** `#include "ComPort.h"`、無 `fComPort` 引用 → HT172 echo line 無法編譯直到加 include。`ComPort.h:39` 有 `memoPanelCom`（Pad/Panel 通道）但無 `memoBinCom`/`cbBinCheckLog`。`ComPort.cpp:376-391` `MemoAddString` 無條件 `g_PadCommLog.Log`(390) → 重用會污染 PadLog CSV。HT172 對應：`MyBinDisp.cpp:564-571` echo、`ComPort.h:113/115`、`ComPort.dfm:230-237/286-295`。

**修法步驟**：
1. **決策閘**：(1) HT160 是否要獨立螢幕 bin memo + checkbox，或 `slBinDispLog`+CSV 已足（→刪 TODO 即可）；(2) gate 用新 `cbBinCheckLog` 或既有 `GeneralSetting.bBinDispLogVerbose`；(3) bin Send/Recv 是否改 `LogBinDisplay(...,true)`。
2. **實作路徑**：
   - `ComPort.dfm`：在 `pnlSetting` 加 `TCheckBox cbBinCheckLog`（Caption 'Check Log'）+ `TMemo memoBinCom`（`pnlLog` 已有 alClient `memoPanelCom`，須調 `memoPanelCom` 為 alLeft 並 dock `memoBinCom` alClient，或新增 `pnlBinLog`）。先備份 dfm+h，byte-safe 編輯，勿開 designer。
   - `ComPort.h`：`memoPanelCom`(L39) 後加 `TCheckBox *cbBinCheckLog; TMemo *memoBinCom;`（名稱須與 DFM 完全相符；所有元件欄位置於所有 event handler 之前）。
   - `ComPort.cpp`：**勿**重用 `MemoAddString`（會污染 PadLog）；新增 memo-only `BinMemoAddString(AnsiString)`（加時戳、`memoBinCom->Lines->Add`、超 1000 行 `Delete(0)`，**不**寫 CSV），公有宣告於 `ComPort.h`。
   - `MyBinDisp.cpp`：加 `#include "ComPort.h"`；行 403 取代為 `if(bMemo && fComPort!=NULL && fComPort->cbBinCheckLog!=NULL && fComPort->cbBinCheckLog->Checked) fComPort->BinMemoAddString(asLine);`。
3. **替代（無新 checkbox）**：gate 用 `GeneralSetting.bBinDispLogVerbose`；或直接刪 TODO。
4. （可選）將部分 HT160 bin Send/Recv site 改 `LogBinDisplay(...,true)` 使 memo 有內容（與 owner 確認後）。

**要動的檔**：`MyBinDisp.cpp`、`ComPort.cpp`、`ComPort.h`、`ComPort.dfm`。

**build 驗證**：觸 `.dfm` 結構變更 + 兩 TU；curated `-Clean` set 不含 `ComPort.obj`/`MyBinDisp.obj` → 用 `-Full`（亦符 designer-save/struct 規則）；no-DFM 變體則刪兩 obj 跑 `-Clean`；encoding check（Big5 檔）；確認 bin memo 用新 `BinMemoAddString`（無 PadLog 污染）、表單仍載入。

**風險/工數**：medium / M。

**仍需決策**：(1) 是否要螢幕 bin memo+checkbox（或刪 TODO）；(2) gate 用新 checkbox 或既有 verbose 旗標；(3) bin site 是否改 `bMemo=true`。技術注意（非決策）：`MemoAddString` 會雙寫 PadLog → 用新 helper；`MyBinDisp.cpp` 缺 include；BCB designer 規則；`pnlLog` 已有 alClient memo 須自有 panel/dock。無硬體。

---

## 附註：重複稽核項對照

- **IsAmrTaken**：`F4-aAuto1To6-IsAmrTaken`、`F1-aAuto1To6-IsAmrTaken-TODO`、`F2-aAuto1To6-IsAmrTaken-TBD-comment`、`F11-aAuto1To6h-IsAmrTaken-decl-TBD` 為同一掛點（`aAuto1To6.cpp:977`）不同視角，已合併於 gap-hardware 節。
- **TrayFeed**：`F1-csystem-CheckAllTrayFeedFinish`、`F5-csystem-CheckAllTrayFeedFinish-stub`、`F2-runmode-Run_TrayFeed`、`F5-trayfeed-finish-stub`、`F6-aTrayArm-IsTrayFeedFinish-incomplete` 描述同一 Run_TrayFeed 子系統；stub 節與 unhandled-state 節各保留主述並交叉指向。
- **RecordSafeDoorStates**：`F4-csystem-RecordSafeDoorStates-empty`、`F4-safedoor-state-record` 皆已被 `csystem.cpp:1165-1210` 實作覆蓋（稽核快照過期）。
