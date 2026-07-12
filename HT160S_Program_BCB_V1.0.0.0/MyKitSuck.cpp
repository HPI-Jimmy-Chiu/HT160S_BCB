//---------------------------------------------------------------------------
#include <vcl.h>
#include <stdio.h>
#pragma hdrstop

#include "IncludeAllHeader.h"
#include "MyKitSuck.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
static const int SuckNullIC=0;
static const int SuckHasNullIC=2;
static const int SuckTestPass=1001;
static const int SuckNullICTest=1002;
static const int SuckHasNullICTest=1002;
//---------------------------------------------------------------------------
__fastcall TMySucker::TMySucker()
{
    Task=1;
    VacuumOnTime=120;
    VacuumOffTime=70;
    SensorName="";
    OnPortName="";
    OffPortName="";
    SuckerName="";
    Alias="";
    iMyRow=0;
    iMyCol=0;
    Tag=0;
    Error=false;
    Enable=false;
    EnableAtDataBase=false;
    Status=false;
    OnAlarmCode="0";
    OffAlarmCode="0";
    OnAlarmTime=100;
    OffAlarmTime=100;
    OnDelayTime=0;
    OffDelayTime=0;
    iManualOffTask=0;
    TempString="";
    RealTimeRefreshVacuumOnOffTime=false;
    Item=SuckNullIC;
    pLed=NULL;
    iGangCount=0;
    for(int GangIndex=0; GangIndex<MAX_SUB_SUCKER_ITEM; GangIndex++)
        pGang[GangIndex]=NULL;
    for(int Index=0; Index<20; Index++)
    {
        VacuumOnTimeBuffer[Index]=(DWORD)-1;
        VacuumOffTimeBuffer[Index]=(DWORD)-1;
    }
}
//---------------------------------------------------------------------------
void TMySucker::Reset()
{
    Task=1;
    Delay.Clear();
    Error=false;
}
//---------------------------------------------------------------------------
bool TMySucker::GetOnBit()
{
    return OnSw.OutValue;
}
//---------------------------------------------------------------------------
bool TMySucker::GetOffBit()
{
    return OffSw.OutValue;
}
//---------------------------------------------------------------------------
bool TMySucker::GetStatus()
{
    if(HSys.LastSet.iRealDummy!=REALLY)
        return true;
    return Sensor.IsOn();
}
//---------------------------------------------------------------------------
//AI(ht160s-suck2-quad) 20260712 : gang sensor judgments. A member whose sensor row
//is disabled is skipped, so a deliberately disabled circuit degrades the gang to the
//remaining sensors instead of wedging pick/place.
bool TMySucker::SensorAllOn()
{
    if(iGangCount>0)
    {
        for(int GangIndex=0; GangIndex<iGangCount; GangIndex++)
            if(pGang[GangIndex]->Sensor.Enable==true && pGang[GangIndex]->Sensor.IsOn()==false)
                return false;
        return true;
    }
    return Sensor.IsOn();
}
//---------------------------------------------------------------------------
bool TMySucker::SensorAnyOn()
{
    if(iGangCount>0)
    {
        for(int GangIndex=0; GangIndex<iGangCount; GangIndex++)
            if(pGang[GangIndex]->Sensor.Enable==true && pGang[GangIndex]->Sensor.IsOn()==true)
                return true;
        return false;
    }
    return Sensor.IsOn();
}
//---------------------------------------------------------------------------
bool TMySucker::GetStatusAllOn()
{
    if(HSys.LastSet.iRealDummy!=REALLY)
        return true;
    return SensorAllOn();
}
//---------------------------------------------------------------------------
bool TMySucker::GetStatusAnyOn()
{
    if(HSys.LastSet.iRealDummy!=REALLY)
        return true;
    return SensorAnyOn();
}
//---------------------------------------------------------------------------
void TMySucker::OnSuck()
{
    if(iGangCount>0)
    {
        for(int GangIndex=0; GangIndex<iGangCount; GangIndex++)
            pGang[GangIndex]->OnSw.On();
        return;
    }
    OnSw.On();
}
//---------------------------------------------------------------------------
void TMySucker::OffSuck()
{
    if(iGangCount>0)
    {
        for(int GangIndex=0; GangIndex<iGangCount; GangIndex++)
            pGang[GangIndex]->OnSw.Off();
        return;
    }
    OnSw.Off();
}
//---------------------------------------------------------------------------
void TMySucker::OnDestroy()
{
    if(iGangCount>0)
    {
        for(int GangIndex=0; GangIndex<iGangCount; GangIndex++)
            pGang[GangIndex]->OffSw.On();
        return;
    }
    OffSw.On();
}
//---------------------------------------------------------------------------
void TMySucker::OffDestroy()
{
    if(iGangCount>0)
    {
        for(int GangIndex=0; GangIndex<iGangCount; GangIndex++)
            pGang[GangIndex]->OffSw.Off();
        return;
    }
    OffSw.Off();
}
//---------------------------------------------------------------------------
void TMySucker::On()
{
    OffDestroy();
    OnSuck();
}
//---------------------------------------------------------------------------
void TMySucker::Off()
{
    OffSuck();
    OnDestroy();
}
//---------------------------------------------------------------------------
void TMySucker::Normal()
{
    OffSuck();
    OffDestroy();
}
//---------------------------------------------------------------------------
bool TMySucker::Suck()
{
    if(Enable==false)
    {
        On();
        Reset();
        return true;
    }

    if(Task==1)
    {
        On();
        Error=false;
        if(Sensor.Enable==true && SensorAllOn()==false && HSys.LastSet.iRealDummy==REALLY)
        {
            Delay.Clear();
            Delay.SetMS(OnAlarmTime);
            Delay.On();
            Task=50;
        }
        else
        {
            Delay.Clear();
            Delay.SetMS(OnDelayTime);
            Delay.On();
            Task=100;
        }
    }

    if(Task==50)
    {
        if(SensorAllOn())
        {
            Delay.Clear();
            Delay.SetMS(OnDelayTime);
            Delay.On();
            Task=100;
        }
        else if(Delay.Off())
        {
            Error=true;
            OffSuck();
            Task=1;
            return false;
        }
        else
        {
            return false;
        }
    }

    if(Task==100)
    {
        if(Delay.Off())
        {
            Task=1;
            Error=false;
            return true;
        }
        return false;
    }

    Task=1;
    return true;
}
//---------------------------------------------------------------------------
bool TMySucker::Destroy()
{
    if(Enable==false)
    {
        Off();
        OffDestroy();
        Reset();
        return true;
    }

    if(Task==1)
    {
        Off();
        Error=false;
        if(Sensor.Enable==true && SensorAnyOn()==true && HSys.LastSet.iRealDummy==REALLY)
        {
            Delay.Clear();
            Delay.SetMS(OffAlarmTime);
            Delay.On();
            Task=50;
        }
        else
        {
            Delay.Clear();
            Delay.SetMS(OffDelayTime);
            Delay.On();
            Task=100;
        }
    }

    if(Task==50)
    {
        if(SensorAnyOn()==false)
        {
            Delay.Clear();
            Delay.SetMS(OffDelayTime);
            Delay.On();
            Task=100;
        }
        else if(Delay.Off())
        {
            Error=true;
            OffDestroy();
            Task=1;
            return false;
        }
        else
        {
            return false;
        }
    }

    if(Task==100)
    {
        if(Delay.Off())
        {
            Task=1;
            Error=false;
            OffDestroy();
            return true;
        }
        return false;
    }

    Task=1;
    return true;
}
//---------------------------------------------------------------------------
void TMySucker::ResetSuckTask()
{
    if(Task!=1)
        Task=2;
}
//---------------------------------------------------------------------------
void TMySucker::SetRetryCount(int Count)
{
    (void)Count;
}
//---------------------------------------------------------------------------
void TMySucker::CheckIsFallDown()
{
    if(Enable==false || Task>2 || HSys.LastSet.iRealDummy!=REALLY)
        return;
    if(Sensor.IsOn()==false)
        Normal();
}
//---------------------------------------------------------------------------
void TMySucker::PushOnTime()
{
}
//---------------------------------------------------------------------------
void TMySucker::PushOffTime()
{
}
//---------------------------------------------------------------------------
__fastcall TMyKitSuck::TMyKitSuck()
{
    Name="";
    FlushPanelName="";
    for(int Index=0; Index<eSuckErrTotal; Index++)
        AlarmName[Index]="";
    MaxItem=MAX_SUB_SUCKER_ITEM;
    MaxItemR=MAX_SUCKER_ROW;
    MaxItemC=MAX_SUCKER_COL;
    Tag=0;
    TrayData=SuckNullIC;
    SuckIC=false;
    Has_SuckIC=false;
    sSuck_2D="";
    sRule="";
    sBin="";
    for(int RowIndex=0; RowIndex<MAX_SUCKER_ROW; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<MAX_SUCKER_COL; ColIndex++)
        {
            int Index=RowIndex*MAX_SUCKER_COL+ColIndex;
            Suck[RowIndex][ColIndex].Item=SuckNullIC;
            Suck[RowIndex][ColIndex].pLed=NULL;
            iWhichAuto[Index]=0;
            iBinData[Index]=999;
            sprintf(cBinInfor[Index], "");
        }
    }
}
//---------------------------------------------------------------------------
void TMyKitSuck::SetMyLed(int RowIndex, int ColIndex, TMyLed *ledPtr)
{
    if(RowIndex<0 || RowIndex>=MaxItemR || ColIndex<0 || ColIndex>=MaxItemC)
        return;
    Suck[RowIndex][ColIndex].pLed=ledPtr;
}
//---------------------------------------------------------------------------
void TMyKitSuck::SetItemData(int RowIndex, int ColIndex, int Data)
{
    if(RowIndex<0 || RowIndex>=MaxItemR || ColIndex<0 || ColIndex>=MaxItemC)
        return;
    Suck[RowIndex][ColIndex].Item=Data;
}
//---------------------------------------------------------------------------
void TMyKitSuck::SetItemAmount(int Count)
{
    if(Count<0)
        Count=0;
    if(Count>MAX_SUB_SUCKER_ITEM)
        Count=MAX_SUB_SUCKER_ITEM;
    SetItemAmount((Count>0)?1:0, Count);
}
//---------------------------------------------------------------------------
void TMyKitSuck::SetItemAmount(int RowCount, int ColCount)
{
    if(RowCount<0)
        RowCount=0;
    if(ColCount<0)
        ColCount=0;
    if(RowCount>MAX_SUCKER_ROW)
        RowCount=MAX_SUCKER_ROW;
    if(ColCount>MAX_SUCKER_COL)
        ColCount=MAX_SUCKER_COL;
    MaxItemR=RowCount;
    MaxItemC=ColCount;
    MaxItem=MaxItemR*MaxItemC;
}
//---------------------------------------------------------------------------
void TMyKitSuck::initMyKitSuck(AnsiString sName, AnsiString sFlushPanel, int RowCount, int ColCount)
{
    Name=sName.Trim();
    FlushPanelName=sFlushPanel.Trim();
    SetItemAmount(RowCount, ColCount);

    AnsiString Str;
    for(int RowIndex=0; RowIndex<MaxItemR; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<MaxItemC; ColIndex++)
        {
            int SuckIndex=RowIndex*MaxItemC+ColIndex;
            if(SuckIndex>=MAX_SUB_SUCKER_ITEM)
                continue;

            Suck[RowIndex][ColIndex].iMyRow=RowIndex;
            Suck[RowIndex][ColIndex].iMyCol=ColIndex;
            Suck[RowIndex][ColIndex].Tag=RowIndex*10+ColIndex;
            Str.sprintf("%c%c", 'A'+RowIndex, 'a'+ColIndex);
            Suck[RowIndex][ColIndex].Alias=Str;
            Suck[RowIndex][ColIndex].SensorName=Name+Suck[RowIndex][ColIndex].Alias;
            if(Suck[RowIndex][ColIndex].SuckerName==AnsiString(""))
            {
                if(MaxItem==1)
                    Suck[RowIndex][ColIndex].SuckerName=Name;
                else
                    Suck[RowIndex][ColIndex].SuckerName.sprintf("%s%d", Name.c_str(), SuckIndex+1);
            }
        }
    }
}
//---------------------------------------------------------------------------
bool TMyKitSuck::NoIC()
{
    for(int RowIndex=0; RowIndex<MaxItemR; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<MaxItemC; ColIndex++)
        {
            if(Suck[RowIndex][ColIndex].Item!=SuckNullIC && Suck[RowIndex][ColIndex].Item!=SuckNullICTest)
                return false;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
bool TMyKitSuck::HasIC()
{
    return !NoIC();
}
//---------------------------------------------------------------------------
bool TMyKitSuck::HasType(int Data)
{
    for(int RowIndex=0; RowIndex<MaxItemR; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<MaxItemC; ColIndex++)
        {
            if(Suck[RowIndex][ColIndex].Item==Data)
                return true;
        }
    }
    return false;
}
//---------------------------------------------------------------------------
int TMyKitSuck::GetCountOfDeiveType(int Data)
{
    int Count=0;
    for(int RowIndex=0; RowIndex<MaxItemR; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<MaxItemC; ColIndex++)
        {
            if(Suck[RowIndex][ColIndex].Item==Data)
                Count++;
        }
    }
    return Count;
}
//---------------------------------------------------------------------------
bool TMyKitSuck::AllIs_HAS_NULL_IC()
{
    return GetCountOfDeiveType(SuckHasNullIC)==MaxItem;
}
//---------------------------------------------------------------------------
bool TMyKitSuck::HasRealIC()
{
    if(GetCountOfDeiveType(SuckNullIC)==0 &&
       GetCountOfDeiveType(SuckHasNullIC)==0 &&
       GetCountOfDeiveType(SuckNullICTest)==0 &&
       GetCountOfDeiveType(SuckHasNullICTest)==0)
        return true;
    return false;
}
//---------------------------------------------------------------------------
void TMyKitSuck::ClearSingle(int RowIndex, int ColIndex)
{
    if(RowIndex<0 || RowIndex>=MaxItemR || ColIndex<0 || ColIndex>=MaxItemC)
        return;
    SetItemData(RowIndex, ColIndex, SuckNullIC);
    Suck[RowIndex][ColIndex].DeviceInfo.Clear();
}
//---------------------------------------------------------------------------
void TMyKitSuck::ClearAll()
{
    for(int RowIndex=0; RowIndex<MaxItemR; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<MaxItemC; ColIndex++)
            ClearSingle(RowIndex, ColIndex);
    }
}
//---------------------------------------------------------------------------
void TMyKitSuck::SetAll(int Type)
{
    for(int RowIndex=0; RowIndex<MaxItemR; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<MaxItemC; ColIndex++)
            SetItemData(RowIndex, ColIndex, Type);
    }
}
//---------------------------------------------------------------------------
bool TMyKitSuck::HasTestedDevice()
{
    for(int RowIndex=0; RowIndex<MaxItemR; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<MaxItemC; ColIndex++)
        {
            if(Suck[RowIndex][ColIndex].Item>=SuckTestPass)
                return true;
        }
    }
    return false;
}
//---------------------------------------------------------------------------
bool TMyKitSuck::HasDeviceNotTested()
{
    for(int RowIndex=0; RowIndex<MaxItemR; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<MaxItemC; ColIndex++)
        {
            if(Suck[RowIndex][ColIndex].Item<SuckTestPass)
                return true;
        }
    }
    return false;
}
//---------------------------------------------------------------------------
bool TMyKitSuck::FullIC()
{
    for(int RowIndex=0; RowIndex<MaxItemR; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<MaxItemC; ColIndex++)
        {
            if(Suck[RowIndex][ColIndex].Item==SuckNullIC)
                return false;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
void TMyKitSuck::MoveSingleItem(TMyKitSuck &Source, int RowIndex, int ColIndex)
{
    if(RowIndex<0 || RowIndex>=MaxItemR || RowIndex>=Source.MaxItemR ||
       ColIndex<0 || ColIndex>=MaxItemC || ColIndex>=Source.MaxItemC)
        return;
    SetItemData(RowIndex, ColIndex, Source.Suck[RowIndex][ColIndex].Item);
    Suck[RowIndex][ColIndex].DeviceInfo.MoveFrom(Source.Suck[RowIndex][ColIndex].DeviceInfo);
    Source.SetItemData(RowIndex, ColIndex, SuckNullIC);
}
//---------------------------------------------------------------------------
void TMyKitSuck::CopySingleItem(TMyKitSuck &Source, int RowIndex, int ColIndex)
{
    if(RowIndex<0 || RowIndex>=MaxItemR || RowIndex>=Source.MaxItemR ||
       ColIndex<0 || ColIndex>=MaxItemC || ColIndex>=Source.MaxItemC)
        return;
    SetItemData(RowIndex, ColIndex, Source.Suck[RowIndex][ColIndex].Item);
    Suck[RowIndex][ColIndex].DeviceInfo.CopyFrom(Source.Suck[RowIndex][ColIndex].DeviceInfo);
}
//---------------------------------------------------------------------------
void TMyKitSuck::MoveAllItem(TMyKitSuck &Source)
{
    int RowCount=(Source.MaxItemR>=MaxItemR)?MaxItemR:Source.MaxItemR;
    int ColCount=(Source.MaxItemC>=MaxItemC)?MaxItemC:Source.MaxItemC;
    for(int RowIndex=0; RowIndex<RowCount; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<ColCount; ColIndex++)
            MoveSingleItem(Source, RowIndex, ColIndex);
    }
}
//---------------------------------------------------------------------------
void TMyKitSuck::CopyFrom(TMyKitSuck &Source)
{
    int RowCount=(Source.MaxItemR>=MaxItemR)?MaxItemR:Source.MaxItemR;
    int ColCount=(Source.MaxItemC>=MaxItemC)?MaxItemC:Source.MaxItemC;
    for(int RowIndex=0; RowIndex<RowCount; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<ColCount; ColIndex++)
            CopySingleItem(Source, RowIndex, ColIndex);
    }
}
//---------------------------------------------------------------------------
void TMyKitSuck::MoveSuckData(TMyKitSuck &Source, int RowIndex, int ColIndex)
{
    MoveSingleItem(Source, RowIndex, ColIndex);
}
//---------------------------------------------------------------------------
void TMyKitSuck::MoveSuckDataDiff(TMyKitSuck &Source, int SourceRowIndex, int SourceColIndex, int TargetRowIndex, int TargetColIndex)
{
    if(SourceRowIndex<0 || SourceRowIndex>=Source.MaxItemR ||
       SourceColIndex<0 || SourceColIndex>=Source.MaxItemC ||
       TargetRowIndex<0 || TargetRowIndex>=MaxItemR ||
       TargetColIndex<0 || TargetColIndex>=MaxItemC)
        return;
    SetItemData(TargetRowIndex, TargetColIndex, Source.Suck[SourceRowIndex][SourceColIndex].Item);
    Suck[TargetRowIndex][TargetColIndex].DeviceInfo.MoveFrom(Source.Suck[SourceRowIndex][SourceColIndex].DeviceInfo);
    Source.SetItemData(SourceRowIndex, SourceColIndex, SuckNullIC);
}
//---------------------------------------------------------------------------
bool TMyKitSuck::AllDeviceTested()
{
    for(int RowIndex=0; RowIndex<MaxItemR; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<MaxItemC; ColIndex++)
        {
            if(Suck[RowIndex][ColIndex].Item<SuckTestPass)
                return false;
        }
    }
    return true;
}
//---------------------------------------------------------------------------
void TMyKitSuck::ResetAll()
{
    for(int RowIndex=0; RowIndex<MaxItemR; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<MaxItemC; ColIndex++)
            Suck[RowIndex][ColIndex].ResetSuckTask();
    }
}
//---------------------------------------------------------------------------
void TMyKitSuck::ClearAllError()
{
    for(int RowIndex=0; RowIndex<MaxItemR; RowIndex++)
    {
        for(int ColIndex=0; ColIndex<MaxItemC; ColIndex++)
            Suck[RowIndex][ColIndex].Error=false;
    }
}
//---------------------------------------------------------------------------
void TMyKitSuck::CheckVaccumIsIniaialON(int RowIndex, int ColIndex, bool &Flag)
{
    if(RowIndex<0 || RowIndex>=MaxItemR || ColIndex<0 || ColIndex>=MaxItemC)
        return;

    if(Suck[RowIndex][ColIndex].GetStatus())
    {
        if(Suck[RowIndex][ColIndex].Item==SuckNullIC ||
           Suck[RowIndex][ColIndex].Item==SuckHasNullIC ||
           Suck[RowIndex][ColIndex].Item==SuckNullICTest ||
           Suck[RowIndex][ColIndex].Item==SuckHasNullICTest)
        {
            Suck[RowIndex][ColIndex].Normal();
            Flag=true;
        }
    }
    else
    {
        Suck[RowIndex][ColIndex].Normal();
    }
}
//---------------------------------------------------------------------------
void TMyKitSuck::ClearBinData()
{
    for(int Index=0; Index<MAX_SUB_SUCKER_ITEM; Index++)
    {
        iBinData[Index]=999;
        sprintf(cBinInfor[Index], "");
    }
}
//---------------------------------------------------------------------------
