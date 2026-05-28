//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "UsecegemMainFrom.h"
#include "database.h"
#include "uHGemClass.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TFSECS *FSECS = NULL;
int USE_SECS_GEM = 1;
//---------------------------------------------------------------------------
__fastcall TFSECS::TFSECS(TComponent *Owner)
    : TComponent(Owner)
{
    bInitialed = false;
}
//---------------------------------------------------------------------------
__fastcall TFSECS::~TFSECS()
{
}
//---------------------------------------------------------------------------
void TFSECS::GemInitial(AnsiString HandlerType, AnsiString SoftwareVersion)
{
    if(HGem==NULL || HSys.MyGem==NULL)
        return;

    HGem->SetTimeFormat(1);
    HGem->SetDefaultAddressAndPort("127.0.0.1", "5098", "0");
    HGem->SetReceipeDirectoryAndGlobalName("..\\data\\", "*.*", 0);
    HGem->SetMachineTypeAndSoftwarseVer(HandlerType.c_str(), SoftwareVersion.c_str());
    SECS_SETData(HGem);
    HGem->SaveEventReportData();
    HGem->Timer1->Enabled = true;
    bInitialed = true;
}
//---------------------------------------------------------------------------
void TFSECS::SECS_SETData(THGem *Ptr)
{
    if(Ptr==NULL || HSys.MyGem==NULL)
        return;

    HSys.MyGem->AddSV();
    HSys.MyGem->AddEC();
    HSys.MyGem->AddAlarmList();
    HSys.MyGem->AddCEID();
    HSys.MyGem->AddReprot();
}
//---------------------------------------------------------------------------
int TFSECS::MySFCode(int Stream, int Function)
{
    if(HSys.MyGem==NULL)
        return 1;

    if(Stream==2 && Function==41)
        return HSys.MyGem->S2F42_Host_Command_Acknowledge();

    return 1;
}
//---------------------------------------------------------------------------
bool TFSECS::IsInitialed()
{
    return bInitialed;
}
//---------------------------------------------------------------------------
void EventReport(unsigned Ceid)
{
    if(USE_SECS_GEM<=0 || HGem==NULL)
        return;

    HGem->EventReport(1, Ceid);
}
//---------------------------------------------------------------------------
