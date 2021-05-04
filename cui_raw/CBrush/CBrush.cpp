/*
** CBrush.cpp - brush helper implementation file
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
