//
// XCreateFont.h - font creation, in points - implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#include "XCreateFont.h"

// double to int (rounds off instead of truncating)
int double2int(double in)
{
	return (int)(0.5 + in);
}

// font attributes
#define FNT_NORMAL		0
#define FNT_BOLD		1
#define FNT_ITALIC		2
#define FNT_UNDERLINE	4

HFONT XCreateFont(HDC m_hdc,
	std::basic_string<TCHAR> sFontName,
	double in_iPointSize)
{
	int iAttributes = FNT_NORMAL;

	int DPI = GetDeviceCaps(m_hdc, LOGPIXELSY);

	// use negative sign to indicate this is a logical size intended to be compatible with points
	int logPxlY = -double2int(in_iPointSize * static_cast<double>(DPI) / 72.0);

	LOGFONT lf;
	lf.lfHeight = logPxlY;
	lf.lfWidth = 0;
	lf.lfEscapement = 0;
	lf.lfOrientation = 0;
	lf.lfWeight = iAttributes & FNT_BOLD ? 700 : 0;
	lf.lfItalic = iAttributes & FNT_ITALIC ? 1 : 0;
	lf.lfUnderline = iAttributes & FNT_UNDERLINE ? 1 : 0;
	lf.lfStrikeOut = false;
	lf.lfCharSet = DEFAULT_CHARSET;
	lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
	lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
	lf.lfQuality = DEFAULT_QUALITY;
	lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
	lstrcpy(lf.lfFaceName, sFontName.c_str());

	HFONT myfont = CreateFontIndirect(&lf);

	return myfont;
} // XCreateFont
