//
// Rect.cpp - rectangle implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#include "../cui_rawImpl.h"

LRESULT CALLBACK cui_rawImpl::RectProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	cui_rawImpl::RectControl* pControl = reinterpret_cast<cui_rawImpl::RectControl*>(ptr);

	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		RECT rcClient;
		GetClientRect(hWnd, &rcClient);

		CBrush hbr(pControl->clr);
		FillRect(hdc, &rcClient, hbr.get());

		EndPaint(hWnd, &ps);
	}
	break;

	case WM_ERASEBKGND:
	{
		HDC hdc = (HDC)wParam;

		RECT rect;
		GetClientRect(hWnd, &rect);

		COLORREF clrBackground = RGB(255, 255, 255);

		if (pControl->d)
			clrBackground = pControl->d->m_clrBackground;

		CBrush hbr(clrBackground);
		FillRect(hdc, &rect, hbr.get());

		return TRUE;
	}
	break;

	case WM_ENABLE:
	{
		bool bEnabled = wParam == TRUE;

		if (bEnabled)
		{
			// control has been enabled
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else
		{
			// control has been disabled
			InvalidateRect(hWnd, NULL, TRUE);
		}
	}
	break;
	}

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)pControl->PrevProc, hWnd, msg, wParam, lParam);
} // RectProc
