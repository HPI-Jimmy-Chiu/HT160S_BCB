// Borland C++ Builder
// Copyright (c) 1995, 1998 by Borland International
// All rights reserved

// (DO NOT EDIT: machine generated header) 'gwportio.pas' rev: 3.00

#ifndef gwportioHPP
#define gwportioHPP
#include <WinSvc.hpp>
#include <Windows.hpp>
#include <SysInit.hpp>
#include <System.hpp>

//-- user supplied -----------------------------------------------------------

namespace Gwportio
{
//-- type declarations -------------------------------------------------------
//-- var, const, procedure ---------------------------------------------------
extern PACKAGE Byte __fastcall PortIn(Word PortNum);
extern PACKAGE void __fastcall PortOut(Word PortNum, Byte a);
extern PACKAGE Word __fastcall PortInW(Word PortNum);
extern PACKAGE void __fastcall PortOutW(Word PortNum, Word a);
extern PACKAGE int __fastcall PortInL(Word PortNum);
extern PACKAGE void __fastcall PortOutL(Word PortNum, int a);

}	/* namespace Gwportio */
#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace Gwportio;
#endif
//-- end unit ----------------------------------------------------------------
#endif	// gwportio
