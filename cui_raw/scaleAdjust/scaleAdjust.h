/*
** scaleAdjust.h - scale adjustment interface
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

#pragma once

#include <Windows.h>

/// <summary>Adjust a rectangle to a given DPI scale.</summary>
/// <param name="rc">The coordinates of the rectangle under 96dpi.</param>
/// <param name="DPIScale">The ratio of the current dpi to 96dpi.</param>
/// <remarks>DPIScale is 1.0 when the current setting is 96dpi.</remarks>
void scaleRECT(RECT &rc, double DPIScale);

/// <summary>Adjust a rectangle to a scale of 96dpi.</summary>
/// <param name="rc">The coordinates of the rectangle under the current dpi.</param>
/// <param name="DPIScale">The ratio of the current dpi to 96dpi.</param>
/// <remarks>DPIScale is 1.0 when the current setting is 96dpi.</remarks>
void UNscaleRECT(RECT &rc, double DPIScale);
