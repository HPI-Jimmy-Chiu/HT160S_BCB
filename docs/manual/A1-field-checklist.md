# 附錄 A　現場驗證清單（待補項目彙整）

> 合計 119 項待補。

本清單彙整全手冊各章標註的「待補/待現場確認」項目。其中與 IO 點位、軸參數、警報碼相關者，多數已可由附錄 B/C/D（自機台設定檔產生）對照補齊；真正需實機驗證的，集中於：實體警告標籤拍照、塔燈/蜂鳴器逐狀態組合、動作時序的實際秒數（ticks->ms）、氣缸物理對應/方向、感測器是否最終接線、伺服警報是否需 power-cycle。請設備工程師逐項勾稽。

## 第 01 章　安全須知
- [ ] 機台外殼上各警告標籤（高壓、夾傷、移動部位、雷射/CCD 等）的圖示、位置與文字內容（SPEC 未涵蓋實體標籤，需現場拍照補齊）
- [ ] 各運轉/故障狀態對應的確切三色塔燈燈色組合（綠/黃/紅 ON/OFF）與蜂鳴器音樂編號預設值（需於 Maintenance → Tower Light 分頁與實機確認）
- [ ] screenshots/tower-light.png 截圖檔尚未產生（目前為插入版位）

## 第 02 章　系統概觀與機構
- [ ] 主畫面 mtWorkArea/mtSortRecv（4x5 盤面格點）與 mtLoaderLTrayWork/mtLoaderRTrayWork（移動盤面）的對應關係，以及哪個顯示生產中盤面、哪個顯示移動盤面，需與 main.cpp 顯示綁定一併確認
- [ ] 「Loader 2D Left/Right」文字標籤與物理 Loader1/Loader2（左/右）的程式綁定（LoaderNo 與左右標籤對應）未明示，需現場確認
- [ ] 主畫面頂部功能列與監看選單多採點陣圖示（DFM Hint 多統一為 Change Language），實際螢幕圖示/標籤文字需以現場截圖確認
- [ ] 堆疊順序 identity/cover/normal 的盤種定義（eTrayKindIdentity/Cover/Normal）與 MAX_TRAY_PER_CAR 實際數值定義在外部標頭，需另行對照
- [ ] 各模組氣缸/感測器的實際 IO 點位與中文標籤須對照 IO_Table / 機構表確認（本章僅引用識別字）

## 第 03 章　操作面板與開機啟動
- [ ] 頂部功能列與 MonitorView 選單多採點陣圖示且 DFM 內 Hint 統一為 Change Language，確切螢幕圖示文字未在 DFM 文字屬性中，待現場截圖確認
- [ ] iStartMode=Continue 在 Start() 流程中的實際差異行為未明；CheckContinusStartIsReady() 已被註解停用，Initial 與 Continue 在啟動路徑看不出分支差異，語意是否僅供記錄/SECS 待確認
- [ ] DUMMY / HAS_TRAY / REALLY 三模式對機台實際 IO/動作的差異，本章僅見 REALLY 啟用離子風扇檢查，其餘差異需查各模組與三層 IO 檢查文件
- [ ] SECS 徽章狀態碼 (0/1/2) 對應實際 HSMS 連線語意 (OFF/CONNECT/ONLINE) 來源在 SECS 引擎，本章僅做顯示對應
- [ ] btnClearCount (Clear All) 與 sbPaperSummary (Summary) 在 main.dfm 未綁定 OnClick、main.h 無對應處理函式，實際功能/是否啟用待現場確認
- [ ] cbbUserSelect 選項與 pnStartMode / pnRealDummy 的實際螢幕中文標籤因 Big5 編碼未直接讀出，UI 文字以程式英文 Caption 為準

## 第 04 章　主畫面詳解
- [ ] 頂部功能列各速度按鈕（Language / Maintance / Offset / Speed / Tools / Message / Monitor / Exit）多採點陣圖示，DFM 內 Hint 統一為 Change Language，標籤名稱依處理函式語意推得，確切圖示文字未在 DFM 文字屬性中，待現場截圖確認
- [ ] btnClearCount（Clear All）與 sbPaperSummary（Summary）在 main.dfm 中未綁定 OnClick、main.h 亦無對應處理函式宣告，實際功能／是否啟用待現場確認（疑為待接線或保留按鈕）
- [ ] Fail 計數標籤 lblloseCnt 的 DFM Visible=False，目前隱藏不顯示
- [ ] palMainStatus_En（英文狀態面板）DFM Visible=False，目前隱藏不顯示
- [ ] SECS 徽章狀態碼（0/1/2）對應的實際 HSMS 連線語意（OFF/CONNECT/ONLINE）來源在 SECS 引擎，本畫面僅做顯示對應
- [ ] Tray Status 分頁（grpLoaderR/grpLoaderL、mtSortRecv/mtWorkArea）大量 TMyTray/Panel 排版屬視覺化盤位呈現，細部對應機構需另行對照 motion-view 文件
- [ ] spbTrayStatus / apbLogs / sbTimeData / btnTrayMap 速度按鈕在 DFM 內 Hint 皆為 Change Language，實際按鈕圖示／文字待現場螢幕截圖確認（OnClick 切換分頁的行為已確認）

## 第 05 章　維護畫面 (Maintenance)
- [ ] RadioGroup6 對應狀態列在記憶體 enum 為 LED_Heating，DFM 列標題為 "Reserved" 且 Visible=False；Heating/Reserved 之實際語意需現場確認
- [ ] Heating（記憶體 LED_Heating）與畫面 "Reserved" 標籤之間的對應與實際語意，需現場確認
- [ ] cbCommType（Option 分頁 "CommType" 勾選框）在 DFM 中存在，但 LoadHardwareSettings/SaveHardwareSettings 與其他 cpp 程式均未引用，實際作用無法從原始碼確認
- [ ] Function Define 分頁的 G[General]/N[Network] 子頁內容為空（Panel4 設 Visible=False）；tsMaintPassword 分頁在 DFM 中為空白頁，實際內容/功能未在本檔出現
- [ ] cbbMCUColor（Color 下拉 GREEN/RED）在送出按鈕程式中未被讀取（Send 動作改用 edMCULightValue 的 Color Code），此下拉是否仍生效需現場確認
- [ ] chkMCUCodeSymbol（"Symbol Code"）勾選框 DFM 有定義，但 btnMCUSend* 程式未引用，作用需現場確認
- [ ] btnTopCcdShot 的 DFM Caption 螢幕文字未逐字讀取（由 .h/.cpp 確認為拍照觸發鈕），確切螢幕文字以現場為準
- [ ] btnColorCcdShot 的 DFM Caption 螢幕文字未逐字讀取（由 .h/.cpp 確認為拍照觸發鈕），確切螢幕文字以現場為準

## 第 06 章　設定 (Config / Setup)
- [ ] TrayForm.XStart/XPitch/YStart/YPitch 的工程單位 (mm 或 1/100mm) 無法由設定畫面原始碼判定 (只做 ReadFloat/WriteFloat)，需確認 TrayForm 結構與 Loader/SortArm 使用端
- [ ] 各 Area (Auto1..Auto6, Color) 對應實體出料站/料盒的物理位置與 enum 編號間隙 (eHT160BinAreaAuto1=3..Auto6=8, Color=9，1/2 用途) 需由 CosFunction.h/機構定義確認
- [ ] 硬體頁 checkbox 與相關控制項 (chkUseLotBinMode/chkAutoEnable*/chkSuckEnable*/chkHardwareColorBinArea/chkUseAMR/cbBinPanelType/edLotNo) 實際螢幕中文 Caption 需以 .dfm 或執行畫面確認
- [ ] SYSTEM_BIN_SELECT BinSelect[2] (iCategData/bStackDefFailCate/bCategoryFail/iCategoryFailCountLimit/iOpenBin) 在 cprod 內僅宣告未見讀寫，填值來源與是否仍在使用需確認
- [ ] Error Auto 決定 (GetErrorBinArea/GetErrorAutoIndex) 在 Error 區被設為 Color (非 Auto) 時 By Lot+Bin fallback 到最後一個 Auto 的邊界行為實機意圖需確認
- [ ] By Lot+Bin 是否提供操作員手動指定/編輯特定 (Lot,Bin)->Auto 綁定的 UI 未在現有來源確認
- [ ] 是否存在獨立於 Product Setup 之外、供新增/複製/刪除/切換配方的另一配方選單畫面 (form 名稱與按鈕) 未完全確認

## 第 07 章　教導 (Teach)
- [ ] ledStatus 標題列（CW/HOME/CCW/EMG/ALARM/Soft CW/Soft CCW/Servo Alarm/In Pos/Z Phase/Servo On）與 Motor->Led[] 各位元的逐一對應由 ScanMotorStatus/驅動層決定；LED 索引常數（iCwLed/iCcwLed/iEmgLed/iAlarmLed/iServoalarmLed 等）定義於 database.h/MachineType，本畫面僅引用，未於本檔展開驗證。
- [ ] JOG +=CW、JOG -=CCW 對應的實際物理方向（右/左、前/後、上/下）依各軸機構與驅動 Direction 設定而定；教導採『+/right、-/left』語意，但實際對應因軸而異，需現場確認。
- [ ] tech.ini 完整節段名（GroupName，如 TeachEmptyAndTrayX/TeachLoader/TeachAuto）與所有鍵名、ed_/edt_ 前綴的回退相容性，需現場核對既有檔案實況。
- [ ] Advanced 分頁 SortArm cell 幾何（Column/Row→實際盤格座標）與 TestGoUpTray/TestGoDownTray/TestGoUpOnce 的內部缸序與感測，屬各模組（aSortArm/aEmpty/aColor/aLoader/aAuto1To6）實作，未於本檔展開。
- [ ] 部分 DFM 標籤雖為英文且可讀，實機若顯示不同（中文化）需現場確認。

## 第 08 章　偏移 (Offset)
- [ ] 畫面標籤與按鈕文字在原始碼中皆為英文 ASCII (Offset / Apply / Save / Re-alignment / Exit / Clear All 等)，機台實機上是否另有中文化字串無法由原始碼判定。
- [ ] 56 個欄位的精確物理意義（各站點 Z1..Z4 對應的吸嘴 / 疊高層級等）僅能由命名與 GetOffsetExplain 概略說明推斷，精確機構對應需現場確認。
- [ ] QwertyKey 軟鍵盤 (fQwertyKey->ShowQwertyKey) 各參數語意 (N_DOUBLE、小數位 2、range 啟用旗標) 定義於 uQwertyKey，未在本畫面原始檔內，僅依呼叫推斷。
- [ ] 原始 DFM (ClientWidth=640 / Height=480) 與 BuildUI() 程式設定 (720×640) 不一致；實際顯示尺寸以執行時 BuildUI 為準，此差異是否刻意需確認。
- [ ] 畫面是否有權限 / 模式（例如維修模式）限制存取，需由開啟它的 main.cpp 上層 (ShowTopForm(fOffset, sbOffset)) 確認。
- [ ] RecipeManager.GetCurrentRecipeName() 與工作檔(recipe)切換的完整流程定義於 RecipeManager / main.cpp，本畫面僅消費其回傳檔名。

## 第 09 章　速度 (Speed)
- [ ] 進入本畫面的實際操作路徑（由哪個畫面／按鈕開啟 Speed Setup）需現場確認後補上。
- [ ] 實際出現的軸名清單：原始碼對所有 MotPtr 非 NULL 的馬達一律建列，無針對特定軸/條件篩選；確切軸名 (Alias) 由機台 Mot_Table 設定決定，需以現場機台確認。
- [ ] accel/decel 可調設定的實際位置（可能在 Teach 或馬達參數設定處）需另行確認；本畫面原始碼確認不提供 acc/dec 編輯欄位。

## 第 10 章　輸出入監看 (I/O)
- [ ] 右側選單 SpeedButton（Load/TOOL/DATABASE/MAP/Sucker/Unloader/SYSTEM/PANEL）在 .dfm 未綁定 OnClick，SelectLegacyIOPageByButton() 定義卻無呼叫處；實際換頁靠直接點 PageIO 分頁標籤，是否仍可用需現場確認
- [ ] sb_IO_CommunicationPad（Pad）按鈕 OnClick 在 .dfm 未綁定，sb_IO_CommunicationPadClick 是否被觸發（開啟 TfPadInterface）需現場確認
- [ ] spbTerminalProgram（Terminal Program）Visible=False 且 OnClick 為空實作；Terminal 功能是否仍使用未知
- [ ] sbEnableIOChang（Enable IO Change）與 Loop Output Interval 相關控制 OnClick 為空實作，實際用途/是否保留未知
- [ ] 多數分頁 TMyLed/TBtnPanel 文字標籤（v ^、<->、Front/Rear、M/V/D 圖例）為 Big5 中文 Hint 可能亂碼；M=馬達/<->前後汽缸/v^上下汽缸/V真空/D破真空 係推測，確切字義需現場確認
- [ ] 各分頁 LED/按鈕綁定的實際 Alias 逐點對照（Loader/Unloader/Sucker/Panel）未全部展開，需另行掃描 .dfm 的 Alias 欄
- [ ] IOMap 匯出檔（SaveInputMap1/SaveOutputMap1）實際輸出路徑/檔名由 SaveIOMap 決定，未確認

## 第 11 章　馬達測試 (Motor Test)
- [ ] ledStatus5/6 對應 Soft CW / Soft CCW，但 RefreshOperateGrid 與 ScanMotorStatus 是否實際驅動 Motor->Led[5]/Led[6]，原始碼未在本檔明確賦值，需確認來源。
- [ ] 本畫面各 LED 確切顏色語意需實機現場確認（Motor Test form 未套用 Teach 畫面的 green/gray/red 三色慣例）。
- [ ] HomeType 並非本畫面獨立欄位；各軸實際 HomeType / 回原方式（例如 type-7）定義於馬達驅動層，非本畫面顯示，需查 Mot_Table 與驅動文件確認。
- [ ] cbbLoopWait 的等待時間具體選項清單 (ms 值) 由 GetLoopWaitMS()/DFM Items 決定，本次未讀取確切數值。
- [ ] 伺服警報是否單靠 SERVO Off->On 即可清除，或部分需 SwMotorRelay 電源重置；本畫面 Servo On/Off 走 SwServerON，是否等同 power-cycle，需實機確認。
- [ ] DFM 內標籤確認：本畫面 Caption/Label 皆為 ASCII 英文，未見 Big5 中文標籤（依萃取結果）。

## 第 12 章　SECS/GEM 與 AMR/AGV
- [ ] 主機端實際使用的 SVID/ECID/CEID 對外名稱對應表（9045 對齊 band 與 HT160 自訂高位 band 是否與客戶主機一致）需與整合者文件比對
- [ ] 完整 GEM 通訊/控制狀態機（E30 COMMUNICATING/ONLINE/OFFLINE、LOCAL/REMOTE）是否實作；本程式僅以 SV 66002 鏡像值（4=Local/5=Remote）表示
- [ ] S5F1/S5F6 警報訊息與 S7Fx recipe 傳輸的完整支援範圍（僅確認分派函式存在）
- [ ] CEID272/273/274 與各 SVID（38202-38245）的對外名稱/中文標籤；完整 SECS 報告定義在 docs/AGV/HT160S_E87_AGV_Communication_Draft_20260527.md（未讀取）
- [ ] DeviceCount（裝置數/IC 數）目前固定為 0；per-tray IC count / AMR 上傳 payload 尚未設計
- [ ] 實機 car-taken 感測器點（SnAutoX_InputEnd）是否為最終正式接線，及 device id/count 等硬體相依規格，需現場確認
- [ ] BinSetting[Auto]（SVID 38234-38245）的寫入來源/格式未確認
- [ ] ShortageDebounce[]、ReadyEntrySensor[]、PrepDone[] 已宣告但本模組未見實際使用邏輯，可能保留或他處使用，需確認
- [ ] Empty 模組（P2）的 AMR 介面方法（IsInputShortageForAmr 等）未逐一讀取確認，假設與 Loader/Color 同型
- [ ] AGVSupplement 觸發要求 RunMode==Run_Normal，但握手未檢查 RunMode；交車途中模式改變的行為未明確界定
- [ ] 各客戶現場是否啟用 SECS/GEM 選配（bUseSecsGem / [SECS] Enable）需現場確認
- [ ] 原始碼中文標籤/註解為 Big5；若現場 UI 另有中文字串，實際畫面文字以機台顯示為準

## 第 13 章　警報訊息與排除
- [ ] C_ErrMessage / C_Description 目前以英文填入（原始碼註解說明為保持 ASCII），實機是否已切換中文語系字串需現場確認 system\AlarmList.csv 內容
- [ ] AlarmType 列舉中 eFunErr(2)、eSystemMess(3)、eRecordProcess(7)、eOther(8) 的實際警報文字/觸發點未在本批檔案內定義；ShowSystemError 未定義分支僅以類型印出 Xxx Code Undefine Error，個別案例需現場確認
- [ ] SwMusic1..4 各音樣式（旋律/長短）對應的實際聲音，需現場聆聽確認；tsMaintTowerLight 各狀態的 Music Select 預設值由 Maintenance 畫面設定，未在本批檔案內固定
- [ ] FlushPanelName 對應到動態建立面板的命名是否與所有 ShowXxxError 呼叫端一致（找不到時退回 pn_System），個別機構面板閃爍正確性需實機確認
- [ ] note.dfm 中文標籤（如 Memo 區與機構示意說明）以 Big5 顯示，部分中文字串未逐字確認

## 第 14 章　各模組運作流程
- [ ] Loader: lblLoadCurrBin_1 (Current Sorting Bin)、lblLoadCurrID_1 / lblLoaderCarID (Loader ID) 三個顯示標籤在 aLoader.cpp 內未被寫入；實際更新來源 (推測 SortArm/分類流程或 main 顯示綁定) 需查 aSortArm/main 確認
- [ ] Loader: main 上 mtWorkArea/mtSortRecv (4x5) 與 mtLoaderLTrayWork/mtLoaderRTrayWork 的對應 (哪個顯示生產盤面、哪個顯示移動盤面) 需與 main.cpp 顯示綁定一併確認
- [ ] Loader: Loader 2D Left/Right 與物理 Loader1/Loader2 (左/右) 的對應由 DFM 版面推得，程式未明示 LoaderNo 與左右文字標籤的綁定，需現場確認
- [ ] Loader: ReadTopCcdBin 在實機 (UseCCD 開啟、非模擬) 直接 bOk=false 回 EMPTY_IC，未見實際向 CCD 取有無料的實作；實機是否另有 Bin 讀取來源或恆走 2D 路徑需確認
- [ ] Loader: SnLoader_Inputend / SnLoader_OutputBottomHasTray 的實際 IO 點位與啟用狀態需依機台 IO_Table 現場確認
- [ ] Loader: TrayMotor->fHasTray 作為『盤已夾緊』代理 (用於安全距離互鎖)，實機與夾盤完成時序是否一致需現場驗證
- [ ] Loader/Empty/Color: DoFrontDestackDown 各 RiseTray/SeparateTray 氣缸的物理對應 (哪顆抬升/分離) 與固定延遲單位 (cycle 或 ms) 需現場確認
- [ ] Color: 氣缸實體位址 / 感測器點位編號 (C_Color_FrontRiseTray_1/_2、FrontSeparateTray_1、RearRiseTray、LeanOnTray、PushTray 與 SnColor_InputHasTray/InputFullTray/InputEnd/OutputBottomHasTray/TrayPos1) 需查 IO_Table/機構表核對
- [ ] Color: iSupplyThreshold (預設 100) 與 NotifyICPlaced/iICCount 在供盤主流程未見實際判斷使用，用途/是否為遺留或外部讀取需確認
- [ ] Color: SortBin 模式 (DoSortBin) 目前僅 case 1 直接 return true 為空殼；分 Bin 實際行為是否規劃中需確認
- [ ] Color: IsAcceptingIC() 固定回 false，是否為預留介面需確認
- [ ] Color: DoClampTray SettleTicks=0 表示 Color 不做 push-on-sensor 確認，是否日後改為 >0 需確認
- [ ] Color: ColorY/ColorX/ColorZ 三軸實際軸名映射 (MColorY、MTopCCDX_Color) 以外的 Z 軸是否存在於本模組控制範圍需確認
- [ ] Empty: 實體感測器 SnEmpty_InputHasTray / OutputBottomHasTray / InputEnd 的中文標籤與 IO 表實際位址需查 sensor 定義確認
- [ ] Empty: 氣缸 C_Empty_FrontRiseTray_1/_2、FrontSeparateTray_1、LeanOnTray、PushTray 的精確機構位置/方向與感測器配置 (OnSensor/OffSensor) 需查 cylinder 設定
- [ ] Empty/Color: HTimer .Set(5) 與 DoClampTray SettleTicks=5 的『5』對應實際時間 (ticks 換算 ms) 取決於主迴圈週期 (約 1ms)，需確認實際秒數
- [ ] Empty: bLotFinish 由何處設定/清除 (本模組僅讀取)，推測由外部批次結束流程設定，需查呼叫端
- [ ] Empty: TestGoUpTray/TestGoDownTray 的觸發按鈕位於 Teach 進階畫面 (uteach)，其按鈕標籤與配置需查該畫面確認
- [ ] Empty: iControlPanel/操作面板上是否有對應 Empty 模組的手動鍵未在本檔出現
- [ ] SortArm/TrayArm: 真空吸嘴與夾爪實體 IO 點位/位址 (C_TrayArm_FrontClamp、C_TrayArm_RearClamp、C_TrayArmZ_Up/Down、SortArmSuck) 需查 IO_Table/cylinder 設定確認
- [ ] SortArm/TrayArm: ArmDelay.Set(3) 的 3 拍對應的實際時間 (取決於 HTimer 計時基準/主迴圈週期) 未在本檔界定
- [ ] SortArm: iHomeLed 索引對應的實際 Home 感應器接點需對照馬達/IO 設定確認
- [ ] SortArm: Bin->Auto 路由表 (BinAreaMap / LotBinBinding) 的設定來源與規則屬其他模組，本檔僅查表
- [ ] Auto: 實機『車已被取走』感測 (IsAmrTaken) 重用 SnAutoX_InputEnd；sensor TBD/未接線時 Enable==false 會停在 Ready，實際是否已接線需現場確認
- [ ] Auto: 堆疊順序 identity->cover->normal 的盤種定義 (eTrayKindIdentity/Cover/Normal) 與 MAX_TRAY_PER_CAR 實際數值定義在外部標頭 (cmydef.h/MyMotor.h)，未在本檔讀取確認
- [ ] Auto: DoDischargeTray 的 CEID 陣列 {136,137,138,140,141,142} 跳過 139，原因 (139 另有用途?) 需查 SECS 事件表確認
- [ ] Auto: Bin->Auto 反查顯示用的 BinAreaMap.GetBinByArea / LotBinBinding 對應表內容由設定檔決定，本模組不持有
- [ ] Auto: maintenance 畫面 chkAutoEnable1~6 的實際螢幕中文 caption 未從原始碼判讀 (Big5)，需現場確認
- [ ] Auto: tRunData.TrayICCnt[eAuto1+i] 計數於何處累加 (SortArm 放 IC 時) 未在本檔，主畫面僅讀取顯示

## 第 15 章　By Lot+Bin 分流模式
- [ ] 是否提供操作員手動指定/編輯特定 (Lot,Bin)->Auto 綁定的 UI；ResolveAuto 採最低 index 先到先得自動綁定，本批檔案未見手動編輯介面，需確認
- [ ] 當 Error 區設為 Color（非 Auto）時，By Lot+Bin 會 fallback 到最後一個 Auto 的邊界行為與實機意圖，需現場確認
- [ ] 配方選單畫面（若有獨立 form 供新增/複製/刪除/切換配方）對應的表單名稱與按鈕，本批檔案未確認（與第 6 章交叉）

## 第 16 章　常見問題 (FAQ)
- [ ] 各使用者權限 (ROLE_OPERATION/SUPERVISOR/ENGINEER/HONPREC) 對應的可操作範圍（UserRoleManager 定義）
- [ ] DUMMY/HAS_TRAY/REALLY 三模式對各模組 (Empty/Color/Loader) IO/動作的細部差異
- [ ] Start Mode = Continue (iStartMode=1) 在啟動流程的實際差異行為（CheckContinusStartIsReady() 已停用，看不出分支差異）
- [ ] Run_TrayFeed（補盤）模式 CheckAllTrayFeedFinish 為 stub，實機是否啟用待確認
- [ ] SwMusic1..4 各音樣式對應的實際聲音，及各運轉狀態 Music Select 預設值
- [ ] C_ErrMessage/C_Description 實機是否已切換中文語系字串（查 system\AlarmList.csv 內容）
- [ ] AlarmType 列舉中 eFunErr/eSystemMess/eRecordProcess/eOther 的實際警報文字與觸發點（未定義分支僅印出 Xxx Code Undefine Error）

