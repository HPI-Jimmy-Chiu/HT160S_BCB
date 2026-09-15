# -*- coding: utf-8 -*-
"""
build-op-guide-pdf.py - convert docs/op-guide/*.md to PDF (md -> html -> headless Chrome).

Usage:  python scripts/ops/build-op-guide-pdf.py            (all *.md in docs/op-guide except README)
        python scripts/ops/build-op-guide-pdf.py <file.md>  (one file)
Needs:  pip module `markdown`; Google Chrome or Microsoft Edge installed.
"""
import io
import os
import sys
import glob
import subprocess
import markdown

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
DOC_DIR = os.path.join(ROOT, "docs", "op-guide")

BROWSERS = [
    r"C:\Program Files\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Google\Chrome\Application\chrome.exe",
    r"C:\Program Files (x86)\Microsoft\Edge\Application\msedge.exe",
    r"C:\Program Files\Microsoft\Edge\Application\msedge.exe",
]

CSS = u"""
@page { size: A4; margin: 14mm 13mm 16mm 13mm; }
body { font-family: "Microsoft JhengHei", "Noto Sans TC", "PMingLiU", sans-serif; font-size: 10.5pt;
       line-height: 1.55; color: #111827; margin: 0; }
h1 { font-size: 19pt; margin: 0 0 10px 0; padding-bottom: 6px; border-bottom: 3px solid #1D4ED8; }
h2 { font-size: 14.5pt; margin: 22px 0 8px 0; padding: 4px 8px; background: #DBEAFE; border-left: 6px solid #1D4ED8;
     page-break-after: avoid; }
h3 { font-size: 12pt; margin: 16px 0 6px 0; color: #1E3A8A; page-break-after: avoid; }
p { margin: 5px 0; }
table { border-collapse: collapse; width: 100%; margin: 6px 0 10px 0; font-size: 9.6pt; page-break-inside: auto; }
th, td { border: 1px solid #9CA3AF; padding: 4px 6px; vertical-align: top; text-align: left; }
th { background: #F3F4F6; }
tr { page-break-inside: avoid; }
code { font-family: Consolas, "Microsoft JhengHei", monospace; font-size: 9.5pt; background: #F3F4F6; padding: 0 3px; }
blockquote { margin: 8px 0; padding: 6px 10px; background: #FFFBEB; border-left: 5px solid #F59E0B; }
img { max-width: 100%; display: block; margin: 8px auto; page-break-inside: avoid; }
img[src$=".svg"] { max-height: 255mm; }
hr { border: 0; border-top: 1px solid #D1D5DB; margin: 14px 0; }
ul, ol { margin: 4px 0 6px 0; }
li { margin: 2px 0; }
strong { color: #991B1B; }
.footer { margin-top: 18px; font-size: 8.5pt; color: #6B7280; border-top: 1px solid #D1D5DB; padding-top: 4px; }
"""


def find_browser():
    for p in BROWSERS:
        if os.path.isfile(p):
            return p
    raise SystemExit("Chrome/Edge not found - install one or add its path to BROWSERS")


def build(md_path):
    with io.open(md_path, encoding="utf-8") as f:
        md_text = f.read()
    body = markdown.markdown(md_text, extensions=["tables", "fenced_code", "sane_lists"])
    title = os.path.splitext(os.path.basename(md_path))[0]
    html = (u'<!doctype html><html lang="zh-Hant"><head><meta charset="utf-8"><title>%s</title>'
            u'<style>%s</style></head><body>%s'
            u'<div class="footer">%s &nbsp;|&nbsp; 鴻勁精密 Hon Precision &nbsp;|&nbsp; 由 docs/op-guide/*.md 產生</div>'
            u'</body></html>') % (title, CSS, body, title)
    html_path = os.path.splitext(md_path)[0] + ".html"
    pdf_path = os.path.splitext(md_path)[0] + ".pdf"
    with io.open(html_path, "w", encoding="utf-8") as f:
        f.write(html)
    url = "file:///" + os.path.abspath(html_path).replace("\\", "/")
    cmd = [find_browser(), "--headless=new", "--disable-gpu", "--no-pdf-header-footer",
           "--virtual-time-budget=4000", "--print-to-pdf=" + os.path.abspath(pdf_path), url]
    subprocess.check_call(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    os.remove(html_path)
    print("wrote", os.path.relpath(pdf_path, ROOT), os.path.getsize(pdf_path), "bytes")


if __name__ == "__main__":
    targets = sys.argv[1:] or [p for p in glob.glob(os.path.join(DOC_DIR, "*.md"))
                               if os.path.basename(p).lower() != "readme.md"]
    for t in targets:
        build(t)
