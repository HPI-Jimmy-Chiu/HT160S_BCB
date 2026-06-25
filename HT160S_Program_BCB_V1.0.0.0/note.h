//---------------------------------------------------------------------------
#ifndef noteH
#define noteH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <ExtCtrls.hpp>
#include "HTray.h"
#include "MyKitSuck.h"
//---------------------------------------------------------------------------
#define K_SKIP       0x0001
#define K_RETRY      0x0002
#define K_TRAY_FEED  0x0004
#define K_TRAY_END   0x0008
#define K_CLEAN_OUT  0x0010
#define K_HOME       0x0020
//---------------------------------------------------------------------------
class MyNoteStruct
{
public:
    TStringList *SystemErrorCode;
    TStringList *SysFlushPanelName;

    MyNoteStruct();
    ~MyNoteStruct();
    void AddSysErr(AnsiString ErrorCode, AnsiString FlushPanel);
    void Clear();
};
//---------------------------------------------------------------------------
class TfNote : public TForm
{
__published:
    TTimer *Timer1;
    TPanel *Panel7;
    TLabel *Label2;
    TEdit *edtAlarmCode;
    TEdit *edtAlarmMsg;
    TPanel *Panel1;
    TPanel *PanelCommand;
    TPanel *BtnHome;
    TPanel *BtnSkip;
    TPanel *BtnRetry;
    TPanel *BtnTrayFeed;
    TPanel *BtnTrayEnd;
    TPanel *BtnCleanOut;
    TPanel *BtnStart;
    TPanel *BtnPause;
    TPanel *BtnOffBuzzer;
    TPanel *PanelMain6;
    TMemo *Memo1;
    void __fastcall FormShow(TObject *Sender);
    void __fastcall FormClose(TObject *Sender, TCloseAction &Action);
    void __fastcall Timer1Timer(TObject *Sender);
    void __fastcall BtnStartClick(TObject *Sender);
    void __fastcall BtnPauseClick(TObject *Sender);
    void __fastcall BtnOffBuzzerClick(TObject *Sender);
    void __fastcall BtnSkipClick(TObject *Sender);
private:
    bool bMachineLayoutBuilt;
    bool bOffBuzzer;
    void __fastcall BuildMachineLayout();
    TPanel *__fastcall CreateMachinePanel(TWinControl *ParentControl, AnsiString Name, AnsiString Caption, int Left, int Top, int Width, int Height, TColor Color);
    TTMyTray *__fastcall CreateMachineTray(TWinControl *ParentControl, AnsiString Name, AnsiString Caption, int Left, int Top, int Width, int Height);
    void __fastcall CreateMachineLabel(TWinControl *ParentControl, AnsiString Name, AnsiString Caption, int Left, int Top, int Width, int Height);
    void __fastcall SetMachinePanelColor(TPanel *Panel, TColor Color);
public:
    __fastcall TfNote(TComponent* Owner);
    int Code;
    int ReturnCode;
    bool fShow;
    bool IsBuzzerOff() { return bOffBuzzer; }   //AI(HT160S-Maintainer) 20260622 : per-scan buzzer driver reads the OFF BUZZER latch (ErrJam silence, HT172 bAlarmBuzzer parity)
    int KeyCode;
    bool Select[6];
    int iBackOldMemo2Y;
    int iBackMemo2Height;
    bool fMemoPos;
    AnsiString sObjName;
    TPanel *FlushPanel;
    TColor FlushPanelColor;
    MyNoteStruct *SystemError;

    TTMyTray *mtLoaderBuffer;
    TTMyTray *mtLoader;
    TTMyTray *mtFix1;
    TTMyTray *mtAuto1;
    TTMyTray *mtAuto2;
    TTMyTray *mtAuto3;
    TTMyTray *mtColor;
    TTMyTray *mtEmpty;
    TTMyTray *mtFix2;
    TTMyTray *mtFix3;
    TTMyTray *mtAuto1Buffer;
    TTMyTray *mtAuto2Buffer;
    TTMyTray *mtAuto3Buffer;
    TTMyTray *mtInCarrier;
    TTMyTray *mtStandbyBuffer1;
    TTMyTray *mtStandbyBuffer2;
    TTMyTray *mtOutCarrier;
    TTMyTray *mtOutCarrierShuttle;
    TTMyTray *mtInCarrierShuttle;
    TTMyTray *mtInCarrierBuffer;
    TTMyTray *mtOutCarrierShuttleBuffer;
    TTMyTray *mtTNTStation1;
    TTMyTray *mtTNTStation2;
    TTMyTray *mtTNTStation3;
    TTMyTray *mtAutoKnock;
    TTMyTray *mtCatchTray;
    TTMyTray *mtInShuttle;
    TTMyTray *mtOutShuttle;
    TTMyTray *mtEmptyBuffer;
    TTMyTray *mtColorBuffer;

    TPanel *pn_MachineFront;
    TPanel *pn_SafeDoor1;
    TPanel *pn_SafeDoor2;
    TPanel *pn_SafeDoor3;
    TPanel *pn_SafeDoor4;
    TPanel *pn_SafeDoor5;
    TPanel *pn_SafeDoor6;
    TPanel *pn_SafeDoor7;
    TPanel *pn_SafeDoor8;
    TPanel *pn_SafeDoor9;
    TPanel *pn_InShuttle;
    TPanel *pn_OutShuttle;
    TPanel *pn_System;
    TPanel *pn_InArmF;
    TPanel *pn_InArmR;
    TPanel *pn_OutArm;
    TPanel *pn_TrayArm;
    TPanel *pn_TempAndTest;
    TPanel *palSysErr;

    void __fastcall FlushLabel();
    void __fastcall ScanKey();
    void __fastcall ProcessErrMessage(AnsiString EC, AnsiString Str, int Type);
    void __fastcall Start();
    void __fastcall UpdateButtonStatus(TObject *Sender);
    void __fastcall LevelProcessErrMessage();
    void __fastcall Reset();
    void GetFlushPanel(TWinControl *PCtrl, AnsiString PanelName);
    void ChangePalPos(TPanel *Panel, int Height, int Left, int Top, int Width, bool Visible=true);
};
//---------------------------------------------------------------------------
extern PACKAGE TfNote *fNote;
//---------------------------------------------------------------------------
void ShowCylinderError(int Code, int Type);
void ShowMotorError(AnsiString Code);
void ShowMotorError(AnsiString Code, AnsiString sFunc);
int  ShowMotorLimitError(AnsiString Code, AnsiString Message, AnsiString Detail);
int  ShowSuckError(TMySucker &Ptr, int CodeType, int KCode, AnsiString HappenRegion);
int  ShowSuckError(TMyKitSuck &Ptr, int CodeType, int KCode, AnsiString errPart, int iDuplicate=0);
int  ShowSystemError(int CodeType, int KCode);
int  ShowSystemError(AnsiString Name, int KCode, int iDuplicate=0, AnsiString Message="");
int  ShowShuttleError(int pos, int KCode);
int  ShowJamError(int iJamSensor, int KCode);
int  ShowMagazineError(int iMagazine, int CodeType, int KCode);
int  ShowSystemCommError(int CodeType, int KCode, AnsiString Note="");
int  ShowCCDError(int CodeType, int KCode, AnsiString Note="");
int  ShowMyError(AnsiString sMyError, int KCode);
int  ShowTNTError(int CodeType, int KCode);
void ShowErrorMessage(AnsiString Code);
void RecordProcess(AnsiString S);
void SearchMessage(AnsiString Code);
void RecordAlarmMessagePassTime(AnsiString AlarmCode, DWORD StartTime, AnsiString HappenTime, int Type);
extern DWORD RecordHappenTime;
//---------------------------------------------------------------------------
#endif
