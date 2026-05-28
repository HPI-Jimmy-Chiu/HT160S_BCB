//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "MC88X1P_DLL.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
static HMODULE g_MC88X1Dll=NULL;
static bool g_MC88X1DllTried=false;
//---------------------------------------------------------------------------
static bool LoadMC88X1Dll()
{
    UINT OldErrorMode;
    if(g_MC88X1Dll!=NULL)
        return true;
    if(g_MC88X1DllTried)
        return false;
    g_MC88X1DllTried=true;
    OldErrorMode=SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
    g_MC88X1Dll=LoadLibrary("MC88X1P_DLL.dll");
    SetErrorMode(OldErrorMode);
    return g_MC88X1Dll!=NULL;
}
//---------------------------------------------------------------------------
static FARPROC GetMC88X1Proc(const char *ProcName)
{
    if(!LoadMC88X1Dll())
        return NULL;
    return GetProcAddress(g_MC88X1Dll, ProcName);
}
//---------------------------------------------------------------------------
#define MC88X1_CALL_LRESULT(FuncName, TypeName, ParamList, ArgList) \
typedef LRESULT (__stdcall *TypeName) ParamList; \
LRESULT __stdcall FuncName ParamList \
{ \
    FARPROC Proc=GetMC88X1Proc(#FuncName); \
    if(Proc==NULL) \
        return ERROR_FILE_NOT_FOUND; \
    return ((TypeName)Proc) ArgList; \
}

#define MC88X1_CALL_BOOL(FuncName, TypeName, ParamList, ArgList) \
typedef BOOL (__stdcall *TypeName) ParamList; \
BOOL __stdcall FuncName ParamList \
{ \
    FARPROC Proc=GetMC88X1Proc(#FuncName); \
    if(Proc==NULL) \
        return FALSE; \
    return ((TypeName)Proc) ArgList; \
}

#ifdef __cplusplus
extern "C" {
#endif

MC88X1_CALL_LRESULT(MC88X1PMotDevOpen, TMC88X1PMotDevOpen, (BYTE board_no), (board_no))
MC88X1_CALL_LRESULT(MC88X1PMotDevClose, TMC88X1PMotDevClose, (BYTE board_no), (board_no))
MC88X1_CALL_LRESULT(MC88X1PMotReset, TMC88X1PMotReset, (BYTE board_no), (board_no))
MC88X1_CALL_LRESULT(MC88X1PMotDI, TMC88X1PMotDI, (BYTE board_no, BYTE axis, BYTE* ret), (board_no, axis, ret))
MC88X1_CALL_LRESULT(MC88X1PMotDO, TMC88X1PMotDO, (BYTE board_no, BYTE axis, BYTE value), (board_no, axis, value))
MC88X1_CALL_LRESULT(MC88X1PMotAxisBusy, TMC88X1PMotAxisBusy, (BYTE board_no, BYTE axis), (board_no, axis))
MC88X1_CALL_LRESULT(MC88X1PMotChgDV, TMC88X1PMotChgDV, (BYTE board_no, BYTE axis, DWORD spd_x, DWORD spd_y, DWORD spd_z, DWORD spd_u, DWORD spd_a, DWORD spd_b, DWORD spd_c, DWORD spd_d), (board_no, axis, spd_x, spd_y, spd_z, spd_u, spd_a, spd_b, spd_c, spd_d))
MC88X1_CALL_LRESULT(MC88X1PMotCmove, TMC88X1PMotCmove, (BYTE board_no, BYTE axis, BYTE dir), (board_no, axis, dir))
MC88X1_CALL_LRESULT(MC88X1PMotStop, TMC88X1PMotStop, (BYTE board_no, BYTE axis, BYTE mode), (board_no, axis, mode))
MC88X1_CALL_LRESULT(MC88X1PMotPtp, TMC88X1PMotPtp, (BYTE board_no, BYTE axis, BYTE ra, LONG pulse_x, LONG pulse_y, LONG pulse_z, LONG pulse_u, LONG pos_a, LONG pos_b, LONG pos_c, LONG pos_d), (board_no, axis, ra, pulse_x, pulse_y, pulse_z, pulse_u, pos_a, pos_b, pos_c, pos_d))
MC88X1_CALL_LRESULT(MC88X1PMotAxisParaSet, TMC88X1PMotAxisParaSet, (BYTE board_no, BYTE axis, BYTE ts, DWORD sv, DWORD dv, DWORD mdv, DWORD ac, DWORD ak), (board_no, axis, ts, sv, dv, mdv, ac, ak))
MC88X1_CALL_LRESULT(MC88X1PGetMotionInput, TMC88X1PGetMotionInput, (BYTE board_no, BYTE axis, USHORT *ret), (board_no, axis, ret))
MC88X1_CALL_LRESULT(MC88X1PSetPulseMode, TMC88X1PSetPulseMode, (BYTE board_no, BYTE axis, BYTE pulse_mode), (board_no, axis, pulse_mode))
MC88X1_CALL_LRESULT(MC88X1PSetTheorecticalRegister, TMC88X1PSetTheorecticalRegister, (BYTE board_no, BYTE axis, LONG value), (board_no, axis, value))
MC88X1_CALL_LRESULT(MC88X1PGetTheorecticalRegister, TMC88X1PGetTheorecticalRegister, (BYTE board_no, BYTE axis, LONG *retval), (board_no, axis, retval))
MC88X1_CALL_LRESULT(MC88X1PSetPracticalRegister, TMC88X1PSetPracticalRegister, (BYTE board_no, BYTE axis, LONG value), (board_no, axis, value))
MC88X1_CALL_LRESULT(MC88X1PGetPracticalRegister, TMC88X1PGetPracticalRegister, (BYTE board_no, BYTE axis, LONG *retval), (board_no, axis, retval))
MC88X1_CALL_LRESULT(MC88X1PSetEncoderDir, TMC88X1PSetEncoderDir, (BYTE board_no, BYTE axis, USHORT value), (board_no, axis, value))
MC88X1_CALL_LRESULT(MC88X1PSetEncoderMultiple, TMC88X1PSetEncoderMultiple, (BYTE board_no, BYTE axis, USHORT value), (board_no, axis, value))
MC88X1_CALL_LRESULT(MC88X1PMotWrReg, TMC88X1PMotWrReg, (BYTE board_no, BYTE axis, WORD port, LONG value), (board_no, axis, port, value))
MC88X1_CALL_LRESULT(MC88X1PMotHome, TMC88X1PMotHome, (BYTE board_no, BYTE axis), (board_no, axis))
MC88X1_CALL_LRESULT(MC88X1PMotHomeStatus, TMC88X1PMotHomeStatus, (BYTE board_no, BYTE axis), (board_no, axis))
MC88X1_CALL_LRESULT(MC88X1PMotHomeReset, TMC88X1PMotHomeReset, (BYTE board_no, BYTE axis), (board_no, axis))
MC88X1_CALL_LRESULT(MC88X1PMotIpReset, TMC88X1PMotIpReset, (BYTE board_no), (board_no))
MC88X1_CALL_LRESULT(MC88X1PEnableCompLimit, TMC88X1PEnableCompLimit, (BYTE board_no, BYTE axis, USHORT enable), (board_no, axis, enable))
MC88X1_CALL_LRESULT(MC88X1PSetCompNLimit, TMC88X1PSetCompNLimit, (BYTE board_no, BYTE axis, LONG value), (board_no, axis, value))
MC88X1_CALL_LRESULT(MC88X1PSetCompPLimit, TMC88X1PSetCompPLimit, (BYTE board_no, BYTE axis, LONG value), (board_no, axis, value))
MC88X1_CALL_LRESULT(MC88X1PSetHomeLogic, TMC88X1PSetHomeLogic, (BYTE board_no, BYTE axis, USHORT value), (board_no, axis, value))
MC88X1_CALL_LRESULT(MC88X1PSetServoAlarm, TMC88X1PSetServoAlarm, (BYTE board_no, BYTE axis, USHORT enable, USHORT value), (board_no, axis, enable, value))
MC88X1_CALL_LRESULT(MC88X1PSetInposition, TMC88X1PSetInposition, (BYTE board_no, BYTE axis, USHORT enable, USHORT value), (board_no, axis, enable, value))
MC88X1_CALL_LRESULT(MC88X1PSetNLimitLogic, TMC88X1PSetNLimitLogic, (BYTE board_no, BYTE axis, USHORT value), (board_no, axis, value))
MC88X1_CALL_LRESULT(MC88X1PSetPLimitLogic, TMC88X1PSetPLimitLogic, (BYTE board_no, BYTE axis, USHORT value), (board_no, axis, value))
MC88X1_CALL_LRESULT(MC88X1PSetPichPulseCounter, TMC88X1PSetPichPulseCounter, (BYTE board_no, BYTE axis, WORD value), (board_no, axis, value))
MC88X1_CALL_LRESULT(MC88X1PSetPichData, TMC88X1PSetPichData, (BYTE board_no, BYTE axis, WORD value), (board_no, axis, value))
MC88X1_CALL_LRESULT(MC88X1PSetPichPulseMode, TMC88X1PSetPichPulseMode, (BYTE board_no, BYTE axis, BYTE mode), (board_no, axis, mode))
MC88X1_CALL_LRESULT(MC88X1PMotLine, TMC88X1PMotLine, (BYTE board_no, BYTE axis, BYTE ra, LONG pos_x, LONG pos_y, LONG pos_z, LONG pos_u, LONG pos_a, LONG pos_b, LONG pos_c, LONG pos_d), (board_no, axis, ra, pos_x, pos_y, pos_z, pos_u, pos_a, pos_b, pos_c, pos_d))
MC88X1_CALL_BOOL(MC88X1PReadWord, TMC88X1PReadWord, (BYTE board_no, DWORD offset, WORD *retval), (board_no, offset, retval))
MC88X1_CALL_BOOL(MC88X1PWriteWord, TMC88X1PWriteWord, (BYTE board_no, DWORD offset, WORD val), (board_no, offset, val))

#ifdef __cplusplus
}
#endif
//---------------------------------------------------------------------------