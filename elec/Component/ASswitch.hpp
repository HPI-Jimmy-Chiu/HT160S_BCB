// Borland C++ Builder
// Copyright (c) 1995, 1999 by Borland International
// All rights reserved

// (DO NOT EDIT: machine generated header) 'ASswitch.pas' rev: 5.00

#ifndef ASswitchHPP
#define ASswitchHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <ASFunc.hpp>	// Pascal unit
#include <DsgnIntf.hpp>	// Pascal unit
#include <MMSystem.hpp>	// Pascal unit
#include <Menus.hpp>	// Pascal unit
#include <Dialogs.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Asswitch
{
//-- type declarations -------------------------------------------------------
class DELPHICLASS TASSoundFileChange;
class PASCALIMPLEMENTATION TASSoundFileChange : public Dsgnintf::TStringProperty 
{
	typedef Dsgnintf::TStringProperty inherited;
	
public:
	virtual Dsgnintf::TPropertyAttributes __fastcall GetAttributes(void);
	virtual void __fastcall Edit(void);
protected:
	#pragma option push -w-inl
	/* TPropertyEditor.Create */ inline __fastcall virtual TASSoundFileChange(const Dsgnintf::_di_IFormDesigner 
		ADesigner, int APropCount) : Dsgnintf::TStringProperty(ADesigner, APropCount) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TPropertyEditor.Destroy */ inline __fastcall virtual ~TASSoundFileChange(void) { }
	#pragma option pop
	
};


#pragma option push -b-
enum TWhereSoundStore { wssExe, wssWav, wssNone };
#pragma option pop

#pragma option push -b-
enum TWaveOption { woSync, woNoDefault, woLoop, woNoStop };
#pragma option pop

typedef Set<TWaveOption, woSync, woNoStop>  TWaveOptions;

class DELPHICLASS TASSwitcher;
class PASCALIMPLEMENTATION TASSwitcher : public Controls::TCustomControl 
{
	typedef Controls::TCustomControl inherited;
	
private:
	int FMin;
	int FMax;
	int FPosition;
	bool FPrintValues;
	TWhereSoundStore FSoundType;
	AnsiString FSoundChange;
	AnsiString FResourceType;
	Word FWaveOptions;
	TWaveOptions FPalyOptions;
	bool FDrawFocused;
	bool FDrawTicks_;
	Classes::TNotifyEvent FOnChange;
	void __fastcall SetMin(int Value);
	void __fastcall SetMax(int Value);
	void __fastcall SetPosition(int Value);
	void __fastcall SetPrintValues(bool Value);
	void __fastcall SetSoundType(TWhereSoundStore Value);
	void __fastcall SetSoundChange(AnsiString Value);
	void __fastcall SetResourceType(AnsiString Value);
	void __fastcall SetDrawFocused(bool Value);
	void __fastcall SetDrawTicks(bool Value);
	__property int Min = {read=FMin, write=SetMin, default=0};
	void __fastcall GradValues_(void);
	void __fastcall GradToCoords_(void);
	void __fastcall SearchGradForPoint(void);
	void __fastcall TextValuesTicks(void);
	HIDESBASE MESSAGE void __fastcall CMMouseLeave(Messages::TMessage &Msg);
	MESSAGE void __fastcall WMGetDlgCode(Messages::TWMNoParams &Msg);
	
protected:
	DYNAMIC void __fastcall Change(void);
	virtual void __fastcall DrawTicks(void);
	virtual void __fastcall DrawFace(void);
	virtual void __fastcall DrawPointer(void);
	virtual void __fastcall Paint(void);
	DYNAMIC void __fastcall MouseDown(Controls::TMouseButton Button, Classes::TShiftState Shift, int X, 
		int Y);
	DYNAMIC void __fastcall MouseMove(Classes::TShiftState Shift, int X, int Y);
	DYNAMIC void __fastcall MouseUp(Controls::TMouseButton Button, Classes::TShiftState Shift, int X, int 
		Y);
	DYNAMIC void __fastcall KeyDown(Word &Key, Classes::TShiftState Shift);
	DYNAMIC void __fastcall DoEnter(void);
	DYNAMIC void __fastcall DoExit(void);
	
public:
	__fastcall virtual TASSwitcher(Classes::TComponent* AOwner);
	
__published:
	__property int Max = {read=FMax, write=SetMax, default=10};
	__property int Position = {read=FPosition, write=SetPosition, default=1};
	__property bool PrintValues = {read=FPrintValues, write=SetPrintValues, default=0};
	__property TWhereSoundStore SoundType = {read=FSoundType, write=SetSoundType, default=1};
	__property AnsiString SoundChange = {read=FSoundChange, write=SetSoundChange};
	__property AnsiString ResourceType = {read=FResourceType, write=SetResourceType};
	__property TWaveOptions WaveOptions = {read=FPalyOptions, write=FPalyOptions, default=11};
	__property bool DrawFocused = {read=FDrawFocused, write=SetDrawFocused, default=0};
	__property bool DrawTicks_ = {read=FDrawTicks_, write=SetDrawTicks, default=1};
	__property Classes::TNotifyEvent OnChange = {read=FOnChange, write=FOnChange};
	__property Enabled ;
	__property Color ;
	__property Hint ;
	__property HelpContext ;
	__property ParentShowHint ;
	__property ShowHint ;
	__property Visible ;
	__property DragCursor ;
	__property DragMode ;
	__property PopupMenu ;
	__property OnClick ;
	__property OnMouseDown ;
	__property OnMouseMove ;
	__property OnMouseUp ;
	__property OnDragDrop ;
	__property OnDragOver ;
	__property OnEndDrag ;
	__property OnKeyDown ;
	__property OnKeyPress ;
	__property OnKeyUp ;
	__property OnEnter ;
	__property OnExit ;
public:
	#pragma option push -w-inl
	/* TCustomControl.Destroy */ inline __fastcall virtual ~TASSwitcher(void) { }
	#pragma option pop
	
public:
	#pragma option push -w-inl
	/* TWinControl.CreateParented */ inline __fastcall TASSwitcher(HWND ParentWindow) : Controls::TCustomControl(
		ParentWindow) { }
	#pragma option pop
	
};


//-- var, const, procedure ---------------------------------------------------
extern PACKAGE int Offset;
extern PACKAGE int Offset2;
extern PACKAGE int Offset4;
extern PACKAGE int TickCount;
extern PACKAGE int i;
extern PACKAGE int j;
extern PACKAGE int StartA;
extern PACKAGE int EndA;
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
extern PACKAGE double PointX;
extern PACKAGE double PointY;
extern PACKAGE double PointX1;
extern PACKAGE double PointY1;
extern PACKAGE double RadTick;
extern PACKAGE int TmpAn;
extern PACKAGE int TmpAn1;
extern PACKAGE char *SoundFile;
extern PACKAGE char *SoundFile1;
extern PACKAGE int wText;
extern PACKAGE int hText;
extern PACKAGE Windows::TPoint sA;
extern PACKAGE Windows::TPoint eA;
extern PACKAGE void __fastcall Register(void);

}	/* namespace Asswitch */
#if !defined(NO_IMPLICIT_NAMESPACE_USE)
using namespace Asswitch;
#endif
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// ASswitch
