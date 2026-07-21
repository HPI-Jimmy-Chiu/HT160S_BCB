# Loader 多趟佇列 + 超量免停機 + 源乾等車窗自動 Clean Out + 自動 Lot End 導入計畫

- 日期：2026-07-21（同日第 3 版：Q1-Q4 全數定案）
- 分支：`feat/iosetview-172-refactor`
- 狀態：**計畫定稿待使用者最終核可**（尚未動任何程式碼；§9 四問已全數定案）
- 使用者決策軌跡（2026-07-21）：
  1. 超量盤：**Log 紀錄、照常 CCD 掃描、有 IC 照分選（物理決定命運）**，不彈警報停機。
  2. 源乾 → **選項 B（多車一 lot）**：先等既有 `iAmrFeedWaitSec` 等車窗，窗內 host 送新 START_AGV 就續產，窗過無車才自動 Clean Out。
  3. 前後端一起做，先出計畫文件確認。
  4. **兩車重疊供料為真實需求**（曾發生：車1剛放 5 work 盤，車2馬上放 3 work 盤；期望序列＝5掃吸+1上蓋+1身分+3掃吸+1上蓋+1身分）→ 單一計數器（iCarTrayTotal/iFeedSerial）會被第二車的 CEID274 重置而分類全錯 → **併回 7/13 計畫的 TripQueue 多趟 FIFO**。
  5. 契約確認：**每台車各自帶 SECS 握手（START_AGV + LoaderTrayCount，work-only 盤數）**；`放N盤`＝N work 盤，上蓋/身分由 firmware header 補（實體＝N+header）。
- 本計畫併入並取代／延伸兩份既有計畫：
  - [loader-multitrip-and-color-identity2d-plan-20260713.md](loader-multitrip-and-color-identity2d-plan-20260713.md) 設計 A（TripQueue）＋§A.5（超量處理）＋審查 F1-F6 —— **本文件為新版本源**；該計畫設計 B（Color 讀身分盤）不在本案範圍、另案不動。
  - [cleanout-amr-unload-lotend-plan-20260716.md](cleanout-amr-unload-lotend-plan-20260716.md) D1-D6（Clean Out 出料 + 自動 Lot End）——**原樣併入**（Part 3），並補一項該計畫刻意延後的進料側閘（Part 2 §2.3）。
- 佐證：現況行為由 workflow `wf_c25796de-96b`（5 agent，2026-07-21）逐行核實；兩車重疊缺陷由本日實碼複查確認（`aLoader.cpp:588` iFeedSerial 歸零於 CEID274、`aLoader.cpp:557-561` InfeedFinished=Inputend ON）。

---

## 0. 問題陳述（現況，程式碼實證）

**P0-1 Normal 模式超量＝每盤一次全機停機。** `DoFeedTray` case 9000（aLoader.cpp:1532）`(iCarTrayTotal-iFeedSerial)<=0 && Inputend ON` → `MES0921` modal（`DecStopAllMotor`+`SystemStart=false`，note.cpp:789-815）。**無 per-episode latch**，K_RETRY 重餵下一盤再炸。KYEC 22 連炸即此。

**P0-2 Clean Out 模式＝永久靜默卡死。** 同條件走 `break`（aLoader.cpp:1540）→ FeedTask 卡 9000；收尾守衛 `IsSupplyCarDry()`（aLoader.cpp:662，Inputend **且** InputHasTray 都 OFF）永不成立 → 該側永不 retire → 全機 Clean Out 收不了尾，僅 STUCK watchdog 默默記 log（cStateRecordHT160.cpp:194-222），無人通知。＝「停住且沒辦法排料」。

**P0-3 超量盤誤標身分盤。** `GetFedTrayKind` 的 `serial > total - idCount → Identity`（aLoader.cpp:624）對超過總數的盤一律回 Identity → 空盤送 Color 讀 2D → 無標籤 → ColorCCD_2D 警報＋空白 ID 污染身分池。

**P0-4 兩車重疊＝分類全錯（「丟趟」缺陷）。** 車籍是單一 scalar：第二車 START_AGV 進來，`InfeedFinished`＝`Inputend.IsOn()`（aLoader.cpp:557-561）——**車1的盤還在時恆真** → CEID274 幾乎立即發 → `RefillSimInfeed` 覆寫 `iCarTrayTotal`、`iFeedSerial=0`（aLoader.cpp:588）→ 車1剩餘的上蓋/身分盤用車2的邊界重算 → 全錯。用戶例（車1=5work、車2=3work）：車1的上蓋/身分被標 Normal、車2的 work 有一張被標 Identity 送 Color。

**P0-5 根因（無 host 情境）。** `iSecsCarTrayCount=0` 時 `iCarTrayTotal` 退回 `iSimAmrMaxTray`（恆>0），檢查點分不出「host 沒給數」與「host 給了數」。

---

## 1. 設計哲學

- **count 只用於分類**（work/cover/identity 邊界），不用於停機。
- **每趟一筆帳（TripQueue FIFO）**：多車重疊各算各的邊界，不互相覆寫。
- **超量不擋、照餵、照掃**：CCD/分選/排出閘本就不看 Kind（`PrepareTrayMap` 全格 UNCHECK_IC、case3000 全格 EMPTY_IC 才排出）→ 有 IC 照分選、混料天然免疫。
- **不報警不等於不留痕**：EventLog + 專屬檔 + fire-once。
- **源乾＋等車窗逾時＝該 lot 結束** → 自動 Clean Out → 自動 Lot End。

與 9045 對照：9045 為 now/latest **雙槽**批次模型（memory `ht9045-loader-amr-traykind-model`），TripQueue 是其一般化；非 ART 9045 無超量警報（MES0921 是 160 自創）；源乾自動 CleanOut 在 9045 是 config opt-in。

---

## Part 1 — 前端：TripQueue 多趟分類 + 超量免停機

### 1.0 資料結構（7/13 §A.1，定案沿用）

```cpp
// aLoader.h
struct TTripEntry { int iTotal; int iServed; };  // iTotal = work + header(cover+identity)
TList *TripQueue;        // FIFO；隊首 = 消耗中的趟
bool   bTripSeen;        // 本生產週期內曾有任何趟入列（區分 host 靜默 vs 趟吃完）
bool   bOverTrayLogged;  // 超量 EventLog 每 episode 一次 latch
```

- `TList` 動態無上限（7/13 A-Q1 已核實安全：佇列只裝「已到、未吃完」的趟，受料倉實體容量限制，不隨 lot 總量長大）。
- 淘汰 `iCarTrayTotal`/`iFeedSerial` 的分類角色（scalar 保留與否見 §1.6 觸點表）。

### 1.1 入列 = CEID274（InfeedRefill(0)）

`uAgvStation.cpp:84-85` 的 `SetExpectedCarTrayCount + RefillSimInfeed` 換成：

```cpp
// n = AgvCoord.TrayCount[0]（該車 S2F41 LoaderTrayCount，work-only）
LoaderModule->EnqueueTrip(n);
AgvCoord.TrayCount[0] = 0;    // consume-once：修 stale 沿用 bug（審查 S4）
```

```cpp
void TLoaderModule::EnqueueTrip(int nWork)
{
    if(nWork <= 0)
    {
        // 契約違規：host 送 START_AGV 未帶 LoaderTrayCount →【Q2 定案 2026-07-21】
        // 不入列 + EventLog 警示；該車盤走超量 Cover 路徑（見 1.2）
        g_EventLog.Log("WRN_TRIP_NOCOUNT", "START_AGV without LoaderTrayCount", "");
        return;
    }
    int iHeader = GeneralSetting.iAmrCoverTray[0]
                + ((GeneralSetting.iAmrIdentityTray[0]>0) ? GeneralSetting.iAmrIdentityTray[0] : 0);
    TTripEntry *e = new TTripEntry;
    e->iTotal  = nWork + iHeader;      // 與現行 RefillSimInfeed:579-583 同數學
    e->iServed = 0;
    TripQueue->Add(e);
    bTripSeen = true;
    bOverTrayLogged = false;           // 新趟到 → 超量 episode 重置
    iSimInfeedCount += e->iTotal;      // sim 料倉庫存改累加（原為覆寫；F4）
}
```

**入列時機採 CEID274（維持現行 hook）而非 START_AGV 收訊時**——分類正確性由 AMR 進料鎖保證：`BeginPrep` 即 `InfeedSetLock(0,true)` → `bAmrLocked` 擋新 destack（aLoader.cpp:1234），解鎖與入列在同一 handler、同 VCL 主執行緒內原子完成（uAgvStation.cpp:414-415）→ **鎖住期間零鑄籍，入列前後不會有盤被錯趟消費**。

### 1.2 鑄籍分類（case 9500 mint，取代 GetFedTrayKind 的 scalar 呼叫）

```cpp
eTrayKind kFed;
if(GeneralSetting.bUseAMR==false)
    kFed = eTrayKindNormal;                       // 既有：手動生產全 Normal
else if(TripQueue->Count > 0)
{
    TTripEntry *h = (TTripEntry*)TripQueue->Items[0];
    h->iServed++;
    kFed = GetFedTrayKind(h->iServed, h->iTotal); // 函式內部不變，鍵於隊首趟
    if(h->iServed >= h->iTotal) { delete h; TripQueue->Delete(0); }  // 趟吃完 → 出列
}
else if(bTripSeen)                                // 趟全吃完但盤還在 = 超量
{
    kFed = eTrayKindCover;                        // 標上蓋：照掃、有IC照分選、空殼回收 Empty
    LogOverTray(...);                             // 專屬 CSV 每盤一列（鑄籍天然每盤一次，無洗版）
    if(!bOverTrayLogged) { g_EventLog.Log("INF_OVERTRAY", msg, ""); bOverTrayLogged = true; }  // F6
}
else
    kFed = eTrayKindNormal;                       // host 靜默（從未宣告）= 全 Normal（解 P0-5/KYEC）
```

- 超量標 **Cover 而非 Identity**（解 P0-3）：Cover 路徑＝TrayArm 送 Empty 池（aTrayArm.cpp:1004 Cover 不進 Auto 分流），不去 Color 讀 2D。
- 超量標 Cover 而非「空盤」：**保留 CCD 掃描**（Kind 不閘掃描）→ 有真 IC 照分選，物理決定（決策 #1；也優於 7/13 A.5 字面的 RecycleExtraToEmpty 若其含義為跳掃）。
- host 靜默分支＝KYEC 手動驗證情境：全 Normal、零 MES0921。

### 1.3 case 9000 超量檢查點改寫（拆 MES0921 + 拆 CleanOut break）

```cpp
// 原 MES0921 分支（aLoader.cpp:1532-1552）整段改為：
// 超量（佇列空 + 曾有趟 + Inputend ON）→ 不報警、不 break、不改流 →
// 直落 present-branch 正常鑄籍（分類與 log 都在 1.2 的 mint 做）
// → Run_CleanOut 的 break 一併移除：CleanOut 下超量盤照餵，
//   餵到 Inputend OFF 後 IsSupplyCarDry() 自然成立、該側 retire（解 P0-2）
```

CleanOut 的 drain 語意（「餵到源乾再收尾」aLoader.cpp:1190）本就寫好，移除 break 後直接可達。

### 1.4 生命週期（keep/wipe；7/13 A-Q3）

| 時機 | 動作 |
|---|---|
| `InitialFlag(bKeepMaterial=false)`（冷啟 / CleanOut 完成後 InitialAllTask） | 清空 TripQueue（逐 entry free）＋`bTripSeen=false`＋`bOverTrayLogged=false`；**佇列非空時先記 log `WRN_TRIP_UNDELIVERED`（趟數+剩餘盤數）**——host 多報/少送的稽核痕跡 |
| `InitialFlag(bKeepMaterial=true)`（HOME 續產） | **整個 TripQueue + 兩旗標原樣保留**（車上實體盤不因 HOME 消失）；HOME-RESUME ledger log 行（aLoader.cpp:107-109）改 dump 佇列深度+隊首 served/total |
| 新趟入列 | `bOverTrayLogged=false`（episode 重置，見 1.1） |

### 1.5 專屬 log 檔（沿用 7/13 A-Q2 規格）

`cCsvDailyLog` 月夾：`D:\HT160S_Log\OverTrayRecycle\<YYYY_MM>\OverTrayRecycle_<YYYY_MM_DD>.csv`；欄位 `Date,Time,Station,TrayKind,TrayID_2D,Reason,Destination`；`SetRetentionDays(n)` 防膨脹。

### 1.6 觸點與 7/13 審查 F1-F6 對應

| 項 | 處理 |
|---|---|
| F1 空佇列 fallback | §1.2 mint 判空：`bTripSeen ? Cover : Normal`，不取 `Items[0]`、不 seed 假趟（AMR=0 由 `bUseAMR==false` early-out）|
| F2 CleanOut 排序 | Part 2 §2.1：源乾觸發掛在 `iAmrFeedWaitSec` 等車窗逾時後（選項 B 本身即 F2）|
| F3 DescribeState | `aLoader.cpp:2391-2397` 改 dump：佇列深度、隊首 iServed/iTotal、bTripSeen、bOverTrayLogged、每 entry 摘要（FeederDecision.txt hang 分析欄位不失血）|
| F4 iSimInfeedCount | 入列時 `+=`（§1.1）；冷啟 InitialFlag 保留既有 seed（sim 可跑）。**實作時必查**：sim 消耗端遞減位置與 `GetCarTrayCount`（Motion View work 數）讀值來源，改為佇列導出，不得留讀已淘汰 scalar 的殘讀 |
| F6 fire-once | EventLog 每 episode 一次（bOverTrayLogged）；CSV 每盤一列（掛鑄籍點，天然一次）|
| 舊 scalar | `iCarTrayTotal`/`iFeedSerial` 從分類/檢查退役；`iSecsCarTrayCount`/`SetExpectedCarTrayCount` 退役或降為 dump 用「最後宣告值」——實作時以全庫 grep 清點消費者逐一遷移，**編譯期殘讀＝硬錯誤優先** |

---

## Part 2 — 觸發：源乾等車窗 → 自動 Clean Out ＋ 進料側閘

### 2.1 源乾 → 等車窗 → 自動 Clean Out（選項 B；aLoader.cpp case 9000 else-branch 1598-1621）

保留既有 `FeedWaitTimer`（`iAmrFeedWaitSec`）等車窗，只換逾時後的動作：

```cpp
if(FeedWaitTimer 逾時)
{
    if(GeneralSetting.bUseAMR && HSys.Sys.RunMode==Run_Normal
       && IsSupplyCarDry()                       // Inputend OFF && InputHasTray OFF
       && AgvCoord.Handshake[P1]==AGV_IDLE)      // 無在途進料握手（PREP/READY 時不觸發，見下）
    {
        RecordProcess("AUTO CleanOut: Loader source dry, car-window expired");
        HSys.Sys.bCleanOut=true;
        HSys.Sys.RunMode=Run_CleanOut;           // 沿用 MES0921 K_CLEAN_OUT 分支既有 idiom
    }
    else if(GeneralSetting.bUseAMR==false)
        ShowMyError("MES0920", ...);             // 非 AMR 維持現行（D6）
    // AMR 且 handshake 在途（PREP/READY）→ 不觸發、timer 重臂：
    // 車在放盤中，交給 iAmrHandshakeWaitSec 既有看門狗兜底
}
```

- **等車窗＝多車安全的關鍵**：趟間隙在窗內被新車補上續產；窗過無車才判 lot 結束。
- **`Handshake[P1] != IDLE` 時不觸發**（新增條件）：窗逾時瞬間若 host 剛好派車（PREP/READY、AMR 正在放盤），不得在放盤中途切 CleanOut；PREP/READY 卡死由既有 `iAmrHandshakeWaitSec` 看門狗兜底。CALLED 掛著（機台要過車、host 未回）＝窗就是它的等待上限 → 觸發並依 §2.3 取消 CALLED。
- 兩側各有 FeedWaitTimer：先逾時者觸發，`RunMode==Run_Normal` 前提使其冪等。

### 2.2 無 host 降級

host 靜默：§1.2 全標 Normal、超量分支不觸發（bTripSeen=false）→ 餵完 → 等車窗（無 host 必逾時）→ §2.1 自動 Clean Out。**KYEC 手動驗證＝零 MES0921/MES0920、多等一個車窗後自動收尾。**

### 2.3 進料側 Clean Out 閘（補 7/16 D5 缺口——選項 B 下必做）

「等車窗＋自動 CleanOut」把 mid-CleanOut 補料 gap（memory `cleanout-amr-intake-gap`，7/14 決議暫不修）從邊角變主路徑，必須補：

1. **進 Clean Out 時取消進料側 pending CALLED**：P1-P3 `Handshake==AGV_CALLED → AGV_IDLE` + `InfeedSetLock(p,false)`。
2. **`BeginPrep` 加 RunMode 閘**：Run_CleanOut 下收到 P1-P3 的 START_AGV → 回 HCACK=4（busy）不進 PREP；**出料側 P4-P9 不受影響**（D1/D4 照走）。`ServiceHandshake` P1-P3 段在 Run_CleanOut 不推進。
3. 效果：Clean Out 期間 host 送進料車＝明確拒收（host 看得到 HCACK=4）；已實體到站的車人工處理（多車情境已由等車窗大幅降低發生率）。

---

## Part 3 — 後端：Clean Out 完成 → CEID28 → 自動 Lot End

**原樣採用 [cleanout-amr-unload-lotend-plan-20260716.md](cleanout-amr-unload-lotend-plan-20260716.md) D1-D6 與 §4 全部變更點**（`IsStationCleanOutUnloadDue`、CALLED 計時、`iCleanOutAmrWaitSec` ini、超時模態 RETRY/SKIP、link-down fallback、`DoLotEndProcess()` 抽出、csystem 完成分支改造、警報碼註冊、docs 同步），細節以該文件為準。接點摘要：

| 決策 | 本計畫接點 |
|---|---|
| D1/D4 出料呼叫 272→273→274 per-Auto | Part 2 §2.3 的閘只擋進料側 P1-P3，出料側照走 |
| D2 握手超時模態 + `[AGV] CleanOutAmrWaitSec`（300s/下限5）| 出料用；進料等車窗用既有 `iAmrFeedWaitSec`，語意分工清楚 |
| D3 CEID28→CEID12 順序、28 AMR/非AMR 都發 | CEID28 定義已在（uHGemHT160.h:38）從未發 |
| D5 原案僅出料側 | 本計畫 §2.3 補進料側閘 |
| D6 全限 `bUseAMR`；非 AMR 照舊 | §2.1 同 gate |

---

## 4. 完整資料流（含兩車驗收案例）

```
AMR 多車一 lot：
  車1 握手（START_AGV+LoaderTrayCount=5）→ CEID274 → enqueue{total=7}
  車2 馬上到（LoaderTrayCount=3）→ 握手 → CEID274 → enqueue{total=5}   佇列=[7,5]
  消耗（由下而上）：
    trip1: serial 1-5 → Normal×5（掃吸）→ 6 → Cover（上蓋）→ 7 → Identity（身分→Color讀2D→CEID275）
           trip1 吃完 → Delete(0)
    trip2: serial 1-3 → Normal×3（掃吸）→ 4 → Cover → 5 → Identity
  = 5掃吸+1上蓋+1身分+3掃吸+1上蓋+1身分 ✅（使用者期望序列，驗收基準）
  → 源乾 → 等車窗（無新車）→ 自動 Clean Out → 餵完殘料 → 各側 retire
  → CEID28 → 出料側 D4 叫 AMR 收車 → 六站車空
  → 自動 Lot End（CEID12+清工單+清分類綁定）→ Run_Normal + SoftStop

超量（host 宣告 N、實體 > N+header）：
  佇列吃完後盤還在 → 標 Cover、照掃、有IC照分選、空殼回收 Empty
  EventLog INF_OVERTRAY（每 episode 一次）+ 專屬 CSV（每盤一列）→ 不停機

host 靜默（KYEC 手動驗證）：
  全 Normal、零警報 → 餵完 → 等車窗逾時 → 自動 Clean Out → …同上
```

---

## 5. 風險與對策

| # | 風險 | 對策 | 殘餘 |
|---|---|---|---|
| S1 混料 | 超量工作盤載真 IC | CCD/分選/排出閘不看 Kind，物理免疫（現況即成立，不動）| 無 |
| S2 假亮 Inputend | sensor 卡 ON | §2.1 用 `IsSupplyCarDry()` 雙 sensor；destack 缸自帶逾時警報 | 硬體故障走 iosetview（既有）|
| S3 count 準確性承重 | host 報錯數 → 邊界錯（如車1少報→其身分盤被推遲/錯標）| 制度面：count 模型本質信任 host（9045 同弱點）；§1.4 UNDELIVERED/OVERTRAY log 留稽核痕跡 | host 報錯數時該車 cover/identity 辨識錯（已知代價，有 log）|
| S4 stale TrayCount[0] | 下一車未帶數沿用上一車 | §1.1 consume-once 清零 + WRN_TRIP_NOCOUNT | 無 |
| S5 補料競態 | mid-CleanOut refill 重啟進料 | §2.3 進料閘 + 取消 CALLED + §2.1 handshake-在途不觸發 | 已到站實體車人工 |
| S6 提早結批 | 多車 lot 誤判結束 | 選項 B 等車窗；PREP/READY 在途不觸發 | host 派車慢於 `iAmrFeedWaitSec` 仍會結批（窗長可調）|
| S7 靜默遮蔽 | 拿掉警報後 count bug 隱形 | INF_OVERTRAY/WRN_TRIP_NOCOUNT/WRN_TRIP_UNDELIVERED 三 log + 專屬檔 | 非阻塞；host 端另可稽核 |
| S8 報表 | 提早結帳分裂 Soter | Part 3 `DoLotEndProcess` 一次關帳（7/16 R1 對策）| 攻防複驗必打 |
| S9 Identity 洪水 | serial>total→Identity | §1.2 超量標 Cover，結構性消除 | 無 |
| **N1 提前 274** | 兩車重疊時 `InfeedFinished`=Inputend ON 立即真 → 274 即發＋destack 解鎖（AMR 可能還在放盤） | 【Q1 定案】對齊 9045＝**接受即時 274**（9045 同為 level 讀值、無 settle、KYEC 已實跑 2 車）；分類由 TripQueue 承擔；補 `ReadyEntrySensor[]` instant-finish log 註記 | 頂放/底抽並行物理安全＝9045 既有假設，上機列觀察項 |
| N2 幽靈趟 | START_AGV 後車實體未到（AMR 故障）| 入列在 274 而非 S2F41 收訊；殘餘由 §1.4 UNDELIVERED log 收口 | 即時 274 弱化「物理確認」語意（兩車情境），instant-finish 註記留稽核痕跡 |

---

## 6. 變更檔案清單（皆在 `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\`）

| 檔案 | Part | 內容 |
|---|---|---|
| `aLoader.h` | 1 | `TTripEntry`/`TripQueue`/`bTripSeen`/`bOverTrayLogged`/`EnqueueTrip`/`LogOverTray` 宣告；舊 scalar 退役標注 |
| `aLoader.cpp` | 1,2 | EnqueueTrip 本體、mint 分類改寫（1.2）、case 9000 改寫（1.3）、InitialFlag keep/wipe（1.4）、DescribeState（F3）、HOME ledger 行、源乾等車窗→CleanOut（2.1）、RefillSimInfeed 退役/瘦身、OverTrayRecycle log |
| `SecsGem\uAgvStation.cpp` `.h` | 1,2,3 | InfeedRefill→EnqueueTrip + TrayCount[0] consume-once（1.1）；進料側 CleanOut 閘 + 取消 CALLED（2.3）；出料側 D1-D4（7/16）|
| `SecsGem\uHGemHT160.cpp` | 2,3 | BeginPrep 呼叫端 HCACK=4（2.3）；CEID28（7/16）|
| `aAuto1To6.cpp` `.h` | 3 | `IsStationCleanOutUnloadDue` + AMR 車空閘（7/16 §4B）|
| `main.cpp` `.h` | 3 | 抽 `DoLotEndProcess()`（7/16 §4C）|
| `csystem.cpp` | 3 | 完成分支改造（7/16 §4D）|
| `GeneralSetting.cpp` `.h` | 3 | `iCleanOutAmrWaitSec`（7/16 §4E）|
| `database.cpp` | 1,3 | **MES0921 自 mapAlarmCodeList 移除（Q3 定案；AlarmList.csv/S5F1 catalog 隨 SSOT 跟進）**；Part 3 超時模態碼註冊；INF_OVERTRAY/WRN_TRIP_* 走純 EventLog 不進 alarm SSOT |
| `docs/SECS_*.md` | 3 | CEID28 + 序列（7/16 §4F）|
| **測試工具**：`D:\AI_Area\Tool\HT160S_SECS_Simulator` | 驗證 | **Auto-AGV 自動回覆補 LoaderTrayCount**（可設定，Loader 站自動帶數）——V 系列情境的前置；另加「連送兩車」腳本 |

---

## 7. 建置 / 驗證 Gate

1. Build gate：刪改動 .obj → `build-ht160s.ps1 -Clean` EXIT 0；動 `SOFT_SIMULATE` 分支 → 真機組態驗證（註解 define → `-Full` EXIT 0 → 還原重建）。
2. 編碼檢查：`check-ht160s-source-encoding.ps1` 通過。
3. `--selftest-home` 迴歸 EXIT 0。
4. SECS simulator 情境（Automation+Auto-AGV ON，含新 traycount 增強）：
   - V1 單車 N=實體：feed→dry→窗逾時→CleanOut→28→12，零警報。
   - **V1b 兩車重疊（驗收案例）**：車1宣告5、車2馬上宣告3 → 分類序列 = 5N+1C+1I+3N+1C+1I（逐盤比對 EventLog/StateRecord tag）；車窗內續產不結批。
   - V2 超量 N+k：k 張 Cover、INF_OVERTRAY 一筆+CSV k 列、有 IC 照分選、dry→窗→CleanOut→LotEnd。
   - V3 host 靜默：全 Normal、零警報、餵完→窗逾時→CleanOut。（KYEC 情境）
   - V4 CleanOut 中送進料 START_AGV → HCACK=4、不重啟進料。
   - V4b 窗逾時瞬間 PREP/READY 在途 → 不觸發 CleanOut、放完盤續產。
   - V5 非 AMR：MES0920/彈窗/不自動 LotEnd 照舊（D6 不回歸）。
   - V6 出料側 D1-D4 全握手（7/16 S1-S5）。
   - V7 未帶 LoaderTrayCount 的 START_AGV → WRN_TRIP_NOCOUNT + 該車依 Q2 定案行為。
5. 攻防複驗：重點打——mint 分類改寫的每條路（佇列空/滿/吃完瞬間、keep-material HOME 前後）、case 9000 改寫後控制流、§2.1 觸發條件矩陣（dry×窗×handshake 狀態）、§2.3 閘每條 handshake 殘留、DoLotEndProcess 無漂移、Soter 不重複關帳、舊 scalar 殘讀。
6. 上機驗證（使用者）：真機 + 模擬器 host 全流程（confirm-compile 慣例）。

---

## 8. 施工順序（可獨立驗證）

- [x] S0 計畫核可（Q1-Q4 定案）— 使用者「動工」2026-07-21
- [x] S1 模擬器增強（Auto-AGV 帶 LoaderTrayCount + `twocar` 腳本 + GUI/CLI）→ scenario_runner 67/67 PASS
- [x] S2 aLoader：TripQueue 結構 + EnqueueTrip + mint 分類改寫 + 生命週期 + DescribeState —— **SHIPPED `bd9f8f0`**（對抗式複驗過；dev+real build EXIT0；selftest PASS）
- [x] S3-core aLoader：case 9000 拆 MES0921 stop + 拆 CleanOut break + FlushTripsOnDry 源乾reconcile + INF_OVERTRAY —— **SHIPPED `bd9f8f0`**
- [x] S3-follow aLoader/database：MES0921 從 mapAlarmCodeList SSOT 移除（Q3，陣列 20→19）+ 專屬 OverTrayRecycle CSV（cCsvDailyLog 惰性 InitLog，§1.5）—— **SHIPPED**
- [x] S4 aLoader：源乾等車窗→自動 CleanOut（選項 B，§2.1；bAmrLocked==false + IsSupplyCarDry 閘，逾時取代 MES0920）—— **SHIPPED**
- [x] S5 uAgvStation：進料側 CleanOut 閘（PollAndCall 釋放 CALLED + ServiceHandshake P1-P3 early-return + BeginPrep 拒收 infeed）—— **SHIPPED**（HCACK 用「return true+不進 PREP」既有 operator-holding idiom，非 HCACK=4；host 靠 timeout；安全結果相同。ReadyEntrySensor instant-finish 註記留待 S6/上機需要時再補）
- [x] S6-enablers `DoLotEndProcess()` 抽出 + `[AGV]CleanOutAmrWaitSec` —— **SHIPPED `c52cc77`**
- [x] S6b 後端 core：CEID28 `EmitCleanOutOK` + AMR 自動 Lot End（csystem 完成分支呼叫 `DoLotEndProcess()`，非 AMR 維持彈窗）—— **CODE-COMPLETE**（調查證實 CleanOut finish 不靠 AMR 收車即可達成，見下）；dev+real build EXIT0、selftest PASS
- [ ] **D4（延後，非必要）** per-Auto 主動叫 AMR 收輸出車（CleanOut 期間 CEID272→273→274）+ `IsStationCleanOutUnloadDue` + `iCleanOutAmrWaitSec` 超時彈窗 + 車空 finish 閘。**調查結論：finish 現況即可達成（drain ladder case 7000 清所有 per-station flag + latch bCleanOutFinish；輸出車留盤不擋 finish）**，故 D4 純為「無人化自動取車」加值，非收尾必要條件——待使用者決定是否要做
- [x] S7 真機組態編譯驗證（每個 commit 都做：SOFT_SIMULATE off → Full → 還原，全 EXIT0）
- [ ] S8 SECS simulator 全情境 V1-V7（上機/連模擬器；含兩車 V1b）—— 待使用者
- [~] S9 攻防複驗：Part 1（TripQueue）已過（wf_48bf8a33）；Part 2+3 自動複驗撞 session usage limit（8pm 重置）→ 已用**主迴圈手動對抗式走查**代替（S4/S5/S6b 全risk點無缺陷，含 modal-in-control-loop 已排除）；建議 8pm 後重跑自動複驗獨立確認
- [ ] S10 docs/SECS comm-examples + 操作說明更新
- [ ] S11 上機驗證（使用者）
- [ ] **S12（最後才處理，獨立調查任務）** AMR 上料動作能否在機台 **halt / pause**（SystemStart=false / SoftStop）情況下執行？現況 `PollAndCall` gate `RunMode==Run_Normal`、`ServiceHandshake` gate HOME——但未查 halt/pause 下 SECS 1s timer 與進料握手是否仍運作。**對照 9045 是否允許**（9045 `CheckAMRAction` 由 Timer4 驅動，csystem.cpp:8591 `DoLoad()` 前有 `SystemStart==false → return` 閘——初步跡象 9045 生產動作也停，但 AMR 握手在 uLotInfo Timer4 可能獨立，須第一手查證）。使用者指示：**最後才處理**，本輪不查。

---

## 9. 開放問題與定案紀錄（2026-07-21 使用者答復）

**Q1｜兩車重疊的 CEID273/274 語意。【定案 2026-07-21：對齊 9045＝維持 level-read 即時 274，分類交給 TripQueue】**

考證來源（workflow `wf_3eb9bc1b` 4 讀者＋**9045 源碼第一手直讀** `D:\HT9045\HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP`）：

使用者陳述的協定理解**全部證實**：
| 使用者理解 | HT160S 佐證 | 9045 佐證（第一手） |
|---|---|---|
| START_AGV＝「即將放盤」預告，實體未放 | E87 Draft §8.1 泳道：AGV 抵達→START_AGV→機構鎖定→273→**AGV 開始上料**→274 | `LoaderAction` case 1→100（uLotInfo.cpp:13011-13031）：收令→等缸就位→273，放盤在 273 之後 |
| 握手中不得 GoDown（Loader/Empty/Color） | `BeginPrep`→`InfeedSetLock`→`bAmrLocked` 擋新 destack（aLoader.cpp:1234；Empty/Color 窄鎖同型） | case 100 設 `bLoaderLockActionFlag=true`（:13021）→ csystem.cpp:8600/acatchtray.cpp:912-915：**整個 `DoLoad()`/`DoAutoEmpty()`/`DoAutoColor()` 凍結**（比 160 更粗——整模組停） |
| 無動作＋已鎖 → 發 273「AGV 可動作」 | Ready 閘＝前堆疊三缸 out-bit 全 false（aLoader.cpp:534-543） | 273 閘＝`C_Load_Middle` 缸 OffStatus（:13019），同義「機構就位」 |
| Auto 握手中不得 GoUp | `SetAmrLock`→`FindDischargeAuto` 跳過鎖站＋`GetTrayRequest` 拒收（aAuto1To6.cpp:497/1245） | unload 側 station locked until pickup（memory 9045-code-cited） |

**274 完成判定——9045 與 160 結構相同，都是 level 讀值：**
- 9045：`Sen[SnLoaderTrayHasTray_AGV].IsOn()`（uLotInfo.cpp:13034，專用 `_AGV` sensor #555；同一顆 sensor OFF 也是 272 叫車條件 :13171）。300s 逾時分支**是空的**（:13048-13050 `//Error`）。
- 兩車重疊時 9045 **同樣即時發 274**（stock 在→sensor 已 ON→case 200 立即過），機構立即解凍；**9045 沒有 settle timer、沒有邊緣偵測、沒有第二完成訊號**。9045 對兩車的處理全在**計數側**：now/latest 兩槽覆寫＋rollover（acatchtray.cpp:5470-5474），且只能排 1 趟（Trip3 會蓋 Trip2）——**TripQueue 正是其一般化**。
- 9045 的 272 叫車閘（sensor OFF＋Loader 另有 count==0 閘，:13176）使正常流程從空倉開始；host 主動推第二車＝9045 既有接受的情境（KYEC 曾實跑 2 車）。

**定案修法：機台側不加 settle、不加新 IO、不改 host 契約**——現行 level-read 274 即為 9045 對齊行為；兩車分類正確性由 Part 1 TripQueue 承擔。附帶兩項：
1. **觀測增強（納入 Part 2 施工）**：`ReadyEntrySensor[]`（uAgvStation.h:70，已宣告未實作的「edge baseline for Finish」死欄位）在 PREP 進入時快照 Inputend 現值；274 時若快照＝ON（有料中收車＝即時 Finish），EventLog 註記 `instant-finish (stock present)`——兩車握手在 log 可辨識，純觀測不改行為。
2. **上機觀察項**：AMR 頂放與 destack 底抽並行的物理安全性是**繼承 9045 的既有假設**（9045 凍結窗在兩車情境同樣 ~0 且現場已實跑）；HT160S 上機驗證時列入觀察。

**Q2｜未帶 LoaderTrayCount 的 START_AGV。【定案】** 不入列＋`WRN_TRIP_NOCOUNT` 警示 log；該車盤走超量 Cover 路徑（照掃照分選、身分不辨識）。不採 fallback 總數（猜錯會把錯的盤送 Color 讀 2D 炸 modal）。

**Q3｜MES0921 退場。【定案】** 從 alarm SSOT（`mapAlarmCodeList`）**移除**，`AlarmList.csv`＋S5F1 catalog 隨 SSOT 自動跟進（alarm-registry 機制）。

**Q4｜實體堆疊順序。【定案，使用者附圖】** 車2 疊在車1 上方；每車內部由下而上＝IC盤×N → 上蓋 → 身分（身分在該車頂端）；分離夾爪在堆疊區底部（front），**由下往上消耗**：

```
車2  身分        ← 最頂
     上蓋
     IC盤 ×3
車1  身分
     上蓋
     IC盤 ×5    ← 車1 最底＝最先餵
──── 分離夾爪 ────
──── 堆疊區底部(front) ────
```

消耗序＝車1 IC×5 → 車1 上蓋 → 車1 身分 → 車2 IC×3 → 車2 上蓋 → 車2 身分。**FIFO 成立**；且「每車 identity 最後餵、cover 倒數第二」與 `GetFedTrayKind` 現行約定（aLoader.cpp:599-606）吻合，TripQueue 邊界數學不需改。

---

## 10. 待實作時確認的小項（不阻斷）

- `INF_OVERTRAY`/`WRN_TRIP_*` 走純 EventLog（不進 alarm SSOT——它們不是警報）。
- 自動進 CleanOut 是否補發 CEID9（host 可見性；其名義為 Press Clean Out button）——預設發，實作時對 host 需求。
- 超量「每車一次通知」目前只 log 不上螢幕（決策 #1）；要非阻塞螢幕提示再加。
- Part 3 超時模態碼實作時查 `mapAlarmCodeList` 避碼。
- F4 的 sim 消耗端遞減位置、`GetCarTrayCount`（Motion View）讀值來源——實作時全庫 grep 清點遷移。
