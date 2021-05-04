/*
** GetEncoderClsid.h - get encoder class ID interface
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
