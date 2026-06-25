# Machine Command Layer — START 遷移與手動/自動同步計畫

狀態：**計畫（不動碼）** ｜ 日期：2026-06-25 ｜ 分支：`feat/iosetview-172-refactor`
關聯記憶：`machine-command-layer`、`secs-9045-vs-160-diff`、`ht160s-secs-host-message-coverage`

---

## 0. 緣起與已查證事實（3-agent workflow 稽核）

使用者反映：「S2F41 START 有危險 bug — 軟體開啟無 Lot/2D，host 下 SECS START，`bProgramStart` 被打 true，之後 SECS 補 Lot+2D 機台突然動。」

**查證結論（對抗式 agent 嘗試反證、無法推翻）：此情境在目前 HEAD 無法成立。**

- SECS S2F41 `RCMD=START`（`SecsGem/uHGemHT160.cpp:814`）是 **gate-before-act**：
  `CheckLotDataReady()==false` → 回 `HCACK=2`，**不寫任何旗標**（不碰 `SystemStart`/`SoftStart`/`bHomeByStart`/registry）。拒絕即遺忘。
- 後補的 2D（WebAPI async `main.cpp:PollLotDataWebApi`）**只更新 `LotRegistry` + 刷 UI，無 start 權限**。
- 真正 arm 動作的 latch 是 `SoftStart`（由 `ProcessStartMode` 下一個 cycle 消費，`csystem.cpp:894`），**只**被三條操作員路徑設 true：Start 鈕、HOME 鈕、Note 異常恢復。**無任何 SECS / async 路徑設它。**
- `bProgramStart` 是命名陷阱 = 「軟體初始化完成」旗標，與生產 START 無關；唯一靠它自動 home 的 `g_SelfTestHome` 區塊被 `#ifdef SOFT_SIMULATE` + `g_SelfTestHome` 雙重包住，真機 build 編不進去。

> 此保護是 commit `9e28ed4`（2026-06-25）才補上。若曾在更早 build 觀察到該行為，現行碼已安全。

### 稽核挖出的三個真實點

1. **`bHomeByStart` 未-home 的 host START 會「home 完才自動進生產」**
   - `main.cpp:1709` 設、`csystem.cpp:1267` 消費：未 home 時 START 先 home，home 完成時壓掉本該停的 `SoftStop` 直接滑進生產。
   - 對操作員按鈕是設計；對 SECS 遠端是「一命令、延遲動作」。
   - **決策（使用者 2026-06-25）：維持 home 後自動跑，不改成拒絕。** → 本計畫保留此行為，不動 `bHomeByStart` 語意。

2. **`CheckLotDataReady` 只查「有沒有」，不查「對不對」**（`main.cpp:1635`）
   - 只檢查 `edLotNo` 非空、`GetLotCount()>0`、`GetItemCount()>0`（全域計數），**未檢查 active lot 的 2D 是否齊**。
   - 風險：host 推 lot A 的 2D、START 想跑 lot B，gate 照樣過。資料正確性缺口。

3. **命令非對稱（模組化未完成）**
   - 只有 `START` 走操作員路徑（`fMain->Start()`）；`PAUSE` 在 GEM 層內聯、跳過 `RecordProcess`+`EventReport`；無 SECS `STOP`/`CLEANOUT`/`ONECYCLE`/`HOME`。
   - `SystemStart=true` 在 4 處升起：`ProcessStartMode`(902)、`Start()`(1693)、`sbHome1Click`(1533)、selftest(128)。

---

## 1. 目標架構

**單一生產起跑閘門**：全機只有一個函式能讓生產 arm，一道前置檢查，手動＝自動＝面板鍵三條路徑共用。

骨架已存在於 `csystem.cpp:1096`（`MachinePause/MachineStop/MachineHomeAbort` + `eMachineTrigger`），註解明寫「operator Start button … is deferred」。本計畫把 Start 收進來、並把 Pause/Stop 接線。

```
              +-------------------------------------+
 operator --> | TfMain::Start(trig, Reason)         |
 SECS     --> |   gate: SystemStart? CheckLotData?  | --> result code
 panel    --> |   act : SystemStart=true/SoftStart  |
              |         OnLotStart/Loader/home-first |
              +-------------------------------------+
 caller maps result:  operator -> ShowMyMessage ; SECS -> HCACK ; panel -> silent
```

---

## 2. Slice A — 單一 START 閘門（核心）

### A1. `csystem.h` — 新增結果列舉與 `MachineStart` 宣告（header 變更 → `-Full` build）

於 `eMachineTrigger`（:57）與 `MachineHomeAbort` 宣告（:69）附近新增：

```cpp
// AI(machine-command-layer) 20260625 : single production-start gate result.
enum eMachineStartResult {
    msStarted,        // production armed (or home-then-run armed when unhomed)
    msRejNoContext,   // fMain not available
    msRejBusy,        // already running
    msRejNotReady     // lot/2D not ready (Reason filled by caller path)
};
eMachineStartResult MachineStart(eMachineTrigger trig, AnsiString &Reason);
```

### A2. `csystem.cpp` — 新增 `MachineStart` 薄包裝（置於 `MachinePause` 上方）

```cpp
// AI(machine-command-layer) 20260625 : single entry for production start. Kernel/SECS
// can arm production without touching fMain directly; the body still lives in
// TfMain::Start (heavy UI coupling: edLotNo / OnLotStart / Loader / fHome monitor).
eMachineStartResult MachineStart(eMachineTrigger trig, AnsiString &Reason)
{
    if(fMain==NULL)
        return msRejNoContext;
    return fMain->Start(trig, Reason);
}
```

### A3. `main.cpp` — `TfMain::Start()` 改簽章、移除內部 modal（:1666）

**改前**（節錄）：`void TfMain::Start()` { 內含 `CheckLotDataReady` 失敗時 `ShowMyMessage(Reason); return;` }

**改後**：

```cpp
// AI(machine-command-layer) 20260625 : returns a result code instead of popping a modal.
// The modal moved OUT to the caller so the SECS receive path can reuse this without a
// blocking dialog (the previous reason the SECS branch had to duplicate the gate).
eMachineStartResult TfMain::Start(eMachineTrigger trig, AnsiString &Reason)
{
    if(HSys.Sys.SystemStart!=false)
        return msRejBusy;
    if(CheckLotDataReady(Reason)==false)
        return msRejNotReady;

    RecordProcess(AnsiString("MACHINE START by ")+MachineTriggerName(trig));
    tSimuData.bRunSimulation=cbEnableSimulation->Checked;
    HSys.Sys.SystemStart=true;
    SoftStart=true;
    g_DeviceInfo.OnLotStart(edLotNo->Text, Now());
    if(LoaderModule!=NULL)
        LoaderModule->SetCurrentLotNumber(edLotNo->Text);

    if(fAllMotorHome==false)
    {
        // unchanged: START on an unhomed machine homes first, then auto-runs
        // (bHomeByStart kept per 2026-06-25 decision).
        bHomeByStart=true;
        ChangeRunMode(Run_Home);
        ArmMotorHome();
        fHome->lstHomeMsg->Clear();
        fHome->lstHomeMsg->Items->Insert(0, "Starting home procedure....");
        fHome->Show();
        fHome->MarkSeenStart();
    }
    return msStarted;
}
```

> 註：原 `RecordProcess("START pressed")` 改為 command-layer 統一格式 `MACHINE START by <trig>`，與 `MachinePause/Stop` 一致、可 grep 出觸發來源。

### A4. `main.h` — 更新宣告

`void Start();` → `eMachineStartResult Start(eMachineTrigger trig, AnsiString &Reason);`
（需 `#include "csystem.h"` 或前置宣告 `enum eMachineStartResult;`；確認 main.h 既有 include 鏈。）

### A5. 三個呼叫端改走閘門

- **`sbStart1Click`（`main.cpp:1585`）**
  ```cpp
  void __fastcall TfMain::sbStart1Click(TObject *Sender)
  {
      AnsiString Reason;
      if(MachineStart(trigOperator, Reason)==msRejNotReady)
          ShowMyMessage(Reason);   // busy/no-context -> silent (nothing to tell operator)
  }
  ```
- **實體面板鍵（`ScanPanelKeys`, `main.cpp:1760`）**：`Start();` → 
  ```cpp
  { AnsiString Reason; if(MachineStart(trigOperator, Reason)==msRejNotReady) ShowMyMessage(Reason); }
  ```
  （面板鍵原本就呼叫無參數 `Start()`，務必一起改。）
- **SECS START 分支（`uHGemHT160.cpp:814-848`）**：移除重複 pre-gate 與 modal-avoid 註解，改：
  ```cpp
  HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
  AnsiString Reason;
  switch(MachineStart(trigSecsRemote, Reason))
  {
      case msStarted:      HCACK=0; break;
      case msRejBusy:      HCACK=4; break;   // already running
      case msRejNoContext: HCACK=2; break;   // no UI context
      case msRejNotReady:                    // lot/2D not ready
      default:             HCACK=2; break;
  }
  ```
  保留 exact `=="START"` 比對（不吃 `START_AGV`）。

**A 完成效果**：全機唯一生產起跑點＝`TfMain::Start`；`SystemStart=true` 升起點由 4 降為 3（移除 SECS 端重複），且 SECS/面板/觸控三路徑保證行為一致。

---

## 3. Slice B — Pause/Stop 接線（修非對稱）

- **`sbPause1Click`（`main.cpp:1589`）**：內聯 `SystemStart=false; SoftStop=true` → 改
  ```cpp
  void __fastcall TfMain::sbPause1Click(TObject *Sender)
  {
      if(HSys.Sys.SystemStart==true)
          EventReport(SECS_EVENT.PressPause);  // operator-specific event stays here
      MachinePause(trigOperator);              // RecordProcess + SystemStart=false + DecStop + SoftStop
  }
  ```
  > 行為差異須留意：`MachinePause` 開頭 `if(SystemStart==false) return;`，故 idle 時按 Pause 變 no-op（原本會多設一次 `SoftStop=true`，功能上等價）。

- **SECS PAUSE 分支（`uHGemHT160.cpp:723-733`）**：內聯兩行 → `MachinePause(trigSecsRemote);`（HCACK=0 不變）。
  - 收穫：SECS PAUSE 從「無任何 log」變成有 `RecordProcess("MACHINE PAUSE by secs-remote")`，且不違反「EventReport 不進 GEM 層」的既有邊界（`EventReport` 仍留在操作員端）。

- `EventReport` 是 `TfMain::EventReport`（非全域），故**不**放進 `MachinePause`（safety/EMG 也會呼叫 `MachinePause`，不應報 PressPause）。維持由操作員 caller 呼叫。

---

## 4. Slice C — 安全/正確性強化（選配；unhomed-reject 已依決策移除）

> 真實點 1 的「unhomed 拒絕」**不做**（使用者選擇維持 home-then-run）。以下為仍建議的兩項。

### C1. `CheckLotDataReady` 加 active-lot 2D 完整性檢查（對應真實點 2）

`THT160LotRegistry` 目前只有 `GetLotCount()`/`GetItemCount()`（全域），**無 per-lot 計數**。需先在 `CosFunction.h/.cpp` 新增：

```cpp
int THT160LotRegistry::GetItemCountForLot(AnsiString LotID);  // count 2D items whose sLotID == LotID
```
（實作前須確認 `THT160LotRegistry` 內部 item 結構是否帶 `sLotID` 欄位；若 2D item 未綁 lot，此檢查需改以 active lot slot 是否有 item 為準 — 實作時驗證。）

`CheckLotDataReady` 於既有 `GetItemCount()<=0` 檢查後加：
```cpp
if(LotRegistry.GetItemCountForLot(edLotNo->Text)<=0)
{
    Reason="Active lot has no 2D data : load this lot's 2D/Bin before Start !";
    return false;
}
```

### C2. 補 SECS `STOP` / `HOME` RCMD（補 parity，對應真實點 3）

於 `uHGemHT160.cpp` S2F42 dispatch 增加分支（皆於 VCL/HSMS receive thread，與現有命令同）：
- `S=="STOP"` → `MachineStop(trigSecsRemote); HCACK=0;`（hard stop，對應無 SECS 等價的操作員停機）。
- `S=="HOME"` → 複用 `sbHome1Click` 的核心（`fHome->Show()` + `RecordProcess` + `ChangeRunMode(Run_Home)` + `SystemStart=true` + `ArmMotorHome` + `SoftStart=true` + `bHomeByStart=false`）。讓 host 能在 START 前明確 HOME。
- 其餘（CLEANOUT/ONECYCLE）視 KYEC host 命令集需求再補（見 `secs-9045-vs-160-diff` 阻塞點）。

---

## 5. 編碼 / 編譯 / 驗證 Gate（強制）

- **Big5 byte-safe 編輯**：`main.cpp`、`csystem.cpp`、`uHGemHT160.cpp` 為 legacy 來源，**禁用 Edit 工具**（會壞 Big5）；用 `scripts/ops/bcb6-bytesafe-edit.ps1` 或 python latin1 splice（見記憶 `edit-tool-corrupts-big5-source`）。新註解 ASCII English。
- **無 C++11 / 無 FSM**；維持 `AnsiString` 流。
- **Build**：`csystem.h` / `main.h` / `CosFunction.h` 有改 → `scripts/ops/build-ht160s.ps1 -Full`（header 變更，見記憶 `ht160s-build-command`）。
- **真機 build 驗證**（本變更動到 SystemStart 核心、SECS 路徑跨 sim/real 共用）：依 CLAUDE.md Compile Gate — 註解掉 `MachineType.h` 的 `#define SOFT_SIMULATE`，`-Full` 確認 exit 0，再還原並重編。
- **selftest**：`ht160s-home-selftest` skill 確認全機 HOME 不退化（START→home-first 路徑未受影響）。
- **encoding check**：`scripts/ops/check-ht160s-source-encoding.ps1`。

---

## 6. 風險與回滾

| 風險 | 緩解 |
|---|---|
| `Start()` 簽章改動牽動所有呼叫端 | grep `Start()` / `sbStart1Click` / `ScanPanelKeys` 全數列出（已知：1585、1760 兩處 + SECS）；漏改會編譯失敗，由 build gate 攔下 |
| `MachinePause` idle no-op 行為差異 | 已標註；功能等價（idle 停機本就 no-op） |
| `RecordProcess` 文字由 "START pressed" 改 "MACHINE START by <trig>" | log 解析若有依賴舊字串需同步（搜 EventLog 解析端） |
| C1 需 LotRegistry 新 API | 先驗證 item 是否綁 lot；Slice C 可獨立延後 |
| header 變更漏 `-Full` | 既有記憶已警示 |

**回滾**：Slice A/B/C 各自獨立 commit；A 為純結構重構（行為等價），B/C 為行為增強，可逐 slice revert。

---

## 7. 建議執行順序

1. **Slice A**（核心、行為等價）→ build(`-Full`, sim+real) → selftest → commit。
2. **Slice B**（Pause/Stop 接線）→ build → commit。
3. **Slice C**（選配，正確性/parity）→ 先驗 LotRegistry API → build → commit。

> 本文件為計畫；待 review 後再動碼。
