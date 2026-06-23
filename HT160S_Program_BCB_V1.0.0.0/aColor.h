//---------------------------------------------------------------------------
#ifndef aColorH
#define aColorH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "HTimer.h"
//---------------------------------------------------------------------------
class TMyCylinder;
//---------------------------------------------------------------------------
enum eHT160ColorMode
{
    eHT160ColorModeSortBin=0,
    eHT160ColorModeTraySupply=1
};
//---------------------------------------------------------------------------
class TColorModule
{
private:
    int SupplyTask;
    int SupplyClampSub;       //AI(HT160S-Maintainer) 20260623 : DoClampTray sub-state for DoSupplyTray
    int ReleaseTask;
    int SortBinTask;
    int ScanTask;             //AI(HT160S-Maintainer) 20260608 : 2D CCD read sub-ladder state
    int GoDownTask;           //AI(HT160S-Maintainer) 20260608 : front-stack separate sub-ladder (mirrors Empty)
    int TestUpTask;           //AI(general) 20260617 : Teach Advanced destacker test (GoUp)
    int TestDownTask;         //AI(general) 20260617 : Teach Advanced destacker test (GoDown)
    HTimer TestDelay;         //AI(general) 20260617 : Teach Advanced destacker test settle delay
    int iMode;
    int iSupplyThreshold;
    int iICCount;
    bool bInputHasTray;
    bool bInputFullTray;
    bool bFrontHasTray;       //AI(HT160S-Maintainer) 20260608 : one tray separated and staged at the front (post DoGoDownTray)
    bool bOutputHasTray;
    bool bTrayReady;
    bool bTrayPicked;
    bool bSupplyRequested;
    AnsiString sTrayID2D;     //AI(HT160S-Maintainer) 20260608 : 2D code read from the supplied identity tray
    HTimer SupplyDelay;
    HTimer ReleaseDelay;
    HTimer GoDownDelay;       //AI(HT160S-Maintainer) 20260608 : front separate settle delay
    HTimer ScanDelay;         //AI(HT160S-Maintainer) 20260608 : Color CCD shot response timeout

    bool bAmrLocked;          //AI(ht160s-agv) 20260623 : AMR handoff lock (freeze front destack)
    int iSimInfeedCount;      //AI(ht160s-agv) 20260623 : sim input-stack tray count (drains per destack)

    bool IsSoftSimulate();
    bool IsInstalled();
    void RefreshStateFromSensors();
    bool PushCylinder(TMyCylinder &Cyn);
    bool PopCylinder(TMyCylinder &Cyn);
    bool MoveColorY(int Position);   //AI(HT160S-Maintainer) 20260622 : move Color carriage in Y (front/back)
    bool DoGoDownTray(int Flag);   //AI(HT160S-Maintainer) 20260608 : separate one tray off the front stack -> front staging (like Empty)
    bool DoSupplyTray(int Flag);
    bool DoReleaseTray(int Flag);
    bool DoSortBin(int Flag);
    bool DoReadColor2D(int Flag);  //AI(HT160S-Maintainer) 20260608 : move CCD X, LON shot, read 2D, LOFF

public:
    TColorModule();
    void InitialFlag();
    void DoColor(int &Task);
    AnsiString DescribeState();   //AI(ht160s-state-record-analysis) 20260622 : read-only inner-state + latch dump (FeederDecision.txt)

    bool SetMode(int Mode);
    int GetMode();
    bool IsTraySupplyMode();
    bool IsSortBinMode();
    bool IsTrayReady();
    bool IsInputHasTray();
    bool IsOutputHasTray();
    bool IsAcceptingIC();
    void RequestSupplyTray();
    void NotifyTrayPicked();
    void NotifyICPlaced(int Count);
    void SetSupplyThreshold(int Count);
    int GetSupplyThreshold();
    int GetICCount();

    //AI(ht160s-agv) 20260623 : AMR handoff interface (mirrors TAutoModule).
    void SetAmrLock(bool bLock);
    bool IsAmrLocked();
    bool IsReadyForAmrHandoff();
    bool IsInputShortageForAmr();
    bool IsInputHandoffFinishedForAmr();
    void RefillSimInfeed();
    AnsiString GetTrayID();   //AI(HT160S-Maintainer) 20260608 : 2D TrayID of the tray Color is presenting

    bool TestGoUpTray(int Flag);     //AI(general) 20260617 : Teach Advanced destacker test (cylinder-only GoUp; Color has no production GoUp)
    bool TestGoDownTray(int Flag);   //AI(general) 20260617 : Teach Advanced destacker test (cylinder-only GoDown, mirrors Empty)
};
//---------------------------------------------------------------------------
extern TColorModule *ColorModule;
void InitializeColorModule();
void ShutdownColorModule();
//---------------------------------------------------------------------------
#endif