//---------------------------------------------------------------------------
// ColorCcdSocket.cpp
// Color-station 2D barcode camera client socket (HT160S_BCB).
// See ColorCcdSocket.h for the design notes and protocol.
//---------------------------------------------------------------------------
#include <vcl.h>
#include <IniFiles.hpp>
#include <winsock.h>
#include <stdio.h>
#pragma hdrstop

#include "ColorCcdSocket.h"
#include "database.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
static const char *DEFAULT_COLORCCD_ADDRESS = "172.16.8.90";
static const int   DEFAULT_COLORCCD_PORT    = 5001;

THT160ColorCcdSocket *ColorCcdSocket = NULL;
//---------------------------------------------------------------------------
void EnsureColorCcdSocketCreated()
{
    if(ColorCcdSocket == NULL)
    {
        ColorCcdSocket = new THT160ColorCcdSocket();
        ColorCcdSocket->LoadConfig();
    }
}
//---------------------------------------------------------------------------
__fastcall THT160ColorCcdSocket::THT160ColorCcdSocket()
{
    sckColorCcd      = INVALID_SOCKET;
    iState           = COLORCCD_IDLE;
    bWsaStarted      = false;
    bColorCcdConnect = false;
    bColorCcdReadDone= false;
    sColorCcd2D      = "";
    sRecvBuffer      = "";
    sLastError       = "";
    sCcdAddress      = DEFAULT_COLORCCD_ADDRESS;
    iCcdPort         = DEFAULT_COLORCCD_PORT;
}
//---------------------------------------------------------------------------
__fastcall THT160ColorCcdSocket::~THT160ColorCcdSocket()
{
    ColorCcdDisconnect();
    if(bWsaStarted)
    {
        WSACleanup();
        bWsaStarted = false;
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160ColorCcdSocket::LoadConfig()
{
    // Ship + hardware install tier: system\General.ini [ColorCCD].
    AnsiString ConfigPath = HSys.CurrentDir + AnsiString("\\system\\General.ini");
    TIniFile *IniFile = new TIniFile(ConfigPath);
    try
    {
        sCcdAddress = IniFile->ReadString("ColorCCD", "Address", DEFAULT_COLORCCD_ADDRESS);
        iCcdPort    = IniFile->ReadInteger("ColorCCD", "Port", DEFAULT_COLORCCD_PORT);
        if(iCcdPort <= 0 || iCcdPort > 65535)
            iCcdPort = DEFAULT_COLORCCD_PORT;
    }
    __finally
    {
        delete IniFile;
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160ColorCcdSocket::SetEndpoint(AnsiString sAddress, int iPort)
{
    if(sAddress.Trim() != "")
        sCcdAddress = sAddress.Trim();
    if(iPort > 0 && iPort <= 65535)
        iCcdPort = iPort;
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160ColorCcdSocket::GetAddress()
{
    return sCcdAddress;
}
//---------------------------------------------------------------------------
int __fastcall THT160ColorCcdSocket::GetPort()
{
    return iCcdPort;
}
//---------------------------------------------------------------------------
bool __fastcall THT160ColorCcdSocket::StartWinsock()
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
void __fastcall THT160ColorCcdSocket::CloseSocket()
{
    if(sckColorCcd != INVALID_SOCKET)
    {
        closesocket(sckColorCcd);
        sckColorCcd = INVALID_SOCKET;
    }
    iState           = COLORCCD_IDLE;
    bColorCcdConnect = false;
    sRecvBuffer      = "";
}
//---------------------------------------------------------------------------
bool __fastcall THT160ColorCcdSocket::ColorCcdConnect()
{
    if(iState == COLORCCD_CONNECTED || iState == COLORCCD_CONNECTING)
        return true;

    if(StartWinsock() == false)
        return false;

    sckColorCcd = socket(AF_INET, SOCK_STREAM, 0);
    if(sckColorCcd == INVALID_SOCKET)
    {
        sLastError = AnsiString("socket error ") + IntToStr(WSAGetLastError());
        return false;
    }

    u_long NonBlocking = 1;
    ioctlsocket(sckColorCcd, FIONBIO, &NonBlocking);

    sockaddr_in ServerAddress;
    memset(&ServerAddress, 0, sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port   = htons((u_short)iCcdPort);
    ServerAddress.sin_addr.s_addr = inet_addr(sCcdAddress.c_str());

    int Result = connect(sckColorCcd, (sockaddr *)&ServerAddress, sizeof(ServerAddress));
    if(Result == 0)
    {
        iState           = COLORCCD_CONNECTED;
        bColorCcdConnect = true;
        sLastError       = "";
        return true;
    }

    int LastSocketError = WSAGetLastError();
    if(LastSocketError == WSAEWOULDBLOCK || LastSocketError == WSAEINPROGRESS)
    {
        iState = COLORCCD_CONNECTING;   // completion confirmed in PollConnecting()
        return true;
    }

    sLastError = AnsiString("connect error ") + IntToStr(LastSocketError);
    CloseSocket();
    return false;
}
//---------------------------------------------------------------------------
void __fastcall THT160ColorCcdSocket::ColorCcdDisconnect()
{
    CloseSocket();
}
//---------------------------------------------------------------------------
void __fastcall THT160ColorCcdSocket::PollConnecting()
{
    if(iState != COLORCCD_CONNECTING || sckColorCcd == INVALID_SOCKET)
        return;

    fd_set WriteSet;
    fd_set ExceptSet;
    FD_ZERO(&WriteSet);
    FD_ZERO(&ExceptSet);
    FD_SET(sckColorCcd, &WriteSet);
    FD_SET(sckColorCcd, &ExceptSet);

    timeval Timeout;
    Timeout.tv_sec  = 0;
    Timeout.tv_usec = 0;   // zero timeout => non-blocking probe

    int Result = select(0, NULL, &WriteSet, &ExceptSet, &Timeout);
    if(Result <= 0)
        return;            // still pending (or transient) => stay CONNECTING

    if(FD_ISSET(sckColorCcd, &ExceptSet))
    {
        sLastError = "Color CCD connect failed";
        CloseSocket();
        return;
    }

    if(FD_ISSET(sckColorCcd, &WriteSet))
    {
        iState           = COLORCCD_CONNECTED;
        bColorCcdConnect = true;
        sLastError       = "";
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160ColorCcdSocket::PollReceive()
{
    if(iState != COLORCCD_CONNECTED || sckColorCcd == INVALID_SOCKET)
        return;

    char Buffer[1025];
    int ReceiveLength = recv(sckColorCcd, Buffer, 1024, 0);
    if(ReceiveLength > 0)
    {
        Buffer[ReceiveLength] = '\0';
        sRecvBuffer += AnsiString(Buffer);

        AnsiString sTrimmed = sRecvBuffer.Trim();
        if(sTrimmed != "")
        {
            sColorCcd2D       = sTrimmed;
            bColorCcdReadDone = true;
            SaveColorCcd2DLog(sColorCcd2D);
        }
        sRecvBuffer = "";
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
void __fastcall THT160ColorCcdSocket::ColorCcdPoll()
{
    if(iState == COLORCCD_CONNECTING)
        PollConnecting();
    if(iState == COLORCCD_CONNECTED)
        PollReceive();
}
//---------------------------------------------------------------------------
bool __fastcall THT160ColorCcdSocket::IsColorCcdConnected()
{
    ColorCcdPoll();
    return (iState == COLORCCD_CONNECTED);
}
//---------------------------------------------------------------------------
void __fastcall THT160ColorCcdSocket::SendColorCcdCmd(AnsiString sData)
{
    if(iState != COLORCCD_CONNECTED || sckColorCcd == INVALID_SOCKET)
    {
        sLastError = "Color CCD not connected";
        return;
    }

    int iSize = sData.Length();
    if(iSize <= 0)
    {
        sLastError = "Color CCD send data empty";
        return;
    }

    int SendResult = send(sckColorCcd, sData.c_str(), iSize, 0);
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
void __fastcall THT160ColorCcdSocket::ColorCcdTriggerShot()
{
    bColorCcdReadDone = false;
    sColorCcd2D       = "";
    sRecvBuffer       = "";
    SendColorCcdCmd("LON");
}
//---------------------------------------------------------------------------
void __fastcall THT160ColorCcdSocket::ColorCcdEndShot()
{
    // End the current shot (lamp off / stop imaging). Safe to call even if the
    // reply already arrived; the read flag/result are left intact for the caller.
    SendColorCcdCmd("LOFF");
}
//---------------------------------------------------------------------------
bool __fastcall THT160ColorCcdSocket::ColorCcdGetResult(AnsiString &sCode)
{
    ColorCcdPoll();
    if(bColorCcdReadDone)
    {
        sCode = sColorCcd2D;
        return true;
    }
    sCode = "";
    return false;
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160ColorCcdSocket::GetLastError()
{
    return sLastError;
}
//---------------------------------------------------------------------------
void __fastcall THT160ColorCcdSocket::SaveColorCcd2DLog(AnsiString sMessage)
{
    if(sMessage == "")
        return;

    AnsiString sDir = HSys.CurrentDir + AnsiString("\\2D_Logs");
    if(DirectoryExists(sDir) == false)
        ForceDirectories(sDir);

    AnsiString sPath = sDir + AnsiString("\\") +
        Now().FormatString("yyyy_mm_dd") + AnsiString("_color_log.txt");

    FILE *F = fopen(sPath.c_str(), "a");
    if(F != NULL)
    {
        AnsiString sLine = Now().FormatString("hh:nn:ss:zzz :") + sMessage + AnsiString("\n");
        fputs(sLine.c_str(), F);
        fclose(F);
    }
}
//---------------------------------------------------------------------------
