# AMR 無人化 Alarm 分流導入計畫

- 日期：2026-07-21
- 分支：`feat/iosetview-172-refactor`
- 狀態：**認知同步定案（Q1/Q2/Q3 全確認），開工中**
- 前置：本案疊在已 SHIPPED 的超量/TripQueue 案（`bd9f8f0`/`e77bb7c`/`c52cc77`/`4cf4b4b`，見 [loader-overcount-nostop-cleanout-lotend-plan-20260721.md](loader-overcount-nostop-cleanout-lotend-plan-20260721.md)）之上。

---

## 1. 原則（使用者定義）

**AMR 模式 = 全自動無人線。** 機台碰到「物料太多 / 太少」時：
- **一律靜默跟 AGV 握手**（叫供料 / 叫收料），**正常情況操作員看不到任何 Alarm**。
- **只有「AGV 叫了卻不來（逾時）」才升級成 `WAR0962`（B 類 Alarm）**。
- **真故障**（撞機、汽缸沒到位、盤掉/迷失、身分盤無法辨識、後方殘留、CCD 斷）→ 照舊 Alarm。

**確認的三個界線：**
- **Q1（yes）**：整案**只在 `GeneralSetting.bUseAMR` 生效**；非 AMR（有人顧的手動線）一切照舊，所有既有操作員彈窗全部保留。
- **Q2（yes）**：AMR + AGV 正常時，操作員**完全看不到**「滿倉/缺料」窗；它們變成「靜默叫車 + 等」。原本的換車/清盤操作員窗**只在 AGV 逾時不來時**化為 `WAR0962`。
- **Q3（接受 Loader 例外）**：`WAR0962` 涵蓋 **Auto 收料** 與 **Empty/Color 供料** 的握手逾時；**Loader 源空走 S4 的「等車窗→自動 CleanOut」**（視為此 lot 供料結束），**不走 WAR0962**（機台分不出「沒車＝lot 結束」還是「AGV 壞」，選項 B 定案當 lot 結束）。
- **拒收閘取消**：供料站（Loader/Empty/Color）機構上都能承受 AGV 主動送料（使用者確認），故**不加「滿時拒絕 AGV 放料」閘**——供料滿只需「不 Alarm + 排料 GoUp 照走」。

## 2. 站別 × 條件 最終處理表

| 站別 | 角色 | 源空（InputEnd OFF）| 滿（Full sensor ON）| AGV 叫了不來 |
|---|---|---|---|---|
| Loader | 供料 | 等車窗→自動 CleanOut（**S4 已做**）| 無 full sensor；接受 AGV 主動供料 | 走 CleanOut（非 WAR0962，Q3 例外）|
| Empty / Color | 供料 | 叫車補（Color 已對；**Empty 抑制 MES1022**）| **不叫 + 不 Alarm（拆 MES1023/1427）+ GoUp 照走** | **WAR0962** |
| Auto1-6 | 出料 | — | **叫 AGV 收（D4）；絕不硬 GoUp** | **WAR0962** |

**其餘 B 類真故障不動**：MES0924/1426（後方殘留）、MES1421/1424（Color 盤遺失/未就緒）、JAM0913/JAM1030/JAM%d02（盤掉/push miss）、MES0925（rise1 沒縮回）、MES1021/MES1024/WAR%d30（tray miss）、MES1721/1722/1723（TrayArm）、WAR0154（SortArm limit）、SortArm/TrayArm move blocked（撞機）、WAR0330/CCD/2D（通訊）。

## 3. WAR0962（AGV 逾時，新增）

- **掛碼**：`WAR0962`（對齊 9045 `asendic_Loader.cpp:1963`——AMR 等料逾時 `htAMRLoaderWaitTimer` 600s → `ShowErrorMessage("WAR0962", K_RETRY|K_CLEAN_OUT)`）。
- **HT160S**：B 類 Alarm；recovery 以 `K_RETRY`（重叫 AGV）為主（`K_CLEAN_OUT` 可選加）。註冊進 `mapAlarmCodeList` SSOT（CSV+S5F1 自動跟進）。
- **逾時秒數**：`tsMaintSECS` 加一個可設定欄位，**預設 300**。可直接用 S6 已加的 `[AGV]CleanOutAmrWaitSec`（預設 300）當來源，或另開新欄位——動工時定。
- **取代**：現行 `ServiceHandshake` 的靜默 watchdog force-release（`iAmrHandshakeWaitSec` 240s，只 `RecordProcess` 不報）→ 改成逾時→`WAR0962`。涵蓋 Auto 收料（P4-P9）與 Empty/Color 供料（P2-P3）握手；Loader（P1）不掛（Q3 例外）。

## 3.5 Phase A 偵察結論 + 最終設計（2026-07-21 認知同步完成）

**使用者不變量（定案）**：每條 Auto 流道**永遠只有一張盤在運作**；該盤 GoUp 完成後、確認 Full 沒亮，才能在 rear 跟 TrayArm 要下一張。Full 亮 → 叫 AGV 收，AGV 結束＋Full off 後 rear 才能要盤。

**實碼佐證**：`GetTrayRequest`（aAuto1To6.cpp:1248-1251）本就要求 working 空＋rear 空＋無在途才要盤＝序列化已存在。推論鏈：Full 只在 GoUp 完成瞬間亮 → 亮後進料停 → **Full 亮時流道必空** → AGV 收車時下方永遠沒盤 → **嚴格 Ready（IsDrainedForAmr）通吃 Normal+CleanOut，不需獨立放寬 Ready**（原 IsReadyForFullCollect 提案收回）。

**缺角**：要盤的滿判斷只看 `iTrayCount>=MAX_TRAY_PER_CAR(100)`（sim 計數，:1218），真機 Full sensor 沒進要盤條件——必補。殘餘毫秒級在途競態（TrayArm 已在途時 Full 恰亮→盤滑入流道→嚴格 Ready 被擋）→ fail-safe 至 WAR0962，非撞機非無聲。

**附帶發現（D4 順修）**：現行已出貨碼 latent silent-stall——CleanOut 最後一張 GoUp 填滿車時 finish 被 :1116 Full 閘擋住、CleanOut 中無人叫車也無彈窗 → 無聲卡死。

### 實作規格（批次 4 先行、批次 1 接續，一起驗）

**W1 GeneralSetting**：`iCleanOutAmrWaitSec`（S6 已加、未消費）改名 `iAgvTimeoutSec`／ini `[AGV] AgvTimeoutSec`（預設 300、clamp≥5）——單一設定服務所有 AGV 逾時（Normal 滿車等收＋CleanOut 等收＋供料等補）。
**W2 database**：註冊 `WAR0962`（standalone，仿 MES0925）「AGV/AMR handshake timeout」進 mapAlarmCodeList SSOT。
**W3 uAgvStation**：CALLED 也納入 aging（現行 watchdog 只 age PREP/READY；CALLED=host 未回 START_AGV 會永遠靜默）。P2-P9（Empty/Color 供料＋Auto 收料）逾時→不再靜默 force-release，改記 pending 逾時旗標；**P1 Loader 維持現行**（Q3 例外）。新增小 helper `AgvRetryStation(si)`（重置該站→IDLE 讓 PollAndCall 重叫）供 RETRY 用。
**W4 csystem/主迴圈彈窗**：WAR0962 modal 一律從主控制迴圈彈（uAgvStation 在 SECS timer，不彈 modal）——消費 pending 旗標→`ShowMyError("WAR0962", 站名+逾時, K_RETRY)`→RETRY=AgvRetryStation 重叫。
**W5 tsMaintSECS UI**：AgvTimeoutSec 秒數設定欄（maintenance 表單，DFM 文字模式編修）。
**D4-1 GetTrayRequest**：AMR 真機加 `IsOutputCarFullForAmr(Index)→eTrayReqNone`（使用者不變量的缺角）。
**D4-2 PollAndCall**：P4-P9 滿車 CALL 迴圈改 Run_Normal ∪ Run_CleanOut 都跑（P1-P3 進料維持 Run_Normal-only + CleanOut 釋放）。
**D4-3 case-4000 滿倉閘**（aAuto1To6.cpp:973）：AMR→**不彈 MES1120~1620、不硬 GoUp**，該站本 tick skip（`continue`），等 ServiceHandshake 收車（273/274/ClearAmrCar 在 CleanOut 本就會跑）＋Full off 後續 GoUp；逾時由 W3/W4 的 WAR0962 兜底。非 AMR 維持現行 do/while 操作員窗。
**D4-4 ServiceCarFull**：AMR 分支的逾時 MES1120/MES1125 操作員窗改 WAR0962（主迴圈 context 可直接彈）；等待語意不變（bWaitingAmrFull+iAgvTimeoutSec）。
**手動自癒保留**：操作員手動清車 → Full/InputEnd sensor 變化 → 既有 `bFull==false&&CALLED→IDLE` 與 `IsAmrTaken(InputEnd OFF)→274→ClearAmrCar` 自然收斂，無需特殊碼。

## 4. 施工批次（每批：Phase A 偵察 → 動碼 → build gate → 對抗複驗）

- **批次 1（主戰場）D4 — Auto 出料滿叫 AGV 收**
  - Run_Normal 已有滿倉 CEID272 CALL（`uAgvStation.cpp:282`）；**補到 CleanOut**（PollAndCall CleanOut 段跑 P4-P9 滿倉 CALL，觸發用 Full sensor `IsOutputCarFullForAmr`）。
  - `aAuto1To6` case-4000 CleanOut-drain 滿倉閘：AMR 下**不彈 MES1120~1620 操作員窗**，改「叫 AGV 收 → 尊重 bAmrLocked 暫停該站 → ClearAmrCar 後續 GoUp」；逾時→WAR0962（批次 4）。
  - Run_Normal `ServiceCarFull` 的 MES1120~1620/MES1125~1625 操作員窗：AMR 下同樣改「叫車+等」，逾時→WAR0962。
  - **Phase A 未知**：`DoAllAutoCleanOut` 排料 ↔ `bAmrLocked` ↔ 收車握手（273 `IsDrainedForAmr` 對滿車 mid-drain 是否成立、drain 是否尊重 lock）——動碼前先查。
- **批次 2 — Empty/Color 供料滿**：拆 `MES1023`（aEmpty.cpp:388）/`MES1427`（aColor.cpp:434）CleanOut-drain 操作員窗，AMR 下 GoUp 照走、不 Alarm、不加拒收閘。
- **批次 3 — 供料空抑制**：Empty `MES1022`（aEmpty.cpp:346）AMR 下抑制（對齊 Color 只叫車）；Loader `MES0920` 確認 AMR 零殘留（S4 已多半覆蓋，補殘留路徑）。
- **批次 4（新）— WAR0962**：新碼註冊 + `tsMaintSECS` 秒數設定（預設 300）+ 取代靜默 watchdog（Auto/Empty/Color 握手逾時→WAR0962；Loader 不掛）。

## 5. 建置 / 驗證 Gate（每批）

1. 刪改動 .obj → `build-ht160s.ps1 -Clean` EXIT 0；動 `#ifdef SOFT_SIMULATE` 分支 → 真機組態 `-Full` EXIT 0 → 還原重建。
2. `check-ht160s-source-encoding.ps1` 通過。
3. `--selftest-home` PASS。
4. SECS simulator 情境：CleanOut 中把某 Auto 灌到滿 → 機台發 CEID272 → 模擬器 START_AGV → 273/274 → ClearAmrCar → 排料續走 → finish；AGV 不回 → WAR0962。
5. 對抗式複驗（session 用量允許時跑 workflow；否則主迴圈手動走查）。
6. 上機驗證（使用者）：真機 AMR + 模擬器 host。

## 5.5 RESUME STATE（2026-07-22 暫停點 — 下次從這裡接續）

**已 SHIPPED（feat/iosetview-172-refactor）：**
- `271560f`：行為全套（W1-W4 + D4-1/2/3/4 + 批次2/3）。dev+真機 build EXIT0、encoding165、selftest PASS。Big5 檔 cp950 byte-safe。
- Run_Home guard 修正（W4-fix，csystem `ServiceAgvTimeoutAlarm` 加 HOME 凍結）——**本段落最後一個 commit**（見下）。

**對抗式複驗結果（workflow w6v3b9iyh，裁定 FIX-FIRST low-urgency；核心 SOLID：WAR0962 aging/latch/main-loop 消費/P1 排除/D4 閘/非AMR不變 全部確認乾淨）。已修/待辦：**

1. **[已修] Run_Home guard**（MEDIUM）：`ServiceAgvTimeoutAlarm` 原本只 gate `bUseAMR`，HOME 前若有 TimeoutPending 會彈 WAR0962（DecStopAllMotor+SystemStart=false）打斷 homing。已加 `if(RunMode==Run_Home || fAllMotorHome==false) return;`。

2. **[已定案 2026-07-22 — 使用者裁定正確，複驗發現撤回] Color MES1421**：使用者更正後重新查證（workflow wf_1b67a2f1-17d，2 reader + 1 adversarial verify，全數 agree）：
   - `SnColor_InputEnd`＝Color **真正的源乾 sensor**（aColor.cpp:186-192 `IsInputShortageForAmr()`＝InputEnd.IsOff()；AGV 叫車/補料完成判定都看這顆，uAgvStation.cpp:52 P3 已接）。
   - `SnColor_OutputBottomHasTray`＝**rear 區有無盤**（aColor.cpp:286-301 → `bRearHasTray`），與使用者說法一致。
   - **MES1421（aColor.cpp:1131-1153）＝盤送到 rear 後不見＝B類真故障**（夾好的盤在搬運途中遺失/未到位，AGV 補供料倉放不了盤到 rear）——**使用者原裁定正確**，是 Empty MES1021（rear-miss）的對等，不是 MES1022 的對等。**維持不抑制；shipped 程式碼本來就沒動它，無需修改**。
   - 錯誤來源＝aColor.cpp:1135-1138 的**程式註解寫錯**（把 rear 檢查標成 "source-dry ... refill the supply magazine"），連 w6v3b9iyh 複驗都被誤導。
   - Color **沒有**源乾 MES 碼（不像 Empty MES1022/Loader MES0920）→ 源乾路徑＝AGV 叫車+WAR0962，**無雙重告警問題**（先前擔憂撤回）。aEmpty.cpp:322 註解「Color 無 supply-empty alarm」**是對的**，不用修（先前修正項取消）。
   - **殘留一個小待裁定**：MES1421 外包著既有 AMR 等待層（aColor.cpp:1139-1152，bUseAMR 下先等 iAmrFeedWaitSec 才跳）——但 AGV 補的是供料倉、放不了盤到 rear，等待無意義且讓 B 類真故障晚跳。選項 A＝拆等待層（立即跳，符合「真故障立即 Alarm」原則）+修註解；選項 B＝只修註解、行為保留。**待使用者裁定**。

3. **[已結案 2026-07-22 — 死鎖不可達，撤回] case-4000 邊角**：同 workflow 查證＋對抗複驗 agree：**「輸出車滿 且 載台有在製盤」在真機上不可能同時發生**，鏈條第一步就不成立。結構性保證（全在 aAuto1To6.cpp）：(A) 在製盤只能由 rear 晉升產生，FindFeedAuto 只在 `bCarHasTray==false` 時動（:474）；(B) rear 只在**車不滿**時才收盤——GetTrayRequest 對 Full/bCarHasTray/rear-pending 全回 None（:1257,1259,1267-1268 D4-1）；(C) 車「變滿」唯一途徑＝GoUp 推疊，而正常出料 case1000 **先清** bCarHasTray（:796）**才**在 case6100 推升觸發 Full sensor（:840）→ Full 亮起瞬間載台必空。故 drain 遇滿車被 D4-3 hold 時載台一定是空的 → `IsDrainedForAmr`（:1428）可滿足 → 273 發 → AGV 收 → 排料續行。複驗附註一個理論窗（case1000→6100 之間 GetTrayRequest 短暫重開），物理上不可能在出料尾段完成整趟 TrayArm 取放，且即使發生也有 WAR0962 兜底非無聲。**無需任何設計變更**。

4. **[NIT] 殘留死碼**（D4-4 後）：`bWaitingAmrFull[]`/`AmrFullWaitTimer[]`/`iAmrFullWaitSec`/`AbortAutoHandshake` 已不再使用；可清。
5. **[LOW] RetryStation 註解過度宣稱**「manual self-heal via bFull==false&&CALLED→IDLE」——實際靠 re-CALL 贏 race；修註解。
6. **[NIT] ini 遷移**：舊 `[AGV]CleanOutAmrWaitSec` 改名 `AgvTimeoutSec`，舊機 ini 舊 key 失效落回預設 300（原本就是 no-op 欄，無行為變化）。

**未做：**
- **W5**：`tsMaintSECS` 加 `AgvTimeoutSec` 秒數 UI 欄（from-scratch DFM，文字模式，有剝除風險）。設定已可由 General.ini `[AGV]AgvTimeoutSec` 編、預設 300。
- **上機 + SECS-sim 情境驗證**（含 CleanOut 灌滿 Auto→AGV 收；AGV 不回→WAR0962）。

**下次開工建議順序**（2026-07-22 更新：#2/#3 兩大設計點已結案）：等使用者裁定 MES1421 等待層 A/B → 修註解（aColor.cpp:1135-1138，cp950 byte-safe）→ 清 #4/#5 → W5 → 全 build gate → 上機。

## 6. 不動項

- 非 AMR 模式一切照舊（Q1）。
- Loader 源空的 S4「等車窗→自動 CleanOut」不改（Q3 例外）。
- 拒收閘不加（使用者取消）。
- B 類真故障全部不動。
- Part 1-3（超量/TripQueue/自動 Lot End）已 SHIPPED，本案疊加其上。
