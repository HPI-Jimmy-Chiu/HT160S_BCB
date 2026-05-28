/*---------------------------------------------------------------------------
	程式目的:計時器
    設計人員:陳鴻德
    設計日期:1998/04/06
  --------------------------------------------------------------------------- */
#ifndef HTimerH
#define HTimerH
// ---------------------------------------------------------------------------
//	計時器
// ---------------------------------------------------------------------------
class PACKAGE HTimer
{
public:
	__fastcall HTimer();
    __fastcall ~HTimer();
	void	On();        			// 打開計時器
	bool	Off();					// 是否時間到
	void    Set(int iTime);			// 計定計時長度
	void    SetTimer(int iTime,int iID);// 計定計時長度
    void    SetMS(int iTime);       // 計定計時長度(1/1000sec)
    void    Clear();                // 清除計時值
    void    Pause();                // 暫停計時
    void    ReStart();              // 重新計時
    void    SetSec(double iTime);		  // 計定計時長度
    void    SetSecAndOn(double iTime);	  // 計定計時長度
    void    SetMSAndOn(int iTime);	  // 計定計時長度	   
	void    ResetMSAndOn(int iTime);

protected:
private:
	DWORD 	ulStartTicks;           // 開始時的TICK COUNT
    DWORD   ulPauseTicks;           // 暫停時的TICK COUNT
    int		iTimeLen;               // 計時長度
    int     iPauseLen;              // 到暫停結束時的長度(暫停多久)
    bool    Paused;                 // 是否暫停中
    bool	InUsed;                 // 是否使用中
    int     iTimerID;               // 計數終了 
};

//---------------------------------------------------------------------------
//  FUNCTION PROTOTYPE
//---------------------------------------------------------------------------
void  PACKAGE  ClearAllTimer();        // 清除所有計時器計時值
void  PACKAGE  PauseAllTimer();        // 暫停所有計時器
void  PACKAGE  ReStartAllTimer();      // 重新起動所有計時器
#endif
