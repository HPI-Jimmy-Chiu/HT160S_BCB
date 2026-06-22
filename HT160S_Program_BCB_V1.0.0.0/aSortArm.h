//---------------------------------------------------------------------------
#ifndef aSortArmH
#define aSortArmH
//---------------------------------------------------------------------------
#include <Classes.hpp>
//---------------------------------------------------------------------------
struct TSortArmSlotState
{
    bool bCanPick;
    bool bHasIC;
    bool bPlaceSelected;
    int PickX;
    int PickY;
    int PlaceX;
    int PlaceY;
    int TrayData;
    int BinValue;
    int LotIndex;        //AI(ht160s-lotbin) 20260615 : owning LotIndex (By Lot+Bin routing key)
    AnsiString Code2D;   //AI(ht160s-lotbin) 20260615 : IC 2D code (Production_Log trace)
};
//---------------------------------------------------------------------------
class TTrayMotor;
class TMySucker;
//---------------------------------------------------------------------------
class TSortArmModule
{
private:
    int PickTask;
    int PlaceTask;
    int iActiveLoaderNo;
    int iActiveAutoIndex;
    int iPlaceBaseX;
    int iPlaceY;
    bool bCleanOutFinish;   //AI(HT160S-Maintainer) 20260605 : SortArm drained in CleanOut
    bool bOneCycleFinish;   //AI(HT160S-Maintainer) 20260605 : SortArm placed held IC then stopped (OneCycle)
    TSortArmSlotState Slot[4];
    unsigned int dwSuckHomeLostStart;   //AI(HT160S-Maintainer) 20260622 : SortArmX suck-home loss debounce (GetTickCount of first loss; 0=clear)

    void ClearSlot(int SlotIndex);
    void ClearPickSelection();
    void ClearPlaceSelection();
    void UpdateKitSuckState();
    bool IsSoftSimulate();
    bool IsPickableData(int Data);

    TTrayMotor *GetLoaderMotor(int LoaderNo);
    TTrayMotor *GetLoaderVMotor(int LoaderNo);
    TTrayMotor *GetAutoMotor(int AutoIndex);
    TTrayMotor *GetAutoVMotor(int AutoIndex);
    TTrayMotor *GetSuckZMotor(int SlotIndex);
    TMySucker *GetSucker(int SlotIndex);

    int GetTrayXCount();
    int GetTrayYCount();
    double GetTrayXPitch();
    double GetTrayYPitch();
    int RoundPosition(double Value);
    int CalculatePitchPosition();

    int GetLoaderSortX(int LoaderNo);
    int GetLoaderFirstSortY(int LoaderNo);
    int GetLoaderZPosition(int LoaderNo, int SlotIndex);
    int GetAutoSortX(int AutoIndex);
    int GetAutoFirstSortY(int AutoIndex);
    int GetAutoZPosition(int AutoIndex, int SlotIndex);

    bool MoveSortArmX(int Position);
    bool MoveLoaderY(int LoaderNo, int Position);
    bool MoveAutoY(int AutoIndex, int Position);
    bool MovePitchToTrayPitch();
    bool SortArmZToSafePos();
    bool MoveToLoaderPick();
    bool MoveToAutoPlace();
    bool MovePickZDown();
    bool MovePlaceZDown();
    void ShowPlaceDebugInfo();   //AI(general) 20260609 : place position check (flag-gated)

    bool FindPickCells(int LoaderNo);
    bool SelectPlaceAuto();
    bool FindPlaceCells(int AutoIndex);
    int GetSlotRoutingBin(int SlotIndex);
    int GetMappedAutoIndex(int BinData, int LotIndex, bool &bFixedArea);
    bool CanPlaceSlotToAuto(int SlotIndex, int AutoIndex);

    bool SuckSelectedSlots();
    bool DestroySelectedSlots();
    void TransferPickDataFromLoader();
    void TransferPlaceDataToAuto();

    bool DoPickFromLoader(int Flag);
    bool DoPlaceToAuto(int Flag);

public:
    TSortArmModule();
    void InitialFlag(bool bKeepMaterial=false);
    bool AreAllSuckersHome();   //AI(HT160S-Maintainer) 20260622 : canonical SortArm-move suck-home interlock (live Led[iHomeLed])
    void DoSortArm(int &Task);
    bool HasHoldingIC();
    bool IsCleanOutFinish();   //AI(HT160S-Maintainer) 20260605 : SortArm CleanOut finish
    bool IsOneCycleFinish();   //AI(HT160S-Maintainer) 20260605 : SortArm OneCycle finish
    int  GetPickTask();        //AI(ht160s-state-record-analysis) 20260612 : sub-task readout for Store Hangup snapshot
    int  GetPlaceTask();       //AI(ht160s-state-record-analysis) 20260612 : sub-task readout for Store Hangup snapshot
    AnsiString DescribeHolding();   //AI(ht160s-state-record-analysis) 20260616 : read-only held-IC + routing dump for SortArmDecision.txt
    bool MoveSuckerToCell(int SlotIndex, int Target, int Col, int Row, bool bZDown, int &Task);   //AI(ht160s-sortarm-flow) 20260617 : Teach Advanced single-nozzle point test (Target 1=Loader1,2=Loader2,11..16=Auto1..6; Col/Row 0-based)
    bool CanMoveSuckerToCell(int SlotIndex, int Target, int Col, int Row, AnsiString &Err);        //AI(ht160s-sortarm-flow) 20260617 : pre-move validation for the point test
};
//---------------------------------------------------------------------------
extern TSortArmModule *SortArmModule;
void InitializeSortArmModule();
void ShutdownSortArmModule();
//---------------------------------------------------------------------------
#endif