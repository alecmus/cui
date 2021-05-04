/*
** ControlButtons.cpp - control buttons implementation
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

LRESULT CALLBACK cui_rawImpl::ControlBtnProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	ControlBtn* pThis = reinterpret_cast<ControlBtn*>(ptr);

	switch (msg)
	{
	case WM_ENABLE:
	{
		bool bEnabled = wParam == TRUE;

		if (bEnabled)
		{
			pThis->state = enumBtn::normal;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else
		{
			InvalidateRect(hWnd, NULL, FALSE);
		}
	}
	break;

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

		int iSize = rc.right - rc.left;

		Gdiplus::Graphics graphics(hdc);
		Gdiplus::Color color;
		color.SetFromCOLORREF(pThis->d->m_clrBackground);
		graphics.Clear(color);

		rc.left += iSize / 4;
		rc.right -= iSize / 4;
		rc.top += iSize / 4;
		rc.bottom -= iSize / 4;

		COLORREF m_clr = pThis->clrBtn;

		if (pThis->bHot)
			m_clr = pThis->clrBtnHot;
		else
			m_clr = pThis->clrBtn;

		if (!IsWindowEnabled(hWnd))
			m_clr = pThis->d->m_clrDisabled;

		if (pThis->iUniqueID == pThis->d->m_iIDClose)
		{
			Gdiplus::PointF point_1(static_cast<Gdiplus::REAL>(rc.left),
				static_cast<Gdiplus::REAL>(rc.top));
			Gdiplus::PointF point_2(static_cast<Gdiplus::REAL>(rc.right),
				static_cast<Gdiplus::REAL>(rc.bottom));
			Gdiplus::PointF point_3(static_cast<Gdiplus::REAL>(rc.left),
				static_cast<Gdiplus::REAL>(rc.bottom));
			Gdiplus::PointF point_4(static_cast<Gdiplus::REAL>(rc.right),
				static_cast<Gdiplus::REAL>(rc.top));

			color.SetFromCOLORREF(m_clr);
			Gdiplus::Pen pen(color, 2.0f);

			graphics.DrawLine(&pen, point_1, point_2);
			graphics.DrawLine(&pen, point_3, point_4);
		}

		if (pThis->iUniqueID == pThis->d->m_iIDMax)
		{
			if (!IsMaximized(pThis->d->m_hWnd))
			{

				RECT rect_max_box;
				rect_max_box.left = rc.left;
				rect_max_box.top = rc.top + 1;
				rect_max_box.right = rc.right;
				rect_max_box.bottom = rc.bottom;

				color.SetFromCOLORREF(m_clr);
				Gdiplus::Pen pen(color, 1.0f);

				Gdiplus::RectF rect =
					liblec::cui::gui_raw::cui_rawImpl::convert_rect(rect_max_box);
				graphics.DrawRectangle(&pen, rect);

				rect_max_box.top = rect_max_box.top + 1;
				
				rect =
					liblec::cui::gui_raw::cui_rawImpl::convert_rect(rect_max_box);
				graphics.DrawRectangle(&pen, rect);
			}
			else
			{
				RECT rect_restore_box;
				rect_restore_box.top = rc.top;
				rect_restore_box.right = rc.right;
				rect_restore_box.left = rect_restore_box.right - (4 * (rc.right - rc.left)) / 5;
				rect_restore_box.bottom = rect_restore_box.top + (4 * (rc.bottom - rc.top)) / 5;

				color.SetFromCOLORREF(m_clr);
				Gdiplus::Pen pen(color, 1.0f);

				Gdiplus::RectF rect =
					liblec::cui::gui_raw::cui_rawImpl::convert_rect(rect_restore_box);
				graphics.DrawRectangle(&pen, rect);

				rect_restore_box.left = rc.left;
				rect_restore_box.bottom = rc.bottom;
				rect_restore_box.top = rect_restore_box.bottom - (4 * (rc.bottom - rc.top)) / 5;
				rect_restore_box.right = rect_restore_box.left + (4 * (rc.right - rc.left)) / 5;

				rect =
					liblec::cui::gui_raw::cui_rawImpl::convert_rect(rect_restore_box);

				color.SetFromCOLORREF(pThis->d->m_clrBackground);
				Gdiplus::SolidBrush brush(color);
				graphics.FillRectangle(&brush, rect);

				graphics.DrawRectangle(&pen, rect);

				rect_restore_box.top = rect_restore_box.top + 1;

				rect =
					liblec::cui::gui_raw::cui_rawImpl::convert_rect(rect_restore_box);
				graphics.DrawRectangle(&pen, rect);
			}
		}

		if (pThis->iUniqueID == pThis->d->m_iIDMin)
		{
			RECT rect_min_box;
			rect_min_box.left = rc.left;
			rect_min_box.top = rc.bottom - 1;
			rect_min_box.right = rc.right;
			rect_min_box.bottom = rc.bottom;

			color.SetFromCOLORREF(m_clr);
			Gdiplus::Pen pen(color, 1.0f);

			Gdiplus::RectF rect =
				liblec::cui::gui_raw::cui_rawImpl::convert_rect(rect_min_box);
			graphics.DrawRectangle(&pen, rect);
		}

		BitBlt(dc, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY);

		EndPaint(hWnd, &ps);

		SelectBitmap(hdc, hbmOld);
		DeleteDC(hdc);
	}
	break;

	case WM_SHOWWINDOW:
	{
		if (wParam == FALSE)
			pThis->toolTip.bAllowToolTip = false;

		if (wParam == TRUE)
			pThis->toolTip.bAllowToolTip = true;
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

		InvalidateRect(hWnd, NULL, TRUE);
	}
	break;
	}

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)pThis->PrevProc, hWnd, msg, wParam, lParam);
} // ControlBtnProc
