# AMR / SECS Handshake-Timeout Fallback — Implementation Plan

> 狀態: 已核可設計, implementation-ready。本文件落地後即依此執行。
> 範圍檔案 (writable, HT160S_BCB only):
> `HT160S_Program_BCB_V1.0.0.0/{GeneralSetting.cpp,GeneralSetting.h,aAuto1To6.cpp,aAuto1To6.h,aEmpty.cpp,aEmpty.h,aColor.cpp,SecsGem/uAgvStation.cpp,SecsGem/uAgvStation.h}` + `system/{language_phrases.txt}`。
> 不改: `aLoader.cpp` (canonical template 已完成), `aColor.cpp:293` (DoColor case 100 bAmrLocked break)。
> 行號基準: 本 session 對 live working-tree 實讀確認 (2026-06-27); `aColor.cpp` 已被平行 `ht160s-color-align-empty` session 改寫, 採用 **live** 行號 (非核可決策文中已過時的 :677-685/:658/:722)。

---

## 1. 目標與範圍

當機台處於 AMR 模式 (`GeneralSetting.bUseAMR==true`) 而 SECS/AGV handshake 在容許時間內沒有完成 (host 沉默、AGV 未到、READY 互鎖卡住),目前各供料/出料站會無限期 `break;` 停在 `DoAllProcess` 迴圈裡 (Auto full / Empty 缺料 / Color 缺料), 整線「Hang up」且操作員看不到任何告警。本 slice 把已落地的 Loader feed-deferral 樣板 (`aLoader.cpp` DoFeedTray, `iAmrFeedWaitSec`, per-side `bWaitingAmrFeed`+`FeedWaitTimer`, `MES0920`) 一般化成「等候 file-configurable 逾時 → 升起 modal Note Alarm (`ShowMyError`) + 人工 recovery」的安全網, 覆蓋四個 hang point (Auto-full / PREP-READY watchdog / Empty 缺料 / Color 缺料)。AMR happy path 完全不變: handshake 在逾時前完成時, 路徑與行為與今日一致, 不多一個 popup、不改 CEID 序、不改 lock 釋放時機。與另一條無 AMR 的 `bManualTraySupply` 設計互補 (complementary): 該設計處理「機台根本沒接 AMR、靠操作員手動補料」的常態流程; 本 slice 處理「有接 AMR 但 handshake 逾時」的異常安全網, 因此本 slice 不依賴、也不需要 `bManualTraySupply`。

---

## 2. 已解決的開放問題 (baked-in)

| 議題 | 結論 (已確認) |
|---|---|
| Empty 缺料感測來源 | `SnEmpty_InputEnd` 為 Empty source-dry 的 source-of-truth (ON=有盤 / OFF=空)。已存在的 `IsInputShortageForAmr()` (`aEmpty.cpp:84-89`) real branch 即讀 `SnEmpty_InputEnd.Enable && IsOff()` (`:88`); sim branch `iSimInfeedCount<=0` (`:87`)。P3 anchor 用此, **不可**用 `bFrontHasTray`。 |
| Empty 告警碼 (HT9046 研究 + decision (b)) | HT9046 canonical Empty source-empty = `MES1021` (asendic_Empty.cpp:108, DoLoadNewEmptyTrayToCar case 1 ELSE 分支)。但 HT160 `MES1021` 已被佔用於不同語意 (`aEmpty.cpp:337` 後段 rear output-bottom miss), `MES1024` 佔用於 front miss (`aEmpty.cpp:481`)。**採 decision (b): P3 source-dry 用 NEW machine-local 碼 `MES1022`** (Empty MES10xx band 內首個 free 號; collision grep 證實 `MES1020/MES1022/MES1023` 全 repo 零佔用)。理由: 操作員與 cloud/host 必須能把「供料 magazine 空了」與既有 Empty `MES1021` (rear bottom-miss) / `MES1024` (front miss) 區分開; 重用 `MES1021` 會讓 host 看到單一碼無法分子階段, 故 decision (b) 改採獨立碼。`MES1022` 須登記於 `system/AlarmList.csv` (AlarmType=8 string 碼, 同 WAR0462 等慣例) 供 cloud/host 差異化, 並在 `language_phrases.txt` 加 EN+ZH。Loader 的 `MES0920` (immediate) vs `WAR0962` (AMR+ART count-shortfall) 之爭解決: HT160 無 ART path, 故 Loader 在 AMR-wait 下用 `MES0920` 為正解, **不**改 `WAR0962`。 |
| Sim 覆蓋面 | Sim 只測得到 **CALLED leg** (host 端 sim/real-AGV 缺席時的呼叫與逾時); PREP/READY watchdog 的 release-then-alarm、shared-rail modal starvation 屬 **on-machine verify** 才能完整驗證 (見 §9)。 |
| Single-owner 與 modal 飢餓 | coordinator watchdog (THGem 1s tick, 與 DoAllProcess 同一 VCL thread) 只 **釋放** lock; station 自己的 per-cycle code 升 modal (`ShowMyError`)。原因: `ShowModal` 會重入 message pump, 若由 watchdog 升 modal 會 stall SECS link。此點 on-machine verify。 |

---

## 3. 可檔案調整逾時參數

`General.ini` `[AGV]` 區三個旋鈕 (秒)。`HTimer::Off()` 在 0 立即回 true (= 不等候直接告警), 負值會 wrap 成 ~49.7 天 (= 永不告警 → 退化成 Hang up)。故三者皆須在 `GeneralSetting::Load()` clamp 到正下界。

| INI key (`[AGV]`) | 欄位 | 預設 | 用途 | 現況 |
|---|---|---|---|---|
| `AmrFeedWaitSec` | `iAmrFeedWaitSec` | 600 | Loader/Empty/Color **供料源缺料** 等候 | 已存在 (default `GeneralSetting.cpp:44`; Load `:91`; Save `:132`; decl `GeneralSetting.h:87`) |
| `AmrFullWaitSec` | `iAmrFullWaitSec` | 600 | Auto **output 滿** 等候 | **NEW** |
| `AmrHandshakeWaitSec` | `iAmrHandshakeWaitSec` | 240 | coordinator watchdog PREP/READY 老化界限, 取代 `#define AGV_HANDSHAKE_WATCHDOG_TICKS 240` (`uAgvStation.cpp:35`) | **NEW** |

### B3 — clamp (必做)
`Load()` 內 (`GeneralSetting.cpp:65-107`) 目前對任何 AGV 逾時 **無** clamp。新增: 讀完三值後加正下界守門 (建議下界 = 5 秒, 避免 0 立即告警/負值 wrap)。

實作位置:
- `GeneralSetting.cpp:91` 之後新增兩行 `ReadInteger`:
  - `iAmrFullWaitSec = Ini->ReadInteger("AGV","AmrFullWaitSec",600);`
  - `iAmrHandshakeWaitSec = Ini->ReadInteger("AGV","AmrHandshakeWaitSec",240);`
- 緊接其後 (仍在 `delete Ini` 之前, `:106`) clamp:
  ```cpp
  // AI(ht160s-agv) clamp : HTimer::Off() returns true at 0 (instant alarm) and wraps
  // negative to ~49.7 days (never alarms). Force a positive lower bound on all AGV waits.
  if(iAmrFeedWaitSec      < 5) iAmrFeedWaitSec      = 5;
  if(iAmrFullWaitSec      < 5) iAmrFullWaitSec      = 5;
  if(iAmrHandshakeWaitSec < 5) iAmrHandshakeWaitSec = 5;
  ```
  > 註: `iAmrFeedWaitSec` 既有讀取在 `:91`, 既無 clamp, 一併納入。
- `SetDefault()` (`GeneralSetting.cpp:38-62`) 在 `iAmrFeedWaitSec=600;` (`:44`) 旁加 `iAmrFullWaitSec=600;` 與 `iAmrHandshakeWaitSec=240;`。
- `Save()` (`:109-147`) 在 `WriteInteger("AGV","AmrFeedWaitSec",...)` (`:132`) 旁加兩行對應 `WriteInteger`。
- `GeneralSetting.h:87` (`int iAmrFeedWaitSec;`) 後加 `int iAmrFullWaitSec;` 與 `int iAmrHandshakeWaitSec;` (兩 `int` 為純新增欄位 → struct 尺寸變更 → **`-Full` build**, 見 §8)。

---

## 4. 逐點實作

### 4.1 Auto output 滿 (主 hang point)

**位置**: `TAutoModule::ServiceCarFull()` (`aAuto1To6.cpp:1152-1205`)。要取代的 HGem-skip 在 `:1165`:
```cpp
if(HGem!=NULL && HGem->IsSelected())
    continue;
```
此行使 link-up 時把滿車交給 AGV 後直接 `continue`, fall-through 的 operator modal 不會升 — 這就是主 hang (handshake 永不完成時整個 Auto 卡住, 只有 HOME 能解 lock)。

**anchor predicate** (fall-through 實際走哪條就 anchor 哪條):
- real build: `bSensorFull` (`aAuto1To6.cpp:1181`, `GetInputFullTray(Index)` Enable&&IsOn) → `MES%d20` = `MES1120..MES1620` (`:1189`, do/while held)。
- sim build: `bLogicalFull` (`:1168`, `Car[Index].iTrayCount>=MAX_TRAY_PER_CAR`) → `MES%d25` = `MES1125..MES1625` (`:1200`, one-shot)。
- 統一的物理穩定條件 = `bFull && bAmrLocked`, 其中 `bFull = IsOutputCarFullForAmr(Index)` (`aAuto1To6.h:106` / `.cpp:986-994`; 此即與 fall-through 對齊的 predicate: real=sensor, sim=count), `bAmrLocked = IsAmrLocked(Index)` (`aAuto1To6.cpp:1010-1015`)。

**改法** (取代 `:1165-1166` 的無條件 skip):
```cpp
bool bFull   = IsOutputCarFullForAmr(Index);
bool bLocked = IsAmrLocked(Index);

if(HGem!=NULL && HGem->IsSelected())
{
    // AGV handshake in flight : defer the operator modal while the car is full AND
    // the AMR lock is held. Start the wait timer once; on expiry, abort the
    // handshake for THIS Auto and fall through to the existing held alarm.
    if(bFull && bLocked)
    {
        if(bWaitingAmrFull[Index]==false)
        {
            AmrFullWaitTimer[Index].SetMS(GeneralSetting.iAmrFullWaitSec*1000);
            AmrFullWaitTimer[Index].On();
            bWaitingAmrFull[Index]=true;
            continue;
        }
        if(AmrFullWaitTimer[Index].Off()==false)
            continue;                       // still waiting for the AGV
        // timed out : take this Auto out of the handshake so PollAndCall does not
        // re-CALL, mark operator-holding, then fall through to ShowMyError below.
        bWaitingAmrFull[Index]=false;
        AmrFullWaitTimer[Index].Clear();
        bOperatorHolding[Index]=true;
        AgvCoord.AbortAutoHandshake(Index);   // releases lock + sets Handshake[si]=AGV_IDLE
    }
    else
    {
        // not (full & locked) : normal AGV path, no operator alarm needed.
        bWaitingAmrFull[Index]=false;
        AmrFullWaitTimer[Index].Clear();
        continue;
    }
}
// fall-through : existing bSensorFull / bLogicalFull ladder (:1180-1203) runs the
// MES%d20 / MES%d25 modal. After it returns, the operator cleared the car.
```
> 既有 `:1168-1203` 不動。`bOperatorHolding[Index]` 在 modal 之後(車已清)隨 HOME/init 由 `InitialFlag` 清 (見下)。
> 注意: `AbortAutoHandshake` 之後 `bAmrLocked` 已是 false, 但 fall-through 仍以 `bSensorFull`/`bLogicalFull` 為 anchor, 與既有 modal 一致, 不受 lock 狀態影響。

**state 欄位** (`aAuto1To6.h`, 緊鄰 `bool bAmrLocked[6];` `:46`):
```cpp
bool   bWaitingAmrFull[6];   //AI(ht160s-agv) per-Auto AMR full-wait latch
bool   bOperatorHolding[6];  //AI(ht160s-agv) operator took the full car (suppress re-CALL)
HTimer AmrFullWaitTimer[6];  //AI(ht160s-agv) per-Auto AMR full-wait timer
```
> `aAuto1To6.h` 已 include `HTimer.h` (既有 `FeedDelay` 等 HTimer 成員)。三新陣列 → struct 尺寸變更 → **`-Full` build**。

**init/reset** (`InitialFlag(bool bKeepMaterial)`, `aAuto1To6.cpp:53-96`): 在 per-Auto loop 內、`bAmrLocked[Index]=false;` (`:76`) 旁、且 **在** `if(bKeepMaterial) continue;` (`:80-81`) **之前**, 加:
```cpp
bWaitingAmrFull[Index]=false;
AmrFullWaitTimer[Index].Clear();
bOperatorHolding[Index]=false;
```
> 放在 `bKeepMaterial` continue 之前, 確保 recoverable home 也會清掉 wait/hold (否則 home 後仍 suppress re-CALL)。

**B-ping-pong (PollAndCall re-CALL gate)**: `uAgvStation.cpp:221` 的 `if(bFull && Handshake[si]==AGV_IDLE)` 在 `AbortAutoHandshake` 把 Handshake 設回 `AGV_IDLE` 後, 下一 tick 會立刻又 `SetAmrLock+EventReport272+AGV_CALLED`, 與 station 端 `bOperatorHolding` 形成 re-CALL ping-pong。守門: 加 `&& AutoModule->IsOperatorHolding(a)==false`:
```cpp
if(bFull && Handshake[si]==AGV_IDLE && AutoModule->IsOperatorHolding(a)==false)
```
> 需在 `aAuto1To6.h`/`.cpp` 加 public accessor `bool IsOperatorHolding(int Index)` (model on `IsAmrLocked`, `aAuto1To6.cpp:1010-1015`)。`bOperatorHolding` 由操作員清車後的 HOME/init (`InitialFlag`) 清回 false, re-CALL 才會恢復。

### 4.2 PREP/READY watchdog (coordinator, release-only)

**位置**: `TAgvCoordinator::ServiceHandshake` 兩處 watchdog:
- Auto leg `uAgvStation.cpp:298-309` (`Handshake[si]==AGV_PREP||AGV_READY` 老化, 逾 `AGV_HANDSHAKE_WATCHDOG_TICKS` → `SetAmrLock(a,false)` + `Handshake[si]=AGV_IDLE`)。
- Infeed P1-P3 leg `:342-351` (同樣老化, → `InfeedSetLock(p,false)` + `AGV_IDLE`)。

**B1/B3 — 改用 file-configurable 界限**: 把兩處 `> AGV_HANDSHAKE_WATCHDOG_TICKS` 改為 `> GeneralSetting.iAmrHandshakeWaitSec`, 並移除 `#define AGV_HANDSHAKE_WATCHDOG_TICKS 240` (`:35`)。`ShortageDebounce[si]` 是 1s-tick 累加 (THGem tick), 故界限單位 = 秒, 與 `iAmrHandshakeWaitSec` 一致。
> 確認 `GeneralSetting.h` 已被 `uAgvStation.cpp` include (既有對 `GeneralSetting.bUseAMR` 的引用); 若無, 加 include。

**single-owner 邊界 (已對齊核可決策)**: watchdog 只老化 **PREP/READY**, **不**老化 **CALLED** (CALLED 老化會經 `PollAndCall:221` 造成 re-CALL ping-pong)。watchdog **只釋放 lock**, **不**升 modal。Auto-full 的 host-silent 上界由 station 端 `iAmrFullWaitSec` (§4.1) 負責; watchdog 的 `iAmrHandshakeWaitSec` 只負責「已進入 PREP/READY 卻卡住」的 lock 釋放。兩者各司其職, 不重疊。

### 4.3 Empty source-dry (P3)

**位置**: `DoEmpty` case 100 的 **REAR-FEED** 分支 `aEmpty.cpp:219-224`:
```cpp
if(bRearHasTray==false && bLotFinish==false)
{
    DoFeedTray(0);   //AI(ht160s-agv) rear feed RUNS while AMR-locked (anti-starve)
    Task=2000;
    break;
}
```
> 注意此分支刻意 **不**含 `bAmrLocked` break (anti-starve, `:221`)。case 100 內三個既有 `bAmrLocked` break 分支 (`:203-204` bReturnTray GoUp / `:212-213` bFrontHasTray==false GoDown / `:228-229` bLotFinish GoUp) **皆非** P3 target。P3 不碰這三個。

**anchor predicate**: `IsInputShortageForAmr()` (`aEmpty.cpp:84-89`; real=`SnEmpty_InputEnd.Enable&&IsOff`, sim=`iSimInfeedCount<=0`) — **不**用 `bFrontHasTray`。Empty 缺料代表「供料 magazine 空了, 後段 rear-feed 無盤可送」, 故在 rear-feed 之前先檢查 source-dry。

**改法** (port Loader template; 在 `:219` 的 rear-feed if 之前插入 source-dry gate):
```cpp
// AI(ht160s-agv) source-dry : the supply magazine ran out (SnEmpty_InputEnd OFF).
// In AMR mode wait iAmrFeedWaitSec for the AGV to refill before alarming; the AGV
// refill is detected on a later cycle (IsInputHandoffFinishedForAmr, aEmpty.cpp:93).
if(IsInputShortageForAmr())
{
    if(GeneralSetting.bUseAMR)
    {
        if(bWaitingAmrFeed==false)
        {
            AmrFeedWaitTimer.SetMS(GeneralSetting.iAmrFeedWaitSec*1000);
            AmrFeedWaitTimer.On();
            bWaitingAmrFeed=true;
            break;
        }
        if(AmrFeedWaitTimer.Off()==false)
            break;
        bWaitingAmrFeed=false;
        AmrFeedWaitTimer.Clear();
    }
    // decision (b) : NEW machine-local code MES1022 (NOT a reuse of MES1021). Distinct
    // from Empty MES1021 (rear bottom-miss, aEmpty.cpp:337) and MES1024 (front miss,
    // aEmpty.cpp:481) so operator + cloud/host can tell source-dry apart. Registered in
    // system/AlarmList.csv (AlarmType=8) + language_phrases.txt (EN+ZH).
    int Ret=ShowMyError("MES1022", LangT("Empty supply magazine empty"), K_RETRY|K_CLEAN_OUT);
    if(Ret==K_RETRY)
        Task=1;       // re-enter case 100; if the AGV refilled, shortage clears
    // K_CLEAN_OUT : RunMode flips to Run_CleanOut elsewhere; DoEmpty top guards it.
    break;
}
// AI(ht160s-agv) refill arrived (or sim) : drop the latch so the next dry edge re-arms.
if(bWaitingAmrFeed && IsInputShortageForAmr()==false)
{
    bWaitingAmrFeed=false;
    AmrFeedWaitTimer.Clear();
}
```
> 放在 rear-feed if (`:219`) **之前**: source 有盤時這段整段跳過, rear-feed anti-starve 行為不變。

**state 欄位** (`aEmpty.h`, 緊鄰 `bAmrLocked` `:25` 與 HTimer `FeedDelay/GoDownDelay/GoUpDelay` `:27-29`; `HTimer.h` 已 include `aEmpty.h:6`):
```cpp
bool   bWaitingAmrFeed;       //AI(ht160s-agv) Empty source-dry AMR wait latch
HTimer AmrFeedWaitTimer;      //AI(ht160s-agv) Empty source-dry AMR wait timer
```
> Empty 是單站, 成員為 scalar (非 `[6]`), 無 `Reset()/ResetSide()`; `InitialFlag()` (`aEmpty.cpp:24-45`) 為唯一 reset 點。

**init/reset**: `InitialFlag()` 在 `bAmrLocked=false;` (`:26`) 旁加:
```cpp
bWaitingAmrFeed=false;
AmrFeedWaitTimer.Clear();
```

**DescribeState()** (`aEmpty.cpp:814-830`, `bAmrLocked` dump `:826`) 旁加 `bWaitingAmrFeed` + timer 剩餘秒數 dump (State Record 排查用; 對齊 strengthen-staterecord-logging-on-gap 慣例)。

> struct 尺寸變更 (Empty 加 bool+HTimer) → **`-Full` build**。

### 4.4 Color source-dry (P4)

**位置 (live, drift-resolved)**: 核可決策的 `aColor.cpp:677-685 case-800 MES1421` 已不存在 (平行 `ht160s-color-align-empty` 改寫)。**LIVE** 的唯一 `MES1421` `ShowMyError` 在 `DoSupplyTray` case 7000 (`aColor.cpp:716-736`):
```cpp
case 7000:
    RefreshStateFromSensors();
    if(HSys.Sen.SnColor_OutputBottomHasTray.Enable==true &&
       HSys.Sen.SnColor_OutputBottomHasTray.IsOff() &&
       HSys.LastSet.iRealDummy!=DUMMY)
    {
        Ret=ShowMyError("MES1421", LangT("Color supply tray is not ready"), K_RETRY);  // :725
        if(Ret==K_RETRY)
            SupplyTask=1;
    }
    else { bTrayReady=true; bTrayPicked=false; bSupplyRequested=false; SupplyTask=13000; }  // :731-734
    break;
```
> **不**新增碼、**不**碰 `aColor.cpp:293` (DoColor case 100 bAmrLocked break)。

**改法**: WRAP 既有 `MES1421` 升起 (`:725`) 為 AMR-wait。在 `if(...IsOff()...)` true 分支內、`ShowMyError` 之前插入等候閘 (與 Empty/Loader 同形):
```cpp
if(HSys.Sen.SnColor_OutputBottomHasTray.Enable==true &&
   HSys.Sen.SnColor_OutputBottomHasTray.IsOff() &&
   HSys.LastSet.iRealDummy!=DUMMY)
{
    if(GeneralSetting.bUseAMR)
    {
        if(bWaitingAmrFeed==false)
        {
            AmrFeedWaitTimer.SetMS(GeneralSetting.iAmrFeedWaitSec*1000);
            AmrFeedWaitTimer.On();
            bWaitingAmrFeed=true;
            break;
        }
        if(AmrFeedWaitTimer.Off()==false)
            break;
        bWaitingAmrFeed=false;
        AmrFeedWaitTimer.Clear();
    }
    Ret=ShowMyError("MES1421", LangT("Color supply tray is not ready"), K_RETRY);
    if(Ret==K_RETRY)
        SupplyTask=1;
}
else
{
    bTrayReady=true; bTrayPicked=false; bSupplyRequested=false;   // :731-733
    SupplyTask=13000;
}
break;
```

**latch+timer 清除 (核可決策)**: 當 `bSupplyRequested` 被撤回時清。LIVE 撤回點 (drift-resolved): `bSupplyRequested=false` 在 case 7000 成功分支 `aColor.cpp:733`, 並由 `RequestSupplyTray` 在 `:1013` 設 true; `InitialFlag()` `:41` 清。在 **`:733`** (成功 present) 旁加 `bWaitingAmrFeed=false; AmrFeedWaitTimer.Clear();` (供料成功 = wait 結束)。另在 `InitialFlag()` (`:27-60`, `bAmrLocked=false` `:29` / `bSupplyRequested=false` `:41`) 旁加同樣兩行 reset。
> 核可決策引用的 `:658/:722` 為過時; 舊 case-10 early-bail clear 已移除 (case 10 `:638-651` 現無條件路由到 case 1000)。**P4 須在平行 color session commit 後 re-verify 行號**。

**state 欄位** (`aColor.h`, 緊鄰 `bAmrLocked`): `bool bWaitingAmrFeed;` + `HTimer AmrFeedWaitTimer;` (確認 `aColor.h` include `HTimer.h`; 若無則加)。
**DescribeState()** (`aColor.cpp:1193+`, `bSupplyRequested` dump `:1231`) 旁加 dump。
> `aColor.h` struct 尺寸變更 → **`-Full` build**。

### 4.5 Loader feed (DONE — 不重做)

canonical template, 已落地且與核可決策一致, 不改:
- `aLoader.cpp:1100-1113` AMR block (`GeneralSetting.bUseAMR` gate, per-side `State->FeedWaitTimer.SetMS(iAmrFeedWaitSec*1000)` / `State->bWaitingAmrFeed`)。
- `aLoader.cpp:1114` `ShowMyError("MES0920", LangT("Loader Tray Empty"), K_RETRY|K_TRAY_END|K_CLEAN_OUT)`。
- recovery: `K_RETRY` → `State->FeedTask=1`; `K_TRAY_END` → `bTrayEmpty=true` (`:1117-1119`)。
- anchor: 真機 push-cylinder sensor 在 if-branch (`:1097`) cancel; `IsInputHandoffFinishedForAmr` sim-true 不用於 cancel (會 defeat wait, 註 `:1098`)。

---

## 5. 執行緒安全 — single-owner

兩條 code path 都在同一 VCL thread (`DoAllProcess` 與 THGem 1s tick 皆主執行緒, 無真並行), 但 **責任** 嚴格分離以避免 `ShowModal` 重入 message pump 而 stall SECS link:

| 角色 | 動作 | 不做 |
|---|---|---|
| coordinator watchdog (`ServiceHandshake`, `uAgvStation.cpp:298-309/342-351`) | 只 **釋放 lock** (`SetAmrLock(a,false)` / `InfeedSetLock(p,false)`) + `Handshake=AGV_IDLE`; 只老化 **PREP/READY** | 不升 modal; 不老化 CALLED |
| station per-cycle (`ServiceCarFull` / `DoEmpty` / `DoSupplyTray` / `DoFeedTray`) | 計時 + 逾時升 `ShowMyError` (modal) + recovery | 不釋放 coordinator lock (除透過 `AbortAutoHandshake`) |

**`AbortAutoHandshake(int Index)` (NEW, `TAgvCoordinator`)**: Auto-full 在 station 端逾時、進入 held alarm 之前呼叫, 把該 Auto 退出 handshake, 使 watchdog 與 `PollAndCall` 都不再介入。model on link-down release (`PollAndCall:178-196`, 即 `SetAmrLock(a,false)` + `Handshake[si]=AGV_IDLE`)。decl 加在 `uAgvStation.h` public block (`:60-73` 區, 緊鄰 `ServiceHandshake`)。實作:
```cpp
// AI(ht160s-agv) station-side timeout : the operator is taking the full car. Drop
// this Auto's handshake so neither the watchdog nor PollAndCall touches it again
// until the next clean full edge (gated by IsOperatorHolding in PollAndCall).
void TAgvCoordinator::AbortAutoHandshake(int Index)
{
    if(Index < 0 || Index >= AGV_AUTO_COUNT) return;
    int si = Index + 3;
    if(AutoModule!=NULL)
        AutoModule->SetAmrLock(Index, false);
    Handshake[si]        = AGV_IDLE;
    ShortageLatch[si]    = 0;
    ShortageDebounce[si] = 0;
}
```

**`bOperatorHolding[6]` (station 端, §4.1)**: 阻止 `PollAndCall:221` 在 `AbortAutoHandshake` 把 Handshake 設回 `AGV_IDLE` 後立刻 re-CALL (ping-pong)。`PollAndCall:221` 加 `&& AutoModule->IsOperatorHolding(a)==false`。操作員清車 → HOME/init → `InitialFlag` 清 `bOperatorHolding` → re-CALL 恢復。

---

## 6. 告警碼表 (final)

| 站 | 碼 | 文字 (English key) | keys | 狀態 | 9046 對齊 |
|---|---|---|---|---|---|
| Loader | `MES0920` | `Loader Tray Empty` | `K_RETRY\|K_TRAY_END\|K_CLEAN_OUT` | DONE (`aLoader.cpp:1114`) | canonical (MES0920) |
| Empty (P3) | `MES1022` | `Empty supply magazine empty` | `K_RETRY\|K_CLEAN_OUT` | NEW machine-local 碼 (decision (b)); `aEmpty.cpp` case 100 source-dry gate | machine-local (與 9046 `MES1021` source-empty 語意對齊但**獨立碼**, 避免與 HT160 既佔的 `MES1021` rear-miss / `MES1024` front-miss 混淆); 須登記 AlarmList.csv |
| Color (P4) | `MES1421` | `Color supply tray is not ready` | `K_RETRY` | WRAP existing (`aColor.cpp:725`) | canonical (asendic_Color.cpp:106) |
| Auto1-6 full (sensor) | `MES1120..MES1620` | `Auto%d output stack FULL (sensor) ...` | `K_RETRY` | existing (`aAuto1To6.cpp:1189`) | machine-local (9046 用非編號 popup + SECS event) |
| Auto1-6 full (count) | `MES1125..MES1625` | `Auto%d output car full ...` | `K_RETRY` | existing (`:1200`) | machine-local (數值上與 9046 `MES1121` source-empty 衝突 → 視為本機碼) |

**AlarmList.csv 佔用 (確認無碰撞)**: `system/AlarmList.csv` 目前只登記四個字串碼: `WAR0462`(:518)、`WAR0970`(:519)、`WAR16120`(:520)、`WAR16121`(:521), 全為 CCD (AlarmType=8 string-code 慣例)。**無** `MES09xx/MES10xx/MES11xx/MES14xx` 行。一般 MES* 由 `ShowMyError`/`ShowErrorMessage` (note.cpp:357) 以 4-char prefix + numeric suffix 在 runtime 解析外部字串表, 不強制登記於 AlarmList.csv。**但 decision (b) 例外**: 新 P3 source-dry 碼 `MES1022` 為 NEW machine-local 碼, **須**新增一行到 `system/AlarmList.csv` (`MES1022,8,"Empty supply magazine empty",...` 格式同 WAR0462 等), 使 cloud/host 能把它與既有 `MES1021`/`MES1024` 差異化 (decision (b) 的核心要求)。source 端命名空間佔用: `MES1021` `aEmpty.cpp:337` (rear bottom-miss), `MES1024` `aEmpty.cpp:481` (front miss); collision grep 證實 `MES1020/MES1022/MES1023` 全 repo (source + AlarmList.csv + docs) 零佔用, 故 `MES1022` 為 Empty MES10xx band 內首個 free 號。不碰 Auto (`MES11xx/MES14xx` 家族) 與 JAM/WAR Auto bands。

**language_phrases.txt 需新增的 EN`<TAB>`ZH** (`system/language_phrases.txt`, Big5/CP950, 單一 literal TAB 分隔):
| English key (literal in source) | 繁中 | 現況 |
|---|---|---|
| `Loader Tray Empty` | `Loader 無盤` | 已存在 (`:136`) |
| `Color supply tray is not ready` | `Color 供盤未就緒` | 已存在 (`:60`) |
| `Empty supply magazine empty` | `Empty 供料匣缺盤` | **NEW** (P3, decision (b) 碼 `MES1022`) |

> `Empty supply magazine empty` 為 P3 唯一需新增的 phrase (decision (b) 的 `MES1022` literal key); ZH `Empty 供料匣缺盤` 刻意與 rear/front miss 文字區隔, 操作員可一眼分辨「供料匣整匣空」非「單盤 miss」。必須 **byte-safe (Big5)** 編輯 (`scripts/ops/bcb6-bytesafe-edit.ps1` 或 python/Latin1 splice), 不可用標準 Edit tool (見 edit-tool-corrupts-big5-source)。同一 byte-safe pass 另須在 `system/AlarmList.csv` 新增 `MES1022` 行 (EN+ZH 文字同此, decision (b))。

---

## 7. InputEnd 慣例 (source-dry sensor)

**慣例**: 任何命名為 `*_InputEnd` (Loader 為 lowercase `_Inputend`) 的感測器 = 該站 **供料 magazine ("Car") 缺料** 的 source-of-truth: **ON = 有盤, OFF = 空**。這是 AMR source-dry 判定的唯一正確輸入。

**陷阱 (B2)**: **不可**用 `bFrontHasTray` 當缺料 anchor。`bFrontHasTray` 是站內 front grid 狀態 (盤已從 magazine 取到 front 後), 它 OFF 不代表 magazine 空 (可能盤正在搬運中), 用它會誤觸 false 缺料告警或漏報真缺料。三個 source-dry P-task (Empty/Color/Loader) 一律 anchor 在各自 `IsInputShortageForAmr()` (= 各自 `*_InputEnd` OFF)。

**全列舉** (HT160 INVENTORY 確認, 皆 `Enable=1`):

| Sensor | 站 | 角色 | IO_Table.csv / database |
|---|---|---|---|
| `SnAuto1_InputEnd` | Auto1 output car | car-taken / input-full edge | IO_Table.csv:46; database.h:300; .cpp:1011; getter `aAuto1To6.cpp:251` |
| `SnAuto2_InputEnd` | Auto2 | 同上 | IO_Table.csv:52; database.h:306; .cpp:1017; :252 |
| `SnAuto3_InputEnd` | Auto3 | 同上 | IO_Table.csv:58; database.h:312; .cpp:1023; :253 |
| `SnAuto4_InputEnd` | Auto4 | 同上 | IO_Table.csv:64; database.h:318; .cpp:1029; :254 |
| `SnAuto5_InputEnd` | Auto5 | 同上 | IO_Table.csv:70; database.h:324; .cpp:1035; :255 |
| `SnAuto6_InputEnd` | Auto6 | 同上 | IO_Table.csv:76; database.h:330; .cpp:1041; :256 |
| `SnEmpty_InputEnd` | Empty 供料 magazine | **P3 anchor**; ON=有盤 OFF=空 | IO_Table.csv:80; database.h:291; .cpp:1002; `aEmpty.cpp:88,:97` |
| `SnLoader_Inputend` | Loader 供料 magazine | **唯一 lowercase 拼法 `Inputend`**; ON=有盤 OFF=空 | IO_Table.csv:81; database.h:297; .cpp:1008; `aLoader.cpp:367,:376` |
| `SnColor_InputEnd` | Color 供料 magazine | **P4 anchor**; ON=有盤 OFF=空 | IO_Table.csv:214; database.h:338; .cpp:1049; `aColor.cpp:93,:102` |

> Loader 拼法陷阱: 是 `SnLoader_Inputend` (小寫 `end`), 不是 `_InputEnd`。grep / 新 code 引用時須照原拼。Auto 的 `*_InputEnd` 語意是 "output car 滿/被取走" 的 edge, 非供料缺料; Auto-full 的 anchor 用 `IsOutputCarFullForAmr` (sensor=`GetInputFullTray`), 不直接用 `SnAutoN_InputEnd` 名稱。

---

## 8. 分階段 P0-P5

每階段獨立可 slice; 每階段後過 build gate。**所有改動含 byte-safe 注意**: BCB6 source 為 Big5 legacy, 新註解 ASCII English; 任何含中文的檔 (含 `language_phrases.txt`) 用 byte-safe 工具改, 不用標準 Edit tool。**struct 尺寸變更 (新增欄位) 一律 `-Full` build** (見 CLAUDE.md build gate)。每階段須刪改動檔對應 `.obj` 再編。

| 階段 | 內容 | 檔案 | build |
|---|---|---|---|
| **P0** | 三旋鈕 + clamp (B3)。`iAmrFullWaitSec`/`iAmrHandshakeWaitSec` 新欄位 + clamp + SetDefault/Load/Save | `GeneralSetting.cpp`, `GeneralSetting.h` | `-Full` (struct 加兩 int) |
| **P1** | watchdog file-configurable (B1): 兩處界限改 `iAmrHandshakeWaitSec`, 移除 `#define AGV_HANDSHAKE_WATCHDOG_TICKS`; 加 `AbortAutoHandshake` + `IsOperatorHolding` accessor; `PollAndCall:221` 加 operator-holding gate (B-ping-pong) | `SecsGem/uAgvStation.cpp`, `SecsGem/uAgvStation.h`, `aAuto1To6.cpp`, `aAuto1To6.h` | `-Full` (Auto struct 加 3 陣列, 與 P2 合) |
| **P2** | Auto-full timed defer (主 hang): 取代 `aAuto1To6.cpp:1165` skip; 加 `bWaitingAmrFull[6]`/`AmrFullWaitTimer[6]`/`bOperatorHolding[6]` + InitialFlag reset + DescribeState dump (B-modal-ownership) | `aAuto1To6.cpp`, `aAuto1To6.h` | `-Full` |
| **P3** | Empty source-dry: case 100 source-dry gate (anchor `IsInputShortageForAmr`); NEW machine-local 碼 `MES1022` (decision (b)) + phrase + AlarmList.csv 行; `bWaitingAmrFeed`+`AmrFeedWaitTimer` + InitialFlag reset + DescribeState | `aEmpty.cpp`, `aEmpty.h`, `system/language_phrases.txt`, `system/AlarmList.csv` | `-Full` (Empty struct 加 bool+HTimer); phrase + AlarmList.csv byte-safe |
| **P4** | Color source-dry: WRAP `aColor.cpp:725` MES1421 為 AMR-wait; latch clear at `:733`/InitialFlag; state+DescribeState。**先確認平行 color session 已 commit**, re-verify 行號 | `aColor.cpp`, `aColor.h` | `-Full` (Color struct 加 bool+HTimer) |
| **P5** | 整合驗證: `SOFT_SIMULATE` on → `-Full` exit0; 註解 off `MachineType.h` → `-Full` exit0 (real-machine path) → 還原 define 重編; encoding check `scripts/ops/check-ht160s-source-encoding.ps1` | 全部 | `-Full` ×2 |

> 依賴: P1 與 P2 共用 `aAuto1To6.h` struct 變更, 建議連續做、一次 `-Full`。P0 須先於 P1-P4 (所有站引用 `GeneralSetting.iAmr*WaitSec`)。P4 在平行 `aColor.cpp` session commit 前不要落地 (行號會再漂)。

---

## 9. 上機 / 客戶驗證清單 (on-machine-only)

Sim 只能測 CALLED leg (host/AGV 缺席的呼叫與逾時); 以下須真機:

1. **Auto-full 逾時 → modal**: 真 AGV link-up, Auto 車滿後 host 不送 START_AGV, 等 `iAmrFullWaitSec` → 升 `MES%d20` (sensor) modal, 操作員清車後恢復。確認逾時前 AGV 正常完成 → **不**升 modal (happy path 不變)。
2. **B-ping-pong 不復發**: `AbortAutoHandshake` + `bOperatorHolding` 後, `PollAndCall` 不再 re-CALL (觀察無 CEID272 連續重發); HOME 後 `bOperatorHolding` 清, re-CALL 恢復。
3. **PREP/READY watchdog release-only**: 人為卡住 READY 互鎖, 等 `iAmrHandshakeWaitSec` → coordinator 只 `SetAmrLock(false)` + `AGV_IDLE`, **不**升 modal, SECS link 不 stall。
4. **shared-rail modal starvation**: Loader1/2 共軌; 一側 modal 開啟 (`ShowModal` 重入 pump) 時另一側 DoAllProcess / SECS tick 行為, 確認 link 不掉、無互鎖死結。
5. **Empty/Color/Loader source-dry → AGV refill cancel**: 真 `*_InputEnd` OFF 進等候, AGV 補料 magazine → `*_InputEnd` ON → `IsInputHandoffFinishedForAmr` 偵測 → 在逾時前自動 cancel wait, 不升 modal。
6. **clamp 行為**: `General.ini` 設 `AmrFullWaitSec=0` / 負值 → 確認 clamp 到 5s (不是立即告警 / 不是永不告警)。
7. **host alarm 表 parity (客戶)**: 確認客戶 host alarm 字串表含 `MES1022`(Empty source-dry, decision (b) NEW 碼, 已登記本機 `AlarmList.csv`)/`MES1421`(Color)/`MES0920`(Loader); `MES1022` 與既有 Empty `MES1021`(rear miss)/`MES1024`(front miss) 為不同碼, host 端須分別建表方能差異化告警。Auto `MES11xx/MES14xx` 為機台本機碼, 須在交付文件標注 (非 9046 host-table 對齊, 且 `MES1125` 與 9046 `MES1121` source-empty 數值衝突)。
8. **language toggle**: `iLanguageCountry==1` 時三告警顯示繁中 (`Empty 供盤未就緒` 等)。
