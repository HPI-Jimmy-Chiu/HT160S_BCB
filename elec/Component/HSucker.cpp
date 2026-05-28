//---------------------------------------------------------------------------
//  吸嘴物件
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "HSucker.h"
#pragma package(smart_init)

bool    SystemICOverride = false;       // 是否 IC 忽視

//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//

static inline void ValidCtrCheck(HSucker *)
{
    new HSucker(NULL);
}

//---------------------------------------------------------------------------
//  建構子
//---------------------------------------------------------------------------
__fastcall HSucker::HSucker(TComponent* Owner)
    : TComponent(Owner)
{
    OpStat     = NULL;          // 操作狀態

    Move       = NULL;          // 移動位置PORT
    Vaccum     = NULL;          // 真空開關PORT
    ChkIsMoveUp   = NULL;       // CHECK移動位置"上"PORT
    ChkIsMoveDown = NULL;       // CHECK移動位置"下"PORT
    ChkIsVaccumOn = NULL;       // CHECK是否真空狀況PORT
    Destroy       = NULL;       // 真空破壞PORT
    iMoveTimes    = 5;          // 內定移動最長時間不得大於50ms
    iSuckTimes    = 10;         // 內定吸取最長時間不得大於50ms
   
    iVaccumOnDelay = 0;
    Alarm = new HAlarm(this);
}

//---------------------------------------------------------------------------
//  解構子
//---------------------------------------------------------------------------
__fastcall  HSucker::~HSucker()
{
    delete Move;
    delete Vaccum;
    delete ChkIsMoveUp;
    delete ChkIsMoveDown;
    delete ChkIsVaccumOn;
    delete Destroy;
}

//---------------------------------------------------------------------------
//  檢查現在是否在"上"
//---------------------------------------------------------------------------
bool  HSucker::IsMoveUp()
{
    if (ChkIsMoveUp == NULL) {      // 如果沒有下死點SENSOR
        MoveTime.Set(iMoveTimes);   // 以計時方式進行
        MoveTime.On();
        if (MoveTime.Off())
            return true;
        else
            return false;
    }
    return ChkIsMoveUp->Stat();
}

//---------------------------------------------------------------------------
//  檢查現在是否在"下"
//---------------------------------------------------------------------------
bool  HSucker::IsMoveDown()
{
    if (ChkIsMoveDown == NULL) {    // 如果沒有下死點SENSOR
        MoveTime.Set(iMoveTimes);   // 以計時方式進行
        MoveTime.On();
        if (MoveTime.Off())
            return true;
        else
            return false;
    }
    return ChkIsMoveDown->Stat();
}

//---------------------------------------------------------------------------
//  檢查現在是否真空ON(吸到IC)
//---------------------------------------------------------------------------
bool  HSucker::IsVaccumOn()
{
    return ChkIsVaccumOn->Stat();
}

//---------------------------------------------------------------------------
//  檢查現在是否真空OFF(沒吸到IC)
//---------------------------------------------------------------------------
bool  HSucker::IsVaccumOff()
{
    return !ChkIsVaccumOn->Stat();
}

//---------------------------------------------------------------------------
//  讀取IO Port設定字串,分寫到Port及Bit數值變數中
//---------------------------------------------------------------------------
bool GetPortAndBitNumber(char *cStr,int &iPort,int &iBit,bool &Read)
{
    sscanf(cStr,"%x,%x,&d",&iPort,&iBit,&Read);

    bool Flag = false;
    if (iPort == 0)
        Flag = true;

    if (iBit != 0x01 &&
        iBit != 0x02 &&
        iBit != 0x04 &&
        iBit != 0x08 &&
        iBit != 0x10 &&
        iBit != 0x20 &&
        iBit != 0x40 &&
        iBit != 0x80)
        Flag = true;    // 如果不是只有一個Bit On

    if (Flag) {
        Application->MessageBox(
                "參數值請以16進位輸入:\n\nPORT編號,BIT編號,是否讀回Port狀態\n如:0x240,0x8,1\n\n如果IO卡為可讀回者,讀回Port狀態才可設為1",
                "參數錯誤",
                MB_ICONINFORMATION | MB_OK);
        return false;
    }
    return true;
}

//---------------------------------------------------------------------------
//  Properties中設定Move用的函數
//---------------------------------------------------------------------------
void __fastcall HSucker::PSetMovePort(AnsiString s)
{
    if (Move != NULL)
        delete Move;

    Move = new HSwitcher(this);
    Move->IOPort = s;
    sMovePort = s;

    if (aOnCreate != NULL)
        aOnCreate(this);
}

//---------------------------------------------------------------------------
//  Properties中設定VaccumPort用的函數
//---------------------------------------------------------------------------
void __fastcall HSucker::PSetVaccumPort(AnsiString s)
{
    if (Vaccum != NULL)
        delete Vaccum;

    Vaccum = new HSwitcher(this);
    Vaccum->IOPort = s;
    sVaccumPort = s;

    if (aOnCreate != NULL)
        aOnCreate(this);
}

//---------------------------------------------------------------------------
//  Properties中設定IsMoveUpPort用的函數
//---------------------------------------------------------------------------
void __fastcall HSucker::PSetIsMoveUpPort(AnsiString s)
{
    if (ChkIsMoveUp != NULL)
        delete ChkIsMoveUp;

    ChkIsMoveUp = new HSensor(this);
    ChkIsMoveUp->IOPort = s;
    sIsMoveUpPort = s;

    if (aOnCreate)
        aOnCreate(this);
}

//---------------------------------------------------------------------------
//  Properties中設定IsMoveDownPort用的函數
//---------------------------------------------------------------------------
void __fastcall HSucker::PSetIsMoveDownPort(AnsiString s)
{
    if (ChkIsMoveDown != NULL)
        delete ChkIsMoveDown;

    ChkIsMoveDown = new HSensor(this);
    ChkIsMoveDown->IOPort = s;
    sIsMoveDownPort = s;

    if (aOnCreate)
        aOnCreate(this);
}

//---------------------------------------------------------------------------
//  Properties中設定IsVaccumOnPort用的函數
//---------------------------------------------------------------------------
void __fastcall HSucker::PSetIsVaccumOnPort(AnsiString s)
{
    if (ChkIsVaccumOn != NULL)
        delete ChkIsVaccumOn;

    ChkIsVaccumOn = new HSensor(this);
    ChkIsVaccumOn->IOPort = s;
    sIsVaccumOnPort = s;

    if (aOnCreate)
        aOnCreate(this);
}

//---------------------------------------------------------------------------
//  Properties中設定DestroyPort用的函數
//---------------------------------------------------------------------------
void __fastcall HSucker::PSetDestroyPort(AnsiString s)
{
    if (DestroyPort != NULL)
        delete Destroy;

    Destroy = new HSwitcher(this);
    Destroy->IOPort = s;
    sDestroyPort = s;

    if (aOnCreate)
        aOnCreate(this);
}

//---------------------------------------------------------------------------
//  重置計時器等相關資訊
//---------------------------------------------------------------------------
void HSucker::Reset()
{
    ChkTime.Clear();
}

//---------------------------------------------------------------------------
//  吸嘴移動向下
//---------------------------------------------------------------------------
bool  HSucker::MoveDown()
{
    *Move = ON;                         // 下移

    if (IsMoveDown()) {                 // 如果要上移且己到頂
        ChkTime.Clear();                // 清除計時器
        Alarm->Clear(ALM_SUCKER_DOWN);   // 清除錯誤訊息
        return true;
    }

    if (!Alarm->GetStat(ALM_SUCKER_DOWN)) {  // 如果之前不是下移錯誤
        ChkTime.Set(iMoveTimes + 1);         // 設定計時長度
        ChkTime.On();                        // 打開計時器
        if (ChkTime.Off())                   // 如果時間到
            Alarm->Set(ALM_SUCKER_DOWN);     // 設定ALARM
    }

    return false;
}

//---------------------------------------------------------------------------
//  吸嘴移動向上
//---------------------------------------------------------------------------
bool  HSucker::MoveUp()
{
    *Move = OFF;                    // 移動向上

    if (IsMoveUp()) {               // 如果要下移且己到
        ChkTime.Clear();
        Alarm->Clear(ALM_SUCKER_UP);
        return true;
    }

    if (!Alarm->GetStat(ALM_SUCKER_UP)) {    // 如果之前不是上移失敗
        ChkTime.Set(iMoveTimes + 1);         // 設定計時長度
        ChkTime.On();                   // 打開計時器
        if (ChkTime.Off())              // 如果時間到
            Alarm->Set(ALM_SUCKER_UP);   // 設定ALARM
    }

    return false;
}

//---------------------------------------------------------------------------
//  吸取並計時
//---------------------------------------------------------------------------
bool HSucker::VaccumOn()
{
    if (Destroy != NULL)                // 如果有真空破壞
        *Destroy = OFF;                 // 關閉真空破壞

    *Vaccum = ON;                       // 開真空


    if (IsVaccumOn() ||                     // 如果己吸到IC
        SystemICOverride) {                 // 或者IC忽視
//        VaccumOnDelayTim.Set(iVaccumOnDelay);
//        VaccumOnDelayTim.On();
//        if (VaccumOnDelayTim.Off()) {
            ChkTime.Clear();
            Alarm->Clear(ALM_SUCKER_VACCUM_ON);  // 清除ALARM
            OpStat = OP_LOAD;                   // 設定FLAG(IC吸起)
            return true;
//        }

//        else
//            return false;
    }
    else {
//    if (!Alarm->GetStat(ALM_SUCKER_VACCUM_ON)) { // 如果之前不是吸取失敗
        ChkTime.Set(iSuckTimes);                // 設定計時長度
        ChkTime.On();                           // 打開計時器
        if (ChkTime.Off())                      // 如果時間到
            Alarm->Set(ALM_SUCKER_VACCUM_ON);    // 設定ALARM
    }
    return false;
}

//---------------------------------------------------------------------------
//  放開並計時
//---------------------------------------------------------------------------
bool HSucker::VaccumOff()
{
    *Vaccum = OFF;                      // 關真空
    if (Destroy != NULL)                // 如果有真空破壞
        *Destroy = ON;                  // 打開真空破壞



    if (IsVaccumOff()) {                // 如果放開IC
      //  DestoryTime.Set(DestoryTimes);                    //增加DestoryTime
      //  DestoryTime.On();                                  //
     //   if (DestoryTime.Off()){                            //
            ChkTime.Clear();

            if (Destroy != NULL)            // 如果有真空破壞
               *Destroy = OFF;             // 關閉真空破壞

        Alarm->Clear(ALM_SUCKER_VACCUM_OFF);    // 成功放開,清除ALARM
        OpStat = NULL;                  // 清除使用FLAG(因為IC己放開)
       //  DestoryTime.Clear();
        return true;

      }

//    if (!Alarm->GetStat(ALM_SUCKER_VACCUM_OFF)) {    // 如果之前不是放失敗
    else {
        ChkTime.Set(iSuckTimes);                    // 設定計時長度
        ChkTime.On();                               // 打開計時器
        if (ChkTime.Off())                          // 如果時間到
            Alarm->Set(ALM_SUCKER_VACCUM_OFF);       // 設定ALARM
    }

    return false;
}

//---------------------------------------------------------------------------
namespace Hsucker
{
    void __fastcall PACKAGE Register()
    {
        TComponentClass classes[1] = {__classid(HSucker)};
        RegisterComponents("HungKai", classes, 0);
    }
}

