object fNote: TfNote
  Left = 214
  Top = 79
  BorderIcons = []
  BorderStyle = bsSingle
  Caption = 'Note'
  ClientHeight = 760
  ClientWidth = 1180
  Color = 12761254
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  Position = poDefault
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object Panel7: TPanel
    Left = 0
    Top = 0
    Width = 1180
    Height = 52
    Align = alTop
    BevelOuter = bvNone
    Color = 12761254
    TabOrder = 0
    object Label2: TLabel
      Left = 8
      Top = 4
      Width = 778
      Height = 20
      AutoSize = False
      Caption = '  Err Code                          Message'
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clRed
      Font.Height = -16
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentColor = False
      ParentFont = False
    end
    object edtAlarmCode: TEdit
      Left = 7
      Top = 21
      Width = 154
      Height = 28
      Color = 14145495
      Enabled = False
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clRed
      Font.Height = -16
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 0
    end
    object edtAlarmMsg: TEdit
      Left = 176
      Top = 21
      Width = 980
      Height = 28
      Color = clWhite
      Enabled = False
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clRed
      Font.Height = -16
      Font.Name = 'MS Sans Serif'
      Font.Style = [fsBold]
      ParentFont = False
      TabOrder = 1
    end
  end
  object Memo1: TMemo
    Left = 0
    Top = 620
    Width = 1180
    Height = 140
    Align = alBottom
    Enabled = False
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    ScrollBars = ssVertical
    TabOrder = 2
  end
  object Panel1: TPanel
    Left = 0
    Top = 52
    Width = 1180
    Height = 568
    Align = alClient
    BevelOuter = bvNone
    Color = 12761254
    TabOrder = 1
    object PanelCommand: TPanel
      Left = 0
      Top = 0
      Width = 157
      Height = 568
      Align = alLeft
      BevelOuter = bvNone
      Color = 12761254
      TabOrder = 0
      object BtnHome: TPanel
        Tag = 6
        Left = 10
        Top = 7
        Width = 135
        Height = 33
        BevelInner = bvRaised
        Caption = 'HOME && RETRY'
        Color = 8404992
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -16
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 0
        OnClick = BtnSkipClick
      end
      object BtnSkip: TPanel
        Tag = 1
        Left = 10
        Top = 56
        Width = 135
        Height = 33
        BevelInner = bvRaised
        Caption = 'SKIP'
        Color = 8421440
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -16
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 1
        OnClick = BtnSkipClick
      end
      object BtnRetry: TPanel
        Tag = 2
        Left = 10
        Top = 106
        Width = 135
        Height = 33
        BevelInner = bvRaised
        Caption = 'RETRY'
        Color = 8404992
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -16
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 2
        OnClick = BtnSkipClick
      end
      object BtnTrayFeed: TPanel
        Tag = 3
        Left = 10
        Top = 155
        Width = 135
        Height = 33
        BevelInner = bvRaised
        Caption = 'TRAY FEED'
        Color = 8404992
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -16
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 3
        OnClick = BtnSkipClick
      end
      object BtnTrayEnd: TPanel
        Tag = 4
        Left = 10
        Top = 205
        Width = 135
        Height = 33
        BevelInner = bvRaised
        Caption = 'TRAY END'
        Color = 8404992
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -16
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 4
        OnClick = BtnSkipClick
      end
      object BtnCleanOut: TPanel
        Tag = 5
        Left = 10
        Top = 254
        Width = 135
        Height = 33
        BevelInner = bvRaised
        Caption = 'CLEAN OUT'
        Color = 8404992
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -16
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 5
        OnClick = BtnSkipClick
      end
      object edtManual2D: TEdit
        Left = 10
        Top = 303
        Width = 135
        Height = 28
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWindowText
        Font.Height = -16
        Font.Name = 'MS Sans Serif'
        Font.Style = [fsBold]
        ParentFont = False
        TabOrder = 9
        OnKeyPress = edtManual2DKeyPress
      end
      object BtnStart: TPanel
        Tag = 7
        Left = 10
        Top = 439
        Width = 135
        Height = 33
        BevelInner = bvRaised
        Caption = 'START'
        Color = clBlue
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -16
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 6
        OnClick = BtnStartClick
      end
      object BtnPause: TPanel
        Tag = 8
        Left = 10
        Top = 489
        Width = 135
        Height = 33
        BevelInner = bvRaised
        Caption = 'PAUSE'
        Color = 8404992
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -16
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 7
        OnClick = BtnPauseClick
      end
      object BtnOffBuzzer: TPanel
        Tag = 9
        Left = 10
        Top = 530
        Width = 135
        Height = 33
        BevelInner = bvRaised
        Caption = 'Off Buzzer'
        Color = 8404992
        Font.Charset = DEFAULT_CHARSET
        Font.Color = clWhite
        Font.Height = -16
        Font.Name = 'MS Sans Serif'
        Font.Style = []
        ParentFont = False
        TabOrder = 8
        OnClick = BtnOffBuzzerClick
      end
    end
    object PanelMain6: TPanel
      Left = 157
      Top = 0
      Width = 1023
      Height = 568
      Align = alClient
      BevelInner = bvLowered
      Color = 12761254
      Font.Charset = DEFAULT_CHARSET
      Font.Color = clWhite
      Font.Height = -11
      Font.Name = 'MS Sans Serif'
      Font.Style = []
      ParentFont = False
      TabOrder = 1
    end
  end
  object Timer1: TTimer
    Interval = 250
    OnTimer = Timer1Timer
    Left = 112
    Top = 472
  end
end
