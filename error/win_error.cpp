/*
** win_error.cpp - windows errors implementation
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

#include "win_error.h"
#include <Windows.h>

std::string get_last_error()
{
	std::string last_error;

	CHAR buffer[256] = "?";

	DWORD dw_last_error = GetLastError();

	if (dw_last_error != 0)    // Don't want to see a "operation done successfully" error ;-)
	{
		::FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM,	// It�s a system error
			NULL,										// No string to be formatted needed
			dw_last_error,								// Hey Windows: Please explain this error!
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// Do it in the standard language
			buffer,										// Put the message here
			256 - 1,									// Number of bytes to store the message
			NULL);

		last_error = buffer;
	}

	return last_error;
} // get_last_error
