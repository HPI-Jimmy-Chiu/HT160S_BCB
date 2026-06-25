# REVISED DESIGN — SortArm Z lightweight base-Z model

## datumConvention

**Decision: `slotOffset[slot]` is ADDED to per-station `baseZ`. Effective `Z(station,slot) = baseZ[station] + slotOffset[slot]`. All 4 slot offsets are FREE (no slot forced to 0).**

Rationale, grounded in the code:
- `GetSuckZMotor()` (aSortArm.cpp:230-237) maps `slot0→MSuckZ_1, slot1→MSuckZ_2, slot2→MSuckZ_3, slot3→MSuckZ_4` identically for **every** station. The registration block confirms this: each station's `_Z1Position` always binds `HSys.Mot.MSuckZ_1`, `_Z2Position` always `MSuckZ_2`, etc. (uteach.cpp:293-324). So slot index ≡ physical sucker, and a sucker's mechanical height delta relative to the carriage datum is genuinely station-independent. That is exactly what a shared `slotOffset[slot]` table models.
- This mirrors HT172's `baseZ + iPosZ_Offset[]` (HT172 aSortArm.cpp ~130-167), minus the auto-cal source for the offsets.

**Why all 4 free, not one datum-pinned to 0:** The HT172 reference does **not** force any sucker to 0 (it reads all of `edSortArm1ZA..ZH_Offset` independently). Pinning slot0≡0 would force the operator to express baseZ as "slot0's absolute Z" and re-derive the other three as deltas — a needless mental conversion. Keeping all 4 free lets the operator type the per-sucker correction directly and lets `baseZ[station]` be a clean nominal carriage Z. The migration step (below) seeds `slotOffset[slot]=0` and folds each station's per-slot variation back into nothing-lost values, so freedom costs nothing.

**Sign:** plain integer addition in 1/100 mm units (per memory: positions are 1/100mm). No negation, no scaling. Current values are negative (e.g. `-2945` = -29.45mm); `baseZ=-2945, slotOffset=0` reproduces them bit-for-bit.

---

## structChanges

File: `d:/HT160S_BCB/HT160S_Program_BCB_V1.0.0.0/uteach.h`

**OLD block — lines 101-133 (delete exactly):**
```cpp
    int SortArmToLoader_1_Z1Position;
    int SortArmToLoader_1_Z2Position;
    int SortArmToLoader_1_Z3Position;
    int SortArmToLoader_1_Z4Position;
    int SortArmToLoader_2_Z1Position;
    int SortArmToLoader_2_Z2Position;
    int SortArmToLoader_2_Z3Position;
    int SortArmToLoader_2_Z4Position;

    int SortArmToAuto_1_Z1Position;
    int SortArmToAuto_1_Z2Position;
    int SortArmToAuto_1_Z3Position;
    int SortArmToAuto_1_Z4Position;
    int SortArmToAuto_2_Z1Position;
    int SortArmToAuto_2_Z2Position;
    int SortArmToAuto_2_Z3Position;
    int SortArmToAuto_2_Z4Position;
    int SortArmToAuto_3_Z1Position;
    int SortArmToAuto_3_Z2Position;
    int SortArmToAuto_3_Z3Position;
    int SortArmToAuto_3_Z4Position;
    int SortArmToAuto_4_Z1Position;
    int SortArmToAuto_4_Z2Position;
    int SortArmToAuto_4_Z3Position;
    int SortArmToAuto_4_Z4Position;
    int SortArmToAuto_5_Z1Position;
    int SortArmToAuto_5_Z2Position;
    int SortArmToAuto_5_Z3Position;
    int SortArmToAuto_5_Z4Position;
    int SortArmToAuto_6_Z1Position;
    int SortArmToAuto_6_Z2Position;
    int SortArmToAuto_6_Z3Position;
    int SortArmToAuto_6_Z4Position;
```

**NEW block (replace with — 8 base Z + 4 shared slot offsets):**
```cpp
    // AI 20260624 : Lightweight base-Z model. 8 per-station base Z + 4 shared
    // per-sucker Z offsets (one table reused for ALL stations). Effective
    // Z(station,slot) = SortArm*_ZBasePosition + SortArmSuckZOffset_(slot+1).
    // slot0=MSuckZ_1 .. slot3=MSuckZ_4 (GetSuckZMotor, station-independent).
    int SortArmToLoader_1_ZBasePosition;
    int SortArmToLoader_2_ZBasePosition;
    int SortArmToAuto_1_ZBasePosition;
    int SortArmToAuto_2_ZBasePosition;
    int SortArmToAuto_3_ZBasePosition;
    int SortArmToAuto_4_ZBasePosition;
    int SortArmToAuto_5_ZBasePosition;
    int SortArmToAuto_6_ZBasePosition;

    int SortArmSuckZOffset_1;
    int SortArmSuckZOffset_2;
    int SortArmSuckZOffset_3;
    int SortArmSuckZOffset_4;
```

Lines 135-137 (`PitchArmXMinPositoin`, `PitchArmXMaxPositoin`, `BottomCCDYCapturePosition`) and `MAX_TEACH_ITEM 96` (line 18) are **unchanged** — item count drops from 96 to 76, well under the cap.

Note: named scalar fields (not a 4-element array) are used deliberately so the existing pointer-based registry (`int *iPara`) and the INI key built from `Caption` keep working without any array-indexing change in `AddTeachItem`/`GetTeachKey`.

---

## registrationChanges

File: `d:/HT160S_BCB/HT160S_Program_BCB_V1.0.0.0/uteach.cpp`

**OLD block — lines 293-324 (the 32 `AddTeachItem` Z registrations, delete exactly):**
```cpp
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_1_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToLoader_1_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_1_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToLoader_1_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_1_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToLoader_1_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_1_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToLoader_1_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_2_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToLoader_2_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_2_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToLoader_2_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_2_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToLoader_2_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_2_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToLoader_2_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_1_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_1_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_1_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToAuto_1_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_1_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToAuto_1_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_1_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToAuto_1_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_2_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_2_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_2_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToAuto_2_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_2_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToAuto_2_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_2_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToAuto_2_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_3_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_3_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_3_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToAuto_3_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_3_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToAuto_3_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_3_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToAuto_3_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_4_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_4_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_4_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToAuto_4_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_4_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToAuto_4_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_4_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToAuto_4_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_5_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_5_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_5_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToAuto_5_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_5_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToAuto_5_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_5_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToAuto_5_Z4Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_6_Z1Position", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_6_Z1Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_6_Z2Position", HSys.Mot.MSuckZ_2, &TeachBase.SortArmToAuto_6_Z2Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_6_Z3Position", HSys.Mot.MSuckZ_3, &TeachBase.SortArmToAuto_6_Z3Position);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_6_Z4Position", HSys.Mot.MSuckZ_4, &TeachBase.SortArmToAuto_6_Z4Position);
```

**NEW block (replace with — 8 base + 4 offset):**
```cpp
    // AI 20260624 : base-Z model. 8 per-station base Z (jog with any suck-Z motor;
    // pick MSuckZ_1 as the representative axis for the GoTeach/SetTeach helper) +
    // 4 shared per-sucker Z offsets bound to their own MSuckZ axis. Grid grdSortZ.
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_1_ZBasePosition", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToLoader_1_ZBasePosition);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToLoader_2_ZBasePosition", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToLoader_2_ZBasePosition);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_1_ZBasePosition", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_1_ZBasePosition);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_2_ZBasePosition", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_2_ZBasePosition);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_3_ZBasePosition", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_3_ZBasePosition);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_4_ZBasePosition", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_4_ZBasePosition);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_5_ZBasePosition", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_5_ZBasePosition);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmToAuto_6_ZBasePosition", HSys.Mot.MSuckZ_1, &TeachBase.SortArmToAuto_6_ZBasePosition);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmSuckZOffset_1", HSys.Mot.MSuckZ_1, &TeachBase.SortArmSuckZOffset_1);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmSuckZOffset_2", HSys.Mot.MSuckZ_2, &TeachBase.SortArmSuckZOffset_2);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmSuckZOffset_3", HSys.Mot.MSuckZ_3, &TeachBase.SortArmSuckZOffset_3);
    AddTeachItem(grdSortZ, "TeachLoader", "SortArmSuckZOffset_4", HSys.Mot.MSuckZ_4, &TeachBase.SortArmSuckZOffset_4);
```

This keeps the registry fully data-driven: `GetTeachKey()` (uteach.cpp:401-408) prepends `ed_` to the `Caption`, so save/load automatically use `ed_SortArmToLoader_1_ZBasePosition`, `ed_SortArmSuckZOffset_1`, etc., with **no change** to `OpenWorkFile`/`SaveWorkFile`/`AddTeachItem`/`GetTeachKey`. `GroupName` stays `"TeachLoader"` so the keys land in the same `[TeachLoader]` section the old keys used.

---

## formulaChanges

File: `d:/HT160S_BCB/HT160S_Program_BCB_V1.0.0.0/aSortArm.cpp`

**(A) `GetLoaderZPosition` — OLD lines 310-330 (replace exactly):**
```cpp
int TSortArmModule::GetLoaderZPosition(int LoaderNo, int SlotIndex)
{
    if(LoaderNo==2)
    {
        switch(SlotIndex)
        {
            case 0: return Teach.SortArmToLoader_2_Z1Position;
            case 1: return Teach.SortArmToLoader_2_Z2Position;
            case 2: return Teach.SortArmToLoader_2_Z3Position;
            case 3: return Teach.SortArmToLoader_2_Z4Position;
        }
    }
    switch(SlotIndex)
    {
        case 0: return Teach.SortArmToLoader_1_Z1Position;
        case 1: return Teach.SortArmToLoader_1_Z2Position;
        case 2: return Teach.SortArmToLoader_1_Z3Position;
        case 3: return Teach.SortArmToLoader_1_Z4Position;
    }
    return 0;
}
```

**NEW:**
```cpp
int TSortArmModule::GetLoaderZPosition(int LoaderNo, int SlotIndex)
{
    // AI 20260624 : effective Z = per-station base Z + shared per-sucker offset.
    int iBase=(LoaderNo==2)?Teach.SortArmToLoader_2_ZBasePosition
                           :Teach.SortArmToLoader_1_ZBasePosition;
    return iBase+GetSuckZOffset(SlotIndex);
}
```

**(B) `GetAutoZPosition` — OLD lines 360-420 (replace exactly):**
```cpp
int TSortArmModule::GetAutoZPosition(int AutoIndex, int SlotIndex)
{
    switch(AutoIndex)
    {
        case 0:
            switch(SlotIndex)
            {
                case 0: return Teach.SortArmToAuto_1_Z1Position;
                case 1: return Teach.SortArmToAuto_1_Z2Position;
                case 2: return Teach.SortArmToAuto_1_Z3Position;
                case 3: return Teach.SortArmToAuto_1_Z4Position;
            }
            break;
        case 1:
            switch(SlotIndex)
            {
                case 0: return Teach.SortArmToAuto_2_Z1Position;
                case 1: return Teach.SortArmToAuto_2_Z2Position;
                case 2: return Teach.SortArmToAuto_2_Z3Position;
                case 3: return Teach.SortArmToAuto_2_Z4Position;
            }
            break;
        case 2:
            switch(SlotIndex)
            {
                case 0: return Teach.SortArmToAuto_3_Z1Position;
                case 1: return Teach.SortArmToAuto_3_Z2Position;
                case 2: return Teach.SortArmToAuto_3_Z3Position;
                case 3: return Teach.SortArmToAuto_3_Z4Position;
            }
            break;
        case 3:
            switch(SlotIndex)
            {
                case 0: return Teach.SortArmToAuto_4_Z1Position;
                case 1: return Teach.SortArmToAuto_4_Z2Position;
                case 2: return Teach.SortArmToAuto_4_Z3Position;
                case 3: return Teach.SortArmToAuto_4_Z4Position;
            }
            break;
        case 4:
            switch(SlotIndex)
            {
                case 0: return Teach.SortArmToAuto_5_Z1Position;
                case 1: return Teach.SortArmToAuto_5_Z2Position;
                case 2: return Teach.SortArmToAuto_5_Z3Position;
                case 3: return Teach.SortArmToAuto_5_Z4Position;
            }
            break;
        case 5:
            switch(SlotIndex)
            {
                case 0: return Teach.SortArmToAuto_6_Z1Position;
                case 1: return Teach.SortArmToAuto_6_Z2Position;
                case 2: return Teach.SortArmToAuto_6_Z3Position;
                case 3: return Teach.SortArmToAuto_6_Z4Position;
            }
            break;
    }
    return 0;
}
```

**NEW:**
```cpp
int TSortArmModule::GetAutoZPosition(int AutoIndex, int SlotIndex)
{
    // AI 20260624 : effective Z = per-station base Z + shared per-sucker offset.
    int iBase;
    switch(AutoIndex)
    {
        case 0: iBase=Teach.SortArmToAuto_1_ZBasePosition; break;
        case 1: iBase=Teach.SortArmToAuto_2_ZBasePosition; break;
        case 2: iBase=Teach.SortArmToAuto_3_ZBasePosition; break;
        case 3: iBase=Teach.SortArmToAuto_4_ZBasePosition; break;
        case 4: iBase=Teach.SortArmToAuto_5_ZBasePosition; break;
        case 5: iBase=Teach.SortArmToAuto_6_ZBasePosition; break;
        default: return 0;
    }
    return iBase+GetSuckZOffset(SlotIndex);
}
```

**(C) New shared helper.** Insert immediately above `GetLoaderZPosition` (i.e. before line 310, after the `GetLoaderFirstSortY` separator at line 309). This is the single point that maps slot→shared offset, so both getters stay in lock-step:
```cpp
//---------------------------------------------------------------------------
int TSortArmModule::GetSuckZOffset(int SlotIndex)
{
    // AI 20260624 : shared per-sucker Z offset, same table for every station.
    switch(SlotIndex)
    {
        case 0: return Teach.SortArmSuckZOffset_1;
        case 1: return Teach.SortArmSuckZOffset_2;
        case 2: return Teach.SortArmSuckZOffset_3;
        case 3: return Teach.SortArmSuckZOffset_4;
    }
    return 0;
}
```

**Declaration:** add `int GetSuckZOffset(int SlotIndex);` to the `TSortArmModule` class declaration in `aSortArm.h`. **Confirmed location** (open question #1 resolved by the critic against live source): `aSortArm.h` already declares `GetLoaderZPosition` at line 62 and `GetAutoZPosition` at line 65 — add the one new line beside them.

The 3 call sites are unchanged: `MovePickZDown` (aSortArm.cpp:593-607, call at ~602) calls `GetLoaderZPosition(iActiveLoaderNo, SlotIndex)`, `MovePlaceZDown` (609-623, call at ~618) calls `GetAutoZPosition(iActiveAutoIndex, SlotIndex)`, and the Teach-jog at line 1466 calls both — all keep their signatures.

---

## iniChanges

This refactor touches **three** Caption-keyed INI files that all live under the same registry/persistence scheme. The original design covered the first two; the critic correctly identified the third (`OffsetLimit.ini`). All three are inventoried here.

### (1) `d:/HT160S_BCB/system/tech.ini` — base/teach values, section `[TeachLoader]`

**Removed keys (32, lines 28-59) — these simply become unread after refactor:**
`ed_SortArmToLoader_1_Z1Position` … `ed_SortArmToLoader_2_Z4Position` (8) and `ed_SortArmToAuto_1_Z1Position` … `ed_SortArmToAuto_6_Z4Position` (24).

**New keys (12, same `[TeachLoader]` section):**
```
ed_SortArmToLoader_1_ZBasePosition=-29.45
ed_SortArmToLoader_2_ZBasePosition=-29.45
ed_SortArmToAuto_1_ZBasePosition=-29.00
ed_SortArmToAuto_2_ZBasePosition=-29.00
ed_SortArmToAuto_3_ZBasePosition=-29.00
ed_SortArmToAuto_4_ZBasePosition=-29.00
ed_SortArmToAuto_5_ZBasePosition=-29.00
ed_SortArmToAuto_6_ZBasePosition=-29.00
ed_SortArmSuckZOffset_1=0.00
ed_SortArmSuckZOffset_2=0.00
ed_SortArmSuckZOffset_3=0.00
ed_SortArmSuckZOffset_4=0.00
```
Values above are the correct migration of the current install (all Loader slots −29.45, all Auto slots −29.00, zero per-slot variation ⇒ all offsets 0). See migration section.

**Back-compat plan (tech.ini).** The registry only ever reads keys it has registered (`OpenWorkFile` loops `i<TECH_MAX_ITEM` and calls `GetTeachKey(i)` — uteach.cpp:444-455). After refactor, the 12 new captions are registered, so it reads only the 12 new keys; the 32 stale keys are never queried and are harmless dead entries. There is **no crash or garbage-read risk** from leaving them.

The behavior of a machine upgrading **without** a migrated tech.ini:
- The 12 new keys are absent → `ReadString` returns `""` → the field keeps its zero-init value (the `TEACH` struct global is zero-initialized as a global). So **every base Z and offset would read 0**, dropping the SortArm Z plane to absolute 0 — a real crash/collision risk.
- Therefore a one-time migration is **required, not optional**. Two acceptable forms:
  1. **Edit tech.ini in place** (recommended for this repo's single committed install — do it as part of the patch; values above). Cheap, deterministic, no code.
  2. **Code-level back-compat seed in `InitialTeachParameter()`** for fielded machines whose tech.ini we cannot pre-edit: after the load, if `ed_SortArmToLoader_1_ZBasePosition` was absent, derive base = old `_Z1Position` of each station and offsets = (old `_Zk` − old `_Z1`) of a reference station. This is the migration math in the next section. Recommendation: do (1) in-repo and document (2) as the field-upgrade note in `DEV_LOG.md`; do not add permanent migration code to the hot path.

The `.ofs` offset files (`data/<workfile>.ofs`, section `[OffsetSortArm]`) carry the matching 32 old offset keys; those are handled by the consumerChanges section. Same "unread = harmless, but absent-new-key = 0" rule applies, but offsets defaulting to 0 is the *correct* no-op, so `.ofs` files need no migration.

### (2) `data/<workfile>.ofs` — per-workfile Offset values, section `[OffsetSortArm]`

Stale 32 old Z-offset keys become unread; the 12 new Z keys default to 0 when absent, which is the correct no-op. **No migration required** (see above). Saved automatically on next Offset save via the same registry loop.

### (3) `d:/HT160S_BCB/system/OffsetLimit.ini` — per-caption Offset `_Max`/`_Min` limit overrides, section `[Offset]` *(critic gap — now inventoried)*

This is a **third** Caption-keyed INI in the same scheme. Confirmed from live source:
- `GetLimitFileName()` → `system\OffsetLimit.ini` (uOffset.cpp:328-331).
- `LoadOffsetLimits()` (uOffset.cpp:348-369) loops only the **registered** `OffsetPara[i]` items, reads `Caption+"_Max"` / `Caption+"_Min"` from section `[Offset]`, and **only overrides when `V!=""`** (lines 361-366). Absent keys leave `iMax`/`iMin` at the `AddOffsetItem` default of `+100000/-100000` (±1000 mm).
- `SeedLimitFile()` (333-346) writes per-caption `_Max`/`_Min` for all registered items, **but runs only when the file does not exist** (called from `LoadOffsetLimits` at 353-356). Since `OffsetLimit.ini` already exists in-repo, it will **not** re-seed.
- `SaveOneLimit(Index)` (371-383) writes a single registered item's `_Max`/`_Min` when the operator edits a limit in the grid.

The file currently holds the 32 stale `SortArmTo*_Z*Position_Max/_Min` pairs (lines 38-101).

**Effect of the refactor on OffsetLimit.ini — self-healing, non-fatal:**
- The 12 new captions (`SortArmToLoader_1_ZBasePosition`, …, `SortArmSuckZOffset_4`) are absent from the file → `LoadOffsetLimits` leaves them at the `AddOffsetItem` default `±100000` (±1000 mm). Offsets remain fully editable; no crash, no zero-clamp.
- The 32 stale `_Z*Position_Max/_Min` pairs linger forever (never queried; harmless dead entries — same treatment as the tech.ini stale keys).
- Because the file already exists, `SeedLimitFile()` never re-runs, so the 12 new limit keys are **never persisted** until the operator edits a limit (then `SaveOneLimit` writes that one).

**Decision (recommended): regenerate OffsetLimit.ini so the 12 new keys persist cleanly.** Because the migration is being done in-repo anyway, the cheapest correct action is to **edit `OffsetLimit.ini` in place**: delete the 32 stale `SortArmTo*_Z*Position_Max/_Min` pairs (lines 38-101) and add the 12 new pairs at the same default `±1000.00`:
```
SortArmToLoader_1_ZBasePosition_Max=1000.00
SortArmToLoader_1_ZBasePosition_Min=-1000.00
SortArmToLoader_2_ZBasePosition_Max=1000.00
SortArmToLoader_2_ZBasePosition_Min=-1000.00
SortArmToAuto_1_ZBasePosition_Max=1000.00
SortArmToAuto_1_ZBasePosition_Min=-1000.00
SortArmToAuto_2_ZBasePosition_Max=1000.00
SortArmToAuto_2_ZBasePosition_Min=-1000.00
SortArmToAuto_3_ZBasePosition_Max=1000.00
SortArmToAuto_3_ZBasePosition_Min=-1000.00
SortArmToAuto_4_ZBasePosition_Max=1000.00
SortArmToAuto_4_ZBasePosition_Min=-1000.00
SortArmToAuto_5_ZBasePosition_Max=1000.00
SortArmToAuto_5_ZBasePosition_Min=-1000.00
SortArmToAuto_6_ZBasePosition_Max=1000.00
SortArmToAuto_6_ZBasePosition_Min=-1000.00
SortArmSuckZOffset_1_Max=1000.00
SortArmSuckZOffset_1_Min=-1000.00
SortArmSuckZOffset_2_Max=1000.00
SortArmSuckZOffset_2_Min=-1000.00
SortArmSuckZOffset_3_Max=1000.00
SortArmSuckZOffset_3_Min=-1000.00
SortArmSuckZOffset_4_Max=1000.00
SortArmSuckZOffset_4_Min=-1000.00
```
Keep all non-Z entries (the X/Y limit pairs at lines 2-37 and 102-113) **unchanged** — they are out of scope.

**Acceptable alternative:** simply **delete `system/OffsetLimit.ini`** so `SeedLimitFile()` rebuilds the whole file from the registry on the next Offset-screen open (it will write exactly the 12 new + all retained X/Y captions at their `AddOffsetItem` defaults). This is the lower-effort option and is fully correct because every limit currently sits at the default ±1000 anyway (verified: all values in the file are ±1000.00). Either action is fine; the in-place edit is preferred only to keep the repo diff explicit. Doing **nothing** is also non-fatal (self-healing per above) but leaves stale keys and unpersisted new keys, so it is the least-preferred option.

---

## consumerChanges

Three source files fold/blend the 32 Z fields and must shrink to the 12 new fields **in tandem** (the map flagged this as CRITICAL: uOffset.cpp realign, uOffset.cpp registration, uOffset.h struct, cprod.cpp fold must all agree, or the Offset-screen preview and production Z diverge).

### 1. `d:/HT160S_BCB/HT160S_Program_BCB_V1.0.0.0/uOffset.h`

**OLD lines 43-76 (the two Z comment+field blocks, delete):**
```cpp
    // SortArm sucker Z over Loader1/2 (8)
    int SortArmToLoader_1_Z1Position;
    int SortArmToLoader_1_Z2Position;
    int SortArmToLoader_1_Z3Position;
    int SortArmToLoader_1_Z4Position;
    int SortArmToLoader_2_Z1Position;
    int SortArmToLoader_2_Z2Position;
    int SortArmToLoader_2_Z3Position;
    int SortArmToLoader_2_Z4Position;
    // SortArm sucker Z over Auto1..6 (24)
    int SortArmToAuto_1_Z1Position;
    int SortArmToAuto_1_Z2Position;
    int SortArmToAuto_1_Z3Position;
    int SortArmToAuto_1_Z4Position;
    int SortArmToAuto_2_Z1Position;
    int SortArmToAuto_2_Z2Position;
    int SortArmToAuto_2_Z3Position;
    int SortArmToAuto_2_Z4Position;
    int SortArmToAuto_3_Z1Position;
    int SortArmToAuto_3_Z2Position;
    int SortArmToAuto_3_Z3Position;
    int SortArmToAuto_3_Z4Position;
    int SortArmToAuto_4_Z1Position;
    int SortArmToAuto_4_Z2Position;
    int SortArmToAuto_4_Z3Position;
    int SortArmToAuto_4_Z4Position;
    int SortArmToAuto_5_Z1Position;
    int SortArmToAuto_5_Z2Position;
    int SortArmToAuto_5_Z3Position;
    int SortArmToAuto_5_Z4Position;
    int SortArmToAuto_6_Z1Position;
    int SortArmToAuto_6_Z2Position;
    int SortArmToAuto_6_Z3Position;
    int SortArmToAuto_6_Z4Position;
```

**NEW:**
```cpp
    // AI 20260624 : SortArm Z base-model offsets - 8 per-station base Z + 4 shared
    // per-sucker offsets (mirror TEACH; folded by UpdateAllParameter).
    int SortArmToLoader_1_ZBasePosition;
    int SortArmToLoader_2_ZBasePosition;
    int SortArmToAuto_1_ZBasePosition;
    int SortArmToAuto_2_ZBasePosition;
    int SortArmToAuto_3_ZBasePosition;
    int SortArmToAuto_4_ZBasePosition;
    int SortArmToAuto_5_ZBasePosition;
    int SortArmToAuto_6_ZBasePosition;
    int SortArmSuckZOffset_1;
    int SortArmSuckZOffset_2;
    int SortArmSuckZOffset_3;
    int SortArmSuckZOffset_4;
```
`OFFSET_MAX_ITEM 60` (uOffset.h:86) stays; total offset items drop from ~56 to ~36.

### 2. `d:/HT160S_BCB/HT160S_Program_BCB_V1.0.0.0/uOffset.cpp` — registration

**OLD lines 222-253 (32 `AddOffsetItem` Z lines, delete):** `AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToLoader_1_Z1Position", &Offset.SortArmToLoader_1_Z1Position, 100000, -100000);` … through `"SortArmToAuto_6_Z4Position"`.

**NEW:**
```cpp
    // AI 20260624 : SortArm Z base-model offsets (8 base + 4 shared per-sucker).
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToLoader_1_ZBasePosition", &Offset.SortArmToLoader_1_ZBasePosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToLoader_2_ZBasePosition", &Offset.SortArmToLoader_2_ZBasePosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_1_ZBasePosition", &Offset.SortArmToAuto_1_ZBasePosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_2_ZBasePosition", &Offset.SortArmToAuto_2_ZBasePosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_3_ZBasePosition", &Offset.SortArmToAuto_3_ZBasePosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_4_ZBasePosition", &Offset.SortArmToAuto_4_ZBasePosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_5_ZBasePosition", &Offset.SortArmToAuto_5_ZBasePosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmToAuto_6_ZBasePosition", &Offset.SortArmToAuto_6_ZBasePosition, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmSuckZOffset_1", &Offset.SortArmSuckZOffset_1, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmSuckZOffset_2", &Offset.SortArmSuckZOffset_2, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmSuckZOffset_3", &Offset.SortArmSuckZOffset_3, 100000, -100000);
    AddOffsetItem(grdSortArm, "OffsetSortArm", "SortArmSuckZOffset_4", &Offset.SortArmSuckZOffset_4, 100000, -100000);
```

### 3. `d:/HT160S_BCB/HT160S_Program_BCB_V1.0.0.0/uOffset.cpp` — `btnReAlignClick` fold

**OLD lines 543-574 (32 `TeachBase.SortArmTo*_Z* += Offset.*` lines, delete):**
```cpp
    TeachBase.SortArmToLoader_1_Z1Position += Offset.SortArmToLoader_1_Z1Position;
    ... (30 lines) ...
    TeachBase.SortArmToAuto_6_Z4Position += Offset.SortArmToAuto_6_Z4Position;
```
**NEW:**
```cpp
    TeachBase.SortArmToLoader_1_ZBasePosition += Offset.SortArmToLoader_1_ZBasePosition;
    TeachBase.SortArmToLoader_2_ZBasePosition += Offset.SortArmToLoader_2_ZBasePosition;
    TeachBase.SortArmToAuto_1_ZBasePosition += Offset.SortArmToAuto_1_ZBasePosition;
    TeachBase.SortArmToAuto_2_ZBasePosition += Offset.SortArmToAuto_2_ZBasePosition;
    TeachBase.SortArmToAuto_3_ZBasePosition += Offset.SortArmToAuto_3_ZBasePosition;
    TeachBase.SortArmToAuto_4_ZBasePosition += Offset.SortArmToAuto_4_ZBasePosition;
    TeachBase.SortArmToAuto_5_ZBasePosition += Offset.SortArmToAuto_5_ZBasePosition;
    TeachBase.SortArmToAuto_6_ZBasePosition += Offset.SortArmToAuto_6_ZBasePosition;
    TeachBase.SortArmSuckZOffset_1 += Offset.SortArmSuckZOffset_1;
    TeachBase.SortArmSuckZOffset_2 += Offset.SortArmSuckZOffset_2;
    TeachBase.SortArmSuckZOffset_3 += Offset.SortArmSuckZOffset_3;
    TeachBase.SortArmSuckZOffset_4 += Offset.SortArmSuckZOffset_4;
```

### 4. `d:/HT160S_BCB/HT160S_Program_BCB_V1.0.0.0/cprod.cpp` — `UpdateAllParameter` fold

**OLD lines 174-205 (32 `Teach.SortArmTo*_Z* += Offset.*` lines, delete):**
```cpp
    Teach.SortArmToLoader_1_Z1Position += Offset.SortArmToLoader_1_Z1Position;
    ... (30 lines) ...
    Teach.SortArmToAuto_6_Z4Position += Offset.SortArmToAuto_6_Z4Position;
```
**NEW:**
```cpp
    Teach.SortArmToLoader_1_ZBasePosition += Offset.SortArmToLoader_1_ZBasePosition;
    Teach.SortArmToLoader_2_ZBasePosition += Offset.SortArmToLoader_2_ZBasePosition;
    Teach.SortArmToAuto_1_ZBasePosition += Offset.SortArmToAuto_1_ZBasePosition;
    Teach.SortArmToAuto_2_ZBasePosition += Offset.SortArmToAuto_2_ZBasePosition;
    Teach.SortArmToAuto_3_ZBasePosition += Offset.SortArmToAuto_3_ZBasePosition;
    Teach.SortArmToAuto_4_ZBasePosition += Offset.SortArmToAuto_4_ZBasePosition;
    Teach.SortArmToAuto_5_ZBasePosition += Offset.SortArmToAuto_5_ZBasePosition;
    Teach.SortArmToAuto_6_ZBasePosition += Offset.SortArmToAuto_6_ZBasePosition;
    Teach.SortArmSuckZOffset_1 += Offset.SortArmSuckZOffset_1;
    Teach.SortArmSuckZOffset_2 += Offset.SortArmSuckZOffset_2;
    Teach.SortArmSuckZOffset_3 += Offset.SortArmSuckZOffset_3;
    Teach.SortArmSuckZOffset_4 += Offset.SortArmSuckZOffset_4;
```

**Folding correctness check:** Offset adds independently to base and slot fields, then `GetLoaderZPosition` computes `(baseZ+ΔbaseZ) + (slotOffset+ΔslotOffset)` = correct effective Z. Both the Offset-screen preview path (`UpdateAllParameter` via `btnApplyClick`, uOffset.cpp:516) and production init (`UpdateAllParameter` in cprod) use the same `Teach = TeachBase + Offset` shape, so preview == production. No other consumer of these Z fields exists (the map's exhaustive search found only the 3 call sites, all behind the two getters).

**GetOffsetExplain — confirmed benign (open question #2 resolved).** `GetOffsetExplain` (uOffset.cpp:385-400) does **substring matching**, not a per-caption map: `if(Caption.Pos("_Z")>0) return "SortArm sucker Z height fine-tune at this station";` (lines 387-388). All 8 base captions (`..._ZBasePosition`) and all 4 shared captions (`SortArmSuckZOffset_*`) contain `_Z`, so every new caption still resolves to the Z explain string. **No change required** — purely cosmetic and already correct.

---

## migration

**What the shared-offset model assumes:** the per-slot Z delta `slotOffset[slot] = Z(station,slot) − baseZ[station]` is the same for every station (it is the physical sucker-tip height difference relative to the carriage). This is well-founded here because `GetSuckZMotor` binds slot k to the same `MSuckZ_(k+1)` axis for all stations (verified aSortArm.cpp:230-240).

**What is LOSSY:** if a fielded tech.ini has per-station-*varying* deltas (e.g. Loader1 slot2 differs from Auto3 slot2 by more than the genuine sucker delta — usually from independent hand-teaching drift), the single shared table cannot reproduce all 32 originals. The shared model collapses 32 numbers into 8+4=12; any variance beyond the rank-1 (base + per-slot) structure is discarded.

**This install is NOT lossy.** Confirmed from tech.ini lines 28-59: all 8 Loader Z = −29.45, all 24 Auto Z = −29.00. Per-slot variation is exactly zero, so the conversion is exact:
- `baseZ[Loader1] = baseZ[Loader2] = −29.45` (i.e. each station's slot0 value)
- `baseZ[Auto1..6] = −29.00`
- `slotOffset[0..3] = 0.00` (delta from slot0 of any reference station; all slots equal ⇒ all 0)

**Recommended conversion formula (for any install):**
- `baseZ[station] = old SortArmTo<station>_Z1Position` (slot0 absolute Z of that station).
- `slotOffset[slot] = old <REF>_Z(slot+1)Position − old <REF>_Z1Position`, taking the reference station `<REF> = Loader_1` (or whichever station the operator trusts most). With all-equal data this is 0 for every slot.

**Recommendation: AUTO-MIGRATE for this repo** (edit tech.ini in place to the 12 values shown in iniChanges; it is lossless here). For fielded machines, since deltas there might vary, the safe operating procedure is: auto-migrate to base = slot0 / offset = reference deltas, **then have the operator verify (re-teach) the SortArm Z plane on the real machine before production** — because (a) the model intentionally discards any genuine per-station deltas, and (b) per the project memory, the user does on-machine verification regardless. Do **not** silently average across stations (that would shift every base). Document the one-time conversion in `docs/lot-webapi/DEV_LOG.md` or a SortArm note so the next upgrade knows old `_Z1..Z4` keys are dead.

**OffsetLimit.ini migration:** none required for correctness (self-healing to ±1000 default), but per the iniChanges decision, edit it in place (swap the 32 stale `_Z*Position_Max/_Min` pairs for the 12 new pairs) **or** delete it to force `SeedLimitFile()` to rebuild — either keeps the limit file in sync. The `.ofs` per-workfile offset files need no migration (absent new Z offset keys default to 0, the correct no-op).

---

## buildConstraints

- **Byte-safe edits:** all source files (`uteach.h`, `uteach.cpp`, `aSortArm.cpp`, `aSortArm.h`, `uOffset.h`, `uOffset.cpp`, `cprod.cpp`) are legacy Big5/CP950. Use `scripts/ops/bcb6-bytesafe-edit.ps1` (confirmed present at `d:/HT160S_BCB/scripts/ops/bcb6-bytesafe-edit.ps1`) for every change — do NOT use the Edit tool, which re-encodes UTF-8 and mangles Big5 (per memory). The blocks being touched are ASCII-only field declarations and `AddTeachItem`/`AddOffsetItem`/`+=` lines, but still route through the byte-safe splicer to preserve CRLF and avoid BOM.
- **New comments ASCII-only English** (shown above all use `// AI 20260624 :` ASCII). No CP950 in new comments.
- **No C++11 / no FSM:** the rewrite uses plain `switch`, `int`, no `auto`/`nullptr`/lambdas/arrays-with-initializers; the `Task`/`switch` jog flow at line 1466 is untouched. No new step tables.
- **INI files** (`system/tech.ini`, `system/OffsetLimit.ini`) are ASCII INI; edit with the byte-safe splicer or a plain ASCII write (no BOM). `OffsetLimit.ini` may instead be **deleted** (it self-rebuilds via `SeedLimitFile`).
- **Struct change ⇒ full rebuild.** `TEACH` (uteach.h) and `RUN_OFFSET` (uOffset.h) both change member layout, so every TU that includes them must recompile. Use `scripts/ops/build-ht160s.ps1 -Full` (per memory: `-Full` deletes ALL obj — required for struct changes; `-Clean` only touches a curated obj set and would leave stale objects compiled against the old struct). No binary serialization of either struct exists (only `memset(&Offset,0,sizeof)` which is size-agnostic), so the layout change is safe across saved files.
- **Real-machine build gate:** the changed code is NOT under `#ifdef SOFT_SIMULATE` (the getters run in both builds; only `AreAllSuckersHome` has the sim guard, untouched). Still, per CLAUDE.md, after the sim `-Full` passes, comment out `#define SOFT_SIMULATE` in `MachineType.h`, run `-Full`, confirm exit 0, then RESTORE the define and rebuild — because shared core (`cprod.cpp`, `aSortArm.cpp`) changed.
- **DFM untouched:** `grdSortZ` is a dynamic `TStringGrid` filled by `ConfigureTeachGrid`/`RefreshTeachGrids` from the registry (rows auto-grow in `AddTeachItem`, uteach.cpp:206-209); no cell content lives in `uteach.dfm`. Confirmed NOT touched — do not open the form in the BCB designer (avoids the known designer-strips-components hazard). Same for `uOffset` grids.
- **Encoding check after edits:** run `scripts/ops/check-ht160s-source-encoding.ps1` (fails on `EF BF BD` and UTF-8 BOM) before declaring clean.
- Never claim build-clean unless BCB6 tools actually ran exit 0.

---

## patchOrder

Apply serially; the struct/field producers must land before consumers in the same build, but since BCB6 compiles all TUs together, do all edits then one `-Full` build. Order to minimize half-applied risk and isolate any compile error:

1. **`uteach.h`** — replace 32 Z fields with 8 base + 4 offset (structChanges).
2. **`uOffset.h`** — replace 32 Z fields with 8 base + 4 offset (consumerChanges #1). *(Do both struct headers first so dependent .cpp edits reference fields that exist.)*
3. **`uteach.cpp`** — replace 32 registrations with 12 (registrationChanges).
4. **`aSortArm.h`** — add `int GetSuckZOffset(int SlotIndex);` prototype (beside lines 62/65).
5. **`aSortArm.cpp`** — add `GetSuckZOffset` helper + rewrite `GetLoaderZPosition` + `GetAutoZPosition` (formulaChanges).
6. **`uOffset.cpp`** — replace registration block (222-253) and realign-fold block (543-574) (consumerChanges #2, #3). `GetOffsetExplain` needs no change (confirmed substring-based).
7. **`cprod.cpp`** — replace `UpdateAllParameter` fold block (174-205) (consumerChanges #4).
8. **`system/tech.ini`** — replace 32 `[TeachLoader]` Z keys with the 12 migrated keys (iniChanges + migration).
9. **`system/OffsetLimit.ini`** — replace the 32 stale `_Z*Position_Max/_Min` pairs (lines 38-101) with the 12 new pairs at ±1000.00, OR delete the file to let `SeedLimitFile()` rebuild it (iniChanges (3)). Leave the X/Y limit pairs unchanged.
10. **Build-verify (sim):** `scripts/ops/build-ht160s.ps1 -Full` → require exit 0. A miss here is almost always a leftover `Teach.SortArmTo*_Zk` / `Offset.SortArmTo*_Zk` reference — grep the tree for `_Z1Position|_Z2Position|_Z3Position|_Z4Position` to confirm zero remaining live references in `.cpp/.h` (ignore `.bak_*` files and the stale INI/`.ofs` keys, which are intentionally dead).
11. **Encoding check:** `scripts/ops/check-ht160s-source-encoding.ps1` → require pass.
12. **Build-verify (real machine):** comment `#define SOFT_SIMULATE` in `MachineType.h`, `-Full`, require exit 0, RESTORE define, `-Full` again.
13. **Doc update (non-blocking):** in `docs/manual/14-module-flows.md`, replace the old `SortArmToLoader_*_Z*` / `SortArmToAuto_*_Z*` caption references with the new base-Z + shared-offset captions so the manual matches the Teach/Offset grids. Documentation-only; does not gate the build.
14. Hand to user for on-machine SortArm Z re-teach/verification (per migration note).

---

## Resolved Questions (formerly Open)

1. **`aSortArm.h` prototype location — RESOLVED.** Critic verified against live source: `aSortArm.h` already declares `GetLoaderZPosition` (line 62) and `GetAutoZPosition` (line 65). Add `int GetSuckZOffset(int SlotIndex);` beside them. No grep step needed at patch time, though confirming before splicing is still cheap insurance.
2. **`GetOffsetExplain` — RESOLVED, benign.** It uses substring `Caption.Pos("_Z")` (uOffset.cpp:387), not a per-caption map. All 12 new captions contain `_Z`, so each gets the "SortArm sucker Z height fine-tune" explain. No edit required; cosmetic and already correct.
3. **`MAX_TEACH_ITEM`/`OFFSET_MAX_ITEM` headroom — RESOLVED.** 96→76 teach items, 60-cap vs ~36 offset items; no cap edit needed.
4. **`system/OffsetLimit.ini` (third Caption-keyed INI) — RESOLVED (critic gap, now closed).** Verified: `LoadOffsetLimits` loops registered items with `if(V!="")` fallback to the `AddOffsetItem` ±1000 default, so the 12 new captions are self-healing (no crash, no zero-clamp). `SeedLimitFile` only runs when the file is absent. Action: edit it in place (swap 32 stale `_Z*Position_Max/_Min` → 12 new pairs) or delete it to force a rebuild; both keep limits in sync. Now inventoried in iniChanges (3), migration, buildConstraints, and patchOrder step 9.

## Doc-drift inventory (non-blocking)

- `docs/manual/14-module-flows.md` references the old `SortArmToLoader_*_Z*` / `SortArmToAuto_*_Z*` captions. Update to the new captions for documentation consistency (patchOrder step 13). Doc-only; not load-bearing for build or motion.

All four CRITIC gaps and both missedConsumers (`system/OffsetLimit.ini`, `docs/manual/14-module-flows.md`) are now covered. Verdict basis from the critic (every before-snippet matches byte-for-byte; only the OffsetLimit.ini omission held `complete=false`) is closed; the design is complete.