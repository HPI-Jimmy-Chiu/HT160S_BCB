---
name: ht160s-config-tiers
description: >-
  Use when classifying or wiring any HT160S_BCB configuration / feature flag,
  or when deciding where a setting belongs among CosFunction (paid features),
  Config (customer self-toggle), General (ship + hardware install), and Recipe
  (per-lot process). Triggers: CosFunction, Config, config.ini, General.ini,
  GeneralSetting, machine_option, Recipe, RecipeManager, BinAreaMap, tFunction,
  CUSTOMER_CODE, paid feature, customer toggle, hardware install, ship parameter.
---

# HT160S Configuration / Function Tier Architecture

HT160S separates settings into **four tiers**. Each tier has a different owner,
a different change method, and a different storage location. Putting a setting in
the wrong tier is the root cause of the legacy "CosFunction catch-all drawer"
problem. Always classify a new flag against this table **before** writing code.

## 1. The Four Tiers

| Tier | Nature | Decided by | How it changes | Storage |
|------|--------|-----------|----------------|---------|
| **CosFunction** | Paid customer-bound features. Unlocked after the customer pays; shipping a new install package is required to enable. | Factory (HarnessEng) | **Compile-time** `switch(CUSTOMER_CODE)` in `CosFunction.cpp`. Rebuild + new install package. | Hard-coded in `FUNC_CC_xxx()` functions. **No ini.** |
| **Config** | Features already delivered to the customer; the customer decides whether to turn them on. | Customer (on site) | **Runtime** UI checkbox / setting. No rebuild. | **`config\config.ini` ONLY.** |
| **General** | Ship parameters + hardware-install state (which optional hardware this machine physically has). | Factory / commissioning engineer | Ship setup, permission-protected (Maintenance form). | `system\General.ini` (per-machine, not copied between machines). |
| **Recipe** | Per-lot process parameters. | Customer per lot | Setup form, per recipe. | `data\<recipe name>\<files>`. |

## 2. Decision Tree

```
Is the setting a feature the customer PAID for, that we gate per customer?
  YES -> CosFunction (compile-time CUSTOMER_CODE switch, no ini)
  NO  v
Does it describe what optional HARDWARE this machine physically has,
or a ship/commissioning parameter set by the engineer?
  YES -> General (system\General.ini, Maintenance form, permission-protected)
  NO  v
Is it a feature already shipped that the CUSTOMER turns on/off themselves?
  YES -> Config (config\config.ini, runtime UI checkbox)
  NO  v
Does it change per production lot / recipe?
  YES -> Recipe (data\<recipe>\)
```

## 3. Tier rules (do / don't)

### CosFunction (paid features)
- MUST be compile-time. Default all flags first, then call `DoCustomerFunction()`
  which does `switch(CUSTOMER_CODE)` and calls `FUNC_CC_xxx()` to hard-set flags.
- MUST NOT read any runtime ini. If a flag is loaded from an ini at runtime it is
  **not** a CosFunction flag — it belongs to Config or General.
- `CUSTOMER_CODE` is defined in `cmydef.cpp` (= `HT160S_DEFAULT_CUSTOMER_CODE`,
  `MachineType.h`). Aligns with HT172 `CosFunction.cpp` `FUNC_CC_*` pattern.
- Struct: `HT160S_CUSTOMER_FUNCTION CosFunction;` (declared in `CosFunction.h`).

### Config (customer self-toggle)
- Storage is **strictly `config\config.ini`**. Never store Config flags in the
  recipe folder, `system\`, or `machine_option.ini`.
- Loaded once at startup and saved when the customer changes a UI checkbox.
- Backing data: `THT160Config Config;` in `Config.h/.cpp`. The legacy
  `TFunction tFunction;` (`cprod.h`) shell is the field set Config owns
  (UseCCD / RejectCCDfail / UseHitCylinder / HitRetry / UsePreAlignment).

### General (ship + hardware install)
- Storage is `system\General.ini`. Per-machine; must NOT be copied between
  machines (hardware install differs).
- Backing data: `THT160GeneralSetting GeneralSetting;` in `GeneralSetting.h/.cpp`.
- Owns hardware-install flags such as `bColorBinAreaInstalled`
  (does this machine physically have the Color bin-area hardware?).
- Edited only through the Maintenance (commissioning) form, permission-protected.

### Recipe (per-lot process)
- Layout: `data\<recipe name>\<files>` (e.g. `BinAreaMap.ini`).
- Managed by `THT160RecipeManager RecipeManager;` (recipe folder lifecycle) and
  `THT160BinAreaMap BinAreaMap;` (Bin->Area routing table per recipe).
- Recipe selection/edit belongs in the Setup form.

## 4. Migration map (legacy -> tier)

| Legacy location | Legacy meaning | Correct tier | New home |
|-----------------|----------------|--------------|----------|
| `CosFunction.bColorBinAreaInstalled` (from `machine_option.ini`) | Color bin-area hardware installed? | **General** | `GeneralSetting.bColorBinAreaInstalled` (`system\General.ini`) |
| `CosFunction.bUseBinAreaMap` (hard `=true`) | Use Bin->Area routing table | **CosFunction** | paid flag set by `CUSTOMER_CODE` switch (default true for current customers) |
| `LoadCosFunctionMachineOption` / `SaveCosFunctionMachineOption` / `machine_option.ini` | Runtime ini for "installed" flags | **General** | `GeneralSetting.Load()` / `Save()` |
| `TFunction tFunction` (`cprod.h`, never loaded) | CCD / hit-cylinder / pre-align toggles | **Config** | loaded/saved by `Config` from `config\config.ini` |
| `THT160RecipeManager` / `THT160BinAreaMap` | Recipe folder + Bin map | **Recipe** | `RecipeManager` / `BinAreaMap` (stay recipe-scoped; future: split into `RecipeManager.cpp`) |

## 5. Common mistakes

- Loading a "paid feature" flag from an ini at runtime — that makes it a Config
  or General flag, not CosFunction. Pick the right tier by **who decides** and
  **how it changes**, not by the variable name.
- Storing a Config flag anywhere other than `config\config.ini`.
- Copying `system\General.ini` between machines — hardware-install state is
  machine-specific.
- Treating "hardware installed?" as a paid feature. Installed-hardware presence
  is a General (commissioning) fact, even if the hardware itself was a paid option.

## 6. Code anchors

- `CosFunction.h/.cpp` — CosFunction struct + `DoCustomerFunction()` + recipe/bin
  infrastructure (recipe split pending).
- `GeneralSetting.h/.cpp` — `THT160GeneralSetting`, `system\General.ini`.
- `Config.h/.cpp` — `THT160Config`, `config\config.ini`, owns `tFunction` fields.
- `cprod.h` — legacy `TFunction tFunction` field shell (read at `aLoader.cpp` CCD guard).
- `MachineType.h` / `cmydef.cpp` — `CUSTOMER_CODE`.
- `database.cpp` — boot sequence calls `InitialCosFunction()`.
