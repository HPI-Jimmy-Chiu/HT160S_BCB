#include "IncludeAllHeader.h"       //Dell 將.h統一,可加速build
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "mycylin.h"
#include "GeneralSetting.h"
#include "cStepTrace.h"            //AI(ht160s-alarm-trace) 20260630 : GetProcStep breadcrumb
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
//AI(ht160s-pause-cylinder) 20260731 : the global Cylinder[MaxCylinderItem] array that
//lived here is DELETED. It was never initialised and, after the pause/resume loops in
//csystem.cpp were pointed at HSys.CynPtr where the real cylinders live, it had zero
//readers project-wide. Keeping a plausible-looking decoy next to the real thing is what
//made the pause bug survive this long. Every cylinder is HSys.Cyn / HSys.CynPtr.
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
//AI(ht160s-clampgrip) 20260806 : tri-state "is this clamp gripping a tray" verdict; see the
//mechanism note on the declaration in mycylin.h. Deliberately NOT folded into
//IsCylinderOnReady : that one answers "may I proceed" and returns TRUE for a disabled point,
//which is exactly the wrong default for a fault detector. Here a point we cannot read returns
//-1 (no verdict) so callers can tell "no tray" apart from "no evidence".
//The GetOutBit gate matters : with the clamp retracted the On reed is legitimately dark, and
//reporting that as "tray missing" would fire on every released carriage on the machine.
//AI(auto-empty-car) 20260908 : the "1 = gripping" reading held for the Loader/Empty/Color clamps
//(20260805 on-site) but NOT for C_Auto1..6_PushTray - that On reed lights on an empty clamp
//(owner bench test 20260908). The 2026-09 mechanical rework fixes that by moving the band,
//which INVERTS the reading on a reworked carriage; see IsClampNewGeometry below.
bool IsClampNewGeometry(int iGeomIdx)
{
    //AI(ht160s-clamp-geom) 20260916 : ONE switch for all ten clamps (owner ruling) -
    //the mechanical rework lands on every carriage together, so there is no staging to
    //express and a per-carriage list would only be a way to get it wrong. iGeomIdx is
    //still the carriage identity (uHome HomeParkCarriage order) and still range-checked,
    //so a cylinder that is not one of the ten can never take the new reading.
    if(GeneralSetting.bTwoBandClamp==false)
        return false;                    //feature off : every carriage reads the OLD way
    if(iGeomIdx<0 || iGeomIdx>=10)
        return false;                    //not a tray clamp, or an index we cannot place
    return true;
}
//---------------------------------------------------------------------------
bool IsTrayClampDeparted(TMyCylinder *Cylinder)
{
    if(Cylinder==NULL)
        return false;
    if(Cylinder->OffSensor.Enable==false)
        return true;                     //cannot read the seat reed -> never block
    return (Cylinder->OffSensor.IsOn()==false);
}
//---------------------------------------------------------------------------
int GetTrayClampVerdict(TMyCylinder *Push, bool bSoftSimulate)
{
    if(bSoftSimulate)
        return -1;                       //no IO card; every read is meaningless
    if(Push==NULL || Push->Enable==false)
        return -1;
    if(Push->OnSensor.Enable==false)
        return -1;                       //cannot read the grip reed -> no verdict
    if(Push->GetOutBit()==false)
        return -1;                       //not clamped at all -> says nothing about a tray
    if(IsClampNewGeometry(Push->iClampGeomIdx)==false)
    {
        if(Push->OnSensor.IsOn())
            return 1;                    //OLD : hook stopped against a tray edge : gripping
        return 0;                        //OLD : commanded out but over-travelled : tray gone
    }
    if(Push->OnSensor.IsOn())
        return 0;                        //NEW : reached the end stop : clamped on NOTHING
    if(Push->OffSensor.Enable==false)
        return -1;                       //NEW : both-dark is ambiguous without the seat reed
    if(IsTrayClampDeparted(Push)==false)
        return 0;                        //NEW : still seated home : the stroke never happened
    return 1;                            //NEW : left the seat, never reached the stop : a tray
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260623 : standardized dual-cylinder tray clamp shared
//  by all cart modules. Motion fixed: lean-stop (Lean) first, then push (Push).
//  The per-cylinder guard mirrors Color's PushCylinder: skip the stroke when
//  simulating or the cylinder is disabled. SettleTicks>0 -> settle delay then
//  confirm Push.OnSensor, Pop on miss. Display/alarm stays in the caller.
int DoClampTray(TMyCylinder &Lean, TMyCylinder &Push, int &SubTask,
                HTimer &Delay, bool bSoftSimulate, int SettleMs)
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
                if(SettleMs<=0)
                {
                    SubTask=0;
                    return 1;
                }
                Delay.SetMS(SettleMs);
                Delay.On();
                SubTask=20;
            }
            break;

        case 20:  // settle then confirm push reached its on-sensor
            if(Delay.Off())
            {
                //AI(ht160s-clamp-geom) 20260916 : ask the tray question, not the
                //in-position question - 0 is the ONLY answer that means "clamped on
                //nothing", which is what this case exists to catch. 1 and -1 both pass
                //(-1 is never evidence of a miss). On the OLD geometry this is
                //equivalent to the IsCylinderOnReady test it replaces at every
                //reachable point, because case 20 is only entered after Push() succeeded.
                if(GetTrayClampVerdict(&Push, bSoftSimulate)!=0)
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
//AI(ht160s-maintainer) 20260625 : short-term mechanical interlock (see mycylin.h /
//  General.ini [Safety] FrontSeparateInterlock). GetOutBit() reflects the commanded
//  output in both real and SOFT_SIMULATE builds, so this serializes in sim too.
bool IsFrontSeparateBlockedBy(TMyCylinder &Other)
{
    if(GeneralSetting.bFrontSeparateInterlock==false)
        return false;
    return Other.GetOutBit();
}
//---------------------------------------------------------------------------
static void SetCylinderAlarm(int AlarmCode, AnsiString sFrom="")
{
    //AI(HT160S-Maintainer) 20260603 : raise-hand to central dispatch (HAlarm). sFrom carries the
    //caller Func/Case context, shown later in the note remark field via HSys.mapAlarmContext.
    if(AlarmCode==0)
        return;
    //AI(ht160s-alarm-trace) 20260630 : append the live process breadcrumb (which action +
    //Task case was driving this cylinder) so the alarm context names the FLOW, not just the
    //cylinder. Captured HERE while the driver is live; the queued ProcessAlarm display is one
    //cycle later with the driving stack already unwound.
    AnsiString sStep=GetProcStep();
    if(sStep!="")
        sFrom=sFrom+" | Step="+sStep;
    if(Alarm==NULL)
    {
        //fallback: central object not created yet -> show directly (same behavior as before)
        ShowSystemError(AnsiString(AlarmCode), K_RETRY, 0, sFrom);
        return;
    }
    //AI(ht160s-alarm-trace) 20260630 : timestamped raise line into the EventLog at the moment
    //of raise so the shipped log carries an ordered "...module steps... -> ALARM raised" trail
    //for post-mortem without breakpoints. GetStat guards double-logging while already queued.
    if(Alarm->GetStat(AlarmCode)==false)
        RecordProcess(AnsiString().sprintf("ALARM raised %d : %s", AlarmCode, sFrom.c_str()));
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
    iLastDir=0;   //AI(ht160s-cylinder-dir) 20260731
    OnOff=2;
    OnAlarmCode=0;
    OffAlarmCode=0;
    OnAlarmTime=0;
    OffAlarmTime=0;
    OnDelayTime=0;
    OffDelayTime=0;
    Tag=0;
    bTrayClamp=false;      //AI(ht160s-clamp-geom) 20260916
    iClampGeomIdx=-1;      //AI(ht160s-clamp-geom) 20260916
    bSeatConfirmed=false;  //AI(clamp-review-B) 20260916
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
    //AI(ht160s-cylinder-dir) 20260731 : Task is SHARED by Push() and Pop(), and BOTH the
    //in-position confirm and the timeout watchdog live only inside the if(Task==1) arm block
    //below. A stroke abandoned mid-flight (the ladder was preempted, HOMEd, or a modal aborted
    //it) leaves Task at 50 or 101; driving the OTHER direction then skipped the arm entirely
    //and fell into this direction's case 101 on the PREVIOUS stroke's stale Delay, returning
    //true without ever reading the reed and without arming any alarm. Re-arm on a direction
    //change. A same-direction re-entry is untouched, so no stroke that works today changes.
    if(iLastDir!=1)
    {
        Task=1;
        Delay.Clear();
        iLastDir=1;
    }
    if(Enable==false && Task==1)
        Task=100;

    //AI(clamp-review-B) 20260916 : sample the commanded state BEFORE Switch.On() - it is
    //the only way to tell a fresh stroke (clamp was retracted, so the piston MUST be on
    //the seat right now) from a re-push of an already extended clamp (seat legitimately
    //dark, nothing left to depart from).
    bool bWasCommandedOut=GetOutBit();
    Switch.On();
    ClearCylinderAlarm(OnAlarmCode);
    if(Task==1)
    {
        //AI(HT160S-Maintainer) 20260624 : DUMMY skips the cylinder in-position confirm +
        //timeout alarm so the dry-run can flow (user 2026-06-24). Switch.On() is still
        //driven above; DUMMY just takes the no-sensor path here, so it never parks at
        //Task=50 with an armed OnAlarmTime timer that the aTrayArm ||IsSoftSimulate()
        //skip would abandon -> stale "can not on" alarm next cycle. HAS_TRAY/REALLY
        //still confirm + alarm. Re-adds the iRealDummy!=DUMMY gate removed 2026-06-22
        //(user reversed that call; the clamp has a real sensor the DUMMY bench cannot satisfy).
        //AI(ht160s-clamp-geom) 20260916 : NEW-geometry tray clamp. A loaded clamp rests
        //BETWEEN the two reed bands, so there is no in-position reed to wait for and the
        //old On-reed confirm would time out on every good tray. The remaining positive
        //evidence is DEPARTURE from the retracted seat, then a settle. Whether a tray is
        //in there is a separate question, answered by GetTrayClampVerdict at the check
        //points - Push() only ever says "the stroke finished". Same iRealDummy gate as
        //the branch below, so a DUMMY bench still skips confirm + watchdog.
        if(bTrayClamp && IsClampNewGeometry(iClampGeomIdx) &&
           OffSensor.Enable==true && HSys.LastSet.iRealDummy!=DUMMY)
        {
            //AI(clamp-review-B) 20260916 : a stroke that starts from the retracted state
            //must be able to SEE the seat reed lit, or its later darkness proves nothing.
            //A re-push of an already-out clamp starts with the seat legitimately dark and
            //is accepted as confirmed. If a fresh stroke cannot see the seat, Task 60 will
            //never accept the departure and the existing OnAlarmTime watchdog raises the
            //cylinder alarm - the SAME fail-safe direction the old On-reed path had.
            bSeatConfirmed=bWasCommandedOut ? true : OffSensor.IsOn();
            Delay.Clear();
            Delay.SetMS(OnAlarmTime);
            Delay.On();
            Task=60;
        }
        else if(OnSensor.Enable==true && HSys.LastSet.iRealDummy!=DUMMY)
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

    //AI(ht160s-clamp-geom) 20260916 : NEW-geometry confirm, cases 60 and 70. 60 waits for
    //the piston to leave the retracted seat under the SAME OnAlarmTime watchdog and the
    //SAME OnAlarmCode as the old path (the ALID catalogue is a shipped SECS contract and
    //must not move); only the alarm context text says what actually failed. 70 is the
    //settle, after which the shared Task>=100 tail below clears the alarm and returns true.
    if(Task==60)
    {
        //AI(clamp-review-B) 20260916 : keep looking for the seat until the watchdog gives
        //up - a reed that is merely slow still confirms, a reed that is dead never does.
        if(bSeatConfirmed==false && OffSensor.IsOn())
            bSeatConfirmed=true;
        if(bSeatConfirmed && IsTrayClampDeparted(this))
        {
            Delay.Clear();
            Delay.SetMS(GeneralSetting.iTrayClampSettleMs);
            Delay.On();
            Task=70;
        }
        else
        {
            if(Delay.Off())
            {
                Task=1;
                UpdateSimulateCompomentPosition(true);
                SetCylinderAlarm(OnAlarmCode, AnsiString().sprintf("Cylinder=%s Func=Push (%s)", CylinderName.c_str(),
                                 bSeatConfirmed ? "never left the retracted seat" : "retracted seat reed never read ON - check the _Off reed"));
                return false;
            }
            return false;
        }
    }

    if(Task==70)
    {
        if(Delay.Off()==false)
            return false;
        Task=100;
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
    //AI(ht160s-cylinder-dir) 20260731 : Task is SHARED by Push() and Pop(), and BOTH the
    //in-position confirm and the timeout watchdog live only inside the if(Task==1) arm block
    //below. A stroke abandoned mid-flight (the ladder was preempted, HOMEd, or a modal aborted
    //it) leaves Task at 50 or 101; driving the OTHER direction then skipped the arm entirely
    //and fell into this direction's case 101 on the PREVIOUS stroke's stale Delay, returning
    //true without ever reading the reed and without arming any alarm. Re-arm on a direction
    //change. A same-direction re-entry is untouched, so no stroke that works today changes.
    if(iLastDir!=2)
    {
        Task=1;
        Delay.Clear();
        iLastDir=2;
    }
    if(Enable==false && Task==1)
        Task=100;

    Switch.Off();
    ClearCylinderAlarm(OffAlarmCode);
    if(Task==1)
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
