//---------------------------------------------------------------------------
#ifndef HDateConvertH
#define HDateConvertH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include <Db.hpp>
#include <DBCtrls.hpp>
#include <DBTables.hpp>


//---------------------------------------------------------------------------
struct HLFieldListItem;
typedef HLFieldListItem *PHLFieldListItem;

struct HLFieldListItem
{
	int Tag;
	TField* Field;
	AnsiString DisplayFormat;
	AnsiString EditMask;
	TFieldGetTextEvent OnGetText;
	TFieldSetTextEvent OnSetText;
} ;

typedef void __fastcall (__closure *TCheckDateEvent)(const System::AnsiString Text, bool &IsValid);

class PACKAGE HDateConvert : public TComponent
{
private:
	System::AnsiString FDisplayFormat;
	System::AnsiString FEditMask;
	Classes::TList* FDataSets;
	Classes::TList* FFields;
	int FYearDigits;
	TCheckDateEvent     FOnCheckDate;

	void    __fastcall FreeFieldListItems();
	int     __fastcall IndexOfField(Db::TField* pField);
	void    __fastcall SetYearDigits(int Value);

protected:
	DYNAMIC void __fastcall HLGetText(Db::TField* Sender, System::AnsiString &Text, bool DisplayText);
	DYNAMIC void __fastcall HLSetText(Db::TField* Sender, const System::AnsiString Text);
	void __fastcall GetRocDate(Db::TField* Sender, System::AnsiString &Text, bool DisplayText);
	void __fastcall SetRocDate(Db::TField* Sender, const System::AnsiString Text);

public:
    __fastcall virtual HDateConvert(TComponent* Owner);
	__fastcall virtual ~HDateConvert();
	void __fastcall Add(Dbtables::TDBDataSet* DataSet);
	void __fastcall Convert();

__published:
	__property int YearDigits = {read=FYearDigits, write=SetYearDigits, default=2};
	__property TCheckDateEvent OnCheckDate = {read=FOnCheckDate, write=FOnCheckDate};
};
//---------------------------------------------------------------------------
extern PACKAGE int HStrToInt( char *str, int len_str );        // ¦r¦êÂà¾ã¼Æ(Integer)
#endif
