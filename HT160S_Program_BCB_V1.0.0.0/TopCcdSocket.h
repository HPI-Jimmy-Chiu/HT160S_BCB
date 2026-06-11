//---------------------------------------------------------------------------
// TopCcdSocket.h
// Top CCD 2D barcode camera client socket (HT160S_BCB).
// Ported from old HT160S TfSetup::ClientSocket1 (Top CCD) logic, rewritten as a
// standalone non-FSM, non-blocking raw winsock client module.
//
// Rename map (old -> new):
//   ClientSocket1        -> sckTopCcd
//   SendCMD_CCD          -> SendTopCcdCmd
//   ClientSocket1Read    -> (folded into TopCcdPoll/recv)
//   bCCD_Connect         -> bTopCcdConnect
//   bCCD_LON_Fin         -> bTopCcdReadDone
//   sCCD_2D              -> sTopCcd2D
//   Save_2D_Log          -> SaveTopCcd2DLog
//
// Protocol: send "LON" to trigger a shot; the camera replies with the 2D code
// string, which is stored (trimmed) in sTopCcd2D.
//
// Non-blocking contract: no Sleep / busy-wait. Callers trigger a shot then poll
// TopCcdGetResult() in their existing loop until it returns true.
//---------------------------------------------------------------------------
#ifndef TopCcdSocketH
#define TopCcdSocketH
//---------------------------------------------------------------------------
#include <vcl.h>
#include <winsock.h>
//---------------------------------------------------------------------------
// Connection lifecycle state.
enum TTopCcdState
{
    TOPCCD_IDLE = 0,        // socket closed
    TOPCCD_CONNECTING = 1,  // non-blocking connect in progress
    TOPCCD_CONNECTED = 2    // connected, ready to send/receive
};
//---------------------------------------------------------------------------
class THT160TopCcdSocket
{
private:
    SOCKET sckTopCcd;
    TTopCcdState iState;
    bool bWsaStarted;
    bool bTopCcdConnect;        // true once OnConnect equivalent fires
    bool bTopCcdReadDone;       // true once a reply for the current shot arrived
    AnsiString sTopCcd2D;       // last received 2D code (trimmed)
    AnsiString sRecvBuffer;     // raw receive accumulation
    AnsiString sLastError;
    AnsiString sCcdAddress;     // camera IP
    int iCcdPort;               // camera port

    bool __fastcall StartWinsock();
    void __fastcall CloseSocket();
    void __fastcall PollConnecting();
    void __fastcall PollReceive();
    void __fastcall SendTopCcdCmd(AnsiString sData);
    void __fastcall SaveTopCcd2DLog(AnsiString sMessage);

public:
    __fastcall THT160TopCcdSocket();
    __fastcall ~THT160TopCcdSocket();

    // Configuration (ship + hardware install tier; read from system\General.ini).
    void __fastcall LoadConfig();
    void __fastcall SetEndpoint(AnsiString sAddress, int iPort);
    AnsiString __fastcall GetAddress();
    int __fastcall GetPort();

    // Connection control.
    bool __fastcall TopCcdConnect();
    void __fastcall TopCcdDisconnect();
    bool __fastcall IsTopCcdConnected();

    // Shot / result (non-blocking).
    void __fastcall TopCcdTriggerShot();                 // send "LON", clear read flag
    bool __fastcall TopCcdGetResult(AnsiString &sCode);  // true + code once received

    // Drive socket state machine (call from a periodic loop, or implicitly via
    // IsTopCcdConnected / TopCcdGetResult).
    void __fastcall TopCcdPoll();

    AnsiString __fastcall GetLastError();
};
//---------------------------------------------------------------------------
extern THT160TopCcdSocket *TopCcdSocket;
void EnsureTopCcdSocketCreated();
//---------------------------------------------------------------------------
#endif
