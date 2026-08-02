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

> **2026-08-02 修訂（本文第一版有兩處錯誤，已更正）**
> 第一版把 CEID 48 與 78 列為「可補、零阻塞」。逐條查證後**兩項都不成立**：
> 48 **早已實作**（我的發射點掃描漏抓了 `EventReport(1, SECS_EVENT.X)` 這種帶 `1,` 前綴的寫法），
> 78 則**需要一個目前不存在的操作員輸入框**才有意義（見 §三）。
> CEID 70 的語意也已從 9045 原始碼定案，結論與第一版的兩種猜測都不同。
>
> **2026-08-02 第二次更新：CEID 78 已裁定並實作完成**（commit `8082f0f`）。
> §三 描述的 (a)(b)(c) 三個動作全部已做，且整個功能**預設關閉**
> （`[SECS] AskSkipICCount=false`），所以“警報路徑多一個對話框”這個行為改變
> 只在現場明確開啟後才發生；未開啟前機台行為與舊版逐位相同。

| 分類 | 數量 | 意義 | 該做什麼 |
|---|---|---|---|
| **A. 假缺口** | **5** | 我方**有發射點**，07-31 只是沒觸發那個動作 | **什麼都不用做** |
| **B. 需新功能才有意義** | **1** | 有發射時機，但**沒有 host 要的那個數字** | ✅ **已實作** `8082f0f`（預設關）|
| **D. 機構不存在** | **13** | 測試機／9045 專屬概念，分選機沒有 | **正確的「沒有」，不是缺口** |
| **E. host 自己關掉** | **1** | host 在 15:02:57 明確 disable | 不用管 |

**修訂後的結論：「20 個缺口」裡沒有任何一個是「補一行就好」的便宜項目。**
唯一還有討論空間的是 CEID 78，而它是一個**需要新增操作員輸入流程**的功能，不是一行發射。

---

## 二、A 類：假缺口（5 個）— 我方有發射點，當天只是沒觸發

| CEID | 名稱 | HT-160S 發射點 | 9045 06-08 |
|---|---|---|---|
| **9** | Switch Real Dummy Mode | `SECS_EVENT.SwitchRunMode`（`main.cpp`） | 未發 |
| **14** | Switch StartMode | `main.cpp:1782` | 發了 16 次 |
| **15** | Switch Setup File | `SECS_EVENT.SwitchSetupFile`（`main.cpp`） | 發了 1 次 |
| **48** | Change EC | **`SecsGem/uHGemHT160.cpp:1194`**（20260729 已加） | 未發 |
| **275** | AMR LD ID | `SecsGem/uAgvStation.cpp:217` | 發了 2 次 |

這五個只是 07-31 當天**沒有人切 Real/Dummy、沒切 Start Mode、沒換 recipe、沒改 EC、沒有身分盤上傳**。
機台一旦做這些動作就會發。

> ⚠ **發射點掃描的兩個盲點（我自己踩過，記下來）**
> HT-160S 的 `EventReport` 有**三種寫法**，只掃其中一種會漏判：
> 1. `EventReport(SECS_EVENT.X)`
> 2. `EventReport(1, SECS_EVENT.X)` ← **CEID 48 漏在這裡**
> 3. `EventReport(1, 275)` 字面值 ← **CEID 275 漏在這裡**
> 另有陣列式 `EventReport(1, AutoCeid[i])` / `AutoFullCeid[i]` 需人工展開。

---

## 三、B 類：CEID 78 — 有發射時機，但**沒有 host 要的那個數字**

第一版說「兩個資料源本週剛好都到位，補一行就好」——**那是錯的**。逐條查證後：

**host 要的到底是什麼**（從它自己的訂閱回推）：

```
host_links_S2F35.csv :  CEID 78  ->  RPTID 517
host_reports_S2F33.csv: RPTID 517 -> { SVID 37010 }
9045 uHGemHT9045_SV.cpp:681 : SVID 37010 = "Enter Skip IC Count"  -> &iJamSkipIC
```

**9045 的完整流程**（`note.cpp:2170-2189`）：

1. 操作員在**卡料警報**選 **SKIP**
2. 機台跳出輸入框 **「Many ICs Taken Out From The Tray :」**
3. 操作員**手動輸入**他從盤裡拿走幾顆 → `iJamSkipICCount`
4. `iJamSkipIC = iJamSkipICCount`
5. **才**發 CEID 78

**所以 CEID 78 的全部意義，就是載那個「操作員手動拿走幾顆」的數字**——
用途是 MES 的庫存對帳。沒有那個輸入框、沒有 SVID 37010，**發 CEID 78 只會送出一個空事件**。

**HT-160S 的現況**：

| 我們有的 | 是什麼 | 能不能當 37010 用 |
|---|---|---|
| `tRunData.iAutoSkipCount` | **機台**在 SortArm 取料失敗時**自動**跳過的格數 | ❌ 不同的量（機台判定 vs 操作員申報） |
| `tRunData.JamCount` | 本週建立的 jam 事件計數 | ❌ 是「發生幾次卡料」，不是「拿走幾顆 IC」 |
| 操作員輸入框 | **不存在** | — |

**要做就是一個真功能，不是一行發射**：
(a) 在 jam-SKIP 路徑加一個「你從盤裡拿走幾顆？」輸入框、
(b) 新增 SVID 37010 綁那個數字、
(c) 才發 CEID 78。

**✅ 2026-08-02：上述 (a)(b)(c) 已全數實作（commit `8082f0f`）。**
下面那個“會在每天走到的警報路徑多一個對話框”的顧慮，是用**設定旗標**解決的：
`[SECS] AskSkipICCount` 預設 `false`，未開啟時完全不會跳輸入框、也不會發 CEID 78；
要用時在 General.ini 把它改成 `1` 即可。操作員按取消（不填）則不發任何事件——
沒回答的問題不等於“拿走 0 顆”。

⚠ (a) 會**在操作員每天都會走到的警報路徑上多一個對話框**——這是行為改變，需要人裁定，
不應由我單方面加。**替代方案**（送 `iAutoSkipCount`）我不建議：SVID 名稱是
`"Enter Skip IC Count"`，`Enter` 就是「操作員輸入」的意思，塞機台自算的數字進去會誤導 host。

---

## 四、CEID 70 — 語意已定案，結論是「我們沒有」

第一版猜「可能是進入條碼頁、也可能是讀到條碼」。**兩個都不對**。從 9045 原始碼定案：

| 觸發點 | 情境 |
|---|---|
| `BarcodeReader.cpp:107` | 操作員在條碼輸入框按 **ENTER** → 字串存進 `asSecsGemBarCode` |
| `mymessbox.cpp:1334` | **ID／密碼驗證對話框**輸入後 → 同樣存進 `asSecsGemBarCode` |

也就是說 CEID 70 = **「操作員用條碼槍／鍵盤輸入了一個字串（工號、密碼、料號）並按 ENTER」**，
目的是把那個字串交給 host。它**不是** 2D 盤碼讀取。

**HT-160S 沒有這個東西**：我們的批號來自 WebAPI 與 CCD 的 2D 讀取，不是操作員條碼輸入；
操作員身分變更我們已有 **CEID 16 Switch UserLevel** 在發。
→ **歸入 D 類（機構不存在），不實作。**

---

## 五、D 類：分選機／9045 專屬機構（13 個）— 正確的「沒有」

| CEID | 名稱 | 為什麼分選機沒有 | 9045 06-08 |
|---|---|---|---|
| 10 | Switch Tester Online | 沒有測試機連線 | 未發 |
| 13 | Switch Temperature Mode | 沒有控溫機構 | 未發 |
| 70 | Barcode Reader Enter | 操作員條碼／密碼輸入，我們沒有這條輸入鏈（見 §四） | 發 13 次 |
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

## 九、建議執行順序（修訂）

| 順序 | 項目 | 阻塞 | 工作量 |
|---|---|---|---|
| 1 | **發文請京元改 RPTID 524 的綁定**，並加訂我們已在發的 16 個事件 | **需京元** | **零（我方）** |
| 2 | CEID 78 —— 已裁定並**實作完成**：jam-SKIP 路徑加操作員輸入框 + SVID 37010，預設關閉 `[SECS] AskSkipICCount=false` | ✅ `8082f0f` | 已完成（上機待驗）|
| 3 | CEID 49 Tray Feed Finish（需先實作 `CheckAllTrayFeedFinish()`，目前是回 false 的 stub） | 無 | 中 |

**第一版列的「CEID 48、78 各補一行」已作廢**：48 早已實作，78 需要新功能。
本文修訂後，這 20 個裡**沒有任何一個是我方可以單方面、低成本補完的**——
（CEID 78 後來經使用者裁定而實作，但它確實不是“一行發射”，而是一個帶設定旗標的新功能）。
真正划算的一手在第 1 項，而且工作量在京元那邊。
