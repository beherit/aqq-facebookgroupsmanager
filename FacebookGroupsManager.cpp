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

#include <vcl.h>
#include <windows.h>
#pragma hdrstop
#pragma argsused
#include <PluginAPI.h>
#include <inifiles.hpp>
#include <IdHashMessageDigest.hpp>
#include "ChangeGroupFrm.h"

int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{
	return 1;
}
//---------------------------------------------------------------------------

//Struktury-glowne-----------------------------------------------------------
TPluginLink PluginLink;
TPluginInfo PluginInfo;
//Uchwyt-do-okna-timera------------------------------------------------------
HWND hTimerFrm;
//Kontakt-z-popupmenu-muItem-------------------------------------------------
TPluginContact muItemContact;
//Tablica-kontaktow----------------------------------------------------------
TPluginContact ContactsList[1000];
//Definicja-struktury-tablicy-unikatowych-ID-timera--------------------------
struct TIdTable
{
  int TimerId;
  int ContactIndex;
};
//Zmienna-tablicy-unikatowych-ID-timera--------------------------------------
TIdTable IdTable[1000];
//ID-wywolania-enumeracji-listy-kontaktow------------------------------------
DWORD ReplyListID = 0;
//Sciezka-do-pliku-sesji-grup------------------------------------------------
UnicodeString GroupsFileDir;
//Stan-polaczenia-sieci-Facebook---------------------------------------------
bool NetworkConnecting = false;
//FORWARD-TIMER--------------------------------------------------------------
LRESULT CALLBACK TimerFrmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
//FORWARD-AQQ-HOOKS----------------------------------------------------------
INT_PTR __stdcall OnContactsUpdate(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnListReady(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnReplyList(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnSetLastState(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnStateChange(WPARAM wParam, LPARAM lParam);
INT_PTR __stdcall OnSystemPopUp(WPARAM wParam, LPARAM lParam);
//---------------------------------------------------------------------------

//Pobieranie sciezki katalogu prywatnego wtyczek
UnicodeString GetPluginUserDir()
{
  return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETPLUGINUSERDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll);
}
//---------------------------------------------------------------------------

//Pobieranie sciezki skorki kompozycji
UnicodeString GetThemeSkinDir()
{
  return StringReplace((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_GETTHEMEDIR,0,0), "\\", "\\\\", TReplaceFlags() << rfReplaceAll) + "\\\\Skin";
}
//---------------------------------------------------------------------------

//Sprawdzanie czy  wlaczona jest zaawansowana stylizacja okien
bool ChkSkinEnabled()
{
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  delete IniList;
  UnicodeString SkinsEnabled = Settings->ReadString("Settings","UseSkin","1");
  delete Settings;
  return StrToBool(SkinsEnabled);
}
//---------------------------------------------------------------------------

//Pobieranie ustawien animacji AlphaControls
bool ChkThemeAnimateWindows()
{
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  delete IniList;
  UnicodeString AnimateWindowsEnabled = Settings->ReadString("Theme","ThemeAnimateWindows","1");
  delete Settings;
  return StrToBool(AnimateWindowsEnabled);
}
//---------------------------------------------------------------------------
bool ChkThemeGlowing()
{
  TStrings* IniList = new TStringList();
  IniList->SetText((wchar_t*)PluginLink.CallService(AQQ_FUNCTION_FETCHSETUP,0,0));
  TMemIniFile *Settings = new TMemIniFile(ChangeFileExt(Application->ExeName, ".INI"));
  Settings->SetStrings(IniList);
  delete IniList;
  UnicodeString GlowingEnabled = Settings->ReadString("Theme","ThemeGlowing","1");
  delete Settings;
  return StrToBool(GlowingEnabled);
}
//---------------------------------------------------------------------------

//Pobieranie ustawien koloru AlphaControls
int GetHUE()
{
  return (int)PluginLink.CallService(AQQ_SYSTEM_COLORGETHUE,0,0);
}
//---------------------------------------------------------------------------
int GetSaturation()
{
  return (int)PluginLink.CallService(AQQ_SYSTEM_COLORGETSATURATION,0,0);
}
//---------------------------------------------------------------------------

//Pobieranie indeksu wolnego rekordu listy kontaktow
int ReciveFreeContact()
{
  for(int Count=0;Count<1000;Count++)
  {
	if(!ContactsList[Count].cbSize)
	 return Count;
  }
  return -1;
}
//---------------------------------------------------------------------------

//Pobieranie indeksu wolnego rekordu listy unikatowych ID timera
int ReciveFreeTable()
{
  for(int Count=0;Count<1000;Count++)
  {
	if(!IdTable[Count].TimerId)
	 return Count;
  }
  return -1;
}
//---------------------------------------------------------------------------

//Pobieranie indeksu tabeli na podstawie indeksu rekordu listy kontaktow
int ReciveTableIndex(int ID)
{
  for(int Count=0;Count<1000;Count++)
  {
	if(IdTable[Count].ContactIndex==ID)
	 return Count;
  }
  return -1;
}
//---------------------------------------------------------------------------

//Pobieranie indeksu rekordu listy kontaktow na podstawie ID timera
int ReciveContactIndex(int TimerID)
{
  for(int Count=0;Count<1000;Count++)
  {
	if(IdTable[Count].TimerId==TimerID)
	 return IdTable[Count].ContactIndex;
  }
  return -1;
}
//---------------------------------------------------------------------------

//Sprawdzanie czy ID timera zostal wygenerowany przez wtyczke
bool IsContactTimer(int ID)
{
  for(int Count=0;Count<1000;Count++)
  {
	if(IdTable[Count].TimerId==ID)
	 return true;
  }
  return false;
}
//---------------------------------------------------------------------------

//Procka okna timera
LRESULT CALLBACK TimerFrmProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  //Notfikacja timera
  if(uMsg==WM_TIMER)
  {
	//Zatrzymanie timera
	KillTimer(hTimerFrm,wParam);
	//Timer zmiany grupy dla danego kontatku
	if(IsContactTimer(wParam))
	{
	  //Pobieranie indeksu rekordu listy kontaktow na podstawie ID timera
	  int ContactIndex = ReciveContactIndex(wParam);
	  //Pobranie indeksu tabeli na podstawie indeksu rekordu listy kontaktow
	  int TableIndex = ReciveTableIndex(ContactIndex);
	  //Zmiania nazwy grupy
	  PluginLink.CallService(AQQ_CONTACTS_CREATE,0,(LPARAM)&ContactsList[ContactIndex]);
	  //Kasowanie danych nt. kontaktu
	  ZeroMemory(&ContactsList[ContactIndex],sizeof(TPluginContact));
	  IdTable[TableIndex].TimerId = -1;
	  IdTable[TableIndex].ContactIndex = -1;
	}
	//Timer wywolania enumeracji list kontaktow
	else
	{
	  //Pobranie ID dla enumeracji kontaktow
	  ReplyListID = GetTickCount();
	  //Wywolanie enumeracji kontaktow
	  PluginLink.CallService(AQQ_CONTACTS_REQUESTLIST,(WPARAM)ReplyListID,0);
	}

	return 0;
  }

  return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
//---------------------------------------------------------------------------

//Zmiana nazwy grupy dla wszystkich kontaktow
void ChangeGroup(UnicodeString OryginalGroup, UnicodeString UserGroup)
{
  //Odczyt pliku z nazwami grup
  TIniFile *Ini = new TIniFile(GroupsFileDir);
  TStringList *Contacts = new TStringList;
  Ini->ReadSection("Contacts",Contacts);
  //Odczyt ilosci kontaktow
  int ContactsCount = Contacts->Count;
  //Odczyt kolejno wszystkich kontaktow
  for(int Count=0;Count<ContactsCount;Count++)
  {
	//Odczyt identyfikatora kontatku
	UnicodeString JID = Contacts->Strings[Count];
	//Odczyt grupy kontatku
	UnicodeString Group = Ini->ReadString("Contacts",JID,"");
	//Kontakt ze zmienionej grupy
	if(Group==OryginalGroup)
	{
	  //Zapisanie nowej grupy dla kontatku
	  Ini->WriteString("Contacts",JID,UserGroup);
	}
  }
  //Sprzatanie :)
  delete Contacts;
  delete Ini;
  //Wygenerowanie ID dla enumeracji kontaktow
  ReplyListID = GetTickCount();
  //Wywolanie enumeracji kontaktow
  PluginLink.CallService(AQQ_CONTACTS_REQUESTLIST,(WPARAM)ReplyListID,0);
}
//---------------------------------------------------------------------------

//Zmiana nazwy grupy dla kontatku z popupmenu muItem
void muItemChangeGroup(UnicodeString Group)
{
  //Pobranie identyfikatora kontatku
  UnicodeString JID = (wchar_t*)muItemContact.JID;
  //Na co nam ten minus? :)
  JID = JID.Delete(1,1);
  //Zapisanie zmiany do pliku INI
  TIniFile *Ini = new TIniFile(GroupsFileDir);
  Ini->WriteString("Contacts",JID,Group);
  delete Ini;
  //Zmiana nazwy grupy w strukturze
  muItemContact.Groups = (Group+";").w_str();
  //Zmiania nazwy grupy
  PluginLink.CallService(AQQ_CONTACTS_CREATE,0,(LPARAM)&muItemContact);
}
//---------------------------------------------------------------------------

//Serwis zmiany nazwy grupy z popupmenu muItem
INT_PTR __stdcall muItemService(WPARAM wParam, LPARAM lParam)
{
  //Pobranie nazwy grupy kontatku
  UnicodeString Group = (wchar_t*)muItemContact.Groups;
  Group = Group.Delete(Group.Pos(";"),Group.Length());
  //Przypisanie uchwytu do formy
  Application->Handle = (HWND)ChangeGroupForm;
  TChangeGroupForm *hModalChangeGroupForm = new TChangeGroupForm(Application);
  //Pokazanie okna
  hModalChangeGroupForm->sGroupEdit->Text = Group;
  hModalChangeGroupForm->sChangeForAllCheckBox->Caption = hModalChangeGroupForm->sChangeForAllCheckBox->Caption + " \"" + Group + "\"";
  hModalChangeGroupForm->OryginalGroupName = Group;
  hModalChangeGroupForm->OKButton->Enabled = false;
  hModalChangeGroupForm->ShowModal();
  //Usuniecie uchwytu do formy
  delete hModalChangeGroupForm;

  return 0;
}
//---------------------------------------------------------------------------

//Zmiana statusu przycisku
void Change_muItem(bool Visible)
{
  TPluginActionEdit PluginActionEdit;
  PluginActionEdit.cbSize = sizeof(TPluginActionEdit);
  PluginActionEdit.pszName = L"FacebookGroupsManager_muItem";
  PluginActionEdit.Caption = L"Zmieñ grupê";
  PluginActionEdit.Hint = L"";
  PluginActionEdit.Enabled = Visible;
  PluginActionEdit.Visible = Visible;
  PluginActionEdit.IconIndex = -1;
  PluginActionEdit.Checked = false;
  PluginLink.CallService(AQQ_CONTROLS_EDITPOPUPMENUITEM,0,(LPARAM)(&PluginActionEdit));
}
//---------------------------------------------------------------------------

//Usuniecie przycisku z popupmenu muItem
void Destroy_muItem()
{
  //Usuwanie serwisu
  PluginLink.DestroyServiceFunction(muItemService);
  //Usuwanie przycisku
  TPluginAction FacebookGroupsManager_muItem;
  ZeroMemory(&FacebookGroupsManager_muItem, sizeof(TPluginAction));
  FacebookGroupsManager_muItem.cbSize = sizeof(TPluginAction);
  FacebookGroupsManager_muItem.pszName = L"FacebookGroupsManager_muItem";
  PluginLink.CallService(AQQ_CONTROLS_DESTROYPOPUPMENUITEM,0,(LPARAM)(&FacebookGroupsManager_muItem));
}
//---------------------------------------------------------------------------

//Tworzenie przycisku w popupmenu muItem
void Build_muItem()
{
  //Tworzenie serwisu dla przycisku
  PluginLink.CreateServiceFunction(L"sFacebookGroupsManager_muItem",muItemService);
  //Tworzenie przycisku
  TPluginAction FacebookGroupsManager_muItem;
  ZeroMemory(&FacebookGroupsManager_muItem, sizeof(TPluginAction));
  FacebookGroupsManager_muItem.cbSize = sizeof(TPluginAction);
  FacebookGroupsManager_muItem.pszName = L"FacebookGroupsManager_muItem";
  FacebookGroupsManager_muItem.pszCaption = L"Zmieñ grupê";
  FacebookGroupsManager_muItem.Position = 14;
  FacebookGroupsManager_muItem.IconIndex = -1;
  FacebookGroupsManager_muItem.pszService = L"sFacebookGroupsManager_muItem";
  FacebookGroupsManager_muItem.pszPopupName = L"muItem";
  FacebookGroupsManager_muItem.PopupPosition = 0;
  PluginLink.CallService(AQQ_CONTROLS_CREATEPOPUPMENUITEM,0,(LPARAM)(&FacebookGroupsManager_muItem));
}
//---------------------------------------------------------------------------

//Hook na zmiane stanu kontaktu
INT_PTR __stdcall OnContactsUpdate(WPARAM wParam, LPARAM lParam)
{
  //Pobieranie danych kontaktu
  TPluginContact ContactsUpdateContact = *(PPluginContact)wParam;
  //Pobieranie identyfikatora kontaktu
  UnicodeString JID = (wchar_t*)ContactsUpdateContact.JID;
  //Kontakt z sieci Facebook
  if(JID.Pos("chat.facebook.com"))
  {
	//Na co nam ten minus? :)
	JID = JID.Delete(1,1);
	//Pobieranie grupy kontatku
	UnicodeString Groups = (wchar_t*)ContactsUpdateContact.Groups;
	//Pobieranie danych z pliku sesji grup
	TIniFile *Ini = new TIniFile(GroupsFileDir);
	if(!Ini->ValueExists("Contacts",JID)) Ini->WriteString("Contacts",JID,"Facebook");
	UnicodeString UserGroup = Ini->ReadString("Contacts",JID,"Facebook");
	//Grupa kontatku nieprawidlowa
	if(Groups!=UserGroup+";")
	{
      //Pobranie wolnych rekordow
	  int FreeContact = ReciveFreeContact();
	  int FreeTable = ReciveFreeTable();
	  //Wygenerowanie losowego ID timera
	  int TimerID = PluginLink.CallService(AQQ_FUNCTION_GETNUMID,0,0);
	  //Przypisanie danych w tablicy unikatowych ID timera
	  IdTable[FreeTable].TimerId = TimerID;
	  IdTable[FreeTable].ContactIndex = FreeContact;
	  //Zapisanie danych w tablicy kontaktow
	  //cbSize
	  ContactsList[FreeContact].cbSize = ContactsUpdateContact.cbSize;
	  //JID
	  ContactsList[FreeContact].JID = (wchar_t*)realloc(ContactsList[FreeContact].JID, sizeof(wchar_t)*(wcslen(ContactsUpdateContact.JID)+1));
	  memcpy(ContactsList[FreeContact].JID, ContactsUpdateContact.JID, sizeof(wchar_t)*wcslen(ContactsUpdateContact.JID));
	  ContactsList[FreeContact].JID[wcslen(ContactsUpdateContact.JID)] = L'\0';
	  //Nick
	  ContactsList[FreeContact].Nick = (wchar_t*)realloc(ContactsList[FreeContact].Nick, sizeof(wchar_t)*(wcslen(ContactsUpdateContact.Nick)+1));
	  memcpy(ContactsList[FreeContact].Nick, ContactsUpdateContact.Nick, sizeof(wchar_t)*wcslen(ContactsUpdateContact.Nick));
	  ContactsList[FreeContact].Nick[wcslen(ContactsUpdateContact.Nick)] = L'\0';
	  //Resource
	  ContactsList[FreeContact].Resource = (wchar_t*)realloc(ContactsList[FreeContact].Resource, sizeof(wchar_t)*(wcslen(ContactsUpdateContact.Resource)+1));
	  memcpy(ContactsList[FreeContact].Resource, ContactsUpdateContact.Resource, sizeof(wchar_t)*wcslen(ContactsUpdateContact.Resource));
	  ContactsList[FreeContact].Resource[wcslen(ContactsUpdateContact.Resource)] = L'\0';
	  //Groups
	  wchar_t* UserGroupW = (UserGroup+";").w_str();
	  ContactsList[FreeContact].Groups = (wchar_t*)realloc(ContactsList[FreeContact].Groups, sizeof(wchar_t)*(wcslen(UserGroupW)+1));
	  memcpy(ContactsList[FreeContact].Groups, UserGroupW, sizeof(wchar_t)*wcslen(UserGroupW));
	  ContactsList[FreeContact].Groups[wcslen(UserGroupW)] = L'\0';
	  //State
	  ContactsList[FreeContact].State = ContactsUpdateContact.State;
	  //Status
	  ContactsList[FreeContact].Status = (wchar_t*)realloc(ContactsList[FreeContact].Status, sizeof(wchar_t)*(wcslen(ContactsUpdateContact.Status)+1));
	  memcpy(ContactsList[FreeContact].Status, ContactsUpdateContact.Status, sizeof(wchar_t)*wcslen(ContactsUpdateContact.Status));
	  ContactsList[FreeContact].Status[wcslen(ContactsUpdateContact.Status)] = L'\0';
	  //Other
	  ContactsList[FreeContact].Temporary = ContactsUpdateContact.Temporary;
	  ContactsList[FreeContact].FromPlugin = ContactsUpdateContact.FromPlugin;
	  ContactsList[FreeContact].UserIdx = ContactsUpdateContact.UserIdx;
	  ContactsList[FreeContact].Subscription = ContactsUpdateContact.Subscription;
	  ContactsList[FreeContact].IsChat = ContactsUpdateContact.IsChat;
	  //Wlacznie timera zmiany grupy kontatku
	  SetTimer(hTimerFrm,TimerID,100,(TIMERPROC)TimerFrmProc);
	}
	//Zamykanie pliku sesji grup
	delete Ini;
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na zakonczenie ladowania listy kontaktow przy starcie AQQ
INT_PTR __stdcall OnListReady(WPARAM wParam, LPARAM lParam)
{
  //Wygenerowanie losowego ID timera
  int TimerID = PluginLink.CallService(AQQ_FUNCTION_GETNUMID,0,0);
  //Wlacznie timera zmiany grupy kontatku
  SetTimer(hTimerFrm,TimerID,1000,(TIMERPROC)TimerFrmProc);

  return 0;
}
//---------------------------------------------------------------------------

//Hook na enumeracje listy kontatkow
INT_PTR __stdcall OnReplyList(WPARAM wParam, LPARAM lParam)
{
  //Sprawdzanie ID wywolania enumeracji
  if(wParam==ReplyListID)
  {
	//Pobieranie danych kontaktu
	TPluginContact ReplyListContact = *(PPluginContact)lParam;
	//Pobieranie identyfikatora kontaktu
	UnicodeString JID = (wchar_t*)ReplyListContact.JID;
	//Kontakt z sieci Facebook
	if(JID.Pos("chat.facebook.com"))
	{
      //Na co nam ten minus? :)
	  JID = JID.Delete(1,1);
	  //Pobieranie grupy kontatku
	  UnicodeString Groups = (wchar_t*)ReplyListContact.Groups;
	  //Pobieranie danych z pliku sesji grup
	  TIniFile *Ini = new TIniFile(GroupsFileDir);
	  if(!Ini->ValueExists("Contacts",JID)) Ini->WriteString("Contacts",JID,"Facebook");
	  UnicodeString UserGroup = Ini->ReadString("Contacts",JID,"Facebook");
	  //Grupa kontatku nieprawidlowa
	  if(Groups!=UserGroup+";")
	  {
		//Pobranie wolnych rekordow
		int FreeContact = ReciveFreeContact();
		int FreeTable = ReciveFreeTable();
		//Wygenerowanie losowego ID timera
		int TimerID = PluginLink.CallService(AQQ_FUNCTION_GETNUMID,0,0);
		//Przypisanie danych w tablicy unikatowych ID timera
		IdTable[FreeTable].TimerId = TimerID;
		IdTable[FreeTable].ContactIndex = FreeContact;
		//Zapisanie danych w tablicy kontaktow
		//cbSize
		ContactsList[FreeContact].cbSize = ReplyListContact.cbSize;
		//JID
		ContactsList[FreeContact].JID = (wchar_t*)realloc(ContactsList[FreeContact].JID, sizeof(wchar_t)*(wcslen(ReplyListContact.JID)+1));
		memcpy(ContactsList[FreeContact].JID, ReplyListContact.JID, sizeof(wchar_t)*wcslen(ReplyListContact.JID));
		ContactsList[FreeContact].JID[wcslen(ReplyListContact.JID)] = L'\0';
		//Nick
		ContactsList[FreeContact].Nick = (wchar_t*)realloc(ContactsList[FreeContact].Nick, sizeof(wchar_t)*(wcslen(ReplyListContact.Nick)+1));
		memcpy(ContactsList[FreeContact].Nick, ReplyListContact.Nick, sizeof(wchar_t)*wcslen(ReplyListContact.Nick));
		ContactsList[FreeContact].Nick[wcslen(ReplyListContact.Nick)] = L'\0';
		//Resource
		ContactsList[FreeContact].Resource = (wchar_t*)realloc(ContactsList[FreeContact].Resource, sizeof(wchar_t)*(wcslen(ReplyListContact.Resource)+1));
		memcpy(ContactsList[FreeContact].Resource, ReplyListContact.Resource, sizeof(wchar_t)*wcslen(ReplyListContact.Resource));
		ContactsList[FreeContact].Resource[wcslen(ReplyListContact.Resource)] = L'\0';
		//Groups
		wchar_t* UserGroupW = (UserGroup+";").w_str();
		ContactsList[FreeContact].Groups = (wchar_t*)realloc(ContactsList[FreeContact].Groups, sizeof(wchar_t)*(wcslen(UserGroupW)+1));
		memcpy(ContactsList[FreeContact].Groups, UserGroupW, sizeof(wchar_t)*wcslen(UserGroupW));
		ContactsList[FreeContact].Groups[wcslen(UserGroupW)] = L'\0';
		//State
		ContactsList[FreeContact].State = ReplyListContact.State;
		//Status
		ContactsList[FreeContact].Status = (wchar_t*)realloc(ContactsList[FreeContact].Status, sizeof(wchar_t)*(wcslen(ReplyListContact.Status)+1));
		memcpy(ContactsList[FreeContact].Status, ReplyListContact.Status, sizeof(wchar_t)*wcslen(ReplyListContact.Status));
		ContactsList[FreeContact].Status[wcslen(ReplyListContact.Status)] = L'\0';
		//Other
		ContactsList[FreeContact].Temporary = ReplyListContact.Temporary;
		ContactsList[FreeContact].FromPlugin = ReplyListContact.FromPlugin;
		ContactsList[FreeContact].UserIdx = ReplyListContact.UserIdx;
		ContactsList[FreeContact].Subscription = ReplyListContact.Subscription;
		ContactsList[FreeContact].IsChat = ReplyListContact.IsChat;
		//Wlacznie timera zmiany grupy kontatku
		SetTimer(hTimerFrm,TimerID,1000,(TIMERPROC)TimerFrmProc);
	  }
	  //Zamykanie pliku sesji grup
	  delete Ini;
	}
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na polaczenie sieci przy starcie AQQ
INT_PTR __stdcall OnSetLastState(WPARAM wParam, LPARAM lParam)
{
  //Pobieranie ilosci kont
  int UserIdxCount = PluginLink.CallService(AQQ_FUNCTION_GETUSEREXCOUNT,0,0);
  //Sprawdzanie stanu wszystkich sieci
  for(int UserIdx=0;UserIdx<UserIdxCount;UserIdx++)
  {
	//Pobieranie stanu sieci
	TPluginStateChange PluginStateChange;
	PluginLink.CallService(AQQ_FUNCTION_GETNETWORKSTATE,(WPARAM)(&PluginStateChange),UserIdx);
	//Sprawdzenie serwera sieci
	UnicodeString Server = (wchar_t*)PluginStateChange.Server;
	//Sieci Facebook
	if(Server=="chat.facebook.com")
	{
	  //Dalsze pobieranie stanu sieci
	  int NewState = PluginStateChange.NewState;
	  //Connected
	  if(NewState)
	  {
        //Wygenerowanie losowego ID timera
		int TimerID = PluginLink.CallService(AQQ_FUNCTION_GETNUMID,0,0);
        //Wlacznie timera zmiany grupy kontatku
		SetTimer(hTimerFrm,TimerID,1000,(TIMERPROC)TimerFrmProc);
	  }
	}
  }

  return 0;
}
//---------------------------------------------------------------------------

//Notyfikacja zmiany stanu
INT_PTR __stdcall OnStateChange(WPARAM wParam, LPARAM lParam)
{
  //Pobranie danych stanu sieci
  TPluginStateChange StateChange = *(PPluginStateChange)lParam;
  UnicodeString Server = (wchar_t*)StateChange.Server;
  //Zmiana stanu sieci Facebook
  if(Server=="chat.facebook.com")
  {
	//Pobranie dalszych danych stanu sieci
	int NewState = StateChange.NewState;
	bool Authorized = StateChange.Authorized;
	//Connecting
	if((!Authorized)&&(NewState))
	 //Ustawianie stanu polaczenia sieci
	 NetworkConnecting = true;
	//Connected
	else if((NetworkConnecting)&&(Authorized)&&(NewState))
	{
      //Wygenerowanie losowego ID timera
	  int TimerID = PluginLink.CallService(AQQ_FUNCTION_GETNUMID,0,0);
	  //Wlacznie timera zmiany grupy kontatku
	  SetTimer(hTimerFrm,TimerID,1000,(TIMERPROC)TimerFrmProc);
	}
	//Disconnected
	else if((NetworkConnecting)&&(Authorized)&&(!NewState))
	 //Ustawianie stanu polaczenia sieci
	 NetworkConnecting = false;
  }

  return 0;
}
//---------------------------------------------------------------------------

//Hook na pokazywanie popumenu
INT_PTR __stdcall OnSystemPopUp(WPARAM wParam, LPARAM lParam)
{
   //Pobieranie danych popupmenu
  TPluginPopUp PopUp = *(PPluginPopUp)lParam;
  //Pobieranie nazwy popupmenu
  UnicodeString PopUpName = (wchar_t*)PopUp.Name;
  //Popupmenu dostepne spod PPM na kontakcie w oknie kontaktow
  if(PopUpName=="muItem")
  {
    //Pobieranie danych kontatku
	TPluginContact SystemPopUContact = *(PPluginContact)wParam;
    //Pobieranie identyfikatora kontatku
	UnicodeString JID = (wchar_t*)SystemPopUContact.JID;
	//Kontakt z sieci Facebook
	if(JID.Pos("chat.facebook.com"))
	{
	  //Pokazanie przycisku
	  Change_muItem(true);
	  //Zapamietanie struktury kontatku
	  //cbSize
	  muItemContact.cbSize = SystemPopUContact.cbSize;
	  //JID
	  muItemContact.JID = (wchar_t*)realloc(muItemContact.JID, sizeof(wchar_t)*(wcslen(SystemPopUContact.JID)+1));
	  memcpy(muItemContact.JID, SystemPopUContact.JID, sizeof(wchar_t)*wcslen(SystemPopUContact.JID));
	  muItemContact.JID[wcslen(SystemPopUContact.JID)] = L'\0';
	  //Nick
	  muItemContact.Nick = (wchar_t*)realloc(muItemContact.Nick, sizeof(wchar_t)*(wcslen(SystemPopUContact.Nick)+1));
	  memcpy(muItemContact.Nick, SystemPopUContact.Nick, sizeof(wchar_t)*wcslen(SystemPopUContact.Nick));
	  muItemContact.Nick[wcslen(SystemPopUContact.Nick)] = L'\0';
	  //Resource
	  muItemContact.Resource = (wchar_t*)realloc(muItemContact.Resource, sizeof(wchar_t)*(wcslen(SystemPopUContact.Resource)+1));
	  memcpy(muItemContact.Resource, SystemPopUContact.Resource, sizeof(wchar_t)*wcslen(SystemPopUContact.Resource));
	  muItemContact.Resource[wcslen(SystemPopUContact.Resource)] = L'\0';
	  //Groups
	   muItemContact.Groups = (wchar_t*)realloc(muItemContact.Groups, sizeof(wchar_t)*(wcslen(SystemPopUContact.Groups)+1));
	  memcpy(muItemContact.Groups, SystemPopUContact.Groups, sizeof(wchar_t)*wcslen(SystemPopUContact.Groups));
	  muItemContact.Groups[wcslen(SystemPopUContact.Groups)] = L'\0';
	  //State
	  muItemContact.State = SystemPopUContact.State;
	  //Status
	  muItemContact.Status = (wchar_t*)realloc(muItemContact.Status, sizeof(wchar_t)*(wcslen(SystemPopUContact.Status)+1));
	  memcpy(muItemContact.Status, SystemPopUContact.Status, sizeof(wchar_t)*wcslen(SystemPopUContact.Status));
	  muItemContact.Status[wcslen(SystemPopUContact.Status)] = L'\0';
	  //Other
	  muItemContact.Temporary = SystemPopUContact.Temporary;
	  muItemContact.FromPlugin = SystemPopUContact.FromPlugin;
	  muItemContact.UserIdx = SystemPopUContact.UserIdx;
	  muItemContact.Subscription = SystemPopUContact.Subscription;
	  muItemContact.IsChat = SystemPopUContact.IsChat;
	}
	//Kontakt nie z sieci Facebook
	else Change_muItem(false);
  }

  return 0;
}
//---------------------------------------------------------------------------

//Zapisywanie zasobow
void ExtractRes(wchar_t* FileName, wchar_t* ResName, wchar_t* ResType)
{
  TPluginTwoFlagParams PluginTwoFlagParams;
  PluginTwoFlagParams.cbSize = sizeof(TPluginTwoFlagParams);
  PluginTwoFlagParams.Param1 = ResName;
  PluginTwoFlagParams.Param2 = ResType;
  PluginTwoFlagParams.Flag1 = (int)HInstance;
  PluginLink.CallService(AQQ_FUNCTION_SAVERESOURCE,(WPARAM)&PluginTwoFlagParams,(LPARAM)FileName);
}
//---------------------------------------------------------------------------

//Obliczanie sumy kontrolnej pliku
UnicodeString MD5File(UnicodeString FileName)
{
  if(FileExists(FileName))
  {
	UnicodeString Result;
    TFileStream *fs;

	fs = new TFileStream(FileName, fmOpenRead | fmShareDenyWrite);
	try
	{
	  TIdHashMessageDigest5 *idmd5= new TIdHashMessageDigest5();
	  try
	  {
	    Result = idmd5->HashStreamAsHex(fs);
	  }
	  __finally
	  {
	    delete idmd5;
	  }
    }
	__finally
    {
	  delete fs;
    }

    return Result;
  }
  else return 0;
}
//---------------------------------------------------------------------------

//Zaladowanie wtyczki
extern "C" INT_PTR __declspec(dllexport) __stdcall Load(PPluginLink Link)
{
  //Linkowanie wtyczki z komunikatorem
  PluginLink = *Link;
  //Pobranie sciezki do katalogu prywatnego uzytkownika
  UnicodeString PluginUserDir = GetPluginUserDir();
  //Pobranie sciezki do pliku sesji grup
  GroupsFileDir = PluginUserDir + "\\\\FacebookGroupsManager\\\\Groups.ini";
  //Folder z ustawieniami wtyczki
  if(!DirectoryExists(PluginUserDir + "\\\\FacebookGroupsManager"))
   CreateDir(PluginUserDir + "\\\\FacebookGroupsManager");
  //Wypakiwanie ikonki FacebookGroupsManager.dll.png
  //7B8319600BD786BDC6D737E35E99AD7B
  if(!DirectoryExists(PluginUserDir + "\\\\Shared"))
   CreateDir(PluginUserDir + "\\\\Shared");
  if(!FileExists(PluginUserDir + "\\\\Shared\\\\FacebookGroupsManager.dll.png"))
   ExtractRes((PluginUserDir + "\\\\Shared\\\\FacebookGroupsManager.dll.png").w_str(),L"SHARED",L"DATA");
  else if(MD5File(PluginUserDir + "\\\\Shared\\\\FacebookGroupsManager.dll.png")!="7B8319600BD786BDC6D737E35E99AD7B")
   ExtractRes((PluginUserDir + "\\\\Shared\\\\FacebookGroupsManager.dll.png").w_str(),L"SHARED",L"DATA");
  //Tworzenie przycisku w popupmenu muItem
  Build_muItem();
  //Hook na zmiane stanu kontaktu
  PluginLink.HookEvent(AQQ_CONTACTS_UPDATE,OnContactsUpdate);
  //Hook na zakonczenie ladowania listy kontaktow przy starcie AQQ
  PluginLink.HookEvent(AQQ_CONTACTS_LISTREADY,OnListReady);
  //Hook na enumeracje listy kontatkow
  PluginLink.HookEvent(AQQ_CONTACTS_REPLYLIST,OnReplyList);
  //Hook na polaczenie sieci przy starcie AQQ
  PluginLink.HookEvent(AQQ_SYSTEM_SETLASTSTATE,OnSetLastState);
  //Hook dla zmiany stanu
  PluginLink.HookEvent(AQQ_SYSTEM_STATECHANGE,OnStateChange);
  //Hook na pokazywanie popumenu
  PluginLink.HookEvent(AQQ_SYSTEM_POPUP,OnSystemPopUp);
  //Rejestowanie klasy okna timera
  WNDCLASSEX wincl;
  wincl.cbSize = sizeof(WNDCLASSEX);
  wincl.style = 0;
  wincl.lpfnWndProc = TimerFrmProc;
  wincl.cbClsExtra = 0;
  wincl.cbWndExtra = 0;
  wincl.hInstance = HInstance;
  wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
  wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;
  wincl.lpszMenuName = NULL;
  wincl.lpszClassName = L"TFacebookGroupsManagerTimer";
  wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
  RegisterClassEx(&wincl);
  //Tworzenie okna timera
  hTimerFrm = CreateWindowEx(0, L"TFacebookGroupsManagerTimer", L"",	0, 0, 0, 0, 0, NULL, NULL, HInstance, NULL);
  //Wszystkie moduly zostaly zaladowane
  if(PluginLink.CallService(AQQ_SYSTEM_MODULESLOADED,0,0))
  {
	//Wygenerowanie ID dla enumeracji kontaktow
	ReplyListID = GetTickCount();
	//Wywolanie enumeracji kontaktow
	PluginLink.CallService(AQQ_CONTACTS_REQUESTLIST,(WPARAM)ReplyListID,0);
  }

  return 0;
}
//---------------------------------------------------------------------------

//Wyladowanie wtyczki
extern "C" INT_PTR __declspec(dllexport) __stdcall Unload()
{
  //Zatrzymanie wszystkich timerow
  for(int Count=0;Count<1000;Count++)
  {
	if(IdTable[Count].TimerId)
	 KillTimer(hTimerFrm,IdTable[Count].TimerId);
  }
  //Usuwanie okna timera
  DestroyWindow(hTimerFrm);
  //Wyrejestowanie klasy okna timera
  UnregisterClass(L"TFacebookGroupsManagerTimer",HInstance);
  //Usuniecie przycisku z popupmenu muItem
  Destroy_muItem();
  //Wyladowanie wszystkich hookow
  PluginLink.UnhookEvent(OnContactsUpdate);
  PluginLink.UnhookEvent(OnListReady);
  PluginLink.UnhookEvent(OnReplyList);
  PluginLink.UnhookEvent(OnSetLastState);
  PluginLink.UnhookEvent(OnStateChange);
  PluginLink.UnhookEvent(OnSystemPopUp);

  return 0;
}
//---------------------------------------------------------------------------

//Informacje o wtyczce
extern "C" PPluginInfo __declspec(dllexport) __stdcall AQQPluginInfo(DWORD AQQVersion)
{
  PluginInfo.cbSize = sizeof(TPluginInfo);
  PluginInfo.ShortName = L"FacebookGroupsManager";
  PluginInfo.Version = PLUGIN_MAKE_VERSION(1,0,0,0);
  PluginInfo.Description = L"Wtyczka umo¿liwia przenoszenie kontaktów z sieci Facebook do wybranych przez nas grup.";
  PluginInfo.Author = L"Krzysztof Grochocki (Beherit)";
  PluginInfo.AuthorMail = L"kontakt@beherit.pl";
  PluginInfo.Copyright = L"Krzysztof Grochocki (Beherit)";
  PluginInfo.Homepage = L"http://beherit.pl";
  PluginInfo.Flag = 0;
  PluginInfo.ReplaceDefaultModule = 0;

  return &PluginInfo;
}
//---------------------------------------------------------------------------
