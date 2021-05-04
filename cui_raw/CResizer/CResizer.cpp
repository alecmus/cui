/*
** CResizer.cpp - resizer implementation
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

#include <vector>
#include <map>
#include "CResizer.h"
#include "../CDeferPos/CDeferPos.h"

class CResizer::CResizerImpl
{
public:
	bool m_bEnabled;

	struct windowinfo
	{
		HWND hWnd;				// window handle
		RECT rectInit;			// initial window coordinates
		RECT rectParentInit;	// initial window coordinates of parent
		int iPercH;				// rate of following bottom right horizontally
		int iPercV;				// rate of following bottom right vertically
		int iPercCX;			// rate of following parent's width changes
		int iPercCY;			// rate of following parent's height changes
	};

	HWND m_hWndParent;

	static std::map<HWND, LONG_PTR> hWnd_map;
	static std::map<HWND, CResizer *> object_map;

	LONG_PTR m_OriParentProc;	// Original WndProc of parent window
	static LRESULT CALLBACK NewParentProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	std::vector<windowinfo> m_vFollowBR;
	static void OnResize(CDeferPos *pDef, windowinfo *pControlinfo);
};

std::map<HWND, LONG_PTR> CResizer::CResizerImpl::hWnd_map;
std::map<HWND, CResizer *> CResizer::CResizerImpl::object_map;

CResizer::CResizer()
{
	d = new CResizerImpl();
	d->m_hWndParent = NULL;
	d->m_bEnabled = false;
}

CResizer::~CResizer()
{
	if (d->hWnd_map.size() && d->m_hWndParent)
	{
		SetWindowLongPtr(d->m_hWndParent, GWLP_WNDPROC, d->m_OriParentProc);	// reset main window procedure to original
		d->m_hWndParent = NULL;
	}

	if (d)
	{
		delete d;
		d = NULL;
	}
}

/*
** follow bottom right corner of m_hWndParent
** when m_hWndParent is resized
*/
void CResizer::OnResize(
	HWND hWndCtrl,
	int iPercH,
	int iPercV,
	int iPercCX,
	int iPercCY
)
{
	CResizerImpl::windowinfo m_wininfo;

	// capture control window handle
	m_wininfo.hWnd = hWndCtrl;
	m_wininfo.iPercH = iPercH;
	m_wininfo.iPercV = iPercV;
	m_wininfo.iPercCX = iPercCX;
	m_wininfo.iPercCY = iPercCY;

	// capture control's initial coordinates
	GetWindowRect(hWndCtrl, &m_wininfo.rectInit);

	// capture control parent's initial coordinates
	GetWindowRect(GetParent(hWndCtrl), &m_wininfo.rectParentInit);

	// add control window info to vector
	d->m_vFollowBR.push_back(m_wininfo);
} // FollowBR

  /*
  ** to be called AFTER all required resizing
  ** members have been called
  ** this function will enable the actual mechanism
  */
void CResizer::enable(
	HWND hWndParent
)
{
	if (d->m_bEnabled)	// fail-safe to prevent duplication
		return;

	d->m_bEnabled = true;

	// capture parent window handle
	d->m_hWndParent = hWndParent;

	// subclass parent so we can handle the resize message
	d->m_OriParentProc = GetWindowLongPtr(hWndParent, GWLP_WNDPROC);	// save original window procedure of parent window
	d->hWnd_map[hWndParent] = d->m_OriParentProc;					// add window to map
	d->object_map[hWndParent] = this;								// add this object to map
	SetWindowLongPtr(hWndParent, GWLP_WNDPROC, (LONG_PTR)d->NewParentProc);	// replace the window procedure so we can "steal" messages
} // enable

  /*
  ** follow bottom right corner
  */
void CResizer::CResizerImpl::OnResize(CDeferPos *pDef, windowinfo *pControlinfo)
{
	// capture new parent dimensions
	RECT rect;
	GetWindowRect(GetParent(pControlinfo->hWnd), &rect);
	int m_newparent_w = rect.right - rect.left;
	int m_newparent_h = rect.bottom - rect.top;

	int iWidthChange = m_newparent_w - (pControlinfo->rectParentInit.right - pControlinfo->rectParentInit.left);
	int iHeightChange = m_newparent_h - (pControlinfo->rectParentInit.bottom - pControlinfo->rectParentInit.top);

	int ixChange = rect.left - pControlinfo->rectParentInit.left;
	int iyChange = rect.top - pControlinfo->rectParentInit.top;

	RECT m_rectInit = pControlinfo->rectInit;
	POINT pt = { 0, 0 };
	ScreenToClient(GetParent(pControlinfo->hWnd), &pt);	// change reference of coordinates to client area
	int x_init = m_rectInit.left + pt.x;
	int y_init = m_rectInit.top + pt.y;

	// TO - DO: fix this for moving things within a tab control
	pDef->SetWindowPos(pControlinfo->hWnd, 0,
		x_init + ixChange + (iWidthChange * pControlinfo->iPercH / 100),
		y_init + iyChange + (iHeightChange * pControlinfo->iPercV / 100),
		(pControlinfo->rectInit.right - pControlinfo->rectInit.left) + (iWidthChange * pControlinfo->iPercCX / 100),
		(pControlinfo->rectInit.bottom - pControlinfo->rectInit.top) + (iHeightChange * pControlinfo->iPercCY / 100),
		SWP_NOZORDER);

	return;
} // FollowBR

  // parent window procedure
LRESULT CALLBACK CResizer::CResizerImpl::NewParentProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	WNDPROC pDefProc = (WNDPROC)hWnd_map[hWnd];
	CResizer *pThis = object_map[hWnd];

	switch (uMsg)
	{
	case WM_SIZE:
	{
		int iWindows = (int)pThis->d->m_vFollowBR.size();

		CDeferPos def(iWindows);

		for (int i = 0; i < iWindows; i++)
			OnResize(&def, &pThis->d->m_vFollowBR[i]);
	}
	break;

	default:
		break;
	}

	// Call the default(original) window procedure for other messages or messages processed but not returned
	return pDefProc(hWnd, uMsg, wParam, lParam);
} // NewProc
