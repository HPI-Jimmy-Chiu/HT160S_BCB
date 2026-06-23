//---------------------------------------------------------------------------
// uAgvStation.cpp
// AI(ht160s-agv) 20260615 : E87/AGV station coordinator implementation.
// Phase A scope = the static P1-P9 table + snapshot data + BuildBitmap /
// LookupByName helpers. The trigger / handshake methods (PollAndCall /
// ServiceHandshake / BeginPrep) are stubs here and get their behavior in
// Phase B / C / D. SVID numbering follows the draft section 6 (note the Auto4-6
// Tray/Device/Bin counts jump past the reserved band, hence the explicit table).
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "uAgvStation.h"
//AI(ht160s-agv) 20260615 : Phase B sources - AMR car-full state + mode gates +
// SECS transport to fire S6F11. All project-root headers (bare names resolve via
// the same include path uHGemHT160.cpp uses).
#include "uHGemEquipment.h"   // THGem (EventReport / IsSelected), HSMS state
#include "database.h"         // HSys.Sys.RunMode / Run_Normal
#include "GeneralSetting.h"   // GeneralSetting.bUseAMR
#include "aAuto1To6.h"        // AutoModule->GetAutoCar (TMyCar : iTrayCount/CarID/IsFull)
//---------------------------------------------------------------------------
#include "aLoader.h"          // LoaderModule (P1 infeed handoff)
#include "aEmpty.h"           // EmptyModule  (P2 infeed handoff)
#include "aColor.h"           // ColorModule  (P3 infeed handoff)
#pragma package(smart_init)
//---------------------------------------------------------------------------
TAgvCoordinator AgvCoord;

//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260623 : P1-P3 (Loader/Empty/Color) infeed handoff dispatch. Each
// infeed module owns its AMR state (bAmrLocked + sim tray count), mirroring how
// TAutoModule owns the Auto side; these map a 0..2 station index to the module.
static bool InfeedShortage(int p)
{
    if(p==0) return (LoaderModule!=NULL && LoaderModule->IsInputShortageForAmr());
    if(p==1) return (EmptyModule!=NULL  && EmptyModule->IsInputShortageForAmr());
    if(p==2) return (ColorModule!=NULL  && ColorModule->IsInputShortageForAmr());
    return false;
}
static bool InfeedReady(int p)
{
    if(p==0) return (LoaderModule!=NULL && LoaderModule->IsReadyForAmrHandoff());
    if(p==1) return (EmptyModule!=NULL  && EmptyModule->IsReadyForAmrHandoff());
    if(p==2) return (ColorModule!=NULL  && ColorModule->IsReadyForAmrHandoff());
    return false;
}
static bool InfeedFinished(int p)
{
    if(p==0) return (LoaderModule!=NULL && LoaderModule->IsInputHandoffFinishedForAmr());
    if(p==1) return (EmptyModule!=NULL  && EmptyModule->IsInputHandoffFinishedForAmr());
    if(p==2) return (ColorModule!=NULL  && ColorModule->IsInputHandoffFinishedForAmr());
    return false;
}
static void InfeedSetLock(int p, bool bLock)
{
    if(p==0 && LoaderModule!=NULL) LoaderModule->SetAmrLock(bLock);
    else if(p==1 && EmptyModule!=NULL) EmptyModule->SetAmrLock(bLock);
    else if(p==2 && ColorModule!=NULL) ColorModule->SetAmrLock(bLock);
}
static void InfeedRefill(int p)
{
    if(p==0 && LoaderModule!=NULL) LoaderModule->RefillSimInfeed();
    else if(p==1 && EmptyModule!=NULL) EmptyModule->RefillSimInfeed();
    else if(p==2 && ColorModule!=NULL) ColorModule->RefillSimInfeed();
}
//---------------------------------------------------------------------------

const TAgvStationDesc AgvStation[AGV_STATION_COUNT] =
{
    /*P1*/ { 1, ASK_LOADER, -1, 38202, 38222, 38228,     0, "Loader" },
    /*P2*/ { 2, ASK_EMPTY,  -1, 38203, 38223, 38229,     0, "Empty"  },
    /*P3*/ { 3, ASK_COLOR,  -1, 38204, 38224, 38230,     0, "Color"  },
    /*P4*/ { 4, ASK_AUTO,    0, 38205, 38225, 38231, 38234, "AUTO1"  },
    /*P5*/ { 5, ASK_AUTO,    1, 38206, 38226, 38232, 38235, "AUTO2"  },
    /*P6*/ { 6, ASK_AUTO,    2, 38207, 38227, 38233, 38236, "AUTO3"  },
    /*P7*/ { 7, ASK_AUTO,    3, 38208, 38237, 38240, 38243, "AUTO4"  },
    /*P8*/ { 8, ASK_AUTO,    4, 38209, 38238, 38241, 38244, "AUTO5"  },
    /*P9*/ { 9, ASK_AUTO,    5, 38210, 38239, 38242, 38245, "AUTO6"  }
};
//---------------------------------------------------------------------------
TAgvCoordinator::TAgvCoordinator()
{
    Reset();
}
//---------------------------------------------------------------------------
void TAgvCoordinator::Reset()
{
    SupplementBitmap = "";
    StatusBitmap     = "";
    FinishBitmap     = "";
    for(int i = 0; i < AGV_STATION_COUNT; i++)
    {
        CarrierID[i]        = "";
        TrayCount[i]        = 0;
        DeviceCount[i]      = 0;
        Handshake[i]        = AGV_IDLE;
        PrepDone[i]         = 0;
        ShortageLatch[i]    = 0;
        ShortageDebounce[i] = 0;
        ReadyEntrySensor[i] = 0;
    }
    for(int a = 0; a < AGV_AUTO_COUNT; a++)
        BinSetting[a] = "";
}
//---------------------------------------------------------------------------
// "P1:0,P2:0,...,Px:1,...,P9:0" with exactly one bit set (single-station rule).
AnsiString TAgvCoordinator::BuildBitmap(int targetPIndex)
{
    AnsiString s = "";
    for(int p = 1; p <= AGV_STATION_COUNT; p++)
    {
        if(p > 1)
            s += ",";
        s += "P" + IntToStr(p) + ":" + IntToStr((p == targetPIndex) ? 1 : 0);
    }
    return s;
}
//---------------------------------------------------------------------------
// Case-insensitive match of an S2F41 CP name against the station table.
int TAgvCoordinator::LookupByName(AnsiString cpName)
{
    AnsiString u = cpName.Trim().UpperCase();
    for(int i = 0; i < AGV_STATION_COUNT; i++)
    {
        if(u == AnsiString(AgvStation[i].Name).UpperCase())
            return i;
    }
    return -1;
}
//---------------------------------------------------------------------------
// Phase B/B-2 : raise AGVSupplement (CEID272). P4-P9 = AMR Auto output-car full
// (IsOutputCarFullForAmr : real InputFullTray sensor / sim tray threshold); P1-P3 =
// input shortage (SnLoader/Empty/Color_Input(e)nd, ON = needs refill, user-confirmed).
// A full Auto also LOCKS TrayArm feed and enters the handshake (CALLED), so production
// stops stacking onto that car while the AGV is on the way. Events fire only while the
// link is SELECTED; on a drop, any in-progress handoff is released so ServiceCarFull's
// operator fallback resumes (no events emitted while disconnected).
void TAgvCoordinator::PollAndCall(THGem *Gem)
{
    if(GeneralSetting.bUseAMR==false)
        return;
    if(AutoModule==NULL)
        return;

    bool bSelected = (Gem!=NULL && Gem->IsSelected());

    if(bSelected==false)
    {
        // link down : abandon any handoff so the operator fallback (ServiceCarFull) runs
        for(int a = 0; a < AGV_AUTO_COUNT; a++)
        {
            int si = a + 3;
            if(Handshake[si]!=AGV_IDLE)
            {
                AutoModule->SetAmrLock(a, false);
                Handshake[si] = AGV_IDLE;
            }
        }
        for(int p = 0; p < 3; p++)
        {
            if(Handshake[p]!=AGV_IDLE)
            {
                InfeedSetLock(p, false);
                Handshake[p] = AGV_IDLE;
            }
            ShortageLatch[p] = 0;
        }
        return;
    }

    if(HSys.Sys.RunMode!=Run_Normal)
        return;

    // --- P4-P9 : Auto output-car full -> lock + AGVSupplement (enter CALLED) ---
    for(int a = 0; a < AGV_AUTO_COUNT; a++)
    {
        int si = a + 3;                 // station index : Auto1->P4(idx3) .. Auto6->P9(idx8)
        TMyCar *Car = AutoModule->GetAutoCar(a);
        if(Car!=NULL)
        {
            // keep the SVID snapshot live for ad-hoc S1F3 reads (DeviceCount stays 0
            // until the per-tray IC count / AMR upload payload is designed).
            TrayCount[si] = Car->iTrayCount;
            CarrierID[si] = Car->CarID;
        }

        bool bFull = AutoModule->IsOutputCarFullForAmr(a);
        if(bFull && Handshake[si]==AGV_IDLE)
        {
            AutoModule->SetAmrLock(a, true);
            SupplementBitmap = BuildBitmap(AgvStation[si].PIndex);
            Gem->EventReport(0, 272);   // CEID272 AGVSupplement
            Handshake[si] = AGV_CALLED;
        }
        else if(bFull==false && Handshake[si]==AGV_CALLED)
        {
            // car emptied before the AGV engaged (e.g. manual) : release the lock
            AutoModule->SetAmrLock(a, false);
            Handshake[si] = AGV_IDLE;
        }
    }

    // --- P1-P3 : input shortage -> AGVSupplement (enter CALLED). Polarity: the input
    // sensor reads ON=has tray, OFF=empty (user-confirmed 20260623 + draft section 9), so
    // shortage = empty. IsInputShortageForAmr lives in the module (sim drains the tray
    // count to 0; real reads SnX_Input(e)nd OFF). Mirrors the Auto full-call above.
    for(int p = 0; p < 3; p++)
    {
        bool bShort = InfeedShortage(p);
        if(bShort && Handshake[p]==AGV_IDLE)
        {
            SupplementBitmap = BuildBitmap(AgvStation[p].PIndex);
            Gem->EventReport(0, 272);   // CEID272 AGVSupplement
            Handshake[p] = AGV_CALLED;
            ShortageLatch[p] = 1;       // kept for the FeederDecision snapshot
        }
        else if(bShort==false && Handshake[p]==AGV_CALLED)
        {
            Handshake[p] = AGV_IDLE;    // refilled before the AGV engaged : re-arm
            ShortageLatch[p] = 0;
        }
    }
}
//---------------------------------------------------------------------------
// Phase D : drive Ready (CEID273) and Finish (CEID274) for the Auto handoff.
// Ready : START_AGV received (AGV_PREP) and the Auto has stacked every tray into the
// car (IsDrainedForAmr). Finish : the AGV removed the car (IsAmrTaken) -> clear the car
// + release the lock so production resumes. Sim reports taken so the flow is testable;
// real machine holds at Ready until the car-taken IO point is wired.
void TAgvCoordinator::ServiceHandshake(THGem *Gem)
{
    if(GeneralSetting.bUseAMR==false)
        return;
    if(AutoModule==NULL)
        return;
    if(Gem==NULL || Gem->IsSelected()==false)
        return;

    for(int a = 0; a < AGV_AUTO_COUNT; a++)
    {
        int si = a + 3;
        if(Handshake[si]==AGV_PREP)
        {
            if(AutoModule->IsDrainedForAmr(a))
            {
                StatusBitmap = BuildBitmap(AgvStation[si].PIndex);
                Gem->EventReport(0, 273);   // CEID273 AGVLDUnLDStatus (Ready)
                Handshake[si] = AGV_READY;
            }
        }
        else if(Handshake[si]==AGV_READY)
        {
            if(AutoModule->IsAmrTaken(a))
            {
                FinishBitmap = BuildBitmap(AgvStation[si].PIndex);
                Gem->EventReport(0, 274);   // CEID274 AGVLDUnLDFinish
                AutoModule->ClearAmrCar(a);
                Handshake[si] = AGV_IDLE;
            }
        }
    }

    // --- P1-P3 : infeed handoff (CEID273 Ready / CEID274 Finish). Ready = the station's
    // front stacking cylinders are home/idle (IsReadyForAmrHandoff), the BeginPrep lock
    // holding them frozen through the handoff. Finish = refill complete (real: the input
    // sensor reads a tray present; sim: auto-completes), then release + restock.
    for(int p = 0; p < 3; p++)
    {
        if(Handshake[p]==AGV_PREP)
        {
            if(InfeedReady(p))
            {
                StatusBitmap = BuildBitmap(AgvStation[p].PIndex);
                Gem->EventReport(0, 273);   // CEID273 AGVLDUnLDStatus (Ready)
                Handshake[p] = AGV_READY;
            }
        }
        else if(Handshake[p]==AGV_READY)
        {
            if(InfeedFinished(p))
            {
                FinishBitmap = BuildBitmap(AgvStation[p].PIndex);
                Gem->EventReport(0, 274);   // CEID274 AGVLDUnLDFinish
                InfeedSetLock(p, false);    // front destack may resume
                InfeedRefill(p);            // sim : restock the input stack to max
                Handshake[p] = AGV_IDLE;
                ShortageLatch[p] = 0;
            }
        }
    }
}
//---------------------------------------------------------------------------
// Phase C : START_AGV(cpName) -> begin the station's AGV-handoff prep (AGV_PREP).
// An Auto station also locks TrayArm feed (idempotent with PollAndCall's full lock).
// Returns true if cpName is a known station (Loader/Empty/Color/AUTO1..6), false
// otherwise (caller handles non-station CPs like LoaderTrayCount). Ready (CEID273)
// follows in ServiceHandshake once the Auto has drained.
bool TAgvCoordinator::BeginPrep(AnsiString cpName)
{
    int i = LookupByName(cpName);
    if(i < 0)
        return false;
    Handshake[i] = AGV_PREP;     // station asked to prepare for AGV handoff
    PrepDone[i]  = 0;
    if(AgvStation[i].Kind==ASK_AUTO && AutoModule!=NULL && AgvStation[i].AutoIndex>=0)
        AutoModule->SetAmrLock(AgvStation[i].AutoIndex, true);
    else if(AgvStation[i].Kind!=ASK_AUTO && i < 3)
        InfeedSetLock(i, true);   //AI(ht160s-agv) 20260623 : P1-P3 freeze front destack for the handoff
    return true;
}
//---------------------------------------------------------------------------
