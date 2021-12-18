//
// clrAdjust.h - color adjustment interface
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#pragma once

#include <Windows.h>

/// <summary>
/// Darken an RGB color.
/// </summary>
/// 
/// <param name="iPerc">
/// The percentage to darken the color.
/// </param>
COLORREF clrDarken(COLORREF clr_in, int iPerc);

/// <summary>
/// Lighten an RGB color.
/// </summary>
/// 
/// <param name="iPerc">
/// The percentage to lighten the color.
/// </param>
COLORREF clrLighten(COLORREF clr_in, int iPerc);
