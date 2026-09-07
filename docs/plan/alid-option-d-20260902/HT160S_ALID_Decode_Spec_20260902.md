# HT160S ALID 解碼規格（Decode Specification）

**文件版本**：2026-09-02（含 Amendment 1 / 1b / A5）
**對象**：KYEC EAP / Host 端工程師
**適用**：搭載本編碼的 HT160S 韌體版本（build id 於 release note 中標明）
**取代**：先前所有 31-polynomial string-hash ALID 說明，以及 2026-09-02 之前的任何草稿。

> 本文件與機台實作為 1:1。機台端 ALID 由**單一 pure function**（`ComputeAlarmAlid()`，輸入只有
> alarm code 字串）產生，沒有第二條路徑，因此本文件描述的規則即是線上實際行為。

---

## 0. ALID 出現的位置

**S5F1 Alarm Report（Equipment → Host，W bit set）**

    L[3]
      1. B[1]   ALCD
      2. U4[1]  ALID        <-- 本文件的主題
      3. A[n]   ALTX        ( = "<AlarmCode> <English message>" )

**S5F6 / S5F8 Alarm catalog listing（S5F5 / S5F7 的回覆）**

    L[n] of L[3] { B[1] ALCD, U4[1] ALID, A[n] ALTX }     -- 出貨版共 485 筆

ALID 的 wire type 在兩種訊息中都是 **U4（不變）**。

**ALCD 語意（沿用現行，本次不變）**

- S5F1：bit7 = 1 表示 alarm **SET**，bit7 = 0 表示 alarm **CLEAR**；低 7 bits 為 0，
  也就是 **S5F1 不帶 alarm category**。category 請取自 ALID 的 class band（見下）或 catalog。
- S5F6 / S5F8：bit7 = 0，低 7 bits = alarm category（機台 AlarmType）。這是定義列表，不是事件。
- 每支 alarm 會回報兩次：發生時 ALCD = 0x80，操作員清除後 ALCD = 0x00，**兩次 ALID 相同**。

---

## 1. 貴公司當初要求的兩項保證（已達成）

**(1) ALID 恆為「恰好 9 位十進位數字」**

| 項目 | 值 |
|---|---|
| 本規則的結構值域 | 100,000,000 ～ 999,999,999 |
| 出貨版實測最小 | **100,000,913**（`JAM0913`） |
| 出貨版實測最大 | **999,752,848**（一筆 class 9 自由字串） |

永遠不會是 8 位，永遠不會是 10 位。

**(2) ALID 恆可放進 signed 32-bit integer**

    999,752,848  <  2,147,483,647   ( = 2^31 - 1 )

甚至結構上限 999,999,999 也小於 2^31-1。因此可直接用 `int` / `INTEGER` / `NUMBER(9)` 欄位儲存。
**舊編碼「必須以 unsigned 32-bit 解析」的警告正式作廢** — 舊編碼 532 筆中有 46 筆超過 2^31-1
（舊最大值 3,992,930,430），新編碼 **0 筆**。

**其他一併成立的保證**

3. **S5F1 的 ALID 與該 alarm 在 S5F6 / S5F8 catalog 中的 ALID 永遠相同**。兩者由同一個
   pure function 產生，構造上不可能分歧。
4. ALID 只是 alarm **code 字串**的函數，**不隨**時間、lot、operator、訊息文字改變
   —— 唯一例外是 class 9（見第 5 節）。
5. **與貴公司 HT9046LS 的 ALID 空間完全不重疊**。我們重算 HT9046LS 全部 2,738 筆並逐筆
   對照貴公司自己的 `AlarmData.def` 第 8 欄（**0 筆不符 / 2,738**），與 HT160S 新編碼的
   交集為 **空集合**，且以窮舉（非抽樣）方式證明。

---

## 2. 公式

    ALID = Class * 100,000,000 + Payload            (編碼，機台端)

    Class   = ALID / 100,000,000                    (整數除法，不四捨五入)
    Payload = ALID % 100,000,000                    (解碼，Host 端)

---

## 3. Class 表（★ = 2026-09-02 修訂）

| Class | 意義 | 代碼形狀 | Payload | Payload 合法範圍 | 出貨版在役筆數 |
|---|---|---|---|---|---|
| 0 | **永不發送** | — | — | — | 0 |
| 1 | `JAM` 卡料 / 取放 / 傳送 | `"JAM"` + 4 或 5 位數字尾 | 數字尾之值 | 0 ～ 99,999 | 14 |
| 2 | `WAR` 警告 | `"WAR"` + 4 或 5 位數字尾 | 數字尾之值 | 0 ～ 99,999 | 15 |
| 3 | `MES` 訊息 / 操作員提示 | `"MES"` + 4 或 5 位數字尾 | 數字尾之值 | 0 ～ 99,999 | 36 |
| 4 | Cylinder 汽缸 | 恰 5 位純數字，首位 `4` | **整個 5 位碼** | 40,000 ～ 49,999 | 234 |
| 5 | Motor 馬達 | 恰 5 位純數字，首位 `5` | **整個 5 位碼** | 50,000 ～ 59,999 | 180 |
| 6 | Sucker / Vacuum 吸嘴 | 恰 5 位純數字，首位 `6` | **整個 5 位碼** | 60,000 ～ 69,999 | 6 |
| ★ 7 | **Record / Process**（機台 `eRecordProcess`） | 恰 5 位純數字，首位 `7` | **整個 5 位碼** | 70,000 ～ 79,999 | 0 |
| ★ 8 | **Other**（機台 `eOther`） | 恰 5 位純數字，首位 `8` | **整個 5 位碼** | 80,000 ～ 89,999 | 0 |
| 9 | 未登錄的自由字串（**不在 catalog 內**） | 自由文字 | 8 位雜湊折值 | 0 ～ 99,999,999（實測 1,441,895 ～ 99,752,848） | 47 |

合計 **532**（485 筆已登錄 + 47 筆自由字串）。

> ★ **Class 7 / 8 於本版起為「正式的數值 class」**，與 class 4 / 5 / 6 完全同構，**不再是保留位**。
> 請務必**現在就**把 7 / 8 寫進數值分支（見第 4.2 節），而不是丟給 class 9 處理。
>
> 出貨版目前 class 7 / 8 各 0 筆，但將來新增的 `7xxxx` / `8xxxx` alarm **會出現在 S5F6 catalog 內**，
> 屆時貴公司 EAP **無需改程式**即可正確解讀。
>
> （這是本版與 2026-09-02 之前草稿最重要的差異：舊草稿寫「class 7 / 8 保留、比照 class 9 處理」，
> 該寫法**現已錯誤** —— 會讓未來的 `7xxxx` / `8xxxx` alarm 被誤判為「不在 catalog」，
> 明明它在 catalog 裡查得到。）

---

## 4. Payload → Code 規則（含補零規則，Amendment 1b 已消除歧義）

### 4.1 Class 1 / 2 / 3（前綴碼 JAM / WAR / MES）

    prefix = { 1: "JAM", 2: "WAR", 3: "MES" }
    width  = (Payload < 10000) ? 4 : 5                  <-- 恰好，不是「至少」
    code   = prefix + sprintf("%0*d", width, Payload)

★ **Amendment 1b — 尾數規範化（本版新增，關鍵）**

機台端只承認以下**兩種**規範形數字尾，其他形狀一律降為 class 9：

    len(tail) == 4  且  value <  10000        例：JAM0913、MES1421、WAR0963
    len(tail) == 5  且  value >= 10000        例：WAR16120

**這對貴公司的直接好處**：解碼是**雙射（bijection）**，解出來的字串就是機台使用的**唯一**字串，
byte-for-byte 相同。機台上**不存在** `MES01421`、`MES001421` 這類非規範寫法
（那些字串在本規則下非法，會落入 class 9），所以貴公司**不必**處理「同一支 alarm 有多種字串形式」
或「同一個 ALID 對應多個字串」的情況。

**Class 1 / 2 / 3 的合法 Payload 上界 = 99,999。**
若收到 class 1 / 2 / 3 而 Payload >= 100,000，該值為 malformed（本規則不可能產生），請依第 6 節處理。
（Payload 0，即字串尾 `"0000"`，規則上合法但目前無在役成員。）

### 4.2 Class 4 / 5 / 6 / 7 / 8（純數值碼）

    code = sprintf("%05d", Payload)                     -- 必定恰 5 位，無需補零判斷
    合法性檢查： Class*10000  <=  Payload  <=  Class*10000 + 9999

Payload **就是** alarm code 本身，因此代碼的首位數字與 Class 必然相同（class 4 的 payload 必以 4 開頭，
以此類推）。這是刻意設計，換得「payload 即代碼」的可讀性。

選用的進一步分群（非必要）：`4xxxx` 可讀成 `"4"` + 3 位 unit index + 1 位 error type，
例如 `40385` = cylinder unit 038、error type 5。`5xxxx`（馬達）、`6xxxx`（吸嘴）同構。

### 4.3 Class 9

見第 5 節。**不要**用第 4.1 / 4.2 節的規則反推 class 9 的 payload —— 它是雜湊值，沒有代碼可以還原。

### 4.4 Class 0 / 值域外

Class 0，或 ALID < 100,000,000，或 ALID > 999,999,999：**本韌體不會產生**。
若收到，視為 malformed，依第 6 節處理。

---

## 5. Class 9 —— 語意與處理方式

**Class 9 是機台刻意發出的訊號，意思是：「此 ALID 不是 catalog key，請看 ALTX。」**

這些是由**自由文字字串**（sensor 名稱、英文句子、runtime `sprintf`）觸發的 alarm，
從未登錄進機台 alarm code 表，因此**不會**出現在 S5F6 / S5F8 catalog。出貨版共 **47 筆**，例如
`SnFKCleanOut`、`Emergency Stop`、`Safety Door Open`、`Motor Power Off`、`Air Pressure Low`、
20 筆 `<MotorAlias>_MotOverLimitErr`、12 筆 runtime `SUC%03d%d` 吸嘴碼。

**處理原則**

- **(a) 不要**去 catalog 查 class 9 —— 它本來就不在裡面。**查不到是正常的，不是 interface error。**
  ★ 若貴公司 EAP 目前把「unknown ALID」當 interface fault，請在換版前**先放寬 class 9**，
  否則換版後第一次 Emergency Stop 會在真實安全事件之上再疊一個假的 interface alarm。
- **(b)** 人可讀的身分在 **ALTX**，請原文顯示 / 原文儲存。
- **(c)** 機器可讀的 key 請用我們隨附的 **47 筆 class-9 清單**
  （`HT160S_ALID_Class9_FreeString_20260902.csv`：字串 / 舊 ALID / 新 ALID / 備註）。
- **(d) 不要**用「ALTX 的第一個 token」當代碼。多筆字串含空白（`Emergency Stop`、
  `Air Pressure Low`、`Safety Door Open`），且當機台沒有獨立訊息文字時，ALTX 會是該字串**重複兩次**
  （`"Emergency Stop Emergency Stop"`）。請比對**整個 ALTX**，或比對 47 筆清單。
- **(e) 不要**把 class 9 的 ALID 當永久 primary key。它是字串雜湊折成 8 位的結果：
  未來韌體只要修改該字串的措辭，ALID 就會變。其中一筆甚至**隨 UI 語言變動**
  （英文介面 954,803,979 / 中文介面 923,646,915 = 同一支 alarm）。今日 47 筆實測 0 碰撞，
  但 class 9 的唯一性是「量測到的」，不是「構造保證的」。
- **(f)** 我們**打算**在後續版本把這 47 筆登錄成正式代碼，屆時它們會移出 class 9、進入 class 1 ～ 8，
  **這 47 個號碼會再變一次**（其餘 485 筆不受影響）。詳見 transition plan 與風險告知。

---

## 6. 錯誤處理矩陣（請照此實作，不要自行猜測）

| 收到的情況 | 判定 | EAP 應有行為 |
|---|---|---|
| Class 1 / 2 / 3，Payload 0 ～ 99,999 | OK | 解出代碼 → 查 dictionary |
| Class 4 ～ 8，Payload 在 `Class*10000` ～ `+9999` | OK | 解出 5 位代碼 → 查 dictionary |
| Class 9 | `NOT_IN_CATALOG` | 記錄 raw ALID + 完整 ALTX，比對 47 筆清單。**不得**視為錯誤 |
| Class 1 / 2 / 3 但 Payload >= 100,000 | `MALFORMED` | 記錄 raw ALID + ALTX，發 interface warning，**不得丟棄該 alarm** |
| Class 4 ～ 8 但 Payload 超出該 band | `MALFORMED` | 同上 |
| Class 0，或 ALID 非 9 位 | `MALFORMED` | 同上 |
| 解出代碼但 dictionary 查不到 | 對照表版本不符 | 記錄 raw ALID + 代碼 + ALTX，並觸發 S5F5 重新核對 |

**通則（唯一一條硬性請求）**：unknown / malformed 的 ALID **必須**連同原始數值與完整 ALTX 一起記錄，
**永不丟棄、永不對映到最近的值、永不重新雜湊**。

---

## 7. 實例（每個 class 皆列；class 1 ～ 6 與 9 全部取自出貨對照表）

### Class 1（JAM）

| ALID | Class | Payload | 代碼 | 訊息 |
|---|---|---|---|---|
| 100000913 | 1 | 913 | `JAM0913` | Loader Tray Lost On Carriage |
| 100001030 | 1 | 1030 | `JAM1030` | Empty Push Tray Miss |
| 100001611 | 1 | 1611 | `JAM1611` | Auto6 : Auto rear tray data but no-tray sensor |

### Class 2（WAR）

| ALID | Class | Payload | 代碼 | 訊息 |
|---|---|---|---|---|
| 200000154 | 2 | 154 | `WAR0154` | Sorting Arm X motor will out of limit |
| 200000963 | 2 | 963 | `WAR0963` | SECS link lost - AMR handoff still held; clear the station then RETRY |
| 200016120 | 2 | 16120 | `WAR16120` | Top CCD connect not ready (Loader Tray ID no respond) |
| 200016121 | 2 | 16121 | `WAR16121` | Color CCD connect not ready (Color Tray ID no respond) |

> `WAR16120` / `WAR16121` 是**唯二**的 5 位尾數在役碼：payload >= 10000 → width = 5 → 不補零。
> 全機 65 筆前綴碼的尾數長度分布實測為 `{4 位: 63, 5 位: 2}`。

### Class 3（MES）

| ALID | Class | Payload | 代碼 | 訊息 |
|---|---|---|---|---|
| 300000920 | 3 | 920 | `MES0920` | Loader Tray Empty |
| 300001025 | 3 | 1025 | `MES1025` | Empty carriage still holds a tray after clean-out - remove it |
| 300001421 | 3 | 1421 | `MES1421` | Color supply tray is not ready |
| 300001921 | 3 | 1921 | `MES1921` | SortArm blocked - waiting for the destination Auto to receive a tray |

### Class 4（Cylinder）

| ALID | Class | Payload | 代碼 | 訊息 |
|---|---|---|---|---|
| 400040000 | 4 | 40000 | `40000` | The cylinder [C_TrayArmZ_Up_Off] can not off error |
| 400040003 | 4 | 40003 | `40003` | The cylinder [C_TrayArmZ_Up_On] can not on error |
| 400040385 | 4 | 40385 | `40385` | The cylinder [C_Color_FrontSeparateTray_1_On] on sensor is on error |

### Class 5（Motor）

| ALID | Class | Payload | 代碼 | 訊息 |
|---|---|---|---|---|
| 500050000 | 5 | 50000 | `50000` | [M01 - MSortingArmX] Motor --Motor Power Off Error |
| 500050001 | 5 | 50001 | `50001` | [M01 - MSortingArmX] Motor --Motor Out Of Torque Error |
| 500050198 | 5 | 50198 | `50198` | [M20 - MTopCCDX_Color] Motor --Motor Target will Out Of Limit Position |

### Class 6（Sucker / Vacuum）

| ALID | Class | Payload | 代碼 | 訊息 |
|---|---|---|---|---|
| 600060000 | 6 | 60000 | `60000` | Suck--Pick Up Error |
| 600060002 | 6 | 60002 | `60002` | Suck--Vacuum Sensor Off Error |
| 600060005 | 6 | 60005 | `60005` | Suck--Initial Sensor On Error |

### ★ Class 7（Record / Process）—— 結構範例

出貨版**在役 0 筆**，故以下為**規則範例**（非取自對照表）。解碼路徑與 class 4 / 5 / 6 完全相同：

| ALID | Class | Payload | 代碼 |
|---|---|---|---|
| 700070000 | 7 | 70000 | `70000` |
| 700079999 | 7 | 79999 | `79999`（band 上界） |

### ★ Class 8（Other）—— 結構範例

出貨版**在役 0 筆**，同上：

| ALID | Class | Payload | 代碼 |
|---|---|---|---|
| 800080000 | 8 | 80000 | `80000` |
| 800089999 | 8 | 89999 | `89999`（band 上界） |

> Class 7 / 8 首次出現時會**同時**出現在 S5F6 catalog，並附完整 ALTX。
> 請以 S5F5 (`L,0`) 重抓 catalog 取得其文字，不需要我們另外補送檔案。

### Class 9（自由字串 —— 不在 catalog，請由 ALTX / 47 筆清單判讀）

| 新 ALID | 字串 | 舊 ALID |
|---|---|---|
| 991410149 | `SnFKCleanOut` | 3891410149 ← **貴公司 8/21 記錄到的值** |
| 983530353 | `Emergency Stop` | 1983530353 |
| 945115854 | `Safety Door Open` | 3445115854 |
| 963283337 | `Motor Power Off` | 3763283337 |
| 903364399 | `Air Pressure Low` | 3603364399 |
| 945192897 | `SUC0011` | 3145192897 |
| 999752848 | `SortArm move blocked : a suck nozzle left its Home sensor (lost steps). Re-home the suckers.` | 2599752848（= 全機最大 ALID） |

---

## 8. Amendment 1b 的已知後果（命名規範，請一併知悉）

因為 class 1 / 2 / 3 只承認 4 位或 5 位規範形尾數：

- **unit 01 ～ 09：每個 unit 上限 100 支 alarm**（尾數形狀 `0UNN`）。
  5 位尾數 `0UNNN` 為**非法**，因此**不會**出現 `WAR09120` 這樣的代碼；
  unit 09 若將來需要超過 100 支 alarm，我們**必須改用 unit >= 10 的編號**。
- **unit 10 ～ 99**：可用 `UUNN`（100 支）**與** `UUNNN`（1,000 支），每個 unit 合計上限 **1,100 支**。
- 影響：未來新增的 alarm，其編號可能與貴公司依現有規律推測的不同（會跳到較大的 unit 號）。
  這是刻意的取捨 —— 換得「一個 payload 值 ↔ 一個代碼字串」的唯一對應。

**容量餘裕（供貴公司評估欄位長度與未來擴充）**

| Class | 可用槽 | 在役 | 使用率 |
|---|---|---|---|
| 1 JAM | 100,000（4 位尾數 10,000 + 5 位尾數 90,000） | 14 | 0.014% |
| 2 WAR | 100,000 | 15 | 0.015% |
| 3 MES | 100,000 | 36 | 0.036% |
| 4 Cylinder | 10,000（碼形恰 5 位） | 234 | 2.34% |
| 5 Motor | 10,000 | 180 | 1.80% |
| 6 Sucker | 10,000 | 6 | 0.06% |
| 7 Record / Process | 10,000 | 0 | 0% |
| 8 Other | 10,000 | 0 | 0% |
| 9 自由字串 | 100,000,000 | 47 | 實測 0 碰撞 |

---

## 9. 自我核對 —— 機台是唯一權威

貴公司**不需要**信任任何試算表。發送 **S5F5 帶空 list（`L,0` = request all）**，機台會以 **S5F6**
回覆全部 **485** 筆 catalog，每筆帶 ALCD、U4 ALID、ALTX，全部直接來自線上韌體表。

**強烈建議**：在每次 link establish（S1F13 / S1F14 之後）自動執行一次
`S5F5 → S5F6 →  與 dictionary 逐筆核對`，有差異即告警。這一步可以**整類消除**
「對照表與韌體不同步」的問題（見 transition plan）。

核對時請注意：catalog 是依 **alarm code 字串的字典序**輸出，因此 S5F6 內的 ALID **並非**遞增排列，
若需排序請在 Host 端自行排序。

---

## 10. 本次不變的部分（因此不需要重新驗證）

- **無任何** CEID / SVID / ECID / report definition 變更。
- ALID 線上型別仍為 **U4**。
- ALCD 語意不變。
- ALTX 仍為 `"<AlarmCode> <English message>"`。
- 貴公司現行的 S5F3 alarm-enable `L[2]{ B 0x01, L[0] }`（enable all）**不指名任何 ALID**，
  因此無需修改，也**不可能**因換版而讓某支 alarm 被停用。

---

## 11. 隨附檔案

| 檔案 | 內容 |
|---|---|
| `SECS_GEM功能_Handler_20260902.xlsx` | 新 SSOT workbook（485 列，含 Class / Payload 解碼欄；0831 版退為歷史） |
| `HT160S_ALID_Map_Old_to_New_20260902.csv` | 532 列新舊對照（AlarmCode / AlarmType / Class / Payload / OldALID / NewALID / Changed / Registered / Message） |
| `HT160S_ALID_Class9_FreeString_20260902.csv` | 47 筆 class-9 自由字串清單 |
| 本文件（`HT160S_ALID_Decode_Spec_20260902`） | 解碼規格 |
| Bench capture | 新韌體實際 S5F5 → S5F6（485 筆）+ 各 class 的 S5F1 set / clear 對 |
| Release note | 導入本編碼的韌體 build id，以及「不變清單」 |
