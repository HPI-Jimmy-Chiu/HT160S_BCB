//----------------------------------------------------------------------------
// MyBinDisp.h
// AI(ht160s-maintainer) 20260615 : Bin display controller ported from HT172
// TMyBinDispCtrl. LED-only (HT9046 class); the HT172 TFT path is intentionally
// NOT ported. Single instance use (HSys.BinDisCtrl). COM/SPComm transport.
// Dependencies remapped to existing HT160 facilities: TQPF_Timer -> HTimer,
// TMyStringList log -> plain TStringList buffer, TextProcess/DataModule3 dropped.
//----------------------------------------------------------------------------
#ifndef MyBinDispH
#define MyBinDispH
//----------------------------------------------------------------------------
#include <Classes.hpp>
#include "SPComm.hpp"
#include "HTimer.h"
#include "cmydef.h"          // TEST_MAX_BIN
//----------------------------------------------------------------------------
#define Bin_MAX_NUM 23
// AI(ht160s-maintainer) 20260615 : HT160 addressable LED units (per P0 lock):
// 0=Empty 1=Loader 2..7=Auto1..6 8=Color.
#define BIN_DISP_UNIT_COUNT 9
//----------------------------------------------------------------------------
// Abstract base: per-unit bin display state machine driven by Spin().
// Concrete serial protocol lives in the subclass (TMyBinDispHT9046).
//----------------------------------------------------------------------------
class TMyBinDispCtrl
{
protected:
    void LogBinDisplay(AnsiString asAction, AnsiString asMessage, bool bMemo);
    int    Addr;
    HTimer BinDisDelay;
    bool   bTimerRun;

    bool  bHasUnitArray[Bin_MAX_NUM];                   // per-unit installed flag
    bool  bHasUnit;                                     // any unit installed
    bool  bSliding[Bin_MAX_NUM];                        // unit needs label cycling
    bool  bStopProcess;                                 // run gate for the state machine
    bool  bSetBin[Bin_MAX_NUM];                         // unit needs a bin write
    int   iSetBin[Bin_MAX_NUM][TEST_MAX_BIN];           // target bin labels per unit
    bool  bSetColor[Bin_MAX_NUM];                       // unit needs a color write
    bool  bGetStatus[Bin_MAX_NUM];                      // unit needs version read
    int   iSetColor[Bin_MAX_NUM];                       // target color per unit
    bool  bStartSetBin;                                 // begin bin write pass
    bool  bStartSetColor;                               // begin color write pass
    int   iDelaySec;                                    // bin label hold time
    int   iVersion[Bin_MAX_NUM];                        // unit firmware variant
    int   iBinNow[Bin_MAX_NUM];                         // current shown bin
    int   iColorNow[Bin_MAX_NUM];                       // current shown color
    bool  bHasError[Bin_MAX_NUM];                       // unit comm error flag
    int   iRusStatus;                                   // current running status

    char SendBuffer[1024];                              // serial TX scratch
    bool BinDispRecv;                                   // last frame echoed back
    char BinDispCom2Buffer[1024];                       // serial RX scratch
    AnsiString  ComPort;                                // COM port name

    int iStartSetBinTask;
    int iStartSetColorTask;
    int iStartGetStatusTask;
    int iBinDispCtrlTask;
    int iTotalInstalledUnit;
    int iTestBinCount;                                  // label cycle safety cap

    // pure virtuals: implemented per hardware protocol in the subclass
    virtual void WriteBin  (int Addr, int Command, short value)=0;
    virtual void WriteColor(int Addr, short value)=0;
    virtual void ReadVersion(int Addr)=0;
    virtual bool DoStartSetBin()    {return false;};
    virtual bool DoStartSetColor()  {return false;};
    virtual bool DoStartGetStatus() {return false;};

    bool GetCOMPortStatus(AnsiString Com);
    unsigned char T_HEX2ASCII_Mac(unsigned char hex2ascii);
    unsigned char T_ASXII2HEX_Mac(unsigned char ascii2hex);
    unsigned char A_Create_LCR(unsigned char *Sptr, unsigned char length);
    int iErrCount[Bin_MAX_NUM];
    int iCount[Bin_MAX_NUM];
    int iUsedBinNumber;
    AnsiString Chararr2Hexstring(char* cstr,int iNum);
public:
    TMyBinDispCtrl();
    ~TMyBinDispCtrl();
    AnsiString Alias[Bin_MAX_NUM];
    TComm  *CommBin;
    void ProcessStopStart(bool Value)  ;// start or stop the state machine
    void SetComPort(AnsiString port)   ;// set the COM port name
    void SetComParity(TParity Parity)  ;// set the COM parity
    bool UnitHasInstall(int Index)     ;// query a unit installed flag
    void CloseUnit(int Index)          ;// mark a unit not installed
    void OpenUnit(int Index)           ;// mark a unit installed
    void SetDelayTime(int Sec)         ;// set the bin label hold time
    int  GetDelayTime()                ;// get the bin label hold time
    int  GetTotalInstalledUnit()       ;// get the installed unit count
    int  GetColorNow(int Index)        ;// get a unit current color
    int  GetBinNow(int Index)          ;// get a unit current bin
    bool GerErrNow(int Index)          ;// get a unit error flag
    void SerErrNow(int Index,bool bErr);// set a unit error flag
    AnsiString GetRunStatus()          ;// running status text
    AnsiString GetComPort(){return ComPort;}
    void __fastcall CommBinReceiveData(TObject *Sender, Pointer Buffer, WORD BufferLength);
    TParity ComParity;                                  // COM parity
    void InstalledUnit(int Index);                      // declare a unit present
    void WriteTargetBin(int Index, int *bin, int color);// set unit labels and color
    void WriteTargetBin(int ibin);                      // set all units color
    // AI(ht160s-maintainer) 20260615 : set ONE unit to a single fixed label +
    // color (old-160 style). value: -1 blank(X), 0..99 digits, 100..125 A..Z.
    void SetUnitLabel(int Index, int value, int color);
    void SetUnitBin(int Index, int value);    // label only (manual test)
    void SetUnitColor(int Index, int color);  // color only (manual test)
    bool InitialOK;                                     // host setup done
    bool bFirstInit;
    TStringList *slBinDispLog;                          // in-memory log buffer
    AnsiString sReadBuffer;
    void Spin();
    bool StartComport(TComm *Comm,AnsiString port);
    bool StopComport(TComm *Comm,AnsiString port);
    void SetUsedBinNumber(int iNum);
    int GetUsedBinNumber(){return iUsedBinNumber;}
    void ResetBinFlow(){iBinDispCtrlTask=1;}
    void AddBinDisplayLog(AnsiString asAction, AnsiString asMessage);
    void FlushBinDisplayLog();
};
//----------------------------------------------------------------------------
// Concrete LED protocol (HT-9046 style 7-seg / LED bin display boards).
//----------------------------------------------------------------------------
class TMyBinDispHT9046:public TMyBinDispCtrl
{
    private:
    protected:
        virtual void WriteBin(int Addr, int Command, short Value);
        virtual void WriteColor(int Addr, short Value);
        virtual void ReadVersion(int Addr);
        virtual bool DoStartSetBin();
        virtual bool DoStartSetColor();
        virtual bool DoStartGetStatus();
};
//----------------------------------------------------------------------------
#endif
