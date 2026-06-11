//---------------------------------------------------------------------------
#ifndef MCUDisplayH
#define MCUDisplayH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <winsock.h>
//---------------------------------------------------------------------------
class THT160MCUDisplay : public TComponent
{
private:
    TStringList *CommandQueue;
    SOCKET ClientSocket;
    bool DisplayEnabled;
    bool Started;
    bool WsaStarted;
    int SocketState;
    int ServerPort;
    int MaxQueueCount;
    int ReconnectIntervalMs;
    DWORD NextConnectTick;
    AnsiString ServerHost;
    AnsiString LastErrorText;
    AnsiString ActiveCommand;
    int ActiveSendOffset;

    void __fastcall LoadConfig();
    void __fastcall WriteLog(AnsiString Text);
    bool __fastcall StartWinsock();
    void __fastcall CloseSocket();
    void __fastcall TryConnect();
    void __fastcall CheckConnectResult();
    void __fastcall SendNextCommand();
    bool __fastcall QueueCommand(AnsiString CommandText);
    bool __fastcall IsTickDue(DWORD NowTick, DWORD DueTick);
public:
    __fastcall THT160MCUDisplay(TComponent *Owner);
    __fastcall ~THT160MCUDisplay();

    bool __fastcall Start();
    void __fastcall Stop();
    void __fastcall Spin();
    bool __fastcall IsConnected();
    bool __fastcall IsEnabled();
    int __fastcall GetQueueCount();
    AnsiString __fastcall GetLastError();

    void __fastcall SetBinCode(int Address, AnsiString CodeText, bool SymbolType);
    void __fastcall SetBinLight(int Address, int LightValue);
    void __fastcall SetBinDisplay(int Address, AnsiString Text, AnsiString ColorText);
};
//---------------------------------------------------------------------------
extern THT160MCUDisplay *HT160MCUDisplay;
void __fastcall EnsureMCUDisplayCreated(TComponent *Owner);
void __fastcall ShutdownHT160MCUDisplay();
void __fastcall SpinMCUDisplay();
void __fastcall SetMCUBinCode(int Address, AnsiString CodeText, bool SymbolType);
void __fastcall SetMCUBinLight(int Address, int LightValue);
void __fastcall SetMCUBinDisplay(int Address, AnsiString Text, AnsiString ColorText);
//---------------------------------------------------------------------------
#endif
//---------------------------------------------------------------------------