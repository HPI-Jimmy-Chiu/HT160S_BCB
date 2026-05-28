//---------------------------------------------------------------------------
#ifndef MapArrayH
#define MapArrayH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
//---------------------------------------------------------------------------
class PACKAGE TMapArray : public TWinControl
{
private:
    void __fastcall Resize(void);
    TImage *map;
    int nXItem;
    int nYItem;

__published:
    __property  int XItem =
        { read = nXItem,write = SetXItem,default = 2 };
    __property  int YItem =
        { read = nYItem,write = SetYItem,default = 2 };

protected:
    virtual void __fastcall Paint();
    virtual void __fastcall WndProc(TMessage& Message);
    virtual void __fastcall CreateWnd();
    virtual void __fastcall OnChange();
public:
    __fastcall TMapArray(TComponent* Owner);

public:		// User declarations
    void __fastcall SetXItem(int a);
    void __fastcall SetYItem(int a);

    TRect R;
    int nXSpace;
    int nYSpace;
    int nWidth;
    int nHeight;
    int nPitchWidth;
    int nBackGroundColor;
    int nMap[100][100];
public:
    void ShowCell();
__published:
};
//---------------------------------------------------------------------------
#endif
 