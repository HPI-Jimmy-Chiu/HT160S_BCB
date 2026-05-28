//---------------------------------------------------------------------------
#ifndef HTimerH
#define HTimerH
//---------------------------------------------------------------------------
#include <Windows.hpp>
//---------------------------------------------------------------------------
class HTimer
{
public:
    __fastcall HTimer();
    void On();
    bool Off();
    void Set(int iTime);
    void SetTimer(int iTime, int iID);
    void SetMS(int iTime);
    void Clear();
    void Pause();
    void ReStart();

private:
    DWORD ulStartTicks;
    DWORD ulPauseTicks;
    int iTimeLen;
    int iPauseLen;
    bool Paused;
    bool InUsed;
    int iTimerID;
};
//---------------------------------------------------------------------------
void ClearAllTimer();
void PauseAllTimer();
void ReStartAllTimer();
//---------------------------------------------------------------------------
#endif
