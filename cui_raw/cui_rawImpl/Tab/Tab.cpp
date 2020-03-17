/*
** Tab.cpp - tab control implementation
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
#include "../DrawRoundRect/DrawRoundRect.h"

LRESULT CALLBACK cui_rawImpl::tabControlProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	tabControl* pControl = reinterpret_cast<tabControl*>(ptr);

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

		// use double buffering to prevent flicker
		HDC hdc = CreateCompatibleDC(dc);

		if (!pControl->hbm_buffer)
			pControl->hbm_buffer = CreateCompatibleBitmap(dc, cx, cy);

		HBITMAP hbmOld = SelectBitmap(hdc, pControl->hbm_buffer);

		Gdiplus::Graphics graphics(hdc);
		Gdiplus::Color color;
		color.SetFromCOLORREF(pControl->d->m_clrBackground);
		graphics.Clear(color);

		// move left five pixels to the right
		rc.left += int(0.5 + 5 * pControl->d->m_DPIScale);

		// move right five pixels to the left
		rc.right -= int(0.5 + 5 * pControl->d->m_DPIScale);

		// reduce rect size by one pixel all round
		InflateRect(&rc, -1, -1);

		bool bIsDisabled = false;

		// reduce size of rect by ten pixels above and beneath for aesthetic purposes
		int iTabHeight = int(0.5 + 40 * pControl->d->m_DPIScale);

		RECT rcTab = rc;
		rcTab.top += int(0.5 + 10 * pControl->d->m_DPIScale);
		rcTab.bottom = rcTab.top + iTabHeight;

		RECT rcTabSelected;
		rcTabSelected.left = 0;
		rcTabSelected.top = 0;
		rcTabSelected.right = 0;
		rcTabSelected.bottom = 0;

		// fill selected tab
		RECT rcTab_og = rcTab;
		for (size_t i = 0; i < pControl->vTabs.size(); i++)
		{
			int iOffset = (int)i * iTabHeight;

			rcTab.top = rcTab_og.top + iOffset;
			rcTab.bottom = rcTab_og.bottom + iOffset;

			// capture tab rect
			pControl->vTabs[i].rcTab = rcTab;

			// deflate tab coordinates to ensure tabs are seperated ... so cursor changing mechanism won't struggle
			InflateRect(&pControl->vTabs[i].rcTab, -int(0.5 + 5 * pControl->d->m_DPIScale), -int(0.5 + 5 * pControl->d->m_DPIScale));

			COLORREF clrText = pControl->d->m_clrTheme;

			Gdiplus::Graphics graphics(hdc);

			if (pControl->vTabs[i].bSelected)
			{
				rcTabSelected = rcTab;

				// set text color to that of the window's background color
				clrText = pControl->d->m_clrBackground;
				COLORREF clr = pControl->d->m_clrTheme;
				DrawRoundRect(graphics, rcTab.left, rcTab.top, rcTab.right, rcTab.bottom, int(0.5 + 3 * pControl->d->m_DPIScale), clr, clr, 1, true);
			}
			else
			{
				if (pControl->vTabs[i].bHot)
				{
					RECT rc = rcTab;
					InflateRect(&rc, -2, -2);

					Gdiplus::RectF rect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rc);

					color.SetFromCOLORREF(pControl->d->m_clrThemeLight);
					Gdiplus::SolidBrush brush(color);
					Gdiplus::Pen pen(&brush, 1.0f);
					pen.SetDashStyle(Gdiplus::DashStyle::DashStyleDash);

					graphics.DrawRectangle(&pen, rect);
				}
			}

			// write tab title
			if (!pControl->vTabs[i].sCaption.empty())
			{
				RECT itemRect = rcTab;
				itemRect.left += 10;
				itemRect.right -= 10;

				Gdiplus::RectF layoutRect = convert_rect(itemRect);

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

				color.SetFromCOLORREF(clrText);
				Gdiplus::SolidBrush text_brush(color);

				Gdiplus::StringFormat format;
				format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
				format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);
				format.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);

				// measure text rectangle
				Gdiplus::RectF text_rect;
				graphics.MeasureString(pControl->vTabs[i].sCaption.c_str(), -1, p_font, layoutRect, &text_rect);

				if (true)
				{
					if (text_rect.Width < layoutRect.Width)
						text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
				}

				// align the text rectangle to the layout rectangle
				align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::middleleft);

				// draw text
				graphics.DrawString(pControl->vTabs[i].sCaption.c_str(),
					-1, p_font, text_rect, &format, &text_brush);

				delete p_font;
				p_font = nullptr;
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

		InvalidateRect(hWnd, NULL, TRUE);
	}
	break;
	}

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)pControl->PrevProc, hWnd, msg, wParam, lParam);
} // tabProc
