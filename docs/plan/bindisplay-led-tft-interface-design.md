# Bin Display LED/TFT 介面層擴充設計（草案，未實作）

- 狀態：**框架 + TFT frame builder + 維護頁面板選擇器已實作並 build clean（T1+T2+T3 builders+UI，2026-06-23）**；
  剩 TFT 狀態機 `ProcessTick`（T4，串接 builders）待面板硬體。見 §7/§8。
- 範圍：把現有 `MyBinDisp` 從「單一 LED 子類」擴充成「抽象基底 + LED / TFT 兩個兄弟子類」，
  讓 LED（HT9046 Modbus-ASCII）與 TFT（HT9011 繪圖面板）共用同一對外介面、各自實作協定，
  **不採用** 9045 的 `NUMBER_PANEL_TYPE` 散落分支模型。
- 相關：skill `.github/skills/ht160s-bindisplay/SKILL.md`、memory `bindisplay-port-plan.md`。
- 不動原則：本草案 **不修改任何 `.cpp/.h/.dfm`**；T1 起才動 source（每階段獨立可編譯）。

---

## 1. 為什麼需要這個設計：LED 與 TFT 是兩套協定

| 面向 | LED（HT9046） | TFT（HT9011 面板） |
|------|----------------|--------------------|
| 框架 | Modbus-ASCII：`:` + 12 ASCII 字元 + 2 字元 LRC + CRLF | 20-byte 二進位：`0x3A` + addr + ByteCount(2) + FuncCode(2) + DataItem(2) + 9 data byte + LRC + CRLF |
| 定址 | 暫存器 `0080`(數字/版本) / `0081`(字母) / `0082`(顏色) | FuncCode `0x02`=字型/版面、`0x03`=文字；DataItem 區分欄位；位址另用 `iAddArrayTFT[]` |
| 內容 | 單一值：一個數字/字母 + R/G 燈 | 真字串（`"Empty"`/`"Loader"`/`"Color"`）+ XY 座標 + 寬高 + RGB + 字體大小 + 填滿 |
| 每 unit 命令 | 1~2 筆（color、bin） | 多筆：背景 → 字型設定 → Bin 字 → "Bin" 字 → "EA" → Count |
| 串列鏈 | 單鏈 `CommBin` | 9045 用雙鏈 `CommBin` / `CommBin2` |
| LRC 函式 | `A_Create_LCR`（HT160 現有） | `A_Create_LRC`（9045；演算法同 LCR，命名不同） |

證據（read-only 參考，905.8）：
- TFT 框架組裝：`BinDisplay/MyBinDisp.cpp` `command_TFT_Input`（FuncCode 0x03，文字）、
  `command_TFT_Font`（FuncCode 0x02，字型/版面）。
- LED 框架：HT160S `MyBinDisp.cpp` `WriteBin`/`WriteColor`/`ReadVersion`（§3 已在 skill 詳列）。

**結論：本質是兩個 protocol，不能用同一組 frame builder 套參數解決。**

---

## 2. 現況與 9045 的反例

### HT160S 現況（已切好一半的介面層）
移植時已把 9045 的肥子類拆成乾淨的介面層（`MyBinDisp.h`）：
- 抽象基底 `TMyBinDispCtrl`：transport + per-unit 狀態 + 對外 API + `Spin()` pump。
- 純虛擬協定方法：`WriteBin/WriteColor/ReadVersion/DoStartSetBin/DoStartSetColor/DoStartGetStatus`。
- 唯一子類 `TMyBinDispHT9046`（LED）。

**但** `Spin()` 的狀態機本體（case 1 開埠 → 50 GetStatus → 100 分派 → 200 color → 300 bin）
**寫死在基底**，而且那組純虛擬是「LED 形狀」的（單值暫存器寫入）。TFT 套不進去。

### 9045 的做法（反例，不要照抄）
9045 把 LED + TFT 塞進**同一個** `TMyBinDispHT9046`，靠 `NUMBER_PANEL_TYPE==4`
（再加 `MAGAZINE_BIN_DISP_TYPE==eTFT`）在整個狀態機散落 15+ 個 `if/else`：

```
// 9045 BinDisplay/MyBinDisp.cpp Timer1Timer() case 100 分派（節錄）
case 100:
    ...
    if(NUMBER_PANEL_TYPE==4) {            // TFT 路徑
        if(bStartOnce){ iOnceTask=1;  Task=400; }   // 400: DoOnce  一次性版面/背景/字型
        else if(bStartCycle){ iCycleTask=1; Task=500; } // 500: DoCycle 週期更新 bin/EA/count
    } else {                              // LED 路徑
        if(bStartSetColor){ iStartSetColorTask=1; Task=200; }  // 200: color
        else if(bStartSetBin){ iStartSetBinTask=1; Task=300; } // 300: bin
    }
```

問題：每加一種面板就要在 case 1 / 100 / 200 / 300 / 400 / 500 / 600 各補一段 `if`，
協定邏輯交織、難測難維護。HT160 已經有抽象基底，沒有理由退回這個模型。

---

## 3. 目標架構：ProcessTick() 介面切點 + 兩個兄弟子類

```
TMyBinDispCtrl            (抽象基底；對外只認這顆指標 HSys.BinDisCtrl)
  - transport / 狀態 / 對外 API / Spin() 的 run-gate
  - virtual void ProcessTick() = 0;   <-- 唯一的跨協定虛擬
        |
        +-- TMyBinDispHT9046   (LED：Modbus-ASCII，現有；ProcessTick 跑 1/50/100/200/300)
        +-- TMyBinDispTFT      (TFT：20-byte 繪圖，新增；ProcessTick 跑 1/50/100/400/500/600)
```

關鍵改動 **把整個 tick 狀態機變成虛擬**：
- 基底 `Spin()` 只保留 run-gate（`InitialOK` / `bStopProcess` / `bTimerRun` 重入鎖），
  然後呼叫 `ProcessTick()`。
- 各子類在自己的 `ProcessTick()` 跑完整的 `switch(iBinDispCtrlTask)`。
- **LED 形狀的純虛擬（WriteBin/.../DoStartGetStatus）不再放在基底** —— 它們只被 LED 的
  ProcessTick 呼叫，所以下放成 `TMyBinDispHT9046` 的私有成員。基底因此真正與協定無關。
- 對外 API（`WriteTargetBin`/`SetUnitLabel`/`GetBinNow`...）表達「意圖」（要哪個 unit 顯示什麼），
  留在基底；兩個子類的 ProcessTick 各自把意圖實現成自己的 frame。

### 共用 vs 下放（誰住哪）

| 項目 | 位置 | 說明 |
|------|------|------|
| `CommBin` / COM 開關 / `GetCOMPortStatus` / `StartComport` / `StopComport` | 基底（protected 工具） | 兩子類都呼叫 |
| `T_HEX2ASCII_Mac` / `T_ASXII2HEX_Mac` / `A_Create_LCR` / `Chararr2Hexstring` | 基底（protected 工具） | LRC/HEX 通用 |
| per-unit 意圖狀態：`iSetBin/bSetBin/iSetColor/bSetColor/bGetStatus/iVersion/iBinNow/iColorNow/bHasError/Alias/bHasUnit*/bSliding/iDelaySec` | 基底 | 協定無關的「要顯示什麼」 |
| 對外 API：`WriteTargetBin`/`SetUnitLabel`/`SetUnitBin`/`SetUnitColor`/`InstalledUnit`/get-set/log | 基底 | 介面契約 |
| `Spin()` run-gate + `ResetBinFlow` | 基底 | 呼叫 `ProcessTick()` |
| `virtual void ProcessTick()=0` | 基底（純虛擬） | **唯一跨協定虛擬** |
| `WriteBin/WriteColor/ReadVersion/DoStartSetBin/DoStartSetColor/DoStartGetStatus` | **下放 LED 子類** | LED 專屬，不再是基底契約 |
| LED 的 `ProcessTick`（case 1/50/100/200/300 本體，即現 Spin 內容） | LED 子類 | 行為不變 |
| `CommBin2`（第二串列）、`iAddArrayTFT[]`、`iCountTFT/bStartOnce/bStartCycle/iOnceTask/iCycleTask/sUnitName/iSetCount` | **TFT 子類** | TFT 專屬狀態 |
| `command_TFT_Input/Font`、`A_Create_LRC`、`WriteBin_TFT/WriteBinWord_TFT/WriteEA_TFT/WriteCount_TFT`、`SetFont*_TFT/SetBackGround_TFT/SetNoBackGround_TFT`、`ReadVersion_TFT`、`DoOnce/DoOnceTFT/DoCycle/DoCycleTFT`、`InitialTask` | TFT 子類 | 從 9045 移植 |
| TFT 的 `ProcessTick`（case 1 雙埠開 / 50 / 100 分派 / 400 DoOnce / 500 DoCycle / 600） | TFT 子類 | 從 9045 重寫成 HT160 程序式 |
| Magazine（HTA18/BT008/TFT） | **不移植** | HT160 無 magazine（沿用既有 lock） |

---

## 4. 提案後的 MyBinDisp.h（設計，ASCII 註解）

> 僅為目標形狀示意；成員清單以現有 `MyBinDisp.h` 為準，差異只在 (a) 加 `ProcessTick()`、
> (b) LED 純虛擬下放、(c) 新增 `TMyBinDispTFT`。基底其餘成員維持原樣。

```cpp
//----------------------------------------------------------------------------
// Abstract base: per-unit bin display state machine driven by Spin().
// Spin() only run-gates and delegates one tick to ProcessTick(); each concrete
// protocol implements its own task switch. The base is protocol-agnostic.
//----------------------------------------------------------------------------
class TMyBinDispCtrl
{
protected:
    // --- shared transport / helpers (both subclasses call these) ---
    bool GetCOMPortStatus(AnsiString Com);
    bool StartComport(TComm *Comm, AnsiString port);
    bool StopComport (TComm *Comm, AnsiString port);
    unsigned char T_HEX2ASCII_Mac(unsigned char hex2ascii);
    unsigned char T_ASXII2HEX_Mac(unsigned char ascii2hex);
    unsigned char A_Create_LCR(unsigned char *Sptr, unsigned char length);
    AnsiString    Chararr2Hexstring(char* cstr, int iNum);
    void LogBinDisplay(AnsiString asAction, AnsiString asMessage, bool bMemo);

    // --- shared per-unit intent state (protocol-agnostic "what to show") ---
    int   Addr;
    HTimer BinDisDelay;
    bool   bTimerRun;
    bool  bHasUnitArray[Bin_MAX_NUM];
    bool  bHasUnit;
    bool  bSliding[Bin_MAX_NUM];
    bool  bStopProcess;
    bool  bSetBin[Bin_MAX_NUM];
    int   iSetBin[Bin_MAX_NUM][TEST_MAX_BIN];
    bool  bSetColor[Bin_MAX_NUM];
    bool  bGetStatus[Bin_MAX_NUM];
    int   iSetColor[Bin_MAX_NUM];
    bool  bStartSetBin;
    bool  bStartSetColor;
    int   iDelaySec;
    int   iVersion[Bin_MAX_NUM];
    int   iBinNow[Bin_MAX_NUM];
    int   iColorNow[Bin_MAX_NUM];
    bool  bHasError[Bin_MAX_NUM];
    int   iRusStatus;
    char  SendBuffer[1024];
    bool  BinDispRecv;
    char  BinDispCom2Buffer[1024];
    AnsiString ComPort;
    int   iBinDispCtrlTask;          // pumped by the subclass ProcessTick switch
    int   iTotalInstalledUnit;
    int   iTestBinCount;
    int   iErrCount[Bin_MAX_NUM];
    int   iCount[Bin_MAX_NUM];
    int   iUsedBinNumber;

    // --- the ONLY cross-protocol virtual: one tick of the state machine ---
    virtual void ProcessTick() = 0;

public:
    TMyBinDispCtrl();
    virtual ~TMyBinDispCtrl();       // virtual dtor (polymorphic delete)
    AnsiString Alias[Bin_MAX_NUM];
    TComm *CommBin;
    TParity ComParity;
    // ... 對外 API 全部維持現狀（ProcessStopStart / SetComPort / WriteTargetBin /
    //     SetUnitLabel / SetUnitBin / SetUnitColor / InstalledUnit / get-set /
    //     CommBinReceiveData / SetUsedBinNumber / ResetBinFlow / log ...）
    void Spin();                     // run-gate only, then ProcessTick()
    bool InitialOK;
    bool bFirstInit;
    TStringList *slBinDispLog;
    AnsiString  sReadBuffer;
};

//----------------------------------------------------------------------------
// Concrete LED protocol (HT-9046 Modbus-ASCII). Behavior unchanged from today;
// the former Spin() body becomes ProcessTick(), and the LED-shaped methods that
// used to be base pure-virtuals are now private members here.
//----------------------------------------------------------------------------
class TMyBinDispHT9046 : public TMyBinDispCtrl
{
protected:
    virtual void ProcessTick();      // case 1/50/100/200/300 (was base Spin body)
private:
    void WriteBin  (int Addr, int Command, short Value);
    void WriteColor(int Addr, short Value);
    void ReadVersion(int Addr);
    bool DoStartSetBin();
    bool DoStartSetColor();
    bool DoStartGetStatus();
};

//----------------------------------------------------------------------------
// Concrete TFT protocol (HT-9011 graphic panel). 20-byte binary frames; richer
// per-unit composition (background/font/Bin/EA/Count). Ported from HT9045.
//----------------------------------------------------------------------------
class TMyBinDispTFT : public TMyBinDispCtrl
{
protected:
    virtual void ProcessTick();      // case 1(dual-COM)/50/100/400 DoOnce/500 DoCycle/600
private:
    // second serial chain + TFT addressing
    TComm *CommBin2;
    int    iAddArrayTFT[Bin_MAX_NUM];
    bool   bStartOnce, bStartCycle;
    int    iOnceTask, iCycleTask, iOnceTFTTask, iCycleTFTTask;
    int    iCountTFT[Bin_MAX_NUM], iSetCount[Bin_MAX_NUM];
    AnsiString sUnitName[Bin_MAX_NUM];
    // frame builders (base layer in 9045)
    unsigned char A_Create_LRC(unsigned char *Sptr, unsigned char length);
    void command_TFT_Input(char *cStr, int index, int iDisplabel, AnsiString sValue);
    void command_TFT_Font (char *cStr, int index, int iDisplabel, Byte X, Byte Y,
                           Byte W, Byte H, Byte FontSize, Byte Fill, int iColor);
    // per-field writers
    void ReadVersion_TFT(int index);
    void SetBackGround_TFT(int index);
    void SetNoBackGround_TFT(int index);
    void SetFontBin_TFT(int index, int iColor, int iValue);
    void SetFontBinWord_TFT(int index, int iColor, int iValue);
    void SetFontEA_TFT(int index, int iColor, int iValue);
    void SetFontCount_TFT(int index, int iColor, int iValue);
    void WriteBin_TFT(int index, int ivalue);
    void WriteBinWord_TFT(int index, int ivalue);
    void WriteEA_TFT(int index, int ivalue);
    void WriteCount_TFT(int index, int ivalue, int iCount);
    // sub state machines
    bool DoOnce();
    bool DoOnceTFT(bool bReset, int iSetType);
    bool DoCycle();
    bool DoCycleTFT(bool bReset, int iWriteType);
    void InitialTask();
};
```

對 `Spin()` 的最小改寫（基底）：
```cpp
void TMyBinDispCtrl::Spin()
{
    if(InitialOK==false) return;
    if(bStopProcess==false) return;
    if(bTimerRun) return;
    bTimerRun = true;
    ProcessTick();           // subclass owns the switch(iBinDispCtrlTask)
    bTimerRun = false;
}
```

---

## 5. 面板型別選擇（工廠 + 設定檔）

`General.ini [BinDisplay]` 新增 key：
```
PanelType=0     ; 0 = LED (HT9046, default)   1 = TFT (HT9011)
```
`GeneralSetting.h/.cpp` 加 `int iBinDispPanelType`（SetDefault=0 / Load / Save）。

`database.cpp`（現為 `BinDisCtrl = new TMyBinDispHT9046;`，ctor ~617）改成工廠：
```cpp
switch(GeneralSetting.iBinDispPanelType)
{
    case 1:  BinDisCtrl = new TMyBinDispTFT;    break;   // TFT panel
    default: BinDisCtrl = new TMyBinDispHT9046; break;   // LED (HT9046)
}
```
其餘整合點（`ComPort.cpp ConfigureBinDisplay`、`maintenance.cpp` 維護頁、`TfComPort::Spin`）
**完全不動** —— 它們只透過基底指標 `HSys.BinDisCtrl` 操作，LED↔TFT 切換零分支。

---

## 6. TFT ProcessTick 流程（從 9045 重寫，HT160 程序式）

```
case 1   : 開 CommBin (+CommBin2)；TFT 設 Timer interval=30、ReadIntervalTimeout=50；
           bStartOnce=bStartCycle=true；-> case 50
case 50  : DoStartGetStatus()（內含 ReadVersion_TFT）-> case 100
case 100 : 斷線重設檢查；然後
             if(bStartOnce){ -> case 400 }     // 一次性版面
             else if(bStartCycle){ -> case 500 } // 週期更新
case 400 : DoOnce()  逐 unit 鋪背景 + 字型設定 (SetBackGround/SetFont*)；done -> bStartOnce=false, case 100
case 500 : DoCycle() 逐 unit 寫 Bin 數字 + "Bin"/"EA"/Count；done -> bStartCycle=false, case 100
case 600 : （magazine-TFT 用，HT160 不需要 -> 不實作）
```
`A_Create_LRC` 計算範圍 = `&cStr[1]` 起 16 bytes（含 addr ~ data[8]，不含 header）。
TFT 顏色：1=R(0xFF0000)、2=G(0x228B22)、3=橙(0xFF8000)、4=透明底灰字。

---

## 7. 分階段移植計畫

| 階段 | 內容 | 相依硬體？ | 狀態 |
|------|------|-----------|------|
| **T0** | 本設計 + §8 決策 | 否 | ✅ DONE |
| **T1** | **純重構**：基底 `Spin()` 抽出 `ProcessTick()` 純虛擬；LED 純虛擬下放 `TMyBinDispHT9046`；virtual dtor；行為零變更 | 否 | ✅ DONE (build clean 2026-06-23) |
| **T2** | 加 `TMyBinDispTFT` 骨架（ProcessTick no-op stub + `GetPanelKind()=1`）+ `ConfigureBinDisplay` 工廠/swap + `PanelType` key（GeneralSetting）；PanelType=1 選到靜默 controller | 否 | ✅ DONE (build clean 2026-06-23) |
| **T3** | 移植 TFT frame builder：`command_TFT_Input/Font`、`WriteBin/BinWord/EA/Count_TFT`、`SetFont*_TFT`、`SetBackGround/NoBackGround_TFT`、`ReadVersion_TFT`、`ShowCommLog`、`iAddArrayTFT`（單鏈；reuse base `A_Create_LCR`；`MyASCIIToDec`→`(unsigned char)cast`） | 否 | ✅ DONE (build clean 2026-06-23) |
| **UI** | tsMaintHardware 加 `cbBinPanelType` 下拉（Panel3 內，預設 LED；Load/Save 綁 `iBinDispPanelType`） | 否 | ✅ DONE (build clean 2026-06-23) |
| **T4** | 實作 `TMyBinDispTFT::ProcessTick`（單埠開 + DoOnce/DoCycle 串接 builders）；review `SetBackGround_TFT` 的 `index<3` 佈局假設（HT160 為 Empty=0/Loader=1/Color=8） | 是（需面板） | ⬜ 待硬體 |
| **T5** | 維護頁手動測試擴充（依 PanelType 切 LED/TFT 測試鈕） | 是 | ⬜ 待硬體 |
| **T6** | 上機 bring-up + 驗證（ReadVersion 回應、版面、Bin/EA/Count 更新） | 是 | ⬜ 待硬體 |

**T1+T2+T3(builders)+UI 已完成**：基底協定無關（唯一跨協定虛擬 `ProcessTick()`）；LED 行為不變；
`TMyBinDispTFT` 兄弟子類含完整 20-byte frame builder（離線移植、可編譯，未被呼叫）；`ProcessTick`
仍為 no-op stub（T4 串接）。`PanelType` 由 tsMaintHardware 的 `cbBinPanelType` 下拉選（預設 LED），
存入 `General.ini [BinDisplay] PanelType`，`ConfigureBinDisplay` factory 依此 swap 子類。
**T4 待面板硬體**：寫 `ProcessTick` 狀態機呼叫 builders，並在串接時 review `SetBackGround_TFT`
的 `index<3` 佈局假設（沿用 9045，HT160 佈局不同，類同先前 LED Color-letter 修正）。

---

## 8. 決策（使用者確認 2026-06-23）

1. **要不要上 TFT 面板？** → **未來會上**。先把框架做出來（T1+T2 已完成），T3 起待面板硬體。
2. **單鏈還是雙鏈（CommBin2）？** → **單串列鏈**。TFT `ProcessTick` case 1 只開 `CommBin`，
   不移植 `CommBin2`、`CommBinReceiveData2`、`ComPort2` 整段。
3. **內容多豐富？** → **Bin 數字 + "EA" + IC Count**（完整）。移植 `WriteEA_TFT` /
   `WriteCount_TFT` / `iSetCount` / `iCountTFT`。注意 HT160 目前 bin 顯示為靜態標籤
   （無 per-IC live update，port-plan P5=no-op）；若 Count 要動態更新，T4 需在 sort flow
   加一個 `WriteTargetCount`-類 hook（屬 T4/T5 範圍，屆時再定）。
4. **Magazine（HTA18/BT008/TFT）** → **不移植**（維持既有 lock）。

---

## 9. 約束（必守）

- no-FSM：用程序式 `switch(iBinDispCtrlTask)`，**不要** FSMRunner/transition table/`*Step.h`。
- no C++11：不用 `auto`/`nullptr`/lambda/range-for/`enum class`；維持 `AnsiString` 流。
- 編碼：BCB6 source 為 Big5，新註解用 **ASCII 英文**；改 `.cpp/.h` 用 byte-safe 工具
  （`scripts/ops/bcb6-bytesafe-edit.ps1`），勿用會重編碼的編輯器。本 docs 為 UTF-8。
- 編譯 gate：每次改 `.cpp/.h/.dfm` 後刪對應 `.obj` 再編；結構/接線變更跑
  `scripts/ops/build-ht160s.ps1 -Full`；TFT 雙埠或共用核心碼受 `SOFT_SIMULATE` 影響時，
  另驗實機 build（註解掉 `SOFT_SIMULATE` 跑 `-Full` 確認 exit 0 後還原）。
- 寫入邊界：只改 `D:\HT160S_BCB`；9045 / HT172 為 read-only 參考。
- 改 TFT 命令前，先回讀 9045 對應 frame 確認格式再對齊。
