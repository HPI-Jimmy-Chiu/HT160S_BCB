//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "TestHeadu.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "HMACHINE"
#pragma link "HCylinder"
#pragma link "HMotor"
#pragma link "HSensor"
#pragma link "HSwitcher"
#pragma resource "*.dfm"
TTestHead *TestHead;
//---------------------------------------------------------------------------
__fastcall TTestHead::TTestHead(TComponent* Owner)
    : TMachine(Owner)
{
}
//---------------------------------------------------------------------------
void TTestHead::FindPosDownIniSet()
{
  Task=10;
}
//---------------------------------------------------------------------------
bool mTestHead::FindPosDown()
{
 switch (Task){
    case 10:Ypos=IntYpos;//初始值設定
            Xpos=IntXpos;
            XWorkTimes=0;
            FlagGoRight=1;
            Xi=1;Yi=1; //第一個
            Task=20;
            break;;
/*
      XMotor;
      YMotor;
      cySocketDown;
      cyHeadDown;
      snBadSocket;
      swFootBtn;


    case 45: T1[0].Set(2);              // when one Socket is done
             T1[0].On();                //all circle is back to case 45
             if (T1[0].Off()){
             XWorkTimes++;
             Task=50;            //if have BadSocketSensor Task=50
             }                        // otherwise  Task=46;ALM_OverSafeLimit
             break;//休息站


             if (Yi==YItem){
                 Task=9999;    //case=9999 means:It is end.
                 break;
                 }
             Yi++;
             Ypos+=YPitch;
             Task=220;
             XWorkTimes=0;
             break;

    case 250:
             if (FlagGoRight==1){
                 Xpos+=XPitch;
                 Xi++;
              }
             else{
                  Xpos-=XPitch;
                  Xi--;
              }
              Task=260;
              break;

      return false; */
} //void
//----------------------------------------------------------------------------

