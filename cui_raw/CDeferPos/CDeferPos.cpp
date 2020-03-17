/*
** CDeferPos.cpp - defer positioning of multiple windows so it's done at once - implementation
**
** cui framework
** Copyright (c) 2016 Alec T. Musasa (alecmus at live dot com)
**
*******************************************************************************
** This file is part of the liblec library which is released under the Creative
** Commons Attribution Non-Commercial 2.0 license (CC-BY-NC 2.0). See the file
** LICENSE.txt or go to https://github.com/alecmus/liblec/blob/master/LICENSE.md
** for full license details.
*************************************************************************
** adapted from http://www.codeproject.com/Articles/155/Using-the-DeferWindowPos-APIs
*/

#include "CDeferPos.h"

#define ENABLE	1	// toggle defer

// Constructor
// This sets up the RAIA idiom by calling BeginDeferWindowPos. The number of windows
// can be passed as an argument to optimize memory management, although the API will
// grow the memory if needed at run time.

CDeferPos::CDeferPos(int nWindows)
{
#if ENABLE
	m_hdwp = BeginDeferWindowPos(nWindows);
#endif // ENABLE
}

// Destructor
// This concludes the RAIA idiom by ensuring EndDeferWindowPos is called.

CDeferPos::~CDeferPos()
{
#if ENABLE
	EndDeferWindowPos(m_hdwp);
#endif // ENABLE
}

// MoveWindow
// Emulates a call to ::MoveWindow but the actual call is delayed until
// the CDeferPos object is destroyed.  All delayed window positions are
// then done "at once", which can reduce flicker.

BOOL CDeferPos::MoveWindow(HWND hWnd, int x, int y, int nWidth, int nHeight,
	BOOL bRepaint)
{
#if ENABLE
	UINT uFlags = SWP_NOACTIVATE | SWP_NOOWNERZORDER | SWP_NOZORDER;
	if (!bRepaint)
		uFlags |= SWP_NOREDRAW;
	return SetWindowPos(hWnd, 0, x, y, nWidth, nHeight, uFlags);
#else
	return ::MoveWindow(hWnd, x, y, nWidth, nHeight, bRepaint);
#endif // ENABLE
}

// SetWindowPos
// Emulates a call to ::SetWindowPos but the actual call is delayed until
// the CDeferPos object is destroyed.  All delayed window positions are
// then done "at once", which can reduce flicker.

BOOL CDeferPos::SetWindowPos(HWND hWnd, HWND hWndAfter, int x, int y, int nWidth,
	int nHeight, UINT uFlags)
{
#if ENABLE
	if (m_hdwp != 0)
	{
		m_hdwp = DeferWindowPos(m_hdwp, hWnd, hWndAfter, x, y, nWidth, nHeight,
			uFlags);
	}
	return m_hdwp != 0;
#else
	return ::SetWindowPos(hWnd, hWndAfter, x, y, nWidth, nHeight, uFlags);
#endif // ENABLE
}
