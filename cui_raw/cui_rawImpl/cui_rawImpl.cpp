/*
** cui_rawImpl.cpp - cui_rawImpl implementation
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

#include "cui_rawImpl.h"
#include "Combobox/Combobox.h"
#include "TooltipControl/TooltipControl.h"
#include "../scaleAdjust/scaleAdjust.h"
#include "../HlpFxs/HlpFxs.h"

#ifdef _UNICODE
#define to_tstring	std::to_wstring
#else
#define to_tstring	std::to_string
#endif // _UNICODE

cui_rawImpl::cui_rawImpl(const std::wstring& title)
{
	if (title != L"cui default window" && !IsProcessDPIAware()) {
		m_dpi_awareness_set_programmatically = true;
		if (!SetProcessDPIAware())
			MessageBox(nullptr,
				L"This program is not DPI aware. As a result, UI elements may not be clear.",
				L"liblec::cui::gui_raw::cui_raw", MB_ICONWARNING);
	}
	else
		m_dpi_awareness_set_programmatically = false;

	m_hWnd = NULL;
	m_hWndParent = NULL;
	m_pcui_rawparent = NULL;
	parent_was_enabled = false;
	m_hResModule = NULL;
	m_ix = CW_USEDEFAULT;
	m_iy = CW_USEDEFAULT;
	m_icx = CW_USEDEFAULT;
	m_icy = CW_USEDEFAULT;

	m_clrBackground = RGB(255, 255, 255);
	m_hbrBackground = NULL;
	m_clrTheme = RGB(0, 0, 0);
	m_clrThemeHot = RGB(255, 180, 0);

	wcex = { 0 };

	m_bCreated = false;
	m_bEnableResize = false;
	m_iBoarderWidth = 1;
	m_bMinbtn = false;
	m_bMaxbtn = false;
	m_bClosebtn = false;
	m_pShadow = new CShadow;
	m_bCentershadow = true;
	m_bShadow = true;
	m_iMinWidth = 0;
	m_iMinHeight = 0;
	m_pMouseTrack = new CMouseTrack();
	m_bMaximized = false;
	m_pResizer = new CResizer();

	m_iIDClose = 0;
	m_iIDMax = 0;
	m_iIDMin = 0;

	m_IDI_ICON = 0;
	m_IDI_ICONSMALL = 0;
	m_iStopQuit = 0;

	m_hNormalCursor = LoadCursor(NULL, IDC_ARROW);
	m_hHotCursor = LoadCursor(NULL, IDC_HAND);

	// TO-DO: use enumerations for predefined constansts like these ones to avoid conflicts in the future, and to promote clearer coding
	m_iLeftBorder = -300;
	m_iTopBorder = -301;
	m_iRightBorder = -302;
	m_iBottomBorder = -304;

	pState_user = NULL;

	m_sTooltipFont = _T("Tahoma");
	m_iTooltipFontSize = 8;
	m_bLButtonDown = false;
	m_bRButtonDown = false;

	m_iShutdownID = -1;

	// capture current DPI scale
	HDC hdcScreen = GetDC(NULL);
	m_DPIScale = (double)GetDeviceCaps(hdcScreen, LOGPIXELSY) / (double)96;
	ReleaseDC(NULL, hdcScreen);

	m_iTitlebarHeight = 30;
	m_iMinWidthCalc = 30;

	// scale for DPI
	m_iTooltipFontSize = m_dpi_awareness_set_programmatically ? m_iTooltipFontSize * m_DPIScale : m_iTooltipFontSize;
	m_iTitlebarHeight = int(0.5 + m_iTitlebarHeight * m_DPIScale);
	m_iMinWidthCalc = int(0.5 + m_iMinWidthCalc * m_DPIScale);

	m_iTimer = 0;
	m_bStartOnMouseMove = true;
	m_bStopOnMouseOverWindow = false;
	m_bTimerRunning = false;

	m_ptStartCheck = { 0 };

	m_bNotification = false;

	m_clrDisabled = RGB(235, 235, 235);	// TO-DO: find a way to do this programmatically through the cui_raw interface

	m_iMessageBoxes = 0;

	m_bParentClosing = false;

	uID = 1;	// tray icon id
	m_iRegID = 0;	// this instance's registration ID
	m_iCopyDataID = 0;	// the id to be called when data is received from another instance
	m_iDropFilesID = 0;
	m_iWidth = 0;
	m_iHeight = 0;
	bFirstRun = true;
	m_vPreventQuitList.clear();
	hRichEdit = NULL;
	iAddToWM_APP = 1;
	vFonts.clear();
}

cui_rawImpl::~cui_rawImpl()
{
	// cleanup shadow objects attached to control button tooltips
	for (auto &m_it : m_ControlBtns)
	{
		if (m_it.second.toolTip.m_pShadow)
		{
			delete m_it.second.toolTip.m_pShadow;
			m_it.second.toolTip.m_pShadow = NULL;
		}
	}

	for (auto &it : m_Pages)
	{
		// cleanup shadow objects attached to image control tooltips
		for (auto &m_it : it.second.m_ImageControls)
		{
			if (m_it.second.toolTip.m_pShadow)
			{
				delete m_it.second.toolTip.m_pShadow;
				m_it.second.toolTip.m_pShadow = NULL;
			}
		}

		// cleanup shadow objects attached to toggle button tooltips
		for (auto &m_it : it.second.m_ToggleButtonControls)
		{
			if (m_it.second.toolTip.m_pShadow)
			{
				delete m_it.second.toolTip.m_pShadow;
				m_it.second.toolTip.m_pShadow = NULL;
			}
		}

		// cleanup shadow objects attached to star rating control tooltips
		for (auto &m_it : it.second.m_StarRatingControls)
		{
			if (m_it.second.toolTip.m_pShadow)
			{
				delete m_it.second.toolTip.m_pShadow;
				m_it.second.toolTip.m_pShadow = NULL;
			}
		}

		// cleanup shadow objects attached to tab controls
		if (m_Pages.at(it.first).m_TabControl.iUniqueID != 123)	// TO-DO: remove magic number
		{
			if (m_Pages.at(it.first).m_TabControl.toolTip.m_pShadow)
			{
				delete m_Pages.at(it.first).m_TabControl.toolTip.m_pShadow;
				m_Pages.at(it.first).m_TabControl.toolTip.m_pShadow = NULL;
			}
		}

		// cleanup shadow objects attached to button controls
		for (auto &m_it : it.second.m_ButtonControls)
		{
			if (m_it.second.toolTip.m_pShadow)
			{
				delete m_it.second.toolTip.m_pShadow;
				m_it.second.toolTip.m_pShadow = NULL;
			}
		}

		// cleanup shadow objects attached to selector controls
		for (auto &m_it : it.second.m_SelectorControls)
		{
			if (m_it.second.toolTip.m_pShadow)
			{
				delete m_it.second.toolTip.m_pShadow;
				m_it.second.toolTip.m_pShadow = NULL;
			}
		}

		// cleanup shadow objects attached to barchart controls
		for (auto &m_it : it.second.m_BarChartControls)
		{
			if (m_it.second.toolTip.m_pShadow)
			{
				delete m_it.second.toolTip.m_pShadow;
				m_it.second.toolTip.m_pShadow = NULL;
			}
		}

		// cleanup shadow objects attached to line chart controls
		for (auto &m_it : it.second.m_LineChartControls)
		{
			if (m_it.second.toolTip.m_pShadow)
			{
				delete m_it.second.toolTip.m_pShadow;
				m_it.second.toolTip.m_pShadow = NULL;
			}
		}

		// cleanup shadow objects attached to piechart controls
		for (auto &m_it : it.second.m_PieChartControls)
		{
			if (m_it.second.toolTip.m_pShadow)
			{
				delete m_it.second.toolTip.m_pShadow;
				m_it.second.toolTip.m_pShadow = NULL;
			}

			// cleanup region objects attached to pie chart control
			for (auto &x_it : m_it.second.chartBarsInfo)
			{
				if (x_it.pRegion)
				{
					delete x_it.pRegion;
					x_it.pRegion = NULL;
				}
			}
		}

		// cleanup shadow objects attached to text controls
		for (auto &m_it : it.second.m_TextControls)
		{
			if (m_it.second.toolTip.m_pShadow)
			{
				delete m_it.second.toolTip.m_pShadow;
				m_it.second.toolTip.m_pShadow = NULL;
			}
		}

		// cleanup font objects attached to font list comboboxes controls
		//for (auto font : vFonts)
		//	DeleteFont(font.second);
	}

	if (m_pShadow)
	{
		delete m_pShadow;
		m_pShadow = NULL;
	}

	if (m_pMouseTrack)
	{
		delete m_pMouseTrack;
		m_pMouseTrack = NULL;
	}

	// delete dynamic objects in all pages
	for (auto &m_it : m_Pages)
	{
		// delete dynamic objects in current page

		for (auto &it : m_it.second.m_ComboBoxControls)
		{
			if (it.second.hfont)
			{
				DeleteFont(it.second.hfont);
				it.second.hfont = NULL;
			}
		}

		for (auto &it : m_it.second.m_listviewControls)
		{
			if (it.second.hfont)
			{
				DeleteFont(it.second.hfont);
				it.second.hfont = NULL;
			}

			if (it.second.pClistview)
			{
				delete it.second.pClistview;
				it.second.pClistview = NULL;
			}
		}

		for (auto &it : m_it.second.m_EditControls)
		{
			if (it.second.hfont)
			{
				DeleteFont(it.second.hfont);
				it.second.hfont = NULL;
			}
		}

		for (auto &it : m_it.second.m_DateControls)
		{
			if (it.second.hfont)
			{
				DeleteFont(it.second.hfont);
				it.second.hfont = NULL;
			}
		}
	}

	if (m_pResizer)
	{
		delete m_pResizer;
		m_pResizer = NULL;
	}

	if (m_hbrBackground)
	{
		DeleteBrush(m_hbrBackground);
		m_hbrBackground = NULL;
	}
}

void cui_rawImpl::GetWorkingArea(
	HWND hWnd,
	RECT &rectout
)
{
	HMONITOR hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);

	MONITORINFO monitorinfo = { 0 };
	monitorinfo.cbSize = sizeof(MONITORINFO);

	GetMonitorInfo(hMonitor, &monitorinfo);

	rectout = monitorinfo.rcWork;
} // GetWorkingArea

  /*
  ** custom function for adding a scaled rect
  ** useful in placing precision lines in a DPI scaled-window
  */
void addRectScaled(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID, COLORREF clr,
	RECT rc, cui_raw::onResize resize, cui_rawImpl* d)
{
	if (!d)
		return;

	cui_rawImpl::RectControl control;
	control.iUniqueID = iUniqueID;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.resize = resize;
	control.clr = clr;

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
	{
		sPageLessKey = d->m_sTitle;
		control.bPageLess = true;
	}
	else
		control.bPageLess = false;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_RectControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_RectControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_RectControls.insert(std::pair<int, cui_rawImpl::RectControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addRectScaled

void cui_rawImpl::OnWM_CREATE(HWND hWnd, cui_raw* pThis)
{
	// get instance handle
	pThis->d->hInstance_ = (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE);

	// add window borders

	// add left border
	RECT rc;
	rc.left = 0;
	rc.right = 1;
	rc.top = 0;
	rc.bottom = pThis->d->m_icy;

	cui_raw::onResize resize;
	resize.iPercH = 0;
	resize.iPercV = 0;
	resize.iPercCY = 100;
	resize.iPercCX = 0;
	addRectScaled(pThis->d->m_sTitle, pThis->d->m_iLeftBorder, pThis->d->m_clrTheme, rc, resize, pThis->d);

	// add top border
	rc.bottom = 1;
	rc.right = pThis->d->m_icx;
	resize.iPercCY = 0;
	resize.iPercCX = 100;
	addRectScaled(pThis->d->m_sTitle, pThis->d->m_iTopBorder, pThis->d->m_clrTheme, rc, resize, pThis->d);

	// add right border
	rc.left = pThis->d->m_icx - 1;
	rc.bottom = pThis->d->m_icy;
	resize.iPercH = 100;
	resize.iPercV = 0;
	resize.iPercCY = 100;
	resize.iPercCX = 0;
	addRectScaled(pThis->d->m_sTitle, pThis->d->m_iRightBorder, pThis->d->m_clrTheme, rc, resize, pThis->d);

	// add bottom border
	rc.left = 0;
	rc.top = pThis->d->m_icy - 1;
	resize.iPercH = 0;
	resize.iPercV = 100;
	resize.iPercCY = 0;
	resize.iPercCX = 100;
	addRectScaled(pThis->d->m_sTitle, pThis->d->m_iBottomBorder, pThis->d->m_clrTheme, rc, resize, pThis->d);

	// add controls
	pThis->d->AddControls(hWnd, pThis->d->m_sCurrentPage, pThis);

	// enable control resizing
	pThis->d->m_pResizer->enable(hWnd);

	// create shadow
	pThis->d->CreateShadow(hWnd);

	// set timer
	if (pThis->d->m_iTimer > 0 && !pThis->d->m_bTimerRunning)
	{
		if (!pThis->d->m_bStartOnMouseMove)
		{
			// set timer running flag to true
			pThis->d->m_bTimerRunning = true;

			// start the timer
			SetTimer(hWnd, pThis->d->ID_TIMER, 1000 * pThis->d->m_iTimer, NULL);
		}
		else
		{
			// check cursor position
			pThis->d->m_ptStartCheck = { 0 };
			GetCursorPos(&pThis->d->m_ptStartCheck);

			// set check every two seconds
			SetTimer(hWnd, pThis->d->ID_TIMER_CHECK, 2000, NULL);
		}
	}
} // OnWM_CREATE

LRESULT cui_rawImpl::OnWM_NCHITTEST(HWND hWnd, LPARAM lParam, cui_rawImpl* d)
{
	RECT WindowRect;
	POINT pt;
	pt.x = 0;
	pt.y = 0;

	GetWindowRect(hWnd, &WindowRect);
	pt.x = GET_X_LPARAM(lParam) - WindowRect.left;
	pt.y = GET_Y_LPARAM(lParam) - WindowRect.top;

	int iBoarder = d->m_iBoarderWidth + 4;

	POINT m_pt = { 0, 0 };
	ScreenToClient(hWnd, &m_pt);

	if (pt.x >= iBoarder && pt.x <= WindowRect.right - WindowRect.left - iBoarder && pt.y >= iBoarder && pt.y <= d->m_iTitlebarHeight)
	{
		for (auto &it : d->m_ControlBtns)
		{
			RECT rc;
			GetWindowRect(it.second.hWnd, &rc);

			rc.left += m_pt.x;
			rc.right += m_pt.x;
			rc.top += m_pt.y;
			rc.bottom += m_pt.y;

			if (PtInRect(&rc, pt))
			{
				// mouse is over a control button, return HTCLIENT
				SetWindowLong(hWnd, DWLP_MSGRESULT, HTCLIENT);
				return HTCLIENT;
			}
		}

		for (auto &it : d->m_rcExclude)
		{
			RECT rc;
			GetWindowRect(it, &rc);

			rc.left += m_pt.x;
			rc.right += m_pt.x;
			rc.top += m_pt.y;
			rc.bottom += m_pt.y;

			if (PtInRect(&rc, pt))
			{
				// mouse is over a control that requested exclusion, return HTCLIENT
				SetWindowLong(hWnd, DWLP_MSGRESULT, HTCLIENT);
				return HTCLIENT;
			}
		}

		// mouse is over title bar, return HTCAPTION
		SetWindowLong(hWnd, DWLP_MSGRESULT, HTCAPTION);
		return HTCAPTION;
	}
	else if (!d->m_bEnableResize)
	{
		return HTCLIENT;
	}
	else if (pt.x < iBoarder && pt.y < iBoarder)
	{
		SetWindowLong(hWnd, DWLP_MSGRESULT, HTTOPLEFT);
		return HTTOPLEFT;
	}
	else if (pt.x > WindowRect.right - WindowRect.left - iBoarder && pt.y < iBoarder)
	{
		SetWindowLong(hWnd, DWLP_MSGRESULT, HTTOPRIGHT);
		return HTTOPRIGHT;
	}
	else if (pt.x > WindowRect.right - WindowRect.left - iBoarder && pt.y > WindowRect.bottom - WindowRect.top - iBoarder)
	{
		SetWindowLong(hWnd, DWLP_MSGRESULT, HTBOTTOMRIGHT);
		return HTBOTTOMRIGHT;
	}
	else if (pt.x < iBoarder && pt.y > WindowRect.bottom - WindowRect.top - iBoarder)
	{
		SetWindowLong(hWnd, DWLP_MSGRESULT, HTBOTTOMLEFT);
		return HTBOTTOMLEFT;
	}
	else if (pt.x < iBoarder)
	{
		SetWindowLong(hWnd, DWLP_MSGRESULT, HTLEFT);
		return HTLEFT;
	}
	else if (pt.y < iBoarder)
	{
		SetWindowLong(hWnd, DWLP_MSGRESULT, HTTOP);
		return HTTOP;
	}
	else if (pt.x > WindowRect.right - WindowRect.left - iBoarder)
	{
		SetWindowLong(hWnd, DWLP_MSGRESULT, HTRIGHT);
		return HTRIGHT;
	}
	else if (pt.y > WindowRect.bottom - WindowRect.top - iBoarder)
	{
		SetWindowLong(hWnd, DWLP_MSGRESULT, HTBOTTOM);
		return HTBOTTOM;
	}
	else
	{
		SetWindowLong(hWnd, DWLP_MSGRESULT, HTCLIENT);
		return HTCLIENT;
	}
} // OnWM_NCHITTEST

void cui_rawImpl::OnWM_GETMINMAXINFO(HWND hWnd, WPARAM wParam, LPARAM lParam, cui_rawImpl* d)
{
	/*
	** set lower limits to window size
	*/
	MINMAXINFO *pMinMaxInfo = (MINMAXINFO*)lParam;
	if (d->m_iMinWidth != -1)
	{
		// do not allow to be less than caption and min/max/close buttons
		pMinMaxInfo->ptMinTrackSize.x = d->m_iMinWidth > d->m_iMinWidthCalc ? d->m_iMinWidth : d->m_iMinWidthCalc;
	}
	if (d->m_iMinHeight != -1)
	{
		// do not allow to be less than title bar height
		pMinMaxInfo->ptMinTrackSize.y = d->m_iMinHeight > d->m_iTitlebarHeight + 2 * d->m_iBoarderWidth ? d->m_iMinHeight : d->m_iTitlebarHeight + 2 * d->m_iBoarderWidth;
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
} // OnWM_GETMINMAXINFO

void cui_rawImpl::OnWM_PAINT(HWND hWnd, cui_rawImpl* d)
{
	PAINTSTRUCT ps;
	HDC hdc = BeginPaint(hWnd, &ps);
	// do nothing
	EndPaint(hWnd, &ps);
} // OnWM_PAINT

void cui_rawImpl::checkPressControlButton(cui_rawImpl::ControlBtn &Control, POINT &pt)
{
	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd) && Control.bPressed)
	{
		RECT rect;
		GetWindowRect(Control.hWnd, &rect);

		POINT m_pt = { 0, 0 };
		ScreenToClient(GetParent(Control.hWnd), &m_pt);

		rect.left += m_pt.x;
		rect.right += m_pt.x;
		rect.top += m_pt.y;
		rect.bottom += m_pt.y;

		if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd) && PtInRect(&rect, pt))
		{
			if (Control.iUniqueID == Control.d->m_iIDClose)
			{
				if (Control.bCustomHandle)
				{
					// call the command procedure
					PostMessage(GetParent(Control.hWnd), WM_COMMAND, Control.iUniqueID, NULL);
				}
				else
				{
					// don't use DestroyWindow() so we can check whether closing app is permitted in handling WM_CLOSE
					PostMessage(GetParent(Control.hWnd), WM_CLOSE, (WPARAM)2, NULL);	// 2 means close IF closing is permitted
				}
			}

			if (Control.iUniqueID == Control.d->m_iIDMax)
			{
				// set cursor to normal
				SetClassLongPtr(GetParent(Control.hWnd), GCLP_HCURSOR, (LONG_PTR)Control.d->m_hNormalCursor);

				if (!IsMaximized(GetParent(Control.hWnd)))
					ShowWindow(GetParent(Control.hWnd), SW_MAXIMIZE);	// maximize button has been clicked
				else
					ShowWindow(GetParent(Control.hWnd), SW_RESTORE);	// restore button has been clicked

																		// make button cold ... window position changing suddenly
				Control.bHot = false;
				InvalidateRect(Control.hWnd, NULL, FALSE);
				UpdateWindow(Control.hWnd);
			}

			if (Control.iUniqueID == Control.d->m_iIDMin)
			{
				RECT rc;
				GetClientRect(Control.d->m_hWnd, &rc);

				UNscaleRECT(rc, Control.d->m_DPIScale);

				// capture window width and height
				Control.d->m_iWidth = rc.right - rc.left;
				Control.d->m_iHeight = rc.bottom - rc.top;

				// set cursor to normal
				SetClassLongPtr(GetParent(Control.hWnd), GCLP_HCURSOR, (LONG_PTR)Control.d->m_hNormalCursor);

				ShowWindow(GetParent(Control.hWnd), SW_MINIMIZE);	// minimize button has been clicked

																	// make button cold ... window position changing suddenly
				Control.bHot = false;
				InvalidateRect(Control.hWnd, NULL, FALSE);
				UpdateWindow(Control.hWnd);
			}
		}
	}

	Control.bPressed = false;
} // checkPressControlButton

void cui_rawImpl::checkPressToggleButtonControl(cui_rawImpl::ToggleButtonControl &Control, POINT &pt)
{
	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd) && Control.bPressed)
	{
		if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
		{
			if (pt.x == Control.ptStart.x)
			{
				// toggle button clicked (mouse has not moved horizontally since the left mouse button was pressed down)
				Control.bOn = !Control.bOn;
			}
			else
			{
				// toggle button dragged
				if (Control.iPercH > 50)
					Control.bOn = true;
				else
					Control.bOn = false;
			}

			if (Control.bOn != Control.bOldState)
				SendMessage(GetParent(Control.hWnd), WM_COMMAND, (WPARAM)Control.iUniqueID, NULL);
		}

		InvalidateRect(Control.hWnd, NULL, FALSE);
		UpdateWindow(Control.hWnd);
	}

	Control.bPressed = false;
	Control.iPercH = 0;
} // checkPressToggleButtonControl

void cui_rawImpl::checkPressStarRatingControl(cui_rawImpl::StarRatingControl &Control, POINT &pt)
{
	if (Control.bStatic)
		return;

	if (Control.bPressed)
	{
		RECT rect = Control.rcStarRating;
		POINT m_pt = { 0, 0 };
		ClientToScreen(Control.hWnd, &m_pt);

		rect.left += m_pt.x;
		rect.right += m_pt.x;
		rect.top += m_pt.y;
		rect.bottom += m_pt.y;

		m_pt = { 0, 0 };
		ScreenToClient(GetParent(Control.hWnd), &m_pt);

		rect.left += m_pt.x;
		rect.right += m_pt.x;
		rect.top += m_pt.y;
		rect.bottom += m_pt.y;

		if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd) && PtInRect(&rect, pt))
		{
			// capture rating
			int iRating = 0;
			for (auto &it : Control.rcStars)
			{
				if (it.bHot)
					iRating++;
				else
					break;
			}

			Control.iRating = iRating;

			// star rating control has been clicked
			SendMessage(GetParent(Control.hWnd), WM_COMMAND, (WPARAM)Control.iUniqueID, NULL);
		}
	}

	Control.bPressed = false;
} // checkPressStarRatingControl

void cui_rawImpl::checkRightPressStarRatingControl(cui_rawImpl::StarRatingControl &Control, POINT &pt)
{
	if (Control.bStatic)
		return;

	if (Control.bRightPressed)
	{
		RECT rect = Control.rcStarRating;
		POINT m_pt = { 0, 0 };
		ClientToScreen(Control.hWnd, &m_pt);

		rect.left += m_pt.x;
		rect.right += m_pt.x;
		rect.top += m_pt.y;
		rect.bottom += m_pt.y;

		m_pt = { 0, 0 };
		ScreenToClient(GetParent(Control.hWnd), &m_pt);

		rect.left += m_pt.x;
		rect.right += m_pt.x;
		rect.top += m_pt.y;
		rect.bottom += m_pt.y;

		if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd) && PtInRect(&rect, pt))
		{
			// set rating to 0 (unrated)
			Control.iRating = 0;

			// star rating control has been clicked
			SendMessage(GetParent(Control.hWnd), WM_COMMAND, (WPARAM)Control.iUniqueID, NULL);
		}
	}

	Control.bRightPressed = false;
} // checkRightPressStarRatingControl

void cui_rawImpl::checkPressButtonControl(cui_rawImpl::ButtonControl &Control, POINT &pt)
{
	bool bClicked = false;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd) && Control.bPressed)
	{
		RECT rect;
		GetWindowRect(Control.hWnd, &rect);

		POINT m_pt = { 0, 0 };
		ScreenToClient(GetParent(Control.hWnd), &m_pt);

		rect.left += m_pt.x;
		rect.right += m_pt.x;
		rect.top += m_pt.y;
		rect.bottom += m_pt.y;

		if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd) && PtInRect(&rect, pt))
		{
			// button clicked
			bClicked = true;
			InvalidateRect(Control.hWnd, NULL, TRUE);
			UpdateWindow(Control.hWnd);
			SendMessage(GetParent(Control.hWnd), WM_COMMAND, (WPARAM)Control.iUniqueID, NULL);
		}
	}

	Control.bPressed = false;

	/*
	** refresh button state ... if necessary
	** NOTE: this has to be done AFTER the bPressed flag is reset
	*/
	if (bClicked)
	{
		Control.state = enumBtn::normal;
		InvalidateRect(Control.hWnd, NULL, FALSE);
		UpdateWindow(Control.hWnd);
	}
} // checkPressButtonControl

void cui_rawImpl::checkPressTextControl(cui_rawImpl::TextControl &Control, POINT &pt)
{
	if (Control.bPressed)
	{
		RECT rect = Control.rcText;
		POINT m_pt = { 0, 0 };
		ClientToScreen(Control.hWnd, &m_pt);

		rect.left += m_pt.x;
		rect.right += m_pt.x;
		rect.top += m_pt.y;
		rect.bottom += m_pt.y;

		m_pt = { 0, 0 };
		ScreenToClient(GetParent(Control.hWnd), &m_pt);

		rect.left += m_pt.x;
		rect.right += m_pt.x;
		rect.top += m_pt.y;
		rect.bottom += m_pt.y;

		if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd) && PtInRect(&rect, pt))
		{
			// text control has been clicked
			Control.bDoubleClicked = false;
			SendMessage(GetParent(Control.hWnd), WM_COMMAND, (WPARAM)Control.iUniqueID, NULL);
		}
	}

	Control.bPressed = false;
} // checkPressTextControl

void cui_rawImpl::checkPressImageControl(cui_rawImpl::ImageControl &Control, POINT &pt)
{
	if (Control.bPressed)
	{
		RECT rect;
		GetWindowRect(Control.hWnd, &rect);

		RECT rcActive = rect;
		int iWidthDiff = (rect.right - rect.left) - (Control.rcActive.right - Control.rcActive.left);

		switch (Control.textPlacement)
		{
		case cui_raw::bottom:
		case cui_raw::top:
			break;
		case cui_raw::right:
		case cui_raw::righttop:
		case cui_raw::rightbottom:
			rcActive.right -= iWidthDiff;
			break;
		case cui_raw::left:
		case cui_raw::lefttop:
		case cui_raw::leftbottom:
			rcActive.left += iWidthDiff;
			break;
		default:
			break;
		}

		POINT m_pt = { 0, 0 };
		ScreenToClient(GetParent(Control.hWnd), &m_pt);

		rcActive.left += m_pt.x;
		rcActive.right += m_pt.x;
		rcActive.top += m_pt.y;
		rcActive.bottom += m_pt.y;

		if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd) && PtInRect(&rcActive, pt))
		{
			// image has been clicked
			Control.bPressed = true;
			SendMessage(GetParent(Control.hWnd), WM_COMMAND, (WPARAM)Control.iUniqueID, NULL);
		}
	}

	Control.bPressed = false;
} // checkPressImageControl

void cui_rawImpl::checkPressSelectorControl(cui_rawImpl::SelectorControl &Control, POINT &pt)
{
	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd) && Control.bPressed)
	{
		if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
		{
			if (pt.y == Control.ptStart.y)
			{
				// toggle button clicked
				RECT rcSelector = Control.rcSelector;

				POINT m_pt = { 0, 0 };
				ClientToScreen(Control.hWnd, &m_pt);

				rcSelector.left += m_pt.x;
				rcSelector.right += m_pt.x;
				rcSelector.top += m_pt.y;
				rcSelector.bottom += m_pt.y;

				m_pt = { 0, 0 };
				ScreenToClient(GetParent(Control.hWnd), &m_pt);

				rcSelector.left += m_pt.x;
				rcSelector.right += m_pt.x;
				rcSelector.top += m_pt.y;
				rcSelector.bottom += m_pt.y;

				int iPerc = 0;

				RECT rc = rcSelector;

				double dPerc = 100 * (double(pt.y) - double(rc.top)) /
					(double(rc.bottom) - double(rc.top));

				if (dPerc <= 0)
					iPerc = 0;
				else
				{
					if (dPerc >= 100)
						iPerc = 100;
					else
						iPerc = int(dPerc + 0.5);
				}

				Control.iPercV = iPerc;
			}
			else
			{
				// toggle button dragged
			}

			InvalidateRect(Control.hWnd, NULL, FALSE);
			UpdateWindow(Control.hWnd);

			if (Control.iSelectedItem != Control.iOldSelectedItem)
				SendMessage(GetParent(Control.hWnd), WM_COMMAND, (WPARAM)Control.iSelectedItem, NULL);
		}
	}

	Control.bPressed = false;
	Control.iPercV = 0;
} // checkPressSelectorControl

void cui_rawImpl::checkPressPieChartControl(cui_rawImpl::PieChartControl &Control, POINT &pt)
{
	Control.toolTip.hWndControl = Control.hWnd;

	bool bControlAvailable = false;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
		bControlAvailable = true;

	POINT m_pt = pt;

	// convert coordinates from parent window to screen
	ClientToScreen(GetParent(Control.hWnd), &m_pt);

	// convert coordinates from screen to control
	ScreenToClient(Control.hWnd, &m_pt);

	// convert to Gdiplus::Point
	Gdiplus::Point point(m_pt.x, m_pt.y);

	// make a graphics object from the control's HWND
	Gdiplus::Graphics g(Control.hWnd);

	bool bUpdate = false;

	bool bInPieChart = false;

	bool bPieClicked = false;

	for (size_t i = 0; i < Control.chartBarsInfo.size(); i++)
	{
		Gdiplus::Region *pRegion = Control.chartBarsInfo[i].pRegion;

		// check if the point is within the region
		bool bInRegion = false;
		if (pRegion->IsVisible(point, &g))
		{
			// The point is in the region
			bInRegion = true;
			bInPieChart = true;
		}
		else
		{
			// The point is not in the region
			bInRegion = false;
		}

		if (!bInRegion)
		{
			// check if point is over a label
			RECT rect = Control.chartBarsInfo[i].rcLabel;

			POINT m_pt = { 0, 0 };
			ClientToScreen(Control.hWnd, &m_pt);

			rect.left += m_pt.x;
			rect.right += m_pt.x;
			rect.top += m_pt.y;
			rect.bottom += m_pt.y;

			m_pt = { 0, 0 };
			ScreenToClient(GetParent(Control.hWnd), &m_pt);

			rect.left += m_pt.x;
			rect.right += m_pt.x;
			rect.top += m_pt.y;
			rect.bottom += m_pt.y;

			if (PtInRect(&rect, pt))
				bInRegion = true;
		}

		if (bControlAvailable && bInRegion)
		{
			if (Control.chartBarsInfo[i].bPressed)
			{
				// user has clicked a pie slice
				bPieClicked = true;
				break;
			}
		}
	}

	if (bPieClicked)
	{
		if (bControlAvailable)
		{
			// pie has been clicked
			SendMessage(GetParent(Control.hWnd), WM_COMMAND, (WPARAM)Control.iUniqueID, NULL);
		}
	}

	// reset ALL
	for (auto &it : Control.chartBarsInfo)
		it.bPressed = false;

	if (bInPieChart)
	{
		// TO-DO: improve ... find a way of doing this BEFORE
		InvalidateRect(Control.hWnd, NULL, FALSE);
		UpdateWindow(Control.hWnd);
	}
} // checkPressPieChartControl

void cui_rawImpl::OnWM_LBUTTONUP(HWND hWnd, cui_rawImpl* d)
{
	// release the mouse capture
	ReleaseCapture();

	d->m_bLButtonDown = false;

	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hWnd, &pt);
	int pos_x = pt.x;
	int pos_y = pt.y;

	try
	{
		// check if there is tab control in this page
		if (d->m_Pages.at(d->m_sCurrentPage).m_TabControl.iUniqueID != -123)	// TO-DO: remove magic number
		{
			bool bSelChange = false;

			size_t iSelected = 0;

			for (size_t i = 0; i < d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs.size(); i++)
			{
				RECT rect = d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].rcTab;

				POINT m_pt = { 0, 0 };
				ClientToScreen(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd, &m_pt);

				rect.left += m_pt.x;
				rect.right += m_pt.x;
				rect.top += m_pt.y;
				rect.bottom += m_pt.y;

				m_pt = { 0, 0 };
				ScreenToClient(hWnd, &m_pt);

				rect.left += m_pt.x;
				rect.right += m_pt.x;
				rect.top += m_pt.y;
				rect.bottom += m_pt.y;

				if (IsWindowEnabled(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd) && IsWindowVisible(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd) && insideRect(pos_x, pos_y, &rect))
				{
					if (d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bSelected == false && d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bPressed)
					{
						bSelChange = true;
						iSelected = i;

						d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bSelected = true;
					}
				}

				d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bPressed = false;
			}

			if (bSelChange)
			{
				// set selection change
				for (size_t i = 0; i < d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs.size(); i++)
				{
					if (i == iSelected)
						d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bSelected = true;
					else
						d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bSelected = false;
				}

				std::vector<HWND> *pVHidden = &d->m_vHiddenControls.at(d->m_sCurrentPage);

				// hide controls within tabs except those in the selected tab
				for (size_t i = 0; i < d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs.size(); i++)
				{
					DWORD dwCmd = SW_HIDE;

					if (d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bSelected)
						dwCmd = SW_SHOW;

					for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].m_Controls)
					{
						bool bHidden = false;

						if (std::find(pVHidden->begin(), pVHidden->end(), it.second) != pVHidden->end())
							bHidden = true;

						if (bHidden)
							ShowWindow(it.second, SW_HIDE);
						else
							ShowWindow(it.second, dwCmd);
					}
				}

				// selection changed ... repaint tab control
				InvalidateRect(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd, NULL, TRUE);
				UpdateWindow(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd);

				// reset cursor ... if neccessary
				if (GetCursor() != d->m_hNormalCursor)
					SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR)d->m_hNormalCursor);
			}
		}
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle control buttons
		for (auto &it : d->m_ControlBtns)
			d->checkPressControlButton(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's buttons
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_ButtonControls)
			d->checkPressButtonControl(it.second, pt);

		// handle pageless buttons
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_ButtonControls)
				if (it.second.bPageLess)
					d->checkPressButtonControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's toggle buttons
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_ToggleButtonControls)
			d->checkPressToggleButtonControl(it.second, pt);

		// handle pageless toggle buttons
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_ToggleButtonControls)
				if (it.second.bPageLess)
					d->checkPressToggleButtonControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's star rating controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_StarRatingControls)
			d->checkPressStarRatingControl(it.second, pt);

		// handle pageless star rating buttons
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_StarRatingControls)
				if (it.second.bPageLess)
					d->checkPressStarRatingControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's selector controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_SelectorControls)
			d->checkPressSelectorControl(it.second, pt);

		// handle pageless selector controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_SelectorControls)
				if (it.second.bPageLess)
					d->checkPressSelectorControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's text controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_TextControls)
			d->checkPressTextControl(it.second, pt);

		// handle pageless text controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_TextControls)
				if (it.second.bPageLess)
					d->checkPressTextControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's image controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_ImageControls)
			d->checkPressImageControl(it.second, pt);

		// handle pageless image controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_ImageControls)
				if (it.second.bPageLess)
					d->checkPressImageControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's pie chart controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_PieChartControls)
			d->checkPressPieChartControl(it.second, pt);

		// handle pageless pie chart controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_PieChartControls)
				if (it.second.bPageLess)
					d->checkPressPieChartControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}
} // OnWM_LBUTTONUP

void cui_rawImpl::OnWM_RBUTTONUP(HWND hWnd, cui_rawImpl* d)
{
	d->m_bRButtonDown = false;

	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hWnd, &pt);
	int pos_x = pt.x;
	int pos_y = pt.y;

	try
	{
		// handle current page's star rating controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_StarRatingControls)
			d->checkRightPressStarRatingControl(it.second, pt);

		// handle pageless star rating buttons
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_StarRatingControls)
				if (it.second.bPageLess)
					d->checkRightPressStarRatingControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}
} // OnWM_RBUTTONUP

void cui_rawImpl::OnWM_LBUTTONDBLCLK(HWND hWnd, cui_rawImpl* d)
{
	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hWnd, &pt);
	int pos_x = pt.x;
	int pos_y = pt.y;

	try
	{
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_TextControls)
		{
			if (!it.second.bStatic)
			{
				RECT rect = it.second.rcText;
				POINT m_pt = { 0, 0 };
				ClientToScreen(it.second.hWnd, &m_pt);

				rect.left += m_pt.x;
				rect.right += m_pt.x;
				rect.top += m_pt.y;
				rect.bottom += m_pt.y;

				m_pt = { 0, 0 };
				ScreenToClient(GetParent(it.second.hWnd), &m_pt);

				rect.left += m_pt.x;
				rect.right += m_pt.x;
				rect.top += m_pt.y;
				rect.bottom += m_pt.y;

				if (IsWindowEnabled(it.second.hWnd) && IsWindowVisible(it.second.hWnd) && insideRect(pos_x, pos_y, &rect))
				{
					// text control has been double-clicked
					it.second.bDoubleClicked = true;
					SendMessage(hWnd, WM_COMMAND, (WPARAM)it.second.iUniqueID, NULL);
				}
			}
		}
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}
} // OnWM_LBUTTONDBLCLK

void cui_rawImpl::flagPressControlButton(cui_rawImpl::ControlBtn &Control, POINT &pt)
{
	HideToolTip(Control.toolTip);

	RECT rect;
	GetWindowRect(Control.hWnd, &rect);

	POINT m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rect.left += m_pt.x;
	rect.right += m_pt.x;
	rect.top += m_pt.y;
	rect.bottom += m_pt.y;

	if (PtInRect(&rect, pt))
	{
		// control button pressed
		Control.bPressed = true;
	}
} // flagPressControlButton

void cui_rawImpl::flagPressButtonControl(cui_rawImpl::ButtonControl &Control, POINT &pt)
{
	HideToolTip(Control.toolTip);

	RECT rect;
	GetWindowRect(Control.hWnd, &rect);

	POINT m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rect.left += m_pt.x;
	rect.right += m_pt.x;
	rect.top += m_pt.y;
	rect.bottom += m_pt.y;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd) && PtInRect(&rect, pt))
	{
		// button pressed
		Control.bPressed = true;
		Control.state = enumBtn::hover;
		InvalidateRect(Control.hWnd, NULL, FALSE);
		UpdateWindow(Control.hWnd);
	}
} // flagPressButtonControl

void cui_rawImpl::flagPressToggleButtonControl(cui_rawImpl::ToggleButtonControl &Control, POINT &pt)
{
	HideToolTip(Control.toolTip);
	Control.bOldState = Control.bOn;

	RECT rcToggler = Control.rcToggler;

	POINT m_pt = { 0, 0 };
	ClientToScreen(Control.hWnd, &m_pt);

	rcToggler.left += m_pt.x;
	rcToggler.right += m_pt.x;
	rcToggler.top += m_pt.y;
	rcToggler.bottom += m_pt.y;

	m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rcToggler.left += m_pt.x;
	rcToggler.right += m_pt.x;
	rcToggler.top += m_pt.y;
	rcToggler.bottom += m_pt.y;

	if (PtInRect(&rcToggler, pt))
	{
		// button pressed
		Control.ptStart = pt;
		Control.bPressed = true;
	}
} // flagPressToggleButtonControl

void cui_rawImpl::flagPressStarRatingControl(cui_rawImpl::StarRatingControl &Control, POINT &pt)
{
	if (Control.bStatic)
		return;

	HideToolTip(Control.toolTip);

	RECT rcToggler = Control.rcStarRating;

	POINT m_pt = { 0, 0 };
	ClientToScreen(Control.hWnd, &m_pt);

	rcToggler.left += m_pt.x;
	rcToggler.right += m_pt.x;
	rcToggler.top += m_pt.y;
	rcToggler.bottom += m_pt.y;

	m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rcToggler.left += m_pt.x;
	rcToggler.right += m_pt.x;
	rcToggler.top += m_pt.y;
	rcToggler.bottom += m_pt.y;

	if (PtInRect(&rcToggler, pt))
	{
		// button pressed
		Control.bPressed = true;
	}
} // flagPressStarRatingControl

void cui_rawImpl::flagRightPressStarRatingControl(cui_rawImpl::StarRatingControl &Control, POINT &pt)
{
	if (Control.bStatic)
		return;

	HideToolTip(Control.toolTip);

	RECT rcToggler = Control.rcStarRating;

	POINT m_pt = { 0, 0 };
	ClientToScreen(Control.hWnd, &m_pt);

	rcToggler.left += m_pt.x;
	rcToggler.right += m_pt.x;
	rcToggler.top += m_pt.y;
	rcToggler.bottom += m_pt.y;

	m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rcToggler.left += m_pt.x;
	rcToggler.right += m_pt.x;
	rcToggler.top += m_pt.y;
	rcToggler.bottom += m_pt.y;

	if (PtInRect(&rcToggler, pt))
	{
		// button pressed
		Control.bRightPressed = true;
	}
} // flagRightPressStarRatingControl

void cui_rawImpl::flagPressTextControl(cui_rawImpl::TextControl &Control, POINT &pt)
{
	HideToolTip(Control.toolTip);

	RECT rect;
	GetWindowRect(Control.hWnd, &rect);

	POINT m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rect.left += m_pt.x;
	rect.right += m_pt.x;
	rect.top += m_pt.y;
	rect.bottom += m_pt.y;

	if (!Control.bStatic && PtInRect(&rect, pt))
	{
		// text control pressed
		Control.bPressed = true;
	}
} // flagPressTextControl

void cui_rawImpl::flagPressImageControl(cui_rawImpl::ImageControl &Control, POINT &pt)
{
	HideToolTip(Control.toolTip);

	RECT rect;
	GetWindowRect(Control.hWnd, &rect);

	RECT rcActive = rect;
	int iWidthDiff = (rect.right - rect.left) - (Control.rcActive.right - Control.rcActive.left);

	switch (Control.textPlacement)
	{
	case cui_raw::bottom:
	case cui_raw::top:
		break;
	case cui_raw::right:
	case cui_raw::righttop:
	case cui_raw::rightbottom:
		rcActive.right -= iWidthDiff;
		break;
	case cui_raw::left:
	case cui_raw::lefttop:
	case cui_raw::leftbottom:
		rcActive.left += iWidthDiff;
		break;
	default:
		break;
	}

	POINT m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rcActive.left += m_pt.x;
	rcActive.right += m_pt.x;
	rcActive.top += m_pt.y;
	rcActive.bottom += m_pt.y;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd) && PtInRect(&rcActive, pt))
	{
		// image pressed
		Control.bPressed = true;
	}
} // flagPressImageControl

void cui_rawImpl::flagPressSelectorControl(cui_rawImpl::SelectorControl &Control, POINT &pt)
{
	HideToolTip(Control.toolTip);
	Control.iOldSelectedItem = Control.iSelectedItem;

	RECT rcSelector = Control.rcSelector;

	POINT m_pt = { 0, 0 };
	ClientToScreen(Control.hWnd, &m_pt);

	rcSelector.left += m_pt.x;
	rcSelector.right += m_pt.x;
	rcSelector.top += m_pt.y;
	rcSelector.bottom += m_pt.y;

	m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rcSelector.left += m_pt.x;
	rcSelector.right += m_pt.x;
	rcSelector.top += m_pt.y;
	rcSelector.bottom += m_pt.y;

	RECT m_Hot = rcSelector;
	m_Hot.top -= Control.yUpper;
	m_Hot.bottom += Control.yLower;

	if (PtInRect(&m_Hot, pt))
	{
		// selector pressed
		Control.ptStart = pt;
		Control.bPressed = true;
	}
} // flagPressSelectorControl

void cui_rawImpl::flagPressPieChartControl(cui_rawImpl::PieChartControl &Control, POINT &pt)
{
	HideToolTip(Control.toolTip);

	Control.toolTip.hWndControl = Control.hWnd;

	bool bControlAvailable = false;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
		bControlAvailable = true;

	POINT m_pt = pt;

	// convert coordinates from parent window to screen
	ClientToScreen(GetParent(Control.hWnd), &m_pt);

	// convert coordinates from screen to control
	ScreenToClient(Control.hWnd, &m_pt);

	// convert to Gdiplus::Point
	Gdiplus::Point point(m_pt.x, m_pt.y);

	// make a graphics object from the control's HWND
	Gdiplus::Graphics g(Control.hWnd);

	bool bUpdate = false;

	bool bInPieChart = false;

	for (size_t i = 0; i < Control.chartBarsInfo.size(); i++)
	{
		Gdiplus::Region *pRegion = Control.chartBarsInfo[i].pRegion;

		// check if the point is within the region
		bool bInRegion = false;
		if (pRegion->IsVisible(point, &g))
		{
			// The point is in the region
			bInRegion = true;
			bInPieChart = true;
		}
		else
		{
			// The point is not in the region
			bInRegion = false;
		}

		if (!bInRegion)
		{
			// check if point is over a label
			RECT rect = Control.chartBarsInfo[i].rcLabel;

			POINT m_pt = { 0, 0 };
			ClientToScreen(Control.hWnd, &m_pt);

			rect.left += m_pt.x;
			rect.right += m_pt.x;
			rect.top += m_pt.y;
			rect.bottom += m_pt.y;

			m_pt = { 0, 0 };
			ScreenToClient(GetParent(Control.hWnd), &m_pt);

			rect.left += m_pt.x;
			rect.right += m_pt.x;
			rect.top += m_pt.y;
			rect.bottom += m_pt.y;

			if (PtInRect(&rect, pt))
				bInRegion = true;
		}

		if (bControlAvailable && bInRegion)
		{
			// user has pressed a bar
			Control.chartBarsInfo[i].bPressed = true;
			bUpdate = true;
		}
		else
		{
			// reset bar
			if (Control.chartBarsInfo[i].bPressed)
			{
				Control.chartBarsInfo[i].bPressed = false;
				bUpdate = true;
			}
		}
	}

	if (bUpdate)
	{
		InvalidateRect(Control.hWnd, NULL, FALSE);
		UpdateWindow(Control.hWnd);
	}
} // flagPressPieChartControl

void cui_rawImpl::OnWM_LBUTTONDOWN(HWND hWnd, cui_rawImpl* d)
{
	// capture the mouse
	SetCapture(hWnd);

	d->m_bLButtonDown = true;

	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hWnd, &pt);
	int pos_x = pt.x;
	int pos_y = pt.y;

	try
	{
		// check if there is tab control in this page
		if (d->m_Pages.at(d->m_sCurrentPage).m_TabControl.iUniqueID != -123)	// TO-DO: remove magic number
		{
			HideToolTip(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.toolTip);

			for (size_t i = 0; i < d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs.size(); i++)
			{
				RECT rect = d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].rcTab;

				POINT m_pt = { 0, 0 };
				ClientToScreen(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd, &m_pt);

				rect.left += m_pt.x;
				rect.right += m_pt.x;
				rect.top += m_pt.y;
				rect.bottom += m_pt.y;

				m_pt = { 0, 0 };
				ScreenToClient(hWnd, &m_pt);

				rect.left += m_pt.x;
				rect.right += m_pt.x;
				rect.top += m_pt.y;
				rect.bottom += m_pt.y;

				if (IsWindowEnabled(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd) && IsWindowVisible(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd) && insideRect(pos_x, pos_y, &rect))
				{
					if (!d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bPressed)
					{
						d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bPressed = true;
					}
				}
			}
		}
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle control buttons
		for (auto &it : d->m_ControlBtns)
			d->flagPressControlButton(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's button controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_ButtonControls)
			d->flagPressButtonControl(it.second, pt);

		// handle pageless button controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_ButtonControls)
				if (it.second.bPageLess)
					d->flagPressButtonControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's toggle button controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_ToggleButtonControls)
			d->flagPressToggleButtonControl(it.second, pt);

		// handle pageless toggle button controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_ToggleButtonControls)
				if (it.second.bPageLess)
					d->flagPressToggleButtonControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's star rating controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_StarRatingControls)
			d->flagPressStarRatingControl(it.second, pt);

		// handle pageless star rating controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_StarRatingControls)
				if (it.second.bPageLess)
					d->flagPressStarRatingControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's selector controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_SelectorControls)
			d->flagPressSelectorControl(it.second, pt);

		// handle pageless selector controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_SelectorControls)
				if (it.second.bPageLess)
					d->flagPressSelectorControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's text controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_TextControls)
			d->flagPressTextControl(it.second, pt);

		// handle pageless text controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_TextControls)
				if (it.second.bPageLess)
					d->flagPressTextControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's image controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_ImageControls)
			d->flagPressImageControl(it.second, pt);

		// handle pageless image controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_ImageControls)
				if (it.second.bPageLess)
					d->flagPressImageControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's pie chart controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_PieChartControls)
			d->flagPressPieChartControl(it.second, pt);

		// handle pageless pie chart controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_PieChartControls)
				if (it.second.bPageLess)
					d->flagPressPieChartControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}
} // OnWM_LBUTTONDOWN

void cui_rawImpl::OnWM_RBUTTONDOWN(HWND hWnd, cui_rawImpl* d)
{
	// capture the mouse
	d->m_bRButtonDown = true;

	POINT pt;
	GetCursorPos(&pt);
	ScreenToClient(hWnd, &pt);
	int pos_x = pt.x;
	int pos_y = pt.y;

	try
	{
		// handle current page's star rating controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_StarRatingControls)
			d->flagRightPressStarRatingControl(it.second, pt);

		// handle pageless star rating controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_StarRatingControls)
				if (it.second.bPageLess)
					d->flagRightPressStarRatingControl(it.second, pt);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}
} // OnWM_RBUTTONDOWN

void cui_rawImpl::OnWM_SIZE(HWND hWnd, WPARAM wParam, cui_rawImpl* d)
{
	// must be before the place where m_bMaximized is revised
	if (wParam == SIZE_RESTORED && d->m_bMaximized)
	{
		// window has just been restored from a maximized state
		if (d->m_bMaxbtn)
		{
			try
			{
				d->m_ControlBtns.at(d->m_iIDMax).bHot = false;
				InvalidateRect(d->m_ControlBtns.at(d->m_iIDMax).hWnd, NULL, FALSE);
				UpdateWindow(d->m_ControlBtns.at(d->m_iIDMax).hWnd);
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
			}
		}

		try
		{
			for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_ButtonControls)
			{
				// reset buttons
				it.second.state = enumBtn::normal;
				InvalidateRect(it.second.hWnd, NULL, FALSE);
				UpdateWindow(it.second.hWnd);
			}
		}
		catch (std::exception &e)
		{
			// do nothing ... map probably out of range
			std::string m_sErr = e.what();
		}
	}

	if (IsMaximized(hWnd))
	{
		d->m_bMaximized = true;

		try
		{
			// disable window borders
			ShowWindow(d->m_Pages.at(d->m_sTitle).m_RectControls.at(d->m_iLeftBorder).hWnd, SW_HIDE);
			ShowWindow(d->m_Pages.at(d->m_sTitle).m_RectControls.at(d->m_iTopBorder).hWnd, SW_HIDE);
			ShowWindow(d->m_Pages.at(d->m_sTitle).m_RectControls.at(d->m_iRightBorder).hWnd, SW_HIDE);
			ShowWindow(d->m_Pages.at(d->m_sTitle).m_RectControls.at(d->m_iBottomBorder).hWnd, SW_HIDE);
		}
		catch (std::exception &e)
		{
			// do nothing ... map probably out of range
			std::string m_sErr = e.what();
		}
	}
	else
	{
		d->m_bMaximized = false;

		try
		{
			// enable window borders
			ShowWindow(d->m_Pages.at(d->m_sTitle).m_RectControls.at(d->m_iLeftBorder).hWnd, SW_SHOW);
			ShowWindow(d->m_Pages.at(d->m_sTitle).m_RectControls.at(d->m_iTopBorder).hWnd, SW_SHOW);
			ShowWindow(d->m_Pages.at(d->m_sTitle).m_RectControls.at(d->m_iRightBorder).hWnd, SW_SHOW);
			ShowWindow(d->m_Pages.at(d->m_sTitle).m_RectControls.at(d->m_iBottomBorder).hWnd, SW_SHOW);
		}
		catch (std::exception &e)
		{
			// do nothing ... map probably out of range
			std::string m_sErr = e.what();
		}
	}
} // OnWM_SIZE

void cui_rawImpl::resetImageControl(cui_rawImpl::ImageControl &Control)
{
	HideToolTip(Control.toolTip);

	if (Control.bHot)
	{
		Control.bHot = false;

		if (Control.bButtonBar)
		{
			InvalidateRect(Control.hWnd, &Control.rcBar, TRUE);
			UpdateWindow(Control.hWnd);
		}

		int iRefreshWidth = 1;

		if (!Control.bChangeColor && (Control.clrBackground == Control.clrBackgroundHot))
		{
			RECT rectButton;
			GetClientRect(Control.hWnd, &rectButton);
			RECT rc = rectButton;
			rc.right = rc.left + iRefreshWidth;
			InvalidateRect(Control.hWnd, &rc, TRUE);
			UpdateWindow(Control.hWnd);

			rc = rectButton;
			rc.bottom = rc.top + iRefreshWidth;
			InvalidateRect(Control.hWnd, &rc, TRUE);
			UpdateWindow(Control.hWnd);

			rc = rectButton;
			rc.left = rc.right - iRefreshWidth;
			InvalidateRect(Control.hWnd, &rc, TRUE);
			UpdateWindow(Control.hWnd);

			rc = rectButton;
			rc.top = rc.bottom - iRefreshWidth;
			InvalidateRect(Control.hWnd, &rc, TRUE);
			UpdateWindow(Control.hWnd);
		}
		else
		{
			InvalidateRect(Control.hWnd, NULL, TRUE);
			UpdateWindow(Control.hWnd);
		}
	}

	if (Control.bToggle)
	{
		RECT rect;
		GetWindowRect(Control.hWnd, &rect);

		if (Control.bOffset)
		{
			switch (Control.toggleAction)
			{
			case cui_raw::toggleLeft:
				rect.right += Control.iOffset;
				break;

			case cui_raw::toggleRight:
				rect.right -= Control.iOffset;
				break;

			case cui_raw::toggleUp:
				/*
				** button is offset, meaning the bottom is 2 pixels above the client specified RECT
				** all other sides are against the borders of the client specified RECT
				*/
				rect.bottom += Control.iOffset;
				break;

			case cui_raw::toggleDown:
				rect.bottom -= Control.iOffset;
				break;

			default:
				/*
				** button is offset, meaning the bottom is 2 pixels above the client specified RECT
				** all other sides are against the borders of the client specified RECT
				*/
				rect.bottom += Control.iOffset;
				break;
			}
		}
		else
		{
			switch (Control.toggleAction)
			{
			case cui_raw::toggleLeft:
				rect.left -= Control.iOffset;
				break;

			case cui_raw::toggleRight:
				rect.left += Control.iOffset;
				break;

			case cui_raw::toggleUp:
				/*
				** button is NOT offset, meaning the top is 2 pixels below the client specified RECT
				** all other sides are against the borders of the client specified RECT
				*/
				rect.top -= Control.iOffset;
				break;

			case cui_raw::toggleDown:
				rect.top += Control.iOffset;
				break;

			default:
				/*
				** button is NOT offset, meaning the top is 2 pixels below the client specified RECT
				** all other sides are against the borders of the client specified RECT
				*/
				rect.top -= Control.iOffset;
				break;
			}

			/*
			** button is NOT offset, meaning the top is 2 pixels below the client specified RECT
			** all other sides are against the borders of the client specified RECT
			*/
			rect.top -= Control.iOffset;
		}

		POINT m_pt = { 0, 0 };
		ScreenToClient(GetParent(Control.hWnd), &m_pt);

		rect.left += m_pt.x;
		rect.right += m_pt.x;
		rect.top += m_pt.y;
		rect.bottom += m_pt.y;

		// reset button position
		if (Control.bOffset)
		{
			switch (Control.toggleAction)
			{
			case cui_raw::toggleLeft:
				SetWindowPos(Control.hWnd, NULL, rect.left + Control.iOffset, rect.top, 0, 0, SWP_NOSIZE);
				break;

			case cui_raw::toggleRight:
				SetWindowPos(Control.hWnd, NULL, rect.left - Control.iOffset, rect.top, 0, 0, SWP_NOSIZE);
				break;

			case cui_raw::toggleUp:
				SetWindowPos(Control.hWnd, NULL, rect.left, rect.top + Control.iOffset, 0, 0, SWP_NOSIZE);
				break;

			case cui_raw::toggleDown:
				SetWindowPos(Control.hWnd, NULL, rect.left, rect.top - Control.iOffset, 0, 0, SWP_NOSIZE);
				break;

			default:
				SetWindowPos(Control.hWnd, NULL, rect.left, rect.top + Control.iOffset, 0, 0, SWP_NOSIZE);
				break;
			}

			Control.bOffset = false;
		}
	}
} // resetImageControl

void cui_rawImpl::OnWM_MOUSELEAVE(HWND hWnd, cui_rawImpl* d)
{
	// reset window cursor
	SetClassLongPtr(d->m_hWnd, GCLP_HCURSOR, (LONG_PTR)d->m_hNormalCursor);

	try
	{
		// check if there is tab control in this page
		if (d->m_Pages.at(d->m_sCurrentPage).m_TabControl.iUniqueID != -123)	// TO-DO: remove magic number
		{
			d->m_Pages.at(d->m_sCurrentPage).m_TabControl.toolTip.sText.clear();
			HideToolTip(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.toolTip);

			for (size_t i = 0; i < d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs.size(); i++)
			{
				// reset buttons
				if (d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bHot)
				{
					d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bHot = false;
					RECT rc = d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].rcTab;
					InflateRect(&rc, 5, 5);
					InvalidateRect(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd, &rc, TRUE);
					UpdateWindow(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd);
				}
			}
		}
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// reset bar chart controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_BarChartControls)
		{
			for (auto &m_it : it.second.chartBarsInfo)
				m_it.bHot = false;

			HideToolTip(it.second.toolTip);
			InvalidateRect(it.second.hWnd, NULL, FALSE);
			UpdateWindow(it.second.hWnd);
		}

		// reset pageless bar chart controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_BarChartControls)
			{
				for (auto &m_it : it.second.chartBarsInfo)
					m_it.bHot = false;

				HideToolTip(it.second.toolTip);
				InvalidateRect(it.second.hWnd, NULL, FALSE);
				UpdateWindow(it.second.hWnd);
			}
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// reset line chart controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_LineChartControls)
		{
			for (auto &it : it.second.linesInfo)
			{
				it.bHot = false;

				for (auto &m_it : it.chartLinesInfo)
					m_it.bHot = false;
			}

			HideToolTip(it.second.toolTip);
			InvalidateRect(it.second.hWnd, NULL, FALSE);
			UpdateWindow(it.second.hWnd);
		}

		// reset pageless line chart controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_LineChartControls)
			{
				for (auto &it : it.second.linesInfo)
				{
					it.bHot = false;

					for (auto &m_it : it.chartLinesInfo)
						m_it.bHot = false;
				}

				HideToolTip(it.second.toolTip);
				InvalidateRect(it.second.hWnd, NULL, FALSE);
				UpdateWindow(it.second.hWnd);
			}
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// reset pie chart controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_PieChartControls)
		{
			for (auto &m_it : it.second.chartBarsInfo)
				m_it.bHot = false;

			HideToolTip(it.second.toolTip);
			InvalidateRect(it.second.hWnd, NULL, FALSE);
			UpdateWindow(it.second.hWnd);
		}

		// reset pageless pie chart controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_PieChartControls)
			{
				for (auto &m_it : it.second.chartBarsInfo)
					m_it.bHot = false;

				HideToolTip(it.second.toolTip);
				InvalidateRect(it.second.hWnd, NULL, FALSE);
				UpdateWindow(it.second.hWnd);
			}
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// reset control buttons
		for (auto &it : d->m_ControlBtns)
		{
			HideToolTip(it.second.toolTip);
			it.second.bHot = false;
			InvalidateRect(it.second.hWnd, NULL, FALSE);
			UpdateWindow(it.second.hWnd);
		}
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// reset toggle buttons
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_ToggleButtonControls)
			HideToolTip(it.second.toolTip);

		// reset pageless toggle buttons
		if (d->m_sCurrentPage != d->m_sTitle)
		{
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_ToggleButtonControls)
				HideToolTip(it.second.toolTip);
		}
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// reset star rating controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_StarRatingControls)
			HideToolTip(it.second.toolTip);

		// reset star rating controls
		if (d->m_sCurrentPage != d->m_sTitle)
		{
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_StarRatingControls)
				HideToolTip(it.second.toolTip);
		}
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// reset selector controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_SelectorControls)
			HideToolTip(it.second.toolTip);

		// reset pageless selector controls
		if (d->m_sCurrentPage != d->m_sTitle)
		{
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_SelectorControls)
				HideToolTip(it.second.toolTip);
		}
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// reset button controls on current page
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_ButtonControls)
		{
			HideToolTip(it.second.toolTip);

			if (it.second.state == enumBtn::hover)
			{
				if (IsWindowEnabled(it.second.hWnd) && IsWindowVisible(it.second.hWnd))
				{
					// reset buttons
					it.second.state = enumBtn::normal;
					InvalidateRect(it.second.hWnd, NULL, FALSE);
					UpdateWindow(it.second.hWnd);
				}
			}
		}

		// reset button controls on pageless controls
		if (d->m_sCurrentPage != d->m_sTitle)
		{
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_ButtonControls)
			{
				HideToolTip(it.second.toolTip);

				if (it.second.bPageLess && it.second.state == enumBtn::hover)
				{
					if (IsWindowEnabled(it.second.hWnd) && IsWindowVisible(it.second.hWnd))
					{
						// reset buttons
						it.second.state = enumBtn::normal;
						InvalidateRect(it.second.hWnd, NULL, FALSE);
						UpdateWindow(it.second.hWnd);
					}
				}
			}
		}
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// reset image controls on current page
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_ImageControls)
			d->resetImageControl(it.second);

		// reset pageless image controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_ImageControls)
				if (it.second.bPageLess)
					d->resetImageControl(it.second);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// reset text controls on current page
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_TextControls)
		{
			HideToolTip(it.second.toolTip);

			if (it.second.bHot)
			{
				it.second.bHot = false;

				if (IsWindowEnabled(it.second.hWnd) && IsWindowVisible(it.second.hWnd))
				{
					// reset buttons
					InvalidateRect(it.second.hWnd, NULL, FALSE);
					UpdateWindow(it.second.hWnd);
				}
			}
		}

		// reset text controls on pageless controls
		if (d->m_sCurrentPage != d->m_sTitle)
		{
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_TextControls)
			{
				HideToolTip(it.second.toolTip);

				if (it.second.bHot)
				{
					it.second.bHot = false;

					if (IsWindowEnabled(it.second.hWnd) && IsWindowVisible(it.second.hWnd))
					{
						// reset buttons
						InvalidateRect(it.second.hWnd, NULL, FALSE);
						UpdateWindow(it.second.hWnd);
					}
				}
			}
		}
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	// reset mouse tracking
	d->m_pMouseTrack->Reset(hWnd);
} // OnWM_MOUSELEAVE

void cui_rawImpl::hitControlButton(cui_rawImpl::ControlBtn &Control, POINT &pt, std::vector<RECT> &m_vHotRects)
{
	Control.toolTip.hWndControl = Control.hWnd;

	RECT rect;
	GetWindowRect(Control.hWnd, &rect);

	POINT m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rect.left += m_pt.x;
	rect.right += m_pt.x;
	rect.top += m_pt.y;
	rect.bottom += m_pt.y;

	bool bControlAvailable = false;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
		bControlAvailable = true;

	if (bControlAvailable)
		m_vHotRects.push_back(rect);

	if (bControlAvailable && PtInRect(&rect, pt))
	{
		// mouse is over a button
		if (!Control.bHot)
		{
			if (Control.iUniqueID == Control.d->m_iIDMax)
			{
				if (IsMaximized(GetParent(Control.hWnd)))
					Control.toolTip.sText = _T("Restore Down");
				else
					Control.toolTip.sText = _T("Maximize");
			}

			ShowToolTip(Control.toolTip);
			Control.bHot = true;
			InvalidateRect(Control.hWnd, NULL, FALSE);
			UpdateWindow(Control.hWnd);
		}
	}
	else
	{
		HideToolTip(Control.toolTip);

		// reset buttons
		if (Control.bHot)
		{
			Control.bHot = false;
			InvalidateRect(Control.hWnd, NULL, FALSE);
			UpdateWindow(Control.hWnd);
		}
	}
} // hitControlButton

void cui_rawImpl::hitButtonControl(cui_rawImpl::ButtonControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects)
{
	Control.toolTip.hWndControl = Control.hWnd;

	RECT rect;
	GetWindowRect(Control.hWnd, &rect);

	POINT m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rect.left += m_pt.x;
	rect.right += m_pt.x;
	rect.top += m_pt.y;
	rect.bottom += m_pt.y;

	bool bControlAvailable = false;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
	{
		bControlAvailable = true;
		m_vHotRects.push_back(rect);
	}

	if (PtInRect(&rect, pt))
	{
		// mouse is over a button
		if (Control.state != enumBtn::hover)
		{
			if (bControlAvailable)
			{
				ShowToolTip(Control.toolTip);
				Control.state = enumBtn::hover;
				InvalidateRect(Control.hWnd, NULL, FALSE);
				UpdateWindow(Control.hWnd);
			}
		}
	}
	else
	{
		HideToolTip(Control.toolTip);

		// reset buttons
		if (Control.state == enumBtn::hover)
		{
			if (bControlAvailable)
			{
				Control.state = enumBtn::normal;
				InvalidateRect(Control.hWnd, NULL, FALSE);
				UpdateWindow(Control.hWnd);
			}
		}
	}
} // hitButtonControl

void cui_rawImpl::hitToggleButtonControl(cui_rawImpl::ToggleButtonControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects)
{
	Control.toolTip.hWndControl = Control.hWnd;

	RECT rcToggler = Control.rcToggler;

	POINT m_pt = { 0, 0 };
	ClientToScreen(Control.hWnd, &m_pt);

	rcToggler.left += m_pt.x;
	rcToggler.right += m_pt.x;
	rcToggler.top += m_pt.y;
	rcToggler.bottom += m_pt.y;

	m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rcToggler.left += m_pt.x;
	rcToggler.right += m_pt.x;
	rcToggler.top += m_pt.y;
	rcToggler.bottom += m_pt.y;

	bool bControlAvailable = false;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
		bControlAvailable = true;

	if (bControlAvailable)
		m_vHotRects.push_back(rcToggler);

	if (bControlAvailable && PtInRect(&rcToggler, pt))
	{
		// mouse is over a button
		if (!Control.bHot)
		{
			ShowToolTip(Control.toolTip);
			Control.bHot = true;
		}
	}
	else
	{
		HideToolTip(Control.toolTip);

		// reset buttons
		if (Control.bHot)
			Control.bHot = false;
	}

	if (bControlAvailable && Control.bPressed)
	{
		if (Control.d->m_bLButtonDown)
		{
			int iPerc = 0;

			RECT rc = rcToggler;
			InflateRect(&rc, -2, -2);

			double dPerc = 100 * (double(pt.x) - double(rc.left)) /
				(double(rc.right) - double(rc.left));

			if (dPerc <= 0)
				iPerc = 0;
			else
			{
				if (dPerc >= 100)
					iPerc = 100;
				else
					iPerc = int(dPerc + 0.5);
			}

			Control.iPercH = iPerc;

			InvalidateRect(Control.hWnd, &Control.rcToggler, FALSE);
		}
	}
} // hitToggleButtonControl

void cui_rawImpl::hitStarRatingControl(cui_rawImpl::StarRatingControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects)
{
	Control.toolTip.hWndControl = Control.hWnd;

	RECT rcStarRating = Control.rcStarRating;

	POINT m_pt = { 0, 0 };
	ClientToScreen(Control.hWnd, &m_pt);

	rcStarRating.left += m_pt.x;
	rcStarRating.right += m_pt.x;
	rcStarRating.top += m_pt.y;
	rcStarRating.bottom += m_pt.y;

	m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rcStarRating.left += m_pt.x;
	rcStarRating.right += m_pt.x;
	rcStarRating.top += m_pt.y;
	rcStarRating.bottom += m_pt.y;

	bool bControlAvailable = false;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
		bControlAvailable = true;

	if (bControlAvailable && !Control.bStatic)
		m_vHotRects.push_back(rcStarRating);

	if (bControlAvailable && PtInRect(&rcStarRating, pt))
	{
		// mouse is over a button
		if (!Control.bHot)
		{
			ShowToolTip(Control.toolTip);
			Control.bHot = true;
		}

		if (Control.bStatic)
			return;

		int iPerc = 0;

		RECT rc = rcStarRating;
		InflateRect(&rc, 0, 0);

		double dPerc = 100 * (double(pt.x) - double(rc.left)) /
			(double(rc.right) - double(rc.left));

		if (dPerc <= 0)
			iPerc = 0;
		else
		{
			if (dPerc >= 100)
				iPerc = 100;
			else
				iPerc = int(dPerc + 0.5);
		}

		for (auto &it : Control.rcStars)
			it.bHot = false;

		Control.rcStars[0].bHot = true;

		int i = 0;
		for (auto &it : Control.rcStars)
		{
			if (i == 0)
			{
				i++;
				continue;
			}

			if (iPerc > ((100 * i) / (int)Control.rcStars.size()))
				it.bHot = true;

			i++;
		}

		InvalidateRect(Control.hWnd, &Control.rcStarRating, FALSE);	// TO-DO: make this conditional to avoid overdrawing
	}
	else
	{
		HideToolTip(Control.toolTip);

		for (auto &it : Control.rcStars)
			it.bHot = false;

		// reset buttons
		if (Control.bHot)
		{
			Control.bHot = false;
			InvalidateRect(Control.hWnd, &Control.rcStarRating, FALSE);
		}
	}

	if (bControlAvailable && Control.bPressed)
	{
		if (Control.d->m_bLButtonDown)
		{

		}
	}
} // hitStarRatingControl

void cui_rawImpl::hitImageControl(cui_rawImpl::ImageControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects)
{
	Control.toolTip.hWndControl = Control.hWnd;

	RECT rect;
	GetWindowRect(Control.hWnd, &rect);

	if (Control.bOffset)
	{
		switch (Control.toggleAction)
		{
		case cui_raw::toggleLeft:
			rect.right += Control.iOffset;
			break;

		case cui_raw::toggleRight:
			rect.right -= Control.iOffset;
			break;

		case cui_raw::toggleUp:
			/*
			** button is offset, meaning the bottom is 2 pixels above the client specified RECT
			** all other sides are against the borders of the client specified RECT
			*/
			rect.bottom += Control.iOffset;
			break;

		case cui_raw::toggleDown:
			rect.bottom -= Control.iOffset;
			break;

		default:
			/*
			** button is offset, meaning the bottom is 2 pixels above the client specified RECT
			** all other sides are against the borders of the client specified RECT
			*/
			rect.bottom += Control.iOffset;
			break;
		}


	}
	else
	{
		switch (Control.toggleAction)
		{
		case cui_raw::toggleLeft:
			rect.left -= Control.iOffset;
			break;

		case cui_raw::toggleRight:
			rect.left += Control.iOffset;
			break;

		case cui_raw::toggleUp:
			/*
			** button is NOT offset, meaning the top is 2 pixels below the client specified RECT
			** all other sides are against the borders of the client specified RECT
			*/
			rect.top -= Control.iOffset;
			break;

		case cui_raw::toggleDown:
			rect.top += Control.iOffset;
			break;

		default:
			/*
			** button is NOT offset, meaning the top is 2 pixels below the client specified RECT
			** all other sides are against the borders of the client specified RECT
			*/
			rect.top -= Control.iOffset;
			break;
		}
	}

	POINT m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rect.left += m_pt.x;
	rect.right += m_pt.x;
	rect.top += m_pt.y;
	rect.bottom += m_pt.y;

	bool bControlAvailable = false;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
		bControlAvailable = true;

	// compute active region
	RECT rcActive = rect;
	int iWidthDiff = (rect.right - rect.left) - (Control.rcActive.right - Control.rcActive.left);

	switch (Control.textPlacement)
	{
	case cui_raw::bottom:
	case cui_raw::top:
		break;
	case cui_raw::right:
	case cui_raw::righttop:
	case cui_raw::rightbottom:
		rcActive.right -= iWidthDiff;
		break;
	case cui_raw::left:
	case cui_raw::lefttop:
	case cui_raw::leftbottom:
		rcActive.left += iWidthDiff;
		break;
	default:
		break;
	}

	if (bControlAvailable)
		m_vHotRects.push_back(rcActive);

	if (bControlAvailable && PtInRect(&rcActive, pt))
	{
		// mouse is over a button
		if (!Control.bHot)
		{
			Control.bHot = true;

			ShowToolTip(Control.toolTip);

			if (Control.bButtonBar)
			{
				InvalidateRect(Control.hWnd, &Control.rcBar, TRUE);
				UpdateWindow(Control.hWnd);
			}

			int iRefreshWidth = 1;

			if (!Control.bChangeColor && (Control.clrBackground == Control.clrBackgroundHot))
			{
				RECT rectButton;
				GetClientRect(Control.hWnd, &rectButton);
				RECT rc = rectButton;
				rc.right = rc.left + iRefreshWidth;
				InvalidateRect(Control.hWnd, &rc, TRUE);
				UpdateWindow(Control.hWnd);

				rc = rectButton;
				rc.bottom = rc.top + iRefreshWidth;
				InvalidateRect(Control.hWnd, &rc, TRUE);
				UpdateWindow(Control.hWnd);

				rc = rectButton;
				rc.left = rc.right - iRefreshWidth;
				InvalidateRect(Control.hWnd, &rc, TRUE);
				UpdateWindow(Control.hWnd);

				rc = rectButton;
				rc.top = rc.bottom - iRefreshWidth;
				InvalidateRect(Control.hWnd, &rc, TRUE);
				UpdateWindow(Control.hWnd);
			}
			else
			{
				InvalidateRect(Control.hWnd, NULL, TRUE);
				UpdateWindow(Control.hWnd);
			}
		}

		// offset button position
		if (!Control.bOffset && Control.bToggle)
		{
			// offset button
			SetWindowPos(Control.hWnd, NULL, rect.left, rect.top, 0, 0, SWP_NOSIZE);
			Control.bOffset = true;
		}
	}
	else
	{
		HideToolTip(Control.toolTip);

		// reset buttons
		if (Control.bHot)
		{
			Control.bHot = false;

			if (Control.bButtonBar)
			{
				InvalidateRect(Control.hWnd, &Control.rcBar, TRUE);
				UpdateWindow(Control.hWnd);
			}

			int iRefreshWidth = 1;

			if (!Control.bChangeColor && (Control.clrBackground == Control.clrBackgroundHot))
			{
				RECT rectButton;
				GetClientRect(Control.hWnd, &rectButton);
				RECT rc = rectButton;
				rc.right = rc.left + iRefreshWidth;
				InvalidateRect(Control.hWnd, &rc, TRUE);
				UpdateWindow(Control.hWnd);

				rc = rectButton;
				rc.bottom = rc.top + iRefreshWidth;
				InvalidateRect(Control.hWnd, &rc, TRUE);
				UpdateWindow(Control.hWnd);

				rc = rectButton;
				rc.left = rc.right - iRefreshWidth;
				InvalidateRect(Control.hWnd, &rc, TRUE);
				UpdateWindow(Control.hWnd);

				rc = rectButton;
				rc.top = rc.bottom - iRefreshWidth;
				InvalidateRect(Control.hWnd, &rc, TRUE);
				UpdateWindow(Control.hWnd);
			}
			else
			{
				InvalidateRect(Control.hWnd, NULL, TRUE);
				UpdateWindow(Control.hWnd);
			}
		}

		// reset button position
		if (Control.bOffset && Control.bToggle)
		{
			switch (Control.toggleAction)
			{
			case cui_raw::toggleLeft:
				SetWindowPos(Control.hWnd, NULL, rect.left + Control.iOffset, rect.top, 0, 0, SWP_NOSIZE);
				break;

			case cui_raw::toggleRight:
				SetWindowPos(Control.hWnd, NULL, rect.left - Control.iOffset, rect.top, 0, 0, SWP_NOSIZE);
				break;

			case cui_raw::toggleUp:
				SetWindowPos(Control.hWnd, NULL, rect.left, rect.top + Control.iOffset, 0, 0, SWP_NOSIZE);
				break;

			case cui_raw::toggleDown:
				SetWindowPos(Control.hWnd, NULL, rect.left, rect.top - Control.iOffset, 0, 0, SWP_NOSIZE);
				break;

			default:
				SetWindowPos(Control.hWnd, NULL, rect.left, rect.top + Control.iOffset, 0, 0, SWP_NOSIZE);
				break;
			}

			Control.bOffset = false;
		}
	}
} // hitImageControl

void cui_rawImpl::hitSelectorControl(cui_rawImpl::SelectorControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects)
{
	Control.toolTip.hWndControl = Control.hWnd;

	RECT rcSelector = Control.rcSelector;

	POINT m_pt = { 0, 0 };
	ClientToScreen(Control.hWnd, &m_pt);

	rcSelector.left += m_pt.x;
	rcSelector.right += m_pt.x;
	rcSelector.top += m_pt.y;
	rcSelector.bottom += m_pt.y;

	m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rcSelector.left += m_pt.x;
	rcSelector.right += m_pt.x;
	rcSelector.top += m_pt.y;
	rcSelector.bottom += m_pt.y;

	bool bControlAvailable = false;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
		bControlAvailable = true;

	RECT m_Hot = rcSelector;
	m_Hot.top -= Control.yUpper;
	m_Hot.bottom += Control.yLower;

	if (bControlAvailable)
		m_vHotRects.push_back(m_Hot);

	if (bControlAvailable && PtInRect(&m_Hot, pt))
	{
		// mouse is over a button
		if (!Control.bHot)
		{
			ShowToolTip(Control.toolTip);
			Control.bHot = true;
		}
	}
	else
	{
		HideToolTip(Control.toolTip);

		// reset buttons
		if (Control.bHot)
			Control.bHot = false;
	}

	if (bControlAvailable && Control.bPressed)
	{
		if (Control.d->m_bLButtonDown)
		{
			int iPerc = 0;

			RECT rc = rcSelector;

			double dPerc = 100 * (double(pt.y) - double(rc.top)) /
				(double(rc.bottom) - double(rc.top));

			if (dPerc <= 0)
				iPerc = 0;
			else
			{
				if (dPerc >= 100)
					iPerc = 100;
				else
					iPerc = int(dPerc + 0.5);
			}

			Control.iPercV = iPerc;

			InvalidateRect(Control.hWnd, NULL, FALSE);
		}
	}
} // hitSelectorControl

void cui_rawImpl::hitBarChartControl(cui_rawImpl::BarChartControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects)
{
	Control.toolTip.hWndControl = Control.hWnd;

	bool bControlAvailable = false;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
		bControlAvailable = true;

	bool bInCaptions = false;
	bool bUpdate = false;

	for (auto &it : Control.chartBarsInfo)
	{
		it.sTooltip = it.sChartInfo;

		// check if point is over a label
		RECT rect = it.rcLabel;

		POINT m_pt = { 0, 0 };
		ClientToScreen(Control.hWnd, &m_pt);

		rect.left += m_pt.x;
		rect.right += m_pt.x;
		rect.top += m_pt.y;
		rect.bottom += m_pt.y;

		m_pt = { 0, 0 };
		ScreenToClient(GetParent(Control.hWnd), &m_pt);

		rect.left += m_pt.x;
		rect.right += m_pt.x;
		rect.top += m_pt.y;
		rect.bottom += m_pt.y;

		if (bControlAvailable)
			m_vHotRects.push_back(rect);

		if (PtInRect(&rect, pt) && bControlAvailable)
			bInCaptions = true;

		if (bControlAvailable && PtInRect(&rect, pt))
		{
			// mouse is over a slice
			if (!it.bHot)
			{
				if (Control.toolTip.sText != it.sTooltip)
				{
					Control.toolTip.sText = it.sTooltip;
					HideToolTip(Control.toolTip);
					ShowToolTip(Control.toolTip);
				}

				it.bHot = true;
				bUpdate = true;
			}
		}
		else
		{
			// reset buttons
			if (it.bHot)
			{
				it.bHot = false;
				bUpdate = true;
			}
		}
	}

	bool bHotLineFound = false;

	if (!bInCaptions)
	{
		for (auto &it : Control.chartBarsInfo)
		{
			it.bHot = false;

			Control.toolTip.hWndControl = Control.hWnd;

			RECT rect = it.rect;

			POINT m_pt = { 0, 0 };
			ClientToScreen(Control.hWnd, &m_pt);

			rect.left += m_pt.x;
			rect.right += m_pt.x;
			rect.top += m_pt.y;
			rect.bottom += m_pt.y;

			m_pt = { 0, 0 };
			ScreenToClient(GetParent(Control.hWnd), &m_pt);

			rect.left += m_pt.x;
			rect.right += m_pt.x;
			rect.top += m_pt.y;
			rect.bottom += m_pt.y;

			if (bControlAvailable)
				m_vHotRects.push_back(rect);

			if (bControlAvailable && PtInRect(&rect, pt) && !bHotLineFound)
			{
				bHotLineFound = true;

				// mouse is over a line
				if (!it.bHot)
				{
					if (Control.toolTip.sText != it.sChartInfo)
					{
						Control.toolTip.sText = it.sChartInfo;
						HideToolTip(Control.toolTip);
						ShowToolTip(Control.toolTip);
					}

					it.bHot = true;
					RECT rc = it.rect;
					bUpdate = true;
				}
			}
			else
			{
				// reset buttons
				if (it.bHot)
				{
					if (Control.toolTip.sText == it.sChartInfo)
					{
						HideToolTip(Control.toolTip);
						Control.toolTip.sText.clear();
					}

					it.bHot = false;
					RECT rc = it.rect;
					bUpdate = true;
				}
			}
		}
	}

	if (!bInCaptions && !bHotLineFound)
	{
		// reset tooltip
		if (!Control.toolTip.sText.empty())
		{
			HideToolTip(Control.toolTip);
			Control.toolTip.sText.clear();
		}
	}

	if (bUpdate)
	{
		InvalidateRect(Control.hWnd, NULL, FALSE);
		UpdateWindow(Control.hWnd);
	}
} // hitBarChartControl

void cui_rawImpl::hitLineChartControl(cui_rawImpl::LineChartControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects)
{
	Control.toolTip.hWndControl = Control.hWnd;

	bool bControlAvailable = false;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
		bControlAvailable = true;

	bool bInCaptions = false;
	bool bUpdate = false;

	for (auto &it : Control.linesInfo)
	{
		it.sTooltip = it.sSeriesName;

		// check if point is over a label
		RECT rect = it.rcLabel;

		POINT m_pt = { 0, 0 };
		ClientToScreen(Control.hWnd, &m_pt);

		rect.left += m_pt.x;
		rect.right += m_pt.x;
		rect.top += m_pt.y;
		rect.bottom += m_pt.y;

		m_pt = { 0, 0 };
		ScreenToClient(GetParent(Control.hWnd), &m_pt);

		rect.left += m_pt.x;
		rect.right += m_pt.x;
		rect.top += m_pt.y;
		rect.bottom += m_pt.y;

		if (bControlAvailable)
			m_vHotRects.push_back(rect);

		if (PtInRect(&rect, pt))
			bInCaptions = true;

		if (bControlAvailable && PtInRect(&rect, pt))
		{
			// mouse is over a slice
			if (!it.bHot)
			{
				if (Control.toolTip.sText != it.sTooltip)
				{
					Control.toolTip.sText = it.sTooltip;
					HideToolTip(Control.toolTip);
					ShowToolTip(Control.toolTip);
				}

				it.bHot = true;
				bUpdate = true;
			}
		}
		else
		{
			// reset buttons
			if (it.bHot)
			{
				if (Control.toolTip.sText == it.sTooltip)
				{
					HideToolTip(Control.toolTip);
					Control.toolTip.sText.clear();
				}

				it.bHot = false;
				bUpdate = true;
			}
		}
	}

	if (bUpdate)
	{
		for (auto &it : Control.linesInfo)
		{
			for (size_t i = 0; i < it.chartLinesInfo.size(); i++)
			{
				it.chartLinesInfo[i].bHot = false;
			}
		}
	}

	if (!bInCaptions)
	{
		bool bHotLineFound = false;

		for (auto &it : Control.linesInfo)
		{
			it.bHot = false;

			for (size_t i = 0; i < it.chartLinesInfo.size(); i++)
			{
				Control.toolTip.hWndControl = Control.hWnd;

				RECT rect = it.chartLinesInfo[i].rect;

				POINT m_pt = { 0, 0 };
				ClientToScreen(Control.hWnd, &m_pt);

				rect.left += m_pt.x;
				rect.right += m_pt.x;
				rect.top += m_pt.y;
				rect.bottom += m_pt.y;

				m_pt = { 0, 0 };
				ScreenToClient(GetParent(Control.hWnd), &m_pt);

				rect.left += m_pt.x;
				rect.right += m_pt.x;
				rect.top += m_pt.y;
				rect.bottom += m_pt.y;

				if (bControlAvailable)
					m_vHotRects.push_back(rect);

				if (bControlAvailable && PtInRect(&rect, pt) && !bHotLineFound)
				{
					it.bHot = true;
					bHotLineFound = true;

					// mouse is over a line
					if (!it.chartLinesInfo[i].bHot)
					{
						if (Control.toolTip.sText != it.chartLinesInfo[i].sChartInfo)
						{
							Control.toolTip.sText = it.chartLinesInfo[i].sChartInfo;
							HideToolTip(Control.toolTip);
							ShowToolTip(Control.toolTip);
						}

						it.chartLinesInfo[i].bHot = true;
						RECT rc = it.chartLinesInfo[i].rect;
						bUpdate = true;
					}
				}
				else
				{
					// reset buttons
					if (it.chartLinesInfo[i].bHot)
					{
						if (Control.toolTip.sText == it.chartLinesInfo[i].sChartInfo)
						{
							HideToolTip(Control.toolTip);
							Control.toolTip.sText.clear();
						}

						it.chartLinesInfo[i].bHot = false;
						RECT rc = it.chartLinesInfo[i].rect;
						bUpdate = true;
					}
				}
			}
		}

		if (!bHotLineFound)
		{
			HideToolTip(Control.toolTip);
			Control.toolTip.sText.clear();
		}
	} // if (!bInCaptions)

	if (bUpdate)
	{
		InvalidateRect(Control.hWnd, NULL, FALSE);
		UpdateWindow(Control.hWnd);
	}
} // hitLineChartControl

void cui_rawImpl::hitPieChartControl(cui_rawImpl::PieChartControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects)
{
	Control.toolTip.hWndControl = Control.hWnd;

	bool bControlAvailable = false;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
		bControlAvailable = true;

	POINT m_pt = pt;

	// convert coordinates from parent window to screen
	ClientToScreen(GetParent(Control.hWnd), &m_pt);

	// convert coordinates from screen to control
	ScreenToClient(Control.hWnd, &m_pt);

	// convert to Gdiplus::Point
	Gdiplus::Point point(m_pt.x, m_pt.y);

	// make a graphics object from the control's HWND
	Gdiplus::Graphics g(Control.hWnd);

	bool bUpdate = false;

	bool bInPieChart = false;

	for (size_t i = 0; i < Control.chartBarsInfo.size(); i++)
	{
		Control.toolTip.hWndControl = Control.hWnd;

		Gdiplus::Region *pRegion = Control.chartBarsInfo[i].pRegion;

		// check if the point is within the region
		bool bInRegion = false;
		if (pRegion->IsVisible(point, &g))
		{
			// The point is in the region
			bInRegion = true;
			bInPieChart = true;
		}
		else
		{
			// The point is not in the region
			bInRegion = false;
		}

		if (!bInRegion)
		{
			// check if point is over a label
			RECT rect = Control.chartBarsInfo[i].rcLabel;

			POINT m_pt = { 0, 0 };
			ClientToScreen(Control.hWnd, &m_pt);

			rect.left += m_pt.x;
			rect.right += m_pt.x;
			rect.top += m_pt.y;
			rect.bottom += m_pt.y;

			m_pt = { 0, 0 };
			ScreenToClient(GetParent(Control.hWnd), &m_pt);

			rect.left += m_pt.x;
			rect.right += m_pt.x;
			rect.top += m_pt.y;
			rect.bottom += m_pt.y;

			if (bControlAvailable)
				m_vHotRects.push_back(rect);

			if (PtInRect(&rect, pt))
				bInRegion = true;
		}

		if (bControlAvailable && bInRegion)
		{
			// mouse is over a slice
			if (!Control.chartBarsInfo[i].bHot)
			{
				if (Control.toolTip.sText != Control.chartBarsInfo[i].sTooltip)
				{
					Control.toolTip.sText = Control.chartBarsInfo[i].sTooltip;
					HideToolTip(Control.toolTip);
					ShowToolTip(Control.toolTip);
				}

				Control.chartBarsInfo[i].bHot = true;
				bUpdate = true;
			}
		}
		else
		{
			// reset buttons
			if (Control.chartBarsInfo[i].bHot)
			{
				if (Control.toolTip.sText == Control.chartBarsInfo[i].sItemLabel)
				{
					HideToolTip(Control.toolTip);
					Control.toolTip.sText.clear();
				}

				Control.chartBarsInfo[i].bHot = false;
				bUpdate = true;
			}
		}
	}

	if (bUpdate)
	{
		InvalidateRect(Control.hWnd, NULL, FALSE);
		UpdateWindow(Control.hWnd);
	}

	if (!bInPieChart)
	{
		Control.toolTip.hWndControl = Control.hWnd;

		RECT rcPieChart = Control.rcPieChart;

		POINT m_pt = { 0, 0 };
		ClientToScreen(Control.hWnd, &m_pt);

		rcPieChart.left += m_pt.x;
		rcPieChart.right += m_pt.x;
		rcPieChart.top += m_pt.y;
		rcPieChart.bottom += m_pt.y;

		m_pt = { 0, 0 };
		ScreenToClient(GetParent(Control.hWnd), &m_pt);

		rcPieChart.left += m_pt.x;
		rcPieChart.right += m_pt.x;
		rcPieChart.top += m_pt.y;
		rcPieChart.bottom += m_pt.y;

		bool bControlAvailable = false;

		if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
			bControlAvailable = true;

		if (bControlAvailable)
			m_vHotRects.push_back(rcPieChart);

		if (bControlAvailable && PtInRect(&rcPieChart, pt))
		{
			// mouse is over a pie chart control, but is outside the actual pie
			if (!Control.bHot)
				Control.bHot = true;

			if (Control.toolTip.sText != Control.sChartName)
			{
				Control.toolTip.sText = Control.sChartName;
				ShowToolTip(Control.toolTip);
			}
		}
		else
		{
			HideToolTip(Control.toolTip);

			// reset buttons
			if (Control.bHot)
				Control.bHot = false;
		}
	}
} // hitPieChartControl

void cui_rawImpl::hitTextControl(cui_rawImpl::TextControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects)
{
	Control.toolTip.hWndControl = Control.hWnd;

	RECT rect = Control.rcText;
	POINT m_pt = { 0, 0 };
	ClientToScreen(Control.hWnd, &m_pt);

	rect.left += m_pt.x;
	rect.right += m_pt.x;
	rect.top += m_pt.y;
	rect.bottom += m_pt.y;

	m_pt = { 0, 0 };
	ScreenToClient(GetParent(Control.hWnd), &m_pt);

	rect.left += m_pt.x;
	rect.right += m_pt.x;
	rect.top += m_pt.y;
	rect.bottom += m_pt.y;

	bool bControlAvailable = false;

	if (IsWindowEnabled(Control.hWnd) && IsWindowVisible(Control.hWnd))
	{
		bControlAvailable = true;

		if (!Control.bStatic)
			m_vHotRects.push_back(rect);
	}

	if (PtInRect(&rect, pt))
	{
		// mouse is over text
		if (Control.bHot == false)
		{
			if (bControlAvailable)
			{
				Control.bHot = true;
				ShowToolTip(Control.toolTip);

				if (!Control.bStatic)
				{
					InvalidateRect(Control.hWnd, NULL, FALSE);
					UpdateWindow(Control.hWnd);
				}
			}
		}
	}
	else
	{
		HideToolTip(Control.toolTip);

		// reset buttons
		if (Control.bHot == true)
		{
			if (bControlAvailable)
			{
				Control.bHot = false;

				if (!Control.bStatic)
				{
					InvalidateRect(Control.hWnd, NULL, FALSE);
					UpdateWindow(Control.hWnd);
				}
			}
		}
	}
} // hitTextControl

void cui_rawImpl::OnWM_MOUSEMOVE(HWND hWnd, LPARAM lParam, cui_rawImpl* d)
{
	if (d->m_iTimer > 0 && d->m_bTimerRunning && d->m_bStopOnMouseOverWindow)
	{
		// set timer running flag to false
		d->m_bTimerRunning = false;

		// stop the timer
		KillTimer(hWnd, d->ID_TIMER);
	}

	// track the mouse
	d->m_pMouseTrack->OnMouseMove(hWnd);

	POINT pt;
	pt.x = 0;
	pt.y = 0;

	pt.x = GET_X_LPARAM(lParam);
	pt.y = GET_Y_LPARAM(lParam);

	// rects that contain interactive UI elements
	std::vector<RECT> m_vHotRects;

	try
	{
		// check if there is tab control in this page
		if (d->m_Pages.at(d->m_sCurrentPage).m_TabControl.iUniqueID != -123)	// TO-DO: remove magic number
		{
			for (size_t i = 0; i < d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs.size(); i++)
			{
				d->m_Pages.at(d->m_sCurrentPage).m_TabControl.toolTip.hWndControl = d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd;

				RECT rect = d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].rcTab;

				POINT m_pt = { 0, 0 };
				ClientToScreen(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd, &m_pt);

				rect.left += m_pt.x;
				rect.right += m_pt.x;
				rect.top += m_pt.y;
				rect.bottom += m_pt.y;

				m_pt = { 0, 0 };
				ScreenToClient(hWnd, &m_pt);

				rect.left += m_pt.x;
				rect.right += m_pt.x;
				rect.top += m_pt.y;
				rect.bottom += m_pt.y;

				bool bControlAvailable = false;

				if (IsWindowEnabled(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd) && IsWindowVisible(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd))
					bControlAvailable = true;

				if (bControlAvailable && PtInRect(&rect, pt))
				{
					// mouse is over a tab
					if (!d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bHot)
					{
						if (d->m_Pages.at(d->m_sCurrentPage).m_TabControl.toolTip.sText != d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].sTooltip)
						{
							d->m_Pages.at(d->m_sCurrentPage).m_TabControl.toolTip.sText = d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].sTooltip;
							HideToolTip(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.toolTip);
							ShowToolTip(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.toolTip);
						}

						// only if not already selected
						if (!d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bSelected)
						{
							d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bHot = true;
							RECT rc = d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].rcTab;
							InflateRect(&rc, 5, 5);
							InvalidateRect(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd, &rc, TRUE);
							UpdateWindow(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd);
						}
					}
				}
				else
				{
					if (d->m_Pages.at(d->m_sCurrentPage).m_TabControl.toolTip.sText == d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].sTooltip)
					{
						HideToolTip(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.toolTip);
						d->m_Pages.at(d->m_sCurrentPage).m_TabControl.toolTip.sText.clear();
					}

					// reset buttons
					if (d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bHot)
					{
						d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bHot = false;
						RECT rc = d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].rcTab;
						InflateRect(&rc, 5, 5);
						InvalidateRect(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd, &rc, TRUE);
						UpdateWindow(d->m_Pages.at(d->m_sCurrentPage).m_TabControl.hWnd);
					}
				}

				if (bControlAvailable && !d->m_Pages.at(d->m_sCurrentPage).m_TabControl.vTabs[i].bSelected)
					m_vHotRects.push_back(rect);
			}
		}
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's bar charts
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_BarChartControls)
			d->hitBarChartControl(it.second, pt, m_vHotRects);

		// handle pageless bar chart controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_BarChartControls)
				if (it.second.bPageLess)
					d->hitBarChartControl(it.second, pt, m_vHotRects);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's line charts
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_LineChartControls)
			d->hitLineChartControl(it.second, pt, m_vHotRects);

		// handle pageless line chart controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_LineChartControls)
				if (it.second.bPageLess)
					d->hitLineChartControl(it.second, pt, m_vHotRects);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's pie charts charts
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_PieChartControls)
			d->hitPieChartControl(it.second, pt, m_vHotRects);

		// handle pageless pie chart controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_PieChartControls)
				if (it.second.bPageLess)
					d->hitPieChartControl(it.second, pt, m_vHotRects);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's buttons
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_ButtonControls)
			d->hitButtonControl(it.second, pt, m_vHotRects);

		// handle pageless button controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_ButtonControls)
				if (it.second.bPageLess)
					d->hitButtonControl(it.second, pt, m_vHotRects);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's toggle buttons
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_ToggleButtonControls)
			d->hitToggleButtonControl(it.second, pt, m_vHotRects);

		// handle pageless toggle buttons
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_ToggleButtonControls)
				if (it.second.bPageLess)
					d->hitToggleButtonControl(it.second, pt, m_vHotRects);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current star rating controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_StarRatingControls)
			d->hitStarRatingControl(it.second, pt, m_vHotRects);

		// handle pageless star rating controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_StarRatingControls)
				if (it.second.bPageLess)
					d->hitStarRatingControl(it.second, pt, m_vHotRects);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's selector controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_SelectorControls)
			d->hitSelectorControl(it.second, pt, m_vHotRects);

		// handle pageless selector controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_SelectorControls)
				if (it.second.bPageLess)
					d->hitSelectorControl(it.second, pt, m_vHotRects);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's image controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_ImageControls)
			d->hitImageControl(it.second, pt, m_vHotRects);

		// handle pageless image controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_ImageControls)
				if (it.second.bPageLess)
					d->hitImageControl(it.second, pt, m_vHotRects);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle current page's text controls
		for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_TextControls)
			d->hitTextControl(it.second, pt, m_vHotRects);

		// handle pageless text controls
		if (d->m_sCurrentPage != d->m_sTitle)
			for (auto &it : d->m_Pages.at(d->m_sTitle).m_TextControls)
				if (it.second.bPageLess)
					d->hitTextControl(it.second, pt, m_vHotRects);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	try
	{
		// handle control button controls
		for (auto &it : d->m_ControlBtns)
			d->hitControlButton(it.second, pt, m_vHotRects);
	}
	catch (std::exception &e)
	{
		// do nothing ... map probably out of range
		std::string m_sErr = e.what();
	}

	bool bSetCursorToHand = false;

	for (size_t i = 0; i < m_vHotRects.size(); i++)
	{
		if (PtInRect(&m_vHotRects[i], pt))
		{
			bSetCursorToHand = true;
			break;
		}
	}

	// change cursor, if necessary ...
	if (bSetCursorToHand)
	{
		if (GetCursor() != d->m_hHotCursor)
			SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR)d->m_hHotCursor);
	}
	else
	{
		if (GetCursor() != d->m_hNormalCursor)
			SetClassLongPtr(hWnd, GCLP_HCURSOR, (LONG_PTR)d->m_hNormalCursor);
	}
} // OnWM_MOUSEMOVE

LRESULT cui_rawImpl::OnWM_NOTIFY(HWND hWnd, LPARAM lParam, cui_rawImpl* d)
{
	NMLISTVIEW *Val_notify = (NMLISTVIEW*)lParam;

	NMHDR* pHdr = (NMHDR*)lParam;

	try
	{
		switch (pHdr->code)
		{
		case DTN_DATETIMECHANGE:
		{
			// get date control handle
			HWND hwndDT = Val_notify->hdr.hwndFrom;

			if (IsWindow(hwndDT))
			{
				// find the date control
				if (d->m_Pages.at(d->m_sCurrentPage).m_DateControls.find(Val_notify->hdr.idFrom) != d->m_Pages.at(d->m_sCurrentPage).m_DateControls.end())
				{
					cui_rawImpl::DateControl *pControl = &d->m_Pages.at(d->m_sCurrentPage).m_DateControls.at(Val_notify->hdr.idFrom);

					// check if date control is not already busy
					if (!pControl->bBusy)
					{
						// set the busy flag to true to prevent simultaneous calls
						pControl->bBusy = true;

						// WM_COMMAND to indicate date has changed
						SendMessage(hWnd, WM_COMMAND, (WPARAM)Val_notify->hdr.idFrom, NULL);

						// set busy flag to false
						pControl->bBusy = false;
					}
					else
					{
						// TO-DO: find a way to
						// 1. check which item was selected when date entered "busy" state
						// 2. force reselection of this item when we get here so that there isn't inconsistency in ui
					}

				} // if

				  // find the time control
				if (d->m_Pages.at(d->m_sCurrentPage).m_TimeControls.find(Val_notify->hdr.idFrom) != d->m_Pages.at(d->m_sCurrentPage).m_TimeControls.end())
				{
					cui_rawImpl::TimeControl *pControl = &d->m_Pages.at(d->m_sCurrentPage).m_TimeControls.at(Val_notify->hdr.idFrom);

					// check if time control is not already busy
					if (!pControl->bBusy)
					{
						// set the busy flag to true to prevent simultaneous calls
						pControl->bBusy = true;

						// WM_COMMAND to indicate time has changed
						SendMessage(hWnd, WM_COMMAND, (WPARAM)Val_notify->hdr.idFrom, NULL);

						// set busy flag to false
						pControl->bBusy = false;
					}
					else
					{
						// TO-DO: find a way to
						// 1. check which item was selected when time entered "busy" state
						// 2. force reselection of this item when we get here so that there isn't inconsistency in ui
					}

				} // if

			} // if
		}
		break; // DTN_DATETIMECHANGE

		case NM_DBLCLK:
		{
			// check if listview item was double clicked
			try
			{
				if (d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.find(Val_notify->hdr.idFrom) != d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.end())
				{
					// set double click flag to true
					cui_rawImpl::listviewControl *pControl = &d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(Val_notify->hdr.idFrom);
					pControl->bDoubleClicked = true;

					// check if listview is not already busy
					if (!pControl->bBusy)
					{
						// set the busy flag to true to prevent simultaneous calls
						pControl->bBusy = true;

						// WM_COMMAND to indicate list view item was double clicked
						SendMessage(hWnd, WM_COMMAND, (WPARAM)Val_notify->hdr.idFrom, NULL);

						// set busy flag to false
						pControl->bBusy = false;
					}
					else
					{
						// TO-DO: find a way to
						// 1. check which item was selected when listview entered "busy" state
						// 2. force reselection of this item when we get here so that there isn't inconsistency in ui
					}
				}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}
		}
		break; // NM_DBLCLK

		case NM_CUSTOMDRAW:
		{
			try
			{
				if (d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.find(Val_notify->hdr.idFrom) != d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.end())
					return HandleCustomDraw((NMLVCUSTOMDRAW*)pHdr, pHdr->hwndFrom, d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(Val_notify->hdr.idFrom).vData, d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(Val_notify->hdr.idFrom).vColumns);
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}
		}
		break; // NM_CUSTOMDRAW

		case LVN_COLUMNCLICK:
		{
			// check for listview column click notification
			try
			{
				if (d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.find(Val_notify->hdr.idFrom) != d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.end())
				{
					int i = Val_notify->hdr.idFrom;

					if (d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(i).bSortByClickingColumn)
					{
						// sort column
						LPNMLISTVIEW pLVInfo = Val_notify;

						LPARAM lParamSort;

						/*
						** get new sort parameters
						*/
						if (pLVInfo->iSubItem == d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(i).nSortColumn)
							d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(i).bSortAscending = !d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(i).bSortAscending;
						else
						{
							d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(i).nSortColumn = pLVInfo->iSubItem;
							d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(i).bSortAscending = TRUE;
						}

						/*
						** combine sort info into a single value we can send to out sort function
						*/
						lParamSort = 1 + (LPARAM)d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(i).nSortColumn;
						if (!d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(i).bSortAscending)
							lParamSort = -lParamSort;

						sortstruct Sort;
						Sort.lParamSort = lParamSort;
						Sort.hlistview = pLVInfo->hdr.hwndFrom;

						/*
						** sort list
						*/
						ListView_SortItemsEx(pLVInfo->hdr.hwndFrom, CompareListItems, (LPARAM)&Sort);

						// change listview index numbers
						if (d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.find(Val_notify->hdr.idFrom) != d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.end())
						{
							// set double click flag to true
							cui_rawImpl::listviewControl *pControl = &d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(Val_notify->hdr.idFrom);

							int iUniqueColumnNumber = -1;

							// determine unique column number
							for (size_t iColumnNames = 0; iColumnNames < pControl->vColumns.size(); iColumnNames++)
							{
								if (
									pControl->vColumns[iColumnNames].sColumnName ==
									pControl->sUniqueColumnName)
								{
									iUniqueColumnNumber = pControl->vColumns[iColumnNames].iColumnID;
									break;
								}
							}

							// updated listview data container that will have items in the new order
							std::vector<cui_raw::listviewRow> vData(pControl->vData.size());

							for (auto &row : pControl->vData)
							{
								int iRowNumber = -1;

								// get number of columns
								const int iCols = pControl->pClistview->Get_NumOfCols();

								// get number of rows
								const int iRows = pControl->pClistview->Get_NumOfRows();

								LVITEM lv = { 0 };

								HWND m_hlistview = pControl->hWnd;

								for (int iRow = 0; iRow < iRows; iRow++)
								{
									lv.mask = LVIF_TEXT;
									lv.iItem = iRow;

									bool bMatch = false;

									for (int iCol = 0; iCol < iCols; iCol++)
									{
										if (iCol == iUniqueColumnNumber)
										{
											lv.iSubItem = iCol;

											TCHAR buf[1024];
											ListView_GetItemText(m_hlistview, iRow, iCol, buf, _countof(buf));

											// insert information into matrix
											std::basic_string<TCHAR> sText(buf);

											// check if this text is in our target row
											for (auto &it : row.vItems)
											{
												if (it.sColumnName != pControl->sUniqueColumnName)
													continue;

												if (it.sItemData == sText)
													bMatch = true;
											}
										}
									}

									if (bMatch)
									{
										// determine the row number
										iRowNumber = iRow;

										// update internal row number in listview items
										for (auto &it : row.vItems)
											it.iRowNumber = iRowNumber;

										// insert row in appropriate slot in our updated listview data container
										vData[iRowNumber] = row;

										break;
									}
								}
							} // for (auto &row : pControl->vData)

							  // replace listview data with update lot
							pControl->vData = vData;
						}
					} // if
				} // for
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}
		}
		break; // LVN_COLUMNCLICK

		case LVN_ITEMCHANGED:
		{
			// check for listview selection change
			if (Val_notify->uNewState & LVIS_SELECTED)
			{
				// get listview handle
				HWND hlistview = Val_notify->hdr.hwndFrom;

				if (IsWindow(hlistview))
				{
					// find the listview
					if (d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.find(Val_notify->hdr.idFrom) != d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.end())
					{
						cui_rawImpl::listviewControl *pControl = &d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(Val_notify->hdr.idFrom);

						// check if listview is not already busy
						if (!pControl->bBusy)
						{
							// set the busy flag to true to prevent simultaneous calls
							pControl->bBusy = true;

							// WM_COMMAND to indicate list view selection has changed
							SendMessage(hWnd, WM_COMMAND, (WPARAM)Val_notify->hdr.idFrom, NULL);

							// set busy flag to false
							pControl->bBusy = false;
						}
						else
						{
							// TO-DO: find a way to
							// 1. check which item was selected when listview entered "busy" state
							// 2. force reselection of this item when we get here so that there isn't inconsistency in ui
						}

					} // if

				} // if

			} // if
		}
		break; // LVN_ITEMCHANGED

		default:
			break;
		}
	}
	catch (std::exception &e)
	{
		std::string sErr = e.what();
	}

	return NULL;
} // OnWM_NOTIFY

void cui_rawImpl::OnWM_ERASEBKGND(HWND hWnd, WPARAM wParam, cui_rawImpl* d)
{
	HDC hdc = (HDC)wParam;

	RECT rect;
	GetClientRect(hWnd, &rect);

	CBrush hbr(d->m_clrBackground);
	FillRect(hdc, &rect, hbr.get());
} // OnWM_ERASEBKGND

void cui_rawImpl::OnWM_COMMAND(HWND hWnd, WPARAM wParam, cui_rawImpl* d, cui_raw* pThis)
{
	int iUniqueID = LOWORD(wParam);
	int iMsg = HIWORD(wParam);

	if (iUniqueID == 0 && iMsg == 0)	// failsafe
		return;

	// check if it's some messages from edit controls that we don't need passed to the command procedure
	if (
		iMsg == EN_SETFOCUS ||
		iMsg == EN_KILLFOCUS ||
		iMsg == EN_UPDATE ||
		iMsg == EN_ERRSPACE ||
		iMsg == EN_MAXTEXT ||
		iMsg == EN_HSCROLL ||
		iMsg == EN_VSCROLL
		)
		return;

	// identify control
	enum MyEnum
	{
		timer,
		buttonControl,
		textControl,
		imageControl,
		comboControl,
		listviewControl,
		listviewPopupMenuItem,
		toggleButtonControl,
		editControl,
		selectorControl,
		trayIconMenuItem,
		closeButton,
		copyData,
		dropFile,
		starRatingControl,
		pieChartControl,
		dateControl,
		timeControl,
	} thisControl = (MyEnum)-1;

	HWND hWndItem = NULL;
	bool bPageLessControl = false;

	try
	{
		bool bDoubleClicked = false;
		Clistview* pClistview = NULL;
		std::vector<cui_raw::listviewColumn> vlistviewColumns;
		bool blistviewItemDoubleClicked = false;
		bool bToggleBtnOn = false;
		cui_raw::contextMenuItem listviewcontextmenuitem;

		do
		{
			// check if it's a timer
			try
			{
				// check current page's button controls
				if (d->m_Timers.find(iUniqueID) != d->m_Timers.end())
				{
					thisControl = MyEnum::timer;
					bPageLessControl = true;
					break;
				}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}

			// check if it's a tray icon menu item
			try
			{
				// check tray icon menu items
				for (auto &it : d->m_trayIcon.m_trayItems)
				{
					thisControl = MyEnum::trayIconMenuItem;
					bPageLessControl = true;
					break;
				}
			}
			catch (std::exception &e)
			{
				// do nothing ... vector probably out of range
				std::string m_sErr = e.what();
			}

			// check if it's a close button
			if (iUniqueID == d->m_iIDClose && d->m_iStopQuit <= 0)
			{
				thisControl = MyEnum::closeButton;
				bPageLessControl = true;
				break;
			}

			// check if it's a copy data message
			if (iUniqueID == d->m_iCopyDataID)
			{
				thisControl = MyEnum::copyData;
				bPageLessControl = true;
				break;
			}

			// check if it's a drop file message
			if (iUniqueID == d->m_iDropFilesID)
			{
				thisControl = MyEnum::dropFile;
				bPageLessControl = true;
				break;
			}

			// check if it's a button control
			try
			{
				// check current page's button controls
				if (d->m_Pages.at(d->m_sCurrentPage).m_ButtonControls.find(iUniqueID) != d->m_Pages.at(d->m_sCurrentPage).m_ButtonControls.end())
				{
					thisControl = MyEnum::buttonControl;
					break;
				}

				// check pageless controls
				if (d->m_sCurrentPage != d->m_sTitle)
					if (d->m_Pages.at(d->m_sTitle).m_ButtonControls.find(iUniqueID) != d->m_Pages.at(d->m_sTitle).m_ButtonControls.end())
						if (d->m_Pages.at(d->m_sTitle).m_ButtonControls.at(iUniqueID).bPageLess)
						{
							thisControl = MyEnum::buttonControl;
							bPageLessControl = true;
							break;
						}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}

			// check if it's a pie chart control
			try
			{
				// check current page's pie chart controls
				if (d->m_Pages.at(d->m_sCurrentPage).m_PieChartControls.find(iUniqueID) != d->m_Pages.at(d->m_sCurrentPage).m_PieChartControls.end())
				{
					thisControl = MyEnum::pieChartControl;
					break;
				}

				// check pageless controls
				if (d->m_sCurrentPage != d->m_sTitle)
					if (d->m_Pages.at(d->m_sTitle).m_PieChartControls.find(iUniqueID) != d->m_Pages.at(d->m_sTitle).m_PieChartControls.end())
						if (d->m_Pages.at(d->m_sTitle).m_PieChartControls.at(iUniqueID).bPageLess)
						{
							thisControl = MyEnum::pieChartControl;
							bPageLessControl = true;
							break;
						}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}

			// check if it's an image control
			try
			{
				// check current page's image controls
				if (d->m_Pages.at(d->m_sCurrentPage).m_ImageControls.find(iUniqueID) != d->m_Pages.at(d->m_sCurrentPage).m_ImageControls.end())
				{
					thisControl = MyEnum::buttonControl;
					break;
				}

				// check pageless image controls
				if (d->m_sCurrentPage != d->m_sTitle)
					if (d->m_Pages.at(d->m_sTitle).m_ImageControls.find(iUniqueID) != d->m_Pages.at(d->m_sTitle).m_ImageControls.end())
						if (d->m_Pages.at(d->m_sTitle).m_ImageControls.at(iUniqueID).bPageLess)
						{
							thisControl = MyEnum::buttonControl;
							bPageLessControl = true;
							break;
						}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}

			// check if it's a text control
			try
			{
				// check current page's text controls
				if (d->m_Pages.at(d->m_sCurrentPage).m_TextControls.find(iUniqueID) != d->m_Pages.at(d->m_sCurrentPage).m_TextControls.end())
				{
					thisControl = MyEnum::textControl;
					bDoubleClicked = d->m_Pages.at(d->m_sCurrentPage).m_TextControls.at(iUniqueID).bDoubleClicked;
					break;
				}

				// check pageless text controls
				if (d->m_sCurrentPage != d->m_sTitle)
					if (d->m_Pages.at(d->m_sTitle).m_TextControls.find(iUniqueID) != d->m_Pages.at(d->m_sTitle).m_TextControls.end())
						if (d->m_Pages.at(d->m_sTitle).m_TextControls.at(iUniqueID).bPageLess)
						{
							thisControl = MyEnum::textControl;
							bDoubleClicked = d->m_Pages.at(d->m_sTitle).m_TextControls.at(iUniqueID).bDoubleClicked;
							bPageLessControl = true;
							break;
						}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}

			// check if it's a combobox control
			try
			{
				// check current page's combobox controls
				if (d->m_Pages.at(d->m_sCurrentPage).m_ComboBoxControls.find(iUniqueID) != d->m_Pages.at(d->m_sCurrentPage).m_ComboBoxControls.end())
				{
					thisControl = MyEnum::comboControl;
					hWndItem = d->m_Pages.at(d->m_sCurrentPage).m_ComboBoxControls.at(iUniqueID).hWnd;
					break;
				}

				// check pageless combobox controls
				if (d->m_sCurrentPage != d->m_sTitle)
					if (d->m_Pages.at(d->m_sTitle).m_ComboBoxControls.find(iUniqueID) != d->m_Pages.at(d->m_sTitle).m_ComboBoxControls.end())
						if (d->m_Pages.at(d->m_sTitle).m_ComboBoxControls.at(iUniqueID).bPageLess)
						{
							thisControl = MyEnum::comboControl;
							hWndItem = d->m_Pages.at(d->m_sTitle).m_ComboBoxControls.at(iUniqueID).hWnd;
							bPageLessControl = true;
							break;
						}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}

			// check if it's a listview control
			try
			{
				// check current page's list view controls
				if (d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.find(iUniqueID) != d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.end())
				{
					thisControl = MyEnum::listviewControl;
					hWndItem = d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(iUniqueID).hWnd;
					pClistview = d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(iUniqueID).pClistview;
					vlistviewColumns = d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(iUniqueID).vColumns;

					if (d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(iUniqueID).bDoubleClicked)
					{
						d->m_Pages.at(d->m_sCurrentPage).m_listviewControls.at(iUniqueID).bDoubleClicked = false;
						blistviewItemDoubleClicked = true;
					}

					break;
				}

				// check pageless list view controls
				if (d->m_sCurrentPage != d->m_sTitle)
					if (d->m_Pages.at(d->m_sTitle).m_listviewControls.find(iUniqueID) != d->m_Pages.at(d->m_sTitle).m_listviewControls.end())
						if (d->m_Pages.at(d->m_sTitle).m_listviewControls.at(iUniqueID).bPageLess)
						{
							thisControl = MyEnum::listviewControl;
							hWndItem = d->m_Pages.at(d->m_sTitle).m_listviewControls.at(iUniqueID).hWnd;
							pClistview = d->m_Pages.at(d->m_sTitle).m_listviewControls.at(iUniqueID).pClistview;
							vlistviewColumns = d->m_Pages.at(d->m_sTitle).m_listviewControls.at(iUniqueID).vColumns;

							if (d->m_Pages.at(d->m_sTitle).m_listviewControls.at(iUniqueID).bDoubleClicked)
							{
								d->m_Pages.at(d->m_sTitle).m_listviewControls.at(iUniqueID).bDoubleClicked = false;
								blistviewItemDoubleClicked = true;
							}

							bPageLessControl = true;
							break;
						}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}

			// check if it's a listview popup item
			try
			{
				if (!bPageLessControl)
				{
					for (auto &it : d->m_Pages.at(d->m_sCurrentPage).m_listviewControls)
					{
						bool bBreak = false;

						for (size_t x = 0; x < it.second.vContextMenu.size(); x++)
						{
							if (it.second.vContextMenu[x].iUniqueID == iUniqueID)
							{
								thisControl = MyEnum::listviewPopupMenuItem;
								pClistview = it.second.pClistview;
								vlistviewColumns = it.second.vColumns;
								listviewcontextmenuitem = it.second.vContextMenu[x];
								bBreak = true;
								break;
							}
						}

						if (bBreak)
							break;
					}
				}
				else
				{
					for (auto &it : d->m_Pages.at(d->m_sTitle).m_listviewControls)
					{
						if (it.second.bPageLess)
						{
							bool bBreak = false;

							for (size_t x = 0; x < it.second.vContextMenu.size(); x++)
							{
								if (it.second.vContextMenu[x].iUniqueID == iUniqueID)
								{
									thisControl = MyEnum::listviewPopupMenuItem;
									pClistview = it.second.pClistview;
									vlistviewColumns = it.second.vColumns;
									listviewcontextmenuitem = it.second.vContextMenu[x];

									bBreak = true;
									break;
								}
							}

							if (bBreak)
								break;
						}
					}
				}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}

			// check if it's a toggle button control
			try
			{
				// handle current page's toggle button controls
				if (d->m_Pages.at(d->m_sCurrentPage).m_ToggleButtonControls.find(iUniqueID) != d->m_Pages.at(d->m_sCurrentPage).m_ToggleButtonControls.end())
				{
					thisControl = MyEnum::toggleButtonControl;
					bToggleBtnOn = d->m_Pages.at(d->m_sCurrentPage).m_ToggleButtonControls.at(iUniqueID).bOn;
					break;
				}

				// handle pageless toggle button controls
				if (d->m_sCurrentPage != d->m_sTitle)
					if (d->m_Pages.at(d->m_sTitle).m_ToggleButtonControls.find(iUniqueID) != d->m_Pages.at(d->m_sTitle).m_ToggleButtonControls.end())
						if (d->m_Pages.at(d->m_sTitle).m_ToggleButtonControls.at(iUniqueID).bPageLess)
						{
							thisControl = MyEnum::toggleButtonControl;
							bToggleBtnOn = d->m_Pages.at(d->m_sTitle).m_ToggleButtonControls.at(iUniqueID).bOn;
							bPageLessControl = true;
							break;
						}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}

			// check if it's a star rating control
			try
			{
				// handle current page's star rating controls
				if (d->m_Pages.at(d->m_sCurrentPage).m_StarRatingControls.find(iUniqueID) != d->m_Pages.at(d->m_sCurrentPage).m_StarRatingControls.end())
				{
					thisControl = MyEnum::starRatingControl;
					break;
				}

				// handle pageless star rating controls
				if (d->m_sCurrentPage != d->m_sTitle)
					if (d->m_Pages.at(d->m_sTitle).m_StarRatingControls.find(iUniqueID) != d->m_Pages.at(d->m_sTitle).m_StarRatingControls.end())
						if (d->m_Pages.at(d->m_sTitle).m_StarRatingControls.at(iUniqueID).bPageLess)
						{
							thisControl = MyEnum::starRatingControl;
							bPageLessControl = true;
							break;
						}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}

			try
			{
				// handle current page's edit controls
				if (d->m_Pages.at(d->m_sCurrentPage).m_EditControls.find(iUniqueID) != d->m_Pages.at(d->m_sCurrentPage).m_EditControls.end())
				{
					if (d->m_Pages.at(d->m_sCurrentPage).m_EditControls[iUniqueID].bUpDown)
					{
						// this is an updown control

						// check the value in the edit control
						auto getWinText = [](HWND hWnd)
						{
							int i = GetWindowTextLength(hWnd) + 1;

							std::basic_string<TCHAR> sText;

							TCHAR *buffer = new TCHAR[i];
							GetWindowText(hWnd, buffer, i);
							sText = buffer;
							delete[] buffer;

							return sText;
						}; // getWinText

						std::basic_string<TCHAR> sText = getWinText(d->m_Pages.at(d->m_sCurrentPage).m_EditControls[iUniqueID].hWnd);

						int iPos = _ttoi(sText.c_str());

						const int iPosManual = iPos;

						// if value is beyond iMax set it to iMax
						if (iPos > d->m_Pages.at(d->m_sCurrentPage).m_EditControls[iUniqueID].iMax)
							iPos = d->m_Pages.at(d->m_sCurrentPage).m_EditControls[iUniqueID].iMax;

						// if value is below iMin set it to iMin
						if (iPos < d->m_Pages.at(d->m_sCurrentPage).m_EditControls[iUniqueID].iMin)
							iPos = d->m_Pages.at(d->m_sCurrentPage).m_EditControls[iUniqueID].iMin;

						if (iPos != iPosManual)
						{
							// Set the position of the up-down control
							SendMessage(d->m_Pages.at(d->m_sCurrentPage).m_EditControls[iUniqueID].hWndUpDown, UDM_SETPOS, 0, LPARAM(iPos));
						}
					}

					// this is an edit control
					thisControl = MyEnum::editControl;
					break;
				}

				// handle pageless edit controls
				if (d->m_sCurrentPage != d->m_sTitle)
					if (d->m_Pages.at(d->m_sTitle).m_EditControls.find(iUniqueID) != d->m_Pages.at(d->m_sTitle).m_EditControls.end())
						if (d->m_Pages.at(d->m_sTitle).m_EditControls.at(iUniqueID).bPageLess)
						{
							thisControl = MyEnum::editControl;
							bPageLessControl = true;
							break;
						}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}

			try
			{
				// handle current page's selector controls
				for (auto &m_it : d->m_Pages.at(d->m_sCurrentPage).m_SelectorControls)
				{
					for (auto &it : m_it.second.vItems)
					{
						if (it.iUniqueID == iUniqueID)
						{
							thisControl = MyEnum::selectorControl;
							break;
						}
					}
				}

				// handle pageless selector controls
				if (d->m_sCurrentPage != d->m_sTitle)
				{
					for (auto &m_it : d->m_Pages.at(d->m_sTitle).m_SelectorControls)
					{
						if (m_it.second.bPageLess)
						{
							for (auto &it : m_it.second.vItems)
							{
								if (it.iUniqueID == iUniqueID)
								{
									thisControl = MyEnum::selectorControl;
									bPageLessControl = true;
									break;
								}
							}
						}
					}
				}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}

			// check if it's a date control
			try
			{
				// check current page's date controls
				if (d->m_Pages.at(d->m_sCurrentPage).m_DateControls.find(iUniqueID) != d->m_Pages.at(d->m_sCurrentPage).m_DateControls.end())
				{
					thisControl = MyEnum::dateControl;
					break;
				}

				// check pageless date controls
				if (d->m_sCurrentPage != d->m_sTitle)
					if (d->m_Pages.at(d->m_sTitle).m_DateControls.find(iUniqueID) != d->m_Pages.at(d->m_sTitle).m_DateControls.end())
						if (d->m_Pages.at(d->m_sTitle).m_DateControls.at(iUniqueID).bPageLess)
						{
							thisControl = MyEnum::dateControl;
							bPageLessControl = true;
							break;
						}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}

			// check if it's a time control
			try
			{
				// check current page's time controls
				if (d->m_Pages.at(d->m_sCurrentPage).m_TimeControls.find(iUniqueID) != d->m_Pages.at(d->m_sCurrentPage).m_TimeControls.end())
				{
					thisControl = MyEnum::timeControl;
					break;
				}

				// check pageless time controls
				if (d->m_sCurrentPage != d->m_sTitle)
					if (d->m_Pages.at(d->m_sTitle).m_TimeControls.find(iUniqueID) != d->m_Pages.at(d->m_sTitle).m_TimeControls.end())
						if (d->m_Pages.at(d->m_sTitle).m_TimeControls.at(iUniqueID).bPageLess)
						{
							thisControl = MyEnum::timeControl;
							bPageLessControl = true;
							break;
						}
			}
			catch (std::exception &e)
			{
				// do nothing ... map probably out of range
				std::string m_sErr = e.what();
			}

		} while (false);

		if (thisControl == (MyEnum)-1)
			return;	// failsafe mechanism

					// set data
		void* pData = NULL;

		cui_raw::textMsg txt;
		cui_raw::comboboxMsg combo;
		cui_raw::listviewMsg listview;
		cui_raw::listviewPopupMenuItemMsg listviewpopup;
		cui_raw::toggleButtonMsg toggle;
		cui_raw::copyDataMsg copydata;
		cui_raw::dropFileMsg dropfile;
		cui_raw::pieChartMsg piechart;

		switch (thisControl)
		{
		case dropFile:
		{
			dropfile.sFullPath = pThis->d->m_sDropFile;
			pData = &dropfile;
		}
		break;

		case copyData:
		{
			copydata.sCommandLine = pThis->d->m_sCopyData;
			pData = &copydata;
		}
		break;

		case buttonControl:
			break;

		case toggleButtonControl:
		{
			toggle.bOn = bToggleBtnOn;
			pData = &toggle;
		}
		break;

		case textControl:
		{
			if (bDoubleClicked)
				txt.bDoubleClick = true;	// text control has been double clicked
			else
				txt.bDoubleClick = false;	// text control has been clicked

											// set pointer to text data
			pData = &txt;
		}
		break;

		case comboControl:
		{
			if (iMsg == CBN_SELCHANGE)
			{
				// check index of currently selected item
				int iIndex = (int)SendMessage(hWndItem, CB_GETCURSEL, NULL, NULL);

				if (iIndex == CB_ERR)
				{
					// no item is selected
					return;
				}
				else
				{
					/*
					** explicitly command the control to select this item
					** this will enable calls by GetDlgItemText to be successful
					*/
					SendMessage(hWndItem, CB_SETCURSEL, iIndex, NULL);
				}

				// get the combo's text
				auto getWinText = [](HWND hWnd)
				{
					int i = GetWindowTextLength(hWnd) + 1;

					std::basic_string<TCHAR> sText;

					TCHAR *buffer = new TCHAR[i];
					GetWindowText(hWnd, buffer, i);
					sText = buffer;
					delete[] buffer;

					return sText;
				}; // getWinText

				combo.sSelected = getWinText(hWndItem);
				pData = &combo;
			}
			else
				return;
		}
		break;

		case listviewControl:
		{
			// retrieve data from selected row
			if (pClistview != NULL)
			{
				/*
				** identify the item selected in list view control
				*/
				pClistview->ResetIndex();
				int index = pClistview->GetNextIndex();

				if (index != -1)
				{
					// check number of selected items
					int iSelected = pClistview->Selected();

					if (iSelected == 1)
					{
						for (size_t iColumn = 0; iColumn < vlistviewColumns.size(); iColumn++)
						{
							cui_raw::listviewColumn column = vlistviewColumns[iColumn];

							cui_raw::listviewItem item;
							item.sColumnName = column.sColumnName;
							item.sItemData = pClistview->GetSelected(column.iColumnID);

							listview.row.vItems.push_back(item);
						}
					}
					else
						return;
				}
				else
					return;


				if (blistviewItemDoubleClicked)
					listview.bDoubleClick = true;
				else
					listview.bDoubleClick = false;

				pData = &listview;
			}
			else
				return;
		}
		break;

		case listviewPopupMenuItem:
		{
			// retrieve data from selected row
			if (pClistview != NULL)
			{
				/*
				** identify the item selected in list view control
				*/
				pClistview->ResetIndex();
				int index = pClistview->GetNextIndex();

				if (index != -1)
				{
					// check number of selected items
					int iSelected = pClistview->Selected();

					if (iSelected > 0)
					{
						while (index != -1)
						{
							cui_raw::listviewRow row;

							for (size_t iColumn = 0; iColumn < vlistviewColumns.size(); iColumn++)
							{
								cui_raw::listviewColumn column = vlistviewColumns[iColumn];

								cui_raw::listviewItem item;
								item.sColumnName = column.sColumnName;
								item.sItemData = pClistview->GetSelected(column.iColumnID);

								row.vItems.push_back(item);
							}

							listviewpopup.vRows.push_back(row);

							index = pClistview->GetNextIndex();
						}
					}
					else
						return;
				}
				else
					return;

				// identify clicked popup item
				listviewpopup.iUniqueID = listviewcontextmenuitem.iUniqueID;
				listviewpopup.sLabel = listviewcontextmenuitem.sLabel;

				pData = &listviewpopup;
			}
			else
				return;
		}
		break;

		case pieChartControl:
		{
			// figure out the item
			if (bPageLessControl)
			{
				for (auto it : d->m_Pages.at(d->m_sTitle).m_PieChartControls.at(iUniqueID).chartBarsInfo)
				{
					if (it.bPressed)
					{
						piechart.iNumber = it.iNumber;
						piechart.sItemLabel = it.sItemLabel;
						pData = &piechart;
					}
				}
			}
			else
			{
				for (auto it : d->m_Pages.at(d->m_sCurrentPage).m_PieChartControls.at(iUniqueID).chartBarsInfo)
				{
					if (it.bPressed)
					{
						piechart.iNumber = it.iNumber;
						piechart.sItemLabel = it.sItemLabel;
						pData = &piechart;
					}
				}
			}
		}
		break;

		default:
			break;
		}

		std::basic_string<TCHAR> sPage;

		if (!bPageLessControl)
			sPage = d->m_sCurrentPage;
		else
			sPage = d->m_sTitle;

		if (!sPage.empty() && d->m_Pages.at(sPage).m_pCommandProc)
		{
			// call the command procedure
			d->m_Pages.at(sPage).m_pCommandProc(*pThis, iUniqueID, pData);
		}
	}
	catch (std::exception &e)
	{
		// std::map member at() throws an out_of_range exception if key value doesn't exist
		std::string sErr = e.what();
	}
} // OnWM_COMMAND

bool cui_rawImpl::CreateShadow(
	HWND hWnd
)
{
	if (m_bShadow && m_pShadow)
	{
		// initialize drop shadow parameters
		CShadow::shadow_properties properties;
		properties.color = m_clrTheme;
		properties.darkness = 50;
		properties.position = { 0, 0 };
		properties.sharpness = 20;
		properties.size = 10;

		m_pShadow->CreateShadow(hWnd, properties);

		return true;
	}
	else
		return false;
} // CreateShadow

BOOL cui_rawImpl::Combo_IsExtended(
	HWND hWndCtl	// handle of the control
)
{
	static TCHAR name[MAX_PATH];
	GetClassName(hWndCtl, name, MAX_PATH);

	return 0 == _tcsicmp(name, WC_COMBOBOXEX);
} // Combo_IsExtended

LRESULT CALLBACK cui_rawImpl::listviewControlProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	cui_rawImpl::listviewControl* pThis = reinterpret_cast<cui_rawImpl::listviewControl*>(ptr);

	switch (msg)
	{
	case WM_CONTEXTMENU:
	{
		/*
		** identify the item selected in list view control
		*/
		pThis->pClistview->ResetIndex();
		int index = pThis->pClistview->GetNextIndex();

		if (index != -1)
		{
			// create popup menu
			CPopupMenu pop;
			pop.Create(hWnd);

			// add items to popup menu
			for (size_t i = 0; i < pThis->vContextMenu.size(); i++)
			{
				cui_raw::contextMenuItem item = pThis->vContextMenu[i];

				pop.AddItem(item.iUniqueID, item.sLabel, item.IDC_PNGICON, pThis->d->m_hResModule, item.bDefault);
			}

			// show popup menu and return ID of clicked item
			WORD cmd = pop.Show();

			// send parent message
			if (cmd)
				SendMessage(GetParent(hWnd), WM_COMMAND, (WPARAM)cmd, NULL);
		}
	}
	break;
	}

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)pThis->PrevProc, hWnd, msg, wParam, lParam);
} // listviewControlProc

int CALLBACK cui_rawImpl::CompareListItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParam)
{
	sortstruct *pSort = (sortstruct*)lParam;

	LPARAM lParamSort = pSort->lParamSort;
	HWND hlistview = pSort->hlistview;

	BOOL bSortAscending = (lParamSort > 0);

	int nColumn = abs(lParamSort) - 1;

	// TO-DO: get rid of these magic numbers!!!!!!!!
	TCHAR szBuf1[256], szBuf2[256];

	/*
	** get the text from listview (the coloumn is nColumn and the indexes are lParam1 and lParam2)
	*/
	ListView_GetItemText(hlistview, lParam1, nColumn, szBuf1, sizeof(szBuf1));
	ListView_GetItemText(hlistview, lParam2, nColumn, szBuf2, sizeof(szBuf2));

	// check if we are sorting numbers
	std::basic_string<TCHAR> sBuf1(szBuf1, 256);
	std::basic_string<TCHAR> sBuf2(szBuf2, 256);

	int dBuf1 = _ttoi(szBuf1);
	int dBuf2 = _ttoi(szBuf2);

	std::basic_string<TCHAR> sBuf1_a = to_tstring(dBuf1);
	std::basic_string<TCHAR> sBuf2_a = to_tstring(dBuf2);

	if (sBuf1 == sBuf1_a && sBuf2 == sBuf2_a)
	{
		// sort numbers
		if (bSortAscending)
		{
			// sort ascending
			return (dBuf1 > dBuf2);
		}
		else
		{
			// sort descending
			return (dBuf1 < dBuf2);
		}
	}

	// sort text
	if (bSortAscending)
	{
		// sort ascending
		return (lstrcmpi(szBuf1, szBuf2) * -1);
	}
	else
	{
		// sort descending
		return (lstrcmpi(szBuf1, szBuf2));
	}
} // CompareListItems

LRESULT cui_rawImpl::HandleCustomDraw(NMLVCUSTOMDRAW* pcd, HWND hWndlistview, std::vector<cui_raw::listviewRow> lvRows, std::vector<cui_raw::listviewColumn> lvColumns)
{
	// TO-DO: get rid of this magic number!!!!!!!!
	TCHAR buffer[256];
	LVITEM item;

	switch (pcd->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		/* Tell the control we are interested in per-item notifications.
		* (We need it just to tell the control we want per-subitem
		* notifications.) */
		return CDRF_DODEFAULT | CDRF_NOTIFYITEMDRAW;

	case (CDDS_ITEM | CDDS_PREPAINT):
		/* Tell the control we are interested in per-subitem notifications. */
		return CDRF_DODEFAULT | CDRF_NOTIFYSUBITEMDRAW;

	case (CDDS_ITEM | CDDS_SUBITEM | CDDS_PREPAINT):
		item.iSubItem = pcd->iSubItem;
		item.pszText = buffer;
		item.cchTextMax = sizeof(buffer) / sizeof(buffer[0]);
		SendMessage(hWndlistview, LVM_GETITEMTEXT, pcd->nmcd.dwItemSpec, (LPARAM)&item);
		int iColumn = pcd->iSubItem;
		int iRow = item.iItem;

		// check if this item requires custom drawing
		for (size_t m_iRow = 0; m_iRow < lvRows.size(); m_iRow++)
		{
			if (m_iRow == iRow)
			{
				for (size_t m_iColumn = 0; m_iColumn < lvRows[m_iRow].vItems.size(); m_iColumn++)
				{
					if (m_iColumn == iColumn)
					{
						if (lvRows[m_iRow].vItems[m_iColumn].bCustom)
						{
							pcd->clrText = lvRows[m_iRow].vItems[m_iColumn].clrText;

							/* Let the control do the painting itself with the new color. */
							return CDRF_DODEFAULT;
						}
						else
							pcd->clrText = RGB(0, 0, 0);
					}
				}
			}
		}

		for (size_t i = 0; i < lvColumns.size(); i++)
		{
			if (lvColumns[i].bBarChart)
			{
				if (lvColumns[i].iColumnID == iColumn)
				{
					/* Customize "progress" column. We paint simple progress
					* indicator. */
					item.iSubItem = iColumn;
					item.pszText = buffer;
					item.cchTextMax = sizeof(buffer) / sizeof(buffer[0]);
					SendMessage(hWndlistview, LVM_GETITEMTEXT, pcd->nmcd.dwItemSpec, (LPARAM)&item);

					int iProgress = _ttoi(buffer);
					int iMax = lvColumns[i].iBarChartMax;

					if (iMax > 0)
					{
						int cx;
						HDC hdc = pcd->nmcd.hdc;
						COLORREF clrBack;
						HBRUSH hBackBrush;
						HBRUSH hProgressBrush;
						HBRUSH hOldBrush;
						HPEN hPen;
						HPEN hOldPen;
						RECT rc;

						clrBack = pcd->clrTextBk;
						if (clrBack == CLR_NONE || clrBack == CLR_DEFAULT)
							clrBack = RGB(255, 255, 255);

						hBackBrush = CreateSolidBrush(clrBack);
						hProgressBrush = CreateSolidBrush(lvColumns[i].clrBarChart);
						hPen = CreatePen(PS_SOLID, 0, lvColumns[i].clrBarChart);

						hOldBrush = (HBRUSH)SelectObject(hdc, hBackBrush);
						FillRect(hdc, &pcd->nmcd.rc, hBackBrush);

						cx = pcd->nmcd.rc.right - pcd->nmcd.rc.left - 6;
						if (cx < 0)
							cx = 0;
						rc.left = pcd->nmcd.rc.left + 3;
						rc.top = pcd->nmcd.rc.top + 2;
						rc.right = rc.left + cx * iProgress / iMax;
						rc.bottom = pcd->nmcd.rc.bottom - 2;
						SelectObject(hdc, hProgressBrush);
						FillRect(hdc, &rc, hProgressBrush);

						rc.right = pcd->nmcd.rc.right - 3;
						SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
						hOldPen = (HPEN)SelectObject(hdc, hPen);
						Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

						COLORREF clrOld = SetTextColor(hdc, lvColumns[i].clrText);

						DrawText(hdc, buffer, -1, &rc,
							DT_CENTER | DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS);

						SetTextColor(hdc, clrOld);
						SelectObject(hdc, hOldBrush);
						DeleteObject(hProgressBrush);
						DeleteObject(hBackBrush);
						SelectObject(hdc, hOldPen);
						DeleteObject(hPen);
					}

					/* Tell the control to not paint as we did so. */
					return CDRF_SKIPDEFAULT;
				} // if
			}
			else
			{
				if (lvColumns[i].iColumnID == pcd->iSubItem && !(pcd->nmcd.dwDrawStage & CDDS_PREPAINT))
				{
					item.iSubItem = pcd->iSubItem;
					item.pszText = buffer;
					item.cchTextMax = sizeof(buffer) / sizeof(buffer[0]);
					SendMessage(hWndlistview, LVM_GETITEMTEXT, pcd->nmcd.dwItemSpec, (LPARAM)&item);

					int iProgress = _ttoi(buffer);
					int iMax = lvColumns[i].iBarChartMax;

					if (iMax > 0)
					{
						int cx;
						HDC hdc = pcd->nmcd.hdc;
						COLORREF clrBack;
						HBRUSH hBackBrush;
						HBRUSH hProgressBrush;
						HBRUSH hOldBrush;
						HPEN hPen;
						HPEN hOldPen;
						RECT rc;

						clrBack = pcd->clrTextBk;
						if (clrBack == CLR_NONE || clrBack == CLR_DEFAULT)
							clrBack = RGB(255, 255, 255);

						hBackBrush = CreateSolidBrush(clrBack);
						hProgressBrush = CreateSolidBrush(lvColumns[i].clrBarChart);
						hPen = CreatePen(PS_SOLID, 0, lvColumns[i].clrBarChart);

						hOldBrush = (HBRUSH)SelectObject(hdc, hBackBrush);
						FillRect(hdc, &pcd->nmcd.rc, hBackBrush);

						cx = pcd->nmcd.rc.right - pcd->nmcd.rc.left - 6;
						if (cx < 0)
							cx = 0;
						rc.left = pcd->nmcd.rc.left + 3;
						rc.top = pcd->nmcd.rc.top + 2;
						rc.right = rc.left + cx * iProgress / iMax;
						rc.bottom = pcd->nmcd.rc.bottom - 2;
						SelectObject(hdc, hProgressBrush);
						//FillRect(hdc, &rc, hProgressBrush);

						rc.right = pcd->nmcd.rc.right - 3;
						SelectObject(hdc, GetStockObject(HOLLOW_BRUSH));
						hOldPen = (HPEN)SelectObject(hdc, hPen);
						//Rectangle(hdc, rc.left, rc.top, rc.right, rc.bottom);

						COLORREF clrOld = SetTextColor(hdc, lvColumns[i].clrText);

						DrawText(hdc, buffer, -1, &rc,
							DT_LEFT | DT_VCENTER | DT_NOPREFIX | DT_SINGLELINE | DT_END_ELLIPSIS);

						SetTextColor(hdc, clrOld);
						SelectObject(hdc, hOldBrush);
						DeleteObject(hProgressBrush);
						DeleteObject(hBackBrush);
						SelectObject(hdc, hOldPen);
						DeleteObject(hPen);
					}

					/* Tell the control to not paint as we did so. */
					return CDRF_SKIPDEFAULT;
				}
			}
		} // for
		break;
	}

	/* For all unhandled cases, we let the control do the default. */
	return CDRF_DODEFAULT;
} // HandleCustomDraw

  // returns true if (x, y) is within rc
bool cui_rawImpl::insideRect(int x, int y, LPRECT lpRect)
{
	if (lpRect)
	{
		if ((x >= lpRect->left) && (x <= lpRect->right) && (y >= lpRect->top) && (y <= lpRect->bottom))
			return true;
		else
			return false;
	}
	else
		return false;
} // insideRect

LRESULT CALLBACK cui_rawImpl::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	cui_raw *pThis;
	if (msg == WM_CREATE)
	{
		CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
		pThis = reinterpret_cast<cui_raw*>(pCreate->lpCreateParams);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)pThis);
	}
	else
	{
		LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
		pThis = reinterpret_cast<cui_raw*>(ptr);
	}

	if (pThis)
	{
		switch (msg)
		{
		case WM_CREATE:
			pThis->d->OnWM_CREATE(hWnd, pThis);
			return TRUE;

		case WM_SIZE:
			pThis->d->OnWM_SIZE(hWnd, wParam, pThis->d);
			return TRUE;

		case WM_MOUSEMOVE:
			pThis->d->OnWM_MOUSEMOVE(hWnd, lParam, pThis->d);
			return NULL;

		case WM_MOUSELEAVE:
			pThis->d->OnWM_MOUSELEAVE(hWnd, pThis->d);
			return NULL;

		case WM_MOUSEHOVER:
			pThis->d->m_pMouseTrack->Reset(hWnd);
			return NULL;

		case WM_LBUTTONUP:
			OnWM_LBUTTONUP(hWnd, pThis->d);
			return NULL;

		case WM_LBUTTONDOWN:
			OnWM_LBUTTONDOWN(hWnd, pThis->d);
			return NULL;

		case WM_LBUTTONDBLCLK:
			OnWM_LBUTTONDBLCLK(hWnd, pThis->d);
			return NULL;

		case WM_RBUTTONDOWN:
			OnWM_RBUTTONDOWN(hWnd, pThis->d);
			return NULL;

		case WM_RBUTTONUP:
			OnWM_RBUTTONUP(hWnd, pThis->d);
			return NULL;

		case WM_COMMAND:
		{
			int iID = LOWORD(wParam);

			auto isFontCombo = [&]()
			{
				for (auto m_it : pThis->d->vFontCombos)
				{
					if (iID == m_it)
						return true;
				}

				return false;
			};

			bool bHandle = true;

			if (isFontCombo())
			{
				bHandle = false;

				// this is a font combobox
				HWND hWndCombo = NULL;
				int m_iMaxNameWidth = 0;

				for (auto &page : pThis->d->m_Pages)
				{
					for (auto combo : page.second.m_ComboBoxControls)
					{
						if (combo.second.iUniqueID == iID)
						{
							hWndCombo = combo.second.hWnd;
							m_iMaxNameWidth = combo.second.m_iMaxNameWidth;
						}
					}
				}

				switch (HIWORD(wParam))
				{
				case CBN_DROPDOWN: // This means that the list is about to display
				{
					int nScrollWidth = ::GetSystemMetrics(SM_CXVSCROLL);
					int nWidth = nScrollWidth;

					nWidth += m_iMaxNameWidth;

					SendMessage(hWndCombo, CB_SETDROPPEDWIDTH, nWidth, 0);
				}
				break;

				case CBN_SELCHANGE:	// This means the selection has changed
					bHandle = true;	// change this flag so font selection change is handled
					break;

				default:
					break;
				}
			}

			if (bHandle)
			{
				auto isRichEditControl = [&]()
				{
					for (auto &page : pThis->d->m_Pages)
					{
						for (auto &ctrl : page.second.m_RichEditControls)
						{
							if (iID == ctrl.second.iIDLineLeft ||
								iID == ctrl.second.iIDLineRight ||
								iID == ctrl.second.iIDLineTop ||
								iID == ctrl.second.iIDLineBottom ||
								iID == ctrl.second.iIDHSeparator ||
								iID == ctrl.second.iIDVSeparator ||
								iID == ctrl.second.iIDFontList ||
								iID == ctrl.second.iIDFontSize ||
								iID == ctrl.second.iIDFontLabel ||
								iID == ctrl.second.iIDParagraphLabel ||
								iID == ctrl.second.iBold ||
								iID == ctrl.second.iItalic ||
								iID == ctrl.second.iUnderline ||
								iID == ctrl.second.iStrikethough ||
								iID == ctrl.second.iSubscript ||
								iID == ctrl.second.iSuperscript ||
								iID == ctrl.second.iLarger ||
								iID == ctrl.second.iSmaller ||
								iID == ctrl.second.iFontColor ||
								iID == ctrl.second.iLeftAlign ||
								iID == ctrl.second.iCenterAlign ||
								iID == ctrl.second.iRightAlign ||
								iID == ctrl.second.iJustify ||
								iID == ctrl.second.iList ||
								iID == ctrl.second.iListType
								)
								return true;
						}
					}

					return false;
				};

				if (isRichEditControl())
					OnRichEdit_COMMAND(hWnd, wParam, pThis->d, pThis);
				else
					OnWM_COMMAND(hWnd, wParam, pThis->d, pThis);
			}
		}
		break;

		case WM_KEYUP:
		{
			switch (wParam)
			{
			case VK_RETURN:
			{
				// enter has been pressed, find the first visible default button of this page and send a message to it
				for (auto &it : pThis->d->m_Pages[pThis->d->m_sCurrentPage].m_ButtonControls)
				{
					if (it.second.bDefaultButton && IsWindowVisible(it.second.hWnd) && IsWindowEnabled(it.second.hWnd))
					{
						SendMessage(GetParent(it.second.hWnd), WM_COMMAND, (WPARAM)it.second.iUniqueID, NULL);
						break;
					}
				}
			}
			break;

			default:
				break;
			}
		}

		case WM_PAINT:
			OnWM_PAINT(hWnd, pThis->d);
			break;

		case WM_CLOSE:
		{
			if (wParam == 0)
			{
				// this is coming from elsewhere ... probably Alt+F4 ...

				for (auto &it : pThis->d->m_ControlBtns)
				{
					if (it.second.iUniqueID == pThis->d->m_iIDClose)
					{
						if (it.second.bCustomHandle)
						{
							// call the command procedure
							PostMessage(GetParent(it.second.hWnd), WM_COMMAND, it.second.iUniqueID, NULL);
							return 0;
						}
					}
				}
			}

			bool bDontForceClose = !(wParam == 1);

			if (bDontForceClose && pThis->d->m_iStopQuit > 0)
				return 0;

			// stop all timers
			// TO-DO: find a way of making sure all timers have actually stopped (like not currently doing anything)
			for (auto &it : pThis->d->m_Timers)
				KillTimer(hWnd, (UINT_PTR)it.first);

			// call the shut down ID
			if (pThis->d->m_Pages[pThis->d->m_sTitle].m_pCommandProc)
				pThis->d->m_Pages[pThis->d->m_sTitle].m_pCommandProc(*pThis, pThis->d->m_iShutdownID, NULL);

			if (pThis->d->m_Children.size())
			{
				// Attempting to close while there are open children. Tell all the children their parent is closing.
				for (auto &it : pThis->d->m_Children)
					it.second->d->m_bParentClosing = true;
			}

			// enable parent
			if (pThis->d->parent_was_enabled)
				EnableWindow(pThis->d->m_hWndParent, TRUE);

			// destroy the window
			DestroyWindow(hWnd);

			return 0;
		}
		break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}
		break;

		case WM_NCHITTEST:
			return OnWM_NCHITTEST(hWnd, lParam, pThis->d);
			break;

		case WM_GETMINMAXINFO:
			OnWM_GETMINMAXINFO(hWnd, wParam, lParam, pThis->d);
			break;

		case WM_ERASEBKGND:
			OnWM_ERASEBKGND(hWnd, wParam, pThis->d);
			return TRUE;

		case WM_CTLCOLORSTATIC:
		{
			for (auto &it : pThis->d->m_Pages.at(pThis->d->m_sCurrentPage).m_EditControls)
			{
				if (it.second.hWnd == (HWND)lParam)
				{
					SetBkMode((HDC)wParam, TRANSPARENT);
					SetTextColor((HDC)wParam, RGB(0, 0, 0));	// TO-DO: find out why this isn't working
					return (LRESULT)pThis->d->m_hbrBackground;
				}
			}
		}
		break;

		case WM_NOTIFY:
		{
			int iControlID = wParam;

			// check if this is a rich edit control
			auto isRichEdit = [&]()
			{
				for (auto &page : pThis->d->m_Pages)
				{
					for (auto &ctrl : page.second.m_RichEditControls)
					{
						if (iControlID == ctrl.second.iUniqueID)
							return true;
					}
				}

				return false;
			};

			if (isRichEdit())
				return OnRichEdit_NOTIFY(hWnd, wParam, lParam, pThis->d, pThis);
			else
				return OnWM_NOTIFY(hWnd, lParam, pThis->d);
		}
		break;

		case WM_TIMER:
		{
			if (wParam == pThis->d->ID_TIMER)
			{
				if (pThis->d->m_bTimerRunning)	// failsafe
				{
					// stop the timer and close window
					KillTimer(hWnd, pThis->d->ID_TIMER);
					PostQuitMessage(0);
				}
			}
			else
				if (wParam == pThis->d->ID_TIMER_CHECK)
				{
					if (!pThis->d->m_bTimerRunning)	// failsafe
					{
						// check cursor position
						POINT pt;
						pt = { 0 };
						GetCursorPos(&pt);

						if (pThis->d->m_iTimer > 0 &&
							(pt.x != pThis->d->m_ptStartCheck.x || pt.y != pThis->d->m_ptStartCheck.y))
						{
							// mouse has moved ... stop checking and start timer
							KillTimer(hWnd, pThis->d->ID_TIMER_CHECK);

							// set timer running flag to true
							pThis->d->m_bTimerRunning = true;

							// start the timer
							SetTimer(hWnd, pThis->d->ID_TIMER, 1000 * pThis->d->m_iTimer, NULL);
						}
					}
				}
				else
					SendMessage(hWnd, WM_COMMAND, wParam, NULL);
		}
		break;

		case WM_APP:
		{
			switch (lParam)
			{
			case WM_RBUTTONUP:
				pThis->d->showTrayPopupMenu();
				PostMessage(hWnd, WM_APP + 1, 0, 0);
				break;

			case WM_LBUTTONUP:
			{
				for (auto &it : pThis->d->m_trayIcon.m_trayItems)
				{
					if (it.bDefault)
					{
						PostMessage(hWnd, WM_COMMAND, it.iUniqueID, NULL);
						break;
					}
				}
			}
			break;

			default:
				break;
			}
		}
		break;

		case WM_COPYDATA:
		{
			COPYDATASTRUCT *pCds = (COPYDATASTRUCT*)lParam;

			if (pCds && pThis->d->m_iCopyDataID != 0)
			{
				std::string s((LPSTR)pCds->lpData, pCds->cbData);

				if (!s.empty())
				{
					// data has been received ... forward it to the command procedure
					pThis->d->m_sCopyData = s;
					SendMessage(hWnd, WM_COMMAND, (WPARAM)pThis->d->m_iCopyDataID, NULL);
				}
			}
		}
		break;

		case WM_DROPFILES:
		{
			if (pThis->d->m_iDropFilesID != 0)
			{
				// a file has been dropped onto the window ... forward its full path to the command procedure
				TCHAR file[MAX_PATH];

				ZeroMemory(file, sizeof(file));

				HDROP hDrp = (HDROP)wParam;
				DragQueryFile(hDrp, 0, file, sizeof(file));
				DragFinish(hDrp);

				pThis->d->m_sDropFile = file;
				SendMessage(hWnd, WM_COMMAND, (WPARAM)pThis->d->m_iDropFilesID, NULL);
			}

			return TRUE;
		}
		break;

		case WM_MEASUREITEM:
		{
			LPMEASUREITEMSTRUCT lpmis;
			lpmis = (LPMEASUREITEMSTRUCT)lParam;
			if (lpmis->CtlType != ODT_COMBOBOX)
				break;

			bool bFound = false;

			for (auto m_it : pThis->d->vFontCombos)
			{
				if (lpmis->CtlID == m_it)
					bFound = true;
			}

			if (!bFound)
				break;

			// this is a font combobox ... set the height accordingly

			//lpmis->itemWidth = 10;
			//lpmis->itemHeight = int(20.0 * pThis->d->m_DPIScale + 0.5);
		}
		break;

		case WM_DRAWITEM:
		{
			DRAWITEMSTRUCT * lpDIS;
			lpDIS = (DRAWITEMSTRUCT *)lParam;

			if (lpDIS->CtlType != ODT_COMBOBOX)
				return 0;

			if (lpDIS->itemData == -1)
				return 0;

			bool bFound = false;

			for (auto m_it : pThis->d->vFontCombos)
			{
				if (lpDIS->CtlID == m_it)
					bFound = true;
			}

			if (!bFound)
				return 0;

			HWND hWndCombo = NULL;
			COLORREF m_clrSample = RGB(0, 0, 0);

			for (auto &page : pThis->d->m_Pages)
			{
				for (auto combo : page.second.m_ComboBoxControls)
				{
					if (combo.second.iUniqueID == lpDIS->CtlID)
					{
						hWndCombo = combo.second.hWnd;
						break;
					}
				}

				if (hWndCombo)
					break;
			}

			if (false)
			{
				// not currently used
				LONG_PTR lData = (LONG_PTR)SendMessage(hWndCombo, CB_GETITEMDATA, lpDIS->itemID, 0L);	// retrieve width of item in data
			}

			RECT rc = lpDIS->rcItem;

			HDC dc = lpDIS->hDC;

			if (lpDIS->itemState & ODS_FOCUS)
				DrawFocusRect(dc, &rc);

			std::basic_string<TCHAR> csCurFontName;
			TCHAR pFilename[_MAX_PATH];
			SendMessageW(hWndCombo, CB_GETLBTEXT, lpDIS->itemID, (LPARAM)pFilename);
			csCurFontName = pFilename;

			Gdiplus::RectF layoutRect = convert_rect(rc);

			// create graphics object
			Gdiplus::Graphics gr(dc);

			// fill background
			Gdiplus::Color color;

			if (lpDIS->itemState & ODS_SELECTED)
				color.SetFromCOLORREF(GetSysColor(COLOR_HIGHLIGHT));
			else
				color.SetFromCOLORREF(GetBkColor(dc));

			Gdiplus::RectF bk_rect = convert_rect(rc);

			Gdiplus::SolidBrush bk_brush(color);
			gr.FillRectangle(&bk_brush, bk_rect);

			// measure text rectangle
			Gdiplus::RectF text_rect;
			gr.MeasureString(csCurFontName.c_str(), -1, pThis->d->vFonts[csCurFontName],
				layoutRect, &text_rect);

			if (true)
			{
				if (text_rect.Width < layoutRect.Width)
					text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
			}

			// align the text rectangle to the layout rectangle
			align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::middleleft);

			Gdiplus::StringFormat format;
			format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
			format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingCharacter);
			format.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);

			// draw text
			if (lpDIS->itemState & ODS_SELECTED)
				color.SetFromCOLORREF(GetSysColor(COLOR_HIGHLIGHTTEXT));
			else
				color.SetFromCOLORREF(GetSysColor(COLOR_WINDOWTEXT));

			Gdiplus::SolidBrush text_brush(color);

			gr.DrawString(csCurFontName.c_str(),
				-1, pThis->d->vFonts[csCurFontName], text_rect, &format, &text_brush);

			return 0;
		}
		break;

		default:
		{
			if (pThis->d->m_iRegID != 0)
			{
				// check if the caller is checking this window's unique registration id
				if (msg == pThis->d->m_iRegID)
				{
					/*
					** ID the caller is checking matches this window ...
					** most likely another instance of this window is checking if it should proceed
					*/
					return pThis->d->m_iRegID;
				}
			}
		}
		break;
		}
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
} // WndProc

void cui_rawImpl::hidePage(std::basic_string<TCHAR> sPageName)
{
	try
	{
		// disable and hide all tooltips in image controls
		for (auto &it : m_Pages.at(sPageName).m_ImageControls)
			HideToolTip(it.second.toolTip);

		// disable and hide all tooltips in toggle buttons
		for (auto &it : m_Pages.at(sPageName).m_ToggleButtonControls)
			HideToolTip(it.second.toolTip);

		// disable and hide all tooltips in star rating controls
		for (auto &it : m_Pages.at(sPageName).m_StarRatingControls)
			HideToolTip(it.second.toolTip);

		// disable and hide all tooltips in button controls
		for (auto &it : m_Pages.at(sPageName).m_ButtonControls)
			HideToolTip(it.second.toolTip);

		// disable and hide all tooltips in text controls
		for (auto &it : m_Pages.at(sPageName).m_TextControls)
			HideToolTip(it.second.toolTip);

		// hide all controls in page
		for (size_t i = 0; i < m_vControls.at(sPageName).size(); i++)
			ShowWindow(m_vControls.at(sPageName)[i], SW_HIDE);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// reset window cursor
	SetClassLongPtr(m_hWnd, GCLP_HCURSOR, (LONG_PTR)m_hNormalCursor);
} // hidePage

void cui_rawImpl::showPage(std::basic_string<TCHAR> sPageName)
{
	try
	{
		std::vector<HWND> *pVHidden = &m_vHiddenControls.at(sPageName);

		// show pageless controls
		if (bFirstRun)
		{
			for (size_t i = 0; i < m_vPagelessControls.at(m_sTitle).size(); i++)
			{
				if (std::find(pVHidden->begin(), pVHidden->end(), m_vPagelessControls.at(m_sTitle)[i]) == pVHidden->end())
					ShowWindow(m_vPagelessControls.at(m_sTitle)[i], SW_SHOW);
			}

			bFirstRun = false;
		}

		// check if this page has a tab control
		if (m_Pages.at(sPageName).m_TabControl.iUniqueID == -123)
		{
			for (size_t i = 0; i < m_vControls.at(sPageName).size(); i++)
			{
				if (std::find(pVHidden->begin(), pVHidden->end(), m_vControls.at(sPageName)[i]) == pVHidden->end())
					ShowWindow(m_vControls.at(sPageName)[i], SW_SHOW);
			}
		}
		else
		{
			// show all controls
			for (size_t i = 0; i < m_vControls.at(sPageName).size(); i++)
			{
				if (std::find(pVHidden->begin(), pVHidden->end(), m_vControls.at(sPageName)[i]) == pVHidden->end())
					ShowWindow(m_vControls.at(sPageName)[i], SW_SHOW);
			}

			// check if tab control is hidden
			bool bTabHidden = false;

			if (std::find(pVHidden->begin(), pVHidden->end(), m_Pages.at(sPageName).m_TabControl.hWnd) != pVHidden->end())
				bTabHidden = true;

			if (bTabHidden)
			{
				// hide controls within tabs
				for (size_t i = 0; i < m_Pages.at(sPageName).m_TabControl.vTabs.size(); i++)
				{
					for (auto &it : m_Pages.at(sPageName).m_TabControl.vTabs[i].m_Controls)
						ShowWindow(it.second, SW_HIDE);
				}
			}
			else
			{
				// hide controls within tabs except those in the selected tab
				for (size_t i = 0; i < m_Pages.at(sPageName).m_TabControl.vTabs.size(); i++)
				{
					DWORD dwCmd = SW_HIDE;

					if (m_Pages.at(sPageName).m_TabControl.vTabs[i].bSelected)
						dwCmd = SW_SHOW;

					for (auto &it : m_Pages.at(sPageName).m_TabControl.vTabs[i].m_Controls)
					{
						bool bHidden = false;

						if (std::find(pVHidden->begin(), pVHidden->end(), it.second) != pVHidden->end())
							bHidden = true;

						if (bHidden)
							ShowWindow(it.second, SW_HIDE);
						else
							ShowWindow(it.second, dwCmd);
					}
				}
			}
		}

	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // showPage

void cui_rawImpl::handleTabControls(const std::basic_string<TCHAR> &sPageName, int iUniqueID)
{
	// check if there is a tab control in this page
	if (m_Pages.at(sPageName).m_TabControl.iUniqueID != 123)	// TO-DO: remove magic number
	{
		for (size_t i = 0; i < m_Pages.at(sPageName).m_TabControl.vTabs.size(); i++)
		{
			if (m_Pages.at(sPageName).m_TabControl.vTabs[i].sCaption == m_Pages.at(sPageName).m_sCurrentTab)
			{
				if (iUniqueID != 444)	// Add exception for tab line so it remains visible whatever tab is selected TO-DO: remove magic number
				{
					m_Pages.at(sPageName).m_TabControl.vTabs[i].m_Controls.insert(std::pair<int, HWND>(iUniqueID, NULL));
				}
			}
		}
	}
} // handleTabControls

void cui_rawImpl::captureControls(const std::basic_string<TCHAR> &sPageName, int iUniqueID, HWND hWnd)
{
	try
	{
		m_vControls.at(sPageName).push_back(hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// check if there is a tab control in this page
	if (m_Pages.at(sPageName).m_TabControl.iUniqueID != 123)	// TO-DO: remove magic number
	{
		for (size_t i = 0; i < m_Pages.at(sPageName).m_TabControl.vTabs.size(); i++)
		{
			for (auto &m_it : m_Pages.at(sPageName).m_TabControl.vTabs[i].m_Controls)
			{
				if (m_it.first == iUniqueID)
				{
					m_it.second = hWnd;
				}
			}
		}
	}
} // captureControls

void cui_rawImpl::capturePagelessControls(int iUniqueID, HWND hWnd)
{
	try
	{
		m_vPagelessControls.at(m_sTitle).push_back(hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // capturePagelessControls

void cui_rawImpl::hideControl(const std::basic_string<TCHAR> &sPageName, HWND hWnd)
{
	try
	{
		if (hWnd)
		{
			std::basic_string<TCHAR> sPageLessKey;

			bool bPageless = false;

			if (sPageName.empty())
			{
				sPageLessKey = m_sTitle;
				bPageless = true;
			}

			m_vHiddenControls.at(sPageName + sPageLessKey).push_back(hWnd);
			ShowWindow(hWnd, SW_HIDE);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // hideControl

void cui_rawImpl::showControl(const std::basic_string<TCHAR> &sPageName, HWND hWnd)
{
	try
	{
		std::basic_string<TCHAR> sPageLessKey;

		bool bPageless = false;

		if (sPageName.empty())
		{
			sPageLessKey = m_sTitle;
			bPageless = true;
		}

		std::vector<HWND> m_HWNDs;

		for (size_t i = 0; i < m_vHiddenControls.at(sPageName + sPageLessKey).size(); i++)
		{
			if (m_vHiddenControls.at(sPageName + sPageLessKey)[i] != hWnd)
			{
				m_HWNDs.push_back(m_vHiddenControls.at(sPageName + sPageLessKey)[i]);
			}
		}

		m_vHiddenControls.at(sPageName + sPageLessKey) = m_HWNDs;

		if ((sPageName + sPageLessKey) == m_sCurrentPage || bPageless)
		{
			// check if this page has a tab control
			if (m_Pages.at(sPageName + sPageLessKey).m_TabControl.iUniqueID == -123)
			{
				ShowWindow(hWnd, SW_SHOWNA);
			}
			else
			{
				// check if this control is in any tab control
				bool bInAnyTab = false;

				for (size_t i = 0; i < m_Pages.at(sPageName + sPageLessKey).m_TabControl.vTabs.size(); i++)
				{
					for (auto &it : m_Pages.at(sPageName + sPageLessKey).m_TabControl.vTabs[i].m_Controls)
					{
						if (it.second == hWnd)
						{
							bInAnyTab = true;
							break;
						}
					}

					if (bInAnyTab)
						break;
				}

				if (bInAnyTab)
				{
					// check if this control is in the selected tab
					bool bInSelectedTab = false;

					for (size_t i = 0; i < m_Pages.at(sPageName + sPageLessKey).m_TabControl.vTabs.size(); i++)
					{
						if (m_Pages.at(sPageName + sPageLessKey).m_TabControl.vTabs[i].bSelected)
						{
							for (auto &it : m_Pages.at(sPageName + sPageLessKey).m_TabControl.vTabs[i].m_Controls)
							{
								if (it.second == hWnd)
									bInSelectedTab = true;
							}
						}
					}

					if (bInSelectedTab)
						ShowWindow(hWnd, SW_SHOWNA);
				}
				else
					ShowWindow(hWnd, SW_SHOWNA);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // showControl

void cui_rawImpl::showTrayPopupMenu()
{
	if (!m_trayIcon.m_trayItems.empty())
	{
		// add menu items
		HMENU hPop = CreatePopupMenu();

		int iDefault = -1;

		for (UINT i = 0; i < m_trayIcon.m_trayItems.size(); i++)
		{
			if (!m_trayIcon.m_trayItems[i].sLabel.empty())
				InsertMenu(hPop, i, MF_BYPOSITION | MF_STRING,
					m_trayIcon.m_trayItems[i].iUniqueID,
					m_trayIcon.m_trayItems[i].sLabel.c_str());
			else
				InsertMenu(hPop, i, MF_BYPOSITION | MF_SEPARATOR, NULL, _T(""));

			if (m_trayIcon.m_trayItems[i].bDefault)
				iDefault = m_trayIcon.m_trayItems[i].iUniqueID;
		}

		// set default menu item
		if (iDefault > 0)
			SetMenuDefaultItem(hPop, iDefault, FALSE);

		// disable menu items
		for (auto &it : m_trayIcon.m_trayItems)
		{
			if (!it.bEnabled && !it.sLabel.empty())
				EnableMenuItem(hPop, it.iUniqueID, MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		}

		// show popup menu

		// get cursor position to create popup menu there
		POINT pt;
		GetCursorPos(&pt);

		// display menu and wait for selection
		WORD cmd = TrackPopupMenu(
			hPop,
			TPM_LEFTALIGN | TPM_RIGHTBUTTON | TPM_RETURNCMD | TPM_NONOTIFY,
			pt.x, pt.y, 0, m_hWnd, NULL);

		// destroy the popup menu
		DestroyMenu(hPop);

		// send message mapped to the selected item
		PostMessage(m_hWnd, WM_COMMAND, cmd, 0);
	}
} // showTrayPopupMenu

HWND cui_rawImpl::getControlHWND(const std::basic_string<TCHAR> &sPageName, std::vector<HWND> &vHWNDs, int iUniqueID)
{
	vHWNDs.clear();

	try
	{
		if (iUniqueID == m_iIDClose || iUniqueID == m_iIDMax || iUniqueID == m_iIDMin)
			return (m_ControlBtns.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = m_sTitle;

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_RectControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_RectControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_RectControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_ButtonControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_ButtonControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_ButtonControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_EditControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_EditControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_EditControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_listviewControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_listviewControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_ToggleButtonControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_ToggleButtonControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_ToggleButtonControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).hWnd);
	}
	catch (std::exception& e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_ImageControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_ImageControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_DateControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_DateControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_DateControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_TimeControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_TimeControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_TimeControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_TextControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_TextControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_TextControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_PasswordStrengthControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_PasswordStrengthControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_PasswordStrengthControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.end())
			return (m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.at(iUniqueID).hWnd);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	try
	{
		if (m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.find(iUniqueID) != m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.end())
		{
			RichEditControl ctrl = m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.at(iUniqueID);

			// TO-DO: watch out for recursion mess
			std::vector<HWND> m_vHWNDs;

			if (ctrl.bBorder)
			{
				// get HWNDs of border lines
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iIDLineLeft));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iIDLineRight));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iIDLineTop));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iIDLineBottom));
			}

			if (!ctrl.bReadOnly)
			{
				// get HWNDs of rich edit controls
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iIDHSeparator));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iIDVSeparator));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iIDFontList));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iIDFontSize));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iIDFontLabel));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iIDParagraphLabel));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iBold));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iItalic));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iUnderline));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iStrikethough));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iSubscript));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iSuperscript));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iLarger));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iLarger));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iSmaller));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iFontColor));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iLeftAlign));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iCenterAlign));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iRightAlign));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iJustify));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iList));
				vHWNDs.push_back(getControlHWND(sPageName, m_vHWNDs, ctrl.iListType));
			}

			return (m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.at(iUniqueID).hWnd);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	return NULL;
} // getControlHWND

int cui_rawImpl::GetNewMessageID()
{
	iAddToWM_APP++;
	return WM_APP + iAddToWM_APP;
}
