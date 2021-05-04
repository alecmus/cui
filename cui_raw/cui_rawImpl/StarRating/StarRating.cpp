/*
** StarRating.cpp - star rating control implementation
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

#include "../cui_rawImpl.h"
#include "../../clrAdjust/clrAdjust.h"

#define _USE_MATH_DEFINES
#include <math.h>

/*
** Return PointFs to define a star.
** Original code from: http://csharphelper.com/blog/2014/08/draw-a-star-in-c/
*/
std::vector<Gdiplus::PointF> StarPoints(int num_points, Gdiplus::Rect bounds)
{
	// Make room for the points.
	std::vector<Gdiplus::PointF> pts(num_points);

	double rx = bounds.Width / 2;
	double ry = bounds.Height / 2;
	double cx = bounds.X + rx;
	double cy = bounds.Y + ry;

	// Start at the top.
	double theta = -M_PI / 2;
	double dtheta = 4 * M_PI / num_points;
	for (int i = 0; i < num_points; i++)
	{
		pts[i] = Gdiplus::PointF(
			(float)(cx + rx * cos(theta)),
			(float)(cy + ry * sin(theta)));
		theta += dtheta;
	}

	return pts;
} // StarPoints

void drawStar(HDC hdc, RECT rc, COLORREF clrLine, int iThickNess, int iPoints = 5)
{
	Gdiplus::Graphics graphics(hdc);
	graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);

	Gdiplus::Rect bounds(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);

	std::vector<Gdiplus::PointF> m_pts = StarPoints(iPoints, bounds);
	Gdiplus::PointF *pts = new Gdiplus::PointF[iPoints];

	for (size_t i = 0; i < (size_t)iPoints; i++)
		pts[i] = m_pts[i];

	Gdiplus::Color color;
	color.SetFromCOLORREF(clrLine);
	Gdiplus::Pen pen(color, (Gdiplus::REAL)iThickNess);
	pen.SetAlignment(Gdiplus::PenAlignmentCenter);

	graphics.DrawPolygon(&pen, pts, iPoints);
} // drawStar

void drawFilledStar(HDC hdc, RECT rc, COLORREF clrFill, int iPoints = 5)
{
	Gdiplus::Graphics graphics(hdc);
	graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);

	Gdiplus::Rect bounds(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);

	std::vector<Gdiplus::PointF> m_pts = StarPoints(iPoints, bounds);
	Gdiplus::PointF *pts = new Gdiplus::PointF[iPoints];

	for (size_t i = 0; i < (size_t)iPoints; i++)
		pts[i] = m_pts[i];

	Gdiplus::Color color;
	color.SetFromCOLORREF(clrFill);
	Gdiplus::SolidBrush brush(color);

	graphics.FillPolygon(&brush, pts, iPoints, Gdiplus::FillMode::FillModeWinding);

	if (pts)
	{
		delete pts;
		pts = NULL;
	}
} // drawFilledStar

LRESULT CALLBACK cui_rawImpl::StarRatingProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	StarRatingControl* pControl = reinterpret_cast<StarRatingControl*>(ptr);

	switch (msg)
	{
	case WM_PAINT:
	{
		RECT itemRect;
		GetClientRect(hWnd, &itemRect);
		const int cx = itemRect.right - itemRect.left;
		const int cy = itemRect.bottom - itemRect.top;

		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hWnd, &ps);

		// use double buffering to avoid flicker
		HDC hdc = CreateCompatibleDC(dc);

		if (!pControl->hbm_buffer)
			pControl->hbm_buffer = CreateCompatibleBitmap(dc, cx, cy);

		HBITMAP hbmOld = SelectBitmap(hdc, pControl->hbm_buffer);

		{
			CBrush brBackground(pControl->d->m_clrBackground);
			FillRect(hdc, &itemRect, brBackground.get());
		}

		SetBkMode(hdc, TRANSPARENT);

		COLORREF clrOn = pControl->clrOn;
		COLORREF clrOff = pControl->clrOff;

		if (!IsWindowEnabled(hWnd))
		{
			clrOn = clrDarken(pControl->d->m_clrDisabled, 30);
			clrOff = pControl->d->m_clrDisabled;
		}
		else
			if (pControl->bHot)
				clrOn = pControl->clrHot;

		if (pControl->iHighestRating >= 3)	// failsafe
		{
			pControl->rcStars.resize(pControl->iHighestRating);

			int i = 0;
			for (auto &it : pControl->rcStars)
			{
				i++;

				if (i > pControl->iRating)
					it.clr = clrOff;
				else
					it.clr = clrOn;
			}

			// divide rect into x pControl->iHighestRating
			RECT rc = itemRect;
			pControl->rcStarRating = rc;

			int iWidth = (rc.right - rc.left) / pControl->iHighestRating;
			int iHeight = rc.bottom - rc.top;

			iWidth = iWidth < iHeight ? iWidth : iHeight;

			rc.right = rc.left + iWidth;
			rc.bottom = rc.top + iWidth;

			auto m_make = [&](int iStar)
			{
				cui_raw::posRect(rc, itemRect, 100 * (iStar - 1) / (pControl->iHighestRating - 1), 50);
				pControl->rcStars[iStar - 1].rc = rc;

				if (pControl->bHot)
				{
					if (pControl->rcStars[iStar - 1].bHot)
						pControl->rcStars[iStar - 1].clr = clrOn;
					else
						pControl->rcStars[iStar - 1].clr = clrOff;
				}

				drawFilledStar(hdc, rc, pControl->rcStars[iStar - 1].clr);
			};

			int iStarNumber = 1;

			// fill stars
			int iTop = 0;
			int iBottom = 0;
			for (auto &it : pControl->rcStars)
			{
				m_make(iStarNumber);
				iStarNumber++;

				iTop = it.rc.top;
				iBottom = it.rc.bottom;
			}

			pControl->rcStarRating.top = iTop;
			pControl->rcStarRating.bottom = iBottom;
		}

		// do the BitBlt
		BitBlt(dc, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY);
		SelectBitmap(hdc, hbmOld);

		EndPaint(hWnd, &ps);

		// cleanup
		DeleteDC(hdc);
	}
	break;

	case WM_ENABLE:
	{
		bool bEnabled = wParam == TRUE;

		if (bEnabled)
		{
			pControl->state = enumBtn::normal;
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else
		{
			InvalidateRect(hWnd, NULL, FALSE);
		}
	}
	break;

	case WM_SHOWWINDOW:
	{
		if (wParam == FALSE)
			pControl->toolTip.bAllowToolTip = false;

		if (wParam == TRUE)
			pControl->toolTip.bAllowToolTip = true;
	}
	break;

	case WM_DESTROY:
	{
		// delete buffer, we're done
		if (pControl->hbm_buffer)
		{
			DeleteBitmap(pControl->hbm_buffer);
			pControl->hbm_buffer = NULL;
		}
	}
	break;

	case WM_SIZE:
	{
		// delete buffer, we need it recreated
		if (pControl->hbm_buffer)
		{
			DeleteBitmap(pControl->hbm_buffer);
			pControl->hbm_buffer = NULL;
		}

		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;
	}

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)pControl->PrevProc, hWnd, msg, wParam, lParam);
} // StarRatingProc
