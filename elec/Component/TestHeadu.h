//---------------------------------------------------------------------------
#ifndef TestHeaduH
#define TestHeaduH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include "HMACHINE.h"
#include "HCylinder.h"
#include "HMotor.h"
#include "HSensor.h"
#include "HSwitcher.h"
//---------------------------------------------------------------------------
class TTestHead : public TMachine
{
__published:	// IDE-managed Components
    HMotor *XMotor;
    HMotor *YMotor;
    HCylinder *cySocketDown;
    HCylinder *cyHeadDown;
    HSensor *snBadSocket;
    HSwitcher *swFootBtn;

private:	// User declarations
           // 初始值設定
           int Xpos,Ypos;         //XYMotor座標
           int iBadSocket[100][3];//資料
           float  FlagGoRight;  // 1:true;;-1:flase
           HTimer T1[8]; // It is used in FindPosDown();
           HTimer T2[5]; // It is used in SingleSocketTest()
           HTimer T3[10]; // It is used in
           HTimer HeadDownTimer;
           HTimer SocketUpTimer;
           HTimer HeadUpTimer;
           int XSpeed,YSpeed;    //
           int XYResetTask;
           int UpToWaitPosTask;
           int SingleSocketTestTask;
           int XWorkTimes; //工作次數
           int XYResetQuen;//Count is ResetMotor Quen


public:		// User declarations
    __fastcall TTestHead(TComponent* Owner);

    int Xi,Yi;             //XY現為第X,Y
    int XPitch,YPitch;     //XY間距
    int XItem,YItem;       //XY行列個數
    int IntXpos,IntYpos;   //起始點
    int HeadDownQKTime;    //Head下降後停留時間 unit 0.1sec
    int RealXPos,RealYPos;//用於單顆測試，實際位置
    //-----------------
    //供外部呼叫函數
    //-----------------
    bool FindPosDown();       //Head移動至測試點
    void FindPosDownIniSet();                         // 註：第一次使用此函數需先呼叫XYResetIniSet();
                              //     及XYReset()且需在不同case 下
    void UpToWaitPosIniSet();
    bool UpToWaitPos();       //Head,Socket上升到等待點
                              //註：需先呼叫UpToWaitPosIniSet()在不同case 下
    void XYResetIniSet();
    bool XYReset();          //XYMotor歸零
                             //註：需先呼叫XYResetIniSet()在不同case 下
    bool FlagFindPosDown;
    int Task;
    void SingleSocketTestIniSet();
    bool SingleSocketTest(int SingleXPos,int SingleYPos);
         //單顆測試需送入該顆的座標
         //左上角第一顆為(1,1).使用此函數前需呼叫SingleTestIniSet()
         // (1,1) (1,2)(1,3)(1,4) ....
         // (2,1)(2,2)(2,3)......
         // (3,1)(3,2)......
         //       ︿
         //       ||
         //       ||　方向
         //

};
//---------------------------------------------------------------------------
extern PACKAGE TTestHead *TestHead;
//---------------------------------------------------------------------------
#define ALM_SocketDown     801
#define ALM_SocketUp       802
#define ALM_HeadUp         803
#define ALM_HeadDown       804
#define ALM_HeadInDown     805
#define ALM_SocketInUp     806
#define ALM_XMotorYetReset 807
#define ALM_YMotorYetReset 808
#define ALM_OverSafeLimit  809
#define ALM_XYDontMove     810
#define ALM_AvoidToBump    811
//---------------------------------------------------------------------------
#endif
