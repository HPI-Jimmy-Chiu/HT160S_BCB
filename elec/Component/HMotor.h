//---------------------------------------------------------------------------
#ifndef HMotorH
#define HMotorH
//---------------------------------------------------------------------------
#include <SysUtils.hpp>
#include <Controls.hpp>
#include <Classes.hpp>
#include <Forms.hpp>
#include <stdio.h>
#include "halarm.h"

//---------------------------------------------------------------------------
class PACKAGE HMotor : public TComponent
{
private:
    AnsiString  sIOPort;
    void __fastcall SetIOPort(AnsiString s);
    void __fastcall SetSpeedProperty(unsigned int iSp) {
        Speed = iSp;
        SetSpeed(iSp);
    }
    bool    bEnable;


__published:
    __property  AnsiString IOPort =
        { read = sIOPort,write = SetIOPort };
    __property  unsigned int PRate =
        { read = Rate,write = SetRate,default = 110 };
    __property  int PHomeTask =
        { read = HomeTask,write = HomeTask,default = true };
    __property  unsigned int PInitSpeed =
        { read = InitSpeed,write = SetInitSpeed,default = 10 };
    __property  unsigned int PHomeLowSpeed =
        { read = HomeLowSpeed,write = HomeLowSpeed,default = 5 };
    __property  unsigned int PHomeHighSpeed =
        { read = HomeHighSpeed,write = HomeHighSpeed,default = 50 };
    __property  unsigned int PSpeed =
        { read = Speed,write = SetSpeedProperty };
    __property  unsigned int PSpeedNow =
        { read = SpeedNow,write = SpeedNow };
    __property  unsigned int PSpeed1 =
        { read = Speed1, write = Speed1 };
    __property  unsigned int PSpeed2 =
        { read = Speed2,write = Speed2 };
    __property  unsigned int PJogLowSpeed =
        { read = JogLowSpeed,write = JogLowSpeed,default = 10 };
    __property  unsigned int PJogHighSpeed =
        { read = JogHighSpeed,write = JogHighSpeed,default = 2000 };
    __property  int PPos =
        { read = Pos,write = SetPos,default = 0 };
    __property  int PHomePos =
        { read = HomePos,write = HomePos,default = 0 };
    __property  int PLastHomePos =
        { read = LastHomePos,write = LastHomePos };
    __property  int PSoftLimitP =
        { read = SoftLimitP,write = SoftLimitP,default = 999999 };
    __property  int PSoftLimitN =
        { read = SoftLimitN,write = SoftLimitN,default = -999999 };
    __property  bool PServoAlarmOn =
        { read = ServoAlarmOn,write = SetServoAlarmOn,default = false };
    __property  bool PInPosOn =
        { read = InPosOn,write = InPosOn,default = false };
    __property  bool PDirection =
        { read = Direction,write = Direction };
    __property  bool PHomeDirection =
        { read = HomeDirection,write = HomeDirection};
    __property  double  PGearRatio =
        { read = GearRatio,write = GearRatio,default = 1 };
    __property  int GoPosBase =
        { read = iGoPosBase,write = iGoPosBase,default = 100 };
    __property bool Enable =
        { read = bEnable,write = bEnable,default = true };

    //new
    __property  unsigned int PRange =
        { read = Range,write = SetRange,default = 25 };
    __property bool PSenType =
        { read = bSensorType,write = SetSensorType,default = true };

    // True :2P      False:1P
    __property bool PMotorType =
        { read = bMotorType,write = SetMotorType,default = true };

public:
    HAlarm  *Alarm;
    unsigned int Address,Rate;
    //new
    unsigned int Range;
    bool     bSensorType;
    bool     bMotorType;
    double  GearRatio;

    int HomeTask;
    unsigned int InitSpeed,HomeLowSpeed,HomeHighSpeed,
               Speed,SpeedNow,Speed1,Speed2,
               JogLowSpeed,JogHighSpeed;
    int  Pos,HomePos,LastHomePos,SoftLimitP,SoftLimitN;
    bool ServoAlarmOn,InPosOn,			//normal false
           Direction,HomeDirection;        	//normal true


public:
    __fastcall HMotor(TComponent* Owner);
    __fastcall ~HMotor();

	void __fastcall SetRate(unsigned int a);
	void __fastcall SetInitSpeed(unsigned int x);
	void __fastcall SetSpeed(unsigned int x);
	void __fastcall SetPos(int p);
    void __fastcall SetEnCoderPos(int p);
    void __fastcall SetServoAlarmOn(bool );

    void __fastcall SetRange(unsigned int a);
    void __fastcall SetSensorType(bool  a);
    void __fastcall SetMotorType(bool  a);
    int ReadEnCoderPos();
    int     ReadeEnCoderRealPos(void);
  	int InitMotor(int IoAddress);		// 起始設定函數
	bool Busy(void);
	bool GetAlarm(void);
    bool GetAlarm_2(void);
	bool HomeFlag(void);
	bool LimitPFlag(void);
	bool LimitNFlag(void);
	bool RealG00(int p);
    bool RealG00_2(int p);
	void Pluse(int p);
	void HomeReset(void);
	bool Home(void);
	bool StopByHome(void);
	bool StopByLimitP(void);
	bool StopByLimitN(void);
	unsigned int ReadRate(void);
	unsigned int ReadInitSpeed(void);
	unsigned int ReadSpeed(void);
	unsigned int ReadRealSpeed(void);
    bool JogP(void);
	bool JogN(void);
    bool JogLowSpeedP(void);
	bool JogLowSpeedN(void);
	bool HighSpeedJogP(void);
	bool HighSpeedJogN(void);
    void    ChangeGearRatio(double Gear);
	void Stop(void);
	void SetSoftLimit(int lp,int ln);
	int     ReadRealPos(void);
    void    GetAlarmStatus(BYTE *ucD2,BYTE *ucD4);
    bool    GoPos(int iPos,double dbRatio = 1.0);   // 移動點位
    void    SetWorkPos(int iPos,int iPitch);
    int     ReadPos();                          // 讀取馬達位置(1/100mm)
    bool    G00(int iPos);                      // 移動點位
    bool    G00_2(int iPos);                      // 移動點位
    int     AddSpecialPos(int iPos);            // 加上一個特殊點位
    void    ClearSpecialPos();                  // 清除所有特殊點位
    int     TargetPosition;

private:
    int     iWorkPos;
    int     iPitch;
//    double  GearRatio;
    int    *iPosArray;                  // 特殊點位陣列
    int     iPosArraySize;              // 特殊點位陣列尺寸
    int     iPosArrayQuan;              // 特殊點位數量
    int     iGoPosBase;                 // 特殊點位起點

};

//---------------------------------------------------------------------------
#define     ALM_MOTOR_MOVE  55555
#endif
