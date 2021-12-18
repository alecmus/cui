//
// Button.cpp - button implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#include "../cui_rawImpl.h"
#include "../DrawRoundRect/DrawRoundRect.h"
#include "../../clrAdjust/clrAdjust.h"

LRESULT CALLBACK cui_rawImpl::BtnProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	ButtonControl* p_control = reinterpret_cast<ButtonControl*>(ptr);

	switch (msg)
	{
	case WM_ENABLE:
	{
		bool enabled = wParam == TRUE;

		if (enabled)
		{
			p_control->state = enumBtn::normal;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else
			InvalidateRect(hWnd, NULL, FALSE);
	}
	break;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hWnd, &ps);

		RECT itemRect;
		GetClientRect(hWnd, &itemRect);
		int cx = itemRect.right - itemRect.left;
		int cy = itemRect.bottom - itemRect.top;

		// use double buffering to avoid flicker
		HDC hdc = CreateCompatibleDC(dc);

		if (!p_control->hbm_buffer)
			p_control->hbm_buffer = CreateCompatibleBitmap(dc, cx, cy);

		HBITMAP hbmOld = SelectBitmap(hdc, p_control->hbm_buffer);

		RECT outerRect = itemRect;

		itemRect.left += 1;
		itemRect.right -= 1;
		itemRect.top += 1;
		itemRect.bottom -= 1;

		SetBkMode(hdc, TRANSPARENT);

		// button state
		bool bHover = p_control->state == enumBtn::hover;
		bool bIsPressed = p_control->bPressed;
		BOOL bIsDisabled = !IsWindowEnabled(hWnd);
		BOOL bDrawFocusRect = FALSE;

		Gdiplus::Graphics graphics(hdc);

		Gdiplus::Color color;
		color.SetFromCOLORREF(p_control->d->m_clrBackground);
		graphics.Clear(color);

		////////////////////////////////////////////////////////////////////
		// draw button Border
		{
			// draw background
			COLORREF clrBorder = p_control->d->m_clrTheme;

			if (bHover)
				clrBorder = p_control->d->m_clrThemeLight;

			if (bIsPressed && bHover)
				clrBorder = p_control->d->m_clrTheme;

			if (bIsDisabled)
				clrBorder = p_control->d->m_clrDisabled;

			DrawRoundRect(graphics, outerRect.left, outerRect.top, outerRect.right, outerRect.bottom, int(0.5 + 3 * p_control->d->m_DPIScale), clrBorder, clrBorder, 1, true);
		} // if (bIsFocused)

		COLORREF clrBackground;
		COLORREF clrText;

		COLORREF clrHot = p_control->d->m_clrTheme;
		COLORREF clrCold = p_control->d->m_clrBackground;
		COLORREF clrColdHover = clrCold;

		if (bIsPressed && bHover)
		{
			clrBackground = p_control->d->m_clrThemeDarker;
			clrText = clrCold;
		}
		else
		{
			if (!bHover)
			{
				clrBackground = clrHot;
				clrText = clrCold;
			}
			else
			{
				clrBackground = p_control->d->m_clrThemeLight;
				clrText = clrCold;
			}
		}

		if (bIsDisabled)
		{
			clrText = clrDarken(p_control->d->m_clrDisabled, 30);	// TO-DO: remove magic number
			clrBackground = clrCold;
		}

		// draw background
		DrawRoundRect(graphics, itemRect.left, itemRect.top, itemRect.right, itemRect.bottom, int(0.5 + 3 * p_control->d->m_DPIScale), clrBackground, clrBackground, 1, true);

		// Write the button caption (if any)
		if (!p_control->sCaption.empty())
		{
			Gdiplus::RectF layoutRect = convert_rect(itemRect);

			Gdiplus::FontFamily ffm(p_control->sFontName.c_str());
			Gdiplus::Font* p_font = new Gdiplus::Font(&ffm,
				static_cast<Gdiplus::REAL>(p_control->iFontSize));

			if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
			{
				delete p_font;
				p_font = new Gdiplus::Font(p_control->sFontName.c_str(),
					static_cast<Gdiplus::REAL>(p_control->iFontSize), Gdiplus::FontStyle::FontStyleRegular,
					Gdiplus::UnitPoint, &p_control->d->m_font_collection);
			}

			color.SetFromCOLORREF(clrText);
			Gdiplus::SolidBrush text_brush(color);

			Gdiplus::StringFormat format;
			format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
			format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);
			format.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);

			// measure text rectangle
			Gdiplus::RectF text_rect;
			graphics.MeasureString(p_control->sCaption.c_str(), -1, p_font, layoutRect, &text_rect);

			if (true)
			{
				if (text_rect.Width < layoutRect.Width)
					text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
			}

			// align the text rectangle to the layout rectangle
			align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::middle);

			// draw text
			graphics.DrawString(p_control->sCaption.c_str(),
				-1, p_font, text_rect, &format, &text_brush);

			delete p_font;
			p_font = nullptr;
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
			p_control->toolTip.bAllowToolTip = false;

		if (wParam == TRUE)
			p_control->toolTip.bAllowToolTip = true;
	}
	break;

	case WM_DESTROY:
	{
		// delete buffer, we're done
		if (p_control->hbm_buffer)
		{
			DeleteBitmap(p_control->hbm_buffer);
			p_control->hbm_buffer = NULL;
		}
	}
	break;

	case WM_SIZE:
	{
		// delete buffer, we need it recreated
		if (p_control->hbm_buffer)
		{
			DeleteBitmap(p_control->hbm_buffer);
			p_control->hbm_buffer = NULL;
		}

		InvalidateRect(hWnd, NULL, TRUE);
	}
	break;

	case WM_KEYUP:
	{
		switch (wParam)
		{
		case VK_RETURN:
		{
			// enter has been pressed
			SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)p_control->iUniqueID, NULL);
		}
		break;

		default:
			break;
		}
	}
	}

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)p_control->PrevProc, hWnd, msg, wParam, lParam);
} // BtnProc
