//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "UserRoleManager.h"
#include "MachineType.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
//---------------------------------------------------------------------------
THT160UserRoleManager UserRoleManager;
//---------------------------------------------------------------------------
void THT160UserRoleRecord::Clear()
{
    sUserID="";
    sPassword="";
    iLevel=ROLE_OPERATION;
    bEnabled=false;
}
//---------------------------------------------------------------------------
__fastcall THT160UserRoleManager::THT160UserRoleManager()
{
    m_iLevel=ROLE_OPERATION;
    m_sUserID="Operation";
    m_tLoginTime=Now();
    m_bManualOperation=false;
    ClearUsers();
    InitializeByBuildMode();
}
//---------------------------------------------------------------------------
int THT160UserRoleManager::NormalizeLevel(int iLevel) const
{
    if(iLevel<ROLE_OPERATION)
        return ROLE_OPERATION;
    if(iLevel>ROLE_HONPREC)
        return ROLE_HONPREC;
    return iLevel;
}
//---------------------------------------------------------------------------
bool THT160UserRoleManager::IsValidLevel(int iLevel)
{
    return (iLevel>=ROLE_OPERATION && iLevel<=ROLE_HONPREC);
}
//---------------------------------------------------------------------------
AnsiString THT160UserRoleManager::GetLevelName(int iLevel)
{
    switch(iLevel)
    {
        case ROLE_SUPERVISOR:
            return "Supervisor";
        case ROLE_ENGINEER:
            return "Engineer";
        case ROLE_HONPREC:
            return "Honprec";
        case ROLE_OPERATION:
        default:
            return "Operation";
    }
}
//---------------------------------------------------------------------------
void THT160UserRoleManager::InitializeByBuildMode()
{
    #ifdef SOFT_SIMULATE
    if(!m_bManualOperation)
        ForceLevel(ROLE_HONPREC, "Honprec");
    #else
    SetUserToOperation(false);
    #endif
}
//---------------------------------------------------------------------------
void THT160UserRoleManager::SetUserToOperation(bool bManual)
{
    m_iLevel=ROLE_OPERATION;
    m_sUserID="Operation";
    m_tLoginTime=Now();
    m_bManualOperation=bManual;
}
//---------------------------------------------------------------------------
bool THT160UserRoleManager::ForceLevel(int iLevel, AnsiString sUserID)
{
    if(!IsValidLevel(iLevel))
        return false;

    m_iLevel=iLevel;
    m_sUserID=sUserID.Trim();
    if(m_sUserID==AnsiString(""))
        m_sUserID=GetLevelName(iLevel);
    m_tLoginTime=Now();
    if(iLevel!=ROLE_OPERATION)
        m_bManualOperation=false;
    return true;
}
//---------------------------------------------------------------------------
bool THT160UserRoleManager::Login(int iLevel, AnsiString sUserID, AnsiString sPassword)
{
    if(iLevel==ROLE_OPERATION)
    {
        SetUserToOperation(true);
        return true;
    }

    if(!CheckPassword(iLevel, sUserID, sPassword))
        return false;
    return ForceLevel(iLevel, sUserID);
}
//---------------------------------------------------------------------------
bool THT160UserRoleManager::HasLevel(int iRequiredLevel) const
{
    return (m_iLevel>=NormalizeLevel(iRequiredLevel));
}
//---------------------------------------------------------------------------
int THT160UserRoleManager::GetLevel() const
{
    return m_iLevel;
}
//---------------------------------------------------------------------------
AnsiString THT160UserRoleManager::GetUserID() const
{
    return m_sUserID;
}
//---------------------------------------------------------------------------
AnsiString THT160UserRoleManager::GetLevelName() const
{
    return GetLevelName(m_iLevel);
}
//---------------------------------------------------------------------------
TDateTime THT160UserRoleManager::GetLoginTime() const
{
    return m_tLoginTime;
}
//---------------------------------------------------------------------------
bool THT160UserRoleManager::IsManualOperation() const
{
    return m_bManualOperation;
}
//---------------------------------------------------------------------------
bool THT160UserRoleManager::IsSimulationDefault() const
{
    #ifdef SOFT_SIMULATE
    return (!m_bManualOperation && m_iLevel==ROLE_HONPREC);
    #else
    return false;
    #endif
}
//---------------------------------------------------------------------------
void THT160UserRoleManager::ClearUsers()
{
    int i;
    for(i=0; i<HT160_USER_ROLE_MAX_COUNT; i++)
        m_Users[i].Clear();
    m_iUserCount=0;
}
//---------------------------------------------------------------------------
int THT160UserRoleManager::FindUser(AnsiString sUserID, int iLevel) const
{
    AnsiString sFind=sUserID.Trim().UpperCase();
    int i;

    if(sFind==AnsiString(""))
        return -1;
    for(i=0; i<m_iUserCount; i++)
    {
        if(!m_Users[i].bEnabled)
            continue;
        if(m_Users[i].iLevel!=iLevel)
            continue;
        if(m_Users[i].sUserID.Trim().UpperCase()==sFind)
            return i;
    }
    return -1;
}
//---------------------------------------------------------------------------
bool THT160UserRoleManager::AddOrUpdateUser(AnsiString sUserID, AnsiString sPassword, int iLevel)
{
    int iIndex;

    if(!IsValidLevel(iLevel))
        return false;
    if(sUserID.Trim()==AnsiString(""))
        return false;

    iIndex=FindUser(sUserID, iLevel);
    if(iIndex<0)
    {
        if(m_iUserCount>=HT160_USER_ROLE_MAX_COUNT)
            return false;
        iIndex=m_iUserCount;
        m_iUserCount++;
    }

    m_Users[iIndex].sUserID=sUserID.Trim();
    m_Users[iIndex].sPassword=sPassword;
    m_Users[iIndex].iLevel=iLevel;
    m_Users[iIndex].bEnabled=true;
    return true;
}
//---------------------------------------------------------------------------
bool THT160UserRoleManager::DeleteUser(AnsiString sUserID, int iLevel)
{
    int iIndex=FindUser(sUserID, iLevel);
    int i;

    if(iIndex<0)
        return false;

    for(i=iIndex; i<m_iUserCount-1; i++)
        m_Users[i]=m_Users[i+1];
    m_iUserCount--;
    m_Users[m_iUserCount].Clear();
    return true;
}
//---------------------------------------------------------------------------
bool THT160UserRoleManager::CheckPassword(int iLevel, AnsiString sUserID, AnsiString sPassword) const
{
    int iIndex;

    if(!IsValidLevel(iLevel))
        return false;
    iIndex=FindUser(sUserID, iLevel);
    if(iIndex<0)
        return false;
    return (m_Users[iIndex].sPassword==sPassword);
}
//---------------------------------------------------------------------------
int THT160UserRoleManager::GetUserCount() const
{
    return m_iUserCount;
}
//---------------------------------------------------------------------------
AnsiString THT160UserRoleManager::GetUserID(int iIndex) const
{
    if(iIndex<0 || iIndex>=m_iUserCount)
        return "";
    return m_Users[iIndex].sUserID;
}
//---------------------------------------------------------------------------
int THT160UserRoleManager::GetUserLevel(int iIndex) const
{
    if(iIndex<0 || iIndex>=m_iUserCount)
        return ROLE_OPERATION;
    return m_Users[iIndex].iLevel;
}
//---------------------------------------------------------------------------
bool THT160UserRoleManager::IsUserEnabled(int iIndex) const
{
    if(iIndex<0 || iIndex>=m_iUserCount)
        return false;
    return m_Users[iIndex].bEnabled;
}
//---------------------------------------------------------------------------