# WhiteList 2D 分流模式（第 4 種 Sort Mode）— 設計與修改計畫

> 建立日期：2026-07-14 分支：feat/iosetview-172-refactor
> 狀態：**v2 重寫（WebAPI-substitution 模型）——偵察 wf_681646da 證實可行；§8 已拍板；待 v2 針對性審查後動工**
> 偵察：wf_fabd2084（SECS/plumbing/config/9045）、wf_681646da（2D→Bin 解析鏈）
> 需求來源：2026-07-14 下午口頭討論（無 session 紀錄），使用者確認：
> 1. 第 4 種模式 `smWhiteList=3`（不取代既有模式）。
> 2. 鍵 = 白名單 2D 碼：**只有名單內的 IC 可以分類；名單外 → Error**。
> 3. 名單來源：`D:\HT160S_BCB` 下新資料夾內的檔案。
> 4. 不比對 PASS/FAIL。
> 5. SECS：客戶要在 Lot Start 時指定是否切白名單模式；修改量最小、不撞 HT9045。
> 6.（修正）**檔案除 2D 資訊外還要有 Bin 別，格式用 JSON。**

> **拍板結果**：
> - Q1（修正版）：白名單檔=含 `2D碼 → Bin` 的 **JSON**；WhiteList 模式下 **2D→Bin 來源由 WebAPI
>   換成此本地檔**；名單內照檔裡 Bin 走 Normal 靜態 Bin→Auto 表分選、名單外 → Error。
> - Q2=LOTSTART 選項對（不做 SET_LOT_INFO 擴充）　Q3=sticky 持久
> - Q4=byte-exact 嚴格比對（大小寫責任在名單提供方，寫進客戶規格）
> - Q5=資料夾 `HT160S_WhiteList\`、固定檔名 JSON　Q6=SVID 66032 未定（可順手做）

---

## 0. 一句話總結（v2 模型）

**WhiteList 模式 = 把 `LotRegistry` 的 2D→Bin 資料來源從「WebAPI 拉取」換成「讀本地 JSON 檔」，
其餘完全走 Normal 流程。** 偵察證實掃描端唯一的 2D→Bin 查詢是
`LotRegistry.FindByCode2D(純2D碼)`（`aLoader.cpp:1776/:1878`），而 WebAPI 只是透過
`LotRegistry.LoadFromJsonString`（`CosFunction.cpp:1365`）把回應灌進同一個 `LotRegistry`。
因此白名單檔用**既有 `"Maps"` JSON schema**、經**同一支 `LoadFromJsonString` 載入**（parser 零改），
命中的 IC 照檔裡 Bin 正常分選（`SetTrayBin`/`OnSorted`/計數全部原封不動），
**未命中（不在檔裡）→ 既有 `FindByCode2D` miss 分支**——只需在 WhiteList 模式把該分支從
「彈 WAR0475 modal」改成「靜默進 Error」（客戶語意：名單外=正常拒收，非操作員例外）。

需要新增的實質邏輯只有三件：
1. WhiteList 模式時**不觸發 WebAPI 拉取**（2 個 call site gate）。
2. WhiteList 模式時 Lot Start **改載本地 JSON 檔**進 `LotRegistry`（重用 `LoadFromJsonString`）。
3. WhiteList 模式時 `FindByCode2D` miss 分支**靜默 Error**（不彈 modal）。

`THT160Bin2DMap` 對帳本是死碼（`Lookup` 零 caller），與本設計無關。

---

## 1. 與既有三模式的關係

| 面向 | Normal | Lot+Bin | Lot+PassFail | **WhiteList（新）** |
|---|---|---|---|---|
| enum | smNormal=0 | smLotBin=1 | smLotPassFail=2 | **smWhiteList=3** |
| 路由 | 靜態 BinAreaMap | 動態綁定 | 動態綁定 | **靜態 BinAreaMap（同 Normal）** |
| 動態綁定表 | 不用 | 用 | 用 | **不用** |
| 2D→Bin 來源 | WebAPI→LotRegistry | WebAPI→LotRegistry | WebAPI→LotRegistry | **本地 JSON→LotRegistry** |
| WebAPI 拉取 | 觸發 | 觸發 | 觸發 | **抑制** |
| 名單外/未命中 IC | WAR0475 modal→操作員 Skip→1001 | 同 | 同 | **靜默 Error（無 modal）** |
| per-cell 新載體 | — | iLot/sCode2D | iPassClass | **無** |
| 新類別/parser | — | — | — | **無（重用 LoadFromJsonString）** |
| IsDynamicBindingMode() | false | true | true | **false（靜態表要可編輯）** |

**凍結原則（天然成立）**：`LotRegistry` 的資料在 **Lot Start** 事件載入一次
（WebAPI 或本地檔），之後整輪常駐、pause→resume **不重載**（WebAPI 本來就只在 Lot Start
觸發、Machine Start/resume 不重拉）。本地檔載入掛在**同樣的 Lot Start 事件**，
所以繼承相同生命週期——**先前審查的「pause→resume 重讀名單」BLOCKER 在此模型自然消失**。

---

## 2. 白名單檔案設計（JSON，重用既有 schema）

### 2.1 位置與命名
- 資料夾 `D:\HT160S_BCB\HT160S_WhiteList\`，固定檔名 **`WhiteList.json`**。
- 路徑 helper 比照 `GetMapFolder` 的 `HSys.CurrentDir + "\\..." ` + `".."` fallback
  慣例（`CosFunction.cpp:730-732`）：`HSys.CurrentDir + "\\HT160S_WhiteList\\WhiteList.json"`。
- **固定檔名**而非「資料夾內最新檔」（偵察建議）：白名單是機台級組態、非每班交付；固定檔名
  對客戶/FE 放檔無歧義，避開 `HT160S_LotInfo` 那套 `yyyymm\dd` 日期分區「午夜換資料夾」與
  「mtime 最新」的脆弱性。程式**只讀不寫**；開機 `ForceDirectories` 建空資料夾引導 FE
  （idiom `main.cpp:2536-2537`）。

### 2.2 格式 —— 重用既有 `"Maps"` schema（parser 零改）
偵察證實 `LoadFromJsonString`（`CosFunction.cpp:1365`）已解析 `"Maps"` 形狀並灌進 `LotRegistry`：
```json
{
  "Maps": [
    { "LotNumber": "LOT001",
      "Items": [
        { "Code2D": "A1B2C3D4E5", "Bin": 1 },
        { "Code2D": "F6G7H8I9J0", "Bin": 2 }
      ]
    }
  ]
}
```
- **`Bin` 是路由 bin**（靜態 BinAreaMap 的 Bin→Auto 表就吃這個值），命中即照此分選。
- 也可用 `"2DIDHistory"` schema（客戶 WebAPI 原格式，含 HBin/SBin/DiePass），若客戶想直接
  存一份 WebAPI 回應當白名單檔——但 `"Maps"` 較精簡、建議用它。
- **大小寫**：byte-exact（Q4）。`m_Code2DIndex` `CaseSensitive=true`；比對端不正規化——
  客戶規格明定名單碼大小寫須與 CCD 讀值一致（§6.6）。

### 2.3 LotID 的處理（v2 模型的關鍵約束，偵察揪出）
`LoadFromJsonString → AddItemEx → AddLot(LotID)`，而 **`AddLot` 對空 LotID 回 -1**
（`CosFunction.cpp:964-966`）→ 該筆丟棄；下游 `OnSorted(LotIndex)` 亦在 LotIndex<0 時 no-op
（產量不計）。**故白名單檔每筆必須帶非空 LotNumber。** 兩種寫法：
- **(a) 對齊真實 Lot**（建議）：`LotNumber` = host `SET_LOT_INFO` 送的真實 Lot ID → 檔案就是
  「WebAPI 回應的本地版」，per-Lot 產量計數與報表完全正確。
- **(b) 合成 Lot**：全部掛在單一 `"WHITELIST"` LotID → lot-agnostic 檔可行，但產量會歸到
  合成 Lot（Lot 清單 UI / WorkOrder.json / SECS lot 報表會多一個 `WHITELIST` lot）。
- → 待拍板 Q7（見 §8）。預設採 (a)，並在客戶規格寫明「檔案 LotNumber 須對齊 SET_LOT_INFO」。
- 2D 碼在 `m_Code2DIndex` 全域唯一：檔內**不得重複碼**（重複回 dup，`CosFunction.cpp:1083-1092`）。
- **clear vs append**（待偵察細節確認、實作前查）：需確認 `LoadFromJsonString` 是否會先清空
  registry。若 append，SET_LOT_INFO 先註冊的 Lot 與檔案 Items 合併；若 clear，載檔會洗掉
  SET_LOT_INFO——影響載入時機與順序，v2 審查列為必查項。

---

## 3. 掃描端與載入端修改（三處）

### 3.1 WebAPI 抑制（2 個 call site，偵察確認唯二觸發）
| 位置 | 現況 | 改為 |
|---|---|---|
| `main.cpp:2215`（btnLotStartClick） | `StartLotWebApiPullAll();` | WhiteList 模式改呼叫 `LoadWhiteListFile()`（§3.2）；否則原樣 |
| `uHGemHT160.cpp:873`（SECS LOTSTART post-block） | `fMain->StartLotWebApiPullAll();` | 同上 |

抑制安全（偵察證實）：pull 是 VCL 主執行緒協作式、無 worker thread；不 arm 就是
`PollLotDataWebApi`/`StartNextLotApiPull` 的 cheap no-op，**不洩漏**。下游只讀 `LotRegistry`，
對「誰填的」無感——本地檔填同一結構即等價。
（實作可選：在 `StartLotWebApiPullAll` 內部單一 choke point gate，但仍須從這 2 站觸發本地載入。）

### 3.2 本地檔載入器 `LoadWhiteListFile()`（新，~15 行，重用 parser）
```cpp
// main.cpp, near the WebAPI orchestration helpers
bool TfMain::LoadWhiteListFile()
{
    AnsiString fn = HSys.CurrentDir + "\\HT160S_WhiteList\\WhiteList.json";
    if(!FileExists(fn)) return false;
    AnsiString text;
    TStringList *raw = new TStringList;
    try { raw->LoadFromFile(fn); text = raw->Text; }
    catch(...) { delete raw; return false; }
    delete raw;
    bool bDup=false; AnsiString dupCode;
    bool ok = LotRegistry.LoadFromJsonString(text, bDup, dupCode);   // SAME path as WebAPI
    if(ok) { RefreshLotListFromRegistry(); SaveWorkOrder(); }         // mirror PollLotDataWebApi:2351-2354
    return ok;
}
```
- 讀檔 idiom 同 `Bin2DMap::LoadFromFile`（`CosFunction.cpp:765-785`：TStringList→Text、try/catch）。
- 灌進 `LotRegistry` 後，掃描端 `FindByCode2D` 命中即回檔裡的 Bin——**掃描端零修改**。

### 3.3 miss 分支靜默 Error（WhiteList 模式唯一的掃描端行為改動）
`aLoader.cpp` 兩站點的 `FindByCode2D` 為 false 分支（現況 `:1810` 彈 `WAR0475`
Retry/Skip/Manual-2D，Skip 才寫 1001 `:1823-1825`）：

```cpp
else   // 2D read OK but not found in LotRegistry
{
    if(GeneralSetting.IsWhiteListSortMode())
    {
        // Not in whitelist file = expected reject, NOT an operator exception.
        TrayMotor->SetTrayBin(State->CcdX, State->CcdY, HT160_BIN_ERROR_NO_BIN_SETTING); // 1001, reuse
        TrayMotor->SetTrayLot(State->CcdX, State->CcdY, -1);
        MachineRun.iNotWhitelisted++;          // dedicated counter (mirror iUnknown2D)
        // set trace 1005 for Production_Log visibility (see 3.4)
        State->CcdTask=1;   // advance silently, no modal
    }
    else
    {
        Ret=ShowMyError("WAR0475", ...);  // existing modal path unchanged
        ...
    }
}
```
- **2D 讀不到（`b2DOk==false`，CCD misread）的路徑不動**——那是 1000 scan-fail，不是白名單拒收，
  仍走既有 retry/operator（誤讀不該被當外來品）。白名單邏輯只在「讀得到碼但查無」的 miss。
- **sentinel 決策**：重用既有 **1001**（NO_BIN_SETTING）→ **零 error-bin registry 改動**
  （先前 v1 的 6-site error-bin BLOCKER 消失）。在 WhiteList 模式，miss 恆等於「不在白名單」，
  模式本身即可辨義（StateRecord 有 dump 模式）。→ 若客戶要外來品進**獨立實體 Error 區**
  （與 no-read 錯誤分開），才需新增 1002 sentinel + 6-site registry（待拍板 Q8，預設重用 1001）。

### 3.4 Production_Log 可追溯（trace code，偵察 MAJOR 維持必做）
Production_Log 的 Bin 欄寫 `Slot[].TrayData`（CCD grade）非 BinValue，且 trace 推導
（`aSortArm.cpp:1462-1467`）只認 999/1000——名單外 IC 否則在 log 與正常品無法區分。
- `aSortArm.cpp:1462-1467` trace 鏈加：命中 miss-reject 的 cell（可用 `BinValue==1001 且模式==WhiteList`
  或凍一個旗標）→ `iTrace2D=1005`。
- `deviceinfo.cpp:~221` 映射加 `1005 -> "NotWhitelisted"`（1002 已被 "ParseFail" 佔用，故用 1005）。
- 註：bin-sentinel 與 trace-code 是兩個編號空間，文件標注。

### 3.5 免改清單（偵察確認）
- `GetMappedAutoIndex` 與 4 call sites、`FindByCode2D`、`SetTrayBin`/`OnSorted`/`ResolveAuto`
  路徑、動態綁定表、`setup.cpp` Bin grid 鎖、LED/TFT bin display、cJSON 連結（已在 .bpr）——**全零改**。

---

## 4. enum 擴充 break 清單（偵察證實完整）

| # | 位置 | 動作 |
|---|---|---|
| 1 | `GeneralSetting.h:14` | enum 加 `smWhiteList=3`；`:62-65` 加 `IsWhiteListSortMode()` |
| 2 | `GeneralSetting.cpp:125-126` | clamp 上界 `smLotPassFail → smWhiteList`（否則 ini Mode=3 開機被打回 Normal） |
| 3 | `maintenance.cpp:1183` | SaveHardwareSettings clamp 同步放寬 |
| 4 | `maintenance.cpp:1931` | rgSortModeClick clamp 放寬；**警語改為事實**（iSortMode 活值、下輪 Lot Start 生效）；**加 `HasICUnderMachine()` 守衛**（§6.3） |
| 5 | `maintenance.dfm:2138-2158` | rgSortMode Items 加 `'By WhiteList'`、Height 74→98；**父容器 `pnlSortModeBox` Height 80→~104**（否則第 4 項被裁切）；`lblLotBinModeHint` 文案+高度檢查。手工編輯 DFM，禁 designer |
| 6 | `cStateRecordHT160.cpp:583-584` | 三元加 WhiteList 分支；[Config gates] 加 `WhiteListFile/載入筆數/時戳` dump |
| 7 | `GeneralSetting.cpp:216` | 免改：legacy `UseLotBinMode` 對 WhiteList 寫 false，downgrade 安全 |
| 8 | `maintenance.cpp:1226-1258` EditLock | 免改：`rgSortMode->Enabled=bEnable` 已鎖整組 |
| 9 | `main.cpp:989-990` ShowUnloadAutoInfo | 免改：WhiteList 走靜態反查分支，顯示正確 |

---

## 5. Start 閘門與生命週期

**事件分工（對齊既有架構，不需 v1 的 CheckLotDataReady latch）**：
- **Lot Start**（btnLotStart / SECS LOTSTART）：註冊 Lot + 載入 2D→Bin 資料。
  WhiteList 模式 → `LoadWhiteListFile()`（§3.2）取代 WebAPI 拉取。**載一次、整輪常駐**。
- **Machine Start**（btnStart / SECS START）：`CheckLotDataReady`（`main.cpp:1891-1941`）閘門，
  **只檢查不重載**。

Start 閘門（照抄 PassBin 閘的**只填 Reason+return false** 風格，`main.cpp:1935-1939`；
`MachineStart` 明文 dialog-free `csystem.cpp:1123-1128`——閘內彈 Note 會卡 SECS T3）：
```cpp
if(GeneralSetting.IsWhiteListSortMode() && LotRegistry.GetCode2DCount()<=0)
{
    Reason="WhiteList mode is ON but HT160S_WhiteList\\WhiteList.json is missing/empty !";
    return false;   // operator caller -> ShowMyMessage ; SECS caller -> HCACK
}
```
- 覆蓋面（偵察證實）：UI Start（`main.cpp:1949`）、SECS START（`uHGemHT160.cpp:894`）、
  One Cycle（`main.cpp:1815`）全走 `CheckLotDataReady`，無獨立 HOME-resume start 路徑。
- 警報碼進 `mapAlarmCodeList` SSOT（seed 模式 `database.cpp:972-989`，CSV+S5F1 自動涵蓋）；
  實際 fire 只在操作員 caller 端既有 ShowMyMessage 路徑。
- **開機檢查**：FormShow 初始化加
  `if(IsWhiteListSortMode() && !FileExists(...)) RecordProcess("BOOT: WhiteList mode ON but WhiteList.json missing")`
  （EventLog、免 modal）——無人職守 AMR 線否則要等 host 打 START 失敗才發現。
- 名單外 IC 落點記錄：比照 PFOverflow **不跳 Note、只記錄**（Production_Log trace 1005 + 計數 `iNotWhitelisted`）。

---

## 6. SECS：Lot Start 指定切換白名單模式（沿用 v1 審查後版本）

### 6.1 衝突面結論
`SET_LOT_INFO` 在 9045 不存在（DIFF:82-83）；`LOTSTART` 兩機 body 形狀不同
（9045=`<L[0]>`、HT160=`L[n]{A lotID}`）；S2F41 host→equipment 單向，9045 host 收不到新形狀。
9045 家族「未消費 CPNAME」=靜默忽略+HCACK=0（HSK:172/411）。S2F42 在 VCL 主執行緒跑，改
`iSortMode`+Save 無跨執行緒問題。

### 6.2 建議案 A：LOTSTART 可選 SORTMODE 選項對
```
S2F41 W  <L[2] <A "LOTSTART">
  <L[n] <A "LOT001"> ... <L[2] <A "SORTMODE"> <A "WHITELIST">> >>   // 可選、任意位置、至多一次；值 NORMAL|WHITELIST
```
解析（審查 4 修正內建）：
1. **stream 讀法**：內層 peek Type；LIST → `GetDataItemLenAndTypeAndDelete(pairLen, LIST_TYPE)`
   （START_AGV :923 同 pattern）；**`pairLen!=2` → HCACK=2 且 break**（否則殘 token 被當 lot 註冊幽靈 Lot 還回 HCACK=0）；name/value 用泛型 `DataItemIn(len,Type,AnsiString&)`。
2. **busy guard**：LOTSTART 命令級 guard（`uHGemHT160.cpp:822-825`）弱於 UI 鎖（暫停中可過）→
   **pair 存在且 `MachineRun.bRunning==true` → 整包 HCACK=4**。
3. **commit 順序**：pair 解析進區域變數，套用 `iSortMode`+`Save()` **只在 `HCACK==0` post-block（:854）**。
4. **UI 同步**：套用後若 `fMaintenance!=NULL`，在 `bLoadingHardwareSettings=true` 括號內更新
   `rgSortMode->ItemIndex`（否則 stale maintenance 頁存檔會靜默改回並持久化）。
5. 邊界：pair-only（無 lot）→ HCACK=2；`SORTMODE` 無值/非 A/值域外 → HCACK=2；重複 pair 取最後。
6. 不含 pair 的 LOTSTART → 位元組級相容。
7.（不採納）SET_LOT_INFO 同步支援——依 Q2 拍板只做 LOTSTART。

**live-apply 正當性**：變更僅發生在 `!SystemStart && !HasICUnderMachine && !bRunning`（全閒、無在製品）
且 NORMAL/WHITELIST 皆不經綁定表；UI「請重啟」警語改為事實（§4 #4）。

### 6.3 UI 端同源守衛
`btnLotEndClick` 無條件 `bRunning=false`（`main.cpp:2409-2419`，不查殘料）且 bRunning 純記憶體
→ `rgSortModeClick` 加 `if(HasICUnderMachine()){ 還原 ItemIndex + ShowMyMessage; return; }`——與 SECS 守衛同源。

### 6.4 備案 B：新 RCMD `SET_SORT_MODE`（與 9045 零碰撞，但 host 變更面較大，列備案）。

### 6.5 SVID 66032 回讀（可選）：`SetSVDataPointer(66032, HType.INT_4_TYPE, "Sort Mode", "", &GeneralSetting.iSortMode, ...)`（66032 已驗證空號）。

### 6.6 給 KYEC EAP 的客戶規格書必含
1. HCACK 表（0/1/2/4=busy 整包層級）。2. 值域+大小寫（設備端對值 UpperCase？→依 Q4 byte-exact 則不）。
3. 畸形 pair 行為。4. **省略語意（粗體）**：不帶 pair 不改模式（sticky）、跨重開機持久。
5. HCACK=2 已註冊 lot 不回滾。6. 部署順序（host 先升級收 HCACK=2）。
7. **白名單檔規格**：路徑/檔名/JSON schema/LotNumber 須對齊 SET_LOT_INFO（§2.3-a）/不得重複碼/大小寫。
8. 白名單檔缺失時 START 拒絕的 Reason + S5F1 碼。9. 範例交易 log（`HT160S_SECS_Simulator` 實跑，不偽造）。
10.（Q6）SVID 66032 對照。11. 白名單=資格非額度（同碼重複投料不攔）。

---

## 7. 修改清單總表（v2，較 v1 大幅縮小）

| 層 | 檔案 | 內容 | 風險 |
|---|---|---|---|
| A 設定 | `GeneralSetting.h/.cpp` | enum+helper+clamp（§4 #1-2） | 低；**.h 變更→full rebuild** |
| B UI | `maintenance.h/.cpp/.dfm` | 第 4 radio+父容器高度+clamp×2+警語+HasICUnderMachine 守衛（§4 #3-5、§6.3） | 中；DFM 手工 |
| C 載入 | `main.cpp` | `LoadWhiteListFile()`+WebAPI 抑制 2 站+Start 閘+開機檢查+ForceDirectories（§3.1-3.2、§5） | 低（重用 LoadFromJsonString） |
| D 掃描 | `aLoader.cpp` | 兩站 miss 分支 WhiteList 靜默 Error+計數（§3.3）；**檔首 Big5，非 ASCII 區段須 python byte-splice** | 中（核心路徑，僅 miss 分支） |
| E SECS | `SecsGem/uHGemHT160.cpp` | LOTSTART pair 解析（§6.2）+抑制 WebAPI 改載本地（§3.1） | 中（配 SECS_Simulator 測） |
| F 擺放 | `aSortArm.cpp`+`deviceinfo.cpp` | trace 1005（§3.4） | 低 |
| G 診斷 | `cStateRecordHT160.cpp` | 4-way 名+WhiteList dump（§4 #6） | 低 |
| H 警報 | `database.cpp` | mapAlarmCodeList seed 檔缺失碼 | 低 |
| I 文件 | `docs/manual/*`+SECS Comm Examples+客戶規格（§6.6） | 第 4 模式+檔案 schema+SORTMODE | 低 |
| — 免改 | `CosFunction.*`（parser/FindByCode2D）、`GetMappedAutoIndex`、綁定表、error-bin registry、bin display | 偵察證實零改 | — |

**Build gate**：改 `.obj` 刪後重編；`GeneralSetting.h` 變更→full rebuild；真機組態
（`SOFT_SIMULATE` off）`-Full` EXIT 0 後還原 define 再 build；`check-ht160s-source-encoding.ps1` 過關；
新註解 ASCII 英文。cJSON 已連結、**無需改 .bpr**。

---

## 8. 待拍板問題（實作前必答）

**2026-07-14 追加拍板**：
- **Q7 = (a) 對齊真實 Lot**（已拍板）。白名單檔 `LotNumber` 填 host 宣告的真實 Lot ID
  （本案載體＝LOTSTART，見 Q2；即對齊 LOTSTART 送入的 Lot），per-Lot 產量報表正確。
  客戶規格須寫明「檔案 LotNumber 須對齊 host 宣告的 Lot」。
- **Q8 = (a) 重用 1001**（已拍板）。名單外 IC 走既有 `HT160_BIN_ERROR_NO_BIN_SETTING`(1001)，
  零 error-bin registry 改動；與 no-bin 錯誤同一實體 Error 區（模式本身即可辨義；trace 1005 仍做，§3.4）。
- **Q2 重新確認 = LOTSTART**（2026-07-14 覆核）。曾重議是否改用 SET_LOT_INFO 當載體
  （SET_LOT_INFO 在 HT160 已實作 `uHGemHT160.cpp:693`、語意上「先宣告組態」更乾淨），
  但仍維持 LOTSTART——2D→Bin 載入本就在 LOTSTART 觸發、9045 兩機皆有此指令、byte-相容。
  **SET_LOT_INFO 方案正式封存（不採納）。**

**仍待拍板**：
- **Q6 SVID 66032（host 查模式）要不要做？** —— 與載體無關的**獨立回讀**功能：host 用 S1F3
  查目前 `iSortMode`（0/1/2/3）。成本極小（一行 `SetSVDataPointer`）。**建議做**（host 切模式後可回讀確認）；
  唯一取決於 KYEC EAP 要不要用。
- ~~Q1-Q5、Q7、Q8 已拍板~~（見檔頭與上）。

---

## 9. 審查狀態

**v1 對抗式審查（wf_4f1f139b，gate 模型）** 的發現，在 v2（substitution 模型）下的處置：

| v1 發現 | v2 狀態 |
|---|---|
| BLOCKER 計數污染（名單外灌 OnSorted/BinICCnt） | **消失**：命中走原路徑（正常計數）、未命中走 miss 分支（本就不計數） |
| BLOCKER pause→resume 重讀名單 | **消失**：載入掛 Lot Start 事件（非 Machine Start），繼承 WebAPI 生命週期 |
| BLOCKER error-bin registry 6-site | **消失（若 Q8 重用 1001）**：不新增 sentinel 就不動 registry |
| MAJOR PassClass 凍成 PASS/FAIL | **消失**：未命中不進 PassClass 計算（在命中分支內） |
| MAJOR modal 風暴 | **由 §3.3 解**：miss 分支 WhiteList 靜默 Error |
| MAJOR Production_Log 無痕 | **保留必做**：§3.4 trace 1005 |
| MAJOR LOTSTART busy guard 弱於 UI 鎖 | **保留**：§6.2-2 bRunning guard |
| MAJOR SECS 設模式被 stale 頁改回 | **保留**：§6.2-4 UI 同步 |
| MAJOR pnlSortModeBox 裁切 | **保留**：§4 #5 |
| MAJOR 閘內彈 Note 卡 T3 | **保留**：§5 只填 Reason |
| MAJOR UI/SECS 鎖不等價 | **保留**：§6.3 HasICUnderMachine 守衛 |
| MAJOR Count/Item 格式陷阱、Trim/BOM | **消失**：改用 JSON（cJSON），非 INI Count/Item |
| MINOR 其餘（commit 順序/邊界/警語/開機檢查/大小寫/重複投料/客戶規格） | **保留**：§6.2/§4/§5/§6.6 |

**v2 新風險（待 v2 針對性審查驗證）**：
1. `LoadFromJsonString` **clear vs append** 語意（§2.3）——會不會洗掉 SET_LOT_INFO 的 Lot？
2. 合成/真實 LotID 對 per-Lot 報表、SECS lot 事件、WorkOrder.json 的影響（Q7）。
3. miss 分支移除 modal 後，「該命中卻因檔案 stale 未命中」的 IC 無操作員介入即進 Error——
   是否符合客戶「名單外=拒收」意圖（應為是，但需確認 Retry/Manual-2D 完全移除的可接受性）。
4. WhiteList 模式抑制 WebAPI 後，`GetUsePull()` 為真但被跳過的診斷/log 一致性。
5. 開機 `RestoreLastWorkOrder` 若 `LoadLatest`（`main.cpp:3154`）在 WhiteList 模式下會不會誤載
   `HT160S_LotInfo` 舊 JSON（非白名單檔）而污染。

→ v2 審查針對以上 5 點 + §2/§3/§5 的新程式碼。
