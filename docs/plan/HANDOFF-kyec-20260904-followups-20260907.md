# HANDOFF — 京元 2026-09-04 測試回覆案（2026-09-07 收工狀態）

> 這份是**接手用**的單一入口。新 session 只要讀這一份，就知道做到哪、下一步是什麼、哪些事已經裁定過不要再問。
>
> - 分支：**`feat/amr-lot-identity-d4`**（從 `6f2a8ec` 分出）
> - 撰寫時間：2026-09-07
> - 相關計畫全文：`docs/plan/cleanout-amr-lotend-collect-all-cars-plan-20260907.md`（**其 v1/v2 設計已被使用者推翻，見 §3**）

---

## 1. 已完成並提交（依時間順序）

| Commit | 內容 | 狀態 |
|---|---|---|
| `03251d1` | AMR bin-setting SVID 38234-38236 / 38243-38245 在 By Lot+PassFail 模式改送**裸分類碼** `1`=PASS / `2`=FAIL（移植自 `steven/`，未複製其檔案 —— 他的 `aAuto1To6.cpp` 第 1 行 Big5 註解損毀成 `EF BF BD`） | 已推送 |
| `6f2a8ec` | SECS 文件補上值格式＋breaking change 註記：`HT160S_SECS_Interface_Spec_20260727.md/.html` ＋ SSOT 工作簿 `SECS_GEM功能_Handler_20260903.xlsx` SVID 頁第 57-59 / 66-68 列 | 已推送 |
| `f799d88` | **變體乙**：Lot End 後保留唯讀身分快照（SVID 1006 / 1009 / 66040-66045 / 38234-38236+38243-38245），sticky-when-empty，下一次 Lot Start 解除。活表 `LotRegistry` / `LotBinBinding` **照舊在 Lot End 清空**，所以路由零風險 | 已推送 |
| `f546eb4` | D4 計畫文件（含兩輪對抗式複驗全文） | 已推送 |
| `9476a9f` | 修 `f799d88` 兩個缺陷：arm 太晚（`NoteLotStartTime(false)` 早 41 行就清掉 1009 來源）、裸 START 不解除凍結（`DoStartArm` 不經 `LotStartCore`） | 已推送 |
| `7aedff5` | SECS `CLEAR_LOT_INFO` 解除凍結（host 明示要清就真的清；操作員按鈕與 AMR 清機收尾兩條路照舊保留） | **未推送** |
| `49a0ff0` | Auto 疊身分改讀 `Car[].CarID`（新 accessor `GetCarIdentityID`），三個消費端：客戶 CSV 第 9 欄、`palAutoNNID` 面板、UPH per-tray log | **未推送** |
| `4f7638d` | AMR 下料回報盤數改成 **work-only**（在 SECS 發布邊界減掉 header，header 取自進料側同一組設定 `iAmrCoverTray`/`iAmrIdentityTray`，clamp 至 0） | **未推送** |

**接手第一件事**：`git push origin feat/amr-lot-identity-d4`
（2026-09-07 當下被權限classifier擋住，未繞過。）

---

## 1.5 🚨 接手第一優先：`49a0ff0` 有一個未證實的回歸風險

**風險**：`49a0ff0` 把客戶 CSV 第 9 欄改讀 `Car[].CarID`。若這台機器的 Auto 出料車**從來沒收到過
身分盤**，`Car[].CarID` 就永遠是空字串 → 第 9 欄會從「錯值」變成「**永遠空白**」。
客戶會看到一個原本有資料的欄位變空。

**支持這個風險的證據**（`eTrayKind`：`Normal=0 / Identity=1 / Cover=2`，`MotorAndIO/MyMotor.h:51-53`）：

```
9/4 EventLog TA_DIVERT :  3× kind=0 (Normal)、1× kind=2 (Cover)   → 零筆 kind=1 (Identity)
9/3 EventLog TA_DIVERT : 12× kind=0 (Normal)                      → 零筆 Identity、零筆 Cover
```

且第 9 欄的舊值（`...012` / `...027` / `...017`）正是「`CarID` 為空、`WorkingTrayID` 帶著別盤洩漏碼」
會呈現的樣子。

**但尚未證實**，原因要說清楚：全天七筆 RPTID 2000（帶 carrier ID 的報表）**全部在 Lot End 之後**
（15:44:13 起），而 Lot End 已經 `Car[].Clear()` —— 所以那七筆的 carrier ID 全空是**被清掉**造成的，
**不能**當成「生產期間也沒值」的證明。生產視窗（15:24:22–15:29:19）內**沒有任何 RPTID 2000 事件**，
所以線上沒有取樣。TA_DIVERT 也只是**改道**那一條交付路徑，正常 AMR_SUPPLY 派工不走這行 log。

**收斂方法（一次上機即可）**：開一個批、在**批次進行中**用 S1F3 讀 SVID 38205-38207
（Auto1-3 Carrier ID），或抓一份 State Record 看 `Car[].CarID`。

- 讀到有值 → `49a0ff0` 正確，直接推送
- 讀到空 → **先不要推 `49a0ff0`**（或推了但要知道第 9 欄會變空），真正的問題在上游：
  身分盤沒有進到 Auto 出料車，而 `[AMR] IdentityTray3..8=1` 的設定說每台車應該有一片。
  這是機構/流程問題，不是這一欄的問題

⚠ 不要用「`CarID` 空就退回 `WorkingTrayID`」當 fallback —— 舊值是**別台車的洩漏碼**，
非空的錯值比空白更危險（稽核裁定）。

---

## 2. 使用者裁定（**已定案，不要再問**）

| # | 事項 | 裁定 |
|---|---|---|
| R1 | SVID 38234 值格式 | **維持裸分類碼 `2`**（不保留 `NQ8002ZAA1:` 前綴）。使用者曾口述為 `NQ8002ZAA1:2`，經確認後裁定照原開發（Steven）的實作 |
| R2 | 變體乙要不要設定開關 | **不要**。只保留新行為，無法切回舊行為 |
| R3 | `CLEAR_LOT_INFO` 要不要解除凍結 | **要**（採納建議） |
| R4 | `MachineType.h` `SOFT_SIMULATE` | **保持關閉**，目前 EXE 是真機組態產物 —— 這是刻意的。⚠ `CLAUDE.md` Compile Gate 寫「dev keeps it on」與此相反，尚未修 |
| R5 | 客戶通知 | **只更新技術文件**，使用者自己手動提供客戶。交付包已備於 `docs/SECS/deliver_20260907/`（僅 `HT160S_SECS_Interface_Spec_20260727.html` 與完整版工作簿有變更；客戶版 `_manual.xlsx` 的 SVID 頁只有四欄、從來沒有「值域或格式」欄，所以無需更新） |
| R6 | `iTrayCount` 要不要改成只算工作盤 | **不動運算**（三重角色會壞），改在**SECS 發布邊界減 header**。已實作 `4f7638d` |
| R7 | 只有身分盤+上蓋、零顆 IC 的流道要不要叫 AMR | **要叫**。邏輯是「`SnAutoX_InputEnd` 有亮就叫車」，不看工作盤數 |
| R8 | D4 逾時自動重送 | **取消，不做**。`WAR0962` 維持現況（會停機、只給 `K_RETRY`）。但**必須確保 Lot End 後給 AMR 的資訊是正確的，不是一堆 `""` 和 0** |
| R9 | Lot End 資料落地要不要拆 | **要拆**（落地類前移到開始等待那刻）—— ⚠ 但 R8 取消等待之後，這一項**已無適用場景**，因為 Lot End 立刻發、28ms 內就落地了 |

---

## 3. ⚠ D4 設計已被推翻，範圍大幅縮小

`docs/plan/cleanout-amr-lotend-collect-all-cars-plan-20260907.md` 裡的 **v1/v2 設計（讓批次等 AMR 收車才結束）已被使用者推翻**。兩輪複驗在那個設計上找出 6+9+9 個缺陷，全部因為推翻而失效 —— **不要照那份文件實作**，只把它當歷史與 code 事實的參考。

**使用者的新架構（2026-09-07 原話）**：

> 全部退出後，Lot end + 保留可交換用資訊 + Auto 有偵測到盤就送 272（超時就重送，直到沒盤）

其中「超時就重送」後來由 R8 取消。所以 **D4 現在只剩兩件事**：

### ① 「`SnAutoX_InputEnd` 亮就叫車」這條觸發**還不存在**

今天 `uAgvStation.cpp` `PollAndCall` 的 P4-P9 迴圈：

```cpp
bool bTrueFull = AutoModule->IsOutputCarFullForAmr(a) || AmrInject.AutoFull(a);
bool bCollect  = IsCleanOutCollectDueForAmr(a);
bool bFull     = bTrueFull || bCollect;
```

- `IsOutputCarFullForAmr` 讀 **`SnAutoX_InputFullTray`**（滿盤 sensor），不是 InputEnd
- `IsCleanOutCollectDueForAmr` 第一道閘就是 `RunMode!=Run_CleanOut → return false`，而 Lot End 後 `csystem.cpp` 已 `ChangeRunMode(Run_Normal)`

→ **目前 Lot End 之後一筆 CEID 272 都不會發。** 這是 R7 要的行為，尚未實作。

現成可用：`TAutoModule::IsAmrTaken`（`aAuto1To6.cpp:1697-1707`）已經讀 `SnAutoX_InputEnd`，註解明載 **ON = has tray**，而且它**已經是 CEID 274 的釋放條件**。

> ### ⚠ 更正（探查結果，推翻先前的說法）
>
> 我先前說「一顆 sensor 就能閉合整個迴圈」—— **錯的**。叫車側與停止側是**兩顆不同的實體 sensor、
> 在不同的 MotionNet 模組上**：
>
> | 用途 | sensor | getter |
> |---|---|---|
> | 叫車側（現有） | `SnAutoX_InputHasTray` / `_InputFullTray` | `GetInputHasTray` `aAuto1To6.cpp:328-340` |
> | 停止側（CEID 274 釋放） | **`SnAutoX_InputEnd`** | `GetInputEndSensor` `aAuto1To6.cpp:356-368` |
>
> 而 2026-09-04 這兩組**在六個站同時互相矛盾**：六顆 `InputEnd` 讀 1，六顆 `InputHasTray` 讀 0。
>
> **`SnAutoX_InputEnd` 全樹只有一個功能性消費者** —— `IsAmrTaken`。它不在任何 ladder、不在 State
> Record 的具名 Auto 傾印裡、沒有任何地方把它當「車在位」讀。而 P1/P2/P3 的同名點雖然極性相同
> （ON=有盤），指的卻是**機台另一側的供料匣**，不是出料車 —— 同一個名字兩種指涉。
>
> **若它誤亮或黏住 ON**：今天損害有界（274 不發、站停在 READY、車帳不清、300 秒後 `WAR0962`
> 給操作員一個 `K_RETRY`）。**把叫車也綁到同一個點，會把這個有界、操作員看得見的停滯，
> 變成無界的 S6F11 發送** —— 因為 `WAR0962` 的答覆路徑刻意不會停止叫車。
>
> **`TMySensor` 完全沒有 debounce**（`mysensor.cpp:35-76`；`IO_Table.csv` 裡 sensor 列的
> `OnDelayTime`/`OffDelayTime` 全空，只有汽缸帶延時）。`Enable==false` 時 `IsOn()` 與 `IsOff()`
> **都回 false** —— 所以停用的 sensor 既不是亮也不是滅，方向由各消費者的守衛決定：
> 叫車側全部 fail-safe（不叫），而 `IsAmrTaken` fail 成「未取走」→ 站永遠停在 `AGV_READY`。
>
> ### 2026-09-04 那次六 bit 同時翻轉，最可能是 MotionNet 埠讀取失敗
>
> 六顆同在 `Lane0 IP2 P1 B0..B5`（同一個 byte）**一起**從 0 變 1，而鄰近埠**各自獨立**變動
> （`IP2 P0`：`SnEmpty_InputEnd` 維持 1、`SnLoader_Inputend` 由 1→0），且每個 Auto 卡
> （IP3/IP4/IP5/IP6）上的 InType=0 點全部維持 0。**一個埠的所有 bit 一起跑到 fallback 值、
> 而鄰埠照常變動，這是埠粒度的失效特徵。** 加上物理上不可能：Auto4/5/6 整批沒收過一顆料，
> 而它們的 `InputHasTray` / `InputFullTray` / `OutputBottomHasTray` 在同一瞬間全讀 0。
>
> → **上機量測這六顆的極性與穩定性是實作前提，且要一併確認該埠沒有讀取失效。**

### ② R8 的「不是一堆 `""` 和 0」尚未達成

`f799d88` 保住了 1006 / 1009 / 66040-66045 / 38234-38236，但**這三個沒保住**：

| SVID | 來源 | Lot End 後 |
|---|---|---|
| 38205-38207 / 38199-38201 Carrier ID | `Car->CarID` | **`""`** |
| 38225-38227 / 38246-38248 Tray Count | `Car->iTrayCount` | **0** |
| 38231-38233 / 38240-38242 Device Count | `GetAmrDeviceCount(a)` | **0** |

成因：`csystem.cpp:1929 InitialAllTask()`（預設 `bKeepMaterial=false`）→ `aAuto1To6.cpp:119-120` `Car[Index].Clear(); InitAutoCarStack(Index);` → `MotorAndIO/MyMotor.cpp:272-278` 把六台車 `CarID=""`、`iTrayCount=0`。

這三個陣列（`uAgvStation.cpp:510-512` 寫入）與 `BinSetting[]`/`LotNumber[]` **同樣是每 tick 刷新的純快照、無 routing 讀者**，所以可以套用同一個凍結機制。

**⚠ 但不能是單純的 sticky**：CEID 274 那條路在 AMR 真的收走車之後會**故意**把盤數歸零（`uAgvStation.cpp:684-685`，註解「car is now empty, keep the SVID snapshot honest`」）。單純 sticky 會讓下一個 tick 把舊值撈回來。

**正確做法（已定，不需再裁定）**：每站一個保留閂，在 CEID 274 那裡釋放。凍結窗邊緣可由協調器自己偵測（`IsLotIdentityFrozen()` 的 0→1 邊緣），不需要跨模組 hook。

探查另外確認了三件事：

- **實作只要 3-6 行。** `TAgvCoordinator::Reset()`（唯一會清空這五個快照陣列的東西）**全樹沒有
  runtime 呼叫者**（只有建構子），所以這些陣列本來就對 Lot End 免疫 —— 它們變空的**唯一原因**
  是 `PollAndCall` 下一個 1 秒 tick 無條件從已被清空的 `Car[]` 重抄。把 `f799d88` 既有的
  sticky-when-empty 規則套到那三行就夠：不需新狀態、不需新呼叫點、不需動 `csystem`、
  不需新 include（`cprod.h` 已由 `f799d88` 引入該檔）。
- **不要用 `InitialAllTask(true)`（keep-material）。** 呼叫點只有一行，但影響擴散到六個模組，
  且會把上一批的物料帳帶過批界。
- **也不要「在 `InitialAllTask` 之前快照 `Car[]`」。** 更多程式碼、同樣結果，而且**不完整** ——
  **手動 Lot End 按鈕從來不呼叫 `InitialAllTask`**（`btnLotEndClick` → `DoLotEndProcess`，
  全函式內沒有 `InitialAllTask` 也沒有 `ChangeRunMode`），所以快照法還得額外避免遮蔽該路徑上仍然
  活著的值。

### ③ 範圍修正：host 叫車**不需要** carrier ID 與盤數

探查證實：**站別 bitmap（SVID 38219）＋ 站名就是派車的全部契約**，而且在現場端到端證明過 ——
全部 carrier ID 空、全部盤數 0 的情況下，`START_AGV` 照樣以站名 + `Action`/`NA` 派車成功
（9/4 15:22:33 那筆 `START_AGV` 的 `L[11]` 就是九個站名各帶 Action/NA ＋ 兩個 Loader 計數）。

→ 所以 ② 要修的不是「讓 AMR 能來收車」（那個本來就能動），而是 **下貨的 IC 件數／盤數對帳交付品**。
把這件事講清楚可以避免下一個人以為不修就叫不到車。

### ④ 另一個要留意的既有缺陷（不在 D4 範圍，但會被 D4 放大）

每一次重新叫車都會**重發**離散的「Auto N Full」CEID（35/36/37/148/149/150），因為那個 emit
在同一個 `IDLE→CALLED` 分支裡、只 gate 在 `bTrueFull` 而**不是滿盤的邊緣**。文件說它是一次性的
「pre-notification」，實際行為是每次叫車都重發 —— 現場 9/4 同一台 Auto2 就發了七次 CEID 36。
R8 已取消自動重送，所以 D4 不會放大它，但這個缺陷本身還在。

### ⑤ CALLED 站的鎖會活過 Lot End、HOME、下一次 Machine Start

`TAgvCoordinator::Reset()` 沒有 runtime 呼叫者；Lot End 的 `InitialAllTask` 會清 `bAmrLocked`，
但緊接著 `database.cpp` 的 `AgvCoord.ReassertLocks()` **又把它加回去**（對每個 handshake 非 IDLE
的 Auto 重新 `SetAmrLock(a,true)`）；`MachineStart` 完全不碰協調器。

而**被鎖住的 Auto 對下一批完全停役**：拒收新盤（`GetTrayRequest` 回 `eTrayReqNone`）、
出料被跳過（`FindDischargeAuto` `continue`）、discharge-tail latch 被凍住，
而且在 Clean Out 會**卡住六站連鎖閘、拖住全部六站**（`aAuto1To6.cpp:1147-1159`，
其註解自述 "bCleanOutCheck is a six-station lockstep barrier, so one AMR-locked station holds the
whole drain until CEID274 releases it"）。

→ **這是 R7「Lot End 後叫車」的真實風險**：叫了、沒人來、批次結束，那一站帶著鎖進到下一批。
實作 ① 時必須一併決定這個鎖怎麼收。

---

## 4. 已知缺陷 / 不要碰的事

| # | 事項 |
|---|---|
| D1 | **客戶 CSV 第 8 欄 `Load Cover Tray ID` 是同族的另一個獨立缺陷，尚未修，且不可對客戶宣稱已修。** `sTrayID2D` 是整機一個純量（`aColor.h:49`），被每次 Color 讀取覆寫。實證：9/3 第 4/5/6 列同一片實體進料盤（`Load_X/Y` = 0,0 / 0,1 / 0,2）卻是 `...014 / ...014 / ...027`。要修得另建進料車 latch |
| D2 | 修完 `49a0ff0` 之後第 8/9 欄會**停止碰巧相等**（9/4 第 7/8/9 列今天 col8==col9==`...017`）。那是修好，不是回歸 —— 要對客戶揭露 |
| D3 | `bUseAMR==0` 時 CSV 第 9 欄**結構上永遠空白**，修前修後一樣（`CarID` 與 `WorkingTrayID` 的寫入都在 `if(bUseAMR)` 內）。若客戶正式生產跑 AMR=0，`49a0ff0` 對他無效益 |
| D4 | `aAuto1To6.cpp` 的 place 時不重驗 tray kind（`iDeliverKind` 在 pick 時 latch）→ AGV 若在 pick→place 途中收走車，一片 Cover 盤會進重播種後的 slot 0，`CarID` 維持 `""` 且 `GetNextTrayKindForAuto` 永遠不再要 Identity。**另案硬化** |
| D5 | 身分盤帶著空 2D 也照樣升位（`aColor.cpp:1332` / `:1382` 在 CCD 失敗時設 `sTrayID2D=""` 仍 return true；`aAuto1To6.cpp:859` 只測 KIND 不測空字串）。第二個獨立的空白成因 |
| D6 | `main.cpp:3357-3359` 的註解聲稱 Lot End 會覆寫 `LastLotList.ini`，但 `SaveLastLotList()` 全樹唯一呼叫者是開機 fresh-start 分支 → **結束批的批號今天就留在磁碟上**。既有缺陷，另案 |
| D7 | `[AGV] AmrFullWaitSec` 是死鍵，全樹零消費者 |
| D8 | `note.cpp:867-892` 對**任何**帶 `K_SKIP` 的警報，只要 `[SECS] AskSkipICCount=1` 就會彈鍵盤問「取出幾顆 IC」並發 CEID 78 + SVID 37010（京元用它對帳庫存）。現場該開關是 `0`，且現有 `WAR0962`/`WAR0963` 只給 `K_RETRY`，所以碰不到。**哪天有人加了帶 `K_SKIP` 的物流警報就會踩到** |
| D9 | ⚠ **`WAR0964` 不存在**。它是被推翻的 v1/v2 計畫提議要新增的碼。樹上 WAR 碼到 `WAR0962` / `WAR0963` / `WAR0970` 為止 |
| D10 | `4f7638d` 讓 SVID 38225-38227 / 38246-38248 回報的數字變小 —— **客戶可見變更，技術文件尚未更新**（38234 那次已更新，這次還沒） |
| D11 | 幾個 BCB6 檔在工作樹是**全 LF**（`aAuto1To6.h`）或**混合行尾**（曾見 `aAuto1To6.cpp` CRLF=69/LF=2166）。編修時務必先偵測該檔行尾再套用，否則 patch 匹配不到或產生巨大無關 diff。修法：commit 後刪檔 + `git checkout` |

---

## 5. 客戶原始三個問題的結論（可直接用於回信）

**Q1 「LotEnd 時機是什麼時候觸發？之前說等 5 分鐘，這次卻馬上就 Clean Out 然後 LotEnd」**

機台行為正確；不一致的是**一個 ini 值**與**一個通報缺口**。

- 觸發鏈：Loader 供料源乾 → 起 `FeedWaitTimer = [AGV] AmrFeedWaitSec × 1000`（`aLoader.cpp:1855`）→ 窗口內無新車 → 自動進 `Run_CleanOut`（`aLoader.cpp:1878-1885`）→ 全機排空 → `CheckCleanOutFinish()` → CEID 42 → `bUseAMR` 時立刻 `DoLotEndProcess` → CEID 8
- **Clean Out → LotEnd 之間沒有任何計時器**，43.98 / 42.43 秒是純機構排空時間
- 等待窗口 = `AmrFeedWaitSec`，**現場設 60**（程式碼預設 600）。三次事件精準吻合 60.000 秒：

| 日期 | 源乾 | AUTO CleanOut | 間隔 |
|---|---|---|---|
| 09/04 | 15:27:35.907 | 15:28:35.905 | 59.998 s |
| 09/03 | 15:25:23.984 | 15:26:23.982 | 59.998 s |
| 09/03 | 17:14:48.735 | 17:16:32.919 | 牆鐘 104.184 s − 暫停 44.179 s = **60.005 s**（計時器不算暫停時間） |

- **客戶覺得「馬上」的真因是通報缺口**：15:27:35.907 的源乾邊緣**線上一行都沒發**（log 第 2144 行 15:28:04.243 與第 2145 行 15:28:35.912 相鄰）。EAP 第一次看到 Clean Out 就是 CEID 27（SVID 1011 = `"Clean Out"`）在**窗口結束那一刻**。從 EAP 視角，「Loader 空」與「Clean Out」是同一瞬間 —— 因為它從來沒被告知窗口**開始**
- 「5 分鐘」的出處**未找到原始文件**。唯一真的會跑 300 秒的是 `[AGV] AgvTimeoutSec`（AGV 握手逾時 → `WAR0962`），09/04 15:49:27 那筆就是在最後一筆 272（15:44:27.363）之後 299.996 秒發的。**回信前應先找出我們當初給客戶的原文**

**Q2 「Other 軌不給 LotNo」**

- 韌體沒有叫「Other」的流道型別。客戶指的是**錯誤/溢出流道**，本機 recipe `BinAreaMap.ini ErrorBinArea=Auto1`
- 錯誤流道**永遠不會被綁定**（`ResolveAuto` 跳過它），而 66040-66045 / 38234-38236 都查 (Lot,分類)→Auto 綁定表 → 必然回空。**這是既有共識，使用者已自行回覆客戶**
- 面板端**已有** fallback（`aAuto1To6.cpp:1635` 回 `(Error lane / reject)`），SECS 端沒有
- **建議給客戶的答案**：走每日生產紀錄 CSV。逐顆記錄 `Which Auto` / `Lot` / `Code2D` / `PassFail` / `ErrorType`。9/4 那批 Auto1 收了 5 顆、每顆 2D 都有（`…LW_025/016/011/029/027`），只是 `Lot` 欄空 —— 因為那 5 顆的 2D 不在工單對照表內（TraceCode=1000 NoMap），**機台本身也不知道它們屬於哪一批**。Other 軌會混裝多批甚至無法歸屬的料，所以「一軌 = 一個 LotNo」在 Other 軌上本質不成立（也與客戶 8/31 自己「一個流道恆只對應一個批號」的裁定衝突）

**Q3 「觸發 LotEnd 後未觸發各軌道 272 收貨」**

- LotEnd **之後**不會發 272 是**設計**：`IsCleanOutCollectDueForAmr` 第一道閘是 `RunMode!=Run_CleanOut`，而 Lot End 同一瞬間就 `ChangeRunMode(Run_Normal)`
- 排空期間也沒發，因為還要 `IsFrontHasTrayForAmr()`（`SnAutoX_InputHasTray` 亮）。9/4 兩份快照六顆全 0 → 沒有盤要收 → **正確行為**
- 對照組：9/3 17:17:04 的 `MES1623 Auto6 clean-out residual tray (front=1 full=0)` 才是這功能要救的情境
- 客戶若要的是「Lot End 時自動叫走六台車」→ 就是 §3 那件尚未實作的工作

---

## 6. 未收斂的 workflow

`w1g07sw9b`（D4 依新架構重新設計）在 2026-09-07 收工時仍在跑，且它是**帶著已被 R8 取消的「重送迴圈」需求**下去的 → 回來後只取 §3 ①② 相關部分，其餘丟棄。

---

## 7. 接手時的最短路徑

1. `git push origin feat/amr-lot-identity-d4`（三個 commit 未推）
2. 讀本文 §2（裁定，不要再問）＋ §3（剩下要做的兩件）
3. 實作 §3 ①（InputEnd 觸發）與 ②（三個 payload SVID 的保留＋274 釋放閂）
4. 更新文件（D10：38225-38227 / 38246-38248 數字變小）並重出 `docs/SECS/deliver_YYYYMMDD/`
5. 上機前提：量測 `SnAutoX_InputEnd` 六顆的極性與穩定性（§3 ①的 ⚠）
