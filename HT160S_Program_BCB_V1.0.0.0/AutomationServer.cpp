//---------------------------------------------------------------------------
#include <vcl.h>
#include <IniFiles.hpp>
#include <winsock.h>
#include <stdio.h>
#pragma hdrstop

#include "AutomationServer.h"
#include "database.h"
#include "main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
static const int DEFAULT_AUTOMATION_PORT = 16060;
THT160AutomationServer *HT160AutomationServer = NULL;
//---------------------------------------------------------------------------
static AnsiString TrimText(AnsiString Text)
{
    return Text.Trim();
}
//---------------------------------------------------------------------------
__fastcall THT160AutomationServer::THT160AutomationServer(TComponent* Owner, TfMain *AMainForm)
    : TComponent(Owner)
{
//    MainForm = AMainForm;
     SocketTimer = NULL;
//    ListenSocket = INVALID_SOCKET;
//    ClientSocket = INVALID_SOCKET;
//    ServerEnabled = true;
//    WsaStarted = false;
//    HasPendingLotInfo = false;
//    LocalOnly = true;
//    ServerPort = DEFAULT_AUTOMATION_PORT;
//    LastError = "";
//    ReceiveBuffer = "";
//    PendingLotPayload = "";
//    BindAddressText = "127.0.0.1";
}
//---------------------------------------------------------------------------
__fastcall THT160AutomationServer::~THT160AutomationServer()
{
    Stop();
    if(SocketTimer != NULL)
    {
        delete SocketTimer;
        SocketTimer = NULL;
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160AutomationServer::LoadConfig()
{
    AnsiString ConfigPath = HSys.CurrentDir + AnsiString("\\system\\automation.ini");
    TIniFile *IniFile = new TIniFile(ConfigPath);

    try
    {
        ServerEnabled = IniFile->ReadBool("Automation", "Enabled", true);
        LocalOnly = IniFile->ReadBool("Automation", "LocalOnly", true);
        ServerPort = IniFile->ReadInteger("Automation", "Port", DEFAULT_AUTOMATION_PORT);
        if(ServerPort <= 0 || ServerPort > 65535)
            ServerPort = DEFAULT_AUTOMATION_PORT;
        BindAddressText = LocalOnly ? "127.0.0.1" : "0.0.0.0";
    }
    __finally
    {
        delete IniFile;
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160AutomationServer::WriteLog(AnsiString Text)
{
    AnsiString LogDir = HSys.CurrentDir + AnsiString("\\logs\\automation");
    ForceDirectories(LogDir);
    AnsiString LogPath = LogDir + AnsiString("\\automation_startup.log");
    FILE *LogFile = fopen(LogPath.c_str(), "a+");
    if(LogFile == NULL)
        return;

    AnsiString Line = FormatDateTime("yyyy-mm-dd hh:nn:ss", Now()) + AnsiString(" ") + Text + AnsiString("\n");
    fwrite(Line.c_str(), 1, Line.Length(), LogFile);
    fclose(LogFile);
}
//---------------------------------------------------------------------------
bool __fastcall THT160AutomationServer::Start()
{
    LoadConfig();
    WriteLog(AnsiString("Start enabled=") + (ServerEnabled ? "1" : "0") +
        AnsiString(" port=") + IntToStr(ServerPort) +
        AnsiString(" bind=") + BindAddressText);

    if(ServerEnabled == false)
    {
        Stop();
        UpdateAutomationLabel("Automation OFF");
        return true;
    }

    try
    {
        Stop();
        if(StartWinsock() == false)
        {
            WriteLog(LastError);
            UpdateAutomationLabel("Automation ERROR");
            return false;
        }

        ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
        if(ListenSocket == INVALID_SOCKET)
        {
            LastError = AnsiString("socket error ") + IntToStr(WSAGetLastError());
            WriteLog(LastError);
            UpdateAutomationLabel("Automation ERROR");
            return false;
        }

        int ReuseAddress = 1;
        setsockopt(ListenSocket, SOL_SOCKET, SO_REUSEADDR, (char *)&ReuseAddress, sizeof(ReuseAddress));

        u_long NonBlocking = 1;
        ioctlsocket(ListenSocket, FIONBIO, &NonBlocking);

        sockaddr_in Address;
        ZeroMemory(&Address, sizeof(Address));
        Address.sin_family = AF_INET;
        if(LocalOnly)
            Address.sin_addr.s_addr = inet_addr("127.0.0.1");
        else
            Address.sin_addr.s_addr = htonl(INADDR_ANY);
        Address.sin_port = htons((unsigned short)ServerPort);

        if(bind(ListenSocket, (sockaddr *)&Address, sizeof(Address)) == SOCKET_ERROR)
        {
            LastError = AnsiString("bind error ") + IntToStr(WSAGetLastError());
            WriteLog(LastError);
            CloseListenSocket();
            UpdateAutomationLabel("Automation ERROR");
            return false;
        }

        if(listen(ListenSocket, 1) == SOCKET_ERROR)
        {
            LastError = AnsiString("listen error ") + IntToStr(WSAGetLastError());
            WriteLog(LastError);
            CloseListenSocket();
            UpdateAutomationLabel("Automation ERROR");
            return false;
        }

        if(SocketTimer == NULL)
        {
            SocketTimer = new TTimer(Application);
            SocketTimer->Enabled = false;
            SocketTimer->Interval = 100;
            SocketTimer->OnTimer = SocketTimerTimer;
        }
        SocketTimer->Enabled = true;
        LastError = "";
        WriteLog("Listen OK");
        UpdateAutomationLabel(AnsiString("Automation:") + IntToStr(ServerPort));
        return true;
    }
    catch(Exception &ExceptionObject)
    {
        LastError = ExceptionObject.Message;
        WriteLog(AnsiString("Exception ") + LastError);
        UpdateAutomationLabel("Automation ERROR");
        return false;
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160AutomationServer::Stop()
{
    try
    {
        if(SocketTimer != NULL)
            SocketTimer->Enabled = false;
        CloseClientSocket();
        CloseListenSocket();
        if(WsaStarted)
        {
            WSACleanup();
            WsaStarted = false;
        }
    }
    catch(...)
    {
    }
}
//---------------------------------------------------------------------------
bool __fastcall THT160AutomationServer::IsActive()
{
    return (ListenSocket != INVALID_SOCKET);
}
//---------------------------------------------------------------------------
void __fastcall THT160AutomationServer::SetMainForm(TfMain *AMainForm)
{
    MainForm = AMainForm;
    WriteLog(AnsiString("SetMainForm ready=") + (MainForm != NULL ? "1" : "0"));
    if(MainForm != NULL && HasPendingLotInfo)
    {
        AnsiString Payload = PendingLotPayload;
        PendingLotPayload = "";
        HasPendingLotInfo = false;
        WriteLog("Apply pending lot info");
        SetLotInfo(Payload);
    }

    if(IsActive())
        UpdateAutomationLabel(AnsiString("Automation:") + IntToStr(ServerPort));
    else if(LastError != "")
        UpdateAutomationLabel("Automation ERROR");
}
//---------------------------------------------------------------------------
int __fastcall THT160AutomationServer::GetPort()
{
    return ServerPort;
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160AutomationServer::GetLastError()
{
    return LastError;
}
//---------------------------------------------------------------------------
void __fastcall THT160AutomationServer::UpdateAutomationLabel(AnsiString Text)
{
    (void)Text;
}
//---------------------------------------------------------------------------
void __fastcall THT160AutomationServer::EnsureMainFormAttached()
{
    if(MainForm == NULL && fMain != NULL)
        SetMainForm(fMain);
}
//---------------------------------------------------------------------------
bool __fastcall THT160AutomationServer::StartWinsock()
{
    if(WsaStarted)
        return true;

    WSADATA WsaData;
    int Result = WSAStartup(MAKEWORD(1, 1), &WsaData);
    if(Result != 0)
    {
        LastError = AnsiString("WSAStartup error ") + IntToStr(Result);
        return false;
    }

    WsaStarted = true;
    return true;
}
//---------------------------------------------------------------------------
void __fastcall THT160AutomationServer::CloseClientSocket()
{
    if(ClientSocket != INVALID_SOCKET)
    {
        closesocket(ClientSocket);
        ClientSocket = INVALID_SOCKET;
    }
    ReceiveBuffer = "";
}
//---------------------------------------------------------------------------
void __fastcall THT160AutomationServer::CloseListenSocket()
{
    if(ListenSocket != INVALID_SOCKET)
    {
        closesocket(ListenSocket);
        ListenSocket = INVALID_SOCKET;
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160AutomationServer::SocketTimerTimer(TObject *Sender)
{
    AcceptClient();
    ReadClient();
}
//---------------------------------------------------------------------------
void __fastcall THT160AutomationServer::AcceptClient()
{
    if(ListenSocket == INVALID_SOCKET)
        return;

    sockaddr_in ClientAddress;
    int ClientAddressLength = sizeof(ClientAddress);
    SOCKET NewClientSocket = accept(ListenSocket, (sockaddr *)&ClientAddress, &ClientAddressLength);

    if(NewClientSocket == INVALID_SOCKET)
    {
        int LastSocketError = WSAGetLastError();
        if(LastSocketError != WSAEWOULDBLOCK)
            LastError = AnsiString("accept error ") + IntToStr(LastSocketError);
        return;
    }

    u_long NonBlocking = 1;
    ioctlsocket(NewClientSocket, FIONBIO, &NonBlocking);

    if(ClientSocket != INVALID_SOCKET)
    {
        AnsiString BusyText = "ERR|BUSY|ONE_CLIENT_ONLY\r\n";
        send(NewClientSocket, BusyText.c_str(), BusyText.Length(), 0);
        closesocket(NewClientSocket);
        return;
    }

    ClientSocket = NewClientSocket;
    ReceiveBuffer = "";
    SendClientText("OK|CONNECTED|HT160S_BCB");
}
//---------------------------------------------------------------------------
void __fastcall THT160AutomationServer::ReadClient()
{
    if(ClientSocket == INVALID_SOCKET)
        return;

    char Buffer[1025];
    int ReceiveLength = recv(ClientSocket, Buffer, 1024, 0);
    if(ReceiveLength > 0)
    {
        Buffer[ReceiveLength] = '\0';
        ReceiveBuffer += AnsiString(Buffer);

        while(ReceiveBuffer.Pos("\n") > 0)
        {
            int LineEndPos = ReceiveBuffer.Pos("\n");
            AnsiString Request = ReceiveBuffer.SubString(1, LineEndPos - 1);
            ReceiveBuffer.Delete(1, LineEndPos);
            SendClientText(ProcessCommand(Request));
        }

        if(ReceiveBuffer.Length() > 2048)
        {
            SendClientText("ERR|LINE_TOO_LONG");
            ReceiveBuffer = "";
        }
        return;
    }

    if(ReceiveLength == 0)
    {
        CloseClientSocket();
        return;
    }

    int LastSocketError = WSAGetLastError();
    if(LastSocketError != WSAEWOULDBLOCK)
    {
        LastError = AnsiString("recv error ") + IntToStr(LastSocketError);
        CloseClientSocket();
    }
}
//---------------------------------------------------------------------------
bool __fastcall THT160AutomationServer::SendClientText(AnsiString Text)
{
    if(ClientSocket == INVALID_SOCKET)
        return false;

    AnsiString SendText = Text + "\r\n";
    int SendResult = send(ClientSocket, SendText.c_str(), SendText.Length(), 0);
    if(SendResult == SOCKET_ERROR)
    {
        int LastSocketError = WSAGetLastError();
        if(LastSocketError != WSAEWOULDBLOCK)
        {
            LastError = AnsiString("send error ") + IntToStr(LastSocketError);
            CloseClientSocket();
        }
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160AutomationServer::ProcessCommand(AnsiString Request)
{
    EnsureMainFormAttached();

    Request = TrimText(Request);
    if(Request == "")
        return "ERR|EMPTY_COMMAND";

    int SpacePos = Request.Pos(" ");
    AnsiString Command = Request;
    AnsiString Payload = "";

    if(SpacePos > 0)
    {
        Command = Request.SubString(1, SpacePos - 1);
        Payload = TrimText(Request.SubString(SpacePos + 1, Request.Length() - SpacePos));
    }

    Command = Command.UpperCase();

    if(Command == "PING")
        return "OK|PONG|HT160S_BCB";
    if(Command == "HELP")
        return "OK|HELP|PING;GET_STATUS;GET_LOT_INFO;SET_LOT_INFO key=value;...;SMOKE_TOP_FORMS";
    if(Command == "GET_STATUS")
        return BuildStatusReply();
    if(Command == "GET_LOT_INFO")
        return BuildLotInfoReply();
    if(Command == "SET_LOT_INFO")
        return SetLotInfo(Payload);
    if(Command == "SMOKE_TOP_FORMS")
        return SmokeProbeTopForms();

    return AnsiString("ERR|UNKNOWN_COMMAND|") + EncodeValue(Command);
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160AutomationServer::BuildStatusReply()
{
    AnsiString Running = "0";
    if(HSys.Sys.SystemStart)
        Running = "1";

    AnsiString LotNo = "";
    if(MainForm != NULL && MainForm->edLotNo != NULL)
        LotNo = MainForm->edLotNo->Text;

    return AnsiString("OK|STATUS|RUNNING=") + Running +
           ";RUN_MODE=" + IntToStr((int)HSys.Sys.RunMode) +
            ";REAL_DUMMY=" + IntToStr(HSys.LastSet.iRealDummy) +
            ";START_MODE=" + IntToStr(HSys.LastSet.iStartMode) +
           ";PORT=" + IntToStr(ServerPort) +
           ";PROCESS_ID=" + IntToStr((int)GetCurrentProcessId()) +
           ";BIND=" + BindAddressText +
           ";MAIN_FORM_READY=" + (MainForm != NULL ? "1" : "0") +
           ";FMAIN_READY=" + (fMain != NULL ? "1" : "0") +
           ";PENDING_LOT=" + (HasPendingLotInfo ? "1" : "0") +
           ";LOT_NO=" + EncodeValue(LotNo);
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160AutomationServer::BuildLotInfoReply()
{
    if(MainForm == NULL)
        return "ERR|MAIN_FORM_NOT_READY";

    return AnsiString("OK|LOT_INFO|LOT_NO=") + EncodeValue(MainForm->edLotNo->Text) +
           ";WAFER_LOT=" + EncodeValue(MainForm->edWaferLot->Text) +
           ";CUS_DEVICE=" + EncodeValue(MainForm->edCusDevice->Text) +
           ";INSERTION=" + EncodeValue(MainForm->edInsertion->Text) +
           ";FLOW_ID=" + EncodeValue(MainForm->edFlowID->Text) +
           ";OPERATOR=" + EncodeValue(MainForm->edOperator->Text) +
           ";RUN_CARD=" + EncodeValue(MainForm->edtRunCard->Text);
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160AutomationServer::SetLotInfo(AnsiString Payload)
{
    Payload = TrimText(Payload);
    if(Payload == "")
        return "ERR|EMPTY_LOT_INFO";

    if(MainForm == NULL)
    {
        PendingLotPayload = Payload;
        HasPendingLotInfo = true;
        return "OK|LOT_INFO_PENDING|MAIN_FORM_NOT_READY";
    }
    if(HSys.Sys.SystemStart)
        return "ERR|SYSTEM_RUNNING";

    AnsiString Value;

    Value = GetPayloadValue(Payload, "LOT_NO");
    if(Value == "")
        Value = GetPayloadValue(Payload, "LOTNO");
    if(Value != "")
        MainForm->edLotNo->Text = Value;

    Value = GetPayloadValue(Payload, "WAFER_LOT");
    if(Value == "")
        Value = GetPayloadValue(Payload, "WAFERLOT");
    if(Value != "")
        MainForm->edWaferLot->Text = Value;

    Value = GetPayloadValue(Payload, "CUS_DEVICE");
    if(Value == "")
        Value = GetPayloadValue(Payload, "DEVICE");
    if(Value != "")
        MainForm->edCusDevice->Text = Value;

    Value = GetPayloadValue(Payload, "INSERTION");
    if(Value != "")
        MainForm->edInsertion->Text = Value;

    Value = GetPayloadValue(Payload, "FLOW_ID");
    if(Value == "")
        Value = GetPayloadValue(Payload, "FLOWID");
    if(Value != "")
        MainForm->edFlowID->Text = Value;

    Value = GetPayloadValue(Payload, "OPERATOR");
    if(Value == "")
        Value = GetPayloadValue(Payload, "OPERATOR_ID");
    if(Value != "")
        MainForm->edOperator->Text = Value;

    Value = GetPayloadValue(Payload, "RUN_CARD");
    if(Value == "")
        Value = GetPayloadValue(Payload, "RUNCARD");
    if(Value != "")
        MainForm->edtRunCard->Text = Value;

    return BuildLotInfoReply();
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160AutomationServer::SmokeProbeTopForms()
{
    if(MainForm == NULL)
        return "ERR|MAIN_FORM_NOT_READY";
    if(HSys.Sys.SystemStart)
        return "ERR|SYSTEM_RUNNING";

    AnsiString OpenedForms;
    AnsiString ErrorText;
    if(MainForm->SmokeProbeTopForms(OpenedForms, ErrorText))
        return AnsiString("OK|TOP_FORMS|OPENED=") + EncodeValue(OpenedForms);

    return AnsiString("ERR|TOP_FORMS|") + EncodeValue(ErrorText);
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160AutomationServer::GetPayloadValue(AnsiString Payload, AnsiString Key)
{
    AnsiString WorkText = Payload;
    Key = Key.UpperCase();

    while(WorkText.Length() > 0)
    {
        int SplitPos = WorkText.Pos(";");
        AnsiString Item;
        if(SplitPos > 0)
        {
            Item = WorkText.SubString(1, SplitPos - 1);
            WorkText.Delete(1, SplitPos);
        }
        else
        {
            Item = WorkText;
            WorkText = "";
        }

        int EqualPos = Item.Pos("=");
        if(EqualPos <= 0)
            continue;

        AnsiString Name = TrimText(Item.SubString(1, EqualPos - 1)).UpperCase();
        AnsiString Value = Item.SubString(EqualPos + 1, Item.Length() - EqualPos);
        if(Name == Key)
            return DecodeValue(TrimText(Value));
    }

    return "";
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160AutomationServer::EncodeValue(AnsiString Value)
{
    Value = StringReplace(Value, "%", "%25", TReplaceFlags() << rfReplaceAll);
    Value = StringReplace(Value, ";", "%3B", TReplaceFlags() << rfReplaceAll);
    Value = StringReplace(Value, "=", "%3D", TReplaceFlags() << rfReplaceAll);
    Value = StringReplace(Value, "|", "%7C", TReplaceFlags() << rfReplaceAll);
    return Value;
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160AutomationServer::DecodeValue(AnsiString Value)
{
    Value = StringReplace(Value, "%7C", "|", TReplaceFlags() << rfReplaceAll);
    Value = StringReplace(Value, "%3D", "=", TReplaceFlags() << rfReplaceAll);
    Value = StringReplace(Value, "%3B", ";", TReplaceFlags() << rfReplaceAll);
    Value = StringReplace(Value, "%25", "%", TReplaceFlags() << rfReplaceAll);
    return Value;
}
//---------------------------------------------------------------------------
bool __fastcall InitializeHT160Automation(TfMain *MainForm)
{
    if(HT160AutomationServer == NULL)
        HT160AutomationServer = new THT160AutomationServer(Application, MainForm);
    else
        HT160AutomationServer->SetMainForm(MainForm);

    return HT160AutomationServer->Start();
}
//---------------------------------------------------------------------------
void __fastcall ShutdownHT160Automation()
{
    if(HT160AutomationServer != NULL)
    {
        delete HT160AutomationServer;
        HT160AutomationServer = NULL;
    }
}
//---------------------------------------------------------------------------
