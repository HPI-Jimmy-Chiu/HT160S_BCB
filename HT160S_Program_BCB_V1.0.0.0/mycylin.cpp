#include "IncludeAllHeader.h"       //Dell 將.h統一,可加速build
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "mycylin.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
class TMyCylinder Cylinder[MaxCylinderItem];
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260623 : shared "ready" predicates (decl in mycylin.h).
//  Moved here from aAuto1To6.cpp static helpers so all cart modules share them.
bool IsSensorOnReady(TMySensor *Sensor)
{
    if(Sensor==NULL || Sensor->Enable==false)
        return true;
    return Sensor->IsOn();
}
//---------------------------------------------------------------------------
bool IsCylinderOnReady(TMyCylinder *Cylinder, bool bSoftSimulate)
{
    if(Cylinder==NULL)
        return false;
    if(bSoftSimulate)
        return true;
    if(Cylinder->OnSensor.Enable==false)
        return true;
    return Cylinder->OnSensor.IsOn();
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260623 : standardized dual-cylinder tray clamp shared
//  by all cart modules. Motion fixed: lean-stop (Lean) first, then push (Push).
//  The per-cylinder guard mirrors Color's PushCylinder: skip the stroke when
//  simulating or the cylinder is disabled. SettleTicks>0 -> settle delay then
//  confirm Push.OnSensor, Pop on miss. Display/alarm stays in the caller.
int DoClampTray(TMyCylinder &Lean, TMyCylinder &Push, int &SubTask,
                HTimer &Delay, bool bSoftSimulate, int SettleTicks)
{
    switch(SubTask)
    {
        case 0:   // lean-stop first
            if(bSoftSimulate || Lean.Enable==false || Lean.Push())
                SubTask=10;
            break;

        case 10:  // push last
            if(bSoftSimulate || Push.Enable==false || Push.Push())
            {
                if(SettleTicks<=0)
                {
                    SubTask=0;
                    return 1;
                }
                Delay.Set(SettleTicks);
                Delay.On();
                SubTask=20;
            }
            break;

        case 20:  // settle then confirm push reached its on-sensor
            if(Delay.Off())
            {
                if(IsCylinderOnReady(&Push, bSoftSimulate))
                {
                    SubTask=0;
                    return 1;
                }
                SubTask=30;
            }
            break;

        case 30:  // miss: retract push, report so caller can alarm/retry
            if(bSoftSimulate || Push.Enable==false || Push.Pop())
            {
                SubTask=0;
                return 2;
            }
            break;
    }
    return 0;
}
//---------------------------------------------------------------------------
static void SetCylinderAlarm(int AlarmCode, AnsiString sFrom="")
{
    //AI(HT160S-Maintainer) 20260603 : raise-hand to central dispatch (HAlarm). sFrom carries the
    //caller Func/Case context, shown later in the note remark field via HSys.mapAlarmContext.
    if(AlarmCode==0)
        return;
    if(Alarm==NULL)
    {
        //fallback: central object not created yet -> show directly (same behavior as before)
        ShowSystemError(AnsiString(AlarmCode), K_RETRY, 0, sFrom);
        return;
    }
    HSys.mapAlarmContext[AlarmCode]=sFrom;
    Alarm->Set(AlarmCode);
}
//---------------------------------------------------------------------------
static void ClearCylinderAlarm(int AlarmCode)
{
    //AI(HT160S-Maintainer) 20260603 : clear this code from the central queue and drop its context
    if(AlarmCode==0)
        return;
    if(Alarm!=NULL)
        Alarm->Clear(AlarmCode);
    HSys.mapAlarmContext.erase(AlarmCode);
}
//---------------------------------------------------------------------------
__fastcall TMyCylinder::TMyCylinder()
{
    Enable=false;
    EnableAtDataBase=false;
    bInitialOk=false;
    Task=1;
    OnOff=2;
    OnAlarmCode=0;
    OffAlarmCode=0;
    OnAlarmTime=0;
    OffAlarmTime=0;
    OnDelayTime=0;
    OffDelayTime=0;
    Tag=0;
    CylinderName="";
    OnSensorName="";
    OffSensorName="";
    FlushPanelName="";
    for(int i=0; i<eCynErrTotal; i++)
        ErrorName[i]="";
    PTempWinCtrl=NULL;
    iOnLeft=0;
    iOnTop=0;
    iOffLeft=0;
    iOffTop=0;
}
//---------------------------------------------------------------------------
bool TMyCylinder::Reset()
{
    Task=1;
    return true;
}
//---------------------------------------------------------------------------
bool TMyCylinder::GetOutBit()
{
    return Switch.OutValue;
}
//---------------------------------------------------------------------------
bool TMyCylinder::IsOn()
{
    return OnSensor.IsOn();
}
//---------------------------------------------------------------------------
bool TMyCylinder::IsOff()
{
    return OffSensor.IsOn();
}
//---------------------------------------------------------------------------
void TMyCylinder::On()
{
    Switch.On();
    Task=1;
    UpdateSimulateCompomentPosition(true);
}
//---------------------------------------------------------------------------
void TMyCylinder::Off()
{
    Task=1;
    Switch.Off();
    UpdateSimulateCompomentPosition(false);
}
//---------------------------------------------------------------------------
bool TMyCylinder::Push()
{
    #ifdef SOFT_SIMULATE
    return true;
    #else
    if(Enable==false && Task==1)
        Task=100;

    Switch.On();
    ClearCylinderAlarm(OnAlarmCode);
    if(Task==1 || Task==2)
    {
        //AI(HT160S-Maintainer) 20260624 : DUMMY skips the cylinder in-position confirm +
        //timeout alarm so the dry-run can flow (user 2026-06-24). Switch.On() is still
        //driven above; DUMMY just takes the no-sensor path here, so it never parks at
        //Task=50 with an armed OnAlarmTime timer that the aTrayArm ||IsSoftSimulate()
        //skip would abandon -> stale "can not on" alarm next cycle. HAS_TRAY/REALLY
        //still confirm + alarm. Re-adds the iRealDummy!=DUMMY gate removed 2026-06-22
        //(user reversed that call; the clamp has a real sensor the DUMMY bench cannot satisfy).
        if(OnSensor.Enable==true && HSys.LastSet.iRealDummy!=DUMMY)
        {
            if(OnSensor.IsOn())
            {
                Delay.Clear();
                Delay.SetMS(OnDelayTime);
                Delay.On();
                Task=100;
            }
            else
            {
                Delay.Clear();
                Delay.SetMS(OnAlarmTime);
                Delay.On();
                Task=50;
            }
        }
        else
        {
            Delay.Clear();
            Delay.SetMS(OnDelayTime);
            Delay.On();
            Task=100;
        }
    }

    if(Task==50)
    {
        if(OnSensor.IsOn())
        {
            Delay.Clear();
            Delay.SetMS(OnDelayTime);
            Delay.On();
            Task=100;
        }
        else
        {
            if(Delay.Off())
            {
                Task=1;
                UpdateSimulateCompomentPosition(true);
                SetCylinderAlarm(OnAlarmCode, AnsiString().sprintf("Cylinder=%s Func=Push", CylinderName.c_str()));
                return false;
            }
            return false;
        }
    }

    if(Task>=100)
    {
        switch(Task)
        {
            case 100:
                if(OnDelayTime==0)
                {
                    Task=1;
                    ClearCylinderAlarm(OnAlarmCode);
                    UpdateSimulateCompomentPosition(true);
                    return true;
                }
                Delay.Clear();
                Delay.SetMS(OnDelayTime);
                Delay.On();
                Task=101;
            case 101:
                if(Delay.Off())
                {
                    ClearCylinderAlarm(OnAlarmCode);
                    UpdateSimulateCompomentPosition(true);
                    Task=1;
                    return true;
                }
                return false;
        }
    }

    Task=1;
    ClearCylinderAlarm(OnAlarmCode);
    UpdateSimulateCompomentPosition(true);
    return true;
    #endif
}
//---------------------------------------------------------------------------
bool TMyCylinder::Pop()
{
    #ifdef SOFT_SIMULATE
    return true;
    #else
    if(Enable==false && Task==1)
        Task=100;

    Switch.Off();
    ClearCylinderAlarm(OffAlarmCode);
    if(Task==1 || Task==2)
    {
        //AI(HT160S-Maintainer) 20260624 : DUMMY skips the cylinder in-position confirm +
        //timeout alarm so the dry-run can flow (user 2026-06-24). Switch.Off() is still
        //driven above; DUMMY just takes the no-sensor path here, so it never parks at
        //Task=50 with an armed OffAlarmTime timer that the aTrayArm ||IsSoftSimulate()
        //skip would abandon -> stale "can not off" alarm next cycle (the 40020 seen on
        //C_TrayArm_FrontClamp). HAS_TRAY/REALLY still confirm + alarm. Re-adds the
        //iRealDummy!=DUMMY gate removed 2026-06-22 (user reversed that call 2026-06-24).
        if(OffSensor.Enable==true && HSys.LastSet.iRealDummy!=DUMMY)
        {
            if(OffSensor.IsOn())
            {
                Delay.Clear();
                Delay.SetMS(OffDelayTime);
                Delay.On();
                Task=100;
            }
            else
            {
                Delay.Clear();
                Delay.SetMS(OffAlarmTime);
                Delay.On();
                Task=50;
            }
        }
        else
        {
            Delay.Clear();
            Delay.SetMS(OffDelayTime);
            Delay.On();
            Task=100;
        }
    }

    if(Task==50)
    {
        if(OffSensor.IsOn())
        {
            Delay.Clear();
            Delay.SetMS(OffDelayTime);
            Delay.On();
            Task=100;
        }
        else
        {
            if(Delay.Off())
            {
                Task=1;
                SetCylinderAlarm(OffAlarmCode, AnsiString().sprintf("Cylinder=%s Func=Pop", CylinderName.c_str()));
                UpdateSimulateCompomentPosition(false);
                return false;
            }
            return false;
        }
    }

    if(Task>=100)
    {
        switch(Task)
        {
            case 100:
                if(OffDelayTime==0)
                {
                    ClearCylinderAlarm(OffAlarmCode);
                    UpdateSimulateCompomentPosition(false);
                    Task=1;
                    return true;
                }
                Delay.Clear();
                Delay.SetMS(OffDelayTime);
                Delay.On();
                Task=101;
            case 101:
                if(Delay.Off())
                {
                    Task=1;
                    ClearCylinderAlarm(OffAlarmCode);
                    UpdateSimulateCompomentPosition(false);
                    return true;
                }
                return false;
        }
    }

    Task=1;
    ClearCylinderAlarm(OffAlarmCode);
    UpdateSimulateCompomentPosition(false);
    return true;
    #endif
}
//---------------------------------------------------------------------------
void TMyCylinder::UpdateSimulateCompomentPosition(bool bFlag)
{
    if(PTempWinCtrl!=NULL)
    {
        if(bFlag)
        {
            PTempWinCtrl->Left=iOnLeft;
            PTempWinCtrl->Top=iOnTop;
        }
        else
        {
            PTempWinCtrl->Left=iOffLeft;
            PTempWinCtrl->Top=iOffTop;
        }
    }
}
//---------------------------------------------------------------------------
void TMyCylinder::SetSimulateCompoment(TObject *PCtrl, TAnchorKind Alignment, int simuStart, int simuEnd)
{
    PTempWinCtrl=dynamic_cast<TControl *>(PCtrl);
    if(PTempWinCtrl!=NULL)
    {
        iOnLeft =(Alignment==akLeft || Alignment==akRight )?PTempWinCtrl->Left:simuStart;
        iOffLeft=(Alignment==akLeft || Alignment==akRight )?PTempWinCtrl->Left:simuEnd;
        iOnTop  =(Alignment==akTop  || Alignment==akBottom)?PTempWinCtrl->Top:simuStart;
        iOffTop =(Alignment==akTop  || Alignment==akBottom)?PTempWinCtrl->Top:simuEnd;
        UpdateSimulateCompomentPosition(false);
    }
}
//---------------------------------------------------------------------------
void InitialCylinderName()
{
}
//---------------------------------------------------------------------------
