# HT160S FTP 上傳功能 — 執行計畫書

> 產出日期:2026-07-21
> 專案根目錄:`D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0\`
> 狀態:**Phase 1-7 code-complete (sim+real build exit 0) — 上機驗證 pending;唯一 blocking = SET_LOT_INFO SML 待 KYEC**（執行紀錄見文末）
> 客戶:京元竹南 (KYEC)　角色:Sorter-HT (FTP 帳號 `Sorter-log`)
> 規格源:客戶 Excel `D:\backup_version\HT160S\Document\Soter檔案格式.xlsx` + KYEC HPB-8 FTP 結構 PPT (Tim, 2026-07-21)

---

## 0. 目標與範圍

在 Lot End 時,把既有的 **Soter per-die CSV**(`cSoterOutput`)依「一個 KYEC 批號一個檔」拆分並本地歸檔,若使用者於維護頁勾選「上傳」則透過**背景 FTP 執行緒**以 KYEC 兩段式交檔協定送到內網 NAS,且每次上傳結果寫入 EventLog 供稽核。

**設計硬約束(貫穿全案)**
1. 所有 FTP 行為(連線/建目錄/上傳/測試/log)**全部封裝在單一 class `TFtpUploadThread`**;呼叫端只碰它的公開介面。
2. FTP 一律在**背景執行緒**,傳輸慢/斷線/重試/例外**永遠不得**阻塞生產流程或 UI。
3. FTP 設定**集中一處**(`system\General.ini [Ftp]`),由 class 自身 Load/Save;呼叫端上傳時**不帶連線參數**(斷開 HT9045 設定散落各 form 的病根)。
4. **UI 元件一律在 DFM 定義**(定義+擺放),不得在 cpp 用 `new` 動態建立;僅**動態顯示**(執行期更新 Caption/Text/Color/Visible、填 memo/grid、狀態列刷新、動態排版既有元件)允許寫 cpp。(使用者 2026-07-21 要求)

**執行節奏**:**逐階段執行**,每個 Phase 完成且 build 綠燈後**回報並等使用者確認**,才進下一階段。Phase 4(動已出貨 Soter)為紅線回歸點,必停下檢閱。

**Phase 1 log 決策(已定)**:背景 FTP log 用 `cCsvDailyLog`(lgDailyFolder+".log"+空 header)——已驗 `AppendLine` 全段在 per-instance `TCriticalSection` 下([cCsvDailyLog.cpp:114](../../HT160S_Program_BCB_V1.0.0.0/cCsvDailyLog.cpp))thread-safe(單寫者:主執行緒 `InitLog` 一次後才啟動 worker);統一 HT160 慣例並免費取得 retention。

**母版**:HT9045 `D:\HT9045\HT9011UC_Code_V3.33.899.0_20260323_Jimmy_20260422\FTPUpload\uFtpUploadThread.{h,cpp}` 的進化版。

---

## 1. 架構總覽

```
生產流程 (主執行緒)                        背景 FTP 執行緒
─────────────────                        ─────────────────
btnLotEndClick / CleanOut finish
  └ g_SoterOutput.OnLotEnd()
       ├ 依 KYECLotNo 分組, 每 Lot 一 CSV
       │   → 歸檔 SoterOutput\yyyymm\ (永久)
       │   → pickup 資料夾 (客戶抓, 下批清空)
       │   → 寫本地 flag 副本
       └ 若 [Ftp]Enable && UploadReport:
            FtpUploadThd->EnqueueLotPublish(          Enqueue (瞬時返回)
                kyecLot, csv本地檔, flag本地檔) ──────┐
                                                       ▼
                                              job queue (深拷貝, CS 保護)
                                                       ▼
                                              UploadLotPublish():
                                                FtpCreateDir /Sorter-log/<kyecLot>/
                                                FtpPutFile CSV
                                                FtpPutFile flag → /Sorter-log/LotEnd/
                                                (flag 最後 = commit; 整包重試冪等)
                                                       ▼
                                              PushResult(OK/GIVEUP)
MainProc 每 cycle:                                     │
  fMain->PollFtpUploadResults() ◀──── FetchResult ─────┘
       └ 成功/放棄 → g_EventLog.Log(...)

維護頁 (tsMaintFtp): 設定 Save/Reload + 手動測試 (連線/上傳) + 狀態列 + log memo
  手動測試也是 job → 結果同樣走背景, UI 絕不碰 WinINet
```

---

## 2. 設定綱要 `system\General.ini [Ftp]`

| 鍵 | 型別 | 預設 | 說明 |
|---|---|---|---|
| `Host` | string | `192.168.11.11` | NAS FTP 主機 |
| `Port` | int | `21` | 連接埠 |
| `User` | string | `Sorter-log` | 帳號 |
| `Password` | string | `Kyec20260720` | 明碼存 INI,UI 以 PasswordChar 遮罩 |
| `RemoteDir` | string | `/Sorter-log/` | 登入後可存取目錄根;CSV→`RemoteDir+<KYLotNo>/`,flag→`RemoteDir+LotEnd/` |
| `Enable` | bool | `0` | 生產上傳總閘門(不閘手動測試) |
| `UploadReport` | bool | `0` | Lot End 是否上傳生產報表 |
| `TimeoutMs` | int | `5000` | 連線逾時(對齊 9045) |
| `Retry` | int | `2` | 失敗重試次數 |

出廠 `Enable=0 / UploadReport=0`:上機以維護頁測通後再開。

---

## 3. 分階段實作

> 每階段獨立可建置。Build gate(每次 C++/DFM/專案改動):刪對應 `.obj` 後編譯,優先 `scripts/ops/build-ht160s.ps1 -Clean`;動到 `aSortArm`/`deviceinfo`/`SOFT_SIMULATE`-guarded 共用核心須**同時驗 sim 與 real**(切 `MachineType.h` 的 `#define SOFT_SIMULATE`,`-Full` exit 0 後還原);跑 `scripts/ops/check-ht160s-source-encoding.ps1`。新檔 ASCII-only 註解,不得 BOM。

### Phase 1 — FTP 執行緒核心 `uFtpUploadThread.{h,cpp}`
**改什麼**:新增與 `LotWebApiClient.cpp` 同層的執行緒 class。
- 保留 9045 骨架:job queue(`std::list`)+ 兩個 manual-reset event + `TCriticalSection`;所有 `AnsiString` 以 `c_str()` 深拷貝入列(防跨 thread COW race);WinINet `InternetOpen→InternetConnect(PASSIVE)→FtpCreateDirectory→FtpPutFile(BINARY)`;worker 內重試 + `::Sleep`(背景可安全睡);dedup(jobKind+key + in-flight key);**絕不 `Synchronize`**,絕不碰 VCL/MOT[]/Sen[]。
- 進化 1(設定內建):`LoadConfig()/SaveConfig()` 讀寫 `[Ftp]`,比照 `THT160LotWebApiClient::LoadConfig`([LotWebApiClient.cpp:81](HT160S_Program_BCB_V1.0.0.0/LotWebApiClient.cpp:81),`HSys.CurrentDir+"\\system\\General.ini"`+`TIniFile`)。設定以 CS 保護,job 取出時 snapshot,改設定不重啟 thread。
- 進化 2(job 種類):`JOBKIND_LOT_PUBLISH`(複合:建目錄+CSV+flag)、`JOBKIND_TEST_CONN`、`JOBKIND_TEST_UPLOAD`。公開 `EnqueueLotPublish(kyecLot, csvLocal, flagLocal)`、`EnqueueTestConn()`、`EnqueueTestUpload()`。
- 進化 3(結果回報):有界 ring buffer(最近 50 筆,`GetResultSnapshot()` 非破壞讀取供 UI);`JOBKIND_LOT_PUBLISH` 成功與 GIVEUP **都** `PushResult`(供主迴圈進 EventLog);測試 job 結果只進 ring buffer(不進 EventLog)。
- 進化 4(狀態):`GetStatusSnapshot()` 回 queue 深度/in-flight/OK-FAIL 累計/最後結果。
- 背景 log:`D:\HT160S_Log\FtpUpload\YYYYMMDD\FtpUpload_YYYYMMDD.log`,用**專屬 `cCsvDailyLog` 實例**(lgDailyFolder+".log"+空 header;已定,見上方 Phase 1 log 決策)。主執行緒建構時 `InitLog`+`SetRetentionDays(GeneralSetting.iLogRetentionEventDays)`,worker 只 `AppendLine`。
- 移除 9045 的 `#ifdef SOFT_SIMULTE` 導向 127.0.0.1 段(160 設定集中,開發機直接填)。

**加到專案**:`ht160s.bpr` + `ht160s.mak`(比照 LotWebApiClient 的登錄方式)。
**驗證**:`build-ht160s.ps1 -Clean` exit 0;此階段無呼叫端,純編譯通過即可。

### Phase 2 — 主程式生命週期 + 結果 pump
**改什麼**:
- 建立:採 lazy singleton `EnsureFtpUploadThreadCreated()`(比照 [EnsureLotWebApiClientCreated](HT160S_Program_BCB_V1.0.0.0/LotWebApiClient.cpp:32)),於程式初始化區([ht160s.cpp:213](HT160S_Program_BCB_V1.0.0.0/ht160s.cpp:213) `g_EventLog.Init()` 附近)呼叫並 `LoadConfig()`。`extern TFtpUploadThread *FtpUploadThd`。
- 銷毀:於 `TfMain::FormClose`([main.cpp:695](HT160S_Program_BCB_V1.0.0.0/main.cpp:695),`MyThread->WaitFor()` 之後)加 `FtpUploadThd->EndThread(); WaitFor(); delete; =NULL`(對齊 9045 main.cpp:10918-10923)。
- 結果 pump:新增 `TfMain::PollFtpUploadResults()`,在 `MainProc` 內 `PollLotDataWebApi()` 旁([csystem.cpp:184](HT160S_Program_BCB_V1.0.0.0/csystem.cpp:184))呼叫。迴圈 `FtpUploadThd->FetchResult(msg)` 直到空,每筆 `g_EventLog.Log("FTP_UPLOAD", msg, "")`。跑在 VCL 主執行緒,`g_EventLog` 呼叫安全。

**驗證**:`-Clean` exit 0;啟動/關閉程式無殘留執行緒或崩潰(sim 下觀察)。此階段仍無 enqueuer,pump 為 no-op。

### Phase 3 — 維護頁 `tsMaintFtp`(設定 + 手動測試 + 狀態)
**改什麼**(版型抄 `tsMaintLotApi`,[maintenance.dfm:3164](HT160S_Program_BCB_V1.0.0.0/maintenance.dfm:3164)):
- DFM 新增 `tsMaintFtp: TTabSheet` + `spbMaintFtp: TSpeedButton`(Caption `FTP`)。元件全寫死 DFM、命名正規化:
  - `pnlFtpSetup`:`edFtpHost / edFtpPort / edFtpUser / edFtpPwd`(PasswordChar='*')`/ edFtpRemoteDir / chkFtpEnable / chkFtpUploadReport / btnFtpSave / btnFtpReload`
  - `pnlFtpStatus`:`lblFtpState / lblFtpLastError`
  - `pnlFtpTest`:`btnFtpTestConn / btnFtpTestUpload / memFtpResult`
  - `memFtpLog`
- 事件 handler 宣告於 `__published`(DFM 事件硬規則):`spbMaintenanceMenuClick`(共用)、`btnFtpSaveClick / btnFtpReloadClick / btnFtpTestConnClick / btnFtpTestUploadClick`。Save→寫 UI 值進 class + `SaveConfig()`;測試→僅 `Enqueue*`(UI 不碰 WinINet)。
- 選單註冊:`RegisterMaintenancePages()` PageDefs 表([maintenance.cpp:1479](HT160S_Program_BCB_V1.0.0.0/maintenance.cpp:1479))在 `{tsMaintSECS,...}` **之後、Exit 之前**插 `{tsMaintFtp, spbMaintFtp, maShowPage, false}`——按鈕位置由表序 + `LayoutMaintenanceButtons()` 自動排,落在 SECS/GEM 下方(DFM Top 仍填合理值維持慣例)。
- 狀態刷新:新增 `RefreshFtpStatus()`(比照 [RefreshLotWebApiStatus](HT160S_Program_BCB_V1.0.0.0/maintenance.cpp:787)),掛進 `tmrTowerLightBlinkTimer`([maintenance.cpp:1461](HT160S_Program_BCB_V1.0.0.0/maintenance.cpp:1461))。讀 `GetStatusSnapshot()`/`GetResultSnapshot()` 顯示,**只顯示不傳輸**。log memo 用 ring 顯示(比照 `AddLotWebApiLog` 200 行上限)。

**驗證**:`-Clean` exit 0;sim 啟動→維護頁→FTP 分頁出現、按鈕在 SECS 下方、Save/Reload 讀寫 INI 正確;手動測試連線(對 127.0.0.1 或真機)結果顯示於狀態列而不卡 UI。

### Phase 4 — Soter per-Lot 拆檔 + `sKyecLotID` 骨架 + 檔名修正
**改什麼**(動到 `cSoterOutput` 已出貨模組 + `aSortArm` 共用核心,**須驗 sim + real**):
- `TSoterRow`([cSoterOutput.h:45](HT160S_Program_BCB_V1.0.0.0/cSoterOutput.h:45))加 `sCustLotID`、`sKyecLotID`、`sProductCode`/`sSubstage` 已有。兩處 `OpenRow` 呼叫端([aSortArm.cpp:1294](HT160S_Program_BCB_V1.0.0.0/aSortArm.cpp:1294)、[:1523](HT160S_Program_BCB_V1.0.0.0/aSortArm.cpp:1523))把該 die 所屬 `Lot->sLotID`(客戶批)與新的 `Lot->sKyecLotID`(京元批)傳入。
- `TLotRunInfo`([CosFunction.h:202](HT160S_Program_BCB_V1.0.0.0/CosFunction.h:202))加 `sKyecLotID`(+`Clear()` 清除)。
- `OnLotEnd()`([cSoterOutput.cpp:261](HT160S_Program_BCB_V1.0.0.0/cSoterOutput.cpp:261))重寫:進場抓**一次** `Now()` 當結批共用時戳;依 die 的 KYECLotNo 分組;每組一檔,檔名共用同一 `YYYYMMDD_HHNNSS`(5 個 Lot 好辨識同一次結批,月夾也用同一時戳算,不跨午夜拆月)。
- 欄位修正:`BuildDataLine` 第 6 欄=該列 `sCustLotID`、第 7 欄=`sKyecLotID`(不再 `m_sLotID` 寫兩次,[cSoterOutput.cpp:177](HT160S_Program_BCB_V1.0.0.0/cSoterOutput.cpp:177))。`BuildFileName` CustomerLotNo/KYECLotNo token 分開取;ProductCode/Substage token 改**直接從 Lot 物件**取(不靠首列 latch,順修 0-die header-only 檔名雙底線瑕疵)。檔名尾端維持 `{SorterID}_{Qty}.csv`(Excel 規格已定案)。
- 缺京元批號 fallback:col7 與檔名 KYECLotNo token 填 `NA`(依客戶:實務不會發生,因無 Lot 資訊不能生產)。
- 0 顆 Lot 照產 header-only 檔(照產照傳)。
- Substage 維持 2D JSON 來源(客戶已確認)。

**驗證**:sim 單 Lot→行為與今日相同(一檔);多 Lot 混跑→每 KYEC 批一檔、共用時戳、col6≠col7;0-die→header-only 檔名無雙底線。real build `-Full` exit 0。

### Phase 5 — Lot End 兩段式交檔接線 + flag 檔
**改什麼**:
- flag 檔產生:`OnLotEnd` 每寫成一個 CSV,產一份 flag `<KYLotNo>_<共用時戳>.txt`,內容=該 CSV 檔名(一行一檔),寫到 SoterOutput 歸檔月夾留副本。
- 上傳掛線:若 `[Ftp]Enable && UploadReport && FtpUploadThd!=NULL`,對每個 Lot `EnqueueLotPublish(kyecLot, csv歸檔路徑, flag歸檔路徑)`。**上傳來源用歸檔區**(pickup 下批會被清空,見 [cSoterOutput.cpp:103](HT160S_Program_BCB_V1.0.0.0/cSoterOutput.cpp:103))。
- 缺京元批號:跳過該 Lot 上傳 + `g_EventLog.Log("FTP_SKIP","no KYEC lot, upload skipped")`(遠端資料夾名即 KYLotNo,無它只會產生 `/NA/` 垃圾)。
- 掛點:`btnLotEndClick`([main.cpp:2537](HT160S_Program_BCB_V1.0.0.0/main.cpp:2537)) 與 CleanOut finish([csystem.cpp:1425](HT160S_Program_BCB_V1.0.0.0/csystem.cpp:1425)) 的 `OnLotEnd()` 內部完成,兩終止點皆受惠(idempotent 先到先寫)。

**驗證**:sim 對 SECS Simulator / 本機 FTP(如有)跑一批;確認 `/Sorter-log/<kyecLot>/` 建立、CSV 上傳、flag 最後落 `/Sorter-log/LotEnd/`、內容為檔名清單;EventLog 每 Lot 一筆 OK/GIVEUP;Enable=0 時完全不上傳但本地照產。real `-Full` exit 0。

### Phase 6 — SET_LOT_INFO 帶京元批(**blocked,先做骨架**)
**改什麼**(等 KYEC 確認 SML 格式前只做向下相容骨架):
- 解析器([uHGemHT160.cpp:741](HT160S_Program_BCB_V1.0.0.0/SecsGem/uHGemHT160.cpp:741)):每 Lot 項目支援 `A "custLot"`(舊,京元批=NA)**與** `L[2]{A custLot, A kyecLot}`(新)兩種,依 DataItem 型別分辨(向下相容,舊 host 不壞)。`AddLot` 帶入 `sKyecLotID`。
- WebAPI 逐筆餵:客戶說明「160 把 lot 訊息逐筆餵 WebAPI 下載 2D JSON」——確認現行 `StartNextLotApiPull`([main.cpp:2290](HT160S_Program_BCB_V1.0.0.0/main.cpp:2290))逐 Lot 拉取即符合,京元批號不影響拉取 key(仍用 LOTID)。
- 持久化:`sKyecLotID` 跟進 `SaveWorkOrder`(重開機不掉)。
- **TODO 插點標記**:格式未定前,`L[2]` 分支以我方提案實作並註記 `//AI TODO: confirm KYEC SML shape`;京元定案若不同,只改此單點。

**驗證**:SECS Simulator 推 `L[2]` lot→`sKyecLotID` 正確落 registry→Soter col7/檔名正確;推舊 `A` lot→京元批=NA 不崩。

### Phase 7 — 文件與設定收尾
- `system\General.ini` 補 `[Ftp]` 預設區(或由 `SaveConfig` 首次寫入)。
- 翻新 `docs/plan/soter-output-csv-gap-analysis-20260714.md`:col6/7 同值舊決策作廢、記錄 per-Lot 拆檔與 sKyecLotID 決策。
- `cSoterOutput.h` class 註解同步更新(Cust lot≠Kyec lot)。
- 更新既有 memory [[soter-output-csv-spec]]、[[ftp-kyec-upload-project]]。

---

## 4. 風險與待決

| 項 | 狀態 | 緩解 |
|---|---|---|
| **SET_LOT_INFO 帶京元批的 SML 格式** | ⏸ KYEC 內部確認中 | 已隔離在解析器單點;Phase 6 骨架向下相容,格式回來只改一分支。**不擋 Phase 1-5 開工**。 |
| 動到已出貨 `cSoterOutput` + 共用核心 `aSortArm` | 中 | Phase 4 強制 sim+real 雙驗;單 Lot 行為須與今日一致(回歸點) |
| FTP 伺服器 chroot 與否(登入點) | 低 | RemoteDir 由 UI 可改;真機用手動測試按鈕驗登入點,錯了改欄位不改碼 |
| SorterID(檔名+col10)實機為空 | 低 | 部署事項:上機前於 `General.ini [MachineIdentity]` 填京元給的正式 Sorter 編號(如 `XX-01`) |
| CSV 檔名尾端格式 | ✅ 已定案 `{SorterID}_{Qty}.csv`(Tim 2026-07-21 確認舊 `R0_ALL` 作廢) | — |

## 5. 建議執行順序
Phase 1 → 2 → 3(此時可用手動測試驗證整條 FTP 傳輸,先於生產接線)→ 4 → 5 → 6(骨架)→ 7。
Phase 6 京元格式回來後補完該單點分支即可,無返工。

## 6. 交付定義
- sim + real build 皆 exit 0、encoding check 通過。
- 維護頁可設定並手動測通 FTP 連線與上傳。
- 多 Lot 混跑產出每 KYEC 批一 CSV(共用結批時戳)+ flag,Enable 勾選時兩段式送達 NAS,EventLog 有逐 Lot 上傳紀錄。
- 生產流程/UI 在 FTP 斷線或逾時下**零阻塞**(拔網路線測試)。
- 最終附**檔案+行號+內容**修改點清單供複查。

---

## 7. 執行紀錄 (2026-07-21, code-complete)

逐階段執行、每階段 build 綠燈後由使用者確認再進下一階;Phase 4-6 均跑對抗式複審(獨立視角 find→adversarial verify)。

| Phase | 內容 | Build | 複審 |
|---|---|---|---|
| 1 | `uFtpUploadThread.h/.cpp` 核心(設定內建/多 job/ring buffer/EventLog 契約/cCsvDailyLog log);登錄 .bpr/.mak(+wininet.lib) | sim exit 0 | — |
| 2 | 生命週期:`ht160s.cpp` EnsureFtpUploadThreadCreated;`main.cpp` FormClose teardown;`PollFtpUploadResults` 掛 `csystem.cpp` MainProc | sim exit 0 | — |
| 3 | 維護頁 `tsMaintFtp`(DFM 全元件 + `__published` handler + PageDefs + RefreshFtpStatus);MAX_MAINTENANCE_MENU_COUNT 16→18 | sim exit 0, __published lint 綠 | — |
| 4 | Soter per-Lot 拆檔 + 共用結批時戳 + `sKyecLotID`;col6≠col7;修 0-die 檔名 | **sim+real exit 0** | 2 low 已修(bucket key CaseSensitive;檔名碰撞唯一性守衛) |
| 5 | Lot End 兩段式交檔:依 kyec 分組、per-kyec flag(列全部 CSV)、全 CSV 上傳完才放 flag;`EnqueueLotPublish` 擴多 CSV | **sim+real exit 0** | 4 low 已修(worker bEndThread 輪詢;handle try/__finally + Execute catch-all 金律;pSeen OOM leak;raw-vs-SafeToken kyec 一致);+≥1-CSV 防禦守衛 |
| 6 | SET_LOT_INFO `L[2]{custLot,kyecLot}` 向下相容 ASCII(**SML 待 KYEC**);`sKyecLotID` 持久化(SaveToJsonFile/LoadFromJsonString 非空守衛) | sim exit 0 | 1 medium 已修(改 buffer-then-commit 原子化,拒收不留半套) |
| 7 | `system\General.ini [Ftp]` 預設;翻新本文件 + gap-analysis;memory | sim exit 0 | — |

**設定預設(General.ini [Ftp])**:Host=192.168.11.11 Port=21 User=Sorter-log Password=Kyec20260720 RemoteDir=/Sorter-log/ Enable=0 UploadReport=0 TimeoutMs=5000 Retry=2。

### 上機驗證清單 (pending)
1. 維護頁 FTP 分頁:填連線→Test Connection→Test Upload,確認登入點與 RemoteDir(錯則改 UI 欄位,不需改碼)。
2. `[MachineIdentity] SerialNo` 填京元給的正式 Sorter 編號(檔名 col10;現為空→"NA")。
3. 開 Enable + UploadReport,跑一批多 Lot,確認 `/Sorter-log/<KYLotNo>/` CSV + `/LotEnd/` flag,EventLog `FTP_UPLOAD` 逐 Lot 紀錄。
4. 拔網路線測試:FTP 逾時/斷線時生產與 UI **零阻塞**。
5. SECS round-trip:KYEC host 送 SET_LOT_INFO(SML 確認後),確認 col7/檔名帶正確京元批號。

### 已知限制 / 待決
- **SET_LOT_INFO SML 格式**:`L[2]{custLot,kyecLot}` 為我方提案,KYEC 內部確認中。若採他式(如 L[3]/SVID/WebAPI),只改 `uHGemHT160.cpp` S2F42 的 L[2] 分支單點。
- **WhiteList 模式 Lot Start 會洗掉 kyec**(pre-existing,跨模式,不在 KYEC NORMAL 流程內):已 spawn 獨立 task 待決 WhiteList reload 語意,本案不擅改。
- CSV 檔名尾端 `{SorterID}_{Qty}`(Excel 規格,舊 `R0_ALL` 作廢已確認)。
