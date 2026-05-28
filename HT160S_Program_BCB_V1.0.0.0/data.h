//---------------------------------------------------------------------------
#ifndef dataH
#define dataH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class TfData : public TForm
{
__published:
private:
public:
    __fastcall TfData(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfData *fData;
//---------------------------------------------------------------------------
#endif
