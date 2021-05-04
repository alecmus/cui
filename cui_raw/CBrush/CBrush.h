/*
** CBrush.h - brush helper interface
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

#include <Windows.h>

// brush creation helper (automatically destroys brush when it goes out of scope)
class CBrush
{
public:
	CBrush(COLORREF clr);
	~CBrush();

	HBRUSH get();

private:
	HBRUSH m_hbr;
}; // CBrush
