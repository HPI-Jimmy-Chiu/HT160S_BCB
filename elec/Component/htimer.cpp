/*---------------------------------------------------------------------------
	程式目的:計時器
    設計人員:陳鴻德
    設計日期:1998/04/06
  --------------------------------------------------------------------------- */
#include <vcl\vcl.h>
#pragma hdrstop
#include "HTimer.h"
#include <list>
#include "HThread.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

static TList  *HTimerList = NULL;
static HMutex *ThMutex = NULL;

//---------------------------------------------------------------------------
//  建構子
//---------------------------------------------------------------------------
__fastcall HTimer::HTimer()
{
    if (ThMutex == NULL)
    {
        ThMutex = new HMutex;
    }

    if (HTimerList == NULL)
    {
        HTimerList = new TList;
        HTimerList->Clear();
    }

    HTimerList->Add(this);
    Clear();
}

//---------------------------------------------------------------------------
//  解構子
//---------------------------------------------------------------------------
__fastcall HTimer::~HTimer()
{
    for (int iP = 0;iP < HTimerList->Count;iP ++)
    {
        if (HTimerList->Items[iP] == (void *) this)
        {
            HTimerList->Delete(iP);
        }
    }

    if (HTimerList->Count == 0)
    {
        HTimerList->Clear();
        delete HTimerList;
        HTimerList = NULL;
        if(ThMutex!=NULL)
        {
            delete ThMutex;
            ThMutex = NULL;
        }
    }
}

//---------------------------------------------------------------------------
//	設定計時器
//	iTime:計時時間(單位0.1秒)
//---------------------------------------------------------------------------
void HTimer::Set(int iTime)
{
	iTimeLen = iTime * 100;
}
//---------------------------------------------------------------------------
void HTimer::SetSec(double iTime)
{
	iTimeLen = iTime * 1000.0;
}
//---------------------------------------------------------------------------
void HTimer::SetSecAndOn(double iTime)	  // 計定計時長度
{
    Clear();
    SetSec(iTime);
    On();
}
//---------------------------------------------------------------------------
void HTimer::SetMSAndOn(int iTime)	  // 計定計時長度
{
    Clear();
    SetMS(iTime);
    On();
}  
//---------------------------------------------------------------------------
void HTimer::ResetMSAndOn(int iTime)
{
    Clear();
    SetMS(iTime);
    On();
}
//---------------------------------------------------------------------------
//	設定計時器
//	iTime:計時時間(單位0.1秒)
//---------------------------------------------------------------------------
void HTimer::SetTimer(int iTime,int iID)
{
    if( iTimerID != iID )
        Clear();

	iTimeLen=iTime * 100;
    iTimerID=iID;
}
//---------------------------------------------------------------------------
//	設定計時器(1/1000 SEC)
//	iTime:計時時間(單位0.001秒)
//---------------------------------------------------------------------------
void HTimer::SetMS(int iTime)
{
	iTimeLen = iTime;
}
//---------------------------------------------------------------------------
// 	開始計時
//---------------------------------------------------------------------------
void HTimer::On()
{
	//if (!InUsed)
    ulStartTicks = GetTickCount();
    InUsed = true;
}
//---------------------------------------------------------------------------
// 	讀取計時器是否時間到
//---------------------------------------------------------------------------
bool HTimer::Off()
{
    if(Paused)
        return false;

    if(ulStartTicks==0)
        return false;

    // if value=0,timer-->ON
    if(iTimeLen<=0)
    {
        //jou 2012-01-04 程式被設入負的等待時間會hang up。
            return true;
    }

    DWORD ulLimited  = ulStartTicks + iTimeLen + iPauseLen;
    DWORD ulNowTicks = ::GetTickCount();
    DWORD ulNowTicksOver=0;
    if(ulLimited < ulStartTicks)               // 如果超過DWORD範圍
    {
         ulLimited = 0xFFFFFFFF-ulStartTicks+ iTimeLen + iPauseLen;
         ulNowTicksOver = 0xFFFFFFFF-ulStartTicks+ ulNowTicks;
//         if (ulNowTicks < ulStartTicks                   // 如果小於開始的Ticks且
//             && ulNowTicks > ulLimited)          // 大於界限值
//        {
//                    InUsed = false;
//             return true;
//        }
        if(ulNowTicksOver < ulStartTicks &&                  // 如果小於開始的Ticks且
           ulNowTicksOver > ulLimited)          // 大於界限值
        {
            InUsed = false;
            return true;
        }
    }
    else
    {
        if(ulNowTicks >= (ulStartTicks + iTimeLen))
        {
            InUsed = false;
            return true;
        }
    }
    return false;
}
//---------------------------------------------------------------------------
//  清除計時值
//---------------------------------------------------------------------------
void HTimer::Clear()
{
    ThMutex->Use();
    ulStartTicks = 0;
    ulPauseTicks = 0;
    iTimeLen = 0;
    InUsed = false;
    Paused = false;
    iPauseLen = 0;
    ThMutex->Release();
}
//---------------------------------------------------------------------------
//  暫停計時
//---------------------------------------------------------------------------
void HTimer::Pause()
{
    ThMutex->Use();
    Paused = true;                      // 設定暫停
    ulPauseTicks = GetTickCount();      // 記錄Tick count
    ThMutex->Release();
}
//---------------------------------------------------------------------------
//  重新啟動
//---------------------------------------------------------------------------
void HTimer::ReStart()
{
    ThMutex->Use();
    Paused = false;
    // 計算總共暫停多久
    DWORD Ticks = GetTickCount();
    DWORD Max = 1;
    Max -= 2;
    if(Ticks < ulPauseTicks)                   // 如果超過DWORD範圍
       // iPauseLen = (Max - ulPauseTicks)+Ticks; // 尾段數值+超過的部份
        iPauseLen = 0xFFFFFFFF-(Max - ulPauseTicks)+Ticks;  //kevin 20140913
    else
        iPauseLen = Ticks - ulPauseTicks;       // 目前值-暫停起始值

    ThMutex->Release();
}
//---------------------------------------------------------------------------
//  清除所有計時器的值
//---------------------------------------------------------------------------
void ClearAllTimer()
{
    for (int iP = 0;iP < HTimerList->Count;iP ++)
    {
        HTimer *P = (HTimer *) HTimerList->Items[iP];
        P->Clear();
    }
}
//---------------------------------------------------------------------------
//  暫停所有計時器
//---------------------------------------------------------------------------
void PauseAllTimer()
{
    for(int iP = 0;iP < HTimerList->Count;iP ++)
    {
        HTimer *P = (HTimer *) HTimerList->Items[iP];
        P->Pause();
    }
}
//---------------------------------------------------------------------------
//  重新起動所有計時器
//---------------------------------------------------------------------------
void ReStartAllTimer()
{
    for(int iP = 0;iP < HTimerList->Count;iP ++)
    {
        HTimer *P = (HTimer *) HTimerList->Items[iP];
        P->ReStart();
    }
}
//---------------------------------------------------------------------------
