object fSecsGemLog: TfSecsGemLog
  Left = 200
  Top = 120
  BorderStyle = bsSizeable
  Caption = 'SECS/GEM Monitor'
  ClientHeight = 512
  ClientWidth = 772
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poScreenCenter
  PixelsPerInch = 96
  TextHeight = 13
  object PageControl1: TPageControl
    Left = 0
    Top = 0
    Width = 772
    Height = 512
    ActivePage = tsLog
    Align = alClient
    TabIndex = 0
    TabOrder = 0
    object tsLog: TTabSheet
      Caption = 'Log'
      object MemoLog: TMemo
        Left = 0
        Top = 34
        Width = 764
        Height = 450
        Align = alClient
        Color = clWindow
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -12
        Font.Name = 'Courier New'
        Font.Style = []
        ParentFont = False
        ReadOnly = True
        ScrollBars = ssBoth
        TabOrder = 0
        WordWrap = False
      end
      object PanelTop: TPanel
        Left = 0
        Top = 0
        Width = 764
        Height = 34
        Align = alTop
        BevelOuter = bvNone
        TabOrder = 1
        object LblState: TLabel
          Left = 350
          Top = 10
          Width = 3
          Height = 13
        end
        object BtnClear: TButton
          Left = 8
          Top = 5
          Width = 70
          Height = 24
          Caption = 'Clear'
          TabOrder = 0
          OnClick = BtnClearClick
        end
        object BtnCopy: TButton
          Left = 84
          Top = 5
          Width = 70
          Height = 24
          Caption = 'Copy'
          TabOrder = 1
          OnClick = BtnCopyClick
        end
        object ChkAutoScroll: TCheckBox
          Left = 170
          Top = 9
          Width = 90
          Height = 17
          Caption = 'Auto Scroll'
          Checked = True
          State = cbChecked
          TabOrder = 2
        end
        object ChkPause: TCheckBox
          Left = 268
          Top = 9
          Width = 70
          Height = 17
          Caption = 'Pause'
          TabOrder = 3
        end
      end
    end
    object tsSV: TTabSheet
      Caption = 'SV Query'
      ImageIndex = 1
      object GridSV: TStringGrid
        Left = 0
        Top = 0
        Width = 764
        Height = 484
        Align = alClient
        ColCount = 4
        DefaultRowHeight = 18
        FixedCols = 0
        RowCount = 2
        Options = [goVertLine, goHorzLine, goRowSelect]
        TabOrder = 0
      end
    end
    object tsEC: TTabSheet
      Caption = 'EC Query'
      ImageIndex = 2
      object GridEC: TStringGrid
        Left = 0
        Top = 0
        Width = 764
        Height = 420
        Align = alClient
        ColCount = 5
        DefaultRowHeight = 18
        FixedCols = 0
        RowCount = 2
        Options = [goVertLine, goHorzLine, goRowSelect]
        TabOrder = 0
        OnClick = GridECClick
      end
      object PanelECEdit: TPanel
        Left = 0
        Top = 420
        Width = 764
        Height = 64
        Align = alBottom
        BevelOuter = bvNone
        TabOrder = 1
        object LblECSel: TLabel
          Left = 8
          Top = 6
          Width = 122
          Height = 13
          Caption = 'Select an EC row to edit.'
        end
        object LblECStatus: TLabel
          Left = 316
          Top = 30
          Width = 3
          Height = 13
        end
        object EdtECValue: TEdit
          Left = 8
          Top = 26
          Width = 200
          Height = 21
          Enabled = False
          TabOrder = 0
        end
        object BtnECWrite: TButton
          Left = 216
          Top = 25
          Width = 90
          Height = 25
          Caption = 'Write EC'
          Enabled = False
          TabOrder = 1
          OnClick = BtnECWriteClick
        end
      end
    end
    object tsConn: TTabSheet
      Caption = 'Connection'
      ImageIndex = 3
      object LblConnAddr: TLabel
        Left = 16
        Top = 16
        Width = 3
        Height = 13
      end
      object LblConnPort: TLabel
        Left = 16
        Top = 40
        Width = 3
        Height = 13
      end
      object LblConnDev: TLabel
        Left = 16
        Top = 64
        Width = 3
        Height = 13
      end
      object LblConnMode: TLabel
        Left = 16
        Top = 88
        Width = 3
        Height = 13
      end
      object LblConnState: TLabel
        Left = 16
        Top = 112
        Width = 3
        Height = 13
      end
      object LblConnNote: TLabel
        Left = 16
        Top = 150
        Width = 420
        Height = 13
        Caption =
          'Live status only. Change the endpoint on the Settings tab, then ' +
          'restart.'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clGrayText
        Font.Height = -11
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
      end
    end
    object tsSet: TTabSheet
      Caption = 'Settings'
      ImageIndex = 4
      object LblSetEnable: TLabel
        Left = 16
        Top = 18
        Width = 64
        Height = 13
        Caption = 'Enable GEM'
      end
      object LblSetMode: TLabel
        Left = 16
        Top = 46
        Width = 88
        Height = 13
        Caption = 'Active Mode (dial)'
      end
      object LblSetAddr: TLabel
        Left = 16
        Top = 78
        Width = 95
        Height = 13
        Caption = 'Host Address (IP)'
      end
      object LblSetPort: TLabel
        Left = 16
        Top = 106
        Width = 20
        Height = 13
        Caption = 'Port'
      end
      object LblSetDev: TLabel
        Left = 16
        Top = 134
        Width = 47
        Height = 13
        Caption = 'Device ID'
      end
      object LblSetStatus: TLabel
        Left = 16
        Top = 214
        Width = 3
        Height = 13
      end
      object LblSetNote: TLabel
        Left = 16
        Top = 238
        Width = 500
        Height = 26
        AutoSize = False
        Caption =
          'Address is only used when Active Mode is checked. Saving writes ' +
          'system\General.ini [SECS]; restart ht160s.exe to apply.'
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clGrayText
        Font.Height = -11
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        WordWrap = True
      end
      object ChkSetEnable: TCheckBox
        Left = 160
        Top = 16
        Width = 17
        Height = 17
        TabOrder = 0
      end
      object ChkSetActive: TCheckBox
        Left = 160
        Top = 44
        Width = 17
        Height = 17
        TabOrder = 1
      end
      object EdtSetAddr: TEdit
        Left = 160
        Top = 74
        Width = 160
        Height = 21
        TabOrder = 2
      end
      object EdtSetPort: TEdit
        Left = 160
        Top = 102
        Width = 100
        Height = 21
        TabOrder = 3
      end
      object EdtSetDev: TEdit
        Left = 160
        Top = 130
        Width = 100
        Height = 21
        TabOrder = 4
      end
      object BtnSetSave: TButton
        Left = 160
        Top = 170
        Width = 90
        Height = 26
        Caption = 'Save'
        TabOrder = 5
        OnClick = BtnSetSaveClick
      end
      object BtnSetReload: TButton
        Left = 258
        Top = 170
        Width = 90
        Height = 26
        Caption = 'Reload'
        TabOrder = 6
        OnClick = BtnSetReloadClick
      end
    end
  end
  object PollTimer: TTimer
    Interval = 400
    OnTimer = PollTimerTimer
    Left = 712
    Top = 8
  end
end
