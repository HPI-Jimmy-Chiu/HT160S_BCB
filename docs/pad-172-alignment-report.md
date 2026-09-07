# Pad 介面對 HT172 對齊報告（per-function）

> 範圍：`uPadInterface.cpp` / `ComPort.cpp` 之 Pad 介面（操作面板 RS232）全函式，逐一與
> HT172（`D:\HT172\HT172_Program_V1.0.25.0_20260420\`）對照。
> 規則：本檔僅做分析輸出，**不修改任何原始碼**；HT172 為唯讀參考。所有 `file:line`、識別字保留 ASCII。

---

## 1. 摘要與方法論

### 1.1 結論摘要

本次以「逐函式（per-function）」方式比對 Pad 介面共 ~50 個函式 / RX-TX 路徑，且每個 divergence 皆已做對抗式
verdict 查證。整體結論：

- **Pad frame 解析語意（parse semantics）與 HT172 對齊**：收訊 frame 切割、`SubString(4,1)` 位址、
  `SubString(6,2)` 型別碼、`SubString(8,6)` 6-hex key、`00->DoScanPanelLed`/`20->version`/`90->DoUpdataPadStatus`
  分派，HT160 與 HT172 完全一致。
- **真正可安全對齊 HT172（`shouldAlignToHT172=true`）只有 2 項**，且皆為**非解析、非安全**的健全性/可診斷性
  改善：`SendCommand` 連線失敗時補 `[Connect Error]` log（高信心、零風險）、`OpenCommPort` 補 already-open
  idempotency guard（潛在路徑、目前無 live caller）。
- 其餘行為差異**絕大多數應保留 HT160**：它們不是誤移植，而是 HT160 平台/架構造成的**必要適配**
  （MN200 IO 取代 HT172 step-tray 卡、Bin 顯示獨立 COM、無 FSM、硬體 PAD_PannelEnable 鍵取代
  `iControlPanelMode`、front+rear 雙面板正確分址、HSys sensor/switch 鏡射），或是 HT160 比 HT172 **更正確/更安全**
  （per-address 重算位元遮罩、NULL guard、`StrToIntDef` 防丟例外、`>=13` off-by-one 修正、`Trim()` 真正生效）。
- **只有 1~2 項需人工決策**：`FormShow` 是否新增 `btnEvent->Down=false` 重置（D3，建議做、低風險硬化），以及
  `RS232Init` 開埠失敗是否彈窗（D2，需產品確認 UX、且**不可照抄會停機的** `ShowMyMessage`）。

### 1.2 本「逐函式」法如何避免先前「整體子系統誤判」

先前以「整段子系統」粗看容易誤判，因為 Pad 介面在 HT160 是被**多重平台改寫**的：動態建 UI（無 DFM）、
MN200 IO、Bin 顯示分線、移除 step-tray 卡、無 FSM。整體看會把這些**刻意適配**誤讀成「移植缺漏」或「解析錯誤」。

逐函式法的修正點：

1. **每個函式各自判定 alignmentStatus**（aligned / divergent / ht160-only / ht172-only），不再把整檔貼一個標籤。
2. **把「解析語意」與「操作/結構差異」分開記**：先確認 `SubString` 索引、長度門檻、hex、frame 切割、`\r`/Trim 等
   解析面是否一致；確認一致後，剩餘差異一律歸類為操作/結構，避免把 UI/COM 生命週期差異誤升級為解析 bug。
3. **每個 divergence 強制給 `affectsBehavior` 與 `recommendation`，並逐一做 verdict 反駁查證**（confidence、
   shouldAlignToHT172、實際 file:line 重驗）。多筆 divergence 的「理由」其實有事實錯誤（例如把 HT172 的
   StepTray/Bin 行為描述反了、把 HT172 `FrontPowerOffMouseDown` 誤說成單一鈕），逐函式 verdict 才抓得出來，並仍導向
   正確結論。
4. **ht160-only / ht172-only 一律先 grep 對面樹確認「沒有對應物」**，避免把「平台沒有此硬體」誤當成「漏移植」
   （如 `dmTrayMotor` / `StepTrayComm` / `NormalizePadInputName` / `SearchChangePageButton`）。

---

## 2. 逐函式對齊表

| Function | h160 | Status | #行為差異 | Recommendation |
|---|---|---|---|---|
| NormalizePadInputName | uPadInterface.cpp:18 | ht160-only | 1 | keep-ht160 |
| SyncHSysPadInputStatus | uPadInterface.cpp:25 | ht160-only | 3 | keep-ht160 |
| SyncHSysPadSwitchStatus | uPadInterface.cpp:38 | ht160-only | 3 | keep-ht160 |
| PAD_PTR::SetItem | uPadInterface.cpp:93 | aligned | 0 | keep-ht160 |
| TfPadInterface (ctor) | uPadInterface.cpp:104 | divergent | 0(解析) | keep-ht160 |
| ~TfPadInterface / FormDestroy | uPadInterface.cpp:111 | divergent | 0(解析) | keep-ht160 |
| InitialVariable | uPadInterface.cpp:119 | aligned | 0(解析) | keep-ht160 |
| BuildUI | uPadInterface.cpp:149 | ht160-only | 1(UI) | keep-ht160 |
| BuildPadPage | uPadInterface.cpp:263 | ht160-only | 0 | keep-ht160 |
| AddPadItem | uPadInterface.cpp:277 | ht160-only | 1(Tag) | keep-ht160 |
| SearchChangePageButton | (none) | ht172-only | 0 | keep-ht160 |
| FormShow | uPadInterface.cpp:328 | divergent | 2 | **D1 keep / D3 needs-decision** |
| FormClose | uPadInterface.cpp:343 | divergent | 1 | keep-ht160 |
| sb_PadInterface_ExitClick | uPadInterface.cpp:354 | divergent | 0 | keep-ht160 |
| PadButtonMouseDown | uPadInterface.cpp:360 | ht160-only | 3 | keep-ht160 |
| sb_PadInterface_FrontPowerOffMouseDown | (none) | aligned | 0 | keep-ht160 |
| PadButtonClick | uPadInterface.cpp:377 | aligned | 0 | keep-ht160 |
| RecordLocalStatusCommand | uPadInterface.cpp:388 | ht160-only | 1 | keep-ht160（更正確） |
| IsPadButton | uPadInterface.cpp:410 | aligned | 0 | keep-ht160 |
| IsPadKey | uPadInterface.cpp:425 | aligned | 1 | keep-ht160 |
| GetPadSwitchStatus | uPadInterface.cpp:442 | ht160-only | 0 | keep-ht160 |
| OpenCommPort | uPadInterface.cpp:463 | divergent | 1 | **OCP-1 align-to-172** / OCP-2 keep |
| CloseCommPort | uPadInterface.cpp:487 | divergent | 2 | keep-ht160 |
| ResetComm | uPadInterface.cpp:506 | divergent | 2 | keep-ht160 |
| sb_PadInterface_ManualSendClick | uPadInterface.cpp:512 | divergent | 1 | keep-ht160 |
| ClearLog1Click | uPadInterface.cpp:520 | aligned | 0 | keep-ht160 |
| RequestPadVersion | uPadInterface.cpp:526 | aligned | 0 | keep-ht160 |
| SendSwitchStatus(TBtnPanelLane*) | uPadInterface.cpp:532 | divergent | 1 | keep-ht160（更正確） |
| SendSwitchStatus(AnsiString,bool) | uPadInterface.cpp:551 | divergent | 2 | keep-ht160 |
| SendCommand(AnsiString) | uPadInterface.cpp:575 | divergent | 3 | **D1 align-to-172** / D5,D6 keep |
| SendCommand(vector<Byte>&) | (none) | ht172-only | 1 | keep-ht160（勿移植 bug） |
| DoScanPanelLed | uPadInterface.cpp:600 | divergent | 4 | keep-ht160 / **D3 needs-decision** |
| DoUpdataPadStatus | uPadInterface.cpp:619 | divergent | 5 | keep-ht160 |
| ProcessReceiceData | uPadInterface.cpp:640 | aligned | 2 | keep-ht160 |
| ProcessSendDataNew | uPadInterface.cpp:708 | divergent | 3 | keep-ht160（更正確） |
| ProcessScanKey | uPadInterface.cpp:737 | aligned | 0 | keep-ht160 |
| Main232 | uPadInterface.cpp:749 | divergent | 0(解析) | keep-ht160 |
| RecordCommunication | uPadInterface.cpp:828 | divergent | 0(解析) | keep-ht160 |
| WriteDataToFile | (none) | ht172-only | 0 | keep-ht160 |
| ConfigurePadComm | ComPort.cpp:96 | aligned | 0 | keep-ht160 |
| PadCommReceiveData | ComPort.cpp:393 | divergent | 0(解析) | keep-ht160 |
| RS232Init | ComPort.cpp:293 | divergent | 4 | keep / **D2 needs-decision** |
| OpenWorkFile | ComPort.cpp:254 | divergent | 3 | keep-ht160 |
| StopPadCom | ComPort.cpp:325 | ht160-only | 5 | keep-ht160 |
| StopAllCom | ComPort.cpp:344 | divergent | 4 | keep-ht160 |
| EnsurePadInterface | ComPort.cpp:75 | ht160-only | 4 | keep-ht160 |
| GetSelectedBaud | ComPort.cpp:86 | ht160-only | 1 | keep-ht160 |
| GetWorkFileName | ComPort.cpp:67 | ht160-only | 2 | keep-ht160 |
| PopulateComList | ComPort.cpp:56 | ht160-only | 3 | keep-ht160 |
| SaveWorkFile | ComPort.cpp:244 | divergent | 1 | keep-ht160 |
| spbResetComClick | ComPort.cpp:217 | divergent | 3 | keep-ht160 |
| StepTrayCommReceiveData | (none) | ht172-only | 0 | keep-ht160 |

> 「#行為差異」計入 `affectsBehavior=true` 的 divergence；標 `0(解析)`／`0(UI)` 表示僅結構/UI 差異、不影響解析。

---

## 3. A) 可安全對齊 HT172（shouldAlignToHT172 = true，高信心）

> 共 2 項，皆**非解析、非安全控制路徑**。以下提供具體修法與 file:line；**本報告不代為修改**。
> 若採用，依 CLAUDE.md build gate：刪 `uPadInterface.obj` 後重編。

### A-1. `SendCommand(AnsiString)` — 連線失敗時補 `[Connect Error]` log（confidence 96，risk low）

- **現況**（已實讀確認）：`uPadInterface.cpp:580-582` 先無條件 `RecordCommunication("[Send]", sData)`，接著
  `if(bRs232Ok==false || PadComm==NULL) return;` **靜默返回**，無 HT172 的 `[Connect Error]` 線索。
- **HT172**：`uPadInterface.cpp:475-480` 於 `bRs232Ok==false` 同時記 `[Send]` 與 `[Connect Error] " Send Fail"`。
- **為何安全**：純可診斷性；HT160 的 catch 分支（`uPadInterface.cpp:596`）已會記 `[Send] FAIL`，作者本就要失敗可見，
  靜默早退是疏漏。**務必保留** HT160 特有的 `PadComm==NULL` 判斷（勿照抄 HT172 只測 `bRs232Ok`）。
- **建議修法**（保留頂端 `[Send]`，避免 success 路徑重複；只在失敗分支補一行）：

```cpp
    RecordCommunication("[Send]", sData);
    if(bRs232Ok==false || PadComm==NULL)
    {
        // AI: match HT172 visibility - explicit connect error when Pad link down
        RecordCommunication("[Connect Error]", " Send Fail");
        return;
    }
```

`RecordCommunication(AnsiString,AnsiString)` 簽章在 `uPadInterface.cpp:837`。對 TX/LED 與 `>=8` 版本握手皆無影響。

### A-2. `OpenCommPort()` — 補 already-open idempotency guard（confidence 78，risk low）

- **現況**（已實讀確認）：`uPadInterface.cpp:463-485` 無 already-open 早退，直接進 `PadComm==NULL` 檢查再
  `PadComm->StartComm()`。
- **HT172**：`uPadInterface.cpp:262-263` 以 `if(bRs232Ok) return true;` 為函式首句。
- **為何安全（且目前為休眠路徑）**：唯一呼叫者 `ResetComm()`（`uPadInterface.cpp:509`）一定先 `CloseCommPort()`，
  故現況不會在開啟狀態再 `StartComm()`；此 guard 是對未來「直接呼叫 `OpenCommPort()` 再進入」的硬化，且在現存
  caller 中為 no-op（彼處 `bRs232Ok` 必為 false），不會回歸。`TComm`/spcomm 為獨佔埠，重複開啟會丟
  `ECommsError` 被 catch 誤標 `bRs232Ok=false` 並在健康鏈路上記假 FAIL。
- **建議修法**（插為函式首句，置於 `PadComm==NULL` 檢查之前）：

```cpp
bool TfPadInterface::OpenCommPort()
{
    // AI: align to HT172 - skip redundant StartComm if Pad link already open.
    // Exclusive TComm port: re-StartComm on an open link throws ECommsError,
    // which the catch below would mistranslate into a spurious [Connect] FAIL.
    if(bRs232Ok)
        return true;

    if(PadComm==NULL)
    ...
```

**勿**一併移植 HT172 的 `CreateFile` 探測或 `bPadInterfaceAndTrayStepCommPort` step-tray 分支（HT160 無此硬體）。

---

## 4. B) 需決策（needs-decision）

### B-1. `FormShow` D3 — 是否於開窗時 `btnEvent->Down=false` 重置（confidence 82，risk low，建議對齊）

- **差異**：HT172 `uPadInterface.cpp:143-146` 開窗時把每個 `PadItem[i].btnEvent->Down=false`；HT160 `FormShow`
  （`uPadInterface.cpp:328-341`）無此重置，只在 `FormClose` 清 `bPadStatus[]`。
- **現況為何未爆**：`fPadInterface` 為長生命單例，`btnEvent->Down` 與 `bPadStatus` 在所有寫入路徑成對更新，
  且關窗後 `ProcessSendDataNew` 的 static `bOldPadStatus[]` 差異會在下一個 spin 內把殘留 `Down` 抹為 false，
  故「下次開窗顯示假按下」實務上被自癒。
- **為何仍建議對齊**：自癒是隱性不變量（依賴 static 累加器 + spin 介於關窗/再開窗之間 + Down 不可單獨被設），
  未來任一改動都可能重新引入隱患。HT172 的顯式 on-show 重置可移除此依賴，純加法、idempotent、無 FSM、僅動維護用診斷
  表單（不涉運動 gating）。
- **若採用之 HT160 修法**（注意 HT160 為 lazy build，需 NULL guard 且以 `CheckPadItem` 為界，非 32）：

```cpp
    // AI: align to HT172 FormShow - force all toggle buttons visually UP on show.
    for(i=0; i<CheckPadItem; i++)
    {
        if(PadItem[i].btnEvent!=NULL)
            PadItem[i].btnEvent->Down=false;
    }
```

`FormShow` 目前無區域 `i`，需於函式頂宣告 `int i;`（比照 `FormClose` 於 `uPadInterface.cpp:345`）。
**保留** `FormClose` 既有 `bPadStatus` 清除（兩者並存，如 HT172）。

### B-2. `RS232Init` D2 / 開埠失敗是否彈窗（confidence 95，risk low，建議部分對齊）

- **差異**：HT172 於 `ComPort.cpp:110-115` 開埠前 `GetCOMPortStatus` 預檢 + 失敗 `ShowMyMessage`；HT160
  （`ComPort.cpp:299-317`）僅 try/catch 設 `bRs232Ok=false` 並寫 memo `[Connect] FAIL`，無操作者彈窗。
- **為何不可照抄**：HT160 `ShowMyMessage`（`mymessbox.cpp`）會 `DecStopAllMotor()` + 清 `SystemStart` + 阻塞
  `ShowModal()`。而 Pad 於 `Spin()` 首 tick **無人值守自動開埠**（`ComPort.cpp:360-369`），照抄會把開機時「Pad 埠
  缺失（非安全裝置）」升級成停機式阻塞 modal，比現況更糟，違反早啟動保護。
- **建議（部分對齊、需產品確認 UX）**：
  - 自動開埠路徑（Spin 首 tick）維持**靜默**，不彈窗、不停機。
  - 僅在**操作者主動重連**（`spbResetComClick` → `RS232Init`）時，於失敗分支用**不停機**的
    `ShowMyOKMessageNoStop`（`mymessbox.cpp`）提示，**切勿**用 `ShowMyMessage`（會停機）。
  - 可加 `bool bNotifyOperator` 參數區分二者；預檢可重用既有 `MyBinDisp.cpp:186 GetCOMPortStatus`，**不要**新增自由函式。
- **決策點**：操作者是否要在手動 Reset 失敗時看到對話框。若否，維持 log-only 即可（`bRs232Ok=false` 已 gating 所有
  `SendCommand`，且失敗已持久化到 `g_PadCommLog`，無正確性缺口）。

---

## 5. C) HT160 刻意差異（保留，勿對齊 HT172）

以下為平台/架構/設計造成的**正確差異**，逐一列關鍵 file:line。除非 HT160 硬體日後改變，否則皆 keep-ht160。

### C-1. `iControlPanelMode` → 硬體 PAD_PannelEnable 鍵（無 FSM 適配）

- HT172 以 `iControlPanelMode==1` gate Pad 鍵；HT160 以**硬體 enable 鍵**取代（PadButtonDefs idx2 `SwFrontActiveLed`
  / idx30 `SwRearActiveLed`，皆 `PAD_PannelEnable`，`uPadInterface.cpp:60-90`）。
- `IsPadKey` 因此**無** HT172 的 `iControlPanelMode==1` caller gate；`ProcessSendDataNew`/`ResetComm` 的 COM 自癒
  亦不採 HT172 in-function FSM（`switch(iTask)` + 550ms `tSendDataDelay`），改為程序式（`uPadInterface.cpp:708-744` /
  `Main232` `case 1/10/20/50`）。符合 HT160「No FSM」鐵則。

### C-2. 已修正的 `>=13` 長度門檻（off-by-one，勿回退到 HT172 的 `>=14`）

- `ProcessReceiceData` 的 `00`/`90` 分支 gate `Frame.Length()>=13`（`uPadInterface.cpp:680,697`）；HT172 為
  `aReciveData.Length()>=14`（HT172 `uPadInterface.cpp:645,658`）。
- HT160 先 `Frame=Frame.Trim()`（`uPadInterface.cpp:670`）剝掉尾端 `\r`，故真實 payload 13 字元
  （addr@4、type@6-7、key@8-13），`>=13` 為**正確**門檻；HT172 把仍含 `\r` 的緩衝測成 `>=14`，會靜默丟掉合法 13-char
  frame。版本回覆 `>=8` 門檻兩邊相同。**勿回退**。

### C-3. MN200 IO 平台（移除 step-tray 卡 / Bin 顯示獨立 COM）

- 整個 step-tray 串列馬達子系統在 HT160 **不存在**（grep `dmTrayMotor`/`StepTrayComm`/`comTrayStepMotor` 於
  `D:\HT160S_BCB` 為 0 命中）。故：
  - `PadCommReceiveData`（`ComPort.cpp:393`）只處理 `t05`，**無** HT172 的 `t07` step-tray 分支（HT172
    `ComPort.cpp:261-322`）；`StepTrayCommReceiveData` 為 ht172-only，無對應物。
  - `SendCommand`/`OpenCommPort`/`CloseCommPort` 皆**不**含 `bPadInterfaceAndTrayStepCommPort` 共埠分支。
- Bin 顯示走**獨立** `BinComm` / `HSys.BinDisCtrl`（`ConfigureBinDisplay`，`ComPort.cpp:111-145`）：
  - `StopPadCom`（`ComPort.cpp:325`，ht160-only）只關 Pad 線；`StopAllCom`（`ComPort.cpp:344`）才連 Bin，
    且**僅** dtor（`ComPort.cpp:53`）呼叫。重連/Stop 走 `StopPadCom`，**Pad 重連不會拆掉 Bin 顯示**。
  - 對應 HT172 的 `StopAllCom`（HT172 `ComPort.cpp:232-238`）實際只關 Pad+StepTray、Bin 行為被註解；逐函式
    verdict 已抓出 divergence 理由文字把 HT172 行為描述反了，但結論 keep-ht160 不變。**陷阱**：HT160 與 HT172 都有
    `StopAllCom` 但**範圍不同**，若天真「對齊」把 HT160 重連改呼叫 `StopAllCom` 會每次重連都拆掉 Bin 顯示，是回歸。

### C-4. HT160 比 HT172「更正確 / 更安全」者（保留）

- **per-address 重算位元遮罩**：`RecordLocalStatusCommand`（`uPadInterface.cpp:388-408`）每次 `iStatus=0` 依
  `btnEvent->Tag==iAddress && Down` 重算；`ProcessSendDataNew` 依 Tag 拆 front(0x0)/rear(0x1) 各送一框
  （`uPadInterface.cpp:717-744`）。HT172 `ProcessSendDataNew` 把 front+rear OR 進一框並**硬寫 PAD_RearControl**
  （HT172 `uPadInterface.cpp:710`），front LED 永遠點不亮且 rear 字混入 front 位元 —— HT160 為修正。
- **`SendSwitchStatus(TBtnPanelLane*)`**：HT160 用 per-address live 重算，避免 HT172 該 overload 的 static
  累加器跨 front/rear 位元洩漏（且該 HT172 overload 為 dead code，無 caller）。
- **NULL guard / 防例外**：`PadButtonMouseDown`、`PadButtonClick`、`CloseCommPort`、`StopPadCom`、`SendCommand` catch、
  `RecordCommunication` 的 `Memo_PadInterface==NULL` 早退（`uPadInterface.cpp:841`）。
- **`StrToIntDef("0x"+aPadKey,0)`**（`uPadInterface.cpp:686,703`）取代 HT172 `StrToInt`，亂碼 frame 不丟例外。
- **`Frame=Frame.Trim()` 真正生效**（`uPadInterface.cpp:670`）；HT172 `aReciveData.Trim()`（HT172
  `uPadInterface.cpp:669`）丟棄回傳值為 no-op。
- **Pad baud、port 可選並持久化**：`GetSelectedBaud`（`ComPort.cpp:86-94`）預設 `PAD_DEFAULT_BAUD=115200`
  與 HT172 一致；`OpenWorkFile`/`SaveWorkFile` 以 INI `[Pad] CommName/BaudRate` round-trip。
  （註：combo 仍可選 9600/19200/...，Pad 韌體固定 115200；若要硬化可在 `GetSelectedBaud` 把未知 baud clamp 回 115200，
  或將 `cbPadBaud->Style=csDropDownList`，屬可選非必要。）

### C-5. ht160-only 補償層 / 鏡射（必要，無 HT172 對應物可對齊）

- **`NormalizePadInputName`**（`uPadInterface.cpp:18`）：HT160 多一顆 `SnRKTray` sensor（`database.cpp:989`、
  `IO_Table.csv:255`、DFM `ml_RKTray`），此 shim 把 `SnRKTray`→`SnRKTrayEnd` 正規化，供
  `IsPadKey`/`ProcessScanKey`/`SyncHSysPadInputStatus` 一致解析 Tray-End 鍵。HT172 無此 sensor、無此函式。**移除會靜默
  丟失後面板 Tray-End 復位鍵**（透過 `mysensor.cpp` IsOn/IsOff → ProcessScanKey 路徑）。
- **`SyncHSysPadInputStatus` / `SyncHSysPadSwitchStatus`**：把 Pad input/switch 鏡射進 HSys（flat
  `SenPtr[]`/`SwPtr[]` 索引模型），HT172 為具名 struct 成員 + 直讀 `mlEvent->Value`，**無 HSys 橋**。
  switch 鏡射的真正負載消費者是 iosetview backup/restore（`iosetview.cpp:1773`，非 live view 的 `:1227`）。
- **動態 UI**：`BuildUI`/`BuildPadPage`/`AddPadItem`（無 DFM）取代 HT172 靜態 DFM + `SearchChangePageButton`；
  31 筆 `PadButtonDefs[0..30]` 之 `(PadName, iData, InputName, PanelTag)` 與 HT172 `SetItem` 逐索引等價，
  `CheckPadItem=31` 兩邊相同。`PadButtonMouseDown` 為通用 MouseDown 直接 toggle+送，等價於 HT172 的
  DFM-toggle(MouseDown)+OnClick(send) 兩段，且 HT160 多 NULL guard。
- **`bPadEverCommunicated`**（`uPadInterface.cpp:689,704`）：Pad 存活旗標，餵 `CheckMotorPowerShutDown`
  開機自激磁路徑，取代 HT172 `DelayMotNo>=100` 啟動計數。為 HT160 啟動 grace 的等價再實作，**勿移除**（移除會讓開機時
  馬達電源無法自動上電，直到操作者實體按 Power On）。
- **`RecordCommunication` 改用 `g_PadCommLog` 每日 CSV**（`uPadInterface.cpp:851`、`cCommLog`）取代 HT172
  `WriteDataToFile` 每分鐘 .txt；RX 亦統一從 `ComPort.cpp:390` 進同一 CSV。具 CsvQuote + 保留天數 + 臨界區鎖，**較 HT172
  更佳**（`SendCommand(vector<Byte>&)` 那個 HT172-only overload 帶 strlen 未 NUL-terminate 的 latent bug，**勿移植**）。

---

## 6. 結語

- **唯二建議實作**：A-1（`SendCommand` 連線錯誤 log）、A-2（`OpenCommPort` idempotency guard）。其餘皆 keep。
- **需人工拍板**：B-1（`FormShow` 顯式重置，建議做）、B-2（手動 Reset 失敗彈窗 UX，需產品確認、勿用停機式
  `ShowMyMessage`）。
- 任何採用後務必依 build gate 刪 obj 重編，並跑 `check-ht160s-source-encoding.ps1`；新增註解一律 ASCII English。
