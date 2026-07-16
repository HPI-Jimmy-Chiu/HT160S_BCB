//---------------------------------------------------------------------------
// LotWebApiClient.cpp
// Lot WebAPI client socket (HT160S_BCB).
// See LotWebApiClient.h for design notes. Non-blocking raw winsock HTTP/1.0
// GET client, modeled on TopCcdSocket.cpp.
//---------------------------------------------------------------------------
#include <vcl.h>
#include <IniFiles.hpp>
#include <winsock.h>
#include <stdio.h>
#include <stdlib.h>
#pragma hdrstop

#include "LotWebApiClient.h"
#include "database.h"
#include "cCsvDailyLog.h"
#include "GeneralSetting.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
static const char *DEFAULT_LOTWEBAPI_BASEURL = "http://127.0.0.1:8160/lot/";
static const int   LOTWEBAPI_TIMEOUT_SEC     = 8;   // overall request timeout
static const int   LOTWEBAPI_LOG_BODY_CAP_DEFAULT = 65536; // max bytes of req/resp dumped

THT160LotWebApiClient *LotWebApiClient = NULL;

// WebAPI log channel. Daily-folder + .log + no header preserves the exact
// path WebAPI\YYYYMMDD\WebAPI_YYYYMMDD.log that cStateRecordHT160::CaptureWebApiLog
// copies into the snapshot. Init() runs once in the client constructor.
static cCsvDailyLog g_WebApiLog;
//---------------------------------------------------------------------------
void EnsureLotWebApiClientCreated()
{
    if(LotWebApiClient == NULL)
    {
        LotWebApiClient = new THT160LotWebApiClient();
        LotWebApiClient->LoadConfig();
    }
}
//---------------------------------------------------------------------------
__fastcall THT160LotWebApiClient::THT160LotWebApiClient()
{
    sckApi        = INVALID_SOCKET;
    iState        = LOTWEBAPI_IDLE;
    bWsaStarted   = false;
    sBaseUrl      = DEFAULT_LOTWEBAPI_BASEURL;
    sHost         = "";
    iPort         = 80;
    sRequestPath  = "";
    sSendBuffer   = "";
    sRecvBuffer   = "";
    sResponseBody = "";
    iHttpStatus   = 0;
    bRequestOk    = false;
    sCurrentLot   = "";
    sLastError    = "";
    dtRequestStart= 0;
    bUsePull      = false;      //AI(ht160s-lot-webapi) 20260612 : Stage 4 : auto-pull off until customer API wired
    bLogBody      = true;       //AI(ht160s-lot-webapi) 20260716 : full body dump on by default (gated by [LotWebApi] LogBody)
    iLogBodyCap   = LOTWEBAPI_LOG_BODY_CAP_DEFAULT;

    //AI(ht160s-lot-webapi) 20260615 : WebAPI log now shares cCsvDailyLog (thread-safe
    //append). lgDailyFolder + ".log" + empty header keeps the exact legacy path
    //WebAPI\YYYYMMDD\WebAPI_YYYYMMDD.log so the state-record snapshot copy is unchanged.
    g_WebApiLog.InitLog("WebAPI", "WebAPI", "", cCsvDailyLog::lgDailyFolder, ".log");
    //AI(general) 20260617 : auto-prune old WebAPI day-folders. Audit retention
    //(EventDays). GeneralSetting is loaded by the time the client is created.
    g_WebApiLog.SetRetentionDays(GeneralSetting.iLogRetentionEventDays);
}
//---------------------------------------------------------------------------
__fastcall THT160LotWebApiClient::~THT160LotWebApiClient()
{
    CloseSocket();
    if(bWsaStarted)
    {
        WSACleanup();
        bWsaStarted = false;
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160LotWebApiClient::LoadConfig()
{
    AnsiString ConfigPath = HSys.CurrentDir + AnsiString("\\system\\General.ini");
    TIniFile *IniFile = new TIniFile(ConfigPath);
    try
    {
        sBaseUrl = IniFile->ReadString("LotWebApi", "BaseUrl", DEFAULT_LOTWEBAPI_BASEURL);
        if(sBaseUrl.Trim() == "")
            sBaseUrl = DEFAULT_LOTWEBAPI_BASEURL;
        bUsePull = IniFile->ReadBool("LotWebApi", "UsePull", false);
        bLogBody = IniFile->ReadBool("LotWebApi", "LogBody", true);
        iLogBodyCap = IniFile->ReadInteger("LotWebApi", "LogBodyCap", LOTWEBAPI_LOG_BODY_CAP_DEFAULT);
    }
    __finally
    {
        delete IniFile;
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160LotWebApiClient::SaveConfig()
{
    AnsiString ConfigPath = HSys.CurrentDir + AnsiString("\\system\\General.ini");
    ForceDirectories(ExtractFilePath(ConfigPath));
    TIniFile *IniFile = new TIniFile(ConfigPath);
    try
    {
        IniFile->WriteString("LotWebApi", "BaseUrl", sBaseUrl);
        IniFile->WriteBool("LotWebApi", "UsePull", bUsePull);
        IniFile->WriteBool("LotWebApi", "LogBody", bLogBody);
        IniFile->WriteInteger("LotWebApi", "LogBodyCap", iLogBodyCap);
    }
    __finally
    {
        delete IniFile;
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160LotWebApiClient::SetBaseUrl(AnsiString Url)
{
    if(Url.Trim() != "")
        sBaseUrl = Url.Trim();
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160LotWebApiClient::GetBaseUrl()
{
    return sBaseUrl;
}
//---------------------------------------------------------------------------
bool __fastcall THT160LotWebApiClient::GetUsePull()
{
    return bUsePull;
}
//---------------------------------------------------------------------------
void __fastcall THT160LotWebApiClient::SetUsePull(bool bUse)
{
    bUsePull = bUse;
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160LotWebApiClient::GetCurrentLot()
{
    return sCurrentLot;
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160LotWebApiClient::GetLastError()
{
    return sLastError;
}
//---------------------------------------------------------------------------
bool __fastcall THT160LotWebApiClient::StartWinsock()
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
void __fastcall THT160LotWebApiClient::CloseSocket()
{
    if(sckApi != INVALID_SOCKET)
    {
        closesocket(sckApi);
        sckApi = INVALID_SOCKET;
    }
}
//---------------------------------------------------------------------------
// Parse "http://host:port/path" into host, port, path. Returns false on a
// grossly malformed URL. Scheme is optional; default port 80.
bool __fastcall THT160LotWebApiClient::ParseBaseUrl(AnsiString Url, AnsiString &Host,
    int &Port, AnsiString &Path)
{
    AnsiString Work = Url.Trim();
    Host = "";
    Port = 80;
    Path = "/";

    // Strip scheme.
    int SchemePos = Work.LowerCase().Pos("http://");
    if(SchemePos == 1)
        Work = Work.SubString(8, Work.Length() - 7);

    if(Work == "")
        return false;

    // Split authority and path on first '/'.
    int SlashPos = Work.Pos("/");
    AnsiString Authority;
    if(SlashPos > 0)
    {
        Authority = Work.SubString(1, SlashPos - 1);
        Path = Work.SubString(SlashPos, Work.Length() - SlashPos + 1);
    }
    else
    {
        Authority = Work;
        Path = "/";
    }

    if(Authority == "")
        return false;

    // Split host and port on ':'.
    int ColonPos = Authority.Pos(":");
    if(ColonPos > 0)
    {
        Host = Authority.SubString(1, ColonPos - 1);
        AnsiString PortText = Authority.SubString(ColonPos + 1, Authority.Length() - ColonPos);
        int ParsedPort = atoi(PortText.c_str());
        if(ParsedPort > 0 && ParsedPort <= 65535)
            Port = ParsedPort;
    }
    else
    {
        Host = Authority;
        Port = 80;
    }

    return (Host.Trim() != "");
}
//---------------------------------------------------------------------------
// Minimal URL encoding for the Lot id: keep unreserved + common Lot chars,
// percent-encode anything else (notably space).
AnsiString __fastcall THT160LotWebApiClient::UrlEncodeLot(AnsiString Lot)
{
    AnsiString Out = "";
    for(int i = 1; i <= Lot.Length(); i++)
    {
        char c = Lot[i];
        bool bSafe =
            (c >= 'A' && c <= 'Z') ||
            (c >= 'a' && c <= 'z') ||
            (c >= '0' && c <= '9') ||
            c == '-' || c == '_' || c == '.' || c == '~';
        if(bSafe)
        {
            Out += c;
        }
        else
        {
            char Hex[4];
            sprintf(Hex, "%%%02X", (unsigned char)c);
            Out += AnsiString(Hex);
        }
    }
    return Out;
}
//---------------------------------------------------------------------------
bool __fastcall THT160LotWebApiClient::StartLotRequest(AnsiString LotID)
{
    if(iState != LOTWEBAPI_IDLE && iState != LOTWEBAPI_DONE && iState != LOTWEBAPI_FAILED)
    {
        sLastError = "request already in progress";
        return false;
    }

    if(LotID.Trim() == "")
    {
        sLastError = "empty Lot id";
        return false;
    }

    AnsiString Host;
    int Port;
    AnsiString Path;
    if(!ParseBaseUrl(sBaseUrl, Host, Port, Path))
    {
        sLastError = AnsiString("bad BaseUrl: ") + sBaseUrl;
        iState = LOTWEBAPI_FAILED;
        return false;
    }

    // Reset per-request state.
    CloseSocket();
    sHost         = Host;
    iPort         = Port;
    sCurrentLot   = LotID.Trim();
    sRequestPath  = Path + UrlEncodeLot(sCurrentLot);
    sRecvBuffer   = "";
    sResponseBody = "";
    iHttpStatus   = 0;
    bRequestOk    = false;
    sLastError    = "";
    dtRequestStart= Now();

    // Build HTTP/1.0 GET (Connection: close so server signals end via FIN).
    sSendBuffer =
        AnsiString("GET ") + sRequestPath + AnsiString(" HTTP/1.0\r\n") +
        AnsiString("Host: ") + sHost + AnsiString("\r\n") +
        AnsiString("Accept: application/json\r\n") +
        AnsiString("Connection: close\r\n") +
        AnsiString("\r\n");

    SaveWebApiLog(AnsiString("Request Lot=") + sCurrentLot +
        AnsiString(" URL=") + sBaseUrl + UrlEncodeLot(sCurrentLot));
    LogHttpDump("Request raw", sSendBuffer);

    BeginConnect();
    return (iState != LOTWEBAPI_FAILED);
}
//---------------------------------------------------------------------------
void __fastcall THT160LotWebApiClient::BeginConnect()
{
    if(StartWinsock() == false)
    {
        iState = LOTWEBAPI_FAILED;
        LogTransportFailure();
        return;
    }

    sckApi = socket(AF_INET, SOCK_STREAM, 0);
    if(sckApi == INVALID_SOCKET)
    {
        sLastError = AnsiString("socket error ") + IntToStr(WSAGetLastError());
        iState = LOTWEBAPI_FAILED;
        LogTransportFailure();
        return;
    }

    u_long NonBlocking = 1;
    ioctlsocket(sckApi, FIONBIO, &NonBlocking);

    sockaddr_in ServerAddress;
    memset(&ServerAddress, 0, sizeof(ServerAddress));
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port   = htons((u_short)iPort);
    ServerAddress.sin_addr.s_addr = inet_addr(sHost.c_str());

    int Result = connect(sckApi, (sockaddr *)&ServerAddress, sizeof(ServerAddress));
    if(Result == 0)
    {
        iState = LOTWEBAPI_SENDING;
        return;
    }

    int LastSocketError = WSAGetLastError();
    if(LastSocketError == WSAEWOULDBLOCK || LastSocketError == WSAEINPROGRESS)
    {
        iState = LOTWEBAPI_CONNECTING;
        return;
    }

    sLastError = AnsiString("connect error ") + IntToStr(LastSocketError);
    CloseSocket();
    iState = LOTWEBAPI_FAILED;
    LogTransportFailure();
}
//---------------------------------------------------------------------------
void __fastcall THT160LotWebApiClient::PollConnecting()
{
    if(iState != LOTWEBAPI_CONNECTING || sckApi == INVALID_SOCKET)
        return;

    fd_set WriteSet;
    fd_set ExceptSet;
    FD_ZERO(&WriteSet);
    FD_ZERO(&ExceptSet);
    FD_SET(sckApi, &WriteSet);
    FD_SET(sckApi, &ExceptSet);

    timeval Timeout;
    Timeout.tv_sec  = 0;
    Timeout.tv_usec = 0;

    int Result = select(0, NULL, &WriteSet, &ExceptSet, &Timeout);
    if(Result <= 0)
        return;

    if(FD_ISSET(sckApi, &ExceptSet))
    {
        sLastError = "WebAPI connect failed";
        CloseSocket();
        iState = LOTWEBAPI_FAILED;
        LogTransportFailure();
        return;
    }

    if(FD_ISSET(sckApi, &WriteSet))
        iState = LOTWEBAPI_SENDING;
}
//---------------------------------------------------------------------------
void __fastcall THT160LotWebApiClient::PollSending()
{
    if(iState != LOTWEBAPI_SENDING || sckApi == INVALID_SOCKET)
        return;

    if(sSendBuffer.Length() <= 0)
    {
        iState = LOTWEBAPI_RECEIVING;
        return;
    }

    int SendResult = send(sckApi, sSendBuffer.c_str(), sSendBuffer.Length(), 0);
    if(SendResult == SOCKET_ERROR)
    {
        int LastSocketError = WSAGetLastError();
        if(LastSocketError != WSAEWOULDBLOCK)
        {
            sLastError = AnsiString("send error ") + IntToStr(LastSocketError);
            CloseSocket();
            iState = LOTWEBAPI_FAILED;
            LogTransportFailure();
        }
        return;   // try again next poll
    }

    if(SendResult >= sSendBuffer.Length())
    {
        sSendBuffer = "";
        iState = LOTWEBAPI_RECEIVING;
    }
    else
    {
        sSendBuffer.Delete(1, SendResult);   // partial send; keep remainder
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160LotWebApiClient::PollReceiving()
{
    if(iState != LOTWEBAPI_RECEIVING || sckApi == INVALID_SOCKET)
        return;

    char Buffer[2049];
    int ReceiveLength = recv(sckApi, Buffer, 2048, 0);
    if(ReceiveLength > 0)
    {
        Buffer[ReceiveLength] = '\0';
        // Binary-safe append (body may contain embedded data; JSON is text but
        // guard against accidental NUL by using explicit length).
        sRecvBuffer += AnsiString(Buffer, ReceiveLength);
        return;   // keep reading until peer closes
    }

    if(ReceiveLength == 0)
    {
        FinishResponse();   // peer closed => full response received
        return;
    }

    int LastSocketError = WSAGetLastError();
    if(LastSocketError != WSAEWOULDBLOCK)
    {
        sLastError = AnsiString("recv error ") + IntToStr(LastSocketError);
        CloseSocket();
        iState = LOTWEBAPI_FAILED;
        LogHttpDump("Response raw(partial, recv error)", sRecvBuffer);
    }
}
//---------------------------------------------------------------------------
void __fastcall THT160LotWebApiClient::FinishResponse()
{
    CloseSocket();

    // Parse status line: "HTTP/1.x <code> <reason>".
    iHttpStatus = 0;
    int SpacePos = sRecvBuffer.Pos(" ");
    if(SpacePos > 0)
    {
        AnsiString Rest = sRecvBuffer.SubString(SpacePos + 1, sRecvBuffer.Length() - SpacePos);
        iHttpStatus = atoi(Rest.c_str());
    }

    // Split headers/body on the first blank line (CRLFCRLF, fallback LFLF).
    int HeaderEnd = sRecvBuffer.Pos("\r\n\r\n");
    int Skip = 4;
    if(HeaderEnd <= 0)
    {
        HeaderEnd = sRecvBuffer.Pos("\n\n");
        Skip = 2;
    }
    if(HeaderEnd > 0)
        sResponseBody = sRecvBuffer.SubString(HeaderEnd + Skip,
            sRecvBuffer.Length() - HeaderEnd - Skip + 1);
    else
        sResponseBody = sRecvBuffer;   // no header delimiter found; treat all as body

    bRequestOk = (iHttpStatus == 200 && sResponseBody.Trim() != "");
    iState = LOTWEBAPI_DONE;

    SaveWebApiLog(AnsiString("Response Lot=") + sCurrentLot +
        AnsiString(" HTTP=") + IntToStr(iHttpStatus) +
        AnsiString(" bytes=") + IntToStr(sResponseBody.Length()) +
        AnsiString(" ok=") + (bRequestOk ? "1" : "0"));
    LogHttpDump("Response raw", sRecvBuffer);
}
//---------------------------------------------------------------------------
bool __fastcall THT160LotWebApiClient::IsTimedOut()
{
    if(dtRequestStart == (TDateTime)0)
        return false;
    TDateTime Delta = Now() - dtRequestStart;
    double Sec = Delta.operator double() * 86400.0;
    return (Sec > (double)LOTWEBAPI_TIMEOUT_SEC);
}
//---------------------------------------------------------------------------
void __fastcall THT160LotWebApiClient::Poll()
{
    if(iState == LOTWEBAPI_IDLE || iState == LOTWEBAPI_DONE || iState == LOTWEBAPI_FAILED)
        return;

    if(IsTimedOut())
    {
        sLastError = "WebAPI request timeout";
        CloseSocket();
        iState = LOTWEBAPI_FAILED;
        SaveWebApiLog(AnsiString("Timeout Lot=") + sCurrentLot);
        LogHttpDump("Response raw(partial, timeout)", sRecvBuffer);
        return;
    }

    if(iState == LOTWEBAPI_CONNECTING)
        PollConnecting();
    if(iState == LOTWEBAPI_SENDING)
        PollSending();
    if(iState == LOTWEBAPI_RECEIVING)
        PollReceiving();
}
//---------------------------------------------------------------------------
bool __fastcall THT160LotWebApiClient::GetResult(AnsiString &Body, bool &bOk, int &HttpStatus)
{
    Poll();
    if(iState == LOTWEBAPI_DONE)
    {
        Body = sResponseBody;
        bOk = bRequestOk;
        HttpStatus = iHttpStatus;
        return true;
    }
    if(iState == LOTWEBAPI_FAILED)
    {
        Body = "";
        bOk = false;
        HttpStatus = iHttpStatus;
        return true;   // finished (with failure)
    }
    Body = "";
    bOk = false;
    HttpStatus = 0;
    return false;
}
//---------------------------------------------------------------------------
bool __fastcall THT160LotWebApiClient::IsBusy()
{
    return (iState == LOTWEBAPI_CONNECTING ||
            iState == LOTWEBAPI_SENDING ||
            iState == LOTWEBAPI_RECEIVING);
}
//---------------------------------------------------------------------------
void __fastcall THT160LotWebApiClient::Cancel()
{
    CloseSocket();
    iState = LOTWEBAPI_IDLE;
    sSendBuffer = "";
    sRecvBuffer = "";
}
//---------------------------------------------------------------------------
void __fastcall THT160LotWebApiClient::SaveWebApiLog(AnsiString sMessage)
{
    if(sMessage == "")
        return;

    //AI(ht160s-lot-webapi) 20260615 : routed through g_WebApiLog (cCsvDailyLog,
    // thread-safe append). Path/format preserved exactly:
    //   D:\HT160S_Log\WebAPI\YYYYMMDD\WebAPI_YYYYMMDD.log , line "hh:nn:ss:zzz :<msg>".
    g_WebApiLog.AppendLine(Now().FormatString("hh:nn:ss:zzz :") + sMessage);
}
//---------------------------------------------------------------------------
//AI(ht160s-lot-webapi) 20260716 : dump the full HTTP request/response verbatim
// to the WebAPI log so a host command or JSON schema mismatch can be verified
// from the log alone (mirrors the SECS LogSmlBody body dump). Gated by
// [LotWebApi] LogBody (default on). Capped at iLogBodyCap ([LotWebApi]
// LogBodyCap, <=0 = unlimited) so a large lot response cannot bloat the daily
// log; the dump header always records the true byte count even when truncated.
void __fastcall THT160LotWebApiClient::LogHttpDump(AnsiString Tag, AnsiString Raw)
{
    if(!bLogBody)
        return;

    int Total = Raw.Length();
    AnsiString Body;
    if(Total <= 0)
        Body = "<empty>";
    else if(iLogBodyCap > 0 && Total > iLogBodyCap)
        Body = SanitizeForLog(Raw.SubString(1, iLogBodyCap)) +
               AnsiString("\n...(truncated, total ") + IntToStr(Total) +
               AnsiString(" bytes)");
    else
        Body = SanitizeForLog(Raw);

    SaveWebApiLog(Tag + AnsiString(" Lot=") + sCurrentLot +
        AnsiString(" (") + IntToStr(Total) + AnsiString(" bytes):\n") + Body);
}
//---------------------------------------------------------------------------
AnsiString __fastcall THT160LotWebApiClient::SanitizeForLog(AnsiString Raw)
{
    // Strip CR so the text-mode log sink (fprintf on a fopen(a) stream) re-adds
    // exactly one CR per LF instead of doubling an embedded CRLF into CR-CR-LF.
    // Map NUL to '.' because fprintf(%s) stops at the first NUL and would
    // silently truncate the dump (legit JSON never contains NUL).
    int n = Raw.Length();
    AnsiString Out;
    Out.SetLength(n);
    int k = 0;
    for(int i = 1; i <= n; i++)
    {
        char c = Raw[i];
        if(c == '\r')
            continue;
        Out[++k] = (c == '\0') ? '.' : c;
    }
    Out.SetLength(k);
    return Out;
}
//---------------------------------------------------------------------------
void __fastcall THT160LotWebApiClient::LogTransportFailure()
{
    SaveWebApiLog(AnsiString("Failed Lot=") + sCurrentLot +
        AnsiString(" err=") + sLastError);
}
//---------------------------------------------------------------------------
