/*
** Combobox.cpp - combobox implementation
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

#include "Combobox.h"
#include "../cui_rawImpl.h"

LRESULT CALLBACK cui_rawImpl::ComboBoxControlProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	cui_rawImpl::ComboBoxControl* pThis = reinterpret_cast<cui_rawImpl::ComboBoxControl*>(ptr);

	PAINTSTRUCT		ps;
	RECT			rect;
	POINT			pt;

	switch (msg)
	{
	case WM_PAINT:
	{
		GetClientRect(hWnd, &rect);
		int cx = rect.right - rect.left;
		int cy = rect.bottom - rect.top;

		HDC dc;

		if (wParam == 0)
			dc = BeginPaint(hWnd, &ps);
		else
			dc = (HDC)wParam;

		HDC hdc = CreateCompatibleDC(dc);

		if (!pThis->hbm_buffer)
			pThis->hbm_buffer = CreateCompatibleBitmap(dc, cx, cy);

		HBITMAP hbmOld = SelectBitmap(hdc, pThis->hbm_buffer);

		/////////////////////////////////////////////////////////////////////////
		//	Mask off the borders and draw ComboBox normally

		int iEdgeX = GetSystemMetrics(SM_CXEDGE);
		int iEdgeY = GetSystemMetrics(SM_CYEDGE);

		InflateRect(&rect, -iEdgeX, -iEdgeY);

		rect.right -= GetSystemMetrics(SM_CXVSCROLL);

		IntersectClipRect(hdc, rect.left, rect.top, rect.right, rect.bottom);

		// Draw the ComboBox
		CallWindowProc((WNDPROC)pThis->PrevProc, hWnd, msg, (WPARAM)hdc, lParam);

		/////////////////////////////////////////////////////////////////////////
		//	Mask off the inner area and draw a custom boarder
		SelectClipRgn(hdc, NULL);

		int iScrollBtnWidth = GetSystemMetrics(SM_CXVSCROLL);

		rect.right += iScrollBtnWidth;

		ExcludeClipRect(hdc, rect.left, rect.top, rect.right, rect.bottom);

		// draw borders
		RECT rectBoarder;
		GetClientRect(hWnd, &rectBoarder);

		CBrush hbrBoarder(pThis->pThis->d->m_clrTheme);
		FillRect(hdc, &rectBoarder, hbrBoarder.get());

		int iBorderX = GetSystemMetrics(SM_CXBORDER);
		int iBorderY = GetSystemMetrics(SM_CYBORDER);
		InflateRect(&rectBoarder, -iBorderX, -iBorderY);

		CBrush hbrBackground(pThis->pThis->d->m_clrBackground);
		FillRect(hdc, &rectBoarder, hbrBackground.get());

		/////////////////////////////////////////////////////////////////////////
		// draw the button
		SelectClipRgn(hdc, NULL);

		rect.left = rect.right - iScrollBtnWidth;

		if (false)
		{
			if (pThis->fButtonDown)
				DrawFrameControl(hdc, &rect, DFC_SCROLL, DFCS_SCROLLCOMBOBOX | DFCS_FLAT | DFCS_PUSHED);
			else
				DrawFrameControl(hdc, &rect, DFC_SCROLL, DFCS_SCROLLCOMBOBOX | DFCS_FLAT);
		}
		else
		{
			RECT rectBk = rect;

			FillRect(hdc, &rectBk, hbrBackground.get());

			Gdiplus::Graphics graphics(hdc);
			graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
			graphics.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeHighQuality);

			COLORREF clrPen = pThis->pThis->d->m_clrTheme;

			if (pThis->fButtonDown)
				clrPen = pThis->pThis->d->m_clrThemeHot;

			if (!IsWindowEnabled(hWnd))
				clrPen = pThis->pThis->d->m_clrDisabled;

			Gdiplus::Color color;
			color.SetFromCOLORREF(clrPen);

			Gdiplus::Pen pen(color, Gdiplus::REAL(2));

			Gdiplus::REAL x_center = Gdiplus::REAL(rect.left) + Gdiplus::REAL(rect.right - rect.left) / 2.0f;
			Gdiplus::REAL y_center = Gdiplus::REAL(rect.top) + Gdiplus::REAL(rect.bottom - rect.top) / 2.0f;

			// calculate bounding rectangle
			Gdiplus::REAL w = Gdiplus::REAL(rect.right - rect.left) / 2.0f;
			Gdiplus::REAL h = w / 2;

			Gdiplus::REAL x = x_center - w / 2;
			Gdiplus::REAL y = y_center - h / 2;

			{
				Gdiplus::PointF pt1(x, y);
				Gdiplus::PointF pt2(x + w / 2, y + h);

				graphics.DrawLine(&pen, pt1, pt2);
			}

			{
				Gdiplus::PointF pt1(x + w, y);
				Gdiplus::PointF pt2(x + w / 2, y + h);

				graphics.DrawLine(&pen, pt1, pt2);
			}
		}

		BitBlt(dc, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY);

		if (wParam == 0)
			EndPaint(hWnd, &ps);

		SelectBitmap(hdc, hbmOld);
		DeleteDC(hdc);
	}
	return 0;

	// check if mouse is within drop-arrow area, toggle
	// a flag to say if the mouse is up/down. Then invalidate
	// the window so it redraws to show the changes.
	case WM_LBUTTONDBLCLK:
	case WM_LBUTTONDOWN:

		pt.x = (short)LOWORD(lParam);
		pt.y = (short)HIWORD(lParam);

		GetClientRect(hWnd, &rect);

		InflateRect(&rect, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CYEDGE));
		rect.left = rect.right - GetSystemMetrics(SM_CXVSCROLL);

		if (PtInRect(&rect, pt))
		{
			// we *should* call SetCapture, but the ComboBox does it for us
			// SetCapture
			pThis->fMouseDown = TRUE;
			pThis->fButtonDown = TRUE;
			InvalidateRect(hWnd, NULL, FALSE);
		}

		break;

		// mouse has moved. Check to see if it is in/out of the drop-arrow
	case WM_MOUSEMOVE:
	{
		pt.x = (short)LOWORD(lParam);
		pt.y = (short)HIWORD(lParam);

		if (pThis->fMouseDown && (wParam & MK_LBUTTON))
		{
			GetClientRect(hWnd, &rect);

			InflateRect(&rect, -GetSystemMetrics(SM_CXEDGE), -GetSystemMetrics(SM_CYEDGE));
			rect.left = rect.right - GetSystemMetrics(SM_CXVSCROLL);

			if (pThis->fButtonDown != PtInRect(&rect, pt))
			{
				pThis->fButtonDown = PtInRect(&rect, pt);
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
	}
	break;

	case WM_LBUTTONUP:
	{
		if (pThis->fMouseDown)
		{
			// No need to call ReleaseCapture, the ComboBox does it for us
			// ReleaseCapture

			pThis->fMouseDown = FALSE;
			pThis->fButtonDown = FALSE;
			InvalidateRect(hWnd, NULL, FALSE);
		}
	}
	break;

	case WM_DESTROY:
	{
		// delete buffer, we're done
		if (pThis->hbm_buffer)
		{
			DeleteBitmap(pThis->hbm_buffer);
			pThis->hbm_buffer = NULL;
		}
	}
	break;

	case WM_SIZE:
	{
		// delete buffer, we need it recreated
		if (pThis->hbm_buffer)
		{
			DeleteBitmap(pThis->hbm_buffer);
			pThis->hbm_buffer = NULL;
		}

		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;
	}

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)pThis->PrevProc, hWnd, msg, wParam, lParam);
} // ComboBoxControlProc

  /*
  ** Finds the first string in a combo box list that begins with the specified string.
  **
  ** hWndCtl is the handle of the combobox
  **
  ** indexStart is the zero-based index of the item before the first item
  ** to be searched. When the search reaches the bottom of the
  ** list, it continues searching from the top of the list back
  ** to the item specified by the indexStart parameter. If
  ** indexStart is –1, the entire list is searched from the
  ** beginning.
  **
  ** lpszFind is the string to find
  **
  ** returns the index of the matching item, or CB_ERR if the search was unsuccessful
  */
int cui_rawImpl::Combo_FindString(HWND hWndCtl, INT indexStart, LPTSTR lpszFind)
{
	/*
	** Note: ComboBox_FindString does not work with ComboBoxEx and so it is necessary
	** to furnish our own version of the function.  We will use this version for
	** both types of comboBoxes
	*/

	TCHAR lpszBuffer[DEFAULT_TXT_LIM];
	TCHAR tmp[DEFAULT_TXT_LIM];
	int ln = (int)_tcslen(lpszFind) + 1;

	if (ln == 1 || indexStart > ComboBox_GetCount(hWndCtl))
		return CB_ERR;

	for (int i = indexStart == -1 ? 0 : indexStart; i < ComboBox_GetCount(hWndCtl); i++)
	{
		ComboBox_GetLBText(hWndCtl, i, lpszBuffer);
		lstrcpyn(tmp, lpszBuffer, ln);

		if (!_tcsicmp(lpszFind, tmp))
			return i;
	}

	return CB_ERR;
} // Combo_FindString

LRESULT CALLBACK cui_rawImpl::ComboBoxEditControlProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	cui_rawImpl::ComboBoxEditControl* pThis = reinterpret_cast<cui_rawImpl::ComboBoxEditControl*>(ptr);

	switch (msg)
	{
	case WM_GETDLGCODE:
		return VK_TAB == wParam ? FALSE : DLGC_WANTALLKEYS;

	case WM_CHAR:
	{
		// character pressed, do autocomplete
		HWND hCombo = GetParent(GetParent(hWnd));

		if (!Combo_IsExtended(hCombo))
			hCombo = GetParent(hWnd);

		TCHAR ch = (TCHAR)wParam;

		/*
		** Note: If user presses VK_RETURN or VK_TAB then
		** the ComboBox Notification = CBN_SELENDCANCEL and
		** a call to ComboBox_GetCurSel() will return the cancelled index.
		** If the user presses any other key that causes a selection
		** and closure of the dropdown then
		** the ComboBox Notification = CBN_SELCHANGE
		*/
		static TCHAR buf[DEFAULT_TXT_LIM];
		static TCHAR toFind[DEFAULT_TXT_LIM];
		static BOOL fMatched = TRUE;
		int index = 0;

		// Handle keyboard input
		if (VK_RETURN == ch)
		{
			// the enter key has been pressed
			ComboBox_ShowDropdown(hCombo, FALSE);
			Combo_SetEditSel(hCombo, 0, -1); //selects the entire item

											 // check if the item exists in the list
											 // Get the substring from 0 to start of selection
			ComboBox_GetText(hCombo, buf, NELEMS(buf));

			// Find the first item in the combo box that matches ToFind
			index = ComboBox_FindStringExact(hCombo, -1, buf);

			if (CB_ERR != index)
			{
				// select the item
				ComboBox_SetCurSel(hCombo, index);

				/*
				** ComboBox_SetCurSel doesn't send CBN_SELCHANGE to the parent
				** notify the parent that the combobox selection has changed
				*/
				DWORD loWord = GetWindowLong(hCombo, GWL_ID);
				DWORD hiWord = CBN_SELCHANGE;

				WPARAM wParam_toSend = MAKELONG(loWord, hiWord);
				SendMessage(GetParent(hCombo), WM_COMMAND, wParam_toSend, NULL);
			}
			else
			{
				// text in combo's edit control has no match in the combobox's list
			}
		}
		else if (VK_BACK == ch)
		{
			if (fMatched)
			{
				/*
				** Backspace normally erases highlighted match
				** we only want to move the highlighter back a step
				*/
				index = ComboBox_GetCurSel(hCombo);
				int bs = LOWORD(ComboBox_GetEditSel(hCombo)) - 1;

				// keep current selection selected
				ComboBox_SetCurSel(hCombo, index);

				/*
				** Move cursor back one space to the insertion point for new text
				** and hilight the remainder of the selected match or near match
				*/
				Combo_SetEditSel(hCombo, bs, -1);
			}
			else
			{
				/*
				** check length of toFind first
				*/
				int len = (int)_tcslen(toFind);

				if (len > 0)
				{
					toFind[len - 1] = 0;
					ComboBox_SetText(hCombo, toFind);
					Combo_SetEditSel(hCombo, -1, -1);
					FORWARD_WM_KEYDOWN(hCombo, VK_END, 0, 0, SNDMSG);
				}
			}
		}
		else if (!_istcntrl(ch))
		{
			BOOL status = GetWindowLongPtr(hCombo, GWL_STYLE) & CBS_DROPDOWN;
			if (status)
				ComboBox_ShowDropdown(hCombo, TRUE);

			if (Combo_IsExtended(hCombo)) // keep focus on edit box
				SetFocus(ComboBoxEx_GetEditControl(hCombo));

			// Get the substring from 0 to start of selection
			ComboBox_GetText(hCombo, buf, NELEMS(buf));
			buf[LOWORD(ComboBox_GetEditSel(hCombo))] = 0;

			_stprintf_s(toFind, NELEMS(toFind),
#ifdef _UNICODE
				_T("%ls%lc"),
#else
				_T("%s%c"),
#endif
				buf, ch);

			// Find the first item in the combo box that matches ToFind
			index = ComboBox_FindStringExact(hCombo, -1, toFind);

			if (CB_ERR == index)
			{
				/*
				** no match
				** Find the first item in the combo box that starts with ToFind
				*/
				index = Combo_FindString(hCombo, -1, toFind);
			}

			if (CB_ERR != index)
			{
				// Else for match
				fMatched = TRUE;
				ComboBox_SetCurSel(hCombo, index);
				Combo_SetEditSel(hCombo, _tcslen(toFind), -1);
			}
			else
			{
				// Display text that is not in the selected list
				fMatched = FALSE;
				ComboBox_SetText(hCombo, toFind);
				Combo_SetEditSel(hCombo, _tcslen(toFind), -1);
				FORWARD_WM_KEYDOWN(hCombo, VK_END, 0, 0, SNDMSG);
			}
		}

		return FALSE;
	} // case WM_CHAR
	break;
	}

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)pThis->PrevProc, hWnd, msg, wParam, lParam);
} // ComboBoxEditControlProc
