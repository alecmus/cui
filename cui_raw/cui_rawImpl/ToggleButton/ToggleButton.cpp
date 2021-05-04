/*
** ToggleButton.cpp - toggle control implementation
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
#include "../DrawRoundRect/DrawRoundRect.h"

static bool design = false;

LRESULT CALLBACK cui_rawImpl::ToggleBtnProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	ToggleButtonControl* pControl = reinterpret_cast<ToggleButtonControl*>(ptr);

	switch (msg)
	{
	case WM_PAINT:
	{
		RECT itemRect;
		GetClientRect(hWnd, &itemRect);
		const int cx = itemRect.right - itemRect.left;
		const int cy = itemRect.bottom - itemRect.top;

		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hWnd, &ps);

		// use double buffering to avoid flicker
		HDC hdc = CreateCompatibleDC(dc);

		if (!pControl->hbm_buffer)
			pControl->hbm_buffer = CreateCompatibleBitmap(dc, cx, cy);

		HBITMAP hbmOld = SelectBitmap(hdc, pControl->hbm_buffer);

		Gdiplus::Graphics graphics(hdc);
		Gdiplus::Color color;
		color.SetFromCOLORREF(pControl->d->m_clrBackground);
		graphics.Clear(color);

		int iDarkenPerc = 20;
		int iLightenPec = 50;

		COLORREF clrButtonBackground = pControl->clrOff;

		if (pControl->d->m_bLButtonDown)
		{
			if (pControl->iPercH > 50)
				clrButtonBackground = pControl->clrOn;
			else
				clrButtonBackground = pControl->clrOff;
		}
		else
		{
			bool bState = pControl->bOn;

			if (bState)
				clrButtonBackground = pControl->clrOn;
		}

		if (!IsWindowEnabled(hWnd))
			clrButtonBackground = pControl->d->m_clrDisabled;

		SetBkMode(hdc, TRANSPARENT);

		// draw toggle button (on)
		RECT rect = itemRect;
		rect.right = rect.left + 2 * (rect.bottom - rect.top);	// width = 2 * height

		pControl->rcToggler = rect;

		RECT rc = rect;
		InflateRect(&rc, 0, -(rc.bottom - rc.top) / 5);

		{
			// draw rounded rectangle
			int iRadius = rc.right - rc.left < rc.bottom - rc.top ? rc.right - rc.left : rc.bottom - rc.top;
			DrawRoundRect(graphics, rc.left, rc.top, rc.right, rc.bottom, iRadius / 2, clrButtonBackground, clrButtonBackground, 1, true);
		}

		rc = rect;

		if (pControl->bOn)
			rc.left = rc.right - (rc.right - rc.left) / 2;
		else
			rc.right = rc.right - (rc.right - rc.left) / 2;

		InflateRect(&rc, -2, -2);

		if (pControl->d->m_bLButtonDown)
		{
			RECT rcTarget = rect;
			InflateRect(&rcTarget, -2, -2);

			cui_raw::posRect(rc, rcTarget, pControl->iPercH, 0);
		}

		{
			// draw circle
			int iRadius = rc.right - rc.left < rc.bottom - rc.top ? rc.right - rc.left : rc.bottom - rc.top;
			int iCenterX = rc.left + (rc.right - rc.left) / 2;
			int iCenterY = rc.top + (rc.bottom - rc.top) / 2;

			rc.left = iCenterX - iRadius / 2;
			rc.right = iCenterX + iRadius / 2;
			rc.top = iCenterY - iRadius / 2;
			rc.bottom = iCenterY + iRadius / 2;

			InflateRect(&rc, 2, 2);

			int iFactor = 30;

			if (!IsWindowEnabled(hWnd))
				iFactor = 20;

			DrawRoundRect(graphics, rc.left, rc.top, rc.right, rc.bottom, iRadius / 2, clrDarken(pControl->d->m_clrBackground, iFactor), clrDarken(pControl->d->m_clrBackground, iFactor), 1, true);

			InflateRect(&rc, -1, -1);
			DrawRoundRect(graphics, rc.left, rc.top, rc.right, rc.bottom, iRadius / 2, pControl->d->m_clrBackground, clrDarken(pControl->d->m_clrBackground, 10), 1, true);
		}

		// Write the button caption (if any)
		COLORREF clrText = pControl->clrText;

		if (!IsWindowEnabled(hWnd))
			clrText = clrDarken(pControl->d->m_clrDisabled, 30);

		color.SetFromCOLORREF(clrText);
		Gdiplus::SolidBrush text_brush(color);

		Gdiplus::Font* p_font = new Gdiplus::Font(&Gdiplus::FontFamily(pControl->sFontName.c_str()),
			static_cast<Gdiplus::REAL>(pControl->iFontSize));

		if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
		{
			delete p_font;
			p_font = nullptr;
			p_font = new Gdiplus::Font(pControl->sFontName.c_str(),
				static_cast<Gdiplus::REAL>(pControl->iFontSize), Gdiplus::FontStyle::FontStyleRegular,
				Gdiplus::UnitPoint, &pControl->d->m_font_collection);
		}

		Gdiplus::StringFormat format;
		format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
		format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);
		format.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);

		std::basic_string<TCHAR> sCaption;

		if (pControl->bOn)
			sCaption = pControl->sCaptionOn;
		else
			sCaption = pControl->sCaptionOff;

		RECT captionRect = itemRect;
		captionRect.left = rect.right + 5;

		Gdiplus::RectF layoutRect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(captionRect);

		Gdiplus::RectF text_rect;
		graphics.MeasureString(sCaption.c_str(), -1, p_font, layoutRect, &text_rect);

		if (true)
		{
			if (text_rect.Width < layoutRect.Width)
				text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
		}

		// align the text rectangle to the layout rectangle
		align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::middleleft);

		if (design)
		{
			Gdiplus::Color color;
			color.SetFromCOLORREF(RGB(240, 240, 240));
			Gdiplus::SolidBrush brush(color);
			graphics.FillRectangle(&brush, layoutRect);

			color.SetFromCOLORREF(RGB(230, 230, 230));
			brush.SetColor(color);
			graphics.FillRectangle(&brush, text_rect);
		}

		// draw text
		graphics.DrawString(sCaption.c_str(),
			-1, p_font, text_rect, &format, &text_brush);

		delete p_font;
		p_font = nullptr;

		BitBlt(dc, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY);
		SelectBitmap(hdc, hbmOld);

		EndPaint(hWnd, &ps);

		// cleanup
		DeleteDC(hdc);
	}
	break;

	case WM_ENABLE:
	{
		bool bEnabled = wParam == TRUE;

		if (bEnabled)
		{
			pControl->state = enumBtn::normal;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else
		{
			InvalidateRect(hWnd, NULL, FALSE);
		}
	}
	break;

	case WM_SHOWWINDOW:
	{
		if (wParam == FALSE)
			pControl->toolTip.bAllowToolTip = false;

		if (wParam == TRUE)
			pControl->toolTip.bAllowToolTip = true;
	}
	break;

	case WM_DESTROY:
	{
		// delete buffer, we're done
		if (pControl->hbm_buffer)
		{
			DeleteBitmap(pControl->hbm_buffer);
			pControl->hbm_buffer = NULL;
		}
	}
	break;

	case WM_SIZE:
	{
		// delete buffer, we need it recreated
		if (pControl->hbm_buffer)
		{
			DeleteBitmap(pControl->hbm_buffer);
			pControl->hbm_buffer = NULL;
		}

		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;
	}

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)pControl->PrevProc, hWnd, msg, wParam, lParam);
} // ToggleBtnProc
