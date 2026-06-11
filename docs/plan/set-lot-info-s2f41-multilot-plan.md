# HT160S SET_LOT_INFO (S2F41/S2F42) 可變長度多 Lot 施工計畫書

作者：JimmyChiu（AI 協作：HT160S-Maintainer）
日期：2026/06/10
範圍：HT160S_BCB SECS/GEM 之 `S2F41 SET_LOT_INFO` 改寫為「可變長度多 Lot」並回 `S2F42`
狀態：規劃中（含一個阻擋性前置條件）

---

## 1. 需求摘要

### 1.1 HT172 現況（參考來源，唯讀）

HT172 `S2F41 SET_LOT_INFO` 為**固定 5 欄**結構，每欄是 `L[2]{key, value}`：

```
S2F41
L[2]
  A "SET_LOT_INFO"
  L[5]
    L[2] { A "LOT_ID",        A "U91T06L1A0" }
    L[2] { A "RC_NO",         A "W24082219-T1" }
    L[2] { A "OP_ID",         A "1095106" }
    L[2] { A "LOT_SUM",       A "<?xml ...>" }
    L[2] { A "TRAY_MAP_PATH", A "D:\TrayMap\" }
```

成功回：

```
S2F42
L[2] { B[1] 0x00, L[0] }
```

來源：`d:\HT172\HT172_Program_V1.0.25.0_20260420\SecsGem\uHGemHT172.cpp`
`HT172Gem::S2F42_Host_Command_Acknowledge()`（L755 起），`SET_LOT_INFO` 分支在 L964；
解析用 `GetDataItemLenAndTypeAndDelete(SVlen, LIST)` 並**硬性檢查 `SVlen==5`**，
逐欄 `i==0..4` 比對 key 字串。回覆段在 L1130（`InitLocalHead(2,42,0)` + `L[2]{B HCACK, L[0]}`）。

### 1.2 HT160 目標需求

HT160 一台機可同時掛多個 Lot，且 **Lot 數量不固定**，要把 body 改成「可變長度的 Lot 清單」。
使用者提供的草稿（巢狀層數有歧義，見 §4.1）：

```
S2F41
L[2]
  A "SET_LOT_INFO"
  L[5]
    L[3]
      A "U91T06L1A0"
      A "U91T06L1A1"
      A "U91T06L1A2"
```

成功回 `S2F42` HCACK=0x00。

---

## 2. 可行性結論

**邏輯上完全可行，而且 HT160 早已預留多 Lot 落點；但有一個阻擋性前置條件：HT160 的 SECS 引擎目前是「不能跑」的空殼。**

### 2.1 為什麼說落點已備妥（利多）

HT160 已內建多 Lot 註冊表，且已為「SECS 遠端推 Lot」預留來源碼：

| 既有資產 | 位置 | 用途 |
|---|---|---|
| `THT160LotRegistry LotRegistry`（全域）| `CosFunction.cpp` L20 / `CosFunction.h` L196 | 多 Lot 註冊表，上限 `HT160_MAX_LOT = 64` |
| `int AddLot(LotID, Source, SourceMachine, DeviceName)` | `CosFunction.h` L225 | 新增一個 Lot |
| `void Clear()` / `FindLotIndex()` / `GetLotCount()` | `CosFunction.h` L211-221 | 清空 / 查詢 |
| `#define HT160_LOT_SOURCE_SECS 1` | `CosFunction.h` L174 | **已為遠端 SECS 推 Lot 預留的來源碼** |

> 也就是說，可變長度 Lot 清單只要逐筆 `LotRegistry.AddLot(lot, HT160_LOT_SOURCE_SECS, "", "")` 即可落地，
> 不需要新建資料結構。這是本案最大的利多。

### 2.2 阻擋性前置條件（利空，必讀）

HT160 `SecsGem/` 目前是 **2026-05-24 從 HT172 移植的可編譯 skeleton**，
所有 SECS-II 解析 / 封包 / HSMS 傳輸基本元件**全是空函式**：

| 引擎元件 | HT160 現況（`uHGemEquipment.cpp`）| 後果 |
|---|---|---|
| `DataItemIn(...)` | `return -1;`（永遠失敗）| 收不到任何欄位值 |
| `GetDataItemLenAndType(...)` | 設 `Len=0, Type=LIST`，`return 0` | 取不到長度/型別 |
| `GetDataItemLenAndTypeAndDelete(...)` | `Len=0; return 0` | 讀不到 list 長度 |
| `InitLocalHead / DataItemOut / SendLocalData` | 空函式 | **送不出 S2F42 回覆** |
| `S2F42_Host_Command_Acknowledge()` | 印 log、`return 1` | remote command 全停用 |

引擎規模對比（行數）凸顯落差：

| 檔案 | HT172（真實引擎）| HT160（空殼）|
|---|---:|---:|
| uHGemClass.cpp | 3638 | 153 |
| uHGemEquipment.cpp | 9251 | 166 |
| uHGemForm.cpp（HSMS 傳輸 + DFM）| 1670 | **缺檔** |
| uHGemHT172/160.cpp | 1779 | 176 |
| UsecegemMainFrom.cpp | 986 | 74 |
| TasmInfo.cpp | 106 | **缺檔** |
| **合計** | **約 17,430** | **約 569** |

**結論**：若不先把真實引擎補上，就算把 `SET_LOT_INFO` 解析寫得再正確，
S2F41 封包仍**進不來、也回不出去**，等於空轉。因此本案分兩階段。

---

## 3. 兩階段施工範圍

### Phase 0（前置 / 阻擋）：移植真實 SECS 引擎

把 HT172 的 `THGem` 真實實作 + HSMS 傳輸層移植進 HT160。

- 移植 `uHGemEquipment.cpp` 真實版（DataItemIn/Out、GetDataItemLenAndType、
  InitLocalHead、SendLocalData、HSMS 收送）。
- 移植 `uHGemForm.cpp`（+ 對應 `.dfm`、`.h`）作為 HSMS socket 傳輸表單。
- 移植 `TasmInfo.cpp`。
- `uHGemClass.cpp` 補上各 `SxFy` 真實分派（目前只有 `SendUnsupported`）。
- `.bpr` / `.mak` 加入新增的 OBJFILES / FILELIST / 對應 `.dfm` 資源。
- **保持非 FSM**：依 HT160 治理，不得引入 FSMRunner / *Step.h / *Table.cpp / *Exec.cpp。
- 編碼：`SecsGem/*.cpp/.h/.dfm` 維持原 Big5，不可用 UTF-8 工具改寫含中文行。

> 風險：Phase 0 是大型移植（約 1.7 萬行），且牽涉 HSMS socket、SECS-II codec、
> SVID/CEID/Report 註冊整套。建議獨立成另一份施工計畫單獨評估與分批驗證。
> 本計畫書 Phase 1 之後的程式碼，以「Phase 0 已完成、引擎可正常收送」為前提。

### Phase 1（本案）：可變長度 SET_LOT_INFO 解析 + S2F42 回覆

僅動 `uHGemHT160.cpp` 的 `S2F42_Host_Command_Acknowledge()`（+必要時 `TFSECS::MySFCode` 路由）。

---

## 4. Phase 1 詳細設計

### 4.1 封包格式（需先定案，含歧義釐清）

使用者草稿同時出現外層 `L[5]` 與內層 `L[3]`，層數對不上（5 疑似沿用舊版 5 欄的殘留；
3 是這次的 Lot 數）。Lot 數既然不固定，**外層不應再寫死數字**。提供兩個正規方案：

**方案 A（推薦，扁平可變長度）**

```
S2F41
L[2]
  A "SET_LOT_INFO"
  L[n]                 <- n = Lot 數量（可變）
    A "U91T06L1A0"
    A "U91T06L1A1"
    A "U91T06L1A2"
    ...
```

**方案 B（保留一層外包，貼近草稿巢狀）**

```
S2F41
L[2]
  A "SET_LOT_INFO"
  L[1]
    L[n]               <- n = Lot 數量（可變）
      A "U91T06L1A0"
      ...
```

> 建議採 **方案 A**：最少巢狀、與「Lot 數不固定」語意一致、解析最單純。
> 最終以客戶端（EAP）實際送的封包為準，須在開發前向對接方確認。

### 4.2 解析虛擬碼（以方案 A、引擎已可運作為前提）

```cpp
else if(S.AnsiPos("SET_LOT_INFO")==1)
{
    int n, len;
    unsigned char Type;
    char str[256];

    // 讀 body 的 L[n]，n = Lot 數量
    if(HGem->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)==1)
    {
        if(n==0)
        {
            HCACK=2;                                   // 空清單 -> 拒絕（參數錯）
        }
        else if(HSys.Sys.SystemStart==true || HasICUnderMachine()==true)
        {
            HCACK=4;                                   // 生產中 / 機內有 IC -> 不可改
        }
        else
        {
            LotRegistry.Clear();                       // 設計決策：覆蓋式（見 §4.4-D1）
            HCACK=0;
            for(int i=0; i<n; i++)
            {
                HGem->GetDataItemLenAndType(len, Type);
                if(Type==HType.ASCII_TYPE)
                {
                    HGem->DataItemIn(len, Type, str);  // 取一筆 Lot ID
                    AnsiString lot = str;
                    LotRegistry.AddLot(lot, HT160_LOT_SOURCE_SECS, "", "");
                    if(i==0)
                        fMain->edLotNo->Text = lot;    // 設計決策：首筆回填 UI（見 §4.4-D2）
                }
                else
                {
                    HCACK=2;                            // 型別不符 -> 參數錯
                    break;
                }
            }
        }
    }
    else
    {
        HCACK=1;                                        // 格式錯
    }
}
```

### 4.3 S2F42 回覆（與 HT172 一致）

```cpp
HGemPtr->InitLocalHead(2,42,0);
HGemPtr->DataItemOut(2, HType.LIST_TYPE, NULL);
HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
HGemPtr->SendLocalData();
```

成功時 `HCACK=0x00` → `L[2]{ B[1] 0x00, L[0] }`，符合需求「收到後沒問題回傳 0」。

### 4.4 設計決策點（已拍板 2026/06/10）

| 編號 | 決策 | 選項 | **定案** |
|---|---|---|---|
| D1 | 收到新清單時 | (a) `Clear()` 後覆蓋 / (b) 累加合併 | **(a) 覆蓋** |
| D2 | 是否回填 `fMain->edLotNo` | (a) 首筆回填 / (b) 不回填 | **(a) 首筆回填** |
| D3 | 生產中收到此命令 | (a) 拒絕 HCACK=4 / (b) 允許 | **(a) 拒絕** |
| D4 | HCACK 碼語意 | 自訂 | **0=成功（客戶已確認），1=格式錯，2=參數錯，4=忙碌（生產中）；其餘碼保留** |
| D5 | Lot 數上限 | `HT160_MAX_LOT=64` | **64 夠用；超過回 HCACK=2** |

> D4 說明：客戶目前僅確認 `0=成功`，其餘碼未定義，採本機自訂 1/2/4；
> 日後若客戶 EAP 規格明定不同數值，只需改 handler 內常數。

---

## 5. 影響檔案清單

| 階段 | 檔案 | 動作 |
|---|---|---|
| Phase 0 | `SecsGem/uHGemEquipment.cpp/.h` | 移植真實引擎 |
| Phase 0 | `SecsGem/uHGemForm.cpp/.h/.dfm` | 新增（HSMS 傳輸）|
| Phase 0 | `SecsGem/uHGemClass.cpp` | 補真實 SxFy 分派 |
| Phase 0 | `SecsGem/TasmInfo.cpp/.h` | 新增 |
| Phase 0 | `ht160s.bpr` / `ht160s.mak` | 加 OBJFILES / FILELIST / 資源 |
| Phase 1 | `SecsGem/uHGemHT160.cpp` | 改寫 `S2F42_Host_Command_Acknowledge()` SET_LOT_INFO 分支 |
| Phase 1 | `SecsGem/UsecegemMainFrom.cpp` | 視需要確認 `MySFCode` S2F41→S2F42 路由（現已存在）|
| 既有，不改 | `CosFunction.cpp/.h`（`LotRegistry`）| 直接呼叫 `AddLot/Clear`，無需改動 |

---

## 6. 驗證計畫

1. **編譯驗證**（每次改檔後）：刪對應 `.obj` → `scripts/ops/build-ht160s.ps1`（或單檔 make）。
2. **Phase 0 連線驗證**：用 SECS Host Simulator（`d:\AI_Area\Tool\HT160S_SECS_Simulator`）
   建立 HSMS 連線、S1F13/F14 上線、確認可收送基本訊息。
3. **Phase 1 功能驗證**：
   - 送方案 A 之 `S2F41 SET_LOT_INFO`，含 1 筆 / 3 筆 / 64 筆 / 0 筆 / 65 筆。
   - 確認 `LotRegistry.GetLotCount()` 與送入數一致；`FindLotIndex()` 可查得各 LotID。
   - 確認回 `S2F42` 之 HCACK：正常 0x00；空清單與超量回非 0。
   - 生產中（`SystemStart=true`）送命令 → 應回 HCACK=4 且不改 Registry。
4. **回歸**：確認既有 OLP `THT160AutomationServer::SetLotInfo`（`AutomationServer.cpp` L441）
   與 SECS 路徑不衝突（兩者都會寫 `edLotNo` / Registry，需確認來源優先序）。

---

## 7. 風險與注意事項

- **最大風險＝Phase 0**：沒有真實引擎，Phase 1 無法端到端運作。Phase 0 是大型移植，
  建議單獨立案、分批移植與驗證，不要與 Phase 1 綁成單一工項。
- **HT172 唯讀鐵律**：Phase 0/1 只能讀 HT172 `uHGem*.cpp` 作參考，**絕不寫入 `D:\HT172`**。
- **非 FSM 鐵律**：SECS 擴充維持程序式 / `switch`/`if-else` 風格，不得引入 FSM 架構。
- **Big5 編碼**：`SecsGem/*.cpp/.h/.dfm` 編輯保留原編碼，含中文行勿用 UTF-8 工具改寫。
- **OLP 與 SECS 雙路徑**：HT160 已有 OLP 版 `SetLotInfo`（單 Lot 寫 `edLotNo`）。
  SECS 版落 `LotRegistry`（多 Lot）。兩條路徑的優先序與互斥需在 D1/D2 一併拍板。
- **HCACK 語意**：須與客戶 EAP 規格逐碼對齊，避免機台回 0 但 host 期望其他碼。

---

## 8. 白話文總結

你問的「可不可行」——**可行，而且 160 早就把多 Lot 的櫃子（`LotRegistry`，可放 64 個 Lot，
還特地留了一個「SECS 送來的」標籤）準備好了**。把不固定數量的 Lot 名單一筆筆塞進去，
程式很短、不難寫。

但有一個「先別高興太早」的前提：**160 的 SECS 通訊引擎目前是模型屋，不是真房子**。
門（HSMS 連線）沒裝、收信拆信的手（解析/封包）是假動作（讀資料永遠回失敗、回覆送不出去）。
HT172 那邊是真房子（約 1.7 萬行），160 這邊只有約 569 行的樣品骨架。

所以建議拆兩步走：

| 步驟 | 做什麼 | 工程量 | 沒做會怎樣 |
|---|---|---|---|
| Phase 0 | 把 172 的真引擎搬進 160（含 HSMS 傳輸）| 大 | S2F41 進不來、S2F42 出不去，寫了也空轉 |
| Phase 1 | 寫可變長度 SET_LOT_INFO 解析 + 回 0 | 小 | — |

我的建議：**Phase 0 另外立一張單獨評估**（它才是真正花力氣的地方），
Phase 1 的程式我已經把虛擬碼寫好（§4.2），等引擎到位就能直接接。
你只要先幫我拍板 §4.4 那五個決策（尤其 D1 覆蓋還是合併、D4 HCACK 碼跟客戶怎麼對），
我就能把 Phase 1 收尾。

---

## 9. Phase 0 執行進度（2026/06/10）

### 9.1 本次完成（已編譯通過、build 綠燈）

採用「**先搬 codec、不搬整顆引擎**」的安全增量策略。原因：HT172 的 `THGem`
是 `public TForm`（含 `*.dfm`、`ClientSocket1/ServerSocket1` socket、3 個大型 DFM GUI、
`IncludeAllHeader.h` 與 HT172 全域物件），與 HT160 的 `THGem : public TComponent`
**架構不相容**，整顆複製這個 session 不可能編出乾淨的 build。因此只萃取
「不依賴 form/socket 的純位元組 codec」。

已實作並通過編譯（`SecsGem/uHGemEquipment.cpp` / `.h`）：

| 項目 | 內容 |
|---|---|
| `HSMS_Head_Struct` | HSMS 10-byte 標頭結構（DeviceID/S/F/W/PType/SType/SystemByte） |
| heap `LocalBuffer` | 1MB 動態緩衝（ctor `new[]`、dtor `delete[]`），取代 HT172 的 64MB 靜態陣列 |
| `GetLengthOfType()` | 各型別每筆位元組數（已對 HType byte 表逐一驗證） |
| `GetLengthByte()` | SECS-II 長度欄 1~3 byte MSB 編碼 |
| `ConvertLocalData()` | MSB-first 寫入 |
| `CreateLocalHead()` | 4-byte 長度 + 10-byte HSMS 標頭組裝 |
| `InitLocalHead()` | 偶數 F 回覆沿用 Remote.SystemByte、奇數 F 遞增 EquipmentSystemByte |
| `DataItemOut(int,uchar,void*)` | 全型別（ASCII/BINARY/BOOLEAN/INT/UINT/FT/LIST）大端編碼，忠實移植自 HT172 |
| `DataItemOut(uchar,AnsiString)` | ASCII 便利多載 |
| `SendLocalData()` | **buffer-only**：完整封裝到 LocalBuffer，僅 log，尚未接 socket |
| `GetLocalBuffer()`/`GetLocalLength()` | 提供未來傳輸層取出已編碼訊息 |

驗證：`scripts/ops/build-ht160s.ps1` → `ExitCode=0`，`EXE\ht160s.exe` 重新產出。
所有 HT160 SecsGem 檔維持純 ASCII（英文註解），未動 `.bpr`/`.mak`（無新增檔案）。

### 9.1b 第二批完成（2026/06/10，build 綠燈 `ExitCode=0`，EXE 2,107,392 bytes）

接續搬入「**接收端 decode codec**」與「**Phase 1 SET_LOT_INFO handler**」，已編譯通過。

已實作並通過編譯（`SecsGem/uHGemEquipment.cpp` / `.h`）：

| 項目 | 內容 |
|---|---|
| `SReceiveData`(TStringList) | 收信 token 模型；ctor `new`、dtor `Clear+delete`，`bReceiveData`/`iReturnCode` 生命週期 |
| `GetSMLLenthByte()` | 由格式 byte 低 2 bit 取長度欄位元組數並還原長度 |
| `StoreToReceiveString()` | 把 type/length/value token 推入 `SReceiveData` |
| `ProcessRemoteHead()` | 拆 HSMS 10-byte 標頭（Ptr[4..13]）寫入 `Remote` |
| `ProcessSML()` | 遞迴 SECS-II body 解析器，全型別（LIST/ASCII/BINARY/BOOLEAN/INT/UINT/FT），**已剝除 HT172 的 GUI 顯示呼叫**，只保留 tokenize |
| `DecodeReceiveBody()` | Clear+ProcessRemoteHead+RunLength=14 起跑 ProcessSML 的對外入口 |
| `DataItemInSub()` | 依 token 驗型別/長度並取值（ASCII/BINARY/BOOLEAN/各 INT/UINT/FT） |
| `DataItemIn(int,uchar,void*)` | 對外多載，sticky `iReturnCode` |
| `DataItemIn(int,uchar,AnsiString&)` | **新增** ASCII/數值轉 AnsiString 多載（Phase 1 解析所需） |
| `GetDataItemLenAndType()` | peek 下一筆 type+len 不消耗 |
| `GetDataItemLenAndTypeAndDelete()` | **HT160 語意：Type 為 by-value 期望值**，驗型別後消耗 type+len token |
| `ResetReturnCode()`/`GetReturnCode()` | sticky 回傳碼控制 |

Phase 1 handler（`SecsGem/uHGemHT160.cpp` `S2F42_Host_Command_Acknowledge()`）：

- 讀外層 `L[2]` → 命令名（ASCII）→ `SET_LOT_INFO` 分支讀內層 `L[n]`。
- D1 覆蓋：`LotRegistry.Clear()` 後逐筆 `AddLot(lot, HT160_LOT_SOURCE_SECS, "", "")`。
- D2 首筆回填 `fMain->edLotNo->Text`。
- D3 `HSys.Sys.SystemStart || HasICUnderMachine()` → HCACK=4 拒絕。
- D4 HCACK：0=成功/1=格式錯/2=參數錯/4=忙碌。
- D5 `n>HT160_MAX_LOT(64)` → HCACK=2。
- 回 S2F42 `L[2]{ B HCACK, L[0] }`（`InitLocalHead(2,42,0)`+`DataItemOut`+`SendLocalData`），並 log `HCACK`/`Lots`。
- 新增 include：`main.h`/`database.h`/`csystem.h`/`CosFunction.h`。

### 9.1c 第三批完成（2026/06/10，build 綠燈 `ExitCode=0`，EXE 2,121,216 bytes）

搬入「**HSMS-SS socket 傳輸層**」與「**訊息分派迴圈**」，已編譯通過、relink 成功。

傳輸層（`SecsGem/uHGemEquipment.cpp` / `.h`，採路線 B `TComponent` 程式建立 socket）：

| 項目 | 內容 |
|---|---|
| `ScktComp.hpp` | 由 `vclie.bpi` 提供 `TClientSocket`/`TServerSocket`（已確認 `ht160s.bpr` 套件清單含 `vclie.bpi`） |
| `ClientSocket1`(主動)/`ServerSocket1`(被動) | ctor 程式建立，`stNonBlocking`/`ctNonBlocking`，掛 8 個事件 handler；預設 **被動(設備監聽)** |
| `StartCommunication()`/`StopCommunication()` | 依 `bActiveMode` 連線或監聽 `sDefaultPort`(預設 5000)；停止時關閉 socket、清緩衝 |
| `SetHsmsMode(bool)`/`IsConnected()`/`IsSelected()` | 主被動切換與狀態查詢（`iHsmsState` NOTCONNECTED/CONNECTED/SELECTED） |
| `RecvBuffer`(TMemoryStream) | 收信半包組裝；`ReadFromPeer` append→`ProcessReceiveBuffer` |
| `ProcessReceiveBuffer()` | 依 4-byte 長度前綴切出完整 frame，malformed/100MB sanity 防呆，`memmove` 壓縮殘包 |
| `HandleControlMessage()` | 自動回覆 Select.req→Select.rsp(進 SELECTED)、Linktest.req→Linktest.rsp、Separate.req→關連線 |
| `HandleDataMessage()` | 取 S/F→`DecodeReceiveBody`→`GemLogic->Dispatch(S,F)` |
| `SendControlReply()` | 組 10-byte 控制標頭（echo SessionID/SystemBytes）送出 |
| `SendLocalData()` | 改為 SELECTED 時 `ActiveSocket->SendBuf(LocalBuffer, LocalLength_4)`，否則只 log |
| `SetGemLogic(HTGem*)` | 傳輸層→邏輯層分派回指標 |

分派層（`SecsGem/uHGemClass.*`）：

- 新增 `virtual void HTGem::Dispatch(int S,int F)`：switch 路由 S1F1→S1F2、S1F13→S1F14、
  S2F17/25/31/41→對應 reply、S5F3/5、S7F1/3/5/17/19、S10F3/5、S14F1，未知 S/F → `S9F3`。
- `HT160Gem` ctor 呼叫 `HGemPtr->SetGemLogic(this)` 完成傳輸↔邏輯雙向接線。

### 9.1d 第四批完成（2026/06/10，build 綠燈 `ExitCode=0`，EXE 2,123,776 bytes）

「**App 啟動時啟用連線**」：`SecsGem/UsecegemMainFrom.cpp` `TFSECS::GemInitial()` 原本硬碼
`SetDefaultAddressAndPort("127.0.0.1","5098","0")` 且不開 socket，現改為：

- 讀 `system\General.ini [SECS]`（與 `[ColorCCD]`/`[TopCCD]` 同樣 ship+hardware install tier）：
  `Enable`(預設 1)、`Address`(127.0.0.1)、`Port`(5098)、`DeviceID`(0)、`ActiveMode`(0=被動監聽)。
- `USE_SECS_GEM = (Enable>0)?1:0`；Port 越界回 5098。
- `SetDefaultAddressAndPort` 套用 ini 值；`SetHsmsMode(ActiveMode!=0)`；`Enable>0` 時呼叫 `StartCommunication()`。
- 新增 include `<IniFiles.hpp>`。同步在 `system/General.ini` 加上文件化 `[SECS]` 區段。
- 關閉：THGem dtor 已將 socket `Active=false`，無需額外 StopCommunication 呼叫。

> 至此 S2F41 可端到端：App 啟動→監聽 5098→host Select→SELECTED→S2F41 SET_LOT_INFO→Dispatch→
> S2F42_Host_Command_Acknowledge→回 S2F42。尚需實機/模擬 host 驗證。

### 9.1e 第五批完成（2026/06/11，build 綠燈 `ExitCode=0`，EXE 2,130,432 bytes）

「**SV/EC/CEID/Report 註冊引擎＋事件報告＋狀態查詢**」。

關鍵發現：`SetSVDataPointer`/`SetECDataPointer`/`SetCEIDContent`(兩個多載)/
`SetReportIDContent`/`SetAlamData` 原本**全是空 stub**，`EventReport` 只寫 log。
所以「補真實 SVID/CEID/EC 對應表」必須先把整個**無 form 的註冊＋序列化後端**做出來
（HT172 把這些存在 GUI `TStringGrid`，HT160 `TComponent` 無 form 不能照搬）。

- `uHGemEquipment.h`：新增 `TGemSVItem`/`TGemECItem`/`TGemReportItem`(SVIDs[64])/
  `TGemCEIDItem`(ReportIDs[32]) struct；private `TList *SVList/ECList/ReportList/CEIDList`
  （ctor `new TList`、dtor 逐項 `delete`＋刪 list）；`Find{SV,EC,Report,CEID}Item`；
  `DataItemOutSVItem`/`DataItemOutSVValue(unsigned)`、`GetSVCount`/`GetSVIDByIndex`。
- `uHGemEquipment.cpp`：`SetSVDataPointer`/`SetECDataPointer` 真正存（去重）；
  `SetCEIDContent`(兩個)/`SetReportIDContent` 真正存；SV 值編碼規約：ASCII SV 的 `Ptr`
  視為 `AnsiString*`、數值 SV 視為 `&value`(配 `Len`)。`EventReport(dataID,ceid)` 改為
  **組裝並送出 S6F11**：`L[3]{U4 DATAID, U4 CEID, L[a]{ L[2]{U4 RPTID, L[b]{<SV value>…}} }}`，
  以 CEID→Report→SV 走訪；非 SELECTED 時略過。
- `uHGemClass.cpp` `Dispatch`：加 even-F 守門（host 回覆如 S6F12 記 log 後丟棄，不回 S9F3）＋
  `S1F3→S1F4` 路由。
- `uHGemHT160.cpp/.h`：`S1F4_SelectedStatusReply` 覆寫（讀 `L,n` SVID，用 AnsiString 數值多載＋
  `StrToIntDef` 統一解析 U1/U2/U4/I1/I2/I4；`n==0`=全部 SV；未知 SVID 回空 `L[0]`）。

> 至此原本的 `AddSV`(3 筆 SVID 1000-1002)／`AddCEID`(31 事件→report1)／
> `AddReprot`(report1={1000,1001,1002}) **真正生效**：host 可用 S1F3 查 SV，
> 機台 `EventReport()` 會送出 S6F11 事件報告。尚需實機/模擬 host 驗證。

### 9.2 Phase 0 尚未完成（待後續單獨評估與實作）

> 本次只完成「傳送端 encode codec」，以下為使 SECS 引擎真正可運作仍欠缺的部分。
> 在這些完成前，S2F41 仍進不來、S2F42 仍送不出去，Phase 1 解析碼無法端到端驗證。

1. ~~**HSMS socket 傳輸層**~~ **已完成（9.1c）**：`TClientSocket`(主動)/`TServerSocket`(被動)
   程式建立，`SendLocalData()` 已接 `ActiveSocket->SendBuf(...)`，收信半包組裝＋
   Select/Linktest/Separate 控制訊息自動回覆皆完成。~~**仍缺：在 App 啟動時呼叫
   `StartCommunication()`（含從 General.ini 讀 IP/Port/主被動模式）才會真正開埠。**~~
   **啟用已完成（9.1d）**：`GemInitial()` 讀 `[SECS]` 並 `StartCommunication()`。
2. **接收端 decode 管線**：~~socket `OnRead` → 取 4-byte 長度組包 → `ProcessRemoteHead`
   拆 HSMS 標頭 → 遞迴 SECS-II body 解析器（HT172 的 `ProcessSML`）寫入
   `SReceiveData`(TStringList) 模型 → `DataItemInSub`/`GetDataItemLenAndType*Sub`。~~
   **decode codec 已完成（9.1b）**：`ProcessRemoteHead`/`ProcessSML`/`DecodeReceiveBody`/
   `DataItemInSub`/`GetDataItemLenAndType*` 皆已實作並 build 綠燈。~~仍缺 socket `OnRead`
   把實際收到的位元組餵進 `DecodeReceiveBody()`~~ **已完成（9.1c）**：`ReadFromPeer`/
   `ProcessReceiveBuffer`/`HandleDataMessage` 已把實際位元組餵進 `DecodeReceiveBody()`。
3. ~~**訊息分派迴圈**：收完一筆訊息後依 `MySFCode(S,F)` 路由到對應處理（S2F41→S2F42 等）。~~
   **已完成（9.1c）**：`HTGem::Dispatch(S,F)` switch 路由＋`HT160Gem` ctor `SetGemLogic(this)`
   接線；`HandleDataMessage` 解碼後呼叫 `GemLogic->Dispatch(S,F)`。
4. **真實 SVID/CEID/EC 註冊表**：~~HT160 `AddSV`/`AddCEID`/`AddEC` 目前僅樣品數筆，
   需補齊與機台對應的完整清單（含資料指標綁定）。~~
   **註冊引擎已完成（9.1e）**：SV/EC/CEID/Report 儲存＋`EventReport`→S6F11＋S1F3/S1F4 皆通。
   **仍缺：擴充 SVID/ECID 目錄**——把 `AddSV`/`AddEC` 綁到機台實際持久變數
   （生產計數／機台狀態／Lot 等，引擎就緒、僅需逐筆 `SetSVDataPointer`；依防幻覺鐵律
   綁定前須先 grep 確認變數型別與生命週期）。`AddEC` 仍為空。
   另 `S1F11/S1F12`(SV namelist)、`S2F13/F14`(EC 查詢)、`S2F15`(EC set) 仍為 stub。
5. **GUI 監看表單**：HT172 的 `uHGemForm`（SML log、連線狀態、手動送訊）尚未移植；
   HT160 走 `TComponent` 無 form，需決定是否另建輕量監看 UI 或純 log 檔。
6. **Host command handlers**：~~`uHGemHT160.cpp` 的 `S2F42_Host_Command_Acknowledge()`
   等仍為 stub（Phase 1 才會填 SET_LOT_INFO，其餘 host 命令亦待補）。~~
   **`S2F42_Host_Command_Acknowledge()` 的 SET_LOT_INFO 分支已完成（9.1b）**；
   其餘 host 命令（PAUSE/ONLINE_REMOTE 等）仍待補。
7. ~~**AnsiString 版 `DataItemIn` 多載**~~ **已完成（9.1b）**。
8. **架構決策（已拍板：採 (B)）**：Phase 0 完整化採 **(B) 維持 `TComponent`，自寫精簡 SECS 引擎**
   （socket＋ProcessSML 解析＋分派），符合 HT160 non-FSM／輕量風格。
   不採 (A)（把 `THGem` 改 `TForm`＋移植 HT172 3 個 DFM）。工程量仍屬「大」，
   宜另立單獨施工單分批進行（socket → 收信解析 → 分派 → 註冊表）。

### 9.3 仍待 user 決策（不影響本次 build）

- ~~§4.4 D1~D5~~ **已拍板（2026/06/10）：D1 覆蓋、D2 首筆回填、D3 生產中拒絕、D4 0/1/2/4、D5 64 夠用。**
- ~~Phase 0 後續走 (A) 或 (B) 架構路線。~~ **已拍板：採 (B) TComponent 自寫精簡引擎（2026/06/10）。**

