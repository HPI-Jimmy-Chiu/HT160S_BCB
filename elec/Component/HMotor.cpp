//---------------------------------------------------------------------------
//              馬達控制類別庫
//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "HMotor.h"
#include "iobyte.h"
#include <assert.h>

#pragma package(smart_init)
//#define m_range (50)

//---------------------------------------------------------------------------
// ValidCtrCheck is used to assure that the components created do not have
// any pure virtual functions.
//
static inline void ValidCtrCheck(HMotor *)
{
    new HMotor(NULL);
}

//---------------------------------------------------------------------------
__fastcall HMotor::HMotor(TComponent* Owner)
    : TComponent(Owner)
{
    bEnable = true;
    bMotorType=true;
    iPosArray = NULL;
    iPosArrayQuan = 0;
    iPosArraySize = 0;
    iGoPosBase    = 100;
    Speed = 1000;

    ServoAlarmOn=false;
    InPosOn=false;
    InitSpeed=10;
    HomeLowSpeed=5;
    HomeHighSpeed=50;
    Speed=1000;
    Rate=110;
    HomeTask=1;
    JogHighSpeed=2000;
    JogLowSpeed=10;
    Pos=0;
    HomePos=0;
    LastHomePos=0;
    SoftLimitP=999999;
    SoftLimitN=-999999;
    bSensorType=true;
    InitMotor(0x340);
    iWorkPos = 0;
    iPitch = 0;
    GearRatio = 1;
    Range=25;
    Alarm = new HAlarm(this);
}

// ------------------------------------------------------------------------
//
// ------------------------------------------------------------------------
__fastcall HMotor::~HMotor()
{
    delete iPosArray;
    delete Alarm;
}

// ------------------------------------------------------------------------
//  設定IO PORT參數
// ------------------------------------------------------------------------
void __fastcall HMotor::SetIOPort(AnsiString s)
{
    int iPort;

    s.Trim();
    if (s.Length() == 0) {
        iPort = 0;
        sIOPort = s;
        Address = iPort;
        return ;
    }
    else
        sscanf(s.c_str(),"%x",&iPort);

    if (iPort != 0) {
        sIOPort = s;
        Address = iPort;
        InitMotor(iPort);
    }
    else {
        Application->MessageBox("請以16進位輸入I/O PORT編號\n\n如:0x240",
                                "參數錯誤",
                                MB_ICONINFORMATION | MB_OK);
    }
}
//-------------------------------------------------------------------------
void HMotor::ChangeGearRatio(double Gear)
{
    GearRatio = Gear;
}
//---------------------------------
int HMotor::ReadeEnCoderRealPos(void)
{
    int p,d5,d6,d7;
    outportb(Address,0x41);
    d5=inportb(Address+5);
    d6=inportb(Address+6);
    d7=inportb(Address+7);
    p=d5+(d6<<8)+(d7<<16);
    if(d7&0x80) p+=0xff000000;
    if(!Direction) p=-p;
    return(p);
}
//-------------------------------------------------------------------------
int HMotor::ReadEnCoderPos()
{
    return ReadeEnCoderRealPos() * GearRatio;
}
// ------------------------------------------------------------------------
//	起始設定函數
// ------------------------------------------------------------------------
int HMotor::InitMotor(int IoAddress)
{
    if (!bEnable)
        return true;

    if (IoAddress == 0)
        return false;

    InitNTPort();                           // Initial NT 上的I/O Port
    EnableNTPort(IoAddress,IoAddress + 8);  // Enable 馬達所用的Port
/*
    BYTE D5=0x1b,D6=0,D7=1;                 //D5=0x1b Limit Normal close

    if( bMotorType )
        D6=0;
    else
        D6=1;

    if(bSensorType==false)
        D5=0x03;


    Address=IoAddress;
    inportb(Address+2);                     //000 11 011
	outportb(Address,5);                    //000 00 011
    if(InPosOn) D7|=0x08;                   //設InPos ServoAlarm 有效
    if(ServoAlarmOn) D7|=0x02;
    outportb(Address+5,D5);
    outportb(Address+6,D6);
    outportb(Address+7,D7);
    outportb(Address,0x20);            //set mode

	outportb(Address+1,0);

    //if(Address!=0x4310)outportb(Address+5,m_range);
    //else outportb(Address+5,m_range*5);

    if(Address!=0x4310)outportb(Address+5,Range);
    else outportb(Address+5,Range);


	outportb(Address+6,0);
	outportb(Address,0x21);                //set range
*/
    SetPos(Pos);
    SetSoftLimit(999999,-999999);
    SetInitSpeed(InitSpeed);
    SetSpeed(Speed);
    SetRate(Rate);
/*
	if(inportb(Address+2)&3) return(false);
	else return(true);
*/
    return true;
}

// ------------------------------------------------------------------------
//	是否Busy
// ------------------------------------------------------------------------
bool HMotor::Busy(void){
    if(inportb(Address)&0x20)return(true);
    else return(false);
}

// ------------------------------------------------------------------------
// 	是否Alarm
// ------------------------------------------------------------------------
bool HMotor::GetAlarm(void){

    int D2;
    D2=inportb(Address+2);
    if(!ServoAlarmOn) D2&=0xbf;


//    if( bSensorType==false)  // limit inverter
//        D2=D2^0x30; //CW is Bit 4  CCW is Bit 5 do XOR operator

    if(D2)
        return(true);
    else
        return(false);


//    IOSET_System_Led[p  ]->Value=D2&0x10;   	 //CWL
//    IOSET_System_Led[p+1]->Value=D4&2;		    //Home
//    IOSET_System_Led[p+2]->Value=D2&0x20;		// CCWL

}
bool HMotor::GetAlarm_2(void){

    int D2;
    D2=inportb(Address+2);
    if(!ServoAlarmOn) D2&=0xbf;
    D2&=0xfd;

//    if( bSensorType==false)  // limit inverter
//        D2=D2^0x30; //CW is Bit 4  CCW is Bit 5 do XOR operator

    if(D2)
        return(true);
    else
        return(false);


//    IOSET_System_Led[p  ]->Value=D2&0x10;   	 //CWL
//    IOSET_System_Led[p+1]->Value=D4&2;		    //Home
//    IOSET_System_Led[p+2]->Value=D2&0x20;		// CCWL

}


// ------------------------------------------------------------------------
// 	是否在Home
// ------------------------------------------------------------------------
bool HMotor::HomeFlag(void)
{
    if(bSensorType)
    {
        if((inportb(Address+4)&2))return(true);
        else return(false);
    }
    else
    {
        if((inportb(Address+4)&2))return(false);
        else return(true);
    }
//
//        if((inportb(Address+4)&2))return(true);
//        else return(false);

}

// ------------------------------------------------------------------------
//	是否在邊界
// ------------------------------------------------------------------------
/*
bool HMotor::LimitPFlag(void)
{
    bool Flag;
    if(Direction)
    {
        if((inportb(Address+2)&0x10))
            Flag=true;
            return(true);
        else return(false);
    }
    else{
        if((inportb(Address+2)&0x20))return(true);
        else return(false);
    }
}
*/
bool HMotor::LimitPFlag(void)
{
    bool Flag;
    if(Direction)
    {
        if((inportb(Address+2)&0x10))
            Flag=true;
        else
            Flag=false;
    }
    else{
        if((inportb(Address+2)&0x20))
            Flag=true;
        else
            Flag=false;
    }

    /*
    if(bSensorType)
        return Flag;
    else
        return !Flag;
    */
    return Flag;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
/*
bool HMotor::LimitNFlag(void){
    if(Direction){
      if((inportb(Address+2)&0x20))return(true);
      else return(false);
    }
    else{
      if((inportb(Address+2)&0x10))return(true);
      else return(false);
    }
}
*/
bool HMotor::LimitNFlag(void){
    bool Flag;
    if(Direction){
      if((inportb(Address+2)&0x20))
        Flag=true;
      else
        Flag=false;
    }
    else{
      if((inportb(Address+2)&0x10))
        Flag=true;
      else
        Flag=false;
    }
    /*
    if(bSensorType)
        return Flag;
    else
        return !Flag;
    */
    return Flag;
}

// ------------------------------------------------------------------------
//	快速移動
// ------------------------------------------------------------------------
bool HMotor::RealG00(int p)
{
    int a;
    if(Busy()) return(false);
    a=p-ReadRealPos();
    if(a==0) return(true);
    if(Direction){
      if(a>0){ Pluse(a); outportb(Address,0);}
      else{ Pluse(-a); outportb(Address,1);}
    }
    else{
      if(a>0){ Pluse(a); outportb(Address,1);}
      else{ Pluse(-a); outportb(Address,0);}
    }

    if (GetAlarm())
        Alarm->Set(ALM_MOTOR_MOVE);             // 無法移動到定位

    return(false);
}
bool HMotor::RealG00_2(int p)
{
    int a;
    a=p-ReadRealPos();
    if(a==0) return(true);
    if(Direction){
      if(a>0){ Pluse(a); outportb(Address,0);}
      else{ Pluse(-a); outportb(Address,1);}
    }
    else{
      if(a>0){ Pluse(a); outportb(Address,1);}
      else{ Pluse(-a); outportb(Address,0);}
    }

    if (GetAlarm_2())
        Alarm->Set(ALM_MOTOR_MOVE);             // 無法移動到定位

    return(false);
}



// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
void HMotor::Pluse(int p)
{
    outportb(Address+5,(BYTE) p);
    outportb(Address+6,BYTE(p>>8));
    outportb(Address+7,BYTE(p>>16));
    outportb(Address,0x25);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
void HMotor::HomeReset(void){
    HomeTask=1;
}

// ------------------------------------------------------------------------
//	移到開頭
// ------------------------------------------------------------------------
bool HMotor::Home(void)
{
    static unsigned int OldSpeed;
    switch(HomeTask){
	case 1:
      OldSpeed=Speed;
      Stop();
      if(!Busy()){
        InitMotor(Address);
        SetSpeed(HomeHighSpeed);
        SetSoftLimit(999999,-999999);
        if(bSensorType)
            outportb(Address+1,12);
        else
            outportb(Address+1,8);

        if(!HomeFlag()){
          if(HomeDirection){
               if(JogN()) HomeTask++;
          }
          else{
            if(JogP()) HomeTask++;
          }
        }
        else HomeTask++;
      }

	  break;

	case 2:
      if(!Busy()){
	    if(StopByHome()||HomeFlag()) HomeTask++;
        else{
          if(StopByLimitN()) JogP();
          else if(StopByLimitP()) JogN();
          else if(HomeDirection) JogN();
          else JogP();
        }
      }
	  break;
	case 3:
      if(!Busy()){
        if(bSensorType)
            outportb(Address+1,8);
        else
            outportb(Address+1,12);

        if(HomeDirection){
          JogP();
          HomeTask++;
        }
        else{
          JogN();
          HomeTask++;
        }
	  }
	  break;
	case 4:
	  if(!Busy()){
        Pluse(25);
        SetSpeed(HomeLowSpeed);
        if(Direction){
          if(HomeDirection) outportb(Address,0);
          else outportb(Address,1);
          HomeTask++;
        }
        else{
          if(HomeDirection) outportb(Address,1);
          else outportb(Address,0);
          HomeTask++;
        }
      }
      break;
	case 5:
      if(!Busy()){
        SetSpeed(HomeLowSpeed);
        if(bSensorType)
            outportb(Address+1,12);        //遇home停止
        else
            outportb(Address+1,8);        //遇home停止
        if(HomeDirection){ JogN(); HomeTask++;}
        else{ JogP(); HomeTask++; }
      }
  	  break;
    case 6:
	  if(!Busy()){
	    if(StopByHome()||HomeFlag()) HomeTask++;
        else HomeTask=1;
      }
  	  break;
    case 7:
      LastHomePos=ReadRealPos();
      SetPos(HomePos);
      outportb(Address+1,0);
      //outportb(Address+1,12);
     //dd SetSoftLimit(SoftLimitP,SoftLimitN);
      SetSoftLimit(SoftLimitP/GearRatio,SoftLimitN/GearRatio);
      SetSpeed(OldSpeed);
	  HomeTask++;
	  break;
	case 8:
	  break;
    default:
      break;
   }
    if(HomeTask==8) return(true);
    else return(false);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
bool HMotor::StopByHome(void)
{
/*
    if(bSensorType)
    {
        if(inportb(Address+3)&2) return(true);
        else return(false);
    }
    else
    {
        if(inportb(Address+3)&2) return(false);
        else return(true);
    }
*/
        if(inportb(Address+3)&2) return(true);
        else return(false);

}
// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
bool HMotor::StopByLimitP(void)
{
    bool Flag;
    if(Direction){
        if(inportb(Address+3)&0x10)
            Flag=true;
        else
            Flag=false;
    }
    else{
        if(inportb(Address+3)&0x20)
            Flag=true;
        else
            Flag=false;
    }
    //if(bSensorType)
    //    return Flag;
    //else return !Flag;
    return Flag;
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
bool HMotor::StopByLimitN(void)
{
    bool Flag;
    if(Direction){
        if(inportb(Address+3)&0x20)
            Flag=true;
        else
            Flag=false;
    }
    else{
        if(inportb(Address+3)&0x10)
            Flag=true;
        else
            Flag=false;
    }
    //if(bSensorType)
    //    return Flag;
    //else return !Flag;
    return Flag;

}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
void __fastcall HMotor::SetPos(int p)
{
/*
    outportb(Address+5,(BYTE) p);
    outportb(Address+6,BYTE(p>>8));
    outportb(Address+7,BYTE(p>>16));
    outportb(Address,0x26);
*/    
}

// ------------------------------------------------------------------------
void __fastcall HMotor::SetEnCoderPos(int p)
{
/*
    outportb(Address+5,(BYTE) p);
    outportb(Address+6,BYTE(p>>8));
    outportb(Address+7,BYTE(p>>16));
    outportb(Address,0x27);
*/    
}
// ------------------------------------------------------------------------
void __fastcall HMotor::SetRate(unsigned int a)            //pps
{
     //rate=(un int)((4000000l/a)*m_range);
     Rate=a;
     if(Rate>65353) Rate=65353;
     if(Rate==0) Rate=1;
/*
     outportb(Address+5,(BYTE) Rate);
     outportb(Address+6,BYTE(Rate>>8));
     outportb(Address,0x22);
*/     
}
void __fastcall HMotor::SetRange(unsigned int a)            //pps
{
     //rate=(un int)((4000000l/a)*m_range);
    Range=a;
/*
    if(Address!=0x4310)outportb(Address+5,Range);
    else outportb(Address+5,Range);
*/
}
void __fastcall HMotor::SetSensorType(bool type)            //pps
{
    bSensorType=type;
//    InitMotor(Address);
}
void __fastcall HMotor::SetMotorType(bool type)            //pps
{
    bMotorType=type;
//    InitMotor(Address);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
unsigned int HMotor::ReadRate(void)
{
    //cprintf("rate===%u",rate);
    return(Rate);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
void __fastcall HMotor::SetInitSpeed(unsigned int x)
{
    InitSpeed=x;
    if(InitSpeed>8191) InitSpeed=8191;
/*
    outportb(Address+5,(BYTE) InitSpeed);
    outportb(Address+6,BYTE(InitSpeed>>8));
    outportb(Address,0x23);
*/    
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
unsigned int HMotor::ReadInitSpeed(void)
{
    return(InitSpeed);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
void __fastcall HMotor::SetSpeed(unsigned int x)
{
    if(x>8191) x=8191;
    if(x==0) x=1;
    Speed=x;
/*
    outportb(Address+5,(BYTE) x);
    outportb(Address+6,BYTE(x>>8));
    outportb(Address,0x24);
*/    
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
unsigned int HMotor::ReadSpeed(void)
{
    return(Speed);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
unsigned int HMotor::ReadRealSpeed(void)
{
    unsigned int D5,D6;
    outportb(Address,0x44);
    D5=inportb(Address+5);
    D6=inportb(Address+6);
    return(D5|(D6<<8));
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
bool HMotor::JogP(void)
{
    if(Busy()) return(false);
    if(Direction) outportb(Address,2);
    else outportb(Address,3);
    return(true);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
bool HMotor::JogN(void)
{
    if(Busy()) return(false);
    if(Direction) outportb(Address,3);
    else outportb(Address,2);
    return(true);
}


// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
bool HMotor::JogLowSpeedP(void)
{
    if(Busy()) return(false);
    SetSpeed(JogLowSpeed);
    if(Direction) outportb(Address,2);
    else outportb(Address,3);
    return(true);
}


// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
bool HMotor::JogLowSpeedN(void)
{
    if(Busy()) return(false);
    SetSpeed(JogLowSpeed);
    if(Direction) outportb(Address,3);
    else outportb(Address,2);
    return(true);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
bool HMotor::HighSpeedJogP(void)
{
    if(Busy()) return(false);
    SetSpeed(JogHighSpeed);
    if(Direction) outportb(Address,2);
    else outportb(Address,3);
    return(true);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
bool HMotor::HighSpeedJogN(void)
{
    if(Busy()) return(false);
    SetSpeed(JogHighSpeed);
    if(Direction) outportb(Address,3);
    else outportb(Address,2);
    return(true);
}


// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
void HMotor::Stop(void)
{
    outportb(Address,4);
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
void HMotor::SetSoftLimit(int lp,int ln)
{
  int LP,LN;

  if (Direction) {
	LP = lp;
    LN = ln;
  }
 else {
 	LP = -ln;
   	LN = -lp;
 }
/*
  outportb(Address+5,(BYTE)LP);
  outportb(Address+6,BYTE(LP>>8));
  outportb(Address+7,BYTE(LP>>16));
  outportb(Address,0x29);
  outportb(Address+5,(BYTE)LN);
  outportb(Address+6,BYTE(LN>>8));
  outportb(Address+7,BYTE(LN>>16));
  outportb(Address,0x2a);
*/  
}

// ------------------------------------------------------------------------
// ------------------------------------------------------------------------
int HMotor::ReadRealPos(void)
{
    int p,d5,d6,d7;
    outportb(Address,0x40);
    d5=inportb(Address+5);
    d6=inportb(Address+6);
    d7=inportb(Address+7);
    p=d5+(d6<<8)+(d7<<16);
    if(d7&0x80) p+=0xff000000;
    if(!Direction) p=-p;
    return(p);
}
// ------------------------------------------------------------------------
//	Get Motor Alarm Byte
// ------------------------------------------------------------------------
void HMotor::GetAlarmStatus(BYTE *ucD2,BYTE *ucD4)
{
   *ucD2=inportb(Address+2);
   *ucD4=inportb(Address+4);
}

// ------------------------------------------------------------------------
//	移動點位
//  iP:點位
//  dbSpeedRatio:移動速度比
// ------------------------------------------------------------------------
bool HMotor::GoPos(int iP,double dbSpeedRatio)
{
    if (!bEnable)                           // 如果是在Disable狀態
        return true;

	int iPosition = 0;						// 實際點位置

    if (iP < (iGoPosBase - 1)) {		    // Call 點位
    	-- iP;
		iPosition = iWorkPos + iP * iPitch;     // 加上 PITCH
    }
    else {                                      // Call 特殊點位
        assert(iPosArray != NULL);			    // 已配置陣列
    	iP -= iGoPosBase;
        assert(iP >= 0 && iP < iPosArrayQuan);  // 必需小於特殊點位的陣列數
        iPosition = iPosArray[iP];
    }

    SetSpeed(Speed * dbSpeedRatio);             // 設定為工作速度

    return G00(iPosition);
}

// ------------------------------------------------------------------------
//	設定工作點
// ------------------------------------------------------------------------
void HMotor::SetWorkPos(int iPos,int iPch)
{
    iWorkPos = iPos;
    iPitch   = iPch;
}

// ------------------------------------------------------------------------
//	加上特殊點位
// ------------------------------------------------------------------------
int HMotor::AddSpecialPos(int iPos)
{
    if ((iPosArrayQuan + 1) >= iPosArraySize) {     // 陣列大小不足
        iPosArraySize += 100;
        int *iAP = new int[iPosArraySize];

        if (iPosArray != NULL) {                    // 如果點位陣列己有資料
            for (int iP = 0;iP < iPosArrayQuan;iP ++)
                iAP[iP] = iPosArray[iP];

            delete iPosArray;
        }
        iPosArray = iAP;
    }

    iPosArray[iPosArrayQuan] = iPos;    // 登記到陣列中
    iPosArrayQuan ++;                   // 陣列數加一

    return iGoPosBase + iPosArrayQuan - 1;
}

// ------------------------------------------------------------------------
//	清除特殊點位
// ------------------------------------------------------------------------
void HMotor::ClearSpecialPos()
{
    delete iPosArray;
    iPosArray = NULL;
    iPosArrayQuan = 0;
    iPosArraySize = 0;
}

// ------------------------------------------------------------------------
//
// ------------------------------------------------------------------------
void __fastcall HMotor::SetServoAlarmOn(bool Value)
{
    ServoAlarmOn = Value;
    InitMotor(Address);
}


// ------------------------------------------------------------------------
//  讀取位置(經過齒輪比)
// ------------------------------------------------------------------------
int HMotor::ReadPos()
{
    return ReadRealPos() * GearRatio;
}

// ------------------------------------------------------------------------
//  定點移動(經過齒輪比)
// ------------------------------------------------------------------------
bool HMotor::G00(int iPos)
{
    /*
    double r;
    int p;
    r=GearRatio;
    r=r*100.0;
    p=iPos*100;
    return RealG00( p/r );
    */

    //return RealG00(iPos / GearRatio);
    TargetPosition=iPos;

    int p;
    double r;
    p=iPos;
    r=GearRatio;
    int p1,p2;
    p1=p/r;                                //1666
    p2=p1*r;                               //999
    if(p2<p){
        while(1){
            p1++;
            p2=p1*r;
            if(p2>=p) break;
        }
    }
    else if(p2>p){
        while(1){
            p1--;
            p2=p1*r;
            if(p2<=p) break;
        }
    }
    return RealG00( p1 );
}
bool HMotor::G00_2(int iPos)
{
    /*
    double r;
    int p;
    r=GearRatio;
    r=r*100.0;
    p=iPos*100;
    return RealG00( p/r );
    */

    //return RealG00(iPos / GearRatio);
    TargetPosition=iPos;

    int p;
    double r;
    p=iPos;
    r=GearRatio;
    int p1,p2;
    p1=p/r;                                //1666
    p2=p1*r;                               //999
    if(p2<p){
        while(1){
            p1++;
            p2=p1*r;
            if(p2>=p) break;
        }
    }
    else if(p2>p){
        while(1){
            p1--;
            p2=p1*r;
            if(p2<=p) break;
        }
    }
    return RealG00_2( p1 );
}


//---------------------------------------------------------------------------
namespace Hmotor
{
    void __fastcall PACKAGE Register()
    {
        TComponentClass classes[1] = {__classid(HMotor)};
        RegisterComponents("HungKai", classes, 0);
    }
}
//---------------------------------------------------------------------------

