//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#include <IniFiles.hpp>
#include <stdio.h>
#pragma hdrstop

#include "MCUDisplay.h"
#include "MCUDisplayProtocol.h"
#include "database.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
static const int MCU_SOCKET_CLOSED = 0;
static const int MCU_SOCKET_CONNECTING = 1;
static const int MCU_SOCKET_CONNECTED = 2;
static const int DEFAULT_MCU_PORT = 7000;
static const int DEFAULT_MCU_MAX_QUEUE = 500;
static const int DEFAULT_MCU_RECONNECT_MS = 3000;
//---------------------------------------------------------------------------
THT160MCUDisplay *HT160MCUDisplay = NULL;
//---------------------------------------------------------------------------
__fastcall THT160MCUDisplay::THT160MCUDisplay(TComponent *Owner)
    : TComponent(Owner)
{
    CommandQueue = new TStringList;
    ClientSocket = INVALID_SOCKET;
    DisplayEnabled = true;
    Started = false;
    WsaStarted = false;
    SocketState = MCU_SOCKET_CLOSED;
    ServerHost = "127.0.0.1";
    ServerPort = DEFAULT_MCU_PORT;
    MaxQueueCount = DEFAULT_MCU_MAX_QUEUE;
    ReconnectIntervalMs = DEFAULT_MCU_RECONNECT_MS;
    NextConnectTick = 0;
    LastErrorText = "";
    ActiveCommand = "";
    ActiveSendOffset = 0;
}
//---------------------------------------------------------------------------
__fastcall THT160MCUDisplay::~THT160MCUDisplay()
{
    Stop();
    delete CommandQueue;
    CommandQueue = NULL;
}
//---------------------------------------------------------------------------
void __fastcall THT160MCUDisplay::LoadConfig()
{
    AnsiString ConfigPath = HSys.CurrentDir + AnsiString("\\system\\MCU.ini");
    TIniFile *IniFile = new TIniFile(ConfigPath);

    try
    {
        DisplayEnabled = IniFile->ReadBool("Setup", "Enabled", true);
        ServerHost = IniFile->ReadString("Setup", "IP", "127.0.0.1");
        ServerPort = IniFile->ReadInteger("Setup", "Port", DEFAULT_MCU_PORT);
        MaxQueueCount = IniFile->ReadInteger("Setup", "MaxQueue", DEFAULT_MCU_MAX_QUEUE);
        ReconnectIntervalMs = IniFile->ReadInteger("Setup", "ReconnectIntervalMs", DEFAULT_MCU_RECONNECT_MS);
    }
    __finally
    {
        delete IniFile;
    }

    if(ServerPort <= 0 || ServerPort > 65535)
        ServerPort = DEFAULT_MCU_PORT;
    if(MaxQueueCount < 1)
        MaxQueueCount = DEFAULT_MCU_MAX_QUEUE;
    if(ReconnectIntervalMs < 500)
        ReconnectIntervalMs = 500;
    if(ServerHost == "")
        ServerHost = "127.0.0.1";
}
//---------------------------------------------------------------------------
void __fastcall THT160MCUDisplay::WriteLog(AnsiString Text)
{
    AnsiString LogDir = HSys.LogRootDir + AnsiString("\\mcu");
    ForceDirectories(LogDir);
    AnsiString LogPath = LogDir + AnsiString("\\mcu_display.log");
    FILE *LogFile = fopen(LogPath.c_str(), "a+");
    if(LogFile == NULL)
        return;

    AnsiString LineText = FormatDateTime("yyyy-mm-dd hh:nn:ss", Now()) +
        AnsiString(" ") + Text + AnsiString("\n");
    fwrite(LineText.c_str(), 1, LineText.Length(), LogFile);
    fclose(LogFile);
}
//---------------------------------------------------------------------------
bool __fastcall THT160MCUDisplay::StartWinsock()
{
    if(WsaStarted)
        return true;

    WSADATA WsaData;
    int Result = WSAStartup(MAKEWORD(1, 1), &WsaData);
    if(Result != 0)
    {
        LastErrorText = AnsiString("WSAStartup error ") + IntToStr(Result);
        WriteLog(LastErrorText);
        return false;
    }

    WsaStarted = true;
    return true;
}
//---------------------------------------------------------------------------
bool __fastcall THT160MCUDisplay::Start()
{
    LoadConfig();
    Started = true;
    WriteLog(AnsiString("Start enabled=") + (DisplayEnabled ? "1" : "0") +
        AnsiString(" ip=") + ServerHost + AnsiString(" port=") + IntToStr(ServerPort));

    if(DisplayEnabled == false)
    {
        CloseSocket();
        if(WsaStarted)
        {
            WSACleanup();
            WsaStarted = false;
        }
        return true;
    }

    return StartWinsock();
}
//---------------------------------------------------------------------------
void __fastcall THT160MCUDisplay::Stop()
{
    CloseSocket();
    ActiveCommand = "";
    ActiveSendOffset = 0;
    if(CommandQueue != NULL)
        CommandQueue->Clear();
    if(WsaStarted)
    {
        WSACleanup();
        WsaStarted = false;
    }
    SocketState = MCU_SOCKET_CLOSED;
    Started = false;
}
//---------------------------------------------------------------------------
void __fastcall THT160MCUDisplay::CloseSocket()
{
    if(ClientSocket != INVALID_SOCKET)
    {
        closesocket(ClientSocket);
        ClientSocket = INVALID_SOCKET;
    }
    SocketState = MCU_SOCKET_CLOSED;
}
//---------------------------------------------------------------------------
bool __fastcall THT160MCUDisplay::IsTickDue(DWORD NowTick, DWORD DueTick)
{
    return ((long)(NowTick - DueTick) >= 0);
}
//---------------------------------------------------------------------------
void __fastcall THT160MCUDisplay::TryConnect()
{
    if(DisplayEnabled == false || WsaStarted == false)
        return;
    if(SocketState != MCU_SOCKET_CLOSED)
        return;

    DWORD NowTick = GetTickCount();
    if(IsTickDue(NowTick, NextConnectTick) == false)
        return;

    ClientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(ClientSocket == INVALID_SOCKET)
    {
        LastErrorText = AnsiString("socket error ") + IntToStr(WSAGetLastError());
        WriteLog(LastErrorText);
        NextConnectTick = NowTick + ReconnectIntervalMs;
        return;
    }

    u_long NonBlocking = 1;
    ioctlsocket(ClientSocket, FIONBIO, &NonBlocking);

    sockaddr_in Address;
    ZeroMemory(&Address, sizeof(Address));
    Address.sin_family = AF_INET;
    Address.sin_addr.s_addr = inet_addr(ServerHost.c_str());
    Address.sin_port = htons((unsigned short)ServerPort);

    if(Address.sin_addr.s_addr == INADDR_NONE)
    {
        LastErrorText = AnsiString("invalid ip ") + ServerHost;
        WriteLog(LastErrorText);
        CloseSocket();
        NextConnectTick = NowTick + ReconnectIntervalMs;
        return;
    }

    int ConnectResult = connect(ClientSocket, (sockaddr *)&Address, sizeof(Address));
    if(ConnectResult == 0)
    {
        SocketState = MCU_SOCKET_CONNECTED;
        LastErrorText = "";
        WriteLog(AnsiString("Connect OK ") + ServerHost + AnsiString(":") + IntToStr(ServerPort));
        return;
    }

    int SocketError = WSAGetLastError();
    if(SocketError == WSAEWOULDBLOCK || SocketError == WSAEINPROGRESS || SocketError == WSAEALREADY)
    {
        SocketState = MCU_SOCKET_CONNECTING;
        return;
    }

    LastErrorText = AnsiString("connect error ") + IntToStr(SocketError);
    WriteLog(LastErrorText);
    CloseSocket();
    NextConnectTick = NowTick + ReconnectIntervalMs;
}
//---------------------------------------------------------------------------
void __fastcall THT160MCUDisplay::CheckConnectResult()
{
    if(SocketState != MCU_SOCKET_CONNECTING || ClientSocket == INVALID_SOCKET)
        return;

    fd_set WriteSet;
    fd_set ErrorSet;
    FD_ZERO(&WriteSet);
    FD_ZERO(&ErrorSet);
    FD_SET(ClientSocket, &WriteSet);
    FD_SET(ClientSocket, &ErrorSet);

    timeval TimeValue;
    TimeValue.tv_sec = 0;
    TimeValue.tv_usec = 0;

    int SelectResult = select(0, NULL, &WriteSet, &ErrorSet, &TimeValue);
    if(SelectResult <= 0)
        return;

    int SocketError = 0;
    int OptionLength = sizeof(SocketError);
    getsockopt(ClientSocket, SOL_SOCKET, SO_ERROR, (char *)&SocketError, &OptionLength);

    if(SocketError == 0 && FD_ISSET(ClientSocket, &WriteSet))
    {
        SocketState = MCU_SOCKET_CONNECTED;
        LastErrorText = "";
        WriteLog(AnsiString("Connect OK ") + ServerHost + AnsiString(":") + IntToStr(ServerPort));
        return;
    }

    LastErrorText = AnsiString("connect error ") + IntToStr(SocketError);
    WriteLog(LastErrorText);
    CloseSocket();
    NextConnectTick = GetTickCount() + ReconnectIntervalMs;
}
//---------------------------------------------------------------------------
void __fastcall THT160MCUDisplay::SendNextCommand()
{
    if(SocketState != MCU_SOCKET_CONNECTED || ClientSocket == INVALID_SOCKET)
        return;

    if(ActiveCommand == "")
    {
        if(CommandQueue->Count <= 0)
            return;
        ActiveCommand = CommandQueue->Strings[0];
        ActiveSendOffset = 0;
        CommandQueue->Delete(0);
    }

    int RemainLength = ActiveCommand.Length() - ActiveSendOffset;
    if(RemainLength <= 0)
    {
        ActiveCommand = "";
        ActiveSendOffset = 0;
        return;
    }

    const char *SendPointer = ActiveCommand.c_str() + ActiveSendOffset;
    int SendResult = send(ClientSocket, SendPointer, RemainLength, 0);
    if(SendResult > 0)
    {
        ActiveSendOffset += SendResult;
        if(ActiveSendOffset >= ActiveCommand.Length())
        {
            WriteLog(AnsiString("Send ") + THT160MCUDisplayProtocol::CommandToHex(ActiveCommand));
            ActiveCommand = "";
            ActiveSendOffset = 0;
        }
        return;
    }

    int SocketError = WSAGetLastError();
    if(SocketError == WSAEWOULDBLOCK)
        return;

    LastErrorText = AnsiString("send error ") + IntToStr(SocketError);
    WriteLog(LastErrorText);
    CloseSocket();
    NextConnectTick = GetTickCount() + ReconnectIntervalMs;
}
//---------------------------------------------------------------------------
void __fastcall THT160MCUDisplay::Spin()
{
    if(Started == false)
        Start();
    if(DisplayEnabled == false || WsaStarted == false)
        return;

    TryConnect();
    CheckConnectResult();
    SendNextCommand();
}
//---------------------------------------------------------------------------
bool __fastcall THT160MCUDisplay::QueueCommand(AnsiString CommandText)
{
    if(DisplayEnabled == false)
        return false;
    if(CommandText == "")
        return false;

    while(CommandQueue->Count >= MaxQueueCount)
    {
        CommandQueue->Delete(0);
        WriteLog("Queue full, drop oldest command");
    }

    CommandQueue->Add(CommandText);
    return true;
}
//---------------------------------------------------------------------------
bool __fastcall THT160MCUDisplay::IsConnected()
{
    return (SocketState == MCU_SOCKET_CONNECTED);
}
//---------------------------------------------------------------------------
bool __fastcall THT160MCUDisplay::IsEnabled()
{
    return DisplayEnabled;
}
//---------------------------------------------------------------------------
int __fastcall THT160MCUDisplay::GetQueueCount()
{
    if(CommandQueue == NULL)
        return 0;
    return CommandQueue->Count + (ActiveCommand != "" ? 1 : 0);
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160MCUDisplay::GetLastError()
{
    return LastErrorText;
}
//---------------------------------------------------------------------------
void __fastcall THT160MCUDisplay::SetBinCode(int Address, AnsiString CodeText, bool SymbolType)
{
    if(Address < 0)
        return;

    QueueCommand(THT160MCUDisplayProtocol::BuildBinCodeCommand(Address, CodeText, SymbolType));
}
//---------------------------------------------------------------------------
void __fastcall THT160MCUDisplay::SetBinLight(int Address, int LightValue)
{
    if(Address < 0)
        return;

    QueueCommand(THT160MCUDisplayProtocol::BuildBinLightCommand(Address, LightValue));
}
//---------------------------------------------------------------------------
void __fastcall THT160MCUDisplay::SetBinDisplay(int Address, AnsiString Text, AnsiString ColorText)
{
    AnsiString CodeCommand;
    AnsiString LightCommand;
    if(THT160MCUDisplayProtocol::BuildBinDisplayCommands(Address, Text, ColorText,
        CodeCommand, LightCommand) == false)
    {
        return;
    }

    QueueCommand(CodeCommand);
    QueueCommand(LightCommand);
}
//---------------------------------------------------------------------------
void __fastcall EnsureMCUDisplayCreated(TComponent *Owner)
{
    if(HT160MCUDisplay != NULL)
        return;
    if(Owner == NULL)
        Owner = Application;
    HT160MCUDisplay = new THT160MCUDisplay(Owner);
    HT160MCUDisplay->Start();
}
//---------------------------------------------------------------------------
void __fastcall ShutdownHT160MCUDisplay()
{
    if(HT160MCUDisplay != NULL)
    {
        delete HT160MCUDisplay;
        HT160MCUDisplay = NULL;
    }
}
//---------------------------------------------------------------------------
void __fastcall SpinMCUDisplay()
{
    EnsureMCUDisplayCreated(Application);
    if(HT160MCUDisplay != NULL)
        HT160MCUDisplay->Spin();
}
//---------------------------------------------------------------------------
void __fastcall SetMCUBinCode(int Address, AnsiString CodeText, bool SymbolType)
{
    EnsureMCUDisplayCreated(Application);
    if(HT160MCUDisplay != NULL)
        HT160MCUDisplay->SetBinCode(Address, CodeText, SymbolType);
}
//---------------------------------------------------------------------------
void __fastcall SetMCUBinLight(int Address, int LightValue)
{
    EnsureMCUDisplayCreated(Application);
    if(HT160MCUDisplay != NULL)
        HT160MCUDisplay->SetBinLight(Address, LightValue);
}
//---------------------------------------------------------------------------
void __fastcall SetMCUBinDisplay(int Address, AnsiString Text, AnsiString ColorText)
{
    EnsureMCUDisplayCreated(Application);
    if(HT160MCUDisplay != NULL)
        HT160MCUDisplay->SetBinDisplay(Address, Text, ColorText);
}
//---------------------------------------------------------------------------