# HT160S_BCB Log 統一作戰計畫 — 抽共用基底 `cCsvDailyLog`（Phase 1 + Phase 3）

作者：JimmyChiu
日期：2026/06/15
適用版本：`HT160S_Program_BCB_V1.0.0.0`（HT160S_BCB 主開發樹）
狀態：**待 Jimmy 確認**。確認後可直接複製本檔最末「新工作階段啟動提示」開新 session 施作。

---

## 0. 目前進度（已完成、已 build 驗證）

| 階段 | 內容 | 狀態 |
|------|------|------|
| Phase 2 方案 A | `note\process_*.log` 寫檔移除；`note.cpp` 三個紀錄點（`ProcessErrMessage` / `RecordProcess` / `RecordAlarmMessagePassTime`）改走 `g_EventLog.Log()`；`TfNote` UI 保留 | ✅ Clean build 2026/06/15 13:33（encoding 153 檔通過） |
| Phase 1 | 新增共用基底 `cCsvDailyLog.h/.cpp`；`cEventLog`、`cCommLog` 改為繼承薄包裝（對外符號/簽章不變）；EventLog 日期 `\`→`/`、Message/ErrorPart 套 `CsvQuote`（依 Jimmy 確認）；接 bpr/mak | ✅ Clean build 2026/06/15 13:59（encoding 155 檔通過） |
| Phase 3（P3-A） | `cCsvDailyLog` 加 `lgDailyFolder` granularity + 副檔名 + 可空表頭（`InitLog` 公開，預設值保持月夾/.csv 不影響 Phase 1）；`SaveWebApiLog` 改走 `g_WebApiLog.AppendLine`（client 建構子 Init）。**路徑/檔名/行格式全保留**（`WebAPI\YYYYMMDD\WebAPI_YYYYMMDD.log`、`hh:nn:ss:zzz :msg`、無表頭）→ state-record 擷取與分析不受影響，新增執行緒鎖 | ✅ Clean build 2026/06/15 14:07（encoding 155 檔通過） |

> 結果：EventLog（告警/操作）、PadLog/BindisplayLog（通訊）、WebAPI（Lot API）全部統一在 `cCsvDailyLog` 基底；各自 infra 不再複製，且行為/路徑/格式對外不變。Production_Log / SECS_GEM / automation 維持獨立（見 §1、§3.4）。**本計畫主體完成。**

---

## 1. 背景與現況（已查證）

Log 根目錄 = `HSys.LogRootDir` = `D:\HT160S_Log`。目前**程式碼實際在寫**的 log：

| Log | 寫入者 | 路徑慣例 | 表頭 / 格式 | 鎖 |
|------|--------|----------|-------------|----|
| **cEventLog** | `g_EventLog`（[cEventLog.cpp](../../HT160S_Program_BCB_V1.0.0.0/cEventLog.cpp)） | `EventLog\YYYY_MM\HT160S_YYYY_MM_DD.csv` | `Date,Time,Recovery,PauseTime,Duplicate,AlarmCode,Message,ErrorPart` | ✅ CS |
| **cCommLog** | `g_PadCommLog`="PadLog"、`g_BinDispCommLog`="BindisplayLog"（[cCommLog.cpp](../../HT160S_Program_BCB_V1.0.0.0/cCommLog.cpp)） | `<Name>\YYYY_MM\<Name>_YYYY_MM_DD.csv` | `Date,Time,Action,Message`（Message 經 `CsvQuote`） | ✅ CS |
| **WebAPI** | `THT160LotWebApiClient::SaveWebApiLog`（[LotWebApiClient.cpp:532](../../HT160S_Program_BCB_V1.0.0.0/LotWebApiClient.cpp#L532)） | `WebAPI\YYYYMMDD\WebAPI_YYYYMMDD.log` | 純文字 `hh:nn:ss:zzz :<msg>` | ❌ |
| **Production_Log** | `g_DeviceInfo`（[deviceinfo.cpp](../../HT160S_Program_BCB_V1.0.0.0/deviceinfo.cpp)） | `Production_Log\YYYYMM\<Lot>_<time>.csv` | 17 欄逐顆 IC 領域 schema | ✅ CS |
| **automation** | `THT160AutomationServer::WriteLog`（[AutomationServer.cpp:70](../../HT160S_Program_BCB_V1.0.0.0/AutomationServer.cpp#L70)） | `automation\automation_startup.log`（單檔） | 純文字 | ❌ |
| **SECS_GEM** | uHGemEquipment | `SECS_GEM\YYYY_MM_DD\...txt` | 廠商 HSMS 庫格式 | （庫內） |

**HT172 參考做法（唯讀）**：`TFormSysTools::RecordAlarmMessage` + `SaveLogRpt`（[systools.cpp:3225](D:/HT172/HT172_Program_V1.0.25.0_20260420/systools.cpp)）寫 `EventLog\YYYY_MM\<ID>_YYYY_MM_DD.csv`，表頭與 HT160S `cEventLog` 完全相同。亦即 `cEventLog` 已是 HT172 此路徑的忠實移植。

### 關鍵觀察（驅動本計畫）
- `cEventLog` 與 `cCommLog` 的基礎設施**幾乎一字不差**：`TCriticalSection`、月夾+日檔路徑、`EnsureHeader`、append、（cCommLog 另有 `CsvQuote`）。差異只在三點：①子資料夾名 ②檔名前綴 ③表頭與每行欄位組裝。
- 兩者**有兩個不一致**需在統一時收斂：
  - 日期欄分隔字元：`cEventLog` 寫 `yyyy\mm\dd`（反斜線，怪），`cCommLog` 寫 `yyyy/mm/dd`。→ **統一為 `yyyy/mm/dd`**。
  - `cEventLog` 不對 Message 做 CSV 跳脫；含逗號的訊息會破欄。→ 統一後一律走 `CsvQuote`。

### 不納入統一（維持獨立）
- **Production_Log**：17 欄領域 schema、逐 nozzle 記錄、Lot 生命週期 API，與通用四/八欄無關。
- **SECS_GEM**：外部工具吃固定格式。
- **automation**：單檔啟動診斷，價值低、無滾動需求；可選擇性納入（見 Phase 3 備註），預設不動。

---

## 2. Phase 1 — 抽共用基底 `cCsvDailyLog`

### 2.1 設計原則
- **行為保留、零呼叫端改動**：對外符號 `g_EventLog`、`g_PadCommLog`、`g_BinDispCommLog` 與其方法簽章 **完全不變**。`g_EventLog.Log(...)`、`g_PadCommLog.Log(...)` 等呼叫端一行都不用改。
- 基底只承接「共用基礎設施」；子類只負責「子資料夾 / 檔名前綴 / 表頭 / 每行欄位組裝」。
- 不引入 C++11、不引入 FSM；維持 `AnsiString` 流、`TCriticalSection`、`fopen/fprintf` append。

### 2.2 基底類別（新檔 `cCsvDailyLog.h/.cpp`）
職責：路徑滾動（月夾+日檔，當日快取）、`TCriticalSection`、首寫補表頭、append 一行、`CsvQuote`。

介面（草案，最終以實作為準）：
```cpp
class cCsvDailyLog
{
protected:
    TCriticalSection* m_pCS;
    AnsiString m_sBaseDir;      // root + "\\" + subfolder
    AnsiString m_sFilePrefix;   // daily file name prefix
    AnsiString m_sHeader;       // CSV header row
    AnsiString m_sLastFilePath;
    AnsiString m_sLastDate;

    AnsiString GetLogFilePath();              // <base>\YYYY_MM\<prefix>_YYYY_MM_DD.csv
    void EnsureHeader(const AnsiString& sPath);

public:
    cCsvDailyLog();
    virtual ~cCsvDailyLog();

    // subfolder = 子資料夾(=base dir 末段); filePrefix = 日檔前綴; header = 表頭行
    void Init(const AnsiString& sSubFolder,
              const AnsiString& sFilePrefix,
              const AnsiString& sHeader);

    void AppendLine(const AnsiString& sLine);     // CS 內 append（含補表頭）
    static AnsiString CsvQuote(const AnsiString& sText);
    static AnsiString NowDate();                  // "yyyy/mm/dd"  (統一分隔)
    static AnsiString NowTime();                  // "hh:nn:ss.zzz"
};
```

### 2.3 `cEventLog` 改寫
- `cEventLog` 改為**薄包裝**：建構時 `Init("EventLog", "HT160S", "Date,Time,Recovery,PauseTime,Duplicate,AlarmCode,Message,ErrorPart")`。
- 保留 `Log(AlarmCode, Message, ErrorPart)` 與 `LogRecovery(...)` 兩個公開方法不變；內部改成「組欄位字串 → 呼叫 base `AppendLine()`」。
- 借此把日期分隔由 `\` 改成 `/`，並對 Message/ErrorPart 套 `CsvQuote`（修掉破欄風險）。
- 兩種收斂方式擇一：
  - **(a) 繼承** `class cEventLog : public cCsvDailyLog`（推薦，最少樣板）。
  - (b) 組合：`cEventLog` 內含一個 `cCsvDailyLog` 成員。
- `g_EventLog` 全域實例與 `ht160s.cpp:133` 的 `Init()` 呼叫保留（`cEventLog::Init()` 內改呼叫 base `Init("EventLog",...)`）。

### 2.4 `cCommLog` 改寫
- 同樣改為薄包裝：`Init(sName)` 內呼叫 base `Init(sName, sName, "Date,Time,Action,Message")`。
- 保留 `Log(Action, Message)` 簽章；內部 `AppendLine(Date + "," + Time + "," + Action + "," + CsvQuote(Message))`。
- `g_PadCommLog` / `g_BinDispCommLog` 與 `ht160s.cpp:136-137` 的 `Init("PadLog")` / `Init("BindisplayLog")` 全保留。

### 2.5 專案接線
- 新增 `cCsvDailyLog.cpp/.h` 到 `ht160s.bpr` 與 `ht160s.mak`（與既有 cEventLog/cCommLog 同段）。
- 因為動到 .bpr/新檔 → **全 clean build**（`scripts\ops\build-ht160s.ps1 -Clean`）。

### 2.6 驗收（Phase 1）
- Clean build 0 error、encoding 檢查 0 `EF BF BD`、無 UTF-8 BOM。
- 跑機（或模擬）後檢查：
  - `EventLog\YYYY_MM\HT160S_*.csv` 照常產生、表頭一致、日期欄變 `/`、含逗號訊息正確被引號包住。
  - `PadLog\` / `BindisplayLog\` 照常產生且格式不變。
- **不改變任何外部可見路徑/檔名/欄位順序**（僅 EventLog 日期分隔字元與跳脫行為微調 — 需 Jimmy 確認可接受；若要 100% bit 相容則保留 `\` 與不跳脫，但不建議）。

> ⚠️ Phase 1 唯一的行為差異點：EventLog 日期分隔 `\`→`/` 與 Message 跳脫。**需 Jimmy 決定**是否接受（建議接受，較正確）。其餘皆行為保留。

---

## 3. Phase 3 — WebAPI log 併入基底

### 3.1 現況與相依
- 現況：`SaveWebApiLog` 寫 `WebAPI\YYYYMMDD\WebAPI_YYYYMMDD.log`（**日資料夾**、`.log`、無鎖、純文字 `hh:nn:ss:zzz :msg`）。

### 3.1.1 State Record 相依（已查證 2026/06/15）
更改 WebAPI log 的「路徑」或「方法/格式」會同時牽動 State Record 的**儲存**與**分析**，兩端都已盤點：

| 端 | 元件 | 如何使用 WebAPI log | 對 Phase 3 的約束 |
|----|------|---------------------|-------------------|
| **儲存** | `cStateRecordHT160::CaptureWebApiLog`（[cStateRecordHT160.cpp:674](../../HT160S_Program_BCB_V1.0.0.0/cStateRecordHT160.cpp#L674)） | `WebSrc = LogRootDir+"\\WebAPI\\"+YYYYMMDD`；`DirectoryExists` 判定→`CopyFolderFiles` **整夾複製**進 snapshot `WebApiLog\YYYYMMDD\` | **路徑/資料夾命名不可變**（`WebAPI\YYYYMMDD\`）。複製是**內容無關**，不解析檔案內容。 |
| 儲存(同類) | `CaptureSecsLog`（[:645](../../HT160S_Program_BCB_V1.0.0.0/cStateRecordHT160.cpp#L645)） | 同上，複製 `SECS_GEM\YYYY_MM_DD\` | （SECS 本案不動，僅記錄相依形式相同。） |
| **分析** | `scripts/ops/analyze-state-record.ps1` + skill `ht160s-state-record-analysis` | 只讀 `MachineState.ini` / `TaskHistory.csv` / `CurrentTasks.txt` / `Snapshot.ini`（Task 數字、時間戳）；**完全不解析** WebAPI / SECS / EventLog 檔案內容 | WebAPI log 格式改變**不影響**分析工具。 |

**結論**：採 **P3-A** 並**同時保留路徑與每行文字格式**（`WebAPI\YYYYMMDD\WebAPI_YYYYMMDD.log`、`hh:nn:ss:zzz :msg`、無 CSV 表頭），則 Capture 整夾複製照常、分析工具不受影響、support 工程師在 snapshot 裡看到的 log 也一字不變。Phase 3 只新增「執行緒鎖 + 統一 append 進入點」之利。
- **同步維護備忘（若日後路徑/格式真要改）**：① 須同步改 `CaptureWebApiLog`（來源資料夾命名）；② EventLog 若未來納入 snapshot 或被分析工具解析，其 Phase 1 格式微調（日期 `/`、CsvQuote）也須一併告知分析工具——目前 EventLog **未**被 snapshot 擷取、亦未被分析工具讀取，故無影響。

### 3.2 兩個方案（需 Jimmy 擇一）
- **方案 P3-A（保留現有路徑+格式，僅換實作）**：讓 `cCsvDailyLog` 支援「日資料夾」granularity（`InitLog` 增 `eLogGranularity {lgMonthlyFolder, lgDailyFolder}` 與副檔名參數，皆預設沿用月夾/.csv；表頭傳空字串則不寫表頭）。WebAPI 用 `lgDailyFolder` + `.log` + 空表頭 + 沿用 `hh:nn:ss:zzz :msg` 行格式，**路徑、檔名、每行格式全不變**（見 §3.1.1），只取得「CS 鎖 + 統一 append 進入點」之利。風險最低。
- **方案 P3-B（完全對齊 EventLog/CommLog）**：改為月夾 + `.csv` + 四欄（`Date,Time,Action,Message`）。需同步修改 `cStateRecordHT160.cpp` 快照讀取邏輯。較一致但牽動 StateRecord，風險較高。

> 預設建議 **P3-A**（先得到鎖與一致性、不動快照）。若日後要全面 csv 化再走 P3-B。

### 3.3 驗收（Phase 3）
- Clean build；WebAPI log 照常產生；**state-record 快照仍能抓到 WebAPI 當日資料**（重點回歸測試）。

### 3.4 備註：automation log
- 若要順手納入：`automation_startup.log` 為單檔啟動診斷，可改用 base 的單檔模式（需 base 另支援「不滾動單檔」）。價值低、預設**不做**，留待有需要再議。

---

## 4. 風險與注意事項
- **安全相關子系統**：EventLog 承載告警紀錄，屬機台稽核軌跡。Phase 1 改寫務必保證「每筆告警仍寫入、欄位順序不變」。
- **編碼**：新檔註解 ASCII English；新檔存 ANSI/Big5（與既有 .cpp 一致），勿 UTF-8/BOM。
- **build gate**：每次 C++/.bpr 編輯後刪對應 `.obj` 再編；動 .bpr/新檔 → `-Clean` 全 build；跑 `check-ht160s-source-encoding.ps1`。
- **不可改 HT172**（唯讀參考）。

---

## 5. 舊資料清理（非程式、選做）
以下資料夾現已**無任何程式寫入**，屬可清理的歷史/殘留資料（清理為人工決定，AI 不主動刪）：
- `D:\HT160S_Log\note\process_*.log` — Phase 2 後已停寫。
- `D:\HT160S_Log\AlarmLog\`、`D:\HT160S_Log\LotHistory\` — 本就無 HT160S 寫入者（HT172/模擬殘留；表頭 `ResponseMask`/`Module=Scheduler`/`Yield,UPH` 在 HT160S 原始碼中不存在）。

---

## 6. 新工作階段啟動提示（複製到新 session）

```
依 docs/plan/log-unification-cCsvDailyLog-plan.md 施作 Phase 1：
1. 新增 cCsvDailyLog.h/.cpp（共用基底：月夾+日檔路徑滾動、TCriticalSection、
   EnsureHeader、AppendLine、CsvQuote、NowDate("yyyy/mm/dd")/NowTime）。
2. cEventLog 改為繼承/組合 cCsvDailyLog；Init("EventLog","HT160S",<8欄表頭>)，
   Log/LogRecovery 內部組欄位後呼叫 AppendLine；日期分隔改 "/"，Message/ErrorPart 套 CsvQuote。
3. cCommLog 改為繼承/組合 cCsvDailyLog；Init(sName) -> base Init(sName, sName, <4欄表頭>)。
4. 對外符號 g_EventLog / g_PadCommLog / g_BinDispCommLog 與簽章不變；
   ht160s.cpp 的 Init 呼叫保留。
5. cCsvDailyLog.cpp/.h 加入 ht160s.bpr + ht160s.mak。
6. Clean build（scripts\ops\build-ht160s.ps1 -Clean）+ encoding 檢查。
先確認 Jimmy 是否接受 EventLog 日期分隔 \ -> / 與 Message 跳脫的行為微調。
Phase 3（WebAPI）預設走方案 P3-A，動工前先查 cStateRecordHT160.cpp:681 的 WebAPI 快照相依。
```
