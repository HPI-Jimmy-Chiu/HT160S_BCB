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
//AI(HT160S-Maintainer) 20260629 : button-row Top, recomputed by FitMessageBox so the
//button row follows the (possibly grown) Panel1 height. LayoutMessageButtons reads it
//instead of the old hard-coded 178 so wrapped/taller messages do not overlap the buttons.
static int gButtonRowTop=178;
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
    int RowTop=gButtonRowTop;
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
//AI(HT160S-Maintainer) 20260629 : measure the wrapped pixel height of S when drawn into a
//Width-wide rect with the given font. Uses Win32 DrawText DT_CALCRECT+DT_WORDBREAK so it
//matches how a WordWrap TLabel lays the same text out. Returns 0 for empty text/zero width.
//---------------------------------------------------------------------------
static int MeasureWrapHeight(TCanvas *Cv, TFont *Fnt, AnsiString S, int Width)
{
    RECT R;

    if(S=="" || Width<=0)
        return 0;
    Cv->Font->Assign(Fnt);
    R.left=0;
    R.top=0;
    R.right=Width;
    R.bottom=0;
    DrawText(Cv->Handle, S.c_str(), -1, &R, DT_CALCRECT|DT_WORDBREAK|DT_LEFT);
    return R.bottom-R.top;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260701 : widest single visual line of S, in pixels, for the given
//font. Cv->TextWidth() measures a whole string as one run, so an explicit multi-line caption
//(built by a caller joining lines with "\r\n", e.g. the motor-parameter-change preview list)
//returned the SUM of all lines' widths instead of the widest one, oversizing the box. Split on
//CRLF/LF first and take the max per-line width instead.
//---------------------------------------------------------------------------
static int MeasureMaxLineWidth(TCanvas *Cv, TFont *Fnt, AnsiString S)
{
    TStringList *Lines;
    int MaxW;
    int LineW;
    int Index;

    if(S=="")
        return 0;
    Cv->Font->Assign(Fnt);
    Lines=new TStringList();
    Lines->Text=S;
    MaxW=0;
    for(Index=0; Index<Lines->Count; Index++)
    {
        LineW=Cv->TextWidth(Lines->Strings[Index]);
        if(LineW>MaxW)
            MaxW=LineW;
    }
    delete Lines;
    return MaxW;
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260629 : auto-fit the message box to its content. The DFM
//Label1/Label2 are fixed 441px, single-line, centered labels, so a long caption (e.g. the
//startup "Inherit last work order ? ..." YES/NO prompt) overflowed and got clipped on BOTH
//ends. Here we measure the caption(s) with the label font and grow the form Width to fit, up
//to a screen-bounded cap. If even the cap is too narrow, we switch the labels to WordWrap and
//grow the height instead, vertically centering the text block inside Panel1. The button-row
//Top (gButtonRowTop) and the form ClientHeight follow the panel, so LayoutMessageButtons and
//the FormShow centering both adapt with no caller changes. Runs every FormShow;
//PrepareNormalMessage resets Width/Height first, so each show recomputes from a clean
//baseline (no drift across reuses of the singleton box).
//AI(HT160S-Maintainer) 20260701 : height is now ALWAYS measured via MeasureWrapHeight
//(DrawText DT_CALCRECT), never hard-coded to the single-line 33px. The old code only measured
//when the caption was wide enough to force auto-wrap; a caller-built explicit multi-line
//caption (e.g. "Motor parameter changes: N\r\n<line>...") is short per-line so it never hit
//that path, leaving H1=33 while the label actually rendered 2+ lines -- clipping every line
//past the first. WordWrap is likewise always turned on; DrawText/TLabel honor embedded
//"\r\n" regardless, and turning it on is a no-op when no line needs wrapping.
//---------------------------------------------------------------------------
static void FitMessageBox()
{
    TMyMessageBox *M;
    TCanvas *Cv;
    AnsiString S1;
    AnsiString S2;
    bool Has2;
    int W1;
    int W2;
    int TextW;
    int PanelW;
    int LabelW;
    int H1;
    int H2;
    int ContentH;
    int PanelH;
    int BlockTop;
    int DesiredClient;
    int MaxClient;

    const int LabelPad=30;     // breathing room added to the measured text width
    const int LabelInset=16;   // label sits 8px in from each panel edge
    const int PanelInset=16;   // panel sits 8px in from each client edge
    const int MinClient=474;   // original ClientWidth baseline
    const int LineGap=12;      // vertical gap between Label1 and Label2 blocks
    const int PanelTop=8;
    const int RowGap=17;       // gap between Panel1 bottom and the button row
    const int RowHeight=33;
    const int BottomMargin=38;
    const int MinPanelH=153;   // original Panel1 height baseline

    if(MyMessageBox==NULL)
        return;
    M=MyMessageBox;
    Cv=M->Canvas;
    S1=M->Label1->Caption;
    Has2=(M->Label2->Visible && M->Label2->Caption!="");
    S2=Has2 ? M->Label2->Caption : AnsiString("");

    W1=MeasureMaxLineWidth(Cv, M->Label1->Font, S1);
    W2=Has2 ? MeasureMaxLineWidth(Cv, M->Label2->Font, S2) : 0;
    TextW=(W1>W2) ? W1 : W2;

    MaxClient=Screen->WorkAreaWidth-40;
    if(MaxClient>900)
        MaxClient=900;
    if(MaxClient<MinClient)
        MaxClient=MinClient;

    DesiredClient=TextW+LabelPad+LabelInset+PanelInset;
    if(DesiredClient<MinClient)
        DesiredClient=MinClient;

    if(DesiredClient>MaxClient)
        DesiredClient=MaxClient;

    M->ClientWidth=DesiredClient;
    PanelW=DesiredClient-PanelInset;
    LabelW=PanelW-LabelInset;
    M->Panel1->Left=8;
    M->Panel1->Width=PanelW;
    M->Label1->Left=8;
    M->Label1->Width=LabelW;
    M->Label2->Left=8;
    M->Label2->Width=LabelW;
    M->Label1->WordWrap=true;
    M->Label2->WordWrap=true;

    H1=MeasureWrapHeight(Cv, M->Label1->Font, S1, LabelW)+4;
    if(H1<33)
        H1=33;
    H2=0;
    if(Has2)
    {
        H2=MeasureWrapHeight(Cv, M->Label2->Font, S2, LabelW)+4;
        if(H2<33)
            H2=33;
    }

    ContentH=H1+(Has2 ? (LineGap+H2) : 0);
    PanelH=ContentH+80;
    if(PanelH<MinPanelH)
        PanelH=MinPanelH;

    BlockTop=(PanelH-ContentH)/2;
    if(BlockTop<8)
        BlockTop=8;

    M->Panel1->Top=PanelTop;
    M->Panel1->Height=PanelH;
    M->Label1->Top=BlockTop;
    M->Label1->Height=H1;
    if(Has2)
    {
        M->Label2->Top=BlockTop+H1+LineGap;
        M->Label2->Height=H2;
    }

    gButtonRowTop=PanelTop+PanelH+RowGap;
    M->ClientHeight=gButtonRowTop+RowHeight+BottomMargin;
}
//---------------------------------------------------------------------------
void __fastcall TMyMessageBox::FormShow(TObject *Sender)
{
    if(bFormShowNoStop==false)
    {
        HSys.DecStopAllMotor();
        HSys.Sys.SystemStart=false;
    }

    //AI(HT160S-Maintainer) 20260622 : a fresh message box starts UN-muted so its
    //LED_Message buzzer (RadioGroup5) can sound, HT172 parity (bOffBuzzer reset).
    fBuzzerOff=false;
    fShow=true;

    //AI(ht160s-panel-sensitivity) 20260706 : pump the physical-panel key scan + buzzer/lamp
    //keepalive while this modal box suspends MainProc (HT172 mymessbox Timer1, 10ms).
    Timer1->Enabled=true;

    //AI(HT160S-Maintainer) 20260629 : auto-fit the box to its caption (grow Width, or
    //WordWrap+grow Height for very long text) BEFORE centering and the button reflow, so the
    //final Width/Height drive both. Fixes long captions being clipped on both ends.
    FitMessageBox();

    //AI(HT160S-Maintainer) 20260624 : with the visible set now finalized (PrepareNormalMessage
    //or the YES/NO path ran before ShowModal/Show), lay the live buttons into an equal-width row.
    LayoutMessageButtons();

    //AI(HT160S-Maintainer) 20260629 : center using the post-fit Width/Height (was the first
    //thing FormShow did, but FitMessageBox now changes the size, so center afterwards).
    if(fMain!=NULL)
    {
        Left=fMain->Left+(fMain->Width-Width)/2;
        Top=fMain->Top+(fMain->Height-Height)/2;
    }
    else
    {
        Position=poScreenCenter;
    }

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
    Timer1->Enabled=false;
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
void __fastcall TMyMessageBox::Timer1Timer(TObject *Sender)
{
    //AI(ht160s-panel-sensitivity) 20260706 : self-pump so the physical operator panel works
    //while this modal box suspends MainProc (HT172 mymessbox.cpp Timer1Timer port). Reentry-
    //guarded. Key scan runs only for alarm/info boxes (fScanPanel); YES/NO + OK stay touch-
    //only (fScanPanel=false), matching HT172 bDisableKeypad. DoSystemMessage() keeps the
    //LED_Message buzzer + panel lamps alive; fBuzzerOff is honored by both.
    static bool bMessageTimer1Check=false;

    if(bMessageTimer1Check)
        return;
    bMessageTimer1Check=true;

    if(fShow==false)
    {
        bMessageTimer1Check=false;
        return;
    }

    if(fScanPanel)
        ScanKey();

    if(fBuzzerOff)
        CloseBuzzerOff();
    DoSystemMessage();

    bMessageTimer1Check=false;
}
//---------------------------------------------------------------------------
void __fastcall TMyMessageBox::ScanKey()
{
    //AI(ht160s-panel-sensitivity) 20260706 : HT160 sensors carry no Tag, so read SnFK*||SnRK*
    //directly and rising-edge latch so a held key fires once (note.cpp/main ScanKey idiom).
    //Physical PAUSE closes the box (ret=PAUSE); physical ALARM RESET latches the buzzer mute.
    //Only reached for fScanPanel boxes -> YES/NO stays touch-only (HT172 parity).
    static bool bWasPause=false;
    static bool bWasReset=false;

    bool bPause=HSys.Sen.SnFKPause.IsOn()      || HSys.Sen.SnRKPause.IsOn();
    bool bReset=HSys.Sen.SnFKAlarmReset.IsOn() || HSys.Sen.SnRKAlarmReset.IsOn();

    if(bPause && bWasPause==false)
    {
        ret=msgrtnPAUSE;
        Close();
    }
    else if(bReset && bWasReset==false)
    {
        fBuzzerOff=true;
        CloseBuzzerOff();
    }

    bWasPause=bPause;
    bWasReset=bReset;
}
//---------------------------------------------------------------------------
