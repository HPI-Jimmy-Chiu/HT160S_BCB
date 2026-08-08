//---------------------------------------------------------------------------
#ifndef uHGemEquipmentH
#define uHGemEquipmentH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <ExtCtrls.hpp>
#include <SysUtils.hpp>
#include <ScktComp.hpp>   //AI(ht160s-secsgem) 20260610 : TClientSocket/TServerSocket (vclie.bpi)
//---------------------------------------------------------------------------
struct HTypeStruct
{
    unsigned char LIST_TYPE;
    unsigned char ASCII_TYPE;
    unsigned char JIS_TYPE;
    unsigned char BINARY_TYPE;
    unsigned char BOOLEAN_TYPE;
    unsigned char UINT_1_TYPE;
    unsigned char UINT_2_TYPE;
    unsigned char UINT_4_TYPE;
    unsigned char UINT_8_TYPE;
    unsigned char INT_1_TYPE;
    unsigned char INT_2_TYPE;
    unsigned char INT_4_TYPE;
    unsigned char INT_8_TYPE;
    unsigned char FT_4_TYPE;
    unsigned char FT_8_TYPE;
    unsigned char VCL_TYPE;
};
//---------------------------------------------------------------------------
extern HTypeStruct HType;
//---------------------------------------------------------------------------
// HSMS 10-byte message header (MSB order on the wire).  Ported from HT172.
struct HSMS_Head_Struct
{
    unsigned short int DeviceID;
    unsigned char MessageID_S;
    unsigned char MessageID_F;
    unsigned W_Bit;
    unsigned char PType;
    unsigned char SType;
    unsigned int SystemByte;
};
//---------------------------------------------------------------------------
class HTGem;   //AI(ht160s-secsgem) 20260610 : forward decl for dispatch back-pointer
//---------------------------------------------------------------------------
// HSMS SType (control message) codes - SEMI E37.
#define HSMS_STYPE_DATA        0
#define HSMS_STYPE_SELECT_REQ  1
#define HSMS_STYPE_SELECT_RSP  2
#define HSMS_STYPE_DESELECT_REQ 3
#define HSMS_STYPE_DESELECT_RSP 4
#define HSMS_STYPE_LINKTEST_REQ 5
#define HSMS_STYPE_LINKTEST_RSP 6
#define HSMS_STYPE_REJECT_REQ  7
#define HSMS_STYPE_SEPARATE_REQ 9
// HSMS connection state.
#define HSMS_STATE_NOTCONNECTED 0
#define HSMS_STATE_CONNECTED    1
#define HSMS_STATE_SELECTED     2
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : Form-less SV/EC/CEID/Report registration model.
//  HT172 kept these in GUI TStringGrid; HT160 THGem has no form, so the registry
//  lives in heap structs held by TList.  SetSVDataPointer / SetECDataPointer /
//  SetReportIDContent / SetCEIDContent populate them; EventReport() (S6F11) and
//  S1F3/S1F4 read them back to encode live SECS-II values.
//  Convention: ASCII SV/EC store Ptr as AnsiString*; numeric store &value.
struct TGemSVItem
{
    unsigned SVID;
    unsigned char Type;     // HType.* code
    int Len;                // element count for numeric arrays (default 1)
    void *Ptr;              // ASCII -> AnsiString*; numeric -> &value
    AnsiString Name;
    AnsiString Unit;
    AnsiString Remark;
};
struct TGemECItem
{
    unsigned ECID;
    unsigned char Type;
    int Len;
    void *Ptr;
    AnsiString Name;
    AnsiString Unit;
    AnsiString Remark;
    AnsiString MinValue;
    AnsiString MaxValue;
    AnsiString DefaultValue;
};
//AI(secs-audit-fix) 20260729 : per-report SVID capacity. Was a bare 64, which made
//ProcessDefineReport_S2F33 reject the WHOLE S2F33 with DRACK=0x01 whenever any report
//carried more than 64 SVIDs. On the KYEC 2026-06-08 traffic that hit 8 of 122 messages -
//RPTID 800 (103/104 SVIDs) and RPTID 505 (179 SVIDs) - so those two reports were never
//defined, S2F35 silently skipped them from every CEID link, and the S6F16 reply came back
//structurally short. HT9045 answered DRACK=0x00 to all 122. 192 covers the largest report
//seen in the field with headroom; keep the reports-per-message cap at 64 (max seen: 3).
#define GEM_MAX_SVID_PER_REPORT   192
//AI(secs-e5-lrack) 20260805 : S2F37 CEID-list capacity. Was a bare 256 in
//ProcessEnableDisableEventReport_S2F37 while this machine's dictionary holds 292 ids, so a host
//that enumerated every CEID after an S1F23 full query had the whole packet rejected with
//ERACK=0x02. 512 clears the dictionary with headroom. HT9045 has no guard at all here (a bare
//unsigned CEID[1024] filled straight from the wire), so raising the cap moves toward it, not away.
//The array it sizes is file-scope static inside that handler, NOT a stack frame - 512*4 = 2 KB of
//BSS, no BCB6 stack concern. Keep the n>cap test BEFORE the fill loop.
#define GEM_MAX_CEID_PER_S2F37    512
struct TGemReportItem
{
    unsigned ReportID;
    int SVCount;
    unsigned SVIDs[GEM_MAX_SVID_PER_REPORT];
    int Mode;   //AI(secs-reportdef) 20260724 : 0=host-defined, 1=firmware-default
};
struct TGemCEIDItem
{
    unsigned CEID;
    int ReportCount;
    unsigned ReportIDs[32];
    int Mode;   //AI(secs-reportdef) 20260724 : 0=host-defined, 1=firmware-default
    bool Enabled;   //AI(secs-reportdef) 20260724 : per-CEID S6F11 enable (S2F37); default true
    AnsiString Alias;
};
//---------------------------------------------------------------------------
class THGem : public TComponent
{
private:
    int iTimeFormat;
    AnsiString sDefaultAddress;
    AnsiString sDefaultPort;
    AnsiString sDeviceId;
    AnsiString sRecipeDirectory;
    AnsiString sRecipeFileMask;
    int iRecipeDirectoryType;
    AnsiString sMachineType;
    AnsiString sSoftwareVersion;
    TStringList *LogList;
    //AI(ht160s-secsgem) 20260611 : disk-logging buffer kept SEPARATE from LogList
    //  so the GUI monitor's DrainLog() (which clears LogList) cannot steal lines
    //  destined for the on-disk SECS log.  Aligned with HT172 LogDataString.
    TStringList *LogFileBuffer;
    bool         bLogToFile;          // gated by CosFunction.bUseSecsGem + [SECS] LogToFile
    // -- Phase 0 transmit-side SECS-II encode codec (ported from HT172) --
    // Self-contained byte encoder. Builds a complete HSMS-framed message
    // (4-byte length + 10-byte header + SECS-II body) into LocalBuffer.
    // Transport (socket SendBuf) is NOT wired yet - see SendLocalData().
    HSMS_Head_Struct Local;             // header currently being built
    HSMS_Head_Struct Remote;            // last received header (for reply SystemByte)
    unsigned char *LocalBuffer;         // heap-allocated encode buffer
    unsigned LocalBufferSize;           // capacity of LocalBuffer
    unsigned LocalLength;               // body length (bytes after the 4-byte len field)
    unsigned LocalLength_4;             // running write cursor (starts at 4)
    //AI(secs-audit-fix) 20260729 : set by DataItemOut when an item will not fit LocalBuffer,
    //cleared by InitLocalHead at the start of every message, checked by SendLocalData. Dropping
    //the item alone would leave a frame whose LIST header declares more items than it carries -
    //a malformed body can desync the host's parser for the rest of the session, which is worse
    //than no reply at all. Poison the whole message instead and say so in the log.
    bool bEncodeOverflow;               // current message overflowed the encode buffer
    unsigned int EquipmentSystemByte;   // outgoing primary message SystemByte counter
    unsigned int DeviceID;              // configured HSMS DeviceID

    //AI(ht160s-secsgem) 20260611 : SV/EC/CEID/Report registry (form-less).
    TList *SVList;       // TGemSVItem*
    TList *ECList;       // TGemECItem*
    TList *ReportList;   // TGemReportItem*
    TList *CEIDList;     // TGemCEIDItem*
    bool bHostManagesReports;   //AI(secs-reportdef) 20260724 : false until first successful S2F37; enable-gate inert while false
    bool bEventDefLoaded;       //AI(secs-reportdef) 20260724 : true after ReadEventReportData; Save is a no-op before
    TGemSVItem     *FindSVItem(unsigned SVID);
    TGemECItem     *FindECItem(unsigned ECID);
    TGemReportItem *FindReportItem(unsigned ReportID);
    TGemCEIDItem   *FindCEIDItem(unsigned CEID);
    void DataItemOutSVItem(TGemSVItem *Item);   // encode one SV's live value
    void DataItemOutECItem(TGemECItem *Item);   // encode one EC's live value
    //AI(ht160s-secsgem) 20260611 : render a bound SV/EC value as display text (GUI).
    AnsiString ItemValueToString(unsigned char Type, void *Ptr);

    int  GetLengthOfType(unsigned char Type);
    unsigned char GetLengthByte(unsigned len, unsigned char *Ptr);
    void ConvertLocalData(int len, void *Value);
    void CreateLocalHead();

    // -- Phase 0 receive-side SECS-II decode codec (ported from HT172) --
    // Tokenizes a received HSMS frame body into SReceiveData (a flat list of
    // Type / length / value strings) so DataItemIn / GetDataItemLenAndType*
    // can read it sequentially.  Socket transport that fills the frame buffer
    // is NOT wired yet - DecodeReceiveBody() is the entry the future receive
    // loop will call.
    TStringList *SReceiveData;          // tokenized received items
    bool bReceiveData;                  // true while storing a received frame
    int  iReturnCode;                   // cumulative read status (1=ok sticky-on-error)

    int  GetSMLLenthByte(unsigned char TypeChar, unsigned char *Ptr, int RunLength);
    void StoreToReceiveString(AnsiString S);
    int  ProcessSML(unsigned char *Ptr, int Len, int &RunLength);
    //AI(ht160s-secsgem) 20260716 : read-only SML pretty-printer for the full body.
    //  Walks raw frame bytes (never touches SReceiveData), renders one item as an
    //  indented SML subtree into Out. 0=ok, <0=bounds/format error (Out keeps the
    //  partial decode). LogSmlBody() wraps it with a header + hex fallback on error.
    int  RenderSmlItem(unsigned char *Ptr, int Len, int &RunLength, int Depth, AnsiString &Out);
    void LogSmlBody(const char *Dir, unsigned char *Ptr, int Len, int S, int F, int W);
    int  DataItemInSub(int len, unsigned char Type, void *P);
    //AI(secs-skip-item) 20260805 : drop ONE whole item (type + length + value tokens, and for a
    //  LIST its children too) from SReceiveData without decoding it. Used by the two reject
    //  exits of the DataItemIn AnsiString overload, which used to return WITHOUT consuming the
    //  item they refused - so the caller's next read met the very same bad item again.
    int  SkipOneItem();
    int  GetDataItemLenAndTypeSub(int &len, unsigned char &Type);
    int  GetDataItemLenAndTypeAndDeleteSub(int &len, unsigned char Type);
    void ProcessRemoteHead(unsigned char *Ptr);

    // -- Phase 0 HSMS-SS socket transport (route B: TComponent lean engine) --
    // Programmatic TClientSocket (active) / TServerSocket (passive). Assembles
    // length-prefixed frames, auto-answers Select/Linktest/Separate control
    // messages, and dispatches complete data messages into GemLogic.
    HTGem *GemLogic;                  // GEM logic layer for S/F dispatch
    TClientSocket *ClientSocket1;     // active mode connector
    TServerSocket *ServerSocket1;     // passive mode listener (equipment default)
    TCustomWinSocket *ActiveSocket;   // currently connected peer
    TMemoryStream *RecvBuffer;        // partial-frame assembly buffer
    //AI(secs-audit-fix) 20260729 : set by OnPeerDisconnected, read by ProcessReceiveBuffer.
    //A handler running inside ProcessReceiveBuffer's loop can drop the link (Separate.req, or
    //any SendLocalData that raises a socket error), and OnPeerDisconnected does
    //RecvBuffer->Clear() - which RELEASES the block that loop cached in its local "base".
    //Without this flag the loop kept walking freed memory and then memmove'd into it.
    bool bRecvBufferReset;            // RecvBuffer was flushed under our feet
    bool bActiveMode;                 // true=active(client), false=passive(server)
    bool bCommStarted;                // StartCommunication() has run
    int  iHsmsState;                  // HSMS_STATE_*

    //AI(ht160s-secsgem) 20260611 : periodic reconnect watchdog (Timer1Timer).
    //  bWantComm = user intent to stay connected (set by Start/StopCommunication).
    //  iReconnectInterval = seconds between attempts (General.ini [SECS], 0=off).
    bool bWantComm;
    int  iReconnectInterval;          // seconds between reconnect attempts (0=disabled)
    int  iReconnectCountdown;         // seconds left until next attempt
    int  iReconnectAttempts;          // attempts since StartCommunication()
    //AI(ht160s-secsgem) 20260801 : yyyymmdd of the last "no host connected" marker, so at
    //  most ONE such line is written per calendar day per process run. Passive mode has
    //  nothing to retry (see DoReconnectAttempt), so there is no periodic logging at all;
    //  this marker exists only to guarantee the day's SECS log folder gets created, which
    //  is what makes a State Record from a host-less day distinguishable from a broken log.
    int  iIdleLogDay;
    void DoReconnectAttempt();        // re-dial (active) or re-listen (passive)

    //AI(ht160s-secsgem) 20260611 : HSMS Linktest heartbeat + T6 timeout so a
    //  silently-dropped peer (no clean FIN / RST) is detected and recovered.
    int  iLinktestInterval;           // seconds between Linktest.req when SELECTED (0=off)
    int  iLinktestCountdown;          // seconds left until next Linktest.req
    int  iT6Timeout;                  // seconds to wait for Linktest.rsp
    int  iT6Countdown;                // seconds left while awaiting Linktest.rsp
    bool bAwaitLinktestRsp;           // a Linktest.req is outstanding
    bool bLogLinktest;                // log routine Linktest req/rsp (default off : avoid log flooding)
    bool bLogSmlBody;                 //AI(ht160s-secsgem) 20260716 : dump full SECS-II body as SML tree (RX+TX, default on)
    unsigned uControlSystemByte;      // SystemBytes generator for our control msgs
    void SendLinktestReq();           // actively send Linktest.req (heartbeat)
    void DropConnection(AnsiString Reason);  // close socket + OnPeerDisconnected

    //AI(ht160s-secsgem) 20260611 : SECS communication file logging. Folder layout
    //  aligned with HT172: D:\HT160S_Log\SECS_GEM\yyyy_mm_dd\ (per-hour text
    //  file + daily error file).  Root comes from HSys.LogRootDir.
    AnsiString BuildSecsLogDir();     // ensure + return today's SECS_GEM\yyyy_mm_dd dir

    void OnPeerConnected(TCustomWinSocket *Socket);
    void OnPeerDisconnected();
    void ReadFromPeer(TCustomWinSocket *Socket);
    void ProcessReceiveBuffer();
    void HandleControlMessage(unsigned char *Ptr, int Len);
    void HandleDataMessage(unsigned char *Ptr, int Len);
    void SendControlReply(unsigned char *ReqPtr, unsigned char ReplySType);

    void __fastcall ClientConnect(TObject *Sender, TCustomWinSocket *Socket);
    void __fastcall ClientDisconnect(TObject *Sender, TCustomWinSocket *Socket);
    void __fastcall ClientRead(TObject *Sender, TCustomWinSocket *Socket);
    void __fastcall ClientError(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode);
    void __fastcall ServerClientConnect(TObject *Sender, TCustomWinSocket *Socket);
    void __fastcall ServerClientDisconnect(TObject *Sender, TCustomWinSocket *Socket);
    void __fastcall ServerClientRead(TObject *Sender, TCustomWinSocket *Socket);
    void __fastcall ServerClientError(TObject *Sender, TCustomWinSocket *Socket, TErrorEvent ErrorEvent, int &ErrorCode);

    void __fastcall Timer1Timer(TObject *Sender);
public:
    TTimer *Timer1;
    AnsiString Alias;
    AnsiString CurrentDirectory;

    __fastcall THGem(TComponent *Owner);
    __fastcall ~THGem();

    void SetTimeFormat(int Format);
    void SetDefaultAddressAndPort(char *Address, char *Port);
    void SetDefaultAddressAndPort(char *Address, char *Port, char *DeviceId);
    int SetReceipeDirectoryAndGlobalName(AnsiString Path, AnsiString FileMask, int Type);
    void SetMachineTypeAndSoftwarseVer(char *MachineType, char *SoftwareVersion);
    void SaveEventReportData();
    void EventReport(unsigned iDataID, unsigned iCeid);
    //AI(ht160s-secsgem) 20260625 : S5F1 alarm report sender (from primitives, like EventReport).
    void SendAlarmS5F1(unsigned alid, unsigned char alcd, AnsiString altx);
    //AI(secs-msggap) 20260728 : host-pull report encoders for S6F15->S6F16 / S6F19->S6F20.
    //Bodies live here (not on HT160Gem) because FindCEIDItem / FindReportItem are private -
    //same delegate pattern as ProcessDefineReport_S2F33 <- S2F34_DefineReportAcknowledge.
    //Pure encoders : RefreshSVData + SendLocalData + one log line, no gates, no state change.
    void EmitEventReportBody(int Func, unsigned iDataID, unsigned iCeid);
    void EmitIndividualReport(unsigned ReportID);
    bool IsEnableEvent(unsigned iDataID, unsigned iCeid);
    //AI(secs-s2f37-visibility) 20260808 : how many CEIDs the host currently has ENABLED, out of
    //how many exist, and whether it has taken control of reporting at all. A host that answers
    //its own init with "disable every CEID" and never enables anything silences the whole tool
    //(KYEC did exactly that in all 8 sessions on 2026-08-07, dropping 567 S6F11), and until now
    //that state was only visible by reading a wire log. Exposed so the maintenance SECS tab can
    //show it next to the link badge.
    int  GetCeidCount();
    int  GetEnabledCeidCount();
    bool IsHostManagingReports();
    void StringOut(AnsiString Text);
    //AI(ht160s-secsgem) 20260611 : move accumulated log lines into Dest and clear
    //  the internal buffer. Used by the GUI log monitor (uHGemLogForm) to pull
    //  live SECS TX/RX trace without exposing the internal TStringList.
    void DrainLog(TStrings *Dest);

    //AI(ht160s-secsgem) 20260611 : registry read-back for S1F3/S1F4 status query.
    void DataItemOutSVValue(unsigned SVID);   // encode one registered SV's live value
    int  GetSVCount();
    unsigned GetSVIDByIndex(int Index);
    //AI(ht160s-secsgem) 20260611 : registry read-back for S2F13/S2F14 EC query.
    void DataItemOutECValue(unsigned ECID);   // encode one registered EC's live value
    int  GetECCount();
    unsigned GetECIDByIndex(int Index);
    //AI(ht160s-secsgem) 20260611 : S1F12 namelist support (name/unit by SVID).
    AnsiString GetSVName(unsigned SVID);
    AnsiString GetSVUnit(unsigned SVID);
    //AI(ht160s-secsgem) 20260611 : S2F15 EC write - convert host text to the EC's
    //  registered type and store into its bound pointer. 0=ok, 1=ECID unknown, 3=range.
    int WriteECValueByString(unsigned ECID, AnsiString sValue);

    //AI(ht160s-secsgem) 20260611 : GUI monitor read-back (SV/EC query+edit tabs).
    AnsiString GetSVValueString(unsigned SVID);   // bound SV live value as text
    AnsiString GetECName(unsigned ECID);
    AnsiString GetECUnit(unsigned ECID);
    AnsiString GetECValueString(unsigned ECID);   // bound EC live value as text
    //AI(secs-namelist) 20260730 : S2F30 EC namelist - declared limits + SECS type by ECID.
    //  The three limits are stored as TEXT at registration (SetECDataPointer) and are
    //  encoded in the EC's OWN type on the wire, so they need both getters and GetECType.
    AnsiString GetECMinValue(unsigned ECID);
    AnsiString GetECMaxValue(unsigned ECID);
    AnsiString GetECDefaultValue(unsigned ECID);
    unsigned char GetECType(unsigned ECID);
    bool IsValidECID(unsigned ECID);
    //AI(secs-namelist) 20260730 : encode ONE scalar of the given SECS type from text.
    //  Empty text -> zero-length item of that type ("no limit declared"), never a LIST.
    void DataItemOutTypedText(unsigned char Type, AnsiString sValue);
    //AI(secs-namelist) 20260730 : S1F24 CEID namelist support. GetCEIDVidList flattens the
    //  CEID -> linked reports -> SVIDs chain here because FindCEIDItem / FindReportItem are
    //  private (same reason the S6F16 / S6F20 encoders live on THGem).
    int  GetCEIDCount();
    unsigned GetCEIDByIndex(int Index);
    AnsiString GetCEIDAlias(unsigned CEID);
    bool IsValidCEID(unsigned CEID);
    int  GetCEIDVidList(unsigned CEID, unsigned *Out, int MaxOut);
    void RefreshSVSnapshot();                      // GemLogic->RefreshSVData() for live SV
    AnsiString GetEndpointAddress();               // configured HSMS address (read-only view)
    AnsiString GetEndpointPort();
    AnsiString GetDeviceIdText();
    bool IsActiveMode();

    void SetSVDataPointer(unsigned SVID, unsigned char Type, AnsiString Name, AnsiString Unit, void *DataPtr, AnsiString Description);
    void SetECDataPointer(unsigned ECID, unsigned char Type, AnsiString Name, AnsiString Unit, void *DataPtr, AnsiString MinValue, AnsiString MaxValue, AnsiString DefaultValue, AnsiString Description);
    void SetAlamData(int Index, AnsiString AlarmCode, AnsiString UnitName, AnsiString Message, AnsiString AlarmType);
    void ReadAlamData();
    void WriteAlamData();
    void ReadEventReportData();
    void SetCEIDContent(unsigned iCeid, unsigned iReportCount, unsigned *iReportIDData, int Mode);
    void SetCEIDContent(unsigned iCeid, AnsiString CeidAlias, unsigned iReportCount, unsigned *iReportIDData, int Mode);
    bool SetReportIDContent(unsigned iReportID, unsigned iReportCount, unsigned *iReportIDData, int Mode);
    //AI(secs-reportdef) 20260724 : S2F33/F35/F37 host report-definition (bodies on THGem; HT160Gem virtuals delegate)
    void ProcessDefineReport_S2F33();
    void ProcessLinkEventReport_S2F35();
    void ProcessEnableDisableEventReport_S2F37();
    void ReportAcknowledge(unsigned char DRACK);
    void LinkReportAcknowledge(unsigned char LRACK);
    void EnableDisableEventReportAcknowledge(unsigned char ERACK);
    bool IsValidSVID(unsigned SVID);
    void DeleteHostReport(unsigned ReportID);
    void UnlinkReportFromAllCeids(unsigned ReportID);
    void DeleteAllHostReports();

    void InitLocalHead(int Stream, int Function, int WaitBit);
    void DataItemOut(int Len, unsigned char Type, void *Value);
    void DataItemOut(unsigned char Type, AnsiString Text);   // ASCII convenience overload
    void SendLocalData();
    //AI(secs-e30-gate) 20260803 : SxF0 Abort Transaction, the E30 refusal for a host primary the
    // equipment must not act on in its current control state. Header-only reply; reuses the
    // refused transaction's SystemByte because Function 0 is even. Must live on THGem : Remote
    // (which holds that SystemByte) is private here.
    void SendAbort(int Stream);
    int DataItemIn(int Len, unsigned char Type, void *Value);
    int DataItemIn(int Len, unsigned char Type, AnsiString &Str);   // ASCII/numeric read overload
    int GetDataItemLenAndType(int &Len, unsigned char &Type);
    int GetDataItemLenAndTypeAndDelete(int &Len, unsigned char Type);
    bool CheckSFFormatOnlyHead(AnsiString ErrStr);

    // Receive-side entry: reset SReceiveData and tokenize a full HSMS frame
    // (Ptr[0..3]=length, Ptr[4..13]=header, Ptr[14..]=SECS-II body).
    int  DecodeReceiveBody(unsigned char *Ptr, int Len);
    void ResetReturnCode();
    int  GetReturnCode();

    // -- HSMS transport control (route B) --
    void SetGemLogic(HTGem *p);       // wire dispatch back-pointer
    void SetHsmsMode(bool bActive);   // true=active(client), false=passive(server)
    void StartCommunication();        // open socket (listen or connect)
    void StopCommunication();         // close socket
    bool IsConnected();
    bool IsSelected();

    //AI(ht160s-secsgem) 20260611 : reconnect watchdog config + status (GUI/INI).
    void SetReconnectInterval(int Seconds);   // <=0 disables auto-reconnect
    int  GetReconnectInterval();
    int  GetReconnectCountdown();
    int  GetReconnectAttempts();
    AnsiString GetReconnectStatusText();       // human text, "" when connected/off

    //AI(ht160s-secsgem) 20260611 : Linktest heartbeat / T6 timeout config.
    void SetLinktestInterval(int Seconds);     // <=0 disables heartbeat
    void SetT6Timeout(int Seconds);            // <=0 falls back to a safe minimum
    void SetLogLinktest(bool On);              //AI(ht160s-secsgem) 20260612 : show routine Linktest in log (default off)
    void SetLogSmlBody(bool On);               //AI(ht160s-secsgem) 20260716 : dump full SECS-II body as SML tree (default on)

    //AI(ht160s-secsgem) 20260611 : SECS communication on-disk logging control.
    void SetLogToFile(bool On);                // enable/disable disk logging
    void FlushSecsLogToFile();                 // append pending lines to today's TextLog
    void SaveSecsErrToLog(AnsiString Reason);  // append one line to today's ErrLog

    // Accessors for the encoded message (future HSMS transport will send these).
    const unsigned char *GetLocalBuffer();
    unsigned GetLocalLength();
};
//---------------------------------------------------------------------------
extern THGem *HGem;
//---------------------------------------------------------------------------
#endif
