//
// DrawRoundRect.h - draw round rectangle interface
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#pragma once

#include <Windows.h>
#include <gdiplus.h>

void DrawRoundRect(Gdiplus::Graphics &graphics,
	int x1, int y1, int x2, int y2, int radius,
	COLORREF color1, COLORREF color2, int iBorderWidth, bool bFill);
