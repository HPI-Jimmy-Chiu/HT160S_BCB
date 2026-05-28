//---------------------------------------------------------------------------
#ifndef HAlarmH
#define HAlarmH
//---------------------------------------------------------------------------
//  CLASS DEFINE
//---------------------------------------------------------------------------
class PACKAGE HAlarm
{
public:
    TList *ErrNoList;                   // 錯誤編號串列

    HAlarm(TComponent *P);
    virtual ~HAlarm();
    operator == (bool t) {              // 目前是否有 ALARM
        if (ErrNoList->Count > 0 && t)
            return true;
        return false;
    }
    void    Set(int iNo);           // 設定ALARM
    bool    Clear(int iNo);         // 清除ALARM
    void    Clear();                // 清除所有ALARM
    bool    GetStat(int iNo);       // 取得該ALARM編號錯誤是否存在

protected:
    TComponent *Parent;

private:
};

//---------------------------------------------------------------------------
extern bool PACKAGE SystemNG;

//---------------------------------------------------------------------------
//  FUNCTION PROTOTYPE
//---------------------------------------------------------------------------
void  PACKAGE  ClearAllAlarm();        // 清除所有物件的錯誤碼
bool  PACKAGE  PopUpAlarm(TComponent **Component,int &iErrCode);   // 取出要顯示的錯誤訊息
bool  PACKAGE  PopUpClrAlarm(TComponent **Component,int &iErrCode);    // 取出要清除顯示的錯誤訊息
#endif
