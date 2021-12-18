//
// CImage.h - image handling - interface
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#pragma once

#include <tchar.h>
#include <string>
#include <Windows.h>
#include <vector>
#include <GdiPlus.h>
#include <sstream>
#include <map>

//class CTimer;

enum Quality
{
	low,
	medium,
	high
};

enum imageformat
{
	PNG = 0,
	BMP,
	JPEG,
};

/*
** resize GDIPlus bitmap
** initialize GDI+ before any calls
*/
Gdiplus::Bitmap * ResizeGdiplusBitmap(
	Gdiplus::Bitmap *pBmpIn,	// source bitmap
	const RECT rectTarget,		// target rect
	bool bStretch,				// stretch bitmap to supplied dimensions
	Quality quality,			// resizing quality
	bool bEnlargeIfSmaller,		// flag to determine whether image is enlarged to (width x height) if it is smaller than that
	bool bCenter,				// center bitmap
	RECT &rectOut				// output rect
);

/*
** CImageConv - for image conversion
*/
class CImageConv
{
public:
	CImageConv();
	~CImageConv();

	/*
	** convert PNG resource to ARGB bitmap (bitmap with alpha channel)
	** CoInitialize must be called before this function is called
	*/
	HBITMAP PNGtoARGB(
		HMODULE hModule,
		int ID,		// ID of PNG resource
		std::basic_string<TCHAR> &sErr	// error information
	);

	enum imageformat
	{
		PNG = 0,
		BMP,
		JPEG,
		NONE,	// no extension is added to the file. Internal format used is PNG.
	};

	/*
	** save HBITMAP to file
	** GDI+ must be initialized before this function if called
	** returns true if successful, else false
	** NOTE: final path is written back to sFileName
	** error information is written to sErr
	*/
	static bool HBITMAPtoFILE(
		HBITMAP hbmp,							// HBITMAP
		std::basic_string<TCHAR> &sFileName,	// filename to save to
		imageformat format,						// image format
		std::basic_string<TCHAR> &sErr			// error information
	);

	/*
	** save Gdiplus bitmap to file
	** GDI+ must be initialized before this function if called
	** returns true if successful, else false
	** NOTE: final path is written back to sFileName
	** error information is written to sErr
	*/
	static bool GDIPLUSBITMAPtoFILE(
		Gdiplus::Bitmap *Gdibitmap,		// Gdiplus bitmap
		std::basic_string<TCHAR> &sFileName,	// filename to save to
		imageformat format,						// image format
		SIZE &maxSize,							// maximum size of the image (0x0 means no size limit) ... image is resized to fit into the specified rectangle without cropping
		std::basic_string<TCHAR> &sErr			// error information
	);

private:

}; // CImageConv

   /*
   ** CSplash - for displaying a splash screen
   */
class CSplash
{
public:
	CSplash();
	~CSplash();

	/*
	** Load splash screen
	** return true if successful, else false
	** error information is written to sErr
	*/
	bool LoadSplash(
		HMODULE hResModule,
		int IDI_SPLASHICON,		// ID of icon
		int IDP_SPLASH,			// ID of splash PNG resource
		std::basic_string<TCHAR> &sErr				// error information
	);

	/*
	** remove splash screen
	*/
	void RemoveSplash();

protected:
	//CTimer *pTimer;

private:
	HWND m_hwndSplash;
	HBITMAP m_bmpSplash;

	/*
	** Registers a window class for the splash and splash owner windows.
	*/
	void RegisterWindowClass(
		int IDI_SPLASHICON		// ID of icon to use for splash screen window
	);

	/*
	** Create the splash owner window and the splash window
	** returns a handle to the splash window
	*/
	HWND CreateSplashWindow(
		int IDI_SPLASHICON		// ID of icon to use for splash screen window
	);

	/*
	** Calls UpdateLayeredWindow to set a bitmap (with an alpha channel) as the content of the splash window
	*/
	void SetSplashImage(
		HWND hwndSplash,		// splash window handle
		HBITMAP hbmpSplash		// ARGB splash bitmap
	);
}; // CSplash
