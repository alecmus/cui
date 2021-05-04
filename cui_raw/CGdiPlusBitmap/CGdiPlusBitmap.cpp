/*
** CGdiPlusBitmap.cpp - GDI+ bitmap helper class implementation
**
** cui framework
** Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
**
*******************************************************************************
** This file is part of the liblec library which is released under the Creative
** Commons Attribution Non-Commercial 2.0 license (CC-BY-NC 2.0). See the file
** LICENSE.txt or go to https://github.com/alecmus/liblec/blob/master/LICENSE.md
** for full license details.
*************************************************************************
** based on code from http://www.codeproject.com/Articles/3537/Loading-JPG-PNG-resources-using-GDI
*/

#pragma once

#include "CGdiPlusBitmap.h"
#include "../Error/Error.h"

#include <comdef.h>	// for _com_error

CGdiPlusBitmap::CGdiPlusBitmap()
{
	m_pBitmap = NULL;
}

CGdiPlusBitmap::~CGdiPlusBitmap()
{
	Empty();
}

void CGdiPlusBitmap::Empty()
{
	if (m_pBitmap)
	{
		delete m_pBitmap;
		m_pBitmap = NULL;
	}
} // Empty

bool CGdiPlusBitmap::Load(LPCWSTR pFile, std::basic_string<TCHAR> &sErr)
{
	Empty();
	m_pBitmap = Gdiplus::Bitmap::FromFile(pFile);

	Gdiplus::Status status = m_pBitmap->GetLastStatus();

	if (status != Gdiplus::Ok)
	{
		sErr.assign(GetGdiplusStatusInfo(&status));
		Empty();
		return false;
	}

	return true;
} // Load

bool CGdiPlusBitmap::Load(HBITMAP bmp, std::basic_string<TCHAR> &sErr)
{
	Empty();
	m_pBitmap = Gdiplus::Bitmap::FromHBITMAP(bmp, NULL);

	Gdiplus::Status status = m_pBitmap->GetLastStatus();

	if (status != Gdiplus::Ok)
	{
		sErr.assign(GetGdiplusStatusInfo(&status));
		Empty();
		return false;
	}

	return true;
} // Load

CGdiPlusBitmapResource::CGdiPlusBitmapResource()
{
	m_hBuffer = NULL;
}

CGdiPlusBitmapResource::~CGdiPlusBitmapResource()
{
	Empty();
}

bool CGdiPlusBitmapResource::Load(UINT id, LPCTSTR pType, std::basic_string<TCHAR> &sErr, HMODULE hInst)
{
	return Load(MAKEINTRESOURCE(id), sErr, pType, hInst);
} // Load

bool CGdiPlusBitmapResource::Load(UINT id, UINT type, std::basic_string<TCHAR> &sErr, HMODULE hInst)
{
	return Load(MAKEINTRESOURCE(id), sErr, MAKEINTRESOURCE(type), hInst);
} // Load

void CGdiPlusBitmapResource::Empty()
{
	CGdiPlusBitmap::Empty();
	if (m_hBuffer)
	{
		::GlobalUnlock(m_hBuffer);
		::GlobalFree(m_hBuffer);
		m_hBuffer = NULL;
	}
} // Empty

bool CGdiPlusBitmapResource::Load(LPCTSTR pName, std::basic_string<TCHAR> &sErr, LPCTSTR pType, HMODULE hInst)
{
	Empty();

	HRSRC hResource = ::FindResource(hInst, pName, pType);

	if (!hResource)
	{
		DWORD dwErr = GetLastError();
		sErr = GetLastErrorInfo(dwErr);
		return false;
	}

	DWORD imageSize = ::SizeofResource(hInst, hResource);

	if (!imageSize)
	{
		DWORD dwErr = GetLastError();
		sErr = GetLastErrorInfo(dwErr);
		return false;
	}

	const void* pResourceData = ::LockResource(::LoadResource(hInst, hResource));

	if (!pResourceData)
	{
		DWORD dwErr = GetLastError();
		sErr = GetLastErrorInfo(dwErr);
		return false;
	}

	m_hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);

	if (m_hBuffer)
	{
		void* pBuffer = ::GlobalLock(m_hBuffer);

		if (pBuffer)
		{
			CopyMemory(pBuffer, pResourceData, imageSize);

			IStream* pStream = NULL;

			HRESULT hRes = CreateStreamOnHGlobal(m_hBuffer, FALSE, &pStream);

			if (hRes == S_OK)
			{
				m_pBitmap = Gdiplus::Bitmap::FromStream(pStream);
				pStream->Release();

				Gdiplus::Status status = m_pBitmap->GetLastStatus();

				if (m_pBitmap)
				{
					if (status == Gdiplus::Ok)
						return true;
					else
						sErr.assign(GetGdiplusStatusInfo(&status));

					delete m_pBitmap;
					m_pBitmap = NULL;
				}
				else
				{
					sErr.assign(GetGdiplusStatusInfo(&status));
				}
			}
			else
			{
				_com_error err(hRes);
				sErr = err.ErrorMessage();
			}

			::GlobalUnlock(m_hBuffer);
		}
		else
		{
			DWORD dwErr = GetLastError();
			sErr = GetLastErrorInfo(dwErr);
		}

		::GlobalFree(m_hBuffer);
		m_hBuffer = NULL;
	}
	else
	{
		DWORD dwErr = GetLastError();
		sErr = GetLastErrorInfo(dwErr);
	}

	return false;
} // Load
