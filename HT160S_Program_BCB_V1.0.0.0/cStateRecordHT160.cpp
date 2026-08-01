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
#include "uHome.h"            //AI(ht160s-state-record-analysis) 20260624 : fHome IsShown/SeenStart for the HOME-lifecycle snapshot
#include "aSortArm.h"         //AI(ht160s-state-record-analysis) 20260612 : SortArmModule sub-task readout (TTrayMotor/TMyKitSuck come via database.h)
#include "aAuto1To6.h"        //AI(ht160s-state-record-analysis) 20260616 : AutoModule per-station cell map for SortArmDecision.txt
#include "aColor.h"           //AI(ht160s-state-record-analysis) 20260622 : ColorModule->DescribeState()
#include "aEmpty.h"           //AI(ht160s-state-record-analysis) 20260622 : EmptyModule->DescribeState()
#include "aLoader.h"          //AI(ht160s-state-record-analysis) 20260622 : LoaderModule->DescribeState()
#include "aTrayArm.h"         //AI(ht160s-rearready-p0) 20260705 : TrayArmModule->DescribeState() + PickTask/Job export
#include "uHGemEquipment.h"   //AI(ht160s-secsgem) 20260611 : HGem->FlushSecsLogToFile()
#include "uAgvStation.h"      //AI(ht160s-agv) 20260625 : AgvCoord.DescribeAgvState() for the AMR handshake block in FeederDecision.txt
#pragma package(smart_init)
//---------------------------------------------------------------------------
static const AnsiString SR_VERSION = "HT160S 1.0.0.0";
//---------------------------------------------------------------------------
cStateRecordHT160 *gStateRecord = NULL;
//---------------------------------------------------------------------------
//AI(diag-av) TEMP breadcrumb tracer for the WriteLotDataJson access-violation
//hunt. Appends one line per step to D:\HT160S_StateRecord\_ldj_trace.txt with a
//flush after every write so the LAST line survives an OS crash. REMOVE after fix.
static void SR_Trace(const char *Msg)
{
    FILE *f = fopen("D:\\HT160S_StateRecord\\_ldj_trace.txt", "ab");
    if(f==NULL)
        return;
    AnsiString T = FormatDateTime("hh:nn:ss.zzz", Now());
    fprintf(f, "%s %s\r\n", T.c_str(), Msg);
    fflush(f);
    fclose(f);
}
//---------------------------------------------------------------------------
//AI(ht160s-state-record-analysis) 20260801 : fixed-width formatting helpers for the IO
//dump. Loops rather than a ternary, because a ternary that yields an AnsiString has
//crashed BCB6 elsewhere in this tree.
static AnsiString SR_PadR(AnsiString S, int Width)
{
    while(S.Length() < Width)
        S += " ";
    return S;
}
//---------------------------------------------------------------------------
static AnsiString SR_PadL(AnsiString S, int Width)
{
    while(S.Length() < Width)
        S = " " + S;
    return S;
}
//---------------------------------------------------------------------------
//AI(ht160s-state-record-analysis) 20260801 : tri-state IO cell. -1 means the point is
//DISABLED in IO_Table.csv, where printing "0" would be a lie.
static AnsiString SR_Tri(int V)
{
    if(V < 0)
        return "-";
    if(V == 0)
        return "0";
    return "1";
}
//---------------------------------------------------------------------------
cStateRecordHT160::cStateRecordHT160()
{
    ModuleCount = 0;
    bInited     = false;
    bPrevRunGate = false;                 //AI(ht160s-obsv) 20260724
    SaveRoot    = "D:\\HT160S_StateRecord\\";

    for(int i=0; i<SR_MAX_MODULE; i++)
    {
        Modules[i].Name      = "";
        Modules[i].LastTask  = -1;
        Modules[i].bHasLast  = false;
        Modules[i].bStuckFired = false;   //AI(ht160s-obsv-p1)
        Modules[i].WatchBase = Now();     //AI(ht160s-obsv) 20260724 : real base set at EnsureInited seed
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
    TDateTime tNow = Now();
    M->Hist[M->HistHead].Time = tNow;
    M->Hist[M->HistHead].Task = Task;
    M->HistHead = (M->HistHead + 1) % SR_MAX_HISTORY;
    if(M->HistCount < SR_MAX_HISTORY)
        M->HistCount++;

    M->LastTask  = Task;
    M->bHasLast  = true;
    M->WatchBase = tNow;   //AI(ht160s-obsv) 20260724 : stuck-clock base = last task change (rebased again on production resume in CheckStuckWatchdog)
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
        Modules[i].bStuckFired = false;   //AI(ht160s-obsv-p1)
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
        {
            PushSample(i, Tag);
            Modules[i].bStuckFired = false;   //AI(ht160s-obsv-p1) : task moved -> re-arm episode
        }
    }
    CheckStuckWatchdog();
}
//---------------------------------------------------------------------------
//AI(ht160s-obsv-p1) 20260720 : generic stuck watchdog. StuckMs was computed for the
//dumps but nothing consumed it live - a module wedged mid-production idled forever
//with no alarm and no evidence. One auto snapshot per stuck episode per module
//(re-armed when its Task changes); HOME rounds excluded via the RunMode gate.
//AI(ht160s-obsv) 20260724 : FIX false fire on resume. A Pause/Stop drops SystemStart
//(MachinePause, csystem.cpp) which freezes task sampling while Now() keeps advancing.
//The old dMs = Now()-lastTaskChangeTime therefore counted the whole paused span, so
//the watchdog fired the instant the operator resumed (stale >threshold carry-over,
//e.g. the 2026-07-23 17:12:07 StuckWatchdog snapshot that was really the 16:59 pause).
//Fix (minimal) : each module's stuck clock is measured from WatchBase, and WatchBase
//is REBASED to now on every false->true edge of the production gate below, so a resume
//grants a fresh full window of ACTUAL running before any module can trip. WatchBase is
//also set to the last task-change time in PushSample, so a module that keeps advancing
//never trips. Pause == Stop for this gate (both drop SystemStart), so mid-run pauses
//also rebase and cannot inflate the clock.
void cStateRecordHT160::CheckStuckWatchdog()
{
    if(GeneralSetting.iStuckSnapshotSec<=0)
        return;

    bool bRunGate = (HSys.Sys.SystemStart!=false) &&
                    (HSys.Sys.RunMode==Run_Normal || HSys.Sys.RunMode==Run_CleanOut);
    TDateTime tNow = Now();
    if(bRunGate && bPrevRunGate==false)
    {
        for(int i=0; i<ModuleCount; i++)
        {
            Modules[i].WatchBase   = tNow;
            Modules[i].bStuckFired = false;
        }
    }
    bPrevRunGate = bRunGate;
    if(bRunGate==false)
        return;

    AnsiString sStuck;
    for(int i=0; i<ModuleCount; i++)
    {
        if(Modules[i].bHasLast==false || Modules[i].bStuckFired)
            continue;
        double dMs=double(tNow-Modules[i].WatchBase)*86400000.0;
        if(dMs > double(GeneralSetting.iStuckSnapshotSec)*1000.0)
        {
            Modules[i].bStuckFired=true;
            sStuck+=" "+Modules[i].Name+"(task "+IntToStr(Modules[i].LastTask)+")";
        }
    }
    if(sStuck!="")
    {
        RecordProcess("STUCK watchdog: task unchanged >"+IntToStr(GeneralSetting.iStuckSnapshotSec)+
            "s while running:"+sStuck+" - auto snapshot");
        TriggerSnapshot("StuckWatchdog");
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
    //AI(ht160s-state-record-analysis) 20260624 : HOME-lifecycle inputs. "HOME doesn't move" is
    //almost always the SystemStart-gated home engine : ProcessMotion returns early when
    //SystemStart==0, so CheckMotorHome / fHome->ProcessMotorHome never step the axes. These show
    //WHY - whether the monitor is up (the ScanSystemSenser motor-power SystemStart-drops are
    //guarded ONLY while fHome is shown), the power-cycle/settle state, and if home already finished.
    Ini->WriteInteger("System", "fHomeShown",        (fHome!=NULL && fHome->IsShown()) ? 1 : 0);
    Ini->WriteInteger("System", "fHomeSeenStart",    (fHome!=NULL && fHome->SeenStart()) ? 1 : 0);
    Ini->WriteInteger("System", "fAllMotorHome",     fAllMotorHome ? 1 : 0);
    Ini->WriteInteger("System", "MotorPowerOnDelay", MotorPowerOnDelay);
    Ini->WriteInteger("System", "bMotorPowerState",  bMotorPowerState ? 1 : 0);
    Ini->WriteInteger("System", "bHomePowerCycling", bHomePowerCycling ? 1 : 0);

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

    SR_Trace("MSI before delete Ini");
    delete Ini;
    SR_Trace("MSI after delete Ini");
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
    SR_Trace("LDJ 01 enter");
    TStringList *Lines=new TStringList;
    TStringList *Ic=new TStringList;
    SR_Trace("LDJ 02 after new TStringList x2");
    try
    {
        AnsiString ActiveLot="";
        if(fMain!=NULL && fMain->edLotNo!=NULL)
            ActiveLot=fMain->edLotNo->Text;
        SR_Trace("LDJ 03 after edLotNo read");

        Lines->Add("{");
        Lines->Add("  \"Snapshot\": { \"Time\": \""+SR_JsonEsc(Stamp)+"\", \"TriggerReason\": \""+SR_JsonEsc(Reason)+"\", \"Version\": \""+AnsiString(SR_VERSION)+"\" },");
        Lines->Add("  \"ActiveLot\": \""+SR_JsonEsc(ActiveLot)+"\",");
        Lines->Add("  \"LotCount\": "+IntToStr(LotRegistry.GetLotCount())+",");
        Lines->Add("  \"ItemCount\": "+IntToStr(LotRegistry.GetItemCount())+",");
        Lines->Add("  \"Lots\": [");

        SR_Trace("LDJ 04 before GetLotSlotCount");
        int SlotCount=LotRegistry.GetLotSlotCount();
        SR_Trace(("LDJ 05 SlotCount=" + IntToStr(SlotCount)).c_str());
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
                //AI(ht160s-kyec) 20260722 : row = Code2D \t Bin \t HBin \t SBin \t RetestCode
                // \t DiePass \t CustLotID(f6) \t ProductCode(f7) \t Substage(f8); dump CustLotID
                // so a snapshot shows which customer lot each IC belongs to under its KYEC lot.
                AnsiString Row=Ic->Strings[k];
                AnsiString f[7];
                for(int fi=0;fi<7;fi++)
                    f[fi]="";
                int fidx=0, start=1;
                for(int p=1;p<=Row.Length() && fidx<7;p++)
                {
                    if(Row[p]=='\t')
                    {
                        f[fidx]=Row.SubString(start, p-start);
                        fidx++;
                        start=p+1;
                    }
                }
                if(fidx<7)
                    f[fidx]=Row.SubString(start, Row.Length()-start+1);

                AnsiString IcComma = (k==0) ? AnsiString("") : AnsiString(",");
                AnsiString Bin  = (f[1].Trim()=="") ? AnsiString("0") : f[1].Trim();
                AnsiString HBin = (f[2].Trim()=="") ? AnsiString("0") : f[2].Trim();
                AnsiString SBin = (f[3].Trim()=="") ? AnsiString("0") : f[3].Trim();
                AnsiString Item="        "+IcComma+"{ \"Code2D\": \""+SR_JsonEsc(f[0])+"\""+
                    ", \"Bin\": "+Bin+", \"HBin\": "+HBin+", \"SBin\": "+SBin+
                    ", \"RetestCode\": \""+SR_JsonEsc(f[4])+"\""+
                    ", \"DiePass\": \""+SR_JsonEsc(f[5])+"\""+
                    ", \"CustLotID\": \""+SR_JsonEsc(f[6])+"\" }";
                Lines->Add(Item);
            }
            Lines->Add("      ]");
            Lines->Add("    }");
        }
        Lines->Add("  ],");
        SR_Trace("LDJ 06 after lot loop");

        //AI(ht160s-lotbin) 20260615 : dump the dynamic (Lot,key)->Auto table so a snapshot
        //taken in a dynamic mode shows which Auto each pair was bound to.
        //AI(ht160s-lotpassfail) 20260709 : 3-way mode name. NOTE the "Bin" field below carries
        //a PASS/FAIL class (1=PASS,2=FAIL) when SortMode is "LotPassFail".
        //AI(ht160s-lotpassfail) 20260730 : the PassBin dump is gone with the setting - the class
        //now comes from each IC's DiePass, which the per-lot 2D dump above already carries.
        {
            SR_Trace("LDJ 07 before IsLot*SortMode");
            //AI(ht160s-whitelist-override) 20260717 : 4-way name off the EFFECTIVE mode (base +
            //WhiteList overlay). Also dump the base mode and the overlay flag so a WhiteList
            //snapshot is distinguishable from a base-only run.
            AnsiString SortModeName;
            switch(GeneralSetting.GetEffectiveSortMode())
            {
                case smLotBin:      SortModeName="LotBin";      break;
                case smLotPassFail: SortModeName="LotPassFail"; break;
                case smWhiteList:   SortModeName="WhiteList";   break;
                default:            SortModeName="Normal";      break;
            }
            Lines->Add("  \"SortMode\": \""+SortModeName+"\",");
            Lines->Add("  \"BaseSortMode\": "+IntToStr(GeneralSetting.iSortMode)+",");
            Lines->Add("  \"WhiteListActive\": "+IntToStr(GeneralSetting.bWhiteListActive?1:0)+",");
        }
        Lines->Add("  \"LotBinBinding\": [");
        {
            SR_Trace("LDJ 09 before GetBindingCount");
            int BindCount=LotBinBinding.GetBindingCount();
            SR_Trace(("LDJ 10 BindCount=" + IntToStr(BindCount)).c_str());
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

        SR_Trace("LDJ 11 before SaveToFile");
        Lines->SaveToFile(Path);
        SR_Trace("LDJ 12 after SaveToFile (OK)");
    }
    __finally
    {
        SR_Trace("LDJ 13 finally (delete lists)");
        delete Ic;
        delete Lines;
    }
    SR_Trace("LDJ 14 return");
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
        //AI(ht160s-state-record-analysis) 20260801 : 'err' is ONLY the soft-limit reject
        //flag (bErrorMove is set solely when CheckSoftLimit rejects a target), so it reads
        //0 while a latched servo alarm has stopped the machine - on 2026-07-31 all 20 axes
        //showed err=0 with SystemStart=0 and the snapshot could not tell an operator stop
        //from a servo fault. Keep the old key for any existing parser, add the real
        //amplifier/status layer beside it. Same key, longer value: anything that splits on
        //", " and looks for "cmd=" is unaffected.
        Val += ", err=" + IntToStr(M->bErrorMove ? 1 : 0);
        Val += ", limrej=" + IntToStr(M->bErrorMove ? 1 : 0);
        Val += ", hfin=" + IntToStr(M->bHomeFinish ? 1 : 0);
        Val += ", en=" + IntToStr(M->GetEnable() ? 1 : 0);
        Val += ", svalm=" + IntToStr(M->ReadServoAlarmOn() ? 1 : 0);
        Val += ", erridx=" + IntToStr(M->GetErrorIndex());
        Val += ", spd=" + IntToStr(M->GetSpeed());
        Val += ", pct=" + IntToStr(M->GetPersentSpeed());
        Val += ", slim=" + IntToStr(M->GetSoftLimitN()) + ".." + IntToStr(M->GetSoftLimitP());
        Val += ", lastHome=" + IntToStr(M->GetLastHomePos());
        //Led[] is free: ScanAllMotorStatus() already refreshes all axes every MainProc
        //cycle. Order is HTMotor.h: CW HOME CCW EMG ALM SCW SCCW SVALM INPOS Z SVON.
        AnsiString Leds = "";
        for(int b=0; b<iMotLedTotalCnt; b++)
        {
            if(M->Led[b])
                Leds += "1";
            else
                Leds += "0";
        }
        Val += ", led=" + Leds;
        Val += ", locks=" + IntToStr(M->GetLockCount());
        Ini->WriteString("Motors", Key, Val);
    }

    //AI(ht160s-state-record-analysis) 20260801 : who owns each axis. TMyMotor::Lock()
    //already records the claiming function and Task; it was simply never dumped, so a
    //stalled axis could not be attributed to a caller. Written ONLY for locked axes, so
    //the normal snapshot gains zero keys.
    for(int lm=0; lm<HSys.iTotalMotor; lm++)
    {
        TTrayMotor *M = (HSys.MotPtr!=NULL) ? HSys.MotPtr[lm] : NULL;
        if(M==NULL)
            continue;
        int LockN = M->GetLockCount();
        if(LockN <= 0)
            continue;

        AnsiString LKey = M->Alias;
        if(LKey==AnsiString(""))
            LKey = "Motor" + IntToStr(lm);

        AnsiString LVal = "";
        for(int li=0; li<LockN; li++)
        {
            if(li > 0)
                LVal += " | ";
            LVal += M->GetLockString(li);
        }
        Ini->WriteString("MotorLocks", LKey, LVal);
    }

    // ---- [SortArm] : sub-task + Z-safe readout ----
    if(SortArmModule!=NULL)
    {
        Ini->WriteInteger("SortArm", "PickTask",  SortArmModule->GetPickTask());
        Ini->WriteInteger("SortArm", "PlaceTask", SortArmModule->GetPlaceTask());
    }
    Ini->WriteInteger("SortArm", "SafeZ", 10);   // SORT_ARM_SAFE_Z_POSITION (aSortArm.cpp)

    // ---- [TrayArm] : sub-task readout (ht160s-rearready-p0 20260705; top-level
    // DoTrayArm Task is only 1/10/100/1000/2000 -- the rear-ready gate wait lives in
    // PickTask 1/10 and was invisible) ----
    if(TrayArmModule!=NULL)
    {
        Ini->WriteInteger("TrayArm", "PickTask",  TrayArmModule->GetPickTask());
        Ini->WriteInteger("TrayArm", "PlaceTask", TrayArmModule->GetPlaceTask());
        Ini->WriteInteger("TrayArm", "Job",       TrayArmModule->GetJob());
    }

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
                //AI(ht160s-state-record-analysis) 20260801 : 'vac=' was a permanently-zero
                //dead field - TMySucker::Status has exactly one writer in the whole tree
                //(the constructor, setting it false) and this line was its only reader, so
                //every snapshot ever taken reported vac=0 on every nozzle. That is the one
                //value the 2026-07-23 "nozzle in position but no vacuum" investigation
                //needed. Report the driver's real state instead: the two output bits (RAM)
                //and the LIVE vacuum sensor. Full per-nozzle detail is in IoDetail.txt;
                //this stays so the historical key keeps a truthful value.
                TMySucker *P = &Kit.Suck[r][c];
                int iVac = -1;
                if(P->Sensor.Enable)
                {
                    iVac = 0;
                    if(P->Sensor.IsOn())
                        iVac = 1;
                }
                AnsiString SK = "S" + IntToStr(r) + "_" + IntToStr(c);
                AnsiString SV = "on=" + IntToStr(P->GetOnBit() ? 1 : 0);
                SV += ", off=" + IntToStr(P->GetOffBit() ? 1 : 0);
                SV += ", vac=" + SR_Tri(iVac);
                Ini->WriteString(KitSection, SK, SV);
            }
        }
    }

    delete Ini;
}
//---------------------------------------------------------------------------
//AI(ht160s-state-record-analysis) 20260801 : IoDetail.txt - the whole-registry IO photo.
//
//WHY: a State Record used to carry 5 of 39 cylinder out-bits (all from one module's
//DescribeState, added ad hoc for one past investigation) and NO reed sensor at all, so
//"the cylinder did not reach position" could never be separated from "the reed is dead".
//The ~18 sensor values it did carry are module LATCHES (aColor.cpp says so in-source),
//i.e. what the module believes - which is exactly the fact that cannot be cross-checked.
//
//READS ARE LIVE AND MUST STAY THAT WAY: every level here goes through TMySensor::IsOn(),
//never a GetStatus()-style accessor. TMySucker::GetStatus() and friends return true
//unconditionally when HSys.LastSet.iRealDummy != REALLY, which would paint an all-green
//picture on any DUMMY/HAS_TRAY bench - the same trap that made iosetview switch to
//Sensor.IsOn(). Equally, nothing here calls a module helper (IsRearReadyForPick,
//RefreshStateFromSensors, ...): those WRITE module latches, and a dump must not.
//
//SAFETY: single-threaded (MainProc and the UI are the same VCL thread), so no locking is
//needed and no race is possible. Cost is ~138 enabled bit reads, roughly 8 MainProc
//cycles' worth, against a snapshot that already blocks this thread for seconds. Outputs
//(switch/cylinder out bits) are pure RAM and cost no card traffic. Bounded for-loops only,
//no blocking, no modal.
void cStateRecordHT160::WriteIoDetailTxt(AnsiString Path)
{
    FILE *f = fopen(Path.c_str(), "wb");
    if(f==NULL)
        return;

    AnsiString L;
    L  = "HT160S IO detail (live registry sweep)\r\n";
    L += "SampleTime=" + FormatDateTime("hh:nn:ss.zzz", Now()) + "\r\n";
    L += "Levels are LIVE reads (TMySensor::IsOn). '-' = point disabled in IO_Table.csv.\r\n";
    L += "\r\n";
    fwrite(L.c_str(), 1, L.Length(), f);

    // ---- [Cylinders] ----
    L  = "[Cylinders] total=" + IntToStr(HSys.iTotalCylinder) + "\r\n";
    L += "Idx | Name                           | En | Out | SnOn | SnOff | Verdict     | OnTmo | OffTmo | OnDly | OffDly | AlmOn | AlmOff\r\n";
    L += "----+--------------------------------+----+-----+------+-------+-------------+-------+--------+-------+--------+-------+-------\r\n";
    fwrite(L.c_str(), 1, L.Length(), f);
    for(int c=0; c<HSys.iTotalCylinder; c++)
    {
        if(HSys.CynPtr==NULL)
            break;
        TMyCylinder *C = &HSys.CynPtr[c];

        int iOut  = 0;
        if(C->GetOutBit())
            iOut = 1;

        //NB TMyCylinder::IsOn() reads OnSensor, IsOff() reads OffSensor - both are
        //"that reed is asserted", not logical opposites.
        int iSnOn = -1;
        if(C->OnSensor.Enable)
        {
            iSnOn = 0;
            if(C->IsOn())
                iSnOn = 1;
        }
        int iSnOff = -1;
        if(C->OffSensor.Enable)
        {
            iSnOff = 0;
            if(C->IsOff())
                iSnOff = 1;
        }

        //Verdict is DERIVED here; it adds no machine state. It exists so an operator's
        //alarm number maps straight to a row and to a physical reading.
        AnsiString Verdict;
        if(iSnOn < 0 && iSnOff < 0)
            Verdict = "NO_SENSOR";
        else if(iSnOn == 1 && iSnOff == 1)
            Verdict = "CONTRADICT";
        else if(iOut == 1 && iSnOn == 1)
            Verdict = "OUT_OK";
        else if(iOut == 0 && iSnOff == 1)
            Verdict = "IN_OK";
        else if(iOut == 1 && iSnOn == 0)
            Verdict = "UNCONFIRMED";
        else if(iOut == 0 && iSnOn == 1)
            Verdict = "MISMATCH";
        else
            Verdict = "-";

        AnsiString Name = C->CylinderName;
        if(Name==AnsiString(""))
            Name = "Cyl" + IntToStr(c);

        L  = SR_PadL(IntToStr(c), 3);
        L += " | " + SR_PadR(Name, 30);
        L += " | " + SR_PadL(SR_Tri(C->Enable ? 1 : 0), 2);
        L += " | " + SR_PadL(SR_Tri(iOut), 3);
        L += " | " + SR_PadL(SR_Tri(iSnOn), 4);
        L += " | " + SR_PadL(SR_Tri(iSnOff), 5);
        L += " | " + SR_PadR(Verdict, 11);
        L += " | " + SR_PadL(IntToStr(C->OnAlarmTime), 5);
        L += " | " + SR_PadL(IntToStr(C->OffAlarmTime), 6);
        L += " | " + SR_PadL(IntToStr(C->OnDelayTime), 5);
        L += " | " + SR_PadL(IntToStr(C->OffDelayTime), 6);
        //Alarm codes are formatted 4<idx:03d><err:1d>, so printing them lets a reported
        //number (e.g. 40020) be resolved to this exact row without opening the source.
        L += " | " + SR_PadL(IntToStr(C->OnAlarmCode), 5);
        L += " | " + SR_PadL(IntToStr(C->OffAlarmCode), 6);
        L += "\r\n";
        fwrite(L.c_str(), 1, L.Length(), f);
    }

    // ---- [Sensors] ----
    L  = "\r\n[Sensors] total=" + IntToStr(HSys.iTotalSensor) + "\r\n";
    L += "Idx | Name                           | En | Typ | Address              | Live\r\n";
    L += "----+--------------------------------+----+-----+----------------------+-----\r\n";
    fwrite(L.c_str(), 1, L.Length(), f);
    for(int s=0; s<HSys.iTotalSensor; s++)
    {
        if(HSys.SenPtr==NULL)
            break;
        TMySensor *S = &HSys.SenPtr[s];

        int iLive = -1;
        if(S->Enable)
        {
            iLive = 0;
            if(S->IsOn())
                iLive = 1;
        }

        //Type is printed alongside the level because IsOn() inverts unless Type==1 - with
        //both, the raw wire level is recoverable.
        AnsiString Addr;
        if(S->Input!=NULL)
        {
            Addr = "Lane" + IntToStr(S->Input->GetLane())
                 + " IP" + IntToStr(S->Input->GetIP())
                 + " P"  + IntToStr(S->Input->GetPort())
                 + " B"  + IntToStr(S->Input->GetBit());
        }
        else
        {
            Addr = "Card" + IntToStr(S->Card)
                 + " P"   + IntToStr(S->Port)
                 + " B"   + IntToStr(S->Bit);
        }

        AnsiString Name = S->Name;
        if(Name==AnsiString(""))
            Name = "Sen" + IntToStr(s);

        L  = SR_PadL(IntToStr(s), 3);
        L += " | " + SR_PadR(Name, 30);
        L += " | " + SR_PadL(SR_Tri(S->Enable ? 1 : 0), 2);
        L += " | " + SR_PadL(IntToStr(S->Type), 3);
        L += " | " + SR_PadR(Addr, 20);
        L += " | " + SR_PadL(SR_Tri(iLive), 4);
        L += "\r\n";
        fwrite(L.c_str(), 1, L.Length(), f);
    }

    // ---- [Switches] : pure RAM, zero card traffic ----
    L  = "\r\n[Switches] total=" + IntToStr(HSys.iTotalSwitch) + "\r\n";
    L += "Idx | Name                           | En | Out\r\n";
    L += "----+--------------------------------+----+----\r\n";
    fwrite(L.c_str(), 1, L.Length(), f);
    for(int w=0; w<HSys.iTotalSwitch; w++)
    {
        if(HSys.SwPtr==NULL)
            break;
        TMySwitch *W = &HSys.SwPtr[w];

        AnsiString Name = W->Name;
        if(Name==AnsiString(""))
            Name = "Sw" + IntToStr(w);

        L  = SR_PadL(IntToStr(w), 3);
        L += " | " + SR_PadR(Name, 30);
        L += " | " + SR_PadL(SR_Tri(W->Enable ? 1 : 0), 2);
        L += " | " + SR_PadL(SR_Tri(W->OutValue ? 1 : 0), 3);
        L += "\r\n";
        fwrite(L.c_str(), 1, L.Length(), f);
    }

    // ---- [Suckers] : the live vacuum level MotionDetail.ini never had ----
    L  = "\r\n[Suckers] totalKit=" + IntToStr(HSys.iTotalSucker) + "\r\n";
    L += "Kit | R | C | Name                     | OnBit | OffBit | Vac | SensorName\r\n";
    L += "----+---+---+--------------------------+-------+--------+-----+-----------\r\n";
    fwrite(L.c_str(), 1, L.Length(), f);
    for(int k=0; k<HSys.iTotalSucker; k++)
    {
        if(HSys.SuckPtr==NULL)
            break;
        TMyKitSuck &Kit = HSys.SuckPtr[k];
        for(int r=0; r<Kit.MaxItemR; r++)
        {
            for(int cc=0; cc<Kit.MaxItemC; cc++)
            {
                TMySucker *P = &Kit.Suck[r][cc];

                int iVac = -1;
                if(P->Sensor.Enable)
                {
                    iVac = 0;
                    if(P->Sensor.IsOn())
                        iVac = 1;
                }

                AnsiString Name = P->SuckerName;
                if(Name==AnsiString(""))
                    Name = "Suck" + IntToStr(r) + "_" + IntToStr(cc);

                L  = SR_PadL(IntToStr(k), 3);
                L += " | " + SR_PadL(IntToStr(r), 1);
                L += " | " + SR_PadL(IntToStr(cc), 1);
                L += " | " + SR_PadR(Name, 24);
                L += " | " + SR_PadL(SR_Tri(P->GetOnBit() ? 1 : 0), 5);
                L += " | " + SR_PadL(SR_Tri(P->GetOffBit() ? 1 : 0), 6);
                L += " | " + SR_PadL(SR_Tri(iVac), 3);
                L += " | " + P->SensorName;
                L += "\r\n";
                fwrite(L.c_str(), 1, L.Length(), f);
            }
        }
    }

    fflush(f);
    fclose(f);
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
        //AI(auto-obsv) 20260801 : module cursors first. All six stations share ONE
        //FeedTask / DischargeTask / CleanOutTask and one iFeedAuto / iDischargeAuto, so
        //"which Auto is the module actually serving" is a module fact, not a station fact,
        //and it was in no snapshot at all until now.
        Out += AutoModule->DescribeModule();
        Out += "------------------------------------------------------------\r\n";
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
         + "  iSortMode=" + IntToStr(GeneralSetting.iSortMode)
         + "  WhiteListActive=" + IntToStr(GeneralSetting.bWhiteListActive ? 1 : 0)
         + "  EffSortMode=" + IntToStr(GeneralSetting.GetEffectiveSortMode()) + "\r\n";
    Out += "  bUsePredictiveAutoSupply=" + IntToStr(GeneralSetting.bUsePredictiveAutoSupply ? 1 : 0)
         + "  bUseAmrRecoveryDivert=" + IntToStr(GeneralSetting.bUseAmrRecoveryDivert ? 1 : 0) + "\r\n";
    Out += "  bUseColorCcd=" + IntToStr(CosFunction.bUseColorCcd ? 1 : 0)
         + "  bUseTopCcd=" + IntToStr(CosFunction.bUseTopCcd ? 1 : 0)
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
    Out += "------------------------------------------------------------\r\n";

    //AI(ht160s-rearready-p0) 20260705 : TrayArm block (report 5.2) -- the consumer of
    //every rear-ready gate above. PickTask 1/10 + Job here, against the producer
    //latches above, is what arbitrates "arm waiting on a gate" vs "arm idle / never
    //dispatched" offline.
    if(TrayArmModule!=NULL)
        Out += TrayArmModule->DescribeState();
    else
        Out += "[TrayArm] (module NULL)\r\n";

    //AI(ht160s-agv) 20260625 : AMR coordinator handshake block. Lets offline analysis
    //distinguish a PREP-stall vs a READY-stall vs a link drop (per-station lock +
    //handshake state + live ready-gate + SECS Selected + bUseAMR), alongside the
    //per-module feeder latches above - the missing view when an AMR lock freezes a feeder.
    Out += "------------------------------------------------------------\r\n";
    Out += "[AMR coordinator]\r\n";
    Out += AgvCoord.DescribeAgvState();

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
//AI(ht160s-obsv-p1) 20260720 : package today's + yesterday's EventLog CSV into the
//snapshot (the zip is what operators ship; without the narrative the state dump has
//no timeline). Mirrors CaptureSecsLog; yesterday covers midnight-spanning shifts.
void cStateRecordHT160::CaptureEventLog(AnsiString DstRootWithSlash)
{
    AnsiString Dst = DstRootWithSlash + "EventLog\\";
    for(int iBack=0; iBack<2; iBack++)
    {
        Word y,mo,d;
        DecodeDate(Now()-iBack, y, mo, d);
        AnsiString Mon; Mon.sprintf("%04d_%02d", (int)y,(int)mo);
        AnsiString Fn;  Fn.sprintf("HT160S_%04d_%02d_%02d.csv", (int)y,(int)mo,(int)d);
        AnsiString Src = HSys.LogRootDir + "\\EventLog\\" + Mon + "\\" + Fn;
        if(FileExists(Src)==false)
            continue;
        ForceDirectories(Dst);
        CopyOneFile(Src, Dst + Fn);
    }
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

    SR_Trace("=== TriggerSnapshot start ===");
    //AI(ht160s-state-record-analysis) 20260801 : VOLATILE READERS FIRST. Motor positions
    //and IO levels are queried from the card live, so every millisecond spent writing
    //something else first makes them staler relative to the trigger instant. LotData.json
    //alone walks every lot slot and every 2D item (13.5 kB on site) and used to run BEFORE
    //the motion dump. Everything below the volatile block is latched or already on disk and
    //cannot change while we run - the module ladders are on this same single thread and are
    //therefore frozen for the whole snapshot.
    //NB the two TrayArm triggers decel-stop before snapshotting, but the StuckWatchdog and
    //the manual button do not, so ordering matters most in exactly the cases that catch a
    //live fault.
    WriteSnapshotIni    (TempDir + "Snapshot.ini",    Reason, Stamp);
    SR_Trace("TS after WriteSnapshotIni");
    WriteMotionDetailIni(TempDir + "MotionDetail.ini");   //AI(ht160s-state-record-analysis) 20260612 : motor pos + SortArm sub-task + sucker vacuum
    SR_Trace("TS after WriteMotionDetailIni");
    WriteIoDetailTxt    (TempDir + "IoDetail.txt");       //AI(ht160s-state-record-analysis) 20260801 : whole-registry cylinder/sensor/switch/sucker sweep
    SR_Trace("TS after WriteIoDetailTxt");

    WriteTaskHistoryCsv (TempDir + "TaskHistory.csv");
    SR_Trace("TS after WriteTaskHistoryCsv");
    WriteCurrentTasksTxt(TempDir + "CurrentTasks.txt");
    SR_Trace("TS after WriteCurrentTasksTxt");
    WriteMachineStateIni(TempDir + "MachineState.ini", Reason, Stamp);
    SR_Trace("TS after WriteMachineStateIni");
    WriteLotDataJson    (TempDir + "LotData.json",     Reason, Stamp);   //AI(ht160s-lot-webapi) 20260612 : full lot + 2D detail as JSON
    SR_Trace("TS after WriteLotDataJson");
    WriteSortArmDecisionTxt(TempDir + "SortArmDecision.txt");   //AI(ht160s-state-record-analysis) 20260616 : held-IC routing + per-Auto cell map (place/discharge deadlock evidence)
    WriteFeederDecisionTxt(TempDir + "FeederDecision.txt");   //AI(ht160s-state-record-analysis) 20260622 : Color/Empty/Loader latch + config-gate dump
    CaptureConfig       (TempDir + "MachineConfig\\");
    CaptureSecsLog      (TempDir);   //AI(ht160s-secsgem) 20260611 : include SECS log if feature on
    CaptureWebApiLog    (TempDir);   //AI(ht160s-lot-webapi) 20260612 : include Lot WebAPI log if any pull ran today
    CaptureEventLog     (TempDir);   //AI(ht160s-obsv-p1) 20260720 : ship the narrative with the state

    AnsiString ZipPath = SaveRoot + Stamp + ".zip";
    bool bZipped = CompressFolder(TempDir, ZipPath);

    if(bZipped)
    {
        LastSnapshotZip = ZipPath;        //AI(general) 20260608 : remember zip path for Explorer /select
        DeleteFolderRecursive(TempDir);   // keep only the zip on success
    }
    RecordProcess(AnsiString("SNAPSHOT ")+Reason+(bZipped?" ok ":" FAILED ")+ZipPath);   //AI(ht160s-obsv-p2) : 7z failure was swallowed

    return bZipped;
}
//---------------------------------------------------------------------------
