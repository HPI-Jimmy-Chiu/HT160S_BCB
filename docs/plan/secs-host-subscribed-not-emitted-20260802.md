# 京元 host 訂了、HT-160S 沒發的 20 個事件 — 逐條裁定

- 日期：2026-08-02
- 分支：`feat/iosetview-172-refactor`
- 問題：2026-07-31 現場 log 顯示 host 用 S2F35 綁了 **31 個 CEID**，其中 **11 個**當天真的收到，
  **20 個**一則都沒收到。本文逐條說明那 20 個是什麼、為什麼沒發、該不該補。

## 證據來源

| 來源 | 路徑 | 用途 |
|---|---|---|
| 京元 host 訂閱 | `docs/plan/onsite-0731-kyec-secs/host_links_S2F35.csv` | host 綁了哪些 CEID |
| HT-160S 當天實際發射 | 07-31 SECS log（27 種、去除 spam 後 8317 行） | 我方實際發了什麼 |
| **9045 CEID 字典** | `D:\backup_version\HT9046\KYEC\20260626\EventReport_CEID.def` | **292 筆**官方名稱（本次新用） |
| **9045 同址實際 log** | `D:\backup_version\HT9046\KYEC\20260626\2026_06_08\`（20 檔） | **9045 當天發了 46 種、645 則**（本次新用） |
| HT-160S 發射點 | `EventReport(SECS_EVENT.*)` + `EventReport(1, nnn)` 全樹掃描 | 我方有無發射點 |

⚠ 本文的分類經我本人逐條開檔覆核；工作流的對抗式複驗因 session 額度中斷未執行。

---

## 一、結論摘要

| 分類 | 數量 | 意義 | 該做什麼 |
|---|---|---|---|
| **A. 假缺口** | **4** | 我方**有發射點**，07-31 只是沒觸發那個動作 | **什麼都不用做** |
| **B. 可補** | **2** | 沒發射點，但資料/機構已存在 | 我方可自行補，零外部阻塞 |
| **C. 需確認** | **1** | 語意不明，要先問清楚 9045 的定義 | 問京元或看 9045 觸發點 |
| **D. 機構不存在** | **12** | 測試機專屬概念，分選機沒有 | **正確的「沒有」，不是缺口** |
| **E. host 自己關掉** | **1** | host 在 15:02:57 明確 disable | 不用管 |

**所以「20 個缺口」實際上只有 2 個是真缺口。**

---

## 二、A 類：假缺口（4 個）— 我方有發射點，當天只是沒觸發

| CEID | 名稱 | HT-160S 發射點 | 9045 06-08 |
|---|---|---|---|
| **9** | Switch Real Dummy Mode | `SECS_EVENT.SwitchRunMode`（有） | 未發 |
| **14** | Switch StartMode | `main.cpp:1782`（有） | 發了 16 次 |
| **15** | Switch Setup File | `SECS_EVENT.SwitchSetupFile`（有） | 發了 1 次 |
| **275** | AMR LD ID | `SecsGem/uAgvStation.cpp:217` `EventReport(1, 275)`（有） | 發了 2 次 |

這四個只是 07-31 當天**沒有人切 Real/Dummy、沒切 Start Mode、沒換 recipe、沒有身分盤上傳**。
機台一旦做這些動作就會發。

> ⚠ 275 我第一次掃描時漏抓——它用**字面值** `EventReport(1, 275)` 而非 `SECS_EVENT` 常數。
> 掃描發射點時兩種寫法都要涵蓋。

---

## 三、B 類：可補，且我方零阻塞（2 個）

### CEID 48 — Change EC（設定值被修改）

- **9045**：06-08 也沒發（該日沒人改 EC）。
- **HT-160S 現況**：無發射點，但**我們已經處理 S2F15 EC 寫入**
  （`SecsGem/uHGemHT160.cpp:1120` 的 `ECID>=2758 && ECID<=2763` 閘）。
- **補法**：在 S2F15 寫入成功後發 CEID 48。工作量極小。
- **價值**：host 訂了它，代表它在意「誰改了機台參數」——這是可稽核性，MES 常見需求。

### CEID 78 — Jam Skip IC Count（卡料/跳過顆數）

- **9045**：06-08 也沒發。
- **HT-160S 現況**：無發射點，但**兩個資料源本週剛好都到位**——
  `tRunData.JamCount`（本週建立的 jam 機制）與 `tRunData.iAutoSkipCount`（既有）。
- **補法**：在 jam 計數或 auto-skip 遞增時發。
- **價值**：這正好接上操作員筆記第 6 條「jam rate」的同一條資料鏈，host 端也能收到。

---

## 四、C 類：語意需確認（1 個）

### CEID 70 — Barcode Reader Enter

- **9045 06-08 發了 13 次**——是它當天第 6 高頻的事件，顯然是實際在用的功能。
- **語意不確定**：可能是「操作員進入條碼輸入頁面」，也可能是「條碼讀取器讀到資料」。
- HT-160S 有 Top CCD / Color CCD 的 2D 讀取，也有 `uQwertyKey` 條碼輸入介面，
  **兩種語意我們都可能有對應**，但補錯邊等於送假資料。
- **動作**：查 9045 的觸發點（`Command.cpp` / `main.cpp` 搜 `BarcodeReaderEnter`）確認語意後再決定。
  這是我方可自行解決的，不必問京元。

---

## 五、D 類：分選機無此機構（12 個）— 正確的「沒有」

| CEID | 名稱 | 為什麼分選機沒有 | 9045 06-08 |
|---|---|---|---|
| 10 | Switch Tester Online | 沒有測試機連線 | 未發 |
| 13 | Switch Temperature Mode | 沒有控溫機構 | 未發 |
| 26 | Get Test Result | 沒有測試結果（我們的 bin 來自 WebAPI/2D，不是測試） | 發 12 次 |
| 34 | Auto Clean Start | 9045 的 auto-clean 是**測試接點清潔**；我們的 Clean Out 是**排盤**，是不同概念且已有 CEID 4/42 | 發 1 次 |
| 44 | Site On Off | 沒有測試 site | **發 31 次** |
| 45 | Arm On Off | 9045 的測試手臂啟停 | 未發 |
| 50 | Auto Clean Finish | 同 34 | 發 1 次 |
| 67 | Tray Test Finish | 沒有盤測試流程 | 發 6 次 |
| 125 | EESUG Offest Select | 測試機專屬 offset | 未發 |
| 126 | EESUG Offest Modify | 同上 | 未發 |
| 212 | Energy Saving Start | 無節能機構；我們對 `ENERGY_SAVING` RCMD 回 **HCACK=2（認得但不執行）**，與 9045 給京元的答案一致 | 發 1 次 |
| 213 | Energy Saving End | 同上 | 發 1 次 |

> **34/50 特別說明**：名稱容易誤解。9045 的 `Auto Clean` 與 HT-160S 的 `Clean Out` **不是同一件事**，
> 9045 自己同時有 CEID 4 `CleanOut Pressed` 和 CEID 34/50 `Auto Clean`。我們有前者、沒有後者。

> **67 特別說明**：host 把 **RPTID 524 = {38237, 38238, 38239}（Auto4-6 盤數）綁在 CEID 67 上**。
> 那是測試機事件，我們永遠不會發 → **Auto4-6 的盤數因此永遠到不了 host**。
> 這不是我方缺陷，是 host 綁錯事件（見 §七）。

---

## 六、E 類：host 自己關掉（1 個）

### CEID 80 — Read Now Handler Data

07-31 `15:02:57.752` host 送 `S2F37 <L[2] <Boolean 0x00> <L[1] <U4 80>>>` = **明確 disable CEID 80**。
它先在 `14:57:41` 關閉全部、`15:02:56` 重新開啟全部、然後**只挑 80 關掉**。
所以這一個是 host 刻意不要的。

---

## 七、順帶查到：要請京元改的兩件事

1. **RPTID 524 綁錯事件**：`{38237,38238,38239}` 是 Auto4-6 的盤數，綁在 CEID 67
   （測試機事件，分選機永不發射）。**建議請他們改綁到已在 CEID 272 上的 RPTID 2001**，
   否則 Auto4-6 盤數永遠收不到。
2. **我們已經在發、他們卻沒訂的 16 個事件**——包含 **Lot End(8)、Pause(2)、
   Auto 卸盤(136-138/145-147)、UPH 記錄(53)、Load Tray Finish(66)**。
   這些**我方零工作量**，他們在 provisioning 加幾行綁定就收得到。

---

## 八、9045 素材的其他發現（本次新讀）

### 8.1 CEID 27 佔 9045 全日事件的 63%

9045 06-08 共發 **645 則、46 種**，其中 **CEID 27 Change Machine State 就發了 406 則**。
這**證實了我們本週補 SVID 1011（Machine State）的優先順序是對的**——它是 host 端資訊量最大的單一欄位。

### 8.2 `EventReport_ReportID.def` 只有一列

全檔只有 `1 → 1027`。這**印證了先前的判定**：9045 的 `AddReprot()` 是個 no-op bug
（迴圈跑 275 次但永遠寫死 RPTID 1），所以 9045 出廠時除 CEID 1 外每個事件都送空 `L,0`。
**我們沒有照抄那個 bug 是對的。**

### 8.3 9045 會發、我方無發射點的 16 個（超出本文那 20 個的範圍）

`23 Enter Debug Page`、`26`、`34`、`44`、**`49 Tray Feed Finish`**、`50`、`55 Initial ART Start`、
`58 Ready for ART`、`59 ART Receive Tray OK`、`60 ART Receive Tray START`、`63 FT Finish`、
`67`、`70`、`212`、`213`、`250 START Auto contact height`。

其中**只有兩個對分選機有意義**：

- **CEID 49 Tray Feed Finish**：我們有 Tray Feed 功能，但 `CheckAllTrayFeedFinish()` 是
  **回傳 false 的 stub**（`csystem.cpp:1955-1961`），所以永遠發不出來。這是既有的已知缺口。
- **CEID 23 Enter Debug Page**：我們有維護頁面，可對應。低價值。

其餘 13 個全是 ART/FT/contact-height 等測試機流程。

---

## 九、建議執行順序

| 順序 | 項目 | 阻塞 | 工作量 |
|---|---|---|---|
| 1 | CEID 48 Change EC（S2F15 寫入成功後發） | 無 | 小 |
| 2 | CEID 78 Jam Skip IC Count（接本週的 jam 計數） | 無 | 小 |
| 3 | CEID 70 語意查證（讀 9045 觸發點）→ 決定補不補 | 無（我方自查） | 小 |
| 4 | 發文請京元改 RPTID 524 的綁定，並加訂我們已在發的 16 個 | **需京元** | 零（我方） |
| 5 | CEID 49 Tray Feed Finish（要先實作 `CheckAllTrayFeedFinish()`） | 無 | 中 |
