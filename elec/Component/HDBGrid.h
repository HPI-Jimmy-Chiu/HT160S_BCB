//---------------------------------------------------------------------------
#ifndef HDBGridH
#define HDBGridH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include "RXDBCtrl.hpp"
#include <DBGrids.hpp>
#include <Grids.hpp>
#include "RXLookup.hpp"
#include "ToolEdit.hpp"
#include "RXDBCtrl.hpp"

//---------------------------------------------------------------------------
class PACKAGE THDBGrid : public TRxDBGrid
{
private:
    TBookmark   OldPos;
    
protected:
    bool    InitAlloc;
    TList   *HDEditObj;
    DYNAMIC void __fastcall KeyDown(Word &Key, Classes::TShiftState Shift);
    DYNAMIC void __fastcall KeyPress(char &Key);
	DYNAMIC void __fastcall DrawColumnCell(const Windows::TRect &Rect, int DataCol, Dbgrids::TColumn* Column
		, Grids::TGridDrawState State);
    DYNAMIC void __fastcall ColExit();
    DYNAMIC bool __fastcall CanEditModify(void);
    void __fastcall OpKeyDown(TObject *Sender, WORD &Key,
          TShiftState Shift);
    void __fastcall OpKeyPress(TObject *Sender, char &Key);
    virtual void __fastcall WndProc(Messages::TMessage &Message);

public:
    __fastcall THDBGrid(TComponent* Owner);
    __fastcall ~THDBGrid();
__published:
};
//---------------------------------------------------------------------------
#endif
