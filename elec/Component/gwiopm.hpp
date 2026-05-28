// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'gwiopm.pas' rev: 6.00

#ifndef gwiopmHPP
#define gwiopmHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Dialogs.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <WinSvc.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Gwiopm
{
//-- type declarations -------------------------------------------------------
typedef Byte TIOPM[8193];

typedef Byte *PIOPM;

class DELPHICLASS TGWIOPM_Driver;
class PASCALIMPLEMENTATION TGWIOPM_Driver : public System::TObject 
{
	typedef System::TObject inherited;
	
private:
	AnsiString HomeDir;
	AnsiString DriverDir;
	AnsiString DriverName;
	AnsiString DriverPath;
	unsigned hSCMan;
	unsigned hDevice;
	unsigned __fastcall IOCTL_IOPMD_Misc1(unsigned &RetVal, int Cmd);
	unsigned __fastcall IOCTL_IOPMD_GET_SET_LIOPM(Word Addr, Byte &B, int cmd);
	
public:
	int LastOutBuf;
	__fastcall TGWIOPM_Driver(void);
	unsigned __fastcall OpenSCM(void);
	unsigned __fastcall CloseSCM(void);
	unsigned __fastcall Install(AnsiString newdriverpath);
	unsigned __fastcall Start(void);
	unsigned __fastcall Stop(void);
	unsigned __fastcall Remove(void);
	unsigned __fastcall DeviceOpen(void);
	unsigned __fastcall DeviceClose(void);
	unsigned __fastcall IOCTL_IOPMD_READ_TEST(unsigned &RetVal);
	unsigned __fastcall IOCTL_IOPMD_READ_VERSION(unsigned &RetVal);
	unsigned __fastcall IOCTL_IOPMD_CLEAR_LIOPM(void);
	unsigned __fastcall IOCTL_IOPMD_SET_LIOPM(Word Addr, Byte B);
	unsigned __fastcall IOCTL_IOPMD_GET_LIOPMB(Word Addr, Byte &B);
	unsigned __fastcall IOCTL_IOPMD_GET_LIOPMA(Byte * A);
	unsigned __fastcall LIOPM_Set_Ports(Word BeginPort, Word EndPort, bool Enable);
	unsigned __fastcall IOCTL_IOPMD_ACTIVATE_KIOPM(void);
	unsigned __fastcall IOCTL_IOPMD_DEACTIVATE_KIOPM(void);
	unsigned __fastcall IOCTL_IOPMD_QUERY_KIOPM(void);
	AnsiString __fastcall ErrorLookup(unsigned ErrorNum);
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TGWIOPM_Driver(void) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
static const Word IOPM_SIZE = 0x2000;
#define DEVICE_NAME_STRING "gwiopm"
static const Word IOPMD_TYPE = 0xf100;
static const Word IOCMD_IOPMD_READ_TEST = 0x900;
static const Word IOCMD_IOPMD_READ_VERSION = 0x901;
static const Word IOCMD_IOPMD_CLEAR_LIOPM = 0x910;
static const Word IOCMD_IOPMD_SET_LIOPM = 0x911;
static const Word IOCMD_IOPMD_GET_LIOPMB = 0x912;
static const Word IOCMD_IOPMD_GET_LIOPMA = 0x913;
static const Word IOCMD_IOPMD_ACTIVATE_KIOPM = 0x920;
static const Word IOCMD_IOPMD_DEACTIVATE_KIOPM = 0x921;
static const Word IOCMD_IOPMD_QUERY_KIOPM = 0x922;
extern PACKAGE TGWIOPM_Driver* GWIOPM_Driver;

}	/* namespace Gwiopm */
using namespace Gwiopm;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// gwiopm
