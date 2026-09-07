//---------------------------------------------------------------------------
#ifndef systoolsH
#define systoolsH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
//---------------------------------------------------------------------------
//AI(ht160s-statusbar) 20260624 : time-string show subsystem ported from HT172
//systools.h:158-188 / systools.cpp:996-1053. A registered TStatusPanel/TControl is
//refreshed with the current time once per changed second by RefreshMyTimeString(),
//driven from the always-on TDataModule1::Timer1 (100ms, every 10th tick = 1 Hz).
//HT160 has no System* time globals, so RefreshMyTimeString reads Now()/DecodeDateTime
//directly instead of HT172's SystemYear/Month/.../Sec.
typedef struct MyTimeStringShowStruct
{
    TObject *palPtr;
    int      dataType;  // 0: yyyy-mm-dd hh:mm:ss (24h), 1: with AM/PM
}MyTimeStringShowList;
//---------------------------------------------------------------------------
class TFormSysTools : public TForm
{
__published:
private:
public:
    __fastcall TFormSysTools(TComponent* Owner);
    TList *ShowTimeStringList;
    void __fastcall AddMyTimeStringShow(TObject *RefPanelPtr, int DataType);
    void __fastcall RefreshMyTimeString();
};
//---------------------------------------------------------------------------
extern PACKAGE TFormSysTools *FormSysTools;
//---------------------------------------------------------------------------
#endif
