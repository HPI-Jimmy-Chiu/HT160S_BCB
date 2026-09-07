//---------------------------------------------------------------------------
// ICommPort.h
// AI(ht160s-bindisplay) 20260624 : runtime-selectable serial backend for the bin
// display COM port. ICommPort is a thin abstraction over the legacy SPComm TComm
// and the self-built TMyComm so the maintenance cbCommType checkbox can switch
// between them at run time (unchecked = SPComm, the proven path; checked =
// TMyComm). Header-only: only ComPort.cpp and MyBinDisp.cpp include it.
// Encoding: ASCII only - all comments in English.
//---------------------------------------------------------------------------
#ifndef ICommPortH
#define ICommPortH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include "SPComm.hpp"   // TComm; ends with "using namespace Spcomm" (None/_8/_1..)
#include "MyComm.h"     // TMyComm + cpNone/cbs8/csb1 enums
//---------------------------------------------------------------------------
// Shared OnReceiveData closure. Identical signature to both SPComm's
// Spcomm::TReceiveDataEvent and TMyComm's TCommReceiveDataEvent; assigned to each
// backend with a C-style cast because BCB6 treats the distinct closure typedefs
// as different types even though their byte layout is the same. The existing
// handler TMyBinDispCtrl::CommBinReceiveData(TObject*, Pointer, WORD) binds here
// because Pointer == void* and WORD == Word.
typedef void __fastcall (__closure *TBinRxEvent)
    (System::TObject* Sender, void* Buffer, Word BufferLength);
//---------------------------------------------------------------------------
// Abstract serial port. Methods cover exactly the surface the bin display uses.
//---------------------------------------------------------------------------
class ICommPort
{
public:
    virtual ~ICommPort() {}
    virtual void       SetCommName(AnsiString Name) = 0;
    virtual AnsiString GetCommName() = 0;
    virtual void       SetBaudRate(int Baud) = 0;
    virtual void       SetParity(int DcbParity) = 0;     // 0..4 NOPARITY..SPACEPARITY
    virtual void       SetByteSize(int Bits) = 0;        // 5..8
    virtual void       SetStopBits(int StopMode) = 0;    // 0=1bit, 1=1.5bit, 2=2bit
    virtual void       SetParityCheck(bool On) = 0;
    virtual void       SetReadIntervalTimeout(int Ms) = 0;
    virtual void       SetOnReceiveData(TBinRxEvent Handler) = 0;
    virtual void       StartComm() = 0;
    virtual void       StopComm() = 0;
    virtual int        WriteCommData(const char* Buffer, int Length) = 0;
    virtual int        GetKind() = 0;                    // 0 = SPComm, 1 = TMyComm
};
//---------------------------------------------------------------------------
// SPComm backend. Owns its TComm (Owner = NULL; deleted in the dtor) so a single
// deterministic owner frees it -- the wrapping adapter, never the VCL Owner sweep.
//---------------------------------------------------------------------------
class TCommPortSP : public ICommPort
{
private:
    TComm* FComm;
public:
    TCommPortSP(TComponent* /*OwnerIgnored*/) { FComm = new TComm((TComponent*)NULL); }
    virtual ~TCommPortSP() { try { FComm->StopComm(); } catch(...) {} delete FComm; }

    virtual void SetCommName(AnsiString Name) { FComm->CommName = Name; }
    virtual AnsiString GetCommName() { return FComm->CommName; }
    virtual void SetBaudRate(int Baud) { FComm->BaudRate = Baud; }
    virtual void SetParity(int DcbParity)
    {
        switch(DcbParity)
        {
            case 1:  FComm->Parity = Spcomm::Odd;   break;
            case 2:  FComm->Parity = Spcomm::Even;  break;
            case 3:  FComm->Parity = Spcomm::Mark;  break;
            case 4:  FComm->Parity = Spcomm::Space; break;
            default: FComm->Parity = Spcomm::None;  break;
        }
    }
    virtual void SetByteSize(int Bits)
    {
        switch(Bits)
        {
            case 5:  FComm->ByteSize = _5; break;
            case 6:  FComm->ByteSize = _6; break;
            case 7:  FComm->ByteSize = _7; break;
            default: FComm->ByteSize = _8; break;
        }
    }
    virtual void SetStopBits(int StopMode)
    {
        switch(StopMode)
        {
            case 1:  FComm->StopBits = _1_5; break;
            case 2:  FComm->StopBits = _2;   break;
            default: FComm->StopBits = _1;   break;
        }
    }
    virtual void SetParityCheck(bool On) { FComm->ParityCheck = On; }
    virtual void SetReadIntervalTimeout(int Ms) { FComm->ReadIntervalTimeout = (unsigned)Ms; }
    virtual void SetOnReceiveData(TBinRxEvent Handler) { FComm->OnReceiveData = (TReceiveDataEvent)Handler; }
    virtual void StartComm() { FComm->StartComm(); }
    virtual void StopComm()  { FComm->StopComm(); }
    virtual int  WriteCommData(const char* Buffer, int Length)
    { return FComm->WriteCommData((char*)Buffer, (Word)Length) ? Length : 0; }
    virtual int  GetKind() { return 0; }
};
//---------------------------------------------------------------------------
// TMyComm backend. Owns its TMyComm (Owner = NULL; deleted in the dtor).
//---------------------------------------------------------------------------
class TCommPortMy : public ICommPort
{
private:
    TMyComm* FComm;
public:
    TCommPortMy(TComponent* /*OwnerIgnored*/)
    {
        FComm = new TMyComm((TComponent*)NULL);
        FComm->SyncReceive = true;   // OnReceiveData marshalled to the main VCL thread
        FComm->EnableLog   = true;   // LogFileName left empty here; set it to enable
                                     // the wire-level RX/TX file log when needed.
    }
    virtual ~TCommPortMy() { try { FComm->StopComm(); } catch(...) {} delete FComm; }

    // StartComport prepends "\\.\" to the port name AND TMyComm::NormalizeName
    // re-adds it; strip a leading "\\.\" here so the device path is not doubled.
    virtual void SetCommName(AnsiString Name)
    {
        if(Name.Length() >= 4 && Name.SubString(1, 4) == "\\\\.\\")
            Name = Name.SubString(5, Name.Length() - 4);
        FComm->CommName = Name;
    }
    virtual AnsiString GetCommName() { return FComm->CommName; }
    virtual void SetBaudRate(int Baud) { FComm->BaudRate = Baud; }
    virtual void SetParity(int DcbParity) { FComm->Parity = (TCommParity)DcbParity; }
    virtual void SetByteSize(int Bits) { FComm->ByteSize = (TCommByteSize)Bits; }
    virtual void SetStopBits(int StopMode) { FComm->StopBits = (TCommStopBits)StopMode; }
    virtual void SetParityCheck(bool On) { FComm->ParityCheck = On; }
    virtual void SetReadIntervalTimeout(int /*Ms*/) { /* TMyComm has no ReadIntervalTimeout */ }
    virtual void SetOnReceiveData(TBinRxEvent Handler) { FComm->OnReceiveData = (TCommReceiveDataEvent)Handler; }
    virtual void StartComm() { FComm->StartComm(); }
    virtual void StopComm()  { FComm->StopComm(); }
    virtual int  WriteCommData(const char* Buffer, int Length) { return FComm->WriteCommData(Buffer, Length); }
    virtual int  GetKind() { return 1; }
};
//---------------------------------------------------------------------------
#endif
