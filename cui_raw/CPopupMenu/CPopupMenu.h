/*
** CPopupMenu.h - pop-up menu interface
**
** cui framework
** Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
**
*******************************************************************************
** This file is part of the liblec library which is released under the Creative
** Commons Attribution Non-Commercial 2.0 license (CC-BY-NC 2.0). See the file
** LICENSE.txt or go to https://github.com/alecmus/liblec/blob/master/LICENSE.md
** for full license details.
*/

#pragma once

#include <Windows.h>
#include <tchar.h>
#include <string>

/*
** CPopupMenu - popup menu class
*/
class CPopupMenu
{
public:
	CPopupMenu();
	~CPopupMenu();

	/*
	** create popup menu
	*/
	void Create(
		HWND hParent		// parent of popup window
	);

	/*
	** add item to popup menu
	*/
	void AddItem(
		WORD ID,				// item ID
		const std::basic_string<TCHAR> &sLabel,		// label of item
		int IDP_ICON,			// PNG resource of item's icon
		HMODULE hModule,
		BOOL bDefault = FALSE	// whether item is the default menu item
	);

	/*
	** show popup menu and return ID of clicked item
	** returns 0 if no item is clicked
	*/
	WORD Show();

private:
	HMENU m_hPopup;
	HWND m_hParent;
	int m_iPos;
	WORD m_default;
}; // CPopupMenu
