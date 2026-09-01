# Auto 出料收尾：AMR 叫車 + 操作員警報顯示批號 — 實作規格

- **日期**：2026-09-01
- **狀態**：**CODE-COMPLETE（2026-09-01）** — 已實作、兩種組態建置 EXIT 0、編碼檢查 166 files PASS。
  **上機驗證 pending**（§7 的 T6–T16）。實作與規格的差異見 §10
- **分支**：`feat/iosetview-172-refactor`
- **來源**：2026-08-31 京元上機 State Record（`D:\HT160S_StateRecord\20260831第一台結束的staterecod.zip`，HT160S-01，`UseAMR=1`）
- **裁定人**：使用者（2026-09-01 逐點確認，見 §2）

---

## 0. 這份規格涵蓋什麼

同一個場景（**Auto 出料車該清空了**）的四個面向，因為彼此耦合所以合併：

| Part | 主題 | 模式 |
|---|---|---|
| **A** | Clean Out 收尾叫 AMR 來收車 | AMR=1 |
| **B** | Auto Full 警報顯示該流道批號（`MES1x20`） | 兩種模式 |
| **C** | 「邏輯滿」`MES1x25` 真機分支移除（模擬分支保留） | 真機 |
| **D** | 清機殘留 EventLog（`MES1x23`）帶上批號 | 兩種模式 |

一句話串起來：**AMR 開 → 叫車（A）；AMR 關 → 叫人，而且要告訴人這疊是哪一批（B/D）；模擬專用的數量替代品不該出現在真機（C）。**

---

## 1. 需求

### 1.1 Part A — Clean Out 收尾叫車

現行 AMR 下料叫車的**唯一**觸發是「出料車滿」（真機 `SnAutoX_InputFullTray` ON）。
Clean Out 收尾時出料車通常**沒滿但有盤**，機台不會叫 AMR，只丟一行 EventLog 請操作員搬。

新增觸發（與現有滿車觸發 **OR**）：以 Auto1 為例

```
叫車 = SnAuto1_InputFullTray == ON                    ← 既有：產能觸發（車滿，換車繼續跑）
    OR ( RunMode == Run_CleanOut
      AND Loader  清機完成（L+R 兩側）
      AND SortArm 清機完成
      AND Auto1  清機完成（只看該站自己）
      AND SnAuto1_InputHasTray == ON )                ← 新增：交件觸發（沒滿也要收走）
```

Auto2→P5、Auto3→P6、Auto4→P7、Auto5→P8、Auto6→P9 類推，**六站各自獨立判斷，不連坐**。

### 1.2 Part B — Full 警報顯示批號

客戶需求：AMR 關閉時 Full sensor 亮會跳警報叫人搬料，**警報上要顯示該 Auto 的批號**，讓操作員辨識。

### 1.3 Part C — 移除真機的「邏輯滿」

`MES1x25`（`Car[].iTrayCount >= MAX_TRAY_PER_CAR`）是**當初為模擬設計的**：模擬沒有實體 sensor，
只能用盤數代替。**真機有 sensor，不該再用數量判斷**，也不需要留 EventLog。

### 1.4 Part D — 清機殘留 log 帶批號

`MES1x23`（清機排空跑完但感測器還看得到盤）目前只寫「Auto6 有殘留盤」。
AMR=0 時這行是操作員唯一的線索，要變成「Auto6 有 `NQ8002ZAA1:FAIL` 的殘留盤」。

### 1.5 現場證據

```
2026/08/31,17:19:08.969,,0,,MES1623,"Auto6 clean-out residual tray after drain - remove it","front=1 full=0 rear=0"
```

`front=1 full=0` ＝ `SnAuto6_InputHasTray` ON、`SnAuto6_InputFullTray` OFF。**車沒滿但有盤**，
清機被 hold 住，機台只發一行 log 叫人搬。Part A 要把它變成叫車，Part D 要把它變成「叫人且說清楚是哪一批」。

同日全機 AMR 統計：CEID 272 共 27 次，P4–P9（Auto 下料）**只有 1 次**（11:25:00，機台未 HOME 的閒置狀態，
Tray/Device 皆 0），CEID 273 / 274 對 P4–P9 **全日 0 次**。

---

## 2. 已裁定事項（使用者 2026-09-01）

| # | 議題 | 裁定 |
|---|---|---|
| ① | 是否寫進 `IsOutputCarFullForAmr()` | **不可**。另開新函式，`IsOutputCarFullForAmr()` 一個字不動 |
| ② | Loader 清機完成的定義 | **L+R 兩側都排空** ＝ `LoaderModule->IsAllCleanOutFinish()` |
| ③ | Auto 清機完成的定義 | **只看該站自己**，不看其他五站 |
| ④ | 「Lot End 前」的界線 | **不另設 Lot 閘**（20260901 覆議後定案）。`Run_CleanOut` + 三個 drain-finish + `InputHasTray` 已是**必要且充分**條件；加一個 Lot 檢查擋不掉任何該擋的，只會製造一條「該叫車卻不叫」的靜默路徑，理由見 §4.6 |
| ⑤ | 撤銷分支 | 併進同一個 `bFull` 布林 |
| ⑥ | 前置通知 CEID（35/36/37/148/149/150） | ~~照發~~ → **20260901 覆議：收尾叫車不發，只有「真的滿」才發**。理由：這六個號碼在交付給客戶的工作簿 CEID 頁上白紙黑字寫著「AutoN **車滿**」，收尾叫車時車並不滿，照發等於讓韌體違反自己剛交付的規格；且 0831 log 顯示京元從未以 S2F35 連結這六個號碼（走預設 Report 1＝純時間戳，內容無從分辨），唯一發射的一次（CEID 149）host 也毫無反應。不發不會少給資訊——CEID 272 的 SVID 38219 bitmap 已標明站別。**此改動不需客戶同意**（少送一則 vs 送一則語意錯的） |
| ⑦ | `bUseAMR==0` 時的收尾叫車 | **完全不叫車**，維持 `MES1x23` 通知操作員 |
| ⑧ | 逾時逃生門 | 沿用 `iAgvTimeoutSec` + `TimeoutPending`，**不做靜默放行** |
| ⑨ | Error 流道的 Full 警報 | 顯示 `Lot=(Error lane / reject)`，**英文** |
| ⑩ | 批號顯示的資料來源 | 用 `DescribeAutoBins()`（`批號:PASS/FAIL` 或 `批號:Bin`），不用 `DescribeAutoLot()` |
| ⑪ | 批號顯示的模式限制 | **只限 By Lot+Bin / By Lot+PassFail**；NORMAL / WhiteList 不顯示 |
| ⑫ | 收尾叫車是否設分選模式閘 | **不設**。只有「批號顯示」受模式限制，叫車本身六種情況都做 |
| ⑬ | 盤數 / IC 數 | **AMR=1 才顯示；AMR=0 兩個都不顯示**（見 §3.3 的原因） |
| ⑭ | 語系 | **一律英文**，不包 `LangT()`（`aAuto1To6.cpp` 現有 6 個 `ShowMyError` 全是英文字面） |
| ⑮ | `MES1x25` 邏輯滿 | **真機分支移除**（模擬分支保留），**不留 EventLog** |
| ⑯ | `MES1x23` 清機殘留 log | **帶上批號**（同 §3.2 的顯示規則） |

---

## 3. 現況調查（決定實作方式的四個關鍵事實）

### 3.1 ⚠ 「hold 住清機」的部分**程式裡已經有了**

[`aAuto1To6.cpp:1274-1277`](../../HT160S_Program_BCB_V1.0.0.0/aAuto1To6.cpp) — `TAutoModule::IsAllCleanOutFinish()`：

```cpp
if(IsSoftSimulate()==false)
{
    TMySensor *Front=GetInputHasTray(Index);          // SnAutoX_InputHasTray
    if(Front!=NULL && Front->Enable==true && Front->IsOn())
        return false;   //residual tray at this Auto front feed position
    ...
}
```

這就是裁定 ④ 的行為，且只在 `RunMode == Run_CleanOut` 期間生效（函式開頭 `:1257` 分流）。
往上一層 [`csystem.cpp:2044`](../../HT160S_Program_BCB_V1.0.0.0/csystem.cpp) `CheckCleanOutFinish()` 已串進六模組串聯。

**所以 Part A 缺的只有「叫車」，不是「hold」。**

### 3.2 ⚠ `DescribeAutoBins()` 在 NORMAL 模式**不會回空字串**

[`uAgvStation.cpp:827-872`](../../HT160S_Program_BCB_V1.0.0.0/SecsGem/uAgvStation.cpp)：

| 分選模式 | 回傳 |
|---|---|
| By Lot+PassFail | `"NQ8002ZAA1:PASS"` / `"NQ8002ZAA1:FAIL"`；該站無綁定時 `""` |
| By Lot+Bin | `"NQ8002ZAA1:3"`；該站無綁定時 `""` |
| **NORMAL** | **靜態 Bin 號**：`"3"`、`"ERR"`、`"3,ERR"` ← **不是空字串** |
| WhiteList（overlay） | 同 NORMAL（`IsDynamicBindingMode()` 為 false） |

**因此模式閘必須寫在呼叫端外層**，不能靠 `DescribeAutoBins()` 自己回空——否則 NORMAL 模式會印出
`Lot=3`，把 Bin 號當成批號，比不顯示更糟。

0831 現場實測值（report 2001）：`38235 A2Bin="SIMU_LOT_A:PASS"`、`38236 A3Bin="SIMU_LOT_A:FAIL"`、
`38243 A4Bin="SIMU_LOT_E:FAIL"`。

### 3.3 ⚠ `Car[].iTrayCount` 只在 AMR=1 累加；`iAmrDeviceCount` 兩模式都累加

| 計數器 | 累加點 | AMR 閘 | 歸零點 |
|---|---|---|---|
| `Car[Index].iTrayCount` | [`:861`](../../HT160S_Program_BCB_V1.0.0.0/aAuto1To6.cpp) | ⚠ **有**（`:848 if(GeneralSetting.bUseAMR)`） | `TMyCar::Clear()`（`MyMotor.cpp:275`） |
| `iAmrDeviceCount[Index]` | [`:923`](../../HT160S_Program_BCB_V1.0.0.0/aAuto1To6.cpp) | 無 | `InitAutoCarStack()`（`:1365`） |

**AMR=0 時 `iTrayCount` 恆為 0**，若顯示會變成永遠的 `trays=0`。
裁定 ⑬ 選擇「AMR=0 時兩個數字都不顯示」——**刻意的一致性選擇**：要嘛都給、要嘛都不給，
不給操作員一個真、一個假的半套資訊。（`ICs=` 在 AMR=0 其實是正確的，這一點已知並接受。）

⚠ **不要用 `tRunData.RecordTrayCnt[]` 代替**：那是「本工單累計出盤數」，只在工單開始歸零，
不是「這一疊有幾盤」，拿來顯示會誤導。

### 3.4 `MES1x25` 是模擬替代品被套用到真機

[`aAuto1To6.cpp:1866-1901`](../../HT160S_Program_BCB_V1.0.0.0/aAuto1To6.cpp) `ServiceCarFull()`：

```cpp
bool bLogicalFull=(Car[Index].iTrayCount>=MAX_TRAY_PER_CAR);   // :1866  MAX_TRAY_PER_CAR=100

if(IsSoftSimulate())                                            // :1868  ← 模擬：保留不動
{
    if(bLogicalFull) { Car[Index].Clear(); InitAutoCarStack(Index); }
    continue;
}

TMySensor *FullSensor=GetInputFullTray(Index);
bool bSensorFull=(FullSensor!=NULL && FullSensor->Enable==true && FullSensor->IsOn());

if(bSensorFull)        { ... MES1x20 ... }                      // :1881  ← Part B 改這裡
else if(bLogicalFull)  { ... MES1x25 ... }                      // :1894  ← Part C 移除這裡
```

原設計意圖：**模擬環境沒有實體 sensor，只能用盤數代替**。`:1868` 的模擬分支是正確的實作；
`:1894` 把同一個替代品套到真機上，是不該存在的。

### 3.5 `IsOutputCarFullForAmr()` 為何不能碰（裁定 ①）

[`aAuto1To6.cpp:1563`](../../HT160S_Program_BCB_V1.0.0.0/aAuto1To6.cpp)，共 5 個 caller：

| 行號 | caller | 用途 |
|---|---|---|
| `:1118` | `DoDischargeTray` | 卸盤後是否已滿 |
| `:1283` | `IsAllCleanOutFinish` | 清機完成閘 |
| **`:1425`** | **`GetTrayRequest`** | **擋 TrayArm 送盤** ← 寫進去會讓 Auto 一清完就永遠要不到盤 |
| `:1756` | 狀態傾印 | State Record |
| `uAgvStation.cpp:464` | `PollAndCall` | 叫車 ← **只有這個是我們要改的語意** |

---

## 4. 改動點清單

> 除三處明確標示者外，全部為新增。

### 4.1 共用 helper：流道批號標籤

**新增於 `aAuto1To6.h`（public）**

```cpp
AnsiString DescribeLaneLotForOperator(int Index);   //AI(auto-lane-label) 20260901
```

**新增於 `aAuto1To6.cpp`**

```cpp
//AI(auto-lane-label) 20260901 : the operator-facing "which lot is this stack" label, shared by
// the Full alarm (ServiceCarFull) and the CleanOut residual log (ServiceCleanOutResidualWatchdog).
// Returns "" when there is nothing TRUE to say - the caller then omits the whole field rather
// than printing an empty one.
//
// THE MODE GATE MUST BE HERE, NOT INSIDE DescribeAutoBins(). That function does NOT return an
// empty string in smNormal - it returns the STATIC BIN NUMBER for the lane ("3", "ERR", "3,ERR",
// uAgvStation.cpp:862-871). Printing that behind a "Lot=" label would show a bin number as if it
// were a lot id, which is worse than showing nothing. Owner ruling 20260901 : lane lot is a
// By Lot+Bin / By Lot+PassFail concept only.
//
// smNormal has no lane-level lot binding AT ALL (routing is by bin, several lots may legitimately
// share a lane), so there is no honest answer to give there - not a missing one, an absent one.
AnsiString TAutoModule::DescribeLaneLotForOperator(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return "";
    if(GeneralSetting.IsDynamicBindingMode()==false)
        return "";                                   // smNormal / WhiteList : no lane lot exists
    AnsiString s = AgvCoord.DescribeAutoBins(Index);  // "LOT:PASS" / "LOT:FAIL" / "LOT:<bin>"
    if(s != "")
        return s;
    //AI(auto-lane-label) 20260901 : the Error / overflow lane is NEVER bound - ResolveAuto skips
    // it (CosFunction.cpp:1973) - yet it physically collects 2D-scan-fail ICs plus any overflow
    // good product, so it CAN fill up and alarm. Left blank it looks like a missing value; say
    // what it is instead. On-site 2026-08-31 this lane was Auto1, and its 66040 / 38234 were
    // empty all day for exactly this reason.
    if(Index == LotBinBinding.GetErrorAutoIndex())
        return "(Error lane / reject)";
    return "";                                       // bound to nothing yet : say nothing
}
```

> 相依性檢查：`aAuto1To6.cpp` 已 `#include "SecsGem\uAgvStation.h"`（第 14 行，`AgvCoord`）與
> `#include "CosFunction.h"`（第 16 行，`LotBinBinding` / `BinAreaMap`）與 `GeneralSetting.h`（第 15 行）。
> **不需要新增任何 include。**

### 4.2 Part B — `MES1x20` 加批號與數量（`aAuto1To6.cpp:1881-1893`）

```cpp
if(bSensorFull)
{
    AnsiString ErrorText;
    ErrorText.sprintf("Auto%d output stack FULL (sensor) - remove finished trays", Index+1);
    //AI(auto-lane-label) 20260901 : tell the operator WHICH LOT this stack is, so a machine
    // running several lots into several lanes can be cleared without guessing. Customer
    // request via KYEC 2026-09-01. Empty label -> the field is omitted entirely.
    AnsiString sLot = DescribeLaneLotForOperator(Index);
    if(sLot != "")
        ErrorText = ErrorText + " | Lot=" + sLot;
    //AI(auto-lane-label) 20260901 : counts are AMR-mode-only ON PURPOSE. Car[].iTrayCount is
    // only ever incremented inside the bUseAMR branch at :848, so in Normal mode it would print
    // a permanent "trays=0". Owner ruling : show BOTH numbers or NEITHER - a half-true pair is
    // worse for the operator than no numbers. (iAmrDeviceCount IS valid in both modes; it is
    // suppressed with the tray count for consistency, knowingly.)
    if(GeneralSetting.bUseAMR)
        ErrorText = ErrorText + AnsiString().sprintf("  trays=%d  ICs=%d",
                                                     Car[Index].iTrayCount, iAmrDeviceCount[Index]);
    do
    {
        ShowMyError(AnsiString().sprintf("MES%d20", 11+Index), ErrorText, FullSensor, false, K_RETRY);
        FullSensor=GetInputFullTray(Index);
    }
    while(FullSensor!=NULL && FullSensor->Enable==true && FullSensor->IsOn());
    Car[Index].Clear();
    InitAutoCarStack(Index);
}
```

⚠ **維持 sensor overload `ShowMyError(Code, Msg, pSn, bExpectedOn, KCode)`**。
不要為了塞批號改用 Detail overload——Detail 欄放的是 20260720 為可觀測性刻意加的 IO 診斷
（expect / actual / addr），換掉會把它弄丟。批號接在 Message 後面。

### 4.3 Part C — 移除真機的邏輯滿（`aAuto1To6.cpp:1894-1901`）

```cpp
        //AI(cleanout-amr-collect) 20260901 : REAL-MACHINE LOGICAL-FULL BRANCH REMOVED.
        // The iTrayCount >= MAX_TRAY_PER_CAR test is a SIMULATION SUBSTITUTE : a laptop run has
        // no InputFullTray sensor, so the tray count stands in for it (that is what the
        // IsSoftSimulate() branch at :1868 is, and it STAYS). Applying the same substitute to a
        // real machine that HAS the sensor was never the intent - the sensor is the truth.
        // Owner ruling 20260901 : real machine is sensor-only, and no EventLog line either.
        // MES1125/1225/1325/1425/1525/1625 therefore no longer fire on a real machine; the
        // catalogue rows in system/AlarmList.csv are LEFT IN PLACE (an entry that never fires is
        // harmless; deleting one changes the ALID set the host holds from S5F6).
        // else if(bLogicalFull)
        // {
        //     AnsiString ErrorText;
        //     ErrorText.sprintf("Auto%d output car full (%d trays) - change car then confirm", Index+1, MAX_TRAY_PER_CAR);
        //     ShowMyError(AnsiString().sprintf("MES%d25", 11+Index), ErrorText, K_RETRY);
        //     Car[Index].Clear();
        //     InitAutoCarStack(Index);
        // }
```

`bLogicalFull` 變數**保留**——`:1870` 的模擬分支還在用它。

### 4.4 Part D — `MES1x23` 帶批號（`aAuto1To6.cpp:1313-1318`）

```cpp
        AnsiString Where;
        Where.sprintf("front=%d full=%d rear=%d", bFrontOn?1:0, bFullOn?1:0, bRearOn?1:0);
        //AI(auto-lane-label) 20260901 : with AMR off this line is the operator's ONLY notice that
        // a lane still holds trays after the drain, so name the lot the same way the Full alarm
        // does. Goes in the detail column, next to the sensor triple, so the message string
        // itself stays greppable.
        AnsiString sLot = DescribeLaneLotForOperator(Index);
        if(sLot != "")
            Where = Where + " Lot=" + sLot;
        g_EventLog.Log(AnsiString().sprintf("MES%d23", 11+Index),
                       AnsiString().sprintf("Auto%d clean-out residual tray after drain - remove it", Index+1),
                       Where);
```

### 4.5 Part A — 新增 accessor（`aAuto1To6.h` / `aAuto1To6.cpp`）

```cpp
bool IsStationCleanOutFinish(int Index);   // 該站自己的 bCleanOutFinish（裁定 ③）
bool IsFrontHasTrayForAmr(int Index);      // 該站 SnAutoX_InputHasTray 即時讀值
```

```cpp
//AI(cleanout-amr-collect) 20260901 : per-station CleanOut drain latch, for the AMR collect-call
// gate. Deliberately NOT IsAllCleanOutFinish() - that one is module-wide (all six stations) and
// would make a station that finished early wait for the slowest. Owner ruling 20260901.
bool TAutoModule::IsStationCleanOutFinish(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return false;
    return State[Index].bCleanOutFinish;
}

//AI(cleanout-amr-collect) 20260901 : "this Auto's output stack still holds at least one tray" -
// the SAME sensor IsAllCleanOutFinish() already blocks on (:1274), exposed so the AGV coordinator
// can turn that block into a COLLECT CALL instead of a silent wait. Sim returns false : a laptop
// run has no AMR to answer the call.
bool TAutoModule::IsFrontHasTrayForAmr(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return false;
    if(IsSoftSimulate())
        return false;
    TMySensor *Front=GetInputHasTray(Index);
    return (Front!=NULL && Front->Enable==true && Front->IsOn());
}
```

### 4.6 Part A — 叫車判斷（`uAgvStation.h` / `uAgvStation.cpp`）

```cpp
bool IsCleanOutCollectDueForAmr(int AutoIndex);   //AI(cleanout-amr-collect) 20260901
```

```cpp
//AI(cleanout-amr-collect) 20260901 : the SECOND reason to call the AMR to an Auto - the CLEAN-OUT
// COLLECT. The first reason (IsOutputCarFullForAmr) is a THROUGHPUT trigger: the car filled up
// mid-run, swap it and keep going. This one is a HANDOVER trigger: this lot is done, the
// front-of-line modules are drained, and whatever is on the output car - full or not - must leave
// before Lot End rather than cross into the next lot.
//
// WHY A SEPARATE FUNCTION AND NOT AN EXTRA TERM INSIDE IsOutputCarFullForAmr(): that predicate has
// five callers and one of them (aAuto1To6.cpp:1425 GetTrayRequest) uses it to REFUSE new trays.
// Widening it there would make an Auto stop asking for trays the moment it latched drain-done - a
// different behaviour entirely, and one that would bite outside CleanOut too. Owner ruling 20260901.
//
// The HOLD side ALREADY EXISTS and is deliberately untouched: TAutoModule::IsAllCleanOutFinish
// (aAuto1To6.cpp:1274) already returns false while SnAutoX_InputHasTray is ON, so
// CheckCleanOutFinish (csystem.cpp:2044) already parks the machine in Run_CleanOut until the car
// leaves. What was missing was any code that ASKS for it to leave: today the only notice is the
// EventLog line from ServiceCleanOutResidualWatchdog, i.e. the machine waits for an OPERATOR.
// On-site 2026-08-31 17:19:08 that is exactly what Auto6 did (MES1623, front=1 full=0).
//
// NO SORT-MODE GATE (owner ruling 20260901) : a NORMAL-mode run finishes CleanOut with trays on
// the car just as a By-Lot run does. Only the OPERATOR-FACING LOT LABEL is mode-limited, not the
// call itself.
bool TAgvCoordinator::IsCleanOutCollectDueForAmr(int AutoIndex)
{
    if(GeneralSetting.bUseAMR==false)                            // owner ruling : AMR off -> operator
        return false;
    if(AutoModule==NULL || LoaderModule==NULL || SortArmModule==NULL)
        return false;
    if(HSys.Sys.RunMode!=Run_CleanOut)                           // collect window = the CleanOut drain
        return false;
    //AI(cleanout-amr-collect) 20260901 : THERE IS DELIBERATELY NO "is a lot still open" TEST HERE.
    // It was specified, then removed on review, and it must not be added back - see the block
    // comment below this function for the on-site evidence.
    if(LoaderModule->IsAllCleanOutFinish()==false)               // owner ruling : BOTH sides drained
        return false;
    if(SortArmModule->IsCleanOutFinish()==false)
        return false;
    if(AutoModule->IsStationCleanOutFinish(AutoIndex)==false)     // owner ruling : this station only
        return false;
    return AutoModule->IsFrontHasTrayForAmr(AutoIndex);
}
```

### 4.6.1 ⚠ 為什麼沒有「Lot 還開著」的檢查 —— 不要把它加回來

規格初稿有一個 `IsLotOpenForCollect()`（`LotRegistry.GetLotCount()>0`，退回
`fMain->ActiveLotID()`），對應「在 Lot End 前」這句需求。**覆議後刪除**，理由三條：

**(1) 兩個方向的失敗代價不對稱。**
誤判「Lot 還開著」（其實沒開）→ 車上有盤就叫 AMR 來收 —— 清機本來就要清乾淨，這個行為是對的，無害。
誤判「Lot 已結束」（其實開著）→ **不叫車 → 清機 hold → 等人**，正是本案要修的洞。
一邊無害、一邊讓功能失效，那這個閘的存在就只是多開一條靜默失敗的路。

**(2) 它在現場的常態操作順序下會把自己關掉。** 2026-08-31 EventLog：

```
17:18:16.782  AUTO CleanOut: Loader source dry ...
17:19:16.653  LOT END pressed                      ← 60 秒後
17:35:48.872  AUTO CleanOut: Loader source dry ...
17:36:39.782  LOT END pressed                      ← 51 秒後
```

**Lot End 是在 Clean Out 之後約 1 分鐘按下的**，而 AMR 從叫車到把車拉走要幾分鐘。所以真實序列是：

```
CleanOut → 收尾叫車(CEID 272) → 操作員按 Lot End(約 1 分鐘後) → LotRegistry 清空
        → IsLotOpenForCollect() 變 false → bFull 變 false
        → :474 釋放分支把 AMR 鎖放掉、Handshake 退回 IDLE
        → 車還在、盤還在，但機台再也不叫車 → 清機永遠 hold
```

**這個閘會在最常見的操作順序下失效**，比它想防的問題嚴重得多。

**(3) `fMain->ActiveLotID()` fallback 還違反本 codebase 自己的規則。**
它會退回讀主畫面 `edLotNo`（TEdit 即時文字）。`DescribeAutoLot()` 的註解就明文禁止這種綁法：
*"Deliberately NOT the panel Caption - binding SECS to a VCL control would make the host answer
depend on the form being painted."* 把機構控制條件綁在操作員隨時可打字的輸入框上是雙重錯誤。

**替代的語意保險**：若日後仍想要「不要跨批」的保證，用 `IsAllCleanOutFinish()` 本身即可 ——
它已經是清機的真實邊界，而且它 hold 住的時候本來就代表這批還沒結束。**不需要另外查批號。**

### 4.7 Part A — 唯一修改的既有行（`uAgvStation.cpp:464`）

```cpp
// 現行
bool bFull = AutoModule->IsOutputCarFullForAmr(a) || AmrInject.AutoFull(a);

// 改為
//AI(cleanout-amr-collect) 20260901 : ONE boolean on purpose - the CALL branch below and the
// release branch at :474 must agree, or a collect-call raised on one tick would be revoked by the
// release branch on the next. Adding a separate if() instead of OR-ing here is the bug.
bool bFull = AutoModule->IsOutputCarFullForAmr(a)
          || AmrInject.AutoFull(a)
          || IsCleanOutCollectDueForAmr(a);
```

裁定 ⑤（撤銷分支）與 ⑥（前置通知 CEID）**由這一行自動滿足**——`:471` 的
`Gem->EventReport(1, AutoFullCeid[a])` 在同一個 if 內，`:474` 的釋放分支讀同一個 `bFull`。

### 4.8 Part A — 逾時逃生門（裁定 ⑧）

**不需要新程式碼**。[`uAgvStation.cpp:596`](../../HT160S_Program_BCB_V1.0.0.0/SecsGem/uAgvStation.cpp)
的既有機制對 `AGV_CALLED` 已生效：逾時 latch `TimeoutPending` → csystem 主迴圈彈 WAR0962 → K_RETRY 重叫。
**要驗證這條路徑在 `Run_CleanOut` 下沒被別的 RunMode 閘擋掉**（§6 T7）。

---

## 5. 顯示規則總表

| 分選模式 | 流道 | `Lot=` 欄 | `trays=` / `ICs=` |
|---|---|---|---|
| By Lot+Bin | 一般（已綁定） | `Lot=NQ8002ZAA1:3` | AMR=1 才顯示 |
| By Lot+Bin | 一般（尚未綁定） | 不顯示 | AMR=1 才顯示 |
| By Lot+Bin | Error 流道 | `Lot=(Error lane / reject)` | AMR=1 才顯示 |
| By Lot+PassFail | 一般（已綁定） | `Lot=NQ8002ZAA1:PASS` / `:FAIL` | AMR=1 才顯示 |
| By Lot+PassFail | Error 流道 | `Lot=(Error lane / reject)` | AMR=1 才顯示 |
| NORMAL | 任何 | **不顯示** | AMR=1 才顯示 |
| WhiteList（overlay） | 任何 | **不顯示** | AMR=1 才顯示 |

### 實際訊息長相

```
By Lot+PassFail + AMR=1，Auto2：
Auto2 output stack FULL (sensor) - remove finished trays | Lot=NQ8002ZAA1:PASS  trays=6  ICs=72

By Lot+PassFail + AMR=0，Auto2：
Auto2 output stack FULL (sensor) - remove finished trays | Lot=NQ8002ZAA1:PASS

Error 流道（Auto1）+ AMR=0：
Auto1 output stack FULL (sensor) - remove finished trays | Lot=(Error lane / reject)

NORMAL 模式 + AMR=0：
Auto2 output stack FULL (sensor) - remove finished trays

清機殘留 EventLog（MES1623）：
message = "Auto6 clean-out residual tray after drain - remove it"
detail  = "front=1 full=0 rear=0 Lot=NQ8002ZAA1:FAIL"
```

---

## 6. 風險與已知副作用

| # | 風險 | 處置 |
|---|---|---|
| R1 | **清機完成被綁在 AMR 身上**：AMR 不來 → 卡在 `Run_CleanOut` | 這個風險**現在就存在**（`IsAllCleanOutFinish` 的 front-sensor 閘早已 hold），本案不會更糟，反而多一條「叫車」的解法。逾時逃生門（§4.8）＋ `bUseAMR` 閘是保底 |
| R2 | `bUseAMR==0` 誤觸發 → 鎖住 Auto 且無人可解 | `IsCleanOutCollectDueForAmr()` 第一行擋掉。與 `START_AGV` 在 AMR off 時回 HCACK=2 的既有守衛一致（`uHGemHT160.cpp:2725`） |
| R3 | 每秒重複發 CEID 272 | 既有 `Handshake[si]==AGV_IDLE` 天然 one-shot |
| R4 | ~~前置通知 CEID 語意失真：車未滿卻發「Auto N Full」~~ | ✅ **已消除（20260901 覆議 ⑥）**：收尾叫車不發這六個 CEID，`Gem->EventReport(1, AutoFullCeid[a])` 改為 `if(bTrueFull)` 才發。工作簿的「AutoN 車滿」敘述維持正確，**不需要改工作簿，也不需要問客戶** |
| R5 | 收尾叫車鎖住 Auto → `GetTrayRequest()` 回 `eTrayReqNone` | **這是要的**。且 `Run_CleanOut` + SortArm 清機完成時本來就已回 `eTrayReqNone`（`:1408`） |
| R6 | 與 `ReleaseInfeedForCleanOut()` 衝突 | 不會：那支只處理 P1–P3，`uAgvStation.cpp:430` 註解明載 outfeed 不碰 |
| R7 | sim build 行為改變 | `IsFrontHasTrayForAmr()` 在 sim 回 false；Part C 只動真機分支；Part B/D 的 helper 在 sim 照跑但無 sensor 觸發點。**sim 行為不變** |
| R8 | **警報文字會一起送到 host**：`AlarmReport(Code, Message, bSet)` → `ALTX = Code + " " + Message`（`UsecegemMainFrom.cpp:190`） | **裁定已接受**。`ALID` 不變（只 hash 警報碼），以 ALID 對號的 EAP 不受影響；ALTX 變長並帶批號，對 MES 是加分。⚠ **需知會京元** |
| R9 | Part C 之後 `MES1x25` 成為永不觸發的目錄項 | `system/AlarmList.csv` 的六列**保留**——刪除會改變 host 從 S5F6 拿到的 ALID 集合，而一個不會觸發的目錄項無害 |
| R10 | Part C 之後 `AMR=1 + SECS 斷線` 累積 100 盤 → 流道停止供盤且無通知 | **已知並接受**：需要三個條件同時成立（AMR 開、SECS 斷、累積 100 盤），且 AMR 斷線本來就需要人介入。裁定 ⑮ 明示不留 EventLog |

---

## 7. 驗證計畫

| # | 項目 | 方法 | 通過標準 |
|---|---|---|---|
| T1 | 編譯（模擬） | `scripts/ops/build-ht160s.ps1 -Full` | EXIT 0 |
| T2 | 編譯（真機） | 註解掉 `MachineType.h` 的 `#define SOFT_SIMULATE`，`-Full`，**驗完復原再 build** | EXIT 0 |
| T3 | 編碼 | `scripts/ops/check-ht160s-source-encoding.ps1` | PASS |
| T4 | HOME 迴歸 | `ht160s-home-selftest` skill | 全軸 HOMED，M13/M18 維持 disabled |
| T5 | sim 清機不受影響 | 模擬 build 跑一次 Clean Out | 正常完成，無 CEID 272 P4–P9 |
| T6 | **上機：收尾叫車** | 真機跑一批，Clean Out 時讓某 Auto 車上留 1–2 盤 | 該站發 CEID 272（bitmap 對應站別）＋ 對應 Auto Full CEID；`FeederDecision.txt` 該站 `hs=CALLED` |
| T7 | **上機：逾時逃生** | 同 T6 但 host 不回 START_AGV | 逾時後彈 WAR0962；選「自行搬走」→ `InputHasTray` OFF → 清機完成 |
| T8 | **上機：完整下料** | 同 T6 且 host 回 `START_AGV { AUTOn = "Action" }` | 272 → 273 → 274 走完 |
| T9 | **上機：AMR off** | `UseAMR=0` 跑清機 | **不發** CEID 272；`MES1x23` 出現且**帶批號** |
| T10 | 六站獨立 | 兩個 Auto 都留盤 | 各自叫車，先清完的不等後清完的 |
| T11 | **Full 警報顯示（dynamic）** | By Lot+PassFail，AMR=0，讓某站 Full sensor 亮 | 警報文字含 `Lot=<批號>:PASS`（或 `:FAIL`），**不含** `trays=` / `ICs=` |
| T12 | **Full 警報顯示（NORMAL）** | NORMAL 模式，AMR=0，同上 | 警報文字**完全沒有** `Lot=` 欄，也沒有數量 |
| T13 | **Full 警報顯示（Error 流道）** | dynamic 模式，讓 Error 流道 Full sensor 亮 | 顯示 `Lot=(Error lane / reject)` |
| T14 | **Full 警報數量（AMR=1）** | AMR=1 但 SECS 斷線，讓某站 Full sensor 亮 | 顯示 `trays=N  ICs=M`，數字與 State Record 的 `CarTrays` 一致 |
| T15 | **清除後歸零** | T14 後移除料、按 RETRY | 下一次警報 `trays=0  ICs=0`（`Car::Clear()` + `InitAutoCarStack()`） |
| T16 | **邏輯滿已移除** | 真機（任何模式） | `MES1125`–`MES1625` 永不出現，EventLog 亦無 |
| T17 | S5F1 ALTX | 用 SECS Host Simulator 接，觸發 T11 | ALTX = `"MES1220 Auto2 output stack FULL (sensor) - remove finished trays | Lot=..."`；ALID 與改動前相同 |

---

## 8. 需要 host / 客戶端配合的事（不在本案韌體範圍）

1. **host 必須真的回 `START_AGV`，且該 AUTO 站的 verb 是 `"Action"`。**
   2026-08-31 全日 3 次 `START_AGV` 都是 `Loader="Action"`、`AUTO1..AUTO6="NA"`，
   下料握手因此從未啟動。**沒有這一步，本案的叫車只會逾時。**
2. **若要在 CEID 274 收到關帳盤數與 IC 數**，host 的 `S2F35` 必須把含
   Tray Count（`38222-38227` / `38246-38248`）與 Device Count（`38228-38233` / `38240-38242`）
   的報表號連到 274。現場連結為 `{502, 2000}`，韌體預設的 Report 6 被覆寫掉了。
   （已寫入 `SECS_GEM功能_Handler_20260831.xlsx` 修訂說明 B9 / B13 與
   `HT160S_SECS_Interface_Spec_20260727.md` §3.3.4，commit `5f9d122`。）
3. ~~【blocker】R4 — 前置通知 CEID 的語意變更~~ → ✅ **已解除**。20260901 覆議裁定 ⑥：
   收尾叫車**不發**那六個 CEID，只有真的滿才發。京元收到的 `Auto N Full` 語意維持不變，
   **不需要問、也不需要改工作簿**。§9.1 的問句草稿保留為歷史紀錄，不需寄出。

4. **知會 R8（低風險，可隨文件走）**：Auto Full 警報的 S5F1 `ALTX` 文字會變長並帶批號；
   `ALID` **不變**（`ComputeAlarmAlid()` 只 hash 警報碼），以 ALID 對號的 EAP 不受影響。
   **這是本案唯一還需要知會客戶的事項**，且可等上機驗完隨新版工作簿一起走。

### 8.1 這兩則的文件時機（20260901 定案）

**現在不寫進工作簿。** `SECS_GEM功能_Handler_20260831.xlsx` 的修訂說明第一頁自己釘著
「依據韌體 Firmware：HT-160S 1.0.0.0 ... **commit 7f7b391**」——工作簿是**釘在某個 firmware commit 上的快照**，
寫入還沒實作的行為，就是 2026-09-01 剛花兩個 commit 修掉的那個病（文件與程式碼不同步）。

**也不放「尚待貴端確認 Open items」頁。** 那一頁放的是「需要京元回答我們才能往下走」的事；
R4 / R8 是**已決定要做的變更通知**。放進去會讓京元把已定案的行為讀成「還沒定案」，製造認知空窗。

**正確時機與位置**：Part A–D 實作完成並上機驗過（T6 / T11 / T17）之後，出新版工作簿時
寫進**「本次修訂內容 Changes」**（A7–A12 區）。在那之前，這兩則的正式落腳處就是本節，
跟著 commit 走不會遺失。

---

## 9. 待辦

**本方待裁定事項：無。** 十六條裁定 + 三條覆議（§2 ④、§2 ⑥、§8.1）已全部定案並實作。

**外部 blocker：無。** 原本的 R4 已由覆議 ⑥ 消除（收尾叫車不發 `Auto N Full`）。

**剩下的只有上機驗證**：§7 的 T6–T16 共 11 項，需要真機 + AMR + host 配合。
`T1`–`T5` 已在 2026-09-01 完成（見 §10）。

### 9.1 給京元的問句草稿（R4）

> **中文**
> 關於 HT-160S 的 AMR 下料流程，我們即將新增一項行為，需要先確認貴端 EAP 是否可接受。
>
> 目前機台只在**出料車實體滿**（`SnAutoX_InputFullTray` 感測器 ON）時才發 `CEID 272 AMR Supplement` 叫車。
> 新增後，在 **Clean Out 收尾階段**（Loader 兩側、SortArm、該 Auto 都已排空，但該 Auto 出料車上還有盤）
> 也會叫車，**此時出料車未必是實體滿**——目的是讓成品盤在 Lot End 前被收走，不跨批留在機上。
>
> 這個新的叫車會沿用既有的前置通知 CEID：Auto1–3 = **35 / 36 / 37**，Auto4–6 = **148 / 149 / 150**
> （即 `Auto N Full`）。
>
> **請確認**：貴端 EAP 若以這六個 CEID 判定「該站出料車已滿」或用於盤數記帳，
> 收到「車未滿」的這一則是否會造成問題？若不可接受，我們可以改為：
> (a) 收尾叫車不發這六個 CEID，只發 `CEID 272`；或
> (b) 由貴端指定另一個 CEID 號碼專用於收尾叫車。
>
> **English**
> We are adding one behaviour to the HT-160S AMR unload flow and need your confirmation first.
>
> Today the equipment raises `CEID 272 AMR Supplement` only when an output car is **physically full**
> (`SnAutoX_InputFullTray` sensor ON). We will additionally raise it during **Clean Out**, once the
> Loader (both sides), the SortArm and that Auto have all drained but the Auto's output car still
> holds trays — **the car is not necessarily full at that moment**. The intent is that finished trays
> leave the machine before Lot End instead of carrying over into the next lot.
>
> This new call reuses the existing pre-notification CEIDs: Auto1-3 = **35 / 36 / 37**,
> Auto4-6 = **148 / 149 / 150** (`Auto N Full`).
>
> **Please confirm**: if your EAP uses those six CEIDs to decide "this output car is full", or for
> tray accounting, would receiving one for a not-full car cause a problem? If it is not acceptable
> we can either (a) omit those six CEIDs on the clean-out collect call and send only `CEID 272`,
> or (b) use a different CEID that you nominate for this case.

---

## 10. 實作紀錄（2026-09-01）與規格差異

### 10.1 實際改動的檔案

| 檔案 | 改動 |
|---|---|
| `SecsGem/uAgvStation.h` | +1 宣告 `IsCleanOutCollectDueForAmr(int)` |
| `SecsGem/uAgvStation.cpp` | +1 include (`aSortArm.h`)、+1 函式、`PollAndCall` 的 `bFull` 拆成 `bTrueFull` / `bCollect` / `bFull`，`AutoFullCeid` 發射改為 `if(bTrueFull)` |
| `aAuto1To6.h` | +3 宣告 `IsStationCleanOutFinish` / `IsFrontHasTrayForAmr` / `DescribeLaneLotForOperator` |
| `aAuto1To6.cpp` | +3 函式定義；`ServiceCarFull` 的 `MES1x20` 加 Lot/counts；清機排空 Full 閘的 `MES1x20` 加 Lot；`MES1x25` 真機分支註解掉；`MES1x23` 加 Lot |
| `CosFunction.h` | `THT160LotBinBinding::GetErrorAutoIndex()` 由 private 改為 public |

### 10.2 與規格的三處差異（都是實作時才發現的）

1. **`MES1x20` 有兩個發射點，規格只寫了一個。**
   除了 `ServiceCarFull()`（`:1955` 附近），`DoAllAutoCleanOut()` case 4000 的 Full 閘（`:1135` 附近）
   也會彈同一個 `MES1x20`（AMR=0 路徑；AMR=1 在該處 `continue` 交給 coordinator）。
   **兩處都加了 Lot 標籤**——同一個警報碼、同一個操作動作，不能一個有一個沒有。
   排空 Full 閘那處**不加數量**（該分支只在 AMR=0 執行，裁定 ⑬ 本來就不顯示）。

2. **`GetErrorAutoIndex()` 原本是 private**，`DescribeLaneLotForOperator()` 呼叫不到（編譯錯誤 E2247）。
   改為 public 並加註說明。它是純唯讀的推導 getter（把 `BinAreaMap.GetErrorBinArea()` 映成 0-based Auto 索引），
   公開它不新增任何狀態、也不新增任何改動表格的途徑。

3. **`uAgvStation.cpp` 原本看不到 `SortArmModule`**，補 `#include "aSortArm.h"`。

### 10.3 已完成的驗證

| # | 項目 | 結果 |
|---|---|---|
| T1 | 模擬建置 `-Full` | ✅ EXIT 0 |
| T2 | 真機建置 `-Full`（`//#define SOFT_SIMULATE`） | ✅ EXIT 0，驗畢已還原 `#define SOFT_SIMULATE` 並重建 |
| T3 | 編碼檢查 | ✅ 166 files PASS，無 `EF BF BD`、無 BOM |
| T4 | HOME 迴歸自測 | ✅ 見下方 commit 訊息 |
| T5 | sim 清機不受影響 | ✅ `IsFrontHasTrayForAmr()` 在 sim 回 false，`MES1x25` 的 sim 分支未動 |

⚠ **`aAuto1To6.cpp` 是 CP950/Big5**，不能用 harness 的 Edit/Write 工具（會把中文變成 U+FFFD）。
本次以 PowerShell + `Encoding.GetEncoding(950)` 逐段錨點取代完成，改後驗證：
仍為非 UTF-8（＝CP950 未被破壞）、`U+FFFD` 計數 0、CRLF 69 / bare-LF 2076→2166（只增不改）。

### 10.4 真機版 EXE

T2 產出的真機版另存為 `EXE\ht160s_REALMACHINE_20260901.exe`（`EXE\ht160s.exe` 已被之後的模擬建置覆蓋）。
⚠ **要出貨給客戶請用前者，或重跑一次真機建置**——`EXE\ht160s.exe` 現在是模擬版。

---

## 11. 相關

- 記憶：`ht160s-onsite-kyec-findings`（2026-08-31 節）、`amr-owner-rules-no-hang-no-interference`、
  `ht9045-cleanout-trigger-model`、`amr-updown-ic-count-contract`、`amr-startagv-action-na-lock`、
  `silent-stop-must-notify`、`alarm-system`
- 現場資料：`D:\HT160S_StateRecord\20260831第一台結束的staterecod.zip`
- 前置 commit：`5f9d122`（規格書 + 工作簿同步）
