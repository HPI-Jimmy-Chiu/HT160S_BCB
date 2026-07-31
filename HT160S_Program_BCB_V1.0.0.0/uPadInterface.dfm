object fPadInterface: TfPadInterface
  Left = 220
  Top = 120
  Width = 840
  Height = 610
  Caption = 'Pad Interface'
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  OldCreateOrder = False
  OnClose = FormClose
  OnShow = FormShow
  PixelsPerInch = 96
  TextHeight = 13
  object pn_PadInterfaceTitle: TPanel
    Left = 0
    Top = 0
    Width = 832
    Height = 40
    Align = alTop
    Caption = 'Pad Interface'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -11
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
    TabOrder = 0
    object sb_PadInterface_Exit: TButton
      Left = 742
      Top = 7
      Width = 80
      Height = 26
      Anchors = [akTop, akRight]
      Caption = 'Exit'
      TabOrder = 0
      OnClick = sb_PadInterface_ExitClick
    end
  end
  object pn_PadInterface_Bottom: TPanel
    Left = 0
    Top = 433
    Width = 832
    Height = 150
    Align = alBottom
    BevelOuter = bvNone
    TabOrder = 1
    object lb_PadInterface_ManualSend: TLabel
      Left = 8
      Top = 10
      Width = 66
      Height = 13
      Caption = 'Manual Send'
    end
    object ed_PadInterface_ManualSend: TEdit
      Left = 92
      Top = 6
      Width = 230
      Height = 21
      TabOrder = 0
      Text = 't051400000000'
    end
    object sb_PadInterface_ManualSend: TButton
      Left = 330
      Top = 5
      Width = 70
      Height = 25
      Caption = 'Send'
      TabOrder = 1
      OnClick = sb_PadInterface_ManualSendClick
    end
    object btnResetCom: TButton
      Left = 408
      Top = 5
      Width = 82
      Height = 25
      Caption = 'Reset COM'
      TabOrder = 2
      OnClick = sb_PadInterface_ManualSendClick
    end
    object btnClearLog: TButton
      Left = 498
      Top = 5
      Width = 82
      Height = 25
      Caption = 'Clear Log'
      TabOrder = 3
      OnClick = ClearLog1Click
    end
    object cb_PadInterface_PadLedBling: TCheckBox
      Left = 590
      Top = 9
      Width = 100
      Height = 17
      Caption = 'Blink LED'
      TabOrder = 4
    end
    object Memo_PadInterface: TMemo
      Left = 8
      Top = 36
      Width = 816
      Height = 106
      Anchors = [akLeft, akTop, akRight, akBottom]
      ScrollBars = ssVertical
      TabOrder = 5
    end
  end
  object pc_PadInterface: TPageControl
    Left = 0
    Top = 40
    Width = 832
    Height = 393
    ActivePage = tsPadFront
    Align = alClient
    TabOrder = 2
    object tsPadFront: TTabSheet
      Caption = 'Front Pad'
      object pn_PadInterface_Front: TPanel
        Left = 0
        Top = 0
        Width = 824
        Height = 365
        Align = alClient
        BevelOuter = bvNone
        TabOrder = 0
      end
    end
    object tsPadRear: TTabSheet
      Caption = 'Rear Pad'
      object pn_PadInterface_Rear: TPanel
        Left = 0
        Top = 0
        Width = 824
        Height = 365
        Align = alClient
        BevelOuter = bvNone
        TabOrder = 0
      end
    end
  end
end
