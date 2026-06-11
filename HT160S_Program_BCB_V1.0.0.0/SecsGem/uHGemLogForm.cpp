//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include <IniFiles.hpp>     // edit system\General.ini [SECS] on the Settings tab
#include "uHGemLogForm.h"
#include "uHGemEquipment.h"
#include "uHGemHT160.h"      // HT160Gem::GuiWriteTrayEC (idle-guarded EC editor)
#include "database.h"        // HSys.MyGem (GEM logic instance)
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : the global GEM engine pointer is defined in
//  uHGemClass.cpp; we only read its log buffer + connection state here.
extern THGem *HGem;
//---------------------------------------------------------------------------
TfSecsGemLog *fSecsGemLog = NULL;
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : visual layout now comes from uHGemLogForm.dfm.
//  Only the grid column widths + header captions are applied in code.
__fastcall TfSecsGemLog::TfSecsGemLog(TComponent* Owner)
    : TForm(Owner)
{
    iMaxLines = 1000;
    uSelECID  = 0;
    InitGridHeaders();
    LoadSecsSettings();
}
//---------------------------------------------------------------------------
void TfSecsGemLog::InitGridHeaders()
{
    GridSV->ColWidths[0] = 70;
    GridSV->ColWidths[1] = 240;
    GridSV->ColWidths[2] = 70;
    GridSV->ColWidths[3] = 320;
    GridSV->Cells[0][0]  = "SVID";
    GridSV->Cells[1][0]  = "Name";
    GridSV->Cells[2][0]  = "Unit";
    GridSV->Cells[3][0]  = "Value";

    GridEC->ColWidths[0] = 70;
    GridEC->ColWidths[1] = 220;
    GridEC->ColWidths[2] = 60;
    GridEC->ColWidths[3] = 240;
    GridEC->ColWidths[4] = 80;
    GridEC->Cells[0][0]  = "ECID";
    GridEC->Cells[1][0]  = "Name";
    GridEC->Cells[2][0]  = "Unit";
    GridEC->Cells[3][0]  = "Value";
    GridEC->Cells[4][0]  = "Settable";
}
//---------------------------------------------------------------------------
AnsiString TfSecsGemLog::HsmsStateText()
{
    if(HGem == NULL)         return "GEM engine not created";
    if(HGem->IsSelected())   return "HSMS: SELECTED";
    if(HGem->IsConnected())  return "HSMS: CONNECTED";
    return "HSMS: not connected";
}
//---------------------------------------------------------------------------
void __fastcall TfSecsGemLog::PollTimerTimer(TObject *Sender)
{
    AnsiString sState = HsmsStateText();
    LblState->Caption     = sState;
    //AI(ht160s-secsgem) 20260611 : append reconnect watchdog status when idle.
    AnsiString sRetry = (HGem != NULL) ? HGem->GetReconnectStatusText() : AnsiString("");
    LblConnState->Caption = "State : " + sState +
                            (sRetry.IsEmpty() ? AnsiString("") : ("   [" + sRetry + "]"));

    if(HGem == NULL)
        return;

    // Log tab: drain new lines unless paused.
    if(!ChkPause->Checked)
    {
        TStringList *pending = new TStringList;
        try
        {
            HGem->DrainLog(pending);
            if(pending->Count > 0)
            {
                MemoLog->Lines->BeginUpdate();
                try
                {
                    for(int i = 0; i < pending->Count; i++)
                        MemoLog->Lines->Add(pending->Strings[i]);
                    while(MemoLog->Lines->Count > iMaxLines)
                        MemoLog->Lines->Delete(0);
                }
                __finally
                {
                    MemoLog->Lines->EndUpdate();
                }
                if(ChkAutoScroll->Checked)
                {
                    MemoLog->SelStart = MemoLog->GetTextLen();
                    MemoLog->Perform(EM_SCROLLCARET, 0, 0);
                }
            }
        }
        __finally
        {
            delete pending;
        }
    }

    // Refresh only the visible data tab to keep it light.
    if(PageControl1->ActivePage == tsSV)
        RefreshSVGrid();
    else if(PageControl1->ActivePage == tsEC)
        RefreshECGrid();
    else if(PageControl1->ActivePage == tsConn)
        RefreshConnTab();
}
//---------------------------------------------------------------------------
void TfSecsGemLog::RefreshSVGrid()
{
    if(HGem == NULL) return;
    HGem->RefreshSVSnapshot();
    int cnt = HGem->GetSVCount();
    int rows = (cnt > 0) ? (cnt + 1) : 2;
    if(GridSV->RowCount != rows)
        GridSV->RowCount = rows;

    for(int i = 0; i < cnt; i++)
    {
        unsigned id = HGem->GetSVIDByIndex(i);
        GridSV->Cells[0][i + 1] = AnsiString((int)id);
        GridSV->Cells[1][i + 1] = HGem->GetSVName(id);
        GridSV->Cells[2][i + 1] = HGem->GetSVUnit(id);
        GridSV->Cells[3][i + 1] = HGem->GetSVValueString(id);
    }
}
//---------------------------------------------------------------------------
void TfSecsGemLog::RefreshECGrid()
{
    if(HGem == NULL) return;
    int cnt = HGem->GetECCount();
    int rows = (cnt > 0) ? (cnt + 1) : 2;
    if(GridEC->RowCount != rows)
        GridEC->RowCount = rows;

    for(int i = 0; i < cnt; i++)
    {
        unsigned id = HGem->GetECIDByIndex(i);
        bool bSettable = (id >= 2011 && id <= 2016);
        GridEC->Cells[0][i + 1] = AnsiString((int)id);
        GridEC->Cells[1][i + 1] = HGem->GetECName(id);
        GridEC->Cells[2][i + 1] = HGem->GetECUnit(id);
        GridEC->Cells[3][i + 1] = HGem->GetECValueString(id);
        GridEC->Cells[4][i + 1] = bSettable ? AnsiString("Yes") : AnsiString("No");
    }
}
//---------------------------------------------------------------------------
void TfSecsGemLog::RefreshConnTab()
{
    if(HGem == NULL) return;
    LblConnAddr->Caption = "Address  : " + HGem->GetEndpointAddress();
    LblConnPort->Caption = "Port     : " + HGem->GetEndpointPort();
    LblConnDev->Caption  = "DeviceID : " + HGem->GetDeviceIdText();
    LblConnMode->Caption = "HSMS Mode: " + AnsiString(HGem->IsActiveMode() ? "Active (connect)" : "Passive (listen)");
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : Settings tab edits system\General.ini [SECS].
//  Same path/section/keys that TFSECS::GemInitial() reads at startup, so a
//  Save + program restart is the supported way to change the endpoint.
AnsiString TfSecsGemLog::SecsIniPath()
{
    return HSys.CurrentDir + AnsiString("\\system\\General.ini");
}
//---------------------------------------------------------------------------
void TfSecsGemLog::LoadSecsSettings()
{
    AnsiString sAddress = "127.0.0.1";
    int        iPort    = 5098;
    int        iDevice  = 0;
    int        iEnable  = 1;
    int        iActive  = 0;

    TIniFile *IniFile = new TIniFile(SecsIniPath());
    try
    {
        iEnable  = IniFile->ReadInteger("SECS", "Enable",     iEnable);
        sAddress = IniFile->ReadString ("SECS", "Address",    sAddress);
        iPort    = IniFile->ReadInteger("SECS", "Port",       iPort);
        iDevice  = IniFile->ReadInteger("SECS", "DeviceID",   iDevice);
        iActive  = IniFile->ReadInteger("SECS", "ActiveMode", iActive);
    }
    __finally
    {
        delete IniFile;
    }

    ChkSetEnable->Checked = (iEnable > 0);
    ChkSetActive->Checked = (iActive != 0);
    EdtSetAddr->Text      = sAddress;
    EdtSetPort->Text      = AnsiString(iPort);
    EdtSetDev->Text       = AnsiString(iDevice);
    LblSetStatus->Caption = "";
}
//---------------------------------------------------------------------------
void __fastcall TfSecsGemLog::BtnSetReloadClick(TObject *Sender)
{
    LoadSecsSettings();
    LblSetStatus->Caption = "Reloaded from General.ini.";
}
//---------------------------------------------------------------------------
void __fastcall TfSecsGemLog::BtnSetSaveClick(TObject *Sender)
{
    int iPort   = StrToIntDef(EdtSetPort->Text, 0);
    int iDevice = StrToIntDef(EdtSetDev->Text, 0);

    if(iPort <= 0 || iPort > 65535)
    {
        LblSetStatus->Caption = "Rejected: Port must be 1-65535.";
        return;
    }
    if(iDevice < 0 || iDevice > 65535)
    {
        LblSetStatus->Caption = "Rejected: Device ID must be 0-65535.";
        return;
    }
    if(ChkSetActive->Checked && EdtSetAddr->Text.Trim() == "")
    {
        LblSetStatus->Caption = "Rejected: Active mode needs a host Address.";
        return;
    }

    TIniFile *IniFile = new TIniFile(SecsIniPath());
    try
    {
        IniFile->WriteInteger("SECS", "Enable",     ChkSetEnable->Checked ? 1 : 0);
        IniFile->WriteString ("SECS", "Address",    EdtSetAddr->Text);
        IniFile->WriteInteger("SECS", "Port",       iPort);
        IniFile->WriteInteger("SECS", "DeviceID",   iDevice);
        IniFile->WriteInteger("SECS", "ActiveMode", ChkSetActive->Checked ? 1 : 0);
    }
    __finally
    {
        delete IniFile;
    }

    LblSetStatus->Caption = "Saved. Restart ht160s.exe to apply the new endpoint.";
}
//---------------------------------------------------------------------------
void __fastcall TfSecsGemLog::GridECClick(TObject *Sender)
{
    if(HGem == NULL) return;
    int row = GridEC->Row;
    if(row < 1) return;

    unsigned id = (unsigned)StrToIntDef(GridEC->Cells[0][row], 0);
    uSelECID = id;
    bool bSettable = (id >= 2011 && id <= 2016);

    LblECSel->Caption   = "EC " + AnsiString((int)id) + "  " + HGem->GetECName(id);
    EdtECValue->Text    = HGem->GetECValueString(id);
    EdtECValue->Enabled = bSettable;
    BtnECWrite->Enabled = bSettable;
    LblECStatus->Caption = bSettable ? AnsiString("") : AnsiString("Read-only EC (not host/GUI settable).");
}
//---------------------------------------------------------------------------
void __fastcall TfSecsGemLog::BtnECWriteClick(TObject *Sender)
{
    if(uSelECID == 0)
    {
        LblECStatus->Caption = "Select an EC row first.";
        return;
    }
    if(HSys.MyGem == NULL)
    {
        LblECStatus->Caption = "GEM logic not ready.";
        return;
    }

    HT160Gem *logic = (HT160Gem*)(HSys.MyGem);
    int r = logic->GuiWriteTrayEC(uSelECID, EdtECValue->Text);
    switch(r)
    {
        case 0:
            LblECStatus->Caption = "Write OK (tray form saved).";
            RefreshECGrid();
            break;
        case 2:
            LblECStatus->Caption = "Rejected: machine busy (running or IC inside).";
            break;
        case 3:
            LblECStatus->Caption = "Rejected: value invalid / out of range.";
            break;
        default:
            LblECStatus->Caption = "Rejected: this EC is not settable.";
            break;
    }
}
//---------------------------------------------------------------------------
void __fastcall TfSecsGemLog::BtnClearClick(TObject *Sender)
{
    MemoLog->Clear();
}
//---------------------------------------------------------------------------
void __fastcall TfSecsGemLog::BtnCopyClick(TObject *Sender)
{
    MemoLog->SelectAll();
    MemoLog->CopyToClipboard();
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : lazy-create + show the monitor (non-modal).
void ShowSecsGemLog()
{
    if(fSecsGemLog == NULL)
        fSecsGemLog = new TfSecsGemLog(Application);
    fSecsGemLog->Show();
    fSecsGemLog->BringToFront();
}
//---------------------------------------------------------------------------
