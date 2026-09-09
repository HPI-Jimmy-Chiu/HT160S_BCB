# HT160S per-feature 權限強化 — 設計與施工計畫

狀態：Stage 1 / 2a / 4 完成（`0f42f9a`, `1e9d0d7`）；Stage 2b / 3 待辦
日期：2026-09-09
分支：`feat/amr-lot-identity-d4`

---

## 0. 起點事實（已由 workflow 攻防複驗）

改造前，全機**只有一個**真正的權限閘門：

| 位置 | 擋什麼 | 需求等級 |
|---|---|---|
| `maintenance.cpp:2801` `bCanEdit=HasLevel(ROLE_ENGINEER)` → 2802-2808 七個控件 `Enabled` | 帳號本編輯 | Engineer(2) |

`HasLevel()` 全 repo 就這一個呼叫點。`IsManualOperation()` / `IsSimulationDefault()` / `GetLoginTime()` **零**消費者。

其餘所有功能（Teach、Offset、Motor Test、IO 檢視/強制/自測/IO 表、配方/Setup、Bin map、硬體安裝設定含機種序號、SECS 控制態與上線開關、EC 寫入、AMR 注入、FTP 帳密、清計數、HOME/START/Clean Out、Real/Dummy 切換、警報解除、Lot/工單、ComPort、速度倍率，連「進維修/Setup 畫面」本身）**只被執行狀態互鎖（停機/無 Lot）擋，完全不看使用者等級**。

---

## 1. 架構決策：導入 HT9045 的編號能力表

參考來源：`D:\HT9045\HT9011UC_Code_V3.33.912.0_20260908_Jimmy\cSecurity.h/.cpp`

### 9045 怎麼做

```cpp
// cSecurity.cpp:574  ── 注意：回傳 true = 「有權限」（函式名會誤導）
bool __fastcall TfSecurity::Insufficient(int iType, bool bAlarm)
{
    if(iType==-1) { return (AccessLevel>=iDefHonPrecLevel); }   // Honprec-only 特例
    if(iType<0 || iType>iMaxLevelItem) return false;            // 越界一律拒絕
    if(AccessLevel < LevelSet.AccessLevel[iType]) {
        if(bAlarm) ShowErrorMessage("WAR1676", ...);            // Insufficient privileges
        return false;
    }
    return true;
}
```

- `LevelSet.AccessLevel[256]` — **每個編號槽位一個「所需等級」**，執行期可調
- `TfSecurity` 表單：每槽一個 `TRadioGroup` 選等級，依畫面分頁分組（Main / Tools / Config / IO / Configuration / …）
- 槽位可依機台選配隱藏（`SetVisible`），設定頁只列有效權限
- 呼叫點形狀固定：`if(fSecurity->Insufficient(29)==false) return;`

### 導入價值：高。理由是可量測的

| 證據 | 意義 |
|---|---|
| 9045 有 **266 個**呼叫點、跨 **32 個**檔案 | 這個模式撐得起產品規模 |
| 其中 **191 個是靜默形式** `Insufficient(id,false)` | **靜默查詢才是主流用法**（灰化控件、隱藏頁籤）。我原本只打算做「點擊拒絕」的吵版 → 這是真實設計缺口，被這份參考補上 |
| 等級表存檔、UI 可調 | 客戶/售服改權限**不必重編譯**。若照原計畫把 `HasLevel(ROLE_ENGINEER)` 寫死在 100+ 個點，每次調整都要重出韌體 |

### 要照抄的

1. **編號槽位**：每個受控功能一個穩定整數 ID
2. **每槽所需等級可設定 + 存檔**
3. ~~雙形式 API~~ → **單一靜默 API**（見下方「API 修訂」：使用者裁定沒權限直接反灰，不跳訊息）
4. **越界即拒絕**（fail-closed）
5. **Honprec-only 特例**
6. **槽位依選配隱藏**

### 要**避開**的 9045 缺陷

| 9045 做法 | 問題 | HT160 改法 |
|---|---|---|
| `"d:\\HT9045\\system\\levelset.dat"` 寫死絕對路徑 | 換槽/換機就爛 | `HSys.CurrentDir+"\\system\\security.txt"`（同 login.txt 慣例） |
| `WriteData(..., sizeof(LevelSet))` 二進位 blob | 陣列一長就對不齊、記事本打不開 | **純文字** `slot,level`（延續 login.txt 明文可讀政策） |
| `GetLevelSet()` 內散落魔術索引 `i==35 / 104 / 163` 硬改等級 | 政策散在載入器裡 | 預設值集中在**單一槽位表**，與名稱同列 |
| `Insufficient()` 回傳 true 表示「**有**權限」 | 命名反義，`==false` 才是拒絕，極易誤讀 | 正名 `SecurityAllows()`（唯一查詢，靜默） |
| `bSecurityHave5Level` 讓等級編號在 4/5 級間浮動 | 編號漂移 | HT160 固定 4 級 `EHT160UserRoleLevel`，不引入浮動 |

---

## 2. 已核定政策（使用者 2026-09-09 裁定）

- **粒度 = Hybrid**：專屬工程畫面（Teach / IO / Motor Test / ComPort / Setup）**進入時擋一次**；維修頁內共用面板（SECS 控制態 / FTP / 硬體設定 / 機種序號 / AMR 注入）**逐動作擋**
- **一般工程設定 = Engineer(2)**：Teach、Offset、IO 強制/自測/IO 表、Motor Test、配方建立刪除、Bin map、ComPort、SECS 控制態切換、AMR 注入、硬體安裝設定
- **最敏感 = Honprec(3)**：FTP 帳密檢視/編輯/測試、機種型號/Handler ID/序號、SECS endpoint/ActiveMode/EC 寫入、Accept Host Online 上線開關
- **跑機/Lot/警報/速度 = Operation(0)**（照常生產，但仍註冊成槽位，客戶要調可往上調）；**清計數 → Supervisor(1)**

## 3. 追加硬規則：OP 模式生產必須 Real

Dummy / Has-Tray 是工程測試模式。工程師把機台留在 Dummy 就交班給操作員，產出資料會是假的。

**9045 已有同形狀先例**（`main.cpp:6061` / `6223`，START 前置檢查內）：

```cpp
if(AccessLevel==0 && LastSet.iTester==OFF_LINE)   // OP 級 + 非正常模式
{
    iStartIn=0;
    ShowErrorMessage("MES1052", 0, MMSystem, false, "Main--Start");
    return false;                                  // 拒絕啟動，不自動改模式
}
```

HT160 對應條件是 `iRealDummy != REALLY`。**做法照 9045：在 START 前置檢查拒絕啟動並跳訊息，不自動切換模式**（自動改模式會讓操作員不知道機台狀態變了）。

- 落點：`main.cpp` `sbStart1Click`(2165) 與 `sbOneCycle1Click`(2124) 的前置檢查
- 這是**硬規則，不做成可調槽位**（使用者說「一定要」）
- Real/Dummy 切換本身另立槽位（Engineer(2)）— 9045 也有對應的 `[11] Main - Real/Dummy`

---

## 4. 關鍵地雷（施工必守）

**`ShowMyMessage()` 會停機** — `mymessbox.cpp:83` 內含 `HSys.DecStopAllMotor()` + `HSys.Sys.SystemStart=false`。
它是為「真實機台警報」設計的，停機正是它的目的。

這原本是權限拒絕訊息的地雷：操作員誤點一下受管制按鈕就會停產。
**使用者裁定「沒權限直接反灰不跳訊息」後，此風險結構性歸零** — 權限路徑完全不產生訊息，
所以連 `ShowMyOKMessageNoStop()`（`mymessbox.cpp:154`，OK-only、不停馬達）都不需要。
若未來要為任何權限情境加訊息，必須用 NoStop 版本，絕不可用 `ShowMyMessage()`。

其他：
- 靜默形式（灰化控件）**不得**跳任何訊息 — 它每次頁面刷新都會跑
- 新增權限閘門必須與現有執行狀態互鎖 **AND**，不可取代（停機/無 Lot 的保護要留）
- `SOFT_SIMULATE` 開發版開機即 Honprec，所有閘門在開發版都是 no-op；**只有真機版才驗得到**
- BCB6：無 C++11、保留 `AnsiString`、新註解 ASCII、**行尾必須逐檔確認**（`main.cpp`/`maintenance.cpp`/`main.h` 是 CRLF，`csystem.cpp` 是純 LF）
- 新增 `.cpp` 是**新編譯單元** → 必須進 `ht160s.bpr` / `.mak`，並跑 `-Full`

---

## 5. 施工階段

| Stage | 內容 | 狀態 |
|---|---|---|
| 1 | `SecurityPolicy.h/.cpp`：31 槽位 enum（append-only）＋槽位表（group/name/預設等級）＋明文存檔 `system\security.txt`＋單一靜默查詢 `SecurityAllows()`；接進 .bpr/.mak | **DONE** `0f42f9a` |
| 2a | 維修頁 + 主畫面閘門（畫面進入 + 逐動作，全部靜默灰化） | **DONE** `1e9d0d7` |
| 2b | 其餘檔案逐動作再驗（`uteach` / `uOffset` / `iosetview` / `uMotorTest` / `setup` / `ComPort` / `uHGemLogForm`） | **不做**（使用者 20260909 裁定：畫面入口已擋，現行做法已足夠） |
| 3 | 權限設定 UI（見 §7 設計）— 採**方案 C** 獨立新頁 `tsMaintSecurity` | **DONE** `e6d8ffb` |
| — | 修 Stage 2a 的 stale 反灰（三處閘門非週期性） | **DONE** `0901ea9` |
| — | 帳號頁改 EventLog 稽核（見 §8） | **DONE** `0df4656` |
| — | 維修頁 19 處資訊型彈窗改不停機版（另一 session） | **DONE** `d73dac4`（已合併 `bfc8c7d`） |
| 4 | OP 模式生產必須 Real 硬規則（`MachineStart()`） | **DONE** `1e9d0d7` |

### API 修訂（使用者 20260909 裁定）

原計畫的「靜默＋吵版」雙形式**收成單一靜默查詢**：沒權限直接反灰，不跳訊息。
因此 `PermitAction()` / `PermitHonprecOnly()` 未實作，`SecurityPolicy` 也不依賴
mymessbox / language。停機風險因此歸零（不會有任何權限拒絕路徑碰到 `ShowMyMessage`）。

### Stage 2a 施工中發現的兩個真實風險

1. **FTP 密碼會被無聲清空**（已修）。`maintenance.cpp` FormClose 無條件呼叫
   `SaveFtpConfigFromUi()` 當 commit-on-close backstop，其註解假設「未動過的欄位原值往返」。
   一旦為低權限使用者不載入密碼，Operation 級使用者只要開關維修畫面就會清掉 FTP 密碼。
   → 守衛必須放在 `SaveFtpConfigFromUi()` 最前面，早於任何欄位讀取。
2. **`csystem.cpp` 是純 LF**（非 CRLF），與 `main.cpp` / `maintenance.cpp` 相反。編修必須逐檔確認行尾。

每階段：刪改動 `.obj` → sim `-Clean` → 真機版（關 `SOFT_SIMULATE`）`-Full` → 還原 → 編碼檢查 → commit。

---

## 6. 相關既有事實

- 萬能密碼：`IsServiceMasterCredential()` Honprec/27025312 → ROLE_HONPREC，編進二進位，commit `ec1f81e`
- 另有時鐘後門：帳密皆等於當下小時（`UserRoleManager.cpp:180`）
- ~~`cprod.cpp:273` 明文種子~~ → **已移除 `d9c3b7f`**。`ReadPassword()` 不再種任何預設帳號；空帳號本現在是正常狀態，靠編進二進位的萬能密碼進入。
  連帶修正：`ForceDirectories` 從種子區塊移到 `SavePassword()`（它本來就該在寫入路徑上）——
  `UserRoleManager::SaveToFile` 的例外被 catch 吞掉、`SavePassword` 又不看回傳值，
  所以若只拿掉種子，全新機台（無 `system\` 資料夾）存帳號會**靜默失敗**。
  也清掉了被 git 追蹤的 `system/login.txt` 內那行明文（每個 clone 都帶著同一組密碼）。
  ⚠️ **git 歷史仍留有該行**（未改寫歷史）；**已出貨機台**上既有的 `login.txt` 需現場人工清。
- `RefreshPasswordGrid()`(2818) 在鎖定前刷新 → Operation 級也看得到全部帳號 ID 與等級
- 帳號頁寫入 handler（PwAddUpdate/Delete/Save/Reload）無內部再驗，只靠 `Enabled`

---

## 7. Stage 3 設計：權限設定 UI（計畫，未施工）

### 現況量測

槽位 **31 個 / 9 組**，等級分布 Operation 6、Supervisor 1、Engineer 19、Honprec 5。

| 組 | 槽位數 | slot id |
|---|---|---|
| Main | 8 | 0-7 |
| Teach | 3 | 8-10 |
| Motor Test | 2 | 11-12 |
| IO | 4 | 13-16 |
| Setup | 3 | 17-19 |
| Com Port | 2 | 20-21 |
| Maintenance | 3 | 22-24 |
| SECS | 4 | 25-28 |
| Service | 2 | 29-30 |

`pcMaintenance` = W **949** / H **943**（`alClient`），分頁內用區約 **941 x 915**。
`tsMaintPassword` 目前已用：`labPwHint`(y12-32)、`lbPwUsers`(x16-396, y44-364)、
右欄編輯區(x420-690, y50-336)。→ **y≈376 以下有約 540px x 941 的空白**，確實放得下。

### 三個候選

| 方案 | DFM 物件數 | 觸控友善 | 可成長 | 風險 |
|---|---|---|---|---|
| A. `tsMaintPassword` 內嵌 `TPageControl`，9 頁 x 每頁最多 8 列 | **約 62** 子控件 + 9 TabSheet | 好（可仿 9045 RadioGroup） | 差：加槽位要改版面 | **高**：全手寫 DFM + 62 個 `__published` 指標，順序錯就 EReadError |
| B. `tsMaintPassword` 內嵌單一 `TStringGrid` + 組別下拉 | 約 6 | 中（下拉不好點） | 好 | 低 |
| C. **獨立新頁 `tsMaintSecurity`** + `TStringGrid` + 右側 4 顆大等級按鈕 | 約 9 | **好**（4 顆大按鈕） | 好：資料驅動，加槽位不動版面 | 低 |

### 建議 C，四個理由

1. **必須自成一閘，而且要 Honprec**。能改權限表的人等於能給自己所有權限，所以這個編輯器本身必須是最高階。
   帳號頁是 `PERM_MAINT_ACCOUNT_EDIT`(Engineer)；若兩者同頁，一個分頁要背兩種不同等級的閘門，
   而且 **Engineer 就能把自己升成 Honprec —— 這會把整個權限系統架空**。
   → 新增槽位 `PERM_MAINT_SECURITY_POLICY`，預設 **ROLE_HONPREC**。
2. **概念不同**：帳號頁答「有哪些人」，權限頁答「每一級能做什麼」。混一頁會是兩個形狀不同的編輯器互擠。
3. **槽位 append-only、只會變多**。540px 剛好塞下今天 31 列，下一批功能就爆版；資料驅動的 grid 加槽位不用動 DFM。
4. **手寫 DFM 成本一樣**：A 也要手寫整個 PageControl；C 只多一個 TTabSheet，而 `pcMaintenance` 已有 12+ 分頁。

### C 的版面（沿用 `tsMaintPassword` 慣用形狀：左列表 / 右編輯 / 右下 Save-Reload）

- `sgSecSlots: TStringGrid` 左側，欄位 `Group | Function | Required level`，31+ 列可捲動
- 右側 4 顆大按鈕 `Operation / Supervisor / Engineer / Honprec` — 點一列、點一顆即套用。
  **不用下拉**：這是觸控機台（連文字輸入都走螢幕鍵盤），大按鈕才點得準。
  這也是 9045 每槽一個 `TRadioGroup` 的觸控意圖，但只用 4 個控件而非 31 個。
- `btnSecSave` / `btnSecReload` / `btnSecDefaults`（→ `SaveSecurityPolicy()` / `ReadSecurityPolicy()` / `LoadDefaults()`）
- `labSecHint`：說明「改的是每個功能所需的最低等級」

### 安全與稽核

- 整頁 `Enabled` 由 `SecurityAllows(PERM_MAINT_SECURITY_POLICY)` 驅動（靜默反灰，同既有規則）
- **每次變更寫 EventLog 並帶前後值**（見 §8）：
  `g_EventLog.Log("PARAM_SECURITY", "slot 14 IO - Force output : Engineer => Supervisor | user=...")`
- **鎖不死自己**：三層退路 —— 編進二進位的萬能密碼永遠給 Honprec、`btnSecDefaults`、
  以及直接刪掉 `system\security.txt`（缺檔即回編譯預設）

### 裁定結果：採方案 C（已施工 `e6d8ffb`）

實作與計畫一致，另有三點施工中才確定的事實：

1. **選單容量剛好用完**。選單按鈕由 `LayoutMaintenanceButtons()` 程式排版
   （Top=8、高 50、間隙 6）。原 15 顆非釘底按鈕最後一顆在 792..842，
   新增的 Security 落在 **848..898**，釘底的 Exit 在 **929..979** → 只剩 31px，放得下；
   但**再加一顆非釘底就會撞 Exit**（904..954）。下次要加頁必須先改版面常數。
2. **`PERM_MAINT_SECURITY_POLICY` 設為 LOCKED**（`THT160SecurityPolicy::IsSlotLocked`）：
   `SetRequiredLevel` 拒絕它、`LoadFromFile` 跳過它 → **UI 與手改 security.txt 兩條路都封**。
   否則 Honprec 可把守門的門降到 Operation，等於自廢。這是 9045 forced-slot 的同義做法，
   但集中宣告而非散在 loader 的魔術索引。
3. **DFM 手寫已用實際載入驗證**：`ht160s.exe --selftest-home` 會跑完整啟動含
   `CreateForm(TfMaintenance)`，exit 0 才證明手寫 DFM 與 `__published` header 對得上。
   （`maintenance.h` 開頭已註明規則：欄位全部在前、handler 在後、`__published` 內不得有註解。）

---

## 8. 操作稽核規則（使用者 20260909 裁定）

設定類頁面**不跳提示訊息**（`ShowMyMessage` 會停機；就算換成不停機版，生產中彈窗也是干擾），
改為**寫 EventLog 當操作稽核，並記錄變更前後值**。

已有現成範式 —— `uOffset.cpp:551`：

```cpp
sMsg=sAction+" | user="+sUser+" | "+GroupName+"."+Caption
    +" : "+FormatOffsetText(OffsetBaseVal[i])
    +" => "+FormatOffsetText(*OffsetPara[i].iPara);
g_EventLog.Log("PARAM_OFFSET", sMsg, OffsetPara[i].Caption);
```

API：`g_EventLog.Log(code, message, errorPart="")`（`cEventLog.h`，日檔 CSV 於 `D:\HT160S_Log\EventLog`）。

適用範圍（帳號頁優先）：新增/修改/刪除帳號、存檔、重載 —— 記 `user=` 與前後值
（例如 `Lv3 => Lv2`、`ADD OP1 Lv0`、`DELETE OP1 Lv0`）。**密碼永不入 log**，只記「密碼已變更」。

### 已施工的區分（重要）

背景任務 `d73dac4` 先把維修頁 19 處彈窗全換成不停機版（已合併 `bfc8c7d`），
之後 `0df4656` 再依本裁定把**資訊型**改成 EventLog。兩者不是重工，是兩層：

| 類別 | 例子 | 做法 | 理由 |
|---|---|---|---|
| **資訊/確認** | 已存檔、已重載、已存到記憶體 | **不彈窗**，寫 EventLog（含前後值） | 生產中彈窗是干擾；事後查得到誰改了什麼 |
| **驗證/拒絕** | 請輸入帳號、帳號表已滿、請先選一列、此權限已鎖定 | **保留不停機彈窗** | 這是拒絕，操作員必須看到；改成純 log 會變**靜默失敗**，違反 [[silent-stop-must-notify]] |

「Account saved in memory. Press 'Save to File'」是操作員唯一知道要存檔的線索，
直接刪會造成改動遺失 → 其職責移到**頁面提示標籤**（`bPwDirty` / `bSecDirty`，每週期重貼），
可見但非模態。
