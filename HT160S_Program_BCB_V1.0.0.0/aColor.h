//---------------------------------------------------------------------------
#ifndef aColorH
#define aColorH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "HTimer.h"
#include "MotorAndIO/MyMotor.h"   //AI(ht160s-tray-source) : TMyTray for MMColorY->Tray (identity grid lives on the motor)
//---------------------------------------------------------------------------
class TMyCylinder;
//---------------------------------------------------------------------------
enum eHT160ColorMode
{
    eHT160ColorModeSortBin=0,
    eHT160ColorModeTraySupply=1
};
//---------------------------------------------------------------------------
//AI(ht160s-status) 20260703 : explicit module status (approved unified-status design,
//docs/plan/module-status-enum-design-20260703.md). Ladder-owned; sensors never write it.
//Frozen at CS_IDLE for the whole SortBin mode (peers never handshake Color there).
enum eColorStatus
{
    CS_IDLE=0,
    CS_DESTACK,       //front destacker separating one identity tray to staging
    CS_FEEDING,       //DoFeedTray active incl. CCD scan : carrier owns the rear
    CS_REAR_READY,    //identity tray presented at rear (bTrayReady moment)
    CS_RETURNING,     //receive/return ladder active (case 1700 / CleanOut drain)
};
//---------------------------------------------------------------------------
class TColorModule
{
private:
    int Status;             //AI(ht160s-status) 20260703 : eColorStatus, ladder-owned (see enum note)
    int FeedTask;
    int FeedClampSub;       //AI(HT160S-Maintainer) 20260623 : DoClampTray sub-state for DoFeedTray
    int SortBinTask;
    int ScanTask;             //AI(HT160S-Maintainer) 20260608 : 2D CCD read sub-ladder state
    int GoDownTask;           //AI(HT160S-Maintainer) 20260608 : front-stack separate sub-ladder (mirrors Empty)
    int TestUpTask;           //AI(general) 20260617 : Teach Advanced destacker test (GoUp)
    int TestDownTask;         //AI(general) 20260617 : Teach Advanced destacker test (GoDown)
    HTimer TestDelay;         //AI(general) 20260617 : Teach Advanced destacker test settle delay
    int iMode;
    int iSupplyThreshold;
    int iICCount;
    bool bInputFullTray;
    bool bFrontHasTray;       //AI(HT160S-Maintainer) 20260608 : one tray separated and staged at the front (post DoGoDownTray)
    bool bRearHasTray;
    bool bTrayReady;
    bool bSupplyRequested;
    AnsiString sTrayID2D;     //AI(HT160S-Maintainer) 20260608 : 2D code read from the supplied identity tray
    TMyTray FrontSourceTray;  //AI(ht160s-color-align-empty) 20260627 : FRONT staging holder (mirror Empty); carried tray lives on HSys.VMot.MMColorY->Tray
    HTimer FeedDelay;
    HTimer GoDownDelay;       //AI(HT160S-Maintainer) 20260608 : front separate settle delay
    HTimer ScanDelay;         //AI(HT160S-Maintainer) 20260608 : Color CCD shot response timeout

    bool bAmrLocked;          //AI(ht160s-agv) 20260623 : AMR handoff lock (freeze front destack)
    int iSimInfeedCount;      //AI(ht160s-agv) 20260623 : sim input-stack tray count (drains per destack)

    //AI(phase6-loader-recycle) 20260625 : Color receive-tray flow, ported near-verbatim
    //from TEmptyModule (U4 : Empty and Color are the same destacker mechanism). The
    //TrayArm returns a leftover identity tray to Color's rear handoff position; Color
    //stacks it back onto the front supply car (DoGoUpTray) for closed-loop reuse.
    int GoUpTask;
    bool bReturnTray;
    bool bTrayXToEmptyFinish;
    int iReturnedCount;
    HTimer GoUpDelay;

    //AI(ht160s-agv-identity2d) 20260714 : Loader-recovery identity-tray intake -- receive at rear,
    //carry to the middle CCD, read 2D, upload S6F11 CEID275/SVID38204 (cloud reconciliation), then
    //retreat to the front car. Distinct from the recycle return (bReturnTray -> DoGoUpTray 1700).
    int  ReadIdentityTask;      //DoReadIdentityRetreat sub-ladder state
    int  RaiseFrontTask;        //RaiseFrontStackClear helper sub-ladder state (extracted front-raise)
    int  ReadClampSub;          //DoClampTray sub-state for the identity intake clamp
    bool bReadIdentityPending;  //TrayArm reserved a Loader identity intake -> Color dedicates to receive
    HTimer ReadIdentityDelay;   //identity intake clamp settle

    bool IsSoftSimulate();
    bool IsInstalled();
    void RefreshStateFromSensors();
    bool PushCylinder(TMyCylinder &Cyn);
    bool PopCylinder(TMyCylinder &Cyn);
    bool MoveColorY(int Position);   //AI(HT160S-Maintainer) 20260622 : move Color carriage in Y (front/back)
    bool MoveColorCcdX(int Position);   //AI(ht160s-color-ccd-xy) 20260628 : move Color CCD reader X (shared by scan-move + DoReadColor2D)
    bool DoGoDownTray(int Flag);   //AI(HT160S-Maintainer) 20260608 : separate one tray off the front stack -> front staging (like Empty)
    bool DoGoUpTray(int Flag);     //AI(phase6-loader-recycle) 20260625 : stack a returned tray back onto the front supply car (mirrors TEmptyModule::DoGoUpTray)
    bool DoReadIdentityRetreat(int Flag);  //AI(ht160s-agv-identity2d) 20260714 : Loader identity at rear -> CCD scan -> upload CEID275 -> retreat to front
    bool RaiseFrontStackClear(int Flag);   //AI(ht160s-agv-identity2d) 20260714 : lift front stack clear of the rest (extracted DoGoUpTray front-raise; self-terminating)
    bool DoFeedTray(int Flag);
    bool DoSortBin(int Flag);
    bool DoReadColor2D(int Flag);  //AI(HT160S-Maintainer) 20260608 : move CCD X, LON shot, read 2D, LOFF
    void BirthFrontTray();           //AI(ht160s-color-align-empty) 20260627 : identity tray born at GoDown front-confirm into FrontSourceTray (mirror Empty)
    void StampReadIdentity2D();      //AI(ht160s-color-align-empty) 20260627 : CCD read UPDATES the carried tray 2D TrayID (not a birth)

public:
    TColorModule();
    void InitialFlag(bool bKeepMaterial=false);   //AI(ht160s-home-resume-w1) 20260711 : keep-material HOME preserves the return handshake + a sensor-confirmed presented identity tray
    void PauseTimeoutTimers();     //AI(ht160s-actuator-timer) 20260627 : freeze ScanDelay timeout window on machine pause
    void ReStartTimeoutTimers();   //AI(ht160s-actuator-timer) 20260627 : thaw it on resume (csystem actuator-timer enrollment)
    void DoColor(int &Task);
    AnsiString DescribeState();   //AI(ht160s-state-record-analysis) 20260622 : read-only inner-state + latch dump (FeederDecision.txt)

    bool SetMode(int Mode);
    int GetMode();
    bool IsTraySupplyMode();
    bool IsSortBinMode();
    bool IsTrayReady();
    bool IsAcceptingIC();
    void RequestSupplyTray();
    void NotifyTrayPicked();
    void RequestReturnTray();        //AI(phase6-loader-recycle) 20260625 : TrayArm asks Color to accept a returned identity tray (same contract name as Empty)
    bool IsRearHasTray();            //AI(phase6-loader-recycle) 20260625 : Color rear handoff position occupied (same contract name as Empty)
    bool IsCleanOutFinish();         //AI(cleanout) 20260701 : Color CleanOut-drain finish (installed? + TrayArm done + flow clear + rise cylinders home)
    void NotifyTrayXToEmptyFinish(); //AI(phase6-loader-recycle) 20260625 : TrayArm finished depositing onto Color's rear (same contract name as Empty)
    void RequestReadIdentityTray();  //AI(ht160s-agv-identity2d) 20260714 : TrayArm reserves a Loader identity intake (scan+upload), distinct from the recycle return
    bool IsReadyToReceiveIdentity(); //AI(ht160s-agv-identity2d) 20260714 : Color fully idle -> safe for TrayArm to deposit a Loader identity at rear (pick-time interlock)
    bool IsReceivingIdentity();      //AI(ht160s-agv-identity2d) 20260714 : an identity intake is reserved/in-progress (TrayArm case-500 gate discriminator)
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
    bool IsInputFullForAmr();   //AI(cleanout) 20260703 : SnColor_InputFullTray verdict (sim-false) - CleanOut GoUp/finish Full gate
    int  GetStatus();           //AI(ht160s-status) 20260703 : eColorStatus for stbMain display / peers
    void RefillSimInfeed();
    int GetCarTrayCount();   //AI(ht160s-agv) 20260624 : sim input-stack tray count on the supply car (PanelMain6 Motion View header)
    AnsiString GetTrayID();   //AI(HT160S-Maintainer) 20260608 : 2D TrayID of the tray Color is presenting
    bool IsTrayID2DGenuine();   //AI(ht160s-agv-identity2d) 20260713 : true if the 2D may upload as SVID38202 (real read/manual/sim; false only on real machine with Color CCD disabled)
    TMyTray GetSourceTray();   //AI(ht160s-tray-source) : return-by-value deep copy of the presented identity tray

    int GetHomeHaulTargetY();
    bool HomeDrainTick();            //AI(ht160s-home-resume-drain) 20260711 : pump the pure-cylinder destack ladders to their phase boundary (true=converged/idle)        //AI(ht160s-home-resume-w3b2) 20260711 : uHome PARK snapshot haul-target (-1 = no haul in flight)
    bool TestGoUpTray(int Flag);     //AI(general) 20260617 : Teach Advanced destacker test (cylinder-only GoUp; Color has no production GoUp)
    bool TestGoDownTray(int Flag);   //AI(general) 20260617 : Teach Advanced destacker test (cylinder-only GoDown, mirrors Empty)
    bool MoveColorCcdToScan();       //AI(ht160s-color-ccd-xy) 20260628 : carriage Y + CCD reader X to photo pos together (prod case3000 + Teach test)
};
//---------------------------------------------------------------------------
extern TColorModule *ColorModule;
void InitializeColorModule();
void ShutdownColorModule();
//---------------------------------------------------------------------------
#endif