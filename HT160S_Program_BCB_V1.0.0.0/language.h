//---------------------------------------------------------------------------
#ifndef languageH
#define languageH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TfLan : public TForm
{
__published:
private:
public:
    __fastcall TfLan(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfLan *fLan;
//---------------------------------------------------------------------------
#endif
