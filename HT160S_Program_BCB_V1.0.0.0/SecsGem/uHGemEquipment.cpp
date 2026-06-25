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

    //AI(ht160s-secsgem) 20260610 : Phase 0 HSMS-SS socket transport init
    GemLogic     = NULL;
    ActiveSocket = NULL;
    bActiveMode  = false;                    // default passive (equipment listens)
    bCommStarted = false;
    iHsmsState   = HSMS_STATE_NOTCONNECTED;
    RecvBuffer   = new TMemoryStream;

    //AI(ht160s-secsgem) 20260611 : reconnect watchdog defaults (overridden by
    // General.ini [SECS] ReconnectInterval via SetReconnectInterval in GemInitial).
    bWantComm          = false;
    iReconnectInterval = 5;                  // seconds; 0 = disabled
    iReconnectCountdown= 0;
    iReconnectAttempts = 0;

    //AI(ht160s-secsgem) 20260611 : Linktest heartbeat / T6 timeout defaults
    // (overridden by General.ini [SECS] LinktestInterval / T6Timeout).
    iLinktestInterval  = 10;                 // seconds; 0 = heartbeat off
    iLinktestCountdown = 0;
    iT6Timeout         = 6;                   // seconds to wait for Linktest.rsp
    bLogLinktest       = false;               //AI(ht160s-secsgem) 20260612 : quiet heartbeat by default
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
    int port = atoi(sDefaultPort.c_str());
    if(port <= 0)
        port = 5000;
    //AI(ht160s-secsgem) 20260611 : cap the attempt counter so a machine left
    //  running for weeks without a host cannot overflow a signed int.
    if(iReconnectAttempts < 1000000000)
        iReconnectAttempts++;
    AnsiString S;
    try
    {
        if(bActiveMode)
        {
            if(ClientSocket1 != NULL)
            {
                ClientSocket1->Active  = false;       // drop any half-open socket
                ClientSocket1->Address = sDefaultAddress;
                ClientSocket1->Port    = port;
                ClientSocket1->Active  = true;        // re-dial host
            }
            S.sprintf("[SECS] reconnect #%d (active dial %s:%d)",
                      iReconnectAttempts, sDefaultAddress.c_str(), port);
        }
        else
        {
            if(ServerSocket1 != NULL && !ServerSocket1->Active)
            {
                ServerSocket1->Port   = port;
                ServerSocket1->Active = true;         // re-open listen socket
            }
            S.sprintf("[SECS] reconnect #%d (passive listen :%d)",
                      iReconnectAttempts, port);
        }
        StringOut(S);
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
    StringOut("[SECS] SaveEventReportData placeholder");
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
        return;
    }
    unsigned uAlid = alid;
    InitLocalHead(5, 1, 1);
    DataItemOut(3, HType.LIST_TYPE, NULL);
    DataItemOut(1, HType.BINARY_TYPE, &alcd);
    DataItemOut(1, HType.UINT_4_TYPE, &uAlid);
    DataItemOut(HType.ASCII_TYPE, altx);
    SendLocalData();
}
//---------------------------------------------------------------------------
bool THGem::IsEnableEvent(unsigned iDataID, unsigned iCeid)
{
    return true;
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
        CEIDList->Add(p);
    }
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
        ReportList->Add(p);
    }
    int n = (int)iReportCount;
    if(n<0) n=0;
    if(n>64) n=64;
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
void THGem::DataItemOut(int len, unsigned char Type, void *P)
{
    int i, k;
    unsigned char SMLLength;
    unsigned char SMLLengthData[3];
    unsigned char DataSize;

    DataSize = (unsigned char)GetLengthOfType(Type);
    SMLLength = GetLengthByte((unsigned)(len*DataSize), SMLLengthData);
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
    StringOut("[SECS] peer disconnected");
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
        iHsmsState = HSMS_STATE_SELECTED;
        StringOut("[SECS] Select.req -> Select.rsp (SELECTED)");
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
    OnPeerDisconnected();
}
void __fastcall THGem::ServerClientRead(TObject *Sender, TCustomWinSocket *Socket)
{
    ReadFromPeer(Socket);
}
void __fastcall THGem::ServerClientError(TObject *Sender, TCustomWinSocket *Socket,
                                         TErrorEvent ErrorEvent, int &ErrorCode)
{
    AnsiString S;
    S.sprintf("[SECS] server socket error event=%d code=%d", (int)ErrorEvent, ErrorCode);
    StringOut(S);
    ErrorCode = 0;   // suppress VCL exception
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
        ItemSize = GetSMLLenthByte(TypeChar, Ptr, RunLength);
        RunLength += TypeChar & 0x03;
        if(c==HType.ASCII_TYPE)
        {
            StoreToReceiveString((int)c);
            StoreToReceiveString(ItemSize);
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
        strncpy(temp, SReceiveData->Strings[0].c_str(), len+1);
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
    if(Len!=1)
        return -1;

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
    else
    {
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
