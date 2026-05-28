{
 Functions For ASMetr and ASSwitcher Components 1.1 Copyright © 1997
       Andrey Abakumov (aga@oficina.rnd.su)
       Sergey Hrusch
}
unit ASFunc;

interface
 uses
  SysUtils,Windows;
Type
 TStylePrint=(spInside,spExternal);

 Procedure GradValues(TickCount1:Integer);
 Procedure GradToCoords(AngleS,AngleE:Integer;
            var stAngle,enAngle:TPoint);
 Procedure SearchGradForPoint;
 Procedure ValuesTicks(Angle:Integer;
            Radius:Integer);
 Procedure TextValuesForTicks(Style:TStylePrint);

Var
   Offset,
   Offset2,
   Offset4:Integer;
   TickCount:Integer;
   i,j:Integer;
   {Variable for account of corners and
    transformations for drawing ticks,
    inscriptions and other things}
   StartA,
   EndA:Integer;
   midX,midY,
   stX,stY,
   endX,endY: Integer;
   sX,sY,
   eX,eY:Real;
   iGrad:Integer;
   rGrad:Real;

   r,p,p1,p2,p4,t,t1:Real;
   x:Integer;
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

   CoordPointX,
   CoordPointY:Integer;
   PointX,PointY,
   PointX1,PointY1,
   RadTick:Real;
   TmpAn,TmpAn1:Integer;

   wText,
   hText:Integer;
implementation

Procedure ValuesTicks(Angle:Integer;Radius:Integer);
begin
  StartA:=Angle;
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
  RadTick:=midX-Radius;

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
    CoordPointX:=Round(midX+PointX);
    CoordPointY:=Round(midY-PointY);
    end;
  If (StartA>=90)and(StartA<=180)
   Then
    begin
    CoordPointX:=Round(midX-PointX);
    CoordPointY:=Round(midY-PointY);
    end;
  If (StartA>=180)and(StartA<=270)
   Then
    begin
    CoordPointX:=Round(midX-PointX);
    CoordPointY:=Round(midY+PointY);
    end;

  If (StartA>=270)and(StartA<=360)
   Then
    begin
    CoordPointX:=Round(midX+PointX);
    CoordPointY:=Round(midY+PointY);
    end;
end;

Procedure SearchGradForPoint;
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

Procedure GradToCoords(AngleS,AngleE:Integer;
                   var stAngle,enAngle:TPoint);
begin
 sX:=Cos((AngleS/180.0)*pi);
 sY:=Sin((AngleS/180.0)*pi);
 eX:=Cos((AngleE/180.0)*pi);
 eY:=Sin((AngleE/180.0)*pi);

 stAngle.X:=Round(sX*100);
 stAngle.Y:=Round(sY*100);
 enAngle.X:=Round(eX*100);
 enAngle.Y:=Round(eY*100);
end;

Procedure GradValues(TickCount1:Integer);
 procedure Tmp;
 begin
 t:=t+1;
 t1:=t1-1;
 p1:=x*t1;
 p2:=t*x+p;
 p4:=Frac(p2/2);
 end;
begin
 p:=80;
 p1:=360-p;
 x:=TickCount1;
 t:=Frac(p1/x);
 t1:=Trunc(p1/x);
 p1:=x*t1;
 p2:=t*x+p;      
 p4:=Frac(p2/2);
 While p4<>0 Do Tmp;
end;

Procedure TextValuesForTicks(Style:TStylePrint);
begin
If Style=spExternal
 Then
  begin
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
    CoordPointY:=CoordPointY-Round(1.3*hText);
    end;
  end
 Else
  begin
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
    CoordPointY:=CoordPointY-Round(1.3*hText);
    end;
  end
end;

end.
