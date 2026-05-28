// Borland C++ Builder
// Copyright (c) 1995, 1999 by Borland International
// All rights reserved

// (DO NOT EDIT: machine generated header) 'RackCtls.pas' rev: 4.00

#ifndef RackCtlsHPP
#define RackCtlsHPP

#pragma delphiheader begin
#pragma option push -w-
#include <Forms.hpp>	// Pascal unit
#include <Menus.hpp>	// Pascal unit
#include <DsgnIntf.hpp>	// Pascal unit
#include <Dialogs.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <ExtCtrls.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Rackctls
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TBorderStyle { bsNone, bsSingle };
#pragma option pop

#pragma option push -b-
enum TButtonDirection { bdBottomUp, bdLeftUp, bdNone, bdRightUp, bdTopUp };
#pragma option pop

#pragma option push -b-
enum TDecSeperator { dsPoint, dsComma, dsDoublePoint, dsMinus };
#pragma option pop

#pragma option push -b-
enum TLEDColor { lcAqua, lcBlue, lcFuchsia, lcGray, lcLime, lcRed, lcWhite, lcYellow };
#pragma option pop

#pragma option push -b-
enum TMeterDirection { mdDown, mdLeft, mdRight, mdUp };
#pragma option pop

typedef Shortint TNumGlyphs;

typedef Shortint TScrewSize;

#pragma option push -b-
enum TSegmentStyle { ssRectangular, ssBeveled };
#pragma option pop

#pragma option push -b-
enum TTextPosition { tpAbove, tpBelow, tpNone, tpOnButton };
#pragma option pop

class DELPHICLASS TAboutProperty;
#pragma pack(push, 4)
class PASCALIMPLEMENTATION TAboutProperty : public Dsgnintf::TPropertyEditor 
{
	typedef Dsgnintf::TPropertyEditor inherited;
	
public:
	virtual void __fastcall Edit(void);
	virtual Dsgnintf::TPropertyAttributes __fastcall GetAttributes(void);
	virtual AnsiString __fastcall GetValue();
protected:
	#pragma option push -w-inl
	/* TPropertyEditor.Create */ inline __fastcall TAboutProperty(const Dsgnintf::_di_IFormDesigner ADesigner
		, int APropCount) : Dsgnintf::TPropertyEditor(ADesigner, APropCount) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TPropertyEditor.Destroy */ inline __fastcall virtual ~TAboutProperty(void) { }
	#pragma option pop
	
};

#pragma pack(pop)

class DELPHICLASS TLEDButton;
#pragma pack(push, 4)
class PASCALIMPLEMENTATION TLEDButton : public Controls::TGraphicControl 
{
	typedef Controls::TGraphicControl inherited;
	
private:
	TAboutProperty* FAbout;
	bool FBeveled;
	TBorderStyle FBorderStyle;
	TButtonDirection FButtonDirection;
	Graphics::TColor FColor;
	Graphics::TColor FColorHighlight;
	TLEDColor FColorLED;
	Graphics::TColor FColorShadow;
	int FDepth;
	bool FDown;
	Graphics::TFont* FFont;
	Graphics::TBitmap* FGlyph;
	TNumGlyphs FNumGlyphs;
	bool FShowLED;
	bool FStateOn;
	bool FSwitching;
	TTextPosition FTextPosition;
	bool FMouseDown;
	Classes::TNotifyEvent FOnClick;
	
protected:
	virtual void __fastcall Paint(void);
	DYNAMIC void __fastcall MouseDown(Controls::TMouseButton Button, Classes::TShiftState Shift, int X, 
		int Y);
	DYNAMIC void __fastcall MouseUp(Controls::TMouseButton Button, Classes::TShiftState Shift, int X, int 
		Y);
	void __fastcall SetBeveled(bool newValue);
	void __fastcall SetBorderStyle(TBorderStyle newBorderStyle);
	void __fastcall SetButtonDirection(TButtonDirection NewDirection);
	HIDESBASE void __fastcall SetColor(Graphics::TColor newColor);
	void __fastcall SetColorLED(TLEDColor newColor);
	void __fastcall SetDepth(int newValue);
	HIDESBASE void __fastcall SetFont(Graphics::TFont* newFont);
	void __fastcall SetGlyph(Graphics::TBitmap* newGlyph);
	void __fastcall SetNumGlyphs(TNumGlyphs newNumGlyphs);
	void __fastcall SetShowLED(bool newValue);
	void __fastcall SetStateOn(bool newValue);
	void __fastcall SetTextPosition(TTextPosition newValue);
	void __fastcall DrawBorder(const Windows::TRect &Dest);
	void __fastcall DrawCaption(const Windows::TRect &Dest);
	void __fastcall DrawGlyph(const Windows::TRect &Dest);
	void __fastcall DrawLED(Windows::TRect &Dest);
	
public:
	__fastcall virtual TLEDButton(Classes::TComponent* AOwner);
	__fastcall virtual ~TLEDButton(void);
	MESSAGE void __fastcall CMTextChanged(Messages::TMessage &msg);
	MESSAGE void __fastcall CMDialogChar(Messages::TWMKey &Message);
	
__published:
	__property TAboutProperty* About = {read=FAbout, write=FAbout};
	__property bool Beveled = {read=FBeveled, write=SetBeveled, nodefault};
	__property TBorderStyle BorderStyle = {read=FBorderStyle, write=SetBorderStyle, nodefault};
	__property TButtonDirection ButtonDirection = {read=FButtonDirection, write=SetButtonDirection, nodefault
		};
	__property Caption ;
	__property Graphics::TColor Color = {read=FColor, write=SetColor, nodefault};
	__property TLEDColor ColorLED = {read=FColorLED, write=SetColorLED, nodefault};
	__property int Depth = {read=FDepth, write=SetDepth, nodefault};
	__property Enabled ;
	__property Graphics::TFont* Font = {read=FFont, write=SetFont};
	__property Graphics::TBitmap* Glyph = {read=FGlyph, write=SetGlyph};
	__property TNumGlyphs NumGlyphs = {read=FNumGlyphs, write=SetNumGlyphs, default=1};
	__property ParentFont ;
	__property ParentShowHint ;
	__property ShowHint ;
	__property bool ShowLED = {read=FShowLED, write=SetShowLED, nodefault};
	__property bool StateOn = {read=FStateOn, write=SetStateOn, nodefault};
	__property bool Switching = {read=FSwitching, write=FSwitching, nodefault};
	__property TTextPosition TextPosition = {read=FTextPosition, write=SetTextPosition, nodefault};
	__property Visible ;
	__property OnClick ;
	__property OnMouseDown ;
	__property OnMouseMove ;
	__property OnMouseUp ;
};

#pragma pack(pop)

class DELPHICLASS TButtonPanel;
#pragma pack(push, 4)
class PASCALIMPLEMENTATION TButtonPanel : public Extctrls::TCustomPanel 
{
	typedef Extctrls::TCustomPanel inherited;
	
private:
	TAboutProperty* FAbout;
	bool FBeveled;
	TBorderStyle FBorderStyle;
	Graphics::TColor FColor;
	Graphics::TColor FColorHighlight;
	Graphics::TColor FColorShadow;
	int FDepth;
	TButtonDirection FPanelDirection;
	bool FShowLED;
	
protected:
	virtual void __fastcall Paint(void);
	void __fastcall SetBeveled(bool newValue);
	HIDESBASE void __fastcall SetBorderStyle(TBorderStyle newBorderStyle);
	HIDESBASE void __fastcall SetColor(Graphics::TColor newColor);
	void __fastcall SetDepth(int newValue);
	void __fastcall SetPanelDirection(TButtonDirection NewDirection);
	void __fastcall SetShowLED(bool newValue);
	void __fastcall DrawBorder(const Windows::TRect &Dest);
	void __fastcall DrawCaption(const Windows::TRect &Dest);
	void __fastcall DrawLED(Windows::TRect &Dest);
	
public:
	__fastcall virtual TButtonPanel(Classes::TComponent* AOwner);
	__fastcall virtual ~TButtonPanel(void);
	
__published:
	__property TAboutProperty* About = {read=FAbout, write=FAbout};
	__property Align ;
	__property Alignment ;
	__property bool Beveled = {read=FBeveled, write=SetBeveled, nodefault};
	__property TBorderStyle BorderStyle = {read=FBorderStyle, write=SetBorderStyle, nodefault};
	__property Caption ;
	__property Graphics::TColor Color = {read=FColor, write=SetColor, nodefault};
	__property Ctl3D ;
	__property int Depth = {read=FDepth, write=SetDepth, nodefault};
	__property DragCursor ;
	__property DragMode ;
	__property Enabled ;
	__property FullRepaint ;
	__property Font ;
	__property Locked ;
	__property TButtonDirection PanelDirection = {read=FPanelDirection, write=SetPanelDirection, nodefault
		};
	__property ParentColor ;
	__property ParentCtl3D ;
	__property ParentFont ;
	__property ParentShowHint ;
	__property PopupMenu ;
	__property ShowHint ;
	__property bool ShowLED = {read=FShowLED, write=SetShowLED, nodefault};
	__property TabOrder ;
	__property TabStop ;
	__property Visible ;
	__property OnClick ;
	__property OnDblClick ;
	__property OnDragDrop ;
	__property OnDragOver ;
	__property OnEndDrag ;
	__property OnEnter ;
	__property OnExit ;
	__property OnMouseDown ;
	__property OnMouseMove ;
	__property OnMouseUp ;
	__property OnResize ;
	__property OnStartDrag ;
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TButtonPanel(HWND ParentWindow) : Extctrls::TCustomPanel(
		ParentWindow) { }
	#pragma option pop
	
};

#pragma pack(pop)

class DELPHICLASS TScrewPanel;
#pragma pack(push, 4)
class PASCALIMPLEMENTATION TScrewPanel : public Extctrls::TCustomPanel 
{
	typedef Extctrls::TCustomPanel inherited;
	
private:
	TAboutProperty* FAbout;
	Graphics::TColor FColor;
	Graphics::TColor FColorHighlight;
	Graphics::TColor FColorShadow;
	int FMargin;
	TScrewSize FScrewSize;
	bool FShowScrews;
	
protected:
	virtual void __fastcall Paint(void);
	HIDESBASE void __fastcall SetColor(Graphics::TColor newColor);
	void __fastcall SetMargin(int newValue);
	void __fastcall SetScrewSize(TScrewSize newValue);
	void __fastcall SetShowScrews(bool newValue);
	void __fastcall DrawScrew(int X, int Y);
	void __fastcall DrawBevel(const Windows::TRect &ARect, bool Raised);
	
public:
	__fastcall virtual TScrewPanel(Classes::TComponent* AOwner);
	__fastcall virtual ~TScrewPanel(void);
	
__published:
	__property TAboutProperty* About = {read=FAbout, write=FAbout};
	__property Align ;
	__property Alignment ;
	__property BevelInner ;
	__property BevelOuter ;
	__property BevelWidth ;
	__property BorderWidth ;
	__property BorderStyle ;
	__property Caption ;
	__property Graphics::TColor Color = {read=FColor, write=SetColor, nodefault};
	__property Ctl3D ;
	__property DragCursor ;
	__property DragMode ;
	__property Enabled ;
	__property FullRepaint ;
	__property Font ;
	__property Locked ;
	__property int Margin = {read=FMargin, write=SetMargin, nodefault};
	__property ParentColor ;
	__property ParentCtl3D ;
	__property ParentFont ;
	__property ParentShowHint ;
	__property PopupMenu ;
	__property TScrewSize ScrewSize = {read=FScrewSize, write=SetScrewSize, nodefault};
	__property ShowHint ;
	__property bool ShowScrews = {read=FShowScrews, write=SetShowScrews, nodefault};
	__property TabOrder ;
	__property TabStop ;
	__property Visible ;
	__property OnClick ;
	__property OnDblClick ;
	__property OnDragDrop ;
	__property OnDragOver ;
	__property OnEndDrag ;
	__property OnEnter ;
	__property OnExit ;
	__property OnMouseDown ;
	__property OnMouseMove ;
	__property OnMouseUp ;
	__property OnResize ;
	__property OnStartDrag ;
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TScrewPanel(HWND ParentWindow) : Extctrls::TCustomPanel(
		ParentWindow) { }
	#pragma option pop
	
};

#pragma pack(pop)

class DELPHICLASS TLEDDisplay;
#pragma pack(push, 4)
class PASCALIMPLEMENTATION TLEDDisplay : public Controls::TGraphicControl 
{
	typedef Controls::TGraphicControl inherited;
	
private:
	TAboutProperty* FAbout;
	Controls::TBevelCut FBevelStyle;
	TBorderStyle FBorderStyle;
	Graphics::TColor FColorBackGround;
	Graphics::TColor FColorSegmentOff;
	Graphics::TColor FColorLED;
	TDecSeperator FDecSeperator;
	Graphics::TBitmap* FDigit[10];
	int FDigitHeight;
	int FDigitWidth;
	int FFractionDigits;
	int FLineWidth;
	int FNumDigits;
	bool FLeadingZeros;
	Graphics::TColor FSegCl[10][7];
	TSegmentStyle FSegmentStyle;
	Extended FValue;
	Classes::TNotifyEvent FOnChange;
	void __fastcall setBevelStyle(Controls::TBevelCut newValue);
	void __fastcall setBorderStyle(TBorderStyle newValue);
	void __fastcall setColorBackGround(Graphics::TColor newValue);
	void __fastcall setColorLED(Graphics::TColor newValue);
	void __fastcall setDecSeperator(TDecSeperator newValue);
	void __fastcall setDigitHeight(int newValue);
	void __fastcall setDigitWidth(int newValue);
	void __fastcall setFractionDigits(int newValue);
	void __fastcall setLeadingZeros(bool newValue);
	void __fastcall setLineWidth(int newValue);
	void __fastcall setNumDigits(int newValue);
	void __fastcall setSegmentStyle(TSegmentStyle newValue);
	void __fastcall setValue(Extended newValue);
	void __fastcall GenerateBitMaps(void);
	void __fastcall AssignColors(int seg, bool s1, bool s2, bool s3, bool s4, bool s5, bool s6, bool s7
		);
	
protected:
	virtual void __fastcall paint(void);
	DYNAMIC void __fastcall Change(void);
	
public:
	__fastcall virtual TLEDDisplay(Classes::TComponent* AOwner);
	__fastcall virtual ~TLEDDisplay(void);
	
__published:
	__property TAboutProperty* About = {read=FAbout, write=FAbout};
	__property Controls::TBevelCut BevelStyle = {read=FBevelStyle, write=setBevelStyle, nodefault};
	__property TBorderStyle BorderStyle = {read=FBorderStyle, write=setBorderStyle, nodefault};
	__property Graphics::TColor ColorBackGround = {read=FColorBackGround, write=setColorBackGround, default=32896
		};
	__property Graphics::TColor ColorLED = {read=FColorLED, write=setColorLED, default=65280};
	__property TDecSeperator DecSeperator = {read=FDecSeperator, write=setDecSeperator, nodefault};
	__property int DigitHeight = {read=FDigitHeight, write=setDigitHeight, default=30};
	__property int DigitWidth = {read=FDigitWidth, write=setDigitWidth, default=20};
	__property int DigitLineWidth = {read=FLineWidth, write=setLineWidth, default=3};
	__property int FractionDigits = {read=FFractionDigits, write=setFractionDigits, default=0};
	__property Height ;
	__property bool LeadingZeros = {read=FLeadingZeros, write=setLeadingZeros, default=1};
	__property int NumDigits = {read=FNumDigits, write=setNumDigits, default=6};
	__property TSegmentStyle SegmentStyle = {read=FSegmentStyle, write=setSegmentStyle, nodefault};
	__property Extended Value = {read=FValue, write=setValue};
	__property Visible ;
	__property Width ;
	__property Classes::TNotifyEvent OnChange = {read=FOnChange, write=FOnChange};
	__property OnMouseDown ;
	__property OnMouseMove ;
	__property OnMouseUp ;
};

#pragma pack(pop)

class DELPHICLASS TLEDMeter;
#pragma pack(push, 4)
class PASCALIMPLEMENTATION TLEDMeter : public Controls::TGraphicControl 
{
	typedef Controls::TGraphicControl inherited;
	
private:
	TAboutProperty* FAbout;
	Controls::TBevelCut FBevelStyle;
	Graphics::TColor FColorLED1;
	Graphics::TColor FColorLED2;
	Graphics::TColor FColorLED3;
	Graphics::TColor FColorOff1;
	Graphics::TColor FColorOff2;
	Graphics::TColor FColorOff3;
	Graphics::TColor FColorSeperator;
	TMeterDirection FDirection;
	int FMax;
	int FMin;
	int FNumDigits;
	int FPosition;
	int FStartColor2;
	int FStartColor3;
	Classes::TNotifyEvent FOnChange;
	void __fastcall setBevelStyle(Controls::TBevelCut newVal);
	void __fastcall setColorLED1(Graphics::TColor newVal);
	void __fastcall setColorLED2(Graphics::TColor newVal);
	void __fastcall setColorLED3(Graphics::TColor newVal);
	void __fastcall setColorSeperator(Graphics::TColor newVal);
	void __fastcall setDirection(TMeterDirection newVal);
	void __fastcall setMax(int newVal);
	void __fastcall setMin(int newVal);
	void __fastcall setNumDigits(int newVal);
	void __fastcall setPosition(int newVal);
	void __fastcall setStartColor2(int newVal);
	void __fastcall setStartColor3(int newVal);
	
protected:
	virtual void __fastcall Paint(void);
	DYNAMIC void __fastcall Change(void);
	
public:
	__fastcall virtual TLEDMeter(Classes::TComponent* AOwner);
	__fastcall virtual ~TLEDMeter(void);
	
__published:
	__property TAboutProperty* About = {read=FAbout, write=FAbout};
	__property Controls::TBevelCut BevelStyle = {read=FBevelStyle, write=setBevelStyle, nodefault};
	__property Graphics::TColor ColorLED1 = {read=FColorLED1, write=setColorLED1, nodefault};
	__property Graphics::TColor ColorLED2 = {read=FColorLED2, write=setColorLED2, nodefault};
	__property Graphics::TColor ColorLED3 = {read=FColorLED3, write=setColorLED3, nodefault};
	__property Graphics::TColor ColorSeperator = {read=FColorSeperator, write=setColorSeperator, nodefault
		};
	__property Cursor ;
	__property TMeterDirection Direction = {read=FDirection, write=setDirection, nodefault};
	__property DragCursor ;
	__property DragMode ;
	__property int Max = {read=FMax, write=setMax, nodefault};
	__property int Min = {read=FMin, write=setMin, nodefault};
	__property int NumDigits = {read=FNumDigits, write=setNumDigits, nodefault};
	__property int Position = {read=FPosition, write=setPosition, nodefault};
	__property int StartColor2 = {read=FStartColor2, write=setStartColor2, nodefault};
	__property int StartColor3 = {read=FStartColor3, write=setStartColor3, nodefault};
	__property Visible ;
	__property Classes::TNotifyEvent OnChange = {read=FOnChange, write=FOnChange};
	__property OnDragDrop ;
	__property OnDragOver ;
	__property OnEndDrag ;
	__property OnMouseDown ;
	__property OnMouseMove ;
	__property OnMouseUp ;
};

#pragma pack(pop)

//-- var, const, procedure ---------------------------------------------------
static const Shortint DefaultWidth = 0x2d;
static const Shortint DefaultHeight = 0x2d;
static const Shortint DefaultDepth = 0x3;
#define VNr "v1.0"
extern PACKAGE void __fastcall Register(void);

}	/* namespace Rackctls */
#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace Rackctls;
#endif
#pragma option pop	// -w-

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// RackCtls
