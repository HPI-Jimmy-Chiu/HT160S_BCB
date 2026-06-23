#include "IncludeAllHeader.h"       //Dell 將.h統一,可加速build
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "aColor.h"
#include "database.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "GeneralSetting.h"
#include "uteach.h"
#include "ColorCcdSocket.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TColorModule *ColorModule=NULL;
//---------------------------------------------------------------------------
TColorModule::TColorModule()
{
    iMode=eHT160ColorModeTraySupply;
    iSupplyThreshold=100;
    InitialFlag();
}
//---------------------------------------------------------------------------
void TColorModule::InitialFlag()
{
    bAmrLocked=false;
    RefillSimInfeed();
    SupplyTask=1;
    SupplyClampSub=0;
    ReleaseTask=1;
    SortBinTask=1;
    iICCount=0;
    bInputHasTray=false;
    bInputFullTray=false;
    bOutputHasTray=false;
    bTrayReady=false;
    bTrayPicked=false;
    bSupplyRequested=false;
    bFrontHasTray=false;
    ScanTask=1;
    GoDownTask=1;
    sTrayID2D="";
    SupplyDelay.Clear();
    ReleaseDelay.Clear();
    GoDownDelay.Clear();
    ScanDelay.Clear();
    TestUpTask=1;
    TestDownTask=1;
    TestDelay.Clear();
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : AMR P3 (ColorTray) handoff interface, mirrors TAutoModule.
void TColorModule::SetAmrLock(bool bLock)
{
    bAmrLocked=bLock;
}
//---------------------------------------------------------------------------
bool TColorModule::IsAmrLocked()
{
    return bAmrLocked;
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : Ready = front stacking cylinders back home (not commanded
//up); destack idle so the AGV may refill. Held stable by bAmrLocked.
bool TColorModule::IsReadyForAmrHandoff()
{
    return (HSys.Cyn.C_Color_FrontRiseTray_1.GetOutBit()==false
            && HSys.Cyn.C_Color_FrontRiseTray_2.GetOutBit()==false
            && HSys.Cyn.C_Color_FrontSeparateTray_1.GetOutBit()==false);
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : shortage (call AGV). Sim drains iSimInfeedCount to 0; real
//reads SnColor_InputEnd (ON=has tray, OFF=empty). Disabled sensor -> no call.
bool TColorModule::IsInputShortageForAmr()
{
    if(IsSoftSimulate())
        return (iSimInfeedCount<=0);
    return (HSys.Sen.SnColor_InputEnd.Enable==true && HSys.Sen.SnColor_InputEnd.IsOff());
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : Finish = refill complete. Sim auto-completes (no sensor);
//real waits for SnColor_InputEnd to read a tray present (ON).
bool TColorModule::IsInputHandoffFinishedForAmr()
{
    if(IsSoftSimulate())
        return true;
    return (HSys.Sen.SnColor_InputEnd.Enable==true && HSys.Sen.SnColor_InputEnd.IsOn());
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : reset sim input-stack to configured max (AGV delivered a
//full magazine). Real machine ignores the count (sensor-driven).
void TColorModule::RefillSimInfeed()
{
    iSimInfeedCount=GeneralSetting.iSimAmrMaxTray[2];
}
//---------------------------------------------------------------------------
bool TColorModule::IsSoftSimulate()
{
    #ifdef SOFT_SIMULATE
    return true;
    #else
    return (HSys.LastSet.iRealDummy==DUMMY);
    #endif
}
//---------------------------------------------------------------------------
bool TColorModule::IsInstalled()
{
    return GeneralSetting.bColorBinAreaInstalled;
}
//---------------------------------------------------------------------------
void TColorModule::RefreshStateFromSensors()
{
    bool bHasInputSensor=false;
    bool bHasOutputSensor=false;
    bool bOutputState=false;

    if(IsInstalled()==false)
    {
        bInputHasTray=false;
        bInputFullTray=false;
        bOutputHasTray=false;
        bTrayReady=false;
        bTrayPicked=false;
        return;
    }

    if(HSys.Sen.SnColor_InputHasTray.Enable==true)
    {
        bHasInputSensor=true;
        bInputHasTray=HSys.Sen.SnColor_InputHasTray.IsOn();
    }

    if(HSys.Sen.SnColor_InputFullTray.Enable==true)
    {
        bInputFullTray=HSys.Sen.SnColor_InputFullTray.IsOn();
        if(bInputFullTray)
            bInputHasTray=true;
    }
    else
        bInputFullTray=false;

    if(HSys.Sen.SnColor_OutputBottomHasTray.Enable==true)
    {
        bHasOutputSensor=true;
        if(HSys.Sen.SnColor_OutputBottomHasTray.IsOn())
            bOutputState=true;
    }

    if(HSys.Sen.SnColor_TrayPos1.Enable==true)
    {
        bHasOutputSensor=true;
        if(HSys.Sen.SnColor_TrayPos1.IsOn())
            bOutputState=true;
    }

    if(bHasOutputSensor)
        bOutputHasTray=bOutputState;
    else if(IsSoftSimulate())
        bOutputHasTray=bTrayReady;

    if(bHasInputSensor==false && IsSoftSimulate())
        bInputHasTray=true;

    //AI(general) 20260609 : Removed the sensor-driven bTrayReady latch. It set
    //bTrayReady=true from any output-sensor read and never cleared it (only
    //DoReleaseTray did), so IsTrayReady() stayed latched true and DoColor case 100
    //always folded back to idle, never reaching the supply ladder. bTrayReady is
    //now owned solely by the supply ladder (DoSupplyTray case 900 sets it,
    //DoReleaseTray clears it), mirroring Empty's just-in-time ready model.
}
//---------------------------------------------------------------------------
bool TColorModule::PushCylinder(TMyCylinder &Cyn)
{
    if(IsSoftSimulate())
        return true;
    if(Cyn.Enable==false)
        return true;
    return Cyn.Push();
}
//---------------------------------------------------------------------------
bool TColorModule::PopCylinder(TMyCylinder &Cyn)
{
    if(IsSoftSimulate())
        return true;
    if(Cyn.Enable==false)
        return true;
    return Cyn.Pop();
}
//---------------------------------------------------------------------------
void TColorModule::DoColor(int &Task)
{
    if(IsInstalled()==false)
    {
        Task=1;
        return;
    }

    switch(Task)
    {
        case 1:
            Task=10;
            break;

        case 10:
            RefreshStateFromSensors();
            if(IsSortBinMode())
            {
                DoSortBin(0);
                Task=2000;
                break;
            }
            Task=100;
            break;

        case 100:
            if(bAmrLocked)
                break;
            RefreshStateFromSensors();
            if(bTrayReady && bTrayPicked)
            {
                DoReleaseTray(0);
                Task=1500;
                break;
            }
            if(bTrayReady)
            {
                Task=1;
                break;
            }
            //AI(HT160S-Maintainer) 20260608 : two-stage supply like Empty. Stage 1
            //keeps the front buffer filled (separate one tray off the stack via
            //DoGoDownTray). Stage 2 pushes that staged tray to the output and reads
            //its 2D code, but only when an AMR supply was actually requested (or
            //simulating), so the identity tray is not presented / scanned early.
            if(bFrontHasTray==false)
            {
                if(bInputHasTray || IsSoftSimulate())
                {
                    DoGoDownTray(0);
                    Task=1200;
                }
                else
                    Task=1;
                break;
            }
            if(bSupplyRequested)
            {
                DoSupplyTray(0);
                Task=1000;
            }
            else
                Task=1;
            break;

        case 1000:
            if(DoSupplyTray(1))
                Task=1;
            break;

        case 1200:
            if(DoGoDownTray(1))
            {
                if(IsSoftSimulate() && iSimInfeedCount>0)
                    iSimInfeedCount--;   //AI(ht160s-agv) sim input drains 1/GoDown
                Task=1;
            }
            break;

        case 1500:
            if(DoReleaseTray(1))
                Task=1;
            break;

        case 2000:
            if(DoSortBin(1))
                Task=1;
            break;
    }
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260608 : separate one tray off the front stack and stage
//it at the front, mirroring TEmptyModule::DoGoDownTray. Sets bFrontHasTray on
//success so DoSupplyTray can later push it to the output. Kept distinct so the
//front buffer can be pre-staged (Empty-style pipelining) while the AMR pull request
//is still pending. Color has no dedicated front-staging sensor, so staging is
//logical; physical presence is confirmed at the output (DoSupplyTray case 800).
bool TColorModule::DoGoDownTray(int Flag)
{
    if(Flag==0)
    {
        GoDownTask=1;
        GoDownDelay.Clear();
        return true;
    }

    switch(GoDownTask)
    {
        case 1:
            if(IsTraySupplyMode()==false)
                return true;
            GoDownTask=10;
            break;

        case 10:
            RefreshStateFromSensors();
            if(bFrontHasTray)
                return true;
            if(bInputHasTray==false && IsSoftSimulate()==false)
                return true;
            GoDownTask=100;
            break;

        case 100:
            //AI(HT160S-Maintainer) 20260608 : dual destacker, mirror Empty. Rise_1 up.
            if(PushCylinder(HSys.Cyn.C_Color_FrontRiseTray_1))
                GoDownTask=150;
            break;

        case 150:
            if(PushCylinder(HSys.Cyn.C_Color_FrontRiseTray_2))
                GoDownTask=200;
            break;

        case 200:
            if(PushCylinder(HSys.Cyn.C_Color_FrontSeparateTray_1))
            {
                GoDownDelay.Set(5);
                GoDownDelay.On();
                GoDownTask=300;
            }
            break;

        case 300:
            if(GoDownDelay.Off())
            {
                PopCylinder(HSys.Cyn.C_Color_FrontRiseTray_2);
                GoDownDelay.Set(5);
                GoDownDelay.On();
                GoDownTask=350;
            }
            break;

        case 350:
            if(GoDownDelay.Off())
                GoDownTask=400;
            break;

        case 400:
            if(PopCylinder(HSys.Cyn.C_Color_FrontSeparateTray_1))
            {
                GoDownDelay.Set(5);
                GoDownDelay.On();
                GoDownTask=450;
            }
            break;

        case 450:
            if(GoDownDelay.Off())
                GoDownTask=500;
            break;

        case 500:
            if(PopCylinder(HSys.Cyn.C_Color_FrontRiseTray_1))
            {
                //AI(HT160S-Maintainer) 20260608 : stage logically; no front sensor.
                bFrontHasTray=true;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TColorModule::MoveColorY(int Position)
{
    //AI(HT160S-Maintainer) 20260622 : move the Color carriage in Y (front/back relative to the
    //operator; X=left/right, Z=up/down). Mirrors MoveEmptyY. Returns true when in position (and
    //immediately under sim, where MotorMove sim-completes). On a teach/limit error, report and
    //return true so the supply ladder is not stalled.
    if(HSys.Mot.MColorY==NULL)
        return true;
    if(HSys.Mot.MColorY->CheckSoftLimit(Position)==false)
    {
        ShowMyMessage("Color Y motor will out of limit");
        return true;
    }
    return HSys.Mot.MColorY->MotorMove(Position);
}
//---------------------------------------------------------------------------
bool TColorModule::DoSupplyTray(int Flag)
{
    int Ret;

    if(Flag==0)
    {
        SupplyTask=1;
        SupplyClampSub=0;
        SupplyDelay.Clear();
        return true;
    }

    switch(SupplyTask)
    {
        case 1:
            if(IsTraySupplyMode()==false)
                return true;
            SupplyTask=5;
            break;

        case 5:
            //AI(HT160S-Maintainer) 20260622 : position the Color carriage at the FRONT
            //receive/read Y (Teach.ColorRead2DYPosition) before the destacker pushes a tray
            //onto it and before the 2D read. Y = front/back. Also returns the carriage to the
            //front each cycle (it ends a cycle at the rear pickup Y).
            if(MoveColorY(Teach.ColorRead2DYPosition))
                SupplyTask=10;
            break;

        case 10:
            RefreshStateFromSensors();
            if(bOutputHasTray)
            {
                //AI(HT160S-Maintainer) 20260608 : a tray already sits at the output
                //read position; still drive the 2D CCD read before TrayArm pickup.
                DoReadColor2D(0);
                SupplyTask=900;
                break;
            }
            //AI(HT160S-Maintainer) 20260608 : the front buffer must already hold a
            //separated tray (staged by DoGoDownTray). Without it there is nothing to
            //push to the output, so bail and let DoColor run a godown cycle first.
            if(bFrontHasTray==false && IsSoftSimulate()==false)
            {
                bSupplyRequested=false;
                return true;
            }
            SupplyTask=600;
            break;

        case 600:
        {
            //AI(HT160S-Maintainer) 20260623 : standardized dual-cylinder clamp via
            //DoClampTray, shared with Empty. SettleTicks=0 keeps Color's current
            //behavior (no push-on-sensor confirm); the output sensor at case 800
            //verifies the tray. Flip to >0 to add the push confirm later.
            int Clamp=DoClampTray(HSys.Cyn.C_Color_LeanOnTray, HSys.Cyn.C_Color_PushTray,
                                  SupplyClampSub, SupplyDelay, IsSoftSimulate(), 0);
            if(Clamp==1)
                SupplyTask=800;
            break;
        }

        case 800:
            RefreshStateFromSensors();
            if(HSys.Sen.SnColor_OutputBottomHasTray.Enable==true &&
               bOutputHasTray==false && HSys.LastSet.iRealDummy!=DUMMY)
            {
                Ret=ShowMyError("Color supply tray is not ready", K_RETRY);
                if(Ret==K_RETRY)
                    SupplyTask=1;
            }
            else
            {
                //AI(HT160S-Maintainer) 20260608 : tray pushed to the read/output
                //position; the front buffer is now empty. Read its 2D TrayID
                //before it is ready for TrayArm pickup.
                bFrontHasTray=false;
                DoReadColor2D(0);
                SupplyTask=900;
            }
            break;

        case 900:
            //AI(HT160S-Maintainer) 20260608 : drive Color 2D CCD (LON/read/LOFF).
            //sTrayID2D filled by DoReadColor2D; empty when simulated or SKIPped.
            if(DoReadColor2D(1))
                SupplyTask=950;
            break;

        case 950:
            //AI(HT160S-Maintainer) 20260622 : after the 2D read, MColorY carries the tray to the
            //REAR pickup Y (Teach.ColorTrayArmPickYPosition) so the TrayArm can grip it. Only
            //then is the tray ready for pickup.
            if(MoveColorY(Teach.ColorTrayArmPickYPosition))
            {
                bTrayReady=true;
                bTrayPicked=false;
                bSupplyRequested=false;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260608 : Color identity-tray 2D barcode read sub-ladder.
//Mirrors the Loader Top CCD flow : move the reader (on the stepper, X axis) to the
//taught read position, LON to shoot, poll the socket for the code, LOFF to end.
bool TColorModule::DoReadColor2D(int Flag)
{
    int Ret;

    if(Flag==0)
    {
        ScanTask=1;
        ScanDelay.Clear();
        return true;
    }

    switch(ScanTask)
    {
        case 1:
            //Offline / sim : no real Color CCD, fabricate a TrayID so the AMR
            //pull flow runs without a camera; cbEnableSimulation fakes too.
            if(IsSoftSimulate() || tSimuData.bRunSimulation)
            {
                sTrayID2D=AnsiString("COLOR2D_")+Now().FormatString("hhnnsszzz");
                return true;
            }
            //AI(HT160S-Maintainer) 20260612 : gate the Color CCD connect/shot
            //trigger on CosFunction.bUseColorCcd ([ColorCCD] Enable), mirroring how
            //aLoader gates the Top CCD on CosFunction.bUse2DBinMap. When disabled,
            //skip the camera and report no 2D code so the supply flow continues.
            if(CosFunction.bUseColorCcd==false)
            {
                sTrayID2D="";
                return true;
            }
            EnsureColorCcdSocketCreated();
            if(ColorCcdSocket!=NULL)
                ColorCcdSocket->ColorCcdConnect();
            ScanTask=10;
            break;

        case 10:
            //Move the 2D reader to the taught read position over the identity tray.
            if(HSys.Mot.MTopCCDX_Color==NULL)
            {
                ScanTask=100;
                break;
            }
            if(HSys.Mot.MTopCCDX_Color->CheckSoftLimit(Teach.ColorRead2DXPosition)==false)
            {
                ShowMyMessage("Color CCD X motor will out of limit");
                ScanTask=100;
                break;
            }
            if(HSys.Mot.MTopCCDX_Color->MotorMove(Teach.ColorRead2DXPosition))
                ScanTask=100;
            break;

        case 100:
            if(ColorCcdSocket==NULL || ColorCcdSocket->IsColorCcdConnected()==false)
            {
                Ret=ShowMyError("Color CCD connect not ready", K_RETRY|K_SKIP);
                if(Ret==K_SKIP)
                {
                    sTrayID2D="";
                    return true;
                }
                EnsureColorCcdSocketCreated();
                if(ColorCcdSocket!=NULL)
                    ColorCcdSocket->ColorCcdConnect();
                break;
            }
            ColorCcdSocket->ColorCcdTriggerShot();   //LON : start shot
            ScanDelay.SetMS(3000);
            ScanDelay.On();
            ScanTask=200;
            break;

        case 200:
            {
                AnsiString sCode="";
                if(ColorCcdSocket!=NULL && ColorCcdSocket->ColorCcdGetResult(sCode))
                {
                    sTrayID2D=sCode;
                    if(ColorCcdSocket!=NULL)
                        ColorCcdSocket->ColorCcdEndShot();   //LOFF : end shot
                    return true;
                }
                else if(ScanDelay.Off())
                {
                    if(ColorCcdSocket!=NULL)
                        ColorCcdSocket->ColorCcdEndShot();   //LOFF : end shot
                    Ret=ShowMyError("Color CCD 2D no response", K_RETRY|K_SKIP);
                    if(Ret==K_RETRY)
                    {
                        if(ColorCcdSocket!=NULL)
                            ColorCcdSocket->ColorCcdTriggerShot();
                        ScanDelay.SetMS(3000);
                        ScanDelay.On();
                    }
                    else
                    {
                        sTrayID2D="";
                        return true;
                    }
                }
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TColorModule::DoReleaseTray(int Flag)
{
    if(Flag==0)
    {
        ReleaseTask=1;
        ReleaseDelay.Clear();
        return true;
    }

    switch(ReleaseTask)
    {
        case 1:
            ReleaseTask=100;
            break;

        case 100:
            if(PopCylinder(HSys.Cyn.C_Color_PushTray))
                ReleaseTask=200;
            break;

        case 200:
            if(PopCylinder(HSys.Cyn.C_Color_LeanOnTray))
                ReleaseTask=300;
            break;

        case 300:
            if(PopCylinder(HSys.Cyn.C_Color_RearRiseTray))
                ReleaseTask=400;
            break;

        case 400:
            if(PopCylinder(HSys.Cyn.C_Color_FrontRiseTray_2))
                ReleaseTask=450;
            break;

        case 450:
            if(PopCylinder(HSys.Cyn.C_Color_FrontRiseTray_1))
                ReleaseTask=500;
            break;

        case 500:
            if(PopCylinder(HSys.Cyn.C_Color_FrontSeparateTray_1))
            {
                bTrayReady=false;
                bTrayPicked=false;
                bOutputHasTray=false;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TColorModule::DoSortBin(int Flag)
{
    if(Flag==0)
    {
        SortBinTask=1;
        return true;
    }

    switch(SortBinTask)
    {
        case 1:
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TColorModule::SetMode(int Mode)
{
    if(Mode!=eHT160ColorModeSortBin && Mode!=eHT160ColorModeTraySupply)
        return false;
    iMode=Mode;
    return true;
}
//---------------------------------------------------------------------------
int TColorModule::GetMode()
{
    return iMode;
}
//---------------------------------------------------------------------------
bool TColorModule::IsTraySupplyMode()
{
    return iMode==eHT160ColorModeTraySupply;
}
//---------------------------------------------------------------------------
bool TColorModule::IsSortBinMode()
{
    return iMode==eHT160ColorModeSortBin;
}
//---------------------------------------------------------------------------
bool TColorModule::IsTrayReady()
{
    RefreshStateFromSensors();
    return IsInstalled() && IsTraySupplyMode() && bTrayReady && bTrayPicked==false;
}
//---------------------------------------------------------------------------
bool TColorModule::IsInputHasTray()
{
    RefreshStateFromSensors();
    return bInputHasTray;
}
//---------------------------------------------------------------------------
bool TColorModule::IsOutputHasTray()
{
    RefreshStateFromSensors();
    return bOutputHasTray;
}
//---------------------------------------------------------------------------
bool TColorModule::IsAcceptingIC()
{
    return false;
}
//---------------------------------------------------------------------------
void TColorModule::RequestSupplyTray()
{
    if(IsInstalled() && IsTraySupplyMode())
        bSupplyRequested=true;
}
//---------------------------------------------------------------------------
void TColorModule::NotifyTrayPicked()
{
    if(bTrayReady)
        bTrayPicked=true;
}
//---------------------------------------------------------------------------
void TColorModule::NotifyICPlaced(int Count)
{
    if(Count>0)
        iICCount+=Count;
}
//---------------------------------------------------------------------------
void TColorModule::SetSupplyThreshold(int Count)
{
    if(Count<1)
        Count=1;
    iSupplyThreshold=Count;
}
//---------------------------------------------------------------------------
int TColorModule::GetSupplyThreshold()
{
    return iSupplyThreshold;
}
//---------------------------------------------------------------------------
int TColorModule::GetICCount()
{
    return iICCount;
}
//---------------------------------------------------------------------------
AnsiString TColorModule::GetTrayID()
{
    return sTrayID2D;
}
//---------------------------------------------------------------------------
//AI(general) 20260617 : Teach Advanced destacker test. Color has no production GoUp;
//these cylinder-only GoDown/GoUp drive the front destacker (FrontRiseTray_1/_2/Separate)
//in isolation, mirroring Empty's rise/separate choreography. No Y-motor / push / lean.
bool TColorModule::TestGoDownTray(int Flag)
{
    if(Flag==0)
    {
        TestDownTask=1;
        TestDelay.Clear();
        return true;
    }

    switch(TestDownTask)
    {
        case 1:
            HSys.Cyn.C_Color_FrontRiseTray_1.On();
            TestDownTask=2000;
            break;

        case 2000:
            if(HSys.Cyn.C_Color_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Color_FrontRiseTray_2.On();
                TestDownTask=3000;
            }
            break;

        case 3000:
            if(HSys.Cyn.C_Color_FrontRiseTray_2.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Color_FrontSeparateTray_1.On();
                TestDelay.Set(5);
                TestDelay.On();
                TestDownTask=4000;
            }
            break;

        case 4000:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Color_FrontRiseTray_2.Off();
                TestDelay.Set(5);
                TestDelay.On();
                TestDownTask=4100;
            }
            break;

        case 4100:
            if(TestDelay.Off())
                TestDownTask=5000;
            break;

        case 5000:
            if(HSys.Cyn.C_Color_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Color_FrontSeparateTray_1.Off();
                TestDelay.Set(5);
                TestDelay.On();
                TestDownTask=6000;
            }
            break;

        case 6000:
            if(TestDelay.Off())
                TestDownTask=6500;
            break;

        case 6500:
            if(HSys.Cyn.C_Color_FrontRiseTray_1.Pop() || IsSoftSimulate())
            {
                TestDownTask=1;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TColorModule::TestGoUpTray(int Flag)
{
    if(Flag==0)
    {
        TestUpTask=1;
        TestDelay.Clear();
        return true;
    }

    switch(TestUpTask)
    {
        case 1:
            HSys.Cyn.C_Color_FrontRiseTray_1.On();
            TestUpTask=200;
            break;

        case 200:
            if(HSys.Cyn.C_Color_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Color_FrontSeparateTray_1.On();
                TestDelay.Set(5);
                TestDelay.On();
                TestUpTask=300;
            }
            break;

        case 300:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Color_FrontRiseTray_2.On();
                TestUpTask=400;
            }
            break;

        case 400:
            if(HSys.Cyn.C_Color_FrontRiseTray_2.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Color_FrontSeparateTray_1.Off();
                TestDelay.Set(5);
                TestDelay.On();
                TestUpTask=500;
            }
            break;

        case 500:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Color_FrontRiseTray_2.Off();
                if(HSys.Cyn.C_Color_FrontRiseTray_1.IsOn() || IsSoftSimulate())
                    TestUpTask=600;
            }
            break;

        case 600:
            if(HSys.Cyn.C_Color_FrontRiseTray_1.Pop() || IsSoftSimulate())
            {
                TestUpTask=1;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
AnsiString TColorModule::DescribeState()
{
    //AI(ht160s-state-record-analysis) 20260622 : read-only inner-state dump for
    //FeederDecision.txt. Reads latched members directly (does NOT call
    //RefreshStateFromSensors, which clobbers the sim/dummy latch). The
    //bTrayReady/bTrayPicked/bSupplyRequested trio is first : a latched bTrayReady
    //with no pick is the Normal-mode (no AMR demand) idle-spin signature.
    AnsiString s;
    s  = "[Color]\r\n";
    s += "  bTrayReady=" + IntToStr(bTrayReady ? 1 : 0)
       + "  bTrayPicked=" + IntToStr(bTrayPicked ? 1 : 0)
       + "  bSupplyRequested=" + IntToStr(bSupplyRequested ? 1 : 0)
       + "  bFrontHasTray=" + IntToStr(bFrontHasTray ? 1 : 0) + "\r\n";
    s += "  Installed=" + IntToStr(IsInstalled() ? 1 : 0)
       + "  SoftSim=" + IntToStr(IsSoftSimulate() ? 1 : 0)
       + "  Mode=" + AnsiString(IsTraySupplyMode() ? "TraySupply" : (IsSortBinMode() ? "SortBin" : "?")) + "\r\n";
    s += "  bInputHasTray=" + IntToStr(bInputHasTray ? 1 : 0)
       + "  bInputFullTray=" + IntToStr(bInputFullTray ? 1 : 0)
       + "  bOutputHasTray=" + IntToStr(bOutputHasTray ? 1 : 0) + "\r\n";
    s += "  SupplyTask=" + IntToStr(SupplyTask)
       + "  GoDownTask=" + IntToStr(GoDownTask)
       + "  ReleaseTask=" + IntToStr(ReleaseTask)
       + "  ScanTask=" + IntToStr(ScanTask)
       + "  SortBinTask=" + IntToStr(SortBinTask) + "\r\n";
    s += "  iICCount=" + IntToStr(iICCount)
       + "  iSupplyThreshold=" + IntToStr(iSupplyThreshold)
       + "  TrayID2D=" + sTrayID2D + "\r\n";
    return s;
}
//---------------------------------------------------------------------------
void InitializeColorModule()
{
    if(ColorModule==NULL)
        ColorModule=new TColorModule;
}
//---------------------------------------------------------------------------
void ShutdownColorModule()
{
    delete ColorModule;
    ColorModule=NULL;
}
//---------------------------------------------------------------------------