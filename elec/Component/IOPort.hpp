// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'ioport.pas' rev: 6.00

#ifndef ioportHPP
#define ioportHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Ioport
{
//-- type declarations -------------------------------------------------------
//-- var, const, procedure ---------------------------------------------------
extern PACKAGE Byte __fastcall inportb(int port);
extern PACKAGE void __fastcall outportb(int port, Byte value);
extern PACKAGE Word __fastcall inportw(Word PortNum);
extern PACKAGE void __fastcall outportw(Word PortNum, Word Value);
extern PACKAGE int __fastcall inportl(Word PortNum);
extern PACKAGE void __fastcall outportl(Word PortNum, int Value);
extern PACKAGE void __fastcall Register(void);

}	/* namespace Ioport */
using namespace Ioport;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// ioport
