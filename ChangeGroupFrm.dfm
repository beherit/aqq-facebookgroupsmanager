object ChangeGroupForm: TChangeGroupForm
  Left = 0
  Top = 0
  BorderIcons = [biSystemMenu]
  BorderStyle = bsToolWindow
  Caption = 'Zmie'#324' grup'#281
  ClientHeight = 114
  ClientWidth = 226
  Color = clBtnFace
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'Tahoma'
  Font.Style = []
  OldCreateOrder = False
  Position = poDesktopCenter
  OnCreate = FormCreate
  DesignSize = (
    226
    114)
  PixelsPerInch = 96
  TextHeight = 13
  object OKButton: TsButton
    Left = 143
    Top = 83
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'OK'
    Enabled = False
    TabOrder = 2
    OnClick = OKButtonClick
    SkinData.SkinSection = 'BUTTON'
    ExplicitLeft = 103
    ExplicitTop = 97
  end
  object CancelButton: TsButton
    Left = 62
    Top = 83
    Width = 75
    Height = 25
    Anchors = [akRight, akBottom]
    Caption = 'Anuluj'
    TabOrder = 3
    OnClick = aExitExecute
    SkinData.SkinSection = 'BUTTON'
    ExplicitLeft = 22
    ExplicitTop = 97
  end
  object sGroupEdit: TsEdit
    Left = 8
    Top = 22
    Width = 210
    Height = 21
    TabOrder = 0
    OnChange = sGroupEditChange
    SkinData.SkinSection = 'EDIT'
    BoundLabel.Active = True
    BoundLabel.Caption = ' Nowa nazwa grupy:'
    BoundLabel.Indent = 3
    BoundLabel.Font.Charset = DEFAULT_CHARSET
    BoundLabel.Font.Color = clWindowText
    BoundLabel.Font.Height = -11
    BoundLabel.Font.Name = 'Tahoma'
    BoundLabel.Font.Style = []
    BoundLabel.Layout = sclTopLeft
    BoundLabel.MaxWidth = 0
    BoundLabel.UseSkinColor = True
  end
  object sChangeForAllCheckBox: TsCheckBox
    Left = 8
    Top = 49
    Width = 210
    Height = 28
    Caption = 'Zmie'#324' grup'#281' dla wszystkich kontakt'#243'w w grupie'
    AutoSize = False
    TabOrder = 1
    SkinData.SkinSection = 'CHECKBOX'
    ImgChecked = 0
    ImgUnchecked = 0
    WordWrap = True
  end
  object sSkinProvider: TsSkinProvider
    AddedTitle.Font.Charset = DEFAULT_CHARSET
    AddedTitle.Font.Color = clNone
    AddedTitle.Font.Height = -11
    AddedTitle.Font.Name = 'Tahoma'
    AddedTitle.Font.Style = []
    SkinData.SkinManager = sSkinManager
    SkinData.SkinSection = 'FORM'
    TitleButtons = <>
    Left = 32
  end
  object sSkinManager: TsSkinManager
    ExtendedBorders = True
    Active = False
    InternalSkins = <>
    MenuSupport.IcoLineSkin = 'ICOLINE'
    MenuSupport.ExtraLineFont.Charset = DEFAULT_CHARSET
    MenuSupport.ExtraLineFont.Color = clWindowText
    MenuSupport.ExtraLineFont.Height = -11
    MenuSupport.ExtraLineFont.Name = 'Tahoma'
    MenuSupport.ExtraLineFont.Style = []
    SkinDirectory = 'c:\Skins'
    SkinInfo = 'N/A'
    ThirdParty.ThirdEdits = ' '
    ThirdParty.ThirdButtons = 'TButton'
    ThirdParty.ThirdBitBtns = ' '
    ThirdParty.ThirdCheckBoxes = ' '
    ThirdParty.ThirdGroupBoxes = ' '
    ThirdParty.ThirdListViews = ' '
    ThirdParty.ThirdPanels = ' '
    ThirdParty.ThirdGrids = ' '
    ThirdParty.ThirdTreeViews = ' '
    ThirdParty.ThirdComboBoxes = ' '
    ThirdParty.ThirdWWEdits = ' '
    ThirdParty.ThirdVirtualTrees = ' '
    ThirdParty.ThirdGridEh = ' '
    ThirdParty.ThirdPageControl = ' '
    ThirdParty.ThirdTabControl = ' '
    ThirdParty.ThirdToolBar = ' '
    ThirdParty.ThirdStatusBar = ' '
    ThirdParty.ThirdSpeedButton = ' '
    ThirdParty.ThirdScrollControl = ' '
    ThirdParty.ThirdUpDown = ' '
    ThirdParty.ThirdScrollBar = ' '
    ThirdParty.ThirdStaticText = ' '
  end
  object ActionList: TActionList
    Left = 64
    object aExit: TAction
      Caption = 'aExit'
      ShortCut = 27
      OnExecute = aExitExecute
    end
    object aEnterPressed: TAction
      Caption = 'aEnterPressed'
      ShortCut = 13
      OnExecute = aEnterPressedExecute
    end
  end
end
