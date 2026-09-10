//---------------------------------------------------------------------------
#ifndef SecurityPolicyH
#define SecurityPolicyH
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------
//AI(ht160s-security) 20260909 : per-feature permission policy, modeled on the
// HT9045 numbered capability table (cSecurity.cpp TfSecurity::Insufficient).
// Each guarded function owns a stable slot ID; the level REQUIRED by that slot
// is data, not code, so a customer or service engineer can retune who may do
// what without a rebuild. The check runs through UserRoleManager.HasLevel(), so
// this is a layer on top of the role model, not a replacement for it.
//
// ENFORCEMENT RULE (ruled 20260909): no permission means the control is GREYED
// OUT - never a popup. So there is exactly ONE query, and it is SILENT:
//
//     SecurityAllows(slot)
//
// Drive ->Enabled / ->TabVisible from it on refresh paths, AND re-check it at
// the top of the action handler as defence in depth (a handler can still be
// reached by a scan key, a programmatic call, or a racing refresh that
// re-enabled the control). It shows NO message in either position, which also
// removes any risk of a permission check stopping production: the stopping
// popup ShowMyMessage() calls HSys.DecStopAllMotor() and clears
// HSys.Sys.SystemStart, so it must never sit behind a permission refusal.
//
// Slot IDs are persisted per machine in system\security.txt. NEVER renumber or
// reuse a shipped slot: append new ones at the end only.
//---------------------------------------------------------------------------
enum EHT160PermSlot
{
    // --- Main screen: running and lot handling (default Operation) ---
    PERM_MAIN_START             = 0,
    PERM_MAIN_HOME              = 1,
    PERM_MAIN_ALARM_RECOVER     = 2,
    PERM_MAIN_SPEED             = 3,
    PERM_MAIN_LOT               = 4,
    PERM_MAIN_WORKORDER         = 5,
    PERM_MAIN_CLEAR_COUNT       = 6,
    PERM_MAIN_REAL_DUMMY        = 7,

    // --- Teach and Offset ---
    PERM_TEACH_SCREEN           = 8,
    PERM_TEACH_EDIT             = 9,
    PERM_OFFSET_EDIT            = 10,

    // --- Motor test ---
    PERM_MOTORTEST_SCREEN       = 11,
    PERM_MOTORTEST_PARAM        = 12,

    // --- IO ---
    PERM_IO_SCREEN              = 13,
    PERM_IO_FORCE               = 14,
    PERM_IO_SELFTEST            = 15,
    PERM_IO_TABLE_EDIT          = 16,

    // --- Setup and recipe ---
    PERM_SETUP_SCREEN           = 17,
    PERM_SETUP_RECIPE_EDIT      = 18,
    PERM_SETUP_BINMAP           = 19,

    // --- Com port ---
    PERM_COMPORT_SCREEN         = 20,
    PERM_COMPORT_CONFIG         = 21,

    // --- Maintenance shared panels ---
    PERM_MAINT_HARDWARE_SETUP   = 22,
    PERM_MAINT_AMR_INJECT       = 23,
    PERM_MAINT_ACCOUNT_EDIT     = 24,

    // --- SECS / GEM ---
    PERM_SECS_CONTROL_STATE     = 25,
    PERM_SECS_ENDPOINT          = 26,
    PERM_SECS_EC_WRITE          = 27,
    PERM_SECS_ACCEPT_ONLINE     = 28,

    // --- Most sensitive: shipping identity and remote credentials ---
    PERM_FTP_CREDENTIALS        = 29,
    PERM_MACHINE_IDENTITY       = 30,

    // --- The gate that guards the gates. LOCKED: see IsSlotLocked(). ---
    PERM_MAINT_SECURITY_POLICY  = 31,

    // --- Main screen, appended out of group order: slot IDs are append-only ---
    // Map Tray page = tray dump memo + the simulation tools (Enable Simulation,
    // simulated 2D data, max-tray table). Honprec only by default.
    PERM_MAIN_MAP_TRAY_PAGE     = 32,

    // Simulation tools live ON that page but keep their own slot: virtual 2D codes are a
    // production-data hazard, so lowering the page must not hand the switch out with it.
    PERM_MAIN_SIMULATION_TOOLS  = 33
};
//---------------------------------------------------------------------------
#define HT160_PERM_SLOT_COUNT 34
//---------------------------------------------------------------------------
class THT160SecurityPolicy
{
private:
    int m_iRequired[HT160_PERM_SLOT_COUNT];

public:
    __fastcall THT160SecurityPolicy();

    void LoadDefaults();
    bool LoadFromFile(AnsiString FileName);
    bool SaveToFile(AnsiString FileName);

    int  GetSlotCount() const;
    int  GetRequiredLevel(int iSlot) const;
    bool SetRequiredLevel(int iSlot, int iLevel);
    AnsiString GetSlotName(int iSlot) const;
    AnsiString GetSlotGroup(int iSlot) const;

    // SILENT. Never shows a message. Fail-closed on an out-of-range slot.
    bool Allows(int iSlot) const;

    static bool IsValidSlot(int iSlot);
    // A locked slot's required level can never be lowered - not from the UI and
    // not by hand-editing system\security.txt. Whoever can rewrite the policy can
    // grant themselves everything, so the slot that guards the policy editor must
    // stay at its compiled level.
    static bool IsSlotLocked(int iSlot);
};
//---------------------------------------------------------------------------
extern THT160SecurityPolicy SecurityPolicy;
//---------------------------------------------------------------------------
// The single permission query. SILENT by design - drive ->Enabled / ->TabVisible
// from it, and re-check it at the top of the action handler.
bool SecurityAllows(int iSlot);
//---------------------------------------------------------------------------
#endif
