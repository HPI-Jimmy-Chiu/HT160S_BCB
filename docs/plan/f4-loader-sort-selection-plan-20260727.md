# F4 — Loader 共軌 Y 軸 sort 死鎖：選側/授權修復計畫

**日期**：2026-07-27　**狀態**：計畫待審（尚未實作）
**來源**：2026-07-24 京元(KYEC)上機手記 #4；多代理幾何分析 + 設計 (wf_a14f461e-1cd)

---

## 0. 一句話結論（先看這個）

使用者想要的「授予 SortArm 使用權前，比 encoder（R 領先+有盤+有IC 就不給 L）」這個檢查，**在目前程式碼裡加上去會是 no-op**——因為現有的 **ready-gate（[aLoader.cpp:1891](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:1891)）保證「同一時間只有一台車是 READY_SORT」**，所以 `GetSortingLoaderNo` 永遠只有一台候選車，encoder 比較永遠不會觸發。要讓這個檢查「真的有作用」，必須**放寬 ready-gate（讓兩台都能 READY_SORT）**，而放寬後非 active 載盤車會停在掃描帶內擋住 active 車，於是**又得補「讓位到 feed」的協調**。也就是說：簡單版是 no-op，有效版是一組較侵入性的 6 處改動（medium confidence，且動到一個保護性不變式）。

**因此本計畫的第一建議是：先確認 F4 在目前 build 是否真的可重現，再決定要不要做這組侵入性修改。**

---

## 1. 已確認的根因事實（都對照過現行碼）

- SortArm 一次只吸一台載盤車。選側 = `GetSortingLoaderNo()`（[aLoader.cpp:907](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:907)）**位置盲**：`if(ready(1)) return 1; if(ready(2)) return 2;`，不比 encoder、不比 IC。
- `AcquireSortOwner`（[aLoader.cpp:926](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:926)）只 gate 在 `iYOwner==NONE && IsLoaderReadyForSort(self)`，**完全不看對面車**。
- sticky-commit（[aSortArm.cpp:2351](../../HT160S_Program_BCB_V1.0.0.0/aSortArm.cpp:2351)）只「黏著已 commit 的側」，不做 encoder 比較、不管初次選側。
- 撞軌後盾 `IsLoaderYMoveSafe`（[aLoader.cpp:347](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:347)）：兩台都載盤時，領先車(|enc|大)只能移到 `|Tgt| ≥ |Other|+SafeDist`；落後車只能移到 `|Tgt| ≤ |Other|−SafeDist`；空車忽略；鍵在 `fHasTray` 不看 Status。

## 2. 幾何（用實際 recipe 算死，單位 1/100mm）

Recipe `Test20260611`：YDivision=3、YPitch=102mm、YStart=55.5mm、YDatumBias=−1000；SafeDist=**32500**。

| | Feed | 首 CCD | 掃描帶頂(停車位) | 首 Sort row | Sort rows(row0/1/2) |
|---|---|---|---|---|---|
| L1 | 100 | 16101 | **36501** | 42278 | 46828 / 57028 / 67228 |
| L2 | 100 | 13104 | **33504** | 42128 | 46678 / 56878 / 67078 |

關鍵幾何：
- **`Sort row0 − 掃描帶頂停車位` ≈ 100~130mm < SafeDist 325** → 一台載盤車停在掃描帶頂(33504/36501)會**擋住**對面領先車的下緣 sort row。
- **`Sort row0 − Feed` ≈ 465mm > SafeDist** → 停在 **feed(100)** 的載盤車**不擋**任何 sort row。
- **FindPickCells 一律先吸 row0（最低、最靠 feed、最難到）**（[aSortArm.cpp:959](../../HT160S_Program_BCB_V1.0.0.0/aSortArm.cpp:959)）→ 只要落後車停在 ~142mm 以上，領先車第一吸就卡住。
- 讓位可行時機：落後車退 feed 需 `100 ≤ leaderPos − 32500` → **leader ≥ 32600(326mm)** 才合法。leader 一旦在 sort 帶(≥42000)必成立。

## 3. 分析分歧（← 這是要你決策的核心）

兩個分析代理對「**F4 死鎖在目前碼是否真的可達**」給出**相反**結論：

- **幾何代理**：`yield_needed = yes`——若兩台都能載盤進帶，選側不夠，還要讓位。
- **可達性代理**：`no yield required`——現行三道閘 **feed-gate([1351](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:1351)) + 帶內互鎖 + ready-gate([1891](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:1891))** 保證「同一時間只有一台 READY_SORT，另一台載盤車被擋在 feed」，於是**active 車永遠不被擋、F4 死鎖不可達**；現場 2026-06-25 看到的那個死鎖是 **2026-06-24 sticky-commit + ready-gate 之前的舊行為**，可能**已經被關掉了**。

**未解**：2026-07-24 手記 #4 是「當場觀察到的 hang」還是「工程師讀碼記下的風險」？我在 `D:\HT160S_StateRecord` 的 07-24 資料只有 CSV + LDJ trace，**沒有能證明 iYOwner 卡住的完整 MachineState 快照**，所以無法從現場資料證實 F4 當天真的觸發。

## 4. 方案設計（若確認 F4 可達才做）

設計代理（medium confidence）給的最小方案 = **選側 + 讓位**，6 處改動、2 個檔（+ .h）：

- **EDIT 1**（[aLoader.cpp:907](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:907) `GetSortingLoaderNo`）：兩台都 ready → 選 `GetLoaderYAbsPos` 較大者(領先車)；單台 ready → 若對面「載盤且領先」則回 0(defer)，否則回該側。
- **EDIT 2**（新 helper，[aLoader.cpp](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp) ~292 + .h）：`GetLoaderYAbsPos(int)`（abs encoder，手動負號無 std::abs）、`IsLoaderLoadedAndLeading(cand,other)`。
- **EDIT 3**（[aLoader.cpp:1891](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:1891) ready-gate）：**放寬**，只在對面 `LS_SORTING` 時 hold（拿掉 `||LS_READY_SORT`）→ 讓兩台都能 READY_SORT，EDIT 1 才選得到領先車。**← 這是最有爭議的一改：它鬆綁了一個保護性序列化不變式。**
- **EDIT 4**（新 helper `YieldTrailingSiblingToFeed` + 呼叫點 [aSortArm.cpp:1835](../../HT160S_Program_BCB_V1.0.0.0/aSortArm.cpp:1835) DoPickFromLoader case 30 的 else）：pick-Y 被擋時，把「落後、在帶內、未被 SortArm 擁有」的對面載盤車 `MoveLoaderY(→feed)`。全程走 `IsLoaderYMoveSafe`（撞不了）、單調(逼近 feed)、到 feed 冪等。
- **EDIT 5**：`aLoader.h` 宣告三個新 method；新增 `static const int FEED_YIELD_TOL=300`。

**安全性（設計代理主張）**：所有 Y 移動仍走未改的 `IsLoaderYMoveSafe` → 不可能撞；讓位單調逼近 feed → 不 livelock；選側穩定黏領先車 → 不 ping-pong。

## 5. 誠實的殘留風險（設計代理自陳，confidence=medium）

- **(a) 142–326mm 凍結子情境**：若兩台都卡在 ~142–326mm 帶（leader<326 且 trailing>142），讓位一時不合法、leader 也吸不到 row0，要等 leader 先爬過 326mm 才解。窄但非證明不可能，需真機確認會自解。
- **(b) EDIT 3 鬆綁 load-bearing 不變式**：任何隱藏依賴「只有一台 READY_SORT」的消費者（CleanOut 完成判定、卸盤排序…）都是回歸風險。
- **(c) abs 相等平手**：兩車 abs Y 相等時無嚴格領先者，讓位回 false，理論上可能無界卡住（teach 值不同，實務罕見）。
- **(d) 陳舊 commit**：若記憶體已 commit 到落後車（本修部署前的殘留），讓位救不了（領先的對面車過不去 feed）——實務靠部署重 HOME 清掉；真出現則**停住(不撞)**而非自癒。
- 撞軌/livelock 證明僅 static+sim 等級；**「各種交錯下不死鎖」的宣稱必須靠真機兩車 State Record 驗**。

## 6. 建置/驗證（若實作）

1. 刪 `aLoader.obj`/`aSortArm.obj`；因動 `aLoader.h` → 走 full build。
2. Sim：`scripts/ops/build-ht160s.ps1 -Clean` exit 0。
3. 真機閘：`MachineType.h` 註解 `#define SOFT_SIMULATE` → `-Full` exit 0 → 還原重編。
4. 編碼檢查（兩 .cpp 為 Big5，須 byte-safe 編修，無 EF BF BD / BOM）。
5. `--selftest-home` 綠燈；SECS 模擬器 twocar 腳本看 iYOwner 循環、無 case-30 spin。

## 7. 上機驗收（唯一能定案）

重現 2026-06-25 幾何（兩車載盤、一台上 sort、另一台在掃描帶 24000–36500）：確認落後車**退到 feed**、領先車**吸完含 row0 的所有 row**、無 case-30 spin、iYOwner 正常循環、兩盤依序吸完無人工介入、無 gap 警報；且**非競爭時 pipeline 吞吐不回歸**（2026-06-24 操作員確認的互不干涉不被破壞）。

---

## 8. 建議路徑（決策點）

- **路徑 A（推薦先做）—— 先確認可達性，暫不改碼**：既然可達性代理認為現行三道閘可能已擋掉 F4、且使用者要的 encoder 檢查在現碼是 no-op，先用**真機兩車情境 / 07-24 完整 State Record**確認 F4 是否真會發生。若不可達 → F4 其實已被 2026-06-24 的修改關掉，結案，不動 gate-1891。
- **路徑 B（若確認可達）—— 實作 §4 的 6 處選側+讓位**：medium confidence、動到 gate-1891、附完整建置+上機驗收，並把 §5 殘留風險列為觀察項。

> 我的專業建議：**先走 A**。放寬一個保護性 gate 去修一個「可能已經不可達」的死鎖，風險/報酬不划算；先花小成本確認，再決定要不要做 B。

---

## 9. Path A 執行結果（2026-07-27）— F4 極可能**已不可達**

**現場資料不足以直接判定**：`D:\HT160S_StateRecord` 只有 3 份 CSV + `_ldj_trace.txt`(23 次快照全數完成) + notes + source，**無任何 MachineState/DescribeState 快照、無 State Record zip**（`D:\HT160S_Log` 亦空）。所以改以**靜態可達性分析**（親手把幾何代理 vs 可達性代理的分歧推到底）。

**關鍵樞紐：掃描時的爬升移動也走互鎖。** `MoveToCcdCell → MoveLoaderY → IsLoaderYMoveSafe`（[aLoader.cpp:506](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:506),[322](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:322)）。所以「非 active 載盤車」要爬進掃描帶，同樣受互鎖限制。

**逐步推導（active 車在 sort 帶 ≥42000 時，非 active 載盤車能到哪）：**
- 非 active 車爬升上限 = `activePos − SafeDist`。active 在 row0(L1 46828/L2 46678) → 上限 ≈ 14178~14328(≈142mm)。
- L2 的首 CCD row = 13104 < 14328 → L2 **爬得到首 CCD row**；此時 L1(leader) row0 46828 ≥ 13104+32500=45604 → **OK，餘裕 +12.24mm**。
- L1 的首 CCD row = 16101 > 14178 → L1(當 L2 在 sort) **連首 CCD row 都爬不到**，被鎖在 feed；L2(leader) 對 feed 有巨大餘裕。
- 兩車都在掃描帶(33504/16101)並存 = 幾何上會擋(幾何代理正確)，**但那組位置在 active 車已上 sort 帶時，非 active 車根本移不進去**(可達性代理正確)。加上 **feed-gate([1351](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:1351)) 只讓一台在前段 + ready-gate([1891](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp:1891)) 只讓一台 READY_SORT**，非 active 載盤車被**限制在 feed / 首 CCD row(≤~142mm)**，永遠**不擋** active 車的 sort row。

**結論**：**在目前 build，F4 sort 死鎖不可達**（非 active 載盤車恆被閘門+互鎖壓在 feed/首 CCD，active 車恆可吸完）。現場 2026-06-25 那次是 **2026-06-24 (`be79adb` gate-1891 加 SORTING) + 2026-06-25 sticky-commit** 之前的**舊行為**，這些修改已把它關掉。07-24 手記 #4 讀來是**讀碼記下的風險**，非當天觀察到的 hang（無快照佐證）。

**唯一的敏感點**：L1 領先、L2 停首 CCD row 時，row0 餘裕僅 **+12.24mm**。teach 值若漂移可能翻負——值得列為 teach 敏感度觀察，但目前為正。

**最終建議**：
- **不要做 Path B**（不放寬 gate-1891、不加 yield）。F4 已被現有閘門序列化擋掉；使用者想要的 encoder 檢查在現碼是 no-op —— 但那**沒關係**，因為序列化已由閘門達成。
- **定案仍需上機**：跑兩車、盯 `DoPickFromLoader` 是否卡 case 30 / iYOwner 是否卡 SORTARM；若始終不發生 → 確認已修。**若真的重現，把當下的 State Record zip 給我**，我再走 Path B。
- 順帶：可考慮把 row0 的 +12mm 餘裕記入 teach 敏感度清單。
