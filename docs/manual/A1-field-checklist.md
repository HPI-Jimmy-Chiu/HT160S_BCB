# 附錄 A　現場驗證清單（真實機驗證項）

> v0.2（2026-07-16）改版說明：原 119 項待補中，可由 repo 設定檔（IO_Table / Mot_Table / AlarmList）、AGV/SECS 草案文件、byte-safe DFM 讀取與原始碼定性解決者，已全數補入各章正文（標「註（定案）」）；本清單只保留**真正需要實機/現場**才能驗證的項目，依驗證性質分組，供一次現場 session 逐項勾稽。

## A. 安全（發行前硬阻擋——未完成不得發行）

- [ ] **實體警告標籤拍照建檔**：機台外殼各警告標籤（高壓、夾傷、移動部位、雷射/CCD）的圖示、位置與文字，逐一拍照並補入第 1 章 1.2。
- [ ] **塔燈/蜂鳴器現場設定記錄**：程式預設值已寫入第 1 章 1.3（Running=綠、Error/Jam=紅閃、Pause=黃、Message/Homing=黃閃、Music 全 Mute）；現場請記錄**該機實際設定**（Maintenance → Tower Light 逐格）並確認出貨設定值，回填第 1 章「以現場設定為準」處。

## B. 機構方向／對應（逐軸低速驗證）

- [ ] **JOG +/− 實際物理方向表**：Teach 畫面逐軸低速 JOG，記錄 `JOG +`（CW）對應的實際方向（右/左、前/後、上/下），做成 18 軸對照表補入第 7 章 7.5。
- [ ] **吸嘴 1~4 機構排列方向**：確認 `MSuckZ_1~4`（Nozzle1~4）在 SortArm 上的實體排列順序與 Offset 欄位 Z1..Z4 的對應（第 8 章 8.2 註）。
- [ ] **拆堆氣缸物理對應**：Loader/Empty/Color 的 `DoFrontDestackDown` 各 RiseTray/SeparateTray 氣缸實際哪顆抬升、哪顆分離（位址見附錄 B；動作方向現場目視確認），補入第 14 章。
- [ ] **TrayMotor->fHasTray 夾緊時序**：`fHasTray` 作為「盤已夾緊」代理（跨側安全距離互鎖用），實機確認與夾盤完成時序一致。

## C. 感測器接線／啟用狀態（以機台為準）

- [ ] **Auto car-taken 感測**：`SnAutoX_InputEnd`（Lane0/IP2/Port1/Bit0~5）是否最終正式接線——未接線時 AMR 握手停在 Ready 不會 Finish（第 12/14 章）。
- [ ] **Loader 進出料感測**：`SnLoader_Inputend`／`SnLoader_OutputBottomHasTray` 的啟用狀態與實際接線核對（位址見附錄 B）。
- [ ] **設定檔 drift 核對**：附錄 B/C 由 repo 工作副本產生；最終以機台 State Record 內 `MachineConfig\system` 副本逐檔核對 IO_Table / Mot_Table（IP/Lane 已知會 drift）。

## D. 動作時序量測（ticks → 實際毫秒）

- [ ] **主迴圈實際週期**：`TRunControl` 標稱約 1ms/拍；實測一次以校準以下換算。
- [ ] **TrayArm `ArmDelay.Set(3)`**：3 拍實際毫秒數（標稱約 3ms）。
- [ ] **Empty/Color `HTimer .Set(5)` 與 `DoClampTray SettleTicks=5`**：5 拍實際毫秒數。

## E. 警報／聲音（實機聆聽與抽驗）

- [ ] **SwMusic1..4 音樣式**：於 Maintenance → Tower Light 的 Music Test 逐一試聽，記錄各編號旋律/長短特徵（第 13/16 章）。
- [ ] **機構面板閃爍抽驗**：逐類警報（氣缸/馬達/吸嘴/JAM）各觸發一次，確認 `FlushPanelName` 對應的機構面板正確閃爍（找不到時退回 `pn_System`）。
- [ ] **伺服警報清除動線**：實測「SERVO OFF→ON 無法清除鎖存警報、`SwMotorRelay` 電源重置可清除」，把完整操作步驟補入第 11 章 11.7。
- [ ] **Motor Test LED 顏色語意抽驗**：Motor Test 畫面未套用 Teach 三色慣例，逐顆確認實際顏色表現（第 11 章 11.2）。

## F. SECS/AMR 對接（與客戶/整合者確認）

- [ ] **SVID/ECID/CEID 對外名稱比對**：以 `docs/SECS/HT160S_SECS_Comm_Examples.md` 為介面合約，與客戶主機整合者文件逐號比對（含 HT160-only 延伸段 38237-38245）。註：2026-08-04 起 66000 段 10 個 SVID 全部下架，只公佈 HT-90XX 家族號（第 12 章）。
- [ ] **主機端報告定義重建**：確認主機開機後會重下 S2F33 + S2F35——連結不持久化，退回的 Report 1 只帶時戳 `{1027}`；並確認主機可處理 SVID 1006 多批逗號串接值（第 12 章）。
- [ ] **S7Fx recipe 傳輸支援範圍**：與客戶確認是否使用；程式僅確認分派函式存在。
- [ ] **SECS/GEM 選配啟用**：各客戶現場確認 `bUseSecsGem`／`[SECS] Enable` 是否開啟（付費選配）。
- [ ] **AMR 端到端演練**：CEID272 叫車 → START_AGV → 273 Ready → 交車 → 274 Finish（含 Report 6 計數核對）、CEID275 身分盤 2D 上傳，實機 host round-trip 驗證一輪。

> 完成勾稽後：更新各章對應段落、移除殘留待補標記、封面版本升版並記入修訂紀錄。
