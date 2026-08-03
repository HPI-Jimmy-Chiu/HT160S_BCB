//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "uHGemClass.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
HTGem::HTGem()
{
    HGemPtr = NULL;
    HandlerPath = "HT160S";
    DataPath = "..\\data";
    SecsAlarmMessage = new TStringList;
}
//---------------------------------------------------------------------------
HTGem::HTGem(THGem *GemPtr)
{
    HGemPtr = GemPtr;
    HandlerPath = "HT160S";
    DataPath = "..\\data";
    SecsAlarmMessage = new TStringList;
}
//---------------------------------------------------------------------------
HTGem::HTGem(AnsiString Path, THGem *GemPtr)
{
    HGemPtr = GemPtr;
    HandlerPath = Path;
    DataPath = "..\\data";
    SecsAlarmMessage = new TStringList;
}
//---------------------------------------------------------------------------
HTGem::~HTGem()
{
    if(SecsAlarmMessage!=NULL)
    {
        SecsAlarmMessage->Clear();
        delete SecsAlarmMessage;
        SecsAlarmMessage = NULL;
    }
}
//---------------------------------------------------------------------------
void HTGem::UpdateDataPath(AnsiString Path)
{
    DataPath = Path;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260610 : primary-message dispatch table (route B).
// Called by THGem after a complete data message is decoded into SReceiveData.
//---------------------------------------------------------------------------
void HTGem::Dispatch(int S, int F)
{
    if(HGemPtr!=NULL)
    {
        AnsiString L;
        L.sprintf("[SECS][RX] S%dF%d dispatch", S, F);
        HGemPtr->StringOut(L);
    }
    //AI(ht160s-secsgem) 20260611 : even F = secondary (host reply to our primary,
    //  e.g. S6F12 ack of S6F11). Log and drop; never answer with S9F3.
    if((F & 1)==0)
    {
        if(HGemPtr!=NULL)
        {
            AnsiString R;
            R.sprintf("[SECS][RX] S%dF%d reply ignored", S, F);
            HGemPtr->StringOut(R);
        }
        return;
    }
    switch(S)
    {
    case 1:
        switch(F)
        {
        case 1:  S1F2_OnLineData();                  return;
        case 3:  S1F4_SelectedStatusReply();         return;
        case 11: S1F12_StatusVariableNamelistReply();return;//AI(ht160s-secsgem) 20260611 : SV namelist
        case 13: S1F14_ConnectRequestAcknowledge();  return;
        case 15: S1F16_OFFLINEAcknowledge();         return;//AI(secs-offline) 20260727 : S1F15 Request OFF-LINE -> S1F16 OFLACK
        case 17: S1F18_ONLINEAcknowledge();          return;//AI(secs-online) 20260724 : S1F17 Request ONLINE -> S1F18 ONLACK
        case 23: S1F24_CollectionEventNamelist();    return;//AI(secs-namelist) 20260730 : S1F23 Collection Event Namelist Request -> S1F24
        }
        break;
    case 2:
        switch(F)
        {
        case 17: S2F18_DateandTimeData();            return;
        case 25: S2F26_DiagnosticLoopbackData();     return;
        case 31: S2F32_DateAndTimeAcknowledge();     return;
        case 41: S2F42_Host_Command_Acknowledge();   return;
        case 13: S2F14_EquipmentConstanData();       return;
        case 15: S2F16_NewEquipmentConstantSendAcknowledge(); return;//AI(ht160s-secsgem) 20260611 : EC write
        case 29: S2F30_EquipmentConstantNamelistReply();      return;//AI(secs-namelist) 20260730 : S2F29 EC Namelist Request -> S2F30
        case 33: S2F34_DefineReportAcknowledge();             return;//AI(secs-reportdef) 20260724 : Define Report
        case 35: S2F36_LinkEventReportAcknowledge();          return;//AI(secs-reportdef) 20260724 : Link Event Report
        case 37: S2F38_EnableDisableEventReportAcknowledge(); return;//AI(secs-reportdef) 20260724 : Enable/Disable Event
        }
        break;
    case 5:
        switch(F)
        {
        case 3: S5F4_EnableDisableAlarmAcknowledge(); return;
        case 5: S5F6_ListAlarmData();                 return;
        case 7: S5F8_ListEnableAlarmAcknowledge();    return;
        }
        break;
    //AI(secs-msggap) 20260728 : S6F15 Event Report Request / S6F19 Individual Report Request.
    //Both were unhandled -> fell through to S9F3_Unrecognized (log line only, ZERO bytes on
    //the wire) -> host T3 timeout. S6F17 / S6F23 stay deliberately unhandled: the KYEC host
    //has never sent them, and with only F15/F19 listed here they keep falling through to the
    //same S9F3 log they hit today, so this case adds no behaviour for them.
    case 6:
        switch(F)
        {
        case 15: S6F16_EventReportData();       return;
        case 19: S6F20_IndividualReportData();  return;
        }
        break;
    case 7:
        switch(F)
        {
        case 1:  S7F2_ProcessProgramLoadGrant();           return;
        case 3:  S7F4_ProcessProgramAcknowledge();         return;
        case 5:  S7F6_ProcessProgramData();                return;
        case 17: S7F18_DeleteProcessProgramAcknowledge();  return;
        case 19: S7F20_CurrentEPPDData();                  return;
        }
        break;
    //AI(secs-msggap) 20260728 : S10F3/S10F5 already dispatched here BEFORE this patch, but
    //landed on the base SendUnsupported stubs, which only StringOut and transmit nothing -
    //that is why the host T3s on stream 10 even though the case exists. Fixed by the
    //HT160Gem overrides, not here; this case is unchanged.
    case 10:
        switch(F)
        {
        case 3: S10F4_TerminalDisplaySingleAcknowledge();   return;
        case 5: S10F6_TerminalDisplayMultiBlockAcknowledge(); return;
        }
        break;
    case 14:
        if(F==1) { ProcessS14F1_GetAttrRequest(""); return; }
        break;
    //AI(secs-msggap) 20260728 : S125F1 Enable/Disable EC Data Send (KYEC private stream).
    //Was unhandled -> S9F3 log only -> host T3. Exactly ONE S125F2 is sent per request.
    //HT9045 sends one ack PER ECID inside its parse loop, so the KYEC captures show 46
    //secondaries for 2 primaries per session (3 x (1 + 45) = 138 vs 6). Because
    //InitLocalHead reuses the request SystemByte for every even Function, those 45 replies
    //all carry the SAME transaction id - a protocol defect, not a feature. Not ported.
    case 125:
        if(F==1) { S125F2_EnableDisableECDataAcknowledge(); return; }
        break;
    }
    // Unhandled primary message -> report unrecognized S/F.
    AnsiString E;
    E.sprintf("S%dF%d", S, F);
    S9F3_Unrecognized_Stream_Function_Type(E);
}
//---------------------------------------------------------------------------
void HTGem::SendUnsupported(AnsiString FunctionName)
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] " + FunctionName + " unsupported in HT160S skeleton");
}
//---------------------------------------------------------------------------
void HTGem::RefreshSVData(){ }
//AI(ht160s-secsgem) 20260612 : base no-op; HT160Gem overrides to drive the main-screen SECS badge.
void HTGem::RefreshSecsBadge(){ }
//AI(ht160s-agv) 20260615 : base no-op; HT160Gem overrides to drive the AGV coordinator.
void HTGem::ServiceAgv(){ }
void HTGem::PollGemControlState(){ }   //AI(secs-controlstate) 20260803
//AI(secs-kyec-rcmd4-fix) 20260728 : base no-op; HT160Gem overrides to release latched host state.
void HTGem::OnCommunicationLost(){ }
//AI(secs-lotstarttime) 20260730 : base no-op; HT160Gem overrides to latch SVID 66033.
void HTGem::NoteLotStartTime(bool /*bStarted*/, AnsiString /*sWhen*/){ }
void HTGem::ReportSkipICCount(int /*iCount*/){ }   //AI(secs-skipiccount) 20260802
void HTGem::AddSV(){ }
void HTGem::AddEC(){ }
void HTGem::AddAlarmList(){ }
void HTGem::AddCEID(){ }
void HTGem::AddReprot(){ }
void HTGem::LookForFile(){ }
void HTGem::ReloadParameter(){ }
//---------------------------------------------------------------------------
void HTGem::S1F1_AreYouThereRequest(){ SendUnsupported("S1F1"); }
void HTGem::S1F2_OnLineData(){ SendUnsupported("S1F2"); }
void HTGem::S1F4_SelectedStatusReply(){ SendUnsupported("S1F4"); }
void HTGem::S1F12_StatusVariableNamelistReply(){ SendUnsupported("S1F12"); }
void HTGem::S1F13_EstablishCommunicationsRequest(){ SendUnsupported("S1F13"); }
void HTGem::S1F14_ConnectRequestAcknowledge(){ SendUnsupported("S1F14"); }
void HTGem::Process_S1F14_ConnectRequestAcknowledge(){ SendUnsupported("Process_S1F14"); }
void HTGem::S1F16_OFFLINEAcknowledge(){ SendUnsupported("S1F16"); }
void HTGem::S1F18_ONLINEAcknowledge(){ SendUnsupported("S1F18"); }
void HTGem::S1F24_CollectionEventNamelist(){ SendUnsupported("S1F24"); }
void HTGem::S2F14_EquipmentConstanData(){ SendUnsupported("S2F14"); }
int HTGem::S2F15_UpdateNewEquipmentConstant(){ SendUnsupported("S2F15"); return 1; }
int HTGem::S2F15_CheckNewEquipmentConstant(){ SendUnsupported("S2F15"); return 1; }
void HTGem::S2F16_NewEquipmentConstantSendAcknowledge(){ SendUnsupported("S2F16"); }
void HTGem::S2F18_DateandTimeData(){ SendUnsupported("S2F18"); }
void HTGem::S2F24_TraceInitializeAcknowledge(){ SendUnsupported("S2F24"); }
void HTGem::S2F26_DiagnosticLoopbackData(){ SendUnsupported("S2F26"); }
void HTGem::S2F30_EquipmentConstantNamelistReply(){ SendUnsupported("S2F30"); }
void HTGem::S2F32_DateAndTimeAcknowledge(){ SendUnsupported("S2F32"); }
void HTGem::S2F34_DefineReportAcknowledge(){ SendUnsupported("S2F34"); }
void HTGem::S2F36_LinkEventReportAcknowledge(){ SendUnsupported("S2F36"); }
void HTGem::S2F38_EnableDisableEventReportAcknowledge(){ SendUnsupported("S2F38"); }
//---------------------------------------------------------------------------
int HTGem::S2F42_Host_Command_Acknowledge()
{
    SendUnsupported("S2F42");
    return 1;
}
//---------------------------------------------------------------------------
void HTGem::S2F44_ResetSpoolingAcknowledge(){ SendUnsupported("S2F44"); }
void HTGem::S5F4_EnableDisableAlarmAcknowledge(){ SendUnsupported("S5F4"); }
void HTGem::S5F6_ListAlarmData(){ SendUnsupported("S5F6"); }
void HTGem::S5F8_ListEnableAlarmAcknowledge(){ SendUnsupported("S5F8"); }
void HTGem::S6F16_EventReportData(){ SendUnsupported("S6F16"); }
void HTGem::S6F18_AnnotatedEventReportData(){ SendUnsupported("S6F18"); }
void HTGem::S6F20_IndividualReportData(){ SendUnsupported("S6F20"); }
void HTGem::S6F24_RequestSpooledDataAcknowledgementSend(){ SendUnsupported("S6F24"); }
//---------------------------------------------------------------------------
int HTGem::S7F2_ProcessProgramLoadGrant()
{
    SendUnsupported("S7F2");
    return 1;
}
//---------------------------------------------------------------------------
void HTGem::S7F4_ProcessProgramAcknowledge(){ SendUnsupported("S7F4"); }
void HTGem::S7F6_ProcessProgramData(){ SendUnsupported("S7F6"); }
void HTGem::S7F6_ProcessProgramData(AnsiString FileName){ SendUnsupported("S7F6 " + FileName); }
void HTGem::S7F18_DeleteProcessProgramAcknowledge(){ SendUnsupported("S7F18"); }
void HTGem::S7F20_CurrentEPPDData(){ SendUnsupported("S7F20"); }
void HTGem::Process_S7F20_CurrentEPPIDData(){ SendUnsupported("Process_S7F20"); }
//---------------------------------------------------------------------------
int HTGem::S7F24_FormattedProcessProgramSendAcknowledge()
{
    SendUnsupported("S7F24");
    return 1;
}
//---------------------------------------------------------------------------
int HTGem::S7F26_FormattedProcessProgramData()
{
    SendUnsupported("S7F26");
    return 1;
}
//---------------------------------------------------------------------------
void HTGem::S10F4_TerminalDisplaySingleAcknowledge(){ SendUnsupported("S10F4"); }
void HTGem::S10F6_TerminalDisplayMultiBlockAcknowledge(){ SendUnsupported("S10F6"); }
//---------------------------------------------------------------------------
void HTGem::ProcessS14F1_GetAttrRequest(AnsiString asTrayID)
{
    SendUnsupported("S14F1 " + asTrayID);
}
//---------------------------------------------------------------------------
unsigned char HTGem::ProcessS14F2_GetAttrData()
{
    SendUnsupported("S14F2");
    return 1;
}
//---------------------------------------------------------------------------
void HTGem::S100F4_ReportAllAlarm(){ SendUnsupported("S100F4"); }
void HTGem::S101F2_CurrentEPPDData(){ SendUnsupported("S101F2"); }
void HTGem::S101F4_CurrentEPPDData(){ SendUnsupported("S101F4"); }
void HTGem::S101F6(){ SendUnsupported("S101F6"); }
void HTGem::S101F8(){ SendUnsupported("S101F8"); }
void HTGem::S103F12_StatusVariableNamelistReply(){ SendUnsupported("S103F12"); }
void HTGem::S125F2_EnableDisableECDataAcknowledge(){ SendUnsupported("S125F2"); }
void HTGem::S125F4_LevelSettingChangeAcknowledge(){ SendUnsupported("S125F4"); }
//---------------------------------------------------------------------------
void HTGem::S9F3_Unrecognized_Stream_Function_Type(AnsiString ErrStr)
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] S9F3 " + ErrStr);
}
//---------------------------------------------------------------------------
