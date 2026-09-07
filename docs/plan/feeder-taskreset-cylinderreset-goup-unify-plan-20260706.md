# Plan — Feeder 狀態機統一：return-true 歸零 + 汽缸 Reset + GoUp idiom/settle

狀態：**PLAN — 等使用者對「變更對照表」確認後才落 byte-safe 編輯。**
日期：2026-07-06
分支：`feat/iosetview-172-refactor`
來源：現場快照 `D:\HT160S_StateRecord\HT160S_Program_BCB_V1.0.0.0_20260706`（帶未進 git 的現場修正）+ 使用者確認的統一方向。
關聯記憶：`onsite-20260706-feeder-task-reset-fixes`、`cleanout-finish-design`、`destacker-unify-color-empty-loader`、`cylinder-idiom-pushcylinder-canonical`。

---

## 1. 根因回顧（為什麼要做）

`IsCleanOutFinish`（[aColor.cpp:1211](../../HT160S_Program_BCB_V1.0.0.0/aColor.cpp)、[aEmpty.cpp:958](../../HT160S_Program_BCB_V1.0.0.0/aEmpty.cpp)）有 idle gate：
```cpp
if(FeedTask!=1 || GoDownTask!=1 || GoUpTask!=1) return false;
```
它假設「子程跑完後 Task 回到 1」。但 `Do*` 的終態是直接 `return true` **未歸零**（GoUp 停在 10000、GoDown 停在 700、Feed 停在 13000），於是完成的 GoUp 被誤判為「還在跑」→ CleanOut 永不完成。
（Auto 用 drain latch、Loader 用 `bCleanOutFinish` latch，故此 idle gate 只在 Color/Empty＝不一致套用；`aAuto1To6.cpp:976` 早已註明 `FeedTask==1` gate 會 false-block。）

## 2. 使用者已確認的設計方向

- **(D1) return-true 歸零 — 走「單一終態 case 收斂」**：每個 `Do*` 在 switch 內**只留一個** `return true` 的**純終態 case**，該 case 只做 `<Task>=1; return true;`（無副作用）。其餘所有「完成 / 提早結束」的點改成 `<Task>=<DONE>; break;`，副作用（birth / release 等）留在自己的 case 跑完再跳 DONE。`if(Flag==0)` 的 init 出口維持 `<Task>=1; …; return true;`（那是外部 re-init 契約，不計入 switch 內的唯一 return）。
  - 對「終態本來就是純 `return true`」的函式（GoUp case 10000），等同只加一行 `<Task>=1;`。
  - 代價：完成時多一個 tick（跑到 DONE 才回 true），對節拍與安全無影響，與既有 ladder 一致。
- **(D2) 汽缸 Reset**：每次 `PushCylinder`/`PopCylinder`（＝ stateful `.Push()`/`.Pop()`）**之前一步**呼叫該缸 `.Reset()`（清內部 Task）。`.On()`/`.Off()` 內部已自歸 Task=1，**不需**外加 Reset。
- **(D3) GoUp idiom + settle（點 4）**：把 Color/Empty 的 `DoGoUpTray` 從舊 `.On()/.IsOn()||sim`、`.Pop()||sim` 轉成 `PushCylinder/PopCylinder + GoUpDelay(iEmpty/ColorDestackSettleMs)` 對稱 settle（拿到到位確認 + 逾時告警 + 一致 settle），這才是「太快/不順」的對症修法。互鎖（`IsFrontSeparateBlockedBy`）原樣保留。

## 3. 分層（建議 Tier 1 先做、build+驗證後再 Tier 2）

- **Tier 1（核心：直接修 CleanOut finish + Empty 不順 + 安全；且都在 idle gate 上）**
  1. Color + Empty 的 `DoGoDownTray` / `DoGoUpTray` / `DoFeedTray` 套 D1。
  2. Color `DoGoDownTray` 六個 PushCylinder/PopCylinder 前補 `.Reset()`（對齊 Empty 現場版）。
  3. Color + Empty `DoGoUpTray` 套 D3（轉 PushCylinder/PopCylinder + settle + Reset）。
  4. **aLoader 兩項現場修正 port 進 repo**（見 §5）。
- **Tier 2（一致性；不在 idle gate 上，風險/收益較低）**
  5. Loader `DoFeedTray` / `DoCcdCheck` / `DoDischargeTray` 套 D1（注意 per-side `State->` + 9500 reroute 不受影響）。
  6. Loader `DoFeedTray` 前端 Lean/Push 夾缸、`DoFrontDestackDown` 的 legacy idiom 補 Reset / 視需要對齊。
  7. Color `DoReadColor2D`(ScanTask) / `DoSortBin` 的 return-true 歸零；三模組 `Test*`（teach 測試）對齊 idiom。

## 4. 變更對照表（依 §1 盤點，行號為現行 repo HEAD）

### 4.1 aColor.cpp
| 函式 | Task | 現有 return true（非 init） | D1 動作 | 汽缸 Reset / idiom |
|---|---|---|---|---|
| DoGoDownTray | GoDownTask | case1 `L458`、case10 `L465`、case700 `L563`(birth) | 加純 DONE case；三處改 `GoDownTask=DONE;break;`（case700 birth 後跳 DONE） | **補 Reset ×6** 於 L474/479/484/501/516/530 前一步 |
| DoGoUpTray | GoUpTask | case10000 `L706`(純) | case10000 → `GoUpTask=1;return true;` | **D3 轉換** L601–643(前段) + L666–689(後段搬運)；每 Push/Pop 前 Reset |
| DoFeedTray | FeedTask | case1 `L790`、case10 `L813`、case13000 `L945` | 加純 DONE case；三處改 `FeedTask=DONE;break;` | case2000 `DoClampTray` 已內含 Reset；L894/899 PopCylinder 前補 Reset |
| DoReadColor2D | ScanTask | L975/1008/1030/1049/1055 | Tier 2：五處收斂到單一 DONE | 無缸 |
| DoSortBin | SortBinTask | case1 `L1109`(stub) | Tier 2：`SortBinTask=1;return true;` | 無缸 |
| TestGoDownTray/TestGoUpTray | Test* | 已歸零 | 不動歸零 | Tier 2：idiom/Reset 對齊 |

### 4.2 aEmpty.cpp
| 函式 | Task | 現有 return true（非 init） | D1 動作 | 汽缸 Reset / idiom |
|---|---|---|---|---|
| DoGoDownTray | GoDownTask | case10 `L543`、case700 `L637`(birth) | 加純 DONE case；兩處改 `GoDownTask=DONE;break;` | **現場已補 Reset ×6**（L551/556/564/581/596/610 前）→ 保留 |
| DoGoUpTray | GoUpTask | case10000 `L773`(純) | case10000 → `GoUpTask=1;return true;` | **D3 轉換** L665–709(前段) + L731–751(後段搬運)；每 Push/Pop 前 Reset |
| DoFeedTray | FeedTask | case10 `L402`、case13000 `L478` | 加純 DONE case；兩處改 `FeedTask=DONE;break;` | case2000 `DoClampTray` 已內含 Reset；L438/443 Pop 前確認 Reset |
| TestGoDownTray/TestGoUpTray | Test* | 已歸零 | 不動 | Tier 2：idiom/Reset 對齊 |

### 4.3 aLoader.cpp
| 函式 | Task | 現有 return true（非 init） | D1 動作 | 備註 |
|---|---|---|---|---|
| DoFeedTray | State->FeedTask | case10 `L1251`、case10000 `L1485`(ReleaseFrontOwner) | Tier 2：加純 DONE；case10000 跑 ReleaseFrontOwner 後跳 DONE | **9500 reroute、bSupplyCarDry 見 §5，先於重構 port** |
| DoCcdCheck | State->CcdTask | case1000 `L1532` | Tier 2：收斂單一 DONE | per-side |
| DoDischargeTray | State->DischargeTask | case4000 `L1894` | Tier 2：case4000 副作用後跳 DONE | per-side |
| DoFrontDestackDown | SubTask | case6 `L1956` | **已歸零 SubTask=1** → 不動 | 是正解範本 |
| TestGoDownTray/TestGoUpTray | Test* | 已歸零 | 不動 | — |

> 註：§1 盤點對部分 `Flag==0` 出口標了 `resetsTaskToOne:false`，經直接核對其實都有設 `Task=1`（標註誤判）。落編輯前每個 `Flag==0` 會再逐一核對。

## 5. aLoader 現場修正 port（獨立於重構，先做）

1. **`bSupplyCarDry` 再 AND `InputHasTray`**（[aLoader.cpp:1069](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp)）— 退站前確保輸入端也無盤＝機台內盤全處理完才退站。運算子優先序已驗（`?:` < `&&`/`||`，兩 `||` 群組各有括號）。原封搬入。
2. **MES0920 `K_RETRY`/`K_CLEAN_OUT` → `FeedTask=9500`**（[aLoader.cpp:1420/1434](../../HT160S_Program_BCB_V1.0.0.0/aLoader.cpp)）— 避免重跑 case3500→`DoFrontDestackDown` 重降 rise1+rise2 夾斷盤；改走 9500 確認+建盤+帶去 CCD。`MachineType.h` 的 `SOFT_SIMULATE` 不進 repo。

## 6. GoUp 轉換草圖（D3，以 Empty 為例，Color 同構）

現行（舊 idiom，節錄）：`case100 Rise1.On → case200 Rise1.IsOn+Separate.On+settle → case300 settle+Rise2.On → case400 Rise2.IsOn+Separate.Off+settle → case500 settle+Rise2.Off+Rise1.IsOn → case600 Rise1.Pop`。
轉換後（mirror DoGoDownTray 的 PushCylinder/PopCylinder + GoUpDelay，每動作前 `.Reset()`；保留 case200 的 Loader 分張互鎖）：每個 `.On()`→`PushCylinder`、`.Off()/.Pop()`→`PopCylinder`，動作間一律 `GoUpDelay.SetMS(iEmptyDestackSettleMs); On()` → 下一 case `GoUpDelay.Off()` 才前進。後段搬運 case3000–7000（LeanOn/PushTray）同樣改 PushCylinder/PopCylinder + settle。**確切逐 case 新碼在落編輯前於本節補完並附 diff。**

## 7. 風險 / 建置閘

- **Big5 byte-safe**：三檔皆 legacy Big5；用 `bcb6-bytesafe-edit.ps1` / python Latin1 splice；新註解 ASCII English；注意 EOL（記憶：aLoader.cpp=LF）。避免 BCB designer save 洗掉元件。
- **SOFT_SIMULATE 守衛**：`TMyCylinder::Push/Pop` 全包在 `#ifdef SOFT_SIMULATE`；GoUp 轉 PushCylinder/PopCylinder 會改變 sim 行為（sim 直接 return true）與真機路徑 → **必須 sim + 真機雙編譯**（切 `MachineType.h` 的 define 跑 `-Full`，確認 exit 0 後還原）。
- **建置**：每檔改完刪對應 `.obj` 再編；wiring 不變故 `build-ht160s.ps1 -Clean` 為主，另做一次真機 `-Full`。encoding check 腳本。
- **落地後覆查**：對 diff 跑一次對抗式 review（確認無 dead-jump、無漏歸零、互鎖與 9500 reroute 未被破壞、Push/Pop 前都有 Reset）。
- **on-machine 驗證**：CleanOut 能正常完成、Empty GoUp 順、Loader 缺料復歸不夾盤 — 由使用者上機確認。

## 8. 待使用者確認

- (Q1) Tier 1 先做並 build/驗證、再做 Tier 2？（建議）還是一次全做？
- (Q2) DONE case 編號慣例：GoUp 沿用既有 10000；GoDown/Feed 新增一個高位純終態（擬用各自 `<現終態+一個新號>`，例如 GoDown 用 `case 9999`）。可否？
- (Q3) `Test*`（teach 測試）與 `DoReadColor2D`/`DoSortBin` 是否納入（Tier 2）？
