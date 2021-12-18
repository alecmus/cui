//
// Selector.cpp - selector control implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#include "../cui_rawImpl.h"
#include "../../clrAdjust/clrAdjust.h"
#include "../DrawRoundRect/DrawRoundRect.h"

static bool design = false;

LRESULT CALLBACK cui_rawImpl::SelectorProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	SelectorControl* pControl = reinterpret_cast<SelectorControl*>(ptr);

	switch (msg)
	{
	case WM_PAINT:
	{
		// selector height
		int iSelectorHeight = int(0.5 + 8 * pControl->d->m_DPIScale);

		// selector width
		int iSelectorWidth = int(0.5 + 30 * pControl->d->m_DPIScale);

		// marker width
		int iMarkerWidth = int(0.5 + 10 * pControl->d->m_DPIScale);

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

		{
			CBrush brBackground(pControl->d->m_clrBackground);
			FillRect(hdc, &itemRect, brBackground.get());
		}

		// figure out number of options
		int iOptions = (int)pControl->vItems.size();
		int iDiv = 80 / (iOptions - 1);	// TO-DO: potential divide by zero!!! Fix this!

										// define rects
		RECT rcSelectorItem;
		rcSelectorItem.left = 0;
		rcSelectorItem.right = rcSelectorItem.left + iSelectorWidth;
		rcSelectorItem.top = 0;
		rcSelectorItem.bottom = rcSelectorItem.top + iSelectorHeight;

		pControl->rcSelector = itemRect;
		pControl->rcSelector.left = rcSelectorItem.left;
		pControl->rcSelector.right = rcSelectorItem.right;

		cui_raw::posRect(rcSelectorItem, itemRect, 0, 10);

		COLORREF clrBar = pControl->clrBar;

		if (!IsWindowEnabled(hWnd))
			clrBar = pControl->d->m_clrDisabled;

		COLORREF clrLine = clrLighten(clrBar, 40);

		if (!IsWindowEnabled(hWnd))
			clrLine = pControl->d->m_clrDisabled;

		HPEN hpen = CreatePen(PS_SOLID, 1, clrLine);
		HPEN hpen_old = SelectPen(hdc, hpen);

		Gdiplus::Graphics graphics(hdc);
		
		Gdiplus::FontFamily ffm(pControl->sFontName.c_str());
		Gdiplus::Font* p_font = new Gdiplus::Font(&ffm,
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

		COLORREF clrText = pControl->clrText;

		if (!IsWindowEnabled(hWnd))
			clrText = clrDarken(pControl->d->m_clrDisabled, 30);	// TO-DO: remove magic number

		Gdiplus::Color color;
		color.SetFromCOLORREF(clrText);

		std::vector<int> vDiffs;

		int iSmallest = -1932;

		if (pControl->bPressed)
		{
			// Control is pressed. Drag selector with the vertical mouse coordinates.
			for (size_t i = 0; i < pControl->vItems.size(); i++)
			{
				int iPerc = 10 + (int)i * iDiv;

				pControl->vItems[i].iDiff = iPerc - pControl->iPercV;

				if (pControl->vItems[i].iDiff < 0)
					pControl->vItems[i].iDiff = 0 - pControl->vItems[i].iDiff;

				if (iSmallest == -1932)
					iSmallest = pControl->vItems[i].iDiff;

				bool bSmaller = true;

				for (size_t x = 0; x < vDiffs.size(); x++)
				{
					if (vDiffs[x] < pControl->vItems[i].iDiff)
						bSmaller = false;
				}

				if (bSmaller)
				{
					cui_raw::posRect(rcSelectorItem, itemRect, 0, iPerc);
					pControl->vItems[i].iYPos = rcSelectorItem.top + int(0.5 + iSelectorHeight / 2);

					pControl->iSelectedItem = pControl->vItems[i].iUniqueID;
					iSmallest = pControl->vItems[i].iDiff;
				}

				vDiffs.push_back(pControl->vItems[i].iDiff);
			}
		}

		{
			// draw vertical line
			int x = int((iSelectorWidth / 2.0f) + 0.5);

			MoveToEx(hdc, x, itemRect.top, NULL);
			LineTo(hdc, x, itemRect.bottom);
		}

		// draw markers and text
		for (size_t i = 0; i < pControl->vItems.size(); i++)
		{
			int iPerc = 10 + (int)i * iDiv;

			cui_raw::posRect(rcSelectorItem, itemRect, 0, iPerc);
			pControl->vItems[i].iYPos = rcSelectorItem.top + int(0.5 + iSelectorHeight / 2);

			if (i == 0)
			{
				pControl->yUpper = int(0.5 + iSelectorHeight / 2);
				pControl->rcSelector.top = rcSelectorItem.top + pControl->yUpper;
			}

			if (i == pControl->vItems.size() - 1)
			{
				pControl->yLower = int(0.5 + iSelectorHeight / 2);
				pControl->rcSelector.bottom = rcSelectorItem.bottom - pControl->yUpper;
			}

			// draw marker
			RECT m_rc = rcSelectorItem;

			InflateRect(&m_rc, -int(0.5 + ((rcSelectorItem.right - rcSelectorItem.left) / 2)), 0);
			InflateRect(&m_rc, int(0.5 + (iMarkerWidth / 2)), 0);

			MoveToEx(hdc, m_rc.left, pControl->vItems[i].iYPos, NULL);
			LineTo(hdc, m_rc.right, pControl->vItems[i].iYPos);

			Gdiplus::Graphics graphics(hdc);

			// capture position of selected item
			if (pControl->vItems[i].iUniqueID == pControl->iSelectedItem)
			{
				// draw selector
				DrawRoundRect(graphics, rcSelectorItem.left, rcSelectorItem.top, rcSelectorItem.right, rcSelectorItem.bottom, int(0.5 + 1 * pControl->d->m_DPIScale), clrBar, clrBar, 1, true);
			}

			// measure text rectangle
			Gdiplus::RectF text_rect;
			graphics.MeasureString((pControl->vItems[i].sDescription + L" ").c_str(), -1, p_font, text_rect, &text_rect);

			Gdiplus::REAL v = (text_rect.Height / 2.0f) + 1.0f;

			Gdiplus::RectF layoutRect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(itemRect);
			layoutRect.Y = static_cast<Gdiplus::REAL>(pControl->vItems[i].iYPos) - v;
			layoutRect.Height = 2.0f * v;
			layoutRect.X = static_cast<Gdiplus::REAL>(rcSelectorItem.right) + 10.0f;

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
			Gdiplus::SolidBrush brush(color);
			graphics.DrawString(pControl->vItems[i].sDescription.c_str(),
				-1, p_font, text_rect, &format, &brush);
		}

		delete p_font;
		p_font = nullptr;

		SelectPen(hdc, hpen_old);
		DeletePen(hpen);

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

	case WM_ERASEBKGND:
	{
		HDC hdc = (HDC)wParam;

		RECT rect;
		GetClientRect(hWnd, &rect);

		CBrush hbr(pControl->d->m_clrBackground);
		FillRect(hdc, &rect, hbr.get());

		return TRUE;
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
} // SelectorProc
