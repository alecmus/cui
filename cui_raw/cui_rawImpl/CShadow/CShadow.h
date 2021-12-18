//
// CShadow.cpp - shadow handling interface
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

/*
** CShadow - shadow handling class implementation
** based on CWndShadow version 0.3 by Mingliang Zhu from here:
** https://www.codeproject.com/Articles/16362/Bring-your-frame-window-a-shadow
**
** - will only apply a shadow on a window with the WS_POPUP style
**
*************************************************************************
** Below is the copyright notice that came with the CShadow class
**
** Version 0.3
**
** This article, along with any associated source code and files, is
** licensed under The Microsoft Public License (Ms-PL)
** https://opensource.org/licenses/ms-pl.html
**
*************************************************************************
** Update history--
**
** Version 0.3, 2007-06-14
**    -The shadow is made Windows Vista Aero awareness.
**    -Fixed a bug that causes the shadow to appear abnormally on Windows Vista.
**    -Fixed a bug that causes the shadow to appear abnormally if parent window
**     is initially minimized or maximized
**
** Version 0.2, 2006-11-23
**    -Fix a critical issue that may make the shadow fail to work under certain
**     conditions, e.g., on Win2000, on WinXP or Win2003 without the visual
**     theme enabled, or when the frame window does not have a caption bar.
**
** Version 0.1, 2006-11-10
**    -First release
*/

#pragma once

#include <Windows.h>
#include <CommCtrl.h>
#include <map>
#include <tchar.h>
#include <math.h>
#include <crtdbg.h>
#include "../../CCriticalSection/CCriticalSection.h"

/*
** CShadow - window shadow class
** will only apply a shadow on a window with the WS_POPUP style
*/
class CShadow
{
public:
	CShadow();
	virtual ~CShadow();

	struct shadow_position
	{
		short x = 0;	// min -20, max +20
		short y = 0;	// min -20, max +20
	};

	struct shadow_properties
	{
		COLORREF color = RGB(0, 0, 0);
		short size = 10;				// min -20, max +20
		unsigned short sharpness = 20;	// min 0, max 20
		short darkness = 50;			// min 0, max 255
		shadow_position position;
	};

	/*
	** create shadow on window
	*/
	void CreateShadow(HWND hParent,
		const shadow_properties &properties);

protected:

	// shadow window procedure
	static LRESULT CALLBACK ShadowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	LONG_PTR m_OriParentProc;	// original WndProc of parent window

							/*
							** new procedure for parent window
							*/
	static LRESULT CALLBACK NewProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	/*
	** update shadow
	** to be called when window is resized, or if shadow properties change
	** do not call when simply moving the window without resizing
	*/
	void UpdateShadow(
		HWND hParent	// parent window handle
	);

	/*
	** Fill in the shadow window alpha blend bitmap with shadow image pixels
	*/
	void MakeShadow(
		UINT32 *pShadBits,
		HWND hParent,
		RECT *rcParent
	);

	/*
	** Show or hide the shadow
	** depending on the enabled status stored in m_Status
	*/
	void ShowShadow(
		HWND hParent	// parent window handle
	);

private:

	/*
	** Helper to calculate the alpha-premultiled value for a pixel
	*/
	inline DWORD PreMultiply(
		COLORREF cl,
		unsigned char nAlpha
	)
	{
		// It's strange that the byte order of RGB in 32b BMP is reverse to in COLORREF
		return (GetRValue(cl) * (DWORD)nAlpha / 255) << 16 |
			(GetGValue(cl) * (DWORD)nAlpha / 255) << 8 |
			(GetBValue(cl) * (DWORD)nAlpha / 255);
	} // PreMultiply

	// Parent HWND and CShadow object pares, in order to find CShadow in ParentProc()
	static std::map<HWND, CShadow *> s_Shadowmap;

	// for protecting multithreaded access to the shadow map
	static CCriticalSection m_ShadowLocker;

	static bool s_bVista;	// whether module is running on Windows Vista or above

	enum ShadowStatus
	{
		SS_ENABLED = 1,				// Shadow is enabled, if not, the following one is always false
		SS_VISABLE = 1 << 1,		// Shadow window is visible
		SS_PARENTVISIBLE = 1 << 2,	// Parent window is visible, if not, the above one is always false
		SS_DISABLEDBYAERO = 1 << 3	// Shadow is enabled, but do not show because areo is enabled
	};

	/*
	** The X and Y offsets of shadow window,
	** relative to the parent window, at center of both windows (not top-left corner), signed
	*/
	signed char m_nxOffset;
	signed char m_nyOffset;

	unsigned char m_nDarkness;	// Darkness, transparency of blurred area
	unsigned char m_nSharpness;	// Sharpness, width of blurred border of shadow window
	signed char m_nSize;		// Shadow window size, relative to parent window size
	LPARAM m_WndSize;	// Restore last parent window size, used to determine the update strategy when parent window is resized
	bool m_bUpdate;		// Set this to true if the shadow should not be update until next WM_PAINT is received
	BYTE m_Status;		// status flag
	COLORREF m_Color;	// Color of shadow
	HWND m_hShadow;		// handle of shadow window
}; // CShadow
