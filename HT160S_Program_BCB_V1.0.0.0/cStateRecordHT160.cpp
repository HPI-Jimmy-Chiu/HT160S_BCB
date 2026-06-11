//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include <stdio.h>
#include "cStateRecordHT160.h"
#include "database.h"
#include "cmydef.h"
#include "CosFunction.h"
#include "main.h"
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

    AnsiString Line = "Idx | Module        | CurTask | LastChangeTime\r\n";
    fwrite(Line.c_str(), 1, Line.Length(), f);
    Line = "----+---------------+---------+----------------\r\n";
    fwrite(Line.c_str(), 1, Line.Length(), f);

    for(int m=0; m<ModuleCount; m++)
    {
        TModuleState *M = &Modules[m];
        AnsiString CurTask = "-";
        AnsiString LastTime = "-";
        if(M->HistCount > 0)
        {
            int Idx = (M->HistHead - 1 + SR_MAX_HISTORY) % SR_MAX_HISTORY;
            CurTask  = IntToStr(M->Hist[Idx].Task);
            LastTime = FormatDateTime("hh:nn:ss.zzz", M->Hist[Idx].Time);
        }

        AnsiString Name = M->Name;
        while(Name.Length() < 13) Name += " ";
        while(CurTask.Length() < 7) CurTask = " " + CurTask;

        Line = IntToStr(m);
        while(Line.Length() < 3) Line = " " + Line;
        Line += " | " + Name + " | " + CurTask + " | " + LastTime + "\r\n";
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

    for(int m=0; m<ModuleCount; m++)
    {
        int Cur = Modules[m].bHasLast ? Modules[m].LastTask : -1;
        Ini->WriteString("Tasks", Modules[m].Name, IntToStr(Cur));
    }

    Ini->WriteString("Snapshot", "Time",          Stamp);
    Ini->WriteString("Snapshot", "Version",       SR_VERSION);
    Ini->WriteString("Snapshot", "TriggerReason", Reason);

    delete Ini;
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
    CaptureConfig       (TempDir + "MachineConfig\\");
    CaptureSecsLog      (TempDir);   //AI(ht160s-secsgem) 20260611 : include SECS log if feature on

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
