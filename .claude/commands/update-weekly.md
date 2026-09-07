---
description: "快速更新 HT160S 週報進度。輸入自然語言描述工作進展，更新共用 weekly_data.json 與 Excel。"
argument-hint: "例如：京元 HT160S Clean out 全空盤退出已驗證通過"
---

> 交給 **weekly-report** 子代理執行（Task 工具）。操作共用 Weekly_AI 工作區。

根據使用者輸入的自然語言，解析並更新週報。

使用者輸入：$ARGUMENTS

步驟：
1. 從輸入提取：客戶（HT160S 端主客戶＝京元竹南）、機台型號（HT160S）、事件關鍵字、行動描述、日期。
2. 在 `d:\Work-jimmychiu\document\WeeklyReport\Weekly_AI\weekly_data.json` 搜尋匹配事件（HT160S 目前主要為 row 4「機台開發」）；模糊就向我確認是哪一筆。**只動 HT160S 相關 row，勿誤改 HT9045/HT172 項目。**
3. 更新該 item：append action、設 `is_active_this_week=true`、自動推斷 status。**走 Python 工具或最小 Edit，不重寫整檔。**
4. **確認後**重產 Excel：
   ```
   cd /d/Work-jimmychiu/document/WeeklyReport/Weekly_AI/tools && python generate_report.py
   ```
5. 簡述改了什麼、Excel 路徑。
