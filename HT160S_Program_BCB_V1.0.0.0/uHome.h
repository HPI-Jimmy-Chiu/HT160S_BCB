//---------------------------------------------------------------------------
// uHome.h - Full machine HOME monitor form for HT160S
// AI(HT160S-Maintainer) 20260602 : Home dashboard ported in 172 style.
//   Press Home -> start full-machine home (reuse Run_Home engine), the
//   monitor auto-shows during homing with live TALed LEDs + motor
//   positions, and auto-closes when homing finishes. "Abort Home" stops.
//   No homing FSM is re-implemented; the existing Run_Home engine drives
//   the motion and this form only triggers it and shows feedback.
//---------------------------------------------------------------------------
#ifndef uHomeH
#define uHomeH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <Buttons.hpp>
#include "aled.hpp"
//---------------------------------------------------------------------------
#define HOME_MOTOR_MAX 128
//---------------------------------------------------------------------------
class TfHome : public TForm
{
__published:
    TPanel       *Panel104;
    TPanel       *Panel1;
    TListBox     *lstHomeMsg;
    TPanel       *Panel2;
    TSpeedButton *SpeedButton1;
    TTimer       *Timer1;
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall Timer1Timer(TObject *Sender);
    void __fastcall SpeedButton1Click(TObject *Sender);
private:
    TLabel *LabelMotorName[HOME_MOTOR_MAX];
    TALed  *LedPtr[HOME_MOTOR_MAX];
    TEdit  *EditMotorPos[HOME_MOTOR_MAX];
    bool    fShow;
    bool    fSeenStart;
    bool    fGridBuilt;
    void BuildMotorGrid();
    void __fastcall ScanKey();
public:
    enum eHomeLedColor { eHomeUnuse=0, eHomeOk=1, eHomeError=2, eHomeBusy=3 };
    __fastcall TfHome(TComponent* Owner);
    void ShowLed(int index, eHomeLedColor attr);
    void ShowMotorHomePos(int i);
    int iHomeStep;
    bool ProcessMotorHome();
    bool IsShown() const;
    bool SeenStart() const;
    void MarkSeenStart();
    void RequestClose();
    void RequestCloseFinished();
};
//---------------------------------------------------------------------------
extern PACKAGE TfHome *fHome;
//---------------------------------------------------------------------------
#endif
