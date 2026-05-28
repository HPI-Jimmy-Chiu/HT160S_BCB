//---------------------------------------------------------------------------
#ifndef HDBLookupComboBoxH
#define HDBLookupComboBoxH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include "RXLookup.hpp"
//---------------------------------------------------------------------------
class PACKAGE THDBLookupComboBox : public TRxDBLookupCombo
{
private:
protected:
    virtual void __fastcall WndProc(Messages::TMessage &Message);
public:
    __fastcall THDBLookupComboBox(TComponent* Owner);
    DYNAMIC void __fastcall KeyDown(Word &Key,Classes::TShiftState Shift);
__published:
};
//---------------------------------------------------------------------------
#endif
