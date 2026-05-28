// Borland C++ Builder
// Copyright (c) 1995, 1999 by Borland International
// All rights reserved

// (DO NOT EDIT: machine generated header) 'ASFunc.pas' rev: 5.00

#ifndef ASFuncHPP
#define ASFuncHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Windows.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Asfunc
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TStylePrint { spInside, spExternal };
#pragma option pop

//-- var, const, procedure ---------------------------------------------------
extern PACKAGE int Offset;
extern PACKAGE int Offset2;
extern PACKAGE int Offset4;
extern PACKAGE int TickCount;
extern PACKAGE int i;
extern PACKAGE int j;
extern PACKAGE int StartA;
extern PACKAGE int EndA;
extern PACKAGE int midX;
extern PACKAGE int midY;
extern PACKAGE int stX;
extern PACKAGE int stY;
extern PACKAGE int endX;
extern PACKAGE int endY;
extern PACKAGE double sX;
extern PACKAGE double sY;
extern PACKAGE double eX;
extern PACKAGE double eY;
extern PACKAGE int iGrad;
extern PACKAGE double rGrad;
extern PACKAGE double r;
extern PACKAGE double p;
extern PACKAGE double p1;
extern PACKAGE double p2;
extern PACKAGE double p4;
extern PACKAGE double t;
extern PACKAGE double t1;
extern PACKAGE int x;
extern PACKAGE int t1_;
extern PACKAGE bool Flag;
extern PACKAGE bool Priv;
extern PACKAGE bool Blick;
extern PACKAGE double R1;
extern PACKAGE double BigRad;
extern PACKAGE double SmallRad;
extern PACKAGE int sAngle;
extern PACKAGE int eAngle;
extern PACKAGE int X_;
extern PACKAGE int Y_;
extern PACKAGE int X1;
extern PACKAGE int Y1;
extern PACKAGE double sinMidAngle;
extern PACKAGE double midAngle1;
extern PACKAGE int midAngle;
extern PACKAGE int CoordPointX;
extern PACKAGE int CoordPointY;
extern PACKAGE double PointX;
extern PACKAGE double PointY;
extern PACKAGE double PointX1;
extern PACKAGE double PointY1;
extern PACKAGE double RadTick;
extern PACKAGE int TmpAn;
extern PACKAGE int TmpAn1;
extern PACKAGE int wText;
extern PACKAGE int hText;
extern PACKAGE void __fastcall ValuesTicks(int Angle, int Radius);
extern PACKAGE void __fastcall SearchGradForPoint(void);
extern PACKAGE void __fastcall GradToCoords(int AngleS, int AngleE, Windows::TPoint &stAngle, Windows::TPoint 
	&enAngle);
extern PACKAGE void __fastcall GradValues(int TickCount1);
extern PACKAGE void __fastcall TextValuesForTicks(TStylePrint Style);

}	/* namespace Asfunc */
#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace Asfunc;
#endif
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// ASFunc
