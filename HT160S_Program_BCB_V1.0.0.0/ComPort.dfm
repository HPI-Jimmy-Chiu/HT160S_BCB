object fComPort: TfComPort
  Left = 601
  Top = 232
  Width = 780
  Height = 430
  Caption = 'COM Port'
  Color = 12761254
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -13
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 16
  object pnlTop: TPanel
    Left = 0
    Top = 0
    Width = 684
    Height = 42
    Align = alTop
    Caption = 'Pad COM Port'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -13
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 0
    object sbExit: TButton
      Left = 593
      Top = 8
      Width = 80
      Height = 26
      Anchors = [akTop, akRight]
      Caption = 'Exit'
      TabOrder = 0
      OnClick = sbExitClick
    end
  end
  object pnlSetting: TPanel
    Left = 0
    Top = 42
    Width = 684
    Height = 64
    Align = alTop
    BevelOuter = bvNone
    Color = 12761254
    TabOrder = 1
    object labPadCom: TLabel
      Left = 12
      Top = 22
      Width = 58
      Height = 16
      Caption = 'Pad COM'
    end
    object cbPadComm: TComboBox
      Left = 78
      Top = 18
      Width = 100
      Height = 24
      ItemHeight = 16
      TabOrder = 0
      Text = 'COM1'
      Items.Strings = (
        'COM1'
        'COM2'
        'COM3'
        'COM4'
        'COM5'
        'COM6'
        'COM7'
        'COM8'
        'COM9'
        'COM10'
        'COM11'
        'COM12'
        'COM13'
        'COM14'
        'COM15'
        'COM16'
        'COM17'
        'COM18'
        'COM19'
        'COM20'
        'COM21'
        'COM22'
        'COM23'
        'COM24'
        'COM25'
        'COM26'
        'COM27'
        'COM28'
        'COM29'
        'COM30'
        'COM31'
        'COM32')
    end
    object spbResetCom: TButton
      Left = 190
      Top = 16
      Width = 95
      Height = 25
      Caption = 'Open / Reset'
      TabOrder = 1
      OnClick = spbResetComClick
    end
    object btnStopCom: TButton
      Left = 294
      Top = 16
      Width = 70
      Height = 25
      Caption = 'Close'
      TabOrder = 2
      OnClick = btnStopComClick
    end
    object sbUpdate: TButton
      Left = 372
      Top = 16
      Width = 70
      Height = 25
      Caption = 'Save'
      TabOrder = 3
      OnClick = sbUpdateClick
    end
    object btnClearMemo: TButton
      Left = 450
      Top = 16
      Width = 82
      Height = 25
      Caption = 'Clear Log'
      TabOrder = 4
      OnClick = btnClearMemoClick
    end
    object labPadBaud: TLabel
      Left = 545
      Top = 22
      Width = 36
      Height = 16
      Caption = 'Baud'
    end
    object cbPadBaud: TComboBox
      Left = 590
      Top = 18
      Width = 95
      Height = 24
      ItemHeight = 16
      TabOrder = 5
      Text = '115200'
      Items.Strings = (
        '9600'
        '19200'
        '38400'
        '57600'
        '115200')
    end
  end
  object pnlManual: TPanel
    Left = 0
    Top = 106
    Width = 684
    Height = 54
    Align = alTop
    BevelOuter = bvNone
    Color = 12761254
    TabOrder = 2
    object labManualSend: TLabel
      Left = 12
      Top = 18
      Width = 79
      Height = 16
      Caption = 'Manual Send'
    end
    object edPanelSend_Com: TEdit
      Left = 100
      Top = 14
      Width = 250
      Height = 24
      TabOrder = 0
      Text = 't051120'
    end
    object sbPanelSend_Com: TButton
      Left = 360
      Top = 13
      Width = 70
      Height = 25
      Caption = 'Send'
      TabOrder = 1
      OnClick = sbPanelSend_ComClick
    end
  end
  object pnlLog: TPanel
    Left = 0
    Top = 160
    Width = 684
    Height = 231
    Align = alClient
    BevelOuter = bvNone
    Color = 12761254
    TabOrder = 3
    object memoPanelCom: TMemo
      Left = 0
      Top = 0
      Width = 684
      Height = 231
      Align = alClient
      ScrollBars = ssVertical
      TabOrder = 0
    end
  end
  object PadComm: TComm
    CommName = 'COM1'
    BaudRate = 115200
    ParityCheck = False
    Outx_CtsFlow = False
    Outx_DsrFlow = False
    DtrControl = DtrEnable
    DsrSensitivity = False
    TxContinueOnXoff = False
    Outx_XonXoffFlow = False
    Inx_XonXoffFlow = False
    ReplaceWhenParityError = False
    IgnoreNullChar = False
    RtsControl = RtsEnable
    XonLimit = 0
    XoffLimit = 0
    ByteSize = _8
    Parity = None
    StopBits = _1
    XonChar = #0
    XoffChar = #0
    ReplacedChar = #0
    ReadIntervalTimeout = 1
    ReadTotalTimeoutMultiplier = 0
    ReadTotalTimeoutConstant = 0
    WriteTotalTimeoutMultiplier = 0
    WriteTotalTimeoutConstant = 0
    OnReceiveData = PadCommReceiveData
    Left = 620
  end
end
