/*
** DrawRoundRect.h - draw round rectangle interface
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
#include <gdiplus.h>

void DrawRoundRect(Gdiplus::Graphics &graphics,
	int x1, int y1, int x2, int y2, int radius,
	COLORREF color1, COLORREF color2, int iBorderWidth, bool bFill);
