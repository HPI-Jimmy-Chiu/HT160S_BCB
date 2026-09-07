# HOME 續產 — 全機相位盤點（批量期 v1，2026-07-11）

來源：44-agent workflow（每表一產表員 + 一敵意覆核員，mustFix 必修一輪 + 完整性批判補漏）。
格式與分級定義見 docs/plan/home-resume-phase-tables.md（含五步 HOME 物料協定與 M1/M2/A1-A4 決議）。
範本兩表（Empty DoGoUpTray、SortArm DoPlaceToAuto）在該文件，不重複於此。

## 總表

| # | action | 判定 | 缺口代碼 |
|---|---|---|---|
| 1 | TEmptyModule::DoFeedTray | 黃 | EF-1, EF-2, EF-3, EF-4, EF-5 |
| 2 | TEmptyModule::DoGoDownTray | 黃 | EG-1, EG-2, EG-3, EG-4 |
| 3 | TColorModule::DoFeedTray + TColorModule::DoReadColor2D (含 DoColor 派工、DoGoDownTray、DoGoUpTray) | 黃 | CF-1, CF-2, CF-3, CF-4, CF-5, CF-6 |
| 4 | TColorModule::DoGoUpTray + TColorModule::DoColor case 100/1700 (return-receipt) + RequestReturnTray/NotifyTrayXToEmptyFinish | 黃 | CG-1, CG-2, CG-3, CG-4, CG-5, CG-6 |
| 5 | TLoaderModule::DoFeedTray + TLoaderModule::DoCcdCheck (CcdTask) + TLoaderModule::FindNextCcdCell | 黃 | LF-1, LF-2, LF-3, LF-4, LF-5, LF-6, LF-7 |
| 6 | TLoaderModule::DoDischargeTray (dispatcher: TLoaderModule::DoLoader case 3000/4000) | 黃 | LD-1, LD-2, LD-3, LD-4, LD-5 |
| 7 | TLoaderModule::DoFrontDestackDown (aLoader.cpp:2020-2099) + 包裹段 TLoaderModule::DoFeedTray (aLoader.cpp:1291-1583);同族 Teach 梯 TestGoDownTray/TestGoUpTray (aLoader.cpp:2103-2207) | 黃 | LK-1, LK-2, LK-3, LK-4, LK-5, LK-6, LK-7 |
| 8 | TTrayArmModule::DoPick + DoMoveToStationZSafe + DoLowerClampRaise (含 DoTrayArm case 100/1000 派工與殘留守衛) | 黃 | TP-1, TP-2, TP-3, TP-4, TP-5 |
| 9 | TTrayArmModule::DoPlace (TAPLACE_AUTO 路徑, aTrayArm.cpp:715-815) | 黃 | TA-1, TA-2, TA-3, TA-4 |
| 10 | TTrayArmModule::DoPlaceToEmpty + TTrayArmModule::DoPlaceToColor (aTrayArm.cpp:903/982, 含 DoTrayArm case-100 resume 與 OnPlaceGateBlocked) | 黃 | TR-1, TR-2, TR-3, TR-4, TR-5 |
| 11 | TSortArmModule::DoPickFromLoader | 黃 | SP-1, SP-2, SP-3, SP-4 |
| 12 | MarkResidueTargets / CheckPlaceResidue / IsResidueCheckBusy / TAutoModule::SetPlaceResidueClear | 黃 | SR-1, SR-2, SR-3, SR-4, SR-5 |
| 13 | TAutoModule::DoFeedTray (含 FindFeedAuto / RefreshAutoState / StageRearGrid / NotifyTrayArmDelivered / SetRearHasTrayFromTrayArm) | 黃 | AF-1, AF-2, AF-3, AF-4, AF-5 |
| 14 | TAutoModule::DoDischargeTray (aAuto1To6.cpp:657-751; 尾段共用 TAutoModule::DoFrontRiseOnce aAuto1To6.cpp:1646-1678) | 黃 | AD-1, AD-2, AD-3, AD-4 |
| 15 | DoAllAutoCleanOut (aAuto1To6.cpp:753) + DoFeedTray rear-collect 子梯 (aAuto1To6.cpp:464) + DoAuto CleanOut 入口/短路 (aAuto1To6.cpp:1561) + csystem post-HOME Run_CleanOut resume (csystem.cpp:1373-1377) | 黃 | AC-1, AC-2, AC-3, AC-4, AC-5, AC-6 |
| 16 | CrossModule-Setter-Checklist (TrayArm/SortArm/AgvCoord → Loader/Empty/Color/Auto) | 黃 | XS-1, XS-2, XS-3, XS-4, XS-5, XS-6 |
| 17 | TAgvCoordinator::PollAndCall / TAgvCoordinator::ServiceHandshake / TAgvCoordinator::BeginPrep(+ 模組側 TAutoModule::SetAmrLock / InfeedSetLock)（補漏） | 紅 | FX-1, FX-2, FX-3, FX-4, FX-5, FX-6, FX-7, XS-4 |
| 18 | TAutoModule::ServiceCarFull（補漏） | 黃 | FX-1, FX-2, FX-3, FX-4, FX-5, FX-6, FX-7, FX-8 |

## 總結（rollup）

### 判定分佈（採批判建議，拆兩行）

- **AMR=0（Normal 客戶模式）＝黃**：16 張主表全黃，缺口全部可枚舉、修法皆為小守衛／保留 latch／drain 規約，無需新機構決策。
- **AMR 模式＝紅**：AGV 協調器（補漏表）判紅——`Handshake[]` 跨 HOME 存活但四模組 `bAmrLocked` 被 InitialFlag 全清（鎖／狀態分裂）、`ServiceHandshake` 無 Run_Home 閘（HOME 中照發 CEID273/274）、且「AMR 已靠站時全機 HOME」的空間與協定完全未定義。

註：兩張補漏表各自使用了 FX-n 代號（撞號）——引用時以 **FX(A)-n**＝AGV 協調器表、**FX(S)-n**＝ServiceCarFull 表區分。

### 待 owner 決策叢集（拍板前這幾項視同紅）

| 代號 | 問題 | 類型 |
|---|---|---|
| FX(A)-6 / XS-4 | AMR 已靠站（READY）時允不允許全機 HOME？PARK 回位／歸位路徑是否穿越 AMR 對接空間？host 端要不要收「設備 HOME 中」訊號？ | owner＋AGV 協定 |
| SP-1 | HOME 進場對「真空 ON 但未 commit」的針，趁 Z 仍在下位 OffSuck 讓 IC 留穴——「IC 會留在穴內」是物理推論，需確認 | 機構確認 |
| AC-1 | EMG 斷氣時 Auto FrontRise 是否自然落下？（決定 drain 跳過路徑的 fallback） | 機構確認 |
| TR-5 / TA-1 | drain 中允不允許「執行開夾交接」（PlaceTask 2000 Pop 已下令未確認的子態）？兩表規則互斥，需裁決（保守解＝一律不納入，靠 TR-1 resume 守衛兜底） | owner 裁決 |

### 合併工項（去重後的實作清單，依相依排序）

| # | 工項 | 吸收的缺口代號 |
|---|---|---|
| W1 | **Empty/Color/Loader InitialFlag 增 bKeepMaterial 變體**（對齊 Auto/TrayArm/SortArm 簽名；一次重構，勿六次補丁） | EF-1、CF-1、CG-1、LF-4、LK-2、LK-7、XS-5 |
| W2 | **drain 引擎仲裁規格**（單一文件先行）：統一各表 drain 邊界（裁決 LF-1 vs LK-3——建議採 LK-3+LK-1：drain 不含 modal 段，鑄造由 resume 側補；LF-2 的 8200 邊界統一為 9000 進入點）、modal 一律抑制（EG-3、AF-3、AC-5、SR-4）、跨模組互鎖同拍 pump（EG-2、LK-5）、Task 值無法區分的子態改讀缸 out-bit | LF-1/LF-2、LK-3/LK-4/LK-5、EG-2/EG-3、AF-3、AC-5、SR-4/SR-5、TA-1、TR-5 |
| W3 | **PARK/RE-ACQUIRE 落地**（含 Empty/Color/Auto 車全部納入枚舉；判準用夾缸 reed 而非 fHasTray；Empty 拖運中斷＝重夾後補完到後座）＋**拆除 uHome case 200 強制移盤+ClearTray**（必須同批原子落地，LF-6 警告：只拆不接＝盤與資料脫鉤） | EF-2、CF-2、CG-2、AF-1、AF-4、LD-3、LF-6、LK-7 |
| W4 | **殘料驗證 fail-open 修正**：aAuto1To6.cpp:94 移到 keep-material continue 之後（SR-1≡AD-4 同一行）；SortArm keep 分支保留驗證名單並於 HOME 後重武裝 | SR-1、SR-2、SR-3、AD-4、XS-3 |
| W5 | **AGV/AMR HOME 整合**：InitialAllTask 重掛鎖或 AbortAutoHandshake、ServiceHandshake 加 Run_Home 凍結、bOperatorHolding 保留＋sensor-OFF 邊沿釋放、BeginPrep 加狀態閘 | FX(A)-1~5、FX(S)-1~5、FX(S)-8、LF-7 |
| W6 | **零散守衛**（各一行～數行）：TP-1 保夾/收養判準不對稱、XS-1/XS-2/TA-2「已放未簽」adopt-as-delivered 守門、TP-4 Auto rear 再驗+MES1723 掛勾、EG-1 三缸 out-bit 姿態閘、LK-1 落盤未鑄造自收、AF-2 case-7000 代跑、AD-1 出料尾段 latch、AD-2 FrontRise Off 收斂、SP-2/SP-3 保留 PickX/PickY 與 sticky 邊、CG-3/CG-4 resume-at-phase 與結案守門、EF-3 重簽空窗、TA-3 iDeliverKind 重設 | 同列各項 |

### 矛盾裁決紀錄

- **「歸 Y 必拖盤」vs「已 home 伺服會跳過」**（EF-1/CF-2 vs CG-2/AF-4）：以現行程式為準——**今日每次全機 HOME 都必實體重歸所有軸**（ArmMotorHome 無條件清全軸 bHomeFlag，`IsHomedServoSkippable` 被上游架空），故「必拖盤」是今日事實；CG-2/AF-4 的「跳過」敘述僅在未來政策改變時成立。維持全機 realign 政策不變（已決議），PARK 是唯一解。
- **TrayArm 互等「端到端關閉」宣稱**（trayarm-place-recycle）：範圍限縮為「**攜盤中**」子集——「已放未簽」視窗（XS-1/XS-2）heal 反而會驅動 GoUp 吃掉剛放的盤，需 W6 的 adopt-as-delivered 守門補齊。

### 驗證備註

AMR 象限的視窗（ServiceCarFull、AGV 協調器）在 SOFT_SIMULATE 下結構性不可見（sim 自動清車、無 Full sensor）——這兩塊的驗收必須排實機或 SECS Simulator 注入（AmrInject 注入點現成）。

## TEmptyModule::DoFeedTray（黃）

> 覆核：OK。缺口代碼：EF-1, EF-2, EF-3, EF-4, EF-5

DoFeedTray(aEmpty.cpp:376-487)是純線性的「前座夾取→Y搬運→後座釋放→感測確認+格子交接」梯形,唯二馬達段為 case 1000/4000(MoveEmptyY, aEmpty.cpp:410/433)。今日全機HOME最大破口:uHome 只釋放 Loader1/2 的 Lean/Push(uHome.cpp:548-562),從不釋放 C_Empty_LeanOnTray/PushTray,卻在 case 200 對 MEmptyY 回原點(uHome.cpp:620)→ 夾持中(FeedTask 2000-6000)落點會拖盤,盤停在軌道中段時前後感測皆 OFF,重建後隱形→下一輪 GoDown+feed 雙盤相撞(紅)。閒置/確認段則綠:RefreshStateFromSensors 的後座 false→true 邊緣自動 BirthRearTray(aEmpty.cpp:178-179,492-495),且 EMPTY_IC=0(cmydef.cpp:41)使 Birth≡Clear(MyMotor.cpp:70-78),Empty 格子無資料可失。bReturnTray 互斥:DoEmpty case 100 先判 bReturnTray(aEmpty.cpp:238)再判餵料(aEmpty.cpp:310),d63d33a 於 aTrayArm.cpp:1090-1091 重簽握手,僅剩一掃描空窗(EF-3,自癒)。任務提示的「case-261/288」實為行號:aEmpty.cpp:262-301 是 MES1022 缺料告警;餵料路徑真正的 sensor-miss 是 JAM1030(FeedTask 2000, :425)與 MES1021(FeedTask 7000, :455),前座是 MES1024(GoDownTask 700, :652)。規劃中協定可解 2000/5000/6000 視窗(drain)與 2000 夾持拖盤(PARK),但 case 4000 中途落點需 EF-2 的「re-acquire 補完到後座」決策才收斂。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|------|-------|------|--------|--------|----------|-------|
| 重置入口 | Flag==0 | FeedTask=1、FeedClampSub=0、FeedDelay.Clear、兩夾缸一次性 Reset(僅清內部 Task,不動出力;mycylin.cpp:162-166)(aEmpty.cpp:380-391) | — | 無 | 相位起點 | — |
| 派工/冪等短路 | FeedTask 1,10 | case 10 先 RefreshStateFromSensors;bRearHasTray→跳 13000(冪等重入),否則→1000(aEmpty.cpp:399-407) | SnEmpty_InputHasTray / SnEmpty_OutputBottomHasTray(sim/DUMMY 早退守 latch,aEmpty.cpp:156-157) | 無 | 相位起點 | —(本身即邊界) |
| 空車就位 | 1000 | MoveEmptyY(Teach.EmptyCarFeedTrayYPosition);內含 TrayArm Z-up/X 防撞閘(aEmpty.cpp:196-205,410) | 馬達到位(MotorMove 可重發) | 無 | 冪等段(車上無盤,重發即可) | 不可(馬達) |
| 前座夾取 | 2000(FeedClampSub 0/10/20/30) | DoClampTray:Lean.Push→Push.Push→settle→Push.OnSensor 確認;miss→退 Push 回報 2→JAM1030 K_RETRY 回 1000(mycylin.cpp:40-85;aEmpty.cpp:414-429) | 夾缸磁簧 + iEmptyFeedClampSettleMs 計時 | 無 | 冪等段(已夾者 Push 數掃描內快轉確認) | 可收斂→4000(邊界=夾持確認,絕不可續入 4000 馬達段;PARK 隨後放夾) |
| 搬運至後座 | 4000 | MoveEmptyY(Teach.EmptyCarDischargeTrayYPosition);到位才 bFrontHasTray=false(aEmpty.cpp:432-438) | 馬達到位 | bFrontHasTray=false(到位那一掃描) | 危險段(夾持盤在軌道中段;只能經 2000 前綴進入) | 不可(馬達+載盤) |
| 後座釋放 | 5000,6000 | 先 Pop PushTray 再 Pop LeanOnTray(sim 直通)(aEmpty.cpp:440-448) | 夾缸 Off 磁簧(Pop 確認) | 無 | 冪等段(重 Pop 快轉) | 可收斂→13000 |
| 後座確認+格子交接 | 7000 | REALLY 且感測 OFF→MES1021(K_SKIP/K_RETRY;SKIP 清空雙 latch);否則單一掃描內:bRearHasTray=true、Status=ES_REAR_READY、Tray.MoveFrom(FrontSourceTray)、fHasTray=true、Refresh、FeedTask=13000(aEmpty.cpp:450-477) | SnEmpty_OutputBottomHasTray(iRealDummy!=DUMMY 才查,:453) | 原子commit(:466-477 同掃描完成,VERIFIED) | 原子commit | 可收斂→13000(純資料+汽缸已放;miss 時有 modal 風險,drain 前應先讀感測) |
| 終端 | 13000 | FeedTask=1、return true(IsCleanOutFinish 的 FeedTask==1 閘依賴此)(aEmpty.cpp:480-484) | — | 無 | 相位起點 | — |

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|------|----------|-----------|
| FeedTask 1/10/13000(閒置) | InitialFlag 全清(aEmpty.cpp:25-50,含 FrontSourceTray.Clear+MMEmptyY->ClearTray);復歸後 RefreshStateFromSensors 重掛前/後座,後座 false→true 邊緣 BirthRearTray 重生格子(:178-179);ComputeRearPickReadyNoRefresh 刻意用負向式容忍 Status 被洗成 ES_IDLE(:1128-1141) | 綠。EMPTY_IC=0 ⇒ 重生格子與原格子內容等價(cmydef.cpp:41;MyMotor.cpp:70-78),Empty 無資料損失 |
| FeedTask 1000(空車移動) | 車上無盤、夾缸實體已放;HOME 直接回原點,前座盤留在座上重掛,復歸重跑餵料 | 綠(今日即可重入) |
| FeedTask 2000(夾取中) | 夾缸在前座咬盤;uHome case 200 homes MEmptyY(uHome.cpp:620)且不放 Empty 夾(僅 Loader,uHome.cpp:548-562)→ A1 反面:夾住即拖盤;拖離前座後前感測 OFF,盤隱形;下輪 GoDown 再落一盤到前座相撞(拖距視原點偏移,INFERRED) | 紅(今日)。規劃協定:drain 收斂到 4000 邊界+PARK 放夾→Y 空跑→RE-ACQUIRE 回位重夾→復歸時盤仍在前座、感測重掛→綠 |
| FeedTask 4000(載盤搬運中) | 拖盤且停軌道中段:前/後感測皆 OFF → 雙 latch false → DoEmpty case 100 先 GoDown(:248)再 feed(:310):新盤落前座+車帶殘盤回前座=雙盤相撞 | 紅(今日)。協定後仍黃:梯形重啟看不見中段盤 → 需 EF-2(re-acquire 補完搬運到後座),否則同樣雙盤 |
| FeedTask 5000/6000(放夾中) | 盤已在後座感測上,但 Lean 可能仍勾住(6000 未完)→ HOME 回原點把盤拖離後座、盤隱形 | 紅偏黃(今日)。協定:drain 直收 13000(全放+確認)→ 綠;收斂後 ComputeRearPickReadyNoRefresh 雙 out-bit 閘(:1139)自然滿足 |
| FeedTask 7000(確認/交接前) | 盤在後座、夾已全放;commit 未跑即 HOME:洗掉 → 復歸邊緣 BirthRearTray 重掛+重生,內容等價 | 綠(今日)。MES1021 miss 分支僅在盤真的不在時觸發,屬正確告警 |
| DoEmpty Task=2000 包裹層 + bReturnTray 互斥 | case 100 先判 bReturnTray(:238)後判 feed(:310)⇒ 保留簽時餵料永不啟動;d63d33a 復歸重簽(aTrayArm.cpp:1090-1091,RequestReturnTray 冪等 aEmpty.cpp:1167-1171);TrayArm 側另有 ES_FEEDING 防跳水閘(aTrayArm.cpp:944)與 60s MES1723 看門狗 | 黃:洗掉→重簽間有一掃描空窗(EF-3),Empty 先掃且後座空可搶跑一整輪 feed;之後重簽驅動的 GoUp 清後座自癒,只損時間不撞機 |

### 缺口

- EF-1(紅,規劃協定即解):uHome 從不釋放 C_Empty_LeanOnTray/PushTray(uHome.cpp 僅 case 50/60 放 Loader1/2,:548-562)卻在 case 200 homes MEmptyY(:620)→ FeedTask 2000-6000 落點必拖盤。最小修正:PARK 階段把 Empty 車納入「tray-carrying MotorY」清單(它正是 Lean/Push 對),依規劃順序先放 LeanOnTray 再放 PushTray;過渡期可在 uHome 比照 case 50/60 加 Empty 放夾+強制移盤。
- EF-2(黃,協定設計決策):FeedTask 4000 中途 PARK 後盤停軌道中段,前後感測皆 OFF;復歸梯形重啟(FeedTask=1)無記憶,DoEmpty case 100 先 GoDown(aEmpty.cpp:248)後 feed(:310)→ 雙盤。最小修正:Empty 的 RE-ACQUIRE 不是「回記憶點就停」,而是重夾後補完搬運到 EmptyCarDischargeTrayYPosition 並放夾;後座感測 false→true 邊緣(:178-179)自動 BirthRearTray+latch,復歸零記憶收斂。
- EF-3(黃,自癒但可縮):bReturnTray 被 InitialFlag(:38)洗掉、TrayArm 到 DoTrayArm case 100 才重簽(aTrayArm.cpp:1090-1091)之間有一掃描空窗;Empty 若先掃且後座空會搶跑整輪 feed,再靠重簽驅動 GoUp 清後座(多一趟來回,60s MES1723 為底線)。最小修正:模組掃描順序讓 TrayArm 重簽先行,或 DoEmpty case 100 的 feed 分支加「TrayArm 正攜盤入 Empty」查詢閘。
- EF-4(黃,協定 drain 順帶解):HOME 落在前座升降中(M1:升起時 SnEmpty_InputHasTray 仍讀有盤)→ 復歸 feed 夾空氣 → MES1021(aEmpty.cpp:455)K_SKIP 清雙 latch(:460-461)但升降缸仍伸出,GoDown 對半升狀態重跑。最小修正:規劃第(1)步對 GoUp/GoDown(純汽缸梯形)先 drain 到終端,DoFeedTray 只需繼承其收斂結果,自身不用改。
- EF-5(綠,文件勘誤):任務提示的「case-261/288」不是 FeedTask case,是 aEmpty.cpp 行號 262-301 的 MES1022 缺料告警(DoEmpty case 100);餵料路徑真正的 sensor-miss 為 JAM1030(FeedTask 2000,:425)與 MES1021(FeedTask 7000,:455),前座對應 MES1024(GoDownTask 700,:652)。修正方向:HOME 計畫文件引用改為告警碼+FeedTask case 編號。

### drain 邊界

DoFeedTray 的 HOME drain 可收斂邊界(全部 VERIFIED 無 Move 呼叫、無真空交接):FeedTask==1/10/13000 本身即邊界,免 drain;FeedTask==2000(含 FeedClampSub 0/10/20/30)為純汽缸+settle 計時,可 drain 至「case 4000 入口」邊界(夾持確認完成)後交由 PARK 放夾——絕不可讓 drain 續入 case 1000/4000(MoveEmptyY 馬達段,aEmpty.cpp:410/433),此二段為不可 drain 段,落點只能停軸+PARK(A1 放雙夾後 Y 移動不拖盤)+記憶 Y/盤;FeedTask==5000/6000/7000 可一路 drain 至 13000(Pop PushTray→Pop LeanOnTray→後座感測確認+單掃描格子交接,aEmpty.cpp:440-477),收斂後盤在後座、雙夾 out-bit OFF,ComputeRearPickReadyNoRefresh(:1137-1140)與 HOME 後的感測邊緣重生(:178-179)都能無記憶重建。注意兩個 modal 風險:case 2000 的 Clamp==2→JAM1030 與 case 7000 感測 OFF→MES1021 皆會開 modal——drain 執行器應先預讀 SnEmpty_OutputBottomHasTray/Push.OnSensor,失敗即改走 PARK+人工移除備援,不得在 drain 中彈窗。

## TEmptyModule::DoGoDownTray（黃）

> 覆核：OK。缺口代碼：EG-1, EG-2, EG-3, EG-4

1. DoGoDownTray(aEmpty.cpp:527-671)全梯=純汽缸+settle timer+sensor:cases 100-600 只有 PushCylinder/PopCylinder/GoDownDelay,零 Move*、零真空(VERIFIED)→是五步協定①drain 的最佳收斂對象,可整段順向 pump 到 case 700 邊界。
2. 唯一毒點=與 DoGoUpTray 的方向不對稱:M1(front sensor 在盤被 Rise 懸空時仍讀 ON)讓 GoUp 重入導向「重跑上升」(安全),卻讓 GoDown 的 case 10 冪等閘(aEmpty.cpp:548-552)導向「跳過=已完成」——缸留伸出、整疊懸空。
3. 今日中斷在 100-500:resume 後 DoEmpty case 100 讀到幻影 bFrontHasTray=true→跳過 GoDown、誤派 DoFeedTray 夾空氣→MES1021 警報 livelock(RETRY/SKIP 都繞回同路),destack 缸無人收;唯 IsReadyForAmrHandoff 讀 out-bit(aEmpty.cpp:97-99)擋住 AMR=安全側。
4. 修法一點(EG-1):跳過閘+DoEmpty 派工加「三缸 GetOutBit 皆 false」姿態條件;out-bit 經 HOME 存活(InitialAllTask 不碰 HSys.Cyn database.cpp:39-67;uHome 僅 620 行歸 MEmptyY),任一 true→續 pump GoDown 冪等快轉即收斂,M2 保證爪操作靜止安全。
5. commit=case 700 單掃描(bFrontHasTray=true+BirthFrontTray 同 scan,aEmpty.cpp:658-659);中斷只有「未確認/已確認」兩態。
6. EMPTY_IC==0(cmydef.cpp:41)使 Clear() 格與 Birth(EMPTY_IC,Normal,"") 資料同值→HOME 後 front 重推導漏掉 Birth 為良性(EG-4 綠);Color 鏡像 Kind=Identity 不等值,屬 Color 表。
7. drain 設計兩注意:case 200 互鎖等 Loader front-separate out-bit→drain 須同拍 pump 兩模組(EG-2);drain 邊界停在 700 之前,MES1024 sensor 判定留給 resume 的 Refresh 重推導(EG-3)。
8. 判定黃:正常 HOME 走協定 drain 後全綠;成因閘門跳過 drain(EMG/安全門)時需 EG-1 小守衛,均可枚舉。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| P0 進入判定 | 1,10 | 無運動;`RefreshStateFromSensors`(aEmpty.cpp:547);front 已有盤→直跳 99999(aEmpty.cpp:548-552);否則 `Rise_1.Reset()` 一次性再武裝(aEmpty.cpp:553) | SnEmpty_InputHasTray | — | 相位起點(**內建冪等快轉——但 M1 使其在缸伸出姿態下誤判,見 EG-1**) | — |
| PD1 升一段 | 100 | `C_Empty_FrontRiseTray_1` Push(aEmpty.cpp:561) | 缸 On reed(PushCylinder=命令+等到位;已達成數 scan 內確認) | — | 冪等段(上升單調段) | 可收斂→700 |
| PD2 升二段 | 150 | `C_Empty_FrontRiseTray_2` Push(aEmpty.cpp:569) | 缸 On reed | — | 冪等段 | 可收斂→700 |
| PD3 插分離爪 | 200 | Loader 互鎖等待 `IsFrontSeparateBlockedBy(C_Loader_FrontSeparateTray_1)`(aEmpty.cpp:578,mycylin.cpp:90-95)→`C_Empty_FrontSeparateTray_1` Push→settle 起動(aEmpty.cpp:580-585) | 缸 On reed+對方 GetOutBit 互鎖 | — | 冪等段(M2:reed 確認=靜止,重插爪安全) | 可收斂→700(**需與 Loader destack 同拍 pump,見 EG-2**) |
| PD4 降二段 | 300 | settle 到期→`Rise_2` Pop→settle(aEmpty.cpp:596-604;Pop 確認防掉疊,590-595 註解) | 缸 Off reed+GoDownDelay | — | 冪等段(下放單調段) | 可收斂→700 |
| PD5 退分離爪 | 350,400 | settle→`Separate` Pop→settle(aEmpty.cpp:610,616-621) | 缸 Off reed | — | 冪等段 | 可收斂→700 |
| PD6 降一段 | 450,500,600 | settle→`Rise_1` Pop→settle(aEmpty.cpp:627,633-638,641-643) | 缸 Off reed | — | 冪等段 | 可收斂→700 |
| PD7 front 確認+commit | 700 | 無運動;讀 SnEmpty_InputHasTray:miss(Enable+IsOff+非DUMMY)→`bFrontHasTray=false`+MES1024,K_RETRY→Task=1 整段重跑(aEmpty.cpp:647-655);有盤→`bFrontHasTray=true`+`BirthFrontTray()`+→99999,同一 scan(aEmpty.cpp:656-661) | SnEmpty_InputHasTray | bFrontHasTray=true+FrontSourceTray.Birth(EMPTY_IC,Normal,"")(單掃描原子,VERIFIED 658-659) | 原子 commit | —(無運動;drain 邊界即在此之前,判定留 resume,見 EG-3) |
| 終端 | 99999 | 無 | — | GoDownTask=1(回 idle,供 IsCleanOutFinish 閒置閘,aEmpty.cpp:664-668) | — | — |

備註:Flag==0 進入式只設 GoDownTask=1+清 delay(aEmpty.cpp:531-536),缸的一次性 Reset 全部內嵌在各 case 的 Push/Pop 之前(feeder-unify 慣例)→從 case 1 重入永遠先再武裝,無殘留 Push Task 問題(VERIFIED)。全梯無任何 Move*/真空呼叫(VERIFIED aEmpty.cpp:527-671)。

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| W0(P0,case 1/10) | 缸全縮、尚無實體動作。InitialFlag 清 GoDownTask=1/bFrontHasTray(aEmpty.cpp:34,36);resume 時 DoEmpty case 100 `RefreshStateFromSensors`(aEmpty.cpp:230)讀真實 sensor 重新派工(front 無盤→重派 GoDown,aEmpty.cpp:248-255) | 綠 |
| W1(PD1-PD3,100-200) | uHome 不碰 C_Empty_* 缸(uHome.cpp 僅 620 行歸 MEmptyY)→姿態凍結:整疊被 Rise 懸空(可能已插爪)。resume Refresh 因 **M1** 讀 ON→幻影 bFrontHasTray=true→DoEmpty 跳過 GoDown、派 DoFeedTray(aEmpty.cpp:310-315)→MoveEmptyY+DoClampTray 夾空氣(aEmpty.cpp:410-430)→case 7000 rear miss MES1021(aEmpty.cpp:450-465);RETRY 重夾空氣、SKIP 清雙閂後下輪 GoDown case 10 又被 M1 快轉跳過→**警報 livelock,destack 缸永留伸出**。唯 IsReadyForAmrHandoff 讀 out-bit(aEmpty.cpp:97-99)擋 AMR 補料=安全側。計畫 drain(純汽缸,可收斂→700)直接消滅此窗;drain 被成因閘門跳過(EMG/安全門)時仍現形 | 今日**紅**;協定後黃(EG-1 一個姿態守衛即補上跳過路徑) |
| W2(PD4-PD6,300-600) | 同 W1(M1 幻影同因;300-500 下放中段判定同 W1)。落在 600(三缸已全縮,只剩 settle)時盤實體已落座→resume sensor 讀 ON 為**真實**,跳過=正確,行為等同 W3。另:若 TrayArm resume(d63d33a)重發 RequestReturnTray→DoEmpty 先走 bReturnTray→DoGoUpTray(aEmpty.cpp:238-245),GoUp 從凍結姿態重跑上升段——屬 GoUp 表已列視窗(M2 已決) | 中段今日**紅**/協定後黃(EG-1 同一修);600 綠 |
| W3(PD7,700) | 缸全縮,盤真實落座(或真缺盤)。commit 兩行單掃描;中斷後 resume Refresh 讀真實 sensor:有盤→true→case 10 快轉=正確(FrontSourceTray 未 Birth,但 EMPTY_IC==0 使 Clear 格同值,EG-4 良性);缺盤→false→重派 GoDown 整段重跑=正確(MES1024 留給 resume 正常彈出) | 綠 |
| W4(99999/已完成) | 與 W3 有盤分支相同:sensor 真實重推導,閂鎖與 Birth 缺失均良性 | 綠 |

### 缺口

- **EG-1(黃)**:M1 使 SnEmpty_InputHasTray 無法區分「盤落座」與「整疊懸空 mid-destack」,而 GoDown 的 case 10 快轉閘(aEmpty.cpp:548-552)與 DoEmpty case 100 派工(aEmpty.cpp:248)都只看 bFrontHasTray→drain 被成因閘門跳過時,缸留伸出+誤派 DoFeedTray→MES1021 livelock。最小修:兩處跳過/派工條件加「C_Empty_FrontRiseTray_1/2+FrontSeparateTray_1 三缸 GetOutBit 皆 false」;任一 true→改派 DoGoDownTray(0) 續 pump(冪等快轉自然收斂,M2 保證靜止安全)。out-bit 經 HOME 存活:InitialAllTask 不碰 HSys.Cyn(database.cpp:39-67)、uHome 不碰 Empty 缸(uHome.cpp:620 僅軸)——VERIFIED。
- **EG-2(黃)**:drain 併發互鎖——case 200 等 Loader C_Loader_FrontSeparateTray_1 out-bit 退出(aEmpty.cpp:578);若 drain 逐模組串行且 Loader destack 也 in-flight(separate 伸出),Empty 收斂卡互鎖到 drain 逾時。最小修:drain 階段沿用 MainProc 同一掃描序同拍 pump 各 feeder 梯,互鎖自解(GetOutBit 在 sim 也成立,mycylin.cpp:88-89)。
- **EG-3(綠/規約)**:drain 邊界規格=「六個 Push/Pop 全 reed 確認+settle 到期」即 case 700 之前;不得在 drain 內執行 700 的 sensor 判定(缺盤會彈 MES1024 modal 且 K_RETRY 會在 drain 中重跑整段)。front 態由 HOME 後 RefreshStateFromSensors(aEmpty.cpp:162-163)重推導,W3 已證安全。
- **EG-4(綠/註記)**:HOME 後 front 重推導不補 BirthFrontTray(Refresh 僅 rear 有 edge-birth,aEmpty.cpp:172-183)——已驗證良性:EMPTY_IC==0(cmydef.cpp:41)使 Clear()(MyMotor.cpp:47-61)與 Birth(EMPTY_IC,eTrayKindNormal,"")(MyMotor.cpp:72-78)資料/Kind/ID 全同值,DoFeedTray 7000 MoveFrom 交接(aEmpty.cpp:472)結果一致。注意:**Color 鏡像不良性**(aColor.cpp:1167 Birth 用 eTrayKindIdentity≠Clear 預設 Normal),歸 Color DoGoDownTray 表處理。

### drain 邊界

DoGoDownTray 是 drain 的教科書對象:cases 100-600 全部只含 PushCylinder/PopCylinder(命令+等位置 reed,冪等快轉)與 GoDownDelay settle,無任何馬達 Move、無真空交接(VERIFIED aEmpty.cpp:557-644)。drain 從任意 in-flight case(100/150/200/300/350/400/450/500/600)順向 pump 至 **case 700 邊界**——即三缸(FrontRiseTray_1、FrontRiseTray_2、FrontSeparateTray_1)Off reed 全確認且末段 settle 到期——然後停住,不執行 case 700 的 SnEmpty_InputHasTray 判定與 commit(缺盤時該判定會彈 MES1024 modal 且 K_RETRY 迴圈重跑整段,不適合在 drain 內發生);front 有盤/缺盤留給 HOME 完成後 DoEmpty case 100 的 RefreshStateFromSensors 重推導,此時缸全縮、M1 幻影不存在,sensor 讀值真實(W3 已證兩分支皆正確)。兩個前置條件:(a) case 200 的 Empty↔Loader front-separate 互鎖要求 drain 同拍 pump 兩模組的 destack 梯,否則 Loader separate 伸出時 Empty 收斂卡死到逾時(EG-2);(b) drain 被成因閘門跳過(EMG/安全門/氣壓失效)時,本 action 依協定「不得自動續跑」——以三缸 GetOutBit 姿態檢查(EG-1)代替 retain latch 即可辨識並收斂,無需新增持久化狀態。

## TColorModule::DoFeedTray + TColorModule::DoReadColor2D (含 DoColor 派工、DoGoDownTray、DoGoUpTray)（黃）

> 覆核：ISSUES（已依覆核修正）。缺口代碼：CF-1, CF-2, CF-3, CF-4, CF-5, CF-6

1. Color 供盤鏈 = DoColor 派工 → DoGoDownTray(純缸分盤) → DoFeedTray(夾盤→掃描→送後→呈盤) → TrayArm 取走;識別在 DoReadColor2D 五個出口 stamp(aColor.cpp:1065/1099/1120/1142/1149)。
2. bSupplyRequested 是「輪詢式」需求:TrayArm 每派工掃描重發(aTrayArm.cpp:524-527),HOME 滅掉會自癒 — 這是 Color 最大的先天優勢(VERIFIED)。
3. 但 InitialFlag() 無 bKeepMaterial(database.cpp:65-66):HOME 把已呈盤好盤的 bTrayReady/識別全滅(aColor.cpp:40-44,54),實機感測重推 bRearHasTray 後被 DoFeedTray case 10 判成剩盤 → MES1426 要操作員拆走(:895)— 降級不死機,identity 盤無 IC 故無料損。
4. 紅色視窗=拖盤家族:凡任一夾缸未確認 Off 的段(Feed 2000-6000、GoUp 3000-7000)— uHome 不碰 Color 夾缸,Y 歸 home 拖盤;運送段重啟又先分新盤再把舊盤開進前接盤位(DoColor :354-383 順序),無 alarm(INFERRED 機構)。鬆夾子段(Feed 5000/6000、GoUp 6000/7000)drain 收斂即除;運送段需規劃 PARK/RE-ACQUIRE 納入 MColorY + 一個續走閘。
5. 兩條分盤缸列(GoDown 100-700、GoUp 100-600)零馬達呼叫(VERIFIED),完全符合規劃 drain;不 drain 時 riser 半空 strand + M1 誤判使 GoDown 跳過、Feed 夾空氣照過(DoClampTray reed 感活塞非盤,mycylin.cpp:64-81)→ 失效簽名後移為 ColorCCD_2D 掃描逾時 modal(:1127-1131)與 case 7000 MES1421(:997-1019),疊 strand 無專屬警示。
6. 回收側:arm 仍持盤已由 d63d33a 補發治癒(aTrayArm.cpp:1078-1093);「已卸盤但 GoUp 未完」不在治癒範圍,退化成 MES1426。
7. 結論 yellow:無需新機構決定,可列舉小修(保留 latch、續走閘、drain、re-arm、識別記憶)即達重入。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| DoColor 派工 | 1,10,100 | RefreshStateFromSensors;分支序:bReturnTray回收(:298)→CleanOut排空(:317)→bTrayReady閒置(:344)→無前盤destack(:354)→bSupplyRequested供料(:378) | 感測器+latch | 無 | 相位起點 | — |
| DoColor 泵/回收等待 | 1000,1200,1700,2000 | 子梯形泵;1700 完成時 bReturnTray=false+iReturnedCount++/iSimInfeedCount++(:415-417 單scan);未卸盤前 return 持住(:413) | 子梯形 | 1700 出口單scan | 依子梯形 | 依子梯形 |
| Feed 入口+剩盤閘 | 1,10 | 模式檢查;bRearHasTray→MES1426「剩盤請拆」(:895)→仍在則 13000 中止(bSupplyRequested 未清→下輪再彈) | SnColor_OutputBottomHasTray / SnColor_TrayPos1(:207-222) | 無 | 相位起點 | — |
| Feed 往前接盤位 | 1000 | MoveColorY(Teach.ColorReceiveTrayYPosition)(:910) | 馬達到位 | 無 | 冪等段 | 不可 |
| Feed 夾盤 | 2000 | DoClampTray(LeanOn→Push)(:919);Clamp==1 同scan MMColorY->Tray.MoveFrom(FrontSourceTray)+fHasTray=true(:928-930)並進3000;Clamp==2→MES1422 retry(:938)— 惟 reed 感活塞位置非盤存在、settle 預設500ms(mycylin.cpp:64-81;GeneralSetting.cpp:52,:128;aColor.cpp:917-918「SettleTicks=0」註解已過時),無盤時活塞自由到底 reed ON→Clamp==1 照過 | 缸 reed | MoveFrom+advance 同一scan(:926-932) | 冪等段(出口原子commit) | 可收斂→3000 |
| Feed 掃描移動 | 3000 | MoveColorCcdToScan()=Y+CCDX 雙馬達(:954,:840-850) | 馬達到位 | 無 | 冪等段 | 不可 |
| Feed 2D讀取泵 | 3100 | DoReadColor2D(1)(:962) | 子梯形 | 見 Scan 各相位 | 依 Scan | 不可 |
| Feed 送後手交位 | 4000 | MoveColorY(ColorTrayArmPickYPosition);到位清 bFrontHasTray(:972-975) | 馬達到位 | 到位單scan清latch | 冪等段 | 不可 |
| Feed 鬆夾 | 5000,6000 | Pop PushTray(:983)→Pop LeanOnTray(:988;5000 進入時 LeanOn 必仍夾) | 缸 reed | 無 | 冪等段 | 可收斂→7000 |
| Feed 呈盤commit | 7000 | 後感測缺→AMR等待/MES1421(:1005-1021);成功 bTrayReady=true+Status=CS_REAR_READY+bSupplyRequested=false(:1025-1030) | SnColor_OutputBottomHasTray | 單scan(:1025-1030) | 原子commit | 不可(材料呈遞latch) |
| Feed 終端 | 13000 | FeedTask=1;return true(:1036) | — | — | — | — |
| Scan 起點 | 1 | sim/tSimuData/bUseColorCcd=false→偽造 COLOR2D_hhnnsszzz+StampReadIdentity2D 即回(:1065-1068);否則 socket connect→10 | 組態 | sim 路徑單scan | 相位起點(出口原子commit) | — |
| Scan CCD X | 10 | MTopCCDX_Color->MotorMove(ColorRead2DXPosition)(:1089);NULL/超限→跳100 | 馬達到位 | 無 | 冪等段 | 不可 |
| Scan LON | 100 | 未連線→modal,K_SKIP→空ID stamp 即回(:1099-1102);連線→ColorCcdTriggerShot+ScanDelay 3s(:1109-1112) | socket | K_SKIP stamp 單scan | 冪等段 | 不可(通訊) |
| Scan 收碼 | 200 | 得碼→LOFF+stamp 即回(:1118-1125);逾時 modal:RETRY 重射(:1132)/K_MANUAL_2D 手輸 stamp(:1142)/SKIP 空ID stamp(:1149) | socket/ScanDelay | 各出口 stamp+return true 單scan | 原子commit | 不可 |
| GoDown 起點 | 1,10 | 模式檢查;bFrontHasTray→直接 DONE(冪等跳過;M1 誤判點)(:467-470) | SnColor_InputHasTray | 無 | 相位起點 | — |
| GoDown 分盤缸列 | 100→600 | Rise1↑(:481)Rise2↑(:489)Separate↑(:497)settle,Rise2↓(:515)settle,Separate↓(:533)settle,Rise1↓(:550)— 全缸+timer,零 Move(VERIFIED) | 缸 reed+GoDownDelay | 無 | 冪等段 | 可收斂→700 |
| GoDown 前確認+誕生 | 700,99999 | 感測缺→MES1424 retry(:575);成功 bFrontHasTray=true+BirthFrontTray()(:581-582 單scan);99999 歸1(:590) | SnColor_InputHasTray | :581-582 單scan | 原子commit | 可收斂→99999 |
| GoUp 起點 | 1,10 | 前無盤→跳1000(防空升砸疊,20260708)(:635-638) | SnColor_InputHasTray | 無 | 相位起點 | — |
| GoUp 前疊回缸列 | 100→600 | 上疊 choreography 全缸+timer;600 成功清 bFrontHasTray(:711-713) | 缸 reed+GoUpDelay | :713 單scan | 冪等段 | 可收斂→1000 |
| GoUp 後段派工 | 1000 | bRearHasTray→2000 否則 9000(:718-723) | 感測/latch | 無 | 相位起點 | — |
| GoUp 取後盤 | 2000 | MoveColorY(ColorTrayArmPickYPosition)(:729) | 馬達 | 無 | 冪等段 | 不可 |
| GoUp 夾後盤 | 3000,4000 | Push LeanOn(:737)→Push PushTray(:745) | 缸 reed | 無 | 冪等段 | 可收斂→5000 |
| GoUp 運回前位 | 5000 | MoveColorY(ColorReceiveTrayYPosition)(:753) | 馬達 | 無 | 冪等段 | 不可 |
| GoUp 放盤commit | 6000,7000 | Pop Push(:761)→Pop LeanOn(:769;6000 進入時 LeanOn 必仍夾);7000 bFrontHasTray=true+bRearHasTray=false(:771-772 單scan) | 缸 reed | :771-772 單scan | 冪等段(出口原子commit) | 可收斂→8000 |
| GoUp 終端 | 8000,9000,10000 | GoUpTask=1 return true(:788) | — | — | — | — |

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| W1 已呈盤未取(bTrayReady, DoColor 閒置) | InitialFlag 滅 bTrayReady/bRearHasTray/MMColorY->ClearTray/sTrayID2D(aColor.cpp:40-44,54);盤實體留後位(夾缸已在5000/6000鬆開,A1 不拖)。實機感測重推 bRearHasTray=true;TrayArm 輪詢重發 RequestSupplyTray(aTrayArm.cpp:524-527)→ DoFeedTray case 10 判剩盤 MES1426 操作員拆走(:895)。好盤降級成垃圾、識別滅失但重掃可重建;identity 盤無 IC(:1167)無料損。規劃五步不改此路(InitialFlag 照滅) | CF-1;黃 |
| W2 GoDown/GoUp 前缸列中(100-600) | uHome 不碰 Color 缸(共用情境)→ riser 半空持疊;M1(Empty 同機構、推及 Color,INFERRED)使 SnColor_InputHasTray 讀「有盤」→ GoDown case 10 跳過(:467)、Feed 去夾空氣 — 但 DoClampTray reed 感活塞位置非盤存在(mycylin.cpp:64-81;settle 預設500ms,GeneralSetting.cpp:52),無盤時活塞自由到底 reed ON → Clamp==1 照常通過、不觸發 MES1422;今日失效簽名後移為 ColorCCD_2D 掃描逾時 modal(:1127-1131)與 case 7000 後感測缺 MES1421(:997-1019;bUseAMR 先走 AmrFeedWaitTimer 等待),疊 strand 在 riser 上無專屬警示。規劃 drain 可整段收斂(零 Move,VERIFIED)即修;EMG/skip-drain 路徑仍裸奔 | CF-3;今日紅→drain後綠(EMG殘黃) |
| W3 Feed 1000(未夾,前往前位) | 盤仍在前 rest、感測重推 bFrontHasTray=true → 不重分盤;輪詢重發需求 → Feed 從1重走,夾到同一盤 | 綠(今日即重入) |
| W4 Feed 2000-4000 / GoUp 3000-5000(已夾運送中) | 夾缸不被 uHome 釋放 → MColorY 歸 home 把夾住的盤拖走(INFERRED 機構);InitialFlag 滅 fHasTray/latch,前後感測皆空 → DoColor 先分新盤(:354 分盤分支先於 :378 供料分支)→ Feed 帶舊盤開進前接盤位撞疊新盤,無 alarm。規劃 PARK/RE-ACQUIRE(MColorY 屬「載盤 MotorY」)解拖盤與定位,但 FeedTask 重啟自1仍會走「先分盤、再進前位」同一撞點 → 需續走閘 | CF-2;今日紅→規劃+續走閘後綠 |
| W5 Feed 5000/6000(後位鬆夾中) | 進入 5000 時 C_Color_LeanOnTray 必仍夾(其 Pop 在 case 6000 才發,:988),且 :983 Pop(PushTray) 未確認前兩缸可能皆夾 → 未達「兩缸皆 Off」(A1);uHome 不釋 Color 缸 → 今日 HOME 落此 MColorY 歸 home 把盤拖離後位(INFERRED 機構)= W4 拖盤家族,非「盤留後位」;「盤已在後位」僅在 6000 完成(=case 7000 邊界)後成立,屆時退化為 W1 型 MES1426。drain 可收斂→7000 邊界(純 Pop,VERIFIED)即除拖盤 | 併 CF-2 拖盤家族;今日紅→drain後轉 W1 同型黃(併 CF-1) |
| W6 Scan 3100 中(LON 已發未收碼) | 實體=W4(夾持中);另 ColorCcdEndShot 未發(:1109 後被斬)、ScanTask/sTrayID2D 歸零 — 重供料時 case 1 重連重掃可重建;但 K_SKIP/連線失敗會蓋空白 ID(:1099,:1149) | CF-5+CF-6;黃(實體歸 W4) |
| W7 回收已卸盤但 GoUp 未完(bTrayXToEmptyFinish 已立) | InitialFlag 滅 bReturnTray/bTrayXToEmptyFinish(:50-51);d63d33a 補發只在 arm 仍持盤時觸發(aTrayArm.cpp:1072-1093,VERIFIED)→ 無人補發,後盤淪 MES1426 剩盤操作員拆 | CF-4;黃 |
| W8 回收中 arm 仍持盤 | d63d33a:resume 依 PlaceDest 重發 ColorModule->RequestReturnTray(aTrayArm.cpp:1092-1093),冪等重臂 1700 收盤 | 綠(已修) |
| W9 GoUp 6000/7000(前位鬆夾中) | 盤已由 case 5000 運回前接盤位;6000 = PushTray Pop 中、LeanOn 必仍夾(其 Pop 在 7000 才發,:769),7000 = LeanOn Pop 未確認 → 今日 HOME 落此同屬拖盤家族:uHome 不釋缸,MColorY 歸 home 把盤拖離前位(INFERRED 機構)。drain 可收斂→8000(case 7000 僅 :771-772 latch 對調、純資料,VERIFIED)即修;EMG/skip-drain 殘留 | 併 CF-2 拖盤家族;今日紅→drain後綠(EMG殘黃) |

### 缺口

- CF-1(黃):ColorModule->InitialFlag() 無 bKeepMaterial(database.cpp:65-66;aColor.cpp:40-45,54)— HOME 把已呈盤好盤滅成 MES1426 剩盤、識別消失;修:仿 Loader rearready-p0(database.cpp:52-58 註)— 後感測確認在位時保留 bRearHasTray(+bTrayReady)與 MMColorY 格/sTrayID2D。
- CF-2(紅→規劃後黃):拖盤家族=凡任一夾缸未確認 Off 的段(FeedTask 2000-6000、GoUpTask 3000-7000)HOME 今日拖盤到 home;其中運送段(Feed 2000-4000、GoUp 3000-5000)重啟先分新盤再把舊盤開進前接盤位相撞(DoColor :354 先於 :378),鬆夾子段(Feed 5000/6000、GoUp 6000/7000)由 step-1 drain 收斂(→7000/→8000)即除;修:規劃 PARK/RE-ACQUIRE 明確納入 MColorY(釋 LeanOn→Push、記 Y+識別),並加續走閘:恢復後 fHasTray(保留)⇒ DoFeedTray 入口直跳 4000 送後呈盤、DoColor 在 fHasTray 期間抑制 destack 分支。實作注意:GoUp 回收段(2000-7000)全程無 MoveFrom 且 fHasTray=false(aColor.cpp:726-775 無識別上車),PARK「記 Y+識別」對回收中的 MColorY 會記到空識別 — 屬預期非缺陷(回收 identity 盤下次供料重掃,影響低)。
- CF-3(黃):GoDown/GoUp 前缸列(100-600)中斷 → riser 半空 strand,M1 讓 SnColor_InputHasTray 誤報有盤 → GoDown 跳過、Feed 夾空氣照過(DoClampTray reed 感活塞位置非盤,無盤活塞到底 reed ON→Clamp==1,mycylin.cpp:64-81;MES1422 只在 push reed settle 後未達才觸發)→ 今日失效簽名後移為 ColorCCD_2D 掃描逾時 modal(aColor.cpp:1127-1131)與 case 7000 MES1421(:997-1019),疊 strand 無專屬警示;修:規劃 step-1 DRAIN 直接覆蓋(兩梯形零 Move,VERIFIED 可收斂);EMG/skip-drain 路徑補一個 Color pre-home riser 歸位序(Pop 順序遵 M2:riser 靜止才動 pawl)。
- CF-4(黃):回收盤已卸(NotifyTrayXToEmptyFinish 已發 :1328-1332)但 GoUp 未完 → bReturnTray 被滅且 d63d33a 只治「arm 仍持盤」(aTrayArm.cpp:1072)→ 淪 MES1426;修:併入 CF-1 — bTrayXToEmptyFinish 已立即保留 bReturnTray,或 resume 時「後感測 ON 且非供料中」自動 re-arm case 1700 收盤。
- CF-5(低):HOME 誘發的重掃若走 K_SKIP/連線失敗會蓋空白 TrayID(aColor.cpp:1099,1149)→ AMR tray[0] 追溯洞;修:PARK 記住識別(規劃 step-2 本就記),重掃失敗時優先回填記憶值而非空字串。
- CF-6(低):HOME 落在 LON(:1109)與收碼(case 200)之間 → ColorCcdEndShot 未發,相機留在 shot 模式;修:HOME 前置清理對 ColorCcdSocket 冪等發一次 EndShot/LOFF。

### drain 邊界

Color 的可 drain 邊界(全部 VERIFIED 無 Move 呼叫):(a) DoGoDownTray — GoDownTask 在 100/150/200/300/350/400/450/500/600/700 任一點時,整段為 PushCylinder/PopCylinder + GoDownDelay settle + case 700 感測確認/BirthFrontTray(純資料),可一路收斂到 99999(完整分盤;注意 case 700 實機感測缺盤會彈 MES1424 modal,drain 實作應改為記錄+放行而非 modal)。(b) DoGoUpTray — 前疊回段 100→600 純缸,收斂邊界 = case 1000(必須停:case 2000 是 MoveColorY);若已在 3000/4000(夾後盤,馬達已過)可收斂到 5000 邊界停(5000 是 MoveColorY);若在 6000/7000 可收斂到 8000(case 7000 只是 latch 對調,無實體交付)。(c) DoFeedTray — case 2000(DoClampTray)純缸可收斂到 3000 邊界(出口的 MMColorY MoveFrom 是純資料,允許);case 5000/6000 純 Pop 可收斂到 7000 邊界後必須停 — case 7000 是 bTrayReady 呈盤 handoff commit(材料契約),留給 resume;case 1000/3000/4000 與 DoReadColor2D(ScanTask 10 有 MTopCCDX_Color MotorMove,100/200 是 socket)一律不可 drain,scan 中斷時僅做冪等 ColorCcdEndShot 清理。Drain 完成後 PARK 對 MColorY 的釋放序 = C_Color_LeanOnTray Pop → C_Color_PushTray Pop(與 A1 相容,兩缸皆 Off 後 Y 移動不拖盤)。

## TColorModule::DoGoUpTray + TColorModule::DoColor case 100/1700 (return-receipt) + RequestReturnTray/NotifyTrayXToEmptyFinish（黃）

> 覆核：ISSUES（已依覆核修正）。缺口代碼：CG-1, CG-2, CG-3, CG-4, CG-5, CG-6

Color 返還接收 = DoColor case 100 派工 (aColor.cpp:298-311) → case 1700 輪詢 DoGoUpTray (aColor.cpp:403-421);GoUp = 前站回疊(純缸 100-600)→ 後站夾持(缸 3000-4000)→ 前送搬運(馬達 5000,夾持載盤)→ 前站釋放(缸 6000-7000);所有 commit(:713、:771-772、:415-419)皆單掃描原子(VERIFIED)。今日 HOME 三重破壞:InitialFlag() 無 keep-material 全清(database.cpp:65-66、aColor.cpp:28-61,含 iReturnedCount=0 :52);uHome 只釋放 Loader Lean/Push(uHome.cpp:548-562),不釋放 C_Color_LeanOnTray/PushTray → GoUpTask 3000-6000 窗夾缸保持夾合,MColorY 歸原點(uHome.cpp:626;伺服已 home 無警報時被 :639 IsHomedServoSkippable 跳過)或 resume 後下一次 MoveColorY(如 aColor.cpp:910)拖著被夾盤 = 紅;交付已完成窗(TrayArm Job=NONE)d63d33a 補發不觸發 → 實機退化 MES1426 人工移除(aColor.cpp:891-903)。計畫 PARK(A1)正中紅窗,前提是 Color 車列入 tray-carrying MotorY 枚舉(CG-2);殘餘缺口皆可枚舉:keep-material 變體(CG-1)、GoUp 5000 中段 resume-at-phase(CG-3,雙感測盲區會誤結案)、1700 結案競態守門(CG-4)。Color 有 CCD:凡盤到達前站/疊回車上,identity 由下次 feed 重掃自然復原 → 資料損失面遠小於 Empty,故 yellow 而非 red。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| 返還派工 | DoColor case 100 (aColor.cpp:298-311) | 讀 bReturnTray;bAmrLocked 則退回 Task=1 暫緩 (:303-307);Status=CS_RETURNING;DoGoUpTray(0) 純重置 (:609-614);Task=1700 | bReturnTray latch(RequestReturnTray :1268-1272,由 aTrayArm.cpp:763/833/1093 設);bAmrLocked | 無 | 相位起點 | — |
| GoUp 入口閘 | GoUpTask 1,10 (aColor.cpp:622-641) | RefreshStateFromSensors;前站無盤→跳 1000(免空抬 20260708 gate) | SnColor_InputHasTray(實機 :195)/ladder latch(sim 早退 :185-186) | 無 | 相位起點 | 可收斂→1000(bFrontHasTray==false 直落 :635-637;有前盤則 10→100 (:640) 續經 100-600 純缸,安全結論不變) |
| 前站回疊抬升 | GoUpTask 100→600 (aColor.cpp:643-716) | Rise_1↑(110)→Separate↑(210)→settle→Rise_2↑(310)→Separate↓(410)→settle→Rise_2↓(510)→Rise_1↓(600);把前站盤疊回車上 | 各缸 reed(PushCylinder/PopCylinder,idempotent 快轉)+GoUpDelay settle | bFrontHasTray=false @600 (:713,單掃描) | 冪等段(自 case 1 重走收斂;M2:靜止中重插 Separate pawl 安全,VERIFIED 純缸無 Move) | 可收斂→1000 |
| 後站調度 | GoUpTask 1000 (aColor.cpp:718-724) | 感測刷新;bRearHasTray→2000,否則→9000 | 實機 SnColor_OutputBottomHasTray/TrayPos1 (:207-222);sim 早退 (:185-186) 後 bRearHasTray 純為 Notify/InitialFlag latch(:223-232 bTrayReady fallback 為不可達死碼) | 無 | 相位起點 | —(本身即邊界;下一相位含馬達) |
| 後站接近 | GoUpTask 2000 (aColor.cpp:726-734) | MoveColorY(Teach.ColorTrayArmPickYPosition)+Lean.Reset;內建 TrayArm 防撞等待 (:808-820,實機) | 馬達到位+防撞判斷 | 無 | 冪等段(Move 可重發;夾未合,Y 動不拖盤=A1) | 不可(含 MoveColorY) |
| 後站夾持 | GoUpTask 3000-4000 (aColor.cpp:736-747) | LeanOnTray(後推)Push→PushTray(前擋)Push,夾住後站盤 | 兩缸 reed | 無 | 冪等段(重 Push 快轉確認) | 可收斂→5000(純缸;之後交 PARK 依 A1 釋放) |
| 前送搬運 | GoUpTask 5000 (aColor.cpp:749-758) | 夾持載盤 MoveColorY(Teach.ColorReceiveTrayYPosition)+Push.Reset | 馬達到位 | 無 | 危險段(正確性依賴前綴之「夾已合」;夾被 PARK 釋放後回存 5000 會空跑丟盤) | 不可(馬達+載盤搬運) |
| 前站釋放 | GoUpTask 6000-7000 (aColor.cpp:760-775) | Pop PushTray→Pop LeanOnTray,盤落前站 | 兩缸 reed | bFrontHasTray=true + bRearHasTray=false @7000 (:771-772,單掃描) | 冪等段(重 Pop 快轉;prefix 保證車已在前站 Y) | 可收斂→10000 |
| 終端 | GoUpTask 8000,9000,10000 (aColor.cpp:777-789) | 空步鏈;10000 設 GoUpTask=1 並 return true(single-terminal 供 IsCleanOutFinish 閒置閘 :1309) | — | 無 | 冪等段 | 可收斂→10000 |
| 收據結案 | DoColor case 1700 (aColor.cpp:403-421) | 每掃描步進 DoGoUpTray(1);回 true 且 bReturnTray&&!bTrayXToEmptyFinish 則 hold 續輪詢 (:413-414,約 5 掃描/圈:1→10→1000→9000→10000);否則結案 | bTrayXToEmptyFinish(NotifyTrayXToEmptyFinish :1328-1332,SOLE trigger,由 aTrayArm.cpp:1045 於 DoPlaceToColor case 4000 呼叫) | bReturnTray=false + iReturnedCount++ + iSimInfeedCount++ + Status + Task=1 (:415-419,單掃描,VERIFIED 一次 DoColor 呼叫內完成) | 原子commit | — |

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| W1 派工前/入口(DoColor 100、GoUp 1/10;TrayArm 尚持盤) | InitialFlag() 全清(database.cpp:65-66→aColor.cpp:28-61),但 TrayArm InitialFlag(bKeepMaterial=true) 保留 Job/盤;resume 於 DoTrayArm case 100 補發 ColorModule->RequestReturnTray (aTrayArm.cpp:1092-1093,d63d33a) → 握手重簽,全程重走 | 已由 d63d33a 修復;綠 |
| W2 前站回疊中(GoUp 100-600) | Rise_1/Rise_2/Separate 凍結於半抬(uHome.cpp 無任何 C_Color_* 操作,VERIFIED grep;InitialFlag 亦不碰缸);GoUpTask 清 1。有待返盤→補發後重走,冪等收斂(M2;Color 前感測抬升中仍讀有=INFERRED,U4 同 Empty 機構之 M1);交付已完成→無人重啟 GoUp,伸出缸令 IsReadyForAmrHandoff()=false (aColor.cpp:101-103) 擋 AMR 至下次 GoDown/GoUp | 黃(CG-6);計畫 DRAIN 收斂至 1000 邊界後→綠 |
| W3 交付已完成、盤在後站(GoUp 1000/2000,或 Notify 後未起步) | TrayArm place 已 return true(aTrayArm.cpp:1050)→Job=NONE→補發不觸發;bReturnTray 被清;實機後感測重推 bRearHasTray=true→下次供料 DoFeedTray case 10 MES1426 人工移除 (aColor.cpp:891-903);sim 因 RefreshStateFromSensors 早退(aColor.cpp:185-186,IsSoftSimulate :149-156 含 SOFT_SIMULATE 與實機 DUMMY)無任何感測重推,bRearHasTray 純為 Notify/InitialFlag latch,HOME wipe 後盤靜默消失(:223-232 bTrayReady fallback 為不可達死碼)。Y 歸原點本身安全(夾未合,A1) | 黃(實機退化人工、identity 可由操作員放回車上經 CCD 重掃復原);sim 紅(靜默);→CG-1 |
| W4 後站夾持已合(GoUp 3000-4000 完成後~5000 前) | uHome 只 Pop Loader1/2 Lean/Push(uHome.cpp:548-562),C_Color_LeanOnTray/PushTray 保持夾合;case 200 陣列含 MColorY(uHome.cpp:626),但 :639 IsHomedServoSkippable 跳過已 home 且無警報之伺服 → 實際歸零拖盤僅於 stepper/警報/未 home 組態;即便被跳過,夾缸未釋放,resume 後下一次 MoveColorY(如 DoFeedTray case 1000, aColor.cpp:910)仍拖夾持盤,卡/撞風險(A1 反面) | 紅(今日);計畫 PARK 正中此窗,前提 Color 車列入 tray-carrying MotorY 枚舉→CG-2 |
| W5 前送搬運中(GoUp 5000,盤夾持於軌道中段) | 同 W4 拖盤(紅)。計畫下:PARK 釋放+記憶 Y 後歸原點安全,但 step 5 重走 GoUpTask=1→case 10 前空、case 1000 後空(盤在中段,雙感測皆盲)→GoUp 空跑完成→1700 見 bTrayXToEmptyFinish=true 誤結案,盤遺留軌道中段且 iReturnedCount 虛增 | 紅(今日)/黃(計畫仍缺 resume-at-phase)→CG-3 |
| W6 前站釋放中(GoUp 6000-7000) | Push 已放、Lean 仍夾(6000/7000 之間)→局部拖曳,同 W4;兩缸皆放(7000 commit 後)→盤已落前站,實機 SnColor_InputHasTray 重推 bFrontHasTray→正常 destack/feed+DoReadColor2D 重掃 identity→自癒 | 半放窗:紅(併入 CG-2 PARK);7000 後:綠 |
| W7 終端/結案(GoUp 8000-10000、DoColor 1700) | 純資料;commit 單掃描原子,要嘛跑完要嘛沒跑;沒跑到僅損 iReturnedCount(診斷)與 iSimInfeedCount(sim 帳) | 綠(CG-5 診斷微損) |
| W8 結案競態(非 HOME 專屬,輪詢盲區) | 交付 Notify 落在輪詢圈 GoUpTask∈{9000,10000}(後檢查僅在 case 1000)→下次 DoGoUpTray(1)==true 時 bTrayXToEmptyFinish 已 true→盤仍在後站即結案;實機因 case 1000 讀感測且感測領先 Notify(釋放先於 PlaceTask 4000)幾乎不中,sim/感測停用組態會中 | 黃→CG-4 |

### 缺口

- CG-1(黃): ColorModule->InitialFlag() 無 bKeepMaterial 參數(database.cpp:65-66;aColor.cpp:28-61)——HOME(含計畫 step 5)清 bReturnTray/bTrayXToEmptyFinish/bRearHasTray/GoUpTask,交付已完成之返還(TrayArm Job=NONE)無人重派;sim 更因 RefreshStateFromSensors 早退(aColor.cpp:185-186)無感測重推 latch,盤靜默消失;修法:加 keep-material 變體保留三 latch,resume 時 DoColor case 100 即自動重派 1700 完成後段搬運。
- CG-2(紅,由已核准計畫解): uHome 從不釋放 C_Color_LeanOnTray/PushTray(uHome.cpp 僅 :548-562 Loader),GoUp 3000-6000 間 HOME 後夾缸保持夾合——MColorY 在 case 200 陣列(uHome.cpp:626),歸零拖盤觸發於 stepper/警報/未 home(:639 IsHomedServoSkippable 跳過已 home 無警報伺服);即便跳過,resume 後任一 MoveColorY(如 aColor.cpp:910)仍拖夾持盤;修法 = PARK 枚舉必須含 Color 車(對稱 Empty/Loader),釋放序 Lean→Push 依 A1。
- CG-3(黃): GoUp 5000 中段中斷之恢復需 resume-at-phase——re-acquire 重夾後須回存 GoUpTask=5000(並保留 bReturnTray),否則梯形自 1 重走落入雙感測盲區(前空+後空)而於 1700 誤結案、盤遺留中段(aColor.cpp:718-724 + :413-419)。
- CG-4(黃): case 1700 結案守門不足(aColor.cpp:413-414 只查 bTrayXToEmptyFinish):Notify 落在輪詢盲區 GoUpTask∈{9000,10000} 時盤仍在後站即 commit(iReturnedCount 虛增,後續 DoFeedTray case 10 MES1426);修法:結案再加 bRearHasTray==false,不成立則 DoGoUpTray(0) 重派再搬一輪。
- CG-5(綠/瑣碎): iReturnedCount 每次 InitialFlag 歸零(aColor.cpp:52),現僅 State Record 診斷(:1593);keep-material 變體應一併保留,否則 HOME 後 dump 之返還史失真。
- CG-6(黃): 前站回疊中斷後 Rise/Separate 凍結伸出且無復位路徑→IsReadyForAmrHandoff()=false(aColor.cpp:101-103)長期擋 AMR CEID273 READY;計畫 DRAIN(100-600→邊界 1000)消除主窗;殘餘保險:resume 時若 GoUpTask/GoDownTask 皆閒而缸伸出,跑一次退缸序(冪等,M2 安全)。

### drain 邊界

HOME drain 階段對本梯形之可收斂邊界(全部 VERIFIED 無 Move/真空):GoUpTask 100-600(含 GoUpDelay settle)僅 PushCylinder/PopCylinder+timer(aColor.cpp:643-716)→ 收斂至 GoUpTask==1000(三缸全回 home、疊盤落定於固定托爪,前站旗 @600 單掃描落定);GoUpTask 3000-4000(Lean/Push 進夾,aColor.cpp:736-747)→ 收斂至 GoUpTask==5000(盤夾定於後站 Y、車靜止),之後 PARK 依 A1 先放 Lean 再放 Push 並記憶 MColorY 位置(identity 可不記——Color 下次 feed 由 CCD 重掃補回);GoUpTask 6000-7000(放夾,aColor.cpp:760-775)→ 收斂至 GoUpTask==10000(8000/9000 為空步;7000 的 bFrontHasTray=true/bRearHasTray=false 為缸完成後單掃描資料 commit,盤已安置前站=安全靜止面),若再允許一掃 DoColor case 1700 之純資料結案(:415-419)更完整,但現行 InitialFlag 仍會歸零 iReturnedCount(CG-5)。不可收斂:GoUpTask 2000 與 5000 皆含 MoveColorY(:729/:753)——2000 由「夾未合不拖盤(A1)+後站感測重推」自然安全,5000 必須交 PARK+resume-at-phase(CG-3)。GoUpTask 1/10/1000/8000/9000/10000 與 DoColor case 100/1700 本身即相位起點或原子 commit,無需 drain。

## TLoaderModule::DoFeedTray + TLoaderModule::DoCcdCheck (CcdTask) + TLoaderModule::FindNextCcdCell（黃）

> 覆核：ISSUES（已依覆核修正）。缺口代碼：LF-1, LF-2, LF-3, LF-4, LF-5, LF-6, LF-7

1. CCD 掃描梯天然可重入 (VERIFIED):游標不是狀態、是由 Tray grid 重推 — DoLoader case 100 的 fHasTray 分叉 (aLoader.cpp:1194-1228) 有盤即走 LS_CCD_SCAN,DoCcdCheck case 2000 的 FindNextCcdCell (aLoader.cpp:1005-1051) 掃 grid 找第一個 UNCHECK_IC,ResetSide 清掉的 CcdX/CcdY/bCcdLeftToRight 全部無害。
2. 今日唯一擋路者是 uHome 自己:case 50/60 無條件開夾 (uHome.cpp:548-563) + case 200 見 fHasTray 即強制人工移除+ClearTray (uHome.cpp:659-668) 把可續掃的 grid 全滅。計畫協定 (PARK/RE-ACQUIRE + 免 ClearTray) 落地後,半掃盤自動續掃,幾乎零改碼。
3. 真正的紅區在進料梯的「未鑄造視窗」:FeedTask 4100..9500 之間盤已實體落在車上但 fHasTray=false (confirm-then-mint, aLoader.cpp:1546),HOME 完全看不見它 → 無提示遺留於送料位,復歸再 destack 疊盤 (危害已被 aLoader.cpp:1514-1517 註解自證)。修法 = drain 收斂到 9500 mint,或 3500 前加 SnLoader_InputHasTray 自收 (現成樣板 aLoader.cpp:1482-1487)。
4. 次要缺口:2D 模式下 cell 的 Data(case 5000, :1659) 與 Bin(case 5500, :1732) 跨兩相位寫入不原子 → 計畫保留料後會出現「無 bin 的 HAS_OK_IC」被 FindNextCcdCell 跳過;以及 InitialFlag 重置 iFeedSerial/iCarTrayTotal (aLoader.cpp:65-66,95) 使 AMR 模式 HOME 後 cover/identity 盤誤判為 Normal。
5. 結論 yellow:骨架 (fHasTray 分叉 + grid 游標重推 + 9500/5500 單掃描 commit) 已備,缺的是可枚舉的小修 (LF-1~LF-4/LF-6),不需新的機構決策 (A1 已涵蓋 Loader 盤不隨車拖動)。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| 外梯調度 (fHasTray 分叉) | DoLoader 1/10/100 (aLoader.cpp:1183-1228) | 無盤→DoFeedTray(Flag=0)+Task=1000;有盤→Status=LS_CCD_SCAN+DoCcdCheck(0)+Task=2000 (既有續掃路徑, VERIFIED) | TrayMotor->fHasTray(:1195)、bAmrLocked(:1197)、OtherState->Status 前區閘(:1211) | 僅 Task/Status 轉移,無實體動作 | 相位起點 | — |
| 外梯輪詢 | DoLoader 1000/2000/3000/4000 (:1231-1280) | 輪詢 DoFeedTray/DoCcdCheck/DoDischargeTray;3000 以 ActiveTrayAllData(EMPTY_IC) 判轉卸盤 | 內梯回傳、Tray grid(:1257)、iYOwner(:1255) | Status 轉移 | 相位起點 | — |
| 進料就位 | DoFeedTray 1/10/100/1000 (:1335-1362) | 10=fHasTray 短路直接 return true(:1340-1343);100=AcquireFrontOwner(:1349);1000=MoveLoaderY→feed Y(:1355) | fHasTray、iFrontOwner(:886-897)、馬達到位 | FeedTask 前進 | 冪等段 (10=短路;Move 可重發) | 不可 (1000 含馬達) |
| 夾爪預開 | 2000/3000 (:1364-1372) | PushCylinder.Pop→LeanCylinder.Pop (車上尚無盤) | 汽缸 reed (TMyCylinder 冪等快進) | FeedTask=3500 | 冪等段 | 可收斂→3500 |
| 料源預檢 | 3500 (:1374-1395) | SnLoader_Inputend 有料→4000;乾→9000 | sensor (Enable 閘) | 單掃描分路 | 相位起點 | — |
| 前分盤 destack | 4000/4100=DoFrontDestackDown 1-7 (:1397-1408, 2029-2097) | Rise1↑,Rise2↑,Separate↑(1s settle;Delay.Set(10) :2058,HTimer::Set 單位=100ms tick, HTimer.cpp:16),Rise2↓,Separate↓(需 Rise1 On :2080),Rise1↓ — 實體落一盤到車上 | 汽缸 reed + HTimer + Loader<->Empty separate 互鎖(:2054) | FeedTask=8200 | 危險段 (中途以 DestackTask=1 重進 = 對已分/已落之盤重分重夾, :1514-1517 自證) | 可收斂→8200 |
| 夾盤 | 8200/8300 (:1410-1418) | LeanCylinder.Push→PushCylinder.Push 夾住新盤 | 汽缸 reed | FeedTask=9000 | 冪等段 | 可收斂→9000 |
| 到位判定 | 9000 (:1420-1531) | Inputend+push-sensor 交叉檢(:1428,:1457)、MES0921/MES0920、AMR 600s(可調 iAmrFeedWaitSec, :1502) 延遲(:1498-1511)、CleanOut 前盤自收(:1482-1487) | sensors + FeedWaitTimer + modal Note | 分路→9500 | 相位起點 (但含 modal 告警) | 可收斂→9500 (需抑制告警之 drain 變體;presence 不成立則止於 9000=人工 fallback) |
| 身份鑄造 | 9500 (:1533-1575) | SnLoader_InputHasTray 確認(:1542-1544)→fHasTray=true+PrepareTrayMap+iFeedSerial+++SetKind/TrayID 全在同一 case 執行 (:1546-1562, 單掃描 VERIFIED) | SnLoader_InputHasTray;失敗=JAM0913 | 單掃描 mint (confirm-then-mint) | 原子commit | — (純資料) |
| 進料收尾 | 10000 (:1577-1580) | ReleaseFrontOwner, return true | — | FeedTask=1 | 冪等段 | 可收斂→1 |
| CCD 調度/游標重推 | DoCcdCheck 1/1000/2000 (:1611-1638) | HasActiveTrayData(UNCHECK_IC)(:1615);FindNextCcdCell 由 grid 蛇行重建 CcdX/CcdY(:1634, 1005-1051);無 UNCHECK→LS_READY_SORT(:1626-1628) | Tray.Data grid (持久,InitialFlag 不清) | CcdX/CcdY 更新 | 相位起點 | — |
| CCD 移位 | 3000/4000 (:1640-1652) | MoveToCcdCell=TopCCDX+LoaderY(:1641, 432-442)+100ms settle | 馬達到位 + CcdDelay | CcdTask=5000 | 冪等段 (可重發;中斷之 cell 仍 UNCHECK→復歸自動重找) | 不可 (馬達) |
| CCD 讀取 | 5000 (:1654-1711) | ReadTopCcdBin→SetTraySingleData(:1659) 單掃描;2D 模式 TriggerShot→5500(:1686-1689);否則→1(:1692) | CCD API(:1053-1063 真機為 stub, bOk=false→WAR0330) | Data 單掃描已寫,Bin 尚未寫 | 原子commit (惟需與 5500 成對才是完整 cell commit) | — |
| 2D 判讀 commit | 5500 (:1713-1811) + BindManual2D(:1816-1887) | 2D poll→SetTrayBin/Lot/Code2D/PassClass+ResolveAuto+OnSorted+BinICCnt+iTotalSorted 同一掃描 (:1730-1760, VERIFIED)→CcdTask=1 | TopCcd socket + 3s CcdDelay + LotRegistry | 成功路徑單掃描 | 原子commit | 可收斂→1 (純 socket/timer、相機已拍且 drain 先於馬達 home;告警支須改為「cell 回填 UNCHECK_IC」而非 modal) |

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| 已鑄造盤在車上 (DoLoader 100/2000、CcdTask 1000-4000、掃畢待 sort) | InitialFlag 重置 FeedTask/CcdTask/CcdX/CcdY/Status(:126-144)+iFrontOwner/iYOwner;uHome case50/60 開夾(:548-563)、case200 home 車後見 fHasTray→強制人工移除全部盤+ClearTray(uHome.cpp:659-668)=半掃 grid/bin 全滅 | 黃(今日:安全但料被逐出、掃描資料全失)→綠(計畫後):免 ClearTray+PARK/RE-ACQUIRE,case100 fork 走 LS_CCD_SCAN,FindNextCcdCell 由 grid 續掃;bCcdLeftToRight 重置僅多一趟行程 (效率, INFERRED 無正確性損失) |
| Feed 1/10/100/1000/2000/3000/3500 (車就位、夾爪開,無盤) | 車被 home、夾爪被重放 (冪等)、無料損失;復歸從頭進料 | 綠 |
| Feed 4100 destack 進行中 | uHome 不碰 Rise1/Rise2/Separate 三缸 (只碰 Lean/Push);DestackTask=1 被重置;fHasTray=false→case200 無提示;復歸後全新 destack 對半分離狀態重跑→重夾/割盤/散 IC (aLoader.cpp:1514-1517 已知危害) | 紅(今日)。計畫 drain 收斂至 8200 即修復 (純汽缸+1s timer,無馬達無真空);EMG/安全門跳過 drain 時 fallback 必須指定人工清除 |
| Feed 8200/8300/9000 (盤已在車上、未鑄造) | case50/60 把剛夾上的夾爪再開 (Pop 冪等反轉)、case200 home 車 — 依 A1 盤留在送料位軌上;fHasTray=false→case200 偵測不到、無任何提示;復歸進料在同位置再落一盤疊上→撞盤 | 紅(今日)。修 = drain 通到 9500 mint 使 PARK 看得見它;過渡修 = 3500 destack 前加 SnLoader_InputHasTray 已有盤→改走 9500 (現成樣板 :1482-1487 CleanOut 自收,但 today 僅 CleanOut 生效) |
| CcdTask 5000→5500 之間 (2D 模式已寫 Data 未寫 Bin) | 今日被強制移除+ClearTray 遮蔽,無症狀 | 黃(計畫後浮現):cell Data=HAS_OK_IC(:1659) 已寫、iBin=0=未指派(MyMotor.cpp:53)、iLot=-1;復歸後 FindNextCcdCell(:1039) 只找 UNCHECK_IC→跳過→無 bin IC 交給 SortArm 誤 routing/漏計。修見 LF-3 |
| AMR 供料車進度 (任何視窗) | InitialFlag 先清 iSecsCarTrayCount(:65) 再 RefillSimInfeed(:66)→iCarTrayTotal 退回 iSimAmrMaxTray、iFeedSerial=0(:95,:559)→GetFedTrayKind(:578-592) 依序號誤標,car 末兩盤 (cover/identity) 被當 Normal misroute | 黃:keep-material 復歸應保留 iFeedSerial/iCarTrayTotal/iSecsCarTrayCount (HOME 不改變實體 car 內容, A3) |
| CcdTask 5500 等 2D 回應中 | 同列1 (強制移除遮蔽);計畫後:cell 已進 5000 寫 Data 之 non-atomic 視窗,同上 | 黃 (併入 LF-3;或 drain 5500 至完成) |

### 缺口

- LF-1 [紅] 未鑄造視窗 (FeedTask 4100 落盤後..9500 mint 前) 盤在車上但 fHasTray=false (confirm-then-mint, aLoader.cpp:1546),uHome case200 的移除提示只看 fHasTray(uHome.cpp:659-661)→盤無聲遺留送料位,復歸再 destack 疊盤;最小修:HOME drain 將 FeedTask 收斂到 9500 完成 mint,過渡修 = case 3500 destack 前加 SnLoader_InputHasTray 已有盤檢查改走 9500 (仿 :1482-1487)。
- LF-2 [紅] DoFrontDestackDown (aLoader.cpp:2029-2097) 中途被 DestackTask=1 重置後重進 = 對已分離之盤重夾/重落 (危害自證於 :1514-1517);純汽缸+timer 相位,計畫 drain 可收斂→FeedTask 8200;EMG 跳過 drain 之 fallback 需明文 = 人工清除前分盤區。
- LF-3 [黃] CCD cell commit 跨兩掃描不原子:Data 在 case 5000(:1659)、Bin/Lot/2D 在 case 5500(:1730-1760),中斷產生 Data=HAS_OK_IC+iBin=0(MyMotor.cpp:53) 的殘 cell,FindNextCcdCell(:1039) 跳過→誤 routing;修:2D 模式下把 SetTraySingleData 延至 5500 成功路徑一起寫,或復歸時將 Data>UNCHECK_IC && iBin==0 的 cell 正規化回 UNCHECK_IC 重掃。
- LF-4 [黃] InitialFlag 清 iSecsCarTrayCount(:65) 後呼叫 RefillSimInfeed(:66,:549-560) 改寫 iCarTrayTotal 並 iFeedSerial=0(:95)→AMR 模式 HOME 後 GetFedTrayKind(:578-592) 把 car 末端 cover/identity 盤誤標 Normal;修:keep-material 路徑保留 iFeedSerial/iCarTrayTotal/iSecsCarTrayCount 三值。
- LF-5 [綠/VERIFIED 無需修] ResetSide 清 CcdX/CcdY/bCcdLeftToRight(:135-137) 無正確性影響 — 游標由 FindNextCcdCell(:1005-1051) 從持久 Tray grid 全量重推,方向重置僅損一次行程效率。
- LF-6 [黃] 拆除 uHome case200 強制移除+ClearTray(:659-668) 必須與 RE-ACQUIRE 同批落地:只刪 ClearTray 而無再取回,fHasTray/grid 指向一個未夾持、停在原 Y 軌上的盤,case100 fork 會直接開掃並以空車移向 CCD 位 (盤與資料脫鉤)。
- LF-7 [黃/INFERRED] InitialFlag 清 bAmrLocked(:64) — AMR 交接中發生 HOME 後 case100 的 bAmrLocked 閘(:1197) 失效,可能在 AMR 佔位時啟動前段進料;需 AGV 協調器於 keep-material 復歸時重掛鎖 (未驗證協調器是否已有此行為)。

### drain 邊界

Loader 進料梯的 HOME drain 邊界 (全部 VERIFIED 無馬達 Move、無真空):FeedTask 2000/3000 (Push.Pop/Lean.Pop, aLoader.cpp:1365,1370) 可收斂→3500;4000/4100 的 DoFrontDestackDown 子梯 1-7 (:2029-2097) 為純汽缸+單一 1s settle timer (Delay.Set(10) at aLoader.cpp:2058;HTimer::Set 單位=100ms tick, HTimer.cpp:16 iTimeLen=iTime*100,對照 SetMS HTimer.cpp:27-30),可收斂→8200 — 注意 subtask 4 的 Loader<->Empty front-separate 互鎖(:2054) 在全機 drain 時依賴 Empty 側先收回其 separate 缸,drain 排程須讓 Empty destacker 先收斂或靠 timeout fallback;8200/8300 (Lean.Push/Push.Push, :1411,1416) 可收斂→9000;9000→9500 無實體動作但含 modal 告警 (MES0920/0921) 與 AMR 等待 timer (600s 為 HT9046 移植預設值,可調 iAmrFeedWaitSec, :1502),drain 須以「抑制告警、僅做 presence 判定」變體執行:presence 成立則進 9500 完成單掃描 mint (:1546-1562, 使 PARK 能看見此盤),不成立則止於 9000 = 人工 fallback。FeedTask 1000 (MoveLoaderY, :1355) 含馬達,不可 drain,但該點車上尚無盤,直接重置安全。CCD 梯 (CcdTask) 不需也不可整段 drain:3000 含 MoveToCcdCell 馬達 (:1641),而中斷的 cell 保持 UNCHECK_IC 天然可重掃;唯一例外是 2D 視窗 5000→5500 — 若不做 LF-3 的原子化修正,可把 5500 (純 socket poll + 3s timer,相機在 drain 階段仍停在該 cell 上因 drain 先於馬達 home) drain 至 CcdTask=1,且其告警分支在 drain 情境須改為將該 cell 回填 UNCHECK_IC 而非跳 modal。

## TLoaderModule::DoDischargeTray (dispatcher: TLoaderModule::DoLoader case 3000/4000)（黃）

> 覆核：OK。缺口代碼：LD-1, LD-2, LD-3, LD-4, LD-5

DoDischargeTray 搬的是「已排完」的來源盤:DoLoader case 3000 只在 ActiveTrayAllData(EMPTY_IC)==true 時才派工(aLoader.cpp:1257,985-1002),故本梯形全程無 IC、無真空 — HOME 落在任何相位都不會損料,最壞只是多一次人工移盤(VERIFIED)。發布點=case 4000:空車退回 feed Y 到位的同一掃描 bRearReadyForPick=true + bRearDischargeInProgress=false(aLoader.cpp:2005-2008),是 TrayArm pick gate 的唯一 setter。中途盤「刻意不保留」:InitialFlag 的 bKeepRear 只認 bRearReadyForPick&&sensor(aLoader.cpp:79-87),abort 殘盤身份不明、自動回收有 cover/identity 盤誤入普通池之險,故走 MES0924 人工移除(aLoader.cpp:76-78,1125-1141)— 但 3000 之後的視窗身份其實已移交,此保守規則在該窗多付一次人工(LD-1,主要缺口)。drain 可收斂段=Task 2000/3000(純缸+單掃描移交)至 4000 邊界;1000/4000 含 MoveLoaderY 不可 drain,1000 交 PARK、4000 由 home 退車物理等效完成。uHome case 200 強制移全部 Loader 盤+ClearTray(uHome.cpp:659-668)必須在 PARK 成功時跳過(LD-3),否則 Loader 的 PARK 白做。判定 yellow:缺口可枚舉、皆小修,無需機構決策。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| 入口/重置 | Flag==0 / case 1 | Task=1;case 1 僅步進→10,無實體動作 (aLoader.cpp:1901-1905,1924-1926) | — | — | 相位起點 | — |
| 起步門 | 10 | 讀兩車 encoder(MLoaderY_1/2 ReadEncoderPos)+ IsOutputBottomOccupied();他車在我後方或後座有盤→return false 純等待 (1928-1942) | encoder 原始值 + SnLoader_OutputBottomHasTray | — | 相位起點 | — |
| 承諾點 | 100 | gate IsRearOccupied()(會 RefreshRearState);通過即同一掃描 arm bRearDischargeInProgress=true、殺 stale bRearReadyForPick=false、Task=1000 (1944-1959) | 後座 sensor(經 refresh) | 單掃描旗標 arm(1957-1958,VERIFIED 同掃描) | 原子commit | —(單掃描,無可收斂物;不得由 drain 前推——下一步即馬達) |
| 載盤赴後 | 1000 | MoveLoaderY→GetLoaderDischargeY(馬達,載盤);到位後 real 機檢 SnLoader_OutputBottomHasTray OFF→modal ShowMyError K_RETRY/K_SKIP(訊息文字寫 IC、實為落地確認,INFERRED);過檢後 Reset 兩缸、Task=2000 (1962-1979) | 馬達到位 + 後座 sensor ON(落地時 RefreshRearState 已 re-latch bRearHasTray,1948-1951) | — | 冪等段(MoveLoaderY 可重發;modal 重查) | 不可(motor Move) |
| 卸盤-前擋釋放 | 2000 | PushCylinder->Pop()(前擋 off-reed 確認);confirm 掃描 bRearHasTray=true(sim/DUMMY 落地 latch)、Task=3000 (1980-1986) | Push off-reed | bRearHasTray=true(1983,單掃描) | 冪等段(已達缸 Pop=快轉確認) | 可收斂→3000 |
| 卸盤-後鉤釋放+身份移交 | 3000 | LeanCylinder->Pop();confirm 掃描:RearKind/RearTrayID/RearSourceTray←Tray、TrayMotor->ClearTray()、Task=4000 全在同一掃描 (1988-2000,VERIFIED 1994-1998 同掃描) | Lean off-reed | 原子身份移交+清車 latch(fHasTray=false 自此) | 原子commit(掛在缸 confirm 上;重入僅能經 2000 前綴) | 可收斂→4000邊界 |
| 空車退+發布 | 4000 | MoveLoaderY→GetLoaderFeedY(空車,馬達);到位掃描:bRearDischargeInProgress=false、bRearReadyForPick=true、Task=1、return true 全同掃描 (2002-2010,VERIFIED 2005-2008) | 馬達到位 | 發布點:bRearReadyForPick 唯一 setter(全檔僅 2006 一處置 true,VERIFIED) | 冪等段(空車移動可重發;發布為完成掃描一次性) | 不可(motor Move;但 home 退車物理等效,見 LD-1) |

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| Task 1/10/100(盤仍夾於車上,未動身) | InitialFlag→ResetSide 清 DischargeTask=1、無條件清 bRearDischargeInProgress(aLoader.cpp:88,132);uHome case 50/60 無條件 Pop 雙缸(uHome.cpp:546-563)→盤失夾停原 Y(A1 不拖);case 200 home 後 fHasTray=true→強制操作員移全部 Loader 盤+ClearTray(uHome.cpp:659-668)。盤為已排完空盤,無 IC 損失 | 黃:安全但人工。計畫協定後:PARK 記 Y+身份→home→RE-ACQUIRE;梯形重啟後 DoLoader case 100 以 fHasTray=true 走 CCD 重掃→all EMPTY_IC→重派 discharge(aLoader.cpp:1194-1273),收斂但多一輪掃描;且需 LD-3(跳過 uHome 強制移盤) |
| Task=1000(載盤移動中) | 同上——盤 stranded 於中途 Y,操作員移除 | 黃:不可 drain(馬達段);純 PARK 情境。恢復路徑同上,收斂 |
| Task=2000/3000(已落地後座,缸釋放中) | 盤已在後座(sensor ON);uHome 50/60 放缸=冪等快轉無害;3000 confirm 前 fHasTray 仍 true→uHome 操作員提示涵蓋(但提示文字只說 Loader L/R 盤,後座盤易漏,belt=MES0924);InitialFlag bKeepRear=false(readiness 未發布)→wipe hold(aLoader.cpp:79-87)→sensor 再 latch→MES0924 一次性 Note(1132-1142,real 機限定) | 黃:此段為 drain 甜蜜點——純缸+單掃描移交,可收斂至 4000 邊界(LD-2);收斂後仍需 LD-1 的 resume 發布,否則照走 MES0924 |
| Task=4000(空車退回中,發布未達) | fHasTray=false→uHome 不提示;盤靜置後座、身份已在 RearKind/RearTrayID/RearSourceTray;InitialFlag 因 latch=false 丟棄整個已知 hold→MES0924 要求操作員移除一張其實已知、已落座、缸已 off 的盤 | 黃:主要缺口 LD-1——home 的空車退場即物理等效 case-4000 退車,只缺發布掃描;最小修即補此發布 |
| 發布後(bRearReadyForPick=true 已 latch) | InitialFlag bKeepRear=bRearReadyForPick&&IsOutputBottomOccupied()=true→整組 rear hold 保留(aLoader.cpp:69-87)→TrayArm 依 IsRearReadyForPick(741)正常回收 | 綠:既有 rearready-p0 修復已使此窗重入(VERIFIED) |

### 缺口

- LD-1(黃,主缺口):HOME 落在 3000 confirm 後~4000 發布前:身份已移交(1994-1996)、雙缸 off-reed 已確認、盤已落座 sensor ON,但 bRearReadyForPick 未發布→InitialFlag 丟棄已知 hold→MES0924 人工移除。最小修:bKeepRear 判定(aLoader.cpp:79)在 line 88 清 bRearDischargeInProgress 之前放寬——bRearDischargeInProgress==true 且身份移交完成(RearSourceTray 非空/ClearTray 已執行)且 IsOutputBottomOccupied() 時保留 hold,HOME 完成(車已 home、離開後座)後補發布 bRearReadyForPick=true。
- LD-2(黃):drain 階段規約:Task∈{2000,3000} 一體收斂至 4000 邊界(兩次缸 Pop+單掃描移交,無馬達無真空);若 reed timeout,fallback=uHome case 50/60 既有無條件 Pop(冪等後盾)。Task∈{1000,4000} 明訂不可 drain。
- LD-3(黃):uHome case 200 強制「移除全部 Loader 盤+ClearTray」(uHome.cpp:659-668)與 PARK/RE-ACQUIRE 正面衝突——PARK 成功保下的 carriage 盤會被此提示人工清掉+資料抹除;需改為 PARK 失敗(A4 servo-alarm 子集)才 fallback 至現行提示。
- LD-4(黃,INFERRED):case 10 以原始 encoder 值直接比大小判他車前後(aLoader.cpp:1929-1940),但 IsLoaderYMoveSafe 明文兩軸 encoder 同一實體行程符號相反(285-286);HOME 重入後兩車皆在 home 端,此門語義需驗證符號約定(疑為既有缺陷,非 HOME 專屬)。
- LD-5(綠,note):MES0924 gate 於 iRealDummy!=DUMMY(1133);SOFT_SIMULATE 下 wipe 後 latch=false→IsRearOccupied()=false 不觸發亦無殘留——sim 驗證 HOME 落點時看不到此 Note 屬預期,勿誤判為缺口。

### drain 邊界

Loader 卸盤梯形的可收斂區間精確為 DischargeTask∈{2000,3000}→收斂至 Task==4000 尚未執行的邊界:此區只含 PushCylinder->Pop()(1981)與 LeanCylinder->Pop()(1989)兩個帶 off-reed 確認的純缸動作,加上掛在 Lean confirm 掃描上的單掃描資料移交(RearKind/RearTrayID/RearSourceTray←Tray + ClearTray + Task=4000,1994-1998),無任何 MoveLoaderY、無真空——drain 停在 2000 結束(Push off、Lean 仍 on、盤已落座)亦安全,uHome case 50/60 的無條件 Pop 是 timeout 後盾。Task==1000(MoveLoaderY 載盤赴後,1963)與 Task==4000(MoveLoaderY 空車退回,2003)含馬達移動,不可 drain:1000 交 PARK(盤仍夾於車上);4000 無需 PARK(車已空、盤在固定後座 rest 上不受 home 影響),home 退車即物理等效完成退場,resume 時僅需補 LD-1 的發布。case 100 為單掃描 arm,不存在「落在中間」;drain 亦不得把 100 前推(下一步即馬達)。

## TLoaderModule::DoFrontDestackDown (aLoader.cpp:2020-2099) + 包裹段 TLoaderModule::DoFeedTray (aLoader.cpp:1291-1583);同族 Teach 梯 TestGoDownTray/TestGoUpTray (aLoader.cpp:2103-2207)（黃）

> 覆核：OK。缺口代碼：LK-1, LK-2, LK-3, LK-4, LK-5, LK-6, LK-7

1. DoFrontDestackDown(aLoader.cpp:2020-2099)全程純氣缸+一次 1000ms settle(Delay.Set(10);HTimer.cpp:16 iTime*100ms),零 Move、零真空 — 整段可 drain 至 SubTask 7 return true(SubTask 自復位 1,aLoader.cpp:2093-2094)。VERIFIED。
2. 生產包裹段 FeedTask 4000→4100→8200→8300 同為純氣缸;drain 邊界 = case 9000「進入點」;完成謂詞 = 三顆 destacker out-bit 全 false(IsReadyForAmrHandoff,aLoader.cpp:524-526)。
3. 最大缺口 = 落盤(DestackTask 5)→鑄造(FeedTask 9500)之窗:盤已在 carriage 但 fHasTray==false,今日 uHome 的 fHasTray 鍵控移除提示(uHome.cpp:659-661)與復歸 feed 都看不見它,復歸重進料 = 第二盤疊壓(aLoader.cpp:1514-1517 註解自證)。drain 只收斂氣缸,此洞需 LK-1 鑄造補救。
4. InitialFlag 每次 HOME 歸零 host 車數並重掛 iCarTrayTotal/iFeedSerial(aLoader.cpp:65-66,555-559)→ 中途車 HOME 後 GetFedTrayKind(1553)盤種錯位(LK-2)。
5. DestackTask 5 帶已 Clear 的 Delay 進入 = Off() 永假(HTimer.cpp:42-43)→ 禁止任何「跨 HOME 保留 SubTask」設計,唯一安全形狀 = wipe 前 drain 至段界(LK-4)。
6. uHome 從不觸碰 C_Loader_FrontRiseTray_1/2、C_Loader_FrontSeparateTray_1(全庫 grep VERIFIED)→ drain 跳過路徑(EMG/斷氣)需 IsReadyForAmrHandoff 前置檢(LK-6)。
7. 判定 yellow:缺口可枚舉(LK-1/2/6/7 為小修,LK-3/4/5 為 drain 設計規約),無需新的機構決策(A1/M2 已答)。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| Feed 初始/閒置 | DoFeedTray Flag==0;FeedTask 1,10,100 | FeedTask=1 初始化(aLoader.cpp:1302-1308);fHasTray 短路(1340-1344);AcquireFrontOwner 純資料仲裁(1349;886-891) | TrayMotor->fHasTray、iFrontOwner | FeedTask 跳轉;iFrontOwner=LoaderNo | 相位起點 | —(無在飛材料) |
| 進料定位 | FeedTask 1000 | MoveLoaderY 至 feed Y(1355;MotorMove aLoader.cpp:278);sim 直跳 3500(1357-1358) | 馬達 in-position | →2000(real)/3500(sim) | 冪等段(Move 可重發) | 不可(馬達運動) |
| 進料前放夾 | FeedTask 2000,3000 | PushTray Pop→LeanOnTray Pop(1365-1371) | 氣缸 off-reed(mycylin.cpp:299-;完成自復位 Task=1) | FeedTask=3500 | 冪等段 | 可收斂→3500(純氣缸;但 case 10 已保證 carriage 空,棄置亦安全) |
| 料源閘 | FeedTask 3500 | SnLoader_Inputend / sim IsContinuousFeed 分派(1384-1394) | SnLoader_Inputend(Enable 閘) | →4000 或 9000 | 相位起點 | —(drain 停止點,禁止越過進 4000) |
| Destack 進場 | FeedTask 4000 | State->DestackTask=1(1401) | — | FeedTask=4100 | 相位起點 | 可收斂→9000(整段起點) |
| 抬升 | DestackTask 1,2,3 | Rise_1.Reset/Push→Rise_2.Reset/Push(2031-2050) | on-reed 確認+逾時警報(mycylin.cpp:216-231) | SubTask 遞進 | 冪等段(已達位快速再確認) | 可收斂→SubTask 7 完成 |
| 分離爪入 | DestackTask 4 | Empty 互鎖等待(2054;mycylin.cpp:90-95,bFrontSeparateInterlock 閘)→Separate.Push→Delay.Set(10)=1000ms(2056-2061;HTimer.cpp:16) | Separate on-reed;C_Empty_FrontSeparateTray_1.GetOutBit | SubTask=5;Delay 起動 | 冪等段(含跨模組互鎖閘) | 可收斂→SubTask 7 |
| 落盤 | DestackTask 5 | Delay.Off 後 Rise_2.Pop(2066-2072);底盤落至 carriage 之瞬間(INFERRED:氣缸角色+1515 註解「a tray already separated below」) | Delay + Rise_2 off-reed | SubTask=6 | 危險段(帶已 Clear 的 Delay 進入=永久卡死,HTimer.cpp:42-43;僅前綴可達) | 可收斂→SubTask 7 |
| 收爪收升 | DestackTask 6,7 | Rise_1.IsOn 安全閘「疊料仍被持住才放爪」(2080,註解 2077-2079)→Separate.Pop→Rise_1.Pop(2082-2094) | off-reed;Rise_1 on-reed 安全閘 | SubTask=1 + return true(2093-2094) | 冪等段 | 可收斂→SubTask 7(=子梯段界) |
| 落盤後夾持 | FeedTask 8200,8300 | LeanOnTray.Push→PushTray.Push(1411-1417) | on-reed | FeedTask=9000 | 冪等段 | 可收斂→9000(段界=9000 進入點) |
| 決策樞紐 | FeedTask 9000 | MES0921 計數交叉檢核 modal(1428-1447);在席檢查→9500(1457-1469);CleanOut 自收路由(1482-1488);AMR 600s 延遲+MES0920 modal,K_RETRY/K_CLEAN_OUT→9500(1498-1529) | SnLoader_Inputend+PushCylinder on-reed+iCarTrayTotal/iFeedSerial | →9500/1/10000;RunMode 變更 | 相位起點(純感測+意圖分派,無運動) | 不可(modal 警報+模式變更,drain 禁入) |
| 確認+鑄造 | FeedTask 9500 | SnLoader_InputHasTray 確認(1542-1544)→fHasTray=true、PrepareTrayMap、iFeedSerial++、SetKind/TrayID 同一 scan 完成(1546-1562);失敗支 JAM0913 modal(1570-1574) | SnLoader_InputHasTray(RealDummy 三層+Enable 閘) | 單掃描鑄造→FeedTask=10000 | 原子commit | 不可(材料資料交接) |
| 收尾 | FeedTask 10000 | ReleaseFrontOwner+FeedTask=1+return true(1578-1580) | — | 單掃描釋放 | 原子commit | — |

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| FeedTask 1/10/100、3500(無在飛料) | InitialAllTask(true)(csystem.cpp:1359)→Loader InitialFlag() 全 wipe(ResetSide aLoader.cpp:126-144);uHome case 50/60 放兩車 Lean/Push(uHome.cpp:548-562);carriage 空、疊料靜置於 destacker;復歸重走 feed | 無缺口。綠 |
| FeedTask 1000/2000/3000 | Y 移動被 HOME 接管重歸零(uHome case 200 homes MLoaderY_1/2);夾缸 uHome 亦釋放;case 10 保證此窗 carriage 無料 | 綠 |
| DestackTask 1-4(落盤前) | uHome 完全不碰三顆 destacker 缸(grep 全庫 VERIFIED:僅 aLoader 生產/測試、aEmpty 互鎖讀、database 命名)→Rise/Separate 保持斷點輸出、疊料被懸持整個 HOME(靜止懸持,M2 類比安全);wipe 後下次 feed 自 case 1 快轉重走(Push 對已達位缸數 scan 內確認,mycylin.cpp:264-288) | 物理快轉未實測+疊料長時間懸持;planned drain 先補完分離段=確定性。黃→drain 後綠 |
| DestackTask 5(落盤瞬間) | 半落盤狀態被凍結;wipe 使復歸從 case 1 重來(非停留 case 5,故今日不觸 Delay 卡死);物理半落狀態進入下欄同樣風險 | drain 於 1s settle 內收斂此窗。紅→drain 後綠 |
| DestackTask 6-7、FeedTask 8200-9000(落盤後、鑄造前) | 盤已在 carriage 但 fHasTray==false→uHome 案 200 的移除提示以 fHasTray 鍵控(uHome.cpp:659-661)看不見它、ClearTray 亦不涵蓋;復歸後 case 10 判空→重新 destack→第二盤疊上已在之盤(aLoader.cpp:1514-1517 註解自證:clamps/cuts it + scatters IC) | LK-1:drain 只收斂氣缸,鑄造缺口仍在。紅→LK-1 後綠 |
| FeedTask 9000(乾料支,無盤在 carriage) | 只是 MES0920/AMR 600s 等待被 wipe;無材料在飛 | 綠 |
| FeedTask 9500 已鑄造 ~ 10000 | fHasTray=true→uHome 案 200 強制人工移除全部 Loader 盤+ClearTray(uHome.cpp:659-668)=keep-material 目標的反面(今日安全但棄料) | LK-7:PARK/RE-ACQUIRE 上線時 LoaderY 應改走記憶+保留。黃 |
| 任一視窗疊加(AMR 中途車) | InitialFlag 歸零 iSecsCarTrayCount 並 RefillSimInfeed 重掛 iCarTrayTotal=fallback、iFeedSerial=0(aLoader.cpp:65-66,555-559)→GetFedTrayKind(1553)對剩餘盤的種類判定錯位(cover/identity 靠 serial==total 慣例,571-574) | LK-2。黃 |

### 缺口

- LK-1 落盤→鑄造窗孤兒盤(紅級今日行為):fHasTray==false 使 uHome 移除提示(uHome.cpp:659-661)與復歸 feed 都看不見已落之盤,重進料=雙盤壓毀(aLoader.cpp:1514-1517)。最小修:HOME 復歸後首次 feed 前,若 SnLoader_InputHasTray.IsOn() && fHasTray==false → 直入 FeedTask 9500 confirm-then-mint(複用 1482-1488 CleanOut 自收 idiom)。黃
- LK-2 InitialFlag 車籍 wipe:aLoader.cpp:65-66 歸零 host 車數後 RefillSimInfeed 以 fallback 重掛 iCarTrayTotal、iFeedSerial=0(555-559)→中途車 HOME 後盤種分類與 MES0921 交叉檢核雙雙錯位。最小修:keep-material 路徑以 bKeepRear(79-87)同款 guard 保留 iSecsCarTrayCount/iCarTrayTotal/iFeedSerial。黃
- LK-3 drain 範圍規約:僅 FeedTask∈{4000,4100,8200,8300} 續泵至 9000 進入點;禁止評估 case 3500(會開啟全新 destack);9000/9500/10000 禁入(modal+鑄造+owner 釋放)。綠(設計註記)
- LK-4 case-5 計時器重入卡死:DestackTask=5 帶已 Clear 的 Delay 進入=Off() 永假(HTimer.cpp:42-43;ResetSide 會 Clear FeedDelay,aLoader.cpp:140)→任何「跨 HOME 保留 SubTask」捷徑被禁;唯一安全形狀=wipe 前 drain 至段界。綠(設計禁令)
- LK-5 Loader↔Empty Separate 互鎖與 drain 併發:DestackTask 4(aLoader.cpp:2054)等 Empty 爪收回,Empty 側對稱等 Loader(aEmpty.cpp:578/723/902/989);drain 引擎須同掃描併泵兩模組(或 Loader 先序),否則順序 drain 可能互等;drain 超時回退兜底。綠
- LK-6 drain 跳過路徑(EMG/安全門/斷氣):三顆 destacker 缸在 HOME 全程無任何釋放者(grep VERIFIED),斷點輸出殘留;跳過 drain 後首次 feed 前應前置檢 IsReadyForAmrHandoff(aLoader.cpp:524-526,三 out-bit 全 false),不符則要求操作員確認 destacker 區。黃
- LK-7 已鑄造盤 vs 案 200 強制移除:uHome.cpp:659-668 無條件人工清除全部 Loader 盤+ClearTray,與 keep-material 目標矛盾;PARK/RE-ACQUIRE 上線時 LoaderY 應改為記憶 Y+identity 保留(Loader InitialFlag 增 bKeepMaterial 參數,與 Auto/TrayArm/SortArm 對齊)。黃

### drain 邊界

HOME drain 階段對 Loader 前 destack 的精確可收斂邊界:當 State->FeedTask∈{4000,4100,8200,8300} 時,持續以 DoFeedTray(LoaderNo,1) 泵送直到 FeedTask==9000 的「進入點」即停(不得執行 case 9000 本體);其中 case 4100 內的 DoFrontDestackDown(SubTask 1→7,aLoader.cpp:2020-2099)必然跑到 return true(SubTask 自復位 1),含唯一等待=1000ms settle(Delay.Set(10),HTimer.cpp:16)與兩個純讀閘(Empty Separate 互鎖 2054、Rise_1 持疊安全閘 2080)。此區間經逐行驗證零 MoveLoaderY/MotorMove、零真空、零 modal — 僅 C_Loader_FrontRiseTray_1/2、C_Loader_FrontSeparateTray_1、LeanOnTray、PushTray 五缸(VERIFIED)。FeedTask<4000(含 3500)一律視為已在邊界直接棄置:1000 是馬達、2000/3000 雖純氣缸但 carriage 必空(case 10 閘)且 uHome case 50/60 之後仍會釋放同兩缸;嚴禁 drain 評估 case 3500(會啟動新 destack)。9000/9500/10000 禁入(MES0920/0921 modal、單掃描鑄造、owner 釋放)。drain 完成謂詞可直接複用 IsReadyForAmrHandoff(aLoader.cpp:524-526,三 destacker out-bit 全 false)+FeedTask 已達 9000 或 <4000。互鎖對偶:drain 引擎須同掃描併泵 Loader 與 Empty/Color 同族梯(LK-5)。同缸 Teach 梯 TestGoDownTray/TestGoUpTray(2103-2207)若在 HOME 觸發時在跑,須以相同規則收斂(InitialFlag 100-102 會 wipe 其 SubTask)。注意:drain 收斂只解決氣缸幾何,落盤後未鑄造之盤仍需 LK-1 的 9500 補鑄,否則復歸即雙盤壓毀。

## TTrayArmModule::DoPick + DoMoveToStationZSafe + DoLowerClampRaise (含 DoTrayArm case 100/1000 派工與殘留守衛)（黃）

> 覆核：OK。缺口代碼：TP-1, TP-2, TP-3, TP-4, TP-5

1. DoPick 取料鏈 = Z-safe就位(PickTask 1/10, 內含 MES1721 watchdog 閘) → 純汽缸夾取(1000–3000) → 單掃描資料commit(4000)。case 4000 全程無感測等待、無 break,一個 scan 內完成 copy→notify→fHasTray=true(VERIFIED aTrayArm.cpp:668-710)。
2. A2 鐵律在碼中只有單向硬鎖:X 走行每次呼叫都檢 Z-up(aTrayArm.cpp:352-364, 100ms debounce, 失守全停+alarm);反向「Z 只在 X 靜止時動」無任何 in-code 檢查,純靠梯形前綴順序保證(DoZDown aTrayArm.cpp:385-389 無 X 檢查)——drain 重入 case 1000 前必須自行補 X 到位確認(TP-3)。
3. 今日 HOME 落點:未夾持(1/10/1000)全綠;已夾持未 commit(2000–3000)→ uHome either-On 保夾 + InitialFlag both-On 收養 → MES1722 強制人工取盤(安全但停產,黃);commit 後(4000 之後)→ keep-material + case-100 heal(d63d33a)乾淨續走。
4. 紅級破口 TP-1:uHome 保夾條件(either-On)與 InitialFlag 收養條件(both-On)不對稱——恰一顆 On 時夾爪閉、盤在手、latch 卻判空,下一次派工會閉爪持盤 Z-down 撞佔用 rear(雙疊料)。一行守衛可關(夾取前確認雙 On reed 皆 OFF)。
5. 計劃 drain 對本段的正確邊界:PickTask∈{1000..3000} 收斂到 DoLowerClampRaise 回 true 後「必須連同 case 4000 與 DoTrayArm case-1000 完成 tick 一起執行」(全為單掃描純資料),落在 Task=2000/PlaceTask=1;停在 4000 未執行會重演 MES1722。
6. 判定 yellow:缺口皆可枚舉、修法小(守衛/邊界規約),無需新的機構決策(riser pawl 不涉及本段)。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| 派工/殘留守衛 | DoTrayArm 100 (+DoPick Flag==0) | HasTray() 重同步(aTrayArm.cpp:1068);bHasTray+Job!=NONE→恢復搬運並重發 RequestReturnTray(EMPTY/COLOR, d63d33a heal, 1090-1093)→DoPlace(0),Task=2000;bHasTray+Job==NONE→un-adopt(雙 On reed 皆滅→釋放 fHasTray, 1111-1117)或一次性 MES1722(1118-1122);空手→DecideJob→DoPick(0)(重置 PickTask=1+清 watchdog 三件組, 591-598)→Task=1000 | fHasTray latch + C_TrayArm_Front/RearClamp.OnSensor + 來源 rear latch(DecideJob 483-568) | Job/iDeliverKind/iDeliverTrayID/iAutoTarget(派工 latch) | 相位起點 | — |
| Z 安全升 | PickTask 1 (DoMoveToStationZSafe case 1, aTrayArm.cpp:399-402) | DoZUp:Down.Off+Up.Push,須 UP reed 實確認(369-383) | C_TrayArmZ_Up reed(IsZUpAtPosition 320-333) | 無 | 冪等段 | 可收斂→10(無益:uHome case 2 本就升 Z, uHome.cpp:474-475) |
| X 走行+取料閘 | PickTask 10 (404-407) + 到位閘(603-658) | MoveTrayArmX(GetPickSourceX())——每次呼叫檢 Z-up 互鎖(352-364,=A2 硬鎖方向);到位後依 Job 檢 IsRearReadyForPick/bTrayReady(614-652),阻塞每 tick 打 OnPickGateBlocked(MES1721:60s 窗+1.5s poll-gap 續性,先 DecStopAllMotor→TriggerSnapshot→Note, 197-244);過閘清 watchdog(653-655)→PickTask=1000。上層 DoTrayArm case 1000 另有 LOADER_RECOVERY 來源被清空之棄工守衛(PickTask<1000, 1152-1162) | motor 到位 + 來源 readiness predicate + PickWaitTimer | 無 | 冪等段 | 不可(含 motor Move) |
| Z 下降 | PickTask 1000 (DoLowerClampRaise case 1000, 424-427) | DoZDown:Up.Off+Down.Push(385-389) | C_TrayArmZ_Down reed(‖IsSoftSimulate) | 無 | 危險段(前提=X 在來源站;DoZDown 內無 X 檢查,A2 反向純靠前綴) | 可收斂→過4000 |
| 夾爪閉合 | PickTask 2000 (429-441) | FrontClamp.Push+RearClamp.Push(冪等快轉),設 settle delay(iTrayArmClampSettleMs) | 雙 clamp On reed(‖sim) | 無 | 冪等段 | 可收斂→過4000 |
| 夾持穩定 | PickTask 2100 (443-446) | ArmDelay.Off() 等待 | timer | 無 | 冪等段 | 可收斂→過4000 |
| Z 升(持盤) | PickTask 3000 (448-451) | DoZUp,UP reed 實確認後回 true→PickTask=4000 | C_TrayArmZ_Up reed | 無 | 冪等段 | 可收斂→過4000 |
| 取料 commit | PickTask 4000 (668-710) | 單掃描、無等待、無 break:Loader→Tray.CopyFrom(GetRearSourceTray())(676)→ID(677)→Kind 取料時重讀(678-684, 防閘上久等後 stale kind 誤路由)→NotifyTrayArmPickRearTray(685, copy 必須先於 notify——notify 清 Loader rear hold, 671-673);Color→GetTrayID+CopyFrom→NotifyTrayPicked(688-697);Empty→CopyFrom→SetRearHasTray(false)(699-705);最後 fHasTray=true/bHasTray=true→return true(707-710) | 無(純資料) | grid/iDeliverTrayID/iDeliverKind/fHasTray | 原子commit | —(即 drain 邊界本身,與 1000–3000 同一 drain 中執行) |
| 完成 tick | DoTrayArm 1000 完成分支 (1163-1175) | DoPick(1)==true 同 scan 內:LOADER_RECOVERY→DecidePlaceDestAfterPick(817-867, 可含 RequestReturnTray);其他→PlaceDest=TAPLACE_AUTO;DoPlace(0);Task=2000 | 無(純決策) | PlaceDest/iAutoTarget | 原子commit(與 case 4000 同 scan) | — |

(以上行號皆 VERIFIED 於 D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\aTrayArm.cpp 與 uHome.cpp 現碼)

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| V1 派工/DoPick(0)(Task=100) | 空手、無latch可失。InitialFlag 清 watchdog 三件組(aTrayArm.cpp:75-80);uHome 升Z開爪;恢復後 DecideJob 重派 | 綠 |
| V2 PickTask 1/10(含 MES1721 阻塞等待) | 未夾持、爪開、Z-up。uHome case 2 升Z開爪(無盤可掉);motor home 停X;盤仍留在來源 rear;bHasTray=false→keep 分支不成立→Job 全清(88-99);watchdog 由 InitialFlag 清除,不誤發 | 綠(TrayArm 側)。INFERRED 依賴:來源 rear 的實體盤須靠來源模組 sensor 再導出重新派工(Loader/Empty 側視窗,非本段) |
| V3 PickTask 1000(Z 已降/降中,爪開) | uHome case 2 先升Z(474-475)、爪未持→Off 開爪(497-501),盤原地留在 rear 未被拖動(A2:Z 動時 X 靜止成立——PickTask≥1000 表示 X 已到位)。之後同 V2 | 綠 |
| V4 PickTask 2000(爪閉合中) | 三分岔:(a)雙 reed 未 On→uHome 開爪、盤留 rear=V3,綠;(b)雙 On→保夾、盤隨Z升、InitialFlag 收養殘留(57-69)→MES1722 人工取盤,黃;(c)恰一顆 On→uHome either-On 保夾(uHome.cpp:493-495)但 InitialFlag both-On 不收養(61-62)→latch 判空、夾爪持盤,case-100 un-adopt 因 bHasTray=false 不會執行(區塊在 if(bHasTray) 內, 1072/1098)→下一派工閉爪持盤 Z-down 入佔用 rear=雙疊料 | 紅(TP-1)。計劃 drain 把 (b)(c) 收斂為 commit 後乾淨續走,但 (c) 的守衛仍須獨立補上(HOME 因 EMG/斷氣時 drain 被跳過) |
| V5 PickTask 2100/3000(已夾持,未 commit) | 盤在爪、grid/ID/Kind 未 copy。uHome both-On 保夾→盤隨Z升過 home;InitialFlag 收養+強制非 keep(53-55:mid-pick 目的地/身分不可信,設計正確)→MES1722 人工取盤;來源側 rear 實體已空,sensor 再導出一致 | 黃:今日安全但停產+人工。計劃 drain(1000→3000 收斂+執行 4000 commit)把它變成乾淨 keep-material 搬運=綠;唯 drain 邊界不可停在「4000 未執行」(TP-2) |
| V6 PickTask 4000 之後(carrying, Task=2000) | 4000+完成 tick 為同 scan 原子,不存在「半 commit」落點。keep 分支保 Job/PlaceDest/Kind/ID/grid(88-92);case-100 heal 重發 RequestReturnTray(EMPTY/COLOR, d63d33a);MES1723 place watchdog 蓋 case-500 等待 | 黃(小):PlaceDest=TAPLACE_AUTO 的恢復路徑無 rear 再驗——DoPlace 無 case-500 等待,直接 X→放盤(738-788),派工時 FindTrayRequestAuto 的 rear-free 閘不重跑(TP-4) |

### 缺口

- TP-1(紅):uHome 保夾(either-On, uHome.cpp:493-495)與 InitialFlag 收養(both-On, aTrayArm.cpp:61-62)判準不對稱——恰一顆 On sensor 亮時夾爪閉持盤而 fHasTray=false,un-adopt 區塊不可達(1098 在 if(bHasTray) 內),下一派工閉爪持盤 Z-down 撞來源 rear。最小修法:DoPick 進入 1000 前(或 DoLowerClampRaise(true) case 1000)加「雙 clamp On reed 皆 OFF 才准 Z-down,否則 MES1722」守衛;或把收養條件放寬為 either-On。
- TP-2(黃):計劃 drain 若把 PickTask 1000–3000 只收斂到「4000 待執行」即停,InitialFlag 收養照發 MES1722(fHasTray 仍 false)——白做。drain 邊界規約必須=執行 case 4000 + DoTrayArm case-1000 完成 tick(皆單掃描純資料,無 motor/真空),落點 Task=2000/PlaceTask=1;且須在 Loader/Empty/Color InitialFlag() 抹除前執行,GetRearSourceTray/GetTrayID 才仍有效。
- TP-3(黃):A2 反向(Z 只在 X 靜止且到位時動)在 DoZDown/DoLowerClampRaise 無任何 in-code 檢查(385-389, 424-427),純靠梯形前綴。drain 重入 case 1000 前應以 CheckArmPosArrival 重確認 MTrayArmX 仍在 GetPickSourceX()(HOME 起因若是 TrayArmX 伺服異常即失前提),失敗則跳過 drain 走今日 MES1722 fallback。
- TP-4(黃):HOME 後 TAPLACE_AUTO 恢復無 rear 再驗:case-100 heal 只重發 EMPTY/COLOR 的 RequestReturnTray(1090-1093),DoPlace Auto 路徑從 X 到位直入 DoLowerClampRaise(775-788),放盤前不檢 Auto rear 佔用。最小修法:DoPlace case 1/10→1000 轉移前加 rear-occupied 檢查+OnPlaceGateBlocked tick(沿用 MES1723 機制)。
- TP-5(綠,備忘):PickTask 1/10 不納入 drain(case 10 含 MoveTrayArmX;case 1 可收斂但無益)——未夾持時 HOME 原生路徑已乾淨,drain 階段應以 PickTask>=1000 為啟用條件。

### drain 邊界

HOME drain 階段對本段的可收斂邊界:僅當 PickTask∈{1000,2000,2100,3000}(夾取choreography已啟動)才啟用;前提守衛=MTrayArmX 以 CheckArmPosArrival 重確認仍在 GetPickSourceX()(TP-3)且雙 clamp 狀態與 PickTask 一致,HOME 起因為 EMG/安全門/斷氣則整段跳過。啟用後逐 tick 續跑 DoLowerClampRaise(true,PickTask)(aTrayArm.cpp:412-454,全為 TMyCylinder Push/Pop+settle timer,無任何 motor Move、無真空)至回 true,然後「必須」在同一 drain 中執行 PickTask 4000 的單掃描資料 commit(grid CopyFrom→ID→Kind 重讀→來源 notify→fHasTray=true, 668-710)以及 DoTrayArm case-1000 完成 tick(DecidePlaceDestAfterPick/PlaceDest=TAPLACE_AUTO+DoPlace(0)+Task=2000, 1163-1175)——這兩步皆純資料、同 scan 原子,停在 4000 未執行會讓 InitialFlag 收養殘留、重演 MES1722 強制人工取盤(TP-2);正確落點 Task=2000/PlaceTask=1 是 place 梯形的相位起點,motor home 從此落點安全。case 4000 對來源側的 notify 效果(NotifyTrayArmPickRearTray/NotifyTrayPicked/SetRearHasTray(false))反正會被步驟(5)的 Loader/Empty/Color InitialFlag() 抹掉,真正需要存活的是臂側 commit(Tray grid/iDeliverKind/iDeliverTrayID/fHasTray),InitialFlag(bKeepMaterial=true) 的 keep 分支(88-92)原樣保留;但 drain 必須在來源模組被抹除之前跑完,GetRearSourceTray/GetTrayID 才讀得到有效資料。PickTask 1/10 一律不 drain(case 10 含 MoveTrayArmX;未夾持時 uHome case 2 原生升Z開爪已是乾淨路徑)。

## TTrayArmModule::DoPlace (TAPLACE_AUTO 路徑, aTrayArm.cpp:715-815)（黃）

> 覆核：OK。缺口代碼：TA-1, TA-2, TA-3, TA-4

DoPlace(TAPLACE_AUTO) 是「Z升→X走位→Z降→開爪放盤→靜置→Z升→單scan資料commit」七段梯;唯一馬達動作在 case 10 (MoveTrayArmX, aTrayArm.cpp:366),1000→4000 全鏈無馬達。case 4000 為原子commit:StageRearGrid 只複製格點、刻意不設 fHasTray(占用權歸 DoFeedTray case 7000,aAuto1To6.cpp:613-616;先設會令 bCarHasTray 假占用、餓死 FindFeedAuto,aTrayArm.cpp:793-796),之後 NotifyTrayArmDelivered/SetRearHasTrayFromTrayArm 設 rear 三latch+Kind/ID、臂側清空、外層同scan Job=NONE(aTrayArm.cpp:1179-1184)——全部單scan,無撕裂中態(VERIFIED)。case 1/10 的 CleanOut 排空分流與 GetTrayRequest 用同一邊界訊號 SortArm.IsCleanOutFinish(aTrayArm.cpp:756-757 vs aAuto1To6.cpp:1129),重入時每tick再判,HOME後亦由 d63d33a case-100 re-request 治癒。今日最弱視窗=開爪後至 4000 前(tray已落 Auto rear、fHasTray仍true):keep-material 重放會開爪 Z 降壓在已放置 tray 上補 commit——幾何等同正常release當下姿態(INFERRED 安全),且 FindFeedAuto 因 pending latch 未設不會搶拉(aAuto1To6.cpp:402)。缺的是:計畫 DRAIN 段的 per-case 邊界規格(TA-1)與 W4 快轉 guard(TA-2)——皆屬可列舉小修,判 yellow。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| 派工/重入口 | DoPlace(0) + 每scan頂端dispatch | PlaceTask=1、ArmDelay.Clear、ClearPlaceGateWatch(aTrayArm.cpp:720-726);每scan依 PlaceDest 轉走 DoPlaceToColor/Empty(730-733) | PlaceDest(記憶體) | 無 | 相位起點 | — |
| CleanOut排空分流 | case 1/10 前段(756-774) | Run_CleanOut 且 SortArmModule->IsCleanOutFinish() → PlaceDest=COLOR(identity)/EMPTY(其他)+RequestReturnTray+iAutoTarget=-1+PlaceTask=1;下一scan由頂端dispatch改走回收梯 | SortArm.IsCleanOutFinish()——與 GetTrayRequest 排空閘同源(aAuto1To6.cpp:1129)VERIFIED | 單scan改寫 PlaceDest/iAutoTarget(RequestReturnTray 冪等) | 相位起點(純判斷無動作) | —(drain 可任其執行) |
| Z升 | case 1 | DoMoveToStationZSafe Task1→DoZUp:C_TrayArmZ_Down.Off+C_TrayArmZ_Up.Push(369-383,399-402) | Z上感測 IsZUpAtPosition(320-333) | PlaceTask=10 | 冪等段 | 可收斂→case 10 |
| X走位 | case 10 | MoveTrayArmX(GetAutoX(iAutoTarget)) MotorMove;每tick Z-up interlock、失位即 StopAllMotor(335-366,404-407) | 馬達到位 + Z上感測每tick | Status=TAS_PLACING、PlaceTask=1000(775-779) | 冪等段(Move 可重發) | 不可(馬達) |
| Z降 | case 1000 | DoLowerClampRaise→DoZDown:C_TrayArmZ_Up.Off+C_TrayArmZ_Down.Push(385-389,424-427);爪仍閉、tray在手 | C_TrayArmZ_Down reed | PlaceTask=2000 | 冪等段 | 可收斂→case 2000(Pop 未命令即停) |
| 放爪=實體交接 | case 2000 | Front+RearClamp.Pop() → tray 落於 Auto rear 架;ArmDelay.SetMS(iTrayArmClampSettleMs)(429-441) | 兩夾爪 Pop reed(冪等快速再確認) | PlaceTask=2100 | 冪等段(但為實體material handoff) | 不可(material handoff;Pop 已命令時例外見 TA-1) |
| 靜置 | case 2100 | ArmDelay dwell(443-446) | 計時器 Off | PlaceTask=3000 | 冪等段(僅可經2000進入,計時必已臂;獨立重入不可達——InitialFlag 重設 PlaceTask=1,aTrayArm.cpp:72) | 可收斂→完成 |
| Z升(空手) | case 3000 | DoZUp(448-451) | Z上感測 | PlaceTask=4000 | 冪等段 | 可收斂→完成 |
| 資料commit | case 4000 | StageRearGrid(iAutoTarget, MMTrayArmX->Tray)——只複製 RearGrid、不設 fHasTray(aAuto1To6.cpp:1214-1219;占用歸 DoFeedTray case7000 之 TrayMotor->fHasTray=true,aAuto1To6.cpp:607-619;先設會翻 bCarHasTray 餓死 FindFeedAuto,aTrayArm.cpp:793-796)→ AMR: NotifyTrayArmDelivered(Kind/ID+三latch+CleanOut自癒+AS_REAR_STAGED,aAuto1To6.cpp:1191-1208)/ Normal: SetRearHasTrayFromTrayArm(440-462)→ 臂側 Tray.Clear+fHasTray=false+bHasTray=false → return true(790-812) | 無感測(純資料;rear 底感測事後由 RefreshAutoState 再證,aAuto1To6.cpp:346-361) | 單scan:Auto 側 bRearHasTray/bRearCanUse/bRearDeliveredPending/RearKind/RearTrayID + 臂側清空 + 外層同scan Job=NONE、Task=100(aTrayArm.cpp:1179-1184)VERIFIED 無break中斷 | 原子commit | 可收斂→完成 |

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| W1 派工~case 1(Z升,tray在手) | uHome 先升 TrayArmZ、夾爪因 HasTray()/爪感測 guard 保持閉、tray 隨臂升(uHome.cpp:474-501);InitialFlag(true) 保 Job/PlaceDest/iAutoTarget/iDeliverKind/ID、PlaceTask 重設1(aTrayArm.cpp:71-72,82-91);resume case100 Job!=NONE→DoPlace(0) 全程重放(1078-1096) | 綠(d63d33a 架構已治癒) |
| W2 case 10(X行進中) | HOME 停軸;tray 爪閉隨臂;同 W1 重放,MoveTrayArmX 可重發 | 綠 |
| W3 case 1000(Z降中,爪閉) | uHome 先 Z 升(爪閉,tray 隨升),重放整梯 | 綠 |
| W4 case 2000/2100/3000 及 PlaceTask==4000 未執行(爪已開、tray 已在 Auto rear 架、fHasTray 仍 true) | uHome 不會重閉爪(只在「未持有」分支 Off,uHome.cpp:497-501);Z 空手升;keep-material 因殘留 latch 進 CARRYING;resume 重放:開爪 Z 降至同一放盤深度(=正常 release 當下姿態)→Pop no-op→case 4000 補 commit。Auto 側:rear 底感測 ON→RefreshAutoState latch bRearHasTray(aAuto1To6.cpp:355-358),但 FindFeedAuto 需 bRearDeliveredPending 才拉(402,防撞註解 395-401)故不會與返場臂互撞;GetTrayRequest 因 rear 占用拒發第二盤(1138) | 黃——自癒鏈完整(VERIFIED),但「開爪下降越過已放置 tray」在 production 無先例路徑(DoPick 開爪下降只發生在 Empty/Color/Loader rear),幾何安全屬 INFERRED;TA-2 快轉 guard 可整段免去 |
| W5 case 4000 已執行(commit 完,同scan Job=NONE) | Auto InitialFlag(true) 以 continue 保 bRearHasTray/pending/RearGrid/RearKind/RearTrayID(aAuto1To6.cpp:107-118);臂空、Job=NONE,resume 無事可做 | 綠 |
| W6 divert 已改寫 PlaceDest=EMPTY/COLOR 後、重放前 HOME | keep-material 保 PlaceDest;Empty/Color InitialFlag(無參數)wipe bReturnTray,但 resume case100 對 EMPTY/COLOR re-issue RequestReturnTray(aTrayArm.cpp:1090-1093,d63d33a) | 綠(已合併修復) |

### 缺口

- TA-1:計畫 DRAIN 段尚無 DoPlace per-case 邊界規格——建議明列:case 1 收斂至 10;case 1000 收斂至 2000(Pop 尚未命令時停在爪閉);case 2000 若 Pop 輸出已命令(交接已不可逆)則續收斂至完成,否則不進入;≥2100 一律收斂至完成(1000→4000 全鏈無馬達,終點含 case 4000 單scan commit,臂空+Auto latch=最佳 pre-home 姿態);case 10 不進入。並規定 drain pump 不得凍結 ArmDelay(其本不在 PauseTimeoutTimers,aTrayArm.cpp:132-141)。黃
- TA-2:W4 開爪重放視窗缺快轉 guard——resume 進 DoPlace case 1 時,若 iAutoTarget 之 rear 底感測 ON 且 bRearDeliveredPending==false 且兩爪 On 感測皆 Off,直接跳 case 4000 補 commit,免去「開爪下降壓過已放置 tray」的機構驗證需求(INFERRED 安全但無 production 先例)。黃
- TA-3:TAJOB_EMPTYTRAY_TO_AUTO 派工不重設 iDeliverKind(DecideJob aTrayArm.cpp:545-557 未寫),殘留 Identity 值理論上會讓 CleanOut divert(759-763)把普通空盤誤送 Color rear;一行修:該路徑派工時 iDeliverKind=eTrayKindNormal。綠(需 AMR→Normal 不經 InitialFlag 之罕見序才觸發)
- TA-4:case 2100 若被獨立重入(非經 2000)ArmDelay 未臂之 HTimer.Off 行為未定;現況不可達(InitialFlag 重設 PlaceTask=1),僅列入規約備註,DRAIN 實作不得新增直跳 2100 之入口。綠

### drain 邊界

HOME DRAIN 段對 DoPlace(TAPLACE_AUTO) 的可收斂邊界:PlaceTask==1(DoZUp 純氣缸,aTrayArm.cpp:369-383)可跑到 PlaceTask==10 即停;PlaceTask==10 含 MotorMove(aTrayArm.cpp:366)絕不可跑;PlaceTask==1000(DoZDown 純氣缸,385-389)可跑到 PlaceTask==2000 即停(爪仍閉、tray 在手,uHome 隨後 Z 升帶盤);PlaceTask==2000 是夾爪 Pop=實體交接,原則不進入——但若 Pop 輸出已命令(交接已不可逆)應續跑;PlaceTask==2100/3000/4000(settle 計時 443-446、DoZUp 448-451、純資料 commit 790-812)為全無馬達尾鏈,應一路收斂到 DoPlace return true(case 4000 的 StageRearGrid+Notify/SetRear+臂側清空為單scan原子commit,絕不可停在 3000 與 4000 之間留下孤兒盤)。case 1/10 的 CleanOut 分流(756-774)是無動作純判斷,drain 可任其執行,改道後改依 DoPlaceToEmpty/Color 之邊界。前提:drain pump 不得凍結 ArmDelay(其不在 PauseTimeoutTimers 清單,aTrayArm.cpp:132-141)。

## TTrayArmModule::DoPlaceToEmpty + TTrayArmModule::DoPlaceToColor (aTrayArm.cpp:903/982, 含 DoTrayArm case-100 resume 與 OnPlaceGateBlocked)（黃）

> 覆核：ISSUES（已依覆核修正）。缺口代碼：TR-1, TR-2, TR-3, TR-4, TR-5

1. 互等迴圈已端到端關閉(VERIFIED):resume 分支重簽 RequestReturnTray(aTrayArm.cpp:1090-1093)→受方 bReturnTray 重建(aEmpty.cpp:1167/aColor.cpp:1268)→DoEmpty case100→3000 / DoColor case100→1700 GoUp 清 rear(佔用 rear 也會被 case1000-7000 拉回前池)→case-500 門檻放行→case-4000 通知完成;受方 case3000/1700 的等待以 bReturnTray 為前提,兩側同被 InitialAllTask 重置,不存在單側殘留預約。
2. 無聲卡死已消滅:case-500 兩處阻塞(FEEDING 防撞 + rear 未清)都掛 OnPlaceGateBlocked 60s MES1723(stop→snapshot→Note),含 poll-gap 重臂與 Pause 凍結。
3. 剩餘缺口 = phantom-carry 窗,邊界修正(VERIFIED):起點是 2000 的 Pop「下令」後——非進入 2000 即不可逆(case1000 於 :426 設 Task=2000 後 break,Pop 下一 scan 才首次下令,scan 間 2000 夾仍閉、完全可逆,由 uHome.cpp:493-497 保夾回收);終點是 case 4000「執行」前(4000 賦值 :964-965/:1037-1038 與執行差一 scan)。窗內 fHasTray 殘留→resume 空手重演;Empty/Color 端靠 rear sensor 收斂(浪費一輪),但經 TryDivertCarriedTrayToAuto 可把 phantom 掛上 Auto(bRearDeliveredPending 擋 sensor 自癒,靠 JAM_x11 攔截需操作員 Skip)。
4. 最小修:把既有 clamp-sensor un-adopt(aTrayArm.cpp:1107-1117)前移到 Job!=NONE resume 分支之前;規劃 DRAIN 收斂邊界=PlaceTask 2100/3000(Pop 已 reed 確認)及待執行之 4000→case 4000 完成;2000 不納入(drain 會「執行」交接而非補完),除非 owner 裁決允許(EMG 跳過 drain 時仍靠前者)。
5. 判定 yellow:修復本體有效,殘留項皆可枚舉、純軟體、不需機構決策(owner 裁決僅為可選強化)。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| resume 派發(DoTrayArm case100 Job!=NONE 分支;aTrayArm.cpp:1078-1096) | 100(外層) | 無動作;依 PlaceDest 重簽 EmptyModule/ColorModule->RequestReturnTray()(冪等:set bReturnTray、clear bTrayXToEmptyFinish;aEmpty.cpp:1167-1171、aColor.cpp:1268-1272),DoPlace(0) 重置 PlaceTask=1+ClearPlaceGateWatch | fHasTray latch(InitialFlag keep 分支 aTrayArm.cpp:88-92)+ uHome 夾閉保護(uHome.cpp:491-501) | 無(旗標重置) | 相位起點 | — |
| Z-up + X 走行(aTrayArm.cpp:928-934 / 1006-1013) | 1 / 10 | case1 DoZUp(氣缸);case10 MoveTrayArmX 到 TrayXArmToEmptyXPosition / TrayXArmToColorXPosition(Z-up 連鎖 aTrayArm.cpp:352-366);Empty 路徑 1/10 另有 TryDivertCarriedTrayToAuto(:930;Color 無,identity 被 :886 擋) | C_TrayArmZ_Up reed + X 馬達到位 | divert 時 CancelReturnTray+PlaceDest/iAutoTarget/PlaceTask=1 一 scan 內改寫(aTrayArm.cpp:894-900) | 冪等段(Push 冪等快轉、Move 可重發) | 不可(case10 含 MoveTrayArmX;case1 之 Z-up 與 uHome case2 的 C_TrayArmZ_Up.On() 等效,無需 drain) |
| rear-clear 等待(aTrayArm.cpp:936-958 / 1015-1031) | 500 | 讀受方 GetStatus()==ES_FEEDING/CS_FEEDING(防撞,sim 跳過)+ IsRearHasTray();阻塞每 tick 呼叫 OnPlaceGateBlocked(60s→stop+snapshot+MES1723;aTrayArm.cpp:246-276);Empty 路徑 :937 仍可 divert | SnEmpty_OutputBottomHasTray(aEmpty.cpp:165-183)/ SnColor_OutputBottomHasTray+TrayPos1(aColor.cpp:207-222)經 RefreshStateFromSensors | 過門檻時 ClearPlaceGateWatch + Status=TAS_PLACING(單 scan) | 相位起點 | —(純等待,無任何致動;本身即安全邊界) |
| Z-down(DoLowerClampRaise;aTrayArm.cpp:424-427) | 1000 | DoZDown(C_TrayArmZ_Up.Off + C_TrayArmZ_Down.Push);雙夾仍閉、盤在手;:426 設 Task=2000 後 break ⇒ Pop 於下一 scan 才下令 | C_TrayArmZ_Down reed(Push 自帶 timeout alarm) | 無 | 冪等段 | 不可(交接尚未發生;夾閉 ⇒ uHome case2 Z-up 自然帶盤回收成攜盤 resume) |
| 開夾 commit(DoLowerClampRaise;aTrayArm.cpp:429-441) | 2000 | 雙夾 Pop 下令並等 reed 確認(=物理交接 commit)。注意 scan 間子態(VERIFIED):case1000 設 2000 後 break(:424-427),Pop 於「下一 scan」才首次下令 ⇒ 存在「PlaceTask==2000、夾仍閉、交接未開始」之完全可逆子態 | 夾 Pop reed;Pop 確認與 Task=2100 同一 scan(:434-438) | 物理交接 commit=Pop 下令→reed 確認;軟體 latch(fHasTray/Tray grid)直到 4000 執行才動 ⇒ phantom-carry 窗 | 危險段(Pop 下令後重跑前綴=空手重演;僅能經前綴到達) | 不可(材料交接點:scan 間停在 2000 時夾可能仍閉,drain 執行它=在 drain 中「進行」一次交接,違反無交接準則;夾閉子態由 uHome.cpp:493-497 latch-OR-reed 保夾回收。除非 owner 明示裁決「Z-down reed 已確認+受方 rear 已確認清空(:936-958/:1015-1031)+受方停等通知(aEmpty.cpp:355-360/aColor.cpp:403-414)」下之開夾為允許 drain 動作) |
| settle+Z-up(DoLowerClampRaise;aTrayArm.cpp:443-451) | 2100 / 3000 | 2100 ArmDelay(iTrayArmClampSettleMs)→3000 DoZUp;進入 2100 時 Pop 必已 reed 確認(2000→2100 與確認同 scan,:434-438) | ArmDelay 計時 + C_TrayArmZ_Up reed | 無(交接已於 2000 完成;latch 至 4000 才清 ⇒ 仍在 phantom 窗內) | 危險段(重跑前綴=空手重演;僅能經前綴到達) | 可收斂→4000(純氣缸+timer+資料,無馬達、無真空;Pop 已 reed 確認=交接既成,收斂只是補完) |
| 通知 commit(aTrayArm.cpp:968-977 / 1041-1050) | 4000 | NotifyTrayXToEmptyFinish()(aEmpty.cpp:1183-1188 / aColor.cpp:1328-1332)+ MMTrayArmX->Tray.Clear + fHasTray=false + bHasTray=false;同一 scan DoTrayArm case2000 收尾 Job=NONE/Task=100(aTrayArm.cpp:1178-1185) | —(純資料) | case4000 之「執行」單 scan 原子(通知+清 latch+清 Job 同一 DoTrayArm pass,VERIFIED);但 4000 於 :964-965/:1037-1038 賦值後至下一 scan 執行前存在待執行瞬間(屬 phantom 窗) | 原子commit | —(drain 終點;待執行之 4000 亦可直接補跑完成) |

### 中斷視窗

| 視窗 | 今日行為(d63d33a) | 缺口/判定 |
|---|---|---|
| HOME 落在 case100 resume 前後 / 1/10 / 500(夾閉攜盤、Z 上) | uHome.cpp:473-501 保 Z-up+夾閉(latch 或 reed 任一 held 即不開夾);InitialFlag(true) keep Job/PlaceDest/grid(aTrayArm.cpp:88-92);resume 重簽 RequestReturnTray → 受方 case100 首分支 GoUp(aEmpty.cpp:238-246 / aColor.cpp:298-312)連佔用 rear 也拉回前池(GoUp case1000-7000)→ 門檻放行 → 放盤完成;若受方 bAmrLocked 暫停 GoUp,60s MES1723 通知(VERIFIED) | 綠 — 這就是修復本體;互等迴圈閉合,唯 AMR-lock 卡死僅告警無自動恢復(見 TR-4) |
| HOME 落在 1000(Z 降中,夾閉)或 2000 尚未首次執行(scan 間,Pop 未下令、夾仍閉;aTrayArm.cpp:424-427) | 夾閉 ⇒ uHome.cpp:493-497 latch-OR-reed 保夾,Z-up 帶盤升起,keep-material resume 重走全程(VERIFIED) | 綠 |
| HOME 落在 2000 Pop 下令後 ~ case 4000 執行前(夾開/開啟中、盤已落受方 rear、fHasTray 殘留 true、通知未送;含 4000 已賦值未執行之瞬間 :964-965/:1037-1038) | phantom-carry:adopt 檢查只在 bHasTray==false 時跑(aTrayArm.cpp:59)、un-adopt 只在 Job==NONE 分支(aTrayArm.cpp:1098-1124)⇒ resume 當有效攜盤空手重演。Empty/Color 端:重簽→GoUp 把實體盤拉回前池→空手重放→spurious NotifyTrayXToEmptyFinish 的 bRearHasTray 由 rear sensor 下輪自癒(aEmpty.cpp:172-183)——浪費一輪但收斂(VERIFIED)。但 Empty 路徑 :930/:937 divert 可把 phantom 掛上 Auto:bRearDeliveredPending 擋掉 sensor-OFF 自癒(aAuto1To6.cpp:355-361),靠 DoFeedTray case200 JAM_x11 交叉檢查攔截、操作員 Skip 清除(aAuto1To6.cpp:495-532);rear sensor 未 Enable 的配置則 latch 滯留(INFERRED,config 相依) | 黃 — TR-1/TR-2/TR-3;規劃 DRAIN 收斂 2100/3000→4000 關窗之主體,2000 之 Pop 已下令子態與 EMG 跳過 drain 場合由 resume 端 un-adopt 前移兜底 |
| HOME 落在 case 4000 執行瞬間 | case 4000 之「執行」單 scan 原子:通知+清 latch+清 Job 於同一 DoTrayArm pass 完成(aTrayArm.cpp:968-977/1041-1050 與 case2000 收尾 :1178-1184 同一次呼叫,VERIFIED);惟 PlaceTask=4000 賦值後至下一 scan 執行前為可觀察中間態(夾開、盤已放、fHasTray true、通知未送)——已歸入上列 phantom 窗,且經同一路徑自癒(InitialAllTask 同時清受方旗標,無 spurious 通知殘留),無新增危害(VERIFIED) | 綠(執行原子;待執行瞬間由上列 phantom 窗涵蓋) |
| 受方等待中落 HOME(DoEmpty case3000 / DoColor case1700 等 bTrayXToEmptyFinish) | InitialAllTask(database.cpp:39-67)同 scan 重置兩側:受方 bReturnTray/bTrayXToEmptyFinish 清零 + Task=1,arm keep 則重簽、non-keep(殘盤 adopt→MES1722)則受方也無殘留預約;等待謂詞以 bReturnTray 為前提故不會單側懸掛(VERIFIED) | 綠 |

### 缺口

- TR-1(黃):phantom-carry 窗(PlaceTask 2000 Pop 下令後→case 4000 執行前落 HOME,或操作員 Teach 開夾取盤但 Job 仍在):fHasTray 殘留 → resume 空手重演。最小修=把既有 clamp-sensor un-adopt(aTrayArm.cpp:1107-1117,雙夾 On-sensor 皆 Off 即清 fHasTray)自 Job==NONE 分支前移到 Job!=TAJOB_NONE resume 分支(aTrayArm.cpp:1078)之前,uHome 對真攜盤保夾閉故 reed 是可靠判別器。
- TR-2(黃):phantom 經 TryDivertCarriedTrayToAuto(aTrayArm.cpp:930/937)掛上 Auto → NotifyTrayArmDelivered 設 bRearDeliveredPending(aAuto1To6.cpp:1199)擋掉 RefreshAutoState 的 sensor-OFF 自癒(aAuto1To6.cpp:359),今日靠 DoFeedTray case200 JAM_x11 攔截(aAuto1To6.cpp:508-532)需操作員 Skip;TR-1 修復後自動消失,過渡方案=resume 後首輪 place 前禁 divert。
- TR-3(黃,config 相依):dest rear sensor 未 Enable 時 spurious NotifyTrayXToEmptyFinish 的 bRearHasTray 無 sensor 自癒(aEmpty.cpp:172 條件 bHasRearSensor;aColor.cpp:221-232 else-if 僅 sim)→ rear 假佔位,後續由受方 leftover guard 告警;TR-1 修復後消失。
- TR-4(綠):受方 bAmrLocked 暫停 return GoUp(aEmpty.cpp:240-241;aColor.cpp:303-307),AGV handoff 卡死時 arm 停 case500 —— 已由 MES1723 每 60s 通知涵蓋,無 silent hang,僅無自動恢復,可接受。
- TR-5(綠,規劃指引):DRAIN 階段收斂邊界=PlaceTask 2100/3000(Pop 已 reed 確認,2000→2100 與確認同 scan aTrayArm.cpp:434-438)及已賦值未執行之 4000 → 收斂到 case 4000 完成(含通知;純氣缸+timer+資料)。PlaceTask 2000「不納入」:其 scan 間子態夾仍閉(case1000 於 :426 設值後 break,Pop 下一 scan 才下令),drain 會在 drain 中「執行」一次新交接而非補完既成交接——夾閉子態由 uHome 保夾回收成攜盤 resume,Pop 已下令未確認子態(無法以 Task 值區分)落回 TR-1 的 resume-side guard。可選強化=owner 裁決「Z-down reed 確認+受方 rear 確認清空+受方停等」下之開夾為允許 drain 動作,裁決通過則 2000 亦可收斂、窗形式上全關。PlaceTask 1000 不收斂(夾閉=攜盤,uHome Z-up 自然回收);EMG/安全門/失氣跳過 drain 的場合由 TR-1 兜底。

### drain 邊界

HOME drain 階段對 DoPlaceToEmpty/DoPlaceToColor 的精確可收斂邊界:PlaceTask==2100(ArmDelay)、3000(DoZUp)、及已賦值未執行之 4000 可一路收斂到 case 4000 完成(含 NotifyTrayXToEmptyFinish 與同 scan 的 DoTrayArm case2000 Job=NONE 收尾)——進入 2100 的前提是雙夾 Pop 已 reed 確認(2000→2100 與確認發生在同一 scan,aTrayArm.cpp:434-438),故此區段的交接已既成,收斂只補完 settle/Z-up 與 latch 寫回,且只含 C_TrayArmZ_Down/Up 氣缸與 ArmDelay 計時、純資料,無任何 Move* 馬達呼叫、無真空(aTrayArm.cpp:443-451、968-977/1041-1050)。PlaceTask==2000 不收斂:case1000 於 :426 設 Task=2000 後 break,Pop 於下一 scan 才首次下令,所以 scan 間停在 2000 時夾可能仍閉、交接尚未開始(完全可逆,uHome.cpp:493-497 latch-OR-reed 保夾回收成攜盤 resume)——從此子態 drain 等於在 drain 中「執行」開夾交接,違反無材料交接準則;Pop 已下令未確認之子態無法以 Task 值區分,落回 TR-1 的 resume 端 clamp-reed un-adopt 兜底。可選強化:owner 裁決「Z-down reed 已確認(case1000)+受方 rear 已確認清空(case500 :936-958/:1015-1031)+受方停等通知(aEmpty.cpp:355-360/aColor.cpp:403-414)」下之開夾為允許 drain 動作,則 2000 亦可納入、窗全關。PlaceTask==1000(夾仍閉)不收斂:直接交給 uHome case2 的 Z-up+夾閉保護回收(繼續 Z-down 反而在 drain 中開始一次新交接)。case 1/10 不收斂(case10 是 MoveTrayArmX 馬達;case1 的 Z-up 與 uHome case2 等效)。case 500 為純等待、自身即安全邊界,無物可 drain;case 4000 之執行為單 scan 原子資料段,是 drain 終點。EMG/安全門/失氣跳過 drain 時,整段 2000 Pop 下令後~4000 執行前的窗由 TR-1 兜底。

## TSortArmModule::DoPickFromLoader（黃）

> 覆核：ISSUES（已依覆核修正）。缺口代碼：SP-1, SP-2, SP-3, SP-4

1. 取料梯 (aSortArm.cpp:1537-1727) 大半是可重發的馬達段,HOME 後由 case 1 從 tray grid + owner handshake 全部重推導,天生可重入。
2. 唯一紅色視窗:PickTask 50→54 之間「真空已 ON 但尚未 TransferPickDataFromLoader」的針 — uHome case 100 先歸 SuckZ(uHome.cpp:567-613,只動 MSuckZ 馬達 572-574,全檔不碰 sucker 輸出)→ 真空留 ON、IC 被帶上天;HOME 完成才 InitialAllTask(true)→InitialFlag→ClearSlot 只 Reset()(csystem.cpp:1359;MyKitSuck.cpp:52-57 不碰輸出)→ bHasIC=false = 無主 IC 藏針上(SP-1,今日即成立);「重扎同一空穴」僅規劃協定下成立(今日 uHome case 200 強制移盤+ClearTray 已洗 grid,uHome.cpp:659-668)。
3. 吸錯誤針本身安全:Suck() 逾時即 OffSuck(MyKitSuck.cpp:155);bPickSuckErr/iPickRetryCount 被 wipe 後 resume 以全新預算重試(今日盤被強制移除=扎下一盤;規劃協定下重試同格),自癒。
4. 已 commit(bHasIC)的針走 keep-material:InitialFlag 保 IC 並再吸(aSortArm.cpp:182-195),resume 直接去 place;但 PickX/PickY 被歸零(187-188)→ HOME 期間掉落的歸因/回填能力喪失(SP-2)。
5. sticky side-commit iActiveLoaderNo 被歸零(aSortArm.cpp:145)→ 規劃協定下半盤 tray 回位時可能中途換邊,重現 6/25 軌道停滯型態(SP-3)。
6. AreAllSuckersHome 為活感測器互鎖(571-597,614),無 latch,HOME 後自動成立;owner handshake 由 case 1/40 重驗、Loader InitialFlag 歸零 iYOwner(aLoader.cpp:92-93),自我重推導。
7. 本梯無可收斂的純氣缸相位:drain 階段不應驅動它,應改做「真空對帳」(SP-1 修法)後凍結。
8. 判定 yellow:三個缺口都是可列舉的小 guard/留 latch 修正,無需機構決策(SP-1 的「Z 在下位斷真空 IC 留穴內」為物理推論,建議向 owner 口頭確認)。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| 選格+取軌 (dispatcher) | PickTask 1 | FindPickCells 從 Loader tray grid 重推導單顆選格 (aSortArm.cpp:1548,783-844);AcquireSortOwner 取 Loader-Y 專屬權 (1554→aLoader.cpp:786-801) | Tray.Data 格資料 + fHasTray + IsLoaderReadyForSort | AcquireSortOwner 單掃描寫 iYOwner+LS_SORTING(已持有時冪等回 true,aLoader.cpp:792-793)【VERIFIED】 | 相位起點 | — |
| Z 升安全位 | 10 | SortArmZToSafePos:4 支 SuckZ MotorMove(10) (1563,682-693) | 馬達 InPos | 無 | 冪等段 | 不可(馬達) |
| Pitch 對盤距 | 20 | MovePitchToTrayPitch (1568,668-680) | 馬達 InPos | 無 | 冪等段 | 不可(馬達) |
| XY 接近 | 30 | MoveToLoaderPick:SortArmX+LoaderY (1573,704-739);X 受 AreAllSuckersHome 互鎖 (614) | 馬達 InPos + suck-Z Home LED 活讀 | 無 | 冪等段 | 不可(馬達) |
| 落針前複驗 | 40 | IsSortOwnerHeld 重驗 handshake (1580→aLoader.cpp:820-829),失敗→70;MoveToLoaderPick 再確認到位 (1585) | iYOwner+LS_SORTING + 馬達 InPos | 無 | 冪等段 | 不可(馬達) |
| Z 下降 | 45 | IsResidueCheckBusy 閘 (1591);MovePickZDown 只降 bCanPick 針 (1593,751-765);StartPnpSettle | 馬達 InPos | 無 | 危險段(預設 XY 已對位,僅經 30/40 前綴可達) | 不可(馬達) |
| 貼附延時 | 47 | PnpSettleElapsed 純 timer,Z 在下位、真空尚未下令 (1600-1603)【VERIFIED:吸取在 case 50 才 On()】 | HTimer | 無 | 冪等段 | 可收斂→50(僅到 50「入口」;無馬達無真空,收斂無物理效果,絕不可執行 case 50 本體) |
| 吸取行程 | 50 | SuckSelectedSlots:首掃描 On()=真空 ON (MyKitSuck.cpp:125),等 reed+dwell;逾時→OffSuck+bPickSuckErr latch (MyKitSuck.cpp:155,aSortArm.cpp:1108);全部完成且無錯→Transfer→60,有錯→iPickRetryCount++→52 (1605-1623) | 真空 reed (Sensor.IsOn) + OnDelay/OnAlarm timer | TransferPickDataFromLoader 單掃描:格→EMPTY_IC+bHasIC=true+trace (1618,1324-1363)【VERIFIED 同掃描完成】 | 危險段(預設 Z 已壓在 IC 上;真空 ON→commit 間為無主視窗) | 不可(真空交接) |
| 重試升針 | 52 | SortArmZToSafePos 升安全,預算內 ClearPickSuckErrors→40,超過→54 (1625-1642) | 馬達 InPos + iPickRetryCount vs GeneralSetting | 無(latch 操作可重來) | 冪等段 | 不可(馬達) |
| 吸錯模態 | 54 | ShowSuckError 模態 K_RETRY\|K_SKIP\|K_TRAY_END (1659,note.cpp:873-881);SKIP:SkipErroredPickCells+Transfer→60 (1664-1669);TRAY_END:另加整盤 ChangeActiveTrayData 兩次 wipe→60 (1670-1686);RETRY→40 | 操作員按鍵(Note dismiss 受 recovery 選擇閘) | 按鍵返回後同一掃描內完成全部格資料改寫+Transfer【VERIFIED 無中間掃描】 | 原子commit | 不可(模態+材料資料交接) |
| 提升+落品檢+交軌 | 60 | SortArmZToSafePos;到頂後 CheckHoldFallDown(true) 掉落檢 (1701,1432-1535);ReleaseSortOwner+PickTask=1+return true (1703-1705) | 馬達 InPos + 持有針活真空 reed(100ms 去彈跳,aSortArm.cpp:43) | ReleaseSortOwner 單掃描 LS_SORTING→LS_ToRear (aLoader.cpp:804-816) | 冪等段 | 不可(馬達+custody 確認) |
| 交還跳出 | 70 | Z 升安全→ReleaseSortOwner+清 latch/selection→回 1 (1709-1720) | 馬達 InPos | 單掃描清場 | 冪等段 | 不可(馬達) |

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| PickTask 1(含 DoSortArm case 1 選邊) | InitialFlag 歸零 PickTask/iActiveLoaderNo (aSortArm.cpp:143,145);Loader InitialFlag 歸零 iYOwner (aLoader.cpp:92-93);uHome case 200 強制移盤+ClearTray → resume 全部由感測器/grid 重推導 | 今日綠;規劃協定下 tray 回位但 sticky 邊遺失 → SP-3 黃 |
| 10/20/30/40 接近段(IC 全在盤上) | uHome case 100 先歸 SuckZ(空針)再 case 200 歸 XY(符合 A2);今日盤被強制移除=材料交回操作員不遺失;HOME 完成後選格才被 wipe;規劃協定下盤回位、resume 由 FindPickCells 重選同格 | 綠(選格為純推導,無 latch 依賴) |
| 45/47(Z 已下、真空未開) | 空針(真空尚未下令)隨 uHome case 100 上升,IC 留在穴內;HOME 完成 ClearSlot 僅清選格,grid 一致 | 綠 |
| 50(真空 ON、未 Transfer)+ 52/54 內已吸成功未 commit 的針 | uHome case 100 先歸 SuckZ(uHome.cpp:567-613;只動 MSuckZ_1..4,572-574,全檔無 Sucker/OnSw 參照)→ 真空留 ON、IC 被帶上天;HOME 完成才 InitialAllTask(true)→InitialFlag→ClearSlot 只 Reset()(csystem.cpp:1359;aSortArm.cpp:224-228;MyKitSuck.cpp:52-57 不碰輸出)→ bHasIC=false、真空仍 ON = 針上藏無主 IC(壓傷/疊料),CheckHoldFallDown 不看它(1446 只查 bHasIC);今日 uHome case 200 強制移盤+ClearTray 洗掉 fHasTray+grid(659-668)→ FindPickCells(791 fHasTray 閘)只會選下一盤新格;「重扎同一空穴」僅在規劃 park/re-acquire 協定(grid 保留)下成立 | 紅 → SP-1(無主 IC 今日即成立;同穴重扎為規劃協定下的加重情境;sim 不受影響,1092-1093) |
| 52/54(錯誤 latch 針本身) | 錯誤針真空已被 Suck() 逾時 OffSuck (MyKitSuck.cpp:155);bPickSuckErr/iPickRetryCount 被 wipe (aSortArm.cpp:172,154) → IC 仍在盤,resume 以新預算重試(今日盤被強制移除=扎下一盤新格;規劃協定下 grid 保留=重試同格) | 黃(錯誤針自癒;但同批吸成功針落入上列紅視窗;54 模態阻塞掃描,HOME 只能在按鍵套用後或 EMG 下落入) |
| 60(已 Transfer,bHasIC=true) | keep-material:InitialFlag 保 IC、Sucker->On() 再吸 (182-195);vacuum 全程無人關(uHome 不碰);resume case 1→HasHoldingIC→place;place 段 10-35 連續 falldown 檢 (1775-1779) 接手掉落 | 黃 → SP-2:PickX/PickY 被歸零 (187-188),HOME 中掉回 Loader 盤的 IC 只能以 transit 語意 SKIP、不回填格資料;規劃協定下盤帶散落 IC 被排後 |
| 70(交還跳出) | 純清理段,wipe 後 resume 等價 | 綠 |

### 缺口

- SP-1(紅→修後綠):真空 ON 但未 commit 的針在 HOME 成無主 IC — uHome case 100 先歸 SuckZ(uHome.cpp:567-613 只動 MSuckZ_1..4,572-574,全檔不碰 sucker 輸出)把 IC 帶上天,HOME 完成才 InitialAllTask(true)→ClearSlot 只 Reset()(csystem.cpp:1359;ClearSlot aSortArm.cpp:224-228;MyKitSuck.cpp:52-57 不碰 OnSw)→ bHasIC=false、真空留 ON【VERIFIED】;「grid 失真+重扎同穴」僅規劃協定下成立(今日 uHome case 200 強制移盤+ClearTray 已洗 fHasTray+grid,659-668)。最小修法:HOME 進場、uHome case 100 歸 SuckZ 前做「真空對帳」(此時 bCanPick 尚未被 wipe):GetOnBit()==ON 且 (bCanPick && !bHasIC) → OffSuck() 趁 Z 仍在下位讓 IC 留穴(IC 留穴為物理推論【INFERRED】,建議 owner 確認);EMG/斷氣成因跳過對帳,改於完成後逐針讀 reed 盤點+報警。
- SP-2(黃):InitialFlag(bKeepMaterial) 對保留 IC 仍歸零 PickX/PickY(aSortArm.cpp:187-188)→ resume 後掉落只能走 CheckHoldFallDown(false) 的 SKIP-only 語意,喪失 bAtPick 回填 Loader 格能力(1527-1528),規劃協定下 Loader 盤可能帶散落 IC 排後【VERIFIED 邏輯鏈/後果 INFERRED】。修法:keep-material 分支保留 PickX/PickY(僅清 bCanPick/bPlaceSelected),或 HOME 完成時先逐持有針讀真空 reed 對帳。
- SP-3(黃):sticky side-commit iActiveLoaderNo 被 InitialFlag 歸零(aSortArm.cpp:145);規劃協定下半盤 tray 回位、兩側 READY_SORT 時 GetSortingLoaderNo 偏好 side 1(aSortArm.cpp:2033-2035 註解【VERIFIED 註解/行為 INFERRED】)→ 中途換邊、被棄車停 sort zone 的 6/25 停滯型態重現。修法:bKeepMaterial 且該側 HasPickableIC 時保留 iActiveLoaderNo,或 resume 以車位重推導 sticky 邊。今日無此問題(盤被強制移除)。
- SP-4(綠,確認項):AreAllSuckersHome 活感測互鎖(aSortArm.cpp:571-597,614-626)與 owner handshake(case 1 取得/40 重驗/aLoader.cpp:92-93 wipe)皆無殘留 latch,HOME 後自我重推導成立;pick-retry latch wipe 亦自癒 — 無需修改。

### drain 邊界

DoPickFromLoader 沒有任何可收斂的純氣缸相位:cases 10/20/30/40/45/52/60/70 全數含 Motor Move*(SortArmZToSafePos/MovePitchToTrayPitch/MoveToLoaderPick/MovePickZDown,aSortArm.cpp:682-765),case 50 是真空(材料)交接、case 54 是操作員模態+單掃描格資料交接,依 drain 規則一律「不可」;唯一例外 case 47 為純 PnpSettle timer(Z 已在下位、真空尚未下令,1600-1603),可收斂到 case 50 的「入口」邊界,但收斂毫無物理效果且 drain 器必須在 PickTask 變 50 後立即停掃、絕不可執行 case 50 本體(那就是吸取交接)。因此 HOME drain 階段對本梯的正確處置不是收斂而是「凍結+真空對帳」:在 uHome case 100 歸 SuckZ 之前,對每支 GetOnBit()==ON 且 (bCanPick && !bHasIC) 的針執行 OffSuck() 讓 IC 趁 Z 在下位留回穴內(即 SP-1 修法);bHasIC==true 的針保持真空(InitialFlag 於完成時會再吸,aSortArm.cpp:194);HOME 成因為 EMG/安全門/斷氣時跳過對帳,改於完成後以真空 reed 逐針盤點+報警。

## MarkResidueTargets / CheckPlaceResidue / IsResidueCheckBusy / TAutoModule::SetPlaceResidueClear（黃）

> 覆核：OK。缺口代碼：SR-1, SR-2, SR-3, SR-4, SR-5

殘料驗證是唯一跨 place 週期存活的背景 ladder:place case 50 武裝(bNeedResidueCheck+Auto bResidueClear=false, aSortArm.cpp:1829-1832)、case 70 啟動(bResidueArmed=true, :1891)、每 scan 由 DoSortArm 頂端泵送(:2008)破吹→再吸→判讀,收斂時單 scan 回報 SetPlaceResidueClear(true)(:1272-1280)。今日 HOME 是雙側全滅:SortArm InitialFlag 無條件清 bNeedResidueCheck/ResidueTask/iResidueAutoIndex/bResidueArmed(:155-161,不看 bKeepMaterial),Auto InitialFlag 在 bKeepMaterial early-out 之前強制 bResidueClear=true(aAuto1To6.cpp:94,在 :107 continue 之前)→ 未完成的驗證被靜默判 PASS,保留 bFullIC 的 Auto 照樣 discharge(:417)/AGV Ready(:1301)。最險視窗:ResidueTask=200/300 真空 ON 時 HOME——TMySucker::Reset 不碰輸出(MyKitSuck.cpp:52-57)、InitialFlag 只清 blow(:162-171)、CheckIsFallDown 無人呼叫 → 殘料 IC 被隱形持住,下次 pick Z-down 壓碎;且殘料 modal 只給 K_RETRY(:1253),操作員最可能就是用 HOME 逃出,正中此窗。修復可枚舉、無需機構決策:Auto 端把 :94 移到 keep-material continue 之後(SR-1)、SortArm 端 keep-material 保留 latch 並在 HOME 後(SuckZ 已在頂)重武裝重驗(SR-2);CheckPlaceResidue 全段零 Move 呼叫,可納入 HOME drain 第(1)階段跑到 bAllDone,但 EMG/air-fail 跳過 drain 時 retention 是唯一底線(SR-5)。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| ARM commit(武裝) | DoPlaceToAuto case 50(aSortArm.cpp:1827-1856) | DestroySelectedSlots 完成後同一 scan:MarkResidueTargets()(bNeedResidueCheck[s]=true、ResidueTask[s]=1,:1184-1191)+ iResidueAutoIndex=iActiveAutoIndex(:1830)+ AutoModule->SetPlaceResidueClear(idx,false)(:1832)+ 捕捉 bBlowSlot、re-assert blow + TransferPlaceDataToAuto、PlaceTask=55 | DestroySelectedSlots()==true(真空破除 reed/timer,:1294-) | 全部寫入在 case 50 判真後的單一 scan 完成(VERIFIED:case body 無中途 break) | 原子commit | —(單 scan,無可停點;其前綴 case 40 Z-down 屬 DoPlaceToAuto 之馬達段) |
| 待武裝窗(armed-pending) | DoPlaceToAuto case 55/60/70(:1859-1894) | blow dwell → SortArmZToSafePos(馬達 Z-up)→ blow OFF → bResidueArmed=true(:1891)。此窗 CheckPlaceResidue 直接 return true(:1202-1203)但 IsResidueCheckBusy()=true 已封鎖 pick Z-down(:1591)與 CleanOut finish(:1918) | BlowDwell HTimer + SortArmZToSafePos 到位 | bResidueArmed=true 與 PlaceTask=1 同 scan(:1891-1893) | 危險段(僅能經 case 50 前綴進入;今日 HOME 落此=驗證被滅、Auto 閘門被強制放行) | 不可(case 60 含 SortArmZToSafePos 馬達 Z 移動) |
| 驗證-破吹 | CheckPlaceResidue ResidueTask[s]=1(:1234-1240) | Sucker->OffDestroy() + ResidueDelay[s]=iDestroyCheckMS 啟動 | HTimer(ResidueDelay,pause-aware :128-138) | ResidueTask=200 | 冪等段(重跑僅重發 OffDestroy,solenoid 冪等) | 可收斂→bAllDone(:1272) |
| 驗證-再吸 | ResidueTask[s]=200(:1241-1247) | Sucker->OnSuck()(真空 ON,MyKitSuck.cpp:76-79)+ delay 重啟 | HTimer | ResidueTask=300 | 冪等段(OnSuck 冪等;但真空輸出自此 ON——見 SR-3) | 可收斂→bAllDone |
| 驗證-判讀 | ResidueTask[s]=300(:1248-1266) | delay 到期讀 GetStatus()(REALLY 真空 sensor,MyKitSuck.cpp:69-74):ON=殘料→modal ShowSuckError(K_RETRY only,:1253)→回 200 重驗;OFF=Sucker->Normal()(真空+吹氣全關)+ bNeedResidueCheck=false、ResidueTask=1 | 真空 sensor + HTimer | 每 slot verdict 單 scan 寫入 | 危險段(殘料 verdict 彈 modal;HOME 落此=已知殘料被遺忘且真空鎖 ON) | 可收斂→bAllDone(殘料分支需 drain-aware:不得 pump modal,超時 fallback 保留 latch) |
| 完成 commit(回報) | CheckPlaceResidue bAllDone(:1272-1280) | SetPlaceResidueClear(iResidueAutoIndex,true)+ iResidueAutoIndex=-1 + bResidueArmed=false | 全 slot bNeedResidueCheck==false | 單 scan(VERIFIED,三寫入同 if-block) | 原子commit | — |
| 非REALLY/sim 旁路 | :1204-1215(sim)、:1221-1224(iRealDummy!=REALLY) | 直接清 latch + 回報 clear=true | IsSoftSimulate / iRealDummy | 單 scan | 冪等段 | 可收斂→bAllDone |
| Auto 閘門(消費端) | FindDischargeAuto(aAuto1To6.cpp:417)、IsDrainedForAmr(:1297-1302);重置點 :94/:695/:1331 | bFullIC && bResidueClear 才選 discharge;AGV Ready(CEID273)需 bResidueClear;discharge case 1000 / ClearAmrCar 各自重置 clear=true(新盤/新車) | State[Index].bResidueClear | Set 為單一 bool 寫入(:1251) | 相位起點(純讀 dispatcher) | — |

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| W1 閒置(bResidueArmed=false 且無 bNeedResidueCheck) | InitialFlag 清 residue 欄位=no-op;Auto bResidueClear 本已 true | 無材料在途,重入乾淨 | 綠 |
| W2 待武裝窗(place case 50 已 commit ~ case 70 未達) | SortArm InitialFlag 滅 bNeedResidueCheck/iResidueAutoIndex(aSortArm.cpp:155-161);Auto :94 強制 clear=true(在 :107 keep-material continue 之前)→ 驗證靜默跳過;blow latch 由 :162-171 清掉;保留 bFullIC 的 Auto 可 discharge(:417)/AGV 離站(:1301);若真有殘料 IC → 短一顆的「滿盤」出貨 + 下次 pick 壓碎 | 紅。規劃協定:武裝前綴含馬達(case 60)不可 drain;但 HOME 本身先升 Z(uHome case 100 homes SuckZ,A2)→ 修復=保留 latch(SR-1/SR-2)+ HOME 後重武裝重驗 |
| W3 驗證中 ResidueTask=1(破吹) | 同 W2 靜默放行;真空此刻 OFF,物理風險較低,但偵測能力已丟失 | 紅(偵測丟失)。drain 第(1)階段可直接把本段跑到 bAllDone(零 Move,VERIFIED :1194-1282) |
| W4 驗證中 ResidueTask=200/300(真空 ON) | 靜默放行 + OnSw 真空輸出殘留 ON:TMySucker::Reset 不碰輸出(MyKitSuck.cpp:52-57)、ClearSlot 只 Reset(:224-228)、InitialFlag 只清 blow、CheckIsFallDown 定義了但無人呼叫(全 repo 僅定義處)→ 殘料 IC 被隱形持住穿越 HOME,resume 後系統認定 nozzle 空;下次該 slot pick Z-down = 雙 IC 壓碎/誤置 | 紅(最危險)。修復=SR-2 retention(重驗自然重新管理真空)或退場時強制 Sucker->Normal()+告警(SR-3) |
| W5 殘料 modal 開啟(:1253,K_RETRY only) | 操作員只有 RETRY 可選,逃生最可能就是觸發 HOME → 直落 W4:已知殘料被遺忘、真空鎖 ON | 紅。此窗不是理論——modal 無 SKIP 使 HOME 成為事實逃生門;drain 對殘料 verdict 必須 abort-fallback 保留 latch,不 pump modal(SR-4) |
| W6 完成 commit 前/後(:1272-1280) | 三寫入單 scan,HOME 只會落在其前(=W3/W4)或其後(=W1),不會落在中間 | 綠(原子) |
| Auto 消費端(HOME 期間) | FindDischargeAuto/IsDrainedForAmr 在 HOME 中不被泵送;危害發生在 resume 後(閘門已被 :94 強制放行) | 併入 W2-W5 判定;獨立缺口=SR-1 |

### 缺口

- SR-1(紅):aAuto1To6.cpp:94 InitialFlag 在 bKeepMaterial early-out(:107-108)之前無條件 State[].bResidueClear=true → keep-material HOME 把未完成驗證判 PASS,bFullIC 保留的 Auto 直接 discharge/AGV Ready。最小修:把該行移到 continue 之後(僅 full-wipe 清),keep-material 保留 false 閘門。
- SR-2(黃):aSortArm.cpp:155-161 InitialFlag 不分 bKeepMaterial 全滅 bNeedResidueCheck/ResidueTask/iResidueAutoIndex/bResidueArmed → 驗證永不恢復、SR-1 的閘門也失去解鈴人。最小修:bKeepMaterial 時保留 bNeedResidueCheck+iResidueAutoIndex,ResidueTask 歸 1,並於 HOME 完成後重設 bResidueArmed=true(SuckZ 已被 uHome case 100 homed 在頂,滿足「絕不在盤附近再吸」武裝前提)→ 背景 ladder 自行重跑收斂,verdict 重新回報 Auto。
- SR-3(紅):HOME 落在 ResidueTask=200/300 時 OnSw 真空殘留 ON——TMySucker::Reset(MyKitSuck.cpp:52-57)僅清 Task/Delay/Error,InitialFlag 只清 blow(aSortArm.cpp:162-171),CheckIsFallDown(MyKitSuck.cpp:259)全 repo 無呼叫者 → 殘料 IC 隱形持住,下次 pick Z-down 壓碎。最小修:SR-2 retention 即由重驗接管真空;若決定放棄 latch 的路徑,退場前必須 Sucker->Normal()+ 一次性告警,不得靜默。
- SR-4(紅→修後黃):殘料 modal(:1253)僅 K_RETRY,HOME 是操作員事實上的逃生門,直落 SR-3 視窗。最小修:同 SR-1/SR-2 retention;規劃 drain 階段遇殘料 verdict 採 abort-fallback(保留 bResidueClear=false + bNeedResidueCheck),嚴禁在 drain 中 pump modal。
- SR-5(黃,drain 設計):CheckPlaceResidue(:1194-1282)全段純 sucker solenoid + HTimer、零 Move 呼叫、nozzle 保證在 Z 頂(:1891 武裝前提)→ 可納入 HOME drain 第(1)階段跑到 bAllDone 邊界(完成 commit :1272-1280 自然回報 clear)。但 EMG/air-fail 跳過 drain 時 re-suck 無氣源不可信 → retention(SR-1/SR-2)是底線,drain 只是優化。

### drain 邊界

可 drain 的精確邊界:CheckPlaceResidue 本體(aSortArm.cpp:1194-1282)是純 sucker solenoid(OffDestroy/OnSuck/Normal)+ HTimer(ResidueDelay,已接 pause/restart :128-138),整段零 motor Move、零材料 handoff(再吸只驗空嘴,武裝前提保證 nozzle 在 Z 頂,:1868/:1891),HOME drain 第(1)階段可將其泵送到 bAllDone 邊界——即全 slot bNeedResidueCheck=false,完成 commit(:1272-1280)單 scan 回報 SetPlaceResidueClear(true) 並解除 bResidueArmed。不可 drain:武裝前綴 DoPlaceToAuto case 55→70(case 60 SortArmZToSafePos 為馬達 Z-up)——HOME 落在 W2 時不要試圖在 drain 內完成武裝,改走 retention + HOME 後重武裝(馬達 home 本身已把 SuckZ 送頂)。drain 中遇殘料 verdict(:1253)不得彈 modal:視同 drain 超時 fallback,保留 bResidueClear=false 與 bNeedResidueCheck latch 交給 resume 後重驗;EMG/safety-door/air-fail 跳過 drain 時(air-fail 下 re-suck 讀值不可信)retention 是唯一正確路徑。

## TAutoModule::DoFeedTray (含 FindFeedAuto / RefreshAutoState / StageRearGrid / NotifyTrayArmDelivered / SetRearHasTrayFromTrayArm)（黃）

> 覆核：ISSUES。缺口代碼：AF-1, AF-2, AF-3, AF-4, AF-5

DoFeedTray(aAuto1To6.cpp:464-655) 把 TrayArm 暫存於 rear 的盤(RearGrid/RearKind/RearTrayID,aTrayArm.cpp:798/804/806 於 Z 抬升確認後才 latch)拉上 Auto 車:選站閘=bCarHasTray==false && bRearHasTray && bRearDeliveredPending(FindFeedAuto :402);case 200 動前感測交叉核對(JAM%d11 Retry/Skip);case 7000 為 VERIFIED 單掃描原子提交(CopyFrom(RearGrid)+fHasTray=true+rear latch 消費,:615-652)。危險窗=case 4000 起夾持後、7000 未執行前:fHasTray 仍 false,HOME 後 PARK(只認 fHasTray)不會釋放此軸(AF-1),且 FeedTask>=6000 時盤已離 rear、latch 未消費,復歸必踩 case-200 假 JAM,Skip 清 RearGrid 產生無身分幽靈夾持盤+二次進料撞擊(AF-2)。可收斂 drain 區={4000,5000,5100,5200}(+3000 感測 gate)→邊界 6000,但 3000/5200 內含 modal 告警需抑制(AF-3)。今日 uHome 完全不碰 Auto Lean/Push(僅 Loader,uHome.cpp:548-562)卻批次歸 MAutoY_1..6(uHome.cpp:621-624),EMG/alarm 實體重歸=夾持拖盤(AF-4,規劃 PARK 即解)。修 AF-1+AF-2 後全窗收斂,無需新機構決策(A1 已確認、7000 提交純資料)。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| 初始化 | Flag==0 (:473-479) | FeedTask=1、iFeedAuto=-1、FeedDelay.Clear;呼叫點=DoAuto case1000(:1596)/DoAllAutoCleanOut case500/600(:781,:790) | 無(純重置) | 無 | 相位起點 | — |
| 起步 | 1 (:483-485) | 空轉→100 | 無 | 無 | 相位起點 | — |
| 選站 | 100 (:487-493) | FindFeedAuto(:385-407):RefreshAutoState 後取 bCarHasTray==false && bRearHasTray && bRearDeliveredPending(:392,:402);無站→return true;中選→Status=AS_LOADING | rear 感測(latch 保護 :355-361)+ TrayArm 交付 latch | 無 | 相位起點 | — |
| 動前核對 | 200 (:495-533) | IsSensorOnReady(OutputBottomHasTray) 交叉核對;失敗→JAM%d11 modal(:516);Skip=單掃描清 bRearHasTray/bRearCanUse/pending/RearGrid/RearKind/RearTrayID+AS_IDLE 並 return true(:522-529, VERIFIED);Retry=下掃再讀 | SnAutoN_OutputBottomHasTray(:288-300) | Skip 為單掃描棄置提交 | 冪等段 | —(無缸無 Move;含 modal) |
| 就位 | 1000 (:535-538) | MoveAutoY→GetAutoFeedY(空車) | MotorMove 完成(:302-314,可重發) | 無 | 冪等段 | 不可(motor) |
| 到位再核 | 3000 (:540-551) | rear 感測再讀;失敗→WAR%d30 Retry→1000(:547-549) | 同 case200 感測 | 無 | 冪等段 | 可收斂→6000(本 case 無 Move;Retry 跳回 1000 時 drain 須中止;含 modal) |
| 夾持-後靠 | 4000 (:553-557) | Lean(GetLean :218-230).Push() | 缸 reed(冪等快進) | 無 | 冪等段 | 可收斂→6000 |
| 夾持-前推 | 5000 (:559-567) | Push(GetPush :204-216).Push()+settle timer(iAutoPushConfirmSettleMs) | 缸 reed+HTimer | 無 | 冪等段 | 可收斂→6000 |
| 夾持確認 | 5100 (:569-578) | timer 到→IsCylinderOnReady(Push)(mycylin.cpp:24-33)→6000/5200 | Push OnSensor | 無 | 冪等段 | 可收斂→6000 |
| 推失重試 | 5200 (:580-589) | Push.Pop()+JAM%d02 modal(:585);Retry→5000 | 缸 reed+操作員 | 無 | 冪等段 | 可收斂→6000(經 5000 重試;含 modal,須計 timeout) |
| 載盤移動 | 6000 (:591-605) | MoveAutoY→GetSortArmCellY(FirstSortY,0)(實體盤已夾在車上、離開 rear) | MotorMove 完成 | 無 | 冪等段(Move 可重發;但盤已離 rear→中斷後見 W5) | 不可(motor) |
| 提交 | 7000 (:607-652) | 單掃描:Tray.CopyFrom(RearGrid)(:615)+fHasTray=true(:616)+Refresh;bCarHasTray=true、bRearHasTray/bRearCanUse=false、bFullIC=false、pending=false、AS_SORTING(:619-624);AMR:WorkingKind/ID←RearKind/ID、Car[] 堆疊登錄、identity→CarID、非 normal→bFullIC+AS_FULL(:629-651);return true(:652) | 無感測(純資料) | 原子 commit(VERIFIED:同一 case 執行內完成、無中途 break/modal) | 原子commit | — |

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| W1 空閒/case1/100(未選站) | InitialFlag(true)(:72-123)無條件重置 FeedTask=1(:76)但 keep-material 於 :107-108 continue,rear latch(bRearHasTray/pending/RearGrid/RearKind/RearTrayID)全保留;RefreshAutoState pending 保護(:355-361)防感測 OFF 抹除;復歸重派乾淨 | 綠 |
| W2 case 200(核對中/JAM modal 開啟) | 重置後從 case1 重跑核對;盤仍在 rear staging→感測 ON→照常進料;Skip/Retry 語意不變 | 綠(modal 開啟中之 HOME 進入屬 Note modal-pump 議題,非本梯) |
| W3 case 1000(空車 Y 移動中) | uHome case200 批次歸 MAutoY_1..6(uHome.cpp:621-624);已 home 無 alarm 之 servo 跳過(uHome.cpp:636-640, :257-262);車上無盤→無拖行;復歸重發 Move | 綠 |
| W4 case 4000–5200(feed Y 夾持中,盤仍壓 rear 感測,fHasTray=false) | uHome 不放 Auto Lean/Push(grep 證實僅 Loader :548-562);servo-skip 時 Y 不動→復歸 case200 感測 ON→全程冪等重播至 7000 收斂(綠);EMG/servo-alarm/非 skippable 實體重歸→夾持拖盤(紅) | 黃:規劃 drain 收斂至 6000+PARK 釋放即解,但 PARK 判準須涵蓋 fHasTray=false 之已夾持態(AF-1);drain 含 modal 路徑(AF-3) |
| W5 case 6000 移動中/FeedTask==7000 未執行(盤在車上、已離 rear、fHasTray=false、rear latch 未消費) | (a)實體重歸→拖盤;(b)復歸 FindFeedAuto 再中選→case200 感測 OFF→JAM%d11 假警報;Retry 永不成功;Skip 清 RearGrid/身分(:522-528)→車上幽靈夾持盤、站顯示全空→TrayArm 可再送盤、下次 feed 對已占用車二次拉盤→撞擊 | 紅(今日):規劃 drain 無法跨 6000(motor);需 AF-2 提交前移;修後綠 |
| W6 case 7000 已執行(working tray,fHasTray=true) | 資料一致;InitialFlag :91 由 fHasTray 再導出 bCarHasTray,CheckAutoTray(:365-383)重建 AS_SORTING/bFullIC;僅剩夾持拖盤風險 | 綠(規劃 PARK 以 fHasTray 正常涵蓋) |
| W7 CleanOut 重用(DoAllAutoCleanOut case500/600→DoFeedTray :780-795;case7000 清 RearGrid :937-953) | HOME 重置 CleanOutTask=1 且清 bCleanOutFinish(:93)→drain 梯整段重跑;晚送自癒(:447-457, :1200-1206)防 wedge | 綠(伴隨 W4/W5 同缺口) |

### 缺口

- AF-1(黃):規劃 PARK 以 fHasTray 選「載盤軸」會漏掉 FeedTask 4000–7000未執行 的已夾持窗(fHasTray 於 :616 才設)→該 AutoY 不釋放即歸 home=拖盤;修:PARK 判準改用 Lean/Push OnSensor reed(或 FeedTask∈[4000,7000)),釋放順序照 A1 Lean→Push。
- AF-2(黃,修前紅):FeedTask>=6000 落點復歸後 rear latch 未消費+感測 OFF→case200 假 JAM(:516),Skip 清 RearGrid(:522-528)留下無身分幽靈夾持盤+二次進料撞擊;修:HOME 入口(drain 收尾)對 FeedTask∈{6000,7000} 先執行 case7000 單掃描純資料提交(:607-652,無任何 motion)再 InitialAllTask(true)。
- AF-3(黃):drain 集合內 case3000/5200 各含 ShowMyError modal(:547,:585),drain 階段彈 modal 會卡死 HOME 且 timeout 越不過 modal;修:drain 期間告警路徑改走 timeout fallback(失敗即放缸退回 rear-latch 狀態),不彈 modal。
- AF-4(今日紅/規劃步驟2即解):uHome 對 Auto 零缸動作(僅 Loader :548-562)卻批次歸 MAutoY_1..6(uHome.cpp:621-624),EMG/alarm/stepper 之實體重歸對夾持中或 working 盤一律拖行;確認 PARK 覆蓋全部 6 個 Auto 站(與 AF-1 合併驗收)。
- AF-5(綠/觀察):FindFeedAuto(:390-403)不查 bAmrLocked(FindDischargeAuto :415 有查)→AMR handoff 中 rear→car 拉盤仍會跑;與 HOME 無直接關聯,若 handoff 期間 AutoY 移動有干涉疑慮需 owner 確認(INFERRED,可能是刻意:AMR 介面在前段 FrontRise)。

### drain 邊界

DoFeedTray 的 HOME drain 可收斂區=FeedTask∈{3000(僅感測 gate)、4000、5000、5100、5200},邊界=6000:此區間只有 Lean/Push 的 Push()/Pop()、HTimer settle 與 reed 讀取,無任何 MoveAutoY、無真空/材料交接(VERIFIED :540-589);drain 泵的停止條件須是「FeedTask==6000(邊界達成)或 FeedTask 跳出集合(case3000 Retry 會設回 1000=motor 段,必須立即中止)或 timeout」,且 3000/5200 的 ShowMyError modal 在 drain 期間必須改走 timeout fallback(AF-3)。case 1000 與 6000 內含 MoveAutoY(:536,:600)故不可 drain;case 7000 是單掃描純資料提交、經由梯形圖無法被 drain 觸及(被 6000 的 motor 擋住),建議依 AF-2 於 FeedTask>=6000 時把 case7000 提交 fast-forward 執行(資料無 motion,先提交再 PARK),使該軸以正常「fHasTray=true 載盤軸」身分進入 PARK/RE-ACQUIRE,復歸時 FindFeedAuto 不再誤選、case200 假 JAM 消失。

## TAutoModule::DoDischargeTray (aAuto1To6.cpp:657-751; 尾段共用 TAutoModule::DoFrontRiseOnce aAuto1To6.cpp:1646-1678)（黃）

> 覆核：OK。缺口代碼：AD-1, AD-2, AD-3, AD-4

DoDischargeTray 的 case-1000 是「到位即 commit」：MoveAutoY 回 true 的同一 scan 內完成 ClearTray+bFullIC=false+bFrontHasTray=true 等 9 項寫入 (aAuto1To6.cpp:687-703, VERIFIED 單 scan、無 modal/等待)。commit 之後到 case-6100 riser 完成之間，實體滿 IC tray 仍在載台/前段預升位，但軟體已無任何在 keep-material HOME 後存活的記錄——RefreshAutoState 會用前疊 sensor 覆寫 bFrontHasTray (:334-340)，FindDischargeAuto 也不看前段殘 tray (:409-421)。今日 HOME 落在此窗 = 資料已清的滿 tray 被夾著拖回 home 或棄置於前段，之後 FindFeedAuto 可再餵 rear tray 疊車、或下一次 discharge 撞殘 tray → 紅。規劃 protocol 的 DRAIN 能把 3000/4000 收斂到 case-5000 邊界、把 6000/6100 收斂到完成，但 case-5000(馬達) 中斷後仍缺「收尾 latch」補打 FrontRise —— AD-1。另 uHome 只 home MAutoY (uHome.cpp:621-624) 不碰 Auto 缸，EMG 跳過 drain 時 FrontRise 可能卡 On (AD-2)；DoFrontRiseOnce 回 true 時 Off 未等 reed 確認 (:1671-1673)。InitialFlag 無條件 bResidueClear=true (:94) 會在 HOME 洗掉殘料閘 (AD-4)。皆為可列舉的小 latch/guard 修正 → yellow。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| 派工 | 1→100 | FindDischargeAuto 選站 (skip bAmrLocked，要求 bFullIC && bResidueClear，aAuto1To6.cpp:409-421)；Status=AS_DISCHARGING (:682)。無任何運動 | 純 latch 讀取：bFullIC 由 CheckAutoTray FullThisIC(HAS_OK_IC) 再導出 (:377-380)，keep-material 下亦保留 (:107-112) | 無 | 相位起點 (VERIFIED) | — |
| 進料位→出料位 | 1000 | MoveAutoY(GetAutoDischargeY) (:687)；到位的同一 scan：ClearTray(:692)、bFullIC=false(:694)、bResidueClear=true(:695)、bCarHasTray=false(:696)、bRearHasTray=false(:697)、bRearDeliveredPending=false(:698)、RearGrid.Clear()(:699)、bFrontHasTray=true(:700)、HGem EventReport CEID136-142(:701-702)、Task=3000(:703) | Motor->MotorMove 到位 (MoveAutoY :302-313) | 到位瞬間 9 項寫入，一個 scan 內無等待/modal (VERIFIED 單 scan) | 原子commit (到位觸發；中斷於移動中則 commit 未發生，Move 可重下=安全前綴) | 不可 (含 motor Move) |
| 鬆前擋 | 3000 | GetPush(i)->Pop() (:708-710)，釋放前擋缸 | TMyCylinder Pop reed (已達位則數 scan 內 fast-forward，idempotent) | 無 | 冪等段 (VERIFIED) | 可收斂→5000 |
| 鬆靠緊 | 4000 | GetLean(i)->Pop() (:714-716)；完成後 tray 自由躺在出料位 | Lean Pop reed | 無 | 冪等段 (VERIFIED) | 可收斂→5000 |
| 載台退開 | 5000 | MoveAutoY(GetAutoFeedY) 退回進料位 + 啟動 iAutoDischargePostYSettleMs settle (:720-725)。A1：雙缸皆 Off，Y 移動不拖 tray | Motor 到位 + timer 起算 | 無 | 冪等段 (Move 可重下；tray 已自由) | 不可 (motor Move) |
| settle | 6000 | 等 DischargeDelay.Off()，DischargeSubTask=1 (:732-736) | 純 timer | 無 | 冪等段 (VERIFIED) | 可收斂→完成 (與 6100 連續收斂) |
| GoUp 升舉 | 6100 | DoFrontRiseOnce：Rise->On()→IsCylinderOnReady→dwell iAutoFrontRiseDwellMs→Rise->Off()→return true (:1656-1676)；Status=AS_IDLE (:745)。注意：Off 只下指令即 return true，未等 Off-reed (:1671-1673, VERIFIED) | Rise On-reed + dwell timer；Off 無確認 | 無 (實體交接=tray 進前疊爪) | 冪等段 (空升無害：cleanout case 4000 對全 6 站無條件 pump riser 之先例 :859-901, INFERRED 可重升) | 可收斂→完成 (純缸+timer，無 motor、無真空) |

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| case 1/100 | 無運動、只寫 Status；InitialFlag(bKeepMaterial) 由 fHasTray 重導 Status (:91-92)，bFullIC 保留 (:107-112)，復歸後 FindDischargeAuto 重新派同一站 | 綠 (今日即可重入, VERIFIED) |
| case 1000 移動中(未到位) | commit 未發生；滿 tray 仍被 Lean+Push 夾在載台。uHome case 200 home MAutoY_1..6 (uHome.cpp:621-624) 時缸未釋放 → 夾著滿 tray 拖到 home 位；若 home 端近 rear 進料端可能撞 TrayArm 已 stage 的 rear tray (INFERRED 幾何)。復歸後 fHasTray/Tray/bFullIC 俱在，全程重跑 | 黃今日 (不受控拖行)。規劃 PARK(記 Y+identity→鬆缸→自由 home)+RE-ACQUIRE 後→綠；注意 Auto 自身釋放順序是 Push 先 Lean 後 (case 3000→4000)，與 plan 文字順序相反 (AD-3) |
| commit 後～case 3000/4000 | 資料已 ClearTray 但實體滿 tray 仍在載台(缸仍夾/半鬆)。今日：拖無資料滿 tray 回 home；復歸後 bCarHasTray=false → FindFeedAuto 可把 rear tray 餵進已佔用載台=tray 疊 tray；bFrontHasTray latch 雖 keep-material 保留，實機 RefreshAutoState 以前疊 sensor 覆寫為 false (:334-340) → tray 軟體隱形 | 紅今日 (VERIFIED 邏輯鏈)。DRAIN 可收斂 3000/4000 至 5000 邊界(純缸)，但復歸梯形從 1 重起、無人補 GoUp → 前段殘 tray、下次 discharge case 1000 載新 tray 進同位=卡撞 (INFERRED 幾何) → 需 AD-1 收尾 latch 才轉綠 |
| case 5000 移動中 | tray 已自由在出料位；uHome home Y 為空移 (A1 安全)；殘 tray 同上被棄置且軟體無記錄 | 紅今日 (同前段殘 tray 問題)；DRAIN 幫不上 (motor 段不可收斂)，靠 AD-1 於復歸補 Y 退開+FrontRise pump → 綠 |
| case 6000/6100 | 若落在 Rise On/dwell：uHome 不碰 Auto 缸 (uHome.cpp 僅列 motor)、InitialFlag 只重置 DischargeSubTask=1 (:84) 不下 Off() → FrontRise 實體卡 On 直到下次同站 case 6100 再 pump；riser 升起時下次 discharge 進位=碰撞風險 (INFERRED) | 紅今日。規劃 DRAIN：6000/6100 純 timer+缸 → 收斂到 return true(tray 已入爪、riser Off 已下令) → 綠；EMG 跳過 drain 路徑仍需 AD-2 復歸強制 Off + Off-reed 確認 |

### 缺口

- AD-1 (黃)：commit(case 1000, :687-703)～case 6100 完成之間，實體滿 tray 無任何 HOME 後存活的軟體記錄（RefreshAutoState 用前疊 sensor 覆寫 bFrontHasTray :334-340；FindDischargeAuto 不檢查前段殘 tray :409-421）→ 復歸無人補 GoUp、之後餵料/出料疊撞。最小修法：keep-material latch `bDischargeTailPending[Index]`，case-1000 commit 內 set、case-6100 return true 清；復歸時對 latch 站先跑尾段（MoveAutoY(feedY)+DoFrontRiseOnce），空升無害（cleanout :859-901 先例）。
- AD-2 (黃)：FrontRise 跨 HOME 卡 On——uHome 只 home MAutoY (uHome.cpp:621-624)、InitialFlag 只重置游標 (:84) 不碰線圈；且 DoFrontRiseOnce 回 true 時 Off 未等 reed (:1671-1673)。修法：復歸段（含 EMG 跳過 drain 的 fallback）對全站下 GetFrontRise(i)->Off() 並等 IsCylinderOffReady 後才放行 motor home / 下次進位（M2：riser 靜止時操作安全）。
- AD-3 (黃)：PARK 釋放順序與資料保全——plan 步驟(2)寫先 Lean 後 Push，Auto 自身梯形一律先 Push 後 Lean（discharge :707-717、cleanout :815-857）；Auto 的 PARK 應複用 GetPush/GetLean 與模組順序，且只鬆缸不得動 Tray 資料（fHasTray/Tray/WorkingKind/WorkingTrayID 在 bKeepMaterial 下天然保留 :104-117）——若 PARK 誤做成 ClearTray 式交接即重演 W3 資料遺失。
- AD-4 (黃)：InitialFlag 無條件 bResidueClear=true (:94) 在 HOME 洗掉 SortArm 殘料閘（FindDischargeAuto 的 discharge 閘 + AMR-leave 閘 :417,:1251）→ 殘料嫌疑站在 HOME 後直接放行出料。修法：bKeepMaterial 路徑保留 bResidueClear 原值（把該行移到 continue 之後的非保留段）。

### drain 邊界

HOME drain 階段對 DoDischargeTray 的可收斂邊界（以 DischargeTask 現值判斷，全部 VERIFIED 無 motor/真空）：Task==3000 或 4000 → 續 pump DoDischargeTray(1) 直到 Task==5000 即停（3000/4000 僅含 TMyCylinder::Pop + reed 等待，:707-717；case 5000 第一行就是 MoveAutoY :720，drain 絕不可 pump 進 5000）；Task==6000 或 6100 → 續 pump 到 return true 完整收斂（僅 DischargeDelay timer :732 + DoFrontRiseOnce 純缸升降 :1656-1676；收斂後建議補等 FrontRise Off-reed，因 return true 時 Off 只下令未確認 :1671-1673，避免 riser 未落底就開始 home Y）；Task==1000 或 5000 → 不可收斂（MoveAutoY 內含 Motor->MotorMove :302-313），交由 PARK（鬆 Push→Lean 後自由 home，A1）與 AD-1 收尾 latch 處理；Task==1/100 → 相位起點，無事可收。timeout fallback：任一缸 reed 逾時即放棄收斂、直接進 PARK+AD-2 強制 FrontRise Off 路徑。

## DoAllAutoCleanOut (aAuto1To6.cpp:753) + DoFeedTray rear-collect 子梯 (aAuto1To6.cpp:464) + DoAuto CleanOut 入口/短路 (aAuto1To6.cpp:1561) + csystem post-HOME Run_CleanOut resume (csystem.cpp:1373-1377)（黃）

> 覆核：OK。缺口代碼：AC-1, AC-2, AC-3, AC-4, AC-5, AC-6

1. CleanOut 的 HOME 恢復骨架今日已成立(VERIFIED):HSys.Sys.bCleanOut 在 main.cpp:1828 設、只在 csystem.cpp:1414 與建構子 database.cpp:644 清,InitialAllTask 不碰它;home 完成走 csystem.cpp:1373-1377 → Run_CleanOut + SoftStop=true(需操作員再按 Start,ProcessStartMode csystem.cpp:932-937)。
2. 梯形全體從 CleanOutTask=1 重跑(InitialFlag aAuto1To6.cpp:78),且 DoAuto:1580-1587 以 SortArm 活算 finish(aSortArm.cpp:1903-1928)守住 Loader→SortArm→Auto 的 cascade 順序,重跑本身冪等收斂。
3. 兩個今日紅色實體視窗:case 2000-4000 之間 tray 已鬆夾未升舉,及 case 4000-5000 之間 FrontRise 仍 On——uHome 不碰 Auto 汽缸就直接 home MAutoY;兩條產線梯(cleanout 5000→6000、DoDischargeTray 720→739)都嚴格「riser Off 才動 Y」,motor home 違反此設計(碰撞 INFERRED)。計畫的 DRAIN 階段(2000→5000 純汽缸+timer)正好收斂到 case 6000 邊界,兩窗皆閉。
4. DoFeedTray FeedTask 6000→7000 是實體先於資料的危險段:tray 已夾上車、fHasTray 未 set;HOME 落此→JAM%d11 modal(case 200)+Skip 產生幽靈盤,但 CleanOut 的 GoUp 無條件把車上物升進疊倉,材料面自癒,僅 Car 記帳/TrayID 遺失(黃)。
5. case 7000 為真正單掃描原子 commit(aAuto1To6.cpp:932-954);7000 後 HOME 會因 InitialFlag:93 無條件清 bCleanOutFinish 而整組空跑重排(浪費+可能誤觸 Full-gate modal,黃)。
6. Full-gate modal(aAuto1To6.cpp:873-886)在 DRAIN 語境違反 A3,需小規則:遇 Full ON 改為重新 Push/Lean 夾回後中止該站 drain。
7. 結論 yellow:骨架綠、兩紅窗由已核准的 DRAIN 直接閉合,剩四個可枚舉小補(AC-1~AC-5)+一個 EMG 斷氣時 FrontRise 行為的機構確認。

### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| 起點派發 | CleanOutTask 1→500、100→1000 | 無實體動作;100 清 bCleanOutCheck[](aAuto1To6.cpp:769-771,797-801) | — | 無 | 相位起點 | — |
| REAR-COLLECT | 500/600(內含 DoFeedTray FeedTask 1/100/200/1000/3000/4000/5000/5100/5200/6000/7000) | 逐站把「已送達未消化」rear tray 拉上工作台;FindFeedAuto 迴圈到無候選(aAuto1To6.cpp:780-795) | SnAutoX_OutputBottomHasTray(case 200 預檢 :508-516、case 3000 :540-550)+ Lean/Push reed(:553-577) | DoFeedTray case 7000 單掃描承接:CopyFrom+fHasTray=true+清 rear latch+Car 記帳(:607-652)VERIFIED 單掃描 | 危險段(FeedTask 6000→7000 實體佔有先於資料 commit) | 不可(FeedTask 1000/6000 為 MoveAutoY;僅 FeedTask 4000/5000/5100 可收斂→FeedTask 6000) |
| 全站 Y 至排出位 | 1000 | MoveAutoY(GetAutoDischargeY)×6,bCleanOutCheck 逐站 latch(:803-813) | MotorMove 到位(:302-313,可重發) | 無 | 冪等段 | 不可 |
| 鬆前擋 | 2000 | GetPush(i)->Pop() ×6(:815-835) | IsCylinderOffReady=OffSensor reed/sim/disabled(aAuto1To6.cpp:30-39) | 無 | 冪等段(Pop 對已到位缸快速再確認) | 可收斂→3000 |
| 鬆後推 | 3000 | GetLean(i)->Pop() ×6(:837-857) | 同上 | 無 | 冪等段 | 可收斂→4000 |
| 滿料閘+升舉 | 4000 | 實機 Full ON→do/while modal MES1120..1620+Car.Clear+InitAutoCarStack(:873-886);FrontRise->On() ×6(:887-889) | InputFullTray sensor + FrontRise OnSensor reed(mycylin.cpp:24-33) | Car[].Clear(操作員清倉後單掃描) | 冪等段(再 On 僅再確認;空升無害為 INFERRED) | 可收斂→5000(前提:drain 語境須繞過 modal,見 AC-5) |
| 停留+下降 | 5000 | CleanOutDelay(iAutoCleanOutRiseDwellMs)到期→FrontRise->Off() ×6(:903-915) | HTimer::Off()(HTimer.cpp:38-52) | 無 | 冪等段(僅可經 4000 進入;timer 被 Clear 後 Off() 永 false,HTimer.cpp:42-43) | 可收斂→6000 |
| 全站 Y 回饋入位 | 6000 | MoveAutoY(GetAutoFeedY)×6(:917-923) | MotorMove 到位 | 無 | 冪等段 | 不可 |
| 晚到回收+完工承接 | 7000 | FindFeedAuto()>=0 → 跳回 500(:932-936);否則 6 站清 bCarHasTray/bRearHasTray/bRearCanUse/pending/RearGrid/bFrontHasTray/bFullIC + bCleanOutFinish=true + AS_CLEANOUT_DONE + VMot ClearTray,return true(:937-954) | FindFeedAuto→RefreshAutoState 實機讀感測(:316-363) | 全部在同一 case 同一掃描內完成,VERIFIED 原子 | 原子commit | —(drain 止於 6000 邊界,到不了此) |
| 完工短路+殘盤看門狗 | DoAuto 前置(:1563-1571) | AllStationsDrainLatched→僅跑 ServiceCleanOutResidualWatchdog(EventLog MES1123..1623,:1030-1053) | 實機 front/full/rear 三感測 | bCleanOutResidualLogged latch | 相位起點 | — |
| DoAuto CleanOut 入口 | DoAuto 100→5000(:1580-1587,1624-1626) | Run_CleanOut 且 SortArm 活算 finish → DoAllAutoCleanOut(0) 初始化 | SortArmModule->IsCleanOutFinish() 活算(aSortArm.cpp:1903-1928) | CleanOutTask=1 | 相位起點 | — |
| HOME 後 resume 膠合 | csystem.cpp:1351-1385 | ProcessMotorHome 完成→InitialAllTask(true)→bCleanOut latch→Run_CleanOut+SoftStop=true(:1373-1377) | HSys.Sys.bCleanOut(main.cpp:1828 設;csystem.cpp:1414 清) | 單次 finalize | 原子commit | — |

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| 派發段(CleanOutTask 1/100;DoAuto 100 入口) | InitialFlag 全重置,重跑無實體副作用;bCleanOut 存活→resume 正確 | 綠 |
| REAR-COLLECT 前半(500/600,FeedTask≤3000:tray 仍在 rear 座) | rear sensor 仍 ON 或 pending latch 保留(InitialFlag bKeepMaterial 不清 rear latch,aAuto1To6.cpp:107-113)→重新 feed 乾淨 | 綠 |
| REAR-COLLECT 後半(FeedTask 4000..7000 未 commit:tray 已夾上車/搬運中) | uHome 不放 Auto Push/Lean(僅放 Loader),Y home 帶夾持 tray 走;fHasTray=false+pending 殘留→resume case 200 讀 rear sensor OFF→JAM%d11 modal(:508-533),Skip 產生幽靈盤;CleanOut GoUp 仍會把車上物實體疊入(自癒),但 Car 計數/TrayID 遺失 | 黃(AC-3) |
| GoUp Y 段(1000/6000,tray 夾持或車空) | 馬達停在半路→home→梯自 1 重跑重新定位;夾持 tray 隨車 home 無妨 | 綠 |
| 鬆夾窗(2000 起至 4000 升起確認前:tray 已無夾持躺在排出位車上) | motor home 直接動 MAutoY,未夾(可能含 IC 的)tray 無約束→位移/散料風險(INFERRED;A1 僅確認 Loader 幾何) | 紅→計畫 DRAIN(2000→5000 純汽缸)收斂後綠(AC-2) |
| 升舉窗(4000 升起中至 5000 Off 前:FrontRise On) | uHome 不碰 Auto 缸→riser 保持 On 之下 home MAutoY;兩條梯皆「riser Off 才動 Y」(cleanout :908-913 先 Off 再 6000;DoDischargeTray :720 Y 先回位 :739 才 pump riser)→motor home 違反設計包絡,碰撞風險 INFERRED | 紅→DRAIN 收斂後綠;EMG 跳過 drain 時 FrontRise 斷氣行為需機構確認(AC-1) |
| 升畢未承接(5000 Off 後至 7000 前,含 6000) | tray 已入疊倉但 fHasTray 仍 true→InitialFlag:91 誤 re-derive bCarHasTray=true→resume 空跑一輪 GoUp 後 7000 ClearTray 自清;結果正確 | 綠(一輪空泵浪費) |
| 已 latch 待 cascade(7000 後,等 TrayArm/Empty/Color 完工) | InitialFlag:93 無條件清 bCleanOutFinish(bKeepMaterial 不豁免)→resume 全 6 站空跑重排(12 次 Y 移動+升降),疊倉滿高時誤觸 Full-gate modal | 黃(AC-4) |
| resume 膠合(csystem) | bCleanOut 存活+SoftStop=true→需按 Start;DoAuto:1580 活算 gate 保住 Loader→SortArm→Auto 順序(Loader InitialFlag() 無參數自清重跑);晚到 tray 由 :453-457/:1202-1206 掉 latch 自癒 | 綠 |

### 缺口

- AC-1:HOME 落在 CleanOutTask 4000-5000 時 FrontRise 仍 On 即 home MAutoY,違反「riser Off 才動 Y」設計包絡(VERIFIED 排序證據 :908-913、DoDischargeTray :720/:739;碰撞本身 INFERRED)。最小修法:核准的 DRAIN 階段納入 4000/5000 泵至 case 6000 邊界;EMG 跳過路徑需向機構方確認 EMG 斷氣時 FrontRise 是否落下。黃
- AC-2:HOME 落在 2000-4000 之間,tray 已鬆 Push/Lean、未升舉,motor home 動 Y 時無約束(A1 僅涵蓋 Loader 幾何,Auto 車載幾何 INFERRED)。最小修法:同 AC-1 由 DRAIN 收斂;drain 逾時 fallback 應先重 Push/Lean 夾回再 home(純汽缸,合法)。黃
- AC-3:DoFeedTray FeedTask 6000→7000 危險段——實體佔有先於單掃描資料 commit(:607-652);HOME 落此→resume JAM%d11+Skip→幽靈盤(CleanOut 材料面由 GoUp 自癒,Car 記帳/TrayID 遺失;Normal 模式更嚴重但非本項範圍)。最小修法:InitialFlag(bKeepMaterial) 增補:pending latch ON+rear sensor OFF+車位≈feed 段時代行 case-7000 資料承接(或至少 EventLog 標記)。黃
- AC-4:InitialFlag:93 無條件清 bCleanOutFinish,已 latch 完工的 HOME 觸發全站空跑重排+可能 Full-gate modal 噪音。最小修法:bKeepMaterial 時改為感測再導出(front/full/rear 全 OFF 且 fHasTray=false 的站保留 latch)。黃
- AC-5:Full-gate 阻塞 modal(:873-886)在 DRAIN 語境違反 A3(HOME 中禁止人手觸料)。最小修法:drain 旗標下遇 Full ON 不進 modal,改重夾(Push/Lean On)後中止該站 drain,modal 留給 resume 後的正常重跑。黃
- AC-6:case 5000 冷進入陷阱——CleanOutDelay 被 Clear 後 HTimer::Off() 永 false(HTimer.cpp:42-43);今日不可達(梯必自 1 重跑),但 DRAIN/resume 實作絕不可直接跳入 5000 而未經 4000 arm。實作守則,免修。綠

### drain 邊界

HOME DRAIN 階段對 Auto CleanOut 的精確可收斂邊界(全部引 D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\aAuto1To6.cpp):(1) 若 CleanOutTask ∈ {2000,3000,4000,5000},持續泵 DoAllAutoCleanOut(1) 直到 CleanOutTask==6000 即停——這四段只有 GetPush/GetLean->Pop()、FrontRise->On()/Off() 與 CleanOutDelay 停留 timer,全部 reed 確認、無任何 MoveAutoY、無真空;case 6000(:917-923)是第一個馬達相位,為硬邊界。前提兩項:case 4000 的 Full-gate modal(:873-886)必須在 drain 語境被繞過(遇 Full ON 改重夾後中止,AC-5),且 drain 必須在 InitialFlag 之前執行(否則 CleanOutDelay 被 Clear,HTimer::Off() 永 false 卡死 5000,HTimer.cpp:42-43)。(2) 若 CleanOutTask ∈ {500,600}(rear-collect),僅當內層 FeedTask ∈ {4000,5000,5100} 時可泵 DoFeedTray(1) 至 FeedTask==6000 邊界(夾持 reed 確認完成、tray 已固定於車上;FeedTask 6000 是 MoveAutoY);FeedTask 200/1000/3000/5200 含馬達或 modal,一律立即停。(3) 若 CleanOutTask ∈ {1,100,7000} 或已全站 latch:純派發/純資料,無可收斂之實體動作,直接進 PARK/home。(4) 若 CleanOutTask ∈ {1000,6000}:馬達相位不可 drain,馬達就地停;MotorMove 可重發(:302-313),resume 自 case 1 重跑會重新定位,唯 2000-5000 未完成時須先套用 (1) 的收斂再 home,否則落入 AC-1/AC-2 紅窗。

## CrossModule-Setter-Checklist (TrayArm/SortArm/AgvCoord → Loader/Empty/Color/Auto)（黃）

> 覆核：ISSUES（已依覆核修正）。缺口代碼：XS-1, XS-2, XS-3, XS-4, XS-5, XS-6

跨模組 setter 共 21 條(含補列 AgvCoord→Auto ClearAmrCar、SortArm→Auto TransferPlaceDataToAuto),核心生產握手已因 d63d33a(case-100 heal)+ rearready-p0(Loader bKeepRear)+ Auto keep-material 呈對稱或已癒合(綠)。剩餘不對稱集中在四類:(1) 「已放未簽」視窗(DoLowerClampRaise 夾爪已 Pop、case-4000 notify 未跑)— AUTO 目的地 resume 無守門即【開爪】重降繞已放之盤(uHome 從不閉爪,double-stack 不成立)、冗餘重放後 case-4000 重簽自癒(XS-1 黃),Empty/Color 目的地 heal 會把剛放的盤 GoUp 吃掉再簽幽靈 finish(XS-2 黃);(2) residue 檢查對 fail-open:HOME 把 Auto bResidueClear 強制 true、SortArm 忘記待驗名單(XS-3 黃);(3) AgvCoord 在 HOME 不 Reset(Reset 全 repo 僅 ctor 呼叫),Handshake[] 存活但模組 bAmrLocked 被全清且再鎖只在邊緣觸發、READY 中 Finish/ClearAmrCar 延後補簽(XS-4 紅,AMR 已靠站時最危險);(4) Loader iFeedSerial/iSecsCarTrayCount 歸零但實體 AMR 車疊數不變 → GetFedTrayKind 錯位(XS-5 黃)。所有 case-4000 commit 均驗證為單一 scan 原子(pick :668-710;place :790-812/:968-977/:1041-1050)。整體 yellow:修法可枚舉(resume-adopt 守門、residue 保留、InitialAllTask 呼叫 AgvCoord.Reset、feed-serial 併入 bKeepRear 模式),僅 XS-4 靠站中復機需 owner/AGV 協定確認。

### 相位表

| setter | 發訊方(call sites) | 收訊方狀態 | HOME後發訊方記得? | HOME後收訊方記得? | 誰負責重同步(post-d63d33a) | 判定 |
|---|---|---|---|---|---|---|
| TEmptyModule::RequestReturnTray (aEmpty.cpp:1167) | aTrayArm.cpp:769(清機分流)、:865(DecidePlaceDestAfterPick)、:1091(case-100 heal) | bReturnTray=true、bTrayXToEmptyFinish=false → DoEmpty case100(:238)轉收盤、case3000(:359)等 deposit | 是(keep-material 保 Job/PlaceDest,aTrayArm.cpp:88-92) | 否(InitialFlag 清 :38-40) | DoTrayArm case-100 heal 重發(aTrayArm.cpp:1090-1091)VERIFIED | 綠(已癒合) |
| TEmptyModule::CancelReturnTray (aEmpty.cpp:1178) | aTrayArm.cpp:895(TryDivertCarriedTrayToAuto,同 scan 改 PlaceDest=AUTO) | bReturnTray=false | 是(PlaceDest=AUTO 保留) | 否(反正被清) | 無需——同 scan 原子 + 對稱失憶後語意一致 VERIFIED | 綠 |
| TEmptyModule::SetRearHasTray(false) (aEmpty.cpp:1155) | aTrayArm.cpp:704(DoPick case4000;:703 先 CopyFrom GetSourceTray) | bRearHasTray=false、MMEmptyY->ClearTray、Status 修正 | 是(keep-material 保盤+grid) | REALLY 由 sensor 重推導(aEmpty.cpp:182);sim 為 latch | 單 scan 原子 commit;pick 中段交由殘留領養(aTrayArm.cpp:59-68)VERIFIED | 綠 |
| TEmptyModule::NotifyTrayXToEmptyFinish (aEmpty.cpp:1183) | aTrayArm.cpp:972(DoPlaceToEmpty case4000) | bTrayXToEmptyFinish=true、bRearHasTray=true、ES_REAR_READY | 「已放未簽」視窗:發訊方以為仍抱盤(fHasTray 至 :975 才清) | 否(等不到 finish) | heal 重發 RequestReturnTray → 誤把已放之盤當 rear 佔用 GoUp 收走,再簽幽靈 finish | 黃(XS-2) |
| TColorModule::RequestSupplyTray (aColor.cpp:1246) | aTrayArm.cpp:526(DecideJob AMR identity,每 scan 重問) | bSupplyRequested=true → DoColor :378 → DoFeedTray(清於 :1027) | 發訊方無狀態(輪詢式) | 否(InitialFlag :42) | 發訊方每 scan 自然重發 VERIFIED | 綠 |
| TColorModule::NotifyTrayPicked (aColor.cpp:1252) | aTrayArm.cpp:696(DoPick case4000;:694-695 先取 TrayID+grid) | bTrayReady/bRearHasTray=false、ClearTray、Status 修正 | 是(keep-material) | InitialFlag 清 + sensor 重推導 | 單 scan 原子 commit VERIFIED | 綠 |
| TColorModule::RequestReturnTray (aColor.cpp:1268) | aTrayArm.cpp:763(清機分流)、:833(identity 回 Color)、:1093(case-100 heal) | bReturnTray=true、bTrayXToEmptyFinish=false | 是 | 否(InitialFlag :50-51) | case-100 heal 已涵蓋 TAPLACE_COLOR VERIFIED | 綠(已癒合) |
| TColorModule::NotifyTrayXToEmptyFinish (aColor.cpp:1328) | aTrayArm.cpp:1045(DoPlaceToColor case4000) | bTrayXToEmptyFinish=true、bRearHasTray=true → DoColor :413 收尾 | 同 Empty 之「已放未簽」視窗 | 否 | 同上幽靈 finish 路徑(Color 無 CancelReturnTray,但 identity 不分流故無額外洞) | 黃(XS-2) |
| TLoaderModule::NotifyTrayArmPickRearTray (aLoader.cpp:858) | aTrayArm.cpp:685(DoPick case4000;:676-684 先 CopyFrom+重讀 Kind/ID) | 清 rear hold 五項(bRearHasTray/bRearReadyForPick/RearKind/RearTrayID/RearSourceTray :863-867) | 是(keep-material) | 是——InitialFlag bKeepRear 保 settled rear(aLoader.cpp:79-87) | 無需(rearready-p0 已修)VERIFIED | 綠 |
| TAutoModule::StageRearGrid (aAuto1To6.cpp:1214) | aTrayArm.cpp:798(DoPlace case4000) | RearGrid CopyFrom(不設 fHasTray,:1213) | 視窗同上 | 是——keep-material 跳過 :118 清除(:107 continue) | 與 Notify 同 scan 原子;重跑僅重複 CopyFrom(冪等)VERIFIED | 綠(資料面) |
| TAutoModule::NotifyTrayArmDelivered (aAuto1To6.cpp:1191) | aTrayArm.cpp:804(AMR 路徑) | RearKind/RearTrayID/bRearHasTray/bRearCanUse/bRearDeliveredPending/AS_REAR_STAGED | 「已放未簽」視窗:發訊方 resume 重跑整條 place | 是(keep-material 全保;僅 Status :92 重推導,消費者用 bRearDeliveredPending :402/:1299,良性) | 無守門:resume【開爪】重降繞已放之盤(夾爪已 Pop、uHome 從不閉爪),冗餘重放後 case-4000 重簽自癒 | 黃(XS-1) |
| TAutoModule::SetRearHasTrayFromTrayArm(true) (aAuto1To6.cpp:440) | aTrayArm.cpp:806(Normal 路徑) | 同上 + CleanOut 自癒(:453-457) | 同上視窗 | 同上 | 同上 | 黃(XS-1) |
| TAutoModule::SetPlaceResidueClear (aAuto1To6.cpp:1247) | aSortArm.cpp:1832(false,place case50)/:1210、:1276(true,CheckPlaceResidue 完成) | State[].bResidueClear(閘 discharge/AMR leave,:417/:1301) | 否(SortArm InitialFlag 清 iResidueAutoIndex/bResidueArmed/bNeedResidueCheck :155-161) | 否——且方向錯:HOME 強制 true=開閘(:94) | 無人;雙側 wipe 但 fail-open | 黃(XS-3) |
| TLoaderModule::ChangeActiveTrayData (aLoader.cpp:918) | aSortArm.cpp:1683-1684(K_TRAY_END 全盤改寫)+ Loader 自呼 :1707;近親 SetTraySingleData(aSortArm.cpp:1172 SKIP 寫格) | MMLoaderY_x Tray.Data 格值 | 一次性即時寫,無 pending 態 | 今日 uHome case200 強制移除+ClearTray(uHome.cpp:655-668)→ 雙方歸零一致;PLANNED PARK 保 Tray.Data 亦一致 | 無需 VERIFIED | 綠 |
| TSortArmModule::TransferPlaceDataToAuto (aSortArm.cpp:1365) | aSortArm.cpp:1854(place case50,與 DestroySelectedSlots 完成同一 scan) | Auto working-tray 格值 SetTraySingleData(HAS_OK_IC)(:1375)+ 計數/Production log + ClearSlot(:1406) | 一次性即時寫,無 pending 態(與實體 destroy 確認同 scan) | 是——Auto working tray 為馬達 Tray.Data,uHome 僅強制清 Loader 盤(uHome.cpp:655-668),keep-material 保留 | 無需;殘 IC 驗證由同 scan 的 SetPlaceResidueClear(false)(:1832)承接 = XS-3 範疇 VERIFIED | 綠 |
| TLoaderModule::AcquireSortOwner/ReleaseSortOwner (aLoader.cpp:786/:804) | aSortArm.cpp:1554/:1703、:1713 | iYOwner[]=SORTARM/NONE、Side.Status=LS_SORTING/LS_ToRear | 否(SortArm InitialFlag iActiveLoaderNo=0 :145) | 否(iYOwner=NONE :92-93、ResetSide) | 對稱失憶,SortArm 重新 Acquire VERIFIED | 綠 |
| AgvCoord→SetAmrLock/InfeedSetLock (uAgvStation.cpp:239,:318,:386,:405;:68-70 分派) | AgvCoord PollAndCall/ServiceHandshake/BeginPrep/AbortAutoHandshake | 各模組 bAmrLocked(Loader:507/Empty:79/Color:84/Auto:1254) | 是——Reset() 全 repo 僅 ctor 一處呼叫(:113);link-down 分支(:188-209)並非呼叫 Reset,而是 inline 部分釋放(只清 active 站的 lock+Handshake,不清 CarrierID/TrayCount/PrepDone/BinSetting)→ HOME 不清 Handshake[] | 否(四個 InitialFlag 全清 bAmrLocked) | 無人;再鎖僅邊緣觸發(:237-243 進 CALLED、:386 BeginPrep),PREP/READY 看門狗 :314-322 只是延遲釋放 | 紅(XS-4) |
| AgvCoord→TAutoModule::ClearAmrCar (uAgvStation.cpp:308 → aAuto1To6.cpp:1324) | AgvCoord ServiceHandshake READY→Finish 邊(IsAmrTaken 成立,CEID274) | Car[].Clear+InitAutoCarStack+bAmrLocked=false+bResidueClear=true(:1328-1331)= 唯一 AMR 清車點 | 是——Handshake=READY 存活(HOME 不清) | 部分——bAmrLocked 已被 InitialFlag 先清(:1254),Finish 未跑則車疊數/CarID 不清 | 無人;READY 中 HOME 後站點已解鎖恢復堆疊,AMR 實際取車才補簽 Finish+清車 | 紅(XS-4 同視窗) |
| Host→TLoaderModule::SetExpectedCarTrayCount (uAgvStation.cpp:81;aLoader.cpp:567)+ iFeedSerial 進度 | SECS S2F49 LoaderTrayCount / BeginPrep | iSecsCarTrayCount、iFeedSerial(Kind 蓋章依 GetFedTrayKind(iFeedSerial,iCarTrayTotal) aLoader.cpp:1553) | 實體車/host 記得(車上剩餘疊數不變) | 否(InitialFlag :65 歸零、:95 iFeedSerial=0) | 無人 | 黃(XS-5) |
| AmrInject.RequestAuto/RequestInput/SetTestMode (maintenance.cpp:907,:921,:923) | 維修 UI(操作員) | uAmrInject one-shot latches(AgvCoord 各閘讀取) | 操作員 | 否——InitialAllTask 開頭 AmrInject.Reset()(database.cpp:41)+ production START 之 MachineStart(csystem.cpp:1137,非開機) | 設計性單向清除(測試設施不得洩入正式 run)VERIFIED | 綠(豁免) |
| TColorModule::NotifyICPlaced (aColor.cpp:1334) | (無任何呼叫端——全 repo grep 僅定義) | iICCount 累加(供給門檻) | — | — | 發訊方缺席,死線 | 黃(XS-6 觀察) |

### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| W1 pick 已夾未簽(PickTask 2000-3000:夾爪已 Push、case-4000 grid copy+notify 未跑) | fHasTray 尚為 false(:708 才設)→ InitialFlag 殘留領養(aTrayArm.cpp:59-68,真機雙 On sensor)→ 強制走非保留分支、Job 清空、MES 殘留通知、操作員移除;來源模組:Loader bKeepRear 因 rear sensor 已空(盤被夾走)不保留→歸零一致;Empty/Color sensor 重推導一致 | 停機領養而非續跑,但已通知+安全;規約 step4 後可改為續跑候選。黃(設計內,VERIFIED) |
| W2 place-to-Auto 已放未簽(DoPlace PlaceTask 2000-3000:夾爪已 Pop、盤已在 Auto rear、case-4000 :790-812 未跑) | fHasTray 仍 true → uHome case 2 因 HasTray()==true 不動夾爪(uHome.cpp:497-501;uHome 全檔對 TrayArm clamp 無任何 Push/On,只有未持盤時的 Off),而夾爪本已 Pop 物理開啟(On sensor 讀 Off)→ resume keep 分支 TAS_CARRYING、Job/iAutoTarget 保留 → case-100 直接 DoPlace(0) 重跑:Z-safe 移 X → case1000【空爪開夾】Z-down 繞已放之盤下降(幾何同正常 pick 下降)→ case2000 Pop 冪等快進 → case4000 重簽 notify+StageRearGrid(盤確在,重簽即正確、資料冪等)。「閉爪+rear 佔用」不可能並存:Pop 未完成則盤仍在爪中隨 Z 上升帶走,rear 不佔用 | 無 rear-occupied 守門 → 冗餘開爪重降+重放動作(非 double-stack),資料面重簽自癒。黃(XS-1) |
| W3 place-to-Empty/Color 已放未簽(DoPlaceToEmpty/Color 同視窗,case-4000 :968-977/:1041-1050 未跑) | heal 重發 RequestReturnTray → Empty case100 轉收盤、GoUp 把「剛放好的盤」當 rear 佔用收回前車(這其實正是 return 想要的結果)→ rear 清空 → TrayArm 空爪走完 deposit → :972/:1045 簽幽靈 finish(bRearHasTray=true 但 rear 無盤) | REALLY 模式 sensor 重推導(aEmpty.cpp:178-182)下一 refresh 自我修正;sim/DUMMY latch 幽靈存活可誘發抓空氣。黃(XS-2) |
| W4 RequestReturnTray 已發、deposit 未開始(PlaceTask 1..500) | Empty/Color 忘記 bReturnTray,TrayArm case-500 等 rear 清 → d63d33a heal 於 case-100 重發,且 RequestReturnTray 冪等並驅動 GoUp 清 rear | 已癒合。綠(VERIFIED) |
| W5 residue 檢查中(place case50 已 SetPlaceResidueClear(false),CheckPlaceResidue 未完) | SortArm InitialFlag 忘記 bNeedResidueCheck/iResidueAutoIndex(:155-161);Auto InitialFlag 強制 bResidueClear=true(:94)→ 閘直接開,黏在吸嘴上的殘 IC(bHasIC 已被 ClearSlot 清,不受 keep-material 保護)未驗證即放行 discharge/AMR leave | fail-open。黃(XS-3) |
| W6 K_TRAY_END/SKIP 已改寫 Loader 格值後 HOME | uHome case200 強制操作員移除 Loader 盤+ClearTray → 寫入與實體同歸零;PLANNED PARK/RE-ACQUIRE 保 MMLoaderY Tray.Data 後,已寫格值隨盤保留,亦一致 | 無不對稱。綠(VERIFIED) |
| W7 AMR handshake 中 HOME(Handshake=CALLED/PREP/READY) | 模組 bAmrLocked 全清,AgvCoord 不 Reset:CALLED→鎖丟到 START_AGV/BeginPrep 才回;PREP/READY→AMR 可能已靠站,站點卻解鎖恢復 destack/feed,看門狗 :314-322 要等 iAmrHandshakeWaitSec 才強制歸位;READY 中 HOME 後 Finish 邊(uAgvStation.cpp:308 ClearAmrCar→aAuto1To6.cpp:1324)延後至 AMR 實際取車才補清車疊/補回 bResidueClear | 發訊方記得、收訊方忘記的教科書案例。紅(XS-4) |
| W8 AMR 供料車中途 HOME | iFeedSerial=0/iSecsCarTrayCount=0,實體車剩餘疊數不變 → 下一盤 GetFedTrayKind 視為新車第 1 盤 → RearKind 蓋錯 → TrayArm 依 Kind 誤路由(identity↔Empty/Color) | 黃(XS-5;Kind 對映細節 INFERRED,serial 依賴 VERIFIED aLoader.cpp:1553) |
| W9 SortOwner 持有中 HOME(SortArm 佔 Loader-Y) | 雙側對稱歸零(iYOwner=NONE、Side ResetSide、SortArm PickTask=1),重新 Acquire;懸吸 IC 由 SortArm keep-material 保住並回 place | 綠(VERIFIED) |

### 缺口

- XS-1(黃): 「已放未簽」@AUTO——DoPlace 夾爪已 Pop 至 case-4000(aTrayArm.cpp:790-812)之間 HOME,resume 無 rear-occupied 守門(DoPlace case 1/10 僅 CleanOut divert 檢查 :756-774)即整條重跑:【開爪】Z-down 繞已放之盤(幾何同正常 pick 下降)+ Pop 冪等快進 + case-4000 重簽自癒——非「閉爪 double-stack」(uHome 從不閉爪,uHome.cpp:491-501 只在未持盤時 Off;Pop 未完成則盤隨 Z 帶走、rear 不佔用),殘餘風險是無守門的冗餘動作與重放。最小修法方向不變:case-100 resume(或 DoPlace case1/10)加「目的地已佔用+雙夾 On sensor 皆 Off ⇒ 視同已交付,直接執行 case-4000 資料 commit(StageRearGrid+Notify+清 arm latch),不重跑動作」。
- XS-2(黃): 同視窗@EMPTY/COLOR——heal 重發 RequestReturnTray 令 GoUp 收走已放之盤後,空爪 deposit 簽幽靈 NotifyTrayXToEmptyFinish(aTrayArm.cpp:972/:1045);REALLY sensor 自癒(aEmpty.cpp:182)、sim/DUMMY 幽靈存活。修法同 XS-1 的 adopt-as-delivered 守門(視同已交付即免重發 return)。
- XS-3(黃): residue 對 fail-open——HOME 令 Auto bResidueClear=true(aAuto1To6.cpp:94)且 SortArm 忘記待驗名單(aSortArm.cpp:155-161)。最小修法:SortArm InitialFlag 在 bKeepMaterial 且(bResidueArmed 或任一 bNeedResidueCheck)時保留 iResidueAutoIndex 並重排 CheckPlaceResidue;或改 Auto 端 HOME 預設對「上次有 pending 檢查」之站保持 false+告警。
- XS-4(紅): AgvCoord 在 HOME 不 Reset——Reset() 全 repo 僅 ctor 一處呼叫(uAgvStation.cpp:113),link-down 分支(:188-209)只是 inline 部分釋放(僅清 active 站 lock+Handshake);Handshake[] 存活、四模組 bAmrLocked 被 InitialFlag 全清、再鎖僅邊緣觸發(:237-243、:386),READY 中 Finish 邊 ClearAmrCar(:308→aAuto1To6.cpp:1324)延後補清。最小修法:InitialAllTask 內呼叫 AgvCoord.Reset()(:116-145,其註解自述即鏡射 link-down 釋放的全站版;bFull/shortage 仍真會自然重 CALL);AMR 已靠站中復機是否允許需 owner/AGV 協定決定。
- XS-5(黃): Loader 車進度失憶——InitialFlag 歸零 iFeedSerial(aLoader.cpp:95)/iSecsCarTrayCount(:65)而實體車疊數不變,GetFedTrayKind(:1553)錯位蓋 Kind。最小修法:比照 bKeepRear 模式,AMR 模式下車未換(無 CEID274 Finish)則保留兩計數。
- XS-6(低/觀察): TColorModule::NotifyICPlaced(aColor.cpp:1334)全 repo 無呼叫端——iICCount/供給門檻死線;非 HOME 缺口,列入待接線清單。

### drain 邊界

對 HOME drain 階段可收斂的跨模組交接邊界:(1) TrayArm DoLowerClampRaise case 1000→3000 全段為純汽缸+計時,無任何 Move* 與真空——TrayArm Z 不是馬達而是雙線圈汽缸(DoZDown aTrayArm.cpp:385-389、DoZUp :369-383,uHome 亦以同一 Z 汽缸操作:uHome.cpp:474-475 C_TrayArmZ_Up.On()),夾爪 Push/Pop 在 :429-441、settle 計時 :443-446。依五步規約「純汽缸相位跑到相位邊界」,建議邊界取 helper 出口(case 3000 DoZUp reed 確認 :448-451):A2 鐵律要求 Z 先升,且 uHome case 2 無論如何會強制 Z-up,停在 3000 是讓 ladder 自己 reed 確認後升,優於停在 2100 讓 uHome 盲升;若取保守 2000→2100 邊界亦安全(該兩 case 確無 Move*/真空),但其理由是「少跑一段少一分狀態」而非 case 3000 屬馬達——本 ladder 的馬達段其實在 case 1/10 的 MoveTrayArmX(:404-407,MotorMove :366)。pick 方向(bGrab=true)drain 到 3000 會完成夾爪抓盤:此為汽缸抓取、非真空 handoff,且結果「雙夾 On」正是 uHome 保爪守門(uHome.cpp:493-495)與 InitialFlag 殘留領養(aTrayArm.cpp:59-68)的高確定訊號,安全。(2) 四個 case-4000 commit(DoPick :668-710、DoPlace :790-812、DoPlaceToEmpty :968-977、DoPlaceToColor :1041-1050)為單一 scan 純資料交接(grid copy+notify+latch),無馬達與真空輸出——嚴格說不是汽缸相位,但建議 drain 階段擴充為「若實體動作已完成(pick:雙夾 On;place:雙夾 Off 且已在目的 X)則先代跑該 case-4000 commit 再進馬達歸原」,即可一次消滅 XS-1/XS-2 的已放未簽視窗。其餘一律不可 drain:Empty/Color 的 DoGoUpTray/DoGoDownTray 與 DoFeedTray 內含 MotorY Move(交由 heal/重發處理)、SortArm CheckPlaceResidue 是真空 handoff(XS-3 走保留待驗名單而非 drain)、AGV handshake 是 SECS 協定狀態(走 AgvCoord.Reset,非 drain)。

## 補漏表（完整性批判新增）

### TAgvCoordinator::PollAndCall / TAgvCoordinator::ServiceHandshake / TAgvCoordinator::BeginPrep(+ 模組側 TAutoModule::SetAmrLock / InfeedSetLock)（紅）

> 覆核：ISSUES。缺口代碼：FX-1, FX-2, FX-3, FX-4, FX-5, FX-6, FX-7, XS-4

AGV 交握是「協調器狀態(AgvCoord.Handshake[],僅 ctor Reset,uAgvStation.cpp:113)+ 模組鎖(bAmrLocked)」的雙本簿記,HOME 只洗一半:InitialAllTask(database.cpp:48-66)→ aLoader.cpp:64 / aEmpty.cpp:27 / aColor.cpp:30 / aAuto1To6.cpp:97 全清鎖,Handshake/ShortageDebounce/TrayCount 全存活(VERIFIED)。更關鍵:ServiceHandshake 掛在 THGem 1s tick(uHGemEquipment.cpp:174-175,在 bWantComm 門之前)且無 RunMode 門(uAgvStation.cpp:282-287)→ HOME 進行中仍會發 CEID273/274、跑 ClearAmrCar/InfeedRefill、watchdog 照齡;PollAndCall 則被 Run_Normal 門(:212)凍結。結果:HOME 落在 PREP/READY 時,完成後鎖消失但 Handshake 殘留 → infeed 前段 destack、Auto 討盤/出料在 AMR 靠站伸入期間恢復(M2 同類違規),且 watchdog 靜默解約無任何 SECS 事件通知 host。FX-1(HOME 後依 Handshake 重掛鎖)+FX-2(Run_Home 凍結 ServiceHandshake/計齡)是可枚舉小修;但 READY(AMR 實體對接中)時全機 home 動 LoaderY/MAutoY 與 AMR 空間是否重疊、AGV 是否應收「設備 HOME 中」訊號,無法由軟體單方收斂 → 紅,需 owner/AGV 協定(即 XS-4 指的那塊)。Finish commit 本身單掃描原子(VCL timer 不可被 HOME 搶斷),資料面(車籍 TrayCount[0] 經 InfeedRefill 於 Finish 重掛,uAgvStation.cpp:81)可自癒。

#### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| 待命輪詢 | AGV_IDLE(PollAndCall,uAgvStation.cpp:221-272;Run_Normal 門 :212) | 逐站讀滿車/缺料 predicate + 更新 TrayCount/CarrierID 快照(:229-231);不動任何缸/軸 | IsOutputCarFullForAmr(aAuto1To6.cpp:1237-1246,real=InputFullTray sensor)、InfeedShortage(:45-51,real=SnX_Input(e)nd OFF) | 無 | 相位起點 | — |
| 呼車(Auto P4-P9) | IDLE→CALLED(:237-244) | SetAmrLock(a,true)(:239)+CEID272(:241)+分站 Full CEID(:242)+Handshake=CALLED(:243),同一 scan | full edge + Handshake==IDLE + !IsOperatorHolding(:237) | 鎖+雙事件+狀態單掃描一次寫入(VERIFIED) | 原子commit | — |
| 呼車(infeed P1-P3) | IDLE→CALLED(:260-266) | CEID272+CALLED+ShortageLatch;**不上鎖**(鎖只在 BeginPrep :388) | InfeedShortage(p) | 單掃描 | 原子commit | — |
| CALLED 等派車 | CALLED(:245-250 Auto 滿消自解;:267-271 infeed 補滿自解) | 等 host START_AGV;條件消失時單掃描自解(Auto 連鎖一起解 :248) | 同上 predicate 反相 | 解除單掃描 | 冪等段 | 不可(等外部 host 決策,無時間邊界) |
| PREP 進入 | BeginPrep(:378-390;呼點=S2F41 START_AGV,uHGemHT160.cpp:910) | Handshake=PREP+PrepDone=0(:383-384)+上鎖(Auto :386 與呼車鎖冪等;infeed :388 首次凍結前段 destack) | host RCMD cpName(LookupByName :380) | 單掃描;**無 RunMode/HOME 門**(VERIFIED) | 原子commit | — |
| PREP 等就緒 | AGV_PREP(ServiceHandshake :293-301 Auto、:335-343 infeed;**無 RunMode 門** :282-287) | 輪詢 IsDrainedForAmr/InfeedReady;真→CEID273+READY | aAuto1To6.cpp:1281-1303(sim 恆真 :1290-1291;real 加 bResidueClear+FrontRise 回家);aLoader.cpp:517-527 / aEmpty.cpp:90-100 / aColor.cpp:94-104(前段缸 out-bit 全回家) | CEID273+READY 單掃描 | 冪等段 | 不可(協調器只等待;可收斂的是模組側 in-flight 前段缸相,見 drainNote) |
| READY 等實體交接 | AGV_READY(:302-311 Auto、:344-355 infeed) | 輪詢 IsAmrTaken/InfeedFinished;真→Finish:CEID274+ClearAmrCar(aAuto1To6.cpp:1324-1332)/解鎖(:350)+InfeedRefill(:351)+IDLE | Auto real=SnAutoX_InputEnd OFF(:1317-1318,sim 恆真 :1313);infeed real=SnX_InputEnd ON(aLoader.cpp:540-545) | Finish 全部在同一 scan(事件+車籍清空+解鎖+IDLE)=原子(VERIFIED) | 危險段(鎖被洗後重入=產線侵入 AMR 實體工作區) | 不可(實體車/料匣交接) |
| watchdog 強制解約 | PREP/READY 齡 > iAmrHandshakeWaitSec(預設240s)(:314-323 Auto、:358-367 infeed) | 靜默解鎖+IDLE+清齡;**不發任何 SECS 事件** | ShortageDebounce[i] 每 1s tick 遞增(狀態未變才計 :314/:358) | 單掃描 | 冪等段 | — |
| 斷線總解約 | PollAndCall !IsSelected(:188-210;在 Run_Normal 門之前→任何 RunMode 都跑) | 全 9 站解鎖+IDLE+清 ShortageLatch | HSMS Selected | 單掃描迴圈 | 原子commit | — |
| 站側棄約 | AbortAutoHandshake(:399-409;呼點 ServiceCarFull 滿車等逾時 aAuto1To6.cpp:1506-1511) | 解鎖+IDLE+清齡;呼點先立 bOperatorHolding(:1510)擋 re-CALL | AmrFullWaitTimer(iAmrFullWaitSec) | 單掃描 | 原子commit | — |

#### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| IDLE 落 HOME | PollAndCall 被 Run_Normal 門(:212)凍結,HOME 後重新輪詢;無殘留 | 綠 |
| CALLED(infeed)落 HOME | 無鎖可洗;real sensor 缺料持續 → CALLED 語意跨 HOME 一致,host 後續 START_AGV 照常 BeginPrep 上鎖(VERIFIED)。sim:InitialFlag 的 RefillSimInfeed(aEmpty.cpp:30 等)填滿虛擬庫存 → HOME 後 :267 自解 IDLE(僅 sim,cosmetic) | 綠(real)/黃(sim 假解約) |
| CALLED(Auto)落 HOME | 鎖被 aAuto1To6.cpp:97 洗掉但 Handshake 存活=CALLED → HOME 後 (a) re-CALL 被 Handshake!=IDLE 擋(:237);(b) ServiceCarFull 在 AMR+selected 走 else-continue(:1513-1519)**永不落到 InputFullTray 操作員 modal**;(c) FindDischargeAuto(:415)/RequestTrayFromArm(:1133) 的鎖門消失 → 對滿車站繼續出料堆疊,無告警,直到 host START_AGV 重掛鎖或人工清車 | 黃(FX-1/FX-5;有 sensor 但 modal 路徑被 AMR 分支跳過) |
| PREP 落 HOME | HOME **進行中** ServiceHandshake 照跑(無 RunMode 門,VERIFIED)→ 軸還在 home 就可能發 CEID273 叫 AGV 對接(real:predicate 恰真;sim:恆真必發);完成時 InitialFlag 洗鎖、Handshake 殘留 PREP → infeed 前段 destack 解凍恢復升降(aEmpty.cpp:240/250 門失效)、Auto 被 TrayArm 重新餵盤 → IsDrainedForAmr 倒退,PREP 卡住 → 240s watchdog 靜默解約(無 SECS 事件)→ host/AGV 懸空;之後 Auto 滿邊緣+IsOperatorHolding 已被 :103 清掉 → 重複 CEID272 二次派車 | 紅(FX-1+FX-2+FX-3) |
| READY 落 HOME | AMR 可能已實體伸入。HOME 中:uHome case 200 批次 home 16 XY 軸(含 LoaderY/MAutoY)— 與 AMR 對接空間是否重疊未經協定(INFERRED 風險);完成時鎖被洗 → 前段 destack 缸在 AMR 補匣中恢復循環 / Auto 對「AGV 正在抬走的車」重新堆盤(M2 同類:僅升降台靜止才可介入,鎖=那個靜止保證)。SECS 面可自癒:sensor 與 AgvCoord 都存活,InfeedFinished/IsAmrTaken 真了照發 CEID274+InfeedRefill(車籍 TrayCount[0] 於 :81 重掛,資料 commit 完好) | 紅(實體互鎖消失;FX-1+FX-6 owner/AGV 決策) |
| Finish commit 當下 | CEID274+ClearAmrCar/解鎖+Refill+IDLE 在同一 1s tick 的同一 C++ 呼叫序列內,VCL timer 不可被 HOME 搶斷 → 不存在半套車籍 | 綠(原子,VERIFIED) |
| watchdog 計齡跨 HOME | ShortageDebounce 存活(不在 InitialFlag 管轄)且 HOME 中照增(ServiceHandshake 無門)→ 長 HOME(uHome case 200 含操作員取盤 modal,可達數分鐘)吃光 240s 額度 → HOME 尚未結束就靜默解約 | 黃(FX-2 凍結計齡即修) |
| HOME 中收到 START_AGV | uHGemHT160.cpp:910 無條件 BeginPrep、HCACK=0 → 上鎖數秒後被 HOME 完成的 InitialAllTask 洗掉 → 天生 PREP-無鎖分裂 | 黃(FX-4) |

#### 缺口

- FX-1(紅):Handshake[] 跨 HOME 存活(Reset 僅 ctor,uAgvStation.cpp:113)vs 四模組 bAmrLocked 被 InitialFlag 全清(aLoader.cpp:64/aEmpty.cpp:27/aColor.cpp:30/aAuto1To6.cpp:97)→ 鎖/狀態分裂;最小修:InitialAllTask 尾端(database.cpp:66 後)加 AgvCoord.ReassertLocks():凡 Handshake∈{CALLED(Auto),PREP,READY} 依站別重呼 SetAmrLock/InfeedSetLock(true)。
- FX-2(紅):ServiceHandshake 無 RunMode 門(uAgvStation.cpp:282-287)→ HOME 中照發 CEID273/274、照跑 ClearAmrCar/InfeedRefill、watchdog 照齡;最小修:HSys.Sys.RunMode==Run_Home(或 fAllMotorHome==false)時 return(轉移與 ShortageDebounce 皆凍結、不清零)。
- FX-3(黃):watchdog 解約(:314-323/:358-367)靜默無 SECS 事件 → host/AGV 不知交握被單方棄約,HOME 後還會重複 CEID272;最小修:解約時發告知事件(或至少 EventLog 落痕)+FX-2 凍齡降低誤觸。
- FX-4(黃):BeginPrep 無機台狀態門(uHGemHT160.cpp:910)→ HOME 中 START_AGV 被接受、鎖旋即被洗;最小修:Run_Home 時回 HCACK=2 或 defer 到 HOME 完成再執行 BeginPrep。
- FX-5(黃):CALLED(Auto)跨 HOME 後鎖消失但 Handshake=CALLED 同時擋掉 re-CALL(:237)與 ServiceCarFull 操作員 modal(aAuto1To6.cpp:1513-1519 else-continue)→ 滿車無鎖繼續被出料且無告警;FX-1 重掛鎖即修,或 PollAndCall 增「CALLED && !IsAmrLocked → 重掛鎖」自癒分支。
- FX-6(紅,owner/AGV 決策):規劃 drain/PARK 未定義「AMR 已靠站(READY)」例外——PARK 記憶回位/全機 home 是否可移動 LoaderY/MAutoY 穿越 AMR 對接空間、AGV 是否收「設備 HOME 中」凍結訊號,皆無協定(XS-4 所指);需 owner 確認 P1-P9 對接面與 home 路徑幾何 + host 端 abort/hold 協定,軟體無法單方收斂。
- FX-7(綠,低):AGV_FINISH 枚舉值(uAgvStation.h:26)、PrepDone、ReadyEntrySensor 從未被讀寫入邏輯、ShortageLatch 僅供快照 → 盤點噪音;註解標明或移除。

#### drain 邊界

AGV 協調器本體零可收斂邊界:uAgvStation.cpp 全檔不含任何 motor Move、TMyCylinder Push/Pop 或真空操作(VERIFIED),它只讀 predicate、發 S6F11、寫鎖位元,因此 phaseTable 各相位的 drain 一律「—/不可」,HOME drain 階段對它的正確處置是 RETAIN(凍結)而非 drain:進 Run_Home 即停 ServiceHandshake 轉移與 ShortageDebounce 計齡(FX-2),完成後依存活的 Handshake[] 重掛模組鎖(FX-1)。與 AGV 相關而真正可 drain 的只有模組側一種:BeginPrep 上鎖只擋「新分支起點」(aEmpty.cpp:240/250、aColor.cpp:303/362、aLoader.cpp:1197 皆在 case 100 派發點),在鎖落下之前已派發的前段 destack 缸梯(GoDown/GoUp 的 FrontRise/Separate,純缸+timer)仍在飛——把它收斂到相位邊界正好滿足 CEID273 的「前段缸全回家」gate(IsReadyForAmrHandoff,aLoader.cpp:517-527),故 PREP 站的 drain 有益且安全;READY 站則由 Ready gate 保證該站前段缸已回家、無 in-flight 缸相,drain 必為 no-op,唯 PARK/re-home 的 Y 軸移動是否允許在 AMR 靠站時執行屬 FX-6 的 owner/AGV 協定範圍,drain 規約不得先斬。

### TAutoModule::ServiceCarFull（黃）

> 覆核：ISSUES（已依覆核修正）。缺口代碼：FX-1, FX-2, FX-3, FX-4, FX-5, FX-6, FX-7, FX-8

ServiceCarFull 全函式無馬達/氣缸/真空——HOME 風險全在「資料 latch × 獨立 SECS tick」。三條已核實的鏈:(1) bOperatorHolding 在 InitialFlag 於 bKeepMaterial early-out 之前被無條件清除(aAuto1To6.cpp:101-103,設計註解自認 :98-100),而 PollAndCall 跑在 SECS 自己的 1s Timer(uHGemEquipment.cpp:158-175),只被 Run_Normal 閘住(uAgvStation.cpp:212)、不看 SystemStart——HOME 完成即 ChangeRunMode(Run_Normal)(csystem.cpp:1370/1380),若 InputFullTray 仍 ON(操作員搬車半途/重啟開機即滿),1 秒內 re-CALL 同站(:237-243)=人車爭道。(2) bOperatorHolding 無事件式清除(全 repo 僅 :103 寫 false、:1510 寫 true),一次 timeout 後該站下次滿:re-CALL 被抑制且 link-up 分支 else-continue(:1513-1519)跳過 modal → 無告警且 sensor-full 不擋餵(邏輯閘 :1106 要 count>=100 才停)——續往滿疊上疊終至無聲飢餓,HOME 是唯一解藥。(3) HOME 不碰 AgvCoord.Handshake[](InitialAllTask 僅 AmrInject.Reset,database.cpp:41)但清 bAmrLocked(:97)→ AGV_CALLED 孤兒(watchdog 只 aging PREP/READY,uAgvStation.cpp:314),且 lock 被清後餵/卸全無 Full-sensor 閘。:1541-1546 blocking do/while 經 bRunning latch(database.cpp:71-76)掛住整機 ladder,但也因此 HOME 無法在 modal 中啟動——出口=sensor OFF,:1547-1548 車籍 commit 冪等,ServiceHandshake→ClearAmrCar 為收斂性並寫(無發散)。sim 自動回收(:1524-1532)使 modal 視窗永不被 laptop 驗證。修法皆為可枚舉小補丁 → yellow。

#### 相位表

| 相位 | cases | 動作 | 確認源 | commit | 重入類別 | drain |
|---|---|---|---|---|---|---|
| P0 入口分流 | DoAuto case 100 每輪呼叫(:1589;一輪=1→100→1000→3000→1,feed/discharge 多掃描段 case 2000/4000 期間不執行,case 100 之 CleanOut 分支 :1581-1587 直接 Task=5000 繞過本函式);ServiceCarFull :1475-1478、:1491 | Run_Normal 閘;逐站迴圈;AMR 分支需 bUseAMR+HGem->IsSelected() | HSys.Sys.RunMode、HGem->IsSelected()(VERIFIED :1475、:1491) | 無 | 相位起點 | — |
| P1 AMR 滿車等待窗 | case 100 子段 :1493-1505 | 首掃描 SetMS+On+bWaitingAmrFull=true(:1497-1502);後續掃描 timer 未到即 continue(:1504-1505) | IsOutputCarFullForAmr:實機=InputFullTray sensor(:1243-1244)/sim=iSimAmrMaxTray(:1241-1242);bAmrLocked 由 PollAndCall SetAmrLock(:239)或 BeginPrep(:386)(VERIFIED) | bWaitingAmrFull latch+timer(非車籍) | 冪等段(重跑僅重看 timer;HOME 清 latch 見 W1) | —(純 timer/旗標,無馬達無氣缸,每掃描本即在邊界) |
| P2 逾時交棒 | :1508-1511 | bWaitingAmrFull=false、timer Clear、bOperatorHolding=true、AgvCoord.AbortAutoHandshake(釋放 bAmrLocked+Handshake=AGV_IDLE,uAgvStation.cpp:399-409 純資料) | AmrFullWaitTimer.Off()==true(:1504) | 同掃描四筆資料寫入;同執行緒無 pump,HOME 不可落中(VERIFIED) | 原子commit | — |
| P3 非滿/未鎖釋放 | :1513-1519 | 清 wait latch 後 continue;link-up 時 :1522 以下全部不可達(僅 P2 fall-through 例外)(VERIFIED 控制流) | bFull/bLocked 即時再評估 | 無 | 冪等段 | — |
| P4 sim 自動回收 | :1524-1532 | bLogicalFull(iTrayCount>=MAX_TRAY_PER_CAR=100,MyMotor.h:112)→ Car[].Clear()+InitAutoCarStack | 純邏輯計數(:1522;唯一寫入點 DoFeedTray :642) | 單掃描車籍歸零(:1528-1529) | 原子commit | — |
| P5 感測滿 blocking modal | :1537-1546 | do/while:ShowMyError→ShowNoteAlarm ShowModal(note.cpp:805,DecStopAllMotor+SystemStart=false :783-784);每圈重讀 sensor(:1544);期間整個 DoAllProcess 被 bRunning latch 掛住(database.cpp:71-76;csystem.cpp:853 註解 VERIFIED) | GetInputFullTray 每圈重讀,出口=sensor OFF(:1546) | 無(commit 在 P6) | 冪等段(sensor-gated,重啟後重入僅再彈 modal;危險在「阻塞」非重入,見 FX-4) | 不可(無馬達/真空,但邊界=人為 sensor OFF、無界;drain 不得進入,必吃 timeout fallback) |
| P6 感測路徑車籍 commit | :1547-1548 | Car[].Clear()+InitAutoCarStack;全機唯二實體滿疊 commit 之一(另一為 clean-out 同型 :884-885) | 前置條件=P5 出口 sensor OFF 剛確認 | 單掃描;modal 期間同顆 1s Gem tick 除 PollAndCall(僅讀 Car,uAgvStation.cpp:224-231)外亦跑 ServiceHandshake(uHGemHT160.cpp:191-192):host 自主 START_AGV→BeginPrep 無 IsOperatorHolding 閘(uAgvStation.cpp:378-389)→PREP→READY(IsDrainedForAmr,:293-300)→IsAmrTaken=InputEnd sensor OFF(aAuto1To6.cpp:1315-1318,操作員清疊時正好成立)→ClearAmrCar(:302-309→aAuto1To6.cpp:1328-1330)可在 modal 中寫 Car[]——但屬收斂性寫入(同為 Clear+InitAutoCarStack,外加釋放 lock),與 :1547-1548 重複執行冪等 → 無發散競態,原子commit 結論不變(VERIFIED;缺閘本身列 FX-8) | 原子commit | — |
| P7 邏輯滿 modal+commit | :1550-1556 | 單次 ShowMyError(K_RETRY)後 Car[].Clear()+InitAutoCarStack(:1555-1556) | 無感測——僅操作員 RETRY(FX-7);Full sensor 為事後最後防線(:1537 下輪補抓) | modal 後單掃描車籍歸零 | 原子commit | 不可(blocking modal,同 P5) |

#### 中斷視窗

| 視窗 | 今日行為 | 缺口/判定 |
|---|---|---|
| W0 共通機制 | Note modal 掛住 MainProc(bRunning,database.cpp:71-76)→ modal 中 HOME 無法啟動;PollAndCall 在 SECS 自有 1s Timer(uHGemEquipment.cpp:158-175)獨立跳動,僅被 Run_Normal 閘(uAgvStation.cpp:212)、不看 SystemStart;HOME 完成即 InitialAllTask(true)+ChangeRunMode(Run_Normal)(csystem.cpp:1359-1382)(VERIFIED) | 判定基準:HOME 後 1 秒內 AGV 協調器即恢復行動,而 Auto 側 latch 已全被清 |
| W1 等待窗中 HOME(P1) | InitialFlag 清 bAmrLocked(:97)+bWaitingAmrFull/timer(:101-102,刻意置於 bKeepMaterial early-out 前 :98-100);Handshake=AGV_CALLED 殘留(HOME 無 AgvCoord hook;Reset() 全 repo 僅 ctor 呼叫 uAgvStation.cpp:113,:188-209 為 PollAndCall 內 link-down inline 釋放迴圈——效果等價但非 Reset 呼叫)→ 半釋放:不 re-CALL(:237 需 IDLE)、不彈 modal(:1513-1519 else-continue)、CALLED 無 watchdog(:314 僅 PREP/READY)。且 bAmrLocked 被清後餵/卸閘全開:GetTrayRequest 僅剩 working/rear 佔用閘(:1136-1139),邏輯滿閘(:1106)要 iTrayCount>=MAX_TRAY_PER_CAR=100(MyMotor.h:112)才擋,而 W1 的「滿」實機=InputFullTray sensor(:1243-1244)任意 count 即成立;FindDischargeAuto(:409-421)亦無 Full-sensor 閘 → 常態(sensor-full 且 count<100)下 TrayArm 照常餵盤、discharge 往已滿疊上疊,無任何防線(VERIFIED)。host 送 START_AGV 則 BeginPrep 重上鎖恢復(:378-389);host 不送則該站無聲加疊/飢餓 | 紅:planned 協議第 5 步(InitialAllTask(true))原樣保留此窗;需 FX-3 |
| W2 逾時交棒瞬間(P2) | 單掃描原子,同執行緒,HOME 不可落中 | 綠 |
| W3a modal 掛起中(P5,sensor ON) | HOME 軟體上不可啟動(modal 擋 UI+MainProc;Note 面板鍵無 HOME,note.cpp:711-717);Gem tick 照跑但 holding=true 抑制 re-CALL(:237)——latch 在 modal 期間正確工作(VERIFIED) | 綠(今日);但 EMG/斷電重啟繞過 modal → 落入 W3c |
| W3b modal 結束後(sensor 已 OFF) | commit(:1547-1548)正確;bOperatorHolding 殘留 true(僅 :103 可清)→ 下次滿:re-CALL 抑制(:237)且 link-up 分支跳過 modal(:1513-1519)→ 無 AGV、無告警,且 count<100 期間 TrayArm/discharge 續往 sensor-full 疊上疊(:1106 邏輯閘不擋)→ 無聲越疊終至飢餓,直到有人 HOME | 紅:FX-2;違反 silent-stop-must-notify 慣例 |
| W3c HOME/程式重啟完成時 sensor 仍 ON | holding 被 InitialFlag 清(:103)或重啟歸零 → Run_Normal 後首個 1s tick re-CALL+上鎖(:237-243)→ AGV 派往操作員仍在作業的同站 | 紅:FX-1(人車爭道);planned 協議不改此行為 |
| W4 sim 路徑(P4) | HOME 任意落點安全(純資料);plain sim 永不進 modal;sim+host-simulator 可再現 timeout→holding 飢餓(:1508-1510 後 :1524 不觸發因 bLogicalFull 用 100 門檻),但 P5/P6/P7 永不可達 | 黃:FX-6,實機獨有視窗無迴歸覆蓋 |
| W5 邏輯滿(P7) | MAX_TRAY_PER_CAR=100(MyMotor.h:112)→ 實機幾乎必由 sensor 路徑先觸發,近死路徑;commit 憑操作員宣稱,Full sensor 事後補抓 | 黃:FX-7(低暴露) |

#### 缺口

- FX-1(紅):bOperatorHolding 於 keep-material HOME 被無條件清除(aAuto1To6.cpp:101-103 在 :107 continue 之前;csystem.cpp:1359 完 HOME 即呼叫;註解 :98-100/uAgvStation.cpp:236 自證為刻意)而該 latch 模型化的實體事實(人在搬滿疊)無法由 InitialFlag 感測重導出 → HOME 完成→Run_Normal→1s 內 PollAndCall re-CALL 同站(:237-243)=人車爭道。最小修法:holding latch 移到 bKeepMaterial early-out 之後保留,改以 InputFullTray OFF 邊沿事件式釋放(兼治 FX-2);殘餘政策問題(sensor OFF 是否=人已離場)可留 owner 一句確認。
- FX-2(紅):bOperatorHolding 無事件式清除(全 repo 唯 :103 寫 false、:1510 寫 true,VERIFIED grep)→ 一次 timeout 後該站下次滿:re-CALL 被 :237 抑制且 link-up 分支 else-continue(:1513-1519)跳過操作員 modal → 無 AGV、無告警;count<100 期間 TrayArm/discharge 續往 sensor-full 疊上疊(:1106 邏輯閘要 count>=100 才停餵)→ 無聲越疊終至單站飢餓,直到 HOME。最小修法:於 :1547-1548 commit 點(sensor OFF 已確認)同步 bOperatorHolding[Index]=false。
- FX-3(黃):HOME 不觸碰 AgvCoord.Handshake[](InitialAllTask 僅 AmrInject.Reset,database.cpp:41;AgvCoord.Reset() 僅 ctor uAgvStation.cpp:113 呼叫)卻清 bAmrLocked(:97)→ AGV_CALLED 半釋放孤兒(無 watchdog :314;PollAndCall 需 IDLE;ServiceCarFull 需 bLocked)→ host 不補 START_AGV 即永不彈 modal,且 sensor-full 站照常受餵/受卸(見 W1)。最小修法:InitialAllTask(或 Auto InitialFlag)對六站呼叫既有冪等 AbortAutoHandshake(uAgvStation.cpp:399-409),讓滿邊沿重新乾淨 re-CALL。
- FX-4(黃):blocking do/while modal(:1541-1546,同型複本 clean-out :878-885)經 bRunning latch(database.cpp:71-76)掛住整機 ladder、無 case 邊界 → 對 planned DRAIN 協議是不可標記的黑洞(雖無馬達/真空,邊界=人為 sensor OFF、無界)。最小修法:改為 case 100 內非阻塞 held-alarm 子狀態(每掃描檢 sensor、modal 走既有 Note 常規),drain 表即可標「—」。
- FX-5(黃):CALLED 孤兒態下實體滿疊被非握手路徑搬走(sensor OFF 邊沿 :245-249 釋放 handshake)時,keep-material HOME 保留的 Car[] 簿冊過期:count 已達 100 者 GetNextTrayKindForAuto=-1(:1106-1107)永不再受餵;count<100(常態)者表現為車籍 identity/疊序過期(新車仍按舊 count 續記)而非飢餓。link up 時 MES%d25 不可達(:1513-1519)→ 車籍重置僅剩 ClearAmrCar(:1324-1329)與非 keep InitialAllTask 兩條路。最小修法:sensor-OFF 邊沿+空車確認時重置車籍或至少 EventLog+Note 告警。
- FX-6(黃):SOFT_SIMULATE 走 :1524-1532 自動回收 → P5/P6/P7(modal+車籍 commit)在 laptop 永不可達;timeout→holding 鏈唯有 sim+SECS simulator 可再現。最小修法:SECS simulator 補「timeout→holding→HOME→re-CALL」腳本場景 + ht160s-home-selftest 註記此為實機獨有視窗。
- FX-7(綠,備註):MES%d25 邏輯滿 commit(:1555-1556)僅憑操作員 RETRY、無感測驗證;MAX_TRAY_PER_CAR=100(MyMotor.h:112)使實機幾乎必先觸發 sensor 路徑 → 低暴露;Full sensor(:1537)為事後最後防線。可不修,文件註記即可。
- FX-8(黃):BeginPrep 無 IsOperatorHolding 閘(uAgvStation.cpp:378-389)→ timeout 後操作員搬車中,host 自主送 START_AGV 即重上鎖進 PREP;IsDrainedForAmr(:293-300)+IsAmrTaken=InputEnd OFF(aAuto1To6.cpp:1315-1318,清疊中正好成立)可一路走到 ClearAmrCar(uAgvStation.cpp:302-309→aAuto1To6.cpp:1328-1330)——寫入收斂(同 commit 形+釋放 lock)故無資料競態,但 AGV 被派往操作員仍在作業的站=窄觸發人車爭道。最小修法:BeginPrep ASK_AUTO 分支檢 IsOperatorHolding,holding 中回 false 讓 host 收 error(或忽略該 START_AGV)。

#### drain 邊界

ServiceCarFull 整函式(aAuto1To6.cpp:1473-1559)不含任何 Move* 馬達呼叫、TMyCylinder Push/Pop、或真空/物料 handoff——全部是旗標、HTimer、sensor 讀取、車籍 RAM 寫入與 Note modal(逐行核實)。因此 HOME 的 DRAIN 階段對本 ladder 的正確策略是「no-op 跳過」:P1(:1493-1505)/P3(:1513-1519)每掃描本即在相位邊界,無在途氣缸可收斂;P2/P4/P6/P7 的 commit 皆為單掃描原子。唯二不可由 drain 進入的是兩個 blocking modal 段(:1541-1546 與 :1550-1554,含 clean-out 同型 :878-885):它們雖無馬達/真空,但邊界條件是人為的 InputFullTray sensor OFF、無時間上界,drain 若進入必吃 timeout fallback 且會經 ShowNoteAlarm 把 SystemStart 打掉(note.cpp:783-784);實務上這不可能發生——modal 掛起時 MainProc 已被 bRunning latch(database.cpp:71-76)凍結,HOME 根本無法啟動。真正要在 HOME 流程補的不是 drain 邊界,而是第 5 步 InitialAllTask(true) 前後的 latch 保留/釋放(FX-1/2/3)。

## 完整性批判

批判指出的遺漏（已以補漏表處理）：
- **AGV/AMR 交握協調 ladder(TAgvCoordinator::PollAndCall / ServiceHandshake / BeginPrep + 模組側 SetAmrLock/InfeedSetLock)** — 這是實體 AMR 換車的唯一節拍器,且 16 張表無一為它建 per-phase 視窗表(XS-4 只列缺口)。VERIFIED:Handshake[] 跨 HOME 存活(Reset 僅 ctor 呼叫)而四模組 bAmrLocked 被 InitialFlag 全清(aLoader.cpp:64、aEmpty.cpp:27、aColor.cpp:30)→ HOME 落在 AGV_PREP/READY 時,destack/堆疊在 AMR 靠站伸入期間恢復動作(InfeedSetLock 的凍結被洗掉);READY 中 HOME 後 CEID274/ClearAmrCar 車籍 commit 視窗、watchdog 計齡(ShortageDebounce)跨 HOME 殘留皆未盤點。規劃 drain/PARK 也未定義「AMR 已靠站」時是否允許執行——正是 XS-4 說需 owner/AGV 協定的那一塊,無表就無法收斂。（何處：D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\SecsGem\uAgvStation.cpp:179-273(PollAndCall)、280-371(ServiceHandshake 含 watchdog 強制解鎖)、378-390(BeginPrep 上鎖);模組側 aAuto1To6.cpp SetAmrLock/IsDrainedForAmr/IsAmrTaken/ClearAmrCar(:1324)、aLoader/aEmpty/aColor 的 bAmrLocked+IsReadyForAmrHandoff/IsInputHandoffFinishedForAmr）
- **TAutoModule::ServiceCarFull(滿車換車背景 ladder,含車籍 commit 與 AMR 滿車等待窗)** — 含全機唯一的滿車車籍 commit:Car[].Clear()+InitAutoCarStack(:1547-1548 sensor 路徑、:1555-1556 邏輯滿路徑),對實體滿疊無任何 HOME 保留;且 :1541-1546 是 blocking do/while modal(機控路徑內的阻塞迴圈)。碼註自證(VERIFIED):bOperatorHolding 於 HOME/InitialFlag 被清 → 操作員正搬滿車途中做 HOME,PollAndCall(:237)立即對同站 re-CALL AGV=人車爭道;另 bWaitingAmrFull/AmrFullWaitTimer 跨 HOME 的殘留/歸零與 AbortAutoHandshake(:1511)的半釋放態皆無表。sim 路徑自動清車(:1524-1532)使 SOFT_SIMULATE 驗證永遠測不到這些窗,實機才會踩。（何處：D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\aAuto1To6.cpp:1473-1559(DoAuto case 100 每輪呼叫 :1589);bOperatorHolding 語意 :1268-1276;uAgvStation.cpp:234-236(re-CALL 防護依賴此 latch)）

**表間矛盾：**

1) 【直接衝突】LF-1 vs LK-3:loader-feed 要求「HOME drain 將 FeedTask 收斂到 9500 完成 mint」以關閉未鑄造視窗;loader-destack LK-3 明文「9000/9500/10000 禁入(modal+鑄造+owner 釋放)」、drain 僅泵 {4000,4100,8200,8300} 至 9000 進入點,改以 LK-1 的 resume 側補鑄造收口。同一視窗兩種互斥 drain 規則,實作前必須裁決(建議採 LK-3+LK-1:9500 前有 modal 段,drain 內執行鑄造違反自家「drain 不得含 modal」規約)。2) 【同 case 不同規則】TA-1 vs TR-5:共用 helper DoLowerClampRaise 的 PlaceTask 2000(兩表同引 aTrayArm.cpp:426/434-438)——place-auto 允許「Pop 輸出已命令則續收斂至完成」的條件式 drain;place-recycle 明文 2000 不納入(除非 owner 裁決),理由是 Pop 已下令未確認子態無法以 Task 值區分。drain 引擎看到的是同一個 Task 變數,規則必須統一或明文按目的地分流;TR-5 的「無法以 Task 區分」若成立,TA-1 的條件其實須改讀缸 out-bit 才可判。3) 【邊界不一致】LF-2 稱 destack drain「收斂→FeedTask 8200」,LK-3 稱 8200/8300 應續泵至 9000 進入點——小差但 drain 表格必須單一邊界。4) 【今日行為判定不一致】EF-1/CF-2 無條件宣稱「uHome case 200 歸 Y 即拖盤(必)」;CG-2/AF-4 則引 IsHomedServoSkippable(uHome.cpp:257-262、:639,已驗證存在)限定「僅 stepper/警報/未 home 才實體重歸,已 home 伺服跳過、拖盤延後到 resume 首次 Move」。結論(需 PARK)不變,但 empty-feed/color-feed 的紅色視窗敘述把觸發時點寫錯位,phaseTable 的「今日行為」欄需統一口徑(且 MEmptyY 是否伺服需查 Mot_Table 確認)。5) 【宣稱範圍過寬】trayarm-place-recycle 第1點「互等迴圈已端到端關閉(VERIFIED)」與 XS-2/CF-4 並存:heal 只覆蓋 arm 仍持盤(aTrayArm.cpp:1072 閘),「已放未簽」視窗 heal 反而驅動 GoUp 吃掉剛放之盤+空爪簽幽靈 finish——關閉宣稱應限定為「攜盤中」子集。6) 【重複修法需合一】AC-3 與 AF-2 同治 Auto FeedTask 6000→7000 視窗但落點不同(drain 收尾代跑 case 7000 vs InitialFlag(bKeepMaterial) 代行承接);SR-1 與 AD-4 為同一行(aAuto1To6.cpp:94)同一修法——皆非矛盾,但驗收清單須合併為單一工項,避免雙重實作。

**批判筆記：**

1) 匯總不是純黃:存在一個「owner 決策叢集」——XS-4(AMR 已靠站中復機協定)、SP-1(Z 下位斷真空 IC 留穴為物理推論)、AC-1(EMG 斷氣時 FrontRise 行為)、TR-5(PlaceTask 2000 開夾裁決)——這些在 owner 拍板前應標紅;加上本審查找出的兩張缺表(AGV 協調器、ServiceCarFull)都落在 AMR 象限,建議 rollup 拆成兩行:AMR=0 Normal 模式=黃(可枚舉小修),AMR 模式=紅(缺表+缺協定)。2) ≥6 張表獨立要求同一件事:Empty/Color/Loader 的 InitialFlag 增 bKeepMaterial 變體(EF-1、CF-1、CG-1、LF-4、LK-2/LK-7、XS-5)——這是單一共用重構(對齊 Auto/TrayArm/SortArm 簽名),排程時應合併為一個工項而非六個補丁,否則會六次改同一函式。3) uHome case 200 強制移盤+ClearTray(uHome.cpp:659-668)被 LF-6、LK-7、LD-3 三表同時要求「與 PARK/RE-ACQUIRE 同批原子落地」,且 sortarm-pick 第2點指出今日的 grid 洗除反而掩蓋了 SP-1 的重扎風險——拆除它是全案耦合度最高的單點,建議在計畫文件單獨立項。4) drain 規約衝突(矛盾 1-3)+四表的 modal 抑制要求(EG-3、AF-3、AC-5、SR-4)顯示 drain 引擎規格需要一份仲裁文件先行,逐表各自宣告邊界無法直接實作。5) ServiceCarFull 與 AGV 協調器的視窗在 SOFT_SIMULATE 下不可見(sim 自動清車 aAuto1To6.cpp:1524-1532、sim 無 Full sensor),依「SOFT_SIMULATE=laptop verification」慣例,這兩張缺表補齊後的驗證必須排實機或 SECS Simulator 注入(AmrInject 已有測試注入點,uAgvStation.cpp:233/259/295/304 可用)。6) 已確認非缺口:Color DoSortBin 為 stub(aColor.cpp:1195-1210 僅 case 1 return true),CheckHoldFallDown 的兩個呼叫點(aSortArm.cpp:1701/1777)都在已建表 ladder 內且 SP-2 已涵蓋其 HOME 後降級語意——不需補表。
