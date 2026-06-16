//---------------------------------------------------------------------------
// cCommLog : generic per-channel serial communication CSV log.
// Now a thin subclass of cCsvDailyLog (shared infrastructure). Each channel
// keeps its own folder under HSys.LogRootDir, split into monthly sub-folders
// + daily files:
//   D:\HT160S_Log\<Name>\YYYY_MM\<Name>_YYYY_MM_DD.csv
// Two instances are provided: g_PadCommLog and g_BinDispCommLog.
//---------------------------------------------------------------------------
#ifndef cCommLogH
#define cCommLogH
//---------------------------------------------------------------------------
#include <vcl.h>
#include "cCsvDailyLog.h"
//---------------------------------------------------------------------------
class cCommLog : public cCsvDailyLog
{
public:
    // sName is both the sub-folder and the file prefix (e.g. "PadLog").
    void Init(const AnsiString& sName);
    // One line per call: Date,Time,Action,"Message".
    void Log(const AnsiString& sAction, const AnsiString& sMessage);
};
//---------------------------------------------------------------------------
extern cCommLog g_PadCommLog;
extern cCommLog g_BinDispCommLog;
//---------------------------------------------------------------------------
#endif
