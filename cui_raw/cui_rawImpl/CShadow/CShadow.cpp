/*
** CShadow.cpp - shadow handling implementation
**
** cui framework
** Copyright (c) 2016 Alec T. Musasa (alecmus at live dot com)
**
*******************************************************************************
** This file is part of the liblec library which is released under the Creative
** Commons Attribution Non-Commercial 2.0 license (CC-BY-NC 2.0). See the file
** LICENSE.txt or go to https://github.com/alecmus/liblec/blob/master/LICENSE.md
** for full license details.
*************************************************************************
** CShadow - shadow handling class implementation
** based on CWndShadow version 0.3 by Mingliang Zhu from here:
** https://www.codeproject.com/Articles/16362/Bring-your-frame-window-a-shadow
**
** - will only apply a shadow on a window with the WS_POPUP style
**
*************************************************************************
** Below is the copyright notice that came with the CShadow class
**
** Version 0.3
**
** This article, along with any associated source code and files, is
** licensed under The Microsoft Public License (Ms-PL)
** https://opensource.org/licenses/ms-pl.html
**
*************************************************************************
** Update history--
**
** Version 0.3, 2007-06-14
**    -The shadow is made Windows Vista Aero awareness.
**    -Fixed a bug that causes the shadow to appear abnormally on Windows Vista.
**    -Fixed a bug that causes the shadow to appear abnormally if parent window
**     is initially minimized or maximized
**
** Version 0.2, 2006-11-23
**    -Fix a critical issue that may make the shadow fail to work under certain
**     conditions, e.g., on Win2000, on WinXP or Win2003 without the visual
**     theme enabled, or when the frame window does not have a caption bar.
**
** Version 0.1, 2006-11-10
**    -First release
*/

#include "CShadow.h"

#include <dwmapi.h>
#pragma comment(lib, "Dwmapi.lib")

/*
** initialize variables
*/
const TCHAR *strWndClassName = _T("cui_rawShadow");
bool CShadow::s_bVista = false;
std::map<HWND, CShadow *> CShadow::s_Shadowmap;
CCriticalSection CShadow::m_ShadowLocker;

CShadow::CShadow()
{
	m_hShadow = NULL;
	m_OriParentProc = NULL;
	m_nDarkness = 100;
	m_nSharpness = 5;
	m_nSize = 0;
	m_nxOffset = 5;
	m_nyOffset = 5;
	m_Color = RGB(0, 0, 0);
	m_WndSize = 0;
	m_bUpdate = false;
}

/*
** CLEAN UP
*/
CShadow::~CShadow()
{
	if (m_hShadow && s_Shadowmap.size())
	{
		// switch back to the original window procedure
		SetWindowLongPtr(GetParent(m_hShadow), GWLP_WNDPROC, m_OriParentProc);

		{
			CCriticalSectionLocker lock(m_ShadowLocker);
			// Remove this window and shadow from the map
			s_Shadowmap.erase(m_hShadow);
		}

		// close the shadow window
		SendMessage(m_hShadow, WM_CLOSE, NULL, NULL);

		// reset the window handle
		m_hShadow = NULL;
	}
}

LRESULT CALLBACK CShadow::ShadowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
} // ShadowProc

  /*
  ** create shadow on window
  */
void CShadow::CreateShadow(HWND hParent,
	const shadow_properties &properties)
{
	if (properties.size > 20 || properties.size < -20)
		m_nSize = (signed char)10;
	else
		m_nSize = (signed char)properties.size;

	if (properties.sharpness > 20)
		m_nSharpness = (unsigned char)20;
	else
		m_nSharpness = (unsigned char)properties.sharpness;

	if (properties.darkness > 255)
		m_nDarkness = (unsigned char)255;
	else
		m_nDarkness = (unsigned char)properties.darkness;

	if (properties.position.x > 20 || properties.position.x < -20 ||
		properties.position.y > 20 || properties.position.y < -20)
	{
		m_nxOffset = (signed char)0;
		m_nyOffset = (signed char)0;
	}
	else
	{
		m_nxOffset = (signed char)properties.position.x;
		m_nyOffset = (signed char)properties.position.y;
	}

	m_Color = properties.color;

	// Register window class for shadow window
	WNDCLASSEX wcex = { 0 };

	memset(&wcex, 0, sizeof(wcex));

	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = ShadowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = GetModuleHandle(NULL);
	wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = strWndClassName;
	wcex.hIconSm = NULL;

	RegisterClassEx(&wcex);

	/*
	** check if shadow has already been created
	*/
	if (m_hShadow)
		return;

	if (!(s_Shadowmap.find(hParent) == s_Shadowmap.end()))
		return;	// Only one shadow for each window

	{
		CCriticalSectionLocker lock(m_ShadowLocker);
		// Add parent window - shadow pair to the map
		s_Shadowmap[hParent] = this;
	}

	// Create the shadow window
	m_hShadow = CreateWindowEx(WS_EX_LAYERED | WS_EX_TRANSPARENT, strWndClassName, NULL,
		WS_POPUPWINDOW, CW_USEDEFAULT, 0, 0, 0, hParent, NULL, GetModuleHandle(NULL), NULL);

	m_Status = SS_ENABLED;	// Enabled by default

							// check if parent has the WM_POPUP style
	DWORD dwStyle = GetWindowLong(hParent, GWL_STYLE);

	if (!(dwStyle & WS_POPUP))
	{
		// Determine the initial show state of shadow according to Aero
		BOOL bAero = FALSE;

		DwmIsCompositionEnabled(&bAero);

		if (bAero)
			m_Status |= SS_DISABLEDBYAERO;
	}

	// Replace the original WndProc of parent window to steal messages
	m_OriParentProc = GetWindowLongPtr(hParent, GWLP_WNDPROC);
	SetWindowLongPtr(hParent, GWLP_WNDPROC, (LONG_PTR)NewProc);

	return;
} // CreateShadow

  /*
  ** new procedure for parent window
  */
LRESULT CALLBACK CShadow::NewProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (!(s_Shadowmap.find(hWnd) != s_Shadowmap.end()))
		return DefWindowProc(hWnd, uMsg, wParam, lParam);	// Shadow not attached to this window (should NEVER happen)

															// get pointer to the current shadow object from the map
	CShadow *pThis = s_Shadowmap[hWnd];

	// retrieve the default(original) window procedure
	WNDPROC pDefProc = (WNDPROC)pThis->m_OriParentProc;

	switch (uMsg)
	{
	case WM_MOVE:
		if (pThis->m_Status & SS_VISABLE)
		{
			// move shadow window together with parent
			RECT WndRect;
			GetWindowRect(hWnd, &WndRect);

			int x = WndRect.left + pThis->m_nxOffset - pThis->m_nSize;
			int y = WndRect.top + pThis->m_nyOffset - pThis->m_nSize;

			SetWindowPos(pThis->m_hShadow, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE);
		}
		break;

	case WM_SIZE:
		if (pThis->m_Status & SS_ENABLED && !(pThis->m_Status & SS_DISABLEDBYAERO))
		{
			if (SIZE_MAXIMIZED == wParam || SIZE_MINIMIZED == wParam)
			{
				// parent window either maximized or minimized, hide the shadow window
				ShowWindow(pThis->m_hShadow, SW_HIDE);
				pThis->m_Status &= ~SS_VISABLE;
				pThis->UpdateShadow(hWnd);
			}
			else
			{
				LONG lParentStyle = GetWindowLong(hWnd, GWL_STYLE);

				if (WS_VISIBLE & lParentStyle)
				{
					// Parent may be resized even if invisible

					// add SS_PARENTVISIBLE flag to m_Status
					pThis->m_Status |= SS_PARENTVISIBLE;

					if (!(pThis->m_Status & SS_VISABLE))
					{
						pThis->m_Status |= SS_VISABLE;

						/*
						** Update before before showing, otherwise restoring
						** from maximized state will reveal a glance of a misplaced shadow
						*/
						pThis->UpdateShadow(hWnd);
						ShowWindow(pThis->m_hShadow, SW_SHOWNA);

						/*
						** If restoring from minimized state, the window region will not be updated until WM_PAINT :(
						*/
						pThis->m_bUpdate = true;
					}
					else
						if (LOWORD(lParam) > LOWORD(pThis->m_WndSize) || HIWORD(lParam) > HIWORD(pThis->m_WndSize))
						{
							/*
							** It seems that if the window size has not decreased
							** the window region would never be updated until WM_PAINT has been sent
							** So do not UpdateShadow() until next WM_PAINT is received in this case
							*/
							pThis->m_bUpdate = true;
						}
						else
							pThis->UpdateShadow(hWnd);
				}

			}

			pThis->m_WndSize = lParam;
		}
		break;

	case WM_PAINT:
	{
		if (pThis->m_bUpdate)
		{
			pThis->UpdateShadow(hWnd);
			pThis->m_bUpdate = false;
		}
		else
		{
			if (!(pThis->m_Status & SS_VISABLE))
				ShowWindow(pThis->m_hShadow, SW_HIDE);
		}
		break;
	}

	case WM_EXITSIZEMOVE:
		if (pThis->m_Status & SS_VISABLE)
		{
			/*
			** In some cases of sizing, the up-right corner of the parent window region would not be properly updated
			** UpdateShadow() again when sizing is finished
			*/
			pThis->UpdateShadow(hWnd);
		}
		break;

	case WM_SHOWWINDOW:
		if (pThis->m_Status & SS_ENABLED && !(pThis->m_Status & SS_DISABLEDBYAERO))
		{
			LRESULT lResult = pDefProc(hWnd, uMsg, wParam, lParam);

			if (!wParam)
			{
				/*
				** the window is being hidden
				** hide the shadow
				*/
				ShowWindow(pThis->m_hShadow, SW_HIDE);
				pThis->m_Status &= ~(SS_VISABLE | SS_PARENTVISIBLE);
			}
			else
			{
				/*
				** show the shadow
				*/
				pThis->m_bUpdate = true;
				pThis->ShowShadow(hWnd);
			}
			return lResult;
		}
		break;

	case WM_DESTROY:
		// Destroy the shadow
		DestroyWindow(pThis->m_hShadow);
		break;

	case WM_NCDESTROY:
	{
		CCriticalSectionLocker lock(m_ShadowLocker);
		// Remove this window and shadow from the map
		s_Shadowmap.erase(hWnd);
	}
	break;

	case WM_DWMCOMPOSITIONCHANGED:
	{
		// check if parent has the WM_POPUP style
		DWORD dwStyle = GetWindowLong(hWnd, GWL_STYLE);

		if (!(dwStyle & WS_POPUP))
		{
			// Determine the initial show state of shadow according to Aero
			BOOL bAero = FALSE;

			DwmIsCompositionEnabled(&bAero);

			if (bAero)
				pThis->m_Status |= SS_DISABLEDBYAERO;
			else
				pThis->m_Status &= ~SS_DISABLEDBYAERO;
		}

		// only show the shadow if the parent is visible
		if (pThis->m_Status & SS_PARENTVISIBLE)
		{
			pThis->ShowShadow(hWnd);
		}
	}
	break;

	default:
		break;
	}

	/*
	** Call the default(original) window procedure for other messages
	** or messages that have been processed but not returned
	*/
	return pDefProc(hWnd, uMsg, wParam, lParam);
} // NewProc

  /*
  ** update shadow
  ** to be called when window is resized, or if shadow properties change
  ** do not call when simply moving the window without resizing
  */
void CShadow::UpdateShadow(
	HWND hParent	// parent window handle
)
{
	RECT WndRect;
	GetWindowRect(hParent, &WndRect);
	int nShadWndWid = WndRect.right - WndRect.left + m_nSize * 2;
	int nShadWndHei = WndRect.bottom - WndRect.top + m_nSize * 2;

	/*
	** Create the alpha blending bitmap
	*/
	BITMAPINFO bmi;	// bitmap header

	ZeroMemory(&bmi, sizeof(BITMAPINFO));
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = nShadWndWid;
	bmi.bmiHeader.biHeight = nShadWndHei;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;	// four 8-bit components
	bmi.bmiHeader.biCompression = BI_RGB;
	bmi.bmiHeader.biSizeImage = nShadWndWid * nShadWndHei * 4;

	BYTE *pvBits;	// pointer to DIB section
	HBITMAP hbitmap = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, (void **)&pvBits, NULL, 0);

	if (hbitmap == NULL)
		return;	// CreateDIBSection was NOT successful

	ZeroMemory(pvBits, bmi.bmiHeader.biSizeImage);

	// make shadow
	MakeShadow((UINT32 *)pvBits, hParent, &WndRect);

	HDC hMemDC = CreateCompatibleDC(NULL);
	HBITMAP hOriBmp = (HBITMAP)SelectObject(hMemDC, hbitmap);

	POINT ptDst = { WndRect.left + m_nxOffset - m_nSize, WndRect.top + m_nyOffset - m_nSize };
	POINT ptSrc = { 0, 0 };
	SIZE WndSize = { nShadWndWid, nShadWndHei };

	// blend function
	BLENDFUNCTION blendPixelFunction = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };

	// move shadow window
	MoveWindow(m_hShadow, ptDst.x, ptDst.y, nShadWndWid, nShadWndHei, FALSE);

	BOOL bRet = UpdateLayeredWindow(m_hShadow, NULL, &ptDst, &WndSize, hMemDC,
		&ptSrc, 0, &blendPixelFunction, ULW_ALPHA);

	if (bRet == FALSE)
	{
		// something was wrong
	}

	// Delete used resources
	SelectObject(hMemDC, hOriBmp);
	DeleteObject(hbitmap);
	DeleteDC(hMemDC);

	return;
} // UpdateShadow

  /*
  ** Fill in the shadow window alpha blend bitmap with shadow image pixels
  */
void CShadow::MakeShadow(
	UINT32 *pShadBits,
	HWND hParent,
	RECT *rcParent
)
{
	/*
	** The shadow algorithm:
	**
	** 1. Get the region of parent window,
	** 2. Apply morphologic erosion to shrink it into the size (ShadowWndSize - Sharpness)
	** 3. Apply modified (with blur effect) morphologic dilation to make the blurred border
	**
	** The algorithm is optimized by assuming parent window is just "one piece" and without "wholes" on it
	*/

	/*
	** STEP 1:
	** Get the region of parent window,
	** Create a full rectangle region in case of the window region is not defined
	*/
	HRGN hParentRgn = CreateRectRgn(0, 0, rcParent->right - rcParent->left, rcParent->bottom - rcParent->top);
	GetWindowRgn(hParent, hParentRgn);

	// Determine the Start and end point of each horizontal scan line
	SIZE szParent = { rcParent->right - rcParent->left, rcParent->bottom - rcParent->top };
	SIZE szShadow = { szParent.cx + 2 * m_nSize, szParent.cy + 2 * m_nSize };

	// Extra 2 lines (set to be empty) in ptAnchors are used in dilation
	int nAnchors = max(szParent.cy, szShadow.cy);	// # of anchor points pares
	int(*ptAnchors)[2] = new int[nAnchors + 2][2];
	int(*ptAnchorsOri)[2] = new int[szParent.cy][2];	// anchor points, will not modify during erosion
	ptAnchors[0][0] = szParent.cx;
	ptAnchors[0][1] = 0;
	ptAnchors[nAnchors + 1][0] = szParent.cx;
	ptAnchors[nAnchors + 1][1] = 0;

	if (m_nSize > 0)
	{
		// Put the parent window anchors at the center
		for (int i = 0; i < m_nSize; i++)
		{
			ptAnchors[i + 1][0] = szParent.cx;
			ptAnchors[i + 1][1] = 0;
			ptAnchors[szShadow.cy - i][0] = szParent.cx;
			ptAnchors[szShadow.cy - i][1] = 0;
		}

		ptAnchors += m_nSize;
	}

	for (int i = 0; i < szParent.cy; i++)
	{
		// find start point
		int j = 0;

		for (j = 0; j < szParent.cx; j++)
		{
			if (PtInRegion(hParentRgn, j, i))
			{
				ptAnchors[i + 1][0] = j + m_nSize;
				ptAnchorsOri[i][0] = j;
				break;
			}
		}

		if (j >= szParent.cx)
		{
			// Start point not found
			ptAnchors[i + 1][0] = szParent.cx;
			ptAnchorsOri[i][1] = 0;
			ptAnchors[i + 1][0] = szParent.cx;
			ptAnchorsOri[i][1] = 0;
		}
		else
		{
			// find end point
			for (j = szParent.cx - 1; j >= ptAnchors[i + 1][0]; j--)
			{
				if (PtInRegion(hParentRgn, j, i))
				{
					ptAnchors[i + 1][1] = j + 1 + m_nSize;
					ptAnchorsOri[i][1] = j + 1;
					break;
				}
			}
		}
	}

	if (m_nSize > 0)
		ptAnchors -= m_nSize;	// Restore pos of ptAnchors for erosion

	int(*ptAnchorsTmp)[2] = new int[nAnchors + 2][2];	// Store the result of erosion

														// First and last line should be empty
	ptAnchorsTmp[0][0] = szParent.cx;
	ptAnchorsTmp[0][1] = 0;
	ptAnchorsTmp[nAnchors + 1][0] = szParent.cx;
	ptAnchorsTmp[nAnchors + 1][1] = 0;

	/*
	** STEP 2:
	** perform morphologic erosion
	*/

	for (int i = 0; i < m_nSharpness - m_nSize; i++)
	{
		for (int j = 1; j < nAnchors + 1; j++)
		{
			ptAnchorsTmp[j][0] = max(ptAnchors[j - 1][0], max(ptAnchors[j][0], ptAnchors[j + 1][0])) + 1;
			ptAnchorsTmp[j][1] = min(ptAnchors[j - 1][1], min(ptAnchors[j][1], ptAnchors[j + 1][1])) - 1;
		}

		// Exchange ptAnchors and ptAnchorsTmp;
		int(*ptAnchorsXange)[2] = ptAnchorsTmp;
		ptAnchorsTmp = ptAnchors;
		ptAnchors = ptAnchorsXange;
	}

	/*
	** STEP 3:
	** perform morphologic dilation
	*/

	ptAnchors += (m_nSize < 0 ? -m_nSize : 0) + 1;	// now coordinates in ptAnchors are same as in shadow window

													// Generate the kernel
	int nKernelSize = m_nSize > m_nSharpness ? m_nSize : m_nSharpness;
	int nCenterSize = m_nSize > m_nSharpness ? (m_nSize - m_nSharpness) : 0;
	UINT32 *pKernel = new UINT32[(2 * nKernelSize + 1) * (2 * nKernelSize + 1)];
	UINT32 *pKernelIter = pKernel;

	for (int i = 0; i <= 2 * nKernelSize; i++)
	{
		for (int j = 0; j <= 2 * nKernelSize; j++)
		{
			double dLength = sqrt((i - nKernelSize) * (i - nKernelSize) + (j - nKernelSize) * (double)(j - nKernelSize));

			if (dLength < nCenterSize)
				*pKernelIter = m_nDarkness << 24 | PreMultiply(m_Color, m_nDarkness);
			else
				if (dLength <= nKernelSize)
				{
					UINT32 nFactor = ((UINT32)((1 - (dLength - nCenterSize) / (m_nSharpness + 1)) * m_nDarkness));
					*pKernelIter = nFactor << 24 | PreMultiply(m_Color, nFactor);
				}
				else
					*pKernelIter = 0;

			pKernelIter++;
		}
	}

	// Generate blurred border
	for (int i = nKernelSize; i < szShadow.cy - nKernelSize; i++)
	{
		int j = 0;

		if (ptAnchors[i][0] < ptAnchors[i][1])
		{
			// Start of line
			for (j = ptAnchors[i][0]; j < min(max(ptAnchors[i - 1][0], ptAnchors[i + 1][0]) + 1, ptAnchors[i][1]); j++)
			{
				for (int k = 0; k <= 2 * nKernelSize; k++)
				{
					UINT32 *pPixel = pShadBits + (szShadow.cy - i - 1 + nKernelSize - k) * szShadow.cx + j - nKernelSize;
					UINT32 *pKernelPixel = pKernel + k * (2 * nKernelSize + 1);

					for (int l = 0; l <= 2 * nKernelSize; l++)
					{
						if (*pPixel < *pKernelPixel)
							*pPixel = *pKernelPixel;

						pPixel++;
						pKernelPixel++;
					}
				}
			} // for() start of line

			  // End of line
			for (j = max(j, min(ptAnchors[i - 1][1], ptAnchors[i + 1][1]) - 1); j < ptAnchors[i][1]; j++)
			{
				for (int k = 0; k <= 2 * nKernelSize; k++)
				{
					UINT32 *pPixel = pShadBits +
						(szShadow.cy - i - 1 + nKernelSize - k) * szShadow.cx + j - nKernelSize;
					UINT32 *pKernelPixel = pKernel + k * (2 * nKernelSize + 1);
					for (int l = 0; l <= 2 * nKernelSize; l++)
					{
						if (*pPixel < *pKernelPixel)
							*pPixel = *pKernelPixel;
						pPixel++;
						pKernelPixel++;
					}
				}
			} // for() end of line

		}
	} // for() Generate blurred border

	  // Erase unwanted parts and complement missing
	UINT32 clCenter = m_nDarkness << 24 | PreMultiply(m_Color, m_nDarkness);

	for (int i = min(nKernelSize, max(m_nSize - m_nyOffset, 0)); i < max(szShadow.cy - nKernelSize, min(szParent.cy + m_nSize - m_nyOffset, szParent.cy + 2 * m_nSize)); i++)
	{
		UINT32 *pLine = pShadBits + (szShadow.cy - i - 1) * szShadow.cx;

		if (i - m_nSize + m_nyOffset < 0 || i - m_nSize + m_nyOffset >= szParent.cy)
		{
			// Line is not covered by parent window
			for (int j = ptAnchors[i][0]; j < ptAnchors[i][1]; j++)
			{
				*(pLine + j) = clCenter;
			}
		}
		else
		{
			for (int j = ptAnchors[i][0]; j < min(ptAnchorsOri[i - m_nSize + m_nyOffset][0] + m_nSize - m_nxOffset, ptAnchors[i][1]); j++)
				*(pLine + j) = clCenter;

			for (int j = max(ptAnchorsOri[i - m_nSize + m_nyOffset][0] + m_nSize - m_nxOffset, 0); j < min(ptAnchorsOri[i - m_nSize + m_nyOffset][1] + m_nSize - m_nxOffset, szShadow.cx); j++)
				*(pLine + j) = 0;

			for (int j = max(ptAnchorsOri[i - m_nSize + m_nyOffset][1] + m_nSize - m_nxOffset, ptAnchors[i][0]); j < ptAnchors[i][1]; j++)
				*(pLine + j) = clCenter;
		}
	}

	// Delete used resources
	delete[](ptAnchors - (m_nSize < 0 ? -m_nSize : 0) - 1);
	delete[] ptAnchorsTmp;
	delete[] ptAnchorsOri;
	delete[] pKernel;
	DeleteObject(hParentRgn);

	return;
} // MakeShadow

  /*
  ** Show or hide the shadow
  ** depending on the enabled status stored in m_Status
  */
void CShadow::ShowShadow(
	HWND hParent	// parent window handle
)
{
	/*
	** Clear all flags except the enabled status
	*/
	m_Status &= SS_ENABLED | SS_DISABLEDBYAERO;

	if ((m_Status & SS_ENABLED) && !(m_Status & SS_DISABLEDBYAERO))	// Enabled
	{
		// Determine the show state of shadow according to parent window's state
		LONG lParentStyle = GetWindowLong(hParent, GWL_STYLE);

		m_Status |= SS_PARENTVISIBLE;

		// Parent is normal, show the shadow
		if (!((WS_MAXIMIZE | WS_MINIMIZE) & lParentStyle))	// Parent visible but does not need shadow
			m_Status |= SS_VISABLE;
	}

	if (m_Status & SS_VISABLE)
	{
		ShowWindow(m_hShadow, SW_SHOWNA);
		UpdateShadow(hParent);
	}
	else
		ShowWindow(m_hShadow, SW_HIDE);

	return;
} // ShowShadow
