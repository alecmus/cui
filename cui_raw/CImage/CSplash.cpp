/*
** CSplash.cpp - splash screen implementation
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

#include "CImage.h"

// Window Class name
const TCHAR * c_szSplashClass = _T("cui_raw SplashWindow");

CSplash::CSplash()
{
	m_hwndSplash = NULL;
	//pTimer = new CTimer();
}

CSplash::~CSplash()
{
	RemoveSplash();
}

/*
** Registers a window class for the splash and splash owner windows.
*/
void CSplash::RegisterWindowClass(
	int IDI_SPLASHICON		// ID of icon to use for splash screen window
)
{
	HINSTANCE g_hInstance = GetModuleHandle(NULL);	// mod
	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = DefWindowProc;
	wc.hInstance = g_hInstance;
	wc.hIcon = LoadIcon(g_hInstance, MAKEINTRESOURCE(IDI_SPLASHICON));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = c_szSplashClass;

	if (RegisterClass(&wc) == NULL)
	{
		/*
		** splash window registration failed
		*/
	}
} // RegisterWindowClass

  /*
  ** Create the splash owner window and the splash window
  ** returns a handle to the splash window
  */
HWND CSplash::CreateSplashWindow(
	int IDI_SPLASHICON		// ID of icon to use for splash screen window
)
{
	RegisterWindowClass(IDI_SPLASHICON);

	HINSTANCE g_hInstance = GetModuleHandle(NULL);
	HWND hwndOwner = CreateWindow(c_szSplashClass, NULL, WS_POPUP,
		0, 0, 0, 0, NULL, NULL, g_hInstance, NULL);

	if (hwndOwner == NULL)
	{
		/*
		** an error occured while creating the main window
		*/
		int rv = GetLastError();

		if (rv != 0)
		{
			/*
			** system error has occured
			** this check is neccessary because
			**
			** GetLastError() will return 0 if WM_CREATE has returned -1 (even though CreateWindowEx will have returned NULL)
			*/

			// GetLastErrorInfo(rv);
		}

		return NULL;
	}

	/*
	** create the window
	** WS_EX_LAYERED flag for extended styles is crucial here
	*/
	return CreateWindowEx(WS_EX_LAYERED, c_szSplashClass, NULL, WS_POPUP | WS_VISIBLE,
		0, 0, 0, 0, hwndOwner, NULL, g_hInstance, NULL);
} // CreateSplashWindow

  /*
  ** Calls UpdateLayeredWindow to set a bitmap (with an alpha channel) as the content of the splash window
  */
void CSplash::SetSplashImage(
	HWND hwndSplash,		// splash window handle
	HBITMAP hbmpSplash		// ARGB splash bitmap
)
{
	// get the size of the bitmap
	BITMAP bm;
	GetObject(hbmpSplash, sizeof(bm), &bm);
	SIZE sizeSplash = { bm.bmWidth, bm.bmHeight };

	// position window to determine which monitor to use
	SetWindowPos(hwndSplash, NULL, 0, 0, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

	// get the monitor's info
	HMONITOR hmon = MonitorFromWindow(hwndSplash, MONITOR_DEFAULTTONEAREST);
	MONITORINFO monitorinfo = { 0 };
	monitorinfo.cbSize = sizeof(monitorinfo);
	GetMonitorInfo(hmon, &monitorinfo);

	// center the splash screen in the middle of the primary work area
	const RECT & rcWork = monitorinfo.rcWork;
	POINT ptOrigin;
	ptOrigin.x = rcWork.left + (rcWork.right - rcWork.left - sizeSplash.cx) / 2;
	ptOrigin.y = rcWork.top + (rcWork.bottom - rcWork.top - sizeSplash.cy) / 2;

	// create a memory DC holding the splash bitmap
	HDC hdcScreen = GetDC(NULL);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hbmpOld = (HBITMAP)SelectObject(hdcMem, hbmpSplash);

	// use the source image's alpha channel for blending
	BLENDFUNCTION blend = { 0 };
	blend.BlendOp = AC_SRC_OVER;
	blend.SourceConstantAlpha = 255;
	blend.AlphaFormat = AC_SRC_ALPHA;

	// paint the window (in the right location) with the alpha-blended bitmap
	POINT ptZero = { 0 };
	UpdateLayeredWindow(hwndSplash, hdcScreen, &ptOrigin, &sizeSplash,
		hdcMem, &ptZero, RGB(0, 0, 0), &blend, ULW_ALPHA);

	// delete temporary objects
	SelectObject(hdcMem, hbmpOld);
	DeleteDC(hdcMem);
	ReleaseDC(NULL, hdcScreen);
} // SetSplashImage

bool CSplash::LoadSplash(
	HMODULE hResModule,
	int IDI_SPLASHICON,
	int IDP_SPLASH,
	std::basic_string<TCHAR> &sErr
)
{
	if (!m_hwndSplash)		// failsafe
	{
		CImageConv imgcv;

		m_hwndSplash = CreateSplashWindow(IDI_SPLASHICON);
		m_bmpSplash = imgcv.PNGtoARGB(hResModule, IDP_SPLASH, sErr);

		if (m_bmpSplash == NULL)
			return false;	// error

		SetSplashImage(m_hwndSplash, m_bmpSplash);
	}

	return true;
} // LoadSplash

  /*
  ** remove splash screen
  */
void CSplash::RemoveSplash()
{
	if (m_hwndSplash)
	{
		DestroyWindow(m_hwndSplash);
		m_hwndSplash = NULL;
	}
} // RemoveSplash
