#include "IncludeAllHeader.h"       //Dell 將.h統一,可加速build
//---------------------------------------------------------------------------
#include <vcl.h>
#include <TypInfo.hpp>
#include <stdlib.h>
#pragma hdrstop

#include "iosetview.h"
#include "csystem.h"
#include "myio_MN200.h"
#include "uPadInterface.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "ALed"
#pragma link "MyLed"
#pragma link "butPa1"
#pragma resource "*.dfm"
Tfiosetview *fiosetview;
//---------------------------------------------------------------------------
static const TColor IO_COLOR_FORM        = (TColor)12761254;
static const TColor IO_COLOR_HEADER      = (TColor)9534289;
static const TColor IO_COLOR_BUTTON_OFF  = (TColor)8404992;
static const TColor IO_COLOR_BUTTON_ON   = (TColor)16744448;
static const TColor IO_COLOR_GRID_FIXED  = (TColor)9534289;
static const TColor IO_COLOR_SUCKER_BG   = (TColor)0x00DFD9CC;

enum eIOTableEditorColumn
{
    eIOTableColTag=0,
    eIOTableColType,
    eIOTableColAlias,
    eIOTableColLane,
    eIOTableColModuleType,
    eIOTableColIP,
    eIOTableColPort,
    eIOTableColBit,
    eIOTableColInType,
    eIOTableColISABase,
    eIOTableColEnable,
    eIOTableColOnAlarmTime,
    eIOTableColOffAlarmTime,
    eIOTableColOnDelayTime,
    eIOTableColOffDelayTime,
    eIOTableColNote,
    eIOTableColTotal
};

static void SetOrdProperty(TObject *ObjectPtr, const char *PropName, int Value)
{
    PPropInfo PropInfo;

    if(ObjectPtr==NULL || PropName==NULL)
        return;

    PropInfo=GetPropInfo(ObjectPtr, AnsiString(PropName));
    if(PropInfo!=NULL)
        SetOrdProp(ObjectPtr, PropInfo, Value);
}

static bool TextEndsWith(AnsiString Value, AnsiString Suffix)
{
    int ValueLength=Value.Length();
    int SuffixLength=Suffix.Length();

    if(SuffixLength<=0 || SuffixLength>ValueLength)
        return false;
    return Value.SubString(ValueLength-SuffixLength+1, SuffixLength)==Suffix;
}

static TTimer *FindNamedTimer(TComponent *Owner, const char *TimerName)
{
    TComponent *ComponentPtr;

    if(Owner==NULL || TimerName==NULL)
        return NULL;

    ComponentPtr=Owner->FindComponent(AnsiString(TimerName));
    return dynamic_cast<TTimer *>(ComponentPtr);
}

enum eIOViewSelectKind
{
    eIOViewNone     =0,
    eIOViewSwitch   =1,
    eIOViewCylinder =2,
    eIOViewSucker   =3
};

struct TIOMapItem
{
    int Lane;
    int IP;
    int Port;
    int Bit;
    AnsiString Type;
    AnsiString Alias;
};
//---------------------------------------------------------------------------
// iosetview.dfm references custom components (TMyLed/TBtnPanel/TALed) plus the
// usual VCL classes. Their design packages are not installed in this IDE, so
// VCL's streaming class registry does not contain them at runtime. Register
// every class the DFM uses here, once, before Tfiosetview is streamed; without
// this, CreateForm(Tfiosetview) throws "Class Txxx not found".
void RegisterIOViewStreamClasses()
{
    static bool bRegistered=false;

    if(bRegistered)
        return;

    RegisterClass(__classid(TPanel));
    RegisterClass(__classid(TSpeedButton));
    RegisterClass(__classid(TButton));
    RegisterClass(__classid(TLabel));
    RegisterClass(__classid(TCheckBox));
    RegisterClass(__classid(TPageControl));
    RegisterClass(__classid(TTabSheet));
    RegisterClass(__classid(TGroupBox));
    RegisterClass(__classid(TEdit));
    RegisterClass(__classid(TRadioButton));
    RegisterClass(__classid(TComboBox));
    RegisterClass(__classid(TStringGrid));
    RegisterClass(__classid(TOpenDialog));
    RegisterClass(__classid(TSaveDialog));
    RegisterClass(__classid(TTimer));
    RegisterClass(__classid(TListBox));
    RegisterClass(__classid(TPopupMenu));
    RegisterClass(__classid(TMenuItem));
    RegisterClass(__classid(TDataSource));
    RegisterClass(__classid(TDBNavigator));
    RegisterClass(__classid(TDBGrid));
    RegisterClass(__classid(TTable));
    RegisterClass(__classid(TMemo));
    RegisterClass(__classid(TImage));
    RegisterClass(__classid(TMyLed));
    RegisterClass(__classid(TBtnPanel));
    RegisterClass(__classid(TALed));

    bRegistered=true;
}
//---------------------------------------------------------------------------
__fastcall Tfiosetview::Tfiosetview(TComponent* Owner)
    : TForm(Owner)
{
    palHeader=NULL;
    lblTitle=NULL;
    lblSummary=NULL;
    lblSelected=NULL;
    chkManualOutput=NULL;
    btnRefresh=NULL;
    btnOutputOn=NULL;
    btnOutputOff=NULL;
    btnSuckerDestroy=NULL;
    PageControl=NULL;
    tsSensors=NULL;
    tsCylinders=NULL;
    tsSwitches=NULL;
    tsSuckers=NULL;
    tsIOTable=NULL;
    grdSensors=NULL;
    grdCylinders=NULL;
    grdSwitches=NULL;
    grdSuckers=NULL;
    grdIOTable=NULL;
    pnIOTableEditorToolbar=NULL;
    cbbType=NULL;
    cbbLane=NULL;
    edtSearchIO=NULL;
    btnAddIO=NULL;
    btnDeleteIO=NULL;
    btnModify=NULL;
    sbUpdate=NULL;
    strngrdIoTable=NULL;
    IOTableDeletedTags=new TStringList();
    ManualOutputLog=new TStringList();
    SelectedKind=eIOViewNone;
    SelectedIndex=-1;
    SelectedRow=-1;
    SelectedCol=-1;
    iSelectRow=0;
    iSelectCol=0;
    Color=IO_COLOR_FORM;
    fShow=false;
}
//---------------------------------------------------------------------------
__fastcall Tfiosetview::~Tfiosetview()
{
    delete IOTableDeletedTags;
    IOTableDeletedTags=NULL;
    delete ManualOutputLog;
    ManualOutputLog=NULL;
}
//---------------------------------------------------------------------------
void Tfiosetview::BuildUI()
{
    Color=IO_COLOR_FORM;
    Font->Name="Arial";
    Font->Size=10;
    BuildHeader();
    BuildPages();
}
//---------------------------------------------------------------------------
void Tfiosetview::BuildHeader()
{
    palHeader=new TPanel(this);
    palHeader->Parent=this;
    palHeader->Align=alTop;
    palHeader->Height=86;
    palHeader->BevelOuter=bvNone;
    palHeader->Color=IO_COLOR_HEADER;

    lblTitle=new TLabel(palHeader);
    lblTitle->Parent=palHeader;
    lblTitle->Left=16;
    lblTitle->Top=10;
    lblTitle->Caption="HT160S IO View";
    lblTitle->Font->Color=clWhite;
    lblTitle->Font->Size=16;
    lblTitle->Font->Style=TFontStyles() << fsBold;

    lblSummary=new TLabel(palHeader);
    lblSummary->Parent=palHeader;
    lblSummary->Left=18;
    lblSummary->Top=48;
    lblSummary->Width=900;
    lblSummary->Caption="";
    lblSummary->Font->Color=clWhite;

    btnRefresh=new TButton(palHeader);
    btnRefresh->Parent=palHeader;
    btnRefresh->Left=940;
    btnRefresh->Top=14;
    btnRefresh->Width=90;
    btnRefresh->Height=28;
    btnRefresh->Caption="Refresh";
    btnRefresh->OnClick=btnRefreshClick;

    chkManualOutput=new TCheckBox(palHeader);
    chkManualOutput->Parent=palHeader;
    chkManualOutput->Left=1045;
    chkManualOutput->Top=18;
    chkManualOutput->Width=145;
    chkManualOutput->Caption="Manual output";
    chkManualOutput->Font->Color=clWhite;
    chkManualOutput->OnClick=chkManualOutputClick;

    btnOutputOn=new TButton(palHeader);
    btnOutputOn->Parent=palHeader;
    btnOutputOn->Left=1205;
    btnOutputOn->Top=14;
    btnOutputOn->Width=80;
    btnOutputOn->Height=28;
    btnOutputOn->Caption="ON";
    btnOutputOn->OnClick=btnOutputOnClick;

    btnOutputOff=new TButton(palHeader);
    btnOutputOff->Parent=palHeader;
    btnOutputOff->Left=1295;
    btnOutputOff->Top=14;
    btnOutputOff->Width=80;
    btnOutputOff->Height=28;
    btnOutputOff->Caption="OFF";
    btnOutputOff->OnClick=btnOutputOffClick;

    btnSuckerDestroy=new TButton(palHeader);
    btnSuckerDestroy->Parent=palHeader;
    btnSuckerDestroy->Left=1385;
    btnSuckerDestroy->Top=14;
    btnSuckerDestroy->Width=110;
    btnSuckerDestroy->Height=28;
    btnSuckerDestroy->Caption="Destroy";
    btnSuckerDestroy->OnClick=btnSuckerDestroyClick;

    lblSelected=new TLabel(palHeader);
    lblSelected->Parent=palHeader;
    lblSelected->Left=940;
    lblSelected->Top=52;
    lblSelected->Width=520;
    lblSelected->Caption="Selected: none";
    lblSelected->Font->Color=clWhite;
}
//---------------------------------------------------------------------------
void Tfiosetview::BuildPages()
{
    const char *SensorHeaders[]   = {"No", "Alias", "Status", "Lane", "IP", "Port", "Bit", "Type", "Enable", "IOPos", "Driver"};
    const int SensorWidths[]      = {40, 220, 70, 55, 55, 55, 45, 55, 60, 120, 110};
    const char *CylinderHeaders[] = {"No", "Cylinder", "Out", "On Sen", "Off Sen", "Enable", "Addr", "OnAddr", "OffAddr"};
    const int CylinderWidths[]    = {40, 220, 65, 80, 80, 70, 120, 120, 120};
    const char *SwitchHeaders[]   = {"No", "Switch", "Out", "Enable", "Addr", "IOPos", "Driver"};
    const int SwitchWidths[]      = {40, 220, 65, 70, 120, 160, 110};
    const char *SuckerHeaders[]   = {"No", "Sucker", "Row", "Col", "Vacuum", "Suck Out", "Destroy Out", "Enable", "SensorAddr", "OnAddr", "OffAddr"};
    const int SuckerWidths[]      = {40, 120, 45, 45, 70, 80, 90, 70, 120, 120, 120};
    const char *IOTableHeaders[]  = {"No", "Type", "Alias", "Lane", "ModuleType", "IP", "Port", "Bit", "InType", "ISA Base", "Enable", "OnAlarmTime", "OffAlarmTime", "OnDelayTime", "OffDelayTime", "Note"};
    const int IOTableWidths[]     = {50, 90, 210, 50, 70, 55, 65, 45, 55, 65, 60, 85, 85, 85, 85, 260};

    PageControl=new TPageControl(this);
    PageControl->Parent=this;
    PageControl->Align=alClient;
    PageControl->TabPosition=tpTop;

    tsSensors=new TTabSheet(PageControl);
    tsSensors->PageControl=PageControl;
    tsSensors->Caption="Sensors";
    grdSensors=CreateGrid(tsSensors, 11, SensorHeaders, SensorWidths);

    tsCylinders=new TTabSheet(PageControl);
    tsCylinders->PageControl=PageControl;
    tsCylinders->Caption="Cylinders";
    grdCylinders=CreateGrid(tsCylinders, 9, CylinderHeaders, CylinderWidths);

    tsSwitches=new TTabSheet(PageControl);
    tsSwitches->PageControl=PageControl;
    tsSwitches->Caption="Switches";
    grdSwitches=CreateGrid(tsSwitches, 7, SwitchHeaders, SwitchWidths);

    tsSuckers=new TTabSheet(PageControl);
    tsSuckers->PageControl=PageControl;
    tsSuckers->Caption="Suckers";
    grdSuckers=CreateGrid(tsSuckers, 11, SuckerHeaders, SuckerWidths);

    tsIOTable=new TTabSheet(PageControl);
    tsIOTable->PageControl=PageControl;
    tsIOTable->Caption="IO Table";
    grdIOTable=CreateGrid(tsIOTable, eIOTableColTotal, IOTableHeaders, IOTableWidths);
}
//---------------------------------------------------------------------------
TStringGrid *Tfiosetview::CreateGrid(TWinControl *Parent, int ColCount, const char **Headers, const int *Widths)
{
    TStringGrid *Grid=new TStringGrid(Parent);
    Grid->Parent=Parent;
    Grid->Align=alClient;
    SetupGrid(Grid, ColCount, Headers, Widths);
    return Grid;
}
//---------------------------------------------------------------------------
void Tfiosetview::SetupGrid(TStringGrid *Grid, int ColCount, const char **Headers, const int *Widths)
{
    Grid->ColCount=ColCount;
    Grid->RowCount=2;
    Grid->FixedRows=1;
    Grid->FixedCols=0;
    Grid->DefaultRowHeight=22;
    Grid->FixedColor=IO_COLOR_GRID_FIXED;
    Grid->Color=clWhite;
    Grid->Font->Name="Arial";
    Grid->Font->Size=9;
    Grid->Options=Grid->Options << goRowSelect << goColSizing;
    Grid->OnSelectCell=GridSelectCell;
    for(int Col=0; Col<ColCount; Col++)
    {
        Grid->Cells[Col][0]=Headers[Col];
        Grid->ColWidths[Col]=Widths[Col];
    }
}
//---------------------------------------------------------------------------
void Tfiosetview::SetGridRowCount(TStringGrid *Grid, int RowCount)
{
    if(RowCount<2)
        RowCount=2;
    Grid->RowCount=RowCount;
}
//---------------------------------------------------------------------------
void Tfiosetview::ClearGridRows(TStringGrid *Grid)
{
    for(int Row=1; Row<Grid->RowCount; Row++)
    {
        for(int Col=0; Col<Grid->ColCount; Col++)
            Grid->Cells[Col][Row]="";
    }
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::FormatEnable(bool Flag)
{
    if(Flag)
        return "YES";
    return "NO";
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::FormatEnableInt(int Flag)
{
    if(Flag!=0)
        return "YES";
    return "NO";
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::FormatOnOff(bool Flag)
{
    if(Flag)
        return "ON";
    return "OFF";
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::FormatAddress(int Lane, int IP, int Port, int Bit)
{
    AnsiString Str;
    Str.sprintf("L%d IP%d P%d B%d", Lane, IP, Port, Bit);
    return Str;
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::FormatIODataAddress(TIODATA *Data)
{
    if(Data==NULL)
        return "";
    return FormatAddress(Data->iLane, Data->iIP, Data->iPort, Data->iBit);
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::FormatIODriver(TIODATA *Data)
{
    if(Data==NULL)
        return "";
    if(IsPadCommunicationData(Data))
        return "Pad COM";
    if(Data->iEnable!=0 && Data->iISABase==eMotionNet)
        return "TMyMN200_IO";
    return "TMyIo";
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::FormatIODriver(TMyIo *IOPtr)
{
    if(IOPtr==NULL)
        return "";
    return IOPtr->GetDriverName();
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::FormatSensor(TMySensor *Sensor)
{
    if(Sensor==NULL || Sensor->Enable==false)
        return "DISABLED";
    if(Sensor->IsOn())
        return "ON";
    return "OFF";
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::FormatSwitch(TMySwitch *SwitchPtr)
{
    if(SwitchPtr==NULL)
        return "";
    return FormatAddress(SwitchPtr->Card/100, SwitchPtr->Card%100, SwitchPtr->Port, SwitchPtr->Bit);
}
//---------------------------------------------------------------------------
int Tfiosetview::CountIOType(AnsiString TypeName)
{
    int Count=0;
    AnsiString Target=TypeName.UpperCase();
    if(HSys.IOTable==NULL)
        return 0;
    for(int Index=0; Index<HSys.IOTable->Count; Index++)
    {
        TIODATA *Data=(TIODATA *)HSys.IOTable->Items[Index];
        if(Data!=NULL && Data->Type.UpperCase()==Target)
            Count++;
    }
    return Count;
}
//---------------------------------------------------------------------------
TIODATA *Tfiosetview::GetIODataByFilteredRow(AnsiString TypeName, int RowIndex)
{
    int Count=0;
    AnsiString Target=TypeName.UpperCase();
    if(HSys.IOTable==NULL || RowIndex<1)
        return NULL;
    for(int Index=0; Index<HSys.IOTable->Count; Index++)
    {
        TIODATA *Data=(TIODATA *)HSys.IOTable->Items[Index];
        if(Data!=NULL && Data->Type.UpperCase()==Target)
        {
            Count++;
            if(Count==RowIndex)
                return Data;
        }
    }
    return NULL;
}
//---------------------------------------------------------------------------
void Tfiosetview::RefreshSummary()
{
    AnsiString Str;
    int IOCount=(HSys.IOTable==NULL)?0:HSys.IOTable->Count;
    Str.sprintf("IO=%d  Sensor=%d  Cylinder=%d  Switch=%d  SuckerGroup=%d  Path=%s",
                IOCount,
                CountIOType("Sensor"),
                HSys.iTotalCylinder,
                HSys.iTotalSwitch,
                HSys.iTotalSucker,
                HSys.IoTablePath.c_str());
    lblSummary->Caption=Str;
}
//---------------------------------------------------------------------------
void Tfiosetview::RefreshSensors()
{
    int Count=CountIOType("Sensor");
    SetGridRowCount(grdSensors, Count+1);
    ClearGridRows(grdSensors);
    for(int Row=1; Row<=Count; Row++)
    {
        TIODATA *Data=GetIODataByFilteredRow("Sensor", Row);
        if(Data==NULL)
            continue;

        if(IsPadCommunicationData(Data))
        {
            bool PadState=false;
            ResolvePadCommunicationInputState(Data->Alias, &PadState);
            grdSensors->Cells[0][Row]=IntToStr(Row);
            grdSensors->Cells[1][Row]=Data->Alias;
            grdSensors->Cells[2][Row]=FormatOnOff(PadState);
            grdSensors->Cells[3][Row]="";
            grdSensors->Cells[4][Row]="";
            grdSensors->Cells[5][Row]="";
            grdSensors->Cells[6][Row]="";
            grdSensors->Cells[7][Row]="COMM";
            grdSensors->Cells[8][Row]="COMM";
            grdSensors->Cells[9][Row]="Pad COM";
            grdSensors->Cells[10][Row]="Pad COM";
            continue;
        }

        TMyIo TempIO;
        TMyMN200_IO TempMN200IO;
        TMyIo *TempIOPtr=&TempIO;
        if(Data->iEnable!=0 && Data->iISABase==eMotionNet)
            TempIOPtr=&TempMN200IO;
        TempIOPtr->iCard=Data->iLane*100+Data->iIP;
        TempIOPtr->iLane=Data->iLane;
        TempIOPtr->iIP=Data->iIP;
        TempIOPtr->iPort=Data->iPort;
        TempIOPtr->iBit=Data->iBit;
        TempIOPtr->ISABase=Data->iISABase;
        TempIOPtr->iModuleType=Data->iModuleType;
        bool RawOn=TempIOPtr->IsOn();
        bool IsOn=(Data->iInType==1)?RawOn:!RawOn;

        grdSensors->Cells[0][Row]=IntToStr(Row);
        grdSensors->Cells[1][Row]=Data->Alias;
        if(Data->iEnable!=0)
            grdSensors->Cells[2][Row]=FormatOnOff(IsOn);
        else
            grdSensors->Cells[2][Row]="DISABLED";
        grdSensors->Cells[3][Row]=IntToStr(Data->iLane);
        grdSensors->Cells[4][Row]=IntToStr(Data->iIP);
        grdSensors->Cells[5][Row]=IntToStr(Data->iPort);
        grdSensors->Cells[6][Row]=IntToStr(Data->iBit);
        grdSensors->Cells[7][Row]=IntToStr(Data->iInType);
        grdSensors->Cells[8][Row]=FormatEnableInt(Data->iEnable);
        grdSensors->Cells[9][Row]=Data->IOPos;
        grdSensors->Cells[10][Row]=FormatIODriver(Data);
    }
}
//---------------------------------------------------------------------------
void Tfiosetview::RefreshCylinders()
{
    SetGridRowCount(grdCylinders, HSys.iTotalCylinder+1);
    ClearGridRows(grdCylinders);
    for(int Index=0; Index<HSys.iTotalCylinder; Index++)
    {
        TMyCylinder *Cyn=&HSys.CynPtr[Index];
        grdCylinders->Cells[0][Index+1]=IntToStr(Index);
        grdCylinders->Cells[1][Index+1]=Cyn->CylinderName;
        grdCylinders->Cells[2][Index+1]=FormatOnOff(Cyn->GetOutBit());
        grdCylinders->Cells[3][Index+1]=FormatSensor(&Cyn->OnSensor);
        grdCylinders->Cells[4][Index+1]=FormatSensor(&Cyn->OffSensor);
        grdCylinders->Cells[5][Index+1]=FormatEnable(Cyn->Enable);
        grdCylinders->Cells[6][Index+1]=FormatSwitch(&Cyn->Switch);
        grdCylinders->Cells[7][Index+1]=FormatAddress(Cyn->OnSensor.Card/100, Cyn->OnSensor.Card%100, Cyn->OnSensor.Port, Cyn->OnSensor.Bit);
        grdCylinders->Cells[8][Index+1]=FormatAddress(Cyn->OffSensor.Card/100, Cyn->OffSensor.Card%100, Cyn->OffSensor.Port, Cyn->OffSensor.Bit);
    }
}
//---------------------------------------------------------------------------
void Tfiosetview::RefreshSwitches()
{
    SetGridRowCount(grdSwitches, HSys.iTotalSwitch+1);
    ClearGridRows(grdSwitches);
    for(int Index=0; Index<HSys.iTotalSwitch; Index++)
    {
        TMySwitch *SwitchPtr=&HSys.SwPtr[Index];
        bool PadState=false;
        grdSwitches->Cells[0][Index+1]=IntToStr(Index);
        grdSwitches->Cells[1][Index+1]=SwitchPtr->Name;
        if(IsPadCommunicationOutput(SwitchPtr->Name))
        {
            ResolvePadCommunicationOutputState(SwitchPtr->Name, &PadState);
            grdSwitches->Cells[2][Index+1]=FormatOnOff(PadState);
            grdSwitches->Cells[3][Index+1]="COMM";
            grdSwitches->Cells[4][Index+1]="Pad COM";
            grdSwitches->Cells[5][Index+1]="Pad COM";
            grdSwitches->Cells[6][Index+1]="Pad COM";
        }
        else
        {
            grdSwitches->Cells[2][Index+1]=FormatOnOff(SwitchPtr->Status());
            grdSwitches->Cells[3][Index+1]=FormatEnable(SwitchPtr->Enable);
            grdSwitches->Cells[4][Index+1]=FormatSwitch(SwitchPtr);
            grdSwitches->Cells[5][Index+1]=SwitchPtr->IOPos;
            grdSwitches->Cells[6][Index+1]=FormatIODriver(SwitchPtr->Output);
        }
    }
}
//---------------------------------------------------------------------------
void Tfiosetview::RefreshSuckers()
{
    int TotalRows=0;
    for(int SuckerIndex=0; SuckerIndex<HSys.iTotalSucker; SuckerIndex++)
        TotalRows+=HSys.SuckPtr[SuckerIndex].MaxItemR*HSys.SuckPtr[SuckerIndex].MaxItemC;

    SetGridRowCount(grdSuckers, TotalRows+1);
    ClearGridRows(grdSuckers);

    int Row=1;
    for(int SuckerIndex=0; SuckerIndex<HSys.iTotalSucker; SuckerIndex++)
    {
        TMyKitSuck *Kit=&HSys.SuckPtr[SuckerIndex];
        for(int RowIndex=0; RowIndex<Kit->MaxItemR; RowIndex++)
        {
            for(int ColIndex=0; ColIndex<Kit->MaxItemC; ColIndex++)
            {
                TMySucker *Sucker=&Kit->Suck[RowIndex][ColIndex];
                grdSuckers->Cells[0][Row]=IntToStr(Row-1);
                grdSuckers->Cells[1][Row]=Sucker->SuckerName;
                grdSuckers->Cells[2][Row]=IntToStr(RowIndex);
                grdSuckers->Cells[3][Row]=IntToStr(ColIndex);
                grdSuckers->Cells[4][Row]=FormatSensor(&Sucker->Sensor);
                grdSuckers->Cells[5][Row]=FormatOnOff(Sucker->GetOnBit());
                grdSuckers->Cells[6][Row]=FormatOnOff(Sucker->GetOffBit());
                grdSuckers->Cells[7][Row]=FormatEnable(Sucker->Enable);
                grdSuckers->Cells[8][Row]=FormatAddress(Sucker->Sensor.Card/100, Sucker->Sensor.Card%100, Sucker->Sensor.Port, Sucker->Sensor.Bit);
                grdSuckers->Cells[9][Row]=FormatSwitch(&Sucker->OnSw);
                grdSuckers->Cells[10][Row]=FormatSwitch(&Sucker->OffSw);
                Row++;
            }
        }
    }
}
//---------------------------------------------------------------------------
void Tfiosetview::RefreshIOTable()
{
    int Count=(HSys.IOTable==NULL)?0:HSys.IOTable->Count;
    SetGridRowCount(grdIOTable, Count+1);
    ClearGridRows(grdIOTable);
    for(int Index=0; Index<Count; Index++)
    {
        TIODATA *Data=(TIODATA *)HSys.IOTable->Items[Index];
        if(Data==NULL)
            continue;
        grdIOTable->Cells[eIOTableColTag][Index+1]=IntToStr(Data->Tag);
        grdIOTable->Cells[eIOTableColType][Index+1]=Data->Type;
        grdIOTable->Cells[eIOTableColAlias][Index+1]=Data->Alias;
        grdIOTable->Cells[eIOTableColLane][Index+1]=IOTableCellFromInt(Data->iLane);
        grdIOTable->Cells[eIOTableColModuleType][Index+1]=IOTableCellFromInt(Data->iModuleType);
        grdIOTable->Cells[eIOTableColIP][Index+1]=IOTableIPToCell(Data);
        grdIOTable->Cells[eIOTableColPort][Index+1]=IOTablePortToCell(Data);
        grdIOTable->Cells[eIOTableColBit][Index+1]=IOTableCellFromInt(Data->iBit);
        grdIOTable->Cells[eIOTableColInType][Index+1]=IOTableCellFromInt(Data->iInType);
        grdIOTable->Cells[eIOTableColISABase][Index+1]=IOTableCellFromInt(Data->iISABase);
        grdIOTable->Cells[eIOTableColEnable][Index+1]=IOTableCellFromInt(Data->iEnable);
        grdIOTable->Cells[eIOTableColOnAlarmTime][Index+1]=IOTableCellFromInt(Data->iOnAlarmTime);
        grdIOTable->Cells[eIOTableColOffAlarmTime][Index+1]=IOTableCellFromInt(Data->iOffAlarmTime);
        grdIOTable->Cells[eIOTableColOnDelayTime][Index+1]=IOTableCellFromInt(Data->iOnDelayTime);
        grdIOTable->Cells[eIOTableColOffDelayTime][Index+1]=IOTableCellFromInt(Data->iOffDelayTime);
        grdIOTable->Cells[eIOTableColNote][Index+1]=GetIOTableNote(Data);
    }
}
//---------------------------------------------------------------------------
void Tfiosetview::EnsureIOTableEditor()
{
    TPanel *Host;
    TLabel *LabelPtr;
    TSpeedButton *RefreshButton;
    int LaneIndex;

    if(strngrdIoTable!=NULL)
        return;

    Host=dynamic_cast<TPanel *>(FindComponent("pn_IODatabase"));
    if(Host==NULL)
        return;

    HideLegacyIOTableEditor();

    pnIOTableEditorToolbar=new TPanel(Host);
    pnIOTableEditorToolbar->Parent=Host;
    pnIOTableEditorToolbar->Align=alTop;
    pnIOTableEditorToolbar->Height=48;
    pnIOTableEditorToolbar->BevelOuter=bvNone;
    pnIOTableEditorToolbar->Color=IO_COLOR_FORM;

    LabelPtr=new TLabel(pnIOTableEditorToolbar);
    LabelPtr->Parent=pnIOTableEditorToolbar;
    LabelPtr->Left=10;
    LabelPtr->Top=7;
    LabelPtr->Caption="Type";

    cbbType=new TComboBox(pnIOTableEditorToolbar);
    cbbType->Parent=pnIOTableEditorToolbar;
    cbbType->Left=10;
    cbbType->Top=22;
    cbbType->Width=105;
    cbbType->Style=csDropDownList;
    cbbType->Items->Add("All");
    cbbType->Items->Add("Sensor");
    cbbType->Items->Add("Sucker");
    cbbType->Items->Add("Switch");
    cbbType->Items->Add("Cylinder");
    cbbType->ItemIndex=0;
    cbbType->OnChange=cbbTypeChange;

    LabelPtr=new TLabel(pnIOTableEditorToolbar);
    LabelPtr->Parent=pnIOTableEditorToolbar;
    LabelPtr->Left=125;
    LabelPtr->Top=7;
    LabelPtr->Caption="Lane";

    cbbLane=new TComboBox(pnIOTableEditorToolbar);
    cbbLane->Parent=pnIOTableEditorToolbar;
    cbbLane->Left=125;
    cbbLane->Top=22;
    cbbLane->Width=75;
    cbbLane->Style=csDropDownList;
    cbbLane->Items->Add("All");
    for(LaneIndex=0; LaneIndex<10; LaneIndex++)
        cbbLane->Items->Add(IntToStr(LaneIndex));
    cbbLane->ItemIndex=0;
    cbbLane->OnChange=cbbTypeChange;

    LabelPtr=new TLabel(pnIOTableEditorToolbar);
    LabelPtr->Parent=pnIOTableEditorToolbar;
    LabelPtr->Left=212;
    LabelPtr->Top=7;
    LabelPtr->Caption="Search Alias";

    edtSearchIO=new TEdit(pnIOTableEditorToolbar);
    edtSearchIO->Parent=pnIOTableEditorToolbar;
    edtSearchIO->Left=212;
    edtSearchIO->Top=22;
    edtSearchIO->Width=180;
    edtSearchIO->OnChange=edtSearchIOChange;

    btnAddIO=new TSpeedButton(pnIOTableEditorToolbar);
    btnAddIO->Parent=pnIOTableEditorToolbar;
    btnAddIO->Left=410;
    btnAddIO->Top=12;
    btnAddIO->Width=70;
    btnAddIO->Height=28;
    btnAddIO->Caption="Add";
    btnAddIO->OnClick=btnAddIOClick;

    btnDeleteIO=new TSpeedButton(pnIOTableEditorToolbar);
    btnDeleteIO->Parent=pnIOTableEditorToolbar;
    btnDeleteIO->Left=488;
    btnDeleteIO->Top=12;
    btnDeleteIO->Width=70;
    btnDeleteIO->Height=28;
    btnDeleteIO->Caption="Delete";
    btnDeleteIO->OnClick=btnDeleteIOClick;

    btnModify=new TSpeedButton(pnIOTableEditorToolbar);
    btnModify->Parent=pnIOTableEditorToolbar;
    btnModify->Left=566;
    btnModify->Top=12;
    btnModify->Width=70;
    btnModify->Height=28;
    btnModify->Caption="Modify";
    btnModify->OnClick=btnModifyClick;

    sbUpdate=new TSpeedButton(pnIOTableEditorToolbar);
    sbUpdate->Parent=pnIOTableEditorToolbar;
    sbUpdate->Left=644;
    sbUpdate->Top=12;
    sbUpdate->Width=70;
    sbUpdate->Height=28;
    sbUpdate->Caption="Save";
    sbUpdate->OnClick=sbUpdateClick;

    RefreshButton=new TSpeedButton(pnIOTableEditorToolbar);
    RefreshButton->Parent=pnIOTableEditorToolbar;
    RefreshButton->Left=722;
    RefreshButton->Top=12;
    RefreshButton->Width=80;
    RefreshButton->Height=28;
    RefreshButton->Caption="Refresh";
    RefreshButton->OnClick=sbIORefreshClick;

    strngrdIoTable=new TStringGrid(Host);
    strngrdIoTable->Parent=Host;
    strngrdIoTable->Align=alClient;
    strngrdIoTable->OnSelectCell=strngrdIoTableSelectCell;
    strngrdIoTable->OnDblClick=strngrdIoTableDblClick;

    SetupIOTableEditorGrid();
    LoadIoTable(0, 0, 0);
}
//---------------------------------------------------------------------------
void Tfiosetview::HideLegacyIOTableEditor()
{
    const char *Names[]={"DBNavigator1", "DBGrid1", "pn_IODatabaseOB0"};
    TComponent *ComponentPtr;
    TControl *ControlPtr;

    for(int Index=0; Index<3; Index++)
    {
        ComponentPtr=FindComponent(Names[Index]);
        ControlPtr=dynamic_cast<TControl *>(ComponentPtr);
        if(ControlPtr!=NULL)
            ControlPtr->Visible=false;
    }
}
//---------------------------------------------------------------------------
void Tfiosetview::SetupIOTableEditorGrid()
{
    const char *Headers[eIOTableColTotal]={"No", "Type", "Alias", "Lane", "ModuleType", "IP", "Port", "Bit", "InType", "ISA Base", "Enable", "OnAlarmTime", "OffAlarmTime", "OnDelayTime", "OffDelayTime", "Note"};
    const int Widths[eIOTableColTotal]={50, 90, 210, 50, 70, 55, 65, 45, 55, 65, 60, 85, 85, 85, 85, 260};

    if(strngrdIoTable==NULL)
        return;

    strngrdIoTable->ColCount=eIOTableColTotal;
    if(strngrdIoTable->RowCount<2)
        strngrdIoTable->RowCount=2;
    strngrdIoTable->FixedRows=1;
    strngrdIoTable->FixedCols=0;
    strngrdIoTable->DefaultRowHeight=22;
    strngrdIoTable->FixedColor=IO_COLOR_GRID_FIXED;
    strngrdIoTable->Color=clWhite;
    strngrdIoTable->Font->Name="Arial";
    strngrdIoTable->Font->Size=9;
    strngrdIoTable->Options=TGridOptions() << goFixedVertLine << goFixedHorzLine << goVertLine << goHorzLine << goRangeSelect << goColSizing;
    for(int Col=0; Col<eIOTableColTotal; Col++)
    {
        strngrdIoTable->Cells[Col][0]=Headers[Col];
        strngrdIoTable->ColWidths[Col]=Widths[Col];
    }
}
//---------------------------------------------------------------------------
void Tfiosetview::LoadIoTable(int iType, int iLane, int iIP)
{
    int Count;
    int Row;
    int LaneFilter;
    AnsiString SearchText;

    (void)iIP;
    if(strngrdIoTable==NULL)
        return;

    SetupIOTableEditorGrid();
    SetGridRowCount(strngrdIoTable, 2);
    ClearGridRows(strngrdIoTable);

    LaneFilter=-1;
    if(cbbLane!=NULL && cbbLane->ItemIndex>0)
        LaneFilter=atoi(cbbLane->Text.c_str());

    SearchText="";
    if(edtSearchIO!=NULL && edtSearchIO->Text.Length()>=2)
        SearchText=edtSearchIO->Text.UpperCase();

    Count=(HSys.IOTable==NULL)?0:HSys.IOTable->Count;
    Row=1;
    for(int Index=0; Index<Count; Index++)
    {
        TIODATA *Data=(TIODATA *)HSys.IOTable->Items[Index];
        if(Data==NULL)
            continue;
        if(IOTableDeletedTags!=NULL && IOTableDeletedTags->IndexOf(IntToStr(Data->Tag))>=0)
            continue;
        if(!RowMatchesIOTableFilter(Data, iType, LaneFilter, SearchText))
            continue;
        if(Row>=strngrdIoTable->RowCount)
            SetGridRowCount(strngrdIoTable, Row+1);
        FillIOTableEditorRow(Row, Data);
        Row++;
    }
    SetGridRowCount(strngrdIoTable, Row);
}
//---------------------------------------------------------------------------
bool Tfiosetview::RowMatchesIOTableFilter(TIODATA *Data, int TypeFilter, int LaneFilter, AnsiString SearchText)
{
    AnsiString TypeName[5]={"", "SENSOR", "SUCKER", "SWITCH", "CYLINDER"};
    AnsiString DataType;
    AnsiString AliasName;

    if(Data==NULL)
        return false;

    DataType=Data->Type.UpperCase();
    if(TypeFilter>0 && TypeFilter<5)
    {
        if(DataType.AnsiPos(TypeName[TypeFilter])<=0)
            return false;
    }

    if(LaneFilter>=0 && Data->iLane!=LaneFilter)
        return false;

    if(SearchText!=AnsiString(""))
    {
        AliasName=Data->Alias.UpperCase();
        if(AliasName.AnsiPos(SearchText)<=0)
            return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void Tfiosetview::FillIOTableEditorRow(int Row, TIODATA *Data)
{
    if(strngrdIoTable==NULL || Data==NULL)
        return;

    strngrdIoTable->Cells[eIOTableColTag][Row]=IntToStr(Data->Tag);
    strngrdIoTable->Cells[eIOTableColType][Row]=Data->Type;
    strngrdIoTable->Cells[eIOTableColAlias][Row]=Data->Alias;
    strngrdIoTable->Cells[eIOTableColLane][Row]=IOTableCellFromInt(Data->iLane);
    strngrdIoTable->Cells[eIOTableColModuleType][Row]=IOTableCellFromInt(Data->iModuleType);
    strngrdIoTable->Cells[eIOTableColIP][Row]=IOTableIPToCell(Data);
    strngrdIoTable->Cells[eIOTableColPort][Row]=IOTablePortToCell(Data);
    strngrdIoTable->Cells[eIOTableColBit][Row]=IOTableCellFromInt(Data->iBit);
    strngrdIoTable->Cells[eIOTableColInType][Row]=IOTableCellFromInt(Data->iInType);
    strngrdIoTable->Cells[eIOTableColISABase][Row]=IOTableCellFromInt(Data->iISABase);
    strngrdIoTable->Cells[eIOTableColEnable][Row]=IOTableCellFromInt(Data->iEnable);
    strngrdIoTable->Cells[eIOTableColOnAlarmTime][Row]=IOTableCellFromInt(Data->iOnAlarmTime);
    strngrdIoTable->Cells[eIOTableColOffAlarmTime][Row]=IOTableCellFromInt(Data->iOffAlarmTime);
    strngrdIoTable->Cells[eIOTableColOnDelayTime][Row]=IOTableCellFromInt(Data->iOnDelayTime);
    strngrdIoTable->Cells[eIOTableColOffDelayTime][Row]=IOTableCellFromInt(Data->iOffDelayTime);
    strngrdIoTable->Cells[eIOTableColNote][Row]=GetIOTableNote(Data);
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::IOTableCellFromInt(int Value)
{
    if(Value==-1)
        return "";
    return IntToStr(Value);
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::IOTableIPToCell(TIODATA *Data)
{
    AnsiString Result;

    if(Data==NULL || Data->iIP==-1)
        return "";
    if(Data->iISABase==eMotionNet && Data->iIP>=10)
    {
        Result.sprintf("%c", (char)(Data->iIP-10+'A'));
        return Result;
    }
    return IntToStr(Data->iIP);
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::IOTablePortToCell(TIODATA *Data)
{
    if(Data==NULL || Data->iPort==-1)
        return "";
    if(Data->iISABase==e_PLCbase)
        return "0x"+IntToHex(Data->iPort, 3);
    return IntToStr(Data->iPort);
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::GetIOTableNote(TIODATA *Data)
{
    AnsiString Note;
    TStringList *Fields;

    Note="";
    if(Data==NULL)
        return Note;

    Fields=new TStringList();
    Fields->CommaText=Data->_CommaText;
    if(HSys.IoNo.eioNote>=0 && HSys.IoNo.eioNote<Fields->Count)
        Note=Fields->Strings[HSys.IoNo.eioNote];
    delete Fields;
    return Note;
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::GetIOTableGridCell(int Row, int Col)
{
    if(strngrdIoTable==NULL || Row<0 || Row>=strngrdIoTable->RowCount ||
       Col<0 || Col>=strngrdIoTable->ColCount)
        return "";
    return strngrdIoTable->Cells[Col][Row].Trim();
}
//---------------------------------------------------------------------------
bool Tfiosetview::IsIOTableGridRowBlank(int Row)
{
    if(strngrdIoTable==NULL || Row<=0 || Row>=strngrdIoTable->RowCount)
        return true;
    for(int Col=eIOTableColType; Col<eIOTableColTotal; Col++)
    {
        if(GetIOTableGridCell(Row, Col)!=AnsiString(""))
            return false;
    }
    return true;
}
//---------------------------------------------------------------------------
bool Tfiosetview::IsIOTableKnownType(AnsiString TypeName)
{
    TypeName=TypeName.Trim().UpperCase();
    return (IsInputIOType(TypeName) || IsOutputIOType(TypeName));
}
//---------------------------------------------------------------------------
bool Tfiosetview::IsIntegerText(AnsiString Value, bool AllowBlank)
{
    int StartIndex;

    Value=Value.Trim();
    if(Value==AnsiString(""))
        return AllowBlank;

    StartIndex=1;
    if(Value[1]=='-' || Value[1]=='+')
        StartIndex=2;
    if(StartIndex>Value.Length())
        return false;

    for(int Index=StartIndex; Index<=Value.Length(); Index++)
    {
        if(Value[Index]<'0' || Value[Index]>'9')
            return false;
    }
    return true;
}
//---------------------------------------------------------------------------
bool Tfiosetview::IsIPCellText(AnsiString Value, bool AllowBlank)
{
    AnsiString Text;

    Text=Value.Trim().UpperCase();
    if(Text==AnsiString(""))
        return AllowBlank;
    if(IsIntegerText(Text, false))
        return true;
    if(Text.Length()==1 && Text[1]>='A' && Text[1]<='Z')
        return true;
    return false;
}
//---------------------------------------------------------------------------
bool Tfiosetview::IsPortCellText(AnsiString Value, bool AllowBlank)
{
    AnsiString Text;

    Text=Value.Trim().UpperCase();
    if(Text==AnsiString(""))
        return AllowBlank;
    if(IsIntegerText(Text, false))
        return true;
    if(Text.Length()>2 && Text.SubString(1, 2)==AnsiString("0X"))
    {
        for(int Index=3; Index<=Text.Length(); Index++)
        {
            if(!((Text[Index]>='0' && Text[Index]<='9') || (Text[Index]>='A' && Text[Index]<='F')))
                return false;
        }
        return true;
    }
    return false;
}
//---------------------------------------------------------------------------
bool Tfiosetview::ValidateIOTableRow(int Row, TStringList *Errors)
{
    int OldCount;
    AnsiString TypeName;
    AnsiString AliasName;
    AnsiString EnableText;
    bool Enabled;

    if(Errors==NULL || IsIOTableGridRowBlank(Row))
        return true;

    OldCount=Errors->Count;
    TypeName=GetIOTableGridCell(Row, eIOTableColType).UpperCase();
    AliasName=GetIOTableGridCell(Row, eIOTableColAlias);
    EnableText=GetIOTableGridCell(Row, eIOTableColEnable);
    Enabled=(EnableText!=AnsiString("") && atoi(EnableText.c_str())!=0);

    if(GetIOTableGridCell(Row, eIOTableColNote).UpperCase()==AnsiString("COMM_PAD") && Enabled)
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": COMM_PAD alias must not be enabled as physical IO."));

    if(TypeName==AnsiString(""))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": Type is required."));
    else if(!IsIOTableKnownType(TypeName))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": Unknown Type ")+TypeName+AnsiString("."));

    if(AliasName==AnsiString(""))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": Alias is required."));

    if(!IsIntegerText(GetIOTableGridCell(Row, eIOTableColLane), true))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": Lane must be numeric."));
    if(!IsIntegerText(GetIOTableGridCell(Row, eIOTableColModuleType), true))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": ModuleType must be numeric."));
    if(!IsIPCellText(GetIOTableGridCell(Row, eIOTableColIP), true))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": IP must be numeric or A-Z."));
    if(!IsPortCellText(GetIOTableGridCell(Row, eIOTableColPort), true))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": Port must be numeric or hex."));
    if(!IsIntegerText(GetIOTableGridCell(Row, eIOTableColBit), true))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": Bit must be numeric."));
    if(!IsIntegerText(GetIOTableGridCell(Row, eIOTableColInType), true))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": InType must be numeric."));
    if(!IsIntegerText(GetIOTableGridCell(Row, eIOTableColISABase), true))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": ISABase must be numeric."));
    if(!IsIntegerText(EnableText, false))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": Enable must be numeric."));
    if(!IsIntegerText(GetIOTableGridCell(Row, eIOTableColOnAlarmTime), true))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": OnAlarmTime must be numeric."));
    if(!IsIntegerText(GetIOTableGridCell(Row, eIOTableColOffAlarmTime), true))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": OffAlarmTime must be numeric."));
    if(!IsIntegerText(GetIOTableGridCell(Row, eIOTableColOnDelayTime), true))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": OnDelayTime must be numeric."));
    if(!IsIntegerText(GetIOTableGridCell(Row, eIOTableColOffDelayTime), true))
        Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": OffDelayTime must be numeric."));

    if(Enabled)
    {
        if(GetIOTableGridCell(Row, eIOTableColLane)==AnsiString(""))
            Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": Enabled IO requires Lane."));
        if(GetIOTableGridCell(Row, eIOTableColIP)==AnsiString(""))
            Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": Enabled IO requires IP."));
        if(GetIOTableGridCell(Row, eIOTableColPort)==AnsiString(""))
            Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": Enabled IO requires Port."));
        if(GetIOTableGridCell(Row, eIOTableColBit)==AnsiString(""))
            Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(": Enabled IO requires Bit."));
    }

    return (Errors->Count==OldCount);
}
//---------------------------------------------------------------------------
bool Tfiosetview::ValidateIOTableGrid(TStringList *Errors)
{
    bool Result=true;

    if(Errors==NULL || strngrdIoTable==NULL)
        return false;

    for(int Row=1; Row<strngrdIoTable->RowCount; Row++)
    {
        AnsiString AliasName;

        if(IsIOTableGridRowBlank(Row))
            continue;
        if(!ValidateIOTableRow(Row, Errors))
            Result=false;

        AliasName=GetIOTableGridCell(Row, eIOTableColAlias).UpperCase();
        if(AliasName==AnsiString(""))
            continue;
        for(int CheckRow=Row+1; CheckRow<strngrdIoTable->RowCount; CheckRow++)
        {
            if(IsIOTableGridRowBlank(CheckRow))
                continue;
            if(AliasName==GetIOTableGridCell(CheckRow, eIOTableColAlias).UpperCase())
            {
                Errors->Add(AnsiString("Row ")+IntToStr(Row)+AnsiString(" and Row ")+IntToStr(CheckRow)+AnsiString(": duplicate Alias ")+AliasName+AnsiString("."));
                Result=false;
                break;
            }
        }
    }

    return Result;
}
//---------------------------------------------------------------------------
bool Tfiosetview::BackupIOTableFile(AnsiString *BackupFile)
{
    AnsiString BackupDir;
    AnsiString BackupName;

    if(BackupFile!=NULL)
        *BackupFile=AnsiString("");
    if(HSys.IoTablePath==AnsiString("") || !FileExists(HSys.IoTablePath))
        return true;

    BackupDir=ExtractFilePath(HSys.IoTablePath)+AnsiString("backup");
    if(!DirectoryExists(BackupDir) && !ForceDirectories(BackupDir))
        return false;

    BackupName=BackupDir+AnsiString("\\IO_Table_")+FormatDateTime("yyyymmdd_hhnnss", Now())+AnsiString(".csv");
    if(!CopyFile(HSys.IoTablePath.c_str(), BackupName.c_str(), false))
        return false;

    if(BackupFile!=NULL)
        *BackupFile=BackupName;
    return true;
}
//---------------------------------------------------------------------------
int Tfiosetview::FindIOTableGridRowByTag(int Tag)
{
    if(strngrdIoTable==NULL)
        return -1;
    for(int Row=1; Row<strngrdIoTable->RowCount; Row++)
    {
        if(GetIOTableGridCell(Row, eIOTableColTag)!=AnsiString("") &&
           atoi(GetIOTableGridCell(Row, eIOTableColTag).c_str())==Tag)
            return Row;
    }
    return -1;
}
//---------------------------------------------------------------------------
void Tfiosetview::AppendIOTableDataToCsv(TStringList *Lines, TStringList *Fields, TIODATA *Data)
{
    if(Lines==NULL || Fields==NULL || Data==NULL)
        return;
    Fields->Clear();
    Fields->Add(Data->Type);
    Fields->Add(Data->Alias);
    Fields->Add(IOTableCellFromInt(Data->iLane));
    Fields->Add(IOTableCellFromInt(Data->iModuleType));
    Fields->Add(IOTableIPToCell(Data));
    Fields->Add(IOTablePortToCell(Data));
    Fields->Add(IOTableCellFromInt(Data->iBit));
    Fields->Add(IOTableCellFromInt(Data->iInType));
    Fields->Add(IOTableCellFromInt(Data->iISABase));
    Fields->Add(IOTableCellFromInt(Data->iEnable));
    Fields->Add(IOTableCellFromInt(Data->iOnAlarmTime));
    Fields->Add(IOTableCellFromInt(Data->iOffAlarmTime));
    Fields->Add(IOTableCellFromInt(Data->iOnDelayTime));
    Fields->Add(IOTableCellFromInt(Data->iOffDelayTime));
    Fields->Add(GetIOTableNote(Data));
    Lines->Add(Fields->CommaText);
}
//---------------------------------------------------------------------------
void Tfiosetview::AppendIOTableGridRowToCsv(TStringList *Lines, TStringList *Fields, int Row)
{
    if(Lines==NULL || Fields==NULL || strngrdIoTable==NULL || Row<=0 || Row>=strngrdIoTable->RowCount)
        return;
    Fields->Clear();
    for(int Col=eIOTableColType; Col<eIOTableColTotal; Col++)
        Fields->Add(GetIOTableGridCell(Row, Col));
    Lines->Add(Fields->CommaText);
}
//---------------------------------------------------------------------------
void Tfiosetview::SaveIoTableFromGrid()
{
    TStringList *Lines;
    TStringList *Fields;
    TStringList *Errors;
    AnsiString ErrorMessage;
    AnsiString BackupFile;
    int Count;

    if(strngrdIoTable==NULL)
        return;

    Errors=new TStringList();
    if(!ValidateIOTableGrid(Errors))
    {
        ErrorMessage="IO table data invalid:";
        for(int Index=0; Index<Errors->Count && Index<20; Index++)
            ErrorMessage=ErrorMessage+AnsiString("\r\n")+Errors->Strings[Index];
        if(Errors->Count>20)
            ErrorMessage=ErrorMessage+AnsiString("\r\n...");
        ShowMessage(ErrorMessage);
        delete Errors;
        return;
    }
    delete Errors;

    BackupFile="";
    if(!BackupIOTableFile(&BackupFile))
    {
        ShowMessage("IO_Table.csv backup failed. Save aborted.");
        return;
    }

    Lines=new TStringList();
    Fields=new TStringList();
    try
    {
        Lines->Add("IOType,Alias,Lane,ModuleType,IP,Port,Bit,InType,ISABase,Enable,OnAlarmTime,OffAlarmTime,OnDelayTime,OffDelayTime,Note");
        Count=(HSys.IOTable==NULL)?0:HSys.IOTable->Count;
        for(int Index=0; Index<Count; Index++)
        {
            TIODATA *Data=(TIODATA *)HSys.IOTable->Items[Index];
            int Row;

            if(Data==NULL)
                continue;
            if(IOTableDeletedTags!=NULL && IOTableDeletedTags->IndexOf(IntToStr(Data->Tag))>=0)
                continue;
            Row=FindIOTableGridRowByTag(Data->Tag);
            if(Row>0 && !IsIOTableGridRowBlank(Row))
                AppendIOTableGridRowToCsv(Lines, Fields, Row);
            else
                AppendIOTableDataToCsv(Lines, Fields, Data);
        }

        for(int Row=1; Row<strngrdIoTable->RowCount; Row++)
        {
            if(GetIOTableGridCell(Row, eIOTableColTag)==AnsiString("") && !IsIOTableGridRowBlank(Row))
                AppendIOTableGridRowToCsv(Lines, Fields, Row);
        }

        Lines->SaveToFile(HSys.IoTablePath);
        HSys.LoadIoData();
        if(IOTableDeletedTags!=NULL)
            IOTableDeletedTags->Clear();
        LoadIoTable((cbbType==NULL)?0:cbbType->ItemIndex, (cbbLane==NULL)?0:cbbLane->ItemIndex, 0);
        RefreshLegacyIOControls();
        RefreshLegacyIOMaps();
        if(BackupFile!=AnsiString(""))
            ShowMessage(AnsiString("IO_Table.csv saved.\r\nBackup: ")+BackupFile);
        else
            ShowMessage("IO_Table.csv saved.");
    }
    catch(...)
    {
        ShowMessage("IO_Table.csv save failed.");
    }

    delete Fields;
    delete Lines;
}
//---------------------------------------------------------------------------
void Tfiosetview::RefreshAll()
{
    RefreshSummary();
    RefreshSensors();
    RefreshCylinders();
    RefreshSwitches();
    RefreshSuckers();
    RefreshIOTable();
    RefreshLegacyIOControls();
    UpdateSelectedInfo();
    UpdateManualButtons();
}
//---------------------------------------------------------------------------
void Tfiosetview::RefreshCurrentView()
{
    if(lblSummary!=NULL && grdSensors!=NULL && grdCylinders!=NULL &&
       grdSwitches!=NULL && grdSuckers!=NULL && grdIOTable!=NULL)
    {
        RefreshAll();
        return;
    }

    RefreshLegacyIOControls();
}
//---------------------------------------------------------------------------
void Tfiosetview::RefreshLegacyIOControls()
{
    for(int ComponentIndex=0; ComponentIndex<ComponentCount; ComponentIndex++)
    {
        TComponent *ComponentPtr=Components[ComponentIndex];
        TMyLed *LedPtr=dynamic_cast<TMyLed *>(ComponentPtr);
        TBtnPanel *ButtonPtr=dynamic_cast<TBtnPanel *>(ComponentPtr);
        bool State=false;

        if(LedPtr!=NULL)
        {
            if(LedPtr->Alias==AnsiString(""))
                continue;
            if(ResolveLegacyLedState(LedPtr->Alias, &State))
            {
                LedPtr->TrueColor=clLime;
                LedPtr->FalseColor=clSilver;
                LedPtr->Value=State;
            }
            else
            {
                LedPtr->TrueColor=clRed;
                LedPtr->FalseColor=clRed;
                LedPtr->Value=false;
            }
        }
        else if(ButtonPtr!=NULL)
        {
            if(ButtonPtr->Alias==AnsiString(""))
                continue;
            if(ResolveLegacyButtonState(ButtonPtr->Alias, &State))
            {
                ButtonPtr->TrueColor=IO_COLOR_BUTTON_ON;
                ButtonPtr->FalseColor=IO_COLOR_BUTTON_OFF;
                ButtonPtr->TrueFontColor=clBlack;
                ButtonPtr->FalseFontColor=clWhite;
                ButtonPtr->Down=State;
            }
            else
            {
                ButtonPtr->TrueColor=clRed;
                ButtonPtr->FalseColor=clRed;
                ButtonPtr->TrueFontColor=clWhite;
                ButtonPtr->FalseFontColor=clWhite;
                ButtonPtr->Down=false;
            }
        }
    }
}
//---------------------------------------------------------------------------
void Tfiosetview::RefreshLegacyIOMaps()
{
    SetLegacyComponentHints();
    ShowInputInformation();
    ShowOutputInformation();
    AppendLegacyIODiagnostics();
}
//---------------------------------------------------------------------------
bool Tfiosetview::IsInputIOType(AnsiString TypeName)
{
    TypeName=TypeName.UpperCase();
    return (TypeName==AnsiString("SENSOR") ||
            TypeName==AnsiString("CYLINDER_ON") ||
            TypeName==AnsiString("CYLINDER_OFF") ||
            TypeName==AnsiString("SUCKER"));
}
//---------------------------------------------------------------------------
bool Tfiosetview::IsOutputIOType(AnsiString TypeName)
{
    TypeName=TypeName.UpperCase();
    return (TypeName==AnsiString("SWITCH") ||
            TypeName==AnsiString("CYLINDER") ||
            TypeName==AnsiString("SUCKER_ON") ||
            TypeName==AnsiString("SUCKER_OFF"));
}
//---------------------------------------------------------------------------
void Tfiosetview::CollectLegacyComponentDiagnostics(TWinControl *PCtrl, TStringList *Lines, int *UnboundInput, int *UnboundOutput)
{
    if(PCtrl==NULL)
        return;

    for(int Index=0; Index<PCtrl->ControlCount; Index++)
    {
        TControl *ControlPtr=PCtrl->Controls[Index];
        TWinControl *WinPtr=dynamic_cast<TWinControl *>(ControlPtr);
        TMyLed *LedPtr=dynamic_cast<TMyLed *>(ControlPtr);
        TBtnPanel *ButtonPtr=dynamic_cast<TBtnPanel *>(ControlPtr);
        AnsiString AliasName;

        if(WinPtr!=NULL)
            CollectLegacyComponentDiagnostics(WinPtr, Lines, UnboundInput, UnboundOutput);

        if(LedPtr!=NULL)
        {
            AliasName=LedPtr->Alias;
            if(AliasName!=AnsiString("") && FindLegacyIODataByAlias(AliasName, true)==NULL)
            {
                if(UnboundInput!=NULL)
                    (*UnboundInput)++;
                if(Lines!=NULL && Lines->Count<40)
                    Lines->Add(AnsiString("Unbound input: ")+AliasName);
            }
        }
        else if(ButtonPtr!=NULL)
        {
            AliasName=ButtonPtr->Alias;
            if(AliasName!=AnsiString("") && FindLegacyIODataByAlias(AliasName, false)==NULL)
            {
                if(UnboundOutput!=NULL)
                    (*UnboundOutput)++;
                if(Lines!=NULL && Lines->Count<40)
                    Lines->Add(AnsiString("Unbound output: ")+AliasName);
            }
        }
    }
}
//---------------------------------------------------------------------------
void Tfiosetview::AppendLegacyIODiagnostics()
{
    TMemo *MemoPtr;
    TStringList *UnboundLines;
    TStringList *DisabledLines;
    int UnboundInput;
    int UnboundOutput;
    int DisabledCount;
    int PadCommCount;

    MemoPtr=dynamic_cast<TMemo *>(FindComponent("MemoIOMap"));
    if(MemoPtr==NULL)
        return;

    UnboundLines=new TStringList();
    DisabledLines=new TStringList();
    UnboundInput=0;
    UnboundOutput=0;
    DisabledCount=0;
    PadCommCount=0;

    CollectLegacyComponentDiagnostics(this, UnboundLines, &UnboundInput, &UnboundOutput);
    if(HSys.IOTable!=NULL)
    {
        for(int Index=0; Index<HSys.IOTable->Count; Index++)
        {
            TIODATA *Data=(TIODATA *)HSys.IOTable->Items[Index];
            if(Data==NULL || Data->Alias==AnsiString("") || Data->iEnable!=0)
                continue;
            if(IsPadCommunicationData(Data))
            {
                PadCommCount++;
                continue;
            }
            DisabledCount++;
            if(DisabledLines->Count<30)
                DisabledLines->Add(AnsiString("Disabled IO: ")+Data->Alias+AnsiString(" [")+Data->Type+AnsiString("]"));
        }
    }

    MemoPtr->Lines->Add(AnsiString("Summary: unbound input=")+IntToStr(UnboundInput)+
                        AnsiString(", unbound output=")+IntToStr(UnboundOutput)+
                        AnsiString(", disabled IO=")+IntToStr(DisabledCount)+
                        AnsiString(", Pad COM alias=")+IntToStr(PadCommCount));
    for(int Index=0; Index<UnboundLines->Count; Index++)
        MemoPtr->Lines->Add(UnboundLines->Strings[Index]);
    if(UnboundInput+UnboundOutput>UnboundLines->Count)
        MemoPtr->Lines->Add("Unbound list truncated.");
    for(int Index=0; Index<DisabledLines->Count; Index++)
        MemoPtr->Lines->Add(DisabledLines->Strings[Index]);
    if(DisabledCount>DisabledLines->Count)
        MemoPtr->Lines->Add("Disabled IO list truncated.");
    AppendManualOutputLogToMemo(MemoPtr);

    delete DisabledLines;
    delete UnboundLines;
}
//---------------------------------------------------------------------------
void Tfiosetview::AppendManualOutputLogToMemo(TMemo *MemoPtr)
{
    if(MemoPtr==NULL || ManualOutputLog==NULL || ManualOutputLog->Count<=0)
        return;

    MemoPtr->Lines->Add("Manual output log:");
    for(int Index=0; Index<ManualOutputLog->Count; Index++)
        MemoPtr->Lines->Add(ManualOutputLog->Strings[Index]);
}
//---------------------------------------------------------------------------
void Tfiosetview::LogManualOutputAction(AnsiString TargetName, AnsiString ActionName, AnsiString ResultText)
{
    TMemo *MemoPtr;
    AnsiString LineText;

    if(TargetName==AnsiString(""))
        TargetName="(none)";
    if(ActionName==AnsiString(""))
        ActionName="ACTION";
    if(ResultText==AnsiString(""))
        ResultText="OK";

    LineText=FormatDateTime("yyyy-mm-dd hh:nn:ss", Now())+
             AnsiString("  ")+ActionName+
             AnsiString("  ")+TargetName+
             AnsiString("  ")+ResultText;

    if(ManualOutputLog!=NULL)
    {
        ManualOutputLog->Add(LineText);
        while(ManualOutputLog->Count>80)
            ManualOutputLog->Delete(0);
    }

    MemoPtr=dynamic_cast<TMemo *>(FindComponent("MemoIOMap"));
    if(MemoPtr!=NULL)
    {
        MemoPtr->Lines->Add(LineText);
        while(MemoPtr->Lines->Count>1000)
            MemoPtr->Lines->Delete(0);
    }
}
//---------------------------------------------------------------------------
void Tfiosetview::SyncPadSwitchStatus(AnsiString SwitchName, bool State)
{
    if(fPadInterface==NULL || SwitchName==AnsiString(""))
        return;
    if(!fPadInterface->IsPadButton(SwitchName))
        return;

    fPadInterface->SendSwitchStatus(SwitchName, State);
}
//---------------------------------------------------------------------------
bool Tfiosetview::IsValidMapIOData(TIODATA *Data)
{
    return (Data!=NULL && Data->Alias!=AnsiString("") && Data->iEnable!=0 && !IsPadCommunicationData(Data) &&
            Data->iPort>=0 && Data->iBit>=0);
}
//---------------------------------------------------------------------------
bool Tfiosetview::IsPadCommunicationData(TIODATA *Data)
{
    if(Data==NULL)
        return false;
    return (GetIOTableNote(Data).Trim().UpperCase()==AnsiString("COMM_PAD"));
}
//---------------------------------------------------------------------------
bool Tfiosetview::IsPadCommunicationInput(AnsiString AliasName)
{
    TIODATA *Data;

    if(AliasName==AnsiString(""))
        return false;
    if(fPadInterface!=NULL && fPadInterface->IsPadKey(AliasName))
        return true;
    Data=FindLegacyIODataByAlias(AliasName, true);
    return IsPadCommunicationData(Data);
}
//---------------------------------------------------------------------------
bool Tfiosetview::IsPadCommunicationOutput(AnsiString AliasName)
{
    TIODATA *Data;

    if(AliasName==AnsiString(""))
        return false;
    if(fPadInterface!=NULL && fPadInterface->IsPadButton(AliasName))
        return true;
    Data=FindLegacyIODataByAlias(AliasName, false);
    return IsPadCommunicationData(Data);
}
//---------------------------------------------------------------------------
bool Tfiosetview::ResolvePadCommunicationInputState(AnsiString AliasName, bool *State)
{
    if(State!=NULL)
        *State=false;
    if(!IsPadCommunicationInput(AliasName))
        return false;
    if(fPadInterface!=NULL && fPadInterface->IsPadKey(AliasName))
    {
        if(State!=NULL)
            *State=fPadInterface->ProcessScanKey(AliasName);
    }
    return true;
}
//---------------------------------------------------------------------------
bool Tfiosetview::ResolvePadCommunicationOutputState(AnsiString AliasName, bool *State)
{
    if(State!=NULL)
        *State=false;
    if(!IsPadCommunicationOutput(AliasName))
        return false;
    if(fPadInterface!=NULL && fPadInterface->GetPadSwitchStatus(AliasName, State))
        return true;
    for(int SwitchIndex=0; SwitchIndex<HSys.iTotalSwitch; SwitchIndex++)
    {
        TMySwitch *SwitchPtr=&HSys.SwPtr[SwitchIndex];
        if(AliasName==SwitchPtr->Name)
        {
            if(State!=NULL)
                *State=SwitchPtr->Status();
            return true;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
TIODATA *Tfiosetview::FindLegacyIODataByAlias(AnsiString AliasName, bool InputSide)
{
    if(HSys.IOTable==NULL || AliasName==AnsiString(""))
        return NULL;

    for(int Index=0; Index<HSys.IOTable->Count; Index++)
    {
        TIODATA *Data=(TIODATA *)HSys.IOTable->Items[Index];
        if(Data==NULL || Data->Alias!=AliasName)
            continue;
        if(InputSide && IsInputIOType(Data->Type))
            return Data;
        if(!InputSide && IsOutputIOType(Data->Type))
            return Data;
    }
    return NULL;
}
//---------------------------------------------------------------------------
AnsiString Tfiosetview::FormatMapAddress(TIODATA *Data)
{
    if(Data==NULL)
        return AnsiString("");

    if(IsPadCommunicationData(Data))
        return AnsiString("Pad COM");

    return AnsiString("Lane=")+IOTableCellFromInt(Data->iLane)+
           AnsiString(" IP=")+IOTableIPToCell(Data)+
           AnsiString(" Port=")+IOTablePortToCell(Data)+
           AnsiString(" Bit=")+IOTableCellFromInt(Data->iBit);
}
//---------------------------------------------------------------------------
void Tfiosetview::SetLegacyComponentHints()
{
    SetLegacyComponentHints(this);
}
//---------------------------------------------------------------------------
void Tfiosetview::SetLegacyComponentHints(TWinControl *PCtrl)
{
    if(PCtrl==NULL)
        return;

    for(int Index=0; Index<PCtrl->ControlCount; Index++)
    {
        TControl *ControlPtr=PCtrl->Controls[Index];
        TWinControl *WinPtr=dynamic_cast<TWinControl *>(ControlPtr);
        TMyLed *LedPtr=dynamic_cast<TMyLed *>(ControlPtr);
        TBtnPanel *ButtonPtr=dynamic_cast<TBtnPanel *>(ControlPtr);
        AnsiString AliasName;
        TIODATA *Data=NULL;

        if(WinPtr!=NULL)
            SetLegacyComponentHints(WinPtr);

        if(LedPtr!=NULL)
        {
            AliasName=LedPtr->Alias;
            if(AliasName==AnsiString(""))
                continue;
            Data=FindLegacyIODataByAlias(AliasName, true);
            LedPtr->ShowHint=true;
            if(Data!=NULL)
                LedPtr->Hint=AnsiString("(")+FormatMapAddress(Data)+AnsiString(") ")+Data->Alias+AnsiString(" [")+Data->Type+AnsiString("]");
            else
                LedPtr->Hint=AnsiString("(unbound) ")+AliasName;
        }
        else if(ButtonPtr!=NULL)
        {
            AliasName=ButtonPtr->Alias;
            if(AliasName==AnsiString(""))
                continue;
            Data=FindLegacyIODataByAlias(AliasName, false);
            ButtonPtr->ShowHint=true;
            if(Data!=NULL)
                ButtonPtr->Hint=AnsiString("(")+FormatMapAddress(Data)+AnsiString(") ")+Data->Alias+AnsiString(" [")+Data->Type+AnsiString("]");
            else
                ButtonPtr->Hint=AnsiString("(unbound) ")+AliasName;
        }
    }
}
//---------------------------------------------------------------------------
void Tfiosetview::FillIOMapGrid(TStringGrid *Grid, bool InputSide)
{
    TList *Items;
    TMemo *MemoPtr;
    AnsiString SideName;

    if(Grid==NULL)
        return;

    Grid->ColCount=6;
    Grid->FixedRows=1;
    Grid->FixedCols=0;
    Grid->Cells[0][0]="Lane";
    Grid->Cells[1][0]="IP";
    Grid->Cells[2][0]="Port";
    Grid->Cells[3][0]="Bit";
    Grid->Cells[4][0]="Type";
    Grid->Cells[5][0]="Alias";
    Grid->ColWidths[0]=45;
    Grid->ColWidths[1]=45;
    Grid->ColWidths[2]=55;
    Grid->ColWidths[3]=40;
    Grid->ColWidths[4]=95;
    Grid->ColWidths[5]=210;

    Items=new TList;
    if(HSys.IOTable!=NULL)
    {
        for(int Index=0; Index<HSys.IOTable->Count; Index++)
        {
            TIODATA *Data=(TIODATA *)HSys.IOTable->Items[Index];
            if(!IsValidMapIOData(Data))
                continue;
            if(InputSide && !IsInputIOType(Data->Type))
                continue;
            if(!InputSide && !IsOutputIOType(Data->Type))
                continue;

            TIOMapItem *Item=new TIOMapItem;
            Item->Lane=Data->iLane;
            Item->IP=Data->iIP;
            Item->Port=Data->iPort;
            Item->Bit=Data->iBit;
            Item->Type=Data->Type;
            Item->Alias=Data->Alias;
            Items->Add(Item);
        }
    }

    for(int Left=0; Left<Items->Count-1; Left++)
    {
        for(int Right=Left+1; Right<Items->Count; Right++)
        {
            TIOMapItem *L=(TIOMapItem *)Items->Items[Left];
            TIOMapItem *R=(TIOMapItem *)Items->Items[Right];
            bool Swap=false;
            if(L->Lane>R->Lane)
                Swap=true;
            else if(L->Lane==R->Lane && L->IP>R->IP)
                Swap=true;
            else if(L->Lane==R->Lane && L->IP==R->IP && L->Port>R->Port)
                Swap=true;
            else if(L->Lane==R->Lane && L->IP==R->IP && L->Port==R->Port && L->Bit>R->Bit)
                Swap=true;
            else if(L->Lane==R->Lane && L->IP==R->IP && L->Port==R->Port && L->Bit==R->Bit && L->Alias.AnsiCompare(R->Alias)>0)
                Swap=true;
            if(Swap)
                Items->Exchange(Left, Right);
        }
    }

    SetGridRowCount(Grid, Items->Count+1);
    ClearGridRows(Grid);
    for(int Index=0; Index<Items->Count; Index++)
    {
        TIOMapItem *Item=(TIOMapItem *)Items->Items[Index];
        Grid->Cells[0][Index+1]=IntToStr(Item->Lane);
        Grid->Cells[1][Index+1]=IntToStr(Item->IP);
        Grid->Cells[2][Index+1]=IntToStr(Item->Port);
        Grid->Cells[3][Index+1]=IntToStr(Item->Bit);
        Grid->Cells[4][Index+1]=Item->Type;
        Grid->Cells[5][Index+1]=Item->Alias;
    }

    MemoPtr=dynamic_cast<TMemo *>(FindComponent("MemoIOMap"));
    SideName=InputSide?AnsiString("Input"):AnsiString("Output");
    if(MemoPtr!=NULL)
    {
        if(InputSide)
            MemoPtr->Lines->Clear();
        for(int Index=0; Index<Items->Count; Index++)
        {
            TIOMapItem *Item=(TIOMapItem *)Items->Items[Index];
            for(int PrevIndex=0; PrevIndex<Index; PrevIndex++)
            {
                TIOMapItem *Prev=(TIOMapItem *)Items->Items[PrevIndex];
                if(Item->Lane==Prev->Lane && Item->IP==Prev->IP && Item->Port==Prev->Port && Item->Bit==Prev->Bit)
                {
                    MemoPtr->Lines->Add(SideName+AnsiString(" repeat: ")+Prev->Alias+AnsiString(" / ")+Item->Alias+AnsiString(" @ ")+IntToStr(Item->Lane)+AnsiString("-")+IntToStr(Item->IP)+AnsiString("-")+IntToStr(Item->Port)+AnsiString("-")+IntToStr(Item->Bit));
                    break;
                }
            }
        }
    }

    for(int Index=Items->Count-1; Index>=0; Index--)
        delete (TIOMapItem *)Items->Items[Index];
    delete Items;
}
//---------------------------------------------------------------------------
void Tfiosetview::ShowInputInformation()
{
    TStringGrid *Grid=dynamic_cast<TStringGrid *>(FindComponent("InputInformationGrid"));
    FillIOMapGrid(Grid, true);
}
//---------------------------------------------------------------------------
void Tfiosetview::ShowOutputInformation()
{
    TStringGrid *Grid=dynamic_cast<TStringGrid *>(FindComponent("OutputInformationGrid"));
    FillIOMapGrid(Grid, false);
}
//---------------------------------------------------------------------------
void Tfiosetview::SaveIOMapGrid(TStringGrid *Grid, AnsiString FileName)
{
    TStringList *Lines;
    TStringList *Fields;

    if(Grid==NULL || FileName==AnsiString(""))
        return;

    Lines=new TStringList;
    Fields=new TStringList;
    for(int Row=0; Row<Grid->RowCount; Row++)
    {
        bool HasText=false;
        Fields->Clear();
        for(int Col=0; Col<Grid->ColCount; Col++)
        {
            if(Grid->Cells[Col][Row]!=AnsiString(""))
                HasText=true;
            Fields->Add(Grid->Cells[Col][Row]);
        }
        if(Row==0 || HasText)
            Lines->Add(Fields->CommaText);
    }
    Lines->SaveToFile(FileName);
    delete Fields;
    delete Lines;
}
//---------------------------------------------------------------------------
void Tfiosetview::SaveIOMap(bool InputSide)
{
    TStringGrid *Grid;
    AnsiString DirName;
    AnsiString FileName;

    Grid=dynamic_cast<TStringGrid *>(FindComponent(InputSide?"InputInformationGrid":"OutputInformationGrid"));
    if(Grid==NULL)
    {
        ShowMessage("IO map grid not found.");
        return;
    }

    FillIOMapGrid(Grid, InputSide);
    DirName=HSys.CurrentDir+AnsiString("\\io_map");
    if(!DirectoryExists(DirName))
        ForceDirectories(DirName);
    FileName=DirName+(InputSide?AnsiString("\\InputMap.csv"):AnsiString("\\OutputMap.csv"));
    SaveIOMapGrid(Grid, FileName);
    ShowMessage(AnsiString("Saved ")+FileName);
}
//---------------------------------------------------------------------------
bool Tfiosetview::ResolveLegacyLedState(AnsiString AliasName, bool *State)
{
    if(State!=NULL)
        *State=false;
    if(AliasName==AnsiString(""))
        return false;

    if(ResolvePadCommunicationInputState(AliasName, State))
        return true;

    if(HSys.IOTable!=NULL)
    {
        for(int IOIndex=0; IOIndex<HSys.IOTable->Count; IOIndex++)
        {
            TIODATA *Data=(TIODATA *)HSys.IOTable->Items[IOIndex];
            if(Data==NULL || Data->Type.UpperCase()!=AnsiString("SENSOR"))
                continue;
            if(Data->Alias!=AliasName)
                continue;

            TMyIo TempIO;
            TempIO.iCard=Data->iLane*100+Data->iIP;
            TempIO.iLane=Data->iLane;
            TempIO.iIP=Data->iIP;
            TempIO.iPort=Data->iPort;
            TempIO.iBit=Data->iBit;
            TempIO.ISABase=Data->iISABase;

            bool RawOn=TempIO.IsOn();
            if(Data->iEnable==0)
                return false;
            if(State!=NULL)
                *State=(Data->iInType==1)?RawOn:!RawOn;
            return true;
        }
    }

    for(int CylinderIndex=0; CylinderIndex<HSys.iTotalCylinder; CylinderIndex++)
    {
        TMyCylinder *CylinderPtr=&HSys.CynPtr[CylinderIndex];
        if(AliasName==CylinderPtr->CylinderName+AnsiString("_On"))
        {
            if(CylinderPtr->OnSensor.Enable==false)
                return false;
            if(State!=NULL)
                *State=CylinderPtr->IsOn();
            return true;
        }
        if(AliasName==CylinderPtr->CylinderName+AnsiString("_Off"))
        {
            if(CylinderPtr->OffSensor.Enable==false)
                return false;
            if(State!=NULL)
                *State=CylinderPtr->IsOff();
            return true;
        }
    }

    for(int SuckerIndex=0; SuckerIndex<HSys.iTotalSucker; SuckerIndex++)
    {
        TMyKitSuck *Kit=&HSys.SuckPtr[SuckerIndex];
        for(int RowIndex=0; RowIndex<Kit->MaxItemR; RowIndex++)
        {
            for(int ColIndex=0; ColIndex<Kit->MaxItemC; ColIndex++)
            {
                TMySucker *SuckerPtr=&Kit->Suck[RowIndex][ColIndex];
                if(AliasName==SuckerPtr->SensorName || AliasName==SuckerPtr->SuckerName)
                {
                    if(SuckerPtr->Sensor.Enable==false)
                        return false;
                    if(State!=NULL)
                        *State=SuckerPtr->GetStatus();
                    return true;
                }
                if(AliasName==SuckerPtr->OnPortName || AliasName==SuckerPtr->OnPortName+AnsiString("_On"))
                {
                    if(SuckerPtr->OnSw.Enable==false)
                        return false;
                    if(State!=NULL)
                        *State=SuckerPtr->GetOnBit();
                    return true;
                }
                if(AliasName==SuckerPtr->OffPortName || AliasName==SuckerPtr->OffPortName+AnsiString("_Off"))
                {
                    if(SuckerPtr->OffSw.Enable==false)
                        return false;
                    if(State!=NULL)
                        *State=SuckerPtr->GetOffBit();
                    return true;
                }
            }
        }
    }

    return false;
}
//---------------------------------------------------------------------------
bool Tfiosetview::ResolveLegacyButtonState(AnsiString AliasName, bool *State)
{
    if(State!=NULL)
        *State=false;
    if(AliasName==AnsiString(""))
        return false;

    if(ResolvePadCommunicationOutputState(AliasName, State))
        return true;

    for(int SwitchIndex=0; SwitchIndex<HSys.iTotalSwitch; SwitchIndex++)
    {
        TMySwitch *SwitchPtr=&HSys.SwPtr[SwitchIndex];
        if(AliasName==SwitchPtr->Name)
        {
            if(SwitchPtr->Enable==false)
                return false;
            if(State!=NULL)
                *State=SwitchPtr->Status();
            return true;
        }
    }

    for(int CylinderIndex=0; CylinderIndex<HSys.iTotalCylinder; CylinderIndex++)
    {
        TMyCylinder *CylinderPtr=&HSys.CynPtr[CylinderIndex];
        if(AliasName==CylinderPtr->CylinderName)
        {
            if(CylinderPtr->Switch.Enable==false)
                return false;
            if(State!=NULL)
                *State=CylinderPtr->GetOutBit();
            return true;
        }
        if(TextEndsWith(AliasName, AnsiString("_On")) && AliasName==CylinderPtr->CylinderName+AnsiString("_On"))
        {
            if(CylinderPtr->OnSensor.Enable==false)
                return false;
            if(State!=NULL)
                *State=CylinderPtr->IsOn();
            return true;
        }
        if(TextEndsWith(AliasName, AnsiString("_Off")) && AliasName==CylinderPtr->CylinderName+AnsiString("_Off"))
        {
            if(CylinderPtr->OffSensor.Enable==false)
                return false;
            if(State!=NULL)
                *State=CylinderPtr->IsOff();
            return true;
        }
    }

    for(int SuckerIndex=0; SuckerIndex<HSys.iTotalSucker; SuckerIndex++)
    {
        TMyKitSuck *Kit=&HSys.SuckPtr[SuckerIndex];
        for(int RowIndex=0; RowIndex<Kit->MaxItemR; RowIndex++)
        {
            for(int ColIndex=0; ColIndex<Kit->MaxItemC; ColIndex++)
            {
                TMySucker *SuckerPtr=&Kit->Suck[RowIndex][ColIndex];
                if(AliasName==SuckerPtr->OnPortName)
                {
                    if(SuckerPtr->OnSw.Enable==false)
                        return false;
                    if(State!=NULL)
                        *State=SuckerPtr->GetOnBit();
                    return true;
                }
                if(AliasName==SuckerPtr->OffPortName)
                {
                    if(SuckerPtr->OffSw.Enable==false)
                        return false;
                    if(State!=NULL)
                        *State=SuckerPtr->GetOffBit();
                    return true;
                }
            }
        }
    }

    return false;
}
//---------------------------------------------------------------------------
bool Tfiosetview::IsManualOutputEnabled()
{
    return (chkManualOutput!=NULL && chkManualOutput->Checked==true);
}
//---------------------------------------------------------------------------
bool Tfiosetview::CanManualOutput(AnsiString *Reason)
{
    if(Reason!=NULL)
        *Reason=AnsiString("");

    if(HSys.Sys.SystemStart)
    {
        if(Reason!=NULL)
            *Reason="Manual output is blocked while machine is running.";
        return false;
    }

    if(IsSafeDoorOpen()>0)
    {
        if(Reason!=NULL)
            *Reason="Manual output is blocked while safe door is open.";
        return false;
    }

    if(!IsManualOutputEnabled())
    {
        if(Reason!=NULL)
            *Reason="Manual output is locked.";
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
bool Tfiosetview::CanLegacyManualOutput(AnsiString *Reason)
{
    if(!CanManualOutput(Reason))
        return false;

    if(!IsLegacyGeneralIOMode())
    {
        if(Reason!=NULL)
            *Reason="General IO mode is required for manual output.";
        return false;
    }

    return true;
}
//---------------------------------------------------------------------------
void Tfiosetview::ShowManualOutputBlocked(AnsiString Reason)
{
    if(Reason==AnsiString(""))
        Reason="Manual output is blocked.";
    LogManualOutputAction("ManualOutput", "BLOCK", Reason);
    ShowMessage(Reason);
}
//---------------------------------------------------------------------------
bool Tfiosetview::GetSelectedSwitch(TMySwitch **SwitchPtr)
{
    if(SwitchPtr!=NULL)
        *SwitchPtr=NULL;
    if(SelectedKind!=eIOViewSwitch || SelectedIndex<0 || SelectedIndex>=HSys.iTotalSwitch)
        return false;
    if(SwitchPtr!=NULL)
        *SwitchPtr=&HSys.SwPtr[SelectedIndex];
    return true;
}
//---------------------------------------------------------------------------
bool Tfiosetview::GetSelectedCylinder(TMyCylinder **CylinderPtr)
{
    if(CylinderPtr!=NULL)
        *CylinderPtr=NULL;
    if(SelectedKind!=eIOViewCylinder || SelectedIndex<0 || SelectedIndex>=HSys.iTotalCylinder)
        return false;
    if(CylinderPtr!=NULL)
        *CylinderPtr=&HSys.CynPtr[SelectedIndex];
    return true;
}
//---------------------------------------------------------------------------
bool Tfiosetview::GetSelectedSucker(TMySucker **SuckerPtr)
{
    if(SuckerPtr!=NULL)
        *SuckerPtr=NULL;
    if(SelectedKind!=eIOViewSucker || SelectedIndex<0 || HSys.iTotalSucker<=0)
        return false;
    TMyKitSuck *Kit=&HSys.SuckPtr[0];
    int Total=Kit->MaxItemR*Kit->MaxItemC;
    if(SelectedIndex>=Total || Kit->MaxItemC<=0)
        return false;
    int RowIndex=SelectedIndex/Kit->MaxItemC;
    int ColIndex=SelectedIndex%Kit->MaxItemC;
    if(SuckerPtr!=NULL)
        *SuckerPtr=&Kit->Suck[RowIndex][ColIndex];
    return true;
}
//---------------------------------------------------------------------------
bool Tfiosetview::IsLegacyGeneralIOMode()
{
    TRadioButton *GeneralRadio;

    GeneralRadio=dynamic_cast<TRadioButton *>(FindComponent("rbGeneralIO"));
    if(GeneralRadio!=NULL)
        return GeneralRadio->Checked;

    return IsManualOutputEnabled();
}
//---------------------------------------------------------------------------
bool Tfiosetview::ToggleLegacyButtonOutput(TBtnPanel *ButtonPtr)
{
    AnsiString AliasName;
    bool CurrentState;

    if(ButtonPtr==NULL)
        return false;

    AliasName=ButtonPtr->Alias;
    if(AliasName==AnsiString(""))
        return false;

    for(int SwitchIndex=0; SwitchIndex<HSys.iTotalSwitch; SwitchIndex++)
    {
        TMySwitch *SwitchPtr=&HSys.SwPtr[SwitchIndex];
        if(AliasName==SwitchPtr->Name)
        {
            if(IsPadCommunicationOutput(AliasName))
            {
                CurrentState=SwitchPtr->Status();
                SwitchPtr->OutValue=!CurrentState;
                SwitchPtr->SetValue=!CurrentState;
                return true;
            }
            if(SwitchPtr->Enable==false)
                return false;

            CurrentState=SwitchPtr->Status();
            if(CurrentState)
                SwitchPtr->Off();
            else
                SwitchPtr->On();
            return true;
        }
    }

    for(int CylinderIndex=0; CylinderIndex<HSys.iTotalCylinder; CylinderIndex++)
    {
        TMyCylinder *CylinderPtr=&HSys.CynPtr[CylinderIndex];
        if(AliasName==CylinderPtr->CylinderName)
        {
            if(CylinderPtr->Enable==false || CylinderPtr->Switch.Enable==false)
                return false;

            CurrentState=CylinderPtr->GetOutBit();
            if(CurrentState)
                CylinderPtr->Off();
            else
                CylinderPtr->On();
            return true;
        }
    }

    for(int SuckerIndex=0; SuckerIndex<HSys.iTotalSucker; SuckerIndex++)
    {
        TMyKitSuck *Kit=&HSys.SuckPtr[SuckerIndex];
        for(int RowIndex=0; RowIndex<Kit->MaxItemR; RowIndex++)
        {
            for(int ColIndex=0; ColIndex<Kit->MaxItemC; ColIndex++)
            {
                TMySucker *SuckerPtr=&Kit->Suck[RowIndex][ColIndex];
                if(AliasName==SuckerPtr->OnPortName)
                {
                    if(SuckerPtr->Enable==false || SuckerPtr->OnSw.Enable==false)
                        return false;

                    CurrentState=SuckerPtr->GetOnBit();
                    if(CurrentState)
                        SuckerPtr->OffSuck();
                    else
                        SuckerPtr->OnSuck();
                    return true;
                }
                if(AliasName==SuckerPtr->OffPortName)
                {
                    if(SuckerPtr->Enable==false || SuckerPtr->OffSw.Enable==false)
                        return false;

                    CurrentState=SuckerPtr->GetOffBit();
                    if(CurrentState)
                        SuckerPtr->OffDestroy();
                    else
                        SuckerPtr->OnDestroy();
                    return true;
                }
            }
        }
    }

    return false;
}
//---------------------------------------------------------------------------
void Tfiosetview::UpdateSelectedInfo()
{
    AnsiString Str="Selected: none";
    TMySwitch *SwitchPtr=NULL;
    TMyCylinder *CylinderPtr=NULL;
    TMySucker *SuckerPtr=NULL;

    if(GetSelectedSwitch(&SwitchPtr) && SwitchPtr!=NULL)
        Str="Selected switch: "+SwitchPtr->Name;
    else if(GetSelectedCylinder(&CylinderPtr) && CylinderPtr!=NULL)
        Str="Selected cylinder: "+CylinderPtr->CylinderName;
    else if(GetSelectedSucker(&SuckerPtr) && SuckerPtr!=NULL)
        Str="Selected sucker: "+SuckerPtr->SuckerName;

    lblSelected->Caption=Str;
}
//---------------------------------------------------------------------------
void Tfiosetview::UpdateManualButtons()
{
    bool HasSwitch=(SelectedKind==eIOViewSwitch);
    bool HasCylinder=(SelectedKind==eIOViewCylinder);
    bool HasSucker=(SelectedKind==eIOViewSucker);
    bool Manual=CanManualOutput(NULL);
    btnOutputOn->Enabled=Manual && (HasSwitch || HasCylinder || HasSucker);
    btnOutputOff->Enabled=Manual && (HasSwitch || HasCylinder || HasSucker);
    btnSuckerDestroy->Enabled=Manual && HasSucker;
    btnOutputOn->Font->Color=btnOutputOn->Enabled?clBlack:clGray;
    btnOutputOff->Font->Color=btnOutputOff->Enabled?clBlack:clGray;
    btnSuckerDestroy->Font->Color=btnSuckerDestroy->Enabled?clBlack:clGray;
}
//---------------------------------------------------------------------------
TPageControl *Tfiosetview::GetLegacyPageIO()
{
    return dynamic_cast<TPageControl *>(FindComponent("PageIO"));
}
//---------------------------------------------------------------------------
void Tfiosetview::SelectLegacyIOPageByButton(TSpeedButton *Button)
{
    TPageControl *LegacyPage;
    TPanel *TitlePanel;
    int PageIndex;

    if(Button==NULL)
        return;

    LegacyPage=GetLegacyPageIO();
    if(LegacyPage==NULL)
        return;

    PageIndex=Button->Tag-1;
    if(PageIndex<0 || PageIndex>=LegacyPage->PageCount)
        return;

    LegacyPage->ActivePageIndex=PageIndex;
    TitlePanel=dynamic_cast<TPanel *>(FindComponent("plIOForm"));
    if(TitlePanel!=NULL)
        TitlePanel->Caption=Button->Caption;
    UpdateLegacyPageTabsVisible();
}
//---------------------------------------------------------------------------
void Tfiosetview::UpdateLegacyPageTabsVisible()
{
    TPageControl *LegacyPage;
    bool bVisible;
    int PageIndex;

    LegacyPage=GetLegacyPageIO();
    if(LegacyPage==NULL)
        return;

    bVisible=!HSys.Sys.SystemStart;
    for(PageIndex=0; PageIndex<LegacyPage->PageCount; PageIndex++)
    {
        if(LegacyPage->Pages[PageIndex]!=NULL && LegacyPage->Pages[PageIndex]->TabVisible!=bVisible)
            LegacyPage->Pages[PageIndex]->TabVisible=bVisible;
    }
}
//---------------------------------------------------------------------------
void Tfiosetview::SetRefreshTimerEnabled(bool Enabled)
{
//    TTimer *LegacyTimer;
//
//    if(tmrRefresh!=NULL)
//    {
//        tmrRefresh->Enabled=Enabled;
//        return;
//    }
//
//    LegacyTimer=FindNamedTimer(this, "Timer1");
//    if(LegacyTimer!=NULL)
//        LegacyTimer->Enabled=Enabled;
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::FormShow(TObject *Sender)
{
    (void)Sender;
    EnsureIOTableEditor();
    RefreshCurrentView();
    RefreshLegacyIOMaps();
    UpdateLegacyPageTabsVisible();
    SetRefreshTimerEnabled(true);
    fShow=true;
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::btnRefreshClick(TObject *Sender)
{
    (void)Sender;
    RefreshCurrentView();
    RefreshLegacyIOMaps();
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::chkManualOutputClick(TObject *Sender)
{
    AnsiString Reason;

    (void)Sender;
    if(IsManualOutputEnabled() && !CanManualOutput(&Reason))
    {
        chkManualOutput->Checked=false;
        ShowManualOutputBlocked(Reason);
    }
    UpdateManualButtons();
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::GridSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect)
{
    (void)ACol;
    CanSelect=true;
    SelectedKind=eIOViewNone;
    SelectedIndex=-1;
    SelectedRow=ARow;
    SelectedCol=ACol;

    if(ARow>0)
    {
        if(Sender==grdSwitches)
        {
            SelectedKind=eIOViewSwitch;
            SelectedIndex=ARow-1;
        }
        else if(Sender==grdCylinders)
        {
            SelectedKind=eIOViewCylinder;
            SelectedIndex=ARow-1;
        }
        else if(Sender==grdSuckers)
        {
            SelectedKind=eIOViewSucker;
            SelectedIndex=ARow-1;
        }
    }
    UpdateSelectedInfo();
    UpdateManualButtons();
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::btnOutputOnClick(TObject *Sender)
{
    AnsiString Reason;

    (void)Sender;
    if(!CanManualOutput(&Reason))
    {
        ShowManualOutputBlocked(Reason);
        return;
    }

    TMySwitch *SwitchPtr=NULL;
    TMyCylinder *CylinderPtr=NULL;
    TMySucker *SuckerPtr=NULL;
    if(GetSelectedSwitch(&SwitchPtr) && SwitchPtr!=NULL)
    {
        SwitchPtr->On();
        SyncPadSwitchStatus(SwitchPtr->Name, SwitchPtr->Status());
        LogManualOutputAction(SwitchPtr->Name, "ON", "OK");
    }
    else if(GetSelectedCylinder(&CylinderPtr) && CylinderPtr!=NULL)
    {
        CylinderPtr->On();
        LogManualOutputAction(CylinderPtr->CylinderName, "ON", "OK");
    }
    else if(GetSelectedSucker(&SuckerPtr) && SuckerPtr!=NULL)
    {
        SuckerPtr->On();
        LogManualOutputAction(SuckerPtr->SuckerName, "ON", "OK");
    }
    else
    {
        LogManualOutputAction("ManualOutput", "ON", "NO_SELECTION");
        ShowMessage("No output selected.");
    }
    RefreshCurrentView();
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::btnOutputOffClick(TObject *Sender)
{
    AnsiString Reason;

    (void)Sender;
    if(!CanManualOutput(&Reason))
    {
        ShowManualOutputBlocked(Reason);
        return;
    }

    TMySwitch *SwitchPtr=NULL;
    TMyCylinder *CylinderPtr=NULL;
    TMySucker *SuckerPtr=NULL;
    if(GetSelectedSwitch(&SwitchPtr) && SwitchPtr!=NULL)
    {
        SwitchPtr->Off();
        SyncPadSwitchStatus(SwitchPtr->Name, SwitchPtr->Status());
        LogManualOutputAction(SwitchPtr->Name, "OFF", "OK");
    }
    else if(GetSelectedCylinder(&CylinderPtr) && CylinderPtr!=NULL)
    {
        CylinderPtr->Off();
        LogManualOutputAction(CylinderPtr->CylinderName, "OFF", "OK");
    }
    else if(GetSelectedSucker(&SuckerPtr) && SuckerPtr!=NULL)
    {
        SuckerPtr->Normal();
        LogManualOutputAction(SuckerPtr->SuckerName, "OFF", "OK");
    }
    else
    {
        LogManualOutputAction("ManualOutput", "OFF", "NO_SELECTION");
        ShowMessage("No output selected.");
    }
    RefreshCurrentView();
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::btnSuckerDestroyClick(TObject *Sender)
{
    AnsiString Reason;

    (void)Sender;
    if(!CanManualOutput(&Reason))
    {
        ShowManualOutputBlocked(Reason);
        return;
    }

    TMySucker *SuckerPtr=NULL;
    if(GetSelectedSucker(&SuckerPtr) && SuckerPtr!=NULL)
    {
        SuckerPtr->Off();
        LogManualOutputAction(SuckerPtr->SuckerName, "DESTROY", "OK");
    }
    else
    {
        LogManualOutputAction("ManualOutput", "DESTROY", "NO_SELECTION");
        ShowMessage("No sucker selected.");
    }
    RefreshCurrentView();
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::BtnPanelClick(TObject *Sender)
{
    TBtnPanel *ButtonPtr;
    AnsiString Reason;

    ButtonPtr=dynamic_cast<TBtnPanel *>(Sender);
    if(ButtonPtr==NULL)
        return;

    if(!CanLegacyManualOutput(&Reason))
    {
        ShowManualOutputBlocked(Reason);
        RefreshCurrentView();
        return;
    }

    if(!ToggleLegacyButtonOutput(ButtonPtr))
    {
        LogManualOutputAction(ButtonPtr->Alias, "TOGGLE", "UNBOUND_OR_DISABLED");
        ShowMessage("Output is not bound or disabled.");
    }
    else
    {
        bool State=false;
        ResolveLegacyButtonState(ButtonPtr->Alias, &State);
        SyncPadSwitchStatus(ButtonPtr->Alias, State);
        LogManualOutputAction(ButtonPtr->Alias, "TOGGLE", State?AnsiString("ON"):AnsiString("OFF"));
    }

    RefreshCurrentView();
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::ComboBox1Change(TObject *Sender)
{
    (void)Sender;
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::rbGeneralIOClick(TObject *Sender)
{
    (void)Sender;
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::SaveInputMap1Click(TObject *Sender)
{
    (void)Sender;
    SaveIOMap(true);
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::SaveOutputMap1Click(TObject *Sender)
{
    (void)Sender;
    SaveIOMap(false);
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::sb_IO_CommunicationPadClick(TObject *Sender)
{
    (void)Sender;
    if(fPadInterface==NULL)
        fPadInterface=new TfPadInterface(this);
    fPadInterface->ShowModal();
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::sbCylinderClick(TObject *Sender)
{
    (void)Sender;
    EnsureIOTableEditor();
    if(cbbType!=NULL)
        cbbType->ItemIndex=4;
    if(edtSearchIO!=NULL)
        edtSearchIO->Text="";
    LoadIoTable((cbbType==NULL)?4:cbbType->ItemIndex, (cbbLane==NULL)?0:cbbLane->ItemIndex, 0);
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::sbEnableIOChangClick(TObject *Sender)
{
    (void)Sender;
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::sbInputClick(TObject *Sender)
{
    (void)Sender;
    EnsureIOTableEditor();
    if(cbbType!=NULL)
        cbbType->ItemIndex=1;
    if(edtSearchIO!=NULL)
        edtSearchIO->Text="";
    LoadIoTable((cbbType==NULL)?1:cbbType->ItemIndex, (cbbLane==NULL)?0:cbbLane->ItemIndex, 0);
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::sbIOExitClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::sbIORefreshClick(TObject *Sender)
{
    (void)Sender;
    if(strngrdIoTable!=NULL)
    {
        HSys.LoadIoData();
        if(IOTableDeletedTags!=NULL)
            IOTableDeletedTags->Clear();
        LoadIoTable((cbbType==NULL)?0:cbbType->ItemIndex, (cbbLane==NULL)?0:cbbLane->ItemIndex, 0);
        RefreshLegacyIOControls();
        RefreshLegacyIOMaps();
        return;
    }
    RefreshCurrentView();
    RefreshLegacyIOMaps();
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::sbIORingLoadClick(TObject *Sender)
{
    TSpeedButton *Button;

    Button=dynamic_cast<TSpeedButton *>(Sender);
    SelectLegacyIOPageByButton(Button);
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::sbOutputClick(TObject *Sender)
{
    (void)Sender;
    EnsureIOTableEditor();
    if(cbbType!=NULL)
        cbbType->ItemIndex=3;
    if(edtSearchIO!=NULL)
        edtSearchIO->Text="";
    LoadIoTable((cbbType==NULL)?3:cbbType->ItemIndex, (cbbLane==NULL)?0:cbbLane->ItemIndex, 0);
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::sbVacuumClick(TObject *Sender)
{
    (void)Sender;
    EnsureIOTableEditor();
    if(cbbType!=NULL)
        cbbType->ItemIndex=2;
    if(edtSearchIO!=NULL)
        edtSearchIO->Text="";
    LoadIoTable((cbbType==NULL)?2:cbbType->ItemIndex, (cbbLane==NULL)?0:cbbLane->ItemIndex, 0);
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::btnAddIOClick(TObject *Sender)
{
    int Row;

    (void)Sender;
    EnsureIOTableEditor();
    if(strngrdIoTable==NULL)
        return;

    if(strngrdIoTable->RowCount==2 && IsIOTableGridRowBlank(1))
        Row=1;
    else
    {
        Row=strngrdIoTable->RowCount;
        SetGridRowCount(strngrdIoTable, Row+1);
    }

    for(int Col=0; Col<strngrdIoTable->ColCount; Col++)
        strngrdIoTable->Cells[Col][Row]="";
    iSelectRow=Row;
    iSelectCol=eIOTableColType;
    strngrdIoTable->Row=Row;
    strngrdIoTable->Col=eIOTableColType;
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::btnDeleteIOClick(TObject *Sender)
{
    AnsiString TagText;

    (void)Sender;
    if(strngrdIoTable==NULL || iSelectRow<=0 || iSelectRow>=strngrdIoTable->RowCount)
        return;

    TagText=GetIOTableGridCell(iSelectRow, eIOTableColTag);
    if(TagText!=AnsiString("") && IOTableDeletedTags!=NULL && IOTableDeletedTags->IndexOf(TagText)<0)
        IOTableDeletedTags->Add(TagText);

    for(int Row=iSelectRow; Row<strngrdIoTable->RowCount-1; Row++)
    {
        for(int Col=0; Col<strngrdIoTable->ColCount; Col++)
            strngrdIoTable->Cells[Col][Row]=strngrdIoTable->Cells[Col][Row+1];
    }

    if(strngrdIoTable->RowCount>2)
        strngrdIoTable->RowCount=strngrdIoTable->RowCount-1;
    else
        ClearGridRows(strngrdIoTable);

    if(iSelectRow>=strngrdIoTable->RowCount)
        iSelectRow=strngrdIoTable->RowCount-1;
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::btnModifyClick(TObject *Sender)
{
    AnsiString Header;
    AnsiString Value;

    (void)Sender;
    if(strngrdIoTable==NULL || iSelectRow<=0 || iSelectCol<=eIOTableColTag ||
       iSelectRow>=strngrdIoTable->RowCount || iSelectCol>=strngrdIoTable->ColCount)
        return;

    Header=strngrdIoTable->Cells[iSelectCol][0];
    Value=InputBox("Modify IO Table", Header, strngrdIoTable->Cells[iSelectCol][iSelectRow]);
    strngrdIoTable->Cells[iSelectCol][iSelectRow]=Value;
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::sbUpdateClick(TObject *Sender)
{
    (void)Sender;
    SaveIoTableFromGrid();
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::cbbTypeChange(TObject *Sender)
{
    (void)Sender;
    LoadIoTable((cbbType==NULL)?0:cbbType->ItemIndex, (cbbLane==NULL)?0:cbbLane->ItemIndex, 0);
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::edtSearchIOChange(TObject *Sender)
{
    (void)Sender;
    if(edtSearchIO==NULL || edtSearchIO->Text==AnsiString("") || edtSearchIO->Text.Length()>=2)
        LoadIoTable((cbbType==NULL)?0:cbbType->ItemIndex, (cbbLane==NULL)?0:cbbLane->ItemIndex, 0);
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::strngrdIoTableDblClick(TObject *Sender)
{
    (void)Sender;
    btnModifyClick(Sender);
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::strngrdIoTableSelectCell(TObject *Sender, int ACol, int ARow, bool &CanSelect)
{
    (void)Sender;
    iSelectRow=ARow;
    iSelectCol=ACol;
    CanSelect=true;
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::spbTerminalProgramClick(TObject *Sender)
{
    (void)Sender;
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::Timer1Timer(TObject *Sender)
{
    (void)Sender;
    RefreshCurrentView();
    UpdateLegacyPageTabsVisible();
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::tmr_IonFanTimer(TObject *Sender)
{
    (void)Sender;
}
//---------------------------------------------------------------------------
void __fastcall Tfiosetview::FormClose(TObject *Sender,
      TCloseAction &Action)
{
    fShow=false;      
}
//---------------------------------------------------------------------------

