# 第 13 章　警報訊息與排除

本章說明 HT160S 的警報與訊息系統：三種提示視窗（全警報 Note、OK 提示框、是/否確認框）的差異、警報如何被消除（必須先選回復鍵，純資訊提示除外）、警報發生時的蜂鳴器與三色塔燈連動行為，以及具代表性的警報訊息原因與排除方式對照表。請操作員務必先理解「絕不無聲停機」鐵則：任何使 `SystemStart` 落下的故障，一定會彈出警報。

## 13.1 警報系統概觀

HT160S 的提示介面分為兩個層級：

- **全警報視窗 `TfNote` (`fNote`)**：機台真正警報的主要提示介面。彈出時會立即停止所有馬達、清除 `SystemStart`，並使對應機構面板紅色閃爍、發出蜂鳴，要求操作員選擇回復方式後才能繼續。
- **輕量訊息框 `TMyMessageBox` (`MyMessageBox`)**：用於一般提示與確認，分三種呼叫：`ShowMyOKMessage`（僅 OK）、`ShowMyMessage`（Pause）、`ShowMyMessageBox_YES_NO`（是/否確認）。

> ⚠️ 注意：本專案規約規定，所有警報與確認一律使用 `TfNote` 或 `TMyMessageBox`，禁止使用原生 VCL `MessageDlg` / `ShowMessage`。

警報代碼目錄並非手寫常數，而是在開機時由 `CreateSystemAlarmCode()` 依氣缸／馬達／吸嘴設定表動態產生，並輸出到 `system\AlarmList.csv` 供操作員查閱本機台所有可能的警報。

## 13.2 鐵則：任何落下 SystemStart 的故障都會彈警報

HT160S 遵守「絕不無聲停機」鐵則：凡是會使 `SystemStart` 落下（運轉中停機）的故障，都必須透過 `ShowSystemError` / `ShowMotorError` 等彈出警示，不得只是默默停機。背景執行緒每週期掃描馬達與安全感測器（`ScanAllMotorStatus` + `ScanSystemSenser`），偵測到下列任一狀況時都會停機並彈警報：

| 故障 | 觸發訊號 | 停機方式 |
| --- | --- | --- |
| Safety Door Open（安全門開啟） | `SnSafeDoorFront` / `Right` / `Left` / `SlideRight` / `SlideLeft` / `Auto6` 任一 Off | `StopAllMotor` + `ShowSystemError(K_RETRY)` + `SystemStart=false` |
| Emergency Stop（急停） | `SnEMG` / `_1`~`_4` 任一 | `StopAllMotor` + `AllBreakLock`；運轉中彈警報後 `SystemStart=false` + `fAllMotorHome=false` |
| Motor Power Off（馬達電源關閉） | `SnMotorPower` Off 且非上電延遲中 | `StopAllMotor` + `AllBreakLock`；彈警報後 `SystemStart=false` + `fAllMotorHome=false` |
| Ion Fan Alarm（離子風扇異常） | `IsIonFanAlarm`（僅 REALLY 模式，含去抖） | `DecStopAllMotor` + 彈警報 + `SystemStart=false` |
| Air Pressure Low（氣壓過低） | `IsAirCheck` | `DecStopAllMotor` + 彈警報 + `SystemStart=false` |
| 伺服軸警報（離限位） | `ScanAllMotorStatus` 偵測 `ReadServoAlarmOn` 且非 CW/CCW 限位 | 運轉中 `ShowMotorError` + `SystemStart=false` |

> ⚠️ 注意：純安全感測類（EMG／門／氣壓／離子風扇）若無對應警報對話框，依設計只亮紅燈、不發蜂鳴；蜂鳴的 `ErrJam` 狀態只由 Note 觸發。
>
> ⚠️ 注意：離子風扇警報僅在 `iRealDummy = REALLY(2)` 模式生效，並具時間去抖（回原點中 20 秒、穩態 5 秒，馬達上電 settle 期間略過）。

## 13.3 全警報視窗 `TfNote`

`TfNote` 為彈出式（ShowModal）全警報視窗。顯示期間整機為 modal 狀態，主流程 `MainProc` 暫停。

### 13.3.1 控制項

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `TfNote` (`fNote`) | form/popup | 全警報彈出視窗；顯示時停止所有馬達、清除 `SystemStart`，並對對應機構面板做紅色閃爍 |
| `edtAlarmCode` | edit（唯讀） | 顯示警報代碼字串（如 `SYS0001`、`SUCxxx`、馬達／氣缸動態碼） |
| `edtAlarmMsg` | edit | 顯示警報訊息文字（依語系顯示 `E_ErrMessage` 或 `C_ErrMessage`） |
| `Memo1` | memo | 顯示警報細節描述（Description）與操作員動作歷程（`RecordProcess` 加時間戳） |
| `BtnHome` | button | 回復鍵：先回原點再重試（`K_HOME=0x20`），僅當 `KeyCode` 含此位元才顯示 |
| `BtnSkip` | button | 回復鍵：跳過此料／動作（`K_SKIP=0x01`） |
| `BtnRetry` | button | 回復鍵：重試造成警報的動作（`K_RETRY=0x02`） |
| `BtnTrayFeed` | button | 回復鍵：補盤／送盤（`K_TRAY_FEED=0x04`） |
| `BtnTrayEnd` | button | 回復鍵：盤子結束（`K_TRAY_END=0x08`） |
| `BtnCleanOut` | button | 回復鍵：清機／清空（`K_CLEAN_OUT=0x10`） |
| `BtnStart` | button | 恢復生產（`SoftStart=true`）並關閉視窗；僅在已選回復鍵或本警報無回復鍵（`KeyCode==0`）時才關閉 |
| `BtnPause` | button | 停機（`SoftStop=true`、`DecStopAllMotor`）並關閉視窗；同樣受回復鍵選擇閘控 |
| `BtnOffBuzzer` | button | 消音：設定 `bOffBuzzer` 鎖存並 `CloseBuzzerOff()`，使蜂鳴不再每掃描重啟；視窗仍開啟 |
| FlushPanel（機構面板群） | panel | 機台俯視示意（`pn_System`、`pn_InArmF`、`pn_InShuttle`、`pn_TempAndTest`、各 SafeDoor 等及料格）；警報機構對應面板由 `Timer1` 以 `ALARM_COLOR` 紅色閃爍定位 |

### 13.3.2 全警報彈出與回復流程

1. 機台動作邏輯偵測異常，呼叫對應的 `ShowXxxError` 包裝函式（`ShowMotorError` / `ShowCylinderError` / `ShowSuckError` / `ShowSystemError` / `ShowJamError` / `ShowShuttleError` / `ShowCCDError` / `ShowTNTError` 等），最終都進入 `ShowNoteAlarm()`。
2. `ShowNoteAlarm` 立即執行 `DecStopAllMotor()`、`SystemStart=false`、清除 `SoftStop` / `SoftStart`（強制停機）。
3. 設定 `KeyCode`（提供哪些回復鍵）、`Code`、`Message`、Memo 明細，並依 `FlushPanelName` 定位閃爍面板（找不到時預設 `pn_System`）。
4. `FormShow` 依 `KeyCode` 位元顯示／隱藏對應回復按鈕，並點亮前面板回復鍵 LED；若未消音則 `PlayAlarmBuzzer()` 立即發聲。
5. `Timer1` 每跳一次呼叫 `ScanKey()` 讀實體面板按鍵，並使 FlushPanel 紅色閃爍；`ScanKey` 讓實體 Skip／Retry／… 鍵等同觸控選取，Start／Pause 鍵呼叫對應 Click。
6. 操作員（畫面或實體面板）選一個回復鍵 → `UpdateButtonStatus` 將 `ReturnCode` 設為對應 `K_*` 位元。
7. 按 **START**（恢復）或 **PAUSE**（停機）；只有「已選回復鍵」或「`KeyCode==0` 純資訊」時才真正 `Close()`。
8. `FormClose` 執行 `CloseBuzzerOff`、清回復鍵 LED、還原面板顏色；`ShowNoteAlarm` 回傳 `ReturnCode` 給呼叫端決定後續動作（Skip／Retry／Home 等）。

> ⚠️ 注意（關鍵安全規則）：若警報有提供回復鍵（`KeyCode!=0`）但操作員未選任何一個，按 START／PAUSE 都不會關閉視窗，避免未處理就帶過警報。只有 `KeyCode==0` 的純資訊提示可直接 START／PAUSE 關閉。此規則對齊 HT172／HT9045。

### 13.3.3 回復鍵 KeyCode 對照

| 回復鍵 | 位元值 | 意義 |
| --- | --- | --- |
| `K_SKIP` | `0x01` | 跳過此料／動作 |
| `K_RETRY` | `0x02` | 重試造成警報的動作 |
| `K_TRAY_FEED` | `0x04` | 補盤／送盤 |
| `K_TRAY_END` | `0x08` | 盤子結束 |
| `K_CLEAN_OUT` | `0x10` | 清機／清空 |
| `K_HOME` | `0x20` | 先回原點再重試 |
| `0`（無回復鍵） | `0` | 純資訊提示，可直接關閉 |

`KeyCode` 為位元遮罩，可由多個 `K_*` 以 OR 組合，由呼叫端傳入；同一警報依此決定顯示哪些回復按鈕。

## 13.4 輕量訊息框 `TMyMessageBox`

`TMyMessageBox` 為輕量訊息框，與 `TfNote` 不同。

| 控制項 | 類型 | 功能 |
| --- | --- | --- |
| `palPause` | button | 一般確認（Pause） |
| `palYes` / `palNo` | button | 是／否確認（`Tag=1/2` 決定回傳 `ret`） |
| `Button2` | button | 消音（Off Buzzer）；僅警報式訊息顯示，是/否確認時隱藏 |

### 13.4.1 OK 提示流程（`ShowMyOKMessage`）

1. 呼叫 `ShowMyOKMessage(字串/代碼)`。
2. 執行 `DecStopAllMotor` + `SystemStart=false`（停機），`PrepareNormalMessage` 設 OK 按鈕。
3. `ShowModal`；`FormShow` 若 Off Buzzer 鈕可見則 `PlayMessageBuzzer()`。
4. 操作員按 OK（`palPause`）關閉，`FormClose` 執行 `CloseBuzzerOff`。

> 另有 `ShowMyOKMessageNoStop()`：設 `bFormShowNoStop=true`，**不停馬達、不清 `SystemStart`**，用於資訊／警示性質訊息，取代原生 VCL `MessageDlg`/`ShowMessage`。

### 13.4.2 是/否確認流程（`ShowMyMessageBox_YES_NO`）

1. 呼叫 `ShowMyMessageBox_YES_NO(字串)`。
2. 隱藏 Pause / Off Buzzer，顯示 Yes / No 兩鈕，預設 `ret=msgrtnNO`。
3. `ShowModal`；不停機（`bFormShowNoStop=true`），是/否確認不發蜂鳴（`Button2` 隱藏）。
4. 操作員按「是」→ `ret=msgrtnYES(1)`；按「否」→ `ret=msgrtnNO(2)`；回傳給呼叫端。

## 13.5 蜂鳴器與三色塔燈連動

警報的聲音與燈號連動分兩種驅動路徑：

- **非 modal 期間**：每掃描由 `DoSystemMessage()` 依 `BuzzState` 階梯（Note → MsgBox → Home → Running → Pause）選擇 `SwMusic` 與 `SwTowerRed` / `SwTowerYellow` / `SwTowerGreen`。
- **modal 對話框期間**：因 `MainProc` 暫停，每掃描蜂鳴驅動不執行，改由 `FormShow` 的 `PlayAlarmBuzzer()`（Note）／`PlayMessageBuzzer()`（訊息框）立即補聲。

各運轉狀態對應的蜂鳴音樣式可於維護畫面 `tsMaintTowerLight` 的 Music Select（`RadioGroup2`~`RadioGroup7`）設定。

| 參數 | 範圍/預設 | 說明 |
| --- | --- | --- |
| `Music Select`（Running / ErrJam / Pause / Message / Heating / Homeing） | `0`=靜音，`1`~`4`→`SwMusic1`~`SwMusic4` | 各運轉狀態對應的蜂鳴音樣式 |
| `FlushPanelName` | 如 `pn_System` / `pn_InArmF` / `pn_InShuttle` / `pn_TempAndTest`；空字串時預設 `pn_System` | 警報對應的機構面板名稱，用於紅色閃爍定位 |
| `iLanguageCountry`（`HSys.LastSet`） | `0`=英文，其他=中文 | 決定顯示 `E_ErrMessage`/`E_Description` 或 `C_ErrMessage`/`C_Description` |
| `AlarmTablePath` | `CurrentDir\system\AlarmList.csv` | 開機產生之警報代碼目錄輸出路徑 |

### 13.5.1 消音（Off Buzzer）

按 **Off Buzzer** 會設定鎖存旗標（`bOffBuzzer` / `fBuzzerOff`）並呼叫 `CloseBuzzerOff()`。鎖存後，`DoSystemMessage` 對 `LED_ErrJam` 會檢查 `fNote->IsBuzzerOff()` 而停止重新驅動蜂鳴；前面板的 **ALARM RESET** 實體鍵亦會呼叫 `CloseBuzzerOff()`。

> ⚠️ 注意：消音僅關閉蜂鳴，警報視窗仍開啟，仍需選擇回復鍵後才能 START／PAUSE 關閉。

## 13.6 警報代碼目錄與開機產生

開機時 `SYSTEM_MODULAR::CreateSystemAlarmCode()` 動態建立整份警報目錄：

1. 清空 `mapNameToAlarm` / `mapAlarmCodeList`。
2. 逐一氣缸：產生 6 種錯誤碼（格式 `%d%03d%1d`，`eCynAlarm=4` + 缸序 + 錯誤子碼 `0..5`），英中訊息與 FlushPanel 一併登錄。
3. 逐一馬達：產生 9 種錯誤碼（`eMotorAlarm=5` + 馬達序 + `0..8`）。
4. 逐一吸嘴：產生 6 種錯誤碼（`eSuckAlarm=6` + 吸嘴序 + `0..5`）。
5. 將整份 `mapAlarmCodeList` 以 CSV 輸出到 `system\AlarmList.csv`（標頭 `AlarmCode,AlarmType,E_ErrMessage,C_ErrMessage,E_Description,C_Description`）。

`ShowSystemError(Name, ...)` 以 `mapNameToAlarm` 查名稱 → 代碼，再以 `mapAlarmCodeList` 取雙語訊息與面板；查無時走未定義分支，以代碼首碼判斷類型並顯示 `Xxx Code Undefine Error`，代碼前加 `-` 標示。

> 【待補：`C_ErrMessage` / `C_Description` 目前以英文填入（原始碼註解說明為保持 ASCII），實機是否已切換中文語系字串需現場確認 `system\AlarmList.csv` 內容。】

## 13.7 代表性警報訊息與排除（對照表）

> ⚠️ 注意：下表為**代表性**清單，並非完整列舉。本機台的完整警報目錄請查閱開機產生的 `system\AlarmList.csv`（依本機氣缸／馬達／吸嘴設定動態產生）。表中代碼皆取自規格，未列舉者請勿臆測。

### 13.7.1 系統與安全類

| 警報訊息／代碼 | 原因 | 排除方式 |
| --- | --- | --- |
| `SYS%04d` / 具名系統警報（`ShowSystemError`） | 系統錯誤，面板 `pn_System`；具名版以名稱查 `mapNameToAlarm` 取代碼與雙語訊息 | 依 `KCode` 提供回復鍵（多為 RETRY/HOME）；查無代碼顯示 `Xxx Code Undefine Error`，需現場確認代碼來源 |
| Safety Door Open | 安全門開啟（`SnSafeDoorFront/Right/Left/SlideRight/SlideLeft/Auto6` 之一 Off） | `ShowSystemError K_RETRY`；關閉所有安全門後按 Retry 重啟 |
| Emergency Stop | 急停按下（`SnEMG/_1..4` 之一），立即 `StopAllMotor` + 鎖煞車 | `ShowSystemError K_RETRY`；釋放急停，需重新回原點（`fAllMotorHome` 已清除） |
| Motor Power Off | 馬達電源關閉（`SnMotorPower` Off 且非上電延遲中） | `ShowSystemError K_RETRY`；重新上電後回原點 |
| Ion Fan Alarm | 離子風扇異常（`IsIonFanAlarm`，僅 REALLY 模式，含時間去抖） | `ShowSystemError K_RETRY`；確認風扇運轉後 Retry |
| Air Pressure Low | 氣壓過低（`IsAirCheck`） | `ShowSystemError K_RETRY`；恢復氣壓後 Retry |
| `COM%04d`（`ShowSystemCommError`） | 系統通訊錯誤 | 依 `KCode`；附帶 Note 細節 |
| `CCD%04d`（`ShowCCDError`） | CCD 影像／讀碼錯誤 | 依 `KCode`；附帶 Note 細節 |

### 13.7.2 機構動作類

| 警報訊息／代碼 | 原因 | 排除方式 |
| --- | --- | --- |
| `CYL%04d` / 動態氣缸碼 `4{缸序3}{子碼1}`（`ShowCylinderError`） | 氣缸動作錯誤；子碼 `eOffNotOnErr`/`eOffNotOffErr`/`eOffIsOnErr`/`eOnNotOnErr`/`eOnNotOffErr`/`eOnIsOnErr`（縮／伸／感測器在位異常） | 預設 `K_RETRY`；描述提示 [1] 檢查感測器 [2] 檢查線路 [3] 檢查氣壓/氣管 |
| `SUC%03d%d` / 動態吸嘴碼 `6{吸嘴序}{子碼}`（`ShowSuckError`） | 吸嘴錯誤；子碼 `eSuckPickErr`（取料失敗）/`eSuckDestroyErr`/`eSuckVacOffErr`（真空感測器關）/`eSuckDropErr`（元件掉落）/`eSuckIniOffErr`/`eSuckIniOnErr`（初始感測異常） | 依 `KCode`；描述提示 [1] 檢查真空 [2] 檢查線路 [3] 檢查氣壓/氣管；重複發生時訊息加 `(Again!)` |
| 動態馬達碼 `5{馬達序}{子碼0..8}` / `eMotorAlarm`（`ShowMotorError`） | 馬達錯誤；子碼 `eMotPwrErr`（電源關）/`eMotTorqueErr`（失步超扭）/`eMotCWOnErr`,`eMotCCWOnErr`（正反向極限觸發）/`eMotSoftPErr`,`eMotSoftNErr`（軟體極限）/`eMotPosErr`（位置錯誤需回原點）/`eMotUnDefErr`/`eMotOverLimitErr`（目標超出極限） | `ShowMotorError` 走 `K_RETRY`，並 `fAllMotorHome=false` + `ChangeRunMode(Run_Home)` 要求重新回原點；描述 [1] 檢查電源 [2] 檢查線路 [3] 檢查機構 |
| Servo alarm NOT cleared by motor power-cycle | 回原點電源循環後伺服軸仍鎖存 ALM（馬達繼電器不切斷 A6 控制電源 L1C/L2C） | `ShowMyMessage` 提示循環 MAIN 電源（斷路器）或排除驅動器故障後重新 HOME；`SystemStart=false` 中止避免無聲重試迴圈 |
| `JAM%04d`（`ShowJamError`） | 卡料錯誤，面板 `pn_System` | 依 `KCode`（常見 SKIP/RETRY/CLEAN OUT） |
| `SHT%04d`（`ShowShuttleError`） | 進／出料 Shuttle 錯誤；`pos==0` 面板 `pn_InShuttle`，否則 `pn_OutShuttle` | 依 `KCode` |
| `TNT%04d`（`ShowTNTError`） | Temp And Test 站錯誤，面板 `pn_TempAndTest` | 依 `KCode` |
| `MAG%02d%02d`（`ShowMagazineError`） | Magazine 錯誤；註解標示為相容舊原始碼而保留的 API | 依 `KCode`（保留相容，現場用途需確認） |

### 13.7.3 通用與流程類

| 警報訊息／代碼 | 原因 | 排除方式 |
| --- | --- | --- |
| `ShowMyError(sMyError)` / `ShowErrorMessage(Code)` | 以任意字串作為代碼與訊息的通用警報，面板 `pn_System` | `ShowMyError` 依 `KCode`；`ShowErrorMessage` 固定 `K_RETRY` |
| Loader still holds a tray | 回原點完成（`case200`）時 Loader 車仍持盤（夾爪已於回原點中開啟，盤已鬆脫） | `ShowMyMessage` 要求手動移除所有 Loader L/R 盤，按 OK 後 `ClearTray` 清除盤身分與 map |
| CleanOut finish note | 清機完成提示（`ShowSystemError SnFKCleanOut, K_SKIP`） | 操作員選 SKIP 結束回 Normal 並停機 |
| Please Enter LotID ! | Start 前 LotID 為空 | `ShowMyMessage`；輸入 LotID 後重按 Start |
| No Lot data / No 2D data / By Lot+Bin no binding | Start 前缺 Lot 註冊、缺 2D/Bin 資料、或 Lot+Bin 模式無綁定（`CheckLotDataReady`） | `ShowMyMessage`；補齊資料／綁定後重按 Start |
| `PROCESS`（`RecordProcess`） | **非警報**：操作員／流程動作紀錄（START/PAUSE/SKIP/RETRY/OFF BUZZER pressed 等），寫入 `g_EventLog` 並附加到 Note Memo | 不適用（紀錄用途） |

> 【待補：`AlarmType` 列舉中 `eFunErr(2)`、`eSystemMess(3)`、`eRecordProcess(7)`、`eOther(8)` 的實際警報文字／觸發點未在本批檔案內定義；`ShowSystemError` 未定義分支僅以類型印出 `Xxx Code Undefine Error`，個別案例需現場確認。】

## 13.8 互鎖與設計要點

- 彈出 `TfNote` 時 `ShowNoteAlarm` 立即 `DecStopAllMotor()` 並清除 `SystemStart` 與 `SoftStart`/`SoftStop`，警報期間機台停止。
- 回復鍵閘控：警報若提供回復鍵（`KeyCode!=0`），未選任何回復鍵時 START/PAUSE 皆不關閉視窗，禁止未處理就放行（對齊 HT172/HT9045）。
- `KeyCode==0` 之純資訊 Note 不提供回復鍵，可直接 START/PAUSE 關閉。
- `ErrJam` 蜂鳴只由 Note 觸發；純安全感測（EMG／門／氣壓／離子風扇）若無警報對話框，只亮紅燈不發蜂鳴。
- Off Buzzer 為鎖存（`bOffBuzzer`/`fBuzzerOff`），`DoSystemMessage` 對 `LED_ErrJam` 會檢查 `fNote->IsBuzzerOff()` 而停止重新驅動蜂鳴。
- modal 對話框會暫停 `MainProc`，故每掃描蜂鳴驅動不執行，需靠 `FormShow` 的 `PlayAlarmBuzzer`/`PlayMessageBuzzer` 立即補聲；同一 `fShow` 已開時重複呼叫 `ShowNoteAlarm` 直接 `return 0`（避免重入）。

> 【待補：`SwMusic1..4` 各音樣式（旋律／長短）對應的實際聲音，需現場聆聽確認；`tsMaintTowerLight` 各狀態的 Music Select 預設值由 Maintenance 畫面設定，未在本批檔案內固定。】

> 【待補：`FlushPanelName` 對應到動態建立面板的命名是否與所有 `ShowXxxError` 呼叫端一致（找不到時退回 `pn_System`），個別機構面板閃爍正確性需實機確認。】

> 【待補：`note.dfm` 中文標籤（如 Memo 區與機構示意說明）以 Big5 顯示，部分中文字串未逐字確認。】

---

## 13.x 警報視窗 (Note) 實機畫面

![警報視窗 Note](screenshots/screen-message.png)
> 圖 13-1 全警報視窗 (TfNote) 實機畫面，含回復鍵與訊息列。（擷取方式：主畫面工具列 Message，或任一警報觸發時彈出。）

