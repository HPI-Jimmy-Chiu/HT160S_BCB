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

## Compile Gate

- Every C++/DFM/project-file change requires BCB6 verification before completion.
- Delete changed `.obj` files before compiling.
- Use `scripts/ops/build-ht160s.ps1 -Clean` for full project verification when project wiring or shared core files changed.
- If a compile fails, fix the code and rerun the compile. Do not return broken objects as complete work.
