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
#include "note.h"          // fNote (alarm dialog : fShow / Code)
#include "cmydef.h"        // SoftStop (S2F42 PAUSE host command)
#include "uAgvStation.h"   // AI(ht160s-agv) 20260615 : E87/AGV station table + AgvCoord
#include "uAmrInject.h"   // AI(ht160s-agv) 20260708 : AMR manual-inject alert (HCACK!=0 surfacing)
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
    // edge. The matching Unloadtray CEIDs (136-138/140-142, fired in aAuto1To6
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

    HGemPtr->SaveEventReportData();
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
                    LotRegistry.Clear();                         // D1 overwrite
                    HCACK = 0;
                    for(i=0; i<n; i++)
                    {
                        HGemPtr->GetDataItemLenAndType(len, Type);
                        if(Type==HType.ASCII_TYPE && len>0 && len<(int)sizeof(str))
                        {
                            if(HGemPtr->DataItemIn(len, HType.ASCII_TYPE, str)==1)
                            {
                                AnsiString lot = str;
                                LotRegistry.AddLot(lot, HT160_LOT_SOURCE_SECS, "", "");
                                if(i==0 && fMain!=NULL)
                                    fMain->edLotNo->Text = lot; // D2 backfill first lot
                            }
                            else
                            {
                                HCACK = 2;                       // read failure -> param error
                                break;
                            }
                        }
                        else
                        {
                            HCACK = 2;                           // type mismatch -> param error
                            break;
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
                    AnsiString FirstLot = "";
                    HCACK = 0;
                    for(i=0; i<n; i++)
                    {
                        HGemPtr->GetDataItemLenAndType(len, Type);
                        if(Type==HType.ASCII_TYPE && len>0 && len<(int)sizeof(str))
                        {
                            if(HGemPtr->DataItemIn(len, HType.ASCII_TYPE, str)==1)
                            {
                                AnsiString lot = str;
                                LotRegistry.AddLot(lot, HT160_LOT_SOURCE_SECS, "", "");
                                if(FirstLot=="")
                                    FirstLot = lot;
                            }
                            else
                            {
                                HCACK = 2;                       // read failure -> param error
                                break;
                            }
                        }
                        else
                        {
                            HCACK = 2;                           // type mismatch -> param error
                            break;
                        }
                    }
                    if(HCACK==0 && FirstLot!="" && fMain!=NULL)
                    {
                        //AI(ht160s-lot-reset) 20260706 : SECS LOTSTART is the host-side
                        //equivalent of pressing Lot Start, so zero the per-run production
                        //counters too (gated idle : SystemStart==false and no IC inside).
                        ResetPerLotProductionCounters();
                        //AI(ht160s-uph) 20260706 : open the per-tray/lot UPH log folder.
                        TrayUphLog_OnLotStart(FirstLot);
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
            int innerLen;
            if(HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)==1)
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

                        if(cpName.Trim().UpperCase()=="LOADERTRAYCOUNT")
                            AgvCoord.TrayCount[0] = StrToIntDef(cpVal, 0);  // P1 Loader expected trays
                        else if(AgvCoord.BeginPrep(cpName)==false)
                            HCACK = 2;                           // unknown CP name -> param error
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
    if(HCACK!=0) AmrInject.NoteHostReject(S, HCACK);   // AI(ht160s-agv) 20260708 : surface rejected host cmd on AMR tab
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
    //AI(ht160s-maintainer) 20260619 : reply to S5F5 with a well-formed (empty)
    //S5F6 alarm list so a W-bit host request does not hit a T3 timeout. HT160
    //has no alarm catalog store yet; emit an empty L,0 until one exists.
    if(HGemPtr==NULL)
        return;
    HGemPtr->InitLocalHead(5, 6, 0);
    HGemPtr->DataItemOut(0, HType.LIST_TYPE, NULL);
    HGemPtr->SendLocalData();
    HGemPtr->StringOut("[SECS] S5F6 sent empty alarm list (HT160 has no alarm catalog yet)");
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
