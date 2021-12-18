//
// XCreateFont.h - font creation, in points - interface
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#pragma once

#include <Windows.h>
#include <tchar.h>
#include <string>

// Call DeleteFont() when done with the font and set to NULL
HFONT XCreateFont(HDC m_hdc,
	std::basic_string<TCHAR> sFontName,
	double in_iPointSize);
