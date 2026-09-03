//---------------------------------------------------------------------------
// LotWebApiClient.h
// Lot WebAPI client socket (HT160S_BCB).
//
// Purpose: given a single Lot name, fetch that Lot's 2D / Bin data from a
// customer factory WebAPI via HTTP GET. Modeled on TopCcdSocket.cpp/.h as a
// standalone, non-FSM, non-blocking raw winsock client.
//
// Endpoint shape (simulator): GET <BaseUrl><LotID>
//   e.g. BaseUrl="http://127.0.0.1:8160/lot/", LotID="A5921.RCS.TEST99"
//   -> GET http://127.0.0.1:8160/lot/A5921.RCS.TEST99
//
// Non-blocking contract: no Sleep / busy-wait. Caller starts a request then
// polls GetResult() in its existing loop until it returns true. JSON parsing
// into the Lot registry is done by the caller (stage 3), not here. This module
// only returns the raw HTTP response body.
//---------------------------------------------------------------------------
#ifndef LotWebApiClientH
#define LotWebApiClientH
//---------------------------------------------------------------------------
#include <vcl.h>
#include <winsock.h>
//---------------------------------------------------------------------------
// Request lifecycle state.
enum TLotWebApiState
{
    LOTWEBAPI_IDLE = 0,        // no request in flight
    LOTWEBAPI_CONNECTING = 1,  // non-blocking connect in progress
    LOTWEBAPI_SENDING = 2,     // connected, sending request
    LOTWEBAPI_RECEIVING = 3,   // request sent, reading response
    LOTWEBAPI_DONE = 4,        // response complete (success or http error)
    LOTWEBAPI_FAILED = 5       // transport-level failure
};
//---------------------------------------------------------------------------
class THT160LotWebApiClient
{
private:
    SOCKET sckApi;
    TLotWebApiState iState;
    bool bWsaStarted;

    AnsiString sBaseUrl;        // full base URL incl. path prefix (edWebapiPath)
    AnsiString sHost;           // parsed host
    int iPort;                  // parsed port (default 80)
    AnsiString sRequestPath;    // parsed path + LotID for current request

    AnsiString sSendBuffer;     // request bytes not yet sent
    AnsiString sRecvBuffer;     // raw response accumulation
    AnsiString sResponseBody;   // parsed body (after header split)
    int iHttpStatus;            // parsed HTTP status code (e.g. 200/404)
    bool bRequestOk;            // true if status 200 and body present
    AnsiString sCurrentLot;     // LotID for the in-flight request
    AnsiString sLastError;
    TDateTime dtRequestStart;   // for timeout guard
    //AI(ht160s-lot-webapi) 20260612 : Stage 4 : gate the machine-flow auto-pull
    // (LotStart / SECS LOTSTART).  Default false so the machine never tries to
    // connect to the WebAPI until the real customer endpoint is wired in.
    bool bUsePull;
    //AI(ht160s-lot-webapi) 20260716 : dump the full HTTP request/response body to
    // the WebAPI log so a host command / JSON schema mismatch can be verified from
    // the log alone (mirrors SECS LogSmlBody). bLogBody gated by [LotWebApi]
    // LogBody (default on); iLogBodyCap = max bytes dumped, [LotWebApi] LogBodyCap
    // (<=0 = unlimited).
    bool bLogBody;
    int  iLogBodyCap;

    bool __fastcall StartWinsock();
    void __fastcall CloseSocket();
    bool __fastcall ParseBaseUrl(AnsiString Url, AnsiString &Host, int &Port, AnsiString &Path);
    AnsiString __fastcall UrlEncodeLot(AnsiString Lot);
    void __fastcall BeginConnect();
    void __fastcall PollConnecting();
    void __fastcall PollSending();
    void __fastcall PollReceiving();
    void __fastcall FinishResponse();
    bool __fastcall IsTimedOut();
    void __fastcall SaveWebApiLog(AnsiString sMessage);
    //AI(ht160s-lot-webapi) 20260716 : capped verbatim dump of a request/response.
    void __fastcall LogHttpDump(AnsiString Tag, AnsiString Raw);
    //AI(ht160s-lot-webapi) 20260716 : sanitize a raw dump for the text-mode log:
    // strip CR (text mode re-adds one per LF, else CRLF would double) and map
    // NUL to '.' (fprintf %s stops at NUL and would silently truncate).
    AnsiString __fastcall SanitizeForLog(AnsiString Raw);
    void __fastcall LogTransportFailure();

public:
    __fastcall THT160LotWebApiClient();
    __fastcall ~THT160LotWebApiClient();

    // Configuration (ship + hardware install tier; system\General.ini [LotWebApi]).
    void __fastcall LoadConfig();
    void __fastcall SaveConfig();
    void __fastcall SetBaseUrl(AnsiString Url);
    AnsiString __fastcall GetBaseUrl();
    //AI(ht160s-lot-webapi) 20260612 : Stage 4 : auto-pull enable toggle ([LotWebApi] UsePull).
    bool __fastcall GetUsePull();
    void __fastcall SetUsePull(bool bUse);

    // Request control (non-blocking).
    bool __fastcall StartLotRequest(AnsiString LotID);     // begins async GET
    bool __fastcall GetResult(AnsiString &Body, bool &bOk, int &HttpStatus); // true when finished
    void __fastcall Poll();                                 // drive state machine
    bool __fastcall IsBusy();
    void __fastcall Cancel();

    AnsiString __fastcall GetLastError();
    AnsiString __fastcall GetCurrentLot();
};
//---------------------------------------------------------------------------
extern THT160LotWebApiClient *LotWebApiClient;
void EnsureLotWebApiClientCreated();
//---------------------------------------------------------------------------
#endif
