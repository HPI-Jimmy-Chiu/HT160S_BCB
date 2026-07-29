//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "uHGemHT160.h"
//AI(ht160s-secsgem) 20260610 : includes for SET_LOT_INFO handler
#include "main.h"          // fMain, edLotNo
#include "database.h"      // HSys.Sys.SystemStart / RunMode
#include "csystem.h"       // HasICUnderMachine()
#include "CosFunction.h"   // LotRegistry, HT160_LOT_SOURCE_SECS, HT160_MAX_LOT
//AI(ht160s-secsgem) 20260611 : includes for live SV snapshot sources
#include "cprod.h"         // tRunData (TotalIC/UPH), MachineRun (iTotalSorted)
#include "cSoterOutput.h"
#include "note.h"          // fNote (alarm dialog : fShow / Code)
#include "cmydef.h"        // SoftStop (S2F42 PAUSE host command)
#include "uAgvStation.h"   // AI(ht160s-agv) 20260615 : E87/AGV station table + AgvCoord
#include "uAmrInject.h"   // AI(ht160s-agv) 20260708 : AMR manual-inject alert (HCACK!=0 surfacing)
#include "UsecegemMainFrom.h" // AI(ht160s-secsgem) 20260715 : ComputeAlarmAlid (S5 ALID SSOT)
#include "GeneralSetting.h" // AI(ht160s-whitelist) 20260715 : IsWhiteListSortMode()
#include "maintenance.h"    // AI(ht160s-whitelist) 20260716 : SyncSortModeSelectorFromSetting (SECS SORTMODE UI sync)
#include "cEventLog.h"      // AI(secs-msggap) 20260728 : g_EventLog (S10F3/S10F5 host terminal text)
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
struct ETypeStruct SECS_EVENT;
char GEM_MachineName[16] = "HT160S";
//---------------------------------------------------------------------------
HT160Gem::HT160Gem(AnsiString Path, THGem *HGemTmp)
    : HTGem(Path, HGemTmp)
{
    iControlState = 0;
    sSystemTime = Now().FormatString("yyyy/mm/dd hh:nn:ss");

    //AI(ht160s-secsgem) 20260611 : init SV snapshot members (refreshed on demand)
    svRunMode       = 0;
    svSystemRunning = 0;
    svAlarmActive   = 0;
    svAlarmCode     = 0;
    svTotalIC       = 0;
    svTotalSorted   = 0;
    svUPH           = 0;
    svLotCount      = 0;
    svCurrentLot    = "";
    svSoftwareVersion = "1.0.0.0";   // keep in step with ht160s.cpp GemInitial("HT160S","1.0.0.0")
    ecRecipeName    = "";

    //AI(ht160s-secsgem) 20260610 : wire transport->logic dispatch back-pointer
    if(HGemPtr!=NULL)
        HGemPtr->SetGemLogic(this);

    EventDescription[SECS_EVENT.HandlerStatus       ] = "1 Handler change status";
    EventDescription[SECS_EVENT.RecipeChange        ] = "2 Recipe Change";
    EventDescription[SECS_EVENT.ClearCount          ] = "3 Press Clear Count button";
    EventDescription[SECS_EVENT.PressStartWithoutIC ] = "4 Press Start button without IC inside handler";
    EventDescription[SECS_EVENT.PressStartWithIC    ] = "5 Press Start button with IC inside handler";
    EventDescription[SECS_EVENT.PressPause          ] = "6 Press Pause button";
    EventDescription[SECS_EVENT.PressHome           ] = "7 Press Home button";
    EventDescription[SECS_EVENT.PressOneCycle       ] = "8 Press One Cycle button";
    EventDescription[SECS_EVENT.PressCleanOut       ] = "9 Press Clean Out button";
    EventDescription[SECS_EVENT.PressTrayFeed       ] = "10 Press Tray Feed button";
    EventDescription[SECS_EVENT.PressLotStart       ] = "11 Press Lot Start button";
    EventDescription[SECS_EVENT.PressLotEnd         ] = "12 Press Lot End button";
    EventDescription[SECS_EVENT.PressExit           ] = "13 Press Exit button";
    EventDescription[SECS_EVENT.PressRetry          ] = "14 Press Retry button";
    EventDescription[SECS_EVENT.PressSkip           ] = "15 Press Skip button";
    EventDescription[SECS_EVENT.PressAlarmReset     ] = "16 Press Alarm Reset button";
    EventDescription[SECS_EVENT.ShowAlarm           ] = "17 Show Alarm";
    EventDescription[SECS_EVENT.ReleaseAlarm        ] = "18 Release Alarm";
    EventDescription[SECS_EVENT.ShowMessage         ] = "19 Show Message";
    EventDescription[SECS_EVENT.ReleaseMessage      ] = "20 Release Message";
    EventDescription[SECS_EVENT.ChangeUser          ] = "21 Switching User Level";
    EventDescription[SECS_EVENT.EnterSetup          ] = "22 Enter Setup Page";
    EventDescription[SECS_EVENT.EnterMaintenPage    ] = "23 Enter Maintenance Page";
    EventDescription[SECS_EVENT.EnterIOPage         ] = "24 Enter I/O Page";
    EventDescription[SECS_EVENT.EnterTeach          ] = "25 Enter Teach Page";
    EventDescription[SECS_EVENT.EnterSECSPage       ] = "26 Enter SECS GEM Page";
    EventDescription[SECS_EVENT.OneCycleOK          ] = "27 One Cycle Finish";
    EventDescription[SECS_EVENT.CleanOutOK          ] = "28 Clean Out Finish";
    EventDescription[SECS_EVENT.TrayFeedOK          ] = "29 Tray Feed Finish";
    EventDescription[SECS_EVENT.TimeEvent           ] = "30 Time Event";
    EventDescription[SECS_EVENT.RealDummy           ] = "31 Switching Real/Dummy Mode";
}
//---------------------------------------------------------------------------
HT160Gem::~HT160Gem()
{
    //AI(secs-audit-fix) 20260729 : un-wire the transport->logic back-pointer set in the ctor.
    //THGem::OnPeerDisconnected() calls GemLogic->OnCommunicationLost(), and THGem outlives us
    //whenever the global HSys (which owns HSys.MyGem == this) is destroyed before Application
    //(which owns HGem). Static-dtor order is not guaranteed here, so both ends clear the link :
    //~THGem nulls GemLogic before it stops its sockets, and we null it from this side too.
    if(HGemPtr!=NULL)
        HGemPtr->SetGemLogic(NULL);
}
//---------------------------------------------------------------------------
void HT160Gem::AddSV()
{
    sSystemTime = Now().FormatString("yyyy/mm/dd hh:nn:ss");
    if(HGemPtr==NULL)
        return;

    //AI(ht160s-secsgem) 20260612 : SVID numbering follows HT9045 AddSV for the
    // common identity/time/throughput band (1001 Machine Model, 1003 Software
    // Version, 1021 UPH, 1027 System Time) so a shared host SVID table works across
    // machines. HT160-specific run/alarm/output/lot SVs are relocated to a high
    // 66000+ band (9045 max SVID 65095 + 1000, rounded) so they can never collide
    // with 9045's 1010-1190 count region or any future 9045 SVID growth.
    // -- 9045-aligned common band --
    HGemPtr->SetSVDataPointer(1001, HType.ASCII_TYPE, "Machine Model", "", &HandlerPath, "machine model name (9045 SVID 1001)");
    HGemPtr->SetSVDataPointer(1003, HType.ASCII_TYPE, "Software Version", "", &svSoftwareVersion, "application software version (9045 SVID 1003)");
    HGemPtr->SetSVDataPointer(1021, HType.INT_4_TYPE, "UPH", "pcs/hr", &svUPH, "units per hour (9045 SVID 1021)");
    HGemPtr->SetSVDataPointer(1027, HType.ASCII_TYPE, "System Time", "", &sSystemTime, "current system time (9045 SVID 1027)");

    //AI(secs-gem-std) 20260727 : SVID 1518 Real/Dummy, 9045-aligned. Bound DIRECTLY to the live
    // HSys.LastSet.iRealDummy (int; DUMMY=0 / HAS_TRAY=1 / REALLY=2 already match the 9045
    // 0:Dummy / 1:Tray Only / 2:Real encoding) so a host RPTID-502 read gets the true mode
    // with no snapshot. The KYEC CJ_EAP host references SVID 1518 in report RPTID 502.
    HGemPtr->SetSVDataPointer(1518, HType.INT_4_TYPE, "Real/Dummy", "", &HSys.LastSet.iRealDummy, "0=Dummy 1=Tray Only 2=Real (9045 SVID 1518)");

    //AI(ht160s-secsgem) 20260612 : HT160 custom high band (66000+). Values are
    // snapshotted into the sv* members by RefreshSVData() right before each
    // serialize, so the bound address stays valid while the reported value tracks
    // live data. Sub-grouped with gaps (66000 state / 66010 alarm / 66020 output /
    // 66030 lot) so each group can grow without renumbering. These are HT160-only
    // semantics; 9045 has no equivalent SVID here.
    // -- Machine state (66000-66009) --
    HGemPtr->SetSVDataPointer(66000, HType.INT_4_TYPE, "Run Mode", "", &svRunMode, "0=Normal 1=Home 2=OneCycle 3=CleanOut 4=TrayFeed");
    HGemPtr->SetSVDataPointer(66001, HType.INT_4_TYPE, "System Running", "", &svSystemRunning, "1=machine started/running, 0=stopped");
    HGemPtr->SetSVDataPointer(66002, HType.INT_4_TYPE, "Control State", "", &iControlState, "GEM control state mirror (4=Local 5=Remote)");
    // -- Alarm state (66010-66019) --
    HGemPtr->SetSVDataPointer(66010, HType.INT_4_TYPE, "Alarm Active", "", &svAlarmActive, "1=alarm dialog showing, 0=no alarm");
    HGemPtr->SetSVDataPointer(66011, HType.INT_4_TYPE, "Alarm Code", "", &svAlarmCode, "current active alarm code (0=none)");
    // -- Output / production (66020-66029) --
    HGemPtr->SetSVDataPointer(66020, HType.INT_4_TYPE, "Total IC", "pcs", &svTotalIC, "total IC processed this lot/run");
    HGemPtr->SetSVDataPointer(66021, HType.INT_4_TYPE, "Total Sorted", "pcs", &svTotalSorted, "total IC sorted into a bin");
    // -- Current Lot (66030-66039) --
    HGemPtr->SetSVDataPointer(66030, HType.INT_4_TYPE, "Active Lot Count", "", &svLotCount, "lots currently loaded on the machine");
    HGemPtr->SetSVDataPointer(66031, HType.ASCII_TYPE, "Current Lot ID", "", &svCurrentLot, "first registered lot id (empty if none)");
    //AI(ht160s-whitelist) 20260716 : Q6 host read-back of the active sort mode. Bound to the
    // live config int (stable global address, read at serialize time) so a SORTMODE switch via
    // S2F41 LOTSTART is confirmable by S1F3. No RefreshSVData mirror : not a per-cycle snapshot.
    //AI(ht160s-whitelist-override) 20260717 : report the EFFECTIVE mode (base + WhiteList overlay),
    // not the raw base, so the host reads WHITELIST during a WhiteList lot and the base between lots.
    //AI(ht160s-whitelist) 20260727 : WhiteList is NOT a 4th entry of the maintenance Sort Mode
    // selector (rgSortMode stays 3-way : 0/1/2). Value 3 means "the per-lot WhiteList overlay is
    // armed", whatever the base is; between lots the base value comes back. Matching semantic on
    // the command side : SORTMODE=NORMAL means "no overlay, keep the operator's base selector",
    // NOT "set base to 0", so a NORMAL lot can legitimately read back 1 or 2 here.
    HGemPtr->SetSVDataPointer(66032, HType.INT_4_TYPE, "Sort Mode", "", &GeneralSetting.iEffectiveSortMode, "effective: 0/1/2=base Normal/LotBin/LotPassFail, 3=WhiteList overlay armed");

    //AI(ht160s-agv) 20260615 : E87/AGV SVIDs (draft 38202-38245), bound to the
    // AgvCoord snapshot block (stable addresses). Bitmaps 38219-38221 are written
    // by the coordinator right before each AGV S6F11; carrier id / counts are
    // refreshed by the coordinator before the relevant events. SVIDs are not all
    // contiguous past Auto3, so they come from the station table, not a base+offset.
    HGemPtr->SetSVDataPointer(38219, HType.ASCII_TYPE, "Supplement Bin",     "", &AgvCoord.SupplementBitmap, "AGVSupplement P1-P9 bitmap");
    HGemPtr->SetSVDataPointer(38220, HType.ASCII_TYPE, "LD UnLD Check AGV",  "", &AgvCoord.StatusBitmap,     "AGVLDUnLDStatus P1-P9 bitmap");
    HGemPtr->SetSVDataPointer(38221, HType.ASCII_TYPE, "LD UnLD Finish AGV", "", &AgvCoord.FinishBitmap,     "AGVLDUnLDFinish P1-P9 bitmap");
    for(int ai = 0; ai < AGV_STATION_COUNT; ai++)
    {
        const TAgvStationDesc *d = &AgvStation[ai];
        AnsiString cName, tName, vName;
        cName.sprintf("%s Carrier ID", d->Name);
        tName.sprintf("AMR %s Tray Count", d->Name);
        vName.sprintf("AMR %s Device Count", d->Name);
        HGemPtr->SetSVDataPointer(d->SvidCarrierID, HType.ASCII_TYPE, cName, "",      &AgvCoord.CarrierID[ai],   "AGV carrier id");
        HGemPtr->SetSVDataPointer(d->SvidTrayCount, HType.INT_4_TYPE,  tName, "trays", &AgvCoord.TrayCount[ai],   "AGV tray count");
        HGemPtr->SetSVDataPointer(d->SvidDeviceCnt, HType.INT_4_TYPE,  vName, "pcs",   &AgvCoord.DeviceCount[ai], "AGV device count");
        if(d->SvidBinSet != 0 && d->AutoIndex >= 0)
        {
            AnsiString bName;
            bName.sprintf("AMR %s Bin Setting", d->Name);
            HGemPtr->SetSVDataPointer(d->SvidBinSet, HType.ASCII_TYPE, bName, "", &AgvCoord.BinSetting[d->AutoIndex], "AGV bin setting");
        }
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : snapshot live machine data into SV members.
// Called from THGem::EventReport (before S6F11) and S1F4_SelectedStatusReply.
// Kept NULL-safe (fNote may not exist yet) and type-safe (no bool/enum binding).
void HT160Gem::RefreshSVData()
{
    sSystemTime = Now().FormatString("yyyy/mm/dd hh:nn:ss");

    svRunMode       = (int)HSys.Sys.RunMode;
    svSystemRunning = HSys.Sys.SystemStart ? 1 : 0;

    svAlarmActive   = (fNote!=NULL && fNote->fShow) ? 1 : 0;
    svAlarmCode     = (fNote!=NULL) ? fNote->Code : 0;

    svTotalIC       = tRunData.TotalIC;
    svTotalSorted   = MachineRun.iTotalSorted;
    svUPH           = tRunData.UPH;

    svLotCount      = LotRegistry.GetLotCount();
    if(svLotCount>0)
    {
        TLotRunInfo *Lot = LotRegistry.GetLot(0);
        svCurrentLot = (Lot!=NULL) ? Lot->sLotID : AnsiString("");
    }
    else
        svCurrentLot = "";
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260612 : 1s tick from THGem::Timer1Timer. Sync the main-
// screen SECS status badge to the live HSMS link state. NULL-safe (fMain may not
// exist during very early boot / shutdown). The actual edge-trigger + repaint is
// inside TfMain::UpdateSecsFeatureBadge() so this stays a thin forwarder.
void HT160Gem::RefreshSecsBadge()
{
    if(fMain!=NULL)
        fMain->UpdateSecsFeatureBadge();
}
//---------------------------------------------------------------------------
//AI(ht160s-agv) 20260615 : 1s tick from THGem::Timer1Timer. Drive the E87/AGV
// coordinator : Phase B poll (Auto car full -> CEID272 AGVSupplement) + Phase D
// handshake service. AgvCoord fires S6F11 through HGemPtr only while SELECTED, so
// this is safe to call every tick regardless of link state.
void HT160Gem::ServiceAgv()
{
    //AI(ht160s-agv-binsetting) 20260713 : keep SVID 38234-45 (per-Auto bin setting) live
    // for host S1F3. Config-derived + RunMode/link-independent, so it runs BEFORE the
    // RunMode-gated PollAndCall (a host reads bin setting at config time, machine idle).
    AgvCoord.RefreshBinSettings();
    AgvCoord.PollAndCall(HGemPtr);
    AgvCoord.ServiceHandshake(HGemPtr);
}
//---------------------------------------------------------------------------
void HT160Gem::AddEC()
{
    if(HGemPtr==NULL)
        return;

    //AI(ht160s-secsgem) 20260611 : Equipment Constant catalog (modeled on
    // HT9045Gem::AddEC). Two binding styles on purpose:
    //  - Tray-form geometry is bound DIRECTLY to the live THT160TrayForm struct
    //    members (persistent + type-matched: double->FT_8, int->INT_4). This is
    //    the 9045 "&USE_RFID_READER" style: the host always reads the current
    //    value with no snapshot, and a future S2F15 write lands straight on the
    //    struct that Loader/SortArm/Monitor already read from.
    //  - Recipe name has no single stable address (it lives behind
    //    RecipeManager's getter/normalizer), so it is reported from the
    //    ecRecipeName snapshot refreshed in S2F14_EquipmentConstanData().
    // -- Recipe selection --
    HGemPtr->SetECDataPointer(1501, HType.ASCII_TYPE, "Recipe Name",     "",   &ecRecipeName,      "", "", "Default", "current recipe (Setup File) name");
    //AI(ht160s-secsgem) 20260612 : Tray Form geometry aligned to HT9045 "Type 1
    //  Tray" band (9045 ECID 2758-2763) instead of the old 2011-2016 (which collided
    //  with 9045's 2011 X-Dimension / 2012 Y-Dimension / 2013 Kit Diameter contact-
    //  force band). HT160 currently exposes a single tray geometry, bound directly to
    //  the live THT160TrayForm struct (persistent + type-matched).
    //  data\<recipe>\setup.ini [TrayForm]
    HGemPtr->SetECDataPointer(2760, HType.FT_8_TYPE,  "Tray X Start",    "mm", &TrayForm.XStart,   "", "", "0", "tray X start pos (9045 ECID 2760 Type1 Start Position X)");
    HGemPtr->SetECDataPointer(2758, HType.FT_8_TYPE,  "Tray X Pitch",    "mm", &TrayForm.XPitch,   "", "", "0", "tray X pitch (9045 ECID 2758 Type1 Pitch X)");
    HGemPtr->SetECDataPointer(2761, HType.FT_8_TYPE,  "Tray Y Start",    "mm", &TrayForm.YStart,   "", "", "0", "tray Y start pos (9045 ECID 2761 Type1 Start Position Y)");
    HGemPtr->SetECDataPointer(2759, HType.FT_8_TYPE,  "Tray Y Pitch",    "mm", &TrayForm.YPitch,   "", "", "0", "tray Y pitch (9045 ECID 2759 Type1 Pitch Y)");
    HGemPtr->SetECDataPointer(2762, HType.INT_4_TYPE, "Tray X Division", "",   &TrayForm.XDivision,"", "", "0", "tray columns / X count (9045 ECID 2762 Type1 Division X)");
    HGemPtr->SetECDataPointer(2763, HType.INT_4_TYPE, "Tray Y Division", "",   &TrayForm.YDivision,"", "", "0", "tray rows / Y count (9045 ECID 2763 Type1 Division Y)");
    //AI(ht160s-secsgem) 20260612 : RESERVED tray-type slots aligned to HT9045 for
    //  future HT160 multi-tray support. NOT registered yet (HT160 has no Type2/3
    //  data source / values today). When HT160 gains Type 2/3 tray geometries,
    //  register them on these reserved IDs and widen the S2F16 / GuiWriteTrayEC
    //  settable range accordingly:
    //    Type 2 -> 2771 Pitch X / 2772 Pitch Y / 2773 Start X / 2774 Start Y / 2775 Div X / 2776 Div Y
    //    Type 3 -> 2784 Pitch X / 2785 Pitch Y / 2786 Start X / 2787 Start Y / 2788 Div X / 2789 Div Y
}
//---------------------------------------------------------------------------
void HT160Gem::AddAlarmList()
{
    //AI(ht160s-secsgem) 20260715 : HT160 has no alarm StringGrid and needs no pre-built
    // store - the S5F6/S5F8 catalog is emitted LIVE from HSys.mapAlarmCodeList (the SSOT
    // populated by CreateSystemAlarmCode). Nothing to build here; kept for framework parity.
}
//---------------------------------------------------------------------------
void HT160Gem::AddCEID()
{
    int EquDefault = 1;
    unsigned ReportID[1];
    ReportID[0] = 1;

    if(HGemPtr==NULL)
        return;

    for(int i=SECS_EVENT.HandlerStatus; i<SECS_EVENT.TotalEvent; i++)
    {
        HGemPtr->SetCEIDContent(i, EventDescription[i], 1, ReportID, EquDefault);
    }

    //AI(ht160s-agv) 20260615 : E87/AGV events mapped to DEDICATED reports (2/3/4/5)
    // NOT report 1, so the host receives the P-bitmap / carrier id, not the 13
    // machine-status SVs that every other CEID carries. Report content is defined
    // in AddReprot() (runs after AddCEID; EventReport resolves report->SV at send).
    //AI(ht160s-agv-devicecount) 20260713 : 272 (call) and 274 (finish) additionally
    // carry report 6 (all-station Tray/Device Count, see AddReprot) so the host gets
    // the real closing tray+IC numbers on the SAME S6F11 as the bitmap, no follow-up
    // S1F3 needed. 273 (Ready) stays bitmap-only : nothing has been counted yet.
    unsigned rptSup[2]; rptSup[0] = 2; rptSup[1] = 6;
    unsigned rptSta[1]; rptSta[0] = 3;
    unsigned rptFin[2]; rptFin[0] = 4; rptFin[1] = 6;
    unsigned rptCid[1]; rptCid[0] = 7;   //AI(ht160s-agv-identity2d) 20260713 : CEID275 -> dedicated report 7 ([38202] only), NOT the 9-wide report 5 (AGVLdID must not ship 8 stale carrier ids)
    HGemPtr->SetCEIDContent(272, "AGVSupplement",   2, rptSup, EquDefault);
    HGemPtr->SetCEIDContent(273, "AGVLDUnLDStatus", 1, rptSta, EquDefault);
    HGemPtr->SetCEIDContent(274, "AGVLDUnLDFinish", 2, rptFin, EquDefault);
    HGemPtr->SetCEIDContent(275, "AGVLdID",         1, rptCid, EquDefault);

    //AI(ht160s-secsgem) 20260625 : two-stage Auto Full pre-notification. Register the
    // discrete Full CEIDs (9045-aligned : Auto1-3=35/36/37, Auto4-6=148/149/150) on
    // report 1 (machine context). Emitted from uAgvStation PollAndCall on the car-full
    // edge. The matching Unloadtray CEIDs (136-138/145-147, fired in aAuto1To6
    // DoDischargeTray) stay unregistered/empty on purpose : lightweight + 9045-faithful.
    unsigned AutoFullCeid[6] = {35, 36, 37, 148, 149, 150};
    for(int af=0; af<6; af++)
    {
        AnsiString DescAutoFull;
        DescAutoFull.sprintf("Auto%d Full", af+1);
        HGemPtr->SetCEIDContent(AutoFullCeid[af], DescAutoFull, 1, ReportID, EquDefault);
    }
}
//---------------------------------------------------------------------------
void HT160Gem::AddReprot()
{
    int EquDefault = 1;
    //AI(ht160s-secsgem) 20260612 : report 1 carries the real machine-data SVs on
    // every event, using the 9045-aligned numbering (1001/1003/1021/1027) plus the
    // HT160 custom high band (66000+).
    unsigned ReportIDContent[16];

    if(HGemPtr==NULL)
        return;

    ReportIDContent[ 0] = 1001;  // Machine Model     (9045-aligned)
    ReportIDContent[ 1] = 1003;  // Software Version  (9045-aligned)
    ReportIDContent[ 2] = 1021;  // UPH               (9045-aligned)
    ReportIDContent[ 3] = 1027;  // System Time       (9045-aligned)
    ReportIDContent[ 4] = 66000; // Run Mode          (HT160 custom band)
    ReportIDContent[ 5] = 66001; // System Running
    ReportIDContent[ 6] = 66002; // Control State
    ReportIDContent[ 7] = 66010; // Alarm Active
    ReportIDContent[ 8] = 66011; // Alarm Code
    ReportIDContent[ 9] = 66020; // Total IC
    ReportIDContent[10] = 66021; // Total Sorted
    ReportIDContent[11] = 66030; // Active Lot Count
    ReportIDContent[12] = 66031; // Current Lot ID
    HGemPtr->SetReportIDContent(1, 13, ReportIDContent, EquDefault);

    //AI(ht160s-agv) 20260615 : dedicated AGV reports. 2/3/4 each carry a single
    // P1-P9 bitmap SV (supplement/status/finish); 5 carries all nine carrier IDs.
    unsigned rSup[1]; rSup[0] = 38219; HGemPtr->SetReportIDContent(2, 1, rSup, EquDefault);
    unsigned rSta[1]; rSta[0] = 38220; HGemPtr->SetReportIDContent(3, 1, rSta, EquDefault);
    unsigned rFin[1]; rFin[0] = 38221; HGemPtr->SetReportIDContent(4, 1, rFin, EquDefault);
    unsigned rCid[AGV_STATION_COUNT];
    for(int ci = 0; ci < AGV_STATION_COUNT; ci++)
        rCid[ci] = AgvStation[ci].SvidCarrierID;
    HGemPtr->SetReportIDContent(5, AGV_STATION_COUNT, rCid, EquDefault);

    //AI(ht160s-agv-devicecount) 20260713 : report 6 = all nine stations' Tray Count
    // then all nine Device Count SVIDs (same fixed P1-P9 order as report 5's carrier
    // ids), attached to CEID272/274 (AddCEID) so the host reads the real per-station
    // tray+IC numbers off the same S6F11 as the bitmap.
    unsigned rCnt[AGV_STATION_COUNT * 2];
    for(int ni = 0; ni < AGV_STATION_COUNT; ni++)
        rCnt[ni] = AgvStation[ni].SvidTrayCount;
    for(int ni = 0; ni < AGV_STATION_COUNT; ni++)
        rCnt[AGV_STATION_COUNT + ni] = AgvStation[ni].SvidDeviceCnt;
    HGemPtr->SetReportIDContent(6, AGV_STATION_COUNT * 2, rCnt, EquDefault);

    //AI(ht160s-agv-identity2d) 20260714 : report 7 = ONLY the identity carrier SVID
    // (AgvStation[AMR_IDENTITY_CARRIER_INDEX].SvidCarrierID = Color P3 / SVID 38204). CEID275
    // (AGVLdID) links to THIS (AddCEID rptCid[0]=7), so the AGVLdID event carries just the
    // freshly-scanned identity-tray 2D and does NOT ship the 8 other per-station carrier ids
    // (which may be stale). The Auto-stack carrier ids stay host-pollable via S1F3; if the host
    // wants them evented it can link report 5 via S2F35. Matches HT9045 (AGVLdID = load id; Auto
    // carrier ids poll-only). SVID derived from the single change-point constant so the stamped
    // CarrierID[] index and the reported SVID stay locked to one edit.
    unsigned rLdId[1]; rLdId[0] = AgvStation[AMR_IDENTITY_CARRIER_INDEX].SvidCarrierID;
    HGemPtr->SetReportIDContent(7, 1, rLdId, EquDefault);

    //AI(secs-reportdef) 20260724 : premature save removed - persistence loads at boot via ReadEventReportData and saves on host S2F33/F37 only
}
//---------------------------------------------------------------------------
int HT160Gem::S2F15_CheckNewEquipmentConstant()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S2F15 EC update is disabled for HT160 framework skeleton");
    return 1;
}
//---------------------------------------------------------------------------
int HT160Gem::S2F15_UpdateNewEquipmentConstant()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S2F15 EC update is disabled for HT160 framework skeleton");
    return 1;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : S2F13 Equipment Constant Request -> S2F14.
//  Mirror of S1F3/S1F4 but over the EC registry. Request body = L,n { ECID };
//  n==0 means "report all EC". Reply body = L,m { <EC live value> }. Tray-form
//  ECs read straight from THT160TrayForm; the recipe-name EC is refreshed from
//  RecipeManager into ecRecipeName first (no stable address to bind).
//---------------------------------------------------------------------------
void HT160Gem::S2F14_EquipmentConstanData()
{
    int n, len, i;
    unsigned char Type;
    AnsiString S;
    unsigned reqList[512];
    int reqCount = 0;

    if(HGemPtr==NULL)
        return;

    HGemPtr->ResetReturnCode();

    //AI(ht160s-secsgem) 20260611 : refresh snapshot-backed EC value (recipe name)
    ecRecipeName = RecipeManager.GetCurrentRecipeName();

    if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
    {
        // malformed request -> reply empty list
        HGemPtr->InitLocalHead(2, 14, 0);
        HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
        HGemPtr->SendLocalData();
        return;
    }

    if(n<=0)
    {
        // report every registered EC
        int total = HGemPtr->GetECCount();
        HGemPtr->InitLocalHead(2, 14, 0);
        HGemPtr->DataItemOut(total, HType.LIST_TYPE, NULL);
        for(i=0; i<total; i++)
            HGemPtr->DataItemOutECValue(HGemPtr->GetECIDByIndex(i));
        HGemPtr->SendLocalData();
        return;
    }

    // read requested ECIDs first (this consumes the receive buffer)
    for(i=0; i<n && reqCount<512; i++)
    {
        if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
            break;
        S = "";
        if(HGemPtr->DataItemIn(len, Type, S)==1)
            reqList[reqCount++] = (unsigned)StrToIntDef(S, 0);
        else
            reqList[reqCount++] = 0;     // unreadable -> empty item
    }

    HGemPtr->InitLocalHead(2, 14, 0);
    HGemPtr->DataItemOut(reqCount, HType.LIST_TYPE, NULL);
    for(i=0; i<reqCount; i++)
        HGemPtr->DataItemOutECValue(reqList[i]);
    HGemPtr->SendLocalData();
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : S1F3 Selected Equipment Status Request -> S1F4.
//  Request body = L,n { SVID ... }; n==0 means "report all SV". Reply body =
//  L,m { <SV live value> ... } in request order (or registry order for all).
//  SVIDs are read via the AnsiString numeric overload so U1/U2/U4/I1/I2/I4 all
//  parse uniformly; unknown SVID encodes an empty L[0] item.
//---------------------------------------------------------------------------
void HT160Gem::S1F4_SelectedStatusReply()
{
    int n, len, i;
    unsigned char Type;
    AnsiString S;
    unsigned reqList[512];
    int reqCount = 0;

    if(HGemPtr==NULL)
        return;

    HGemPtr->ResetReturnCode();

    //AI(ht160s-secsgem) 20260611 : snapshot live machine data before replying
    RefreshSVData();

    if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
    {
        // malformed request -> reply empty list
        HGemPtr->InitLocalHead(1, 4, 0);
        HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
        HGemPtr->SendLocalData();
        return;
    }

    if(n<=0)
    {
        // report every registered SV
        int total = HGemPtr->GetSVCount();
        HGemPtr->InitLocalHead(1, 4, 0);
        HGemPtr->DataItemOut(total, HType.LIST_TYPE, NULL);
        for(i=0; i<total; i++)
            HGemPtr->DataItemOutSVValue(HGemPtr->GetSVIDByIndex(i));
        HGemPtr->SendLocalData();
        return;
    }

    // read requested SVIDs first (this consumes the receive buffer)
    for(i=0; i<n && reqCount<512; i++)
    {
        if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
            break;
        S = "";
        if(HGemPtr->DataItemIn(len, Type, S)==1)
            reqList[reqCount++] = (unsigned)StrToIntDef(S, 0);
        else
            reqList[reqCount++] = 0;     // unreadable -> empty item
    }

    HGemPtr->InitLocalHead(1, 4, 0);
    HGemPtr->DataItemOut(reqCount, HType.LIST_TYPE, NULL);
    for(i=0; i<reqCount; i++)
        HGemPtr->DataItemOutSVValue(reqList[i]);
    HGemPtr->SendLocalData();
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : S1F11 Status Variable Namelist Request -> S1F12.
//  Request  L,n { U4 SVID }        n<=0 -> every registered SV.
//  Reply    L,n { L,3 { U4 SVID, A SVNAME, A UNITS } }.
//  Read-only: touches no machine state. Unknown SVID -> empty name+unit per E5.
//---------------------------------------------------------------------------
void HT160Gem::S1F12_StatusVariableNamelistReply()
{
    int n, len, i;
    unsigned char Type;
    AnsiString S;
    unsigned reqList[512];
    int reqCount = 0;

    if(HGemPtr==NULL)
        return;

    HGemPtr->ResetReturnCode();

    if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
    {
        // malformed request -> reply empty list
        HGemPtr->InitLocalHead(1, 12, 0);
        HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
        HGemPtr->SendLocalData();
        return;
    }

    if(n<=0)
    {
        int total = HGemPtr->GetSVCount();
        for(i=0; i<total && reqCount<512; i++)
            reqList[reqCount++] = HGemPtr->GetSVIDByIndex(i);
    }
    else
    {
        // read requested SVIDs first (this consumes the receive buffer)
        for(i=0; i<n && reqCount<512; i++)
        {
            if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
                break;
            S = "";
            if(HGemPtr->DataItemIn(len, Type, S)==1)
                reqList[reqCount++] = (unsigned)StrToIntDef(S, 0);
            else
                reqList[reqCount++] = 0;
        }
    }

    HGemPtr->InitLocalHead(1, 12, 0);
    HGemPtr->DataItemOut(reqCount, HType.LIST_TYPE, NULL);
    for(i=0; i<reqCount; i++)
    {
        unsigned SVID = reqList[i];
        AnsiString nm = HGemPtr->GetSVName(SVID);
        AnsiString un = HGemPtr->GetSVUnit(SVID);
        HGemPtr->DataItemOut(3, HType.LIST_TYPE, NULL);
        HGemPtr->DataItemOut(1, HType.UINT_4_TYPE, &SVID);
        HGemPtr->DataItemOut(HType.ASCII_TYPE, nm);
        HGemPtr->DataItemOut(HType.ASCII_TYPE, un);
    }
    HGemPtr->SendLocalData();
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : S2F15 New Equipment Constant Send -> S2F16.
//  Request  L,n { L,2 { ECID, ECV } }.   Reply  S2F16  B EAC.
//  EAC: 0=ok, 1=one+ ECID unknown / not host-settable, 2=busy, 3=range.
//  SAFETY: only the tray-form geometry ECs (2011-2016) are host-settable, and
//  only while the machine is idle (SystemStart==false && no IC under machine).
//  Writing tray geometry while running could shift sort/place coordinates, so a
//  busy machine rejects the whole request (EAC=2, nothing written). Recipe-name
//  (1501) and any other ECID are rejected (EAC=1) to keep host EC-set out of
//  recipe switching. A successful tray write is persisted via TrayForm.Save().
//  NOTE: writes are applied incrementally; on a mixed batch the accepted items
//  are already written when a later item fails. Host should send tray ECs alone.
//---------------------------------------------------------------------------
void HT160Gem::S2F16_NewEquipmentConstantSendAcknowledge()
{
    int n, inner, len, i;
    unsigned char Type;
    unsigned char EAC = 0;
    AnsiString sECID, sVal, sLog;
    unsigned ECID;
    bool bWroteTrayForm = false;

    if(HGemPtr==NULL)
        return;

    HGemPtr->ResetReturnCode();

    bool bBusy = (HSys.Sys.SystemStart==true || HasICUnderMachine()==true);

    if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
    {
        EAC = 1;        // malformed request
        n = 0;
    }

    for(i=0; i<n; i++)
    {
        // inner L,2 { ECID, ECV }
        if(HGemPtr->GetDataItemLenAndTypeAndDelete(inner, HType.LIST_TYPE)!=1)
        {
            EAC = 1;
            break;
        }
        sECID = "";
        if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
            HGemPtr->DataItemIn(len, Type, sECID);
        sVal = "";
        if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
            HGemPtr->DataItemIn(len, Type, sVal);

        ECID = (unsigned)StrToIntDef(sECID, 0);

        if(bBusy)
        {
            EAC = 2;                 // busy -> reject, write nothing
            continue;
        }
        if(ECID>=2758 && ECID<=2763)  //AI(ht160s-secsgem) 20260612 : 9045 Type1 tray band
        {
            if(HGemPtr->WriteECValueByString(ECID, sVal)==0)
                bWroteTrayForm = true;
            else
                EAC = 3;             // unconvertible / out of range
        }
        else
        {
            EAC = 1;                 // not a host-settable constant
        }
    }

    if(bWroteTrayForm && !bBusy)
        TrayForm.Save(RecipeManager.GetCurrentRecipeName());

    // S2F16 reply : B EAC
    HGemPtr->InitLocalHead(2, 16, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &EAC);
    HGemPtr->SendLocalData();

    sLog.sprintf("[SECS] S2F16 EAC=%u busy=%d trayWritten=%d",
                 (unsigned)EAC, (int)bBusy, (int)bWroteTrayForm);
    HGemPtr->StringOut(sLog);
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260612 : local GUI EC editor write path. Enforces the
//  exact same policy as the host S2F16: only the tray-form geometry ECs
//  (9045 Type1 band 2758-2763) are settable, only while the machine is idle, and
//  a successful write is persisted to the active recipe's setup.ini. Returns 0=ok,
//  1=not settable / unknown, 2=busy, 3=convert/range error.
int HT160Gem::GuiWriteTrayEC(unsigned ECID, AnsiString sValue)
{
    if(HGemPtr==NULL)
        return 1;

    bool bBusy = (HSys.Sys.SystemStart==true || HasICUnderMachine()==true);
    if(bBusy)
        return 2;

    if(ECID<2758 || ECID>2763)
        return 1;        // not a host/GUI-settable constant

    if(HGemPtr->WriteECValueByString(ECID, sValue)!=0)
        return 3;        // unconvertible value

    TrayForm.Save(RecipeManager.GetCurrentRecipeName());
    HGemPtr->StringOut("[SECS][GUI] EC " + AnsiString((int)ECID) + " = " + sValue + " (tray form saved)");
    return 0;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260610 : S2F41 SET_LOT_INFO variable-length multi-Lot.
//  Reads outer L[2]{ A "SET_LOT_INFO", L[n]{ A lotID ... } }, refills
//  LotRegistry (overwrite, D1), backfills first lot to edLotNo (D2), rejects
//  while producing / IC under machine (D3, HCACK=4). HCACK: 0=ok,1=format,
//  2=param,4=busy (D4); cap HT160_MAX_LOT=64, over -> HCACK=2 (D5).
//  S2F42 reply: L[2]{ B HCACK, L[0] }.
//---------------------------------------------------------------------------
int HT160Gem::S2F42_Host_Command_Acknowledge()
{
    unsigned char HCACK = 1;        // default = format error
    int len, n, i;
    unsigned char Type;
    char CommandStr[256];
    char str[256];
    AnsiString S;
    AnsiString sRxDetail = "";   //AI(ht160s-agv) 20260720 : parsed-content echo for AMR-tab host-cmd feedback

    if(HGemPtr==NULL)
        return 1;

    ZeroMemory(CommandStr, sizeof(CommandStr));
    ZeroMemory(str, sizeof(str));
    HGemPtr->ResetReturnCode();

    // Outer L[2]
    if(HGemPtr->DataItemIn(2, HType.LIST_TYPE, NULL)==1)
    {
        // Command name (ASCII)
        HGemPtr->GetDataItemLenAndType(len, Type);
        if(Type==HType.ASCII_TYPE && len>0 && len<(int)sizeof(CommandStr))
        {
            if(HGemPtr->DataItemIn(len, HType.ASCII_TYPE, CommandStr)==1)
                S = CommandStr;
            else
                S = "";
        }
        else
        {
            S = "";
        }
        S = S.UpperCase();

        if(S.AnsiPos("SET_LOT_INFO")==1)
        {
            // Inner L[n] : n = number of Lots (variable)
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)==1)
            {
                sRxDetail = "lots=" + IntToStr(n);
                if(n==0)
                {
                    HCACK = 2;                                   // empty list -> param error
                }
                else if(HSys.Sys.SystemStart==true || HasICUnderMachine()==true)
                {
                    HCACK = 4;                                   // producing / IC inside -> busy
                }
                else if(n>HT160_MAX_LOT)
                {
                    HCACK = 2;                                   // exceeds capacity -> param error
                }
                else if(fMain!=NULL && fMain->ArchiveDiscardedWorkOrder("SECS_SETLOT")==false)
                {
                    //AI(ht160s-workorder-backup) 20260630 : prior work order could not be
                    //archived (real data + disk/log-path failure). Refuse rather than
                    //destroy it untraceably; host can retry once storage recovers. A
                    //first-ever load (nothing to archive) returns true and is NOT refused.
                    HCACK = 4;
                    RecordProcess("SECS SET_LOT_INFO refused : work-order backup failed");
                }
                else
                {
                    //AI(ht160s-ftp) 20260721 : parse into local buffers FIRST, commit atomically
                    // (Clear + AddLot) ONLY after the whole list parses cleanly. A mid-list reject
                    // (HCACK!=0) then leaves the prior work order untouched, so the host's "rejected"
                    // belief matches the machine. Mirrors the LOTSTART buffer-then-commit path; the
                    // previous code Cleared before the loop and committed incrementally, leaving a
                    // partial lot set live after a reject.
                    AnsiString bufCust[HT160_MAX_LOT];
                    AnsiString bufKyec[HT160_MAX_LOT];
                    int nBuf = 0;
                    HCACK = 0;
                    for(i=0; i<n; i++)
                    {
                        //AI(ht160s-ftp) 20260721 : each SET_LOT_INFO item is EITHER
                        //   A  "custLot"                     (legacy : Cust lot only, no KYEC batch)
                        //   L[2]{ A "custLot", A "kyecLot" }  (KYEC : Cust lot + Kyec batch id, Soter col7)
                        // Backward compatible : an old host sending bare ASCII still works unchanged; a
                        // new host sends the pair. The L[2] READ idiom mirrors the proven LOTSTART
                        // SORTMODE-pair reader below, so it cannot mis-consume the stream. **The pair
                        // SHAPE is OUR PROPOSAL pending KYEC confirmation of the SET_LOT_INFO SML**
                        // (docs/plan/ftp-kyec-upload-plan-20260721) -- if KYEC carries the KYEC lot a
                        // different way, ONLY this L[2] branch changes; the ASCII branch is the shipped path.
                        AnsiString custLot = "";
                        AnsiString kyecLot = "";
                        if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
                        {
                            HCACK = 2;                           // truncated list -> param error
                            break;
                        }
                        if(Type==HType.LIST_TYPE)
                        {
                            int pairLen = 0;
                            if(HGemPtr->GetDataItemLenAndTypeAndDelete(pairLen, HType.LIST_TYPE)!=1
                               || pairLen<1 || pairLen>2)
                            {
                                HCACK = 2;                       // malformed (custLot[,kyecLot]) pair
                                break;
                            }
                            if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                                HGemPtr->DataItemIn(len, Type, custLot);
                            if(pairLen>=2 && HGemPtr->GetDataItemLenAndType(len, Type)==1)
                                HGemPtr->DataItemIn(len, Type, kyecLot);
                            if(custLot.Trim()=="")
                            {
                                HCACK = 2;                       // pair carried no Cust lot -> param error
                                break;
                            }
                        }
                        else if(Type==HType.ASCII_TYPE && len>0 && len<(int)sizeof(str))
                        {
                            if(HGemPtr->DataItemIn(len, HType.ASCII_TYPE, str)!=1)
                            {
                                HCACK = 2;                       // read failure -> param error
                                break;
                            }
                            custLot = AnsiString(str);
                        }
                        else
                        {
                            HCACK = 2;                           // type mismatch -> param error
                            break;
                        }

                        if(nBuf>=HT160_MAX_LOT)
                        {
                            HCACK = 2;                           // more items than capacity -> param error
                            break;
                        }
                        bufCust[nBuf] = custLot;
                        bufKyec[nBuf] = kyecLot;
                        nBuf++;
                    }

                    // Commit atomically only on a clean parse (mirrors LOTSTART). On any reject the
                    // prior work order is left intact, matching the host's HCACK=2 belief.
                    if(HCACK==0)
                    {
                        LotRegistry.Clear();                     // D1 overwrite, now AFTER a clean parse
                        //AI(ht160s-lotbin) 20260722 : a SET_LOT_INFO work-order overwrite must
                        // ALSO drop the prior order's dynamic (Lot,Bin)->Auto bindings, mirroring
                        // the manual Lot Start (main.cpp LotBinBinding.Clear/SaveToIni). Otherwise
                        // the stale bindings keep IsAutoBound holding Autos, so the new order's
                        // (Lot,Bin) gets routed to the Error Auto. Guarded by the same clean-parse
                        // HCACK==0 gate as the LotRegistry overwrite, so a rejected list leaves
                        // both intact. LOTSTART stays additive (mid-lot resume) and does NOT clear.
                        LotBinBinding.Clear();
                        LotBinBinding.SaveToIni();
                        for(int j=0; j<nBuf; j++)
                        {
                            int iLotIdx = LotRegistry.AddLot(bufCust[j], HT160_LOT_SOURCE_SECS, "", "");
                            if(iLotIdx>=0)
                            {
                                //AI(ht160s-ftp) 20260721 : carry the KYEC batch id like the 2D-JSON
                                // path sets Substage; "" -> Soter renders "NA" (col7 / FTP token).
                                TLotRunInfo *pLot = LotRegistry.GetLot(iLotIdx);
                                if(pLot!=NULL)
                                    pLot->sKyecLotID = bufKyec[j];
                            }
                            if(j==0 && fMain!=NULL)
                                fMain->edLotNo->Text = bufCust[j]; // D2 backfill first lot
                        }
                    }
                }
            }
            else
            {
                HCACK = 1;                                       // bad list format
            }
            //AI(general) 20260610 : project LotRegistry onto the on-screen Lot list
            // (sgLotList) so SECS-pushed lots become visible, using the single shared
            // refresh that Simu / JSON / manual paths also use. SECS runs on the VCL
            // main thread (stNonBlocking OnClientRead), so this UI call is safe.
            if(HCACK==0 && fMain!=NULL)
            {
                fMain->RefreshLotListFromRegistry();
                //AI(ht160s-2dbin-manual) 20260628 : persist the SECS-pushed lot list.
                fMain->SaveWorkOrder();
            }
        }
        else if(S.AnsiPos("PAUSE")==1)
        {
            //AI(ht160s-secsgem) 20260611 : PAUSE host command. Consume the (usually empty)
            // parameter list, then pause via the shared MachinePause() choke point
            // (csystem.cpp) : drop SystemStart, DecStop the motors, raise SoftStop, and
            // RecordProcess-log "MACHINE PAUSE by secs-remote". EventReport stays out of
            // the GEM layer (existing boundary; it is operator-specific).
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            MachinePause(trigSecsRemote);
            HCACK = 0;
        }
        else if(S.AnsiPos("CLEARCOUNT")==1)
        {
            //AI(ht160s-lot-reset) 20260706 : CLEARCOUNT host command (HT172 SECS
            // ClearCount parity). Zero the per-run production counters and persist to
            // lastdata. Refused while producing so a running total is never wiped.
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            if(HSys.Sys.SystemStart==true)
            {
                HCACK = 4;                                       // producing -> busy
            }
            else
            {
                ResetPerLotProductionCounters();
                WriteLastDataIni();
                HCACK = 0;
            }
        }
        else if(S.AnsiPos("ONLINE_REMOTE")==1 || S=="ONLINE")
        {
            //AI(ht160s-secsgem) 20260611 : ONLINE (remote) host command. Consume the
            // parameter list and move the local control-state mirror to Online Remote(5).
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            iControlState = 5;
            HCACK = 0;
        }
        else if(S.AnsiPos("ONLINE_LOCAL")==1)
        {
            //AI(ht160s-secsgem) 20260611 : ONLINE_LOCAL host command. Consume the
            // parameter list and move the control-state mirror to Online Local(4).
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            iControlState = 4;
            HCACK = 0;
        }
        else if(S.AnsiPos("LOTSTART")==1)
        {
            //AI(ht160s-lot-webapi) 20260612 : Stage 4 : LOTSTART host command (ref
            // HT9045 899 S2F42 LOTSTART). Inner L[n] of ASCII Lot id(s) (usually 1).
            // We register the lot(s) (additive : no Clear, unlike SET_LOT_INFO) and
            // kick off a NON-blocking Lot WebAPI pull for the first lot's 2D/Bin data.
            // We do NOT auto-start machine motion : starting motion stays operator-
            // gated (safety-critical). No modal dialog : this runs on the HSMS/VCL
            // receive path and a popup would stall SECS communication.
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)==1)
            {
                sRxDetail = "lots=" + IntToStr(n);
                if(n<=0)
                {
                    HCACK = 2;                                   // empty list -> param error
                }
                else if(HSys.Sys.SystemStart==true || HasICUnderMachine()==true)
                {
                    HCACK = 4;                                   // producing / IC inside -> busy
                }
                else
                {
                    //AI(ht160s-whitelist) 20260716 : Phase 2 -- the inner list carries ASCII
                    // lot ids (as before) plus at most one optional L[2]{ A"SORTMODE", A value }
                    // pair (value NORMAL|WHITELIST) that switches the sort mode for this lot.
                    // Lots are buffered (not committed) until the whole list parses AND the busy
                    // guard passes, so a malformed pair or a mode-change-while-running rejects the
                    // entire packet with no ghost lot left registered. A plain lot-only list
                    // behaves exactly as before (same lots, same order, same HCACK).
                    AnsiString FirstLot = "";
                    AnsiString bufLots[HT160_MAX_LOT];
                    int nBuf = 0;
                    int pairLen;
                    AnsiString PendingSortMode = "";      // "" = no SORTMODE pair seen
                    HCACK = 0;
                    for(i=0; i<n; i++)
                    {
                        if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
                        {
                            HCACK = 2;                       // truncated list -> param error
                            break;
                        }
                        if(Type==HType.LIST_TYPE)
                        {
                            if(HGemPtr->GetDataItemLenAndTypeAndDelete(pairLen, HType.LIST_TYPE)!=1 || pairLen!=2)
                            {
                                HCACK = 2;                   // malformed SORTMODE pair -> param error
                                break;
                            }
                            AnsiString cpName="", cpVal="";
                            if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                                HGemPtr->DataItemIn(len, Type, cpName);
                            if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                                HGemPtr->DataItemIn(len, Type, cpVal);
                            if(cpName.Trim().UpperCase()!="SORTMODE")
                            {
                                HCACK = 2;                   // only the SORTMODE pair is recognized
                                break;
                            }
                            AnsiString sMode = cpVal.Trim().UpperCase();
                            if(sMode!="NORMAL" && sMode!="WHITELIST")
                            {
                                HCACK = 2;                   // value out of domain
                                break;
                            }
                            PendingSortMode = sMode;         // duplicate pair -> last one wins
                        }
                        else if(Type==HType.ASCII_TYPE && len>0 && len<(int)sizeof(str))
                        {
                            if(HGemPtr->DataItemIn(len, HType.ASCII_TYPE, str)!=1)
                            {
                                HCACK = 2;                   // read failure -> param error
                                break;
                            }
                            if(nBuf>=HT160_MAX_LOT)
                            {
                                HCACK = 2;                   // more lots than capacity -> param error
                                break;
                            }
                            bufLots[nBuf++] = AnsiString(str);
                            if(FirstLot=="")
                                FirstLot = AnsiString(str);
                        }
                        else
                        {
                            HCACK = 2;                       // type mismatch -> param error
                            break;
                        }
                    }
                    //AI(ht160s-whitelist) 20260716 : a SORTMODE pair with no lot is malformed
                    // (the switch rides along with a Lot Start, per the customer spec).
                    if(HCACK==0 && PendingSortMode!="" && nBuf==0)
                        HCACK = 2;
                    //AI(ht160s-whitelist) 20260716 : a SORTMODE change is only safe fully idle. The
                    // command-level guard above (SystemStart/HasICUnderMachine) is weaker than the
                    // UI Sort-mode lock : a lot can be Started (bRunning) with no IC yet under the
                    // machine. Reject the whole packet as busy rather than switch the classification
                    // model mid-lot. (No pair -> unchanged additive lot-registration behavior.)
                    if(HCACK==0 && PendingSortMode!="" && MachineRun.bRunning==true)
                        HCACK = 4;
                    if(HCACK==0)
                    {
                        //AI(ht160s-whitelist) 20260716 : list parsed + guards passed -> commit the
                        // buffered lots (order preserved), then apply the host sort-mode switch
                        // BEFORE the 2D-source load decision below so this lot loads via the newly
                        // selected mode.
                        int k;
                        for(k=0; k<nBuf; k++)
                            LotRegistry.AddLot(bufLots[k], HT160_LOT_SOURCE_SECS, "", "");
                        //AI(ht160s-whitelist-override) 20260717 : WhiteList is a per-lot OVERLAY, not a
                        // base mode. Set it on EVERY accepted LOTSTART : WHITELIST arms it for THIS lot;
                        // NORMAL *or NO SORTMODE pair* disarms (base production mode). Per the customer
                        // spec the host must send SORTMODE=WHITELIST for every whitelist lot, so a no-pair
                        // LOTSTART means base - disarming here stops WhiteList leaking into a subsequent
                        // no-pair lot (adversarial review 2026-07-17 MAJOR). The accept path is idle-gated
                        // (SystemStart==false && !HasICUnderMachine above) and pair+bRunning is already
                        // rejected, so this never flips routing on in-flight material. Do NOT write the
                        // base iSortMode; persist the overlay (work-order lifecycle, NOT sticky
                        // General.ini) and keep the maintenance selector + Main badge in sync.
                        GeneralSetting.SetWhiteListActive(PendingSortMode=="WHITELIST");
                        GeneralSetting.SaveWhiteListOverlay();
                        if(fMaintenance!=NULL)
                            fMaintenance->SyncSortModeSelectorFromSetting();
                        if(fMain!=NULL)
                            fMain->UpdateSortModeFeatureBadge();
                    }
                    if(HCACK==0 && FirstLot!="" && fMain!=NULL)
                    {
                        //AI(ht160s-lot-reset) 20260706 : SECS LOTSTART is the host-side
                        //equivalent of pressing Lot Start, so zero the per-run production
                        //counters too (gated idle : SystemStart==false and no IC inside).
                        ResetPerLotProductionCounters();
                        //AI(ht160s-uph) 20260706 : open the per-tray/lot UPH log folder.
                        TrayUphLog_OnLotStart(FirstLot);
                        //AI(ht160s-soter) 20260714 : host LOTSTART is the SECS analog of the manual Lot
                        //Start button; force clear+arm a fresh Soter buffer for the new lot (mirrors main.cpp).
                        g_SoterOutput.OnLotStart(FirstLot);
                        fMain->ClearProductInfoAtLotStart();
                        fMain->edLotNo->Text = FirstLot;         // active lot backfill
                        fMain->RefreshLotListFromRegistry();
                        //AI(ht160s-2dbin-manual) 20260628 : persist the SECS-registered lots
                        //(the 2D items themselves arrive via the WebAPI pull below, which
                        //also calls SaveWorkOrder in PollLotDataWebApi).
                        fMain->SaveWorkOrder();
                        //AI(ht160s-lot-webapi) 20260612 : pull EVERY registered lot's
                        // 2D/Bin data (matches the manual LotStart path). Previously
                        // only the first lot was pulled, so SET_LOT_INFO/LOTSTART lots
                        // 2..n arrived with no 2D items.
                        //AI(ht160s-whitelist) 20260715 : WhiteList mode loads the local WhiteList.json instead of the WebAPI pull.
                        if(GeneralSetting.IsWhiteListSortMode())
                            fMain->LoadWhiteListFile();
                        else
                            fMain->StartLotWebApiPullAll();            // async, no modal
                    }
                }
            }
            else
            {
                HCACK = 1;                                       // bad list format
            }
        }
        else if(S=="START")
        {
            //AI(machine-command-layer) 20260625 : production START host command. Route
            // through the single MachineStart() gate (csystem.cpp) -- the SAME entry the
            // operator Start button uses, so the lot/2D gate, the SystemStart raise, the
            // per-IC trace batch and home-first-when-unhomed all run identically. The gate
            // now lives inside MachineStart (not duplicated here); map its result to HCACK.
            // MachineStart never pops a modal, so the HSMS receive path is not stalled.
            // Exact "START" compare (NOT AnsiPos) so it does not swallow "START_AGV".
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            {
                AnsiString Reason;
                switch(MachineStart(trigSecsRemote, Reason))
                {
                    case msStarted:      HCACK = 0; break;   // armed (homes first if unhomed)
                    case msRejBusy:      HCACK = 4; break;   // already running
                    case msRejNoContext: HCACK = 2; break;   // no UI context
                    default:             HCACK = 2; break;   // msRejNotReady : lot/2D not ready
                }
            }
        }
        else if(S.AnsiPos("START_AGV")==1)
        {
            //AI(ht160s-agv) 20260615 : Phase C START_AGV. Inner L[n] of
            // L[2]{ A cpName, <cpValue> } pairs (spec 4.1). Station names
            // (Loader/Empty/Color/AUTO1..6) record a per-station AGV-handoff prep
            // intent via AgvCoord.BeginPrep; LoaderTrayCount sets the expected loader
            // tray count (SVID 38222). Records intent + ACKs ONLY : no motion is
            // driven and no Ready (CEID273) is faked - that is Phase D + in-place sensor.
            //AI(secs-kyec-startagv) 20260728 : refuse START_AGV outright when AMR is switched off.
            // BeginPrep/ReassertLocks have no bUseAMR gate, but EVERY release path does -
            // PollAndCall (uAgvStation.cpp:229), ServiceHandshake (:360) and the WAR0962 timeout
            // sweep (csystem.cpp:78) all early-return when bUseAMR==false. So a START_AGV received
            // with AMR off would lock a module with no code path left that can ever unlock it.
            // HCACK=2 also matches the documented host-simulator contract
            // (D:\AI_Area\Tool\HT160S_SECS_Simulator\code\ht160s_presets.py "2=param (unknown CP or non-AMR)").
            int innerLen;
            if(GeneralSetting.bUseAMR==false)
            {
                HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
                HGemPtr->StringOut("[SECS] START_AGV refused : AMR is disabled (bUseAMR=0)");
                HCACK = 2;
            }
            else if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)==1)
            {
                if(n<=0)
                {
                    HCACK = 2;                                   // empty list -> param error
                }
                else
                {
                    HCACK = 0;
                    for(i=0; i<n; i++)
                    {
                        if(HGemPtr->GetDataItemLenAndTypeAndDelete(innerLen, HType.LIST_TYPE)!=1)
                        {
                            HCACK = 1;                           // pair not a list -> format error
                            break;
                        }
                        AnsiString cpName="", cpVal="";
                        if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                            HGemPtr->DataItemIn(len, Type, cpName);
                        if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                            HGemPtr->DataItemIn(len, Type, cpVal);

                        //AI(ht160s-agv-binsetting) 20260713 : ONLY the Loader has a
                        // host tray-count CP. There is deliberately NO EmptyTrayCount /
                        // ColorTrayCount CP : SVID 38223/38224 (AMR Empty/Color Tray
                        // Count) stay reserved 0. HT9045/KYEC drive Empty/Color purely by
                        // sensor+TrayArm with zero SECS (docs/AGV/HT9045_vs_HT160_SECS_Diff)
                        // and HT160 has no stack-depth counting hardware. Loader is host-
                        // supplied because only it consumes the value (tray-kind tagging).
                        if(sRxDetail!="") sRxDetail = sRxDetail + " ";
                        sRxDetail = sRxDetail + cpName.Trim() + "=" + cpVal.Trim();
                        AnsiString cpN = cpName.Trim().UpperCase();
                        AnsiString cpV = cpVal.Trim().UpperCase();
                        if(cpN=="LOADERTRAYCOUNT")
                            AgvCoord.TrayCount[0] = StrToIntDef(cpVal, 0);  // P1 Loader expected trays
                        //AI(secs-kyec-startagv) 20260728 : LoaderICCount is a REAL KYEC CP, not an
                        // unknown one - the field host sends it on every START_AGV upload packet
                        // (KYEC log 2026-06-08 SECSGEM_TextLog_15.txt:544). It used to fall through
                        // to BeginPrep -> HCACK=2 and the whole handoff was refused. Park it in the
                        // P1 Loader device slot (SVID 38228 AMR Loader Device Count), which had no
                        // writer at all and always reported 0. Latched, not consume-once: unlike
                        // TrayCount[0] (consumed by EnqueueTrip, uAgvStation.cpp:85) nothing acts on
                        // it, it is purely the host-declared incoming IC count for read-back.
                        else if(cpN=="LOADERICCOUNT")
                            AgvCoord.DeviceCount[0] = StrToIntDef(cpVal, 0);
                        else if(AgvCoord.LookupByName(cpName) < 0)
                            HCACK = 2;                           // unknown CP name -> param error
                        //AI(secs-kyec-startagv) 20260728 : the KYEC host sends the FULL station
                        // vector every time, exactly one station carrying "Action" and all the rest
                        // "NA" (SECSGEM_TextLog_15.txt:507-546). "NA" means "listed for vector
                        // completeness, do nothing". HT160 used to ignore the value and BeginPrep
                        // EVERY named station, so one Loader dispatch put up to 8 innocent stations
                        // into AGV_PREP and locked their mechanisms - and the lock has no cheap
                        // release (RetryStation keeps it, AgvCoord.Reset has no runtime caller), so
                        // it ends in WAR0962 = DecStopAllMotor. HT9045 gates on the same literal
                        // (uHGemHT9045.cpp:1676-1709 requires S2=="Action"). Match it.
                        else if(cpV=="ACTION")
                            AgvCoord.BeginPrep(cpName);
                        else if(cpV!="NA")
                        {
                            //AI(secs-kyec-startagv) 20260728 : a known station with a verb we do not
                            // model yet (customer may later add LOAD/UNLOAD/SUPPLY routing, see
                            // docs\AGV\HT160S_E87_AGV_Communication_Draft_20260527.md:299). Do NOT
                            // act and do NOT reject - 9045 would silently no-op it and rejecting the
                            // packet would break a host we cannot re-spec. Log it so the field tells
                            // us what the real verb set is.
                            AnsiString sUnk;
                            sUnk.sprintf("[SECS] START_AGV %s: unmodelled CP value '%s' - no action taken",
                                         cpName.Trim().c_str(), cpVal.Trim().c_str());
                            HGemPtr->StringOut(sUnk);
                            RecordProcess("AGV: START_AGV "+cpName.Trim()+" value '"+cpVal.Trim()+"' not modelled - ignored");
                        }
                    }
                }
            }
            else
            {
                HCACK = 1;                                       // bad list format
            }
        }
        else if(S=="STOP")
        {
            //AI(machine-command-layer) 20260625 : STOP host command (S2F41 RCMD "STOP").
            // Hard stop / remote abort. Route through the single MachineStop() choke point
            // (csystem.cpp) so "SystemStart=false always stops the motors" holds : it
            // clears SystemStart, StopAllMotor(), raises SoftStop and RecordProcess-logs
            // the source. No modal, no UI context needed. (PAUSE only soft-pauses.)
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            MachineStop(trigSecsRemote);
            HCACK = 0;
        }
        else if(S=="HOME")
        {
            //AI(machine-command-layer) 20260625 : HOME host command (S2F41 RCMD "HOME"),
            // equivalent to the operator Home button. Calls the shared TfMain::HomeCore()
            // (main.cpp) -- the sbHome1Click body WITHOUT the "Confirm home?" modal, which
            // would stall the HSMS receive path. Runs on the VCL main thread; the home
            // engine is non-blocking. Gate on fMain (UI context) + not-already-running.
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            if(fMain==NULL)
                HCACK = 2;                                       // no UI context -> param error
            else if(HSys.Sys.SystemStart==true)
                HCACK = 4;                                       // already running/homing -> busy
            else
            {
                fMain->HomeCore();
                HCACK = 0;
            }
        }
        else if(S=="ONE_CYCLE")
        {
            //AI(secs-kyec-rcmd4) 20260728 : KYEC host ONE_CYCLE. Field body 2026-06-08,
            // invariant across all 11 occurrences : L[2]{ A[9] "ONE_CYCLE", L[0] } - the
            // parameter list is ALWAYS empty, so it is consumed and ignored.
            // ORDERING NOTE : these four new branches use EXACT == (never AnsiPos) and sit
            // after HOME, immediately before the catch-all else. Verified that no earlier
            // AnsiPos prefix can swallow them - in particular "ONE_CYCLE" does NOT begin with
            // "ONLINE" (O-N-E vs O-N-L). Keep PP_SIGNALTOWER ahead of PP_MUSIC so the longer
            // of the shared "PP_" pair matches first if these are ever converted to AnsiPos.
            // Routed through the factored TfMain::OneCycleCore() (main.cpp) : the
            // sbOneCycle1Click body WITHOUT its two ShowMyMessage modals, which would stall
            // the HSMS receive path. Same precedent as the HOME branch (HomeCore) and the
            // START branch (MachineStart -> HCACK) above.
            // bRequireRunning=true is the SECS-ONLY stale-arm guard : arming Run_OneCycle on
            // a stopped machine is a LATCH nothing clears until the next Start (ProcessMotion
            // early-returns while SystemStart==false), which would silently turn the
            // operator's next Start into one cycle plus an automatic stop.
            //AI(secs-msggap-fix) 20260729 : recounted from the KYEC logs, and the guard is on
            // FIRMER ground than the first draft said. The 11 commands split 3 ACCEPTED /
            // 8 SWALLOWED (not 2/9), and the third acceptance is the smoking gun : ONE_CYCLE at
            // 19:00:19.281 armed (9045 CEID 3 at 19:00:19.306) on a machine that went HALT
            // 0.85 s later, so the arm LATCHED - exactly the failure this guard prevents.
            // 9045 clears its own BtnOneCycle->Down latch at exactly one site, inside the
            // finish handler that then never ran. The retry burst is 9 commands over
            // 8 min 00.3 s at an exact 60.0 s cadence (7 unambiguously in HALT, one on the
            // ART->HALT boundary, the last in Alarm) - "nine times in nine minutes" was loose.
            // DELIBERATE DEVIATION from HT9045, which answers HCACK=0 unconditionally and
            // swallows every rejection silently - its HCACK=0 means "message received", not
            // "cycle accepted". HT160 tells the truth.
            //AI(secs-msggap-fix) 20260729 : do NOT claim this is field-proven host-safe. The
            // "host tolerates refusals and keeps its cadence" evidence is ENERGY_SAVING's
            // (23/23 HCACK=2, zero S9Fx all day) and does not transfer : ONE_CYCLE got HCACK=0
            // on all 11 field occurrences, so a non-zero answer to it has never been on the
            // wire. Treat the host's reaction to HCACK=2/4 here as an ON-MACHINE WATCH ITEM;
            // reproduce the 60 s retry storm on the simulator first.
            //AI(secs-msggap-fix) 20260729 : the empty parameter list IS consumed below, which
            // 9045 does not do at all. Harmless while the body stays L[0] (11/11) and it keeps
            // the token stream clean if a host ever attaches parameters.
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            if(fMain==NULL)
            {
                HCACK = 2;                                       // no UI context -> param error
            }
            else
            {
                AnsiString ocReason;
                switch(fMain->OneCycleCore(true, ocReason))
                {
                    case ocStarted:      HCACK = 0; break;   // armed on a running machine
                    case ocRejBusy:      HCACK = 4; break;   // a cycle is already armed and running
                    //AI(secs-audit-fix) 20260729 : machine stopped -> 2 "cannot perform now",
                    //never 4. SEMI E5 HCACK=4 is a POSITIVE ack ("will be performed, completion
                    //signalled later by an event"), and this same change made that promise
                    //checkable by emitting CEID 27 on cycle finish - a stopped machine will
                    //never emit it, so 4 would leave the host waiting forever. This was the
                    //dominant field case: 8 of the 11 KYEC ONE_CYCLE commands on 2026-06-08
                    //arrived while the equipment was reporting HALT.
                    case ocRejStopped:   HCACK = 2; break;   // stopped -> cannot perform now
                    default:             HCACK = 2; break;   // ocRejMode / ocRejNotReady
                }
                sRxDetail = ocReason;
                if(HCACK==0)
                    RecordProcess("ONE CYCLE by secs-remote");
                else
                    RecordProcess("SECS ONE_CYCLE refused : "+ocReason);
            }
        }
        else if(S=="ENERGY_SAVING")
        {
            //AI(secs-kyec-rcmd4) 20260728 : KYEC host ENERGY_SAVING. Field body 2026-06-08,
            // invariant across all 23 occurrences :
            //   L[2]{ A[13] "ENERGY_SAVING", L[1]{ L[2]{ A[5] "STATE", U4[1] 0 } } }
            // STATE always 0 (= leave power save; 1 never seen).
            //AI(secs-msggap-fix) 20260729 : "a ~30-minute heartbeat" is only half the story -
            // 22 beats run 00:03:24 to 10:33:54 at 28.9-30.8 min spacing, then the host STOPS
            // for 4 h 59.6 min, sends one orphan at 15:33:32, and nothing for the last 3 h 49.
            // So do not treat the arrival of this command as a liveness signal.
            // HT-160S HAS NO POWER-SAVE SUBSYSTEM to enter or leave : no heater
            // (DoTemptureControl() is an empty stub), no ATC, no motor-current idle-off.
            // So : VALIDATE the STATE syntax (a malformed packet is still reported as a format
            // error) and then REFUSE with HCACK=2. Deliberately NOT accept-and-no-op - replying
            // 0 would tell the host the machine changed a power state that does not exist.
            // This is byte-identical to what KYEC's OWN HT9045 already answers : 23/23 HCACK=2,
            // after which the host neither escalated (zero S9Fx all day) nor changed cadence.
            // The refusal is field-proven host-safe. It is also strictly better than the
            // unknown-command path : HCACK=1 for a well-formed command reads to a host audit as
            // "command not recognised" rather than "known, not available".
            //AI(secs-msggap-fix) 20260729 : the HCACK=2 itself is directly observed, but which
            // 9045 rung produced it is NOT observable in a SECS log - it is narrowed by
            // elimination to one of two config-level Function Disable rungs (the IC / Contact /
            // ATC-online gates are time-varying, while HCACK stayed 2 across 15.5 h and states
            // SLEEP / Alarm / HALT / Running). No HT160 behaviour depends on which one.
            //AI(secs-kyec-rcmd4-fix) 20260728 : an EMPTY parameter list is a well-formed
            // ENERGY_SAVING, so it must get the same "known command, not available" answer as
            // any other well-formed one. Returning 1 here contradicted this branch's own
            // rationale two paragraphs up (1 = not recognised) and disagreed with the sibling
            // PP_SIGNALTOWER / PP_MUSIC branches, which both accept n<=0 as a valid shape.
            // Only a genuinely unreadable list stays HCACK=1.
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
            {
                HCACK = 1;                                       // body is not a list -> format error
            }
            else if(n<=0)
            {
                HCACK = 2;                                       // well-formed but empty -> known command, not available
            }
            else
            {
                int esPairLen;
                HCACK = 2;                                       // syntax good so far -> "known command, not available"
                for(i=0; i<n; i++)
                {
                    if(HGemPtr->GetDataItemLenAndTypeAndDelete(esPairLen, HType.LIST_TYPE)!=1 || esPairLen!=2)
                    {
                        HCACK = 1;                               // pair is not an L[2] -> format error
                        break;
                    }
                    AnsiString esName="", esValStr="";
                    if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                        HGemPtr->DataItemIn(len, Type, esName);
                    if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                        HGemPtr->DataItemIn(len, Type, esValStr);
                    if(sRxDetail!="") sRxDetail = sRxDetail + " ";
                    sRxDetail = sRxDetail + esName.Trim() + "=" + esValStr.Trim();
                    int esState = StrToIntDef(esValStr.Trim(), -1);
                    if(esName.Trim().UpperCase()!="STATE" || esState<0 || esState>1)
                    {
                        HCACK = 1;                               // unknown CP name / value out of domain -> format error
                        break;
                    }
                }
                if(HCACK==2)
                    RecordProcess("SECS ENERGY_SAVING refused : HT-160S has no power-save subsystem");
            }
        }
        else if(S=="PP_SIGNALTOWER")
        {
            //AI(secs-kyec-rcmd4) 20260728 : KYEC host PP_SIGNALTOWER - a LATCHED host override
            // of the per-RunState tower-lamp table (SetSecsTowerOverride / DoSystemMessage,
            // csystem.cpp). Field bodies 2026-06-08, 15 occurrences :
            //   SET   L[2]{ A[14] "PP_SIGNALTOWER", L[3]{ L[2]{A "RED",U4 v},
            //                    L[2]{A "GREEN",U4 v}, L[2]{A "YELLOW",U4 v} } }   (6x)
            //   CLEAR L[2]{ A[14] "PP_SIGNALTOWER", L[0] }                          (9x)
            // Values seen are only 0 and 2, never 1; documented domain 0=off / 1=on / 2=blink.
            // MORE clears than sets, so the clear must be idempotent. Paired with PP_MUSIC
            // ~0.3 s apart (0.256-0.361 s, PP_MUSIC always first).
            //AI(secs-msggap-fix) 20260729 : two corrections to the field reading above.
            // (1) NOT all six SETs are 2/2/2 - five are, and one (19:07:50.526) is
            //     RED=2 GREEN=0 YELLOW=0. The host does use non-uniform per-colour states, so
            //     never collapse the three colours into one value. Per-colour handling below is
            //     load-bearing, not defensive.
            // (2) The trigger is NOT specifically "tester is IDLE, priority lot waiting" - only
            //     3 of the 6 SET bursts follow that text; the others follow "has no schedule.",
            //     "MES_Status Changed toSetUp", and an S10F3 "[ART]User manually abort ART
            //     process.". And two of the nine CLEARs arrive 0.3 s after the equipment's OWN
            //     alarm event report. So this is a GENERIC host attention annunciator that the
            //     host does also use around alarm-ish conditions. The alarm-suppression gates in
            //     csystem.cpp stand on their own safety argument (machine-derived red must win) -
            //     they must NOT be justified by "the host never uses this for alarms".
            // Buffer-then-commit (LOTSTART / SET_LOT_INFO idiom) : an unknown CP name or an
            // out-of-domain value rejects the WHOLE packet and applies NOTHING. HT9045 partially
            // applies and latches the flag anyway - not ported. A colour the host does not name
            // keeps its previous value (-1), matching 9045's write-only-what-is-present.
            // No busy/idle pre-gate : a lamp command is safe in every machine state.
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
            {
                HCACK = 1;                                       // bad list format
            }
            else if(n<=0)
            {
                ClearSecsTowerOverride();                        // empty list -> release (idempotent : 9 clears for 6 sets)
                sRxDetail = "clear";
                RecordProcess("SECS PP_SIGNALTOWER : tower override released (empty parameter list)");
                HCACK = 0;
            }
            else
            {
                int stPairLen;
                int stRed=-1, stYellow=-1, stGreen=-1;           // -1 = colour not named -> keep previous
                HCACK = 0;
                for(i=0; i<n; i++)
                {
                    if(HGemPtr->GetDataItemLenAndTypeAndDelete(stPairLen, HType.LIST_TYPE)!=1 || stPairLen!=2)
                    {
                        HCACK = 1;                               // pair is not an L[2] -> format error
                        break;
                    }
                    AnsiString stName="", stValStr="";
                    if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                        HGemPtr->DataItemIn(len, Type, stName);
                    if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                        HGemPtr->DataItemIn(len, Type, stValStr);
                    if(sRxDetail!="") sRxDetail = sRxDetail + " ";
                    sRxDetail = sRxDetail + stName.Trim() + "=" + stValStr.Trim();
                    AnsiString stN = stName.Trim().UpperCase();
                    int stVal = StrToIntDef(stValStr.Trim(), -1);
                    if(stVal<0 || stVal>2)
                    {
                        HCACK = 2;                               // value outside 0..2 -> param error, apply nothing
                        break;
                    }
                    if(stN=="RED")
                        stRed=stVal;
                    else if(stN=="YELLOW")
                        stYellow=stVal;
                    else if(stN=="GREEN")
                        stGreen=stVal;
                    else
                    {
                        HCACK = 2;                               // unknown CP name -> param error, apply nothing
                        break;
                    }
                }
                if(HCACK==0)
                {
                    SetSecsTowerOverride(stRed, stYellow, stGreen);
                    RecordProcess("SECS PP_SIGNALTOWER : tower override armed ("+sRxDetail+")");
                }
            }
        }
        else if(S=="PP_MUSIC")
        {
            //AI(secs-kyec-rcmd4) 20260728 : KYEC host PP_MUSIC - a LATCHED host override of the
            // per-RunState buzzer selection (SetSecsMusicOverride / DoSystemMessage,
            // csystem.cpp). Field bodies 2026-06-08, 15 occurrences :
            //   SET   L[2]{ A[8] "PP_MUSIC", L[1]{ L[2]{ A[0] "", U4[1] 1 } } }   (6x)
            //   CLEAR L[2]{ A[8] "PP_MUSIC", L[0] }                               (9x)
            // *** THE CP NAME IS A ZERO-LENGTH ASCII ITEM A[0] "". *** It is READ - to keep the
            // token stream in sync - and then DISCARDED WITHOUT COMPARISON, exactly as HT9045
            // does. Adding a name check here would reject every real KYEC PP_MUSIC packet; this
            // is the single easiest way to get this command wrong.
            // The A[0] read is proven safe in HT160's own tokenizer : ProcessSML stores three
            // tokens for a zero-length ASCII item, the peek returns 1 at len=0 and DataItemInSub
            // passes the l>len check and deletes all three.
            // Only the VALUE is validated : 1..4 -> SwMusic1..SwMusic4 (database.h:453-456).
            // HT9045 does SW[SwMusic1+CLASS-1].On() with NO range check, so CLASS=0 drives the
            // switch immediately BEFORE SwMusic1 - that latent bug is deliberately NOT ported;
            // out-of-range -> HCACK=2 and SetSecsMusicOverride clamps again defensively.
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1)
            {
                HCACK = 1;                                       // bad list format
            }
            else if(n<=0)
            {
                ClearSecsMusicOverride();                        // empty list -> release (idempotent)
                sRxDetail = "clear";
                RecordProcess("SECS PP_MUSIC : music override released (empty parameter list)");
                HCACK = 0;
            }
            else
            {
                int pmPairLen;
                int pmClass = -1;
                HCACK = 0;
                for(i=0; i<n; i++)
                {
                    if(HGemPtr->GetDataItemLenAndTypeAndDelete(pmPairLen, HType.LIST_TYPE)!=1 || pmPairLen!=2)
                    {
                        HCACK = 1;                               // pair is not an L[2] -> format error
                        break;
                    }
                    AnsiString pmName="", pmValStr="";
                    if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                        HGemPtr->DataItemIn(len, Type, pmName);   // A[0] "" at KYEC : read to stay in sync, NEVER compared
                    if(HGemPtr->GetDataItemLenAndType(len, Type)==1)
                        HGemPtr->DataItemIn(len, Type, pmValStr);
                    if(sRxDetail!="") sRxDetail = sRxDetail + " ";
                    sRxDetail = sRxDetail + "class=" + pmValStr.Trim();
                    pmClass = StrToIntDef(pmValStr.Trim(), -1);
                    if(pmClass<1 || pmClass>4)
                    {
                        HCACK = 2;                               // class outside SwMusic1..4 -> param error, apply nothing
                        break;
                    }
                }
                if(HCACK==0 && pmClass>=1)
                {
                    SetSecsMusicOverride(pmClass);
                    RecordProcess("SECS PP_MUSIC : music override armed (class="+IntToStr(pmClass)+")");
                }
            }
        }
        else
        {
            HCACK = 1;                                           // unknown command
        }
    }
    else
    {
        HCACK = 1;                                               // bad outer format
    }

    // S2F42 reply : L[2]{ B HCACK, L[0] }
    HGemPtr->InitLocalHead(2, 42, 0);
    HGemPtr->DataItemOut(2, HType.LIST_TYPE, NULL);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &HCACK);
    HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
    HGemPtr->SendLocalData();

    {
        AnsiString sLog;
        sLog.sprintf("[SECS] S2F42 cmd=%s HCACK=%u Lots=%d", S.c_str(), (unsigned)HCACK,
                     (int)LotRegistry.GetLotCount());
        HGemPtr->StringOut(sLog);
    }
    AmrInject.NoteHostCommand(S, HCACK, sRxDetail);   // AI(ht160s-agv) 20260720 : every host cmd -> AMR tab (accept/reject + reason + content)
    return 1;
}
//---------------------------------------------------------------------------
void HT160Gem::S1F2_OnLineData()
{
    //AI(ht160s-secsgem) 20260625 : host S1F1 Are-You-There -> reply S1F2 On Line Data.
    //  L[2]{ A:MDLN, A:SOFTREV }. The sMachineType/sSoftwareVersion fields are private on
    //  THGem (no public getter), so emit fixed ASCII literals here - same idiom HT172
    //  uses at its GemInitial call site (literal model string). No clock/side effects.
    if(HGemPtr==NULL)
        return;
    HGemPtr->InitLocalHead(1, 2, 0);
    HGemPtr->DataItemOut(2, HType.LIST_TYPE, NULL);
    HGemPtr->DataItemOut(HType.ASCII_TYPE, AnsiString("HT-160S"));
    HGemPtr->DataItemOut(HType.ASCII_TYPE, AnsiString("1.0.0.0"));
    HGemPtr->SendLocalData();
    HGemPtr->StringOut("[SECS] S1F2 on-line data sent (MDLN=HT-160S)");
}
//---------------------------------------------------------------------------
void HT160Gem::S1F14_ConnectRequestAcknowledge()
{
    //AI(ht160s-secsgem) 20260625 : host S1F13 Establish Communications Request -> reply
    //  S1F14 L[2]{ B:COMMACK, L[2]{ A:MDLN, A:SOFTREV } }. COMMACK=0 (always accept),
    //  matching HT172. MDLN/SOFTREV are ASCII literals (sMachineType/sSoftwareVersion are
    //  private on THGem). No inbound parse needed - HT160 has no abort-on-format option.
    if(HGemPtr==NULL)
        return;
    unsigned char COMMACK = 0;
    HGemPtr->InitLocalHead(1, 14, 0);
    HGemPtr->DataItemOut(2, HType.LIST_TYPE, NULL);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &COMMACK);
    HGemPtr->DataItemOut(2, HType.LIST_TYPE, NULL);
    HGemPtr->DataItemOut(HType.ASCII_TYPE, AnsiString("HT-160S"));
    HGemPtr->DataItemOut(HType.ASCII_TYPE, AnsiString("1.0.0.0"));
    HGemPtr->SendLocalData();
    HGemPtr->StringOut("[SECS] S1F14 establish-comm acknowledged (COMMACK=0)");
}
//---------------------------------------------------------------------------
void HT160Gem::S1F18_ONLINEAcknowledge()
{
    //AI(secs-online) 20260724 : host S1F17 Request ONLINE -> reply S1F18 <B ONLACK>. ONLACK=0
    //  (accepted). Also lift the GEM control-state mirror to Online-Remote(5), matching the
    //  existing "ONLINE" host command (S2F41) semantics at S2F42 dispatch. Previously unhandled
    //  -> the S1 dispatch fell through to S9F3 (Unrecognized), so a GEM host could never bring
    //  the tool online via S1F17/F18. Send idiom mirrors S1F14_ConnectRequestAcknowledge.
    if(HGemPtr==NULL)
        return;
    unsigned char ONLACK = 0;
    HGemPtr->InitLocalHead(1, 18, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &ONLACK);
    HGemPtr->SendLocalData();
    iControlState = 5;   //Online-Remote (same target as the ONLINE host command)
    HGemPtr->StringOut("[SECS] S1F18 ONLINE acknowledged (ONLACK=0, control state -> Online-Remote 5)");
}
//---------------------------------------------------------------------------
void HT160Gem::S1F16_OFFLINEAcknowledge()
{
    //AI(secs-offline) 20260727 : host S1F15 Request OFF-LINE -> reply S1F16 <B OFLACK>. OFLACK=0
    //  (accepted). Mirror of S1F18_ONLINEAcknowledge: also drop the GEM control-state mirror to
    //  Off-Line(1). Single-binary reply idiom (InitLocalHead + DataItemOut(BINARY) + SendLocalData).
    //  Previously S1F15 had no Dispatch case -> fell to S9F3, so a GEM host could not take the
    //  tool OFF-LINE. Pairs with the S1F17/F18 online path (commit 21be26a).
    if(HGemPtr==NULL)
        return;
    unsigned char OFLACK = 0;
    HGemPtr->InitLocalHead(1, 16, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &OFLACK);
    HGemPtr->SendLocalData();
    iControlState = 1;   //Equipment Off-Line (GEM control state)
    //AI(secs-kyec-rcmd4-fix) 20260728 : going OFF-LINE means the host has handed the machine back
    //  to the operator, so it must not leave the tower/buzzer driven by a stale host override.
    if(IsSecsPanelOverrideActive())
    {
        ClearSecsPanelOverride();
        HGemPtr->StringOut("[SECS] S1F16 OFF-LINE : host panel override (tower/buzzer) released");
        RecordProcess("SECS: OFF-LINE released host panel override");
    }
    HGemPtr->StringOut("[SECS] S1F16 OFF-LINE acknowledged (OFLACK=0, control state -> Off-Line 1)");
}
//---------------------------------------------------------------------------
//AI(secs-kyec-rcmd4-fix) 20260728 : HSMS link lost (peer disconnect / socket error /
//  Separate.req / watchdog DropConnection). KYEC arms PP_SIGNALTOWER + PP_MUSIC and clears them
//  seconds later with an empty parameter list; if the link drops in between, the operator is left
//  with a tower and buzzer nobody can release except the panel ALARM RESET key - which arrives
//  over the Pad RS232 link (SnFKAlarmReset/SnRKAlarmReset are COMM_PAD), so a Pad outage would
//  mean no escape at all. Release the latch with the host that set it.
void HT160Gem::OnCommunicationLost()
{
    if(IsSecsPanelOverrideActive()==false)
        return;
    ClearSecsPanelOverride();
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] link lost : host panel override (tower/buzzer) released");
    RecordProcess("SECS: link lost released host panel override");
}
//---------------------------------------------------------------------------
void HT160Gem::S2F34_DefineReportAcknowledge()
{
    //AI(secs-reportdef) 20260724 : Find* are private on THGem -> body lives there; delegate.
    if(HGemPtr!=NULL)
        HGemPtr->ProcessDefineReport_S2F33();
}
//---------------------------------------------------------------------------
void HT160Gem::S2F36_LinkEventReportAcknowledge()
{
    if(HGemPtr!=NULL)
        HGemPtr->ProcessLinkEventReport_S2F35();
}
//---------------------------------------------------------------------------
void HT160Gem::S2F38_EnableDisableEventReportAcknowledge()
{
    if(HGemPtr!=NULL)
        HGemPtr->ProcessEnableDisableEventReport_S2F37();
}
//---------------------------------------------------------------------------
void HT160Gem::S2F32_DateAndTimeAcknowledge()
{
    //AI(ht160s-secsgem) 20260625 : host S2F31 Date and Time Set Request -> reply S2F32 TIACK.
    //  TIACK=0 (accepted). MINIMAL port: acknowledge ONLY - deliberately does NOT write the
    //  equipment OS clock (HT172 settime/setdate is excluded; setting the PC clock from a host
    //  message is a side-effect that needs an explicit product decision + on-machine verify).
    //  Single-byte reply via the proven S5F4 triad (HT160 has no LocalAcknowledge wrapper).
    if(HGemPtr==NULL)
        return;
    unsigned char TIACK = 0;
    HGemPtr->InitLocalHead(2, 32, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &TIACK);
    HGemPtr->SendLocalData();
    HGemPtr->StringOut("[SECS] S2F32 date/time acknowledged (TIACK=0, clock NOT set)");
}
//---------------------------------------------------------------------------
void HT160Gem::S2F18_DateandTimeData()
{
    //AI(secs-gem-std) 20260727 : host S2F17 Date and Time Request -> reply S2F18 <A TIME>.
    //  16-char SEMI E5 TIME "YYYYMMDDhhmmsscc" (cc=centiseconds, fixed "00") from the equipment
    //  clock. READ-ONLY : reports current time, sets nothing (the write side stays S2F31->S2F32
    //  ack-only). Previously routed to the base SendUnsupported stub -> host S2F17 T3-timed-out.
    if(HGemPtr==NULL)
        return;
    AnsiString sTime = Now().FormatString("yyyymmddhhnnss") + "00";
    HGemPtr->InitLocalHead(2, 18, 0);
    HGemPtr->DataItemOut(HType.ASCII_TYPE, sTime);
    HGemPtr->SendLocalData();
    HGemPtr->StringOut("[SECS] S2F18 date/time data sent (" + sTime + ")");
}
//---------------------------------------------------------------------------
void HT160Gem::S2F26_DiagnosticLoopbackData()
{
    //AI(secs-gem-std) 20260727 : host S2F25 Loopback Diagnostic Request -> reply S2F26 echoing the
    //  received binary (ABS) back unchanged - the standard HSMS link diagnostic. Mirrors HT9045
    //  S2F26. HT160 has no S9F7 primitive, so a malformed request is logged (no reply) rather than
    //  an on-wire S9F7. Previously routed to the base SendUnsupported stub -> host S2F25 T3-timeout.
    if(HGemPtr==NULL)
        return;
    HGemPtr->ResetReturnCode();
    int len = 0;
    unsigned char Type = 0;
    if(HGemPtr->GetDataItemLenAndType(len, Type)==1 && Type==HType.BINARY_TYPE && len>=0 && len<=4096)
    {
        unsigned char *Temp = new unsigned char[len+1];
        if(HGemPtr->DataItemIn(len, Type, Temp)==1)
        {
            HGemPtr->InitLocalHead(2, 26, 0);
            HGemPtr->DataItemOut(len, HType.BINARY_TYPE, Temp);
            HGemPtr->SendLocalData();
            HGemPtr->StringOut("[SECS] S2F26 loopback echoed (len=" + IntToStr(len) + ")");
        }
        else
            HGemPtr->StringOut("[SECS] S2F25 loopback read failed - no echo");
        delete[] Temp;
    }
    else
        HGemPtr->StringOut("[SECS] S2F25 loopback format error (not BINARY) - no echo");
}
//---------------------------------------------------------------------------
void HT160Gem::S5F4_EnableDisableAlarmAcknowledge()
{
    //AI(ht160s-secsgem) 20260625 : host S5F3 Enable/Disable Alarm Send -> reply S5F4
    //  ACKC5=0 so the host's W-bit alarm-enable request does not T3-timeout. HT160
    //  keeps alarms always-enabled for now (per-ALID enable/disable is a follow-up).
    if(HGemPtr==NULL)
        return;
    unsigned char ACKC5 = 0;
    HGemPtr->InitLocalHead(5, 4, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &ACKC5);
    HGemPtr->SendLocalData();
    HGemPtr->StringOut("[SECS] S5F4 alarm enable/disable acknowledged (ACKC5=0)");
}
//---------------------------------------------------------------------------
void HT160Gem::S5F6_ListAlarmData()
{
    //AI(ht160s-secsgem) 20260715 : reply to S5F5 (List Alarm Request) with the real alarm
    // catalog built live from HSys.mapAlarmCodeList (the SSOT). See EmitAlarmCatalog.
    EmitAlarmCatalog(6);
}
//---------------------------------------------------------------------------
void HT160Gem::S5F8_ListEnableAlarmAcknowledge()
{
    //AI(ht160s-secsgem) 20260715 : reply to S5F7 (List Enabled Alarm Request) with S5F8.
    // HT160 keeps every registered alarm enabled (no per-ALID enable/disable), so the
    // enabled-alarm list is identical to the full catalog.
    EmitAlarmCatalog(8);
}
//---------------------------------------------------------------------------
void HT160Gem::EmitAlarmCatalog(int Func)
{
    //AI(ht160s-secsgem) 20260715 : emit an alarm catalog as S5F<Func> from the SSOT map.
    // Body: L[n]{ L[3]{ B ALCD, U4 ALID, A ALTX } }. ALCD = alarm category (AlarmType
    // low 7 bits; the set/clear bit 7 is 0 for a definition listing); ALID = ComputeAlarmAlid()
    // so it matches the S5F1 report ALID; ALTX = "<code> <english message>".
    // NOTE: the S5F5/S5F7 ALID filter is NOT applied - the full catalog is always returned
    // (a safe superset; catalog requests normally send L,0 = all).
    if(HGemPtr==NULL)
        return;
    int n = (int)HSys.mapAlarmCodeList.size();
    HGemPtr->InitLocalHead(5, Func, 0);
    HGemPtr->DataItemOut(n, HType.LIST_TYPE, NULL);
    std::map<AnsiString, MyAlarmCodeStruct>::iterator it;
    for(it=HSys.mapAlarmCodeList.begin(); it!=HSys.mapAlarmCodeList.end(); it++)
    {
        unsigned char alcd  = (unsigned char)(it->second.AlarmType & 0x7F);
        unsigned      uAlid = ComputeAlarmAlid(it->second.AlarmCode);
        AnsiString    altx  = it->second.AlarmCode;
        if(it->second.E_ErrMessage!="")
            altx = it->second.AlarmCode + " " + it->second.E_ErrMessage;
        HGemPtr->DataItemOut(3, HType.LIST_TYPE, NULL);
        HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &alcd);
        HGemPtr->DataItemOut(1, HType.UINT_4_TYPE, &uAlid);
        HGemPtr->DataItemOut(HType.ASCII_TYPE, altx);
    }
    HGemPtr->SendLocalData();
    AnsiString msg;
    msg.sprintf("[SECS][TX] S5F%d alarm catalog sent (%d entries)", Func, n);
    HGemPtr->StringOut(msg);
}
//---------------------------------------------------------------------------
void HT160Gem::S6F16_EventReportData()
{
    //AI(secs-msggap) 20260728 : host S6F15 Event Report Request -> reply S6F16.
    //  KYEC sends a BARE <U4[1] CEID> (verified in all 3 occurrences), not an L,1 wrapper,
    //  so read one scalar directly. An optional L,1 wrapper is tolerated for other hosts.
    //  ResetReturnCode() FIRST : iReturnCode is sticky (GetDataItemLenAndType only ever
    //  lowers it), so a previous message's read failure would otherwise poison this one.
    //  Same first line as S2F26 / S2F33 / S2F37.
    //  DATAID is 1 : hardcoded in HT9045, matches the wire, and matches the global
    //  EventReport(unsigned) glue which calls HGem->EventReport(1, Ceid).
    if(HGemPtr==NULL)
        return;
    HGemPtr->ResetReturnCode();
    int len = 0;
    unsigned char Type = 0;
    AnsiString sTmp;
    //AI(secs-msggap-fix) 20260728 : NEVER return without sending. A silent return is the exact
    //  failure this whole patch exists to remove - the host just T3s again. Every parse failure
    //  now falls through to EmitEventReportBody with CEID 0, which is a well-formed
    //  L,3{ DATAID, CEID=0, L,0 } (FindCEIDItem returns NULL -> reportCount 0), i.e. "no such
    //  event". Reachable without a malformed host: DataItemIn(AnsiString&) has no BINARY /
    //  BOOLEAN / U8 / F4 / F8 branch and rejects Len!=1 for numerics, so a host encoding the
    //  CEID as <U8[1]> or <U4[2]> would otherwise get total silence.
    unsigned iCeid = 0;
    if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
    {
        HGemPtr->StringOut("[SECS] S6F15 format error (empty body) - replying S6F16 CEID=0");
    }
    else
    {
        bool bReadable = true;
        if(Type==HType.LIST_TYPE)
        {
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(len, HType.LIST_TYPE)!=1 ||
               HGemPtr->GetDataItemLenAndType(len, Type)!=1)
            {
                HGemPtr->StringOut("[SECS] S6F15 format error (bad list wrapper) - replying S6F16 CEID=0");
                bReadable = false;
            }
        }
        if(bReadable)
        {
            if(HGemPtr->DataItemIn(len, Type, sTmp)!=1)
                HGemPtr->StringOut("[SECS] S6F15 format error (CEID unreadable) - replying S6F16 CEID=0");
            else
                iCeid = (unsigned)strtoul(sTmp.c_str(), NULL, 10);   //saturates at ULONG_MAX; atoi would wrap a U4 >= 2^31
        }
    }
    HGemPtr->EmitEventReportBody(16, 1, iCeid);
}
//---------------------------------------------------------------------------
void HT160Gem::S6F20_IndividualReportData()
{
    //AI(secs-msggap) 20260728 : host S6F19 Individual Report Request -> reply S6F20.
    //  KYEC sends a BARE <U4[1] RPTID> (verified in all 8 occurrences: 700 x3, 600 x3,
    //  506 x2). The answer is a flat list of that report's SV values read from the LIVE
    //  registry, so a host that deletes and redefines the RPTID mid-campaign - KYEC does
    //  exactly this to 506, L[5] in one session and L[6] in another - gets the new shape.
    if(HGemPtr==NULL)
        return;
    HGemPtr->ResetReturnCode();
    int len = 0;
    unsigned char Type = 0;
    AnsiString sTmp;
    //AI(secs-msggap-fix) 20260728 : same rule as S6F16 - never return without sending. RPTID 0
    //  is not defined, so EmitIndividualReport answers the well-formed empty L,0 that S6F20
    //  already uses for an unknown report. See the S6F16 comment for why this is reachable
    //  without a malformed host.
    unsigned uRpt = 0;
    if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
    {
        HGemPtr->StringOut("[SECS] S6F19 format error (empty body) - replying S6F20 L,0");
    }
    else
    {
        bool bReadable = true;
        if(Type==HType.LIST_TYPE)
        {
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(len, HType.LIST_TYPE)!=1 ||
               HGemPtr->GetDataItemLenAndType(len, Type)!=1)
            {
                HGemPtr->StringOut("[SECS] S6F19 format error (bad list wrapper) - replying S6F20 L,0");
                bReadable = false;
            }
        }
        if(bReadable)
        {
            if(HGemPtr->DataItemIn(len, Type, sTmp)!=1)
                HGemPtr->StringOut("[SECS] S6F19 format error (RPTID unreadable) - replying S6F20 L,0");
            else
                uRpt = (unsigned)strtoul(sTmp.c_str(), NULL, 10);   //saturates; atoi would wrap a U4 >= 2^31
        }
    }
    HGemPtr->EmitIndividualReport(uRpt);
}
//---------------------------------------------------------------------------
//AI(secs-msggap) 20260728 : shared sink for host terminal text (S10F3 and S10F5).
//  Writes one SECS log line, one EventLog CSV row, and queues the raw text on
//  HTGem::SecsAlarmMessage - a TStringList that already exists (allocated in all three
//  HTGem constructors, freed in the dtor) and until now had ZERO consumers.
//  It NEVER pops a dialog. The caller runs inside the HSMS receive callback
//  (THGem::HandleDataMessage -> GemLogic->Dispatch), and every HT160 message box is a
//  ShowModal that suspends MainProc, so a popup here would stall SECS itself.
//  Deliberately NOT classified by severity: the host text cannot be judged in firmware.
//  Substring matching is provably broken on this very corpus - the purely informational
//  "[MIScheduleSetAlarm]|HP93K-1042EXA is IDLE now." contains the word "Alarm", while the
//  real action text "[OMS1] Criteria Check Alarm" arrives with TID=0x00 in one capture and
//  TID=0x01 in another, and informational texts also arrive with TID=0x01.
//AI(secs-msggap-fix) 20260729 : TID is not a severity LEVEL, but it is not meaningless either -
//  9045 consumes it as iSECSMessageCanCloseByOperator and its message box keys the dismissal
//  gate off it (0 = normal flow, 1 = operator may close directly, 2 = employee-ID check). Read
//  that way this host's usage is mostly coherent with one off-pattern text. So : it is a
//  DISMISSAL-POLICY flag, not a severity rank, and this host's use of it is not fully
//  self-consistent - which is why nothing here branches on it. Whoever implements the phase-2
//  operator popup must honour it as a dismissal gate, not sort by it.
//  Everything is logged identically; the operator reads it.
//  CR/LF are flattened for the log/CSV copy (one row per message); the queued copy keeps
//  its line breaks for a future popup.
//---------------------------------------------------------------------------
static void SinkHostTerminalText(THGem *Gem, TStringList *Queue, int F,
                                 unsigned char TID, AnsiString sText, unsigned char ACKC10)
{
    AnsiString sFlat = sText;
    for(int i=1; i<=sFlat.Length(); i++)
    {
        if(sFlat[i]=='\r' || sFlat[i]=='\n')
            sFlat[i] = ' ';
    }
    AnsiString sMsg;
    sMsg.sprintf("S10F%d TID=%u ACKC10=%u : %s",
                 F, (unsigned)TID, (unsigned)ACKC10, sFlat.c_str());
    if(Gem!=NULL)
    {
        Gem->StringOut("[SECS] host terminal text " + sMsg);
        Gem->FlushSecsLogToFile();
    }
    g_EventLog.Log("SECS_TERM", sMsg, "");
    if(Queue!=NULL && sText!="")
    {
        Queue->Add(sText);
        while(Queue->Count > 50)   // bounded : nothing drains this in phase 1
            Queue->Delete(0);
    }
}
//---------------------------------------------------------------------------
void HT160Gem::S10F4_TerminalDisplaySingleAcknowledge()
{
    //AI(secs-msggap) 20260728 : host S10F3 Terminal Display Single -> reply S10F4 <B ACKC10>.
    //  Body: L,2{ B[1] TID, A[n] TEXT }. Bare binary reply, no list wrapper - same single-byte
    //  triad as S5F4 / S1F16 / S1F18 / S2F32 (HT160 has no LocalAcknowledge wrapper).
    //  The Dispatch case already existed but landed on the base SendUnsupported stub, which
    //  only writes a log line - that is why the host T3s today.
    //  ACKC10 per SEMI E5: 0=accepted, 1=will not be displayed, 2=terminal not available.
    //  0 on a clean parse (the text IS accepted - logged, CSV'd and queued), 1 on any format
    //  error. 2 is not used: HT9045 answered 0 every single time at KYEC, so 0 is the
    //  field-proven answer.
    //  TEXT READ - safety critical: the literal HT9045 form for THIS function is
    //  `char str[1024]` plus DataItemIn(1024, ASCII_TYPE, str). That OVERFLOWS on HT160, because
    //  DataItemInSub only guards `if(l>len) return -2;` and then does strncpy(temp, src, len+1)
    //  - 1025 bytes into a 1024 buffer. The safe form is to peek the REAL length with
    //  GetDataItemLenAndType and hand it to the AnsiString overload, which allocates
    //  new char[Len+4] and null-terminates at [Len]. No fixed buffer at all.
    //AI(secs-msggap-fix) 20260729 : do NOT generalise that 1-byte-overrun description to S10F6.
    //  9045's S10F6 is WORSE : it peeks the request's own length and passes it straight to
    //  DataItemIn with no 1024 clamp at all, so a verbatim port there is an unbounded stack
    //  smash, not an off-by-one. Both HT160 handlers use the AnsiString overload, so neither is
    //  exposed - but a maintainer reading only the paragraph above would "fix" S10F6 by capping
    //  at 1024, which is not even the defect.
    if(HGemPtr==NULL)
        return;
    HGemPtr->ResetReturnCode();
    unsigned char TID    = 0;
    unsigned char ACKC10 = 0;
    int len = 0;
    unsigned char Type = 0;
    AnsiString sText;
    if(HGemPtr->DataItemIn(2, HType.LIST_TYPE, NULL)!=1)
        ACKC10 = 1;
    else if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
        ACKC10 = 1;
    else if(Type!=HType.BINARY_TYPE && Type!=HType.UINT_1_TYPE)
        ACKC10 = 1;
    else if(HGemPtr->DataItemIn(1, Type, &TID)!=1)
        ACKC10 = 1;
    else if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
        ACKC10 = 1;
    else if(Type!=HType.ASCII_TYPE || len<0 || len>4096)
        ACKC10 = 1;
    else if(HGemPtr->DataItemIn(len, Type, sText)!=1)
        ACKC10 = 1;

    HGemPtr->InitLocalHead(10, 4, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &ACKC10);
    HGemPtr->SendLocalData();

    SinkHostTerminalText(HGemPtr, SecsAlarmMessage, 3, TID, sText, ACKC10);
}
//---------------------------------------------------------------------------
void HT160Gem::S10F6_TerminalDisplayMultiBlockAcknowledge()
{
    //AI(secs-msggap) 20260728 : host S10F5 Terminal Display Multi-block -> reply S10F6.
    //  Body: L,2{ B[1] TID, L,n{ A TEXT ... } }, n observed 1..3 across the 7 KYEC captures.
    //  Lines are joined with CRLF exactly like HT9045, but read via the AnsiString overload
    //  (see the buffer-overflow note in S10F4). Reply is the same bare <B ACKC10>.
    if(HGemPtr==NULL)
        return;
    HGemPtr->ResetReturnCode();
    unsigned char TID    = 0;
    unsigned char ACKC10 = 0;
    int n = 0, len = 0, i = 0;
    unsigned char Type = 0;
    AnsiString sLine;
    AnsiString sText;
    if(HGemPtr->DataItemIn(2, HType.LIST_TYPE, NULL)!=1)
        ACKC10 = 1;
    else if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
        ACKC10 = 1;
    else if(Type!=HType.BINARY_TYPE && Type!=HType.UINT_1_TYPE)
        ACKC10 = 1;
    else if(HGemPtr->DataItemIn(1, Type, &TID)!=1)
        ACKC10 = 1;
    else if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1 || n<0 || n>64)
        ACKC10 = 1;
    else
    {
        for(i=0; i<n; i++)
        {
            if(HGemPtr->GetDataItemLenAndType(len, Type)!=1 ||
               Type!=HType.ASCII_TYPE || len<0 || len>4096 ||
               HGemPtr->DataItemIn(len, Type, sLine)!=1)
            {
                ACKC10 = 1;
                break;
            }
            if(sText.Length() < 8192)
                sText = sText + sLine + "\r\n";
        }
    }

    HGemPtr->InitLocalHead(10, 6, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &ACKC10);
    HGemPtr->SendLocalData();

    SinkHostTerminalText(HGemPtr, SecsAlarmMessage, 5, TID, sText, ACKC10);
}
//---------------------------------------------------------------------------
void HT160Gem::S125F2_EnableDisableECDataAcknowledge()
{
    //AI(secs-msggap) 20260728 : host S125F1 Enable/Disable EC Data Send (KYEC private stream)
    //  -> reply exactly ONE S125F2 <B ACK>. Body: L,2{ B[1] ALED, L,n{ U4 ECID ... } }.
    //  KYEC sends a pair per session: ALED=0x01 with L,0 (disable all), then ALED=0x80 with
    //  L,45. ALED is a BIT-7 flag on this host, not a boolean - HT9045 tests `T & 0x80`, which
    //  is why 0x01 means DISABLE despite looking like a true.
    //  Nothing is stored. The only thing the EC-enable list gates on HT9045 is whether an EC
    //  change fires CEID 48, and HT160 registers no such CEID and has no EC-enable table, so
    //  an enable table here would be dead weight. The referenced ECIDs are logged instead,
    //  the same referenced-set discovery logging S2F33 does for unknown SVIDs.
    //  ACK 0=Acknowledge, 1=Denied. Exactly ONE reply per request - see the Dispatch comment
    //  for why HT9045's 45-acks-per-request loop is a defect and is not ported.
    //AI(secs-msggap-fix) 20260729 : ACK=1 is also returned when the list names an ECID this
    //  machine does not register, so a 0 never claims an enable that cannot happen. See the
    //  in-loop comment for the 9045 and KYEC-wire evidence.
    if(HGemPtr==NULL)
        return;
    HGemPtr->ResetReturnCode();
    unsigned char ALED = 0;
    unsigned char ACK  = 0;
    int n = 0, len = 0, i = 0;
    int nUnknownEc = 0;                  //AI(secs-msggap-fix) 20260729 : ECIDs this machine does not register
    unsigned char Type = 0;
    AnsiString sTmp;
    AnsiString sIds;
    AnsiString sUnknownIds;
    if(HGemPtr->DataItemIn(2, HType.LIST_TYPE, NULL)!=1)
        ACK = 1;
    else if(HGemPtr->GetDataItemLenAndType(len, Type)!=1)
        ACK = 1;
    else if(Type!=HType.BINARY_TYPE && Type!=HType.UINT_1_TYPE)
        ACK = 1;
    else if(HGemPtr->DataItemIn(1, Type, &ALED)!=1)
        ACK = 1;
    else if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1 || n<0 || n>256)
        ACK = 1;
    else
    {
        for(i=0; i<n; i++)
        {
            if(HGemPtr->GetDataItemLenAndType(len, Type)!=1 ||
               HGemPtr->DataItemIn(len, Type, sTmp)!=1)
            {
                ACK = 1;
                break;
            }
            if(i<32)
            {
                if(sIds!="")
                    sIds = sIds + ",";
                sIds = sIds + sTmp;
            }
            //AI(secs-msggap-fix) 20260729 : do not ACK=0 an ECID this machine does not have.
            //A 0 tells the host "EC-change reporting is now enabled for that ECID" - it is not,
            //and with no EC-enable table and no EC-change event on HT160 it never will be.
            //HT9045 answers 1 per unregistered ECID (uHGemClass.cpp:2650-2653), wire-proven at
            //KYEC by the four 0x01 acks at list positions 7/8/10/45 = ECIDs 2101/2102/2004/8506,
            //the exact four missing from the whole 9045 tree. HT160 sends ONE ack per request, so
            //the honest compression is : any unregistered ECID -> ACK=1 for the request, and name
            //the offenders in the log. KYEC's list is 45 ECIDs of which HT160 owns exactly one
            //(1501), so this normally answers 1 - which is the truth, where 0 was a lie.
            //GetECName() is used instead of the private FindECItem(): it returns "" for an
            //unregistered ECID, and all seven ECs this machine registers have a non-empty name.
            if(HGemPtr->GetECName((unsigned)strtoul(sTmp.c_str(), NULL, 10))=="")
            {
                nUnknownEc++;
                if(nUnknownEc<=32)
                {
                    if(sUnknownIds!="")
                        sUnknownIds = sUnknownIds + ",";
                    sUnknownIds = sUnknownIds + sTmp;
                }
            }
        }
        if(ACK==0 && nUnknownEc>0)
            ACK = 1;                                 // list references ECIDs this machine has no EC for
    }

    HGemPtr->InitLocalHead(125, 2, 0);
    HGemPtr->DataItemOut(1, HType.BINARY_TYPE, &ACK);
    HGemPtr->SendLocalData();

    const char *pMode = "disable";
    if((ALED & 0x80)!=0)
        pMode = "enable";
    AnsiString sLog;
    sLog.sprintf("[SECS] S125F2 ACK=%u ALED=0x%02X (%s) ECIDcount=%d notRegistered=%d",
                 (unsigned)ACK, (unsigned)ALED, pMode, n, nUnknownEc);
    HGemPtr->StringOut(sLog);
    if(sIds!="")
        HGemPtr->StringOut("[SECS] S125F1 referenced ECIDs (first 32): " + sIds);
    //AI(secs-msggap-fix) 20260729 : the not-registered set is the discovery list - it is what the
    //host expects this machine to report EC changes for and what a future EC port would add.
    if(sUnknownIds!="")
        HGemPtr->StringOut("[SECS] S125F1 ECIDs NOT registered here (first 32): " + sUnknownIds);
    HGemPtr->FlushSecsLogToFile();
}
//---------------------------------------------------------------------------
int HT160Gem::S7F2_ProcessProgramLoadGrant()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S7F2 recipe load is disabled for HT160 framework skeleton");
    return 1;
}
//---------------------------------------------------------------------------
void HT160Gem::S7F4_ProcessProgramAcknowledge()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S7F4 recipe acknowledge is disabled for HT160 framework skeleton");
}
//---------------------------------------------------------------------------
void HT160Gem::S7F6_ProcessProgramData()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S7F6 recipe upload is disabled for HT160 framework skeleton");
}
//---------------------------------------------------------------------------
void HT160Gem::S7F6_ProcessProgramData(AnsiString FileName)
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S7F6 recipe upload is disabled for HT160 framework skeleton: " + FileName);
}
//---------------------------------------------------------------------------
void HT160Gem::ProcessS14F1_GetAttrRequest(AnsiString asTrayID)
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S14F1 tray attribute request is disabled for HT160 framework skeleton: " + asTrayID);
}
//---------------------------------------------------------------------------
unsigned char HT160Gem::ProcessS14F2_GetAttrData()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S14F2 tray attribute data is disabled for HT160 framework skeleton");
    return 1;
}
//---------------------------------------------------------------------------
void HT160Gem::ReloadParameter()
{
}
//---------------------------------------------------------------------------
void HT160Gem::LookForFile()
{
}
//---------------------------------------------------------------------------
