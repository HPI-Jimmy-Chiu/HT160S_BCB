//---------------------------------------------------------------------------
//  吸嘴物件
//---------------------------------------------------------------------------
#ifndef HSuckerH
#define HSuckerH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include "iobyte.h"
#include <stdio.h>
#include "hSensor.h"
#include "hswitcher.h"
#include "hsensor.h"
#include "halarm.h"
#include "htimer.h"

//---------------------------------------------------------------------------
#define     OP_UNLOAD   100         // UNLOAD作業
#define     OP_OK       101         // 完成
#define     OP_MARK     102         // 使用
#define     OP_LOAD     103         // 己吸取IC

// 錯誤碼
#define     ALM_SUCKER_DOWN         100     // 無法移到下
#define     ALM_SUCKER_UP           101     // 無法移到上
#define     ALM_SUCKER_VACCUM_ON    102     // 沒吸到IC
#define     ALM_SUCKER_VACCUM_OFF   103     // 放不掉IC


//---------------------------------------------------------------------------
extern bool GetAndBitNumber(char *,int &,int &,bool &);

//---------------------------------------------------------------------------
class PACKAGE HSucker : public TComponent
{
private:
    HTimer  ChkTime;            // 設備計時器
    HTimer  MoveTime;           // 移動用計時器
    HTimer  VaccumOnDelayTim;   //
 //   HTimer  DestoryTime;         //真空破壞時間
__published:
    __property Classes::TNotifyEvent AOnCreate =
        { read=aOnCreate, write=aOnCreate};
    __property  AnsiString  MoveIOPort =
        { read = sMovePort,write = PSetMovePort };
    __property  AnsiString  VaccumIOPort =
        { read = sVaccumPort,write = PSetVaccumPort };
    __property  AnsiString  IsMoveUpIOPort =
        { read = sIsMoveUpPort,write = PSetIsMoveUpPort };
    __property  AnsiString  IsMoveDownIOPort =
        { read = sIsMoveDownPort,write = PSetIsMoveDownPort };
    __property  AnsiString  IsVaccumOnIOPort =
        { read = sIsVaccumOnPort,write = PSetIsVaccumOnPort };
    __property  AnsiString  DestroyPort =
        { read = sDestroyPort,write = PSetDestroyPort };
    __property  int MoveTimes =
        { read = iMoveTimes,write = iMoveTimes };
    __property  int SuckTimes =
        { read = iSuckTimes,write = iSuckTimes };
//    __property  int DestoryTimes =
//        { read = iDestoryTimes,write = iDestoryTimes ,default = 5};
//    __property  int VaccumOnDelay =
//        { read = iVaccumOnDelay,write = iVaccumOnDelay };

private:    // 設定PROPERTIES用的函數
    AnsiString  sMovePort;
    AnsiString  sVaccumPort;
    AnsiString  sIsMoveUpPort;
    AnsiString  sIsMoveDownPort;
    AnsiString  sIsVaccumOnPort;
    AnsiString  sDestroyPort;
    int         iMoveTimes;
    int         iSuckTimes;
//    int         iDestoryTimes;        //    真空破壞時間

    __fastcall void PSetMovePort(AnsiString s);
    __fastcall void PSetVaccumPort(AnsiString s);
    __fastcall void PSetIsMoveUpPort(AnsiString s);
    __fastcall void PSetIsMoveDownPort(AnsiString s);
    __fastcall void PSetIsVaccumOnPort(AnsiString s);
    __fastcall void PSetDestroyPort(AnsiString s);

protected:
	Classes::TNotifyEvent aOnCreate;

public:
    HAlarm  *Alarm;              // 錯誤處理
    HSwitcher   *Move;              // 移動位置PORT
    HSwitcher   *Vaccum;            // 真空開關PORT
    HSwitcher   *Destroy;           // 真空破壞PORT
    HSensor   *ChkIsMoveUp;         // CHECK移動位置"上"PORT
    HSensor   *ChkIsMoveDown;       // CHECK移動位置"下"PORT
    HSensor   *ChkIsVaccumOn;       // CHECK是否真空狀況PORT
    int         OpStat;             // 操作狀態(MARK,LOAD ...)
    int         iVaccumOnDelay;     // 吸取時的DELAY

    __fastcall HSucker(TComponent* Owner);
    __fastcall virtual ~HSucker();
    void    SetPos(int pos);
    int     GetPos();

    bool    IsMoveUp();
	bool    IsMoveDown();
    bool    IsVaccumOn();
    bool    IsVaccumOff();

    void    Reset();                    // 重置計時器相關資訊
    bool    MoveDown();                 //吸嘴移動向下
    bool    MoveUp();                   //吸嘴移動向上
    bool    VaccumOn();                 //開始吸氣
    bool    VaccumOff();                //停止吸氣

};

//---------------------------------------------------------------------------
extern bool    SystemICOverride;        // 是否 IC 忽視
#endif
