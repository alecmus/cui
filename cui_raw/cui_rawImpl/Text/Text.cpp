/*
** Text.cpp - text control implementation
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

static bool design = false;

LRESULT CALLBACK cui_rawImpl::TextControlProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	cui_rawImpl::TextControl* pThis = reinterpret_cast<cui_rawImpl::TextControl*>(ptr);

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

		COLORREF clr_text = pThis->clrText;
		INT style = Gdiplus::FontStyle::FontStyleRegular;

		if (pThis->bHot && !pThis->bStatic)
		{
			clr_text = pThis->clrTextHot;
			style = Gdiplus::FontStyle::FontStyleUnderline;
		}

		// create graphics object
		Gdiplus::Graphics graphics(hdc);

		// fill background
		Gdiplus::Color color;
		color.SetFromCOLORREF(pThis->clrBackground);
		graphics.Clear(color);

		Gdiplus::RectF layoutRect = convert_rect(rc);

		Gdiplus::Font* p_font = new Gdiplus::Font(&Gdiplus::FontFamily(pThis->sFontName.c_str()),
			static_cast<Gdiplus::REAL>(pThis->iFontSize), style);

		if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
		{
			delete p_font;
			p_font = nullptr;
			p_font = new Gdiplus::Font(pThis->sFontName.c_str(),
				static_cast<Gdiplus::REAL>(pThis->iFontSize), style, Gdiplus::UnitPoint, &pThis->d->m_font_collection);
		}

		color.SetFromCOLORREF(clr_text);
		Gdiplus::SolidBrush text_brush(color);

		Gdiplus::StringFormat format;
		format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
		format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);

		if (!pThis->bMultiLine)
			format.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);

		// measure text rectangle
		Gdiplus::RectF text_rect;
		graphics.MeasureString(pThis->sText.c_str(), -1, p_font, layoutRect, &text_rect);

		if (!pThis->bMultiLine)
		{
			if (text_rect.Width < layoutRect.Width)
				text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
		}

		// align the text rectangle to the layout rectangle
		align_text(text_rect, layoutRect, pThis->align);

		if (design)
		{
			color.SetFromCOLORREF(RGB(240, 240, 240));
			Gdiplus::SolidBrush brush(color);
			graphics.FillRectangle(&brush, layoutRect);

			color.SetFromCOLORREF(RGB(230, 230, 230));
			brush.SetColor(color);
			graphics.FillRectangle(&brush, text_rect);
		}

		// draw text
		graphics.DrawString(pThis->sText.c_str(),
			-1, p_font, text_rect, &format, &text_brush);

		delete p_font;
		p_font = nullptr;

		// capture the text rectangle
		pThis->rcText = convert_rect(text_rect);

		BitBlt(dc, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY);

		EndPaint(hWnd, &ps);

		SelectBitmap(hdc, hbmOld);
		DeleteDC(hdc);
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
} // TextControlProc
