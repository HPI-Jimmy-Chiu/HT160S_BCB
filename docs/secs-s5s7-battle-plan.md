# SECS S5/S7 作戰計畫 (Battle Plan)

> HT160S_BCB — S5 警報 (Alarm) 與 S7 配方 (Recipe / Process Program) 子系統
> 文件性質：**備用設計 (preparation only)**，非立即實作。

---

## 1. 摘要與狀態

本文件為 HT160S SECS/GEM 之 **S5 (警報) 與 S7 (配方)** 子系統的定案作戰計畫。

**狀態 (CONTEXT)：**
- 目前**無真實 host / EAP**、**無進行中的 SECS 里程碑**。本文件僅為**前置設計與實作地圖**。
- 任何**會觸及運轉中機台的行為**（S5F1 主動推播）一律**編譯關閉 (`#ifdef` off)**，待真實里程碑再啟用。
- 所有實作須遵守專案鐵則：**HT172 唯讀**、**no-FSM**、**ASCII 新註解**、**AnsiString flows**、**no C++11**、**BCB6 序列 build gate**。
- 本文件為**唯一產出**；**不修改任何原始碼**。

**核心結論（兩個子系統的選定路線）：**

| 子系統 | 選定路線 (spine) | 嫁接 (grafts) | 本期交付 |
|---|---|---|---|
| **S5** | Approach #3：TList 警報目錄 + 以 ALID 為鍵的持久化 Enable 儲存；S5F5/F6 列表 + S5F3/F4 啟停 上線；S5F1 推播實作但編譯關閉 | (A) 沿用既有 `SetAlamData` 簽章 (#1/#2)；(B) Read→seed→Write 合併順序與 ALID-keyed 穩健性 (#2)；(C) 目錄誠實 — 僅 cylinder/motor/sucker (#1) | host 可**跨重開機**列出並啟停警報；唯一觸及活機台的 S5F1 維持編譯關閉 |
| **S7** | Approach #1：僅上傳 (upload-only) S7F5/S7F6 | 真實 S7F2 grant ladder (#3)；section-parsed body 紀律 (#2)；**不**新增 SECS primitive (S9F7/LocalAcknowledge 不做)；**不**做 S7F3/F4 下載 | host 可執行 upload-inquire (S7F1→S7F2 grant) 與配方上傳 (S7F5→S7F6)，對既有資料夾**唯讀** |

---

## 2. 前置決策表 (Open Decisions)

> 以下決策已有「預設方向」，但部分需在真實 host 整合前與 EAP 確認。實作前請逐項複核。

### S5 決策

| 代號 | 議題 | 預設方向 / 開放點 |
|---|---|---|
| **D0** | **持久化深度** | 持久化 per-ALID Enable 跨重開機（#1 無法滿足的真實 host 需求）。開放：`AlarmEnable.def` 放 `<root>\SECS\SYSTEM`（THGem `CurrentDirectory` 錨點，預設）還是 `<root>\system`（操作員可見、與 `AlarmList.csv` 同處）。 |
| **D1** | **S5F1 推播 go-live** | 推播已實作但 `#ifdef SECS_S5F1_PUSH` 關閉，`note.cpp` 觸發延後。開放：哪個里程碑啟用旗標 + 連線 `note.cpp` 觸發點？是否需要實際會消費 S5F1 的 customer/EAP？ |
| **D2** | **目錄涵蓋範圍** | HT160 僅建 cylinder(4)/motor(5)/sucker(6)，**無** system(3)/sensor。開放：目標 host 是否預期 type-3/sensor ALID？若是，需在 `CreateSystemAlarmCode` (database.cpp) 新增分支，**超出 SECS 層範圍**，須先回報整合。 |
| **D3** | **ALID 線上型別** | S5F6（兩分支）與 S5F1 統一用 **I8/`__int64`**，解決 HT172 的 INT_8-vs-UINT_4 不一致。開放：確認目標 host parser 接受 I8。 |
| **D4** | **錯誤輸入回覆策略** | 畸形 S5F5 → 回 well-formed 空 `L,0`（無 T3 timeout）+ `StringOut` 診斷。開放：是否有 host 要求顯式 error stream？（需真實 S9F3 encoder 或新 S9F7 primitive）— S9F7 primitive 延後至 S7 設計。 |
| **D5** | **ALID = atoi(code) 耦合** | ALID 由 `"%d%03d%1d"` code 字串導出，改格式會悄悄改變每個 host 已學到的 ALID。開放：鎖定 code 字串格式為 host-facing 契約，或維護顯式 code→ALID 對照表。 |

### S7 決策

| 代號 | 議題 | 預設方向 / 開放點 |
|---|---|---|
| **D1** | **PPID→recipe 對應與目前配方語意** | 確認 PPID == HT160 recipe **資料夾**名 (經 `NormalizeRecipeName`)。開放：S7F5 空白 PPID 是否代表「目前載入配方」(`GetCurrentRecipeName()`) 或拒絕？預設：空白 S7F5→目前配方；空白 S7F1→HCACK=7。 |
| **D2** | **PP-body 封裝契約** | `[[FILE:name]]` + escaped-CRLF 多檔信封為 HT172 單檔 `PPID.ini` 的替代。開放：host 視 body 為**不透明**(僅 round-trip) 還是**解析** marker？ |
| **D3** | **下載階段 go/no-go (= Approach #2 寫入側)** | S7F3→S7F4 host 寫配方為**最高風險**（覆寫資料夾、需凍結 INI schema、auto-switch 決策）。建議：延後至真實 host 里程碑，再依 #2 section-parsed 設計實作，auto-switch 預設 OFF。 |
| **D4** | **固定檔清單 vs 資料夾列舉** | `BuildRecipePPBody` 序列化固定清單 `{setup.ini, BinAreaMap.ini}`。開放：保留確定性固定清單（利於 round-trip）或改 FindFirst 列舉。預設：固定清單，內容增長時再檢討。 |
| **D5** | **錯誤回覆策略** | 沿用 log-only S9F3 + well-formed benign reply（house convention，見 `S2F14:339`, `S5F6:870`），**不**新增 vtable-changing primitive。若 host 嚴格要求 on-wire S9F7，另開 scoped 任務（#3 設計保留為其地圖）。 |

---

## 3. HT172 如何運作 (參考摘要，唯讀)

> 來源：`D:\HT172\HT172_Program_V1.0.25.0_20260420\`。**HT172 為唯讀參考**，禁止編輯。

### 3.1 S5 警報 (HT172)

HT172 的 S5 是「目錄儲存 + 推播」模型，**核心不相容點**：整個設計掛在一個 **form-resident `TStringGrid` (`THGem::strGrdAlarm`)** 上 — HT160 THGem **無 form/StringGrid**，只有 TList registries。

- **目錄儲存** = `strGrdAlarm`（11 欄）+ 磁碟檔 `AlarmData.def`。欄位：col0=No, col1=ID, col2=AlarmCode, col3=UnitNo, col4=UnitName, col5=Type, col6=Message, col7=Enable, col8=AlarmID, col9=Class, col10=Position（`uHGemHT172.cpp:365`）。
- **機台 → 目錄橋接**：`AddAlarmList` (`uHGemHT172.cpp:362`) 寫 header → `ReadAlamData()` → 走訪 `HSys.mapAlarmCodeList` 逐筆 `SetAlamData` → `WriteAlamData()`。於 GEM bring-up `UsecegemMainFrom.cpp:196` 呼叫一次。
- **S5F5→S5F6** (`uHGemHT172.cpp:1158`)：specific-ALID 分支 (`:1237`) 經 `GetAlarmIndex` 線掃 col8，回 `L,3{ B ALCD=Enable+0x80, I8 ALID, A ALTX=col6 }`；list-all 分支 (`:1341`) 回 `L,(RowCount-1)` 同樣三元組。
- **S5F1 推播** = `ReportAlarm` (`uHGemEquipment.cpp:6319`)：線掃 col2==AlarmCode，僅當 col7(Enable)=="1" 才送 S5F1。由 `note.cpp:107` 在警報 UI 路徑觸發。
- **已知缺陷/quirk**：`SetAlamData` 只填 col7/8/9/10（不填 col2/col6），參數命名錯位；`ReadAlamData` 載入 `strGrdAlarmOld`（**另一個** grid，潛在 bug）。HT160 port **不可**照抄此欄位錯位。
- HT172 在畸形 S5F5 用 `S9F7_IllegalData`（HT160 **無**此 primitive）。

### 3.2 S7 配方 (HT172)

HT172 配方為**單一扁平檔** `D:\HT172\data\<PPID>.ini`；PPID 同時是檔名與邏輯 id，**無**資料夾/多檔概念。Body 為 INI 文字，線上以字面 `\r`/`\n` 兩字元 escape 傳遞。

- **S7F2** (`uHGemHT172.cpp:1362`)：讀 PPID（忽略長度項），HCACK ladder — `SystemStart`→6、空 PPID→7、非法檔名字元(`TRegExpr [/*?:"><|]`)→8、`HasICUnderMachine()`→9、否則 0。手建 `InitLocalHead(7,2,0)+DataItemOut(BINARY HCACK)+SendLocalData`。
- **S7F4** 下載存檔 (`:1411`)：讀 `L,2{PPID, PPBODY}`，存 `D:\HT172\Data\<PPID>.ini`，byte 迴圈把 `\r`/`\n` escape 轉回真 CR/LF，成功回 `LocalAcknowledge(7,4,0)`，格式錯誤落入 `S9F7_IllegalData`。
- **S7F6** 上傳 (`:1557`)：建 `D:\HT172\data\<PPID>.ini`，不存在→`S9F7_IllegalData("S7,F5 PPID Not Exists!!!")`；否則經 `memoPPBody`（form 上的 TMemo 暫存）載入，CRLF→escape，回 `L,2{A PPID, A body}` (`:1593`)。
- **依賴的 primitive**：`S9F7_IllegalData` (`uHGemClass.cpp:2303`)、`LocalAcknowledge` (`uHGemEquipment.cpp:2204`) — **HT160 兩者皆無**。

---

## 4. HT160 定案設計

### 4.0 HT160 SECS 框架背景（共用認知）

- 兩層：**transport+codec** = `THGem` (`uHGemEquipment.cpp/.h`，`TComponent`，無 form)；**logic** = `HTGem` 基底 (`uHGemClass.cpp/.h`) + `HT160Gem` 具體覆寫 (`uHGemHT160.cpp/.h`)。
- registries 為 `TList*`：`SVList/ECList/ReportList/CEIDList` (`uHGemEquipment.h:136-139`)。
- **回覆慣用語**：`ResetReturnCode` → `GetDataItemLenAndTypeAndDelete(n, LIST)` → `InitLocalHead(S, F+1, 0)` → `DataItemOut(...)` body → `SendLocalData()`。範例：`S1F4` (`uHGemHT160.cpp:385`)、`S2F14` (`:323`)。
- **TX gate**：`SendLocalData` 僅在 `iHsmsState==HSMS_STATE_SELECTED` 才送 (`uHGemEquipment.cpp:998`)；Dispatch 本身不 gate。
- **S9 現況**：僅有 `S9F3_Unrecognized_Stream_Function_Type`（`uHGemClass.cpp:232`，**僅 log，不上線**）。`S9F7_IllegalData`、`LocalAcknowledge` 全樹**缺席**。
- **HType** 常數：`LIST_TYPE=0x00, ASCII_TYPE=0x40, BINARY_TYPE=0x20, I8 (INT_8_TYPE)=0x60, U4=0xB0`（`uHGemEquipment.cpp:15`）。

---

### 4.1 S5 警報設計

**SCOPE**：S5F5/S5F6（列表）、S5F3/S5F4（啟停 + 持久化）、S5F1（推播，編譯關閉）達 host-milestone parity，採 **form-less TList** 模型。

#### 資料結構（新增，置於 `uHGemEquipment.h` TGemCEIDItem 之後，與既有 flat-POD 同風格）

```
struct TGemAlarmItem {
    AnsiString AlarmCode;   // 5-char "%d%03d%1d" key, == HSys.mapAlarmCodeList key
    __int64    ALID;        // wire id, = _atoi64(AlarmCode); stable across map order
    int        AlarmType;   // eAlarmType (4 cyl / 5 motor / 6 suck); also GEM Class source
    AnsiString ALTX;        // alarm text == MyAlarmCodeStruct.E_ErrMessage (opaque bytes, may be Big5)
    AnsiString Position;    // MyAlarmCodeStruct.FlushPanelName (optional)
    int        Enable;      // 0/1 host enable flag; persisted; default 1
    bool       Active;      // runtime set/clear state; NOT persisted
};
```

`HSys.mapAlarmCodeList` (`database.h:592-594`) 仍是唯一真實來源；`AlarmList` 是其 SECS-facing 投影，每次開機重建。

#### 新增 THGem 成員（`uHGemEquipment.h`，置於 SVList/.../CEIDList 旁）

```
TList *AlarmList;                                    // TGemAlarmItem* -- 第 5 個 registry
TGemAlarmItem *FindAlarmByALID(__int64 ALID);       // private linear scan, NULL if absent
TGemAlarmItem *FindAlarmByCode(AnsiString AlarmCode);// private linear scan
void ReportAlarm(AnsiString AlarmCode, bool bRelease, int iReserved, AnsiString SubMessage); // public S5F1 sender
```

- **ctor**：`AlarmList = new TList`（與 `SVList=new TList` 同處，`uHGemEquipment.cpp:50`）。
- **dtor**：逐項 `delete (TGemAlarmItem*)` 後 `delete AlarmList; AlarmList=NULL;`（鏡像 SVList cleanup `:124-139`）。
- **配置慣例（採對抗審查 low 修正）**：**唯一**配置點在 ctor，**移除** `SetAlamData` 的 lazy-new，與 SV/EC/CEID 慣例一致。

#### 填入既有空 stub（**簽章不變** — graft A）

| 函式 | 位置 | 內容 |
|---|---|---|
| `THGem::SetAlamData(int Index, AnsiString AlarmCode, AnsiString UnitName, AnsiString Message, AnsiString AlarmType)` | `uHGemEquipment.cpp:656` | 建一 `TGemAlarmItem`：`AlarmCode`=AlarmCode、`ALID`=`_atoi64(AlarmCode.c_str())`、`ALTX`=Message (E_ErrMessage；**修正** HT172 drop-Message quirk)、`Position`=UnitName、`AlarmType`=`StrToIntDef(AlarmType,0)`、`Active`=false、`Enable`=從 restore map 取（缺則預設 1）。`AlarmList->Add(p)`。**不**照抄 HT172 col8/col2 錯位。 |
| `THGem::ReadAlamData()` | `:660` | 載 `CurrentDirectory + "\\SYSTEM\\AlarmEnable.def"`（`CurrentDirectory` = `HSys.CurrentDir+"\\SECS"`，`uHGemEquipment.cpp:23`）入暫態 `std::map<__int64,int>`（TAB 行 `ALID<TAB>Enable`）。檔缺→空 map→全預設啟用。於 `AddAlarmList` **開頭、SetAlamData 迴圈前**呼叫（#2 ordering graft）。 |
| `THGem::WriteAlamData()` | `:664` | `ForceDirectories` SYSTEM 目錄；走訪 AlarmList，每 **live** row 寫 `ALID<TAB>Enable` 至 `AlarmEnable.def`（`TStringList::SaveToFile`）。只寫 live row → 修剪 orphan（目錄 churn 穩健，graft B）。於 `AddAlarmList` 後與每次 S5F4 變更後呼叫。 |

#### HT160Gem 處理器（**body-fill 既有覆寫 — 非 vtable 變更**）

- **`HT160Gem::AddAlarmList()`** (`uHGemHT160.cpp:232`，已於 `UsecegemMainFrom.cpp:113` 連線)：map→TList 橋接。順序：(1) `ReadAlamData()` 先；(2) 用 `HSys.IterAlarmCodeList` 經典 iterator for-loop（**無** range-for/auto）逐筆 `SetAlamData(i, a.AlarmCode, a.FlushPanelName, a.E_ErrMessage, IntToStr(a.AlarmType))`（欄位：AlarmCode `database.h:34`、AlarmType `:35`、E_ErrMessage `:36`、FlushPanelName `:40`）；(3) `WriteAlamData()` 後。目錄誠實（graft C）：僅 cyl(4)/motor(5)/suck(6)，**不**捏造 system(3)/sensor。
- **`HT160Gem::S5F6_ListAlarmData()`** (`uHGemHT160.cpp:870`，目前 `L,0`)：**EXTEND**，保留空 `L,0` 為 AlarmList null/empty 的 fallback（**不**重新 flag 既有 guard）。`ResetReturnCode` → `GetDataItemLenAndTypeAndDelete(n, LIST)`。
  - **Branch A** (n>0 specific)：每筆請求 ALID → `FindAlarmByALID`；找到→`L,3{ B ALCD, I8 ALID, A ALTX }`；未知→`L,3{ B 0x80, I8 42, A "Unknown Alarm Code" }`。
  - **Branch B** (n<=0 list-all)：`InitLocalHead(5,6,0)` + `DataItemOut(AlarmList->Count, LIST)` + 每項同樣三元組。
  - 兩分支 ALID 一律 **I8** (D3)。畸形 S5F5 → well-formed 空 `L,0`（無 T3）+ `StringOut`，**不**引入 S9F7。
  - **⚠ 對抗審查 high (見 §6)**：ALCD 應由 **Active 狀態**（0x80 active / 0x00 cleared）+ bits1-7 alarm class 構成，**不可**把 Enable 塞進 ALCD（HT172 quirk）；Enable 狀態應改由 S5F7→S5F8 回報。此為 host-contract 決策，列為開放議題。
- **`HT160Gem::S5F4_EnableDisableAlarmAcknowledge()`** （**覆寫既有 base virtual** `uHGemClass.h:76` / base body `uHGemClass.cpp:176`；於 `uHGemHT160.h` public 加宣告 + `uHGemHT160.cpp` 加 body — **不**動 `uHGemClass.h`）：
  - 讀 S5F3 `L,2{ ALED, ALID }`。**ALED 型別寬鬆**（對抗審查 high）：接受 **BINARY 與 U1**，取低位 byte 測 bit7 (0x80)。ALID 接受 U1/U2/U4/U8/I1/I2/I4/I8 正規化為 `__int64`。
  - ALED bit7 → Enable=1 否則 0；ALID==0 → 套用所有項；否則 `FindAlarmByALID` 翻轉該項。
  - `WriteAlamData()` 持久化。回覆手建：`InitLocalHead(5,4,0)` + `unsigned char ACKC5; DataItemOut(1, BINARY, &ACKC5)` + `SendLocalData`（**無** enclosing list）。`FindAlarmByALID` miss 時 ACKC5=1（非零，鏡像 HT172 失敗路徑），否則 0。

#### S5F1 推播（實作但 staged OFF）

`THGem::ReportAlarm(...)`：`FindAlarmByCode`；early-out 若 item NULL **或** Enable!=1 **或** `iHsmsState!=HSMS_STATE_SELECTED`（鏡像 EventReport `:316`）。設 `item->Active=!bRelease`；`InitLocalHead(5,1,1)`（W=1 primary）；`DataItemOut(3,LIST)`；ALCD=`bRelease?0x00:0x80` BINARY；ALID I8；ALTX=`SubMessage` 或 `item->ALTX`。只 port 3-item TSMC 形狀，**丟棄** HT172 6-item KYEC/DL_TEK 分支與 CC_TFME_CHINA offline early-return。

- **整個 body 包在 `#ifdef SECS_S5F1_PUSH`** → 預設編譯關閉。
- `note.cpp` 觸發點為 **設計 only / 延後**（HT160 對應 HT172 `note.cpp:107`），本期**不**加 call site。
- **go-live 時**（D1）需**同時**加上 online/control-state gate（對抗審查 medium）：除 SELECTED 外，僅 ONLINE/REMOTE 才推（重用 `iControlState`，`uHGemHT160.cpp:25`）。

#### 磁碟佈局 (S5)

- **新增**：`<programroot>\SECS\SYSTEM\AlarmEnable.def` — TAB 分隔 `ALID<TAB>Enable`，**唯一**持久化產物，以 ALID 為鍵。
- **不變**：`<programroot>\system\AlarmList.csv`（`database.cpp:878`）— 唯讀人類參考，SECS 層**不**讀。
- **不做** `AlarmData.def`（HT172 11 欄格式刻意不 port）。

#### 待補缺口（對抗審查 high — 強烈建議納入本期或明確標記為已知缺口）

- **S5F7→S5F8 (List Enabled Alarm)**：目前 Dispatch **未**路由 S5F7（僅 F3/F5，已於 `uHGemClass.cpp:96-97` 確認），fallthrough 至 log-only S9F3 → host W-bit S5F7 會 **T3 timeout**。這是持久化 Enable 儲存的**自然 read-back 配套**。建議：在 Dispatch S5 switch 加 `case 7: S5F8_ListEnableAlarmAcknowledge()`，填 S5F8 回 `L,n{ ALID }`（Enable==1 者，空則 `L,0`）。base stub 已存在 (`uHGemClass.h:78`)。若延後，須明確標記為已知缺口而非靜默。

---

### 4.2 S7 配方設計

**SCOPE 本期**：S7F1→S7F2（load/upload-inquire grant）+ S7F5→S7F6（配方上傳）。**無** S7F3/S7F4 下載。**無**新 SECS primitive。全為既宣告 virtual 的 body-fill。

#### Handler 1 — `S7F2_ProcessProgramLoadGrant()`（回 int HCACK）

> 替換 stub `uHGemHT160.cpp:883`。所有 codec 呼叫須 `HGemPtr->` 前綴。

1. `if(HGemPtr==NULL) return 1;` → `HGemPtr->ResetReturnCode();`
2. **畸形 guard（對抗審查 high）**：`GetDataItemLenAndTypeAndDelete(n, LIST_TYPE)`，若 `!=1` 或 `n<1` → HCACK=7 並立即回覆，不讀 item0/item1（鏡像 S1F4/S2F14 house pattern）。
3. 讀 item0 PPID：`GetDataItemLenAndType(len,Type)` + `DataItemIn(len,Type,sPPID)`。
4. 讀棄 item1 PPLEN（僅 `n>=2` 時）：`DataItemIn` 入 throwaway，保持 cursor 對齊（graft #3）。
5. **HCACK ladder（順序載重）**：
   ```
   unsigned char HCACK=0;
   if(HSys.Sys.SystemStart)                                HCACK=6;  // busy
   else if(sPPID.Trim()=="")                               HCACK=7;  // empty PPID
   else if(<sPPID.Trim() 含非法字元 \/:*?"<>|>)             HCACK=8;  // illegal chars
   else if(HasICUnderMachine())                            HCACK=9;  // IC under machine
   else                                                    HCACK=0;  // grant
   ```
   - **HCACK=8 偵測（對抗審查 medium 修正）**：**直接** for-loop 掃 `sPPID.Trim()` 是否含非法集 `\/:*?"<>|`（對應 `CosFunction` InvalidChars，無 C++11、無 regex）。**不可**用 `NormalizeRecipeName(sPPID)!=sPPID.Trim()` — `NormalizeRecipeName` 會把空字串映成 `"Default"` (`CosFunction.cpp:36-37`)，且內部也 trim，會誤判。`NormalizeRecipeName` 僅用於 ladder 通過後的資料夾解析。
6. 回覆手建：`InitLocalHead(7,2,0)` + `DataItemOut(1, BINARY_TYPE, &HCACK)` + `SendLocalData()`。
7. `StringOut` 一行 ASCII trace；`return (int)HCACK;`

> **註（對抗審查 low / D3）**：本期 granted action 是**唯讀上傳**，HCACK=6/9 在此為 advisory load-intent gate，非 data-safety gate。須與 host 確認「running/IC 時 6/9 擋住後續唯讀 S7F5」可接受；若 host 期望邊跑邊 upload-inspect，需放寬 6/9。

#### Handler 2 — `S7F6_ProcessProgramData()`（inbound dispatch 入口，no-arg）

> 替換 stub `uHGemHT160.cpp:896`。dispatch router 對 S7F5 呼叫此（`uHGemClass.cpp:105`）。

1. `if(HGemPtr==NULL) return;` → `ResetReturnCode();`
2. 讀 ASCII PPID：`GetDataItemLenAndType(len,Type)` + `DataItemIn(len,Type,sPPID)`。
3. 讀失敗 → **benign well-formed reply**（鏡像 S2F14 `:339-345`）：`InitLocalHead(7,6,0)` + `DataItemOut(2,LIST)` + 兩個空 ASCII + `SendLocalData`；`S9F3_Unrecognized_Stream_Function_Type("S7F5 bad PPID")`（僅 log）；return。
4. 否則 forward：`S7F6_ProcessProgramData(sPPID);`

#### Handler 3 — `S7F6_ProcessProgramData(AnsiString FileName)`（worker，param = PPID）

> 替換 stub `uHGemHT160.cpp:902`。本期序列化**內聯**於此（無 header 編輯）。

1. `if(HGemPtr==NULL) return;` → `AnsiString sPPID = RecipeManager.NormalizeRecipeName(FileName);`
2. `if(!RecipeManager.RecipeExists(sPPID))`（資料夾檢查，`CosFunction.cpp:133`）→ benign reply `L,2{ A sPPID, A "" }` + log-only `S9F3(...)`；return。
   - **⚠ 對抗審查 medium (見 §6)**：空 body 對 S7F6 是**in-band 值碰撞**（host 無法區分「不存在」與「存在但空」）。須在 D2 host-contract 明確定義哪個訊號代表 not-found；若 host 嚴格要求，另開 S9F7 任務。
3. 內聯建 body（固定清單 `{"setup.ini","BinAreaMap.ini"}`，D4）：
   ```
   const char *KnownFiles[2] = {"setup.ini", "BinAreaMap.ini"};   // C-array, no brace-init range-for
   TStringList *T = new TStringList();
   AnsiString sBody = "";
   for(int k=0; k<2; k++) {
       AnsiString f = RecipeManager.GetRecipeFileName(sPPID, KnownFiles[k]);  // CosFunction.cpp:86
       sBody += AnsiString("[[FILE:") + KnownFiles[k] + "]]\r\n";
       T->Clear();                                  // 對抗審查 medium: 每次 Load 前 Clear
       if(FileExists(f)) { T->LoadFromFile(f); sBody += T->Text; }
       // 段界明確: 若 sBody 非空且未以 \r\n 結尾, 補一個再進下個 marker
   }
   delete T;
   ```
4. wire-escape（對稱 HT172 `:1593`）：`sBody = StringReplace(sBody, "\r\n", "\\r\\n", TReplaceFlags()<<rfReplaceAll);`
5. 回覆：`InitLocalHead(7,6,0)` + `DataItemOut(2, LIST_TYPE, NULL)` + `DataItemOut(ASCII_TYPE, sPPID)` + `DataItemOut(ASCII_TYPE, sBody)` + `SendLocalData()`。
6. `StringOut` trace。**不**呼叫 `LookForFile()`（base no-op `:925`，設計保留 no-op）。

#### Handler 4 — `S7F4_ProcessProgramAcknowledge()`（下載延後，誠實標記）

> 保留 stub `uHGemHT160.cpp:890`。**僅**升級 `StringOut` 文字為 `"[SECS] S7F4 recipe download deferred - Phase 2 (host write path not enabled)"`，避免被誤認為遺漏。**不**加回覆、**不**解析 S7F3、**不**動 dispatch。

#### Body schema（wire 上、escape 前）

```
[[FILE:setup.ini]]<CRLF>[TrayForm]<CRLF>XStart=...<CRLF>...
[[FILE:BinAreaMap.ini]]<CRLF>[BinAreaMap]<CRLF>...<CRLF>[ErrorBinAreaMap]<CRLF>...
```

此為 HT172 扁平 `PPID.ini` 的資料夾→單 blob 替代，亦為未來 Phase-2 S7F4 下載解析回的形式。

#### Wire 形狀（與 HT172 / 模擬器相容）

```
S7F1 in:  L,2 { A PPID, U4/numeric PPLEN }  -> S7F2 out: <B HCACK>
S7F5 in:  A PPID                            -> S7F6 out: L,2 { A PPID, A body(escaped) }
```

---

### 4.3 S7 OPTIONAL refactor（若要 BuildRecipePPBody helper）

若採可讀性 helper：在 `uHGemHT160.h` private 區加 **non-virtual** `AnsiString BuildRecipePPBody(AnsiString sRecipe);`，把 §4.2 Handler 3 序列化搬入。**注意（對抗審查 medium）**：non-virtual 不改 vtable 但仍是 header 變更，會迫使所有 include 此 header 的 TU 重編 → **必須 `-Full`**。若不想 `-Full`，**完全內聯**於 worker、不動 header。**二擇一，不可同時提供 `-Clean` 與 `-Full`**。

---

## 5. 序列實作清單 (Incremental Checklist)

> 強調**主迴圈序列**與 **build gate**。每增量：檔 / 改動 / 刪 obj / build flag / 模擬器測項 / 風險。
> Big5 既有行編輯一律用 `scripts/ops/bcb6-bytesafe-edit.ps1`（Edit 工具會破壞 Big5）。每次後跑 `scripts/ops/check-ht160s-source-encoding.ps1`。

### 5.1 S5 增量

| # | 標題 | 檔 | 刪 obj | build flag | 模擬器測項 | 風險 |
|---|---|---|---|---|---|---|
| 1 | 加 `TGemAlarmItem` struct + `AlarmList` 成員/宣告（**僅 header**） | `uHGemEquipment.h` | 全部（struct-size 變更） | **-Full** | S5F5 list-all 仍回空 `L,0`，無 T3 → 證明 layout 變更不退化 skeleton | medium |
| 2 | ctor/dtor 配置/釋放 `AlarmList` | `uHGemEquipment.cpp` | `SecsGem\uHGemEquipment.obj` | -Clean | HSMS 起降跑 ctor+dtor，log 乾淨無洩漏；S5F5 仍空回 | low |
| 3 | 實作 `FindAlarmByALID`/`FindAlarmByCode` | `uHGemEquipment.cpp` | `uHGemEquipment.obj` | -Clean | link clean；S5F5 仍空回 | low |
| 4 | 實作 `ReadAlamData`（載入暫態 map）+ `WriteAlamData`（寫 live row） | `uHGemEquipment.cpp` | `uHGemEquipment.obj`（**不**加 header 成員→保持 -Clean） | -Clean | 首次 `AddAlarmList` 後產生 `<root>\SECS\SYSTEM\AlarmEnable.def`（步驟6驗） | medium |
| 5 | 填 `SetAlamData` body（建 item，Enable 從 restore map seed） | `uHGemEquipment.cpp` | `uHGemEquipment.obj` | -Clean | link clean；S5F5 仍空回（待步驟6 AddAlarmList 呼叫） | low |
| 6 | 填 `AddAlarmList`：map→SetAlamData，Read-before/Write-after 順序 | `uHGemHT160.cpp` | `SecsGem\uHGemHT160.obj` | -Clean | 重啟跑 `GemInitial→AddAlarmList`；`AlarmEnable.def` 每目錄 alarm 一 TAB row；S5F6 仍空回（步驟7前） | medium |
| 7 | EXTEND `S5F6_ListAlarmData` 讀 AlarmList（list-all + by-ALID 分支） | `uHGemHT160.cpp` | `uHGemHT160.obj` | -Clean | (a) S5F5 list-all→`L,n` 每項 `L,3{B,I8,A}`，抽驗 ALID==`_atoi64`(code)、ALTX==E_ErrMessage；(b) 單一已知 ALID→單 `L,3`；(c) bogus ALID→`L,3{0x80,42,"Unknown..."}`；(d) 畸形→空 `L,0` 無 T3 | medium |
| 8 | 加 + 實作 `S5F4_EnableDisableAlarmAcknowledge`（持久化啟停） | `uHGemHT160.h`, `uHGemHT160.cpp` | `uHGemHT160.obj` + `UsecegemMainFrom.obj` + `uHGemClass.obj`（slot 已在 base→ **-Clean** 正確） | -Clean | (a) S5F3 disable 一 ALID→S5F4 ACKC5=0，再 S5F5 by-ALID 顯示該位清除；(b) **重啟**再查 S5F6→維持 disabled（證明 `AlarmEnable.def` 持久化，核心 host 需求）；(c) ALID==0 enable-all；(d) 確認磁碟反映翻轉 | medium |
| 9 | 實作 `ReportAlarm` S5F1 推播 body，包在 `#ifdef SECS_S5F1_PUSH` | `uHGemEquipment.cpp` | `uHGemEquipment.obj` | -Clean | 預設 build（旗標未定義）：任何情況**零** S5F1 流量。Gated 驗證（僅 scratch `-DSECS_S5F1_PUSH`，**不**提交）：手動呼叫確認模擬器解 `S5F1 W L,3{B,I8,A}`；Enable=0 不推。驗畢還原旗標 | medium |

> **S5 build gate 修正（對抗審查 medium）**：步驟 1 的 `-Full` 由**新 struct 成員 (struct size) 與新方法宣告 (`ReportAlarm`/`FindAlarm*`/HT160Gem S5F4 override)** 共同驅動，**非**S5F4 override 本身（其 slot 已存在 base virtual `uHGemClass.h:76`）。一旦 layout 穩定後，後續 body-fill 為 -Clean。

### 5.2 S7 增量

| # | 標題 | 檔 | 刪 obj | build flag | 模擬器測項 | 風險 |
|---|---|---|---|---|---|---|
| 1 | S7F2 upload-inquire grant ladder（S7F1→S7F2） | `uHGemHT160.cpp` | `SecsGem\uHGemHT160.obj` | -Clean | `send_raw(7,1, L(A("<存在配方>"), U4(0)))`→`<B 0x00>`；SystemStart 時→`<B 0x06>`；`A("")`→`<B 0x07>`；`A("bad:name*")` idle→`<B 0x08>`；無 T3，每 S7F1 恰一 S7F2 | low |
| 2 | S7F6 worker — 存在配方回真 body / 不存在回 benign（內聯序列化） | `uHGemHT160.cpp` | `uHGemHT160.obj` | -Clean | 經步驟3 dispatch 達；先確認 build link 通過（worker 編譯）。完整 wire 測延至步驟3 | low |
| 3 | S7F6 inbound dispatch 入口（S7F5→S7F6）讀 PPID forward worker | `uHGemHT160.cpp` | `uHGemHT160.obj` | -Clean | `send_raw(7,5, A("<存在 PPID>"))`→`L,2{A PPID, A body}`，un-escape 後含 `[[FILE:setup.ini]]`；`A("<不存在>")`→`L,2{A PPID, A ""}` + log-only S9F3，無 T3；`A("")`→正規化為 "Default"，視 `data\Default` 存在與否回 body 或空（記錄結果，D1） | medium |
| 4 | OPTIONAL refactor — 抽 `BuildRecipePPBody` helper（**header touch → -Full**） | `uHGemHT160.h`, `uHGemHT160.cpp` | `uHGemHT160.obj` | **-Full** | 行為須與步驟3 byte-identical；重跑步驟3 序列，decoded body diff 必須為空 | low |
| 5 | S7F4 download-deferred log 行（誠實標記，無行為變更） | `uHGemHT160.cpp` | `uHGemHT160.obj` | -Clean | `send_raw(7,3, L(A("X"),A("Y")))`→HT160 不 crash、不回 S7F4、log 顯示 "download deferred - Phase 2"；host 端無 T3 期待 | low |

> **S7 測試前置（對抗審查 low）**：source checkout **無** `data\` 資料夾（配方 runtime 在 EXE 旁 `HSys.CurrentDir\data`）。測前須在 built EXE 旁建 `<programroot>\data\TESTPPID\setup.ini` + `BinAreaMap.ini`，並另以一個**確定不存在**的 PPID 測 not-found 分支。純 source-tree 測試只會走 not-found 路徑。

> **共用注意（對抗審查 low）**：所有 S7 回覆經 `SendLocalData`，僅 SELECTED 才上線（`uHGemEquipment.cpp:998`）。模擬器須先完成 Select.req/rsp（passive 模式預設會）再送 S7，否則 W-bit 請求看到的是 T3 timeout 而非錯誤回覆。

### 5.3 模擬器驅動（`D:\AI_Area\Tool\HT160S_SECS_Simulator`）

- Topology：HT160 維持 `ActiveMode=0`（passive listen TCP 5098, Device/Session ID=1）；模擬器預設 Passive，HT160 撥入，模擬器送 Select.req。
- 五個 round-trip 中僅 S5F1 (E→H) 今日已連線（auto S5F2）。其餘以 `HostApp.send_raw(stream, function, item, w_bit)`（`secs_host_simulator.py:173`）手建，universal RX decoder 自動 render。
- S5F3 已有 preset (`_s5f3`)。建議新增 S5F5/S7F1/S7F3/S7F5 preset（仿 `_s5f3`/`_s1f3` 風格）— **本期不實作**，僅 prep 地圖。

---

## 6. 對抗審查發現 (Adversarial Findings)

> 依嚴重度排序。已將修正納入 §4/§5 設計。

### S5

| 嚴重度 | 領域 | 議題 | 修正 |
|---|---|---|---|
| **high** | semi-e5 | S5F6 ALCD 編碼為 `(Enable + 0x80)`，照抄 HT172 quirk，把 host enable 旗標塞進 SEMI E5 的 Alarm Code Byte（bit8=set/clear、bits1-7=category），spec-driven EAP 會誤讀每個 alarm 為 category 0/1 | ALCD bit8 由 runtime **Active** 設（0x80 active / 0x00 cleared），bits1-7 由真實 alarm class（可由 AlarmType 導）；Enable 狀態改由 **S5F7→S5F8** 回報，**不**放 S5F6 ALCD。列為 host-contract 開放決策 |
| **high** | semi-e5 | S5F3 ALED 型別不匹配：設計解析為 U1，但 SEMI E5/HT172/模擬器 preset 皆送 **Binary** ALED；嚴格型檢會拒絕 → 啟停靜默失效 | S5F4 ALED 解析**型別寬鬆**：接受 BINARY 與 U1（取低 byte 測 0x80）；ALID 接受 U1/U2/U4/U8/I1-I8 正規化 `__int64`。列開放議題確認 EAP ALED 編碼 |
| **high** | host-scenario | **S5F7→S5F8 未路由**：Dispatch 僅 F3/F5（`uHGemClass.cpp:96-97`），host 送 W-bit S5F7 會 fallthrough 至 log-only S9F3 → **T3 timeout**。持久化 Enable 儲存缺 read-back，直接打臉「持久化讓 host 信任 disable 存活」的 rationale | 加 `case 7: S5F8_ListEnableAlarmAcknowledge()`，填 S5F8 回 `L,n{ALID}`（Enable==1，空則 `L,0`）。base stub 已在 `uHGemClass.h:78`。若延後須明確標記已知缺口 |
| medium | build-gate | `-Full` 理由半對：S5F4 override 確是既有 base virtual（不改 base vtable），但加 HT160Gem 宣告 + 新 `ReportAlarm` + 新 TList 成員仍改具體類別 layout/vtable | 維持 `-Full`（結論不變），理由更正為：**新 struct 成員 (struct size) 與新方法宣告共同**驅動 → 刪全部 obj，**不**用 curated 刪 |
| medium | semi-e5 | S5F4 ack：`DataItemOut(BINARY, &ACKC5)` 若 ACKC5 是 int，靠 little-endian 巧合才對 | ACKC5 宣告為 `unsigned char`（一 byte），無 enclosing list；miss 時回 ACKC5=1（非零，鏡像 HT172 失敗），否則 0 |
| medium | host-scenario | S5F1 推播缺 GEM control-state (Local/Remote/Offline) gate；僅 SELECTED 即推可能違反 host online/offline 契約 | go-live (D1) 時在 `#ifdef` body 內加 control-state gate（重用 `iControlState`），與 note.cpp 觸發**一併**連線 |
| low | ht160-integration | `SetAlamData` lazy-new 與 ctor 配置雙重所有權味道；SV/EC/CEID 皆只 ctor 配置 | 唯一配置點在 ctor，**移除** lazy-new；`SetAlamData` 假設 AlarmList 非 NULL |
| low | data-source | `ALID=_atoi64(code)` 今日安全，但 `%03d` 在 device count >999 會溢位寬度→ALID 碰撞/位移，靜默 | 保留 `_atoi64`，加 ASCII 註解 + 整合註記：`%d%03d%1d` 每族上限 index 999；`AddAlarmList` 時 assert/log 非預期 code 長度，讓未來擴張**大聲失敗** |

### S7

| 嚴重度 | 領域 | 議題 | 修正 |
|---|---|---|---|
| **high** | host-scenario | S7F2 缺 malformed-L,2 guard（同檔 S1F4/S2F14 皆有）：無條件讀 item0/item1，host 送非 list/空/L,1 會 cursor desync，PPLEN discard 讀過界 | 仿 house pattern：`GetDataItemLenAndTypeAndDelete(n,LIST)` 後若 `!=1` 或 `n<1`→HCACK=7 立即回；PPLEN discard 用 `if(n>=2)` 守護；信任 sPPID 前查一次 `GetReturnCode()` |
| medium | semi-e5 | HCACK=8 用 `NormalizeRecipeName(sPPID)!=sPPID.Trim()` 偵測非法字元語意錯誤（Normalize 把空→"Default" 且內部 trim，會誤判 leading/trailing 空白為非法字元） | 直接 for-loop 掃非法集 `\/:*?"<>|`（對應 `CosFunction` InvalidChars，無 regex/C++11）；`NormalizeRecipeName` 僅 ladder 通過後解析資料夾。使 HCACK=7/8 互斥誠實 |
| medium | semi-e5 | S7F6 missing-recipe 回 `L,2{A PPID, A ""}` 空 body → host 無法區分「不存在」與「存在但空」(in-band 值碰撞)；對 S7F6 不正確 | 不靜默回空：(a) 用 host-contract 定義的 not-found sentinel，或 (b) 另開 scoped S9F7 primitive 任務（接受 -Full）。最低限度提升為開放決策，整合前須定義 |
| medium | build-gate | OPTIONAL helper 加入 `uHGemHT160.h` 是 header 變更（雖 non-virtual 不改 vtable，但迫使含此 header 的 TU 重編）；設計同時說 -Clean「涵蓋」與「-Full once」矛盾 | 明確化：加 helper decl → **-Full**（無 -Clean 選項）；不想 -Full 就**完全內聯**無 header 編輯。不可同提供兩者 |
| medium | other | `const known files = {...}` + `for each` 非 BCB6 C++，有 C++11 brace-init/range-for 翻譯風險；單一 TStringList 跨檔重用未 Clear；`T->Text` 尾無 CRLF 時段界黏連 | 用 `const char *KnownFiles[2]={...}` + index for-loop；每次 `LoadFromFile` 前 `T->Clear()`；每檔附加後確保尾端 CRLF 再寫下個 marker |
| low | semi-e5 | S7F2 ladder 照抄 HT172 HCACK=6/9，但本期 granted action 是**唯讀上傳**，6/9 拒絕無害的讀 | 保留 ladder（前向相容 Phase-2），於 D1/D3 註明本期 6/9 為 advisory load-intent gate 非 data-safety gate，與 host 確認可接受；host 若要邊跑邊 inspect 則放寬 6/9 |
| low | data-source | D4 固定清單今日完整，但未來配方新增檔會靜默不上傳，Phase-2 下載無法還原，無錯誤 | 保留固定清單，加防禦：建 body 時 FindFirst 列舉資料夾，若有不在 KnownFiles 的 `*.ini` → `StringOut` 警告（甚至考慮 fail/納入），把靜默資料遺失轉為可見 log |
| low | ht160-integration | 驗證計畫依賴 disk 上配方，但 source tree **無** `data\` 資料夾 → 純 checkout 測試每查都走 missing 分支，易誤認 handler bug | 加前置步驟：測前在 built EXE 旁建 `data\TESTPPID\setup.ini`+`BinAreaMap.ini`，並以確定不存在 PPID 測 not-found；註明 source-tree-only 只測 not-found 路徑 |
| low | host-scenario | S7 回覆經 `SendLocalData` 僅 SELECTED 上線；Select 握手前送 S7 → handler 跑但不送 → W-bit S7F1 T3 timeout 無法區分「跑了未 select」與「crash」 | 無須改碼（pre-Select 是 host 端畸形）；S7F2/S7F6 的 `StringOut` trace 須讓 not-selected 明顯（確認既有 "(not connected)" log 觸發）；驗證計畫註明模擬器須先完成 Select |

---

## 7. 風險與鐵則 (Risks & Iron Rules)

1. **HT172 唯讀**：`D:\HT172\...` 僅供比對/移植，**禁止編輯**。reads 不受限。若任務看似要改 HT172，停止並導回 HT160S_BCB。
2. **no-FSM**：不得引入 `FSMRunner`/`FSM_GOTO`/transition table/`*Step.h`/`*Table.cpp`/`*Exec.cpp`/`FSM/`。以 procedural / VCL-event / `switch(F)` 重寫 HT172 行為。
3. **ASCII 新註解**：BCB6 source 新註解一律 ASCII English（避免 CP950/UTF-8 mojibake）。legacy `.cpp/.h/.dfm` 保留既有 Big5/CP950，**不**轉 UTF-8、**不**加 BOM。本文件 (`docs/`) 為 UTF-8 no BOM。
4. **no C++11**：不用 `auto`/`nullptr`/lambda/range-for/`enum class`；維持 `AnsiString` flows。
5. **vtable/struct 變更 → `-Full`**：任何改 THGem/HT160Gem struct layout 或新增 method 宣告，須 `scripts/ops/build-ht160s.ps1 -Full`（刪全部 obj）。純 body-fill 可只刪變更的 `.obj` 後 build（`-Clean` 涵蓋）。
6. **先補 S9F7（僅在 host 要求時）**：S9F7/LocalAcknowledge 為 greenfield。本期**刻意不做**，沿用 S9F3 (log-only) + benign reply 的 house convention。若未來 host 嚴格要求 on-wire S9F7，另開 scoped 任務（宣告 virtual + encode + SendLocalData + `-Full`）；S9F7 設計地圖保留於 Approach #3。
7. **勿動無關進入點**：S5F6 既有空 `L,0` guard、Dispatch 既有路由、S1F4/S2F14 reply 範本 — 除非設計明列，**不**重構/重新 flag。
8. **Big5 byte-safe 編輯**：觸及既有 Big5 行用 `scripts/ops/bcb6-bytesafe-edit.ps1`（Edit 工具會 re-encode 破壞 Big5）。ALTX/E_ErrMessage 可能含 Big5 → 直送 `c_str()` raw bytes，**不**重新編碼（同 S1F4/S2F14 現狀）。
9. **靜默停機必通知**（鐵則，本設計範圍外但相關）：任何拖垮 SystemStart 的 fault 必須 popup；S5F1 推播 go-live 時須與 control-state gate 一併連線，勿讓 SECS 推播路徑影響機台控制路徑。
10. **build 後驗證**：跑 `scripts/ops/check-ht160s-source-encoding.ps1`（偵測 `EF BF BD` 與 UTF-8 BOM）。BCB6 工具未實際執行時，**絕不**宣稱 build-clean。

---

*文件結束。本文件僅為設計地圖，未修改任何原始碼。*
