// Borland C++ Builder
// Copyright (c) 1995, 1999 by Borland International
// All rights reserved

// (DO NOT EDIT: machine generated header) 'GSwitch.pas' rev: 5.00

#ifndef GSwitchHPP
#define GSwitchHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <Graphics.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Gswitch
{
//-- type declarations -------------------------------------------------------
typedef Windows::TPoint RectArray[4];

typedef Windows::TPoint TriArray[3];

class DELPHICLASS TSwitch;
class PASCALIMPLEMENTATION TSwitch : public Controls::TCustomControl 
{
	typedef Controls::TCustomControl inherited;
	
private:
	Windows::TPoint TopShape[3];
	Windows::TPoint OnShape[4];
	Windows::TPoint OffShape[4];
	Windows::TPoint SideShape[4];
	Classes::TNotifyEvent FOnChanged;
	Classes::TNotifyEvent FOnChecked;
	Classes::TNotifyEvent FOnUnChecked;
	AnsiString FCaptionOn;
	AnsiString FCaptionOff;
	bool FChecked;
	bool FCheckedLeft;
	Byte FSlope;
	Byte FSideLength;
	Graphics::TColor FOnColor;
	Graphics::TColor FOffColor;
	Graphics::TColor FTopColor;
	Graphics::TColor FSideColor;
	int ALeft;
	int ATop;
	int AHeight;
	int AWidth;
	int LabelLen;
	int LabelOfs;
	int Side;
	HIDESBASE MESSAGE void __fastcall WMSetFocus(Messages::TWMSetFocus &Message);
	HIDESBASE MESSAGE void __fastcall WMKillFocus(Messages::TWMKillFocus &Message);
	void __fastcall CallNotifyEvent(void);
	void __fastcall Setup(void);
	void __fastcall Draw(void);
	void __fastcall SetCaptionOn(AnsiString Value);
	void __fastcall SetCaptionOff(AnsiString Value);
	void __fastcall SetChecked(bool Value);
	void __fastcall SetCheckedLeft(bool Value);
	void __fastcall SetSlope(Byte Value);
	void __fastcall SetSideLength(Byte Value);
	void __fastcall SetOnColor(Graphics::TColor Value);
	void __fastcall SetOffColor(Graphics::TColor Value);
	void __fastcall SetTopColor(Graphics::TColor Value);
	void __fastcall SetSideColor(Graphics::TColor Value);
	
public:
	__fastcall virtual TSwitch(Classes::TComponent* AOwner);
	virtual void __fastcall Paint(void);
	DYNAMIC void __fastcall MouseDown(Controls::TMouseButton Button, Classes::TShiftState Shift, int X, 
		int Y);
	DYNAMIC void __fastcall KeyDown(Word &Key, Classes::TShiftState Shift);
	
__published:
	__property AnsiString CaptionOn = {read=FCaptionOn, write=SetCaptionOn};
	__property AnsiString CaptionOff = {read=FCaptionOff, write=SetCaptionOff};
	__property bool Checked = {read=FChecked, write=SetChecked, default=0};
	__property bool CheckedLeft = {read=FCheckedLeft, write=SetCheckedLeft, default=1};
	__property Byte Slope = {read=FSlope, write=SetSlope, default=6};
	__property Byte SideLength = {read=FSideLength, write=SetSideLength, default=6};
	__property Graphics::TColor OnColor = {read=FOnColor, write=SetOnColor, default=255};
	__property Graphics::TColor OffColor = {read=FOffColor, write=SetOffColor, default=128};
	__property Graphics::TColor TopColor = {read=FTopColor, write=SetTopColor, default=12632256};
	__property Graphics::TColor SideColor = {read=FSideColor, write=SetSideColor, default=12632256};
	__property Font ;
	__property TabStop ;
	__property TabOrder ;
	__property ShowHint ;
	__property OnClick ;
	__property OnMouseDown ;
	__property Classes::TNotifyEvent OnChanged = {read=FOnChanged, write=FOnChanged};
	__property Classes::TNotifyEvent OnChecked = {read=FOnChecked, write=FOnChecked};
	__property Classes::TNotifyEvent OnUnChecked = {read=FOnUnChecked, write=FOnUnChecked};
public:
	#pragma option push -w-inl
	/* TCustomControl.Destroy */ inline __fastcall virtual ~TSwitch(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TSwitch(HWND ParentWindow) : Controls::TCustomControl(
		ParentWindow) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE void __fastcall Register(void);

}	/* namespace Gswitch */
#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace Gswitch;
#endif
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// GSwitch
