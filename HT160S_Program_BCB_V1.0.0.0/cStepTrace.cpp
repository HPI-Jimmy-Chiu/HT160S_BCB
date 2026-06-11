//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "cStepTrace.h"
#include "database.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
static const AnsiString TRACE_FLAG_FILE = "D:\\HT160S_Log\\steptrace.on";
static const AnsiString TRACE_DIR       = "D:\\HT160S_Log\\StepTrace";
static const int FLAG_RECHECK_CYCLES    = 200; // re-test flag file every N ticks
//---------------------------------------------------------------------------
static bool   s_Enabled    = false;
static bool   s_Checked    = false;
static int    s_RecheckCnt = 0;
static int    s_TickNo     = 0;
static FILE  *s_File       = NULL;

// Last written snapshot (RunMode + up to 16 action tags) for change detection.
static int    s_LastRunMode = -1;
static int    s_LastTags[16];
static int    s_LastCount    = -1;
//---------------------------------------------------------------------------
static AnsiString RunModeName(int Mode)
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
static void CloseTraceFile()
{
    if(s_File != NULL)
    {
        fclose(s_File);
        s_File = NULL;
    }
}
//---------------------------------------------------------------------------
static bool OpenTraceFile()
{
    CloseTraceFile();

    if(DataModule1 == NULL || DataModule1->UserMotion == NULL)
        return false;

    if(DirectoryExists(TRACE_DIR) == false)
        ForceDirectories(TRACE_DIR);

    AnsiString Stamp = FormatDateTime("yyyymmdd_hhnnss", Now());
    AnsiString Path  = TRACE_DIR + "\\steptrace_" + Stamp + ".csv";

    s_File = fopen(Path.c_str(), "wb");
    if(s_File == NULL)
        return false;

    // Header: tick,time,RunMode + one column per action (its Caption).
    AnsiString Header = "tick,time,RunMode";
    TActionList *Motion = DataModule1->UserMotion;
    for(int i = 0; i < Motion->ActionCount; i++)
    {
        AnsiString Col = Motion->Actions[i]->Name;
        if(Col == AnsiString(""))
            Col = "Action" + IntToStr(i);
        Header += "," + Col;
    }
    Header += "\r\n";
    fwrite(Header.c_str(), 1, Header.Length(), s_File);
    fflush(s_File);

    s_LastRunMode = -1;
    s_LastCount   = -1;
    return true;
}
//---------------------------------------------------------------------------
static void UpdateEnableState()
{
    bool FlagOn = FileExists(TRACE_FLAG_FILE);

    if(FlagOn && s_Enabled == false)
    {
        if(OpenTraceFile())
            s_Enabled = true;
    }
    else if(FlagOn == false && s_Enabled)
    {
        CloseTraceFile();
        s_Enabled = false;
    }
}
//---------------------------------------------------------------------------
void StepTraceTick()
{
    // Lazily detect the flag file, then re-check periodically so the operator
    // can start/stop without restarting the application.
    if(s_Checked == false)
    {
        s_Checked = true;
        UpdateEnableState();
    }
    else if(++s_RecheckCnt >= FLAG_RECHECK_CYCLES)
    {
        s_RecheckCnt = 0;
        UpdateEnableState();
    }

    s_TickNo++;

    if(s_Enabled == false || s_File == NULL)
        return;
    if(DataModule1 == NULL || DataModule1->UserMotion == NULL)
        return;

    TActionList *Motion = DataModule1->UserMotion;
    int Count   = Motion->ActionCount;
    int RunMode = (int)HSys.Sys.RunMode;
    if(Count > 16)
        Count = 16;

    // Only write a row when something changed (state-change "movie").
    bool Changed = (RunMode != s_LastRunMode) || (Count != s_LastCount);
    for(int i = 0; i < Count && Changed == false; i++)
    {
        if(Motion->Actions[i]->Tag != s_LastTags[i])
            Changed = true;
    }
    if(Changed == false)
        return;

    AnsiString Row = IntToStr(s_TickNo) + "," +
                     FormatDateTime("hh:nn:ss.zzz", Now()) + "," +
                     RunModeName(RunMode);
    for(int i = 0; i < Count; i++)
    {
        Row += "," + IntToStr(Motion->Actions[i]->Tag);
        s_LastTags[i] = Motion->Actions[i]->Tag;
    }
    Row += "\r\n";
    fwrite(Row.c_str(), 1, Row.Length(), s_File);
    fflush(s_File);

    s_LastRunMode = RunMode;
    s_LastCount   = Count;
}
//---------------------------------------------------------------------------
