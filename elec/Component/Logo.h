//---------------------------------------------------------------------------
#ifndef logoH
#define logoH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Graphics.hpp>

//---------------------------------------------------------------------------
class TLogoForm : public TForm
{
__published:	// IDE-managed Components
    TPanel *Panel1;
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label3;
    TLabel *Label4;
    TImage *Image1;
private:	// User declarations
public:		// User declarations
    __fastcall TLogoForm(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TLogoForm *LogoForm;
//---------------------------------------------------------------------------
#endif
