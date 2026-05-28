// Borland C++ Builder
// Copyright (c) 1995, 2002 by Borland Software Corporation
// All rights reserved

// (DO NOT EDIT: machine generated header) 'XLSFile.pas' rev: 6.00

#ifndef XLSFileHPP
#define XLSFileHPP

#pragma delphiheader begin
#pragma option push -w-
#pragma option push -Vx
#include <ComCtrls.hpp>	// Pascal unit
#include <DBCtrls.hpp>	// Pascal unit
#include <DB.hpp>	// Pascal unit
#include <Dialogs.hpp>	// Pascal unit
#include <Forms.hpp>	// Pascal unit
#include <Grids.hpp>	// Pascal unit
#include <Controls.hpp>	// Pascal unit
#include <Graphics.hpp>	// Pascal unit
#include <Classes.hpp>	// Pascal unit
#include <SysUtils.hpp>	// Pascal unit
#include <Messages.hpp>	// Pascal unit
#include <Windows.hpp>	// Pascal unit
#include <SysInit.hpp>	// Pascal unit
#include <System.hpp>	// Pascal unit

//-- user supplied -----------------------------------------------------------

namespace Xlsfile
{
//-- type declarations -------------------------------------------------------
#pragma option push -b-
enum TAtributCell { acHidden, acLocked, acShaded, acBottomBorder, acTopBorder, acRightBorder, acLeftBorder, acLeft, acCenter, acRight, acFill };
#pragma option pop

typedef Set<TAtributCell, acHidden, acFill>  TSetOfAtribut;

class DELPHICLASS TXLSWriter;
class PASCALIMPLEMENTATION TXLSWriter : public System::TObject 
{
	typedef System::TObject inherited;
	
private:
	Classes::TFileStream* fstream;
	void __fastcall WriteWord(Word w);
	
protected:
	void __fastcall WriteBOF(void);
	void __fastcall WriteEOF(void);
	void __fastcall WriteDimension(void);
	
public:
	Word maxCols;
	Word maxRows;
	void __fastcall CellWord(Word vCol, Word vRow, Word aValue, TSetOfAtribut vAtribut = System::Set<TAtributCell, acHidden, acFill> () );
	void __fastcall CellDouble(Word vCol, Word vRow, double aValue, TSetOfAtribut vAtribut = System::Set<TAtributCell, acHidden, acFill> () );
	void __fastcall CellStr(Word vCol, Word vRow, AnsiString aValue, TSetOfAtribut vAtribut = System::Set<TAtributCell, acHidden, acFill> () );
	void __fastcall WriteField(Word vCol, Word vRow, Db::TField* Field);
	__fastcall TXLSWriter(AnsiString vFileName);
	__fastcall virtual ~TXLSWriter(void);
};


//-- var, const, procedure ---------------------------------------------------
static const Shortint CBOF = 0x9;
static const Word BIT_BIFF5 = 0x800;
static const Word BOF_BIFF5 = 0x809;
static const Shortint BIFF_EOF = 0xa;
static const Shortint DOCTYPE_XLS = 0x10;
static const Shortint DIMENSIONS = 0x0;
extern PACKAGE void __fastcall DataSetToXLS(Db::TDataSet* ds, AnsiString fname);
extern PACKAGE void __fastcall StringGridToXLS(Grids::TStringGrid* grid, AnsiString fname);
extern PACKAGE void __fastcall SetCellAtribut(TSetOfAtribut value, Byte * FAtribut, const int FAtribut_Size);

}	/* namespace Xlsfile */
using namespace Xlsfile;
#pragma option pop	// -w-
#pragma option pop	// -Vx

#pragma delphiheader end.
//-- end unit ----------------------------------------------------------------
#endif	// XLSFile
