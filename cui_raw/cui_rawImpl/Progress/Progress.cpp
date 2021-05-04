/*
** Progress.cpp - progress control implementation
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

#include "../cui_rawImpl.h"
#include "../../clrAdjust/clrAdjust.h"

LRESULT CALLBACK cui_rawImpl::ProgressProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	cui_rawImpl::ProgressControl* pThis = reinterpret_cast<cui_rawImpl::ProgressControl*>(ptr);

	int iIDTIMER = pThis->iUniqueID;

	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hWnd, &ps);

		RECT rc;
		GetClientRect(hWnd, &rc);
		int cx = rc.right - rc.left;
		int cy = rc.bottom - rc.top;

		// use double buffering to avoid flicker
		HDC hdc = CreateCompatibleDC(dc);

		if (!pThis->hbm_buffer)
			pThis->hbm_buffer = CreateCompatibleBitmap(dc, cx, cy);

		HBITMAP hbmOld = SelectBitmap(hdc, pThis->hbm_buffer);

		int iBorderChangeFactor = 10;

		if (pThis->clrUnfilled == pThis->d->m_clrBackground)
			iBorderChangeFactor = 0;	// if user sets background color to equal that of window make border transparent

		Gdiplus::RectF rect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rc);

		Gdiplus::Graphics graphics(hdc);
		Gdiplus::Color color;
		color.SetFromCOLORREF(clrDarken(pThis->clrUnfilled, iBorderChangeFactor));
		Gdiplus::SolidBrush brush(color);
		graphics.FillRectangle(&brush, rect);

		rect.Inflate(-1.0f, -1.0f);

		color.SetFromCOLORREF(pThis->clrUnfilled);
		brush.SetColor(color);
		graphics.FillRectangle(&brush, rect);

		if (!pThis->bBusy)
		{
			// draw normal progress bar
			if (pThis->bTimerRunning)
			{
				// stop the timer
				pThis->bTimerRunning = false;
				KillTimer(hWnd, iIDTIMER);
			}

			Gdiplus::REAL iAdd = 0.0f;

			if (pThis->iPercentage >= 0)
				iAdd = static_cast<Gdiplus::REAL>(pThis->iPercentage) * rect.Width / 100.0f;

			if (pThis->bReverse)
				rect.X += (rect.Width - iAdd);

			rect.Width = iAdd;

			color.SetFromCOLORREF(pThis->clrBar);
			brush.SetColor(color);
			graphics.FillRectangle(&brush, rect);
		}
		else
		{
			// draw "busy" progress bar
			Gdiplus::RectF m_rc = rect;
			m_rc.X = 0;
			m_rc.Width = 0.1f * rect.Width;

			cui_rawImpl::pos_rect(m_rc, rect, static_cast<Gdiplus::REAL>(pThis->iBusyPerc), 0);

			color.SetFromCOLORREF(pThis->clrBar);
			brush.SetColor(color);
			graphics.FillRectangle(&brush, m_rc);

			if (!pThis->bTimerRunning)
			{
				// start the timer
				pThis->bTimerRunning = true;
				SetTimer(hWnd, iIDTIMER, 20, NULL);
			}
		}

		BitBlt(dc, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY);

		EndPaint(hWnd, &ps);

		SelectBitmap(hdc, hbmOld);
		DeleteDC(hdc);
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

	case WM_TIMER:
	{
		if (wParam == iIDTIMER)
		{
			if (pThis->iBusyPerc == 100)
				pThis->bBackward = true;
			else
				if (pThis->iBusyPerc == 0)
					pThis->bBackward = false;

			if (pThis->bBackward)
				pThis->iBusyPerc--;
			else
				pThis->iBusyPerc++;

			InvalidateRect(hWnd, NULL, FALSE);
			UpdateWindow(hWnd);
		}
	}
	break;

	}

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)pThis->PrevProc, hWnd, msg, wParam, lParam);
} // ProgressProc
