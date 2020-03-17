/*
** Error.h - error interface
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
#include <tchar.h>
#include <string>
#include <GdiPlus.h>

/*
** get detailed windows error information
*/
std::basic_string<TCHAR> GetLastErrorInfo(DWORD dwLastError);

/*
** get detailed Gdiplus Status information
*/
std::basic_string<TCHAR> GetGdiplusStatusInfo(Gdiplus::Status* pstatus);
