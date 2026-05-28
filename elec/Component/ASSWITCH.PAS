{
 ASSwitcher Component 1.2.1 Copyright © 1997
       Andrey Abakumov (aga@oficina.rnd.su)
       Sergey Hrusch

The switch of a type TrackBar, but only round. Can show a scale
and values.

The component is written under influence
of a seen  similar by the form component
from a  package  Gadgets for Windows 1.1
Universal Dynamics.

Many thanks:
  Ali Butaev behind component TWave
   E-mail: alee@datacom.ru
   
  Alistair George behind found errors
   E-mail: bigal@xtra.co.nz

 and other.

Properties:
 Position    - current position on the switch;
 Max         - maximum value
               (Min - allways zero);
 PrintValues - to print or not print values
               (it is better to not print if
                values more 25)

 SoundType       - disabled playing if SoundType=wssNone
                   of a sound at switching,
                   enabled and (SoundType=wssExe) to keep in EXE the file,
                   enabled and (SoundType=wssWav) to keep in Wav the file,

 SoundChange - choice of a sound file or sound resource

 ResourceType - type of a resource if SoundType=wssExe

 WaveOptions -
               woSync      - The sound is played synchronously and the
                             function does not return until the sound ends.
               woNoDefault - If the sound can't be found, the function returns
                             silently without playing the default sound.
               woLoop      - The sound will continue to play repeatedly.
               woNoStop    - If a sound is currently playing, the function
                             will immediately return FALSE without playing
                             the requested sound.

How to make *.WAV in resource?

Create file filename.rc
For example:
CLICK WAVE filename.wav - CLICK is resource name, WAVE is your filetype, plus filename
To create *.res file (brcc32 filename.rc)

In Object Inspector, AASwitcher properties must be defined as follows:
CLICK WAVE
      ^^^         - you must specify ResourceType property
^^^^^             - you must specify ResourceName.
Also select sound type eg WssEXE }

//To include resource in our EXE place after {$R *.DFM}
//{$R yourresourcefilename.res} <---Insert the following without the comments(//)

unit ASswitch;

interface


uses
  SysUtils, Windows, Messages, Classes, Graphics, Controls,
  Forms, Dialogs,Menus,MMSystem,DsgnIntf,ASFunc;

type
  TASSoundFileChange = class(TStringProperty)
     function GetAttributes:TPropertyAttributes;override;
     procedure Edit;override;
    end;

  TWhereSoundStore = (wssExe,wssWav,wssNone);
  
  TWaveOption = (woSync, woNoDefault, woLoop, woNoStop);
  TWaveOptions = set of TWaveOption;

  TASSwitcher = class(TCustomControl)
  private
    { Private declarations }
    FMin:Integer;
    FMax:Integer;
    FPosition:Integer;
    FPrintValues:Boolean;
    FSoundType:TWhereSoundStore;
    FSoundChange:String;
    FResourceType :String;
    FWaveOptions:Word;
    FPalyOptions:TWaveOptions;
    
    FDrawFocused:Boolean;
    FDrawTicks_:Boolean;

    FOnChange:TNotifyEvent;

    Procedure SetMin(Value:Integer);
    Procedure SetMax(Value:Integer);
    Procedure SetPosition(Value:Integer);
    Procedure SetPrintValues(Value:Boolean);
    Procedure SetSoundType(Value:TWhereSoundStore);
    Procedure SetSoundChange(Value:String);
    Procedure SetResourceType(Value:String);
    Procedure SetDrawFocused(Value:Boolean);
    Procedure SetDrawTicks(Value:Boolean);

    property Min:Integer
             read FMin
             write SetMin
             default 0;

    Procedure GradValues_;
    Procedure GradToCoords_;
    Procedure SearchGradForPoint;
    Procedure TextValuesTicks;

    Procedure CMMouseLeave(var Msg:TMessage);
            message cm_MouseLeave;
    Procedure WMGetDlgCode(var Msg:TWMGetDlgCode);
              message wm_GetDlgCode;
  protected
    { Protected declarations }
   Procedure Change;dynamic;

   Procedure DrawTicks;virtual;
   Procedure DrawFace;virtual;
   Procedure DrawPointer;virtual;


   Procedure Paint;override;
   Procedure MouseDown(Button:TMouseButton;
       Shift:TShiftState;X,Y:Integer);override;
   Procedure MouseMove(Shift:TShiftState;
              X,Y:Integer);override;
   Procedure MouseUp(Button:TMouseButton;
       Shift:TShiftState;X,Y:Integer);override;
   Procedure KeyDown(var Key:Word;
                   Shift:TShiftState);override;

   Procedure DoEnter;override;
   Procedure DoExit;override;
   public
    { Public declarations }
   Constructor Create(AOwner:TComponent);override;
  published
    { Published declarations }
    property Max:Integer
             read FMax
             write SetMax
             default 10;
    property Position:Integer
             read FPosition
             write SetPosition
             default 1;
    property PrintValues:Boolean
             read FPrintValues
             write SetPrintValues
             default False;
    property SoundType:TWhereSoundStore
             read FSoundType
             write SetSoundType
             default wssWav;
    property SoundChange:String
             read FSoundChange
             write SetSoundChange;
    property ResourceType:String
             read FResourceType
             write SetResourceType;
    property WaveOptions:TWaveOptions
             read FPalyOptions
             write FPalyOptions
             default [woSync,woNoDefault,woNoStop];
    property DrawFocused:Boolean
             read FDrawFocused
             write SetDrawFocused
             default False;

    property DrawTicks_:Boolean
             read FDrawTicks_
             write SetDrawTicks
             default True;
    property OnChange:TNotifyEvent
             read FOnChange
             write FOnChange;

    property Enabled;
    property Color;
    property Hint;
    property HelpContext;
    property ParentShowHint;
    property ShowHint;
    property Visible;
    property DragCursor;
    property DragMode;
    property PopupMenu;

    property OnClick;
    property OnMouseDown;
    property OnMouseMove;
    property OnMouseUp;
    property OnDragDrop;
    property OnDragOver;
    property OnEndDrag;
    property OnKeyDown;
    property OnKeyPress;
    property OnKeyUp;
    property OnEnter;
    property OnExit;
end;

Var
   Offset,
   Offset2,
   Offset4:Integer;
   TickCount:Integer;
   i,j:Integer;

   StartA,
   EndA:Integer;
//   midX,midY,
   stX,stY,
   endX,endY: Integer;
   sX,sY,
   eX,eY:Real;
   iGrad:Integer;
   rGrad:Real;

   r,p,p1,p2,p4,t,t1:Real;
//   x:Integer;
   t1_:Integer;

   Flag,
   Priv,
   Blick:Boolean;

   R1,
   BigRad,
   SmallRad:Real;
   sAngle,
   eAngle:Integer;
   X_,Y_,
   X1,Y1:Integer;
   sinMidAngle,
   midAngle1:Real;
   midAngle:Integer;

//   CoordPointX,
//   CoordPointY:Integer;
   PointX,PointY,
   PointX1,PointY1,
   RadTick:Real;
   TmpAn,TmpAn1:Integer;

   SoundFile,SoundFile1:PChar;
   wText,hText:Integer;
   sA,eA:TPoint;
   
procedure Register;
implementation
{------------ TASSoundFileChange -----------}
Function TASSoundFileChange.GetAttributes:TPropertyAttributes;
begin
Result := inherited GetAttributes + [paDialog];
end;

Procedure TASSoundFileChange.Edit;
var
   Dialog:TOpenDialog;
begin
Dialog:=TOpenDialog.Create(Application);
Dialog.Filter:='Sound files|*.wav';
Dialog.Title:='Open sound file';
If Dialog.Execute
 Then SetStrValue(Dialog.FileName);
Dialog.Free;
end;

{----------- TASSwitcher ------------}
Constructor TASSwitcher.Create(AOwner:TComponent);
begin
inherited Create(AOwner);
// ControlStyle := [csClickEvents, csCaptureMouse];
// ControlState := ControlState + [csCreating];
 Width:=150;
 Height:=150;
 FMin:=0;
 FMax:=10;
 FPosition:=0;
 ControlStyle:=ControlStyle-[csOpaque];
 Offset:=20;
 Offset2:=Round(Offset / 1.5);
 Offset4:=Round(Offset / 10);
 New(SoundFile);
 FDrawFocused:=False;
 FDrawTicks_:=True;
 SoundType:=wssWav;
 WaveOptions:=[woSync,woNoDefault,woNoStop];

// ControlState := ControlState - [csCreating];
end;

Procedure TASSwitcher.TextValuesTicks;
begin
  TmpAn1:=StartA;
  TmpAn:=TmpAn1;
  If StartA>360 Then StartA:=StartA-360;
  If TmpAn>360 Then TmpAn:=TmpAn-360;

  If (TmpAn>=90)and(TmpAn<=180)
   Then TmpAn:=TmpAn-90;
  If (TmpAn>=180)and(TmpAn<=270)
   Then TmpAn:=TmpAn-180;
  If (TmpAn>=270)and(TmpAn<=360)
   Then TmpAn:=TmpAn-270;

  PointY:=(TmpAn*Pi)/180;
  PointY1:=Sin(PointY);
  RadTick:=(Width div 2)-Offset4;

  If ((StartA>=0)and(StartA<=90))Or
     ((StartA>=180)and(StartA<=270))
   Then
    begin
    PointY:=Abs(PointY1*RadTick);
    PointX:=Abs(Sqrt(Sqr(RadTick)-Sqr(PointY)));
    end;
  If ((StartA>=90)and(StartA<=180))Or
     ((StartA>=270)and(StartA<=360))
   Then
    begin
    PointX:=Abs(PointY1*RadTick);
    PointY:=Abs(Sqrt(Sqr(RadTick)-Sqr(PointX)));
    end;
  If (StartA>=0)and(StartA<=90)
   Then
    begin
    CoordPointX:=Trunc(midX+PointX);
    CoordPointY:=Trunc(midY-PointY);
    end;
  If (StartA>=90)and(StartA<=180)
   Then
    begin
    CoordPointX:=Trunc(midX-PointX);
    CoordPointY:=Trunc(midY-PointY);
    end;
  If (StartA>=180)and(StartA<=270)
   Then
    begin
    CoordPointX:=Trunc(midX-PointX);
    CoordPointY:=Trunc(midY+PointY);
    end;

  If (StartA>=270)and(StartA<=360)
   Then
    begin
    CoordPointX:=Trunc(midX+PointX);
    CoordPointY:=Trunc(midY+PointY);
    end;

  If (StartA>=130)and(StartA<=180)
   Then
    begin
    CoordPointX:=CoordPointX-Round(1.5*wText);
    CoordPointY:=CoordPointY-hText;
    end;
  If (StartA>=180)and(StartA<=270)
   Then
    begin
    CoordPointX:=CoordPointX-Round(1.5*wText);
    CoordPointY:=CoordPointY-(hText div 2);
    end;
  If (StartA>=0)and(StartA<=50)
   Then
    begin
    CoordPointX:=CoordPointX+(wText div {2}3);
    CoordPointY:=CoordPointY-(hText div 2);
    end;
  If (StartA>=50)and(StartA<=130)
   Then
    begin
    CoordPointX:=CoordPointX-(wText div 2);
    CoordPointY:=CoordPointY-Round({2.5}1.3*hText);
    end;
end;

Procedure TASSwitcher.SearchGradForPoint;
begin
BigRad:=midX-Offset-7;
SmallRad:=midX-Offset-27;
x1:=midX-X_;
Y1:=midY-Y_;

R1:=SQRT(SQR(X1)+SQR(Y1));
If (R1<=BigRad) and (R1>=SmallRad)
 Then Priv:=True
 Else Priv:=False;

If (X_<=midX) and (Y_<midY) Then midAngle:=90;
If (X_<midX) and (Y_>=midY) Then midAngle:=180;
If (X_>=midX) and (Y_>midY) Then midAngle:=270;
If (X_>midX) and (Y_<=midY) Then midAngle:=0;

If (midAngle=0) or (midAngle=180)
 Then sinMidAngle:=Abs(Trunc(Y1))/(Sqrt(Sqr(X1)+Sqr(Y1)));
If (midAngle=90) or (midAngle=270)
 Then sinMidAngle:=Abs(Trunc(X1))/(Sqrt(Sqr(X1)+Sqr(Y1)));

midAngle1:=ArcTan(sinMidAngle/Sqrt(1-sqr(sinMidAngle)));
MidAngle1:=(midAngle1/Pi)*180;
midAngle:=Trunc(midAngle+midAngle1);
If midAngle>270 Then midAngle:=midAngle-360;
end;

Procedure TASSwitcher.GradToCoords_;
begin
 sX:=Cos((StartA/180.0)*pi);
 sY:=Sin((StartA/180.0)*pi);
 eX:=Cos((EndA/180.0)*pi);
 eY:=Sin((EndA/180.0)*pi);

 stX:=Round(sX*100);
 stY:=Round(sY*100);
 endX:=Round(eX*100);
 endY:=Round(eY*100);
end;
 
Procedure TASSwitcher.GradValues_;
var
   x:Integer;
 procedure Tmp;
 begin
 t:=t+1;
 t1:=t1-1;
 p1:=x*t1;
 p2:=t*x+p;
 p4:=Frac(p2/2);
 end;
begin
 p:=90;
 p1:=360-p;
 x:=TickCount;
 t:=Frac(p1/x);
 t1:=Trunc(p1/x);
 p1:=x*t1;
 p2:=t*x+p;
 p4:=Frac(p2/2);
 While p4<>0 Do Tmp;
end;

Procedure TASSwitcher.DrawFace;
begin
With Canvas Do
 begin
 Pen.Mode:=pmCopy;
 Brush.Style:=bsClear;
 Pen.Color := clBlack;
 Brush.Color:=$00D4D4D4;
 Ellipse(Offset+2,Offset+2,
         Width-2-Offset,Height-2-Offset);

 Pen.Color:=$00909090;
 Brush.Style:=bsDiagCross;
 Pen.Width:=5;
 Arc(Offset+5,Offset+5,Width-5-Offset,
     Height-5-Offset,Offset+5,Offset+Height+5,
     Width-5-Offset,-5-Offset);

 Brush.Style:=bsClear;
 Pen.Color:=clBlack;
 Pen.Width:=1;
 Arc(Offset+2,Offset+2,Width-2-Offset,
     Height-2-Offset,Offset+2,Offset+2,
     -2-Offset,-2-Offset);

 Pen.Color:=clSilver;
 Brush.Color:=$00B6B6B6;
 Ellipse(Offset+7,Offset+7,Width-7-Offset,
         Height-7-Offset);
 end;
end;

Procedure TASSwitcher.DrawPointer;
var
   tW,tH:Integer;
begin
With Canvas Do
 begin
 Brush.Style:=bsClear;
 Pen.Color := clSilver;
 Brush.Color:=clRed;
 end;
 GradValues_;
 iGrad:=Trunc(t1);
 StartA:=270+Trunc(p2 / 2);
 EndA:=270-Trunc(p2 / 2);
 t1_:=Trunc(t1/2);

 StartA:=EndA-(iGrad*(FPosition-FMin))+t1_;
 EndA:=StartA-iGrad;

 sAngle:=StartA;
 eAngle:=EndA;

 GradToCoords_;
 With Canvas Do
  begin
  Pie(Offset+7,Offset+7,Width-7-Offset,
            Height-7-Offset,
            midX+endX,midY-endY,
            midX+stX,midY-stY);

  Pen.Color:=clSilver;
  Brush.Color:=$00B6B6B6;
  Ellipse(Offset+27,Offset+27,Width-27-Offset,
         Height-27-Offset);
  If not PrintValues Then
   begin
   Brush.Color:=Self.Color;
   Brush.Style:=bsSolid;
   tW:=TextWidth(Format('%d',[FPosition]));
   tH:=TextHeight(Format('%d',[FPosition]));
   Rectangle(midX-5,Height-20,
    midX+5+tW,
    Height-20+tH);
   Pen.Color:=clBlack;
   TextOut(midX-5,Height-20,Format('%d',[FPosition]));
   end;
  end;
end;

Procedure TASSwitcher.DrawTicks;
var
   j:Integer;
begin
With Canvas Do
 begin
 Brush.Color:=clMenu;
 GradValues_;//(TickCount);
 iGrad:=Trunc(t1);
 StartA:=270+Trunc(p2 / 2);
 EndA:=270-Trunc(p2 / 2);

 GradToCoords_;
 Arc(Offset2,Offset2,Width-Offset2,
     Height-Offset2,Width-Offset2,
     Height-Offset2,Offset2,Height-Offset2);


 Brush.Style:=bsClear;
 Brush.Color:=Self.Color;
 FillRect(ClientRect);

 Brush.Style:=bsClear;
 Pen.Color := clBlack;
 Brush.Color:=Color;

 For j:=1 To TickCount+1 Do
  begin
  GradToCoords(StartA,EndA,sA,eA);
  ValuesTicks(StartA,Offset4+11);
  MoveTo(CoordPointX,CoordPointY);
  ValuesTicks(StartA,Offset4);
  LineTo(CoordPointX,CoordPointY);

  If FPrintValues Then
   begin
   wText:=TextWidth(IntToStr(FMax-j+1));
   hText:=TextHeight(IntToStr(FMax-j+1));
   TextValuesTicks;
   If FMax=0
    Then TextOut(CoordPointX,CoordPointY,'1')
    Else TextOut(CoordPointX,CoordPointY,IntToStr(FMax-j+1));
   StartA:=TmpAn1;
   end;
  Inc(StartA,iGrad);
  end;

 If FPrintValues Then
  begin
  TmpAn1:=StartA;
  StartA:=EndA;
  TextValuesTicks;
  TextOut(CoordPointX,CoordPointY,IntToStr(FMin));
  StartA:=TmpAn1;
  end;
  Ellipse(Offset2,Offset2,Width-Offset2,
      Height-Offset2);

 Pen.Color := Color;
 Brush.Color:=Color;
 Pen.Mode:=pmCopy;

 StartA:=270+Trunc(p2 / 2);
 EndA:=270-Trunc(p2 / 2);
 GradToCoords_;

 Arc(Offset4,Offset4,Width-Offset4,Height-Offset4,
      midX+stX,midY-stY,
      midX+endX,midY-endY);

 Arc(Offset2,Offset2,Width-Offset2,Height-Offset2,
      midX+endX,midY-endY,
      midX+stX,midY-stY);
end;
end;

Procedure TASSwitcher.Paint;
begin
 Height:=Width;
 midX:=Width div 2;
 midY:=Height div 2;
 If FMax=0
  Then TickCount:=FMax-FMin+1
  Else TickCount:=FMax-FMin;
 Canvas.Brush.Style:=bsClear;
 Canvas.Brush.Color:=Self.Color;
 Canvas.FillRect(ClientRect);
 If (FDrawFocused) and (not FPrintValues) Then
  If Focused Then
   begin
   Canvas.Pen.Style:=psDot;
   Canvas.Ellipse(ClientRect.Left,ClientRect.Top,
                  ClientRect.Right,ClientRect.Bottom);
   Canvas.Pen.Style:=psSolid;
   end;

 If FDrawTicks_ Then DrawTicks;
 DrawFace;
 DrawPointer;
end;

Procedure TASSwitcher.SetPosition(Value:Integer);
var
  Changed: Boolean;
  
  FindResHandle : THandle;
  LoadResHandle : THandle;
  LockResPointer : Pointer;

begin
If Value <> FPosition Then
 begin
 If (Value < FMin)
  Then Value:=FMin
  Else
   If Value > FMax Then Value:=FMax;

 Changed := FPosition <> Value;
 FPosition:=Value;

// Change;
 If csDesigning in ComponentState
  Then Invalidate
  Else
   begin
   If FMax=0
    Then TickCount:=FMax-FMin+1
    Else TickCount:=FMax-FMin;

   With Canvas Do
    begin
    Pen.Color:=clSilver;
    Brush.Style:=bsClear;
    Brush.Color:=$00B6B6B6;
    Ellipse(Offset+7,Offset+7,Width-7-Offset,
            Height-7-Offset);
    DrawPointer;
    if Changed and Assigned(FOnChange) then FOnChange(Self);

    If FSoundType <> wssNone Then
     begin
     FWaveOptions:=0;
     If woSync In WaveOptions
     Then
       FWaveOptions := FWaveOptions Or SND_SYNC
     Else
       FWaveOptions := FWaveOptions Or SND_ASYNC;
     If woNoDefault In WaveOptions
     Then
       FWaveOptions := FWaveOptions Or SND_NODEFAULT;
     If woLoop In WaveOptions
     Then
       FWaveOptions := FWaveOptions Or SND_LOOP;
     If woNoStop In WaveOptions
     Then
       FWaveOptions := FWaveOptions Or SND_NOSTOP;

     If FSoundType = wssWav
      Then sndPlaySound(PChar(FSoundChange),FWaveOptions)
      Else
       begin
       FindResHandle := FindResource(HInstance,PChar(FSoundChange),PChar(FResourceType));
       If FindResHandle = 0
        Then Abort {Resource not found}
        Else
         begin
         LoadResHandle := LoadResource(HInstance,FindResHandle);
         If LoadResHandle = 0
          Then Abort {Resource not load}
          Else
           begin
           LockResPointer := LockResource(LoadResHandle);
           If LockResPointer = Nil
            Then
             begin
             FreeResource(LoadResHandle);
             Abort; {Can't lock resource}
             end
            Else
             begin
             sndPlaySound(LockResPointer,FWaveOptions Or SND_MEMORY);
             end;
           end;
         end;
       end;
      end;
    end;
   end;
 end;
end;

Procedure TASSwitcher.SetMin(Value:Integer);
begin
If Value <> FMin Then
 begin
 If (Value >= 0)and
    (Value < FMax )and
    (Value <= 270) Then FMin:=Value;
 If FMax=0
  Then TickCount:=FMax-FMin+1
  Else TickCount:=FMax-FMin;
 DrawPointer;
 Invalidate;
 Width:=Width+1;
 Width:=Width-1;
 end;
end;

Procedure TASSwitcher.SetMax(Value:Integer);
begin
If Value <> FMax Then
 begin
 If (Value >= 0)and(Value <= 270) Then FMax:=Value;
 If FMax=0
  Then TickCount:=FMax-FMin+1
  Else TickCount:=FMax-FMin;
 DrawPointer;
 Invalidate;
 Width:=Width+1;
 Width:=Width-1;
 end;
end;

Procedure TASSwitcher.SetPrintValues(Value:Boolean);
begin
If Value <> FPrintValues
 Then
  begin
  If Value
   Then
    begin
//    Offset:=60;
    Offset:=30;
    Offset2:=Round(Offset / 1.2);
    Offset4:=Round(Offset / 2);
    end
   Else
    begin
//    Offset:=40;
    Offset:=20;
    Offset2:=Round(Offset / 1.5);
    Offset4:=Round(Offset / 10);
    end;

  FPrintValues:=Value;
   If FMax=0
    Then TickCount:=FMax-FMin+1
    Else TickCount:=FMax-FMin;
  Canvas.Brush.Style:=bsClear;
  Canvas.Brush.Color:=Self.Color;
  Canvas.FillRect(ClientRect);
  Invalidate;
  end;
end;

Procedure TASSwitcher.SetSoundType(Value:TWhereSoundStore);
begin
If Value <> FSoundType
 Then FSoundType:=Value;
end;

Procedure TASSwitcher.SetSoundChange(Value:String);
begin
If Value <> FSoundChange
 Then
  FSoundChange:=Value;
end;

Procedure TASSwitcher.SetResourceType(Value:String);
begin
If Value <> FResourceType
 Then FResourceType:=Value;
end;

Procedure TASSwitcher.CMMouseLeave(var Msg:TMessage);
begin
Flag:=False;
end;

Procedure TASSwitcher.MouseDown(Button:TMouseButton;
       Shift:TShiftState;X,Y:Integer);
begin
inherited MouseDown(Button,Shift,X,Y);
SetFocus;
X_:=X;
Y_:=Y;
SearchGradForPoint;
If (Trunc(midAngle) <= sAngle) and
   (Trunc(midAngle) >= eAngle)
 Then
  If (Button=mbLeft) and (Priv) Then Flag:=True
 Else Flag:=False;
end;

Procedure TASSwitcher.MouseMove(Shift:TShiftState;X,Y:Integer);
var
   i:Integer;
begin
If (X=midX)and(Y=midX) Then Flag:=False;
If Flag Then
 begin
 X_:=X;
 Y_:=Y;
 SearchGradForPoint;
 For i:=FMin To FMax Do
  begin
  StartA:=270+Trunc(p2 / 2);
  EndA:=270-Trunc(p2 / 2);
  t1_:=Trunc(t1/2);
  StartA:=EndA-(iGrad*i)+t1_;
  EndA:=StartA-iGrad;
  If (Trunc(midAngle) <= StartA) and
     (Trunc(midAngle) >= EndA)
    Then
     begin
     j:=i;
     Blick:=True;
     Break;
     end
    Else Blick:=False;
  end;
 If Blick Then Position:=j;
 end;
inherited MouseMove(Shift,X,Y);
end;

Procedure TASSwitcher.MouseUp(Button:TMouseButton;
       Shift:TShiftState;X,Y:Integer);
begin
If (Button=mbLeft) Then Flag:=False;
inherited MouseUp(Button,Shift,X,Y);
end;

Procedure TASSwitcher.Change;
begin
 If Assigned(FOnChange) Then FOnChange(Self);
end;

Procedure TASSwitcher.KeyDown(var Key:Word;Shift:TShiftState);
begin
inherited KeyDown(Key,Shift);
Case Key Of
     vk_End:Position:=FMax;
     vk_Home:Position:=FMin;
     vk_Left: If FPosition>FMin
               Then Position:=FPosition-1;
     vk_Right: If FPosition<FMax
               Then Position:=FPosition+1;
    end;
end;

Procedure TASSwitcher.WMGetDlgCode(var Msg:TWMGetDlgCode);
begin
inherited;
Msg.Result:=dlgc_WantArrows;
end;

Procedure TASSwitcher.DoEnter;
begin
inherited DoEnter;
Refresh;
end;

Procedure TASSwitcher.DoExit;
begin
inherited DoExit;
Refresh;
end;

Procedure TASSwitcher.SetDrawFocused(Value:Boolean);
begin
If Value <> FDrawFocused Then FDrawFocused:=Value;
end;

Procedure TASSwitcher.SetDrawTicks(Value:Boolean);
begin
If Value <> FDrawTicks_
 Then
  begin
  FDrawTicks_:=Value;
  Invalidate;
  end;
end;

procedure Register;
begin
  RegisterPropertyEditor(TypeInfo(String),
TASSwitcher,'SoundChange',TASSoundFileChange);

  RegisterComponents('AbakSoft', [TASSwitcher]);
end;
end.
