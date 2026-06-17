//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#pragma hdrstop

#include <IniFiles.hpp>
#include <SysUtils.hpp>

#include "uMotorTest.h"
#include "cmydef.h"
#include "csystem.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "ALed"
#pragma resource "*.dfm"
TfMotorTest *fMotorTest;
//---------------------------------------------------------------------------
static const TColor MOTOR_TEST_COLOR_GRID=(TColor)14670284;

enum
{
    eOperateColLoop = 0,
    eOperateColNo,
    eOperateColMotor,
    eOperateColCommand,
    eOperateColEncoder,
    eOperateColHome,
    eOperateColEnable,
    eOperateColServoOn,
    eOperateColServoAlarm,
    eOperateColEmg,
    eOperateColAlarm,
    eOperateColInPos,
    eOperateColSoftLimit,
    eOperateColTotal
};

enum
{
    eMotorParamColNo = 0,
    eMotorParamColAlias,
    eMotorParamColCard,
    eMotorParamColAddress,
    eMotorParamColEnable,
    eMotorParamColHome,
    eMotorParamColSoftN,
    eMotorParamColSoftP,
    eMotorParamColInit,
    eMotorParamColJogLow,
    eMotorParamColJogHigh,
    eMotorParamColHomeLow,
    eMotorParamColHomeHigh,
    eMotorParamColAcc,
    eMotorParamColDec,
    eMotorParamColRange,
    eMotorParamColTotal
};

enum
{
    eInfoColNo = 0,
    eInfoColAlias,
    eInfoColCard,
    eInfoColBoard,
    eInfoColPort,
    eInfoColAddress,
    eInfoColKind,
    eInfoColFlushPanel,
    eInfoColServoConfig,
    eInfoColIsServo,
    eInfoColMotorRelay,
    eInfoColServerON,
    eInfoColMotorPower,
    eInfoColPowerState,
    eInfoColPowerDelay,
    eInfoColTotal
};

enum
{
    eDriverRegColRegister = 0,
    eDriverRegColValue,
    eDriverRegColResult,
    eDriverRegColTotal
};

enum
{
    eServoGuardColItem = 0,
    eServoGuardColValue,
    eServoGuardColNote,
    eServoGuardColTotal
};

static AnsiString MotorTestTrim(AnsiString Text)
{
    while(Text.Length()>0 && (Text[1]==' ' || Text[1]=='\t'))
        Text.Delete(1, 1);
    while(Text.Length()>0 && (Text[Text.Length()]==' ' || Text[Text.Length()]=='\t'))
        Text.Delete(Text.Length(), 1);
    return Text;
}

static bool MotorTestTryParseInt(AnsiString Text, int &Value)
{
    char *EndPtr;
    long Parsed;
    AnsiString Work=MotorTestTrim(Text);

    if(Work==AnsiString(""))
        return false;
    EndPtr=NULL;
    Parsed=strtol(Work.c_str(), &EndPtr, 10);
    while(EndPtr!=NULL && (*EndPtr==' ' || *EndPtr=='\t'))
        EndPtr++;
    if(EndPtr==NULL || *EndPtr!='\0')
        return false;
    Value=(int)Parsed;
    return true;
}

static bool MotorTestTryParseDouble(AnsiString Text, double &Value)
{
    char *EndPtr;
    AnsiString Work=MotorTestTrim(Text);

    if(Work==AnsiString(""))
        return false;
    EndPtr=NULL;
    Value=strtod(Work.c_str(), &EndPtr);
    while(EndPtr!=NULL && (*EndPtr==' ' || *EndPtr=='\t'))
        EndPtr++;
    return (EndPtr!=NULL && *EndPtr=='\0');
}

static AnsiString MotorTestCsvField(TStringList *List, int Index)
{
    if(List==NULL || Index<0 || Index>=List->Count)
        return "";
    return List->Strings[Index];
}

static bool MotorTestReadIntField(TStringList *List, int Index, AnsiString FieldName, int RowNo, int &Value, AnsiString &ErrorText)
{
    AnsiString Text=MotorTestTrim(MotorTestCsvField(List, Index));

    if(Text==AnsiString(""))
    {
        ErrorText=AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" missing ")+FieldName+AnsiString(".");
        return false;
    }
    if(MotorTestTryParseInt(Text, Value)==false)
    {
        ErrorText=AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" ")+FieldName+AnsiString(" must be integer.");
        return false;
    }
    return true;
}

static bool MotorTestReadDoubleField(TStringList *List, int Index, AnsiString FieldName, int RowNo, double &Value, AnsiString &ErrorText)
{
    AnsiString Text=MotorTestTrim(MotorTestCsvField(List, Index));

    if(Text==AnsiString(""))
    {
        ErrorText=AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" missing ")+FieldName+AnsiString(".");
        return false;
    }
    if(MotorTestTryParseDouble(Text, Value)==false)
    {
        ErrorText=AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" ")+FieldName+AnsiString(" must be numeric.");
        return false;
    }
    return true;
}

static AnsiString MotorTestFormatDouble(double Value)
{
    AnsiString Text;

    Text.sprintf("%.6f", Value);
    while(Text.Length()>0 && Text[Text.Length()]=='0')
        Text.Delete(Text.Length(), 1);
    if(Text.Length()>0 && Text[Text.Length()]=='.')
        Text.Delete(Text.Length(), 1);
    if(Text==AnsiString(""))
        Text="0";
    return Text;
}

static AnsiString MotorTestFormatMS(DWORD Value)
{
    AnsiString Text;

    Text.sprintf("%.3f sec", (double)Value/1000.0);
    return Text;
}
//---------------------------------------------------------------------------
__fastcall TfMotorTest::TfMotorTest(TComponent* Owner)
    : TForm(Owner)
{
    bUIBuilt=false;
    bHomeRunning=false;
    bLoopRunning=false;
    bLoopTargetIsPos2=false;
    bLoopWaiting=false;
    bLoopMultiMode=false;
    bMotorParameterDirty=false;
    bMotorTableDirty=false;
    ActiveMotorIndex=-1;
    iHomeMotorIndex=-1;
    iLoopMotorIndex=-1;
    iLoopTarget=0;
    iLoopRemainCount=0;
    iLoopFinishedCount=0;
    dwLoopWaitUntil=0;
    dwLoopStartTick=0;
    dwLoopLegStartTick=0;

    for(int MotorIndex=0; MotorIndex<MAX_MOTOR_TEST_MOTOR_COUNT; MotorIndex++)
    {
        Pos1[MotorIndex]=0;
        Pos2[MotorIndex]=0;
        bMultiLoopMotor[MotorIndex]=false;
        MultiLoopTarget[MotorIndex]=0;
    }
    for(int LedIndex=0; LedIndex<iMotLedTotalCnt; LedIndex++)
    {
        lblStatus[LedIndex]=NULL;
        ledStatus[LedIndex]=NULL;
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::FormCreate(TObject *Sender)
{
    (void)Sender;
    BuildUI();
    FillMotorList();
    LoadLoopPositions();
    RefreshAllGrids();
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::FormShow(TObject *Sender)
{
    (void)Sender;
    if(bUIBuilt==false)
        BuildUI();
    FillMotorList();
    LoadLoopPositions();
    RefreshAllGrids();
    if(tmrUpdate!=NULL)
        tmrUpdate->Enabled=true;
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::FormClose(TObject *Sender, TCloseAction &Action)
{
    (void)Sender;
    if(ConfirmDiscardMotorParameterEdit()==false)
    {
        Action=caNone;
        return;
    }
    if(tmrUpdate!=NULL)
        tmrUpdate->Enabled=false;
    StopActiveMotor();
}
//---------------------------------------------------------------------------
void TfMotorTest::BuildUI()
{
    if(bUIBuilt)
        return;

    BindDfmComponents();

    BuildPageArea();

    if(tmrUpdate!=NULL)
    {
        tmrUpdate->Enabled=false;
    }

    bUIBuilt=true;
}
//---------------------------------------------------------------------------
void TfMotorTest::BindDfmComponents()
{
    lblStatus[0]=lblStatus0;
    lblStatus[1]=lblStatus1;
    lblStatus[2]=lblStatus2;
    lblStatus[3]=lblStatus3;
    lblStatus[4]=lblStatus4;
    lblStatus[5]=lblStatus5;
    lblStatus[6]=lblStatus6;
    lblStatus[7]=lblStatus7;
    lblStatus[8]=lblStatus8;
    lblStatus[9]=lblStatus9;
    lblStatus[10]=lblStatus10;

    ledStatus[0]=ledStatus0;
    ledStatus[1]=ledStatus1;
    ledStatus[2]=ledStatus2;
    ledStatus[3]=ledStatus3;
    ledStatus[4]=ledStatus4;
    ledStatus[5]=ledStatus5;
    ledStatus[6]=ledStatus6;
    ledStatus[7]=ledStatus7;
    ledStatus[8]=ledStatus8;
    ledStatus[9]=ledStatus9;
    ledStatus[10]=ledStatus10;
}
//---------------------------------------------------------------------------
void TfMotorTest::BuildPageArea()
{
    ArrangeOperatePage();
    ConfigureOperateGrid();
    ConfigureMotorParameterGrid();
    ConfigureInformationGrid();
    ConfigureDriverRegisterGrid();
    ConfigureServoGuardGrid();
}
//---------------------------------------------------------------------------
void TfMotorTest::ArrangeOperatePage()
{
    if(PageMotorTest!=NULL)
        PageMotorTest->Align=alClient;

    if(tsOperate!=NULL && palMotorControl!=NULL)
    {
        palMotorControl->Parent=tsOperate;
        palMotorControl->Align=alRight;
        palMotorControl->Width=360;
        palMotorControl->Visible=true;
        palMotorControl->TabOrder=1;
    }

    if(grdOperate!=NULL)
    {
        grdOperate->Align=alClient;
        grdOperate->TabOrder=0;
    }
}
//---------------------------------------------------------------------------
// grdOperate is both the status list and the motor picker. The active motor row
// is drawn with the system highlight (blue bar) keyed off ActiveMotorIndex, not
// the grid's own selection, so it stays correct regardless of focus. The first
// column ("Loop") is a checkbox marking multi-loop membership (bMultiLoopMotor).
void __fastcall TfMotorTest::grdOperateDrawCell(TObject *Sender, int ACol,
    int ARow, const TRect &Rect, TGridDrawState State)
{
    TStringGrid *Grid=(TStringGrid*)Sender;
    TCanvas *Cv=Grid->Canvas;
    bool bFixed=(ARow<Grid->FixedRows)||(ACol<Grid->FixedCols);
    bool bActive=(ARow>=Grid->FixedRows)&&(ARow-1==ActiveMotorIndex);
    (void)State;
    if(bFixed)
    {
        Cv->Brush->Color=(TColor)Grid->FixedColor;
        Cv->Font->Color=clWindowText;
    }
    else if(bActive)
    {
        Cv->Brush->Color=clHighlight;
        Cv->Font->Color=clHighlightText;
    }
    else
    {
        Cv->Brush->Color=(TColor)Grid->Color;
        Cv->Font->Color=clWindowText;
    }
    Cv->FillRect(Rect);

    if(!bFixed && ACol==eOperateColLoop)
    {
        int MotorIndex=ARow-1;
        bool bChecked=(MotorIndex>=0 && MotorIndex<MAX_MOTOR_TEST_MOTOR_COUNT && bMultiLoopMotor[MotorIndex]);
        int BoxLeft=Rect.Left+(Rect.right-Rect.Left-14)/2;
        int BoxTop=Rect.Top+(Rect.Bottom-Rect.Top-14)/2;
        Cv->Brush->Color=clWindow;
        Cv->Pen->Color=clBlack;
        Cv->Pen->Width=1;
        Cv->Rectangle(BoxLeft, BoxTop, BoxLeft+14, BoxTop+14);
        if(bChecked)
        {
            Cv->Pen->Color=clGreen;
            Cv->Pen->Width=2;
            Cv->MoveTo(BoxLeft+3, BoxTop+7);
            Cv->LineTo(BoxLeft+6, BoxTop+10);
            Cv->LineTo(BoxLeft+11, BoxTop+3);
            Cv->Pen->Width=1;
        }
        return;
    }

    AnsiString Text=Grid->Cells[ACol][ARow];
    if(!Text.IsEmpty())
        Cv->TextRect(Rect, Rect.Left+4, Rect.Top+4, Text);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::grdOperateMouseDown(TObject *Sender,
    TMouseButton Button, TShiftState Shift, int X, int Y)
{
    int ACol;
    int ARow;
    int MotorIndex;
    (void)Sender;
    (void)Button;
    (void)Shift;

    if(grdOperate==NULL)
        return;
    grdOperate->MouseToCell(X, Y, ACol, ARow);
    if(ARow<1)
        return;
    MotorIndex=ARow-1;
    if(MotorIndex<0 || MotorIndex>=GetMotorCount())
        return;
    if(ACol==eOperateColLoop)
    {
        if(MotorIndex<MAX_MOTOR_TEST_MOTOR_COUNT)
        {
            bMultiLoopMotor[MotorIndex]=!bMultiLoopMotor[MotorIndex];
            grdOperate->Invalidate();
        }
        return;
    }
    SetActiveMotor(MotorIndex);
}
//---------------------------------------------------------------------------
void TfMotorTest::ConfigureOperateGrid()
{
    if(grdOperate==NULL)
        return;
    grdOperate->ColCount=eOperateColTotal;
    grdOperate->FixedRows=1;
    grdOperate->FixedCols=0;
    grdOperate->DefaultRowHeight=24;
    grdOperate->Color=MOTOR_TEST_COLOR_GRID;
    grdOperate->Options=grdOperate->Options << goColSizing;
    grdOperate->OnDrawCell=grdOperateDrawCell;
    grdOperate->OnMouseDown=grdOperateMouseDown;
    grdOperate->Cells[eOperateColLoop][0]="Loop";
    grdOperate->Cells[eOperateColNo][0]="No";
    grdOperate->Cells[eOperateColMotor][0]="Motor";
    grdOperate->Cells[eOperateColCommand][0]="Command";
    grdOperate->Cells[eOperateColEncoder][0]="Encoder";
    grdOperate->Cells[eOperateColHome][0]="Home";
    grdOperate->Cells[eOperateColEnable][0]="Enable";
    grdOperate->Cells[eOperateColServoOn][0]="Servo";
    grdOperate->Cells[eOperateColServoAlarm][0]="Svr Alm";
    grdOperate->Cells[eOperateColEmg][0]="EMG";
    grdOperate->Cells[eOperateColAlarm][0]="Alarm";
    grdOperate->Cells[eOperateColInPos][0]="InPos";
    grdOperate->Cells[eOperateColSoftLimit][0]="Soft Limit";
    grdOperate->ColWidths[eOperateColLoop]=42;
    grdOperate->ColWidths[eOperateColNo]=45;
    grdOperate->ColWidths[eOperateColMotor]=145;
    grdOperate->ColWidths[eOperateColCommand]=75;
    grdOperate->ColWidths[eOperateColEncoder]=75;
    grdOperate->ColWidths[eOperateColHome]=50;
    grdOperate->ColWidths[eOperateColEnable]=55;
    grdOperate->ColWidths[eOperateColServoOn]=55;
    grdOperate->ColWidths[eOperateColServoAlarm]=65;
    grdOperate->ColWidths[eOperateColEmg]=45;
    grdOperate->ColWidths[eOperateColAlarm]=55;
    grdOperate->ColWidths[eOperateColInPos]=55;
    grdOperate->ColWidths[eOperateColSoftLimit]=130;
}
//---------------------------------------------------------------------------
void TfMotorTest::ConfigureMotorParameterGrid()
{
    if(grdMotorParameter==NULL)
        return;
    grdMotorParameter->ColCount=eMotorParamColTotal;
    grdMotorParameter->FixedRows=1;
    grdMotorParameter->FixedCols=0;
    grdMotorParameter->DefaultRowHeight=24;
    grdMotorParameter->Color=MOTOR_TEST_COLOR_GRID;
    grdMotorParameter->Options=TGridOptions() << goFixedVertLine << goFixedHorzLine << goVertLine << goHorzLine << goColSizing;
    grdMotorParameter->OnSelectCell=grdMotorParameterSelectCell;
    grdMotorParameter->OnDblClick=grdMotorParameterDblClick;
    grdMotorParameter->Cells[eMotorParamColNo][0]="No";
    grdMotorParameter->Cells[eMotorParamColAlias][0]="Alias";
    grdMotorParameter->Cells[eMotorParamColCard][0]="Card";
    grdMotorParameter->Cells[eMotorParamColAddress][0]="Address";
    grdMotorParameter->Cells[eMotorParamColEnable][0]="Enable";
    grdMotorParameter->Cells[eMotorParamColHome][0]="Home";
    grdMotorParameter->Cells[eMotorParamColSoftN][0]="Soft N";
    grdMotorParameter->Cells[eMotorParamColSoftP][0]="Soft P";
    grdMotorParameter->Cells[eMotorParamColInit][0]="Init";
    grdMotorParameter->Cells[eMotorParamColJogLow][0]="Jog Low";
    grdMotorParameter->Cells[eMotorParamColJogHigh][0]="Jog High";
    grdMotorParameter->Cells[eMotorParamColHomeLow][0]="Home Low";
    grdMotorParameter->Cells[eMotorParamColHomeHigh][0]="Home High";
    grdMotorParameter->Cells[eMotorParamColAcc][0]="Acc";
    grdMotorParameter->Cells[eMotorParamColDec][0]="Dec";
    grdMotorParameter->Cells[eMotorParamColRange][0]="Range";
    grdMotorParameter->ColWidths[eMotorParamColNo]=55;
    grdMotorParameter->ColWidths[eMotorParamColAlias]=150;
    grdMotorParameter->ColWidths[eMotorParamColCard]=80;
    grdMotorParameter->ColWidths[eMotorParamColAddress]=70;
}
//---------------------------------------------------------------------------
void TfMotorTest::ConfigureInformationGrid()
{
    if(grdInformation==NULL)
        return;
    grdInformation->ColCount=eInfoColTotal;
    grdInformation->FixedRows=1;
    grdInformation->FixedCols=0;
    grdInformation->DefaultRowHeight=24;
    grdInformation->Color=MOTOR_TEST_COLOR_GRID;
    grdInformation->Options=grdInformation->Options << goRowSelect << goColSizing;
    grdInformation->Cells[eInfoColNo][0]="No";
    grdInformation->Cells[eInfoColAlias][0]="Alias";
    grdInformation->Cells[eInfoColCard][0]="Card";
    grdInformation->Cells[eInfoColBoard][0]="Board";
    grdInformation->Cells[eInfoColPort][0]="Port";
    grdInformation->Cells[eInfoColAddress][0]="Address";
    grdInformation->Cells[eInfoColKind][0]="Kind";
    grdInformation->Cells[eInfoColFlushPanel][0]="Flush Panel";
    grdInformation->Cells[eInfoColServoConfig][0]="ServoCfg";
    grdInformation->Cells[eInfoColIsServo][0]="IsServo";
    grdInformation->Cells[eInfoColMotorRelay][0]="MotorRelay";
    grdInformation->Cells[eInfoColServerON][0]="ServerON";
    grdInformation->Cells[eInfoColMotorPower][0]="SnMotorPower";
    grdInformation->Cells[eInfoColPowerState][0]="PowerState";
    grdInformation->Cells[eInfoColPowerDelay][0]="Delay";
    grdInformation->ColWidths[eInfoColNo]=45;
    grdInformation->ColWidths[eInfoColAlias]=145;
    grdInformation->ColWidths[eInfoColCard]=80;
    grdInformation->ColWidths[eInfoColBoard]=55;
    grdInformation->ColWidths[eInfoColPort]=45;
    grdInformation->ColWidths[eInfoColAddress]=70;
    grdInformation->ColWidths[eInfoColKind]=45;
    grdInformation->ColWidths[eInfoColFlushPanel]=110;
    grdInformation->ColWidths[eInfoColServoConfig]=70;
    grdInformation->ColWidths[eInfoColIsServo]=60;
    grdInformation->ColWidths[eInfoColMotorRelay]=80;
    grdInformation->ColWidths[eInfoColServerON]=70;
    grdInformation->ColWidths[eInfoColMotorPower]=90;
    grdInformation->ColWidths[eInfoColPowerState]=75;
    grdInformation->ColWidths[eInfoColPowerDelay]=50;
}
//---------------------------------------------------------------------------
void TfMotorTest::ConfigureDriverRegisterGrid()
{
    const char *DefaultRegisters[]={"0x00", "0x02", "0x04", "0x08"};

    if(grdDriverRegister==NULL)
        return;
    grdDriverRegister->ColCount=eDriverRegColTotal;
    grdDriverRegister->RowCount=5;
    grdDriverRegister->FixedRows=1;
    grdDriverRegister->FixedCols=0;
    grdDriverRegister->DefaultRowHeight=24;
    grdDriverRegister->Color=MOTOR_TEST_COLOR_GRID;
    grdDriverRegister->Options=TGridOptions() << goFixedVertLine << goFixedHorzLine << goVertLine << goHorzLine << goRowSelect << goColSizing;
    grdDriverRegister->Cells[eDriverRegColRegister][0]="Register";
    grdDriverRegister->Cells[eDriverRegColValue][0]="Value";
    grdDriverRegister->Cells[eDriverRegColResult][0]="Result";
    grdDriverRegister->ColWidths[eDriverRegColRegister]=110;
    grdDriverRegister->ColWidths[eDriverRegColValue]=160;
    grdDriverRegister->ColWidths[eDriverRegColResult]=420;
    for(int Index=0; Index<4; Index++)
    {
        grdDriverRegister->Cells[eDriverRegColRegister][Index+1]=DefaultRegisters[Index];
        grdDriverRegister->Cells[eDriverRegColValue][Index+1]="";
        grdDriverRegister->Cells[eDriverRegColResult][Index+1]="Legacy MC88X1 read-only offset";
    }
}
//---------------------------------------------------------------------------
void TfMotorTest::ConfigureServoGuardGrid()
{
    if(grdServoGuard==NULL)
        return;
    grdServoGuard->ColCount=eServoGuardColTotal;
    grdServoGuard->RowCount=18;
    grdServoGuard->FixedRows=1;
    grdServoGuard->FixedCols=0;
    grdServoGuard->DefaultRowHeight=24;
    grdServoGuard->Color=MOTOR_TEST_COLOR_GRID;
    grdServoGuard->Options=TGridOptions() << goFixedVertLine << goFixedHorzLine << goVertLine << goHorzLine << goRowSelect << goColSizing;
    grdServoGuard->Cells[eServoGuardColItem][0]="Item";
    grdServoGuard->Cells[eServoGuardColValue][0]="Value";
    grdServoGuard->Cells[eServoGuardColNote][0]="Note";
    grdServoGuard->ColWidths[eServoGuardColItem]=160;
    grdServoGuard->ColWidths[eServoGuardColValue]=160;
    grdServoGuard->ColWidths[eServoGuardColNote]=470;
    SetServoGuardRow(1, "Action", "Not checked", "Dry-run guard only");
    SetServoGuardRow(2, "Result", "Not checked", "No IO changed");
    SetServoGuardRow(15, "Mode", "Not checked", "Guard or apply");
    SetServoGuardRow(16, "Actuation", "Not checked", "No output changed");
    SetServoGuardRow(17, "ServerON After", "Not checked", "Read after apply");
}
//---------------------------------------------------------------------------
int TfMotorTest::GetMotorCount()
{
    if(HSys.iTotalMotor<MAX_MOTOR_TEST_MOTOR_COUNT)
        return HSys.iTotalMotor;
    return MAX_MOTOR_TEST_MOTOR_COUNT;
}
//---------------------------------------------------------------------------
void TfMotorTest::FillMotorList()
{
    int OldIndex;
    int MotorCount;

    // Motor selection moved to the grdOperate row (blue bar). This keeps the
    // active index valid and re-syncs the right-hand active-motor panel.
    MotorCount=GetMotorCount();
    if(MotorCount<=0)
        return;
    OldIndex=ActiveMotorIndex;
    if(OldIndex<0 || OldIndex>=MotorCount)
        OldIndex=0;
    SetActiveMotor(OldIndex);
}
//---------------------------------------------------------------------------
void TfMotorTest::SetActiveMotor(int Index)
{
    TTrayMotor *Motor;

    Motor=GetMotor(Index);
    if(Motor==NULL)
        return;

    ActiveMotorIndex=Index;
    if(palMotorName!=NULL)
        palMotorName->Caption=Motor->NumberAlias;
    if(grdMotorParameter!=NULL && Index+1<grdMotorParameter->RowCount)
        grdMotorParameter->Row=Index+1;
    if(grdOperate!=NULL)
        grdOperate->Invalidate();
    if(edSpeedPercent!=NULL)
    {
        // Always default to a safe 30% speed on every motor selection (operator
        // request), regardless of the motor's stored percent, so a high speed left
        // from a previously selected axis cannot cause a dangerous first move.
        int Percent=30;
        edSpeedPercent->Text=IntToStr(Percent);
        if(scrSpeedPercent!=NULL)
            scrSpeedPercent->Position=Percent;
        Motor->SetPersentSpeed(Percent, false);
    }
    UpdateActivePositionEdits();
    UpdateMotorMonitor();
}
//---------------------------------------------------------------------------
TTrayMotor *TfMotorTest::GetMotor(int Index)
{
    if(Index<0 || Index>=GetMotorCount() || HSys.MotPtr==NULL)
        return NULL;
    return HSys.MotPtr[Index];
}
//---------------------------------------------------------------------------
TTrayMotor *TfMotorTest::GetActiveMotor()
{
    return GetMotor(ActiveMotorIndex);
}
//---------------------------------------------------------------------------
int TfMotorTest::GetEditInt(TEdit *Edit, int DefaultValue)
{
    if(Edit==NULL || Edit->Text==AnsiString(""))
        return DefaultValue;
    return atoi(Edit->Text.c_str());
}
//---------------------------------------------------------------------------
bool TfMotorTest::ShowMotorTestKeyboard(TEdit *Edit, int Function, int DecimalPoint, bool CheckRange, double MinValue, double MaxValue, AnsiString TitleText)
{
    if(Edit==NULL)
        return false;
    if(fQwertyKey==NULL)
        fQwertyKey=new TfQwertyKey(this);
    return fQwertyKey->ShowQwertyKey(Edit, Function, DecimalPoint, CheckRange, MinValue, MaxValue, TitleText);
}
//---------------------------------------------------------------------------
int TfMotorTest::ParsePositionText(AnsiString Text)
{
    double Value;

    if(Text==AnsiString(""))
        return 0;
    Value=atof(Text.c_str());
    if(Value>=0.0)
        return (int)(Value*100.0+0.5);
    return (int)(Value*100.0-0.5);
}
//---------------------------------------------------------------------------
AnsiString TfMotorTest::FormatPositionText(int Value)
{
    AnsiString Text;

    Text.sprintf("%.2f", (double)Value/100.0);
    return Text;
}
//---------------------------------------------------------------------------
AnsiString TfMotorTest::BoolText(bool Value)
{
    return Value?AnsiString("ON"):AnsiString("OFF");
}
//---------------------------------------------------------------------------
AnsiString TfMotorTest::GetMotorTestIniFileName()
{
    AnsiString RootPath=HSys.CurrentDir;

    if(RootPath==AnsiString(""))
        RootPath="..";
    return RootPath+AnsiString("\\system\\motor_test.ini");
}
//---------------------------------------------------------------------------
bool TfMotorTest::ConfirmDiscardMotorParameterEdit()
{
    int Ret;

    if(bMotorParameterDirty==false)
        return true;

    Ret=MessageDlg("Discard unsaved motor parameter changes?", mtConfirmation, TMsgDlgButtons() << mbYes << mbNo, 0);
    if(Ret==mrYes)
    {
        bMotorParameterDirty=false;
        RefreshMotorParameterGrid();
        SetMessage("Motor parameter edit discarded");
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool TfMotorTest::CheckNoUnsavedMotorParameter(AnsiString ActionText)
{
    if(bMotorParameterDirty==false)
        return true;
    MessageDlg(AnsiString("Save or reload motor parameter first before ")+ActionText+AnsiString("."), mtWarning, TMsgDlgButtons() << mbOK, 0);
    SetMessage("Action blocked: parameter not saved");
    return false;
}
//---------------------------------------------------------------------------
bool TfMotorTest::CheckNoUnsavedMotorTable(AnsiString ActionText)
{
    if(bMotorTableDirty==false)
        return true;
    MessageDlg(AnsiString("Save or reload Mot_Table first before ")+ActionText+AnsiString("."), mtWarning, TMsgDlgButtons() << mbOK, 0);
    SetMessage("Action blocked: Mot_Table not saved");
    return false;
}
//---------------------------------------------------------------------------
bool TfMotorTest::CheckMotorDataEditIdle()
{
    if(HSys.Sys.SystemStart)
    {
        MessageDlg("Machine is running.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Action blocked: system start");
        return false;
    }
    if(bHomeRunning || bLoopRunning)
    {
        MessageDlg("Motor is busy.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Action blocked: motor busy");
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
bool TfMotorTest::IsMotorParameterEditableColumn(int ColIndex)
{
    // Every configurable column is editable here; access to this screen is gated at
    // the screen level (permission), so individual fields are not locked. Only the
    // No column (row identity) and the Home column (live home status, not a setting)
    // stay read-only.
    return (ColIndex==eMotorParamColAlias     || ColIndex==eMotorParamColCard   ||
            ColIndex==eMotorParamColAddress   || ColIndex==eMotorParamColEnable ||
            ColIndex==eMotorParamColSoftN     || ColIndex==eMotorParamColSoftP  ||
            ColIndex==eMotorParamColInit      || ColIndex==eMotorParamColJogLow ||
            ColIndex==eMotorParamColJogHigh   || ColIndex==eMotorParamColHomeLow||
            ColIndex==eMotorParamColHomeHigh  || ColIndex==eMotorParamColAcc    ||
            ColIndex==eMotorParamColDec       || ColIndex==eMotorParamColRange);
}
//---------------------------------------------------------------------------
AnsiString TfMotorTest::GetMotorParameterCsvName(int ColIndex)
{
    switch(ColIndex)
    {
        case eMotorParamColAlias:    return "Alias";
        case eMotorParamColCard:     return "CardModel";
        case eMotorParamColEnable:   return "Enable";
        // eMotorParamColAddress maps to two CSV columns (BoardID + Port) and is
        // handled specially in SaveMotorParameterToFile, not via this 1:1 lookup.
        case eMotorParamColSoftN:    return "SoftLimitN";
        case eMotorParamColSoftP:    return "SoftLimitP";
        case eMotorParamColInit:     return "InitSpeed";
        case eMotorParamColJogLow:   return "JogLowSpeed";
        case eMotorParamColJogHigh:  return "JogHighSpeed";
        case eMotorParamColHomeLow:  return "HomeLowSpeed";
        case eMotorParamColHomeHigh: return "HomeHighSpeed";
        case eMotorParamColAcc:      return "Acc";
        case eMotorParamColDec:      return "Dec";
        case eMotorParamColRange:    return "Range";
    }
    return "";
}
//---------------------------------------------------------------------------
bool TfMotorTest::ValidateMotorParameterValue(int ColIndex, AnsiString InputText, AnsiString &DisplayText, AnsiString &CsvText, AnsiString &ErrorText)
{
    double DoubleValue;
    int IntValue;

    DisplayText="";
    CsvText="";
    ErrorText="";

    if(IsMotorParameterEditableColumn(ColIndex)==false)
    {
        ErrorText="This parameter is read only.";
        return false;
    }

    if(ColIndex==eMotorParamColAlias)
    {
        DisplayText=MotorTestTrim(InputText);
        if(DisplayText==AnsiString(""))
        {
            ErrorText="Alias must not be empty.";
            return false;
        }
        CsvText=DisplayText;
        return true;
    }

    if(ColIndex==eMotorParamColCard)
    {
        DisplayText=MotorTestTrim(InputText).UpperCase();
        // Match the card models the system actually instantiates (database.cpp /
        // MyMotor InitialMotorObject). Anything else would build a no-op HTMotor.
        if(DisplayText!=AnsiString("MC88X1") && DisplayText!=AnsiString("MC88X1P") &&
           DisplayText!=AnsiString("SMC")    && DisplayText!=AnsiString("MN200")   &&
           DisplayText!=AnsiString("SYNTEK"))
        {
            ErrorText="Card must be MC88X1, MC88X1P, SMC, MN200 or SYNTEK.";
            return false;
        }
        CsvText=DisplayText;
        return true;
    }

    if(ColIndex==eMotorParamColEnable)
    {
        AnsiString Flag=MotorTestTrim(InputText).UpperCase();
        if(Flag==AnsiString("ON") || Flag==AnsiString("1") || Flag==AnsiString("TRUE") || Flag==AnsiString("Y"))
        {
            DisplayText="ON";
            CsvText="1";
            return true;
        }
        if(Flag==AnsiString("OFF") || Flag==AnsiString("0") || Flag==AnsiString("FALSE") || Flag==AnsiString("N"))
        {
            DisplayText="OFF";
            CsvText="0";
            return true;
        }
        ErrorText="Enable must be ON or OFF.";
        return false;
    }

    if(ColIndex==eMotorParamColAddress)
    {
        // Address is shown as the card-specific composite (BoardID*10+Port for
        // MC88X1/SMC, BoardID*100+Port for MN200). Validate the integer here; the
        // BoardID/Port split is done in SaveMotorParameterToFile using the row's
        // Card value so it stays correct when both columns are edited together.
        if(MotorTestTryParseInt(InputText, IntValue)==false)
        {
            ErrorText="Address must be integer.";
            return false;
        }
        if(IntValue<0)
        {
            ErrorText="Address must not be negative.";
            return false;
        }
        CsvText=IntToStr(IntValue);
        DisplayText=CsvText;
        return true;
    }

    if(ColIndex==eMotorParamColSoftN || ColIndex==eMotorParamColSoftP)
    {
        if(MotorTestTryParseDouble(InputText, DoubleValue)==false)
        {
            ErrorText="Soft limit must be numeric.";
            return false;
        }
        if(DoubleValue>999999.0 || DoubleValue<-999999.0)
        {
            ErrorText="Soft limit is out of range.";
            return false;
        }
        if(DoubleValue>=0.0)
            IntValue=(int)(DoubleValue*100.0+0.5);
        else
            IntValue=(int)(DoubleValue*100.0-0.5);
        CsvText=IntToStr(IntValue);
        DisplayText=FormatPositionText(IntValue);
        return true;
    }

    if(ColIndex==eMotorParamColAcc || ColIndex==eMotorParamColDec)
    {
        if(MotorTestTryParseDouble(InputText, DoubleValue)==false)
        {
            ErrorText="Acc/Dec must be numeric.";
            return false;
        }
        if(DoubleValue<=0.0 || DoubleValue>100000.0)
        {
            ErrorText="Acc/Dec must be greater than 0.";
            return false;
        }
        DisplayText=MotorTestFormatDouble(DoubleValue);
        CsvText=DisplayText;
        return true;
    }

    if(MotorTestTryParseInt(InputText, IntValue)==false)
    {
        ErrorText="Parameter must be integer.";
        return false;
    }
    if(IntValue<=0)
    {
        ErrorText="Parameter must be greater than 0.";
        return false;
    }
    CsvText=IntToStr(IntValue);
    DisplayText=CsvText;
    return true;
}
//---------------------------------------------------------------------------
bool TfMotorTest::ValidateMotorParameterRow(int RowIndex, AnsiString &ErrorText)
{
    AnsiString DisplayText;
    AnsiString CsvText;
    AnsiString LocalError;
    int SoftN;
    int SoftP;
    int JogLow;
    int JogHigh;
    int HomeLow;
    int HomeHigh;

    ErrorText="";
    if(grdMotorParameter==NULL || RowIndex<=0 || RowIndex>=grdMotorParameter->RowCount)
    {
        ErrorText="Invalid motor parameter row.";
        return false;
    }

    for(int ColIndex=0; ColIndex<grdMotorParameter->ColCount; ColIndex++)
    {
        if(IsMotorParameterEditableColumn(ColIndex)==false)
            continue;
        if(ValidateMotorParameterValue(ColIndex, grdMotorParameter->Cells[ColIndex][RowIndex], DisplayText, CsvText, LocalError)==false)
        {
            ErrorText=grdMotorParameter->Cells[ColIndex][0]+AnsiString(": ")+LocalError;
            return false;
        }
    }

    ValidateMotorParameterValue(eMotorParamColSoftN, grdMotorParameter->Cells[eMotorParamColSoftN][RowIndex], DisplayText, CsvText, LocalError);
    MotorTestTryParseInt(CsvText, SoftN);
    ValidateMotorParameterValue(eMotorParamColSoftP, grdMotorParameter->Cells[eMotorParamColSoftP][RowIndex], DisplayText, CsvText, LocalError);
    MotorTestTryParseInt(CsvText, SoftP);
    ValidateMotorParameterValue(eMotorParamColJogLow, grdMotorParameter->Cells[eMotorParamColJogLow][RowIndex], DisplayText, CsvText, LocalError);
    MotorTestTryParseInt(CsvText, JogLow);
    ValidateMotorParameterValue(eMotorParamColJogHigh, grdMotorParameter->Cells[eMotorParamColJogHigh][RowIndex], DisplayText, CsvText, LocalError);
    MotorTestTryParseInt(CsvText, JogHigh);
    ValidateMotorParameterValue(eMotorParamColHomeLow, grdMotorParameter->Cells[eMotorParamColHomeLow][RowIndex], DisplayText, CsvText, LocalError);
    MotorTestTryParseInt(CsvText, HomeLow);
    ValidateMotorParameterValue(eMotorParamColHomeHigh, grdMotorParameter->Cells[eMotorParamColHomeHigh][RowIndex], DisplayText, CsvText, LocalError);
    MotorTestTryParseInt(CsvText, HomeHigh);

    if(SoftN>=SoftP)
    {
        ErrorText="Soft N must be less than Soft P.";
        return false;
    }
    if(JogLow>JogHigh)
    {
        ErrorText="Jog Low must not exceed Jog High.";
        return false;
    }
    if(HomeLow>HomeHigh)
    {
        ErrorText="Home Low must not exceed Home High.";
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
int TfMotorTest::FindCsvColumn(TStringList *HeaderList, AnsiString ColumnName)
{
    if(HeaderList==NULL)
        return -1;
    ColumnName=ColumnName.UpperCase();
    for(int ColIndex=0; ColIndex<HeaderList->Count; ColIndex++)
    {
        if(HeaderList->Strings[ColIndex].UpperCase()==ColumnName)
            return ColIndex;
    }
    return -1;
}
//---------------------------------------------------------------------------
int TfMotorTest::FindMotorTableRow(TStringList *LineList, TTrayMotor *Motor, TStringList *HeaderList)
{
    TStringList *RowFields;
    int MotorNameCol;
    int AliasCol;
    int Result;

    if(LineList==NULL || Motor==NULL || HeaderList==NULL)
        return -1;

    MotorNameCol=FindCsvColumn(HeaderList, "Motorname");
    AliasCol=FindCsvColumn(HeaderList, "Alias");
    Result=-1;
    RowFields=new TStringList;
    try
    {
        for(int RowIndex=1; RowIndex<LineList->Count; RowIndex++)
        {
            RowFields->CommaText=LineList->Strings[RowIndex];
            if(MotorNameCol>=0 && MotorNameCol<RowFields->Count && RowFields->Strings[MotorNameCol]==Motor->Number)
            {
                Result=RowIndex;
                break;
            }
            if(AliasCol>=0 && AliasCol<RowFields->Count && RowFields->Strings[AliasCol]==Motor->Alias)
            {
                Result=RowIndex;
                break;
            }
        }
    }
    __finally
    {
        delete RowFields;
    }
    return Result;
}
//---------------------------------------------------------------------------
AnsiString TfMotorTest::GetMotorTableColumnName(int ColumnIndex)
{
    switch(ColumnIndex)
    {
        case 0:  return "Motorname";
        case 1:  return "Alias";
        case 2:  return "Direction";
        case 3:  return "GearRatio";
        case 4:  return "HomeDirectior";
        case 5:  return "HomeHighSpeed";
        case 6:  return "HomeLowSpeed";
        case 7:  return "InitSpeed";
        case 8:  return "JogHighSpeed";
        case 9:  return "JogLowSpeed";
        case 10: return "Rate";
        case 11: return "SoftLimitN";
        case 12: return "SoftLimitP";
        case 13: return "Enable";
        case 14: return "ServoAlarmOn";
        case 15: return "Range";
        case 16: return "1P2P";
        case 17: return "SensorType";
        case 18: return "SimulateSpeed";
        case 19: return "CardModel";
        case 20: return "BoardID";
        case 21: return "Port";
        case 22: return "Acc";
        case 23: return "Dec";
        case 24: return "MotorKind";
        case 25: return "FlushPanel";
        case 26: return "HomeOrder";
        case 27: return "LimitLogic";
        case 28: return "In1Logic";
    }
    return AnsiString("Column ")+IntToStr(ColumnIndex);
}
//---------------------------------------------------------------------------
bool TfMotorTest::ValidateMotorTableCsv(AnsiString &SummaryText, AnsiString &ErrorText)
{
    TStringList *LineList;
    TStringList *HeaderList;
    TStringList *RowFields;
    TStringList *MotorNameList;
    TStringList *AliasList;
    TMOTNO TableNo;
    int MissingIndex;
    int ValidRows;

    SummaryText="";
    ErrorText="";
    if(HSys.MotTablePath==AnsiString("") || FileExists(HSys.MotTablePath)==false)
    {
        ErrorText="Mot_Table.csv not found.";
        return false;
    }

    LineList=new TStringList;
    HeaderList=new TStringList;
    RowFields=new TStringList;
    MotorNameList=new TStringList;
    AliasList=new TStringList;
    ValidRows=0;
    try
    {
        LineList->LoadFromFile(HSys.MotTablePath);
        if(LineList->Count<=1)
            throw Exception("Mot_Table.csv has no motor data.");

        MissingIndex=TableNo.SetMOTTableNo(LineList->Strings[0]);
        if(MissingIndex!=TableNo.emotTotal)
            throw Exception(AnsiString("Missing Mot_Table column: ")+GetMotorTableColumnName(MissingIndex));

        HeaderList->CommaText=LineList->Strings[0];
        for(int RowIndex=1; RowIndex<LineList->Count; RowIndex++)
        {
            AnsiString MotorName;
            AnsiString Alias;
            AnsiString CardModel;
            AnsiString Key;
            int RowNo=RowIndex+1;
            int Direction;
            int HomeDirection;
            int HomeHigh;
            int HomeLow;
            int InitSpeed;
            int JogHigh;
            int JogLow;
            int Rate;
            int SoftN;
            int SoftP;
            int Enable;
            int ServoAlarmOn;
            int Range;
            int OnePTwoP;
            int SensorType;
            int SimulateSpeed;
            int BoardID;
            int Port;
            int MotorKind;
            int LimitLogic;
            int In1Logic;
            double GearRatio;
            double Acc;
            double Dec;

            if(MotorTestTrim(LineList->Strings[RowIndex])==AnsiString(""))
                continue;

            RowFields->CommaText=LineList->Strings[RowIndex];
            if(RowFields->Count<HeaderList->Count)
                throw Exception(AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" field count is less than header count."));

            MotorName=MotorTestTrim(MotorTestCsvField(RowFields, TableNo.emotNo));
            Alias=MotorTestTrim(MotorTestCsvField(RowFields, TableNo.emotAlias));
            CardModel=MotorTestTrim(MotorTestCsvField(RowFields, TableNo.emotCardModel));
            if(MotorName==AnsiString(""))
                throw Exception(AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" missing Motorname."));
            if(Alias==AnsiString(""))
                throw Exception(AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" missing Alias."));
            if(CardModel==AnsiString(""))
                throw Exception(AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" missing CardModel."));

            Key=MotorName.UpperCase();
            if(MotorNameList->IndexOf(Key)>=0)
                throw Exception(AnsiString("Duplicate Motorname: ")+MotorName);
            MotorNameList->Add(Key);
            Key=Alias.UpperCase();
            if(AliasList->IndexOf(Key)>=0)
                throw Exception(AnsiString("Duplicate Alias: ")+Alias);
            AliasList->Add(Key);

            if(MotorTestReadIntField(RowFields, TableNo.emotDirection, "Direction", RowNo, Direction, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadDoubleField(RowFields, TableNo.emotGearRatio, "GearRatio", RowNo, GearRatio, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotHomeDirectior, "HomeDirectior", RowNo, HomeDirection, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotHomeHighSpeed, "HomeHighSpeed", RowNo, HomeHigh, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotHomeLowSpeed, "HomeLowSpeed", RowNo, HomeLow, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotInitSpeed, "InitSpeed", RowNo, InitSpeed, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotJogHighSpeed, "JogHighSpeed", RowNo, JogHigh, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotJogLowSpeed, "JogLowSpeed", RowNo, JogLow, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotRate, "Rate", RowNo, Rate, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotSoftLimitN, "SoftLimitN", RowNo, SoftN, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotSoftLimitP, "SoftLimitP", RowNo, SoftP, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotEnable, "Enable", RowNo, Enable, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotServoAlarmOn, "ServoAlarmOn", RowNo, ServoAlarmOn, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotRange, "Range", RowNo, Range, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emot1P2P, "1P2P", RowNo, OnePTwoP, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotSensorType, "SensorType", RowNo, SensorType, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotSimulateSpeed, "SimulateSpeed", RowNo, SimulateSpeed, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotBoardID, "BoardID", RowNo, BoardID, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotPort, "Port", RowNo, Port, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadDoubleField(RowFields, TableNo.emotAcc, "Acc", RowNo, Acc, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadDoubleField(RowFields, TableNo.emotDec, "Dec", RowNo, Dec, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotMotorKind, "MotorKind", RowNo, MotorKind, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotLimitLogic, "LimitLogic", RowNo, LimitLogic, ErrorText)==false) throw Exception(ErrorText);
            if(MotorTestReadIntField(RowFields, TableNo.emotIn1Logic, "In1Logic", RowNo, In1Logic, ErrorText)==false) throw Exception(ErrorText);

            if(GearRatio<=0.0) throw Exception(AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" GearRatio must be greater than 0."));
            if(HomeLow>HomeHigh) throw Exception(AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" HomeLowSpeed must not exceed HomeHighSpeed."));
            if(JogLow>JogHigh) throw Exception(AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" JogLowSpeed must not exceed JogHighSpeed."));
            if(SoftN>=SoftP) throw Exception(AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" SoftLimitN must be less than SoftLimitP."));
            if(InitSpeed<=0 || JogHigh<=0 || JogLow<=0 || HomeHigh<=0 || HomeLow<=0 || Rate<=0 || Range<=0 || SimulateSpeed<=0)
                throw Exception(AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" speed/rate/range values must be greater than 0."));
            if(Acc<=0.0 || Dec<=0.0) throw Exception(AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" Acc/Dec must be greater than 0."));
            if(BoardID<0 || Port<0) throw Exception(AnsiString("Row ")+IntToStr(RowNo)+AnsiString(" BoardID/Port must not be negative."));

            ValidRows++;
        }

        if(ValidRows<=0)
            throw Exception("Mot_Table.csv has no valid motor rows.");
        SummaryText=AnsiString("Mot_Table validate OK. Rows=")+IntToStr(ValidRows)+AnsiString(", Columns=")+IntToStr(HeaderList->Count);
    }
    catch(Exception &E)
    {
        ErrorText=E.Message;
    }
    catch(...)
    {
        ErrorText="Unknown Mot_Table validation error.";
    }

    delete AliasList;
    delete MotorNameList;
    delete RowFields;
    delete HeaderList;
    delete LineList;

    return ErrorText==AnsiString("");
}
//---------------------------------------------------------------------------
AnsiString TfMotorTest::GetMotorTableLogFileName()
{
    AnsiString RootPath=HSys.CurrentDir;

    if(RootPath==AnsiString(""))
        RootPath="..";
    return RootPath+AnsiString("\\logs\\motor_table_save.log");
}
//---------------------------------------------------------------------------
bool TfMotorTest::AppendMotorTableSaveLog(AnsiString BackupPath, TStringList *ChangeList, AnsiString &ErrorText)
{
    TStringList *LogList;
    AnsiString FileName;

    ErrorText="";
    if(ChangeList==NULL)
    {
        ErrorText="No change list.";
        return false;
    }

    FileName=GetMotorTableLogFileName();
    LogList=new TStringList;
    try
    {
        ForceDirectories(ExtractFilePath(FileName));
        if(FileExists(FileName))
            LogList->LoadFromFile(FileName);
        LogList->Add("------------------------------------------------------------");
        LogList->Add(AnsiString("Time=")+FormatDateTime("yyyy/mm/dd hh:nn:ss", Now()));
        LogList->Add(AnsiString("MotTable=")+HSys.MotTablePath);
        LogList->Add(AnsiString("Backup=")+BackupPath);
        LogList->Add(AnsiString("ChangeCount=")+IntToStr(ChangeList->Count));
        for(int Index=0; Index<ChangeList->Count; Index++)
            LogList->Add(ChangeList->Strings[Index]);
        LogList->SaveToFile(FileName);
    }
    catch(Exception &E)
    {
        ErrorText=E.Message;
    }
    catch(...)
    {
        ErrorText="Unknown Mot_Table save log error.";
    }
    delete LogList;

    return ErrorText==AnsiString("");
}
//---------------------------------------------------------------------------
bool TfMotorTest::BackupMotorTable(AnsiString &BackupPath, AnsiString &ErrorText)
{
    AnsiString BackupDir;
    AnsiString Mask;
    TSearchRec SearchRec;
    TStringList *Names;
    int FindResult;
    int DeleteCount;
    const int MaxKeep=10;

    BackupPath="";
    ErrorText="";
    if(HSys.MotTablePath==AnsiString("") || FileExists(HSys.MotTablePath)==false)
    {
        ErrorText="Mot_Table.csv not found.";
        return false;
    }

    // Mirror the IO_Table backup model (iosetview BackupIOTableFile): keep Mot_Table
    // backups in a dedicated backupMotor subfolder instead of loose next to
    // Mot_Table.csv, and cap the count by deleting the oldest first so the folder
    // does not grow without bound. The Mot_Table_yyyymmdd_hhnnss name sorts
    // chronologically, so the first (Count-MaxKeep) entries are the oldest.
    BackupDir=ExtractFilePath(HSys.MotTablePath)+AnsiString("backupMotor");
    if(!DirectoryExists(BackupDir) && !ForceDirectories(BackupDir))
    {
        ErrorText="Cannot create backupMotor folder.";
        return false;
    }
    BackupPath=BackupDir+AnsiString("\\Mot_Table_")+FormatDateTime("yyyymmdd_hhnnss", Now())+AnsiString("_")+IntToStr((int)(GetTickCount() & 0xFFFF))+AnsiString(".csv");
    if(CopyFile(HSys.MotTablePath.c_str(), BackupPath.c_str(), false)==0)
    {
        ErrorText=SysErrorMessage(GetLastError());
        return false;
    }

    Names=new TStringList();
    Mask=BackupDir+AnsiString("\\Mot_Table_*.csv");
    FindResult=FindFirst(Mask, faAnyFile, SearchRec);
    while(FindResult==0)
    {
        if((SearchRec.Attr & faDirectory)==0)
            Names->Add(SearchRec.Name);
        FindResult=FindNext(SearchRec);
    }
    FindClose(SearchRec);
    Names->Sort();
    DeleteCount=Names->Count-MaxKeep;
    for(int Index=0; Index<DeleteCount; Index++)
        DeleteFile(BackupDir+AnsiString("\\")+Names->Strings[Index]);
    delete Names;

    return true;
}
//---------------------------------------------------------------------------
void TfMotorTest::EditMotorParameterCell(int RowIndex, int ColIndex)
{
    AnsiString ValueText;
    AnsiString DisplayText;
    AnsiString CsvText;
    AnsiString ErrorText;
    AnsiString Prompt;

    if(grdMotorParameter==NULL)
        return;
    if(RowIndex<=0 || RowIndex>=grdMotorParameter->RowCount)
        return;
    if(RowIndex-1!=ActiveMotorIndex)
        SetActiveMotor(RowIndex-1);
    if(IsMotorParameterEditableColumn(ColIndex)==false)
    {
        SetMessage("Read only parameter");
        return;
    }
    if(CheckNoUnsavedMotorTable("editing motor parameter")==false)
        return;
    if(CheckMotorDataEditIdle()==false)
        return;

    if(ColIndex==eMotorParamColEnable)
    {
        // Enable is a simple ON/OFF flag; toggle it directly instead of opening the
        // keyboard so the operator just taps the cell.
        AnsiString CurFlag=MotorTestTrim(grdMotorParameter->Cells[ColIndex][RowIndex]).UpperCase();
        ValueText=(CurFlag==AnsiString("ON"))?AnsiString("OFF"):AnsiString("ON");
        if(ValidateMotorParameterValue(ColIndex, ValueText, DisplayText, CsvText, ErrorText)==false)
        {
            MessageDlg(ErrorText, mtWarning, TMsgDlgButtons() << mbOK, 0);
            return;
        }
        grdMotorParameter->Cells[ColIndex][RowIndex]=DisplayText;
        if(ValidateMotorParameterRow(RowIndex, ErrorText)==false)
        {
            MessageDlg(ErrorText, mtWarning, TMsgDlgButtons() << mbOK, 0);
            RefreshMotorParameterGrid();
            return;
        }
        bMotorParameterDirty=true;
        SetMessage("Motor parameter modified, save required");
        return;
    }

    ValueText=grdMotorParameter->Cells[ColIndex][RowIndex];
    Prompt=grdMotorParameter->Cells[ColIndex][0]+AnsiString(" : ")+grdMotorParameter->Cells[eMotorParamColAlias][RowIndex];
    TEdit *Edit=new TEdit(this);
    try
    {
        Edit->Parent=this;
        Edit->Visible=false;
        Edit->Text=ValueText;
        if(ColIndex==eMotorParamColAlias)
        {
            // String column -> QWERTY text keyboard (no spaces in motor aliases).
            if(ShowMotorTestKeyboard(Edit, N_NO_SPACE, 0, false, 0.0, 0.0, Prompt)==false)
                return;
        }
        else if(ColIndex==eMotorParamColCard)
        {
            // Card model is an uppercase token -> uppercase QWERTY keyboard.
            if(ShowMotorTestKeyboard(Edit, N_UPPERCASE|N_NO_SPACE, 0, false, 0.0, 0.0, Prompt)==false)
                return;
        }
        else if(ColIndex==eMotorParamColSoftN || ColIndex==eMotorParamColSoftP ||
                ColIndex==eMotorParamColAcc   || ColIndex==eMotorParamColDec)
        {
            if(ShowMotorTestKeyboard(Edit, N_DOUBLE, 2, false, 0.0, 0.0, Prompt)==false)
                return;
        }
        else
        {
            if(ShowMotorTestKeyboard(Edit, N_INTEGER, 1, false, 0.0, 0.0, Prompt)==false)
                return;
        }
        ValueText=Edit->Text;
    }
    __finally
    {
        delete Edit;
    }

    if(ValidateMotorParameterValue(ColIndex, ValueText, DisplayText, CsvText, ErrorText)==false)
    {
        MessageDlg(ErrorText, mtWarning, TMsgDlgButtons() << mbOK, 0);
        return;
    }

    grdMotorParameter->Cells[ColIndex][RowIndex]=DisplayText;
    if(ValidateMotorParameterRow(RowIndex, ErrorText)==false)
    {
        MessageDlg(ErrorText, mtWarning, TMsgDlgButtons() << mbOK, 0);
        RefreshMotorParameterGrid();
        return;
    }

    bMotorParameterDirty=true;
    SetMessage("Motor parameter modified, save required");
}
//---------------------------------------------------------------------------
void TfMotorTest::SaveMotorParameterToFile()
{
    TStringList *LineList;
    TStringList *HeaderList;
    TStringList *RowFields;
    TStringList *ChangeList;
    TTrayMotor *Motor;
    AnsiString ErrorText;
    AnsiString BackupPath;
    AnsiString DisplayText;
    AnsiString CsvText;
    AnsiString PreviewText;
    AnsiString LogError;
    AnsiString RowLabel;
    int MotorIndex;
    int MotorCount;
    int RowIndex;
    int TableRow;
    int ChangeCount;
    int PreviewLimit;
    bool bSuccess;
    bool bLogOK;

    if(bMotorParameterDirty==false)
    {
        SetMessage("No motor parameter changes");
        return;
    }
    if(CheckNoUnsavedMotorTable("saving motor parameter")==false)
        return;
    if(CheckMotorDataEditIdle()==false)
        return;

    // 172-style batch save: validate every motor row first, then write all
    // changed rows to Mot_Table.csv in one pass (no per-motor save lock).
    MotorCount=GetMotorCount();
    for(MotorIndex=0; MotorIndex<MotorCount; MotorIndex++)
    {
        if(GetMotor(MotorIndex)==NULL)
            continue;
        if(ValidateMotorParameterRow(MotorIndex+1, ErrorText)==false)
        {
            MessageDlg(ErrorText, mtWarning, TMsgDlgButtons() << mbOK, 0);
            return;
        }
    }

    LineList=new TStringList;
    HeaderList=new TStringList;
    RowFields=new TStringList;
    ChangeList=new TStringList;
    bSuccess=false;
    try
    {
        LineList->LoadFromFile(HSys.MotTablePath);
        if(LineList->Count<=1)
            throw Exception("Mot_Table.csv has no motor data.");

        HeaderList->CommaText=LineList->Strings[0];

        for(MotorIndex=0; MotorIndex<MotorCount; MotorIndex++)
        {
            Motor=GetMotor(MotorIndex);
            if(Motor==NULL)
                continue;
            RowIndex=MotorIndex+1;
            TableRow=FindMotorTableRow(LineList, Motor, HeaderList);
            if(TableRow<=0)
                throw Exception(AnsiString("Motor row not found in Mot_Table.csv: ")+Motor->Alias);

            RowFields->CommaText=LineList->Strings[TableRow];
            while(RowFields->Count<HeaderList->Count)
                RowFields->Add("");
            RowLabel=AnsiString("SAVE_PARAM Row ")+IntToStr(TableRow+1)+AnsiString(" ")+Motor->Alias;

            for(int ColIndex=0; ColIndex<grdMotorParameter->ColCount; ColIndex++)
            {
                int CsvCol;
                AnsiString OldText;
                AnsiString Header;
                if(IsMotorParameterEditableColumn(ColIndex)==false)
                    continue;
                if(ColIndex==eMotorParamColAddress)
                {
                    // Address has no single CSV column: split the composite back into
                    // BoardID + Port using THIS row's (possibly just-edited) Card so
                    // the divisor matches database.cpp InitialMotorObject.
                    AnsiString CardText;
                    AnsiString OldBoard;
                    AnsiString OldPort;
                    AnsiString NewBoard;
                    AnsiString NewPort;
                    int AddrValue;
                    int BoardValue;
                    int PortValue;
                    int BoardCol;
                    int PortCol;
                    if(ValidateMotorParameterValue(ColIndex, grdMotorParameter->Cells[ColIndex][RowIndex], DisplayText, CsvText, ErrorText)==false)
                        throw Exception(ErrorText);
                    AddrValue=StrToIntDef(CsvText, -1);
                    CardText=MotorTestTrim(grdMotorParameter->Cells[eMotorParamColCard][RowIndex]).UpperCase();
                    if(CardText==AnsiString("MN200") || CardText==AnsiString("SYNTEK"))
                    {
                        BoardValue=AddrValue/100;
                        PortValue=AddrValue%100;
                    }
                    else
                    {
                        BoardValue=AddrValue/10;
                        PortValue=AddrValue%10;
                        if(PortValue>7)
                            throw Exception(AnsiString("Address Port (last digit) must be 0-7 for ")+CardText+AnsiString("."));
                    }
                    BoardCol=FindCsvColumn(HeaderList, "BoardID");
                    PortCol=FindCsvColumn(HeaderList, "Port");
                    if(BoardCol<0)
                        throw Exception("Missing Mot_Table column: BoardID");
                    if(PortCol<0)
                        throw Exception("Missing Mot_Table column: Port");
                    while(RowFields->Count<=BoardCol)
                        RowFields->Add("");
                    while(RowFields->Count<=PortCol)
                        RowFields->Add("");
                    NewBoard=IntToStr(BoardValue);
                    NewPort=IntToStr(PortValue);
                    OldBoard=RowFields->Strings[BoardCol];
                    OldPort=RowFields->Strings[PortCol];
                    if(OldBoard!=NewBoard)
                        ChangeList->Add(RowLabel+AnsiString(" | BoardID: ")+OldBoard+AnsiString(" -> ")+NewBoard);
                    if(OldPort!=NewPort)
                        ChangeList->Add(RowLabel+AnsiString(" | Port: ")+OldPort+AnsiString(" -> ")+NewPort);
                    RowFields->Strings[BoardCol]=NewBoard;
                    RowFields->Strings[PortCol]=NewPort;
                    continue;
                }
                Header=GetMotorParameterCsvName(ColIndex);
                CsvCol=FindCsvColumn(HeaderList, Header);
                if(CsvCol<0)
                    throw Exception(AnsiString("Missing Mot_Table column: ")+Header);
                if(ValidateMotorParameterValue(ColIndex, grdMotorParameter->Cells[ColIndex][RowIndex], DisplayText, CsvText, ErrorText)==false)
                    throw Exception(ErrorText);
                while(RowFields->Count<=CsvCol)
                    RowFields->Add("");
                OldText=RowFields->Strings[CsvCol];
                if(OldText!=CsvText)
                    ChangeList->Add(RowLabel+AnsiString(" | ")+Header+AnsiString(": ")+OldText+AnsiString(" -> ")+CsvText);
                RowFields->Strings[CsvCol]=CsvText;
            }
            LineList->Strings[TableRow]=RowFields->CommaText;
        }

        ChangeCount=ChangeList->Count;
        if(ChangeCount<=0)
        {
            bMotorParameterDirty=false;
            bSuccess=true;
        }
        else
        {
            PreviewText=AnsiString("Motor parameter changes: ")+IntToStr(ChangeCount);
            PreviewLimit=(ChangeCount<12)?ChangeCount:12;
            for(int Index=0; Index<PreviewLimit; Index++)
                PreviewText=PreviewText+AnsiString("\r\n")+ChangeList->Strings[Index];
            if(ChangeCount>PreviewLimit)
                PreviewText=PreviewText+AnsiString("\r\n...")+IntToStr(ChangeCount-PreviewLimit)+AnsiString(" more change(s)");
            if(MessageDlg(PreviewText+AnsiString("\r\n\r\nSave motor parameter changes?"), mtConfirmation, TMsgDlgButtons() << mbYes << mbNo, 0)!=mrYes)
                throw Exception("User cancelled motor parameter save.");

            if(BackupMotorTable(BackupPath, ErrorText)==false)
                throw Exception(AnsiString("Backup failed: ")+ErrorText);

            LineList->SaveToFile(HSys.MotTablePath);
            bSuccess=true;
        }
    }
    catch(Exception &E)
    {
        ErrorText=E.Message;
    }
    catch(...)
    {
        ErrorText="Unknown save error.";
    }

    delete RowFields;
    delete HeaderList;
    delete LineList;

    if(bSuccess==false)
    {
        delete ChangeList;
        if(ErrorText==AnsiString("User cancelled motor parameter save."))
        {
            SetMessage("Motor parameter save cancelled");
            return;
        }
        MessageDlg(ErrorText, mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Motor parameter save failed");
        return;
    }

    if(ChangeList->Count<=0)
    {
        delete ChangeList;
        SetMessage("No effective motor parameter changes");
        return;
    }

    bLogOK=AppendMotorTableSaveLog(BackupPath, ChangeList, LogError);
    delete ChangeList;

    for(MotorIndex=0; MotorIndex<MotorCount; MotorIndex++)
        if(GetMotor(MotorIndex)!=NULL)
            HSys.LoadSingleMotorParameterFromDataBase(MotorIndex, false);
    bMotorParameterDirty=false;
    FillMotorList();
    RefreshAllGrids();
    UpdateMotorMonitor();
    if(bLogOK==false)
    {
        MessageDlg(AnsiString("Motor parameter saved, but log failed: ")+LogError, mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Motor parameter saved. Log failed");
    }
    else
        SetMessage("Motor parameter saved");
}
//---------------------------------------------------------------------------
void TfMotorTest::ReloadActiveMotorParameter()
{
    if(CheckMotorDataEditIdle()==false)
        return;
    if(CheckNoUnsavedMotorTable("reloading motor parameter")==false)
        return;
    if(ConfirmDiscardMotorParameterEdit()==false)
        return;
    if(GetActiveMotor()!=NULL)
        HSys.LoadSingleMotorParameterFromDataBase(ActiveMotorIndex, false);
    FillMotorList();
    RefreshAllGrids();
    UpdateMotorMonitor();
    SetMessage("Motor parameter reloaded");
}
//---------------------------------------------------------------------------
void TfMotorTest::LoadLoopPositions()
{
    TIniFile *Ini;
    TTrayMotor *Motor;
    AnsiString FileName;
    int MotorIndex;

    FileName=GetMotorTestIniFileName();
    if(FileExists(FileName)==false)
    {
        UpdateActivePositionEdits();
        return;
    }

    Ini=new TIniFile(FileName);
    try
    {
        for(MotorIndex=0; MotorIndex<GetMotorCount(); MotorIndex++)
        {
            Motor=GetMotor(MotorIndex);
            if(Motor==NULL)
                continue;
            Pos1[MotorIndex]=Ini->ReadInteger(Motor->Alias, "Pos1", 0);
            Pos2[MotorIndex]=Ini->ReadInteger(Motor->Alias, "Pos2", 0);
        }
    }
    __finally
    {
        delete Ini;
    }
    UpdateActivePositionEdits();
}
//---------------------------------------------------------------------------
void TfMotorTest::SaveLoopPositions()
{
    TIniFile *Ini;
    TTrayMotor *Motor;
    AnsiString FileName;
    int MotorIndex;

    if(ActiveMotorIndex>=0 && ActiveMotorIndex<GetMotorCount())
    {
        Pos1[ActiveMotorIndex]=ParsePositionText(edPos1->Text);
        Pos2[ActiveMotorIndex]=ParsePositionText(edPos2->Text);
    }

    FileName=GetMotorTestIniFileName();
    ForceDirectories(ExtractFilePath(FileName));
    Ini=new TIniFile(FileName);
    try
    {
        for(MotorIndex=0; MotorIndex<GetMotorCount(); MotorIndex++)
        {
            Motor=GetMotor(MotorIndex);
            if(Motor==NULL)
                continue;
            Ini->WriteInteger(Motor->Alias, "Pos1", Pos1[MotorIndex]);
            Ini->WriteInteger(Motor->Alias, "Pos2", Pos2[MotorIndex]);
        }
    }
    __finally
    {
        delete Ini;
    }
}
//---------------------------------------------------------------------------
void TfMotorTest::RefreshOperateGrid()
{
    TTrayMotor *Motor;
    int MotorIndex;
    int RowIndex;
    int MotorCount;
    AnsiString SoftLimit;

    if(grdOperate==NULL)
        return;
    MotorCount=GetMotorCount();
    grdOperate->RowCount=(MotorCount>0)?MotorCount+1:2;
    for(RowIndex=1; RowIndex<grdOperate->RowCount; RowIndex++)
        for(int ColIndex=0; ColIndex<grdOperate->ColCount; ColIndex++)
            grdOperate->Cells[ColIndex][RowIndex]="";

    for(MotorIndex=0; MotorIndex<MotorCount; MotorIndex++)
    {
        Motor=GetMotor(MotorIndex);
        if(Motor==NULL)
            continue;
        Motor->ScanMotorStatus();
        RowIndex=MotorIndex+1;
        SoftLimit=FormatPositionText(Motor->GetSoftLimitN())+AnsiString(" ~ ")+FormatPositionText(Motor->GetSoftLimitP());
        grdOperate->Cells[eOperateColNo][RowIndex]=Motor->Number;
        grdOperate->Cells[eOperateColMotor][RowIndex]=Motor->Alias;
        grdOperate->Cells[eOperateColCommand][RowIndex]=FormatPositionText(Motor->ReadPos());
        grdOperate->Cells[eOperateColEncoder][RowIndex]=FormatPositionText(Motor->ReadEncoderPos());
        grdOperate->Cells[eOperateColHome][RowIndex]=BoolText(Motor->bHomeFlag);
        grdOperate->Cells[eOperateColEnable][RowIndex]=BoolText(Motor->GetEnable());
        grdOperate->Cells[eOperateColServoOn][RowIndex]=BoolText(Motor->Led[iServoOn]);
        grdOperate->Cells[eOperateColServoAlarm][RowIndex]=BoolText(Motor->Led[iServoalarmLed]);
        grdOperate->Cells[eOperateColEmg][RowIndex]=BoolText(Motor->Led[iEmgLed]);
        grdOperate->Cells[eOperateColAlarm][RowIndex]=BoolText(Motor->Led[iAlarmLed]);
        grdOperate->Cells[eOperateColInPos][RowIndex]=BoolText(Motor->Led[iInposLed]);
        grdOperate->Cells[eOperateColSoftLimit][RowIndex]=SoftLimit;
    }
}
//---------------------------------------------------------------------------
void TfMotorTest::RefreshMotorParameterGrid()
{
    TTrayMotor *Motor;
    int MotorIndex;
    int RowIndex;
    int MotorCount;

    if(grdMotorParameter==NULL)
        return;
    MotorCount=GetMotorCount();
    grdMotorParameter->RowCount=(MotorCount>0)?MotorCount+1:2;
    for(RowIndex=1; RowIndex<grdMotorParameter->RowCount; RowIndex++)
        for(int ColIndex=0; ColIndex<grdMotorParameter->ColCount; ColIndex++)
            grdMotorParameter->Cells[ColIndex][RowIndex]="";

    for(MotorIndex=0; MotorIndex<MotorCount; MotorIndex++)
    {
        Motor=GetMotor(MotorIndex);
        if(Motor==NULL)
            continue;
        RowIndex=MotorIndex+1;
        grdMotorParameter->Cells[eMotorParamColNo][RowIndex]=Motor->Number;
        grdMotorParameter->Cells[eMotorParamColAlias][RowIndex]=Motor->Alias;
        grdMotorParameter->Cells[eMotorParamColCard][RowIndex]=Motor->CardModel;
        grdMotorParameter->Cells[eMotorParamColAddress][RowIndex]=IntToStr(Motor->GetAddress());
        grdMotorParameter->Cells[eMotorParamColEnable][RowIndex]=BoolText(Motor->GetEnable());
        grdMotorParameter->Cells[eMotorParamColHome][RowIndex]=BoolText(Motor->bHomeFlag);
        grdMotorParameter->Cells[eMotorParamColSoftN][RowIndex]=FormatPositionText(Motor->GetSoftLimitN());
        grdMotorParameter->Cells[eMotorParamColSoftP][RowIndex]=FormatPositionText(Motor->GetSoftLimitP());
        grdMotorParameter->Cells[eMotorParamColInit][RowIndex]=IntToStr(Motor->GetInitSpeed());
        grdMotorParameter->Cells[eMotorParamColJogLow][RowIndex]=IntToStr(Motor->GetJogLowSpeed());
        grdMotorParameter->Cells[eMotorParamColJogHigh][RowIndex]=IntToStr(Motor->GetJogHighSpeed());
        grdMotorParameter->Cells[eMotorParamColHomeLow][RowIndex]=IntToStr(Motor->GetHomeLowSpeed());
        grdMotorParameter->Cells[eMotorParamColHomeHigh][RowIndex]=IntToStr(Motor->GetHomeHighSpeed());
        grdMotorParameter->Cells[eMotorParamColAcc][RowIndex]=MotorTestFormatDouble(Motor->GetAcc());
        grdMotorParameter->Cells[eMotorParamColDec][RowIndex]=MotorTestFormatDouble(Motor->GetDec());
        grdMotorParameter->Cells[eMotorParamColRange][RowIndex]=IntToStr(Motor->GetRange());
    }
}
//---------------------------------------------------------------------------
void TfMotorTest::RefreshInformationGrid()
{
    TTrayMotor *Motor;
    TMOTDATA *Data;
    int MotorIndex;
    int RowIndex;
    int MotorCount;

    if(grdInformation==NULL)
        return;
    MotorCount=GetMotorCount();
    grdInformation->RowCount=(MotorCount>0)?MotorCount+1:2;
    for(RowIndex=1; RowIndex<grdInformation->RowCount; RowIndex++)
        for(int ColIndex=0; ColIndex<grdInformation->ColCount; ColIndex++)
            grdInformation->Cells[ColIndex][RowIndex]="";

    for(MotorIndex=0; MotorIndex<MotorCount; MotorIndex++)
    {
        Motor=GetMotor(MotorIndex);
        if(Motor==NULL)
            continue;
        Data=HSys.FindMotData(Motor->Alias);
        RowIndex=MotorIndex+1;
        grdInformation->Cells[eInfoColNo][RowIndex]=Motor->Number;
        grdInformation->Cells[eInfoColAlias][RowIndex]=Motor->Alias;
        grdInformation->Cells[eInfoColCard][RowIndex]=Motor->CardModel;
        grdInformation->Cells[eInfoColBoard][RowIndex]=(Data==NULL)?AnsiString(""):IntToStr(Data->iBoardID);
        grdInformation->Cells[eInfoColPort][RowIndex]=(Data==NULL)?AnsiString(""):IntToStr(Data->iPort);
        grdInformation->Cells[eInfoColAddress][RowIndex]=IntToStr(Motor->GetAddress());
        grdInformation->Cells[eInfoColKind][RowIndex]=IntToStr((int)Motor->GetMotorKind());
        grdInformation->Cells[eInfoColFlushPanel][RowIndex]=(Data==NULL)?Motor->FlushPanelName:Data->FlushPanel;
        grdInformation->Cells[eInfoColServoConfig][RowIndex]=BoolText(Motor->ReadServoAlarmOn());
        grdInformation->Cells[eInfoColIsServo][RowIndex]=BoolText(Motor->bIsServoMotor);
        grdInformation->Cells[eInfoColMotorRelay][RowIndex]=BoolText(HSys.Sw.SwMotorRelay.Status());
        grdInformation->Cells[eInfoColServerON][RowIndex]=BoolText(HSys.Sw.SwServerON.Status());
        grdInformation->Cells[eInfoColMotorPower][RowIndex]=HSys.Sen.SnMotorPower.Enable?BoolText(HSys.Sen.SnMotorPower.IsOn()):AnsiString("DISABLE");
        grdInformation->Cells[eInfoColPowerState][RowIndex]=BoolText(bMotorPowerState);
        grdInformation->Cells[eInfoColPowerDelay][RowIndex]=IntToStr(MotorPowerOnDelay);
    }
}
//---------------------------------------------------------------------------
bool TfMotorTest::ParseRegisterOffset(AnsiString Text, DWORD &Offset)
{
    char *EndPtr;
    unsigned long Value;
    AnsiString Work;

    Offset=0;
    Work=MotorTestTrim(Text).UpperCase();
    if(Work.SubString(1, 2)==AnsiString("0X"))
        Work.Delete(1, 2);
    if(Work==AnsiString(""))
        return false;

    EndPtr=NULL;
    Value=strtoul(Work.c_str(), &EndPtr, 16);
    if(EndPtr==NULL || *EndPtr!='\0')
        return false;
    if(Value>0xFFFF || (Value & 0x01)!=0)
        return false;

    Offset=(DWORD)Value;
    return true;
}
//---------------------------------------------------------------------------
void TfMotorTest::ReadDriverRegister(bool bDefaultList)
{
    TTrayMotor *Motor;
    DWORD Offset;
    WORD Data;
    int StartRow;
    int EndRow;
    bool bAnyRead;

    Motor=GetActiveMotor();
    if(Motor==NULL || grdDriverRegister==NULL)
    {
        SetMessage("Register read abort: no motor");
        return;
    }
    if(CheckMotorDataEditIdle()==false)
        return;
    if(Motor->GetEnable()==false)
    {
        MessageDlg("Active motor is disabled.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Register read abort: motor disabled");
        return;
    }
    if(Motor->CardModel.UpperCase()!=AnsiString("MC88X1"))
    {
        MessageDlg("Register read is enabled only for MC88X1 motors.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Register read abort: not MC88X1");
        return;
    }

    if(bDefaultList)
    {
        StartRow=1;
        EndRow=grdDriverRegister->RowCount-1;
    }
    else
    {
        StartRow=1;
        EndRow=1;
        if(edRegisterOffset==NULL || ParseRegisterOffset(edRegisterOffset->Text, Offset)==false)
        {
            MessageDlg("Input an even hex register offset, for example 0x08.", mtWarning, TMsgDlgButtons() << mbOK, 0);
            SetMessage("Register read abort: invalid offset");
            return;
        }
        if(grdDriverRegister->RowCount<2)
            grdDriverRegister->RowCount=2;
        grdDriverRegister->Cells[eDriverRegColRegister][1]=edRegisterOffset->Text;
    }

    bAnyRead=false;
    for(int RowIndex=StartRow; RowIndex<=EndRow; RowIndex++)
    {
        if(ParseRegisterOffset(grdDriverRegister->Cells[eDriverRegColRegister][RowIndex], Offset)==false)
        {
            grdDriverRegister->Cells[eDriverRegColValue][RowIndex]="";
            grdDriverRegister->Cells[eDriverRegColResult][RowIndex]="Invalid even hex offset";
            continue;
        }
        Data=0;
        if(Motor->ReadStatus(Offset, &Data))
        {
            AnsiString ValueText;
            ValueText.sprintf("0x%04X / %u", (unsigned int)Data, (unsigned int)Data);
            grdDriverRegister->Cells[eDriverRegColValue][RowIndex]=ValueText;
            grdDriverRegister->Cells[eDriverRegColResult][RowIndex]=AnsiString("OK ")+Motor->NumberAlias;
            bAnyRead=true;
        }
        else
        {
            grdDriverRegister->Cells[eDriverRegColValue][RowIndex]="";
            grdDriverRegister->Cells[eDriverRegColResult][RowIndex]=AnsiString("Read failed ")+Motor->NumberAlias;
        }
    }

    if(bAnyRead)
        SetMessage("MC88X1 register read complete");
    else
        SetMessage("MC88X1 register read produced no data");
}
//---------------------------------------------------------------------------
AnsiString TfMotorTest::GetServoGuardLogFileName()
{
    AnsiString RootPath=HSys.CurrentDir;

    if(RootPath==AnsiString(""))
        RootPath="..";
    return RootPath+AnsiString("\\logs\\servo_power_guard.log");
}
//---------------------------------------------------------------------------
bool TfMotorTest::AppendServoGuardLog(TStringList *GuardList, AnsiString &ErrorText)
{
    TStringList *LogList;
    AnsiString FileName;

    ErrorText="";
    if(GuardList==NULL)
    {
        ErrorText="No guard list.";
        return false;
    }

    FileName=GetServoGuardLogFileName();
    LogList=new TStringList;
    try
    {
        ForceDirectories(ExtractFilePath(FileName));
        if(FileExists(FileName))
            LogList->LoadFromFile(FileName);
        LogList->Add("------------------------------------------------------------");
        LogList->Add(AnsiString("Time=")+FormatDateTime("yyyy/mm/dd hh:nn:ss", Now()));
        for(int Index=0; Index<GuardList->Count; Index++)
            LogList->Add(GuardList->Strings[Index]);
        LogList->SaveToFile(FileName);
    }
    catch(Exception &E)
    {
        ErrorText=E.Message;
    }
    catch(...)
    {
        ErrorText="Unknown Servo Guard log error.";
    }
    delete LogList;

    return ErrorText==AnsiString("");
}
//---------------------------------------------------------------------------
void TfMotorTest::SetServoGuardRow(int RowIndex, AnsiString Item, AnsiString Value, AnsiString Note)
{
    if(grdServoGuard==NULL || RowIndex<0)
        return;
    if(grdServoGuard->RowCount<=RowIndex)
        grdServoGuard->RowCount=RowIndex+1;
    grdServoGuard->Cells[eServoGuardColItem][RowIndex]=Item;
    grdServoGuard->Cells[eServoGuardColValue][RowIndex]=Value;
    grdServoGuard->Cells[eServoGuardColNote][RowIndex]=Note;
}
//---------------------------------------------------------------------------
bool TfMotorTest::ExecuteServoPowerGuard(bool bServoOn, bool bApplyMode)
{
    TStringList *GuardList;
    AnsiString ActionText;
    AnsiString ModeText;
    AnsiString ConfirmText;
    AnsiString ReasonText;
    AnsiString LogError;
    int ServoCount;
    int EnabledServoCount;
    int ServoAlarmCount;
    int ActuatedCount;
    int EmgState;
    int SafeDoorState;
    bool GuardOK;
    bool bLogOK;

    ModeText=bApplyMode?AnsiString("APPLY"):AnsiString("GUARD");
    ActionText=bServoOn?AnsiString("Servo On"):AnsiString("Servo Off");
    if(bApplyMode)
        ConfirmText=ActionText+AnsiString(" APPLY will change SwServerON and servo output. Continue?");
    else
        ConfirmText=ActionText+AnsiString(" guard check only?\r\nNo SwServerON, ServoOnOff, or motor relay output will be changed.");
    if(MessageDlg(ConfirmText, mtConfirmation, TMsgDlgButtons() << mbYes << mbNo, 0)!=mrYes)
    {
        SetMessage(bApplyMode?AnsiString("Servo apply cancelled"):AnsiString("Servo guard cancelled"));
        return false;
    }

    GuardOK=true;
    ReasonText="";
    ServoCount=0;
    EnabledServoCount=0;
    ServoAlarmCount=0;
    ActuatedCount=0;
    EmgState=IsEMGPressed();
    SafeDoorState=IsSafeDoorOpen();
    bLogOK=false;

    if(bMotorParameterDirty)
    {
        GuardOK=false;
        ReasonText=ReasonText+AnsiString("Motor parameter not saved; ");
    }
    if(bMotorTableDirty)
    {
        GuardOK=false;
        ReasonText=ReasonText+AnsiString("Mot_Table not saved; ");
    }
    if(HSys.Sys.SystemStart)
    {
        GuardOK=false;
        ReasonText=ReasonText+AnsiString("Machine running; ");
    }
    if(bHomeRunning || bLoopRunning)
    {
        GuardOK=false;
        ReasonText=ReasonText+AnsiString("MotorTest busy; ");
    }
    if(EmgState>0)
    {
        GuardOK=false;
        ReasonText=ReasonText+AnsiString("EMG pressed; ");
    }
    if(SafeDoorState>0)
    {
        GuardOK=false;
        ReasonText=ReasonText+AnsiString("Safe door open; ");
    }

    for(int MotorIndex=0; MotorIndex<GetMotorCount(); MotorIndex++)
    {
        TTrayMotor *Motor=GetMotor(MotorIndex);
        if(Motor==NULL)
            continue;
        Motor->ScanMotorStatus();
        if(Motor->ReadServoAlarmOn())
        {
            ServoCount++;
            if(Motor->GetEnable())
                EnabledServoCount++;
            // A triggered over-travel limit forces iAlarmLed true (see MC88X1
            // ScanMotorStatus) but is NOT a servo-drive alarm, so it must not block
            // Servo On. Count a servo alarm only when ALARM is on for a reason OTHER
            // than a CW/CCW limit, or on a real servo-alarm / EMG signal.
            if((Motor->Led[iAlarmLed] && !(Motor->Led[iCwLed] || Motor->Led[iCcwLed]))
               || Motor->Led[iServoalarmLed] || Motor->Led[iEmgLed])
                ServoAlarmCount++;
        }
    }
    if(EnabledServoCount<=0)
    {
        GuardOK=false;
        ReasonText=ReasonText+AnsiString("No enabled servo motor; ");
    }
    if(bServoOn)
    {
        if(HSys.Sw.SwMotorRelay.Status()==false)
        {
            GuardOK=false;
            ReasonText=ReasonText+AnsiString("Motor relay off; ");
        }
        if(bMotorPowerState==false)
        {
            GuardOK=false;
            ReasonText=ReasonText+AnsiString("Motor power state off; ");
        }
        if(MotorPowerOnDelay>0)
        {
            GuardOK=false;
            ReasonText=ReasonText+AnsiString("Motor power delay active; ");
        }
        if(ServoAlarmCount>0)
        {
            GuardOK=false;
            ReasonText=ReasonText+AnsiString("Servo motor alarm exists; ");
        }
    }
    if(ReasonText==AnsiString(""))
        ReasonText="OK";

    if(GuardOK && bApplyMode)
    {
        if(bServoOn)
            HSys.Sw.SwServerON.On();
        else
        {
            StopActiveMotor();
            AllBreakLock();
            HSys.Sw.SwServerON.Off();
        }
        for(int MotorIndex=0; MotorIndex<GetMotorCount(); MotorIndex++)
        {
            TTrayMotor *Motor=GetMotor(MotorIndex);
            if(Motor==NULL)
                continue;
            if(Motor->ReadServoAlarmOn())
            {
                Motor->ServoOnOff(bServoOn);
                ActuatedCount++;
            }
        }
        RefreshInformationGrid();
        UpdateMotorMonitor();
    }

    SetServoGuardRow(1, "Action", ActionText, bApplyMode?AnsiString("Guarded live apply"):AnsiString("Dry-run guard only"));
    SetServoGuardRow(2, "Result", GuardOK?AnsiString("PASS"):AnsiString("BLOCKED"), ReasonText);
    SetServoGuardRow(3, "SystemStart", BoolText(HSys.Sys.SystemStart), "Must be OFF");
    SetServoGuardRow(4, "Home/Loop Busy", BoolText(bHomeRunning || bLoopRunning), "Must be OFF");
    SetServoGuardRow(5, "EMG", IntToStr(EmgState), "Must be 0");
    SetServoGuardRow(6, "SafeDoor", IntToStr(SafeDoorState), "Must be 0");
    SetServoGuardRow(7, "MotorRelay", BoolText(HSys.Sw.SwMotorRelay.Status()), bServoOn?AnsiString("Servo On needs motor relay ON"):AnsiString("Read only"));
    SetServoGuardRow(8, "ServerON", BoolText(HSys.Sw.SwServerON.Status()), bApplyMode?AnsiString("Current output state"):AnsiString("Read only, no output changed"));
    SetServoGuardRow(9, "SnMotorPower", HSys.Sen.SnMotorPower.Enable?BoolText(HSys.Sen.SnMotorPower.IsOn()):AnsiString("DISABLE"), "Read only");
    SetServoGuardRow(10, "PowerState", BoolText(bMotorPowerState), "Read only");
    SetServoGuardRow(11, "PowerDelay", IntToStr(MotorPowerOnDelay), "Must be 0 for Servo On");
    SetServoGuardRow(12, "Servo Motors", IntToStr(ServoCount), "ReadServoAlarmOn=true count");
    SetServoGuardRow(13, "Enabled Servo", IntToStr(EnabledServoCount), "Enabled servo motor count");
    SetServoGuardRow(14, "Servo Alarms", IntToStr(ServoAlarmCount), "Blocks Servo On apply");
    SetServoGuardRow(15, "Mode", ModeText, bApplyMode?AnsiString("Live output mode"):AnsiString("Report only"));
    SetServoGuardRow(16, "Actuation", (GuardOK && bApplyMode)?AnsiString("EXECUTED"):AnsiString("LOCKED"), IntToStr(ActuatedCount)+AnsiString(" servo command(s)"));
    SetServoGuardRow(17, "ServerON After", BoolText(HSys.Sw.SwServerON.Status()), "Read after guard/apply");

    GuardList=new TStringList;
    try
    {
        GuardList->Add(AnsiString("Action=")+ActionText);
        GuardList->Add(AnsiString("Mode=")+ModeText);
        GuardList->Add(AnsiString("Result=")+(GuardOK?AnsiString("PASS"):AnsiString("BLOCKED")));
        GuardList->Add(AnsiString("Reason=")+ReasonText);
        GuardList->Add(AnsiString("SystemStart=")+BoolText(HSys.Sys.SystemStart));
        GuardList->Add(AnsiString("HomeLoopBusy=")+BoolText(bHomeRunning || bLoopRunning));
        GuardList->Add(AnsiString("EMG=")+IntToStr(EmgState));
        GuardList->Add(AnsiString("SafeDoor=")+IntToStr(SafeDoorState));
        GuardList->Add(AnsiString("MotorRelay=")+BoolText(HSys.Sw.SwMotorRelay.Status()));
        GuardList->Add(AnsiString("ServerON=")+BoolText(HSys.Sw.SwServerON.Status()));
        GuardList->Add(AnsiString("SnMotorPower=")+(HSys.Sen.SnMotorPower.Enable?BoolText(HSys.Sen.SnMotorPower.IsOn()):AnsiString("DISABLE")));
        GuardList->Add(AnsiString("PowerState=")+BoolText(bMotorPowerState));
        GuardList->Add(AnsiString("PowerDelay=")+IntToStr(MotorPowerOnDelay));
        GuardList->Add(AnsiString("ServoMotors=")+IntToStr(ServoCount));
        GuardList->Add(AnsiString("EnabledServoMotors=")+IntToStr(EnabledServoCount));
        GuardList->Add(AnsiString("ServoAlarmMotors=")+IntToStr(ServoAlarmCount));
        GuardList->Add(AnsiString("Actuation=")+((GuardOK && bApplyMode)?AnsiString("EXECUTED"):AnsiString("LOCKED")));
        GuardList->Add(AnsiString("ActuatedServoMotors=")+IntToStr(ActuatedCount));
        GuardList->Add(AnsiString("ServerONAfter=")+BoolText(HSys.Sw.SwServerON.Status()));
        bLogOK=AppendServoGuardLog(GuardList, LogError);
    }
    __finally
    {
        delete GuardList;
    }

    if(bLogOK==false)
        MessageDlg(AnsiString("Servo Guard log failed: ")+LogError, mtWarning, TMsgDlgButtons() << mbOK, 0);
    if(GuardOK)
    {
        if(bApplyMode)
        {
            MessageDlg(AnsiString("Servo apply executed. Servo command count: ")+IntToStr(ActuatedCount), mtInformation, TMsgDlgButtons() << mbOK, 0);
            SetMessage("Servo apply executed");
        }
        else
        {
            MessageDlg("Servo guard passed. Actual servo control is still locked.", mtInformation, TMsgDlgButtons() << mbOK, 0);
            SetMessage("Servo guard passed, actuation locked");
        }
    }
    else
    {
        MessageDlg(AnsiString("Servo ")+(bApplyMode?AnsiString("apply"):AnsiString("guard"))+AnsiString(" blocked: ")+ReasonText, mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage(bApplyMode?AnsiString("Servo apply blocked"):AnsiString("Servo guard blocked, actuation locked"));
    }
    return GuardOK;
}
//---------------------------------------------------------------------------
void TfMotorTest::RunServoPowerGuard(bool bServoOn)
{
    ExecuteServoPowerGuard(bServoOn, false);
}
//---------------------------------------------------------------------------
void TfMotorTest::ApplyServoPower(bool bServoOn)
{
    ExecuteServoPowerGuard(bServoOn, true);
}
//---------------------------------------------------------------------------
void TfMotorTest::RefreshAllGrids()
{
    RefreshOperateGrid();
    RefreshMotorParameterGrid();
    RefreshInformationGrid();
}
//---------------------------------------------------------------------------
void TfMotorTest::UpdateMotorMonitor()
{
    TTrayMotor *Motor;
    int NowPos;
    int EncoderPos;
    bool bSysEmg;

    Motor=GetActiveMotor();
    if(Motor==NULL)
    {
        if(palMotorName!=NULL)
            palMotorName->Caption="";
        if(edNowPos!=NULL)
            edNowPos->Text="";
        if(edEncoder!=NULL)
            edEncoder->Text="";
        return;
    }

    Motor->ScanMotorStatus();
    NowPos=Motor->ReadPos();
    EncoderPos=Motor->ReadEncoderPos();

    if(palMotorName!=NULL)
        palMotorName->Caption=Motor->NumberAlias;
    if(edNowPos!=NULL)
        edNowPos->Text=FormatPositionText(NowPos);
    if(edEncoder!=NULL)
        edEncoder->Text=FormatPositionText(EncoderPos);

    //AI(general) 20260616 : the per-axis EMG led bit never reacts on this machine
    //because EMG is wired to a system DI read by IsEMGPressed(), not to each motor
    //card's EMG input pin. Reflect the real system EMG on the panel led.
    bSysEmg=(IsEMGPressed()>0);
    for(int LedIndex=0; LedIndex<iMotLedTotalCnt; LedIndex++)
    {
        if(LedIndex==iEmgLed)
            UpdateStatusLed(LedIndex, Motor->Led[LedIndex] || bSysEmg);
        else
            UpdateStatusLed(LedIndex, Motor->Led[LedIndex]);
    }
}
//---------------------------------------------------------------------------
void TfMotorTest::UpdateStatusLed(int LedIndex, bool Value)
{
    if(LedIndex<0 || LedIndex>=iMotLedTotalCnt || ledStatus[LedIndex]==NULL)
        return;
    ledStatus[LedIndex]->Value=Value;
}
//---------------------------------------------------------------------------
void TfMotorTest::UpdateActivePositionEdits()
{
    if(ActiveMotorIndex<0 || ActiveMotorIndex>=GetMotorCount())
        return;
    if(edPos1!=NULL)
        edPos1->Text=FormatPositionText(Pos1[ActiveMotorIndex]);
    if(edPos2!=NULL)
        edPos2->Text=FormatPositionText(Pos2[ActiveMotorIndex]);
}
//---------------------------------------------------------------------------
void TfMotorTest::UpdateSpeedScrollFromEdit()
{
    int Percent;

    if(scrSpeedPercent==NULL || edSpeedPercent==NULL)
        return;
    Percent=GetEditInt(edSpeedPercent, scrSpeedPercent->Position);
    if(Percent<1)
        Percent=1;
    if(Percent>100)
        Percent=100;
    edSpeedPercent->Text=IntToStr(Percent);
    scrSpeedPercent->Position=Percent;
}
//---------------------------------------------------------------------------
void TfMotorTest::ResetLoopStatisticLabels()
{
    if(lblLoopTripValue!=NULL)
        lblLoopTripValue->Caption="0.000 sec";
    if(lblLoopAverageValue!=NULL)
        lblLoopAverageValue->Caption="0.000 sec";
    if(lblLoopTotalValue!=NULL)
        lblLoopTotalValue->Caption="0";
}
//---------------------------------------------------------------------------
void TfMotorTest::UpdateLoopStatisticLabels(DWORD TripMS, DWORD AverageMS)
{
    if(lblLoopTripValue!=NULL)
        lblLoopTripValue->Caption=MotorTestFormatMS(TripMS);
    if(lblLoopAverageValue!=NULL)
        lblLoopAverageValue->Caption=MotorTestFormatMS(AverageMS);
    if(lblLoopTotalValue!=NULL)
        lblLoopTotalValue->Caption=IntToStr(iLoopFinishedCount);
}
//---------------------------------------------------------------------------
void TfMotorTest::SetMessage(AnsiString Text)
{
    if(palMessage!=NULL)
        palMessage->Caption=Text;
}
//---------------------------------------------------------------------------
bool TfMotorTest::CheckSortArmZHome()
{
    // SortArm X/Y may move only when every suck-nozzle Z is physically sitting on
    // its Home sensor (sensor LED lit RIGHT NOW), not merely "has homed before"
    // (bHomeFlag). Read the live Home sensor each call. Disabled nozzles are skipped
    // (no meaningful sensor), matching the old bHomeFlag behaviour for !Enable.
    TTrayMotor *SuckZ[4];
    int Index;

    SuckZ[0]=HSys.Mot.MSuckZ_1;
    SuckZ[1]=HSys.Mot.MSuckZ_2;
    SuckZ[2]=HSys.Mot.MSuckZ_3;
    SuckZ[3]=HSys.Mot.MSuckZ_4;
    for(Index=0; Index<4; Index++)
    {
        if(SuckZ[Index]==NULL || SuckZ[Index]->GetEnable()==false)
            continue;
        SuckZ[Index]->ScanMotorStatus();
        if(SuckZ[Index]->Led[iHomeLed]==false)
            return false;
    }
    return true;
}
//---------------------------------------------------------------------------
bool TfMotorTest::CheckCanMotorMove(TTrayMotor *Motor, bool bRequireHome, bool bUseTarget, int Target)
{
    int Emg;

    if(Motor==NULL)
    {
        SetMessage("Move abort: no motor");
        return false;
    }
    if(bMotorParameterDirty)
    {
        MessageDlg("Save or reload motor parameter first.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: parameter not saved");
        return false;
    }
    if(bMotorTableDirty)
    {
        MessageDlg("Save or reload Mot_Table first.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: Mot_Table not saved");
        return false;
    }
    if(HSys.Sys.SystemStart)
    {
        MessageDlg("Machine is running.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: system start");
        return false;
    }
    // Home or loop already running: block any new move/jog/step/loop/home start
    // until it finishes (matches HT172, where spbHome->Down latches and gates the
    // other operate buttons). bHomeRunning/bLoopRunning are set AFTER the start
    // gate calls this, so the first home/loop start is not self-blocked.
    if(bHomeRunning || bLoopRunning)
    {
        MessageDlg("Motor Test is busy (home or loop running).", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: home/loop busy");
        return false;
    }
    Emg=IsEMGPressed();
    if(Emg>0)
    {
        MessageDlg("EMG is pressed.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: EMG");
        return false;
    }

    Motor->ScanMotorStatus();
    if(Motor->GetEnable()==false)
    {
        MessageDlg("Motor is disabled.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: motor disable");
        return false;
    }
    if(Motor->Led[iAlarmLed] || Motor->Led[iServoalarmLed] || Motor->Led[iEmgLed])
    {
        MessageDlg("Motor alarm is active.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: motor alarm");
        return false;
    }
    if(bRequireHome && Motor->bHomeFlag==false)
    {
        MessageDlg("Motor is not home.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: motor not home");
        return false;
    }
    if(HSys.Mot.MSortingArmX!=NULL && Motor->Tag==HSys.Mot.MSortingArmX->Tag && CheckSortArmZHome()==false)
    {
        MessageDlg("Sort arm Z must be home before X move.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: Sort Z not home");
        return false;
    }
    if(bUseTarget && Motor->CheckSoftLimit(Target)==false)
    {
        MessageDlg("Target over soft limit.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Move abort: soft limit");
        return false;
    }
    return true;
}
//---------------------------------------------------------------------------
int TfMotorTest::ApplySpeedPercent(TTrayMotor *Motor)
{
    int Percent;
    int RawSpeed;

    if(Motor==NULL)
        return 0;
    Percent=GetEditInt(edSpeedPercent, Motor->GetPersentSpeed()>0?Motor->GetPersentSpeed():20);
    if(Percent<1)
        Percent=1;
    if(Percent>100)
        Percent=100;
    edSpeedPercent->Text=IntToStr(Percent);
    if(scrSpeedPercent!=NULL)
        scrSpeedPercent->Position=Percent;
    Motor->SetPersentSpeed(Percent, false);

    RawSpeed=Motor->GetJogHighSpeed()*Percent/100;
    if(RawSpeed<Motor->GetJogLowSpeed())
        RawSpeed=Motor->GetJogLowSpeed();
    if(RawSpeed<=0)
        RawSpeed=Motor->GetInitSpeed();
    if(RawSpeed<=0)
        RawSpeed=1;
    Motor->SetSpeed(RawSpeed);
    return RawSpeed;
}
//---------------------------------------------------------------------------
void TfMotorTest::StartJog(bool bPositive)
{
    TTrayMotor *Motor;

    Motor=GetActiveMotor();
    if(Motor==NULL)
        return;
    // Jog: home/loop busy and EMG always block. For the over-travel limits, allow
    // jogging AWAY but block jogging FURTHER into a triggered limit -- a lit CW (+)
    // limit blocks Jog+ (only Jog- allowed), a lit CCW (-) limit blocks Jog- (only
    // Jog+ allowed). The limit-derived ALARM LED itself does NOT block jog, and the
    // MC88X1 card also auto-stops at the limit. Servo-drive alarm / disable / soft
    // limit are not pre-checked (JogP/JogN no-op safely when disabled and still run
    // the MC88X1 safe-door callback). Move/Step keep the full CheckCanMotorMove gate.
    if(bHomeRunning || bLoopRunning)
    {
        MessageDlg("Motor Test is busy (home or loop running).", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Jog abort: home/loop busy");
        return;
    }
    if(IsEMGPressed()>0)
    {
        MessageDlg("EMG is pressed.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Jog abort: EMG");
        return;
    }
    Motor->ScanMotorStatus();
    if(bPositive && Motor->Led[iCwLed])
    {
        MessageDlg("CW (+) limit is triggered. Jog + is blocked; use Jog - to move away.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Jog+ abort: CW limit");
        return;
    }
    if(!bPositive && Motor->Led[iCcwLed])
    {
        MessageDlg("CCW (-) limit is triggered. Jog - is blocked; use Jog + to move away.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Jog- abort: CCW limit");
        return;
    }
    ApplySpeedPercent(Motor);
    if(bPositive)
    {
        Motor->JogP();
        SetMessage("Jog +");
    }
    else
    {
        Motor->JogN();
        SetMessage("Jog -");
    }
}
//---------------------------------------------------------------------------
void TfMotorTest::StepMove(bool bPositive)
{
    TTrayMotor *Motor;
    int NowPos;
    int Step;
    int Target;

    Motor=GetActiveMotor();
    if(Motor==NULL)
        return;
    NowPos=Motor->ReadPos();
    Step=ParsePositionText(edStep->Text);
    if(Step==0)
        Step=100;
    Target=bPositive?(NowPos+Step):(NowPos-Step);
    edTarget->Text=FormatPositionText(Target);
    MoveActiveMotorToTarget();
}
//---------------------------------------------------------------------------
void TfMotorTest::MoveActiveMotorToTarget()
{
    MoveActiveMotorToPosition(ParsePositionText(edTarget->Text));
}
//---------------------------------------------------------------------------
void TfMotorTest::MoveActiveMotorToPosition(int Target)
{
    TTrayMotor *Motor;

    Motor=GetActiveMotor();
    if(Motor==NULL)
        return;
    if(CheckCanMotorMove(Motor, true, true, Target)==false)
        return;
    ApplySpeedPercent(Motor);
    edTarget->Text=FormatPositionText(Target);
    Motor->MotorMove(Target);
    SetMessage(AnsiString("Move to ")+FormatPositionText(Target));
}
//---------------------------------------------------------------------------
void TfMotorTest::MoveActiveMotorToSoftLimit(bool bPositive)
{
    TTrayMotor *Motor;
    int Target;

    Motor=GetActiveMotor();
    if(Motor==NULL)
        return;
    Target=bPositive?Motor->GetSoftLimitP():Motor->GetSoftLimitN();
    if(CheckCanMotorMove(Motor, true, true, Target)==false)
        return;
    ApplySpeedPercent(Motor);
    edTarget->Text=FormatPositionText(Target);
    Motor->MotorMove(Target);
    SetMessage(bPositive?AnsiString("Go soft +"):AnsiString("Go soft -"));
}
//---------------------------------------------------------------------------
void TfMotorTest::StopActiveMotor()
{
    TTrayMotor *Motor;

    Motor=GetActiveMotor();
    if(Motor!=NULL)
        Motor->Stop();
    // If a single-axis home was in progress, MC88X1PMotStop leaves the motion-card
    // home state machine in AxisHomeBusy, so MotionDone() stays false and Jog/Move
    // silently no-op afterwards. Re-arm the home task (InitHomeTask_forSingleAxis ->
    // HomeReset) on the homing axis so manual jog works right after Stop without
    // needing a fresh HOME press.
    if(bHomeRunning && iHomeMotorIndex>=0 && iHomeMotorIndex<GetMotorCount())
    {
        TTrayMotor *HomeMotor=GetMotor(iHomeMotorIndex);
        if(HomeMotor!=NULL)
            HomeMotor->InitHomeTask_forSingleAxis();
    }
    bHomeRunning=false;
    iHomeMotorIndex=-1;
    StopLoopMove(true);
}
//---------------------------------------------------------------------------
void TfMotorTest::SetPositionFromNow(int PosIndex)
{
    TTrayMotor *Motor;
    int NowPos;

    Motor=GetActiveMotor();
    if(Motor==NULL || ActiveMotorIndex<0 || ActiveMotorIndex>=GetMotorCount())
        return;
    NowPos=Motor->ReadPos();
    if(PosIndex==1)
    {
        Pos1[ActiveMotorIndex]=NowPos;
        edPos1->Text=FormatPositionText(NowPos);
        SetMessage("Set Pos1");
    }
    else
    {
        Pos2[ActiveMotorIndex]=NowPos;
        edPos2->Text=FormatPositionText(NowPos);
        SetMessage("Set Pos2");
    }
}
//---------------------------------------------------------------------------
void TfMotorTest::StartLoopMove()
{
    TTrayMotor *Motor;

    if(chkMultiLoop!=NULL && chkMultiLoop->Checked)
    {
        StartMultiLoopMove();
        return;
    }

    Motor=GetActiveMotor();
    if(Motor==NULL || ActiveMotorIndex<0 || ActiveMotorIndex>=GetMotorCount())
        return;

    Pos1[ActiveMotorIndex]=ParsePositionText(edPos1->Text);
    Pos2[ActiveMotorIndex]=ParsePositionText(edPos2->Text);
    iLoopRemainCount=GetEditInt(edLoopCount, 1);
    if(iLoopRemainCount<1)
        iLoopRemainCount=1;
    edLoopCount->Text=IntToStr(iLoopRemainCount);

    if(CheckCanMotorMove(Motor, true, true, Pos1[ActiveMotorIndex])==false)
        return;
    if(CheckCanMotorMove(Motor, true, true, Pos2[ActiveMotorIndex])==false)
        return;

    iLoopMotorIndex=ActiveMotorIndex;
    bLoopRunning=true;
    bLoopTargetIsPos2=false;
    bLoopWaiting=false;
    bLoopMultiMode=false;
    iLoopFinishedCount=0;
    dwLoopStartTick=GetTickCount();
    dwLoopLegStartTick=dwLoopStartTick;
    dwLoopWaitUntil=0;
    ResetLoopStatisticLabels();
    ApplySpeedPercent(Motor);
    RunNextLoopMove();
}
//---------------------------------------------------------------------------
void TfMotorTest::StartMultiLoopMove()
{
    TTrayMotor *Motor;
    int MotorIndex;
    int SelectedCount;

    if(ActiveMotorIndex>=0 && ActiveMotorIndex<GetMotorCount())
    {
        Pos1[ActiveMotorIndex]=ParsePositionText(edPos1->Text);
        Pos2[ActiveMotorIndex]=ParsePositionText(edPos2->Text);
    }

    SelectedCount=GetSelectedMultiLoopCount();
    if(SelectedCount<2)
    {
        MessageDlg("Select at least two motors in Motor List for multi loop.", mtWarning, TMsgDlgButtons() << mbOK, 0);
        SetMessage("Multi loop abort: select motors");
        return;
    }

    iLoopRemainCount=GetEditInt(edLoopCount, 1);
    if(iLoopRemainCount<1)
        iLoopRemainCount=1;
    edLoopCount->Text=IntToStr(iLoopRemainCount);

    for(MotorIndex=0; MotorIndex<MAX_MOTOR_TEST_MOTOR_COUNT; MotorIndex++)
        MultiLoopTarget[MotorIndex]=0;

    // Multi-loop members are the Loop-column checkboxes (bMultiLoopMotor), kept
    // live as the operator toggles them in grdOperate.
    for(MotorIndex=0; MotorIndex<GetMotorCount(); MotorIndex++)
    {
        if(MotorIndex>=MAX_MOTOR_TEST_MOTOR_COUNT || bMultiLoopMotor[MotorIndex]==false)
            continue;
        Motor=GetMotor(MotorIndex);
        if(CheckCanMotorMove(Motor, true, true, Pos1[MotorIndex])==false)
            return;
        if(CheckCanMotorMove(Motor, true, true, Pos2[MotorIndex])==false)
            return;
    }

    bLoopRunning=true;
    bLoopTargetIsPos2=false;
    bLoopWaiting=false;
    bLoopMultiMode=true;
    iLoopMotorIndex=-1;
    iLoopFinishedCount=0;
    dwLoopStartTick=GetTickCount();
    dwLoopLegStartTick=dwLoopStartTick;
    dwLoopWaitUntil=0;
    ResetLoopStatisticLabels();
    RunNextMultiLoopMove();
}
//---------------------------------------------------------------------------
void TfMotorTest::StopLoopMove(bool bStopMotor)
{
    TTrayMotor *Motor;

    if(bStopMotor)
    {
        if(bLoopMultiMode)
        {
            for(int MotorIndex=0; MotorIndex<GetMotorCount(); MotorIndex++)
            {
                if(bMultiLoopMotor[MotorIndex])
                {
                    Motor=GetMotor(MotorIndex);
                    if(Motor!=NULL)
                        Motor->Stop();
                }
            }
        }
        else
        {
            Motor=GetMotor(iLoopMotorIndex);
            if(Motor!=NULL)
                Motor->Stop();
        }
    }
    bLoopRunning=false;
    bLoopTargetIsPos2=false;
    bLoopWaiting=false;
    bLoopMultiMode=false;
    iLoopMotorIndex=-1;
    iLoopTarget=0;
    iLoopRemainCount=0;
    dwLoopWaitUntil=0;
}
//---------------------------------------------------------------------------
void TfMotorTest::RunNextLoopMove()
{
    TTrayMotor *Motor;

    Motor=GetMotor(iLoopMotorIndex);
    if(Motor==NULL || bLoopRunning==false)
        return;
    if(iLoopRemainCount<=0)
    {
        StopLoopMove(false);
        SetMessage("Loop finish");
        return;
    }

    if(bLoopTargetIsPos2)
        iLoopTarget=Pos2[iLoopMotorIndex];
    else
        iLoopTarget=Pos1[iLoopMotorIndex];

    ApplySpeedPercent(Motor);
    Motor->MotorMove(iLoopTarget);
    dwLoopLegStartTick=GetTickCount();
    SetMessage(AnsiString("Loop move to ")+FormatPositionText(iLoopTarget));
}
//---------------------------------------------------------------------------
void TfMotorTest::RunNextMultiLoopMove()
{
    TTrayMotor *Motor;
    int MotorIndex;
    int Target;

    if(bLoopRunning==false)
        return;
    if(iLoopRemainCount<=0)
    {
        StopLoopMove(false);
        SetMessage("Multi loop finish");
        return;
    }

    for(MotorIndex=0; MotorIndex<GetMotorCount(); MotorIndex++)
    {
        if(bMultiLoopMotor[MotorIndex]==false)
            continue;
        Motor=GetMotor(MotorIndex);
        if(Motor==NULL)
            continue;
        Target=bLoopTargetIsPos2?Pos2[MotorIndex]:Pos1[MotorIndex];
        MultiLoopTarget[MotorIndex]=Target;
        ApplySpeedPercent(Motor);
        Motor->MotorMove(Target);
    }
    dwLoopLegStartTick=GetTickCount();
    SetMessage(bLoopTargetIsPos2?AnsiString("Multi loop to Pos2"):AnsiString("Multi loop to Pos1"));
}
//---------------------------------------------------------------------------
int TfMotorTest::GetLoopWaitMS()
{
    double WaitSec;
    int WaitMS;

    if(cbbLoopWait==NULL)
        return 0;
    WaitSec=atof(cbbLoopWait->Text.c_str());
    if(WaitSec<=0.0)
        return 0;
    if(WaitSec>100.0)
        WaitSec=100.0;
    WaitMS=(int)(WaitSec*1000.0+0.5);
    if(WaitMS<0)
        WaitMS=0;
    return WaitMS;
}
//---------------------------------------------------------------------------
void TfMotorTest::StartLoopWaitOrNext()
{
    int WaitMS;

    WaitMS=GetLoopWaitMS();
    if(WaitMS>0)
    {
        bLoopWaiting=true;
        dwLoopWaitUntil=GetTickCount()+WaitMS;
        SetMessage("Loop wait");
        return;
    }
    if(bLoopMultiMode)
        RunNextMultiLoopMove();
    else
        RunNextLoopMove();
}
//---------------------------------------------------------------------------
void TfMotorTest::OnLoopTargetArrived(TTrayMotor *Motor)
{
    DWORD NowTick;
    DWORD TripMS;
    DWORD AverageMS;

    if(Motor==NULL)
        return;

    NowTick=GetTickCount();
    TripMS=NowTick-dwLoopLegStartTick;
    AverageMS=0;

    if(bLoopTargetIsPos2)
    {
        iLoopFinishedCount++;
        if(iLoopFinishedCount>0)
            AverageMS=(NowTick-dwLoopStartTick)/(DWORD)iLoopFinishedCount;
        UpdateLoopStatisticLabels(TripMS, AverageMS);

        iLoopRemainCount--;
        edLoopCount->Text=IntToStr(iLoopRemainCount);
        if(iLoopRemainCount<=0)
        {
            StopLoopMove(false);
            SetMessage("Loop finish");
            return;
        }
        bLoopTargetIsPos2=false;
        StartLoopWaitOrNext();
    }
    else
    {
        UpdateLoopStatisticLabels(TripMS, AverageMS);
        bLoopTargetIsPos2=true;
        StartLoopWaitOrNext();
    }
}
//---------------------------------------------------------------------------
void TfMotorTest::OnMultiLoopTargetsArrived()
{
    DWORD NowTick;
    DWORD TripMS;
    DWORD AverageMS;

    NowTick=GetTickCount();
    TripMS=NowTick-dwLoopLegStartTick;
    AverageMS=0;

    if(bLoopTargetIsPos2)
    {
        iLoopFinishedCount++;
        if(iLoopFinishedCount>0)
            AverageMS=(NowTick-dwLoopStartTick)/(DWORD)iLoopFinishedCount;
        UpdateLoopStatisticLabels(TripMS, AverageMS);

        iLoopRemainCount--;
        edLoopCount->Text=IntToStr(iLoopRemainCount);
        if(iLoopRemainCount<=0)
        {
            StopLoopMove(false);
            SetMessage("Multi loop finish");
            return;
        }
        bLoopTargetIsPos2=false;
        StartLoopWaitOrNext();
    }
    else
    {
        UpdateLoopStatisticLabels(TripMS, AverageMS);
        bLoopTargetIsPos2=true;
        StartLoopWaitOrNext();
    }
}
//---------------------------------------------------------------------------
bool TfMotorTest::IsLoopTargetArrived(TTrayMotor *Motor)
{
    return IsMotorAtTarget(Motor, iLoopTarget);
}
//---------------------------------------------------------------------------
bool TfMotorTest::IsMotorAtTarget(TTrayMotor *Motor, int Target)
{
    int NowPos;
    int Tolerance;
    int Diff;

    if(Motor==NULL)
        return false;
    NowPos=Motor->ReadPos();
    Tolerance=Motor->GetRange();
    if(Tolerance<=0)
        Tolerance=100;
    Diff=NowPos-Target;
    if(Diff<0)
        Diff=-Diff;
    return Diff<=Tolerance;
}
//---------------------------------------------------------------------------
int TfMotorTest::GetSelectedMultiLoopCount()
{
    int Count=0;

    for(int MotorIndex=0; MotorIndex<GetMotorCount(); MotorIndex++)
        if(MotorIndex<MAX_MOTOR_TEST_MOTOR_COUNT && bMultiLoopMotor[MotorIndex])
            Count++;
    return Count;
}
//---------------------------------------------------------------------------
bool TfMotorTest::CheckAllMultiLoopTargetsArrived()
{
    TTrayMotor *Motor;

    for(int MotorIndex=0; MotorIndex<GetMotorCount(); MotorIndex++)
    {
        if(bMultiLoopMotor[MotorIndex]==false)
            continue;
        Motor=GetMotor(MotorIndex);
        if(IsMotorAtTarget(Motor, MultiLoopTarget[MotorIndex])==false)
            return false;
    }
    return true;
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::tmrUpdateTimer(TObject *Sender)
{
    TTrayMotor *Motor;
    AnsiString Err;
    static bool bEmgActive=false;
    (void)Sender;

    //AI(general) 20260616 : EMG only blocked the START of a move before; a move,
    //jog, home or loop already in progress kept running. Poll EMG here and stop
    //it (StopActiveMotor stops the active axis and aborts home and loop moves).
    //bEmgActive latches so we stop once per press and do not advance while held.
    if(IsEMGPressed()>0)
    {
        if(!bEmgActive)
        {
            bEmgActive=true;
            StopActiveMotor();
            SetMessage("Move abort: EMG");
        }
        UpdateMotorMonitor();
        return;
    }
    bEmgActive=false;

    // Reflect home-in-progress on the Home button itself (caption + red text) so
    // the operator sees it is busy, like HT172's latched spbHome. Sync only on
    // state change to avoid per-tick repaint flicker; covers finish and abort.
    static bool bHomeBtnShown=false;
    if(btnHome!=NULL && bHomeRunning!=bHomeBtnShown)
    {
        bHomeBtnShown=bHomeRunning;
        if(bHomeRunning)
        {
            btnHome->Caption="HOMING..";
            btnHome->Font->Color=clRed;
        }
        else
        {
            btnHome->Caption="HOME";
            btnHome->Font->Color=clWindowText;
        }
    }

    if(bHomeRunning && iHomeMotorIndex>=0 && iHomeMotorIndex<GetMotorCount())
    {
        Motor=GetMotor(iHomeMotorIndex);
        if(Motor!=NULL && Motor->Home(Err))
        {
            bHomeRunning=false;
            iHomeMotorIndex=-1;
            SetMessage("Home finish");
        }
        else if(Err!=AnsiString(""))
        {
            SetMessage(Err);
        }
    }

    if(bLoopRunning)
    {
        if(bLoopMultiMode)
        {
            if(bLoopWaiting)
            {
                if((int)(GetTickCount()-dwLoopWaitUntil)>=0)
                {
                    bLoopWaiting=false;
                    RunNextMultiLoopMove();
                }
            }
            else if(CheckAllMultiLoopTargetsArrived())
            {
                OnMultiLoopTargetsArrived();
            }
        }
        else
        {
            Motor=GetMotor(iLoopMotorIndex);
            if(Motor==NULL)
            {
                StopLoopMove(false);
            }
            else if(bLoopWaiting)
            {
                if((int)(GetTickCount()-dwLoopWaitUntil)>=0)
                {
                    bLoopWaiting=false;
                    RunNextLoopMove();
                }
            }
            else if(IsLoopTargetArrived(Motor))
            {
                OnLoopTargetArrived(Motor);
            }
        }
    }

    UpdateMotorMonitor();
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::grdMotorParameterSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect)
{
    (void)Sender;
    (void)ACol;

    if(ARow<=0)
        return;
    if(ARow-1==ActiveMotorIndex)
        return;
    // Batch-edit model (172-style): switching motors keeps pending edits in the
    // grid; one SAVE PARAM writes all changed rows. No save-before-switch lock.
    SetActiveMotor(ARow-1);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::grdMotorParameterDblClick(TObject *Sender)
{
    (void)Sender;
    if(grdMotorParameter==NULL)
        return;
    EditMotorParameterCell(grdMotorParameter->Row, grdMotorParameter->Col);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::edMotorInputClick(TObject *Sender)
{
    TEdit *Edit;

    Edit=dynamic_cast<TEdit *>(Sender);
    if(Edit==NULL)
        return;
    if(Edit==edSpeedPercent)
    {
        if(ShowMotorTestKeyboard(Edit, N_INTEGER, 1, true, 1.0, 100.0, "Speed %"))
            UpdateSpeedScrollFromEdit();
    }
    else if(Edit==edLoopCount)
    {
        ShowMotorTestKeyboard(Edit, N_INTEGER, 1, true, 1.0, 99999.0, "Loop Count");
    }
    else if(Edit==edStep)
    {
        ShowMotorTestKeyboard(Edit, N_DOUBLE, 2, true, 0.01, 999999.0, "Step");
    }
    else if(Edit==edTarget)
    {
        ShowMotorTestKeyboard(Edit, N_DOUBLE, 2, false, 0.0, 0.0, "Move To");
    }
    else if(Edit==edPos1)
    {
        ShowMotorTestKeyboard(Edit, N_DOUBLE, 2, false, 0.0, 0.0, "Pos1");
    }
    else if(Edit==edPos2)
    {
        ShowMotorTestKeyboard(Edit, N_DOUBLE, 2, false, 0.0, 0.0, "Pos2");
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnJogPMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Sender;
    (void)Button;
    (void)Shift;
    (void)X;
    (void)Y;
    StartJog(true);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnJogNMouseDown(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Sender;
    (void)Button;
    (void)Shift;
    (void)X;
    (void)Y;
    StartJog(false);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnJogMouseUp(TObject *Sender, TMouseButton Button, TShiftState Shift, int X, int Y)
{
    (void)Sender;
    (void)Button;
    (void)Shift;
    (void)X;
    (void)Y;
    StopActiveMotor();
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnStepPClick(TObject *Sender)
{
    (void)Sender;
    StepMove(true);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnStepNClick(TObject *Sender)
{
    (void)Sender;
    StepMove(false);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnMoveClick(TObject *Sender)
{
    (void)Sender;
    MoveActiveMotorToTarget();
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnHomeClick(TObject *Sender)
{
    TTrayMotor *Motor;
    (void)Sender;

    Motor=GetActiveMotor();
    if(Motor==NULL)
        return;
    if(CheckCanMotorMove(Motor, false, false, 0)==false)
        return;
    Motor->InitHomeTask_forSingleAxis();
    bHomeRunning=true;
    iHomeMotorIndex=ActiveMotorIndex;
    SetMessage("Home start");
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnStopClick(TObject *Sender)
{
    (void)Sender;
    StopActiveMotor();
    SetMessage("Stop");
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnRefreshClick(TObject *Sender)
{
    (void)Sender;
    if(ConfirmDiscardMotorParameterEdit()==false)
        return;
    FillMotorList();
    RefreshAllGrids();
    UpdateMotorMonitor();
    SetMessage("Refresh");
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnSetPos1Click(TObject *Sender)
{
    (void)Sender;
    SetPositionFromNow(1);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnSetPos2Click(TObject *Sender)
{
    (void)Sender;
    SetPositionFromNow(2);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnGoPos1Click(TObject *Sender)
{
    (void)Sender;
    if(ActiveMotorIndex>=0 && ActiveMotorIndex<GetMotorCount())
    {
        Pos1[ActiveMotorIndex]=ParsePositionText(edPos1->Text);
        MoveActiveMotorToPosition(Pos1[ActiveMotorIndex]);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnGoPos2Click(TObject *Sender)
{
    (void)Sender;
    if(ActiveMotorIndex>=0 && ActiveMotorIndex<GetMotorCount())
    {
        Pos2[ActiveMotorIndex]=ParsePositionText(edPos2->Text);
        MoveActiveMotorToPosition(Pos2[ActiveMotorIndex]);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnGoSoftNClick(TObject *Sender)
{
    (void)Sender;
    MoveActiveMotorToSoftLimit(false);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnGoSoftPClick(TObject *Sender)
{
    (void)Sender;
    MoveActiveMotorToSoftLimit(true);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnLoopStartClick(TObject *Sender)
{
    (void)Sender;
    StartLoopMove();
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnLoopStopClick(TObject *Sender)
{
    (void)Sender;
    StopLoopMove(true);
    SetMessage("Loop stop");
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnSaveClick(TObject *Sender)
{
    (void)Sender;
    SaveLoopPositions();
    SetMessage("Save motor_test.ini");
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnReloadClick(TObject *Sender)
{
    (void)Sender;
    LoadLoopPositions();
    SetMessage("Reload motor_test.ini");
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnCloseClick(TObject *Sender)
{
    (void)Sender;
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnParamSaveClick(TObject *Sender)
{
    (void)Sender;
    SaveMotorParameterToFile();
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnParamReloadClick(TObject *Sender)
{
    (void)Sender;
    ReloadActiveMotorParameter();
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnParamValidateClick(TObject *Sender)
{
    AnsiString SummaryText;
    AnsiString ErrorText;

    (void)Sender;
    if(bMotorParameterDirty || bMotorTableDirty)
    {
        if(MessageDlg("Unsaved MotorTest edits are not included in file validation. Continue?", mtConfirmation, TMsgDlgButtons() << mbYes << mbNo, 0)!=mrYes)
            return;
    }

    if(ValidateMotorTableCsv(SummaryText, ErrorText))
    {
        SetMessage(SummaryText);
        MessageDlg(SummaryText, mtInformation, TMsgDlgButtons() << mbOK, 0);
    }
    else
    {
        SetMessage("Mot_Table validation failed");
        MessageDlg(ErrorText, mtWarning, TMsgDlgButtons() << mbOK, 0);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnRegisterReadClick(TObject *Sender)
{
    (void)Sender;
    ReadDriverRegister(false);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnRegisterReadDefaultClick(TObject *Sender)
{
    (void)Sender;
    ReadDriverRegister(true);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnServoGuardOnClick(TObject *Sender)
{
    (void)Sender;
    RunServoPowerGuard(true);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnServoGuardOffClick(TObject *Sender)
{
    (void)Sender;
    RunServoPowerGuard(false);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnServoApplyOnClick(TObject *Sender)
{
    (void)Sender;
    ApplyServoPower(true);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnServoApplyOffClick(TObject *Sender)
{
    (void)Sender;
    ApplyServoPower(false);
}
//---------------------------------------------------------------------------
// Operate-panel Servo On/Off (mirrors HT172, which keeps servo power on the
// main motor-test screen). Routes through the same guarded apply path as the
// Servo Guard tab so the motor-relay sequencing/safety checks still run.
void __fastcall TfMotorTest::btnServoOnClick(TObject *Sender)
{
    (void)Sender;
    ApplyServoPower(true);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::btnServoOffClick(TObject *Sender)
{
    (void)Sender;
    ApplyServoPower(false);
}
//---------------------------------------------------------------------------
void __fastcall TfMotorTest::scrSpeedPercentScroll(TObject *Sender, TScrollCode ScrollCode, int &ScrollPos)
{
    TTrayMotor *Motor;
    (void)Sender;
    (void)ScrollCode;

    if(edSpeedPercent!=NULL)
        edSpeedPercent->Text=IntToStr(ScrollPos);
    Motor=GetActiveMotor();
    if(Motor!=NULL && bHomeRunning==false)
        ApplySpeedPercent(Motor);
}
//---------------------------------------------------------------------------