//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "cSelfCheck.h"
#include "database.h"
#include "aLoader.h"
#include "aEmpty.h"
#include "aAuto1To6.h"
#include "aTrayArm.h"
#include "aSortArm.h"
#include "aColor.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
static const int EXPECTED_MOTION_ACTION_COUNT = 7;
//---------------------------------------------------------------------------
static void CheckPtr(AnsiString &Report, bool &AllOk,
                     const AnsiString &Name, void *Ptr)
{
    if(Ptr != NULL)
    {
        Report += Name + " ... OK\r\n";
    }
    else
    {
        Report += Name + " ... X NULL\r\n";
        AllOk = false;
    }
}
//---------------------------------------------------------------------------
bool ValidateWiring(AnsiString &Report)
{
    bool AllOk = true;

    Report = "[WIRING CHECK]\r\n";

    // 1. Module objects must be created (ht160s.cpp InitializeXxxModule()).
    CheckPtr(Report, AllOk, "LoaderModule  ", LoaderModule);
    CheckPtr(Report, AllOk, "EmptyModule   ", EmptyModule);
    CheckPtr(Report, AllOk, "AutoModule    ", AutoModule);
    CheckPtr(Report, AllOk, "TrayArmModule ", TrayArmModule);
    CheckPtr(Report, AllOk, "SortArmModule ", SortArmModule);
    CheckPtr(Report, AllOk, "ColorModule   ", ColorModule);

    // 2. DataModule and its motion action list must exist.
    CheckPtr(Report, AllOk, "DataModule1   ", DataModule1);

    TActionList *Motion = NULL;
    if(DataModule1 != NULL)
        Motion = DataModule1->UserMotion;
    CheckPtr(Report, AllOk, "UserMotion    ", Motion);

    // 3. Action count and every OnExecute binding ("Tick register").
    if(Motion != NULL)
    {
        if(Motion->ActionCount == EXPECTED_MOTION_ACTION_COUNT)
        {
            Report += "UserMotion.Count=" +
                      IntToStr(Motion->ActionCount) + " ... OK\r\n";
        }
        else
        {
            Report += "UserMotion.Count=" + IntToStr(Motion->ActionCount) +
                      " ... X expected " +
                      IntToStr(EXPECTED_MOTION_ACTION_COUNT) + "\r\n";
            AllOk = false;
        }

        for(int i = 0; i < Motion->ActionCount; i++)
        {
            TContainedAction *Act = Motion->Actions[i];
            AnsiString Caption = "Action[" + IntToStr(i) + "]";
            if(Act != NULL)
                Caption = "Action " + Act->Name;

            bool Bound = (Act != NULL && Act->OnExecute != NULL);
            if(Bound)
            {
                Report += Caption + ".OnExecute ... OK\r\n";
            }
            else
            {
                Report += Caption + ".OnExecute ... X NULL\r\n";
                AllOk = false;
            }
        }
    }

    if(AllOk)
        Report += "-> all wires connected.\r\n";
    else
        Report += "-> wiring incomplete. Fix before start.\r\n";

    return AllOk;
}
//---------------------------------------------------------------------------
