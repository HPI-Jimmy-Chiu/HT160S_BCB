---
description: "顯示 HT160S 週報/案件管理（Hub 模式，共用 Weekly_AI 工作區）的指令地圖與自然語觸發總覽。"
---

請顯示週報管理系統在 Claude Code（Hub 模式，操作共用 Weekly_AI 工作區）的完整指令地圖：

## 子代理（Task 工具自動路由，或指名呼叫）
- `weekly-report` — 週報與客戶異常 case 管理（HT160S 端主客戶＝京元竹南/KYEC）
- 根因分析 / 改程式碼 → 交回 **HT160S 開發 session 本體**（不另設分析子代理）

## 斜線指令
| 指令 | 用途 |
|------|------|
| `/update-weekly` | 自然語更新 HT160S 工作進度 |
| `/weekly-case-intake` | 京元（或其他 HT160S 客戶）新回報/重啟異常，建 case 並回報 `01_intake` 放檔路徑 |
| `/weekly-status` | 查詢狀態/未完成/健康檢查 |
| `/weekly-case-integrity` | 驗證未完成項目與 Customer case 一致性 |
| `/weekly-next-week` | 建立下週週報（已完成轉黑、日期推進）— 破壞性，先確認 |
| `/weekly-help` | 顯示此地圖 |

## 自然語範例（會自動路由到 weekly-report）
| 說法 | 效果 |
|------|------|
| 「今天是新的一周」 | 建立下週週報（先確認再執行） |
| 「京元反應 HT160S 異常，如圖」 | 建/重啟 case，回報 `01_intake` 放檔路徑 |
| 「京元 HT160S Clean out 全空盤退出已驗證通過」 | 更新該事件進度 |
| 「這週有哪些未完成？」 | 列 open 項目 |
| 「幫我做健康檢查」 | 完整分析 |

## 資料位置（共用 Weekly_AI 工作區，絕對路徑）
- 根目錄：`d:\Work-jimmychiu\document\WeeklyReport\Weekly_AI`
- 唯一真相：`weekly_data.json`（HT9045/HT172/HT160S 共用）；Excel 產出：`output/`
- 客戶 case：`Customer/<客戶>/<CASE>/01_intake~04_release/`
- HT160S 現況：`京元竹南 / HT160S / 機台開發 (in-progress)`（row 會隨重排變動，以 `list_open.py` 查為準）
- 詳細 SOP：`weekly-case-flow` skill

## HT160S 專屬提醒
- 客戶名稱只用使用者原話（勿把 HT9045/HT172 客戶帶進來）。
- 結案**不用** `close_case.py`（內含 HT9045 專用 release-note 會失敗）；改 `update_report.py` 設 done + 重產 Excel。
- release note / 安裝包走 `ht160s-installer` skill / NSIS updater。

## 狀態圖標
✅ done｜🔧 in-progress｜⏳ waiting｜📩 pending-response｜🆕 new
