//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "myswitch.h"
#include "uPadInterface.h"
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
    //AI(ht160s-maintainer) 20260616 : if this switch drives a physical Pad button
    //LED, push the state to the Pad instead of a (usually unbound) IO output, so
    //machine state lights the panel LEDs. Mirrors HT172 myswitch.cpp On() (the
    //iControlPanelMode==1 gate there is an always-on #define, so it is dropped).
    if(fPadInterface!=NULL && fPadInterface->IsPadButton(Name))
    {
        fPadInterface->SendSwitchStatus(Name, true);
        return;
    }
    if(Enable==false || Output==NULL)
        return;
    Output->On();
}
//---------------------------------------------------------------------------
void TMySwitch::Off()
{
    OutValue=false;
    if(fPadInterface!=NULL && fPadInterface->IsPadButton(Name))
    {
        fPadInterface->SendSwitchStatus(Name, false);
        return;
    }
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
