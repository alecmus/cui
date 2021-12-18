//
// GetEncoderClsid.h - get encoder class ID interface
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#pragma once

#include "../CImage.h"

/*
** get encoder class ID
** Form examples: "image/jpeg" or "image/png"
** returns value greater than -1 if successful
** class ID is written to Clsid
*/
int GetEncoderClsid(
	const std::basic_string<TCHAR> &Form,	// form
	CLSID &Clsid		// class ID
);
