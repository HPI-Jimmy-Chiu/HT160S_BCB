//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "SecurityPolicy.h"
#include "UserRoleManager.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
THT160SecurityPolicy SecurityPolicy;
//---------------------------------------------------------------------------
//AI(ht160s-security) 20260909 : the one place where policy lives. Group and
// name are engineer-facing labels for the configuration page (routed through
// LangT at display time by the caller, never baked into a DFM). iDefaultLevel
// encodes the 20260909 ruling: run / lot / alarm / speed stay Operation,
// general engineering setup needs Engineer, and shipping identity plus remote
// credentials need Honprec. Clear-counters is the one Supervisor item.
// ROW ORDER MUST MATCH EHT160PermSlot - the array index IS the slot ID.
struct THT160PermSlotDef
{
    const char *pGroup;
    const char *pName;
    int iDefaultLevel;
};
//---------------------------------------------------------------------------
static const THT160PermSlotDef PermSlotTable[HT160_PERM_SLOT_COUNT]=
{
    { "Main",        "Start / One Cycle / Pause / Clean Out",      ROLE_OPERATION  },
    { "Main",        "Home",                                       ROLE_OPERATION  },
    { "Main",        "Alarm recover / skip / buzzer off",          ROLE_OPERATION  },
    { "Main",        "Speed override",                             ROLE_OPERATION  },
    { "Main",        "Lot Start / Lot End",                        ROLE_OPERATION  },
    { "Main",        "Work order edit / import / WebAPI pull",     ROLE_OPERATION  },
    { "Main",        "Clear all production counters",              ROLE_SUPERVISOR },
    { "Main",        "Real / Dummy / Has-Tray mode",               ROLE_ENGINEER   },
    { "Teach",       "Enter Teach screen",                         ROLE_ENGINEER   },
    { "Teach",       "Set / save teach position, jog / step",      ROLE_ENGINEER   },
    { "Teach",       "Offset edit / realign / clear / limits",     ROLE_ENGINEER   },
    { "Motor Test",  "Enter Motor Test screen",                    ROLE_ENGINEER   },
    { "Motor Test",  "Motor parameter table edit",                 ROLE_ENGINEER   },
    { "IO",          "Enter IO view screen",                       ROLE_ENGINEER   },
    { "IO",          "Force output / cylinder / vacuum / sucker",  ROLE_ENGINEER   },
    { "IO",          "IO self test",                               ROLE_ENGINEER   },
    { "IO",          "IO table edit / save input-output map",      ROLE_ENGINEER   },
    { "Setup",       "Enter Setup screen",                         ROLE_ENGINEER   },
    { "Setup",       "Recipe new / save / save as / delete",       ROLE_ENGINEER   },
    { "Setup",       "Bin map load / save / clear / default",      ROLE_ENGINEER   },
    { "Com Port",    "Enter Com Port screen",                      ROLE_ENGINEER   },
    { "Com Port",    "Com port config / reset / manual send",      ROLE_ENGINEER   },
    { "Maintenance", "Hardware install setup",                     ROLE_ENGINEER   },
    { "Maintenance", "AMR / AGV signal injection test",            ROLE_ENGINEER   },
    { "Maintenance", "User account book edit",                     ROLE_ENGINEER   },
    { "SECS",        "Control state Off-Line / Local / Remote",    ROLE_ENGINEER   },
    { "SECS",        "Endpoint / ActiveMode config",               ROLE_HONPREC    },
    { "SECS",        "Equipment constant (EC) write",              ROLE_HONPREC    },
    { "SECS",        "Accept host on-line request switch",         ROLE_HONPREC    },
    { "Service",     "FTP credentials view / edit / test",         ROLE_HONPREC    },
    { "Service",     "Machine model / handler ID / serial number", ROLE_HONPREC    },
    { "Security",    "Security policy edit (locked)",              ROLE_HONPREC    },
    //AI(ht160s-security) 20260910 : appended out of group order on purpose - a shipped
    // slot ID is never renumbered (system\security.txt keys on it), so a new Main-screen
    // slot still lands at the end of the table.
    { "Main",        "Map Tray page (tray dump / simulation)",     ROLE_HONPREC    }
};
//---------------------------------------------------------------------------
__fastcall THT160SecurityPolicy::THT160SecurityPolicy()
{
    LoadDefaults();
}
//---------------------------------------------------------------------------
bool THT160SecurityPolicy::IsValidSlot(int iSlot)
{
    return (iSlot>=0 && iSlot<HT160_PERM_SLOT_COUNT);
}
//---------------------------------------------------------------------------
//AI(ht160s-security) 20260909 : the gate that guards the gates. Anyone who can rewrite the
// policy can grant themselves every permission, so the slot controlling the
// policy editor is pinned to its compiled level: SetRequiredLevel refuses it and
// LoadFromFile skips it, which closes the UI path AND the hand-edited-file path.
// Same intent as the HT9045 forced slots in GetLevelSet(), but declared here in
// one place instead of as magic indices inside the loader.
bool THT160SecurityPolicy::IsSlotLocked(int iSlot)
{
    return (iSlot==PERM_MAINT_SECURITY_POLICY);
}
//---------------------------------------------------------------------------
void THT160SecurityPolicy::LoadDefaults()
{
    int i;

    for(i=0; i<HT160_PERM_SLOT_COUNT; i++)
        m_iRequired[i]=PermSlotTable[i].iDefaultLevel;
}
//---------------------------------------------------------------------------
int THT160SecurityPolicy::GetSlotCount() const
{
    return HT160_PERM_SLOT_COUNT;
}
//---------------------------------------------------------------------------
int THT160SecurityPolicy::GetRequiredLevel(int iSlot) const
{
    if(!IsValidSlot(iSlot))
        return ROLE_HONPREC;
    return m_iRequired[iSlot];
}
//---------------------------------------------------------------------------
bool THT160SecurityPolicy::SetRequiredLevel(int iSlot, int iLevel)
{
    if(!IsValidSlot(iSlot))
        return false;
    if(IsSlotLocked(iSlot))
        return false;
    if(!THT160UserRoleManager::IsValidLevel(iLevel))
        return false;
    m_iRequired[iSlot]=iLevel;
    return true;
}
//---------------------------------------------------------------------------
AnsiString THT160SecurityPolicy::GetSlotName(int iSlot) const
{
    if(!IsValidSlot(iSlot))
        return "";
    return AnsiString(PermSlotTable[iSlot].pName);
}
//---------------------------------------------------------------------------
AnsiString THT160SecurityPolicy::GetSlotGroup(int iSlot) const
{
    if(!IsValidSlot(iSlot))
        return "";
    return AnsiString(PermSlotTable[iSlot].pGroup);
}
//---------------------------------------------------------------------------
//AI(ht160s-security) 20260909 : SILENT. Fail-closed - an out-of-range slot is a
// programming mistake, so it denies rather than opening the function up (same
// bound behavior as the HT9045 Insufficient() range check). Shows NO message:
// callers use this on refresh paths that run on every page open, and the ruling
// is that a denied function is greyed out rather than announced.
bool THT160SecurityPolicy::Allows(int iSlot) const
{
    if(!IsValidSlot(iSlot))
        return false;
    return UserRoleManager.HasLevel(m_iRequired[iSlot]);
}
//---------------------------------------------------------------------------
//AI(ht160s-security) 20260909 : plaintext "SlotID,Level" book, deliberately NOT
// the HT9045 binary blob (levelset.dat written with sizeof(LevelSet)): a text
// file is notepad-editable like login.txt, and it stays readable when slots are
// appended, whereas a fixed-size blob misaligns. Unknown or out-of-range slots
// are skipped and any slot missing from the file keeps its compiled default, so
// an older file still loads correctly after new slots are added.
bool THT160SecurityPolicy::LoadFromFile(AnsiString FileName)
{
    TStringList *List;
    AnsiString Line, sSlot, sLevel;
    int i, p, iSlot, iLevel;
    bool bLoaded;

    LoadDefaults();
    if(!FileExists(FileName))
        return false;

    List=new TStringList;
    bLoaded=false;
    try
    {
        try
        {
            List->LoadFromFile(FileName);
            bLoaded=true;
        }
        catch(...)
        {
            bLoaded=false;
        }

        if(bLoaded)
        {
            for(i=0; i<List->Count; i++)
            {
                Line=List->Strings[i];
                p=Line.Pos("#");
                if(p>0)
                    Line=Line.SubString(1, p-1);
                p=Line.Pos(";");
                if(p>0)
                    Line=Line.SubString(1, p-1);
                Line=Line.Trim();
                if(Line==AnsiString(""))
                    continue;

                p=Line.Pos(",");
                if(p<=0)
                    continue;
                sSlot=Line.SubString(1, p-1).Trim();
                sLevel=Line.SubString(p+1, Line.Length()).Trim();
                p=sLevel.Pos(",");
                if(p>0)
                    sLevel=sLevel.SubString(1, p-1).Trim();

                iSlot=StrToIntDef(sSlot, -1);
                iLevel=StrToIntDef(sLevel, -1);
                if(!IsValidSlot(iSlot))
                    continue;
                if(IsSlotLocked(iSlot))
                    continue;
                if(!THT160UserRoleManager::IsValidLevel(iLevel))
                    continue;
                m_iRequired[iSlot]=iLevel;
            }
        }
    }
    __finally
    {
        delete List;
    }
    return bLoaded;
}
//---------------------------------------------------------------------------
bool THT160SecurityPolicy::SaveToFile(AnsiString FileName)
{
    TStringList *List;
    AnsiString Line;
    int i;
    bool bSaved;

    List=new TStringList;
    bSaved=false;
    try
    {
        List->Add("# HT160S Security Policy - required user level per function");
        List->Add("# format: SlotID,Level  (Level 0=Operation 1=Supervisor 2=Engineer 3=Honprec)");
        List->Add("# Text after '#' is a label only and is ignored on load.");
        for(i=0; i<HT160_PERM_SLOT_COUNT; i++)
        {
            Line=IntToStr(i)+","+IntToStr(m_iRequired[i])+
                 "    # "+AnsiString(PermSlotTable[i].pGroup)+" - "+
                 AnsiString(PermSlotTable[i].pName);
            List->Add(Line);
        }
        try
        {
            List->SaveToFile(FileName);
            bSaved=true;
        }
        catch(...)
        {
            bSaved=false;
        }
    }
    __finally
    {
        delete List;
    }
    return bSaved;
}
//---------------------------------------------------------------------------
bool SecurityAllows(int iSlot)
{
    return SecurityPolicy.Allows(iSlot);
}
//---------------------------------------------------------------------------
