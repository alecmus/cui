/*
** CPopupMenu.cpp - pop-up menu implementation
**
** cui framework
** Copyright (c) 2016 Alec T. Musasa (alecmus at live dot com)
**
*******************************************************************************
** This file is part of the liblec library which is released under the Creative
** Commons Attribution Non-Commercial 2.0 license (CC-BY-NC 2.0). See the file
** LICENSE.txt or go to https://github.com/alecmus/liblec/blob/master/LICENSE.md
** for full license details.
*/

#include "CPopupMenu.h"
#include "../CImage/CImage.h"

CPopupMenu::CPopupMenu()
{
	m_hPopup = NULL;
	m_iPos = 0;
	m_default = 0;
}

CPopupMenu::~CPopupMenu()
{
	if (m_hPopup)
		DestroyMenu(m_hPopup);
}

/*
** create popup menu
*/
void CPopupMenu::Create(
	HWND hParent		// parent of popup window
)
{
	m_hParent = hParent;
	m_hPopup = CreatePopupMenu();
} // Create

  /*
  ** add item to popup menu
  */
void CPopupMenu::AddItem(
	WORD ID,				// item ID
	const std::basic_string<TCHAR> &sLabel,		// label of item
	int IDP_ICON,			// PNG resource of item's icon
	HMODULE hModule,
	BOOL bDefault			// whether item is the default menu item
)
{
	if (sLabel.empty())
	{
		// make this a seperator and ignore everything else
		InsertMenu(m_hPopup, m_iPos, MF_BYPOSITION | MF_SEPARATOR, NULL, _T(""));
	}
	else
	{
		CImageConv imgcv;

		// TO-DO: remove magic number???? Question ... find the answer ...
		TCHAR buffer[256];
		lstrcpyn(buffer, sLabel.c_str(), _countof(buffer));

		MENUITEMINFO info = { 0 };
		info.cbSize = sizeof(info);
		info.fMask = MIIM_STRING | MIIM_FTYPE | MIIM_ID | MIIM_BITMAP;
		info.fType = MFT_STRING;
		info.wID = (UINT)ID;
		info.dwTypeData = buffer;

		std::basic_string<TCHAR> sErr;

		HBITMAP hbmp = NULL;
		hbmp = imgcv.PNGtoARGB(hModule, IDP_ICON, sErr);	// TO-DO: handle error

		info.hbmpItem = hbmp;

		InsertMenuItem(m_hPopup, m_iPos, TRUE, &info);

		/*
		** CAN DO WITHOUT STUFF
		*/
		SetFocus(m_hParent);
		SendMessage(m_hParent, WM_INITMENUPOPUP, (WPARAM)m_hPopup, 0);

		if (bDefault == TRUE)
			m_default = ID;
	}

	m_iPos++;
} // AddItem

  /*
  ** show popup menu and return ID of clicked item
  */
WORD CPopupMenu::Show()
{
	if (m_default != 0)
		SetMenuDefaultItem(m_hPopup, m_default, FALSE);

	/*
	** get cursor position so we can create popup menu there
	*/
	POINT curpos;
	GetCursorPos(&curpos);

	/*
	** display popup menu and wait for selection
	*/
	WORD cmd = TrackPopupMenu(m_hPopup, TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY, curpos.x, curpos.y, 0, m_hParent, NULL);

	// return the ID of the clicked item
	return cmd;
} // Show
