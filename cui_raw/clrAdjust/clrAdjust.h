/*
** clrAdjust.h - color adjustment interface
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
