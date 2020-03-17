/*
** clrAdjust.cpp - color adjustment implementation
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

#include "clrAdjust.h"

COLORREF clrDarken(COLORREF clr_in, int iPerc)
{
	COLORREF clr = clr_in;

	int r_in = (int)GetRValue(clr);
	int g_in = (int)GetGValue(clr);
	int b_in = (int)GetBValue(clr);

	clr = RGB(0, 0, 0);

	int r_target = (int)GetRValue(clr);
	int g_target = (int)GetGValue(clr);
	int b_target = (int)GetBValue(clr);

	int r = int((double)(r_in - r_target) * ((double)100 - (double)iPerc) / (double)100);
	int g = int((double)(g_in - g_target) * ((double)100 - (double)iPerc) / (double)100);
	int b = int((double)(b_in - b_target) * ((double)100 - (double)iPerc) / (double)100);

	return RGB(r, g, b);
} // clrDarken

COLORREF clrLighten(COLORREF clr_in, int iPerc)
{
	COLORREF clr = clr_in;

	int r_in = (int)GetRValue(clr);
	int g_in = (int)GetGValue(clr);
	int b_in = (int)GetBValue(clr);

	clr = RGB(255, 255, 255);

	int r_target = (int)GetRValue(clr);
	int g_target = (int)GetGValue(clr);
	int b_target = (int)GetBValue(clr);

	int r = r_in + int((double)(r_target - r_in) * ((double)iPerc) / (double)100);
	int g = g_in + int((double)(g_target - g_in) * ((double)iPerc) / (double)100);
	int b = b_in + int((double)(b_target - b_in) * ((double)iPerc) / (double)100);

	return RGB(r, g, b);
} // clrLighten
