// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'ALed.pas' rev: 6.00

#ifndef ALedHPP
#define ALedHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <ExtCtrls.hpp>	// Pascal unit
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

namespace Aled
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TLEDStyle { LEDSmall, LEDLarge, LEDSqSmall, LEDSqLarge, LEDVertical, LEDHorizontal };
#pragma option pop

class DELPHICLASS TALed;
class PASCALIMPLEMENTATION TALed : public Controls::TGraphicControl 
{
	typedef Controls::TGraphicControl inherited;
	
protected:
	Graphics::TBitmap* fLedBitmap[2];
	Extctrls::TTimer* fLedTimer;
	Graphics::TColor fTrueColor;
	Graphics::TColor fFalseColor;
	bool fBlink;
	TLEDStyle fLEDStyle;
	int fInterval;
	bool fValue;
	bool ColorTemp;
	Classes::TNotifyEvent fOnTimer;
	Classes::TNotifyEvent fOnMouseEnter;
	Classes::TNotifyEvent fOnMouseLeave;
	virtual void __fastcall Paint(void);
	void __fastcall OnLedTimer(System::TObject* Sender);
	HIDESBASE MESSAGE void __fastcall CMMouseEnter(Messages::TMessage &Message);
	HIDESBASE MESSAGE void __fastcall CMMouseLeave(Messages::TMessage &Message);
	
public:
	__fastcall virtual TALed(Classes::TComponent* AOwner);
	__fastcall virtual ~TALed(void);
	void __fastcall CreateLedBitmap(void);
	void __fastcall FreeLedBitmap(void);
	void __fastcall ChangeValue(bool V);
	void __fastcall ChangeBlink(bool V);
	void __fastcall ChangeStyle(TLEDStyle V);
	void __fastcall SetTrueColor(Graphics::TColor V);
	void __fastcall SetFalseColor(Graphics::TColor V);
	void __fastcall SetInterval(int V);
	void __fastcall SetToTrueColor(void);
	void __fastcall SetToFalseColor(void);
	void __fastcall SetLedTimer(void);
	void __fastcall ResetLedTimer(void);
	
__published:
	__property Graphics::TColor TrueColor = {read=fTrueColor, write=SetTrueColor, default=65280};
	__property Graphics::TColor FalseColor = {read=fFalseColor, write=SetFalseColor, default=12632256};
	__property bool Blink = {read=fBlink, write=ChangeBlink, default=0};
	__property bool Value = {read=fValue, write=ChangeValue, default=0};
	__property int Interval = {read=fInterval, write=SetInterval, default=1000};
	__property TLEDStyle LEDStyle = {read=fLEDStyle, write=ChangeStyle, default=0};
	__property Classes::TNotifyEvent OnTimer = {read=fOnTimer, write=fOnTimer};
	__property Classes::TNotifyEvent OnMouseEnter = {read=fOnMouseEnter, write=fOnMouseEnter};
	__property Classes::TNotifyEvent OnMouseLeave = {read=fOnMouseLeave, write=fOnMouseLeave};
	__property OnClick ;
	__property ShowHint ;
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE void __fastcall Register(void);

}	/* namespace Aled */
using namespace Aled;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// ALed
