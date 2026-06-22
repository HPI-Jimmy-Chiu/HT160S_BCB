#include "IncludeAllHeader.h"       //Dell 將.h統一,可加速build
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "aEmpty.h"
#include "database.h"
#include "cmydef.h"
#include "mymessbox.h"
#include "uteach.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TEmptyModule *EmptyModule=NULL;
//---------------------------------------------------------------------------
TEmptyModule::TEmptyModule()
{
    InitialFlag();
}
//---------------------------------------------------------------------------
void TEmptyModule::InitialFlag()
{
    FeedTask=1;
    GoDownTask=1;
    GoUpTask=1;
    bFrontHasTray=false;
    bRearHasTray=false;
    bBottomHasTray=false;
    bReturnTray=false;
    bTrayXToEmptyFinish=false;
    bLotFinish=false;
    FeedDelay.Clear();
    GoDownDelay.Clear();
    GoUpDelay.Clear();
    TestUpTask=1;
    TestDownTask=1;
    TestDelay.Clear();
}
//---------------------------------------------------------------------------
bool TEmptyModule::IsSoftSimulate()
{
    #ifdef SOFT_SIMULATE
    return true;
    #else
    return (HSys.LastSet.iRealDummy==DUMMY);
    #endif
}
//---------------------------------------------------------------------------
void TEmptyModule::RefreshStateFromSensors()
{
    //AI(HT160S-Maintainer) 20260622 : in SOFT_SIMULATE / real-machine DUMMY mode there are no
    //real trays, so DO NOT read the physical sensors here - the tray state is a LATCH owned by
    //the action sequence (DoFeedTray/DoGoDownTray set it when a tray is fed / comes down; the
    //TrayArm clears it via SetRearHasTray(false) when MotorY takes the tray away).
    //WHY reading sensors is wrong in sim/dummy: these HasTray inputs are wired InType=0
    //(active-low), and with no MN200 card TMyIo::IsOn() falls back to bOutValue(=false for a
    //never-written input), which InType=0 then inverts to TMySensor::IsOn()==true. So every
    //InType=0 input would read 'tray present' regardless of reality (the opposite of what
    //IOsetview shows on real hardware), wiping the latch and stalling the module. (Mirrors the
    //iRealDummy!=DUMMY gates on the sensor-miss checks in DoFeedTray/DoGoDownTray.)
    if(IsSoftSimulate())
        return;

    bool bHasRearSensor=false;
    bool bRearState=false;

    if(HSys.Sen.SnEmpty_InputHasTray.Enable==true)
        bFrontHasTray=HSys.Sen.SnEmpty_InputHasTray.IsOn();

    if(HSys.Sen.SnEmpty_OutputHasTray.Enable==true)
    {
        bHasRearSensor=true;
        if(HSys.Sen.SnEmpty_OutputHasTray.IsOn())
            bRearState=true;
    }

    if(HSys.Sen.SnEmpty_OutputBottomHasTray.Enable==true)
    {
        bHasRearSensor=true;
        bBottomHasTray=HSys.Sen.SnEmpty_OutputBottomHasTray.IsOn();
        if(bBottomHasTray)
            bRearState=true;
    }

    if(bHasRearSensor)
        bRearHasTray=bRearState;
}
//---------------------------------------------------------------------------
bool TEmptyModule::MoveEmptyY(int Position)
{
    if(HSys.Mot.MEmptyY==NULL)
        return false;
    if(HSys.Mot.MEmptyY->CheckSoftLimit(Position)==false)
    {
        ShowMyMessage("Empty Y motor will out of limit");
        return false;
    }

    #ifndef SOFT_SIMULATE
    int TrayArmPos=0;
    if(HSys.Mot.MTrayArmX!=NULL)
        TrayArmPos=HSys.Mot.MTrayArmX->ReadEncoderPos();
    if(HSys.Cyn.C_TrayArmZ_Up.IsOn()==false &&
       (TrayArmPos+500)>=Teach.TrayXArmToEmptyXPosition)
    {
        return false;
    }
    #endif

    return HSys.Mot.MEmptyY->MotorMove(Position);
}
//---------------------------------------------------------------------------
void TEmptyModule::DoEmpty(int &Task)
{
    switch(Task)
    {
        case 1:
            Task=10;
            break;

        case 10:
            Task=100;
            break;

        case 100:
            RefreshStateFromSensors();
            if(bReturnTray)
            {
                DoGoUpTray(0);
                Task=3000;
                break;
            }

            if(bFrontHasTray==false && bLotFinish==false)
            {
                DoGoDownTray(0);
                Task=1000;
                break;
            }

            if(bRearHasTray==false && bLotFinish==false)
            {
                DoFeedTray(0);
                Task=2000;
                break;
            }

            if(bLotFinish && bFrontHasTray)
            {
                DoGoUpTray(0);
                Task=3000;
            }
            break;

        case 1000:
            if(DoGoDownTray(1))
                Task=1;
            break;

        case 2000:
            if(DoFeedTray(1))
                Task=1;
            break;

        case 3000:
            if(DoGoUpTray(1))
            {
                if(bReturnTray && bTrayXToEmptyFinish==false)
                    return;
                bReturnTray=false;
                Task=1;
            }
            break;
    }
}
//---------------------------------------------------------------------------
bool TEmptyModule::DoFeedTray(int Flag)
{
    int Ret;

    if(Flag==0)
    {
        FeedTask=1;
        FeedDelay.Clear();
        return true;
    }

    switch(FeedTask)
    {
        case 1:
            FeedTask=10;
            break;

        case 10:
            RefreshStateFromSensors();
            if(bRearHasTray)
                return true;
            FeedTask=1000;
            break;

        case 1000:
            if(MoveEmptyY(Teach.EmptyCarFeedTrayYPosition))
                FeedTask=2000;
            break;

        case 2000:
            if(HSys.Cyn.C_Empty_LeanOnTray.Push() || IsSoftSimulate())
                FeedTask=3000;
            break;

        case 3000:
            if(HSys.Cyn.C_Empty_PushTray.Push() || IsSoftSimulate())
            {
                FeedDelay.Set(5);
                FeedDelay.On();
                FeedTask=3100;
            }
            break;

        case 3100:
            if(FeedDelay.Off())
            {
                if(HSys.Cyn.C_Empty_PushTray.OnSensor.Enable==false ||
                   HSys.Cyn.C_Empty_PushTray.OnSensor.IsOn() || IsSoftSimulate())
                {
                    FeedTask=4000;
                }
                else
                    FeedTask=3200;
            }
            break;

        case 3200:
            if(HSys.Cyn.C_Empty_PushTray.Pop() || IsSoftSimulate())
            {
                Ret=ShowMyError("Empty Push Tray Miss", K_RETRY);
                if(Ret==K_RETRY)
                    FeedTask=1000;
            }
            break;

        case 4000:
            if(MoveEmptyY(Teach.EmptyCarDischargeTrayYPosition))
            {
                bFrontHasTray=false;
                FeedTask=5000;
            }
            break;

        case 5000:
            if(HSys.Cyn.C_Empty_PushTray.Pop() || IsSoftSimulate())
                FeedTask=6000;
            break;

        case 6000:
            if(HSys.Cyn.C_Empty_LeanOnTray.Pop() || IsSoftSimulate())
                FeedTask=7000;
            break;

        case 7000:
            if(HSys.Sen.SnEmpty_OutputBottomHasTray.Enable==true &&
               HSys.Sen.SnEmpty_OutputBottomHasTray.IsOff() &&
               HSys.LastSet.iRealDummy!=DUMMY)
            {
                Ret=ShowMyError("Bottom Empty Tray Is Miss Error", K_SKIP|K_RETRY);
                if(Ret==K_RETRY)
                    FeedTask=1;
                if(Ret==K_SKIP)
                {
                    bRearHasTray=false;
                    bFrontHasTray=false;
                    FeedTask=8000;
                }
            }
            else
            {
                bRearHasTray=true;
                bBottomHasTray=true;
                FeedTask=13000;
            }
            break;

        case 8000:
            FeedTask=12000;
            break;

        case 12000:
            if(HSys.Sen.SnEmpty_OutputHasTray.Enable==true &&
               HSys.Sen.SnEmpty_OutputHasTray.IsOff() &&
               HSys.LastSet.iRealDummy!=DUMMY)
            {
                Ret=ShowMyError("Rear Empty Tray Is Miss Error", K_RETRY);
                if(Ret==K_RETRY)
                    FeedTask=1;
            }
            else
            {
                bRearHasTray=true;
                bBottomHasTray=false;
                FeedTask=13000;
            }
            break;

        case 13000:
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TEmptyModule::DoGoDownTray(int Flag)
{
    int Ret;

    if(Flag==0)
    {
        GoDownTask=1;
        GoDownDelay.Clear();
        return true;
    }

    switch(GoDownTask)
    {
        case 1:
            GoDownTask=10;
            break;

        case 10:
            GoDownTask=100;
            break;

        case 100:
            GoDownTask=1000;
            break;

        case 1000:
            HSys.Cyn.C_Empty_FrontRiseTray_1.On();
            GoDownTask=2000;
            break;

        case 2000:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.On();
                GoDownTask=3000;
            }
            break;

        case 3000:
            if(HSys.Cyn.C_Empty_FrontRiseTray_2.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Empty_FrontSeparateTray_1.On();
                GoDownDelay.Set(5);
                GoDownDelay.On();
                GoDownTask=4000;
            }
            break;

        case 4000:
            if(GoDownDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.Off();
                GoDownDelay.Set(5);
                GoDownDelay.On();
                GoDownTask=4100;
            }
            break;

        case 4100:
            if(GoDownDelay.Off())
                GoDownTask=5000;
            break;

        case 5000:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Empty_FrontSeparateTray_1.Off();
                GoDownDelay.Set(5);
                GoDownDelay.On();
                GoDownTask=6000;
            }
            break;

        case 6000:
            if(GoDownDelay.Off())
                GoDownTask=6500;
            break;

        case 6500:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.Pop() || IsSoftSimulate())
                GoDownTask=7000;
            break;

        case 7000:
            if(HSys.Sen.SnEmpty_InputHasTray.Enable==true &&
               HSys.Sen.SnEmpty_InputHasTray.IsOff() &&
               HSys.LastSet.iRealDummy!=DUMMY)
            {
                bFrontHasTray=false;
                Ret=ShowMyError("Front Empty Tray Is Miss Error", K_RETRY);
                if(Ret==K_RETRY)
                    GoDownTask=1;
            }
            else
            {
                bFrontHasTray=true;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TEmptyModule::DoGoUpTray(int Flag)
{
    if(Flag==0)
    {
        GoUpTask=1;
        GoUpDelay.Clear();
        return true;
    }

    switch(GoUpTask)
    {
        case 1:
            GoUpTask=10;
            break;

        case 10:
            GoUpTask=100;
            break;

        case 100:
            HSys.Cyn.C_Empty_FrontRiseTray_1.On();
            GoUpTask=200;
            break;

        case 200:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Empty_FrontSeparateTray_1.On();
                GoUpDelay.Set(5);
                GoUpDelay.On();
                GoUpTask=300;
            }
            break;

        case 300:
            if(GoUpDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.On();
                GoUpTask=400;
            }
            break;

        case 400:
            if(HSys.Cyn.C_Empty_FrontRiseTray_2.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Empty_FrontSeparateTray_1.Off();
                GoUpDelay.Set(5);
                GoUpDelay.On();
                GoUpTask=500;
            }
            break;

        case 500:
            if(GoUpDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.Off();
                if(HSys.Cyn.C_Empty_FrontRiseTray_1.IsOn() || IsSoftSimulate())
                    GoUpTask=600;
            }
            break;

        case 600:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.Pop() || IsSoftSimulate())
            {
                bFrontHasTray=false;
                GoUpTask=1000;
            }
            break;

        case 1000:
            RefreshStateFromSensors();
            if(bRearHasTray)
                GoUpTask=2000;
            else
                GoUpTask=9000;
            break;

        case 2000:
            if(MoveEmptyY(Teach.EmptyCarDischargeTrayYPosition))
                GoUpTask=3000;
            break;

        case 3000:
            if(HSys.Cyn.C_Empty_LeanOnTray.Push() || IsSoftSimulate())
                GoUpTask=4000;
            break;

        case 4000:
            if(HSys.Cyn.C_Empty_PushTray.Push() || IsSoftSimulate())
                GoUpTask=5000;
            break;

        case 5000:
            if(MoveEmptyY(Teach.EmptyCarFeedTrayYPosition))
                GoUpTask=6000;
            break;

        case 6000:
            if(HSys.Cyn.C_Empty_PushTray.Pop() || IsSoftSimulate())
                GoUpTask=7000;
            break;

        case 7000:
            if(HSys.Cyn.C_Empty_LeanOnTray.Pop() || IsSoftSimulate())
            {
                bFrontHasTray=true;
                bRearHasTray=false;
                bBottomHasTray=false;
                GoUpTask=8000;
            }
            break;

        case 8000:
            GoUpTask=9000;
            break;

        case 9000:
            GoUpTask=10000;
            break;

        case 10000:
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
//AI(general) 20260617 : Teach Advanced destacker test. Cylinder-only versions of
//GoDown/GoUp (no Y-motor / push / lean) so the front destacker rise+separate can be
//exercised in isolation. GoDown mirrors DoGoDownTray; GoUp mirrors DoGoUpTray 100-600.
bool TEmptyModule::TestGoDownTray(int Flag)
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
            HSys.Cyn.C_Empty_FrontRiseTray_1.On();
            TestDownTask=2000;
            break;

        case 2000:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.On();
                TestDownTask=3000;
            }
            break;

        case 3000:
            if(HSys.Cyn.C_Empty_FrontRiseTray_2.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Empty_FrontSeparateTray_1.On();
                TestDelay.Set(5);
                TestDelay.On();
                TestDownTask=4000;
            }
            break;

        case 4000:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.Off();
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
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Empty_FrontSeparateTray_1.Off();
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
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.Pop() || IsSoftSimulate())
            {
                TestDownTask=1;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TEmptyModule::TestGoUpTray(int Flag)
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
            HSys.Cyn.C_Empty_FrontRiseTray_1.On();
            TestUpTask=200;
            break;

        case 200:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Empty_FrontSeparateTray_1.On();
                TestDelay.Set(5);
                TestDelay.On();
                TestUpTask=300;
            }
            break;

        case 300:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.On();
                TestUpTask=400;
            }
            break;

        case 400:
            if(HSys.Cyn.C_Empty_FrontRiseTray_2.IsOn() || IsSoftSimulate())
            {
                HSys.Cyn.C_Empty_FrontSeparateTray_1.Off();
                TestDelay.Set(5);
                TestDelay.On();
                TestUpTask=500;
            }
            break;

        case 500:
            if(TestDelay.Off())
            {
                HSys.Cyn.C_Empty_FrontRiseTray_2.Off();
                if(HSys.Cyn.C_Empty_FrontRiseTray_1.IsOn() || IsSoftSimulate())
                    TestUpTask=600;
            }
            break;

        case 600:
            if(HSys.Cyn.C_Empty_FrontRiseTray_1.Pop() || IsSoftSimulate())
            {
                TestUpTask=1;
                return true;
            }
            break;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TEmptyModule::IsFrontHasTray()
{
    RefreshStateFromSensors();
    return bFrontHasTray;
}
//---------------------------------------------------------------------------
bool TEmptyModule::IsRearHasTray()
{
    RefreshStateFromSensors();
    return bRearHasTray;
}
//---------------------------------------------------------------------------
bool TEmptyModule::IsBottomHasTray()
{
    RefreshStateFromSensors();
    return bBottomHasTray;
}
//---------------------------------------------------------------------------
bool TEmptyModule::IsReturnTrayRequested()
{
    return bReturnTray;
}
//---------------------------------------------------------------------------
void TEmptyModule::SetRearHasTray(bool bHasTray)
{
    bRearHasTray=bHasTray;
    if(bHasTray==false)
        bBottomHasTray=false;
}
//---------------------------------------------------------------------------
void TEmptyModule::RequestReturnTray()
{
    bReturnTray=true;
    bTrayXToEmptyFinish=false;
}
//---------------------------------------------------------------------------
void TEmptyModule::NotifyTrayXToEmptyFinish()
{
    bTrayXToEmptyFinish=true;
    bRearHasTray=true;
}
//---------------------------------------------------------------------------
void InitializeEmptyModule()
{
    if(EmptyModule==NULL)
        EmptyModule=new TEmptyModule;
}
//---------------------------------------------------------------------------
void ShutdownEmptyModule()
{
    delete EmptyModule;
    EmptyModule=NULL;
}
//---------------------------------------------------------------------------