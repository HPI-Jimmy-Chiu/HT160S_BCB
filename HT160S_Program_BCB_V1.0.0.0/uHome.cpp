//---------------------------------------------------------------------------
// uHome.cpp - Full machine HOME monitor form for HT160S
// AI(HT160S-Maintainer) 20260602 : Ported in HT172 TfHome style.
//   Display-only motor grid (name + TALed home LED + live position).
//   The Home button on the main form triggers the existing Run_Home
//   engine and shows this monitor non-modally; the engine drives the
//   actual homing while the timer here updates LEDs/positions. When all
//   motors are homed (fAllMotorHome) the monitor auto-closes. The bottom
//   "Abort Home" button stops all motors and closes.
//---------------------------------------------------------------------------
#include "IncludeAllHeader.h"
#pragma hdrstop

#include "uHome.h"
#include "database.h"
#include "cmydef.h"
#include "csystem.h"
#include "aTrayArm.h"   //AI(HT160S-Maintainer) 20260612 : TrayArmModule->HasTray() guard for held-tray home
#pragma package(smart_init)
#pragma link "ALed"
#pragma resource "*.dfm"
//---------------------------------------------------------------------------
TfHome *fHome;
//---------------------------------------------------------------------------
static const int iRowCount = 14;
//---------------------------------------------------------------------------
__fastcall TfHome::TfHome(TComponent* Owner)
    : TForm(Owner)
{
    iHomeStep=1;
    fShow=false;
    fSeenStart=false;
    fGridBuilt=false;
    for(int i=0;i<HOME_MOTOR_MAX;i++)
    {
        LabelMotorName[i]=NULL;
        LedPtr[i]=NULL;
        EditMotorPos[i]=NULL;
    }
    if(ComponentState.Contains(csDesigning))
        return;
    BuildMotorGrid();
}
//---------------------------------------------------------------------------
// Build the motor grid dynamically (HT172 layout: 14 rows per column,
// 300px column stride; label / LED / position edit per motor).
//---------------------------------------------------------------------------
void TfHome::BuildMotorGrid()
{
    const int LedPitch  = 150;
    const int EditPitch = 185;
    int i;

    if(fGridBuilt)
        return;
    if(HSys.MotPtr==NULL)
        return;

    for(i=0;i<HSys.iTotalMotor && i<HOME_MOTOR_MAX;i++)
    {
        if(HSys.MotPtr[i]==NULL)
            continue;

        LabelMotorName[i]=new TLabel(this);
        LabelMotorName[i]->Parent=Panel1;
        LabelMotorName[i]->Left=5+300*(i/iRowCount);
        LabelMotorName[i]->Top =8+ 30*(i%iRowCount);
        LabelMotorName[i]->Caption=HSys.MotPtr[i]->NumberAlias;
        LabelMotorName[i]->Font->Size=10;

        LedPtr[i]=new TALed(this);
        LedPtr[i]->Parent=Panel1;
        LedPtr[i]->Left=LedPitch+300*(i/iRowCount);
        LedPtr[i]->Top =8+ 30*(i%iRowCount);
        LedPtr[i]->LEDStyle=LEDSqLarge;
        LedPtr[i]->Blink=false;
        LedPtr[i]->FalseColor=clSilver;
        LedPtr[i]->TrueColor=clLime;

        EditMotorPos[i]=new TEdit(this);
        EditMotorPos[i]->Parent=Panel1;
        EditMotorPos[i]->Left=EditPitch+300*(i/iRowCount);
        EditMotorPos[i]->Top =8+ 30*(i%iRowCount);
        EditMotorPos[i]->Width=95;
        EditMotorPos[i]->Font->Size=10;
        EditMotorPos[i]->ReadOnly=true;
    }
    fGridBuilt=true;
}
//---------------------------------------------------------------------------
void TfHome::ShowLed(int index, eHomeLedColor attr)
{
    if(index<0 || index>=HOME_MOTOR_MAX)
        return;
    if(LedPtr[index]==NULL)
        return;

    if(attr==eHomeUnuse)
    {
        LedPtr[index]->Value=false;
    }
    else
    {
        if(attr==eHomeOk)
            LedPtr[index]->TrueColor=clLime;
        else if(attr==eHomeError)
            LedPtr[index]->TrueColor=clRed;
        else if(attr==eHomeBusy)
            LedPtr[index]->TrueColor=clYellow;
        LedPtr[index]->Value=true;
    }
}
//---------------------------------------------------------------------------
void TfHome::ShowMotorHomePos(int i)
{
    if(i<0 || i>=HOME_MOTOR_MAX)
        return;
    if(HSys.MotPtr==NULL || HSys.MotPtr[i]==NULL)
        return;

    if(HSys.MotPtr[i]->GetEnable())
    {
        EditMotorPos[i]->Text=HSys.MotPtr[i]->ReadPos();
        if(HSys.MotPtr[i]->bHomeFlag)
            ShowLed(i, eHomeOk);
        else
            ShowLed(i, eHomeBusy);
    }
    else
    {
        EditMotorPos[i]->Text=0;
        ShowLed(i, eHomeUnuse);
    }
}
//---------------------------------------------------------------------------
void __fastcall TfHome::FormShow(TObject *Sender)
{
    BuildMotorGrid();
    Top=0;
    fShow=true;
    fSeenStart=false;
}
//---------------------------------------------------------------------------
void __fastcall TfHome::FormClose(TObject *Sender, TCloseAction &Action)
{
    iHomeStep=1;
    fShow=false;
    bHomePowerCycling=false;
}
//---------------------------------------------------------------------------
// Abort Home: stop all motors and close. fAllMotorHome stays false so the
// machine still requires a successful home before running.
//---------------------------------------------------------------------------
void __fastcall TfHome::SpeedButton1Click(TObject *Sender)
{
    HSys.StopAllMotor();
    fAllMotorHome=false;
    bHomePowerCycling=false;
    SoftStop=true;
    Close();
}
//---------------------------------------------------------------------------
void __fastcall TfHome::Timer1Timer(TObject *Sender)
{
    int i;
    if(fShow==false)
        return;

    for(i=0;i<HSys.iTotalMotor && i<HOME_MOTOR_MAX;i++)
        ShowMotorHomePos(i);

    if(HSys.Sys.SystemStart)
        fSeenStart=true;

    if(fAllMotorHome)                                  // home finished -> stop
    {
        lstHomeMsg->Items->Insert(0, "Home finished.");
        Close();
        return;
    }
    //AI(HT160S-Maintainer) 20260616 : a power-cycle recovery deliberately drops
    //SystemStart (motor power off); keep the monitor open through it so the
    //re-home that follows is not aborted.
    if(fSeenStart && HSys.Sys.SystemStart==false && bHomePowerCycling==false)
    {
        Close();
        return;
    }
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260612 : true when a motor can be treated as already homed
//and skipped during a full-machine home. ONLY servo axes qualify : a servo retains its
//absolute position, so once homed (bHomeFlag) and currently free of any drive alarm
//(Led[iAlarmLed]==false) it does not need to physically re-home. A live alarm means the
//position is no longer trustworthy, so the axis must re-home. STEPPER axes always return
//false : with no position memory they must re-home on every full-machine home (this is
//why the 4 sucker Z and TrayArm Z, which must rise on every home, are never skipped).
//Led[] is refreshed each cycle by ScanAllMotorStatus() in DoSystem() (runs before
//ProcessMotion/ProcessMotorHome in MainProc), and that same scan clears bHomeFlag on a
//live servo alarm, so an alarmed servo can never satisfy this test.
static bool IsHomedServoSkippable(TTrayMotor *m)
{
	if(m==NULL)
		return false;
	return (m->bIsServoMotor && m->bHomeFlag && (m->Led[iAlarmLed]==false));
}
//---------------------------------------------------------------------------
//AI(HT160S-Maintainer) 20260602 : full-machine motor home engine (HT172 0420
//ProcessMotorHome equivalent, rewritten non-FSM / no FSMRunner). Drives every
//enabled axis home in a safe order: raise the TrayArm Z cylinder and open the
//clamps FIRST so the TrayArm head clears any tray below, then batch-home the 4
//sucker Z axes, and only after the TrayArm Z is confirmed up batch-home all XY
//axes (TrayArm X included). Mirrors same-machine V300A DoTrayXHome case 1000
//where Z-up = C_TrayArmZ_Up.On() verified by IsOn()==true (NOT Pop, which would
//drop Z and crash into the tray below). Non-blocking: returns false while busy.
//AI(HT160S-Maintainer) 20260612 : "home only the not-yet-homed". Steppers (no position
//memory) always re-home; an already-homed, non-alarmed servo keeps its position and is
//skipped (see IsHomedServoSkippable). The TrayArm-Z-up + sucker-Z home safety actions
//are unaffected because those axes/cylinders are steppers/cylinders, never skipped.
bool TfHome::ProcessMotorHome()
{
//#ifdef SOFT_SIMULATE
//	return true;
//#else
	AnsiString sErr="";
    int &iMotorHomeTask=iHomeStep;
	static HTimer HomePowerTimer;
	switch(iMotorHomeTask)
	{
		case 1:
		{
			//AI(HT160S-Maintainer) 20260616 : power-cycle gate (compromise port
			//of the old-160 home Off->On). A latched servo-amp alarm cannot be
			//cleared in software (MC88X1 SetServoOn is a no-op), so the ONLY way
			//to re-energize and clear it is a physical SwMotorRelay Off->On. We
			//run that ONLY when motor power was lost OR an enabled servo is
			//alarmed; a clean operator HOME (power on, no alarm) skips it and
			//keeps the fast selective home. SOFT_SIMULATE always skips.
			bool bNeedPowerCycle=false;
			if(bMotorPowerState==false)
				bNeedPowerCycle=true;
			if(HSys.MotPtr!=NULL)
			{
				for(int i=0; i<HSys.iTotalMotor; i++)
				{
					TTrayMotor *m=HSys.MotPtr[i];
					if(m==NULL || m->GetEnable()==false)
						continue;
					if(m->bIsServoMotor && m->ReadServoAlarmOn() && m->Led[iAlarmLed])
					{
						bNeedPowerCycle=true;
						break;
					}
				}
			}
#ifdef SOFT_SIMULATE
			bNeedPowerCycle=false;
#endif
			if(bNeedPowerCycle)
			{
				if(lstHomeMsg!=NULL)
					lstHomeMsg->Items->Insert(0, "Motor power cycle (servo alarm recovery)....");
				RecordProcess("Home: motor power-cycle to clear latched servo alarm");
				bHomePowerCycling=true;
				AllBreakLock();
				HSys.Sw.SwMotorRelay.Off();
				bMotorPowerState=false;
				bMotorHomePowerOn=false;
				HomePowerTimer.Set(50);
				HomePowerTimer.On();
				iMotorHomeTask=10;
				return false;
			}
			iMotorHomeTask=2;
			return false;
		}
		case 10:
			//Servo discharge complete -> re-energize motor power and re-assert
			//servo-on for every alarm-monitored axis, then settle.
			if(HomePowerTimer.Off())
			{
				HSys.Sw.SwMotorRelay.On();
				bMotorPowerState=true;
				if(HSys.MotPtr!=NULL)
				{
					for(int i=0; i<HSys.iTotalMotor; i++)
					{
						if(HSys.MotPtr[i]!=NULL && HSys.MotPtr[i]->ReadServoAlarmOn())
							HSys.MotPtr[i]->ServoOnOff(true);
					}
				}
				HomePowerTimer.Set(30);
				HomePowerTimer.On();
				iMotorHomeTask=20;
			}
			return false;
		case 20:
			//Servo-on settle complete -> release brakes, end the power-cycle and
			//fall into the normal home (the previously-alarmed axis re-homes
			//because ScanAllMotorStatus cleared its bHomeFlag).
			if(HomePowerTimer.Off())
			{
				AllBreakFree();
				bMotorHomePowerOn=true;
				bHomePowerCycling=false;
				if(lstHomeMsg!=NULL)
					lstHomeMsg->Items->Insert(0, "Motor power restored.");
				iMotorHomeTask=2;
			}
			return false;
		case 2:
			//Arm a fresh home: clear stale home flags so re-home actually runs.
			//AI(HT160S-Maintainer) 20260612 : but do NOT clear the flag of an
			//already-homed, non-alarmed servo : it keeps its position and is skipped
			//in the home batches below, so re-arming it would force a needless re-home.
			if(HSys.MotPtr!=NULL)
			{
				for(int i=0; i<HSys.iTotalMotor; i++)
				{
					if(HSys.MotPtr[i]==NULL)
						continue;
					if(IsHomedServoSkippable(HSys.MotPtr[i]))
						continue;
					HSys.MotPtr[i]->InitHomeTask();
				}
			}
			//Interference guard: raise TrayArm Z + open clamps before any X home.
			HSys.Cyn.C_TrayArmZ_Up.On();
			HSys.Cyn.C_TrayArmZ_Down.Off();
			//AI(HT160S-Maintainer) 20260612 : if the TrayArm is carrying a tray, keep the
			//clamps CLOSED through the home so the tray is not dropped. Z is raised first,
			//so the head+tray clears any tray below before X moves : opening the clamps is
			//only needed to release an empty/unknown head. The held tray is then placed on
			//resume (see TTrayArmModule::InitialFlag bKeepMaterial / DoTrayArm case 100).
			if(TrayArmModule==NULL || TrayArmModule->HasTray()==false)
			{
				HSys.Cyn.C_TrayArm_FrontClamp.Off();
				HSys.Cyn.C_TrayArm_RearClamp.Off();
			}
			iMotorHomeTask=100;
			return false;
		case 100:
		{
			//Hold TrayArm Z raised; batch-home the 4 sucker Z axes in parallel.
			HSys.Cyn.C_TrayArmZ_Up.On();
			HSys.Cyn.C_TrayArmZ_Down.Off();
			TTrayMotor *Z[4];
			Z[0]=HSys.Mot.MSuckZ_1; Z[1]=HSys.Mot.MSuckZ_2;
			Z[2]=HSys.Mot.MSuckZ_3; Z[3]=HSys.Mot.MSuckZ_4;
			bool bZHomed=true;
			for(int i=0; i<4; i++)
			{
				if(Z[i]==NULL || Z[i]->GetEnable()==false)
					continue;
				//AI(HT160S-Maintainer) 20260612 : sucker Z are steppers (never skipped),
				//but apply the same servo-skip guard generically for config safety.
				if(IsHomedServoSkippable(Z[i]))
					continue;
				if(Z[i]->Home(sErr)==false)
					bZHomed=false;
			}
            if(bZHomed)
			    iMotorHomeTask=200;
			return false;
		}
		case 200:
		{
			//TrayArm Z confirmed up -> batch-home all XY axes (TrayArm X safe now).
            TTrayMotor *XY[16];
			XY[0]=HSys.Mot.MSortingArmX; XY[1]=HSys.Mot.MTrayArmX;
			XY[2]=HSys.Mot.MEmptyY;      XY[3]=HSys.Mot.MLoaderY_1;
			XY[4]=HSys.Mot.MLoaderY_2;   XY[5]=HSys.Mot.MAutoY_1;
			XY[6]=HSys.Mot.MAutoY_2;     XY[7]=HSys.Mot.MAutoY_3;
			XY[8]=HSys.Mot.MAutoY_4;     XY[9]=HSys.Mot.MAutoY_5;
			XY[10]=HSys.Mot.MAutoY_6;    XY[11]=HSys.Mot.MTopCCDX;
			XY[12]=HSys.Mot.MBottomCCDY; XY[13]=HSys.Mot.MPitchX;
            XY[14]=HSys.Mot.MColorY;     XY[15]=HSys.Mot.MTopCCDX_Color;
			bool bAllHomed=true;
            for(int i=0; i<16; i++)
			{
				if(XY[i]==NULL || XY[i]->GetEnable()==false)
					continue;
				//AI(HT160S-Maintainer) 20260612 : skip an already-homed, non-alarmed
				//servo (keeps absolute position); steppers + alarmed/never-homed servos
				//still physically re-home here.
				if(IsHomedServoSkippable(XY[i]))
					continue;
				if(XY[i]->Home(sErr)==false)
					bAllHomed=false;
			}
			if(bAllHomed)
			{
				iMotorHomeTask=1;
				return true;
			}
			return false;
		}
	}
	iMotorHomeTask=1;
	return false;
}
//---------------------------------------------------------------------------
