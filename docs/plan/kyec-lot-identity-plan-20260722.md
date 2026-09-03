# KYEC 雙批號身分整合計畫（v3 — KYEC Lot 為機台身分 + 1:N + latest-wins）

> 日期：2026-07-22（v3）　狀態：**計畫待使用者確認，尚未動工**
> 依據：12-agent 稽核 wf_c3c05e02-fd3 + 使用者 2026-07-22 兩輪澄清（§0）
> 稽核原始摘要：session scratchpad `lot-audit-digest.md`

## 0. 使用者拍板的模型（v3 定案）

1. **機台一切 Lot 判別、SECS 通訊、分選綁定＝以 KYEC Lot（京元批）為身分；客戶批只進 Soter 報表 col6。**（第 2、5 點）
2. **一個 KYEC Lot 會對應「多個」客戶批（1:N，已確認）。** → 客戶批/ProductCode/Substage 必須 **per-IC** 儲存（一個 KYEC Lot 內每顆 IC 各自記自己的客戶批/產品/製程段）。
3. **資料以 SECS + WebAPI 為唯一依據，永遠取最新** → re-pull 對既有碼要 **upsert（覆蓋成最新值）＋大聲記 log**（第 1、6 點）。
4. By Lot+Bin 鍵＝(KYEC Lot, HBin)；需 `iSortBinSource=HBIN`（第 5 點）。

京元批＝SECS `SET_LOT_INFO`/`LOTSTART` 宣告字串（NQ…）＝WebAPI `OSATLot` 查詢鍵。客戶批＝回應 `QRCodeIDHis[].LOTID`（N8R334…）。

## 1. 資料結構改動（已驗證現況）

- `TLotRunInfo`（CosFunction.h）：sLotID＝KYEC Lot（機台身分，不變）。`sProductCode`/`sSubstage` 降為「代表值」（僅 round-trip / State Record 用，Soter 改讀 per-IC）；`sKyecLotID` 變 vestigial（col7 改讀 sLotID）。兩者保留不移除以縮小 blast radius。
- `TLotIcInfo`（CosFunction.h:228）：**新增三個 per-IC 欄位** `sCustLotID`（客戶批 col6）、`sProductCode`（col4）、`sSubstage`（col5）。既有 sLotID＝owning lot＝KYEC Lot。
- **已驗證 `Lot->sProductCode/sSubstage` 消費端**：只有 Soter OpenRow 兩站（aSortArm.cpp:1288-1289 reject、1517-1518 pick）、SaveToJsonFile/LoadFromJsonString round-trip、cStateRecordHT160 傾印。無路由/SECS/UI 依賴 → 改 per-IC 安全。
- **已驗證 `AddItemEx`**（CosFunction.cpp:1070-1110）：重複碼走 reject（:1085-1093）；正常加 `iPlanQty++`（:1097）+ 建 `TLotIcInfo`。upsert 改的就是 reject 段。
- **已驗證 reverse index**：`PackRef=LotIndex*1000000+Bin`（:890），upsert 改 routing bin 時更新 `m_Code2DIndex->Objects[Exist]`。

## 2. 修改計畫

### P0 — 核心（單一 commit）

**P0-1 `TLotIcInfo` 加 per-IC 三欄 + round-trip**
- `TLotIcInfo` += `sCustLotID` / `sProductCode` / `sSubstage`；`FindIcInfo` 因回傳 `*Rec` 自動帶出。
- `GetLotIcList` 的 tab-line（現 `Code2D\tBin\tHBin\tSBin\tRetestCode\tDiePass`）尾端append `\tCustLotID\tProductCode\tSubstage`（F6/F7/F8）。
- `SaveToJsonFile` 的 ICIInfo 每筆加 `CustLotID`/`ProductCode`/`Substage`（讀新 F6-F8）；`LoadFromJsonString` ICIInfo 讀回填入 per-IC 欄。

**P0-2 `AddItemEx` 加參數 + upsert 模式**
- 新簽名（BCB6 預設參數）：`AddItemEx(LotID, Code2D, Bin, HBin, SBin, RetestCode, DiePass, CustLotID, ProductCode, Substage, DupExistingLot, bUpsert=false)`。
- `bUpsert=false`（AddItem/手動 2D 編輯維持原行為，reject 重複 → 上游邏輯不變）。
- `bUpsert=true`（僅 WebAPI 解析用）：碼已存在時**不 reject，改就地更新** `TLotIcInfo` 全欄 + 更新 reverse-index 的 routing bin（`PackRef(OldLotIndex, newBin)`，owning lot 不變）+ **不加 iPlanQty** + registry 計數器 `m_RefreshCount++`；碼不存在則照舊新增。若 OldLotIndex≠新 LotIndex（KYEC 流程不會發生的跨 lot 碰撞）→ 記異常 log、資料仍更新、owning lot 保持不變。
- `AddItem`（:1063）改傳 `""/""/""` + `bUpsert=false`。

**P0-3 `LoadFromJsonString(+AnsiString StampKyecLotId="")`**（CosFunction.h/.cpp:1370）
- **2DIDHistory / QRCodeIDHis 分支**：`lotKey = StampKyecLotId.Trim()!="" ? StampKyecLotId : groupLOTID`。逐 QRCodeIDHis group 讀 `groupLOTID`（客戶批）、`groupSubstage`、`groupProduct`；該 group 每顆 IC 呼叫 `AddItemEx(lotKey, ..., CustLotID=groupLOTID, ProductCode=groupProduct, Substage=groupSubstage, ..., bUpsert=(StampKyecLotId!=""))`。→ **stamped（WebAPI pull）時所有 group 的 IC 全收斂到 KYEC Lot 下，各 IC 記自己的客戶批/產品/製程段**。
- Lot 記錄的 `sProductCode/sSubstage` 設代表值（last group）；不再是 Soter 來源。
- `StampKyecLotId=""`（開機還原自家 WorkOrder.json / 本地匯入）：`lotKey=groupLOTID`（還原時只有一個 group＝KYEC Lot；本地匯入維持每 group 一 lot 的舊行為），per-IC 欄讀 ICIInfo 的新欄。
- **Maps 分支（WhiteList）完全不動**（第 3 點：不影響正常模式）。
- 解析前 `m_RefreshCount=0`；解析後 `m_RefreshCount>0` → `RecordProcess("WebAPI refreshed N existing 2D codes to latest data: <lot>")`。

**P0-4 `PollLotDataWebApi`（main.cpp:2467）**：改 `LoadFromJsonString(Body, dup, code, sLotApiPullLot)`。SaveWorkOrder（:2470）持久化 per-IC 三欄。

**P0-5 Soter `OpenRow` 兩站改讀 per-IC**（aSortArm.cpp:1517-1525 pick + 1282-1297 reject）：col6=`SoterIc.sCustLotID`、col4=`SoterIc.sProductCode`、col5=`SoterIc.sSubstage`（全來自 FindIcInfo），col7=`Lot->sLotID`（KYEC Lot）。cSoterOutput 內部零改（bucket 仍以 CustLot 拆檔、FTP 仍以 KyecLot 成組）。

**P0-6 掃描迴圈死停修復（B4）→ 拆為獨立 task（未納入本 commit）**：屬既有、與雙批號正交的 async-sweep bug；安全修法需動 retry 計數避免 bad-URL spin，自成一小工程，不混入本案以保持 commit 聚焦。已 spawn task 追蹤。

**P0-7 routing bin 永久寫死 HBIN（第 1 點）**：現況 `iSortBinSource` 已是「建構期設 HBIN、僅 CosFunction.cpp:1504 讀、無 UI/無 ini」的隱性預設；為結構上杜絕未來走到 SBIN，**移除 `iSortBinSource` 成員（CosFunction.h:152）+ 預設設定行（CosFunction.cpp:1850）+ SBIN macro 分支**，把 CosFunction.cpp:1504 直接改成 `int RouteBin=HBin;`（永遠 HBin）。**只影響 routing/分選判斷**；Soter col13(Hbin)/col14(Sbin) 仍照 WebAPI 原值雙欄回報（客戶原始資料透傳，不因寫死而丟 SBin 欄位）。相關註解（:1161/:1429/CosFunction.h:232）一併更新。

### P1 — 同 commit 順手

- **P1-1 ✅ DONE Soter 零顆保底檔**（cSoterOutput.cpp）：col7 token=armed lot（=KYEC Lot）、col6 token 傳字面 `"NA"`；改寫 FTP_SKIP 字句（含 KYEC lot、註明 archive/pickup only）；更新 cSoterOutput.h 註解與 `m_sArmCustLot` 說明。
- **P1-2 ✅ DONE State Record**：`WriteLotDataJson` 的 Items 每筆補 `CustLotID`（f[6]），供 KYEC↔客戶對應診斷（cStateRecordHT160.cpp）。
- **P1-3 SECS SET_LOT_INFO 清 LotBinBinding → 拆為獨立 task（未納入本 commit）**：既有正交問題（SECS 路徑從不清 binding），會把 commit 擴到 uHGemHT160.cpp SECS 子系統，另案。已 spawn task 追蹤。
- **P1-4 64-lot 上限記錄 → 延後（v3 已大幅緩解）**：v3 客戶批不佔 registry slot（收斂到 KYEC lot），cap 壓力遠低於 v1；且 CosFunction 內無 RecordProcess 管道需自建 log channel，另案。

### P2 — 已定案

| # | 事項 | 決議 |
|---|------|------|
| A | routing bin 來源 | **寫死 HBIN**（P0-7，移除選項，第 1 點） |
| B | 零顆批 header-only 檔不 FTP 上傳，只改 log | 已定（第 4 點依建議） |
| C | 復測 upsert（latest-wins + log） | 已定（P0-2，第 1 點） |

### 已延後（使用者指示，不在本案）

- **By Lot+PassFail 改用 WebAPI DiePass 判定（1=pass/0=fail）取代依 Bin 別**：獨立 TODO 下次做（第 1 點）。※注意：本案已把 DiePass 存 per-IC，屆時該功能可直接取用。
- **WhiteList 整合**：後續，且不得影響正常模式；v3 Maps 分支不動天然隔離（第 3 點）。
- **RenameLot per-IC 不改鍵→round-trip 掉 items 資料遺失**：已 spawn 獨立 task。

### v3 相對 v1（蓋章版）已「消失」的改動

因 IC 改掛 KYEC Lot：✗ sOrigin 欄、✗ 掃描跳過、✗ CheckLotDataReady 閘 fallback、✗ edLotNo 翻轉守衛、✗ svLotCount 護欄、✗ 蓋章/自蓋機制——全不需要（客戶批不再是 registry 條目，B1/B3/edLotNo翻轉/膨脹自動消失，見 §3）。

## 3. v3 為何自動解掉 v1 稽核發現

IC 掛 KYEC Lot 後：`edLotNo`=KYEC Lot 本身有 items → **B1 Start 閘直接過**（無需 fallback）。客戶批永不成 registry slot → **B3 掃描誤查 / grid 翻轉 / svLotCount 膨脹全消失**。Soter col7=`sLotID`=KYEC Lot（恆存）、col6=per-IC 客戶批 → **B2 col7=NA + FTP skip 消失**。1:N 由既有 Soter 設計天然處理：bucket 以客戶批拆檔（N 個 CSV）、FTP 以 KYEC Lot 成組（1 個 `/Sorter-log/<KYEC Lot>/` 資料夾 + 1 flag）。只剩 B4（P0-6 順修）、B5（WhiteList 延後）。

## 4. 影響檔案

`CosFunction.h/.cpp`（per-IC 三欄 + AddItemEx upsert + LoadFromJsonString 參數 + tab-line/JSON round-trip + m_RefreshCount）、`main.cpp`（PollLotDataWebApi 傳參 + B4）、`aSortArm.cpp`（OpenRow 兩站改讀 per-IC）、`cSoterOutput.cpp/.h`（P1-1）、`SecsGem/uHGemHT160.cpp`（P1-3）、`cStateRecordHT160.cpp`（P1-2）。零 DFM、無 SECS 出站格式變更。

## 5. 「復測 re-pull」白話（第 6 點）+ v3 處置

機台記住每顆已載入的 2D 碼，同碼不載第二次（防重複計數）。中途沒 Lot End 就再載同一批（host 重送 LOTSTART / 中途重開機再拉）時，第二次每顆碼都已存在。

- 舉例：早上拉 NQ7009，ABC123=HBin1(pass)；客戶下午重測變 HBin3(fail)；機台（沒 Lot End）再拉 → **現況會保留舊的 HBin1、給 KYEC 錯報**。
- **v3 處置（第 1、6 點）**：upsert——碼已存在時**覆蓋成最新** HBin/SBin/RetestCode/DiePass/客戶批/產品/製程段 + 更新 routing bin，並 `RecordProcess` 記「refreshed N existing codes」。既 latest-wins 又留記錄。
- 殘餘 edge（已置換 bin 的 die 若已被放置則來不及改）為 mid-lot re-pull 固有，由 log 覆蓋、註記於程式。

## 6. 驗證計畫

1. **建置閘**：sim `-Full` + 真機（`SOFT_SIMULATE` off）`-Full` exit 0、還原 define、encoding check（動 CosFunction.h 共用 header → 全建）。
2. **模擬情境**（SECS 模擬器 + WebAPI 模擬器）：
   - S1 HEAD 重現：LOTSTART→pull→START 被擋（B1）。
   - S2 修後 1:1 happy path：col6=客戶批 / col7=KYEC Lot、檔名兩 token、FTP `/Sorter-log/<KYEC>/` 成組。
   - **S3 1:N**：一個 KYEC Lot 回 2 個客戶批 group（各自 product/substage/dies）→ 產 **2 個 CSV**（各以客戶批命名、各列 col4/5/6 正確）**同一 KYEC 資料夾 + 1 flag**；分選鍵全為 KYEC Lot。
   - **S4 upsert**：同 KYEC Lot 二次 pull，某碼 bin/DiePass 變更 → 記錄更新為最新 + log「refreshed N」。
   - S5 掃描：只查 KYEC 宣告批、不查客戶批；手動 Fetch 佔線時 sweep 不死停（B4）。
   - S6 斷電續單：WorkOrder.json 帶 per-IC 三欄還原、edLotNo=KYEC Lot、Soter col6/4/5 仍正確。
   - S7 迴歸：本地匯入 / WhiteList 行為與今日一致（未被 v3 動到）。
3. **對抗式複驗**（find→adversarial verify），修完才 commit。
4. 回報逐檔逐行修改點清單。上機真 host/真 WebAPI round-trip 由使用者驗。

## 7. KYEC 回覆（2026-07-22 已鎖定）

- **Q1（同顆 IC 一次回應會否多筆）→ 每顆只回一筆。** → 單次解析內每個 2D 碼只出現一次，**解析內無自撞**；upsert 只在「re-pull（同 KYEC Lot 二次整包拉）」才會觸發，語意乾淨（新記錄取代舊記錄）。
- **Q2（正式 server）→ `:7825` 為正式版、設為預設；`:7835` 為短期測試用，忽略。** → 部署事項：General.ini `[LotWebApi] BaseUrl` 預設指向 7825 正式端點（`http://192.168.11.18:7825/api/GetBISummaryByLot?OSATLot=`）。
- **§8-A（同顆 IC 跨兩客戶批）→ 不會發生**：每顆 IC 有專一的 2D + 專一客戶批。→ 不做 Alarm，僅保留「若偵測到則記 log」的廉價防禦（should-not-happen 保險）。
- **§8-B（一 KYEC Lot 產 N 個 CSV、同資料夾）→ 正確**，KYEC 收檔端接受一資料夾多檔。
- **§8-C（mid-lot re-pull 改到已放置的 die）→ 依建議**：只記 log，不物理回追（重拉固有限制）。

## 8. 已知限制（記錄，不另處理）

- **re-pull 只更新/新增、不刪除**：re-pull 若回傳的 die 集合與首拉不同（少了某顆），registry 內原有的那顆維持舊資料、不會被移除（避免 mid-sort 動態刪 die 的風險）。復測情境 die 集合相同，不受影響；集合改變屬異常，由 §8-C 的 log 涵蓋。
- **§8-C**：mid-lot re-pull 改到已放置 die 來不及回追（log 記錄）。
