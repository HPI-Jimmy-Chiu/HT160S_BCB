---
description: "Use when: editing HT160S_BCB C++Builder6 source, VCL forms, project files, governance assets, or porting features from HT172 0420. Keywords: HT160, HT160S, HT172, 0420, porting, BCB6, no FSM."
applyTo: ["HT160S_Program_BCB_V1.0.0.0/**/*.cpp", "HT160S_Program_BCB_V1.0.0.0/**/*.h", "HT160S_Program_BCB_V1.0.0.0/**/*.dfm", "HT160S_Program_BCB_V1.0.0.0/**/*.bpr", "HT160S_Program_BCB_V1.0.0.0/**/*.mak", ".github/**", "scripts/ops/**", "docs/**"]
---

# HT160S Development Instruction

## Target Boundary

- Main development target: `D:\HT160S_BCB`.
- Main source folder: `HT160S_Program_BCB_V1.0.0.0/`.
- HT172 is read-only reference. Never write to `D:\HT172`.

## No FSM Rule

HT160 must remain non-FSM.

Forbidden in HT160 work:

- `FSMRunner`
- `FSM_GOTO`
- `FSM/` folders
- transition table files
- `*Step.h`, `*Table.cpp`, `*Exec.cpp` architecture
- HT172 `fsm-refactor` skill or pattern adoption

If HT172 reference code is FSM-based, translate the behavior into HT160 legacy flow instead of porting the architecture.

## Porting Rule

Before implementing a migrated feature:

1. Search HT160 for an existing equivalent.
2. Read HT172 0420 only as a reference.
3. Identify data, IO, motor, sensor, and UI dependencies.
4. Implement only in HT160.
5. Keep the edit small and BCB6-compatible.

## Encoding And Compatibility

- Preserve Big5/CP950 encoding for legacy `.cpp`, `.h`, `.dfm`, `.rc` files.
- New or changed comments in BCB6 source files must be ASCII English. Do not add new Chinese comments to `.cpp`, `.h`, `.dfm`, or `.rc`; this avoids mojibake when tools read CP950 files as UTF-8.
- Before and after editing BCB6 source files, run `scripts/ops/check-ht160s-source-encoding.ps1` or the normal `scripts/ops/build-ht160s.ps1 -Clean`. The check fails on `EF BF BD` replacement bytes and UTF-8 BOMs.
- If a legacy source file appears garbled in VS Code/tool output but has no `EF BF BD`, treat it as CP950 display mismatch. Convert only touched comments to ASCII English; do not bulk-rewrite unrelated legacy comments.
- Do not use C++11 or newer syntax.
- Prefer existing HT160 naming, globals, forms, and task patterns.
- Do not copy HT172 runtime data files into HT160.

## VCL Form Headers (`__published` Layout Rules)

The BCB6 form designer parses the `.h` source when you click a component's event in the
Object Inspector. Its simplified parser raises a modal
`Error in module <unit>: Incorrect method declaration in class <TForm>` on TWO distinct
mistakes in `__published`. Either one is a whole-class parse failure: it breaks EVERY
event handler on that form, not just the component you clicked. A clean command-line
build does NOT catch either; only design-time clicking reproduces it (streaming uses
compiled RTTI, not source, so the form still runs).

**Rule 1 — no comments in `__published`.**
- Do NOT put comments inside the `__published` section of a VCL form/frame/data-module
  class (`class TfXxx : public TForm`). This includes standalone `//` lines among the
  members and trailing `// ...` comments on a member declaration line.

**Rule 2 — all fields before all handlers in `__published`.**
- Declare ALL component fields (`TXxx *name;`) first, THEN all event handlers
  (`void __fastcall ...Click(...)`). A component FIELD that appears AFTER a handler line
  trips the same parser; the first such field is reported as the incorrect declaration.
- This typically happens when tabs/pages are added incrementally as a field+handler pair
  per tab. When adding a new tab's controls, insert its fields into the field block above
  the handlers — do NOT append a field+handler pair per tab.
- Fixed 2026-06-16 in `maintenance.h` (TfMaintenance); `iosetview.h` was already correct
  and is the reference layout. Reordering is behavior-safe (DFM streaming and event
  binding resolve by name via RTTI, not by declaration order).
- Comments in `private:`/`public:` user-declaration sections are tolerated by the designer
  (e.g. `main.h` carries many and works), but prefer keeping notes out of the class body
  entirely. The IDE-written trailing comments on the section keywords themselves
  (`__published:\t// IDE-managed Components`) are fine.
- Put explanatory notes ABOVE the `class` line or BELOW the closing `};`. When you move a
  control from a runtime `Build*Page()` builder into the DFM, document the move there, not
  among the `__published` members.

## Compile Gate

- Every C++/DFM/project-file change requires BCB6 verification before completion.
- Delete changed `.obj` files before compiling.
- Use `scripts/ops/build-ht160s.ps1 -Clean` for full project verification when project wiring or shared core files changed.
- If a compile fails, fix the code and rerun the compile. Do not return broken objects as complete work.
- **Verify BOTH build configurations of `SOFT_SIMULATE`.** The dev tree keeps `#define SOFT_SIMULATE`
  active (`MachineType.h`), so the `#ifndef SOFT_SIMULATE` real-machine branches are NEVER compiled by a
  normal build and can rot silently (missing include, renamed symbol, stale signature). After the default
  (sim-on) build passes, also confirm the real-machine build: comment out `#define SOFT_SIMULATE` in
  `MachineType.h`, run `scripts/ops/build-ht160s.ps1 -Full` (a `#define` in a shared header forces a full
  recompile — `-Clean` is insufficient), confirm exit 0, then RESTORE the active `#define SOFT_SIMULATE`
  and rebuild so the tree is handed back in dev state. Required whenever a change touches code inside
  `#ifdef/#ifndef SOFT_SIMULATE` guards or shared core/header files; recommended for any non-trivial change.
- `build-installer.ps1` already toggles this define for the release package, but that path auto-reverts only
  on success — never leave `MachineType.h` with `SOFT_SIMULATE` commented out after a verification pass.
