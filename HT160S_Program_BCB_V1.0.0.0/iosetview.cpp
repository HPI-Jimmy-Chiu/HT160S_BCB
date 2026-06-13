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


static bool TextEndsWith(AnsiString Value, AnsiString Suffix)
{
    int ValueLength=Value.Length();
    int SuffixLength=Suffix.Length();

    if(SuffixLength<=0 || SuffixLength>ValueLength)
        return false;
    return Value.SubString(ValueLength-SuffixLength+1, SuffixLength)==Suffix;
}



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
//AI(general) 20260613 : MN200 connection status page. Shows card open state, ring
//count, and per-ring start/device/error captured by OpenMN200Card(), plus the
//count of enabled MotionNet IO points in IO_Table. Under SOFT_SIMULATE the card is
//never opened, so the page reports simulation mode instead of live ring data.
void Tfiosetview::RefreshMN200()
{
    if(grdMN200==NULL)
        return;

    //AI(general) 20260613 : header/column setup formerly done by the removed
    //code-built BuildPages(); set here so the DFM StringGrid shows MN200 columns.
    grdMN200->ColCount=4;
    grdMN200->FixedRows=1;
    grdMN200->Cells[0][0]="Ring";
    grdMN200->Cells[1][0]="Started";
    grdMN200->Cells[2][0]="Devices";
    grdMN200->Cells[3][0]="ErrorCode";

    TMN200Connection *Conn=GetMN200Connection();

    int MotionNetPoints=0;
    int IOCount=(HSys.IOTable==NULL)?0:HSys.IOTable->Count;
    for(int Index=0; Index<IOCount; Index++)
    {
        TIODATA *Data=(TIODATA *)HSys.IOTable->Items[Index];
        if(Data!=NULL && Data->iEnable!=0 && Data->iISABase==eMotionNet)
            MotionNetPoints++;
    }

    AnsiString Summary;
#ifdef SOFT_SIMULATE
    Summary.sprintf("SOFT_SIMULATE active - MN200 card NOT opened (IO is simulated).  MotionNet IO points in table=%d",
                    MotionNetPoints);
#else
    if(Conn!=NULL && Conn->bOpened)
        Summary.sprintf("MN200 opened.  Rings=%d  MotionNet IO points=%d  LastMsg=%s",
                        Conn->iNumLine, MotionNetPoints, Conn->sLastMessage.c_str());
    else
        Summary.sprintf("MN200 NOT opened (LastError=%d %s).  MotionNet IO points=%d",
                        (Conn==NULL)?0:Conn->iLastError,
                        (Conn==NULL)?"":Conn->sLastMessage.c_str(),
                        MotionNetPoints);
#endif
    if(lblMN200Summary!=NULL)
        lblMN200Summary->Caption=Summary;

    int Rings=(Conn==NULL)?0:Conn->iNumLine;
    if(Rings<0)
        Rings=0;
    if(Rings>MN200_MAX_RING)
        Rings=MN200_MAX_RING;
    SetGridRowCount(grdMN200, Rings+1);
    ClearGridRows(grdMN200);
    for(int i=0; i<Rings; i++)
    {
        grdMN200->Cells[0][i+1]=IntToStr(i);
        grdMN200->Cells[1][i+1]=(Conn->bRingStarted[i])?AnsiString("YES"):AnsiString("NO");
        grdMN200->Cells[2][i+1]=IntToStr(Conn->iNumDev[i]);
        grdMN200->Cells[3][i+1]=IntToStr(Conn->iRingError[i]);
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

    PruneIOTableBackups(BackupDir, 10);

    if(BackupFile!=NULL)
        *BackupFile=BackupName;
    return true;
}
//---------------------------------------------------------------------------
//AI(general) 20260613 : keep at most MaxKeep IO_Table backup files in BackupDir
//and delete the oldest first, so the system backup folder does not grow without
//bound. Backup names are IO_Table_yyyymmdd_hhnnss.csv, so an ascending name sort
//is also chronological; the first (Count-MaxKeep) entries are therefore the oldest.
void Tfiosetview::PruneIOTableBackups(AnsiString BackupDir, int MaxKeep)
{
    TSearchRec SearchRec;
    TStringList *Names;
    AnsiString Mask;
    int FindResult;
    int DeleteCount;

    if(BackupDir==AnsiString("") || MaxKeep<0)
        return;

    Names=new TStringList();
    Mask=BackupDir+AnsiString("\\IO_Table_*.csv");
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
void Tfiosetview::RefreshCurrentView()
{
    //AI(general) 20260613 : the code-built generic-grid view was removed; the
    //live IO view is the DFM legacy LED/panel layout that RefreshLegacyIOControls
    //repaints from current IO state.
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
    //AI(general) 20260613 : Timer1 is a DFM component (OnTimer=Timer1Timer) shipped
    //with Enabled=False. FormShow calls this with true so the live legacy IO view
    //(RefreshLegacyIOControls) is re-scanned periodically; without it a sensor that
    //changes after the window opens never updates on screen.
    if(Timer1!=NULL)
        Timer1->Enabled=Enabled;
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
void __fastcall Tfiosetview::BtnPanelClick(TObject *Sender)
{
    TBtnPanel *ButtonPtr;

    //AI(general) 20260613 : direct manual toggle, matching HT172 IOSetViewOutput.
    //The previous IsManualOutputEnabled()/CanLegacyManualOutput() gate is removed;
    //ToggleLegacyButtonOutput already refuses unbound or disabled outputs.
    ButtonPtr=dynamic_cast<TBtnPanel *>(Sender);
    if(ButtonPtr==NULL)
        return;

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
    if(!fShow)
        return;
    RefreshCurrentView();
    RefreshMN200();
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

