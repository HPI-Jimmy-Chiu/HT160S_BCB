# HT160S Loader 工作盤數顯示異常 — 根因分析與修正驗證報告

- **日期**：2026-07-13
- **分支**：`feat/iosetview-172-refactor`
- **修正 commit**：`111b976 fix(loader): SECS LoaderTrayCount is work-only; firmware adds cover/identity header`（2026-07-13 17:17）
- **相關模組**：`aLoader.cpp` / `main.cpp` / `GeneralSetting.*` / `SecsGem/uAgvStation.cpp`
- **相關記憶**：`ht9045-loader-amr-traykind-model`、`ht9045-cleanout-trigger-model`

---

## 1. 問題現象

AMR 模式下，host 透過 SECS 送 **8 盤** work 給 Loader；Loader 餵入 **2 盤**後，Motion View 的
Loader 工作盤數顯示 **剩 4 盤**。操作端預期為 `8 − 2 = 6`。

證據來源：State Record 快照 `2026-07-13 15_58_42`
- [FeederDecision.txt](../../../HT160S_StateRecord)：`iCarTrayTotal=8  iSecsCarTrayCount=8  iFeedSerial=2  Remain=6`
- 快照內 `system/General.ini` **無 `[AMR]` 區段**（僅 `[SimAMR]`）

---

## 2. 根本原因

顯示位置：`main.cpp` 的 `ShowCarTrayCount()` → `lbCarTrayCount_Loader`，格式為
`identity / cover / work`，由 `FmtCarKinds()` 計算。

舊版 `aLoader.cpp::RefillSimInfeed()` 將 host 宣告的盤數**直接當成整車實體總數**：

```
iCarTrayTotal = iSecsCarTrayCount = 8        // 錯：SECS 的 8 是「純 work 盤數」，被當成整車總數
餵 2 盤後 → iSimInfeedCount = 6
FmtCarKinds(6, id=1, cv=1) → "1/1/4"         // work = 6 − 1(identity) − 1(cover) = 4
```

依「9045 AutoUP」對齊規格（已攻防驗證，見記憶 `ht9045-loader-amr-traykind-model`）：

> **SECS `LoaderTrayCount` = 純 work 盤數；firmware 才另外加上 cover/identity 表頭盤**
> （`iAMRLDSECSTrayCount = iSECSSetTrayCount + iAMRCoverTray`）

因此 8 應為 **8 片 work**，實體車總數 = `8 + cover(1) + identity(1) = 10`，餵 2 後 work 應剩 6。
舊碼把 8 當總數，顯示端又扣掉表頭，等於**被雙重扣減**，才會出現 4。

---

## 3. 修正內容（`111b976`）

`aLoader.cpp::RefillSimInfeed()` 改為對 work-only 的 SECS 數加上表頭：

```cpp
if(GeneralSetting.bUseAMR && iSecsCarTrayCount>0)
{
    int iHeader = GeneralSetting.iAmrCoverTray[0]
                + ((GeneralSetting.iAmrIdentityTray[0]>0) ? GeneralSetting.iAmrIdentityTray[0] : 0);
    iCarTrayTotal = iSecsCarTrayCount + iHeader;   // 8 + 2 = 10
}
else
    iCarTrayTotal = GeneralSetting.iSimAmrMaxTray[0];
```

- 表頭組成改為 config 化：`General.ini [AMR] CoverTrayN / IdentityTrayN`
  （`GeneralSetting.iAmrCoverTray[9] / iAmrIdentityTray[9]`，索引 `0=Loader,1=Empty,2=Color,3..8=Auto1..6`）。
- `GetFedTrayKind()` 的種別邊界與 `FmtCarKinds()` 顯示改為讀同一份 config。
- 預設值 `CoverTray0=1, IdentityTray0=1` 重現舊有硬編碼行為；無 `[AMR]` 區段時亦套用此預設。

修正後：

```
iCarTrayTotal = 8 + 2 = 10
餵 2 盤後 → iSimInfeedCount = 8
FmtCarKinds(8, 1, 1) → "1/1/6"               // work = 6  ✓
```

---

## 4. 驗證方法與結果

### 4.1 決定論邏輯 harness（工作盤數重現）

GUI + SECS 全流程無法 headless 驅動，故以 g++ 忠實複製
`RefillSimInfeed / FmtCarKinds / GetFedTrayKind` 的舊/新邏輯，跑快照同一情境
（SECS=8、cover=1、identity=1、餵 2）。原始碼：[loader_worktray_count_harness.cpp](loader_worktray_count_harness.cpp)

執行輸出：

```
=== Scenario: SECS=8 work-only, cover=1, identity=1, fed=2 ===

[OLD pre-111b976]  iCarTrayTotal=8
  after 2 feeds: iSimInfeedCount=6  Motion-View label="1/1/4"  => work=4
  full car kind map: 1..6=work 7=COVER 8=IDENTITY  => car of 8 = 6 work + 1 cover + 1 identity

[NEW post-111b976] iCarTrayTotal=10  (= SECS 8 + header 2)
  after 2 feeds: iSimInfeedCount=8  Motion-View label="1/1/6"  => work=6
  full car kind map: 1..8=work 9=COVER 10=IDENTITY  => car of 10 = 8 work + 1 cover + 1 identity

=== RESULT ===
OLD work = 4  (expect 4, the reported bug)  -> MATCH
NEW work = 6  (expect 6, correct)           -> MATCH
VERDICT: CONFIRMED - fix turns 4 into 6
```

OLD 的 `iCarTrayTotal=8` 與快照值完全一致 → 佐證快照為修正前 binary 產生。

### 4.2 Compile-clean（現行 HEAD）

`scripts/ops/build-ht160s.ps1 -Clean` → **exit 0**
- encoding 通過（161 files）、alarm-registry 通過、`__published` shape 無違反
- （唯一 ladder WARN 為 `UpdateSecsFeatureBadge` 既存警告，與本修正無關）

### 4.3 獨立攻防複複（5 攻擊軸）

由獨立 reviewer agent 對 `111b976` 嘗試反證，結論：**程式面正確且完整，無殘存 bug。**

| 攻擊軸 | 結果 |
|---|---|
| 算術／一致性 | ✅ 成立（`feedSerial` 為 1-based，於 `aLoader.cpp:1597` 先自增再於 `:1599` 呼叫，故 `>` 邊界正確；四處共用同一模型） |
| Real 機餵料風險 | ✅ 反被推翻——source-dry 由 `SnLoader_Inputend` sensor 驅動，非 count 驅動；**且本修正順帶消除一個真機誤報**（見下） |
| Config 邊界 | ✅ `SetDefault()` 與 `Load()` 預設一致、索引 `[0..8]` 不越界；cover=0 / id=0 / SECS=0 皆合理 |
| 其他 `iCarTrayTotal` 使用者 | ✅ 乾淨；`+2` 不外洩至任何 SECS SVID、CleanOut、Auto 供料或 TrayArm |
| 編碼／BCB6 規範 | ✅ ASCII 英文註解、無 C++11、無 DFM 變更、`[AMR]` 與 `[AGV]` 區段無 key 衝突 |

**額外收穫（非自明）**：舊碼 `iCarTrayTotal=work數(8)`，餵完 8 片 work 後 `iFeedSerial=8`、
`(8−8)=0`，但實體上的 cover+identity 盤仍使 `SnLoader_Inputend` 維持 ON →
case-9000 交叉檢查（`aLoader.cpp:1474`）會**誤報 MES0921**。修正成 10 後正確涵蓋，
故此修正同時修掉一個真機假警報。

---

## 5. 時間軸（證明快照為修正前）

| 時刻 | 事件 |
|---|---|
| 15:58:42 | State Record 快照（`iCarTrayTotal=8`、無 `[AMR]` 區段、work 顯示 4） |
| 17:17:17 | 修正 commit `111b976` |
| 18:24:52 | EXE 重新編譯（含修正） |

15:58 快照早於 17:17 的修正，且快照內 `iCarTrayTotal=8`（修正後應為 10），確認其為**修正前 binary** 產生。

---

## 6. 結論與待辦

**結論**：數字異常屬實（work 應為 6），根因為舊碼把 SECS work-only 盤數當整車總數；修正 `111b976`
已將 4 正確變 6，並通過 harness、clean build、二次獨立攻防三重驗證，且順帶消除一個真機誤報 MES0921。

**待辦（非程式問題，需上機驗證）**：
1. **承載前提**：整套修正依賴「host 的 SECS `LoaderTrayCount` = 純 work 盤數，實體磁匣另含 cover+identity」。
   須以真實 AMR/SECS 上機驗證一次。若某客戶 host 實際送的是「實體總數」，`+header` 會反向多算，
   屆時把 `[AMR] CoverTray0 / IdentityTray0` 設為 0 即可。
2. **極低風險硬化（選擇性）**：Loader 的 `[AMR] IdentityTray0` 若被誤設為 `-1`（Color 專用哨符），
   `FmtCarKinds` 會顯示 `t/0/0` 而餵料邏輯視為 0 identity，兩者不一致；預設 `+1` 不受影響。

---

*附註：本報告的 harness 為離線邏輯複本，僅驗證盤數/種別的算術與一致性；實機 Motion View 顯示仍以
18:24 之後的 build 上機重跑為準。*
