# 第 16 章　常見問題 (FAQ)

本章彙整 HT160S 操作員與維護人員最常遇到的問題，採問答 (Q/A) 形式呈現。所有答覆均依據程式原始碼萃取之規格撰寫；凡規格未完整支持之處，已採保守措辭並標註「待現場確認」。實際操作仍以實機行為與「第 01 章 安全須知」為準。

> ⚠️ 注意：本機含伺服馬達與氣壓致動器，即使在 DUMMY 模式下馬達與氣缸仍會實際動作。排除故障時務必確認機台已停止且人員手部離開動作行程。

---

## 16.1 開機、登入與啟動

### Q1. 開機後到可以生產，正常的操作順序是什麼？

依規格的可見操作序列如下：

1. 程式啟動後背景執行緒 (TRunControl) 開始以約 1ms 週期運轉，狀態列顯示 `INIT`。
2. 於 `cbbUserSelect` 選擇使用者權限（見 Q4）。
3. 視需要以 `pnRealDummy` 切換 Real/HasTray/Dummy、以 `pnStartMode` 切換 Initial/Continue（兩者僅能在 `SystemStart==false` 時切換）。
4. 按 `Home` 回原點，確認 `Confirm home?` 後等待 `Motor Home Monitor` 顯示 `Home finished.`。
5. 於 `edLotNo` 輸入 LotID。
6. 按 `Start` 啟動生產。

### Q2. 為什麼一開機馬達就上電了？這正常嗎？

是正常的。依規格，若 Pad 操作面板尚未通訊 (`bPadEverCommunicated==false`)，軟體啟動時會自動 `SwMotorRelay.On` 上電（無互鎖）。當操作面板開始通訊後，馬達電源改由前／後面板的 Power On/Off 鍵控制 (`CheckMotorPowerShutDown`)。

### Q3. 我按了 `Start` 但機台先去回原點，沒有馬上生產，是故障嗎？

不是。若機台尚未回原點 (`fAllMotorHome==false`)，`Start` 會以 `bHomeByStart=true` 先進入 `Run_Home` 並顯示 HOME 監控；回原點完成後會「自動續跑生產」，不會停機。這與直接按 `Home`（完成後停機）不同。

### Q4. 我選了 Supervisor/Engineer 權限，卻出現「User password login is not available yet.」？

依規格，正式機目前僅 `Operation` 權限可直接生效並降權；`Supervisor/Engineer/Honprec` 在正式機會顯示「User password login is not available yet.」（密碼登入尚未實作）。

> 【待補：各使用者權限 (ROLE_OPERATION/SUPERVISOR/ENGINEER/HONPREC) 對應的可操作範圍須查 UserRoleManager 定義並現場確認。】

### Q5. 按 `Start` 後跳出「Please Enter LotID !」怎麼辦？

`Start` 前 LotID 不可為空。請於 `edLotNo` 輸入 Lot 編號後重按 `Start`。

### Q6. 按 `Start` 後提示缺 Lot/2D 資料或「By Lot+Bin no binding」？

`Start` 前會由 `CheckLotDataReady` 驗證 LotID、Lot 數、2D/Bin 資料（以及 By Lot+Bin 模式的綁定）。若缺 Lot 註冊、缺 2D/Bin 資料、或 Lot+Bin 模式無綁定，會以 `ShowMyMessage` 提示。請補齊對應資料／綁定後重按 `Start`。

---

## 16.2 回原點 (HOME)

### Q7. `Home` 與 `Start` 帶起的回原點有何不同？

| 觸發方式 | 完成後行為 |
| --- | --- |
| 直接按 `Home` | `bHomeByStart=false`，回原點完成後停機，監控畫面顯示 `Home finished.` 並自動關閉 |
| 按 `Start`（機台尚未回原點時） | `bHomeByStart=true`，回原點完成後自動續跑生產 |

按 `Home` 會先彈出 `Confirm home?` 是／否確認，並清除所有 `bHomeFlag` 強制重新回原點。

### Q8. 回原點的動作順序是什麼？

依規格的回原點步序（`ProcessMotorHome`）：

1. 電源循環判斷（必要時做伺服電源 Off→On，見 Q9）。
2. 先升 TrayArm Z 上缸並開夾爪（若持盤則保持夾緊不掉盤）。
3. 放 Loader 後鉤、再放前擋（避免兩車共軌拖盤對撞）。
4. 批次回 4 支吸嘴 Z。
5. 確認 Z 上升後，批次回所有 XY 軸。

> ⚠️ 注意：回原點防撞為硬性規則——批次回 XY 前一定先升 TrayArm Z 上缸並確認，避免撞到下方盤；持盤時保持夾爪閉合。

### Q9. HOME 中途中止後再 HOME，要注意什麼？

可按 `Abort Home`（或回原點中按面板 Pause）中止：會 `StopAllMotor`、`fAllMotorHome=false`、關閉視窗。中止後機台仍要求「重新成功回原點」才能運轉。回原點關閉／中止的決策由核心 `ProcessHomeLifecycle` 掌控，HOME 監控畫面 (`Timer1`, 100ms) 只負責顯示。

### Q10. 回原點完成後提示「請移除 Loader 盤」？

當回原點完成 (`case200`) 時若 Loader 車仍持盤（夾爪已於回原點過程開啟、盤已鬆脫），會以 `ShowMyMessage` 要求手動移除所有 Loader L/R 盤。請取盤後按 `OK`，系統會 `ClearTray` 清除盤身分與 map。

---

## 16.3 伺服／馬達警報

### Q11. 出現伺服馬達警報 (Motor Alarm) 該如何清除？

馬達警報 (`ShowMotorError`) 會 `DecStopAllMotor` + `SystemStart=false`，並 `ChangeRunMode(Run_Home)` 要求重新回原點。處理方式：

1. 依警報描述檢查 `[1] 電源 [2] 線路 [3] 機構`。
2. 清除驅動器故障後，重新回原點 (`Home`)。

馬達警報代碼為動態格式 `5{馬達序}{子碼0..8}`，子碼涵蓋電源關 (`eMotPwrErr`)、失步超扭 (`eMotTorqueErr`)、正／反向極限 (`eMotCWOnErr`/`eMotCCWOnErr`)、軟體極限 (`eMotSoftPErr`/`eMotSoftNErr`)、位置錯誤需回原點 (`eMotPosErr`) 等。

### Q12. 回原點做了電源循環，伺服軸還是亮 ALM，HOME 清不掉？

依規格，這代表馬達繼電器並未切斷驅動器（A6）控制電源（L1C/L2C），故鎖存的伺服 ALM 無法靠 `SwMotorRelay` Off→On 清除。此時系統會 `ShowMyMessage` 提示：「循環 MAIN 電源（斷路器）或排除驅動器故障後重新 HOME」，並將 `SystemStart=false` 中止，避免進入無聲重試迴圈。

處理步驟：

1. 依提示循環 MAIN 電源（總斷路器），或排除驅動器故障。
2. 重新回原點 (`Home`)。

### Q13. 為什麼有時候伺服軸在限位上反而不報警就繼續回原點？

依規格，`ScanAllMotorStatus` 若偵測 enabled 伺服軸的 `ReadServoAlarmOn` 且該軸正位於 CW/CCW 限位，會視為回原點正常 (`continue` 不停機)。只有真正「離限位」的警報才會在運轉中觸發 `ShowMotorError` 並放下 `SystemStart`。此外，電源循環期間 (`bHomePowerCycling`) 會略過此判斷。

---

## 16.4 安全感測器與「為什麼機台停了」

### Q14. 機台運轉中突然停止並彈警報，常見原因有哪些？

依規格，下列安全感測器在運轉中觸發時會停機並彈出 `ShowSystemError`（皆為 K_RETRY，排除後 Retry 重啟）：

| 警報 | 觸發條件 | 排除方向 |
| --- | --- | --- |
| Safety Door Open | 安全門開啟（Front/Right/Left/SlideRight/SlideLeft/Auto6 之一） | 關閉所有安全門後 Retry |
| Emergency Stop | 急停按下（EMG/_1.._4）；立即 StopAllMotor + 鎖煞車，並清除 `fAllMotorHome` | 釋放急停後需重新回原點 |
| Motor Power Off | 馬達電源關閉（且非上電延遲中）；清除 `fAllMotorHome` | 重新上電後回原點 |
| Ion Fan Alarm | 離子風扇異常（僅 REALLY 模式，含時間去抖） | 確認風扇運轉後 Retry |
| Air Pressure Low | 氣壓過低 | 恢復氣壓後 Retry |

> ⚠️ 注意：依「絕不無聲停機」鐵則，任何使 `SystemStart` 下降的故障都會彈出警示，不會默默停機。

### Q15. 離子風扇警報只有偶爾出現，且只在某些模式才有？

依規格，離子風扇警報 (`IsIonFanAlarm`) 僅在 REALLY 模式生效，且帶有時間去抖：回原點中為 20s、穩態為 5s，並在馬達電源 settle（上電延遲）期間略過。因此 DUMMY/HAS_TRAY 模式不會觸發此警報。

### Q16. 狀態列顯示 `LOCK` 是什麼意思？面板按鍵沒反應？

`LOCK` 對應 SafeLock (`SnSafeLock`) 啟動的狀態。依規格，在 SafeLock 狀態下 `ScanPanelKeys` 不會消費面板鍵，因此前／後面板按鍵會無作用。請確認 SafeLock 解除後再操作。

### Q17. 主畫面狀態列各狀態代表什麼？

由 `ProcessRunStatus` 設定，可能顯示：`INIT`、`HALT`、`HOMING`、`RUNNING`、`Clean Out`、`Tray Feed`、`One Cycle`、`LOCK`、`EMG`、`MOTOR OFF`、`SAFE DOOR`、`AIR`、`PAUSE`，並對應紅／綠／黃三色燈。

---

## 16.5 暫停、停止與清機

### Q18. 暫停 (`Pause`) 後可以直接續跑嗎？

可以。`Pause` 會設 `SystemStart=false` + `SoftStop=true`，下一週期 `DecStopAllMotor` 減速停止，但「保留回原點狀態」，再按 `Start` 即可續跑。

### Q19. `One Cycle` 與 `Clean Out` 有什麼限制？

| 按鈕 | 限制條件 | 行為 |
| --- | --- | --- |
| `One Cycle` (sbOneCycle1) | 僅允許 `Run_Normal`/`Run_CleanOut` 模式且 Lot 資料就緒 | 切到 `Run_OneCycle`，完成放料後回 Normal 並停機 |
| `Clean Out` (sbCleanOut1) | 僅在 `Run_Normal` 時動作 | 設 `bCleanOut=true` 切到 `Run_CleanOut`，排出機台內殘料 |

### Q20. 清機完成後出現提示要選 `SKIP`？

清機完成提示為 `ShowSystemError(SnFKCleanOut, K_SKIP)`。操作員選 `SKIP` 即結束、回 `Normal` 並停機。

> 【待補：Run_TrayFeed（補盤）模式之 `CheckAllTrayFeedFinish` 目前為 stub，TrayFeed 分支無法自動完成，實機是否啟用待現場確認。】

---

## 16.6 Real / Dummy 模式

### Q21. 如何切換 Real / Dummy？什麼時候才能切？

按 `pnRealDummy` 循環切換，順序為 `DUMMY(0) → HAS_TRAY(1) → REALLY(2)`。依規格，僅能在 `SystemStart==false` 時切換；切換後會寫入 INI（`[System] RealDummy`）、記錄並透過 SECS EventReport 回報。

### Q22. 三種模式有什麼差別？

由規格可確認：

- 離子風扇警報等「實機檢查」僅在 REALLY 模式生效。
- DUMMY 模式下馬達與氣缸仍會「實際動作」，僅略過正確性感應器確認（防撞互鎖在 DUMMY/HAS_TRAY/REALLY 三模式皆持續生效，只在編譯期 SOFT_SIMULATE 略過）。

> 【待補：DUMMY/HAS_TRAY/REALLY 三模式對各模組 (Empty/Color/Loader) IO/動作的細部差異須查三層 IO 檢查文件並現場確認。】

### Q23. 如何切換 Start Mode（Initial / Continue）？

按 `pnStartMode` 切換 `iStartMode`（0=Initial / 1=Continue）。僅能在 `SystemStart==false` 時切換，並同步寫入 INI 與更新 `sbStartIcon` 圖示。

> 【待補：Continue (`iStartMode=1`) 在 `Start()` 流程的實際差異行為——`CheckContinusStartIsReady()` 已被註解停用，目前 Initial 與 Continue 在啟動路徑上看不出程式分支差異，其語意是否僅供記錄/SECS 須現場確認。】

---

## 16.7 Loader（進料）模組

### Q24. Loader 一直不取盤／不掃描，為什麼？

Loader 兩側 (Loader1 左／Loader2 右) 共用同一 Loader-Y 導軌、共用前置疊盤機與後排排出口，因此有多重互鎖會「故意」讓某側等待。常見原因：

1. 對側正在 `FEEDING`／`CCD_SCAN`：另一側 `DoFeedTray` 會回 `false`、暫不開始取盤（Status 互鎖）。
2. 共用前置疊盤機被對側佔用 (`iFrontOwner` 互斥)。
3. Loader-Y 跨側安全距不足：對側持盤且目標位置與對側距離 < `iLoaderYSafeDistance`（預設 10000=100mm）時拒絕移動。
4. SortArm 佔用 Y 軸 (`iYOwner=SORTARM`)：`Task 3000` 時等待。
5. AMR 鎖 (`bAmrLocked`)：補料交握期間凍結前置疊盤／取盤。

這些等待為非阻塞設計（回 `false` 由上層輪詢重試），多數情況屬正常排程，不是故障。

### Q25. 出現「Loader Tray Empty」怎麼處理？

代表取盤上料時推盤氣缸感測器／模擬判定無盤（來料疊盤已空）。回復鍵：

- `K_RETRY`：重新取盤。
- `K_TRAY_END`：標該側盤盡 (`bTrayEmpty`)。
- `K_CLEAN_OUT`：進入 CleanOut 排空模式。

### Q26. Top CCD 相關警報（連線/讀料/2D）如何處理？

| 警報 | 意義 | 回復鍵 |
| --- | --- | --- |
| Top CCD Connect not ready | REALLY 模式且啟用 2D Bin Map 時連線未就緒 | RETRY 重試 / SKIP 標 2D 掃描失敗碼 |
| Top CCD API not ready | 讀該格有無料失敗 | RETRY 重讀 / SKIP 該格設 EMPTY / TRAY_END 整盤改 EMPTY 結束掃描 |
| 2D code not found in any lot : `<code>` | 讀到的 2D 在所有 Lot 反查不到 Bin | RETRY 重拍 / SKIP 標 NO_BIN_SETTING（導向 Error Auto） |
| Top CCD 2D no response | 3000ms 內未回傳 2D | RETRY 重拍 / SKIP 標 NO_BIN_SETTING |

### Q27. 排出空盤前提示「Loader Tray has IC, please remove」？

代表實機排出前底部感測器顯示盤上仍有 IC 殘留。回復鍵：`K_RETRY`（移除後重試）或 `K_SKIP`（略過繼續排出）。

---

## 16.8 SortArm（分類臂）與 TrayArm（盤臂）

### Q28. 為什麼分類臂 (SortArm) 突然不能左右移動並報警？

依規格，SortArm X 軸 (`MoveSortArmX`) 每拍都先過 `AreAllSuckersHome()`——任一啟用的 Suck-Z 吸嘴不在即時 Home 感應器位置就不准 X 移動，以免吸嘴下伸時橫移撞料。若移動中感應器掉失（失步／漏氣），會先每拍夾停，超過去抖窗 `SUCK_HOME_LOST_MS=100ms` 確認後 `StopAllMotor` 並彈出：

> 「SortArm move blocked : a suck nozzle left its Home sensor (lost steps). Re-home the suckers.」(K_RETRY)

處理：重新 Home 吸嘴後重試（此急停會連帶使 `SystemStart` 下降）。

### Q29. 為什麼盤臂 (TrayArm) 不能左右移動並報警？

依規格，TrayArm X 軸 (`MoveTrayArmX`) 每拍都先過 `IsZUpAtPosition()`——Z 升降氣缸須在上位 (`C_TrayArmZ_Up.IsOn()`) 才准橫移，以免頭／盤在低位時橫掃撞站。若移動中離開上位超過 `TRAYARM_ZUP_LOST_MS=100ms`，會 `StopAllMotor` 並彈出：

> 「TrayArm move blocked : the Z lift left its UP sensor. Check the TrayArmZ up cylinder / air pressure.」(K_RETRY)

處理：檢查 TrayArmZ 上位氣缸與氣壓後重試。

> ⚠️ 注意：上述兩臂防撞互鎖只在編譯期 SOFT_SIMULATE 版被略過；實機 DUMMY/HAS_TRAY/REALLY 三模式皆生效。請勿期待在 DUMMY 模式下繞過這些互鎖。

### Q30. 出現「... motor will out of limit」是什麼意思？

代表該軸目標位置超出軟體極限 (`CheckSoftLimit` 失敗)，系統以 `ShowMyMessage` 提示並「拒絕移動」。可能出現於 SortArm X、Loader Y、Auto Y、Pitch X、Tray Arm X、Top CCD X 等軸。請檢查教導位置／極限設定後重試。

### Q31. 分類臂吸取或放料時出現 ShowSuckError？

代表真空動作失敗——吸取 (Suck/Pick) 或放料 (Destroy/Place) 異常。回復鍵為 `K_RETRY | K_SKIP`，隨後系統會 `Sucker->Reset()`。吸嘴錯誤碼為動態格式 `6{吸嘴序}{子碼}`，子碼涵蓋取料失敗 (`eSuckPickErr`)、真空感測器關 (`eSuckVacOffErr`)、元件掉落 (`eSuckDropErr`) 等；描述提示 `[1] 檢查真空 [2] 檢查線路 [3] 檢查氣壓/氣管`，重複發生時訊息會加註 `(Again!)`。

---

## 16.9 警報視窗與蜂鳴器

### Q32. 警報視窗 (Note) 上的回復鍵 (SKIP/RETRY/...) 各代表什麼？

`TfNote` 依該警報的 `KeyCode` 位元顯示對應回復按鈕：

| 按鈕 | 位元 | 意義 |
| --- | --- | --- |
| HOME & RETRY | K_HOME (0x20) | 先回原點再重試 |
| SKIP | K_SKIP (0x01) | 跳過此料/動作 |
| RETRY | K_RETRY (0x02) | 重試造成警報的動作 |
| TRAY FEED | K_TRAY_FEED (0x04) | 補盤/送盤 |
| TRAY END | K_TRAY_END (0x08) | 盤子結束 |
| CLEAN OUT | K_CLEAN_OUT (0x10) | 清機/清空 |

只會顯示該警報實際提供的回復鍵；`KeyCode==0` 為純資訊提示，不提供回復鍵。

### Q33. 警報視窗按 `START`/`PAUSE` 都關不掉，怎麼回事？

這是刻意的安全閘控。依規格，若警報「有提供回復鍵」(`KeyCode!=0`) 但操作員「尚未選任何一個」回復鍵，則 `START`（恢復）與 `PAUSE`（停機）皆「不會關閉視窗」，避免未處理就帶過警報。請先選一個回復鍵（畫面觸控或實體面板鍵皆可），再按 `START` 或 `PAUSE`。只有純資訊提示 (`KeyCode==0`) 可直接關閉。

### Q34. 警報還沒解決，可以先讓蜂鳴器停下來嗎？

可以。按 `Off Buzzer`：會設定 `bOffBuzzer` 鎖存並 `CloseBuzzerOff()`，使每掃描的 `LED_ErrJam` 蜂鳴不再重新驅動；但「警報視窗仍開啟」，仍需處理警報。實體 `ALARM RESET` 鍵亦會呼叫 `CloseBuzzerOff()`。

### Q35. 為什麼安全門開／急停時只亮紅燈、不一定有蜂鳴聲？

依規格，`ErrJam` 蜂鳴只由 Note 警報觸發。純安全感測（EMG/門/氣壓/離子風）若當下沒有對應的警報對話框，只會亮紅燈而不發蜂鳴。

> 【待補：SwMusic1..4 各音樣式（旋律/長短）對應的實際聲音需現場聆聽確認；各運轉狀態的 Music Select 預設值由 Maintenance 的 tsMaintTowerLight 設定。】

### Q36. 警報代碼從哪裡查？

開機時 `CreateSystemAlarmCode()` 會依氣缸/馬達/吸嘴設定表「動態產生」整份警報代碼目錄，並輸出到 `CurrentDir\system\AlarmList.csv`（欄位：AlarmCode, AlarmType, E_ErrMessage, C_ErrMessage, E_Description, C_Description）。可開啟此 CSV 查閱機台所有可能警報。

> 【待補：`C_ErrMessage`/`C_Description` 在原始碼目前以英文填入（為保持 ASCII），實機是否已切換中文語系字串需現場確認 AlarmList.csv 內容。】

### Q37. 提示框（OK / 是否確認）跟全警報視窗有什麼不同？

兩者不同元件：

- 全警報 `TfNote`：會停機（`DecStopAllMotor` + 清 `SystemStart`）、面板紅色閃爍、發蜂鳴並提供回復鍵。
- 輕量提示 `TMyMessageBox`：`ShowMyOKMessage`（僅 OK，會停機）、`ShowMyOKMessageNoStop`（不停機，資訊用）、`ShowMyMessageBox_YES_NO`（是/否確認，不停機、不發蜂鳴）。

依專案規約，警報/確認一律使用 `TMyMessageBox` 或 `TfNote`，不使用原生 VCL 對話框。

---

## 16.10 狀態快照（Hang up）

### Q38. 機台異常時要怎麼留存現場資料給工程分析？

按 `sbStoreHangup`（Hang up/狀態快照）會手動觸發 State Record 快照 (`TriggerSnapshot`)，把 TaskHistory/MachineState/config 打包成 zip 供離線分析，並用檔案總管開啟該資料夾。發生不易重現的異常時，建議先做一次快照再排除。

---

## 本章待補項目 (unknowns)

> 下列項目須由設備工程師於實機逐項校對確認後補齊。

- 各使用者權限 (ROLE_OPERATION/SUPERVISOR/ENGINEER/HONPREC) 對應的可操作範圍（UserRoleManager 定義）。
- DUMMY/HAS_TRAY/REALLY 三模式對各模組 (Empty/Color/Loader) IO/動作的細部差異。
- Start Mode = Continue (`iStartMode=1`) 在啟動流程的實際差異行為（`CheckContinusStartIsReady()` 已停用，目前看不出分支差異）。
- Run_TrayFeed（補盤）模式：`CheckAllTrayFeedFinish` 為 stub，實機是否啟用待確認。
- SwMusic1..4 各音樣式對應的實際聲音，及各運轉狀態 Music Select 預設值。
- `C_ErrMessage`/`C_Description` 實機是否已切換中文語系字串（查 `system\AlarmList.csv` 內容）。
- AlarmType 列舉中 eFunErr/eSystemMess/eRecordProcess/eOther 的實際警報文字/觸發點（未定義分支僅印出 "Xxx Code Undefine Error"）。
