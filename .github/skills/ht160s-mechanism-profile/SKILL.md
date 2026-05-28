---
name: ht160s-mechanism-profile
description: "Use when mapping HT160S_BCB mechanisms before porting HT172 behavior: motors, IO, sensors, switches, suckers, forms, runtime flags, and module ownership."
---

# HT160S Mechanism Profile Skill

## Purpose

Build or update the HT160 mechanism map before adapting HT172 behavior into HT160S_BCB.

## Scope

- Target project: `D:\HT160S_BCB\HT160S_Program_BCB_V1.0.0.0/`
- Reference project: `D:\HT172\HT172_Program_V1.0.25.0_20260420/` read-only

## Required Output

For the requested mechanism or feature, produce a compact profile:

| Item | Required detail |
| --- | --- |
| Module owner | Main form, module class, helper file, or global owner |
| Motors | HT160 motor names and HT172 equivalent names if known |
| IO | Cylinders, sensors, switches, suckers, and missing mappings |
| Data | INI, CSV, runtime variables, tray maps, counters, or flags |
| UI | Forms, buttons, timers, grids, and event handlers |
| Flow | Existing HT160 control path and likely insertion point |
| Risk | Motion, IO timing, production flow, operator action, and data safety |

## Rules

- Search HT160 first.
- Read HT172 only for comparison.
- Do not write to HT172.
- Do not copy HT172 runtime data.
- Do not import unfamiliar architecture from HT172.
- If a HT172 hardware name has no HT160 equivalent, stop and ask for mapping confirmation before implementation.

## Workflow

1. Identify the requested HT160 mechanism or feature.
2. Search HT160 source, forms, data-loading code, and project file.
3. Search HT172 0420 only for the behavior being compared.
4. Build a name and dependency map.
5. Mark uncertain mappings clearly.
6. Hand the profile to the HT160S Maintainer for implementation.
