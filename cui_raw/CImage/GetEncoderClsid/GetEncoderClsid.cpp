/*
** GetEncoderClsid.cpp - get encoder class ID implementation
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

#include "GetEncoderClsid.h"

/*
** get encoder class ID
** returns value greater than -1 if successful
** class ID is written to Clsid
*/
int GetEncoderClsid(
	const std::basic_string<TCHAR> &Form,	// form
	CLSID &Clsid		// class ID
)
{
	UINT num;
	UINT size;
	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;
	Gdiplus::GetImageEncodersSize(&num, &size);

	if (size == 0)
		return -1;

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));

	if (pImageCodecInfo == NULL)
		return -1;

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j<num; j++)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, Form.c_str()) == 0)
		{
			Clsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;
		}
	}

	free(pImageCodecInfo);

	return -1;
} // GetEncoderClsid
