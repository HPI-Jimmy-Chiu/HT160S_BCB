//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <string.h>
#include <stdlib.h>
#include <stdio.h>   //AI(ht160s-secsgem) 20260611 : fopen/fputs for SECS file log
#include "uHGemEquipment.h"
#include "uHGemClass.h"   //AI(ht160s-secsgem) 20260610 : HTGem for Dispatch back-pointer
#include "database.h"   //AI(HT160S-Maintainer) 20260609 : for HSys.CurrentDir
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
THGem *HGem = NULL;
HTypeStruct HType = {0x00, 0x40, 0x44, 0x20, 0x24, 0xA4, 0xA8, 0xB0, 0xA0, 0x64, 0x68, 0x70, 0x60, 0x90, 0x80, 0xFF};
//AI(ht160s-secsgem) 20260716 : caps for the SML body dump so a pathological item
//  (huge process-program / XML blob) cannot flood the disk log. Host commands are
//  small, so these bounds never truncate a normal S2F41; they only guard outliers.
#define SECS_SML_ASCII_CAP 4096   // max ASCII bytes rendered per item
#define SECS_SML_BIN_CAP     64   // max binary bytes rendered per item
#define SECS_SML_RAW_CAP    512   // max raw bytes in the hex fallback on parse error
//---------------------------------------------------------------------------
__fastcall THGem::THGem(TComponent *Owner)
    : TComponent(Owner)
{
    iTimeFormat = 1;
    iRecipeDirectoryType = 0;
    Alias = "SECS";
    CurrentDirectory = HSys.CurrentDir + "\\SECS";   //AI(HT160S-Maintainer) 20260609 : was "..\\SECS"; anchor to program root
    LogList = new TStringList;
    //AI(ht160s-secsgem) 20260611 : disk-log buffer; default on, finally gated in
    //  GemInitial by CosFunction.bUseSecsGem + [SECS] LogToFile.
    LogFileBuffer = new TStringList;
    bLogToFile    = true;
    Timer1 = new TTimer(this);
    Timer1->Enabled = false;
    Timer1->Interval = 1000;
    Timer1->OnTimer = Timer1Timer;

    //AI(ht160s-secsgem) 20260610 : Phase 0 transmit-side encode codec init
    LocalBufferSize = 1024 * 1024;          // 1 MB encode buffer (heap)
    LocalBuffer = new unsigned char[LocalBufferSize];
    LocalLength = 0;
    LocalLength_4 = 4;
    bEncodeOverflow = false;                 //AI(secs-audit-fix) 20260729
    EquipmentSystemByte = 0;
    DeviceID = 0;
    memset(&Local, 0, sizeof(Local));
    memset(&Remote, 0, sizeof(Remote));

    //AI(ht160s-secsgem) 20260610 : Phase 0 receive-side decode codec init
    SReceiveData = new TStringList;
    bReceiveData = false;
    iReturnCode = 1;

    //AI(ht160s-secsgem) 20260611 : SV/EC/CEID/Report registry init
    SVList     = new TList;
    ECList     = new TList;
    ReportList = new TList;
    CEIDList   = new TList;
    bHostManagesReports = false;   //AI(secs-reportdef) 20260724
    bEventDefLoaded     = false;   //AI(secs-reportdef) 20260724

    //AI(ht160s-secsgem) 20260610 : Phase 0 HSMS-SS socket transport init
    GemLogic     = NULL;
    ActiveSocket = NULL;
    bActiveMode  = false;                    // default passive (equipment listens)
    bCommStarted = false;
    iHsmsState   = HSMS_STATE_NOTCONNECTED;
    RecvBuffer   = new TMemoryStream;
    bRecvBufferReset = false;                //AI(secs-audit-fix) 20260729

    //AI(ht160s-secsgem) 20260611 : reconnect watchdog defaults (overridden by
    // General.ini [SECS] ReconnectInterval via SetReconnectInterval in GemInitial).
    bWantComm          = false;
    iReconnectInterval = 5;                  // seconds; 0 = disabled
    iReconnectCountdown= 0;
    iReconnectAttempts = 0;
    iIdleLogDay        = 0;                       //AI(ht160s-secsgem) 20260801 : no "no host" marker written yet

    //AI(ht160s-secsgem) 20260611 : Linktest heartbeat / T6 timeout defaults
    // (overridden by General.ini [SECS] LinktestInterval / T6Timeout).
    iLinktestInterval  = 10;                 // seconds; 0 = heartbeat off
    iLinktestCountdown = 0;
    iT6Timeout         = 6;                   // seconds to wait for Linktest.rsp
    bLogLinktest       = false;               //AI(ht160s-secsgem) 20260612 : quiet heartbeat by default
    bLogSmlBody        = true;                //AI(ht160s-secsgem) 20260716 : full SML body dump on by default (gated by [SECS] LogSmlBody)
    //AI(secs-strict-reportdef) 20260810 : DEFAULT TRUE = E5-conformant strict report validation.
    // Overridden by General.ini [SECS] StrictReportValidation (see GemInitial). See the S2F33
    // header comment for why the old Path A tolerance was wrong and what strict costs.
    bStrictReportValidation = true;
    iT6Countdown       = 0;
    bAwaitLinktestRsp  = false;
    uControlSystemByte = 1;

    ClientSocket1 = new TClientSocket(this);
    ClientSocket1->ClientType   = ctNonBlocking;
    ClientSocket1->Active       = false;
    ClientSocket1->OnConnect    = ClientConnect;
    ClientSocket1->OnDisconnect = ClientDisconnect;
    ClientSocket1->OnRead       = ClientRead;
    ClientSocket1->OnError      = ClientError;

    ServerSocket1 = new TServerSocket(this);
    ServerSocket1->ServerType         = stNonBlocking;
    ServerSocket1->Active             = false;
    ServerSocket1->OnClientConnect    = ServerClientConnect;
    ServerSocket1->OnClientDisconnect = ServerClientDisconnect;
    ServerSocket1->OnClientRead       = ServerClientRead;
    ServerSocket1->OnClientError      = ServerClientError;
}
//---------------------------------------------------------------------------
__fastcall THGem::~THGem()
{
    //AI(secs-shutdown-uaf) 20260806 : null the GLOBAL back-pointer the moment this object
    //dies. THGem belongs to Application while HT160Gem is HSys.MyGem, freed by the global
    //SYSTEM_MODULAR dtor AFTER Application's teardown - so ~HT160Gem used to reach a live
    //check "HGemPtr!=NULL" on a pointer to THIS, already-freed, object and write through it
    //(SetGemLogic(NULL) = a store into freed memory). Latent since 20260729; it surfaced as a
    //100%-reproducible 0xC0000005 at selftest exit once an unrelated heap-size change let the
    //freed page be decommitted (Windows fault offset 0x12f2a1 -> THGem::SetGemLogic, mapped
    //via a -m linker map). With the global nulled here, ~HT160Gem's "HGem==HGemPtr" liveness
    //test (its side of this fix) skips the dead call in either destruction order.
    if(HGem==this)
        HGem = NULL;
    //AI(secs-audit-fix) 20260729 : drop the machine-logic back-pointer FIRST. Below we set
    //ClientSocket1/ServerSocket1->Active=false, and ScktComp's TCustomWinSocket::Disconnect
    //fires seDisconnect SYNCHRONOUSLY on a live socket -> ClientDisconnect ->
    //OnPeerDisconnected() -> GemLogic->OnCommunicationLost(). That virtual call runs
    //machine-layer code (ClearSecsPanelOverride, RecordProcess -> EventLog fopen, fNote memo)
    //at a point where this object's LogList / SReceiveData / registries are already freed, and
    //HT160Gem may itself already be gone : the two objects have different owners (THGem belongs
    //to Application, HT160Gem is HSys.MyGem freed by the global SYSTEM_MODULAR dtor) and
    //nothing sequences their teardown. Clearing the pointer here makes the whole window inert.
    GemLogic = NULL;
    if(LogList!=NULL)
    {
        LogList->Clear();
        delete LogList;
        LogList = NULL;
    }
    //AI(ht160s-secsgem) 20260611 : flush any tail lines then free the disk buffer.
    if(LogFileBuffer!=NULL)
    {
        try { FlushSecsLogToFile(); } catch(...) {}
        LogFileBuffer->Clear();
        delete LogFileBuffer;
        LogFileBuffer = NULL;
    }
    if(LocalBuffer!=NULL)
    {
        delete [] LocalBuffer;
        LocalBuffer = NULL;
    }
    if(SReceiveData!=NULL)
    {
        SReceiveData->Clear();
        delete SReceiveData;
        SReceiveData = NULL;
    }
    //AI(ht160s-secsgem) 20260611 : free SV/EC/CEID/Report registry items + lists.
    if(SVList!=NULL)
    {
        for(int i=0; i<SVList->Count; i++) delete (TGemSVItem*)SVList->Items[i];
        delete SVList; SVList = NULL;
    }
    if(ECList!=NULL)
    {
        for(int i=0; i<ECList->Count; i++) delete (TGemECItem*)ECList->Items[i];
        delete ECList; ECList = NULL;
    }
    if(ReportList!=NULL)
    {
        for(int i=0; i<ReportList->Count; i++) delete (TGemReportItem*)ReportList->Items[i];
        delete ReportList; ReportList = NULL;
    }
    if(CEIDList!=NULL)
    {
        for(int i=0; i<CEIDList->Count; i++) delete (TGemCEIDItem*)CEIDList->Items[i];
        delete CEIDList; CEIDList = NULL;
    }
    //AI(ht160s-secsgem) 20260610 : Phase 0 HSMS-SS socket transport teardown.
    // ClientSocket1/ServerSocket1 are owned by this component and freed by the
    // base dtor; just stop them.  RecvBuffer (TMemoryStream) is freed here.
    if(ClientSocket1!=NULL) ClientSocket1->Active = false;
    if(ServerSocket1!=NULL) ServerSocket1->Active = false;
    ActiveSocket = NULL;
    if(RecvBuffer!=NULL)
    {
        delete RecvBuffer;
        RecvBuffer = NULL;
    }
}
//---------------------------------------------------------------------------
void __fastcall THGem::Timer1Timer(TObject *Sender)
{
    //AI(ht160s-secsgem) 20260612 : sync the main-screen SECS status badge to the
    //  live HSMS link state every tick. Routed through the machine-specific GEM
    //  logic so the lean transport engine stays decoupled from the main form.
    //  Edge-triggered inside UpdateSecsFeatureBadge() so VCL is touched only on
    //  a real state change. Done before the bWantComm guard so the badge still
    //  drops to OFF while the link is down / reconnecting.
    if(GemLogic != NULL)
        GemLogic->RefreshSecsBadge();

    //AI(ht160s-agv) 20260615 : drive the E87/AGV coordinator each tick (Phase B
    //  car-full -> CEID272; Phase D handshake). Routed through GemLogic so the lean
    //  transport stays decoupled from the AGV/machine modules. Done before the
    //  bWantComm guard so polling/snapshot continues; EventReport itself no-ops
    //  unless the link is SELECTED.
    if(GemLogic != NULL)
        GemLogic->ServiceAgv();

    //AI(secs-controlstate) 20260803 : publish the GEM control state and fire its change events
    //  each tick (SVID 4 / 9, CEID 141 + 91/92/93), mirroring how HT9045 detects the edge in its
    //  own periodic pass. Before the bWantComm guard for the same reason as the AGV tick above :
    //  the latch must track the state even while the link is down, and EventReport itself no-ops
    //  unless HSMS is SELECTED.
    if(GemLogic != NULL)
        GemLogic->PollGemControlState();

    //AI(ht160s-secsgem) 20260611 : batch-flush pending SECS log lines to disk
    //  every tick.  Done first (before the bWantComm guard) so logs are written
    //  even while the link is down / reconnecting.
    FlushSecsLogToFile();

    //AI(ht160s-secsgem) 20260611 : periodic HSMS reconnect watchdog (1s tick).
    //  StartCommunication() only fires once at boot, so without this a failed
    //  active dial or a dropped link would never recover.  Non-blocking, no FSM.
    if(!bWantComm)                          return;   // user not asking to connect

    //AI(ht160s-secsgem) 20260611 : Linktest heartbeat + T6 while SELECTED so a
    //  silently-dead peer (RST / killed host / half-open) is detected in seconds.
    if(iHsmsState == HSMS_STATE_SELECTED && iLinktestInterval > 0)
    {
        if(bAwaitLinktestRsp)
        {
            if(iT6Countdown > 0)
                iT6Countdown--;
            if(iT6Countdown <= 0)
            {
                DropConnection("Linktest T6 timeout (peer not responding)");
                return;   // watchdog below will re-arm next tick
            }
        }
        else
        {
            if(iLinktestCountdown > 0)
                iLinktestCountdown--;
            if(iLinktestCountdown <= 0)
                SendLinktestReq();
        }
    }

    if(iReconnectInterval <= 0)             return;   // auto-reconnect disabled
    if(iHsmsState >= HSMS_STATE_CONNECTED)            // TCP up (CONNECTED/SELECTED)
    {
        iReconnectCountdown = iReconnectInterval;     // arm for the next drop
        return;
    }
    if(iReconnectCountdown > 0)
    {
        iReconnectCountdown--;
        return;
    }
    DoReconnectAttempt();
    iReconnectCountdown = iReconnectInterval;
}
//---------------------------------------------------------------------------
void THGem::DoReconnectAttempt()
{
    //AI(ht160s-secsgem) 20260611 : one reconnect try - re-dial (active) or
    //  re-listen (passive).  Wrapped in try/catch so a socket exception cannot
    //  break the 1s timer loop.
    //AI(ht160s-secsgem) 20260801 : KYEC on-site note 1 "SECS passive unconnect record".
    //  In PASSIVE mode the socket work below is guarded on !ServerSocket1->Active, and
    //  after StartCommunication() that flag is never cleared again (the only other writers
    //  are the ctor, the dtor and the uncalled StopCommunication(); DropConnection guards
    //  on bActiveMode, and OnPeerDisconnected / the Separate.req handler close only
    //  ActiveSocket). So the body was unreachable and this function did nothing but bump a
    //  counter and print - yet the StringOut sat OUTSIDE the guard and fired every
    //  iReconnectInterval. On 2026-07-31 that produced 883 lines / 62 KB and left 14 of the
    //  day's 17 hourly SECS log files with zero real content.
    //  Fix: only count and report an attempt when one actually happened. Keep a rare
    //  breadcrumb while the link is down - the counter's continuity is what made that day's
    //  14 h 18 min outage provable, so the record must not disappear entirely.
    int port = atoi(sDefaultPort.c_str());
    if(port <= 0)
        port = 5000;
    AnsiString S;
    try
    {
        if(bActiveMode)
        {
            //AI(ht160s-secsgem) 20260801 : active mode genuinely re-dials on every pass,
            //  so every pass IS an attempt and stays worth one line.
            if(iReconnectAttempts < 1000000000)   // cap : weeks without a host must not overflow
                iReconnectAttempts++;
            if(ClientSocket1 != NULL)
            {
                ClientSocket1->Active  = false;       // drop any half-open socket
                ClientSocket1->Address = sDefaultAddress;
                ClientSocket1->Port    = port;
                ClientSocket1->Active  = true;        // re-dial host
            }
            S.sprintf("[SECS] reconnect #%d (active dial %s:%d)",
                      iReconnectAttempts, sDefaultAddress.c_str(), port);
            StringOut(S);
            return;
        }

        if(ServerSocket1 != NULL && !ServerSocket1->Active)
        {
            //AI(ht160s-secsgem) 20260801 : the listener really was down - this is a real
            //  recovery action and the only passive case worth counting.
            ServerSocket1->Port   = port;
            ServerSocket1->Active = true;         // re-open listen socket
            if(iReconnectAttempts < 1000000000)
                iReconnectAttempts++;
            S.sprintf("[SECS] listen socket re-opened #%d (passive :%d)",
                      iReconnectAttempts, port);
            StringOut(S);
            return;
        }

        //AI(ht160s-secsgem) 20260801 : bound listener, no host. Nothing to retry, so stay
        //  silent - HT9045's passive branch (DoOpenCommuncation, its uHGemEquipment.cpp
        //  :3548-3598) logs nothing here either, and the link edges are already recorded by
        //  OnPeerConnected / OnPeerDisconnected / DropConnection / ServerClientError.
        //
        //  ONE marker per calendar day per process run is kept, and only for this reason:
        //  FlushSecsLogToFile early-outs on an empty buffer, so a day with no SECS traffic
        //  at all never creates D:\HT160S_Log\SECS_GEM\<yyyy_mm_dd>\ - and CaptureSecsLog
        //  then ships NO SecsLog folder in the State Record, which is indistinguishable
        //  from "SECS logging is broken". 2026-07-31 00:00-14:19 is exactly that case.
        //  This is not a periodic heartbeat; it is proof the log itself is alive.
        //
        //  COST, stated so nobody rediscovers it the hard way: an outage can no longer be
        //  measured from this file alone when the disconnect edge fell on a PREVIOUS day.
        //  For the 07-31 outage the disconnect happened on 07-30, so the day marker says
        //  "listening, no host on 07-31" but not for how long. Outages that start and end
        //  inside one session are still exact - subtract the two edge timestamps.
        Word wIdleY, wIdleMo, wIdleD;
        DecodeDate(Now(), wIdleY, wIdleMo, wIdleD);
        int iToday = (int)wIdleY * 10000 + (int)wIdleMo * 100 + (int)wIdleD;
        if(iIdleLogDay != iToday)
        {
            iIdleLogDay = iToday;
            S.sprintf("[SECS] listening :%d, no host connected", port);
            StringOut(S);
        }
    }
    catch(...)
    {
        StringOut("[SECS] reconnect attempt raised an exception (ignored)");
    }
}
//---------------------------------------------------------------------------
void THGem::SetTimeFormat(int Format)
{
    iTimeFormat = Format;
}
//---------------------------------------------------------------------------
void THGem::SetDefaultAddressAndPort(char *Address, char *Port)
{
    SetDefaultAddressAndPort(Address, Port, "0");
}
//---------------------------------------------------------------------------
void THGem::SetDefaultAddressAndPort(char *Address, char *Port, char *DeviceId)
{
    sDefaultAddress = Address;
    sDefaultPort = Port;
    sDeviceId = DeviceId;
    DeviceID = (unsigned int)atoi(sDeviceId.c_str());   //AI(ht160s-secsgem) 20260610 : feed HSMS header DeviceID
}
//---------------------------------------------------------------------------
int THGem::SetReceipeDirectoryAndGlobalName(AnsiString Path, AnsiString FileMask, int Type)
{
    sRecipeDirectory = Path;
    sRecipeFileMask = FileMask;
    iRecipeDirectoryType = Type;
    return 0;
}
//---------------------------------------------------------------------------
void THGem::SetMachineTypeAndSoftwarseVer(char *MachineType, char *SoftwareVersion)
{
    sMachineType = MachineType;
    sSoftwareVersion = SoftwareVersion;
}
//---------------------------------------------------------------------------
void THGem::SaveEventReportData()
{
    //AI(secs-reportdef) 20260724 : persist HOST-defined reports (Mode==0) ONLY. Links to firmware
    //CEIDs and per-CEID enable are session-only by design, so a reboot never sends FEWER events
    //than the firmware baseline. No-op until ReadEventReportData has run (guards the boot-time
    //firmware registration from clobbering the host file with an empty one).
    if(!bEventDefLoaded)
        return;
    AnsiString Dir = HSys.CurrentDir + "\\system";
    AnsiString Fn  = Dir + "\\EventReportDef.ini";
    try { ForceDirectories(Dir); } catch(...) {}
    TStringList *sl = new TStringList;
    try
    {
        sl->Add("FormatVersion=1");
        if(ReportList!=NULL)
        {
            for(int i=0; i<ReportList->Count; i++)
            {
                TGemReportItem *Rp = (TGemReportItem*)ReportList->Items[i];
                if(Rp->Mode!=0) continue;
                AnsiString row = "Report=" + IntToStr((int)Rp->ReportID) + "|";
                for(int s=0; s<Rp->SVCount; s++)
                {
                    if(s>0) row += ",";
                    row += IntToStr((int)Rp->SVIDs[s]);
                }
                sl->Add(row);
            }
        }
        sl->SaveToFile(Fn);
    }
    catch(...) {}
    delete sl;
}
//---------------------------------------------------------------------------
void THGem::EventReport(unsigned iDataID, unsigned iCeid)
{
    //AI(ht160s-secsgem) 20260611 : build & send S6F11 Event Report from registry.
    //  S6F11 W L[3]{ U4 DATAID, U4 CEID,
    //                L[a]{ L[2]{ U4 RPTID, L[b]{ <SV value> ... } } ... } }
    AnsiString Text;
    Text.sprintf("[SECS][TX] S6F11 EventReport DataID=%u CEID=%u", iDataID, iCeid);
    StringOut(Text);

    if(iHsmsState!=HSMS_STATE_SELECTED)
    {
        StringOut("[SECS][TX] S6F11 skipped (not selected)");
        FlushSecsLogToFile();   //AI(ht160s-obsv-p1) : event evidence crash-safe
        return;
    }
    //AI(secs-reportdef) 20260724 : per-CEID enable gate. IsEnableEvent is inert until the host
    //sends its first S2F37, so this is a no-op for today's always-on stream; after the host opts
    //in, only a CEID it explicitly disabled is suppressed (unregistered CEIDs still fire).
    if(IsEnableEvent(iDataID, iCeid)==false)
    {
        StringOut("[SECS][TX] S6F11 suppressed (CEID disabled by host)");
        FlushSecsLogToFile();
        return;
    }


    //AI(ht160s-secsgem) 20260611 : let the GEM logic snapshot live machine data
    // (run mode / lot / output / UPH / alarm) into its SV members before encode.
    if(GemLogic!=NULL)
        GemLogic->RefreshSVData();

    TGemCEIDItem *Ce = FindCEIDItem(iCeid);
    int reportCount = (Ce!=NULL) ? Ce->ReportCount : 0;

    unsigned uDataID = iDataID;
    unsigned uCeid   = iCeid;

    InitLocalHead(6, 11, 1);
    DataItemOut(3, HType.LIST_TYPE, NULL);
    DataItemOut(1, HType.UINT_4_TYPE, &uDataID);
    DataItemOut(1, HType.UINT_4_TYPE, &uCeid);
    DataItemOut(reportCount, HType.LIST_TYPE, NULL);
    for(int r=0; r<reportCount; r++)
    {
        unsigned uRpt = Ce->ReportIDs[r];
        TGemReportItem *Rp = FindReportItem(uRpt);
        int svCount = (Rp!=NULL) ? Rp->SVCount : 0;

        DataItemOut(2, HType.LIST_TYPE, NULL);
        DataItemOut(1, HType.UINT_4_TYPE, &uRpt);
        DataItemOut(svCount, HType.LIST_TYPE, NULL);
        for(int s=0; s<svCount; s++)
            DataItemOutSVValue(Rp->SVIDs[s]);
    }
    SendLocalData();
    FlushSecsLogToFile();   //AI(ht160s-obsv-p1) : event evidence crash-safe
}
//---------------------------------------------------------------------------
//AI(secs-msggap) 20260728 : S6F16 body builder (host S6F15 Event Report Request).
//  Same wire shape as the S6F11 event report -
//      L,3{ U4 DATAID, U4 CEID, L,a{ L,2{ U4 RPTID, L,b{ SV values } } } }
//  - but Func is a parameter, W=0, and the two EventReport gates are deliberately absent:
//  the HSMS_STATE_SELECTED early-out and IsEnableEvent (S6F15 is an EXPLICIT host pull, so a
//  CEID the host disabled for unsolicited reporting must still be answered - HT9045 has the
//  same enable gate commented out at uHGemClass.cpp:1973, and the KYEC wire proves it would
//  have mattered: all 3 S6F15s arrived while the host had globally disabled every event, and
//  9045 answered in full). Unknown CEID falls out naturally as L,3{ DATAID, CEID, L,0 }
//  because FindCEIDItem returns NULL and reportCount becomes 0; that keeps the top-level
//  shape constant and is E5-clean, unlike HT9045's bare zero-length U4.
//AI(secs-msggap-fix) 20260729 : dropping the SELECTED early-out is safe because the SEND is
//  gated, NOT because the receive is - ProcessReceiveBuffer hands every well-framed DATA
//  message to Dispatch with no iHsmsState test, so "you cannot receive an S6F15 unless
//  SELECTED" (the original wording here) was false. SendLocalData is the real gate.
//  Bounds: ReportCount by TGemCEIDItem (ReportIDs[32], clamped in SetCEIDContent; max seen on
//  the KYEC wire is 12) and SVCount by TGemReportItem (GEM_MAX_SVID_PER_REPORT, clamped in
//  SetReportIDContent). That cap was a bare 64 until 20260729 and it was NOT merely a safety
//  bound: KYEC defines RPTID 505 with 179 SVIDs and 800 with 103, so 8 of its 122 S2F33s were
//  rejected outright with DRACK=0x01, those reports never existed, and this very builder then
//  answered a structurally short report for CEID 1. Do not shrink it back.
//  EventReport() is NOT refactored to call this: the shipping S6F11 path keeps its own
//  gates, W=1 and FlushSecsLogToFile, and is not worth destabilising for 12 shared lines.
//---------------------------------------------------------------------------
void THGem::EmitEventReportBody(int Func, unsigned iDataID, unsigned iCeid)
{
    if(GemLogic!=NULL)
        GemLogic->RefreshSVData();

    TGemCEIDItem *Ce = FindCEIDItem(iCeid);
    int reportCount = (Ce!=NULL) ? Ce->ReportCount : 0;

    unsigned uDataID = iDataID;
    unsigned uCeid   = iCeid;

    InitLocalHead(6, Func, 0);
    DataItemOut(3, HType.LIST_TYPE, NULL);
    DataItemOut(1, HType.UINT_4_TYPE, &uDataID);
    DataItemOut(1, HType.UINT_4_TYPE, &uCeid);
    DataItemOut(reportCount, HType.LIST_TYPE, NULL);
    for(int r=0; r<reportCount; r++)
    {
        unsigned uRpt = Ce->ReportIDs[r];
        TGemReportItem *Rp = FindReportItem(uRpt);
        int svCount = (Rp!=NULL) ? Rp->SVCount : 0;

        DataItemOut(2, HType.LIST_TYPE, NULL);
        DataItemOut(1, HType.UINT_4_TYPE, &uRpt);
        DataItemOut(svCount, HType.LIST_TYPE, NULL);
        for(int s=0; s<svCount; s++)
            DataItemOutSVValue(Rp->SVIDs[s]);
    }
    SendLocalData();

    AnsiString T;
    T.sprintf("[SECS][TX] S6F%d host pull CEID=%u reports=%d", Func, iCeid, reportCount);
    StringOut(T);
    FlushSecsLogToFile();
}
//---------------------------------------------------------------------------
//AI(secs-msggap) 20260728 : S6F20 body builder (host S6F19 Individual Report Request).
//  FLAT L,n of the report's SV VALUES - no RPTID echo, no per-SV wrapper. Verified on the
//  KYEC wire: RPTID 700 -> L[21] (3/3) and 600 -> L[12] (3/3).
//AI(secs-msggap-fix) 20260729 : the RPTID-506 story was written backwards and the real
//  requirement is STRONGER than it claimed. Facts, recounted from the 20 KYEC logs:
//  506 answers L[6] at 17:52 and L[5] at 19:07, so L[6] is FIRST, not "after"; BOTH sessions
//  delete-then-redefine, so the delete is not what distinguishes them; the two SVID sets are
//  disjoint (3800-3806 vs 3616-3677), i.e. a different report, not a grown one; and 506 also
//  goes 5 -> 6 INSIDE one session via a plain overwrite with NO delete at all (S6F11 carries
//  L[5], S2F33 redefines, S6F16 then carries L[6]). So n MUST come from the live registry
//  (Rp->SVCount) on EVERY call - a cached length breaks even without a delete and even within
//  one session. THGem::SetReportIDContent find-or-creates and OVERWRITES SVCount/SVIDs in
//  place, and ProcessDefineReport_S2F33 maps an empty SV list to DeleteHostReport, so the
//  registry always holds the host's latest definition. SVCount is capped at
//  GEM_MAX_SVID_PER_REPORT by SetReportIDContent (and a larger host report is rejected earlier
//  with DRACK=0x01), so looping over Rp->SVCount cannot run past the array.
//  Unknown RPTID -> L,0 rather than HT9045's silent S9F7-and-no-reply : HT160 has no S9F7
//  sender, and silence is just another T3. Unknown SVIDs inside a known report are already
//  safe - DataItemOutSVValue emits an empty item, keeping the list length aligned (the
//  Path-A tolerance S2F33 depends on), so no SVID validation is done here.
//---------------------------------------------------------------------------
void THGem::EmitIndividualReport(unsigned ReportID)
{
    if(GemLogic!=NULL)
        GemLogic->RefreshSVData();

    TGemReportItem *Rp = FindReportItem(ReportID);
    int svCount = (Rp!=NULL) ? Rp->SVCount : 0;

    InitLocalHead(6, 20, 0);
    DataItemOut(svCount, HType.LIST_TYPE, NULL);
    for(int s=0; s<svCount; s++)
        DataItemOutSVValue(Rp->SVIDs[s]);
    SendLocalData();

    AnsiString T;
    if(Rp==NULL)
        T.sprintf("[SECS][TX] S6F20 RPTID=%u not defined -> L,0", ReportID);
    else
        T.sprintf("[SECS][TX] S6F20 RPTID=%u SVs=%d", ReportID, svCount);
    StringOut(T);
    FlushSecsLogToFile();
}
//---------------------------------------------------------------------------
void THGem::SendAlarmS5F1(unsigned alid, unsigned char alcd, AnsiString altx)
{
    //AI(ht160s-secsgem) 20260625 : build & send S5F1 Alarm Report from primitives.
    //  S5F1 W L[3]{ B[1] ALCD, U4[1] ALID, A[n] ALTX } ; only when HSMS SELECTED.
    AnsiString Text;
    Text.sprintf("[SECS][TX] S5F1 Alarm ALID=%u ALCD=%u", alid, (unsigned)alcd);
    StringOut(Text);
    if(iHsmsState!=HSMS_STATE_SELECTED)
    {
        StringOut("[SECS][TX] S5F1 skipped (not selected)");
        FlushSecsLogToFile();   //AI(ht160s-obsv-p1) : alarm evidence crash-safe (was: <=1s RAM window)
        return;
    }
    unsigned uAlid = alid;
    InitLocalHead(5, 1, 1);
    DataItemOut(3, HType.LIST_TYPE, NULL);
    DataItemOut(1, HType.BINARY_TYPE, &alcd);
    DataItemOut(1, HType.UINT_4_TYPE, &uAlid);
    DataItemOut(HType.ASCII_TYPE, altx);
    SendLocalData();
    FlushSecsLogToFile();   //AI(ht160s-obsv-p1) : alarm evidence crash-safe (was: <=1s RAM window)
}
//---------------------------------------------------------------------------
bool THGem::IsEnableEvent(unsigned iDataID, unsigned iCeid)
{
    //AI(secs-reportdef) 20260724 : provably inert until the host manages reports.
    //(A) no host S2F37 yet -> identical to legacy unconditional send.
    //(B) unregistered CEID -> fail OPEN (never silence, e.g. CEID 138).
    //(C) host is managing -> honour the per-CEID enable flag.
    if(!bHostManagesReports)
        return true;
    TGemCEIDItem *Ce = FindCEIDItem(iCeid);
    if(Ce==NULL)
        return true;
    return Ce->Enabled;
}
//---------------------------------------------------------------------------
void THGem::StringOut(AnsiString Text)
{
    if(LogList!=NULL)
    {
        LogList->Add(Text);
        //AI(ht160s-secsgem) 20260611 : bound unconsumed log so a closed monitor
        // window cannot leak memory across a long production run.
        while(LogList->Count > 5000)
            LogList->Delete(0);
    }
    //AI(ht160s-secsgem) 20260611 : queue a timestamped copy for the disk log.
    //  Flushed batch-wise by FlushSecsLogToFile() on the 1s Timer.
    if(bLogToFile && LogFileBuffer!=NULL)
    {
        Word y,mo,d,h,mi,s,ms;
        TDateTime tn = Now();
        DecodeDate(tn, y, mo, d);
        DecodeTime(tn, h, mi, s, ms);
        AnsiString Stamp;
        Stamp.sprintf("%04d/%02d/%02d %02d:%02d:%02d.%03d",
                      (int)y,(int)mo,(int)d,(int)h,(int)mi,(int)s,(int)ms);
        LogFileBuffer->Add(Stamp + "  " + Text);
        while(LogFileBuffer->Count > 5000)   // safety cap if disk write stalls
            LogFileBuffer->Delete(0);
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : drain LogList -> Dest (append), then clear.
void THGem::DrainLog(TStrings *Dest)
{
    if(LogList==NULL)
        return;
    if(Dest!=NULL && LogList->Count>0)
        Dest->AddStrings(LogList);
    LogList->Clear();
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : registry find helpers.
TGemSVItem *THGem::FindSVItem(unsigned SVID)
{
    if(SVList==NULL) return NULL;
    for(int i=0; i<SVList->Count; i++)
    {
        TGemSVItem *p = (TGemSVItem*)SVList->Items[i];
        if(p->SVID==SVID) return p;
    }
    return NULL;
}
//---------------------------------------------------------------------------
TGemECItem *THGem::FindECItem(unsigned ECID)
{
    if(ECList==NULL) return NULL;
    for(int i=0; i<ECList->Count; i++)
    {
        TGemECItem *p = (TGemECItem*)ECList->Items[i];
        if(p->ECID==ECID) return p;
    }
    return NULL;
}
//---------------------------------------------------------------------------
TGemReportItem *THGem::FindReportItem(unsigned ReportID)
{
    if(ReportList==NULL) return NULL;
    for(int i=0; i<ReportList->Count; i++)
    {
        TGemReportItem *p = (TGemReportItem*)ReportList->Items[i];
        if(p->ReportID==ReportID) return p;
    }
    return NULL;
}
//---------------------------------------------------------------------------
TGemCEIDItem *THGem::FindCEIDItem(unsigned CEID)
{
    if(CEIDList==NULL) return NULL;
    for(int i=0; i<CEIDList->Count; i++)
    {
        TGemCEIDItem *p = (TGemCEIDItem*)CEIDList->Items[i];
        if(p->CEID==CEID) return p;
    }
    return NULL;
}
//---------------------------------------------------------------------------
int THGem::GetSVCount()
{
    return (SVList!=NULL) ? SVList->Count : 0;
}
//---------------------------------------------------------------------------
unsigned THGem::GetSVIDByIndex(int Index)
{
    if(SVList==NULL || Index<0 || Index>=SVList->Count) return 0;
    return ((TGemSVItem*)SVList->Items[Index])->SVID;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : encode one SV's live value into the current msg.
//  ASCII SV -> Ptr is AnsiString*; numeric SV -> Ptr is &value.
void THGem::DataItemOutSVItem(TGemSVItem *Item)
{
    if(Item==NULL || Item->Ptr==NULL)
    {
        DataItemOut(0, HType.LIST_TYPE, NULL);   // empty item for unknown SVID
        return;
    }
    if(Item->Type==HType.ASCII_TYPE)
    {
        AnsiString *p = (AnsiString*)Item->Ptr;
        AnsiString s = *p;
        DataItemOut(HType.ASCII_TYPE, s);        // ASCII convenience overload
    }
    else
    {
        DataItemOut(Item->Len, Item->Type, Item->Ptr);
    }
}
//---------------------------------------------------------------------------
void THGem::DataItemOutSVValue(unsigned SVID)
{
    DataItemOutSVItem(FindSVItem(SVID));
}
//---------------------------------------------------------------------------
int THGem::GetECCount()
{
    return (ECList!=NULL) ? ECList->Count : 0;
}
//---------------------------------------------------------------------------
unsigned THGem::GetECIDByIndex(int Index)
{
    if(ECList==NULL || Index<0 || Index>=ECList->Count) return 0;
    return ((TGemECItem*)ECList->Items[Index])->ECID;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : encode one EC's live value into the current msg.
//  ASCII EC -> Ptr is AnsiString*; numeric EC -> Ptr is &value (read in place,
//  so tray-form ECs bound directly to THT160TrayForm members stay live).
void THGem::DataItemOutECItem(TGemECItem *Item)
{
    if(Item==NULL || Item->Ptr==NULL)
    {
        DataItemOut(0, HType.LIST_TYPE, NULL);   // empty item for unknown ECID
        return;
    }
    if(Item->Type==HType.ASCII_TYPE)
    {
        AnsiString *p = (AnsiString*)Item->Ptr;
        AnsiString s = *p;
        DataItemOut(HType.ASCII_TYPE, s);        // ASCII convenience overload
    }
    else
    {
        DataItemOut(Item->Len, Item->Type, Item->Ptr);
    }
}
//---------------------------------------------------------------------------
void THGem::DataItemOutECValue(unsigned ECID)
{
    DataItemOutECItem(FindECItem(ECID));
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : S1F12 namelist - name/unit lookup by SVID.
AnsiString THGem::GetSVName(unsigned SVID)
{
    TGemSVItem *Item = FindSVItem(SVID);
    return (Item!=NULL) ? Item->Name : AnsiString("");
}
//---------------------------------------------------------------------------
AnsiString THGem::GetSVUnit(unsigned SVID)
{
    TGemSVItem *Item = FindSVItem(SVID);
    return (Item!=NULL) ? Item->Unit : AnsiString("");
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : S2F15 EC write - store a host-supplied value
//  (carried as text) into the EC's bound storage, converting to its registered
//  SECS type. Returns 0=ok, 1=ECID not registered. Range policy is left to the
//  caller (HT160Gem only writes the idle-safe tray-form ECs).
int THGem::WriteECValueByString(unsigned ECID, AnsiString sValue)
{
    TGemECItem *Item = FindECItem(ECID);
    if(Item==NULL || Item->Ptr==NULL)
        return 1;

    if(Item->Type==HType.ASCII_TYPE)
        *((AnsiString*)Item->Ptr) = sValue;
    else if(Item->Type==HType.FT_8_TYPE)
        *((double*)Item->Ptr) = StrToFloatDef(sValue, 0.0);
    else if(Item->Type==HType.FT_4_TYPE)
        *((float*)Item->Ptr) = (float)StrToFloatDef(sValue, 0.0);
    else if(Item->Type==HType.BOOLEAN_TYPE)
        *((unsigned char*)Item->Ptr) = (StrToIntDef(sValue, 0)!=0) ? 1 : 0;
    else if(Item->Type==HType.INT_1_TYPE || Item->Type==HType.UINT_1_TYPE)
        *((unsigned char*)Item->Ptr) = (unsigned char)StrToIntDef(sValue, 0);
    else if(Item->Type==HType.INT_2_TYPE || Item->Type==HType.UINT_2_TYPE)
        *((short*)Item->Ptr) = (short)StrToIntDef(sValue, 0);
    else
        *((int*)Item->Ptr) = StrToIntDef(sValue, 0);   // INT_4 / UINT_4 default
    return 0;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : render a bound SV/EC value as display text for
//  the GUI monitor.  Mirrors the type set used by WriteECValueByString.
AnsiString THGem::ItemValueToString(unsigned char Type, void *Ptr)
{
    if(Ptr==NULL)
        return AnsiString("");
    if(Type==HType.ASCII_TYPE)
        return *((AnsiString*)Ptr);
    if(Type==HType.FT_8_TYPE)
        return FloatToStr(*((double*)Ptr));
    if(Type==HType.FT_4_TYPE)
        return FloatToStr((double)(*((float*)Ptr)));
    if(Type==HType.BOOLEAN_TYPE)
        return AnsiString((int)(*((unsigned char*)Ptr)));
    if(Type==HType.INT_1_TYPE || Type==HType.UINT_1_TYPE)
        return AnsiString((int)(*((unsigned char*)Ptr)));
    if(Type==HType.INT_2_TYPE || Type==HType.UINT_2_TYPE)
        return AnsiString((int)(*((short*)Ptr)));
    return AnsiString(*((int*)Ptr));   // INT_4 / UINT_4 default
}
//---------------------------------------------------------------------------
AnsiString THGem::GetSVValueString(unsigned SVID)
{
    TGemSVItem *Item = FindSVItem(SVID);
    if(Item==NULL) return AnsiString("");
    return ItemValueToString(Item->Type, Item->Ptr);
}
//---------------------------------------------------------------------------
AnsiString THGem::GetECName(unsigned ECID)
{
    TGemECItem *Item = FindECItem(ECID);
    return (Item!=NULL) ? Item->Name : AnsiString("");
}
//---------------------------------------------------------------------------
AnsiString THGem::GetECUnit(unsigned ECID)
{
    TGemECItem *Item = FindECItem(ECID);
    return (Item!=NULL) ? Item->Unit : AnsiString("");
}
//---------------------------------------------------------------------------
AnsiString THGem::GetECValueString(unsigned ECID)
{
    TGemECItem *Item = FindECItem(ECID);
    if(Item==NULL) return AnsiString("");
    return ItemValueToString(Item->Type, Item->Ptr);
}
//---------------------------------------------------------------------------
//AI(secs-namelist) 20260730 : S2F30 EC namelist read-back. The limits are the TEXT
//  supplied at SetECDataPointer time; "" means the EC declares no such limit.
AnsiString THGem::GetECMinValue(unsigned ECID)
{
    TGemECItem *Item = FindECItem(ECID);
    return (Item!=NULL) ? Item->MinValue : AnsiString("");
}
//---------------------------------------------------------------------------
AnsiString THGem::GetECMaxValue(unsigned ECID)
{
    TGemECItem *Item = FindECItem(ECID);
    return (Item!=NULL) ? Item->MaxValue : AnsiString("");
}
//---------------------------------------------------------------------------
AnsiString THGem::GetECDefaultValue(unsigned ECID)
{
    TGemECItem *Item = FindECItem(ECID);
    return (Item!=NULL) ? Item->DefaultValue : AnsiString("");
}
//---------------------------------------------------------------------------
//AI(secs-namelist) 20260730 : declared SECS type of a registered EC. An unknown ECID
//  answers ASCII so the S2F30 unknown-ECID row ships zero-length ASCII limits, which
//  is the shape HT9045 emits for an EC it cannot resolve.
unsigned char THGem::GetECType(unsigned ECID)
{
    TGemECItem *Item = FindECItem(ECID);
    return (Item!=NULL) ? Item->Type : HType.ASCII_TYPE;
}
//---------------------------------------------------------------------------
bool THGem::IsValidECID(unsigned ECID)
{
    return (FindECItem(ECID)!=NULL);
}
//---------------------------------------------------------------------------
//AI(secs-namelist) 20260730 : encode ONE scalar of the given type from its text form.
//  Type coverage mirrors WriteECValueByString / ItemValueToString so an EC round-trips
//  through S2F30 (read limits) and S2F15 (write value) with the same type mapping.
//  Empty text -> zero-length item of that type: E5 "this EC declares no limit", and the
//  same thing HT9045 sends when its Min/Max/Default pointer is NULL.
void THGem::DataItemOutTypedText(unsigned char Type, AnsiString sValue)
{
    if(sValue.Trim()=="")
    {
        DataItemOut(0, Type, NULL);
        return;
    }
    if(Type==HType.ASCII_TYPE || Type==HType.JIS_TYPE)
    {
        DataItemOut(Type, sValue);
        return;
    }
    if(Type==HType.FT_8_TYPE)
    {
        double d = StrToFloatDef(sValue, 0.0);
        DataItemOut(1, Type, &d);
        return;
    }
    if(Type==HType.FT_4_TYPE)
    {
        float f = (float)StrToFloatDef(sValue, 0.0);
        DataItemOut(1, Type, &f);
        return;
    }
    if(Type==HType.BOOLEAN_TYPE)
    {
        unsigned char b = (StrToIntDef(sValue, 0)!=0) ? 1 : 0;
        DataItemOut(1, Type, &b);
        return;
    }
    if(Type==HType.BINARY_TYPE || Type==HType.INT_1_TYPE || Type==HType.UINT_1_TYPE)
    {
        unsigned char c = (unsigned char)StrToIntDef(sValue, 0);
        DataItemOut(1, Type, &c);
        return;
    }
    if(Type==HType.INT_2_TYPE || Type==HType.UINT_2_TYPE)
    {
        short s = (short)StrToIntDef(sValue, 0);
        DataItemOut(1, Type, &s);
        return;
    }
    int i = StrToIntDef(sValue, 0);
    DataItemOut(1, Type, &i);        // INT_4 / UINT_4 default (matches WriteECValueByString)
}
//---------------------------------------------------------------------------
//AI(secs-namelist) 20260730 : S1F24 CEID namelist read-back.
int THGem::GetCEIDCount()
{
    return (CEIDList!=NULL) ? CEIDList->Count : 0;
}
//---------------------------------------------------------------------------
unsigned THGem::GetCEIDByIndex(int Index)
{
    if(CEIDList==NULL || Index<0 || Index>=CEIDList->Count) return 0;
    return ((TGemCEIDItem*)CEIDList->Items[Index])->CEID;
}
//---------------------------------------------------------------------------
AnsiString THGem::GetCEIDAlias(unsigned CEID)
{
    TGemCEIDItem *Item = FindCEIDItem(CEID);
    return (Item!=NULL) ? Item->Alias : AnsiString("");
}
//---------------------------------------------------------------------------
bool THGem::IsValidCEID(unsigned CEID)
{
    return (FindCEIDItem(CEID)!=NULL);
}
//---------------------------------------------------------------------------
//AI(secs-namelist) 20260730 : flatten CEID -> linked reports -> SVIDs for S1F24.
//  Order follows the CEID's ReportIDs[] then each report's SVIDs[], i.e. exactly the
//  order those SVs appear in the S6F11 body for this event, so a host can diff the
//  namelist against a received report positionally.
//  Duplicates are NOT removed: HT9045 does not remove them either (it concatenates
//  every linked report's variable column), and a CEID linked to two reports that share
//  an SVID really does ship that SVID twice in S6F11.
//  Returns the number written. A full buffer is a TRUNCATION - the caller must say so
//  rather than silently shipping a short list.
int THGem::GetCEIDVidList(unsigned CEID, unsigned *Out, int MaxOut)
{
    if(Out==NULL || MaxOut<=0)
        return 0;
    TGemCEIDItem *Ce = FindCEIDItem(CEID);
    if(Ce==NULL)
        return 0;

    int n = 0;
    for(int r=0; r<Ce->ReportCount && n<MaxOut; r++)
    {
        TGemReportItem *Rp = FindReportItem(Ce->ReportIDs[r]);
        if(Rp==NULL)
            continue;
        for(int s=0; s<Rp->SVCount && n<MaxOut; s++)
            Out[n++] = Rp->SVIDs[s];
    }
    return n;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : trigger a live SV snapshot (same path the host
//  S1F4/S6F11 uses) so the GUI shows current machine values.
void THGem::RefreshSVSnapshot()
{
    if(GemLogic!=NULL)
        GemLogic->RefreshSVData();
}
//---------------------------------------------------------------------------
AnsiString THGem::GetEndpointAddress() { return sDefaultAddress; }
AnsiString THGem::GetEndpointPort()    { return sDefaultPort; }
AnsiString THGem::GetDeviceIdText()    { return sDeviceId; }
bool       THGem::IsActiveMode()       { return bActiveMode; }
//---------------------------------------------------------------------------
void THGem::SetSVDataPointer(unsigned SVID, unsigned char Type, AnsiString Name, AnsiString Unit, void *DataPtr, AnsiString Description)
{
    if(SVList==NULL) SVList = new TList;
    if(FindSVItem(SVID)!=NULL)
    {
        StringOut("[SECS] SVID duplicate ignored: " + AnsiString((int)SVID));
        return;
    }
    TGemSVItem *p = new TGemSVItem;
    p->SVID   = SVID;
    p->Type   = Type;
    p->Len    = 1;
    p->Ptr    = DataPtr;
    p->Name   = Name;
    p->Unit   = Unit;
    p->Remark = Description;
    SVList->Add(p);
}
//---------------------------------------------------------------------------
void THGem::SetECDataPointer(unsigned ECID, unsigned char Type, AnsiString Name, AnsiString Unit, void *DataPtr, AnsiString MinValue, AnsiString MaxValue, AnsiString DefaultValue, AnsiString Description)
{
    if(ECList==NULL) ECList = new TList;
    if(FindECItem(ECID)!=NULL)
    {
        StringOut("[SECS] ECID duplicate ignored: " + AnsiString((int)ECID));
        return;
    }
    TGemECItem *p = new TGemECItem;
    p->ECID         = ECID;
    p->Type         = Type;
    p->Len          = 1;
    p->Ptr          = DataPtr;
    p->Name         = Name;
    p->Unit         = Unit;
    p->MinValue     = MinValue;
    p->MaxValue     = MaxValue;
    p->DefaultValue = DefaultValue;
    p->Remark       = Description;
    ECList->Add(p);
}
//---------------------------------------------------------------------------
void THGem::SetAlamData(int Index, AnsiString AlarmCode, AnsiString UnitName, AnsiString Message, AnsiString AlarmType)
{
}
//---------------------------------------------------------------------------
void THGem::ReadAlamData()
{
}
//---------------------------------------------------------------------------
void THGem::WriteAlamData()
{
}
//---------------------------------------------------------------------------
void THGem::ReadEventReportData()
{
    //AI(secs-reportdef) 20260724 : overlay HOST-defined reports onto the firmware baseline (already
    //registered by AddReprot). Robust: missing/corrupt/bad-version -> keep firmware defaults, never
    //throw, never block boot. Links + enable are NOT restored (session-only by design). Sets
    //bEventDefLoaded so Save is armed only after this overlay has loaded.
    bEventDefLoaded = true;
    AnsiString Fn = HSys.CurrentDir + "\\system\\EventReportDef.ini";
    if(!FileExists(Fn))
        return;
    TStringList *sl = new TStringList;
    try
    {
        sl->LoadFromFile(Fn);
        if(sl->Count>=1 && sl->Strings[0].Trim()=="FormatVersion=1")
        {
            for(int i=1; i<sl->Count; i++)
            {
                AnsiString line = sl->Strings[i].Trim();
                if(line.SubString(1,7)!="Report=") continue;
                AnsiString body = line.SubString(8, line.Length()-7);
                int bar = body.Pos("|");
                if(bar<=0) continue;
                unsigned rid = (unsigned)atoi(body.SubString(1, bar-1).c_str());
                if(rid==0) continue;
                TGemReportItem *ex = FindReportItem(rid);
                if(ex!=NULL && ex->Mode==1) continue;
                AnsiString rest = body.SubString(bar+1, body.Length()-bar);
                unsigned sv[GEM_MAX_SVID_PER_REPORT];
                int n = 0;
                while(rest.Trim()!="" && n<GEM_MAX_SVID_PER_REPORT)
                {
                    int comma = rest.Pos(",");
                    AnsiString tok;
                    if(comma>0) { tok = rest.SubString(1, comma-1); rest = rest.SubString(comma+1, rest.Length()-comma); }
                    else        { tok = rest; rest = ""; }
                    unsigned s = (unsigned)atoi(tok.Trim().c_str());
                    //AI(secs-audit-fix) 20260729 : keep every SVID the host defined, byte-identically
                    //to ProcessDefineReport_S2F33's Path-A tolerance - INCLUDING 0, which that path
                    //also accepts and SaveEventReportData also writes back. Dropping unknown SVIDs
                    //here made the round trip lossy : a host report of only firmware-unknown SVIDs
                    //(e.g. the on-site RPTID 504={20001,20002,20003}) was accepted with DRACK=0x00,
                    //written to EventReportDef.ini, then silently VANISHED on the next boot because
                    //n stayed 0 and the "if(n>0)" below skipped the whole report. A PARTIAL drop was
                    //worse still - it shortened the report's LIST so S6F11 no longer matched what the
                    //host defined and every later value shifted one slot. Any filter here, however
                    //narrow, reintroduces exactly that shift, so there is none. DataItemOutSVItem
                    //emits an empty item for an SVID the firmware does not know, so the LIST length
                    //always matches the stored SVCount.
                    //AI(secs-strict-reportdef) 20260810 : the no-filter rule above still holds - see
                    //the whole-report drop after this loop, which is a different thing from filtering
                    //SVIDs out of a report. Nothing here changed.
                    sv[n++] = s;
                }
                //AI(secs-strict-reportdef) 20260810 : upgrade path. This overlay does NOT go through
                //ProcessDefineReport_S2F33, so without this a machine that ran tolerant would silently
                //REBUILD, on every boot, the very reports strict mode now refuses - EventReportDef.ini
                //still holds whatever the host defined back then (on-site 2026-08-07: 7 reports, 54
                //undefined SVIDs). Drop the WHOLE report, exactly as a live S2F33 would now; never a
                //subset, which is what the 20260729 note above rightly forbids (it shifts S6F11 slots).
                if(bStrictReportValidation)
                {
                    for(int k=0; k<n; k++)
                    {
                        if(IsValidSVID(sv[k])==false)
                        {
                            AnsiString sDrop;
                            sDrop.sprintf("[SECS][boot] drop persisted RPTID=%u - SVID=%u undefined (strict; a live S2F33 would DRACK=0x04)",
                                          rid, sv[k]);
                            StringOut(sDrop);
                            n = 0;
                            break;
                        }
                    }
                }
                if(n>0) SetReportIDContent(rid, (unsigned)n, sv, 0);
            }
        }
    }
    catch(...) {}
    delete sl;
}
//---------------------------------------------------------------------------
//AI(secs-reportdef) 20260724 : S2F34/F36/F38 binary-ack senders (single B code; reuses request SystemByte).
void THGem::ReportAcknowledge(unsigned char DRACK)
{
    InitLocalHead(2, 34, 0);
    DataItemOut(1, HType.BINARY_TYPE, &DRACK);
    SendLocalData();
}
//---------------------------------------------------------------------------
void THGem::LinkReportAcknowledge(unsigned char LRACK)
{
    InitLocalHead(2, 36, 0);
    DataItemOut(1, HType.BINARY_TYPE, &LRACK);
    SendLocalData();
}
//---------------------------------------------------------------------------
void THGem::EnableDisableEventReportAcknowledge(unsigned char ERACK)
{
    InitLocalHead(2, 38, 0);
    DataItemOut(1, HType.BINARY_TYPE, &ERACK);
    SendLocalData();
}
//---------------------------------------------------------------------------
bool THGem::IsValidSVID(unsigned SVID)
{
    return (FindSVItem(SVID)!=NULL);
}
//---------------------------------------------------------------------------
//AI(secs-reportdef) 20260724 : remove ReportID from every CEID's link list (compact in place).
void THGem::UnlinkReportFromAllCeids(unsigned ReportID)
{
    if(CEIDList==NULL) return;
    for(int i=0; i<CEIDList->Count; i++)
    {
        TGemCEIDItem *Ce = (TGemCEIDItem*)CEIDList->Items[i];
        int w = 0;
        for(int r=0; r<Ce->ReportCount; r++)
            if(Ce->ReportIDs[r]!=ReportID)
                Ce->ReportIDs[w++] = Ce->ReportIDs[r];
        Ce->ReportCount = w;
    }
}
//---------------------------------------------------------------------------
//AI(secs-reportdef) 20260724 : delete a HOST report (Mode==0) and unlink it. Firmware reports are
//protected upstream (DRACK 0x03) and never reach here.
void THGem::DeleteHostReport(unsigned ReportID)
{
    if(ReportList==NULL) return;
    for(int i=0; i<ReportList->Count; i++)
    {
        TGemReportItem *Rp = (TGemReportItem*)ReportList->Items[i];
        if(Rp->ReportID==ReportID && Rp->Mode==0)
        {
            UnlinkReportFromAllCeids(ReportID);
            delete Rp;
            ReportList->Delete(i);
            return;
        }
    }
}
//---------------------------------------------------------------------------
void THGem::DeleteAllHostReports()
{
    if(ReportList==NULL) return;
    for(int i=ReportList->Count-1; i>=0; i--)
    {
        TGemReportItem *Rp = (TGemReportItem*)ReportList->Items[i];
        if(Rp->Mode==0)
        {
            UnlinkReportFromAllCeids(Rp->ReportID);
            delete Rp;
            ReportList->Delete(i);
        }
    }
}
//---------------------------------------------------------------------------
//AI(secs-reportdef) 20260724 : S2F33 Define Report. L,2{ DATAID, L,a{ L,2{ RPTID, L,b{ SVID.. } } } }.
//Parse into scratch, validate ALL, then commit atomically. DRACK 0=ok 1=space 2=fmt 3=firmware-dup 4=badSVID.
//AI(secs-strict-reportdef) 20260810 : DRACK=0x04 is LIVE again - this comment used to be stale, it
//documented a 0x04 that had no emit site. The 20260727 Path A tolerance (accept SVIDs we do not
//define, log them, let S6F11 emit an empty item) is now OFF by default, on customer instruction:
//HT160S hardware genuinely differs from HT9045, so an SVID we do not have must be REFUSED, not
//silently accepted into a report that can never carry a value. Evidence that tolerance hid a real
//failure: 2026-08-07 on-site, KYEC's host defined 7 reports carrying 54 SVIDs, EVERY ONE undefined
//here, and got DRACK=0x00 on all of them - the host had no way to know the data would never come.
//Matches HT9045, which validates the same way (uHGemEquipment.cpp:7937-7940 ReportAcknowledge(0x04),
//scan-all-then-commit loop uHGemClass.cpp:1300-1312 committing only at :1314).
//Rejection is ATOMIC and DEFERRED: the bad SVID is recorded, the rest of the message is still parsed
//to completion (so the receive path stays in step), and the reject is answered after the parse loop
//but BEFORE the commit loop - so not one report is created. General.ini [SECS] StrictReportValidation=0
//restores the old tolerance as an on-site escape hatch without a rebuild.
void THGem::ProcessDefineReport_S2F33()
{
    int a=0, b=0, len=0, i=0, j=0;
    unsigned char Type=0;
    AnsiString sTmp;
    static unsigned rid[64];
    static int      svn[64];
    static unsigned svv[64][GEM_MAX_SVID_PER_REPORT];
    int nRpt = 0;
    bool     bBadSvid = false;   //AI(secs-strict-reportdef) 20260810 : deferred atomic DRACK=0x04
    unsigned uBadSvid = 0;       // first offender, for the reject log line
    int      iBadSvidCount = 0;  // how many undefined SVIDs the whole packet named
    ResetReturnCode();
    if(DataItemIn(2, HType.LIST_TYPE, NULL)!=1)              { ReportAcknowledge(0x02); return; }
    if(GetDataItemLenAndType(len, Type)!=1)                 { ReportAcknowledge(0x02); return; }
    if(DataItemIn(len, Type, sTmp)!=1)                      { ReportAcknowledge(0x02); return; }
    if(GetDataItemLenAndTypeAndDelete(a, HType.LIST_TYPE)!=1){ ReportAcknowledge(0x02); return; }
    if(a==0)  { DeleteAllHostReports(); ReportAcknowledge(0x00); SaveEventReportData(); return; }
    if(a>64)  { ReportAcknowledge(0x01); return; }
    for(i=0; i<a; i++)
    {
        if(DataItemIn(2, HType.LIST_TYPE, NULL)!=1)         { ReportAcknowledge(0x02); return; }
        if(GetDataItemLenAndType(len, Type)!=1)             { ReportAcknowledge(0x02); return; }
        if(DataItemIn(len, Type, sTmp)!=1)                  { ReportAcknowledge(0x02); return; }
        //AI(secs-idparse-unify) 20260805 : strtoul, not atoi. RPTID / SVID / CEID are U4 on the
        //  wire; atoi returns a signed int and wraps anything >= 2^31 to a negative value, which the
        //  (unsigned) cast then turns into a completely different id. Same reason S6F16 / S6F20
        //  already use strtoul (see their inline notes). KYEC's numbers top out at 38245 so nothing
        //  is fixed today - this is the consistency half of the pair.
        unsigned rr = (unsigned)strtoul(sTmp.c_str(), NULL, 10);
        TGemReportItem *ex = FindReportItem(rr);
        if(ex!=NULL && ex->Mode==1)                         { ReportAcknowledge(0x03); return; }
        if(GetDataItemLenAndTypeAndDelete(b, HType.LIST_TYPE)!=1){ ReportAcknowledge(0x02); return; }
        if(b>GEM_MAX_SVID_PER_REPORT)                       { ReportAcknowledge(0x01); return; }
        rid[nRpt]=rr; svn[nRpt]=b;
        for(j=0; j<b; j++)
        {
            if(GetDataItemLenAndType(len, Type)!=1)         { ReportAcknowledge(0x02); return; }
            if(DataItemIn(len, Type, sTmp)!=1)              { ReportAcknowledge(0x02); return; }
            unsigned sv = (unsigned)strtoul(sTmp.c_str(), NULL, 10);   //AI(secs-idparse-unify) 20260805 : U4 id, see S2F33 RPTID above
            //AI(secs-pathA) 20260727 : Path A tolerance - accept host-referenced SVIDs the
            //firmware does not define (was DRACK=0x04 hard reject, which blocked the on-site
            //CJ_EAP report def, e.g. RPTID 504={20001,20002,20003}). S6F11 emits an empty item
            //for an unknown SVID (DataItemOutSVItem NULL branch @ this file), so the report LIST
            //length stays aligned. Log the unknown SVID for referenced-set discovery.
            if(IsValidSVID(sv)==false)
            {
                AnsiString sUnkSv;
                if(bStrictReportValidation)
                {
                    if(!bBadSvid) { bBadSvid=true; uBadSvid=sv; }
                    iBadSvidCount++;
                    sUnkSv.sprintf("[SECS][S2F33] REJECT unknown SVID=%u in RPTID=%u (strict; whole S2F33 -> DRACK=0x04)", sv, rr);
                }
                else
                {
                    sUnkSv.sprintf("[SECS][S2F33] accept unknown SVID=%u (Path A tolerate; reports empty)", sv);
                }
                StringOut(sUnkSv);
            }
            svv[nRpt][j]=sv;
        }
        nRpt++;
        //AI(secs-boundary-fix) 20260727 : removed spurious post-increment guard - rid[64] fill is already bounded by the a>64 reject above; the old nRpt>=64 check wrongly DRACK=0x01-rejected a legitimate 64-report batch and skipped the commit.
    }
    //AI(secs-strict-reportdef) 20260810 : atomic strict reject. Answered AFTER the whole message has
    //been parsed (receive path stays in step) but BEFORE the commit loop below, so a packet naming
    //even one undefined SVID creates NO reports at all - same all-or-nothing shape as HT9045.
    if(bBadSvid)
    {
        AnsiString sRej;
        sRej.sprintf("[SECS][S2F33] DRACK=0x04 - %d undefined SVID(s) in %d report(s), first=%u; nothing committed",
                     iBadSvidCount, nRpt, uBadSvid);
        StringOut(sRej);
        ReportAcknowledge(0x04);
        return;
    }
    for(i=0; i<nRpt; i++)
    {
        if(svn[i]==0) DeleteHostReport(rid[i]);
        else          SetReportIDContent(rid[i], (unsigned)svn[i], svv[i], 0);
    }
    ReportAcknowledge(0x00);
    SaveEventReportData();
}
//---------------------------------------------------------------------------
//AI(secs-reportdef) 20260724 : S2F35 Link Event Report. L,2{ DATAID, L,a{ L,2{ CEID, L,b{ RPTID.. } } } }.
//LRACK 0=ok 1=space 2=fmt 4=CEID-not-exist 5=RPTID-not-exist.
//AI(secs-strict-reportdef) 20260810 : the 20260727 Path A tolerance (unknown CEID host-created and
//linked, unknown RPTID silently skipped) is now OFF by default - same customer instruction and same
//atomic/deferred shape as the S2F33 sibling above, and the same HT9045 precedent
//(uHGemEquipment.cpp:7786-7813 -> LRACK 0x04 unknown CEID / 0x05 unknown RPTID).
//Tolerance mattered here too: silently skipping an unknown RPTID let the host believe a link existed
//that would never fire. General.ini [SECS] StrictReportValidation=0 restores the old behaviour.
//CEID error outranks RPTID error when a packet has both - 0x04 names the outer object.
void THGem::ProcessLinkEventReport_S2F35()
{
    int a=0, b=0, len=0, i=0, j=0;
    unsigned char Type=0;
    AnsiString sTmp;
    static unsigned cid[64];
    static int      rpn[64];
    static int      rpb[64];
    static unsigned rpv[64][32];
    int nCe = 0;
    bool     bBadCeid = false;   //AI(secs-strict-reportdef) 20260810 : deferred atomic LRACK=0x04
    unsigned uBadCeid = 0;
    bool     bBadRptid = false;  //AI(secs-strict-reportdef) 20260810 : deferred atomic LRACK=0x05
    unsigned uBadRptid = 0;
    ResetReturnCode();
    if(DataItemIn(2, HType.LIST_TYPE, NULL)!=1)              { LinkReportAcknowledge(0x02); return; }
    if(GetDataItemLenAndType(len, Type)!=1)                 { LinkReportAcknowledge(0x02); return; }
    if(DataItemIn(len, Type, sTmp)!=1)                      { LinkReportAcknowledge(0x02); return; }
    if(GetDataItemLenAndTypeAndDelete(a, HType.LIST_TYPE)!=1){ LinkReportAcknowledge(0x02); return; }
    if(a==0)  { LinkReportAcknowledge(0x00); return; }
    //AI(secs-e5-lrack) 20260805 : capacity overflow is LRACK=0x01 "insufficient space", not 0x02
    //"invalid format" - E5 separates the two, and the sibling ProcessDefineReport_S2F33 in this same
    //file already answers DRACK=0x01 for the identical condition. HT9045 has NO capacity check on
    //S2F35 at all, so there is no 9045 precedent either way; 0x01 is the E5 code and matches our own
    //S2F33, which is the closest thing to a house rule we have.
    if(a>64)  { LinkReportAcknowledge(0x01); return; }
    for(i=0; i<a; i++)
    {
        if(DataItemIn(2, HType.LIST_TYPE, NULL)!=1)         { LinkReportAcknowledge(0x02); return; }
        if(GetDataItemLenAndType(len, Type)!=1)             { LinkReportAcknowledge(0x02); return; }
        if(DataItemIn(len, Type, sTmp)!=1)                  { LinkReportAcknowledge(0x02); return; }
        unsigned cc = (unsigned)strtoul(sTmp.c_str(), NULL, 10);   //AI(secs-idparse-unify) 20260805 : U4 id, see S2F33 RPTID above
        //AI(secs-pathA) 20260727 : Path A discovery-probe tolerance. Do NOT LRACK=0x04-reject a link
        //to a CEID the firmware lacks - the on-site CJ_EAP host is numbered for the HT9045 CEID
        //dictionary, and a hard reject blocks its link phase (and our capture of the referenced set).
        //SetCEIDContent find-or-creates (Mode=0 host) so an unknown CEID is created + linked; it has
        //no fire site in HT160 so it simply never fires. Log it for referenced-set discovery.
        if(FindCEIDItem(cc)==NULL)
        {
            AnsiString sUnkCe;
            if(bStrictReportValidation)
            {
                if(!bBadCeid) { bBadCeid=true; uBadCeid=cc; }
                sUnkCe.sprintf("[SECS][S2F35] REJECT unknown CEID=%u (strict; whole S2F35 -> LRACK=0x04)", cc);
            }
            else
            {
                sUnkCe.sprintf("[SECS][S2F35] link to unknown CEID=%u (Path A tolerate; host-created, never fires)", cc);
            }
            StringOut(sUnkCe);
        }
        if(GetDataItemLenAndTypeAndDelete(b, HType.LIST_TYPE)!=1){ LinkReportAcknowledge(0x02); return; }
        //AI(secs-e5-lrack) 20260805 : per-CEID RPTID overflow is "insufficient space" (0x01) too.
        if(b>32)                                            { LinkReportAcknowledge(0x01); return; }
        cid[nCe]=cc;
        int w=0;
        for(j=0; j<b; j++)
        {
            if(GetDataItemLenAndType(len, Type)!=1)         { LinkReportAcknowledge(0x02); return; }
            if(DataItemIn(len, Type, sTmp)!=1)              { LinkReportAcknowledge(0x02); return; }
            unsigned rr = (unsigned)strtoul(sTmp.c_str(), NULL, 10);   //AI(secs-idparse-unify) 20260805 : U4 id, see S2F33 RPTID above
            //AI(secs-pathA) 20260727 : skip (do not store) a link to an undefined RPTID rather than
            //LRACK=0x05-rejecting, so no dangling ref reaches S6F11 serialize. With S2F33 tolerance
            //the host RPTIDs normally exist; this is a safety net.
            if(FindReportItem(rr)==NULL)
            {
                AnsiString sUnkRp;
                if(bStrictReportValidation)
                {
                    if(!bBadRptid) { bBadRptid=true; uBadRptid=rr; }
                    sUnkRp.sprintf("[SECS][S2F35] REJECT unknown RPTID=%u in CEID=%u link (strict; whole S2F35 -> LRACK=0x05)", rr, cc);
                }
                else
                {
                    sUnkRp.sprintf("[SECS][S2F35] skip unknown RPTID=%u in CEID=%u link (Path A tolerate)", rr, cc);
                }
                StringOut(sUnkRp);
                continue;
            }
            rpv[nCe][w]=rr; w++;
        }
        rpn[nCe]=w; rpb[nCe]=b;
        nCe++;
        //AI(secs-boundary-fix) 20260727 : removed spurious post-increment guard - cid[64] fill is already bounded by the a>64 reject above; the old nCe>=64 check wrongly LRACK=0x02-rejected a legitimate 64-CEID batch and skipped the commit.
    }
    //AI(secs-strict-reportdef) 20260810 : atomic strict reject, after the full parse and before any
    //commit, so a packet with a bad id links nothing. 0x04 (CEID) is reported ahead of 0x05 (RPTID)
    //because the CEID is the outer object the host got wrong.
    if(bBadCeid)
    {
        AnsiString sRej;
        sRej.sprintf("[SECS][S2F35] LRACK=0x04 - CEID=%u not defined; nothing linked", uBadCeid);
        StringOut(sRej);
        LinkReportAcknowledge(0x04);
        return;
    }
    if(bBadRptid)
    {
        AnsiString sRej;
        sRej.sprintf("[SECS][S2F35] LRACK=0x05 - RPTID=%u not defined; nothing linked", uBadRptid);
        StringOut(sRej);
        LinkReportAcknowledge(0x05);
        return;
    }
    for(i=0; i<nCe; i++)
    {
        //AI(secs-pathA) 20260727 : adversarial-review MEDIUM fix - do NOT commit a link
        //whose reports were ALL unknown-skipped (rpn==0 while the host DID request rpb>0).
        //That would overwrite an existing firmware CEID binding to L,0 and silently drop
        //its S6F11 payload for the session. Preserve the binding; a genuine host unlink
        //(b==0 -> rpb==0) still commits ReportCount=0 as before.
        if(rpn[i]==0 && rpb[i]>0)
        {
            AnsiString sKeep;
            sKeep.sprintf("[SECS][S2F35] CEID=%u link had only unknown RPTIDs - preserve existing binding, no commit", cid[i]);
            StringOut(sKeep);
            continue;
        }
        SetCEIDContent(cid[i], (unsigned)rpn[i], rpv[i], 0);
    }
    LinkReportAcknowledge(0x00);
}
//---------------------------------------------------------------------------
//AI(secs-reportdef) 20260724 : S2F37 Enable/Disable Event. L,2{ CEED(BOOL), L,n{ CEID.. } }. n==0 = all.
//ERACK 0=ok 1=CEID-not-exist 2=fmt. Success is the ONLY point that arms bHostManagesReports.
void THGem::ProcessEnableDisableEventReport_S2F37()
{
    int n=0, len=0, i=0;
    unsigned char Type=0, rawb=0;
    bool bCeed=false;
    AnsiString sTmp;
    static unsigned cid[GEM_MAX_CEID_PER_S2F37];
    int nCe = 0;
    ResetReturnCode();
    if(DataItemIn(2, HType.LIST_TYPE, NULL)!=1)             { EnableDisableEventReportAcknowledge(0x02); return; }
    if(GetDataItemLenAndType(len, Type)!=1)                { EnableDisableEventReportAcknowledge(0x02); return; }
    if(Type==HType.BOOLEAN_TYPE)
    {
        if(DataItemIn(1, HType.BOOLEAN_TYPE, &bCeed)!=1)   { EnableDisableEventReportAcknowledge(0x02); return; }
    }
    else if(Type==HType.BINARY_TYPE || Type==HType.UINT_1_TYPE)
    {
        if(DataItemIn(1, Type, &rawb)!=1)                  { EnableDisableEventReportAcknowledge(0x02); return; }
        bCeed = (rawb!=0);
    }
    else
    {
        if(DataItemIn(len, Type, sTmp)!=1)                 { EnableDisableEventReportAcknowledge(0x02); return; }
        bCeed = (atoi(sTmp.c_str())!=0);
    }
    if(GetDataItemLenAndTypeAndDelete(n, HType.LIST_TYPE)!=1){ EnableDisableEventReportAcknowledge(0x02); return; }
    //AI(secs-e5-lrack) 20260805 : cap raised 256 -> GEM_MAX_CEID_PER_S2F37 (512) so a host that
    //enumerates the whole 292-id dictionary after an S1F23 full query is no longer rejected wholesale.
    //This test MUST stay ahead of the fill loop below - that is what bounds cid[].
    if(n>GEM_MAX_CEID_PER_S2F37) { EnableDisableEventReportAcknowledge(0x02); return; }
    for(i=0; i<n; i++)
    {
        if(GetDataItemLenAndType(len, Type)!=1)            { EnableDisableEventReportAcknowledge(0x02); return; }
        if(DataItemIn(len, Type, sTmp)!=1)                 { EnableDisableEventReportAcknowledge(0x02); return; }
        unsigned cc = (unsigned)strtoul(sTmp.c_str(), NULL, 10);   //AI(secs-idparse-unify) 20260805 : U4 id, see S2F33 RPTID above
        if(FindCEIDItem(cc)==NULL)                         { EnableDisableEventReportAcknowledge(0x01); return; }
        cid[nCe++]=cc;
    }
    if(n==0)
    {
        if(CEIDList!=NULL)
            for(i=0; i<CEIDList->Count; i++)
                ((TGemCEIDItem*)CEIDList->Items[i])->Enabled = bCeed;
    }
    else
    {
        for(i=0; i<nCe; i++)
        {
            TGemCEIDItem *Ce = FindCEIDItem(cid[i]);
            if(Ce!=NULL) Ce->Enabled = bCeed;
        }
    }
    bHostManagesReports = true;
    EnableDisableEventReportAcknowledge(0x00);
    SaveEventReportData();
    //AI(secs-s2f37-visibility) 20260808 : say on the wire log what the host just did to reporting.
    //On 2026-08-07 the KYEC host opened all 8 sessions with this exact packet - CEED=FALSE with an
    //empty CEID list, i.e. disable EVERY event - and never once sent CEED=TRUE, so the tool dropped
    //508 S6F11 reports that shift (567 counting the pre-Select ones) and delivered no event data at
    //all. The per-event breadcrumb at SendEventReport only says "suppressed" one report at a time,
    //which is unreadable at 508 lines; this is the one line that states the resulting policy.
    //The operator-facing EventLog line is raised one layer up, in
    //HT160Gem::S2F38_EnableDisableEventReportAcknowledge, because reaching machine-layer code from
    //here would cross the GemLogic boundary this file documents in its destructor.
    {
        AnsiString sEff;
        sEff.sprintf("[SECS][S2F37] CEED=%d n=%d -> %d of %d CEIDs enabled",
                     bCeed?1:0, n, GetEnabledCeidCount(), GetCeidCount());
        StringOut(sEff);
    }
}
//---------------------------------------------------------------------------
//AI(secs-s2f37-visibility) 20260808 : reporting-state read-back (see the header note).
int THGem::GetCeidCount()
{
    return (CEIDList!=NULL) ? CEIDList->Count : 0;
}
//---------------------------------------------------------------------------
int THGem::GetEnabledCeidCount()
{
    int iEn = 0;

    if(CEIDList==NULL)
        return 0;
    for(int i=0; i<CEIDList->Count; i++)
    {
        TGemCEIDItem *Ce = (TGemCEIDItem*)CEIDList->Items[i];
        if(Ce!=NULL && Ce->Enabled)
            iEn++;
    }
    return iEn;
}
//---------------------------------------------------------------------------
bool THGem::IsHostManagingReports()
{
    return bHostManagesReports;
}
//---------------------------------------------------------------------------
void THGem::SetHostManagesReports(bool bOn)
{
    bHostManagesReports = bOn;
}
//---------------------------------------------------------------------------
void THGem::SetCEIDContent(unsigned iCeid, unsigned iReportCount, unsigned *iReportIDData, int Mode)
{
    SetCEIDContent(iCeid, "", iReportCount, iReportIDData, Mode);
}
//---------------------------------------------------------------------------
void THGem::SetCEIDContent(unsigned iCeid, AnsiString CeidAlias, unsigned iReportCount, unsigned *iReportIDData, int Mode)
{
    if(CEIDList==NULL) CEIDList = new TList;
    TGemCEIDItem *p = FindCEIDItem(iCeid);
    if(p==NULL)
    {
        p = new TGemCEIDItem;
        p->CEID = iCeid;
        p->Enabled = true;   //AI(secs-reportdef) 20260724 : default enabled (preserve today's always-on)
        p->Mode = Mode;      //AI(secs-reportdef) 20260724 : 0=host, 1=firmware
        CEIDList->Add(p);
    }
    //AI(secs-ceid-alias-keep) 20260805 : only ADOPT a non-empty alias. The 4-arg overload above
    //  passes "" (it only wants to change the link), and S2F35 goes through that overload - so an
    //  unconditional assign here wiped the firmware CENAME of every CEID the host linked. S1F24 then
    //  answered a zero-length CENAME, breaking the published "292 entries byte-identical to your
    //  EventReport_CEID.def" promise; KYEC's provisioning links 31 CEIDs including 272-275, i.e. the
    //  four AMR names that were only just corrected. A freshly created node's Alias is already "",
    //  so the empty case needs no assignment and the firmware's own AddCEID (always non-empty for
    //  named events) is unaffected.
    if(CeidAlias!="")
        p->Alias = CeidAlias;
    int n = (int)iReportCount;
    if(n<0) n=0;
    if(n>32) n=32;
    p->ReportCount = n;
    for(int i=0; i<n; i++)
        p->ReportIDs[i] = iReportIDData[i];
}
//---------------------------------------------------------------------------
bool THGem::SetReportIDContent(unsigned iReportID, unsigned iReportCount, unsigned *iReportIDData, int Mode)
{
    if(ReportList==NULL) ReportList = new TList;
    TGemReportItem *p = FindReportItem(iReportID);
    if(p==NULL)
    {
        p = new TGemReportItem;
        p->ReportID = iReportID;
        p->Mode = Mode;   //AI(secs-reportdef) 20260724 : 0=host, 1=firmware
        ReportList->Add(p);
    }
    int n = (int)iReportCount;
    if(n<0) n=0;
    if(n>GEM_MAX_SVID_PER_REPORT) n=GEM_MAX_SVID_PER_REPORT;
    p->SVCount = n;
    for(int i=0; i<n; i++)
        p->SVIDs[i] = iReportIDData[i];
    return true;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260610 : Phase 0 SECS-II transmit-side encode codec.
//  Ported faithfully from HT172 uHGemEquipment.cpp (byte math only, no socket,
//  no GUI).  Builds a complete HSMS-framed message into LocalBuffer:
//      [4-byte length][2 DeviceID][S|W][F][PType][SType][4 SystemByte][body...]
//  Receive/decode path and socket transport are NOT implemented yet.
//---------------------------------------------------------------------------
int THGem::GetLengthOfType(unsigned char Type)
{
    if(Type==HType.ASCII_TYPE)   return 1;
    if(Type==HType.JIS_TYPE)     return 1;
    if(Type==HType.BINARY_TYPE)  return 1;
    if(Type==HType.BOOLEAN_TYPE) return 1;
    if(Type==HType.INT_1_TYPE)   return 1;
    if(Type==HType.UINT_1_TYPE)  return 1;
    if(Type==HType.INT_2_TYPE)   return 2;
    if(Type==HType.UINT_2_TYPE)  return 2;
    if(Type==HType.INT_4_TYPE)   return 4;
    if(Type==HType.UINT_4_TYPE)  return 4;
    if(Type==HType.FT_4_TYPE)    return 4;
    if(Type==HType.INT_8_TYPE)   return 8;
    if(Type==HType.UINT_8_TYPE)  return 8;
    if(Type==HType.FT_8_TYPE)    return 8;
    if(Type==HType.LIST_TYPE)    return 1;   // length field = item count
    return 1;
}
//---------------------------------------------------------------------------
// Encode the SECS-II item length into 1..3 bytes (MSB first) and return the
// number of length bytes used.  The low 2 bits of the format byte carry this.
//---------------------------------------------------------------------------
unsigned char THGem::GetLengthByte(unsigned len, unsigned char *Ptr)
{
    if(len > 0xffff)
    {
        Ptr[0] = (unsigned char)((len>>16)&0xff);
        Ptr[1] = (unsigned char)((len>>8)&0xff);
        Ptr[2] = (unsigned char)(len&0xff);
        return 3;
    }
    else if(len > 0xff)
    {
        Ptr[0] = (unsigned char)((len>>8)&0xff);
        Ptr[1] = (unsigned char)(len&0xff);
        return 2;
    }
    Ptr[0] = (unsigned char)(len&0xff);
    return 1;
}
//---------------------------------------------------------------------------
// Append len bytes of Value to LocalBuffer in MSB-first (big-endian) order.
//---------------------------------------------------------------------------
void THGem::ConvertLocalData(int len, void *Value)
{
    int i;
    unsigned char *p;
    p = (unsigned char *)Value;
    for(i=0; i<len; i++)
        LocalBuffer[LocalLength_4+i] = p[len-1-i];
    LocalLength_4 += len;
}
//---------------------------------------------------------------------------
// Build the 4-byte length field + 10-byte HSMS header from the Local head.
//---------------------------------------------------------------------------
void THGem::CreateLocalHead()
{
    unsigned int i, j;
    unsigned char c;
    memset(LocalBuffer, 0, LocalBufferSize);
    LocalLength = 0;
    LocalLength_4 = 4;
    ConvertLocalData(2, &Local.DeviceID);
    c = Local.MessageID_S;
    if(Local.W_Bit==1)
        c |= 0x80;
    ConvertLocalData(1, &c);
    ConvertLocalData(1, &Local.MessageID_F);
    ConvertLocalData(1, &Local.PType);
    ConvertLocalData(1, &Local.SType);
    j = Local.SystemByte;
    for(i=0; i<4; i++)
    {
        LocalBuffer[LocalLength_4+3-i] = (unsigned char)(j&0xff);
        j >>= 8;
    }
    LocalLength_4 += 4;
    LocalLength = LocalLength_4 - 4;
    j = LocalLength;
    for(i=0; i<4; i++)
    {
        LocalBuffer[3-i] = (unsigned char)(j&0xff);
        j >>= 8;
    }
}
//---------------------------------------------------------------------------
void THGem::InitLocalHead(int Stream, int Function, int WaitBit)
{
    //AI(secs-audit-fix) 20260729 : every outgoing message starts here, so this is the one place
    //that clears the encode-overflow poison flag for the message about to be built.
    bEncodeOverflow   = false;
    Local.DeviceID    = (unsigned short int)DeviceID;
    Local.MessageID_S = (unsigned char)Stream;
    Local.MessageID_F = (unsigned char)Function;
    Local.W_Bit       = WaitBit;
    Local.PType       = 0;
    Local.SType       = 0;
    if((Function % 2) == 0)
        Local.SystemByte = Remote.SystemByte;   // even F = reply, reuse request SystemByte
    else
    {
        EquipmentSystemByte++;                  // odd F = primary, new SystemByte
        Local.SystemByte = EquipmentSystemByte;
    }
    CreateLocalHead();
}
//---------------------------------------------------------------------------
//AI(secs-e30-gate) 20260803 : SxF0 Abort Transaction - the SEMI E30 refusal for a host primary
//  the equipment must not act on in its current control state ("the equipment responds with SxF0
//  to any primary other than S1F13 or S1F17 while OFF-LINE").
//  Mechanics, all free from the existing codec : Function 0 is EVEN, so InitLocalHead reuses
//  Remote.SystemByte and the abort therefore closes the very transaction being refused;
//  CreateLocalHead emits the 10-byte header and stamps 10 into the 4-byte length prefix, so this
//  is a legal 14-byte header-only frame with no body; SendLocalData still gates on
//  ActiveSocket != NULL && SELECTED. W_Bit is 0 - an abort is a reply and never expects one.
//  MUST be preferred over "log and send nothing" : silence is what made the host T3-timeout on
//  2026-07-23, and a T3 is indistinguishable from a dead machine.
void THGem::SendAbort(int Stream)
{
    InitLocalHead(Stream, 0, 0);
    SendLocalData();
    AnsiString S;
    S.sprintf("[SECS][TX] S%dF0 abort (refused in this control state)", Stream);
    StringOut(S);
}
//---------------------------------------------------------------------------
void THGem::DataItemOut(int len, unsigned char Type, void *P)
{
    int i, k;
    unsigned char SMLLength;
    unsigned char SMLLengthData[3];
    unsigned char DataSize;

    DataSize = (unsigned char)GetLengthOfType(Type);
    SMLLength = GetLengthByte((unsigned)(len*DataSize), SMLLengthData);
    //AI(secs-audit-fix) 20260729 : capacity backstop. Every write below indexes LocalBuffer by
    //LocalLength_4 with no check against LocalBufferSize (1 MB), so the encode side trusted the
    //caller completely. That was safe while every body was firmware-shaped, but S2F33 now lets
    //the HOST define reports of up to GEM_MAX_SVID_PER_REPORT SVIDs and S6F16/S6F11 serialize
    //them, so body size is host-influenced for the first time. Realistic worst case is ~2
    //orders of magnitude under 1 MB - this is a backstop, not an expected path - but a heap
    //overrun here would be remotely triggerable. Refuse the item and say so loudly.
    if(len<0 || DataSize<=0 ||
       (double)LocalLength_4 + 1.0 + (double)SMLLength + (double)len*(double)DataSize > (double)LocalBufferSize)
    {
        AnsiString sOvf;
        sOvf.sprintf("[SECS][TX] ENCODE OVERFLOW - message poisoned (cursor=%u len=%d size=%u cap=%u)",
                     LocalLength_4, len, (unsigned)DataSize, LocalBufferSize);
        StringOut(sOvf);
        bEncodeOverflow = true;      // SendLocalData will refuse to transmit this frame
        return;
    }
    LocalBuffer[LocalLength_4] = Type | SMLLength;
    LocalLength_4++;
    for(i=0; i<SMLLength; i++)
        LocalBuffer[LocalLength_4+i] = SMLLengthData[i];
    LocalLength_4 += SMLLength;

    if(Type==HType.ASCII_TYPE || Type==HType.BINARY_TYPE || Type==HType.JIS_TYPE)
    {
        unsigned char *ptr;
        ptr = (unsigned char *)P;
        for(i=0; i<len; i++)
            LocalBuffer[LocalLength_4+i] = ptr[i];
        LocalLength_4 += len;
    }
    else if(Type==HType.BOOLEAN_TYPE)
    {
        bool *ptr;
        ptr = (bool *)P;
        for(i=0; i<len; i++)
        {
            LocalBuffer[LocalLength_4] = (ptr[i]==true) ? 1 : 0;
            LocalLength_4++;
        }
    }
    else if(Type==HType.UINT_4_TYPE)
    {
        unsigned int *ptr, j;
        ptr = (unsigned int *)P;
        for(i=0; i<len; i++)
        {
            j = ptr[i];
            for(k=0; k<DataSize; k++) { LocalBuffer[LocalLength_4+DataSize-1-k]=(unsigned char)(j&0xff); j>>=8; }
            LocalLength_4 += DataSize;
        }
    }
    else if(Type==HType.UINT_2_TYPE)
    {
        unsigned short *ptr, j;
        ptr = (unsigned short *)P;
        for(i=0; i<len; i++)
        {
            j = ptr[i];
            for(k=0; k<DataSize; k++) { LocalBuffer[LocalLength_4+DataSize-1-k]=(unsigned char)(j&0xff); j>>=8; }
            LocalLength_4 += DataSize;
        }
    }
    else if(Type==HType.UINT_1_TYPE)
    {
        unsigned char *ptr, j;
        ptr = (unsigned char *)P;
        for(i=0; i<len; i++)
        {
            j = ptr[i];
            for(k=0; k<DataSize; k++) { LocalBuffer[LocalLength_4+DataSize-1-k]=(unsigned char)(j&0xff); }
            LocalLength_4 += DataSize;
        }
    }
    else if(Type==HType.UINT_8_TYPE)
    {
        unsigned __int64 *ptr, j;
        ptr = (unsigned __int64 *)P;
        for(i=0; i<len; i++)
        {
            j = ptr[i];
            for(k=0; k<DataSize; k++) { LocalBuffer[LocalLength_4+DataSize-1-k]=(unsigned char)(j&0xff); j>>=8; }
            LocalLength_4 += DataSize;
        }
    }
    else if(Type==HType.INT_1_TYPE)
    {
        char *ptr, j;
        ptr = (char *)P;
        for(i=0; i<len; i++)
        {
            j = ptr[i];
            for(k=0; k<DataSize; k++) { LocalBuffer[LocalLength_4+DataSize-1-k]=(unsigned char)(j&0xff); }
            LocalLength_4 += DataSize;
        }
    }
    else if(Type==HType.INT_2_TYPE)
    {
        short *ptr, j;
        ptr = (short *)P;
        for(i=0; i<len; i++)
        {
            j = ptr[i];
            for(k=0; k<DataSize; k++) { LocalBuffer[LocalLength_4+DataSize-1-k]=(unsigned char)(j&0xff); j>>=8; }
            LocalLength_4 += DataSize;
        }
    }
    else if(Type==HType.INT_4_TYPE)
    {
        int *ptr, j;
        ptr = (int *)P;
        for(i=0; i<len; i++)
        {
            j = ptr[i];
            for(k=0; k<DataSize; k++) { LocalBuffer[LocalLength_4+DataSize-1-k]=(unsigned char)(j&0xff); j>>=8; }
            LocalLength_4 += DataSize;
        }
    }
    else if(Type==HType.INT_8_TYPE)
    {
        __int64 *ptr, j;
        ptr = (__int64 *)P;
        for(i=0; i<len; i++)
        {
            j = ptr[i];
            for(k=0; k<DataSize; k++) { LocalBuffer[LocalLength_4+DataSize-1-k]=(unsigned char)(j&0xff); j>>=8; }
            LocalLength_4 += DataSize;
        }
    }
    else if(Type==HType.FT_4_TYPE)
    {
        float *floatPtr;
        unsigned char *ptr;
        floatPtr = (float *)P;
        for(i=0; i<len; i++)
        {
            ptr = (unsigned char *)(&floatPtr[i]);
            for(k=0; k<DataSize; k++) { LocalBuffer[LocalLength_4+DataSize-1-k]=ptr[0]; ptr++; }
            LocalLength_4 += DataSize;
        }
    }
    else if(Type==HType.FT_8_TYPE)
    {
        double *doublePtr;
        unsigned char *ptr;
        doublePtr = (double *)P;
        for(i=0; i<len; i++)
        {
            ptr = (unsigned char *)(&doublePtr[i]);
            for(k=0; k<DataSize; k++) { LocalBuffer[LocalLength_4+DataSize-1-k]=*ptr; ptr++; }
            LocalLength_4 += DataSize;
        }
    }
    // LIST_TYPE: header only (item count), no payload bytes.

    LocalLength = LocalLength_4 - 4;
    k = LocalLength;
    for(i=0; i<4; i++)
    {
        LocalBuffer[3-i] = (unsigned char)(k&0xff);
        k >>= 8;
    }
}
//---------------------------------------------------------------------------
// ASCII convenience overload.
//---------------------------------------------------------------------------
void THGem::DataItemOut(unsigned char Type, AnsiString Text)
{
    DataItemOut(Text.Length(), Type, (void *)Text.c_str());
}
//---------------------------------------------------------------------------
// Phase 0: the message is fully encoded in LocalBuffer[0..LocalLength_4-1].
// When an HSMS peer is connected and SELECTED, push the framed bytes out.
// Still logs the framed size for trace.
//---------------------------------------------------------------------------
void THGem::SendLocalData()
{
    AnsiString S;
    bool bSent = false;

    //AI(secs-audit-fix) 20260729 : refuse to transmit a frame that overflowed the encode buffer.
    //DataItemOut dropped at least one item, so the body's LIST header now declares more items
    //than the frame carries. A host parsing that would consume the following bytes as list
    //members and desync for the rest of the session; letting the primary time out (T3) instead
    //is recoverable. Unreachable with firmware-shaped bodies - this is a backstop for
    //host-defined S2F33 reports, which are the only host-sized thing we serialize.
    if(bEncodeOverflow)
    {
        S.sprintf("[SECS][TX] S%uF%u DROPPED - encode buffer overflow, frame would be malformed",
                  (unsigned)(Local.MessageID_S & 0x7f), (unsigned)Local.MessageID_F);
        StringOut(S);
        FlushSecsLogToFile();
        return;
    }

    if(ActiveSocket!=NULL && iHsmsState==HSMS_STATE_SELECTED)
    {
        try
        {
            ActiveSocket->SendBuf(LocalBuffer, (int)LocalLength_4);
            bSent = true;
        }
        catch(...)
        {
            bSent = false;
        }
    }
    S.sprintf("[SECS][TX] S%uF%u W=%u len=%u %s",
              (unsigned)(Local.MessageID_S & 0x7f), (unsigned)Local.MessageID_F,
              (unsigned)Local.W_Bit, (unsigned)LocalLength_4,
              bSent ? "(sent)" : "(not connected)");
    StringOut(S);
    //AI(ht160s-secsgem) 20260716 : full SML dump of the outgoing body (reply/event
    //  visibility, symmetric with the RX dump). LocalBuffer holds the framed message.
    LogSmlBody("TX", LocalBuffer, (int)LocalLength_4,
               (int)(Local.MessageID_S & 0x7f), (int)Local.MessageID_F, (int)Local.W_Bit);
}
//---------------------------------------------------------------------------
const unsigned char *THGem::GetLocalBuffer()
{
    return LocalBuffer;
}
//---------------------------------------------------------------------------
unsigned THGem::GetLocalLength()
{
    return LocalLength_4;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260610 : Phase 0 HSMS-SS socket transport (route B).
//  Active(client) or passive(server) TCP, length-prefixed HSMS frame assembly,
//  auto-answer of Select/Linktest/Separate control messages, and dispatch of
//  complete data messages into the GEM logic layer (HTGem::Dispatch).
//---------------------------------------------------------------------------
void THGem::SetGemLogic(HTGem *p)
{
    GemLogic = p;
}
//---------------------------------------------------------------------------
void THGem::SetHsmsMode(bool bActive)
{
    bActiveMode = bActive;
}
//---------------------------------------------------------------------------
void THGem::StartCommunication()
{
    int port;
    AnsiString S;

    //AI(ht160s-secsgem) 20260611 : record the intent to stay connected so the
    //  Timer1Timer watchdog will re-dial / re-listen after a failure or drop.
    bWantComm           = true;
    iReconnectAttempts  = 0;
    iReconnectCountdown = iReconnectInterval;

    if(bCommStarted)
        return;

    port = atoi(sDefaultPort.c_str());
    if(port <= 0)
        port = 5000;

    if(RecvBuffer!=NULL)
        RecvBuffer->Clear();
    ActiveSocket = NULL;
    iHsmsState   = HSMS_STATE_NOTCONNECTED;

    if(bActiveMode)
    {
        if(ClientSocket1!=NULL)
        {
            ClientSocket1->Address = sDefaultAddress;
            ClientSocket1->Port    = port;
            ClientSocket1->Active  = true;   // initiate outbound connect
        }
    }
    else
    {
        if(ServerSocket1!=NULL)
        {
            ServerSocket1->Port   = port;
            ServerSocket1->Active = true;    // start listening
        }
    }
    bCommStarted = true;
    S.sprintf("[SECS] StartCommunication mode=%s port=%d",
              bActiveMode ? "active" : "passive", port);
    StringOut(S);
}
//---------------------------------------------------------------------------
void THGem::StopCommunication()
{
    //AI(ht160s-secsgem) 20260611 : explicit stop = clear intent so the watchdog
    //  does not immediately re-open the socket we just closed.
    bWantComm = false;
    if(ClientSocket1!=NULL) ClientSocket1->Active = false;
    if(ServerSocket1!=NULL) ServerSocket1->Active = false;
    ActiveSocket = NULL;
    iHsmsState   = HSMS_STATE_NOTCONNECTED;
    bCommStarted = false;
    if(RecvBuffer!=NULL)
        RecvBuffer->Clear();
    StringOut("[SECS] StopCommunication");
}
//---------------------------------------------------------------------------
bool THGem::IsConnected()
{
    return (iHsmsState >= HSMS_STATE_CONNECTED);
}
//---------------------------------------------------------------------------
bool THGem::IsSelected()
{
    return (iHsmsState == HSMS_STATE_SELECTED);
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : reconnect watchdog config + status read-back.
void THGem::SetReconnectInterval(int Seconds)
{
    iReconnectInterval = (Seconds < 0) ? 0 : Seconds;
    if(iReconnectCountdown > iReconnectInterval)
        iReconnectCountdown = iReconnectInterval;
}
int THGem::GetReconnectInterval()  { return iReconnectInterval; }
int THGem::GetReconnectCountdown() { return iReconnectCountdown; }
int THGem::GetReconnectAttempts()  { return iReconnectAttempts; }
AnsiString THGem::GetReconnectStatusText()
{
    if(iHsmsState >= HSMS_STATE_CONNECTED)
        return AnsiString("");                 // connected, no retry pending
    if(!bWantComm || iReconnectInterval <= 0)
        return AnsiString("auto-reconnect off");
    AnsiString S;
    S.sprintf("retry in %ds (attempts %d)", iReconnectCountdown, iReconnectAttempts);
    return S;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : Linktest heartbeat / T6 timeout config.
void THGem::SetLinktestInterval(int Seconds)
{
    iLinktestInterval = (Seconds < 0) ? 0 : Seconds;
    if(iLinktestCountdown > iLinktestInterval)
        iLinktestCountdown = iLinktestInterval;
}
void THGem::SetT6Timeout(int Seconds)
{
    iT6Timeout = (Seconds <= 0) ? 6 : Seconds;   // never 0: must allow time for a reply
}
//AI(ht160s-secsgem) 20260612 : toggle routine Linktest req/rsp logging. Off by
//  default so the heartbeat does not flood the operator/disk log; anomalies
//  (T6 timeout, DropConnection) are always logged regardless of this flag.
void THGem::SetLogLinktest(bool On)
{
    bLogLinktest = On;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260716 : enable/disable the full SECS-II body SML dump.
void THGem::SetLogSmlBody(bool On)
{
    bLogSmlBody = On;
}
//---------------------------------------------------------------------------
//AI(secs-strict-reportdef) 20260810 : strict (E5-conformant) vs Path A tolerant report validation.
// ON  = S2F33 DRACK 0x04 on any undefined SVID, S2F35 LRACK 0x04/0x05 on any undefined CEID/RPTID,
//       each rejecting the whole packet without committing anything. This is the default.
// OFF = the 20260727 Path A behaviour, kept only as an on-site escape hatch.
void THGem::SetStrictReportValidation(bool On)
{
    bStrictReportValidation = On;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : SECS communication file logging. Layout aligned
//  with HT172 (per-day folder), but rooted at HSys.LogRootDir = D:\HT160S_Log.
//    D:\HT160S_Log\SECS_GEM\yyyy_mm_dd\SECSGEM_TextLog_<hh>.txt   (per hour)
//    D:\HT160S_Log\SECS_GEM\yyyy_mm_dd\SECSGEM_ErrLog.txt         (per day)
//  All disk I/O is try/catch-wrapped: a disk failure must never disturb comms.
void THGem::SetLogToFile(bool On)
{
    bLogToFile = On;
}
//---------------------------------------------------------------------------
AnsiString THGem::BuildSecsLogDir()
{
    Word y,mo,d;
    DecodeDate(Now(), y, mo, d);
    AnsiString Day;
    Day.sprintf("%04d_%02d_%02d", (int)y,(int)mo,(int)d);
    AnsiString Dir = HSys.LogRootDir + "\\SECS_GEM\\" + Day;
    try { ForceDirectories(Dir); } catch(...) {}
    return Dir;
}
//---------------------------------------------------------------------------
void THGem::FlushSecsLogToFile()
{
    if(!bLogToFile || LogFileBuffer==NULL || LogFileBuffer->Count <= 0)
        return;
    try
    {
        AnsiString Dir = BuildSecsLogDir();
        Word h,mi,s,ms;
        DecodeTime(Now(), h, mi, s, ms);
        AnsiString FN;
        FN.sprintf("%s\\SECSGEM_TextLog_%02d.txt", Dir.c_str(), (int)h);
        FILE *P = fopen(FN.c_str(), "a+");
        if(P != NULL)
        {
            for(int i=0; i<LogFileBuffer->Count; i++)
            {
                fputs(LogFileBuffer->Strings[i].c_str(), P);
                fputs("\n", P);
            }
            fclose(P);
            LogFileBuffer->Clear();
        }
    }
    catch(...)
    {
        // swallow: disk-logging failure must never disturb communication
    }
}
//---------------------------------------------------------------------------
void THGem::SaveSecsErrToLog(AnsiString Reason)
{
    if(!bLogToFile)
        return;
    try
    {
        AnsiString Dir = BuildSecsLogDir();
        AnsiString FN  = Dir + "\\SECSGEM_ErrLog.txt";
        Word y,mo,d,h,mi,s,ms;
        TDateTime tn = Now();
        DecodeDate(tn, y, mo, d);
        DecodeTime(tn, h, mi, s, ms);
        AnsiString Line;
        Line.sprintf("%04d/%02d/%02d %02d:%02d:%02d  %s",
                     (int)y,(int)mo,(int)d,(int)h,(int)mi,(int)s, Reason.c_str());
        FILE *P = fopen(FN.c_str(), "a+");
        if(P != NULL)
        {
            fputs(Line.c_str(), P);
            fputs("\n", P);
            fclose(P);
        }
    }
    catch(...)
    {
        // swallow
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : actively send a Linktest.req (own SessionID
//  0xFFFF + fresh SystemBytes) and arm the T6 wait.  Mirrors SendControlReply's
//  10-byte control header but is self-originated, not an echo.
void THGem::SendLinktestReq()
{
    if(ActiveSocket == NULL)
        return;
    unsigned char buf[14];
    unsigned sysb = uControlSystemByte++;
    if(uControlSystemByte == 0)
        uControlSystemByte = 1;                  // skip 0 on wrap
    memset(buf, 0, sizeof(buf));
    buf[0]=0; buf[1]=0; buf[2]=0; buf[3]=10;     // length = 10 header bytes
    buf[4]=0xFF; buf[5]=0xFF;                     // SessionID 0xFFFF (HSMS control)
    buf[6]=0;                                      // HeaderByte2
    buf[7]=0;                                      // HeaderByte3
    buf[8]=0;                                      // PType
    buf[9]=HSMS_STYPE_LINKTEST_REQ;               // SType = Linktest.req
    buf[10]=(unsigned char)((sysb>>24)&0xFF);
    buf[11]=(unsigned char)((sysb>>16)&0xFF);
    buf[12]=(unsigned char)((sysb>>8)&0xFF);
    buf[13]=(unsigned char)(sysb&0xFF);           // SystemBytes
    try { ActiveSocket->SendBuf(buf, 14); } catch(...) {}
    bAwaitLinktestRsp = true;
    iT6Countdown      = iT6Timeout;
    iLinktestCountdown= iLinktestInterval;
    //AI(ht160s-secsgem) 20260612 : routine heartbeat is logged only when LogLinktest=1
    //  so the operator/disk log is not flooded (industry: quiet keepalive, loud failures).
    if(bLogLinktest)
        StringOut("[SECS] Linktest.req sent (heartbeat)");
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : force-drop a dead/errored connection so the
//  reconnect watchdog can recover it.  Closes the peer socket and resets state.
void THGem::DropConnection(AnsiString Reason)
{
    AnsiString S = "[SECS] drop connection: " + Reason;
    StringOut(S);
    SaveSecsErrToLog(Reason);   //AI(ht160s-secsgem) 20260611 : keep a dedicated error trail
    if(ActiveSocket != NULL)
    {
        try { ActiveSocket->Close(); } catch(...) {}
    }
    if(bActiveMode && ClientSocket1 != NULL)
    {
        try { ClientSocket1->Active = false; } catch(...) {}   // allow clean re-dial
    }
    OnPeerDisconnected();
}
//---------------------------------------------------------------------------
void THGem::OnPeerConnected(TCustomWinSocket *Socket)
{
    //AI(secs-halfopen) 20260902 : a new inbound accept can silently REPLACE a half-open
    //  socket - the old peer died without a FIN/RST ever reaching us, so the OS never
    //  raised OnDisconnect.  Without this tear-down the stale ActiveSocket pointer is
    //  simply overwritten below, OnPeerDisconnected() never runs, and therefore
    //  GemLogic->OnCommunicationLost() never releases the latched host state : the
    //  PP_SIGNALTOWER / PP_MUSIC panel override and the AMR station lock stay orphaned
    //  (see that hook's own note in OnPeerDisconnected below).  Until now the Linktest
    //  T6 watchdog hid this by always dropping first - 211 drops on 2026-09-02 alone -
    //  so raising T6 to a sane value exposes it.  Guard it here instead.
    //AI(secs-halfopen-2) 20260903 : and CLOSE the stale socket, exactly as DropConnection()
    //  does. The 20260902 tear-down released only our state and left the old
    //  TCustomWinSocket alive inside TServerSocket; its LATE OnClientDisconnect /
    //  OnClientError would then have run into OnPeerDisconnected() / DropConnection() and
    //  torn down the healthy NEW link instead - the very failure this guard exists for,
    //  merely postponed. Close() may itself route through ServerClientDisconnect (which
    //  already calls OnPeerDisconnected), so tear down explicitly only if that did not
    //  happen. The identity checks in ServerClientDisconnect / ServerClientRead /
    //  ServerClientError below are the other half of this fix.
    if(ActiveSocket != NULL && ActiveSocket != Socket)
    {
        TCustomWinSocket *pStale = ActiveSocket;
        StringOut("[SECS] stale half-open peer replaced by a new connection - closing it");
        try { pStale->Close(); } catch(...) {}
        if(ActiveSocket != NULL)
            OnPeerDisconnected();
    }
    ActiveSocket = Socket;
    iHsmsState   = HSMS_STATE_CONNECTED;
    iReconnectCountdown = iReconnectInterval;   //AI(ht160s-secsgem) 20260611 : arm for next drop
    bAwaitLinktestRsp   = false;                //AI(ht160s-secsgem) 20260611 : reset heartbeat
    iLinktestCountdown  = iLinktestInterval;
    if(RecvBuffer!=NULL)
        RecvBuffer->Clear();
    StringOut("[SECS] peer connected (TCP up, awaiting Select)");
}
//---------------------------------------------------------------------------
void THGem::OnPeerDisconnected()
{
    ActiveSocket = NULL;
    iHsmsState   = HSMS_STATE_NOTCONNECTED;
    bAwaitLinktestRsp = false;                  //AI(ht160s-secsgem) 20260611 : reset heartbeat
    if(RecvBuffer!=NULL)
        RecvBuffer->Clear();
    //AI(secs-audit-fix) 20260729 : tell ProcessReceiveBuffer its cached base pointer just died.
    bRecvBufferReset = true;
    StringOut("[SECS] peer disconnected");
    //AI(secs-kyec-rcmd4-fix) 20260728 : tell the logic layer the host is gone so it can release
    // latched host state (PP_SIGNALTOWER / PP_MUSIC panel override). KYEC arms and clears that
    // pair seconds apart; if the link drops in between, the tower and buzzer would otherwise
    // stay host-driven forever - the same orphan-latch failure already seen with the AGV lock.
    // DropConnection() funnels here too, so this single hook covers peer disconnect, socket
    // error, Separate.req and our own watchdog drop.
    if(GemLogic!=NULL)
        GemLogic->OnCommunicationLost();
}
//---------------------------------------------------------------------------
void THGem::ReadFromPeer(TCustomWinSocket *Socket)
{
    int avail, got;
    unsigned char *tmp;

    if(Socket==NULL || RecvBuffer==NULL)
        return;
    avail = Socket->ReceiveLength();
    if(avail <= 0)
        return;
    tmp = new unsigned char[avail];
    got = Socket->ReceiveBuf(tmp, avail);
    if(got > 0)
    {
        RecvBuffer->Position = RecvBuffer->Size;   // append at end
        RecvBuffer->Write(tmp, got);
        ProcessReceiveBuffer();
    }
    delete [] tmp;
}
//---------------------------------------------------------------------------
// Assemble length-prefixed HSMS frames from RecvBuffer and process each one.
//---------------------------------------------------------------------------
void THGem::ProcessReceiveBuffer()
{
    unsigned char *base;
    int total, consumed, Value, frameLen;
    unsigned char SType;
    unsigned char *p;

    if(RecvBuffer==NULL)
        return;
    base  = (unsigned char *)RecvBuffer->Memory;
    total = (int)RecvBuffer->Size;
    consumed = 0;
    //AI(secs-audit-fix) 20260729 : arm the "buffer pulled out from under us" detector. See the
    //bRecvBufferReset comment in the header - a handler below can reach OnPeerDisconnected
    //(Separate.req at HandleControlMessage, or a socket error raised by any SendLocalData
    //inside a handler) which calls RecvBuffer->Clear() and frees the block "base" points at.
    bRecvBufferReset = false;
    if(base==NULL)
        return;

    while((total - consumed) >= 4)
    {
        p = base + consumed;
        Value = (p[0]<<24) | (p[1]<<16) | (p[2]<<8) | p[3];
        if(Value < 10 || Value > 100*1024*1024)
        {
            // Malformed length -> drop entire buffer to resync.
            StringOut("[SECS] RX malformed length, flushing buffer");
            consumed = total;
            break;
        }
        if((total - consumed) < (Value + 4))
            break;                              // incomplete frame, wait for more
        frameLen = Value + 4;
        SType    = p[9];
        if(SType == HSMS_STYPE_DATA)
            HandleDataMessage(p, frameLen);
        else
            HandleControlMessage(p, frameLen);
        //AI(secs-audit-fix) 20260729 : the handler dropped the link and OnPeerDisconnected
        //already cleared RecvBuffer, so "base" is dangling and there is nothing left to
        //compact. Bail out BEFORE touching base again or resizing the buffer - resizing here
        //would resurrect a stream holding whatever the allocator put back.
        if(bRecvBufferReset)
            return;
        consumed += frameLen;
    }

    if(consumed > 0)
    {
        int remain = total - consumed;
        if(remain > 0)
            memmove(base, base + consumed, remain);
        RecvBuffer->SetSize(remain);
        RecvBuffer->Position = remain;
    }
}
//---------------------------------------------------------------------------
void THGem::HandleControlMessage(unsigned char *Ptr, int Len)
{
    unsigned char SType = Ptr[9];
    switch(SType)
    {
    case HSMS_STYPE_SELECT_REQ:
        SendControlReply(Ptr, HSMS_STYPE_SELECT_RSP);
        //AI(secs-audit-fix) 20260729 : only promote to SELECTED if the reply actually went out.
        //SendControlReply's SendBuf can raise a socket error, which ScktComp routes to ClientError
        //-> DropConnection -> OnPeerDisconnected : ActiveSocket=NULL and iHsmsState=NOTCONNECTED.
        //Overwriting that with SELECTED produced a PERMANENT dead link - Timer1Timer takes the
        //SELECTED branch, SendLinktestReq returns immediately on ActiveSocket==NULL without arming
        //bAwaitLinktestRsp/iT6Countdown so the T6 drop can never fire, and the reconnect watchdog
        //skips a state >= CONNECTED, so DoReconnectAttempt never re-dials. SECS stayed dead until
        //the program restarted while the on-screen badge still reported the link up.
        if(ActiveSocket!=NULL)
        {
            iHsmsState = HSMS_STATE_SELECTED;
            StringOut("[SECS] Select.req -> Select.rsp (SELECTED)");
        }
        break;
    case HSMS_STYPE_LINKTEST_REQ:
        SendControlReply(Ptr, HSMS_STYPE_LINKTEST_RSP);
        if(bLogLinktest)
            StringOut("[SECS] Linktest.req -> Linktest.rsp");
        break;
    case HSMS_STYPE_SEPARATE_REQ:
        StringOut("[SECS] Separate.req -> closing connection");
        if(ActiveSocket!=NULL)
        {
            try { ActiveSocket->Close(); } catch(...) {}
        }
        OnPeerDisconnected();
        break;
    case HSMS_STYPE_SELECT_RSP:
        iHsmsState = HSMS_STATE_SELECTED;   // active mode: our Select accepted
        StringOut("[SECS] Select.rsp received (SELECTED)");
        break;
    case HSMS_STYPE_LINKTEST_RSP:
        bAwaitLinktestRsp = false;          //AI(ht160s-secsgem) 20260611 : heartbeat ack
        if(bLogLinktest)
            StringOut("[SECS] Linktest.rsp received");
        break;
    default:
        {
            AnsiString S;
            S.sprintf("[SECS] unhandled control SType=%u", (unsigned)SType);
            StringOut(S);
        }
        break;
    }
}
//---------------------------------------------------------------------------
void THGem::HandleDataMessage(unsigned char *Ptr, int Len)
{
    int S, F, rc;
    AnsiString L;

    S = Ptr[6] & 0x7f;
    F = Ptr[7];
    rc = DecodeReceiveBody(Ptr, Len);
    L.sprintf("[SECS][RX] S%dF%d decoded rc=%d items=%d", S, F, rc,
              (SReceiveData!=NULL) ? SReceiveData->Count : 0);
    StringOut(L);
    //AI(ht160s-secsgem) 20260716 : full SML dump of the received body (host command
    //  visibility). Reads the raw frame bytes, so it is unaffected by Dispatch()
    //  consuming SReceiveData right after this.
    LogSmlBody("RX", Ptr, Len, S, F, (Ptr[6] & 0x80) ? 1 : 0);

    if(GemLogic!=NULL)
        GemLogic->Dispatch(S, F);
    else
        StringOut("[SECS] no GemLogic dispatch target set");
}
//---------------------------------------------------------------------------
// Build a 10-byte HSMS control header (no body) echoing the request's
// SessionID and SystemBytes, then send it.
//---------------------------------------------------------------------------
void THGem::SendControlReply(unsigned char *ReqPtr, unsigned char ReplySType)
{
    unsigned char buf[14];

    if(ActiveSocket==NULL)
        return;
    memset(buf, 0, sizeof(buf));
    buf[0]=0; buf[1]=0; buf[2]=0; buf[3]=10;        // length = 10 header bytes
    buf[4]=ReqPtr[4]; buf[5]=ReqPtr[5];             // echo SessionID
    buf[6]=0;                                       // HeaderByte2
    buf[7]=0;                                       // HeaderByte3 / status = 0 (OK)
    buf[8]=0;                                       // PType (SECS-II)
    buf[9]=ReplySType;                              // SType
    buf[10]=ReqPtr[10]; buf[11]=ReqPtr[11];
    buf[12]=ReqPtr[12]; buf[13]=ReqPtr[13];         // echo SystemBytes
    try { ActiveSocket->SendBuf(buf, 14); } catch(...) {}
}
//---------------------------------------------------------------------------
// Socket event handlers (active client mode).
//---------------------------------------------------------------------------
void __fastcall THGem::ClientConnect(TObject *Sender, TCustomWinSocket *Socket)
{
    OnPeerConnected(Socket);
}
void __fastcall THGem::ClientDisconnect(TObject *Sender, TCustomWinSocket *Socket)
{
    OnPeerDisconnected();
}
void __fastcall THGem::ClientRead(TObject *Sender, TCustomWinSocket *Socket)
{
    ReadFromPeer(Socket);
}
void __fastcall THGem::ClientError(TObject *Sender, TCustomWinSocket *Socket,
                                   TErrorEvent ErrorEvent, int &ErrorCode)
{
    AnsiString S;
    S.sprintf("[SECS] client socket error event=%d code=%d", (int)ErrorEvent, ErrorCode);
    StringOut(S);
    ErrorCode = 0;   // suppress VCL exception
    //AI(ht160s-secsgem) 20260611 : a socket error (incl. RST) does NOT raise
    //  OnDisconnect, so force the state down here -> watchdog re-dials.
    if(iHsmsState >= HSMS_STATE_CONNECTED)
        DropConnection("client socket error");
}
//---------------------------------------------------------------------------
// Socket event handlers (passive server mode).
//---------------------------------------------------------------------------
void __fastcall THGem::ServerClientConnect(TObject *Sender, TCustomWinSocket *Socket)
{
    OnPeerConnected(Socket);
}
void __fastcall THGem::ServerClientDisconnect(TObject *Sender, TCustomWinSocket *Socket)
{
    //AI(secs-halfopen-2) 20260903 : only the ACTIVE peer may tear the session down. A socket
    //  that OnPeerConnected() already replaced still delivers its own late disconnect;
    //  acting on it would drop the healthy link. (ActiveSocket==NULL keeps the old path.)
    if(ActiveSocket != NULL && Socket != ActiveSocket)
    {
        StringOut("[SECS] stale peer disconnected (ignored, live link untouched)");
        return;
    }
    OnPeerDisconnected();
}
void __fastcall THGem::ServerClientRead(TObject *Sender, TCustomWinSocket *Socket)
{
    //AI(secs-halfopen-2) 20260903 : bytes from a stale socket must never enter the live
    //  HSMS frame buffer (RecvBuffer is shared and length-prefixed - one foreign byte
    //  desynchronises every frame after it).
    if(ActiveSocket != NULL && Socket != ActiveSocket)
        return;
    ReadFromPeer(Socket);
}
void __fastcall THGem::ServerClientError(TObject *Sender, TCustomWinSocket *Socket,
                                         TErrorEvent ErrorEvent, int &ErrorCode)
{
    AnsiString S;
    S.sprintf("[SECS] server socket error event=%d code=%d", (int)ErrorEvent, ErrorCode);
    StringOut(S);
    ErrorCode = 0;   // suppress VCL exception
    //AI(secs-halfopen-2) 20260903 : an error on a socket OnPeerConnected() already replaced
    //  concerns the dead peer only. DropConnection() would Close() ActiveSocket - the
    //  healthy new link - so log it and leave the session alone.
    if(ActiveSocket != NULL && Socket != ActiveSocket)
    {
        StringOut("[SECS] stale peer error (ignored, live link untouched)");
        return;
    }
    //AI(ht160s-secsgem) 20260611 : same as client side - a peer RST/error gives
    //  no OnClientDisconnect, so drop the state so a new client can be accepted.
    if(iHsmsState >= HSMS_STATE_CONNECTED)
        DropConnection("server socket error");
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260610 : Phase 0 SECS-II receive-side decode codec.
//  Ported faithfully from HT172 uHGemEquipment.cpp (ProcessSML + DataItemInSub
//  + public wrappers), stripped of GUI/spool display.  Tokenizes a received
//  HSMS frame body into SReceiveData; DataItemIn / GetDataItemLenAndType* then
//  read it.  Socket transport that fills the frame buffer is NOT wired yet.
//---------------------------------------------------------------------------
// Number of length bytes (low 2 bits of the format byte) -> decoded item length.
//---------------------------------------------------------------------------
int THGem::GetSMLLenthByte(unsigned char TypeChar, unsigned char *Ptr, int RunLength)
{
    int len = TypeChar & 0x03;   // number of length bytes
    int ct = 0;
    int i;
    for(i=0; i<len; i++)
    {
        ct <<= 8;
        ct += Ptr[RunLength+i];
    }
    return ct;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260716 : read-only recursive SECS-II body pretty-printer.
//  Renders one item at Ptr[RunLength..] as an indented SML subtree appended to Out,
//  in the HT9045/HT172 log format, then advances RunLength past it. It NEVER stores
//  into SReceiveData, so it is immune to the destructive token consumption done by
//  the S/F handlers (DataItemInSub Delete(0)) and can dump the true wire bytes even
//  after a handler has read them. Every byte access is bounds-checked against Len.
//  Returns 0 on success; <0 on a bounds/format error (Out still holds the partial
//  decode). ASCII/binary payloads are capped so a giant blob cannot flood the log.
int THGem::RenderSmlItem(unsigned char *Ptr, int Len, int &RunLength, int Depth,
                         AnsiString &Out)
{
    int j, k, ItemSize, TypeSize, ValueCount;
    unsigned char TypeChar, c;
    unsigned int ct, i;
    AnsiString Indent, S;

    if(Ptr==NULL || RunLength<0 || RunLength>=Len || Depth>64)
        return -1;

    for(j=0; j<Depth; j++)
        Indent += "  ";

    TypeChar = Ptr[RunLength];
    c = (unsigned char)(TypeChar & 0xfc);

    if(c==HType.LIST_TYPE)
    {
        RunLength++;
        if(RunLength + (TypeChar & 0x03) > Len)   // truncated length field
            return -1;
        ct = (unsigned int)GetSMLLenthByte(TypeChar, Ptr, RunLength);
        RunLength += TypeChar & 0x03;
        S.sprintf("%s<L[%u]", Indent.c_str(), ct);
        Out += S + "\n";
        for(i=0; i<ct; i++)
        {
            if(RenderSmlItem(Ptr, Len, RunLength, Depth+1, Out) < 0)
                return -2;
        }
        Out += Indent + ">\n";
        return 0;
    }

    // scalar item : one type byte + length byte(s) + payload
    RunLength++;
    if(RunLength + (TypeChar & 0x03) > Len)   // truncated length field
        return -1;
    ItemSize = GetSMLLenthByte(TypeChar, Ptr, RunLength);
    RunLength += TypeChar & 0x03;
    if(ItemSize < 0 || RunLength + ItemSize > Len)
    {
        S.sprintf("%s<0x%02X[%d] TRUNCATED>", Indent.c_str(), (unsigned)c, ItemSize);
        Out += S + "\n";
        return -3;
    }

    if(c==HType.ASCII_TYPE || c==HType.JIS_TYPE)
    {
        int cap = (ItemSize > SECS_SML_ASCII_CAP) ? SECS_SML_ASCII_CAP : ItemSize;
        S.sprintf("%s<A[%d] \"", Indent.c_str(), ItemSize);
        for(j=0; j<cap; j++)
        {
            unsigned char ch = Ptr[RunLength+j];
            if(ch>=0x20 && ch<0x7f)
                S += (char)ch;               // printable ASCII as-is
            else
            {
                AnsiString e;                // escape control/high bytes so the log stays one tree
                e.sprintf("\\x%02X", (unsigned)ch);
                S += e;
            }
        }
        if(ItemSize > SECS_SML_ASCII_CAP)
        {
            AnsiString t;
            t.sprintf("...(+%d bytes)", ItemSize - SECS_SML_ASCII_CAP);
            S += t;
        }
        RunLength += ItemSize;
        S += "\">";
        Out += S + "\n";
        return 0;
    }

    if(c==HType.BINARY_TYPE)
    {
        int cap = (ItemSize > SECS_SML_BIN_CAP) ? SECS_SML_BIN_CAP : ItemSize;
        S.sprintf("%s<B[%d]", Indent.c_str(), ItemSize);
        for(j=0; j<cap; j++)
        {
            AnsiString h;
            h.sprintf(" 0x%02X", (unsigned)Ptr[RunLength+j]);
            S += h;
        }
        if(ItemSize > SECS_SML_BIN_CAP)
        {
            AnsiString t;
            t.sprintf(" ...(+%d)", ItemSize - SECS_SML_BIN_CAP);
            S += t;
        }
        RunLength += ItemSize;
        S += ">";
        Out += S + "\n";
        return 0;
    }

    if(c==HType.BOOLEAN_TYPE)
    {
        S.sprintf("%s<Boolean[%d]", Indent.c_str(), ItemSize);
        for(j=0; j<ItemSize; j++)
        {
            unsigned char b = (Ptr[RunLength+j]==0) ? 0 : 1;
            AnsiString h;
            h.sprintf(" 0x%02X", (unsigned)b);
            S += h;
        }
        RunLength += ItemSize;
        S += ">";
        Out += S + "\n";
        return 0;
    }

    // numeric item : pick element width + label, then render each value
    AnsiString label;
    int isFloat = 0, isUnsigned = 0;   // signed is the default (label encodes exact type)
    TypeSize = 1;
    if(c==HType.UINT_1_TYPE)      { TypeSize=1; label="U1"; isUnsigned=1; }
    else if(c==HType.UINT_2_TYPE) { TypeSize=2; label="U2"; isUnsigned=1; }
    else if(c==HType.UINT_4_TYPE) { TypeSize=4; label="U4"; isUnsigned=1; }
    else if(c==HType.UINT_8_TYPE) { TypeSize=8; label="U8"; isUnsigned=1; }
    else if(c==HType.INT_1_TYPE)  { TypeSize=1; label="I1"; }
    else if(c==HType.INT_2_TYPE)  { TypeSize=2; label="I2"; }
    else if(c==HType.INT_4_TYPE)  { TypeSize=4; label="I4"; }
    else if(c==HType.INT_8_TYPE)  { TypeSize=8; label="I8"; }
    else if(c==HType.FT_4_TYPE)   { TypeSize=4; label="F4"; isFloat=1; }
    else if(c==HType.FT_8_TYPE)   { TypeSize=8; label="F8"; isFloat=1; }
    else
    {
        // unknown format code : dump raw hex so nothing is silently swallowed
        int cap = (ItemSize > SECS_SML_BIN_CAP) ? SECS_SML_BIN_CAP : ItemSize;
        S.sprintf("%s<0x%02X[%d]", Indent.c_str(), (unsigned)c, ItemSize);
        for(j=0; j<cap; j++)
        {
            AnsiString h;
            h.sprintf(" 0x%02X", (unsigned)Ptr[RunLength+j]);
            S += h;
        }
        if(ItemSize > SECS_SML_BIN_CAP)
        {
            AnsiString t;
            t.sprintf(" ...(+%d)", ItemSize - SECS_SML_BIN_CAP);
            S += t;
        }
        RunLength += ItemSize;
        S += ">";
        Out += S + "\n";
        return 0;
    }

    ValueCount = (TypeSize>0) ? (ItemSize/TypeSize) : 0;
    S.sprintf("%s<%s[%d]", Indent.c_str(), label.c_str(), ValueCount);
    for(j=0; j<ValueCount; j++)
    {
        AnsiString v;
        if(isFloat && TypeSize==4)
        {
            float f;
            unsigned char *pf = (unsigned char *)&f;
            for(k=0; k<4; k++) pf[k] = Ptr[RunLength+3-k];   // MSB-first on the wire
            v = " " + FloatToStr((double)f);                 // full precision (matches HT172); %g lost digits
        }
        else if(isFloat && TypeSize==8)
        {
            double d;
            unsigned char *pd = (unsigned char *)&d;
            for(k=0; k<8; k++) pd[k] = Ptr[RunLength+7-k];
            v = " " + FloatToStr(d);
        }
        else if(TypeSize<=4)
        {
            if(isUnsigned)
            {
                unsigned u = 0;
                for(k=0; k<TypeSize; k++) { u = (u<<8) | Ptr[RunLength+k]; }
                v.sprintf(" %u", u);
            }
            else
            {
                int s = 0;
                for(k=0; k<TypeSize; k++) { s = (s<<8) | Ptr[RunLength+k]; }
                if(TypeSize<4)   // sign-extend from TypeSize bytes
                {
                    int signbit = 1 << (TypeSize*8 - 1);
                    if(s & signbit)
                        s -= (1 << (TypeSize*8));
                }
                v.sprintf(" %d", s);
            }
        }
        else if(isUnsigned)   // U8 : IntToStr is signed, so render the true magnitude by hand
        {
            unsigned __int64 u = 0;
            for(k=0; k<8; k++) { u = (u<<8) | (unsigned char)Ptr[RunLength+k]; }
            if(u==0)
                v = " 0";
            else
            {
                AnsiString d = "";
                while(u > 0) { d = AnsiString((int)(u%10)) + d; u /= 10; }
                v = " " + d;
            }
        }
        else   // I8 : IntToStr(__int64) dodges printf %I64/%L portability traps
        {
            __int64 q = 0;
            for(k=0; k<8; k++) { q = (q<<8) | (unsigned char)Ptr[RunLength+k]; }
            v = " " + IntToStr(q);
        }
        RunLength += TypeSize;
        S += v;
    }
    S += ">";
    Out += S + "\n";
    return 0;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260716 : emit a full SML body dump for one HSMS frame when
//  LogSmlBody is on. Dir = "RX"/"TX". Ptr[0..3]=len, [4..13]=header, [14..]=body.
//  On a parse error the raw frame bytes are appended as hex so a malformed host
//  command is never hidden -- exactly the "verify a format error" use case.
void THGem::LogSmlBody(const char *Dir, unsigned char *Ptr, int Len, int S, int F, int W)
{
    if(!bLogSmlBody || Ptr==NULL || Len<14)
        return;

    //AI(secs-e30-gate) 20260803 : Function 0 (Abort Transaction) is header-only BY DEFINITION, so
    //  there is no body to render and RenderSmlItem would report a parse error on every abort -
    //  proven on this wire already, the host's own header-only S1F17 logs exactly that. Skip the
    //  dump instead of teaching the field to ignore a recurring "[SML parse error rc=-1]".
    if(F==0)
        return;

    AnsiString Block, Tree;
    Block.sprintf("[SECS][%s] S%dF%d W=%d body:", Dir, S, F, W);
    int RunLength = 14;
    int rc = RenderSmlItem(Ptr, Len, RunLength, 0, Tree);
    Block += "\n" + Tree;
    if(rc < 0)
    {
        int cap = (Len > SECS_SML_RAW_CAP) ? SECS_SML_RAW_CAP : Len;
        AnsiString Hex;
        Hex.sprintf("[SML parse error rc=%d] RAW %d bytes:", rc, Len);
        for(int n=0; n<cap; n++)
        {
            AnsiString h;
            h.sprintf(" %02X", (unsigned)Ptr[n]);
            Hex += h;
        }
        if(Len > SECS_SML_RAW_CAP)
            Hex += " ...";
        Block += Hex + "\n";
    }
    StringOut(Block);
}
//---------------------------------------------------------------------------
void THGem::StoreToReceiveString(AnsiString S)
{
    if(bReceiveData==false)
        return;
    if(SReceiveData!=NULL)
        SReceiveData->Add(S);
}
//---------------------------------------------------------------------------
// Read the 10-byte HSMS header (Ptr[4..13]) into Remote.  Used so that an
// even-function reply can reuse the request SystemByte (see InitLocalHead).
//---------------------------------------------------------------------------
void THGem::ProcessRemoteHead(unsigned char *Ptr)
{
    Remote.DeviceID    = (unsigned short int)(((unsigned)Ptr[4]<<8) | Ptr[5]);
    Remote.W_Bit       = (Ptr[6] & 0x80) ? 1 : 0;
    Remote.MessageID_S = (unsigned char)(Ptr[6] & 0x7f);
    Remote.MessageID_F = Ptr[7];
    Remote.PType       = Ptr[8];
    Remote.SType       = Ptr[9];
    Remote.SystemByte  = ((unsigned int)Ptr[10]<<24) | ((unsigned int)Ptr[11]<<16) |
                         ((unsigned int)Ptr[12]<<8)  |  (unsigned int)Ptr[13];
}
//---------------------------------------------------------------------------
// Recursive SECS-II body walker.  Pushes Type / length / value tokens into
// SReceiveData in the exact order DataItemInSub expects.
//---------------------------------------------------------------------------
int THGem::ProcessSML(unsigned char *Ptr, int Len, int &RunLength)
{
    int j, k, ItemSize, TypeSize, ret;
    unsigned char TypeChar;
    unsigned int ct, i;
    unsigned char c;

    if(RunLength>=Len)
        return -1;

    TypeChar = Ptr[RunLength];
    c = TypeChar & 0xfc;        // format code (mask off length bits)
    if(c!=HType.LIST_TYPE)
    {
        ct = 1;
    }
    else
    {
        StoreToReceiveString(HType.LIST_TYPE);
        RunLength++;
        //AI(secs-audit-fix) 20260729 : bound the LENGTH FIELD itself, not just the payload.
        //GetSMLLenthByte reads (TypeChar & 0x03) bytes at Ptr[RunLength..] with no check of its
        //own, so a frame truncated in the middle of a length field read up to 3 bytes past it.
        //Identical in form to the guard RenderSmlItem already has for the same read.
        if(RunLength + (TypeChar & 0x03) > Len)
            return -2;
        ct = GetSMLLenthByte(TypeChar, Ptr, RunLength);
        RunLength += TypeChar & 0x03;
        StoreToReceiveString((int)ct);
    }
    for(i=0; i<ct; i++)
    {
        if(RunLength>=Len)
            return -2;
        TypeChar = Ptr[RunLength];
        c = Ptr[RunLength] & 0xfc;
        RunLength++;
        if(c==HType.LIST_TYPE)
        {
            RunLength--;
            ret = ProcessSML(Ptr, Len, RunLength);
            if(ret<0)
                return ret;
            else
                continue;
        }
        //AI(secs-audit-fix) 20260729 : bound the scalar item's LENGTH FIELD before reading it,
        //same as the LIST case above and as RenderSmlItem already does.
        if(RunLength + (TypeChar & 0x03) > Len)
            return -2;
        ItemSize = GetSMLLenthByte(TypeChar, Ptr, RunLength);
        RunLength += TypeChar & 0x03;
        if(c==HType.ASCII_TYPE)
        {
            StoreToReceiveString((int)c);
            StoreToReceiveString(ItemSize);
            //AI(secs-audit-fix) 20260729 : frame-bounds check. This was the ONLY item branch
            //without one - every sibling below (BINARY / BOOLEAN / U* / I* / F*) tests
            //RunLength>=Len per byte. ItemSize is HOST-DECLARED (up to 0xFFFFFF from three
            //length bytes), so a host that declares a longer ASCII item than it actually sent
            //made this loop read past the frame - up to 16 MB of adjacent heap. Harmless while
            //no handler kept the value, but the S10F3/S10F5 terminal-text handlers now sink
            //host ASCII verbatim into the SECS text log, the EventLog CSV and SecsAlarmMessage,
            //which turns the over-read into a data-disclosure sink that lands on disk. Reject
            //the frame with -2 (same code the siblings use) instead of copying.
            if(ItemSize<0 || RunLength+ItemSize>Len)
                return -2;
            char *Temp;
            Temp = new char[ItemSize+4];
            for(j=0; j<ItemSize; j++)
                Temp[j] = Ptr[RunLength+j];
            RunLength += ItemSize;
            Temp[ItemSize] = 0;
            StoreToReceiveString(Temp);
            delete [] Temp;
            Temp = NULL;
        }
        else if(c==HType.BINARY_TYPE)
        {
            StoreToReceiveString((int)c);
            StoreToReceiveString(ItemSize);
            for(j=0; j<ItemSize; j++)
            {
                if(RunLength>=Len)
                    return -2;
                StoreToReceiveString((int)(unsigned)(Ptr[RunLength]));
                RunLength++;
            }
        }
        else if(c==HType.BOOLEAN_TYPE)
        {
            StoreToReceiveString((int)c);
            StoreToReceiveString(ItemSize);
            for(j=0; j<ItemSize; j++)
            {
                if(RunLength>=Len)
                    return -2;
                if(Ptr[RunLength]==1 || Ptr[RunLength]==0xFF)
                    StoreToReceiveString(1);
                else
                    StoreToReceiveString(0);
                RunLength++;
            }
        }
        else if(c==HType.UINT_1_TYPE || c==HType.UINT_2_TYPE || c==HType.UINT_4_TYPE)
        {
            StoreToReceiveString((int)c);
            if(c==HType.UINT_1_TYPE)   TypeSize = 1;
            else if(c==HType.UINT_2_TYPE)   TypeSize = 2;
            else                            TypeSize = 4;
            StoreToReceiveString(ItemSize/TypeSize);
            unsigned int Temp;
            for(j=0; j<(ItemSize/TypeSize); j++)
            {
                Temp = 0;
                for(k=0; k<TypeSize; k++)
                {
                    if(RunLength>=Len)
                        return -2;
                    Temp <<= 8;
                    Temp += Ptr[RunLength];
                    RunLength++;
                }
                StoreToReceiveString((unsigned)Temp);
            }
        }
        else if(c==HType.UINT_8_TYPE)
        {
            StoreToReceiveString((int)c);
            TypeSize = 8;
            StoreToReceiveString(ItemSize/TypeSize);
            unsigned __int64 Temp;
            for(j=0; j<(ItemSize/TypeSize); j++)
            {
                Temp = 0;
                for(k=0; k<TypeSize; k++)
                {
                    if(RunLength>=Len)
                        return -2;
                    Temp <<= 8;
                    Temp += Ptr[RunLength];
                    RunLength++;
                }
                StoreToReceiveString((unsigned __int64)Temp);
            }
        }
        else if(c==HType.INT_1_TYPE || c==HType.INT_2_TYPE ||
                c==HType.INT_4_TYPE || c==HType.INT_8_TYPE)
        {
            StoreToReceiveString((int)c);
            if(c==HType.INT_1_TYPE)        TypeSize = 1;
            else if(c==HType.INT_2_TYPE)   TypeSize = 2;
            else if(c==HType.INT_4_TYPE)   TypeSize = 4;
            else                           TypeSize = 8;
            StoreToReceiveString(ItemSize/TypeSize);
            __int64 temp;
            unsigned char *p;
            for(j=0; j<(ItemSize/TypeSize); j++)
            {
                temp = 0;
                if(RunLength>=Len)
                    return -2;
                p = (unsigned char *)&temp;
                for(k=0; k<TypeSize; k++)
                {
                    if((RunLength+TypeSize-1-k)>=Len)
                        return -2;
                    p[k] = Ptr[RunLength+TypeSize-1-k];
                }
                RunLength += TypeSize;
                // sign-extend from TypeSize bytes
                if(TypeSize<8 && (p[TypeSize-1] & 0x80))
                {
                    for(k=TypeSize; k<8; k++)
                        p[k] = 0xFF;
                }
                StoreToReceiveString((__int64)temp);
            }
        }
        else if(c==HType.FT_4_TYPE)
        {
            StoreToReceiveString((int)c);
            TypeSize = 4;
            StoreToReceiveString(ItemSize/TypeSize);
            float temp;
            unsigned char *p;
            for(j=0; j<(ItemSize/TypeSize); j++)
            {
                temp = 0;
                if(RunLength>=Len)
                    return -2;
                p = (unsigned char *)&temp;
                for(k=0; k<TypeSize; k++)
                {
                    if((RunLength+TypeSize-1-k)>=Len)
                        return -2;
                    p[k] = Ptr[RunLength+TypeSize-1-k];
                }
                RunLength += TypeSize;
                StoreToReceiveString(AnsiString(temp));
            }
        }
        else if(c==HType.FT_8_TYPE)
        {
            StoreToReceiveString((int)c);
            TypeSize = 8;
            StoreToReceiveString(ItemSize/TypeSize);
            double temp;
            unsigned char *p;
            for(j=0; j<(ItemSize/TypeSize); j++)
            {
                temp = 0;
                if(RunLength>=Len)
                    return -2;
                p = (unsigned char *)&temp;
                for(k=0; k<TypeSize; k++)
                {
                    if((RunLength+TypeSize-1-k)>=Len)
                        return -2;
                    p[k] = Ptr[RunLength+TypeSize-1-k];
                }
                RunLength += TypeSize;
                StoreToReceiveString(AnsiString(temp));
            }
        }
    }
    return 0;
}
//---------------------------------------------------------------------------
// Reset and tokenize a full HSMS frame (header at Ptr[4..13], body from 14).
//---------------------------------------------------------------------------
int THGem::DecodeReceiveBody(unsigned char *Ptr, int Len)
{
    int RunLength = 14;
    if(SReceiveData!=NULL)
        SReceiveData->Clear();
    iReturnCode = 1;
    if(Ptr==NULL || Len<14)
        return -1;
    ProcessRemoteHead(Ptr);
    bReceiveData = true;
    ProcessSML(Ptr, Len, RunLength);
    return 1;
}
//---------------------------------------------------------------------------
void THGem::ResetReturnCode()
{
    iReturnCode = 1;
}
//---------------------------------------------------------------------------
int THGem::GetReturnCode()
{
    return iReturnCode;
}
//---------------------------------------------------------------------------
// Read one item's data into P, verifying type and length.  Consumes tokens.
//---------------------------------------------------------------------------
int THGem::DataItemInSub(int len, unsigned char Type, void *P)
{
    unsigned char t;
    int l, i;

    if(SReceiveData->Count==0)
        return -1;
    t = (unsigned char)atoi(SReceiveData->Strings[0].c_str());   // Type
    if(t!=Type)
        return -1;
    SReceiveData->Delete(0);
    if(SReceiveData->Count==0)
        return -1;
    l = atoi(SReceiveData->Strings[0].c_str());                 // length of data

    if(t==HType.ASCII_TYPE)
    {
        if(l>len)
            return -2;
    }
    else
    {
        if(len!=l)
            return -2;
    }
    SReceiveData->Delete(0);

    if(t==HType.LIST_TYPE)
    {
        return 1;
    }
    else if(t==HType.ASCII_TYPE)
    {
        char *temp;
        temp = (char *)P;
        if(SReceiveData->Count==0)
            return -1;
        //AI(secs-audit-fix) 20260729 : was strncpy(temp, src, len+1). strncpy ZERO-PADS to n, so
        //that wrote exactly len+1 bytes on EVERY call, however short the string. Now only
        //strlen(src)+1 bytes are written, so the typical call touches far less of the caller's
        //buffer - but be clear about what did NOT change: the CONTRACT is still
        //"len = maximum string length, and the caller's buffer must be len+1 bytes", because a
        //host string of exactly len characters still needs temp[len] for the terminator.
        //HT160's own raw-buffer callers honour it (char CommandStr[256] guarded with len<256 at
        //uHGemHT160.cpp:717/805/987, so at most 256 bytes into 256). The HT9045 idiom
        //"char str[1024]; DataItemIn(1024, ASCII_TYPE, str);" still overflows by one byte at the
        //boundary and must NOT be copied - clamping to len-1 here is not the answer either, since
        //that would silently truncate the last character for every conforming caller. New code
        //should use GetDataItemLenAndType + the DataItemIn(int, unsigned char, AnsiString&)
        //overload, which allocates its own buffer; S10F4/S10F6 already do (uHGemHT160.cpp:1914/1955).
        {
            const char *src = SReceiveData->Strings[0].c_str();
            int cp = (int)strlen(src);
            if(cp>len) cp=len;                     // l>len already rejected above; belt and braces
            memcpy(temp, src, cp);
            temp[cp] = 0;
        }
        SReceiveData->Delete(0);
    }
    else if(t==HType.BINARY_TYPE || t==HType.UINT_1_TYPE)
    {
        unsigned char *temp;
        temp = (unsigned char *)P;
        for(i=0; i<len; i++)
        {
            if(SReceiveData->Count!=0)
            {
                temp[i] = (unsigned char)atoi(SReceiveData->Strings[0].c_str());
                SReceiveData->Delete(0);
            }
        }
    }
    else if(t==HType.BOOLEAN_TYPE)
    {
        bool *temp;
        temp = (bool *)P;
        for(i=0; i<len; i++)
        {
            if(SReceiveData->Count!=0)
            {
                temp[i] = (atoi(SReceiveData->Strings[0].c_str())==1);
                SReceiveData->Delete(0);
            }
        }
    }
    else if(t==HType.INT_1_TYPE)
    {
        char *temp;
        temp = (char *)P;
        for(i=0; i<len; i++)
        {
            if(SReceiveData->Count!=0)
            {
                temp[i] = (char)atoi(SReceiveData->Strings[0].c_str());
                SReceiveData->Delete(0);
            }
        }
    }
    else if(t==HType.INT_2_TYPE)
    {
        short *temp;
        temp = (short *)P;
        for(i=0; i<len; i++)
        {
            if(SReceiveData->Count!=0)
            {
                temp[i] = (short)atoi(SReceiveData->Strings[0].c_str());
                SReceiveData->Delete(0);
            }
        }
    }
    else if(t==HType.INT_4_TYPE)
    {
        int *temp;
        temp = (int *)P;
        for(i=0; i<len; i++)
        {
            if(SReceiveData->Count!=0)
            {
                temp[i] = atoi(SReceiveData->Strings[0].c_str());
                SReceiveData->Delete(0);
            }
        }
    }
    else if(t==HType.INT_8_TYPE)
    {
        __int64 *temp;
        temp = (__int64 *)P;
        for(i=0; i<len; i++)
        {
            if(SReceiveData->Count!=0)
            {
                temp[i] = _atoi64(SReceiveData->Strings[0].c_str());
                SReceiveData->Delete(0);
            }
        }
    }
    else if(t==HType.UINT_2_TYPE)
    {
        unsigned short *temp;
        temp = (unsigned short *)P;
        for(i=0; i<len; i++)
        {
            if(SReceiveData->Count!=0)
            {
                temp[i] = (unsigned short)atoi(SReceiveData->Strings[0].c_str());
                SReceiveData->Delete(0);
            }
        }
    }
    else if(t==HType.UINT_4_TYPE)
    {
        unsigned *temp;
        temp = (unsigned *)P;
        for(i=0; i<len; i++)
        {
            if(SReceiveData->Count!=0)
            {
                temp[i] = (unsigned)_atoi64(SReceiveData->Strings[0].c_str());
                SReceiveData->Delete(0);
            }
        }
    }
    else if(t==HType.UINT_8_TYPE)
    {
        unsigned __int64 *temp;
        temp = (unsigned __int64 *)P;
        for(i=0; i<len; i++)
        {
            if(SReceiveData->Count!=0)
            {
                temp[i] = (unsigned __int64)_atoi64(SReceiveData->Strings[0].c_str());
                SReceiveData->Delete(0);
            }
        }
    }
    else if(t==HType.FT_4_TYPE)
    {
        float *temp;
        temp = (float *)P;
        for(i=0; i<len; i++)
        {
            if(SReceiveData->Count!=0)
            {
                temp[i] = (float)atof(SReceiveData->Strings[0].c_str());
                SReceiveData->Delete(0);
            }
        }
    }
    else if(t==HType.FT_8_TYPE)
    {
        double *temp;
        temp = (double *)P;
        for(i=0; i<len; i++)
        {
            if(SReceiveData->Count!=0)
            {
                temp[i] = atof(SReceiveData->Strings[0].c_str());
                SReceiveData->Delete(0);
            }
        }
    }
    else
    {
        return -1;
    }
    return 1;
}
//---------------------------------------------------------------------------
// Peek next item's length+type without consuming.
//---------------------------------------------------------------------------
int THGem::GetDataItemLenAndTypeSub(int &len, unsigned char &Type)
{
    if(SReceiveData->Count<2)
        return -2;
    Type = (unsigned char)atoi(SReceiveData->Strings[0].c_str());
    len  = atoi(SReceiveData->Strings[1].c_str());
    return 1;
}
//---------------------------------------------------------------------------
// Read next item's length, verify its type == expected, consume type+len.
//---------------------------------------------------------------------------
int THGem::GetDataItemLenAndTypeAndDeleteSub(int &len, unsigned char Type)
{
    unsigned char t;
    if(SReceiveData->Count<2)
        return -2;
    t   = (unsigned char)atoi(SReceiveData->Strings[0].c_str());
    len = atoi(SReceiveData->Strings[1].c_str());
    if(t!=Type)
        return -1;
    SReceiveData->Delete(0);
    SReceiveData->Delete(0);
    return 1;
}
//---------------------------------------------------------------------------
//AI(secs-skip-item) 20260805 : consume exactly ONE item from SReceiveData and throw it away.
//  Same token arithmetic as DataItemInSub, but with no destination buffer. It exists for the two
//  early `return -1` exits of DataItemIn(int, unsigned char, AnsiString&) below, which returned
//  WITHOUT consuming anything: every token of the refused item stayed at the head of the buffer,
//  so the caller's next read hit the SAME bad item forever. In the five request loops that share
//  that idiom (S1F4 / S1F12 / S1F24 / S2F14 / S2F30) the effect was "one unreadable id turns every
//  id AFTER it into 0"; in S2F15 it turned an EAC=3 into an EAC=1.
//  Token layout produced by ProcessSML, per item:
//      LIST : [type][child count]              - children are pushed INLINE next, no value tokens
//      ASCII: [type][byte length][the string]  - ALWAYS exactly one value token, even for A[0]
//      other: [type][element count][v]..[v]    - element-count value tokens (none for a [0] item)
//  A LIST is skipped WITH ITS CHILDREN. That is the one place this deliberately differs from
//  DataItemInSub, which unwraps a list by design (every caller uses it to strip an L wrapper).
//  Here the contract is "drop the i-th item of the caller's L,n", and in SECS-II a nested list IS
//  one item - unwrapping would leave its children to be read as siblings and shift every field
//  after it, trading the leak for a misalignment. Iterative (pending counter), not recursive, so a
//  deeply nested body cannot blow the stack; pending can only be fed by tokens that exist, and a
//  truncated frame simply runs the buffer dry and returns -1.
//  Returns 1 when one whole item was dropped, -1 when the buffer ran out first.
//---------------------------------------------------------------------------
int THGem::SkipOneItem()
{
    unsigned char t;
    int l, i;
    int pending = 1;                 // items still to drop; a LIST adds its children
    AnsiString sSkip;

    if(SReceiveData==NULL || SReceiveData->Count<2)
        return -1;

    //One line per refused item. This path is never taken by a well-formed request the machine
    //can answer, so it cannot flood the log - and it is the only trace a silently dropped host
    //item leaves behind.
    sSkip.sprintf("[SECS] unreadable item dropped (type=%u len=%d) - receive stream stays aligned",
                  (unsigned)(unsigned char)atoi(SReceiveData->Strings[0].c_str()),
                  atoi(SReceiveData->Strings[1].c_str()));
    StringOut(sSkip);

    while(pending>0)
    {
        if(SReceiveData->Count<2)
            return -1;               // truncated : nothing left to stay in step with
        t = (unsigned char)atoi(SReceiveData->Strings[0].c_str());
        l = atoi(SReceiveData->Strings[1].c_str());
        SReceiveData->Delete(0);     // type token
        SReceiveData->Delete(0);     // length / element-count token
        pending--;
        if(t==HType.LIST_TYPE)
        {
            if(l>0)
                pending += l;        // children were pushed inline right after this header
            continue;
        }
        if(t==HType.ASCII_TYPE)
        {
            if(SReceiveData->Count!=0)
                SReceiveData->Delete(0);   // one value token whatever the byte length
            continue;
        }
        for(i=0; i<l && SReceiveData->Count!=0; i++)
            SReceiveData->Delete(0);
    }
    return 1;
}
//---------------------------------------------------------------------------
int THGem::DataItemIn(int Len, unsigned char Type, void *Value)
{
    int ret;
    ret = DataItemInSub(Len, Type, Value);
    if(iReturnCode==1)
        iReturnCode = ret;
    return ret;
}
//---------------------------------------------------------------------------
// ASCII / numeric read overload: result delivered as AnsiString.
//---------------------------------------------------------------------------
int THGem::DataItemIn(int Len, unsigned char t, AnsiString &Str)
{
    int ret;
    if(t==HType.ASCII_TYPE)
    {
        char *temp;
        temp = new char[Len+4];
        ret = DataItemInSub(Len, t, temp);
        if(ret==1)
        {
            temp[Len] = 0;
            Str = temp;
            iReturnCode = ret;
        }
        delete [] temp;
        temp = NULL;
        return ret;
    }
    //AI(secs-skip-item) 20260805 : a non-ASCII item whose ELEMENT COUNT is not 1 - a <U4[2]> id
    //  vector, or a zero-length <F8[0]> value - cannot be delivered through this overload. Drop the
    //  whole item BEFORE answering -1. This exit used to leave its tokens at the head of the buffer,
    //  so the caller's next read met the same item again and every field after it decoded as 0.
    if(Len!=1)
    {
        SkipOneItem();
        return -1;
    }

    if(t==HType.UINT_1_TYPE)
    {
        unsigned char P;
        ret = DataItemInSub(Len, t, &P);
        if(ret==1) { Str = (int)(unsigned)P; iReturnCode = ret; }
        return ret;
    }
    else if(t==HType.UINT_2_TYPE)
    {
        unsigned short P;
        ret = DataItemInSub(Len, t, &P);
        if(ret==1) { Str = (int)P; iReturnCode = ret; }
        return ret;
    }
    else if(t==HType.UINT_4_TYPE)
    {
        unsigned P;
        ret = DataItemInSub(Len, t, &P);
        if(ret==1) { Str = (unsigned)P; iReturnCode = ret; }
        return ret;
    }
    else if(t==HType.INT_1_TYPE)
    {
        char P;
        ret = DataItemInSub(Len, t, &P);
        if(ret==1) { Str = (int)P; iReturnCode = ret; }
        return ret;
    }
    else if(t==HType.INT_2_TYPE)
    {
        short P;
        ret = DataItemInSub(Len, t, &P);
        if(ret==1) { Str = (int)P; iReturnCode = ret; }
        return ret;
    }
    else if(t==HType.INT_4_TYPE)
    {
        int P;
        ret = DataItemInSub(Len, t, &P);
        if(ret==1) { Str = P; iReturnCode = ret; }
        return ret;
    }
    //AI(secs-ecv-types) 20260804 : FT_8 / FT_4 / BOOLEAN were missing, so every caller that reads a
    //  host value through this overload silently got an EMPTY string for a perfectly legal float or
    //  boolean item (the else below returns -1 AND leaves the item unconsumed). The S2F15 EC-write
    //  path ignored that -1 and fed "" to StrToFloatDef, i.e. a host <F8 102.5> wrote 0.0 into the
    //  tray geometry and still answered EAC=0. The value is already decoded to text by ProcessSML,
    //  so all this needs is the branch. FloatToStrF (not sprintf) because the consumer parses it
    //  back with StrToFloatDef - both honour the same locale decimal separator.
    else if(t==HType.FT_8_TYPE)
    {
        double P;
        ret = DataItemInSub(Len, t, &P);
        if(ret==1) { Str = FloatToStrF(P, ffGeneral, 15, 0); iReturnCode = ret; }
        return ret;
    }
    else if(t==HType.FT_4_TYPE)
    {
        float P;
        ret = DataItemInSub(Len, t, &P);
        if(ret==1) { Str = FloatToStrF((double)P, ffGeneral, 7, 0); iReturnCode = ret; }
        return ret;
    }
    else if(t==HType.BOOLEAN_TYPE)
    {
        bool P;
        ret = DataItemInSub(Len, t, &P);
        if(ret==1) { Str = (P ? "1" : "0"); iReturnCode = ret; }
        return ret;
    }
    else
    {
        //AI(secs-skip-item) 20260805 : a type this overload cannot render as text - BINARY, U8, I8,
        //  a LIST the caller peeked as one item, or an unknown format code. Consume it first, then
        //  report the failure, for exactly the reason spelled out at the Len!=1 exit above.
        SkipOneItem();
        return -1;
    }
}
//---------------------------------------------------------------------------
int THGem::GetDataItemLenAndType(int &Len, unsigned char &Type)
{
    int ret;
    ret = GetDataItemLenAndTypeSub(Len, Type);
    if(iReturnCode==1)
        iReturnCode = ret;
    return ret;
}
//---------------------------------------------------------------------------
int THGem::GetDataItemLenAndTypeAndDelete(int &Len, unsigned char Type)
{
    int ret;
    ret = GetDataItemLenAndTypeAndDeleteSub(Len, Type);
    if(iReturnCode==1)
        iReturnCode = ret;
    return ret;
}
//---------------------------------------------------------------------------
bool THGem::CheckSFFormatOnlyHead(AnsiString ErrStr)
{
    return true;
}
//---------------------------------------------------------------------------
