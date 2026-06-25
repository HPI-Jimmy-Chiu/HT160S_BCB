//---------------------------------------------------------------------------
#ifndef mymessboxH
#define mymessboxH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TMyMessageBox : public TForm
{
__published:
    TPanel *Panel1;
    TLabel *Label1;
    TLabel *Label2;
    TPanel *palPause;
    TPanel *palYes;
    TPanel *palNo;
    TButton *btnOffBuzzer;
    void __fastcall palPauseClick(TObject *Sender);
    void __fastcall palYesClick(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall btnOffBuzzerClick(TObject *Sender);
private:
    int Status;
public:
    __fastcall TMyMessageBox(TComponent* Owner);
    int ret;
    bool flushState;
    int flushCT;
    bool fShow;
    bool fScanPanel;
    bool bFormShowNoStop;
    //AI(HT160S-Maintainer) 20260622 : message-box buzzer-mute latch, HT172 bOffBuzzer parity.
    //Set by btnOffBuzzerClick (Off Buzzer), cleared each FormShow/FormClose; read by DoSystemMessage
    //so a muted message box drops out of the LED_Message buzzer state instead of re-sounding.
    bool fBuzzerOff;
    char Message[256];
    char CHMessage[256];
    char ENMessage[256];
    enum eMyMessageRtn
    {
        msgrtnPAUSE=0,
        msgrtnYES=1,
        msgrtnNO=2
    };
};
//---------------------------------------------------------------------------
extern PACKAGE TMyMessageBox *MyMessageBox;
void ShowMyMessage(AnsiString S);
void ShowMyMessage(char *S);
void ShowMyMessage(int Code);
void ShowMyOKMessage(char *str);
void ShowMyOKMessage(int Code);
void ShowMyOKMessageNoStop(AnsiString S);
void ShowMyMessage_Run(AnsiString S1, AnsiString S2);
int  ShowMyMessageBox_YES_NO(AnsiString str);
int  ShowMyMessageBox_YES_NO(char *str);
AnsiString LoadLanguageString(char *str, int type);
AnsiString LoadLanguageStringForCode(int Code, int type);
AnsiString Changelanguagea(AnsiString S);
void CloseBuzzerOff();
void ShowSECSGEMMessage(AnsiString S);
//---------------------------------------------------------------------------
#endif
