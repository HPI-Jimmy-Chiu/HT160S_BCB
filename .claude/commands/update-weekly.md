---
description: "快速更新 HT160S 週報進度。輸入自然語言描述工作進展，更新共用 weekly_data.json 與 Excel。"
argument-hint: "例如：京元 HT160S Clean out 全空盤退出已驗證通過"
---

> 交給 **weekly-report** 子代理執行（Task 工具）。操作共用 Weekly_AI 工作區。

根據使用者輸入的自然語言，解析並更新週報。

使用者輸入：$ARGUMENTS

步驟：
1. 從輸入提取：客戶（HT160S 端主客戶＝京元竹南）、機台型號（HT160S）、事件關鍵字、行動描述、日期。
2. 判斷歸屬（Weekly_AI ADR-008，2026-09-15 起）：
   - **客訴 case 的進度** → 用 CASE-ID 或 title 片段在 `d:\Work-jimmychiu\document\WeeklyReport\Weekly_AI\weekly_data.json` 找該 case 的列（`python tools/list_open.py` / `python tools/case_registry.py --customer 京元竹南 --print`）；找不到對應 case 代表是新異常 → 改走 `/weekly-case-intake` 新開一列 + case，**不可塞進量產維護列**。
   - **非客訴開發 / 維護工作**（元件汰換、工具導入、技術債、版本定錨）→ 「京元竹南 / HT160S / HT160S 量產維護（非客訴開發紀錄）」列（`--search "HT160S 量產維護"`）。
   模糊就向我確認是哪一筆。**只動 HT160S 相關 row，勿誤改 HT9045/HT172 項目；row 會隨重排變動，勿記死號碼。**
3. 更新該 item：`python tools/update_report.py update --search "<CASE-ID|HT160S 量產維護>" --desc "<詳細>" --brief "<白話一句>" [--status ...]`（D 欄雙層制；`--desc` 避開結案關鍵字「結案/出貨/已完成/已解決…」否則工具拒絕）。**只走 Python 工具，不手改 JSON。**
4. **確認後**重產 Excel：
   ```
   cd /d/Work-jimmychiu/document/WeeklyReport/Weekly_AI/tools && python generate_report.py
   ```
5. 簡述改了什麼、Excel 路徑。
