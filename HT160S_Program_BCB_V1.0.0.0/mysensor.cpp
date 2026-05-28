//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "mysensor.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall TMySensor::TMySensor()
{
    Name="";
    OnAlarmCode="";
    OffAlarmCode="";
    AlarmType="";
    Tag=0;
    CardModal="";
    IOPos="";
    Card=0;
    Port=0;
    Bit=0;
    Type=0;
    Enable=false;
    EnableAtDataBase=false;
    bErroHappen=false;
    iStatus=false;
    Input=NULL;
}
//---------------------------------------------------------------------------
bool TMySensor::Status()
{
    return IsOn();
}
//---------------------------------------------------------------------------
bool TMySensor::IsOn()
{
    bool bOn=false;
    if(Enable==false || Input==NULL)
    {
        iStatus=-1;
        return false;
    }

    if(Type==1)
        bOn=Input->IsOn();
    else
        bOn=!Input->IsOn();

    iStatus=bOn?1:0;
    return bOn;
}
//---------------------------------------------------------------------------
bool TMySensor::IsOff()
{
    bool bOff=false;
    if(Enable==false || Input==NULL)
    {
        iStatus=-1;
        return false;
    }

    if(Type==1)
        bOff=Input->IsOff();
    else
        bOff=!Input->IsOff();

    iStatus=bOff?0:1;
    return bOff;
}
//---------------------------------------------------------------------------
void CopySensor(TMySensor *Source, TMySensor *Target)
{
    if(Source==NULL || Target==NULL)
        return;
    Target->Card=Source->Card;
    Target->Port=Source->Port;
    Target->Bit=Source->Bit;
    Target->Type=Source->Type;
}
//---------------------------------------------------------------------------
bool GetTrayBuildState(int pos)
{
    return false;
}
//---------------------------------------------------------------------------
void SetTrayBuildState(int pos)
{
}
//---------------------------------------------------------------------------
void ClrTrayBuildState(int pos)
{
}
//---------------------------------------------------------------------------
