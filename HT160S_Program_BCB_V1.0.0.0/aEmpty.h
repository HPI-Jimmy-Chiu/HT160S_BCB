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
    int FeedClampSub;   //AI(HT160S-Maintainer) 20260623 : DoClampTray sub-state for DoFeedTray
    int GoDownTask;
    int GoUpTask;
    int TestUpTask;
    int TestDownTask;
    HTimer TestDelay;
    bool bFrontHasTray;
    bool bRearHasTray;
    bool bBottomHasTray;
    bool bReturnTray;
    bool bTrayXToEmptyFinish;
    bool bLotFinish;
    bool bAmrLocked;          //AI(ht160s-agv) 20260623 : AMR P2 handoff lock (freeze front destack)
    int iSimInfeedCount;      //AI(ht160s-agv) 20260623 : sim input-stack tray count (drains per GoDown)
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
    AnsiString DescribeState();   //AI(ht160s-state-record-analysis) 20260622 : read-only inner-state dump (FeederDecision.txt)

    bool IsFrontHasTray();
    bool IsRearHasTray();
    bool IsBottomHasTray();
    bool IsReturnTrayRequested();
    void SetRearHasTray(bool bHasTray);
    void RequestReturnTray();
    void NotifyTrayXToEmptyFinish();

    //AI(ht160s-agv) 20260623 : AMR P2 (EmptyTray) handoff interface (mirrors TAutoModule).
    void SetAmrLock(bool bLock);
    bool IsAmrLocked();
    bool IsReadyForAmrHandoff();
    bool IsInputShortageForAmr();
    bool IsInputHandoffFinishedForAmr();
    void RefillSimInfeed();

    bool TestGoUpTray(int Flag);     //AI(general) 20260617 : Teach Advanced destacker test (cylinder-only GoUp, mirrors DoGoUpTray rise steps)
    bool TestGoDownTray(int Flag);   //AI(general) 20260617 : Teach Advanced destacker test (cylinder-only GoDown, mirrors DoGoDownTray)
};
//---------------------------------------------------------------------------
extern TEmptyModule *EmptyModule;
void InitializeEmptyModule();
void ShutdownEmptyModule();
//---------------------------------------------------------------------------
#endif