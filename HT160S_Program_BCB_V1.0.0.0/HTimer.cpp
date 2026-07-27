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
    //AI(ht160s-ccd-timeout) 20260727 : On() starts a FRESH timing window. Reset the
    //accumulated pause budget: only pauses that occur AFTER this arm (via Pause()/
    //ReStart()) should extend THIS window. Previously iPauseLen was cleared only in
    //Clear(); a SetMS()+On() re-arm (CcdDelay 100ms/3000ms, FeedWait, Rise1, cylinder/
    //sucker Delay, TrayArm ArmDelay -- none Clear() between cells) inherited pause time
    //accumulated while the timer was idle earlier in the same tray, so every "100ms"
    //settle became 100ms + total mid-tray pause, appearing as a hang. Resume uses
    //ReStart(), never On(), so nothing relies on iPauseLen surviving an On().
    iPauseLen=0;
    Paused=false;
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
//AI(HT160S-Maintainer) 20260623 : these three are INTENTIONALLY EMPTY. There is
//no global HTimer registry by design. A blanket pause-all would also freeze the
//Pad operator-panel link (HeartbeatTimer/VerTimeout/ReconnectDelay, uPadInterface)
//and the bin-display COM (BinDisDelay, MyBinDisp), which are pumped by the
//free-running ComPort spin (no SystemStart gate) and MUST keep running while the
//machine is paused -- pausing them deadlocks their Spin state machines. Pause is
//done SCOPED, per actuator, in csystem.cpp::PauseActuatorTimeoutTimers /
//ReStartActuatorTimeoutTimers over Cylinder[] + SortArmSuck, wired to the
//SystemStart pause/resume edges. The per-instance Pause()/ReStart() above are
//correct (iPauseLen compensation); do NOT add a TList registry here that would
//capture the comm timers without redoing that risk analysis.
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
