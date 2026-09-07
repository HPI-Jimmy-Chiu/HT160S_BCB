# 手冊完整性審查意見（workflow 自動產生）

## 總評

整體而言，此手冊的「章節骨架」對一台 tray 分類/搬運機而言相當完整：安全、系統概觀、操作面板/開機、主畫面、維護、設定、教導/偏移/速度、I/O 監看、馬達測試、SECS/AMR、警報排除、模組流程、By Lot+Bin、FAQ 共 16 章，已涵蓋操作員與維護工程兩個面向，結構對齊參考的 HT9045/9046 手冊。骨架沒有重大主題缺漏。

真正的問題不在「缺章節」，而在「待補項目的性質」與「分類錯誤」：
1. 待補總量過大（光本批列出就 100+ 項），且高度集中在三類來源——(a) Big5 中文 UI 標籤未讀出、(b) 實體 IO 點位/中文標籤、(c) 截圖未產生。這三類其實大多「可由 repo 內既有檔案補齊」，不必全部排現場，目前卻被一律標成「需現場確認」，會誇大現場工作量。
2. 多項「待補」標示為來源未讀，但檔案其實就在 repo 內、可立即讀取補上：docs/AGV/HT160S_E87_AGV_Communication_Draft_20260527.md（第12章 SECS 報告/P-mapping，已確認存在且含 P1-P9 站點對應表）、system/AlarmList.csv（第13/16章警報中英文）、system/Mot_Table.csv（第9/11章軸名 Alias、HomeType）、system/IO_Table.csv（第2/10/14章 IO 點位與中文標籤）。這些應在標「現場確認」前先用 repo 檔案補完。
3. 已存在一份 docs/AGV/HT160S_E87_AGV_Operation_Manual.md（AGV 操作手冊），第12章與第15章應與它交叉對照、避免規格不一致或重複造輪子。
4. 安全章（第1章）的待補項（警告標籤、塔燈/蜂鳴器組合、tower-light.png）屬於發行前的「硬阻擋」項目——安全內容未定不應交付；應列為最高優先並要求現場拍照+實機確認。
5. 真正只能靠現場/實機驗證的，是「動作時序的實際秒數（ticks→ms）」「氣缸物理對應/方向」「感測器是否最終接線（如 Auto car-taken、Loader CCD 有無料）」「伺服警報清除是否需 power-cycle」這類動態行為——這些才是值得集中安排一次現場驗證 session 的核心，建議獨立成一份「現場驗證清單」附在手冊末。

建議的補完優先序：先用 repo 既有 CSV/草案文件批次補完可查項（降低待補數約一半），再產生截圖，最後排一次現場 session 專攻動態時序/接線/安全標籤。

## 各項建議

### 通用 / 跨章
**問題**：大量『待補』標為需現場確認，但其來源檔案其實就在 repo 內可直接讀取（system/IO_Table.csv、system/Mot_Table.csv、system/AlarmList.csv、docs/AGV/*）。目前一律標現場，會誇大現場工作量並延後可立即補完的內容。

**建議**：在排現場前，先以 repo 既有檔案批次補完：IO 點位/中文標籤查 system/IO_Table.csv；軸名 Alias 與 HomeType 查 system/Mot_Table.csv；警報中英文查 system/AlarmList.csv；SECS 報告/P-mapping 查 docs/AGV/HT160S_E87_AGV_Communication_Draft_20260527.md。只把這些檔案也查不到的，才保留『現場確認』標記。

### 通用 / 跨章
**問題**：『Big5 中文 UI 標籤未直接讀出，UI 文字以程式英文 Caption 為準』在第02/03/04/05/06/14章重複出現多次，被當成各自獨立的待補。

**建議**：用 byte-safe 方式（記憶體已記載 scripts/ops/bcb6-bytesafe-edit.ps1 / Latin1 splice）一次性把各 .dfm 的 Caption/Hint 以 Big5 解碼匯出成對照表，集中補完所有畫面標籤，避免逐章重列同一個 root cause。

### 第01章 安全須知
**問題**：警告標籤（高壓/夾傷/移動部位/雷射CCD）的圖示位置文字、各狀態三色塔燈組合+蜂鳴器音樂編號、tower-light.png 截圖三項全待補。安全章是發行前硬阻擋，未定不應交付。

**建議**：列為最高優先：(a) 現場逐一拍攝機台實體警告標籤並標位置；(b) 在 Maintenance→Tower Light 分頁逐狀態記錄綠/黃/紅 ON/OFF 與 Music 編號（記憶體 towerlight-buzzer 已說明由 DoSystemMessage 套用，可先讀 DFM 預設值再實機確認）；(c) 產生 tower-light.png。安全內容未定前不得標『可發行』。

### 第12章 SECS/GEM 與 AMR/AGV
**問題**：待補 12 項中多項（CEID272/273/274 與 SVID 38202-38245 對外名稱、P-mapping、完整報告定義）標為『docs/AGV/...Draft 未讀取』，但該檔已存在且含 P1-P9 站點表；另已存在 HT160S_E87_AGV_Operation_Manual.md 未被交叉對照。

**建議**：先讀 docs/AGV/HT160S_E87_AGV_Communication_Draft_20260527.md 補完 P-mapping/CEID/SVID 對應與報告格式，並與 docs/AGV/HT160S_E87_AGV_Operation_Manual.md 對齊避免規格衝突/重複。真正待現場的縮減為：car-taken 感測器（SnAutoX_InputEnd）是否最終接線、DeviceCount/IC count payload 設計、交車途中改模式行為、各客戶是否啟用 SECS 選配——這幾項才標現場。

### 第13章 警報訊息與排除
**問題**：C_ErrMessage/C_Description 是否已中文化、AlarmType 各列舉（eFunErr/eSystemMess/eRecordProcess/eOther）的實際警報文字與觸發點皆待補，但 system/AlarmList.csv 已存在於 repo。

**建議**：先讀 system/AlarmList.csv 補完警報碼↔文字（並判定目前是中文或英文），再從 ShowSystemError/ShowMotorError 呼叫端反查各 AlarmType 的觸發點補上『可能原因/排除步驟』表。SwMusic1..4 實際聲音與各狀態 Music 預設值才需實機聆聽確認。警報排除章是操作手冊核心，建議補成『代碼/訊息/原因/處置』四欄表。

### 第09章 速度 / 第11章 馬達測試
**問題**：實際軸名清單（Alias）、HomeType/回原方式被標為需現場確認，但 system/Mot_Table.csv 已存在且為這些值的設定來源。

**建議**：直接讀 system/Mot_Table.csv 列出全部 enabled 軸的 Alias 與 Direction/HomeType 補進第9/11章軸表；伺服警報是否需 power-cycle 已有記憶體記載（SetServoOn 為 no-op、需 SwMotorRelay Off→On）可直接寫入，僅『本畫面 Servo On/Off 是否等同 power-cycle』保留實機確認。

### 第02章 / 第10章 / 第14章
**問題**：各模組氣缸/感測器的實際 IO 點位與中文標籤在多章重複待補（Color/Empty/Loader/SortArm/TrayArm/Auto 的 C_*/Sn* 識別字），但 system/IO_Table.csv 已存在。注意記憶體記載 repo 工作副本與機台 system 副本會 drift（IP/Lane 不同）。

**建議**：以 system/IO_Table.csv 補完各 C_*/Sn* 對應的位址與中文標籤，產生一張全機 IO 對照表供第2/10/14章共用。為避免 drift，最終位址應以機台 State Record 內 MachineConfig\system 副本核對（記憶體 io-table-compare-use-staterecord 已記載此原則）。第10章圖例 M/<->/v^/V/D 字義也應由此 IO 表中文 Hint 一併補正。

### 第14章 各模組運作流程
**問題**：29 項待補中，動作時序的固定延遲單位（HTimer .Set(3)/(5)、ArmDelay、DoClampTray SettleTicks 對應實際 ms）反覆出現，且取決於主迴圈週期（約1ms）；屬動態行為，無法由靜態原始碼定值。

**建議**：將所有『ticks→實際秒數』『氣缸物理對應/方向』『感測器最終接線狀態』集中成一份『現場/實機動態驗證清單』，安排一次現場 session 一併量測（可用 State Record FeederDecision.txt 輔助，記憶體已記載其 DescribeState 可 dump 各模組內態）。靜態可定者（如 CEID {136..142} 跳過139 的原因）改查 SECS 事件表/草案文件而非標現場。

### 第06章 設定 / 第08章 偏移
**問題**：工程單位（TrayForm.XStart 等、Offset 56 欄位）是 mm 或 1/100mm 標為無法由設定畫面判定。但記憶體已明確記載本機 teach/encoder 位置一律 1/100mm（100 units/mm）。

**建議**：依記憶體 ht160-position-units-1-100mm 的既定結論補上單位說明，並交叉確認 TrayForm 結構在 Loader/SortArm 使用端的換算（mm-facing 設定 ×100/÷100）。Offset 各欄精確機構意義可先用 GetOffsetExplain 補概述，精確吸嘴/疊層對應再現場確認。第08章 DFM(640x480) 與 BuildUI(720x640) 尺寸不一致應直接在原始碼確認何者生效，不需排現場。

### 第03章 / 第16章 (Start Mode 行為)
**問題**：iStartMode=Continue 的實際差異行為、CheckContinusStartIsReady() 已停用導致 Initial/Continue 看不出分支，在第3章與第16章重複待補且結論模糊。

**建議**：直接在 Start() 與 SECS 寫值路徑追 iStartMode 的所有使用點，確認 Continue 是否僅供記錄/SECS SV 鏡像而對動作無分支（若是即可定稿，不需現場）。第3、16章應給一致結論，避免兩章各寫一半。

### 功能/按鈕啟用狀態 (第03/04/10章)
**問題**：多個按鈕標為『DFM 未綁定 OnClick、無處理函式，功能/是否啟用待現場確認』（btnClearCount/sbPaperSummary、IO 右側選單 SpeedButton、Pad/Terminal/Enable IO Change 等）。這些是靜態事實，不需現場即可定性。

**建議**：由原始碼直接定性：未綁定 OnClick 且無 handler = 目前『無作用/保留鈕』，可在手冊明確標註『此按鈕目前未啟用（保留）』而非『待現場確認』。僅當需判斷是否規劃啟用時才詢問負責工程師，而非排機台現場。

### 缺漏主題 — 現場驗證清單 / 例行保養 / 規格表
**問題**：目前 16 章未見三項操作手冊常備內容：(1) 集中的『現場/實機驗證清單』（把全書散落的待現場項收斂成可勾選清單）；(2) 例行保養/點檢與消耗品（氣壓、清潔、感測器校正週期）；(3) 機台規格表（電源/氣壓/尺寸/節拍/盤種 identity/cover/normal 與 MAX_TRAY_PER_CAR 數值，後者記憶體指出定義於 cmydef.h/MyMotor.h）。

**建議**：新增『附錄A：現場驗證清單』彙整所有待現場項目（拍標籤、塔燈、時序秒數、接線狀態、伺服清除）；新增『例行保養/點檢』章（即使內容少也要佔位）；在第02章補一張規格表，盤種與 MAX_TRAY_PER_CAR 數值直接讀 cmydef.h/MyMotor.h 定值，不標現場。

