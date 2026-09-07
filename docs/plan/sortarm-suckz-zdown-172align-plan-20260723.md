# SortArm 取料/放料 Z-down 對齊 HT172 保護網 — 計畫 (2026-07-23)

狀態：**PLAN ONLY，尚未動 code。** 待使用者核可設計決策後再實作。
關聯記憶：`sortarm-pick-zdown-no-timeout-deadlock`、`silent-stop-must-notify`、
`alarm-registry-ssot`、`alarm-system`、`realdummy-three-tier-io-check`、
`confirm-compile-user-verifies-machine`。

## 1. 問題 (State Record 2026-07-23，真機 RealDummy=2/REALLY)

- 17:15:37 起全機死鎖：`SortArm task 100 / PickTask=45` 卡 5.8 分鐘 → 佔住 Loader-Y
  ownership → Empty/Loader/Auto/TrayArm 連鎖 stuck；只有 300s STUCK watchdog 記一筆。
- MotionDetail：`MSuckZ_2 cmd=-2495 tgt=-2500 home=1`。**命令下降、home sensor 卻仍亮**
  = 步進失步（卡片送脈衝但噴嘴沒離頂）或 home sensor 假亮。
- 根因：pick `case 45` 的 `MovePickZDown()`（[aSortArm.cpp:838](../../HT160S_Program_BCB_V1.0.0.0/aSortArm.cpp)）
  只等 `MotorMove` 回 true；MC88X1 開迴路 `MotionDone()=MC88X1PMotAxisBusy` 一旦閂在
  busy（limit/alarm/未 reset）就永遠回 false → **無逾時、無 home 交叉檢查、無 ClearAxisAlarm →
  無聲鎖死**。place `case 40` 的 `MovePlaceZDown()`（[:854](../../HT160S_Program_BCB_V1.0.0.0/aSortArm.cpp)）同構同險。

## 2. HT172 參考行為（`D:\HT172\HT172_Program_V1.0.25.0_20260420`）

- HT172 用 SMC 卡；`TMySMCMotor::MoveTo` 以 `CheckArmPosArrival(actualPos,Tar,2)` 判到位
  （**有實際位置回讀**）。HT160 SuckZ 是 MC88X1 純開迴路脈衝，**無回讀**。
- runtime 取料 Z-down 保護（aSortArm.cpp:643-651）：命令 Z 下降(ZNeedDown/bZNeedSuck)後，
  **若該 Z 的 `Led[iHomeLed]` 仍亮 → Stop X/Y + 告警**
  `"<axis> Home sensor error, if suck is down, maybe sensor fail!"`。← **要對齊的核心。**
- teach/校正另有失步界限告警（aSortArm.cpp:2810-2820 `"SortArm suck motor step loss"`，
  真空沒吸到就逐步補壓 −10、壓過 −2000 告警）— 屬 teach 路徑，非 runtime。
- 註：HT172 那個「等 MotorMove done」本身(case 500)**也沒有逾時**——真正讓 172 不會無聲卡死的
  是 **home-sensor 交叉檢查 + 位置回讀**，這正是 HT160 漏掉的。

## 3. HT160 現況缺口

- 取料**吸不到**：已有重試+告警（`case 50/52/54`，`SUC0011`/`ShowSuckError`）✓。
- Z-down **移動未完成 / 失步**：**無任何防護**（grep 確認無 step-loss / 補壓 / home 交叉檢查）。
- 現成可用積木：
  - `AreAllSuckersHome()`（[:658](../../HT160S_Program_BCB_V1.0.0.0/aSortArm.cpp)）已讀 live
    `Motor->Led[iHomeLed]`（[:679](../../HT160S_Program_BCB_V1.0.0.0/aSortArm.cpp)）— 與 172 同原語。
  - `ShowSuckError(sucker, codeType, keys, part)` 告警慣用法（1459/1695/1853）。
  - MC88X1 `ClearAxisAlarm()` 存在但只在 driver Jog 路徑內用（myMC88X1motor.cpp:403/421），
    **未透過 `TMyMotor` 對外暴露**。

## 4. 提議修改（對齊 172，適配 MC88X1 開迴路）

> 全部維持 HT160 程序式/switch(Task) 風格、無 FSM、無 C++11、Big5 檔 byte-safe 編修。

- **S1 — Z-down 到位交叉檢查（核心，對齊 172:643）**
  在 pick `case 45`（`MovePickZDown` 未完成時）與 place `case 40`（`MovePlaceZDown`）加：
  對每個「本次命令下降」的 slot，若在一個**有界時間**內仍未完成 **且** 其
  `SuckZMotor->Led[iHomeLed]` 仍為真（噴嘴沒離頂）→ 判為 **move fault**（失步/卡片閂 busy/home 假亮）。
  改為 `MovePickZDown()` 的 helper 版本回傳「done / pending / fault」三態，或在 case 45 內加計時+home 判讀。

- **S2 — 有界告警取代無聲鎖死（對齊 silent-stop-must-notify + 172:649）**
  fault 時：Stop 相關軸 + 發操作員告警（沿用 `ShowSuckError` 或 motor 告警），訊息指名噴嘴，
  例：`"SuckZ_<n> did not leave home (suck down but home on) - step loss or home sensor fail"`。
  提供 `K_RETRY|K_SKIP`（是否給 `K_TRAY_END` 待議）。

- **S3 —（可選）RETRY 自動清警報**：RETRY 時比照 Jog 呼叫 `ClearAxisAlarm()` 並重下 Z
  （需在 `TMyMotor` 暴露 `ClearAxisAlarm()`，或改走重 home 該軸）。**風險**：若真的撞 limit/機構，
  自動清+重下會反覆撞、遮蔽真故障 → 預設**不做**，等 S1/S2 上機看資料再決定。

- **S4 —（可選，低優先）teach 補壓/失步界限**：對齊 172:2810 的 teach 行為；runtime 的
  「吸不到」已由 `SUC0011` 覆蓋，優先度低。

- **告警碼**：新碼登錄進 `mapAlarmCodeList`（SSOT，見 `alarm-registry-ssot`），雙語 remedy；
  沿用穩定碼原則（`alarm-system`）。**不改既有碼字面。**

## 5. 需使用者拍板的設計決策
1. **逾時界限值**：Z-down 允許多久未完成才判 fault？（建議依 Z 行程 + 裕度，先給保守值可設定）
2. **fault 處置**：S2 告警後 `K_RETRY|K_SKIP`，要不要也給 `K_TRAY_END`？
3. **S3 自動清警報**：要（較自動、有反覆撞風險）／不要（預設，人工 jog 清）？
4. **告警碼家族**：走哪個既有家族（MES/JAM/WAR）？由我提候選碼再確認。

## 6. 建置/驗證（實作後）
- 動 `aSortArm.cpp`（Big5+混合行尾 → byte-safe）、可能動 `MyMotor.h/.cpp`（S3）、`mapAlarmCodeList`。
- 刪改動 obj → sim build (`-Clean`)；再真機 build（`SOFT_SIMULATE` off, `-Full`）exit 0 後還原 rebuild。
- encoding/form/alarm 檢查全過。commit 只 add 自己的檔。
- 我保證編譯+模擬；**機台行為（是否真的攔到失步）由使用者上機驗**。

## 7. 範圍外
- 殘料複驗 B2：已洗清、暫緩（另案）。
- SuckZ_2 為何實體到不了位（teach 深度/機構/卡片 limit）：屬硬體/上機診斷，本計畫只保證「不再無聲鎖死、會告警指名」。
