//---------------------------------------------------------------------------
#ifndef uQwertyKeyH
#define uQwertyKeyH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <Buttons.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
enum EQwertyKeyFunction
{
    QK_INTEGER    =0x0001,
    QK_DOUBLE     =0x0002,
    QK_NO_SYMBOL  =0x0004,
    QK_PASSWORD   =0x0008,
    QK_NO_SPACE   =0x0010,
    QK_UPPERCASE  =0x0020,
    QK_NO_NUM_PAD =0x0040,
    QK_NUM_PAD    =0x0080,
    N_INTEGER     =QK_INTEGER,
    N_DOUBLE      =QK_DOUBLE,
    N_NO_SYMBOL   =QK_NO_SYMBOL,
    N_PASSWORD    =QK_PASSWORD,
    N_NO_SPACE    =QK_NO_SPACE,
    N_UPPERCASE   =QK_UPPERCASE,
    N_NO_NUM_PAD  =QK_NO_NUM_PAD,
    N_NUM_PAD     =QK_NUM_PAD
};
//---------------------------------------------------------------------------
class TfQwertyKey : public TForm
{
__published:
    TLabel *labPrompt;
    TEdit *edContent;
    TPanel *palRange;
    TLabel *labCurrent;
    TLabel *labMin;
    TLabel *labMax;
    TEdit *edCurrent;
    TEdit *edMin;
    TEdit *edMax;
    TPanel *palKeyboardArea;
    TPanel *palFullKey;
    TSpeedButton *spbKey00;
    TSpeedButton *spbKey01;
    TSpeedButton *spbKey02;
    TSpeedButton *spbKey03;
    TSpeedButton *spbKey04;
    TSpeedButton *spbKey05;
    TSpeedButton *spbKey06;
    TSpeedButton *spbKey07;
    TSpeedButton *spbKey08;
    TSpeedButton *spbKey09;
    TSpeedButton *spbKey10;
    TSpeedButton *spbKey11;
    TSpeedButton *spbKey12;
    TSpeedButton *spbKey13;
    TSpeedButton *spbKey14;
    TSpeedButton *spbKey15;
    TSpeedButton *spbKey16;
    TSpeedButton *spbKey17;
    TSpeedButton *spbKey18;
    TSpeedButton *spbKey19;
    TSpeedButton *spbKey20;
    TSpeedButton *spbKey21;
    TSpeedButton *spbKey22;
    TSpeedButton *spbKey23;
    TSpeedButton *spbKey24;
    TSpeedButton *spbKey25;
    TSpeedButton *spbKey26;
    TSpeedButton *spbKey27;
    TSpeedButton *spbKey28;
    TSpeedButton *spbKey29;
    TSpeedButton *spbKey30;
    TSpeedButton *spbKey31;
    TSpeedButton *spbKey32;
    TSpeedButton *spbKey33;
    TSpeedButton *spbKey34;
    TSpeedButton *spbKey35;
    TSpeedButton *spbKey36;
    TSpeedButton *spbKey37;
    TSpeedButton *spbKey38;
    TSpeedButton *spbKey39;
    TSpeedButton *spbKey40;
    TSpeedButton *spbKey41;
    TSpeedButton *spbKey42;
    TSpeedButton *spbKey43;
    TSpeedButton *spbKey44;
    TSpeedButton *spbKey45;
    TSpeedButton *spbKey46;
    TSpeedButton *spbKey47;
    TSpeedButton *spbChangeCase;
    TSpeedButton *spbBackspace;
    TSpeedButton *spbClear;
    TSpeedButton *spbCancel;
    TSpeedButton *spbOk;
    TPanel *palNumKey;
    TSpeedButton *spbNum7;
    TSpeedButton *spbNum8;
    TSpeedButton *spbNum9;
    TSpeedButton *spbNum4;
    TSpeedButton *spbNum5;
    TSpeedButton *spbNum6;
    TSpeedButton *spbNum1;
    TSpeedButton *spbNum2;
    TSpeedButton *spbNum3;
    TSpeedButton *spbNum0;
    TSpeedButton *spbDecimal;
    TSpeedButton *spbMinus;
    TSpeedButton *spbStepMode;
    TSpeedButton *spbNumBackspace;
    TSpeedButton *spbNumClear;
    TSpeedButton *spbNumCancel;
    TSpeedButton *spbNumOk;
    TSpeedButton *spbAdd1;
    TSpeedButton *spbMinus1;
    TSpeedButton *spbAdd10;
    TSpeedButton *spbMinus10;
    TSpeedButton *spbAdd100;
    TSpeedButton *spbMinus100;
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall edContentKeyPress(TObject *Sender, char &Key);
    void __fastcall btnKeyClick(TObject *Sender);
    void __fastcall btnBackspaceClick(TObject *Sender);
    void __fastcall btnClearClick(TObject *Sender);
    void __fastcall btnOkClick(TObject *Sender);
    void __fastcall btnCancelClick(TObject *Sender);
    void __fastcall btnChangeCaseClick(TObject *Sender);
    void __fastcall btnMinusClick(TObject *Sender);
    void __fastcall btnDecimalClick(TObject *Sender);
    void __fastcall btnStepClick(TObject *Sender);
    void __fastcall btnDecimalModeClick(TObject *Sender);
private:
    enum { MAX_QWERTY_KEYS=48 };
    enum { QKT_ALPHA=0, QKT_SYMBOL=1, QKT_NUM_SYMBOL=2, QKT_SPACE=3 };

    bool bUIBuilt;
    bool bAccepted;
    bool bShow;
    bool bUpperCase;
    bool bNoSymbol;
    bool bIntegerOnly;
    bool bCheckRange;
    int KeyCode;
    int iDecimalPoint;
    int iKeyCount;
    double dMinValue;
    double dMaxValue;
    AnsiString sBackup;
    TCustomEdit *TargetEdit;

    TSpeedButton *btnKeys[MAX_QWERTY_KEYS];
    AnsiString KeyNormal[MAX_QWERTY_KEYS];
    AnsiString KeyShift[MAX_QWERTY_KEYS];
    int KeyType[MAX_QWERTY_KEYS];

    void BuildUI();
    void RegisterQwertyKey(TSpeedButton *Button, AnsiString NormalText, AnsiString ShiftText, int Type);
    void UpdateKeyCaptions();
    void ConfigureMode(AnsiString TitleText);
    void ChangeDecimalPoint();
    void InsertContent(AnsiString Text);
    void BackspaceContent();
    bool ValidateAndClamp(AnsiString &Text);
    bool IsNumericMode();
public:
    __fastcall TfQwertyKey(TComponent* Owner);
    bool __fastcall ShowQwertyKey(TCustomEdit *Edit, int Function, int DecimalPoint=0, bool CheckRange=false, double MinValue=0, double MaxValue=0, AnsiString TitleText="");
};
//---------------------------------------------------------------------------
extern PACKAGE TfQwertyKey *fQwertyKey;
//---------------------------------------------------------------------------
#endif
