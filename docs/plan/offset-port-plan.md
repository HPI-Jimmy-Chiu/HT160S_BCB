# Offset 功能完整導入計畫 (HT160S_BCB)

> 狀態：**P1-P4 全部完成，雙編譯 (sim + 真機) 皆 exit 0** (2026-06-23)。待使用者上機驗證。
> 參考來源：舊 HT160S (`D:\HT160S -Original 20260323\Code_V300A\Program_HT160S_20240806_20241111-SECSGem`) 為主、HT172 (`D:\HT172\HT172_Program_V1.0.25.0_20260420`) 為模式參考。
> 關聯記憶：`uoffset-port-plan`、`bcb6-coding-style`、`edit-tool-corrupts-big5-source`、`bcb-designer-save-strips-components`、`bcb6-dfm-events-must-be-published`、`ht160s-build-command`。

---

## 1. 目標

完整導入 Offset（每產品/每 workfile 的位置微調）畫面與資料層，使操作員能在不破壞基準 Teach 值的前提下，對 56 個位置欄位做非破壞性微調，並依 workfile 切換套用不同的 offset。

目前狀態（已查證）：
- `uOffset.{h,cpp,dfm}` 為空殼 (`TfOffset` 無欄位、無邏輯；dfm 僅 18 行)。
- `main.cpp:548-550` 的 `sbOffsetClick` 已接 `ShowTopForm(fOffset, sbOffset)`，點按只開到空白表單。
- `cprod.cpp:146-148` `UpdateAllParameter()` 為空 stub（折算的安裝點）。
- 全樹無 `Prod` 結構、無 `RUN_OFFSET`、無 `cinitial`（重構時整組移除）。

---

## 2. 已鎖定的設計決策（使用者 2026-06-23）

- **Route A**：忠實 3 層模型（base + offset → effective）。使用情境＝「相同基準 Teach、每產品微調、非破壞性」。
- **實作 A1（apply-time fold，motion 不動）**：現有 `Teach` 結構保持為 **effective**（motion 直接讀，0 改動）；新增 `TeachBase`（Teach 畫面編輯/存檔的基準）＋ `Offset`（Offset 畫面編輯）。折算在 `UpdateAllParameter()`：`Teach = TeachBase + Offset`。
- **儲存**：Teach 畫面改存 **base**（沿用現有 `tech.ini` / `data\<WorkFile>.tech`）；Offset 獨立存 `data\<WorkFile>.ofs`；effective Teach = base + offset。

> 註（2026-06-23 更正，先前描述有誤）：HT172 的**主要/實際**模型**就是持久式**，與本案相同。`UpdateAllParameter()`（`cprod.cpp:285`）→各模組 `Update*Parameter()` 於執行期計算 `effective = fTeach->edX + fOffset->edX`（證據 `aMagArm.cpp:103+`，數十行），offset 留在 `.ofs`、base(tech.ini) 不動、每次重算再套用、**從不歸零**。正常 Update 鈕 `sbOffsetUpdateClick`（`uOffset.cpp:98`）只存 `.ofs`+`UpdateAllParameter()`。
> 另有一個**選配且本版未接線**的「重對位 Re-alignment」按鈕 `sbOffsetReAligmentClick`（`uOffset.cpp:140`）會呼叫 `SetToTech`（`uOffset.cpp:117`：`Teach += Offset; Offset = 0`）把 offset 永久 bake 進 base 並歸零；但該呼叫在本版**註解掉**（`uOffset.cpp:146` 樣板佔位），非 172 日常行為。
> 結論：本案持久式＝**對齊 172 主行為**。差異僅在儲存（172 讀表單 edit 文字；我們讀 `Teach` struct，更乾淨）。我們用**指派式** `Teach = TeachBase + Offset`（見 §5），天生 idempotent。可選日後加同款「重對位 bake」按鈕作 superset。

---

## 3. 三層資料模型

| 層 | 全域變數 | 持久化檔案 | 由誰編輯 | motion 是否讀 |
|---|---|---|---|---|
| Base（基準） | `TeachBase`（新增，型別＝`TEACH`） | `system\tech.ini`（優先）或 `data\<WorkFile>.tech` | Teach 畫面 | 否 |
| Offset（差值） | `Offset`（新增，型別＝`RUN_OFFSET`，僅 56 活欄位） | `data\<WorkFile>.ofs`（新增，TIniFile） | Offset 畫面 | 否 |
| Effective（生效） | `Teach`（現有，不動） | 不落地（執行期由折算產生） | 無（折算寫入） | **是（99 處）** |

**相容性**：現有 `tech.ini` 內的值即「現行生效的 teach 值」。重新詮釋為 base 後，若 `.ofs` 尚不存在 → `Offset=0` → `Teach = TeachBase + 0 = 原值`，**行為零變化**，直到操作員設了第一個 offset。無需資料遷移。

---

## 4. 對應欄位（56 可映射 + 5 排除）

依舊 160 `SetTechDataToProd()`（`cinitial.cpp:589-758`）中「`= Teach.* + Offset.*`」的欄位，且在 live `uteach.h` 有對應 base 欄位者：

| 群組 | 數量 | 欄位 |
|---|---|---|
| Loader1 Y | 4 | FeedTray / DischargeTray / FirstCCD / FirstSort |
| Loader2 Y | 4 | FeedTray / DischargeTray / FirstCCD / FirstSort |
| Loader CCD X | 1 | `LoaderCarFirstCCDXPosition` |
| SortArm→X | 9 | Loader1/2 X、Auto1-6 X、BottomCCDFirst X |
| SortArm→Loader Z | 8 | Loader_1/2 的 Z1-Z4 |
| SortArm→Auto Z | 24 | Auto_1..6 的 Z1-Z4 |
| Auto FirstSort Y | 6 | Auto1-6 CarFirstSortY |
| **合計** | **56** | |

**排除（5，舊 160 有 offset 但 live TEACH 無 base 欄位，無法映射）**：
1. `LoaderCarLastCCDXPositionOffset`（live 只有 First，無 Last）
2-5. `SortArmToBottomCCDZ1..Z4PositionOffset`（live 無此 base）

> 若日後需要這 5 項，必須先在 `TEACH` 結構＋Teach UI＋tech.ini schema 增加 base 欄位（屬另一個較大工項，本計畫不含）。

**永不 offset（保持 Teach-only，折算時純複製 base→effective）**：Empty Feed/Discharge Y、所有 TrayXArm X、Auto Feed/Discharge Y、PitchArmX Min/Max、BottomCCDYCapture，以及整段 Ring/Tape 死碼。

**命名**：`Offset` 結構欄位 = 對應 Teach 欄位名 + `Offset` 後綴（Loader Y 群組舊 160 用 `OffSet` 大寫 S — 宣告前對照舊 160 `uOffset.h` 拼字，內部一致即可，因我們改用自有 INI key）。單位 0.01mm，預設上下限 ±100000（±1000mm）。

---

## 5. 折算公式與觸發時機

**折算（裝在 `cprod.cpp UpdateAllParameter()`）**：對全部 96 個 `TEACH` 欄位：
- 56 個可 offset 欄位：`Teach.f = TeachBase.f + Offset.f`
- 其餘 40 個欄位：`Teach.f = TeachBase.f`（純複製）

指派式（非 `+=`）→ 重複呼叫安全（idempotent），不會累加。

**觸發矩陣**（對齊舊 160 呼叫點）：
1. **開機**：載入 `TeachBase`（沿用現有 teach 載入）＋載入 `Offset`（新增）後 → `UpdateAllParameter()`。
   - P1 工項：找出現有開機載 teach 的呼叫點，於其後插入 offset 載入 + 折算。
2. **Teach 畫面存檔**：存完 base → `UpdateAllParameter()`（重折算）。
3. **Offset 畫面 Apply**（`sbOffsetUpdate`）：`StorOffsetValue()` 讀 UI → `SaveWorkFile(.ofs)` → `UpdateAllParameter()`。
4. **workfile / recipe 切換**：重載 `TeachBase` + `Offset` → `UpdateAllParameter()`。

---

## 6. 表單設計（新建小型 byte-safe DFM）

舊 160 `uOffset.dfm` 達 ~12062 行、4 大頁含 Ring/Tape 死頁、~145 個 TEdit — **不整包搬**。新表單：

- 單一 `TPageControl`，**3 大頁**：
  - **Loader**：9 欄（8 Y + 1 CCD X）
  - **SortArm**：41 欄（9 X + 8 Loader-Z + 24 Auto-Z）→ 內部再分子頁籤（X / Loader-Z / Auto-Z）避免擁擠
  - **Auto**：6 欄（FirstSort Y）
  - 共 56 個 TEdit。
- **以陣列驅動，仿 live Teach 的 `TECH_PARA` 模式**：建 `OFFSET_PARA[]`（`{ int MotorSelect; TEdit *edEdit; int *iPara; int iMax; int iMin; ... }`），用 `int *iPara` 綁到 `Offset` 欄位 — 不做每欄硬綁，DFM 維持精簡。
- 按鈕：Apply/Update、Re-alignment (bake)、**Clear All（全部清除，model B：只清暫存記憶體+重整，需按 Apply 才存檔/生效，2026-06-23 加）**、Exit、右鍵 SetMax/SetMin 上下限。
- 輸入：沿用 **160S Teach 自己的小鍵盤路徑**（不要搬 HT172 的 `ShowQwertyKey`）。

**byte-safe 製作鐵則**（依記憶）：
- 用 `scripts/ops/bcb6-bytesafe-edit.ps1` 手寫 `.dfm/.h/.cpp`；component 名稱純 ASCII。
- `__published` 欄位宣告**全部排在 event handler 之前**（否則 designer 報 "Incorrect method declaration"）。
- DFM 綁定的 event handler **必須在 `__published`**（否則 ctor 期 `EReadError`）。
- form class body 內**不放註解**。
- 載入乾淨前**絕不**在 IDE designer 開新表單（designer save 會默默砍掉元件 + 對應 `.h` 宣告）。

---

## 7. 檔案 / IO

| 項目 | 設計 |
|---|---|
| `.ofs`（每 workfile offset） | `HSys.CurrentDir\data\<WorkFile>.ofs`，**TIniFile**（非舊 160 binary FormSysTools）。WorkFile 來源＝`lastset.ini [LastSet] cob_MainWorkFile`（同 `GetCurrentRecipeName()` / Teach 的 `data\<WorkFile>.tech`）。key 格式對齊 Teach（`ed_<欄位名>`），value 為 mm 小數字串（`value/100.0`）。 |
| workfile 選擇 | 已存在：`lastset.ini` 的 `cob_MainWorkFile`（`CosFunction.cpp:115/129`、`uteach.cpp:347`）。**沿用**，不需新建選擇器。 |
| `OffsetLimit.ini` | （選配，視 §10 範圍決策）`system\OffsetLimit.ini`，`[Offset]` 內 `<Edit>_Max` / `<Edit>_Min`，預設 ±100000。右鍵 SetMax/SetMin 寫回。 |
| 說明文字 OffsetMessage.DB | 舊 160 用 Paradox/BDE `TTable` — **建議不移植**（避免 BDE 依賴）。改以 inline 靜態字串或簡單 INI 提供同等說明，或 MVP 先略。 |

---

## 8. 不移植清單（do-not-port）

- 舊 160 binary `FormSysTools::Open/Save/LoadFormData`（改 TIniFile）。
- Paradox/BDE `OffsetMessage.db`（`TTable`）。
- HT172 `ShowQwertyKey` 小鍵盤（用 160S Teach 的）。
- HT172 的 StoreArm 20 位 / 3 Magazine / SortArm1,2 ZA-ZH / PushStoreArm / Pitch（機構不同）。
- 舊 160 死頁：Ring / Tape / RingCatchArm / CoverArm / LowerArm 及其 ~24 個註解欄位。
- A2 路線（重建 `Prod` 結構 + 改 99 處 motion 讀取）— 明確不採用。

---

## 9. 階段計畫（每階段以 build gate 收尾）

> Build：`scripts/ops/build-ht160s.ps1`（`-Clean` 一般 / `-Full` 結構變更刪全部 obj）。
> 編碼檢查：`scripts/ops/check-ht160s-source-encoding.ps1`。
> 真機 build：註解掉 `MachineType.h` 的 `#define SOFT_SIMULATE` 跑 `-Full` 確認 exit 0，再還原重建。

> **完成紀錄 (2026-06-23)**：P1-P4 全做完。雙編譯 gate 通過 — sim (`SOFT_SIMULATE` on) `-Full` exit 0、真機 (`SOFT_SIMULATE` off) `-Full` exit 0、還原後再 sim `-Full` exit 0；exe 產出於 `EXE\ht160s.exe`。`MachineType.h` 已還原 byte-identical。變更檔：`uteach.{h,cpp}`、`cprod.cpp`、`uOffset.{h,cpp}`、`main.cpp`。表單採全程式碼建構（bare .dfm，不進 designer）。P2 另含 172 式「重對位 (bake)」按鈕。

- **P0（已完成 2026-06-23）**：欄位盤點與映射確認（56 映射 / 5 排除）。本計畫 §4 已收斂並查證。
- **P1 — 資料層**：
  1. `uOffset.h` 新增 `RUN_OFFSET`（56 活欄位）+ `extern Offset`；`uOffset.cpp` 定義 `Offset`。
  2. 新增 `TeachBase`（型別 `TEACH`）+ extern；將 Teach 畫面 `TechPara[].iPara` 由 `&Teach.x` 改指 `&TeachBase.x`（載入/存檔即操作 base）。
  3. `cprod.cpp UpdateAllParameter()` 實作折算（§5）；全 96 欄位複製，56 欄加 offset。
  4. Offset 載入/存檔（`OpenOffsetFile()` / `SaveOffsetFile()`，TIniFile，`data\<WorkFile>.ofs`）。
  5. 觸發接線：開機載入後、Teach 存檔後、workfile 切換時呼叫 `UpdateAllParameter()`。
  - **Gate**：`-Full` build exit 0（結構新增）；確認 `.ofs` 不存在時行為零變化。
- **P2 — 表單**：依 §6 byte-safe 重建 `uOffset.{dfm,h,cpp}`（3 頁、56 TEdit、`OFFSET_PARA[]` 驅動）。移植 `InitialOffsetParameter`/`StorOffsetValue`/Apply/Exit。`FormShow` 載入 `.ofs` 並填 UI。
  - **Gate**：build exit 0；`sbOffset` 可開新表單、可編輯、Apply 後 `Teach` 生效；保護 `main.cpp:1345 SmokeShowTopForm(fOffset)` selftest 不卡（`FormShow` 不做阻塞 I/O / 不 ShowModal）。
- **P3 — 上下限與說明**（範圍見 §10 決策）：`OffsetLimit.ini` 自動產生 + 右鍵 SetMax/SetMin；說明文字以 inline / INI 取代 Paradox。
  - **Gate**：build exit 0；上下限讀寫正確。
- **P4 — 驗證**：`-Full` + 真機 build（`SOFT_SIMULATE` off）exit 0；交付後由使用者上機驗證：offset 正確套用、base 未被污染、`.ofs` 隨 workfile 切換。

---

## 10. 決策點（已鎖定 2026-06-23）

1. **折算語義 → 持久式（非破壞）**：`Teach = TeachBase + Offset`，offset 保留、可重複套用、隨 workfile 切換。**不採** HT172 歸零式。
2. **範圍 → 完整**：含右鍵 SetMax/SetMin 上下限選單（`OffsetLimit.ini`）＋欄位說明；說明以 **inline 字串 / INI 取代 Paradox BDE**（不引入 BDE 依賴）。P3 為必做，非延後。
3. **5 個排除欄位 → 維持排除**：`LoaderCarLastCCDX`、`SortArmToBottomCCD Z1-4` 不納入本計畫（live TEACH 無 base，無法映射）。

---

## 11. 檔案錨點（live HT160S_BCB）

- 折算安裝點：`cprod.cpp:146-148` `UpdateAllParameter()`（空 stub）。
- Teach 資料層樣板：`uteach.cpp:225-329`（`InitialTeachParameter` 綁定）、`uteach.cpp:427-483`（`OpenWorkFile`/`SaveWorkFile` TIniFile）、`uteach.cpp:331-396`（`GetTeachFileName`/`FindTeachFileName`/`GetWorkFileTeachName`）。
- 按鈕（已接）：`main.cpp:548-550`；selftest smoke：`main.cpp:1345`。
- TEACH 結構：`uteach.h:20-138`（96 int 欄位）；`extern Teach` 在 `uteach.h:368`、定義 `uteach.cpp:26`。
- workfile/recipe：`CosFunction.cpp:115/129`（`GetCurrentRecipeName` 讀寫 `lastset.ini`）。
- motion 讀取（折算後免改）：99 處 `Teach.*`（aSortArm 52 / aAuto1To6 18 / aTrayArm 11 / aLoader 7 / aColor 6 / aEmpty 5）。

### 舊 160 參考錨點
- 折算公式：`cinitial.cpp:589-758`（56 條 `+ Offset.*`，主體 622-720）。
- 折算觸發：`cprod.cpp:363-377` `UpdateAllParameter`；呼叫點 `main.cpp:511`、`setup.cpp:841`、`main.cpp:1604`。
- Offset 結構/邏輯：`uOffset.h:25-149`（`RUN_OFFSET`）、`uOffset.cpp`（`InitialOffsetParameter` 29-247、`StorOffsetValue` 590-620、`OpenWorkFile` 333-349、`SaveWorkFile` 353-365、上下限 510-586、說明 655-689）。

### HT172 模式參考
- **持久折算實例（直接背書本案）**：`aMagArm.cpp:103-179` `UpdateMagArmParameter()` = `effective = fTeach->edX + fOffset->edX`，offset 不歸零；由 `cprod.cpp:285 UpdateAllParameter()` 觸發。
- 選配 bake：`uOffset.cpp:117-152` `SetToTech` + `sbOffsetReAligmentClick`（`Teach+=Offset;Offset=0`，本版呼叫註解掉，未接線）。
- `systools.cpp:702-887` FormSysTools（172 用表單 edit 持久化，**本案不移植**，改 struct+TIniFile）。
