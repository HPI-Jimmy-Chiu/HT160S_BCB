# HT160S_BCB「規劃卻沒做」稽核報表

## 一、摘要

- 產生方式: 唯讀偵測 (detection-only, 無改碼/無編譯). 對照基準: HT172 0420 (軟體結構) 與舊 HT160S SECSGem_ToBCB6 (硬體). B 類採偏嚴 (只報有規劃痕跡者).
- 數據: 候選 71 → 確認為真 43 → 可直接修 1 / 需決策 42.
- 一句話結論: 43 筆「規劃卻沒做」全屬「已知刻意延後 / 移植未完成」性質, 多數無立即執行風險; 真正具營運衝擊的是 AGV `IsAmrTaken` 在實機永遠 false (產線會 park)、`TMySMCMotor` 靜默失效 stub, 與 SECS S5 alarm 子系統會讓 host T3 timeout; 其餘以低風險的 orphan-ui / dead-config 清理為主, 全部修改前都需產品 scope 確認且須走 build gate 與 byte-safe Big5 編輯規範.

---

## 二、分類統計

| 類別 (category) | 需決策數 | 可直接修數 |
|:---|---:|---:|
| stub | 11 | 0 |
| dead-config | 10 | 0 |
| unhandled-state | 10 | 0 |
| orphan-ui | 5 | 1 |
| gap-hardware | 4 | 0 |
| todo | 2 | 0 |
| **合計** | **42** | **1** |

> 數字取自 JSON `byCategory`. `orphan-ui` 共 6 筆 (5 需決策 + 1 可直接修).

---

## 三、A 區 — 可直接修 (信心≥80%, 低風險, 單點)

> NOTE: 因為本次是 detection-only 模式, 連這筆也尚未套用, 需使用者核可後才動.

### A-1. edMCUMaxQueue / lblMCUMaxQueueCap 維護欄位已死 (無 load/save/read)

- **id**: `DC-MaintUI-edMCUMaxQueue`
- **位置**: `HT160S_Program_BCB_V1.0.0.0/maintenance.h:194` (另 `maintenance.dfm:1676` 與 `:1724`)
- **問題說明**: 這兩個控制項是已移除的「TCP MCU display bridge」殘留. 同頁的 `edMCUPort/edMCUIP/edMCUReconnect` 已被改用於 COM bin-display 設定, 但 `edMCUMaxQueue` 沒有 COM 對應, 變成孤兒. 程式碼註解 `maintenance.cpp:1088-1090` 明說該頁「由 TCP MCU 改作 COM bin display ... TCP Port/MaxQueue 欄位在此不用且在 DFM 隱藏」.
- **證據**: `TEdit *edMCUMaxQueue; TLabel *lblMCUMaxQueueCap;` 全樹搜尋只出現在 `maintenance.h` 宣告與 `maintenance.dfm` 物件; `LoadMCUDisplaySettings`(`maintenance.cpp:1091`) / `SaveMCUDisplaySettings`(`:1104`) 皆刻意不處理 MaxQueue; `GeneralSetting` struct (`GeneralSetting.h:66-93`) 無 MaxQueue 欄位; 過時的 `system/MCU.ini [Setup]MaxQueue` 也已無任何來源讀取.
- **建議修法 (摘自 fixSummary)**: 刪除兩個孤兒控制項並保持 `.h` 與 `.dfm` 同步. `maintenance.h` 移除第 193-194 行; `maintenance.dfm` 移除 `lblMCUMaxQueueCap`(1676-1684) 與 `edMCUMaxQueue`(1724-1732) 物件. 兩者皆已 `Visible=False`, 屬純死碼移除無行為改變. **關鍵 (依專案 BCB 記憶)**: `.h/.dfm` 刪除請以 byte-safe 手動編輯 (`scripts/ops/bcb6-bytesafe-edit.ps1`), 切勿在 IDE 開啟該表單刪除 (designer save 可能靜默移除其他元件); 之後做 full build + 編碼檢查. 過時的 `system/MCU.ini [Setup]MaxQueue` 行可順手清掉 (無功能影響). 若選擇保留隱藏控制項亦無功能需求 (它們不可見且惰性).
- **信心 / 風險**: 95% / low (single-point = true).

---

## 四、B 區 — 需決策 (報表式 + 條列)

### 4.0 快速掃描表 (全部 42 筆)

| # | id | 類別 | 標題 (簡) | file:line | 信心% | 風險 | 單點? |
|---:|:---|:---|:---|:---|---:|:---|:---|
| 1 | F1-csystem-CheckAllTrayFeedFinish | stub | CheckAllTrayFeedFinish 硬寫 return false | csystem.cpp:1097 | 72 | medium | no |
| 2 | F3-csystem-RecordSafeDoorStates | stub | RecordSafeDoorStates 空 body 每循環呼叫 | csystem.cpp:1120 | 55 | low | yes |
| 3 | F4-aAuto1To6-IsAmrTaken | gap-hardware | IsAmrTaken 實機硬寫 false (AGV 取車 IO 未接) | aAuto1To6.cpp:971 | 93 | medium | no |
| 4 | F6-uHGemEquipment-SetAlamData | stub | SECS SetAlamData 空 (alarm 登錄 no-op) | uHGemEquipment.cpp:656 | 80 | medium | no |
| 5 | F11-uHGemHT160-S7F6-recipe | stub | S7F6 recipe 上傳僅 log skeleton | uHGemHT160.cpp:890 | 90 | medium | no |
| 6 | F13-uHGemHT160-S5F6-alarmlist | stub | S5F6 alarm list 停用 (空清單) | uHGemHT160.cpp:871 | 88 | medium | no |
| 7 | F14-mySMCmotor-skeleton | stub | TMySMCMotor 52 行 stub (Stop/Jog/Alarm no-op) | mySMCmotor.cpp:33 | 88 | low | yes |
| 8 | F18-aColor-IsAcceptingIC | orphan-ui | IsAcceptingIC 硬寫 false 且無呼叫者 | aColor.cpp:627 | 70 | low | yes |
| 9 | F21-note-compat-empties | dead-config | note.cpp 7 個舊版相容 stub 空/常數 | note.cpp:816 | 72 | low | yes |
| 10 | F22-myio-IOInputLongByte | dead-config | IOInputLongByte 回 0 (繼承死 stub) | myio.cpp:446 | 85 | low | yes |
| 11 | F24-uHGemEquipment-IsEnableEvent | stub | IsEnableEvent 硬寫 true (事件 enable 濾失) | uHGemEquipment.cpp:353 | 82 | medium | no |
| 12 | F1-aAuto1To6-IsAmrTaken-TODO | todo | IsAmrTaken TODO (CEID274 實機不觸發) | aAuto1To6.cpp:977 | 96 | low | yes |
| 13 | F2-aAuto1To6-IsAmrTaken-TBD-comment | gap-hardware | 每 Auto 取車 IO 點設計註解標 TBD | aAuto1To6.cpp:969 | 93 | low | yes |
| 14 | F3-MyMotor-PackForAmrUpload-stub | stub | PackForAmrUpload 空 stub (AMR payload 未設計) | MyMotor.cpp:237 | 86 | medium | no |
| 15 | F4-csystem-RecordSafeDoorStates-empty | stub | RecordSafeDoorStates 空但每循環呼叫 | csystem.cpp:1120 | 85 | low | no |
| 16 | F5-csystem-CheckAllTrayFeedFinish-stub | stub | CheckAllTrayFeedFinish 恆 false; TRAY_FEED 無法完成 | csystem.cpp:1097 | 88 | medium | no |
| 17 | F6-aTrayArm-IsTrayFeedFinish-incomplete | unhandled-state | TrayFeed 撤離依賴未建的 Loader/EmptyTray 交握 | aTrayArm.cpp:85 | 82 | medium | no |
| 18 | F7-aLoader-TrayArmTakeout-not-wired | dead-config | LOADER_Y_OWNER_TRAYARM token 保留未接線 | aLoader.cpp:49 | 88 | low | yes |
| 19 | F8-csystem-bSortArmNeedHome-not-wired | unhandled-state | SortArm 單 Z-home 請求層之 writer 未接 | csystem.cpp:951 | 88 | medium | no |
| 20 | F10-MyBinDisp-P3-bMemo-echo-TODO | todo | bMemo 設定時未回顯 ComPort bin memo (P3 TODO) | MyBinDisp.cpp:403 | 88 | low | no |
| 21 | F11-aAuto1To6h-IsAmrTaken-decl-TBD | gap-hardware | IsAmrTaken 公開宣告文件標 sensor TBD | aAuto1To6.h:118 | 90 | medium | yes |
| 22 | ORPH-iosetview-spbTerminalProgram | orphan-ui | spbTerminalProgram 按鈕 OnClick 空 (關程式動作 stub) | iosetview.cpp:2161 | 93 | medium | no |
| 23 | ORPH-iosetview-ComboBox1Change | orphan-ui | ComboBox1 (loop-time) OnChange 空 no-op | iosetview.cpp:1952 | 90 | low | no |
| 24 | ORPH-iosetview-sbEnableIOChang | orphan-ui | sbEnableIOChang 按鈕 OnClick 空 (IO 編輯鎖失) | iosetview.cpp:1993 | 62 | low | yes |
| 25 | DC-tFunction-RejectCCDfail | dead-config | [Function] RejectCCDfail 讀寫但無消費 | cprod.h:77 | 88 | low | yes |
| 26 | DC-tFunction-UseHitCylinder | dead-config | [Function] UseHitCylinder 讀寫但無消費 | cprod.h:78 | 95 | low | yes |
| 27 | DC-tFunction-HitRetry | dead-config | [Function] HitRetry 讀寫但無消費 | cprod.h:79 | 95 | low | yes |
| 28 | DC-tFunction-UsePreAlignment | dead-config | [Function] UsePreAlignment 讀寫但無消費 | cprod.h:80 | 95 | low | yes |
| 29 | DC-GeneralSetting-iBinDispColor | dead-config | [BinDisplay] Color0..8 讀寫但從不讀取 | GeneralSetting.h:93 | 93 | low | yes |
| 30 | DC-MotTable-HomeOrder | dead-config | Mot_Table HomeOrder 解析進唯寫 TStringList | MyMotor.h:162 | 93 | low | no |
| 31 | DC-Motor-OriginRangeRate | dead-config | OriginRange/OriginRate 被寫入但從不讀取 | MyMotor.h:149 | 90 | low | no |
| 32 | F2-runmode-Run_TrayFeed | unhandled-state | Run_TrayFeed 無 live producer; 消費者不可達 | database.h:250 | 90 | high | no |
| 33 | F3-cmydef-eSystemTime | unhandled-state | 整個 eSystemTime enum 定義但從不引用 | cmydef.h:66 | 88 | low | no |
| 34 | F4-uHome-eHomeError | unhandled-state | eHomeError LED 有畫但從不產生 (home 失敗不亮紅) | uHome.h:46 | 80 | low | yes |
| 35 | F5-trayarm-TAS_PLACING | unhandled-state | TAS_PLACING 從不產生; Status 唯寫 | aTrayArm.h:17 | 80 | low | yes |
| 36 | F7-motor-eMotorKind-nondefault | unhandled-state | eMotorKind 非預設值可配置但從不分支 | HTMotor.h:35 | 70 | medium | no |
| 37 | F8-motioncard-nonproduced | unhandled-state | eMC8040A/ePCI885X/ePLCbase 從不產生; 欄位唯寫 | HTMotor.h:24 | 72 | low | yes |
| 38 | F1-systools-emptyform | orphan-ui | FormSysTools 空殼接到主畫面 Tools 鈕 | systools.cpp:11 | 88 | low | yes |
| 39 | F2-gem-SaveEventReportData-placeholder | stub | SaveEventReportData 僅 log placeholder | uHGemEquipment.cpp:302 | 63 | low | yes |
| 40 | F5-trayfeed-finish-stub | unhandled-state | TrayFeed run-mode 半成品 (stub + 註解掉分支) | csystem.cpp:1097 | 88 | high | no |
| 41 | F2-tapefeed-ringcatch-cassette-teach | gap-hardware | 捲帶/取環/卡匣機構僅剩死 teach 欄位 | uteach.h:22 | 92 | low | yes |
| 42 | F4-safedoor-state-record | unhandled-state | RecordSafeDoorStates 接入掃描迴圈但空 | csystem.cpp:1120 | 86 | low | yes |

---

### 4.1 stub (11 筆)

#### B-1. CheckAllTrayFeedFinish() 硬寫 return false, 阻擋 Run_TrayFeed 自動完成 (`F1-csystem-CheckAllTrayFeedFinish`)

- 位置: `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\csystem.cpp:1097`
- 規劃來源: in-code 註解明指其為 stub; HT172 `csystem.cpp:1176` 有真實 body 聚合 `AllMagArmTrayFeedFinish()&AllMagTrayFeedFinish()&CheckEmpty1TrayFeedFinish()&TrayArmPara->bTrayArmTrayFeedFinish`, 且於 RunMode state machine 被實際呼叫.
- 現況/證據: `CheckAllTrayFeedFinish()` 恆回 false, 於 `:1017` 被呼叫以 gate Run_TrayFeed 完成; 但唯一會設 `Run_TrayFeed` 的 `ChangeRunMode(Run_TrayFeed)` (`:986`) 已被註解掉, 故 `:1017` 分支當前為死碼/不可達 — 今日執行不會 hang.
- 驗證結論: 信心 72% / 風險 medium / 單點 no — 經 reachability 檢查確認分支不可達, 是刻意延後的部分移植 (非潛在 bug); HT172 所需的 magazine/TrayArm 子系統在 HT160 完全不存在.
- 修法摘要: 非一行可改. 需 (1) 決定 HT160 是否支援 CleanOut->TrayFeed drain; 若是, (2) 在現有 HT160 模組加上 per-module TrayFeed-finished 狀態 (仿其 IsAllCleanOutFinish/IsCleanOutFinish/IsOneCycleFinish), (3) 重寫 `csystem.cpp:1097` 聚合並尊重 reset 參數, (4) 解除 `csystem.cpp:983-987` 註解; 之後刪 `csystem.obj` 重編.
- ❓需決策: HT160S 是否需要 CleanOut->TrayFeed「排空剩餘 tray」功能, 或 SKIP-only (現行) 即是 HT160 預期 scope? **選項 A**: 保留為刻意停用 stub (零工, 模式不可達, 記為 won't-do). **選項 B**: 跨 Loader/SortArm/Auto 實作 per-module TrayFeed-finish 並重啟入口 (多檔, 需定義 HT160 無 Magazine/TrayArm 子系統下「tray feed finished」之硬體/流程語意).

#### B-2. RecordSafeDoorStates() 空 body 但每 DoSystem() 循環呼叫 (`F3-csystem-RecordSafeDoorStates`)

- 位置: `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\csystem.cpp:1120`
- 規劃來源: HT172 `csystem.cpp:1132` 有完整 body (per-door rising-edge 偵測, RecordProcess 'Safe Door N is Opened', PLC-safety 處理). HT160 每循環呼叫但 body 空.
- 現況/證據: `void RecordSafeDoorStates(){ }` 於 `:607` 無條件呼叫. 但 candidate 所稱「門開事件不再記錄」為 FALSE: HT160 在 running 時已於 `ScanSystemSenser`(`csystem.cpp:486-491`) 以 RecordProcess('MACHINE STOP by safety-door') 記錄; HT172 此函式是補充性的「停機/閒置時」per-door 上升緣紀錄. 直接照抄 HT172 不可編譯 (其依賴 `SnSafeDoor_01.Tag+i` 連續陣列、`bSafeDoorOpen[10]`、`Enable_PLCSafety_IO` 等 HT160 皆無).
- 驗證結論: 信心 55% / 風險 low / 單點 yes — 確認空 body 且每循環呼叫; 但實質影響僅「閒置時門開稽核 log」之完整性, 非安全/控制功能.
- 修法摘要: 若需閒置稽核 log, 以 HT160 自身 sensor model 填寫: 用 static 上升緣陣列 + SystemStart 清除/return, 列舉 HT160 實際的 6 個門 sensor (`SnSafeDoorFront/Right/Left`, `SnSafeSlideDoorRight/Left`, `SnSafeAuto6`); 每個若 Enable && IsOff() && !prevOpen[i] 則 RecordProcess(name+' is Opened'). 不可引用 HT160 不存在的 PLC-safety 全域. ASCII 註解、AnsiString、無 C++11; 改後刪 `csystem.obj` 重編.
- ❓需決策: HT160S 是否真的需要閒置狀態的 per-door 'Safe Door N is Opened' 稽核 log? **選項 A**: 對 HT160 的 6 個具名門 sensor 實作 (低風險、單點). **選項 B**: 視為 by-design no-op, 保留或移除 `:607` 死呼叫. 建議先向維護者確認此屬稽核完整性需求 (非安全/控制) 再投入.

#### B-3. THGem::SetAlamData() 空 (SECS alarm-data 登錄 setter 是 no-op) (`F6-uHGemEquipment-SetAlamData`)

- 位置: `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uHGemEquipment.cpp:656`
- 規劃來源: SECS alarm-list 資料層的一環 (`SetAlamData/ReadAlamData/WriteAlamData/ReadEventReportData` 在此全空). HT172 `AddAlarmList` 驅動這些以持久化/讀取 alarm grid; HT160 留 no-op, 使 S5F5/S5F6 alarm catalog 無 backing store.
- 現況/證據: `SetAlamData` 與其兄弟 (660-670) 皆空; HT160 唯一引用只有 `.h` 宣告與空定義, 無呼叫者. 會呼叫它的 override `HT160Gem::AddAlarmList`(`uHGemHT160.cpp:232-234`) 本身亦空. HT160 SECS 為明確的 framework skeleton; HT172 有完整 backing (`uHGemHT172.cpp:362` AddAlarmList → strGrdAlarm + AlarmData.def). HT160 無 strGrdAlarm/ReportAlarm/S5F1/mapAlarmCodeList.
- 驗證結論: 信心 80% / 風險 medium / 單點 no — 確認空且無 backing; 它是「整個未移植 alarm-catalog 子系統」的一片葉子, 非孤立 setter.
- 修法摘要: 單修 SetAlamData 無用 (無人呼叫). 需整層移植 (no FSM, procedural): (1) THGem 加 alarm store (重用 grid/TList 或新增 in-memory TList), SetAlamData 填一筆; (2) 實作 ReadAlamData/WriteAlamData 讀寫至 `SecsGemPath\SYSTEM\AlarmData.def`; (3) 實作 `HT160Gem::AddAlarmList` 走 HT160 實際 alarm-code 來源; (4) 實作 `S5F6_ListAlarmData` 序列化進 S5F6, 視需要加 S5F1. 建議當成獨立「SECS alarm catalog」工作項.
- ❓需決策: SECS alarm catalog (S5F5/S5F6 + alarm 回報) 是否在目前 HT160 SECS phase scope 內? **選項 A** (建議現在): 不修, 維持 skeleton. **選項 B**: 若 in scope, 需先定 HT160 權威 alarm-code 來源 + 儲存方式 (HT172 StringGrid+def 檔 或更輕的 TList). 在未明確排優先前, 不建議現在修.

#### B-4. S7F6_ProcessProgramData recipe-upload handler 停用 (僅 log skeleton) (`F11-uHGemHT160-S7F6-recipe`)

- 位置: `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uHGemHT160.cpp:890`
- 規劃來源: 由 live S7 router (`uHGemClass.cpp:104-105` case 3/case 5) 分派. body 僅 log 'disabled ... skeleton', 故 SECS recipe (process-program) 上傳/確認未實作.
- 現況/證據: `S7F6_ProcessProgramData()` 僅 StringOut '[SECS] S7F6 recipe upload is disabled...'. router 路由 S7 (不 S9F3), host 送 S7 會抵達這些 body. SKILL.md L35 明列「recipe 上下傳 (S7)」於「仍待補」. HT172 `uHGemHT172.cpp:1362-1606` 完整實作 S7F2/F4/F6. 另 S7F18/F20 被分派但 HT160Gem 未 override, fall through 到 base `SendUnsupported`.
- 驗證結論: 信心 90% / 風險 medium / 單點 no — 確認可達且為未完成移植; HT160 已有相同的 serialize helpers (`InitLocalHead/DataItemOut/SendLocalData`), 能力存在.
- 修法摘要: 將 HT172 S7 bodies 移植進 HT160Gem, 改用 HT160 RecipeManager 與 data root (非 HT172 硬寫 `D:\HT172\data\%s.ini`). 最小可行: 實作 S7F6 讀 PPID/filename → 解析 recipe 檔 (`RecipeManager.GetDataRootPath()`) → 缺 PPID 回 `S9F7_IllegalData` → 載入 .ini 序列化回覆; 實作 S7F2 回 HCACK、S7F4 存 PP body; 並補 S7F18/S7F20 override. HT160 無 memoPPBody, 用 AnsiString/TStringList. 不可改 HT172/Original. 走 build gate.
- ❓需決策: (1) host recipe model 需 equip→host (S7F5/F6)、host→equip (S7F1-F4) 或兩者? (2) HT172 用 `data\PPID.ini`, HT160 用 folder-based recipe — 確認 PPID↔recipe-folder 對應與磁碟格式. (3) S7 recipe transfer 是否在目前 HT160 SECS milestone, 或延到 SKILL.md L35 所述「真機現場驗證」後. **選項**: 視上述決定哪些 handler 須 functional vs 留 acknowledged stub.

#### B-5. S5F6_ListAlarmData handler 停用 (回報空 alarm list) (`F13-uHGemHT160-S5F6-alarmlist`)

- 位置: `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uHGemHT160.cpp:871`
- 規劃來源: 由 `uHGemClass.cpp:97` case 5 分派. 與空 `AddAlarmList`/`*AlamData` 家族成對; host S5F5 取 alarm-list 請求得到空/log-only 回應.
- 現況/證據: handler 僅 StringOut '[SECS] S5F6 alarm list is empty...'. handler 確實可達 (`HandleDataMessage` → `Dispatch(S,F)` → case 5). SKILL.md L35/L282/L134 標為 pending. 整個 S5 alarm 子系統皆空 (S5F6 log-only、AddAlarmList base+override 皆空、SetAlamData/ReadAlamData/WriteAlamData 皆空), 全樹無 `InitLocalHead(5,...)`. 一個 candidate 小誤: 它「不回 S5F6」而非「回空 list」, 故 W-bit 的 S5F5 會讓 host T3 timeout (比空更糟).
- 驗證結論: 信心 88% / 風險 medium / 單點 no — 三向反駁皆失敗; 為已知待補, 非永久省略.
- 修法摘要: 兩部分. (1) 最小: 讓 `S5F6_ListAlarmData()` 一律送出合法 S5F6 回覆以避免 T3 timeout — 解析 inbound ALID list (仿 `uHGemHT172.cpp:1158`), `InitLocalHead(5,6,0)` + ALCD/ALID/ALTX triplet LIST, 無 store 時回空 `L[0]`. (2) 完整: 實作 SetAlamData/ReadAlamData/WriteAlamData 與 AddAlarmList 建真實 store 再走訪. 不要直接套用 — 見決策.
- ❓需決策: HT160 是否投入 SEMI S5 alarm 子系統, 或維持 SV-based (alarm 僅以 SV 66010/66011 在 S6F11 內呈現)? HT160 無等同 HT172 `HSys.IterAlarmCodeList` 的 alarm-code store. **選項 A** (建議現在, 低風險, 消除 timeout): 讓 S5F6 一律回合法 (可空) 回覆, 真實 list 延後. **選項 B**: 建 HT160 alarm-code 來源 + 完整 AddAlarmList/*AlamData 家族 + S5F1. **選項 C**: 確認 SV-based 為永久設計並下修 SKILL.md 的 pending 註記. 建議 (A) 現做, (B) 視 host 整合需求.

#### B-6. TMySMCMotor driver 為 52 行 stub skeleton (Stop/DecStop/Jog/Alarm/Home no-op) (`F14-mySMCmotor-skeleton`)

- 位置: `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\MotorAndIO\mySMCmotor.cpp:33`
- 規劃來源: 由 `TMyMotor::InitialMotorObject`(`MyMotor.cpp:395`) 在 `Mot_Table CardModel=="SMC"` 時實例化. HT172 mySMCmotor.cpp 為完整 1054 行 driver (JogP@646/JogN@669/Stop@685/DecStop@690/GetAlarm@855/HomeFlag@876 皆真實 SMC-card 呼叫). HT160 只移植 52 行 skeleton. 目前 dormant 因 `system/Mot_Table.csv` 全 20 軸用 MC88X1; 但任何 SMC 軸都會靜默失效.
- 現況/證據: `Stop(){} DecStop(){} JogP(){return true;} JogN(){return true;} GetAlarm(){return false;} HomeFlag(){return true;}` — body「主動說謊」而非安全 no-op (JogP/N 回 true, GetAlarm 回 false=從不警報, HomeFlag 回 true=總已 home). `uMotorTest.cpp:909-13` 仍把 "SMC" 當合法可編卡型; 是 latent 危險.
- 驗證結論: 信心 88% / 風險 low / 單點 yes — 四向檢查; 唯一部分反駁是「現行配置 0 軸用 SMC」, 故 stub dormant.
- 修法摘要: **選項 A** (建議, 若 SMC 不在 roadmap): 讓 stub 改成 fail-safe — `GetAlarm()` 回 true (視為警報使 kernel 停)、`HomeFlag()` 回 false、`JogP/N` 回 false, 把靜默失效轉為 fail-safe. **選項 B** (若有 SMC 軸規劃): 移植 HT172 driver bodies, 但需 SMC card SDK (SmcW* API/Id 接線) 進 HT160 build, 目前無. 兩者皆單檔; 改後刪 `mySMCmotor.obj` 重編, 新註解 ASCII.
- ❓需決策: HT160S 是否有/規劃任何 SMC 運動卡軸? 若 NO (現行 100% MC88X1) → **選項 A** (fail-safe stub, 便宜、單點、移除靜默危險). 若 YES → **選項 B** (完整移植, 依賴新增 SMC SDK, 工作量大). 依全 MC88X1 配置, 預設建議 **選項 A**.

#### B-7. THGem::IsEnableEvent() 硬寫 return true (per-event enable 濾失) (`F24-uHGemEquipment-IsEnableEvent`)

- 位置: `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uHGemEquipment.cpp:353`
- 規劃來源: HT172 `IsEnableEvent`(`uHGemEquipment.cpp:7688`) 走訪 strGrdCEID, 僅當 CEID enable cell='1' 才回 true. HT160 硬寫 true 且目前無呼叫者, 故 SECS S2F37 collection-event enable/disable 濾鏡未實作.
- 現況/證據: 僅 `.h:244` 宣告 + `.cpp:353` 定義, 零呼叫者. 真實 S6F11 送出路徑 `EventReport()`(`:307`) 不呼叫它, HSMS SELECTED 後無條件送. lean `TGemCEIDItem` 無 enable 欄位; S2F37 無接收 handler; S2F38 ack 為 stub (`SendUnsupported("S2F38")`). 為 Phase-0 rewrite placeholder.
- 驗證結論: 信心 82% / 風險 medium / 單點 no — 它是更大未移植 S2F37/S2F38 enable 子系統的可見葉子.
- 修法摘要: 非只填 IsEnableEvent. 兩部分: (1) `TGemCEIDItem` 加 `bool Enabled`(預設 true), 實作 S2F37 接收 (解析 L[2]{CEED, L of CEIDs}) + S2F38 ack 設定 per-CEID Enabled (替換 SendUnsupported stub, 加 S2F37 dispatch case). (2) 實作 IsEnableEvent 走 `FindCEIDItem(iCeid)` 回 `Ce?Ce->Enabled:true`(CEED 空=全 enable), 在 `EventReport()`(`:307`) 於 SELECTED 檢查後加 `if(!IsEnableEvent(...)) return;`. 若只想清 stub, 維持 return true (report-all) 是安全的.
- ❓需決策: HT160 GEM 是否需 host 控制 collection-event enable/disable (S2F37/S2F38)? **選項 A**: 完整移植 (合規, 變動較大). **選項 B**: 延後, 保留 return true 並加 TODO. **選項 C**: 若永不支援, 刪除死 stub 以免暗示能力. 僅在 host 整合 spec 要求 S2F37 合規時選 A.

#### B-8. TMyCar::PackForAmrUpload 空 stub; AMR 上傳 payload 未設計 (`F3-MyMotor-PackForAmrUpload-stub`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/MotorAndIO/MyMotor.cpp:237`
- 規劃來源: body 空且有明確 'AMR upload payload not designed yet; stub' 註解; `MyMotor.h:115` 宣告 'payload TBD, stub for now'. 隱含規劃的 AMR 上傳 payload 序列化 (DeviceCount/per-tray IC count) 供 AGV S6F11 reports.
- 現況/證據: 全樹只在宣告與定義出現, 零呼叫. `uAgvStation.cpp:132-134` 明說「DeviceCount 在 payload 設計前維持 0」. `DeviceCount[]` 是真實 SECS SVID 欄位 (`uAgvStation.h:49`, 38202 band), ctor 設 0 (`:53`) 且別處不指派, host 永遠讀到 0. AGV 路徑 live (`bUseAMR` 被消費). 一個更正: HT172 無 PackForAmrUpload/DeviceCount, 故此為 HT160 原生前向 placeholder, 非 HT172 移植缺口.
- 驗證結論: 信心 86% / 風險 medium / 單點 no — 三向反駁皆失敗; 為 planned-but-unimplemented.
- 修法摘要: 待 payload schema 決定後實作. 最小可見價值 slice: 在 `PollAndCall`(`uAgvStation.cpp:~134`) 填 DeviceCount (與 `TrayCount[si]=Car->iTrayCount`/`CarrierID[si]=Car->CarID` 並列), 加 `int TMyCar::GetDeviceCount()` 走訪 `Tray[0..iTrayCount-1]` 累加每盤 IC count (需 TMyTray 有 IC-count accessor, 先確認). 然後刪 PackForAmrUpload 或讓它 build/cache 該 count. AnsiString、無 C++11、無 FSM. payload 欄位未定前不要實作.
- ❓需決策: (1) AMR/S6F11 上傳 payload 內容: 僅 DeviceCount, 或 per-tray (TrayID/Bin/IC count)、CarrierID、BinSetting? (2) IC-count 來源: TMyTray 是否已追蹤 per-tray IC count, 或須先加進 tray 資料模型? 若 tray 尚無 IC count, DeviceCount 無法計算, fix 會擴大為資料模型變更. 建議先與 AMR/host 整合 spec 確認 SVID 38202-band report 合約.

#### B-9. RecordSafeDoorStates 空 body 但每 system cycle 呼叫 (HT172 有完整 safe-door-open recorder) (`F4-csystem-RecordSafeDoorStates-empty`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/csystem.cpp:1120`
- 規劃來源: 每循環於 `csystem.cpp:607` 呼叫但 body 空. HT172 (`csystem.cpp:1132`) 完整實作 'JerryYang 20240807: Record when door opened', per-door `bSafeDoorOpen[]` 邊緣追蹤 + PLC-safety gating. 空 body = ported-signature-without-body 缺口.
- 現況/證據: 確認 `:607` 每循環呼叫且 `:1120-1122` 空; HT160 已於 `csystem.cpp:488` 以 `RecordProcess("MACHINE STOP by safety-door")` 記錄 running 時門開, 但僅在 SystemStart==true; HT172 此函式做「機台 idle/stopped 時」的互補紀錄, 非重複. footprint 真實具體.
- 驗證結論: 信心 85% / 風險 low / 單點 no — 三向確認; idle 狀態門開 log 確實缺. 信心不超 85% 因省略可能是刻意 scope drop (僅事件 log, 無安全/動作功能損失).
- 修法摘要: 以 HT160 sensor model 實作 (非照抄 HT172). 保留 HT172 控制流程 (static bClear + static `bSafeDoorOpen[N]`; SystemStart 時 ZeroMemory 一次並 return), 但列舉 HT160 6 個具名門 sensor (`SnSafeDoorFront/Right/Left`, `SnSafeSlideDoorRight/Left`, `SnSafeAuto6`, 經 `GetSensorOffIndex`); 各若 `Enable && IsOff() && !open` → `RecordProcess("Safe Door <name> is Opened")`. 移除 PLC-safety gate (HT160 無對應). 單檔; 改後刪 `csystem.obj` 重編.
- ❓需決策: (1) sensor 對映: 走訪全部 6 個門 sensor, 或只 front/right/left 對應 HT172 的 3 個? (2) PLC-safety gate: 確認直接省略 (建議), 不新引入. (3) 確認 idle 狀態門開 log 是否真的要進 HT160 production log, 或空 body 是刻意抑制此雜訊的決定 (此函式僅供 log 豐富化, 空著無功能/安全損失).

#### B-10. CheckAllTrayFeedFinish 恆回 false; Run_TrayFeed 與 CleanOut TRAY_FEED 分支無法自動完成 (`F5-csystem-CheckAllTrayFeedFinish-stub`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/csystem.cpp:1097`
- 規劃來源: body 硬寫 `return false;` 加註解「no per-module TrayFeed finish flag exists yet ... Wire real module finish state here when modules expose it」; 呼叫端 `csystem.cpp:979` 註明後果「SKIP is the only fully-working choice today」, TRAY_FEED dispatch (`:983-987`) 因此被註解掉.
- 現況/證據: live-called 於 `:1017` Run_TrayFeed 分支, 但該分支不可達 (唯一 writer 已註解, note.cpp PressTrayFeed 僅發 SECS event 不改 RunMode). HT172 `csystem.cpp:906-922` 顯示意圖 (CleanOut 後選 SKIP 或 TRAY_FEED 排空剩餘空盤). 一個小修正: per-module placeholder 確存在 (`TTrayArmModule::IsTrayFeedFinish()` `aTrayArm.cpp:82-88` 硬寫 true), 但與 CheckAllTrayFeedFinish 未接.
- 驗證結論: 信心 88% / 風險 medium / 單點 no — 確認 TRAY_FEED 端到端停用且自洽 (今日無誤動作); HT172 有完整實作, HT160 為知情延後.
- 修法摘要: 兩部分非一行. (1) `csystem.cpp:1097` 聚合真實 module finish state 取代 false (仿 HT172, reset 參數須清各 flag). (2) 解除 `csystem.cpp:983-987` 註解. **前置阻擋**: 底層 module API 尚不存在 — `IsTrayFeedFinish()` 硬寫 true、`CheckEmpty1TrayFeedFinish()` 硬寫 true、Loader/Empty/Auto 無真實 drain-finish. 各 module 須先實作真實空盤撤離 finish flag, 否則重啟入口會 false-complete 或永不完成. 建議維持現狀直到 module drain 邏輯建好.
- ❓需決策: TRAY_FEED (CleanOut 後空盤排空) 是刻意延後的 HT172 移植, 其前置 module API 在 HT160 未實作. **選項 A**: 維持現狀 (stub false + 註解 dispatch), 記 TRAY_FEED out-of-scope (SKIP-only), 最低風險. **選項 B**: 完整功能移植 (per-module drain + 真 finish flag + 重啟入口), 跨 aTrayArm/aLoader/aEmpty/aAuto + csystem, 需硬體/操作驗證. 除非客戶/operator 真需 CleanOut 後 drain, 選 A.

#### B-11. SECS/GEM SaveEventReportData() 為 logging placeholder (host-defined event reports 不持久化) (`F2-gem-SaveEventReportData-placeholder`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/SecsGem/uHGemEquipment.cpp:302`
- 規劃來源: HT160 宣告 (`uHGemEquipment.h:242`) 並輸出字面 'placeholder' log, 即顯式 not-yet-implemented marker. HT172 `uHGemEquipment.cpp:8604` 實際 `CopyStringGridAsTabFormat` 並 SaveToFile `EventReport_CEID.def`/`EventReport_ReportID.def`, 從 ~9 處被呼叫.
- 現況/證據: 為一行 placeholder, 但已接線 (2 live callers: `uHGemHT160.cpp:300`、`UsecegemMainFrom.cpp:91`). 部分反駁 candidate 框架: HT160 根本無法接收 host-defined reports (S2F33/35/37 皆 SendUnsupported), registry 為 form-less、code-defined 每次開機由 SetReportIDContent/SetCEIDContent 重建; load 端 `ReadEventReportData()`(`:668`) 亦空 — 故 save+restore 一同刻意停用, 今日 functionally inert.
- 驗證結論: 信心 63% / 風險 low / 單點 yes — 為 phased 部分移植 (Phase 0/skeleton); 功能影響僅在未來移植 host report-definition 後才浮現.
- 修法摘要: 單點: 實作 `SaveEventReportData()` 序列化 in-memory TList registries (TGemReportItem/TGemCEIDItem) 至 SECS data dir 文字檔 (仿 HT172 tab-format 但走 heap struct 而非 StringGrid); 配對給 `ReadEventReportData()`(`:668`) 對應 load. 若不需持久化, 較便宜的修法是刪 2 個 call site 與 stub 以移除誤導 marker. 不要直接套用.
- ❓需決策: SECS event-report 持久化是否在 HT160 scope? 今日 registry code-defined 每次開機重建, 且 host report-definition (S2F33/35/37) unsupported, 無東西可持久化. **選項 A**: 維持現狀 (by-design no-op, 與 form-less 靜態 registry 一致). **選項 B**: 移除 stub + 2 caller + 'placeholder' log 以消除誤導 marker. **選項 C**: 完整實作 save+load (含 ReadEventReportData), 僅在移植 S2F33/35/37 後有用. 除非近期需求, 建議 A 或 B.

---

### 4.2 gap-hardware (4 筆)

#### B-12. IsAmrTaken() 實機回硬寫 false (AGV car-taken IO 點未接線) (`F4-aAuto1To6-IsAmrTaken`)

- 位置: `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\aAuto1To6.cpp:971`
- 規劃來源: 由 `SecsGem/uAgvStation.cpp:207` 作 E87/AGV Finish (CEID274) 條件呼叫. header 註解 (`aAuto1To6.cpp:967`) 說實機需 per-Auto car-taken IO 點; 未接前 AGV finish 交握停在 Ready, 產線 park 在該 Auto. 對應 memory 'AGV E87 port plan'.
- 現況/證據: `if(IsSoftSimulate()) return true; return false; // TODO: ...`. 三軸反駁失敗: (a) caller live (`uAgvStation.cpp:207` → `uHGemHT160.cpp:191`, 唯一推進 AGV_READY→AGV_IDLE 並發 CEID274). (b) sim 回 true 且 TODO, HT172 完全無 AGV-station/CEID274, 是 HT160 原生功能待硬體. (c) `BeginPrep` 設 `bAmrLocked=true`, `GetTrayRequest` 鎖住時回 eTrayReqNone 停 TrayArm feed; lock 僅由 `ClearAmrCar` 釋放, 其唯一 post-prep caller 受 IsAmrTaken gate → 實機 false 使 lock 永不清, 首次滿車交接後該 Auto 永久 park (僅靠 home/init 解, 屬破壞性 reset). `database.h` per-Auto sensor 無任何代表「AGV 已取車」者.
- 驗證結論: 信心 93% / 風險 medium / 單點 no — 確認需真正新 IO 點; 無替代 real path 呼叫 ClearAmrCar.
- 修法摘要: 接一個 per-Auto「output car taken / car-present」sensor 並在 IsAmrTaken 讀取取代 false. 仿 `IsOutputCarFullForAmr`(`aAuto1To6.cpp:929`): 加 `GetCarTakenSensor(Index)` accessor 回新 `TMySensor*`, 在 `database.h` 宣告 6 個成員、加 IO_Table.csv 行, 實作 `if(IsSoftSimulate()) return true; ...return (s && s->Enable && car-removed-state);`. 極性須符所選 sensor; ClearAmrCar/CEID274/lock-release 已就緒不需改. IO 點未定前可加防護的手動清除路徑避免永久 park (屬 scope 增項, 非核心 fix).
- ❓需決策: 需硬體/IO spec 決定: (1) 哪個實體訊號代表 AGV 已取走每個 Auto 的滿車? **(a)** 每 Auto 專用新 'output car taken' 光電/極限 sensor (最乾淨, 需 AUTO1..6 電氣 + IO_Table.csv 位址); **(b)** 重用/反相既有訊號如 SnAutoX_OutputHasTray 清除 (便宜無新線, 但「無盤」≠「車離開」, 可能誤動; ⚠ 註 2026-06-25: SnAutoX_OutputHasTray 已全面移除, 此選項作廢, 改用其他既有訊號或選 a/c); **(c)** 以 AGV 自身 SECS 完成訊息作 Finish (軟交握, 無 sensor) 直接呼叫 ClearAmrCar. (2) sensor 極性/debounce. a/b → database.h + IO_Table.csv + aAuto1To6.cpp; c → SecsGem handler + uAgvStation.cpp, IsAmrTaken 不動. 另決定是否加臨時手動清除避免實機在 sensor 到位前永久 park.

#### B-13. Per-Auto 'car taken' IO 點在設計註解 + header 文件標 TBD (`F2-aAuto1To6-IsAmrTaken-TBD-comment`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/aAuto1To6.cpp:969`
- 規劃來源: 設計註解標 'TBD' 硬體 IO 點; public-API echo `aAuto1To6.h:118` 'IsAmrTaken(int Index); // ... (sim=true; real sensor TBD)'. 與 F1 同一缺口, 於設計/註解 + header 層捕獲. 對應 memory agv-e87-port-plan.
- 現況/證據: 同 B-12 — caller live (`uAgvStation.cpp:207`, 每 1s tick), 非 by-design-empty (sim 回 true, real TODO). footprint 具體: 實機 IsAmrTaken==false 使 Handshake[si] 永停 AGV_READY → CEID274 永不發、ClearAmrCar 永不跑、AMR lock 永不釋、產線 park. `database.h` grep CarTaken/Taken/Removed/AmrTaken 無匹配, IO 點確不存在.
- 驗證結論: 信心 93% / 風險 low / 單點 yes — 軟體掛勾單點 (`:977` real 分支); 三軸反駁皆失敗.
- 修法摘要: 硬體 IO 未定前不應改碼, 現行 sim=true/real=false 是正確 interim. 硬體定義後: (1) `database.h` 加 `TMySensor SnAutoX_CarTaken`(X=1..6) 於 SnAutoX_* 群, database.cpp 命名註冊 + IO_Table.csv/iosetview.dfm 位址; (2) 加 `GetCarTakenSensor(Index)` selector 仿 `GetInputFullTray`; (3) `:977` 改 `TMySensor *S=GetCarTakenSensor(Index); return (S && S->Enable && S->IsOff());`; (4) 加 IO_Table.csv 行 + iosetview.dfm alias. **此為硬體待定缺口, IO 點與極性未指定前不要套用 false→sensor 變更**, 否則 Finish 交握會誤動; 在此之前 `return false` 是正確安全預設.
- ❓需決策: 硬體/spec 決定: (1) 機台是否有 per-Auto「car taken/output car absent」sensor, 或以他法確認 Finish (AGV SECS msg/operator confirm)? (2) 若有 sensor: IO 位址與 active 極性 (「車已移除」讀 ON 或 OFF?). 答前 real 分支須維持 return false (安全: 停在 Ready 而非誤報 Finish 清掉仍在的車).

#### B-14. IsAmrTaken 公開宣告文件標 real car-taken sensor 為 TBD (`F11-aAuto1To6h-IsAmrTaken-decl-TBD`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/aAuto1To6.h:118`
- 規劃來源: header 文件標 'sim=true; real sensor TBD', 在 API 宣告層確認 F1/F2. 屬 AGV E87 硬體待定集 (memory agv-e87-port-plan「only real car-taken IO point + device counts pending hardware」).
- 現況/證據: 三軸反駁失敗 — caller live (`uAgvStation.cpp:207`, gate CEID274 `EventReport(0,274)` `:210`, SVID 38221, ClearAmrCar, 回 AGV_IDLE). impl `:971-978` 僅 sim 回 true, real false + TODO. 唯一 AGV_READY→AGV_IDLE transition 需 `IsAmrTaken()==true`; 實機硬寫 false → Finish 永不發、Auto 永遠 AMR-locked. `database.h:296-331` SnAutoX_* 只 Input/Output/TrayPos, 無 car-taken 點.
- 驗證結論: 信心 90% / 風險 medium / 單點 yes — IO 點確不存在.
- 修法摘要: 硬體 IO 未定前無適當改碼; 現行 placeholder 為正確 interim. sensor 指定後: (1) `database.h` 加 `SnAutoX_CarTaken` 於 SnAutoX_* 群, database.cpp `~926-961` 命名註冊 + IO_Table.csv/iosetview.dfm; (2) 加 `GetAutoCarTakenSensor(Index)` 仿 `:250-255` switch; (3) `:977` `return false;//TODO` 改 `TMySensor *s=GetAutoCarTakenSensor(Index); return (s && s->IsOn());`; (4) 更新 `aAuto1To6.h:118` 註解去除 TBD. caller 與 CEID274 wiring 不需改. AnsiString/無 C++11/ASCII 註解.
- ❓需決策: 硬體/spec: HT160S output-car AMR handoff 是否含 per-Auto 'car taken/car present' sensor, 各 Auto1..6 IO 位址為何? **選項 A**: 接實體 car-taken sensor → 依 fixSummary. **選項 B**: 以既有訊號代理 (原提議 SnAutoX_OutputHasTray 由有→無), 硬體成本低但語意較弱. ⚠ 註(2026-06-25): SnAutoX_OutputHasTray 已全面移除, 此代理來源不再存在; 改用其他既有訊號或選 A/C. **選項 C**: 以 host SECS msg/operator confirm 驅動 Finish. 現碼刻意 park 在 Ready, 屬硬體待定, 勿盲修.

#### B-15. 捲帶/取環/卡匣裝載機構僅以死 teach-position 欄位殘存 (`F2-tapefeed-ringcatch-cassette-teach`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/uteach.h:22`
- 規劃來源: TEACH struct 成員 (LoadCassette*/LoadArm*/TapeCutter*/TapeIn*/TapeShuttle*/RingCatchArm*/TapeIn_WaitTapeVacuumDistance 等) 為舊捲帶/取環/卡匣機構的規劃殘留. 舊源有專屬模組 (aTapeIn.h、aTapeShuttle/aRingCatchArm/aRingLoadArm/aCassette 與對應 teach UI). HT160S_BCB 中模組已移除, 欄位從未接入 teach grid, 也無任何 .cpp 讀取.
- 現況/證據: 全樹 grep 19 個 tape/ring/cassette 成員只出現在 `uteach.h` 宣告, 零讀寫; `InitialTeachParameter()`(`224-326`) 的 AddTeachItem 清單接入每個 TEACH 欄位「除了」22-43 區塊; 持久化 (OpenWorkFile/SaveWorkFile) 只走 TechPara[] (由 AddTeachItem 填), 故這些欄位連 tech.ini 都不寫不讀, 純零初始化死儲存. 原樹有 aTapeIn/aCassette/aRingCatchArm/aRingLoadArm/aTapeShuttle, HT160S_BCB glob 全無. iosetview.dfm 留有的 tape/ring IO-view tab 為另一筆惰性殘留, 非這些欄位的消費者.
- 驗證結論: 信心 92% / 風險 low / 單點 yes — 每項檢查皆確認; HT160 刻意無此機構, 屬死碼 footprint 而非待定功能; 無 memcpy/sizeof 二進位耦合.
- 修法摘要: 刪除 `uteach.h:22-43` 的 19 個死 TEACH 成員 (LoadCassetteFirstRingToLoadArmZPosition ... TapeIn_WaitTapeVacuumDistance). 從未 AddTeachItem、從未持久化、從未讀取. 單一 header (一個 struct) 單點編輯, 無需改 .cpp, 無需 tech.ini 遷移. 因 uteach.h 為 legacy Big5, 用 `scripts/ops/bcb6-bytesafe-edit.ps1` (Edit 工具會毀 Big5), 之後刪 `uteach.obj` 重編. 若全面清理在 scope, 可順帶移除惰性 iosetview tape/ring IO-view tab (屬另一筆, 較重 DFM/.h/.cpp 變更). 純死碼移除非行為缺口; 若只想記錄, 留欄位加 ASCII 'DEAD' 註解為零風險替代.
- ❓需決策: 此為真正機台能力 GAP 待重做, 或確認永久移除的死碼待清? 證據強烈顯示 HT160 刻意無此機構. **選項 1**: 確認已移除 → 刪 19 個成員 (並可後續處理惰性 iosetview tab). **選項 2**: 若未來 HT160 變體規劃此機構 → 留欄位加 ASCII TODO 視為延後. 除非 product owner 確認在 roadmap, 建議選 1.

---

### 4.3 orphan-ui (5 筆)

#### B-16. TColorModule::IsAcceptingIC() 硬寫 return false 且從不被呼叫 (`F18-aColor-IsAcceptingIC`)

- 位置: `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\aColor.cpp:627`
- 規劃來源: `aColor.h:66` 宣告, 定義回硬寫 false; HT160 無呼叫者、HT172 無對應. 似 HT160 引入而未接線的 accessor stub.
- 現況/證據: 全 writable 樹 grep 只見宣告 + 定義, 零呼叫. HT172/HT160-Original 皆無此函式 → HT160-only. candidate 框架部分有誤 (拿 IsInputHasTray/IsOutputHasTray 當已接, 但那兩個也零呼叫); 真正接線的是 IsTrayReady/RequestSupplyTray/NotifyTrayPicked (`aTrayArm.cpp:189/191/312`). IsAcceptingIC 屬同一 dormant IC-accounting cluster (NotifyICPlaced/GetICCount/Set+GetSupplyThreshold), 皆無呼叫者. 硬寫 false 因無人讀故無害.
- 驗證結論: 信心 70% / 風險 low / 單點 yes — 真孤兒 stub, 但是未接線功能群中數個之一, 非唯一壞 accessor.
- 修法摘要: **選項 A** (建議, 最低風險): 刪死 stub — 移除 `aColor.h:66` 宣告 + `aColor.cpp:627-630` 定義 (無呼叫無行為); 重編 ColorModule. **選項 B** (若 IC-accept 功能要復活): 實作 body 反映真實狀態 (如 `IsInstalled() && IsTraySupplyMode() && bTrayPicked==false`, 由 iSupplyThreshold vs iICCount 容量 gate), 再接 aTrayArm/sortarm 的呼叫者. 不要在未確認 IC-accounting cluster 是否出貨前選 B. 任一皆刪 .obj 重編 + 編碼檢查.
- ❓需決策: Color IC-accounting cluster (IsAcceptingIC + NotifyICPlaced/GetICCount/SetSupplyThreshold/GetSupplyThreshold) 是延後待做還是廢棄 scaffolding? 若 planned → 實作 IsAcceptingIC 真容量邏輯並接 placement caller (B). 若廢棄 → 刪整個未接線 cluster (A scope 擴及整 cluster). 只修 IsAcceptingIC 技術上可行但因兄弟同孤兒而美觀不全.

#### B-17. spbTerminalProgram 'Terminal Program' 按鈕 OnClick 空 (HT172 關程式動作之 stub) (`ORPH-iosetview-spbTerminalProgram`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/iosetview.cpp:2161`
- 規劃來源: DFM event binding `spbTerminalProgram.OnClick=spbTerminalProgramClick`(`iosetview.dfm:777`). HT172 (`iosetview.cpp:1864`) 完整實作 → MessageBox confirm → `fMaintenance->SaveData()` → `FormSysTools->TerminalProgram()` (停 thread 關程式). HT160S_BCB 缺 `FormSysTools->TerminalProgram()`, 移植退化為 no-op stub 且按鈕隱藏 (Visible=False).
- 現況/證據: body 僅 `(void)Sender;`. 三軸反駁失敗: (a) 無 C++ 呼叫, 僅 DFM binding 引用, 且無處設 Visible=true 故永久隱藏. (b) 原 HT160S baseline (`iosetview.cpp:73` Visible=true; `:1475-1477`→TerminalProgram; `systools.cpp:1052` MyThread->Suspend()+Application->Terminate()) 曾可用, 現空 stub 是 refactor regression. (c) 無 UI-reachable 替代關閉路徑 (唯一 `Application->Terminate()` 在 `csystem.cpp:137` 的 `--selftest-home` `#ifdef` 內). HT160S_BCB TFormSysTools 為空 stub (systools.h 只有 ctor).
- 驗證結論: 信心 93% / 風險 medium / 單點 no — planned/regressed but unimplemented, 非誤報.
- 修法摘要: 復原原 baseline 關程式動作. 兩耦合編輯: (1) 給 TFormSysTools 加 `TerminalProgram()` 方法 (systools.h/.cpp), 暫停 worker thread 並 `Application->Terminate()` — 但先 grep 確認 HT160S_BCB 實際 thread 物件名 (勿盲抄 MyThread/MyPad232Thread). (2) 實作 `spbTerminalProgramClick` 以 HT160 慣例確認 (`ShowMyMessageBox_YES_NO`, 非 VCL MessageBox), YES 則 save + `FormSysTools->TerminalProgram()`. 要真正露出還需在適當 gate 點設 Visible=true (或團隊決定刻意退役則維持 False). 改後刪 iosetview.obj + systools.obj 重編 + 編碼檢查.
- ❓需決策: in-app 'Terminal Program' (關程式) 動作在 HT160S 是否該存在? **選項 A** (刻意退役): 刪死按鈕 + 空 handler + DFM OnClick binding. **選項 B** (依原 baseline 復原): 實作 `TFormSysTools::TerminalProgram()` + handler + 決定可見性 gating (engineer/maintenance 或常顯). 並須考量 HT160 safety-door/thread 模型下從此畫面關程式是否妥當. 按鈕現隱藏無害, 建議向 owner 確認 A/B.

#### B-18. ComboBox1 (IOTool loop-time 下拉) OnChange handler 空 no-op (`ORPH-iosetview-ComboBox1Change`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/iosetview.cpp:1952`
- 規劃來源: DFM binding `ComboBox1.OnChange=ComboBox1Change`(`iosetview.dfm:9605`); Items 為 loop-time 值 ('0.1'/'0.3'/...). HT172 (`iosetview.cpp:2044`) 'ComboBox1Change // set loop time' 將下拉文字映射成 IOTool refresh 間隔. HT160S_BCB 下拉選擇因此什麼都不改.
- 現況/證據: body 無邏輯. (a) ComboBox1 僅在空 handler、宣告、DFM binding 出現, 無人讀 Text/ItemIndex; HT172 寫的 `TimeTick` 在 HT160 全樹 0 hits, `TimeCT`/LoopOutput 亦無. (b) 是空 stub 非刻意設計 — 2026-06-13 refactor 改用 live alias-bound 視圖並丟掉 HT172 'Loop Output' 閃爍測試; HT160 Timer1Timer 只 RefreshCurrentView/RefreshMN200. 32 個 DFM checkbox 亦孤兒. (c) 證據小誤: 它在 HT172 設的是 Loop-Output 閃爍週期, 非一般 view refresh; 但「下拉無效」結論成立.
- 驗證結論: 信心 90% / 風險 low / 單點 no.
- 修法摘要: **方向 A** (移除死 UI, 建議): 刪 ComboBox1 + 'Loop Output Interval' label (lb_IOToolOB1) + 孤兒 loop-output checkbox + Enable-IO-Change 鈕 (DFM), 移除空 ComboBox1Change/sbEnableIOChangClick handler 與宣告. **方向 B** (復原功能): 移植 HT172 loop-output 閃爍測試 (加 TimeTick + TimeCT, ComboBox1Change 映射 + checkbox→IOByteOut toggle), 多檔多元件非單點. 任一改後刪 iosetview.obj 重編並確認 .dfm/.h __published 同步.
- ❓需決策: HT160S 應 **(A)** 丟掉 2026-06-13 IO-view refactor 後遺留的 'Loop Output Interval' 下拉 + 孤兒 checkbox, 或 **(B)** 重移植 HT172 loop-output 閃爍測試 (operator 手動以選定週期脈衝/閃爍選定 output 作硬體診斷)? refactor 似刻意以 live view 取代此功能, 投入 B 前先確認是否仍需閃爍測試.

#### B-19. sbEnableIOChang 'Enable IO Change' SpeedButton OnClick 空 no-op (失去 IO-card edit-lock 行為) (`ORPH-iosetview-sbEnableIOChang`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/iosetview.cpp:1993`
- 規劃來源: DFM binding `sbEnableIOChang.OnClick=sbEnableIOChangClick`(`iosetview.dfm:9563`). HT160-Original (`iosetview.cpp:1731`) 實作 IO-card edit lock: Down 時禁 ed_OutPort/ed_InPort 編輯、停 macro、呼叫 `myIO[i].SetPortInformation`; Up 時重啟. HT160S_BCB 此 toggle 仍可見可點但無作用.
- 現況/證據: handler 空 `{ (void)Sender; }`; DFM TSpeedButton GroupIndex=1 在 ts_IOTool 'TOOL' tab 可見; 僅 DFM binding 引用, 從不程式呼叫. **「失去行為」框架被反駁**: HT160-Original handler 驅動的 ed_OutPort_*/ed_InPort_*/ed_*Card_*/sbStopMacro/myIO[] 在 HT160S_BCB 全不存在; 'HT172 live IO-view' refactor 改以 CSV/StringGrid 模型 (strngrdIoTable/LoadIoTable/SaveIoTableFromGrid), IO 配置現經 'Save' 鈕 → SaveIoTableFromGrid() → HSys.LoadIoData() (已完整實作). HT172 根本無 sbEnableIOChang. 故無機台控制行為遺失, 純惰性死控制.
- 驗證結論: 信心 62% / 風險 low / 單點 yes — 真實但 cosmetic 孤兒 UI 缺陷, 非功能遺失.
- 修法摘要: **不要**移植過時 HT160-Original handler (其依賴皆不存在, IO-port commit 已由 Save 處理). 視為死 UI 移除. 最小: DFM 刪 `sbEnableIOChang` 區塊 (9549-9564) 與 OnClick 行, 移除 `iosetview.h:88` __published 宣告與 `iosetview.cpp:1993-1996` 空 body. 若 designer 刪除有風險 (save 可能 strip 元件), 保守替代是保留控制但 DFM 設 Visible=False 並留 no-op. 可併入整個惰性 pn_IOTool3 panel (ComboBox1/TimeTick) 清理. 任一改後刪 iosetview.obj 重編 + 編碼檢查.
- ❓需決策: candidate 暗示復原 IO-card edit-lock 是 **錯誤** fix (依賴已被 refactor 移除, commit 路徑已存在). 選: **(A)** 移除死控制 (DFM 區塊 + .h 宣告 + .cpp body), 最乾淨, 但大型 form 的 designer 編輯有 strip 風險, 建議 byte-safe DFM 編輯 + re-inject; **(B)** 保留但 Visible=False 隱藏; **(C)** WontFix 視為無害惰性 UI. 並決定是否一併清理整個惰性 pn_IOTool3 panel (ComboBox1/TimeTick).

#### B-20. FormSysTools (System Tools 頁) 為空殼接到主畫面 'Tools' 鈕 (`F1-systools-emptyform`)

> **✅ 已結案 2026-08-04 (commit `1f17067`)**：採「移除按鈕、保留 unit」。sbTool 按鈕 + handler + smoke probe 全刪，
> 其右側按鈕左移一個 pitch(139)。**systools.cpp/.h/.dfm 保留未刪** —— 本節與 fixmethods 的 Option 2「連 unit 一起刪」
> 已過期：systools 自 20260624 起是狀態列時鐘的宿主（`AddMyTimeStringShow`/`RefreshMyTimeString`，
> `main.cpp` 註冊、`database.cpp` Timer1 每秒推動），刪掉會打掉時鐘。

- 位置: `HT160S_Program_BCB_V1.0.0.0/systools.cpp:11`
- 規劃來源: form 完整註冊接線非缺席: `ht160s.cpp:17` USEFORM、`:136` CreateForm; `main.h:106` sbTool + `:371` sbToolClick; `main.cpp:517` `ShowTopForm(FormSysTools, sbTool)`; `:1264-1267` lazy create+SmokeShow. 故真實 toolbar 按鈕開一個空白視窗.
- 現況/證據: 整個 systools.cpp 15 行 (僅空 ctor `:11`); systools.dfm 為空 640x480 'Tools' form 無子元件. HT172 systools.cpp 3411 行 (含 TAlarmManager/JAM alarm table/LevelSetup/NoNeedHomeCheckList). 三軸反駁失敗: (a) caller live — `sbToolClick` 呼叫 `ShowTopForm` (無條件 ShowModal); sbTool 是 main.dfm:766 真實可見按鈕. (b) 無 Visible/Enabled guard, 永遠可點. (c) 部分 by-design — 註解稱其為空 stub, HT172 內部 alarm-code-generation 已遷至 SYSTEM_MODULAR (database.cpp:748/755), 但 toolbar 按鈕未重用/隱藏/移除.
- 驗證結論: 信心 88% / 風險 low / 單點 yes — footprint 皆驗證精確.
- 修法摘要: 消除死路按鈕而非重建 HT172 3411 行頁 (其內部多已遷至 database.cpp). 優先序: (1) 隱藏孤兒: main.dfm 設 sbTool Visible=False 或 FormCreate/FormShow 設 `sbTool->Visible=false`. (2) 整個移除按鈕+handler: 刪 main.dfm sbTool、`main.h:106`/`:371`、`main.cpp:515-518` body; 可選刪 USEFORM/CreateForm/BPR/SmokeShow 與 systools.cpp/.h/.dfm. 走 build gate (wiring/BPR 變更 → full build). 不要整批移植 HT172 FormSysTools.
- ❓需決策: 'Tools' 頁是否會回到 HT160, 或 FormSysTools 為永久 stub? 若永久 stub, 偏好完全移除按鈕+form+註冊 (選項 2). 若可能日後重填 (如未移植的 LevelSetup/NoNeedHomeCheckList), 偏好只隱藏按鈕 (選項 1) 留空殼為 placeholder. **預設建議: 選項 1 (隱藏按鈕)** — 最小、可逆、移除「operator 看到空白 modal」症狀.

---

### 4.4 dead-config (10 筆)

#### B-21. note.cpp 舊源相容 helper 空/未呼叫 (LevelRecordProcess/SetShowAlarmLocation/SetShowSuckerLocation/ShowImageTrayFuntion/CheckCodeIsExist/CheckAlarmIsShow/GetRefrenceCode) (`F21-note-compat-empties`)

- 位置: `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\note.cpp:816`
- 規劃來源: 皆宣告但 HT160 無呼叫. 數個 (LevelProcessErrMessage/LevelRecordProcess) 在 HT172 亦空, 屬繼承相容 shim. `GetRefrenceCode` 值得注意: HT172 查 `HSys.mapNameToAlarm[S]`, HT160 回常數 'No Code', 但無呼叫故無 live regression. 保留為舊源 API 相容 stub.
- 現況/證據: 7 個 stub 皆空/常數 (CheckCodeIsExist 561 return false; LevelRecordProcess 816 {}; CheckAlarmIsShow 831 return true; SetShowAlarmLocation 836 {}; SetShowSuckerLocation 840 {}; ShowImageTrayFuntion 844 {}; GetRefrenceCode 848 return "No Code"). 全樹 grep 每名只在定義 (note.cpp) 與宣告 (note.h), 零呼叫/無 DFM/無 string dispatch. HT172 中 GetRefrenceCode/LevelRecordProcess 確被呼叫, 但 HT160 RecordProcess 已改寫為 g_EventLog.Log 並刻意丟掉查詢, maintenance IO-monitor 路徑亦註解/移除, 故 HT160 呼叫路徑被改寫消除. 真死.
- 驗證結論: 信心 72% / 風險 low / 單點 yes — GetRefrenceCode 常數 'No Code' 偏離 HT172, 但無呼叫故無 live regression.
- 修法摘要: 純死碼清理 (無行為改變). 刪 note.cpp 7 個 stub (CheckCodeIsExist ~560-563、LevelRecordProcess ~816-818、CheckAlarmIsShow ~831-834、SetShowAlarmLocation ~836-838、SetShowSuckerLocation ~840-842、ShowImageTrayFuntion ~844-846、GetRefrenceCode ~848-851) 與 note.h 對應宣告 (CheckCodeIsExist 137; 自由函式 164/167-171). note.cpp/h 為 legacy CP950, 用 `scripts/ops/bcb6-bytesafe-edit.ps1` (非 Edit 工具) 保 byte-safe. 移除後刪 note.obj 重編. 若偏好保留相容 shim, 維持現狀並對 GetRefrenceCode 加 ASCII 註解標明其為未用 stub 且偏離 HT172.
- ❓需決策: 移除這些死相容 stub (清理) vs 保留為刻意舊源 API 相容 shim. 零呼叫零影響, 移除安全但低價值; 若團隊要與 legacy/HT172 note.cpp API 表面保持 parity 則保留合理. 建議僅在 note.cpp 廣域死碼掃描在 scope 時移除, 否則不動. 另: GetRefrenceCode 常數 'No Code' 是相對 HT172 的潛在偏離, 未來若重啟 alarm-code 查詢須還原為 mapNameToAlarm 查詢而非沿用.

#### B-22. TMyIo::IOInputLongByte() 回 0 (繼承死 stub, 未呼叫) (`F22-myio-IOInputLongByte`)

- 位置: `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\myio.cpp:446`
- 規劃來源: `myio.h:36` 宣告, HT160 無呼叫. HT172 `IOInputLongByte`(`MotorAndIO/myio.cpp:111`) 亦回 0 且真實 body 註解掉 — 繼承的未完成 long-word IO 讀.
- 現況/證據: `int TMyIo::IOInputLongByte(int port){ return 0; }`. 三樹 grep: HT160S 只有定義+宣告零呼叫; HT172 同為 return 0 stub (真 body 註解); HT160S-Original `myio_ISA.cpp:233` 有可用硬體讀 body 但也無呼叫. 真繼承未完成 long-word 讀, 但無害 (未呼叫回 0).
- 驗證結論: 信心 85% / 風險 low / 單點 yes.
- 修法摘要: 刪死 stub: 移除 `myio.cpp:446-449` 4 行 body 與 `myio.h:36` 宣告. 無呼叫故安全. 重編 myio.obj 確認. 或若未來真規劃 long-word/32-bit MN200 input 讀, 保留但須實作 (HT160 用 MN200, 真 body 走 MN200 long-read 非 legacy ISA inportl). 因 HT160 無消費者, 建議移除.
- ❓需決策: 刪死 stub (建議, HT160 無 long-word IO 消費者且 MN200-based 非 ISA), 或保留為未來 32-bit MN200 input 讀的 placeholder. 若保留應標明 TODO 而非靜默 return 0. 函式未呼叫故無強制功能決定.

#### B-23. LOADER_Y_OWNER_TRAYARM token 保留但後盤取出交握未接線 (`F7-aLoader-TrayArmTakeout-not-wired`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:49`
- 規劃來源: 第三個 Loader-Y ownership token (`LOADER_Y_OWNER_TRAYARM=2`) 已定義, 但註解稱後盤取出交握 (狀態 LS_READY_TakeOut/LS_TakeOutIng)「Not yet wired」. token 為保留給未來交握的未用死配置.
- 現況/證據: 全樹 grep 任何 `LOADER_Y_OWNER_TRAYARM` 消費者與 `iYOwner[...]==2/=2` 指派: 零讀者 (僅 `:51` 定義). 兄弟 token NONE/SORTARM 有被消費 (`:391-397/411-412/425/659/727`), 故 ownership 模型真實而 TRAYARM 是例外. TrayArm→Loader 互動 (`aTrayArm.cpp:303`→`NotifyTrayArmPickRearTray` `aLoader.cpp:436`) 只設 bRearHasTray=false, 從不取/釋 Y-owner token. 註解所述兩狀態不存在於 eLoaderStatus (`aLoader.h:11-19` 止於 LS_ToRear=5).
- 驗證結論: 信心 88% / 風險 low / 單點 yes — 刻意保留、目前未接線.
- 修法摘要: 此為文件化的前向相容保留非缺陷, 最小「fix」是 scope 決定非改碼. **選項 A** (建議): 維持現狀, 註解已正確述意, 未用編譯期常數無 runtime 成本. **選項 B** (若 dead-config 須移除): 刪 `:51` `static const int LOADER_Y_OWNER_TRAYARM=2;` 並修剪 `:48-50` 保留註解去除 LS_READY_TakeOut/LS_TakeOutIng 字樣; 無其他碼觸及. 任一重編 aLoader.obj. 不要在後盤取出功能真要做前加入未實作的 LS_* 狀態.
- ❓需決策: 保留或移除保留 token? 真未用 (planned-but-unimplemented), 但作者刻意保留作前向相容 anchor. **選項 A** (建議, 零 runtime 成本): 保留. **選項 B**: 移除 `:51` + 修剪註解, 待功能落地再加回. 取決於後盤取出交握是否在近期 roadmap.

#### B-24. Config.ini [Function] RejectCCDfail 讀寫但從不消費 (`DC-tFunction-RejectCCDfail`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/cprod.h:77`
- 規劃來源: 定義為 config key: `Config.cpp:46` ReadBool 載入、`:61` WriteBool 寫回; `Config.Load()` 於 `CosFunction.cpp:1679` 實際呼叫. 兄弟 key `tFunction.UseCCD` 有被消費 (`aLoader.cpp:586`), 故 [Function] section live; RejectCCDfail 為其死成員.
- 現況/證據: 四向反駁失敗. grep 只命中 `Config.cpp:29/46/61` (SetDefault/Load/Save) 與 `cprod.h:77` 宣告, 零控制邏輯讀. [Function] section 確 live (UseCCD `aLoader.cpp:586`). `CustomerFunctionSelect()`(`cprod.cpp:150`) 與 `UpdateAllParameter`(`:146`) 皆空 stub. HT172 cprod.h:76 宣告但亦從不消費也不載入存檔; Original 無此欄位. 真死 config I/O round-trip; 因 reference 亦未實作, 無文件化的預期行為.
- 驗證結論: 信心 88% / 風險 low / 單點 yes.
- 修法摘要: **選項 A** (最安全若不要功能): 刪 struct TFunction 的 RejectCCDfail (`cprod.h:77`) 與 Config.cpp 3 處 (SetDefault 29/Load 46/Save 61); 既有 ini key 變無害忽略. **選項 B** (實作功能): 在 CCD 檢測結果路徑 (近 `aLoader.cpp:586` UseCCD gate) 加 `if(tFunction.RejectCCDfail && ccdResult==FAIL) route to reject/NG bin`, 用既有 bin-routing. B 需定義 reject 行為 (哪個 bin/狀態碼), HT160/HT172 皆無 reference impl. 建議 A 除非真要此功能. 不要套用.
- ❓需決策: 「CCD fail 退件」是否為 HT160S 要的客戶功能? 若 NO → **選項 A** (刪死成員 + 3 行 Config.cpp). 若 YES → **選項 B**, 但 reject 行為未指定 (HT172 宣告卻從不實作), 需 spec: CCD fail 時退到哪個 bin/tray (HT172 有相關但分離的 iCCDFailBinTray 概念) 與記錄何種 production status/yield counter.

#### B-25. Config.ini [Function] UseHitCylinder 讀寫但從不消費 (`DC-tFunction-UseHitCylinder`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/cprod.h:78`
- 規劃來源: config key (`Config.cpp:47` ReadBool/`:62` WriteBool). `config\config.ini` 磁碟上甚至不存在, 故 Load 只套預設, 值從不被作用.
- 現況/證據: 5 路反駁皆閉合. grep 只命中 `Config.cpp:30/47/62` + `cprod.h:78`. 唯一 live TFunction 成員是 UseCCD (`aLoader.cpp:586`). 'Cylinder' grep 命中是無關的 TMyCylinder 類 (aAuto1To6.cpp), 非此 flag. config.ini 不存在 (Load 在 `if(!FileExists) return;` 早退), 值永遠預設 false. HT172 cprod.h:77 僅宣告無消費; Original 無此欄位.
- 驗證結論: 信心 95% / 風險 low / 單點 yes — 繼承死 config.
- 修法摘要: 最小: 4 處移除 UseHitCylinder (cprod.h:78 + Config.cpp SetDefault 30/Load 47/Save 62). 無呼叫者. **替代** (若 HitCylinder 為規劃功能): 保留 config key, 在 loader/auto 控制路徑加讀 `tFunction.UseHitCylinder`(+兄弟 HitRetry) gate 一個 hit-cylinder retry 步驟. 任一前須 scope 決定. 移除後刪 Config.obj 重編.
- ❓需決策: HitCylinder 是規劃未建功能或廢棄死 config? **選項 A** (死): 刪 UseHitCylinder. 但 3 個兄弟 [Function] key (RejectCCDfail、HitRetry、UsePreAlignment) 同樣死, 決定只移除 UseHitCylinder 或整個死 [Function] cluster (HitRetry 與 UseHitCylinder 成對, 應一起移). **選項 B** (規劃): 保留 key 並在控制路徑實作缺的 hit-cylinder 消費者. 編輯前先選 A/B.

#### B-26. Config.ini [Function] HitRetry 讀寫但從不消費 (`DC-tFunction-HitRetry`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/cprod.h:79`
- 規劃來源: config key (`Config.cpp:48` ReadInteger/`:63` WriteInteger), 為未用 UseHitCylinder flag 的同伴 — 整個「hit cylinder retry」功能僅在 config scaffolded 從未接入控制邏輯.
- 現況/證據: 三向反駁失敗. grep 只命中 `Config.cpp:31/48/63` + `cprod.h:79`, 無控制路徑讀. Config.Load() 有跑 (`CosFunction.cpp:1679`) 故填值但無人讀; Config.Save() 從不被呼叫. 屬死 TFunction 欄位群之一 (RejectCCDfail/UseHitCylinder/UsePreAlignment 亦從不消費, 僅 UseCCD `aLoader.cpp:586` 被讀一次). HT172 cprod.h:77-78 同宣告同不消費; Original 零引用.
- 驗證結論: 信心 95% / 風險 low / 單點 yes.
- 修法摘要: **(A)** 死碼移除 (建議): 刪 `cprod.h:79` `int HitRetry;` 與 Config.cpp 3 行 (31/48/63), 最好與兄弟死欄位 UseHitCylinder/RejectCCDfail/UsePreAlignment 一起, 只留真被消費的 UseCCD. **(B)** 接線: 若要 hit-cylinder retry, 在 hit cylinder 致動處 (由 UseHitCylinder gate) 實作迴圈最多 `tFunction.HitRetry` 次 — 但 HT172 從未實作, 無 reference, 屬新功能 spec. 不要套用.
- ❓需決策: 改前定意圖: **(1)** 當死 scaffold 移除 (HitRetry + 同伴 UseHitCylinder, 可能含 RejectCCDfail/UsePreAlignment) — 低風險縮小 config 面; 或 **(2)** 實作真 hit-cylinder retry — 淨新邏輯無 HT172 reference, 需硬體/製程 spec 定義 hit cylinder 何時觸發與「retry」意義. 除非 hit-cylinder 機構在 roadmap, 建議選項 1.

#### B-27. Config.ini [Function] UsePreAlignment 讀寫但從不消費 (`DC-tFunction-UsePreAlignment`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/cprod.h:80`
- 規劃來源: config key (`Config.cpp:49` ReadBool/`:64` WriteBool). 本應編輯這些的 Maintenance 'Function Define' tab (tsMaintFunctionDef, `maintenance.cpp:1255`) 是空殼 — 其子 tab tsFunctionGeneral/tsNetwork 無控制項 (`maintenance.dfm:903-919`), 確認 planned-but-unbuilt 的功能面.
- 現況/證據: 全樹 UsePreAlignment 只在 `Config.cpp:32/49/64` + `cprod.h:80`, 零消費分支. 為四個 load/save-only 死 TFunction 欄位之一. maintenance 'Function Define' tab 確空殼 (`maintenance.dfm:866-922`, tsFunctionGeneral 無控制項、tsNetwork 僅隱形空 panel; maintenance.cpp 無任何 TFunction 引用). HT172 cprod.h:79 亦宣告無消費, 為繼承死欄位; 無曾存在的 pre-alignment 消費者可移植.
- 驗證結論: 信心 95% / 風險 low / 單點 yes.
- 修法摘要: 最小: 刪 `cprod.h:80` `bool UsePreAlignment;` 與 Config.cpp 3 行 (32/49/64). 單點自足無消費者要更新. 改後刪 Config.obj 重編. 廣域可選清理 (同類): RejectCCDfail/UseHitCylinder/HitRetry 亦 load/save-only, 整個 TFunction 除 UseCCD 可一次修剪.
- ❓需決策: 保留或移除. 空 'Function Define' maintenance tab 暗示曾打算有 operator toggle. **選項 A**: 現移除死欄位 (可選含 3 兄弟). **選項 B**: 保留並改為 **建** 缺的消費者 (pre-alignment 分支 + 空 tab 的 UI 控制) 若 pre-alignment 為真 roadmap 功能. 除非 product 打算出貨 pre-alignment, 選 A.

#### B-28. General.ini [BinDisplay] Color0..8 (iBinDispColor[9]) 讀寫但從不讀取 (`DC-GeneralSetting-iBinDispColor`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/GeneralSetting.h:93`
- 規劃來源: 持久化 config 陣列 (General.ini [BinDisplay] Color0..8, `GeneralSetting.cpp:84/115`). 程式碼註解 `ComPort.cpp:164-165` 明說「LED 顏色現依 error/non-error 狀態, 非 per-unit General.ini Color0..8 (留作 manual-test 用)」, 但無 manual-test 路徑讀 iBinDispColor.
- 現況/證據: grep 只在 `GeneralSetting.cpp:50/84/115` + `.h:93`, 零讀者. bin-display 消費者 `ComPort.cpp:184` 只讀 sBinDispText[i], Color 於 runtime 計算 (`:185` `int Color=((i+1)==ErrorArea)?BIN_COLOR_RED:BIN_COLOR_GREEN;`). 連 manual-test 理由都假: maintenance `btnMCUSendDisplayClick`(`:1567`)/`btnMCUSendLightClick`(`:1602`) 從 UI edit `edMCULightValue` 讀色, 非 iBinDispColor. 同伴 sBinDispText 活著, iBinDispColor 是死雙生.
- 驗證結論: 信心 93% / 風險 low / 單點 yes.
- 修法摘要: 移除死陣列 iBinDispColor 與 INI plumbing: (1) `GeneralSetting.h:93` 刪 `int iBinDispColor[9];` (保留被消費的 sBinDispText[9]), 修 `89-91` 註解去除 per-unit color. (2) GeneralSetting.cpp SetDefault 刪 `:50`、Load 刪 `:84`、Save 刪 `:115`. (3) `ComPort.cpp:164-165` 修正過時註解去除 'manual-test use' 字樣. 改後刪 GeneralSetting.obj + ComPort.obj 重編 (header struct 變更使含它的 ~13 個 dependent 重編, 用 `build-ht160s.ps1 -Clean` 或 `-Full`). 既有 General.ini Color0..8 key 變無害孤兒. 這些是 ASCII, plain Edit OK.
- ❓需決策: **(A)** 刪死陣列 (建議 — 真未讀, runtime error/non-error 色規則為議定行為, 依 2026-06-17 註解與 user rule), 或 **(B)** 接回作 per-unit base-color override 讓 ApplyBinDisplayConfig 對 non-error unit 用 iBinDispColor[i] 取代硬寫 BIN_COLOR_GREEN (僅在未來 spec 要 per-unit 可配置色時有意義, 還需 maintenance UI grid 編輯, 今日無). 除非 product 要可配置 per-unit 色, 選 A. 注: 移除為一行 header struct 變更, 強制 ~13 dependent 重編 (低風險, build 時非純單檔).

#### B-29. Mot_Table HomeOrder 欄解析進唯寫 motor TStringList (`DC-MotTable-HomeOrder`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/MotorAndIO/MyMotor.h:162`
- 規劃來源: Mot_Table.csv 'HomeOrder' 欄 (header col 27) → `database.cpp` emotHomeOrder=26/FindMotColumn(517) → TMOTDATA.HomeOrder(`:546`) → `MotPtr[i]->SetHomeOrder(Data->HomeOrder)`(`:1574`). 該欄在 `system/Mot_Table.csv` 全空 (各列 '...,,,0,0'), 且 `database.cpp:1587-1595` 註解確認 home 排序已改為固定 card-native HomeType=7, 故 per-axis HomeOrder list 死.
- 現況/證據: `TStringList *HomeOrder;` (`MyMotor.h:162`) 由 `SetHomeOrder()` (`MyMotor.cpp:275-286`) 填 (`HomeOrder->CommaText=`), 但 list 從不被走訪/讀; 無 GetHomeOrder accessor. `uMotorTest.cpp:1871` echo 的是 `Data->HomeOrder` (raw CSV 欄) 非 motor list. HT172 uhome.cpp:1412-1435 主動走訪 HomeOrder gate dependency-ordered homing — 該 reader 在 HT160 no-FSM 移植被丟. 20 列 CSV HomeOrder (col 27) 全空.
- 驗證結論: 信心 93% / 風險 low / 單點 no — by-design 改為 HomeType=7, 但殘留 write-only list/欄/解析鏈無消費者.
- 修法摘要: 死 config 清理. **最小** (只移成員): 刪 TMyMotor::HomeOrder + SetHomeOrder — `MyMotor.h:162`(成員)/`:228`(宣告)、`MyMotor.cpp:246`(new)/`:271`(delete)/`:275-286`(body)、`database.cpp:1574`(呼叫). **全鏈** (含 CSV plumbing): 另移 `database.cpp:450`(emotHomeOrder)/`:517-518`/`:546`、`database.h:139`/`:189`、uMotorTest 欄 (`:74`/`:538`/`:545`/`:832`/`:1093`/`:1871`). CSV 第 27 欄可留 (無害尾欄) 或從 header+列移除. struct 變更建議 `-Full`. 最低風險替代是留著只加註死碼.
- ❓需決策: 清理範圍? **(A)** 只移 write-only list + SetHomeOrder (最小 diff, 留 CSV/TMOTDATA/grid). **(B)** 移整個 parse-and-mirror 鏈含 database.cpp 欄索引 + TMOTDATA 欄 + uMotorTest 欄. **(C)** 保留標明死碼 (惰性無害). 另決定是否實體刪 CSV 第 27 欄. 建議 A 或 C; B 待更廣 Mot_Table schema 清理時再做 (動 TMOTDATA/grid 範圍較大, 純美觀收益).

#### B-30. Motor OriginRange/OriginRate 由 Mot_Table Range/Rate 餵入但從不讀取 (`DC-Motor-OriginRangeRate`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/MotorAndIO/MyMotor.h:149`
- 規劃來源: 源自 Mot_Table Range/Rate 欄 (`Data->iRange/iRate`, 來自 emotRange=15/emotRate=10). 作為 'Origin' baseline 快照寫入 motor (`database.cpp:1581-1582`), 但無碼查詢快照.
- 現況/證據: `int OriginRate; int OriginRange;` (`MyMotor.h:149-150`). 全 repo grep 只有指派 (`database.cpp:1581-1582`) 與 init (`MyMotor.cpp:255-256` =0), 無讀, 特別無 SetRange(OriginRange)/SetRate(OriginRate) 還原路徑. 真實資料路徑分離 live: SetRange/SetRate (`MyMotor.cpp:355-356`) 寫 driver, GetRange() 被 `uMotorTest.cpp:3019` (Tolerance=GetRange()) 消費. HT172 同死模式 (指派 `database.cpp:1659-1660` 從不讀); 意圖是 baseline 快照供還原 accel/range, 但任何版本皆未實作消費者.
- 驗證結論: 信心 90% / 風險 low / 單點 no — 非 HT160 by-design 省略 (所有版本皆未做消費者).
- 修法摘要: 刪兩個未用 shadow 欄與其寫入 (無讀者): `MyMotor.h:149-150`、`MyMotor.cpp:255-256`、`database.cpp:1581-1582`. 跨 3 檔 6 行; live range/rate 路徑 (SetRange/SetRate + GetRange/GetRate) 不動. 走 build gate (刪 MyMotor.obj + database.obj 重編). 非嚴格單點 (3 檔) 但機械式, 皆相依死碼.
- ❓需決策: **(A)** 刪死欄 (建議 — 去除混淆, 零行為改變), 或 **(B)** 保留並 **實作** 原意 (移動後改了 range/rate 時呼叫 SetRange(OriginRange)/SetRate(OriginRate) 還原 baseline). B 需 spec 決定: HT160 是否有任何機制在運作中變動 range/rate 需 baseline 還原? 若無, 選 A.

---

### 4.5 unhandled-state (10 筆)

#### B-31. TrayArm TrayFeed 撤離依賴尚不存在的 Loader/EmptyTray 復原交握 (`F6-aTrayArm-IsTrayFeedFinish-incomplete`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/aTrayArm.cpp:85`
- 規劃來源: 註解述空盤撤離「needs Loader/EmptyTray recovery handshakes that do not exist yet (see DecideJob TODO)」, 該方法回 finished 以免 gate, 因「this incomplete path」未做. 隱含規劃的 EmptyTray 復原/撤離流程.
- 現況/證據: 三向反駁. (a) `IsTrayFeedFinish()` 宣告 (`aTrayArm.h:73`) 定義 (`aTrayArm.cpp:82`) 但零呼叫者; bTrayFeedFinish 只被設 true (ctor 29, InitialFlag 41) 從不 false. 死 placeholder accessor. (b) 周邊子系統確認意圖接線: `csystem.cpp:1097` CheckAllTrayFeedFinish stub return false; Run_TrayFeed revert (`:1015-1024`) 依賴它; mode-entry (`:983-987`) 註解掉. HT172 `csystem.cpp:1176` 證明目標形狀 (含真 per-module TrayArm finish flag). (c) 一個不準: 註解 '(see DecideJob TODO)' cross-ref 已 STALE — DecideJob 不再有該 TODO (它引的 Loader-recovery 交握已滿足, `aTrayArm.cpp:226-233`); stale ref 不否定缺口, 只是過度歸因.
- 驗證結論: 信心 82% / 風險 medium / 單點 no.
- 修法摘要: 最小正確 fix **非** 孤立改 IsTrayFeedFinish (無人呼叫). (1) 讓 bTrayFeedFinish 反映真實: DoTrayArm 中 active 時設 false, 手臂 idle 無盤 (`HasTray()==false && Status==TAS_IDLE && Job==TAJOB_NONE`) 時 true; InitialFlag(reset) 重置. (2) 由 `csystem.cpp:1097` 消費: 以聚合 live module accessors (TrayArmModule->IsTrayFeedFinish() + EmptyModule/LoaderModule/AutoModule finish) 取代 return false, 仿 HT172 `csystem.cpp:1176`. (3) 僅在 (1)(2) 真實後重啟 `csystem.cpp:983-987` mode entry, 否則 TrayFeed mode 仍 hang. 不要在 aggregator 仍 stub 時 enable mode. 改後刪 .obj 重編.
- ❓需決策: HT160S TrayArm 只搬空盤無 Magazine 子系統 (HT172 TrayFeed 排空 magazine + magazine-arm + empty1 + tray-arm). 決定: **(A)** 'TrayFeed finish' 對 HT160 意義 — 僅「TrayArm idle 無盤且所有 Auto 需求已滿足」, 或還須排空 EmptyTray/Loader-rear 滯留盤? **(B)** Run_TrayFeed run-mode 是否現在在 HT160 scope, 或刻意延後 ship SKIP-only (現行)? 若 (B) 為「by design 延後至硬體/流程存在」, 則留 IsTrayFeedFinish()/stub 為文件化 placeholder 可接受、現在不改碼 — 僅最終應修正 `aTrayArm.cpp:84-87` 的 stale '(see DecideJob TODO)' cross-ref.

#### B-32. SortArm 單 Z-home 請求層存在但設旗標的 writer 尚未接線 (`F8-csystem-bSortArmNeedHome-not-wired`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/csystem.cpp:951`
- 規劃來源: home-lifecycle handler 保留 'Layer 3' 分支作用於 bSortArmNeedHome 並呼叫 DoSortArmZHome(), 但註解稱 writer (SortArm fault path)「is not yet wired; the flag stays dormant until a future module change sets it.」隱含規劃的 SortArm fault-recovery 單軸 re-home.
- 現況/證據: (a) DoSortArmZHome() 有被呼叫 (`csystem.cpp:954`) 且 bSortArmNeedHome 有被讀 (`:181` gate DoAllProcess; `:952` Layer-3) — 消費端已接線 live. 但全樹 grep `bSortArmNeedHome = true` 零命中; flag 只被寫 false (cmydef.cpp:36 init、csystem.cpp:927/955). 無 true-writer → Layer-3 + DoSortArmZHome() 不可達, `:181` gate 永不觸發. (b) 非 by-design (註解明述延後). (c) HT172 0420 在 `MySortArmParameter::MoveSortArmToAutoSafe()` (`aSortArm.cpp:2282-2288`) 於 suck-Z 軸 production 中失去 HomeLed 時設旗標 (iRetryCount>100); HT160 重寫的 TSortArmModule 無等同路徑 (無 MoveSortArmToAutoSafe/MotorMove(10) retry/iRetryCount/旗標寫). HT160 缺 SortArm suck-Z home-loss 自動復原.
- 驗證結論: 信心 88% / 風險 medium / 單點 no — 真 planned-but-unimplemented.
- 修法摘要: 接 producer 端. 在 HT160 TSortArmModule (aSortArm.cpp) 加 suck-Z home-loss 偵測, 仿 HT172 MoveSortArmToAutoSafe(): production 路徑中 ScanMotorStatus() 後, 若任何 enabled MSuckZ_n 連續 N 次失去 home/HomeLed (retry counter) 則設 bSortArmNeedHome=true 一次 (重置 counter). 既有消費者 (Layer 3 → DoSortArmZHome() 與 `:181` gate) 即驅動單軸 re-home 並完成時清旗標 — csystem.cpp 不需改. non-blocking 無 Sleep、ASCII 註解、AnsiString、無 FSM. 改後刪 .obj 重編.
- ❓需決策: (1) HT160 重寫 SortArm 是否採用 HT172 週期 suck-Z home-loss 自動復原, 或 dormant Layer-3 刻意為不同/日後 HT160 設計的 placeholder? (2) 若是, detector 該放 TSortArmModule 流程何處 (HT172 在 MoveSortArmToAutoSafe, HT160 無), 失 home 偵測條件與 retry 閾值為何? **低工替代** (若不要復原): 刪死 Layer-3 分支 + DoSortArmZHome() + `:181` gate 以移除誤導 dormant 碼.

#### B-33. RunModeEnum Run_TrayFeed 無 live producer (唯一 setter 註解掉); 消費者不可達 (`F2-runmode-Run_TrayFeed`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/database.h:250`
- 規劃來源: Run_TrayFeed 為完整接線的 run mode (status label、lamp bLampTrayFeed、ProcessRunStatus finish loop、database.cpp:1614-1713 SystemStart guard 皆列舉它). 註解掉的 producer (`csystem.cpp:983-987`) 與 stub CheckAllTrayFeedFinish (`:1097`) 顯示它為 CleanOut->TrayFeed drain 規劃但從未啟用.
- 現況/證據: 唯一 `ChangeRunMode(Run_TrayFeed)` (`csystem.cpp:986`) 註解掉; 全樹無 live 指派. 消費者存在但永不觸發: `:672` (status 'Tray Feed'+lamp) 與 `:1015` (finish loop, 其 CheckAllTrayFeedFinish 本身 stub return false `:1102`). 所有 live ChangeRunMode 只 target Run_Home/Normal/OneCycle/CleanOut. HT172 有完整 working Magazine-drain mode. 三處 in-source 註解記為延後非廢棄.
- 驗證結論: 信心 90% / **風險 high** / 單點 no.
- 修法摘要: **非安全單點 fix; 單獨解除 `:983-987` 註解會有害**. CheckAllTrayFeedFinish() 硬寫 false (`:1102`), 一進 Run_TrayFeed finish loop (`:1015-1024`) 永不完成, 機台卡在 Tray Feed mode (永不回 Normal/SoftStop). 真 fix 為多部分移植 HT172 TrayFeed drain: (1) 實作 CheckAllTrayFeedFinish() 聚合真 per-module finish (HT160 只有 TrayArm, 無 Magazine/MagArm, 須以 Loader/EmptyTray/Auto 取代); (2) 實作 TTrayArmModule::IsTrayFeedFinish() 與 aTrayArm.cpp:84-88 TODO 旗下的 Loader/EmptyTray 撤離交握; (3) 接入 CheckAllTrayFeedFinish; (4) 才解除 producer 與 reset 呼叫. **最小 interim** (若只要手動 drain): 維持 SKIP, 現碼已安全刻意. 現行行為無缺陷, 屬功能補完非除錯.
- ❓需決策: HT160 是否需要 Run_TrayFeed (CleanOut 後空盤 drain)? HT172 用它排空 HT160 沒有的 Magazine 子系統. **選項 A**: 永久維持現狀 — 宣告 TrayFeed 不適用 HT160 機構, 可選刪死消費者/lamp/enum (低價值, Big5 編碼風險). **選項 B**: 完成移植 (per-module finish 交握 + 真 CheckAllTrayFeedFinish 再啟 producer), 多檔, 需硬體/機構 spec 定義「所有盤已 drain」之意. 選 A 或 B; **無 B 不要啟 producer**.

#### B-34. 整個 eSystemTime enum (stStartTime..stMTBA, stTotalCnt) 定義但從未引用 (`F3-cmydef-eSystemTime`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/cmydef.h:66`
- 規劃來源: 以 stTotalCnt 作 sentinel 的索引 enum, 供 per-category system-time accumulator 陣列 (Start/Pause/PowerOn/Product/Jam/ContactTest/Home/MTBA) — 標準 HT-family 統計表. 完整賦值 enum + count sentinel 指向規劃卻從未接線的 timing/statistics 子系統.
- 現況/證據: grep 每成員與 tag eSystemTime 跨所有 .cpp/.h 只回 `cmydef.h:66-74` 定義, 零消費. 無 SystemTimeRecord 陣列、無 cprod.h 成員、無 MTBA/MTBF/ContactTest 計算、無 labMTBA UI. (唯一 SystemTime hit 是 uHGemHT160 sSystemTime SVID 1027, 無關 SECS 時戳). HT172 完整接線此 enum (cprod.h SystemTimeRecord[stTotalCnt] + main.cpp:2006 MTBA + data.cpp:100 labMTBA); 原 HT160S 亦有 (SystemTimeRecord[8] 持久化 lastdata.ini [TimeData]). HT160S_BCB 帶過 enum + TimeData UI 殼 (sgTimeData/tsTimeData/btSaveTimeData) 但丟掉整個資料層.
- 驗證結論: 信心 88% / 風險 low / 單點 no — 真 partial-port 孤兒.
- 修法摘要: **(A)** 若要 per-category timing 統計: 重移植資料層仿 HT172 — 在 cprod.h/database.h LAST_GENERAL_SET 加 `long SystemTimeRecord[stTotalCnt];`(可選 Name 陣列), ClearLastSet/database.cpp init 歸零, run loop 累加 (移植 main.cpp:1998-2008 MTBA, HT172 FSM 改 HT160 procedural switch(Task)), 持久化 lastdata.ini [TimeData], 綁既有 sgTimeData/tsTimeData/btSaveTimeData. 多檔. **(B)** 若不要: 刪 `cmydef.h:66-74` 死 enum, 移除前確認 sgTimeData/tsTimeData UI 亦 intended-dead/repurposed. 不要在 scope 決定前套用.
- ❓需決策: per-category system-time/MTBA 統計子系統是否該存在於 HT160S? **(1)** 移植 — 重實作資料層 + 累加 + ini 持久化 + 綁已存在的 sgTimeData/tsTimeData/btSaveTimeData UI (多檔中度工, 符 HT172 + 原 HT160S). **(2)** 丟棄 — 刪孤兒 enum (並決定 TimeData UI tab 去留). enum 本身無害 (編譯無 runtime), 屬產品/scope 決定非 correctness bug.

#### B-35. eHomeError LED 狀態由 ShowLed 畫但從不產生 (home 失敗永不亮紅) (`F4-uHome-eHomeError`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/uHome.h:46`
- 規劃來源: eHomeError=2 為刻意 LED 色狀態, 有專屬 clRed paint 分支. 同 enum 與同 unreachable eHomeError 分支也存在 HT172 (`uhome.cpp:99`, `uhome.h:39`), 確認移植為規劃 home-failure 指示但無 trigger.
- 現況/證據: 消費者存在 (`uHome.cpp:107` `else if(attr==eHomeError) LedPtr[index]->TrueColor=clRed;`) 但 producer 缺: 唯一 caller ShowMotorHomePos (`:115-134`) 只發 eHomeOk(:126)/eHomeBusy(:128)/eHomeUnuse(:133). grep 任何 `ShowLed(...,eHomeError)`/`= eHomeError` producer 無. 中途 alarm 的 motor 永遠停在 eHomeBusy (黃), 紅 error 不可達. 註: 全機台 alarm 仍經全域 Note/alarm 浮現, 故為缺少 on-form 指示非靜默安全失效. 關鍵: HT172 (移植源) 亦從不產生 eHomeError, HT160S-Original 無此 enum.
- 驗證結論: 信心 80% / 風險 low / 單點 yes — 繼承自 reference 的死碼.
- 修法摘要: 單點於 TfHome::ShowMotorHomePos (`uHome.cpp ~115-135`). enabled motor 未 home 時區分 error 與 busy: 若 motor 帶 live drive alarm (`HSys.MotPtr[i]->Led[iAlarmLed]==true`) 發 `ShowLed(i, eHomeError)` (紅), 否則 eHomeBusy (黃). Sketch: `if(bHomeFlag) ShowLed(i,eHomeOk); else if(...Led[iAlarmLed]) ShowLed(i,eHomeError); else ShowLed(i,eHomeBusy);` 重用既有 iAlarmLed 訊號與已畫的 clRed 分支, 無新狀態無 engine 改. 走 build gate (刪 uHome.obj 重編); ASCII 註解保 Big5. 不要套用.
- ❓需決策: 點亮 per-motor 紅色 home-error LED 是否為 HT160S 想要的 UX (全機台 alarm 已經 Note/alarm 浮現, 且 HT172 reference 本身從不接此紅狀態)? 若要 per-axis 指示, 上述一行 iAlarmLed-gated fix 即足; 若不要, 替代是維持現狀並刪死 eHomeError enum + clRed 分支以去除 unreachable-state smell. 擇一; 勿兩者都做.

#### B-36. eTrayArmStatus TAS_PLACING 定義但從不產生; Status 唯寫 (GetStatus 從不呼叫) (`F5-trayarm-TAS_PLACING`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/aTrayArm.h:17`
- 規劃來源: TAS_PLACING 是已產生的 TAS_PICKING/TAS_CARRYING 在 pick/carry/place 循環中的自然 place-phase 同伴; DoPlace 在本該 placing 的階段執行但 Status 留在 TAS_CARRYING. 完整 enum + 公開 GetStatus() 指向規劃但從未消費的 status-reporting/telemetry hook.
- 現況/證據: Status 只被指派 TAS_IDLE/TAS_PICKING/TAS_CARRYING (`aTrayArm.cpp:21,54,541,546,552,555,563,579`); TAS_PLACING 從不指派 (grep 只回 `aTrayArm.h:17` 定義). 唯一 Status 讀是 GetStatus() 內 `return Status;` (`:71`), 無 if/switch 比較. TTrayArmModule::GetStatus 零呼叫者 (iosetview/MyBinDisp 的 GetStatus 屬無關類). 對比: 同儕 LoaderModule 的 Status 是 load-bearing (多處 conditional 讀), 證明 status 可有意義, 但單實例 TTrayArmModule 從不讀自己的. 純 write-only telemetry; 嚴重性 cosmetic.
- 驗證結論: 信心 80% / 風險 low / 單點 yes.
- 修法摘要: 局部單點於 aTrayArm.cpp DoTrayArm(). case 1000 中 DoPick(1) 成功且 PlaceDest 選定後, 在 DoPlace(0) 前設 `Status=TAS_PLACING` (取代 `:563` 的 TAS_CARRYING, 或加在 PlaceDest 決定後使 CARRYING 標 transit、PLACING 標實際 place). home-resume 分支 (case 100, `:541-542`) 亦同. 使 enum 完整可觀察. **純 cosmetic/telemetry** — GetStatus() 無 caller 且 Status 從不控制, 不改機台行為. 若不要 telemetry, 同等有效替代是刪 TAS_PLACING + 從不呼叫的 GetStatus() + write-only Status. 決定前不套用.
- ❓需決策: TTrayArmModule status 是真 telemetry/UI/SECS hook 或死樣板? **選項 A** (additive, 若未來規劃 TrayArm status 顯示/SECS): place 階段 (case 1000 與 case-100 resume) 指派 TAS_PLACING 使 enum 完整 — 但在接 GetStatus() 消費者前 inert (今日無). **選項 B** (subtractive, 若不規劃 status reporting): 刪 TAS_PLACING + write-only Status + 從不呼叫的 GetStatus() (單實例手臂不需內部 status). 任一皆乾淨單點; 依 TrayArm telemetry 是否在 roadmap 而定.

#### B-37. eMotorKind 非預設值 (eLinerMotor/eCylinderMotor/eVoiceCoilMotor/eStepServo/eYASKAWA) 可由 config 產生但從不分支 (`F7-motor-eMotorKind-nondefault`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/MotorAndIO/HTMotor.h:35`
- 規劃來源: 完整 motor-type taxonomy (linear/cylinder/voice-coil/step-servo/Yaskawa) 接入 Mot_Table loader 且 per-axis 儲存, 仿 HT-family driver dispatch. 完整 enum + config-binding + getter 但無 behavioral switch, 指向規劃卻從未實作的 per-motor-kind 處理 (每軸都當 plain eMotor).
- 現況/證據: `database.cpp:1576` 可由 raw 'MotorKind' 欄產生任值 0..5. grep 行為使用 (GetMotorKind()==/==eLinerMotor 等/switch) 無. GetMotorKind() 只被 `uMotorTest.cpp:1906` 顯示讀. HT172 **有** 分支 (mymotor.cpp:208 VoiceCoilMotorHome、:569 eStepServo 免 move-completion alarm、mySMCmotor.cpp 多分支); HT160 兩 base-class 分支點皆缺 (Home() `MyMotor.cpp:510` 無 eVoiceCoilMotor dispatch、VoiceCoilMotorHome() 不存在、move-completion 無 eStepServo 豁免). 三個 driver subclass 皆不引用 MotorKind. 上限信心因屬 latent config-vs-behavior 缺口非可觀 runtime bug (實機 Mot_Table 幾乎都 kind 0, 倉庫無 CSV 可確認).
- 驗證結論: 信心 70% / 風險 medium / 單點 no.
- 修法摘要: **(A) 最小/scope-defensive** (建議若 HT160S 單一 kind 硬體): 留 enum+loader, 但讓 loader 主動拒非 eMotor — `database.cpp:1576` 若 MotorKind!=eMotor 則 log Note/EventLog 警告並強制 SetMotorKind(eMotor), 使缺的行為顯式 fail-loud. 單點低風險. **(B) 移植行為** (僅在 HT160S 須驅動 voice-coil/step-servo): 移植 HT172 兩 base-class 分支點進 MyMotor.cpp — (i) Home() 加 VoiceCoilMotorHome() 與 dispatch; (ii) move-completion alarm 加 `!=eStepServo` guard; 及相關 mySMCmotor kind 分支. 多點中/高風險 (動 home+alarm+move-completion), 須 procedural 重寫 (no FSM). 不要在決定前套用.
- ❓需決策: 任何 HT160S 量產機是否真跑非標準 motor kind (linear/cylinder/voice-coil/step-servo/Yaskawa) 於任一軸? 若 NO (單 kind MC88X1/MN200) → fix **(A)** (loader 拒/警非 eMotor). 若 YES → fix **(B)** (移植 per-kind Home() dispatch、eStepServo alarm 豁免、SMC driver 分支), 真功能移植需完整 home/alarm regression (ht160s-home-selftest) 與真的設 kind 欄的 Mot_Table. 解決需看 field Mot_Table.csv (機台側不在 repo) 與硬體 BOM.

#### B-38. eMotionCardType eMC8040A/ePCI885X/ePLCbase 定義但從不產生; MotionCardType 欄從不讀 (`F8-motioncard-nonproduced`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/MotorAndIO/HTMotor.h:24`
- 規劃來源: eMotionCardType 列舉 HT-family 運動卡系列 (MC8040A/PCI885X/MC88x1/SMC/MN200/PLC-base). card-type-specific 行為是此 enum 的標準用法; HT160 以 subclass type (myMC88X1motor/mySMCmotor/myMN200motor) 達成 driver polymorphism, 使 eMC8040A/ePCI885X/ePLCbase 為規劃卻不支援的卡型且 enum 欄本身 vestigial.
- 現況/證據: eMC8040A/ePCI885X/ePLCbase 只在 enum 定義 (`HTMotor.h:24-29`); SetMotionCardType 只以 eMC88x1/eSMC/eMN200/eMotionCardUnknown 呼叫 (`database.cpp:1550-1556` + subclass ctor). 無 producer. GetMotionCardType 3 處皆宣告/定義零 call site — write-only. driver dispatch 實由 CardModel AnsiString 字串比較 + C++ subclass 多型, enum accessor 冗餘. HT172 **有** 讀 GetMotionCardType (mymotor.cpp:1638/1657) gate SMC/MN200 servo-on/command-pos reset; HT160 未移植該 gate. HT172 亦不產生那三個 legacy type (兩樹皆 vestigial), Original 無此 enum/accessor. 實質缺口: HT160 帶 write-only 欄但丟掉 HT172 SMC/MN200 servo-on 消費.
- 驗證結論: 信心 72% / 風險 low / 單點 yes.
- 修法摘要: **(A)** 若 HT160 永不跑 SMC/MN200 卡 (現 Mot_Table MC88X1-only): 視欄為死, 刪 write-only MotionCardType + Set/GetMotionCardType (HTMotor.h:56/103/104; MyMotor.h:226/227; MyMotor.cpp:380/381; database.cpp:1549-1556 + 各 ctor + HTMotor.cpp:23 init), 或維持加註 vestigial. 三個未產生 enum 值留著 (無害 legacy, 同 HT172). **(B)** 若須支援 SMC/MN200 伺服: 移植 HT172 消費者 — 加 `GetMotionCardType()==eSMC||eMN200` gate 進 HT160 MyMotor ServoOnOff/ServoOnResetPos 等同, 使欄 live 符 HT172. 除非 SMC/MN200 在 roadmap, 建議 A/加註. 勿動唯讀 HT172.
- ❓需決策: HT160 是否會驅動 SMC 或 MN200 運動卡 (vs 現 Mot_Table MC88X1-only)? 若 NO → MotionCardType 欄真死; 移除或加註 (三個未產生 enum 值留為無害 legacy 同 HT172). 若 YES → 真缺口是未移植的 HT172 SMC/MN200 servo-on/command-pos-reset gate (mymotor.cpp:1638/1657), 應移植該消費者使欄 live. candidate 把 eMC8040A/ePCI885X/ePLCbase 框為問題是誤導 (HT172 中亦 vestigial); 可行項是 write-only 欄 + 丟掉的 servo-on 消費.

#### B-39. TrayFeed run-mode 撤離半實作: CheckAllTrayFeedFinish stub + 註解掉的 TRAY_FEED 分支 (`F5-trayfeed-finish-stub`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/csystem.cpp:1097`
- 規劃來源: HT160 宣告 CheckAllTrayFeedFinish (`csystem.h:40`)、跑 Run_TrayFeed RunMode 分支 (`:1015-1024`)、保留 wiring 註解在原處引用 HT172 `ShowSystemError(SnFKCleanOut,K_SKIP|K_TRAY_FEED)` — 顯式 planned-but-unwired TrayFeed drain.
- 現況/證據: `:1097` body 註解 'no per-module TrayFeed finish flag exists yet ... preserve current behavior'; `:979` 'CheckAllTrayFeedFinish() is still a stub ... will not auto-complete'; CleanOut->TRAY_FEED operator 路徑註解掉 (`:983-987`); `aTrayArm.cpp:82-88` IsTrayFeedFinish 註 'EmptyTray recovery handshakes that do not exist yet'. CleanOut-finish prompt (`:982`) 只傳 K_SKIP, operator 連 TRAY_FEED 都選不到. HT172 有完整 live 實作. 三處 in-code 註解記為延後. 小誤: candidate 命名 HT172 bFlagAuto1/2TrayFeedFinish + bEmpty1TrayFeedFinish, 但 HT172 aggregator 實用 Magazine/MagArm flag + TrayArmPara->bTrayArmTrayFeedFinish + CheckEmpty1TrayFeedFinish() — 不削弱缺口.
- 驗證結論: 信心 88% / **風險 high** / 單點 no.
- 修法摘要: 非單點; 為延後子系統移植. 最小可行 wiring 需於 `csystem.cpp ~982`: (1) 還原 `ShowSystemError(SnFKCleanOut.Name, K_SKIP|K_TRAY_FEED, 0)` 並解除 retCleanOut==K_TRAY_FEED → CheckAllTrayFeedFinish(true) + ChangeRunMode(Run_TrayFeed) 分支; (2) 實作 `:1097` CheckAllTrayFeedFinish() 聚合真 per-module finish (今日 return false 且不呼叫 IsTrayFeedFinish, 後者亦硬寫 true); (3) 加 Run_TrayFeed job 分支進相關 HT160 模組 (aLoader/aTrayArm/aEmpty) 使盤實際 drain 並設 finish flag — 今日皆無. 因 (2)(3) 依賴 aTrayArm.cpp:84-88 所述「不存在」的 EmptyTray 復原交握, 無該流程無法安全完成. **不要只解除分支**: CheckAllTrayFeedFinish 回 false 且無 module job 時機台會進 Run_TrayFeed 永遠 hang — 比今日 K_SKIP-only 更糟. 留現狀直到 EmptyTray 復原流程指定. 未改任何檔.
- ❓需決策: CleanOut->TrayFeed operator drain 路徑是否真要在 HT160 機構 (單 TrayArm 搬空盤, 無 HT172 的 Magazine/MagArm), 若要, gate per-module TrayFeed-finish 的 EmptyTray-recovery 交握為何? **選項 A**: 指定復原交握並完整移植 Run_TrayFeed (prompt + aggregation + module job) — 大工. **選項 B**: 決定 HT160 不需 operator TrayFeed drain; 移除死 scaffolding (註解分支、stub、Run_TrayFeed 分支、IsTrayFeedFinish placeholder、宣告) 消除陷阱. **選項 C**: 延後 (現狀): 留 K_SKIP-only working path 與 dormant scaffolding. 硬體/復原流程定義前建議 (B) 或 (C).

#### B-40. RecordSafeDoorStates() 接入掃描迴圈但空 (`F4-safedoor-state-record`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/csystem.cpp:1120`
- 規劃來源: stub 函式每 system scan 主動呼叫 (`csystem.cpp:607`) 但什麼都不做. safe-door 硬體存在 (舊 IsSafeDoorOpen 讀 SnSafeDoorFront/Right/Left/Auto6). 空 body 為記錄/持久化門開狀態的規劃 footprint. 註: 舊源無同名函式, 故為新 HT160-rebuild placeholder 而非逐字 dropped 舊例程 — 故較低信心對映 dropped 硬體.
- 現況/證據: 確認 live 非 by-design 空. (a) `:607` DoSystem() 每 tick 無條件呼叫; 宣告 `csystem.h:46`. (b) HT172 (`csystem.cpp:1132`) 有完整實作 'JerryYang 20240807: Record when door opened' (per-door 邊緣偵測, system stopped 時每次開記一次, SystemStart 清, PLC-safety bypass). HT160 空 body 為 ported-signature-body-not-written. (c) footprint 具體 (空 body `:1120-1122` + caller `:607` + 宣告 `:46`). candidate 降信心註記只查 HT160-Original (正確為空) 漏了 HT172 (依 CLAUDE.md 為權威移植 reference) — 實際 **提高** 信心. building block 在 HT160 已備 (RecordProcess 通用; safe-door sensor `:228-238` 已列舉).
- 驗證結論: 信心 86% / 風險 low / 單點 yes. (與 B-2/B-9 為同一 csystem.cpp:1120 空 body 的不同捕獲角度.)
- 修法摘要: 填空 RecordSafeDoorStates() body (`:1120`) 移植 HT172 邏輯, 改用 HT160 離散 safe-door sensor (非 HT172 SnSafeDoor_01.Tag+i 陣列, HT160 無). 具體最小 body: static bool[] 紀錄上次開狀態 + static bClear; SystemStart true 時 ZeroMemory 一次 (bClear gate) 並 return; stopped 時對 IsSafeDoorOpen() 已用的各 sensor (SnSafeDoorFront/Right/Left, SnSafeSlideDoorRight/Left, SnSafeAuto6) 查 `Enable && IsOff()`; false→true 邊緣呼叫 `RecordProcess(sprintf("Safe Door %s is Opened", Name))` 並 latch. ASCII 註解、無 C++11、保 Big5 (bcb6-bytesafe-edit). 之後刪 csystem.obj 重編. 現在不套用.
- ❓需決策: 小 scope 選擇 (非阻擋): HT160 缺 HT172 連續 SnSafeDoor_01 陣列與 PLC-safety 全域 (Enable_PLCSafety_IO/bPLCIOEffect/bIOPowered) — 確認 port 應走訪 HT160 既有離散 safe-door sensor (建議, 符 IsSafeDoorOpen) 且若那些全域不存在則可丟 PLC-safety bypass 分支. 並確認 RecordProcess (operator/process 事件 log) 為意圖 sink, vs alarm/EventLog channel.

---

### 4.6 todo (2 筆)

#### B-41. AGV car-taken sensor 未接線; IsAmrTaken 硬寫 false (實機 Finish/CEID274 永不觸發) (`F1-aAuto1To6-IsAmrTaken-TODO`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/aAuto1To6.cpp:977`
- 規劃來源: 顯式 TODO + 函式 header (`:967-970`): 'Finish (CEID274) condition. The AGV has removed the full car ... real machine needs a per-Auto "car taken" IO point (TBD) wired here - until then returns false and handshake holds at Ready (production stays parked).' Simulation 回 true (`:975-976`) 故 path 可測; 只缺實機 sensor 讀.
- 現況/證據: `return false; // TODO: read the SnAutoX car-taken sensor once the IO point exists`. 三軸反駁失敗: (a) caller live 1s-tick 鏈 (`uHGemEquipment.cpp:175` ServiceAgv → `uHGemHT160.cpp:188` → AgvCoord.ServiceHandshake → `uAgvStation.cpp:207` IsAmrTaken(a)); bUseAMR 為真實持久 INI flag (`GeneralSetting.cpp:66`). (b) 非 by-design-空 (sim 分支 working). (c) 無替代 CEID274 path: grep `EventReport(0,274)`/ClearAmrCar/AGV_READY 恰一處 (`uAgvStation.cpp:210-211`), 無條件由 `:207` IsAmrTaken gate. 實機 false → CEID274 永不發、lock 永不釋、產線 park. HT172 有 SnAutoXHasTray 但無 car-taken sensor, 確認 HT160S-new 待硬體.
- 驗證結論: 信心 96% / 風險 low / 單點 yes.
- 修法摘要: 單點於 `:977`: `return false;` 改 `THTSensor *Sn = GetAutoCarTakenSensor(Index); return (Sn!=NULL && Sn->Enable && Sn->IsOn());` (仿 `uAgvStation.cpp:158-162` shortage-sensor pattern). **前置**: sensor 須先定義於 HT160S sensor table (HSys.Sen.SnAutoX...) 並在 IO_Table.csv 配 MN200 IO 位址 — 該點尚不存在. IO 點配妥前維持 sim-true/real-false stub. 其餘無需改; handshake state machine 與 ClearAmrCar 已完整.
- ❓需決策: 需硬體/spec 決定才能落碼: (1) 實機是否有 per-Auto AGV car-taken/car-removed sensor, 各 Auto 站 (1..6) IO 位址? (2) 若無專用 sensor, 實機 Finish trigger 為何 — 重用 SnAutoXHasTray 由 ON→OFF, AGV-side SECS handshake, 或 operator confirm? **選項 A**: 接新專用 SnAutoXCarTaken (加 HSys.Sen + IO_Table.csv) 再實作讀. **選項 B**: 由既有 presence sensor 邊緣推導. **選項 C**: 由 AGV-side SECS msg 驅動 Finish. 決定 fix 為純 IO-table+一行 (A) 或還需新 SECS 處理 (C).

#### B-42. Bin-display log 在 bMemo 設定時未回顯 ComPort bin memo (P3 TODO) (`F10-MyBinDisp-P3-bMemo-echo-TODO`)

- 位置: `HT160S_Program_BCB_V1.0.0.0/MyBinDisp.cpp:403`
- 規劃來源: 顯式 'P3 TODO' 於 LogBinDisplay(...) 內; 函式簽章帶 bMemo 參數 (AddBinDisplayLog 以 false 呼叫) 目前未用於 ComPort memo echo. 隱含規劃將 bin-display log 行鏡像進 ComPort 表單 memo.
- 現況/證據: `// P3 TODO: when bMemo, also echo to the ComPort bin memo.`. 三軸反駁. (a) LogBinDisplay(...,bMemo=true) 在 11 個 production path 被 live 呼叫 (`MyBinDisp.cpp:181/463/471/495/503/527/547/566/717/819/916`) — candidate 稱「只 AddBinDisplayLog 以 false 呼叫」**錯**且反而強化: bMemo=true 被 live 傳但函式 body (`387-404`) 從不讀 bMemo, 是 dead 參數. (b) 非 by-design: HT172 (`MyBinDisp.cpp:569-570`) 正是 `if(bMemo && fComPort->cbBinCheckLog->Checked) fComPort->MemoAddString(fComPort->memoBinCom,"",asLine);` — 刻意部分移植. (c) 目標控制項在 HT160 不存在 — 無 cbBinCheckLog、無專屬 memoBinCom; ComPort.dfm 唯一 memo 是 memoPanelCom (Pad/Panel COM channel, 不同裝置). echo 今日無目的地.
- 驗證結論: 信心 88% / 風險 low / 單點 no.
- 修法摘要: 鏡像 HT172. 最小: (1) ComPort.dfm 加 TMemo (如 memoBinCom) 與 gate TCheckBox (如 cbBinCheckLog) + ComPort.h 對應 __published 宣告. (2) `MyBinDisp.cpp:403` 把 TODO 換成 `if(bMemo && fComPort!=NULL && fComPort->cbBinCheckLog!=NULL && fComPort->cbBinCheckLog->Checked) fComPort->MemoAddString(fComPort->memoBinCom,"",asLine);` (MemoAddString 已存在 `ComPort.cpp:376`, 上限 500 行). 不要重用 memoPanelCom (Pad/Panel channel, 不同裝置). 因動 .dfm + .h, 遵 BCB designer/build 規則 (備份 dfm+h、刪 .obj、full build).
- ❓需決策: (1) HT160 是否真要獨立的 on-screen bin-display memo + checkbox gate, 或既有 slBinDispLog in-memory buffer + g_BinDispCommLog CSV (同函式已寫) 已足, 使 TODO 可刪而非實作? (2) 若實作, echo 由何 gate — 新 cbBinCheckLog (HT172 style), 或重用既有 GeneralSetting.bBinDispLogVerbose flag (`MyBinDisp.cpp:401` 已查) 以免加新控制項? 加 UI 前建議向 owner 確認, 因需新 DFM 控制項.

---

## 五、後續建議

- **先做低風險清理群 (orphan-ui / dead-config)**: B-21~B-30 的 dead-config (note 相容 stub、myio long-byte、4 個 [Function] key、iBinDispColor、HomeOrder、OriginRange/Rate) 與 orphan-ui 的 B-20 systools 空殼、B-18/B-19 iosetview 惰性控制, 多為單點低風險; 但凡刪 cprod.h/GeneralSetting.h struct 欄位會牽動多 dependent, 仍須走 `-Clean`/`-Full` 重編驗證, 且 [Function] 4 死 key 建議一次處理 (UseHitCylinder 與 HitRetry 成對).

- **這些須先確認產品 scope 才動**: SECS 子系統的 S5/S7 alarm/recipe catalog (B-3/B-5/B-4/B-7/B-11) — 目前皆 framework skeleton, 其中 B-5 (S5F6) 至少應先做「一律回合法 (可空) 回覆」消除 host T3 timeout; AGV `IsAmrTaken` 取車 IO 點 (B-12/B-13/B-14/B-41) 與 AMR upload payload (B-8) 需硬體/IO spec; CleanOut→TrayFeed drain (B-1/B-10/B-31/B-33/B-39) 為跨模組大工且 **風險 high (B-33/B-39)**, 切勿單獨解除註解.

- **不要盲修「規劃殘留」**: 多筆 (B-15 捲帶/取環/卡匣、B-34 eSystemTime、B-37 eMotorKind、B-38 motioncard) 屬刻意移除或 vestigial; 修前先向 product owner 確認該機構/功能是否在 HT160 roadmap, 否則做死碼清理或加註即可.

- **靜默失效優先轉 fail-safe**: B-6 (`TMySMCMotor` stub 回 true/false 假裝正常) 雖目前 dormant (全 MC88X1), 建議優先採 Option A 讓其 fail-loud, 以防誤配 SMC 軸時靜默通過.

- **全部修改皆走規範**: 編譯走 build gate (刪改動 .obj 後 `scripts/ops/build-ht160s.ps1`, struct/wiring 變更用 `-Clean`/`-Full`); legacy Big5 source (`.cpp/.h/.dfm`, 如 note/uteach/csystem) 一律用 `scripts/ops/bcb6-bytesafe-edit.ps1` 避免 Edit 工具毀 Big5; .dfm 編輯避免在 IDE designer 開啟以防靜默 strip 元件; 新註解 ASCII English; 之後跑編碼檢查 `scripts/ops/check-ht160s-source-encoding.ps1`.
