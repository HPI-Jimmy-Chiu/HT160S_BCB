//----------------------------------------------------------------------------
// MyBinDisp.cpp
// AI(ht160s-maintainer) 20260615 : LED-only Bin display controller, ported from
// HT172 TMyBinDispCtrl / TMyBinDispHT9046. TFT path NOT ported. Adaptations:
//   TQPF_Timer.SetSecAndOn(n) -> HTimer.Set(n)+On(); log -> TStringList buffer;
//   MyDBIProcess -> LogBinDisplay; the HT172 256-bin sliding special case (which
//   needed iTestBinCount/BinSelect) is removed - HT160 uses one label per unit.
//----------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#include <stdio.h>
#include <string.h>
#pragma hdrstop

#include "MyBinDisp.h"
#include "cCommLog.h"
#include "GeneralSetting.h"
//----------------------------------------------------------------------------
#pragma package(smart_init)
//----------------------------------------------------------------------------
#define Bin_CR        13
#define Bin_LF        10
//----------------------------------------------------------------------------
const unsigned char T_HEX2ASCII[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
const unsigned char T_ASXII2HEX[] = {0,1,2,3,4,5,6,7,8,9,0,0,0,0,0,0,0,10,11,12,13,14,15};
//----------------------------------------------------------------------------
TMyBinDispCtrl::TMyBinDispCtrl()
{
    for(int i=0; i<Bin_MAX_NUM; i++)
    {
        bHasUnitArray[i]=false;
        bSetBin[i]=false;
        for(int j=0; j<TEST_MAX_BIN; j++)
            iSetBin[i][j]=-1;
        iSetColor[i]=1;
        Alias[i]="";
        bSliding[i]=true;
        iVersion[i]=0;
        iBinNow[i]=0;
        iColorNow[i]=1;
        bHasError[i]=false;
    }

    slBinDispLog=new TStringList;
    iSetColor[0]=3;
    iSetColor[1]=3;
    iSetColor[2]=3;

    bStopProcess=false;
    bStartSetColor=false;
    bStartSetBin=false;
    bHasUnit=false;
    BinDispRecv=false;
    ComPort="";
    ComParity=None;

    iDelaySec=5;
    InitialOK=false;

    iTotalInstalledUnit=-1;
    iBinDispCtrlTask=1;
    bFirstInit=true;
    iRusStatus=0;
    iUsedBinNumber=0;
    iTestBinCount=47;
    bTimerRun=false;
    Addr=0;
    iStartSetBinTask=1;
    iStartSetColorTask=1;
    iStartGetStatusTask=1;
    CommBin=NULL;
    ZeroMemory(iErrCount, sizeof(iErrCount));
    ZeroMemory(iCount, sizeof(iCount));
    ZeroMemory(bGetStatus, sizeof(bGetStatus));
    ZeroMemory(bSetColor, sizeof(bSetColor));
}
//----------------------------------------------------------------------------
TMyBinDispCtrl::~TMyBinDispCtrl()
{
    try
    {
        if(slBinDispLog!=NULL)
        {
            delete slBinDispLog;
            slBinDispLog=NULL;
        }
    }
    catch(...)
    {
    }
}
//----------------------------------------------------------------------------
unsigned char TMyBinDispCtrl::T_HEX2ASCII_Mac(unsigned char hex2ascii) {return(T_HEX2ASCII[(hex2ascii)&0x0f]);}
unsigned char TMyBinDispCtrl::T_ASXII2HEX_Mac(unsigned char ascii2hex)
{
    if(ascii2hex-'0'>22 || ascii2hex-'0'<0)
        return 0;
    return(T_ASXII2HEX[ascii2hex-'0']);
}
void  TMyBinDispCtrl::SetComParity(TParity Parity)  {ComParity=Parity;}
bool  TMyBinDispCtrl::UnitHasInstall(int Index)     {return bHasUnitArray[Index];}
void  TMyBinDispCtrl::CloseUnit(int Index)          {bHasUnitArray[Index]=false;}
void  TMyBinDispCtrl::OpenUnit(int Index)           {bHasUnitArray[Index]=true;}
void  TMyBinDispCtrl::SetDelayTime(int Sec)         {iDelaySec=Sec;}
int   TMyBinDispCtrl::GetDelayTime()                {return iDelaySec;}
int   TMyBinDispCtrl::GetTotalInstalledUnit()       {return iTotalInstalledUnit+1;}
int   TMyBinDispCtrl::GetColorNow(int Index)        {return iColorNow[Index];}
int   TMyBinDispCtrl::GetBinNow(int Index)          {return iBinNow[Index];}
bool  TMyBinDispCtrl::GerErrNow(int Index)          {return bHasError[Index];}
void  TMyBinDispCtrl::SerErrNow(int Index,bool bErr){bHasError[Index]=bErr;}
//----------------------------------------------------------------------------
AnsiString TMyBinDispCtrl::GetRunStatus()
{
    AnsiString Message="";
    switch(iRusStatus)
    {
        case 0: Message="Initialing...";    break;
        case 1: Message="Get status...";    break;
        case 2: Message="Color Setting.";   break;
        case 3: Message="Bin Setting.";     break;
        case 4: Message="Display Error!!";  break;
        case 5: Message="Bin Running.";     break;
    }
    return Message;
}
//----------------------------------------------------------------------------
void TMyBinDispCtrl::SetComPort(AnsiString port)
{
    ComPort=port;
}
//----------------------------------------------------------------------------
void TMyBinDispCtrl::ProcessStopStart(bool Value)
{
    bStopProcess=Value;
    if(bFirstInit==true)
    {
        iBinDispCtrlTask=1;
        bStartSetColor=true;
        bStartSetBin=true;
        bFirstInit=false;
    }
    else
    {
        if(bStopProcess==true)
            iBinDispCtrlTask=50;
    }
}
//----------------------------------------------------------------------------
unsigned char TMyBinDispCtrl::A_Create_LCR(unsigned char *Sptr, unsigned char length)
{
    unsigned char Btmp,Btmp1;
    Btmp1=0;
    do
    {
        Btmp =T_ASXII2HEX_Mac(*Sptr);
        Sptr++;
        Btmp =(Btmp<<4)|T_ASXII2HEX_Mac(*Sptr);
        Btmp1+=Btmp;
        Sptr++;
        --length;
        if(length==0)
            break;
    }
    while(--length);
    return ((~Btmp1)+1);
}
//----------------------------------------------------------------------------
void __fastcall TMyBinDispCtrl::CommBinReceiveData(TObject *Sender,
      Pointer Buffer, WORD BufferLength)
{
    if(BufferLength>=1024)
        return;
    ZeroMemory(BinDispCom2Buffer, sizeof(BinDispCom2Buffer));
    strncpy(BinDispCom2Buffer, (char*)Buffer, BufferLength);
    BinDispCom2Buffer[BufferLength]='\x0';

    sReadBuffer.sprintf("%s", AnsiString(BinDispCom2Buffer));
    if(sReadBuffer=="")
        return;

    AnsiString asHex=Chararr2Hexstring(BinDispCom2Buffer,BufferLength);
    LogBinDisplay("Recv", asHex, true);

    BinDispRecv=true;
}
//----------------------------------------------------------------------------
bool TMyBinDispCtrl::GetCOMPortStatus(AnsiString Com)
{
    HANDLE h=INVALID_HANDLE_VALUE;
    AnsiString CN="\\\\.\\"+Com;
    h=::CreateFile(CN.c_str(),
        GENERIC_READ|GENERIC_WRITE,
        0,
        0,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
        0);
    CloseHandle(h);
    if(h==INVALID_HANDLE_VALUE)
        return false;
    return true;
}
//----------------------------------------------------------------------------
void TMyBinDispCtrl::InstalledUnit(int Index)
{
    bHasUnitArray[Index]=true;
    bHasUnit=true;
    bSetBin[Index]=true;
    if(iTotalInstalledUnit<Index)
        iTotalInstalledUnit=Index;
}
//----------------------------------------------------------------------------
void TMyBinDispCtrl::Spin()
{
    AnsiString Str;
    if(InitialOK==false)
        return;
    if(bStopProcess==false)
        return;
    if(bTimerRun)
        return;

    bTimerRun=true;
    int &Task=iBinDispCtrlTask;
    AnsiString CN;

    switch(Task)
    {
        case 1:
            iRusStatus=0;
            if(ComPort.Pos("COM")==0)
            {
                bTimerRun=false;
                return;
            }
            bStartSetBin=true;
            for(int i=0; i<iUsedBinNumber; i++)
            {
                bHasError[i]=false;
            }
            #ifndef SOFT_SIMULATE
            bStartSetColor=true;
            if(GetCOMPortStatus(ComPort))
            {
                StartComport(CommBin,ComPort);
                Task=50;
                iStartGetStatusTask=1;
            }
            else
            {
                StopComport(CommBin,ComPort);
            }
            #else
                bStartSetColor=false;
                Task=50;
                iStartGetStatusTask=1;
            #endif
            break;
        case 50:
            if(DoStartGetStatus())
            {
                Task=100;
            }
            break;
        case 100:
            if(bHasUnit==false)
                break;
            #ifndef SOFT_SIMULATE
            if(GetCOMPortStatus(ComPort))
            {
                iRusStatus=0;
                CommBin->StopComm();

                for(int i=0; i<iUsedBinNumber; i++)
                {
                    iBinNow[i]=0;
                    iColorNow[i]=1;
                }

                bStartSetColor=true;
                bStartSetBin=true;
                Task=1;
                break;
            }
            #endif
            // AI(ht160s-maintainer) 20260615 : HT172's LED Spin never reached the
            // color pass (case 200 was unreachable). HT160 needs per-unit color
            // (old-160 behavior), so run the color pass first when one is pending.
            if(bStartSetColor)
            {
                iStartSetColorTask=1;
                Task=200;
            }
            else if(bStartSetBin)
            {
                iStartSetBinTask=1;
                Task=300;
            }
            else
            {
                iStartSetBinTask=100;
                Task=300;
            }
            break;
        case 200:
            if(DoStartSetColor())
            {
                bStartSetColor=false;
                Task=100;
            }
            break;
        case 300:
            if(DoStartSetBin())
            {
                Task=100;
            }
            break;
    }
    bTimerRun=false;
}
//----------------------------------------------------------------------------
void TMyBinDispCtrl::WriteTargetBin(int Index, int *bin, int color)
{
    if(Index>iUsedBinNumber)
        return;
    if(Index<0)
        return;

    // bin = -1      : blank (shows X)
    // bin = 0~99    : digits 0~99
    // bin = 100~125 : letters A~Z

    iSetColor[Index]=color;
    for(int i=0; i<TEST_MAX_BIN; i++)
    {
        iSetBin[Index][i]=bin[i];
        bStartSetBin=true;
        bStartSetColor=true;
        bSetBin[Index]=true;
    }
}
//----------------------------------------------------------------------------
void TMyBinDispCtrl::WriteTargetBin(int color)
{
    bStartSetBin=true;
    bStartSetColor=true;
    for(int i=0; i<Bin_MAX_NUM; i++)
    {
        iSetColor[i]=color;
        bSetBin[i]=true;
    }
}
//----------------------------------------------------------------------------
void TMyBinDispCtrl::SetUnitLabel(int Index, int value, int color)
{
    if(Index<0 || Index>=Bin_MAX_NUM)
        return;
    iSetColor[Index]=color;
    iSetBin[Index][0]=value;
    iSetBin[Index][1]=-1;
    bSliding[Index]=true;
    bSetBin[Index]=true;
    bSetColor[Index]=true;
    bStartSetBin=true;
    bStartSetColor=true;
}
//----------------------------------------------------------------------------
void TMyBinDispCtrl::SetUnitBin(int Index, int value)
{
    if(Index<0 || Index>=Bin_MAX_NUM)
        return;
    iSetBin[Index][0]=value;
    iSetBin[Index][1]=-1;
    bSliding[Index]=true;
    bSetBin[Index]=true;
    bStartSetBin=true;
}
//----------------------------------------------------------------------------
void TMyBinDispCtrl::SetUnitColor(int Index, int color)
{
    if(Index<0 || Index>=Bin_MAX_NUM)
        return;
    iSetColor[Index]=color;
    bSetColor[Index]=true;
    bStartSetColor=true;
}
//----------------------------------------------------------------------------
void TMyBinDispCtrl::LogBinDisplay(AnsiString asAction, AnsiString asMessage, bool bMemo)
{
    if(slBinDispLog==NULL)
        return;
    AnsiString asLine=FormatDateTime("yyyy-mm-dd hh:nn:ss", Now())+", "+asAction+", "+asMessage;
    slBinDispLog->Add(asLine);
    while(slBinDispLog->Count>500)
        slBinDispLog->Delete(0);

    //AI(general) 20260617 : routine TX/Recv frames flood the daily CSV during
    //production (one pair per bin update). Persist them only when verbose tracing
    //is enabled; always persist non-routine events (open/close/errors/no-reply)
    //so faults stay traceable. The 500-line in-memory memo above is unaffected.
    bool bRoutineFrame = (asAction == "TX" || asAction == "Recv");
    if(GeneralSetting.bBinDispLogVerbose || bRoutineFrame == false)
        g_BinDispCommLog.Log(asAction, asMessage);
    // P3 TODO: when bMemo, also echo to the ComPort bin memo.
}
//----------------------------------------------------------------------------
void TMyBinDispCtrl::AddBinDisplayLog(AnsiString asAction, AnsiString asMessage)
{
    LogBinDisplay(asAction, asMessage, false);
}
//----------------------------------------------------------------------------
void TMyBinDispCtrl::FlushBinDisplayLog()
{
    if(slBinDispLog==NULL)
        return;
    try
    {
        slBinDispLog->SaveToFile("BinDisplayLog.txt");
    }
    catch(...)
    {
    }
}
//----------------------------------------------------------------------------
void TMyBinDispCtrl::SetUsedBinNumber(int iNum)
{
    if(iNum>0 && iNum<Bin_MAX_NUM)
    {
        iUsedBinNumber=iNum;
    }
    else
    {
        iUsedBinNumber=Bin_MAX_NUM;
    }
}
//----------------------------------------------------------------------------
AnsiString TMyBinDispCtrl::Chararr2Hexstring(char* cstr, int iNum)
{
    AnsiString tempStr="";
    for(int i=0; i<iNum; i++)
        tempStr+=" "+AnsiString().sprintf("%02X",(Byte)cstr[i]);
    return tempStr;
}
//----------------------------------------------------------------------------
bool TMyBinDispCtrl::StartComport(TComm *Comm,AnsiString port)
{
    bool bret=false;
    AnsiString CN="", Str="";
    try
    {
        if(Comm==NULL)
        {
            SetComPort(port);
            SetDelayTime(100);
            InitialOK=true;
            return false;
        }
        Comm->Parity=ComParity;
        Comm->OnReceiveData=CommBinReceiveData;
        CN="\\\\.\\"+port;
        Comm->CommName=CN;
        Comm->StartComm();
        Str.sprintf("BinDisp, Start Comm OK., %s", Comm->CommName);
        LogBinDisplay("Open", Str, true);
        bret=true;
    }
    catch(...)
    {
        ShowMyMessage("Error open com port");
        if(Comm!=NULL)
            Str.sprintf("BinDisp, Start Comm NG!, %s", Comm->CommName);
        LogBinDisplay("OpenNG", Str, true);
    }
    return bret;
}
//----------------------------------------------------------------------------
bool TMyBinDispCtrl::StopComport(TComm *Comm,AnsiString port)
{
    bool bret=false;
    AnsiString CN="", Str="";
    try
    {
        if(Comm==NULL)
        {
            SetComPort(port);
            SetDelayTime(100);
            InitialOK=true;
            return false;
        }
        Comm->Parity=ComParity;
        Comm->OnReceiveData=CommBinReceiveData;
        CN="\\\\.\\"+port;
        Comm->CommName=CN;
        Comm->StopComm();
        Str.sprintf("BinDisp, Stop Comm OK., %s", Comm->CommName);
        LogBinDisplay("Close", Str, true);
        bret=true;
    }
    catch(...)
    {
        ShowMyMessage("Error close com port");
        if(Comm!=NULL)
            Str.sprintf("BinDisp, Stop Comm NG!, %s", Comm->CommName);
        LogBinDisplay("CloseNG", Str, true);
    }
    return bret;
}
//============================================================================
// TMyBinDispHT9046 : concrete LED serial protocol
//============================================================================
void TMyBinDispHT9046::WriteBin(int Addr, int Command, short Value)
{
    unsigned char Btmp1;
    AnsiString Str;
    // AI(ht160s-maintainer) 20260616 : the HT9046 board is a single Modbus slave
    // at the high (color) address; bin/version share that address and differ only
    // by register. The inherited Addr+1 (low) target had no listener on this
    // machine - use the same Addr+32(+38) hex base as WriteColor.
    int wAddr=(Addr>=10)?(Addr+38):(Addr+32);
    sprintf(SendBuffer, ":%02X06008%d00%02d00%c%c", wAddr, Command, Value, Bin_CR, Bin_LF);

    Btmp1=A_Create_LCR((unsigned char*)&SendBuffer[1], 12);
    SendBuffer[13]=T_HEX2ASCII_Mac(Btmp1>>4);
    SendBuffer[14]=T_HEX2ASCII_Mac(Btmp1);
    CommBin->WriteCommData(SendBuffer, strlen(SendBuffer));

    Str.sprintf(":%02X06008%d00%02d00, WriteBin", wAddr, Command, Value);
    LogBinDisplay("TX", Str, true);
}
//----------------------------------------------------------------------------
void TMyBinDispHT9046::WriteColor(int Addr, short Value)
{
    unsigned char Btmp1;
    AnsiString Str;
    if(Addr>=10)
        sprintf(SendBuffer, ":%02X06008200%02d00%c%c", Addr+38, Value, Bin_CR, Bin_LF);
    else
        sprintf(SendBuffer, ":%02X06008200%02d00%c%c", Addr+32, Value, Bin_CR, Bin_LF);
    Btmp1=A_Create_LCR((unsigned char*)&SendBuffer[1], 12);
    SendBuffer[13]=T_HEX2ASCII_Mac(Btmp1>>4);
    SendBuffer[14]=T_HEX2ASCII_Mac(Btmp1);
    CommBin->WriteCommData(SendBuffer, strlen(SendBuffer));

    if(Addr>=10)
        Str.sprintf(":%02X06008200%02d, WriteColor", Addr+38, Value);
    else
        Str.sprintf(":%02X06008200%02d, WriteColor", Addr+32, Value);
    LogBinDisplay("TX", Str, true);
}
//----------------------------------------------------------------------------
void TMyBinDispHT9046::ReadVersion(int Addr)
{
    unsigned char Btmp1;
    AnsiString Str;
    // AI(ht160s-maintainer) 20260616 : read version from the same high address as
    // WriteColor (see WriteBin note). The old Addr+1 target never replied, so
    // iVersion stayed 0 and every bin/color reply was rejected as NoReply.
    int wAddr=(Addr>=10)?(Addr+38):(Addr+32);
    sprintf(SendBuffer, ":%02X030080000100%c%c", wAddr, Bin_CR, Bin_LF);

    Btmp1=A_Create_LCR((unsigned char*)&SendBuffer[1], 12);
    SendBuffer[13]=T_HEX2ASCII_Mac(Btmp1>>4);
    SendBuffer[14]=T_HEX2ASCII_Mac(Btmp1);
    CommBin->WriteCommData(SendBuffer, strlen(SendBuffer));

    Str.sprintf(":%02X0300800001, ReadVersion", wAddr);
    LogBinDisplay("TX", Str, true);
}
//----------------------------------------------------------------------------
bool TMyBinDispHT9046::DoStartSetBin()
{
    AnsiString sCheckWord="";
    int &Task=iStartSetBinTask;

    if(bStartSetBin==true)
    {
        iRusStatus=3;
    }
    else
    {
        iRusStatus=5;
    }

    switch(Task)
    {
        case 1:
            Addr=0;
            Task=100;
            for(int i=0; i<iUsedBinNumber; i++)
            {
                bSliding[i]=true;
                bSetBin[i]=true;
                iCount[i]=0;
            }
            break;
        case 100:
            while(1)
            {
                if(Addr>=iUsedBinNumber)
                {
                    Addr=0;
                    if(bStartSetBin)
                    {
                        Task=1000;
                        bStartSetBin=false;
                    }
                    break;
                }

                if(bSliding[Addr]==true && bSetBin[Addr]==true && bHasUnitArray[Addr])
                {
                    if(iSetBin[Addr][iCount[Addr]]==-1)
                    {
                        iCount[Addr]=0;
                    }

                    if(iSetBin[Addr][0]==-1)                    // not set, show X
                    {
                        WriteBin(Addr, 1, 123-100);
                    }
                    else if(iSetBin[Addr][iCount[Addr]]<100)    // digit
                    {
                        WriteBin(Addr, 0, iSetBin[Addr][iCount[Addr]]);
                    }
                    // AI(ht160s-maintainer) 20260617 : HT160 layout puts Color
                    // at the last unit (index 8); 9045 Addr<=3 only covered its
                    // low-index L/E/C, so include the Color unit to send its letter.
                    else if(Addr<=3 || Addr==BIN_DISP_UNIT_COUNT-1 || iSetBin[Addr][iCount[Addr]]==104)
                    {
                        WriteBin(Addr, 1, iSetBin[Addr][iCount[Addr]]-100);
                    }
                    else
                    {
                        Addr++;
                        break;
                    }

                    Task=200;
                    BinDispRecv=false;
                    BinDisDelay.Set(2);
                    BinDisDelay.On();
                    break;
                }
                Addr++;
            }
            break;
        case 200:
            if(BinDispRecv)
            {
                if(Addr>=iUsedBinNumber)
                {
                    Task=100;
                    break;
                }
                // AI(ht160s-maintainer) 20260616 : reply echoes the high address
                // (Addr+32/+38), so the checkword must use that same hex base.
                int wAddr=(Addr>=10)?(Addr+38):(Addr+32);
                if(iVersion[Addr]==1)
                {
                    sCheckWord.sprintf(":%02X06020010", wAddr);
                }
                else if(iVersion[Addr]==2)
                {
                    if(iSetBin[Addr][0]==-1)
                        sCheckWord.sprintf(":%02X060201%02d", wAddr, 123-100);
                    else if(iSetBin[Addr][iCount[Addr]]<100)
                        sCheckWord.sprintf(":%02X060200%02d", wAddr, iSetBin[Addr][iCount[Addr]]);
                    else
                        sCheckWord.sprintf(":%02X060201%02d", wAddr, iSetBin[Addr][iCount[Addr]]-100);
                }

                if(sReadBuffer.Pos(sCheckWord)==1)
                {
                    iBinNow[Addr]=iSetBin[Addr][iCount[Addr]];
                    bHasError[Addr]=false;
                    iErrCount[Addr]=0;

                    iCount[Addr]++;
                    if(iSetBin[Addr][iCount[Addr]]==-1 ||
                       iCount[Addr]>=iTestBinCount)
                    {
                        iCount[Addr]=0;
                    }
                    if(iSetBin[Addr][0]==-1 || iSetBin[Addr][1]==-1)
                    {
                        bSliding[Addr]=false;
                    }
                    Addr++;
                    if(Addr>=iUsedBinNumber)
                    {
                        Task=100;
                        break;
                    }
                }
                else
                {
                    iErrCount[Addr]++;
                }
                Task=100;
            }
            else if(BinDisDelay.Off())
            {
                iErrCount[Addr]++;
                Task=100;
            }

            if(Addr>=iUsedBinNumber)
            {
                Addr=0;
            }
            if(iErrCount[Addr]>2)
            {
                //AI(ht160s-maintainer) 20260616 : do not block the whole bus on a
                //silent unit. Log the anomaly, flag the unit, clear its work flags
                //and advance to the next address so other units still get updated.
                AnsiString asErr;
                asErr.sprintf("Addr=%d, no Bin reply, skip", Addr+1);
                LogBinDisplay("BinNoReply", asErr, true);
                iRusStatus=4;
                bHasError[Addr]=true;
                bSliding[Addr]=false;
                bSetBin[Addr]=false;
                iErrCount[Addr]=0;
                Addr++;
                Task=100;
            }
            break;
        case 1000:
            BinDisDelay.Set(iDelaySec);
            BinDisDelay.On();
            Task=1100;
        case 1100:
            if(BinDisDelay.Off())
            {
                for(int i=0; i<iUsedBinNumber ; i++)
                    if(bHasUnitArray[i])
                        bSetBin[i]=true;
                return true;
            }
            break;
    }
    return false;
}
//----------------------------------------------------------------------------
bool TMyBinDispHT9046::DoStartSetColor()
{
    AnsiString sCheckWord="";
    int &Task=iStartSetColorTask;
    iRusStatus=2;
    switch(Task)
    {
        case 1:
            Addr=0;
            Task=100;
            for(int i=0; i<iUsedBinNumber; i++)
                bSetColor[i]=true;
            break;
        case 100:
            while(1)
            {
                if(Addr>=iUsedBinNumber)
                    return true;

                if(bSetColor[Addr]==true && bHasUnitArray[Addr])
                {
                    WriteColor(Addr, iSetColor[Addr]);
                    Task=200;
                    BinDispRecv=false;
                    BinDisDelay.Set(2);
                    BinDisDelay.On();
                    break;
                }
                Addr++;
            }
            break;
        case 200:
            if(BinDispRecv)
            {
                if(iVersion[Addr]==1)
                {
                    if(Addr>=10)
                        sCheckWord.sprintf(":%02X06020010", Addr+38);
                    else
                        sCheckWord.sprintf(":%02X06020010", Addr+32);
                }
                else if(iVersion[Addr]==2)
                {
                    if(Addr>=10)
                        sCheckWord.sprintf(":%02X060202%02d", Addr+38, iSetColor[Addr]);
                    else
                        sCheckWord.sprintf(":%02X060202%02d", Addr+32, iSetColor[Addr]);
                }

                if(sReadBuffer.Pos(sCheckWord)==1)
                {
                    iColorNow[Addr]=iSetColor[Addr];
                    bHasError[Addr]=false;
                    iErrCount[Addr]=0;
                    bSetColor[Addr]=false;
                }
                else
                {
                    iErrCount[Addr]++;
                }
                Task=100;
            }
            else if(BinDisDelay.Off())
            {
                iErrCount[Addr]++;
                Task=100;
            }

            if(iErrCount[Addr]>2)
            {
                //AI(ht160s-maintainer) 20260616 : a unit that never echoes its color
                //used to force a full StopComm + restart, stalling every other unit.
                //Now just log + flag the unit and move on to the next address.
                AnsiString asErr;
                asErr.sprintf("Addr=%d, no Color reply, skip", Addr+1);
                LogBinDisplay("ColorNoReply", asErr, true);
                iRusStatus=4;
                bHasError[Addr]=true;
                bSetColor[Addr]=false;
                iErrCount[Addr]=0;
                Addr++;
                Task=100;
            }
            break;
    }
    return false;
}
//----------------------------------------------------------------------------
bool TMyBinDispHT9046::DoStartGetStatus()
{
    AnsiString sCheckWord="";
    int &Task=iStartGetStatusTask;
    iRusStatus=1;

    switch(Task)
    {
        case 1:
            Addr=0;
            Task=100;
            for(int i=0; i<iUsedBinNumber; i++)
            {
                bGetStatus[i]=true;
                bHasUnitArray[Addr]=true;
            }
            break;
        case 100:
            while(1)
            {
                if(Addr>=iUsedBinNumber)
                    return true;

                if(bGetStatus[Addr]==true)
                {
                    ReadVersion(Addr);
                    Task=200;
                    BinDispRecv=false;
                    BinDisDelay.Set(2);
                    BinDisDelay.On();
                    break;
                }
                Addr++;
            }
            break;
        case 200:
            if(BinDispRecv)
            {
                // AI(ht160s-maintainer) 20260616 : version reply echoes the high
                // address (Addr+32/+38); match the checkword to it.
                int wAddr=(Addr>=10)?(Addr+38):(Addr+32);
                sCheckWord.sprintf(":%02X03020001", wAddr);
                if(sReadBuffer.Pos(sCheckWord)==1)
                {
                    iVersion[Addr]=1;
                }
                else
                {
                    sCheckWord.sprintf(":%02X03020002", wAddr);

                    if(sReadBuffer.Pos(sCheckWord)==1)
                    {
                        iVersion[Addr]=2;
                    }
                    else
                    {
                        iVersion[Addr]=0;
                    }
                }
                if(iVersion[Addr]!=0)
                {
                    bHasUnitArray[Addr]=true;
                    bGetStatus[Addr]=false;
                    iErrCount[Addr]=0;
                }
                else
                {
                    iErrCount[Addr]++;
                }
                Task=100;
            }
            else if(BinDisDelay.Off())
            {
                iErrCount[Addr]++;
                Task=100;
            }

            if(iErrCount[Addr]>2)
            {
                //AI(ht160s-maintainer) 20260616 : a unit that never returns its
                //version used to be retried forever, blocking GetStatus from ever
                //completing. Log + flag it, clear its poll flag and skip ahead.
                AnsiString asErr;
                asErr.sprintf("Addr=%d, no Version reply, skip", Addr+1);
                LogBinDisplay("VerNoReply", asErr, true);
                iRusStatus=4;
                bHasError[Addr]=true;
                bGetStatus[Addr]=false;
                iVersion[Addr]=0;
                iErrCount[Addr]=0;
                Addr++;
                Task=100;
            }
            break;
    }
    return false;
}
//----------------------------------------------------------------------------
