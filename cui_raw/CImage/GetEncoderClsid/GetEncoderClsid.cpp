//
// GetEncoderClsid.cpp - get encoder class ID implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

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
