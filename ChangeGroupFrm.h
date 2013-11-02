//---------------------------------------------------------------------------
// Copyright (C) 2013 Krzysztof Grochocki
//
// This file is part of FacebookGroupsManager
//
// FacebookGroupsManager is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// FacebookGroupsManager is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GNU Radio; see the file COPYING. If not, write to
// the Free Software Foundation, Inc., 51 Franklin Street,
// Boston, MA 02110-1301, USA.
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
#ifndef ChangeGroupFrmH
#define ChangeGroupFrmH
#define WM_ALPHAWINDOWS (WM_USER + 666)
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include "sBevel.hpp"
#include "sButton.hpp"
#include <Vcl.ExtCtrls.hpp>
#include "sEdit.hpp"
#include "sSkinManager.hpp"
#include "sSkinProvider.hpp"
#include <System.Actions.hpp>
#include <Vcl.ActnList.hpp>
#include "sCheckBox.hpp"
//---------------------------------------------------------------------------
class TChangeGroupForm : public TForm
{
__published:	// IDE-managed Components
	TsButton *OKButton;
	TsButton *CancelButton;
	TsSkinProvider *sSkinProvider;
	TsSkinManager *sSkinManager;
	TsEdit *sGroupEdit;
	TActionList *ActionList;
	TAction *aExit;
	TsCheckBox *sChangeForAllCheckBox;
	TAction *aEnterPressed;
	void __fastcall FormCreate(TObject *Sender);
	void __fastcall aExitExecute(TObject *Sender);
	void __fastcall sGroupEditChange(TObject *Sender);
	void __fastcall OKButtonClick(TObject *Sender);
	void __fastcall aEnterPressedExecute(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TChangeGroupForm(TComponent* Owner);
	UnicodeString OryginalGroupName;
	void __fastcall WMTransparency(TMessage &Message);
	UnicodeString __fastcall IdHTTPGet(UnicodeString URL);
	BEGIN_MESSAGE_MAP
	MESSAGE_HANDLER(WM_ALPHAWINDOWS,TMessage,WMTransparency);
	END_MESSAGE_MAP(TForm)
};
//---------------------------------------------------------------------------
extern PACKAGE TChangeGroupForm *ChangeGroupForm;
//---------------------------------------------------------------------------
#endif
