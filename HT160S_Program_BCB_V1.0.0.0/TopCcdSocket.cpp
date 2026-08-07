//---------------------------------------------------------------------------
// TopCcdSocket.cpp
// Top CCD 2D barcode camera client socket (HT160S_BCB).
// See TopCcdSocket.h for the rename map and design notes.
//---------------------------------------------------------------------------
#include <vcl.h>
#include <IniFiles.hpp>
#include <winsock.h>
#include <stdio.h>
#pragma hdrstop

#include "TopCcdSocket.h"
#include "database.h"
#include "GeneralSetting.h"   //AI(ht160s-ccd-2dsanitize) 20260807 : SanitizeScanned2D at the read source
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
static const char *DEFAULT_TOPCCD_ADDRESS = "172.16.8.89";
static const int   DEFAULT_TOPCCD_PORT    = 5001;

THT160TopCcdSocket *TopCcdSocket = NULL;
//---------------------------------------------------------------------------
void EnsureTopCcdSocketCreated()
{
    if(TopCcdSocket == NULL)
    {
        TopCcdSocket = new THT160TopCcdSocket();
        TopCcdSocket->LoadConfig();
    }
}
//---------------------------------------------------------------------------
__fastcall THT160TopCcdSocket::THT160TopCcdSocket()
{
    sckTopCcd      = INVALID_SOCKET;
    iState         = TOPCCD_IDLE;
    bWsaStarted    = false;
    bTopCcdConnect = false;
    bTopCcdReadDone= false;
    sTopCcd2D      = "";
    sRecvBuffer    = "";
    sLastError     = "";
    sCcdAddress    = DEFAULT_TOPCCD_ADDRESS;
    iCcdPort       = DEFAULT_TOPCCD_PORT;
    bWantConnected = false;
    iLastReconnectTick = 0;
}
//---------------------------------------------------------------------------
__fastcall THT160TopCcdSocket::~THT160TopCcdSocket()
{
    TopCcdDisconnect();
    if(bWsaStarted)
    {
        WSACleanup();
        bWsaStarted = false;
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160TopCcdSocket::LoadConfig()
{
    // Ship + hardware install tier: system\General.ini [TopCCD].
    AnsiString ConfigPath = HSys.CurrentDir + AnsiString("\\system\\General.ini");
    TIniFile *IniFile = new TIniFile(ConfigPath);
    try
    {
        sCcdAddress = IniFile->ReadString("TopCCD", "Address", DEFAULT_TOPCCD_ADDRESS);
        iCcdPort    = IniFile->ReadInteger("TopCCD", "Port", DEFAULT_TOPCCD_PORT);
        if(iCcdPort <= 0 || iCcdPort > 65535)
            iCcdPort = DEFAULT_TOPCCD_PORT;
    }
    __finally
    {
        delete IniFile;
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160TopCcdSocket::SetEndpoint(AnsiString sAddress, int iPort)
{
    if(sAddress.Trim() != "")
        sCcdAddress = sAddress.Trim();
    if(iPort > 0 && iPort <= 65535)
        iCcdPort = iPort;
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160TopCcdSocket::GetAddress()
{
    return sCcdAddress;
}
//---------------------------------------------------------------------------
int __fastcall THT160TopCcdSocket::GetPort()
{
    return iCcdPort;
}
//---------------------------------------------------------------------------
bool __fastcall THT160TopCcdSocket::StartWinsock()
{
    if(bWsaStarted)
        return true;

    WSADATA WsaData;
    int Result = WSAStartup(MAKEWORD(1, 1), &WsaData);
    if(Result != 0)
    {
        sLastError = AnsiString("WSAStartup error ") + IntToStr(Result);
        return false;
    }
    bWsaStarted = true;
    return true;
}
//---------------------------------------------------------------------------
void __fastcall THT160TopCcdSocket::CloseSocket()
{
    if(sckTopCcd != INVALID_SOCKET)
    {
        closesocket(sckTopCcd);
        sckTopCcd = INVALID_SOCKET;
    }
    iState         = TOPCCD_IDLE;
    bTopCcdConnect = false;
    sRecvBuffer    = "";
}
//---------------------------------------------------------------------------
bool __fastcall THT160TopCcdSocket::TopCcdConnect()
{
    //AI(HT160S-Maintainer) 20260612 : align HT172 (GAP E) - remember that a
    //connection is wanted so TopCcdPoll can auto-reconnect if the link drops.
    bWantConnected = true;
    if(iState == TOPCCD_CONNECTED || iState == TOPCCD_CONNECTING)
        return true;

    if(StartWinsock() == false)
        return false;

    sckTopCcd = socket(AF_INET, SOCK_STREAM, 0);
    if(sckTopCcd == INVALID_SOCKET)
    {
        sLastError = AnsiString("socket error ") + IntToStr(WSAGetLastError());
        return false;
    }

    u_long NonBlocking = 1;
    ioctlsocket(sckTopCcd, FIONBIO, &NonBlocking);

    sockaddr_in ServerAddress;
    memset(&ServerAddress, 0, sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port   = htons((u_short)iCcdPort);
    ServerAddress.sin_addr.s_addr = inet_addr(sCcdAddress.c_str());

    int Result = connect(sckTopCcd, (sockaddr *)&ServerAddress, sizeof(ServerAddress));
    if(Result == 0)
    {
        iState         = TOPCCD_CONNECTED;
        bTopCcdConnect = true;
        sLastError     = "";
        return true;
    }

    int LastSocketError = WSAGetLastError();
    if(LastSocketError == WSAEWOULDBLOCK || LastSocketError == WSAEINPROGRESS)
    {
        iState = TOPCCD_CONNECTING;   // completion confirmed in PollConnecting()
        return true;
    }

    sLastError = AnsiString("connect error ") + IntToStr(LastSocketError);
    CloseSocket();
    return false;
}
//---------------------------------------------------------------------------
void __fastcall THT160TopCcdSocket::TopCcdDisconnect()
{
    bWantConnected = false;   //AI(HT160S-Maintainer) 20260612 : stop auto-reconnect
    CloseSocket();
}
//---------------------------------------------------------------------------
void __fastcall THT160TopCcdSocket::PollConnecting()
{
    if(iState != TOPCCD_CONNECTING || sckTopCcd == INVALID_SOCKET)
        return;

    fd_set WriteSet;
    fd_set ExceptSet;
    FD_ZERO(&WriteSet);
    FD_ZERO(&ExceptSet);
    FD_SET(sckTopCcd, &WriteSet);
    FD_SET(sckTopCcd, &ExceptSet);

    timeval Timeout;
    Timeout.tv_sec  = 0;
    Timeout.tv_usec = 0;   // zero timeout => non-blocking probe

    int Result = select(0, NULL, &WriteSet, &ExceptSet, &Timeout);
    if(Result <= 0)
        return;            // still pending (or transient) => stay CONNECTING

    if(FD_ISSET(sckTopCcd, &ExceptSet))
    {
        sLastError = "Top CCD connect failed";
        CloseSocket();
        return;
    }

    if(FD_ISSET(sckTopCcd, &WriteSet))
    {
        iState         = TOPCCD_CONNECTED;
        bTopCcdConnect = true;
        sLastError     = "";
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160TopCcdSocket::PollReceive()
{
    if(iState != TOPCCD_CONNECTED || sckTopCcd == INVALID_SOCKET)
        return;

    char Buffer[1025];
    int ReceiveLength = recv(sckTopCcd, Buffer, 1024, 0);
    if(ReceiveLength > 0)
    {
        Buffer[ReceiveLength] = '\0';
        sRecvBuffer += AnsiString(Buffer);

        //AI(HT160S-Maintainer) 20260612 : align HT172 (GAP D) - only treat the
        //reply as a complete 2D code once a line terminator (CR or LF) arrives,
        //so a TCP-fragmented reply is not mistaken for a truncated code. Strip
        //CR/LF like HT172 Barcode.cpp; keep any bytes after the terminator.
        int iPos = sRecvBuffer.Pos("\n");
        if(iPos == 0)
            iPos = sRecvBuffer.Pos("\r");
        if(iPos > 0)
        {
            AnsiString sLine = sRecvBuffer.SubString(1, iPos);
            sLine = StringReplace(sLine, "\r", "", TReplaceFlags() << rfReplaceAll);
            sLine = StringReplace(sLine, "\n", "", TReplaceFlags() << rfReplaceAll);
            sLine = sLine.Trim();
            sRecvBuffer = sRecvBuffer.SubString(iPos + 1, sRecvBuffer.Length());
            if(sLine != "")
            {
                //AI(ht160s-ccd-2dsanitize) 20260807 : comma -> underscore at the read
                //source (no-op unless GeneralSetting.bCcd2DCommaToUnderscore). The log
                //below records the sanitized form - the form every consumer will see.
                sTopCcd2D       = GeneralSetting.SanitizeScanned2D(sLine);
                bTopCcdReadDone = true;
                SaveTopCcd2DLog(sTopCcd2D);
            }
        }
        return;
    }

    if(ReceiveLength == 0)
    {
        CloseSocket();          // peer closed
        return;
    }

    int LastSocketError = WSAGetLastError();
    if(LastSocketError != WSAEWOULDBLOCK)
    {
        sLastError = AnsiString("recv error ") + IntToStr(LastSocketError);
        CloseSocket();
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160TopCcdSocket::TopCcdPoll()
{
    if(iState == TOPCCD_CONNECTING)
        PollConnecting();
    if(iState == TOPCCD_CONNECTED)
        PollReceive();

    //AI(HT160S-Maintainer) 20260612 : align HT172 TimerCCDConnect (GAP E) - if a
    //connection is desired but the link is down, re-attempt at most every 2 s.
    if(bWantConnected && iState == TOPCCD_IDLE)
    {
        unsigned long Tick = GetTickCount();
        if(Tick - iLastReconnectTick > 2000)
        {
            iLastReconnectTick = Tick;
            TopCcdConnect();
        }
    }
}
//---------------------------------------------------------------------------
bool __fastcall THT160TopCcdSocket::IsTopCcdConnected()
{
    TopCcdPoll();
    return (iState == TOPCCD_CONNECTED);
}
//---------------------------------------------------------------------------
void __fastcall THT160TopCcdSocket::SendTopCcdCmd(AnsiString sData)
{
    if(iState != TOPCCD_CONNECTED || sckTopCcd == INVALID_SOCKET)
    {
        sLastError = "Top CCD not connected";
        return;
    }

    int iSize = sData.Length();
    if(iSize <= 0)
    {
        sLastError = "Top CCD send data empty";
        return;
    }

    int SendResult = send(sckTopCcd, sData.c_str(), iSize, 0);
    if(SendResult == SOCKET_ERROR)
    {
        int LastSocketError = WSAGetLastError();
        if(LastSocketError != WSAEWOULDBLOCK)
        {
            sLastError = AnsiString("send error ") + IntToStr(LastSocketError);
            CloseSocket();
        }
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160TopCcdSocket::DrainSocketInput()
{
    //AI(HT160S-Maintainer) 20260612 : align HT172 FSM_PhotoInit (GAP D) - discard
    //any stale bytes left in the OS socket buffer before a new shot, so the next
    //read cannot return a code from the previous scan.
    if(sckTopCcd == INVALID_SOCKET)
        return;
    char Buffer[1025];
    for(;;)
    {
        int ReceiveLength = recv(sckTopCcd, Buffer, 1024, 0);
        if(ReceiveLength <= 0)
            break;
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160TopCcdSocket::TopCcdTriggerShot()
{
    bTopCcdReadDone = false;
    sTopCcd2D       = "";
    sRecvBuffer     = "";
    DrainSocketInput();
    SendTopCcdCmd("LON\r\n");   //AI(HT160S-Maintainer) 20260612 : align HT172 - CRLF terminator (GAP B)
}
//---------------------------------------------------------------------------
void __fastcall THT160TopCcdSocket::TopCcdEndShot()
{
    //AI(HT160S-Maintainer) 20260612 : align HT172 FSM_PhotoFinish (GAP C) - send
    //LOFF to end the shot / lamp off after the code is read or on timeout.
    SendTopCcdCmd("LOFF\r\n");
}
//---------------------------------------------------------------------------
bool __fastcall THT160TopCcdSocket::TopCcdGetResult(AnsiString &sCode)
{
    TopCcdPoll();
    if(bTopCcdReadDone)
    {
        sCode = sTopCcd2D;
        return true;
    }
    sCode = "";
    return false;
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160TopCcdSocket::GetLastError()
{
    return sLastError;
}
//---------------------------------------------------------------------------
void __fastcall THT160TopCcdSocket::SaveTopCcd2DLog(AnsiString sMessage)
{
    if(sMessage == "")
        return;

    AnsiString sDir = HSys.CurrentDir + AnsiString("\\2D_Logs");
    if(DirectoryExists(sDir) == false)
        ForceDirectories(sDir);

    AnsiString sPath = sDir + AnsiString("\\") +
        Now().FormatString("yyyy_mm_dd") + AnsiString("_log.txt");

    FILE *F = fopen(sPath.c_str(), "a");
    if(F != NULL)
    {
        AnsiString sLine = Now().FormatString("hh:nn:ss:zzz :") + sMessage + AnsiString("\n");
        fputs(sLine.c_str(), F);
        fclose(F);
    }
}
//---------------------------------------------------------------------------
