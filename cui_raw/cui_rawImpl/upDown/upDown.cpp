/*
** upDown.cpp - up-down control implementation
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

#include "../cui_rawImpl.h"

LRESULT CALLBACK cui_rawImpl::UpDownControlProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	cui_rawImpl::EditControl* pThis = reinterpret_cast<cui_rawImpl::EditControl*>(ptr);

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

		// draw borders
		RECT rectBoarder;
		GetClientRect(hWnd, &rectBoarder);

		CBrush hbrBackground(pThis->d->m_clrBackground);
		FillRect(hdc, &rectBoarder, hbrBackground.get());

		ExcludeClipRect(hdc, rectBoarder.left - 1, rectBoarder.top + 1, rectBoarder.left + 2, rectBoarder.bottom - 1);

		CBrush hbrBoarder(pThis->d->m_clrTheme);
		FillRect(hdc, &rectBoarder, hbrBoarder.get());

		int iBoarderWidth = 1;

		InflateRect(&rectBoarder, -iBoarderWidth, -iBoarderWidth);

		FillRect(hdc, &rectBoarder, hbrBackground.get());

		InflateRect(&rect, -1, -1);

		COLORREF clrArrow = pThis->d->m_clrTheme;

		if (!IsWindowEnabled(pThis->hWnd))
			clrArrow = pThis->d->m_clrDisabled;

		/////////////////////////////////////////////////////////////////////////
		// draw the upper button
		{
			Gdiplus::RectF rectBk(Gdiplus::REAL(rect.left), Gdiplus::REAL(rect.top), Gdiplus::REAL(rect.right - rect.left), Gdiplus::REAL(rect.bottom - rect.top));
			rectBk.Height /= 2.0f;

			Gdiplus::Graphics graphics(hdc);
			graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
			graphics.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeHighQuality);

			COLORREF m_clrArrow = clrArrow;

			if (IsWindowEnabled(pThis->hWnd) && pThis->fButtonDown && pThis->bUp)
				m_clrArrow = pThis->d->m_clrThemeHot;

			Gdiplus::Color color;
			color.SetFromCOLORREF(m_clrArrow);

			Gdiplus::Pen pen(color, Gdiplus::REAL(2));

			Gdiplus::REAL x_center = rectBk.X + rectBk.Width / 2.0f;
			Gdiplus::REAL y_center = rectBk.Y + rectBk.Height / 2.0f;

			// calculate bounding rectangle
			Gdiplus::REAL w = rectBk.Width / 2.0f;
			Gdiplus::REAL h = w / 2.0f;

			{
				Gdiplus::PointF pt1(x_center, rectBk.Y + ((rectBk.Height - h) / 2.0f));
				Gdiplus::PointF pt2(x_center - (w / 2.0f), rectBk.Y + ((rectBk.Height - h) / 2.0f) + h);

				graphics.DrawLine(&pen, pt1, pt2);
			}

			{
				Gdiplus::PointF pt1(x_center, rectBk.Y + ((rectBk.Height - h) / 2.0f));
				Gdiplus::PointF pt2(x_center + (w / 2.0f), rectBk.Y + ((rectBk.Height - h) / 2.0f) + h);

				graphics.DrawLine(&pen, pt1, pt2);
			}
		}

		/////////////////////////////////////////////////////////////////////////
		// draw the lower button
		{
			Gdiplus::RectF rectBk(Gdiplus::REAL(rect.left), Gdiplus::REAL(rect.top), Gdiplus::REAL(rect.right - rect.left), Gdiplus::REAL(rect.bottom - rect.top));
			rectBk.Height /= 2.0f;
			rectBk.Y += rectBk.Height;

			Gdiplus::Graphics graphics(hdc);
			graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
			graphics.SetInterpolationMode(Gdiplus::InterpolationMode::InterpolationModeHighQuality);

			COLORREF m_clrArrow = clrArrow;

			if (IsWindowEnabled(pThis->hWnd) && pThis->fButtonDown && !pThis->bUp)
				m_clrArrow = pThis->d->m_clrThemeHot;

			Gdiplus::Color color;
			color.SetFromCOLORREF(m_clrArrow);

			Gdiplus::Pen pen(color, Gdiplus::REAL(2));

			Gdiplus::REAL x_center = rectBk.X + rectBk.Width / 2.0f;
			Gdiplus::REAL y_center = rectBk.Y + rectBk.Height / 2.0f;

			// calculate bounding rectangle
			Gdiplus::REAL w = rectBk.Width / 2.0f;
			Gdiplus::REAL h = w / 2.0f;

			{
				Gdiplus::PointF pt1(x_center, rectBk.Y + rectBk.Height - ((rectBk.Height - h) / 2.0f));
				pt1.Y -= 1;
				Gdiplus::PointF pt2(x_center - (w / 2.0f), rectBk.Y + rectBk.Height - ((rectBk.Height - h) / 2.0f) - h);
				pt2.Y -= 1;

				graphics.DrawLine(&pen, pt1, pt2);
			}

			{
				Gdiplus::PointF pt1(x_center, rectBk.Y + rectBk.Height - ((rectBk.Height - h) / 2.0f));
				pt1.Y -= 1;
				Gdiplus::PointF pt2(x_center + (w / 2.0f), rectBk.Y + rectBk.Height - ((rectBk.Height - h) / 2.0f) - h);
				pt2.Y -= 1;

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

			if (pt.y > rect.top + (rect.bottom - rect.top) / 2)
				pThis->bUp = false;
			else
				pThis->bUp = true;

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
	return CallWindowProc((WNDPROC)pThis->UpDownPrevProc, hWnd, msg, wParam, lParam);
} // UpDownControlProc
