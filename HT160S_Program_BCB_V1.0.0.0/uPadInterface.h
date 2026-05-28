//---------------------------------------------------------------------------
#ifndef uPadInterfaceH
#define uPadInterfaceH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <Menus.hpp>
#include "SPComm.hpp"
#include "MyLed.h"
#include "BtnPanelLane.h"
//---------------------------------------------------------------------------
#define PAD_ControlDLC      0x04
#define PAD_FrontControl    0x00
#define PAD_RearControl     0x01
#define PAD_LedLight        0x00
#define PAD_LedBling        0x01
#define PAD_PowerOff        0x000001
#define PAD_PowerOn         0x000002
#define PAD_PannelEnable    0x000004
#define PAD_Reset           0x000008
#define PAD_Pause           0x000010
#define PAD_Home            0x000020
#define PAD_Start           0x000040
#define PAD_OneCycle        0x000080
#define PAD_Retry           0x000100
#define PAD_Skip            0x000200
#define PAD_CleanOut        0x000400
#define PAD_TrayFeed        0x000800
#define PAD_TrayEnd         0x001000
#define PAD_AlarmReset      0x002000
#define PAD_SafeLock        0x004000
#define PAD_Step            0x008000
#define PAD_TStart          0x010000
//---------------------------------------------------------------------------
typedef struct
{
    TMyLed          *mlEvent;
    TBtnPanelLane   *btnEvent;
    AnsiString      PadName;
    int             iData;
    AnsiString      InputName;

    void SetItem(TMyLed *_mlEvent, TBtnPanelLane *_btnEvent, AnsiString _PadName, int _iData, AnsiString _InputName);
} PAD_PTR;
//---------------------------------------------------------------------------
class TfPadInterface : public TForm
{
__published:
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall sb_PadInterface_ManualSendClick(TObject *Sender);
    void __fastcall ClearLog1Click(TObject *Sender);
    void __fastcall sb_PadInterface_ExitClick(TObject *Sender);
private:
    TPanel *pn_PadInterfaceTitle;
    TPageControl *pc_PadInterface;
    TTabSheet *tsPadFront;
    TTabSheet *tsPadRear;
    TPanel *pn_PadInterface_Front;
    TPanel *pn_PadInterface_Rear;
    TButton *sb_PadInterface_Exit;
    TButton *sb_PadInterface_ManualSend;
    TButton *btnResetCom;
    TButton *btnClearLog;
    TEdit *ed_PadInterface_ManualSend;
    TCheckBox *cb_PadInterface_PadLedBling;
    TMemo *Memo_PadInterface;

    void BuildUI();
    void BuildPadPage(TPanel *ParentPanel, int BaseIndex, int Count);
    void AddPadItem(TPanel *ParentPanel, int Index, int Row, int Col);
    void RecordLocalStatusCommand(int iAddress);
    void RecordCommunication(AnsiString aTitle, AnsiString Command);
    void __fastcall PadButtonMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y);
public:
    __fastcall TfPadInterface(TComponent* Owner);
    __fastcall ~TfPadInterface();

    TComm *PadComm;
    AnsiString PadComPort;
    bool bRs232Ok;
    TStringList *CommReceiveList;
    TStringList *CommReceiveLength;
    TStringList *CommSendList;
    TStringList *CommSendLength;
    PAD_PTR PadItem[32];
    int CheckPadItem;
    bool bPadInputStatus[32];
    bool bPadStatus[32];
    bool bShow;
    bool bRequestVer;
    bool bScanSwitch;
    bool bSendSwitchStatusing;

    void InitialVariable();
    void __fastcall PadButtonClick(TObject *Sender);
    void __fastcall ResetComm();
    bool OpenCommPort();
    bool CloseCommPort();
    void SendCommand(AnsiString sData);
    void __fastcall ProcessReceiceData();
    void __fastcall DoScanPanelLed(int iAddress, int iKey);
    void __fastcall DoUpdataPadStatus(int iAddress, int iKey);
    void __fastcall Main232();
    void SendSwitchStatus(TBtnPanelLane *bpPtr);
    void SendSwitchStatus(AnsiString aName, bool Type);
    bool IsPadButton(AnsiString aName);
    bool IsPadKey(AnsiString aName);
    bool GetPadSwitchStatus(AnsiString aName, bool *State);
    void __fastcall ProcessSendDataNew();
    bool __fastcall ProcessScanKey(AnsiString aSenName);
    void RequestPadVersion();
};
//---------------------------------------------------------------------------
extern PACKAGE TfPadInterface *fPadInterface;
//---------------------------------------------------------------------------
#endif
