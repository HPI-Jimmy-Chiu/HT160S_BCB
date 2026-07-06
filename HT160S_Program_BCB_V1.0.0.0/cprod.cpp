//---------------------------------------------------------------------------

#include "IncludeAllHeader.h"
#include <stdio.h>
#pragma hdrstop

#include "cprod.h"
#include "uteach.h"   //AI 20260623 : TEACH Teach/TeachBase for Offset fold
#include "uOffset.h"  //AI 20260623 : RUN_OFFSET Offset for Offset fold
#include "UserRoleManager.h"   //AI(ht160s-password) 20260624 : text login book persistence
#include "aAuto1To6.h"         //AI(ht160s-uph) 20260706 : AutoModule/GetStationStatus/AS_SORTING (read-only observer)
#include "cCsvDailyLog.h"      //AI(ht160s-uph) 20260706 : PruneFolderTree + CsvQuote
#include "GeneralSetting.h"    //AI(ht160s-uph) 20260706 : iLogRetentionUPHLogDays
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
TRunData tRunData;
TLatchCycleTime lctLoader;
TFunction tFunction;
TSimulationData tSimuData;
TMotionnetIO tMotionnetIO;
TRAY_DATA TrayDef;
SYSTEM_BIN_SELECT BinSelect[2];
RUN_INFO RunInfo;
RUN_INFO RunInfo2;
MACHINE_RUN_INFO MachineRun;
//---------------------------------------------------------------------------
void MACHINE_RUN_INFO::Clear()
{
    bRunning=false;
    iSystemStatus=0;
    iUPH=0;
    iTotalScanned=0;
    iTotalSorted=0;
    iUnknown2D=0;
    iActiveLotCount=0;
    for(int i=0;i<eTrayCount;i++)
        iAreaCount[i]=0;
    szRunStatus[0]=0;
}
//---------------------------------------------------------------------------
static AnsiString GetLastDataFileName()
{
    return HSys.CurrentDir+"\\system\\lastdata.dat";   //AI(HT160S-Maintainer) 20260609 : store beside other system data instead of beside the EXE
}
//---------------------------------------------------------------------------
int GetJamRateDenom()
{
    return tRunData.JamRateDenom;
}
//---------------------------------------------------------------------------
int TLatchCycleTime::LatchCycleTime(bool Start)
{
    if(Start)
        iCount=0;
    return iCount++;
}
//---------------------------------------------------------------------------
bool WriteData(char *cFName,char *ptr,int size)
{
    FILE *Fp=fopen(cFName,"wb");
    bool bResult=false;

    if(Fp!=NULL)
    {
        bResult=(fwrite(ptr,size,1,Fp)==1);
        fclose(Fp);
    }
    return bResult;
}
//---------------------------------------------------------------------------
bool ReadData(char *cFName,char *ptr,int size)
{
    FILE *Fp=fopen(cFName,"rb");
    bool bResult=false;

    if(Fp!=NULL)
    {
        bResult=(fread(ptr,size,1,Fp)==1);
        fclose(Fp);
    }
    return bResult;
}
//---------------------------------------------------------------------------
bool CheckFileExist(char *cFName)
{
    if(cFName==NULL)
        return false;
    return FileExists(AnsiString(cFName));
}
//---------------------------------------------------------------------------
bool CheckFileCanAccess(char *cFName)
{
    FILE *Fp;

    if(cFName==NULL)
        return false;
    Fp=fopen(cFName,"rb+");
    if(Fp==NULL)
        return false;
    fclose(Fp);
    return true;
}
//---------------------------------------------------------------------------
bool WriteLastDataFile()
{
    AnsiString FileName=GetLastDataFileName();
    return WriteData(FileName.c_str(), (char *)&tRunData, sizeof(tRunData));
}
//---------------------------------------------------------------------------
bool ReadLastDataFile()
{
    AnsiString FileName=GetLastDataFileName();
    return ReadData(FileName.c_str(), (char *)&tRunData, sizeof(tRunData));
}
//---------------------------------------------------------------------------
void ClearLastSet()
{
    tRunData.Clear();
}
//---------------------------------------------------------------------------
//AI(ht160s-lot-reset) 20260706 : reset only the per-run production counters for a
//new work order. Called by the manual Lot Start button and the SECS LOTSTART
//handler. Leaves configured quantities (iAutoQuantity/iMagQuantity), the lifetime
//ejection-pin counter and run-state fields (bRunning/iActiveLotCount) to callers.
//bFirstRun is re-armed so the next production cycle re-stamps StartTime and clears
//the UPH pause accumulator (see csystem DoAllProcess bFirstRun block).
void ResetPerLotProductionCounters()
{
    MachineRun.iTotalScanned=0;
    MachineRun.iTotalSorted=0;
    MachineRun.iUnknown2D=0;
    for(int AreaIndex=0; AreaIndex<eTrayCount; AreaIndex++)
        MachineRun.iAreaCount[AreaIndex]=0;

    ZeroMemory(tRunData.TrayICCnt, sizeof(tRunData.TrayICCnt));
    ZeroMemory(tRunData.BinICCnt, sizeof(tRunData.BinICCnt));
    tRunData.TotalIC=0;
    tRunData.UPH=0;
    tRunData.LoaderIC=0;
    tRunData.JamCount=0;
    tRunData.StartTime=Now();

    bFirstRun=true;
}
//---------------------------------------------------------------------------
//AI(ht160s-password) 20260624 : the password book is now a notepad-openable
// text file (system\login.txt, one "ID,Password,Level" line per account)
// owned by THT160UserRoleManager. The former binary login.dat / PASS_WORD
// USER path was dead code (no caller) and is removed.
static AnsiString GetLoginFileName()
{
    return HSys.CurrentDir+"\\system\\login.txt";
}
//---------------------------------------------------------------------------
void SavePassword()
{
    UserRoleManager.SaveToFile(GetLoginFileName());
}
//---------------------------------------------------------------------------
void ReadPassword()
{
    AnsiString FileName=GetLoginFileName();

    UserRoleManager.LoadFromFile(FileName);
    if(UserRoleManager.GetUserCount()<=0)
    {
        ForceDirectories(ExtractFilePath(FileName));
        UserRoleManager.AddOrUpdateUser("Honprec", "27025312", ROLE_HONPREC);
        UserRoleManager.SaveToFile(FileName);
    }
}
//---------------------------------------------------------------------------
void CheckLastData()
{
    if(ReadLastDataFile()==false)
        tRunData.Clear();
}
//---------------------------------------------------------------------------
void ReadLastDataIni()
{
    ReadLastDataFile();
}
//---------------------------------------------------------------------------
void WriteLastDataIni()
{
    WriteLastDataFile();
}
//---------------------------------------------------------------------------
void UpdateAllParameter()
{
    //AI(HT160S-Maintainer) 20260623 : Offset feature fold (Route A1, persistent).
    //Effective Teach (read by motion, ~99 sites) = TeachBase (edited/saved by the
    //Teach screen -> system\\tech.ini) + Offset (per-workfile -> data\\<wf>.ofs).
    //Assignment fold (struct copy + 56 adds), NOT +=, so repeat calls are idempotent.
    //Offset==0 (no .ofs) => Teach==TeachBase => zero behavior change.
    Teach = TeachBase;
    Teach.Loader1CarFeedTrayYPosition += Offset.Loader1CarFeedTrayYPosition;
    Teach.Loader1CarDischargeTrayYPosition += Offset.Loader1CarDischargeTrayYPosition;
    Teach.Loader1CarFirstCCDYPosition += Offset.Loader1CarFirstCCDYPosition;
    Teach.Loader1CarFirstSortYPosition += Offset.Loader1CarFirstSortYPosition;
    Teach.Loader2CarFeedTrayYPosition += Offset.Loader2CarFeedTrayYPosition;
    Teach.Loader2CarDischargeTrayYPosition += Offset.Loader2CarDischargeTrayYPosition;
    Teach.Loader2CarFirstCCDYPosition += Offset.Loader2CarFirstCCDYPosition;
    Teach.Loader2CarFirstSortYPosition += Offset.Loader2CarFirstSortYPosition;
    Teach.LoaderCarFirstCCDXPosition += Offset.LoaderCarFirstCCDXPosition;
    Teach.SortArmToLoader1XPosition += Offset.SortArmToLoader1XPosition;
    Teach.SortArmToLoader2XPosition += Offset.SortArmToLoader2XPosition;
    Teach.SortArmToAuto1XPosition += Offset.SortArmToAuto1XPosition;
    Teach.SortArmToAuto2XPosition += Offset.SortArmToAuto2XPosition;
    Teach.SortArmToAuto3XPosition += Offset.SortArmToAuto3XPosition;
    Teach.SortArmToAuto4XPosition += Offset.SortArmToAuto4XPosition;
    Teach.SortArmToAuto5XPosition += Offset.SortArmToAuto5XPosition;
    Teach.SortArmToAuto6XPosition += Offset.SortArmToAuto6XPosition;
    Teach.SortArmToBottomCCDFirstXPosition += Offset.SortArmToBottomCCDFirstXPosition;
    Teach.SortArmToLoader_1_Z1Position += Offset.SortArmToLoader_1_Z1Position;
    Teach.SortArmToLoader_1_Z2Position += Offset.SortArmToLoader_1_Z2Position;
    Teach.SortArmToLoader_1_Z3Position += Offset.SortArmToLoader_1_Z3Position;
    Teach.SortArmToLoader_1_Z4Position += Offset.SortArmToLoader_1_Z4Position;
    Teach.SortArmToLoader_2_Z1Position += Offset.SortArmToLoader_2_Z1Position;
    Teach.SortArmToLoader_2_Z2Position += Offset.SortArmToLoader_2_Z2Position;
    Teach.SortArmToLoader_2_Z3Position += Offset.SortArmToLoader_2_Z3Position;
    Teach.SortArmToLoader_2_Z4Position += Offset.SortArmToLoader_2_Z4Position;
    Teach.SortArmToAuto_1_Z1Position += Offset.SortArmToAuto_1_Z1Position;
    Teach.SortArmToAuto_1_Z2Position += Offset.SortArmToAuto_1_Z2Position;
    Teach.SortArmToAuto_1_Z3Position += Offset.SortArmToAuto_1_Z3Position;
    Teach.SortArmToAuto_1_Z4Position += Offset.SortArmToAuto_1_Z4Position;
    Teach.SortArmToAuto_2_Z1Position += Offset.SortArmToAuto_2_Z1Position;
    Teach.SortArmToAuto_2_Z2Position += Offset.SortArmToAuto_2_Z2Position;
    Teach.SortArmToAuto_2_Z3Position += Offset.SortArmToAuto_2_Z3Position;
    Teach.SortArmToAuto_2_Z4Position += Offset.SortArmToAuto_2_Z4Position;
    Teach.SortArmToAuto_3_Z1Position += Offset.SortArmToAuto_3_Z1Position;
    Teach.SortArmToAuto_3_Z2Position += Offset.SortArmToAuto_3_Z2Position;
    Teach.SortArmToAuto_3_Z3Position += Offset.SortArmToAuto_3_Z3Position;
    Teach.SortArmToAuto_3_Z4Position += Offset.SortArmToAuto_3_Z4Position;
    Teach.SortArmToAuto_4_Z1Position += Offset.SortArmToAuto_4_Z1Position;
    Teach.SortArmToAuto_4_Z2Position += Offset.SortArmToAuto_4_Z2Position;
    Teach.SortArmToAuto_4_Z3Position += Offset.SortArmToAuto_4_Z3Position;
    Teach.SortArmToAuto_4_Z4Position += Offset.SortArmToAuto_4_Z4Position;
    Teach.SortArmToAuto_5_Z1Position += Offset.SortArmToAuto_5_Z1Position;
    Teach.SortArmToAuto_5_Z2Position += Offset.SortArmToAuto_5_Z2Position;
    Teach.SortArmToAuto_5_Z3Position += Offset.SortArmToAuto_5_Z3Position;
    Teach.SortArmToAuto_5_Z4Position += Offset.SortArmToAuto_5_Z4Position;
    Teach.SortArmToAuto_6_Z1Position += Offset.SortArmToAuto_6_Z1Position;
    Teach.SortArmToAuto_6_Z2Position += Offset.SortArmToAuto_6_Z2Position;
    Teach.SortArmToAuto_6_Z3Position += Offset.SortArmToAuto_6_Z3Position;
    Teach.SortArmToAuto_6_Z4Position += Offset.SortArmToAuto_6_Z4Position;
    Teach.Auto1CarFirstSortYPosition += Offset.Auto1CarFirstSortYPosition;
    Teach.Auto2CarFirstSortYPosition += Offset.Auto2CarFirstSortYPosition;
    Teach.Auto3CarFirstSortYPosition += Offset.Auto3CarFirstSortYPosition;
    Teach.Auto4CarFirstSortYPosition += Offset.Auto4CarFirstSortYPosition;
    Teach.Auto5CarFirstSortYPosition += Offset.Auto5CarFirstSortYPosition;
    Teach.Auto6CarFirstSortYPosition += Offset.Auto6CarFirstSortYPosition;
}
//---------------------------------------------------------------------------
void CustomerFunctionSelect()
{
}
//---------------------------------------------------------------------------
AnsiString GetTotalQuantityAutoPercent()
{
    int Total=tRunData.GetTotalQuantity();
    if(Total<=0)
        return "0%";
    return AnsiString((tRunData.iAutoQuantity*100)/Total)+"%";
}
//---------------------------------------------------------------------------
AnsiString GetTotalQuantityMagPercent()
{
    int Total=tRunData.GetTotalQuantity();
    if(Total<=0)
        return "0%";
    return AnsiString((tRunData.iMagQuantity*100)/Total)+"%";
}
//---------------------------------------------------------------------------
//AI(ht160s-uph) 20260706 : per-tray + per-lot UPH logging (see plan doc). Free
//functions here (not a new .cpp) so ht160s.bpr needs no edit. The per-tray timing
//is a NON-INVASIVE observer : it only READS the public TAutoModule status, so it
//never touches aAuto1To6.cpp (which a concurrent session is editing).
//---------------------------------------------------------------------------
#define UPH_MAX_AUTO 6

static AnsiString g_UphLotFolder = "";
static AnsiString g_UphLotID     = "";
static bool       g_UphActive    = false;
static int        g_UphPrevStatus[UPH_MAX_AUTO];
static TDateTime  g_UphTrayStart[UPH_MAX_AUTO];
static TDateTime  g_UphTrayStartPause[UPH_MAX_AUTO];
static int        g_UphTrayStartIC[UPH_MAX_AUTO];
static AnsiString g_UphTrayID[UPH_MAX_AUTO];
static int        g_UphLotTrayCount = 0;
//---------------------------------------------------------------------------
static AnsiString UphRootDir()
{
    return HSys.LogRootDir + AnsiString("\\UPHLog");
}
//---------------------------------------------------------------------------
static AnsiString UphSanitizeName(AnsiString sName)
{
    int i;
    char c;
    for(i=1; i<=sName.Length(); i++)
    {
        c=sName[i];
        if(c=='\\' || c=='/' || c==':' || c=='*' || c=='?' ||
           c=='"' || c=='<' || c=='>' || c=='|')
            sName[i]='_';
    }
    sName=sName.Trim();
    if(sName=="")
        sName="NA";
    return sName;
}
//---------------------------------------------------------------------------
static void UphAppendCsv(AnsiString sPath, AnsiString sHeader, AnsiString sLine)
{
    bool bNew=!FileExists(sPath);
    FILE *Fp=fopen(sPath.c_str(), "a+");
    if(Fp==NULL)
        return;
    if(bNew && sHeader!="")
    {
        AnsiString h=sHeader+"\r\n";
        fwrite(h.c_str(), 1, h.Length(), Fp);
    }
    AnsiString l=sLine+"\r\n";
    fwrite(l.c_str(), 1, l.Length(), Fp);
    fclose(Fp);
}
//---------------------------------------------------------------------------
void TrayUphLog_OnLotStart(AnsiString LotID)
{
    int i;
    AnsiString sMonth=FormatDateTime("yyyy_mm", Now());
    AnsiString sStamp=FormatDateTime("yyyymmdd_hhnnss", Now());

    g_UphLotID=LotID;
    g_UphLotFolder=UphRootDir()+"\\"+sMonth+"\\"+UphSanitizeName(LotID)+"__"+sStamp;
    ForceDirectories(g_UphLotFolder);
    g_UphLotTrayCount=0;
    for(i=0; i<UPH_MAX_AUTO; i++)
    {
        g_UphPrevStatus[i]=-1;
        g_UphTrayStart[i]=Now();
        g_UphTrayStartPause[i]=tUPH_PauseTime;
        g_UphTrayStartIC[i]=0;
        g_UphTrayID[i]="";
    }
    g_UphActive=true;
}
//---------------------------------------------------------------------------
void TrayUphLog_Tick()
{
    int i, n, cur, prev, trayIC, trayUPH;
    double dPause, dDays, dHour;
    TDateTime tEnd;
    AnsiString sPath, sHeader, sLine;

    if(g_UphActive==false)
        return;
    if(AutoModule==NULL)
        return;

    n=AutoModule->GetStationCount();
    if(n>UPH_MAX_AUTO)
        n=UPH_MAX_AUTO;

    for(i=0; i<n; i++)
    {
        cur=AutoModule->GetStationStatus(i);
        prev=g_UphPrevStatus[i];

        if(cur==AS_SORTING && prev!=AS_SORTING)
        {
            //open a tray window : snapshot time / pause / IC baseline / trayID
            g_UphTrayStart[i]=Now();
            g_UphTrayStartPause[i]=tUPH_PauseTime;
            g_UphTrayStartIC[i]=tRunData.TrayICCnt[eAuto1+i];
            g_UphTrayID[i]=AutoModule->GetWorkingTrayID(i);
        }
        else if(prev==AS_SORTING && cur!=AS_SORTING)
        {
            //close the tray window : log only if this tray actually placed IC
            trayIC=tRunData.TrayICCnt[eAuto1+i]-g_UphTrayStartIC[i];
            if(trayIC>0)
            {
                tEnd=Now();
                dPause=double(tUPH_PauseTime)-double(g_UphTrayStartPause[i]);
                dDays=double(tEnd)-double(g_UphTrayStart[i])-dPause;
                dHour=dDays*24.0;
                trayUPH=(dHour>0.0) ? (int)((double)trayIC/dHour) : 0;

                sPath=g_UphLotFolder+"\\tray_uph.csv";
                sHeader="DataTime,LotID,Auto,TrayID,TrayIC,TrayStart,TrayEnd,DurationSec,TrayUPH";
                sLine=FormatDateTime("yyyy/mm/dd hh:nn:ss", tEnd)+","+
                      cCsvDailyLog::CsvQuote(g_UphLotID)+","+
                      IntToStr(i+1)+","+
                      cCsvDailyLog::CsvQuote(g_UphTrayID[i])+","+
                      IntToStr(trayIC)+","+
                      FormatDateTime("hh:nn:ss", g_UphTrayStart[i])+","+
                      FormatDateTime("hh:nn:ss", tEnd)+","+
                      IntToStr((int)(dDays*86400.0))+","+
                      IntToStr(trayUPH);
                UphAppendCsv(sPath, sHeader, sLine);
                g_UphLotTrayCount++;
            }
        }
        g_UphPrevStatus[i]=cur;
    }
}
//---------------------------------------------------------------------------
void TrayUphLog_OnLotEnd(AnsiString LotID, int TotalIC, int LotUPH)
{
    AnsiString sPath, sHeader, sLine;

    if(g_UphActive==false)
        return;
    sPath=g_UphLotFolder+"\\lot_summary.csv";
    sHeader="LotEndTime,LotID,TotalIC,TotalUPH,TrayCount";
    sLine=FormatDateTime("yyyy/mm/dd hh:nn:ss", Now())+","+
          cCsvDailyLog::CsvQuote(LotID)+","+
          IntToStr(TotalIC)+","+
          IntToStr(LotUPH)+","+
          IntToStr(g_UphLotTrayCount);
    UphAppendCsv(sPath, sHeader, sLine);
    g_UphActive=false;
}
//---------------------------------------------------------------------------
void TrayUphLog_PruneOld()
{
    cCsvDailyLog::PruneFolderTree(UphRootDir(), GeneralSetting.iLogRetentionUPHLogDays);
}
//---------------------------------------------------------------------------