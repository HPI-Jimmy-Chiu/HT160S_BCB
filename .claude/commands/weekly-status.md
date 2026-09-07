---
description: "列出週報未完成事項、健康檢查、客戶/機台統計分析（共用 Weekly_AI）。"
argument-hint: "例如：未完成事項、健康檢查、京元竹南 的進度"
---

> 交給 **weekly-report** 子代理執行（Task 工具）。純查詢，可直接跑，不需確認。

根據使用者需求，從共用 Weekly_AI 的 `weekly_data.json` 查詢並呈現週報狀態。

使用者輸入：$ARGUMENTS

未完成 / open 類查詢，工具優先：
```
cd /d/Work-jimmychiu/document/WeeklyReport/Weekly_AI
python tools/list_open.py
python tools/check_case_integrity.py
```

以 `(row, case_id, path, customer)` 為比對 key（不可只用 case_id，跨客戶會同號）。
- `[FAIL]` → 回報為工具/資料一致性問題，不可靠猜。
- `[WARN] weekly open item has no case` → 該未完成項目尚未歸檔。

支援：「未完成事項」「本週活躍」「健康檢查」「京元竹南的進度」「HT160S 問題」。
HT160S 相關查詢可加 `python tools/case_registry.py --customer 京元竹南 --print`。
其餘清單可用 `python tools/update_report.py list [active|open|waiting|new|all]` 或 `health`。
