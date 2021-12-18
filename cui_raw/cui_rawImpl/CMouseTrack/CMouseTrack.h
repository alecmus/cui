//
// CMouseTrack.h - mouse tracking implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#pragma once

#include <Windows.h>

class CMouseTrack
{
	bool m_bMouseTracking;

public:
	CMouseTrack() : m_bMouseTracking(false)
	{
	}

	void OnMouseMove(HWND hWnd)
	{
		if (!m_bMouseTracking)
		{
			// Enable mouse tracking.
			TRACKMOUSEEVENT tme;
			tme.cbSize = sizeof(tme);
			tme.hwndTrack = hWnd;
			tme.dwFlags = TME_HOVER | TME_LEAVE;
			tme.dwHoverTime = HOVER_DEFAULT;
			TrackMouseEvent(&tme);
			m_bMouseTracking = true;
		}
	}

	void Reset(HWND hWnd)
	{
		m_bMouseTracking = false;
	}
}; // CMouseTrack
