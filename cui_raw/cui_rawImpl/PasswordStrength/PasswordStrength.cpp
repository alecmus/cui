/*
** PasswordStrength.cpp - password strength control implementation
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

LRESULT CALLBACK cui_rawImpl::PasswordStrengthProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	cui_rawImpl::PasswordStrengthControl* pThis = reinterpret_cast<cui_rawImpl::PasswordStrengthControl*>(ptr);

	switch (msg)
	{
	case WM_COMMAND:
	{
		if (LOWORD(wParam) == IDOK)
		{
			HDC hdc = GetDC(hWnd);

			RECT rc_;
			GetClientRect(hWnd, &rc_);

			Gdiplus::Graphics graphics(hdc);

			Gdiplus::RectF rc = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rc_);

			// define colors
			Gdiplus::Color red(Gdiplus::Color::Red);
			Gdiplus::Color orange(Gdiplus::Color::Orange);
			Gdiplus::Color green(Gdiplus::Color::Green);

			Gdiplus::REAL iMargin = 2.0f;
			Gdiplus::REAL iSegmentWidth = (rc.Width / 10.0f) - iMargin;

			Gdiplus::RectF rc_segment = rc;

			// draw segments
			Gdiplus::Color color;
			color.SetFromCOLORREF(pThis->clrUnfilled);
			Gdiplus::SolidBrush brush_unfilled(color);

			Gdiplus::Color clr = red;

			Gdiplus::REAL icx = 0.0f;

			for (size_t i = 0; i < 10; i++)
			{
				if (i > 4)
					clr = orange;

				if (i > 7)
					clr = green;

				rc_segment = rc;
				rc_segment.X = (iMargin / 2.0f) + static_cast<Gdiplus::REAL>(i) * (iSegmentWidth + iMargin);
				rc_segment.Width = iSegmentWidth;

				// fill entire segment with grey
				graphics.FillRectangle(&brush_unfilled, rc_segment);

				if (pThis->iPercentage > 10.0 * static_cast<double>(i))
				{
					icx = static_cast<Gdiplus::REAL>(pThis->iPercentage) -
						10.0f * static_cast<Gdiplus::REAL>(i);

					if (icx < 10.0f)
						rc_segment.Width = icx * iSegmentWidth / 10.0f;

					// fill segment with indicator color
					Gdiplus::SolidBrush brush(clr);
					graphics.FillRectangle(&brush, rc_segment);
				}
			}

			ReleaseDC(hWnd, hdc);
		}
	}
	break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		SendMessage(hWnd, WM_COMMAND, (WPARAM)IDOK, NULL);

		EndPaint(hWnd, &ps);
	}
	break;
	}

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)pThis->PrevProc, hWnd, msg, wParam, lParam);
} // PasswordStrengthProc
