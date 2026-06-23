//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include <stdio.h>
#include "cStateRecordHT160.h"
#include "database.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "GeneralSetting.h"   //AI(ht160s-lotbin) 20260615 : SortMode flag + LotBinBinding for the snapshot
#include "main.h"
#include "aSortArm.h"         //AI(ht160s-state-record-analysis) 20260612 : SortArmModule sub-task readout (TTrayMotor/TMyKitSuck come via database.h)
#include "aAuto1To6.h"        //AI(ht160s-state-record-analysis) 20260616 : AutoModule per-station cell map for SortArmDecision.txt
#include "aColor.h"           //AI(ht160s-state-record-analysis) 20260622 : ColorModule->DescribeState()
#include "aEmpty.h"           //AI(ht160s-state-record-analysis) 20260622 : EmptyModule->DescribeState()
#include "aLoader.h"          //AI(ht160s-state-record-analysis) 20260622 : LoaderModule->DescribeState()
#include "uHGemEquipment.h"   //AI(ht160s-secsgem) 20260611 : HGem->FlushSecsLogToFile()
#pragma package(smart_init)
//---------------------------------------------------------------------------
static const AnsiString SR_VERSION = "HT160S 1.0.0.0";
//---------------------------------------------------------------------------
cStateRecordHT160 *gStateRecord = NULL;
//---------------------------------------------------------------------------
cStateRecordHT160::cStateRecordHT160()
{
    ModuleCount = 0;
    bInited     = false;
    SaveRoot    = "D:\\HT160S_StateRecord\\";

    for(int i=0; i<SR_MAX_MODULE; i++)
    {
        Modules[i].Name      = "";
        Modules[i].LastTask  = -1;
        Modules[i].bHasLast  = false;
        Modules[i].HistHead  = 0;
        Modules[i].HistCount = 0;
    }
}
//---------------------------------------------------------------------------
cStateRecordHT160::~cStateRecordHT160()
{
}
//---------------------------------------------------------------------------
void cStateRecordHT160::SetSaveFolder(AnsiString Folder)
{
    if(Folder==AnsiString(""))
        return;
    if(Folder[Folder.Length()]!='\\')
        Folder += "\\";
    SaveRoot = Folder;
}
//---------------------------------------------------------------------------
AnsiString cStateRecordHT160::RunModeText(int Mode)
{
    switch(Mode)
    {
        case Run_Normal:   return "Normal";
        case Run_Home:     return "Home";
        case Run_OneCycle: return "OneCycle";
        case Run_CleanOut: return "CleanOut";
        case Run_TrayFeed: return "TrayFeed";
        default:           return "Mode" + IntToStr(Mode);
    }
}
//---------------------------------------------------------------------------
AnsiString cStateRecordHT160::MakeStamp()
{
    // Mirror HT172 zip naming: "YYYY-MM-DD HH_MM_SS".
    return FormatDateTime("yyyy-mm-dd hh_nn_ss", Now());
}
//---------------------------------------------------------------------------
AnsiString cStateRecordHT160::GetProjectRoot()
{
    AnsiString Root = HSys.CurrentDir;
    if(Root==AnsiString(""))
        Root = "..";
    return Root;
}
//---------------------------------------------------------------------------
AnsiString cStateRecordHT160::Get7ZipPath()
{
    AnsiString Candidates[4];
    Candidates[0] = "C:\\Program Files\\7-Zip\\7z.exe";
    Candidates[1] = "C:\\Program Files (x86)\\7-Zip\\7z.exe";
    Candidates[2] = GetProjectRoot() + "\\7z.exe";
    Candidates[3] = GetProjectRoot() + "\\EXE\\7z.exe";

    for(int i=0; i<4; i++)
    {
        if(FileExists(Candidates[i]))
            return Candidates[i];
    }
    return "";
}
//---------------------------------------------------------------------------
void cStateRecordHT160::PushSample(int ModuleIndex, int Task)
{
    if(ModuleIndex<0 || ModuleIndex>=SR_MAX_MODULE)
        return;

    TModuleState *M = &Modules[ModuleIndex];
    M->Hist[M->HistHead].Time = Now();
    M->Hist[M->HistHead].Task = Task;
    M->HistHead = (M->HistHead + 1) % SR_MAX_HISTORY;
    if(M->HistCount < SR_MAX_HISTORY)
        M->HistCount++;

    M->LastTask = Task;
    M->bHasLast = true;
}
//---------------------------------------------------------------------------
void cStateRecordHT160::EnsureInited()
{
    if(bInited)
        return;
    if(DataModule1==NULL || DataModule1->UserMotion==NULL)
        return;

    TActionList *Motion = DataModule1->UserMotion;
    int Count = Motion->ActionCount;
    if(Count > SR_MAX_MODULE)
        Count = SR_MAX_MODULE;

    ModuleCount = Count;
    for(int i=0; i<Count; i++)
    {
        // TContainedAction exposes Name; Caption lives on TCustomAction.
        TCustomAction *Act = dynamic_cast<TCustomAction *>(Motion->Actions[i]);
        AnsiString Name = "";
        if(Act!=NULL)
            Name = Act->Caption;
        if(Name==AnsiString(""))
            Name = Motion->Actions[i]->Name;
        if(Name==AnsiString(""))
            Name = "Action" + IntToStr(i);
        Modules[i].Name      = Name;
        Modules[i].HistHead  = 0;
        Modules[i].HistCount = 0;
        Modules[i].bHasLast  = false;
        // Seed an initial sample so every module has at least one entry.
        PushSample(i, Motion->Actions[i]->Tag);
    }
    bInited = true;
}
//---------------------------------------------------------------------------
void cStateRecordHT160::SampleTasks()
{
    EnsureInited();
    if(!bInited)
        return;
    if(DataModule1==NULL || DataModule1->UserMotion==NULL)
        return;

    TActionList *Motion = DataModule1->UserMotion;
    int Count = Motion->ActionCount;
    if(Count > ModuleCount)
        Count = ModuleCount;

    for(int i=0; i<Count; i++)
    {
        int Tag = Motion->Actions[i]->Tag;
        if(Modules[i].bHasLast==false || Modules[i].LastTask!=Tag)
            PushSample(i, Tag);
    }
}
//---------------------------------------------------------------------------
bool cStateRecordHT160::CopyOneFile(AnsiString Src, AnsiString Dst)
{
    if(FileExists(Src)==false)
        return false;
    return CopyFile(Src.c_str(), Dst.c_str(), FALSE) ? true : false;
}
//---------------------------------------------------------------------------
int cStateRecordHT160::CopyFolderFiles(AnsiString SrcDir, AnsiString DstDirWithSlash)
{
    int Copied = 0;
    TSearchRec Sr;

    if(FindFirst(SrcDir + "\\*.*", faAnyFile, Sr)==0)
    {
        do
        {
            if(Sr.Name==AnsiString(".") || Sr.Name==AnsiString(".."))
                continue;
            if(Sr.Attr & faDirectory)
                continue;   // copy files only (recipe subfolders are flat here)
            AnsiString Src = SrcDir + "\\" + Sr.Name;
            AnsiString Dst = DstDirWithSlash + Sr.Name;
            if(CopyOneFile(Src, Dst))
                Copied++;
        }
        while(FindNext(Sr)==0);
        FindClose(Sr);
    }
    return Copied;
}
//---------------------------------------------------------------------------
bool cStateRecordHT160::DeleteFolderRecursive(AnsiString Dir)
{
    TSearchRec Sr;

    if(FindFirst(Dir + "\\*.*", faAnyFile, Sr)==0)
    {
        do
        {
            if(Sr.Name==AnsiString(".") || Sr.Name==AnsiString(".."))
                continue;
            AnsiString Full = Dir + "\\" + Sr.Name;
            if(Sr.Attr & faDirectory)
                DeleteFolderRecursive(Full);
            else
                DeleteFile(Full);
        }
        while(FindNext(Sr)==0);
        FindClose(Sr);
    }
    return RemoveDir(Dir) ? true : false;
}
//---------------------------------------------------------------------------
bool cStateRecordHT160::CompressFolder(AnsiString SrcDirWithSlash, AnsiString ZipPath)
{
    AnsiString SevenZip = Get7ZipPath();
    if(SevenZip==AnsiString(""))
        return false;

    //AI(HT160S-Maintainer) 20260610 : zip the folder ITSELF (not just its
    //contents) so the archive expands into ONE top-level folder named like the
    //zip (e.g. "2026-06-10 09_21_52\...") instead of loose files at the root.
    //Strip the trailing backslash, split into parent + folder name, then run
    //7-Zip with the parent as the working directory so the stored entry keeps
    //the single folder prefix.
    AnsiString SrcDir = SrcDirWithSlash;
    while(SrcDir.Length()>0 && SrcDir[SrcDir.Length()]=='\\')
        SrcDir = SrcDir.SubString(1, SrcDir.Length()-1);
    AnsiString ParentDir  = ExtractFilePath(SrcDir);
    AnsiString FolderName = ExtractFileName(SrcDir);

    // "7z.exe" a -tzip "out.zip" "FolderName"   (cwd = ParentDir)
    AnsiString Cmd = AnsiString("\"") + SevenZip + "\" a -tzip \"" +
                     ZipPath + "\" \"" + FolderName + "\"";

    STARTUPINFO         Si;
    PROCESS_INFORMATION Pi;
    ZeroMemory(&Si, sizeof(Si));
    Si.cb          = sizeof(Si);
    Si.dwFlags     = STARTF_USESHOWWINDOW;
    Si.wShowWindow = SW_HIDE;
    ZeroMemory(&Pi, sizeof(Pi));

    char CmdBuf[1024];
    strncpy(CmdBuf, Cmd.c_str(), sizeof(CmdBuf)-1);
    CmdBuf[sizeof(CmdBuf)-1] = 0;

    char DirBuf[1024];
    strncpy(DirBuf, ParentDir.c_str(), sizeof(DirBuf)-1);
    DirBuf[sizeof(DirBuf)-1] = 0;

    if(CreateProcess(NULL, CmdBuf, NULL, NULL, FALSE,
                     CREATE_NO_WINDOW, NULL, DirBuf, &Si, &Pi)==FALSE)
        return false;

    WaitForSingleObject(Pi.hProcess, 60000);
    DWORD ExitCode = 1;
    GetExitCodeProcess(Pi.hProcess, &ExitCode);
    CloseHandle(Pi.hThread);
    CloseHandle(Pi.hProcess);

    return (ExitCode==0) && FileExists(ZipPath);
}
//---------------------------------------------------------------------------
void cStateRecordHT160::WriteSnapshotIni(AnsiString Path, AnsiString Reason, AnsiString Stamp)
{
    TIniFile *Ini = new TIniFile(Path);
    Ini->WriteString("Snapshot", "TriggerReason", Reason);
    Ini->WriteString("Snapshot", "Time", Stamp);
    Ini->WriteString("Snapshot", "Version", SR_VERSION);
    delete Ini;
}
//---------------------------------------------------------------------------
void cStateRecordHT160::WriteTaskHistoryCsv(AnsiString Path)
{
    FILE *f = fopen(Path.c_str(), "wb");
    if(f==NULL)
        return;

    // Header: Module + (Time_k,Task_k) pairs, k=0 newest .. SR_MAX_HISTORY-1.
    AnsiString Header = "Module";
    for(int k=0; k<SR_MAX_HISTORY; k++)
        Header += ",Time_" + IntToStr(k) + ",Task_" + IntToStr(k);
    Header += "\r\n";
    fwrite(Header.c_str(), 1, Header.Length(), f);

    for(int m=0; m<ModuleCount; m++)
    {
        TModuleState *M = &Modules[m];
        AnsiString Row = M->Name;
        for(int k=0; k<SR_MAX_HISTORY; k++)
        {
            if(k < M->HistCount)
            {
                int Idx = (M->HistHead - 1 - k + 2*SR_MAX_HISTORY) % SR_MAX_HISTORY;
                AnsiString T = FormatDateTime("hh:nn:ss.zzz", M->Hist[Idx].Time);
                Row += "," + T + "," + IntToStr(M->Hist[Idx].Task);
            }
            else
            {
                Row += ",,";
            }
        }
        Row += "\r\n";
        fwrite(Row.c_str(), 1, Row.Length(), f);
    }

    fflush(f);
    fclose(f);
}
//---------------------------------------------------------------------------
void cStateRecordHT160::WriteCurrentTasksTxt(AnsiString Path)
{
    FILE *f = fopen(Path.c_str(), "wb");
    if(f==NULL)
        return;

    //AI(ht160s-state-record-analysis) 20260616 : StuckMs = ms since this module's last
    //Task change. NB when SystemStart=0 the module loop is frozen so Now() keeps moving
    //while the last change is fixed - StuckMs is then inflated by the operator's
    //notice->stop->snapshot delay (same caveat as the timestamps). It is exact when a
    //watchdog auto-triggers the snapshot while the line is still running.
    AnsiString Line = "Idx | Module        | CurTask | LastChangeTime  | StuckMs\r\n";
    fwrite(Line.c_str(), 1, Line.Length(), f);
    Line = "----+---------------+---------+-----------------+--------\r\n";
    fwrite(Line.c_str(), 1, Line.Length(), f);

    TDateTime NowT = Now();
    for(int m=0; m<ModuleCount; m++)
    {
        TModuleState *M = &Modules[m];
        AnsiString CurTask = "-";
        AnsiString LastTime = "-";
        AnsiString StuckMs = "-";
        if(M->HistCount > 0)
        {
            int Idx = (M->HistHead - 1 + SR_MAX_HISTORY) % SR_MAX_HISTORY;
            CurTask  = IntToStr(M->Hist[Idx].Task);
            LastTime = FormatDateTime("hh:nn:ss.zzz", M->Hist[Idx].Time);
            double dms = (double)(NowT - M->Hist[Idx].Time) * 86400000.0;
            if(dms < 0) dms = 0;
            StuckMs = IntToStr((int)dms);
        }

        AnsiString Name = M->Name;
        while(Name.Length() < 13) Name += " ";
        while(CurTask.Length() < 7) CurTask = " " + CurTask;
        while(LastTime.Length() < 15) LastTime += " ";

        Line = IntToStr(m);
        while(Line.Length() < 3) Line = " " + Line;
        Line += " | " + Name + " | " + CurTask + " | " + LastTime + " | " + StuckMs + "\r\n";
        fwrite(Line.c_str(), 1, Line.Length(), f);
    }

    fflush(f);
    fclose(f);
}
//---------------------------------------------------------------------------
void cStateRecordHT160::WriteMachineStateIni(AnsiString Path, AnsiString Reason, AnsiString Stamp)
{
    TIniFile *Ini = new TIniFile(Path);

    Ini->WriteString ("System", "RunMode",     IntToStr((int)HSys.Sys.RunMode));
    Ini->WriteString ("System", "RunModeName", RunModeText((int)HSys.Sys.RunMode));
    Ini->WriteInteger("System", "SystemStart", HSys.Sys.SystemStart ? 1 : 0);
    Ini->WriteInteger("System", "bCleanOut",   HSys.Sys.bCleanOut ? 1 : 0);

    Ini->WriteString("Recipe", "Name", RecipeManager.GetCurrentRecipeName());

    AnsiString Lot = "";
    if(fMain!=NULL && fMain->edLotNo!=NULL)
        Lot = fMain->edLotNo->Text;
    Ini->WriteString("Lot", "LotNo", Lot);

    //AI(ht160s-state-record-analysis) 20260612 : dump the live Lot registry so an
    //offline snapshot shows WHICH lots are loaded and which ones actually have 2D
    //data. The [Lot] LotNo alone cannot explain "lot B..E show 0 2D" symptoms.
    //  LotCount  = lots with a non-blank name ; ItemCount = total 2D codes loaded.
    //  Lot<n>    = LotID | Src(SECS/OFF) | 2D-plan-qty | sorted-qty.
    Ini->WriteInteger("LotList", "LotCount",  LotRegistry.GetLotCount());
    Ini->WriteInteger("LotList", "ItemCount", LotRegistry.GetItemCount());
    {
        int OutIdx=0;
        int SlotCount=LotRegistry.GetLotSlotCount();
        for(int i=0;i<SlotCount;i++)
        {
            TLotRunInfo *L=LotRegistry.GetLot(i);
            if(L==NULL || L->sLotID.Trim()==AnsiString(""))
                continue;
            AnsiString Src=(L->iSource==HT160_LOT_SOURCE_SECS)?AnsiString("SECS"):AnsiString("OFF");
            AnsiString Val=L->sLotID+"|"+Src+"|"+IntToStr(L->iPlanQty)+"|"+IntToStr(L->iSortedQty);
            Ini->WriteString("LotList", "Lot"+IntToStr(OutIdx), Val);
            OutIdx++;
        }
    }

    //AI(ht160s-state-record-analysis) 20260616 : [StuckMs] mirrors [Tasks] with the
    //ms-since-last-change per module (machine-readable companion to CurrentTasks.txt;
    //same SystemStart=0 inflation caveat).
    TDateTime NowT = Now();
    for(int m=0; m<ModuleCount; m++)
    {
        int Cur = Modules[m].bHasLast ? Modules[m].LastTask : -1;
        Ini->WriteString("Tasks", Modules[m].Name, IntToStr(Cur));

        int StuckMs = -1;
        if(Modules[m].HistCount > 0)
        {
            int Idx = (Modules[m].HistHead - 1 + SR_MAX_HISTORY) % SR_MAX_HISTORY;
            double dms = (double)(NowT - Modules[m].Hist[Idx].Time) * 86400000.0;
            if(dms < 0) dms = 0;
            StuckMs = (int)dms;
        }
        Ini->WriteString("StuckMs", Modules[m].Name, IntToStr(StuckMs));
    }

    Ini->WriteString("Snapshot", "Time",          Stamp);
    Ini->WriteString("Snapshot", "Version",       SR_VERSION);
    Ini->WriteString("Snapshot", "TriggerReason", Reason);

    delete Ini;
}
//---------------------------------------------------------------------------
//AI(ht160s-lot-webapi) 20260612 : escape a string for embedding in JSON.
static AnsiString SR_JsonEsc(AnsiString s)
{
    AnsiString out="";
    for(int i=1;i<=s.Length();i++)
    {
        char c=s[i];
        switch(c)
        {
            case '\"': out+="\\\""; break;
            case '\\': out+="\\\\"; break;
            case '\b': out+="\\b";  break;
            case '\f': out+="\\f";  break;
            case '\n': out+="\\n";  break;
            case '\r': out+="\\r";  break;
            case '\t': out+="\\t";  break;
            default:
                if((unsigned char)c < 0x20)
                {
                    char buf[8];
                    sprintf(buf, "\\u%04x", (unsigned char)c);
                    out+=buf;
                }
                else
                    out+=c;
                break;
        }
    }
    return out;
}
//---------------------------------------------------------------------------
//AI(ht160s-lot-webapi) 20260612 : dump the FULL Lot work order as JSON so an
//offline snapshot carries the real sort data (every 2D code + Bin/HBin/SBin/
//RetestCode/DiePass), not just the [LotList] counts in MachineState.ini.
//Walks the RAW slot span and skips freed (blank) slots. Leading-comma style
//keeps the JSON valid without trailing commas.
void cStateRecordHT160::WriteLotDataJson(AnsiString Path, AnsiString Reason, AnsiString Stamp)
{
    TStringList *Lines=new TStringList;
    TStringList *Ic=new TStringList;
    try
    {
        AnsiString ActiveLot="";
        if(fMain!=NULL && fMain->edLotNo!=NULL)
            ActiveLot=fMain->edLotNo->Text;

        Lines->Add("{");
        Lines->Add("  \"Snapshot\": { \"Time\": \""+SR_JsonEsc(Stamp)+"\", \"TriggerReason\": \""+SR_JsonEsc(Reason)+"\", \"Version\": \""+AnsiString(SR_VERSION)+"\" },");
        Lines->Add("  \"ActiveLot\": \""+SR_JsonEsc(ActiveLot)+"\",");
        Lines->Add("  \"LotCount\": "+IntToStr(LotRegistry.GetLotCount())+",");
        Lines->Add("  \"ItemCount\": "+IntToStr(LotRegistry.GetItemCount())+",");
        Lines->Add("  \"Lots\": [");

        int SlotCount=LotRegistry.GetLotSlotCount();
        bool bFirstLot=true;
        for(int i=0;i<SlotCount;i++)
        {
            TLotRunInfo *L=LotRegistry.GetLot(i);
            if(L==NULL || L->sLotID.Trim()==AnsiString(""))
                continue;

            AnsiString LotComma = bFirstLot ? AnsiString("") : AnsiString(",");
            bFirstLot=false;

            AnsiString Src=(L->iSource==HT160_LOT_SOURCE_SECS)?AnsiString("SECS"):AnsiString("OFFLINE");
            Lines->Add("    "+LotComma+"{");
            Lines->Add("      \"LotID\": \""+SR_JsonEsc(L->sLotID)+"\",");
            Lines->Add("      \"Source\": \""+Src+"\",");
            Lines->Add("      \"ProductCode\": \""+SR_JsonEsc(L->sProductCode)+"\",");
            Lines->Add("      \"Substage\": \""+SR_JsonEsc(L->sSubstage)+"\",");
            Lines->Add("      \"PlanQty\": "+IntToStr(L->iPlanQty)+",");
            Lines->Add("      \"SortedQty\": "+IntToStr(L->iSortedQty)+",");

            int IcCount=LotRegistry.GetLotIcList(L->sLotID, Ic);
            Lines->Add("      \"Items\": [");
            for(int k=0;k<IcCount;k++)
            {
                // GetLotIcList row = Code2D \t Bin \t HBin \t SBin \t RetestCode \t DiePass
                AnsiString Row=Ic->Strings[k];
                AnsiString f[6];
                for(int fi=0;fi<6;fi++)
                    f[fi]="";
                int fidx=0, start=1;
                for(int p=1;p<=Row.Length() && fidx<6;p++)
                {
                    if(Row[p]=='\t')
                    {
                        f[fidx]=Row.SubString(start, p-start);
                        fidx++;
                        start=p+1;
                    }
                }
                if(fidx<6)
                    f[fidx]=Row.SubString(start, Row.Length()-start+1);

                AnsiString IcComma = (k==0) ? AnsiString("") : AnsiString(",");
                AnsiString Bin  = (f[1].Trim()=="") ? AnsiString("0") : f[1].Trim();
                AnsiString HBin = (f[2].Trim()=="") ? AnsiString("0") : f[2].Trim();
                AnsiString SBin = (f[3].Trim()=="") ? AnsiString("0") : f[3].Trim();
                AnsiString Item="        "+IcComma+"{ \"Code2D\": \""+SR_JsonEsc(f[0])+"\""+
                    ", \"Bin\": "+Bin+", \"HBin\": "+HBin+", \"SBin\": "+SBin+
                    ", \"RetestCode\": \""+SR_JsonEsc(f[4])+"\""+
                    ", \"DiePass\": \""+SR_JsonEsc(f[5])+"\" }";
                Lines->Add(Item);
            }
            Lines->Add("      ]");
            Lines->Add("    }");
        }
        Lines->Add("  ],");

        //AI(ht160s-lotbin) 20260615 : dump the dynamic (Lot,Bin)->Auto table so a
        //snapshot taken in By Lot+Bin mode shows which Auto each pair was bound to.
        Lines->Add("  \"SortMode\": \""+AnsiString(GeneralSetting.bUseLotBinSortMode?"LotBin":"Normal")+"\",");
        Lines->Add("  \"LotBinBinding\": [");
        {
            int BindCount=LotBinBinding.GetBindingCount();
            for(int b=0;b<BindCount;b++)
            {
                AnsiString BLot;
                int BBin;
                int BAuto;
                if(LotBinBinding.GetBindingByIndex(b, BLot, BBin, BAuto)==false)
                    continue;
                AnsiString BComma = (b==0) ? AnsiString("") : AnsiString(",");
                Lines->Add("    "+BComma+"{ \"LotID\": \""+SR_JsonEsc(BLot)+"\", \"Bin\": "+IntToStr(BBin)+", \"Auto\": "+IntToStr(BAuto+1)+" }");
            }
        }
        Lines->Add("  ]");
        Lines->Add("}");

        Lines->SaveToFile(Path);
    }
    __finally
    {
        delete Ic;
        delete Lines;
    }
}
//---------------------------------------------------------------------------
//AI(ht160s-state-record-analysis) 20260612 : capture the live motion state that
//the Task-only history cannot show. KEY for the "SortArm sucker stayed DOWN while
//the arm moved in XY" symptom :
//  [Motors]   cmd/enc/target of every real motor. Compare MSuckZ_1..4 cmd against
//             SafeZ (=SORT_ARM_SAFE_Z_POSITION) ; if a SuckZ is far from SafeZ while
//             SortingArmX / *Y are mid-travel, the sucker was not lifted before move.
//  [SortArm]  PickTask / PlaceTask sub-step (top-level DoSortArm Task is only 1/100/200),
//             plus the 4 SuckZ positions and SortArmX broken out for quick reading.
//  [Suckers]  vacuum command bit per kit sucker (SortArmSuck etc.).
//All getters are read-only ; this does NOT change any motion logic. NOTE: with
//SystemStart=0 the module loop is frozen, but ReadPos()/ReadEncoderPos() still query
//the card live, so the frozen physical position is captured correctly.
void cStateRecordHT160::WriteMotionDetailIni(AnsiString Path)
{
    TIniFile *Ini = new TIniFile(Path);

    // ---- [Motors] : every real motor's live command / encoder / target ----
    Ini->WriteInteger("Motors", "TotalMotor", HSys.iTotalMotor);
    for(int i=0; i<HSys.iTotalMotor; i++)
    {
        TTrayMotor *M = (HSys.MotPtr!=NULL) ? HSys.MotPtr[i] : NULL;
        if(M==NULL)
            continue;

        AnsiString Key = M->Alias;
        if(Key==AnsiString(""))
            Key = "Motor" + IntToStr(i);

        AnsiString Val;
        Val  = "cmd=" + IntToStr(M->ReadPos());
        Val += ", enc=" + IntToStr(M->ReadEncoderPos());
        Val += ", tgt=" + IntToStr(M->TargetPosition);
        Val += ", home=" + IntToStr(M->bHomeFlag ? 1 : 0);
        Val += ", err=" + IntToStr(M->bErrorMove ? 1 : 0);
        Ini->WriteString("Motors", Key, Val);
    }

    // ---- [SortArm] : sub-task + Z-safe readout ----
    if(SortArmModule!=NULL)
    {
        Ini->WriteInteger("SortArm", "PickTask",  SortArmModule->GetPickTask());
        Ini->WriteInteger("SortArm", "PlaceTask", SortArmModule->GetPlaceTask());
    }
    Ini->WriteInteger("SortArm", "SafeZ", 10);   // SORT_ARM_SAFE_Z_POSITION (aSortArm.cpp)

    TTrayMotor *Zmot[4];
    Zmot[0]=HSys.Mot.MSuckZ_1; Zmot[1]=HSys.Mot.MSuckZ_2;
    Zmot[2]=HSys.Mot.MSuckZ_3; Zmot[3]=HSys.Mot.MSuckZ_4;
    for(int s=0; s<4; s++)
    {
        AnsiString K = "SuckZ_" + IntToStr(s+1);
        if(Zmot[s]==NULL)
        {
            Ini->WriteString("SortArm", K, "NULL");
            continue;
        }
        AnsiString V = "cmd=" + IntToStr(Zmot[s]->ReadPos())
                     + ", enc=" + IntToStr(Zmot[s]->ReadEncoderPos());
        Ini->WriteString("SortArm", K, V);
    }
    if(HSys.Mot.MSortingArmX!=NULL)
        Ini->WriteInteger("SortArm", "SortArmX", HSys.Mot.MSortingArmX->ReadPos());

    // ---- [Suckers] : vacuum command bit per kit sucker ----
    Ini->WriteInteger("Suckers", "TotalKit", HSys.iTotalSucker);
    for(int k=0; k<HSys.iTotalSucker; k++)
    {
        if(HSys.SuckPtr==NULL)
            break;
        TMyKitSuck &Kit = HSys.SuckPtr[k];

        AnsiString KitSection = Kit.Name;
        if(KitSection==AnsiString(""))
            KitSection = "Kit" + IntToStr(k);

        Ini->WriteInteger(KitSection, "Has_SuckIC", Kit.Has_SuckIC ? 1 : 0);
        for(int r=0; r<Kit.MaxItemR; r++)
        {
            for(int c=0; c<Kit.MaxItemC; c++)
            {
                AnsiString SK = "S" + IntToStr(r) + "_" + IntToStr(c);
                AnsiString SV = "vac=" + IntToStr(Kit.Suck[r][c].Status ? 1 : 0);
                Ini->WriteString(KitSection, SK, SV);
            }
        }
    }

    delete Ini;
}
//---------------------------------------------------------------------------
//AI(ht160s-state-record-analysis) 20260616 : SortArmDecision.txt - the place/discharge
//threshold-mismatch deadlock evidence the Task-only history cannot show. Dumps WHAT the
//SortArm holds + WHERE each held IC routes (DescribeHolding), then each Auto's live
//state + working-tray cell map (DescribeStation). Reading the two together shows why a
//held IC cannot be placed (its target Auto exposes no contiguous EMPTY_IC run that fits)
//while that Auto also cannot discharge (discharge needs every cell HAS_OK_IC). All
//getters are read-only and do NOT disturb the live place decision.
void cStateRecordHT160::WriteSortArmDecisionTxt(AnsiString Path)
{
    FILE *f = fopen(Path.c_str(), "wb");
    if(f==NULL)
        return;

    AnsiString Out;
    Out  = "SortArm place / Auto discharge decision snapshot\r\n";
    Out += "Tray geometry (recipe): X=" + IntToStr(TrayForm.XDivision)
         + "  Y=" + IntToStr(TrayForm.YDivision) + "\r\n";
    Out += "------------------------------------------------------------\r\n";

    if(SortArmModule!=NULL)
        Out += SortArmModule->DescribeHolding();
    else
        Out += "[SortArm] (module NULL)\r\n";
    Out += "------------------------------------------------------------\r\n";

    if(AutoModule!=NULL)
    {
        int n = AutoModule->GetStationCount();
        for(int i=0; i<n; i++)
            Out += AutoModule->DescribeStation(i);
    }
    else
        Out += "[Auto] (module NULL)\r\n";

    fwrite(Out.c_str(), 1, Out.Length(), f);
    fflush(f);
    fclose(f);
}
//---------------------------------------------------------------------------
//AI(ht160s-state-record-analysis) 20260622 : FeederDecision.txt - Color/Empty/Loader
//inner sub-task + flag latches plus the config gates that decide whether each feeder
//is even active. KEY for the "Color latched bTrayReady and spins idle because Normal
//mode (bUseAMR off / no AMR identity demand) never picks the tray" symptom, which the
//outer Task-only history (1/10/100) cannot show. Read-only; runs ONLY at snapshot
//trigger time (no per-cycle cost), mirroring WriteSortArmDecisionTxt.
void cStateRecordHT160::WriteFeederDecisionTxt(AnsiString Path)
{
    FILE *f = fopen(Path.c_str(), "wb");
    if(f==NULL)
        return;

    AnsiString Out;
    Out  = "Feeder (Color / Empty / Loader) inner-state + config-gate snapshot\r\n";
    Out += "RunMode=" + RunModeText((int)HSys.Sys.RunMode)
         + "  SystemStart=" + IntToStr(HSys.Sys.SystemStart ? 1 : 0) + "\r\n";
    Out += "------------------------------------------------------------\r\n";

    Out += "[Config gates]\r\n";
    Out += "  bUseAMR=" + IntToStr(GeneralSetting.bUseAMR ? 1 : 0)
         + "  bColorBinAreaInstalled=" + IntToStr(GeneralSetting.bColorBinAreaInstalled ? 1 : 0)
         + "  bUseLotBinSortMode=" + IntToStr(GeneralSetting.bUseLotBinSortMode ? 1 : 0) + "\r\n";
    Out += "  bUseColorCcd=" + IntToStr(CosFunction.bUseColorCcd ? 1 : 0)
         + "  bUse2DBinMap=" + IntToStr(CosFunction.bUse2DBinMap ? 1 : 0) + "\r\n";
    Out += "------------------------------------------------------------\r\n";

    if(ColorModule!=NULL)
        Out += ColorModule->DescribeState();
    else
        Out += "[Color] (module NULL)\r\n";
    Out += "------------------------------------------------------------\r\n";

    if(EmptyModule!=NULL)
        Out += EmptyModule->DescribeState();
    else
        Out += "[Empty] (module NULL)\r\n";
    Out += "------------------------------------------------------------\r\n";

    if(LoaderModule!=NULL)
        Out += LoaderModule->DescribeState();
    else
        Out += "[Loader] (module NULL)\r\n";

    fwrite(Out.c_str(), 1, Out.Length(), f);
    fflush(f);
    fclose(f);
}
//---------------------------------------------------------------------------
void cStateRecordHT160::CaptureConfig(AnsiString DstRootWithSlash)
{
    ForceDirectories(DstRootWithSlash);

    AnsiString Root = GetProjectRoot();

    // 1) system\ folder (General.ini, Mot_Table.csv, IO_Table.csv, etc.)
    AnsiString SysDst = DstRootWithSlash + "system\\";
    ForceDirectories(SysDst);
    int nSys = CopyFolderFiles(Root + "\\system", SysDst);

    // 2) current recipe folder (data\<recipe>\ setup.ini etc.)
    AnsiString RecipeName = RecipeManager.GetCurrentRecipeName();
    AnsiString RecipeDir  = RecipeManager.GetRecipeDirName();
    AnsiString RecDst = DstRootWithSlash + "recipe\\";
    if(RecipeName!=AnsiString(""))
        RecDst += RecipeName + "\\";
    ForceDirectories(RecDst);
    int nRec = CopyFolderFiles(RecipeDir, RecDst);

    // 3) manifest summarising what was packaged
    TIniFile *Ini = new TIniFile(DstRootWithSlash + "ConfigManifest.ini");
    Ini->WriteString ("Source", "ProjectRoot", Root);
    Ini->WriteString ("Source", "SystemDir",   Root + "\\system");
    Ini->WriteString ("Source", "RecipeName",  RecipeName);
    Ini->WriteString ("Source", "RecipeDir",   RecipeDir);
    Ini->WriteInteger("Count",  "SystemFiles", nSys);
    Ini->WriteInteger("Count",  "RecipeFiles", nRec);
    delete Ini;
}
//---------------------------------------------------------------------------
//AI(ht160s-secsgem) 20260611 : package today's SECS/GEM log into the snapshot,
//  but ONLY when the SECS paid feature is enabled (matches GemInitial gating).
//  Source = D:\HT160S_Log\SECS_GEM\yyyy_mm_dd ; Dst = <snapshot>\SecsLog\<day>.
void cStateRecordHT160::CaptureSecsLog(AnsiString DstRootWithSlash)
{
    if(!CosFunction.bUseSecsGem)
        return;   // feature not purchased -> no SECS log to ship

    // Flush the latest in-memory lines to disk so the snapshot is up-to-date.
    if(HGem!=NULL)
    {
        try { HGem->FlushSecsLogToFile(); } catch(...) {}
    }

    Word y,mo,d;
    DecodeDate(Now(), y, mo, d);
    AnsiString Day;
    Day.sprintf("%04d_%02d_%02d", (int)y,(int)mo,(int)d);

    AnsiString SecsSrc = HSys.LogRootDir + "\\SECS_GEM\\" + Day;
    if(DirectoryExists(SecsSrc)==false)
        return;   // nothing logged today

    AnsiString SecsDst = DstRootWithSlash + "SecsLog\\" + Day + "\\";
    ForceDirectories(SecsDst);
    CopyFolderFiles(SecsSrc, SecsDst);
}
//---------------------------------------------------------------------------
//AI(ht160s-lot-webapi) 20260612 : package today's Lot WebAPI log into the snapshot.
//  Source = D:\HT160S_Log\WebAPI\YYYYMMDD ; Dst = <snapshot>\WebApiLog\YYYYMMDD.
//  No feature flag needed : the log dir only exists when a pull actually ran
//  (manual UsePull path or SECS LOTSTART), so its presence == feature was used.
void cStateRecordHT160::CaptureWebApiLog(AnsiString DstRootWithSlash)
{
    Word y,mo,d;
    DecodeDate(Now(), y, mo, d);
    AnsiString Day;
    Day.sprintf("%04d%02d%02d", (int)y,(int)mo,(int)d);

    AnsiString WebSrc = HSys.LogRootDir + "\\WebAPI\\" + Day;
    if(DirectoryExists(WebSrc)==false)
        return;   // no WebAPI activity today -> nothing to ship

    AnsiString WebDst = DstRootWithSlash + "WebApiLog\\" + Day + "\\";
    ForceDirectories(WebDst);
    CopyFolderFiles(WebSrc, WebDst);
}
//---------------------------------------------------------------------------
bool cStateRecordHT160::TriggerSnapshot(AnsiString Reason)
{
    if(Reason==AnsiString(""))
        Reason = "Manual";

    EnsureInited();

    AnsiString Stamp   = MakeStamp();
    AnsiString TempDir = SaveRoot + Stamp + "\\";

    ForceDirectories(SaveRoot);
    if(ForceDirectories(TempDir)==false)
        return false;

    WriteSnapshotIni    (TempDir + "Snapshot.ini",    Reason, Stamp);
    WriteTaskHistoryCsv (TempDir + "TaskHistory.csv");
    WriteCurrentTasksTxt(TempDir + "CurrentTasks.txt");
    WriteMachineStateIni(TempDir + "MachineState.ini", Reason, Stamp);
    WriteLotDataJson    (TempDir + "LotData.json",     Reason, Stamp);   //AI(ht160s-lot-webapi) 20260612 : full lot + 2D detail as JSON
    WriteMotionDetailIni(TempDir + "MotionDetail.ini");   //AI(ht160s-state-record-analysis) 20260612 : motor pos + SortArm sub-task + sucker vacuum
    WriteSortArmDecisionTxt(TempDir + "SortArmDecision.txt");   //AI(ht160s-state-record-analysis) 20260616 : held-IC routing + per-Auto cell map (place/discharge deadlock evidence)
    WriteFeederDecisionTxt(TempDir + "FeederDecision.txt");   //AI(ht160s-state-record-analysis) 20260622 : Color/Empty/Loader latch + config-gate dump
    CaptureConfig       (TempDir + "MachineConfig\\");
    CaptureSecsLog      (TempDir);   //AI(ht160s-secsgem) 20260611 : include SECS log if feature on
    CaptureWebApiLog    (TempDir);   //AI(ht160s-lot-webapi) 20260612 : include Lot WebAPI log if any pull ran today

    AnsiString ZipPath = SaveRoot + Stamp + ".zip";
    bool bZipped = CompressFolder(TempDir, ZipPath);

    if(bZipped)
    {
        LastSnapshotZip = ZipPath;        //AI(general) 20260608 : remember zip path for Explorer /select
        DeleteFolderRecursive(TempDir);   // keep only the zip on success
    }

    return bZipped;
}
//---------------------------------------------------------------------------
