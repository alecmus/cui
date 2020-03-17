/*
** Error.cpp - error implementation
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

#include "Error.h"

/*
** get detailed windows error information
*/
std::basic_string<TCHAR> GetLastErrorInfo(DWORD dwLastError)
{
	std::basic_string<TCHAR> err;

	TCHAR   lpBuffer[256] = _T("?");

	if (dwLastError != 0)    // Don't want to see a "operation done successfully" error ;-)
	{
		::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,						// It´s a system error
			NULL,										// No string to be formatted needed
			dwLastError,								// Hey Windows: Please explain this error!
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	// Do it in the standard language
			lpBuffer,									// Put the message here
			256 - 1,										// Number of bytes to store the message
			NULL);

		err = lpBuffer;
	}

	return err;
} // GetLastErrorInfo

  /*
  ** get detailed Gdiplus Status information
  */
std::basic_string<TCHAR> GetGdiplusStatusInfo(Gdiplus::Status* pstatus)
{
	using namespace Gdiplus;

	std::basic_string<TCHAR> msg = _T("");

	switch (*pstatus)
	{
	case Ok:
		msg = _T("Ok: Indicates that the method call was successful.");
		break;
	case GenericError:
		msg = _T("GenericError");
		break;
	case InvalidParameter:
		msg = _T("InvalidParameter");
		break;
	case OutOfMemory:
		msg = _T("OutOfMemory");
		break;
	case ObjectBusy:
		msg = _T("ObjectBusy");
		break;
	case InsufficientBuffer:
		msg = _T("InsufficientBuffer");
		break;
	case NotImplemented:
		msg = _T("NotImplemented");
		break;
	case Win32Error:
		msg = _T("Win32Error");
		break;
	case WrongState:
		msg = _T("WrongState");
		break;
	case Aborted:
		msg = _T("Aborted");
		break;
	case FileNotFound:
		msg = _T("FileNotFound");
		break;
	case ValueOverflow:
		msg = _T("ValueOverflow");
		break;
	case AccessDenied:
		msg = _T("AccessDenied");
		break;
	case UnknownImageFormat:
		msg = _T("UnknownImageFormat");
		break;
	case FontFamilyNotFound:
		msg = _T("FontFamilyNotFound");
		break;
	case FontStyleNotFound:
		msg = _T("FontStyleNotFound");
		break;
	case NotTrueTypeFont:
		msg = _T("NotTrueTypeFont");
		break;
	case UnsupportedGdiplusVersion:
		msg = _T("UnsupportedGdiplusVersion");
		break;
	case GdiplusNotInitialized:
		msg = _T("GdiplusNotInitialized");
		break;
	case PropertyNotFound:
		msg = _T("PropertyNotFound");
		break;
	case PropertyNotSupported:
		msg = _T("PropertyNotSupported");
		break;
#if (GDIPVER >= 0x0110)
	case ProfileNotFound:
		msg = _T("ProfileNotFound");
		break;
#endif //(GDIPVER >= 0x0110)
	default:
		msg = _T("Invalid status");
		break;
	}

	return msg;
} // GetGdiplusStatusInfo
