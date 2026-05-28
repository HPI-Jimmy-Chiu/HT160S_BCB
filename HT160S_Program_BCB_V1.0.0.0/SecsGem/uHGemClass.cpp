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
void HTGem::SendUnsupported(AnsiString FunctionName)
{
    if(HGemPtr!=NULL)
        HGemPtr->StringOut("[SECS] " + FunctionName + " unsupported in HT160S skeleton");
}
//---------------------------------------------------------------------------
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
