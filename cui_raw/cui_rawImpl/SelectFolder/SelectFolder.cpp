//
// SelectFolder.cpp - select folder implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#include <Windows.h>
#include <CommCtrl.h>
#include <ShellAPI.h>
#include <ShlObj.h>						// for SHGetFolderPath
#pragma comment (lib, "Shell32.lib")

#include "../../cui_raw.h"
#include "../DrawRoundRect/DrawRoundRect.h"
#include "../cui_rawImpl.h"
#include "../../scaleAdjust/scaleAdjust.h"
#include "../../XCreateFont/XCreateFont.h"

enum enumCustomMessage
{
	WM_STARTED = 11119,	// message to send to pass the stateinfo struct
};

struct stateinfo_sf
{
	CResizer *m_pResizer = NULL;
	CShadow *m_pShadow = NULL;
	COLORREF clrUIbk;
	COLORREF clrUItheme;
	COLORREF clrUIthemeDarker;
	COLORREF clrUItxt;

	int m_iBoarderWidth = 1;

	int m_iMinWidth = 100;
	int m_iMinHeight = 100;
	int m_iMinWidthCalc = 100;
	int m_iMinHeightCalc = 100;

	std::basic_string<TCHAR> sTitle;

	bool bEnableResize = true;

	HBRUSH m_UIbckgndBrush = NULL;

	int IDP_PNGIconSmall = 0;

	cui_rawImpl::RectControl leftBorder, topBorder, rightBorder, bottomBorder;

	cui_rawImpl::ImageControl captionIconControl;
	cui_rawImpl::TextControl titleTextControl;

	std::basic_string<TCHAR> sFontName;
	double iFontSize;

	bool bSmallWindow = false;

	cui_rawImpl* d = NULL;

	HFONT hfont = nullptr;
};	// state info struct

	//IDs of the controls in the shell dialog
static const int FOLDER_TREE = 0x3741;
static const int STATIC1 = 0x3742;
static const int STATIC2 = 0x3743;
static const int ID_OK = 0x1;
static const int ID_CANCEL = 0x2;

/*
** change layout of dialog
*/
static void ChangeLayout(HWND hWnd, stateinfo_sf* pThis)
{
	// change fonts
	HDC hdc = GetDC(hWnd);

	pThis->hfont = XCreateFont(hdc, pThis->sFontName, pThis->iFontSize);
	SetWindowFont(GetDlgItem(hWnd, STATIC1), pThis->hfont, TRUE);
	SetWindowFont(GetDlgItem(hWnd, FOLDER_TREE), pThis->hfont, TRUE);

	ReleaseDC(hWnd, hdc);

	/*
	** change window size
	*/
	RECT rect;
	::GetClientRect(hWnd, &rect);

	// capture min width and height
	pThis->m_iMinWidth = rect.right - rect.left;
	pThis->m_iMinHeight = rect.bottom - rect.top;

	if (!pThis->bSmallWindow)
	{
		rect.right = rect.left + 780;
		rect.bottom = rect.top + 550;
	}

	/*
	** center window to parent
	*/
	RECT mainRect;
	GetWindowRect(::GetParent(hWnd), &mainRect);
	int x = (mainRect.right + mainRect.left) / 2 - (rect.right - rect.left) / 2;
	int y = (mainRect.bottom + mainRect.top) / 2 - (rect.bottom - rect.top) / 2;
	::MoveWindow(hWnd, x, y, (rect.right - rect.left), (rect.bottom - rect.top), TRUE);

	//calculate y-position offset
	int themetitlebar_ = pThis->d->m_iTitlebarHeight;
	int themeboarder_ = pThis->d->m_iBoarderWidth;

	//constants
	const int DLG_WIDTH = rect.right - rect.left;
	const int DLG_HEIGHT = rect.bottom - rect.top;

	const int BUTTON_H = int(0.5 + 25 * pThis->d->m_DPIScale);
	const int BUTTON_W = int(0.5 + 90 * pThis->d->m_DPIScale);

	const int MARGIN = int(0.5 + 10 * pThis->d->m_DPIScale);

	//extend the window first
	::SetWindowPos(hWnd, NULL, 0, 0, DLG_WIDTH, DLG_HEIGHT, SWP_NOZORDER | SWP_NOMOVE);

	//memorize the coordinates of the "grid" in which we place the controls
	int LEFT = rect.left + MARGIN;
	int RIGHT = rect.right - MARGIN;
	int TOP = rect.top + themetitlebar_ + MARGIN;
	int BOTTOM = rect.bottom - MARGIN;
	int MIDDLE = 250;
	int TOT_WIDTH = RIGHT - LEFT;
	int TOT_HEIGHT = BOTTOM - TOP;

	RECT rect_stat1, rect_stat2;
	GetWindowRect(GetDlgItem(hWnd, STATIC1), &rect_stat1);
	GetWindowRect(GetDlgItem(hWnd, STATIC2), &rect_stat2);

	const int static1_h = rect_stat1.bottom - rect_stat1.top;
	const int static2_h = rect_stat2.bottom - rect_stat2.top;
	const int STATIC1_H = static1_h;
	const int STATIC2_H = static2_h;

	cui_raw::onResize resize;

	// static 1
	HWND h_ctrl = GetDlgItem(hWnd, STATIC1);
	if (h_ctrl != NULL)
	{
		::SetWindowPos(h_ctrl, NULL,
			LEFT, TOP, TOT_WIDTH, STATIC1_H, SWP_NOZORDER);

		resize.iPercCY = 0;
		resize.iPercCX = 100;
		resize.iPercH = 0;
		resize.iPercV = 0;
		pThis->m_pResizer->OnResize(h_ctrl, resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	// folder tree
	h_ctrl = GetDlgItem(hWnd, FOLDER_TREE);
	if (h_ctrl != NULL)
	{
		LONG style = ::GetWindowLong(h_ctrl, GWL_STYLE);
		style |= TVS_SHOWSELALWAYS;
		::SetWindowLong(h_ctrl, GWL_STYLE, style);

		::SetWindowPos(h_ctrl, NULL,
			LEFT, TOP + STATIC1_H + MARGIN / 2,
			RIGHT - MARGIN, (TOT_HEIGHT - (STATIC1_H + MARGIN / 2) - (BUTTON_H + MARGIN)), SWP_NOZORDER);

		resize.iPercCY = 100;
		resize.iPercCX = 100;
		resize.iPercH = 0;
		resize.iPercV = 0;
		pThis->m_pResizer->OnResize(h_ctrl, resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	// static 2
	h_ctrl = GetDlgItem(hWnd, STATIC2);
	if (h_ctrl != NULL)
	{
		::SetWindowPos(h_ctrl, NULL,
			MIDDLE + MARGIN, TOP + STATIC1_H + MARGIN / 2,
			RIGHT - MIDDLE - MARGIN, STATIC2_H, SWP_NOZORDER);

		resize.iPercCY = 0;
		resize.iPercCX = 100;
		resize.iPercH = 0;
		resize.iPercV = 0;
		pThis->m_pResizer->OnResize(h_ctrl, resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	//OK and Cancel buttons

	// id_ok
	h_ctrl = GetDlgItem(hWnd, ID_OK);
	if (h_ctrl != NULL)
	{
		::SetWindowPos(h_ctrl, NULL,
			RIGHT - 2 * BUTTON_W - MARGIN, BOTTOM - BUTTON_H,
			BUTTON_W, BUTTON_H, SWP_NOZORDER);

		resize.iPercCY = 0;
		resize.iPercCX = 0;
		resize.iPercH = 100;
		resize.iPercV = 100;
		pThis->m_pResizer->OnResize(h_ctrl, resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}

	// id_cancel
	h_ctrl = GetDlgItem(hWnd, ID_CANCEL);
	if (h_ctrl != NULL)
	{
		::SetWindowPos(h_ctrl, NULL,
			RIGHT - BUTTON_W, BOTTOM - BUTTON_H,
			BUTTON_W, BUTTON_H, SWP_NOZORDER);

		resize.iPercCY = 0;
		resize.iPercCX = 0;
		resize.iPercH = 100;
		resize.iPercV = 100;
		pThis->m_pResizer->OnResize(h_ctrl, resize.iPercH, resize.iPercV, resize.iPercCX, resize.iPercCY);
	}
} // ChangeLayout

static bool CreateShadow(
	HWND hWnd,
	stateinfo_sf *pThis
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

static bool design = false;

  /*
  ** subclass open file name dialog box
  */
static WNDPROC oldProc;
static INT_PTR CALLBACK subProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	stateinfo_sf* pState = NULL; if ((msg == WM_COMMAND) && (LOWORD(wParam) == (DWORD)WM_STARTED)) { pState = (stateinfo_sf*)lParam; SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pState); }
	else { pState = reinterpret_cast<stateinfo_sf*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)); }

	switch (msg)
	{
	case WM_NCHITTEST:
	{
		int m_iTitlebarHeight = pState->d->m_iTitlebarHeight;

		RECT WindowRect;
		int x, y;

		GetWindowRect(hWnd, &WindowRect);
		x = GET_X_LPARAM(lParam) - WindowRect.left;
		y = GET_Y_LPARAM(lParam) - WindowRect.top;

		int m_iBoarderWidth = pState->m_iBoarderWidth;
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

	case WM_GETMINMAXINFO:
	{
		/*
		** set lower limits to window size
		*/
		MINMAXINFO *pMinMaxInfo = (MINMAXINFO*)lParam;
		if (pState->m_iMinWidth != -1)
		{
			// do not allow to be less than caption and min/max/close buttons
			pMinMaxInfo->ptMinTrackSize.x = pState->m_iMinWidth > pState->m_iMinWidthCalc ? pState->m_iMinWidth : pState->m_iMinWidthCalc;
		}
		if (pState->m_iMinHeight != -1)
		{
			// do not allow to be less than title bar height
			pMinMaxInfo->ptMinTrackSize.y = pState->m_iMinHeight > pState->d->m_iTitlebarHeight + 2 * pState->m_iBoarderWidth ? pState->m_iMinHeight : pState->d->m_iTitlebarHeight + 2 * pState->m_iBoarderWidth;
		}

		/*
		** set upper limits to main window size and position (with consideration of multi-monitor setup)
		** essential for maximizing WS_POPUP window
		** code acquired from http://social.msdn.microsoft.com/Forums/vstudio/en-US/fb4de52d-66d4-44da-907c-0357d6ba894c/swmaximize-is-same-as-fullscreen?forum=vcgeneral
		** TO-DO: post a correction on the above mentioned site for the last line. It should be:
		**
		** minmaxinfo->ptMaxPosition.y = monitorinfo.rcWork.top - monitorinfo.rcMonitor.top;
		**
		** NOT
		**
		** minmaxinfo->ptMaxPosition.y = monitorinfo.rcWork.bottom - monitorinfo.rcMonitor.bottom;
		*/
		HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

		MONITORINFO monitorinfo = { 0 };
		monitorinfo.cbSize = sizeof(MONITORINFO);

		GetMonitorInfo(hMonitor, &monitorinfo);

		pMinMaxInfo->ptMaxSize.x = monitorinfo.rcWork.right - monitorinfo.rcWork.left;
		pMinMaxInfo->ptMaxSize.y = monitorinfo.rcWork.bottom - monitorinfo.rcWork.top;
		pMinMaxInfo->ptMaxPosition.x = monitorinfo.rcWork.left - monitorinfo.rcMonitor.left;
		pMinMaxInfo->ptMaxPosition.y = monitorinfo.rcWork.top - monitorinfo.rcMonitor.top;
	}
	break;

	case WM_SIZE:
	{
		// TO-DO: find out why we have to do this to get the static control repainted ... we didn't have to in the Old UIEng.dll for Grades 1.0!!!!!
		InvalidateRect(GetDlgItem(hWnd, STATIC1), NULL, TRUE);
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

	case WM_DRAWITEM:
	{
		// draw the owner draw button
		LPDRAWITEMSTRUCT lpDIS = (LPDRAWITEMSTRUCT)lParam;

		int iIDFrom = lpDIS->CtlID;

		// check whether this is one of our custom buttons
		bool bButton = false;

		if (iIDFrom == ID_OK || iIDFrom == ID_CANCEL)
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
				DrawRoundRect(graphics, outerRect.left, outerRect.top, outerRect.right, outerRect.bottom, int(0.5 + 3 * pState->d->m_DPIScale), clrBoarder, clrBoarder, 1, true);
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
			DrawRoundRect(graphics, itemRect.left, itemRect.top, itemRect.right, itemRect.bottom, int(0.5 + 3 * pState->d->m_DPIScale), clrBackground, clrBackground, 1, true);

			// Read the button's title
			TCHAR sTitle[100];
			GetWindowText(lpDIS->hwndItem, sTitle, 100);

			RECT captionRect = lpDIS->rcItem;

			BOOL bHasTitle = (sTitle[0] != '\0');

			// Write the button title (if any)
			if (bHasTitle)
			{
				Gdiplus::FontFamily ffm(pState->sFontName.c_str());
				Gdiplus::Font* p_font = new Gdiplus::Font(&ffm,
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

static void addCaptionIcon(HWND hWnd, stateinfo_sf* pThis, int &iCaptionIconWidth)
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
		pThis->captionIconControl.hWnd = CreateWindow(
			WC_STATIC,					// Predefined class
			_T(""),						// text 
			dwStyle,					// Styles 
			x, y, cx, cy,				// control coordinates
			hWnd,						// Parent window
			(HMENU)(INT_PTR)pThis->captionIconControl.iUniqueID,	// button ID
			(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
			NULL);      // Pointer not needed.

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

static void addWindowTitle(HWND hWnd, stateinfo_sf* pThis, int iCaptionIconWidth)
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

		Gdiplus::FontFamily ffm(font_name.c_str());
		Gdiplus::Font* p_font = new Gdiplus::Font(&ffm,
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

	// add static control
	pThis->titleTextControl.hWnd = CreateWindow(
		WC_STATIC,					// Predefined class
		_T(""),						// text
		dwStyle,					// Styles 
		pThis->titleTextControl.coords.left,		// x position 
		pThis->titleTextControl.coords.top,		// y position 
		pThis->titleTextControl.coords.width(),	// Button width
		pThis->titleTextControl.coords.height(),	// Button height
		hWnd,						// Parent window
		(HMENU)(INT_PTR)pThis->titleTextControl.iUniqueID,	// button ID
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL);      // Pointer not needed.

					// subclass static control so we can do custom drawing
	SetWindowLongPtr(pThis->titleTextControl.hWnd, GWLP_USERDATA, (LONG_PTR)&pThis->titleTextControl);
	pThis->titleTextControl.PrevProc = SetWindowLongPtr(pThis->titleTextControl.hWnd, GWLP_WNDPROC, (LONG_PTR)cui_rawImpl::TextControlProc);

	// set resizing behaviour
	pThis->m_pResizer->OnResize(pThis->titleTextControl.hWnd, pThis->titleTextControl.resize.iPercH, pThis->titleTextControl.resize.iPercV, pThis->titleTextControl.resize.iPercCX, pThis->titleTextControl.resize.iPercCY);
} // addWindowTitle

static void addRect(HWND hWnd, int iUniqueID, cui_rawImpl::RectControl* pControl, stateinfo_sf* pThis)
{
	DWORD dwStyle = WS_VISIBLE | WS_CHILD;

	// add button
	pControl->hWnd = CreateWindow(
		WC_STATIC,					// Predefined class
		_T(""),						// Button text 
		dwStyle,					// Styles 
		pControl->coords.left,		// x position 
		pControl->coords.top,		// y position 
		pControl->coords.width(),	// rect width
		pControl->coords.height(),	// rect height
		hWnd,						// Parent window
		(HMENU)(INT_PTR)pControl->iUniqueID,	// button ID
		(HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE),
		NULL);      // Pointer not needed.

					// subclass static control so we can do custom drawing
	SetWindowLongPtr(pControl->hWnd, GWLP_USERDATA, (LONG_PTR)pControl);
	pControl->PrevProc = SetWindowLongPtr(pControl->hWnd, GWLP_WNDPROC, (LONG_PTR)cui_rawImpl::RectProc);

	// set resizing behaviour
	pThis->m_pResizer->OnResize(pControl->hWnd, pControl->resize.iPercH, pControl->resize.iPercV, pControl->resize.iPercCX, pControl->resize.iPercCY);
} // addRect

static void addBorders(HWND hWnd, stateinfo_sf *pThis)
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


  /*
  ** customization callback function
  */
static int CALLBACK customizationcallback(HWND hWnd, UINT msg, LPARAM lParam, LPARAM lpData)
{
	stateinfo_sf* pState = NULL; if (msg == BFFM_INITIALIZED) { pState = (stateinfo_sf*)lpData; SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pState); }
	else { pState = reinterpret_cast<stateinfo_sf*>(GetWindowLongPtr(hWnd, GWLP_USERDATA)); }

	switch (msg)
	{
	case BFFM_INITIALIZED:
	{
		SetWindowLongPtr(hWnd, GWL_STYLE, WS_POPUP);	// set window style to ONLY WS_POPUP
		SetWindowLongPtr(hWnd, GWL_EXSTYLE, 0);			// remove all extended window styles

														// get window style of buttons and make them custom drawn
		DWORD dwStyle = (DWORD)GetWindowLong(GetDlgItem(hWnd, ID_OK), GWL_STYLE);
		dwStyle |= BS_OWNERDRAW;
		SetWindowLong(GetDlgItem(hWnd, ID_OK), GWL_STYLE, (LONG)dwStyle);

		dwStyle = (DWORD)GetWindowLong(GetDlgItem(hWnd, ID_CANCEL), GWL_STYLE);
		dwStyle |= BS_OWNERDRAW;
		SetWindowLong(GetDlgItem(hWnd, ID_CANCEL), GWL_STYLE, (LONG)dwStyle);

		/*
		** subclass dialog
		*/
		oldProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)subProc);

		// change layout of dialog
		ChangeLayout(hWnd, pState);

		// add caption icon
		int iCaptionIconWidth = 0;
		addCaptionIcon(hWnd, pState, iCaptionIconWidth);

		// add title
		addWindowTitle(hWnd, pState, iCaptionIconWidth);

		// add borders
		addBorders(hWnd, pState);

		// enable the resizer
		pState->m_pResizer->enable(hWnd);

		if (pState->d->m_bShadow)
		{
			// add shadow to window
			CreateShadow(hWnd, pState);
		}
	}
	break;

	default:
		break;
	}
	return (int)FALSE;
} // customizationcallback

  /*
  ** Browse folder and return the path to the selected one
  */
void cui_raw::selectFolder(
	const selectFolderParams &params,
	std::basic_string<TCHAR> &sPath
)
{
	sPath.clear();

	std::basic_string<TCHAR> titlestr = params.sMessage;

	/*
	** declare stateinfo struct and (optionally) initialize some of the members
	*/
	stateinfo_sf State;
	State.bSmallWindow = params.bSmallWindow;
	State.IDP_PNGIconSmall = d->m_IDP_ICONSMALL;
	State.clrUItxt = RGB(0, 0, 0);		// TO-DO: fix this in cui_raw
	State.clrUIbk = d->m_clrBackground;
	State.clrUItheme = d->m_clrTheme;
	State.clrUIthemeDarker = d->m_clrThemeDarker;
	State.sFontName = params.sFontName;
	State.iFontSize = params.iFontSize;
	State.sTitle = params.sTitle;
	State.d = d;

	CBrush hbr(State.clrUIbk);
	State.m_UIbckgndBrush = hbr.get();

	// TO-DO: use smart pointers
	State.m_pResizer = new CResizer;
	State.m_pShadow = new CShadow;

	TCHAR szDir[MAX_PATH];
	BROWSEINFO bInfo = { 0 };

	if (d->m_hWnd)
		bInfo.hwndOwner = d->m_hWnd;
	else
		bInfo.hwndOwner = NULL;

	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szDir;		// Address of a buffer to receive the display name of the folder selected by the user
	bInfo.lpszTitle = titlestr.c_str();	// Title of the dialog
	bInfo.ulFlags = 0;
	bInfo.lpfn = customizationcallback;	// function to call for customization
	bInfo.lParam = (LPARAM)&State;		// parameter to pass to the customization function
	bInfo.iImage = -1;

	LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);

	if (lpItem != NULL)
	{
		SHGetPathFromIDList(lpItem, szDir);
		sPath = szDir;
	}

	// clean-up
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
} // selectFolder
