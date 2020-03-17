/*
** DrawRoundRect.cpp - draw round rectangle implementation
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

#include "DrawRoundRect.h"

using namespace Gdiplus;

void GetRoundRectPath(GraphicsPath *pPath, Rect r, int dia)
{
	// diameter can't exceed width or height
	if (dia > r.Width)    dia = r.Width;
	if (dia > r.Height)    dia = r.Height;

	// define a corner 
	Rect Corner(r.X, r.Y, dia, dia);

	// begin path
	pPath->Reset();

	// top left
	pPath->AddArc(Corner, 180, 90);

	// top right
	Corner.X += (r.Width - dia - 1);
	pPath->AddArc(Corner, 270, 90);

	// bottom right
	Corner.Y += (r.Height - dia - 1);
	pPath->AddArc(Corner, 0, 90);

	// bottom left
	Corner.X -= (r.Width - dia - 1);
	pPath->AddArc(Corner, 90, 90);

	// end path
	pPath->CloseFigure();
}

void DrawRoundRect(Graphics* pGraphics, Rect r, Color color, int radius, int width)
{
	int dia = 2 * radius;

	// set to pixel mode
	int oldPageUnit = pGraphics->SetPageUnit(UnitPixel);

	// define the pen
	Pen pen(color, 1);
	pen.SetAlignment(PenAlignmentCenter);

	// get the corner path
	GraphicsPath path;

	// get path
	GetRoundRectPath(&path, r, dia);

	// draw the round rect
	pGraphics->DrawPath(&pen, &path);

	// if width > 1
	for (int i = 1; i<width; i++)
	{
		// left stroke
		r.Inflate(-1, 0);
		// get the path
		GetRoundRectPath(&path, r, dia);

		// draw the round rect
		pGraphics->DrawPath(&pen, &path);

		// up stroke
		r.Inflate(0, -1);

		// get the path
		GetRoundRectPath(&path, r, dia);

		// draw the round rect
		pGraphics->DrawPath(&pen, &path);
	}

	// restore page unit
	pGraphics->SetPageUnit((Unit)oldPageUnit);
}

void FillRoundRect(Graphics* pGraphics, LinearGradientBrush* pBrush, Rect r, Color border, int radius)
{
	int dia = 2 * radius;

	// set to pixel mode
	int oldPageUnit = pGraphics->SetPageUnit(UnitPixel);

	// define the pen
	Pen pen(border, 1);
	pen.SetAlignment(PenAlignmentCenter);

	// get the corner path
	GraphicsPath path;

	// get path
	GetRoundRectPath(&path, r, dia);

	// fill
	pGraphics->FillPath(pBrush, &path);

	// draw the border last so it will be on top
	pGraphics->DrawPath(&pen, &path);

	// restore page unit
	pGraphics->SetPageUnit((Unit)oldPageUnit);
}

void FillRoundRect(Graphics* pGraphics, Rect r, Color color1, Color color2, int radius)
{
	//SolidBrush sbr(color);
	LinearGradientBrush sbr(r, color1, color2, Gdiplus::LinearGradientMode::LinearGradientModeVertical);
	FillRoundRect(pGraphics, &sbr, r, color1, radius);
}

void DrawRoundRect(Gdiplus::Graphics &graphics, int x1, int y1, int x2, int y2, int radius, COLORREF color1, COLORREF color2, int iBorderWidth, bool bFill)
{
	graphics.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
	Gdiplus::Color c;
	c.SetFromCOLORREF(color1);

	Color c2;
	c2.SetFromCOLORREF(color2);

	Gdiplus::Rect rect;
	rect.X = x1;
	rect.Y = y1;
	rect.Width = x2 - x1;
	rect.Height = y2 - y1;

	if (bFill)
		FillRoundRect(&graphics, rect, c, c2, radius);
	else
		DrawRoundRect(&graphics, rect, c, radius, iBorderWidth);

	return;
} // DrawRoundRect
