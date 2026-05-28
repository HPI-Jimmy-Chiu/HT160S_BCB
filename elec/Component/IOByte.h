//---------------------------------------------------------------------------
#ifndef IOByteH
#define IOByteH
#include "gwiopm.hpp"
#include "ioport.hpp"

//---------------------------------------------------------------------------
typedef struct  {
    int     iNo;
    byte    Value;
} PORT_DATA;

//---------------------------------------------------------------------------
//  I/O BYTE資料
//---------------------------------------------------------------------------
class PACKAGE IOByte
{
public:
    IOByte(int iPortNo,bool Read = false);
    void    Out(byte Value);
    byte    In();

private:
    int         iPortNo;
    bool        ReadFlag;
    PORT_DATA   *PortData;

protected:
};

//---------------------------------------------------------------------------
//  NT PORT的相關函數
//---------------------------------------------------------------------------
extern void PACKAGE InitNTPort();                       // 起始NT I/O PORT DRIVER
extern void PACKAGE CloseNTPort();                      // 關閉NT I/O PORT DRIVER
extern void PACKAGE EnableNTPort(int iNo,int iENo);     // Enable NT上的I/O PORT
extern void PACKAGE DisableNTPort(int iNo,int iENo);    // Disable NT上的I/O PORT
#endif
