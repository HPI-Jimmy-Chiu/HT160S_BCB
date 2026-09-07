# HT160S_BCB — Claude Code Entry Point

This file is the session-loaded entry point. The full, authoritative rules live in
`.github/` (shared with GitHub Copilot). Read those when working on a subsystem.

## Authoritative sources (read as needed)
- `.github/copilot-instructions.md` — master project guidelines
- `.github/instructions/ht160s-development.instructions.md` — boundary, no-FSM, encoding, compile gate
- `.github/skills/ht160s-*/SKILL.md` — domain skills (sortarm, loader, topccd, secsgem, 2dbin-map, state-record, config-tiers, simulation, motion-view, elec-to-iotable, mechanism-profile, bindisplay, motion-card, panasonic-a6-servo, installer)
- `.github/agents/` — ht160s-maintainer, ht172-0420-reference-analyst
- `.claude/skills/weekly-case-flow/SKILL.md` — weekly report + customer-case flow (Hub mode). HT160S progress + KYEC(京元竹南) anomaly cases go into the SHARED store `D:\Work-jimmychiu\document\WeeklyReport\Weekly_AI` (used by HT9045/HT172 too). Driven by `.claude/agents/weekly-report.md` + `.claude/commands/weekly-*.md`. NOTE: release-note/close_case tools there are HT9045-specific; HT160S uses ht160s-installer/NSIS instead.
- Write-boundary hook (ACTIVE): registered in `.claude/settings.json` -> `scripts/ops/check-ht160s-writeboundary.ps1`.
  Whitelist: `D:\HT160S_BCB` + `D:\AI_Area\Tool\HT160S_SECS_Simulator` (the shared SECS test tool) plus the Claude state dir and temp are writable; every other tree (HT172, HT160S, HT160S -Original, HT160S_StateRecord, the rest of `D:\AI_Area\Tool`, ...) is denied at the tool layer. Extra roots are passed to the hook via `-AllowedRoots` in `.claude/settings.json` and are ADDED on top of the base roots, not substituted.
  `.github/hooks/*.json` are reference templates only — Claude Code loads hooks from settings files, not from there. The old `pretool-ht172-readonly.json` was never wired and is superseded by this hook.

## Hard rules (summary — defer to `.github/` for detail)

**Write boundary** (now hook-enforced — see write-boundary hook above)
- Writable: `D:\HT160S_BCB` and `D:\AI_Area\Tool\HT160S_SECS_Simulator` (shared SECS test tool). `D:\HT172` and all other reference roots are READ-ONLY. Reads are unrestricted.
- Read HT172 only for comparison/porting. If a task seems to require editing HT172, stop and redirect into HT160S_BCB.
- Do not copy HT172 runtime data (`system/`, `data/`, `EXE/`, logs) into HT160.
- Open the repo via `HT160S_BCB.code-workspace` (HT160S_BCB writable + HT172 read-only). Do NOT use `D:\HT-Handler.code-workspace` — it points "New" at `D:\HT160S`, a different repo, not this one.

**Project**
- BCB6 (C++Builder 6) project: `HT160S_Program_BCB_V1.0.0.0/ht160s.bpr`
- HT172 reference root: `D:\HT172\HT172_Program_V1.0.25.0_20260420\`

**No FSM in HT160**
- No `FSMRunner`, `FSM_GOTO`, transition tables, `*Step.h`, `*Table.cpp`, `*Exec.cpp`, or `FSM/` module.
- Port HT172 FSM behavior by rewriting into HT160 procedural / VCL-event / `switch(Task)` style.

**Encoding** (do NOT blanket "all Chinese in Big5")
- Legacy `.cpp/.h/.dfm/.rc`: preserve existing Big5/CP950; no UTF-8 conversion, no BOM.
- New comments in BCB6 source: ASCII English only (avoid CP950/UTF-8 mojibake).
- Docs / reports / governance (`docs/`, `.github/`, memory): UTF-8.

**Coding style**
- No C++11+ (`auto`, `nullptr`, lambdas, range-for, `enum class`); keep `AnsiString` flows.
- Prefer existing HT160 naming, globals, forms, task patterns. Search HT160 before adding new code.
- No blocking loops / `Sleep()` in machine-control paths unless an existing HT160 pattern requires it.

**Build gate**
- After every C++/DFM/project edit: delete the changed `.obj`, then compile.
- Wiring changes → full build. Preferred: `scripts/ops/build-ht160s.ps1 -Clean`.
- Also verify the real-machine build: comment out `#define SOFT_SIMULATE` in `MachineType.h`, run `-Full`, confirm exit 0, then RESTORE the active define and rebuild (dev keeps it on, so `#ifndef SOFT_SIMULATE` branches never compile and rot silently). Required when touching `SOFT_SIMULATE`-guarded or shared core code. See `.github/instructions/ht160s-development.instructions.md` Compile Gate.
- Encoding check: `scripts/ops/check-ht160s-source-encoding.ps1` (fails on `EF BF BD` and UTF-8 BOM).
- Never claim build-clean if BCB6 tools could not run.
