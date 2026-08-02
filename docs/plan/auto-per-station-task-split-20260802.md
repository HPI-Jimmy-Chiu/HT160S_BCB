# Auto1~6 各站獨立 Task — 設計、危害與分階段執行計畫

- 日期：2026-08-02
- 分支：`feat/iosetview-172-refactor`
- 起因（使用者原話）：「我看現在 Auto 寫法，是不是不能每個 Auto 都有獨立運作的 task？如果當下
  `DoAuto(int &Task)` 在 task 2000，也許 auto1 正在執行，但是 auto2 其實需要空盤，會導致 auto1 要跑完，
  才能換 auto2？但其實 auto1-6 是獨立個體，不用互相等待。auto1-6 當初設計一套是想要學 Loader 一樣，
  同樣的動作模組而已。」
- 方法：4 路平行設計查證（變更面 / 併行危害 / 運動與氣壓可行性 / 遷移設計），117 條發現。
  ⚠ **對抗式複驗階段全數因 session 額度中斷未執行**，因此本文所有「已驗證」項目均為**我本人重新開檔覆核**
  的結果，其餘標示為 finder 未複驗。

---

## 1. 結論先講：拆，但**只拆一半**

**要做（P1，本文的執行範圍）**：把 `DoFeedTray` / `DoDischargeTray` 參數化為 per-station，
游標移進 per-station 狀態。**派工維持序列，行為位元不變。**

**先不要做（P2，需要現場數據才值得）**：真正的併行派工（六個 action 同時推進）。

**為什麼**——三個獨立理由：

### 1.1 使用者描述的症狀，不是共用游標造成的

`GetTrayRequest()` / `FindTrayRequestAuto()`（`aAuto1To6.cpp:1228-1303`）是**純狀態函式**，
由 TrayArm 直接讀取，**與 `DoAuto` 的 Task 完全無關**。所以「auto2 需要空盤」這個**需求本身
從來不排隊**——車一空、盤一到位，需求即刻對 TrayArm 可見（實測梯形圖 4.07 ms 一步）。

真正序列化的只有兩段**動作**：把盤從 rear 拉到工作位（feed）、把滿盤送上堆疊車（discharge）。

### 1.2 實測：六份現場快照裡，**從來沒有兩台 Auto 同時符合條件**

| 快照 | 同時符合條件的站數 |
|---|---|
| 14:36:33 | 0 |
| 14:42:17 | 0 |
| 14:43:29 | 1（Auto3 DISCHARGING） |
| 14:51:06 | 0 |
| 16:17:31 | 0 |
| 16:38:37 | 1（Auto5 FULL） |

判定式取自 `aAuto1To6.cpp:448-470`（feed：`CarHasTray==0 && RearHasTray && RearPending`）與
`:472-484`（discharge：`FullIC && ResidueClear && !AmrLocked`）。

更關鍵：**六份快照裡每一台的 `RearPending` 都是 0**——TrayArm 從來沒有在任何一個 Auto rear
積壓過盤，也就是**供料路徑從未排隊**。

節拍量測：TrayArm 一趟 lap 實測 17.31 / 10.76 / 9.36 / 20.94 / 17.81 / 11.44 / 15.45 / 20.74 / 17.90 秒
→ **平均 15.75 秒送一盤**。對比一次完整 `DoFeedTray` 實測 **1.988 秒**（其中 1.000 秒是設定固定的
`AutoPushConfirmSettleMs`）。**單一手臂的供料速率是硬上限，Auto 梯形圖併行不會變出盤來。**

### 1.3 而且這份數據**高估**了效益

現場 recipe `Test20260611` 的盤面幾何是 `XDivision=1 YDivision=3`（每盤 **3 格**，
`setup.ini:10-11`）——**三顆 IC 就換一盤**，這是本機理論上的**最大換盤頻率**。
量產盤面（數十到數百格）會讓換盤頻率掉一到兩個數量級。
**在最大換盤率下都從未出現兩台同時符合條件，量產幾何只會更少。**

### 1.4 但 P1 仍然非做不可——它修掉一個真實的潛伏資料錯誤

`aAuto1To6.cpp` 裡 **78 處**站別相關讀寫**全部**透過共用游標 `iFeedAuto` / `iDischargeAuto`，
**沒有一處**用區域索引。今天靠「梯形圖線性、一次只服務一站」這個**偶然性質**才安全。
其中 `DoFeedTray` case 7000（`:705-729`）會蓋 `Car[].Tray[n].SetKind` / `TrayID` / `CarID` /
`iTrayCount`。一旦有第二站插入，就會蓋錯車——而且**會直接上到 host**：
CEID 272/273/274 的 Report 6 會回報錯的 TrayCount、DeviceCount、CarrierID，
**客戶可見的 MES 資料完整性缺陷，且無任何執行期偵測**。

把 index 參數化之後，`Car[Index]` 天生指向正確的站，**這個危害從結構上消失**，
而且是後續任何併行化的必要前提。

---

## 2. 現況結構（證據）

```
DoAuto(int &Task)                      // Task = action 的 Tag，全模組唯一
  case 1    -> 100
  case 100  -> CleanOut 入口 / CheckAutoTray() / ServiceCarFull() -> 1000
  case 1000 -> FindFeedAuto()>=0 ? (DoFeedTray(0), 2000) : 3000
  case 2000 -> DoFeedTray(1) 完成 -> 3000
  case 3000 -> HOME 尾單 / FindDischargeAuto()>=0 ? (DoDischargeTray(0), 4000) : 1
  case 4000 -> DoDischargeTray(1) 完成 -> 1
  case 5000 -> DoAllAutoCleanOut(1) 完成 -> 1
```
（`aAuto1To6.cpp:1695-1782`）

- **feed 與 discharge 在時間上互斥**——它們是同一條線性梯形圖的兩個相位。
- 每個相位由 first-match 掃描（`FindFeedAuto` `:448` / `FindDischargeAuto` `:472`）挑**恰好一台**。
- `TAutoStationState`（`aAuto1To6.h:34-44`）**沒有任何 task 成員**。
- 模組持有**各一份**：`FeedTask`(25 處引用) `DischargeTask`(10) `CleanOutTask`(18)
  `DischargeSubTask`(5) `iFeedAuto`(**51**) `iDischargeAuto`(28) `FeedDelay`(5)
  `DischargeDelay`(7) `CleanOutDelay`(5) `TestUpTask`(3) `TestDelay`(3)。

**派工層**（`database.cpp:79-99`）：`DoAllProcess()` 泛型走訪 `UserMotion->ActionCount`，
每個 `act*Execute` 把**自己 action 的 Tag** 以參考傳入當模組游標。
Loader 是雙線道前例：`DoLoader(1, P->Tag)` / `DoLoader(2, P->Tag)`，
且每條子梯形圖游標都在 `TLoaderSideState Side[2]`（`aLoader.h:23-41,:59`）。

---

## 3. 已本人覆核的關鍵事實

| 項目 | 判定 | 證據 |
|---|---|---|
| 六支 Y 軸真正獨立 | ✅ 成立 | `MoveAutoY(Index,Pos)` → `GetAutoMotor(Index)->MotorMove()`，**無 Loader 那種共軌 `iYOwner` 所有權權杖**。`Mot_Table.csv`：MAutoY_1..3 在 BoardID **0**（軸 5/6/7），MAutoY_4..6 在 BoardID **1**（軸 0/1/2） |
| 六站同動已在真機發生過 | ✅ 成立 | `DoAllAutoCleanOut` 一個 scan 內下六個 `MoveAutoY` 與六個 FrontRise（`:889-892`、`:945-996`、`:1012-1015`） |
| 派工可乾淨擴充 | ✅ 成立 | `DoAllProcess` 泛型走訪；`cStateRecordHT160::EnsureInited` 讀 `ActionCount` 自動建表（`:169-193`）；`SR_MAX_MODULE=16`，目前 7 個 |
| **開機自檢閘門** | 🔴 **確認的地雷** | `cSelfCheck.cpp:16` `EXPECTED_MOTION_ACTION_COUNT = 7`；`:57` 不符即 `AllOk=false`；`ht160s.cpp:259-266` 收到 false 直接 `return 0`——**應用程式拒絕開機**。加 action 必須同 commit 改這個常數 |
| `FindFeedAuto()` 有副作用 | 🔴 確認 | `:451-452` 先把**全部六站**的 `State[*].bRearCanUse=false`，再於 `:457` 對候選站設 true。它**不是**純判定函式，per-station 化必須換成非變異述詞 |
| 整模組 housekeeping | 🔴 確認 | `CheckAutoTray()`（`:428`）與 `ServiceCarFull()`（`:1613`）都是 `for(0..5)` 全站迴圈；六條梯形圖各自呼叫會變成一個 cycle 跑 6 次 |
| 現場 IO 表與 repo 分歧 | 🔴 確認（獨立問題） | 見 §6 |

---

## 4. 選定設計

### P1（本次執行）：參數化 + per-station 游標，派工維持序列

```
bool DoFeedTray(int Index, int Flag);        // 原 DoFeedTray(int Flag)
bool DoDischargeTray(int Index, int Flag);   // 原 DoDischargeTray(int Flag)
```
- `iFeedAuto` / `iDischargeAuto` **刪除**，改為參數 `Index`。
- `FeedTask` / `FeedDelay` / `DischargeTask` / `DischargeDelay` / `DischargeSubTask`
  移入 `TAutoStationState`。
- `CleanOutTask` / `CleanOutDelay` / `bCleanOutCheck[6]` / `TestUpTask` / `TestDelay`
  **維持模組層級**——`DoAllAutoCleanOut` 本質是六站 lockstep 編排（約 200 行、自帶
  `AreAllFlagsOn` 收斂模型），**刻意不拆**。
- `DoAuto` 仍用單一 Tag，case 1000/3000 仍各挑一站——**所以行為位元不變**，
  只是 `Car[Index]` 從此天生正確。
- `FindFeedAuto()` 拆成：非變異的 `IsFeedEligible(Index)` + 保留原副作用的
  `RefreshRearCanUse()`（在 `CheckAutoTray` 之後呼叫一次）。

### P2（本次**不做**，待現場數據）：併行派工

若日後要做，形狀是 SHAPE A（六個 `actAuto1..6` + 一個模組 action）。**執行前必須先有**：
1. §5 全部危害的防禦
2. 現場「同時符合條件站數」直方圖證明真的有併行機會
3. `cSelfCheck.cpp` 常數同 commit 更新
4. `General.ini [Auto] Concurrency`，**0 = 維持現行序列梯形圖**當一鍵回退

⚠ finder 原建議的 `MaxConcurrentFeed=1 + MaxConcurrentDischarge=1` 當「等同今天」的預設
**是錯的**：今天 feed 與 discharge 是同一條梯形圖的兩個相位、時間互斥；
那組預設會允許「一個 feed + 一個 discharge 同時跑」，**上線第一天就是行為改變**。

---

## 5. 危害清單（P2 才會觸發，先記錄）

| 危害 | 失效模式 | 可偵測？ | 防禦 | 風險 |
|---|---|---|---|---|
| `Car[]` 蓋章 | 兩站併行 feed 蓋錯車的 Kind/TrayID/CarID/iTrayCount | ❌ 無執行期偵測 | **P1 的參數化即根治** | blocker |
| SECS DeviceCount | 上一條經 CEID 272/273/274 Report 6 上到 host | ❌ | 同上 | high |
| CleanOut 入口競態 | 站1 進 CleanOut 分支時站2-6 可能正在 feed/discharge，`DoAllAutoCleanOut` 會對正在轉移中的站下衝突 Y 目標並鬆夾 | ✅ 會撞 | Run_CleanOut 期間六個站 action 全部關閉 | blocker |
| HOME 尾單只 latch 一個 | `HomeDrainTick` 看單一 `iDischargeAuto`/`DischargeTask`；併行下最多六站在 5000-6100 eject 尾，**會漏掉五個**——已提交的盤鬆夾、Y 未退、FrontRise 未泵 | ❌ | 尾單改 per-station 陣列 | blocker |
| SortArm 共用 MAutoY | `TSortArmModule` 有自己的 `MoveAutoY`，與 Auto 模組**無所有權交握**；今天最多兩支 Y 同時被下令，併行後最多七支 | ❌ | 需要真正的 per-axis 所有權 | high |
| Modal 凍結 | 站3 跳 modal 時其餘五站梯形圖凍結但馬達仍在動 | 部分 | 每個 `ShowMyError` 前後暫停 Auto 計時器 | high |
| 警報站別身分 | 警報碼由共用游標組成（`sprintf("JAM%d02", 11+iFeedAuto)`, `:665`） | ✅ | P1 參數化即修正 | high |
| Action 順序 | UserMotion 的**順序決定 scan 內執行序**，程式明確假設 Auto 在 TrayArm **之前**；把 actAuto2..6 附加在 Color 後面會把五站放到邊界錯邊 | ❌ | 六個 action 必須插在原位置 | medium |

---

## 6. 順帶查到的獨立問題：現場 IO 表與 repo 基準分歧

**現場 `IO_Table.csv` 有 27 列 `RearRiseTray`，repo 基準 `system/IO_Table.csv` 一列都沒有。**
程式端零消費者（`aColor.cpp:738` 註明「the whole rear-riser family」已被移除）。

因為 cylinder 是**按名稱綁定**、`CYLINDER_MODULAR` 沒有 RearRiseTray 成員，
這些列**從未被綁定** → **今天沒有實際串音**。但有兩處輸出位址與**生產中的汽缸重疊**：

| 位址 | 衝突的兩者 |
|---|---|
| `Lane0 IP8 P2 B5` | `C_Loader_RearRiseTray` ↔ **`C_Auto2_FrontRiseTray`** |
| `Lane0 IP8 P3 B1` | `C_Auto1_RearRiseTray` ↔ **`C_Auto2_PushTray`** |

`C_Auto2_PushTray` 正是 07-31 16:31:40 觸發 JAM1202 的那支。
另外 `C_Auto6_RearRiseTray` 的 IP 欄位是 **`W`**（非數字，格式錯誤）。

**要問現場工程師**：這些 rear-riser 硬體實際存在並接線嗎？還是當初臆測填的？
在回答之前，**不可假設現場 IO 圖等同 repo 基準**。

---

## 7. 分階段 commit 與驗證

| 階段 | 內容 | 建置閘 | 「沒有改變行為」的證明 |
|---|---|---|---|
| **P1**（本次） | 參數化 + per-station 游標，派工不動 | `-Clean` + `-Full`(關 SOFT_SIMULATE) + 編碼檢查 | sim 跑 5 分鐘，`TaskHistory.csv` 的 Auto Task 序列與改前**逐筆相同** |
| P1b | 每站 feed/discharge 進出時間戳 + 「同時符合條件站數」計數 | 同上 | 純新增 log，無行為改變 |
| P2a | 六個 action + `cSelfCheck` 常數 + `Concurrency=0` 預設 | 同上 | `Concurrency=0` 時 `TaskHistory` 與 P1 相同 |
| P2b | 開併行 | 上機 | 只能上機驗 |

**模擬可驗**：編譯、Task 序列不變、`Car[]` 蓋章正確性（sim 下強制兩站 feed）。
**只能上機驗**：氣壓餘裕、MC88X1 單板多軸同動、SortArm/Auto 共軸實際互斥。

---

## 8. 回退

P1 無需回退（行為不變）。P2 的回退是 `General.ini [Auto] Concurrency=0`，
現場改一個值 + 重啟，不需要韌體回退。

---

## 9. 待外部確認

1. **現場 rear-riser 硬體是否存在**（§6）——問現場工程師。
2. **氣壓餘裕**：六個 Lean + 六個 Push + 六個 FrontRise 在同一個 50 ms tick 內。
   軟體無法回答，需 ME 提供缸徑、行程、manifold Cv、調壓器容量。
   現有 `SnAirIsEnough` 互鎖把失效模式限縮成「停機擾動」而非撞機，
   所以 1→2→3 分段放大是 ME 無法回答時的合法替代驗證法。
3. **MC88X1 單板同動軸數上限**：board 0 上已有 8 軸（含 MAutoY_1..3 + SortArmX + TrayArmX
   + EmptyY + LoaderY_1/2）。CleanOut 證明短脈衝六軸可行，但**持續生產併行無前例**。
   需要 MC88X1 程式手冊的每板同動軸數規格。
4. **模組 action 的 Caption**：目前 `actAuto1to6` 的 Caption 寫死是 `'Auto1'`
   （這就是快照那列叫 Auto1 的原因）。P2 若加六個站 action，模組 action **不能**還叫 `'Auto1'`，
   否則 `MachineState.ini [Tasks]` 會出現重複 key。

---

## 10. 未複驗

本輪 117 條 finder 發現的**對抗式複驗全數未執行**（session 額度）。
本文採信的每一條都經我本人重新開檔覆核（§3 表格）；其餘僅供 P2 立案時的起點，
**實作前必須逐條重驗**。原始發現保存於工作區 journal
`wf_0a6adbc6-391/journal.jsonl`。
