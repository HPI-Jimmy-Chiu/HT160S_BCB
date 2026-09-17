# -*- coding: utf-8 -*-
"""make-release-note-html.py -- render HT160S release-note .md into the
"鴻勁紅" branded .html (and optionally .pdf), byte-compatible with the HT9045
pipeline that customers already receive.

Why this exists
---------------
The shared Weekly_AI tool `tools/make_release_note.py` owns the brand template,
but it is hard-wired to HT9045 (it scans `d:\\HT9045\\HT9011UC_Code_V*` for
`//AI(...)` comments and refuses to run otherwise).  HT160S therefore writes its
release notes by hand as Markdown and uses THIS script only for the last mile:
Markdown -> branded HTML.  The CSS, the header block, the logo and the accent
rule below are copied verbatim from that tool so the two machines' customer
deliverables look identical.

Accent rule (same as HT9045 `make_release_note.py:562`):
    internal  -> blue  #1f4e79      (廠內版)
    customer  -> red   #c0392b      (客戶版 = 鴻勁紅)
    distributor -> red #c0392b      (代理商版)

Usage
-----
  # all of docs/release/*.md
  python scripts/ops/make-release-note-html.py

  # specific files
  python scripts/ops/make-release-note-html.py docs/release/HT160S_V1.0.0.4_customer_zh-TW.md

  # also produce PDF (needs Chrome or Edge, same as build-op-guide-pdf.py)
  python scripts/ops/make-release-note-html.py --pdf

  # override the header title parts
  python scripts/ops/make-release-note-html.py --customer 京元竹南 --version V1.0.0.4

Audience is taken from the filename (`_internal_` / `_distributor_` / `_customer_`);
`--accent red|blue` overrides it.
"""
import argparse
import base64
import glob
import io
import os
import re
import subprocess
import sys

import markdown

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
RELEASE_DIR = os.path.join(ROOT, "docs", "release")
LOGO_PATH = os.path.join(RELEASE_DIR, "assets", "honprec-logo.png")

BROWSERS = [
    r"C:\Program Files\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
    r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
]

# ---------------------------------------------------------------- brand CSS
# Verbatim from Weekly_AI/tools/make_release_note.py CSS_RED (lines 368-401),
# plus a @page/@media print block so the PDF keeps the same look.
CSS_RED = u"""
* { box-sizing: border-box; margin: 0; padding: 0; }
body { font-family: "微軟正黑體","Microsoft JhengHei","Calibri",sans-serif;
  background:#f4f6f9; padding:32px 24px; color:#2c2c2c; line-height:1.7; font-size:14px; }
.page { max-width: 900px; margin: 0 auto; }
header { display:flex; align-items:center; gap:18px; border-left:6px solid #c0392b;
  padding:14px 18px; margin-bottom:28px; background:#fff; border-radius:0 6px 6px 0;
  box-shadow:0 1px 4px rgba(0,0,0,.08); }
header img { height:56px; object-fit:contain; }
header h1 { font-size:20px; color:#c0392b; margin-bottom:4px; }
header p { font-size:12px; color:#888; }
h1 { font-size:22px; color:#c0392b; margin:24px 0 12px; }
h2 { font-size:15px; background:#c0392b; color:#fff; padding:6px 14px;
  border-radius:4px; margin:28px 0 12px 0; }
h3 { font-size:14px; color:#c0392b; margin:18px 0 8px; }
table { width:100%; border-collapse:collapse; background:#fff; margin-bottom:16px; }
th, td { border:1px solid #e0e0e0; padding:9px 14px; font-size:13px; vertical-align:top; }
th { background:#f4f4f4; font-weight:bold; color:#444; text-align:left; }
code { background:#f0f0f0; padding:1px 6px; border-radius:3px;
  font-family:Consolas,monospace; font-size:12px; }
pre { background:#f6f6f6; border:1px solid #ddd; border-left:4px solid #c0392b;
  padding:12px 16px; border-radius:4px; overflow-x:auto; font-family:Consolas,monospace; font-size:12px; }
ul, ol { padding-left:24px; margin-bottom:14px; }
li { margin-bottom:8px; line-height:1.7; }
hr { border:none; border-top:1px solid #ddd; margin:20px 0; }
blockquote { border-left:4px solid #c0392b; padding:10px 16px; background:#fff8f8;
  color:#555; margin:12px 0; }
footer { margin-top:36px; font-size:11px; color:#bbb; border-top:1px solid #ddd;
  padding-top:10px; text-align:center; }
@page { size: A4; margin: 12mm 10mm 14mm 10mm; }
@media print {
  body { background:#fff; padding:0; }
  header, table, pre, blockquote { box-shadow:none; }
  h2, h3 { page-break-after: avoid; }
  tr, pre, blockquote { page-break-inside: avoid; }
}
"""

# 廠內版用藍色（Steven 規範：廠內 = 藍）
CSS_BLUE = CSS_RED.replace(u"#c0392b", u"#1f4e79").replace(u"#fff8f8", u"#eef3fa")


def md_to_html(md_text, title, accent="red"):
    body = markdown.markdown(md_text, extensions=["tables", "fenced_code"])
    css = CSS_BLUE if accent == "blue" else CSS_RED

    logo_b64 = u""
    if os.path.exists(LOGO_PATH):
        with open(LOGO_PATH, "rb") as fh:
            logo_b64 = base64.b64encode(fh.read()).decode("ascii")

    if logo_b64:
        header_html = (
            u'\n<header>\n'
            u'  <img src="data:image/png;base64,%s" alt="鴻勁精密 Logo">\n'
            u'  <div>\n'
            u'    <h1>%s</h1>\n'
            u'    <p>鴻勁精密股份有限公司 HONPREC INC.</p>\n'
            u'  </div>\n'
            u'</header>\n' % (logo_b64, title)
        )
    else:
        header_html = u"<header><h1>%s</h1></header>" % title

    return (
        u'<!DOCTYPE html>\n'
        u'<html lang="zh-Hant">\n'
        u'<head>\n'
        u'<meta charset="UTF-8">\n'
        u'<title>%s</title>\n'
        u'<style>%s</style>\n'
        u'</head>\n'
        u'<body>\n'
        u'<div class="page">\n'
        u'%s\n'
        u'%s\n'
        u'</div>\n'
        u'</body>\n'
        u'</html>' % (title, css, header_html, body)
    )


def audience_of(path):
    name = os.path.basename(path)
    if "_internal_" in name:
        return "internal"
    if "_distributor_" in name:
        return "distributor"
    return "customer"


def version_of(path, override):
    if override:
        return override
    m = re.search(r"_(V\d+(?:\.\d+)*)_", os.path.basename(path))
    return m.group(1) if m else ""


def find_browser():
    for b in BROWSERS:
        if os.path.exists(b):
            return b
    return None


def to_pdf(html_path):
    browser = find_browser()
    if not browser:
        print("[pdf]   SKIP - no Chrome/Edge found; install one or open the .html and print to PDF")
        return None
    pdf_path = os.path.splitext(html_path)[0] + ".pdf"
    url = "file:///" + html_path.replace("\\", "/")
    cmd = [browser, "--headless", "--disable-gpu", "--no-pdf-header-footer",
           "--print-to-pdf=" + pdf_path, url]
    rc = subprocess.call(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    if rc != 0 or not os.path.exists(pdf_path):
        print("[pdf]   FAILED (%s) for %s" % (rc, html_path))
        return None
    print("[pdf]   %s" % os.path.relpath(pdf_path, ROOT))
    return pdf_path


def main():
    ap = argparse.ArgumentParser(
        description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("files", nargs="*",
                    help="release-note .md files (default: docs/release/*.md)")
    ap.add_argument("--customer", default="京元竹南", help="header title customer label")
    ap.add_argument("--version", default=None, help="override the version in the title")
    ap.add_argument("--accent", choices=["red", "blue"], default=None,
                    help="override the accent picked from the filename")
    ap.add_argument("--pdf", action="store_true", help="also render .pdf via headless Chrome/Edge")
    args = ap.parse_args()

    files = args.files or sorted(glob.glob(os.path.join(RELEASE_DIR, "*.md")))
    if not files:
        sys.exit("no .md found in %s" % RELEASE_DIR)

    if not os.path.exists(LOGO_PATH):
        print("[warn]  logo missing: %s -- header will render without it" % LOGO_PATH)

    for f in files:
        f = os.path.abspath(f)
        with io.open(f, encoding="utf-8") as fh:
            md_text = fh.read()

        aud = audience_of(f)
        accent = args.accent or ("blue" if aud == "internal" else "red")
        ver = version_of(f, args.version)
        title = "Release Note %s - %s" % (ver, args.customer) if ver \
            else "Release Note - %s" % args.customer

        html_text = md_to_html(md_text, title, accent=accent)
        html_path = os.path.splitext(f)[0] + ".html"
        with io.open(html_path, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(html_text)
        print("[html]  %s  (%s / %s)" % (os.path.relpath(html_path, ROOT), aud, accent))

        if args.pdf:
            to_pdf(html_path)


if __name__ == "__main__":
    main()
