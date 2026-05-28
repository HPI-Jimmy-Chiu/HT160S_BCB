//---------------------------------------------------------------------------
#ifndef ComPortH
#define ComPortH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include "SPComm.hpp"
//---------------------------------------------------------------------------
class TfComPort : public TForm
{
__published:
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall sbExitClick(TObject *Sender);
    void __fastcall sbUpdateClick(TObject *Sender);
    void __fastcall spbResetComClick(TObject *Sender);
    void __fastcall btnStopComClick(TObject *Sender);
    void __fastcall btnClearMemoClick(TObject *Sender);
    void __fastcall sbPanelSend_ComClick(TObject *Sender);
    void __fastcall PadCommReceiveData(TObject *Sender, Pointer Buffer, WORD BufferLength);
private:
    TPanel *pnlTop;
    TPanel *pnlSetting;
    TPanel *pnlManual;
    TPanel *pnlLog;
    TLabel *labPadCom;
    TLabel *labManualSend;
    TComboBox *cbPadComm;
    TButton *sbUpdate;
    TButton *spbResetCom;
    TButton *btnStopCom;
    TButton *btnClearMemo;
    TButton *sbExit;
    TButton *sbPanelSend_Com;
    TEdit *edPanelSend_Com;
    TMemo *memoPanelCom;
    TComm *PadComm;

    void BuildUI();
    void PopulateComList();
    AnsiString GetWorkFileName();
    void EnsurePadInterface();
    void ConfigurePadComm();
public:
    __fastcall TfComPort(TComponent* Owner);
    __fastcall ~TfComPort();

    void __fastcall SaveWorkFile();
    void __fastcall OpenWorkFile();
    bool __fastcall RS232Init();
    void StopAllCom();
    void Spin();
    void MemoAddString(TMemo *Memo, AnsiString Title, AnsiString Text);
    bool bShow;
};
//---------------------------------------------------------------------------
extern PACKAGE TfComPort *fComPort;
void EnsureComPortCreated(TComponent *Owner);
void SpinComPort();
//---------------------------------------------------------------------------
#endif
