/*
** scaleAdjust.cpp - scale adjustment implementation
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
