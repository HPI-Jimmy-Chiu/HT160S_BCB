---
description: "驗證共用 weekly_data.json 未完成項目與 Customer case 資料夾/case_id/issue.md 是否一致。"
argument-hint: "例如：列未完成後比對 case id 是否一致"
---

> 交給 **weekly-report** 子代理執行（Task 工具）。純查詢，可直接跑。

請用工具優先方式檢查共用 Weekly_AI 的 `weekly_data.json` 與 Customer case 資料是否一致。

使用者輸入：$ARGUMENTS

必跑命令：
```
cd /d/Work-jimmychiu/document/WeeklyReport/Weekly_AI
python tools/list_open.py
python tools/check_case_integrity.py
```

判讀規則：
1. 未完成清單以 `list_open.py` 為準。
2. 比對用 `(customer, case_id, path)`，不可只用 case_id。
3. `row` 會因排序變動，只當臨時定位。
4. CASE-ID 跨客戶同號：**舊格式** `CASE-YYYYMMDD-NNN` 跨客戶同號允許（NNN 每客戶獨立計數）；**新格式** `CASE-<EngCode>-YYYYMMDD-NNN`（ADR-007）自帶客戶代號、全域唯一。任一格式「同客戶同號」才是錯誤。
5. `[FAIL]` 是工具/資料問題，不可靠猜回答。
6. `[WARN] weekly open item has no case` 表示該未完成事項尚未歸檔。

回覆表格：

| Row | Case | Path | 客戶 | 機型/版本 | 狀態 | 比對結果 |
|---:|---|---|---|---|---|---|

比對結果只允許：`OK` / `WARN: no case` / `FAIL: ...`。最後補 `check_case_integrity.py exit=<N>`。
