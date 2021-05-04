/*
** CDeferPos.h - defer positioning of multiple windows so it's done at once - interface
**
** cui framework
** Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
**
*******************************************************************************
** This file is part of the liblec library which is released under the Creative
** Commons Attribution Non-Commercial 2.0 license (CC-BY-NC 2.0). See the file
** LICENSE.txt or go to https://github.com/alecmus/liblec/blob/master/LICENSE.md
** for full license details.
*************************************************************************
** adapted from http://www.codeproject.com/Articles/155/Using-the-DeferWindowPos-APIs
*/

#pragma once

#include <Windows.h>

/*
** This class wraps the BeginDeferWindowPos/DeferWindowPos/EndDeferWindowPos
** APIs using a "resource allocation is acquisition" idiom.
** Declare a CDeferPos object initialized with as many windows as you intend to move,
** then call the MoveWindow() and SetWindowPos() members a matching number of times on the windows.
** The positioning is done in a sweep when the object goes out of scope
*/
class CDeferPos
{
public:
	CDeferPos(int nWindows = 1);
	~CDeferPos();

	BOOL MoveWindow(HWND hWnd, int x, int y, int nWidth, int nHeight, BOOL bRepaint);
	BOOL SetWindowPos(HWND hWnd, HWND hWndAfter, int x, int y, int nWidth,
		int nHeight, UINT uFlags);

private:
	HDWP m_hdwp;
}; // CDeferPos
