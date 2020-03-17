/*
** PieChart.cpp - pie chart implementation
**
** cui framework
** Copyright (c) 2016 Alec T. Musasa (alecmus at live dot com)
**
*******************************************************************************
** This file is part of the liblec library which is released under the Creative
** Commons Attribution Non-Commercial 2.0 license (CC-BY-NC 2.0). See the file
** LICENSE.txt or go to https://github.com/alecmus/liblec/blob/master/LICENSE.md
** for full license details.
*/

#include "../cui_rawImpl.h"
#include "../../clrAdjust/clrAdjust.h"

/// <summary>
/// Rounding off class.
/// </summary>
class CRoundOff
{
public:
	/// <summary>
	/// Round-off a double to a string.
	/// </summary>
	/// 
	/// <param name="d">
	/// The double to round-off.
	/// </param>
	/// 
	/// <param name="iDecimalPlaces">
	/// The number of decimal places to round it off to.
	/// </param>
	/// 
	/// <returns>
	/// The rounded-off value, as a string.
	/// </returns>
	template <typename T>
	static std::basic_string<T> tostr(const double &d, int iDecimalPlaces)
	{
		std::basic_stringstream<T> ss;
		ss << std::fixed;
		ss.precision(iDecimalPlaces);
		ss << d;
		return ss.str();
	} // tostr

	/// <summary>
	/// Round-off a double to another double.
	/// </summary>
	/// 
	/// <param name="d">
	/// The double to round-off.
	/// </param>
	/// 
	/// <param name="iDecimalPlaces">
	/// The number of decimal places to round it off to.
	/// </param>
	/// 
	/// <returns>
	/// The rounded-off value.
	/// </returns>
	static double tod(const double &d, int iDecimalPlaces)
	{
		int y = (int)d;
		double z = d - (double)y;
		double m = pow(10, iDecimalPlaces);
		double q = z * m;
		double r = round(q);

		return static_cast<double>(y) + (1.0 / m) * r;
	} // tod
}; // CRoundOff

template<typename It>
class Range
{
	It b, e;
public:
	Range(It b, It e) : b(b), e(e) {}
	It begin() const { return b; }
	It end() const { return e; }
};

template<typename ORange, typename OIt = decltype(std::begin(std::declval<ORange>())), typename It = std::reverse_iterator<OIt>>
Range<It> reverse(ORange && originalRange) {
	return Range<It>(It(std::end(originalRange)), It(std::begin(originalRange)));
}

static bool design = false;
static bool border = false;

// TO-DO: use proportional drawing instead of fixed items like iBorderWidth = 1
void DrawPieChart(HWND hGraph, HDC hdc, cui_rawImpl::PieChartControl* pState)
{
	Gdiplus::Graphics graphics(hdc);
	graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);

	Gdiplus::Color color;
	color.SetFromCOLORREF(pState->d->m_clrBackground);
	graphics.Clear(color);

	int iBorderWidth = 1;

	// get control coordinates
	RECT rectChart;
	GetClientRect(hGraph, &rectChart);

	InflateRect(&rectChart, -iBorderWidth, -iBorderWidth);

	RECT rectChartControl = rectChart;

	// 1. draw bounding rectangle for chart control
	if (border)
	{
		RECT rcBorder = rectChartControl;
		InflateRect(&rcBorder, -1, -1);

		Gdiplus::RectF rect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rcBorder);

		color.SetFromCOLORREF(RGB(200, 200, 200));
		Gdiplus::Pen pen(color, 1);
		pen.SetAlignment(Gdiplus::PenAlignment::PenAlignmentCenter);
		graphics.DrawRectangle(&pen, rect);
	}

	Gdiplus::REAL top = static_cast<Gdiplus::REAL>(rectChartControl.top);
	Gdiplus::REAL bottom = static_cast<Gdiplus::REAL>(rectChartControl.bottom);
	Gdiplus::REAL right = 0.0f;

	// draw labels
	Gdiplus::RectF rectLabels;
	{
		Gdiplus::REAL width =
			static_cast<Gdiplus::REAL>(rectChartControl.right - rectChartControl.left) / 8;

		width = width > 100.0f ? width : 100.0f;
		
		rectLabels.X = static_cast<Gdiplus::REAL>(rectChartControl.right) - width - 4.0f;
		rectLabels.Y = top + 4.0f;
		rectLabels.Width = width;
		rectLabels.Height = bottom - top - 4.0f * 2.0f;

		if (design)
		{
			color.SetFromCOLORREF(RGB(240, 240, 240));
			Gdiplus::SolidBrush brush(color);
			graphics.FillRectangle(&brush, rectLabels);
		}

		right = rectLabels.GetLeft();
	}

	rectChartControl.right = static_cast<LONG>(right);

	pState->rcPieChart = rectChart;

	int iWidth = rectChartControl.right - rectChartControl.left;
	int iHeight = rectChartControl.bottom - rectChartControl.top;

	Gdiplus::REAL angle = 270;
	Gdiplus::REAL sweep = 0;

	// total
	double dTotal = 0;

	for (auto it : pState->vData)
		dTotal += it.dValue;

	bool bFill = true;

	int side = min(iWidth, iHeight);

	Gdiplus::RectF rect;
	rect.Width = static_cast<Gdiplus::REAL>(side);
	rect.Height = static_cast<Gdiplus::REAL>(side);
	rect.X = static_cast<Gdiplus::REAL>(rectChartControl.left + ((rectChartControl.right - rectChartControl.left) - side) / 2);
	rect.Y = static_cast<Gdiplus::REAL>(rectChartControl.top + ((rectChartControl.bottom - rectChartControl.top) - side) / 2);

	bool bHover = false;

	if (!pState->bInfoCaptured)
	{
		// cleanup previous pie chart info
		for (auto &it : pState->chartBarsInfo)
		{
			if (it.pRegion)
			{
				delete it.pRegion;
				it.pRegion = NULL;
			}
		}

		pState->chartBarsInfo.clear();
	}
	else
	{
		// check if there is a hover
		for (auto it : pState->chartBarsInfo)
		{
			if (it.bHot)
			{
				bHover = true;
				break;
			}
		}
	}

	auto drawPie = [](Gdiplus::Graphics &g, Gdiplus::Color &color, Gdiplus::RectF &rect, Gdiplus::REAL &angle, Gdiplus::REAL &sweep, int iBorderWidth)
	{
		// draw pie
		Gdiplus::SolidBrush brush(color);
		g.FillPie(&brush, rect, angle, sweep);

		if (iBorderWidth > 0)
		{
			// draw border
			color.SetFromCOLORREF(RGB(255, 255, 255));
			Gdiplus::Pen pen(color, (Gdiplus::REAL)iBorderWidth);
			g.DrawPie(&pen, rect, angle, sweep);
		}
	};

	int ix = 0;
	for (auto it : pState->vData)
	{
		// calculate sweep
		sweep = 0;

		if (dTotal > 0)
			sweep = Gdiplus::REAL(360) * Gdiplus::REAL(it.dValue) / Gdiplus::REAL(dTotal);

		{
			cui_rawImpl::pieChartItemInfo item;
			item.bHot = false;

			// create path
			Gdiplus::GraphicsPath path;

			// add pie to path
			path.AddPie(rect, angle, sweep);

			// make region from path
			Gdiplus::Region region(&path);

			if (!pState->bInfoCaptured)
			{
				// assign region to pie chart item
				item.pRegion = region.Clone();	// TO-DO: read about the Clone member and check whether we need to clear this memory later ... very crucial this check

				// assign item number
				item.iNumber = it.iNumber;

				// assign item label
				item.sItemLabel = it.sItemLabel;

				std::basic_string<TCHAR> sTooltip = it.sItemLabel;
				double dPercentage = 0;
				
				if (dTotal > 0)
					dPercentage = 100 * it.dValue / dTotal;

				sTooltip += _T(" (") + CRoundOff::tostr<TCHAR>(dPercentage, 1) + _T("%)");

				// capture tooltip text in the form "itemLabel (percentage %)
				item.sTooltip = sTooltip;

				// grab the item
				pState->chartBarsInfo.push_back(item);
			}
			else
			{
				// cleanup previous region
				if (pState->chartBarsInfo[ix].pRegion)
				{
					delete pState->chartBarsInfo[ix].pRegion;
					pState->chartBarsInfo[ix].pRegion = NULL;
				}

				// update pie chart region
				pState->chartBarsInfo[ix].pRegion = region.Clone();
			}

		}

		COLORREF clr = it.clrItem;

		if (pState->chartBarsInfo[ix].bHot)
		{
			if (pState->chartBarsInfo[ix].bPressed)
				clr = clrDarken(it.clrItem, 40);
			else
				clr = clrDarken(it.clrItem, 25);
		}

		Gdiplus::Color color;
		color.SetFromCOLORREF(clr);
		
		// if fill mode, fill pie
		if (bFill)
		{
			switch (pState->hoverEffect)
			{
			case cui_raw::pieChartHoverEffect::glowAndArc:
			{
				if (pState->chartBarsInfo[ix].bHot)
				{
					// draw arc
					Gdiplus::RectF rect_arc = rect;
					Gdiplus::Pen pen(color, (Gdiplus::REAL)iBorderWidth);
					graphics.DrawArc(&pen, rect_arc, angle, sweep);

					// draw pie
					Gdiplus::RectF rect_pie = rect;
					rect_pie.Inflate(static_cast<Gdiplus::REAL>(-2 * iBorderWidth), static_cast<Gdiplus::REAL>(-2 * iBorderWidth));

					drawPie(graphics, color, rect_pie, angle, sweep, iBorderWidth);
				}
				else
				{
					// draw pie
					Gdiplus::RectF rect_pie = rect;
					rect_pie.Inflate(static_cast<Gdiplus::REAL>(-2 * iBorderWidth), static_cast<Gdiplus::REAL>(-2 * iBorderWidth));

					drawPie(graphics, color, rect_pie, angle, sweep, iBorderWidth);
				}
			}
			break;

			case cui_raw::pieChartHoverEffect::glowAndGrow:
			{
				Gdiplus::RectF rect_pie = rect;

				if (!pState->chartBarsInfo[ix].bHot)
					rect_pie.Inflate(static_cast<Gdiplus::REAL>(-2 * iBorderWidth), static_cast<Gdiplus::REAL>(-2 * iBorderWidth));

				// draw pie
				drawPie(graphics, color, rect_pie, angle, sweep, iBorderWidth);
			}
			break;

			case cui_raw::pieChartHoverEffect::glowAndShrinkOthers:
			{
				if (bHover)
				{
					Gdiplus::RectF rect_pie = rect;

					if (!pState->chartBarsInfo[ix].bHot)
						rect_pie.Inflate(static_cast<Gdiplus::REAL>(-2 * iBorderWidth), static_cast<Gdiplus::REAL>(-2 * iBorderWidth));

					// draw pie
					drawPie(graphics, color, rect_pie, angle, sweep, iBorderWidth);
				}
				else
					drawPie(graphics, color, rect, angle, sweep, iBorderWidth);
			}
			break;

			case cui_raw::pieChartHoverEffect::glow:
			default:
				drawPie(graphics, color, rect, angle, sweep, iBorderWidth);
				break;
			}
		}
		else
		{
			Gdiplus::Pen pen(color, (Gdiplus::REAL)iBorderWidth);
			graphics.DrawPie(&pen, rect, angle, sweep);
		}

		angle += sweep;
		ix++;
	}

	if (dTotal <= 0)
	{
		// everything is a zero ... draw a light grey empty pie

		Gdiplus::Color color;
		color.SetFromCOLORREF(RGB(245, 245, 245));

		Gdiplus::RectF rect_pie = rect;

		Gdiplus::REAL angle = 270;
		Gdiplus::REAL sweep = 360;
		drawPie(graphics, color, rect_pie, angle, sweep, 0);
	}

	if (pState->bDoughnut)
	{
		Gdiplus::Color color;
		color.SetFromCOLORREF(RGB(255, 255, 255));

		// get height
		Gdiplus::REAL iFactor = static_cast<Gdiplus::REAL>(side) / 10.0f;

		iFactor = max(iFactor, 20.0f);

		Gdiplus::RectF rect_pie = rect;
		rect_pie.Inflate(-iFactor, -iFactor);

		Gdiplus::REAL angle = 270;
		Gdiplus::REAL sweep = 360;
		drawPie(graphics, color, rect_pie, angle, sweep, 0);
	}

	pState->bInfoCaptured = true;

	const int iItemHeight = 18;
	const int iMargin = 2;

	// check if items fit in the given space
	if (((iItemHeight + iMargin) * static_cast<int>(pState->vData.size())) > (rectChart.bottom - rectChart.top))
	{
		for (auto &it : pState->chartBarsInfo)
			it.rcLabel = { 0 };

		return;
	}

	int iLeftBorder = static_cast<int>(rectLabels.GetLeft());
	int iBottom = static_cast<int>(rectLabels.GetBottom());

	size_t m_x = pState->chartBarsInfo.size();
	for (auto it : reverse(pState->vData))
	{
		RECT rcBlock;
		rcBlock.bottom = iBottom;
		rcBlock.top = rcBlock.bottom - iItemHeight;
		rcBlock.left = iLeftBorder;
		rcBlock.right = rcBlock.left + iItemHeight;

		RECT rcLabel = rcBlock;

		m_x--;
		bool bHot = pState->chartBarsInfo[m_x].bHot;
		bool bPressed = pState->chartBarsInfo[m_x].bPressed;

		COLORREF clr = it.clrItem;

		if (bHot)
		{
			if (bPressed)
				clr = clrDarken(it.clrItem, 40);
			else
				clr = clrDarken(it.clrItem, 25);
		}

		// paint item block
		color.SetFromCOLORREF(clr);
		Gdiplus::SolidBrush block_brush(color);

		RECT rcBlockSmall = rcBlock;

		if (bHot)
			InflateRect(&rcBlockSmall, -4, -4);
		else
			InflateRect(&rcBlockSmall, -5, -5);

		Gdiplus::RectF rect_block = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rcBlockSmall);
		graphics.FillRectangle(&block_brush, rect_block);

		// write text
		RECT rcText = rcBlock;
		rcText.left = rcText.right + 5;
		rcText.right = static_cast<LONG>(rectLabels.GetRight()) - 5;

		{
			Gdiplus::RectF layoutRect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rcText);

			Gdiplus::Font* p_font = new Gdiplus::Font(&Gdiplus::FontFamily(pState->sFontName.c_str()), 7);

			if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
			{
				delete p_font;
				p_font = nullptr;
				p_font = new Gdiplus::Font(pState->sFontName.c_str(),
					7, Gdiplus::FontStyle::FontStyleRegular,
					Gdiplus::UnitPoint, &pState->d->m_font_collection);
			}

			color.SetFromCOLORREF(RGB(0, 0, 0));
			Gdiplus::SolidBrush text_brush(color);

			// measure text rectangle
			Gdiplus::RectF text_rect;
			graphics.MeasureString(it.sItemLabel.c_str(), -1, p_font, layoutRect, &text_rect);

			if (true)
			{
				if (text_rect.Width < layoutRect.Width)
					text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
			}

			// align the text rectangle to the layout rectangle
			liblec::cui::gui_raw::cui_rawImpl::align_text(text_rect, layoutRect,
				liblec::cui::gui_raw::cui_raw::textAlignment::middleleft);

			if (design)
			{
				color.SetFromCOLORREF(RGB(240, 240, 240));
				Gdiplus::SolidBrush brush(color);
				graphics.FillRectangle(&brush, layoutRect);

				color.SetFromCOLORREF(RGB(230, 230, 230));
				brush.SetColor(color);
				graphics.FillRectangle(&brush, text_rect);
			}

			Gdiplus::StringFormat format;
			format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
			format.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);
			format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingWord);

			// draw text
			graphics.DrawString(it.sItemLabel.c_str(),
				-1, p_font, text_rect, &format, &text_brush);

			delete p_font;
			p_font = nullptr;
		}

		// get overall label RECT
		rcLabel.right = rcText.right;

		// capture RECT
		pState->chartBarsInfo[m_x].rcLabel = rcLabel;

		iBottom -= (iItemHeight + iMargin);
	}
} // DrawPieChart

LRESULT CALLBACK cui_rawImpl::PieChartControlProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	cui_rawImpl::PieChartControl* pControl = reinterpret_cast<cui_rawImpl::PieChartControl*>(ptr);

	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC dc = BeginPaint(hWnd, &ps);

		RECT rcClient;
		GetClientRect(hWnd, &rcClient);
		int cx = rcClient.right - rcClient.left;
		int cy = rcClient.bottom - rcClient.top;

		// use double buffering to avoid flicker
		HDC hdc = CreateCompatibleDC(dc);

		if (!pControl->hbm_buffer)
			pControl->hbm_buffer = CreateCompatibleBitmap(dc, cx, cy);

		HBITMAP hbmOld = SelectBitmap(hdc, pControl->hbm_buffer);

		////////////////////////////////////////////////////////////////////////////////////////////////////////
		DrawPieChart(hWnd, hdc, pControl);
		////////////////////////////////////////////////////////////////////////////////////////////////////////

		BitBlt(dc, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY);

		EndPaint(hWnd, &ps);

		SelectBitmap(hdc, hbmOld);
		DeleteDC(hdc);
	}
	break;

	case WM_ENABLE:
	{
		bool bEnabled = wParam == TRUE;

		if (bEnabled)
		{
			// control has been enabled
			InvalidateRect(hWnd, NULL, TRUE);
		}
		else
		{
			// control has been disabled
			InvalidateRect(hWnd, NULL, TRUE);
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
} // PieChartControlProc
