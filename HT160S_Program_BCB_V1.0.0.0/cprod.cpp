//---------------------------------------------------------------------------

#include "IncludeAllHeader.h"
#include <stdio.h>
#pragma hdrstop

#include "cprod.h"
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
PASS_WORD USER;
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
void SavePassword()
{
    AnsiString FileName=HSys.CurrentDir+"\\system\\login.dat";   //AI(HT160S-Maintainer) 20260609 : keep login data under system folder
    WriteData(FileName.c_str(), (char *)&USER, sizeof(USER));
}
//---------------------------------------------------------------------------
void ReadPassword()
{
    AnsiString FileName=HSys.CurrentDir+"\\system\\login.dat";   //AI(HT160S-Maintainer) 20260609 : keep login data under system folder
    if(ReadData(FileName.c_str(), (char *)&USER, sizeof(USER))==false)
        ZeroMemory(&USER, sizeof(USER));
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