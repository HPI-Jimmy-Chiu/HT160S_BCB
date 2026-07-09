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
#include "uAmrInject.h"      // AI(ht160s-agv) 20260708 : AMR manual-inject test facility
#pragma package(smart_init)
//---------------------------------------------------------------------------
TAgvCoordinator AgvCoord;
TAmrInject AmrInject;

// AI(ht160s-agv) 20260625 : ServiceHandshake runs on the THGem 1s tick (see
// HT160Gem::ServiceAgv). A station that enters AGV_PREP / AGV_READY but never
// reaches its release gate (stuck Ready interlock, starved tick) would latch its
// AMR lock forever (only HOME clears it) and freeze the feed loop. This is a
// GENEROUS force-release dwell (ticks ~= seconds) so a real AGV in transit is
// never aborted prematurely; it only catches a truly stuck handshake.
// AI(ht160s-agv) 20260627 : the watchdog aging limit is now file-configurable via
// GeneralSetting.iAmrHandshakeWaitSec ([AGV] AmrHandshakeWaitSec, default 240s,
// clamped >=5 in GeneralSetting::Load). The old fixed #define was removed.

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
    if(p==0 && LoaderModule!=NULL)
    {
        //AI(ht160s-agv) 20260627 : latch the host-declared physical magazine total
        //(SECS LoaderTrayCount = IC + cover + identity) that the preceding S2F41
        //START_AGV captured into AgvCoord.TrayCount[0], so the Loader tags tray kind
        //and runs the count-vs-Inputend cross-check against the REAL total. 0 = host
        //silent -> the Loader falls back to iSimAmrMaxTray inside RefillSimInfeed.
        LoaderModule->SetExpectedCarTrayCount(AgvCoord.TrayCount[0]);
        LoaderModule->RefillSimInfeed();
    }
    else if(p==1 && EmptyModule!=NULL) EmptyModule->RefillSimInfeed();
    else if(p==2 && ColorModule!=NULL) ColorModule->RefillSimInfeed();
}
// AI(ht160s-agv) 20260625 : read-only AMR lock state for DescribeAgvState (the lock
// itself is owned by each infeed module; this just reflects it).
static bool InfeedLocked(int p)
{
    if(p==0) return (LoaderModule!=NULL && LoaderModule->IsAmrLocked());
    if(p==1) return (EmptyModule!=NULL  && EmptyModule->IsAmrLocked());
    if(p==2) return (ColorModule!=NULL  && ColorModule->IsAmrLocked());
    return false;
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
    // AI(ht160s-agv) 20260625 : a reconnect / Reset must NOT orphan an AMR infeed
    // or Auto lock. Zeroing Handshake[] alone leaves bAmrLocked=1 latched on the
    // module (only HOME would clear it) -> the freed station stays frozen forever.
    // Release every station lock here, mirroring the link-down release in PollAndCall.
    for(int p = 0; p < 3; p++)
        InfeedSetLock(p, false);
    if(AutoModule!=NULL)
    {
        for(int a = 0; a < AGV_AUTO_COUNT; a++)
            AutoModule->SetAmrLock(a, false);
    }
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
    //AI(ht160s-secsgem) 20260625 : two-stage Auto Full pre-notification CEIDs (9045-
    // aligned : Auto1-3=35/36/37, Auto4-6=148/149/150). Emitted on the full edge next
    // to AGVSupplement so a 9045-style host gets the discrete Full signal before it
    // decides and sends START_AGV. DataID=1 matches the sibling Unloadtray event.
    int AutoFullCeid[6] = {35, 36, 37, 148, 149, 150};
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

        bool bFull = AutoModule->IsOutputCarFullForAmr(a) || AmrInject.AutoFull(a);   //AI(ht160s-agv) 20260708 : test-mode inject (handshake-only)
        // AI(ht160s-agv) 20260627 : do NOT re-CALL an Auto the operator is taking after a
        // station-side full-wait timeout (AbortAutoHandshake set Handshake=AGV_IDLE);
        // IsOperatorHolding is cleared by HOME/InitialFlag so re-CALL then resumes.
        if(bFull && Handshake[si]==AGV_IDLE && AutoModule->IsOperatorHolding(a)==false)
        {
            AutoModule->SetAmrLock(a, true);
            SupplementBitmap = BuildBitmap(AgvStation[si].PIndex);
            Gem->EventReport(0, 272);   // CEID272 AGVSupplement
            Gem->EventReport(1, AutoFullCeid[a]);   // discrete Auto Full (two-stage pre-notification)
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
        bool bShort = InfeedShortage(p) || AmrInject.InputShort(p);   //AI(ht160s-agv) 20260708 : test-mode inject (handshake-only)
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
        unsigned char hsBefore = Handshake[si];
        if(Handshake[si]==AGV_PREP)
        {
            if(AutoModule->IsDrainedForAmr(a) || AmrInject.AutoDrained(a))   //AI(ht160s-agv) 20260708 : test-mode inject (SECS-273 gate only)
            {
                StatusBitmap = BuildBitmap(AgvStation[si].PIndex);
                Gem->EventReport(0, 273);   // CEID273 AGVLDUnLDStatus (Ready)
                Handshake[si] = AGV_READY;
            }
        }
        else if(Handshake[si]==AGV_READY)
        {
            if(AutoModule->IsAmrTaken(a) || AmrInject.AutoTaken(a))   //AI(ht160s-agv) 20260708 : test-mode inject (one-shot)
            {
                FinishBitmap = BuildBitmap(AgvStation[si].PIndex);
                Gem->EventReport(0, 274);   // CEID274 AGVLDUnLDFinish
                AutoModule->ClearAmrCar(a);
                Handshake[si] = AGV_IDLE;
            }
        }
        // AI(ht160s-agv) 20260625 : watchdog. Age PREP/READY; on a stuck gate
        // force-release the Auto lock so the feed loop is never latched forever.
        if((Handshake[si]==AGV_PREP || Handshake[si]==AGV_READY) && Handshake[si]==hsBefore)
        {
            if(++ShortageDebounce[si] > GeneralSetting.iAmrHandshakeWaitSec)
            {
                AutoModule->SetAmrLock(a, false);
                Handshake[si]        = AGV_IDLE;
                ShortageLatch[si]    = 0;
                ShortageDebounce[si] = 0;
            }
        }
        else
            ShortageDebounce[si] = 0;   // state changed (or idle) : restart the age
    }

    // --- P1-P3 : infeed handoff (CEID273 Ready / CEID274 Finish). Ready = the station's
    // front stacking cylinders are home/idle (IsReadyForAmrHandoff), the BeginPrep lock
    // holding them frozen through the handoff. Finish = refill complete (real: the input
    // sensor reads a tray present; sim: auto-completes), then release + restock.
    for(int p = 0; p < 3; p++)
    {
        unsigned char hsBefore = Handshake[p];
        if(Handshake[p]==AGV_PREP)
        {
            if(InfeedReady(p) || AmrInject.InputReady(p))   //AI(ht160s-agv) 20260708 : test-mode inject (SECS-273 gate only)
            {
                StatusBitmap = BuildBitmap(AgvStation[p].PIndex);
                Gem->EventReport(0, 273);   // CEID273 AGVLDUnLDStatus (Ready)
                Handshake[p] = AGV_READY;
            }
        }
        else if(Handshake[p]==AGV_READY)
        {
            if(InfeedFinished(p) || AmrInject.InputFinish(p))   //AI(ht160s-agv) 20260708 : test-mode inject (one-shot)
            {
                FinishBitmap = BuildBitmap(AgvStation[p].PIndex);
                Gem->EventReport(0, 274);   // CEID274 AGVLDUnLDFinish
                InfeedSetLock(p, false);    // front destack may resume
                InfeedRefill(p);            // sim : restock the input stack to max
                Handshake[p] = AGV_IDLE;
                ShortageLatch[p] = 0;
            }
        }
        // AI(ht160s-agv) 20260625 : watchdog. Age PREP/READY; on a stuck gate force-
        // release the infeed lock so the WHOLE feed loop is never latched forever.
        if((Handshake[p]==AGV_PREP || Handshake[p]==AGV_READY) && Handshake[p]==hsBefore)
        {
            if(++ShortageDebounce[p] > GeneralSetting.iAmrHandshakeWaitSec)
            {
                InfeedSetLock(p, false);
                Handshake[p]        = AGV_IDLE;
                ShortageLatch[p]    = 0;
                ShortageDebounce[p] = 0;
            }
        }
        else
            ShortageDebounce[p] = 0;   // state changed (or idle) : restart the age
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
// AI(ht160s-agv) 20260627 : station-side timeout release for an Auto output car.
// ServiceCarFull waited iAmrFullWaitSec for the AGV and timed out; the operator is
// taking the full car manually. Drop THIS Auto's handshake (release the lock + reset
// the handshake/aging state) so neither the watchdog (ServiceHandshake) nor the
// re-CALL path (PollAndCall) touches it again until the next clean full edge -- the
// re-CALL is additionally gated by IsOperatorHolding, cleared on HOME/InitialFlag.
// Modeled on the link-down release in PollAndCall (SetAmrLock false + AGV_IDLE).
void TAgvCoordinator::AbortAutoHandshake(int Index)
{
    if(Index < 0 || Index >= AGV_AUTO_COUNT)
        return;
    int si = Index + 3;
    if(AutoModule!=NULL)
        AutoModule->SetAmrLock(Index, false);
    Handshake[si]        = AGV_IDLE;
    ShortageLatch[si]    = 0;
    ShortageDebounce[si] = 0;
}
//---------------------------------------------------------------------------
// AI(ht160s-agv) 20260625 : read-only multi-line dump of coordinator state. Used by
// BOTH the State Record snapshot writer and the AMR maintenance panel (single source,
// no extra getters). Header = Selected (live HSMS link) + bUseAMR; then one line per
// P1..P9 with the live lock / handshake / ready-gate value. Changes NO state.
static const char *AgvHsName(unsigned char hs)
{
    switch(hs)
    {
        case AGV_IDLE:   return "IDLE";
        case AGV_CALLED: return "CALLED";
        case AGV_PREP:   return "PREP";
        case AGV_READY:  return "READY";
        case AGV_FINISH: return "FINISH";
    }
    return "?";
}
//---------------------------------------------------------------------------
AnsiString TAgvCoordinator::DescribeAgvState()
{
    int iSelected = (HGem!=NULL && HGem->IsSelected()) ? 1 : 0;
    int iUseAmr   = (GeneralSetting.bUseAMR) ? 1 : 0;
    AnsiString s = "Selected=" + IntToStr(iSelected) + " bUseAMR=" + IntToStr(iUseAmr) + "\r\n";
    for(int i = 0; i < AGV_STATION_COUNT; i++)
    {
        int iLock  = 0;
        int iReady = 0;
        if(AgvStation[i].Kind==ASK_AUTO)
        {
            int a = AgvStation[i].AutoIndex;
            if(AutoModule!=NULL && a>=0)
            {
                iLock  = AutoModule->IsAmrLocked(a)    ? 1 : 0;
                iReady = AutoModule->IsDrainedForAmr(a) ? 1 : 0;
            }
        }
        else if(i < 3)
        {
            iLock  = InfeedLocked(i) ? 1 : 0;
            iReady = InfeedReady(i)  ? 1 : 0;
        }
        s += "P" + IntToStr(AgvStation[i].PIndex) + " " + AnsiString(AgvStation[i].Name)
           + ": lock=" + IntToStr(iLock)
           + " hs="    + AnsiString(AgvHsName(Handshake[i]))
           + " ready=" + IntToStr(iReady) + "\r\n";
    }
    s += AmrInject.Describe();   //AI(ht160s-agv) 20260708 : test-mode + armed-latch state
    return s;
}
//---------------------------------------------------------------------------
