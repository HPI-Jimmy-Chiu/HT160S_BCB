//---------------------------------------------------------------------------
//  執行緒用的同步類別
//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#include <stdlib.h>
#include <Filectrl.hpp>
#include <winbase.h>
#pragma hdrstop

#include "HThread.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

static int  iMutexNo = 0;

//---------------------------------------------------------------------------
//  建構子
//---------------------------------------------------------------------------
HMutex::HMutex()
{
    AnsiString s;
    s = "ComponentMUTEX_0" + IntToStr(iMutexNo ++);
    Handle = CreateMutex(NULL,false,s.c_str());    // 同步用的互斥元
}

//---------------------------------------------------------------------------
//  解構子
//---------------------------------------------------------------------------
HMutex::~HMutex()
{
    CloseHandle(Handle);      // RELEASE MUTEX
}
//---------------------------------------------------------------------------
//  申請使用Mutex
//---------------------------------------------------------------------------
void HMutex::Use()
{
    DWORD lTicks = GetTickCount() + 2000;
    DWORD lSTicks = GetTickCount();
    while (WaitForSingleObject(Handle,1) != WAIT_OBJECT_0) {   // 等待取得MUTEX
        if (GetTickCount() > lTicks ||
            GetTickCount() < lSTicks)
            break;
    }
}

//---------------------------------------------------------------------------
//  解構子
//---------------------------------------------------------------------------
void HMutex::Release()
{
    ReleaseMutex(Handle);
}

