# HOME 汽缸收斂段（drain）仲裁規格（W2，v1，2026-07-11）

單一權威文件：drain 引擎的位置、閘門、規則、與**全機統一的 per-ladder 收斂邊界表**。
各 action 相位表（docs/plan/home-resume-phase-inventory.md）的 drain 欄與本文件牴觸時，
**以本文件為準**。決議輸入：五步協定＋M1/M2/A1-A4＋D1-D4
（docs/plan/home-resume-phase-tables.md 決議表）。

## 1. 引擎位置與時序

- 落點：`TfHome::ProcessMotorHome` 新前置段（case 1 之前，建議 iHomeStep=0 的
  「Drain」段），由 HOME 引擎直接 tick 各模組的**汽缸子梯**。此時 Run_Home 已凍結
  模組派工、任何馬達尚未收到命令 → D4 的「馬達未動、歸位未開始」前提天然滿足。
- 全程馬達零命令：drain 段內**禁止任何 Move\*/MotorMove/歸位命令**。標「可收斂」的
  相位必須經敵意覆核確認無馬達呼叫（盤點已做）。
- 同拍 pump（EG-2/LK-5）：drain 引擎在**同一掃描**依 MainProc 原有順序 tick 全部
  參與模組的子梯——跨模組缸互鎖（Empty↔Loader front-separate）自然解，不得逐模組
  串行 drain。
- 全域逾時：可調（GeneralSetting，預設 15s）。逾時或任一缸自身 Push/Pop 逾時
  → 該梯 drain 中止（abort），落入 fallback（PARK／resume 側守衛／人工），
  HOME 引擎續行。缸逾時**不得**在 drain 中彈告警（見 §3）。

## 2. 成因閘門（依 D3 收斂）

| 觸發情境 | drain？ | 理由 |
|---|---|---|
| 一般操作員／SECS HOME、伺服警報復位後 HOME、EMG 復位後 HOME | **跑** | D3：EMG 只斷馬達電，氣不斷；HOME 本來就在復位後才跑 |
| 氣壓不足 Alarm 現存 | **跳過** | D3：氣不足由 sensor 判斷並中斷所有動作，缸動作不可信 |
| 安全門開／SystemStart 斷 | 不會發生 | HOME 引擎本身不步進（既有行為） |

drain 被跳過的梯：不得自動續跑，靠 retain latch（如 `bGoUpPassedApex`）或
resume 側守衛收斂（各表 fallback 項）。

## 3. modal 禁令與預讀規則（EG-3/AF-3/AC-5/SR-4 統一）

- drain 段**絕對禁止**彈出任何 Note/MyMessageBox（modal 會凍結 MainProc、卡死 HOME）。
- 凡收斂路徑上存在 sensor 判定告警（JAM1030、MES1021、MES1024、JAM%d11、Full-gate、
  MES1421…）：drain 先**預讀**該 sensor——會走告警分支者，該梯 drain 中止，
  留給 resume 後的正常重跑去彈（該彈就彈，只是不在 drain 內彈）。
- 殘料驗證 verdict 若為「有殘料」：abort-fallback＝保留 `bResidueClear=false`
  ＋`bNeedResidueCheck`，不彈 modal（SR-4）。

## 4. AMR 靠站約束（依 D1）

- AMR 靠站中**允許 HOME**。
- 約束：HOME 全程（含 drain）對「Handshake 非 IDLE 的介面站」**凍結 GoUp/GoDown 與
  分離夾爪動作**（destacker rise/separate 家族），只做 sensor 判斷。該站的 destacker
  子梯不納入本輪 drain，走 retain latch／resume 守衛 fallback。
- 非 destacker 類（TrayArm 放盤交接、Auto rear→car 代跑、殘料驗證）不受此約束。

## 5. timer 陷阱規約（LK-4/AC-6 統一）

- 禁止任何「跳段進入」帶已 Clear 之 HTimer 的 case（`Off()` 永假＝卡死）。
- drain 只能**順向 pump 既有梯形**（從當前 Task 值往前走），或執行本文件明列的
  「代跑 commit」；不得直接改寫 Task 跳入段中。

## 6. 全機統一收斂邊界表（仲裁後）

「→X 進入點」＝pump 到 Task 停在 X、不執行 X 本體。「完成」＝含該梯終端 commit。

| 梯 | 當前 Task 範圍 | 收斂邊界 | 仲裁註記 |
|---|---|---|---|
| Empty DoGoUpTray 相位A | 100-600 | 完成（600 commit） | 純缸 |
| Empty DoGoUpTray 相位B | 3000/4000（夾持缸段） | →5000 進入點後改走 PARK | 5000=馬達，不可入 |
| Empty DoGoUpTray 相位B | 6000/7000 | 完成（7000 commit） | 純缸+資料 |
| Empty DoGoDownTray | 100-600 | →700 進入點 | 700 的 sensor 判定留給 resume（EG-3） |
| Empty DoFeedTray | 2000（含 ClampSub） | →4000 進入點後改走 PARK | 1000/4000=馬達；PARK 放夾（A1） |
| Empty DoFeedTray | 5000/6000/7000 | 完成（→13000） | 預讀後座 sensor，miss→abort（§3） |
| Color 對應梯 | 同 Empty 各段 | 同 Empty | 鏡像機構 |
| Loader destack（DoFrontDestackDown＋FeedTask 4000/4100/8200/8300） | 上列 | **→9000 進入點** | **裁決 LF-1 vs LK-3：採 LK-3＋LK-1**——drain 不執行 9500 鑄造（9500 前有 modal 段）；未鑄造盤由 resume 側自收（SnLoader_InputHasTray ON 且 fHasTray==false → 直入 9500，LK-1）。LF-2 的 8200 邊界作廢，統一 9000 |
| Loader DoDischargeTray | 2000/3000 | →4000 進入點 | 4000=馬達退車；發布補打見 LD-1 工項 |
| TrayArm DoPick | PickTask 1000-3000 | **完成＝case 4000＋DoTrayArm case-1000 收尾 tick 一起執行**（TP-2） | 純缸+單掃描資料；**必須在 Loader/Empty/Color InitialFlag 抹除前執行**（來源身分還在） |
| TrayArm DoPlace／DoPlaceToEmpty／DoPlaceToColor | PlaceTask ≥1000 | **完成（含 case 4000 通知）** | **裁決 TA-1 vs TR-5：依 D4 開夾交接允許**——deposit 梯（Z 下、開夾、dwell、Z 上、通知）全為缸+資料，rear-clear 閘（case 500）已通過才會進 1000。case 1/10 不納入（含 MoveTrayArmX）。A2 滿足：drain 中 ArmX 靜止 |
| SortArm pick 梯 | 全段 | **不 drain；改做真空對帳（§7）** | 無純缸相位 |
| SortArm 殘料驗證 CheckPlaceResidue | ResidueTask 全段 | 完成（bAllDone，含 SetPlaceResidueClear 回報） | 純 solenoid+timer；D3 保證氣在 |
| Auto DoFeedTray | 4000/5000/5100/5200（含 3000 gate） | →6000 進入點 | 3000/5200 內含告警→§3 預讀 |
| Auto DoFeedTray | 6000/7000 | **完成＝代跑 case 7000 單掃描 commit** | AF-2≡AC-3 合併工項；純資料 |
| Auto DoDischargeTray | 3000/4000 | →5000 進入點 | 5000=馬達；中斷補償走 AD-1 latch |
| Auto DoDischargeTray | 6000/6100 | 完成 | 純缸（FrontRise） |
| Auto CleanOut | 2000-5000 | →6000 進入點 | Full-gate 遇 ON：重夾（Push/Lean On）後中止該站 drain（AC-5），不彈 modal |
| ServiceCarFull | — | 不 drain（改非阻塞化，W5/FX(S)-4） | blocking modal 無 case 邊界 |

## 7. SortArm 真空對帳（SP-1 程序，依 D2 修正）

時點：drain 段末尾、uHome case 100（SuckZ 歸位）**之前**——此時 bCanPick 尚未被
wipe、Z 仍在下位（穴內）。

1. 逐針檢查：真空輸出 ON 且 `bCanPick && !bHasIC`（吸了但未 commit）→
   **斷真空＋開吹氣（OnDestroy）＋dwell**（D2：不吹氣 IC 會黏嘴不留穴）。
2. IC 留在來源穴內；該針 Slot 清除（來源格資料未動過，resume 重掃格自然重揀）。
3. **吹氣關閉點＝case 100 SuckZ 歸位完成之後**（Z 已回 Safe；D2 提醒），
   不等到 InitialFlag——在 case 100 完成點統一 Off 對帳吹氣。
4. 氣壓不足 Alarm 現存（§2 跳過情境）：不做對帳，HOME 完成後逐針讀真空 reed
   盤點＋一次性告警（不靜默）。
5. 已 commit（bHasIC）的針維持既有 keep-material 行為（保留酬載＋重新上真空）。

## 8. 驗收注記

- drain 引擎與各梯邊界的驗證：SOFT_SIMULATE 可驗 pump 邏輯與邊界停點；
  缸 reed 快轉、modal 預讀、真空對帳需實機。
- AMR 象限（§4 凍結、ServiceCarFull/AGV 協調器）sim 結構性不可見——排實機或
  SECS Simulator（AmrInject 注入點）。
- 每梯落地時：相位表 drain 欄若與本表不一致，以本表修正相位表並註記。
