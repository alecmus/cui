//
// scaleAdjust.cpp - scale adjustment implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#include "scaleAdjust.h"

void scaleRECT(RECT &rc, double DPIScale)
{
	int iUnscaledW = rc.right - rc.left;
	int iUnscaledH = rc.bottom - rc.top;

	rc.left = int(0.5 + rc.left * DPIScale);
	rc.top = int(0.5 + rc.top * DPIScale);
	rc.right = rc.left + int(0.5 + iUnscaledW * DPIScale);
	rc.bottom = rc.top + int(0.5 + iUnscaledH * DPIScale);
} // scaleRECT

void UNscaleRECT(RECT &rc, double DPIScale)
{
	int iScaledW = rc.right - rc.left;
	int iScaledH = rc.bottom - rc.top;

	rc.left = int(0.5 + rc.left / DPIScale);
	rc.top = int(0.5 + rc.top / DPIScale);
	rc.right = rc.left + int(0.5 + iScaledW / DPIScale);
	rc.bottom = rc.top + int(0.5 + iScaledH / DPIScale);
} // scaleRECT
