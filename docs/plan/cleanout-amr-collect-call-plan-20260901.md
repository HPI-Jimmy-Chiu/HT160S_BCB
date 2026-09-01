# Clean Out 收尾叫車（AMR 下料）實作規格

- **日期**：2026-09-01
- **狀態**：**規格待審 — 尚未動任何程式碼**
- **分支**：`feat/iosetview-172-refactor`
- **來源**：2026-08-31 京元上機 State Record 分析（`D:\HT160S_StateRecord\20260831第一台結束的staterecod.zip`，機台 HT160S-01，`UseAMR=1`）
- **裁定人**：使用者（2026-09-01 逐點確認，見 §2）

---

## 1. 需求

現行 AMR 下料叫車的**唯一**觸發條件是「出料車滿」——真機 `SnAutoX_InputFullTray` 感測器 ON。
Clean Out 收尾時，出料車通常**沒滿但有盤**，此時機台不會叫 AMR 來收，只會丟一行 EventLog 請操作員來搬。

**要新增的觸發**（與現有滿車觸發 **OR**，兩者互不取代）：

以 Auto1 為例，

```
叫車 = SnAuto1_InputFullTray == ON                    ← 既有：產能觸發（車滿了，換車繼續跑）
    OR ( Loader  清機完成
      AND SortArm 清機完成
      AND Auto1  清機完成
      AND 貨批尚未 Lot End
      AND SnAuto1_InputHasTray == ON )                ← 新增：收尾觸發（車沒滿也要收走）
```

Auto2~Auto6 依站別類推，**六站各自獨立判斷，不連坐**。

### 1.1 語意

| 觸發 | 意義 |
|---|---|
| 滿車觸發 | **產能觸發**：跑到一半車滿，換車繼續跑 |
| 收尾觸發 | **交件觸發**：這批做完、前段模組都排空，Auto 上的成品盤不論幾盤都要在 Lot End 前收走，不留在機上跨批 |

用 `SnAutoX_InputHasTray`（車上有盤）而非 `InputFullTray`（車滿）就是這個差別——有東西就收，不看量。

### 1.2 現場證據

2026-08-31 17:19:08 的 EventLog：

```
2026/08/31,17:19:08.969,,0,,MES1623,"Auto6 clean-out residual tray after drain - remove it","front=1 full=0 rear=0"
```

`front=1 full=0` ＝ `SnAuto6_InputHasTray` ON、`SnAuto6_InputFullTray` OFF。**車沒滿但有盤**，
清機被 hold 住，機台只發一行 EventLog 叫人來搬。這正是本案要補的洞。

同日全機 AMR 統計：CEID 272 共 27 次，其中 P4–P9（Auto 下料）**只有 1 次**（11:25:00，
機台未 HOME 的閒置狀態，Tray/Device 皆 0），CEID 273 / 274 對 P4–P9 **全日 0 次**。

---

## 2. 已裁定事項（使用者 2026-09-01）

| # | 議題 | 裁定 |
|---|---|---|
| ① | 是否寫進 `IsOutputCarFullForAmr()` | **不可**。另開新函式，`IsOutputCarFullForAmr()` 一個字不動 |
| ② | Loader 清機完成的定義 | **L+R 兩側都排空** ＝ `LoaderModule->IsAllCleanOutFinish()` |
| ③ | Auto 清機完成的定義 | **只看該站自己**，不看其他五站 |
| ④ | 「Lot End 前」的界線 | 以 `CheckCleanOutFinish()` 為框：其餘條件成立後開始不滿車檢查，**直到 `SnAutoX_InputHasTray` 變 OFF，`IsAllCleanOutFinish()` 才是真的 true**；外框接受「只要 Lot 還開著就算」 |
| ⑤ | 撤銷分支 | 併進同一個 `bFull` 布林，讓現有的 `bFull==false → 放鎖回 IDLE` 自動適用 |
| ⑥ | 前置通知 CEID（35/36/37/148/149/150） | **照發**，因為這就是叫車流程 |
| ⑦ | `bUseAMR==0` | **完全不叫車**，維持現行「MES1x23 通知操作員搬走」的行為 |
| ⑧ | 逾時逃生門 | 沿用 `iAgvTimeoutSec` + `TimeoutPending`，逾時彈 WAR 讓操作員選重叫或自行搬走。**不做靜默放行** |

---

## 3. 現況調查（本案的關鍵發現）

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

這就是裁定 ④ 說的「直到 `SnAutoX_InputHasTray` 為 OFF，`IsAllCleanOutFinish()` 才是真的 true」，
且只在 `RunMode == Run_CleanOut` 期間生效（函式開頭 `:1257` 分流）。
往上一層 [`csystem.cpp:2044`](../../HT160S_Program_BCB_V1.0.0.0/csystem.cpp) `CheckCleanOutFinish()` 已把它串進六模組串聯。

**所以本案缺的只有「叫車」，不是「hold」。改動範圍比原本估計小很多。**

### 3.2 現行叫車判斷

[`uAgvStation.cpp:443-479`](../../HT160S_Program_BCB_V1.0.0.0/SecsGem/uAgvStation.cpp) — `TAgvCoordinator::PollAndCall()`：

```cpp
int AutoFullCeid[6] = {35, 36, 37, 148, 149, 150};             // :447
for(int a = 0; a < AGV_AUTO_COUNT; a++)
{
    int si = a + 3;                                             // Auto1->P4(idx3) .. Auto6->P9(idx8)
    ...
    bool bFull = AutoModule->IsOutputCarFullForAmr(a) || AmrInject.AutoFull(a);   // :464  ← 改這一行
    if(bFull && Handshake[si]==AGV_IDLE)
    {
        AutoModule->SetAmrLock(a, true);
        SupplementBitmap = BuildBitmap(AgvStation[si].PIndex);
        Gem->EventReport(1, 272);                               // :470
        Gem->EventReport(1, AutoFullCeid[a]);                   // :471
        Handshake[si] = AGV_CALLED;
    }
    else if(bFull==false && Handshake[si]==AGV_CALLED)           // :474  ← 自動適用
    {
        AutoModule->SetAmrLock(a, false);
        Handshake[si] = AGV_IDLE;
    }
}
```

這個迴圈**已經會在 `Run_CleanOut` 執行**（`:437` 的 D4-2 註解與 RunMode 閘），只是條件永遠不成立。

### 3.3 `IsOutputCarFullForAmr()` 為何不能碰

[`aAuto1To6.cpp:1563`](../../HT160S_Program_BCB_V1.0.0.0/aAuto1To6.cpp)，共 5 個 caller：

| 行號 | caller | 用途 |
|---|---|---|
| `:1118` | `DoDischargeTray` | 卸盤後是否已滿 |
| `:1283` | `IsAllCleanOutFinish` | 清機完成閘 |
| **`:1425`** | **`GetTrayRequest`** | **擋 TrayArm 送盤** ← 若把新條件寫進去，Auto 一清完就永遠要不到盤 |
| `:1756` | 狀態傾印 | State Record |
| `uAgvStation.cpp:464` | `PollAndCall` | 叫車 |

**只有最後一個是我們要改的語意。**

---

## 4. 改動點清單

> 全部為新增；除 `uAgvStation.cpp:464` 那一行外，沒有任何既有邏輯被修改。

### 4.1 `aAuto1To6.h` — 新增兩個 accessor 宣告

| 位置 | 內容 |
|---|---|
| public 區（`IsOutputCarFullForAmr` 宣告附近，約 `:169`） | `bool IsStationCleanOutFinish(int Index);`  — 該站自己的 `State[Index].bCleanOutFinish`（裁定 ③） |
| 同上 | `bool IsFrontHasTrayForAmr(int Index);` — 該站 `SnAutoX_InputHasTray` 的即時讀值（真機讀 sensor；sim 回 false） |

### 4.2 `aAuto1To6.cpp` — 兩個 accessor 實作

```cpp
//AI(cleanout-amr-collect) 20260901 : per-station CleanOut drain latch, for the AMR
// collect-call gate. Deliberately NOT IsAllCleanOutFinish() - that one is module-wide
// (all six stations) and would make a station that finished early wait for the slowest.
bool TAutoModule::IsStationCleanOutFinish(int Index)
{
    if(Index<0 || Index>=AUTO_STATION_COUNT)
        return false;
    return State[Index].bCleanOutFinish;
}

//AI(cleanout-amr-collect) 20260901 : "this Auto's output stack still holds at least one
// tray" - the SAME sensor IsAllCleanOutFinish() already blocks on (aAuto1To6.cpp:1274),
// exposed so the AGV coordinator can turn that block into a COLLECT CALL instead of a
// silent wait. Sim returns false : a laptop run has no AMR to answer the call.
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

### 4.3 `uAgvStation.h` — 新增一個私有方法宣告

```cpp
bool IsCleanOutCollectDueForAmr(int AutoIndex);   //AI(cleanout-amr-collect) 20260901
```

### 4.4 `uAgvStation.cpp` — 新增判斷函式

```cpp
//AI(cleanout-amr-collect) 20260901 : the SECOND reason to call the AMR to an Auto -
// the CLEAN-OUT COLLECT. The first reason (IsOutputCarFullForAmr) is a THROUGHPUT
// trigger: the car filled up mid-run, swap it and keep going. This one is a HANDOVER
// trigger: this lot is done, the front-of-line modules are drained, and whatever is on
// the output car - full or not - must leave before Lot End rather than cross into the
// next lot.
//
// WHY THIS IS A SEPARATE FUNCTION AND NOT AN EXTRA TERM INSIDE IsOutputCarFullForAmr():
// that predicate has five callers and one of them (aAuto1To6.cpp:1425 GetTrayRequest)
// uses it to REFUSE new trays. Widening it there would make an Auto stop asking for
// trays the moment it latched drain-done - a different behaviour entirely, and one that
// would bite outside CleanOut too. Owner ruling 20260901.
//
// The HOLD side of this behaviour ALREADY EXISTS and is deliberately left untouched:
// TAutoModule::IsAllCleanOutFinish (aAuto1To6.cpp:1274) already returns false while
// SnAutoX_InputHasTray is ON, so CheckCleanOutFinish (csystem.cpp:2044) already parks
// the machine in Run_CleanOut until the car leaves. What was missing was any code that
// ASKS for it to leave: today the only notice is the EventLog line from
// ServiceCleanOutResidualWatchdog ("AutoN clean-out residual tray after drain - remove
// it"), i.e. the machine waits for an OPERATOR. On-site 2026-08-31 17:19:08 that is
// exactly what happened on Auto6 (MES1623, front=1 full=0).
bool TAgvCoordinator::IsCleanOutCollectDueForAmr(int AutoIndex)
{
    if(GeneralSetting.bUseAMR==false)          // owner ruling : AMR off -> operator handles it
        return false;
    if(AutoModule==NULL || LoaderModule==NULL || SortArmModule==NULL)
        return false;
    if(HSys.Sys.RunMode!=Run_CleanOut)         // collect window is the CleanOut drain only
        return false;
    if(IsLotOpenForCollect()==false)           // "before Lot End" (see helper below)
        return false;
    if(LoaderModule->IsAllCleanOutFinish()==false)   // owner ruling : BOTH sides drained
        return false;
    if(SortArmModule->IsCleanOutFinish()==false)
        return false;
    if(AutoModule->IsStationCleanOutFinish(AutoIndex)==false)   // owner ruling : this station only
        return false;
    return AutoModule->IsFrontHasTrayForAmr(AutoIndex);
}
```

### 4.5 `uAgvStation.cpp:464` — 唯一修改的既有行

```cpp
// 現行
bool bFull = AutoModule->IsOutputCarFullForAmr(a) || AmrInject.AutoFull(a);

// 改為
//AI(cleanout-amr-collect) 20260901 : ONE boolean on purpose - the CALL branch below and the
// release branch at :474 must agree, or a collect-call raised on one tick would be revoked by
// the release branch on the next. Adding a separate if() instead of OR-ing here is the bug.
bool bFull = AutoModule->IsOutputCarFullForAmr(a)
          || AmrInject.AutoFull(a)
          || IsCleanOutCollectDueForAmr(a);
```

裁定 ⑤（撤銷分支）與裁定 ⑥（前置通知 CEID）**都由這一行自動滿足**——`:471` 的
`Gem->EventReport(1, AutoFullCeid[a])` 在同一個 if 內，`:474` 的釋放分支讀同一個 `bFull`。

### 4.6 「Lot 還開著」的判定

新增一個檔案內 static helper（放在 `uAgvStation.cpp` 檔頭的 static 區，與 `InfeedShortage` 等同區）：

```cpp
//AI(cleanout-amr-collect) 20260901 : "the lot has not ended yet". CleanOut runs INSIDE an open
// lot (Lot End is a separate operator/host action afterwards), so this is the outer boundary of
// the collect window - owner ruling 20260901, "只要 Lot 還開著就算".
static bool IsLotOpenForCollect()
{
    if(LotRegistry.GetLotCount() > 0)
        return true;
    return (fMain!=NULL && fMain->ActiveLotID().Trim()!="");
}
```

> ⚠ **待確認**：`fMain->ActiveLotID()` 的 fallback 會在「操作員在批號欄打了字但沒開批」時回非空。
> 若要嚴格，改成只看 `LotRegistry.GetLotCount()>0`。**實作前請裁定。**

### 4.7 逾時逃生門（裁定 ⑧）

**不需要新程式碼** —— [`uAgvStation.cpp:596`](../../HT160S_Program_BCB_V1.0.0.0/SecsGem/uAgvStation.cpp) 的
既有機制對 `AGV_CALLED` 已經生效：

```cpp
if((Handshake[si]==AGV_CALLED || Handshake[si]==AGV_PREP || Handshake[si]==AGV_READY)
   && Handshake[si]==hsBefore)
{
    if(++ShortageDebounce[si] > GeneralSetting.iAgvTimeoutSec && TimeoutPending[si]==0)
    { ... latch TimeoutPending -> csystem 主迴圈彈 WAR0962 ... }
}
```

收尾叫車進的就是 `AGV_CALLED`，所以逾時後一樣會 latch `TimeoutPending`、由 csystem 主迴圈彈
WAR0962，K_RETRY 重叫。**唯一要做的是驗證這條路徑在 `Run_CleanOut` 下沒有被別的 RunMode 閘擋掉**（見 §6 T7）。

---

## 5. 風險與已知副作用

| # | 風險 | 處置 |
|---|---|---|
| R1 | **清機完成被綁在 AMR 身上**：AMR 不來 → `CheckCleanOutFinish()` 永遠 false → 機台卡在 `Run_CleanOut` | ⚠ 這個風險**現在就已經存在**（`IsAllCleanOutFinish` 的 front-sensor 閘早就 hold），本案不會讓它更糟，反而多了一條「叫車」的解法。逾時逃生門（§4.7）是保底。牴觸「握手不可以是卡住的那個東西」的部分由 §4.7 + `bUseAMR` 閘覆蓋 |
| R2 | `bUseAMR==0` 誤觸發 → 鎖住 Auto 且無人可解 | `IsCleanOutCollectDueForAmr()` 第一行就擋掉。與 `START_AGV` 在 AMR off 時回 HCACK=2 的既有守衛一致（`uHGemHT160.cpp:2725`） |
| R3 | 每秒重複發 CEID 272 | 既有的 `Handshake[si]==AGV_IDLE` 條件天然 one-shot；進 `AGV_CALLED` 後不再發 |
| R4 | 前置通知 CEID 語意失真：車其實沒滿卻發「Auto N Full」 | 裁定 ⑥ 已接受。⚠ **需在客戶工作簿的 CEID 頁補一句**：35/36/37/148/149/150 亦會在 Clean Out 收尾叫車時發射，此時出料車未必滿 |
| R5 | 收尾叫車鎖住 Auto（`SetAmrLock`）→ `GetTrayRequest()` 回 `eTrayReqNone` | **這是要的**：清機收尾階段本來就不該再送盤進來。且 `GetTrayRequest` 在 `Run_CleanOut` + SortArm 清機完成時本來就已回 `eTrayReqNone`（`aAuto1To6.cpp:1408`） |
| R6 | 與 `ReleaseInfeedForCleanOut()` 衝突 | 不會：那支只處理 P1–P3 進料站，`uAgvStation.cpp:430` 註解明載「Outfeed (P4-P9 Auto unload) is intentionally NOT touched」 |
| R7 | sim build 行為改變 | `IsFrontHasTrayForAmr()` 在 `IsSoftSimulate()` 回 false → sim 完全不受影響，`--selftest-home` 與模擬清機照舊 |

---

## 6. 驗證計畫

| # | 項目 | 方法 | 通過標準 |
|---|---|---|---|
| T1 | 編譯（模擬） | `scripts/ops/build-ht160s.ps1 -Full` | EXIT 0 |
| T2 | 編譯（真機） | 註解掉 `MachineType.h` 的 `#define SOFT_SIMULATE`，`-Full`，**驗完復原再 build** | EXIT 0 |
| T3 | 編碼 | `scripts/ops/check-ht160s-source-encoding.ps1` | PASS（無 `EF BF BD`、無 UTF-8 BOM） |
| T4 | HOME 迴歸 | `ht160s-home-selftest` skill | 全軸 HOMED，M13/M18 維持 disabled |
| T5 | sim 清機不受影響 | 模擬 build 跑一次 Clean Out | 清機正常完成，無 CEID 272 P4–P9 |
| T6 | **上機：收尾叫車** | 真機跑一批，Clean Out 時讓某個 Auto 車上留 1–2 盤 | 該站發 CEID 272（bitmap 對應站別）+ 對應 Auto Full CEID；`FeederDecision.txt` 該站 `hs=CALLED` |
| T7 | **上機：逾時逃生** | 同 T6 但 host 不回 START_AGV | `iAgvTimeoutSec` 後彈 WAR0962；選「自行搬走」後移除盤 → `InputHasTray` OFF → 清機完成 |
| T8 | **上機：完整下料** | 同 T6 且 host 回 `START_AGV { AUTOn = "Action" }` | 272 → 273 → 274 走完；274 body 帶 Tray/IC 數（前提：host 已把含 count 的報表號連到 274，見 §7） |
| T9 | **上機：AMR off** | `UseAMR=0` 跑清機 | **不發** CEID 272；維持 MES1x23 通知操作員 |
| T10 | 六站獨立 | 兩個 Auto 都留盤 | 兩站各自叫車，先清完的不等後清完的 |

---

## 7. 需要 host 端配合的事（不在本案韌體範圍）

1. **host 必須真的回 `START_AGV`，且該 AUTO 站的 verb 是 `"Action"`。**
   2026-08-31 全日 3 次 `START_AGV` 都是 `Loader="Action"`、`AUTO1..AUTO6="NA"`，
   下料握手因此從未啟動。**沒有這一步，本案的叫車只會逾時。**
2. **若要在 CEID 274 收到關帳盤數與 IC 數**，host 的 `S2F35` 必須把含
   Tray Count（`38222-38227` / `38246-38248`）與 Device Count（`38228-38233` / `38240-38242`）
   的報表號連到 274。現場連結為 `{502, 2000}`，韌體預設的 Report 6 被覆寫掉了。
   （已寫入 `SECS_GEM功能_Handler_20260831.xlsx` 修訂說明 B9 / B13 與
   `HT160S_SECS_Interface_Spec_20260727.md` §3.3.4。）

---

## 8. 待裁定（實作前需回覆）

1. §4.6 的「Lot 還開著」是否要嚴格化為只看 `LotRegistry.GetLotCount()>0`？
2. R4：前置通知 CEID 在車未滿時照發，是否要在客戶工作簿 CEID 頁補說明？（我建議補）
3. 收尾叫車是否也要寫一行 EventLog（例如 `AMR collect call: AutoN clean-out residual, trays=N`）？
   現行滿車叫車不寫 log，只有 SECS 事件；但收尾叫車是新行為，有 log 比較好追。

---

## 9. 相關

- 記憶：`ht160s-onsite-kyec-findings`（2026-08-31 節）、`amr-owner-rules-no-hang-no-interference`、
  `ht9045-cleanout-trigger-model`、`amr-updown-ic-count-contract`、`amr-startagv-action-na-lock`
- 現場資料：`D:\HT160S_StateRecord\20260831第一台結束的staterecod.zip`
