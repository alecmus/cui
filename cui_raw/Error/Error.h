//
// Error.h - error interface
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

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
