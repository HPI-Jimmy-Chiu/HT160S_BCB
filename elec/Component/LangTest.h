//---------------------------------------------------------------------------
#ifndef LangTestH
#define LangTestH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include "HLanguage.h"
//---------------------------------------------------------------------------
class TFrmLangTest : public TForm
{
__published:	// IDE-managed Components
    TPageControl *PageControl1;
    TTabSheet *TabSheet1;
    TTabSheet *TabSheet2;
    TTabSheet *TabSheet3;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label3;
    TLabel *Label4;
    TButton *Button1;
    TButton *Button2;
    TButton *Button3;
    TRadioButton *RadioButton1;
    TRadioButton *RadioButton2;
    TRadioButton *RadioButton3;
    TSpeedButton *SpeedButton1;
    TSpeedButton *SpeedButton2;
    TSpeedButton *SpeedButton3;
    TBitBtn *BitBtn1;
    TBitBtn *BitBtn2;
    HLanguage *HLanguage1;
    void __fastcall BitBtn2Click(TObject *Sender);
private:	// User declarations
    bool flag;
public:		// User declarations
    __fastcall TFrmLangTest(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFrmLangTest *FrmLangTest;
//---------------------------------------------------------------------------
#endif
