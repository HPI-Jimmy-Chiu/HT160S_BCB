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

## 6. 不動項

- 非 AMR 模式一切照舊（Q1）。
- Loader 源空的 S4「等車窗→自動 CleanOut」不改（Q3 例外）。
- 拒收閘不加（使用者取消）。
- B 類真故障全部不動。
- Part 1-3（超量/TripQueue/自動 Lot End）已 SHIPPED，本案疊加其上。
