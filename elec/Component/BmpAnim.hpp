// Borland C++ Builder
// Copyright (c) 1995, 1998 by Borland International
// All rights reserved

// (DO NOT EDIT: machine generated header) 'BmpAnim.pas' rev: 3.00

#ifndef BmpAnimHPP
#define BmpAnimHPP
#include <ExtCtrls.hpp>
#include <Commctrl.hpp>
#include <Controls.hpp>
#include <Graphics.hpp>
#include <Classes.hpp>
#include <Messages.hpp>
#include <Windows.hpp>
#include <SysUtils.hpp>
#include <SysInit.hpp>
#include <System.hpp>

//-- user supplied -----------------------------------------------------------

namespace Bmpanim
{
//-- type declarations -------------------------------------------------------
enum TOrientation { toHorizontal, toVertical };

enum TDirection { tdForward, tdBack, tdFwdBack, tdBackFwd };

class DELPHICLASS TCustomBmpAnimator;
class PASCALIMPLEMENTATION TCustomBmpAnimator : public Controls::TGraphicControl 
{
	typedef Controls::TGraphicControl inherited;
	
private:
	Controls::TImageList* FImageList;
	Extctrls::TTimer* FTimer;
	int FIndex;
	Graphics::TBitmap* FImage;
	bool FEnabled;
	int FWidth;
	int FHeight;
	int FNumGlyphs;
	TOrientation FOrientation;
	int FSpeed;
	bool FTransparent;
	bool FAutoSize;
	int FStart;
	int FStop;
	int FPosition;
	Graphics::TColor FColor;
	TDirection FDirection;
	bool FGoingUp;
	bool FCenter;
	void __fastcall SetCenter(bool Value);
	void __fastcall SetDirection(TDirection Value);
	HIDESBASE void __fastcall SetColor(Graphics::TColor Value);
	void __fastcall SetPosition(int Value);
	void __fastcall SetStart(int Value);
	void __fastcall SetStop(int Value);
	void __fastcall SetAutoSize(bool Value);
	void __fastcall SetTransparent(bool Value);
	void __fastcall SetImage(Graphics::TBitmap* Value);
	HIDESBASE void __fastcall SetEnabled(bool Value);
	void __fastcall SetNumGlyphs(int Value);
	void __fastcall SetOrientation(TOrientation Value);
	void __fastcall SetSpeed(int Value);
	void __fastcall TimerEvent(System::TObject* Sender);
	void __fastcall UpdateImages(void);
	
protected:
	virtual void __fastcall Paint(void);
	__property bool AutoSize = {read=FAutoSize, write=SetAutoSize, default=0};
	__property bool Centered = {read=FCenter, write=SetCenter, nodefault};
	__property Graphics::TColor Color = {read=FColor, write=SetColor, default=-2147483633};
	__property TDirection Direction = {read=FDirection, write=SetDirection, nodefault};
	__property bool Enabled = {read=FEnabled, write=SetEnabled, default=0};
	__property Graphics::TBitmap* Image = {read=FImage, write=SetImage};
	__property int NumFrames = {read=FNumGlyphs, write=SetNumGlyphs, default=0};
	__property TOrientation Orientation = {read=FOrientation, write=SetOrientation, default=0};
	__property int Position = {read=FPosition, write=SetPosition, default=0};
	__property int Speed = {read=FSpeed, write=SetSpeed, default=100};
	__property int Min = {read=FStart, write=SetStart, default=0};
	__property int Max = {read=FStop, write=SetStop, default=0};
	__property bool Transparent = {read=FTransparent, write=SetTransparent, default=0};
	
public:
	__fastcall virtual TCustomBmpAnimator(Classes::TComponent* AOwner);
	__fastcall virtual ~TCustomBmpAnimator(void);
};

class DELPHICLASS TBmpAnimator;
class PASCALIMPLEMENTATION TBmpAnimator : public Bmpanim::TCustomBmpAnimator 
{
	typedef Bmpanim::TCustomBmpAnimator inherited;
	
__published:
	__property Align ;
	__property AutoSize ;
	__property Centered ;
	__property Color ;
	__property Direction ;
	__property Enabled ;
	__property Height ;
	__property Image ;
	__property Left ;
	__property Name ;
	__property NumFrames ;
	__property Orientation ;
	__property Position ;
	__property Speed ;
	__property Min ;
	__property Max ;
	__property Tag ;
	__property Top ;
	__property Transparent ;
	__property Width ;
	__property OnClick ;
	__property OnMouseDown ;
	__property OnMouseMove ;
	__property OnMouseUp ;
	__property OnDragDrop ;
	__property OnEndDrag ;
	__property OnStartDrag ;
	__property OnDragOver ;
public:
	/* TCustomBmpAnimator.Create */ __fastcall virtual TBmpAnimator(Classes::TComponent* AOwner) : Bmpanim::
		TCustomBmpAnimator(AOwner) { }
	/* TCustomBmpAnimator.Destroy */ __fastcall virtual ~TBmpAnimator(void) { }
	
};

//-- var, const, procedure ---------------------------------------------------
extern PACKAGE void __fastcall Register(void);

}	/* namespace Bmpanim */
#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace Bmpanim;
#endif
//-- end unit ----------------------------------------------------------------
#endif	// BmpAnim
