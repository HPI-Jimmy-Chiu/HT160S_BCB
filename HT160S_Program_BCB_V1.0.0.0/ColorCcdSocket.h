//---------------------------------------------------------------------------
// ColorCcdSocket.h
// Color-station 2D barcode camera client socket (HT160S_BCB).
//
// Mirrors TopCcdSocket (the Loader Top CCD client) but serves the Color station
// identity-tray reader. The 2D reader rides a stepper motor and is moved in X to
// the teach "read 2D position" before a shot, exactly like the Loader Top CCD.
//
// Protocol (per machine wiring):
//   send "LON"  -> trigger a shot (camera starts imaging)
//   send "LOFF" -> end the shot   (camera stops imaging / lamp off)
// The camera replies with the 2D code string, stored (trimmed) in sColorCcd2D.
//
// Non-blocking contract: no Sleep / busy-wait. Callers trigger a shot, poll
// ColorCcdGetResult() in their existing ladder until it returns true, then call
// ColorCcdEndShot() to close the shot.
//
// Config (ship + hardware install tier): system\General.ini [ColorCCD].
//---------------------------------------------------------------------------
#ifndef ColorCcdSocketH
#define ColorCcdSocketH
//---------------------------------------------------------------------------
#include <vcl.h>
#include <winsock.h>
//---------------------------------------------------------------------------
// Connection lifecycle state.
enum TColorCcdState
{
    COLORCCD_IDLE = 0,        // socket closed
    COLORCCD_CONNECTING = 1,  // non-blocking connect in progress
    COLORCCD_CONNECTED = 2    // connected, ready to send/receive
};
//---------------------------------------------------------------------------
class THT160ColorCcdSocket
{
private:
    SOCKET sckColorCcd;
    TColorCcdState iState;
    bool bWsaStarted;
    bool bColorCcdConnect;        // true once OnConnect equivalent fires
    bool bColorCcdReadDone;       // true once a reply for the current shot arrived
    AnsiString sColorCcd2D;       // last received 2D code (trimmed)
    AnsiString sRecvBuffer;       // raw receive accumulation
    AnsiString sLastError;
    AnsiString sCcdAddress;       // camera IP
    int iCcdPort;                 // camera port
    bool bWantConnected;          // true while a connection is desired (drives auto-reconnect)
    unsigned long iLastReconnectTick; // throttle auto-reconnect attempts (GetTickCount)

    bool __fastcall StartWinsock();
    void __fastcall CloseSocket();
    void __fastcall PollConnecting();
    void __fastcall PollReceive();
    void __fastcall SendColorCcdCmd(AnsiString sData);
    void __fastcall DrainSocketInput();   // discard stale bytes before a shot
    void __fastcall SaveColorCcd2DLog(AnsiString sMessage);

public:
    __fastcall THT160ColorCcdSocket();
    __fastcall ~THT160ColorCcdSocket();

    // Configuration (ship + hardware install tier; read from system\General.ini).
    void __fastcall LoadConfig();
    void __fastcall SetEndpoint(AnsiString sAddress, int iPort);
    AnsiString __fastcall GetAddress();
    int __fastcall GetPort();

    // Connection control.
    bool __fastcall ColorCcdConnect();
    void __fastcall ColorCcdDisconnect();
    bool __fastcall IsColorCcdConnected();

    // Shot / result (non-blocking).
    void __fastcall ColorCcdTriggerShot();                 // send "LON", clear read flag
    void __fastcall ColorCcdEndShot();                     // send "LOFF" (end the shot)
    bool __fastcall ColorCcdGetResult(AnsiString &sCode);  // true + code once received

    // Drive socket state machine (call from a periodic loop, or implicitly via
    // IsColorCcdConnected / ColorCcdGetResult).
    void __fastcall ColorCcdPoll();

    AnsiString __fastcall GetLastError();
};
//---------------------------------------------------------------------------
extern THT160ColorCcdSocket *ColorCcdSocket;
void EnsureColorCcdSocketCreated();
//---------------------------------------------------------------------------
#endif
