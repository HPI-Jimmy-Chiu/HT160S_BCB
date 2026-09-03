# 京元 2026-07-30 上機九項問題 — 根因分析與修復計畫

- 分析日期：2026-07-31
- 素材：`D:\HT160S_StateRecord\`（15 份 State Record + 現場原始碼 7z + `20260730.txt`）
- 比對基準：repo `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0`（branch `feat/iosetview-172-refactor`，HEAD `1b64e66`）
- 現場版本身分：= repo commit `0ba540a`（07-30 11:53）+ **兩處本機修改**（`MachineType.h` 的 SOFT_SIMULATE 註解 + `aLoader.cpp` 一處 hot-fix）。除此之外整棵樹沒有其他現場改動。
- 本文件為唯讀分析，**尚未動任何程式碼**。

---

## 0. 一句話結論

九項回報收斂成 **7 個真缺陷**，其中兩個是 **P0 且先前沒人知道**：

1. **D1 — CleanOut 退場守衛會半途搶斷進料，並洩漏「前站互斥鎖」** → 這才是 14:54 Loader hang 的真兇（不是您改的那一行）。
2. **D2 — `bCleanOutFinish` 是單向黏著 latch** → CleanOut 可能在**兩台載盤車都還壓著盤**的狀態下宣告「已排空」。

您現場改的那一行**沒有修到 14:54 的 hang**（在那段 trace 裡它是 no-op），但**確實修對了第 9 項**（AMR 不再問 Clean out），代價是**很可能造成了 17:35 之後那一輪新的 hang**。

---

## 1. 九項回報 → 缺陷對照

| 您的項次 | 缺陷 | 嚴重度 | 判定 |
|---|---|---|---|
| ③ Loader hangup（inend 還有盤沒下來） | **D1** 前站互斥鎖洩漏 + 半途搶斷 | **P0** | CONFIRMED（code+log）。**您的修改不是這題的解** |
| ④ PassBinZero | 已由 `cb0591f` + `1b64e66` 移除整個設定 | — | 該停機模式**已關閉**；但 0↔1 跳動的真因**未解釋**（見 §6） |
| ⑤ empty + trayarm 互卡 | **D3** `aEmpty.cpp:433` latch 提早清掉 | P1 | CONFIRMED。**現場版本已含 `710ba4f`，所以那次修不夠** |
| ⑥ Loader 兩汽缸夾著沒盤就進去分料 | 「載盤車夾爪」說法**查無實證**；實證指向**前分料器**卡在伸出 | P1 | **需您確認實際看到哪一組汽缸**（見 §5） |
| ⑦ offset 設定跳兩次 Alarm | **D7** `uOffset.cpp:522/592/608` 用到「停機警報箱」 | P2 | CONFIRMED（含 SECS log 三次實證） |
| ⑧ `IsInputHasTrayTrustworthy()==false` 卡死 | **= D1 的另一種外顯**（分料器被丟下、rise-1 留在伸出） | **P0** | CONFIRMED |
| ⑨ AMR 模式仍問 Clean out | **D5** `aLoader.cpp:1722` 用錯 predicate | P1 | CONFIRMED |
| 額外發現 | **D2** CleanOut 假完成 | **P0** | 狀態 CONFIRMED、路徑 HYPOTHESIS |
| 額外發現 | **D4** LK1 幽靈盤（兩側同時認養同一顆感測器） | P1 | CONFIRMED |

---

## 2. D1（P0）— 14:54 Loader hang 的真因

### 機構語言說明

Clean Out 的「這一側可以收工了」判斷寫在 `DoLoader` 的**最上面**，每個 tick 都跑，而且跑在 `switch(Task)` **之前**。它對「後段正在退盤」有豁免，對「前段正在進料／分料」**沒有**。因為盤的身分是很晚才鑄造的（FeedTask 9500），整個進料+分料過程中 `fHasTray` 都還是 false，所以這個守衛會誤判「這側閒著」，直接把 `Task` 拉回 1 就 return。

被丟下的東西沒人收：

| 洩漏的資源 | 唯一釋放點 | 外顯 |
|---|---|---|
| `iFrontOwner`（前站互斥鎖） | `aLoader.cpp:1858` `ReleaseFrontOwner()`（只在 `case 10000`） | **第 ③ 項**：兩側循環等待，只有 HOME 解得開 |
| 分料器（rise-1 留在伸出） | `aLoader.cpp:2391` `C_Loader_FrontRiseTray_1.Pop()`（只在 `DoFrontDestackDown case 7`，且要走 FeedTask 4100） | **第 ⑧ 項**：case 10 的 trustworthy 死結 |

### 程式錨點

```
aLoader.cpp:1299   if(HSys.Sys.RunMode==Run_CleanOut && iYOwner[...]==LOADER_Y_OWNER_NONE)
aLoader.cpp:1309       if(TrayMotor->fHasTray==false && bSupplyCarDry && bRearDischargeInProgress==false)
aLoader.cpp:1311           State->bCleanOutFinish=true;
aLoader.cpp:1313           Task=1;
aLoader.cpp:1314           return;              // <- 前站鎖沒還、分料器沒收
```
死結兩邊：
- 持鎖側卡在 `aLoader.cpp:1353` `if(OtherState->Status==LS_FEEDING || ...) break;`
- 缺鎖側卡在 `aLoader.cpp:1558` `if(AcquireFrontOwner(LoaderNo)==false) break;`

`iFrontOwner` 只有三個寫入點：`:1031` 取得、`:1038` 釋放、`:138` HOME 歸零。**沒有逾時、沒有搶奪。**

### State Record 實證（`2026-07-30 14_54_57_Loaderhangup`）

```
FeederDecision.txt : iFrontOwner=2
                     Side1: Status=FEEDING  Feed=100   fHasTray=0  CleanOutFin=1
                     Side2: Status=IDLE     Feed=9000  fHasTray=0  CleanOutFin=1
CurrentTasks.txt   : Loader1 | 1000 | 14:50:01.390 | 295921
                     Loader2 |  100 | 14:50:01.386 | 295925
MachineState.ini   : RunModeName=CleanOut
```
Side2 在 14:48:16.451 從 `case 1000` 被拉回 `Task=1`；`case 1000` 本身**沒有任何路徑**可以到 `Task=1`（`aLoader.cpp:1373-1389` 只會前進到 2000 或 break），`default:` 分支會寫 `LogLadderFault` 而 EventLog 裡沒有。**`aLoader.cpp:1313` 是唯一可能的寫入者。**

這個狀態在 14:53:19 與 14:56:14 兩份快照裡完全一樣 —— **撐過了一次 Pause 和一次 Start，而且全程沒有任何警報。**

第 ⑧ 項的實證（`2026-07-30 17_36_55`）：
```
FeederDecision : Side1: Status=FEEDING  Feed=10  Destack=5
                 P1 Loader: ready=0            <- IsReadyForAmrHandoff()，即 rise1/rise2/sep 全部沒縮回
EventLog       : 17:35:49.151 "WAIT Loader1: rise1 (C_Loader_FrontRiseTray_1) not retracted..."
                 17:35:59.145 MES0925
General.ini    : Rise1SettleWaitSec=10
```
時序：17:35:37.856 開始進料 → 17:35:48.057 守衛搶斷 → 17:35:49.141 重啟 → 7 ms 後就開始 WAIT。

### 建議修法（兩個 hunk，必須一起上）

`aLoader.cpp:1309`：加 `bDestackInFlight`（FeedTask 4000/4100/8200/8300）條件，並在退場時 `ReleaseFrontOwner(LoaderNo);` + `FeedTask=1` + `RecordProcess` 麵包屑。

`aLoader.cpp:1353`：進料入口再加一項 `(iFrontOwner!=0 && iFrontOwner!=LoaderNo)`，避免「先掛 LS_FEEDING 再發現搶不到鎖」。

**已評估並否決的替代方案**（別再提）：
- 用 `State->DestackTask!=1` 當條件 → 已在該姿態的機台永遠退不了場，整機 CleanOut 卡死。
- 用 `State->FeedTask==1` 收緊 → 停在 9000 的側永遠回不到 1。
- case 9000 直接導到 10000 收尾 → `case 10000` 回 true，`DoLoader case 1000` 會對空車跑 Top-CCD。
- case 10 直接 `Pop()` rise-1 → DestackTask 4..6 時分離爪還伸著，**會把整疊盤摔下來**（`aLoader.cpp:2376-2380` 明文禁止）。

**注意：`ReleaseFrontOwner(LoaderNo);` 這一行就是第 ③ 項的全部解方**，`FeedTask=1` 其實是惰性的（`DoFeedTray(LoaderNo,0)` 已經做了）。

---

## 3. D2（P0）— CleanOut 可能在載盤車還壓著盤時宣告完成

`bCleanOutFinish` 全樹只有兩個寫入點：`aLoader.cpp:1311` 設起、`aLoader.cpp:205`（`InitLoader`／HOME）清掉。

而退場守衛在 `iYOwner != NONE`（SortArm 佔用該側 Y）時**整段跳過**（`aLoader.cpp:1299-1300`），於是一個已經「宣告完成」的側可以直接掉進 `switch(Task)` 繼續正常進料、抓一盤新的盤，**旗標還掛著**。

而完成判定 `IsAllCleanOutFinish()`（`aLoader.cpp:973-997`）只驗 latch、`SnLoader_InputHasTray`、`SnLoader_Inputend` —— **全是進料端的點，看不到載盤車上的盤。**

實證（`2026-07-30 17_32_15_Loader no tray to input` / FeederDecision.txt 第 31、33 行）：
```
Side1: Status=ToRear    Feed=1  fHasTray=1  CleanOutFin=1
Side2: Status=CCD_SCAN  Feed=1  fHasTray=1  CleanOutFin=1
```
**兩側都抱著盤，兩側都掛著「已完成」。**

建議修法：
1. 在 `aLoader.cpp:1320` 附近（`switch(Task)` 之前、CleanOut 區塊之外）加：CleanOut 中若這側繼續跑 ladder，就把 `bCleanOutFinish=false` 並記一筆 `RecordProcess`。
2. `IsAllCleanOutFinish()` 加入 `TrayMotor->fHasTray` 檢查。

> ⚠️ D2 的 `fHasTray` 檢查要和 **D4**（幽靈盤）同一版上線，否則幽靈盤會反過來卡住 CleanOut。

---

## 4. D3（P1）— Empty 與 TrayArm 搶同一盤（第 ⑤ 項）

**現場版本的 `aEmpty.cpp` / `aTrayArm.cpp` 與 repo 完全 byte-identical**，也就是 07-30 09:21 的 `710ba4f` **已經在現場那顆 exe 裡了** —— 所以那次修的是 place 側，這次撞的是**另一條路**。

根因 `aEmpty.cpp:430-435`：
```cpp
case 3000:
    if(DoGoUpTray(1))
    {
        bRearReturnInProgress=false;                    // :433 太早清
        if(bReturnTray && bTrayXToEmptyFinish==false)
            return;                                     // :435 其實第二趟還沒跑
```
`bRearReturnInProgress=true` 全樹只有 `aEmpty.cpp:764`（`DoGoUpTray(Flag==0)`）會寫，而第二趟是從 idle 端呼叫 `DoGoUpTray(1)` 重啟的 → **整個第二趟裸奔**。同時 `NotifyTrayXToEmptyFinish()`（`aEmpty.cpp:1318-1322`）會無條件把 `Status` 蓋成 `ES_REAR_READY`，把 `ES_RETURNING` 洗掉。TrayArm 讀到「閒置可取」就下來夾了同一盤。

實證（`2026-07-30 16_58_45_emptyandtrayarm_lock`）：
```
EventLog       : 16:58:07.319 alarm 600 "Cylinder=C_Empty_PushTray Func=Push | Step=actEmpty task=3000"
FeederDecision : [Empty]   Status=REAR_READY  GoUpTask=4000  bRearReturnInProgress=0
                           RearLeanOut=1  RearPushOut=1
                 [TrayArm] Status=PICKING  PickTask=2000  PickWaitArmed=0
```
`PickTask=2000` 代表 `DoZDown` 已經回 true → **閘門確實放行了**。
20 分鐘前的對照組（`16_38_10`）：`Status=RETURNING  bRearReturnInProgress=1`、TrayArm `PlaceWaitArmed=1` → 正確擋住。**這是 race，不是必現**。

建議修法（一行換序，不動 predicate）：
```cpp
// aEmpty.cpp:431-435  AFTER
if(DoGoUpTray(1))
{
    if(bReturnTray && bTrayXToEmptyFinish==false)
        return;                     // 還欠第二趟：latch 保持 true
    bRearReturnInProgress=false;
```
外加 `aTrayArm.cpp:630` 的派工守衛：`|| EmptyModule->IsReturnTrayRequested()`（該 helper 已存在於 `aEmpty.cpp:1285`）。

> ⚠️ `aEmpty.cpp` 含 1 行非 ASCII → **必須 byte-safe 編修**。`aLoader.cpp`／`aLoader.h`／`uOffset.cpp`／`uOffset.h`／`aTrayArm.cpp` 實測為純 ASCII，Edit 安全。
> 兄弟位置：`aColor.cpp:509-520` 是同一段 park 的鏡像，建議一併檢視。

---

## 5. 第 ⑥ 項 — 「兩汽缸夾著且沒有盤」查無實證，需要您確認

15 份 State Record 裡：
- Loader 的 `FeedTask` **從來沒有出現過 8200 或 8300**（那才是夾爪 Push 的兩步）。
- 進料源乾時 `case 3500`（`aLoader.cpp:1583-1604`）**直接跳到 9000**，整段分料和兩個夾爪都不會執行。
- 兩個夾爪在每次進料開頭（`case 2000`/`case 3000`）和每次 HOME（`uHome.cpp:732`/`:763`）都會 Pop 打開，而且帶 `OnAlarmTime=5000`，失敗不會無聲。

**真正有實證的是「前分料器卡在伸出」**：`17_36_55` 與 `17_41_53` 的 `P1 Loader: ready=0`，就是 `IsReadyForAmrHandoff()`（`aLoader.cpp:591-595`）＝ `Rise_1 out==false && Rise_2 out==false && Sep_1 out==false`，恢復後回到 `ready=1`。這正好就是 D1／第 ⑧ 項的姿態。

**請確認：您在現場看到夾著的，是載盤車上的 Push/Lean 兩支夾爪，還是前分料器的 Rise-1/Rise-2？**

---

## 6. 回答您的兩個直接問題

### Q-A：「Loader hangup 我有修，你確認一下」

**結論：這個修改不是那個 hang 的解，也不在 repo 裡，建議「取代」而非「採用」—— 但您的意圖是對的，我會用 D5 的形式接進來。**

改動內容（`site0730/.../aLoader.cpp:779-780`，存檔 07-30 16:12:06）：
```cpp
//    bool bInputEmpty  = (...SnLoader_InputHasTray...);   //20260730 need continue supply tray
    return bSourceEmpty;
```
repo 端該函式自 `3a99a25`（07-06）以來未動過。

- **有編譯上機嗎？** 有。`aLoader.obj` 16:12:17、EventLog 16:12:29 "Program Start"。16:14 之後所有快照都是這顆 binary。
- **修好 14:54 的 hang 了嗎？—— 沒有，在那段 trace 裡是 no-op。** Side1 停在 `FeedTask=100`；要停在那裡，就必須通過 `case 10`，而 `aLoader.cpp:1546` 只要 `SnLoader_InputHasTray` 讀到 ON 就會轉去 9500、`:1514` 讀不可信就會停在 10。它停在 100 → **當下 `SnLoader_InputHasTray` 是 OFF → `bInputEmpty` 為 true → 加不加那一項結果完全一樣。**
- **修好什麼了嗎？—— 修好第 ⑨ 項。** 17:31:26.395 出現 `"AUTO CleanOut: Loader source dry, AMR car-window elapsed, no new car (side 2)"`，之後 `MES0920` 再也沒出現。您抓對了那個否決自動判斷的條件。
- **但也帶來三個問題：**
  1. **很可能造成 17:35–17:45 那一輪新 hang。** 拿掉該項後，rise-1 伸出時 `IsSupplyCarDry()` 會變成 true，於是 D1 的退場守衛可以在分料半途開火 —— 這個死結**只在您那顆 hot-fix binary 上出現**，repo 的 AND 形式剛好意外擋住它。
  2. **它刪掉了 case 9000 自我回收的唯一路徑**（`aLoader.cpp:1672-1683`），而 `IsAllCleanOutFinish()` 的 `:988` 還是硬卡同一顆感測器 → 該側退場、沒人清那盤、CleanOut 永遠不會完成、**而且沒有警報**。這比原本的問題更糟。
  3. `IsSupplyCarDry()` 有**兩個需求相反的呼叫者**，這一改兩邊都被改到。

**另外要更正一個說法**：`SnLoader_Inputend` 與 `SnLoader_InputHasTray` 都是**單一全域感測器**（`database.h:290`/`:295`），AND 形式是**兩側對稱地**被擋，不會出現「A 側擋住 B 側」的不對稱現象；外顯應該是「CleanOut 永遠不完成」。

### Q-B：「PassBinZero 昨晚修好了」

**結論：那個具名的停機模式已經關閉，但調查沒有結束。**

已關閉：`cb0591f`（19:10）修掉 `BinAreaMap.Clear()` 洗掉 PassBin 的順序錯誤；`1b64e66`（19:43）整個移除該設定，`main.cpp:2134-2138` 的「沒設 Pass Bin 就不准 Start」閘門也刪了。兩者都在現場結束後才進 repo。

現場那顆 bug 確實存在：`setup.cpp:773 BinAreaMap.Clear();`（其最後一行是 `CosFunction.cpp:304 PassBin=0;`）跑在 `setup.cpp:789-790` 讀 `GetPassBin()` 的 **17 行之前**。

**但四點未結：**

1. **07-30 的真因從來沒被確立，而且主流假說被您自己的 log 推翻。**
   `csystem.cpp:1421` 先過 `CheckLotDataReady` 才在 `:1424` 記錄，所以每一行 `"MACHINE START by operator"` 都證明閘門**沒有**開火。16:35:39 洗掉之後的 log 裡有 **18 行**，第一行在 **16:38:10.963**（2.5 分鐘後）。不是「不斷」。
   更關鍵：`LotData.json` 裡的 live `PassBin` 在 16:38、16:58、17:07、17:32、17:41、17:45、17:47 **讀到 1**，只在 16:35、17:36、17:50 讀到 0，而 `BinAreaMap.ini` 從 16:35:39 起 **md5 完全凍結**。在那份原始碼裡 `PassBin` 只有 `Clear()`、`LoadFromIni()`、`SetPassBin()`（後者必接 `SaveDefault()`）三個寫入者 —— **「記憶體 0→1 但檔案沒動」在該程式碼裡不可能發生。**
   兩個候選解釋：**(a) 記憶體覆寫** —— `CosFunction.h:89-93` 裡 `PassBin` 是 struct **最後一個成員**，緊接在 `ErrorBinArea` 之後，`AreaToBin[]`/`ErrorBinToArea[]` 的越界寫入正好會落在這兩個 int 上；若是這個原因，**拿掉 `PassBin` 只是把靶子往前移一格，沒修到越界寫入**。(b) 執行的 exe 不是那棵樹連出來的（但 obj 時戳與之相符，反而更難解釋）。
   **這是全報告價值最高的未結項目。**
2. **16:35:39 真正洗掉檔案的，可能不是 reject 分支。** `setup.cpp:742-749` 重建 `cbbPassBin` 後做 `ItemIndex=(PassIdx>=0)?PassIdx:0;` —— 只要存的值不在 grid 裡或 grid 還沒填好，combo 就**靜默退回 item 0**，下次存檔 `NewPassBin==0`，`:789-790` 判定「沒變」，`else` 走 `SetPassBin(0)`，`:798` 落盤。這類「UI combo 靜默退回 index 0 再覆寫設定」值得在整個 `setup.cpp` 掃一遍。
3. **`2b51088` 換上了更嚴格的新閘門**：`main.cpp:2113-2127` 要求每個註冊 lot 都要有 2D 資料。搭配 host 疊加式宣告（`0ba540a`），一個操作員從沒輸入、WebAPI 又拉失敗的 lot，現在會用 `"No 2D data for lot <X>"` 擋住 Start —— 操作員一樣會回報成「不能 start」。**請確認閘門擋住時畫面上有可達的 Delete/Remove Lot 控制。**
4. **新的靜默風險取代了舊的吵鬧風險** —— 見下方 D6。

**可以對客戶講的一句話**：「會被歸零的 Pass Bin 設定已經整個從機台移除，這個停機不會再發生；但 7/30 當天記憶體裡數值來回跳動的原因我們還沒解釋，列為未結項目追蹤。」

---

## 7. 其餘缺陷

### D4（P1）— LK1 幽靈盤

`aLoader.cpp:1545-1546` 的自我認養跑在 `AcquireFrontOwner`（case 100）**和** Y 軸移動（case 1000）**之前**，而 `SnLoader_InputHasTray` 是**前站唯一一顆固定點感測器**（不隨載盤車移動）。

實證（`17_32_15`）：
```
17:31:32.268 "HEAL Loader LK1: orphan tray ... (side 1)"
17:31:34.070 "HEAL Loader LK1: orphan tray ... (side 2)"     <- 相隔 1.8 秒，同一顆感測器
FeederDecision: Y1enc=83500  Y2enc=36501    (feed Y = 100)   <- 兩台都不在前站
TaskHistory   : Side1 1000->2000 僅 24 ms（同日真實進料要 19.4 秒）
17:31:58.949 "Loader Tray has IC,please remove"  -> 17:32:13 RETRY -> 17:35:35 SKIP
```
修法：`aLoader.cpp:1545` 加「本車確實在 feed Y」的位置條件（用既有 `MoveLoaderY` 的到位容差，**不要零容差**）。
**不要用 front-owner 當守衛** —— 這條路徑上根本還沒有人持鎖（快照就寫著 `iFrontOwner=0`）。

### D8（P1，併入 D4 一起修）— `aLoader.cpp:2269` 自由字串警報 + 未處理的 K_SKIP

```cpp
aLoader.cpp:2269  int ret=ShowMyError(LangT("Loader Tray has IC,please remove"), K_RETRY|K_SKIP);
aLoader.cpp:2270  if(ret==K_RETRY) break;
aLoader.cpp:2275  PushCylinder->Reset(); LeanCylinder->Reset(); Task=2000;   // <- SKIP 從這裡掉下去
```
三個問題：沒有註冊碼（違反警報 SSOT）、**文字語意相反**（感測器 OFF 代表盤**沒有**到後段）、`K_SKIP` 有給但沒處理 → 把不存在的盤「退料」掉並在 case 2000 把 `bRearHasTray` 設 true。操作員 17:35:35 按的就是這個 SKIP。

修法：註冊新碼（例如 `JAM0915`）→ 換成有碼、文字修正、`K_SKIP` 明確處理（仍要保留兩個 `Reset()`）。**AlarmList.csv 與雙語 remedy 要同步（Big5，byte-safe）。**

### D5（P1）— AMR 仍問 Clean out（第 ⑨ 項）

`aLoader.cpp:1722` 用 `IsSupplyCarDry()`（source **AND** input），但 input 端還有盤正是 Clean Out 要清的東西，不該否決「要不要進 Clean Out」的決定。落空後往下六行就是 `aLoader.cpp:1731` 的 `MES0920 ... K_RETRY|K_CLEAN_OUT` —— 全程式**只有這一處** `ShowMyError` 帶 `K_CLEAN_OUT`，`MES0920` 也只有這一個 raise site，**列舉是完整的**。

實證：
```
14:44:54.225 MES0920 "... SnLoader_Inputend expect=ON actual=OFF ..."   <- live 取樣
14:45:08.384 "CLEAN OUT pressed"
14:47:12.791 CLEAN_OUT,138,,MES0920
General.ini : UseAMR=1   AmrFeedWaitSec=60
```

修法 —— **拆分 predicate，不要刪項**：
```cpp
// aLoader.h:125 新增宣告
bool IsSupplySourceDry();

// aLoader.cpp 於 :767 之前新增（上下都要 //--- 分隔線）
bool TLoaderModule::IsSupplySourceDry()
{
    if(IsSoftSimulate())
        return (IsContinuousFeed()==false);
    return (HSys.Sen.SnLoader_Inputend.Enable==false || HSys.Sen.SnLoader_Inputend.IsOff());
}

// aLoader.cpp:1722
&& IsSupplySourceDry())
```
**`aLoader.cpp:767-781` 本體保持 byte-identical**（退場守衛仍需 AND 形式）；**`:1724` 的 `RecordProcess` 字串也不要動**（那是 07-30 唯一的正面證據，現有 log grep 依賴它）。順帶更新 `:772-774` 已失真的契約註解。

接受的行為改變：AMR 模式下操作員失去「不要 clean out」的選擇。換車比 `AmrFeedWaitSec`（目前 60 秒）慢的話，**請調高該參數，不要把否決條件加回去**。

### D6（P2）— DiePass 空值 = 全批 FAIL，且沒有閘門

Pass Bin 移除後，PASS/FAIL 純由客戶 per-IC `DiePass` 決定，`CosFunction.cpp:1391-1407` 把「非 PASS token」一律當 FAIL，**空字串也算 FAIL**。兩種常見情況會讓整批空白：機台手動輸入的 2D（`CosFunction.cpp:1128-1133` 的 `AddItem` 硬塞空字串，UI 也沒有 DiePass 欄）、以及 JSON/WebAPI 省略該欄位（`CosFunction.cpp:1644-1646` 是 optional）。結果是**整批靜默倒進同一個 Auto**。

`sr/...16_35_46.../LotData.json`：100 筆記錄，`"DiePass": ""` 100 筆、非空 0 筆 —— 但每筆也都是 `"Source": "OFFLINE"`、`SIMU_LOT_*`，**是工程模擬資料，不能當成客戶實際會漏欄位的證據**。

修法需先取得客戶答覆（見 §9），形狀應為 **per-lot**、且測的是「這批有沒有任何 DiePass 值」（資料來源測試）而非「有沒有 PASS」，否則合法的全 FAIL 重測批會被擋。

### D7（P2）— Offset 用「停機警報箱」報成功（第 ⑦ 項）

```cpp
uOffset.cpp:518   if(ShowMyMessageBox_YES_NO("Save offset and apply ?")!=...)   // 第一個框，有把關作用
uOffset.cpp:520       SaveWorkFile();
uOffset.cpp:521       UpdateAllParameter();
uOffset.cpp:522   ShowMyMessage(LangT("Offset saved and applied."));            // 第二個框，什麼都沒把關
```
`ShowMyMessage` 是**警報介面**：`mymessbox.cpp:91-92` `DecStopAllMotor()` + `SystemStart=false`，`:447-453` FormShow 再套一次，`:489-490` 蜂鳴器，`csystem.cpp:1095-1096` 塔燈轉 Pause。**機台為了說「存好了」而停機、響蜂鳴器、亮 Pause 燈。**
同樣的組合也在 `:528`/`:592`（Re-alignment，會重寫 `system\tech.ini`）與 `:604`/`:608`（Clear All，**根本沒寫檔卻照樣停機**）。

**SECS log 三次實證**（CEID 19 = 進 Offset 頁、CEID 73 = 訊息框關閉）：
```
16:29:47.864 CEID=19 -> 16:30:14.824 CEID=73 -> 16:30:15.992 CEID=73   (相隔 1.168 s)
17:08:51.284 CEID=19 -> 17:09:01.682 CEID=73 -> 17:09:02.747 CEID=73   (相隔 1.065 s)
17:11:17.363 CEID=19 -> 17:11:29.323 CEID=73 -> 17:11:29.867 CEID=73   (相隔 0.544 s)
```
嚴重度更正：同一時間軸顯示操作員每次進 Offset 前都先按了 Pause，所以**那次停機在現場是 no-op**；實際確認的是「兩個框 + 蜂鳴 + Pause 燈」，停機是**潛在**危害。

**Stage 1（立即、近乎零風險）**：`uOffset.cpp:522`、`:592`、`:608` 三處把 `ShowMyMessage` 換成 `ShowMyOKMessageNoStop`。

**Stage 2（需先回答 §9 Q6-Q8）**：拿掉完成框改為 `lblExplain` 行內提示 + 寫入變更歷程。兩點必須注意：
- **`btnReAlignClick` 也要加 `SystemStart` 守衛**（那是最具破壞性的按鈕：56 個 `TeachBase` 欄位 + `memset` + 重寫 `tech.ini`）。注意 `UpdateAllParameter()` 在 `:521`/`:590` 就已經跑了，現在的框是「事後停機」，**從來就不是前置互鎖**。
- **要在 Apply 當下記錄，不要在編輯當下記錄**：鍵盤輸入只存在 RAM 的 `Offset` 結構，`btnExitClick`（`:595-599`）只是 `Close()`，下次 `FormShowHandler`→`OpenWorkFile`（`:407-435`）會重新從 `.ofs` 載入而**靜默丟棄**。記錄了後來被丟棄的變更、又沒有丟棄紀錄，比不記錄更糟。

對齊 9045 的變更歷程 sink 已存在：`cEventLog`（月夾 + retention，且每份 State Record 都會帶走），建議碼 `PARAM_OFFSET`，格式仿 HT9045 `TMyLog::Compare_Diff`（`handlerlog.cpp:507-532`）：`user / field / old => new / timestamp`。

### D9（P3）— 觀測性缺口

1. **`FeederDecision.txt` 沒有 dump `SnLoader_Inputend` / `SnLoader_InputHasTray`** —— 本次調查所有關於 `IsSupplyCarDry()` 的結論都只能用排除法推得。這是最大的缺口。
2. 載盤車夾爪狀態沒 dump（`aLoader.cpp:2598` 之後補 Push/Lean 的 out/on/off，仿 `aEmpty.cpp:1373-1374`）。加之前先確認 dump path 裡呼叫 `IsOn()` 不會觸發 sensor refresh（`aEmpty.cpp:1365-1371` 有明文警告）。
3. Offset 變更歷程（見 D7 Stage 2）。

---

## 8. 現場版本 ↔ repo 對帳（Issue H）

**要往前搬的，只有一項**：`aLoader.cpp:779-780` —— 但**搬意圖、不搬 diff**，實作為 D5。不要保留您註解掉的那一行，直接刪除並在 ASCII 註解裡寫明原因。

**不要搬回來的**：
- `MachineType.h:7` —— dev tree 保持 `#define SOFT_SIMULATE` 開啟。
- 其餘 **18 個檔案全部是 repo-ahead**（`cb0591f` / `2b51088` / `1b64e66` 三個 commit，不是兩個；`setup.cpp` 把現場版本釘在 pre-`cb0591f`）。搬回去等於 revert 掉整晚的工作。

**合併陷阱：`aLoader.cpp` 是混合的** —— 同一個檔案同時有 site-newer 的 hunk（779-780）與 repo-newer 的 hunk（`BinAreaMap.GetPassFailClass` → `LotRegistry.GetPassFailClass`，約 2024 與 2138-2140 附近）。**只能手動合併，兩個方向都不可整檔覆蓋。**

獨立時戳佐證：現場每份 `FeederDecision.txt` 第 5 行都還印著 `PassBin=`、`LotData.json` 也還有 `"PassBin":` —— 這兩個 dump 欄位只存在於 pre-`1b64e66` 的程式碼。**現場 exe 確實比 HEAD 舊。**

**沒有任何 repo 變更會 revert 掉現場的修改。**

---

## 9. 實作順序

全程遵守：BCB6、無 C++11、無 FSM、機台控制路徑不得有 `Sleep()`／阻塞迴圈、新註解一律 ASCII English。每步編譯閘門：**刪掉改動的 `.obj` 再建置**。

| 步驟 | 內容 | 檔案 | 建置 |
|---|---|---|---|
| 0 | 基準線 | — | `scripts/ops/build-ht160s.ps1 -Clean` 取得綠燈 |
| 1 | **D1（P0）** `:1309` + `:1353` | `aLoader.cpp` | 刪 obj → `-Clean` |
| 2 | **D2（P0）** un-retire 清 latch + `IsAllCleanOutFinish` 加 `fHasTray` | `aLoader.cpp` | 刪 obj → `-Clean` |
| 3 | **D5（P1）** `IsSupplySourceDry()` | `aLoader.h`、`aLoader.cpp` | **full build**（新增公開方法） |
| 4 | **D4 + D8（P1）** 先註冊警報碼 → Y 位置守衛 → 修警報與 K_SKIP | `database.cpp`、`AlarmList.csv`(Big5)、`aLoader.cpp` | 刪 obj → `-Clean` |
| 5 | 編譯閘門 | `MachineType.h` | `-Full`；註解掉 SOFT_SIMULATE 再 `-Full` 確認 exit 0；**還原後重建** |
| 6 | **D3（P1）** | `aEmpty.cpp`(byte-safe)、`aTrayArm.cpp` | 刪兩個 obj → `-Clean` |
| 7 | **D7 Stage 1（P2）** 三處換 helper | `uOffset.cpp` | 刪 obj → `-Clean` |
| 8 | **D9（P3）** 觀測性 | `aLoader.cpp` | 刪 obj → `-Clean` |

**模擬驗證不了的部分（必須上機）**：`IsInputHasTrayTrustworthy()` 在 sim 恆回 true（`aLoader.cpp:1443-1444`）；D3 的 `IsCarrierParked()` 在 sim 恆 true、place gate sim 繞過、夾持確認 sim 短路。**D3 因為本質是 race，「跑一次沒撞」不算證據 —— 要直接讀 `FeederDecision.txt` 的 `bRearReturnInProgress` 是否全程保持 1。**

### 需要決策後才能動的項目

| # | 項目 | 卡在 |
|---|---|---|
| 1 | 已被丟下的分料器自動復原 | 設計。天真作法會**無聲卡死**：`DoFeedTray(...,0)` 清掉 `State->FeedDelay`，而 `DoFrontDestackDown case 5` 是 `if(Delay.Off())`（`aLoader.cpp:2366`），`HTimer::Off()` 在 `ulStartTicks==0` 時**永遠回 false**。目前解法是手動 IO 畫面（`iosetview.h:124`）。 |
| 2 | D6 DiePass 閘門 | 客戶答覆 Q1-Q3 |
| 3 | D7 Stage 2 | Q6-Q8 |
| 4 | 改用 `SnLoader_TrayPos1/2` 當每車實際盤況真值 | 上機確認實體安裝位置與 `InType` 極性。**這是整個幽靈盤家族的結構性正解，優先度高於上面所有 patch。** 目前這兩顆在 `IO_Table.csv` 是 `Enable=1`、`database.cpp:1103-1104` 有命名，卻**沒有任何邏輯在讀**（只有 `iosetview.h:104-105` 的 LED）。注意 `TrayPos2` 是 `InType=1`（反相）、`TrayPos1` 是 `InType=0`。 |
| 5 | PassBin 0↔1 跳動 | 復現資料／記憶體佈局稽核 |

---

## 10. 待確認問題

**對客戶（京元）**
1. 全 FAIL 批（完全沒有 PASS 單元）是不是合法的生產情境？
2. WebService#11 回應是否**保證**帶 `PssFail`/`DiePass`？若可能省略，`1b64e66` 沒有 D6 就不宜出貨。
3. 手動輸入的 2D 需不需要在 By Lot+PassFail 下運作？（目前做不到）
4. AMR 模式下失去「不要 clean out」的選擇是否可接受？

**對現場工程師**
5. **第 ⑥ 項您實際看到的是哪一組汽缸？**（見 §5）
6. Offset 畫面要保留幾個對話框？建議：保留三個 YES/NO 確認（它們在把關破壞性動作），移除三個「完成」框。
7. Max/Min 上下限編輯（`uOffset.cpp:611-641`，會立刻寫入 `OffsetLimit.ini`）要不要進變更歷程？
8. 現場實務上有登入使用者嗎？若都跑 `ROLE_OPERATION` 不登入，稽核行會全部是 `user=(no login)`。
9. **感測器健康度**：`SnLoader_Inputend` 在 14:48-14:50 什麼都沒動的情況下以 8-40 秒週期跳動；`JAM0913` 在 2026/07/29 15:37 對 `SnLoader_InputHasTray` 以**相反方向**開火。兩顆都像是臨界。建議做 `SnLoader_InputHasTray` 的四組合測試（rise-1 下/上 × 盤在車上/盤在進料站），確認「rise-1 上升會誤亮」這個前提（`aLoader.cpp:1433-1436`）是否成立 —— 這個前提是「repo 版本比現場版本安全」這個判斷的承重點。
10. `D:\HT160S_BCB\data\Test20260611\*` 是否可寫？部署 exe 的連結時戳是多少？（§6 第 1 點）
