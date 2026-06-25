//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop

#include "mymessbox.h"
#include "database.h"
#include "main.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TMyMessageBox *MyMessageBox=NULL;
//---------------------------------------------------------------------------
static bool bCanShow=true;
//---------------------------------------------------------------------------
static void EnsureMyMessageBox()
{
    if(MyMessageBox==NULL)
        MyMessageBox=new TMyMessageBox(Application);
}
//---------------------------------------------------------------------------
__fastcall TMyMessageBox::TMyMessageBox(TComponent* Owner)
    : TForm(Owner)
{
    Status=0;
    ret=msgrtnPAUSE;
    flushState=false;
    flushCT=0;
    fShow=false;
    fScanPanel=false;
    bFormShowNoStop=false;
    fBuzzerOff=false;
    Message[0]=0;
    CHMessage[0]=0;
    ENMessage[0]=0;
}
//---------------------------------------------------------------------------
AnsiString LoadLanguageString(char *str, int type)
{
    return AnsiString(str);
}
//---------------------------------------------------------------------------
AnsiString LoadLanguageStringForCode(int Code, int type)
{
    return AnsiString().sprintf("%d", Code);
}
//---------------------------------------------------------------------------
AnsiString Changelanguagea(AnsiString S)
{
    return S;
}
//---------------------------------------------------------------------------
static void PrepareNormalMessage(AnsiString S1, AnsiString S2, AnsiString ButtonCaption)
{
    EnsureMyMessageBox();

    MyMessageBox->Width=480;
    MyMessageBox->Height=255;
    MyMessageBox->Panel1->Left=8;
    MyMessageBox->Panel1->Top=8;
    MyMessageBox->Panel1->Width=457;
    MyMessageBox->Panel1->Height=153;
    MyMessageBox->Label1->Caption=S1;
    MyMessageBox->Label2->Caption=S2;
    MyMessageBox->Label2->Visible=(S2!="");
    //AI(HT160S-Maintainer) 20260624 : button Left/Width are no longer set here. After the
    //visible set is chosen, LayoutMessageButtons() (called from FormShow) lays whichever
    //buttons stay Visible into an equal-width, evenly-spaced row, so the 1- and 2-button
    //modes both look uniform. Here we only pick the Caption and which buttons show.
    MyMessageBox->palPause->Caption=ButtonCaption;
    MyMessageBox->palPause->Visible=true;
    MyMessageBox->palYes->Visible=false;
    MyMessageBox->palNo->Visible=false;
    // Off Buzzer is an alarm-only control; default visible here so alarm
    // dialogs keep it. YES/NO confirmations hide it (no buzzer is running).
    MyMessageBox->btnOffBuzzer->Visible=true;
    MyMessageBox->ret=TMyMessageBox::msgrtnPAUSE;
}
//---------------------------------------------------------------------------
void ShowMyMessage(AnsiString S)
{
    if(bCanShow==false)
        return;
    EnsureMyMessageBox();
    if(MyMessageBox->fShow)
        return;

    HSys.DecStopAllMotor();
    HSys.Sys.SystemStart=false;
    PrepareNormalMessage(S, "", "Pause");
    MyMessageBox->fScanPanel=true;
    MyMessageBox->bFormShowNoStop=false;
    MyMessageBox->ShowModal();
}
//---------------------------------------------------------------------------
//AI(ht160s-maintainer) 20260625 : alarm popup with an explicit second detail line (S2 ->
//Label2). Same stopping behavior as the single-arg ShowMyMessage; used by the motor
//out-of-limit callers to show the numeric target / soft-limit values.
void ShowMyMessage(AnsiString S1, AnsiString S2)
{
    if(bCanShow==false)
        return;
    EnsureMyMessageBox();
    if(MyMessageBox->fShow)
        return;

    HSys.DecStopAllMotor();
    HSys.Sys.SystemStart=false;
    PrepareNormalMessage(S1, S2, "Pause");
    MyMessageBox->fScanPanel=true;
    MyMessageBox->bFormShowNoStop=false;
    MyMessageBox->ShowModal();
}
//---------------------------------------------------------------------------
void ShowMyMessage(char *S)
{
    ShowMyMessage(AnsiString(S));
}
//---------------------------------------------------------------------------
void ShowMyMessage(int Code)
{
    ShowMyMessage(LoadLanguageStringForCode(Code, 0));
}
//---------------------------------------------------------------------------
void ShowMyOKMessage(char *str)
{
    if(bCanShow==false)
        return;
    EnsureMyMessageBox();
    if(MyMessageBox->fShow)
        return;

    HSys.DecStopAllMotor();
    HSys.Sys.SystemStart=false;
    PrepareNormalMessage(AnsiString(str), "", "OK");
    MyMessageBox->fScanPanel=false;
    MyMessageBox->bFormShowNoStop=false;
    MyMessageBox->ShowModal();
}
//---------------------------------------------------------------------------
void ShowMyOKMessage(int Code)
{
    AnsiString S=LoadLanguageStringForCode(Code, 0);
    ShowMyOKMessage(S.c_str());
}
//---------------------------------------------------------------------------
//AI(ht160s-maintainer) 20260617 : OK-only popup that does NOT stop motors or clear
//SystemStart (bFormShowNoStop=true). Behavior-preserving replacement for plain VCL
//MessageDlg/ShowMessage info/warning dialogs, which never stopped the machine. Use the
//stopping ShowMyMessage/ShowMyOKMessage for real machine alarms, Note for full alarms.
void ShowMyOKMessageNoStop(AnsiString S)
{
    if(bCanShow==false)
        return;
    EnsureMyMessageBox();
    if(MyMessageBox->fShow)
        return;

    PrepareNormalMessage(S, "", "OK");
    MyMessageBox->fScanPanel=false;
    MyMessageBox->bFormShowNoStop=true;
    MyMessageBox->ShowModal();
}
//---------------------------------------------------------------------------
void ShowMyMessage_Run(AnsiString S1, AnsiString S2)
{
    EnsureMyMessageBox();
    if(MyMessageBox->fShow)
        return;

    PrepareNormalMessage(S1, S2, "Alarm Reset");
    MyMessageBox->fScanPanel=true;
    MyMessageBox->bFormShowNoStop=true;
    MyMessageBox->Show();
}
//---------------------------------------------------------------------------
int ShowMyMessageBox_YES_NO(AnsiString str)
{
    EnsureMyMessageBox();
    if(MyMessageBox->fShow)
        return TMyMessageBox::msgrtnPAUSE;

    PrepareNormalMessage(str, "", "Pause");
    MyMessageBox->palPause->Visible=false;
    MyMessageBox->palYes->Visible=true;
    MyMessageBox->palNo->Visible=true;
    MyMessageBox->btnOffBuzzer->Visible=false;
    MyMessageBox->ret=TMyMessageBox::msgrtnNO;
    MyMessageBox->fScanPanel=false;
    MyMessageBox->bFormShowNoStop=true;
    MyMessageBox->ShowModal();
    return MyMessageBox->ret;
}
//---------------------------------------------------------------------------
int ShowMyMessageBox_YES_NO(char *str)
{
    return ShowMyMessageBox_YES_NO(AnsiString(str));
}
//---------------------------------------------------------------------------
void ShowSECSGEMMessage(AnsiString S)
{
    ShowMyMessage_Run("SECS/GEM", S);
}
//---------------------------------------------------------------------------
void CloseBuzzerOff()
{
    int Index;
    int SwitchIndex;

    for(Index=0; Index<4; Index++)
    {
        SwitchIndex=HSys.Sw.SwMusic1.Tag+Index;
        if(HSys.SwPtr!=NULL && SwitchIndex>=0 && SwitchIndex<HSys.iTotalSwitch)
            HSys.SwPtr[SwitchIndex].Off();
    }
}
//---------------------------------------------------------------------------
void __fastcall TMyMessageBox::palPauseClick(TObject *Sender)
{
    ret=msgrtnPAUSE;
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TMyMessageBox::palYesClick(TObject *Sender)
{
    TPanel *PanelPtr=dynamic_cast<TPanel *>(Sender);

    if(PanelPtr!=NULL && PanelPtr->Tag==1)
        ret=msgrtnYES;
    else if(PanelPtr!=NULL && PanelPtr->Tag==2)
        ret=msgrtnNO;
    Close();
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260624 : lay whichever response buttons are currently Visible into
//equal widths with equal gaps across the button row. The visible set differs by mode (palPause
//+ Off Buzzer for alarm/info/OK, or palYes + palNo for confirmations), so the old fixed DFM
//coords only lined up in one mode; this distributes the live set uniformly on every show.
//Off Buzzer participates as an equal button (chosen layout). Display order: Yes, Pause, No,
//Off Buzzer. TControl* covers both the TPanel action buttons and the TButton Off Buzzer.
//---------------------------------------------------------------------------
static void LayoutMessageButtons()
{
    if(MyMessageBox==NULL)
        return;

    TControl *Btns[4];
    int N=0;
    if(MyMessageBox->palYes->Visible)       Btns[N++]=MyMessageBox->palYes;
    if(MyMessageBox->palPause->Visible)     Btns[N++]=MyMessageBox->palPause;
    if(MyMessageBox->palNo->Visible)        Btns[N++]=MyMessageBox->palNo;
    if(MyMessageBox->btnOffBuzzer->Visible) Btns[N++]=MyMessageBox->btnOffBuzzer;
    if(N==0)
        return;

    int Margin=12;
    int Gap=16;
    int RowTop=178;
    int RowHeight=33;
    int Avail=MyMessageBox->ClientWidth-2*Margin-(N-1)*Gap;
    if(Avail<N)
        return;
    int ButtonWidth=Avail/N;
    int X=Margin;
    int i;
    for(i=0; i<N; i++)
    {
        Btns[i]->Left=X;
        Btns[i]->Top=RowTop;
        Btns[i]->Width=ButtonWidth;
        Btns[i]->Height=RowHeight;
        X+=ButtonWidth+Gap;
    }
}
//---------------------------------------------------------------------------
void __fastcall TMyMessageBox::FormShow(TObject *Sender)
{
    if(fMain!=NULL)
    {
        Left=fMain->Left+(fMain->Width-Width)/2;
        Top=fMain->Top+(fMain->Height-Height)/2;
    }
    else
    {
        Position=poScreenCenter;
    }

    if(bFormShowNoStop==false)
    {
        HSys.DecStopAllMotor();
        HSys.Sys.SystemStart=false;
    }

    //AI(HT160S-Maintainer) 20260622 : a fresh message box starts UN-muted so its
    //LED_Message buzzer (RadioGroup5) can sound, HT172 parity (bOffBuzzer reset).
    fBuzzerOff=false;
    fShow=true;

    //AI(HT160S-Maintainer) 20260624 : with the visible set now finalized (PrepareNormalMessage
    //or the YES/NO path ran before ShowModal/Show), lay the live buttons into an equal-width row.
    LayoutMessageButtons();

    //AI(HT160S-Maintainer) 20260622 : a modal message box suspends MainProc, so the per-scan
    //DoSystemMessage LED_Message buzzer driver never runs while it is up -> the message had no
    //audible cue. Kick it here. Gate on the Off Buzzer button: alarm-style messages show it,
    //YES/NO confirmations hide it and must stay silent (see PrepareNormalMessage).
    if(btnOffBuzzer->Visible)
        PlayMessageBuzzer();
}
//---------------------------------------------------------------------------
void __fastcall TMyMessageBox::FormClose(TObject *Sender, TCloseAction &Action)
{
    CloseBuzzerOff();
    fShow=false;
    fScanPanel=false;
    bFormShowNoStop=false;
}
//---------------------------------------------------------------------------
void __fastcall TMyMessageBox::btnOffBuzzerClick(TObject *Sender)
{
    //AI(HT160S-Maintainer) 20260622 : Off Buzzer -> latch mute so DoSystemMessage stops
    //re-driving the LED_Message buzzer every scan (one CloseBuzzerOff alone would be
    //overwritten next tick). HT172 mymessbox.cpp Button2Click bOffBuzzer parity.
    fBuzzerOff=true;
    CloseBuzzerOff();
}
//---------------------------------------------------------------------------
