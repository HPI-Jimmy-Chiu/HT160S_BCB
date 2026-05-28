// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'SPComm.pas' rev: 6.00

#ifndef SPCommHPP
#define SPCommHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Dialogs.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Spcomm
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TParity { None, Odd, Even, Mark, Space };
#pragma option pop

#pragma option push -b-
enum TStopBits { _1, _1_5, _2 };
#pragma option pop

#pragma option push -b-
enum TByteSize { _5, _6, _7, _8 };
#pragma option pop

#pragma option push -b-
enum TDtrControl { DtrEnable, DtrDisable, DtrHandshake };
#pragma option pop

#pragma option push -b-
enum TRtsControl { RtsEnable, RtsDisable, RtsHandshake, RtsTransmissionAvailable };
#pragma option pop

class DELPHICLASS ECommsError;
class PASCALIMPLEMENTATION ECommsError : public Sysutils::Exception 
{
	typedef Sysutils::Exception inherited;
	
public:
	#pragma option push -w-inl
	/* Exception.Create */ inline __fastcall ECommsError(const AnsiString Msg) : Sysutils::Exception(Msg) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateFmt */ inline __fastcall ECommsError(const AnsiString Msg, const System::TVarRec * Args, const int Args_Size) : Sysutils::Exception(Msg, Args, Args_Size) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateRes */ inline __fastcall ECommsError(int Ident)/* overload */ : Sysutils::Exception(Ident) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResFmt */ inline __fastcall ECommsError(int Ident, const System::TVarRec * Args, const int Args_Size)/* overload */ : Sysutils::Exception(Ident, Args, Args_Size) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateHelp */ inline __fastcall ECommsError(const AnsiString Msg, int AHelpContext) : Sysutils::Exception(Msg, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateFmtHelp */ inline __fastcall ECommsError(const AnsiString Msg, const System::TVarRec * Args, const int Args_Size, int AHelpContext) : Sysutils::Exception(Msg, Args, Args_Size, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResHelp */ inline __fastcall ECommsError(int Ident, int AHelpContext)/* overload */ : Sysutils::Exception(Ident, AHelpContext) { }
	#pragma option pop
	#pragma option push -w-inl
	/* Exception.CreateResFmtHelp */ inline __fastcall ECommsError(System::PResStringRec ResStringRec, const System::TVarRec * Args, const int Args_Size, int AHelpContext)/* overload */ : Sysutils::Exception(ResStringRec, Args, Args_Size, AHelpContext) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~ECommsError(void) { }
	#pragma option pop
	
};


typedef void __fastcall (__closure *TReceiveDataEvent)(System::TObject* Sender, void * Buffer, Word BufferLength);

typedef void __fastcall (__closure *TReceiveErrorEvent)(System::TObject* Sender, unsigned EventMask);

typedef void __fastcall (__closure *TModemStateChangeEvent)(System::TObject* Sender, unsigned ModemEvent);

typedef void __fastcall (__closure *TSendDataEmptyEvent)(System::TObject* Sender);

class DELPHICLASS TReadThread;
class PASCALIMPLEMENTATION TReadThread : public Classes::TThread 
{
	typedef Classes::TThread inherited;
	
protected:
	virtual void __fastcall Execute(void);
	
public:
	unsigned hCommFile;
	unsigned hCloseEvent;
	unsigned hComm32Window;
	bool __fastcall SetupCommEvent(Windows::POverlapped lpOverlappedCommEvent, unsigned &lpfdwEvtMask);
	bool __fastcall SetupReadEvent(Windows::POverlapped lpOverlappedRead, char * lpszInputBuffer, unsigned dwSizeofBuffer, unsigned &lpnNumberOfBytesRead);
	bool __fastcall HandleCommEvent(Windows::POverlapped lpOverlappedCommEvent, unsigned &lpfdwEvtMask, bool fRetrieveEvent);
	bool __fastcall HandleReadEvent(Windows::POverlapped lpOverlappedRead, char * lpszInputBuffer, unsigned dwSizeofBuffer, unsigned &lpnNumberOfBytesRead);
	bool __fastcall HandleReadData(char * lpszInputBuffer, unsigned dwSizeofBuffer);
	BOOL __fastcall ReceiveData(char * lpNewString, unsigned dwSizeofNewString);
	BOOL __fastcall ReceiveError(unsigned EvtMask);
	BOOL __fastcall ModemStateChange(unsigned ModemEvent);
	void __fastcall PostHangupCall(void);
public:
	#pragma option push -w-inl
	/* TThread.Create */ inline __fastcall TReadThread(bool CreateSuspended) : Classes::TThread(CreateSuspended) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TThread.Destroy */ inline __fastcall virtual ~TReadThread(void) { }
	#pragma option pop
	
};


class DELPHICLASS TWriteThread;
class PASCALIMPLEMENTATION TWriteThread : public Classes::TThread 
{
	typedef Classes::TThread inherited;
	
protected:
	virtual void __fastcall Execute(void);
	bool __fastcall HandleWriteData(Windows::POverlapped lpOverlappedWrite, char * pDataToWrite, unsigned dwNumberOfBytesToWrite);
	
public:
	unsigned hCommFile;
	unsigned hCloseEvent;
	unsigned hComm32Window;
	bool *pFSendDataEmpty;
	void __fastcall PostHangupCall(void);
public:
	#pragma option push -w-inl
	/* TThread.Create */ inline __fastcall TWriteThread(bool CreateSuspended) : Classes::TThread(CreateSuspended) { }
	#pragma option pop
	#pragma option push -w-inl
	/* TThread.Destroy */ inline __fastcall virtual ~TWriteThread(void) { }
	#pragma option pop
	
};


class DELPHICLASS TComm;
class PASCALIMPLEMENTATION TComm : public Classes::TComponent 
{
	typedef Classes::TComponent inherited;
	
private:
	TReadThread* ReadThread;
	TWriteThread* WriteThread;
	unsigned hCommFile;
	unsigned hCloseEvent;
	unsigned FHWnd;
	bool FSendDataEmpty;
	AnsiString FCommName;
	unsigned FBaudRate;
	bool FParityCheck;
	bool FOutx_CtsFlow;
	bool FOutx_DsrFlow;
	TDtrControl FDtrControl;
	bool FDsrSensitivity;
	bool FTxContinueOnXoff;
	bool FOutx_XonXoffFlow;
	bool FInx_XonXoffFlow;
	bool FReplaceWhenParityError;
	bool FIgnoreNullChar;
	TRtsControl FRtsControl;
	Word FXonLimit;
	Word FXoffLimit;
	TByteSize FByteSize;
	TParity FParity;
	TStopBits FStopBits;
	char FXonChar;
	char FXoffChar;
	char FReplacedChar;
	unsigned FReadIntervalTimeout;
	unsigned FReadTotalTimeoutMultiplier;
	unsigned FReadTotalTimeoutConstant;
	unsigned FWriteTotalTimeoutMultiplier;
	unsigned FWriteTotalTimeoutConstant;
	TReceiveDataEvent FOnReceiveData;
	Classes::TNotifyEvent FOnRequestHangup;
	TReceiveErrorEvent FOnReceiveError;
	TModemStateChangeEvent FOnModemStateChange;
	TSendDataEmptyEvent FOnSendDataEmpty;
	void __fastcall SetBaudRate(unsigned Rate);
	void __fastcall SetParityCheck(bool b);
	void __fastcall SetOutx_CtsFlow(bool b);
	void __fastcall SetOutx_DsrFlow(bool b);
	void __fastcall SetDtrControl(TDtrControl c);
	void __fastcall SetDsrSensitivity(bool b);
	void __fastcall SetTxContinueOnXoff(bool b);
	void __fastcall SetOutx_XonXoffFlow(bool b);
	void __fastcall SetInx_XonXoffFlow(bool b);
	void __fastcall SetReplaceWhenParityError(bool b);
	void __fastcall SetIgnoreNullChar(bool b);
	void __fastcall SetRtsControl(TRtsControl c);
	void __fastcall SetXonLimit(Word Limit);
	void __fastcall SetXoffLimit(Word Limit);
	void __fastcall SetByteSize(TByteSize Size);
	void __fastcall SetParity(TParity p);
	void __fastcall SetStopBits(TStopBits Bits);
	void __fastcall SetXonChar(char c);
	void __fastcall SetXoffChar(char c);
	void __fastcall SetReplacedChar(char c);
	void __fastcall SetReadIntervalTimeout(unsigned v);
	void __fastcall SetReadTotalTimeoutMultiplier(unsigned v);
	void __fastcall SetReadTotalTimeoutConstant(unsigned v);
	void __fastcall SetWriteTotalTimeoutMultiplier(unsigned v);
	void __fastcall SetWriteTotalTimeoutConstant(unsigned v);
	void __fastcall CommWndProc(Messages::TMessage &msg);
	void __fastcall _SetCommState(void);
	void __fastcall _SetCommTimeout(void);
	
protected:
	void __fastcall CloseReadThread(void);
	void __fastcall CloseWriteThread(void);
	void __fastcall ReceiveData(char * Buffer, Word BufferLength);
	void __fastcall ReceiveError(unsigned EvtMask);
	void __fastcall ModemStateChange(unsigned ModemEvent);
	void __fastcall RequestHangup(void);
	void __fastcall _SendDataEmpty(void);
	
public:
	__property unsigned Handle = {read=hCommFile, nodefault};
	__property bool SendDataEmpty = {read=FSendDataEmpty, nodefault};
	__fastcall virtual TComm(Classes::TComponent* AOwner);
	__fastcall virtual ~TComm(void);
	void __fastcall StartComm(void);
	void __fastcall StopComm(void);
	bool __fastcall WriteCommData(char * pDataToWrite, Word dwSizeofDataToWrite);
	unsigned __fastcall GetModemState(void);
	
__published:
	__property AnsiString CommName = {read=FCommName, write=FCommName};
	__property unsigned BaudRate = {read=FBaudRate, write=SetBaudRate, nodefault};
	__property bool ParityCheck = {read=FParityCheck, write=SetParityCheck, nodefault};
	__property bool Outx_CtsFlow = {read=FOutx_CtsFlow, write=SetOutx_CtsFlow, nodefault};
	__property bool Outx_DsrFlow = {read=FOutx_DsrFlow, write=SetOutx_DsrFlow, nodefault};
	__property TDtrControl DtrControl = {read=FDtrControl, write=SetDtrControl, nodefault};
	__property bool DsrSensitivity = {read=FDsrSensitivity, write=SetDsrSensitivity, nodefault};
	__property bool TxContinueOnXoff = {read=FTxContinueOnXoff, write=SetTxContinueOnXoff, nodefault};
	__property bool Outx_XonXoffFlow = {read=FOutx_XonXoffFlow, write=SetOutx_XonXoffFlow, nodefault};
	__property bool Inx_XonXoffFlow = {read=FInx_XonXoffFlow, write=SetInx_XonXoffFlow, nodefault};
	__property bool ReplaceWhenParityError = {read=FReplaceWhenParityError, write=SetReplaceWhenParityError, nodefault};
	__property bool IgnoreNullChar = {read=FIgnoreNullChar, write=SetIgnoreNullChar, nodefault};
	__property TRtsControl RtsControl = {read=FRtsControl, write=SetRtsControl, nodefault};
	__property Word XonLimit = {read=FXonLimit, write=SetXonLimit, nodefault};
	__property Word XoffLimit = {read=FXoffLimit, write=SetXoffLimit, nodefault};
	__property TByteSize ByteSize = {read=FByteSize, write=SetByteSize, nodefault};
	__property TParity Parity = {read=FParity, write=FParity, nodefault};
	__property TStopBits StopBits = {read=FStopBits, write=SetStopBits, nodefault};
	__property char XonChar = {read=FXonChar, write=SetXonChar, nodefault};
	__property char XoffChar = {read=FXoffChar, write=SetXoffChar, nodefault};
	__property char ReplacedChar = {read=FReplacedChar, write=SetReplacedChar, nodefault};
	__property unsigned ReadIntervalTimeout = {read=FReadIntervalTimeout, write=SetReadIntervalTimeout, nodefault};
	__property unsigned ReadTotalTimeoutMultiplier = {read=FReadTotalTimeoutMultiplier, write=SetReadTotalTimeoutMultiplier, nodefault};
	__property unsigned ReadTotalTimeoutConstant = {read=FReadTotalTimeoutConstant, write=SetReadTotalTimeoutConstant, nodefault};
	__property unsigned WriteTotalTimeoutMultiplier = {read=FWriteTotalTimeoutMultiplier, write=SetWriteTotalTimeoutMultiplier, nodefault};
	__property unsigned WriteTotalTimeoutConstant = {read=FWriteTotalTimeoutConstant, write=SetWriteTotalTimeoutConstant, nodefault};
	__property TReceiveDataEvent OnReceiveData = {read=FOnReceiveData, write=FOnReceiveData};
	__property TReceiveErrorEvent OnReceiveError = {read=FOnReceiveError, write=FOnReceiveError};
	__property TModemStateChangeEvent OnModemStateChange = {read=FOnModemStateChange, write=FOnModemStateChange};
	__property Classes::TNotifyEvent OnRequestHangup = {read=FOnRequestHangup, write=FOnRequestHangup};
	__property TSendDataEmptyEvent OnSendDataEmpty = {read=FOnSendDataEmpty, write=FOnSendDataEmpty};
};


//-- var, const, procedure ---------------------------------------------------
static const Word PWM_GOTCOMMDATA = 0x401;
static const Word PWM_RECEIVEERROR = 0x402;
static const Word PWM_REQUESTHANGUP = 0x403;
static const Word PWM_MODEMSTATECHANGE = 0x404;
static const Word PWM_SENDDATAEMPTY = 0x405;
static const Shortint ME_CTS = 0x1;
static const Shortint ME_DSR = 0x2;
static const Shortint ME_RING = 0x4;
static const Shortint ME_RLSD = 0x8;
static const Word PWM_COMMWRITE = 0x401;
static const Word INPUTBUFFERSIZE = 0x4000;
extern PACKAGE void __fastcall Register(void);

}	/* namespace Spcomm */
using namespace Spcomm;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// SPComm
