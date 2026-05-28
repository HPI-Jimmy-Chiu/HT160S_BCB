//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "HTimer.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
__fastcall HTimer::HTimer()
{
    Clear();
}
//---------------------------------------------------------------------------
void HTimer::Set(int iTime)
{
    iTimeLen=iTime*100;
}
//---------------------------------------------------------------------------
void HTimer::SetTimer(int iTime, int iID)
{
    if(iTimerID!=iID)
        Clear();
    iTimeLen=iTime*100;
    iTimerID=iID;
}
//---------------------------------------------------------------------------
void HTimer::SetMS(int iTime)
{
    iTimeLen=iTime;
}
//---------------------------------------------------------------------------
void HTimer::On()
{
    ulStartTicks=GetTickCount();
    InUsed=true;
}
//---------------------------------------------------------------------------
bool HTimer::Off()
{
    if(Paused)
        return false;
    if(ulStartTicks==0)
        return false;
    if(iTimeLen==0)
    {
        InUsed=false;
        return true;
    }
    if(GetTickCount()-ulStartTicks>=(DWORD)(iTimeLen+iPauseLen))
    {
        InUsed=false;
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
void HTimer::Clear()
{
    ulStartTicks=0;
    ulPauseTicks=0;
    iTimeLen=0;
    iPauseLen=0;
    Paused=false;
    InUsed=false;
    iTimerID=0;
}
//---------------------------------------------------------------------------
void HTimer::Pause()
{
    if(Paused)
        return;
    Paused=true;
    ulPauseTicks=GetTickCount();
}
//---------------------------------------------------------------------------
void HTimer::ReStart()
{
    if(!Paused)
        return;
    Paused=false;
    iPauseLen+=GetTickCount()-ulPauseTicks;
}
//---------------------------------------------------------------------------
void ClearAllTimer()
{
}
//---------------------------------------------------------------------------
void PauseAllTimer()
{
}
//---------------------------------------------------------------------------
void ReStartAllTimer()
{
}
//---------------------------------------------------------------------------
