# HT160S 回HOME 機制根因分析與對齊計畫

> 產出：多代理 workflow（13 agents）對 HT160S / HT172(0420) / HT9045 三套程式碼的交叉分析，
> 6 個根因假設，5 個通過對抗式驗證。日期 2026-06-26。
> 本文為 UTF-8 文件；引用之程式碼行號為快照，動工前請以當下檔案複核。

## 實作狀態（2026-06-26）

- **Step 1-4 已實作**（csystem.cpp，byte-safe Latin1 splice，備份 `csystem.cpp.bak-20260626-homefix`）：
  - ion rung(653) + air rung(660) 加 `&& (fHome==NULL || fHome->IsShown()==false)`
  - DoSystem 沉降砍法(868) 加同 guard
  - ProcessHomeLifecycle(1160-1199) 加 `static int iAbortDebounce`，≥2 cycle 才 abort（三處 reset）
  - ion/air rung 補引用 HT172 fShow 的註解（審查 nit）
- **驗證閘全綠**：編碼檢查 160 檔通過｜dev sim `-Clean` exit0｜真機 `SOFT_SIMULATE off` `-Full` exit0｜還原後 dev `-Clean` exit0｜`--selftest-home` PASS（18 enabled 軸全 HOMED、M13/M18 disabled 未跑、exit0）
- **對抗式審查 workflow（4 視角）判定 GO**：0 blocker、0 required fix、invariant 保留=true。
- **上機驗證待辦（user）**：真機重現原始兩症狀已修（維修中按 HOME 不再瞬間關；離開維修後按 HOME 不再反覆關）。
- **Step 5 真機 build 閘已過；Step 6（9045 iHome 解耦）依計畫延後。**

---

## 1. 問題現象（使用者回報）

- **症狀 A（維修中按 HOME）**：進入 maintenance 後按 HOME，HOME 視窗有跳出，但「瞬間」關閉並停止機台動作。
- **症狀 B（離開 maintenance 後按 HOME）**：離開維修後按 HOME，HOME form 不斷被關閉、機台完全不動作。

兩者皆為**真機限定**：`IsSafeDoorOpen / IsAirCheck / IsEMGPressed / UpdateIonFanDebounce`
在 `SOFT_SIMULATE` 下全回 0/false，所以模擬建置重現不了，這也是為何 sim 測試一直過、現場卻爆。

---

## 2. 已驗證的根因（逐週期觸發鏈）

### 共同前提：HOME 的生命週期怎麼串的

```
MainProc() [csystem.cpp:90]
  -> ScanPanelKeys / ProcessStartMode
  -> DoSystem()        [csystem.cpp:158]  --> ScanAllMotorStatus + ScanSystemSenser  (這裡會把 SystemStart 砍掉)
  -> ProcessMotion()   [csystem.cpp:184]
       -> ProcessHomeLifecycle()  [csystem.cpp:1247]   <-- 在 if(SystemStart==false) return 之前
       -> if(SystemStart==false) return;  [csystem.cpp:1249]
```

- `HomeCore()` 在**同一瞬間**設 `SystemStart=true`（main.cpp:1544）+ `MarkSeenStart()`→`fSeenStart=true`（main.cpp:1545 / uHome.cpp:227-230）。`fSeenStart` 一旦 latch 就**不會在 home 中途解除**（只有 form 重建時才 reset）。
- `ProcessHomeLifecycle` 的關窗 / abort 條件（csystem.cpp:1176）：
  ```
  if(fHome->SeenStart() && SystemStart==false && bHomePowerCycling==false)
      { MachineHomeAbort(trigHomeStop); fHome->RequestClose(); }
  ```
- 關鍵：`DoSystem`（安全掃描）跑在 `ProcessHomeLifecycle` **之前**，且 `ProcessHomeLifecycle` 跑在
  `if(SystemStart==false) return` **之前**。所以「同一個 cycle」內：安全掃描砍掉 SystemStart → 生命週期立刻判定為 abort → 關窗。這個順序是 load-bearing（H6 已驗證）。

### 症狀 A 根因（H1 + H5，已驗證 high）

維修時安全門通常是開的／離子風扇還沒到轉速／空壓暫低，而 `TfMaintenance` 的 FormShow/FormClose
**完全不碰** SwMotorRelay / MotorPowerOnDelay / door / RunMode / SystemStart（maintenance.cpp:1780-1791），
所以這些 interlock 帶著「assert 狀態」進到 HOME。

`ScanSystemSenser` 的 `if(SystemStart){…}` 區塊（csystem.cpp:633）裡，四條 soft-interlock rung
**沒有** `fHome->IsShown()` 保護：

| rung | 行號 | 是否有 fHome guard |
|---|---|---|
| SafeDoor | 635-642 | ❌ 無 |
| Emg（區塊內） | 644-650 | ❌ 無 |
| IonFan（REALLY） | 653-658 | ❌ 無 |
| Air | 660-665 | ❌ 無 |
| Emg（MotorPowerOnDelay==0 區塊） | 593-609 | ❌ 無（額外一條，比原假設多） |
| IsSystemPowerOff | 618 | ✅ 有 fHome guard |
| MotorPowerOnDelay 沉降 | 671 | ✅ 有 fHome guard |

→ 按 HOME 後第一個 cycle，任一 leftover interlock 把 SystemStart 砍回 false，
`ProcessHomeLifecycle`（fSeenStart 已 latch、fAllMotorHome 還 false、bHomePowerCycling false）
就 `MachineHomeAbort + RequestClose` → 視窗開了就瞬間關、機台停。**每按必中**。

> 重要更正（來自驗證者）：這四條 rung 是**忠實從 HT172 移植**的（HT172 csystem.cpp 同樣把它們放在
> `if(SystemStart)` 內、不加 fHome guard）。所以「rung 沒 guard」本身不是 160 的偏差。
> 真正讓它**致命**的是 160 多了一個 kernel 端 latched-SeenStart abort（見 §4）。

### 症狀 B 根因（H2，已驗證 high）

離開維修時若馬達電源被切（panel Power-Off：csystem.cpp:563-565 latch `bMotorPowerState=false` + relay off + SystemStart=false），
按 HOME 後電源是經 **panel 路徑**（不是 home 引擎）回來的：
`MotorPowerOnDelay = SERVER_MOTOR_POWER_ON_DELAY = 10`（csystem.cpp:573-574 / cmydef.h:8），
**且 `bHomePowerCycling` 維持 false**（只有引擎 case 1 才會設它 true，uHome.cpp:357）。

於是 `DoSystem` 的這一行（csystem.cpp:868-869）成為元兇——**完全無 guard**：
```
if(CountMotorPowerDelay()==false)   // MotorPowerOnDelay>0 時恆 false [csystem.cpp:433]
    HSys.Sys.SystemStart=false;     // 無 fHome / Run_Home / bHomePowerCycling 任何保護
```
整個約 10 秒沉降窗內，**每個 cycle 都把 SystemStart 砍掉**；每按一次 HOME 都在 cycle 1 被
`ProcessHomeLifecycle`（csystem.cpp:1176）abort+關窗 → 反覆關、永遠跑不起來，直到延遲/電源狀態自清。

### H3 被推翻（重要）

原本懷疑「HomeCore 不擁有 power-cycle，所以引擎 case 1 永遠進不去」——**驗證後推翻**：
`ArmMotorHome` 設 `iHomeStep=1`（csystem.cpp:1074），ProcessMotion Layer 1 每 cycle 步進引擎（csystem.cpp:1275）；
而 `ScanAllMotorStatus` 的 servo-alarm 砍 SystemStart 在 `RunMode==Run_Home` 時被跳過（csystem.cpp:460），
`IsSystemPowerOff`/`MotorPowerOnDelay` 砍在 `fHome->IsShown()` 時被跳過（618/671）。
HomeCore 的順序（Show→Run_Home→SystemStart=true）讓這些 guard 在下個 cycle 已生效，所以 **case 1 進得去**。
→ power-cycle 不是元兇；元兇是上面那些**沒被 guard 到**的 soft rung 與 868-869。

---

## 3. 三套機台 HOME 架構對比

| 面向 | HT160S（現況） | HT172（0420 參考） | HT9045（參考） |
|---|---|---|---|
| home 引擎 gate 在什麼 | **SystemStart**（與 production 共用） | SystemStart（與 160 同） | **獨立 iHome 旗標**，home 完全不寫 SystemStart |
| 關窗決策位置 | **kernel 端** ProcessHomeLifecycle（latched-SeenStart abort，跑在引擎前一步） | **引擎自身** top guard（uhome.cpp:859-868），SystemStart==false 時自己 Close | 引擎/Timer **永不自動關**；只有操作員 Pause/abort 才關（uhome.cpp:4867-4871 純顯示） |
| home 期間 soft interlock | door/emg/ion/air 與 868 沉降 **多半沒 fHome guard** | motor-power 用 `&& fHome->fShow==false` 抑制（csystem.cpp:409/631）、ion 20s 窗 | interlock 整塊在 `if(SystemStart)` 內，home 時 SystemStart==false → **整塊不評估**（csystem.cpp:4394） |
| 寬限窗 | 零散補丁（ion 20s、power-cycle 等，補了 8 次） | 有（ion 20s、iHomeStep>1000 等） | 有系統性寬限（iHomeStep<5 忽略 motor-power、ion 20s 直到 homed） |
| 看門狗 | 無 | — | 有 main-loop liveness（csystem.cpp:16373-16409） |

**結論（架構偏差，H4 已驗證 high）**：160 同時採了兩個最脆弱的選擇——
(1) 把 home 引擎綁在 production 的 SystemStart，**又** (2) 把關窗搬到 kernel 端、用 latched-SeenStart abort
且跑在 SystemStart early-return 之前。任何一個 cycle 短暫把 SystemStart 砍掉，就**不可逆地**被當成操作員 abort。
HT172、HT9045 都沒有「(1)+(2) 同時存在」這個組合，所以都不會有這個 bug。

---

## 4. 決策：問題 1「重新參考 172 還是 9045」→ **兩者都借鏡（hybrid），分階段**

- **即刻修（低風險、現在做）→ 對齊 HT172**：因為 HT172 跟 160 一樣 gate 在 SystemStart，
  它的 `fHome->fShow==false` 抑制手法可以**幾乎零 blast radius** 直接套進 160 現有結構，不必新增狀態機。
  做法是把「home 期間屬於 HOME 自身電源/沉降產物」的 soft interlock 在 fHome 顯示時**抑制掉**（不要去誤砍 SystemStart），
  EMG／安全門**不抑制**（真故障仍須停，由引擎自身 guard 收尾）——這同時保住
  「SystemStart=false 永遠停馬達」這條鐵律（我們是「一開始就不誤設 false」，不是「事後忽略 false」）。
- **架構終局（延後，只有殘留脆弱才做）→ 對齊 HT9045**：引入獨立 `iHome` home-mode，
  讓引擎以 home-mode 步進、完全不碰 SystemStart，從結構上消滅整類「transient 砍 SystemStart → abort HOME」。
  blast radius 大（每個 force-rehome 的呼叫點都要改走 home-mode），不與即刻修綁在一起。

> 一句話回答使用者：**先用 172 的手法把症狀根除（步驟 1–3），架構上若仍脆弱再走 9045 的 iHome 解耦（步驟 6）。不建議繼續零散 patch（已失敗 8 次）。**

---

## 5. 具體步驟（最小 blast radius 先；每步皆過 build gate）

> 編碼鐵律：csystem.cpp 是 Big5，**勿用 Edit 工具直接改**（會壞 Big5）。用
> `scripts/ops/bcb6-bytesafe-edit.ps1` 或 python/Latin1 splice；新註解 ASCII-only；
> 改後 `scripts/ops/check-ht160s-source-encoding.ps1` + 刪 obj 重編。

1. **STEP 1 — 抑制 home 期間的 soft interlock（殺症狀 A 的 ion/air 面）**
   在 `ScanSystemSenser()` 的 IonFan rung（csystem.cpp:653）與 Air rung（660）加
   `&& (fHome==NULL || fHome->IsShown()==false)`，對齊 HT172 csystem.cpp:409。
   **SafeDoor(635)、Emg(644) 不加**（真故障必停）。
   ⚠ 需真機（SOFT_SIMULATE off）`-Full` build 檢查（這些分支 sim 不編譯）。

2. **STEP 2 — guard 住沉降砍法（殺症狀 B）**
   把 `DoSystem()` 的 csystem.cpp:868-869
   `if(CountMotorPowerDelay()==false) SystemStart=false;` 改成
   `… && (fHome==NULL || fHome->IsShown()==false)`，與 671 既有 guard 一致，
   停掉 panel power-on 後整個 10-cycle 反覆關窗。⚠ 需真機 build 檢查。

3. **STEP 3 — kernel abort 加 transient 去抖（硬化 A/B，不破壞鐵律）**
   `ProcessHomeLifecycle` abort gate（csystem.cpp:1176）只在 SystemStart 連續 ≥2 個 home cycle
   為 false 時才 abort（小 static/member 計數器，SystemStart 為 true 時歸零），
   讓被 STEP 1/2 抑制掉的單 cycle transient 不會誤觸 RequestClose。
   `fAllMotorHome`-first 完成檢查維持在前面不動。

4. **STEP 4 — 收斂「關窗 owner」單一化**
   稽核 fHome 的關閉者只剩：引擎完成、kernel abort（已去抖）、操作員 Pause/abort。
   確認 `uHome.cpp` Timer1Timer 仍是純顯示、**沒有 else Close()**（呼應既有 memory
   home-timer-aligned-to-HT172）。clean 的話不改碼，只在 abort gate 補 ASCII 註解記錄此不變式。

5. **STEP 5 — 回歸閘**
   跑 `ht160s-home-selftest`（--selftest-home headless 全機 HOME，確認每個 Mot_Table-enabled 軸到 HOMED、M13/M18 保持 disabled）；
   再跑文件化的真機 build pass：MachineType.h 註解掉 `#define SOFT_SIMULATE` → `-Full` exit 0 → **還原 define** 重編。
   交付使用者上機驗兩個原始症狀。

6. **STEP 6（延後，架構終局）**
   只有步驟 1–3 上機後仍殘留脆弱才做：引入 `iHome`/`bHomeMode`，讓 ProcessMotion 以 home-mode 步進引擎、脫離 SystemStart。
   高 blast radius，獨立進行，全 `-Full` + 真機 + selftest + 上機簽核。
   **併入此步的審查 nit**：目前 air rung(660) 抑制是「整個 HOME 顯示期間全抑制」且 `IsAirCheck()` 無時間窗（不像 ion 有 20s/5s debounce）；長/卡住的 HOME 中真實低氣壓會整段不報警。HT172 因 home 不綁 SystemStart 而無此問題。Step 6 解耦後 air 應回到 always-live；在那之前若機構上有「HOME 中低氣壓」風險，可仿 HT9045 `iHomeStep<5` 給 air 抑制一個 HomeStep/時間上限，而非整段抑制。

---

## 6. 借鏡穩定機台作法清單（問題 3）

1. **home 期間抑制 volatile interlock**（motor-power-off / 沉降 / ion-fan）以 fHome guard——HT172 csystem.cpp:409/631、ion 20s 窗 128-132。好處：HOME 自身的 power-cycle/沉降不會自我 abort。
2. **EMG / 安全門 home 期間不抑制**——保住「SystemStart=false 永遠停馬達」鐵律；抑制只限已知 power-cycle 產物。
3. **單一關窗 owner**：引擎完成 + 操作員 abort；週期 timer 純顯示永不關窗——HT9045 uhome.cpp:4867-4871。
4. **系統性早期寬限窗**（iHomeStep<5 忽略 motor-power、ion 20s 直到 homed、EMG 進場去抖）——HT9045 csystem.cpp:4358/3170、HT172 265-281。根治時序競態，取代逐症狀補丁。
5. **獨立 home-mode 旗標 iHome**，引擎選擇不綁 SystemStart——HT9045 main.cpp:6921 / csystem.cpp:17534。結構性消滅整類 bug（步驟 6）。
6. **SystemStart drop 一律配操作員彈窗 + 乾淨 abort（含 servo 斷電）+ 每軸 home 失敗 jam code 分類**——HT9045 csystem.cpp:10159、uhome.cpp:442-462/4962-4998。無聲停機是 160 既有鐵律。
7. **main-loop liveness 看門狗**（last-enter 時戳 + call count + IsMainProcAlive）——HT9045 csystem.cpp:16373-16409。給未來 HOME hang 的 State Record 分析一個明確 liveness 訊號。

---

## 7. 風險

- STEP 1/2 抑制可能遮掉 home 中「真的」斷電/低氣壓 → 緩解：只限 fHome 顯示時、EMG/門仍 live；真缺電源會讓該軸到不了 HOMED，被 --selftest-home 抓到。
- STEP 2 讓 SystemStart 撐過沉降窗 → 緩解：只在 fHome 顯示時；production START 路徑 fHome 不顯示，不受影響；真機 `-Full` 驗 `#ifndef SOFT_SIMULATE` 分支。
- STEP 3 去抖延遲操作員 abort 一個 tick（<50ms，無感）；且 Pause 鍵直接關 fHome，不靠 kernel gate。
- Big5 編碼回歸（Edit 會壞）→ 用 byte-safe 工具 + 編碼檢查。
- STEP 6 高 blast radius → 明確延後、單獨做、全閘把關。
- 四條 soft interlock 在 SOFT_SIMULATE 全回 0 → sim 無法重現症狀 A；真機 `-Full` build + 上機驗證為必要，不可只憑 sim exit-0 宣稱已修。
