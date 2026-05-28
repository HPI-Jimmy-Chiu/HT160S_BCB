//---------------------------------------------------------------------------
#ifndef HCommH
#define HCommH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>

enum TComPortBaudRate {
    br110,   br300,   br600,
    br1200,  br2400,  br4800,
    br9600,  br14400, br19200,
    br38400, br56000, br57600, br115200 };

enum TComPortNumber {
    pnCOM1,  pnCOM2,  pnCOM3,
    pnCOM4,  pnCOM5,  pnCOM6,
    pnCOM7,  pnCOM8,  pnCOM9,
    pnCOM10, pnCOM11, pnCOM12,
    pnCOM13, pnCOM14, pnCOM15, pnCOM16 };

enum TComPortDataBits {
    db5BITS, db6BITS,
    db7BITS, db8BITS };

enum TComPortStopBits {
    sb1BITS, sb1HALFBITS,
    sb2BITS };

enum TComPortParity {
    ptNONE, ptODD, ptEVEN,
    ptMARK, ptSPACE };

enum TComPortHwHandshaking {
    hhNONE, hhRTSCTS };

enum TComPortSwHandshaking {
    shNONE, shXONXOFF };

enum TPacketMode {
    pmDiscard, pmPass };

typedef void __fastcall (__closure *HCommReceDataEvent)
    (TObject* Sender,void *DataPtr,int DataSize);

typedef void __fastcall (__closure *HCommRecePackEvent)
    (TObject* Sender,void * Packet,int DataSize,int ElapsedTime);

typedef void __fastcall (__closure *HCommProc) ();

//---------------------------------------------------------------------------
//  讀取執行緒
//---------------------------------------------------------------------------
class PACKAGE HCommThread : public TThread
{
private:
protected:
    void __fastcall Execute();

public:
    HWND        hMainWnd;
    HCommProc   RunFunc;

    __fastcall HCommThread();
};

//---------------------------------------------------------------------------
//
//---------------------------------------------------------------------------
class PACKAGE HComm : public TComponent
{
private:
    DWORD   PackLose;

    void    __fastcall WriteToPort(void *Data,int iLen);
    HWND    hMainWnd;
    int     iReceDataLength;
    void       __fastcall WndCommProc(TMessage &msg);
    AnsiString __fastcall GetPackSChar();
    void       __fastcall SetPackSChar(AnsiString s);

protected:
    TList       *SendData;
    HCommThread *ThRead;                // 讀取執行緒
    HCommThread *ThWrite;               // 寫出執行緒

	HANDLE FComPortHandle;
	TComPortNumber FComPort;
	TComPortBaudRate FComPortBaudRate;
	TComPortDataBits FComPortDataBits;
	TComPortStopBits FComPortStopBits;
	TComPortParity FComPortParity;
	TComPortHwHandshaking FComPortHwHandshaking;
	TComPortSwHandshaking FComPortSwHandshaking;
	int FComPortInBufSize;
	int FComPortOutBufSize;
	short FPacketSize;
	int FPacketTimeout;
	TPacketMode FPacketMode;
	HCommReceDataEvent FComPortReceiveData;
	HCommRecePackEvent FComPortReceivePacket;
	bool FEnableDTROnOpen;
	int FOutputTimeout;
	void   *FTempInBuffer;
	int     FFirstByteOfPacketTime;
    BYTE    FPacketStartChar;

	void __fastcall SetComHandle(HANDLE Value);
	void __fastcall SetComPort(TComPortNumber Value);
	void __fastcall SetComPortBaudRate(TComPortBaudRate Value);
	void __fastcall SetComPortDataBits(TComPortDataBits Value);
	void __fastcall SetComPortStopBits(TComPortStopBits Value);
	void __fastcall SetComPortParity(TComPortParity Value);
	void __fastcall SetComPortHwHandshaking(TComPortHwHandshaking Value);
	void __fastcall SetComPortSwHandshaking(TComPortSwHandshaking Value);
	void __fastcall SetComPortInBufSize(int Value);
	void __fastcall SetComPortOutBufSize(int Value);
	void __fastcall SetPacketSize(short Value);
	void __fastcall SetPacketTimeout(int Value);
	void __fastcall ApplyCOMSettings(void);
	void __fastcall ReceProc();
    void __fastcall WriteProc();

public:
    __fastcall HComm(TComponent* Owner);
    __fastcall ~HComm();
	bool __fastcall Connect(void);
	void __fastcall Disconnect(void);
	bool __fastcall Connected(void);
	void __fastcall FlushBuffers(bool inBuf, bool outBuf);
	long __fastcall OutFreeSpace(void);
	void __fastcall Send(void *DataPtr, int DataSize);
	void __fastcall Send(AnsiString s);
	void __fastcall Send(char *s);
	void __fastcall ToggleDTR(bool onOff);
	void __fastcall ToggleRTS(bool onOff);
	__property HANDLE ComHandle = {read=FComPortHandle, write=SetComHandle, nodefault};
    int     InQueueCount();
    int     OutQueueCount();

__published:
    // ------------------------------------
    //  Properties ......
    // ------------------------------------
	__property TComPortNumber ComPort =
        {read=FComPort, write=SetComPort, default=1};
	__property TComPortBaudRate ComPortSpeed =
        {read=FComPortBaudRate, write=SetComPortBaudRate, default=6 };
	__property TComPortDataBits ComPortDataBits =
        {read=FComPortDataBits, write=SetComPortDataBits, default=3 };
	__property TComPortStopBits ComPortStopBits =
        {read=FComPortStopBits, write=SetComPortStopBits, default=0	};
	__property TComPortParity ComPortParity =
        {read=FComPortParity, write=SetComPortParity, default=0};
	__property TComPortHwHandshaking ComPortHwHandshaking =
        {read=FComPortHwHandshaking, write=SetComPortHwHandshaking,default=0};
	__property TComPortSwHandshaking ComPortSwHandshaking =
        {read=FComPortSwHandshaking, write=SetComPortSwHandshaking,default=0};
	__property int ComPortInBufSize =
        {read=FComPortInBufSize, write=SetComPortInBufSize, default=2048};
	__property int ComPortOutBufSize =
        {read=FComPortOutBufSize, write=SetComPortOutBufSize, default=2048};
	__property short PacketSize =
        {read=FPacketSize, write=SetPacketSize, default=-1};
	__property int PacketTimeout =
        {read=FPacketTimeout, write=SetPacketTimeout, default=-1};
	__property TPacketMode PacketMode =
        {read=FPacketMode, write=FPacketMode, default=0};
	__property bool EnableDTROnOpen =
        {read=FEnableDTROnOpen, write=FEnableDTROnOpen, default=1};
	__property int OutputTimeout =
        {read=FOutputTimeout, write=FOutputTimeout, default=4000};
    __property AnsiString PacketStartChar =
        {read=GetPackSChar,write=SetPackSChar };

    // ------------------------------------
    //  Events ......
    // ------------------------------------
	__property HCommReceDataEvent OnReceiveData =
        {read=FComPortReceiveData, write=FComPortReceiveData};
	__property HCommRecePackEvent OnReceivePacket =
        {read=FComPortReceivePacket, write=FComPortReceivePacket};
};

extern PACKAGE int __fastcall BaudRateOf(TComPortBaudRate bRate);
extern PACKAGE int __fastcall DelayForRX(TComPortBaudRate bRate, int DataSize);
//---------------------------------------------------------------------------
#endif
