> ## 文件狀態(2026-09-07)
>
> **尚未動工。** 本文是 D4(Lot End 前把 Auto 出料車全部叫走)的實作計畫,經過兩輪獨立對抗式複驗:
>
> | 輪次 | 裁決 | 結果 |
> |---|---|---|
> | v1 | **UNSOUND** | 6 個阻斷缺陷(B1–B6)+ 9 個缺失逃生門 |
> | v2(本文主體) | **SOUND_WITH_FIXES** | B1/B3/B5/B6 關閉;**B2 未完全關閉**、**B4(b) 未關閉**;另引入 NI-1…NI-9 |
>
> **使用者已裁定兩項(2026-09-07),已套用進本文:**
>
> - **OD-1 = 分支 A(拆)**:落地類副作用(`g_SoterOutput.OnLotEnd()` + `WriteLastDataIni()`)前移到
>   「開始等待」那一刻;CEID 8 / 歸檔 / 清帳留在原時刻。
> - **OD-2 = 分支 A(停下來等人)**:AMR 一直不來且 RETRY 用完 → `WAR0964` 一直掛著,`K_SKIP` 是唯一
>   無條件出路,機台不自行放行。使用者明示接受「夜班可能停整夜」的代價,理由是料跨批不可接受。
>   → 因為選了 A,OD-1 的「拆」從優化變成**必要條件**:不拆的話停整夜等於整批交付品整夜留在 RAM。
>
> **仍待裁定:OD-3(阻斷)、OD-4、OD-5** — 見文末 §OD。
>
> **動工前必修:第二次複驗的 NI-1 / B2-case(c) / B4-(b) / NI-2 / NI-3** — 見文末 §V3。
> NI-1 是「批永遠結不了」的可達狀態,NI-2 / NI-3 是客戶看得見的 SECS 行為缺陷,三者都必須先修。

---

# D4:Lot End 前把 Auto 出料車全部叫走 — 實作計畫 **v2**

- 日期:2026-09-07
- 分支:`feat/iosetview-172-refactor`
- 建議檔名:`D:\HT160S_BCB\docs\plan\cleanout-amr-lotend-collect-all-cars-plan-20260907.md`
- 狀態:**v1 經對抗性審查判定 UNSOUND(6 個阻斷缺陷 B1–B6 + 9 個缺失逃生門)。本 v2 逐條關閉,計畫定稿待使用者核可(尚未動工)**
- 範圍:僅出料側 P4–P9(Auto1–6 輸出車)。進料側 P1–P3 在 Clean Out 的凍結行為完全不動。
- 前置計畫:`docs/plan/cleanout-amr-unload-lotend-plan-20260716.md`(D1–D6 原始裁定)、
  `docs/plan/cleanout-amr-collect-call-plan-20260901.md`(0901 已交付的部分實作)、
  `docs/plan/loader-overcount-nostop-cleanout-lotend-plan-20260721.md`(D4 正式延後紀錄)

---

## 0. v2 對 v1 的實質變更(速查)

| 缺陷 | v2 的處置 | 章節 |
|---|---|---|
| **B1** 純重構會抽走 AMR=0 的 CEID 42 與 `Run_Normal` 復位 | 抽出範圍縮到 `csystem.cpp:1927-1929` **三行**;`:1914` 與 `:1952-1953` **留在原處**;給一條單一 grep 不變量 | §4.C-1、§4.C-3、§8 S2 |
| **B2** WAR0962 抑制 + 放棄前置 base = 全新無聲停機 | **宣告不設任何前置**;只有「關帳」要求 base;抑制窗改為**由 D4 自己的計時器結構性封頂** | §4.B-3、§4.C-2 |
| **B3** 放棄動作無 once-per-episode 閂,`HTimer::Off()` 逾時後恆真 | 快照 once-per-episode(`bHandoverSnapshotDone`);`HTimer` 消費即 `Clear()`;RETRY 加**上限 5** | §4.C-2 第 6 步、§4.D |
| **B4** 條件 (a) 被 1-tick HSMS 抖動觸發;SKIP 銷毀車帳 | (a) 加「所有欠交接站 `Handshake==AGV_IDLE`」+ `iAgvTimeoutSec` 去抖;SKIP 先把帳寫進 EventLog+快照再清,並同步歸零協調器 SVID | §4.C-2 第 5/6 步 |
| **B5** Lot End 的資料落地被押到交接之後 | 明確兩段化。**`g_SoterOutput.OnLotEnd()` 自身冪等**(`cSoterOutput.cpp:380/545`)且 csystem 早已直接呼叫它(`:1936`)→ 前移**不需要動 `DoLotEndProcess` 一個字**。CEID 8 / 歸檔 / 清帳留在原處(理由見 §4.G) | §4.G、OD-1 |
| **B6** 「latch 就不會被撤銷」在規格上不成立 | latch 改成**短路 return**,插在 `bUseAMR`/NULL/**RunMode** 三閘之後、三個 finish 閘之前(**不是** critic 建議的五閘之前,理由見 V2-8) | §4.B-1 |

**v2 另外自行發現、v1 與 critic 都沒抓到的 10 點(V2-1 … V2-10)** 見 §6.4。其中 **V2-1(SKIP 會觸發 CEID 78 假庫存沖銷)** 與 **V2-2(Auto 排空梯是六站連鎖閘,D4 自己的鎖能把 base 壓成 false)** 是新的阻斷級發現。

---

## 0.5 覆核 critic 的每一條主張(我逐條自己對過原始碼)

**成立(維持 critic 的裁定)**:B1、B2、B3、B4、B5、B6 六個阻斷缺陷全部成立,缺失逃生門 1–9 全部成立,N1/N3/N4/N5/N6/N7/N8 成立。

**行號漂移(實質成立,但引用要更正,以免下一個讀者查不到)**:

| critic 寫的 | 實際位置 |
|---|---|
| `aAuto1To6.cpp:92` 清 `bCleanOutFinish` | **`:93`**(`:92` 是 `State[].Status=`);仍在 `if(bKeepMaterial) continue;`(`:100-101`)**之上**,結論不變 |
| `SetRearHasTrayFromTrayArm` `:660-669` | 清除語句在 **`:665`**,包在 `if(bHasTray && RunMode==Run_CleanOut)`(`:663-667`)內 |
| `ShowNoteAlarm` 停機在 `note.cpp:806-807` | **`:809-810`**(`:806-807` 是「ALARM DROPPED(modal busy)」的 early return)。**v1 寫的 809-810 才是對的** |
| 274 路徑歸零 SVID 在 `uAgvStation.cpp:681-682` | **`:684-685`**(`:681-682` 是 `FinishBitmap` + `EventReport(1,274)`) |
| 掉線裁定在 `uAgvStation.cpp:461-470` | 註解 **`:447-461`**,整段 `bSelected==false` 分支 **`:445-469`** |
| `HTimer::Off()` `:46-62` / `:50-58` | **`:48-65`**;不重置 `ulStartTicks`/`iTimeLen`,只 `InUsed=false` → **逾時後每次呼叫恆真,實質完全成立** |

**critic 有一處事實錯誤**:

- **N2「六站的 `bCleanOutFinish` 是排空梯 case 7000 逐站走出來的」→ 不成立。** `aAuto1To6.cpp:1210-1219` 的 case 7000 是**一個迴圈把六站一起設 true**(`:1219`),而 case 4000 的 `AreAllFlagsOn(bCleanOutCheck)`(`:1166`)是**六站連鎖閘**,所以六站必然**同一 tick 同時**武裝。→ **per-station deadline 是多餘的**;v2 改用「單一 episode deadline,只在欠交接集合**新增成員**時重新武裝」(§4.C-2 第 3 步)。理由與證據見 V2-3。
- **N1 的前提「`MachineType.h` 的 `SOFT_SIMULATE` 是開的」**:對 **committed(HEAD)** 成立(`git show HEAD:...MachineType.h` → `:7 #define SOFT_SIMULATE`),但**現在的工作樹是註解掉的**(`MachineType.h:7` = `//#define SOFT_SIMULATE`,`git status` 亦顯示該檔 M)。這是編譯閘流程「驗真機組態後未還原 define」的殘留。→ 列為 **S0 動工前的整理項**(V2-10)。N1 的結論本身完全成立。
- **`IsAmrCallableNow()` 不能放在 `csystem.cpp`**:`csystem.cpp` 只 include 了 `SecsGem/uAgvStation.h`(`:23`)與 `uHGemClass.h`(`:24`),**沒有** `uHGemEquipment.h`(`HGem` 的 extern 在 `uHGemEquipment.h:449`)。→ v1 把它放進協調器的決定是對的(`uAgvStation.cpp:19` 已 include)。

---

## 1. 背景

### 1.1 現況(v1 的七點已全數複驗通過,以下只補 v2 新查到的關鍵事實)

1. **Clean Out 完成分支的真實結構(B1 的根據)**:

```
csystem.cpp:1908  if(HSys.Sys.RunMode==Run_CleanOut)
          :1910      if(CheckCleanOutFinish())
          :1914          if(fMain!=NULL) fMain->EmitCleanOutOK();      <-- 共用前綴(兩種模式都跑)
          :1915          if(GeneralSetting.bUseAMR)
          :1927              HSys.Sys.bCleanOut=false;                 <-- AMR 專屬,僅此三行
          :1928              DoLotEndProcess("auto (CleanOut finish)");
          :1929              InitialAllTask();
          :1931          else { :1933-1950 非 AMR 模態路徑 }
          :1952          ChangeRunMode(Run_Normal);                    <-- 共用後綴
          :1953          SoftStop=true;
```

2. **`CheckCleanOutFinish()`(`csystem.cpp:2031-2055`)七項**:Loader / SortArm / Auto / TrayArm / Empty / Color / `HasICUnderMachineForCleanOut()`。全樹唯一呼叫點是 `:1910`。

3. **Auto 排空梯是六站連鎖閘(v2 新發現,V2-2)**:`aAuto1To6.cpp:1158-1159` 的 `if(bAmrLocked[Index]) continue;` 讓被鎖站不設 `bCleanOutCheck[Index]`,而 `:1166` 要求 `AreAllFlagsOn(bCleanOutCheck)` 才能推進 → 原始碼自己的註解 `:1151-1152` 寫明「**bCleanOutCheck is a six-station lockstep barrier, so one AMR-locked station holds the whole drain until CEID274 releases it**」。

4. **`TAutoModule::IsAllCleanOutFinish()`(`:1261-1294`)是模組級**:`:1271-1277` 任一站的 rear/car/front/fullIC 旗成立即 return false;`:1278-1291` 真機分支再讀四顆 sensor。

5. **`g_SoterOutput.OnLotEnd()` 自身冪等(v2 新發現,V2-5)**:`cSoterOutput.cpp:380` `if(!m_bActive) return;`(註解自稱 "already flushed by an earlier terminal path"),`:545` `m_bActive=false`。而 `csystem.cpp:1936` 的非 AMR 分支**早就**直接呼叫它。

6. **`DoLotEndProcess` 的停機三行在函式最上方(v2 新發現,V2-6)**:`main.cpp:3271-3274` `SystemStart=false; SoftStop=true; bRunning=false;`。所以「把 `DoLotEndProcess` 提前呼叫」在物理上不可行 —— 會凍結排空與 D4 閘門本身(`ProcessMotion` 在 `SystemStart==false` 直接 return,`csystem.cpp:1808-1809`;`DoAllProcess` 的閘在 `:416-417`)。

7. **`ArchiveWorkOrderToLotStory` 有 `GetLotCount()<=0` 早退(`main.cpp:3609-3610`)**。→ **v1 §4.C-3 與 R9 的理由「第二次會歸檔一份已被清空的工單」是錯的**(V2-4):第二次是**無作為**。真正的雙跑代價只是多一份帶時戳的 JSON。

8. **`ShowMyError` 會阻塞(v2 新發現,V2-7)**:`note.cpp:833` `fNote->ShowModal()`,`:895` 才 return。所以放棄動作**一次答覆重跑一次**,不是每 tick 重跑;B3 仍然成立(操作員連按 RETRY = 連續重量級快照),而房規的解法已存在:`csystem.cpp:184-187` —— 答覆不是 RETRY 就**消費掉 latch**,註解自己寫「re-popping the same alarm every tick would trap them in a loop」。

9. **WAR0962 在現場會**先**於 D4 觸發(v2 新發現,V2-9)**:`ServiceHandshake` 的老化要求 `Handshake[si] ∈ {CALLED,PREP,READY}`(`uAgvStation.cpp:696-697`)。京元現況是 272 發得出去(link SELECTED)、host 不回 `START_AGV` → 站停在 `AGV_CALLED` → 老化**會**跑 → 300 s 就 `WAR0962`,遠早於 D4 的 1800 s。而 `WAR0962` **只給 K_RETRY**,清不掉 D4 的 latch → 批仍然結不了。所以 D4 自己的警報**必須存在**,且抑制窗必須有結構性上界。

10. **SKIP 會踩到 skip-IC-count 陷阱(v2 新發現,V2-1)**:`note.cpp:871-893` —— 只要 `ReturnCode==K_SKIP && GeneralSetting.bAskSkipICCount`,就彈**第二個**模態鍵盤問「How many ICs were taken out of the tray?」並發 **CEID 78 + SVID 37010(Jam Skip IC Count)**,而該段註解明寫 "Deliberately NOT gated on the alarm being a JAM* code"。京元的 host 正是用 CEID 78 對帳庫存。目前 `bAskSkipICCount` 預設 false(`GeneralSetting.cpp:119/257`,`system/General.ini:26` = 0),但這是客戶可開的 opt-in。

### 1.2 2026-09-04 現場證據

v1 §1.2 全部逐筆核對過、全部成立,不重複。三個要點:CEID 42 → CEID 8 相隔 **28 ms**;六顆 `SnAutoX_InputHasTray` 兩張快照皆 Live=0(0901 的收尾叫車從未成立過);唯一完成的一趟交接 **57.367 s**。

### 1.3 本案在推翻什麼

`csystem.cpp:1922-1926` 與 `docs/plan/loader-overcount-nostop-cleanout-lotend-plan-20260721.md:306` 的「輸出車留盤不擋 finish」是刻意裁定。本案推翻它,理由就是 0904 那一批:**批在 43.981 秒內結掉,三台車帶著這一批的料跨到下一批**。這段必須留檔。

---

## 2. 使用者定案(2026-09-07)+ 與既有裁定的關係

N1–N8 與 v1 完全相同(此處不重排,見 v1 §2):全部叫走 / 不得在交接前結批或須有明確有界放棄 / 沿用 272→START_AGV→273/274 / 六站不連坐 / 不發 35/36/37/148/149/150 / 不設 Lot 閘 / 僅 `bUseAMR` / 只動出料側。

> **N1 與 N5 的張力**:叫車範圍變寬 = 「沒有離散 Full CEID 陪同的 CEID 272」變多。host 只能靠「沒有離散 Full 事件」+ `SVID 38219` bitmap 分辨。若京元要正面分辨,那是**私有段 9001–9099 的新 CEID 申請**,不是挪用 35–150。

---

## 3. 目標行為(僅 `bUseAMR==1`)

v2 把 v1 混在一起的「宣告」與「關帳」**拆成兩個動作**,這是 B2 的核心修正。

```
Run_CleanOut 排空中
 ├─ 武裝(每站獨立、只升不降)
 │    Loader 兩側清機完成 && SortArm 清機完成 && 該站 drain latch(case 7000)
 │    && Car[a].iTrayCount > 0
 │        → LATCH bHandoverOwed[a] = true
 │    (case 7000 一次設六站 -> 實務上六站同 tick 武裝, 見 V2-3)
 ├─ 叫車(latch 短路,握手全程不被撤銷)
 │    PollAndCall 下一 tick:SetAmrLock + CEID 272(單站 bitmap)
 │    → host START_AGV(AUTOn="Action")→ 273 → 274 → ClearAmrCar → 降旗 → IDLE
 ├─ 六站併發;TrayArm / Empty / Color 同時照常排空(不被 AMR 等待凍住)
 ├─ 全部降旗 + 原有六項 cascade + 機內無 IC
 │        → CheckCleanOutFinish() == true → CEID 42 → Lot End(CEID 8)→ Run_Normal + SoftStop
 └─ 放棄(三條件,任一成立即進入「宣告」)
      (a) 完全叫不出車:!IsAmrCallableNow() 且 **每一個欠交接站 Handshake==AGV_IDLE**
          且該狀態連續成立 iAgvTimeoutSec 秒            <-- B4 修正:不再被 1-tick 抖動觸發
      (b) episode deadline(iCleanOutHandoverWaitSec)到期
      (c) RunMode 離開 Run_CleanOut 而仍欠交接(Start 觸發的 HOME)

      ┌ 宣告(ANNOUNCE)—— **不設任何前置條件**              <-- B2 修正
      │   State Record 快照(**每 episode 一次**)+ EventLog(帶 RunMode / bCleanOut /
      │   base 判定與第一個不成立的項 / 每站盤數・IC 數・CarID・sensor 三態・Handshake)
      │   + 一則聚合警報 WAR0964
      │     K_RETRY 只在(預算內 && RunMode==Run_CleanOut && IsAmrCallableNow())才提供
      └ 關帳(CLOSE)—— **只有這一步要求 CheckCleanOutFinishBase()==true**
          RETRY → 逐站 RetryStation + 重新武裝 deadline;latch 保留
          SKIP  → 先寫帳 → 逐站 ReleaseStationByOperator(reason) + 歸零協調器 SVID
                  + ClearAmrCar + 降旗
                  base 成立 → CloseCleanOutForAmr()(CEID 42 → Lot End → Run_Normal)
                  base 不成立 → 留在 Run_CleanOut,EventLog 明寫「交接已放棄,清機仍在等 <原因>」
                              由既有 :1910 dispatcher 收斂後自己關帳
          其他/沒按 → 重新武裝 deadline 再問一次(**不再拍快照**),機台停著等人
```

**「批不會結束直到車被收走」的適用範圍(必讀)**:只適用於上面這條**自動**路徑。`btnLotEndClick`(`main.cpp:3251-3258`)與 SECS `CLEAR_LOT_INFO`(`SecsGem/uHGemHT160.cpp:2387-2423`)**維持立即結批**。理由見 §5 第 3 條。

---

## 4. 變更點

### 4.A `aAuto1To6.h` / `aAuto1To6.cpp` —— 車有盤述詞 + 欠交接 latch

**A-1 新成員** `bool bHandoverOwed[AUTO_STATION_COUNT];`,宣告位置緊鄰既有的 `Car[]` / `iAmrDeviceCount[]` 群組。

**A-2 新述詞** `bool TAutoModule::IsOutputCarHoldingTraysForAmr(int Index)`

```cpp
if(Index<0 || Index>=AUTO_STATION_COUNT) return false;
if(GeneralSetting.bUseAMR==false)        return false;   //AMR off -> operator owns the car
return (Car[Index].iTrayCount > 0);
```

- **刻意沒有 `IsSoftSimulate()` 分支**:累加點的閘是**模式**(`aAuto1To6.cpp:848` `if(GeneralSetting.bUseAMR)`,遞增在 `:861`),不是模擬。真機停在 DUMMY 檔案模式時 `IsSoftSimulate()` 也是 true,選帳而不選 sensor 讓兩者行為一致。**筆電上的後果見 §6.3 N1,不是「握手會自己走完」。**
- **求值時機是關鍵**:遞增發生在後段升位(rear → working promotion),生產中會多算手上那一盤;但排空梯 case 4000/5000/6000 把 working 盤堆進車、case 7000 才 latch(`:1219`),所以**在 drain latch 那一刻帳是精確的**。求值點必須綁 `IsStationCleanOutFinish(a)==true`,不可挪到 Lot End。

**A-3 新 latch 存取**:`IsHandoverOwed(int)`、`SetHandoverOwed(int,bool)`、`IsAnyHandoverOwed()`、`int GetHandoverOwedMask()`(六站 bitmask,供閘門服務偵測「集合有沒有新增成員」)、`AnsiString DescribeHandoverOwedForOperator()`(欠交接站名 + `trays=` / `ICs=` / `CarID=` + `front/full/rear` 三態 + `hs=`)。

**A-4 latch 生命週期(位置有硬要求)**

- `InitialFlag`:`bHandoverOwed[Index]=false;` 必須寫在 **`:100-101` 的 `if(bKeepMaterial) continue;` 之下**,和 `Car[Index].Clear()`(`:120-121`)同一段。
  - **上面不行**:`bCleanOutFinish=false`(`:93`)與 `bAmrLocked[Index]=false`(`:96`)都在 early-out **之上**,所以每一次 HOME(含 keep-material)都會清掉它們;latch 若放上面會被每次 HOME 靜默取消。
  - **下面才對**:latch 與它的證據(帳)同生同死。冷 `InitialAllTask()` 兩者一起清 —— 而關帳分支自己就呼叫冷的 `InitialAllTask()`(`:1929`),所以 latch 不會活過結批。
  - **keep-material HOME 之後**:latch 與帳都在,`bAmrLocked` 被 `:96` 清掉但 `AgvCoord.ReassertLocks()`(`database.cpp:91` = `InitialAllTask` 尾端)依 `Handshake` 重新耦合(`uAgvStation.cpp` `ReassertLocks` 本體)。搭配 §4.B-1 的短路,叫車在 HOME 前後**都不會被撤銷**。
- `ClearAmrCar`(`:1712-1720`):在 `Car[Index].Clear()` 旁加 `bHandoverOwed[Index]=false;` —— CEID 274 成立就是交接完成。
- 操作員 SKIP:由 §4.C-2 呼叫 `SetHandoverOwed(a,false)`。

**A-5 註解更正(四處自述錯誤,本案一併修掉)**

| 位置 | 現況 | 應為 |
|---|---|---|
| `aAuto1To6.cpp:1597-1600` | `this Auto output stack still holds at least one tray` | 說清楚它是 **front transfer position** sensor,並指向 `IsOutputCarHoldingTraysForAmr` 才是「車上有盤」 |
| `aAuto1To6.h:200` / `aAuto1To6.cpp:1693-1696` | `IsAmrTaken ... (sim=true; real sensor TBD)` | `SnAutoX_InputEnd` 已於 20260623 接上(`:1705-1706`),不是 TBD;並註明**尚未經現場逐站量測** |
| `aAuto1To6.cpp:1963-1964` | 把遞增點寫成 `DoDischargeTray` | 實際是 `DoFeedTray`(`:848-861`) |

---

### 4.B `SecsGem/uAgvStation.h` / `uAgvStation.cpp`

**B-1 【修 B6】`IsCleanOutCollectDueForAmr` 改成 latch 短路,位置有硬要求**

```cpp
bool TAgvCoordinator::IsCleanOutCollectDueForAmr(int AutoIndex)
{
    if(GeneralSetting.bUseAMR==false)                          return false;   // :414-415 unchanged
    if(AutoModule==NULL || LoaderModule==NULL || SortArmModule==NULL) return false;   // :416-417 unchanged
    if(HSys.Sys.RunMode!=Run_CleanOut)                         return false;   // :418-419 unchanged
    //D4 : OWED IS A LATCH AND SHORT-CIRCUITS HERE. The three finish gates below are LIVE
    //and would revoke a call MID-HANDSHAKE : State[].bCleanOutFinish is cleared by
    //SetRearHasTrayFromTrayArm (aAuto1To6.cpp:665) on any late TrayArm delivery, and by
    //InitialFlag (aAuto1To6.cpp:93) on EVERY home - that line sits ABOVE the bKeepMaterial
    //early-out at :100-101. A revoked bFull hits the release branch (:540-545), hands the
    //lock back while the AMR may be under the car, and the drain-raise guard
    //(aAuto1To6.cpp:1158-1159) then stops protecting it.
    if(AutoModule->IsHandoverOwed(AutoIndex))
        return true;
    if(LoaderModule->IsAllCleanOutFinish()==false)             return false;   // :420-421 unchanged
    if(SortArmModule->IsCleanOutFinish()==false)               return false;   // :422-423 unchanged
    if(AutoModule->IsStationCleanOutFinish(AutoIndex)==false)  return false;   // :424-425 unchanged
    return AutoModule->IsFrontHasTrayForAmr(AutoIndex);                        // :426 unchanged
}
```

- **為什麼不照 critic 的 P6 插在五閘之前(V2-8)**:`PollAndCall` 的 RunMode 閘是 `Run_Normal || Run_CleanOut`(`:489-490`),所以短路若跨過 RunMode 閘,`RunMode` 一旦回到 `Run_Normal`(condition (c) 的前提),`bCollect` 仍為真 → `SetAmrLock` + **在恢復生產的 Run_Normal 發一則幻影 CEID 272**,並把 TrayArm 對該 Auto 的供料鎖住,傷到剛開的新批。留下 RunMode 閘,「離開收集窗」交給放棄條件 (c) 處理。
- **保留最後一項的 OR 語意**:短路是「加一條路」,`:426` 原句不動 → 本次改動是 0901 行為的**嚴格超集**,不可能回退 0831 Auto6 那個 `front=1 full=0` 的案例。
- 「沒有 Lot 閘」那段大註解(`:403-411`)**一字不動**。
- **不新增 handshake 狀態**;`AGV_FINISH`(`uAgvStation.h:26`)仍是死的,本案不喚醒。

**B-2 新 accessor** `bool TAgvCoordinator::IsAmrCallableNow()` = `return (HGem!=NULL && HGem->IsSelected());`
放在 `uAgvStation.cpp`(該檔 `:19` 已 include `uHGemEquipment.h`;`csystem.cpp` **沒有** include 它,`HGem` 在那裡不可見)。這一個布林涵蓋三個「叫不出車」情境:SECS 付費功能關(`UsecegemMainFrom.cpp:33-37` 提前 return)、`[SECS]Enable=0`、link 從未 SELECTED(`PollAndCall` 走 `:445-469` 直接 return)。

**B-3 【修 B2 / N3】看門狗抑制改成「由 D4 計時器結構性封頂」**

- 新增公開陣列 `unsigned char HandoverSuppress[AGV_STATION_COUNT];`(與 `TimeoutPending`(`uAgvStation.h:104`)同慣例,建構子與 `Reset()` 一併歸零)。**只由 §4.C-2 第 4 步每 tick 覆寫**,協調器自己不寫。
- `ServiceHandshake` 的唯一 latch 點(`:699`)加一項:

```cpp
if(++ShortageDebounce[si] > GeneralSetting.iAgvTimeoutSec
   && TimeoutPending[si]==0 && HandoverSuppress[si]==0)
```

`ShortageDebounce[si]` 照舊累加(所以抑制解除時它不是從 0 重新等 300 s,而是立即補上警報),`else` 分支(`:707-711`)一字不動。

- **為什麼必要**:`WAR0962` 走 `ShowNoteAlarm` → `DecStopAllMotor(); SystemStart=false;`(`note.cpp:809-810`),而 `ServiceAgvTimeoutAlarm` 一次 sweep 只出一則(`csystem.cpp:191`)。六站同時逾時 = 六個接連的停機模態。
- **為什麼這次安全(v1 的論證不充分之處)**:v1 把抑制掛在 `IsHandoverOwed(a)` 上,而 latch 只能由 274 / SKIP / 冷 init 降旗 → 抑制窗與 latch 同壽 = **無界**。v2 的抑制條件是

```
HandoverSuppress[a+3] = (bUseAMR && RunMode==Run_CleanOut
                         && IsHandoverOwed(a)
                         && IsOutputCarFullForAmr(a)==false      // 真滿車永不抑制
                         && Handshake[a+3]==AGV_CALLED           // 只抑制「host 沒回」
                         && bHandoverSnapshotDone==false         // 一宣告就放開
                         && bDeadlineExpired==false)             // 一到期就放開
```

四個項裡有兩個是 D4 自己的有界計時狀態 → **抑制窗長度上界 = `iCleanOutHandoverWaitSec`,之後 `WAR0962` 與 `WAR0964` 兩條逃生門同時活著**。這比 critic 的 P9(`&& CheckCleanOutFinishBase()`)更緊:base 為 false 恰恰是排空卡住的時候,那時把 962 放回來是對的,但用「D4 計時器還在跑」表達,不需要引入 base 這個 D4 自己就能壓成 false 的項(V2-2)。
- **PREP / READY 不抑制**:那是「AMR 實體到位但機構卡住」(R13,`IsAmrTaken` 因 `SnAutoX_InputEnd` 不轉 OFF 而永假),原本 300 s 就有 `WAR0962`,不該被拉長到 1800 s。

**B-4 【修 N6】`ReleaseStationByOperator` 加 reason 多載**

`uAgvStation.cpp:299-300` 硬編 `"AGV: WAR0963 operator confirmed AMR left P…"`。新增
`void ReleaseStationByOperator(int si, AnsiString sReason);`,把現行本體搬進去、把那句改成 `"AGV: "+sReason+" P"+…`;現行 1 參數版改為呼叫它並傳 `"WAR0963 operator confirmed AMR left"` → **既有行為 byte 不變**。D4 傳 `"WAR0964 handover given up - operator will remove the cars"`。
同時在 D4 的 SKEP 支線把 `TrayCount[si]=0; DeviceCount[si]=0;`(比照 274 路徑 `:684-685`)—— 否則車實體離開後 host 仍讀得到盤數/IC 數(N7)。

---

### 4.C `csystem.cpp` —— 閘、閘門服務、關帳抽函式

**C-1 【修 B2 的一半】`CheckCleanOutFinish()` 拆兩段**(現行 `:2031-2055`)

- 新增 `static bool CheckCleanOutFinishBase()` = 現行 `:2040-2054` 那七項,**一字不動搬進去**。
- 新增 `static AnsiString DescribeCleanOutBaseBlockerForOperator()`:回傳第一個不成立的項名(`"Loader"` / `"SortArm"` / `"Auto"` / `"TrayArm"` / `"Empty"` / `"Color"` / `"IC under machine"`)。**這是 §4.C-2 宣告訊息裡「base 為何不成立」那一句的來源**,沒有它,B2 修好之後操作員還是看不懂為什麼放棄了卻沒結批。
- `CheckCleanOutFinish()` 變成:

```cpp
if(CheckCleanOutFinishBase()==false) return false;
if(GeneralSetting.bUseAMR && AutoModule!=NULL && AutoModule->IsAnyHandoverOwed())
    return false;                       // D4 : output cars not handed over yet
return true;
```

- **為什麼放這裡,不放 `TAutoModule::IsAllCleanOutFinish()`**:`aTrayArm.cpp:408-411` 要求 `AutoModule->IsAllCleanOutFinish()`;`aEmpty.cpp:384-385` 與 `aColor.cpp:487-488` 又由 TrayArm 的 finish 推導自己的 `bLotFinish`。放進 Auto 述詞 = 讓三個沒有東西要交接的模組陪著等 AMR。(已複驗:`Empty`/`Color` 的 `if(bAmrLocked) break;` 讀的是**自己站**的鎖,不是 Auto 的,所以 Auto 被鎖不會凍住它們的排空。)

**C-2 新服務** `static void ServiceCleanOutHandoverGate()`

呼叫點:`MainProc` 內,緊接 `ServiceAgvTimeoutAlarm(); ServiceAgvLinkLostAlarm();`(`csystem.cpp:423-424`)之後。放主迴圈而不是協調器的兩個理由不變:①任何模態只能從主迴圈彈;②SECS 付費功能關掉時 `ServiceAgv` 根本不跑,武裝與計時必須活在跟 SECS 無關的地方。

檔案級靜態狀態(全部在 `csystem.cpp`,BCB6 無 lambda/無 `enum class`,純 POD):

```cpp
static HTimer HandoverDeadline;         // episode deadline
static HTimer HandoverUncallable;       // give-up (a) debounce, iAgvTimeoutSec
static int    iHandoverRetryUsed  = 0;
static bool   bHandoverSnapshotDone = false;   // announce-once for the HEAVY snapshot
static int    iHandoverOwedMask   = 0;
```

每 tick 七步:

1. **前置否決**:`bUseAMR==false` → `ResetHandoverGateState()`(含把六個 `HandoverSuppress[]` 歸零)後 return。`RunMode==Run_Home` → return(**保留狀態、不求值**,比照 `:151-152`、`:214-215`)。`fNote!=NULL && fNote->fShow` → return(比照 `:164-165`,避免 `ShowModal` 的訊息泵重入本 sweep)。`AutoModule/LoaderModule/SortArmModule` 任一 NULL → return。

2. **武裝(只升不降)**:`RunMode==Run_CleanOut` 時逐站求值
`LoaderModule->IsAllCleanOutFinish() && SortArmModule->IsCleanOutFinish() && AutoModule->IsStationCleanOutFinish(a) && AutoModule->IsOutputCarHoldingTraysForAmr(a)`
→ `SetHandoverOwed(a,true)` + 升旗那一次寫一行 `RecordProcess`(帶 trays / ICs / CarID)。降旗只有 CEID 274 / SKIP / 冷 init 三條路。

3. **【修 N2 / V2-3】episode deadline**:`int m = GetHandoverOwedMask();`
   - `m==0` → `ResetHandoverGateState()`;return。
   - `(m & ~iHandoverOwedMask)!=0`(**有新站加入**)→ `HandoverDeadline.Clear(); HandoverDeadline.SetMS(GeneralSetting.iCleanOutHandoverWaitSec*1000); HandoverDeadline.On();`
   - `iHandoverOwedMask = m;`
   `bool bDeadlineExpired = HandoverDeadline.Off();`(**一 tick 只求值一次**,因為 `HTimer::Off()` 逾時後恆真且會把 `InUsed` 設 false,`HTimer.cpp:48-65`)
   不做 per-station deadline:case 7000(`aAuto1To6.cpp:1210-1219`)一次設六站、case 4000 是六站連鎖閘(`:1166`),六站必然同 tick 武裝(V2-3)。

4. **發佈抑制旗**:逐站依 §4.B-3 的五項公式寫 `AgvCoord.HandoverSuppress[a+3]`;非欠交接站一律寫 0。

5. **放棄條件(三者取 OR,宣告**不**設任何前置)**
   - **(a)【修 B4】** `bool bIdleAll=true;` 對每個欠交接站檢查 `AgvCoord.Handshake[a+3]==AGV_IDLE`,任一不是就 `bIdleAll=false`。
     條件成立的**上升緣**才 `HandoverUncallable.SetMS(GeneralSetting.iAgvTimeoutSec*1000); On();`;條件掉了就 `Clear()`。給的分是 `HandoverUncallable.Off()`。
     → 這同時關掉兩個洞:①**link 抖一下不再立即放棄**,遵守 `uAgvStation.cpp:445-469` 的 20260819 裁定(「a dropped HSMS link is NOT evidence that the AMR has left」),已進 CALLED/PREP/READY 的站一律不走這條、交給 `WAR0963`;②SECS 永久關閉的機台不再每 tick 重觸發,而是等一個 `iAgvTimeoutSec` 窗。
     **不用 tick 計數**:`ServiceCleanOutHandoverGate` 跑在 MainProc 節拍上(不是 1 Hz),數 tick 不等於數秒;用 `HTimer` 是房規。
   - **(b)** `bDeadlineExpired`。
   - **(c)** `RunMode!=Run_CleanOut && IsAnyHandoverOwed()`。蓋住 Start 觸發的 HOME:`csystem.cpp:2368-2384`(main.cpp)只在 `fAllMotorHome==false` 時設 `bHomeByStart=true`,而 `csystem.cpp:1852-1856` 會把 `RunMode` 拉回 `Run_Normal` **且不清 `HSys.Sys.bCleanOut`** —— 正是 `csystem.cpp:1588-1596` 警告過的 split。**把這條靜默丟掉就是使用者禁止的無聲停機。**

6. **【修 B2 / B3】宣告(ANNOUNCE)—— 無前置**
   ```
   a. 先把 AgvCoord.HandoverSuppress[] 全部歸零(逃生門立刻放回來)
   b. if(bHandoverSnapshotDone==false)
        { bHandoverSnapshotDone=true;                       // 先設旗再做重活, 防 ShowModal 重入
          gStateRecord->TriggerSnapshot("CleanOutHandoverGiveUp"); }
   c. g_EventLog.Log(...) + RecordProcess(...) 一律寫(每次宣告一行, 但快照只一次):
        which=(a|b|c) / RunMode / bCleanOut / base=0|1 + DescribeCleanOutBaseBlockerForOperator()
        / DescribeHandoverOwedForOperator() / retryUsed
   d. int iKeys = K_SKIP;
      if(iHandoverRetryUsed < GeneralSetting.iCleanOutHandoverRetry
         && HSys.Sys.RunMode==Run_CleanOut
         && AgvCoord.IsAmrCallableNow())
          iKeys |= K_RETRY;                                  // 修 N4
      int iKey = ShowMyError("WAR0964",
              LangT("AMR did not collect the Auto output cars")+" : "
              + AutoModule->DescribeHandoverOwedForOperator(), iKeys);
   ```
   - **快照必須 once-per-episode**:`cStateRecordHT160.cpp:1755-1832` 的 `TriggerSnapshot` 做全 IO registry 傾印 + `CaptureSecsLog/EventLog/ProductionLog/SoterOutput`(**整天的 log**)+ `CompressFolder` 7-Zip,**同步跑在機控執行緒**;該檔自己的註解 `:478-479` 寫明「re-copies the WHOLE day's … every time, so same-day growth is **quadratic** in the snapshot count」。
   - `bHandoverSnapshotDone` 只由「欠交接集合變空」或冷 `InitialAllTask()`(經第 3 步的 `m==0` 重設)清除。
   - `K_RETRY|K_SKIP` **不需要任何 UI 改動**:`note.cpp:340-348` 依 `KeyCode` bitmask 顯示按鈕,`:442-469` 的 `KeyCodeByIndex[6]` 已涵蓋兩者。(`csystem.cpp:205-206` 拒絕的是「新增第七顆鍵」,不是用現有的兩顆。)
   - **【修 N4】**`RetryStation`(`:316-328`)只把站設回 IDLE,重叫靠 `PollAndCall`,而後者需要 `RunMode==Run_CleanOut`(`:418-419`)+ SELECTED(`:445-469`)。兩者不成立時 `K_RETRY` 是**保證無效**的按鍵,不得提供。

7. **答覆處理(關帳 CLOSE)**
   - **`K_RETRY`**:`iHandoverRetryUsed++`;逐個欠交接站 `AgvCoord.RetryStation(a+3)`;`HandoverDeadline.Clear(); SetMS(...); On();`;`HandoverUncallable.Clear();`;**latch 保留**;`bHandoverSnapshotDone` 保持 true(不再拍第二顆快照)。
     **必須有預算** —— 現行的無界 RETRY 已被現場證明不收斂(`EventLog/HT160S_2026_09_03.csv`:Auto2 在 18 分 56 秒內循環五次零進展)。
   - **`K_SKIP`**:
     1. **先寫帳(修 B4)**:對每個欠交接站把 `Car[].iTrayCount` / `GetAmrDeviceCount(a)` / `Car[].CarID` 寫進 `g_EventLog` 與 `RecordProcess`(快照已在第 6 步拍過)。`ClearAmrCar` 會經 `InitAutoCarStack`(`:1373-1377`)把 `iAmrDeviceCount[Index]` 一併歸零,而這條路上**沒有 CEID 274 帶走那組數字**。
     2. 逐站 `AgvCoord.ReleaseStationByOperator(a+3, "WAR0964 handover given up …")`(§4.B-4)+ `AgvCoord.TrayCount[a+3]=0; DeviceCount[a+3]=0;`
        **`ReleaseStationByOperator` 不可省**:它把 `Handshake[si]` 強制回 `AGV_IDLE`,否則接下來 `InitialAllTask()` 尾端的 `AgvCoord.ReassertLocks()`(`database.cpp:91`)會依 `Handshake` 把鎖重新耦合 → 孤兒鎖。
     3. `AutoModule->ClearAmrCar(a)` → `SetHandoverOwed(a,false)`。
     4. **`CheckCleanOutFinishBase()==true`** → `EmitCleanOutOK()` → `CloseCleanOutForAmr("auto (CleanOut finish, AMR handover skipped)")` → `ChangeRunMode(Run_Normal); SoftStop=true;`
        **`false`** → **不**發 CEID 42、**不**關帳;`RecordProcess("CleanOut handover GIVEN UP; clean-out still waiting on <blocker> - press Start to resume the drain")`,留在 `Run_CleanOut`,由既有 `:1910` dispatcher 收斂後自己關帳。
        (機台此刻是停著的 —— `ShowNoteAlarm` 已 `SystemStart=false`。`ProcessStartMode`(`csystem.cpp:1339-1348`)只把 `SystemStart` 設回 true、**不動 `RunMode`**,且 Start 只在 `fAllMotorHome==false` 時才觸發 HOME(`main.cpp:2368-2384`),所以「按 Start 續排空」是可用的復原路徑,必須寫進警報文字與手冊。)
     5. **【修 V2-1】**`bAskSkipICCount` 若為真,`note.cpp:871-893` 會再彈一個鍵盤模態並發 CEID 78 + SVID 37010。見 §4.F 與 **OD-5**。
   - **不是 RETRY 也不是 SKIP(含 ReturnCode=0)**:寫一行 `RecordProcess`,`HandoverDeadline` 重新武裝一輪,**不再拍快照**,下一個到期再問一次。**絕不**消費 latch、**絕不**靜默放行 —— 這是「不得無聲停止」與「不得無界」之間唯一站得住的折衷:機台停著、有記錄、而且會再問。

**C-3 【修 B1】抽出的只有三行** `static void CloseCleanOutForAmr(const char *pSource)`

本體 = 現行 `csystem.cpp:1927-1929` **原封搬移**,一行不多:

```cpp
HSys.Sys.bCleanOut=false;
if(fMain!=NULL) fMain->DoLotEndProcess(pSource);
InitialAllTask();
```

- **`:1914 EmitCleanOutOK()` 與 `:1952-1953 ChangeRunMode(Run_Normal); SoftStop=true;` 一律留在原處。** 它們是整個 `if(CheckCleanOutFinish())`(`:1910-1954`)的**共用前後綴**,`else` 非 AMR 分支(`:1931-1951`)同樣依賴它們。v1 §4.C-3「本體 = `:1914` + `:1927-1929` + `:1952-1953` 原封搬移」照做的後果就是 B1:AMR=0 從此不發 CEID 42,且 `RunMode` 停在 `Run_CleanOut` 而 `bCleanOut` 已被 `:1938` 清成 false → 下一 tick `CheckCleanOutFinish()` 仍成立 → `ShowSystemError(HSys.Sen.SnFKCleanOut.Name, K_SKIP, 0)`(`:1945`)每 tick 重彈,機台永久卡在清機完成模態。
- 因此**放棄支線必須自己補上前後綴**(§4.C-2 第 7 步 4.)。
- **抽函式而不是複製,是硬要求**:`DoLotEndProcess`(`main.cpp:3260-3372`)的副作用不可重跑也不可換序(清單見 §4.G)。
- **恰好一次的證明**:兩條路徑都在同一函式把 `bCleanOut=false`,呼叫端隨即 `ChangeRunMode(Run_Normal)`;放棄支線走完後 `IsAnyHandoverOwed()` 已為 false 且 `RunMode!=Run_CleanOut`,`:1908` 的整段分支再也進不去 → `CEID 42` 與 Lot End 各一次。
  (順帶更正 v1 R9 的理由:`ArchiveWorkOrderToLotStory` 有 `GetLotCount()<=0` 早退(`main.cpp:3609-3610`),第二次是**無作為**,不是「歸檔一份已清空的工單」。恰好一次的要求仍然成立,理由改為「避免重複的帶時戳 JSON + 不可換序的 15 個副作用」。)

**reviewer 的單一 grep 不變量(§8 S2 / S5 都適用)**

```
git diff -- HT160S_Program_BCB_V1.0.0.0/csystem.cpp | grep "^[-+]" \
  | grep -E "EmitCleanOutOK|SnFKCleanOut|FreezeProductInfoAtLotEnd|GetCalculateUPH|LotEndTime|ChangeRunMode\(Run_Normal\)|SoftStop=true"
```

**在 S2(純重構)必須輸出空**。在 S5 之後,唯一允許出現的 `+` 行是新函式 `ServiceCleanOutHandoverGate()` 內部的那三句(`EmitCleanOutOK` / `ChangeRunMode(Run_Normal)` / `SoftStop=true`);`:1908-1955` 這段區間內的每一行都只能以 context(空白前綴)出現,**任何一行帶 `-` 或 `+` 就是 B1 回歸**。

---

### 4.D `GeneralSetting.h` / `GeneralSetting.cpp` —— 兩個新 key(含上限)

比照 `iAgvTimeoutSec` 的四處寫法(建構子 `:141`、`ReadInteger` `:281`、clamp `:308`、`WriteInteger` `:376`):

| 成員 | ini | 預設 | 下限 | **上限** | 用途 |
|---|---|---|---|---|---|
| `iCleanOutHandoverWaitSec` | `[AGV] CleanOutHandoverWaitSec` | 1800 | 60 | **7200** | 一個 episode 的交接 deadline |
| `iCleanOutHandoverRetry` | `[AGV] CleanOutHandoverRetry` | 2 | 0 | **5** | `WAR0964` 的 RETRY 次數預算 |

- **【修 B3 / N5】上限是新的**:`GeneralSetting.cpp:305-308` 現行只有下限。理由:每一次 RETRY 買走一個完整的 deadline 窗,現場填 50 就是 50 × 1800 s 的停機窗;而 `iHandoverRetryUsed` 是 **episode-scoped**(第 3 步 `m==0` 時歸零),不是 function-static 永不重置(N5)。上限 7200 同理 —— 避免 ini 打錯字變成事實上的無限等待。
- **1800 的依據**:實測一趟 57.367 s(`SECSGEM_TextLog_15.txt:1140` → `:1206`),六趟純交接 `57.367 × 6 = 344.202 s ≈ 5 分 44 秒`;1800 s 給約 5 倍餘裕吸收 AMR 移行與排隊。
- **不重用 `AgvTimeoutSec`**:那把鑰匙的語意是「**單一**握手沒回應」(現場值 300,`system/General.ini:132`),而且 v2 的放棄條件 (a) 正是**沿用**它做去抖,兩個語意會打架。不過新開一把鑰匙是對 20260721「AMR 逾時統一成一把鑰匙」裁定的逆轉 → 併入 **OD-3**。
- **不可挪用 `[AGV] AmrFullWaitSec=60`**(`system/General.ini:130`):全樹 grep 零消費者,是 20260627 廢案的孤兒。清理它是另一件事。

---

### 4.E 警報註冊 —— `WAR0964`

- `database.cpp`:比照 `WAR0962`(`:1034-1038`)/ `WAR0963`(`:1046-1050`)的 standalone 慣例,在 `:1050` 之後加一段
  `cd="WAR0964", mg="AMR did not collect the Auto output cars - remove them by hand then SKIP"`,寫入 `mapAlarmCodeList` + `mapNameToAlarm`,`eMessageErr` / `"pn_System"` 同 family。
- `system/AlarmList.csv`:在 **`:478`(`WAR0963`)之後、`:479`(`WAR0970`)之前**插一列,`AlarmType=1`(同 AMR family),總列數 **486 → 487**(現檔 487 行 = 1 標題 + 486 列)。
- ALID 自動跟進:`ComputeAlarmAlid`(`SecsGem/UsecegemMainFrom.cpp:185-298`)class 2 = `WAR` 前綴(`:195`),digit tail `"0964"`(4 位、值 964 < 10000,通過 `:280-283`)→ **ALID = 2 × 100000000 + 964 = 200000964**。
- **為什麼不用 per-station 碼**(0716 §4E-2 的 `AMR0101..AMR0601`):①全樹與 `AlarmList.csv` 都沒有 `AMR01xx`;②`MES14xx` 段位已被 Color 佔用(`system/AlarmList.csv:455-462`);③本案要的就是**一則聚合警報**,per-station 碼會重新製造六個模態。

---

### 4.F 【修 V2-1】SKIP 的 skip-IC-count / CEID 78 防護

`note.cpp:871-893`:`ReturnCode==K_SKIP && GeneralSetting.bAskSkipICCount` → 彈第二個模態鍵盤問「How many ICs were taken out of the tray?」→ `ReportSkipICCount()` 發 **CEID 78 + SVID 37010**。該段註解明寫刻意不按 JAM 碼設閘。

對 `WAR0964` 這是**兩個獨立的傷害**:①無人線上出現第二個阻塞模態,而 D4 存在的唯一理由就是無人線;②京元用 CEID 78 對帳庫存 —— 交接放棄時**沒有任何 IC 被從盤裡取出**,發這則事件是**假的庫存沖銷**。

最小、可複查的修法(**列為 OD-5,因為它動到共用的 SKIP 路徑且是客戶可見的 SECS 行為**):在 `:871` 的條件加一項「本警報屬 AMR 物流 family(`WAR0962/0963/0964`)則不問」。今日 0962/0963 只提供 `K_RETRY`,永遠到不了這個分支,所以**唯一受影響的成員就是 0964**,行為改動面積為零。

### 4.G 【修 B5】Lot End 兩段化 —— 哪些必須落地、哪些可以等

`DoLotEndProcess`(`main.cpp:3260-3372`)的 15 個副作用,按「斷電是否遺失客戶交付品」分三類:

| 類 | 內容 | 斷電後果 | v2 處置 |
|---|---|---|---|
| **(i) 落地類** | `g_SoterOutput.OnLotEnd()`(`:3302`,per-unit CSV 的**寫檔點**與 FTP publish 排入點,`EnqueueLotPublish` 在 `cSoterOutput.cpp:525`;之前資料只在 RAM buckets)、`WriteLastDataIni()`(`:3304`) | **整批 Soter CSV + FTP 交件永久遺失**;累計計數回退 | **前移**(見下) |
| **(ii) 已在磁碟類** | `ArchiveWorkOrderToLotStory()`(`:3355`)、`LotRegistry.Clear()`(`:3356`)、`LotBinBinding`(`:3359-3360`)、`m_sActiveLot`/`edLotNo`(`:3363-3365`)、`DeleteFile(WorkOrder.json)`(`:3367`)、`ClearWorkOrderMeta()`(`:3370`) | **零遺失** —— `WorkOrder.json` 整段等待期間都還在磁碟上(只在 `:3367` 才刪),重開機會被還原 | **不動**,留在原時刻 |
| **(iii) 通知/顯示類** | `EventReport(SECS_EVENT.DoLotEnd)`(CEID 8,`:3313-3314`)、`NoteLotStartTime(false)`(`:3318`)、WebAPI 取消(`:3322-3347`)、`FreezeProductInfoAtLotEnd()`(`:3303`)、UPH/`TrayUphLog`(`:3292-3301`)、WhiteList 還原(`:3280-3287`) | 無交付品遺失 | **不動** —— CEID 8 的語意就是「這批結束了」,車還在等交接時提前發等於對 host 說謊,而且會與 CEID 42 的順序打架 |

**前移為什麼幾乎零成本(V2-5 / V2-6)**:

- **不可以**「提前呼叫 `DoLotEndProcess`」:它最上面三行就是 `SystemStart=false; SoftStop=true; bRunning=false`(`:3271-3274`),會凍結排空與 D4 閘門本身。
- **可以**在 csystem 直接呼叫落地兩項,因為 **`g_SoterOutput.OnLotEnd()` 自身冪等**(`cSoterOutput.cpp:380` 的 `if(!m_bActive) return;` + `:545` 的 `m_bActive=false`),而且 **`csystem.cpp:1935-1936` 的非 AMR 分支早就這樣直接呼叫 `FreezeProductInfoAtLotEnd()` + `g_SoterOutput.OnLotEnd()`**。`WriteLastDataIni()`(`cprod.cpp:275`)是純快照寫檔,已有三個呼叫點(`main.cpp:724/917/3304`),重複呼叫無副作用。
- 因此「前移」= 在 §4.C-2 第 2 步「本 episode 第一次有站升旗」那一刻,加兩行 + 一行 EventLog。`DoLotEndProcess` **一個字都不用改**,`:3302`/`:3304` 屆時分別是 no-op 與重寫一次。

**前移的三個已知副作用(必須寫進文件)**:
1. Soter CSV 的檔名時戳(`cSoterOutput.cpp:393-395` 的 `{Date}_{Time}`)會提前到「排空完成」而不是「批結束」,差距 = 交接時間。
2. 前移後 `m_bActive=false`;若操作員之後按 Start 恢復生產(keep-material HOME + `bCleanOut` split,`csystem.cpp:1588-1596`),`main.cpp:2363` 的 `g_SoterOutput.EnsureActive(ActiveLotID())` 會 `DoArm` **重新武裝**,續產的料寫進**另一支** CSV → 同一批被拆成兩檔(不是遺失)。
3. `DoArm` 會 `ClearPickupDir()`(`cSoterOutput.cpp:336`)清掉操作員取件夾。**FTP 上傳來源不受影響** —— publish 的 CSV 與 flag 路徑都指向永久 archive `sArch`(`:505`、`:516`),不是 pickup 夾。

**這是 OD-1,兩個分支都設計好了,由 owner 裁定。**

---

## 5. 不動項(明確排除)

1. **`bUseAMR==0` 的一切行為 byte 不變。** `IsCleanOutCollectDueForAmr` 首行否決(`uAgvStation.cpp:414-415`)、`IsOutputCarHoldingTraysForAmr` 首行否決(§4.A-2)、`CheckCleanOutFinish` 的 D4 項掛在 `bUseAMR` 之下、**非 AMR 收尾分支 `:1931-1951` 與共用前後綴 `:1914`/`:1952-1953` 一字不動**(§4.C-3 的 grep 不變量)。而且 `Car[].iTrayCount` 在該模式恆為 0(`aAuto1To6.cpp:848`),D4 述詞天然惰性 —— **請不要有人去「修」那道模式閘**。
2. **`CEID 35/36/37/148/149/150`** 只在真滿車發(`uAgvStation.cpp:536-537` `if(bTrueFull)`),D4 只走 `bCollect`。
3. **手動 Lot End 按鈕與 SECS `CLEAR_LOT_INFO` 不加等待閘。** 按鈕(`main.cpp:3251-3258`)是操作員的明示覆寫、也是 AMR 永不來時唯一的出路,而且它在 VCL click handler 裡,那個上下文表達不出「等」。host 路徑(`uHGemHT160.cpp:2387-2423`)已有兩道閘,多一條「車上有盤就拒」是**新的 HCACK 語意,客戶沒同意過**。
4. **`Run_Normal` 的滿車叫車、`WAR0962` 對真滿車與 PREP/READY 卡住的逃生門、`WAR0963` 的 link-lost 機制** —— 照舊。
5. **進料側 P1–P3 在 Clean Out 的凍結** —— 照舊(`uAgvStation.cpp:466-467`、`:482-483`、`:719-720`、`:806-810`)。
6. **`TAutoModule::IsAllCleanOutFinish()` 本體不動**(`aAuto1To6.cpp:1261-1294`)。
7. **`IsOutputCarFullForAmr()` 一字不動**(0901 裁定 ①):它有五個呼叫者,`GetTrayRequest` 用它**拒收**新盤。
8. **排空梯的六站連鎖閘不動**(`aAuto1To6.cpp:1158-1159`、`:1166`)。它是 V2-2 的成因,但改它等於改 Clean Out 的核心時序,不併入 D4;D4 的對策是「宣告不看 base」。
9. **不喚醒 `AGV_FINISH`**、不加第五個 handshake 狀態、**不加第七顆操作員鍵**(`csystem.cpp:205-206`)。
10. **`ServiceCleanOutResidualWatchdog`(`aAuto1To6.cpp:1296-1332`)不動**。
11. **`iAmrFeedWaitSec` / `iAmrHandshakeWaitSec` / `iAgvTimeoutSec` 三把既有鑰匙的值與語意不動**(`iAgvTimeoutSec` 被 (a) 條件**沿用**,不是重定義)。

---

## 6. 缺陷關閉逐條答覆

### 6.1 六個阻斷缺陷

- **B1** → §4.C-3。抽出範圍縮到 `:1927-1929` 三行;`:1914` 與 `:1952-1953` 留在原處;放棄支線自己補前後綴;§8 S2 的驗收條件改成「**AMR=0 組態的清機收尾逐行相同:CEID 42 照發、SKIP 後回 `Run_Normal` 且模態不重彈**」+ 單一 grep 不變量。
- **B2** → §4.C-2 第 6 步。**宣告不設任何前置**;只有關帳要求 `CheckCleanOutFinishBase()`;base 不成立時 SKIP 只清 latch/鎖/帳並把機台留在 `Run_CleanOut`;宣告訊息帶 `DescribeCleanOutBaseBlockerForOperator()`。抑制窗改成由 D4 自己的兩個有界計時狀態封頂(§4.B-3),不引入 base 這個 D4 自己就能壓成 false 的項(V2-2)。
- **B3** → §4.C-2 第 6 步 b. + §4.D。`bHandoverSnapshotDone` 每 episode 只拍一顆快照(先設旗再做重活,防 `ShowModal` 重入);`HandoverDeadline` 一 tick 只 `Off()` 一次、消費後 `Clear()` 再 `SetMS+On()`;RETRY 加上限 5、`iHandoverRetryUsed` episode-scoped。
- **B4** → §4.C-2 第 5 步 (a) + 第 7 步 `K_SKIP` 1.。(a) 加「所有欠交接站 `Handshake==AGV_IDLE`」+ `iAgvTimeoutSec` 去抖,已進握手的站交回 `WAR0963`;SKIP 先把 `iTrayCount`/`GetAmrDeviceCount`/`CarID` 寫進 EventLog 與快照再 `ClearAmrCar`,並同步歸零 `AgvCoord.TrayCount/DeviceCount`;**不發 CEID 274**(沒有交接發生)。§8 H4 的期望同步改成「link 掉不得立即放棄」。
- **B5** → §4.G + **OD-1**。落地類(Soter + lastdata)前移到「開始等待」那一刻,靠 `OnLotEnd()` 自身冪等 + csystem 既有的直呼慣例,`DoLotEndProcess` 零改動;CEID 8 / 歸檔 / 清帳留在原處,因為 `WorkOrder.json` 整段等待都還在磁碟上。R12 的「往後移數分鐘」改寫為「**CEID 8 最長後移 `1800 + 5×1800` 秒;不答則無限期**」。
- **B6** → §4.B-1。latch 改成短路 `return true`,插在 RunMode 閘**之後**、三個 finish 閘**之前**(V2-8 說明為何不能跨過 RunMode 閘);註解明寫兩條清除路徑(`aAuto1To6.cpp:665` 與 `:93`)與被撤銷的後果(`:540-545` → `:1158-1159` 失效)。

### 6.2 九個缺失逃生門

| # | 對象 | v2 的有界出口 |
|---|---|---|
| 1 | `CheckCleanOutFinish()` 的 D4 項 | **宣告不看 base**(B2),所以 base 為 false 時仍會出 `WAR0964`;SKIP 清 latch 後該項恆真為 false,`:1910` 由既有 cascade 收斂 |
| 2 | `bHandoverOwed[a]` latch | 三條降旗路仍是 274 / SKIP / 冷 init,但**到達 SKIP 的路現在是有界的**:(a) 有 `iAgvTimeoutSec` 去抖、(b) 有 `iCleanOutHandoverWaitSec`、(c) 覆蓋離窗;無人線的最終結局交 **OD-2** 裁定 |
| 3 | `WAR0964` 模態本身 | 模態無逾時是既有設計(`note.cpp:833` 阻塞在 `ShowModal`)。v2 不改它,但:①`iKeys` 一定含 `K_SKIP`(唯一無條件可按的出路);②答非 RETRY/SKIP 時**重新武裝 deadline 再問**,不消費 latch;③無人線的預設由 **OD-2** 決定(可設成到期自動 SKIP) |
| 4 | `ShortageDebounce[si]` 在抑制下 | 抑制窗上界 = `iCleanOutHandoverWaitSec`,一到期或一宣告就放開,且 `ShortageDebounce` 照舊累加 → 抑制解除當 tick `WAR0962` 立刻補上。抑制只作用於 `Handshake==AGV_CALLED` 且非真滿車的站 |
| 5 | 放棄用的 deadline `HTimer` | 一 tick 只 `Off()` 一次;消費後 `Clear()`;只在「欠交接集合新增成員」或 RETRY 時重新武裝;`bHandoverSnapshotDone` 把「永久觸發器」與「重量級動作」解耦 |
| 6 | 條件 (a) `!IsAmrCallableNow()` | 兩頭都補上:1-tick 抖動被 `iAgvTimeoutSec` + `bIdleAll` 擋掉;SECS 永久關閉的機台等一個窗後宣告一次,之後由第 7 步的「答非 RETRY/SKIP → 重問」節流 |
| 7 | RETRY 之後的 `Handshake` + 鎖 | `K_RETRY` 只在 `RunMode==Run_CleanOut && IsAmrCallableNow()` 時提供(N4),所以「保證無效的 RETRY」不存在;鎖仍由 `RetryStation` 刻意保留(防 GoUp 進滿車),而抑制此時已解除 → `WAR0962` 是第二條逃生門 |
| 8 | AMR=0 分支在重構之後 | B1 已修:`:1914`/`:1952-1953` 不動 + grep 不變量 + S2 的 AMR=0 驗收條件 |
| 9 | 中途切 `UseAMR` 0→1 | 帳從未累加(`:848`)→ D4 述詞恆假 → **靜默不收車**。v2 明寫進承諾範圍:**D4 只涵蓋「整批都在 `bUseAMR==1` 下跑」的裝載**;並與 R14(斷電)合併成 §8 H11 / H13 的一個具名結局(是否在下一次 Lot Start 出具名警報,沿用 v1 的 D-6,不阻斷) |

### 6.3 非阻斷項 N1–N6

- **N1(sim / 無 host 路徑)—— 白話說明**:committed 的 `MachineType.h:7` 是 `#define SOFT_SIMULATE`(工作樹現在被註解掉,見 V2-10),`system/General.ini:3` 是 `UseAMR=1`,而帳的累加閘(`aAuto1To6.cpp:848`)是 `bUseAMR` **不是** `!IsSoftSimulate()` → **筆電每一次清機都會武裝 D4**。v1 說「sim 的 `IsDrainedForAmr`/`IsAmrTaken` 都回 true,握手會自己走完」**是錯的**:`PollAndCall` 在 `bSelected==false` 就 return(`uAgvStation.cpp:445-469`),CALL 根本發不出去;`AGV_CALLED→AGV_PREP` 只發生在 host 的 `BeginPrep`(`:~800-819`),需要 S2F41。
  **所以 D4 之後,沒接 SECS 模擬器的筆電清機一定以放棄路徑收場:等 `iAgvTimeoutSec`(現場 300 s)→ 一顆 State Record 快照 → 一則會停機的 `WAR0964`。** 必要配套:①無人值守的 sim 迴歸腳本改設 `UseAMR=0` 或掛上模擬器;②`--selftest-home` 不跑清機,不受影響;③§8 H2/H3 的期望欄改寫成「**立即放棄 = 一則會停機的 `WAR0964`**」,不是「靜默完成」。
- **N2** → 見 0.5:**critic 這條不成立**(V2-3)。改用單一 episode deadline + 「集合新增成員才重新武裝」。
- **N3** → §4.B-3 已收窄到 `Handshake==AGV_CALLED`。
- **N4** → §4.C-2 第 6 步 d. 的按鍵遮罩。
- **N5** → §4.D:`iHandoverRetryUsed` episode-scoped + 上限 5。
- **N6** → §4.B-4:`ReleaseStationByOperator` 加 reason 多載,既有 1 參數版行為 byte 不變。
- **N7** → §4.C-2 第 7 步 `K_SKIP` 2.:歸零 `TrayCount[si]/DeviceCount[si]`(比照 `:684-685`)。
- **N8** → §4.C-2 第 5 步 (c) 已明寫 `bHomeByStart` 不清 `bCleanOut` 的 split;宣告訊息與 EventLog 一律帶 `RunMode` 與 `bCleanOut`;§8 H6 的期望改成「**批會被結掉**(或在 base 不成立時留在 `Run_CleanOut`),而且一定留下紀錄」。

### 6.4 v2 自行發現(V2-1 … V2-10)

| id | 內容 | 處置 |
|---|---|---|
| **V2-1** | SKIP → `note.cpp:871-893` 的 skip-IC-count 第二模態 + **CEID 78 / SVID 37010 假庫存沖銷**(京元用它對帳),且刻意不按 JAM 碼設閘 | §4.F,**OD-5** |
| **V2-2** | Auto 排空梯是六站連鎖閘(`:1151-1152`、`:1158-1159`、`:1166`);D4 自己的鎖 + 一次遲到交件(`:665` → `:1271`)就能把 `IsAllCleanOutFinish()` 壓成 false → **base 前置的放棄是自我參照死鎖**,與假亮 sensor 無關也成立 | 強化 B2 的修法(§4.B-3、§4.C-2) |
| **V2-3** | case 7000(`:1210-1219`)一個迴圈設六站 → 六站同 tick 武裝;critic 的 N2「逐站」不成立 | 不做 per-station deadline(§4.C-2 第 3 步) |
| **V2-4** | `ArchiveWorkOrderToLotStory` 有 `GetLotCount()<=0` 早退(`main.cpp:3609-3610`)→ v1 R9 的理由錯了 | §4.C-3 更正理由,結論(恰好一次)不變 |
| **V2-5** | `g_SoterOutput.OnLotEnd()` 自身冪等(`:380`/`:545`),且 `csystem.cpp:1936` 早就直呼它 | 讓 B5 的前移**零改動** `DoLotEndProcess`(§4.G) |
| **V2-6** | `DoLotEndProcess` 的 `SystemStart=false` 在**最上面**(`:3271-3274`)→「提前呼叫 DoLotEndProcess」物理上不可行 | §4.G 的前移只搬兩個具名副作用 |
| **V2-7** | `ShowMyError` 阻塞在 `ShowModal`(`note.cpp:833`)→ 放棄動作是「一次答覆重跑一次」,不是每 tick;房規解法已存在於 `csystem.cpp:184-187` | B3 的修法照抄房規 + `bHandoverSnapshotDone` |
| **V2-8** | critic 的 P6 若插在 RunMode 閘之前 → `Run_Normal` 也會 `bCollect==true`,`PollAndCall` 服務 `Run_Normal`(`:489-490`)→ 幻影 CEID 272 + 對新批的 TrayArm 供料鎖 | §4.B-1 的插入點在 RunMode 閘之後 |
| **V2-9** | `WAR0962` 在京元現況會**先**觸發(站停在 `AGV_CALLED`,300 s),而它只給 `K_RETRY`、清不掉 D4 latch | D4 警報必須存在;抑制窗必須結構性封頂(§4.B-3) |
| **V2-10** | 工作樹 `MachineType.h:7` 是 `//#define SOFT_SIMULATE` 而 HEAD 是 `#define SOFT_SIMULATE`(編譯閘驗真機後未還原) | **S0**:動工前先還原,讓所有基準量測從 committed 組態起算 |

---

## 7. 風險與對策

v1 §11 的 R1–R18 全部保留(不重排),以下是 v2 改寫或新增的:

| 風險 | 怎麼咬人 | v2 對策 |
|---|---|---|
| **R1' 帳沒有實體取消路徑** | 排空完成後 `DoAuto` 已短路(`aAuto1To6.cpp:2002-2011`)、`ServiceCarFull` 只跑 `Run_Normal`(`:1910-1911`),操作員推走車時沒有程式會把帳歸零 → latch 永真 | `K_SKIP` 的 `ClearAmrCar` 是唯一解,**不可省**;而 v2 保證到得了 SKIP(有界的 (a)(b)(c) + `K_SKIP` 永遠可按) |
| **R3' 警報停機 → finish dispatcher 凍結** | `ShowNoteAlarm` 做 `SystemStart=false`(`note.cpp:809-810`),`ProcessMotion` 在 `SystemStart==false` return(`csystem.cpp:1808-1809`) | 放棄路徑自己關帳(§4.C-3);base 不成立時明寫「按 Start 續排空」是復原路徑,並已複驗 `ProcessStartMode`(`:1339-1348`)不動 `RunMode`、Start 只在未 home 時才觸發 HOME(`main.cpp:2368-2384`) |
| **R19(新)base 被 D4 自己壓成 false** | V2-2 的連鎖閘 + 遲到交件 | 宣告不看 base;抑制窗不掛 base |
| **R20(新)CEID 78 假庫存沖銷** | V2-1 | §4.F / OD-5 |
| **R21(新)前移落地後續產拆檔** | §4.G 副作用 2/3 | 寫進客戶知會與手冊;FTP 來源在 archive 不在 pickup,已複驗 |
| **R22(新)筆電清機從此以停機警報收場** | N1 | sim 迴歸改 `UseAMR=0` 或掛模擬器;寫進 SOP |
| **R16' BCB6 檔案被編輯工具弄壞** | `Edit` 會把 `.cpp/.h` 轉成 LF,徵兆只有 commit 那句 LF warning | byte-safe 編修;改完驗 CRLF;新註解 **ASCII English only**;`check-ht160s-source-encoding.ps1` 必過 |

---

## 8. 驗證計畫

### 8.1 建置閘(每一步都跑)

1. 改到的 `.obj` 先刪,`scripts/ops/build-ht160s.ps1 -Clean` EXIT 0;接線層改動跑 `-Full`。
2. **真機組態也要驗**:註解掉 `MachineType.h` 的 `#define SOFT_SIMULATE` → `-Full` EXIT 0 → **還原 define 並重建**(本案動到 `IsSoftSimulate()` 相鄰的述詞群)。
3. `scripts/ops/check-ht160s-source-encoding.ps1` 通過(不得出現 `EF BF BD` 或 UTF-8 BOM)。
4. `--selftest-home` 迴歸 EXIT 0(`InitialAllTask` / `InitialFlag` 路徑被動到)。

### 8.2 不可卡住測試(最高優先)

| # | 情境 | 期望 |
|---|---|---|
| H1 | host 連著但完全不回 AUTO 站的 `START_AGV` | 站停在 `AGV_CALLED`;抑制窗內不出 `WAR0962`;deadline 到 → **一顆**快照 + EventLog + `WAR0964`;RETRY 用完只剩 SKIP;SKIP → 批結掉,`CEID 42` → `8` 各一次 |
| H2 | `[SECS] Enable=0` | 條件 (a):所有站 IDLE + `iAgvTimeoutSec` 去抖 → 宣告一次(**一則會停機的 `WAR0964`**,不是靜默完成) |
| H3 | SECS 付費功能關(`bUseSecsGem=0`) | 同 H2;閘門服務仍跑(在主迴圈) |
| H4 | **等待中 link 掉,且某站已在 PREP/READY** | **不得**觸發放棄條件 (a);鎖與握手保留;`WAR0963` 照舊;D4 latch 存活,由 deadline 收尾。**不可**出現「鎖放了、批也結了、車還滿的」組合 |
| H5 | 操作員在等待中把車手動推走 | 帳仍 > 0 → latch 仍真 → `WAR0964` → SKIP 清帳 → 批結掉。**不可**永久卡住 |
| H6 | 等待中按 Start 觸發 HOME(機台未 home) | (c) 觸發並留紀錄;訊息帶 `RunMode` 與 `bCleanOut`;keep-material HOME 之後帳與 latch 都在、鎖由 `ReassertLocks` 重新耦合、**短路使叫車不被撤銷** |
| H7 | 兩站 → 六站欠交接 | 只出**一則** `WAR0964`;不得出現六個接連的停機模態;抑制解除後 `WAR0962` 才允許出現 |
| H8 | **`bUseAMR=0` 迴歸(B1 的專屬測試)** | 逐項比對:`CEID 42` 照發;SKIP 後回 `Run_Normal` 且**模態不重彈**;`MES1x23` 照出;不發 272;不自動 Lot End。**外加 §4.C-3 的 grep 不變量** |
| H9 | 手動 Lot End 按鈕在等待中被按 | 立即結批;`bCleanOut` 與 latch 由隨後的冷 `InitialAllTask()` 清乾淨,不留幽靈欠交接 |
| H10 | SECS `CLEAR_LOT_INFO` 在等待中送達 | 兩道既有 HCACK 閘照舊;接受時立即結批 |
| H11 | 斷電後重開,車上有料 | 帳與 latch 都是 RAM-only → D4 **無法**保證收車;必須是「已知且有記錄」(D-6) |
| H12 | 某站 `InputFullTray` 卡 ON | 既有停滯(`:1283-1285`),不是 D4 造成;`WAR0964` 的 sensor 三態 + `DescribeCleanOutBaseBlockerForOperator()` 必須讓兩者可分辨 |
| **H13(新)** | **base 被壓成 false**(手動把某站 `SnAutoX_InputHasTray` 拉亮,或在等待中讓 TrayArm 遲到交件一盤) | **必須**照樣出 `WAR0964`(B2);訊息要說出「base 不成立:Auto」;SKIP 後**不**發 CEID 42、留在 `Run_CleanOut` 並提示按 Start |
| **H14(新)** | **連按 RETRY 到用完預算** | `SaveRoot` 只多**一顆** zip(B3);EventLog 每次宣告一行;預算用完後按鍵只剩 SKIP |
| **H15(新)** | **`bAskSkipICCount=1` 下按 SKIP** | **不得**彈「How many ICs …」鍵盤,**不得**發 CEID 78 / SVID 37010(V2-1) |
| **H16(新)** | **前移落地後斷電**(OD-1 選 A 時) | 拔電後重開:Soter CSV 已在 archive、FTP publish job 已排入、`WorkOrder.json` 仍在磁碟並被還原 |

### 8.3 SECS 序列測試(`D:\AI_Area\Tool\HT160S_SECS_Simulator`)

- **S1 正常**:sim build 跑 Clean Out → 六站各發 `CEID 272`(bitmap 單站)→ 模擬器 `auto_agv` 回 `START_AGV`(該站 `"Action"`)→ `273` → `274` → 六站收完 → `CEID 42` → `CEID 8`,順序與次數正確,**沒有** `35/36/37/148/149/150`,**沒有** `78`。
- **S2** 全 NA / bad-CP 預設(`code/ht160s_presets.py`):走 H1 的放棄路徑。
- **S3** 某站整批沒出料(帳 = 0):**不發** 272,不 latch,不擋 finish。
- **S4** 真滿車與交接叫車混合:真滿的那站照發離散 Full CEID 且 `WAR0962` **不被抑制**;交接的那站不發、且在窗內被抑制。
- **S5** `S5F1`:`WAR0964` 的 `ALID == 200000964`,`ALTX` 以 `"WAR0964 "` 開頭;`S5F6` 目錄多一列。
- **S6(新)** 交接途中把站推進 `AGV_READY` 後拔掉 link:確認**沒有** CEID 272 重發、**沒有**放棄條件 (a)。

### 8.4 上機驗證(使用者執行;硬前置)

- **P1(硬前置)** 逐站量測 `SnAutoX_InputEnd`:IO Monitor,車空/車滿/無車三態各記一次。它是 `IsAmrTaken`(`aAuto1To6.cpp:1697-1707`,CEID 274 的唯一釋放閘)的唯一依據;不驗就把批的結束綁上去,等於把「靜默留料」換成「結不了批」。
- **P2** 排空**進行中**抓一張 State Record,確認每站 `CarTrays` 有值(0904 兩張快照都不是生產中取的,既沒否證也沒佐證帳的正確性)。
- **P3** 真機 + 真 AMR 全流程;以及 `AMR=1` 但 AMR 不在線的放棄流程。
- 依 confirm-compile 慣例:我保證編譯與模擬,機台行為由使用者上機驗。

### 8.5 攻防複驗

實作完成後再跑一次對抗性審查,重點打:①`CloseCleanOutForAmr` 的恰好一次;②`Handshake`/鎖/latch/抑制旗在 RETRY、SKIP、答非二鍵、HOME、link 掉、Start 六條路徑上的殘留;③抑制解除後每條路是否真的都還有逃生門;④前移落地在「排空完成 → 續產 → 再結批」序列下的檔案結果;⑤BCB6 檔案行尾(CRLF)與 ASCII 註解。

---

## 9. 實作順序(每一步可獨立建置與驗證)

| 步 | 內容 | 獨立驗收 |
|---|---|---|
| **S0** | 整理:還原 `MachineType.h` 的 `#define SOFT_SIMULATE`(V2-10),確認工作樹與 HEAD 一致 | `git diff MachineType.h` 空;`-Clean` EXIT 0 |
| **S1** | `GeneralSetting`:`iCleanOutHandoverWaitSec`(1800/[60,7200])+ `iCleanOutHandoverRetry`(2/[0,5]),四處寫法比照 `iAgvTimeoutSec` | build EXIT 0;`General.ini` 讀寫往返;無消費者 → 行為零變化 |
| **S2** | `csystem`:`CheckCleanOutFinishBase()` + `DescribeCleanOutBaseBlockerForOperator()`;抽出 `CloseCleanOutForAmr()`(**只搬 `:1927-1929`**),`:1914`/`:1952-1953` 不動 | build EXIT 0;`--selftest-home` EXIT 0;**§4.C-3 的 grep 輸出必須為空**;**H8(AMR=0 迴歸)必過** |
| **S3** | `aAuto1To6`:`IsOutputCarHoldingTraysForAmr` + `bHandoverOwed[]` + 四個存取 + `DescribeHandoverOwedForOperator` + latch 生命週期 + §4.A-5 註解更正 | build EXIT 0(sim + 真機組態);尚無消費者 → 行為零變化 |
| **S4** | `uAgvStation`:latch 短路(RunMode 閘之後)+ `IsAmrCallableNow()` + `HandoverSuppress[]` 陣列與 `:699` 那一項 + `ReleaseStationByOperator` reason 多載 | build EXIT 0;`ReleaseStationByOperator(si)` 1 參數版行為 byte 不變;掛模擬器的 sim Clean Out 開始出現 `CEID 272` 並自己走完握手 |
| **S5** | `csystem`:`ServiceCleanOutHandoverGate()` + `MainProc` 呼叫點 +(**此時才**)`CheckCleanOutFinish()` 的 D4 項 | H1/H2/H3/H4/H5/H6/H7/H13/H14 + S1–S4/S6 序列測試 |
| **S6** | `database.cpp` + `system/AlarmList.csv`:`WAR0964`(486 → 487) | S5F1 ALID = 200000964;S5F6 目錄多一列 |
| **S7** | **(OD-1 選 A 才做)** §4.G 的落地前移:兩行 + 一行 EventLog | H16;續產拆檔行為與文件敘述一致 |
| **S8** | **(OD-5 選「加防護」才做)** `note.cpp` 的 skip-IC-count family 防護 | H15;`bAskSkipICCount=1` 下其他 SKIP 警報行為不變 |
| **S9** | 兩種組態編譯驗證 + 編碼檢查 + 行尾檢查 + 攻防複驗 | 兩種組態 EXIT 0;編碼 PASS |
| **S10** | 文件:工作簿 + 兩本 `.md` + `build-secs-html.py --build` + `docs/AGV` 兩份 SOP | 見 §10 |
| **S11** | 上機驗證(使用者):P1 → P2 → P3 | — |

**可以只出一步的分界**:S0–S4 全部是「加東西但沒人用」或純重構,任何一步單獨出貨都不改變機台行為。真正改變行為的是 **S5**。若上機時間不夠,S0–S4 先進 main,S5 單獨掛一個版本上機。**S2 是唯一有回歸風險的重構步,它的驗收條件(grep 不變量 + H8)不得省略。**

---

## 10. 文件與工作簿同步(S10 明細)

**SSOT = `D:\HT160S_BCB\docs\SECS\SECS_GEM功能_Handler_20260903.xlsx`**。動手前先 md5 比對底本;`openpyxl` 會吃掉內嵌的兩張 PNG,依既有配方處理。**工作簿只在程式碼進 main 之後才改**(它是釘在某個 firmware commit 上的快照)。

1. **CEID 頁**:`42 Clean Out Finish` 加一句時序 ——「AMR 模式下,42 會等到 Auto 出料車全部交接完成(或交接放棄)之後才發送;42 與 8 之間可能相隔數分鐘至數十分鐘,而非既有的數十毫秒。」`35/36/37/148/149/150` 說明**不動**。
2. **ALID 頁**:486 → **487** 列,新增 `WAR0964` / `200000964`,F、G 解碼欄照既有規則。
3. **修訂說明頁**:釘住的 firmware commit 更新為本案 commit。
4. **`docs/SECS/HT160S_SECS_Comm_Examples.md`**:§3.4 補「Clean Out 收尾的交接序列」+ 放棄序列;第 2 章 mermaid 在 AGV 那一輪旁註明收尾情境。**不偽造 log** —— 引用 0904 那一趟真實的 `START_AGV → 273 → 274`。
5. **`docs/SECS/HT160S_SECS_Interface_Spec_20260727.md`**:§3.3 的 `42`/`8` 兩列;§3.3.4 補交接觸發與放棄語意;§3.4 強調 `START_AGV` 的該站 CP 值必須是 `"Action"`。
6. **重生 HTML**:先 `--selftest`,再 `python scripts/ops/build-secs-html.py --build`。
7. **`docs/AGV/HT160S_AMR_Field_Verification_SOP_20260720.md`**:新增「`SnAutoX_InputEnd` 逐站三態量測」為 D4 上機硬前置。
8. **`docs/AGV/HT160S_AMR_ManualInject_Test_SOP_20260720.md`**:補 D4 的 sim 情境,**並明寫「筆電未掛模擬器時清機會以停機 `WAR0964` 收場,無人值守迴歸請設 `UseAMR=0`」**(N1),以及 `AmrInject.bTestMode` 撐不過一次 HOME。
9. **`docs/plan/loader-overcount-nostop-cleanout-lotend-plan-20260721.md:306`** 的 D4 待辦改為指向本計畫。
10. **操作手冊**(`docs/manual`)Clean Out 一節:等上機驗完再更新,OP 面語言(禁用元件 Name),必須寫明 `WAR0964` 的 SKIP 語意與「base 不成立時按 Start 續排空」。
11. **客戶知會(隨新版一起走)**:①host 必須以 `AUTOn="Action"` 回應 AUTO 站的 `START_AGV`,否則本功能只會逾時;②`CEID 42` 與 `CEID 8` 的牆上時序後移(**上界說清楚**);③新增 ALID `200000964`;④Clean Out 窗變長期間 P1–P3 進料 `START_AGV` 被拒的時間一併變長(既有 by-design,但時長對客戶是新的);⑤**若 OD-1 選 A**:Soter CSV 檔名時戳提前到排空完成;⑥**若 OD-5 選「加防護」**:`WAR0964` 的 SKIP 不會發 CEID 78。

---

## 11. 開放裁定

見 `owner_decisions` 欄的 **OD-1 … OD-5**。**OD-1、OD-2、OD-3 三項必須先裁定才能開工**(分別決定 S7 做不做、放棄的最終結局、以及是否允許修改既有看門狗);OD-4 決定叫車台數;OD-5 決定 S8 做不做。

v1 的 D-6(斷電後 Lot Start 具名警報)、D-7(CEID 42 留原時刻 vs 新私有段 CEID)、D-8(`InputEnd` 上機量測列硬前置)、D-9(「本回已服務」一次性 latch)維持原建議、不阻斷。

---

## 12. 附:研究過程中仍不確定的地方(如實記錄)

1. **`SnAutoX_InputEnd` 的語意有兩種讀法**:一路稱它「車上有盤」(`aAuto1To6.cpp:1703-1706` 的註解),一路依 0904 現場資料(六站同時 0 → 1,其中三站整批沒收過料)主張它是「車在位」。本計畫**兩種都不採信**,把它留在唯一的既有角色(CEID 274 釋放閘),逐站量測列為上機硬前置。
2. **0904 的 `CEID 272` 次數**:簡報說「下午另有七則」,該快照的 SECS log 只找到四則,推測其餘落在快照範圍外。本計畫的論證不依賴這個數字。
3. **0904 15:24:21 那張快照的性質**:開機時的 HOME-RESUME 快照,在本批生產任何一顆料之前;它的 `CarTrays=0` 是預期值,**不能**用來佐證或否證帳的正確性 → §8.4 P2。
4. **`iAmrFeedWaitSec` 的預設值**:韌體預設 600(`GeneralSetting.cpp:139/279`),repo 與現場 ini 都是 60(`system/General.ini:129`)。時序論證一律以**現場值**為基準。
5. **重複叫車風險(0901 遺留)**:叫車讀 `InputHasTray`、完成讀 `InputEnd`,兩顆不同 sensor;若有盤實體卡在交件位,車被收走後 `InputHasTray` 仍 ON → 又叫一次車給一個沒有車的站。這是**既有**缺陷,D4 保留 OR 所以沒有加重,但把它變成每次收尾都可能踩到(D-9,建議補、可延後)。
6. **`WAR0962` 與 `WAR0964` 的先後在現場尚未實測**:V2-9 的推論(站停在 `AGV_CALLED` → 300 s 先出 962)來自程式碼,0904 的 log 裡沒有 AUTO 站的 962 可以佐證(當天四則 272 全是 Auto2 且屬真滿車)。§8 H1/H7 必須把兩者的先後順序當成明確的觀察項記錄下來。
7. **`bAskSkipICCount` 在京元現場的實際值未知**:repo 是 0(`system/General.ini:26`)。若現場為 1,V2-1 就是**已經在發生**的問題(0962/0963 只給 RETRY,所以今天還碰不到,但任何未來給 `K_SKIP` 的 AMR 警報都會踩)。OD-5 的答案應該連帶要求現場確認一次這個值。


---

# §V3 第二次對抗式複驗的必修項(動工前)

第二輪複驗裁決 **SOUND_WITH_FIXES**。B1 / B3 / B5 / B6 經獨立覆核確認關閉;以下是**未關閉**與
**v2 的修正自己引入**的項目。動工前必須逐條處理,並把 §8 的驗收案例補上第 9 點列的三個情境。

## 必修(阻斷)

### V3-1 = NI-1 / B2 case (c):批永遠結不了的可達狀態

等待期間 `fAllMotorHome` 掉(伺服警報 `csystem.cpp:701`/`:1319`、EMG、馬達電源掉)→ 操作員按 Start
→ `main.cpp:2368-2384` 設 `bHomeByStart=true` → HOME。`csystem.cpp:1857-1861` 的 `if(bHomeByStart)`
走 `ChangeRunMode(Run_Normal)` 且**不清** `HSys.Sys.bCleanOut`。而 `aAuto1To6.cpp:93` 已把六站的
`State[].bCleanOutFinish` 清掉(在 `bKeepMaterial` early-out `:100-101` **之上**),
`TAutoModule::IsAllCleanOutFinish()` 第一行 `if(RunMode!=Run_CleanOut) return AllStationsDrainLatched();`
(`aAuto1To6.cpp:1263-1264`)→ false → base 恆 false。

此時 v2 §4.C-2 第 7 步 `K_SKIP` case 4 說「留在 Run_CleanOut,由既有 `:1910` dispatcher 收斂後自己
關帳」—— **不可能**:`RunMode` 已是 `Run_Normal`,`csystem.cpp:1908` 的 `if(RunMode==Run_CleanOut)`
永遠進不去。結果:latch 清了、鎖放了、人答了,但 `bCleanOut==true` / `RunMode==Run_Normal` /
**無 CEID 42、無 CEID 8、批永遠開著**。全樹 `ChangeRunMode(Run_CleanOut)` 只有三處
(`csystem.cpp:1859`、`:2000`、`main.cpp:2148`),全部需要人。

**修法(二選一,實作時定)**:SKIP/關帳分支明確處理 `RunMode!=Run_CleanOut` ——
(i) 先 `ChangeRunMode(Run_CleanOut)`(`bCleanOut` 仍 latched,這正是 `:1859` 會做的事)再交回
dispatcher;或 (ii) 在 (c) 路徑記錄「base 為 false」後**無條件關帳**。
把 latched 的 `bCleanOut` 留在 `Run_Normal` 就是 `csystem.cpp:1595` 自己稱的
"the drain was silently abandoned ... the batch never ended"。

### V3-2 = NI-2:`SoftStart` 撐過模態 → 關帳後機台重新啟動,並多發一次 CEID 42

`ShowNoteAlarm` 設 `SoftStop=false; SoftStart=false;`(`note.cpp:811-812`),而
`BtnStartClick` 設 `SoftStart=true`(`:513-514`)—— 帶鍵的模態**必須**按 Start 或 Pause 才關得掉。
關帳分支跑完後,`ProcessStartMode`(`csystem.cpp:1337-1354`)**先測 `if(SoftStart)`** →
`SystemStart=true` → 機台在 `Run_Normal` 下以剛清空的工單繼續跑。接著 `aLoader.cpp:1878-1886`
(閘門只有 `RunMode==Run_Normal && bAmrLocked==false && IsSupplySourceDry()`)重新武裝
`FeedWaitTimer`,60 秒後又 `bCleanOut=true; RunMode=Run_CleanOut`。全機已排空 → 梯子跑完 →
`csystem.cpp:1910` **第二次** `EmitCleanOutOK()`。
**客戶線上會看到 42 → 8 → 42**。`DoLotEndProcess` 也跑第二次(重複 LOT END 行、UPH 因跨距變長而衰減、
多一筆 TrayUphLog、又一次 `WriteLastDataIni`);只有 CEID 8 因 `LotRegistry.GetLotCount()>0` 已為
false 而沒重發(`main.cpp:3345-3346`)。

**修法**:關帳分支強制 `SoftStart=false`(或讓 `WAR0964` 的關帳不依賴操作員選 Start/Pause)。

### V3-3 = NI-3:等待中按手動 Lot End 或 host 發 `CLEAR_LOT_INFO` → 重複結批 + CEID 42 排在 8 之後

v2 §5 第 3 項刻意不對這兩條路加閘,而 §8 H9 聲稱「`bCleanOut` 與 latch 由隨後的冷
`InitialAllTask()` 清乾淨」——**那條路上沒有 `InitialAllTask`**。`btnLotEndClick`
(`main.cpp:3251-3258`)是薄殼,只呼叫 `DoLotEndProcess()`,而後者從不碰 `HSys.Sys.bCleanOut` 或
`RunMode`(全樹寫 `Sys.bCleanOut=false` 的只有 `csystem.cpp:1927`、`:1938`、`database.cpp:672`)。
所以等待中手動結批後:`RunMode` 仍 `Run_CleanOut`、`bCleanOut` 仍 true、latch 仍欠、deadline 仍在跑
→ `WAR0964` 照樣彈並停一台批已經結掉的機台 → SKIP → base 成立 → 關帳分支**第二次**跑
`DoLotEndProcess`,且 `EmitCleanOutOK()` 把 **CEID 42 排在 CEID 8 之後**。

**修法**:關帳分支對「已結批」冪等(例如 `LotRegistry.GetLotCount()<=0 && m_sActiveLot==""` 就跳過),
並修正 §8 H9/H10 的期望。

### V3-4 = B4-(b):deadline 到期時沒有握手狀態閘

放棄條件 (a) 已正確加上 `bIdleAll` 去抖,但 (b)(deadline 到期)**完全沒有握手狀態閘**。站可能停在
`AGV_READY`、AMR 實體就在車下(R13:`IsAmrTaken` 讀 `SnAutoX_InputEnd`,`aAuto1To6.cpp:1703-1706`,
若該 sensor 不轉 OFF 就恆 false)。SKIP 會 `ReleaseStationByOperator` + `ClearAmrCar` →
`bAmrLocked=false` → 排空梯 `aAuto1To6.cpp:1158-1159` 的保護失效;而操作員只能按 Start 或 Pause
才關得掉模態,按 Start 就是**往可能仍停著 AMR 的站恢復動作**。

**修法**:(b) 比照 (a) 加握手狀態閘,**或**把「先確認站內無 AMR」寫進 `WAR0964` 的訊息文字
(`WAR0963` 的文字本來就有這句,`WAR0964` 目前沒有 = NI-6)。

## 必修(非阻斷但客戶可見 / 會誤導)

- **V3-5 = NI-5 `WAR0962` 風暴 + 重試預算被繞過**:§4.B-3 的抑制公式含
  `bHandoverSnapshotDone==false`,而該旗在宣告時就永久為 true → 宣告之後抑制對**所有**欠交接站全部
  解除,而它們的 `ShortageDebounce` 早已超過 `iAgvTimeoutSec` → `WAR0964` 之後會**接連彈最多六個
  `WAR0962` 停機模態**。更糟:`WAR0962` 只給 `K_RETRY` → `RetryStation` → `Handshake=IDLE` →
  v2 的 latch 短路讓 `bCollect` 為 true → 重發 CEID 272,**且不消費 `iHandoverRetryUsed`** →
  §4.D 新增的重試上限可被繞過。
- **V3-6 = NI-4 宣告會凍結控制迴圈最多 60 秒**:v2 的順序是「先快照後警報」,而
  `TriggerSnapshot` 的 `CompressFolder` 是 `CreateProcess` + `WaitForSingleObject(...,60000)`
  (`cStateRecordHT160.cpp:596-606`),**無訊息泵**、跑在機控執行緒上 —— 所以在
  `ShowNoteAlarm` 走到 `DecStopAllMotor()` 之前,`MainProc` 會停止服務動作最多一分鐘,而其他模組
  可能還在排空。**改成先彈警報(它會停機)再快照**,或把快照閘在 `SystemStart==false` 之後。
- **V3-7 = B3 規格歧義必須釘死**:§4.C-2 第 3 步說「只在欠交接集合新增成員時重新武裝」,§6.2
  逃生門 5 說「消費後 `Clear()`」——**兩者矛盾**。若實作者選 `Clear()` 而不重新武裝,(b) 只會觸發
  一次;若那一 tick 剛好被 `fNote->fShow` 守衛吞掉,宣告就整個遺失。**釘一個。**
- **V3-8 = NI-9 tick 順序必須寫成不變量**:D4 之所以成立,靠的是一個未明說的順序 ——
  `ProcessMotion()`(`csystem.cpp:381`,承載 `:1908` 的 finish dispatcher)在
  `DataModule1->DoAllProcess()`(`:415`,跑 Auto 梯子到 case 7000)**之前**,而新閘門在 `:425`、
  在其**之後**。若日後有人把 `ServiceCleanOutHandoverGate()` 移到 `ProcessMotion()` 之上,
  dispatcher 會在 latch 還沒武裝時看到 base 成立而當場結批 —— **D4 靜默變成 no-op,且沒有任何測試
  會失敗**。把這個順序要求寫在呼叫點旁邊。
- **V3-9 = NI-8 可建置性**:`csystem.cpp` 今天對 `g_EventLog` 是**零引用**,也沒有 include
  `SecsGem/UsecegemMainFrom.h`(`AlarmReport(...)` 宣告在 `:35`,OD-2 若選 B 才需要)。
  §8 各步「可獨立建置」的聲明要補上這兩行。
- **V3-10 = NI-7 SKIP 可能留下暫時性錯亂的車帳**(H13 遲到交件情境):`ClearAmrCar` 歸零 `Car[]` 並
  重新播種角色後,重收的遲到盤會以 `iTrayCount=1` / `CarID=""` / `Tray[0]` 型別錯置重疊
  (`aAuto1To6.cpp:852-861` 把 `WorkingKind` 從 `RearKind` 複製,普通盤蓋到身分槽)。
  最終會被 `CloseCleanOutForAmr` 裡的冷 `InitialAllTask()` 清掉,是暫時性;但**清除前寫出的
  EventLog 行與清除後的實體車不符**。

## 引用行號漂移(落檔前更正,否則下一個讀者查不到)

第二輪複驗逐條開檔核對,以下是 v2 內文需要修正的引用:

| v2 寫的 | 實際位置 |
|---|---|
| `SetRearHasTrayFromTrayArm` 清 `bCleanOutFinish` 在 `aAuto1To6.cpp:665` | **`:664`** |
| `ArchiveWorkOrderToLotStory` 早退在 `main.cpp:3609-3610` | **`:3619-3620`** |
| `DoLotEndProcess` 停機三行 | `main.cpp:3269-3272` |
| §4.C-2 第 5(c) 步寫 `csystem.cpp:2368-2384(main.cpp)` | 應為 **`main.cpp:2368-2384`** |
| `g_SoterOutput.OnLotEnd()` / `WriteLastDataIni()` 在 `:3302`/`:3304` | 為**改動前**的行號,實作時需重新確認 |

數值面全部通過覆核:`AlarmList.csv` 486 → 487 列、ALID **200000964**、
57.367 × 6 = 344.202 s、`[60,7200]` 與 `[0,5]` 兩組 clamp。

## §8 驗收案例必須補三項

1. **case (c) 且 base 為 false** —— 斷言批**真的**結束了(V3-1)。
2. **每一條關帳分支跑完後的 `SoftStart` 狀態** —— 斷言機台不會帶著空工單自行恢復生產(V3-2)。
3. **等待中手動 Lot End / `CLEAR_LOT_INFO`** —— 斷言 `DoLotEndProcess` 恰好跑一次,且沒有
   CEID 42 排在 CEID 8 之後(V3-3)。

---

# §V3-RISK 已知殘留風險(必須寫進客戶知會)

1. **`SnAutoX_InputEnd` 仍未經現場量測,而它是 CEID 274 的唯一釋放條件**
   (`aAuto1To6.cpp:1697-1707`)。P1 必須維持為上機硬前提。若它不轉 OFF,每一次交接都會走
   (b) deadline —— 放棄路徑會成為**唯一**路徑,此時 V3-4 與 NI-6 從「修正」升級為「關鍵」。
2. **每次宣告一份完整 State Record 快照**:同步的整日 log 複製 + 7-Zip(60 秒上限),
   檔案自身註解稱其成長為 "quadratic"(`cStateRecordHT160.cpp:478-481`)。一個 episode 一次是對的
   預算,不可讓 RETRY 再買一次。
3. **筆電 / CI 行為改變,必須寫進 SOP**:`system/General.ini:3` 是 `UseAMR=1`,帳的累加閘是
   `bUseAMR` 而**不是** `!IsSoftSimulate()`(`aAuto1To6.cpp:847`),`PollAndCall` 在
   `bSelected==false` 就 return,`AGV_CALLED→AGV_PREP` 只發生在 host 的 `BeginPrep`。
   → **無人值守的 sim 清機會以一則停機 `WAR0964` 收場**。任何 headless 迴歸必須設 `UseAMR=0`
   或掛上模擬器;`--selftest-home` 不受影響。
4. **牆上時間要用數字告訴客戶,不要用形容詞**:CEID 42 → CEID 8 從 **28 ms** 變成最長
   `1800 + 5×1800` 秒,不答則無限期;P1–P3 進料 `START_AGV` 的拒收窗
   (`uAgvStation.cpp:806-810`,設計如此)同步延長;OD-4 選 A 時每批叫車台數加倍
   (光是身分盤+上蓋盤就讓 `iTrayCount==2`)。
5. **OD-5 應該擴大範圍**:`note.cpp:867-892` 不是 `WAR0964` 專屬問題,而是政策漏洞 ——
   `WAR0964` 只是第一個提供 `K_SKIP` 的 AMR 物流警報。應向現場確認 `[SECS] AskSkipICCount` 的實際值,
   並一次決定整個 family 的規則。

---

# §V3-ENV 動工前的環境整理項(V2-10)

`HT160S_Program_BCB_V1.0.0.0\MachineType.h:7` 在**工作樹**是 `//#define SOFT_SIMULATE`(關閉),
而 **HEAD 是 `#define SOFT_SIMULATE`(開啟)**,`git status` 顯示該檔為 M。依 `CLAUDE.md` 的
Compile Gate,開發預設應為**開啟**,關閉只是驗真機組態的暫時狀態。這是先前「驗完真機組態未還原
define」的殘留,不是本計畫造成的。

**後果**:目前 `EXE\ht160s.exe` 是**真機組態**建置的產物,拿到筆電上不會模擬。
動工前請確認要恢復成開發預設(開啟)還是維持現狀。

---


---

# §OD 決策狀態

| 編號 | 主題 | 狀態 |
|---|---|---|
| **OD-1** | Lot End 的落地類副作用要不要前移 | ✅ **已裁定 = 分支 A(拆)** |
| **OD-2** | 無人線 AMR 不來、RETRY 用完後怎麼結束 | ✅ **已裁定 = 分支 A(停下來等人)** |
| **OD-3** | 可否修改既有 `WAR0962` 看門狗 + 是否新開逾時鑰匙 | ⛔ **阻斷,待裁定** |
| **OD-4** | 「車上有盤」是否包含只有身分盤+上蓋盤(`iTrayCount==2`、零顆 IC) | ⚠ 建議先裁 |
| **OD-5** | `WAR0964` 的 SKIP 要不要擋掉 skip-IC-count 提示與 CEID 78 | ⚠ 建議先裁 |

以下保留 v2 撰寫的完整五項說明(OD-1 / OD-2 已由上表覆蓋,保留供追溯裁定當時的選項與代價)。

## fix_table
| defect id | v1 said | v2 says | why this closes it |
|---|---|---|---|
| **B1** CEID 42 / Run_Normal 被抽走 | §4.C-3「本體 = 現行 `csystem.cpp:1914` + `:1927-1929` + `:1952-1953` **原封搬移**」,且只有 AMR 分支呼叫 | 抽出範圍縮到 **`:1927-1929` 三行**;`:1914 EmitCleanOutOK()` 與 `:1952-1953 ChangeRunMode(Run_Normal); SoftStop=true;` **留在原處**;放棄支線自己補前後綴 | 已複驗 `:1914` 在 `if(bUseAMR)`(`:1915`)之上、`:1952-1953` 在 `else` 區塊(`:1931-1951`)之下 → 兩者是整個 `if(CheckCleanOutFinish())` 的共用前後綴。不搬它們,AMR=0 路徑就不可能改變。附**單一 grep 不變量**(`git diff csystem.cpp \| grep "^[-+]" \| grep -E "EmitCleanOutOK\|SnFKCleanOut\|ChangeRunMode\(Run_Normal\)\|SoftStop=true\|FreezeProductInfoAtLotEnd\|GetCalculateUPH\|LotEndTime"`,S2 必須空)+ 新增 H8 專測 AMR=0(CEID 42 照發、SKIP 後模態不重彈) |
| **B2** 抑制 + base 前置 = 無聲無界停機 | 三條放棄條件的**共同前置**是 `CheckCleanOutFinishBase()==true`;抑制掛在 `IsHandoverOwed(a)` 上,理由「旗在 = deadline 必然武裝」 | **宣告(快照+EventLog+WAR0964)不設任何前置**;只有**關帳**要求 base;base 為 false 時 SKIP 只清 latch/鎖/帳,留在 `Run_CleanOut`。抑制條件改成 `IsHandoverOwed && !IsOutputCarFullForAmr && Handshake==AGV_CALLED && bHandoverSnapshotDone==false && bDeadlineExpired==false` | 「已武裝 ≠ 能觸發」被切斷:宣告不再依賴任何外部述詞。而且 v2 新查到 **V2-2** —— 排空梯是六站連鎖閘(`aAuto1To6.cpp:1151-1152/1158-1159/1166`),D4 自己的鎖加一次遲到交件(`:665` → `:1271`)就能把 base 壓成 false → base 前置是**自我參照死鎖**,不只是假亮 sensor 的問題。抑制窗現在有兩個 D4 自己的有界計時項封頂,上界 = `iCleanOutHandoverWaitSec`,之後 WAR0962 與 WAR0964 兩門同時活著 |
| **B3** 無 announce-once;`HTimer::Off()` 恆真;快照重量級;retry 無上限 | 放棄動作 = `TriggerSnapshot` + EventLog + `ShowMyError`,無「已宣告」旗;`iCleanOutHandoverRetry` 只有下限 0 | 加 `bHandoverSnapshotDone`(**先設旗再做重活**,防 `ShowModal` 重入),每 episode 只拍一顆快照;`HandoverDeadline` 一 tick 只 `Off()` 一次、消費後 `Clear()` 再 `SetMS+On()`;`iHandoverRetryUsed` **episode-scoped**;`iCleanOutHandoverRetry` 加**上限 5**、`iCleanOutHandoverWaitSec` 加上限 7200 | 已複驗 `HTimer::Off()`(`HTimer.cpp:48-65`)不重置 `ulStartTicks`/`iTimeLen` → 逾時後恆真;`TriggerSnapshot`(`cStateRecordHT160.cpp:1755-1832`)同步做全 IO 傾印 + 整天 log 複製 + 7-Zip,該檔註解 `:478-479` 自稱同日成長是**quadratic**。v2 另查到 **V2-7**:`ShowMyError` 阻塞在 `ShowModal`(`note.cpp:833`),所以重跑節奏是「一次答覆一次」,修法照抄房規既有解 `csystem.cpp:184-187`(答非 RETRY 就消費 latch)。新增 H14 專測「連按 RETRY 只多一顆 zip」 |
| **B4** (a) 被 1-tick HSMS 抖動觸發;SKIP 抹掉車帳 | (a) `IsAnyHandoverOwed() && IsAmrCallableNow()==false` → **立即**放棄;SKIP 直接 `ClearAmrCar` | (a) 加 **`bIdleAll`(每個欠交接站 `Handshake==AGV_IDLE`)** + **`iAgvTimeoutSec` 去抖(`HTimer`,不數 tick)**;任一站已進 CALLED/PREP/READY 就不走這條、交回 `WAR0963`。SKIP **先**把 `iTrayCount`/`GetAmrDeviceCount(a)`/`CarID` 寫進 EventLog+快照,**再** `ClearAmrCar`,並同步歸零 `AgvCoord.TrayCount/DeviceCount`(比照 274 路徑 `:684-685`),且**不發 CEID 274** | 尊重 `uAgvStation.cpp:445-469` 的 20260819 裁定(「a dropped HSMS link is NOT evidence that the AMR has left」);已複驗 `ClearAmrCar`(`:1712-1720`)經 `InitAutoCarStack`(`:1373-1377`)把 `iAmrDeviceCount` 一併歸零,這條路上沒有 274 帶走那組數字 → 先寫後清讓帳誠實。H4 的期望同步改成「link 掉不得立即放棄」,新增 S6 序列(READY 中拔 link) |
| **B5** Lot End 資料落地退化成 30–90 分鐘或永不 | 只在 R12 寫「Soter CSV / FTP 交件往後移數分鐘」;無兩段化設計 | 明確三分類:**(i) 落地類前移** = `g_SoterOutput.OnLotEnd()`(`:3302`)+ `WriteLastDataIni()`(`:3304`);**(ii) 已在磁碟類不動** = 歸檔/清帳/`DeleteFile(WorkOrder.json)`;**(iii) 通知類不動** = CEID 8 等。R12 改寫為「CEID 8 最長後移 1800 + 5×1800 秒,不答則無限期」 | v2 查到三個關鍵事實讓這個切法既安全又便宜:**V2-5** `OnLotEnd()` 自身冪等(`cSoterOutput.cpp:380` `if(!m_bActive) return;` + `:545`)且 `csystem.cpp:1936` 早就直呼它 → 前移**不用改 `DoLotEndProcess` 一個字**;**V2-6** `DoLotEndProcess` 的 `SystemStart=false` 在最上面(`:3271-3274`)→ 「提前呼叫整個函式」物理上不可行;`WorkOrder.json` 整段等待都還在磁碟(`:3367` 才刪)→ 第 (ii) 類斷電**零遺失**,不需前移。另更正 v1 的 R9 理由(**V2-4**:`main.cpp:3609-3610` 有 `GetLotCount()<=0` 早退,第二次歸檔是無作為)。切法本身列為 **OD-1** 由 owner 定 |
| **B6** 「latch 就不會被撤銷」不成立 | 只把 latch **OR 進最後一項**(`:426`),前五閘照舊每 tick 重算 | latch 改成**短路 `return true`**,插在 `bUseAMR`/NULL/**RunMode** 三閘之後、三個 finish 閘之前;`:426` 原句不動(維持嚴格超集) | 已複驗兩條清除路徑:`SetRearHasTrayFromTrayArm` 在 `:665`(`:663-667` 內)、`InitialFlag` 在 **`:93`**(不是 critic 說的 `:92`)且在 `if(bKeepMaterial) continue;`(`:100-101`)**之上** → 每次 HOME 都清。短路後 `bCollect` 在整段握手與 keep-material HOME 前後都恆真,`:540-545` 的釋放分支進不去,`aAuto1To6.cpp:1158-1159` 的排空守衛不會在 AMR 還在車下時失效。**並且改良了 critic 的 P6**(**V2-8**):P6 把短路放在五閘之前會跨過 RunMode 閘,而 `PollAndCall` 也服務 `Run_Normal`(`:489-490`)→ 會在恢復生產時發幻影 CEID 272 並鎖住 TrayArm 對該 Auto 的供料;保留 RunMode 閘,離窗交給放棄條件 (c) |
| **escapes 1–9** | 九處只有外力出口或完全無出口 | §6.2 逐條對照表:1→宣告不看 base;2→(a)(b)(c) 全部有界 + OD-2;3→`K_SKIP` 永遠可按 + 答非二鍵時重新武裝再問 + OD-2;4→抑制窗上界 = `iCleanOutHandoverWaitSec` 且 `ShortageDebounce` 照舊累加;5→一 tick 一次 `Off()` + 消費即 `Clear()` + snapshot 旗;6→`bIdleAll` + 去抖 + 重問節流;7→`K_RETRY` 只在 `RunMode==Run_CleanOut && IsAmrCallableNow()` 才提供;8→B1 已修 + grep 不變量 + H8;9→`UseAMR` 0→1 明寫進承諾範圍 | 每一條都指向一個**具名的、有數值上界的**機制,或指向一個 owner 裁定(OD-2),沒有一條靠「操作員總會來」 |
| **N1** sim 路徑被改變而計畫聲稱沒有 | 「sim 的 `IsDrainedForAmr`/`IsAmrTaken` 回 true,握手會自己走完,筆電 Clean Out 仍會完成」 | 白話寫明:**沒掛模擬器的筆電清機,D4 之後一定以「等 `iAgvTimeoutSec` → 一顆快照 → 一則會停機的 `WAR0964`」收場**;無人值守 sim 迴歸須設 `UseAMR=0` 或掛模擬器;H2/H3 期望欄同步改寫;寫進 ManualInject SOP | 已複驗 `PollAndCall` 在 `bSelected==false` 直接 return(`:445-469`),`AGV_CALLED→AGV_PREP` 只在 host 的 `BeginPrep` 發生;帳的累加閘是 `bUseAMR`(`:848`)不是 `!IsSoftSimulate()`;`system/General.ini:3` 是 `UseAMR=1`;HEAD 的 `MachineType.h:7` 是 `#define SOFT_SIMULATE`(工作樹現被註解 → **V2-10** 列為 S0 整理項) |
| **N2** 單一全域 deadline vs 錯開武裝 | 「第一次有任何一站升旗時起算」,之後不重新武裝 | **critic 這條不成立**(**V2-3**):`aAuto1To6.cpp:1210-1219` 的 case 7000 一個迴圈設**六站**,`:1166` 的 `AreAllFlagsOn` 是六站連鎖閘 → 六站必然同 tick 武裝。v2 用單一 episode deadline,只在「欠交接集合**新增成員**」時重新武裝 | 不做多餘的 per-station 計時器;而 v1 真正的漏洞(重 latch 吃掉預算)由「集合新增即重新武裝」覆蓋 |
| **N3/N4** 抑制過寬;提供保證無效的 RETRY | `bOwedOnly` 無 handshake 項;按鍵遮罩只看 retry 預算 | 抑制加 `Handshake[si]==AGV_CALLED`(PREP/READY 的機構卡住仍走 300 s WAR0962);按鍵遮罩加 `RunMode==Run_CleanOut && IsAmrCallableNow()` | `RetryStation`(`:316-328`)只把站設回 IDLE,重叫靠 `PollAndCall`,而它需要 `Run_CleanOut`(`:418-419`)+ SELECTED(`:445-469`);兩者不成立時 RETRY 保證無效卻買走一個完整停機窗 |
| **N5/N6/N7** retry 生命週期;WAR0963 錯誤歸屬;SKIP 後 SVID 留舊值 | 未定義 / 直接借用 `ReleaseStationByOperator` / 未提 | `iHandoverRetryUsed` episode-scoped + 上限 5;新增 `ReleaseStationByOperator(int si, AnsiString sReason)` 多載(既有 1 參數版傳原字串 → byte 不變);SKIP 歸零 `TrayCount[si]/DeviceCount[si]` | `uAgvStation.cpp:299-300` 確實硬編 `"AGV: WAR0963 operator confirmed AMR left P…"`;274 路徑在 **`:684-685`**(不是 critic 的 `:681-682`)才歸零那兩個 SVID |
| **N8** (c) 可能在操作員以為已恢復生產時結批 | 未提 | (c) 明寫 `bHomeByStart`(`csystem.cpp:1852-1856`)不清 `bCleanOut`,即 `:1588-1596` 警告的 split;宣告訊息與 EventLog 一律帶 `RunMode` + `bCleanOut`;H6 期望改成「批會被結掉(或 base 不成立時留在 Run_CleanOut),而且一定留紀錄」 | 行為可辯護但必須可預期;另複驗 `ProcessStartMode`(`:1339-1348`)不動 `RunMode`、Start 只在 `fAllMotorHome==false` 時觸發 HOME(`main.cpp:2368-2384`)→ 「按 Start 續排空」是可用的復原路徑 |
| **V2-1(v2 新發現,阻斷級)** | — | SKIP → `note.cpp:871-893` 會彈第二個模態鍵盤並發 **CEID 78 + SVID 37010**(京元用它對帳庫存),而交接放棄時沒有任何 IC 被取出 = **假庫存沖銷**;加「AMR 物流 family 不問」防護,列 **OD-5** + H15 | 該段註解自稱 "Deliberately NOT gated on the alarm being a JAM* code";今日 0962/0963 只給 `K_RETRY` 所以碰不到,`WAR0964` 是第一個給 `K_SKIP` 的 AMR 警報 → 改動面積為零但影響是客戶可見的 SECS 行為 |
| **V2-2(v2 新發現)** | v1 認為 base 前置是安全措施 | 排空梯是六站連鎖閘(`:1151-1152/1158-1159/1166`),D4 自己的鎖 + 一次遲到交件就能把 base 壓成 false | 這讓 B2 的修法在**不依賴假亮 sensor**的情況下也必要:base 前置的放棄是自我參照死鎖。新增 H13 專測 |

## owner_decisions
以下五項是真的需要您裁定的;其餘(v1 的 D-6/D-7/D-8/D-9)維持原建議、不阻斷。**OD-1、OD-2、OD-3 必須先裁定才能開工。**

---

## OD-1【阻斷】Lot End 的「落地類」副作用要不要留在原時刻?(修 B5)

**問題白話**:今天清機一完成,44 秒內這批的客戶交付品就寫進磁碟了(Soter per-unit CSV + 排入 FTP 上傳 + 累計計數)。D4 之後,這一切會被押到「AMR 把車收走」之後 —— 而 §6/R11 已認定「host 從不回 `AUTOn="Action"`」是京元現場的**預設**結局,所以那段等待是常態而非邊緣。這段窗內斷電/當掉,整批 Soter CSV 與 FTP 交件就沒了。

**分支 A:落地類前移到「開始等待」那一刻(建議)**
- 做法:在閘門服務第一次有站升旗時,呼叫 `g_SoterOutput.OnLotEnd()`(`main.cpp:3302` 的同一個函式)+ `WriteLastDataIni()`(`:3304`),外加一行 EventLog。
- 為什麼便宜:`g_SoterOutput.OnLotEnd()` **自身冪等**(`cSoterOutput.cpp:380` 的 `if(!m_bActive) return;` + `:545` 的 `m_bActive=false`),而 `csystem.cpp:1936` 的非 AMR 分支**早就這樣直接呼叫它**。所以 `DoLotEndProcess`(`main.cpp:3260-3372`)**一個字都不用改**,屆時 `:3302` 是 no-op、`:3304` 只是重寫一次。`WriteLastDataIni` 是純快照寫檔,已有三個呼叫點。
- 為什麼只搬這兩項:歸檔/清帳那一類**斷電零遺失** —— `WorkOrder.json` 整段等待都還在磁碟上,`:3367` 才刪,重開機會被還原。CEID 8 不能提前,它的語意就是「這批結束了」,車還在等交接時提前發等於對 host 說謊,還會與 CEID 42 的順序打架。
- 三個已知副作用(要寫進客戶知會):①Soter CSV 檔名時戳(`cSoterOutput.cpp:393-395`)提前到排空完成;②若操作員之後按 Start 恢復生產,`main.cpp:2363` 的 `EnsureActive` 會重新武裝,續產的料寫進**另一支** CSV(拆檔,不是遺失);③`DoArm` 會 `ClearPickupDir()`(`:336`)清掉操作員取件夾 —— **FTP 上傳來源不受影響**,publish 的路徑都指向永久 archive(`:505`、`:516`)而不是 pickup 夾。
- 代價:出現一個新的中間狀態「這批的檔案已經寫了,但批還沒結束」。

**分支 B:什麼都不前移,改用硬上界壓住暴露時間**
- 做法:`DoLotEndProcess` 保持單一原子呼叫;把 `CleanOutHandoverWaitSec` 預設從 1800 降到 300(= 沿用 `AgvTimeoutSec` 的量級),RETRY 預設 0,並在 OD-2 選「到期自動關帳」。
- 好處:沒有半結束的批,Lot End 的 15 個副作用維持既有順序,審查面積最小。
- 代價:①暴露窗仍有 5 分鐘;②300 s 對「六台車依序交接」偏緊(實測一趟 57.367 s,六趟約 5 分 44 秒),合法排隊中的第 5、6 台會被誤判逾時,D4 的成功率下降;③OD-2 若選「等人」,暴露窗仍是無限期。

**我的建議:A(且只搬 Soter + lastdata 這兩項)。** 它是唯一能保護客戶交付品的分支,實作成本兩行,冪等性與呼叫慣例都是既有的,而且它把「批的資料」與「批的狀態」這兩件事分開 —— 前者本來就在排空完成那一刻定案(SortArm 已結束、不會再有 IC 落下),後者才該等交接。

---

## OD-2【阻斷】無人線上 AMR 一直不來、RETRY 也用完了,機台該怎麼結束?

**問題白話**:D4 存在的唯一理由是無人線。但 v1 選的「不答 = 模態留著、鎖留著、機台停著」在無人線上就是「停整夜」,而且(若 OD-1 選 B)這批的檔案還沒落地。

**分支 A:停下來等人(v1 的選擇,與 `WAR0963` 既有政策一致)**
- 做法:`WAR0964` 一直掛著,`K_SKIP` 是唯一無條件可按的出路;答非 RETRY/SKIP 時重新武裝 deadline 再問一次(不再拍快照)。機台不會靜默,但也不會自己往前走。
- 好處:①絕不擅自把帶料的車判定為「可以不管」;②`ClearAmrCar` 會抹掉盤數/IC 數,由人按下去才抹是最保守的;③與 `csystem.cpp:203-206` 的既有政策一字不差。
- 代價:夜班無人 = 停整夜 + 產能歸零 + (OD-1 選 B 時)整批檔案懸空。

**分支 B:到期自動放棄、記錄、關帳,車留在原地**
- 做法:新增 `[AGV] CleanOutHandoverGiveUpAction`(0 = 問操作員 / 1 = 自動),預設 **0**,現場無人線設 1。設 1 時:預算用盡後不彈模態,而是 `TriggerSnapshot` + EventLog + **`AlarmReport` 發一次 S5F1 `WAR0964`(set 後立刻 clear)讓 host 知道**,然後走與 SKIP 完全相同的關帳序列(先寫帳 → `ReleaseStationByOperator(reason)` → 歸零 SVID → `ClearAmrCar` → 降旗 → base 成立則關帳)。
- 好處:①產能不中斷;②有快照、有 EventLog、有 S5F1 → **不是無聲**;③下一批不會被上一批的 latch 卡住。
- 代價:①車帶著這一批的料留在原地跨到下一批 —— 這正是 D4 要修的那個病,只是現在**有記錄**;②`ClearAmrCar` 在沒有 CEID 274 的情況下抹掉盤數/IC 數(v2 已要求先寫進 EventLog+快照);③這是對 20260716 D2「不得靜默放行」的逼近,必須靠「有快照+有 S5F1」才站得住。

**我的建議:B,但預設值是 0(問人),由現場 ini 自己選 1。** 理由:韌體預設維持最保守的行為(不擅自放行),而無人線是**現場組態**不是預設組態,讓需要它的機台自己打開。並且 B 的自動路徑必須帶 S5F1 —— 只寫 EventLog 對無人線等於無聲。

---

## OD-3【阻斷】可不可以修改既有的 `WAR0962` 看門狗?(以及要不要新開一把逾時鑰匙)

**問題白話**:兩件事綁在一起,都是動既有的、已出貨的 AMR 逃生門。

**(a) 抑制 `WAR0962`**:不抑制的話,六站同時逾時 = 六個接連的停機模態(每則 `ShowNoteAlarm` 都 `DecStopAllMotor` + `SystemStart=false`,`note.cpp:809-810`,而 `csystem.cpp:191` 一次 sweep 只出一則)。而 v2 新查到:京元現況下 **`WAR0962` 會先於 D4 觸發**(站停在 `AGV_CALLED`,`uAgvStation.cpp:696-697` 的老化照跑,300 s 就出),而它**只給 `K_RETRY`**,清不掉 D4 的 latch → 批還是結不了。

**分支 A:不抑制。** `WAR0962` 照舊 300 s 出,操作員按 RETRY,D4 的 1800 s 到了再出 `WAR0964`。
- 好處:既有逃生門一個字不動,審查面積最小。
- 代價:六站情境下操作員要連續清六個停機模態,而且清完還是沒有結批的路 —— 現場會把它當成「機台壞了」。

**分支 B:條件性抑制(建議)。** 只在 `IsHandoverOwed(a) && !IsOutputCarFullForAmr(a) && Handshake[a+3]==AGV_CALLED && D4 尚未宣告 && D4 deadline 未到期` 時抑制。
- 好處:①**上界是結構性的** —— 抑制窗長度 ≤ `CleanOutHandoverWaitSec`,一到期或一宣告就放開,而且 `ShortageDebounce` 照舊累加,放開那一 tick `WAR0962` 立刻補上;②真滿車(產能觸發)與 PREP/READY 的機構卡住(R13,`InputEnd` 不轉 OFF)都**不抑制**,仍走既有 300 s;③實作是在 `uAgvStation.cpp:699` 加一項 `&& HandoverSuppress[si]==0`,旗由主迴圈的閘門服務單點覆寫。
- 代價:「host 沒回」這一種故障的操作員通知從 300 s 延到最多 1800 s。

**(b) 逾時預算從哪來**:新開 `[AGV] CleanOutHandoverWaitSec`(1800,[60,7200]),或沿用 `AgvTimeoutSec=300`?
- 新開的理由:300 s 是「單一握手沒回應」的預算,而六台車整體交接實測需約 5 分 44 秒,用 300 s 會讓合法排隊中的第 5、6 站誤報。而且 v2 的放棄條件 (a) 正是**沿用** `AgvTimeoutSec` 做去抖,兩個語意放在同一把鑰匙上會打架。
- 但這是對 20260721「AMR 逾時統一成一把鑰匙」裁定的**逆轉**(`docs/plan/amr-unmanned-alarm-reflow-plan-20260721.md:52`、`:109`),需要您明確推翻。
- 附註:`[AGV] AmrFullWaitSec=60`(`system/General.ini:130`)全樹零消費者,是 20260627 廢案的孤兒,**不建議挪用**(現場工程師會以為那個 60 有意義)。清理它是另一件事。

**我的建議:(a) 分支 B + (b) 新開鑰匙。** 兩者是同一個裁定的兩面:B 的上界就是新鑰匙的值,所以新鑰匙不只是「等多久」,它同時是「舊逃生門最多被壓住多久」。

---

## OD-4【建議先裁】「車上有盤」包含只有身分盤 + 上蓋盤(`iTrayCount==2`、零顆 IC)嗎?

**問題白話**:AMR 模式下每一個閒置 Auto 都會先拉身分盤再拉上蓋盤(`aAuto1To6.cpp:1392-1404` + `:1439-1440`),不管有沒有料路由到它。所以「整批沒收過料的流道」在現場是常態,不是邊緣。

**分支 A:算要收(`iTrayCount > 0`,建議)**
- 好處:①那兩盤屬於正在結束的這一批,不該跨到下一批;②述詞只有一行,沒有 `CanHoldIC` 掃描,沒有新的解讀空間;③與「Lot End 前把六台車全部叫走」的字面要求一致。
- 代價:每次收尾要叫的車數從「有料的 3 台」變成「全部 6 台」,牆上時間與 host 負擔都約 2 倍(實測一趟 57.367 s → 六趟約 5 分 44 秒)。

**分支 B:只收「載過 IC」的車(`GetAmrDeviceCount(a) > 0`)**
- 好處:牆上時間與 host 負擔減半;沒料的車留在原地也不影響下一批(那兩盤還是可用的)。
- 代價:①「全部叫走」變成「有料才叫走」,與 N1 定案的字面不符;②`iAmrDeviceCount` 的累加點與 `iTrayCount` 不同(discharge 時累加,不是升位時),求值時機的正確性要另外論證;③身分盤帶的是**這一批**的 2D 身分,留到下一批會讓車的身分與新批不符。

**我的建議:A。** 主要是理由 ③ —— 身分盤是批的身分,不該跨批。牆上時間變 2 倍由 `CleanOutHandoverWaitSec`(OD-3)吸收。

---

## OD-5【建議先裁】`WAR0964` 的 SKIP 要不要擋掉 skip-IC-count 提示與 CEID 78?

**問題白話**(v2 新發現,v1 與審查都沒抓到):`note.cpp:871-893` —— 只要操作員以 `K_SKIP` 清掉任何警報,且 `[SECS] AskSkipICCount=1`,機台就**再彈一個模態鍵盤**問「How many ICs were taken out of the tray?」,並把答案發成 **CEID 78 + SVID 37010(Jam Skip IC Count)**。該段註解明寫刻意**不**按 JAM 碼設閘。京元的 host 正是用 CEID 78 對帳庫存。

對 `WAR0964` 這是兩個獨立的傷害:①無人線上出現第二個阻塞模態,而 D4 存在的唯一理由就是無人線;②交接放棄時**沒有任何 IC 被從盤裡取出**,發這則事件是**假的庫存沖銷**。

`bAskSkipICCount` 目前預設 false(`GeneralSetting.cpp:119/257`,`system/General.ini:26` = 0),而今天 `WAR0962`/`WAR0963` 只提供 `K_RETRY`,所以這個分支**還碰不到**。`WAR0964` 會是第一個提供 `K_SKIP` 的 AMR 物流警報。

**分支 A:加防護(建議)**
- 做法:`note.cpp:871` 的條件加一項「本警報屬 AMR 物流 family(`WAR0962/0963/0964`)則不問」。因為 0962/0963 到不了這個分支,**唯一受影響的成員就是 0964**,行為改動面積為零。
- 好處:無人線不會被第二個模態卡住;不會送出假的庫存沖銷。
- 代價:①在共用的 SKIP 路徑上引入一個按代碼分類的例外(該段註解刻意反對按代碼設閘,所以要在註解裡寫清楚理由:AMR 物流警報不是材料沖銷警報);②要向客戶說明「`WAR0964` 的 SKIP 不會發 CEID 78」。

**分支 B:不動 `note.cpp`,改用別的按鍵**
- 做法:`WAR0964` 不用 `K_SKIP`,改用 `K_TRAY_END` 或 `K_CLEAN_OUT` 之類已存在的鍵。
- 好處:完全不動共用路徑。
- 代價:①`K_TRAY_END` 的語意在本專案已被定為 pick-error only(memory `sortarm-fixes`),挪用會製造新的語意混亂;②操作員面板上的按鍵標籤與「我已手動取車」對不上;③`LogRecovery` 的 `sRecovery` 欄會記成錯的復原動作。

**我的建議:A。** 並且請現場順便確認一次 `[SECS] AskSkipICCount` 的實際值 —— 如果京元是 1,那麼**任何**未來給 `K_SKIP` 的 AMR 警報都會踩到同一顆雷,這個防護的價值不只在 D4。

---

# 附錄:第二次對抗式複驗全文(2026-09-07)

VERDICT: SOUND_WITH_FIXES

====================== still_blocking ======================
I opened every cited line myself. Verdict per defect:

**B1 — CLOSED (verified).** `csystem.cpp:1914 EmitCleanOutOK()` really does sit above `:1915 if(GeneralSetting.bUseAMR)`, and `:1952-1953 ChangeRunMode(Run_Normal); SoftStop=true;` really do sit below the `else` block (`:1931-1951`). Narrowing the extraction to `:1927-1929` is the correct fix; the grep invariant + H8 is an adequate gate. The non-AMR `ShowSystemError(SnFKCleanOut,...)` at `:1945` is untouched.

**B2 — NOT FULLY CLOSED. One reachable state where the LOT NEVER ENDS, and the plan asserts an escape the code cannot provide.**
Scenario (give-up condition (c), SKIP, base false):
1. During the wait `fAllMotorHome` drops (servo alarm `csystem.cpp:701/1319`, EMG, motor-power drop). Operator presses Start → `main.cpp:2368-2384` sets `bHomeByStart=true` → HOME.
2. `csystem.cpp:1857-1861`: `if(bHomeByStart) ChangeRunMode(Run_Normal);` — and it does **not** clear `HSys.Sys.bCleanOut`. This is the split `csystem.cpp:1588-1600` already warns about in 20 lines of comment.
3. `aAuto1To6.cpp:93` cleared `State[].bCleanOutFinish` for all six (above the `bKeepMaterial` early-out at `:100-101`). `TAutoModule::IsAllCleanOutFinish()` **first line** is `if(HSys.Sys.RunMode!=Run_CleanOut) return AllStationsDrainLatched();` (`aAuto1To6.cpp:1263-1264`) → **false**. So `CheckCleanOutFinishBase()` is false, deterministically, on every (c) path that came through a HOME.
4. v2 §4.C-2 step 7 `K_SKIP` case 4 says: base false → "留在 `Run_CleanOut`,由既有 `:1910` dispatcher 收斂後自己關帳". **That is impossible here.** `RunMode` is `Run_Normal`; `csystem.cpp:1908 if(HSys.Sys.RunMode==Run_CleanOut)` can never be entered.
5. Result: latch cleared, locks released, machine answered, `bCleanOut==true`, `RunMode==Run_Normal`, **no CEID 42, no CEID 8, no `DoLotEndProcess`, lot open forever**. `grep ChangeRunMode(Run_CleanOut)` returns exactly three sites — `csystem.cpp:1859` (a *non*-Start HOME), `csystem.cpp:2000` (OneCycle finish), `main.cpp:2148` (the Clean Out button). All three need a human. The plan's own §6.2 escape row 1 is therefore false in case (c).
Fix required before coding: the SKIP/close leg must handle `RunMode!=Run_CleanOut` explicitly — either restore `ChangeRunMode(Run_CleanOut)` (bCleanOut is still latched, so this is exactly what `:1859` would do) before handing back to the dispatcher, or close unconditionally in (c) after logging that base was false. Silently leaving a latched `bCleanOut` in `Run_Normal` is the state `csystem.cpp:1595` calls "the drain was silently abandoned ... the batch never ended".

**B2 second gap (bounded, not a hang):** the suppression cap works as advertised (`ShortageDebounce[si]` really is incremented inside the condition at `uAgvStation.cpp:699`, so it fires the tick suppression lifts), but §4.B-3's formula includes `bHandoverSnapshotDone==false`, which is set true forever for the episode at the announce. So from the announce onward suppression is OFF for **all** owed stations, every one of which already has `ShortageDebounce > iAgvTimeoutSec` → `ServiceAgvTimeoutAlarm` (`csystem.cpp:167-192`, one per sweep, each `ShowNoteAlarm` → `DecStopAllMotor(); SystemStart=false;` at `note.cpp:808-809`) pops **up to six sequential WAR0962 stop modals immediately after WAR0964**. That is precisely the storm R4/OD-3 exists to prevent — v2 defers it rather than removing it. Worse, WAR0962's only key is `K_RETRY` → `RetryStation` → `Handshake=IDLE` → v2's latch short-circuit makes `bCollect` true → `PollAndCall` re-CALLs and emits another CEID 272, **without consuming `iHandoverRetryUsed`**. The retry budget (the whole point of §4.D's new clamp) is bypassable through WAR0962. §8 H7's expectation must be extended to cover 962-after-964.

**B3 — CLOSED, with one spec ambiguity that must be pinned.** `HTimer::Off()` (`HTimer.cpp:47-64`) confirmed: no reset of `ulStartTicks`/`iTimeLen`, only `InUsed=false` → true forever after expiry. `TriggerSnapshot`'s `CompressFolder` (`cStateRecordHT160.cpp:596-606`) is `CreateProcess` + `WaitForSingleObject(...,60000)` — a hard block, **no message pump**, so `bHandoverSnapshotDone` is not needed for re-entrancy but is needed for the quadratic growth the file's own comment at `:478-481` describes. Clamps [0,5] and [60,7200] are correct against the existing idiom (`GeneralSetting.cpp:305-320` today has lower bounds only). Ambiguity: §4.C-2 step 3 says re-arm only on a new-member edge, while §6.2 escape 5 says "消費後 `Clear()`". Those contradict. An implementer who picks `Clear()` without re-arming makes (b) fire exactly once — and if that tick is swallowed by the `fNote->fShow` guard the announce is lost entirely. Pin one.

**B4 — CLOSED for (a), NOT closed for (b).** (a) is correctly hardened: `ServiceHandshake` early-returns at `uAgvStation.cpp:621-622` when `Gem==NULL || IsSelected()==false`, so a station that had reached CALLED/PREP/READY genuinely freezes there and `bIdleAll` blocks (a) — the 20260819 ruling at `:446-468` is respected. Ledger honesty is right: `ClearAmrCar` (`:1712-1720`) → `InitAutoCarStack` (`:1377`) does zero `iAmrDeviceCount`, and the 274 path zeroes `TrayCount/DeviceCount` at `:684-685`, so write-before-clear + manual SVID zeroing is correct.
**But (b) has no handshake gate at all.** Deadline expiry fires WAR0964 with a station sitting at `AGV_READY` and the AMR physically docked (R13: `IsAmrTaken` reads `SnAutoX_InputEnd` at `aAuto1To6.cpp:1703-1706` and is false forever if it never turns OFF). SKIP then calls `ReleaseStationByOperator` + `ClearAmrCar` → `bAmrLocked=false` → the drain-raise guard at `aAuto1To6.cpp:1158-1159` stops protecting — and the operator can only close the modal by pressing Start or Pause (`note.cpp:498-527`/`:530-560` refuse to close unless a key is selected), so pressing Start resumes motion into a possibly-docked AMR. WAR0963's text explicitly says "Check the station is clear of the AMR, then RETRY"; v2's proposed WAR0964 text ("remove them by hand then SKIP") does not. Either gate (b) on handshake state the way (a) is gated, or put the AMR-clear instruction in the text. This is a fix, not a redesign.

**B5 — CLOSED as a design.** All three enabling facts check out: `cSoterOutput.cpp:380 if(!m_bActive) return;` + `:545 m_bActive=false` (idempotent); `csystem.cpp:1936` already calls it directly in the non-AMR branch; `DoLotEndProcess`'s stop is at the very top (`main.cpp:3269-3272 SystemStart=false; SoftStop=true; MachineRun.bRunning=false;`) so "call it early" really is impossible (V2-6 correct); `DeleteFile(GetWorkOrderFileName())` is at `main.cpp:3377`, i.e. the work order survives the whole wait (class (ii) really is zero-loss); `ArchiveWorkOrderToLotStory` really does early-return on `GetLotCount()<=0` (`main.cpp:3619-3620`, v2's own cite of `:3609` drifted). I also checked the one thing v2 did not: Soter rows are opened/committed **only** from `aSortArm.cpp:1484/1487/1717/1804/1955`, and arming requires `SortArmModule->IsCleanOutFinish()`, so an early flush cannot lose rows. Branch A is safe. It remains an owner decision (OD-1), so B5 is closed *conditionally*.

**B6 — CLOSED, and v2's correction of the critic is right.** `PollAndCall`'s RunMode gate (`uAgvStation.cpp:489-490`) sits **before** the P4-P9 loop and admits `Run_Normal`, so the critic's P6 (short-circuit above the RunMode gate) would indeed have produced a phantom CEID 272 + `SetAmrLock` in restored production. V2-8 is correct. I also confirmed the residual revocation after leaving `Run_CleanOut` is comparatively benign: the release branch is `else if(bFull==false && Handshake[si]==AGV_CALLED)` (`:543`) — `AGV_PREP`/`AGV_READY` are never released there, so only a station the host has not yet authorized can lose its lock. Run_Home cannot revoke either (the gate returns first).

**The nine escapes:** 1 closed *except* in case (c) (see B2). 2 partial — (a)(b)(c) bound the *announce*, not the disposition; the disposition is OD-2 and the firmware default v2 recommends is "ask a human". 3 **NOT closed by any v2 mechanism.** I verified `TfNote::BtnStartClick`/`BtnPauseClick` (`note.cpp:498-560`): with `KeyCode!=0` the form will not `Close()` until a key is selected. So §6.2 row 3's item ② ("答非 RETRY/SKIP → 重新武裝 deadline 再問一次") is unreachable dead code — the modal genuinely blocks forever. Row 3 therefore rests entirely on "K_SKIP is always offered" plus OD-2. On an unmanned line with OD-2=A the machine stops until a human arrives, full stop. That is honestly disclosed, but it is a deferred decision, not a closed escape. 4 closed (with the storm side effect above). 5 closed (pin the ambiguity). 6 closed. 7 closed — `RetryStation` (`:317-329`) does only set IDLE, and re-call needs `Run_CleanOut` (`:418-419`) + SELECTED (`:445-469`), so gating `K_RETRY` on both is right. 8 closed with B1. 9 declared out of scope — acceptable as scope, not as an escape.

**Facts v2 asserts that I confirmed independently:** `aAuto1To6.cpp:93/96/100-101/120-121` ordering; case 7000 sets all six flags in one loop (`:1210-1219`) and case 4000's `AreAllFlagsOn(bCleanOutCheck)` + `if(bAmrLocked[Index]) continue;` (`:1158-1166`) is a six-station lockstep barrier with that exact comment at `:1150-1156` → V2-2 and V2-3 both correct, the critic's N2 is indeed wrong; `note.cpp:867-892` fires the skip-IC-count keyboard + `ReportSkipICCount` on **any** `K_SKIP` with `bAskSkipICCount`, comment "Deliberately NOT gated on the alarm being a JAM* code" → V2-1 correct, and `system/General.ini:26 AskSkipICCount=0` today; `csystem.cpp` includes neither `uHGemEquipment.h` nor anything exporting `HGem` (extern is at `uHGemEquipment.h:449`) while `uAgvStation.cpp:19` does → `IsAmrCallableNow()` must live in the coordinator, correct; `MachineType.h:7` is `//#define SOFT_SIMULATE` in the worktree → V2-10 correct; `system/AlarmList.csv` is 487 lines (1 header + 486 rows), WAR0963 at line 478, WAR0970 at 479 → the insert point and 486→487 are correct; `ComputeAlarmAlid` class 2 + 4-digit tail 0964 → **200000964** confirmed (`UsecegemMainFrom.cpp:185-300`); `ReleaseStationByOperator` hardcodes "WAR0963" at `:300-301` and does not touch `TrayCount/DeviceCount` → N6/N7 fixes correct.

====================== newly_introduced ======================
The fixes themselves create these. NI-1 must be fixed before coding; NI-2 and NI-3 are reachable customer-visible SECS defects.

**NI-1 — the stuck-lot state described under B2 above.** Caused specifically by the B2 fix (announce with no base precondition + SKIP that only clears the latch). v1's base precondition accidentally made this unreachable.

**NI-2 — `SoftStart` survives the modal, so the give-up close now RESTARTS the machine, and the restart re-enters Clean Out and emits a spurious CEID 42 after CEID 8.**
`ShowNoteAlarm` sets `SoftStop=false; SoftStart=false;` (`note.cpp:811-812`), then `BtnStartClick` sets `SoftStart=true` (`note.cpp:513-514`) — and the operator *must* press Start or Pause to close a keyed modal. v2's close leg then runs `CloseCleanOutForAmr` (`DoLotEndProcess` sets `SoftStop=true`) + `ChangeRunMode(Run_Normal); SoftStop=true;`. But `ProcessStartMode` (`csystem.cpp:1337-1354`) tests **`if(SoftStart)` first**: `SoftStart=false; SoftStop=false; SystemStart=true;`. So the next cycle the machine is RUNNING in `Run_Normal` with the work order just cleared. Then `aLoader.cpp:1878-1886` — whose only gate is `RunMode==Run_Normal && bAmrLocked==false && IsSupplySourceDry()` — re-arms `FeedWaitTimer` and after `iAmrFeedWaitSec` (60 s on site, `system/General.ini:129`) sets `bCleanOut=true; RunMode=Run_CleanOut` again. Everything is already drained, so the ladder completes and `csystem.cpp:1910` fires a **second** `EmitCleanOutOK()` (CEID 42) plus a second `DoLotEndProcess` (duplicate "LOT END" line, `tRunData.LotEndTime`/`UPH` recomputed over a longer span so UPH decays, a second `TrayUphLog_OnLotEnd` row, another `WriteLastDataIni`); `EventReport(SECS_EVENT.DoLotEnd)` is skipped only because `LotRegistry.GetLotCount()>0` is now false (`main.cpp:3345-3346`). Net wire order seen by KYEC: 42, 8, 42. This cannot happen today because the AMR clean-out close never shows a modal, so `SoftStart` is never set. Fix: force `SoftStart=false` in the close leg (or make WAR0964's close leg not depend on the operator's Start/Pause choice).

**NI-3 — the manual Lot End button / SECS `CLEAR_LOT_INFO` during the wait produces a duplicate Lot End and an out-of-order CEID 42.** §5 item 3 deliberately leaves both un-gated, and §8 H9 claims "`bCleanOut` 與 latch 由隨後的冷 `InitialAllTask()` 清乾淨". **There is no `InitialAllTask` on that path** — `btnLotEndClick` (`main.cpp:3251-3258`) is a thin shell that calls `DoLotEndProcess()` and nothing else, and `DoLotEndProcess` never touches `HSys.Sys.bCleanOut` or `RunMode` (verified over `main.cpp:3260-3380`; the only `Sys.bCleanOut=false` writers are `csystem.cpp:1927`, `:1938`, `database.cpp:672`). So after a manual Lot End during the wait: `RunMode` is still `Run_CleanOut`, `bCleanOut` still true, the latch still owed, the deadline still running → WAR0964 still pops and stops a machine whose lot the operator already closed → SKIP → base true → `CloseCleanOutForAmr` runs `DoLotEndProcess` a **second** time, and `EmitCleanOutOK()` puts CEID 42 **after** CEID 8. §4.C-3's "恰好一次的證明" only covers the two AMR-branch paths; it does not cover interleaving with the two paths v2 explicitly left un-gated. H9/H10 expectations must be corrected and the close leg must be idempotent against an already-closed lot (e.g. skip if `LotRegistry.GetLotCount()<=0 && m_sActiveLot==""`).

**NI-4 — the announce can now freeze the control loop for 60 s with motors still enabled and other modules still moving.** Removing the base precondition (the B2 fix) means the announce can fire while `TrayArm`/`Empty`/`Color` are mid-drain (that is exactly test H13). v2's order is snapshot **then** alarm, and `CompressFolder` blocks on `WaitForSingleObject(...,60000)` (`cStateRecordHT160.cpp:600`) with no message pump, on the machine-control thread — so `MainProc` stops servicing motion for up to a minute *before* `ShowNoteAlarm` reaches `DecStopAllMotor()`. Reorder: pop the alarm (which stops the machine) first, snapshot after; or gate the snapshot on `SystemStart==false`.

**NI-5 — the WAR0962 storm + retry-budget bypass** (detailed under B2 above). The suppression's structural cap converts one deferred alarm into up to six sequential stop modals at the announce instant.

**NI-6 — WAR0964's text omits the AMR-clear instruction** while its SKIP releases the lock and the only way to close it resumes motion (detailed under B4).

**NI-7 (minor) — SKIP can leave a transiently corrupt car ledger.** In the H13 late-delivery case, `SetRearHasTrayFromTrayArm` (`aAuto1To6.cpp:663-666`) cleared this station's drain latch; after SKIP's `ClearAmrCar` zeroes `Car[]` and re-seeds roles via `InitAutoCarStack`, the re-collect restacks the late tray with `iTrayCount=1`, `CarID=""`, and `Tray[0]` mis-typed (`aAuto1To6.cpp:852-861` copies `WorkingKind` from `RearKind`, so the normal tray overwrites the identity slot). It is cleaned by the eventual cold `InitialAllTask()` inside `CloseCleanOutForAmr`, so it is transient — but the EventLog line written before the clear will not match the physical car afterwards.

**NI-8 (minor, buildability) — S5/S7 need includes the plan does not list.** `csystem.cpp` contains **zero** references to `g_EventLog` today (grep), and does not include `SecsGem/UsecegemMainFrom.h` (where `void AlarmReport(...)` is declared at `:35`, needed if OD-2 branch B is chosen). The plan's per-step "independently buildable" claim needs those two lines.

**NI-9 (fragile ordering the plan must state as an invariant).** D4 works only because of an unstated tick order: `ProcessMotion()` — which hosts the `:1908` finish dispatcher — is called at `csystem.cpp:381`, **before** `DataModule1->DoAllProcess()` at `:415` (which runs the Auto ladder to case 7000), and the new gate would be at `:425`, after it. If anyone later moves `ServiceCleanOutHandoverGate()` above `ProcessMotion()`, the dispatcher sees a finished base with the latch not yet armed and closes the lot in the same cycle — D4 becomes a silent no-op with no test failing. (Today there is extra margin because TrayArm/Empty/Color derive their finish from Auto's, so base cannot be true on the case-7000 cycle — but that margin is incidental, not designed.) Write the ordering requirement next to the call site.

====================== residual_risk ======================
Acceptable, but the plan must say all of this in writing.

1. **The plan is only sound conditional on OD-2.** With the recommended firmware default (`CleanOutHandoverGiveUpAction=0`, ask a human), an unmanned line stops until someone arrives — escape #3 is a human, not a mechanism, and I verified the Note form cannot self-dismiss (`note.cpp:498-560`). If OD-2=B is chosen, the S5F1 set-then-clear is mandatory, not optional: EventLog alone on an unmanned line is the "silent stop" the owner banned.

2. **OD-1 branch B leaves a real data-loss window.** If nothing is moved forward, the Soter per-unit CSV and the FTP publish job are exposed for `1800 + 5×1800` s, and indefinitely if nobody answers — and §6/R11's own finding is that "host never answers `AUTOn="Action"`" is the *default* KYEC outcome, so that is the normal path, not the edge. Branch A costs two lines and I confirmed it loses nothing (Soter rows come only from `aSortArm`, which is finished before arming).

3. **`SnAutoX_InputEnd` is still unmeasured and is still the only release for CEID 274** (`aAuto1To6.cpp:1697-1707`). P1 must stay a hard on-machine prerequisite. If it never turns OFF, every handover ends on the (b) deadline, i.e. the give-up path becomes the *only* path — which makes NI-6 (alarm text) and B4-(b) (handshake gate) upgrade from "fix" to "critical".

4. **Each announce costs one full State Record snapshot: a synchronous whole-day log copy + 7-Zip with a 60 s cap** (`cStateRecordHT160.cpp:596-606`, growth called "quadratic" in the file's own comment at `:478-481`). One per episode is the right budget; do not let RETRY buy another.

5. **Laptop/CI behaviour changes and must be in the SOP.** `system/General.ini:3 UseAMR=1`, `MachineType.h:7` is currently commented out (V2-10), the ledger gate is `bUseAMR` not `!IsSoftSimulate()` (`aAuto1To6.cpp:847`), `PollAndCall` returns at `bSelected==false` (`:445-469`) and `AGV_CALLED→AGV_PREP` only happens in the host's `BeginPrep`. So an unattended sim clean-out ends in a stopping WAR0964 after `iAgvTimeoutSec`. Any headless regression must set `UseAMR=0` or attach the simulator; `--selftest-home` is unaffected.

6. **Wall-clock consequences to state to the customer with numbers, not adjectives:** CEID 42→CEID 8 goes from 28 ms to up to `1800 + 5×1800` s, unbounded if unanswered; P1-P3 infeed `START_AGV` rejection (`uAgvStation.cpp:806-810`, by design) lengthens by the same amount; the number of cars called per lot doubles under OD-4=A (identity + cover trays alone make `iTrayCount==2`); if OD-1=A the Soter CSV filename stamp moves earlier by the handover span; if OD-5=A, WAR0964's SKIP will not emit CEID 78.

7. **OD-5 should be widened.** `note.cpp:867-892` is not a WAR0964 problem, it is a policy hole: WAR0964 is simply the first AMR-logistics alarm to offer `K_SKIP`. Ask the field for the live `[SECS] AskSkipICCount` value and decide the family rule once.

8. **Minor citation drift in v2 itself** (fix before the doc is filed, so the next reader can follow it): the `bCleanOutFinish=false` in `SetRearHasTrayFromTrayArm` is at `aAuto1To6.cpp:664`, not `:665`; `ArchiveWorkOrderToLotStory`'s early return is `main.cpp:3619-3620`, not `:3609-3610`; `DoLotEndProcess`'s stop trio is `main.cpp:3269-3272`, and its `g_SoterOutput.OnLotEnd()` / `WriteLastDataIni()` are at `:3302`/`:3304` only in the pre-edit numbering — re-verify at implementation time; §4.C-2 step 5(c) writes "csystem.cpp:2368-2384(main.cpp)" for what is `main.cpp:2368-2384`. Everything numerically load-bearing (486→487 rows, ALID 200000964, 57.367×6=344.202 s, the [60,7200]/[0,5] clamps) checks out.

9. **§8.5's adversarial re-check must add three cases the current list misses:** (i) case (c) with base false — assert the lot actually ends; (ii) `SoftStart` state after every close leg — assert the machine does not resume with an empty work order; (iii) manual Lot End / `CLEAR_LOT_INFO` during the wait — assert exactly one `DoLotEndProcess` and no CEID 42 after CEID 8.

