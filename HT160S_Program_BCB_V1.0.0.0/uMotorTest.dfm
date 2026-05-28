object fMotorTest: TfMotorTest
  Left = 150
  Top = 50
  BorderIcons = [biSystemMenu]
  BorderStyle = bsSingle
  Caption = 'Motor Test'
  ClientHeight = 920
  ClientWidth = 1200
  Color = 12761254
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDefault
  OnClose = FormClose
  OnCreate = FormCreate
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object palMotorControl: TPanel
    Left = 840
    Top = 0
    Width = 360
    Height = 920
    Align = alRight
    BevelInner = bvLowered
    Color = 12761254
    TabOrder = 0
    object lblTitle: TLabel
      Left = 2
      Top = 2
      Width = 356
      Height = 34
      Alignment = taCenter
      AutoSize = False
      Caption = 'Active Motor'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clBlue
      Font.Height = -24
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentColor = False
      ParentFont = False
    end
    object lblNowPos: TLabel
      Left = 12
      Top = 96
      Width = 100
      Height = 20
      AutoSize = False
      Caption = 'Now Position'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblEncoder: TLabel
      Left = 12
      Top = 126
      Width = 100
      Height = 20
      AutoSize = False
      Caption = 'Encoder'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblSpeedPercent: TLabel
      Left = 12
      Top = 156
      Width = 100
      Height = 20
      AutoSize = False
      Caption = 'Speed %'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStep: TLabel
      Left = 12
      Top = 186
      Width = 100
      Height = 20
      AutoSize = False
      Caption = 'Step'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblTarget: TLabel
      Left = 12
      Top = 216
      Width = 100
      Height = 20
      AutoSize = False
      Caption = 'Move To'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblPos1: TLabel
      Left = 12
      Top = 246
      Width = 100
      Height = 20
      AutoSize = False
      Caption = 'Pos1'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblPos2: TLabel
      Left = 12
      Top = 276
      Width = 100
      Height = 20
      AutoSize = False
      Caption = 'Pos2'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblLoopCount: TLabel
      Left = 12
      Top = 306
      Width = 100
      Height = 20
      AutoSize = False
      Caption = 'Loop Count'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblLoopWait: TLabel
      Left = 252
      Top = 306
      Width = 38
      Height = 20
      AutoSize = False
      Caption = 'Wait'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblLoopTrip: TLabel
      Left = 18
      Top = 516
      Width = 104
      Height = 18
      AutoSize = False
      Caption = 'Trip Time'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblLoopTripValue: TLabel
      Left = 132
      Top = 516
      Width = 110
      Height = 18
      AutoSize = False
      Caption = '0.000 sec'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clBlue
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblLoopAverage: TLabel
      Left = 18
      Top = 536
      Width = 104
      Height = 18
      AutoSize = False
      Caption = 'Average'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblLoopAverageValue: TLabel
      Left = 132
      Top = 536
      Width = 110
      Height = 18
      AutoSize = False
      Caption = '0.000 sec'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clBlue
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblLoopTotal: TLabel
      Left = 18
      Top = 556
      Width = 104
      Height = 18
      AutoSize = False
      Caption = 'Total Count'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblLoopTotalValue: TLabel
      Left = 132
      Top = 556
      Width = 110
      Height = 18
      AutoSize = False
      Caption = '0'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clBlue
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus0: TLabel
      Left = 18
      Top = 584
      Width = 94
      Height = 18
      AutoSize = False
      Caption = 'CW'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus1: TLabel
      Left = 18
      Top = 608
      Width = 94
      Height = 18
      AutoSize = False
      Caption = 'HOME'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus2: TLabel
      Left = 18
      Top = 632
      Width = 94
      Height = 18
      AutoSize = False
      Caption = 'CCW'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus3: TLabel
      Left = 18
      Top = 656
      Width = 94
      Height = 18
      AutoSize = False
      Caption = 'EMG'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus4: TLabel
      Left = 18
      Top = 680
      Width = 94
      Height = 18
      AutoSize = False
      Caption = 'ALARM'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus5: TLabel
      Left = 18
      Top = 704
      Width = 94
      Height = 18
      AutoSize = False
      Caption = 'Soft CW'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus6: TLabel
      Left = 186
      Top = 584
      Width = 94
      Height = 18
      AutoSize = False
      Caption = 'Soft CCW'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus7: TLabel
      Left = 186
      Top = 608
      Width = 94
      Height = 18
      AutoSize = False
      Caption = 'Servo Alarm'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus8: TLabel
      Left = 186
      Top = 632
      Width = 94
      Height = 18
      AutoSize = False
      Caption = 'In Pos'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus9: TLabel
      Left = 186
      Top = 656
      Width = 94
      Height = 18
      AutoSize = False
      Caption = 'Z Phase'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblStatus10: TLabel
      Left = 186
      Top = 680
      Width = 94
      Height = 18
      AutoSize = False
      Caption = 'Servo On'
      Color = 8421440
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object lblMotorList: TLabel
      Left = 18
      Top = 730
      Width = 110
      Height = 20
      AutoSize = False
      Caption = 'Motor List'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWindowText
      Font.Height = -13
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object palMotorName: TPanel
      Left = 12
      Top = 44
      Width = 336
      Height = 40
      BevelOuter = bvLowered
      Color = 8421440
      Caption = ''
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -15
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 0
    end
    object edNowPos: TEdit
      Left = 116
      Top = 92
      Width = 124
      Height = 24
      ReadOnly = True
      TabOrder = 1
    end
    object edEncoder: TEdit
      Left = 116
      Top = 122
      Width = 124
      Height = 24
      ReadOnly = True
      TabOrder = 2
    end
    object edSpeedPercent: TEdit
      Left = 116
      Top = 152
      Width = 124
      Height = 24
      TabOrder = 3
      Text = '20'
      OnClick = edMotorInputClick
    end
    object scrSpeedPercent: TScrollBar
      Left = 246
      Top = 152
      Width = 102
      Height = 24
      Min = 1
      PageSize = 0
      Position = 20
      TabOrder = 28
      OnScroll = scrSpeedPercentScroll
    end
    object edStep: TEdit
      Left = 116
      Top = 182
      Width = 124
      Height = 24
      TabOrder = 4
      Text = '1.00'
      OnClick = edMotorInputClick
    end
    object edTarget: TEdit
      Left = 116
      Top = 212
      Width = 124
      Height = 24
      TabOrder = 5
      Text = '0.00'
      OnClick = edMotorInputClick
    end
    object edPos1: TEdit
      Left = 116
      Top = 242
      Width = 124
      Height = 24
      TabOrder = 6
      Text = '0.00'
      OnClick = edMotorInputClick
    end
    object edPos2: TEdit
      Left = 116
      Top = 272
      Width = 124
      Height = 24
      TabOrder = 7
      Text = '0.00'
      OnClick = edMotorInputClick
    end
    object edLoopCount: TEdit
      Left = 116
      Top = 302
      Width = 124
      Height = 24
      TabOrder = 8
      Text = '1'
      OnClick = edMotorInputClick
    end
    object cbbLoopWait: TComboBox
      Left = 292
      Top = 302
      Width = 56
      Height = 21
      ItemHeight = 13
      TabOrder = 29
      Text = '0'
      Items.Strings = (
        '0'
        '0.05'
        '0.1'
        '0.2'
        '0.5'
        '1.0'
        '2.0'
        '5.0'
        '10.0')
    end
    object btnMove: TButton
      Left = 252
      Top = 210
      Width = 96
      Height = 28
      Caption = 'MOVE'
      TabOrder = 9
      OnClick = btnMoveClick
    end
    object btnSetPos1: TButton
      Left = 252
      Top = 240
      Width = 46
      Height = 28
      Caption = 'SET'
      TabOrder = 10
      OnClick = btnSetPos1Click
    end
    object btnGoPos1: TButton
      Left = 302
      Top = 240
      Width = 46
      Height = 28
      Caption = 'GO'
      TabOrder = 11
      OnClick = btnGoPos1Click
    end
    object btnSetPos2: TButton
      Left = 252
      Top = 270
      Width = 46
      Height = 28
      Caption = 'SET'
      TabOrder = 12
      OnClick = btnSetPos2Click
    end
    object btnGoPos2: TButton
      Left = 302
      Top = 270
      Width = 46
      Height = 28
      Caption = 'GO'
      TabOrder = 13
      OnClick = btnGoPos2Click
    end
    object btnJogP: TButton
      Left = 18
      Top = 342
      Width = 90
      Height = 34
      Caption = 'JOG +'
      TabOrder = 14
      OnMouseDown = btnJogPMouseDown
      OnMouseUp = btnJogMouseUp
    end
    object btnJogN: TButton
      Left = 124
      Top = 342
      Width = 90
      Height = 34
      Caption = 'JOG -'
      TabOrder = 15
      OnMouseDown = btnJogNMouseDown
      OnMouseUp = btnJogMouseUp
    end
    object btnStepP: TButton
      Left = 230
      Top = 342
      Width = 54
      Height = 34
      Caption = '+'
      TabOrder = 16
      OnClick = btnStepPClick
    end
    object btnStepN: TButton
      Left = 292
      Top = 342
      Width = 54
      Height = 34
      Caption = '-'
      TabOrder = 17
      OnClick = btnStepNClick
    end
    object btnHome: TButton
      Left = 18
      Top = 386
      Width = 96
      Height = 34
      Caption = 'HOME'
      TabOrder = 18
      OnClick = btnHomeClick
    end
    object btnStop: TButton
      Left = 132
      Top = 386
      Width = 96
      Height = 34
      Caption = 'STOP'
      TabOrder = 19
      OnClick = btnStopClick
    end
    object btnRefresh: TButton
      Left = 246
      Top = 386
      Width = 96
      Height = 34
      Caption = 'REFRESH'
      TabOrder = 20
      OnClick = btnRefreshClick
    end
    object btnLoopStart: TButton
      Left = 18
      Top = 430
      Width = 96
      Height = 34
      Caption = 'LOOP'
      TabOrder = 21
      OnClick = btnLoopStartClick
    end
    object btnLoopStop: TButton
      Left = 132
      Top = 430
      Width = 96
      Height = 34
      Caption = 'LOOP STOP'
      TabOrder = 22
      OnClick = btnLoopStopClick
    end
    object btnSave: TButton
      Left = 246
      Top = 430
      Width = 96
      Height = 34
      Caption = 'SAVE'
      TabOrder = 23
      OnClick = btnSaveClick
    end
    object chkMultiLoop: TCheckBox
      Left = 246
      Top = 410
      Width = 100
      Height = 17
      Caption = 'MULTI LOOP'
      TabOrder = 32
    end
    object btnReload: TButton
      Left = 18
      Top = 474
      Width = 96
      Height = 34
      Caption = 'RELOAD'
      TabOrder = 24
      OnClick = btnReloadClick
    end
    object btnGoSoftN: TButton
      Left = 132
      Top = 474
      Width = 54
      Height = 34
      Caption = 'SOFT -'
      TabOrder = 30
      OnClick = btnGoSoftNClick
    end
    object btnGoSoftP: TButton
      Left = 192
      Top = 474
      Width = 54
      Height = 34
      Caption = 'SOFT +'
      TabOrder = 31
      OnClick = btnGoSoftPClick
    end
    object btnClose: TButton
      Left = 254
      Top = 474
      Width = 88
      Height = 34
      Caption = 'CLOSE'
      TabOrder = 25
      OnClick = btnCloseClick
    end
    object ledStatus0: TALed
      Left = 116
      Top = 584
      Width = 22
      Height = 22
      FalseColor = 12632256
      TrueColor = 65280
      LEDStyle = LEDSqLarge
    end
    object ledStatus1: TALed
      Left = 116
      Top = 608
      Width = 22
      Height = 22
      FalseColor = 12632256
      TrueColor = 65280
      LEDStyle = LEDSqLarge
    end
    object ledStatus2: TALed
      Left = 116
      Top = 632
      Width = 22
      Height = 22
      FalseColor = 12632256
      TrueColor = 65280
      LEDStyle = LEDSqLarge
    end
    object ledStatus3: TALed
      Left = 116
      Top = 656
      Width = 22
      Height = 22
      FalseColor = 12632256
      TrueColor = 255
      LEDStyle = LEDSqLarge
    end
    object ledStatus4: TALed
      Left = 116
      Top = 680
      Width = 22
      Height = 22
      FalseColor = 12632256
      TrueColor = 255
      LEDStyle = LEDSqLarge
    end
    object ledStatus5: TALed
      Left = 116
      Top = 704
      Width = 22
      Height = 22
      FalseColor = 12632256
      TrueColor = 65280
      LEDStyle = LEDSqLarge
    end
    object ledStatus6: TALed
      Left = 284
      Top = 584
      Width = 22
      Height = 22
      FalseColor = 12632256
      TrueColor = 65280
      LEDStyle = LEDSqLarge
    end
    object ledStatus7: TALed
      Left = 284
      Top = 608
      Width = 22
      Height = 22
      FalseColor = 12632256
      TrueColor = 255
      LEDStyle = LEDSqLarge
    end
    object ledStatus8: TALed
      Left = 284
      Top = 632
      Width = 22
      Height = 22
      FalseColor = 12632256
      TrueColor = 65280
      LEDStyle = LEDSqLarge
    end
    object ledStatus9: TALed
      Left = 284
      Top = 656
      Width = 22
      Height = 22
      FalseColor = 12632256
      TrueColor = 65280
      LEDStyle = LEDSqLarge
    end
    object ledStatus10: TALed
      Left = 284
      Top = 680
      Width = 22
      Height = 22
      FalseColor = 12632256
      TrueColor = 65280
      LEDStyle = LEDSqLarge
    end
    object lstMotors: TListBox
      Left = 18
      Top = 752
      Width = 328
      Height = 124
      ItemHeight = 13
      MultiSelect = True
      TabOrder = 26
      OnClick = lstMotorsClick
    end
    object palMessage: TPanel
      Left = 18
      Top = 884
      Width = 328
      Height = 28
      Alignment = taLeftJustify
      BevelOuter = bvLowered
      Caption = 'Ready'
      TabOrder = 27
    end
  end
  object PageMotorTest: TPageControl
    Left = 0
    Top = 0
    Width = 840
    Height = 920
    ActivePage = tsOperate
    Align = alClient
    TabOrder = 1
    object tsOperate: TTabSheet
      Caption = 'Operate'
      object grdOperate: TStringGrid
        Left = 0
        Top = 0
        Width = 832
        Height = 892
        Align = alClient
        ColCount = 8
        DefaultRowHeight = 24
        FixedCols = 0
        FixedRows = 1
        RowCount = 2
        TabOrder = 0
      end
    end
    object tsMotorParameter: TTabSheet
      Caption = 'Motor Parameter'
      ImageIndex = 1
      object palMotorParameterTools: TPanel
        Left = 0
        Top = 0
        Width = 832
        Height = 44
        Align = alTop
        BevelOuter = bvLowered
        Color = 12761254
        TabOrder = 0
        object btnParamSave: TButton
          Left = 8
          Top = 8
          Width = 112
          Height = 28
          Caption = 'SAVE PARAM'
          TabOrder = 0
          OnClick = btnParamSaveClick
        end
        object btnParamReload: TButton
          Left = 128
          Top = 8
          Width = 112
          Height = 28
          Caption = 'RELOAD PARAM'
          TabOrder = 1
          OnClick = btnParamReloadClick
        end
        object btnParamValidate: TButton
          Left = 248
          Top = 8
          Width = 112
          Height = 28
          Caption = 'VALIDATE ALL'
          TabOrder = 2
          OnClick = btnParamValidateClick
        end
      end
      object grdMotorParameter: TStringGrid
        Left = 0
        Top = 44
        Width = 832
        Height = 848
        Align = alClient
        ColCount = 16
        DefaultRowHeight = 24
        FixedCols = 0
        FixedRows = 1
        RowCount = 2
        TabOrder = 1
        OnDblClick = grdMotorParameterDblClick
        OnSelectCell = grdMotorParameterSelectCell
      end
    end
    object tsMotorTable: TTabSheet
      Caption = 'Mot Table View'
      ImageIndex = 2
      object palMotorTableTools: TPanel
        Left = 0
        Top = 0
        Width = 832
        Height = 44
        Align = alTop
        BevelOuter = bvLowered
        Color = 12761254
        TabOrder = 0
        object lblMotorTableSearch: TLabel
          Left = 128
          Top = 14
          Width = 48
          Height = 16
          AutoSize = False
          Caption = 'Search'
          Color = 12761254
          ParentColor = False
        end
        object btnMotorTableReload: TButton
          Left = 8
          Top = 8
          Width = 112
          Height = 28
          Caption = 'RELOAD TABLE'
          TabOrder = 0
          OnClick = btnMotorTableReloadClick
        end
        object edMotorTableSearch: TEdit
          Left = 180
          Top = 10
          Width = 180
          Height = 21
          TabOrder = 1
        end
        object btnMotorTableFind: TButton
          Left = 368
          Top = 8
          Width = 78
          Height = 28
          Caption = 'FIND'
          TabOrder = 2
          OnClick = btnMotorTableFindClick
        end
        object btnMotorTableLocateActive: TButton
          Left = 454
          Top = 8
          Width = 118
          Height = 28
          Caption = 'LOCATE ACTIVE'
          TabOrder = 3
          OnClick = btnMotorTableLocateActiveClick
        end
        object btnMotorTableEdit: TButton
          Left = 580
          Top = 8
          Width = 86
          Height = 28
          Caption = 'EDIT CELL'
          TabOrder = 4
          OnClick = btnMotorTableEditClick
        end
        object btnMotorTableSave: TButton
          Left = 674
          Top = 8
          Width = 86
          Height = 28
          Caption = 'SAVE TABLE'
          TabOrder = 5
          OnClick = btnMotorTableSaveClick
        end
      end
      object grdMotorTable: TStringGrid
        Left = 0
        Top = 44
        Width = 832
        Height = 848
        Align = alClient
        ColCount = 1
        DefaultRowHeight = 24
        FixedCols = 0
        FixedRows = 1
        RowCount = 2
        TabOrder = 1
        OnDblClick = grdMotorTableDblClick
      end
    end
    object tsInformation: TTabSheet
      Caption = 'Information'
      ImageIndex = 3
      object grdInformation: TStringGrid
        Left = 0
        Top = 0
        Width = 832
        Height = 892
        Align = alClient
        ColCount = 8
        DefaultRowHeight = 24
        FixedCols = 0
        FixedRows = 1
        RowCount = 2
        TabOrder = 0
      end
    end
    object tsDriverRegister: TTabSheet
      Caption = 'Driver Register'
      ImageIndex = 4
      object palDriverRegisterTools: TPanel
        Left = 0
        Top = 0
        Width = 832
        Height = 44
        Align = alTop
        BevelOuter = bvLowered
        Color = 12761254
        TabOrder = 0
        object lblRegisterOffset: TLabel
          Left = 8
          Top = 14
          Width = 80
          Height = 16
          AutoSize = False
          Caption = 'Offset'
          Color = 12761254
          ParentColor = False
        end
        object edRegisterOffset: TEdit
          Left = 94
          Top = 10
          Width = 84
          Height = 21
          TabOrder = 0
          Text = '0x08'
        end
        object btnRegisterRead: TButton
          Left = 188
          Top = 8
          Width = 112
          Height = 28
          Caption = 'READ OFFSET'
          TabOrder = 1
          OnClick = btnRegisterReadClick
        end
        object btnRegisterReadDefault: TButton
          Left = 308
          Top = 8
          Width = 128
          Height = 28
          Caption = 'READ DEFAULTS'
          TabOrder = 2
          OnClick = btnRegisterReadDefaultClick
        end
      end
      object grdDriverRegister: TStringGrid
        Left = 0
        Top = 44
        Width = 832
        Height = 848
        Align = alClient
        ColCount = 3
        DefaultRowHeight = 24
        FixedCols = 0
        FixedRows = 1
        RowCount = 5
        TabOrder = 1
      end
    end
    object tsServoGuard: TTabSheet
      Caption = 'Servo Guard'
      ImageIndex = 5
      object palServoGuardTools: TPanel
        Left = 0
        Top = 0
        Width = 832
        Height = 44
        Align = alTop
        BevelOuter = bvLowered
        Color = 12761254
        TabOrder = 0
        object btnServoGuardOn: TButton
          Left = 8
          Top = 8
          Width = 142
          Height = 28
          Caption = 'SERVO ON GUARD'
          TabOrder = 0
          OnClick = btnServoGuardOnClick
        end
        object btnServoGuardOff: TButton
          Left = 158
          Top = 8
          Width = 142
          Height = 28
          Caption = 'SERVO OFF GUARD'
          TabOrder = 1
          OnClick = btnServoGuardOffClick
        end
        object btnServoApplyOn: TButton
          Left = 316
          Top = 8
          Width = 142
          Height = 28
          Caption = 'SERVO ON APPLY'
          TabOrder = 2
          OnClick = btnServoApplyOnClick
        end
        object btnServoApplyOff: TButton
          Left = 466
          Top = 8
          Width = 142
          Height = 28
          Caption = 'SERVO OFF APPLY'
          TabOrder = 3
          OnClick = btnServoApplyOffClick
        end
      end
      object grdServoGuard: TStringGrid
        Left = 0
        Top = 44
        Width = 832
        Height = 848
        Align = alClient
        ColCount = 3
        DefaultRowHeight = 24
        FixedCols = 0
        FixedRows = 1
        RowCount = 18
        TabOrder = 1
      end
    end
  end
  object tmrUpdate: TTimer
    Enabled = False
    Interval = 200
    OnTimer = tmrUpdateTimer
    Left = 800
    Top = 16
  end
end