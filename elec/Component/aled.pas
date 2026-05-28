unit ALed;

{ Original by Chang-Ting SU.  E-mail:ctsu@ms12.hinet.net }

{ Modified by H.J. Harvey     E-mail:hharvey@dove.net.au }
{ Version 1.03  31/JULY/97 }

{ Now provides 6 different LED styles:
      Large Round (the original)
      Small Round
      Large Square
      Small Square
      Vertical Rect
      Horizontal Rect }

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs,
  ExtCtrls;

type
  TLEDStyle = (LEDSmall,LEDLarge,LEDSqSmall,LEDSqLarge,LEDVertical,LEDHorizontal) ;
  TALed = class(TGraphicControl)
  private
    { Private declarations }
  protected
     { Protected declarations }
     fLedBitmap    : Array[0..1] of TBITMAP;
     fLedTimer     : TTimer;
     fTrueColor    : TColor;
     fFalseColor   : TColor;
     fBlink        : Boolean;
     fLEDStyle     : TLEDStyle;
     fInterval     : longint;
     fValue        : Boolean;
     ColorTemp     : Boolean;
     fOnTimer      : TNotifyEvent;
     fOnMouseEnter : TNotifyEvent;
     fOnMouseLeave : TNotifyEvent;
     procedure Paint;override;
     procedure OnLedTimer(Sender : TObject);
     procedure CMMouseEnter(var Message:TMessage);message CM_MOUSEENTER;
     procedure CMMouseLeave(var Message:TMessage);message CM_MOUSELEAVE;
  public
    { Public declarations }
    constructor Create(AOwner:TComponent);override;
    destructor  Destroy;override;
    procedure CreateLedBitmap;
    procedure FreeLedBitmap;
    procedure ChangeValue(V : Boolean);
    procedure ChangeBlink(V : Boolean);
    procedure ChangeStyle(V : TLEDStyle);
    procedure SetTrueColor(V : TColor);
    procedure SetFalseColor(V : TColor);
    procedure SetInterval(V : longint);
    procedure SetToTrueColor;
    procedure SetToFalseColor;
    procedure SetLedTimer;
    procedure ResetLedTimer;
  published
    { Published declarations }
    property TrueColor  : TColor  read fTrueColor write SetTrueColor default clLime;
    property FalseColor : TColor  read fFalseColor write SetFalseColor default clSilver;
    property Blink      : Boolean read fBlink write ChangeBlink default false;
    property Value      : Boolean read fValue write ChangeValue default false;
    property Interval   : longint read fInterval write SetInterval default 1000;
    property LEDStyle   : TLEDStyle read fLEDStyle write ChangeStyle default LEDSmall;
    property OnTimer    : TNotifyEvent read fOnTimer write fOnTimer;
    property OnMouseEnter : TNotifyEvent read fOnMouseEnter write fOnMouseEnter;
    property OnMouseLeave : TNotifyEvent read fOnMouseLeave write fOnMouseLeave;
    property OnClick;
    property ShowHint;
  end;

procedure Register;

implementation
{$R ALed.res}

constructor TALed.Create(AOwner:TComponent);
begin
   inherited Create(AOwner);
   Height      := 16;
   Width       := 16;
   fTrueColor  := clLime;
   fFalseColor := clSilver;
   fBlink      := false;
   fValue      := false;
   fLEDStyle    := LEDSmall ;
   fInterval   := 1000;
   fLedTimer   := nil;
   fLedBitmap[0] := nil;
   fLedBitmap[1] := nil;
   ColorTemp   := true;
   CreateLedBitmap;
end;

destructor TALed.Destroy;
begin
   ResetLedTimer;
   FreeLedBitmap;
   inherited;
end;

procedure TALed.CreateLedBitmap;
begin
   FreeLedBitmap;
   fLedBitmap[0] := TBitmap.Create;
   fLedBitmap[1] := TBitmap.Create;
   case fLEDStyle of
   LEDSmall:
     begin
     Width := 22 ;
     Height := 22 ;
     fLedBitmap[0].Handle := LoadImage(HINSTANCE,'ALEDSM',IMAGE_BITMAP,0,0,0);
     fLedBitmap[1].Handle := LoadImage(HINSTANCE,'ALEDSM',IMAGE_BITMAP,0,0,0);
     end ;
   LEDLarge:
     begin
     Width := 32 ;
     Height := 32 ;
     fLedBitmap[0].Handle := LoadImage(HINSTANCE,'ALEDLG',IMAGE_BITMAP,0,0,0);
     fLedBitmap[1].Handle := LoadImage(HINSTANCE,'ALEDLG',IMAGE_BITMAP,0,0,0);
     end;
   LEDSqSmall:
     begin
     Width := 22 ;
     Height := 22 ;
     fLedBitmap[0].Handle := LoadImage(HINSTANCE,'ALEDSQSM',IMAGE_BITMAP,0,0,0);
     fLedBitmap[1].Handle := LoadImage(HINSTANCE,'ALEDSQSM',IMAGE_BITMAP,0,0,0);
     end;
   LEDSqLarge:
     begin
     Width := 22 ;
     Height := 22 ;
     fLedBitmap[0].Handle := LoadImage(HINSTANCE,'ALEDSQLG',IMAGE_BITMAP,0,0,0);
     fLedBitmap[1].Handle := LoadImage(HINSTANCE,'ALEDSQLG',IMAGE_BITMAP,0,0,0);
     end;
   LEDHorizontal:
     begin
     Width := 22 ;
     Height := 14 ;
     fLedBitmap[0].Handle := LoadImage(HINSTANCE,'ALEDHZ',IMAGE_BITMAP,0,0,0);
     fLedBitmap[1].Handle := LoadImage(HINSTANCE,'ALEDHZ',IMAGE_BITMAP,0,0,0);
     end;
   LEDVertical:
     begin
     Width := 14 ;
     Height := 22 ;
     fLedBitmap[0].Handle := LoadImage(HINSTANCE,'ALEDVT',IMAGE_BITMAP,0,0,0);
     fLedBitmap[1].Handle := LoadImage(HINSTANCE,'ALEDVT',IMAGE_BITMAP,0,0,0);
     end;

   end ;
   fLedBitmap[0].Canvas.Brush.Color := fTrueColor;
   fLedBitmap[0].Canvas.FloodFill(Width DIV 2,Height DIV 2, clLime, fsSurface);
   fLedBitmap[1].Canvas.Brush.Color := fFalseColor;
   fLedBitmap[1].Canvas.FloodFill(Width DIV 2, Height DIV 2, clLime, fsSurface);
end;

procedure TALed.FreeLedBitmap;
begin
   if (Assigned(fLedBitmap[0])) then fLedBitmap[0].Destroy;
   if (Assigned(fLedBitmap[1])) then fLedBitmap[1].Destroy;
   fLedBitmap[0] := nil;
   fLedBitmap[1] := nil;
end;

procedure TALed.Paint;
begin
    if (Visible) then Canvas.StretchDraw(Rect(0,0,Width,Height),fLedBitmap[integer(ColorTemp)]);    {Steven 20140423 Add if(Visible)}
end;

procedure TALed.OnLedTimer(Sender: TObject);
begin
   ColorTemp := not ColorTemp;
   if (Visible) then Canvas.StretchDraw(Rect(0,0,Width,Height),fLedBitmap[integer(ColorTemp)]);     {Steven 20140423 Add if(Visible)}
   if (Assigned(OnTimer)) then fOnTimer(Self);
end;

procedure TALed.SetToTrueColor;
begin
   if (Visible) then Canvas.StretchDraw(Rect(0,0,Width,Height),fLedBitmap[0]);  {Steven 20140423 Add if(Visible)}
   ColorTemp := false;
end;

procedure TALed.SetToFalseColor;
begin
   if (Visible) then Canvas.StretchDraw(Rect(0,0,Width,Height),fLedBitmap[1]);  {Steven 20140423 Add if(Visible)}
   ColorTemp := true;
end;

procedure TALed.SetLedTimer;
begin
   if (Assigned(fLedTimer)) then Exit;
   if (csDesigning in ComponentState) then Exit;
   ColorTemp := false;
   fLedTimer := TTimer.Create(Self);
   fLedTimer.Interval := fInterval;
   fLedTimer.OnTimer  := OnLedTimer;
end;

procedure TALed.ResetLedTimer;
begin
   if (Assigned(fLedTimer)) then begin
      fLedTimer.Destroy;
      fLedTimer := nil;
   end;
end;


procedure TALed.ChangeStyle(V : TLEDStyle);
begin
   if (fLEDStyle <> V) then begin
      fLEDStyle := V ;
      CreateLedBitmap;
      Repaint;
   end;
end;


procedure TALed.ChangeValue(V : Boolean);
begin
   if (fValue <> V) then begin
       fValue := V;
       if (fValue) then begin
          SetToTrueColor;
          if (fBlink) then
             SetLedTimer;
       end else begin
          ResetLedTimer;
          SetToFalseColor;
       end
       (* fValue == true or false *)
   end; (*(fValue <> V)*)
end;

procedure TALed.ChangeBlink(V : Boolean);
begin
   if (fBlink <> V) then begin
       if (fValue) then
          SetToTrueColor
       else
          SetToFalseColor;
       fBlink := V;
       if (V and fValue) then
          SetLedTimer
       else
          ResetLedTimer
   end;
end;

procedure TALed.SetTrueColor(V : TColor);
var
   Temp : Boolean;
begin
   if (fTrueColor <> V) then begin
      Temp := fBlink;
      if (fBlink) then Blink := false;
      fTrueColor := V;
      CreateLedBitmap;
      Blink := Temp;
      Repaint;
   end
end;

procedure TALed.SetFalseColor(V : TColor);
var
   Temp : Boolean;
begin
   if (fFalseColor <> V) then begin
      Temp := fBlink;
      if (fBlink) then Blink := false;
      fFalseColor := V;
      CreateLedBitmap;
      Blink := Temp;
      Repaint;
   end
end;

procedure TALed.SetInterval(V : longint);
begin
   fInterval := V;
   if (Assigned(fLedTimer)) then
      fLedTimer.Interval := V;
end;

procedure TALed.CmMouseEnter(var Message:TMessage);
begin
   inherited;
   if (Assigned(fOnMouseEnter)) then fOnMouseEnter(Self);
end;

procedure TALed.CmMouseLeave(var Message:TMessage);
begin
   inherited;
   if (Assigned(fOnMouseLeave)) then fOnMouseLeave(Self);
end;

procedure Register;
begin
  RegisterComponents('Extras', [TALed]);
end;

end.
