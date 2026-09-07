# TODO：Top / Color CCD 2D 掃描失敗時，Note 畫面允許人員手動補填 2D 資料

> 狀態：規劃中（未實作）。建立日期：2026-06-26。
> 編碼規則：本文件 UTF-8；改碼時 BCB6 `.cpp/.h/.dfm` 為 Big5，新增註解 ASCII English。

## 需求

Top CCD（IC 2D）與 Color CCD（盤身分 2D）掃描失敗、跳出 Note 提示時，
目前操作員只能選 **Retry / Skip**。需求是再提供一個**人工輸入**選項，
讓人員可手動鍵入正確的 2D 字串，系統以該字串繼續後續流程（Bin 查表 / 誕生身分盤），
等同一次成功的掃描，而非被迫 Skip 成 Error/Empty。

## 現況掛點（三處，皆只有 K_RETRY|K_SKIP）

| # | 來源 | 位置 | 訊息 | Skip 現行後果 |
|---|---|---|---|---|
| 1 | Color CCD 無回應 | `aLoader.cpp` 同型，見 `aColor.cpp:799` | "Color CCD 2D no response" | `sTrayID2D=""` → `BirthIdentityTray()`（空 2D 身分盤） |
| 2 | Top CCD 無回應 | `aLoader.cpp:1297` | "Top CCD 2D no response" | Bin=`HT160_BIN_ERROR_NO_BIN_SETTING`、Lot=-1、Code2D="" |
| 3 | Top CCD 查無此碼 | `aLoader.cpp:1274` | "2D code not found in any lot : "+sCode | Bin=Error、Lot=-1（保留 sCode） |

## 規劃修法（草案，待決策）

1. **輸入 UI**：Note/MyMessageBox 不可用 VCL `InputBox`/`ShowMessage`
   （見記憶 [[alarm-message-use-mymessagebox-or-note]]）。需確認 HT160 既有
   是否有可帶文字輸入的對話框；若無，評估擴充 `note` / `mymessagebox`
   或借用 QwertyKey 軟鍵盤（參考登入流程 [[user-account-password-feature]]）。
2. **流程接回**：
   - Color（#1）：手填字串 → `sTrayID2D=<手填>` → `BirthIdentityTray()` → return true。
   - Top（#2/#3）：手填字串 → 走與 `b2DOk` 成功相同的 `LotRegistry.FindByCode2D()`
     反查 Bin/Lot 分支；查得到照正常綁定，查不到則退回現有 Error 處理或再次提示。
3. **權限 / 稽核**：手動補填屬人為覆寫，需確認是否限定權限等級、是否要寫
     EventLog / LotHistory 記錄「manual 2D = <值>」以利追溯。
4. **決策點**：
   - 三處是否都要支援，或僅 Top CCD？
   - 手填後 SECS 上報是否要標記為 manual？
   - 是否需與 HT172 對應行為對齊（先查 HT172 同畫面是否有手填）。

## 鎖定決策（2026-06-26 user）

1. Top CCD 手填仍查無碼 → **繼續給提示**（迴圈再 prompt，不直接轉 Error；Retry 才回去重掃、Skip 才轉 Error）。
2. 現場**有手持掃描槍** → Note 上放可聚焦 `TEdit`，掃描槍輸出字元 + 結尾 CR 自動送出；軟鍵盤 uQwertyKey 僅退路。
3. `edtManual2D` **大小比照 `BtnCleanOut`，位置等距放在 `BtnCleanOut` 下方**（PanelCommand 內）。
4. 手填值要**雙寫**：EventLog（`MANUAL 2D entered : <code>`）+ **Production_Log 新增欄位**標記手動輸入值；**SECS 維持不變**。

## 關聯待辦：Production_Log 欄位檢視（user 2026-06-26 點出需修正）

Production_Log 由 `TDeviceInfo`（deviceinfo.cpp/.h）寫出，欄位：
`Start Time,Load_X,Load_Y,Load_Time,Tray_ID,Bin,Output tray,Unload_X,Unload_Y,Unload_Time,Error log,TraceCode,ErrorType,Lot,Code2D`（deviceinfo.cpp:72-75）。

調查結論（2026-06-26）：**5 個欄位全是規劃了沒接線的空殼**。
- 3.1 `Tray_ID`(eLoadTrayID)：**永遠空白**。唯一 writer `AddInputInfo` 被 aSortArm.cpp:1124 傳寫死 `""`。不是 Loader 入盤順序碼；IC 2D 走 `Code2D` 欄。
- 3.2 `Output tray`(eOutTrayID)：**永遠空白**。`AddOutputInfo` 被 aSortArm.cpp:1160 傳 `""`；目的地 Auto 寫進 `Which Auto`(eWhichAuto)。
- 3.3 `Error log`(eErrorCode)：**死碼**。唯一 writer `SaveRejectRecord`(deviceinfo.cpp:240) 零呼叫。設計=不良 IC 自由文字錯誤字串。
- 3.4 `TraceCode`(eTraceCode)：**死碼**。唯一 writer `AddTraceInfo`(deviceinfo.cpp:190) 零呼叫。0=正常,999=ScanFail/1000=NoMap/1001=2DMapMissing/1002=ParseFail/1003=SortFail。
- 3.5 `ErrorType`(eErrorType)：**死碼**,只由 AddTraceInfo 依 TraceCode 推導,無獨立 writer。
- 新增欄位安全位置：**APPEND**(enum 加在 e2DCode 之後、`PROD_LOG_FIELD_COUNT` 19→20、header 末端加 `,Manual2D`)。無 in-app parser 讀此檔;外部 MES/Excel 靠欄位順序,**絕不可中插**。per-IC 旗標需經 tray-cell→Slot[]→AddIcIdentity 載體(同 Code2D)。詳見記憶 [[prodlog-dead-placeholder-columns]]。

實作進度（2026-06-26 全部完成，待上機驗證）：
- Phase 1 (Note: K_MANUAL_2D=0x0040 / edtManual2D Top=303 / edtManual2DKeyPress) — DONE。**keypress 只 capture+log+Close,不設 SoftStart（不恢復機台,只 Start 鍵可，user 邊界）**，見 [[operator-keypress-no-auto-resume]]。
- Phase 2 (aColor case200 + aLoader case5500 兩處掛點 + `BindManual2D` **有界迴圈** for(iGuard<100)+後備出口) — DONE。EventLog 在 note.cpp 集中記 `MANUAL 2D entered : <code>`。
- Phase 3 (Production_Log `Manual2D` 欄；載體 tray-cell `bManual2D[][]`→`Slot.bManual2D`→`AddIcIdentity(...,bool)`→`eManual2D`；APPEND，count 19→20，header 末加 `,Manual2D`，手填="1" 否則空) — DONE。
- 檔案：note.h/.cpp/.dfm、aColor.cpp、aLoader.h/.cpp、MyMotor.h/.cpp、aSortArm.h/.cpp、deviceinfo.h/.cpp。全 byte-safe。
- Build gate：dev `-Full` EXIT=0；真機(SOFT_SIMULATE off) `-Full` EXIT=0（首次 link 撞暫時性檔案鎖，重 link 即過）；`SOFT_SIMULATE` 已還原並 dev `-Full` 重建。編碼檢查 160 檔過。
- 上機待驗：掃描槍 CR 自動送出進 edtManual2D；Top 手填查無→繼續提示、查到→正常分流；Color 手填→身分盤；Production_Log 末欄 Manual2D=1。

附加（2026-06-26，依建議執行）：把原本死碼的 `TraceCode`/`ErrorType` 欄接活。aSortArm pick 時(AddIcIdentity 之後)依 slot 既有資料推導呼叫 `AddTraceInfo(SlotIndex, code)`：`Code2D==""`→999 ScanFail；有碼但 `LotIndex<0`→1000 NoMap；正常→0(空)。不需新載體(用 slot 現有 Code2D/LotIndex)。dev build EXIT=0。仍為死碼：`Tray_ID`/`Output tray`(寫死"")、`Error log`(SaveRejectRecord 零呼叫)。

## Build gate

改 `.cpp/.h/.dfm` 後刪對應 `.obj` 再 `scripts/ops/build-ht160s.ps1`（結構變更用 `-Full`），
並跑 `scripts/ops/check-ht160s-source-encoding.ps1`；sim + 真機（`SOFT_SIMULATE` off）皆 exit 0 後交付，上機由使用者驗證。
