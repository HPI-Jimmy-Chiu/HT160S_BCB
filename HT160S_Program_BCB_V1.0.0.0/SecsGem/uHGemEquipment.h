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
struct TGemReportItem
{
    unsigned ReportID;
    int SVCount;
    unsigned SVIDs[64];
};
struct TGemCEIDItem
{
    unsigned CEID;
    int ReportCount;
    unsigned ReportIDs[32];
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
    unsigned int EquipmentSystemByte;   // outgoing primary message SystemByte counter
    unsigned int DeviceID;              // configured HSMS DeviceID

    //AI(ht160s-secsgem) 20260611 : SV/EC/CEID/Report registry (form-less).
    TList *SVList;       // TGemSVItem*
    TList *ECList;       // TGemECItem*
    TList *ReportList;   // TGemReportItem*
    TList *CEIDList;     // TGemCEIDItem*
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
    int  DataItemInSub(int len, unsigned char Type, void *P);
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
    void DoReconnectAttempt();        // re-dial (active) or re-listen (passive)

    //AI(ht160s-secsgem) 20260611 : HSMS Linktest heartbeat + T6 timeout so a
    //  silently-dropped peer (no clean FIN / RST) is detected and recovered.
    int  iLinktestInterval;           // seconds between Linktest.req when SELECTED (0=off)
    int  iLinktestCountdown;          // seconds left until next Linktest.req
    int  iT6Timeout;                  // seconds to wait for Linktest.rsp
    int  iT6Countdown;                // seconds left while awaiting Linktest.rsp
    bool bAwaitLinktestRsp;           // a Linktest.req is outstanding
    bool bLogLinktest;                // log routine Linktest req/rsp (default off : avoid log flooding)
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
    bool IsEnableEvent(unsigned iDataID, unsigned iCeid);
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

    void InitLocalHead(int Stream, int Function, int WaitBit);
    void DataItemOut(int Len, unsigned char Type, void *Value);
    void DataItemOut(unsigned char Type, AnsiString Text);   // ASCII convenience overload
    void SendLocalData();
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
