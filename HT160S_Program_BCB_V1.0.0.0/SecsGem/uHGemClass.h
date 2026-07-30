//---------------------------------------------------------------------------
#ifndef uHGemClassH
#define uHGemClassH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <SysUtils.hpp>
#include "uHGemEquipment.h"
//---------------------------------------------------------------------------
class HTGem
{
protected:
    void SendUnsupported(AnsiString FunctionName);
public:
    THGem *HGemPtr;
    AnsiString HandlerPath;
    AnsiString DataPath;
    TStringList *SecsAlarmMessage;

    HTGem();
    HTGem(THGem *GemPtr);
    HTGem(AnsiString Path, THGem *GemPtr);
    virtual ~HTGem();

    void UpdateDataPath(AnsiString Path);

    //AI(ht160s-secsgem) 20260610 : route an incoming primary message (Stream S,
    // Function F) decoded by THGem to the matching virtual handler.
    virtual void Dispatch(int S, int F);

    //AI(ht160s-secsgem) 20260611 : refresh SV snapshot values just before THGem
    // serializes an S6F11 event report or an S1F4 status reply. Base is a no-op.
    virtual void RefreshSVData();

    //AI(ht160s-secsgem) 20260612 : called once per second from THGem::Timer1Timer
    // so the machine-specific GEM logic can sync any HSMS-link UI (e.g. the main
    // screen SECS status badge). Base is a no-op.
    virtual void RefreshSecsBadge();

    //AI(ht160s-agv) 20260615 : called once per second from THGem::Timer1Timer so
    // the machine-specific GEM logic can drive the E87/AGV coordinator (poll car
    // full -> CEID272, service handshake). Base is a no-op.
    virtual void ServiceAgv();
    //AI(secs-kyec-rcmd4-fix) 20260728 : transport -> logic notification that the HSMS link is
    // gone (peer disconnect, socket error, Separate.req, or our own DropConnection). Lets the
    // logic layer drop any latched host state that would otherwise outlive the host. Base is a
    // no-op so THGem stays free of machine dependencies.
    virtual void OnCommunicationLost();

    //AI(secs-lotstarttime) 20260730 : production -> logic notification that the current work
    // order has started (true) or ended (false), so SVID 66033 Lot Start Time can latch the
    // moment. Called from BOTH the manual Lot Start / Lot End buttons and the SECS LOTSTART
    // accept path. Latched, NOT recomputed in RefreshSVData : the host must be able to read
    // back WHEN the lot started, not what time it is now. Base is a no-op.
    //AI(secs-lotstarttime-persist) 20260730 : sWhen (already formatted) overrides "now", so a
    // power-on work-order restore can re-latch the ORIGINAL start time instead of the resume
    // moment. Empty = stamp now.
    virtual void NoteLotStartTime(bool bStarted, AnsiString sWhen="");

    virtual void AddSV();
    virtual void AddEC();
    virtual void AddAlarmList();
    virtual void AddCEID();
    virtual void AddReprot();
    virtual void LookForFile();
    virtual void ReloadParameter();

    virtual void S1F1_AreYouThereRequest();
    virtual void S1F2_OnLineData();
    virtual void S1F4_SelectedStatusReply();
    virtual void S1F12_StatusVariableNamelistReply();
    virtual void S1F13_EstablishCommunicationsRequest();
    virtual void S1F14_ConnectRequestAcknowledge();
    virtual void Process_S1F14_ConnectRequestAcknowledge();
    virtual void S1F16_OFFLINEAcknowledge();
    virtual void S1F18_ONLINEAcknowledge();
    virtual void S1F24_CollectionEventNamelist();
    virtual void S2F14_EquipmentConstanData();
    virtual int S2F15_UpdateNewEquipmentConstant();
    virtual int S2F15_CheckNewEquipmentConstant();
    virtual void S2F16_NewEquipmentConstantSendAcknowledge();
    virtual void S2F18_DateandTimeData();
    virtual void S2F24_TraceInitializeAcknowledge();
    virtual void S2F26_DiagnosticLoopbackData();
    virtual void S2F30_EquipmentConstantNamelistReply();
    virtual void S2F32_DateAndTimeAcknowledge();
    virtual void S2F34_DefineReportAcknowledge();
    virtual void S2F36_LinkEventReportAcknowledge();
    virtual void S2F38_EnableDisableEventReportAcknowledge();
    virtual int S2F42_Host_Command_Acknowledge();
    virtual void S2F44_ResetSpoolingAcknowledge();
    virtual void S5F4_EnableDisableAlarmAcknowledge();
    virtual void S5F6_ListAlarmData();
    virtual void S5F8_ListEnableAlarmAcknowledge();
    virtual void S6F16_EventReportData();
    virtual void S6F18_AnnotatedEventReportData();
    virtual void S6F20_IndividualReportData();
    virtual void S6F24_RequestSpooledDataAcknowledgementSend();
    virtual int S7F2_ProcessProgramLoadGrant();
    virtual void S7F4_ProcessProgramAcknowledge();
    virtual void S7F6_ProcessProgramData();
    virtual void S7F6_ProcessProgramData(AnsiString FileName);
    virtual void S7F18_DeleteProcessProgramAcknowledge();
    virtual void S7F20_CurrentEPPDData();
    virtual void Process_S7F20_CurrentEPPIDData();
    virtual int S7F24_FormattedProcessProgramSendAcknowledge();
    virtual int S7F26_FormattedProcessProgramData();
    virtual void S10F4_TerminalDisplaySingleAcknowledge();
    virtual void S10F6_TerminalDisplayMultiBlockAcknowledge();
    virtual void ProcessS14F1_GetAttrRequest(AnsiString asTrayID);
    virtual unsigned char ProcessS14F2_GetAttrData();
    virtual void S100F4_ReportAllAlarm();
    virtual void S101F2_CurrentEPPDData();
    virtual void S101F4_CurrentEPPDData();
    virtual void S101F6();
    virtual void S101F8();
    virtual void S103F12_StatusVariableNamelistReply();
    virtual void S125F2_EnableDisableECDataAcknowledge();
    virtual void S125F4_LevelSettingChangeAcknowledge();
    virtual void S9F3_Unrecognized_Stream_Function_Type(AnsiString ErrStr);
};
//---------------------------------------------------------------------------
#endif
