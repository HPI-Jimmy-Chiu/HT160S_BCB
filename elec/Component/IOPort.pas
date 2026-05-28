//---------------------------------------------------------------------------
//  程式用途:直接PORT I/O函數庫
//  程式說明:使用本系列函數可直接控制I/O PORT,但如果要在NT上I/O,必須
//           搭配使用I/O-permissions-map "driver"
//---------------------------------------------------------------------------
unit ioport;

//---------------------------------------------------------------------------
//  函數的宣告
//---------------------------------------------------------------------------
interface
    function  inportb(port : integer) : byte;           // 由PORT讀回1BYTE資料
    procedure outportb(port : integer; value : byte);   // 將1BYTE資料寫入PORT

    function  inportw( PortNum: word)  : word;          // 由PORT讀回2BYTE資料
    procedure outportw(PortNum: word; Value: word);     // 將2BYTE資料寫入PORT

    function  inportl( PortNum: word)  : longint;       // 由PORT讀回4BYTE資料
    procedure outportl(PortNum: word; Value: longint);  // 將4BYTE資料寫入PORT

procedure Register;

//---------------------------------------------------------------------------
//  函數的定義
//---------------------------------------------------------------------------
implementation


//---------------------------------------------------------------------------
//  由I/O PORT讀取1個BYTE資料
//---------------------------------------------------------------------------
function  inportb(port : integer) : byte;
var
	Value : byte;
begin
	asm                         // 加上inline assembler
    	pusha;
    	mov dx, WORD PTR port;
         in	al, dx;
        mov Value, al;
        popa;
    end;
    Result := Value;
end;

//---------------------------------------------------------------------------
//  由I/O PORT送出1個BYTE資料
//---------------------------------------------------------------------------
procedure outportb(port : integer; value : byte);
begin
	asm
    	pusha;
        mov dx, WORD PTR port;
        mov al, value;
        out dx, al;
        popa;
    end;
end;

//---------------------------------------------------------------------------
//  由I/O PORT讀回2個BYTE資料
//---------------------------------------------------------------------------
function  inportw( PortNum: word): word;
Var
    Value : word;
Begin
    asm
        mov DX, PortNum;
        in  AX, DX;
        mov Value, AX;
    end;
    Result := Value;
end;

//---------------------------------------------------------------------------
//  由I/O PORT送出2個BYTE資料
//---------------------------------------------------------------------------
procedure outportw( PortNum: word; Value: word);
Begin
    asm
        mov DX, PortNum;
        mov AX, Value;
        out DX, AX;
    end;
end;

//---------------------------------------------------------------------------
//  由I/O PORT讀回4個BYTE資料
//---------------------------------------------------------------------------
function  inportl( PortNum: word): longint;
Var
    Value : longint;
Begin
    asm
        mov DX, PortNum;
        in  EAX, DX;
        mov Value, EAX;
    end;
    Result := Value;
end;

//---------------------------------------------------------------------------
//  由I/O PORT送出4個BYTE資料
//---------------------------------------------------------------------------
procedure outportl( PortNum: word; Value: longint);
Begin
    asm
        mov DX, PortNum;
        mov EAX, Value;
        out DX, EAX;
    end;
end;

//---------------------------------------------------------------------------
//  Register CLASS
//---------------------------------------------------------------------------
procedure Register;
begin
end; { of Register }

end.

