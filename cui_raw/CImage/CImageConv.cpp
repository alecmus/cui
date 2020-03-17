/*
** CImageConv.cpp - image conversion implementation
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

#include <tchar.h>
#include <wincodec.h>
#pragma comment (lib, "Windowscodecs.lib")

#include "../CImage/GetEncoderClsid/GetEncoderClsid.h"
#include "../Error/Error.h"
#include "CImage.h"

#include <comdef.h>	// for _com_error

/*
** ensure the use of a given extension for full path specified
** forces database to be saved with the ".extension" extension regardless of what the user specified
** whatever existing extension will be removed
** formatted path will be written back to sFullPath
*/
void FormatToExt(std::basic_string<TCHAR> &sFullPath, const std::basic_string<TCHAR> &extension)
{
	std::basic_string<TCHAR> ext;

	ext = extension;

	// Remove extension if present
	while (true)
	{
		const size_t period_idx1 = sFullPath.rfind('.');

		if (std::basic_string<TCHAR>::npos != period_idx1)
			sFullPath.erase(period_idx1);
		else
			break;
	}

	// remove dot(s) from supplied extension (if necessary)
	const size_t period_idx2 = ext.rfind('.');

	if (std::basic_string<TCHAR>::npos != period_idx2)
		ext = ext.substr(period_idx2 + 1);

	// add extension to path
	if (!ext.empty())
		sFullPath = sFullPath + _T(".") + ext;
} // FormatToExt

  /*
  ** Creates a stream object initialized with the data from an executable resource
  */
IStream * CreateStreamOnResource(
	HMODULE hModule,
	LPCTSTR lpName,
	LPCTSTR lpType,
	std::basic_string<TCHAR> &sErr
)
{
	// initialize return value
	IStream * ipStream = NULL;

	DWORD dwResourceSize = NULL;
	HGLOBAL hglbImage = NULL;
	LPVOID pvSourceResourceData = NULL;
	HGLOBAL hgblResourceData = NULL;
	LPVOID pvResourceData = NULL;

	// find the resource
	HRSRC hrsrc = FindResource(hModule, lpName, lpType);
	if (hrsrc == NULL)
	{
		DWORD dwLastError = GetLastError();
		sErr = GetLastErrorInfo(dwLastError);
		goto Return;
	}
	// load the resource
	dwResourceSize = SizeofResource(hModule, hrsrc);
	hglbImage = LoadResource(hModule, hrsrc);
	if (hglbImage == NULL)
		goto Return;

	// lock the resource, getting a pointer to its data
	pvSourceResourceData = LockResource(hglbImage);
	if (pvSourceResourceData == NULL)
		goto Return;

	// allocate memory to hold the resource data
	hgblResourceData = GlobalAlloc(GMEM_MOVEABLE, dwResourceSize);
	if (hgblResourceData == NULL)
		goto Return;

	// get a pointer to the allocated memory
	pvResourceData = GlobalLock(hgblResourceData);
	if (pvResourceData == NULL)
		goto FreeData;

	// copy the data from the resource to the new memory block
	CopyMemory(pvResourceData, pvSourceResourceData, dwResourceSize);
	GlobalUnlock(hgblResourceData);

	// create a stream on the HGLOBAL containing the data
	if (SUCCEEDED(CreateStreamOnHGlobal(hgblResourceData, TRUE, &ipStream)))
		goto Return;

FreeData:
	// couldn't create stream; free the memory
	GlobalFree(hgblResourceData);

Return:
	// no need to unlock or free the resource
	return ipStream;
} // CreateStreamOnResource

  /*
  ** Loads a PNG image from the specified stream (using Windows Imaging Component)
  */
IWICBitmapSource * LoadBitmapFromStream(
	IStream * ipImageStream,
	std::basic_string<TCHAR> &sErr
)
{
	// initialize return value
	IWICBitmapSource * ipBitmap = NULL;

	// load WIC's PNG decoder
	IWICBitmapDecoder * ipDecoder = NULL;

	UINT nFrameCount = 0;

	IWICBitmapFrameDecode * ipFrame = NULL;

	/*
	** using CLSID_WICPngDecoder1 instead of CLSID_WICPngDecoder
	** because the latter returns "class not registered" in Win7
	** with build from vs2017
	** Idea gotten from https://stackoverflow.com/questions/11985999/regdb-e-classnotreg-on-clsid-wicpngdecoder
	**
	** This is all because of what's explained in https://docs.microsoft.com/en-us/windows/desktop/wic/what-s-new-in-wic-for-windows-8-1
	** Here's an extract from the link:
	** CLSID_WICPngDecoder1 has been added with the same GUID as CLSID_WICPngDecoder,
	** and CLSID_WICPngDecoder2 has been added. When compiled against the Windows 8 SDK,
	** CLSID_WICPngDecoder is #defined to CLSID_WICPngDecoder2 to promote newly compiled
	** apps using the new PNG decoder behavior. Apps should continue to specify CLSID_WICPngDecoder.
	**
	** An app can specify CLSID_WICPngDecoder1
	** to create a version of the WIC PNG decoder that does not generate an IWICColorContext
	** from the gAMA and cHRM chunks. This matches the behavior of the PNG decoder in previous versions of Windows.
	**
	** Before now, we've been using XP support which was causing CLSID_WICPngDecoder to be defined as CLSID_WICPngDecoder1
	** Now it was being defined as CLSID_WICPngDecoder2 hence breaking things in Windows 7
	*/
	HRESULT hRes = CoCreateInstance(CLSID_WICPngDecoder1, NULL, CLSCTX_INPROC_SERVER, __uuidof(ipDecoder), reinterpret_cast<void**>(&ipDecoder));

	if (FAILED(hRes))
	{
		_com_error err(hRes);
		sErr = err.ErrorMessage();
		goto Return;
	}

	// load the PNG

	hRes = ipDecoder->Initialize(ipImageStream, WICDecodeMetadataCacheOnLoad);

	if (FAILED(hRes))
	{
		_com_error err(hRes);
		sErr = err.ErrorMessage();
		goto ReleaseDecoder;
	}

	// check for the presence of the first frame in the bitmap
	hRes = ipDecoder->GetFrameCount(&nFrameCount);

	if (FAILED(hRes) || nFrameCount != 1)
	{
		_com_error err(hRes);
		sErr = err.ErrorMessage();
		goto ReleaseDecoder;
	}

	// load the first frame (i.e., the image)
	hRes = ipDecoder->GetFrame(0, &ipFrame);

	if (FAILED(hRes))
	{
		_com_error err(hRes);
		sErr = err.ErrorMessage();
		goto ReleaseDecoder;
	}

	// convert the image to 32bpp BGRA format with pre-multiplied alpha
	//   (it may not be stored in that format natively in the PNG resource,
	//   but we need this format to create the DIB to use on-screen)
	hRes = WICConvertBitmapSource(GUID_WICPixelFormat32bppPBGRA, ipFrame, &ipBitmap);
	ipFrame->Release();

	if (FAILED(hRes))
	{
		_com_error err(hRes);
		sErr = err.ErrorMessage();
	}

ReleaseDecoder:
	ipDecoder->Release();
Return:
	return ipBitmap;
} // LoadBitmapFromStream

  /*
  ** Creates a 32-bit DIB from the specified WIC bitmap
  */
HBITMAP CreateHBITMAP(
	IWICBitmapSource * ipBitmap
)
{
	// initialize return value
	HBITMAP hbmp = NULL;

	void * pvImageBits = NULL;

	// get image attributes and check for valid image
	UINT width = 0;
	UINT height = 0;
	if (FAILED(ipBitmap->GetSize(&width, &height)) || width == 0 || height == 0)
		goto Return;

	// prepare structure giving bitmap information (negative height indicates a top-down DIB)
	BITMAPINFO bminfo;
	ZeroMemory(&bminfo, sizeof(bminfo));
	bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bminfo.bmiHeader.biWidth = width;
	bminfo.bmiHeader.biHeight = -((LONG)height);
	bminfo.bmiHeader.biPlanes = 1;
	bminfo.bmiHeader.biBitCount = 32;
	bminfo.bmiHeader.biCompression = BI_RGB;

	// create a DIB section that can hold the image
	{
		HDC hdcScreen = GetDC(NULL);
		hbmp = CreateDIBSection(hdcScreen, &bminfo, DIB_RGB_COLORS, &pvImageBits, NULL, 0);
		ReleaseDC(NULL, hdcScreen);
	}

	if (hbmp == NULL)
		goto Return;

	// extract the image into the HBITMAP
	{
		const UINT cbStride = width * 4;
		const UINT cbImage = cbStride * height;
		if (FAILED(ipBitmap->CopyPixels(NULL, cbStride, cbImage, static_cast<BYTE *>(pvImageBits))))
		{
			// couldn't extract image; delete HBITMAP
			DeleteObject(hbmp);
			hbmp = NULL;
		}
	}

Return:
	return hbmp;
} // CreateHBITMAP

CImageConv::CImageConv() { }

CImageConv::~CImageConv() {}

/*
** convert PNG resource to ARGB bitmap (bitmap with alpha channel)
*/
HBITMAP CImageConv::PNGtoARGB(
	HMODULE hModule,
	int ID,		// ID of PNG resource
	std::basic_string<TCHAR> &sErr	// error information
)
{
	HBITMAP hbmp = NULL;

	// load the PNG image data into a stream
	IStream * ipImageStream = CreateStreamOnResource(hModule, MAKEINTRESOURCE(ID), _T("PNG"), sErr);

	if (ipImageStream == NULL)
		goto Return;

	// load the bitmap with WIC
	{
		IWICBitmapSource * ipBitmap = LoadBitmapFromStream(ipImageStream, sErr);

		if (ipBitmap == NULL)
			goto ReleaseStream;

		// create a HBITMAP containing the image
		hbmp = CreateHBITMAP(ipBitmap);
		ipBitmap->Release();
	}

ReleaseStream:
	ipImageStream->Release();

Return:
	return hbmp;
} // PNGtoARGB

bool CImageConv::HBITMAPtoFILE(
	HBITMAP hbmp,			// HBITMAP
	std::basic_string<TCHAR> &sFileName,		// filename to save to
	imageformat format,		// image format
	std::basic_string<TCHAR> &sErr				// error information
)
{
	bool bRes = true;

	if (sFileName.empty())
	{
		sErr = _T("File name not specified.");
		return false;
	}

	/*
	** create GDI+ bitmap from the HBITMAP
	*/
	Gdiplus::Bitmap *Gdibitmap;

	Gdibitmap = Gdiplus::Bitmap::FromHBITMAP(hbmp, NULL);

	if (Gdibitmap)
	{
		/*
		** save resized bitmap to file
		*/
		CLSID encId;
		std::basic_string<TCHAR> mimetype;

		switch (format)
		{
		case CImageConv::PNG:
			mimetype = _T("image/png");
			FormatToExt(sFileName, _T("png"));
			break;

		case CImageConv::BMP:
			mimetype = _T("image/bmp");
			FormatToExt(sFileName, _T("bmp"));
			break;

		case CImageConv::JPEG:
			mimetype = _T("image/jpeg");
			FormatToExt(sFileName, _T("jpg"));
			break;

		case CImageConv::NONE:
			mimetype = _T("image/png");
			FormatToExt(sFileName, _T(""));
			break;

		default:
			mimetype = _T("image/png");
			FormatToExt(sFileName, _T("png"));
			break;
		}

		if (GetEncoderClsid(mimetype, encId) > -1)
		{
			Gdibitmap->Save(sFileName.c_str(), &encId);

			/*
			** get status information
			*/
			Gdiplus::Status status = Gdibitmap->GetLastStatus();

			/*
			** check if an error occured
			*/
			if (status != 0)
			{
				sErr.assign(GetGdiplusStatusInfo(&status));
				bRes = false;	// error
			}
			else
				bRes = true;
		}
	}

	/*
	** delete bitmap from memory
	*/
	if (Gdibitmap)
	{
		delete Gdibitmap;
		Gdibitmap = NULL;
	}

	return bRes;
} // HBITMAPtoFILE

bool CImageConv::GDIPLUSBITMAPtoFILE(
	Gdiplus::Bitmap *Gdibitmap_in,		// Gdiplus bitmap
	std::basic_string<TCHAR> &sFileName,	// filename to save to
	imageformat format,						// image format
	SIZE &maxSize,
	std::basic_string<TCHAR> &sErr			// error information
)
{
	bool bRes = true;

	if (sFileName.empty())
	{
		sErr = _T("File name not specified.");
		return false;
	}

	bool bResized = false;
	Gdiplus::Bitmap * Gdibitmap = NULL;

	if (maxSize.cx <= 0 || maxSize.cy <= 0)
	{
		bResized = false;
		Gdibitmap = Gdibitmap_in;
	}
	else
	{
		RECT rectOut;
		rectOut.left = 0;
		rectOut.right = 0;
		rectOut.top = 0;
		rectOut.bottom = 0;

		// determine target rect
		RECT rectTarget;
		rectTarget.left = 0;
		rectTarget.right = 0;
		rectTarget.top = 0;
		rectTarget.bottom = 0;

		rectTarget.right = (LONG)maxSize.cx;
		rectTarget.bottom = (LONG)maxSize.cy;

		// resize bitmap to the specification set in maxSize
		bResized = true;
		Gdibitmap = ResizeGdiplusBitmap(Gdibitmap_in, rectTarget, false, Quality::high, false, true, rectOut);

		// write back actual size
		maxSize.cx = rectOut.right - rectOut.left;
		maxSize.cy = rectOut.bottom - rectOut.top;
	}

	if (Gdibitmap)
	{
		// save resized bitmap to file
		CLSID encId;
		std::basic_string<TCHAR> mimetype;

		switch (format)
		{
		case CImageConv::PNG:
			mimetype = _T("image/png");
			FormatToExt(sFileName, _T("png"));
			break;

		case CImageConv::BMP:
			mimetype = _T("image/bmp");
			FormatToExt(sFileName, _T("bmp"));
			break;

		case CImageConv::JPEG:
			mimetype = _T("image/jpeg");
			FormatToExt(sFileName, _T("jpg"));
			break;

		case CImageConv::NONE:
			mimetype = _T("image/png");
			FormatToExt(sFileName, _T(""));
			break;

		default:
			mimetype = _T("image/png");
			FormatToExt(sFileName, _T("png"));
			break;
		}

		if (GetEncoderClsid(mimetype, encId) > -1)
		{
			switch (format)
			{
			case CImageConv::PNG:
			{
				// save resized bitmap to file
				Gdiplus::Status status = Gdibitmap->Save(sFileName.c_str(), &encId);

				if (status != Gdiplus::Status::Ok)
				{
					sErr.assign(GetGdiplusStatusInfo(&status));
					bRes = false;	// error
				}
				else
					bRes = true;
			}
			break;

			case CImageConv::BMP:
			case CImageConv::JPEG:
			case CImageConv::NONE:
			default:
			{
				// clear background in case source is in PNG and has an alpha channel
				Gdiplus::Bitmap* bmp_out = new Gdiplus::Bitmap(Gdibitmap->GetWidth(), Gdibitmap->GetHeight(), Gdibitmap->GetPixelFormat());
				Gdiplus::Graphics graphics(bmp_out);
				graphics.Clear(Gdiplus::Color::White);
				graphics.DrawImage(Gdibitmap, 0, 0);

				// save resized bitmap to file
				Gdiplus::Status status = bmp_out->Save(sFileName.c_str(), &encId);

				if (status != Gdiplus::Status::Ok)
				{
					sErr.assign(GetGdiplusStatusInfo(&status));
					bRes = false;	// error
				}
				else
					bRes = true;

				delete bmp_out;
			}
			break;
			}
		}

		if (bResized)
			delete Gdibitmap;
	}

	return bRes;
} // GDIPLUSBITMAPtoFILE
