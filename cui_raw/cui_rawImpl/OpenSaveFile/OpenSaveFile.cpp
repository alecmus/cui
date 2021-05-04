/*
** OpenSaveFile.cpp - open and save file implementation
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

#include <Windows.h>
#include <CommCtrl.h>
#include "../../cui_raw.h"
#include "../cui_rawImpl.h"
#include "../DrawRoundRect/DrawRoundRect.h"
#include "../../scaleAdjust/scaleAdjust.h"
#include "../../XCreateFont/XCreateFont.h"

struct stateinfo_ofn
{
	COLORREF clrUIbk;
	COLORREF clrUItxt;
	COLORREF clrUItheme;
	COLORREF clrUIthemeDarker;
	int IDP_PNGIconSmall = 0;
	cui_rawImpl::ImageControl captionIconControl;
	cui_rawImpl::TextControl titleTextControl;

	std::basic_string<TCHAR> sFontName;
	double iFontSize;

	std::basic_string<TCHAR> sTitle;

	CResizer *m_pResizer = NULL;
	CShadow *m_pShadow = NULL;

	cui_rawImpl::RectControl leftBorder, topBorder, rightBorder, bottomBorder;

	bool bEnableResize = false;	// TO-DO: make this work. It has issues. Do not use until fixed!!!

	bool bLarge = true;

	HBRUSH m_UIbckgndBrush = NULL;

	bool bHideNavigationToolbar = false;

	cui_rawImpl *d = NULL;

	HFONT hfont = nullptr;
};	// state info struct

enum enumCustomMessage
{
	WM_STARTED = 11119,	// message to send to pass the stateinfo struct
};

//IDs of the controls in the shell dialog
static const int STATIC1 = 0x443;
static const int COMBO1 = 0x471;
static const int STATIC2 = 0x440;
static const int TOOLBAR1 = 0x440;	// TO-DO: find a way to move this toolbar
static const int TOOLBAR2 = 0x4A0;
static const int LISTBOX1 = 0x460;
static const int DEFVIEW1 = 0x461;
static const int STATIC3 = 0x442;
static const int COMBO2 = 0x47C;
static const int STATIC4 = 0x441;
static const int COMBO3 = 0x470;
static const int BUTTON1 = 0x410;
static const int BUTTON2 = 0x1;
static const int BUTTON3 = 0x2;
static const int BUTTON4 = 0x0;
static const int DLG1 = 0x0;

/*
** move control window down by the selected offset value
*/
void movectrl(HWND hWnd, int ID, int yoffset, int xoffset, int cxoffset)
{
	RECT rec;
	HWND h_ctrl = GetDlgItem(hWnd, ID);

	if (h_ctrl != NULL)
	{
		GetWindowRect(h_ctrl, &rec);

		POINT pt = { 0, 0 };
		ScreenToClient(hWnd, &pt);			// change reference of coordinates to client area
		int x = rec.left + pt.x;
		int y = rec.top + pt.y;

		::SetWindowPos(h_ctrl, NULL,
			x + xoffset, y + yoffset, rec.right - rec.left + cxoffset, rec.bottom - rec.top, SWP_NOZORDER);
	}
} // movectrl

void movectrl_to_another_control_y(HWND hWnd, int ID, int IDtoMatchYof)
{
	// use y-coordinate of another control
	int y = 0;
	{
		RECT rec;
		GetWindowRect(GetDlgItem(hWnd, IDtoMatchYof), &rec);

		POINT pt = { 0, 0 };
		ScreenToClient(hWnd, &pt);			// change reference of coordinates to client area
		y = rec.top + pt.y;
	}

	RECT rec;
	HWND h_ctrl = GetDlgItem(hWnd, ID);

	if (h_ctrl != NULL)
	{
		GetWindowRect(h_ctrl, &rec);

		POINT pt = { 0, 0 };
		ScreenToClient(hWnd, &pt);			// change reference of coordinates to client area
		int x = rec.left + pt.x;

		::SetWindowPos(h_ctrl, NULL,
			x, y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
	}
} // movectrl_to_another_control_y

static bool design = false;

  /*
  ** change layout of dialog
  */
static void ChangeLayout(HWND hWnd, stateinfo_ofn* pThis)
{
	// change fonts
	HDC hdc = GetDC(hWnd);

	pThis->hfont = XCreateFont(hdc, pThis->sFontName, pThis->iFontSize);
	SetWindowFont(GetDlgItem(hWnd, STATIC1), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, COMBO1), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, STATIC2), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, TOOLBAR1), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, TOOLBAR2), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, LISTBOX1), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, DEFVIEW1), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, STATIC3), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, COMBO2), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, STATIC4), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, COMBO3), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, BUTTON1), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, BUTTON2), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, BUTTON3), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, BUTTON4), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, DLG1), pThis->hfont, TRUE);

	ReleaseDC(hWnd, hdc);

	//calculate y-position offset
	int themetitlebar_ = int(0.5 + pThis->d->m_iTitlebarHeight);
	int themeboarder_ = 1;

	int yoffset = themetitlebar_ + themeboarder_;

	RECT rect;
	::GetClientRect(hWnd, &rect);
	rect.bottom += (themetitlebar_ + themeboarder_);

	/*
	** center window to parent
	*/
	RECT mainRect;
	GetWindowRect(::GetParent(hWnd), &mainRect);
	int x = (mainRect.right + mainRect.left) / 2 - (rect.right - rect.left) / 2;
	int y = (mainRect.bottom + mainRect.top) / 2 - (rect.bottom - rect.top) / 2;
	::MoveWindow(hWnd, x, y, (rect.right - rect.left), (rect.bottom - rect.top), TRUE);

	//constants
	const int DLG_WIDTH = rect.right - rect.left;
	const int DLG_HEIGHT = rect.bottom - rect.top;

	const int BUTTON_H = 22;
	const int BUTTON_W = 75;

	const int MARGIN = 10;

	// move controls
	bool bCustomMove = true;

	int m_ixoffset = 0;

	if (pThis->bHideNavigationToolbar)
	{
		// disable toolbar on the left
		EnableWindow(GetDlgItem(hWnd, TOOLBAR2), FALSE);

		// hide toolbar on the left
		ShowWindow(GetDlgItem(hWnd, TOOLBAR2), SW_HIDE);

		RECT rcToolbar, rclistview;
		GetWindowRect(GetDlgItem(hWnd, TOOLBAR2), &rcToolbar);
		GetWindowRect(GetDlgItem(hWnd, LISTBOX1), &rclistview);

		m_ixoffset = rcToolbar.left - rclistview.left;
	}

	// static1 ("Look in:" static text)
	cui_raw::onResize resize;
	{
		movectrl(hWnd, STATIC1, yoffset, 0, 0);

		resize.iPercCY = 0;
		resize.iPercCX = 0;
		resize.iPercH = 0;
		resize.iPercV = 0;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, STATIC1), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	// combo1 ("Folder list combobox at the top")
	{
		movectrl(hWnd, COMBO1, yoffset, 0, 0);

		resize.iPercCY = 0;
		resize.iPercCX = 100;
		resize.iPercH = 0;
		resize.iPercV = 0;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, COMBO1), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	{
		// static 2 (somehow connected to TOOLBAR1)
		movectrl(hWnd, STATIC2, yoffset, 0, 0);

		resize.iPercCY = 0;
		resize.iPercCX = 0;
		resize.iPercH = 100;
		resize.iPercV = 0;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, STATIC2), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);

		// toolbar1 (four navigation icons on top right, left of combobox)
		movectrl_to_another_control_y(hWnd, TOOLBAR1, COMBO1);	// align this toolbar to the combo y-coordinate

		resize.iPercCY = 0;
		resize.iPercCX = 0;
		resize.iPercH = 100;
		resize.iPercV = 0;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, TOOLBAR1), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	// toolbar 2 (icons on the left)
	{
		movectrl(hWnd, TOOLBAR2, yoffset, 0, 0);

		resize.iPercCY = 0;
		resize.iPercCX = 0;
		resize.iPercH = 0;
		resize.iPercV = 0;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, TOOLBAR2), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	// listbox 1
	{
		movectrl(hWnd, LISTBOX1, yoffset, m_ixoffset, 0 - m_ixoffset);

		resize.iPercCY = 100;
		resize.iPercCX = 100;
		resize.iPercH = 0;
		resize.iPercV = 0;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, LISTBOX1), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);

	}

	// skip this
	//movectrl(hWnd, DEFVIEW1, yoffset, m_ixoffset);

	// static 3 ("File name:" static text)
	{
		movectrl(hWnd, STATIC3, yoffset, m_ixoffset, 0 - m_ixoffset);

		resize.iPercCY = 0;
		resize.iPercCX = 0;
		resize.iPercH = 0;
		resize.iPercV = 100;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, STATIC3), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	// combo 2 (File name list)
	{
		movectrl(hWnd, COMBO2, yoffset, m_ixoffset, 0 - m_ixoffset);

		resize.iPercCY = 0;
		resize.iPercCX = 100;
		resize.iPercH = 0;
		resize.iPercV = 100;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, COMBO2), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	// static 4 ("Files of type:" static text)
	{
		movectrl(hWnd, STATIC4, yoffset, m_ixoffset, 0 - m_ixoffset);

		resize.iPercCY = 0;
		resize.iPercCX = 0;
		resize.iPercH = 0;
		resize.iPercV = 100;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, STATIC4), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	// combo 3 (File type selector)
	{
		movectrl(hWnd, COMBO3, yoffset, m_ixoffset, 0 - m_ixoffset);

		resize.iPercCY = 0;
		resize.iPercCX = 100;
		resize.iPercH = 0;
		resize.iPercV = 100;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, COMBO3), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	// button1
	{
		movectrl(hWnd, BUTTON1, yoffset, m_ixoffset, 0 - m_ixoffset);

		resize.iPercCY = 0;
		resize.iPercCX = 0;
		resize.iPercH = 100;
		resize.iPercV = 100;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, BUTTON1), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	// button 2 ("Open" button on bottom right)
	{
		movectrl(hWnd, BUTTON2, yoffset, 0, 0);

		resize.iPercCY = 0;
		resize.iPercCX = 0;
		resize.iPercH = 100;
		resize.iPercV = 100;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, BUTTON2), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	// button 3 ("Cancel" button on bottom right)
	{
		movectrl(hWnd, BUTTON3, yoffset, 0, 0);

		resize.iPercCY = 0;
		resize.iPercCX = 0;
		resize.iPercH = 100;
		resize.iPercV = 100;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, BUTTON3), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	// button 4
	{
		movectrl(hWnd, BUTTON4, yoffset, 0, 0);

		resize.iPercCY = 0;
		resize.iPercCX = 0;
		resize.iPercH = 100;
		resize.iPercV = 100;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, BUTTON4), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	// dlg1
	{
		movectrl(hWnd, DLG1, yoffset, m_ixoffset, 0 - m_ixoffset);

		resize.iPercCY = 100;
		resize.iPercCX = 100;
		resize.iPercH = 0;
		resize.iPercV = 0;

		if (bCustomMove)
			pThis->m_pResizer->OnResize(GetDlgItem(hWnd, DLG1), resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}
} // ChangeLayout

static void addCaptionIcon(HWND hWnd, stateinfo_ofn* pThis, int &iCaptionIconWidth)
{
	iCaptionIconWidth = 0;
	int m_iTitlebarHeight = int(0.5 + pThis->d->m_iTitlebarHeight / pThis->d->m_DPIScale);

	if (pThis->IDP_PNGIconSmall)
	{
		int iCaptionIconSize = 16;

		// define rect for window title (centered vertically)
		RECT rc;
		rc.top = (m_iTitlebarHeight - iCaptionIconSize) / 2;	// TO-DO: remove magic number
		rc.bottom = rc.top + iCaptionIconSize;
		rc.left = rc.top;
		rc.right = rc.left + iCaptionIconSize;

		scaleRECT(rc, pThis->d->m_DPIScale);

		iCaptionIconWidth += m_iTitlebarHeight;

		// add image control to place caption icon in
		pThis->captionIconControl.iUniqueID = -10;	// TO-DO: remove magic number
		pThis->captionIconControl.sText = _T("");
		pThis->captionIconControl.coords.left = rc.left;
		pThis->captionIconControl.coords.top = rc.top;
		pThis->captionIconControl.coords.right = rc.right;
		pThis->captionIconControl.coords.bottom = rc.bottom;

		cui_raw::onResize resize;
		pThis->captionIconControl.resize = resize;
		pThis->captionIconControl.clrText = RGB(0, 0, 0);
		pThis->captionIconControl.sFontName = _T("");
		pThis->captionIconControl.iFontSize = 10;

		pThis->captionIconControl.iPNGResource = pThis->IDP_PNGIconSmall;	// load icon from PNG resource

		pThis->captionIconControl.sFileName = _T("");
		pThis->captionIconControl.clrBackground = pThis->clrUIbk;
		pThis->captionIconControl.clrBorder = RGB(255, 255, 255);
		pThis->captionIconControl.clrBorderHot = RGB(255, 255, 255);
		pThis->captionIconControl.bButtonBar = false;
		pThis->captionIconControl.clrBar = RGB(255, 255, 255);
		pThis->captionIconControl.bToggle = false;
		pThis->captionIconControl.toggleAction = cui_raw::onToggle::toggleDown;
		pThis->captionIconControl.bImageOnlyTightFit = true;	// critical for caption icon
		pThis->captionIconControl.d = pThis->d;

		DWORD dwStyle = WS_TABSTOP | WS_VISIBLE | WS_CHILD;

		int x, y, cx, cy;

		if (pThis->captionIconControl.bToggle)
		{
			switch (pThis->captionIconControl.toggleAction)
			{
			case cui_raw::toggleLeft:
			{
				x = pThis->captionIconControl.coords.left + pThis->captionIconControl.iOffset;
				y = pThis->captionIconControl.coords.top;
				cx = pThis->captionIconControl.coords.width() - pThis->captionIconControl.iOffset;
				cy = pThis->captionIconControl.coords.height();
			}
			break;

			case cui_raw::toggleRight:
			{
				x = pThis->captionIconControl.coords.left;
				y = pThis->captionIconControl.coords.top;
				cx = pThis->captionIconControl.coords.width() - pThis->captionIconControl.iOffset;
				cy = pThis->captionIconControl.coords.height();
			}
			break;

			case cui_raw::toggleUp:
			{
				x = pThis->captionIconControl.coords.left;
				y = pThis->captionIconControl.coords.top + pThis->captionIconControl.iOffset;
				cx = pThis->captionIconControl.coords.width();
				cy = pThis->captionIconControl.coords.height() - pThis->captionIconControl.iOffset;
			}
			break;

			case cui_raw::toggleDown:
			{
				x = pThis->captionIconControl.coords.left;
				y = pThis->captionIconControl.coords.top;
				cx = pThis->captionIconControl.coords.width();
				cy = pThis->captionIconControl.coords.height() - pThis->captionIconControl.iOffset;
			}
			break;

			default:
			{
				x = pThis->captionIconControl.coords.left;
				y = pThis->captionIconControl.coords.top + pThis->captionIconControl.iOffset;
				cx = pThis->captionIconControl.coords.width();
				cy = pThis->captionIconControl.coords.height() - pThis->captionIconControl.iOffset;
			}
			break;
			}
		}
		else
		{
			x = pThis->captionIconControl.coords.left;
			y = pThis->captionIconControl.coords.top;
			cx = pThis->captionIconControl.coords.width();
			cy = pThis->captionIconControl.coords.height();
		}

		// add static control
		pThis->captionIconControl.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle, x, y, cx, cy,
			hWnd, (HMENU)(INT_PTR)pThis->captionIconControl.iUniqueID,
			(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

		if (pThis->captionIconControl.iPNGResource)
		{
			// attempt to load image from PNG resource
			std::basic_string<TCHAR> sErr;
			bool bRes = pThis->captionIconControl.GdiplusBitmap_res.Load(pThis->captionIconControl.iPNGResource, _T("PNG"), sErr, pThis->d->m_hResModule);
		}
		else
			if (!pThis->captionIconControl.sFileName.empty())
			{
				// attempt to load image from file

				// TO-DO: implement error response
				std::basic_string<TCHAR> sErr;
				bool bRes = pThis->captionIconControl.GdiplusBitmap.Load(pThis->captionIconControl.sFileName.c_str(), sErr);
			}

		// subclass static control so we can do custom drawing
		SetWindowLongPtr(pThis->captionIconControl.hWnd, GWLP_USERDATA, (LONG_PTR)&pThis->captionIconControl);
		pThis->captionIconControl.PrevProc = SetWindowLongPtr(pThis->captionIconControl.hWnd, GWLP_WNDPROC, (LONG_PTR)cui_rawImpl::ImageProc);

		// set resizing behaviour
		pThis->m_pResizer->OnResize(pThis->captionIconControl.hWnd, pThis->captionIconControl.resize.iPercH, pThis->captionIconControl.resize.iPercV, pThis->captionIconControl.resize.iPercCX, pThis->captionIconControl.resize.iPercCY);
	}
} // addCaptionIcon

static void addWindowTitle(HWND hWnd, stateinfo_ofn* pThis, int iCaptionIconWidth)
{
	auto text_size = [&](const std::basic_string<TCHAR> &text,
		const double &font_size,
		const std::basic_string<TCHAR> &font_name,
		const double &max_text_width)
	{
		HDC hdcScreen = GetDC(NULL);

		// capture current DPI scale
		const Gdiplus::REAL dpi_scale =
			(Gdiplus::REAL)GetDeviceCaps(hdcScreen, LOGPIXELSY) / (Gdiplus::REAL)96.0f;

		Gdiplus::Graphics graphics(hdcScreen);

		Gdiplus::Font* p_font = new Gdiplus::Font(&Gdiplus::FontFamily(font_name.c_str()),
			static_cast<Gdiplus::REAL>(font_size));

		if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
		{
			delete p_font;
			p_font = nullptr;
			p_font = new Gdiplus::Font(font_name.c_str(),
				static_cast<Gdiplus::REAL>(font_size), Gdiplus::FontStyle::FontStyleRegular,
				Gdiplus::UnitPoint, &pThis->d->m_font_collection);
		}

		Gdiplus::RectF text_rect;
		Gdiplus::RectF layoutRect;
		layoutRect.Width = static_cast<Gdiplus::REAL>(max_text_width);

		graphics.MeasureString(text.c_str(), -1, p_font, layoutRect, &text_rect);

		if (true)
		{
			if (text_rect.Width < layoutRect.Width)
				text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
		}

		delete p_font;
		p_font = nullptr;

		auto round_up = [](const Gdiplus::REAL &real)
		{
			LONG long_ = static_cast<LONG>(real);
			Gdiplus::REAL real_ = static_cast<Gdiplus::REAL>(long_);

			if (real_ < real)
				long_++;

			return long_;
		}; // round_up

		SIZE size;
		size.cx = round_up(text_rect.Width / dpi_scale);
		size.cy = round_up(text_rect.Height / dpi_scale);

		ReleaseDC(NULL, hdcScreen);

		return size;
	}; // text_size

	HDC hdc = GetDC(hWnd);

	SIZE size = text_size(pThis->sTitle, pThis->iFontSize, pThis->sFontName, 0);

	// define rect for window title (centered vertically)
	RECT rc;

	if (iCaptionIconWidth)
		rc.left = iCaptionIconWidth;	// TO-DO: remove magic number
	else
		rc.left = 10;

	rc.right = rc.left + size.cx;

	rc.top = 1;
	rc.bottom = int(0.5 + (double)pThis->d->m_iTitlebarHeight / pThis->d->m_DPIScale) - 1;

	scaleRECT(rc, pThis->d->m_DPIScale);

	// add text control
	pThis->titleTextControl.iUniqueID = -5;			// TO-DO: remove magic number
	pThis->titleTextControl.sText = pThis->sTitle;
	pThis->titleTextControl.sFontName = pThis->sFontName;
	pThis->titleTextControl.iFontSize = pThis->iFontSize;
	pThis->titleTextControl.coords.left = rc.left;
	pThis->titleTextControl.coords.top = rc.top;
	pThis->titleTextControl.coords.right = rc.right;
	pThis->titleTextControl.coords.bottom = rc.bottom;
	pThis->titleTextControl.clrText = pThis->clrUItheme;
	pThis->titleTextControl.align = cui_raw::textAlignment::middleleft;
	pThis->titleTextControl.bMultiLine = false;
	pThis->titleTextControl.d = pThis->d;

	cui_raw::onResize resize;
	pThis->titleTextControl.resize = resize;

	DWORD dwStyle = WS_VISIBLE | WS_CHILD;

	// set background color
	pThis->titleTextControl.clrBackground = pThis->clrUIbk;

	pThis->titleTextControl.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
		pThis->titleTextControl.coords.left,
		pThis->titleTextControl.coords.top,
		pThis->titleTextControl.coords.width(),
		pThis->titleTextControl.coords.height(),
		hWnd, (HMENU)(INT_PTR)pThis->captionIconControl.iUniqueID,
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

					// subclass static control so we can do custom drawing
	SetWindowLongPtr(pThis->titleTextControl.hWnd,
		GWLP_USERDATA, (LONG_PTR)&pThis->titleTextControl);
	pThis->titleTextControl.PrevProc =
		SetWindowLongPtr(pThis->titleTextControl.hWnd,
			GWLP_WNDPROC, (LONG_PTR)cui_rawImpl::TextControlProc);

	// set resizing behaviour
	pThis->m_pResizer->OnResize(pThis->titleTextControl.hWnd,
		pThis->titleTextControl.resize.iPercH, pThis->titleTextControl.resize.iPercV,
		pThis->titleTextControl.resize.iPercCX, pThis->titleTextControl.resize.iPercCY);
} // addWindowTitle

static void addRect(HWND hWnd, int iUniqueID, cui_rawImpl::RectControl* pControl, stateinfo_ofn* pThis)
{
	DWORD dwStyle = WS_VISIBLE | WS_CHILD;

	pControl->hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
		pControl->coords.left,
		pControl->coords.top,
		pControl->coords.width(),
		pControl->coords.height(),
		hWnd, (HMENU)(INT_PTR)pControl->iUniqueID, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);

					// subclass static control so we can do custom drawing
	SetWindowLongPtr(pControl->hWnd, GWLP_USERDATA, (LONG_PTR)pControl);
	pControl->PrevProc =
		SetWindowLongPtr(pControl->hWnd, GWLP_WNDPROC, (LONG_PTR)cui_rawImpl::RectProc);

	// set resizing behaviour
	pThis->m_pResizer->OnResize(pControl->hWnd, pControl->resize.iPercH, pControl->resize.iPercV,
		pControl->resize.iPercCX, pControl->resize.iPercCY);
} // addRect

static void addBorders(HWND hWnd, stateinfo_ofn *pThis)
{
	// add window borders
	int m_icy, m_icx;
	RECT rcWindow;
	GetWindowRect(hWnd, &rcWindow);
	m_icx = rcWindow.right - rcWindow.left;
	m_icy = rcWindow.bottom - rcWindow.top;

	int m_iLeftBorder = -300;
	int m_iTopBorder = -301;
	int m_iRightBorder = -302;
	int m_iBottomBorder = -304;

	// add left border
	RECT rc;
	rc.left = 0;
	rc.right = 1;
	rc.top = 0;
	rc.bottom = m_icy;

	cui_raw::onResize resize;
	resize.iPercH = 0;
	resize.iPercV = 0;
	resize.iPercCY = 100;
	resize.iPercCX = 0;
	{
		pThis->leftBorder.clr = pThis->clrUItheme;
		pThis->leftBorder.coords.left = rc.left;
		pThis->leftBorder.coords.top = rc.top;
		pThis->leftBorder.coords.right = rc.right;
		pThis->leftBorder.coords.bottom = rc.bottom;
		pThis->leftBorder.iUniqueID = m_iLeftBorder;
		pThis->leftBorder.resize = resize;
	}
	addRect(hWnd, m_iLeftBorder, &pThis->leftBorder, pThis);

	// add top border
	rc.bottom = 1;
	rc.right = m_icx;
	resize.iPercCY = 0;
	resize.iPercCX = 100;
	{
		pThis->topBorder.clr = pThis->clrUItheme;
		pThis->topBorder.coords.left = rc.left;
		pThis->topBorder.coords.top = rc.top;
		pThis->topBorder.coords.right = rc.right;
		pThis->topBorder.coords.bottom = rc.bottom;
		pThis->topBorder.iUniqueID = m_iTopBorder;
		pThis->topBorder.resize = resize;
	}
	addRect(hWnd, m_iTopBorder, &pThis->topBorder, pThis);

	// add right border
	rc.left = m_icx - 1;
	rc.bottom = m_icy;
	resize.iPercH = 100;
	resize.iPercV = 0;
	resize.iPercCY = 100;
	resize.iPercCX = 0;
	{
		pThis->rightBorder.clr = pThis->clrUItheme;
		pThis->rightBorder.coords.left = rc.left;
		pThis->rightBorder.coords.top = rc.top;
		pThis->rightBorder.coords.right = rc.right;
		pThis->rightBorder.coords.bottom = rc.bottom;
		pThis->rightBorder.iUniqueID = m_iRightBorder;
		pThis->rightBorder.resize = resize;
	}
	addRect(hWnd, m_iRightBorder, &pThis->rightBorder, pThis);

	// add bottom border
	rc.left = 0;
	rc.top = m_icy - 1;
	resize.iPercH = 0;
	resize.iPercV = 100;
	resize.iPercCY = 0;
	resize.iPercCX = 100;
	{
		pThis->bottomBorder.clr = pThis->clrUItheme;
		pThis->bottomBorder.coords.left = rc.left;
		pThis->bottomBorder.coords.top = rc.top;
		pThis->bottomBorder.coords.right = rc.right;
		pThis->bottomBorder.coords.bottom = rc.bottom;
		pThis->bottomBorder.iUniqueID = m_iBottomBorder;
		pThis->bottomBorder.resize = resize;
	}
	addRect(hWnd, m_iBottomBorder, &pThis->bottomBorder, pThis);
} // addBorders

static bool CreateShadow(
	HWND hWnd,
	stateinfo_ofn *pThis
)
{
	if (pThis->m_pShadow)
	{
		// initialize drop shadow parameters
		CShadow::shadow_properties properties;
		properties.color = pThis->clrUItheme;
		properties.darkness = 50;
		properties.position = { 0, 0 };
		properties.sharpness = 20;
		properties.size = 10;

		pThis->m_pShadow->CreateShadow(hWnd, properties);

		return true;
	}
	else
		return false;
} // CreateShadow

static WNDPROC oldProc;
static INT_PTR CALLBACK subProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	stateinfo_ofn* pState = NULL; if ((msg == WM_COMMAND) && (LOWORD(wParam) == (DWORD)WM_STARTED)) { pState = (stateinfo_ofn*)lParam; SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pState); }
	else { pState = reinterpret_cast<stateinfo_ofn*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)); }

	switch (msg)
	{
	case WM_COMMAND:
		if (LOWORD(wParam) == WM_STARTED)
		{
			/*
			** set the CCS_NOPARENTALIGN style to tool bar 1 to enable moving it
			** TO-DO: find a way to ADD this control to the existing flags and REMOVE incompatible flags
			*/
			SetWindowLong(GetDlgItem(hWnd, TOOLBAR1), GWL_STYLE, CCS_NOPARENTALIGN);

			// change layout of dialog
			ChangeLayout(hWnd, pState);

			// add caption icon
			int iCaptionIconWidth = 0;
			addCaptionIcon(hWnd, pState, iCaptionIconWidth);

			// add title
			addWindowTitle(hWnd, pState, iCaptionIconWidth);

			// add borders
			addBorders(hWnd, pState);

			// enable control resizing
			pState->m_pResizer->enable(hWnd);

			int cx, cy;

			if (pState->bLarge)
			{
				cx = int(0.5 + 780 * pState->d->m_DPIScale);
				cy = int(0.5 + 550 * pState->d->m_DPIScale);
			}
			else
			{
				RECT rc;
				GetWindowRect(hWnd, &rc);
				cx = rc.right - rc.left + 20;
				cy = rc.bottom - rc.top;
			}

			{
				// resize window
				SetWindowPos(hWnd, NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOMOVE);

				RECT rect;
				::GetClientRect(hWnd, &rect);

				/*
				** center window to parent
				*/
				RECT mainRect;
				GetWindowRect(::GetParent(hWnd), &mainRect);
				int x = (mainRect.right + mainRect.left) / 2 - (rect.right - rect.left) / 2;
				int y = (mainRect.bottom + mainRect.top) / 2 - (rect.bottom - rect.top) / 2;
				::MoveWindow(hWnd, x, y, (rect.right - rect.left), (rect.bottom - rect.top), TRUE);
			}

			if (pState->d->m_bShadow)
			{
				// add shadow
				CreateShadow(hWnd, pState);
			}
		}
		break;

	case WM_ERASEBKGND:
	{
		HDC hdc = (HDC)wParam;

		RECT rect;
		GetClientRect(hWnd, &rect);

		CBrush hbr(pState->clrUIbk);
		FillRect(hdc, &rect, hbr.get());

		return TRUE;
	}
	break;

	// not called when WM_ERASEBKGND has been handled
	case WM_CTLCOLORDLG:
	{
		// essential for toolbars
		return (INT_PTR)pState->m_UIbckgndBrush;
	}
	break;

	case WM_CTLCOLORSTATIC:
	{
		// essential for static controls
		HDC hdcStatic = (HDC)wParam;

		SetTextColor(hdcStatic, pState->clrUItxt);

		SetBkMode(hdcStatic, TRANSPARENT);

		return (INT_PTR)pState->m_UIbckgndBrush;
	}
	break;

	case WM_NCHITTEST:
	{
		int m_iTitlebarHeight = 30;	// TO-DO: remove magic number

		RECT WindowRect;
		int x, y;

		GetWindowRect(hWnd, &WindowRect);
		x = GET_X_LPARAM(lParam) - WindowRect.left;
		y = GET_Y_LPARAM(lParam) - WindowRect.top;

		int m_iBoarderWidth = 1;	// TO-DO: remove magic number
		int iBoarder = m_iBoarderWidth + 4;

		if (x >= iBoarder && x <= WindowRect.right - WindowRect.left - iBoarder && y >= iBoarder && y <= m_iTitlebarHeight)
		{
			// mouse is over title bar, return HTCAPTION
			SetWindowLong(hWnd, DWLP_MSGRESULT, HTCAPTION);
			return HTCAPTION;
		}
		else if (!pState->bEnableResize)
		{
			return HTCLIENT;
		}
		else if (x < iBoarder && y < iBoarder)
		{
			SetWindowLong(hWnd, DWLP_MSGRESULT, HTTOPLEFT);
			return HTTOPLEFT;
		}
		else if (x > WindowRect.right - WindowRect.left - iBoarder && y < iBoarder)
		{
			SetWindowLong(hWnd, DWLP_MSGRESULT, HTTOPRIGHT);
			return HTTOPRIGHT;
		}
		else if (x > WindowRect.right - WindowRect.left - iBoarder && y > WindowRect.bottom - WindowRect.top - iBoarder)
		{
			SetWindowLong(hWnd, DWLP_MSGRESULT, HTBOTTOMRIGHT);
			return HTBOTTOMRIGHT;
		}
		else if (x < iBoarder && y > WindowRect.bottom - WindowRect.top - iBoarder)
		{
			SetWindowLong(hWnd, DWLP_MSGRESULT, HTBOTTOMLEFT);
			return HTBOTTOMLEFT;
		}
		else if (x < iBoarder)
		{
			SetWindowLong(hWnd, DWLP_MSGRESULT, HTLEFT);
			return HTLEFT;
		}
		else if (y < iBoarder)
		{
			SetWindowLong(hWnd, DWLP_MSGRESULT, HTTOP);
			return HTTOP;
		}
		else if (x > WindowRect.right - WindowRect.left - iBoarder)
		{
			SetWindowLong(hWnd, DWLP_MSGRESULT, HTRIGHT);
			return HTRIGHT;
		}
		else if (y > WindowRect.bottom - WindowRect.top - iBoarder)
		{
			SetWindowLong(hWnd, DWLP_MSGRESULT, HTBOTTOM);
			return HTBOTTOM;
		}
		else
		{
			SetWindowLong(hWnd, DWLP_MSGRESULT, HTCLIENT);
			return HTCLIENT;
		}
	}
	break;

	case WM_DRAWITEM:
	{
		// draw the owner draw button
		LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;

		int iIDFrom = lpDIS->CtlID;

		// check whether this is one of our custom buttons
		bool bButton = false;

		if (iIDFrom == BUTTON1 || iIDFrom == BUTTON2 || iIDFrom == BUTTON3 || iIDFrom == BUTTON4)
			bButton = true;

		if (bButton)
		{
			HDC dc = lpDIS->hDC;

			RECT itemRect = lpDIS->rcItem;
			RECT outerRect = itemRect;

			itemRect.left += 1;
			itemRect.right -= 1;
			itemRect.top += 1;
			itemRect.bottom -= 1;

			SetBkMode(dc, TRANSPARENT);

			// button state
			BOOL bIsPressed = (lpDIS->itemState & ODS_SELECTED);
			BOOL bIsFocused = (lpDIS->itemState & ODS_FOCUS);
			BOOL bIsDisabled = (lpDIS->itemState & ODS_DISABLED);
			BOOL bDrawFocusRect = !(lpDIS->itemState & ODS_NOFOCUSRECT);

			// draw background
			CBrush brBackground(pState->clrUIbk);
			FillRect(dc, &outerRect, brBackground.get());

			////////////////////////////////////////////////////////////////////
			// draw button boarder
			COLORREF clrDisabled = RGB(200, 200, 200);

			Gdiplus::Graphics graphics(dc);

			{
				// draw background
				COLORREF clrBoarder = pState->clrUItheme;

				if (bIsDisabled)
					clrBoarder = clrDisabled;

				CBrush brBackground(clrBoarder);

				DrawRoundRect(graphics, outerRect.left, outerRect.top, outerRect.right, outerRect.bottom, 3, clrBoarder, clrBoarder, 1, true);
			} // if (bIsFocused)

			COLORREF clrBackground;
			COLORREF clrText;

			COLORREF clrHot = pState->clrUItheme;
			COLORREF clrCold = pState->clrUIthemeDarker;
			COLORREF clrColdHover = clrCold;

			if (!bIsPressed)
			{
				clrBackground = clrHot;
				clrText = pState->clrUIbk;
			}
			else
			{
				clrBackground = clrCold;
				clrText = pState->clrUIbk;
			}

			if (bIsDisabled)
				clrText = clrDisabled;

			// draw background
			DrawRoundRect(graphics, itemRect.left, itemRect.top, itemRect.right, itemRect.bottom, 3, clrBackground, clrBackground, 1, true);

			// Read the button's title
			TCHAR sTitle[100];
			GetWindowText(lpDIS->hwndItem, sTitle, 100);

			RECT captionRect = lpDIS->rcItem;

			BOOL bHasTitle = (sTitle[0] != '\0');

			// Write the button title (if any)
			if (bHasTitle)
			{
				Gdiplus::Font* p_font = new Gdiplus::Font(&Gdiplus::FontFamily(pState->sFontName.c_str()),
					static_cast<Gdiplus::REAL>(pState->iFontSize));

				if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
				{
					delete p_font;
					p_font = nullptr;
					p_font = new Gdiplus::Font(pState->sFontName.c_str(),
						static_cast<Gdiplus::REAL>(pState->iFontSize), Gdiplus::FontStyle::FontStyleRegular,
						Gdiplus::UnitPoint, &pState->d->m_font_collection);
				}

				Gdiplus::StringFormat format;
				format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
				format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);
				format.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);
				format.SetHotkeyPrefix(Gdiplus::HotkeyPrefix::HotkeyPrefixShow);

				Gdiplus::Color color;
				color.SetFromCOLORREF(clrText);
				Gdiplus::SolidBrush text_brush(color);

				Gdiplus::RectF layoutRect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(itemRect);

				// measure text rectangle
				Gdiplus::RectF text_rect;
				graphics.MeasureString(std::basic_string<TCHAR>(sTitle).c_str(), -1, p_font, layoutRect, &text_rect);

				if (true)
				{
					if (text_rect.Width < layoutRect.Width)
						text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
				}

				// align the text rectangle to the layout rectangle
				liblec::cui::gui_raw::cui_rawImpl::align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::middle);

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
				graphics.DrawString(sTitle,
					-1, p_font, text_rect, &format, &text_brush);

				delete p_font;
				p_font = nullptr;
			}

			break;
		} // if (bButton)
	}
	break;

	default:
		break;
	}
	return CallWindowProc(oldProc, hWnd, msg, wParam, lParam);
} // subProc

  /*
  ** customization callback function
  */
UINT_PTR CALLBACK customizationcallback(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	OPENFILENAME* pofn = NULL; if (msg == WM_INITDIALOG) { pofn = (OPENFILENAME*)lParam; SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pofn); }
	else { pofn = reinterpret_cast<OPENFILENAME*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)); }

	switch (msg)
	{
	case WM_INITDIALOG:
	{
		/*
		** change dialog window style
		*/
		SetWindowLongPtr(GetParent(hWnd), GWL_STYLE, WS_POPUP);	// set ONLY the WS_POPUP style
		SetWindowLongPtr(GetParent(hWnd), GWL_EXSTYLE, 0);		// remove all extended styles

																// get window style of buttons and make them custom drawn
		DWORD dwStyle = (DWORD)GetWindowLong(GetDlgItem(GetParent(hWnd), BUTTON1), GWL_STYLE);
		dwStyle |= BS_OWNERDRAW;
		SetWindowLong(GetDlgItem(GetParent(hWnd), BUTTON1), GWL_STYLE, (LONG)dwStyle);

		dwStyle = (DWORD)GetWindowLong(GetDlgItem(GetParent(hWnd), BUTTON2), GWL_STYLE);
		dwStyle |= BS_OWNERDRAW;
		SetWindowLong(GetDlgItem(GetParent(hWnd), BUTTON2), GWL_STYLE, (LONG)dwStyle);

		dwStyle = (DWORD)GetWindowLong(GetDlgItem(GetParent(hWnd), BUTTON3), GWL_STYLE);
		dwStyle |= BS_OWNERDRAW;
		SetWindowLong(GetDlgItem(GetParent(hWnd), BUTTON3), GWL_STYLE, (LONG)dwStyle);

		dwStyle = (DWORD)GetWindowLong(GetDlgItem(GetParent(hWnd), BUTTON4), GWL_STYLE);
		dwStyle |= BS_OWNERDRAW;
		SetWindowLong(GetDlgItem(GetParent(hWnd), BUTTON4), GWL_STYLE, (LONG)dwStyle);

		/*
		** subclass dialog
		*/
		oldProc = (WNDPROC)SetWindowLongPtr(GetParent(hWnd), GWLP_WNDPROC, (LONG_PTR)subProc);
	}
	return (UINT_PTR)TRUE;

	case WM_NOTIFY:
	{
		if (((LPNMHDR)lParam)->code == CDN_FIRST)
		{
		}

		if (((LPNMHDR)lParam)->code == CDN_INITDONE)
		{
			OPENFILENAME ofn = (*pofn);

			stateinfo_ofn *pStateinfo;
			pStateinfo = (stateinfo_ofn*)ofn.lCustData;

			SendMessage(GetParent(hWnd), WM_COMMAND, (DWORD)WM_STARTED, (LPARAM)pStateinfo);
		}

		if (((LPNMHDR)lParam)->code == CDN_LAST)
		{
		}
	}
	return (UINT_PTR)TRUE;

	default:
		break;
	}
	return (UINT_PTR)FALSE;
} // customizationcallback

void cui_raw::openFile(
	const opensavefileParams &params,
	std::basic_string<TCHAR> &sFile
)
{
	/*
	** declare stateinfo struct and (optionally) initialize some of the members
	*/
	stateinfo_ofn State;
	State.d = d;
	State.bHideNavigationToolbar = params.bHideNavigationToolbar;

	// TO-DO: use smart pointers
	State.m_pResizer = new CResizer();
	State.m_pShadow = new CShadow;

	if (!State.bHideNavigationToolbar)
		State.clrUIbk = (COLORREF)GetSysColor(COLOR_BTNFACE);	// because toolbar looks ugly against white ... we still haven't found a way of manipulating it's color
	else
		State.clrUIbk = d->m_clrBackground;

	CBrush hbr(State.clrUIbk);
	State.m_UIbckgndBrush = hbr.get();

	/*
	If we indicate a NULL termminator with a new line character,
	The filter string format is as follows :

	Bitmap Files(*.bmp)
	*.BMP
	JPEG(*.jpg; *.jpeg)
	*.JPG; *.JPEG
	PNG(*.png)
	*.PNG
	All supported images
	*.JPG; *.JPEG; *.PNG; *.BMP
	*/
	std::basic_string<TCHAR> sFilter;
	std::vector<std::basic_string<TCHAR>> vFileTypesIndexed;

	// set filter
	{
		// create NULL terminator
		std::basic_string<TCHAR> sNULL;
		{
			std::string m_sNULL({ '\0' });
			sNULL = std::basic_string<TCHAR>(m_sNULL.begin(), m_sNULL.end());
		}

		std::vector<cui_raw::fileType> m_types;

		for (size_t i = 0; i < params.vFileTypes.size(); i++)
		{
			// check if type description hasn't already been captured; if it has, append it's extension to the other
			bool bNew = true;

			for (size_t x = 0; x < m_types.size(); x++)
			{
				if (m_types[x].sDescription == params.vFileTypes[i].sDescription)
				{
					m_types[x].sFileExtension += _T(";*.") + params.vFileTypes[i].sFileExtension;

					bNew = false;
					break;
				}
			}

			if (bNew)
			{
				cui_raw::fileType type;
				type.sDescription = params.vFileTypes[i].sDescription;
				type.sFileExtension = _T("*.") + params.vFileTypes[i].sFileExtension;
				m_types.push_back(type);
			}
		}

		std::basic_string<TCHAR> m_sAllFileExtensions;

		for (size_t i = 0; i < m_types.size(); i++)
		{
			std::basic_string<TCHAR> sDescription = m_types[i].sDescription;
			std::basic_string<TCHAR> sFileExtension = m_types[i].sFileExtension;

			if (m_sAllFileExtensions.empty())
				m_sAllFileExtensions = sFileExtension;
			else
				m_sAllFileExtensions += _T(";") + sFileExtension;

			std::basic_string<TCHAR> m_sFilter;
			m_sFilter = sDescription + _T(" (") + sFileExtension + _T(")") + sNULL;
			m_sFilter += sFileExtension + sNULL;

			sFilter += m_sFilter;
			vFileTypesIndexed.push_back(m_types[i].sFileExtension);
		}

		// add all supported files
		if (params.bIncludeAllSupportedTypes)
		{
			std::basic_string<TCHAR> m_sFilter;
			std::basic_string<TCHAR> sDescription = _T("All supported files");
			std::basic_string<TCHAR> sFileExtension = m_sAllFileExtensions;
			m_sFilter = sDescription + _T(" (") + sFileExtension + _T(")") + sNULL;
			m_sFilter += sFileExtension + sNULL;

			sFilter += m_sFilter;
			vFileTypesIndexed.push_back(_T(""));
		}
	}

	OPENFILENAME ofn;
	TCHAR _FilePath[MAX_PATH];

	State.IDP_PNGIconSmall = d->m_IDP_ICONSMALL;
	State.clrUItxt = RGB(0, 0, 0);	// TO-DO: fix this in cui_raw
	State.clrUItheme = d->m_clrTheme;
	State.clrUIthemeDarker = d->m_clrThemeDarker;
	State.sFontName = params.sFontName;
	State.iFontSize = params.iFontSize;
	State.sTitle = params.sTitle;
	State.bLarge = !params.bSmallWindow;

	/*
	** initialize OPENFILENAME
	*/
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);

	if (d->m_hWnd)
		ofn.hwndOwner = d->m_hWnd;
	else
		ofn.hwndOwner = NULL;

	ofn.lpstrFile = _FilePath;

	/*
	** set FilePath[0] to '\0' so that GetOpenFileName doesn't use the contents FilePath to initialize itself
	*/
	ofn.lpstrFile[0] = '\0';

	ofn.nMaxFile = MAX_PATH;
	ofn.lpstrFilter = sFilter.c_str();
	ofn.nFilterIndex = 1;
	ofn.nMaxFileTitle = NULL;
	ofn.lpstrInitialDir = NULL;
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrTitle = params.sTitle.c_str();

	ofn.lpfnHook = customizationcallback;	// function to call for customization
	ofn.lCustData = (LPARAM)&State;			// parameter to pass to the customization function

											/*
											** TO-DO: solve issue with window size when OFN_HIDEREADONLY is not specified
											*/
	ofn.Flags = OFN_ENABLEHOOK | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (State.bEnableResize)
	{
		ofn.Flags |= OFN_ENABLESIZING;
	}

	/*
	** Display the Open dialog Box
	*/
	if (GetOpenFileName(&ofn) != TRUE)
		sFile.clear();
	else
		sFile = _FilePath;

	// cleanup
	if (State.m_pShadow)
	{
		delete State.m_pShadow;
		State.m_pShadow = NULL;
	}

	if (State.m_pResizer)
	{
		delete State.m_pResizer;
		State.m_pResizer = NULL;
	}

	if (State.hfont)
	{
		DeleteFont(State.hfont);
		State.hfont = nullptr;
	}

	// TO-DO: check if this is really necessary, or unecessary overkill
	if (d->m_hWnd)
		SetForegroundWindow(d->m_hWnd);

	return;
} // openFile

  /*
  ** remove anything that includes and comes after the first occurence of c in the string fullpath
  */
void rem_tail(std::basic_string<TCHAR> &sStr, char c)
{
	/*
	** Remove extension if present.
	*/
	const size_t char_idx = sStr.find(c);
	if (std::basic_string<TCHAR>::npos != char_idx)
	{
		sStr.erase(char_idx);
	}

	return;
} // rem_tail

void cui_raw::saveFile(
	const opensavefileParams &params,
	const std::basic_string<TCHAR> &sFile,
	std::basic_string<TCHAR> &sFullPath
)
{
	/*
	** declare stateinfo struct and (optionally) initialize some of the members
	*/
	stateinfo_ofn State;
	State.d = d;
	State.bHideNavigationToolbar = params.bHideNavigationToolbar;

	// TO-DO: use smart pointers
	State.m_pResizer = new CResizer();
	State.m_pShadow = new CShadow;

	if (!State.bHideNavigationToolbar)
		State.clrUIbk = (COLORREF)GetSysColor(COLOR_BTNFACE);	// because toolbar looks ugly against white ... we still haven't found a way of manipulating it's color
	else
		State.clrUIbk = d->m_clrBackground;

	CBrush hbr(State.clrUIbk);
	State.m_UIbckgndBrush = hbr.get();

	/*
	If we indicate a NULL termminator with a new line character,
	The filter string format is as follows :

	Bitmap Files(*.bmp)
	*.BMP
	JPEG(*.jpg; *.jpeg)
	*.JPG; *.JPEG
	PNG(*.png)
	*.PNG
	All supported images
	*.JPG; *.JPEG; *.PNG; *.BMP
	*/
	std::basic_string<TCHAR> sFilter;
	std::vector<std::basic_string<TCHAR>> vFileTypesIndexed;

	// set filter
	{
		// create NULL terminator
		std::basic_string<TCHAR> sNULL;
		{
			std::string m_sNULL({ '\0' });
			sNULL = std::basic_string<TCHAR>(m_sNULL.begin(), m_sNULL.end());
		}

		std::vector<cui_raw::fileType> m_types;

		for (size_t i = 0; i < params.vFileTypes.size(); i++)
		{
			// check if type description hasn't already been captured; if it has, append it's extension to the other
			bool bNew = true;

			for (size_t x = 0; x < m_types.size(); x++)
			{
				if (m_types[x].sDescription == params.vFileTypes[i].sDescription)
				{
					m_types[x].sFileExtension += _T(";*.") + params.vFileTypes[i].sFileExtension;

					bNew = false;
					break;
				}
			}

			if (bNew)
			{
				cui_raw::fileType type;
				type.sDescription = params.vFileTypes[i].sDescription;
				type.sFileExtension = _T("*.") + params.vFileTypes[i].sFileExtension;
				m_types.push_back(type);
			}
		}

		std::basic_string<TCHAR> m_sAllFileExtensions;

		for (size_t i = 0; i < m_types.size(); i++)
		{
			std::basic_string<TCHAR> sDescription = m_types[i].sDescription;
			std::basic_string<TCHAR> sFileExtension = m_types[i].sFileExtension;

			if (m_sAllFileExtensions.empty())
				m_sAllFileExtensions = sFileExtension;
			else
				m_sAllFileExtensions += _T(";") + sFileExtension;

			std::basic_string<TCHAR> m_sFilter;
			m_sFilter = sDescription + _T(" (") + sFileExtension + _T(")") + sNULL;
			m_sFilter += sFileExtension + sNULL;

			sFilter += m_sFilter;
			vFileTypesIndexed.push_back(m_types[i].sFileExtension);
		}

		// add all supported files
		if (params.bIncludeAllSupportedTypes)
		{
			std::basic_string<TCHAR> m_sFilter;
			std::basic_string<TCHAR> sDescription = _T("All supported files");
			std::basic_string<TCHAR> sFileExtension = m_sAllFileExtensions;
			m_sFilter = sDescription + _T(" (") + sFileExtension + _T(")") + sNULL;
			m_sFilter += sFileExtension + sNULL;

			sFilter += m_sFilter;
			vFileTypesIndexed.push_back(_T(""));
		}
	}

	OPENFILENAME ofn;
	TCHAR _FilePath[MAX_PATH];
	if (!sFile.empty())
		lstrcpyn(_FilePath, sFile.c_str(), _countof(_FilePath));

	State.IDP_PNGIconSmall = d->m_IDP_ICONSMALL;
	State.clrUItxt = RGB(0, 0, 0);			// TO-DO: fix this in cui_raw
	State.clrUItheme = d->m_clrTheme;
	State.clrUIthemeDarker = d->m_clrThemeDarker;
	State.sFontName = params.sFontName;
	State.iFontSize = params.iFontSize;
	State.sTitle = params.sTitle;
	State.bLarge = !params.bSmallWindow;

	/*
	** initialize OPENFILENAME
	*/
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);

	if (d->m_hWnd)
		ofn.hwndOwner = d->m_hWnd;
	else
		ofn.hwndOwner = NULL;

	ofn.lpstrFile = _FilePath;

	/*
	** set FilePath[0] to '\0' so that GetOpenFileName doesn't use the contents FilePath to initialize itself
	*/
	if (sFile.empty())
		ofn.lpstrFile[0] = '\0';

	ofn.nMaxFile = MAX_PATH;	// TO-DO: shouldn't this be sizeof(ofn.lpstrFile) / sizeof(TCHAR) ???
	ofn.lpstrFilter = sFilter.c_str();
	ofn.nFilterIndex = 1;
	ofn.nMaxFileTitle = NULL;
	ofn.lpstrInitialDir = NULL;
	ofn.hInstance = GetModuleHandle(NULL);
	ofn.lpstrTitle = params.sTitle.c_str();

	ofn.lpfnHook = customizationcallback;	// function to call for customization
	ofn.lCustData = (LPARAM)&State;			// parameter to pass to the customization function

											/*
											** TO-DO: solve issue with window size when OFN_HIDEREADONLY is not specified
											*/
	ofn.Flags = OFN_ENABLEHOOK | OFN_EXPLORER | OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_NOTESTFILECREATE;

	if (State.bEnableResize)
	{
		ofn.Flags |= OFN_ENABLESIZING;
	}

	/*
	** Display the Open dialog Box
	*/
	if (GetSaveFileName(&ofn) != TRUE)
		sFullPath.clear();
	else
	{
		// capture full path
		sFullPath = _FilePath;

		// remove multiple file extensions
		for (size_t i = 0; i < vFileTypesIndexed.size(); i++)
		{
			rem_tail(vFileTypesIndexed[i], ';');
		}

		// determine selected file type
		std::basic_string<TCHAR> sExtension(_T(""));

		/*
		** nFilterIndex
		** Type: DWORD
		** The index of the currently selected filter in the File Types control. The buffer pointed to by lpstrFilter
		** contains pairs of strings that define the filters. The first pair of strings has an index value of 1, the
		** second pair 2, and so on. An index of zero indicates the custom filter specified by	lpstrCustomFilter.
		** You can specify an index on input to indicate the initial filter description and filter pattern for the
		** dialog box. When the user selects a file, nFilterIndex returns the index of the currently displayed filter.
		** If nFilterIndex is zero and lpstrCustomFilter is NULL, the system uses the first filter in the lpstrFilter buffer.
		** If all three members are zero or NULL, the system does not use any filters and does not show any files in
		** the file list control of the dialog box.
		** https://msdn.microsoft.com/en-us/library/windows/desktop/ms646839(v=vs.85).aspx
		*/
		if (vFileTypesIndexed.size() > 0 && vFileTypesIndexed.size() >= ofn.nFilterIndex - 1)
			sExtension = vFileTypesIndexed[ofn.nFilterIndex - 1];

		// add the selected extension to the full path (if it exists)
		if (!sExtension.empty())
		{
			// format extension properly
			const size_t period_idx = sExtension.rfind('.');

			if (std::basic_string<TCHAR>::npos != period_idx)
				sExtension.erase(0, period_idx);

			// add the extension if it's not there
			if (sFullPath.find(sExtension) == std::basic_string<TCHAR>::npos)
				sFullPath += sExtension;
			else
			{
				// user probably selected an existing file or typed it in with an extension
			}
		}
	}

	// cleanup
	if (State.m_pShadow)
	{
		delete State.m_pShadow;
		State.m_pShadow = NULL;
	}

	if (State.m_pResizer)
	{
		delete State.m_pResizer;
		State.m_pResizer = NULL;
	}

	if (State.hfont)
	{
		DeleteFont(State.hfont);
		State.hfont = nullptr;
	}

	// TO-DO: check if this is really necessary, or unecessary overkill
	if (d->m_hWnd)
		SetForegroundWindow(d->m_hWnd);

	return;
} // saveFile
