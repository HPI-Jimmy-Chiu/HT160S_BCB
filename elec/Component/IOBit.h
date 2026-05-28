//---------------------------------------------------------------------------
#ifndef IOBitH
#define IOBitH
#include "ioport.hpp"
#include "IOByte.h"

//---------------------------------------------------------------------------
//  IO Bit ¸ê®Æ
//---------------------------------------------------------------------------
class PACKAGE IOBit
{
public:
    IOBit(int iPortNo,int iBitNo,bool Read = false);
    ~IOBit();
    void    Out(bool Value);
    bool    In();

private:
    IOByte  *IOByteObj;
    byte     BitNo;

protected:
};

//---------------------------------------------------------------------------
#endif
