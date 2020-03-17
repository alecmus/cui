/*
** BarChart.cpp - bar chart implementation
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
#include "../../reverse.h"

/*
** double to int (rounds off instead of truncating)
*/
static int double2int(double in)
{
	return (int)(0.5 + in);
}

/*
** round off a double to a given number of decimal places
*/
static std::basic_string<TCHAR> round_off(double number, int decimal_places)
{
	if ((decimal_places < 0) | (decimal_places > 5))
		return std::to_wstring(number);	// invalid decimal place value

	std::string statement = "%." + std::to_string(decimal_places) + "f";

	char perc_rounded[256];
	sprintf_s(perc_rounded, statement.c_str(), number, 256);

	std::string sPerc_rounded(perc_rounded);
	return std::basic_string<TCHAR>(sPerc_rounded.begin(), sPerc_rounded.end());
}

/////////////////////////////////////////////////////////////////////

// chart parameters
struct chartParams
{
	int iLowerLimit = 0;
	int iUpperLimit = 0;
	int iNumberOfLines = 0;
};

/*
** ensures that the lower limit
** - is less than supplied value
** - is divisible by 5
*/
static int adjustLowerLimit(int iLowest)
{
	int iNewLowerLimit = 0;

	if ((iLowest % 5) == 0)
	{
		// calculate new lower limit
		iNewLowerLimit = iLowest - 5;

		if (iNewLowerLimit < 0)
			iNewLowerLimit = 0;
	}
	else
	{
		// calculate new lower limit
		iNewLowerLimit = iLowest;

		do
		{
			iNewLowerLimit--;
		} while ((iNewLowerLimit % 5) != 0);
	}

	return iNewLowerLimit;
} // adjustLowerLimit

  /*
  ** ensures that the upper limit
  ** - is greater than supplied value
  ** - is divisible by 5
  */
static int adjustUpperLimit(int iHighest)
{
	int iNewUpperLimit = 0;

	if ((iHighest % 5) == 0)
	{
		// calculate new upper limit
		iNewUpperLimit = iHighest;
	}
	else
	{
		// calculate new upper limit
		iNewUpperLimit = iHighest;

		do
		{
			iNewUpperLimit++;
		} while ((iNewUpperLimit % 5) != 0);
	}

	return iNewUpperLimit;
} // adjustUpperLimit

static bool isRangeOk(int iLowerValue, int iUpperValue)
{
	if (iLowerValue < iUpperValue)
		return true;
	else
		return false;
} // isRangeOk

  /*
  ** iLowerLimit and iUpperLimit must be multiples of 5 first!!!!!
  ** works best for a range < 1000
  ** TO-DO: remove magic numbers
  */
static void calcParams(
	const int iLowerLimit,
	const int iUpperLimit,
	int &iNumberOfLines
)
{
	int iRange = iUpperLimit - iLowerLimit;

	double dSeperator = 200;

	// less than 1000
	if (iRange <= 1000)
		dSeperator = 100;

	if (iRange <= 500)
		dSeperator = 50;

	if (iRange <= 200)
		dSeperator = 20;

	// less than 100
	if (iRange <= 100)
		dSeperator = 10;

	if (iRange <= 50)
		dSeperator = 5;

	if (iRange <= 20)
		dSeperator = 2;

	// less than 10
	if (iRange <= 10)
		dSeperator = 1;

	if (iRange <= 5)
		dSeperator = 0.5;

	if (iRange <= 2)
		dSeperator = 0.3;

	// less than 1
	if (iRange <= 1)
		dSeperator = 0.1;

	iNumberOfLines = int((iRange / dSeperator) + 0.5);
} // calcParams

  /*
  ** calculate chart parameters
  ** returns true if successful, else returns false
  */
static bool calcChartParams(
	const double dLowest,
	const double dHighest,
	chartParams &params
)
{
	int iLowest = (int)dLowest;				// truncate
	int iHighest = (int)(dHighest + 0.5);	// round off

											// clear params
	params = {};

	if (isRangeOk(iLowest, iHighest))
	{
		int iLowerLimit = adjustLowerLimit(iLowest);
		int iUpperLimit = adjustUpperLimit(iHighest);

		int iNumberOfLines = 0;

		calcParams(iLowerLimit, iUpperLimit, iNumberOfLines);

		// write back parameters
		params.iLowerLimit = iLowerLimit;
		params.iNumberOfLines = iNumberOfLines;
		params.iUpperLimit = iUpperLimit;

		return true;
	}
	else
		return false;
} // calcChartParams

static bool design = false;
static bool border = true;

static void DrawChart(HWND hGraph, HDC hdc, cui_rawImpl::BarChartControl* pState)
{
	Gdiplus::Graphics graphics(hdc);
	Gdiplus::Color color;
	color.SetFromCOLORREF(pState->d->m_clrBackground);
	graphics.Clear(color);

	// get control coordinates
	RECT rectChartControl;
	GetClientRect(hGraph, &rectChartControl);

	const int absolute_right = rectChartControl.right;

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

	int iWidth = rectChartControl.right - rectChartControl.left;
	int iHeight = rectChartControl.bottom - rectChartControl.top;

	Gdiplus::REAL top = 0.0f;

	// draw chart title
	{
		RECT rectTitle;
		rectTitle.top = rectChartControl.top + 2;
		rectTitle.bottom = rectTitle.top;	// make height 0 so MeasureString can figure it out
		rectTitle.left = rectChartControl.left + 2;
		rectTitle.right = rectChartControl.right - 2;

		Gdiplus::RectF layoutRect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rectTitle);

		Gdiplus::Font* p_font = new Gdiplus::Font(&Gdiplus::FontFamily(pState->sFontName.c_str()), 11);

		if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
		{
			delete p_font;
			p_font = nullptr;
			p_font = new Gdiplus::Font(pState->sFontName.c_str(),
				11, Gdiplus::FontStyle::FontStyleRegular, Gdiplus::UnitPoint, &pState->d->m_font_collection);
		}

		color.SetFromCOLORREF(RGB(0, 120, 200));
		Gdiplus::SolidBrush text_brush(color);

		Gdiplus::StringFormat format;
		format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
		format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);
		format.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);

		// measure text rectangle
		Gdiplus::RectF text_rect;
		graphics.MeasureString(pState->sChartName.c_str(), -1, p_font, layoutRect, &text_rect);

		if (true)
		{
			if (text_rect.Width < layoutRect.Width)
				text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
		}

		layoutRect.Height = text_rect.Height;

		// align the text rectangle to the layout rectangle
		liblec::cui::gui_raw::cui_rawImpl::align_text(text_rect, layoutRect,
			liblec::cui::gui_raw::cui_raw::textAlignment::middle);

		if (design)
		{
			color.SetFromCOLORREF(RGB(240, 240, 240));
			Gdiplus::SolidBrush brush(color);
			graphics.FillRectangle(&brush, layoutRect);

			color.SetFromCOLORREF(RGB(230, 230, 230));
			brush.SetColor(color);
			graphics.FillRectangle(&brush, text_rect);
		}

		// draw text
		graphics.DrawString(pState->sChartName.c_str(),
			-1, p_font, text_rect, &format, &text_brush);

		delete p_font;
		p_font = nullptr;

		top = layoutRect.GetBottom();
	}

	Gdiplus::REAL bottom = 0.0f;

	// label x-axis
	{
		Gdiplus::RectF layoutRect;
		layoutRect.X = 0;
		layoutRect.Y = 0;
		layoutRect.Width =
			static_cast<Gdiplus::REAL>(rectChartControl.right - rectChartControl.left - 2 * 4);
		layoutRect.Height = 0;	// make 0 so measureString can figure it out

		Gdiplus::Font* p_font = new Gdiplus::Font(&Gdiplus::FontFamily(pState->sFontName.c_str()), 9);

		if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
		{
			delete p_font;
			p_font = nullptr;
			p_font = new Gdiplus::Font(pState->sFontName.c_str(),
				9, Gdiplus::Status::Ok, Gdiplus::UnitPoint, &pState->d->m_font_collection);
		}

		color.SetFromCOLORREF(RGB(0, 0, 0));
		Gdiplus::SolidBrush text_brush(color);

		Gdiplus::StringFormat format;
		format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
		format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);
		format.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);

		// measure text rectangle
		Gdiplus::RectF text_rect;
		graphics.MeasureString(pState->sXaxisLabel.c_str(), -1, p_font, layoutRect, &text_rect);

		if (true)
		{
			if (text_rect.Width < layoutRect.Width)
				text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
		}

		layoutRect.X = static_cast<Gdiplus::REAL>(rectChartControl.left + 4);
		layoutRect.Y = static_cast<Gdiplus::REAL>(rectChartControl.bottom - 4) - text_rect.Height;
		layoutRect.Height = text_rect.Height;

		// align the text rectangle to the layout rectangle
		liblec::cui::gui_raw::cui_rawImpl::align_text(text_rect, layoutRect,
			liblec::cui::gui_raw::cui_raw::textAlignment::middle);

		if (design)
		{
			color.SetFromCOLORREF(RGB(240, 240, 240));
			Gdiplus::SolidBrush brush(color);
			graphics.FillRectangle(&brush, layoutRect);

			color.SetFromCOLORREF(RGB(230, 230, 230));
			brush.SetColor(color);
			graphics.FillRectangle(&brush, text_rect);
		}

		graphics.DrawString(pState->sXaxisLabel.c_str(),
			-1, p_font, text_rect, &format, &text_brush);

		delete p_font;
		p_font = nullptr;

		bottom = layoutRect.GetTop();
	}

	Gdiplus::REAL left = 0.0f;
	
	// label y-axis
	{
		Gdiplus::RectF layoutRect;
		layoutRect.X = static_cast<Gdiplus::REAL>(rectChartControl.left + 4);
		layoutRect.Y = top + 4.0f;
		layoutRect.Width = 0;
		layoutRect.Height = 0;	// make 0 so measureString can figure it out

		Gdiplus::Font* p_font = new Gdiplus::Font(&Gdiplus::FontFamily(pState->sFontName.c_str()), 9);

		if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
		{
			delete p_font;
			p_font = nullptr;
			p_font = new Gdiplus::Font(pState->sFontName.c_str(),
				9, Gdiplus::Status::Ok, Gdiplus::UnitPoint, &pState->d->m_font_collection);
		}

		color.SetFromCOLORREF(RGB(0, 0, 0));
		Gdiplus::SolidBrush text_brush(color);

		Gdiplus::StringFormat format;
		format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
		format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);
		format.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);

		// measure text rectangle
		Gdiplus::RectF text_rect;
		graphics.MeasureString(pState->sYaxisLabel.c_str(), -1, p_font, layoutRect, &text_rect);

		if (true)
		{
			if (text_rect.Width < layoutRect.Width)
				text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
		}

		// change dimensions
		Gdiplus::REAL width = text_rect.Width;
		Gdiplus::REAL height = text_rect.Height;

		layoutRect.Height = bottom - top - 4.0f * 2.0f;
		layoutRect.Width = height;

		text_rect.Height = width;
		text_rect.Width = height;

		// align the text rectangle to the layout rectangle
		liblec::cui::gui_raw::cui_rawImpl::align_text(text_rect, layoutRect,
			liblec::cui::gui_raw::cui_raw::textAlignment::middle);

		if (design)
		{
			color.SetFromCOLORREF(RGB(240, 240, 240));
			Gdiplus::SolidBrush brush(color);
			graphics.FillRectangle(&brush, layoutRect);

			color.SetFromCOLORREF(RGB(230, 230, 230));
			brush.SetColor(color);
			graphics.FillRectangle(&brush, text_rect);
		}
		
		// figure out center
		Gdiplus::REAL x = layoutRect.X + (layoutRect.Width / 2);
		Gdiplus::REAL y = layoutRect.Y + (layoutRect.Height / 2);

		graphics.TranslateTransform(x, y);		// set rotation point
		graphics.RotateTransform(-90);			// rotate text
		graphics.TranslateTransform(-x, -y);	// reset translate transform

		// Draw string centered in x, y
		Gdiplus::PointF pt(x - width / 2, y - height / 2);
		graphics.DrawString(pState->sYaxisLabel.c_str(),
			(INT)pState->sYaxisLabel.length(), p_font, pt, &text_brush);
		graphics.ResetTransform();

		left = layoutRect.GetRight();
	}

	Gdiplus::REAL right = 0.0f;

	// draw labels
	Gdiplus::RectF rectLabels;
	{
		Gdiplus::REAL width = static_cast<Gdiplus::REAL>(iWidth / 8);

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

	// calculate chart rectangle
	RECT rectChart;
	rectChart.left = static_cast<LONG>(left);
	rectChart.right = static_cast<LONG>(right);
	rectChart.top = static_cast<LONG>(top) + 8;
	rectChart.bottom = static_cast<LONG>(bottom);

	int marker_Y_width = iWidth / 20;
	marker_Y_width = marker_Y_width > 50 ? marker_Y_width : 50;

	int marker_X_height = iHeight / 20;
	marker_X_height = marker_X_height > 20 ? marker_X_height : 20;

	rectChartControl = rectChart;
	rectChartControl.right = absolute_right;

	rectChart.left += marker_Y_width;
	rectChart.bottom -= marker_X_height;

	// indicate actual chart shall sit
	if (design)
	{
		Gdiplus::RectF rectChart_ = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rectChart);

		color.SetFromCOLORREF(RGB(250, 250, 250));
		Gdiplus::SolidBrush brush(color);
		graphics.FillRectangle(&brush, rectChart_);
	}

	// draw chart

	Gdiplus::Font* p_font = new Gdiplus::Font(&Gdiplus::FontFamily(pState->sFontName.c_str()),
		static_cast<Gdiplus::REAL>(pState->iFontSize));

	if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
	{
		delete p_font;
		p_font = nullptr;
		p_font = new Gdiplus::Font(pState->sFontName.c_str(),
			static_cast<Gdiplus::REAL>(pState->iFontSize), Gdiplus::FontStyle::FontStyleRegular,
			Gdiplus::UnitPoint, &pState->d->m_font_collection);
	}


	color.SetFromCOLORREF(RGB(0, 0, 0));
	Gdiplus::SolidBrush text_brush(color);

	Gdiplus::StringFormat format;
	format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
	format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);
	format.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);

	// calculate lowest and highest value
	std::vector<cui_raw::barChartData> vValues = pState->vValues;

	// 2. Draw Axes
	if (vValues.size() > 0)
	{
		double dMin = 0;
		double dMax = 0;

		bool bFirst = true;
		for (auto it : vValues)
		{
			if (bFirst)
			{
				bFirst = false;

				dMin = it.dValue;
				dMax = it.dValue;
			}
			else
			{
				dMin = std::fmin(it.dValue, dMin);
				dMax = std::fmax(it.dValue, dMax);
			}
		}

		double dxSep = double(rectChart.right - rectChart.left) / (double)vValues.size();

		// calculate the most suitable chart parameters
		chartParams params;

		if (!calcChartParams(dMin, dMax, params) || !pState->bAutoScale)
		{
			// Function failed. Fall back to defaults.
			params.iLowerLimit = (int)pState->iLowerLimit;			// truncate
			params.iUpperLimit = (int)(pState->iUpperLimit + 0.5);	// round off
			params.iNumberOfLines = 10;
		}

		double dySep = double(rectChart.bottom - rectChart.top) / params.iNumberOfLines;
		int iLowerLimit = params.iLowerLimit;
		int iUpperLimit = params.iUpperLimit;

		// draw y-axis markers
		for (int i = 0; i <= params.iNumberOfLines; i++)
		{
			double dY = (double)rectChart.bottom - (i * dySep);

			color.SetFromCOLORREF(RGB(200, 200, 200));
			Gdiplus::Pen pen(color, 1.0f);
			pen.SetAlignment(Gdiplus::PenAlignment::PenAlignmentCenter);

			Gdiplus::PointF point_1, point_2;
			point_1.X = static_cast<Gdiplus::REAL>(rectChart.left);
			point_1.Y = static_cast<Gdiplus::REAL>(dY);
			point_2.X = static_cast<Gdiplus::REAL>(rectChart.right);
			point_2.Y = static_cast<Gdiplus::REAL>(dY);
			graphics.DrawLine(&pen, point_1, point_2);

			Gdiplus::RectF layoutRect;
			layoutRect.X = static_cast<Gdiplus::REAL>(rectChartControl.left);
			layoutRect.Y = static_cast<Gdiplus::REAL>(dY);
			layoutRect.Width = static_cast<Gdiplus::REAL>(marker_Y_width);
			layoutRect.Height = static_cast<Gdiplus::REAL>(dySep);

			double dMarker = (double)iLowerLimit + (double)i * (double)(iUpperLimit - iLowerLimit) / (double)params.iNumberOfLines;

			// measure text rectangle
			Gdiplus::RectF text_rect;
			graphics.MeasureString(round_off(dMarker, 1).c_str(), -1, p_font, layoutRect, &text_rect);

			if (true)
			{
				if (text_rect.Width < layoutRect.Width)
					text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
			}

			layoutRect.Height = text_rect.Height;

			layoutRect.Y = static_cast<Gdiplus::REAL>(dY) - layoutRect.Height / 2.0f;

			// align the text rectangle to the layout rectangle
			liblec::cui::gui_raw::cui_rawImpl::align_text(text_rect, layoutRect,
				liblec::cui::gui_raw::cui_raw::textAlignment::middle);

			if (design)
			{
				color.SetFromCOLORREF(RGB(240, 240, 240));
				Gdiplus::SolidBrush brush(color);
				graphics.FillRectangle(&brush, layoutRect);

				color.SetFromCOLORREF(RGB(230, 230, 230));
				brush.SetColor(color);
				graphics.FillRectangle(&brush, text_rect);
			}

			// draw text
			graphics.DrawString(round_off(dMarker, 1).c_str(),
				-1, p_font, text_rect, &format, &text_brush);
		}

		if (!pState->bInfoCaptured)
			pState->chartBarsInfo.clear();

		// figure out if every label fits (important for aesthetics)
		bool bFits = true;
		for (int i = 0; i < (int)vValues.size(); i++)
		{
			int iX = rectChart.left + int(i * dxSep);

			RECT rect;
			rect.bottom = rectChart.bottom + (rectChartControl.bottom - rectChart.bottom) / 2;
			rect.top = rectChart.bottom + (rectChartControl.bottom - rectChart.bottom) / 8;
			rect.left = iX;
			rect.right = rect.left + int(dxSep);

			SIZE size;
			GetTextExtentPoint32(hdc, std::to_wstring(vValues[i].iNumber).c_str(),
				lstrlen(std::to_wstring(vValues[i].iNumber).c_str()), &size);

			if (double(size.cx) >= (dxSep / 1.5))
			{
				bFits = false;
				break;
			}
		}

		// draw x-axis markers and bars
		for (int i = 0; i < (int)vValues.size(); i++)
		{
			int iX = rectChart.left + int(i * dxSep);

			RECT rect;
			rect.bottom = rectChartControl.bottom;
			rect.top = rectChart.bottom;
			rect.left = iX;
			rect.right = rect.left + int(dxSep);

			// draw marker
			if (bFits)
			{
				Gdiplus::RectF layoutRect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rect);

				// measure text rectangle
				Gdiplus::RectF text_rect;
				graphics.MeasureString(std::to_wstring(vValues[i].iNumber).c_str(), -1, p_font, layoutRect, &text_rect);

				if (true)
				{
					if (text_rect.Width < layoutRect.Width)
						text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
				}

				layoutRect.Height = text_rect.Height;

				// align the text rectangle to the layout rectangle
				liblec::cui::gui_raw::cui_rawImpl::align_text(text_rect, layoutRect,
					liblec::cui::gui_raw::cui_raw::textAlignment::middle);

				if (design)
				{
					color.SetFromCOLORREF(RGB(240, 240, 240));
					Gdiplus::SolidBrush brush(color);
					graphics.FillRectangle(&brush, layoutRect);

					color.SetFromCOLORREF(RGB(230, 230, 230));
					brush.SetColor(color);
					graphics.FillRectangle(&brush, text_rect);
				}

				// draw text
				graphics.DrawString(std::to_wstring(vValues[i].iNumber).c_str(),
					-1, p_font, text_rect, &format, &text_brush);
			}

			// draw bar chart
			RECT rectBar;
			rectBar.bottom = rectChart.bottom;

			double dValue = vValues[i].dValue;

			double dAboveLower = dValue - iLowerLimit;

			double dRatioOfRange = dAboveLower / (iUpperLimit - iLowerLimit);

			// don't permit negative
			if (dRatioOfRange < 0)
				dRatioOfRange = 0;

			// don't permit excess
			if (dRatioOfRange > 1)
				dRatioOfRange = 1;

			rectBar.top = double2int((double)rectBar.bottom - dRatioOfRange * double(rectChart.bottom - rectChart.top));

			rectBar.left = double2int((double)rect.left + (dxSep / 4));
			rectBar.right = double2int((double)rect.right - (dxSep / 4));

			double dPass = 50;
			bool bFlagFail = false;	// whether to flag marks less than dPass

			if (!pState->bInfoCaptured)
			{
				// capture chart bar info
				cui_rawImpl::chartBarInfo m_chartbarinfo;

				m_chartbarinfo.rect = rectBar;

				m_chartbarinfo.sChartInfo = (vValues[i].sLabel + _T(" - ") + round_off(vValues[i].dValue, 1)).c_str();

				pState->chartBarsInfo.push_back(m_chartbarinfo);
			}

			// update bar rect
			pState->chartBarsInfo[i].rect = rectBar;

			COLORREF clr;

			if (vValues[i].dValue >= dPass || !bFlagFail)
				clr = pState->vValues[i].clrBar;
			else
				clr = RGB(250, 150, 70);

			if (pState->chartBarsInfo[i].bHot)
				clr = clrDarken(pState->vValues[i].clrBar, 40);

			// paint bar
			CBrush hbr(clr);
			FillRect(hdc, &rectBar, hbr.get());
		}

		pState->bInfoCaptured = true;
	}

	delete p_font;
	p_font = nullptr;

	const int iItemHeight = 18;
	const int iMargin = 2;

	// check if items fit in the given space
	if (((iItemHeight + iMargin) * static_cast<int>(vValues.size())) > (rectChart.bottom - rectChart.top))
	{
		for (auto &it : pState->chartBarsInfo)
			it.rcLabel = { 0 };

		return;
	}

	int iLeftBorder = static_cast<int>(rectLabels.GetLeft());
	int iBottom = static_cast<int>(rectLabels.GetBottom());

	size_t m_x = vValues.size();
	for (auto it : reverse(vValues))
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

		COLORREF clr = it.clrBar;

		if (bHot)
		{
			if (bPressed)
				clr = clrDarken(it.clrBar, 40);
			else
				clr = clrDarken(it.clrBar, 25);
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

		std::basic_string<TCHAR> sText = it.sLabel;

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
					7, Gdiplus::FontStyle::FontStyleRegular, Gdiplus::UnitPoint, &pState->d->m_font_collection);
			}

			color.SetFromCOLORREF(RGB(0, 0, 0));
			Gdiplus::SolidBrush text_brush(color);

			// measure text rectangle
			Gdiplus::RectF text_rect;
			graphics.MeasureString(sText.c_str(), -1, p_font, layoutRect, &text_rect);

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

			format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingWord);

			// draw text
			graphics.DrawString(sText.c_str(),
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
} // DrawChart

LRESULT CALLBACK cui_rawImpl::BarChartControlProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	cui_rawImpl::BarChartControl* pControl = reinterpret_cast<cui_rawImpl::BarChartControl*>(ptr);

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
		DrawChart(hWnd, hdc, pControl);
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
} // BarChartControlProc
