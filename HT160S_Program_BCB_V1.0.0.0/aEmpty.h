//---------------------------------------------------------------------------
#ifndef aEmptyH
#define aEmptyH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "HTimer.h"
//---------------------------------------------------------------------------
class TEmptyModule
{
private:
    int FeedTask;
    int GoDownTask;
    int GoUpTask;
    bool bFrontHasTray;
    bool bRearHasTray;
    bool bBottomHasTray;
    bool bReturnTray;
    bool bTrayXToEmptyFinish;
    bool bLotFinish;
    HTimer FeedDelay;
    HTimer GoDownDelay;
    HTimer GoUpDelay;

    bool IsSoftSimulate();
    void RefreshStateFromSensors();
    bool MoveEmptyY(int Position);
    bool DoFeedTray(int Flag);
    bool DoGoDownTray(int Flag);
    bool DoGoUpTray(int Flag);

public:
    TEmptyModule();
    void InitialFlag();
    void DoEmpty(int &Task);

    bool IsFrontHasTray();
    bool IsRearHasTray();
    bool IsBottomHasTray();
    bool IsReturnTrayRequested();
    void SetRearHasTray(bool bHasTray);
    void RequestReturnTray();
    void NotifyTrayXToEmptyFinish();
};
//---------------------------------------------------------------------------
extern TEmptyModule *EmptyModule;
void InitializeEmptyModule();
void ShutdownEmptyModule();
//---------------------------------------------------------------------------
#endif