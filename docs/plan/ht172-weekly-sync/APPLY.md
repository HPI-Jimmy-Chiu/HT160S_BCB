# 套用 HT172 週報套件

此套件把 HT172 版的 weekly-case-flow（子代理 + 6 個 weekly 指令 + skill）落位到
`D:\HT172\.claude\`，讓 HT172 也能像 HT9045 一樣把進度/客戶異常寫進共用 Weekly_AI。

> 為什麼是套件而非直接落位：HT172 在 HT160S_BCB 專案裡是**唯讀參考**（write-boundary），
> HT160S session 無法寫入 `D:\HT172\`。由**你**執行本腳本（或在 HT172 session 執行），
> 由你的動作把檔案寫進 HT172。

## 一鍵套用

先預覽（不寫檔）：
```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File "D:\HT160S_BCB\docs\plan\ht172-weekly-sync\apply-ht172-weekly.ps1" -DryRun
```
確認清單無誤後正式套用：
```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File "D:\HT160S_BCB\docs\plan\ht172-weekly-sync\apply-ht172-weekly.ps1"
```

## 會複製什麼（ADD-ONLY）
- `D:\HT172\.claude\agents\weekly-report.md`
- `D:\HT172\.claude\commands\` 6 個：`weekly-help.md`、`update-weekly.md`、`weekly-status.md`、`weekly-case-intake.md`、`weekly-case-integrity.md`、`weekly-next-week.md`
- `D:\HT172\.claude\skills\weekly-case-flow\SKILL.md`

**不會動** HT172 既有的 `settings.json` 與 `hooks\guard-big5.ps1`（HT172 無路徑寫入邊界，
weekly 流程走 python 不需白名單，故不需改 settings）。

## 套用後
1. **重啟 HT172 的 Claude Code session** —— agent / commands 是啟動時載入，套用後要重啟才會出現。
2. 打 `/weekly-help` 看指令地圖，或 `/weekly-status` 查 HT172 進度。
3. 用 `_customers.py "<客戶名>"` 確認客戶名解析後再建 case。

## HT172 專屬鐵律（已寫進 agent/skill）
- 結案**不用** `close_case.py`、不跑 `make_release_note.py`（HT9045 專用，掃 `d:\HT9045\HT9011UC_Code_V*` 會失敗）；
  結案改 `update_report.py` 設 done + `generate_report.py`；release/安裝包走 `D:\HT172_Updater_NSIS`。
- 改 code 遵守 HT172 的 **Big5 + FSM** 慣例（交回 HT172 開發 session 本體）。
- 絕不手動 Edit `weekly_data.json` 整檔；一律走 python 工具（且 Big5 guard 會擋 Edit 非 UTF-8 檔）。
- 客戶名只用使用者原話，勿把 HT9045/HT160S 客戶帶進來。
