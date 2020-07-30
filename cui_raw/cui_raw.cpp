/*
** cui_raw.cpp - cui_raw framework - implemetation
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

#include "cui_raw.h"
#include "cui_rawImpl/cui_rawImpl.h"
#include "CImage/CImage.h"
#include "clrAdjust/clrAdjust.h"
#include "scaleAdjust/scaleAdjust.h"
#include "cui_rawImpl/RichEdit/RichEdit.h"
#include <map>
#include <algorithm>
#include <vector>	// for
#include <random>	// for
#include <iterator>	// for

#include <fstream>

#pragma comment(lib, "GdiPlus.lib")

// rich edit
#include "RichEdit.h"

#include "Error/Error.h"

// commandline
#include "CCmdLine/CCmdLine.h"

#include <Windows.h>

using namespace liblec::cui::gui_raw;

std::basic_string<TCHAR> cui_raw::GetVersion()
{
	std::string version = liblec::cui::version();
	return std::basic_string<TCHAR>(version.begin(), version.end());
} // GetVersion

cui_raw::cui_raw(
	const std::basic_string<TCHAR> &sPageName,
	CommandProcedure pCommandProc,
	COLORREF clrBackground,
	COLORREF clrTheme,
	COLORREF clrThemeHot,
	COLORREF clrDisabled,
	const std::basic_string<TCHAR> &sTooltipFont,
	double iTooltipFontSize,
	COLORREF clrTooltipText,
	COLORREF clrTooltipBackground,
	COLORREF clrTooltipBorder,
	HMODULE hResModule,
	cui_raw* pParent,
	void* pState
)
{
	d = new cui_rawImpl;

	d->m_sCurrentPage = sPageName;

	// capture title
	d->m_sTitle = sPageName;

	// add page
	d->m_Pages[d->m_sCurrentPage];

	// set page's command procedure
	d->m_Pages[d->m_sCurrentPage].m_pCommandProc = pCommandProc;

	// set window colors
	d->m_clrBackground = clrBackground;
	d->m_hbrBackground = CreateSolidBrush(clrBackground);
	d->m_clrTheme = clrTheme;
	d->m_clrThemeHot = clrThemeHot;
	d->m_clrDisabled = clrDisabled;

	// capture tooltip parameters
	d->m_sTooltipFont = sTooltipFont;
	d->m_iTooltipFontSize = iTooltipFontSize;
	d->m_clrTooltipText = clrTooltipText;
	d->m_clrTooltipBackground = clrTooltipBackground;
	d->m_clrTooltipBorder = clrTooltipBorder;

	// calculated lightened theme color
	d->m_clrThemeLight = clrLighten(clrTheme, 20);

	// calculate darkened theme color
	d->m_clrThemeDarker = clrDarken(clrTheme, 20);

	if (hResModule)
		d->m_hResModule = hResModule;
	else
		d->m_hResModule = GetModuleHandle(NULL);	// failsafe

	if (pParent)
	{
		// copy the parent's shadow status
		d->m_bShadow = pParent->d->m_bShadow;

		// get parent window's handle (if available)
		d->m_hWndParent = pParent->d->m_hWnd;

		// this is a child window; add it to the parent's list of child windows.
		d->m_pcui_rawparent = pParent;
		d->m_pcui_rawparent->d->m_Children.insert(std::pair<cui_raw*, cui_raw*>(this, this));
	}

	// capture user supplied state info
	d->pState_user = pState;

	// load rich edit library
	d->sRichEditClass = MSFTEDIT_CLASS;	// MSFTEDIT_CLASS

	/*
	** load the dll don't forget this
	** and don't forget to free it in when done
	*/
	d->hRichEdit = LoadLibrary(_T("msftedit.dll"));
	
	if (d->hRichEdit == NULL)
	{
		d->sRichEditClass = RICHEDIT_CLASS;

		// try version 3.0
		d->hRichEdit = LoadLibrary(_T("Riched20.dll"));
		
		if (d->hRichEdit == NULL)
		{
			// Failed to load rich edit library
		}
	}
}

bool liblec::cui::gui_raw::cui_raw::addFont(const std::basic_string<TCHAR> &sFontFullPath,
	std::basic_string<TCHAR> &sErr)
{
	Gdiplus::Status status = d->m_font_collection.AddFontFile(sFontFullPath.c_str());

	if (status == Gdiplus::Status::Ok)
	{
		d->m_font_collection_files.push_back(sFontFullPath);
		return true;
	}
	else
	{
		sErr.assign(GetGdiplusStatusInfo(&status));
		return false;
	}
} // addFont

std::vector<std::basic_string<TCHAR>> liblec::cui::gui_raw::cui_raw::get_font_files()
{
	return d->m_font_collection_files;
}

cui_raw::~cui_raw()
{
	// remove tray icon
	removeTrayIcon();

	// wait for all notification windows to close
	if (!d->m_bNotification)
	{
		/// <summary>
		/// Loop until there's absolutely NO OTHER notification thread running.
		/// This will make sure the user sees all pending notifications that 
		/// hadn't had an opportunity to be displayed.
		/// </summary>
		while (true)
		{
			// get a handle to any thread that's currently running
			HANDLE* pHandle = NULL;

			try
			{
				CCriticalSectionLocker locker(d->m_locker_for_m_nots);

				for (auto &it : d->m_nots)
				{
					if (it.second.hThread)
					{
						pHandle = &it.second.hThread;
						break;
					}
				}
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
			}

			if (pHandle)
				WaitForSingleObject(*pHandle, INFINITE);
			else
				break;
		}

		// cleanup
		CCriticalSectionLocker locker(d->m_locker_for_m_nots);

		for (auto &it : d->m_nots)
		{
			if (it.second.pcui_raw)
			{
				delete it.second.pcui_raw;
				it.second.pcui_raw = NULL;
			}
		}
	}

	// failsafe
	if (IsWindow(d->m_hWnd))
	{
		DestroyWindow(d->m_hWnd);

		// enable parent
		if (IsWindow(d->m_hWndParent) && d->parent_was_enabled)
			EnableWindow(d->m_hWndParent, TRUE);
	}

	if (d->m_pcui_rawparent)
	{
		// this is a child window; remove it from the parent's list of child windows.
		d->m_pcui_rawparent->d->m_Children.erase(this);
	}

	// release the rich edit control DLL
	if (d->hRichEdit)
		FreeLibrary(d->hRichEdit);

	if (d)
	{
		delete d;
		d = NULL;
	}
}

template<typename iter, typename RandomGenerator>
iter select_randomly(iter start, iter end, RandomGenerator& g)
{
	std::uniform_int_distribution<>dis(0, std::distance(start, end) - 1);
	std::advance(start, dis(g));
	return start;
} // select_randomly

template<typename iter>
iter select_randomly(iter start, iter end)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	return select_randomly(start, end, gen);
} // select_randomly

template<typename type>
void randFromVec(const std::vector<type> &vIn, type &out)
{
	if (vIn.empty())
		return;
	else
	{
		out = *select_randomly(vIn.begin(), vIn.end());
		return;
	}
} // randFromVec

COLORREF randomColor(bool bDarkColors)
{
	std::vector<int> vValues;

	for (int i = 1; i < 256; i++)
		vValues.push_back(i);

	while (true)
	{
		// make a random color
		int r = 0;
		randFromVec(vValues, r);

		int g = 0;
		randFromVec(vValues, g);

		int b = 0;
		randFromVec(vValues, b);

		if (bDarkColors && r * g * b > 150 * 150 * 150)
			continue;

		return RGB(r, g, b);
	}

	return RGB(0, 0, 0);	// should never ever get here, ever!
} // randomColor

void cui_raw::posRect(RECT &rcInOut, const RECT &rcTarget, int iPercH, int iPercV)
{
	RECT rcIn = rcInOut;

	int iDeltaX = (rcTarget.right - rcTarget.left) - (rcIn.right - rcIn.left);
	rcInOut.left = rcTarget.left + (iPercH * iDeltaX) / 100;

	int iDeltaY = (rcTarget.bottom - rcTarget.top) - (rcIn.bottom - rcIn.top);
	rcInOut.top = rcTarget.top + (iPercV * iDeltaY) / 100;

	rcInOut.right = rcInOut.left + (rcIn.right - rcIn.left);
	rcInOut.bottom = rcInOut.top + (rcIn.bottom - rcIn.top);
} // posRect

void cui_raw::setPosition(
	int ix,
	int iy,
	int icx,
	int icy
)
{
	icx = int(0.5 + icx * d->m_DPIScale);
	icy = int(0.5 + icy * d->m_DPIScale);

	// TO-DO: consider whether to factor DPI scaling in x and y coordinates
	d->m_ix = ix;
	d->m_iy = iy;

	d->m_icx = icx;
	d->m_icy = icy;
} // setPosition

void cui_raw::setPosition(
	windowPosition wndPos,
	int icx,
	int icy
)
{
	icx = int(0.5 + icx * d->m_DPIScale);
	icy = int(0.5 + icy * d->m_DPIScale);

	// get coordinates of working area
	RECT rcWork;
	d->GetWorkingArea(GetDesktopWindow(), rcWork);

	cui_raw::windowPosition m_wndPos = wndPos;

	if (wndPos == cui_raw::centerToParent && d->m_hWndParent && IsWindow(d->m_hWndParent) && (!IsWindowVisible(d->m_hWndParent) | IsIconic(d->m_hWndParent)))
		m_wndPos = cui_raw::centerToWorkingArea;

	switch (m_wndPos)
	{
	case cui_raw::centerToWorkingArea:
	{
		int user_width = rcWork.right - rcWork.left;
		int user_height = rcWork.bottom - rcWork.top;

		int ix = 0;
		int iy = 0;

		// center to working area
		ix = rcWork.left + ((user_width - icx) / 2);
		iy = rcWork.top + ((user_height - icy) / 2);

		// ensure visibility of top left
		if (icx > user_width)
			ix = rcWork.left;

		if (icy > user_height)
			iy = rcWork.top;

		d->m_ix = ix;
		d->m_iy = iy;
		d->m_icx = icx;
		d->m_icy = icy;
	}
	break;

	case centerToParent:
	{
		if (d->m_hWndParent && IsWindow(d->m_hWndParent))
		{
			// get coordinates of parent window
			RECT rcParent;
			GetWindowRect(d->m_hWndParent, &rcParent);

			int user_width = rcParent.right - rcParent.left;
			int user_height = rcParent.bottom - rcParent.top;

			int ix = 0;
			int iy = 0;

			// center to working area
			ix = rcParent.left + ((user_width - icx) / 2);
			iy = rcParent.top + ((user_height - icy) / 2);

			// ensure visibility of top left
			if (icx > user_width)
				ix = rcParent.left;

			if (icy > user_height)
				iy = rcParent.top;

			if (ix < 0)	// failsafe ... like when this is somehow called while parent is minimized
				ix = 0;

			if (iy < 0)	// failsafe ... like when this is somehow called while parent is minimized
				iy = 0;

			d->m_ix = ix;
			d->m_iy = iy;
			d->m_icx = icx;
			d->m_icy = icy;
		}
	}
	break;

	case topLeft:
	{
		d->m_ix = rcWork.left;
		d->m_iy = rcWork.top;
		d->m_icx = icx;
		d->m_icy = icy;
	}
	break;

	case topLeftOffset:
	{
		d->m_ix = rcWork.left + 10;
		d->m_iy = rcWork.top + 10;
		d->m_icx = icx;
		d->m_icy = icy;
	}
	break;

	case topRight:
	{
		d->m_ix = rcWork.right - icx;
		d->m_iy = rcWork.top;
		d->m_icx = icx;
		d->m_icy = icy;
	}
	break;

	case topRightOffset:
	{
		d->m_ix = rcWork.right - icx - 10;
		d->m_iy = rcWork.top + 10;
		d->m_icx = icx;
		d->m_icy = icy;
	}
	break;

	case bottomRight:
	{
		d->m_ix = rcWork.right - icx;
		d->m_iy = rcWork.bottom - icy;
		d->m_icx = icx;
		d->m_icy = icy;
	}
	break;

	case bottomRightOffset:
	{
		d->m_ix = rcWork.right - icx - 10;
		d->m_iy = rcWork.bottom - icy - 10;
		d->m_icx = icx;
		d->m_icy = icy;
	}
	break;

	case bottomLeft:
	{
		d->m_ix = rcWork.left;
		d->m_iy = rcWork.bottom - icy;
		d->m_icx = icx;
		d->m_icy = icy;
	}
	break;

	case bottomLeftOffset:
	{
		d->m_ix = rcWork.left + 10;
		d->m_iy = rcWork.bottom - icy - 10;
		d->m_icx = icx;
		d->m_icy = icy;
	}
	break;

	default:
	{
		// default to top left
		d->m_ix = rcWork.left;
		d->m_iy = rcWork.top;
		d->m_icx = icx;
		d->m_icy = icy;
	}
	break;
	}
} // setPosition

void cui_raw::addButton(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::basic_string<TCHAR> &sTooltip,
	const std::basic_string<TCHAR> &sCaption,
	const std::basic_string<TCHAR> &sFontName,
	double iFontSize,
	RECT rc,
	onResize resize,
	bool bDefaultButton
)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::ButtonControl control;
	control.iUniqueID = iUniqueID;

	control.toolTip.sText = sTooltip;
	control.toolTip.sFontName = d->m_sTooltipFont;
	control.toolTip.iFontSize = d->m_iTooltipFontSize;
	control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
	control.toolTip.m_clrText = d->m_clrTooltipText;
	control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
	control.toolTip.d = d;

	control.sCaption = sCaption;
	control.sFontName = sFontName;
	control.iFontSize = iFontSize;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.bDefaultButton = bDefaultButton;
	control.resize = resize;

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ButtonControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_ButtonControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_ButtonControls.insert(std::pair<int, cui_rawImpl::ButtonControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addButton

void cui_raw::hideShadow()
{
	if (d)
		d->m_bShadow = false;
} // hideShadow

  /// <summary>
  /// Enable a window.
  /// </summary>
  /// 
  /// <param name="hWnd">
  /// Native handle of the window.
  /// </param>
  /// 
  /// <remarks>
  /// This function makes sure the control is drawn completely before exiting by processing
  /// all messages in the message queue before exiting.
  /// </remarks>
void enableWindow(HWND hWnd)
{
	EnableWindow(hWnd, TRUE);

	// process all messages so control is drawn immediately ... this is essential for aesthetic purposes
	MSG uMsg = {};

	while (PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE))
	{
		if (uMsg.message == WM_QUIT)
		{
			PostQuitMessage(0);
			return;
		}
		else
		{
			TranslateMessage(&uMsg);
			DispatchMessage(&uMsg);
		}
	}
} // enableWindow

  /// <summary>
  /// Disable a window.
  /// </summary>
  /// 
  /// <param name="hWnd">
  /// Native handle of the window.
  /// </param>
  /// 
  /// <remarks>
  /// This function makes sure the control is drawn completely before exiting by processing
  /// all messages in the message queue before exiting.
  /// </remarks>
void disableWindow(HWND hWnd)
{
	EnableWindow(hWnd, FALSE);

	MSG uMsg = {};

	while (PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE))
	{
		if (uMsg.message == WM_QUIT)
		{
			PostQuitMessage(0);
			return;
		}
		else
		{
			TranslateMessage(&uMsg);
			DispatchMessage(&uMsg);
		}
	}
} // disableWindow

void cui_raw::enableControl(const std::basic_string<TCHAR> &sPageName, int iUniqueID)
{
	if (d)
	{
		// check if this is a menu item
		for (auto &it : d->m_trayIcon.m_trayItems)
		{
			if (iUniqueID == it.iUniqueID)
			{
				it.bEnabled = true;
				return;
			}
		}

		std::vector<HWND> vHWNDs;
		HWND hWnd = d->getControlHWND(sPageName, vHWNDs, iUniqueID);

		if (hWnd)
			enableWindow(hWnd);

		for (auto &it : vHWNDs)
			enableWindow(it);
	}
} // enableControl

void cui_raw::disableControl(const std::basic_string<TCHAR> &sPageName, int iUniqueID)
{
	if (d)
	{
		// check if this is a menu item
		for (auto &it : d->m_trayIcon.m_trayItems)
		{
			if (iUniqueID == it.iUniqueID)
			{
				it.bEnabled = false;
				return;
			}
		}

		std::vector<HWND> vHWNDs;
		HWND hWnd = d->getControlHWND(sPageName, vHWNDs, iUniqueID);

		if (hWnd)
			disableWindow(hWnd);

		for (auto &it : vHWNDs)
			disableWindow(it);
	}
} // disableControl

bool cui_raw::controlEnabled(const std::basic_string<TCHAR> &sPageName, int iUniqueID)
{
	if (d)
	{
		std::vector<HWND> vHWNDs;
		HWND hWnd = d->getControlHWND(sPageName, vHWNDs, iUniqueID);

		if (hWnd)
			return IsWindowEnabled(hWnd) == TRUE;
		else
			return false;
	}
	else
		return false;
} // controlEnabled

void cui_raw::hideControl(const std::basic_string<TCHAR> &sPageName, int iUniqueID)
{
	if (d)
	{
		try
		{
			{
				std::basic_string<TCHAR> sPageLessKey;

				if (sPageName.empty())
					sPageLessKey = d->m_sTitle;

				// check if this control is a tab control
				if (d->m_Pages.at(sPageName + sPageLessKey).m_TabControl.iUniqueID == iUniqueID)
				{
					// hide controls within tabs
					for (size_t i = 0; i < d->m_Pages.at(sPageName + sPageLessKey).m_TabControl.vTabs.size(); i++)
					{
						for (auto &it : d->m_Pages.at(sPageName + sPageLessKey).m_TabControl.vTabs[i].m_Controls)
							ShowWindow(it.second, SW_HIDE);
					}

					// hide the tab control itself
					d->m_vHiddenControls.at(sPageName + sPageLessKey).push_back(d->m_Pages.at(sPageName + sPageLessKey).m_TabControl.hWnd);
					ShowWindow(d->m_Pages.at(sPageName + sPageLessKey).m_TabControl.hWnd, SW_HIDE);

					// hide the tab control's line
					for (auto it : d->m_Pages.at(sPageName + sPageLessKey).m_RectControls)
					{
						if (it.second.iUniqueID == 444)
						{
							d->m_vHiddenControls.at(sPageName + sPageLessKey).push_back(it.second.hWnd);
							ShowWindow(it.second.hWnd, SW_HIDE);
							break;
						}
					}

					return;
				}
			}

			std::vector<HWND> vHWNDs;
			HWND hWnd = d->getControlHWND(sPageName, vHWNDs, iUniqueID);

			if (hWnd)
				d->hideControl(sPageName, hWnd);

			for (auto &it : vHWNDs)
				d->hideControl(sPageName, it);
		}
		catch (std::exception &e)
		{
			std::string m_sErr = e.what();
		}
	}
} // hideControl

void cui_raw::showControl(const std::basic_string<TCHAR> &sPageName, int iUniqueID)
{
	if (d)
	{
		try
		{
			{
				std::basic_string<TCHAR> sPageLessKey;

				if (sPageName.empty())
					sPageLessKey = d->m_sTitle;

				// check if this control is a tab control
				if (d->m_Pages.at(sPageName + sPageLessKey).m_TabControl.iUniqueID == iUniqueID)
				{
					std::vector<HWND> *pVHidden = &d->m_vHiddenControls.at(sPageName);

					// show controls within tabs
					for (size_t i = 0; i < d->m_Pages.at(sPageName + sPageLessKey).m_TabControl.vTabs.size(); i++)
					{
						DWORD dwCmd = SW_HIDE;

						if (d->m_Pages.at(sPageName).m_TabControl.vTabs[i].bSelected)
							dwCmd = SW_SHOW;

						HWND hWnd = NULL;

						// show only the controls in the selected tab
						if (d->m_Pages.at(sPageName + sPageLessKey).m_TabControl.vTabs[i].bSelected)
						{
							for (auto it : d->m_Pages.at(sPageName + sPageLessKey).m_TabControl.vTabs[i].m_Controls)
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

					// get the tab control line's HWND
					HWND hwndTabLine = NULL;
					for (auto it : d->m_Pages.at(sPageName + sPageLessKey).m_RectControls)
					{
						if (it.second.iUniqueID == 444)
						{
							hwndTabLine = it.second.hWnd;
							break;
						}
					}

					// remove tab control from the list of hidden controls
					std::vector<HWND> m_HWNDs;

					for (size_t i = 0; i < d->m_vHiddenControls.at(sPageName + sPageLessKey).size(); i++)
					{
						if (
							d->m_vHiddenControls.at(sPageName + sPageLessKey)[i] != d->m_Pages.at(sPageName + sPageLessKey).m_TabControl.hWnd &&
							d->m_vHiddenControls.at(sPageName + sPageLessKey)[i] != hwndTabLine
							)
							m_HWNDs.push_back(d->m_vHiddenControls.at(sPageName + sPageLessKey)[i]);
					}

					d->m_vHiddenControls.at(sPageName + sPageLessKey) = m_HWNDs;

					// show the tab control itself
					ShowWindow(d->m_Pages.at(sPageName + sPageLessKey).m_TabControl.hWnd, SW_SHOWNA);

					// show the tab control's line
					ShowWindow(hwndTabLine, SW_SHOWNA);

					return;
				}
			}

			std::vector<HWND> vHWNDs;
			HWND hWnd = d->getControlHWND(sPageName, vHWNDs, iUniqueID);

			if (hWnd)
				d->showControl(sPageName, hWnd);

			for (auto &it : vHWNDs)
				d->showControl(sPageName, it);
		}
		catch (std::exception &e)
		{
			std::string m_sErr = e.what();
		}
	}
} // showControl

bool cui_raw::controlVisible(const std::basic_string<TCHAR> &sPageName, int iUniqueID)
{
	if (d)
	{
		std::vector<HWND> vHWNDs;
		HWND hWnd = d->getControlHWND(sPageName, vHWNDs, iUniqueID);

		if (hWnd)
			return IsWindowVisible(hWnd) == TRUE;
		else
			return false;
	}
	else
		return false;
} // controlVisible

void cui_raw::setFocus(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID)
{
	if (d)
	{
		std::vector<HWND> vHWNDs;
		HWND hWnd = d->getControlHWND(sPageName, vHWNDs, iUniqueID);

		if (hWnd)
			SetFocus(hWnd);
	}
} // setFocus

void cui_raw::enable()
{
	if (d->m_hWnd)
		EnableWindow(d->m_hWnd, TRUE);
}

void cui_raw::disable()
{
	if (d->m_hWnd)
		EnableWindow(d->m_hWnd, FALSE);
}

bool cui_raw::isEnabled()
{
	if (d->m_hWnd)
		return IsWindowEnabled(d->m_hWnd) == TRUE;
	else
		return false;
}

bool cui_raw::isVisible()
{
	if (d->m_hWnd)
		return IsWindowVisible(d->m_hWnd) == TRUE;
	else
		return false;
}

bool cui_raw::isMessageBoxDisplayed()
{
	return d->m_iMessageBoxes > 0;
}

void cui_raw::setTimer(int iUniqueID, unsigned int iMilliSeconds)
{
	if (d->m_hWnd)
	{
		if (d->m_Timers.find(iUniqueID) == d->m_Timers.end())
		{
			d->m_Timers.insert(std::pair<int, int>(iUniqueID, iUniqueID));
			SetTimer(d->m_hWnd, (UINT_PTR)iUniqueID, (UINT)iMilliSeconds, NULL);
		}
	}
} // setTimer

bool cui_raw::timerRunning(int iUniqueID)
{
	if (d->m_hWnd)
	{
		if (d->m_Timers.find(iUniqueID) != d->m_Timers.end())
			return true;
	}

	return false;
} // timerRunning

void cui_raw::stopTimer(int iUniqueID)
{
	if (d->m_hWnd)
	{
		if (d->m_Timers.find(iUniqueID) != d->m_Timers.end())
		{
			KillTimer(d->m_hWnd, (UINT_PTR)iUniqueID);
			d->m_Timers.erase(iUniqueID);
		}
	}
}

void cui_raw::addText(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID, const std::basic_string<TCHAR> &sText,
	bool bStatic,
	COLORREF clrText,
	COLORREF clrTextHot,
	const std::basic_string<TCHAR> &sTooltip,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	RECT rc,
	textAlignment align,
	onResize resize,
	bool bMultiLine
)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::TextControl control;
	control.d = d;
	control.iUniqueID = iUniqueID;
	control.sText = sText;
	control.sTextDisplay = sText;
	control.bStatic = bStatic;
	control.sFontName = sFontName;
	control.iFontSize = iFontSize;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.clrText = clrText;
	control.clrTextHot = clrTextHot;
	control.align = align;
	control.bMultiLine = bMultiLine;
	control.resize = resize;

	control.toolTip.sText = sTooltip;
	control.toolTip.sFontName = d->m_sTooltipFont;
	control.toolTip.iFontSize = d->m_iTooltipFontSize;
	control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
	control.toolTip.m_clrText = d->m_clrTooltipText;
	control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
	control.toolTip.d = d;

	/*
	** replace string
	** will replace "search" with "replace" in the string "subject"
	*/
	auto replaceString = [](
		std::basic_string<TCHAR> &s,
		const std::basic_string<TCHAR> &search,
		const std::basic_string<TCHAR> &replace
		)
	{
		size_t pos = 0;
		while ((pos = s.find(search, pos)) != std::string::npos) {
			s.replace(pos, search.length(), replace);
			pos += replace.length();
		}

		return;
	};

	// replace all occurences of the ampersand with double ampersand
	replaceString(control.sTextDisplay, _T("&"), _T("&&"));

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.insert(std::pair<int, cui_rawImpl::TextControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addText

bool cui_raw::getText(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	std::basic_string<TCHAR> &sText,
	std::basic_string<TCHAR> &sErr
)
{
	sText.clear();
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.end())
		{
			try
			{
				sText = d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.at(iUniqueID).sText;

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Text control not found");
	return false;
} // getText

bool cui_raw::setText(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::basic_string<TCHAR> &sText,
	std::basic_string<TCHAR> &sErr
)
{

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.end())
		{
			try
			{
				d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.at(iUniqueID).sText = sText;
				d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.at(iUniqueID).sTextDisplay = sText;

				/*
				** replace string
				** will replace "search" with "replace" in the string "subject"
				*/
				auto replaceString = [](
					std::basic_string<TCHAR> &s,
					const std::basic_string<TCHAR> &search,
					const std::basic_string<TCHAR> &replace
					)
				{
					size_t pos = 0;
					while ((pos = s.find(search, pos)) != std::string::npos) {
						s.replace(pos, search.length(), replace);
						pos += replace.length();
					}

					return;
				};

				// replace all occurences of the ampersand with double ampersand
				replaceString(d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.at(iUniqueID).sTextDisplay, _T("&"), _T("&&"));

				InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.at(iUniqueID).hWnd, NULL, TRUE);

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Text control not found");
	return false;
} // setText

bool cui_raw::setText(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::basic_string<TCHAR> &sText,
	COLORREF clrText,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.end())
		{
			try
			{
				d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.at(iUniqueID).sText = sText;
				d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.at(iUniqueID).sTextDisplay = sText;
				d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.at(iUniqueID).clrText = clrText;

				/*
				** replace string
				** will replace "search" with "replace" in the string "subject"
				*/
				auto replaceString = [](
					std::basic_string<TCHAR> &s,
					const std::basic_string<TCHAR> &search,
					const std::basic_string<TCHAR> &replace
					)
				{
					size_t pos = 0;
					while ((pos = s.find(search, pos)) != std::string::npos) {
						s.replace(pos, search.length(), replace);
						pos += replace.length();
					}

					return;
				};

				// replace all occurences of the ampersand with double ampersand
				replaceString(d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.at(iUniqueID).sTextDisplay, _T("&"), _T("&&"));

				InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_TextControls.at(iUniqueID).hWnd, NULL, TRUE);

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Text control not found");
	return false;
} // setText

void cui_raw::addCloseBtn(int iUniqueID)
{
	if (!d->m_bClosebtn)
		d->m_iIDClose = iUniqueID;

	d->m_bClosebtn = true;

	cui_rawImpl::ControlBtn control;
	control.iUniqueID = iUniqueID;
	control.clrBtn = d->m_clrTheme;
	control.clrBtnHot = d->m_clrThemeHot;
	control.bCustomHandle = false;
	control.d = d;

	control.toolTip.sText = _T("Close");
	control.toolTip.sFontName = d->m_sTooltipFont;
	control.toolTip.iFontSize = d->m_iTooltipFontSize;
	control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
	control.toolTip.m_clrText = d->m_clrTooltipText;
	control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
	control.toolTip.d = d;

	if (d->m_ControlBtns.find(iUniqueID) == d->m_ControlBtns.end())	// do not duplicate IDs
		d->m_ControlBtns.insert(std::pair<int, cui_rawImpl::ControlBtn>(iUniqueID, control));
} // addCloseBtn

void cui_raw::addCloseBtn(int iUniqueID, bool bCustomHandle)
{
	if (!d->m_bClosebtn)
		d->m_iIDClose = iUniqueID;

	d->m_bClosebtn = true;

	cui_rawImpl::ControlBtn control;
	control.iUniqueID = iUniqueID;
	control.clrBtn = d->m_clrTheme;
	control.clrBtnHot = d->m_clrThemeHot;
	control.bCustomHandle = bCustomHandle;
	control.d = d;

	control.toolTip.sText = _T("Close");
	control.toolTip.sFontName = d->m_sTooltipFont;
	control.toolTip.iFontSize = d->m_iTooltipFontSize;
	control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
	control.toolTip.m_clrText = d->m_clrTooltipText;
	control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
	control.toolTip.d = d;

	if (d->m_ControlBtns.find(iUniqueID) == d->m_ControlBtns.end())	// do not duplicate IDs
		d->m_ControlBtns.insert(std::pair<int, cui_rawImpl::ControlBtn>(iUniqueID, control));
} // addCloseBtn

void cui_raw::excludeFromTitleBar(const std::basic_string<TCHAR> &sPageName, int iUniqueID)
{
	if (d)
	{
		std::vector<HWND> vHWNDs;
		HWND hWnd = d->getControlHWND(sPageName, vHWNDs, iUniqueID);

		if (hWnd)
			d->m_rcExclude.push_back(hWnd);

		for (auto &it : vHWNDs)
			d->m_rcExclude.push_back(it);
	}
} // excludeRectFromTitleBar

void cui_raw::addMaxBtn(int iUniqueID)
{
	if (!d->m_bMaxbtn)
		d->m_iIDMax = iUniqueID;

	d->m_bMaxbtn = true;

	cui_rawImpl::ControlBtn control;
	control.iUniqueID = iUniqueID;
	control.clrBtn = d->m_clrTheme;
	control.clrBtnHot = d->m_clrThemeHot;
	control.d = d;

	control.toolTip.sText = _T("Maximize");
	control.toolTip.sFontName = d->m_sTooltipFont;
	control.toolTip.iFontSize = d->m_iTooltipFontSize;
	control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
	control.toolTip.m_clrText = d->m_clrTooltipText;
	control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
	control.toolTip.d = d;

	if (d->m_ControlBtns.find(iUniqueID) == d->m_ControlBtns.end())	// do not duplicate IDs
		d->m_ControlBtns.insert(std::pair<int, cui_rawImpl::ControlBtn>(iUniqueID, control));
} // addMaxBtn

void cui_raw::addMinBtn(int iUniqueID)
{
	if (!d->m_bMinbtn)
		d->m_iIDMin = iUniqueID;

	d->m_bMinbtn = true;

	cui_rawImpl::ControlBtn control;
	control.iUniqueID = iUniqueID;
	control.clrBtn = d->m_clrTheme;
	control.clrBtnHot = d->m_clrThemeHot;
	control.d = d;

	control.toolTip.sText = _T("Minimize");
	control.toolTip.sFontName = d->m_sTooltipFont;
	control.toolTip.iFontSize = d->m_iTooltipFontSize;
	control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
	control.toolTip.m_clrText = d->m_clrTooltipText;
	control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
	control.toolTip.d = d;

	if (d->m_ControlBtns.find(iUniqueID) == d->m_ControlBtns.end())	// do not duplicate IDs
		d->m_ControlBtns.insert(std::pair<int, cui_rawImpl::ControlBtn>(iUniqueID, control));
} // addMinBtn

void cui_raw::addComboBox(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::vector<std::basic_string<TCHAR>> &vItems,
	const std::basic_string<TCHAR> &sSelectedItem,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	RECT rc, onResize resize, bool bAutoComplete, bool bReadOnly)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::ComboBoxControl control;
	control.iUniqueID = iUniqueID;
	control.vData = vItems;
	control.sSelectedItem = sSelectedItem;
	control.sFontName = sFontName;
	control.iFontSize = iFontSize;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.bAutoComplete = bAutoComplete;
	control.bReadOnly = bReadOnly;
	control.pThis = this;
	control.resize = resize;
	control.d = d;

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.insert(std::pair<int, cui_rawImpl::ComboBoxControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addComboBox

bool cui_raw::getComboText(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	std::basic_string<TCHAR> &sText,
	std::basic_string<TCHAR> &sErr
)
{

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.end())
		{
			try
			{
				HWND hWnd = d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.at(iUniqueID).hWnd;

				auto getWinText = [&]()
				{
					int i = GetWindowTextLength(hWnd) + 1;

					TCHAR *buffer = new TCHAR[i];
					GetWindowText(hWnd, buffer, i);
					sText = buffer;
					delete buffer;

					return sText;
				}; // getWinText

				getWinText();

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Combobox control not found");
	return false;
} // getComboText

bool cui_raw::selectComboItem(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::basic_string<TCHAR> &sSelectedItem,
	std::basic_string<TCHAR> &sErr)
{

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.end())
		{
			try
			{
				// select item
				int nIndex = (int)::SendMessage(d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.at(iUniqueID).hWnd, CB_FINDSTRINGEXACT, NULL, LPARAM(sSelectedItem.c_str()));

				SendMessage(d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.at(iUniqueID).hWnd, CB_SETCURSEL, nIndex, NULL);

				if (nIndex == -1 && !sSelectedItem.empty())
				{
					sErr = _T("Item not found: '") + sSelectedItem + _T("'");
					return false;
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Combobox control not found");
	return false;
} // selectComboItem

bool cui_raw::setComboText(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::basic_string<TCHAR> &sText,
	std::basic_string<TCHAR> &sErr)
{

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.end())
		{
			try
			{
				// select item
				int nIndex = (int)::SendMessage(d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.at(iUniqueID).hWnd, CB_FINDSTRINGEXACT, NULL, LPARAM(sText.c_str()));

				SendMessage(d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.at(iUniqueID).hWnd, CB_SETCURSEL, nIndex, NULL);

				if (nIndex == -1 && !sText.empty())
				{
					if (!d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.at(iUniqueID).bReadOnly)
						SendMessage(d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.at(iUniqueID).hWnd, WM_SETTEXT, NULL, LPARAM(sText.c_str()));
					else
					{
						sErr = _T("Item not found: '") + sText + _T("'");
						return false;
					}
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Combobox control not found");
	return false;
} // setComboText

bool cui_raw::repopulateCombo(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::vector<std::basic_string<TCHAR>> &vItems,
	std::basic_string<TCHAR> &sErr
)
{

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.end())
		{
			try
			{
				HWND hCombo = d->m_Pages.at(sPageName + sPageLessKey).m_ComboBoxControls.at(iUniqueID).hWnd;

				// reset combobox
				SendMessage(
					hCombo,
					CB_RESETCONTENT,
					NULL,
					NULL
				);

				// populate combobox
				for (size_t x = 0; x < vItems.size(); x++)
				{
					SendMessage(
						hCombo,
						CB_ADDSTRING,
						NULL,
						LPARAM(vItems[x].c_str())
					);
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Combobox control not found");
	return false;
} // repopulateCombo

void cui_raw::addListview(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	std::vector<listviewColumn> vColumns,
	const std::basic_string<TCHAR> &sUniqueColumnName,
	std::vector<listviewRow> vData,
	std::vector<contextMenuItem> vContextMenu,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	RECT rc,
	cui_raw::onResize resize,
	bool bBorder,
	bool bGridLines,
	bool bSortByClickingColumn
)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::listviewControl control;
	control.iUniqueID = iUniqueID;
	control.vColumns = vColumns;
	control.sFontName = sFontName;
	control.iFontSize = iFontSize;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.vData = vData;
	control.vContextMenu = vContextMenu;
	control.bSortByClickingColumn = bSortByClickingColumn;
	control.bBorder = bBorder;
	control.bGridLines = bGridLines;
	control.resize = resize;
	control.d = d;
	control.sUniqueColumnName = sUniqueColumnName;

	// TO-DO: check if unique column name exists in the supplied columns and return an error if it doesn't

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.insert(std::pair<int, cui_rawImpl::listviewControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addListview

bool cui_raw::addListviewRow(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	listviewRow &vRow,
	bool bScrollToBottom,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.end())
		{
			try
			{
				// insert list view row
				int iNumberOfRows = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).pClistview->Get_NumOfRows();

				for (auto &it : vRow.vItems)
				{
					int iColumnNumber = -1;

					// determine column number
					for (size_t iColumnNames = 0; iColumnNames < d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vColumns.size(); iColumnNames++)
					{
						if (d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vColumns[iColumnNames].sColumnName == it.sColumnName)
						{
							iColumnNumber = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vColumns[iColumnNames].iColumnID;
							break;
						}
					}

					// insert item
					if (iColumnNumber != -1)	// failsafe in-case there's a typo in the column name
					{
						d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).pClistview->InsertItem(iNumberOfRows, iColumnNumber, it.sItemData);
					}
				}

				int iRowNumber = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vData.size();

				for (auto &it : vRow.vItems)
					it.iRowNumber = iRowNumber;

				d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vData.push_back(vRow);

				// scroll to bottom
				if (bScrollToBottom)
				{
					RECT rc;
					ListView_GetViewRect(d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).hWnd, &rc);
					int iHeight = rc.bottom - rc.top;
					ListView_Scroll(d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).hWnd, 0, iHeight);
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Listview control not found");
	return false;
} // addListviewRow

bool cui_raw::repopulateListview(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	std::vector<listviewRow> &vData,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.end())
		{
			try
			{
				// clear list view
				d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).pClistview->Clear();
				d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vData.clear();

				// populate listview
				int iRow = 0;
				for (auto &it : vData)
				{
					for (auto &m_it : it.vItems)
					{
						m_it.iRowNumber = iRow;

						std::basic_string<TCHAR> sColumnName = m_it.sColumnName;
						std::basic_string<TCHAR> sItemData = m_it.sItemData;

						int iRowNumber = m_it.iRowNumber;
						int iColumnNumber = -1;

						// determine column number
						for (size_t iColumnNames = 0; iColumnNames < d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vColumns.size(); iColumnNames++)
						{
							if (d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vColumns[iColumnNames].sColumnName == sColumnName)
							{
								iColumnNumber = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vColumns[iColumnNames].iColumnID;
								break;
							}
						}

						// insert item
						if (iColumnNumber != -1)	// failsafe in-case there's a typo in the column name
						{
							d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).pClistview->InsertItem(iRowNumber, iColumnNumber, sItemData);
						}
					}

					iRow++;
				}

				// update data
				d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vData = vData;

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Listview control not found");
	return false;
} // repopulateListview

bool cui_raw::updateListViewItem(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	listviewItem item,
	const listviewRow &row,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.end())
		{
			try
			{
				int iColumnNumber = -1;

				int iUniqueColumnNumber = -1;

				// determine column number
				for (size_t iColumnNames = 0; iColumnNames < d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vColumns.size(); iColumnNames++)
				{
					if (d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vColumns[iColumnNames].sColumnName == item.sColumnName)
					{
						iColumnNumber = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vColumns[iColumnNames].iColumnID;
						break;
					}
				}

				// determine unique column number
				for (size_t iColumnNames = 0; iColumnNames < d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vColumns.size(); iColumnNames++)
				{
					if (
						d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vColumns[iColumnNames].sColumnName ==
						d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).sUniqueColumnName)
					{
						iUniqueColumnNumber = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vColumns[iColumnNames].iColumnID;
						break;
					}
				}

				int iRowNumber = item.iRowNumber;	// this will have changed if listview has been sorted ...

													// determine proper row number
				{
					// get number of columns
					const int iCols = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).pClistview->Get_NumOfCols();

					// get number of rows
					const int iRows = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).pClistview->Get_NumOfRows();

					LVITEM lv = { 0 };

					HWND m_hlistview = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).hWnd;

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
									if (it.sColumnName != d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).sUniqueColumnName)
										continue;

									if (it.sItemData == sText)
										bMatch = true;
								}
							}
						}

						if (bMatch)
						{
							// this is our target row
							iRowNumber = iRow;
							break;
						}
					}
				}

				// update item
				if (iColumnNumber != -1)	// failsafe in-case there's a typo in the column name
				{
					d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).pClistview->UpdateItem(iRowNumber, iColumnNumber, item.sItemData);

					// update entry
					bool updated = false;
					for (auto &it : d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vData)
					{
						for (auto &m_it : it.vItems)
						{
							if (m_it.iRowNumber != iRowNumber)
								break;

							if (m_it.sColumnName == item.sColumnName)
							{
								m_it = item;	// do the update
								updated = true;
								break;
							}
						}

						if (updated)
							break;
					}
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Listview control not found");
	return false;
} // updateListViewItem

bool cui_raw::removeListViewRow(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	int iRowNumber,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.end())
		{
			try
			{
				std::vector<listviewRow> vData_new;

				for (auto &it : d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vData)
				{
					bool bSkip = false;

					for (auto &m_it : it.vItems)
					{
						if (m_it.iRowNumber == iRowNumber)
						{
							bSkip = true;
							break;
						}
					}

					if (!bSkip)
						vData_new.push_back(it);
				}

				d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).pClistview->RemoveRow(iRowNumber);

				// update items
				int m_iNewRowNumber = 0;
				for (auto &it : vData_new)
				{
					for (auto &m_it : it.vItems)
						m_it.iRowNumber = m_iNewRowNumber;

					m_iNewRowNumber++;
				}

				d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vData = vData_new;

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Listview control not found");
	return false;
} // removeListViewRow

bool cui_raw::getListview(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	std::vector<listviewColumn> &vColumns,
	std::vector<listviewRow> &vData,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.end())
		{
			try
			{
				// get data
				vColumns = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vColumns;
				vData = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vData;

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Listview control not found");
	return false;
} // getListview

bool cui_raw::getListviewSelected(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	std::vector<listviewRow> &vRows,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;
	vRows.clear();

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.end())
		{
			try
			{
				/*
				** identify the item selected in list view control
				*/
				d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).pClistview->ResetIndex();
				int index = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).pClistview->GetNextIndex();

				if (index != -1)
				{
					// check number of selected items
					int iSelected = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).pClistview->Selected();

					if (iSelected > 0)
					{
						/*
						** get selected items
						*/
						while (index != -1)
						{
							listviewRow row;

							for (auto &it : d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).vColumns)
							{
								// get item
								int iColNumber = it.iColumnID;
								std::basic_string<TCHAR> sData = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).pClistview->GetSelected(iColNumber);

								listviewItem item;
								item.iRowNumber = index;
								item.sColumnName = it.sColumnName;
								item.sItemData = sData;
								row.vItems.push_back(item);
							}

							vRows.push_back(row);

							index = d->m_Pages.at(sPageName + sPageLessKey).m_listviewControls.at(iUniqueID).pClistview->GetNextIndex();
						}
					}
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Listview control not found");
	return false;
} // getListviewSelected

void cui_raw::addEdit(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	const std::basic_string<TCHAR> &sCueBanner, RECT rc, onResize resize,
	bool bMultiLine, bool bScrollBar, bool bPassword, bool bReadOnly,
	int iLimit,
	const std::basic_string<TCHAR> &sAllowedCharacterSet,
	const std::basic_string<TCHAR> &sForbiddenCharacterSet,
	int iControlToInvoke
)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::EditControl control;
	control.iUniqueID = iUniqueID;
	control.sFontName = sFontName;
	control.iFontSize = iFontSize;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.resize = resize;
	control.bMultiLine = bMultiLine;
	control.bScrollBar = bScrollBar;
	control.d = d;
	control.bPassword = bPassword;
	control.sCueBanner = sCueBanner;
	control.bReadOnly = bReadOnly;
	control.iLimit = iLimit;
	control.sAllowedCharacterSet = sAllowedCharacterSet;
	control.sForbiddenCharacterSet = sForbiddenCharacterSet;
	control.iControlToInvoke = iControlToInvoke;

	if (iControlToInvoke == iUniqueID)
		control.iControlToInvoke = 0;

	control.bUpDown = false;

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.insert(std::pair<int, cui_rawImpl::EditControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addEdit

bool cui_raw::getEditText(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	std::basic_string<TCHAR> &sText,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.end())
		{
			try
			{
				HWND hWnd = d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.at(iUniqueID).hWnd;
				
				auto getWinText = [](HWND hWnd)
				{
					int i = GetWindowTextLength(hWnd) + 1;

					std::basic_string<TCHAR> sText;

					TCHAR *buffer = new TCHAR[i];
					GetWindowText(hWnd, buffer, i);
					sText = buffer;
					delete buffer;

					return sText;
				}; // getWinText

				sText = getWinText(hWnd);

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Edit control not found");
	return false;
} // getEditText

bool cui_raw::setEditText(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::basic_string<TCHAR> &sText,
	std::basic_string<TCHAR> &sErr
)
{

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.end())
		{
			try
			{
				std::basic_string<TCHAR> m_sText(sText);

				if (!d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.at(iUniqueID).sAllowedCharacterSet.empty())
				{
					// remove any characters that are not in the character set
					std::basic_string<TCHAR> sCharacters;
					for (auto &it : m_sText)
					{
						auto idx = d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.at(iUniqueID).sAllowedCharacterSet.find(it);

						if (idx == std::basic_string<TCHAR>::npos)
						{
							// this character shouldnt be in the edit control
							sCharacters += it;
						}
					}

					if (!sCharacters.empty())
					{
						// remove characters
						m_sText.erase(remove_if(m_sText.begin(), m_sText.end(), [&sCharacters](const char& c) {
							return sCharacters.find(c) != std::basic_string<TCHAR>::npos;
						}), m_sText.end());
					}
				}

				if (!d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.at(iUniqueID).sForbiddenCharacterSet.empty())
				{
					// remove any characters that are in the forbidden character set
					std::basic_string<TCHAR> sCharacters;
					for (auto &it : m_sText)
					{
						auto idx = d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.at(iUniqueID).sForbiddenCharacterSet.find(it);

						if (idx != std::basic_string<TCHAR>::npos)
						{
							// this character shouldnt be in the edit control
							sCharacters += it;
						}
					}

					if (!sCharacters.empty())
					{
						// remove characters
						m_sText.erase(remove_if(m_sText.begin(), m_sText.end(), [&sCharacters](const char& c) {
							return sCharacters.find(c) != std::basic_string<TCHAR>::npos;
						}), m_sText.end());
					}
				}

				SetWindowText(d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.at(iUniqueID).hWnd, m_sText.c_str());

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Edit control not found");
	return false;
} // setEditText

bool cui_raw::getEditCharsLeft(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	int &iCharsLeft,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.end())
		{
			try
			{
				if (d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.at(iUniqueID).iLimit > 0)
				{
					int iChars = GetWindowTextLength(d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.at(iUniqueID).hWnd);
					iCharsLeft = d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.at(iUniqueID).iLimit - iChars;
				}
				else
					iCharsLeft = -1;

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Edit control not found");
	return false;
} // getEditCharsLeft

void cui_raw::addToggleButton(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::basic_string<TCHAR> &sTooltip,
	const std::basic_string<TCHAR> &sCaptionOn,
	const std::basic_string<TCHAR> &sCaptionOff,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	COLORREF clrText,
	COLORREF clrOn,
	COLORREF clrOff,
	RECT rc, onResize resize, bool bOn)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::ToggleButtonControl control;
	control.iUniqueID = iUniqueID;

	control.toolTip.sText = sTooltip;
	control.toolTip.sFontName = d->m_sTooltipFont;
	control.toolTip.iFontSize = d->m_iTooltipFontSize;
	control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
	control.toolTip.m_clrText = d->m_clrTooltipText;
	control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
	control.toolTip.d = d;

	control.sCaptionOn = sCaptionOn;
	control.sCaptionOff = sCaptionOff;
	control.sFontName = sFontName;
	control.iFontSize = iFontSize;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.bOn = bOn;
	control.resize = resize;
	control.clrText = clrText;
	control.clrOn = clrOn;
	control.clrOff = clrOff;

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ToggleButtonControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_ToggleButtonControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_ToggleButtonControls.insert(std::pair<int, cui_rawImpl::ToggleButtonControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addToggleButton

bool cui_raw::setToggleButton(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	bool bOn,
	std::basic_string<TCHAR> &sErr
)
{

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ToggleButtonControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ToggleButtonControls.end())
		{
			try
			{
				d->m_Pages.at(sPageName + sPageLessKey).m_ToggleButtonControls.at(iUniqueID).bOn = bOn;
				InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_ToggleButtonControls.at(iUniqueID).hWnd, NULL, FALSE);
				UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ToggleButtonControls.at(iUniqueID).hWnd);

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Toggle button control not found");
	return false;
} // setToggleButton

bool cui_raw::getToggleButton(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	bool &bOn,
	std::basic_string<TCHAR> &sErr
)
{

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ToggleButtonControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ToggleButtonControls.end())
		{
			try
			{
				bOn = d->m_Pages.at(sPageName + sPageLessKey).m_ToggleButtonControls.at(iUniqueID).bOn;

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Toggle button control not found");
	return false;
} // getToggleButton

void cui_raw::addSelector(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::basic_string<TCHAR> &sTooltip,
	std::vector<selectorItem> vItems,
	int iDefaultItem,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	COLORREF clrText, COLORREF clrBar,
	RECT rc, onResize resize)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::SelectorControl control;
	control.iUniqueID = iUniqueID;

	control.toolTip.sText = sTooltip;
	control.toolTip.sFontName = d->m_sTooltipFont;
	control.toolTip.iFontSize = d->m_iTooltipFontSize;
	control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
	control.toolTip.m_clrText = d->m_clrTooltipText;
	control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
	control.toolTip.d = d;

	control.sFontName = sFontName;
	control.iFontSize = iFontSize;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.resize = resize;
	control.clrText = clrText;
	control.clrBar = clrBar;
	control.vItems = vItems;
	control.iSelectedItem = iDefaultItem;

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.insert(std::pair<int, cui_rawImpl::SelectorControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addSelector

bool cui_raw::setSelector(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	int iSelectorItemID,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.end())
		{
			try
			{
				d->m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.at(iUniqueID).iOldSelectedItem = d->m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.at(iUniqueID).iSelectedItem;
				d->m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.at(iUniqueID).iSelectedItem = iSelectorItemID;

				if (iSelectorItemID != d->m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.at(iUniqueID).iOldSelectedItem)
				{
					InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.at(iUniqueID).hWnd, NULL, FALSE);
					UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.at(iUniqueID).hWnd);
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Selector control not found");
	return false;
} // setSelector

bool cui_raw::getSelector(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	int &iSelectorItemID,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.end())
		{
			try
			{
				iSelectorItemID = d->m_Pages.at(sPageName + sPageLessKey).m_SelectorControls.at(iUniqueID).iSelectedItem;

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Selector control not found");
	return false;
} // getSelector

bool cui_raw::setProgressBar(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID, double iPercentage, bool bChangeColor, COLORREF clrBar,
	std::basic_string<TCHAR> &sErr)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.end())
		{
			try
			{
				if (bChangeColor &&
					clrBar != d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).clrBar)
				{
					d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).clrBar = clrBar;
					InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).hWnd, NULL, FALSE);
					UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).hWnd);
				}

				int m_iPerc = int(iPercentage + 0.5);

				if (iPercentage < 0)
					m_iPerc = int(iPercentage - 0.5);
				else
					m_iPerc = int(iPercentage + 0.5);

				bool bBusyOld = d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).bBusy;
				bool bBusyNew = m_iPerc == -1;

				d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).bBusy = bBusyNew;

				if (bBusyOld != bBusyNew)
					d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).iPercentage = iPercentage;

				if (iPercentage >= 0)
				{
					// move progress bar in multiple steps for aesthetic purposes
					double dStartPerc = d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).iPercentage;
					double dEndPerc = iPercentage;

					if (dEndPerc > 100)
						dEndPerc = 100;

					size_t iSteps = size_t(dEndPerc - dStartPerc);

					if (dEndPerc < dStartPerc)
						iSteps = 0 - iSteps;

					if (!IsWindowVisible(d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).hWnd))
						iSteps = 1;

					for (size_t i = 0; i < iSteps; i++)
					{
						if (iSteps > 1)
							Sleep(5);	// seperate each transition with the previous using 5ms intervals (total delay is approximately 500ms if progress jumps from 0-100 suddenly)

						double dStepPerc = (dEndPerc - dStartPerc) / double(iSteps);

						d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).iPercentage += dStepPerc;

						InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).hWnd, NULL, FALSE);
						UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).hWnd);
					}
				}
				else
				{
					if (m_iPerc == -1)
					{
						// change the progress style to a "busy" one
						InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).hWnd, NULL, FALSE);
						UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.at(iUniqueID).hWnd);
					}
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Progress control not found");
	return false;
} // setProgressBar

void cui_raw::addProgressBar(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	COLORREF clrBar,
	COLORREF clrUnfilled,
	RECT rc, onResize resize, double iInitialPercentage)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::ProgressControl control;
	control.iUniqueID = iUniqueID;
	control.clrBar = clrBar;
	control.clrUnfilled = clrUnfilled;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.iPercentage = iInitialPercentage;

	if (iInitialPercentage > 100)
		control.iPercentage = 100;
	else {
		control.bBusy = control.iPercentage == -1.0;
		control.bReverse = control.iPercentage == -2.0;

		if (!control.bBusy && !control.bReverse)
			control.iPercentage = max(control.iPercentage, 0.0);
	}

	control.resize = resize;

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_ProgressControls.insert(std::pair<int, cui_rawImpl::ProgressControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addProgressBar

bool cui_raw::setPasswordStrengthBar(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID, double iPercentage, std::basic_string<TCHAR> &sErr)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_PasswordStrengthControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_PasswordStrengthControls.end())
		{
			try
			{
				d->m_Pages.at(sPageName + sPageLessKey).m_PasswordStrengthControls.at(iUniqueID).iPercentage = iPercentage;

				SendMessage(d->m_Pages.at(sPageName + sPageLessKey).m_PasswordStrengthControls.at(iUniqueID).hWnd, WM_COMMAND, (WPARAM)IDOK, NULL);

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Password strength control not found");
	return false;
} // setPasswordStrengthBar

void cui_raw::addPasswordStrengthBar(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID, COLORREF clrUnfilled,
	RECT rc, onResize resize, double iInitialPercentage)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::PasswordStrengthControl control;
	control.iUniqueID = iUniqueID;
	control.clrUnfilled = clrUnfilled;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.iPercentage = iInitialPercentage;
	control.resize = resize;

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_PasswordStrengthControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_PasswordStrengthControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_PasswordStrengthControls.insert(std::pair<int, cui_rawImpl::PasswordStrengthControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addPasswordStrengthBar

void cui_raw::addUpDown(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	RECT rc, onResize resize, int iMin, int iMax, int iPos)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::EditControl control;
	control.iUniqueID = iUniqueID;

	control.sFontName = sFontName;
	control.iFontSize = iFontSize;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.resize = resize;
	control.iMin = iMin;
	control.iMax = iMax;
	control.iPos = iPos;

	control.bUpDown = true;

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.insert(std::pair<int, cui_rawImpl::EditControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addUpDown

bool cui_raw::setUpDown(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	int iPos,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.end())
		{
			try
			{
				if (d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.at(iUniqueID).bUpDown)
				{
					// Set the position of the up-down control
					SendMessage(d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.at(iUniqueID).hWndUpDown, UDM_SETPOS, 0, LPARAM(iPos));
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("UpDown control not found");
	return false;
} // setUpDown

bool cui_raw::getUpDown(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	int &iPos,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.end())
		{
			try
			{
				if (d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.at(iUniqueID).bUpDown)
				{
					// Set the position of the up-down control
					LRESULT lRes = SendMessage(d->m_Pages.at(sPageName + sPageLessKey).m_EditControls.at(iUniqueID).hWndUpDown, UDM_GETPOS, 0, 0);

					if (HIWORD(lRes) == 0)
					{
						iPos = LOWORD(lRes);
					}
					else
					{
						/*
						** an error occured.
						** TO-DO: find if there is a way to get details on this error
						*/
					}
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("UpDown control not found");
	return false;
} // getUpDown

void cui_raw::addDate(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	RECT rc, onResize resize, bool bAllowNone)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::DateControl control;
	control.iUniqueID = iUniqueID;
	control.d = d;
	control.sFontName = sFontName;
	control.iFontSize = iFontSize;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.resize = resize;
	control.bAllowNone = bAllowNone;

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_DateControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_DateControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_DateControls.insert(std::pair<int, cui_rawImpl::DateControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addDate

bool cui_raw::setDate(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	date dDate,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_DateControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_DateControls.end())
		{
			try
			{
				// set time in date control
				_SYSTEMTIME time;
				GetLocalTime(&time);

				time.wDay = (WORD)dDate.iDay;
				time.wMonth = (WORD)dDate.enMonth;
				time.wYear = (WORD)dDate.iYear;

				if (dDate.iYear == -1)
				{
					// set time in date control to NONE
					DateTime_SetSystemtime(d->m_Pages.at(sPageName + sPageLessKey).m_DateControls.at(iUniqueID).hWnd, GDT_NONE, &time);
				}
				else
				{
					// set time in date control
					DateTime_SetSystemtime(d->m_Pages.at(sPageName + sPageLessKey).m_DateControls.at(iUniqueID).hWnd, GDT_VALID, &time);
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Date control not found");
	return false;
} // setDate

bool cui_raw::getDate(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	date &dDate,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_DateControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_DateControls.end())
		{
			try
			{
				// set time in date control
				_SYSTEMTIME time;
				time = { 0 };

				/*
				** https://msdn.microsoft.com/en-us/library/windows/desktop/bb761801(v=vs.85).aspx
				**
				** Returns GDT_VALID if the time information was successfully placed in lpSysTime.
				** Returns GDT_NONE if the control was set to the DTS_SHOWNONE style and the
				** control check box was not selected. Returns GDT_ERROR if an error occurs.
				*/
				LRESULT lRes = DateTime_GetSystemtime(d->m_Pages.at(sPageName + sPageLessKey).m_DateControls.at(iUniqueID).hWnd, &time);

				if (lRes == GDT_VALID)
				{
					// write back date
					dDate.iDay = (int)time.wDay;
					dDate.enMonth = (cui_raw::month)time.wMonth;
					dDate.iYear = (int)time.wYear;
				}
				else
					if (lRes == GDT_ERROR)
					{
						sErr = _T("Setting date failed");
						return false;
					}
					else
						if (lRes == GDT_NONE)
						{
							/*
							** the control was set to the DTS_SHOWNONE style and the
							** control check box was not selected.Returns GDT_ERROR if an error occurs.
							** indicate that the date control is disabled by writing back a year of -1
							*/
							dDate.iDay = 0;
							dDate.enMonth = cui_raw::month::january;
							dDate.iYear = -1;
						}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Date control not found");
	return false;
} // getDate

void cui_raw::addTime(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	RECT rc, onResize resize, bool bAllowNone)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::TimeControl control;
	control.iUniqueID = iUniqueID;
	control.d = d;
	control.sFontName = sFontName;
	control.iFontSize = iFontSize;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.resize = resize;
	control.bAllowNone = bAllowNone;

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_TimeControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_TimeControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_TimeControls.insert(std::pair<int, cui_rawImpl::TimeControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addTime

bool cui_raw::setTime(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	time tTime,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_TimeControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_TimeControls.end())
		{
			try
			{
				// set time in time control
				_SYSTEMTIME time;
				GetLocalTime(&time);

				time.wHour = (WORD)tTime.iHour;
				time.wMinute = (WORD)tTime.iMinute;
				time.wSecond = (WORD)tTime.iSecond;

				if (tTime.iHour == -1)
				{
					// set time in time control to NONE
					DateTime_SetSystemtime(d->m_Pages.at(sPageName + sPageLessKey).m_TimeControls.at(iUniqueID).hWnd, GDT_NONE, &time);
				}
				else
				{
					// set time in time control
					DateTime_SetSystemtime(d->m_Pages.at(sPageName + sPageLessKey).m_TimeControls.at(iUniqueID).hWnd, GDT_VALID, &time);
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Time control not found");
	return false;
} // setTime

bool cui_raw::getTime(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	time &tTime,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_TimeControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_TimeControls.end())
		{
			try
			{
				// set time in time control
				_SYSTEMTIME time;
				time = { 0 };

				/*
				** https://msdn.microsoft.com/en-us/library/windows/desktop/bb761801(v=vs.85).aspx
				**
				** Returns GDT_VALID if the time information was successfully placed in lpSysTime.
				** Returns GDT_NONE if the control was set to the DTS_SHOWNONE style and the
				** control check box was not selected. Returns GDT_ERROR if an error occurs.
				*/
				LRESULT lRes = DateTime_GetSystemtime(d->m_Pages.at(sPageName + sPageLessKey).m_TimeControls.at(iUniqueID).hWnd, &time);

				if (lRes == GDT_VALID)
				{
					// write back time
					tTime.iHour = (int)time.wHour;
					tTime.iMinute = (int)time.wMinute;
					tTime.iSecond = (int)time.wSecond;
				}
				else
					if (lRes == GDT_ERROR)
					{
						sErr = _T("Setting time failed");
						return false;
					}
					else
						if (lRes == GDT_NONE)
						{
							/*
							** the control was set to the DTS_SHOWNONE style and the
							** control check box was not selected.Returns GDT_ERROR if an error occurs.
							** indicate that the time control is disabled by writing back an hour of -1
							*/
							tTime.iHour = -1;
							tTime.iMinute = 0;
							tTime.iSecond = 0;
						}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Time control not found");
	return false;
} // getTime

void cui_raw::addBarChart(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	RECT rc, onResize resize,
	COLORREF clrBar,
	std::basic_string<TCHAR> sChartName,
	std::basic_string<TCHAR> sXaxisLabel, std::basic_string<TCHAR> sYaxisLabel,
	int iLowerLimit, int iUpperLimit, bool bAutoScale,
	std::vector<barChartData> vValues,
	bool autocolor
)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::BarChartControl control;

	control.toolTip.sText = _T("");	// tooltip is going to be set on a per-bar basis
	control.toolTip.sFontName = d->m_sTooltipFont;
	control.toolTip.iFontSize = d->m_iTooltipFontSize;
	control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
	control.toolTip.m_clrText = d->m_clrTooltipText;
	control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
	control.toolTip.d = d;

	control.iUniqueID = iUniqueID;
	control.d = d;
	control.sFontName = sFontName;
	control.iFontSize = iFontSize;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.resize = resize;

	control.bInfoCaptured = false;
	control.sChartName = sChartName;
	control.sXaxisLabel = sXaxisLabel;
	control.sYaxisLabel = sYaxisLabel;
	control.iLowerLimit = iLowerLimit;
	control.iUpperLimit = iUpperLimit;
	control.bAutoScale = bAutoScale;
	control.vValues = vValues;

	if (autocolor)
	{
		std::vector<int> vValues;

		for (int i = 1; i < 256; i++)
			vValues.push_back(i);

		for (auto &it : control.vValues)
			it.clrBar = randomColor(true);
	}

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.insert(std::pair<int, cui_rawImpl::BarChartControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addBarChart

bool cui_raw::barChartScaleSet(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	bool bAutoScale,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.end())
		{
			try
			{
				if (d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).bAutoScale != bAutoScale)
				{
					d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).bInfoCaptured = false;	// important to ensure control parameters are reset	
					d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).bAutoScale = bAutoScale;

					InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).hWnd, NULL, FALSE);
					UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).hWnd);
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Bar chart control not found");
	return false;
} // barChartScaleSet

bool cui_raw::barChartScaleGet(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	bool &bAutoScale,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.end())
		{
			try
			{
				bAutoScale = d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).bAutoScale;

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Bar chart control not found");
	return false;
} // barChartScaleGet

bool cui_raw::barChartNameGet(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	std::basic_string<TCHAR> &sBarChartName,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.end())
		{
			try
			{
				sBarChartName = d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).sChartName;

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Bar chart control not found");
	return false;
} // barChartNameGet

bool cui_raw::barChartReload(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	std::basic_string<TCHAR> sChartName,
	std::basic_string<TCHAR> sXaxisLabel, std::basic_string<TCHAR> sYaxisLabel,
	int iLowerLimit, int iUpperLimit, bool bAutoScale,
	std::vector<barChartData> vValues,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.end())
		{
			try
			{
				d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).bInfoCaptured = false;	// important to ensure control parameters are reset	
				d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).bAutoScale = bAutoScale;
				d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).sChartName = sChartName;
				d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).sXaxisLabel = sXaxisLabel;
				d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).sYaxisLabel = sYaxisLabel;
				d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).iLowerLimit = iLowerLimit;
				d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).iUpperLimit = iUpperLimit;
				d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).vValues = vValues;

				if (/*bAutoColor*/true)
				{
					std::vector<int> vValues;

					for (int i = 1; i < 256; i++)
						vValues.push_back(i);

					for (auto &it : d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).vValues)
						it.clrBar = randomColor(true);
				}

				InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).hWnd, NULL, FALSE);
				UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).hWnd);

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Bar chart control not found");
	return false;
}

bool cui_raw::barChartSave(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	imgFormat format,
	std::basic_string<TCHAR> &sFullPath,
	std::basic_string<TCHAR> &sErr)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.end())
		{
			try
			{
				CImageConv::imageformat m_format;

				switch (format)
				{
				case cui_raw::PNG:
					m_format = CImageConv::imageformat::PNG;
					break;
				case cui_raw::BMP:
					m_format = CImageConv::imageformat::BMP;
					break;
				case cui_raw::JPEG:
					m_format = CImageConv::imageformat::JPEG;
					break;
				case cui_raw::NONE:
					m_format = CImageConv::imageformat::NONE;
					break;
				default:
					m_format = CImageConv::imageformat::PNG;
					break;
				}

				bool bRes = true;

				if (d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).hbm_buffer)
				{
					// attempt to save image to file
					bRes = CImageConv::HBITMAPtoFILE(d->m_Pages.at(sPageName + sPageLessKey).m_BarChartControls.at(iUniqueID).hbm_buffer, sFullPath, m_format, sErr);
				}
				else
				{
					sErr = _T("No bar chart in control");
					bRes = false;
				}

				return bRes;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Bar chart control not found");
	return false;
} // barChartSave

void cui_raw::addLineChart(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	RECT rc, onResize resize,
	std::basic_string<TCHAR> sChartName,
	std::basic_string<TCHAR> sXaxisLabel, std::basic_string<TCHAR> sYaxisLabel,
	int iLowerLimit, int iUpperLimit, bool bAutoScale,
	std::vector<lineInfo> vLines,
	bool autocolor
)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::LineChartControl control;

	control.toolTip.sText = _T("");	// tooltip is going to be set on a per-bar basis
	control.toolTip.sFontName = d->m_sTooltipFont;
	control.toolTip.iFontSize = d->m_iTooltipFontSize;
	control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
	control.toolTip.m_clrText = d->m_clrTooltipText;
	control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
	control.toolTip.d = d;

	control.iUniqueID = iUniqueID;
	control.d = d;
	control.sFontName = sFontName;
	control.iFontSize = iFontSize;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.resize = resize;

	control.bInfoCaptured = false;
	control.sChartName = sChartName;
	control.sXaxisLabel = sXaxisLabel;
	control.sYaxisLabel = sYaxisLabel;
	control.iLowerLimit = iLowerLimit;
	control.iUpperLimit = iUpperLimit;
	control.bAutoScale = bAutoScale;
	control.vLines = vLines;

	if (autocolor)
	{
		std::vector<int> vValues;

		for (int i = 1; i < 256; i++)
			vValues.push_back(i);

		for (auto &it : control.vLines)
			it.clrLine = randomColor(true);
	}

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.insert(std::pair<int, cui_rawImpl::LineChartControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addLineChart

bool cui_raw::lineChartScaleSet(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	bool bAutoScale,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.end())
		{
			try
			{
				if (d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).bAutoScale != bAutoScale)
				{
					d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).bInfoCaptured = false;	// important to ensure control parameters are reset	
					d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).bAutoScale = bAutoScale;

					InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).hWnd, NULL, FALSE);
					UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).hWnd);
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Line chart control not found");
	return false;
} // lineChartScaleSet

bool cui_raw::lineChartScaleGet(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	bool &bAutoScale,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.end())
		{
			try
			{
				bAutoScale = d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).bAutoScale;

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Line chart control not found");
	return false;
} // lineChartScaleGet

bool cui_raw::lineChartNameGet(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	std::basic_string<TCHAR> &sLineChartName,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.end())
		{
			try
			{
				sLineChartName = d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).sChartName;

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Line chart control not found");
	return false;
} // lineChartNameGet

bool cui_raw::lineChartReload(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	std::basic_string<TCHAR> sChartName,
	std::basic_string<TCHAR> sXaxisLabel, std::basic_string<TCHAR> sYaxisLabel,
	int iLowerLimit, int iUpperLimit, bool bAutoScale,
	std::vector<lineInfo> vLines,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.end())
		{
			try
			{
				d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).bInfoCaptured = false;	// important to ensure control parameters are reset	
				d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).bAutoScale = bAutoScale;
				d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).sChartName = sChartName;
				d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).sXaxisLabel = sXaxisLabel;
				d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).sYaxisLabel = sYaxisLabel;
				d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).iLowerLimit = iLowerLimit;
				d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).iUpperLimit = iUpperLimit;
				d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).vLines = vLines;

				if (/*bAutoColor*/true)
				{
					std::vector<int> vValues;

					for (int i = 1; i < 256; i++)
						vValues.push_back(i);

					for (auto &it : d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).vLines)
						it.clrLine = randomColor(true);
				}

				InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).hWnd, NULL, FALSE);
				UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).hWnd);

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Line chart control not found");
	return false;
}

bool cui_raw::lineChartSave(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	imgFormat format,
	std::basic_string<TCHAR> &sFullPath,
	std::basic_string<TCHAR> &sErr)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.end())
		{
			try
			{
				CImageConv::imageformat m_format;

				switch (format)
				{
				case cui_raw::PNG:
					m_format = CImageConv::imageformat::PNG;
					break;
				case cui_raw::BMP:
					m_format = CImageConv::imageformat::BMP;
					break;
				case cui_raw::JPEG:
					m_format = CImageConv::imageformat::JPEG;
					break;
				case cui_raw::NONE:
					m_format = CImageConv::imageformat::NONE;
					break;
				default:
					m_format = CImageConv::imageformat::PNG;
					break;
				}

				bool bRes = true;

				if (d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).hbm_buffer)
				{
					// attempt to save image to file
					bRes = CImageConv::HBITMAPtoFILE(d->m_Pages.at(sPageName + sPageLessKey).m_LineChartControls.at(iUniqueID).hbm_buffer, sFullPath, m_format, sErr);
				}
				else
				{
					sErr = _T("No line chart in control");
					bRes = false;
				}

				return bRes;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Line chart control not found");
	return false;
} // lineChartSave

void cui_raw::addPieChart(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	RECT rc, onResize resize,
	bool bAutoColor,
	std::basic_string<TCHAR> sChartName,
	std::vector<pieChartData> vData,
	pieChartHoverEffect hoverEffect,
	bool bDoughnut
)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::PieChartControl control;

	control.toolTip.sText = _T("");	// tooltip is going to be set on a per-pie basis, or will it?
	control.toolTip.sFontName = d->m_sTooltipFont;
	control.toolTip.iFontSize = d->m_iTooltipFontSize;
	control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
	control.toolTip.m_clrText = d->m_clrTooltipText;
	control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
	control.toolTip.d = d;

	control.iUniqueID = iUniqueID;
	control.d = d;
	control.sFontName = sFontName;
	control.iFontSize = iFontSize;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.resize = resize;

	control.bInfoCaptured = false;
	control.bAutoColor = bAutoColor;
	control.sChartName = sChartName;
	control.vData = vData;
	control.hoverEffect = hoverEffect;
	control.bDoughnut = bDoughnut;

	if (bAutoColor)
	{
		for (auto &it : control.vData)
		{
			// make a random color
			it.clrItem = randomColor(true);
		}
	}

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.insert(std::pair<int, cui_rawImpl::PieChartControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addPieChart

bool cui_raw::pieChartSave(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	imgFormat format,
	std::basic_string<TCHAR> &sFullPath,
	std::basic_string<TCHAR> &sErr)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.end())
		{
			try
			{
				CImageConv::imageformat m_format;

				switch (format)
				{
				case cui_raw::PNG:
					m_format = CImageConv::imageformat::PNG;
					break;
				case cui_raw::BMP:
					m_format = CImageConv::imageformat::BMP;
					break;
				case cui_raw::JPEG:
					m_format = CImageConv::imageformat::JPEG;
					break;
				case cui_raw::NONE:
					m_format = CImageConv::imageformat::NONE;
					break;
				default:
					m_format = CImageConv::imageformat::PNG;
					break;
				}

				bool bRes = true;

				if (d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.at(iUniqueID).hbm_buffer)
				{
					// attempt to save image to file
					bRes = CImageConv::HBITMAPtoFILE(d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.at(iUniqueID).hbm_buffer, sFullPath, m_format, sErr);
				}
				else
				{
					sErr = _T("No pie chart in control");
					bRes = false;
				}

				return bRes;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Pie chart control not found");
	return false;
} // pieChartSave

bool cui_raw::pieChartReload(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	bool bAutoColor,
	std::basic_string<TCHAR> sChartName,
	std::vector<pieChartData> vData,
	pieChartHoverEffect hoverEffect,
	bool bDoughnut,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.end())
		{
			try
			{
				d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.at(iUniqueID).bInfoCaptured = false;	// important to ensure control parameters are reset	
				d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.at(iUniqueID).bAutoColor = bAutoColor;
				d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.at(iUniqueID).sChartName = sChartName;
				d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.at(iUniqueID).vData = vData;
				d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.at(iUniqueID).hoverEffect = hoverEffect;
				d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.at(iUniqueID).bDoughnut = bDoughnut;

				if (bAutoColor)
				{
					std::vector<int> vValues;

					for (int i = 1; i < 256; i++)
						vValues.push_back(i);

					auto randomColor = [&]()
					{
						// make a random color
						int r = 0;
						randFromVec(vValues, r);

						int g = 0;
						randFromVec(vValues, g);

						int b = 0;
						randFromVec(vValues, b);

						return RGB(r, g, b);
					};

					for (auto &it : d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.at(iUniqueID).vData)
					{
						// make a random color
						it.clrItem = randomColor();
					}
				}

				InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.at(iUniqueID).hWnd, NULL, FALSE);
				UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_PieChartControls.at(iUniqueID).hWnd);

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Pie chart control not found");
	return false;
} // pieChartReload

void cui_raw::addRichEdit(
	const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	const std::basic_string<TCHAR> &sUIFontName, double iUIFontSize,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	RECT rc, onResize resize,
	bool bBorder,
	COLORREF clrBorder,
	bool bReadOnly,
	int IDP_BOLD_,
	int IDP_ITALIC_,
	int IDP_UNDERLINE_,
	int IDP_STRIKETHROUGH_,
	int IDP_SUBSCRIPT_,
	int IDP_SUPERSCRIPT_,
	int IDP_LARGER_,
	int IDP_SMALLER_,
	int IDP_FONTCOLOR_,
	int IDP_LEFT_ALIGN_,
	int IDP_CENTER_ALIGN_,
	int IDP_RIGHT_ALIGN_,
	int IDP_JUSTIFY_,
	int IDP_LIST_,
	HMODULE resource_module
)
{
	int iBtnSize = 20;
	int iBtnMargin = 2;

	RECT m_rc = rc;
	InflateRect(&m_rc, -5, -5);	// for borders

	if (!bReadOnly)
		m_rc.top += 2 + 20 + 5 + iBtnSize + 2 + 15 + 5;			// for formatting controls

	scaleRECT(m_rc, d->m_DPIScale);

	cui_rawImpl::RichEditControl control;

	control.iUniqueID = iUniqueID;
	control.d = d;
	control.sFontName = sFontName;
	control.iFontSize = iFontSize;
	control.coords.left = m_rc.left;
	control.coords.top = m_rc.top;
	control.coords.right = m_rc.right;
	control.coords.bottom = m_rc.bottom;
	control.resize = resize;
	control.bReadOnly = bReadOnly;
	control.bBorder = bBorder;

	if (control.bBorder)
	{
		// register messages for lines
		control.iIDLineLeft = d->GetNewMessageID();
		control.iIDLineRight = d->GetNewMessageID();
		control.iIDLineTop = d->GetNewMessageID();
		control.iIDLineBottom = d->GetNewMessageID();

		// add left line
		RECT rcLeftLine;
		{
			cui_raw::onResize m_resize = resize;
			m_resize.iPercH = resize.iPercH;
			m_resize.iPercV = resize.iPercV;
			m_resize.iPercCX = 0;
			m_resize.iPercCY = resize.iPercCY;

			rcLeftLine = rc;
			rcLeftLine.right = rcLeftLine.left + 1;

			addHairLine(sPageName, control.iIDLineLeft, clrBorder, rcLeftLine, m_resize);
		}

		// add top line
		RECT rcTopLine;
		{
			cui_raw::onResize m_resize = resize;
			m_resize.iPercH = resize.iPercH;
			m_resize.iPercV = resize.iPercV;
			m_resize.iPercCX = resize.iPercCX;
			m_resize.iPercCY = 0;

			rcTopLine = rc;
			rcTopLine.left = rcLeftLine.right;
			rcTopLine.bottom = rcTopLine.top + 1;

			addHairLine(sPageName, control.iIDLineTop, clrBorder, rcTopLine, m_resize);
		}

		// add right line
		RECT rcRightLine;
		{
			cui_raw::onResize m_resize = resize;
			m_resize.iPercH = resize.iPercH + resize.iPercCX;	// is this right? test
			m_resize.iPercV = resize.iPercV;
			m_resize.iPercCX = 0;
			m_resize.iPercCY = resize.iPercCY;

			rcRightLine = rc;
			rcRightLine.left = rcTopLine.right;
			rcRightLine.right = rcRightLine.left + 1;

			addHairLine(sPageName, control.iIDLineRight, clrBorder, rcRightLine, m_resize);
		}

		// add bottom line
		RECT rcBottomLine;
		{
			cui_raw::onResize m_resize = resize;
			m_resize.iPercH = resize.iPercH;
			m_resize.iPercV = resize.iPercV + resize.iPercCY;	// is this right? test
			m_resize.iPercCX = resize.iPercCX;
			m_resize.iPercCY = 0;

			rcBottomLine = rc;
			rcBottomLine.top = rcLeftLine.bottom;
			rcBottomLine.bottom = rcBottomLine.top + 1;

			addHairLine(sPageName, control.iIDLineBottom, clrBorder, rcBottomLine, m_resize);
		}
	}

	if (!bReadOnly)
	{
		int iFontRight = 0;

		cui_raw::onResize m_resize = resize;
		m_resize.iPercH = resize.iPercH;
		m_resize.iPercV = resize.iPercV;
		m_resize.iPercCX = 0;
		m_resize.iPercCY = 0;

		// add formatting controls
		RECT rcFormatting = rc;
		rcFormatting.left += 2;
		rcFormatting.top += 2;
		rcFormatting.bottom = rcFormatting.top + 20;
		rcFormatting.right = rcFormatting.left + 115;

		// add font list combobox
		control.iIDFontList = d->GetNewMessageID();

		// add this combobox to the font combo list (this is absolutely important for owner drawing the font list)
		d->vFontCombos.push_back(control.iIDFontList);

		std::vector<std::basic_string<TCHAR>> vItems;
		addComboBox(sPageName,
			control.iIDFontList, vItems, _T(""), sFontName, iFontSize, rcFormatting, m_resize, false, true);

		// add font size combobox
		control.iIDFontSize = d->GetNewMessageID();

		vItems.clear();
		vItems.push_back(_T("8"));
		vItems.push_back(_T("9"));
		vItems.push_back(_T("10"));
		vItems.push_back(_T("11"));
		vItems.push_back(_T("12"));
		vItems.push_back(_T("14"));
		vItems.push_back(_T("16"));
		vItems.push_back(_T("18"));
		vItems.push_back(_T("20"));
		vItems.push_back(_T("22"));
		vItems.push_back(_T("24"));
		vItems.push_back(_T("26"));
		vItems.push_back(_T("28"));
		vItems.push_back(_T("36"));
		vItems.push_back(_T("48"));
		vItems.push_back(_T("72"));

		rcFormatting.left = rcFormatting.right + 2;
		rcFormatting.right = rcFormatting.left + 50;

		addComboBox(sPageName,
			control.iIDFontSize, vItems, std::to_wstring(int(iFontSize + 0.5)), sUIFontName, iUIFontSize, rcFormatting, m_resize, true, false);

		rcFormatting.left = rcFormatting.right + iBtnMargin;
		rcFormatting.right = rcFormatting.left + iBtnSize;

		// add larger button
		control.iLarger = d->GetNewMessageID();

		addImage(sPageName,
			control.iLarger, _T("Make your text a bit bigger"), true, false, RGB(0, 0, 0), RGB(0, 0, 0), RGB(255, 255, 255),
			RGB(255, 255, 255), false, RGB(255, 255, 255), RGB(255, 255, 255), RGB(200, 200, 255), IDP_LARGER_,
			_T(""), RGB(0, 0, 0), RGB(0, 0, 0), _T("Segoe UI"), _T("Segoe UI"), 9, cui_raw::imageTextPlacement::bottom,
			rcFormatting, m_resize, false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 }, resource_module);

		rcFormatting.left = rcFormatting.right + iBtnMargin;
		rcFormatting.right = rcFormatting.left + iBtnSize;

		iFontRight = rcFormatting.right + 2;

		// add smaller button
		control.iSmaller = d->GetNewMessageID();

		addImage(sPageName,
			control.iSmaller, _T("Make your text a bit smaller"), true, false, RGB(0, 0, 0), RGB(0, 0, 0), RGB(255, 255, 255),
			RGB(255, 255, 255), false, RGB(255, 255, 255), RGB(255, 255, 255), RGB(200, 200, 255), IDP_SMALLER_,
			_T(""), RGB(0, 0, 0), RGB(0, 0, 0), _T("Segoe UI"), _T("Segoe UI"), 9, cui_raw::imageTextPlacement::bottom,
			rcFormatting, m_resize, false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 }, resource_module);

		rcFormatting.top = rcFormatting.bottom + 5;
		rcFormatting.bottom = rcFormatting.top + iBtnSize;
		rcFormatting.left = rc.left + 2;
		rcFormatting.right = rcFormatting.left + iBtnSize;

		// add bold button
		control.iBold = d->GetNewMessageID();

		addImage(sPageName,
			control.iBold, _T("Make your text bold"), true, true, RGB(0, 0, 0), RGB(0, 0, 0), RGB(255, 255, 255),
			RGB(255, 255, 255), false, RGB(255, 255, 255), RGB(255, 255, 255), RGB(200, 200, 255), IDP_BOLD_,
			_T(""), RGB(0, 0, 0), RGB(0, 0, 0), _T("Segoe UI"), _T("Segoe UI"), 9, cui_raw::imageTextPlacement::bottom,
			rcFormatting, m_resize, false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 }, resource_module);

		rcFormatting.left = rcFormatting.right + iBtnMargin;
		rcFormatting.right = rcFormatting.left + iBtnSize;

		// add italic button
		control.iItalic = d->GetNewMessageID();

		addImage(sPageName,
			control.iItalic, _T("Italicize your text"), true, true, RGB(0, 0, 0), RGB(0, 0, 0), RGB(255, 255, 255),
			RGB(255, 255, 255), false, RGB(255, 255, 255), RGB(255, 255, 255), RGB(200, 200, 255), IDP_ITALIC_,
			_T(""), RGB(0, 0, 0), RGB(0, 0, 0), _T("Segoe UI"), _T("Segoe UI"), 9, cui_raw::imageTextPlacement::bottom,
			rcFormatting, m_resize, false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 }, resource_module);

		rcFormatting.left = rcFormatting.right + iBtnMargin;
		rcFormatting.right = rcFormatting.left + iBtnSize;

		// add underline button
		control.iUnderline = d->GetNewMessageID();

		addImage(sPageName,
			control.iUnderline, _T("Underline your text"), true, true, RGB(0, 0, 0), RGB(0, 0, 0), RGB(255, 255, 255),
			RGB(255, 255, 255), false, RGB(255, 255, 255), RGB(255, 255, 255), RGB(200, 200, 255), IDP_UNDERLINE_,
			_T(""), RGB(0, 0, 0), RGB(0, 0, 0), _T("Segoe UI"), _T("Segoe UI"), 9, cui_raw::imageTextPlacement::bottom,
			rcFormatting, m_resize, false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 }, resource_module);

		rcFormatting.left = rcFormatting.right + iBtnMargin;
		rcFormatting.right = rcFormatting.left + iBtnSize;

		// add strikethough button
		control.iStrikethough = d->GetNewMessageID();

		addImage(sPageName,
			control.iStrikethough, _T("Draw a line through your text"), true, false, RGB(0, 0, 0), RGB(0, 0, 0), RGB(255, 255, 255),
			RGB(255, 255, 255), false, RGB(255, 255, 255), RGB(255, 255, 255), RGB(200, 200, 255), IDP_STRIKETHROUGH_,
			_T(""), RGB(0, 0, 0), RGB(0, 0, 0), _T("Segoe UI"), _T("Segoe UI"), 9, cui_raw::imageTextPlacement::bottom,
			rcFormatting, m_resize, false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 }, resource_module);

		rcFormatting.left = rcFormatting.right + iBtnMargin;
		rcFormatting.right = rcFormatting.left + iBtnSize;

		// add subscript button
		control.iSubscript = d->GetNewMessageID();

		addImage(sPageName,
			control.iSubscript, _T("Make your text smaller and lower than regular text"), true, false, RGB(0, 0, 0), RGB(0, 0, 0), RGB(255, 255, 255),
			RGB(255, 255, 255), false, RGB(255, 255, 255), RGB(255, 255, 255), RGB(200, 200, 255), IDP_SUBSCRIPT_,
			_T(""), RGB(0, 0, 0), RGB(0, 0, 0), _T("Segoe UI"), _T("Segoe UI"), 9, cui_raw::imageTextPlacement::bottom,
			rcFormatting, m_resize, false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 }, resource_module);

		rcFormatting.left = rcFormatting.right + iBtnMargin;
		rcFormatting.right = rcFormatting.left + iBtnSize;

		// add superscript button
		control.iSuperscript = d->GetNewMessageID();

		addImage(sPageName,
			control.iSuperscript, _T("Make your text smaller and higher than regular text"), true, false, RGB(0, 0, 0), RGB(0, 0, 0), RGB(255, 255, 255),
			RGB(255, 255, 255), false, RGB(255, 255, 255), RGB(255, 255, 255), RGB(200, 200, 255), IDP_SUPERSCRIPT_,
			_T(""), RGB(0, 0, 0), RGB(0, 0, 0), _T("Segoe UI"), _T("Segoe UI"), 9, cui_raw::imageTextPlacement::bottom,
			rcFormatting, m_resize, false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 }, resource_module);

		rcFormatting.left = rcFormatting.right + iBtnMargin;
		rcFormatting.right = rcFormatting.left + iBtnSize;

		// add font color button
		control.iFontColor = d->GetNewMessageID();

		addImage(sPageName,
			control.iFontColor, _T("Change the color of your text"), false, false, RGB(0, 0, 0), RGB(0, 0, 0), RGB(255, 255, 255),
			RGB(255, 255, 255), true, RGB(255, 0, 0), RGB(255, 255, 255), RGB(200, 200, 255), IDP_FONTCOLOR_,
			_T(""), RGB(0, 0, 0), RGB(0, 0, 0), _T("Segoe UI"), _T("Segoe UI"), 9, cui_raw::imageTextPlacement::top,
			rcFormatting, m_resize, false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 }, resource_module);

		iFontRight = max(iFontRight, rcFormatting.right + 2);

		// add font label
		RECT rcFontLabel = rc;
		rcFontLabel.left += 2;
		rcFontLabel.right = iFontRight;
		rcFontLabel.right -= 2;
		rcFontLabel.top = rcFormatting.bottom;
		rcFontLabel.bottom = rcFontLabel.top + 15;

		control.iIDFontLabel = d->GetNewMessageID();

		addText(sPageName,
			control.iIDFontLabel, _T("Font"), true, RGB(100, 100, 100), clrBorder, _T(""), sUIFontName, iUIFontSize, rcFontLabel, cui_raw::textAlignment::center, m_resize, false);

		// add vertical separator

		RECT rcSepV = rc;
		rcSepV.left = rcFontLabel.right + 2;
		rcSepV.right = rcSepV.left + 1;
		rcSepV.top += 2;
		rcSepV.bottom = rcFontLabel.bottom - 2;

		control.iIDVSeparator = d->GetNewMessageID();

		addHairLine(sPageName, control.iIDVSeparator, clrBorder, rcSepV, m_resize);

		int iParagraphRight = 0;

		// add align left
		rcFormatting = rcSepV;
		rcFormatting.left = rcSepV.right + 2;
		rcFormatting.right = rcFormatting.left + iBtnSize;
		rcFormatting.top = rc.top + 2;
		rcFormatting.bottom = rcFormatting.top + iBtnSize;

		control.iLeftAlign = d->GetNewMessageID();

		addImage(sPageName,
			control.iLeftAlign, _T("Align your text to the left margin"), true, false, RGB(0, 0, 0), RGB(0, 0, 0), RGB(255, 255, 255),
			RGB(255, 255, 255), false, RGB(255, 255, 255), RGB(255, 255, 255), RGB(200, 200, 255), IDP_LEFT_ALIGN_,
			_T(""), RGB(0, 0, 0), RGB(0, 0, 0), _T("Segoe UI"), _T("Segoe UI"), 9, cui_raw::imageTextPlacement::bottom,
			rcFormatting, m_resize, false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 }, resource_module);

		// add align center
		rcFormatting.left = rcFormatting.right + iBtnMargin;
		rcFormatting.right = rcFormatting.left + iBtnSize;

		control.iCenterAlign = d->GetNewMessageID();

		addImage(sPageName,
			control.iCenterAlign, _T("Align your text to the center"), true, false, RGB(0, 0, 0), RGB(0, 0, 0), RGB(255, 255, 255),
			RGB(255, 255, 255), false, RGB(255, 255, 255), RGB(255, 255, 255), RGB(200, 200, 255), IDP_CENTER_ALIGN_,
			_T(""), RGB(0, 0, 0), RGB(0, 0, 0), _T("Segoe UI"), _T("Segoe UI"), 9, cui_raw::imageTextPlacement::bottom,
			rcFormatting, m_resize, false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 }, resource_module);

		// add align right
		rcFormatting.left = rcFormatting.right + iBtnMargin;
		rcFormatting.right = rcFormatting.left + iBtnSize;

		control.iRightAlign = d->GetNewMessageID();

		addImage(sPageName,
			control.iRightAlign, _T("Align your text to the right margin"), true, false, RGB(0, 0, 0), RGB(0, 0, 0), RGB(255, 255, 255),
			RGB(255, 255, 255), false, RGB(255, 255, 255), RGB(255, 255, 255), RGB(200, 200, 255), IDP_RIGHT_ALIGN_,
			_T(""), RGB(0, 0, 0), RGB(0, 0, 0), _T("Segoe UI"), _T("Segoe UI"), 9, cui_raw::imageTextPlacement::bottom,
			rcFormatting, m_resize, false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 }, resource_module);

		// add justify align
		rcFormatting.left = rcFormatting.right + iBtnMargin;
		rcFormatting.right = rcFormatting.left + iBtnSize;

		control.iJustify = d->GetNewMessageID();

		addImage(sPageName,
			control.iJustify, _T("Justify your text"), true, false, RGB(0, 0, 0), RGB(0, 0, 0), RGB(255, 255, 255),
			RGB(255, 255, 255), false, RGB(255, 255, 255), RGB(255, 255, 255), RGB(200, 200, 255), IDP_JUSTIFY_,
			_T(""), RGB(0, 0, 0), RGB(0, 0, 0), _T("Segoe UI"), _T("Segoe UI"), 9, cui_raw::imageTextPlacement::bottom,
			rcFormatting, m_resize, false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 }, resource_module);

		iParagraphRight = rcFormatting.right + 2;

		// add list icon
		rcFormatting.top = rcFormatting.bottom + 5;
		rcFormatting.bottom = rcFormatting.top + iBtnSize;
		rcFormatting.left = iFontRight + 2;
		rcFormatting.right = rcFormatting.left + iBtnSize;

		control.iList = d->GetNewMessageID();

		addImage(sPageName,
			control.iList, _T("Create a list"), true, false, RGB(0, 0, 0), RGB(0, 0, 0), RGB(255, 255, 255),
			RGB(255, 255, 255), false, RGB(255, 255, 255), RGB(255, 255, 255), RGB(200, 200, 255), IDP_LIST_,
			_T(""), RGB(0, 0, 0), RGB(0, 0, 0), _T("Segoe UI"), _T("Segoe UI"), 9, cui_raw::imageTextPlacement::bottom,
			rcFormatting, m_resize, false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 }, resource_module);

		// add list type combobox
		rcFormatting.left = rcFormatting.right + iBtnMargin;
		rcFormatting.right = iParagraphRight - 2;

		control.iListType = d->GetNewMessageID();

		vItems.clear();
		vItems.push_back(_T("• • •"));
		vItems.push_back(_T("1. 2. 3."));
		vItems.push_back(_T("a. b. c."));
		vItems.push_back(_T("A. B. C. "));	// the space at the end absolutely necessary
		vItems.push_back(_T("i. ii. iii."));
		vItems.push_back(_T("I. II. III. "));	// the space at the end absolutely necessary

		addComboBox(sPageName,
			control.iListType, vItems, _T("1. 2. 3."), sUIFontName, iUIFontSize, rcFormatting, m_resize, false, true);

		







		// add paragraph label
		RECT rcParagraphLabel = rc;
		rcParagraphLabel.left = iFontRight;
		rcParagraphLabel.left += 2;
		rcParagraphLabel.right = iParagraphRight;
		rcParagraphLabel.right -= 2;
		rcParagraphLabel.bottom = rcFontLabel.bottom;
		rcParagraphLabel.top = rcParagraphLabel.bottom - 15;

		control.iIDParagraphLabel = d->GetNewMessageID();

		addText(sPageName,
			control.iIDParagraphLabel, _T("Paragraph"), true, RGB(100, 100, 100), clrBorder, _T(""), sUIFontName, iUIFontSize, rcParagraphLabel, cui_raw::textAlignment::center, m_resize, false);


		// add horizontal separator

		m_resize.iPercH = resize.iPercH;
		m_resize.iPercV = resize.iPercV;
		m_resize.iPercCX = resize.iPercCX;
		m_resize.iPercCY = 0;

		RECT rcSep = rc;
		rcSep.left += 2;
		rcSep.right -= 2;
		rcSep.top = rcFontLabel.bottom + 1;
		rcSep.bottom = rcSep.top + 1;

		control.iIDHSeparator = d->GetNewMessageID();

		addHairLine(sPageName, control.iIDHSeparator, clrBorder, rcSep, m_resize);
	}

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.insert(std::pair<int, cui_rawImpl::RichEditControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addRichEdit

void cui_raw::addRichEdit(const std::basic_string<TCHAR>& sPageName, int iUniqueID, const std::basic_string<TCHAR>& sUIFontName, double iUIFontSize, const std::basic_string<TCHAR>& sFontName, double iFontSize, RECT rc, onResize resize, bool bBorder, COLORREF clrBorder)
{
	return addRichEdit(
		sPageName, iUniqueID,
		sUIFontName, iUIFontSize,
		sFontName, iFontSize,
		rc, resize,
		bBorder,
		clrBorder,
		true,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1,
		-1
	);
} // addRichEdit

bool cui_raw::richEditLoad(
	const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	const std::basic_string<TCHAR> &sFullPath,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.end())
		{
			try
			{
				// load rtf file
				std::fstream fileio;

				fileio.open(std::string(sFullPath.begin(), sFullPath.end()), std::ios::in);

				if (fileio.fail())
				{
					fileio.close();
					
					sErr = _T("Error opening file");
					return false;
				}

				SetStreamType(StreamType_File);	//set stream type
												//setting the stream type is OUR way of knowing if it's
												//a buffer from a string or if it's a file object
												//this isn't window's code, this is our makeshift way

				EDITSTREAM		editstream;
				editstream.pfnCallback = EditStreamCallback;	//tell it the callback function
				editstream.dwCookie = (DWORD_PTR)&fileio;	//pass the file object through cookie
				SendMessage(d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.at(iUniqueID).hWnd, EM_STREAMIN, SF_RTF, (LPARAM)&editstream); //tell it to start stream in
				fileio.close();	//close file

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Rich Edit control not found");
	return false;
} // richEditLoad

bool cui_raw::richEditRTFLoad(
	const std::basic_string<TCHAR> &sPageName, int iUniqueID,
	const std::string &sRTF,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.end())
		{
			try
			{
				// load rtf file
				std::string text(sRTF);

				SetStreamType(StreamType_Buffer);	//set stream as buffer

				EDITSTREAM		editstream;
				editstream.pfnCallback = EditStreamCallback;	//tell it the callback function
				editstream.dwCookie = (DWORD_PTR)&text;	//pass address of buffer
				SendMessage(d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.at(iUniqueID).hWnd, EM_STREAMIN, SF_RTF, (LPARAM)&editstream); //tell it to start stream in

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Rich Edit control not found");
	return false;
} // richEditRTFLoad

bool cui_raw::richEditSave(const std::basic_string<TCHAR>& sPageName, int iUniqueID, std::basic_string<TCHAR>& sFullPath, std::basic_string<TCHAR>& sErr)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.end())
		{
			try
			{
				/*
				** ensure the use of a given extension for full path specified
				** forces database to be saved with the ".extension" extension regardless of what the user specified
				** whatever existing extension will be removed
				** formatted path will be written back to sFullPath
				*/
				auto FormatToExt = [](std::basic_string<TCHAR> &sFullPath, const std::basic_string<TCHAR> &extension)
				{
					std::basic_string<TCHAR> ext;

					ext = extension;

					// Remove extension if present
					while (true)
					{
						const size_t period_idx1 = sFullPath.rfind('.');

						if (std::basic_string<TCHAR>::npos != period_idx1)
							sFullPath.erase(period_idx1);
						else
							break;
					}

					// remove dot(s) from supplied extension (if necessary)
					const size_t period_idx2 = ext.rfind('.');

					if (std::basic_string<TCHAR>::npos != period_idx2)
						ext = ext.substr(period_idx2 + 1);

					// add extension to path
					if (!ext.empty())
						sFullPath = sFullPath + _T(".") + ext;
				}; // FormatToExt

				FormatToExt(sFullPath, _T("rtf"));

				SaveRichTextToFile(d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.at(iUniqueID).hWnd, sFullPath.c_str());

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Rich Edit control not found");
	return false;
} // richEditSave

bool cui_raw::richEditRTFGet(const std::basic_string<TCHAR>& sPageName, int iUniqueID, std::string & sRTF, std::basic_string<TCHAR>& sErr)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.end())
		{
			try
			{
				// get RTF data
				sRTF.clear();

				SetStreamType(StreamType_Buffer);	//set stream as buffer

				EDITSTREAM		editstream;
				editstream.pfnCallback = EditStreamCallbackRead;	//tell it the callback function
				editstream.dwCookie = (DWORD_PTR)&sRTF;	//pass address of buffer
				SendMessage(d->m_Pages.at(sPageName + sPageLessKey).m_RichEditControls.at(iUniqueID).hWnd, EM_STREAMOUT, SF_RTF, (LPARAM)&editstream); //tell it to start stream in
				
				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Rich Edit control not found");
	return false;
} // richEditRTFGet

void cui_raw::addImage(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::basic_string<TCHAR> &sTooltip,
	bool bTightFit,
	bool bChangeColor,
	COLORREF clrImage,
	COLORREF clrImageHot,
	COLORREF clrBorder,
	COLORREF clrBorderHot,
	bool bButtonBar,
	COLORREF clrBar,
	COLORREF clrBackground,
	COLORREF clrBackgroundHot,
	int IDC_PNG,
	const std::basic_string<TCHAR> &sText,
	COLORREF clrText,
	COLORREF clrTextHot,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	imageTextPlacement textPlacement,
	RECT rc, onResize resize, bool bToggle, onToggle toggleAction, bool bDescriptive,
	const std::basic_string<TCHAR> &sDescription, SIZE imageSize,
	HMODULE resource_module
)
{
	return addImage(sPageName,
		iUniqueID,
		sTooltip,
		bTightFit,
		bChangeColor,
		clrImage,
		clrImageHot,
		clrBorder,
		clrBorderHot,
		bButtonBar,
		clrBar,
		clrBackground,
		clrBackgroundHot,
		IDC_PNG,
		sText,
		clrText,
		clrTextHot,
		sFontName, sFontName, iFontSize,
		textPlacement,
		rc, resize, bToggle, toggleAction, bDescriptive,
		sDescription, imageSize, resource_module);
} // addImage

void cui_raw::addImage(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::basic_string<TCHAR> &sTooltip,
	bool bTightFit,
	bool bChangeColor,
	COLORREF clrImage,
	COLORREF clrImageHot,
	COLORREF clrBorder,
	COLORREF clrBorderHot,
	bool bButtonBar,
	COLORREF clrBar,
	COLORREF clrBackground,
	COLORREF clrBackgroundHot,
	int IDC_PNG,
	const std::basic_string<TCHAR> &sText,
	COLORREF clrText,
	COLORREF clrTextHot,
	const std::basic_string<TCHAR> &sFontName, const std::basic_string<TCHAR> &sFontNameDescription, double iFontSize,
	imageTextPlacement textPlacement,
	RECT rc, onResize resize, bool bToggle, onToggle toggleAction, bool bDescriptive,
	const std::basic_string<TCHAR> &sDescription, SIZE imageSize,
	HMODULE resource_module
)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::ImageControl control;
	control.iUniqueID = iUniqueID;

	control.toolTip.sText = sTooltip;
	control.toolTip.sFontName = d->m_sTooltipFont;
	control.toolTip.iFontSize = d->m_iTooltipFontSize;
	control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
	control.toolTip.m_clrText = d->m_clrTooltipText;
	control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
	control.toolTip.d = d;

	control.bImageOnlyTightFit = bTightFit;
	control.sText = sText;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.resize = resize;
	control.clrText = clrText;
	control.clrTextHot = clrTextHot;
	control.sFontName = sFontName;
	control.sFontNameDescription = sFontNameDescription;
	if (control.sFontNameDescription.empty())
		control.sFontNameDescription = control.sFontName;
	control.iFontSize = iFontSize;
	control.iPNGResource = IDC_PNG;
	control.resize = resize;
	control.clrBackground = clrBackground;
	control.clrBackgroundHot = clrBackgroundHot;
	control.clrBorder = clrBorder;
	control.clrBorderHot = clrBorderHot;
	control.bButtonBar = bButtonBar;
	control.clrBar = clrBar;
	control.bToggle = bToggle;
	control.toggleAction = toggleAction;
	control.bChangeColor = bChangeColor;
	control.clrImage = clrImage;
	control.clrImageHot = clrImageHot;
	control.textPlacement = textPlacement;
	control.bDescriptive = bDescriptive;
	control.sDescription = sDescription;
	control.imageSize = imageSize;
	control.resource_module = resource_module;

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.insert(std::pair<int, cui_rawImpl::ImageControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addImage

void cui_raw::addImage(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::basic_string<TCHAR> &sTooltip,
	bool bTightFit,
	bool bChangeColor,
	COLORREF clrImage,
	COLORREF clrImageHot,
	COLORREF clrBorder,
	COLORREF clrBorderHot,
	bool bButtonBar,
	COLORREF clrBar,
	COLORREF clrBackground,
	COLORREF clrBackgroundHot,
	const std::basic_string<TCHAR> &sFileName,
	const std::basic_string<TCHAR> &sText,
	COLORREF clrText,
	COLORREF clrTextHot,
	const std::basic_string<TCHAR> &sFontName, double iFontSize,
	imageTextPlacement textPlacement,
	RECT rc, onResize resize, bool bToggle, onToggle toggleAction, bool bDescriptive,
	const std::basic_string<TCHAR> &sDescription, SIZE imageSize
)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::ImageControl control;
	control.iUniqueID = iUniqueID;

	control.toolTip.sText = sTooltip;
	control.toolTip.sFontName = d->m_sTooltipFont;
	control.toolTip.iFontSize = d->m_iTooltipFontSize;
	control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
	control.toolTip.m_clrText = d->m_clrTooltipText;
	control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
	control.toolTip.d = d;

	control.bImageOnlyTightFit = bTightFit;
	control.sText = sText;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.resize = resize;
	control.clrText = clrText;
	control.clrTextHot = clrTextHot;
	control.sFontName = sFontName;
	control.sFontNameDescription = sFontName;
	control.iFontSize = iFontSize;
	control.iPNGResource = 0;
	control.sFileName = sFileName;
	control.resize = resize;
	control.clrBackground = clrBackground;
	control.clrBackgroundHot = clrBackgroundHot;
	control.clrBorder = clrBorder;
	control.clrBorderHot = clrBorderHot;
	control.bButtonBar = bButtonBar;
	control.clrBar = clrBar;
	control.bToggle = bToggle;
	control.toggleAction = toggleAction;
	control.bChangeColor = bChangeColor;
	control.clrImage = clrImage;
	control.clrImageHot = clrImageHot;
	control.textPlacement = textPlacement;
	control.bDescriptive = bDescriptive;
	control.sDescription = sDescription;
	control.imageSize = imageSize;

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.insert(std::pair<int, cui_rawImpl::ImageControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addImage

bool cui_raw::changeImage(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID, const std::basic_string<TCHAR> &sNewFileName,
	bool bChangeText,
	const std::basic_string<TCHAR> &sNewText,
	bool bUpdate,
	std::basic_string<TCHAR> &sErr)
{

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.end())
		{
			try
			{
				bool bRes = true;

				// remove PNG resource
				d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).iPNGResource = 0;
				d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).GdiplusBitmap_res.Load(-1, _T("PNG"), sErr, d->m_hResModule);

				// attempt to load image from file
				bRes = d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).GdiplusBitmap.Load(sNewFileName.c_str(), sErr);

				if (bRes)
				{
					// delete the display bitmap so that it can be redrawn
					if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).m_pDisplaybitmap)
					{
						delete d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).m_pDisplaybitmap;
						d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).m_pDisplaybitmap = NULL;
					}

					// change the text if the user so desires
					if (bChangeText)
						d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).sText = sNewText;

					// redraw image
					if (bUpdate)
					{
						InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd, NULL, TRUE);
						UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd);
					}
				}

				return bRes;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Image control not found");
	return false;
} // changeImage

bool cui_raw::changeImage(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID, int IDC_PNG,
	bool bChangeText,
	const std::basic_string<TCHAR> &sNewText,
	bool bUpdate,
	std::basic_string<TCHAR> &sErr)
{

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.end())
		{
			try
			{
				bool bRes = true;

				if (IDC_PNG != 0)
				{
					// remove image
					d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).GdiplusBitmap.Load(_T(""), sErr);

					// set PNG resource
					d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).iPNGResource = IDC_PNG;

					// attempt to load image from PNG resource
					bRes = d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).GdiplusBitmap_res.Load(IDC_PNG, _T("PNG"), sErr, d->m_hResModule);

					if (bRes || IDC_PNG == -1)
					{
						// delete the display bitmap so that it can be redrawn
						if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).m_pDisplaybitmap)
						{
							delete d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).m_pDisplaybitmap;
							d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).m_pDisplaybitmap = NULL;
						}

						// change the text if the user so desires
						if (bChangeText)
							d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).sText = sNewText;

						// redraw image
						if (bUpdate)
						{
							InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd, NULL, TRUE);
							UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd);
						}
					}
				}

				return bRes;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Image control not found");
	return false;
} // changeImage

bool cui_raw::changeImageText(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::basic_string<TCHAR> &sNewText,
	const std::basic_string<TCHAR> &sNewDescription,
	bool bUpdate,
	std::basic_string<TCHAR> &sErr)
{

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.end())
		{
			try
			{
				std::basic_string<TCHAR> sOldText = d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).sText;
				std::basic_string<TCHAR> sOldDescription = d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).sDescription;

				if (sOldText != sNewText || sOldDescription != sNewDescription)
				{
					// change the text

					d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).sText = sNewText;
					d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).sDescription = sNewDescription;

					// redraw image
					if (bUpdate)
					{
						LPRECT rcUpdate = NULL;

						if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).m_pDisplaybitmap)
						{
							delete d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).m_pDisplaybitmap;
							d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).m_pDisplaybitmap = NULL;
						}

						if (sOldText.length() != sNewText.length() ||
							(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).bDescriptive &&
								sOldDescription != sNewDescription
								))
							rcUpdate = NULL;
						else
							rcUpdate = &d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).rcText;

						InvalidateRect(
							d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd,
							rcUpdate,
							TRUE);
						UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd);
					}
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Image control not found");
	return false;
} // changeImageText

bool cui_raw::setImageBar(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	COLORREF clrBar,
	bool bUpdate,
	std::basic_string<TCHAR> &sErr)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.end())
		{
			try
			{
				d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).bButtonBar = true;
				d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).clrBar = clrBar;

				if (bUpdate)
				{
					InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd, &d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).rcBar, TRUE);
					UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd);
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Image button control not found");
	return false;
} // setImageBar

bool cui_raw::removeImageBar(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	bool bUpdate,
	std::basic_string<TCHAR> &sErr)
{

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.end())
		{
			try
			{
				d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).bButtonBar = false;

				if (bUpdate)
				{
					InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd, &d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).rcBar, TRUE);
					UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd);
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Image button control not found");
	return false;
} // removeImageBar

bool cui_raw::updateImage(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID, std::basic_string<TCHAR> &sErr)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.end())
		{
			if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).m_pDisplaybitmap)
			{
				delete d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).m_pDisplaybitmap;
				d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).m_pDisplaybitmap = NULL;
			}

			InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd, NULL, TRUE);
			UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd);
			return true;
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Image button control not found");
	return false;
} // updateImage

bool cui_raw::saveImage(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	imgFormat format,
	SIZE maxSize,
	std::basic_string<TCHAR> &sFullPath,	// including or excluding extension
	std::basic_string<TCHAR> &sErr)
{

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.end())
		{
			try
			{
				CImageConv::imageformat m_format;

				switch (format)
				{
				case cui_raw::PNG:
					m_format = CImageConv::imageformat::PNG;
					break;
				case cui_raw::BMP:
					m_format = CImageConv::imageformat::BMP;
					break;
				case cui_raw::JPEG:
					m_format = CImageConv::imageformat::JPEG;
					break;
				case cui_raw::NONE:
					m_format = CImageConv::imageformat::NONE;
					break;
				default:
					m_format = CImageConv::imageformat::PNG;
					break;
				}

				bool bRes = true;

				if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).GdiplusBitmap)
				{
					// attempt to save image to file
					bRes = CImageConv::GDIPLUSBITMAPtoFILE(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).GdiplusBitmap, sFullPath, m_format, maxSize, sErr);
				}
				else
					if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).GdiplusBitmap_res)
					{
						// attempt to save image to file
						bRes = CImageConv::GDIPLUSBITMAPtoFILE(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).GdiplusBitmap_res, sFullPath, m_format, maxSize, sErr);
					}
					else
					{
						sErr = _T("No image in control");
						bRes = false;
					}

				return bRes;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Image control not found");
	return false;
} // saveImage

bool cui_raw::getImageText(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	std::basic_string<TCHAR> &sText,
	std::basic_string<TCHAR> &sErr
)
{
	sText.clear();
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.end())
		{
			try
			{
				sText = d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).sText;

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Image control not found");
	return false;
} // getImageText

bool cui_raw::setImageColors(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	COLORREF clrImage,
	COLORREF clrImageHot,
	COLORREF clrText,
	COLORREF clrTextHot,
	COLORREF clrBorder,
	COLORREF clrBorderHot,
	bool bUpdate,
	std::basic_string<TCHAR> &sErr)
{

	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.end())
		{
			try
			{
				d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).clrImage = clrImage;
				d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).clrImageHot = clrImageHot;
				d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).clrText = clrText;
				d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).clrTextHot = clrTextHot;
				d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).clrBorder = clrBorder;
				d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).clrBorderHot = clrBorderHot;

				int iRefreshWidth = 1;

				if (bUpdate)
				{
					if (!d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).bChangeColor)
					{
						RECT rectButton;
						GetClientRect(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd, &rectButton);
						RECT rc = rectButton;
						rc.right = rc.left + iRefreshWidth;
						InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd, &rc, TRUE);
						UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd);

						rc = rectButton;
						rc.bottom = rc.top + iRefreshWidth;
						InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd, &rc, TRUE);
						UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd);

						rc = rectButton;
						rc.left = rc.right - iRefreshWidth;
						InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd, &rc, TRUE);
						UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd);

						rc = rectButton;
						rc.top = rc.bottom - iRefreshWidth;
						InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd, &rc, TRUE);
						UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd);
					}
					else
					{
						InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd, NULL, TRUE);
						UpdateWindow(d->m_Pages.at(sPageName + sPageLessKey).m_ImageControls.at(iUniqueID).hWnd);
					}
				}

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Image button control not found");
	return false;
} // setImageColors

void cui_raw::addRect(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID, COLORREF clr,
	RECT rc, onResize resize)
{
	scaleRECT(rc, d->m_DPIScale);

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
} // addRect

void cui_raw::addHairLine(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID, COLORREF clr,
	RECT rc, onResize resize)
{
	bool bVerticalHairLine = false;
	bool bHorizontalHairLine = false;

	// verify that either width or height is indeed a hairline
	if ((rc.right - rc.left) == 1)
		bVerticalHairLine = true;
	else
		if ((rc.bottom - rc.top) == 1)
			bHorizontalHairLine = true;

	scaleRECT(rc, d->m_DPIScale);

	if (bVerticalHairLine)
		rc.right = rc.left + 1;
	else
		if (bHorizontalHairLine)
			rc.bottom = rc.top + 1;

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
} // addHairLine

void cui_raw::addStarRating(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	const std::basic_string<TCHAR> &sTooltip,
	COLORREF clrBorder,
	COLORREF clrOn,
	COLORREF clrOff,
	COLORREF clrHot,
	RECT rc, onResize resize, int iInitialRating, int &iHighestRating, bool bStatic)
{
	scaleRECT(rc, d->m_DPIScale);

	cui_rawImpl::StarRatingControl control;
	control.iUniqueID = iUniqueID;

	control.toolTip.sText = sTooltip;
	control.toolTip.sFontName = d->m_sTooltipFont;
	control.toolTip.iFontSize = d->m_iTooltipFontSize;
	control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
	control.toolTip.m_clrText = d->m_clrTooltipText;
	control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
	control.toolTip.d = d;

	control.bStatic = bStatic;

	if (iHighestRating < 3)
		iHighestRating = 3;	// do not allow a rating of less than three stars

	if (iHighestRating > 50)
		iHighestRating = 50;	// maximum rating permitted

	if (iInitialRating < 0)
		iInitialRating = 0;

	if (iInitialRating > iHighestRating)
		iInitialRating = iHighestRating;

	control.iRating = iInitialRating;
	control.iHighestRating = iHighestRating;
	control.clrBorder = clrBorder;
	control.coords.left = rc.left;
	control.coords.top = rc.top;
	control.coords.right = rc.right;
	control.coords.bottom = rc.bottom;
	control.resize = resize;
	control.clrOn = clrOn;
	control.clrOff = clrOff;
	control.clrHot = clrHot;

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
		if (d->m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.find(iUniqueID) == d->m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.end())	// do not duplicate IDs
		{
			d->m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.insert(std::pair<int, cui_rawImpl::StarRatingControl>(iUniqueID, control));
			d->handleTabControls(sPageName + sPageLessKey, iUniqueID);
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // addStarRating

bool cui_raw::getStarRating(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	int &iRating,
	std::basic_string<TCHAR> &sErr
)
{
	iRating = 0;
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.end())
		{
			try
			{
				iRating = d->m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.at(iUniqueID).iRating;

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Star rating control not found");
	return false;
} // getStarRating

bool cui_raw::setStarRating(
	const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	int iRating,
	std::basic_string<TCHAR> &sErr
)
{
	std::basic_string<TCHAR> sPageLessKey;

	if (sPageName.empty())
		sPageLessKey = d->m_sTitle;

	try
	{
		if (d->m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.find(iUniqueID) != d->m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.end())
		{
			try
			{
				// sanity check
				if (iRating < 0)
					iRating = 0;

				if (iRating > d->m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.at(iUniqueID).iHighestRating)
					iRating = d->m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.at(iUniqueID).iHighestRating;

				d->m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.at(iUniqueID).iRating = iRating;

				InvalidateRect(d->m_Pages.at(sPageName + sPageLessKey).m_StarRatingControls.at(iUniqueID).hWnd, NULL, TRUE);

				return true;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
				sErr = std::basic_string<TCHAR>(m_sErr.begin(), m_sErr.end());
				return false;
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	sErr = _T("Star rating control not found");
	return false;
} // setStarRating

void cui_raw::setMinWidthAndHeight(int iMinW, int iMinH)
{
	d->m_iMinWidth = int(0.5 + iMinW * d->m_DPIScale);
	d->m_iMinHeight = int(0.5 + iMinH * d->m_DPIScale);
} // setMinWidthAndHeight

void cui_raw::setIcons(
	int IDI_IconLarge,
	int IDI_IconSmall,
	int IDP_PNGIconSmall
)
{
	d->m_IDI_ICON = IDI_IconLarge;
	d->m_IDI_ICONSMALL = IDI_IconSmall;
	d->m_IDP_ICONSMALL = IDP_PNGIconSmall;
} // setIcon

bool cui_raw::create(int iInitID, int iShutdownID)
{
	return create(iInitID, iShutdownID, true, false, true);
} // create

bool cui_raw::create(int iInitID,
	int iShutdownID,
	bool bActivate,
	bool bTopMost,
	bool bVisible
)
{
	if (d->m_bCreated)
		return false;

	if (bActivate == false && bTopMost == true)
	{
		// this is a notification window. TO-DO: find a more reliable way of th
		d->m_bNotification = true;
	}

	d->m_sPageList.push_back(d->m_sCurrentPage);

	// register window class
	d->wcex.cbSize = sizeof(WNDCLASSEX);
	d->wcex.lpfnWndProc = d->WndProc;
	d->wcex.cbClsExtra = 0;
	d->wcex.cbWndExtra = 0;
	d->wcex.style = CS_DBLCLKS;	// receive double-click events
	d->wcex.hInstance = GetModuleHandle(NULL);

	// load main application icon
	if (d->m_IDI_ICON)
		d->wcex.hIcon = LoadIcon(d->m_hResModule, MAKEINTRESOURCE(d->m_IDI_ICON));

	// load small application icon
	if (d->m_IDI_ICONSMALL)
	{
		d->wcex.hIconSm = (HICON)LoadImage(
			d->m_hResModule,
			MAKEINTRESOURCE(d->m_IDI_ICONSMALL),
			IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
			LR_DEFAULTCOLOR
		);
	}

	d->wcex.hCursor = LoadCursor(NULL, IDC_ARROW);

	CBrush brBck(d->m_clrBackground);
	d->wcex.hbrBackground = brBck.get();

	//d->wcex.lpszMenuName = MAKEINTRESOURCE(IDC_WIN32PROJECT7);

	d->wcex.lpszClassName = _T("cui_raw Window");

	RegisterClassEx(&d->wcex);

	DWORD dwExtended = NULL;

	if (bTopMost)
		dwExtended = WS_EX_TOPMOST;

	// Perform initialization
	d->m_hWnd = CreateWindowEx(dwExtended, d->wcex.lpszClassName, d->m_sCurrentPage.c_str(), WS_POPUP | WS_CLIPSIBLINGS,
		d->m_ix, d->m_iy, d->m_icx, d->m_icy, d->m_hWndParent, NULL, GetModuleHandle(NULL), this);

	if (!d->m_hWnd)
		return false;

	// if resizing is disabled, disable the maximize button
	if (!d->m_bEnableResize)
	{
		if (d->m_bMaxbtn)
			disableControl(d->m_sCurrentPage, d->m_iIDMax);
	}

	// call the command procedure (initialization)
	if (d->m_Pages[d->m_sCurrentPage].m_pCommandProc)
		d->m_Pages[d->m_sCurrentPage].m_pCommandProc(*this, iInitID, NULL);

	// capture shutdown ID
	d->m_iShutdownID = iShutdownID;

	DWORD swShow = SW_SHOWNA;

	if (bActivate)
		swShow = SW_SHOW;

	if (bVisible)
	{
		if (d->m_bMaximized)
			swShow = SW_MAXIMIZE;
	}
	else
	{
		swShow = SW_HIDE;

		if (d->m_bMaximized)
			swShow |= SW_MAXIMIZE;
	}

	// critical for tab controls
	d->showPage(d->m_sCurrentPage);

	ShowWindow(d->m_hWnd, swShow);
	UpdateWindow(d->m_hWnd);

	if (bActivate && IsWindow(d->m_hWndParent) && !IsWindowVisible(d->m_hWndParent))
		SetForegroundWindow(d->m_hWnd);

	d->m_bCreated = true;

	bool bHasParent = false;

	// disable parent
	if (IsWindow(d->m_hWndParent))
	{
		d->parent_was_enabled = IsWindowEnabled(d->m_hWndParent) == TRUE;
		bHasParent = true;

		if (d->parent_was_enabled)
			EnableWindow(d->m_hWndParent, FALSE);
	}

	MSG msg = {};

	// Main message loop
	while (GetMessage(&msg, NULL, 0, 0))
	{
		// IsDialogMessage() is critical for WS_TABSTOP to work in a custom window
		if (!IsDialogMessage(d->m_hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	if (bHasParent)
	{
		if (d->m_bParentClosing)
		{
			// child has been closed by parent and parent wants to close
			PostQuitMessage(0);
		}
		else
		{
			// child is closing
		}
	}

	return true;
} // create

void cui_raw::closeHard()
{
	if (IsWindow(d->m_hWnd))
		PostMessage(d->m_hWnd, WM_CLOSE, (WPARAM)1, NULL);	// 1 means close NOW
}

void cui_raw::close()
{
	if (IsWindow(d->m_hWnd))
		PostMessage(d->m_hWnd, WM_CLOSE, (WPARAM)2, NULL);	// 2 means close IF closing is permitted
}

void cui_raw::hideWindow()
{
	if (IsWindow(d->m_hWnd))
		ShowWindow(d->m_hWnd, SW_HIDE);
}

void cui_raw::showWindow()
{
	if (IsWindow(d->m_hWnd))
	{
		if (IsMinimized(d->m_hWnd))
		{
			// restore
			ShowWindow(d->m_hWnd, SW_RESTORE);
		}
		else
			if (!IsWindowVisible(d->m_hWnd))
			{
				// show
				ShowWindow(d->m_hWnd, SW_SHOW);
			}
	}
} // showWindow

void cui_raw::showWindow(bool bForeground)
{
	if (IsWindow(d->m_hWnd))
	{
		if (bForeground)
			SetForegroundWindow(d->m_hWnd);

		showWindow();
	}
} // showWindow

void cui_raw::preventResizing()
{
	d->m_bEnableResize = false;

	// if max button exists, disable it
	if (d->m_bMaxbtn)
	{
		try
		{
			disableWindow(d->m_ControlBtns.at(d->m_iIDMax).hWnd);
		}
		catch (std::exception &e)
		{
			std::string m_sErr = e.what();
		}
	}
}

void cui_raw::allowResizing()
{
	d->m_bEnableResize = true;

	// if max button exists, enable it
	if (d->m_bMaxbtn)
	{
		try
		{
			enableWindow(d->m_ControlBtns.at(d->m_iIDMax).hWnd);
		}
		catch (std::exception &e)
		{
			std::string m_sErr = e.what();
		}
	}
}

bool cui_raw::resizing()
{
	return d->m_bEnableResize;
}

std::basic_string<TCHAR> cui_raw::getTitle()
{
	return d->m_sTitle;
} // getTitle

void cui_raw::addToPreventQuitList(const std::basic_string<TCHAR> &sPageName, int iUniqueID)
{
	std::vector<HWND> vHWNDs;
	HWND hWnd = d->getControlHWND(sPageName, vHWNDs, iUniqueID);

	if (hWnd)
	{
		if (std::find(d->m_vPreventQuitList.begin(), d->m_vPreventQuitList.end(), hWnd) == d->m_vPreventQuitList.end())
			d->m_vPreventQuitList.push_back(hWnd);

		for (auto &it : vHWNDs)
		{
			if (std::find(d->m_vPreventQuitList.begin(), d->m_vPreventQuitList.end(), it) == d->m_vPreventQuitList.end())
				d->m_vPreventQuitList.push_back(it);
		}
	}

	return;
} // addToPreventQuitList

void cui_raw::preventQuit()
{
	d->m_iStopQuit++;

	try
	{
		// disable close button
		if (d->m_bClosebtn)
			disableWindow(d->m_ControlBtns.at(d->m_iIDClose).hWnd);

		// disable all controls in the prevent quit list
		for (auto &it : d->m_vPreventQuitList)
			disableWindow(it);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // PreventQuit

void cui_raw::allowQuit()
{
	if (d->m_iStopQuit > 0)
		d->m_iStopQuit--;

	if (d->m_iStopQuit <= 0)
	{
		try
		{
			// enable all controls in the prevent quit list
			// disable all controls in the prevent quit list
			for (auto &it : d->m_vPreventQuitList)
				enableWindow(it);

			// enable close button
			if (d->m_bClosebtn)
				enableWindow(d->m_ControlBtns.at(d->m_iIDClose).hWnd);
		}
		catch (std::exception &e)
		{
			std::string m_sErr = e.what();
		}
	}
} // AllowQuit

std::basic_string<TCHAR> cui_raw::currentPage()
{
	return d->m_sCurrentPage;
} // currentPage

void cui_raw::setTimer(unsigned int iTimer, bool bStartOnMouseMove, bool bStopOnMouseOverWindow)
{
	d->m_iTimer = iTimer;
	d->m_bStartOnMouseMove = bStartOnMouseMove;
	d->m_bStopOnMouseOverWindow = bStopOnMouseOverWindow;
	d->m_bTimerRunning = false;

	if (d->m_bCreated && IsWindow(d->m_hWnd))
	{
		// window has already been created, start timer immediately
		if (!d->m_bStartOnMouseMove)
		{
			// set timer running flag to true
			d->m_bTimerRunning = true;

			// start the timer
			SetTimer(d->m_hWnd, d->ID_TIMER, 1000 * d->m_iTimer, NULL);
		}
		else
		{
			// check cursor position
			d->m_ptStartCheck = { 0 };
			GetCursorPos(&d->m_ptStartCheck);

			// set check every two seconds
			SetTimer(d->m_hWnd, d->ID_TIMER_CHECK, 2000, NULL);
		}
	}

	return;
} // setTimer

bool cui_raw::HBITMAPtoFile(
	HBITMAP hBmp,
	std::basic_string<TCHAR> &sFileName,
	imgFormat format,
	std::basic_string<TCHAR> &sErr
)
{
	CImageConv::imageformat m_format;

	switch (format)
	{
	case cui_raw::PNG:
		m_format = CImageConv::imageformat::PNG;
		break;
	case cui_raw::BMP:
		m_format = CImageConv::imageformat::BMP;
		break;
	case cui_raw::JPEG:
		m_format = CImageConv::imageformat::JPEG;
		break;
	case cui_raw::NONE:
		m_format = CImageConv::imageformat::NONE;
		break;
	default:
		m_format = CImageConv::imageformat::PNG;
		break;
	}

	return CImageConv::HBITMAPtoFILE(hBmp, sFileName, m_format, sErr);
} // HBITMAPtoFile

bool cui_raw::addPage(const std::basic_string<TCHAR> &sPageName, CommandProcedure pCommandProc)
{
	if (d->m_Pages.find(sPageName) == d->m_Pages.end())	// do not duplicate IDs
	{
		// page doesn't exist ... add it
		d->m_Pages[sPageName];

		// set page's command procedure
		d->m_Pages.at(sPageName).m_pCommandProc = pCommandProc;

		return true;
	}
	else
	{
		// page already exists ... return false
		return false;
	}
} // addPage

bool cui_raw::checkPage(const std::basic_string<TCHAR> &sPageName)
{
	if (d->m_Pages.find(sPageName) == d->m_Pages.end())	// do not duplicate IDs
		return false;
	else
		return true;
} // checkPage

void cui_raw::createPage(const std::basic_string<TCHAR> &sPageName)
{
	if (d->m_Pages.find(sPageName) == d->m_Pages.end())
		return;	// page doesn't exist

				// add controls to new page
	d->AddControls(d->m_hWnd, sPageName, this);
} // createPage

void cui_raw::showPage(const std::basic_string<TCHAR> &sPageName, bool bCreatePage)
{
	if (IsIconic(d->m_hWnd))
	{
		// window is minimized ... restore it ...
		ShowWindow(d->m_hWnd, SW_RESTORE);
		UpdateWindow(d->m_hWnd);
	}

	if (d->m_Pages.find(sPageName) == d->m_Pages.end())
		return;	// page doesn't exist

	std::basic_string<TCHAR> sCurrentPage = d->m_sCurrentPage;
	std::basic_string<TCHAR> sNewPage = sPageName;

	try
	{
		if (previousPage() != sNewPage)
		{
			if (sCurrentPage == sNewPage)
			{
				// same page is "refresing", return without doing anything
				return;
			}

			///////////////////////////////////
			// opening a new page
			// add page to end of page list
			d->m_sPageList.push_back(sNewPage);
			///////////////////////////////////
		}
		else
		{
			///////////////////////////////////
			// switching back to previous page
			// remove last page in page list
			d->m_sPageList.pop_back();
			///////////////////////////////////
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// hide current page
	d->hidePage(sCurrentPage);

	if (bCreatePage)
	{
		// add controls to new page
		d->AddControls(d->m_hWnd, sNewPage, this);
	}

	// show new page
	d->showPage(sNewPage);

	// switch to the new page
	d->m_sCurrentPage = sNewPage;
} // showPage

std::basic_string<TCHAR> cui_raw::previousPage()
{
	std::basic_string<TCHAR> sPreviousPage;

	try
	{
		if (!d->m_sPageList.empty())
		{
			// there are pages in the page list
			size_t iSize = d->m_sPageList.size();

			if (iSize > 1)
			{
				// the current page has a previous page
				sPreviousPage = d->m_sPageList[iSize - 2];
			}
			else
			{
				// the current page is the root page (no previous page)
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	return sPreviousPage;
} // previousPage

int cui_raw::getWidth()
{
	if (IsIconic(d->m_hWnd))
	{
		// use last saved dimensions
		return d->m_iWidth;
	}
	else
	{
		RECT rc;
		GetClientRect(d->m_hWnd, &rc);

		UNscaleRECT(rc, d->m_DPIScale);

		return (rc.right - rc.left);
	}
} // getWidth

int cui_raw::getHeight()
{
	if (IsIconic(d->m_hWnd))
	{
		// use last saved dimensions
		return d->m_iHeight;
	}
	else
	{
		RECT rc;
		GetClientRect(d->m_hWnd, &rc);

		UNscaleRECT(rc, d->m_DPIScale);

		return (rc.bottom - rc.top);
	}
} // getHeight

int cui_raw::getX()
{
	RECT rc;
	GetWindowRect(d->m_hWnd, &rc);

	return rc.left;
} // getX

int cui_raw::getY()
{
	RECT rc;
	GetWindowRect(d->m_hWnd, &rc);

	return rc.top;
} // getY

void cui_raw::setXY(int x, int y)
{
	RECT rc;
	rc.left = x;
	rc.top = y;

	// make sure bottom right of window is visible
	RECT rcWork;
	d->GetWorkingArea(d->m_hWnd, rcWork);

	RECT rcWindow;
	GetWindowRect(d->m_hWnd, &rcWindow);

	int iWidth = rcWindow.right - rcWindow.left;
	int iHeight = rcWindow.bottom - rcWindow.top;

	if (rc.left + iWidth > rcWork.right)
		rc.left = rcWork.right - iWidth;

	if (rc.top + iHeight > rcWork.bottom)
		rc.top = rcWork.bottom - iHeight;

	SetWindowPos(d->m_hWnd, NULL, rc.left, rc.top, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
} // setXY

void cui_raw::setXY(int x, int y, int cx, int cy)
{
	RECT rc;
	rc.left = x;
	rc.top = y;

	// make sure bottom right of window is visible
	RECT rcWork;
	d->GetWorkingArea(d->m_hWnd, rcWork);

	// make sure dimensions are not less than minimum allowed
	int iWidth = cx > d->m_iMinWidth ? cx : d->m_iMinWidth;
	int iHeight = cy > d->m_iMinHeight ? cy : d->m_iMinHeight;

	if (rc.left + iWidth > rcWork.right)
		rc.left = rcWork.right - iWidth;

	if (rc.top + iHeight > rcWork.bottom)
		rc.top = rcWork.bottom - iHeight;

	SetWindowPos(d->m_hWnd, NULL, rc.left, rc.top, iWidth, iHeight, SWP_NOZORDER | SWP_NOACTIVATE);
} // setXY

void cui_raw::getWorkingArea(RECT &rcWork)
{
	d->GetWorkingArea(d->m_hWnd, rcWork);
} // getWorkingArea

HWND cui_raw::getHWND()
{
	if (IsWindow(d->m_hWnd))
		return d->m_hWnd;
	else
		return NULL;
} // getNativeHandle

bool cui_raw::isMaximized()
{
	return IsMaximized(d->m_hWnd) == TRUE;
} // isMaximized

void cui_raw::maximize()
{
	if (IsWindowVisible(d->m_hWnd))
	{
		ShowWindow(d->m_hWnd, SW_MAXIMIZE);
	}
	else
	{
		// window hasn't yet been displayed ... this function is either being called before create()
		// or within the initialization procedure (when iInitID is called in the window procedure).
		d->m_bMaximized = true;
	}
} // maximize

int cui_raw::getTitleBarHeight()
{
	return int(0.5 + (double)d->m_iTitlebarHeight / d->m_DPIScale);
} // getTitleBarHeight

double cui_raw::getDPIScale()
{
	return d->m_DPIScale;
} // getDPIScale

void* cui_raw::getState()
{
	return d->pState_user;
} // getState

void cui_raw::setState(void* pState)
{
	d->pState_user = pState;
} // setState

bool cui_raw::addTabControl(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	RECT rc,
	const std::basic_string<TCHAR> &sFontName, double iFontSize, COLORREF clrTabLine)
{
	scaleRECT(rc, d->m_DPIScale);

	if (d->m_Pages.find(sPageName) == d->m_Pages.end())	// do not duplicate IDs
	{
		// page doesn't exist ...
		return false;
	}
	else
	{
		if (d->m_Pages.at(sPageName).m_TabControl.iUniqueID == -123)	// to-do: remove magic number
		{
			// page exists ... add tab control
			cui_rawImpl::tabControl control;
			control.iUniqueID = iUniqueID;

			control.toolTip.sText = _T("");	// tooltip is going to be set on a per-tab basis
			control.toolTip.sFontName = d->m_sTooltipFont;
			control.toolTip.iFontSize = d->m_iTooltipFontSize;
			control.toolTip.m_clrBackground = d->m_clrTooltipBackground;
			control.toolTip.m_clrText = d->m_clrTooltipText;
			control.toolTip.m_clrBorder = d->m_clrTooltipBorder;
			control.toolTip.d = d;

			control.sFontName = sFontName;
			control.iFontSize = iFontSize;
			control.coords.left = rc.left;
			control.coords.top = rc.top;
			control.coords.right = rc.right;
			control.coords.bottom = rc.bottom;
			control.clrTabLine = clrTabLine;

			cui_raw::onResize resize;
			resize.iPercCY = 100;
			resize.iPercH = 0;
			resize.iPercV = 0;
			control.resize = resize;

			try
			{
				// tab control doesn't exist yet ... add it to the page
				d->m_Pages.at(sPageName).m_TabControl = control;
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
			}
		}

		return true;
	}
} // addTabControl

bool cui_raw::addTab(const std::basic_string<TCHAR> &sPageName,
	const std::basic_string<TCHAR> &sTabName,
	const std::basic_string<TCHAR> &sTooltip)
{
	if (d->m_Pages.find(sPageName) == d->m_Pages.end())	// do not duplicate IDs
	{
		// page doesn't exist ...
		return false;
	}
	else
	{
		// page exists ...
		d->m_Pages.at(sPageName).m_sCurrentTab = sTabName;

		if (d->m_Pages.at(sPageName).m_TabControl.iUniqueID != -123)	// to-do: remove magic number
		{
			// check if tab has already been added
			for (size_t i = 0; i < d->m_Pages.at(sPageName).m_TabControl.vTabs.size(); i++)
			{
				if (d->m_Pages.at(sPageName).m_TabControl.vTabs[i].sCaption == sTabName)
				{
					// tab has already been added
					return true;
				}
			}

			// tab control exists ... add tab
			cui_rawImpl::tab tab;
			tab.sCaption = sTabName;
			tab.sTooltip = sTooltip;

			d->m_Pages.at(sPageName).m_TabControl.vTabs.push_back(tab);
		}

		return true;
	}
} // addTab

bool cui_raw::showTab(const std::basic_string<TCHAR> &sPageName,
	const std::basic_string<TCHAR> &sTabName)
{
	if (d->m_Pages.find(sPageName) == d->m_Pages.end())	// do not duplicate IDs
	{
		// page doesn't exist ...
		return false;
	}
	else
	{
		// page exists ...

		if (d->m_Pages.at(sPageName).m_TabControl.iUniqueID != -123)	// to-do: remove magic number
		{
			// check if there is already a selected tab
			int iSelectedCount = 0;
			for (size_t i = 0; i < d->m_Pages.at(sPageName).m_TabControl.vTabs.size(); i++)
			{
				if (d->m_Pages.at(sPageName).m_TabControl.vTabs[i].bSelected == true)
					iSelectedCount++;
			}

			if (iSelectedCount == 0)
			{
				// mark tab as selected
				for (size_t i = 0; i < d->m_Pages.at(sPageName).m_TabControl.vTabs.size(); i++)
				{
					if (d->m_Pages.at(sPageName).m_TabControl.vTabs[i].sCaption == sTabName)
						d->m_Pages.at(sPageName).m_TabControl.vTabs[i].bSelected = true;
					else
						d->m_Pages.at(sPageName).m_TabControl.vTabs[i].bSelected = false;
				}
			}
			else
			{
				if (iSelectedCount == 1)
				{
					// do nothing ... there is already a selected tab ... this page is probably being shown again after an earlier initialization ...
				}
				else
				{
					// failsafe mechanism in case more than one tab is selected ... for some crazy reason ...
					for (size_t i = 0; i < d->m_Pages.at(sPageName).m_TabControl.vTabs.size(); i++)
					{
						if (i == 0)	// select tab 1 by default
							d->m_Pages.at(sPageName).m_TabControl.vTabs[i].bSelected = true;
						else
							d->m_Pages.at(sPageName).m_TabControl.vTabs[i].bSelected = false;
					}
				}
			}
		}

		return true;
	}
} // showTab

bool cui_raw::getSelectedTab(const std::basic_string<TCHAR> &sPageName,
	int iUniqueID,
	std::basic_string<TCHAR> &sSelectedTab,
	std::basic_string<TCHAR> &sErr)
{
	sSelectedTab.clear();

	if (d->m_Pages.find(sPageName) == d->m_Pages.end())	// do not duplicate IDs
	{
		// page doesn't exist ...
		return false;
	}
	else
	{
		// page exists ...

		if (d->m_Pages.at(sPageName).m_TabControl.iUniqueID != -123)	// to-do: remove magic number
		{
			// check if there is already a selected tab
			int iSelectedCount = 0;
			for (auto it : d->m_Pages.at(sPageName).m_TabControl.vTabs)
			{
				if (it.bSelected == true)
				{
					sSelectedTab = it.sCaption;
					break;
				}
			}
		}

		return true;
	}
} // getSelectedTab

  /// <summary>
  /// Splash screen implementation class.
  /// </summary>
class liblec::cui::gui_raw::CSplashScreenImpl
{
public:
	CSplashScreenImpl()
	{
		pSplash = NULL;
	}

	~CSplashScreenImpl()
	{
		if (pSplash)
		{
			RemoveSplash();
		}
	}

	// Load splash screen
	bool LoadSplash(
		HMODULE hResModule,
		int IDI_SPLASHICON,
		int IDP_SPLASH,
		std::basic_string<TCHAR> &sErr
	)
	{
		if (!pSplash)
		{
			pSplash = new CSplash();

			return pSplash->LoadSplash(hResModule, IDI_SPLASHICON, IDP_SPLASH, sErr);
		}
		else
			return true;
	}

	// remove splash screen
	void RemoveSplash()
	{
		if (pSplash)
		{
			pSplash->RemoveSplash();
			delete pSplash;
			pSplash = NULL;
		}
	}

private:
	CSplash* pSplash;
}; // CSplashScreenImpl

cui_raw::CSplashScreen::CSplashScreen()
{
	d = new CSplashScreenImpl();
}

cui_raw::CSplashScreen::~CSplashScreen()
{
	if (d)
	{
		delete d;
		d = NULL;
	}
}

bool cui_raw::CSplashScreen::Load(
	HMODULE hResModule,
	int IDI_ICONSMALL,
	int IDP_SPLASH,
	std::basic_string<TCHAR> &sErr
)
{
	return d->LoadSplash(hResModule, IDI_ICONSMALL, IDP_SPLASH, sErr);
}

void cui_raw::CSplashScreen::Remove()
{
	return d->RemoveSplash();
}

bool cui_raw::addTrayIcon(
	int IDI_SMALLICON,
	const std::basic_string<TCHAR> &sTitle,
	const std::vector<trayIconItem> &trayItems,
	std::basic_string<TCHAR> &sErr
)
{
	if (!IDI_SMALLICON)
	{
		sErr = _T("Small application icon not specified");
		return false;
	}

	// remove tray icon if it exists
	if (!d->m_trayIcon.m_trayItems.empty())
		removeTrayIcon();

	if (!trayItems.empty())
	{
		UINT uCallbackMsg = WM_APP;

		// create system tray icon
		NOTIFYICONDATA  nid;
		nid.hWnd = d->m_hWnd;
		nid.uID = d->uID;
		nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
		nid.uCallbackMessage = uCallbackMsg;

		nid.hIcon = (HICON)LoadImage(
			d->m_hResModule,
			MAKEINTRESOURCE(IDI_SMALLICON),
			IMAGE_ICON,
			GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
			LR_DEFAULTCOLOR);

		lstrcpy(nid.szTip, sTitle.c_str());	// tooltip title

											// send message to system tray to add icon
		Shell_NotifyIcon(NIM_ADD, &nid);

		// add tray items to list
		d->m_trayIcon.IDI_SMALLICON = IDI_SMALLICON;
		d->m_trayIcon.sTitle = sTitle;
		d->m_trayIcon.m_trayItems = trayItems;
	}

	return true;
} // CTrayIcon

bool cui_raw::changeTrayIcon(
	int IDI_SMALLICON,
	const std::basic_string<TCHAR> &sTitle,
	std::basic_string<TCHAR> &sErr
)
{
	if (!IDI_SMALLICON)
	{
		sErr = _T("Small application icon not specified");
		return false;
	}

	if (!d->m_trayIcon.m_trayItems.empty())
	{
		if (
			IDI_SMALLICON != d->m_trayIcon.IDI_SMALLICON ||	// tray icon has changed
			sTitle != d->m_trayIcon.sTitle					// tray icon title has changed
			)
		{
			UINT uCallbackMsg = WM_APP;

			// create system tray icon
			NOTIFYICONDATA  nid;
			nid.hWnd = d->m_hWnd;
			nid.uID = d->uID;
			nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
			nid.uCallbackMessage = uCallbackMsg;

			nid.hIcon = (HICON)LoadImage(
				d->m_hResModule,
				MAKEINTRESOURCE(IDI_SMALLICON),
				IMAGE_ICON,
				GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON),
				LR_DEFAULTCOLOR);

			lstrcpy(nid.szTip, sTitle.c_str());	// tooltip title

												// send message to system tray to modify icon
			Shell_NotifyIcon(NIM_MODIFY, &nid);

			// update in-class tray icon info
			d->m_trayIcon.IDI_SMALLICON = IDI_SMALLICON;
			d->m_trayIcon.sTitle = sTitle;
		}
	}

	return true;
} // changeTrayIcon

void cui_raw::removeTrayIcon()
{
	if (!d->m_trayIcon.m_trayItems.empty())
	{
		NOTIFYICONDATA  nid;
		nid.hWnd = d->m_hWnd;
		nid.uID = d->uID;

		Shell_NotifyIcon(NIM_DELETE, &nid);

		// reset in-class tray icon info
		d->m_trayIcon.m_trayItems.clear();
		d->m_trayIcon.IDI_SMALLICON = 0;
		d->m_trayIcon.sTitle.clear();
	}
} // removeTrayIcon

void cui_raw::registerInstance(
	const std::basic_string<TCHAR> &sUniqueRegID,
	int iCopyDataID
)
{
	if (!sUniqueRegID.empty())
		d->m_iRegID = RegisterWindowMessage(sUniqueRegID.c_str());

	d->m_iCopyDataID = iCopyDataID;
} // registerInstance

  /// <summary>
  /// Data struct for use with the searcher callback function.
  /// </summary>
struct searcherStruct
{
	/// <summary>
	/// The window's unique registration ID.
	/// </summary>
	UINT iUniqueRegID = 0;

	/// <summary>
	/// The window handle.
	/// </summary>
	HWND hWnd = NULL;
}; // searcherStruct

   /// <summary>
   /// Search for an existing instance of such a registered window.
   /// </summary>
   /// 
   /// <param name="hWnd">
   /// The handle of the window.
   /// </param>
   /// 
   /// <param name="lParam">
   /// Pointer to data struct.
   /// </param>
   /// 
   /// <returns>
   /// return TRUE if the window is not found, and FALSE if it is found.
   /// </returns>
BOOL CALLBACK searcher(HWND hWnd, LPARAM lParam)
{
	searcherStruct *p = (searcherStruct*)lParam;

	UINT UWM_ARE_YOU_ME = p->iUniqueRegID;

	DWORD_PTR result = 0;
	LRESULT ok = ::SendMessageTimeout(hWnd,
		UWM_ARE_YOU_ME,	// message
		0,				// WPARAM
		0,
		SMTO_BLOCK |
		SMTO_ABORTIFHUNG,
		200,
		&result);

	if (ok == 0)
		return TRUE; // ignore this and continue

	if (result == UWM_ARE_YOU_ME)
	{
		// found it
		p->hWnd = hWnd;
		return FALSE; // stop search
	}

	return TRUE; // continue search
} // searcher

bool cui_raw::openExistingInstance(const std::basic_string<TCHAR> &sUniqueRegID)
{
	if (!sUniqueRegID.empty())
	{
		searcherStruct m;
		m.iUniqueRegID = RegisterWindowMessage(sUniqueRegID.c_str());
		EnumWindows(searcher, (LPARAM)&m);

		if (m.hWnd != NULL)
		{
			// pop up
			::SetForegroundWindow(m.hWnd);

			if (IsMinimized(m.hWnd))
			{
				// restore
				ShowWindow(m.hWnd, SW_RESTORE);
			}
			else
				if (!IsWindowVisible(m.hWnd))
				{
					// show
					ShowWindow(m.hWnd, SW_SHOW);
				}

			// check if there is data in the commandline
			CCmdLine args;

			std::string s;

			if (args.size() > 1)
			{
				for (size_t i = 1; i < args.size(); i++)
				{
					std::string m_s = args[i];

					if (m_s.find(" ") != std::string::npos)
						m_s = "\"" + m_s + "\"";

					if (s.empty())
						s = m_s;
					else
						s += " " + m_s;
				}

				// pass the commandline to the existing instance
				LPSTR szCmdLine = (LPSTR)s.c_str();

				COPYDATASTRUCT cds;
				cds.cbData = strlen(szCmdLine) + 1;
				cds.lpData = szCmdLine;

				DWORD_PTR result = 0;
				LRESULT ok = ::SendMessageTimeout(m.hWnd,
					WM_COPYDATA,	// message
					0,				// WPARAM
					(LPARAM)&cds,	// LPARAM
					SMTO_BLOCK |
					SMTO_ABORTIFHUNG,
					500,			// a bit more generaous since we know out window
					&result);
			}

			return true;
		}
	}

	return false;
} // openExistingInstance

void cui_raw::dropFilesAccept(int iDropFilesID)
{
	if (d->m_hWnd && IsWindow(d->m_hWnd) && iDropFilesID != 0)
	{
		d->m_iDropFilesID = iDropFilesID;
		DragAcceptFiles(d->m_hWnd, TRUE);
	}
} // dropFilesAccept

void cui_raw::dropFilesReject()
{
	if (d->m_hWnd && IsWindow(d->m_hWnd))
	{
		DragAcceptFiles(d->m_hWnd, FALSE);
		d->m_iDropFilesID = 0;
	}
} // dropFilesReject

bool cui_raw::resizeImage(
	const std::basic_string<TCHAR> &sSourceFullPath,
	imgFormat format,
	std::basic_string<TCHAR> &sDestinationFullPath,
	SIZE maxSize,
	SIZE &realSize,
	std::basic_string<TCHAR> &sErr
)
{
	CGdiPlusBitmap bitmap;

	if (!bitmap.Load(sSourceFullPath.c_str(), sErr))
		return false;

	CImageConv::imageformat m_format;

	switch (format)
	{
	case cui_raw::PNG:
		m_format = CImageConv::imageformat::PNG;
		break;
	case cui_raw::BMP:
		m_format = CImageConv::imageformat::BMP;
		break;
	case cui_raw::JPEG:
		m_format = CImageConv::imageformat::JPEG;
		break;
	case cui_raw::NONE:
		m_format = CImageConv::imageformat::NONE;
		break;
	default:
		m_format = CImageConv::imageformat::PNG;
		break;
	}

	realSize = maxSize;

	// save resized bitmap to file
	if (!CImageConv::GDIPLUSBITMAPtoFILE(bitmap, sDestinationFullPath, m_format, realSize, sErr))
		return false;

	return true;
} // resizeImage

void cui_raw::pickColor(bool & bColorPicked, COLORREF & rgb)
{
	rgb = RGB(0, 0, 0);
	bColorPicked = false;

	COLORREF crCustColor[] = {
		RGB(0,     5,   5),
		RGB(0,    15,  55),
		RGB(0,    25, 155),
		RGB(0,    35, 255),
		RGB(10,    0,   5),
		RGB(10,   20,  55),
		RGB(10,   40, 155),
		RGB(10,   60, 255),
		RGB(100,   5,   5),
		RGB(100,  25,  55),
		RGB(100,  50, 155),
		RGB(100, 125, 255),
		RGB(200, 120,   5),
		RGB(200, 150,  55),
		RGB(200, 200, 155),
		RGB(200, 250, 255)
	};

	CHOOSECOLOR cc;
	cc.lStructSize = sizeof(CHOOSECOLOR);
	cc.hwndOwner = d->m_hWnd;
	cc.hInstance = NULL;
	cc.rgbResult = RGB(0x80, 0x80, 0x80);
	cc.lpCustColors = crCustColor;
	cc.Flags = CC_RGBINIT/* | CC_FULLOPEN*/;
	cc.lCustData = 0;
	cc.lpfnHook = NULL;
	cc.lpTemplateName = NULL;

	if (ChooseColor(&cc))
	{
		rgb = cc.rgbResult;
		bColorPicked = true;
	}
	else
		bColorPicked = false;

	return;
} // pickColor
