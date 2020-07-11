/*
** TooltipControl.cpp - tooltip control implementation
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

#include <Windows.h>
#include <WindowsX.h>
#include "../../CBrush/CBrush.h"
#include "../cui_rawImpl.h"
#include "TooltipControl.h"

#define IDC_MSG	14253625

// get the size of the opaque area of bitmap
SIZE getSizeOfOpaque(HBITMAP hbmSrc)
{
	SIZE size;
	size = { 0 };

	HDC hdcSrc, hdcDst;
	HBITMAP hbmOld;
	BITMAP bm;

	if ((hdcSrc = CreateCompatibleDC(NULL)) != NULL)
	{
		if ((hdcDst = CreateCompatibleDC(NULL)) != NULL)
		{
			int iRow, iCol;
			GetObject(hbmSrc, sizeof(bm), &bm);
			hbmOld = (HBITMAP)SelectObject(hdcSrc, hbmSrc);

			BitBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);

			// loop through every pixel
			for (iRow = 0; iRow < bm.bmHeight; iRow++)
			{
				for (iCol = 0; iCol < bm.bmWidth; iCol++)
				{
					// get color of pixel
					// this is a layer mask so
					// black pixel means opaque
					// white pixel means transparent
					COLORREF clrTP = GetPixel(hdcSrc, iCol, iRow);

					DWORD r = GetRValue(clrTP);
					DWORD g = GetGValue(clrTP);
					DWORD b = GetBValue(clrTP);

					if (clrTP == RGB(0, 0, 0))
					{
						// black (opaque)
						if (iRow > size.cy)
							size.cy = iRow;

						if (iCol > size.cx)
							size.cx = iCol;
					}
				}
			}

			DeleteDC(hdcDst);
		}

		DeleteDC(hdcSrc);
	}

	return size;
} // getSizeOfOpaque

SIZE GetSize(HCURSOR ico)
{
	SIZE res = { 0 };
	if (ico)
	{
		ICONINFO info = { 0 };
		if (::GetIconInfo(ico, &info) != 0)
		{
			bool bBWCursor = (info.hbmColor == NULL);
			BITMAP bmpinfo = { 0 };
			if (::GetObject(info.hbmMask, sizeof(BITMAP), &bmpinfo) != 0)
			{
				// size of ENTIRE cursor (this includes a whole transparent area!)
				res.cx = bmpinfo.bmWidth;
				res.cy = abs(bmpinfo.bmHeight) / (bBWCursor ? 2 : 1);
			}

			// get the size of ONLY the opaque area of the cursor (clip out the transparent area)
			res = getSizeOfOpaque(info.hbmMask);

			::DeleteObject(info.hbmColor);
			::DeleteObject(info.hbmMask);
		}
	}

	return res;
} // GetSize

  // get coordinates of cursor
  // TO-DO: fix this ... the results are not consistent ... we're getting 32x32 but the cursor is clearly smaller than that ... fix fix fix fix ... others are doing it ... do it!!!
  // to-do: factor in dpi scale as in lecui::dimensions and you're done
void getCursorRect(RECT &rc)
{
	SIZE size = GetSize(GetCursor());

	POINT pt;
	GetCursorPos(&pt);

	rc.left = pt.x;
	rc.top = pt.y;
	rc.right = rc.left + size.cx;
	rc.bottom = rc.top + size.cy;
} // getCursorRect

bool isCursorWithin(cui_rawImpl::ToolTipControl &tooltip)
{
	POINT pt;
	GetCursorPos(&pt);

	RECT rectControl;
	GetWindowRect(tooltip.hWndControl, &rectControl);

	return PtInRect(&rectControl, pt) == TRUE;
} // isCursorWithin

void ensureVisible(HWND hWndControl, int &x, int &y, const int cx, const int cy, bool bDontShowWithinControl)
{
	RECT rcWindow;
	GetWindowRect(GetParent(hWndControl), &rcWindow);

	RECT rectControl;
	GetWindowRect(hWndControl, &rectControl);

	RECT rcCursor;
	getCursorRect(rcCursor);

	if (bDontShowWithinControl)
	{
		if (x < rcWindow.left + 5)
			x = rcWindow.left + 5;

		if ((y + cy) > rcWindow.bottom)
			y = rectControl.top - cy - 5;
		else
		{
			if (y < rcCursor.bottom)
				y = rcCursor.bottom;

			if ((y + cy) > rcWindow.bottom)
				y = rectControl.top - cy - 5;
		}

		if ((x + cx) > rcWindow.right - 5)
			x = rcWindow.right - cx - 5;
	}
	else
	{
		x = rcCursor.left;
		y = rcCursor.bottom + 2;

		if (x < rcWindow.left + 5)
			x = rcWindow.left + 5;

		if ((y + cy) > rcWindow.bottom)
			y = rcCursor.top - cy - 2;

		if ((x + cx) > rcWindow.right - 5)
			x = rcWindow.right - cx - 5;
	}
} // ensureVisible

void positionTooltip(cui_rawImpl::ToolTipControl &tooltip)
{
	RECT rectControl;
	GetWindowRect(tooltip.hWndControl, &rectControl);

	int x = 0, y = 0, cx = 0, cy = 0;

	{
		const int iMargin = 5;

		// calculate width and height
		HDC hdc = GetDC(tooltip.hWndTooltip);

		Gdiplus::Graphics graphics(hdc);
		
		Gdiplus::Font* p_font = new Gdiplus::Font(&Gdiplus::FontFamily(tooltip.sFontName.c_str()),
			static_cast<Gdiplus::REAL>(tooltip.iFontSize));

		if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
		{
			delete p_font;
			p_font = nullptr;
			p_font = new Gdiplus::Font(tooltip.sFontName.c_str(),
				static_cast<Gdiplus::REAL>(tooltip.iFontSize), Gdiplus::FontStyle::FontStyleRegular,
				Gdiplus::UnitPoint, &tooltip.d->m_font_collection);
		}

		Gdiplus::RectF text_rect;
		graphics.MeasureString((tooltip.sText + L" ").c_str(), -1, p_font, text_rect, &text_rect);

		delete p_font;
		p_font = nullptr;

		text_rect.Inflate(1.0f, 1.0f);	// critical because of the InflateRect in WM_PAINT

		auto round_up = [](const Gdiplus::REAL &real)
		{
			LONG long_ = static_cast<LONG>(real);
			Gdiplus::REAL real_ = static_cast<Gdiplus::REAL>(long_);

			if (real_ < real)
				long_++;

			return long_;
		}; // round_up

		SIZE size;
		size.cx = round_up(text_rect.Width);
		size.cy = round_up(text_rect.Height);

		ReleaseDC(tooltip.hWndTooltip, hdc);

		///////////////////////////
		cy = size.cy + 2 * iMargin;
		cx = size.cx + 2 * iMargin;
	}

	x = rectControl.left + ((rectControl.right - rectControl.left) - cx) / 2;
	y = rectControl.bottom + 5;

	// ensure tooltip is within working area
	ensureVisible(tooltip.hWndControl, x, y, cx, cy, tooltip.bDontShowWithinControl);

	/*
	** Does not activate the window. If this flag is not set, the window is activated
	** and moved to the top of either the topmost or non-topmost group (depending on
	** the setting of the hWndInsertAfter parameter).
	** https://msdn.microsoft.com/en-us/library/windows/desktop/ms633545(v=vs.85).aspx
	*/
	SetWindowPos(tooltip.hWndTooltip, 0, x, y, cx, cy, SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER);
}

bool CreateShadow(
	cui_rawImpl::ToolTipControl &tooltip
)
{
	if (tooltip.m_pShadow)
	{
		// initialize drop shadow parameters
		CShadow::shadow_properties properties;
		properties.color = tooltip.m_clrBorder;
		properties.darkness = 50;
		properties.position = { 0, 0 };
		properties.sharpness = 20;
		properties.size = 10;

		tooltip.m_pShadow->CreateShadow(tooltip.hWndTooltip, properties);

		return true;
	}
	else
		return false;
} // CreateShadow

static bool design = false;

LRESULT CALLBACK ToolProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	cui_rawImpl::ToolTipControl *pControl;
	if (msg == WM_CREATE)
	{
		CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pControl = reinterpret_cast<cui_rawImpl::ToolTipControl*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pControl);
	}
	else
	{
		LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
		pControl = reinterpret_cast<cui_rawImpl::ToolTipControl*>(ptr);
	}

	static int ID_TIMER = 133;

	switch (msg)
	{
	case WM_CREATE:
	{
		// capture tooltip window handle
		pControl->hWndTooltip = hWnd;

		// create shadow
		pControl->m_pShadow = new CShadow();

		CreateShadow(*pControl);

		// set timer
		SetTimer(hWnd, ID_TIMER, 1000, NULL);
	}
	return TRUE;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);

		RECT itemRect_;
		GetClientRect(hWnd, &itemRect_);

		RECT outerRect_ = itemRect_;
		InflateRect(&itemRect_, -1, -1);

		Gdiplus::RectF outerRect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(outerRect_);
		Gdiplus::RectF itemRect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(itemRect_);

		Gdiplus::Graphics graphics(hdc);
		Gdiplus::Color color;

		// draw border
		color.SetFromCOLORREF(pControl->m_clrBorder);
		Gdiplus::SolidBrush brush(color);
		graphics.FillRectangle(&brush, outerRect);

		color.SetFromCOLORREF(pControl->m_clrBackground);
		brush.SetColor(color);
		graphics.FillRectangle(&brush, itemRect);

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

		// draw text
		Gdiplus::RectF layoutRect = itemRect;

		Gdiplus::RectF text_rect;
		graphics.MeasureString(pControl->sText.c_str(), -1, p_font, layoutRect, &text_rect);

		if (true)
		{
			if (text_rect.Width < layoutRect.Width)
				text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
		}

		// align the text rectangle to the layout rectangle
		liblec::cui::gui_raw::cui_rawImpl::align_text(text_rect,
			layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::middle);

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
		color.SetFromCOLORREF(pControl->m_clrText);
		brush.SetColor(color);
		graphics.DrawString(pControl->sText.c_str(),
			-1, p_font, text_rect, &format, &brush);

		delete p_font;
		p_font = nullptr;

		EndPaint(hWnd, &ps);
	}
	break;

	case WM_ERASEBKGND:
	{
		HDC hdc = (HDC)wParam;

		RECT rect;
		GetClientRect(hWnd, &rect);

		CBrush hbr(pControl->m_clrBackground);
		FillRect(hdc, &rect, hbr.get());

		return TRUE;
	}
	break;

	case WM_DESTROY:
		KillTimer(hWnd, ID_TIMER);
		break;

	case WM_COMMAND:
	{
		if (wParam == IDC_MSG)
		{
			KillTimer(hWnd, ID_TIMER);
			SetTimer(hWnd, ID_TIMER, 1000, NULL);
			pControl->bAllowToolTip = true;
		}
	}
	break;

	case WM_SHOWWINDOW:
	{
		if (wParam == FALSE)
			KillTimer(hWnd, ID_TIMER);

		if (wParam == TRUE)
		{
			KillTimer(hWnd, ID_TIMER);
			SetTimer(hWnd, ID_TIMER, 5000, NULL);
		}
	}
	break;

	case WM_TIMER:
	{
		if (IsWindowVisible(hWnd))
			ShowWindow(hWnd, SW_HIDE);
		else
		{
			if (IsWindowEnabled(pControl->hWndControl))
			{
				if (pControl->bAllowToolTip && isCursorWithin(*pControl))
				{
					positionTooltip(*pControl);

					/*
					** Displays the window in its current size and position. This value is similar to SW_SHOW, except that the window is not activated.
					** https://msdn.microsoft.com/en-us/library/windows/desktop/ms633548(v=vs.85).aspx
					*/
					ShowWindow(hWnd, SW_SHOWNA);
				}
			}
			else
				KillTimer(hWnd, ID_TIMER);
		}
	}
	break;

	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
} // ToolProc

void ShowToolTip(cui_rawImpl::ToolTipControl &tooltip)
{
	if (tooltip.sText.empty())
		return;

	tooltip.bAllowToolTip = true;

	// add implementation for tooltips
	RECT rectControl;
	GetWindowRect(tooltip.hWndControl, &rectControl);

	if (!tooltip.hWndTooltip)
	{
		// register window class
		WNDCLASSEX wcex;
		wcex = { 0 };
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.lpfnWndProc = ToolProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.style = CS_DBLCLKS;	// receive double-click events
		wcex.hInstance = GetModuleHandle(NULL);

		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);

		//CBrush brBck(m_clrBackground);
		//wcex.hbrBackground = brBck.get();

		wcex.lpszClassName = _T("cui_raw Tooltip Window");

		RegisterClassEx(&wcex);

		int x = 0, y = 0, cx = 200, cy = 20;

		x = rectControl.left + ((rectControl.right - rectControl.left) - cx) / 2;
		y = rectControl.bottom + 5;

		// Perform initialization
		tooltip.hWndTooltip = CreateWindowEx(NULL,
			wcex.lpszClassName, tooltip.sText.c_str(),
			WS_POPUP,
			x, y, cx, cy,
			GetParent(tooltip.hWndControl), NULL, GetModuleHandle(NULL), &tooltip);
	}
	else
	{
		// align tooltip to control and show it
		SendMessage(tooltip.hWndTooltip, WM_COMMAND, IDC_MSG, NULL);
	}

	return;
} // ShowToolTip

void HideToolTip(cui_rawImpl::ToolTipControl &tooltip)
{
	tooltip.bAllowToolTip = false;

	// add implementation for hiding tooltips
	ShowWindow(tooltip.hWndTooltip, SW_HIDE);

	return;
} // HideToolTip
