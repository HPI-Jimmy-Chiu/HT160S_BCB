// Borland C++ Builder
// Copyright (c) 1995, 1999 by Borland International
// All rights reserved

// (DO NOT EDIT: machine generated header) 'APHeadLabel.pas' rev: 4.00

#ifndef APHeadLabelHPP
#define APHeadLabelHPP

#pragma delphiheader begin
#pragma option push -w-
#include <Menus.hpp>	// Pascal unit
#include <DsgnIntf.hpp>	// Pascal unit
#include <ExtCtrls.hpp>	// Pascal unit
#include <StdCtrls.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Dialogs.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Apheadlabel
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TGlyphMode { gmNormal, gmOverlayed };
#pragma option pop

#pragma option push -b-
enum TGradientStyle { gsNone, gsBottom, gsLeft, gsRight, gsTop };
#pragma option pop

#pragma option push -b-
enum TTextAlignmt { taCenter, taLeft, taRight };
#pragma option pop

typedef Byte TColorSteps;

typedef Shortint TNumGlyphs;

typedef Graphics::TColor TGradientArray[255];

#pragma option push -b-
enum APHeadLabel__1 { blLeft, blTop, blRight, blBottom };
#pragma option pop

typedef Set<APHeadLabel__1, blLeft, blBottom>  TBoundLines;

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

class DELPHICLASS TGradient;
class DELPHICLASS TAPHeadLabel;
class DELPHICLASS TSubCaption;
#pragma pack(push, 4)
class PASCALIMPLEMENTATION TSubCaption : public Classes::TPersistent 
{
	typedef Classes::TPersistent inherited;
	
private:
	TAPHeadLabel* Parent;
	AnsiString FCaption;
	Graphics::TColor FColor;
	Graphics::TFontStyles FStyle;
	Byte FSize;
	bool FEllipsis;
	Byte FMargin;
	void __fastcall SetMargin(Byte Value);
	void __fastcall SetSubCaption(AnsiString Value);
	void __fastcall SetSubColor(Graphics::TColor Value);
	void __fastcall SetSubStyle(Graphics::TFontStyles Value);
	void __fastcall SetSubSize(Byte Value);
	void __fastcall SetSubEllipsis(bool Value);
	
__published:
	__property AnsiString Caption = {read=FCaption, write=SetSubCaption};
	__property Graphics::TColor Color = {read=FColor, write=SetSubColor, default=-2147483640};
	__property bool Ellipsis = {read=FEllipsis, write=SetSubEllipsis, nodefault};
	__property Byte Size = {read=FSize, write=SetSubSize, default=8};
	__property Byte Margin = {read=FMargin, write=SetMargin, default=8};
	__property Graphics::TFontStyles Style = {read=FStyle, write=SetSubStyle, nodefault};
public:
	#pragma option push -w-inl
	/* TPersistent.Destroy */ inline __fastcall virtual ~TSubCaption(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Create */ inline __fastcall TSubCaption(void) : Classes::TPersistent() { }
	#pragma option pop
	
};

#pragma pack(pop)

#pragma pack(push, 4)
class PASCALIMPLEMENTATION TAPHeadLabel : public Stdctrls::TCustomLabel 
{
	typedef Stdctrls::TCustomLabel inherited;
	
private:
	Graphics::TColor GradientBand[255];
	TAboutProperty* FAbout;
	bool FAutoBounds;
	Controls::TAlign FAlign;
	TBoundLines FBoundLines;
	Graphics::TColor FBoundColor;
	Graphics::TFont* FFont;
	bool FFormMove;
	TGradient* FGradient;
	Graphics::TBitmap* FGlyph;
	TGlyphMode FGlyphMode;
	Byte FMargin;
	TNumGlyphs FNumGlyphs;
	Byte FSpacing;
	TSubCaption* FSubCaption;
	TTextAlignmt FTextAlignmt;
	Classes::TNotifyEvent FOnClick;
	Classes::TNotifyEvent FOnDblClick;
	Classes::TNotifyEvent FOnGlyphClick;
	Classes::TNotifyEvent FOnGlyphDblClick;
	HIDESBASE MESSAGE void __fastcall CMTextChanged(Messages::TMessage &Msg);
	HIDESBASE MESSAGE void __fastcall CMFontChanged(Messages::TMessage &Msg);
	HIDESBASE MESSAGE void __fastcall WMLButtonDown(Messages::TWMMouse &Msg);
	HIDESBASE MESSAGE void __fastcall WMLButtonDblClk(Messages::TWMMouse &Msg);
	void __fastcall DrawGradientBand(void);
	void __fastcall SetAutoBounds(bool Value);
	HIDESBASE void __fastcall SetAlign(Controls::TAlign Value);
	void __fastcall SetBoundLines(TBoundLines Value);
	void __fastcall SetBoundColor(Graphics::TColor Value);
	void __fastcall SetFormMove(bool Value);
	void __fastcall SetGlyph(Graphics::TBitmap* Value);
	void __fastcall SetGlyphMode(TGlyphMode Value);
	void __fastcall SetMargin(Byte Value);
	void __fastcall SetNumGlyphs(TNumGlyphs Value);
	void __fastcall SetSpacing(Byte Value);
	void __fastcall SetTextAlignmt(TTextAlignmt Value);
	void __fastcall CalcNumGlyphs(void);
	void __fastcall ResizeHeadLabel(void);
	AnsiString __fastcall GetClippedCaption(int Offset);
	AnsiString __fastcall GetSubCaption();
	bool __fastcall MouseInGlyph(void);
	
protected:
	virtual void __fastcall Paint(void);
	void __fastcall CalcGradientBand(Graphics::TColor StartCol, Graphics::TColor EndCol, Byte Steps);
	
public:
	__fastcall virtual TAPHeadLabel(Classes::TComponent* AOwner);
	__fastcall virtual ~TAPHeadLabel(void);
	
__published:
	__property TAboutProperty* About = {read=FAbout, write=FAbout};
	__property TTextAlignmt Alignment = {read=FTextAlignmt, write=SetTextAlignmt, default=1};
	__property Controls::TAlign Align = {read=FAlign, write=SetAlign, default=0};
	__property bool AutoBounds = {read=FAutoBounds, write=SetAutoBounds, default=1};
	__property TBoundLines BoundLines = {read=FBoundLines, write=SetBoundLines, nodefault};
	__property Graphics::TColor BoundColor = {read=FBoundColor, write=SetBoundColor, default=8421504};
	__property bool FormMove = {read=FFormMove, write=SetFormMove, default=0};
	__property Graphics::TBitmap* Glyph = {read=FGlyph, write=SetGlyph};
	__property TGlyphMode GlyphMode = {read=FGlyphMode, write=SetGlyphMode, default=0};
	__property TGradient* Gradient = {read=FGradient, write=FGradient};
	__property Byte Margin = {read=FMargin, write=SetMargin, default=5};
	__property TNumGlyphs NumGlyphs = {read=FNumGlyphs, write=SetNumGlyphs, default=1};
	__property Byte Spacing = {read=FSpacing, write=SetSpacing, default=5};
	__property TSubCaption* SubCaption = {read=FSubCaption, write=FSubCaption};
	__property Anchors ;
	__property BiDiMode ;
	__property Caption ;
	__property Constraints ;
	__property Enabled ;
	__property FocusControl ;
	__property Font ;
	__property ParentBiDiMode ;
	__property ParentFont ;
	__property ParentShowHint ;
	__property PopupMenu ;
	__property ShowHint ;
	__property Classes::TNotifyEvent OnClick = {read=FOnClick, write=FOnClick};
	__property Classes::TNotifyEvent OnDblClick = {read=FOnDblClick, write=FOnDblClick};
	__property Classes::TNotifyEvent OnGlyphClick = {read=FOnGlyphClick, write=FOnGlyphClick};
	__property Classes::TNotifyEvent OnGlyphDblClick = {read=FOnGlyphDblClick, write=FOnGlyphDblClick};
		
	__property OnDragDrop ;
	__property OnDragOver ;
	__property OnEndDrag ;
	__property OnMouseDown ;
	__property OnMouseUp ;
	__property OnMouseMove ;
	__property OnStartDrag ;
};

#pragma pack(pop)

#pragma pack(push, 4)
class PASCALIMPLEMENTATION TGradient : public Classes::TPersistent 
{
	typedef Classes::TPersistent inherited;
	
private:
	TAPHeadLabel* Parent;
	TColorSteps FColorSteps;
	Graphics::TColor FEndColor;
	Graphics::TColor FStartColor;
	TGradientStyle FGradientStyle;
	void __fastcall SetColorSteps(TColorSteps Value);
	void __fastcall SetEndColor(Graphics::TColor Value);
	void __fastcall SetGradientStyle(TGradientStyle Value);
	void __fastcall SetStartColor(Graphics::TColor Value);
	
__published:
	__property TColorSteps ColorSteps = {read=FColorSteps, write=SetColorSteps, default=64};
	__property Graphics::TColor EndColor = {read=FEndColor, write=SetEndColor, default=12632256};
	__property Graphics::TColor StartColor = {read=FStartColor, write=SetStartColor, default=8421504};
	__property TGradientStyle Style = {read=FGradientStyle, write=SetGradientStyle, default=2};
public:
	#pragma option push -w-inl
	/* TPersistent.Destroy */ inline __fastcall virtual ~TGradient(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Create */ inline __fastcall TGradient(void) : Classes::TPersistent() { }
	#pragma option pop
	
};

#pragma pack(pop)

class DELPHICLASS TAPHeadEditor;
#pragma pack(push, 4)
class PASCALIMPLEMENTATION TAPHeadEditor : public Dsgnintf::TDefaultEditor 
{
	typedef Dsgnintf::TDefaultEditor inherited;
	
public:
	virtual void __fastcall ExecuteVerb(int Index);
	virtual AnsiString __fastcall GetVerb(int Index);
	virtual int __fastcall GetVerbCount(void);
public:
	#pragma option push -w-inl
	/* TComponentEditor.Create */ inline __fastcall virtual TAPHeadEditor(Classes::TComponent* AComponent
		, Dsgnintf::_di_IFormDesigner ADesigner) : Dsgnintf::TDefaultEditor(AComponent, ADesigner) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TObject.Destroy */ inline __fastcall virtual ~TAPHeadEditor(void) { }
	#pragma option pop
	
};

#pragma pack(pop)

//-- var, const, procedure ---------------------------------------------------
extern PACKAGE void __fastcall Register(void);

}	/* namespace Apheadlabel */
#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace Apheadlabel;
#endif
#pragma option pop	// -w-

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// APHeadLabel
