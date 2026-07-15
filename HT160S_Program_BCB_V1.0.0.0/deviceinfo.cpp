//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "deviceinfo.h"
#include "database.h"
#include "cCsvDailyLog.h"    // cCsvDailyLog::CsvQuote for Production_Log fields
#include <stdio.h>
#include <FileCtrl.hpp>
//---------------------------------------------------------------------------
#pragma package(smart_init)

TDeviceInfo g_DeviceInfo;

//---------------------------------------------------------------------------
TDeviceInfo::TDeviceInfo()
    : m_pCS(NULL)
    , m_sBaseDir("")
    , m_sCachedFilePath("")
    , m_sStartTimeStr("")
    , m_sLotID("")
{
}

//---------------------------------------------------------------------------
TDeviceInfo::~TDeviceInfo()
{
    delete m_pCS;
    m_pCS = NULL;
}

//---------------------------------------------------------------------------
void TDeviceInfo::Init()
{
    if (!m_pCS)
        m_pCS = new TCriticalSection();

    // Central log root constant (HSys.LogRootDir = "D:\\HT160S_Log")
    m_sBaseDir = HSys.LogRootDir + "\\Production_Log";
    ForceDirectories(m_sBaseDir);

    //AI(ht160s-prodlog) 20260716 : ALSO open a per-DAY aggregate CSV so a full calendar
    //day is one readable file (the per-lot file above is unchanged). Resolves to
    //Production_Log/Daily/YYYY_MM/Production_YYYY_MM_DD.csv under HSys.LogRootDir. Same
    //21-column header as the per-lot files (GetTitleLine). Retention set from ht160s.cpp.
    m_dailyProd.InitLog("Production_Log\\Daily", "Production", GetTitleLine(),
                        cCsvDailyLog::lgMonthlyFolder, ".csv");
}

//---------------------------------------------------------------------------
void TDeviceInfo::OnLotStart(const AnsiString& sLotID, TDateTime dtStart)
{
    m_sLotID = sLotID;
    m_sStartTimeStr = FormatDateTime("yyyy-mm-dd_hhnnss", dtStart);
    m_sCachedFilePath = "";  // Force regeneration
    ClearBatch();
}

//---------------------------------------------------------------------------
AnsiString TDeviceInfo::GetLogFilePath()
{
    if (!m_sCachedFilePath.IsEmpty())
        return m_sCachedFilePath;

    AnsiString sYYYYMM = FormatDateTime("yyyymm", Now());
    AnsiString sDir = m_sBaseDir + "\\" + sYYYYMM;
    ForceDirectories(sDir);

    AnsiString sLot = m_sLotID;
    if (sLot.IsEmpty())
        sLot = "NoLot";

    m_sCachedFilePath = sDir + "\\" + sLot + "_" + m_sStartTimeStr + ".csv";
    return m_sCachedFilePath;
}

//---------------------------------------------------------------------------
AnsiString TDeviceInfo::GetTitleLine()
{
    return "Start Time,Load_X,Load_Y,Load_Time,Tray_ID,"
           "Which Arm,Suck_X,Suck_Y,Which Auto,"
           "Bin,Output tray,Unload_X,Unload_Y,Unload_Time,Error log,"
           "TraceCode,ErrorType,Lot,Code2D,Manual2D,PassFail";
}

//---------------------------------------------------------------------------
//AI(ht160s-2dbin-import) 20260714 : RFC-4180 quote-if-needed for one
//Production_Log field. A 2D code (Code2D/Manual2D) or free-text error may
//contain a comma; quote it (doubling embedded ") so the comma cannot break the
//column layout. Plain values stay bare so existing rows/consumers are
//byte-identical. Mirrors cSoterOutput::CsvField. Production_Log is write-only.
static AnsiString ProdCsvField(const AnsiString& s)
{
    if (s.Pos(",") > 0 || s.Pos("\"") > 0 || s.Pos("\n") > 0 || s.Pos("\r") > 0)
        return cCsvDailyLog::CsvQuote(s);
    return s;
}
//---------------------------------------------------------------------------
AnsiString TDeviceInfo::GetDataLine(int iNozzle)
{
    if (iNozzle < 0 || iNozzle >= 4)
        return "";

    AnsiString sLine;
    for (int i = 0; i < PROD_LOG_FIELD_COUNT; ++i)
    {
        if (i > 0)
            sLine += ",";
        sLine += ProdCsvField(m_records[iNozzle].sField[i]);
    }
    return sLine;
}

//---------------------------------------------------------------------------
void TDeviceInfo::EnsureHeader(const AnsiString& sPath)
{
    if (FileExists(sPath))
        return;

    FILE* fp = fopen(sPath.c_str(), "w");
    if (fp)
    {
        fprintf(fp, "%s\n", GetTitleLine().c_str());
        fclose(fp);
    }
}

//---------------------------------------------------------------------------
void TDeviceInfo::AppendLine(const AnsiString& sPath, const AnsiString& sLine)
{
    if (!m_pCS)
        return;

    m_pCS->Acquire();
    try
    {
        EnsureHeader(sPath);
        FILE* fp = fopen(sPath.c_str(), "a");
        if (fp)
        {
            fprintf(fp, "%s\n", sLine.c_str());
            fclose(fp);
        }
    }
    __finally
    {
        m_pCS->Release();
    }
}

//---------------------------------------------------------------------------
AnsiString TDeviceInfo::NowTimeStr()
{
    return FormatDateTime("yyyymmdd_hhnnss", Now());
}

//---------------------------------------------------------------------------
void TDeviceInfo::AddInputInfo(int iNozzle, int iRow, int iCol,
                               const AnsiString& sTrayID)
{
    if (iNozzle < 0 || iNozzle >= 4)
        return;

    m_records[iNozzle].Clear();
    m_records[iNozzle].bActive = true;
    m_records[iNozzle].sField[eStartTime]  = m_sStartTimeStr;
    m_records[iNozzle].sField[eLoadX]      = IntToStr(iCol);
    m_records[iNozzle].sField[eLoadY]      = IntToStr(iRow);
    m_records[iNozzle].sField[eLoadTime]   = NowTimeStr();
    m_records[iNozzle].sField[eLoadTrayID] = sTrayID;
    m_records[iNozzle].sField[eWhichArm]   = "1";
    m_records[iNozzle].sField[eSuckX]      = IntToStr(iNozzle);
    m_records[iNozzle].sField[eSuckY]      = "0";
}

//---------------------------------------------------------------------------
void TDeviceInfo::AddIcIdentity(int iNozzle, const AnsiString& sLotID,
                                const AnsiString& sCode2D, bool bManual2D)
{
    if (iNozzle < 0 || iNozzle >= 4)
        return;
    if (!m_records[iNozzle].bActive)
        return;
    m_records[iNozzle].sField[eLotID]  = sLotID;
    m_records[iNozzle].sField[e2DCode] = sCode2D;
    m_records[iNozzle].sField[eManual2D] = bManual2D ? "1" : "";
}

//---------------------------------------------------------------------------
void TDeviceInfo::AddBinInfo(int iNozzle, int iBinIndex, int iGradeCode)
{
    if (iNozzle < 0 || iNozzle >= 4)
        return;
    if (!m_records[iNozzle].bActive)
        return;

    if (iBinIndex >= 0)
    {
        m_records[iNozzle].sField[eWhichAuto] = "Auto" + FormatFloat("00", iBinIndex + 1);
        m_records[iNozzle].sField[eBin]       = IntToStr(iGradeCode);
    }
    else
    {
        m_records[iNozzle].sField[eWhichAuto] = "Reject";
        m_records[iNozzle].sField[eBin]       = "-1";
    }
}

//---------------------------------------------------------------------------
void TDeviceInfo::AddPassFail(int iNozzle, const AnsiString& sPassFail)
{
    if (iNozzle < 0 || iNozzle >= 4)
        return;
    if (!m_records[iNozzle].bActive)
        return;
    m_records[iNozzle].sField[ePassFail] = sPassFail;
}
//---------------------------------------------------------------------------
// iTraceCode: 0 = no trace (normal); 999~1099 = trace code; mapped to ErrorType label.
void TDeviceInfo::AddTraceInfo(int iNozzle, int iTraceCode)
{
    if (iNozzle < 0 || iNozzle >= 4)
        return;
    if (!m_records[iNozzle].bActive)
        return;

    if (iTraceCode == 0)
    {
        m_records[iNozzle].sField[eTraceCode] = "";
        m_records[iNozzle].sField[eErrorType] = "";
        return;
    }

    m_records[iNozzle].sField[eTraceCode] = IntToStr(iTraceCode);
    AnsiString sLabel;
    switch (iTraceCode)
    {
        case 999:  sLabel = "ScanFail";      break;
        case 1000: sLabel = "NoMap";         break;
        case 1001: sLabel = "2DMapMissing";  break;
        case 1002: sLabel = "ParseFail";     break;
        case 1003: sLabel = "SortFail";      break;
        case 1004: sLabel = "PFOverflow";    break;   //AI(ht160s-lotpassfail) 20260709 : PASS/FAIL product forced to Error Auto (all non-Error Autos taken)
        default:   sLabel = "TraceCode_" + IntToStr(iTraceCode); break;
    }
    m_records[iNozzle].sField[eErrorType] = sLabel;
}

//---------------------------------------------------------------------------
void TDeviceInfo::AddOutputInfo(int iNozzle, const AnsiString& sAutoName,
                                const AnsiString& sOutputTray,
                                int iRow, int iCol)
{
    if (iNozzle < 0 || iNozzle >= 4)
        return;
    if (!m_records[iNozzle].bActive)
        return;

    if (!sAutoName.IsEmpty())
        m_records[iNozzle].sField[eWhichAuto] = sAutoName;
    m_records[iNozzle].sField[eOutTrayID]   = sOutputTray;
    m_records[iNozzle].sField[eUnloadX]     = IntToStr(iCol);
    m_records[iNozzle].sField[eUnloadY]     = IntToStr(iRow);
    m_records[iNozzle].sField[eUnloadTime]  = NowTimeStr();

    //AI(ht160s-prodlog) 20260716 : build the row ONCE and write it to BOTH the per-lot
    //file and the per-DAY aggregate so the two stay byte-identical (m_records is not
    //locked between the two appends).
    AnsiString sPath = GetLogFilePath();
    AnsiString sLine = GetDataLine(iNozzle);
    AppendLine(sPath, sLine);
    m_dailyProd.AppendLine(sLine);
}

//---------------------------------------------------------------------------
void TDeviceInfo::SaveRejectRecord(int iNozzle, const AnsiString& sError)
{
    if (iNozzle < 0 || iNozzle >= 4)
        return;
    if (!m_records[iNozzle].bActive)
        return;

    m_records[iNozzle].sField[eWhichAuto]   = "Reject";
    m_records[iNozzle].sField[eOutTrayID]   = "";
    m_records[iNozzle].sField[eUnloadX]     = "";
    m_records[iNozzle].sField[eUnloadY]     = "";
    m_records[iNozzle].sField[eUnloadTime]  = NowTimeStr();
    m_records[iNozzle].sField[eErrorCode]   = sError;

    //AI(ht160s-prodlog) 20260716 : build the row ONCE; write to per-lot + per-DAY (see AddOutputInfo).
    AnsiString sPath = GetLogFilePath();
    AnsiString sLine = GetDataLine(iNozzle);
    AppendLine(sPath, sLine);
    m_dailyProd.AppendLine(sLine);
}

//---------------------------------------------------------------------------
void TDeviceInfo::ClearBatch()
{
    for (int i = 0; i < 4; ++i)
        m_records[i].Clear();
}

//---------------------------------------------------------------------------
//AI(ht160s-prodlog) 20260716 : set the per-day aggregate log retention. Called from the
//ht160s.cpp boot block (same place as the other channels SetRetentionDays), so deviceinfo
//need not depend on GeneralSetting. 0 = keep forever.
void TDeviceInfo::SetDailyRetentionDays(int nDays)
{
    m_dailyProd.SetRetentionDays(nDays);
}
//---------------------------------------------------------------------------
