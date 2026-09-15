# -*- coding: utf-8 -*-
"""
gen-op-guide-flowcharts.py - render the OP-guide flowcharts (docs/op-guide/img/*.svg).

Pure Python, no third-party modules. Each chart is a small grid layout:
nodes sit on (col, row) cells, edges are orthogonal polylines with arrowheads.
Re-run after editing a chart:  python scripts/ops/gen-op-guide-flowcharts.py
The script prints a WARN line for any node whose text is wider than its box.
"""
import os
import io

OUT_DIR = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..", "docs", "op-guide", "img")
FONT = "Microsoft JhengHei, PMingLiU, Noto Sans TC, sans-serif"

# kind -> (fill, stroke, text colour, tag shown top-left)
STYLE = {
    "start":    ("#E5E7EB", "#4B5563", "#111827", ""),
    "end":      ("#E5E7EB", "#4B5563", "#111827", ""),
    "step":     ("#DBEAFE", "#1D4ED8", "#0F172A", "機台"),
    "op":       ("#DCFCE7", "#15803D", "#052E16", "OP"),
    "decision": ("#FEF9C3", "#A16207", "#1C1917", ""),
    "alarm":    ("#FEE2E2", "#B91C1C", "#450A0A", "畫面警報"),
    "auto":     ("#EDE9FE", "#6D28D9", "#2E1065", "主機/AMR"),
    "note":     ("#F8FAFC", "#94A3B8", "#334155", ""),
}

LEGEND = [
    ("step", "機台自動動作"),
    ("op", "操作員 (OP) 要做的事"),
    ("decision", "判斷 / 分岐"),
    ("alarm", "畫面彈出的警報或訊息"),
    ("auto", "主機 (EAP) / AMR 車自動"),
]


def _esc(s):
    return (s.replace("&", "&amp;").replace("<", "&lt;").replace(">", "&gt;"))


def _text_w(s, fs):
    w = 0.0
    for ch in s:
        w += fs * (1.0 if ord(ch) > 0x2E7F else 0.56)
    return w


class Flow(object):
    def __init__(self, title, col_w=250, row_h=118, node_w=210, margin=28,
                 legend=True, lanes=None, font_size=15, top_pad=None):
        self.title = title
        self.col_w = col_w
        self.row_h = row_h
        self.node_w = node_w
        self.margin = margin
        self.legend = legend
        self.lanes = lanes or []      # list of (col_from, col_to, label)
        self.fs = font_size
        self.nodes = {}
        self.order = []
        self.edges = []
        self.top_pad = top_pad if top_pad is not None else (76 if lanes else 56)

    # ---- model -------------------------------------------------------------
    def node(self, nid, kind, col, row, text, w=None, h=None):
        self.nodes[nid] = dict(id=nid, kind=kind, col=col, row=row, text=text, w=w, h=h)
        self.order.append(nid)
        return nid

    def edge(self, src, dst, label=None, route=None, lane_col=None, dash=False, color=None):
        """route: None(auto) | 'right' | 'left' (exit decision side) | 'loop-right' | 'loop-left' | 'h' (side to side, Z-shaped when rows differ)"""
        self.edges.append(dict(src=src, dst=dst, label=label, route=route,
                               lane_col=lane_col, dash=dash, color=color))

    # ---- geometry ----------------------------------------------------------
    def _cx(self, col):
        return self.margin + col * self.col_w + self.col_w / 2.0

    def _cy(self, row):
        return self.top_pad + row * self.row_h + self.row_h / 2.0

    def _size(self, n):
        lines = n["text"].split("\n")
        fs = self.fs
        if n["kind"] == "decision":
            tw = max(_text_w(l, fs) for l in lines)
            w = n["w"] or max(170, tw + 70)
            h = n["h"] or max(84, len(lines) * (fs + 6) + 44)
            return w, h
        w = n["w"] or self.node_w
        h = n["h"] or max(54, len(lines) * (fs + 6) + 22 + (12 if STYLE[n["kind"]][3] else 0))
        return w, h

    def _box(self, n):
        w, h = self._size(n)
        cx, cy = self._cx(n["col"]), self._cy(n["row"])
        return cx - w / 2.0, cy - h / 2.0, w, h, cx, cy

    # ---- render ------------------------------------------------------------
    def _render_node(self, out, n):
        fill, stroke, tcol, tag = STYLE[n["kind"]]
        x, y, w, h, cx, cy = self._box(n)
        fs = self.fs
        lines = n["text"].split("\n")
        widest = max(_text_w(l, fs) for l in lines)
        limit = (w - 40) if n["kind"] == "decision" else (w - 14)
        if widest > limit:
            print("WARN %s/%s text too wide (%.0f > %.0f): %s" % (self.title[:6], n["id"], widest, limit, lines[0]))
        if n["kind"] == "decision":
            pts = "%.1f,%.1f %.1f,%.1f %.1f,%.1f %.1f,%.1f" % (cx, y, x + w, cy, cx, y + h, x, cy)
            out.write('<polygon points="%s" fill="%s" stroke="%s" stroke-width="2"/>\n' % (pts, fill, stroke))
        elif n["kind"] in ("start", "end"):
            out.write('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="%.1f" fill="%s" stroke="%s" stroke-width="2"/>\n'
                      % (x, y, w, h, h / 2.0, fill, stroke))
        else:
            dash = ' stroke-dasharray="6,4"' if n["kind"] == "note" else ""
            out.write('<rect x="%.1f" y="%.1f" width="%.1f" height="%.1f" rx="8" fill="%s" stroke="%s" stroke-width="2"%s/>\n'
                      % (x, y, w, h, fill, stroke, dash))
        tag_h = 0
        if tag and n["kind"] not in ("decision",):
            tag_h = 12
            out.write('<text x="%.1f" y="%.1f" font-family="%s" font-size="11" font-weight="bold" fill="%s">%s</text>\n'
                      % (x + 8, y + 14, FONT, stroke, _esc(tag)))
        total_h = len(lines) * (fs + 6)
        y0 = cy - total_h / 2.0 + fs + tag_h / 2.0 - 2
        bold = ' font-weight="bold"' if n["kind"] in ("start", "end") else ""
        out.write('<text x="%.1f" y="%.1f" font-family="%s" font-size="%d" fill="%s" text-anchor="middle"%s>' % (cx, y0, FONT, fs, tcol, bold))
        for i, l in enumerate(lines):
            dy = 0 if i == 0 else fs + 6
            out.write('<tspan x="%.1f" dy="%d">%s</tspan>' % (cx, dy, _esc(l)))
        out.write('</text>\n')

    def _anchor(self, n, side):
        x, y, w, h, cx, cy = self._box(n)
        if side == "top":
            return cx, y
        if side == "bottom":
            return cx, y + h
        if side == "left":
            return x, cy
        return x + w, cy

    def _render_edge(self, out, e):
        s, d = self.nodes[e["src"]], self.nodes[e["dst"]]
        route = e["route"]
        col = e["color"] or "#374151"
        pts = []
        label_at = None
        anchor = "start"
        if route in ("loop-right", "loop-left"):
            side = "right" if route == "loop-right" else "left"
            sx, sy = self._anchor(s, side)
            dx, dy = self._anchor(d, side)
            lane_x = self._cx(e["lane_col"]) if e["lane_col"] is not None else (
                max(sx, dx) + 46 if side == "right" else min(sx, dx) - 46)
            pts = [(sx, sy), (lane_x, sy), (lane_x, dy), (dx, dy)]
            label_at = (lane_x, (sy + dy) / 2.0)
            anchor = "middle"
        elif route == "h" or (d["row"] == s["row"] and route is None):
            if d["col"] > s["col"]:
                sx, sy = self._anchor(s, "right")
                dx, dy = self._anchor(d, "left")
            else:
                sx, sy = self._anchor(s, "left")
                dx, dy = self._anchor(d, "right")
            if abs(dy - sy) < 1:
                pts = [(sx, sy), (dx, dy)]
                label_at = ((sx + dx) / 2.0, sy - 8)
            else:
                mx = (sx + dx) / 2.0
                pts = [(sx, sy), (mx, sy), (mx, dy), (dx, dy)]
                label_at = (mx, (sy + dy) / 2.0 + 4)
            anchor = "middle"
        elif route in ("right", "left"):
            sx, sy = self._anchor(s, route)
            dx, dy = self._anchor(d, "top")
            if abs(dx - sx) < 1:
                pts = [(sx, sy), (dx, dy)]
            elif d["row"] == s["row"]:
                dx, dy = self._anchor(d, "left" if route == "right" else "right")
                pts = [(sx, sy), (dx, dy)]
            else:
                pts = [(sx, sy), (dx, sy), (dx, dy)]
            label_at = ((sx + dx) / 2.0, sy - 8)
            anchor = "middle"
        else:
            sx, sy = self._anchor(s, "bottom")
            dx, dy = self._anchor(d, "top")
            if abs(dx - sx) < 1:
                pts = [(sx, sy), (dx, dy)]
                label_at = (sx + 8, (sy + dy) / 2.0 + 4)
            else:
                my = sy + max(14, (dy - sy) / 2.0)
                pts = [(sx, sy), (sx, my), (dx, my), (dx, dy)]
                label_at = (sx + 8, sy + 16)
        dash = ' stroke-dasharray="7,5"' if e["dash"] else ""
        path = " ".join(("M" if i == 0 else "L") + "%.1f,%.1f" % p for i, p in enumerate(pts))
        out.write('<path d="%s" fill="none" stroke="%s" stroke-width="2" marker-end="url(#arrow)"%s/>\n' % (path, col, dash))
        if e["label"]:
            lx, ly = label_at
            tw = _text_w(e["label"], 12) + 10
            if anchor == "middle":
                lx_box = lx - tw / 2.0
            else:
                lx_box = lx - 4
            out.write('<rect x="%.1f" y="%.1f" width="%.1f" height="17" rx="3" fill="#FFFFFF" fill-opacity="0.92"/>\n' % (lx_box, ly - 13, tw))
            out.write('<text x="%.1f" y="%.1f" font-family="%s" font-size="12" fill="%s" text-anchor="%s">%s</text>\n'
                      % (lx, ly, FONT, col, anchor, _esc(e["label"])))

    def render(self, filename):
        max_col = max(n["col"] for n in self.nodes.values())
        max_row = max(n["row"] for n in self.nodes.values())
        extra_r = 0
        extra_l = 0
        for e in self.edges:
            if e["route"] == "loop-right":
                extra_r = max(extra_r, 70)
            if e["route"] == "loop-left":
                extra_l = max(extra_l, 70)
        width = self.margin * 2 + (max_col + 1) * self.col_w + extra_r + extra_l
        legend_h = 34 if self.legend else 0
        height = self.top_pad + (max_row + 1) * self.row_h + legend_h + 10
        out = io.StringIO()
        out.write('<svg xmlns="http://www.w3.org/2000/svg" width="%d" height="%d" viewBox="0 0 %d %d" font-family="%s">\n'
                  % (width, height, width, height, FONT))
        out.write('<defs><marker id="arrow" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="8" markerHeight="8" orient="auto-start-reverse">'
                  '<path d="M0,0 L10,5 L0,10 z" fill="#374151"/></marker></defs>\n')
        out.write('<rect x="0" y="0" width="%d" height="%d" fill="#FFFFFF"/>\n' % (width, height))
        out.write('<text x="%d" y="28" font-family="%s" font-size="19" font-weight="bold" fill="#111827">%s</text>\n'
                  % (self.margin, FONT, _esc(self.title)))
        out.write('<g transform="translate(%d,0)">\n' % extra_l)
        # swim lanes
        for i, (c0, c1, label) in enumerate(self.lanes):
            lx = self.margin + c0 * self.col_w
            lw = (c1 - c0 + 1) * self.col_w
            shade = "#F9FAFB" if i % 2 == 0 else "#F3F4F6"
            out.write('<rect x="%.1f" y="%d" width="%.1f" height="%d" fill="%s"/>\n'
                      % (lx, 42, lw, height - 42 - legend_h - 6, shade))
            out.write('<rect x="%.1f" y="42" width="%.1f" height="28" fill="#1F2937"/>\n' % (lx, lw))
            out.write('<text x="%.1f" y="62" font-family="%s" font-size="15" font-weight="bold" fill="#FFFFFF" text-anchor="middle">%s</text>\n'
                      % (lx + lw / 2.0, FONT, _esc(label)))
            if i > 0:
                out.write('<line x1="%.1f" y1="42" x2="%.1f" y2="%d" stroke="#D1D5DB" stroke-width="1"/>\n'
                          % (lx, lx, height - legend_h - 6))
        for e in self.edges:
            self._render_edge(out, e)
        for nid in self.order:
            self._render_node(out, self.nodes[nid])
        out.write('</g>\n')
        if self.legend:
            x = self.margin
            y = height - legend_h + 4
            for kind, label in LEGEND:
                fill, stroke, _, _ = STYLE[kind]
                if kind == "decision":
                    out.write('<polygon points="%.1f,%.1f %.1f,%.1f %.1f,%.1f %.1f,%.1f" fill="%s" stroke="%s" stroke-width="1.5"/>\n'
                              % (x + 11, y, x + 22, y + 9, x + 11, y + 18, x, y + 9, fill, stroke))
                else:
                    out.write('<rect x="%d" y="%d" width="22" height="18" rx="3" fill="%s" stroke="%s" stroke-width="1.5"/>\n'
                              % (x, y, fill, stroke))
                out.write('<text x="%d" y="%d" font-family="%s" font-size="12" fill="#374151">%s</text>\n'
                          % (x + 28, y + 14, FONT, _esc(label)))
                x += 28 + _text_w(label, 12) + 26
        out.write('</svg>\n')
        path = os.path.join(OUT_DIR, filename)
        with io.open(path, "w", encoding="utf-8") as f:
            f.write(out.getvalue())
        print("wrote", os.path.relpath(path))


# =============================================================================
# 1. Non-AMR : one-lot overview
# =============================================================================
def chart_nonamr_overview():
    f = Flow("圖 1  非 AMR 模式：一批料的作業總覽", col_w=280, row_h=104, node_w=252)
    f.node("s", "start", 1, 0, "上班 / 開電")
    f.node("a", "op", 1, 1, "確認畫面：AMR=OFF、Real、\nRecipe 正確、User=Operation")
    f.node("b", "op", 1, 2, "Lot 資料就緒\n(主機自動下達，或手動建 Lot)")
    f.node("c", "op", 1, 3, "放料：來料盤 → Loader\n空盤 → Empty\nAuto1~6 出料車空車就位")
    f.node("d", "op", 1, 4, "Home → Yes\n等「Home finished.」")
    f.node("e", "op", 1, 5, "Monitor → Start")
    f.node("run", "step", 1, 6, "RUNNING（綠燈）\n掃描 → 分類 → 堆疊")
    f.node("ev", "alarm", 2, 6, "運轉中警報：\n補來料 / 補空盤 / 收滿盤\n處理後 START 續跑（見圖 3）")
    f.node("dry", "alarm", 1, 7, "來料用完：\nLoader Tray Empty")
    f.node("q", "decision", 1, 8, "本批還有料？")
    f.node("more", "op", 0, 8, "補來料盤\n→ RETRY → START")
    f.node("co", "op", 1, 9, "選 CLEAN OUT → START\n（機台清機，見圖 4）")
    f.node("fin", "alarm", 1, 10, "清機完成提示\n→ 選 SKIP → PAUSE")
    f.node("col", "op", 1, 11, "取走 Auto1~6 出料車的盤\n核對 Total / 各站 Cnt")
    f.node("le", "op", 1, 12, "Lot 分頁 → Lot End")
    f.node("nx", "decision", 1, 13, "還有下一批？")
    f.node("off", "end", 1, 14, "Exit → 關電 / 下班")
    for a, b in [("s", "a"), ("a", "b"), ("b", "c"), ("c", "d"), ("d", "e"), ("e", "run"),
                 ("run", "dry"), ("dry", "q"), ("co", "fin"), ("fin", "col"), ("col", "le"), ("le", "nx")]:
        f.edge(a, b)
    f.edge("run", "ev", label="有警報時", route="h", dash=True)
    f.edge("q", "more", label="是", route="left")
    f.edge("more", "run", route="loop-left", label="續跑")
    f.edge("q", "co", label="否，全部進完")
    f.edge("nx", "off", label="否")
    f.edge("nx", "b", label="是，下一批", route="loop-right")
    f.render("flow-nonamr-overview.svg")


# =============================================================================
# 2. Non-AMR : power-on to Start
# =============================================================================
def chart_nonamr_startup():
    f = Flow("圖 2  非 AMR 模式：開機到開始生產", col_w=295, row_h=112, node_w=258)
    f.node("s", "start", 1, 0, "機台上電")
    f.node("p", "step", 1, 1, "程式自動啟動\n狀態文字 INIT → HALT\n馬達自動上電")
    f.node("b", "op", 1, 2, "看右側三個徽章\nSECS / SAFE / AMR")
    f.node("qamr", "decision", 1, 3, "AMR 徽章 = OFF？")
    f.node("wrong", "note", 2, 3, "顯示 ON = AMR 模式\n請改看 AMR 模式文件\n並通知工程師確認")
    f.node("qreal", "decision", 1, 4, "Real/Dummy 顯示 Real？")
    f.node("eng", "op", 2, 4, "顯示 Dummy / HasTray\n→ 通知工程師切回 Real\n(OP 權限無法自行切換)")
    f.node("user", "op", 1, 5, "User = Operation\nRecipe Name = 本批配方")
    f.node("qlot", "decision", 1, 6, "Lot 分頁清單已有\n本批 Lot 且 2D 欄 > 0？")
    f.node("man", "op", 2, 6, "手動建 Lot：\n輸入 Lot No. → Add Lot →\nLot Start → Update WebAPI")
    f.node("man2", "op", 2, 7, "等按鈕由「Updating ...」\n變回「Update WebAPI」\n雙擊 Lot 列確認 2D 明細")
    f.node("mat", "op", 1, 7, "放料：來料盤 → Loader 進料堆疊\n空盤 → Empty 堆疊\nAuto1~6 出料車空車就位")
    f.node("home", "op", 1, 8, "Monitor → Home\n「Confirm home?」→ Yes")
    f.node("hw", "step", 1, 9, "各軸原點燈轉綠\n「Home finished.」自動關閉")
    f.node("st", "op", 1, 10, "Start（畫面或面板）")
    f.node("gate", "decision", 1, 11, "機台檢查通過？")
    f.node("rej", "alarm", 2, 11, "被擋下的訊息：\nPlease Enter LotID ! /\nNo Lot data / No 2D data /\nOperation level must run\nin Real mode")
    f.node("run", "end", 1, 12, "RUNNING 綠燈，開始生產")
    for a, b in [("s", "p"), ("p", "b"), ("b", "qamr"), ("qreal", "user"), ("user", "qlot"),
                 ("mat", "home"), ("home", "hw"), ("hw", "st"), ("st", "gate")]:
        f.edge(a, b)
    f.edge("qamr", "qreal", label="是")
    f.edge("qamr", "wrong", label="否", route="right", dash=True)
    f.edge("qreal", "eng", label="否", route="right")
    f.edge("eng", "user", label="切回 Real 後")
    f.edge("qlot", "mat", label="是")
    f.edge("qlot", "man", label="否", route="right")
    f.edge("man", "man2")
    f.edge("man2", "mat", label="有資料", route="h")
    f.edge("gate", "run", label="是")
    f.edge("gate", "rej", label="否", route="right")
    f.edge("rej", "user", route="loop-right", label="依訊息補齊後再 Start")
    f.render("flow-nonamr-startup.svg")


# =============================================================================
# 3. Non-AMR : running - the three manual material events
# =============================================================================
def chart_nonamr_running():
    f = Flow("圖 3  非 AMR 模式：運轉中 OP 要處理的三種補料 / 收料", col_w=320, row_h=118, node_w=284)
    f.node("run", "step", 1, 0, "RUNNING\n（機台不會叫車，補料收料全靠 OP）")
    f.node("a1", "alarm", 0, 1, "MES0920\nLoader Tray Empty\n（來料盤用完）")
    f.node("a2", "alarm", 1, 1, "MES1022\nEmpty supply magazine empty\n（空盤用完）")
    f.node("a3", "alarm", 2, 1, "MES1120 ~ MES1620\nAutoN output stack FULL (sensor)\n- remove finished trays | Lot=…")
    f.node("q1", "decision", 0, 2, "本批還有\n來料盤？")
    f.node("b1", "op", 0, 3, "補來料盤到進料堆疊\n→ 選 RETRY → START\n→ 續跑")
    f.node("c1", "op", 0, 4, "本批料已全部進完：\n選 CLEAN OUT → START\n→ 進入清機（見圖 4）")
    f.node("b2", "op", 1, 3, "補空盤到 Empty 堆疊\n→ 選 RETRY → START\n→ 續跑")
    f.node("b3", "op", 2, 3, "把該 Auto 車上的滿盤全部搬走\n（滿盤感測器要熄滅）\n→ 選 RETRY → START → 續跑")
    f.node("n3", "note", 2, 4, "沒搬乾淨 → 同一警報會再彈\n搬完再按 RETRY")
    f.edge("run", "a1")
    f.edge("run", "a2")
    f.edge("run", "a3")
    f.edge("a1", "q1")
    f.edge("q1", "b1", label="是")
    f.edge("q1", "c1", label="否，全部進完", route="loop-left")
    f.edge("a2", "b2")
    f.edge("a3", "b3")
    f.edge("b3", "n3", dash=True)
    f.render("flow-nonamr-running.svg")


# =============================================================================
# 4. Non-AMR : batch end (Clean Out + Lot End)
# =============================================================================
def chart_nonamr_lotend():
    f = Flow("圖 4  非 AMR 模式：批尾清機與結批", col_w=280, row_h=112, node_w=250)
    f.node("s", "start", 1, 0, "本批來料全部進完")
    f.node("q", "decision", 1, 1, "怎麼進入清機？")
    f.node("w1", "op", 0, 2, "Loader Tray Empty 警報\n→ 選 CLEAN OUT → START")
    f.node("w2", "op", 2, 2, "Monitor → Clean Out\n「Confirm Clean Out?」→ Yes")
    f.node("co", "step", 1, 3, "狀態文字 Clean Out（黃）\n機內剩餘 IC 全部放完\nAuto 作業位的盤堆入出料車")
    f.node("wait", "note", 2, 3, "清機中若彈警報：\n照圖 5 排除後 START 續跑")
    f.node("fin", "alarm", 1, 4, "清機完成提示（只有 SKIP 鍵）")
    f.node("skip", "op", 1, 5, "選 SKIP → 按 PAUSE\n機台回 Normal 並停機 (HALT)")
    f.node("col", "op", 1, 6, "取走 Auto1~6 出料車上的盤\n交給下一站")
    f.node("chk", "op", 1, 7, "核對主畫面 Total 與\nUnload Auto1~6 各站 Cnt")
    f.node("le", "op", 1, 8, "Lot 分頁 → Lot End\n（Lot 清單清空 = 結批完成）")
    f.node("n", "note", 2, 8, "非 AMR 模式清機完成\n不會自動結批，一定要按\nLot End（或由主機結批）")
    f.node("e", "end", 1, 9, "本批結束，可建下一批")
    f.edge("s", "q")
    f.edge("q", "w1", label="警報時選", route="left")
    f.edge("q", "w2", label="直接按鈕", route="right")
    f.edge("w1", "co")
    f.edge("w2", "co")
    f.edge("co", "wait", route="h", dash=True)
    f.edge("co", "fin")
    f.edge("fin", "skip")
    f.edge("skip", "col")
    f.edge("col", "chk")
    f.edge("chk", "le")
    f.edge("le", "n", route="h", dash=True)
    f.edge("le", "e")
    f.render("flow-nonamr-lotend.svg")


# =============================================================================
# 5. Shared : alarm window handling
# =============================================================================
def chart_alarm():
    f = Flow("圖 5  警報視窗（Note）處理通則 — 兩種模式共用", col_w=285, row_h=112, node_w=252)
    f.node("s", "alarm", 1, 0, "蜂鳴 + 紅燈 + 彈出 Note 視窗\n機台已自動停止")
    f.node("r", "op", 1, 1, "讀 Err Code 與 Message\n看示意圖哪個機構在紅色閃爍")
    f.node("mute", "op", 2, 1, "太吵可先按 Off Buzzer\n（只消音，視窗不會關）")
    f.node("fix", "op", 1, 2, "到現場排除：\n補料 / 搬走滿盤 / 移除卡料 /\n關好安全門 / 釋放急停")
    f.node("q", "decision", 1, 3, "排除得了？")
    f.node("key", "op", 1, 4, "選一個回復鍵：\nRETRY（重試）/ SKIP（跳過）\nTRAY END / CLEAN OUT\nHOME & RETRY")
    f.node("go", "decision", 1, 5, "要繼續生產？")
    f.node("start", "op", 0, 6, "按 START\n機台續跑")
    f.node("pause", "op", 2, 6, "按 PAUSE\n機台停機，稍後再 Start")
    f.node("no", "op", 2, 3, "排除不了：\n1. 按 Store Hangup 存快照\n2. 記下時間 + Err Code\n3. 通知工程師")
    f.node("n", "note", 0, 4, "沒選回復鍵時\nSTART / PAUSE 都不會關閉視窗\n（防止警報被帶過）")
    f.node("e", "end", 1, 7, "視窗關閉，蜂鳴停止")
    f.edge("s", "r")
    f.edge("r", "mute", route="h", dash=True)
    f.edge("r", "fix")
    f.edge("fix", "q")
    f.edge("q", "key", label="是")
    f.edge("q", "no", label="否", route="right")
    f.edge("key", "n", route="h", dash=True)
    f.edge("key", "go")
    f.edge("go", "start", label="是", route="left")
    f.edge("go", "pause", label="否", route="right")
    f.edge("start", "e")
    f.edge("pause", "e")
    f.render("flow-alarm-handling.svg")


# =============================================================================
# 6. AMR : overview of one lot, OP vs automatic
# =============================================================================
def chart_amr_overview():
    f = Flow("圖 6  AMR 模式：一批料的流程 — 哪些自動、哪些要 OP 動手",
             col_w=300, row_h=112, node_w=252,
             lanes=[(0, 0, "OP 操作員"), (1, 1, "HT160S 機台"), (2, 2, "主機 EAP / AMR 車")])
    f.node("s", "start", 0, 0, "上班 / 開電")
    f.node("chk", "op", 0, 1, "確認徽章：AMR=ON（綠）\nSECS=ONLINE（綠）、Real")
    f.node("lot", "auto", 2, 2, "主機下達 Lot 資訊\n2D 資料自動下載")
    f.node("lotm", "step", 1, 2, "Lot 分頁自動出現本批\n2D 欄 > 0")
    f.node("home", "op", 0, 3, "Home → Yes\n等「Home finished.」")
    f.node("st", "op", 0, 4, "Monitor → Start\n（主機不會幫你按 Start）")
    f.node("call", "step", 1, 5, "缺料 / 車滿 → 自動叫車\n(CEID 272)")
    f.node("amr", "auto", 2, 5, "AMR 送來料盤、空盤、身分盤\n收走滿的出料車")
    f.node("run", "step", 1, 6, "RUNNING\n掃描 → 分類 → 堆疊\n（身分盤 → 上蓋 → 工作盤）")
    f.node("watch", "op", 0, 6, "只看不動：\n塔燈、Unload 面板\nAMR 作業中不開門不搬料")
    f.node("alm", "alarm", 0, 7, "WAR0962 車沒來 /\nSECS 斷線 / 其他警報\n→ 見圖 8")
    f.node("dry", "step", 1, 8, "來料用完，等 60 秒沒新車\n→ 自動進入 Clean Out")
    f.node("drain", "step", 1, 9, "機內排空 → 六站出料車\n叫 AMR 收走")
    f.node("take", "auto", 2, 9, "AMR 收走六台出料車")
    f.node("le", "step", 1, 10, "自動 Lot End\n機台停機 (HALT)")
    f.node("nx", "op", 0, 11, "等主機送下一批 Lot\n→ 回到 Start")
    f.edge("s", "chk")
    f.edge("chk", "lotm", label="等主機", route="h", dash=True)
    f.edge("lot", "lotm", route="h")
    f.edge("lotm", "home", route="h", dash=True)
    f.edge("home", "st")
    f.edge("st", "call", route="h", dash=True)
    f.edge("call", "amr", route="h")
    f.edge("amr", "run", label="交接完成", route="h")
    f.edge("call", "run")
    f.edge("run", "watch", route="h", dash=True)
    f.edge("watch", "alm", dash=True)
    f.edge("run", "dry")
    f.edge("dry", "drain")
    f.edge("drain", "take", route="h")
    f.edge("take", "le", label="全部收走", route="h")
    f.edge("drain", "le")
    f.edge("le", "nx", route="h", dash=True)
    f.edge("nx", "st", route="loop-left", label="下一批")
    f.render("flow-amr-overview.svg")


# =============================================================================
# 7. AMR : one station handoff (what the OP sees, what not to touch)
# =============================================================================
def chart_amr_handoff():
    f = Flow("圖 7  AMR 模式：一個站台的交接（以 Auto 出料車滿為例）",
             col_w=300, row_h=104, node_w=252,
             lanes=[(0, 0, "HT160S 機台"), (1, 1, "主機 EAP / AMR 車"), (2, 2, "OP 操作員")])
    f.node("full", "step", 0, 0, "AutoN 出料車滿\n（滿盤感測器亮）")
    f.node("lock", "step", 0, 1, "鎖住該站：不再往這台車送盤\n發叫車事件 CEID 272")
    f.node("disp", "auto", 1, 1, "主機收到叫車\n派 AMR 前往 AutoN")
    f.node("cmd", "auto", 1, 2, "主機下 START_AGV\n（車已到站前）")
    f.node("ready", "step", 0, 3, "作業位的盤收完、氣缸歸位\n發「可交車」CEID 273")
    f.node("take", "auto", 1, 4, "AMR 取走整台滿車")
    f.node("fin", "step", 0, 5, "感測器熄滅 = 車已被取走\n發「完成」CEID 274\n解鎖、清車帳、生產恢復")
    f.node("op1", "op", 2, 1, "不用做任何事\n（其他站照常生產）")
    f.node("op2", "op", 2, 4, "AMR 在站前作業時：\n不要開門、不要碰車、\n不要手動搬盤")
    f.node("to", "alarm", 2, 2, "若 300 秒內車沒來：\nWAR0962 逾時警報\n→ 見圖 8")
    f.node("dsc", "alarm", 2, 3, "若 SECS 斷線：\n改由 OP 人工換車\n(MES1x20 警報 → 搬走 → RETRY)")
    f.edge("full", "lock")
    f.edge("lock", "disp", route="h")
    f.edge("disp", "cmd")
    f.edge("cmd", "ready", route="h")
    f.edge("ready", "take", route="h")
    f.edge("take", "fin", route="h")
    f.edge("lock", "op1", route="h", dash=True)
    f.edge("cmd", "to", route="h", dash=True)
    f.edge("ready", "dsc", label="斷線時", route="h", dash=True)
    f.edge("take", "op2", route="h", dash=True)
    f.render("flow-amr-handoff.svg")


# =============================================================================
# 8. AMR : exception decision tree
# =============================================================================
def chart_amr_alarms():
    f = Flow("圖 8  AMR 模式：警報與例外處理判斷", col_w=292, row_h=124, node_w=266, font_size=14)
    f.node("s", "alarm", 1, 0, "蜂鳴 + Note 視窗彈出")
    f.node("q", "decision", 1, 1, "Err Code 是哪一種？")
    f.node("w62", "alarm", 0, 2, "WAR0962\nAGV/AMR handshake timeout -\nAGV did not respond : AutoN (P#)")
    f.node("w63", "alarm", 1, 2, "WAR0963\nSECS link lost - AMR handoff\nstill held; clear the station\nthen RETRY")
    f.node("full", "alarm", 2, 2, "MES1120 ~ MES1620\nAutoN output stack FULL (sensor)\n（SECS 斷線期間，或\nError 流道設為人工下料）")
    f.node("oth", "alarm", 3, 2, "其他警報\n（安全門、急停、氣缸、吸嘴、\nCCD、2D not found …）")
    f.node("a62", "op", 0, 3, "1. 聯絡 AMR / EAP 控制室確認派車\n2. 車到、交接完成後\n   選 RETRY → START\n   （機台重新叫車）")
    f.node("a62b", "note", 0, 4, "AMR 長時間無法派車：\n經領班同意可人工搬走該站\n滿車（或補料），再 RETRY → START")
    f.node("a63", "op", 1, 3, "1. 確認 AMR 已離開該站\n2. 站台已清空 / 已補好\n3. 選 RETRY → START\n4. 通知 EAP 連線異常")
    f.node("afull", "op", 2, 3, "確認 AMR 不在站前\n→ 人工搬走滿盤（感測器熄滅）\n→ 選 RETRY → START")
    f.node("aoth", "op", 3, 3, "照圖 5 通則處理\n排除不了 → Store Hangup\n→ 通知工程師")
    f.node("secs", "decision", 1, 5, "SECS 徽章仍是 ONLINE？")
    f.node("off", "op", 0, 5, "OFF / CONNECT = 主機斷線\n機台退回人工模式（不叫車）\n→ 通知 EAP；期間滿車要人工搬")
    f.node("e", "end", 1, 6, "恢復 AMR 自動運轉")
    f.edge("s", "q")
    f.edge("q", "w62", route="left")
    f.edge("q", "w63")
    f.edge("q", "full", route="right")
    f.edge("q", "oth", route="right")
    f.edge("w62", "a62")
    f.edge("a62", "a62b", dash=True)
    f.edge("w63", "a63")
    f.edge("full", "afull")
    f.edge("oth", "aoth")
    f.edge("a63", "secs")
    f.edge("a62b", "secs", label="處理後", route="h")
    f.edge("afull", "secs", label="處理後", route="h")
    f.edge("secs", "e", label="是")
    f.edge("secs", "off", label="否", route="left")
    f.render("flow-amr-alarms.svg")


if __name__ == "__main__":
    if not os.path.isdir(OUT_DIR):
        os.makedirs(OUT_DIR)
    chart_nonamr_overview()
    chart_nonamr_startup()
    chart_nonamr_running()
    chart_nonamr_lotend()
    chart_alarm()
    chart_amr_overview()
    chart_amr_handoff()
    chart_amr_alarms()
