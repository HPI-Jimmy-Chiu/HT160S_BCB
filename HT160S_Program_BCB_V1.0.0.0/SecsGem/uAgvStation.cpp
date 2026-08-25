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
#include "cmydef.h"      //AI(amr-nohang) 20260825 : was fAllMotorHome for the HOME freeze gate (that term is gone); kept for the shared globals
#include "note.h"        //AI(ht160s-home-resume-w5) : RecordProcess
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
#include "CosFunction.h"     // AI(ht160s-agv-binsetting) 20260713 : BinAreaMap / LotBinBinding routing model (bin setting source)
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
        //AI(ht160s-overcount-tripqueue) 20260721 : enqueue this car as a per-car feed trip
        //(work count from the preceding S2F41 LoaderTrayCount CP into AgvCoord.TrayCount[0]).
        //TripQueue keeps overlapping cars' cover/identity boundaries separate; nWork<=0
        //warns + skips (over-count Cover path at mint). Consume-once: clear TrayCount[0] so
        //a later car whose START_AGV omits LoaderTrayCount does NOT silently inherit this
        //car's count (the old stale-reuse footgun).
        LoaderModule->EnqueueTrip(AgvCoord.TrayCount[0]);
        AgvCoord.TrayCount[0] = 0;
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
    //AI(secs-align-810b01) 20260817 : P2/P3 CarrierID column is now 0 = NOT REGISTERED.
    // Authority ruling (user, 20260817) : the ONE tree HT160S aligns to is
    // HT9046LS_Code_V3.32.810_B01_20260527KeyPro_01_AutoUP. That tree registers exactly four
    // carrier ids - 38202 Load Port (uHGemHT9045_SV.cpp:833) and 38205/38206/38207 Auto1-3
    // (:834-:836). It has NO 38203 and NO 38204 in either namespace (whole-tree sweep, both
    // SetSVDataPointer and SetECDataPointer, 0 hits). HT160S had invented them for Empty/Color
    // and then never wrote them : CarrierID[] is only ever assigned at index 0 (ReportLoaderIdentity)
    // and 3..8 (the Auto loop), so the host received two permanently empty <A[0]> items on report 5.
    // Worse, 38203/38204 sat inside the vendor's own carrier-id run (38202 .. 38205-38207) with
    // exactly the meaning the vendor would give them if it ever grew there - a silent semantic
    // collision. Deleting the registration removes both problems and loses no family compatibility.
    // A 0 in this column means "no SVID" ; the registration loop and report 5 skip it.
    // NOTE the Tray/Device columns 38223/38224/38229/38230 are deliberately KEPT even though they
    // are equally unwritten : those ARE family ids (uHGemHT9045_SV.cpp:843/:844/:849/:850) and
    // 810_B01 leaves them unwritten too (every write there is iAMRTrayCount/iAMRDeviceCount
    // [3]/[Pos+3]/[i+3] = Auto1-3 only). Keeping them registered-but-0 IS the aligned behavior.
    /*P1*/ { 1, ASK_LOADER, -1, 38202, 38222, 38228,     0, "Loader" },
    /*P2*/ { 2, ASK_EMPTY,  -1,     0, 38223, 38229,     0, "Empty"  },
    /*P3*/ { 3, ASK_COLOR,  -1,     0, 38224, 38230,     0, "Color"  },
    /*P4*/ { 4, ASK_AUTO,    0, 38205, 38225, 38231, 38234, "AUTO1"  },
    /*P5*/ { 5, ASK_AUTO,    1, 38206, 38226, 38232, 38235, "AUTO2"  },
    /*P6*/ { 6, ASK_AUTO,    2, 38207, 38227, 38233, 38236, "AUTO3"  },
    //AI(secs-auto-align-899) 20260803 : the Auto4-6 CARRIER ID column is no longer invented.
    // Rule from the customer : Auto1-3 follow HT9046LS V3.32.810, Auto4-6 follow HT9011UC
    // V3.33.899. 810 numbers only three output-port tray ids (38205/38206/38207, and it declares
    // them as SVs - uHGemHT9045_SV.cpp:797-799), so P4-P6 above are untouched. 899 - the six-Auto
    // machine - names the other three "Output 4/5/6 Tray ID" = 38199/38200/38201
    // (uHGemHT9045_EC.cpp:1664-1666), so P7-P9 adopt those numbers instead of the former
    // HT160S-invented 38208/38209/38210. Note 899 declares that whole family (its own 38205-38207
    // included) in the EC namespace; HT160S keeps the SV namespace because that is what 810 does
    // for Auto1-3 and the six ports must answer through ONE namespace.
    // Checked before renumbering : 810 uses nothing in 38195-38201 (its band is 38202, 38205-38207,
    // 38219+) and HT160S had no other use of 38199-38201. 38208/38209/38210 are now RETIRED - do
    // not reuse them, a host provisioned before 20260803 still has them bound to AUTO4-6 Carrier ID.
    // The tray / device / bin-setting columns for Auto4-6 stay HT160S extensions : 810's AMR band
    // ends at 38236 and 899 has no AMR SVID family at all, so no family number exists to adopt.
    //AI(secs-record-traycount) 20260810 : THE TRAY COLUMN MOVED, 38237-38239 -> 38246-38248.
    // The 20260803 note above said 38237-38245 were free to extend into, "verified in both trees".
    // That verification has since been overtaken by the reference tree itself : HT9046LS 810_B01
    // now defines 38237/38238/38239 as "Record Auto 1/2/3 Tray Count" (uHGemHT9045_SV.cpp:861-863,
    // added by the vendor 20260710), which is exactly the quantity KYEC asked us to publish there.
    // The family number wins, so AMR Auto4-6 Tray Count vacates to 38246/38247/38248 - the first
    // ids free in BOTH trees (HT160S owns up to 38245; 810_B01's next used id above 38239 is
    // 38511, so 38240-38510 is one unbroken gap). 38237/38238/38239 are re-registered with the
    // family meaning in uHGemHT160 AddSV, backed by tRunData.RecordTrayCnt.
    // 38237-38239 as AMR Auto4-6 Tray Count are RETIRED for that meaning - a host provisioned
    // before 20260810 still has them bound that way, and it will now read the Record counter
    // instead. This is the one renumbering in this band that changes an id's MEANING rather than
    // just vacating it, so it must be called out to the host, not left to be discovered.
    // LESSON, worth keeping : inventing ids inside a band the family also grows into gets
    // overtaken. Re-run this check against every 9045 upgrade.
    /*P7*/ { 7, ASK_AUTO,    3, 38199, 38246, 38240, 38243, "AUTO4"  },
    /*P8*/ { 8, ASK_AUTO,    4, 38200, 38247, 38241, 38244, "AUTO5"  },
    /*P9*/ { 9, ASK_AUTO,    5, 38201, 38248, 38242, 38245, "AUTO6"  }
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
        TimeoutPending[i]   = 0;   //AI(amr-unmanned W3) 20260721
        LinkLostAge[i]      = 0;   //AI(agv-linklost-hold) 20260819
        LinkLostPending[i]  = 0;
    }
    for(int a = 0; a < AGV_AUTO_COUNT; a++)
        BinSetting[a] = "";
}
//---------------------------------------------------------------------------
//AI(agv-linklost-hold) 20260819 : does this station still physically hold its module lock?
//Deliberately asks the MODULE, not Handshake[]. RetryStation leaves a station at AGV_IDLE while
//keeping its lock, so a Handshake test would miss exactly the orphan we most need to cover.
bool TAgvCoordinator::IsStationLockHeld(int si)
{
    if(si < 0 || si >= AGV_STATION_COUNT)
        return false;
    if(si < 3)
        return InfeedLocked(si);
    if(AutoModule==NULL)
        return false;
    return AutoModule->IsAmrLocked(si - 3);
}
//---------------------------------------------------------------------------
//AI(agv-linklost-hold) 20260819 : the Clean Out P1-P3 force-release, factored out so the
//disconnected branch of PollAndCall can still reach it (it used to sit below that branch's
//return, and was only harmless because the old code had already released everything).
void TAgvCoordinator::ReleaseInfeedForCleanOut()
{
    for(int p = 0; p < 3; p++)
    {
        if(Handshake[p]!=AGV_IDLE)
        {
            InfeedSetLock(p, false);
            Handshake[p] = AGV_IDLE;
        }
        ShortageLatch[p]   = 0;
        LinkLostAge[p]     = 0;   //AI(agv-linklost-hold) : released here, no hold to age
        LinkLostPending[p] = 0;
    }
}
//---------------------------------------------------------------------------
//AI(agv-linklost-hold) 20260819 : age the link-lost hold. Runs ONLY from the disconnected
//branch of PollAndCall, which the 1s THGem tick keeps calling (its only gates are bUseAMR and
//AutoModule). Nothing here depends on the link, on HOME, or on a sensor, so a held lock always
//has a bounded way out.
//ShortageDebounce is zeroed every tick on purpose : it measures "the AGV is not answering",
//which is unknowable while WE are the ones off the wire. Leaving it running would blow straight
//past iAgvTimeoutSec on the first tick after reconnect and raise a false WAR0962; the AGV gets a
//full fresh timeout instead, and this function times the disconnect itself.
void TAgvCoordinator::ServiceLinkLostHold()
{
    int iLimit = GeneralSetting.iAgvTimeoutSec;
    if(iLimit < 5)
        iLimit = 5;                      // mirror the Load()/maintenance clamp
    for(int si = 0; si < AGV_STATION_COUNT; si++)
    {
        ShortageDebounce[si] = 0;
        if(IsStationLockHeld(si)==false)
        {
            LinkLostAge[si]     = 0;
            LinkLostPending[si] = 0;
            continue;
        }
        if(LinkLostAge[si]==0)
            RecordProcess("AGV: HSMS link lost while P"+IntToStr(si+1)+
                          " holds an AMR lock - lock and handshake are HELD (hs="+
                          IntToStr(Handshake[si])+")");
        LinkLostAge[si]++;
        if(LinkLostAge[si] > iLimit && LinkLostPending[si]==0)
        {
            RecordProcess("AGV: link-lost hold on P"+IntToStr(si+1)+" exceeded "+
                          IntToStr(iLimit)+"s -> WAR0963 pending");
            LinkLostPending[si] = 1;
        }
    }
}
//---------------------------------------------------------------------------
//AI(agv-linklost-hold) 20260819 : SELECTED again. Drop the hold counters only - the lock and the
//handshake state survive on purpose so ServiceHandshake picks the handoff up where it stopped.
void TAgvCoordinator::ClearLinkLostHold()
{
    AnsiString sHeld;
    for(int si = 0; si < AGV_STATION_COUNT; si++)
    {
        if(LinkLostAge[si]!=0)
            sHeld += " P"+IntToStr(si+1)+"("+IntToStr(LinkLostAge[si])+"s)";
        LinkLostAge[si]     = 0;
        LinkLostPending[si] = 0;
    }
    if(sHeld!="")
        RecordProcess("AGV: HSMS link restored - held locks resume their handshake:"+sHeld);
}
//---------------------------------------------------------------------------
//AI(agv-linklost-hold) 20260819 : the WAR0963 answer. This is the ONLY path allowed to release a
//lock without SECS evidence, and it exists because the operator physically checked the station.
//Handshake MUST be forced to IDLE here : leaving it at PREP/READY would let ReassertLocks re-couple
//the module lock at the next InitialAllTask and the station would silently freeze again.
void TAgvCoordinator::ReleaseStationByOperator(int si)
{
    if(si < 0 || si >= AGV_STATION_COUNT)
        return;
    if(si < 3)
        InfeedSetLock(si, false);
    else if(AutoModule!=NULL)
        AutoModule->SetAmrLock(si - 3, false);
    RecordProcess("AGV: WAR0963 operator confirmed AMR left P"+IntToStr(si+1)+
                  " - lock released after "+IntToStr(LinkLostAge[si])+"s hold");
    Handshake[si]        = AGV_IDLE;
    ShortageLatch[si]    = 0;
    ShortageDebounce[si] = 0;
    TimeoutPending[si]   = 0;
    LinkLostAge[si]      = 0;
    LinkLostPending[si]  = 0;
}
//---------------------------------------------------------------------------
//AI(amr-unmanned W3) 20260721 : WAR0962 K_RETRY handler. Reset the station to IDLE and
//clear the timeout/shortage latches so PollAndCall re-evaluates its trigger (full car /
//supply shortage) next tick and re-CALLs if it is still true. Locks are deliberately NOT
//released : between IDLE and the re-CALL a released Auto lock would let FindDischargeAuto
//GoUp into the still-full car. Real recovery is the underlying condition clearing (the AGV
//takes the car / a refill restocks) -> PollAndCall then sees not-full/not-short and simply
//does not re-CALL; a served car completes via IsAmrTaken (InputEnd-OFF -> CEID274).
void TAgvCoordinator::RetryStation(int si)
{
    if(si < 0 || si >= AGV_STATION_COUNT)
        return;
    TimeoutPending[si]   = 0;
    ShortageDebounce[si] = 0;
    ShortageLatch[si]    = 0;
    if(Handshake[si]!=AGV_IDLE)
    {
        RecordProcess("AGV: WAR0962 RETRY - station P"+IntToStr(si+1)+" reset to IDLE for re-call");
        Handshake[si] = AGV_IDLE;
    }
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
//AI(ht160s-agv-identity2d) 20260714 : HT9045 AGVLdID port. The identity tray's 2D is read by the
// Color CCD (DoReadColor2D) at the Loader-recovery intake; when the scan completes, upload it as
// S6F11 CEID275. The value rides SVID AgvStation[stationIndex].SvidCarrierID via the dedicated
// single-SVID report 7 (stationIndex = AMR_IDENTITY_CARRIER_INDEX). Stamp
//AI(secs-identity2d-38202) 20260805 : that constant is now 0 = Loader P1 / SVID 38202, NOT Color P3
// / 38204. The Color CCD is still what reads the code - only the SVID it is published on changed,
// so this function is untouched. See the constant's comment in uAgvStation.h for why.
// CarrierID[stationIndex] then fire. EventReport self-gates on HSMS SELECTED, so this is a no-op
// when no host is connected (incl. laptop SOFT_SIMULATE without the SECS simulator attached).
// DataID=1 aligns to HT9045 (all equipment-initiated S6F11 use DataID=1) AND to HT160S's own
// operator events; the host dispatches on CEID, DataID is host-informational. AI(secs-pathA-align)
// 20260727 : changed from the earlier HT160S-only DataID=0 on AGV events (272/273/274/275) to align
// the whole AMR command set to HT9045 per customer request. (The 1-arg EventReport wrapper also uses DataID=1.)
void TAgvCoordinator::ReportLoaderIdentity(THGem *Gem, int stationIndex, AnsiString id2D)
{
    if(Gem == NULL)
        return;
    if(id2D == "")
        return;                    // never upload a blank carrier id (skipped/failed 2D read)
    if(stationIndex < 0 || stationIndex >= AGV_STATION_COUNT)
        return;                    // guard the index (single change-point AMR_IDENTITY_CARRIER_INDEX)
    CarrierID[stationIndex] = id2D;   // SVID = AgvStation[stationIndex].SvidCarrierID; CEID275 ships it via report 7
    Gem->EventReport(1, 275);      // CEID275 AGVLdID
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
        //AI(agv-linklost-hold) 20260819 : a dropped HSMS link is NOT evidence that the AMR has
        //left. This block used to release every lock whose handshake was not IDLE - which
        //INCLUDES AGV_PREP and AGV_READY, i.e. exactly the states meaning "the AMR was called
        //and has been told it may come in". The machine has no AMR-presence input at all
        //(IO_Table.csv has no agv/amr/dock row), so nothing could contradict that guess, and
        //the release emitted no RecordProcess, so nothing was traceable afterwards. A host EAP
        //restart or one late linktest un-froze a station an AMR could still be docked at,
        //within one 1s tick, and production motion resumed into it.
        //The stated justification does not hold either : ServiceCarFull's operator fallback
        //does NOT depend on the lock. Its only AMR gate is bUseAMR && HGem->IsSelected()
        //(aAuto1To6.cpp), it contains no bAmrLocked test at all, and its own comment says a
        //link-down case "still falls through to the operator modal below".
        //Now : keep every lock and every handshake state, and age a link-lost hold instead.
        //The escape is link-independent - see ServiceLinkLostHold and the WAR0963 consumer in
        //csystem - so a retained lock always has a bounded way out and never becomes an orphan.
        ServiceLinkLostHold();
        //AI(agv-linklost-hold) 20260819 : Clean Out's P1-P3 force-release must stay reachable
        //while disconnected. It used to sit below this branch's return and was only harmless
        //because the release above had already cleared everything.
        if(HSys.Sys.RunMode==Run_CleanOut)
            ReleaseInfeedForCleanOut();
        return;
    }

    //AI(agv-linklost-hold) 20260819 : SELECTED again - drop the hold counters. The lock and the
    //handshake are deliberately NOT touched : ServiceHandshake resumes them from their real
    //state, so a handoff interrupted by a link blip continues instead of restarting.
    ClearLinkLostHold();

    //AI(ht160s-overcount-tripqueue S5) 20260721 : in Clean Out, release any pending INFEED
    //(P1-P3) handshake so a CALLED latched just before CleanOut entry does not stay stuck
    //(PollAndCall's normal CALLED->IDLE re-arm is below the RunMode gate, so it never runs
    //in CleanOut) and no infeed refill restarts mid-drain (which would reset the trip queue
    //+ re-feed). Outfeed (P4-P9 Auto unload) is intentionally NOT touched -- that is the D4
    //CleanOut unload path.
    if(HSys.Sys.RunMode==Run_CleanOut)
        ReleaseInfeedForCleanOut();

    //AI(amr-unmanned D4-2) 20260721 : the P4-P9 full-collect CALL now also runs in
    //Run_CleanOut (a drain GoUp can fill the output car; without a CALL the finish gate
    //IsAllCleanOutFinish blocks on the Full sensor forever = the latent silent stall).
    //P1-P3 shortage CALLs stay Run_Normal-only (infeed frozen in CleanOut, S5).
    if(HSys.Sys.RunMode!=Run_Normal && HSys.Sys.RunMode!=Run_CleanOut)
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
            // AI(ht160s-agv-devicecount) 20260713 : keep the SVID snapshot live for ad-hoc
            // S1F3 reads AND the AGVSupplement/AGVLDUnLDFinish Report 6. DeviceCount comes
            // from the per-Auto running IC total (GetAmrDeviceCount, tallied at discharge) -
            // NOT from summing the car's Tray[] grids, which are never filled with placed-IC
            // data and always summed to 0 (that was the original no-op bug).
            TrayCount[si] = Car->iTrayCount;
            DeviceCount[si] = AutoModule->GetAmrDeviceCount(a);
            CarrierID[si] = Car->CarID;
        }

        bool bFull = AutoModule->IsOutputCarFullForAmr(a) || AmrInject.AutoFull(a);   //AI(ht160s-agv) 20260708 : test-mode inject (handshake-only)
        // AI(ht160s-agv) 20260627 : full car + station idle -> CALL the AGV to collect it.
        if(bFull && Handshake[si]==AGV_IDLE)
        {
            AutoModule->SetAmrLock(a, true);
            SupplementBitmap = BuildBitmap(AgvStation[si].PIndex);
            Gem->EventReport(1, 272);   // CEID272 AGVSupplement
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

    //AI(amr-unmanned D4-2) 20260721 : infeed shortage CALLs stay Run_Normal-only (the
    //P4-P9 loop above also serves Run_CleanOut for the full-collect).
    if(HSys.Sys.RunMode!=Run_Normal)
        return;

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
            Gem->EventReport(1, 272);   // CEID272 AGVSupplement
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
    //AI(ht160s-home-resume-w5) 20260711 : HOME freeze (owner ruling D1 + FX(A)-2).
    //This runs on the SECS 1s timer with no RunMode gate, so a full-machine HOME used
    //to keep advancing the handshake : CEID273/274 fired off module state that
    //InitialAllTask was about to rebuild, and the PREP/READY watchdog aged through
    //the homing span (a slow HOME silently force-released a docked AMR's handshake).
    //Freeze transitions AND aging while homing; nothing is cleared, and InitialAllTask's
    //ReassertLocks() re-couples the module locks afterwards.
    //AI(amr-nohang) 20260825 : the fAllMotorHome==false term is REMOVED (owner ruling :
    //the handshake must never be the thing that hangs). Run_Home is BOUNDED - a HOME ends -
    //and is the actual window where InitialAllTask rebuilds module state on a different
    //thread cadence, so it stays. fAllMotorHome==false is UNBOUNDED : a servo alarm
    //(csystem.cpp:701 / :1319), EMG (:868), motor-power drop (:783 / :845) or a fresh
    //program start (cmydef.cpp:32) clears it and ONLY a completed HOME restores it. On
    //20260824 at KYEC that froze P1 for 27 minutes : the host re-sent START_AGV every 10s,
    //got HCACK=0 every time, and CEID273 was never even attempted (SECSGEM_TextLog_16).
    //Safe to drop : ServiceHandshake drives NO motor - it reads state, fires S6F11 and
    //releases locks. A released lock only lets a production ladder run, and production is
    //gated on SystemStart, which an un-homed machine has false anyway.
    {
        static bool s_bFreezeLogged=false;
        if(HSys.Sys.RunMode==Run_Home)
        {
            if(s_bFreezeLogged==false)
            {
                RecordProcess("AGV: handshake frozen for Run_Home (transitions + watchdog aging held)");   //AI(ht160s-obsv-p1)
                s_bFreezeLogged=true;
            }
            return;
        }
        s_bFreezeLogged=false;
    }

    for(int a = 0; a < AGV_AUTO_COUNT; a++)
    {
        int si = a + 3;
        unsigned char hsBefore = Handshake[si];
        if(Handshake[si]==AGV_PREP)
        {
            if(AutoModule->IsDrainedForAmr(a) || AmrInject.AutoDrained(a))   //AI(ht160s-agv) 20260708 : test-mode inject (SECS-273 gate only)
            {
                StatusBitmap = BuildBitmap(AgvStation[si].PIndex);
                Gem->EventReport(1, 273);   // CEID273 AGVLDUnLDStatus (Ready)
                Handshake[si] = AGV_READY;
            }
        }
        else if(Handshake[si]==AGV_READY)
        {
            if(AutoModule->IsAmrTaken(a) || AmrInject.AutoTaken(a))   //AI(ht160s-agv) 20260708 : test-mode inject (one-shot)
            {
                //AI(ht160s-agv-devicecount) 20260713 : final snapshot BEFORE ClearAmrCar
                //wipes the car, so the Report 6 SVIDs on THIS S6F11 carry the real closing
                //tray/IC count instead of the post-clear 0 (mirrors the HT9045 AutoUP
                //discipline of populate-then-send-then-reset, e.g. asendic_Auto.cpp:1642-1646).
                TMyCar *FinishCar = AutoModule->GetAutoCar(a);
                if(FinishCar!=NULL)
                {
                    TrayCount[si]   = FinishCar->iTrayCount;
                    DeviceCount[si] = AutoModule->GetAmrDeviceCount(a);
                }
                FinishBitmap = BuildBitmap(AgvStation[si].PIndex);
                Gem->EventReport(1, 274);   // CEID274 AGVLDUnLDFinish
                AutoModule->ClearAmrCar(a);
                TrayCount[si]   = 0;        //AI(ht160s-agv-devicecount) : car is now empty, keep the SVID snapshot honest
                DeviceCount[si] = 0;
                Handshake[si] = AGV_IDLE;
                AmrInject.ClearAutoCycle(a);   //AI(ht160s-agv) 20260720 : sim one-inject = one cycle (clear stuck level latch)
            }
        }
        //AI(amr-unmanned W3) 20260721 : Auto (P4-P9) handshake aging now covers CALLED too
        //(host never answered the 272 - previously CALLED sat silent forever) and, instead
        //of the old SILENT force-release (which unlocked the station and let a discharge
        //GoUp into a still-full car), latches TimeoutPending once. The MAIN loop (csystem)
        //pops WAR0962 there (never a modal on this SECS-timer path); K_RETRY -> RetryStation
        //re-CALLs. Lock and state are KEPT while pending so nothing moves into the full car.
        if((Handshake[si]==AGV_CALLED || Handshake[si]==AGV_PREP || Handshake[si]==AGV_READY)
           && Handshake[si]==hsBefore)
        {
            if(++ShortageDebounce[si] > GeneralSetting.iAgvTimeoutSec && TimeoutPending[si]==0)
            {
                RecordProcess("AGV: handshake timeout P"+IntToStr(si+1)+" (Auto"+IntToStr(a+1)+
                    ") after "+IntToStr(GeneralSetting.iAgvTimeoutSec)+"s -> WAR0962 pending");   //AI(amr-unmanned W3)
                TimeoutPending[si]   = 1;
                ShortageDebounce[si] = 0;
            }
        }
        else
        {
            ShortageDebounce[si] = 0;   // state changed (or idle) : restart the age
            TimeoutPending[si]   = 0;   // handshake moved on : stale pending is void
        }
    }

    //AI(ht160s-overcount-tripqueue S5) 20260721 : do NOT advance the INFEED (P1-P3) handoff
    //during Clean Out -- a mid-drain refill would EnqueueTrip + re-feed and stall the finish.
    //Outfeed (P4-P9) above already ran (the D4 unload path). P1-P3 is the last section so an
    //early return here skips only infeed. PollAndCall additionally forces P1-P3 to IDLE in
    //CleanOut, so this is belt-and-suspenders + removes any tick-ordering dependency.
    if(HSys.Sys.RunMode==Run_CleanOut)
        return;

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
                Gem->EventReport(1, 273);   // CEID273 AGVLDUnLDStatus (Ready)
                Handshake[p] = AGV_READY;
            }
        }
        else if(Handshake[p]==AGV_READY)
        {
            if(InfeedFinished(p) || AmrInject.InputFinish(p))   //AI(ht160s-agv) 20260708 : test-mode inject (one-shot)
            {
                FinishBitmap = BuildBitmap(AgvStation[p].PIndex);
                Gem->EventReport(1, 274);   // CEID274 AGVLDUnLDFinish
                InfeedSetLock(p, false);    // front destack may resume
                InfeedRefill(p);            // sim : restock the input stack to max
                Handshake[p] = AGV_IDLE;
                ShortageLatch[p] = 0;
                AmrInject.ClearInputCycle(p);   //AI(ht160s-agv) 20260720 : sim one-inject = one cycle (clear stuck level latch)
            }
        }
        //AI(amr-unmanned W3) 20260721 : P1 (Loader) keeps the legacy SILENT force-release
        //(its supply timeout is the S4 source-dry auto-CleanOut path, Q3 ruling - no WAR0962).
        //P2-P3 (Empty/Color supply) age CALLED+PREP+READY vs iAgvTimeoutSec and latch
        //TimeoutPending for the main-loop WAR0962 instead of silently releasing.
        if(p==0)
        {
            if((Handshake[p]==AGV_PREP || Handshake[p]==AGV_READY) && Handshake[p]==hsBefore)
            {
                if(++ShortageDebounce[p] > GeneralSetting.iAmrHandshakeWaitSec)
                {
                    RecordProcess("AGV: watchdog force-release P1 (Loader infeed) after "+
                        IntToStr(GeneralSetting.iAmrHandshakeWaitSec)+"s stuck handshake");   //AI(ht160s-obsv-p1)
                    InfeedSetLock(p, false);
                    Handshake[p]        = AGV_IDLE;
                    ShortageLatch[p]    = 0;
                    ShortageDebounce[p] = 0;
                }
            }
            else
                ShortageDebounce[p] = 0;   // state changed (or idle) : restart the age
        }
        else if((Handshake[p]==AGV_CALLED || Handshake[p]==AGV_PREP || Handshake[p]==AGV_READY)
                && Handshake[p]==hsBefore)
        {
            if(++ShortageDebounce[p] > GeneralSetting.iAgvTimeoutSec && TimeoutPending[p]==0)
            {
                RecordProcess("AGV: handshake timeout P"+IntToStr(p+1)+" (infeed) after "+
                    IntToStr(GeneralSetting.iAgvTimeoutSec)+"s -> WAR0962 pending");   //AI(amr-unmanned W3)
                TimeoutPending[p]   = 1;
                ShortageDebounce[p] = 0;
            }
        }
        else
        {
            ShortageDebounce[p] = 0;   // state changed (or idle) : restart the age
            TimeoutPending[p]   = 0;   // handshake moved on : stale pending is void
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
    //AI(ht160s-overcount-tripqueue S5) 20260721 : refuse an INFEED (P1-P3) handoff while the
    //machine is draining in Clean Out -- restarting infeed would EnqueueTrip a new car + re-
    //feed and stall the CleanOut finish. The CP name is a known station, so return true
    //(well-formed SECS reply, HCACK=0) but do NOT enter PREP, so no Ready(273) follows and
    //the host's own timeout handles it. Outfeed (Auto) is allowed.
    if(AgvStation[i].Kind!=ASK_AUTO && i<3 && HSys.Sys.RunMode==Run_CleanOut)
    {
        RecordProcess("AGV: START_AGV "+cpName+" ignored - infeed frozen during Clean Out");
        return true;
    }
    Handshake[i] = AGV_PREP;     // station asked to prepare for AGV handoff
    PrepDone[i]  = 0;
    if(AgvStation[i].Kind==ASK_AUTO && AutoModule!=NULL && AgvStation[i].AutoIndex>=0)
        AutoModule->SetAmrLock(AgvStation[i].AutoIndex, true);
    else if(AgvStation[i].Kind!=ASK_AUTO && i < 3)
        InfeedSetLock(i, true);   //AI(ht160s-agv) 20260623 : P1-P3 freeze front destack for the handoff
    return true;
}
//---------------------------------------------------------------------------
//AI(ht160s-home-resume-w5) 20260711 : post-HOME lock re-assert (D1 + FX(A)-1/FX(A)-5).
//InitialAllTask wipes every module bAmrLocked while Handshake[] survives (Reset runs
//only in the ctor) -> lock/state split : after a HOME during CALLED/PREP/READY the
//docked/en-route AMR keeps handshaking but the supposedly-frozen station resumes
//destacking/stacking into it. Called at the InitialAllTask tail : re-assert the module
//lock for every station whose handshake is still in flight. Locks are idempotent; a
//cold init has all stations IDLE so this is a no-op there. Infeed CALLED takes no lock
//by design (it is only locked from BeginPrep), mirroring the normal flow.
void TAgvCoordinator::ReassertLocks()
{
    AnsiString sLocked;   //AI(ht160s-obsv-p1) 20260720 : which stations got locks re-coupled
    for(int a = 0; a < AGV_AUTO_COUNT; a++)
    {
        int si = a + 3;
        if(Handshake[si]==AGV_CALLED || Handshake[si]==AGV_PREP || Handshake[si]==AGV_READY)
        {
            if(AutoModule!=NULL)
                AutoModule->SetAmrLock(a, true);
            sLocked+=" P"+IntToStr(si+1)+"(hs="+IntToStr(Handshake[si])+")";
        }
    }
    for(int p = 0; p < 3; p++)
    {
        if(Handshake[p]==AGV_PREP || Handshake[p]==AGV_READY)
        {
            InfeedSetLock(p, true);
            sLocked+=" P"+IntToStr(p+1)+"(hs="+IntToStr(Handshake[p])+")";
        }
    }
    if(sLocked!="")
        RecordProcess("HOME-RESUME AGV: locks re-asserted:"+sLocked);   //AI(ht160s-obsv-p1)
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
        AnsiString sBins = "";   //AI(ht160s-agv-binsetting) 20260713 : live SVID 38234-45 value per Auto
        if(AgvStation[i].Kind==ASK_AUTO && AgvStation[i].AutoIndex>=0)
            sBins = " bins=[" + DescribeAutoBins(AgvStation[i].AutoIndex) + "]";
        //AI(agv-linklost-hold) 20260819 : surface a lock that outlived the HSMS link so a
        //State Record post-mortem can tell "held across a link drop" from "handoff running".
        AnsiString sHold = "";
        if(LinkLostAge[i]!=0)
        {
            sHold = " linklost=" + IntToStr(LinkLostAge[i]) + "s";
            if(LinkLostPending[i]!=0)
                sHold = sHold + " WAR0963-pending";
        }
        s += "P" + IntToStr(AgvStation[i].PIndex) + " " + AnsiString(AgvStation[i].Name)
           + ": lock=" + IntToStr(iLock)
           + " hs="    + AnsiString(AgvHsName(Handshake[i]))
           + " ready=" + IntToStr(iReady) + sBins + sHold + "\r\n";
    }
    s += AmrInject.Describe();   //AI(ht160s-agv) 20260708 : test-mode + armed-latch state
    return s;
}
//---------------------------------------------------------------------------
// AI(ht160s-agv-binsetting) 20260713 : build the "bin setting" string for one Auto
// (SVID 38234-38236 / 38243-38245). Semantics mirror HT9045 AMRUnloadBin : it tells the
// AMR/host which sort-result bin(s) land in that Auto's output car. HT160 has three
// routing models (GeneralSetting.iSortMode) :
//   smNormal      : BinAreaMap is a 1:1 Bin<->Area bijection, so each Auto carries
//                   exactly one bin (GetBinByArea). If this Auto is also the Error /
//                   overflow area, append an "ERR" marker (it additionally collects
//                   2D-scan-fail / no-bin-setting ICs plus any overflow).
//   smLotBin      : dynamic (LotID,Bin)->Auto bindings; emit "LotID:Bin" tokens.
//   smLotPassFail : dynamic (LotID,PASS/FAIL)->Auto bindings; emit "LotID:PASS"/":FAIL".
// A bin number alone is meaningless in the dynamic modes (the same Auto means a
// different grade per lot), hence the LotID prefix (user-confirmed 20260713). Read-only.
AnsiString TAgvCoordinator::DescribeAutoBins(int AutoIndex)
{
    if(AutoIndex < 0 || AutoIndex >= AGV_AUTO_COUNT)
        return "";

    if(GeneralSetting.IsDynamicBindingMode())
    {
        AnsiString s = "";
        int n = LotBinBinding.GetBindingCount();
        for(int i = 0; i < n; i++)
        {
            AnsiString LotID;
            int Key;
            int BoundAuto;
            if(LotBinBinding.GetBindingByIndex(i, LotID, Key, BoundAuto) == false)
                continue;
            if(BoundAuto != AutoIndex)
                continue;
            AnsiString token;
            if(GeneralSetting.IsLotPassFailSortMode())
            {
                AnsiString kt;   //AI(bcb6-ternary) 20260723 : nested ?: -> AnsiString miscompiles in BCB6; use if/else
                if(Key == 1) kt = AnsiString("PASS");
                else if(Key == 2) kt = AnsiString("FAIL");
                else kt = IntToStr(Key);
                token = LotID + ":" + kt;
            }
            else
                token = LotID + ":" + IntToStr(Key);
            s = (s == "") ? token : (s + "," + token);
        }
        return s;
    }

    // smNormal : the single bin mapped to this Auto area, plus an Error marker if this
    // Auto is the configured error / overflow target.
    int Area = eHT160BinAreaAuto1 + AutoIndex;
    AnsiString s = "";
    int Bin = BinAreaMap.GetBinByArea(Area);
    if(Bin > 0)
        s = IntToStr(Bin);
    if(BinAreaMap.GetErrorBinArea() == Area)
        s = (s == "") ? AnsiString("ERR") : (s + ",ERR");
    return s;
}
//---------------------------------------------------------------------------
// AI(ht160s-agv-binsetting) 20260713 : repopulate all six Auto BinSetting snapshots from
// the live routing config. Called every 1s from HT160Gem::ServiceAgv, ungated by RunMode
// / link so a host config-time S1F3 read while the machine is idle still sees a current
// value. Gated on bUseAMR : no point maintaining AMR SVIDs when AMR is off.
void TAgvCoordinator::RefreshBinSettings()
{
    if(GeneralSetting.bUseAMR == false)
        return;
    for(int a = 0; a < AGV_AUTO_COUNT; a++)
        BinSetting[a] = DescribeAutoBins(a);
}
//---------------------------------------------------------------------------
