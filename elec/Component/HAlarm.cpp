//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdlib.h>
#pragma hdrstop

#include "HAlarm.h"
#include <stdlib.h>

#include "HThread.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

//---------------------------------------------------------------------------
bool    SystemNG;               // 系統故障

typedef struct {                // 錯誤訊息結構
    TComponent  *ObjPtr;        // 物件指標
    int         iErrCode;       // 錯誤碼
} ERR_MSG;

static TList *HAlarmList = NULL;        // 系統錯誤串列
static TList *ShowAlarmList = NULL;     // 顯示錯誤串列
static TList *ClearAlarmList = NULL;    // 清除錯誤串列
static HMutex *ThMutex = NULL;
static bool   HALBusy = false;
//---------------------------------------------------------------------------
static void UpdateSystemNG();   // 更新系統錯誤狀態

//---------------------------------------------------------------------------
//  ALARM建構子
//  Parent:本Alarm的擁有物件
//---------------------------------------------------------------------------
HAlarm::HAlarm(TComponent *_Parent)
{
    ErrNoList = new TList;

    if (ThMutex == NULL)
        ThMutex = new HMutex;

    if (HAlarmList == NULL)
        HAlarmList = new TList;

    if (ShowAlarmList == NULL)
        ShowAlarmList = new TList;

    if (ClearAlarmList == NULL)
        ClearAlarmList = new TList;

    HAlarmList->Add(this);
    Parent = _Parent;
}

//---------------------------------------------------------------------------
//  解構子(清除配置區域)
//---------------------------------------------------------------------------
HAlarm::~HAlarm()
{
    // 先移除顯示串列上的相同ALARM .........................................
    for (int iP = 0;iP < ShowAlarmList->Count;iP ++)
    {  // 找顯示串列
        ERR_MSG *P = (ERR_MSG *) ShowAlarmList->Items[iP];
        if (P->ObjPtr == Parent)
        {
            ShowAlarmList->Delete(iP);
            iP = 0;
            delete P;
        }
    }

    // 再移除清除串列上的相同ALARM .........................................
    for(int iP=0; iP<ClearAlarmList->Count; iP++)
    {
        ERR_MSG *P = (ERR_MSG*) ClearAlarmList->Items[iP];
        if (P->ObjPtr == Parent)
        {
            ClearAlarmList->Delete(iP);
            delete P;
            iP = 0;
        }
    }

    // 從ALARM串列中移除
    for(int iP=0; iP<HAlarmList->Count; iP++)
    {
        if(HAlarmList->Items[iP]==this)
        {
            HAlarmList->Delete(iP);
            break;
        }
    }

    Clear();                            // 清掉錯誤碼串列
    delete ErrNoList;

    if(HAlarmList->Count==0)            // 如果己經沒有ALARM物作存在
    {
        delete HAlarmList;
        delete ShowAlarmList;
        delete ClearAlarmList;
        delete ThMutex;
        HAlarmList      = NULL;
        ShowAlarmList   = NULL;
        ClearAlarmList  = NULL;
        ThMutex         = NULL;
    }
}
//---------------------------------------------------------------------------
//  設定ALARM
//---------------------------------------------------------------------------
void HAlarm::Set(int iCode)
{
    if(GetStat(iCode))                  // 如果此一訊息己存在
        return ;

    try
    {
        while (HALBusy);
        HALBusy = true;
        ThMutex->Use();                 // 申請使用Mutex
        int *iECode = new int;
        *iECode = iCode;
        ErrNoList->Add(iECode);         // 在串列上加上錯誤碼

        ERR_MSG *Msg = new ERR_MSG;
        Msg->ObjPtr = Parent;           // 錯誤發生的物件指標
        Msg->iErrCode = iCode;          // 錯誤碼
        ShowAlarmList->Add(Msg);        // 在顯示的錯誤串列中加入訊息
        ThMutex->Release();             // Free Mutex
        HALBusy = false;
    }
    catch (...)
    {
        ShowMessage("無法配置系統錯誤訊息");
        abort();
    }
    UpdateSystemNG();
}

//---------------------------------------------------------------------------
//  清除ALARM
//---------------------------------------------------------------------------
bool HAlarm::Clear(int iECode)
{
    bool Flag=false;

    while(HALBusy);
    HALBusy=true;
    ThMutex->Use();                                 // 申請使用Mutex
    for(int iP=0; iP<ErrNoList->Count; iP++)
    {
        int *P=(int *)ErrNoList->Items[iP];
        if(*P==iECode)                              // 如果串列中的錯誤碼相同
        {
            ErrNoList->Delete(iP);                  // 刪除該節點
            Flag=true;
            break;
        }
    }
    if(!Flag)
    {
        HALBusy=false;
        ThMutex->Release();
        return false;
    }

    // 先移除顯示串列上的相同ALARM .........................................
    for(int iP=ShowAlarmList->Count-1; iP>=0; iP--)
    {
        ERR_MSG *P=(ERR_MSG *)ShowAlarmList->Items[iP];
        if(P->ObjPtr  ==Parent &&                   // 如果是本錯誤物件
           P->iErrCode==iECode)                     // 且錯誤編號相同
        {
            ShowAlarmList->Delete(iP);              // 將顯示串列中的移除
            delete P;
            break;
        }
    }

    // 將本ALARM加到清除串列上..........................................
    try
    {
        ERR_MSG *Msg =new ERR_MSG;
        Msg->ObjPtr  =Parent;        // 錯誤發生的物件指標
        Msg->iErrCode=iECode;        // 錯誤碼
        ClearAlarmList->Add(Msg);    // 在顯示清除的串列中加入訊息
    }
    catch (...)
    {
        ShowMessage("無法配置清除系統錯誤訊息");
        ThMutex->Release();          // Release Mutex
        abort();
    }

    ThMutex->Release();
    HALBusy = false;
    UpdateSystemNG();

    return true;
}
//---------------------------------------------------------------------------
//  清除所有ALARM
//---------------------------------------------------------------------------
void HAlarm::Clear()
{
    for(int iP=0; iP<ErrNoList->Count; iP=0)
    {
        int *P=(int *)ErrNoList->Items[iP];
        Clear(*P);
    }
}
//---------------------------------------------------------------------------
//  取得該ALARM編號錯誤是否存在
//---------------------------------------------------------------------------
bool HAlarm::GetStat(int iECode)
{
    ThMutex->Use();
    bool bResult=false;

    for(int iP=0; iP<ErrNoList->Count; iP++)
    {
        int *P=(int *)ErrNoList->Items[iP];
        if (*P==iECode)
        {
            bResult=true;
            break;
        }
    }
    ThMutex->Release();
    return bResult;
}
//---------------------------------------------------------------------------
//  取得該ALARM編號錯誤是否存在
//---------------------------------------------------------------------------
void ClearAllAlarm()
{
    for(int iP=0; iP<HAlarmList->Count; iP++)
    {
        HAlarm *P=(HAlarm *)HAlarmList->Items[iP];
        P->Clear();
    }
    SystemNG=false;
}
//---------------------------------------------------------------------------
//  更新系統錯誤狀態
//---------------------------------------------------------------------------
void UpdateSystemNG()
{
    bool Flag=false;

    for(int iP=0; iP<HAlarmList->Count; iP++)
    {
        HAlarm *P=(HAlarm *)HAlarmList->Items[iP];
        if(*P==true)                // 如果有機構上有錯誤碼
        {
            Flag=true;
            break;
        }
    }
    SystemNG=Flag;
}
//---------------------------------------------------------------------------
//  取出要顯示的錯誤訊息
//---------------------------------------------------------------------------
bool PopUpAlarm(TComponent **Component, int &iErrCode)
{
    if(ShowAlarmList->Count>0)
    {
        while(HALBusy);
        HALBusy = true;
        ThMutex->Use();                      // 申請使用Mutex
        ERR_MSG *P = (ERR_MSG *) ShowAlarmList->Items[0];

        *Component = P->ObjPtr;             // 指向錯誤物件
        iErrCode   = P->iErrCode;           // 傳回錯誤碼
        ShowAlarmList->Delete(0);
        delete P;                           // 將錯誤結構FREE
        ThMutex->Release();                  // Release Mutex
        HALBusy = false;
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
//  取出要清除顯示的錯誤訊息
//---------------------------------------------------------------------------
bool PopUpClrAlarm(TComponent **Component,int &iErrCode)
{
    if (ClearAlarmList->Count > 0)
    {
        while (HALBusy);
        HALBusy = true;
        ThMutex->Use();                      // Use Mutex
        ERR_MSG *P = (ERR_MSG *) ClearAlarmList->Items[0];

        *Component = P->ObjPtr;             // 指向錯誤物件
        iErrCode   = P->iErrCode;           // 傳回錯誤碼
        ClearAlarmList->Delete(0);
        delete P;                           // 將錯誤結構FREE
        ThMutex->Release();                  // Release Mutex
        HALBusy = false;
        return true;
    }

    return false;
}
//---------------------------------------------------------------------------
