//
// CBrush.h - brush helper interface
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

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
