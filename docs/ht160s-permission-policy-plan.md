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
| 2b | 其餘檔案：`uteach.cpp` / `uOffset.cpp` / `iosetview.cpp` / `uMotorTest.cpp` / `setup.cpp` / `ComPort.cpp` / `uHGemLogForm.cpp` | 待辦 |
| 3 | 維修頁權限設定頁（依 group 列槽位 + 等級選擇 + Save/Reload），仿 `tsMaintPassword` | 待辦 |
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
- `cprod.cpp:273` 仍會在帳號本為空時把 `Honprec,27025312,3` **明文**寫進 `system\login.txt`（外洩點，移除與否待裁）
- `RefreshPasswordGrid()`(2818) 在鎖定前刷新 → Operation 級也看得到全部帳號 ID 與等級
- 帳號頁寫入 handler（PwAddUpdate/Delete/Save/Reload）無內部再驗，只靠 `Enabled`
