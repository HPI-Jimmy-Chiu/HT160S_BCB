//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "myswitch.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
class TMySwitch SWBackup[8];
//---------------------------------------------------------------------------
__fastcall TMySwitch::TMySwitch()
{
    Name="";
    Output=NULL;
    Tag=0;
    CardModal="";
    IOPos="";
    Card=0;
    Port=0;
    Bit=0;
    OutValue=false;
    SetValue=false;
    Type=0;
    Enable=false;
    EnableAtDataBase=false;
}
//---------------------------------------------------------------------------
void TMySwitch::On()
{
    OutValue=true;
    if(Enable==false || Output==NULL)
        return;
    Output->On();
}
//---------------------------------------------------------------------------
void TMySwitch::Off()
{
    OutValue=false;
    if(Enable==false || Output==NULL)
        return;
    Output->Off();
}
//---------------------------------------------------------------------------
bool TMySwitch::Status()
{
    return OutValue;
}
//---------------------------------------------------------------------------
void TMySwitch::OnOff(bool Type)
{
    if(Type)
        On();
    else
        Off();
}
//---------------------------------------------------------------------------
void CopySwitch(TMySwitch *Source, TMySwitch *Target)
{
    if(Source==NULL || Target==NULL)
        return;
    Target->Port=Source->Port;
    Target->Bit=Source->Bit;
    Target->Type=Source->Type;
}
//---------------------------------------------------------------------------
