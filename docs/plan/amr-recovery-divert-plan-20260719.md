# AMR 模式回收盤直供 Auto（divert 解禁）作戰計畫

- 日期：2026-07-19
- 分支：feat/iosetview-172-refactor
- 狀態：**已核可（2026-07-19「全照建議」：D1 預設 OFF／D2 UI 勾選框／D3 divert log／D4 kind 過濾）→ 已實作＋雙組態建置通過；待模擬情境與上機驗證**
- 範圍：僅前次分析的「① AMR 模式被整個排除在外」缺口；②（imminent hold）與③（量測 log 全套）不在本計畫內。

---

## 1. 目標

AMR 模式下，TrayArm 從 Loader rear 回收的**Normal 空盤**，在「夾到手當下」與「送往 Empty rear 途中／等待淨空時」若偵測到有 Auto 正在要 Normal 盤，直接送去該 Auto，不再固定「先放 Empty rear → 之後再夾回」，消除必然發生的雙重 Z 循環（外加 Empty 站一輪多餘的 GoUp 讓位）。

Normal（非 AMR）模式行為**完全不變**（該模式的 divert 已存在且運作中）。

## 2. 現況與缺口（錨點）

| # | 位置 | 現況 |
|---|------|------|
| 1 | `aTrayArm.cpp:1029` `TryDivertCarriedTrayToAuto` | `if(GeneralSetting.bUseAMR) return false;` — AMR 全面禁 divert |
| 2 | `aTrayArm.cpp:982` `DecidePlaceDestAfterPick` | `bUseAMR==false` 才考慮供 Auto；AMR 回收盤一律回 Empty |
| 3 | `aTrayArm.cpp:935` `DoPlace` case 4000 | 放盤通知分支看 `Job==TAJOB_AMR_SUPPLY`：AMR 走 `NotifyTrayArmDelivered`（stamp RearKind/RearTrayID），其餘走 `SetRearHasTrayFromTrayArm`（**不碰 RearKind**） |
| 4 | `aAuto1To6.cpp:1241` `FindTrayRequestAuto` | 回傳「第一個有需求的 Auto」，不分 kind；AMR 車在 identity/cover 階段（車數 n<2，`GetNextTrayKindForAuto` aAuto1To6.cpp:1193）時第一需求者的 kind 不是 Normal |
| 5 | `aLoader.cpp:598` `GetFedTrayKind` | AMR 模式回收盤 kind 可能是 Normal/Cover/Identity（identity 疊最後、cover 次之）；非 AMR 一律強制 Normal |

**缺口 3 是本計畫的正確性關鍵**：若只解除缺口 1/2 的閘，divert 過去的工作型別仍是 `TAJOB_LOADER_RECOVERY`，case 4000 會走 `SetRearHasTrayFromTrayArm`，RearKind 殘留前一次配送的值（可能是 Cover/Identity）→ `DoFeedTray` 拉盤時 `WorkingKind[i]=RearKind[i]`（aAuto1To6.cpp:710）→ `IsReadyForSortArmPlace` 拒絕放 IC（aAuto1To6.cpp:1316）→ **空盤直通 discharge、堆疊多一片、車帳錯**。

## 3. 設計定案

1. **只直供 Normal 對 Normal**：攜帶盤 kind 必須是 `eTrayKindNormal`，且目標 Auto 當下要求的 kind 必須是 `eTrayKindNormal`（= 車已疊完 identity+cover、進入工作盤階段）。回收到的 Cover/Identity 盤維持現行路徑（Cover→Empty 回收、Identity→Color 掃碼），**絕不插壞 identity→cover→normal 堆疊序**。
2. **kind 沿用 Auto 自己的 `GetTrayRequest`**，divert 只改「送去哪」，不覆寫 kind、不動 IC 去向。
3. **新增 opt-in 旗標** `bUseAmrRecoveryDivert`（General.ini `[SortMode]`，預設 OFF），仿 `bUsePredictiveAutoSupply` 全套接線（含 maintenance UI 勾選框、FeederDecision [Config gates] dump）。旗標關 = 行為與現況 bit-for-bit 相同。
4. `FindTrayRequestAuto` 增加**可選 kind 過濾參數**（預設不過濾 = 既有呼叫端行為不變），否則「第一需求者正好要 identity/cover」時 divert 永遠不觸發，即使別的 Auto 正在等 Normal 盤（車剛換走、正重疊 identity/cover 是 AMR 常態時段）。
5. CleanOut 期間、`bAmrLocked`、rear 已占用／pending 等拒絕條件全部由既有 `GetTrayRequest`（aAuto1To6.cpp:1213）內建把關，不另建守衛。

## 4. 變更清單

### C1. `GeneralSetting.h`（~:91，緊鄰 `bUsePredictiveAutoSupply`）
新增 `bool bUseAmrRecoveryDivert;`

### C2. `GeneralSetting.cpp`（三點，全仿 predictive 旗標）
- 建構子預設（~:97）：`bUseAmrRecoveryDivert=false;`
- Load（~:197）：`Ini->ReadBool("SortMode", "UseAmrRecoveryDivert", false);`
- Save（~:287）：`Ini->WriteBool("SortMode", "UseAmrRecoveryDivert", bUseAmrRecoveryDivert);`

### C3. `aAuto1To6.h` / `aAuto1To6.cpp` — `FindTrayRequestAuto` 加 kind 過濾
- 宣告（h:154 附近）：`int FindTrayRequestAuto(int &OutKind, int WantKind=eTrayReqNone);`（BCB6/C++98 支援預設參數；既有 3 個呼叫端不動、行為不變）
- 實作（cpp:1241）：predictive 迴圈與 by-index 迴圈的判斷都改為
  `if(Req!=eTrayReqNone && (WantKind==eTrayReqNone || Req==WantKind))`

### C4. `aTrayArm.cpp` — `TryDivertCarriedTrayToAuto`（:1025）
```cpp
// 舊
if(GeneralSetting.bUseAMR)
    return false;
if(iDeliverKind==eTrayKindIdentity)
    return false;
...
int idx=AutoModule->FindTrayRequestAuto(kind);
// 新
if(GeneralSetting.bUseAMR && GeneralSetting.bUseAmrRecoveryDivert==false)
    return false;
if(iDeliverKind!=eTrayKindNormal)   // Identity->Color, AMR Cover->Empty (both unchanged)
    return false;
...
int idx=AutoModule->FindTrayRequestAuto(kind, eTrayKindNormal);
```
既有 `kind!=eTrayKindNormal` 檢查保留當保險。函式頭註解（:1014-1024）同步改寫（ASCII English）。
（非 AMR 模式 `GetFedTrayKind` 強制 Normal，`iDeliverKind!=eTrayKindNormal` 等價於原本的 Identity 檢查 → Normal 模式行為不變。）

### C5. `aTrayArm.cpp` — `DecidePlaceDestAfterPick`（:982）
```cpp
// 舊
if(GeneralSetting.bUseAMR==false && AutoModule!=NULL)
// 新
bool bMaySupplyAuto = (GeneralSetting.bUseAMR==false) ||
                      (GeneralSetting.bUseAmrRecoveryDivert && iDeliverKind==eTrayKindNormal);
if(bMaySupplyAuto && AutoModule!=NULL)
```
內部 `FindTrayRequestAuto(kind, eTrayKindNormal)` 帶過濾；`idx>=0 && kind==eTrayKindNormal`（:992）保留。過時註解（:961-963「it always recycles」）改寫。
走 Auto 分支時不呼叫 `RequestReturnTray()` → Empty 站不做多餘 GoUp（既有結構天然如此）。

### C6. `aTrayArm.cpp` — `DoPlace` case 4000 通知分支（:935）★正確性關鍵
```cpp
// 舊
if(Job==TAJOB_AMR_SUPPLY)
    AutoModule->NotifyTrayArmDelivered(iAutoTarget, iDeliverKind, iDeliverTrayID);
else
    AutoModule->SetRearHasTrayFromTrayArm(iAutoTarget, true);
// 新
if(GeneralSetting.bUseAMR)
    AutoModule->NotifyTrayArmDelivered(iAutoTarget, iDeliverKind, iDeliverTrayID);
else
    AutoModule->SetRearHasTrayFromTrayArm(iAutoTarget, true);
```
理由：AMR 模式下送達 Auto 的只有 `TAJOB_AMR_SUPPLY` 與（本功能新增的）diverted `TAJOB_LOADER_RECOVERY`，兩者都必須 stamp RearKind/RearTrayID；對既有 AMR_SUPPLY 行為恆等，對 Normal 模式恆等。RearTrayID 部分：Normal 回收盤的 `GetRearTrayID()` 為空字串（aLoader.cpp:91/:893 重設；只有 identity 有 2D），與既有 AMR normal 供應傳入值相同。
（此分支同時覆蓋 HOME 續行的 adopt fast-forward 路徑 aTrayArm.cpp:845 → case 4000。）

### C7. `cStateRecordHT160.cpp` — FeederDecision `[Config gates]`（:796 後）
`Out += "  bUseAmrRecoveryDivert=" + IntToStr(GeneralSetting.bUseAmrRecoveryDivert ? 1 : 0) + "\r\n";`

### C8. maintenance UI（仿 predictive 全套）
- `maintenance.dfm`：`tsFunctionGeneral` 內、`pnlPredictiveSupplyBox`（:1082-1121）之後複製一組 `pnlAmrDivertBox`（Align=alTop、Top 排在 predictive 後）＋ `chkUseAmrRecoveryDivert` ＋ 提示 `lblAmrDivertHint`（caption 純 ASCII，例：`'When ON (AMR), a recovered Normal tray goes straight to a requesting Auto instead of parking at Empty first.'`）；後續 `pnlUphSampleBox` 等的 Top/TabOrder 順移。
- `maintenance.h`：`__published` 段補元件與 `chkUseAmrRecoveryDivertClick` 宣告（form class body 內**不得加註解**）。
- `maintenance.cpp`：Load（:1121 旁）、Save（:1201 旁）、OnClick handler（複製 :2028-2036 模式，含 `bLoadingHardwareSettings` 守衛）。旗標即時讀取，無需 restart 警告；不納入 `ApplyHardwareEditLock`（mid-lot 切換安全，與 predictive 同判）。

### C9.（驗證輔助，建議）divert 成功記錄一行
在 `TryDivertCarriedTrayToAuto` 成功處與 `DecidePlaceDestAfterPick` 選 Auto 處，沿用 TrayArm 現成 log 慣例寫一行（模式、目標 Auto、kind），供模擬與上機確認 divert 有無觸發。低頻事件（每次回收至多一行），不做③的完整量測。

## 5. 風險與防護

| 風險 | 防護 |
|------|------|
| RearKind 汙染 → 空盤直通 discharge、車帳錯 | C6 為必改項；驗證項 V3 專測 WorkingKind |
| `FindTrayRequestAuto` 簽名變更 | 預設參數 `eTrayReqNone`＝不過濾，3 個既有呼叫端零改動、行為不變 |
| Big5+LF 原始檔被 Edit 工具毀損 | 全部 C/C++/DFM 編輯用 byte-safe splice（PowerShell），新註解 ASCII English only |
| DFM 手編壞表單 | `check-bcb-form-published.ps1` linter＋編譯把關 |
| bUseAMR 運轉中切換 | HardwareInstall 群組受編輯鎖；即使切換，C6 以當下旗標選通知函式，無機構危險 |
| CleanOut／AMR 換車（bAmrLocked）誤供 | `GetTrayRequest` 既有拒絕條件把關，divert 不新增旁路 |
| HOME 帶盤續行 | 續行重放 `DecidePlaceDestAfterPick`（aTrayArm.cpp:148）與 adopt fast-forward 都收斂到 case 4000，kind stamp 正確 |

## 6. 驗證計畫

編譯閘（動到共用核心 → 雙組態必跑）：
1. 刪除受改 `.obj` → `scripts/ops/build-ht160s.ps1 -Clean` EXIT=0
2. 註解 `SOFT_SIMULATE`（MachineType.h）→ `-Full` EXIT=0 → 還原 define → 再 `-Full` EXIT=0
3. `scripts/ops/check-ht160s-source-encoding.ps1` PASS；`check-bcb-form-published.ps1` PASS

模擬驗證（SOFT_SIMULATE）：
- V1 AMR ON＋旗標 ON：某 Auto 車數 n≥2、工作盤耗盡、rear 空 → Loader rear 出回收盤 → TrayArm 直接送該 Auto（無 Empty 停靠、無 RequestReturnTray）；C9 log 佐證
- V2 旗標 OFF：回到現況（先回 Empty、需求出現再夾回）；General.ini 無鍵時預設 OFF
- V3 直供後 `WorkingKind==Normal`、SortArm 可放 IC（打 C6 風險）
- V4 車在 identity/cover 階段（n<2）：回收 Normal 盤照舊回 Empty；回收 Identity 照舊去 Color；kind 過濾使「另一台要 Normal 的 Auto」仍可被選中
- V5 CleanOut 中不 divert；Normal 模式全流程回歸不變
- V6 FeederDecision.txt `[Config gates]` 出現新旗標

上機驗證（使用者執行）：
- AMR 實車：堆疊序 identity→cover→normal 正確、車帳 tray count／host 報表（Report6/SECS）無異常
- divert 發生時 Empty 站無多餘 GoUp；中途 HOME（帶盤）續行後放盤與 kind 正確
- 旗標 OFF 快速回退確認

實作後依慣例做攻防複驗（adversarial review），驗證通過才 scoped commit＋push。

## 7. 明確不做（本計畫範圍外）

- Cover 盤直供（維持回收）
- ②「即將有需求」hold／預測性等待
- ③ park-then-repick 完整量測記錄
- `DecideJob` 的 `TAJOB_AMR_SUPPLY` 排程邏輯（不動）

## 8. 待使用者裁決

- **D1 旗標預設值**：建議 OFF（上機驗過再開）。要一步到位預設 ON 也可，請指示。
- **D2 maintenance UI 勾選框**：建議做（仿 predictive）。不要 UI 就只留 INI 鍵。
- **D3 divert 成功 log 一行（C9）**：建議做。
- **D4 `FindTrayRequestAuto` kind 過濾（C3）**：建議做；若你要最小侵入版（不改簽名），divert 在「第一需求者要 identity/cover」時就不觸發，效益打折。
