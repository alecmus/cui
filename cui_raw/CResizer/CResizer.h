//
// CResizer.h - resizer interface
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#pragma once

#include <Windows.h>

/*
** CResizer - resizing handling class
*/
class CResizer
{
public:
	CResizer();
	~CResizer();

	/// <summary>Follow changes to the parent's resizing.</summary>
	/// <param name="hWndCtrl">The handle of the control.</param>
	/// <param name="iPercH">The rate at which the control's left border follows the parent's right border.</param>
	/// <param name="iPercV">The rate at which the control's top border follow the parent's bottom border.</param>
	/// <param name="iPercCX">The rate at which this control's width follows that of the parent.</param>
	/// <param name="iPercCY">The rate at which this control's height follows that of the parent.</param>
	/// <return>No return value.</return>
	void OnResize(
		HWND hWndCtrl,
		int iPercH,
		int iPercV,
		int iPercCX,
		int iPercCY
	);

	/*
	** to be called AFTER all required resizing
	** members have been called
	** this function will enable the actual mechanism
	*/
	void enable(
		HWND hWndParent
	);

private:
	class CResizerImpl;
	CResizerImpl *d;
}; // CResizer
