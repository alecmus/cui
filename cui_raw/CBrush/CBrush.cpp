//
// CBrush.cpp - brush helper implementation file
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#include "CBrush.h"
#include <windowsx.h>

CBrush::CBrush(COLORREF clr)
{
	m_hbr = CreateSolidBrush(clr);
}

CBrush::~CBrush()
{
	if (m_hbr)
		DeleteBrush(m_hbr);

	m_hbr = NULL;
}

HBRUSH CBrush::get()
{
	return m_hbr;
}
