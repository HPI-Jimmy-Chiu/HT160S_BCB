//---------------------------------------------------------------------------
#ifndef uspeedH
#define uspeedH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include <Forms.hpp>
#include <vector>
#include "database.h"
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260602 : HT172 0420 Speed module port.
//  Free helpers mirror HT172 cinitial.cpp SetMotorSpeed plus machine-level
//  INI persistence (HT172 used FormSysTools which is an empty stub in HT160,
//  so persistence is done with the HT160 TIniFile idiom instead).
//---------------------------------------------------------------------------
void SetMotorSpeed(bool bSet=false);    // re-apply each motor working percentage
void LoadMotorSpeedFromIni();           // load + apply baseline at program start
void SaveMotorSpeedToIni();             // persist current percentages
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260602 : one editable row (label + edit + scrollbar)
//  per motor, built dynamically (the .dfm is an empty placeholder). The wrapper
//  holds raw pointers to VCL controls that are owned by the host ScrollBox, so
//  VCL frees the controls; only the wrapper objects are deleted by the form.
//---------------------------------------------------------------------------
class TMySpeedPanel
{
public:
    TMySpeedPanel(TTrayMotor *Mot, TWinControl *Host);  //AI(HT160S-Maintainer) 20260602 : Host passed in (global fSpeed not yet set during FormCreate)
    ~TMySpeedPanel();

    TPanel      *palMotorSpeed;
    TLabel      *labMotorSpeed;
    TEdit       *edtMotorSpeed;
    TScrollBar  *scbMotorSpeed;
    TTrayMotor  *Motor;
    bool         bEnable;
    bool         bUpdating;

    void SyncFromMotor();
    void Apply(int persent);
    void __fastcall scbMotorSpeedChange(TObject *Sender);
    void __fastcall edtMotorSpeedExit(TObject *Sender);
};
//---------------------------------------------------------------------------
class TfSpeed : public TForm
{
__published:    // DFM-wired controls and event handlers
    TPanel      *Panel10;
    TButton     *spbFullSpeed;
    TButton     *spbAddSpeed;
    TButton     *spbSubSpeed;
    TButton     *spbExit;
    TScrollBox  *ScrollBox1;
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall FormDestroy(TObject *Sender);
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall spbFullSpeedClick(TObject *Sender);
    void __fastcall spbAddSpeedClick(TObject *Sender);
    void __fastcall spbSubSpeedClick(TObject *Sender);
    void __fastcall spbExitClick(TObject *Sender);
private:
    std::vector<TMySpeedPanel *> MySpeedPanel;

    void BuildPanels();
    void GroupSpeedAddSub(int flag);    // 0=sub, 1=add, 2=full
public:
    __fastcall TfSpeed(TComponent* Owner);
    void RefreshAll();
};
//---------------------------------------------------------------------------
extern PACKAGE TfSpeed *fSpeed;
//---------------------------------------------------------------------------
#endif
