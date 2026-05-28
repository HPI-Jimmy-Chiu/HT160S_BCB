//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "IOByte.h"
#include <assert.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)

//---------------------------------------------------------------------------
#define     MAX_PORT_QUAN   1024

//---------------------------------------------------------------------------
static  int         iPortAlloc = 0;                 // PORT己配置數量
static  PORT_DATA   PortDataArr[MAX_PORT_QUAN];     // I/O PORT狀態陣列
static  bool        HPMInitialization = false;      // 是不已起動Driver

//---------------------------------------------------------------------------
//  建構子
//---------------------------------------------------------------------------
IOByte::IOByte(int iNo,bool Read)
{
    iPortNo  = iNo;
    ReadFlag = Read;
    bool Flag = false;
    for (int iP = 0;iP < iPortAlloc;iP ++) {
        if (PortDataArr[iP].iNo == iNo) {   // 如果有同編號的I/O PORT
            PortData = &PortDataArr[iP];    // 指向該區
            Flag = true;
            break;
        }
    }

    if (!Flag) {                                    // 如果尚未配置此一編號的PORT
        assert(iPortAlloc < MAX_PORT_QUAN);

        InitNTPort();                               // 起始NT Port Driver
        EnableNTPort(iNo,iNo);                      // ENABLE NT I/O PORT

        PortDataArr[iPortAlloc].iNo   = iNo;
        if (ReadFlag)
            PortDataArr[iPortAlloc].Value = inportb(iNo);
        else
            PortDataArr[iPortAlloc].Value = 0;
        PortData = &PortDataArr[iPortAlloc];
        iPortAlloc ++;
    }
}

//---------------------------------------------------------------------------
//  送出資料到PORT
//---------------------------------------------------------------------------
void IOByte::Out(byte Value)
{
    PortData->Value = Value;            // 先記錄到變數中
    outportb(iPortNo,Value);            // 送出
}

//---------------------------------------------------------------------------
//  讀回PORT資料
//---------------------------------------------------------------------------
byte IOByte::In()
{
    if (ReadFlag) {                             // 如果需要讀回資料
        PortData->Value = inportb(iPortNo);     // 讀回資料
    }
    return PortData->Value;                     // 傳回
}

//---------------------------------------------------------------------------
//  起始NT I/O PORT DRIVER
//---------------------------------------------------------------------------
void InitNTPort()
{
    // 要求Driver要使用本Port ........................
    if (!HPMInitialization) {                           // 如果尚未起動Driver
        GWIOPM_Driver->OpenSCM();                       // 開啟Driver
        GWIOPM_Driver->Install(                         // 安裝Driver
                    ExtractFilePath(Application->ExeName) + "HKNT_IO");
        GWIOPM_Driver->Start();                         // Start Driver
        GWIOPM_Driver->DeviceOpen();                    // 開啟Device
        HPMInitialization = true;
    }
}

//---------------------------------------------------------------------------
//  Enable NT上的I/O PORT
//---------------------------------------------------------------------------
void EnableNTPort(int iNo,int iENo)
{
    bool Flag = true;
    if (GWIOPM_Driver->LIOPM_Set_Ports(iNo,iENo,true) == ERROR_SUCCESS) {
        if (GWIOPM_Driver->IOCTL_IOPMD_ACTIVATE_KIOPM() != ERROR_SUCCESS)
           Flag = false;
    }
    else Flag = false;

    if (!Flag) {
        char cBuf[100];
        wsprintf(cBuf,"無法起始NT上的I/O PORT:%x",iNo);
        ShowMessage(cBuf);
    }
}

//---------------------------------------------------------------------------
//  Disable NT上的I/O PORT
//---------------------------------------------------------------------------
void DisableNTPort(int iNo,int iENo)
{
    if (GWIOPM_Driver->LIOPM_Set_Ports(iNo,iENo,false) != ERROR_SUCCESS) {
        char cBuf[100];
        wsprintf(cBuf,"無法關閉NT上的I/O PORT:%x",iNo);
        ShowMessage(cBuf);
    }
}

//---------------------------------------------------------------------------
//  關閉NT I/O PORT DRIVER
//---------------------------------------------------------------------------
void CloseNTPort()
{
    // 要求Driver要使用本Port ........................
    if (HPMInitialization) {                            // 如果尚未起動Driver
//        GWIOPM_Driver->DeviceClose();
        GWIOPM_Driver->Stop();                           // Stop Device
        GWIOPM_Driver->Remove();                         // 移除Device
        GWIOPM_Driver->CloseSCM();                       // 開關Driver物件
        HPMInitialization = false;
    }
}

