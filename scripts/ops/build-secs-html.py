# -*- coding: utf-8 -*-
"""build_secs_html.py -- regenerate the two docs/SECS manuals' .html from their .md.

Recipe (memory secs-comm-examples-doc-maintenance, verified byte-exact 2026-07-29):
  * python-markdown, output_format="xhtml" (html5 would turn <hr /> into <hr> = fake diff)
  * the chrome (everything outside the markdown body) is taken VERBATIM from the existing
    .html: Comm_Examples -> prefix up to the first "<h1", suffix from "<footer>";
    Interface_Spec   -> prefix up to and including "<body>" plus the original separator,
                        suffix from "</body>".
  * Comm_Examples: ```mermaid fences become <pre class="mermaid"> with the content unescaped.

Usage:
  python build_secs_html.py --selftest [REV]   regenerate from `git show REV:md` and cmp with
                                               `git show REV:html` (default REV = af32607, the
                                               last commit that wrote both .html files)
  python build_secs_html.py --build            regenerate both .html from the working .md
"""
import html as htmlmod
import io
import os
import re
import subprocess
import sys

import markdown

REPO = r"D:\HT160S_BCB"
DOCS = [
    dict(key="comm",
         md="docs/SECS/HT160S_SECS_Comm_Examples.md",
         html="docs/SECS/HT160S_SECS_Comm_Examples.html",
         ext=["tables", "fenced_code", "sane_lists", "toc"],
         chrome="h1-footer", mermaid=True),
    dict(key="spec",
         md="docs/SECS/HT160S_SECS_Interface_Spec_20260727.md",
         html="docs/SECS/HT160S_SECS_Interface_Spec_20260727.html",
         ext=["tables", "fenced_code", "toc", "sane_lists", "attr_list"],
         chrome="body", mermaid=False),
]
MERMAID_RE = re.compile(r'<pre><code class="language-mermaid">(.*?)</code></pre>', re.S)


def git_show(rev, path):
    return subprocess.check_output(["git", "-C", REPO, "show", "%s:%s" % (rev, path)])


def render(md_text, ext, mermaid, mermaid_variant):
    body = markdown.markdown(md_text, extensions=ext, output_format="xhtml")
    if mermaid:
        def sub(m):
            content = htmlmod.unescape(m.group(1))
            if mermaid_variant == "nl":
                return '<pre class="mermaid">\n' + content + '</pre>'
            return '<pre class="mermaid">' + content + '</pre>'
        body = MERMAID_RE.sub(sub, body)
    return body


def split_chrome(old_html, mode):
    if mode == "h1-footer":
        i = old_html.index("<h1")
        j = old_html.index("<footer>")
        return old_html[:i], old_html[j:]
    i = old_html.index("<body>") + len("<body>")
    sep = ""
    while old_html[i + len(sep)] in "\r\n":
        sep += old_html[i + len(sep)]
    j = old_html.index("</body>")
    return old_html[:i + len(sep)], old_html[j:]


def assemble(prefix, body, suffix, join):
    return prefix + body + join + suffix


VARIANTS = [(mv, jv) for mv in ("nl", "none") for jv in ("\n", "")]


def try_variants(md_text, old_html, doc):
    prefix, suffix = split_chrome(old_html, doc["chrome"])
    for mv, jv in VARIANTS:
        out = assemble(prefix, render(md_text, doc["ext"], doc["mermaid"], mv), suffix, jv)
        if out == old_html:
            return (mv, jv), out
    return None, None


def selftest(rev):
    ok = True
    for doc in DOCS:
        md_text = git_show(rev, doc["md"]).decode("utf-8")
        old_html = git_show(rev, doc["html"]).decode("utf-8")
        variant, out = try_variants(md_text, old_html, doc)
        if variant is None:
            ok = False
            prefix, suffix = split_chrome(old_html, doc["chrome"])
            out = assemble(prefix, render(md_text, doc["ext"], doc["mermaid"], "nl"), suffix, "\n")
            # first differing offset for diagnosis
            k = next((i for i, (a, b) in enumerate(zip(out, old_html)) if a != b), min(len(out), len(old_html)))
            print("%-5s NO byte-exact variant. len new=%d old=%d first diff @%d: new=%r old=%r"
                  % (doc["key"], len(out), len(old_html), k, out[k:k + 60], old_html[k:k + 60]))
        else:
            print("%-5s EXACT MATCH at %s with variant mermaid=%s join=%r (%d bytes)"
                  % (doc["key"], rev, variant[0], variant[1], len(out.encode("utf-8"))))
            doc["variant"] = variant
    return ok


def build(rev):
    if not selftest(rev):
        raise SystemExit("selftest failed - refusing to build")
    for doc in DOCS:
        mv, jv = doc["variant"]
        md_path = os.path.join(REPO, doc["md"])
        html_path = os.path.join(REPO, doc["html"])
        md_text = io.open(md_path, encoding="utf-8", newline="").read()
        old_html = io.open(html_path, encoding="utf-8", newline="").read()
        prefix, suffix = split_chrome(old_html, doc["chrome"])
        out = assemble(prefix, render(md_text, doc["ext"], doc["mermaid"], mv), suffix, jv)
        if out == old_html:
            print("%-5s unchanged" % doc["key"])
            continue
        with io.open(html_path, "w", encoding="utf-8", newline="") as fh:
            fh.write(out)
        print("%-5s written: %d -> %d bytes" % (doc["key"], len(old_html.encode("utf-8")), len(out.encode("utf-8"))))


if __name__ == "__main__":
    rev = "af32607"
    if "--build" in sys.argv:
        build(rev)
    else:
        args = [a for a in sys.argv[1:] if not a.startswith("--")]
        if args:
            rev = args[0]
        sys.exit(0 if selftest(rev) else 1)
