//---------------------------------------------------------------------------
#ifndef AutomationServerH
#define AutomationServerH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <ExtCtrls.hpp>
#include <winsock.h>
//---------------------------------------------------------------------------
class TfMain;
//---------------------------------------------------------------------------
class THT160AutomationServer : public TComponent
{
private:
    TTimer *SocketTimer;
    TfMain *MainForm;
    SOCKET ListenSocket;
    SOCKET ClientSocket;
    bool ServerEnabled;
    bool WsaStarted;
    bool HasPendingLotInfo;
    bool LocalOnly;
    int ServerPort;
    AnsiString LastError;
    AnsiString ReceiveBuffer;
    AnsiString PendingLotPayload;
    AnsiString BindAddressText;

    void __fastcall SocketTimerTimer(TObject *Sender);

    void __fastcall LoadConfig();
    void __fastcall WriteLog(AnsiString Text);
    void __fastcall UpdateAutomationLabel(AnsiString Text);
    void __fastcall EnsureMainFormAttached();
    bool __fastcall StartWinsock();
    void __fastcall CloseClientSocket();
    void __fastcall CloseListenSocket();
    void __fastcall AcceptClient();
    void __fastcall ReadClient();
    bool __fastcall SendClientText(AnsiString Text);
    AnsiString __fastcall ProcessCommand(AnsiString Request);
    AnsiString __fastcall BuildStatusReply();
    AnsiString __fastcall BuildLotInfoReply();
    AnsiString __fastcall SetLotInfo(AnsiString Payload);
    AnsiString __fastcall SmokeProbeTopForms();
    AnsiString __fastcall GetPayloadValue(AnsiString Payload, AnsiString Key);
    AnsiString __fastcall EncodeValue(AnsiString Value);
    AnsiString __fastcall DecodeValue(AnsiString Value);
public:
    __fastcall THT160AutomationServer(TComponent* Owner, TfMain *AMainForm);
    __fastcall ~THT160AutomationServer();

    bool __fastcall Start();
    void __fastcall Stop();
    void __fastcall SetMainForm(TfMain *AMainForm);
    bool __fastcall IsActive();
    int __fastcall GetPort();
    AnsiString __fastcall GetLastError();
};
//---------------------------------------------------------------------------
extern THT160AutomationServer *HT160AutomationServer;
bool __fastcall InitializeHT160Automation(TfMain *MainForm);
void __fastcall ShutdownHT160Automation();
//---------------------------------------------------------------------------
#endif
