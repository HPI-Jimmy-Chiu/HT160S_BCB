> ## 文件狀態（2026-09-07）
>
> **本文是 D4 解耦架構（R-A..R-D）的設計草案，第三輪對抗式複驗判 UNSOUND，不可照著實作。**
>
> - **R-D（逾時自動重送）已由使用者裁定取消** —— 本文 §4.4 整節作廢， 維持現況。
> - **R-A §4.1 的「變更點：無」是錯的**，**R-C §4.2 的新述詞在它要服務的狀態下必為 false**。
>   兩者的推翻理由與現場實證見
>   `docs/plan/HANDOFF-kyec-20260904-followups-20260907.md` **§3.0**。
> - 仍然有價值的部分：RPTID 2001 三十個 SVID 的逐欄對照、host linkage 是權威而非 firmware 預設、
>   `IsLotIdentityFrozen()` 當作叫車窗口的理由、以及 2026-09-03 那顆 WAR0962 `PauseTime=12180`
>   （3 小時 23 分）的現場代價。
>
> **接手請先讀 HANDOFF，再把本文當設計素材。**

---

# AMR 退料後解耦叫車計畫 (R-A..R-D) — 20260907

> 依 owner 20260907 指示重寫。**推翻並取代**舊 v1/v2 設計
> (`docs/plan/cleanout-amr-lotend-collect-all-cars-plan-20260907.md`) 的「等 AMR 才 Lot End」架構。
> 本文所有數字皆親自從授權樹 `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0` 與兩顆現場 State Record
> (`2026-09-04 15_24_21`、`2026-09-04 15_50_24`) 讀出並附 file:line。

---

## 1. 背景

Owner 原話:

> 「主要問題來源是機台都退料出來,又要等 AMR 派車來且能夠提供正確的 lot number 或其他資訊。
> 按照現況,全部退出後,Lot end + 保留可交換用資訊 + Auto 有偵測到盤就送 272
> (超時就重送,直到沒盤),可行嗎?」

答案:**可行,而且大部分已經是機台今天的行為**。2026-09-04 現場實測:

| 時間 | 事件 | 出處 |
|---|---|---|
| 15:29:19.882 | CEID 42 CleanOutFinish (TX) | `SecsLog/2026_09_04/SECSGEM_TextLog_15.txt:2305` |
| 15:29:19.886 | `PROCESS "LOT END auto (CleanOut finish)"` | `EventLog/HT160S_2026_09_04.csv:58` |
| 15:29:19.910 | CEID 8 Lot End (TX) — 距 CEID 42 **28 ms** | `SECSGEM_TextLog_15.txt:2358` |
| 15:44:13.360 | CEID 272 (Lot End 後 14 分 53 秒) | `:2414` |
| 15:44:18.361 | CEID 272 (間隔 **5.001 s**) | `:2514` |
| 15:44:27.363 | CEID 272 (間隔 **9.002 s**) | `:2614` |
| 15:49:27.359 | `AGV: handshake timeout P5 (Auto2) after 300s -> WAR0962 pending` (299.996 s) | `EventLog/HT160S_2026_09_04.csv:62` |
| 15:50:21.542 | `AGV: WAR0962 RETRY - station P5 reset to IDLE for re-call` | `:68` |
| 15:50:22.364 | CEID 272 (RETRY 後 **0.822 s**) | `:2810` |

也就是說 **R-A 與 R-C 的骨幹已經在跑**,SystemStart=0 也照跑
(`MachineState.ini` 15:50:24: `RunMode=0 / RunModeName=Normal / SystemStart=0 / LotNo=`)。
真正缺的是三件事:

1. **payload 是空的**。15:50:22 那封 272 的 RPTID 2001 三十個欄位全部 `0` 或 `""`,
   RPTID 502 的 Lot ID 是 `<A[0] "">`(全文見 `SECSGEM_TextLog_15.txt:2810-2878`)。
2. **不 full 但有盤的車叫不出來**(Lot End 後 `bCollect` 兩個條件同時死掉,詳 §4.2)。
3. **超時不是重送,是停機**。`note.cpp` `ShowNoteAlarm` 會 `DecStopAllMotor()` + `SystemStart=false`
   再 `ShowModal()` 卡住等人按鍵。2026-09-03 11:06:24 那顆 WAR0962 到 14:29:25 才有人按,
   `PauseTime` 欄 = **12180**(3 小時 23 分)—— `EventLog/HT160S_2026_09_03.csv:663` 與 `:879`。
   同日 `grep -c WAR0962` = **25 列**。

---

## 2. 使用者架構 (R-A .. R-D)

| 代號 | 需求 | 本計畫的定位 |
|---|---|---|
| **R-A** | 機台照今天排空,**Lot End 立刻發**。`CheckCleanOutFinish` 不加閘、不放棄式 modal。CEID 42 → CEID 8 維持 28 ms。 | **零改碼**,轉為可稽核的不變式(§7) |
| **R-B** | AMR 交換要用的資訊(lot number 與「其他資訊」)在 Lot End 後**保留**,不歸零。 | 一半已 SHIPPED(f799d88),**缺 18 個 SVID**(§4.3) |
| **R-C** | Auto **偵測到盤**就發 CEID 272 叫車。 | 加**第三個觸發項**(§4.2) |
| **R-D** | 超時就**重送**,重複到盤沒了為止。 | 把 `TimeoutPending` 改成自我重送(§4.4) |

---

## 3. 為何比舊設計好

| 舊 v1/v2(已推翻) | 本設計 |
|---|---|
| Lot End 等 AMR → 「lot 永遠結不掉」 | Lot End 不等 → 該缺陷**不存在** |
| 放棄式 modal → 與 SoftStart 互撞、重複 Lot End | 沒有 modal → 該缺陷**不存在** |
| 需要 WAR0964 新警報碼 | 不需要(§10 待裁定 D3 例外) |
| K_SKIP / CEID-78 路徑風險 | 不觸及 |
| 要動 `csystem.cpp` 的 clean-out 收尾 | `csystem.cpp` **一行都不動**(§7) |
| Lot 生命週期與 AMR 生命週期綁死 | 兩者**解耦**:lot 走 lot 的、車走車的 |

代價只有一個,而且必須明講:**Lot End 後一台 Auto 可能一直被鎖住到 AMR 來**(§5.1)。

---

## 4. 現況 + 變更點

### 4.1 R-A:已經成立,改成不變式

* `csystem.cpp:1908` `if(HSys.Sys.RunMode==Run_CleanOut)` → `:1910 CheckCleanOutFinish()`
  → `:1914 EmitCleanOutOK()` → `:1915 if(GeneralSetting.bUseAMR)` → `:1928 DoLotEndProcess("auto (CleanOut finish)")`
  → `:1929 InitialAllTask()`;`:1952 ChangeRunMode(Run_Normal)`、`:1953 SoftStop=true` 在 if/else **外面**,兩支共用。
* `csystem.cpp:2031-2055 CheckCleanOutFinish()` 內**沒有任何** `bUseAMR` / `AgvCoord` 字樣 —— 六個模組 finish + `HasICUnderMachineForCleanOut()`,純機構條件。
* `main.cpp:3278 DoLotEndProcess()` → `:3341 EventReport(SECS_EVENT.DoLotEnd)`(只被 `LotRegistry.GetLotCount()>0` 守住),
  無 AMR 閘、無等待。

**變更點:無。** R-A = 「不要加閘」。列入 §7 不變式清單。

### 4.2 R-C:第三個觸發項

**今天的觸發布林**(`SecsGem/uAgvStation.cpp:530-537`):

```
bool bTrueFull = AutoModule->IsOutputCarFullForAmr(a) || AmrInject.AutoFull(a);   // :530
bool bCollect  = IsCleanOutCollectDueForAmr(a);                                   // :536
bool bFull     = bTrueFull || bCollect;                                           // :537
```

* `bTrueFull` → `aAuto1To6.cpp:1589-1597`,真機讀 `SnAutoX_InputFullTray`,sim 用盤數門檻。
* `bCollect` → `uAgvStation.cpp:413-431`,結尾是 `IsFrontHasTrayForAmr` = `SnAutoX_InputHasTray`
  (`aAuto1To6.cpp:1615-1623`,sim 一律 false)。

**Lot End 後 `bCollect` 必然為 false,兩個獨立原因:**

1. `csystem.cpp:1952` 把 RunMode 打回 `Run_Normal`,而 `uAgvStation.cpp:417` `if(HSys.Sys.RunMode!=Run_CleanOut) return false;`
2. `aAuto1To6.cpp:92` `State[Index].bCleanOutFinish=false;` 在 `bKeepMaterial` early-out(`:100-101`)**上面**,
   所以 `InitialAllTask()` 一定清掉,`IsStationCleanOutFinish` 回 false。

**結論:Lot End 後只有 `bTrueFull` 能發 272。** 這就是「有盤但沒滿」的車被困住的機制。
而現場證明兩支感測器**真的會不一致**:15:50:24 `SnAuto2_InputFullTray` Live=1 但
`SnAuto2_InputHasTray` Live=0(`IoDetail.txt:87-88`,同一顆 `Lane0 IP3 P2` 的 B2/B3),
`SortArmDecision.txt` 同刻 `[Auto2] FullVerdict=1 InputFullSn=1` 而 `CleanOut: FrontSn=0`。
所以 **R-C 必須是兩支的 OR,不能挑一支**。

#### 新增述詞

`SecsGem/uAgvStation.cpp`,緊接 `IsCleanOutCollectDueForAmr`(`:413-431`)之後新增:

```cpp
// AI(amr-afterlot-collect) 20260907 : R-C - THIRD reason to call the AMR.
// bTrueFull = throughput (car filled mid-run). bCollect = the CleanOut handover window.
// This one = "the lot has ENDED, the machine is idle, and this Auto car still holds trays".
// Owner ruling 20260907 : the lot must NOT wait for the AMR, so after Lot End the car is
// still loaded and nobody is calling. bCollect cannot cover it - RunMode is Run_Normal by
// then (csystem.cpp:1952) and State[].bCleanOutFinish was cleared (aAuto1To6.cpp:92).
// WINDOW = IsLotIdentityFrozen(), i.e. exactly Lot End -> next Start. Reusing that flag
// instead of inventing a second epoch is deliberate : the retained SVID snapshot (R-B) and
// this call window MUST NOT be able to disagree, or a 272 ships outside the frozen window
// and carries live (next-lot) values.
// A plain operator PAUSE does NOT arm it (only DoLotEndProcess does, main.cpp:3295), so a
// mid-lot pause with trays on a car raises nothing.
// SENSOR = InputHasTray only. InputFullTray is already carried by bTrueFull, so the OR at
// the call site is the OR of both points - which the 2026-09-04 Auto2 disagreement requires.
// OutputBottomHasTray is deliberately EXCLUDED : it describes the rear stage, not the car.
// NOT SnAutoX_InputEnd - see the plan's hazard H1.
bool TAgvCoordinator::IsHoldingTraysAfterLotEnd(int AutoIndex)
{
    if(GeneralSetting.bUseAMR==false)
        return false;
    if(AutoModule==NULL)
        return false;
    if(HSys.Sys.RunMode!=Run_Normal)      // the CleanOut window belongs to bCollect
        return false;
    if(IsLotIdentityFrozen()==false)      // only between Lot End and the next Lot Start
        return false;
    return AutoModule->IsFrontHasTrayForAmr(AutoIndex);   // live, Enable-guarded, sim=false
}
```

宣告加在 `SecsGem/uAgvStation.h`(緊鄰 `IsCleanOutCollectDueForAmr` 宣告)。
`cprod.h` 已在 `uAgvStation.cpp:30` include 過(f799d88 帶進來的),**不需要新 include**。

呼叫點改 `uAgvStation.cpp:537`:

```cpp
bool bAfterLot = IsHoldingTraysAfterLotEnd(a);   //AI(amr-afterlot-collect) 20260907 : R-C
bool bFull     = bTrueFull || bCollect || bAfterLot;
```

**為什麼窗口用 `IsLotIdentityFrozen()` 而不是 `SystemStart==false`:**
`SystemStart==false` 在單純暫停時也成立,會讓產線中途暫停就叫車。
`IsLotIdentityFrozen()` 只由 `DoLotEndProcess` 開(`main.cpp:3295`,三條 Lot End 路徑共用:
操作員按鈕 `main.cpp:3272`、AMR clean-out finish `csystem.cpp:1928`、SECS `CLEAR_LOT_INFO`
`uHGemHT160.cpp:2444`),由 `main.cpp:2368`(`DoStartArm`,裸 START)與 `main.cpp:2729`
(`LotStartCore`)關。這正是「lot 已結、下一 lot 未開」的窗口。

### 4.3 R-B:到底哪些值必須活過 Lot End

**權威是 host 的 linkage,不是 firmware 預設。** 2026-09-04 客戶 EAP 用 S2F35 把
CEID 272 綁到 `{502, 2000, 2001}`(`SECSGEM_TextLog_15.txt:998-1008`);
273/274/275 只綁 `{502, 2000}`(`:1020-1072`)。
**RPTID 2001 只掛在 272 上**,所以 Lot End 後的 payload 工作只會經由 272 到客戶眼前。

RPTID 2001 的 30 個 SVID 順序(`SECSGEM_TextLog_15.txt:957-986`,親自核對過 body 的
`12×I4 → 3×A → 6×I4 → 9×A` 型別分佈完全吻合):
`38222,38223,38224 | 38225,38226,38227 | 38228,38229,38230 | 38231,38232,38233 |
38234,38235,38236 | 38246,38247,38248 | 38240,38241,38242 | 38243,38244,38245 | 66040..66045`

#### CEID 272 完整 payload 表

| SVID | 背後變數 | Lot End 清掉? | f799d88 已保? | 用途 |
|---|---|---|---|---|
| **502** | | | | |
| 1006 Lot ID | `svActiveLot` | 是(來源 `LotRegistry.Clear()` `main.cpp:3376`) | **是**(`uHGemHT160.cpp:859`) | 這批貨屬於哪個 lot |
| 1007 Operator ID | `GeneralSetting.sOperatorID` | 否 | n/a | 追溯 |
| 1011 Machine State | `svMachineState` | 否 | n/a | 15:50:22 讀到 `<A[4] "HALT">` |
| 3 GemClock | `sGemClock` | 否 | n/a | 時戳 |
| 1501 Setup File | `ecRecipeName` | 否 | n/a | 配方 |
| 1517 Start Mode / 1518 Real-Dummy | `svStartMode` / `iRealDummy` | 否 | n/a | 背景 |
| **2000** | | | | |
| 38202 | `CarrierID[0]` | 否(只 `ReportLoaderIdentity` 寫) | n/a | 身分盤 2D |
| **38205-38207 / 38199-38201** Auto1-6 Carrier ID | `CarrierID[3..8]` ← `Car[a].CarID` | **是** | **否** | 疊盤身分 |
| **38219 Supplement bitmap** | `SupplementBitmap`,發射時重建(`uAgvStation.cpp:542`) | 否 | n/a | **哪一站在叫車 — 最關鍵的欄** |
| 38220 / 38221 | `StatusBitmap` / `FinishBitmap` | 否(沿用上一次 273/274) | n/a | 上次交握 |
| **2001** | | | | |
| 38222 Loader Tray Count | host `START_AGV` CP 帶進、consume-once | 否(消耗即歸零,非 Lot End) | n/a | trip 帳 |
| 38223,38224,38228,38229,38230 | 從未寫入(保留) | n/a | n/a | 無 |
| **38225-38227 / 38246-38248** Auto1-6 Tray Count | `TrayCount[3..8]` ← `Car[a].iTrayCount - 表頭` | **是** | **否** | 車上盤數 |
| **38231-38233 / 38240-38242** Auto1-6 Device Count | `DeviceCount[3..8]` ← `iAmrDeviceCount[a]` | **是** | **否** | 車上 IC 數(下料 IC-count 契約) |
| 38234-38236 / 38243-38245 Bin Setting | `BinSetting[0..5]` ← `DescribeAutoBins` | 是 | **是**(`uAgvStation.cpp:1046-1049`) | lane class/bin |
| 66040-66045 Lane Lot No | `LotNumber[0..5]` ← `DescribeAutoLot` | 是 | **是**(`uAgvStation.cpp:1096-1099`) | 這條 lane 為哪個 lot 分料 |

**缺口正好是 3 個家族 × 6 lane = 18 個 SVID**:Auto Carrier ID / Tray Count / Device Count。
清除鏈路(親自逐層驗過):
`csystem.cpp:1929 InitialAllTask()`(預設 `bKeepMaterial=false`)
→ `database.cpp:78 AutoModule->InitialFlag(bKeepMaterial)`
→ `aAuto1To6.cpp:119-120 Car[Index].Clear(); InitAutoCarStack(Index);`(在 `:100-101` early-out **下方**)
→ `MotorAndIO/MyMotor.cpp:274 CarID="";`,以及 `aAuto1To6.cpp:1377 iAmrDeviceCount[Index]=0;`

#### R-B 的重要性排序(照「叫車能不能用」排,不照工程直覺排)

1. **SVID 38219 bitmap** —— 唯一必要欄,而且**已經活著**(發射時重建)。
   現場鐵證:2026-09-04 全天七封 RPTID 2000 裡**七個 carrier id 全部是空字串**,而
   `START_AGV` 照樣成功:host 用**站名**下令
   (`SECSGEM_TextLog_15.txt:1079-1129`,`{"Loader","Action"},{"AUTO1".."AUTO6","NA"}`,
   由 `uAgvStation.cpp:345-354 LookupByName` 解),HCACK=0 在 `:1140`,
   CEID 273 `:1141`(15:22:34.356)、CEID 274 `:1206`(15:23:31.364) 全部走完。
   **所以 R-C 就算帶著全零 payload 也叫得到車。**
2. **66040-66045 lane lot number** —— 客戶 20260907 直接點名的欄,f799d88 已覆蓋。
3. **TrayCount / DeviceCount** —— MES / trip 對帳用。**保留,但絕不可拿來當叫車或重送的閘門。**
4. **CarrierID** —— 價值最低。唯一寫入點 `aAuto1To6.cpp:860 Car[Index].CarID=WorkingTrayID[Index];`,
   而 `aAuto1To6.cpp:1380-1387` 的 20260907 註解已記載 `WorkingTrayID` 不可靠
   (「2026-09-04 Soter CSV 對一台 Auto2 車給出三個不同值」)。為對稱而保留,不要在它上面蓋東西。

#### 變更點 S1a(最小、可獨立出貨,約 8 行)

`TAgvCoordinator::Reset()` **沒有任何 runtime 呼叫者** —— 全樹 grep `AgvCoord.Reset` 只命中
`uHGemHT160.cpp:2859` 的註解;`uAgvStation.cpp:162-165` 建構子是唯一使用者。
所以這三個陣列**結構上已經免於 Lot End**;唯一把它們吹空的是 `PollAndCall` 每 1 秒從被清空的
`Car[]` 重抄一次。把 f799d88 現成的 sticky 規則套到那三行即可:

`SecsGem/uAgvStation.cpp:525-527`(現況)

```cpp
TrayCount[si] = iWork;                                    // :525
DeviceCount[si] = AutoModule->GetAmrDeviceCount(a);       // :526
CarrierID[si] = Car->CarID;                               // :527
```

改為(照抄 `:1046-1049` / `:1096-1099` 的既有慣例):

```cpp
// AI(amr-afterlot-payload) 20260907 : STICKY inside the frozen window (Lot End -> next Lot
// Start), same rule RefreshBinSettings/RefreshLotNumbers already use for 38234-38245 and
// 66040-66045. InitialAllTask wipes Car[] at Lot End (aAuto1To6.cpp:119-120 ->
// MyMotor.cpp:274), so from the next 1 s tick these three columns publish "" / 0 and every
// post-Lot-End CEID 272 carries an identity-less payload. Field: 2026-09-04 15:50:22, all
// 30 items of RPTID 2001 were 0 or "" (SECSGEM_TextLog_15.txt:2810-2878).
// SEMANTIC WRINKLE, on purpose : for the two int columns 0 is a LEGITIMATE value (a genuinely
// empty car), unlike "" for a string, so this is sticky-when-ZERO and could hold a stale
// number for a car that really emptied. Acceptable only because (a) the window ends at the
// next Lot Start, and (b) an empty car raises no trigger, so no 272 ever ships those numbers.
// The post-274 zeroing at :700-701 stays UNCONDITIONAL - at 274 the car really did leave.
bool bHold = IsLotIdentityFrozen();
if(iWork!=0 || bHold==false)
    TrayCount[si] = iWork;
int iDev = AutoModule->GetAmrDeviceCount(a);
if(iDev!=0 || bHold==false)
    DeviceCount[si] = iDev;
if(Car->CarID!="" || bHold==false)
    CarrierID[si] = Car->CarID;
```

同時 `uAgvStation.cpp:692-696`(274 發射前的最後快照)套同一條規則,否則收尾的 274 會把
sticky 值覆寫回 0,與前面幾封 272 自相矛盾。
(附帶觀察,不在本次範圍:`:694` 寫的是**物理**盤數 `FinishCar->iTrayCount`,沒有做
`:518-525` 那個「工作盤 = 物理 − 表頭」的減法。今天客戶沒把 2001 綁到 274,所以看不見,
但這是既存的不一致。)

#### 變更點 S1b(隨 R-D 出貨:per-station payload PIN)

S1a 只覆蓋「Lot End → 下一次 Start」。一旦 R-D 讓一次 episode **跨過** Lot Start,
`RefreshLotNumbers`(`uAgvStation.cpp:1089`,每 tick 跑、且刻意不受 `bUseAMR` 管)就會把
66040-66045 換成**新 lot** 的號碼,而那台車上的盤屬於**舊 lot** —— host 收到重送的 272 會拿到錯的 lot。

作法:在該站發出**第一封** 272 時 PIN 住 payload,episode 期間停止刷新。
SVID 是**指標綁定**到這些陣列的(`uHGemHT160.cpp:742-744`、`:749`、`:760`
`SetSVDataPointer(..., &AgvCoord.CarrierID[ai], ...)` 等),所以「凍結陣列」等於「凍結 payload」,
不需要另一套 publish 間接層。

* `uAgvStation.h`:新增 `unsigned char PayloadPinned[AGV_AUTO_COUNT];`
* `uAgvStation.cpp:541-554` CALL 分支:`PayloadPinned[a]=1;`(在 `EventReport(1,272)` 之前)
* `uAgvStation.cpp:518-527` 每 tick 抄寫:`if(PayloadPinned[a]) ` 時跳過
* `uAgvStation.cpp:1042-1051 RefreshBinSettings` / `:1093-1101 RefreshLotNumbers`:pinned lane 跳過
* 解除 PIN 的**三個**點:(a) `:556-561` release 分支(觸發轉 false);
  (b) `:699` `ClearAmrCar` 之後;(c) `ReleaseStationByOperator`(`:292-308`)

### 4.4 R-D:重送

**今天的超時機制**(全部親自讀過):

* 節拍是**純 VCL TTimer 1000 ms**:`SecsGem/uHGemEquipment.cpp:35-37` 建立、`Interval=1000`,
  `:211-212 Timer1Timer -> GemLogic->ServiceAgv()`,而且在 `:230` 的 `bWantComm` 守衛**之前**。
  → `uHGemHT160.cpp:910-922 ServiceAgv` 依序跑 `RefreshBinSettings / RefreshLotNumbers / PollAndCall / ServiceHandshake`。
* 計時是**數 tick**,不是時鐘:`uAgvStation.cpp:712-726`
  `if(++ShortageDebounce[si] > GeneralSetting.iAgvTimeoutSec && TimeoutPending[si]==0){ ... TimeoutPending[si]=1; ShortageDebounce[si]=0; }`,
  `else { ShortageDebounce[si]=0; TimeoutPending[si]=0; }`。
  現場驗算:CALL 15:44:27.363 → pending 15:49:27.359 = **299.996 s**,`iAgvTimeoutSec=300`
  (`GeneralSetting.cpp:141` 預設、`:281` 讀取、`:308` clamp≥5;現場 `system/General.ini:132 AgvTimeoutSec=300`)。
* 唯一消費者把它變成停機:`csystem.cpp:135-193 ServiceAgvTimeoutAlarm` → `:183 ShowMyError("WAR0962",...,K_RETRY)`
  → `note.cpp` `ShowNoteAlarm`:`DecStopAllMotor()`、`SystemStart=false`、`AlarmReport(set)` 發 S5F1、
  `ShowModal()` **阻塞**。`K_RETRY` → `AgvCoord.RetryStation(si)`(`:185`)。
  此 sweep 掛在 `csystem.cpp:423`,在 `:416-417` 的 `SystemStart` 守衛**外面** —— 所以今天在已停機的機台上
  照樣會彈,2026-09-04 15:49:27 就是 Lot End 後 20 分鐘彈的(`EventLog:62-63`)。
* 一次 episode 的成本是 **4 列 EventLog + 一對 S5F1**(`EventLog/HT160S_2026_09_03.csv:986-991`
  完整一輪:pending / WAR0962 set / RETRY 答覆 / RETRY 處理)。同日 25 列 = 8 個 episode。

#### R-D 不需要新的計時機構

`RetryStation`(`uAgvStation.cpp:317-329`)**就是**需要的原語:清 `TimeoutPending` /
`ShortageDebounce` / `ShortageLatch`,`Handshake=AGV_IDLE`,而且**刻意不放鎖**
(`:312-316` 的註解說明:放掉 Auto 鎖會讓 `FindDischargeAuto` GoUp 進還沒清空的車)。
之後 `PollAndCall` 的 `bFull && Handshake[si]==AGV_IDLE`(`:539`)下一 tick 就重發。
現場證實這個閉環 0.822 秒完成。

#### 變更點

`uAgvStation.cpp:712-726` 的 Auto 老化區塊,把「latch 給 csystem」換成「自己重送」:

```cpp
// AI(amr-afterlot-resend) 20260907 : R-D - the Auto (P4-P9) collect timeout RESENDS instead
// of latching TimeoutPending for csystem's WAR0962 modal. Owner ruling 20260907.
// WHY the alarm must go for this case : ShowNoteAlarm does DecStopAllMotor() + SystemStart=
// false and then blocks in ShowModal, so "timeout" today means "stop the machine and wait
// for a human, for an unbounded time". Field: 2026-09-03 11:06:24 raised -> 14:29:25
// answered, PauseTime column = 12180 s (EventLog HT160S_2026_09_03.csv:663 / :879).
// The station keeps its LOCK and its trigger, exactly as RetryStation already does, so
// nothing moves into a loaded car between the reset and the re-CALL.
// P1 (Loader) keeps its silent force-release (:772-787) and P2/P3 (Empty/Color infeed) keep
// latching TimeoutPending for WAR0962 (:788-800) - R-D does not cover infeed.
if((Handshake[si]==AGV_CALLED || Handshake[si]==AGV_PREP || Handshake[si]==AGV_READY)
   && Handshake[si]==hsBefore)
{
    if(++ShortageDebounce[si] > GeneralSetting.iAmrResendSec)
    {
        ShortageDebounce[si] = 0;
        ResendAttempt[si]++;
        LogResendThrottled(si, a);          // see the log budget below
        Handshake[si]      = AGV_IDLE;      // PollAndCall re-CALLs next tick
        TimeoutPending[si] = 0;             // never reach csystem's WAR0962 for P4-P9
        // lock deliberately KEPT (same rationale as RetryStation :312-316)
    }
}
else
{
    ShortageDebounce[si] = 0;
    TimeoutPending[si]   = 0;
    ResendAttempt[si]    = 0;   // episode over : the handshake moved on
}
```

#### 節拍

**不可重用 `iAgvTimeoutSec`。** 那個 key 今天已經有四份工作:
`GeneralSetting.h:281` 註解列了三份(Normal full-collect / CleanOut collect / Empty-Color supply),
第四份是 `uAgvStation.cpp:246-248 ServiceLinkLostHold` 的 link-lost 上限。
若同時當重送週期,兩者就無法各自調。

新增 `[AGV] AmrResendSec`,預設 300,**clamp ≥ 30**,照 `GeneralSetting.cpp:279-282` /
`:305-308` / `:374-377` 的三處慣例(Load / clamp / Save)各加一行。

> **不要偷用現成的死 key `AmrFullWaitSec`。** `system/General.ini:130 AmrFullWaitSec=60` 存在
> 但全樹沒有消費者(`GeneralSetting.cpp` 不讀它)。現場檔案已經填了 60,悄悄接管會讓一個
> 沒人改過的設定改變行為。要嘛新開 key,要嘛先明確廢除舊 key。

**clamp ≥ 30 的理由**:它數的是 VCL tick 不是時鐘,負載下會延展。300 s 無所謂,10 s 以下就不能信。

#### 觸發確認與最短重叫間隔(**必須**,不是選配)

`TMySensor` **完全沒有 debounce**:`mysensor.h` 類別裡沒有任何 delay 成員,
`system/IO_Table.csv` 的 sensor 列末五欄(`OnAlarmTime,OffAlarmTime,OnDelayTime,OffDelayTime,Note`)
全部空白(第 44-79 行),只有 cylinder 才帶 OnDly/OffDly。
而 `PollAndCall` 的 release 分支(`:556-561`)會在 `bFull` 轉 false 時把站打回 IDLE ——
**所以感測器抖一下就是一整輪 CALL/release**。現場已經看到:15:44:13 → 15:44:18(5.001 s)
→ 15:44:27(9.002 s),三封 272 之間沒有任何 SECS 事件,是 `bFull` 自己 false 又 true。

因此加兩件事:

1. **N 連續 tick 確認**(建議 N=3):`bAfterLot` 只有連續 3 tick 成立才算真。用一個
   `unsigned char AfterLotConfirm[AGV_AUTO_COUNT]` 計數,任一 tick 為 false 就歸零。
2. **每站最短重叫間隔**:一個 `int RecallFloor[AGV_AUTO_COUNT]`,`IDLE → CALLED` 之後倒數,
   未到 0 之前不允許再 CALL。建議與 `AmrResendSec` 同值。

#### 上限:CALL 無上限,**通知有上限**

停止條件必須是**壞掉的感測器也能滿足的**條件。四條:

| # | 條件 | 出處 |
|---|---|---|
| (a) | 觸發轉 false → `PollAndCall` release 分支放鎖回 IDLE | `uAgvStation.cpp:556-561`。與啟動它的是**同一個布林**(`:531-537` 的註解已明訂「one value, one owner」) |
| (b) | AMR 到了 → CEID 274 → `ClearAmrCar` | `uAgvStation.cpp:685-704`;`aAuto1To6.cpp:1726-1735` 內 `bAmrLocked=false`、`bResidueClear=true` |
| (c) | 下一 lot Start 關掉窗口 → `bAfterLot` 轉 false → 走 (a) | `main.cpp:2368` / `:2729`。**這是第 4 題的答案,見 §5.1** |
| (d) | 第 N 次重送後,**一列** EventLog escalation(每 episode 一次),**CALL 繼續** | 新增,照 `aAuto1To6.cpp:1303-1331 ServiceCleanOutResidualWatchdog` 的慣例 |

#### EventLog 量

`iAmrResendSec=300` 時,一站無上限重送 = 86400/300 = **288 次/天**、8 小時班 = **96 次**。
每次一列 → 六站全卡 = **1728 列/天**。節流方案:

* episode 的**第一次**重送完整記一列;
* 之後**每 10 次**記一列(300 s × 10 = **每 50 分鐘**),訊息帶累計次數;
* episode **結束**無條件記一列(觸發清掉 / AMR 收走 / 操作員放行,分別註明)。

→ 一站一班約 **8 列**(對比今天一個 episode 4 列但可能伴隨 3 小時停機)。

#### Host 通知:**什麼都不加**

* 重送的 CEID 272 **本身就是**通知。它已經帶站別 bitmap(SVID 38219)與機台狀態 ——
  15:50:22 那封的 502 第 3 欄是 `<A[4] "HALT">`,host 收到重送就知道機台停著。
* 不開新 private CEID:9001-9099 私有段今天只有一個成員
  (`SecsGem/uHGemHT160.h` `HT160_CEID_LOTDATA_OK 9001`,註冊於 `uHGemHT160.cpp:1213`),
  新號碼會送到一台字典裡沒有它的 EAP(既有規矩:客戶機台自存 `.def` 是最高權威)。
* 不加 S5F1:WAR0962 的 S5F1 對是從 `ShowNoteAlarm` 內部發的(`note.cpp` 的
  `AlarmReport(set)` / `AlarmReport(clear)` → `UsecegemMainFrom.cpp:343-363 SendAlarmS5F1`),
  與停機**不可分割**。要單獨發就得寫新的發射點,不值得。

### 4.5 前置修正 S0:離散 Full CEID 必須改成**邊緣**觸發

**這是既存缺陷,R-D 會把它放大 288 倍,所以必須先修。**

`uAgvStation.cpp:541-554`:離散的 `AutoFullCeid[6]={35,36,37,148,149,150}`(`:498`)
emit 在同一個 `IDLE → CALLED` 分支內,只用 `if(bTrueFull)` 守(`:552-553`)——
守的是**位準**,不是**上升緣**。20260901 的 owner 裁定(引在 `:544-551`)說這六個只在
「真的滿」時發,**沒說**可以重複發。

現場:2026-09-04 一台 Auto2 車發了**四次** CEID 36,與四封 272 成對
(`SECSGEM_TextLog_15.txt:2414/2486`、`2514/2586`、`2614/2686`、`2810/2882`),
其中兩次只隔 5 秒。這與交付客戶的 workbook(`Handler_20260831` CEID 表把它們寫成「AutoN car full」)矛盾。

**修法**:每站一個 `unsigned char FullEdge[AGV_AUTO_COUNT]`,在 `bTrueFull` 的 false→true
轉態時 set,在 `bTrueFull` 轉 false 或 `ClearAmrCar` 時 clear;離散 emit 改成
`if(bTrueFull && FullEdge[a]==0){ ...emit...; FullEdge[a]=1; }`。**272 保持可重複** —— 那是 R-D 要的行為。

---

## 5. 危害與對策

### H1(最高)—— 絕不可把 R-C / R-D 綁在 `SnAutoX_InputEnd` 上

Brief 的既有事實 #1 說這支感測器「已經為此目的被信任」。**它不是。** 它在全樹只有**一個**消費者:
`TAutoModule::IsAmrTaken`(`aAuto1To6.cpp:1711-1721`),即 CEID 274 的**釋放**閘。
今天的 collect 觸發用的是**另一支** `SnAutoX_InputHasTray`(`aAuto1To6.cpp:1615-1623`),
throughput 觸發用的是**第三支** `SnAutoX_InputFullTray`(`:1589-1597`)。

三個具體理由:

1. **故障方向就是叫車方向,而且無法與真值區分。**
   `system/IO_Table.csv:44-79` 六支 Auto 感測器**全部** `InType=0`;
   `mysensor.cpp:50-53` `if(Type==1) bOn=Input->IsOn(); else bOn=!Input->IsOn();`;
   `myio.cpp:403-411` `if(ISABase==eIOBaseMotionNet && MN200ReadBit(iBit,&State)) return State; return bOutValue;`
   —— 讀取失敗回 `bOutValue`(input 從未寫過 → false),`InType=0` 再反相 → `TMySensor::IsOn()==true`
   =「有盤」。這個陷阱樹裡已經記載過(`aEmpty.cpp:240-245`)。
2. **爆炸半徑差六倍。** 六支 `InputEnd` 全在**同一個 port**:`Lane0 IP2 P1 B0..B5`
   (`IO_Table.csv:46,52,58,64,70,76`)。而 `InputHasTray`/`InputFullTray` 分散在六個不同 port:
   `IP3 P0`(Auto1)、`IP3 P2`(Auto2)、`IP4 P0`(Auto3)、`IP5 P0`(Auto4)、`IP5 P2`(Auto5)、`IP6 P0`(Auto6)。
   MN200 讀取是 port 粒度的(`myio.cpp:297-323 MN200ReadBit` 用成員 `iPort`)。
   → **一個 port 故障,InputEnd 會讓六條 lane 同時叫車;前側感測器最多影響一條。**
   (誠實補一句:同一條 lane 的 HasTray 與 FullTray 共用同一個 port,所以那個 OR 買到的是
   **覆蓋率**——B2 的不一致——**不是**容錯。)
3. **現場有一次無法解釋的全六支同時假亮。** 2026-09-04 15:24:21 六支 `InputEnd` 全 Live=0;
   15:50:24 六支全 Live=1(`IoDetail.txt:83,89,95,101,107,113`),而機台機構是凍結的
   (LOT END 15:29:19.886)。同一時刻**每一支** per-Auto 卡上的 InType=0 點仍讀 0,
   而 `IP2 P0` 的兩點各自獨立移動(`SnEmpty_InputEnd` 維持 1、`SnLoader_Inputend` 由 1 變 0)。
   物理上也不可能:Auto4/5/6 那一輪根本沒收到任何 unit,其 InputHasTray / InputFullTray /
   OutputBottomHasTray 同刻全讀 0。
   (`IoDetail.txt` 的 `Live` 欄 = `TMySensor::IsOn()`,已反相後的值 —— `cStateRecordHT160.cpp:1329-1334`。
   所以 `Typ=0 且 Live=1` 就是「raw bit 0 **或**讀取失敗」,無法分辨。)

**對策(已寫進 §4.2)**:R-C 只用 `InputHasTray`(加上 `bTrueFull` 已帶的 `InputFullTray`);
R-D 的停止條件用「觸發轉 false」(即同一個布林),**不用**「InputEnd 轉 OFF」;
`InputEnd` 維持只做它今天的 274 釋放。極性另案上機驗證(本樹自己的文件也還標為未驗:
`docs/AGV/HT160S_AMR_ManualInject_Test_SOP_20260720.md:167`)。

若不遵守:15:50:24 那個狀態下,六條 lane 會永久叫車,而且因為停止條件正是壞掉的 port
產不出來的值,**這個迴圈設計上就無法終止**。

### H2 —— `Enable==false` 的方向不對稱(不必修,但要知道)

`mysensor.cpp:44-48` 與 `:63-67`:`IsOn()` 和 `IsOff()` 在 `Enable==false || Input==NULL` 時**都**回 false。
所以停用的感測器既不 on 也不 off,方向由各消費者的守衛決定:

* **叫車側全部 fail-safe**:`IsOutputCarFullForAmr`(`:1596` `Enable==true && IsOn()`)、
  `IsFrontHasTrayForAmr`(`:1622` 同型)→ 停用即不叫車。R-C 免費繼承。
* **釋放側 fail-unsafe**:`IsAmrTaken`(`:1720` `Enable==true && IsOff()`)→ 停用即「沒被取走」,
  永遠卡在 `AGV_READY`。這是既存行為,R-D 不改它,但 R-D 讓它變成「永遠重送」而非「300 秒後停機」,
  所以 (d) 的 escalation 列是必要的。

### H3(第 4 題:重送迴圈跨到下一個 lot)—— **最大的新風險**

已驗證的事實:**CALLED 站的鎖活過一切。**

* `TAgvCoordinator::Reset()` 無 runtime 呼叫者(§4.3),所以 `Handshake[]` 什麼都活得過。
* Lot End 清 `bAmrLocked`(`aAuto1To6.cpp:96`,無條件、在 early-out 上方),
  然後 `database.cpp:91 AgvCoord.ReassertLocks()` 立刻把它裝回去 ——
  `uAgvStation.cpp:851-856` 對每個 CALLED/PREP/READY 的 Auto `SetAmrLock(a,true)`。
* HOME 也一樣(`csystem.cpp:1842 InitialAllTask(true)`)。
* MachineStart 完全不碰(`csystem.cpp:1546-1558`:只有 `AmrInject.Reset()` 然後 `DoStartArm()`)。
* 現場鐵證:`FeederDecision.txt` 15:50:24 `P5 AUTO2: lock=1 hs=CALLED ready=1 bins=[] label=[] lot=[]`
  —— Lot End 後 **21 分鐘**,其餘五站 `lock=0 hs=IDLE`。

**被鎖住的 Auto 對下一個 lot 是完全停用的**(全在 `aAuto1To6.cpp`):
`GetTrayRequest` 回 `eTrayReqNone`(`:1438-1439`)、`FindDischargeAuto` `continue`(`:510`)、
per-station 出料資格 false(`:523`)、出料尾巴 latch 凍結(`:570`、`:2114`)、
teach 被擋(`uteach.cpp:1806`),而且 **Clean Out 的六站 lockstep barrier 被整體卡住**
(`:1158-1159 continue`,barrier 是 `:1166 AreAllFlagsOn(bCleanOutCheck)`;`:1151-1153` 的註解
自己就寫著「one AMR-locked station holds the whole drain until CEID274 releases it」)。

**量化這個代價,請 owner 明確接受:**
一站卡 CALLED → 下一個 lot 用五條 lane 跑;若下一個 lot 又走到 Clean Out 而該站還是 CALLED,
**六站的 drain 全都結不掉,直到 AMR 來** —— 這就把 R-A 要消除的停滯**延後一個 lot** 重現。

**三個選項:**

| | 作法 | 評價 |
|---|---|---|
| (a) | **維持鎖**(今天的行為) | 盤真的還在車上,鎖真的該把下一個 lot 擋在外面。**推薦** |
| (b) | 下一個 Lot Start 重發 272 並重新蓋 SVID 快照 | **錯**。盤屬於舊 lot |
| (c) | Lot Start 放鎖 | **不安全**。下一個 lot 會疊到裝著前一個 lot 產品的車上 |

**推薦 (a),但必須把 R-C 的第三項做成 latch,不是活述詞。**
理由:若第三項只是「窗口內的活述詞」,下一次 Start 關掉窗口後,
若該車只是 HasTray(沒滿)則 `bFull` 轉 false → `:556-561` 放鎖回 IDLE →
**該 lane 帶著前一個 lot 的盤重新加入生產**,而 66040 那欄已經改報新 lot。
這是最壞情況(混料 + 錯誤 lot 標籤),必須堵住。

作法:`unsigned char AfterLotHold[AGV_AUTO_COUNT]`,在第三項首次成立時 set,
**只**在下列情形 clear:(a) `IsFrontHasTrayForAmr` 與 `IsOutputCarFullForAmr` 皆 false
(盤真的沒了);(b) CEID 274 / `ClearAmrCar`;(c) 操作員明確放行。
`bAfterLot` 改讀這個 latch。這樣 Lot Start **不能**悄悄把裝了料的車還給生產。

**操作員放行**:重用既有的 WAR0963 答覆路徑 `ReleaseStationByOperator`(`uAgvStation.cpp:292-308`)。
它把 `Handshake` 強制設 IDLE,而 `ReassertLocks` 只對 CALLED/PREP/READY 重新上鎖,所以**推不回來**;
而且它是唯一在沒有 SECS 證據時放行仍屬安全的既有路徑。需要同時清 `AfterLotHold` 與 `PayloadPinned`。

### H4(第 5 題:AMR 在下一個 lot 期間才到)

* **協定層沒問題。** `BeginPrep`(`uAgvStation.cpp:812-828`)對 Auto **不要求** `AGV_CALLED`、
  不看 lot;唯一拒絕是 infeed P1-P3 在 `Run_CleanOut`(`:816-820`)。host 端只在
  `bUseAMR==0` 時整包拒絕。所以 272 → START_AGV → 273 → 274 → `ClearAmrCar` 全走得完。
* **`ClearAmrCar` 對「已屬於別的 lot 的車」是安全的 —— 前提是鎖從未斷過。**
  鎖在的期間 `GetTrayRequest` 拒收盤(`:1438`)、`FindDischargeAuto` 跳過(`:510`),
  所以沒有新 lot 的盤能進去。**這正是 H3 那個 latch 存在的理由**:沒有它,鎖可能在 Lot Start
  被交回,車就會被混料,然後 `ClearAmrCar` 銷毀一份混合帳,而 AMR 會用一台車、一個 lane lot 號
  帶走兩個 lot 的產品。
* **274 的 payload**:客戶只把 RPTID 2001 綁在 272 上,所以那些計數**從不**搭 274。
  但 `:692-696` 的發射前重讀會覆寫 sticky 值成 0 —— 已在 §4.3 處理。
* **273 在測試計畫上的注意事項**:`IsDrainedForAmr` 會先 `RefreshAutoState()`(`aAuto1To6.cpp:1694`)
  再讀 `State[]` latch,而 `RefreshAutoState`(`:398`)在 sim early-out、真機只單調上鎖存。
  所以停機期間一次 episode 可以憑停機前 latch 的狀態走 CALLED → PREP → READY,**機構完全沒動**。
  無害(`ServiceHandshake` 不驅動任何馬達 —— `uAgvStation.cpp:652-654` 的註解如此聲明),但 log 會看起來很怪。

### H5 —— `AmrInject` 在 Lot End 被清

`database.cpp:59 AmrInject.Reset()` 在 `InitialAllTask` **開頭**,而 Lot End 會呼叫它
(`csystem.cpp:1929`)。所以任何注入的測試狀態不會活過 Lot End。測試腳本要知道。

---

## 6. 第 6 題:SECS 事件

**要發:**

| 事件 | 何時 | 出處 |
|---|---|---|
| CEID 272 | 每次 CALL 與**每次重送** | `uAgvStation.cpp:543`。payload = host 的 `{502,2000,2001}` |
| CEID 273 / 274 | 不變 | `:679` / `:698` |
| CEID 42 / CEID 8 | 不變,**28 ms 基準不可退** | `csystem.cpp:1914` / `main.cpp:3341` |

**不可發:**

| 事件 | 理由 |
|---|---|
| 35/36/37/148/149/150 | **20260901 「只在真的滿」裁定成立**。今天已經 level-gate 在 `bTrueFull`(`:552-553`),所以第三項不會拖到它們;但 level→edge 修正(S0)是前置條件,否則重送把「一台車四封 CEID 36」放大成 288 封/天 |
| 任何新的 private CEID(9002+) | EAP 字典沒有 |
| 重送的 S5F1 | 與停機不可分割 |
| Auto collect 的 WAR0962 | **完全停止產生**(P4-P9)。P2/P3 infeed 保留 |

---

## 7. 第 7 題:為何 `csystem.cpp` 一行都不用動 —— 五條可稽核的不變式

每條都可用一個指令驗:

| # | 不變式 | 驗法 |
|---|---|---|
| I1 | 本次變更不含 `csystem.cpp` | `git diff --stat` 無 `csystem.cpp` hunk |
| I2 | `InitialAllTask` 呼叫點維持 **5** 個,且 `csystem.cpp:1929` 的引數維持預設 | `grep -c "InitialAllTask()" csystem.cpp` → 對應 `:1453`(開機)、`:1842`(HOME,傳 true)、`:1929`(AMR)、`:1937`(非 AMR)、`:2015`(Run_TrayFeed) |
| I3 | `CheckCleanOutFinish` 內沒有 AMR 詞彙 | `sed -n '2031,2055p' csystem.cpp \| grep -c "bUseAMR\|AgvCoord"` == 0 |
| I4 | `ServiceAgvTimeoutAlarm`(`:135-193`)未被修改;它只是對 si>=3 **不再有輸入**,因為生產者停止 latch。P2/P3 照常 | `git diff csystem.cpp` 為空 + 迴歸測 P3 infeed 超時仍彈 WAR0962 |
| I5 | `PollAndCall` / `ServiceHandshake` 早就在 `SystemStart=0` + `Run_Normal` 下跑,所以不需要新的 RunMode 閘、不需要新呼叫點 | 已由 2026-09-04 現場證明:Lot End 15:29:19.886 之後 CEID 272 於 15:44:13 與 15:50:22 照發,`MachineState.ini` 同刻 `SystemStart=0 RunMode=0` |

### 明確拒絕 `InitialAllTask(true)`(不要再被提出)

呼叫點只有一行,但影響面擴散到六個模組(`database.cpp:74-85`),而且會把**前一個 lot 的物料帳**帶過 lot 邊界:
`GetNextTrayKindForAuto`(`aAuto1To6.cpp:1385-1395`)會讀到非零的 `iTrayCount`,
以為下一個 lot 的身分盤 + 表頭盤已經疊好。同時 `aAuto1To6.cpp:102-121` 的 keep-material pass 還會保留
`bResidueClear` / `bRearHasTray` / `bFrontHasTray` / `bFullIC` / `RearKind` / `WorkingKind` /
`RearTrayID` / `WorkingTrayID` / `RearGrid`,而 Loader / Empty / Color / TrayArm / SortArm 同旗標同樣受影響。
而且**手動 Lot End 按鈕根本不呼叫 `InitialAllTask`**
(`main.cpp:3272 btnLotEndClick → DoLotEndProcess`,`main.cpp:3278-3396` 全段沒有 `InitialAllTask`),
所以兩條 Lot End 路徑仍會不一致。
**用改動物料帳的方式去修一個「回報欄位空白」的問題,是拿 host 可見的空欄去換路由/身分風險。**

---

## 8. 不動項

* `SnAutoX_InputEnd` 的唯一職責維持 CEID 274 釋放(`aAuto1To6.cpp:1711-1721`)。
* `IsCleanOutCollectDueForAmr`(`uAgvStation.cpp:413-431`)一字不改,含其中「不要加 lot-open 判斷」的既有裁定。
* `RetryStation`(`:317-329`)語意不變(含刻意不放鎖)。
* WAR0963 / `ServiceLinkLostHold`(`:244-270`)/ `ServiceAgvLinkLostAlarm`(`csystem.cpp:209`)完全不動 ——
  link-lost 真的需要操作員確認 AMR 已離開。
* P1(Loader)的靜默 force-release(`:772-787`)與 P2/P3 的 `TimeoutPending`(`:788-800`)不動。
* `AgvCoord.TrayCount[0]` 的 consume-once trip 帳(`:87-88`、`uHGemHT160.cpp:2793`)不動。
* 274 之後的歸零(`:700-701`)維持**無條件**。
* 報表 body / RPTID 不動(`uHGemHT160.cpp:1259-1286`);不擴 report 6、不開新 report id。
* `bUseAMR==0`:所有被改的函式都已 early-return 在它上面
  (`uAgvStation.cpp:439-440 PollAndCall`、`:632 ServiceHandshake`、`:415 IsCleanOutCollectDueForAmr`;
  `csystem.cpp:137-138`、`:209-210`)。AMR=0 的出料滿是另一條完全獨立的操作員 modal
  (`aAuto1To6.cpp:1130-1145`,MES1120..MES1620),裡面沒有任何 `bAmrLocked`。
  **escalation 列也必須 gate 在 `bUseAMR`,且不可放進 `aAuto1To6`**,否則 AMR=0 路徑會撿到它。
  → **`bUseAMR==0` 行為 byte-unchanged。**

---

## 9. 驗證計畫

### 9.1 編譯閘(每階段)

1. 刪掉受影響的 `.obj`,`scripts/ops/build-ht160s.ps1 -Clean`。
2. 接線層變更(新增 GeneralSetting key、新 method)→ `-Full`。
3. `scripts/ops/check-ht160s-source-encoding.ps1`(擋 `EF BF BD` 與 UTF-8 BOM)。
4. 真機組態驗證:註解掉 `MachineType.h` 的 `#define SOFT_SIMULATE`,`-Full`,確認 exit 0,
   **還原**後重建 —— 本變更大量落在 `IsSoftSimulate()` 分支旁,必做。
5. 行尾檢查:改完 `.cpp/.h` 後確認 CRLF 未被轉成 LF。

### 9.2 SOFT_SIMULATE 能驗到什麼(誠實範圍)

**R-D 在筆電上完全驗不到。** `IsAmrTaken` 在 sim **硬回 true**(`aAuto1To6.cpp:1714-1715`),
所以每個 sim 交握在一兩個 tick 內自我完成,永遠老化不到超時;
`TAmrInject`(`SecsGem/uAmrInject.h:51-90`)只存正向布林,只能**加**真值,**無法壓制**任何述詞
(注入點:`uAgvStation.cpp:530`、`:676`、`:685` 都是 `|| AmrInject.X()`)。
且 `IsFrontHasTrayForAmr` 在 sim 回 false(`aAuto1To6.cpp:1619-1620`),`RefreshAutoState` 在 sim early-out(`:398` 附近)。

**必須先加測試鉤(列為 S4,建議與 R-D 同批):**
`TAmrInject` 加 `bool bHoldAtReady[AMR_INJ_AUTO_COUNT]`(+ setter/getter/Reset),
`uAgvStation.cpp:685` 改成
`if((AutoModule->IsAmrTaken(a) || AmrInject.AutoTaken(a)) && AmrInject.HoldAtReady(a)==false)`。
這樣重送節拍、log 節流、escalation 全部可在筆電上驗。**沒有這個鉤,R-D 第一次證據就是京元的一個班。**

sim 可驗清單:
* `bUseAMR=0` 全流程行為 byte-unchanged(對照 EventLog / SECS log 與基線)。
* Lot End 後 `IsLotIdentityFrozen()` 為 true、Start 後為 false(三條 Lot End 路徑 × 兩條 Start 路徑,共六組)。
* S1a sticky:sim 用盤數門檻造出 `bTrueFull`,Lot End 後確認 38225 / 38231 / 38205 不歸零,
  而 274 之後歸零。
* S0 邊緣閘:一台車一次 full 只發一封 CEID 36,即使 272 重發多次。
* HOME 期間 `ServiceHandshake` 凍結(`:658`)仍成立。

### 9.3 上機驗證(必須,由使用者執行)

| 項 | 步驟 | 通過標準 |
|---|---|---|
| V1 | 量測 CEID 42 → CEID 8 間隔 | ≤ 28 ms 級,不退步 |
| V2 | **`SnAutoX_InputEnd` 極性專項**:對一台 Auto 車實體放/取一盤,同步看 IO 監控 | 有盤 = ? / 無盤 = ? 明確記錄。同時試著複現 15:50:24 的全六支假亮(拔 `IP2 P1` 網段) |
| V3 | 前側感測器極性:同樣對 `InputHasTray` / `InputFullTray` 逐 lane 放/取 | 兩支各自的 ON/OFF 對應實體狀態 |
| V4 | Lot End 後留一台**不滿但有盤**的車 | 發出 CEID 272,bitmap 只點該站 |
| V5 | 檢查 V4 那封 272 的 body | RPTID 2001 的 38225/38231(或對應 lane)非零,66040-66045 帶舊 lot,502 的 Lot ID 非空 |
| V6 | 讓 V4 的 call 超時三次 | 每 `AmrResendSec` 一封 272;**零** WAR0962;EventLog 只有第 1 次與第 10 次 |
| V7 | V4 進行中按下一個 Lot Start | 該 lane 維持 CALLED + locked(latch 生效);其餘五 lane 正常生產;66040-66045 該欄仍報**舊** lot |
| V8 | V7 之後叫 AMR 來收 | 274 正常、`ClearAmrCar`、latch 與 PIN 一起解除、lane 回歸生產 |
| V9 | 操作員放行路徑 | `ReleaseStationByOperator` 能把 V7 的站放掉,且 `ReassertLocks` 不會推回來 |
| V10 | 迴歸:P3 Color infeed 超時 | WAR0962 照舊彈(R-D 不覆蓋 infeed) |
| V11 | 一整班的 EventLog 列數 | 一站卡住約 8 列/班,不是 96 列 |

---

## 10. 分階段實作順序(每階段可獨立建置)

| 階段 | 內容 | 檔案 | 依賴 | 可獨立出貨 |
|---|---|---|---|---|
| **S0** | 離散 Full CEID 改**邊緣**觸發(§4.5) | `SecsGem/uAgvStation.{h,cpp}`(`:498`、`:541-554`、`:556-561`、`:699`) | 無 | **是** — 修今天就有的缺陷 |
| **S1a** | R-B 剩餘 18 SVID:三行 sticky + 274 發射前同規則(§4.3) | `SecsGem/uAgvStation.cpp:518-527`、`:692-696` | 無 | **是** — 約 8 行,立刻補上觀測到的空 payload |
| **S2** | R-C 第三觸發項 + `AfterLotHold` latch + N-tick 確認 + 最短重叫間隔(§4.2、H3) | `SecsGem/uAgvStation.{h,cpp}` | S1a(否則新發的 272 仍是空的) | 是 |
| **S3** | R-D 重送:新 `[AGV] AmrResendSec`、老化區塊改自我重送、log 節流、escalation 列;P4-P9 停止 latch `TimeoutPending`(§4.4) | `GeneralSetting.{h,cpp}`(三處)、`SecsGem/uAgvStation.{h,cpp}` | S0(否則 CEID 36 被放大)、S2 | 是 |
| **S1b** | payload PIN(跨 lot 邊界正確性,§4.3) | `SecsGem/uAgvStation.{h,cpp}`、`:1042-1051`、`:1093-1101` | S3(只有 R-D 會讓 episode 跨 lot) | 是 |
| **S4** | 測試鉤 `TAmrInject::HoldAtReady` + 操作員放行接線(§9.2、H3) | `SecsGem/uAmrInject.h`、`SecsGem/uAgvStation.cpp:685`、`:292-308` | S3 | 是 |
| **S5** | 文件:本計畫定稿 + SECS 手冊補「CEID 272 現在會重複發送」+ `system/General.ini` 加 `AmrResendSec` | `docs/`、`system/General.ini` | S3 | 是 |

**BCB6 約束(全階段)**:不用 C++11(無 `auto`/`nullptr`/lambda/range-for/`enum class`);
不引入 FSM;新註解一律 ASCII 英文;維持 `AnsiString` 流;機控路徑無阻塞迴圈 / `Sleep()`;
新旗標一律 `unsigned char[AGV_AUTO_COUNT]` / `int[]` 的既有陣列型式,不用 STL 容器;
`AnsiString` 不用嵌套三元運算(`uAgvStation.cpp:1020` 已記載 BCB6 會誤編)。
所有新行為 gate 在 `bUseAMR`;`bUseAMR==0` byte-unchanged。

---

## 11. 待裁定

以下每一項都需要 owner 決定,附上分支與我的建議。

**D1 —— `AfterLotHold` latch:要不要?(§H3,影響最大)**
* **不做**:下一次 Lot Start 之後,一台只是「有盤沒滿」的車會被自動放行,帶著**前一個 lot 的盤**重新加入生產,
  且 66040 那欄已改報新 lot → 混料 + 錯誤標籤。
* **做**:那台 Auto 從 Lot End 起一直停用到 AMR 來。**下一個 lot 只有五條 lane**;若下一個 lot 也走到 Clean Out
  而該站還卡著,**六站的 drain 全結不掉**。
* **建議:做。** 混料無法事後修復,少一條 lane 可以。配 D2 的操作員放行當逃生門。

**D2 —— 卡住的站要不要給操作員一個強制放行按鈕?**
* 建議:重用既有的 `ReleaseStationByOperator`(`uAgvStation.cpp:292-308`),掛在維護頁,
  按鈕文案要明講「確認車上盤已人工取走」。它是唯一在沒有 SECS 證據時放行仍安全的既有路徑。

**D3 —— escalation 通知用什麼?(§4.4 (d))**
* (a) 純 `RecordProcess` 一列 —— 零代價,但通知最弱(EventLog 裡的 PROCESS 列)。
* (b) 真警報碼,只寫 EventLog、不走 `ShowMyError`(照 `aAuto1To6.cpp:1327-1330` 殘料 watchdog 的慣例)。
  空號都確認過:`WAR0964` 全樹無使用;per-Auto 家族 `MES%d29`(MES1129..MES1629)也全空
  (後綴 20/23/25 已被生成家族佔用,21/22/24/26/27/28 在 prefix 14 全與 Color seed 相撞)。
* **代價**:選 (b) 會讓交付的警報目錄由 **486 → 487**
  (`docs/plan/alid-option-d-execution-plan-20260903.md:131`、`:141-143`:`AlarmList.csv` 重生、
  產生器常數 bump、S5F6/S5F8 目錄改變、workbook 需重新交付)。
* **建議:(b) + `WAR0964`**,標準與 WAR0962/WAR0963 同族(`database.cpp` 的 standalone 註冊慣例),
  但請 owner 確認願意付「目錄 487 + workbook 重交付」這個代價。若不願意,退 (a)。

**D4 —— `AmrResendSec` 預設值**
* 300 s(與 `AgvTimeoutSec` 一致,好解釋)還是更短(例如 60 s,叫車更積極但 log 量 ×5)?
* clamp 建議 ≥ 30(因為數的是 VCL tick 不是時鐘)。
* **建議:預設 300,clamp ≥ 30**,現場可用 `General.ini` 調而不必重建。

**D5 —— SECS `CLEAR_LOT_INFO` 是否也該關掉 R-C 的窗口?**
* 現況:`uHGemHT160.cpp:2444-2448` 該指令在 `DoLotEndProcess` 之後**立刻** `SetLotIdentityFrozen(false)`。
  若 R-C 綁在同一個旗標上,host 一下清除指令,窗口就關,`bAfterLot` 隨之轉 false —— 但盤還在車上。
* 兩種讀法都自洽:「host 明確要求清掉,就一起清」vs「盤還在,叫車不該停」。
* **建議:讓 `AfterLotHold` latch 不受這個 disarm 影響**(latch 一旦 set 只由 D1 列的三個條件清),
  這樣 host 的清除只影響 payload retention,不影響叫車。請 owner 確認。

**D6 —— 死 key `AmrFullWaitSec` 怎麼處置?**
* `system/General.ini:130 AmrFullWaitSec=60` 存在但全樹無消費者。
* **建議:不要偷用**(現場檔已填 60,接管會改變沒人動過的設定的行為);
  新開 `AmrResendSec`,並在 S5 文件裡明確記載 `AmrFullWaitSec` 為廢棄鍵。

**D7 —— `SnAutoX_InputEnd` 極性/接線的專項上機驗證,要不要排在本計畫之前?**
* 本計畫刻意**不依賴**它,所以不是阻擋項。
* 但它今天是 CEID 274 的唯一釋放閘,而 2026-09-04 有一次無法解釋的全六支假亮。
* **建議:與 V4-V8 同一趟上機一起做(V2)**,結果單獨回報,不阻擋 S0/S1a 出貨。
