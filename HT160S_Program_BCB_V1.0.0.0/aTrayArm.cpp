//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop

#include "aTrayArm.h"
#include "database.h"
#include "uteach.h"
#include "mymessbox.h"
#include "aEmpty.h"
#include "aAuto1To6.h"
#include "aColor.h"            //AI(HT160S-Maintainer) 20260605 : AMR identity-tray source
#include "aLoader.h"           //AI(HT160S-Maintainer) 20260606 : Loader rear empty-tray recovery source
#include "GeneralSetting.h"    //AI(HT160S-Maintainer) 20260605 : GeneralSetting.bUseAMR mode switch
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TTrayArmModule *TrayArmModule=NULL;
//---------------------------------------------------------------------------
TTrayArmModule::TTrayArmModule()
{
    Status=TAS_IDLE;
    Job=TAJOB_NONE;
    PickTask=1;
    PlaceTask=1;
    iAutoTarget=-1;
    iDeliverKind=eTrayKindNormal;
    PlaceDest=TAPLACE_AUTO;
    bCleanOutFinish=true;
    bTrayFeedFinish=true;
    InitialFlag();
}
//---------------------------------------------------------------------------
void TTrayArmModule::InitialFlag(bool bKeepMaterial)
{
    bHasTray=false;
    if(HSys.VMot.MMTrayArmX!=NULL)
        bHasTray=HSys.VMot.MMTrayArmX->fHasTray;
    PickTask=1;
    PlaceTask=1;
    bCleanOutFinish=true;
    bTrayFeedFinish=true;
    ArmDelay.Clear();
    //AI(HT160S-Maintainer) 20260612 : recoverable home while a tray is in hand. Keep the
    //delivery job + destination (Auto target, AMR kind, 2D TrayID, place dest) so the arm
    //resumes placing the SAME tray after home instead of losing where it must go. The
    //clamps are kept closed during the home (see uHome ProcessMotorHome) so the tray rides
    //up with the head and is never dropped. Only the transient pick/place sub-tasks above
    //are restarted.
    if(bKeepMaterial && bHasTray)
    {
        Status=TAS_CARRYING;
        return;
    }
    Status=TAS_IDLE;
    Job=TAJOB_NONE;
    iAutoTarget=-1;
    iDeliverKind=eTrayKindNormal;
    iDeliverTrayID="";
    PlaceDest=TAPLACE_AUTO;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::HasTray()
{
    if(HSys.VMot.MMTrayArmX!=NULL)
        bHasTray=HSys.VMot.MMTrayArmX->fHasTray;
    return bHasTray;
}
//---------------------------------------------------------------------------
int TTrayArmModule::GetStatus()
{
    return Status;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::IsCleanOutFinish()
{
    //AI(HT160S-Maintainer) 20260605 : TrayArm only transports EMPTY trays, it never
    //holds IC, so it can never block an IC drain-out. CleanOut finish is always true
    //for this module.
    return bCleanOutFinish;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::IsTrayFeedFinish()
{
    //AI(HT160S-Maintainer) 20260605 : TrayFeed empty-tray evacuation needs Loader/
    //EmptyTray recovery handshakes that do not exist yet (see DecideJob TODO). Report
    //finished so this incomplete path never gates the run-mode revert. Wire real state
    //here once the recovery flow is implemented.
    return bTrayFeedFinish;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::IsSoftSimulate()
{
    #ifdef SOFT_SIMULATE
    return true;
    #else
    return (HSys.LastSet.iRealDummy==DUMMY);
    #endif
}
//---------------------------------------------------------------------------
bool TTrayArmModule::MoveTrayArmX(int Position)
{
    if(HSys.Mot.MTrayArmX==NULL)
        return false;
    if(HSys.Mot.MTrayArmX->CheckSoftLimit(Position)==false)
    {
        ShowMyMessage("Tray Arm X motor will out of limit");
        return false;
    }
    return HSys.Mot.MTrayArmX->MotorMove(Position);
}
//---------------------------------------------------------------------------
bool TTrayArmModule::DoZUp()
{
    //AI(HT160S-Maintainer) 20260605 : dual-coil Z, drop the down coil before driving up.
    HSys.Cyn.C_TrayArmZ_Down.Off();
    return (HSys.Cyn.C_TrayArmZ_Up.Push() || IsSoftSimulate());
}
//---------------------------------------------------------------------------
bool TTrayArmModule::DoZDown()
{
    HSys.Cyn.C_TrayArmZ_Up.Off();
    return (HSys.Cyn.C_TrayArmZ_Down.Push() || IsSoftSimulate());
}
//---------------------------------------------------------------------------
int TTrayArmModule::GetAutoX(int Index)
{
    switch(Index)
    {
        case 0: return Teach.TrayXArmToAuto1XPosition;
        case 1: return Teach.TrayXArmToAuto2XPosition;
        case 2: return Teach.TrayXArmToAuto3XPosition;
        case 3: return Teach.TrayXArmToAuto4XPosition;
        case 4: return Teach.TrayXArmToAuto5XPosition;
        case 5: return Teach.TrayXArmToAuto6XPosition;
    }
    return Teach.TrayXArmToAuto1XPosition;
}
//---------------------------------------------------------------------------
int TTrayArmModule::GetColorX()
{
    //AI(HT160S-Maintainer) 20260605 : AMR identity-tray pickup position at Color station.
    return Teach.TrayXArmToColorXPosition;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::IsPickFromColor()
{
    //AI(HT160S-Maintainer) 20260605 : only the AMR identity tray (stack bottom, carries
    //the 2D TrayID) is picked from Color. Cover and normal trays come from EmptyTray.
    return (Job==TAJOB_AMR_SUPPLY && iDeliverKind==eTrayKindIdentity);
}
//---------------------------------------------------------------------------
int TTrayArmModule::DecideJob()
{
    //AI(HT160S-Maintainer) 20260606 : Priority 1 : recover an empty tray stranded at the
    //Loader rear. The Loader only discharges a tray to its rear after its own gate proves
    //the tray has no IC (DoLoader case3000 rejects trays that still carry IC), so
    //IsRearHasTray()==true here means an EMPTY tray is waiting to be cleared. Clearing it
    //first (as the old 160 did) keeps the Loader free to feed the next source tray. The
    //destination (supply an Auto vs recycle to EmptyTray) is decided after the pick, in
    //DecidePlaceDestAfterPick(), so the arm reacts to the live Auto demand at that moment.
    if(LoaderModule!=NULL && LoaderModule->IsRearHasTray())
    {
        iAutoTarget=-1;
        iDeliverKind=eTrayKindNormal;
        return TAJOB_LOADER_RECOVERY;
    }

    //AI(HT160S-Maintainer) 20260605 : AMR mode builds each Auto output stack in a fixed
    //order : tray[0]=identity (picked from Color, carries the 2D TrayID, no IC),
    //tray[1]=cover (empty tray from EmptyTray, no IC), tray[2..]=normal work trays.
    //The needed kind for the next delivery is derived from the Auto car's tray count.
    if(GeneralSetting.bUseAMR)
    {
        if(AutoModule==NULL)
            return TAJOB_NONE;
        //AI(general) 20260608 : Stage1 demand-driven - ask the Auto stations which one
        //wants a tray and what kind. No requesting Auto -> the arm stays idle (no
        //needless shuttling). The needed kind still follows the AMR stack order.
        int kind=eTrayReqNone;
        int idx=AutoModule->FindTrayRequestAuto(kind);
        if(idx<0)
            return TAJOB_NONE;             //no Auto wants a tray, nothing to deliver
        if(kind==eTrayKindIdentity)
        {
            //Identity tray comes from Color. Only start when Color presents one at the
            //pickup; otherwise ask Color to supply and wait (no deadlock : Color refills).
            if(ColorModule==NULL)
                return TAJOB_NONE;
            if(ColorModule->IsTrayReady()==false)
            {
                ColorModule->RequestSupplyTray();
                return TAJOB_NONE;
            }
        }
        else
        {
            //Cover / normal trays come from the EmptyTray rear pickup.
            if(EmptyModule==NULL || EmptyModule->IsRearHasTray()==false)
                return TAJOB_NONE;
        }
        iAutoTarget=idx;
        iDeliverKind=kind;
        return TAJOB_AMR_SUPPLY;
    }

    //AI(HT160S-Maintainer) 20260605 : Supply path : deliver an empty tray from the
    //EmptyTray rear pickup position to an Auto station whose rear is free. APIs and
    //Teach coordinates for this path are complete, and it has no cross-module deadlock
    //(EmptyTray auto-refills its rear after each pick), so it is safe to dispatch.
    if(EmptyModule!=NULL && AutoModule!=NULL)
    {
        if(EmptyModule->IsRearHasTray())
        {
            //AI(general) 20260608 : Stage1 demand-driven - only fetch an empty tray when
            //an Auto actually wants one (Normal-mode request kind is always Normal).
            int kind=eTrayReqNone;
            int idx=AutoModule->FindTrayRequestAuto(kind);
            if(idx>=0)
            {
                iAutoTarget=idx;
                return TAJOB_EMPTYTRAY_TO_AUTO;
            }
        }
    }

    //AI(HT160S-Maintainer) 20260606 : Loader empty-tray recovery is now dispatched as
    //Priority 1 at the top of this function (TAJOB_LOADER_RECOVERY). Its two missing
    //handshakes are now satisfied : (1) the Loader only parks EMPTY trays at its rear
    //(DoLoader rejects trays that still hold IC before discharge), so IsRearHasTray() here
    //is safe to pick; (2) recycling to EmptyTray uses the RequestReturnTray() ->
    //IsRearHasTray()==false -> NotifyTrayXToEmptyFinish() handshake, so EmptyTray first
    //goes up to free its rear before accepting the returned tray (no rear contention).
    return TAJOB_NONE;
}
//---------------------------------------------------------------------------
int TTrayArmModule::GetPickSourceX()
{
    //AI(HT160S-Maintainer) 20260606 : pick X depends on the source of the current job.
    //Loader recovery picks from the Loader rear; the AMR identity tray from Color; every
    //other job picks from the EmptyTray rear.
    if(Job==TAJOB_LOADER_RECOVERY)
        return Teach.TrayXArmToLoaderXPosition;
    if(IsPickFromColor())
        return GetColorX();
    return Teach.TrayXArmToEmptyXPosition;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::DoPick(int Flag)
{
    //AI(HT160S-Maintainer) 20260605 : pick an empty tray from the EmptyTray rear.
    //Z-safe before X, then Z-down, clamp the tray (front+rear clamps hold the same
    //tray on its front/rear edges), Z-up, then hand off the EmptyTray rear slot.
    if(Flag==0)
    {
        PickTask=1;
        ArmDelay.Clear();
        return true;
    }

    switch(PickTask)
    {
        case 1:
            if(DoZUp())
                PickTask=10;
            break;

        case 10:
            if(MoveTrayArmX(GetPickSourceX()))
                PickTask=1000;
            break;

        case 1000:
            if(DoZDown())
                PickTask=2000;
            break;

        case 2000:
            if((HSys.Cyn.C_TrayArm_FrontClamp.Push() &&
                HSys.Cyn.C_TrayArm_RearClamp.Push()) || IsSoftSimulate())
            {
                ArmDelay.Set(3);
                ArmDelay.On();
                PickTask=2100;
            }
            break;

        case 2100:
            if(ArmDelay.Off())
                PickTask=3000;
            break;

        case 3000:
            if(DoZUp())
                PickTask=4000;
            break;

        case 4000:
            if(Job==TAJOB_LOADER_RECOVERY)
            {
                //AI(HT160S-Maintainer) 20260606 : tell the Loader its rear slot is now
                //free so it can feed/discharge the next tray.
                if(LoaderModule!=NULL)
                    LoaderModule->NotifyTrayArmPickRearTray();
            }
            else if(IsPickFromColor())
            {
                //AI(HT160S-Maintainer) 20260608 : carry the 2D TrayID that Color read off
                //the identity tray so it can be stamped onto the Auto stack on place.
                if(ColorModule!=NULL)
                {
                    iDeliverTrayID=ColorModule->GetTrayID();
                    ColorModule->NotifyTrayPicked();
                }
            }
            else
            {
                if(EmptyModule!=NULL)
                    EmptyModule->SetRearHasTray(false);
            }
            if(HSys.VMot.MMTrayArmX!=NULL)
                HSys.VMot.MMTrayArmX->fHasTray=true;
            bHasTray=true;
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TTrayArmModule::DoPlace(int Flag)
{
    //AI(HT160S-Maintainer) 20260605 : place the carried empty tray onto the target
    //Auto rear. Z-safe before X, Z-down, release clamps, Z-up, then mark the Auto rear
    //as filled so SortArm can sort IC into it.
    if(Flag==0)
    {
        PlaceTask=1;
        ArmDelay.Clear();
        return true;
    }

    //AI(HT160S-Maintainer) 20260606 : Loader-recovery jobs may instead recycle the tray
    //back to the EmptyTray rear when no Auto needs one. Dispatch to that path.
    if(PlaceDest==TAPLACE_EMPTY)
        return DoPlaceToEmpty(Flag);

    switch(PlaceTask)
    {
        case 1:
            if(DoZUp())
                PlaceTask=10;
            break;

        case 10:
            if(MoveTrayArmX(GetAutoX(iAutoTarget)))
                PlaceTask=1000;
            break;

        case 1000:
            if(DoZDown())
                PlaceTask=2000;
            break;

        case 2000:
            if((HSys.Cyn.C_TrayArm_FrontClamp.Pop() &&
                HSys.Cyn.C_TrayArm_RearClamp.Pop()) || IsSoftSimulate())
            {
                ArmDelay.Set(3);
                ArmDelay.On();
                PlaceTask=2100;
            }
            break;

        case 2100:
            if(ArmDelay.Off())
                PlaceTask=3000;
            break;

        case 3000:
            if(DoZUp())
                PlaceTask=4000;
            break;

        case 4000:
            if(AutoModule!=NULL && iAutoTarget>=0)
            {
                if(Job==TAJOB_AMR_SUPPLY)
                    //AI(HT160S-Maintainer) 20260605 : record the delivered tray's stack
                    //role so the Auto knows identity/cover trays must NOT receive IC.
                    //AI(HT160S-Maintainer) 20260608 : also pass the identity tray's 2D
                    //TrayID (empty for cover/normal) so the Auto car carries the stack ID.
                    AutoModule->NotifyTrayArmDelivered(iAutoTarget, iDeliverKind, iDeliverTrayID);
                else
                    AutoModule->SetRearHasTrayFromTrayArm(iAutoTarget, true);
            }
            if(HSys.VMot.MMTrayArmX!=NULL)
                HSys.VMot.MMTrayArmX->fHasTray=false;
            bHasTray=false;
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
void TTrayArmModule::DecidePlaceDestAfterPick()
{
    //AI(HT160S-Maintainer) 20260606 : called once the Loader empty tray is in hand. The
    //arm reacts to the live demand : if an Auto rear is free it supplies that Auto, else
    //it recycles the tray back into the EmptyTray supply pool. AMR stacks have a strict
    //identity/cover/normal order that is built only by the dedicated AMR supply job, so a
    //recovered plain tray is never injected mid-stack in AMR mode : it always recycles.
    bool bSupplyAuto=false;
    if(GeneralSetting.bUseAMR==false && AutoModule!=NULL)
    {
        //AI(general) 20260608 : Stage2 demand-driven Loader recovery - use the same pull
        //source as DecideJob (FindTrayRequestAuto) instead of FindEmptyRearForTrayArm, so
        //the recovered Loader tray is only handed to an Auto that actually requests one,
        //and only when it wants a plain Normal empty tray (the recovered tray carries no
        //identity/cover role). This also respects the Stage0 pending latch, so a tray
        //already on its way is never double-targeted. Otherwise recycle to EmptyTray.
        int kind=eTrayReqNone;
        int idx=AutoModule->FindTrayRequestAuto(kind);
        if(idx>=0 && kind==eTrayKindNormal)
        {
            iAutoTarget=idx;
            bSupplyAuto=true;
        }
    }

    if(bSupplyAuto)
    {
        PlaceDest=TAPLACE_AUTO;
    }
    else
    {
        PlaceDest=TAPLACE_EMPTY;
        iAutoTarget=-1;
        //Ask EmptyTray to free its rear (it goes up, pushing any parked rear tray into
        //the car) so it can accept the returned tray. We then wait for the rear to clear.
        if(EmptyModule!=NULL)
            EmptyModule->RequestReturnTray();
    }
}
//---------------------------------------------------------------------------
bool TTrayArmModule::DoPlaceToEmpty(int Flag)
{
    //AI(HT160S-Maintainer) 20260606 : recycle the carried empty tray back to the
    //EmptyTray rear. Action sequence mirrors the old 160 DoTrayX_Change_Empty (Z-safe,
    //move to Empty X, wait rear clear, Z-down, release clamps, Z-up). The handshake is
    //strengthened over the old global-flag version : instead of writing shared flags,
    //this calls EmptyModule->RequestReturnTray() (done in DecidePlaceDestAfterPick) and
    //only places once IsRearHasTray()==false confirms EmptyTray has gone up and freed its
    //rear, then signals completion with NotifyTrayXToEmptyFinish(). Clamp/Z cylinders
    //self-alarm on a sensor timeout (TMyCylinder::Push/Pop), so a stuck move is reported
    //rather than silently hanging.
    if(Flag==0)
    {
        PlaceTask=1;
        ArmDelay.Clear();
        return true;
    }

    switch(PlaceTask)
    {
        case 1:
            if(DoZUp())
                PlaceTask=10;
            break;

        case 10:
            if(MoveTrayArmX(Teach.TrayXArmToEmptyXPosition))
                PlaceTask=1000;
            break;

        case 1000:
            //Wait until EmptyTray has raised and cleared its rear before depositing.
            if(EmptyModule==NULL || EmptyModule->IsRearHasTray()==false || IsSoftSimulate())
                PlaceTask=2000;
            break;

        case 2000:
            if(DoZDown())
                PlaceTask=3000;
            break;

        case 3000:
            if((HSys.Cyn.C_TrayArm_FrontClamp.Pop() &&
                HSys.Cyn.C_TrayArm_RearClamp.Pop()) || IsSoftSimulate())
            {
                ArmDelay.Set(3);
                ArmDelay.On();
                PlaceTask=3100;
            }
            break;

        case 3100:
            if(ArmDelay.Off())
                PlaceTask=4000;
            break;

        case 4000:
            if(DoZUp())
                PlaceTask=5000;
            break;

        case 5000:
            //Tell EmptyTray the returned tray is now parked at its rear (this also marks
            //the rear as having a tray, so it re-enters the supply pool).
            if(EmptyModule!=NULL)
                EmptyModule->NotifyTrayXToEmptyFinish();
            if(HSys.VMot.MMTrayArmX!=NULL)
                HSys.VMot.MMTrayArmX->fHasTray=false;
            bHasTray=false;
            return true;
    }
    return false;
}
//---------------------------------------------------------------------------
void TTrayArmModule::DoTrayArm(int &Task)
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
            HasTray();
            //AI(HT160S-Maintainer) 20260605 : idle. If a tray is unexpectedly still
            //held (e.g. abort residue) do not guess where to drop it : stay idle until
            //a teach/recovery flow exists. Only start a new job when arm is empty.
            if(bHasTray)
            {
                //AI(HT160S-Maintainer) 20260612 : EXCEPTION - if the tray was in hand for
                //a still-valid delivery job that survived a recoverable home (Job!=NONE,
                //destination already chosen), this is not unknown residue : resume placing
                //that same tray so production continues without losing/dropping it.
                if(Job!=TAJOB_NONE)
                {
                    Status=TAS_CARRYING;
                    DoPlace(0);
                    Task=2000;
                    break;
                }
                Status=TAS_IDLE;
                break;
            }
            Job=DecideJob();
            if(Job==TAJOB_NONE)
            {
                Status=TAS_IDLE;
                break;
            }
            Status=TAS_PICKING;
            DoPick(0);
            Task=1000;
            break;

        case 1000:
            if(DoPick(1))
            {
                Status=TAS_CARRYING;
                //AI(HT160S-Maintainer) 20260606 : a Loader-recovery tray now in hand : decide
                //here whether to supply an Auto or recycle to EmptyTray. Supply jobs always
                //place to their pre-chosen Auto.
                if(Job==TAJOB_LOADER_RECOVERY)
                    DecidePlaceDestAfterPick();
                else
                    PlaceDest=TAPLACE_AUTO;
                DoPlace(0);
                Task=2000;
            }
            break;

        case 2000:
            if(DoPlace(1))
            {
                Status=TAS_IDLE;
                Job=TAJOB_NONE;
                Task=100;
            }
            break;
    }
}
//---------------------------------------------------------------------------
void InitializeTrayArmModule()
{
    if(TrayArmModule==NULL)
        TrayArmModule=new TTrayArmModule;
}
//---------------------------------------------------------------------------
void ShutdownTrayArmModule()
{
    delete TrayArmModule;
    TrayArmModule=NULL;
}
//---------------------------------------------------------------------------