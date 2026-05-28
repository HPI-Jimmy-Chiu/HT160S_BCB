//---------------------------------------------------------------------------
#ifndef systoolsH
#define systoolsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TFormSysTools : public TForm
{
__published:
private:
public:
    __fastcall TFormSysTools(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TFormSysTools *FormSysTools;
//---------------------------------------------------------------------------
#endif
