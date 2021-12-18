//
// scaleAdjust.h - scale adjustment interface
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

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
