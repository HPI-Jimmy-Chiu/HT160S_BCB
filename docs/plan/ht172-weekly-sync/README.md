# HT172 週報套件（ht172-weekly-sync）

apply-ready 套件：把 HT172 版的 **weekly-case-flow** 落位到 `D:\HT172\.claude\`，
讓 HT172 與 HT9045 / HT160S 一樣接入共用週報系統
`D:\Work-jimmychiu\document\WeeklyReport\Weekly_AI`。

## 為什麼在這裡
HT172 在 HT160S_BCB 專案是**唯讀參考**（write-boundary hook + CLAUDE.md 硬規則），
從 HT160S session 無法寫入 `D:\HT172\`。因此把 HT172 版檔案先備妥於此（可寫），
再由**你**用 `apply-ht172-weekly.ps1` 落位（見 [APPLY.md](APPLY.md)）。

## 內容
```
claude/
  agents/weekly-report.md                 # 子代理（Hub 模式，機型=HT172、多客戶）
  commands/weekly-help.md
  commands/update-weekly.md
  commands/weekly-status.md
  commands/weekly-case-intake.md
  commands/weekly-case-integrity.md
  commands/weekly-next-week.md
  skills/weekly-case-flow/SKILL.md         # HT172 端 SOP
apply-ht172-weekly.ps1                     # 一鍵套用（-DryRun 可預覽）
APPLY.md                                   # 套用說明
README.md                                  # 本檔
```

## 與 HT160S / HT9045 版的差異
| 面向 | HT9045（原版） | HT160S（已落位） | HT172（本套件） |
|---|---|---|---|
| 機型 | HT9045 | HT160S | HT172 |
| 主客戶 | 多客戶 | 京元竹南 (KYEC) | 多客戶（力成/力成PTI 為主） |
| 程式碼慣例 | Big5 + //AI 註解 | no-FSM + Big5 | **Big5 + FSM**（FSM 參考專案） |
| 寫入邊界 | 無 | 需把 Weekly_AI 加入 `-AllowedRoots` | **無需**（HT172 只有 guard-big5，非路徑邊界） |
| release note | `make_release_note.py`（HT9011UC glob） | ht160s-installer / NSIS | **D:\HT172_Updater_NSIS** |
| close_case | 用 | 不用 | **不用**（HT9045 專用會失敗） |

## 三機台共用真相檔
`weekly_data.json` 由 HT9045 / HT172 / HT160S 共用；只動自己機台相關的 row，
一律走 Weekly_AI 的 python 工具，勿手動 Edit 整檔。

來源計畫：[../weekly-caseflow-wire-into-ht160s-plan-20260715.md](../weekly-caseflow-wire-into-ht160s-plan-20260715.md)
