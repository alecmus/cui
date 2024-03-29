//
// CGdiPlusBitmap.h - GDI+ bitmap helper class interface
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

/*
** based on code from http://www.codeproject.com/Articles/3537/Loading-JPG-PNG-resources-using-GDI
*/

#pragma once

#include <Windows.h>
#include <GdiPlus.h>
#include <string>
#include <tchar.h>

class CGdiPlusBitmap
{
public:
	CGdiPlusBitmap();
	virtual ~CGdiPlusBitmap();

	bool Load(
		LPCWSTR pFile,
		std::basic_string<TCHAR> &sErr
	);

	bool Load(
		HBITMAP bmp,
		std::basic_string<TCHAR> &sErr
	);

	operator Gdiplus::Bitmap*() const { return m_pBitmap; }

protected:
	Gdiplus::Bitmap* m_pBitmap;
	void Empty();
}; // CGdiPlusBitmap

class CGdiPlusBitmapResource : public CGdiPlusBitmap
{
public:
	CGdiPlusBitmapResource();
	virtual ~CGdiPlusBitmapResource();

	bool Load(
		UINT id,
		LPCTSTR pType,
		std::basic_string<TCHAR> &sErr,
		HMODULE hInst
	);

protected:
	HGLOBAL m_hBuffer;
	void Empty();

private:
	bool Load(
		LPCTSTR pName,
		std::basic_string<TCHAR> &sErr,
		LPCTSTR pType = RT_RCDATA,
		HMODULE hInst = NULL
	);

	bool Load(
		UINT id,
		UINT type,
		std::basic_string<TCHAR> &sErr,
		HMODULE hInst = NULL
	);
}; // CGdiPlusBitmapResource
