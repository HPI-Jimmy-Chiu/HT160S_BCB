# Auto1~6 Clean Out Finish 述詞修復計畫（即時運算 + Sensor 複查 + SortArm 依賴）

- 日期：2026-07-06
- 分支：`feat/iosetview-172-refactor`
- 狀態：**設計中（DESIGN ONLY）**，尚未動任何程式碼、未 build
- 依據：live repo 2026-07-06 HEAD（含 commit `b5d9d91`）逐字讀碼 + 2026-07-06 全模組稽核報告
- 前置參照：`docs/plan/cleanout-finish-design-20260703.md` §3 Auto 段、`docs/plan/cleanout-cascade-refactor-plan.md`（Auto 當時標 NO CHANGE，已過期）

---

## 0. 一句話問題陳述

Auto1~6 的 `IsAllCleanOutFinish()`（`aAuto1To6.cpp:936-942`）是**六個模組中唯一的純一次性 latch**：只掃 `State[].bCleanOutFinish`，六站全 true 就回報完成。而該 latch 在 `DoAllAutoCleanOut` case 7000（`aAuto1To6.cpp:923`）**無條件設 true**，且是在**同一個 block 先清掉所有軟體殘料旗標之後**才設，等於「自己剛把帳面清乾淨，再宣告自己乾淨」，**沒有感應器複查、設 true 後也不再即時複算**。SortArm、TrayArm、Empty、Color 都已改成即時運算，只有 Auto 沒跟上。本計畫把 Auto 拉齊到相同水準，並補三件事：**(1) 真機感應器複查、(2) 述詞內即時檢查 SortArm 完成、(3) 避免把「假完成」換成「無聲卡死」的殘料 watchdog**。

---

## 1. 先講清楚：Auto1 現在是怎麼「結束」的（逐 tick 實例）

> 這節專門回答你的第 3 點——之前「只看 State[].bCleanOutFinish 六站全 true」講得太抽象。以下用 Auto1（Index=0）走一遍。

驅動來源：`main` 每輪呼叫 `AutoModule->DoAuto(Task)`（`aAuto1To6.cpp:1398`）。清機期間關鍵是 `CleanOutTask` 這個子狀態機（在 `DoAllAutoCleanOut`, `aAuto1To6.cpp:730`）。

**現況流程（Normal → Clean Out → 完成）：**

| 階段 | 發生什麼 | file:line |
|---|---|---|
| A | 操作員按 Clean Out：`sbCleanOut1Click` 只設 `bCleanOut=true` + `ChangeRunMode(Run_CleanOut)`，**不 reset 任何模組** | `main.cpp:1600-1609` |
| B | `DoAuto` case 100：**只有當 `SortArmModule->IsCleanOutFinish()` 為真**才進清機——`DoAllAutoCleanOut(0)`（把 `CleanOutTask=1`）+ `Task=5000` | `aAuto1To6.cpp:1409-1416` |
| C | `DoAuto` case 5000 每 tick 呼叫 `DoAllAutoCleanOut(1)`，跑子狀態機：`1→500→600`（把 TrayArm late-deliver 到後方、還沒收進去的盤，用 `DoFeedTray` 拉進 working）`→100→1000`（Y 移到 discharge 位）`→2000`（Push 缸 Pop）`→3000`（Lean 缸 Pop）`→4000`（**Full 閘**：真機若輸出堆疊滿→modal 逼操作員清空；然後 FrontRise 缸 On 把整盤頂上車）`→5000`（dwell 後 FrontRise Off）`→6000`（Y 回 feed 位）`→7000` | `aAuto1To6.cpp:744-931` |
| D | case 7000：先 `FindFeedAuto()>=0` 再確認沒有漏收的後方盤（有就跳回 500 再收一次）；接著 **6 站迴圈**：把 `bCarHasTray/bRearHasTray/bRearCanUse/bRearDeliveredPending/bFrontHasTray/bFullIC` 全清 false、`RearGrid.Clear()`、`GetAutoVMotor(i)->ClearTray()`（清虛擬盤 `fHasTray`），**最後 `State[i].bCleanOutFinish=true`**、`Status=AS_CLEANOUT_DONE`；`return true` | `aAuto1To6.cpp:902-931` |
| E | `DoAllAutoCleanOut(1)` 回 true → `DoAuto` case 5000 設 `Task=1` | `aAuto1To6.cpp:1453-1455` |
| F | 下一 tick `DoAuto` 最上面：`if(RunMode==Run_CleanOut && IsAllCleanOutFinish()) return;`——此時六站 latch 都 true → **直接 return，ladder 停住不再跑**。這就是 latch 的實際用途：當「別再抽了」的煞車 | `aAuto1To6.cpp:1400-1401` |
| G | 上層 `csystem CheckCleanOutFinish()` AND 進 `AutoModule->IsAllCleanOutFinish()`（＝六站 latch）→ 連同其他模組都 true → CleanOut 收斂，`InitialAllTask()` 全機 reset、回 `Run_Normal` | `csystem.cpp:1399-1421`；`aTrayArm.cpp:226`（TrayArm 也 AND 進 Auto） |

**現況的兩個要害（用這個實例點出來）：**

- **要害 1（stale-true / 無感應器）**：階段 D 是「先清帳面、再宣告完成」。如果 case 4000 的 FrontRise 缸動作了但**盤實際沒頂上車（卡住 / 缸到位但盤沒走）**，`SnAuto1_OutputBottomHasTray` / `SnAuto1_InputHasTray` 實體上還亮，但 case 7000 照樣把 `bRearHasTray=false` 然後 `bCleanOutFinish=true`。`IsAllCleanOutFinish()` 只讀 latch，**永遠看不到那片實體殘盤** → 整機宣告清機完成、停機，盤還在 Auto1 上。這正是六模組稽核裡把 Auto 列為最高風險的原因。
- **要害 2（SortArm 只在入口 gate，不在述詞複算）**：階段 B 的 SortArm 完成檢查只發生在「要不要**開始**抽」的當下（case 100）。一旦 ladder 開始跑並在 case 7000 latch 完成後，`IsAllCleanOutFinish()` **再也不回頭看 SortArm**。若 SortArm 因晚到的 IC / 殘料重吸而重新變成「未完成」，Auto 仍回報完成 → cascade 判定不一致。這與 SortArm 自己（即時查 Loader）、TrayArm（即時查 Loader+Auto）的作法不對稱。

---

## 2. 修復目標（對應你的三點）

1. **Sensor 複查**：`IsAllCleanOutFinish()` 在真機模式下，逐站確認 `SnAutoX_InputHasTray`、`SnAutoX_InputFullTray`、`SnAutoX_OutputBottomHasTray` 皆 OFF（Enable-gated），鏡射 `RefreshAutoState()`（`aAuto1To6.cpp:315-362`）既有的 sensor→flag idiom，不自創新讀法。
2. **述詞內含 SortArm 完成**：`IsAllCleanOutFinish()` 直接 AND 進 `SortArmModule->IsCleanOutFinish()`（即時），與 SortArm/TrayArm 的 cascade 寫法一致。`SortArmModule` 指標在本檔已可用（case 100 line 1411 已在用），**不需新 include**。
3. **改為即時運算，但不製造無聲卡死**：把純 latch 述詞改成即時重算（盤中途再出現 → 自動取消完成），同時補一個**有界殘料 watchdog**：ladder 已跑完但實體殘盤逾時未清 → 指名該站彈 Note（silent-stop-must-notify 合規），而非無限重抽或無聲停機。

**cascade 無環確認**：SortArm→Loader；Auto→SortArm→Loader；TrayArm→(Loader, Auto)；Empty/Color→TrayArm。加「Auto→SortArm」後仍是 DAG（Auto 不被 SortArm/Loader 反向依賴），無循環等待。

---

## 3. 核心設計決策：拆分「煞車 latch」與「對外完成述詞」

`IsAllCleanOutFinish()` 目前**身兼兩職**，而這兩職要的語意相反，是所有複雜度的根源：

- **職責 A（DoAuto 內部煞車，line 1400）**：要的是「抽料 ladder 有沒有跑到 case 7000」。應該是**純 latch**——一旦 latch 就停，不因感應器抖動重抽。
- **職責 B（cascade 對外完成，csystem / TrayArm）**：要的是「Auto 真的空了 + 上游仍完成」。應該是**即時運算**。

若硬把單一函式改成即時：職責 A 會被連累——感應器一亮，line 1400 不再 early-return → case 100 見 SortArm 仍完成 → `DoAllAutoCleanOut(0)` 重置 → **整條抽料 ladder 從頭重跑**，對著一片卡住的盤無限 re-GoUp。這是要避免的 thrash。

**因此拆成兩個函式：**

```
// 私有：純 latch 迴圈（＝現在 IsAllCleanOutFinish 的 body）。DoAuto 煞車用這個。
bool TAutoModule::AllStationsDrainLatched()
{
    for(int i=0;i<AUTO_STATION_COUNT;i++)
        if(State[i].bCleanOutFinish==false) return false;
    return true;
}

// 公有：對外 cascade 用的即時述詞（重寫）。
bool TAutoModule::IsAllCleanOutFinish() { ...見 §4... }
```

- `DoAuto` line 1400 的煞車改用 `AllStationsDrainLatched()`（行為與今日 100% 相同，抽料跑完就停，零 thrash）。
- `csystem` / `TrayArm` 走公有 `IsAllCleanOutFinish()`（即時、含 SortArm、含 sensor 複查）。

---

## 4. 新版 `IsAllCleanOutFinish()` 述詞（設計；行號待實作時定位）

順序刻意由「便宜/上游」到「逐站硬體」排列，短路成本最低。全程鏡射 Empty（`aEmpty.cpp:936-971`）與 Color（`aColor.cpp:1185-1224`）的既有結構。

```
bool TAutoModule::IsAllCleanOutFinish()
{
    // (0) 非 CleanOut：此值在 cascade 外不被消費;比照 SortArm/TrayArm 回傳 latch 即可
    if(HSys.Sys.RunMode!=Run_CleanOut)
        return AllStationsDrainLatched();

    // (1) 上游 SortArm 即時完成 —— 你的第 2 點
    if(SortArmModule==NULL || SortArmModule->IsCleanOutFinish()==false)
        return false;

    // (2) 抽料 ladder 確實跑到 case 7000（六站 latch）—— 保留「ladder 真的執行過」的證據
    if(AllStationsDrainLatched()==false)
        return false;

    // (3) 模組層 idle gate。注意:CleanOutTask 完成後停在 7000(見 §5 陷阱),
    //     故此處只 gate Feed/Discharge,不 gate CleanOutTask。
    if(FeedTask!=1 || DischargeTask!=1)
        return false;

    // (4) 逐站即時複查
    for(int i=0;i<AUTO_STATION_COUNT;i++)
    {
        // (4a) 軟體/虛擬盤 —— sim 與真機都查(比照 Loader bRearHasTray 那條不分模式)
        if(State[i].bRearHasTray || State[i].bRearCanUse || bRearDeliveredPending[i])
            return false;
        if(State[i].bCarHasTray || State[i].bFrontHasTray || State[i].bFullIC)
            return false;
        TTrayMotor *VMot=GetAutoVMotor(i);
        if(VMot!=NULL && VMot->fHasTray)
            return false;

        // (4b) 真機感應器複查 —— 你的第 1 點。sim 早退(比照 RefreshAutoState 324-325),
        //      否則 laptop 上 InType=0 假 present 會令清機永不完成。
        if(IsSoftSimulate()==false)
        {
            TMySensor *Front=GetInputHasTray(i);
            if(Front!=NULL && Front->Enable && Front->IsOn())   return false;
            TMySensor *Full=GetInputFullTray(i);
            if(Full!=NULL && Full->Enable && Full->IsOn())       return false;
            TMySensor *Rear=GetOutputBottomHasTray(i);
            if(Rear!=NULL && Rear->Enable && Rear->IsOn())       return false;
        }
    }
    return true;
}
```

**為何(4a)也在 sim 查**：case 7000 在 sim 下已把這些軟體旗標清為 false，所以 sim 通過；但若日後有旗標在 case 7000 之後被重設(late deliver 的 `bRearDeliveredPending`)，sim 也能即時反映，維持「盤再出現→取消完成」語意。這與 Loader 的 `bRearHasTray` 不分模式檢查(`aLoader.cpp:760`)一致。

**Full 閘（選配，預設納入以對齊 Empty/Color）**：可再加
`if(IsSoftSimulate()==false && IsOutputCarFullForAmr(i)) return false;`——但注意 case 4000 已在抽料當下處理 Full modal，此處重覆多屬防禦性；納入與否列為 §7 待確認 Q3。

---

## 5. 實作陷阱（務必寫進 code comment，避免下一個 session 踩雷）

1. **`CleanOutTask` 完成後停在 7000，不會回 1**：case 7000 只 `return true`，不改 `CleanOutTask`。之後 `DoAuto` case 5000 設的是 `Task`（外層），不是 `CleanOutTask`（內層）。故述詞的 idle gate **不可**寫 `CleanOutTask==1`（會永遠 false → 整機永不完成）。§4 (3) 只 gate `FeedTask/DischargeTask`。這是本修復最容易錯的一點。
2. **拆分後 line 1400 一定要改用 `AllStationsDrainLatched()`**：若忘記、仍用公有即時述詞，感應器一亮就整條 ladder 重抽（thrash）。
3. **sim 早退位置**：sensor 複查(4b)包在 `IsSoftSimulate()==false` 內即可；不要把整個 (4) 包進去，否則 sim 會漏掉軟體旗標的即時性。
4. **不要在述詞裡呼叫 `RefreshAutoState()`**：Empty/Color 有呼叫 `RefreshStateFromSensors()`，但 Auto 的 `RefreshAutoState()` 會**寫回** `State[].bXxx`（有副作用，且含 delivered-pending latch 邏輯）。在唯讀述詞裡呼叫會在 dump/查詢路徑刷狀態。**直接讀 sensor 的 `IsOn()`**（如 §4 (4b)），比照 Loader 述詞不呼叫 refresh 只讀 raw sensor 的作法(`aLoader.cpp:751-758`)。

---

## 6. 殘料 Watchdog（防止「假完成」變成「無聲卡死」）

即時述詞的代價：ladder 已 latch 完成、但某站實體殘盤(卡住/感應器卡死)令 (4b) 永遠 false → `IsAllCleanOutFinish()` 永遠 false → cascade 永不收斂、機器停在那不動、**零告警**。這違反 `silent-stop-must-notify`，且正是現場最容易「誤判機器壞了」的情境。cascade 文件當初把此列為 DEFERRED watchdog，本計畫一併補上。

**設計（鏡射 Loader MES0924 rear-leftover + Auto 既有 `AmrFullWaitTimer` 模式）：**

- 位置：`DoAuto` 最上面的煞車分支——當 `RunMode==Run_CleanOut && AllStationsDrainLatched()==true`（抽料已完成、ladder 已停）時，不要只 `return`，先跑 watchdog：
  ```
  if(HSys.Sys.RunMode==Run_CleanOut && AllStationsDrainLatched())
  {
      ServiceCleanOutResidualWatchdog();   // 新增
      return;
  }
  ```
- `ServiceCleanOutResidualWatchdog()`：真機模式下逐站跑 §4 (4b) 的 sensor 複查；發現殘料就 arm 一個計時器(新 `HTimer CleanOutResidualTimer` 或複用既有 per-station timer)，逾時 `iAutoCleanOutResidualSec`(新 `GeneralSetting` 欄位，鏡射 `iAmrFullWaitSec` 的宣告 `GeneralSetting.h:112` + 讀檔/存檔/預設) → 指名該站彈 Note，例如
  `ShowMyError("MESxxxx", "AutoN clean-out residual tray - remove it", Sensor, false, K_RETRY)`；操作員移除、感應器 OFF 後 (4b) 通過、述詞轉 true、cascade 收斂。
- **新告警碼**：需在 `database.cpp` 的 `CreateSystemAlarmCode` SeedCode 表註冊(比照 MES0922/MES0923/MES0924 的登錄方式)。碼號待定(避開既用的 MES11xx~16xx per-station Full 區段)——列為 §7 Q2。
- sim 不 arm(sensor 複查早退)，laptop 清機照常瞬收。

**替代（較輕量）**：若不想新增 timer/告警碼，最小版本可只在 watchdog 命中時走既有 `LogLadderFault`/EventLog 記一筆 + 沿用某個通用 Note。但那樣操作員現場仍只看到「停著不動」，信任修復效果差。**建議採計時 Note 版**（列 §7 Q1）。

---

## 7. 待你確認（實作前）

1. **Watchdog 採「計時 Note」完整版，還是先只記 EventLog 的輕量版？** 建議完整版（現場可操作、符合 silent-stop-must-notify）。
2. **殘料 Note 的 MES 碼號**：Auto per-station Full 已用 `MES1120~1620`(`MES%d20`) 與 `MES1125~1625`(`MES%d25`)。殘料 watchdog 想用哪個區段？（建議新開一段，如 `MES%d27` → MES1127~1627，逐站可辨識。）
3. **§4 的 Full 閘 (4c) 是否納入述詞？** case 4000 抽料當下已處理 Full modal，述詞再查屬防禦性重覆。納入＝與 Empty/Color 完全對稱；不納入＝更精簡。傾向納入以求一致，聽你決定。
4. **要不要順手把 Auto 納入 State Record 的 DescribeState 觀測**（印出「AllStationsDrainLatched / 逐站 sensor / 計算後 verdict」）？稽核報告指出撞機模組往往儀器最少；建議與本修復同車，但可拆成獨立小 commit。

---

## 8. 實作切片與 build gate（實作階段才執行）

- **Slice 1**：`aAuto1To6.h` 宣告 `AllStationsDrainLatched()`（private）；`aAuto1To6.cpp` 實作它 + 重寫 `IsAllCleanOutFinish()`（§4）；`DoAuto` line 1400 改用 `AllStationsDrainLatched()`。刪改動到的 `.obj` → `scripts/ops/build-ht160s.ps1 -Clean`，EXIT 0。
- **Slice 2**：watchdog（§6）+ `GeneralSetting` 新欄位 + `database.cpp` 註冊告警碼。編碼安全(Big5)：改動區皆 ASCII C++/英文註解，仍以 `scripts/ops/check-ht160s-source-encoding.ps1` 驗。
- **真機 build gate**：`MachineType.h` 註解掉 `#define SOFT_SIMULATE` → `-Full` → 期望 EXIT 0（確認 sensor 複查與 watchdog 的 `#ifndef SOFT_SIMULATE`/`IsSoftSimulate()` 分支能編譯）→ **還原 define** → 重建。此修復觸及 sim/real 分歧，真機 gate 為必跑。
- 兩組 build 皆 EXIT 0 前，不得聲稱 build-clean（BCB6 由使用者端跑）。

## 9. 上機驗證清單（交付後）

1. **正常清機（sim, AMR=0）**：取消勾 "Load New Tray"（模擬供料車抽乾）→ 全機清空回 Normal，Auto 六站無殘盤、無誤停。
2. **真機殘盤**：人為讓 Auto1 一片盤 GoUp 失敗（或卡住 `SnAuto1_OutputBottomHasTray`）→ 期望：整機**不**宣告完成、逾 `iAutoCleanOutResidualSec` 後彈指名 Auto1 的 Note；移除後自動收斂。（此步驗證要害 1 已修）
3. **SortArm 晚到 IC**：清機中 SortArm 因殘料重吸暫回未完成 → 期望 `IsAllCleanOutFinish()` 即時轉 false、cascade 不誤收（驗證要害 2 已修）。
4. **thrash 檢查**：真機殘盤情境下觀察 Auto ladder **不**反覆重抽（`CleanOutTask` 不從 7000 跳回 500）——確認 line 1400 用的是 latch 版煞車。
5. **State Record**（若採 §7 Q4）：FeederDecision/CurrentTasks 能看到逐站 sensor 與 verdict，殘盤事故一翻兩瞪眼。

---

**誠實度聲明**：本計畫所有現況描述(§1 表格、§4 引用的既有 idiom)均出自 2026-07-06 對 live repo 的直接讀碼，行號已核對；新函式/告警碼/GeneralSetting 欄位為**設計提案**，實際行號與碼號於實作時定位並回填。最大 caveat：即時述詞 + watchdog 全屬新碼，零實機時數，以 §9 上機結果為最終仲裁。

---

## 10. 決策鎖定 + 據此調整（2026-07-06，使用者確認後）

**使用者拍板：**
1. **Watchdog = EventLog-only 輕量版**：不彈 modal Note、不用計時器、不加 `GeneralSetting` 欄位。改用 `g_EventLog.Log(code, msg, part)`（`cEventLog.h:17`）+ **per-episode log-once latch** `bCleanOutResidualLogged[6]`（`InitialFlag` 歸零）防洗版。→ 原 §6 的計時 Note 版與 `iAutoCleanOutResidualSec` 欄位**取消**。
2. **MES 碼段 = `MES%d27` → MES1127~1627**（11+Index，逐站可辨識，對齊既有 `MES%d20`/`MES%d25`）。因走 EventLog-only（非 `ShowMyError` modal），**不需在 `database.cpp CreateSystemAlarmCode` 註冊**——碼號僅作為 EventLog 的 tag。
3. **§4 Full 閘 (4c) 納入**：真機模式逐站 `IsOutputCarFullForAmr(Index)` 為真即擋，與 Empty/Color 完全對稱。
4. **Auto 納入 State Record DescribeState 觀測**：擴充既有 `DescribeStation()`（`aAuto1To6.cpp:1226`，已被 `cStateRecordHT160.cpp:713` 逐站呼叫，**不需改 cStateRecordHT160.cpp**）多印一行 CleanOut 診斷：`DrainLatch / FrontSn / RearSn / ResidualLogged / BlocksFinish(計算後 verdict)`。

**實作中發現、據此修正計畫的一點（重要）：**
- **移除 §4 的 `FeedTask==1 && DischargeTask==1` idle gate**。讀 `DoFeedTray`（`aAuto1To6.cpp:584-629`）確認 case 7000 完成時 **`return true` 但不重置 `FeedTask=1`**；`FeedTask` 只在 `DoFeedTray(0)`（Flag==0，下一輪開頭，`:461-467`）歸 1。清機 drain 的 rear-collect（`DoAllAutoCleanOut` case 500 先 `DoFeedTray(0)` 再 600 `DoFeedTray(1)`）跑完後，`FeedTask` 停在 100（無漏收盤時 case 100 直接 return true）或 7000（有收盤時），**永遠不是 1**。若沿用原 idle gate 會令述詞永久 false、整機清機永不完成。→ **Auto 不採 FeedTask/DischargeTask/CleanOutTask idle gate**；`AllStationsDrainLatched()`（CleanOutTask 已達 case 7000＝所有 Y 移動＋汽缸循環都完成）就是 Auto 正確且足夠的「drain 完整跑完」證據。此為 Empty/Color 的 idle gate 概念**不可平移到 Auto** 的具體原因，已寫進述詞 code comment。

**鎖定後的最終述詞條件（取代 §4 清單）：**
```
IsAllCleanOutFinish() (RunMode==Run_CleanOut) =
   SortArmModule->IsCleanOutFinish()            // (1) 上游即時
&& AllStationsDrainLatched()                    // (2) 六站 drain latch(case7000)
&& 逐站: !bRearHasTray && !bRearCanUse && !bRearDeliveredPending
       && !bCarHasTray && !bFrontHasTray && !bFullIC
       && GetAutoVMotor(i)->fHasTray==false     // (3) 軟體/虛擬盤(sim+real)
&& 逐站(IsSoftSimulate()==false):               // (4) 真機 sensor 複查 + (5) Full
       InputHasTray OFF && InputFullTray OFF && OutputBottomHasTray OFF
       && IsOutputCarFullForAmr(i)==false
```
`DoAuto` 煞車（`:1400`）改用 `AllStationsDrainLatched()`（純 latch，零 thrash），並在該分支呼叫 `ServiceCleanOutResidualWatchdog()`。

**受影響檔案（最終）**：`aAuto1To6.h`（+1 欄位 +2 私有宣告）、`aAuto1To6.cpp`（AllStationsDrainLatched / 重寫 IsAllCleanOutFinish / ServiceCleanOutResidualWatchdog / DoAuto 煞車 / InitialFlag 歸零 / DescribeStation 觀測 / 兩處 late-delivery self-heal）、`database.cpp`（註冊 Auto residual family 第 6 組）。**不動** `GeneralSetting.*`、`cStateRecordHT160.cpp`。編輯以 byte-safe latin1 splice（檔案 Big5、EOL=LF）。

---

## 11. 實作完成 + 對抗審查修正（2026-07-06）

**實作 + build gate 完成**：sim `-Clean`、真機 `-Full`（SOFT_SIMULATE off）、還原後 `-Full` 三者皆 EXIT 0；編碼檢查通過 160 檔；MachineType.h byte-identical 還原。**未 commit**（worktree 內有他 session 既有改動）。

**3-lens 對抗審查（false-BLOCK / false-ALLOW / thrash+cascade）裁決**：原始 false-finish bug 確認關閉、sim 仍完成、cascade 無環、stop-gate 不 thrash、predicate 無副作用 — 均 cleared。發現並**已修**兩項：

- **[P1 已修] MES 碼撞號**：`MES%d27` 在 index=3 → **MES1427**，與 `database.cpp:957` 既有 Color seed「Color supply stack full」撞號（消費端 code→name 會誤標 Auto4 殘留為 Color 滿料），其餘 MES1127/1227/1327/1527/1627 未註冊。**修法**：watchdog 後綴改 `MES%d23`（MES1123~1623，全樹 grep 確認 6 前綴皆空），並在 `database.cpp` Auto family 生成器加第 6 組 `FamFmt[6]`（`MES%d23` = "Auto clean-out residual tray after drain"），使 6 碼皆解析為 per-station 正確名稱。仍為 EventLog-only 通知（決策 1 不變）。
- **[MEDIUM 已修] 遲到交盤 wedge + rear-sensor-停用靜默卡死**：Auto 六站於 case 7000 latch 後，因 module pump 順序 Auto 早於 TrayArm（`database.cpp:1828` vs `:1845`），TrayArm 可能在**同/下一 tick** 才把盤放到某站後方（`SetRearHasTrayFromTrayArm`/`NotifyTrayArmDelivered` 設 `bRearHasTray/bRearDeliveredPending`）→ live predicate 永久卡（`aAuto1To6.cpp:976`），但 `DoAuto` 已對純 latch 短路、case-7000 re-collect 不再跑 → 永久 hang；rear sensor 停用時連 watchdog 都不記（真靜默）。**修法**：在上述兩個交盤進入點內，若 `RunMode==Run_CleanOut` 則清該站 `bCleanOutFinish`（+ re-arm `bCleanOutResidualLogged`），使 `AllStationsDrainLatched()` 轉 false → `DoAuto` 解除短路 → drain ladder 重跑 → case 500/600 re-collect 把遲到盤拉進 working→GoUp 出車→重 latch（自癒）。此修法不依賴任何實體 sensor，故**同時關閉 rear-sensor-停用的靜默 hang 面**。有界（`GetTrayRequest` 於 SortArm 完成後停派新盤 + TrayArm 手上至多一盤）。

**已知殘留限制（未修，交付說明須載明）**：`Enable==false` 的 sensor 是盲的——若某 Auto 站 sensor 被**刻意停用**、其上有**未被任何軟體旗標追蹤**的實體盤（非交接來源），predicate 的實體複查與 watchdog 皆看不到 → 仍可能 false-finish。此為**全專案既有** `Enable==true` 慣例（`RefreshAutoState` 亦同），非本次引入。緩解：CleanOut 期間輸出路徑 sensor 應保持 Enabled。severity LOW。
