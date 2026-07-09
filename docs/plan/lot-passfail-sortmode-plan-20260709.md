# Lot+PassFail 分流模式 — 設計與修改計畫

> 建立日期：2026-07-09 分支：feat/iosetview-172-refactor
> 狀態：**已實作（2026-07-09），待現場驗證**。sim `-Full` EXIT 0、real-machine（`SOFT_SIMULATE` off）`-Full` EXIT 0、dev define 還原後再 build EXIT 0。未 commit（依慣例由使用者上機驗證）。
> 已採用選項：per-cell `iPassClass` 凍結；錯誤/無 Lot IC → Error Auto。
> 已內建 blocker 對策：分類凍結（§4-1）、對稱 error gate（§4-2，PassFail 以 `PassClass>0` 收斂）、Mode-first 相容+clamp+legacy mirror（§4-3）、PassBin>0 Start gate（§4-5，於 `CheckLotDataReady`）、綁定 .ini `Mode=` 標頭+跨模式清除（§4-6）、執行中拒改 PassBin（§4-5，於 `SaveBinSettingMap`，以 binding count>0 判定執行中）。
> **未做（後續）**：§4-7 溢位到 Error Auto 的操作告警（與 LotBin 現況相同，非退步）；Error Bin 執行中鎖定；docs/manual 更新。
> 目標：在既有 `pnlSortModeBox` 的 Normal / By Lot+Bin 兩模式之外，新增第三種 **By Lot+PassFail**。

---

## 0. 一句話總結

分流綁定表 `THT160LotBinBinding` 本質上已是泛型的 `(LotID, int) -> Auto`。
- **Lot+Bin**：第二鍵 = `Bin`，一個 Lot 最多 N 個桶。
- **Lot+PassFail**：第二鍵 = `PASS/FAIL 分類碼`（由 `Bin == PassBin` 導出），一個 Lot 最多 **2 個桶**。

所以**重用同一套綁定機制**（FCFS 配置、持久化、讀取、Lot 生命週期清除、StateRecord dump 全部不動），
只換「餵給綁定表的整數」。但有一個關鍵差異必須處理（見 §4 blocker）：
`Bin` 是 IC 內生屬性、掃描後不變；`PASS/FAIL` 是從**可變設定 `PassBin` 導出**的，
因此必須在 Top CCD 掃描當下**凍結**分類碼到 cell 上（比照 `Bin/Lot/Code2D` 的凍結搬運），
讀取端一律讀凍結值、**不得重算**，否則掃描時與擺放時會算出不同的桶 → 誤分流。

---

## 1. 舉一反三：Lot+Bin 教會我們的每一件事，對應到 Lot+PassFail

| 面向 | Lot+Bin（現況） | Lot+PassFail（新） |
|---|---|---|
| 分類鍵 | `(LotID, Bin)` | `(LotID, PassFailClass)`，PASS=1 / FAIL=2 |
| 桶數/Lot | 最多 N（不同 Bin 數） | **最多 2**（PASS-Auto、FAIL-Auto） |
| 鍵來源 | `Bin`（2D 反查、內生、不變） | `Bin == BinAreaMap.GetPassBin()` 導出（設定可變） |
| 綁定時機 | Top CCD 掃描 `ResolveAuto`（先到先得） | 相同 |
| 綁定表 | `THT160LotBinBinding`（`LotID\x01int -> Auto+1`） | **同一個類別、同一支 .ini** |
| 讀取（擺放） | `GetMappedAutoIndex` → `FindAuto` 只讀 | 相同（改讀凍結的分類碼） |
| 溢位 | 非 Error Auto 用完 → Error Auto | 相同 |
| 錯誤 Bin/無 Lot | 略過綁定 → Error Auto | 相同（**兩端都要對稱 gate**，見 §4-2） |
| 搬運載體 | cell 已帶 `iBin`、`iLot`、`sCode2D` | **新增 `iPassClass`**（比照 iLot/Code2D 三個載體） |
| 每模式互斥旗標 | `bUseLotBinSortMode`（bool） | 升級為 `iSortMode`（int enum，三態） |
| 靜態 Bin 表編輯 | Lot+Bin 時鎖住（`setup.cpp:1047`） | 相同，但 **PassBin 選單必須保持可設**（見 §4-5） |
| PASS/FAIL 判定 | 已存在但**只寫 log**（`aSortArm.cpp:1364`） | **同一判定升級為路由決定因素**（改讀凍結值） |

**核心 aha：** Lot+Bin 當初為了搬運「這顆 IC 屬於哪個 Lot / 哪個 2D」而新增了 `iLot`、`sCode2D` 兩個 per-cell 載體並凍結在掃描當下。Lot+PassFail 只是**再加一個同性質的 per-cell 載體 `iPassClass`**，把「這顆 IC 是 PASS 還是 FAIL」在掃描當下凍結。綁定表對第二鍵是什麼毫不在意，所以其餘機制原封不動。

---

## 2. 變數/符號對照（要引用什麼）

### 2.1 模式旗標（升級：bool → int enum）
- 現況：`GeneralSetting.bUseLotBinSortMode`（`GeneralSetting.h:41`、預設 `.cpp:32`、讀 `.cpp:96` `[SortMode] UseLotBinMode`、寫 `.cpp:161`）。
- 新：
  ```cpp
  // GeneralSetting.h  (top-level enum, NOT enum class — no C++11)
  enum THT160SortMode { smNormal=0, smLotBin=1, smLotPassFail=2 };
  int iSortMode;                 // 取代 bUseLotBinSortMode
  // inline accessors (放在 class body，讓 ~12 個讀取點改動最小)
  bool IsNormalSortMode()      { return iSortMode==smNormal; }
  bool IsLotBinSortMode()      { return iSortMode==smLotBin; }
  bool IsLotPassFailSortMode() { return iSortMode==smLotPassFail; }
  bool IsDynamicBindingMode()  { return iSortMode==smLotBin || iSortMode==smLotPassFail; }
  ```
- ini key：`[SortMode] Mode`（整數）。

### 2.2 綁定表（重用，不新增類別）
- `THT160LotBinBinding`（`CosFunction.h:328`、`.cpp:1600~1795`）。
- `MakeKey / ResolveAuto / FindAuto / GetBindingByIndex / Save/LoadFromIni` 全部沿用。
- 建議把成員參數 `Bin` 改名為 `KeyId`（純文件性，語意更準；非必要）。

### 2.3 PASS/FAIL 設定（已存在，recipe 層）
- `THT160BinAreaMap.PassBin`（`CosFunction.h:93`、預設 0 `.cpp:304`、讀 `.cpp:547` `[BinAreaMap] PassBin`、寫 `.cpp:585`、`GetPassBin/SetPassBin`）。
- UI：Bin Setting 頁 `cbbPassBin`（`setup.cpp:720~731` 建立「0=OFF + 各 grid bin」、`:764` 存檔）。

### 2.4 新增：唯一的分類器（單一真相來源）
```cpp
// THT160BinAreaMap 新增（宣告 CosFunction.h:~114，定義 .cpp:~600 GetPassBin 旁）
// 回傳 0=ERROR/不適用, 1=PASS, 2=FAIL
int GetPassFailClass(int Bin);   // 內部用 GetPassBin() + IsErrorBin()
```
放在 `THT160BinAreaMap` 因為分類需要 `GetPassBin()`（`:600`）與 `IsErrorBin()`（`:397`），兩者已是成員。
**掃描端與擺放-log 端一律呼叫這一支，不得各自 inline 計算。**

### 2.5 新增：per-cell 凍結載體（比照 iLot / sCode2D）
- `TMyTray`：新增 `int iPassClass[][]`（鏡射 `iBin/iLot`）。
- `TTrayMotor`：新增 `SetTrayPassClass / GetTrayPassClass`。
- `TSortArmSlotState`：新增 `int PassClass`（鏡射 `LotIndex + Code2D`），於 `FindPickCells` 擷取。

---

## 3. 修改計畫（分層清單，含 file:line）

### 層 A — 設定/持久化（GeneralSetting）
| 位置 | 動作 |
|---|---|
| `GeneralSetting.h:41` | 刪 `bool bUseLotBinSortMode`，改 `int iSortMode` + enum + 4 個 inline helper（§2.1）。ASCII 註解。 |
| `GeneralSetting.cpp:32` (SetDefault) | `iSortMode=smNormal;` |
| `GeneralSetting.cpp:96` (Load) | **Mode-first 向後相容**：`int legacy=Ini->ReadBool("SortMode","UseLotBinMode",false)?1:0; iSortMode=Ini->ReadInteger("SortMode","Mode",legacy); if(iSortMode<0||iSortMode>2) iSortMode=smNormal;`（見 §4-3） |
| `GeneralSetting.cpp:161` (Save) | `Ini->WriteInteger("SortMode","Mode",iSortMode);` **並保留** `Ini->WriteBool("SortMode","UseLotBinMode", iSortMode==smLotBin);`（downgrade 安全，見 §4-3） |
| `system/General.ini` | 不需手改；靠 Load 相容讓現場 `UseLotBinMode=1` → `smLotBin`。此檔即為向後相容回歸樣本。 |

### 層 B — UI（maintenance，pnlSortModeBox）
決策：**用單一 `TRadioGroup`**（非兩個 checkbox、非 combo）。專案內有先例 `rgPnpUseSuck`；TRadioGroup 執行期自動生成子 radio，DFM 無子元件可被 designer 破壞；ExtCtrls 已隨 TPanel 引入。

| 位置 | 動作 |
|---|---|
| `maintenance.h:292` | 刪 `TCheckBox *chkUseLotBinMode;`，改 `TRadioGroup *rgSortMode;`（放 **field 區塊**） |
| `maintenance.h:372` | 刪 `chkUseLotBinModeClick`，改 `void __fastcall rgSortModeClick(TObject*);`（放 **handler 區塊**；`__published` 內不得有 `//` 註解，見 `maintenance.h:54-62` 規則） |
| `maintenance.dfm:2026-2040` | **手工**替換 TCheckBox 物件為 TRadioGroup（`Items.Strings`=('Normal','By Lot+Bin','By Lot+PassFail')、`ItemIndex=0`、`Columns=1`）。參照 `setup.dfm` 的 rgPnpUseSuck 區塊。**切勿用 BCB 設計器開啟**（會 strip 元件、破壞 Big5/Glyph）。建議 **OnClick 於程式碼綁定**（ctor 內 `rgSortMode->OnClick=rgSortModeClick;`，比照 `setup.cpp:386`），DFM 不寫 `OnClick=` 行。 |
| `maintenance.dfm:2008-2025` | `lblLotBinModeHint.Caption` 改寫成三模式說明並提示需重開軟體 + PassBin 於 Bin Setting 頁設定。ASCII、保留 `+` 續行語法。 |
| `maintenance.cpp:1104-1105` (Load) | `if(rgSortMode!=NULL) rgSortMode->ItemIndex=GeneralSetting.iSortMode;`（須在 `bLoadingHardwareSettings=true` 區間內，避免觸發 OnClick 彈窗） |
| `maintenance.cpp:1174-1175` (Save) | `if(rgSortMode!=NULL){ int idx=rgSortMode->ItemIndex; GeneralSetting.iSortMode=(idx>=0&&idx<=2)?idx:smNormal; }` |
| `maintenance.cpp:1894-1904` (handler) | 改名 `rgSortModeClick`：**首行保留** `if(bLoadingHardwareSettings) return;`；寫回 `iSortMode`；`RefreshHardwareSettingsStatus()`；保留「請重開軟體」`ShowMyMessage`。 |
| `maintenance.cpp:1215-1231` (EditLock) | **TRadioGroup 不是 TCheckBox**：從 `TCheckBox *Locked[13]` 移除 index 2、陣列縮為 `[12]`、重排索引、迴圈上界 13→12；另加一行 `if(rgSortMode!=NULL) rgSortMode->Enabled=(MachineRun.bRunning==false);`（見 §4-6） |

**清潔替換**：`chkUseLotBinMode` 四處（Load/Save/EditLock/Click）與 .h、.dfm 一次刪乾淨，勿留孤兒（比照已死的 `chkUseTrayDatumModel` 之教訓）。

### 層 C — 路由核心（凍結分類 + 對稱 gate）
| 位置 | 動作 |
|---|---|
| `CosFunction.cpp:~600` | 新增 `THT160BinAreaMap::GetPassFailClass(int Bin)`：`if(IsErrorBin(Bin)) return 0; if(GetPassBin()<=0) return 0; return (Bin==GetPassBin())?1:2;` |
| `aLoader.cpp:1739-1741`（掃描成功） | 改為：`if(IsDynamicBindingMode()){ int key; bool bErr; if(IsLotBinSortMode()){ key=Bin; bErr=BinAreaMap.IsErrorBin(Bin); } else { key=BinAreaMap.GetPassFailClass(Bin); bErr=(key==0); } TrayMotor->SetTrayPassClass(X,Y,key); if(HitLotIndex>=0 && !bErr) LotBinBinding.ResolveAuto(HitLotIndex, key); }`（**錯誤/無 Lot 不 ResolveAuto**，見 §4-2） |
| `aLoader.cpp:1828-1829`（BindManual2D） | 同上，走同一支 helper，讓手輸 2D 與自動掃描一致。 |
| `aSortArm.cpp` `FindPickCells` | 擷取 cell 的 `PassClass` 到 `Slot[].PassClass`（比照 LotIndex/Code2D）。 |
| `aSortArm.cpp:856-880` `GetMappedAutoIndex` | 新增 PassFail 分支：`if(IsLotPassFailSortMode()){ bFixedArea=true; int cls=Slot對應的凍結PassClass; if(LotIndex>=0 && cls>0){ int a=LotBinBinding.FindAuto(LotIndex,cls); if(a>=0) return a; } return ErrorAuto; }`。**讀凍結值，不重算。** LotBin 分支維持不動。（注意：此函式現簽名為 `(BinData,LotIndex,&bFixedArea)`，需讓它能取得該 slot 的凍結 PassClass — 見 §5 決策）。 |
| `aSortArm.cpp:1364-1367`（place-time PASS/FAIL log） | 改讀**同一個凍結 PassClass**（不要用 `Slot.TrayData` 重算），刪除「routing untouched」註解。讓 log 與實際路由一致（見 §4-1、§4-4）。 |

### 層 D — 週邊（診斷/顯示/setup/docs；多數不動）
| 位置 | 動作 |
|---|---|
| `cStateRecordHT160.cpp:557` (JSON SortMode) | 三態字串：Normal/LotBin/LotPassFail；建議加 `"PassBin"` 數值欄。 |
| `cStateRecordHT160.cpp:558` (binding dump) | PassFail 模式下把中間欄顯示為 `PASS`/`FAIL`（`GetBindingByIndex` 回傳的是分類碼）。 |
| `cStateRecordHT160.cpp:744` (text) | `bUseLotBinSortMode=0/1` → `iSortMode=` + 值（+ 模式名）；[Config gates] 區塊加 PassBin。 |
| `main.cpp:946` `ShowUnloadAutoInfo` | PassFail 模式下 per-Auto 面板顯示 `PASS`/`FAIL`+Lot，而非分類碼整數；非 PassFail 分支的 LotPanel 保持清空不殘留。 |
| `setup.cpp:1047` grid 鎖 | gate 改 `IsDynamicBindingMode()`；但確保 **PassBin 選單仍可設**（見 §4-5）。 |
| `setup.cpp:720/764` `cbbPassBin` | 邏輯不動（已是分類器讀的設定）。 |
| Lot 生命週期 `main.cpp:2132/2377/3073/3082/3090/3115` | **不動**（`LotBinBinding` 清除/還原對鍵語意無關；PassBin 是 recipe 層，不可在 Lot Start/End 清除）。 |
| `deviceinfo.cpp:190` `AddPassFail` / `ePassFail` 欄 | 不動（既有欄，語意由「參考」升級為「實際路由結果」）。 |
| docs `15-lotbin-mode.md / 06-config.md / 05-maintenance.md / 02-overview.md / 14-module-flows.md` | 補第三模式語意（PASS/FAIL 導出、每 Lot 最多 2 桶、錯誤→Error Auto、分類凍結保證）。UTF-8。 |

---

## 4. 改變影響層面 — 對抗式審查揪出的關鍵風險（必須內建於設計）

> 這些是 workflow 4 位對手 reviewer 的結論；前 3 條是 **blocker**，若不處理會靜默誤分流。

### 4-1【blocker・正確性】分類必須凍結，不可在擺放時重算
`PassBin` 是可變全域（`cbbPassBin`、`SetPassBin`、recipe `LoadFromIni` 皆會改）。若掃描時綁定 `(Lot,PASS)`、擺放時用**當下** `PassBin` 重算得到 FAIL → `FindAuto` 找不到 → 落 Error Auto，且預留的 PASS-Auto 被孤立。
**對策**：掃描當下把分類碼**凍結**到 cell（`SetTrayPassClass`），擺放端只讀凍結值（§3 層 C）。

### 4-2【blocker→重要・正確性】掃描端與讀取端的「錯誤 Bin / 無 Lot」gate 必須對稱
現況 `aLoader` 掃描成功分支**無** `IsErrorBin` 檢查就 `ResolveAuto`，而 `aSortArm:870` 讀取端**有** gate。PassFail 下錯誤 Bin 會算成 FAIL=2（看似正常鍵）→ 掃描端佔掉一個 Auto、讀取端卻走 Error Auto → 孤立綁定 + 分歧。
**對策**：兩端都套「`IsErrorBin(Bin) || LotIndex<0` → 不綁定、走 Error Auto」，經由同一 helper（§3 層 C 已含）。

### 4-3【blocker・設定遷移】向後相容要 Mode 優先，並清理舊鍵
- **Mode-first**：`iSortMode = ReadInteger("Mode", legacyBoolAsInt)`，legacy 只當「Mode 不存在時的預設」。否則升級過的機台（磁碟仍留 `UseLotBinMode=true`）若之後選 PassFail，重開會被打回 LotBin。
- **舊鍵殘留**：TIniFile 不刪未觸碰的鍵；Save 時保留寫 `UseLotBinMode=(iSortMode==smLotBin)` 使兩鍵永不矛盾（且對舊 exe downgrade 安全，PassFail 會退成 Normal）。
- **clamp**：讀入後 `if(iSortMode<0||iSortMode>2) iSortMode=smNormal;`（比照 `GeneralSetting.cpp:106/144` 既有慣例）。

### 4-4【重要・正確性】兩處餵給分類器的 Bin 來源不同
掃描端用 `FindByCode2D` 的原始 `Bin`；讀取端 `GetSlotRoutingBin` 在 `BinValue<=0` 時**退回 `TrayData`**（IC 狀態碼 EMPTY/UNCHECK/OK，非 bin 號）。若 `PassBin` 恰為 1/2 會誤判。
**對策**：凍結分類碼即根治（§4-1）；另在動態模式下嚴格以凍結 2D bin 為準，`BinValue<=0` 兩端一律當 no-bin → Error Auto，**不退回 TrayData**（順帶強化 LotBin）。

### 4-5【blocker・現場安全】PassBin 的兩個致命情境
- **PassBin==0（OFF）卻選 PassFail**：每顆都算 FAIL → 全擠一個 Auto 再溢位 Error，靜默。→ **強制** Lot Start gate：`if(IsLotPassFailSortMode() && GetPassBin()<=0)` 擋開始並跳 Note。（非「optional」）
- **Lot 執行中改 PassBin**：`SaveBinSettingMap`（`setup.cpp:738/1063`）**無 `bRunning` gate**，改了就重新分割在跑的 Lot、混料。→ 執行中**硬鎖** `cbbPassBin` 且在 `SaveBinSettingMap` 拒絕變更；同時 §4-2 的靜態 grid 鎖不可連 PassBin 選單一起鎖死（否則無法設 PASS bin）。
- **Error Bin 執行中可改**：它同時是 FCFS 略過目標與溢位目標，改了會讓現行桶撞上新 Error Auto。→ 動態模式執行中一併鎖 Error Bin 選單。

### 4-6【重要・跨模式污染】綁定 .ini 無模式標記
`LotBinBinding.ini` 的中間欄在 LotBin 是真 Bin、在 PassFail 是 1/2，磁碟上無法區分。模式切換只「提示重開」不強制清除；`RestoreLastWorkOrder` 也不驗模式。
**對策**：(a) 任何模式切換 → `LotBinBinding.Clear()` + 重寫/刪 .ini；(b) .ini 寫入 `Mode=` 標頭，`LoadFromIni` 遇模式不符即 `Clear()`。

### 4-7【重要・容量】每 Lot 只有 2 桶 × 多 Lot 易耗盡 Auto
5 個可用 Auto，PassFail 每 Lot 吃 2 個；≥3 個並行 Lot（或多個 Auto 被停用）就溢位 Error（混良品與錯誤品），靜默。
**對策**：Lot Start 檢查「啟用的非 Error Auto 數 ≥ 2×預估並行 Lot」，或至少在某桶落到 Error Auto 時跳 Note，不要靜默路由。

### 4-8【UI】TRadioGroup 相關
- Locked 陣列型別問題（§3 層 B 已處理）。
- OnClick reentry guard：程式設定 `ItemIndex` 會觸發 OnClick，須保留 `bLoadingHardwareSettings` 早退（比照 `rgPnpUseSuck`/checkbox 慣例）。
- 清潔替換，勿留孤兒元件。

---

## 5. 待你拍板的設計選項

1. **分類凍結載體**（建議 A）
   - **A：per-cell 凍結 `iPassClass`**（比照 iLot/Code2D）— 最貼合現有架構、結構性根治 §4-1/4-4，但要動搬運鏈三個載體。
   - B：只在 Lot Start 凍結 `PassBin` 到 lot 變數 + 嚴格以 `BinValue` 為準 — 改動較少，但依賴不變式。
2. **`GetMappedAutoIndex` 如何取得 slot 的凍結分類**：改簽名多帶一個 `RoutingKey/PassClass` 參數（由 caller 從 slot 算好），或函式內依 mode 讀 slot。建議前者（明確、無隱藏狀態）。
3. **PassFail 下錯誤/無 bin 的 IC**：→ Error Auto（建議，與 LotBin 一致）還是併入 FAIL 桶？
4. **顯示**：Auto 面板 / StateRecord 顯示 `PASS`/`FAIL`（建議）還是分類碼數字？
5. **舊 ini 鍵**：保留鏡射（建議，downgrade 安全）還是 `DeleteKey`？

---

## 6. Build gate（動工後）
- 每次 C++/DFM/專案編輯：刪對應 `.obj` 再編譯；wiring 變更 → full build（`GeneralSetting.h` 型別改變 → **強制 full rebuild**）。
- 另驗真機組態：註解掉 `MachineType.h` 的 `#define SOFT_SIMULATE`，`-Full` 確認 exit 0，還原 define 再 build。
- 編碼檢查：`scripts/ops/check-ht160s-source-encoding.ps1`（禁 `EF BF BD`、UTF-8 BOM）。
- 首選：`scripts/ops/build-ht160s.ps1 -Clean`。
