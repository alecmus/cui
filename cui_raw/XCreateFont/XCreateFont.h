/*
** XCreateFont.h - font creation, in points - interface
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

#pragma once

#include <Windows.h>
#include <tchar.h>
#include <string>

// Call DeleteFont() when done with the font and set to NULL
HFONT XCreateFont(HDC m_hdc,
	std::basic_string<TCHAR> sFontName,
	double in_iPointSize);
