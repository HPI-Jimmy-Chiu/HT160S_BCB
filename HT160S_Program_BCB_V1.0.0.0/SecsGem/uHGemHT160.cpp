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

    //AI(ht160s-secsgem) 20260611 : SVID numbering follows HT9045 AddSV for the
    // common identity/time/throughput band (1001 Machine Model, 1003 Software
    // Version, 1021 UPH, 1027 System Time) so a shared host SVID table works across
    // machines. HT160-specific run/alarm/output/lot SVs are relocated to a 1100+
    // band to avoid colliding with the 9045 1010-1036 region.
    // -- 9045-aligned common band --
    HGemPtr->SetSVDataPointer(1001, HType.ASCII_TYPE, "Machine Model", "", &HandlerPath, "machine model name (9045 SVID 1001)");
    HGemPtr->SetSVDataPointer(1003, HType.ASCII_TYPE, "Software Version", "", &svSoftwareVersion, "application software version (9045 SVID 1003)");
    HGemPtr->SetSVDataPointer(1021, HType.INT_4_TYPE, "UPH", "pcs/hr", &svUPH, "units per hour (9045 SVID 1021)");
    HGemPtr->SetSVDataPointer(1027, HType.ASCII_TYPE, "System Time", "", &sSystemTime, "current system time (9045 SVID 1027)");

    //AI(ht160s-secsgem) 20260611 : HT160 custom band 1100+. Values are snapshotted
    // into the sv* members by RefreshSVData() right before each serialize, so the
    // bound address stays valid while the reported value tracks live data.
    // -- Machine state --
    HGemPtr->SetSVDataPointer(1100, HType.INT_4_TYPE, "Run Mode", "", &svRunMode, "0=Normal 1=Home 2=OneCycle 3=CleanOut 4=TrayFeed");
    HGemPtr->SetSVDataPointer(1101, HType.INT_4_TYPE, "System Running", "", &svSystemRunning, "1=machine started/running, 0=stopped");
    HGemPtr->SetSVDataPointer(1102, HType.INT_4_TYPE, "Control State", "", &iControlState, "GEM control state mirror (4=Local 5=Remote)");
    // -- Alarm state --
    HGemPtr->SetSVDataPointer(1110, HType.INT_4_TYPE, "Alarm Active", "", &svAlarmActive, "1=alarm dialog showing, 0=no alarm");
    HGemPtr->SetSVDataPointer(1111, HType.INT_4_TYPE, "Alarm Code", "", &svAlarmCode, "current active alarm code (0=none)");
    // -- Output / production --
    HGemPtr->SetSVDataPointer(1120, HType.INT_4_TYPE, "Total IC", "pcs", &svTotalIC, "total IC processed this lot/run");
    HGemPtr->SetSVDataPointer(1121, HType.INT_4_TYPE, "Total Sorted", "pcs", &svTotalSorted, "total IC sorted into a bin");
    // -- Current Lot --
    HGemPtr->SetSVDataPointer(1130, HType.INT_4_TYPE, "Active Lot Count", "", &svLotCount, "lots currently loaded on the machine");
    HGemPtr->SetSVDataPointer(1131, HType.ASCII_TYPE, "Current Lot ID", "", &svCurrentLot, "first registered lot id (empty if none)");
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
    // -- Tray Form geometry (data\<recipe>\setup.ini [TrayForm]) --
    HGemPtr->SetECDataPointer(2011, HType.FT_8_TYPE,  "Tray X Start",    "mm", &TrayForm.XStart,   "", "", "0", "tray form X start position");
    HGemPtr->SetECDataPointer(2012, HType.FT_8_TYPE,  "Tray X Pitch",    "mm", &TrayForm.XPitch,   "", "", "0", "tray form X pitch");
    HGemPtr->SetECDataPointer(2013, HType.FT_8_TYPE,  "Tray Y Start",    "mm", &TrayForm.YStart,   "", "", "0", "tray form Y start position");
    HGemPtr->SetECDataPointer(2014, HType.FT_8_TYPE,  "Tray Y Pitch",    "mm", &TrayForm.YPitch,   "", "", "0", "tray form Y pitch");
    HGemPtr->SetECDataPointer(2015, HType.INT_4_TYPE, "Tray X Division", "",   &TrayForm.XDivision,"", "", "0", "tray form columns (X count)");
    HGemPtr->SetECDataPointer(2016, HType.INT_4_TYPE, "Tray Y Division", "",   &TrayForm.YDivision,"", "", "0", "tray form rows (Y count)");
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
}
//---------------------------------------------------------------------------
void HT160Gem::AddReprot()
{
    int EquDefault = 1;
    //AI(ht160s-secsgem) 20260611 : report 1 carries the real machine-data SVs on
    // every event, using the 9045-aligned numbering (1001/1003/1021/1027) plus the
    // HT160 custom 1100+ band.
    unsigned ReportIDContent[16];

    if(HGemPtr==NULL)
        return;

    ReportIDContent[ 0] = 1001;  // Machine Model     (9045-aligned)
    ReportIDContent[ 1] = 1003;  // Software Version  (9045-aligned)
    ReportIDContent[ 2] = 1021;  // UPH               (9045-aligned)
    ReportIDContent[ 3] = 1027;  // System Time       (9045-aligned)
    ReportIDContent[ 4] = 1100;  // Run Mode          (HT160 custom)
    ReportIDContent[ 5] = 1101;  // System Running
    ReportIDContent[ 6] = 1102;  // Control State
    ReportIDContent[ 7] = 1110;  // Alarm Active
    ReportIDContent[ 8] = 1111;  // Alarm Code
    ReportIDContent[ 9] = 1120;  // Total IC
    ReportIDContent[10] = 1121;  // Total Sorted
    ReportIDContent[11] = 1130;  // Active Lot Count
    ReportIDContent[12] = 1131;  // Current Lot ID
    HGemPtr->SetReportIDContent(1, 13, ReportIDContent, EquDefault);
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
        if(ECID>=2011 && ECID<=2016)
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
//AI(ht160s-secsgem) 20260611 : local GUI EC editor write path. Enforces the
//  exact same policy as the host S2F16: only the tray-form geometry ECs
//  (2011-2016) are settable, only while the machine is idle, and a successful
//  write is persisted to the active recipe's setup.ini. Returns 0=ok,
//  1=not settable / unknown, 2=busy, 3=convert/range error.
int HT160Gem::GuiWriteTrayEC(unsigned ECID, AnsiString sValue)
{
    if(HGemPtr==NULL)
        return 1;

    bool bBusy = (HSys.Sys.SystemStart==true || HasICUnderMachine()==true);
    if(bBusy)
        return 2;

    if(ECID<2011 || ECID>2016)
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
                fMain->RefreshLotListFromRegistry();
        }
        else if(S.AnsiPos("PAUSE")==1)
        {
            //AI(ht160s-secsgem) 20260611 : PAUSE host command. Consume the (usually
            // empty) parameter list, then replicate the operator sbPause side-effects
            // ONLY: drop SystemStart and raise SoftStop. RecordProcess/EventReport are
            // TfMain methods and are intentionally not called from the GEM layer.
            HGemPtr->GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE);
            HSys.Sys.SystemStart = false;
            SoftStop = true;
            HCACK = 0;
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
    return 1;
}
//---------------------------------------------------------------------------
void HT160Gem::S5F6_ListAlarmData()
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S5F6 alarm list is empty in HT160 framework skeleton");
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
