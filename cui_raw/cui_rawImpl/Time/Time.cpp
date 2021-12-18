//
// Time.cpp - time control implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#include "../cui_rawImpl.h"

LRESULT CALLBACK cui_rawImpl::TimeControlProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	cui_rawImpl::TimeControl* pThis = reinterpret_cast<cui_rawImpl::TimeControl*>(ptr);

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
		//	Mask off the borders and draw ComboBox normally
		GetClientRect(hWnd, &rect);

		int iEdgeX = GetSystemMetrics(SM_CXEDGE);
		int iEdgeY = GetSystemMetrics(SM_CYEDGE);

		InflateRect(&rect, -iEdgeX, -iEdgeY);

		IntersectClipRect(hdc, rect.left, rect.top, rect.right, rect.bottom);

		// Draw the ComboBox
		CallWindowProc((WNDPROC)pThis->PrevProc, hWnd, msg, (WPARAM)hdc, lParam);

		/////////////////////////////////////////////////////////////////////////
		//	Mask off the inner area and draw a custom boarder
		SelectClipRgn(hdc, NULL);

		ExcludeClipRect(hdc, rect.left, rect.top, rect.right, rect.bottom);

		// draw borders
		RECT rectBoarder;
		GetClientRect(hWnd, &rectBoarder);

		CBrush hbrBoarder(pThis->d->m_clrTheme);
		FillRect(hdc, &rectBoarder, hbrBoarder.get());

		int iBoarderWidth = 1;

		InflateRect(&rectBoarder, -iBoarderWidth, -iBoarderWidth);

		CBrush hbrBackground(pThis->d->m_clrBackground);
		FillRect(hdc, &rectBoarder, hbrBackground.get());

		if (wParam == 0)
			EndPaint(hWnd, &ps);
	}
	return 0;

	default:
		break;
	}

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)pThis->PrevProc, hWnd, msg, wParam, lParam);
} // TimeControlProc
