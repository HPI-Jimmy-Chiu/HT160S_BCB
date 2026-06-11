//---------------------------------------------------------------------------
#include <vcl.h>
#include <IniFiles.hpp>
#include <stdio.h>
#pragma hdrstop

#include "uspeed.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfSpeed *fSpeed;
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260602 : HT172 0420 Speed module port.
//  SetMotorSpeed mirrors HT172 cinitial.cpp; persistence uses the HT160
//  TIniFile idiom (HT172's FormSysTools is an empty stub here). HT172's
//  SortArmTrayZ/MagX fixed-speed block and FuncS.iS01_* config are NOT ported
//  (HT160 has neither those axes nor a FuncS struct).
//---------------------------------------------------------------------------
static AnsiString GetMachineSpeedFileName()
{
    AnsiString RootPath=HSys.CurrentDir;
    if(RootPath==AnsiString(""))
        RootPath="..";
    return RootPath+AnsiString("\\system\\machine_speed.ini");
}
//---------------------------------------------------------------------------
static AnsiString GetMotorSpeedKey(int i)
{
    if(HSys.MotPtr==NULL || HSys.MotPtr[i]==NULL)
        return AnsiString("Mot")+IntToStr(i);
    AnsiString Key=HSys.MotPtr[i]->Alias;
    if(Key==AnsiString(""))
        Key=AnsiString("Mot")+IntToStr(i);
    return Key;
}
//---------------------------------------------------------------------------
void SetMotorSpeed(bool bSet)
{
    //AI(HT160S-Maintainer) 20260602 : re-apply each motor's working percentage
    //  to its speed register. bSet=false is blocked while homing or while the
    //  machine is running so live speed is not changed mid-motion (matches
    //  HT172). bSet=true forces a re-apply (used right after Home completes).
    if(HSys.MotPtr==NULL)
        return;
    if(bSet==false && (HSys.Sys.RunMode==Run_Home || HSys.Sys.SystemStart==true))
        return;
    for(int i=0; i<HSys.iTotalMotor; i++)
    {
        if(HSys.MotPtr[i]!=NULL)
            HSys.MotPtr[i]->SetPersentSpeed(HSys.MotPtr[i]->GetPersentSpeed());
    }
}
//---------------------------------------------------------------------------
void LoadMotorSpeedFromIni()
{
    if(HSys.MotPtr==NULL)
        return;
    AnsiString FileName=GetMachineSpeedFileName();
    bool bExist=FileExists(FileName);
    TIniFile *Ini=NULL;
    if(bExist)
        Ini=new TIniFile(FileName);
    for(int i=0; i<HSys.iTotalMotor; i++)
    {
        if(HSys.MotPtr[i]==NULL)
            continue;
        int pct=100;
        if(Ini!=NULL)
            pct=Ini->ReadInteger("Speed", GetMotorSpeedKey(i), 100);
        if(pct<1)
            pct=1;
        else if(pct>100)
            pct=100;
        HSys.MotPtr[i]->SetPersentSpeed(pct);   // bSave=true: store + apply
    }
    if(Ini!=NULL)
        delete Ini;
    if(bExist==false)
        SaveMotorSpeedToIni();                  // create baseline file at 100%
}
//---------------------------------------------------------------------------
void SaveMotorSpeedToIni()
{
    if(HSys.MotPtr==NULL)
        return;
    AnsiString sDir=HSys.CurrentDir;
    if(sDir==AnsiString(""))
        sDir="..";
    sDir=sDir+AnsiString("\\system");
    if(DirectoryExists(sDir)==false)
        ForceDirectories(sDir);
    TIniFile *Ini=new TIniFile(GetMachineSpeedFileName());
    for(int i=0; i<HSys.iTotalMotor; i++)
    {
        if(HSys.MotPtr[i]==NULL)
            continue;
        Ini->WriteInteger("Speed", GetMotorSpeedKey(i), HSys.MotPtr[i]->GetPersentSpeed());
    }
    delete Ini;
}
//---------------------------------------------------------------------------
//                              TMySpeedPanel
//---------------------------------------------------------------------------
TMySpeedPanel::TMySpeedPanel(TTrayMotor *Mot, TWinControl *Host)
{
    //AI(HT160S-Maintainer) 20260602 : colors/sizes aligned to HT172 0420 (Color=0x00C2B8A6, panel 573x43)
    //  Host (ScrollBox1) is passed in: the global fSpeed pointer is still NULL
    //  while FormCreate runs inside the constructor, so we must not use fSpeed here.
    Motor=Mot;
    bEnable=true;
    bUpdating=false;

    palMotorSpeed=new TPanel(Host);
    palMotorSpeed->Parent=Host;
    palMotorSpeed->Left=16;
    palMotorSpeed->Top=0;
    palMotorSpeed->Width=573;
    palMotorSpeed->Height=43;
    palMotorSpeed->BevelOuter=bvRaised;
    palMotorSpeed->Caption="";
    palMotorSpeed->Color=TColor(0x00C2B8A6);    // same as HT172 warm-beige

    labMotorSpeed=new TLabel(palMotorSpeed);
    labMotorSpeed->Parent=palMotorSpeed;
    labMotorSpeed->Left=15;
    labMotorSpeed->Top=9;
    labMotorSpeed->Width=146;
    labMotorSpeed->Height=27;
    labMotorSpeed->AutoSize=false;
    labMotorSpeed->Font->Size=14;
    labMotorSpeed->Caption=(Motor!=NULL ? Motor->Alias : AnsiString(""));

    edtMotorSpeed=new TEdit(palMotorSpeed);
    edtMotorSpeed->Parent=palMotorSpeed;
    edtMotorSpeed->Left=225;
    edtMotorSpeed->Top=9;
    edtMotorSpeed->Width=65;
    edtMotorSpeed->Height=24;
    edtMotorSpeed->Text="100";
    edtMotorSpeed->OnExit=edtMotorSpeedExit;

    scbMotorSpeed=new TScrollBar(palMotorSpeed);
    scbMotorSpeed->Parent=palMotorSpeed;
    scbMotorSpeed->Kind=sbHorizontal;
    scbMotorSpeed->Left=295;
    scbMotorSpeed->Top=10;
    scbMotorSpeed->Width=256;
    scbMotorSpeed->Height=23;
    scbMotorSpeed->Min=1;
    scbMotorSpeed->Max=100;
    scbMotorSpeed->Position=100;
    scbMotorSpeed->OnChange=scbMotorSpeedChange;

    SyncFromMotor();
}
//---------------------------------------------------------------------------
TMySpeedPanel::~TMySpeedPanel()
{
    // VCL controls are owned by their parent/host and freed by VCL; nothing
    // to delete here.
}
//---------------------------------------------------------------------------
void TMySpeedPanel::SyncFromMotor()
{
    if(Motor==NULL)
        return;
    bUpdating=true;
    int v=Motor->GetPersentSpeed();
    if(v<1)
        v=1;
    else if(v>100)
        v=100;
    scbMotorSpeed->Position=v;
    edtMotorSpeed->Text=IntToStr(v);
    bUpdating=false;
}
//---------------------------------------------------------------------------
void TMySpeedPanel::Apply(int persent)
{
    if(Motor==NULL)
        return;
    if(persent<1)
        persent=1;
    else if(persent>100)
        persent=100;
    Motor->iPersentSpeed=persent;   // store only; SetMotorSpeed() applies (guarded)
    SyncFromMotor();
}
//---------------------------------------------------------------------------
void __fastcall TMySpeedPanel::scbMotorSpeedChange(TObject *Sender)
{
    if(bUpdating || Motor==NULL)
        return;
    int pos=scbMotorSpeed->Position;
    Motor->iPersentSpeed=pos;       // store only
    bUpdating=true;
    edtMotorSpeed->Text=IntToStr(pos);
    bUpdating=false;
    SetMotorSpeed();                // apply now if idle; deferred while running
}
//---------------------------------------------------------------------------
void __fastcall TMySpeedPanel::edtMotorSpeedExit(TObject *Sender)
{
    if(Motor==NULL)
        return;
    int v=StrToIntDef(edtMotorSpeed->Text, Motor->GetPersentSpeed());
    if(v<1)
        v=1;
    else if(v>100)
        v=100;
    Motor->iPersentSpeed=v;
    SyncFromMotor();
    SetMotorSpeed();
}
//---------------------------------------------------------------------------
//                                 TfSpeed
//---------------------------------------------------------------------------
__fastcall TfSpeed::TfSpeed(TComponent* Owner)
    : TForm(Owner)
{
    // DFM handles layout; BuildPanels is called from FormCreate (OnCreate in DFM)
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260602 : layout (Panel10+ScrollBox1+buttons) is now in
//  uspeed.dfm. BuildPanels creates per-motor rows dynamically (count varies at runtime).
void TfSpeed::BuildPanels()
{
    if(HSys.MotPtr==NULL)
        return;
    for(int i=0; i<HSys.iTotalMotor; i++)
    {
        if(HSys.MotPtr[i]==NULL)
            continue;
        MySpeedPanel.push_back(new TMySpeedPanel(HSys.MotPtr[i], ScrollBox1));
    }
    // position panels (pitch=52 matches HT172 layout)
    int iStart=2, iPitch=52;
    for(unsigned int i=0; i<MySpeedPanel.size(); i++)
    {
        MySpeedPanel[i]->palMotorSpeed->Top=iStart;
        iStart+=iPitch;
    }
}
//---------------------------------------------------------------------------
void TfSpeed::RefreshAll()
{
    for(unsigned int i=0; i<MySpeedPanel.size(); i++)
        MySpeedPanel[i]->SyncFromMotor();
}
//---------------------------------------------------------------------------
void TfSpeed::GroupSpeedAddSub(int flag)
{
    for(unsigned int i=0; i<MySpeedPanel.size(); i++)
    {
        TTrayMotor *Mot=MySpeedPanel[i]->Motor;
        if(Mot==NULL)
            continue;
        int pct=Mot->GetPersentSpeed();
        if(flag==1)                 // add
        {
            pct=pct+10;
        }
        else if(flag==0)            // sub (tiered, mirrors HT172)
        {
            if(pct>20)
                pct=pct-10;
            else if(pct>10)
                pct=pct-5;
            else if(pct>1)
                pct=pct-1;
        }
        else                        // full
        {
            pct=100;
        }
        MySpeedPanel[i]->Apply(pct);
    }
    SetMotorSpeed();
}
//---------------------------------------------------------------------------
void __fastcall TfSpeed::spbFullSpeedClick(TObject *Sender)
{
    GroupSpeedAddSub(2);
}
//---------------------------------------------------------------------------
void __fastcall TfSpeed::spbAddSpeedClick(TObject *Sender)
{
    GroupSpeedAddSub(1);
}
//---------------------------------------------------------------------------
void __fastcall TfSpeed::spbSubSpeedClick(TObject *Sender)
{
    GroupSpeedAddSub(0);
}
//---------------------------------------------------------------------------
void __fastcall TfSpeed::spbExitClick(TObject *Sender)
{
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TfSpeed::FormCreate(TObject *Sender)
{
    BuildPanels();  //AI(HT160S-Maintainer) 20260602 : DFM controls ready; build per-motor rows
}
//---------------------------------------------------------------------------
void __fastcall TfSpeed::FormShow(TObject *Sender)
{
    RefreshAll();
}
//---------------------------------------------------------------------------
void __fastcall TfSpeed::FormClose(TObject *Sender, TCloseAction &Action)
{
    SaveMotorSpeedToIni();
}
//---------------------------------------------------------------------------
void __fastcall TfSpeed::FormDestroy(TObject *Sender)
{
    for(unsigned int i=0; i<MySpeedPanel.size(); i++)
        delete MySpeedPanel[i];
    MySpeedPanel.clear();
}
//---------------------------------------------------------------------------
