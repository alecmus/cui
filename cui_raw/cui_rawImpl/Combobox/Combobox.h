/*
** Combobox.h - combobox interface
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

#ifndef ComboBoxEx_GetEditControl
/*
** ComboBoxEx_GetEditControl(hWnd)
**
** Gets the handle to the edit control portion of a ComboBoxEx control.
** A ComboBoxEx control uses an edit box when it is set to the CBS_DROPDOWN style.
**
** hWnd Handle of the comboboxEx.
**
** returns The handle to the edit control within the ComboBoxEx control if
** it uses the CBS_DROPDOWN style. Otherwise, the message returns NULL
*/
#define ComboBoxEx_GetEditControl(hWnd) (HWND)SNDMSG((hWnd),CBEM_GETEDITCONTROL,0,0)
#endif

/*
** Combo_SetEditSel(hWndCtl, ichStart,ichEnd)
**
** Selects characters in the edit control of a combo box
**
** hWndCtl Handle of the combobox
**
** ichStart The starting position. If this is –1, the selection, if any, is removed
**
** ichEnd The ending position. If this is –1, all text from the starting position to
** the last character in the edit control is selected
*/
#define Combo_SetEditSel(hWndCtl, ichStart, ichEnd) Combo_IsExtended(hWndCtl) ? \
	(Edit_SetSel(ComboBoxEx_GetEditControl(hWndCtl),ichStart,ichEnd),0) : \
	(ComboBox_SetEditSel(hWndCtl,ichStart,ichEnd),0)

/*
** NELEMS(a)
**
** Computes number of elements of an array.
**
** param a - An array
*/
#define NELEMS(a)  (sizeof(a) / sizeof((a)[0]))

#ifndef DEFAULT_TXT_LIM
#define DEFAULT_TXT_LIM 0x7530 ///< Constant
#endif // !DEFAULT_TXT_LIM
