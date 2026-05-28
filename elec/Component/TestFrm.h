//---------------------------------------------------------------------------
#ifndef TestFrmH
#define TestFrmH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class PACKAGE TTestForm : public TForm
{
  __published: // IDE-managed Components
    TLabel *Label1;
    TLabel *Label2;
    TLabel *Label3;
    TLabel *Label4;
private: // User declarations
    bool bbb;
protected: // User declarations
public: // User declarations
  __fastcall TTestForm(TComponent* Owner);
__published: // User declarations
    __property bool BBB = {read=bbb,write=bbb};
};
//---------------------------------------------------------------------------
extern PACKAGE TTestForm *TestForm;
//---------------------------------------------------------------------------
#endif