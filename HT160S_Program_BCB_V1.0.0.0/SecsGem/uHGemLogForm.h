//---------------------------------------------------------------------------
#ifndef uHGemLogFormH
#define uHGemLogFormH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include <ComCtrls.hpp>
#include <Grids.hpp>
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : SECS/GEM monitor window. The visual layout lives
//  in uHGemLogForm.dfm (standard VCL form); only the dynamic grid headers,
//  live-value refresh, and the idle-guarded EC editor are coded here. Tabs:
//    Log        - live SECS TX/RX trace drained from THGem::LogList
//    SV Query   - read-only grid of registered Status Variables + live values
//    EC Query   - grid of Equipment Constants; tray-form ECs (2011-2016) are
//                 editable only while the machine is idle (same guard as S2F16)
//    Connection - read-only view of the configured HSMS endpoint + state
//---------------------------------------------------------------------------
class TfSecsGemLog : public TForm
{
__published:
    TPageControl *PageControl1;
    TTabSheet    *tsLog;
    TTabSheet    *tsSV;
    TTabSheet    *tsEC;
    TTabSheet    *tsConn;

    // -- Log tab --
    TPanel    *PanelTop;
    TMemo     *MemoLog;
    TButton   *BtnClear;
    TButton   *BtnCopy;
    TCheckBox *ChkAutoScroll;
    TCheckBox *ChkPause;
    TLabel    *LblState;

    // -- SV tab --
    TStringGrid *GridSV;

    // -- EC tab --
    TStringGrid *GridEC;
    TPanel      *PanelECEdit;
    TLabel      *LblECSel;
    TEdit       *EdtECValue;
    TButton     *BtnECWrite;
    TLabel      *LblECStatus;

    // -- Connection tab --
    TLabel *LblConnAddr;
    TLabel *LblConnPort;
    TLabel *LblConnDev;
    TLabel *LblConnMode;
    TLabel *LblConnState;
    TLabel *LblConnNote;

    // -- Settings tab (edit General.ini [SECS]) --
    TTabSheet *tsSet;
    TLabel    *LblSetEnable;
    TLabel    *LblSetMode;
    TLabel    *LblSetAddr;
    TLabel    *LblSetPort;
    TLabel    *LblSetDev;
    TLabel    *LblSetStatus;
    TLabel    *LblSetNote;
    TCheckBox *ChkSetEnable;
    TCheckBox *ChkSetActive;
    TEdit     *EdtSetAddr;
    TEdit     *EdtSetPort;
    TEdit     *EdtSetDev;
    TButton   *BtnSetSave;
    TButton   *BtnSetReload;

    TTimer *PollTimer;

    void __fastcall PollTimerTimer(TObject *Sender);
    void __fastcall BtnClearClick(TObject *Sender);
    void __fastcall BtnCopyClick(TObject *Sender);
    void __fastcall GridECClick(TObject *Sender);
    void __fastcall BtnECWriteClick(TObject *Sender);
    void __fastcall BtnSetSaveClick(TObject *Sender);
    void __fastcall BtnSetReloadClick(TObject *Sender);

private:
    unsigned uSelECID;
    int      iMaxLines;

    void InitGridHeaders();
    void RefreshSVGrid();
    void RefreshECGrid();
    void RefreshConnTab();
    void LoadSecsSettings();
    AnsiString SecsIniPath();
    AnsiString HsmsStateText();
public:
    __fastcall TfSecsGemLog(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern TfSecsGemLog *fSecsGemLog;
void ShowSecsGemLog();
//---------------------------------------------------------------------------
#endif
