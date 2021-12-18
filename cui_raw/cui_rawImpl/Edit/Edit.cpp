//
// Edit.cpp - edit implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#include "../cui_rawImpl.h"

LRESULT CALLBACK cui_rawImpl::EditControlProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	cui_rawImpl::EditControl* pThis = reinterpret_cast<cui_rawImpl::EditControl*>(ptr);

	switch (msg)
	{
	case WM_PAINT:
	{
		HDC				hdc;
		PAINTSTRUCT		ps;
		RECT			rect;

		if (wParam == 0)
			hdc = BeginPaint(hWnd, &ps);
		else
			hdc = (HDC)wParam;

		/////////////////////////////////////////////////////////////////////////
		//	Mask off the borders and draw edit box normally
		GetClientRect(hWnd, &rect);

		int iEdgeX = GetSystemMetrics(SM_CXBORDER);
		int iEdgeY = GetSystemMetrics(SM_CYBORDER);

		InflateRect(&rect, -iEdgeX, -iEdgeY);

		IntersectClipRect(hdc, rect.left, rect.top, rect.right, rect.bottom);

		// Draw the edit control
		CallWindowProc((WNDPROC)pThis->PrevProc, hWnd, msg, (WPARAM)hdc, lParam);

		/////////////////////////////////////////////////////////////////////////
		//	Mask off the inner area and draw a custom boarder
		SelectClipRgn(hdc, NULL);

		ExcludeClipRect(hdc, rect.left, rect.top, rect.right, rect.bottom);

		if (pThis->bUpDown)
		{
			/*
			** for aesthetic purposes, this is important. The right edge of the edit
			** box tends to appear here and there on the left of the up-down control
			*/
			ExcludeClipRect(hdc, rect.right - 1, rect.top - 1, rect.right + 2, rect.bottom + 1);
		}

		// draw borders
		RECT rectBoarder;
		GetClientRect(hWnd, &rectBoarder);

		CBrush hbrBoarder(pThis->d->m_clrTheme);
		FillRect(hdc, &rectBoarder, hbrBoarder.get());

		InflateRect(&rectBoarder, -iEdgeX, -iEdgeY);

		CBrush hbrBackground(pThis->d->m_clrBackground);
		FillRect(hdc, &rectBoarder, hbrBackground.get());

		if (wParam == 0)
			EndPaint(hWnd, &ps);
	}
	return 0;

	case WM_CHAR:
	{
		/*
		** This switch statement structure copied from
		** https://msdn.microsoft.com/en-us/library/windows/desktop/ms646268(v=vs.85).aspx
		*/
		switch (wParam)
		{
		case 0x08:
			// Process a backspace. 
			break;

		case 0x0A:
			// Process a linefeed. 
			break;

		case 0x1B:
			// Process an escape. 
			break;

		case 0x09:
			// Process a tab. 
			break;

		case 0x0D:
			// Process a carriage return. 
			break;

		default:
		{
			// Process displayable characters. 
			if (pThis->bPassword)
				break;

			const TCHAR c = (TCHAR)wParam;
			if (!pThis->sForbiddenCharacterSet.empty())
			{
				for (auto &it : pThis->sForbiddenCharacterSet)
				{
					if (it == c)
					{
						// hide baloon tooltip
						Edit_HideBalloonTip(hWnd);

						// show ballon tooltip
						EDITBALLOONTIP bt;
						bt.cbStruct = sizeof(bt);
						bt.pszText = _T("The character is not allowed");
						//bt.pszTitle = sPageMap.c_str();
						bt.pszTitle = _T("Error");
						bt.ttiIcon = TTI_ERROR;

						Edit_ShowBalloonTip(hWnd, &bt);

						return 0;
					}
				}
			}

			if (!pThis->sAllowedCharacterSet.empty())
			{
				bool bAllowed = false;

				for (auto &it : pThis->sAllowedCharacterSet)
				{
					if (it == c)
					{
						bAllowed = true;
						break;
					}
				}

				if (!bAllowed)
				{
					// hide baloon tooltip
					Edit_HideBalloonTip(hWnd);

					// show ballon tooltip
					EDITBALLOONTIP bt;
					bt.cbStruct = sizeof(bt);
					bt.pszText = _T("The character is not allowed");
					//bt.pszTitle = sPageMap.c_str();
					bt.pszTitle = _T("Error");
					bt.ttiIcon = TTI_ERROR;

					Edit_ShowBalloonTip(hWnd, &bt);

					return 0;
				}
			}
		}
		break;
		}
	}
	break;

	case WM_SHOWWINDOW:
	{
		if (pThis->bUpDown)
		{
			// this is an up-down edit control
			if (wParam == FALSE)
			{
				// hide the up-down control
				ShowWindow(pThis->hWndUpDown, SW_HIDE);
			}
			else
				if (wParam == TRUE)
				{
					// show the up-down control
					ShowWindow(pThis->hWndUpDown, SW_SHOW);
				}
		}
	}
	break;

	case WM_KEYUP:
	{
		switch (wParam)
		{
		case VK_RETURN:
		{
			if (pThis->iControlToInvoke > 0)
			{
				SendMessage(GetParent(pThis->hWnd), WM_COMMAND, (WPARAM)pThis->iControlToInvoke, NULL);
				break;
			}

			// enter has been pressed, find the first visible default button of this page and send a message to it
			for (auto &it : pThis->d->m_Pages[pThis->d->m_sCurrentPage].m_ButtonControls)
			{
				if (it.second.bDefaultButton && IsWindowVisible(it.second.hWnd) && IsWindowEnabled(it.second.hWnd))
				{
					SendMessage(GetParent(it.second.hWnd), WM_COMMAND, (WPARAM)it.second.iUniqueID, NULL);
					break;
				}
			}
		}
		break;

		default:
			break;
		}
	}

	}

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)pThis->PrevProc, hWnd, msg, wParam, lParam);
} // EditControlProc
