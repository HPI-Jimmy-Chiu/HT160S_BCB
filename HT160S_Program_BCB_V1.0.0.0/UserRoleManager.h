//---------------------------------------------------------------------------
#ifndef UserRoleManagerH
#define UserRoleManagerH
//---------------------------------------------------------------------------
#include <System.hpp>
//---------------------------------------------------------------------------
enum EHT160UserRoleLevel
{
    ROLE_OPERATION  = 0,
    ROLE_SUPERVISOR = 1,
    ROLE_ENGINEER   = 2,
    ROLE_HONPREC    = 3
};
//---------------------------------------------------------------------------
#define HT160_USER_ROLE_MAX_COUNT 30
//---------------------------------------------------------------------------
struct THT160UserRoleRecord
{
    AnsiString sUserID;
    AnsiString sPassword;
    int iLevel;
    bool bEnabled;

    void Clear();
};
//---------------------------------------------------------------------------
class THT160UserRoleManager
{
private:
    int m_iLevel;
    AnsiString m_sUserID;
    TDateTime m_tLoginTime;
    bool m_bManualOperation;
    bool m_bServiceMaster;
    THT160UserRoleRecord m_Users[HT160_USER_ROLE_MAX_COUNT];
    int m_iUserCount;
    AnsiString m_sLoadDuplicateIDs;

    int NormalizeLevel(int iLevel) const;
    int FindUser(AnsiString sUserID) const;
    bool IsHourMasterCredential(AnsiString sUserID, AnsiString sPassword) const;
    bool IsServiceMasterCredential(AnsiString sUserID, AnsiString sPassword) const;

public:
    __fastcall THT160UserRoleManager();

    void InitializeByBuildMode();
    void SetUserToOperation(bool bManual);
    bool ForceLevel(int iLevel, AnsiString sUserID);
    bool Login(int iLevel, AnsiString sUserID, AnsiString sPassword);
    bool HasLevel(int iRequiredLevel) const;

    int GetLevel() const;
    AnsiString GetUserID() const;
    AnsiString GetLevelName() const;
    TDateTime GetLoginTime() const;
    bool IsManualOperation() const;
    bool IsSimulationDefault() const;
    bool IsServiceMasterSession() const;

    void ClearUsers();
    bool AddOrUpdateUser(AnsiString sUserID, AnsiString sPassword, int iLevel);
    bool DeleteUser(AnsiString sUserID);
    bool CheckPassword(AnsiString sUserID, AnsiString sPassword) const;
    int FindUserLevel(AnsiString sUserID) const;   //AI(ht160s-security) 20260911 : the book row's level IS the level (HT9045 book-path model); -1 when the ID is not in the book
    bool LoadFromFile(AnsiString FileName);
    bool SaveToFile(AnsiString FileName);

    int GetUserCount() const;
    AnsiString GetLoadDuplicateIDs() const;   //AI(ht160s-security) 20260911 : IDs whose 2nd+ row LoadFromFile dropped (ID is the key now)
    AnsiString GetUserID(int iIndex) const;
    int GetUserLevel(int iIndex) const;
    bool IsUserEnabled(int iIndex) const;

    static bool IsValidLevel(int iLevel);
    static AnsiString GetLevelName(int iLevel);
};
//---------------------------------------------------------------------------
extern THT160UserRoleManager UserRoleManager;
//---------------------------------------------------------------------------
#endif