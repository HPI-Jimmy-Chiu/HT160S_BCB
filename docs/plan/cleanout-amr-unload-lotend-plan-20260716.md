# Clean Out(AMR 模式)強制下料 + 自動 Lot End 導入計畫

- 日期:2026-07-16
- 分支:`feat/iosetview-172-refactor`
- 狀態:**計畫定稿待使用者核可**(尚未動工)
- 範圍:僅出料側(Auto1-6 輸出車)。進料側 mid-CleanOut AMR 補料 gap 維持觀察不動
  (見 memory `cleanout-amr-intake-gap`,2026-07-14 決議)。

---

## 1. 背景

現行 Clean Out 完成(`CheckCleanOutFinish`,csystem.cpp:1476)在 AMR 模式下有三個洞:

1. 完成後彈 `ShowSystemError(SnFKCleanOut, K_SKIP)` 模態(csystem.cpp:1423),
   無人化流程會停在等人按鍵。
2. AMR 呼叫(CEID272)唯一觸發是滿倉 sensor(`IsOutputCarFullForAmr`),且
   `PollAndCall` 有 `RunMode!=Run_Normal return`(uAgvStation.cpp:237)——
   Clean Out 抽乾後車上通常是不滿的殘量,**永遠不會叫 AMR 來收**;
   而 `IsAllCleanOutFinish`(aAuto1To6.cpp:1068)只擋「滿」不擋「有」,
   車上留著盤也算完成,盤丟給人處理。
3. 完成後只做部分結帳(凍結面板+Soter 關檔),**不做 Lot End**:
   不發 SECS、不清工單、不清 (Lot,Bin)→Auto 分類綁定。

## 2. 使用者定案(2026-07-16)

| # | 決策 |
|---|------|
| D1 | 呼叫機制沿用現有 CEID272 → host `START_AGV` → CEID273/274 標準握手 |
| D2 | 握手超時 → **fallback 彈窗叫操作員**(不再無聲 force-release);超時秒數要可設定 |
| D3 | 自動 Lot End 發 CEID 12(可以);CEID 28 `CleanOutOK`(目前只定義從未發送)**這次一併補上** |
| D4 | 呼叫時序 per-Auto 獨立:哪站先抽乾先叫車 |
| D5 | 本案只動出料側 |
| D6 | 全部行為限定 AMR 模式(`GeneralSetting.bUseAMR`);非 AMR 維持現行為 |

## 3. 目標行為(AMR 模式)

```
Run_CleanOut 抽乾中
  ├─ 每個 Auto 站:State[i].bCleanOutFinish(drain ladder 跑完,站別 latch)
  │    └─ SnAutoX_InputEnd == ON(車上有盤)
  │         → 強制發 CEID272 AGVSupplement(+ 該站離散 Full CEID)→ AGV_CALLED
  │         → host START_AGV → PREP → drained → CEID273 Ready
  │         → AMR 收車(InputEnd 轉 OFF)→ CEID274 Finish → ClearAmrCar
  │         → 該站「車空」= 該 Auto 下料 finish
  │    └─ InputEnd == OFF(這站無產出)→ 直接算 finish
  ├─ 六站全數車空 + 其餘模組 cascade 照舊 → CheckCleanOutFinish == true
  ├─ 發 CEID 28 CleanOutOK
  ├─ 不彈 Clean Out Finish 模態
  ├─ 自動執行完整 Lot End(= 手動 btnLotEnd 全部語意,含 CEID 12、
  │    LotRegistry.Clear、LotBinBinding.Clear ← Auto1-6 分類類別清除)
  └─ 停在 Run_Normal + SoftStop,等新工單 + Lot Start 重新綁定
```

超時(等 host 派車 / 等 AMR 到位 / 等收車)→ 彈操作員模態:
- **RETRY** = 重發呼叫(回 CALLED 重走)
- **SKIP** = 操作員手動取車;取走後 InputEnd 轉 OFF,完成判定自然放行

關鍵巧合(零新增 IO):`SnAutoX_InputEnd` 現在就是 `IsAmrTaken` 的
「車被收走」sensor(aAuto1To6.cpp:1407,ON=有盤/OFF=被取走),
「InputEnd=ON 叫車、轉 OFF 才 finish」與既有握手收尾判定同一顆 sensor。

## 4. 變更點

### A. `SecsGem/uAgvStation.cpp` — Clean Out 下料呼叫(核心)

1. `PollAndCall` 的 `if(HSys.Sys.RunMode!=Run_Normal) return;`(:237)改為:
   - `Run_Normal`:維持現行(滿倉觸發 + P1-P3 缺料觸發)。
   - `Run_CleanOut`:**只跑 Auto 側(P4-P9)**,觸發條件換成 Clean Out 專屬:
     ```
     AutoModule->IsStationCleanOutUnloadDue(a)   // 新 helper,見 B
       && Handshake[si]==AGV_IDLE
       && IsOperatorHolding(a)==false
     → SetAmrLock(a,true) + CEID272 + AutoFullCeid[a] + AGV_CALLED
     ```
   - P1-P3(進料)在 Run_CleanOut 一律不觸發(D5:進料側不動)。
2. **CALLED 階段計時**(新):Run_CleanOut 期間,CALLED 狀態每秒 aging,
   超過 `iCleanOutAmrWaitSec` → 彈操作員模態(見 E)。
   Run_Normal 的 CALLED 維持現行(不計時,滿→不滿自動解除)。
3. `ServiceHandshake` PREP/READY watchdog(:364-373):Run_CleanOut 期間
   超時改為彈操作員模態(不再無聲 SetAmrLock(false)+IDLE);
   Run_Normal 維持現行無聲釋放。
4. 「車在 CALLED 期間被人先取走」:InputEnd 轉 OFF → 比照現行
   `bFull==false && CALLED` 分支釋放(SetAmrLock false + IDLE),
   該站憑 InputEnd=OFF 自然 finish。

### B. `aAuto1To6.cpp/.h` — 站別判定

1. 新 helper `bool TAutoModule::IsStationCleanOutUnloadDue(int Index)`:
   ```
   State[Index].bCleanOutFinish == true          // 站別 drain latch(case 7000)
   && SortArmModule->IsCleanOutFinish() == true  // 上游保險(便宜,防早叫)
   && 車上有盤:
        real : SnAutoX_InputEnd.Enable && IsOn()
        sim  : Car[Index].iTrayCount > 0
   ```
2. `IsAllCleanOutFinish`(:1068)在 `bUseAMR==true` 時,每站**加一道車空閘**:
   ```
   real : SnAutoX_InputEnd.Enable && IsOn() → return false  // 車上還有盤,下料未完
   sim  : Car[Index].iTrayCount > 0        → return false
   ```
   非 AMR(bUseAMR==false)不加此閘 → 維持現行「盤上車即完成、車留人處理」。
   (現有的 `IsOutputCarFullForAmr` Full 閘保留不動,車空閘是它的嚴格超集,
   但保留可讀性與非 AMR 行為。)

### C. `main.cpp/.h` — 抽共用 Lot End

1. 把 `btnLotEndClick`(main.cpp:2474-2528)本體抽成
   `void __fastcall TfMain::DoLotEndProcess()`(public 一般方法,非 __published;
   form class body 不加註解,註解放實作檔)。
   `btnLotEndClick` 變成薄殼呼叫它。行為 byte-for-byte 等價,無語意變更。
2. 防重複關帳:`DoLotEndProcess` 進入時若 `LotRegistry.GetLotCount()==0`
   且 LotNo 已空,仍可安全全跑(CEID12 本來就 gate 在 count>0,
   Clear 空表無害)——不需另加 latch,天然冪等。

### D. `csystem.cpp` — Clean Out 完成分支改造(:1407-1435)

```cpp
if(CheckCleanOutFinish())
{
    EventReport(SECS_EVENT.CleanOutOK);        // D3:CEID28,AMR/非AMR都發(自 gate SECS)
    InitialAllTask();
    HSys.Sys.bCleanOut=false;
    if(GeneralSetting.bUseAMR)
    {
        // AMR:不彈窗,直接完整 Lot End(內含 LotEndTime/UPH/Freeze/Soter/
        // TrayUphLog/lastdata/CEID12/歸檔/清工單/清分類綁定)
        if(fMain!=NULL) fMain->DoLotEndProcess();
    }
    else
    {
        // 非 AMR:維持現行部分結帳 + 彈窗
        tRunData.LotEndTime=Now();
        tRunData.UPH=GetCalculateUPH(tRunData.LotEndTime);
        if(fMain!=NULL) fMain->FreezeProductInfoAtLotEnd();
        g_SoterOutput.OnLotEnd();
        int retCleanOut=ShowSystemError(HSys.Sen.SnFKCleanOut.Name, K_SKIP, 0);
    }
    ChangeRunMode(Run_Normal);
    SoftStop=true;
}
```
注意:AMR 分支**不再**先做部分結帳(LotEndTime/Freeze/Soter),
統一由 `DoLotEndProcess` 一次做,避免 Soter 關檔/面板凍結跑兩次。
CEID28 在 Lot End 之前發(host 先看到 Clean Out Finish 再看到 Lot End)。

### E. 超時 fallback 模態 + 設定

1. `GeneralSetting.cpp/.h`:新欄位 `iCleanOutAmrWaitSec`,
   `[AGV] CleanOutAmrWaitSec`,**預設 300 秒、下限 5**(比照現有 240 的寫法:
   建構子預設 + ReadInteger + clamp + WriteInteger)。
   既有 `AmrHandshakeWaitSec`(240)不動,仍管 Run_Normal 的 PREP/READY。
2. 模態走 `ShowMyError(穩定碼, 訊息, K_RETRY|K_SKIP)`:
   - 穩定碼:**每站一碼**,建議沿用 AMR/Full 既有段位風格,暫定
     `AMR0101..AMR0601`(Auto1-6 Clean Out 下料超時)——實作時先查
     `mapAlarmCodeList` 避碼,並照 alarm-registry SSOT 註冊(CSV+S5F1 自動跟進)。
   - 訊息(雙語):「AutoX Clean Out 下料等待 AMR 超時,請確認 AMR/host 派車,
     或手動取走輸出車後按 SKIP」。
   - RETRY → 該站回 AGV_CALLED 重發 272;SKIP → 釋放 lock 回 IDLE,
     等操作員取車(InputEnd OFF 後自然 finish)。
3. 邊界:**SECS link down**(`IsSelected()==false`)時 `PollAndCall` 走
   link-down 分支直接 return,叫不了車也計不了時 → 在該分支補:
   Run_CleanOut 且某站 `IsStationCleanOutUnloadDue` 為真 → 直接彈同一模態
   (等於立即 fallback 操作員,不空等)。彈過要 latch 防重複彈(per-station
   bool,站 finish 或 RETRY 時清)。

### F. 文件同步

- `docs/SECS_*.md` comm-examples:補 CEID28 CleanOutOK 發送時機 +
  Clean Out 下料 272/273/274 序列一節(照 memory `secs-comm-examples-doc-maintenance`
  規則:.md 真源、python-markdown 重生 .html、不偽造 log)。
- 操作手冊 Clean Out 一節待上機驗證後再更新。

## 5. 不動項(明確排除)

- 進料側(P1-P3)在 Clean Out 的 REQUEST/ACCEPT/FEED 行為全部不動(D5)。
  已知 gap(mid-CleanOut host 補料會重啟進料+卡 finish)維持 2026-07-14 決議:待上機確認 host 時序。
- 非 AMR 模式:彈窗、部分結帳、不自動 Lot End —— 全部照舊。
- `Run_Normal` 的滿倉呼叫、watchdog 無聲釋放 —— 照舊。
- Empty/Color 的 Clean Out 完成判定(GoUp 回前供料車)—— 照舊,不叫 AMR。

## 6. 風險與對策

| # | 風險 | 對策 |
|---|------|------|
| R1 | Soter/Freeze 重複關帳(CleanOut 分支與 DoLotEndProcess 都做) | AMR 分支移除部分結帳,只呼叫共用函式(§4D) |
| R2 | 抽 DoLotEndProcess 時行為漂移 | 純搬移不改寫;diff 逐行對照原 btnLotEndClick |
| R3 | CALLED 計時誤傷 Run_Normal 握手 | 計時邏輯以 `RunMode==Run_CleanOut` 為前提條件 |
| R4 | 模態在機控迴圈彈出(pump/z-order) | 沿用既有 ShowSystemError/ShowMyError 於 csystem/uAgvStation 的呼叫模式;比照 note-modal pump 慣例 |
| R5 | 站別早叫:drain latch 設了但 SortArm 還會放 IC | drain ladder 入口本就 gate SortArm 完成;helper 再疊 `SortArmModule->IsCleanOutFinish()` 雙保險 |
| R6 | sim 無 sensor(InType=0 恆 present) | helper 與車空閘都走 sim early-out(`iTrayCount`),比照 `IsOutputCarFullForAmr` 模式 |
| R7 | link down 空等死鎖 | §4E-3 立即 fallback 模態 + 防重彈 latch |
| R8 | Big5 原始碼被 Edit 弄壞 | 依 memory:BCB6 legacy 檔案用既有安全編輯流程;新註解 ASCII English only |

## 7. 驗證計畫

1. **Build gate**:改動後刪 .obj → `scripts/ops/build-ht160s.ps1 -Clean` EXIT 0;
   本案動到 `#ifdef SOFT_SIMULATE` 分支(sim/real InputEnd 判定)→
   **真機組態也要驗**:註解 `SOFT_SIMULATE` → `-Full` EXIT 0 → 還原重建。
2. **編碼檢查**:`scripts/ops/check-ht160s-source-encoding.ps1` 通過。
3. **`--selftest-home`** 迴歸 EXIT 0(InitialAllTask/HOME 路徑有共用)。
4. **SECS simulator 情境**(D:\AI_Area\Tool\HT160S_SECS_Simulator):
   - S1:CleanOut → 站 drain 完 + 車有盤 → 收到 272(+離散 CEID)→
     host 回 START_AGV → 273 → 模擬取車 → 274 → 六站完 → **28 → 12** 順序正確,
     工單/綁定清空,無彈窗。
   - S2:host 不回 START_AGV → `CleanOutAmrWaitSec` 到 → 彈窗;RETRY 重發 272;
     SKIP 後模擬取車 → finish 放行。
   - S3:某站車空(無產出)→ 不發 272,直接 finish。
   - S4:非 AMR 模式同場景 → 行為與現行完全一致(彈窗、不發 12、不清工單)。
   - S5:CleanOut 中 link down → 立即 fallback 模態(不空等)。
5. **攻防複驗**(memory:adversarial-review-after-feature):實作完成後
   跑對抗性審查,重點打 R1(重複關帳)、R3(計時串台)、狀態機殘留
   (Handshake/lock 在模態 RETRY/SKIP 後的每條路徑)。
6. **上機驗證**(使用者執行):真機 AMR + host 全流程;confirm-compile 慣例,
   我只保證編譯與模擬,機台行為由使用者驗。

## 8. 實作順序(可獨立驗證的步驟)

- [ ] S1 GeneralSetting:`iCleanOutAmrWaitSec`(讀/寫/clamp)→ build
- [ ] S2 main.cpp:抽 `DoLotEndProcess()`,btnLotEnd 薄殼化 → build + 按鈕行為不變
- [ ] S3 aAuto1To6:`IsStationCleanOutUnloadDue` + AMR 車空閘 → build
- [ ] S4 uAgvStation:PollAndCall CleanOut 分支 + CALLED 計時 + watchdog 模態化
      + link-down fallback → build
- [ ] S5 csystem:完成分支改造(CEID28 + AMR 免彈窗 + 自動 Lot End)→ build
- [ ] S6 警報碼註冊(mapAlarmCodeList + AlarmList.csv 跟進)→ build
- [ ] S7 真機組態編譯驗證(SOFT_SIMULATE off → Full → 還原)
- [ ] S8 SECS simulator 情境 S1-S5
- [ ] S9 攻防複驗 → 修正 → 提交
- [ ] S10 docs/SECS comm-examples 更新
- [ ] S11 上機驗證(使用者)

## 9. 待實作時確認的小項(不阻斷)

- 警報碼段位最終取碼(避開既有 mapAlarmCodeList)。
- 模態文案雙語措辭(照 alarm-manual-operator-rewrite 慣例:操作員視角、去程式碼字眼)。
- CEID28 是否需要掛特定 report(現行 EventReport(1,ceid) DataID=1 慣例 vs
  AGV 事件 DataID=0)——實作時對照 host 端需求,預設跟既有按鈕事件同 DataID=1。
