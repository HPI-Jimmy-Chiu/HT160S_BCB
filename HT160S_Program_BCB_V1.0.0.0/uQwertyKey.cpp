//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#pragma hdrstop

#include "uQwertyKey.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfQwertyKey *fQwertyKey;
//---------------------------------------------------------------------------
static const int QK_FULL_WIDTH=1005;
static const int QK_FULL_HEIGHT=469;
static const int QK_NUM_WIDTH=356;
//---------------------------------------------------------------------------
static AnsiString QwertyFormatDouble(double Value)
{
    AnsiString Text;

    Text.sprintf("%.6f", Value);
    while(Text.Length()>0 && Text[Text.Length()]=='0')
        Text.Delete(Text.Length(), 1);
    if(Text.Length()>0 && Text[Text.Length()]=='.')
        Text.Delete(Text.Length(), 1);
    if(Text==AnsiString(""))
        Text="0";
    return Text;
}
//---------------------------------------------------------------------------
static double QwertyClampDouble(double Value, double MinValue, double MaxValue)
{
    double LowValue;
    double HighValue;

    if(MinValue<MaxValue)
    {
        LowValue=MinValue;
        HighValue=MaxValue;
    }
    else
    {
        LowValue=MaxValue;
        HighValue=MinValue;
    }

    if(Value<LowValue)
        return LowValue;
    if(Value>HighValue)
        return HighValue;
    return Value;
}
//---------------------------------------------------------------------------
__fastcall TfQwertyKey::TfQwertyKey(TComponent* Owner)
    : TForm(Owner)
{
    int i;

    bUIBuilt=false;
    bAccepted=false;
    bShow=false;
    bUpperCase=false;
    bNoSymbol=false;
    bIntegerOnly=false;
    bCheckRange=false;
    KeyCode=0;
    iDecimalPoint=0;
    iKeyCount=0;
    dMinValue=0.0;
    dMaxValue=0.0;
    TargetEdit=NULL;
    for(i=0; i<MAX_QWERTY_KEYS; i++)
    {
        btnKeys[i]=NULL;
        KeyNormal[i]="";
        KeyShift[i]="";
        KeyType[i]=QKT_ALPHA;
    }
    BuildUI();
}
//---------------------------------------------------------------------------
void TfQwertyKey::BuildUI()
{
    if(bUIBuilt)
        return;
    bUIBuilt=true;

    RegisterQwertyKey(spbKey00, "`", "~", QKT_SYMBOL);
    RegisterQwertyKey(spbKey01, "1", "!", QKT_NUM_SYMBOL);
    RegisterQwertyKey(spbKey02, "2", "@", QKT_NUM_SYMBOL);
    RegisterQwertyKey(spbKey03, "3", "#", QKT_NUM_SYMBOL);
    RegisterQwertyKey(spbKey04, "4", "$", QKT_NUM_SYMBOL);
    RegisterQwertyKey(spbKey05, "5", "%", QKT_NUM_SYMBOL);
    RegisterQwertyKey(spbKey06, "6", "^", QKT_NUM_SYMBOL);
    RegisterQwertyKey(spbKey07, "7", "&", QKT_NUM_SYMBOL);
    RegisterQwertyKey(spbKey08, "8", "*", QKT_NUM_SYMBOL);
    RegisterQwertyKey(spbKey09, "9", "(", QKT_NUM_SYMBOL);
    RegisterQwertyKey(spbKey10, "0", ")", QKT_NUM_SYMBOL);
    RegisterQwertyKey(spbKey11, "-", "_", QKT_SYMBOL);
    RegisterQwertyKey(spbKey12, "=", "+", QKT_SYMBOL);
    RegisterQwertyKey(spbKey13, "q", "Q", QKT_ALPHA);
    RegisterQwertyKey(spbKey14, "w", "W", QKT_ALPHA);
    RegisterQwertyKey(spbKey15, "e", "E", QKT_ALPHA);
    RegisterQwertyKey(spbKey16, "r", "R", QKT_ALPHA);
    RegisterQwertyKey(spbKey17, "t", "T", QKT_ALPHA);
    RegisterQwertyKey(spbKey18, "y", "Y", QKT_ALPHA);
    RegisterQwertyKey(spbKey19, "u", "U", QKT_ALPHA);
    RegisterQwertyKey(spbKey20, "i", "I", QKT_ALPHA);
    RegisterQwertyKey(spbKey21, "o", "O", QKT_ALPHA);
    RegisterQwertyKey(spbKey22, "p", "P", QKT_ALPHA);
    RegisterQwertyKey(spbKey23, "[", "{", QKT_SYMBOL);
    RegisterQwertyKey(spbKey24, "]", "}", QKT_SYMBOL);
    RegisterQwertyKey(spbKey25, "\\", "|", QKT_SYMBOL);
    RegisterQwertyKey(spbKey26, "a", "A", QKT_ALPHA);
    RegisterQwertyKey(spbKey27, "s", "S", QKT_ALPHA);
    RegisterQwertyKey(spbKey28, "d", "D", QKT_ALPHA);
    RegisterQwertyKey(spbKey29, "f", "F", QKT_ALPHA);
    RegisterQwertyKey(spbKey30, "g", "G", QKT_ALPHA);
    RegisterQwertyKey(spbKey31, "h", "H", QKT_ALPHA);
    RegisterQwertyKey(spbKey32, "j", "J", QKT_ALPHA);
    RegisterQwertyKey(spbKey33, "k", "K", QKT_ALPHA);
    RegisterQwertyKey(spbKey34, "l", "L", QKT_ALPHA);
    RegisterQwertyKey(spbKey35, ";", ":", QKT_SYMBOL);
    RegisterQwertyKey(spbKey36, "'", "\"", QKT_SYMBOL);
    RegisterQwertyKey(spbKey37, "z", "Z", QKT_ALPHA);
    RegisterQwertyKey(spbKey38, "x", "X", QKT_ALPHA);
    RegisterQwertyKey(spbKey39, "c", "C", QKT_ALPHA);
    RegisterQwertyKey(spbKey40, "v", "V", QKT_ALPHA);
    RegisterQwertyKey(spbKey41, "b", "B", QKT_ALPHA);
    RegisterQwertyKey(spbKey42, "n", "N", QKT_ALPHA);
    RegisterQwertyKey(spbKey43, "m", "M", QKT_ALPHA);
    RegisterQwertyKey(spbKey44, ",", "<", QKT_SYMBOL);
    RegisterQwertyKey(spbKey45, ".", ">", QKT_SYMBOL);
    RegisterQwertyKey(spbKey46, "/", "?", QKT_SYMBOL);
    RegisterQwertyKey(spbKey47, " ", " ", QKT_SPACE);

    spbAdd1->Tag=101;
    spbMinus1->Tag=102;
    spbAdd10->Tag=103;
    spbMinus10->Tag=104;
    spbAdd100->Tag=105;
    spbMinus100->Tag=106;
    palFullKey->Visible=true;
    palNumKey->Visible=true;
}
//---------------------------------------------------------------------------
void TfQwertyKey::RegisterQwertyKey(TSpeedButton *Button, AnsiString NormalText, AnsiString ShiftText, int Type)
{

    if(Button==NULL || iKeyCount>=MAX_QWERTY_KEYS)
        return;
    Button->Tag=iKeyCount+1;
    btnKeys[iKeyCount]=Button;
    KeyNormal[iKeyCount]=NormalText;
    KeyShift[iKeyCount]=ShiftText;
    KeyType[iKeyCount]=Type;
    iKeyCount++;
}
//---------------------------------------------------------------------------
bool TfQwertyKey::IsNumericMode()
{
    return ((KeyCode&N_INTEGER) || (KeyCode&N_DOUBLE) || (KeyCode&N_NUM_PAD));
}
//---------------------------------------------------------------------------
void TfQwertyKey::ConfigureMode(AnsiString TitleText)
{
    bool NumericMode;

    NumericMode=IsNumericMode();
    Caption=(TitleText==AnsiString(""))?AnsiString("Qwerty Keyboard"):TitleText;
    labPrompt->Caption=Caption;
    bIntegerOnly=(KeyCode&N_INTEGER)!=0;
    bNoSymbol=(KeyCode&N_NO_SYMBOL)!=0;
    bUpperCase=(KeyCode&N_UPPERCASE)!=0;
    edContent->PasswordChar=(KeyCode&N_PASSWORD)?'*':0;
    palRange->Visible=bCheckRange && !(KeyCode&N_PASSWORD);
    edCurrent->Text=sBackup;
    if(bCheckRange)
    {
        edMin->Text=QwertyFormatDouble(dMinValue);
        edMax->Text=QwertyFormatDouble(dMaxValue);
    }
    if(NumericMode)
    {
        ClientWidth=QK_NUM_WIDTH;
        ClientHeight=QK_FULL_HEIGHT;
        edContent->Width=340;
        labPrompt->Width=340;
        palRange->Width=340;
        palKeyboardArea->Width=340;
        palFullKey->Visible=false;
        palNumKey->Visible=true;
    }
    else
    {
        ClientWidth=(KeyCode&N_NO_NUM_PAD)?745:QK_FULL_WIDTH;
        ClientHeight=QK_FULL_HEIGHT;
        edContent->Width=(KeyCode&N_NO_NUM_PAD)?729:989;
        labPrompt->Width=(KeyCode&N_NO_NUM_PAD)?729:989;
        palRange->Width=(KeyCode&N_NO_NUM_PAD)?729:989;
        palKeyboardArea->Width=(KeyCode&N_NO_NUM_PAD)?745:QK_FULL_WIDTH;
        palFullKey->Visible=true;
        palNumKey->Visible=!(KeyCode&N_NO_NUM_PAD);
    }
    ChangeDecimalPoint();
    UpdateKeyCaptions();
}
//---------------------------------------------------------------------------
void TfQwertyKey::UpdateKeyCaptions()
{
    int i;
    AnsiString Text;

    for(i=0; i<iKeyCount; i++)
    {
        Text=bUpperCase?KeyShift[i]:KeyNormal[i];
        if(bNoSymbol && KeyType[i]==QKT_NUM_SYMBOL)
            Text=KeyNormal[i];
        if(Text==AnsiString("&"))
            btnKeys[i]->Caption="&&";
        else
            btnKeys[i]->Caption=Text;
        btnKeys[i]->Enabled=!(bNoSymbol && KeyType[i]==QKT_SYMBOL);
        if(KeyType[i]==QKT_SPACE)
            btnKeys[i]->Visible=!(KeyCode&N_NO_SPACE);
    }
}
//---------------------------------------------------------------------------
void TfQwertyKey::ChangeDecimalPoint()
{
    int i;
    TSpeedButton *Button;

    if(bIntegerOnly && iDecimalPoint>1)
        iDecimalPoint=1;

    if(palNumKey==NULL)
        return;
    for(i=0; i<palNumKey->ControlCount; i++)
    {
        Button=dynamic_cast<TSpeedButton *>(palNumKey->Controls[i]);
        if(Button==NULL)
            continue;
        switch(iDecimalPoint)
        {
            case 0:
                if(Button->Tag==101) Button->Caption="+10";
                if(Button->Tag==102) Button->Caption="-10";
                if(Button->Tag==103) Button->Caption="+100";
                if(Button->Tag==104) Button->Caption="-100";
                if(Button->Tag==105) Button->Caption="+1000";
                if(Button->Tag==106) Button->Caption="-1000";
                break;
            case 1:
                if(Button->Tag==101) Button->Caption="+1";
                if(Button->Tag==102) Button->Caption="-1";
                if(Button->Tag==103) Button->Caption="+10";
                if(Button->Tag==104) Button->Caption="-10";
                if(Button->Tag==105) Button->Caption="+100";
                if(Button->Tag==106) Button->Caption="-100";
                break;
            case 2:
                if(Button->Tag==101) Button->Caption="+1.0";
                if(Button->Tag==102) Button->Caption="-1.0";
                if(Button->Tag==103) Button->Caption="+0.1";
                if(Button->Tag==104) Button->Caption="-0.1";
                if(Button->Tag==105) Button->Caption="+0.01";
                if(Button->Tag==106) Button->Caption="-0.01";
                break;
            case 3:
                if(Button->Tag==101) Button->Caption="+0.1";
                if(Button->Tag==102) Button->Caption="-0.1";
                if(Button->Tag==103) Button->Caption="+0.01";
                if(Button->Tag==104) Button->Caption="-0.01";
                if(Button->Tag==105) Button->Caption="+0.001";
                if(Button->Tag==106) Button->Caption="-0.001";
                break;
        }
    }
}
//---------------------------------------------------------------------------
void TfQwertyKey::InsertContent(AnsiString Text)
{
    AnsiString OldText;
    int StartPos;
    int DeleteLen;

    if(edContent==NULL)
        return;
    OldText=edContent->Text;
    StartPos=edContent->SelStart;
    DeleteLen=edContent->SelLength;
    if(DeleteLen>0)
        OldText.Delete(StartPos+1, DeleteLen);
    OldText.Insert(Text, StartPos+1);
    edContent->Text=OldText;
    edContent->SelStart=StartPos+Text.Length();
}
//---------------------------------------------------------------------------
void TfQwertyKey::BackspaceContent()
{
    AnsiString OldText;
    int StartPos;
    int DeleteLen;

    if(edContent==NULL)
        return;
    OldText=edContent->Text;
    StartPos=edContent->SelStart;
    DeleteLen=edContent->SelLength;
    if(DeleteLen>0)
    {
        OldText.Delete(StartPos+1, DeleteLen);
    }
    else if(StartPos>0)
    {
        OldText.Delete(StartPos, 1);
        StartPos--;
    }
    edContent->Text=OldText;
    edContent->SelStart=StartPos;
}
//---------------------------------------------------------------------------
bool TfQwertyKey::ValidateAndClamp(AnsiString &Text)
{
    double Value;

    if(IsNumericMode())
    {
        if(Text==AnsiString("") || Text==AnsiString("-") || Text==AnsiString(".") || Text==AnsiString("-."))
        {
            MessageDlg("Input value is invalid.", mtWarning, TMsgDlgButtons() << mbOK, 0);
            return false;
        }
        Value=atof(Text.c_str());
        if(bCheckRange)
            Value=QwertyClampDouble(Value, dMinValue, dMaxValue);
        if(bIntegerOnly)
            Text=IntToStr((int)Value);
        else
            Text=QwertyFormatDouble(Value);
    }
    return true;
}
//---------------------------------------------------------------------------
bool __fastcall TfQwertyKey::ShowQwertyKey(TCustomEdit *Edit, int Function, int DecimalPoint, bool CheckRange, double MinValue, double MaxValue, AnsiString TitleText)
{
    AnsiString Text;

    if(Edit==NULL)
        return false;
    if(bShow)
        return false;
    TargetEdit=Edit;
    KeyCode=Function;
    iDecimalPoint=DecimalPoint;
    bCheckRange=CheckRange;
    dMinValue=MinValue;
    dMaxValue=MaxValue;
    bAccepted=false;
    sBackup=Edit->Text;
    edContent->Text=sBackup;
    edContent->SelStart=edContent->Text.Length();
    ConfigureMode(TitleText);
    ShowModal();
    Text=edContent->Text;
    if(bAccepted)
    {
        TargetEdit->Text=Text;
    }
    TargetEdit=NULL;
    return bAccepted;
}
//---------------------------------------------------------------------------
void __fastcall TfQwertyKey::FormShow(TObject *Sender)
{
    (void)Sender;
    bShow=true;
    if(edContent!=NULL)
        edContent->SetFocus();
}
//---------------------------------------------------------------------------
void __fastcall TfQwertyKey::FormClose(TObject *Sender, TCloseAction &Action)
{
    (void)Sender;
    (void)Action;
    bShow=false;
}
//---------------------------------------------------------------------------
void __fastcall TfQwertyKey::edContentKeyPress(TObject *Sender, char &Key)
{
    AnsiString Text;

    (void)Sender;
    Text=edContent->Text;
    if(Key==13)
    {
        Key=0;
        btnOkClick(this);
        return;
    }
    if(Key==8)
        return;
    if(IsNumericMode())
    {
        if(Key>='0' && Key<='9')
            return;
        if(Key=='-' && edContent->SelStart==0 && Text.Pos("-")==0)
            return;
        if((KeyCode&N_DOUBLE) && Key=='.' && Text.Pos(".")==0)
            return;
        Key=0;
        return;
    }
    if((KeyCode&N_NO_SYMBOL) && !((Key>='0' && Key<='9') || (Key>='a' && Key<='z') || (Key>='A' && Key<='Z') || Key=='.'))
    {
        Key=0;
        return;
    }
    if((KeyCode&N_NO_SPACE) && Key==' ')
        Key=0;
}
//---------------------------------------------------------------------------
void __fastcall TfQwertyKey::btnKeyClick(TObject *Sender)
{
    TSpeedButton *Button;
    int Index;
    AnsiString Text;

    Button=(TSpeedButton *)Sender;
    if(Button==NULL)
        return;
    if(Button->Tag>0)
    {
        Index=Button->Tag-1;
        if(Index<0 || Index>=iKeyCount)
            return;
        Text=bUpperCase?KeyShift[Index]:KeyNormal[Index];
        if(bNoSymbol && KeyType[Index]==QKT_NUM_SYMBOL)
            Text=KeyNormal[Index];
    }
    else
    {
        Text=Button->Caption;
    }
    InsertContent(Text);
}
//---------------------------------------------------------------------------
void __fastcall TfQwertyKey::btnBackspaceClick(TObject *Sender)
{
    (void)Sender;
    BackspaceContent();
}
//---------------------------------------------------------------------------
void __fastcall TfQwertyKey::btnClearClick(TObject *Sender)
{
    (void)Sender;
    edContent->Text="";
}
//---------------------------------------------------------------------------
void __fastcall TfQwertyKey::btnOkClick(TObject *Sender)
{
    AnsiString Text;

    (void)Sender;
    Text=edContent->Text;
    if(ValidateAndClamp(Text)==false)
        return;
    edContent->Text=Text;
    bAccepted=true;
    ModalResult=mrOk;
}
//---------------------------------------------------------------------------
void __fastcall TfQwertyKey::btnCancelClick(TObject *Sender)
{
    (void)Sender;
    bAccepted=false;
    edContent->Text=sBackup;
    ModalResult=mrCancel;
}
//---------------------------------------------------------------------------
void __fastcall TfQwertyKey::btnChangeCaseClick(TObject *Sender)
{
    (void)Sender;
    bUpperCase=!bUpperCase;
    UpdateKeyCaptions();
}
//---------------------------------------------------------------------------
void __fastcall TfQwertyKey::btnMinusClick(TObject *Sender)
{
    AnsiString Text;

    (void)Sender;
    Text=edContent->Text;
    if(Text.Pos("-")==1)
        Text.Delete(1, 1);
    else
        Text=AnsiString("-")+Text;
    edContent->Text=Text;
    edContent->SelStart=edContent->Text.Length();
}
//---------------------------------------------------------------------------
void __fastcall TfQwertyKey::btnDecimalClick(TObject *Sender)
{
    (void)Sender;
    if(bIntegerOnly)
        return;
    if(edContent->Text.Pos(".")==0)
        InsertContent(".");
}
//---------------------------------------------------------------------------
void __fastcall TfQwertyKey::btnStepClick(TObject *Sender)
{
    TSpeedButton *Button;
    double Value;
    double AddValue;

    Button=(TSpeedButton *)Sender;
    if(Button==NULL)
        return;
    AddValue=atof(Button->Caption.c_str());
    Value=atof(edContent->Text.c_str())+AddValue;
    if(bIntegerOnly)
        edContent->Text=IntToStr((int)Value);
    else
        edContent->Text=QwertyFormatDouble(Value);
    edContent->SelStart=edContent->Text.Length();
}
//---------------------------------------------------------------------------
void __fastcall TfQwertyKey::btnDecimalModeClick(TObject *Sender)
{
    (void)Sender;
    iDecimalPoint++;
    if(iDecimalPoint>3)
        iDecimalPoint=0;
    ChangeDecimalPoint();
}
//---------------------------------------------------------------------------
