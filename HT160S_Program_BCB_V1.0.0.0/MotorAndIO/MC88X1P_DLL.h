#ifndef __MC8841DLL_H
#define __MC8841DLL_H

#ifdef __cplusplus
#define MC88X1P_API extern "C"
#else
#define MC88X1P_API
#endif
																		// Hex			// Dec
#define BoardNumErr									0x0001		//	1
#define CreateKernelDriverFail					0x0002		//	2
#define CallKernelDriverFail						0x0003		//	3
#define AxisNumErr										0x0006		//	6	
#define AxisDrvBusy									0x0015		//	21
#define ParaValueErr									0x0017		//	23	
#define ParaValueOverRange						0x0018		//	24
#define ParaValueUnderRange					0x0019		//	25
#define AxisHomeBusy								0x001a		//	26
#define OpenEventFail									0x0030		//	48
#define FirewareCheckFail							0x0035		//	53	// MC8882P only
#define UnknowDevice								0x0040		// 64
#define AllocateBufferFail							0x0050		//	80
#define BufferReAllocate								0x0051		//	81
#define FreeBufferFail									0x0052		//	82
#define FirstPointNumberFail						0x0053		//	83
#define PointNumExceedAllocatedSize		0x0054		//	84
#define BufferNoneAllocate						0x0055		//	85
#define SequenceNumberErr						0x0056		//	86
#define PathTypeErr										0x0057		//	87
#define PathTypeMixErr								0x0060		//	96
#define BufferDataNotEnough					0x0061		//	97
#define StartCintiMoveTimeOut					0x0062		// 98

// Multi-Encoder Latch									// Hex			// Dec
#define EcLatchCCreateFail							0x0070		// 112
#define EcLatchTCreateFail							0x0071		// 113
#define EcLatchAlreadyEnabled					0x0072		// 114
#define EcLatchBufLenErr							0x0073		// 115
#define EcLatchPointerErr							0x0074		// 116
#define EcLatchAlreadyDisabled				0x0076		// 118
#define EcLatchNotEnable							0x0077		// 119
#define EcLatchTrigTimeErr						0x0078		// 120

// 2D Spiral													// Hex			// Dec
#define SpiralWorking									0x0080		// 128

// Suspension
#define InSuspension									0x0090		//
#define NoSuspension									0x0091		//
#define NoCmdSusp									0x0092		//					

//****************************************
#define PulseModeErr							0x0301
#define AxisHomeErr							0x0302
#define UnknownErr							0x0303
#define AxisIpBusy								0x0304
#define ArcDataErr								0x0305
#define SignalErr									0x0306
#define AngleErr									0x0307
#define OutPtnErr									0x0308
#define IpNotSupport							0x0309
#define IpHomeErr								0x030a
#define IpErr											0x030b
#define IpPgErr										0x030c
#define CorrectBacklashBusy				0x030d
//****************************************
#define X1_SV_UnderRange				0x1000
#define X1_SV_OverRange					0x1001
#define X1_DV_UnderRange				0x1002
#define X1_DV_OverRange					0x1003
#define X1_MDV_UnderRange			0x1004
#define X1_MDV_OverRange				0x1005
#define X1_AC_UnderRange				0x1006
#define X1_AC_OverRange					0x1007
#define X1_SCW1_UnderRang			0x1008
#define X1_SCW1_OverRange			0x1009
//****************************************
#define Y1_SV_UnderRange				0x1020
#define Y1_SV_OverRange					0x1021
#define Y1_DV_UnderRange				0x1022
#define Y1_DV_OverRange					0x1023
#define Y1_MDV_UnderRange			0x1024
#define Y1_MDV_OverRange				0x1025
#define Y1_AC_UnderRange				0x1026
#define Y1_AC_OverRange					0x1027
#define Y1_SCW1_UnderRange			0x1028
#define Y1_SCW1_OverRange			0x1029
//****************************************
#define Z1_SV_UnderRange				0x1040
#define Z1_SV_OverRange					0x1041
#define Z1_DV_UnderRange				0x1042
#define Z1_DV_OverRange					0x1043
#define Z1_MDV_UnderRange			0x1044
#define Z1_MDV_OverRange				0x1045
#define Z1_AC_UnderRange				0x1046
#define Z1_AC_OverRange					0x1047
#define Z1_SCW1_UnderRange			0x1048
#define Z1_SCW1_OverRange			0x1049
//****************************************
#define U1_SV_UnderRange				0x1060
#define U1_SV_OverRange					0x1061
#define U1_DV_UnderRange				0x1062
#define U1_DV_OverRange				0x1063
#define U1_MDV_UnderRange			0x1064
#define U1_MDV_OverRange				0x1065
#define U1_AC_UnderRange				0x1066
#define U1_AC_OverRange					0x1067
#define U1_SCW1_UnderRange		0x1068
#define U1_SCW1_OverRange			0x1069
//****************************************
#define X2_SV_UnderRange				0x1080
#define X2_SV_OverRange					0x1081
#define X2_DV_UnderRange				0x1082
#define X2_DV_OverRange					0x1083
#define X2_MDV_UnderRange			0x1084
#define X2_MDV_OverRange				0x1085
#define X2_AC_UnderRange				0x1086
#define X2_AC_OverRange					0x1087
#define X2_SCW1_UnderRange			0x1088
#define X2_SCW1_OverRange			0x1089
//****************************************
#define Y2_SV_UnderRange				0x10a0
#define Y2_SV_OverRange					0x10a1
#define Y2_DV_UnderRange				0x10a2
#define Y2_DV_OverRange					0x10a3
#define Y2_MDV_UnderRange			0x10a4
#define Y2_MDV_OverRange				0x10a5
#define Y2_AC_UnderRange				0x10a6
#define Y2_AC_OverRange					0x10a7
#define Y2_SCW1_UnderRange			0x10a8
#define Y2_SCW1_OverRange			0x10a9
//****************************************
#define Z2_SV_UnderRange				0x10c0
#define Z2_SV_OverRange					0x10c1
#define Z2_DV_UnderRange				0x10c2
#define Z2_DV_OverRange					0x10c3
#define Z2_MDV_UnderRange			0x10c4
#define Z2_MDV_OverRange				0x10c5
#define Z2_AC_UnderRange				0x10c6
#define Z2_AC_OverRange					0x10c7
#define Z2_SCW1_UnderRange			0x10c8
#define Z2_SCW1_OverRange			0x10c9
//****************************************
#define U2_SV_UnderRange				0x10e0
#define U2_SV_OverRange					0x10e1
#define U2_DV_UnderRange				0x10e2
#define U2_DV_OverRange				0x10e3
#define U2_MDV_UnderRange			0x10e4
#define U2_MDV_OverRange				0x10e5
#define U2_AC_UnderRange				0x10e6
#define U2_AC_OverRange					0x10e7
#define U2_SCW1_UnderRange		0x10e8
#define U2_SCW1_OverRange			0x10e9

#define Rcnt									0x0100
#define Lcnt									0x0101
#define Pcmp								0x0102
#define Ncmp								0x0102
#define HomeOffset						0x0309
#define HomeType						0x030b
#define HomeP0_Dir					0x030c
#define HomeP0_Speed				0x030d
#define HomeP1_Dir					0x030e
#define HomeP1_Speed				0x030f
#define HomeP2_Dir					0x0310
#define HomeOffset_Speed		0x0311
#define HomeLimitAlarm			0x0330
#define HomeClearPos				0x0331

#define IPO_L2				0x0030		// 48
#define IPO_L3				0x0031		// 49
#define IPO_CW			0x0032		// 50
#define IPO_CCW			0x0034		// 52
#define IPO_L4				0x0036		// 54
#define IPO_ARC3        0x0037		// 55

#define X_AXIS				0x1
#define Y_AXIS				0x2
#define Z_AXIS				0x4
#define U_AXIS				0x8
#define X2_AXIS			0x10
#define Y2_AXIS			0x20
#define Z2_AXIS			0x40
#define U2_AXIS			0x80
#define ALL_AXIS			0xff

// Device ID
#define DEV_MC8841P		0xA0F0
#define DEV_MC8880P		0xA802
#define DEV_MC8881P		0xA0F7
#define DEV_MC8882P		0xA805
#define DEV_MC8883P		0xA807

// Board Infomation
#define MAX_BOARD		16
#define MAX_LATCH		200

// For conti-interpolation
typedef struct {
	DWORD dwEndPoint_ax1;
	DWORD dwEndPoint_ax2;
	DWORD dwEndPoint_ax3;
	union {
		DWORD dwCenPoint_ax1;
		DWORD dwEndPoint_ax4; // new
	};
	DWORD dwCenPoint_ax2;
	DWORD dwPointNum;
	WORD wCommand;
	DWORD	dwPgSpd;
	WORD TempB;
	DWORD TempA;
} MotMoveBuffer, *LPMotMoveBuffer;

typedef struct {
	BYTE board_no;
	BYTE axis; // Axis pattern
	BYTE ra;
	LONG pos[8];
} LinePara, *LPLinePara;

typedef struct 
{
	WORD	Num;
	LONG		Data[100];
} TrigDataBase, *LpTrigDataBase;

// Latch for 8882P
typedef struct
{
	WORD TotLen;
	WORD Len[8];
	LONG Data[8][MAX_LATCH];
}LatDataBase, *pLatDataBase;


typedef struct
{
	// Device ID
	WORD DevID;
	// Software Version
	char SwVer[6];
	// Firmware Version
	char FwVer[6];
	// Reserve
	WORD RsvW[20];
}BoardInfo, *pBoardInfo;

typedef struct
{
	WORD BoardIdx;							// Board in use, displayed with bit format
	WORD Board[MAX_BOARD];		// Board device ID
}BoardCheck, *pBoardCheck;

typedef void (__stdcall *MC88X1P_EVENT_HANDLER)( BYTE board_No, void* );

// System Command
MC88X1P_API LRESULT __stdcall MC88X1PBoardCheck(pBoardCheck pCheck);
MC88X1P_API LRESULT __stdcall MC88X1PMotDevOpen( BYTE board_no );
MC88X1P_API LRESULT __stdcall MC88X1PMotDevClose( BYTE board_no );
MC88X1P_API LRESULT __stdcall MC88X1PGetBoardInfo(BYTE board_no , pBoardInfo pInfo);
MC88X1P_API LRESULT __stdcall MC88X1PMotGetTotalAxis( BYTE board_no, USHORT *ret );
MC88X1P_API LRESULT __stdcall MC88X1PMotReset( BYTE board_no );


// Interpolation
MC88X1P_API LRESULT __stdcall MC88X1PMotArc( BYTE board_no, BYTE move_axis, BYTE ra, BYTE direction, LONG center1, LONG center2, LONG end1, LONG end2 );           
MC88X1P_API LRESULT __stdcall MC88X1PMotArcTheta( BYTE board_no, BYTE move_axis, BYTE ra, BYTE direction, LONG center1, LONG center2, double degree );
MC88X1P_API LRESULT __stdcall MC88X1PMotArcThreePoints( BYTE board_no, BYTE move_axis, BYTE ra, LONG pass1, LONG pass2, LONG end1, LONG end2 );  
MC88X1P_API LRESULT __stdcall MC88X1PMotLine( BYTE board_no, BYTE axis, BYTE ra, LONG pos_x, LONG pos_y, LONG pos_z, LONG pos_u, LONG pos_a, LONG pos_b, LONG pos_c, LONG pos_d );
MC88X1P_API LRESULT __stdcall MC88X1PMotLines( LPLinePara para, int count );
MC88X1P_API LRESULT __stdcall MC88X1PMotEndlessLine( BYTE board_no, BYTE axis, BYTE ra, LONG pos_x, LONG pos_y, LONG pos_z, LONG pos_u, LONG pos_a, LONG pos_b, LONG pos_c, LONG pos_d );
MC88X1P_API LRESULT __stdcall MC88X1PMoveCircle( BYTE board_no, BYTE axis, BYTE ra, BYTE direction, LONG center1, LONG center2 );

// Conti-interpolation
MC88X1P_API LRESULT __stdcall MC88X1PInitialContiBuf( BYTE buf_id, DWORD point_count );
MC88X1P_API LRESULT __stdcall MC88X1PFreeContiBuf( BYTE buf_id );
MC88X1P_API LRESULT __stdcall MC88X1PSetContiData( BYTE buf_id, LPMotMoveBuffer path_data, DWORD point_num );
MC88X1P_API LRESULT __stdcall MC88X1PStartContiDrive( BYTE board_no, BYTE axis, BYTE buf_id );
MC88X1P_API LRESULT __stdcall MC88X1PStartContiDriveSC( BYTE board_no, BYTE axis, BYTE buf_id);
MC88X1P_API LRESULT __stdcall MC88X1PGetCurContiNum( BYTE board_no,  DWORD* current_number );

// Status command
MC88X1P_API LRESULT __stdcall MC88X1PMotDI( BYTE board_no, BYTE axis, BYTE* ret );
MC88X1P_API LRESULT __stdcall MC88X1PMotDO( BYTE board_no, BYTE axis, BYTE value );
MC88X1P_API LRESULT __stdcall MC88X1PMotAxisBusy( BYTE board_no, BYTE axis );
MC88X1P_API LRESULT __stdcall MC88X1PGetOutput( BYTE board_no, BYTE axis, BYTE *out_stat );

// Driving command
MC88X1P_API LRESULT __stdcall MC88X1PMotChgDV( BYTE board_no, BYTE axis, DWORD spd_x, DWORD spd_y, DWORD spd_z, DWORD spd_u, DWORD spd_a, DWORD spd_b, DWORD spd_c, DWORD spd_d );
MC88X1P_API LRESULT __stdcall MC88X1PMotCmove( BYTE board_no, BYTE axis, BYTE dir );
MC88X1P_API LRESULT __stdcall MC88X1PMotStop( BYTE board_no, BYTE axis, BYTE mode );
MC88X1P_API LRESULT __stdcall MC88X1PMotPtp( BYTE board_no, BYTE axis, BYTE ra, LONG pulse_x, LONG pulse_y, LONG pulse_z, LONG pulse_u, LONG pos_a, LONG pos_b, LONG pos_c, LONG pos_d );
 
MC88X1P_API LRESULT __stdcall MC88X1PMotAxisParaSet( BYTE board_no, BYTE axis, BYTE ts, DWORD sv, DWORD dv, DWORD mdv, DWORD ac, DWORD ak );
MC88X1P_API LRESULT __stdcall MC88X1PMotAxisParaSet2( BYTE board_no, BYTE axis, BYTE ts, DWORD sv, DWORD dv, DWORD mdv, float ac, DWORD ak );


// Staus & Input
MC88X1P_API LRESULT __stdcall MC88X1PGetMotionInput( BYTE board_no, BYTE axis, USHORT *ret );
MC88X1P_API LRESULT __stdcall MC88X1PGetMotionState( BYTE board_no, BYTE axis, USHORT *ret );
MC88X1P_API LRESULT __stdcall MC88X1PGetDriveStatus( BYTE board_no, BYTE axis, WORD *ret );

MC88X1P_API LRESULT __stdcall MC88X1PSetPulseMode( BYTE board_no, BYTE axis, BYTE pulse_mode );
MC88X1P_API LRESULT __stdcall MC88X1PSetTheorecticalRegister( BYTE board_no, BYTE axis, LONG value );
MC88X1P_API LRESULT __stdcall MC88X1PGetTheorecticalRegister( BYTE board_no, BYTE axis, LONG *retval );
MC88X1P_API LRESULT __stdcall MC88X1PSetPracticalRegister( BYTE board_no, BYTE axis, LONG value );
MC88X1P_API LRESULT __stdcall MC88X1PGetPracticalRegister( BYTE board_no, BYTE axis, LONG *retval );
MC88X1P_API LRESULT __stdcall MC88X1PSetEncoderDir( BYTE board_no, BYTE axis, USHORT value );
MC88X1P_API LRESULT __stdcall MC88X1PSetEncoderMultiple( BYTE board_no, BYTE axis, USHORT value );
MC88X1P_API LRESULT __stdcall MC88X1PMotExtModeDir( BYTE board_no, BYTE axis, USHORT value );
MC88X1P_API LRESULT __stdcall MC88X1PMotExtMode( BYTE board_no, BYTE axis, USHORT value );
MC88X1P_API LRESULT __stdcall MC88X1PMotExtDrive( BYTE board_no, BYTE axis, DWORD value );

MC88X1P_API LRESULT __stdcall MC88X1PMotWrReg( BYTE board_no, BYTE axis, WORD port, LONG value );
MC88X1P_API LRESULT __stdcall MC88X1PMotHome( BYTE board_no, BYTE axis );
MC88X1P_API LRESULT __stdcall MC88X1PMotHomeStatus( BYTE board_no, BYTE axis );
MC88X1P_API LRESULT __stdcall MC88X1PMotHomeReset( BYTE board_no, BYTE axis );
MC88X1P_API LRESULT __stdcall MC88X1PMotIpStatus( BYTE board_no, DWORD *status1, DWORD *status2 );
MC88X1P_API LRESULT __stdcall MC88X1PMotIpReset( BYTE board_no );

MC88X1P_API LRESULT __stdcall MC88X1PEnableCompLimit( BYTE board_no, BYTE axis, USHORT enable );
MC88X1P_API LRESULT __stdcall MC88X1PSetCompNLimit( BYTE board_no, BYTE axis, LONG value );
MC88X1P_API LRESULT __stdcall MC88X1PSetCompPLimit( BYTE board_no, BYTE axis, LONG value );
MC88X1P_API LRESULT __stdcall MC88X1PGetError( BYTE board_no, BYTE axis, USHORT *ret );
MC88X1P_API LRESULT __stdcall MC88X1PSetLogicCompData( BYTE board_no, BYTE axis, long data );

MC88X1P_API LRESULT __stdcall MC88X1PSetHomeLogic( BYTE board_no, BYTE axis, USHORT value );
MC88X1P_API LRESULT __stdcall MC88X1PSetZLogic( BYTE board_no, BYTE axis, USHORT value );

MC88X1P_API LRESULT __stdcall MC88X1PSetInputLogic( BYTE board_no, BYTE axis, USHORT value1, USHORT value2 );
MC88X1P_API LRESULT __stdcall MC88X1PSetServoAlarm( BYTE  board_no, BYTE axis, USHORT enable, USHORT value );
MC88X1P_API LRESULT __stdcall MC88X1PSetInposition( BYTE board_no, BYTE axis, USHORT enable, USHORT value );

MC88X1P_API LRESULT __stdcall MC88X1PSetHardLimitMode( BYTE board_no, BYTE axis, USHORT mode );
MC88X1P_API LRESULT __stdcall MC88X1PSetCompLimitMode( BYTE board_no, BYTE axis, USHORT mode );
MC88X1P_API LRESULT __stdcall MC88X1PSetNLimitLogic( BYTE board_no, BYTE axis, USHORT value );
MC88X1P_API LRESULT __stdcall MC88X1PSetPLimitLogic( BYTE board_no, BYTE axis, USHORT value );
MC88X1P_API LRESULT __stdcall MC88X1PEnableStopSignal( BYTE board_no, BYTE axis, LONG mode );
MC88X1P_API LRESULT __stdcall MC88X1PDisableStopSignal( BYTE board_no, BYTE axis );

MC88X1P_API LRESULT __stdcall MC88X1PGetSpeed( BYTE board_no, BYTE axis, DWORD *value );

// Encoder Latch
MC88X1P_API LRESULT __stdcall MC88X1PEnableEcLatch( BYTE board_no, BYTE axis, BYTE enable );
MC88X1P_API LRESULT __stdcall MC88X1PSetEcLatchMode( BYTE board_no, BYTE axis, USHORT mode );
MC88X1P_API LRESULT __stdcall MC88X1PReadEcLatchedData( BYTE board_no, BYTE axis, LONG *value );

// Encoder Clear
MC88X1P_API LRESULT __stdcall MC88X1PEnableEcClear( BYTE board_no, BYTE axis, BYTE full_time );
MC88X1P_API LRESULT __stdcall MC88X1PDisableEcClear( BYTE board_no, BYTE axis );
MC88X1P_API LRESULT __stdcall MC88X1PSetEcClearMode( BYTE board_no, BYTE axis, USHORT mode );

MC88X1P_API LRESULT __stdcall MC88X1PGetPichPulseCounter( BYTE board_no, BYTE axis, WORD *retval );
MC88X1P_API LRESULT __stdcall MC88X1PSetPichPulseCounter( BYTE board_no, BYTE axis, WORD value );
MC88X1P_API LRESULT __stdcall MC88X1PGetPichData( BYTE board_no, BYTE axis, WORD *retval );
MC88X1P_API LRESULT __stdcall MC88X1PSetPichData( BYTE board_no, BYTE axis, WORD value );
MC88X1P_API LRESULT __stdcall MC88X1PSetPichPulseMode( BYTE board_no, BYTE axis, BYTE mode );

MC88X1P_API LRESULT __stdcall MC88X1PSetLogicalPrescaleData( BYTE board_no, BYTE axis, DWORD prescale );
MC88X1P_API LRESULT __stdcall MC88X1PSetRealPrescaleData( BYTE board_no, BYTE axis, DWORD prescale );

MC88X1P_API long __stdcall RShift( long value, int count );
MC88X1P_API long __stdcall LShift( long value, int count );

// 2D Spiral 
MC88X1P_API LRESULT __stdcall MC88X1PMot2DSpiral(  BYTE board_no, BYTE axis, LONG Xe, LONG Ye, LONG pitch, LONG Vt, LONG Vs, LONG Acc, LONG Dec, USHORT mode , double time);
MC88X1P_API LRESULT __stdcall MC88X1PMot2DSpiralStop(BYTE board_no);
MC88X1P_API LRESULT __stdcall MC88X1PMot2DSpiralSts(BYTE board_no);

//******************************* Register Level Operation ***********************************
MC88X1P_API BOOL __stdcall MC88X1PReadWord( BYTE board_no, DWORD offset, WORD *retval );
MC88X1P_API BOOL __stdcall MC88X1PWriteWord( BYTE board_no, DWORD offset, WORD val );


#endif
