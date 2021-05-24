/*
** Image.cpp - image implementation
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

/// <summary>
/// Fit rectangle into another.
/// </summary>
/// 
/// <param name="rcIn">
/// The rectangle to fit into another.
/// </param>
/// 
/// <param name="rectTarget">
/// The rectangle to fit to.
/// </param>
/// 
/// <param name="rectOut">
/// The resized rectangle which properly fits.
/// </param>
void fitRect(
	const RECT &rcIn,
	const RECT &rectTarget,
	RECT &rectOut
)
{
	bool bStretch = false;
	bool bEnlargeIfSmaller = false;
	bool bCenter = true;

	int iWidth = rectTarget.right - rectTarget.left;
	int iHeight = rectTarget.bottom - rectTarget.top;

	/*
	** deduce old height and ratio
	*/
	UINT old_height = rcIn.bottom - rcIn.top;
	UINT old_width = rcIn.right - rcIn.left;
	double ratio = ((double)old_width) / ((double)old_height);

	if (bEnlargeIfSmaller == false)
	{
		if ((int)old_width < iWidth && (int)old_height < iHeight)
		{
			// both sides of the image are smaller than the target dimensions, preserve size
			iWidth = old_width;
			iHeight = old_height;
		}
	}

	/*
	** save target dimensions of picture control
	*/
	const int control_w = iWidth;
	const int control_h = iHeight;

	if (ratio == 1)
	{
		if (!bStretch)
		{
			if (iWidth > iHeight)
			{
				/*
				** landscape picture control
				*/
				iWidth = iHeight;
			}
			else
			{
				/*
				** portrait picture control
				*/
				iHeight = iWidth;
			}
		}
	}
	else
	{
		if (!bStretch)
		{
			/*
			** adjust either new width or height to keep aspect ratio
			*/
			if (old_width > old_height)
			{
				/*
				** old width is greater than old height
				** adjust new height using new width to keep aspect ratio
				*/
				iHeight = (int)(iWidth / ratio);

				if (iHeight > control_h)
				{
					// new width is greater than target width, adjust it accordingly
					iHeight = control_h;
					iWidth = (int)(iHeight * ratio);
				}
			}
			else
			{
				/*
				** old height is greater than old width
				** adjust new width using new height to keep aspect ratio
				*/
				iWidth = (int)(iHeight * ratio);

				if (iWidth > control_w)
				{
					// new width is greater than target width, adjust it accordingly
					iWidth = control_w;
					iHeight = (int)(iWidth / ratio);
				}
			}
		}
	}

	RECT m_rectOut;
	m_rectOut.left = 0;
	m_rectOut.top = 0;

	if (bCenter)
	{
		// set x-position for painting image on center
		m_rectOut.left = ((rectTarget.right - rectTarget.left) - iWidth) / 2;

		// set y-position for painting image on center
		m_rectOut.top = ((rectTarget.bottom - rectTarget.top) - iHeight) / 2;
	}

	m_rectOut.left += rectTarget.left;
	m_rectOut.top += rectTarget.top;

	m_rectOut.right = m_rectOut.left + iWidth;
	m_rectOut.bottom = m_rectOut.top + iHeight;

	rectOut = m_rectOut;

	return;
} // fitRect

static bool design = false;

LRESULT CALLBACK cui_rawImpl::ImageProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	cui_rawImpl::ImageControl* pControl = reinterpret_cast<cui_rawImpl::ImageControl*>(ptr);

	switch (msg)
	{
	case WM_PAINT:
		if (pControl->bDescriptive)
		{
			bool bTextOnLeft = false;
			if (pControl->textPlacement == cui_raw::imageTextPlacement::left ||
				pControl->textPlacement == cui_raw::imageTextPlacement::leftbottom ||
				pControl->textPlacement == cui_raw::imageTextPlacement::lefttop
				)
				bTextOnLeft = true;	// set text onto the left if any of the "left" options is selected

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

			Gdiplus::Graphics graphics(hdc);
			Gdiplus::Color color;
			color.SetFromCOLORREF(pControl->d->m_clrBackground);
			graphics.Clear(color);

			// draw boarder and fill area (optional)
			COLORREF clrBorder = pControl->clrBorder;

			if (pControl->bHot)
				clrBorder = pControl->clrBorderHot;

			if (!IsWindowEnabled(hWnd))
			{
				if (pControl->d->m_clrBackground != pControl->clrBorder)
					clrBorder = pControl->d->m_clrDisabled;
				else
					clrBorder = pControl->d->m_clrBackground;
			}

			color.SetFromCOLORREF(clrBorder);
			Gdiplus::Pen pen(color, 1.0f);

			COLORREF clr_bk = pControl->clrBackground;

			if (pControl->bHot)
				clr_bk = pControl->clrBackgroundHot;

			if (!pControl->bImageOnlyTightFit)
			{
				Gdiplus::RectF rect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rcClient);
				Gdiplus::SolidBrush brush(color);
				graphics.FillRectangle(&brush, rect);

				rect.Inflate(-1.0f, -1.0f);
				color.SetFromCOLORREF(clr_bk);
				brush.SetColor(color);
				graphics.FillRectangle(&brush, rect);
			}

			RECT rcBar = rcClient;

			// compute bar placement
			rcBar.top += int(1.0 * pControl->d->m_DPIScale + 0.5);
			rcBar.bottom -= int(1.0 * pControl->d->m_DPIScale + 0.5);

			if (bTextOnLeft)
				rcBar.left = rcBar.right - int(3.0 * pControl->d->m_DPIScale + 0.5);
			else
				rcBar.right = rcBar.left + int(3.0 * pControl->d->m_DPIScale + 0.5);

			pControl->rcBar = rcBar;

			// draw bar (uses text color when cold, and border's hot color when hot)
			if (pControl->bButtonBar && !pControl->bImageOnlyTightFit)
			{
				Gdiplus::RectF rect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rcBar);

				color.SetFromCOLORREF(pControl->clrBar);
				Gdiplus::SolidBrush brush(color);
				graphics.FillRectangle(&brush, rect);
			}

			// TO-DO: skip text issues entirely if there is no text to avoid unnecessary overhead
			COLORREF clrText = pControl->clrText;

			if (pControl->bHot && pControl->bChangeColor)
				clrText = pControl->clrTextHot;

			if (!IsWindowEnabled(hWnd))
				clrText = clrDarken(pControl->d->m_clrDisabled, 30);	// TO-DO: remove magic number

			Gdiplus::Font* p_font = new Gdiplus::Font(&Gdiplus::FontFamily(pControl->sFontName.c_str()),
				static_cast<Gdiplus::REAL>(11.0 * pControl->iFontSize / 9));

			if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
			{
				delete p_font;
				p_font = nullptr;
				p_font = new Gdiplus::Font(pControl->sFontName.c_str(),
					static_cast<Gdiplus::REAL>(11.0 * pControl->iFontSize / 9), Gdiplus::FontStyle::FontStyleRegular,
					Gdiplus::UnitPoint, &pControl->d->m_font_collection);
			}

			// measure text rectangle
			Gdiplus::RectF text_rect;
			graphics.MeasureString((pControl->sText + L" ").c_str(), -1, p_font, text_rect, &text_rect);

			// compute text placement
			const int iMargin = int(2.0 * pControl->d->m_DPIScale + 0.5);
			RECT rc = rcClient;

			rc.top += iMargin;
			rc.bottom = rc.top + static_cast<LONG>(text_rect.Height);

			Gdiplus::RectF layoutRect;

			if (bTextOnLeft)
			{
				rc.left = rcClient.left + iMargin;
				rc.right = rcClient.right - iMargin - int((double(pControl->imageSize.cx) * pControl->d->m_DPIScale) + 0.5) - 10;

				// align text to top right
				layoutRect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rc);
				align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::topright);
			}
			else
			{
				rc.right = rcClient.right - iMargin;
				rc.left = rcClient.left + iMargin + int((double(pControl->imageSize.cx) * pControl->d->m_DPIScale) + 0.5) + 10;
				
				// align text to top left
				layoutRect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rc);
				align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::topleft);
			}

			pControl->rcText = rc;

			// compute image position
			rc.top = rcClient.top + iMargin;
			rc.bottom = rc.top + int((double(pControl->imageSize.cy) * pControl->d->m_DPIScale) + 0.5);

			if (bTextOnLeft)
			{
				rc.right = rcBar.left - iMargin;
				rc.left = rc.right - int((double(pControl->imageSize.cx) * pControl->d->m_DPIScale) + 0.5);
			}
			else
			{
				rc.left = rcBar.right + iMargin;
				rc.right = rc.left + int((double(pControl->imageSize.cx) * pControl->d->m_DPIScale) + 0.5);
			}

			if (pControl->bImageOnlyTightFit)
				rc = rcClient;

			if (pControl->iPNGResource)
			{
				// load image created from PNG resource
				if (!pControl->m_pDisplaybitmap && pControl->GdiplusBitmap_res)	// load only once
					pControl->m_pDisplaybitmap = ResizeGdiplusBitmap(pControl->GdiplusBitmap_res, rc, false, Quality::high, false, true, pControl->rcImage);
			}
			else
			{
				// load image created from file
				if (!pControl->m_pDisplaybitmap && pControl->GdiplusBitmap)	// load only once
					pControl->m_pDisplaybitmap = ResizeGdiplusBitmap(pControl->GdiplusBitmap, rc, false, Quality::high, false, true, pControl->rcImage);
			}

			// capture active rect
			pControl->rcActive = rcClient;

			if (!pControl->bImageOnlyTightFit)
			{
				// if orientation has text on the left or right, make the image move to the end and move the text right next to the image
				int iHeight = pControl->rcImage.bottom - pControl->rcImage.top;
				pControl->rcImage.top = rcClient.top + iMargin + int(5.0 * pControl->d->m_DPIScale + 0.5);
				pControl->rcImage.bottom = pControl->rcImage.top + iHeight;

				if (bTextOnLeft)
					pControl->rcActive.left = pControl->rcText.left;
				else
					pControl->rcActive.right = pControl->rcText.right;
			}

			// draw text
			if (!pControl->bImageOnlyTightFit)
			{
				{
					Gdiplus::StringFormat format;
					format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
					format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);
					format.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);

					if (design)
					{
						Gdiplus::Color color;
						color.SetFromCOLORREF(RGB(240, 240, 240));
						Gdiplus::SolidBrush brush(color);
						graphics.FillRectangle(&brush, layoutRect);

						color.SetFromCOLORREF(RGB(230, 230, 230));
						brush.SetColor(color);
						graphics.FillRectangle(&brush, text_rect);
					}

					// draw text
					color.SetFromCOLORREF(clrText);
					Gdiplus::SolidBrush text_brush(color);
					graphics.DrawString(pControl->sText.c_str(),
						-1, p_font, text_rect, &format, &text_brush);

					delete p_font;
					p_font = nullptr;
				}

				{
					Gdiplus::StringFormat format;
					format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
					format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);

					Gdiplus::Font* p_font_description = new Gdiplus::Font(&Gdiplus::FontFamily(pControl->sFontNameDescription.c_str()),
						static_cast<Gdiplus::REAL>(pControl->iFontSize));

					if (p_font_description->GetLastStatus() != Gdiplus::Status::Ok)
					{
						delete p_font_description;
						p_font_description = nullptr;
						p_font_description = new Gdiplus::Font(pControl->sFontNameDescription.c_str(),
							static_cast<Gdiplus::REAL>(pControl->iFontSize), Gdiplus::FontStyle::FontStyleRegular,
							Gdiplus::UnitPoint, &pControl->d->m_font_collection);
					}

					RECT rcDescription = pControl->rcText;
					rcDescription.top = rcDescription.bottom;
					rcDescription.bottom = rcClient.bottom - iMargin;

					if (bTextOnLeft)
					{
						// align text to top right
					}
					else
					{
						// align text to top left
					}

					pControl->rcText.bottom = rcDescription.bottom;	// include descriptive text in the rcText variable

					Gdiplus::RectF rect_description = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rcDescription);

					if (design)
					{
						color.SetFromCOLORREF(RGB(230, 230, 230));
						Gdiplus::SolidBrush brush(color);
						graphics.FillRectangle(&brush, rect_description);
					}

					color.SetFromCOLORREF(clrLighten(clrText, 40));
					Gdiplus::SolidBrush text_brush(color);
					graphics.DrawString(pControl->sDescription.c_str(),
						-1, p_font_description, rect_description, &format, &text_brush);

					delete p_font_description;
					p_font_description = nullptr;
				}
			}

			// draw image
			if (pControl->m_pDisplaybitmap)
			{
				Gdiplus::Bitmap* original = new Gdiplus::Bitmap(pControl->rcImage.right - pControl->rcImage.left, pControl->rcImage.bottom - pControl->rcImage.top, pControl->m_pDisplaybitmap->GetPixelFormat());

				Gdiplus::Graphics graphics(original);

				if (!pControl->bChangeColor)
				{
					if (IsWindowEnabled(hWnd))
						graphics.DrawImage(pControl->m_pDisplaybitmap, 0, 0);
					else
					{
						// change bitmap color to greyscale

						// create a color matrix
						Gdiplus::ColorMatrix clrMatrix = {
							{
								{ .3f, .3f, .3f, 0, 0 },
								{ .6f, .6f, .6f, 0, 0 },
								{ .1f, .1f, .1f, 0, 0 },
								{ 0, 0, 0, 1, 0 },
								{ 0, 0, 0, 0, 1 }
							}
						};

						Gdiplus::ImageAttributes attributes;
						attributes.SetColorMatrix(&clrMatrix,
							Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);

						Gdiplus::RectF destRect;
						destRect.X = 0;
						destRect.Y = 0;
						destRect.Width = (Gdiplus::REAL)(pControl->rcImage.right - pControl->rcImage.left);
						destRect.Height = (Gdiplus::REAL)(pControl->rcImage.bottom - pControl->rcImage.top);
						graphics.DrawImage(pControl->m_pDisplaybitmap, destRect, 0, 0, (Gdiplus::REAL)(original->GetWidth()), (Gdiplus::REAL)(original->GetHeight()), Gdiplus::UnitPixel, &attributes);
					}
				}
				else
				{
					// change bitmap color
					Gdiplus::Color c;

					if (!pControl->bHot)
						c.SetFromCOLORREF(pControl->clrImage);
					else
						c.SetFromCOLORREF(pControl->clrImageHot);

					if (!IsWindowEnabled(hWnd))
						c.SetFromCOLORREF(pControl->d->m_clrDisabled);

					// create a color matrix
					Gdiplus::ColorMatrix clrMatrix = {
						{
							{ 0, 0, 0, 0, 0 },
							{ 0, 0, 0, 0, 0 },
							{ 0, 0, 0, 0, 0 },
							{ 0, 0, 0, 1, 0 },
							{ c.GetR() / 255.0f,
							c.GetG() / 255.0f,
							c.GetB() / 255.0f,
							0, 1 }
						}
					};

					Gdiplus::ImageAttributes attributes;
					attributes.SetColorMatrix(&clrMatrix);

					Gdiplus::RectF destRect;
					destRect.X = 0;
					destRect.Y = 0;
					destRect.Width = (Gdiplus::REAL)(pControl->rcImage.right - pControl->rcImage.left);
					destRect.Height = (Gdiplus::REAL)(pControl->rcImage.bottom - pControl->rcImage.top);
					graphics.DrawImage(pControl->m_pDisplaybitmap, destRect, 0, 0, (Gdiplus::REAL)(original->GetWidth()), (Gdiplus::REAL)(original->GetHeight()), Gdiplus::UnitPixel, &attributes);
				}

				Gdiplus::Graphics graphics_out(hdc);
				graphics_out.DrawImage(original, (Gdiplus::REAL)pControl->rcImage.left, (Gdiplus::REAL)pControl->rcImage.top);

				if (original)
				{
					delete original;
					original = NULL;
				}
			}

			BitBlt(dc, 0, 0, cx, cy, hdc, 0, 0, SRCCOPY);

			EndPaint(hWnd, &ps);

			SelectBitmap(hdc, hbmOld);
			DeleteDC(hdc);
		}
		else
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

			Gdiplus::Graphics graphics(hdc);
			Gdiplus::Color color;
			color.SetFromCOLORREF(pControl->d->m_clrBackground);
			graphics.Clear(color);

			// draw boarder and fill area (optional)
			COLORREF clrBorder = pControl->clrBorder;

			if (pControl->bHot)
				clrBorder = pControl->clrBorderHot;

			if (!IsWindowEnabled(hWnd))
			{
				if (pControl->d->m_clrBackground != pControl->clrBorder)
					clrBorder = pControl->d->m_clrDisabled;
				else
					clrBorder = pControl->d->m_clrBackground;
			}

			color.SetFromCOLORREF(clrBorder);

			COLORREF clr_bk = pControl->clrBackground;

			if (pControl->bHot)
				clr_bk = pControl->clrBackgroundHot;

			//if (!pControl->bImageOnlyTightFit)
			{
				Gdiplus::RectF rect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rcClient);
				Gdiplus::SolidBrush brush(color);
				graphics.FillRectangle(&brush, rect);

				rect.Inflate(-1.0f, -1.0f);
				color.SetFromCOLORREF(clr_bk);
				brush.SetColor(color);
				graphics.FillRectangle(&brush, rect);
			}

			RECT rcBar = rcClient;

			// compute bar placement
			switch (pControl->textPlacement)
			{
			case cui_raw::bottom:
				rcBar.left += 1;
				rcBar.right -= 1;
				rcBar.bottom = rcBar.top + 3;
				break;

			case cui_raw::top:
				rcBar.left += 1;
				rcBar.right -= 1;
				rcBar.top = rcBar.bottom - 3;
				break;

			case cui_raw::right:
			case cui_raw::righttop:
			case cui_raw::rightbottom:
				rcBar.top += 1;
				rcBar.bottom -= 1;
				rcBar.right = rcBar.left + 3;
				break;

			case cui_raw::left:
			case cui_raw::lefttop:
			case cui_raw::leftbottom:
				rcBar.top += 1;
				rcBar.bottom -= 1;
				rcBar.left = rcBar.right - 3;
				break;

			default:
				// default to bottom
				rcBar.left += 1;
				rcBar.right -= 1;
				rcBar.bottom = rcBar.top + 3;
				break;
			}

			pControl->rcBar = rcBar;

			// draw bar (uses text color when cold, and border's hot color when hot)
			if (pControl->bButtonBar && !pControl->bImageOnlyTightFit)
			{
				Gdiplus::RectF rect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(rcBar);

				color.SetFromCOLORREF(pControl->clrBar);
				Gdiplus::SolidBrush brush(color);
				graphics.FillRectangle(&brush, rect);
			}

			// TO-DO: skip text issues entirely if there is no text to avoid unnecessary overhead
			COLORREF clrText = pControl->clrText;

			if (pControl->bHot && pControl->bChangeColor)
				clrText = pControl->clrTextHot;

			if (!IsWindowEnabled(hWnd))
				clrText = clrDarken(pControl->d->m_clrDisabled, 30);	// TO-DO: remove magic number

			Gdiplus::Font* p_font = new Gdiplus::Font(&Gdiplus::FontFamily(pControl->sFontName.c_str()),
				static_cast<Gdiplus::REAL>(pControl->iFontSize));

			if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
			{
				delete p_font;
				p_font = nullptr;
				p_font = new Gdiplus::Font(pControl->sFontName.c_str(),
					static_cast<Gdiplus::REAL>(pControl->iFontSize), Gdiplus::FontStyle::FontStyleRegular,
					Gdiplus::UnitPoint, &pControl->d->m_font_collection);
			}

			// measure text rectangle
			Gdiplus::RectF text_rect;
			graphics.MeasureString((pControl->sText + L" ").c_str(), -1, p_font, text_rect, &text_rect);

			const int iMargin = int(1.0 * pControl->d->m_DPIScale + 0.5);

			if (true)
			{
				// image size
				SIZE imageSize;

				Gdiplus::Bitmap* bmp = NULL;

				if (pControl->iPNGResource)
					bmp = pControl->GdiplusBitmap_res;
				else
					bmp = pControl->GdiplusBitmap;

				if (bmp)
				{
					imageSize.cx = (LONG)bmp->GetWidth();
					imageSize.cy = (LONG)bmp->GetHeight();
				}
				else
				{
					imageSize.cx = 0;
					imageSize.cy = 0;
				}

				// RECT to draw in
				RECT rcDraw = rcClient;

				switch (pControl->textPlacement)
				{
				case cui_raw::bottom:
					rcDraw.top = rcBar.bottom + iMargin;
					break;

				case cui_raw::top:
					rcDraw.bottom = rcBar.top - iMargin;
					break;

				case cui_raw::right:
				case cui_raw::righttop:
				case cui_raw::rightbottom:
					rcDraw.left = rcBar.right + iMargin;
					break;

				case cui_raw::left:
				case cui_raw::lefttop:
				case cui_raw::leftbottom:
					rcDraw.right = rcBar.left - iMargin;
					break;

				default:
					// default to bottom
					rcDraw.top = rcBar.bottom + iMargin;
					break;
				}

				// compute image placement
				RECT rcImage = rcDraw;
				rcImage.right = rcImage.left + imageSize.cx;
				rcImage.bottom = rcImage.top + imageSize.cy;

				// fit image
				fitRect(rcImage, rcDraw, rcImage);

				imageSize.cx = rcImage.right - rcImage.left;
				imageSize.cy = rcImage.bottom - rcImage.top;

				// compute image position
				switch (pControl->textPlacement)
				{
				case cui_raw::bottom:
					rcImage.top = rcBar.bottom + iMargin;
					rcImage.bottom = rcImage.top + imageSize.cy;
					break;

				case cui_raw::top:
					rcImage.bottom = rcBar.top - iMargin;
					rcImage.top = rcImage.bottom - imageSize.cy;
					break;

				case cui_raw::right:
				case cui_raw::righttop:
				case cui_raw::rightbottom:
					rcImage.left = rcBar.right + iMargin;
					rcImage.right = rcImage.left + imageSize.cx;
					break;

				case cui_raw::left:
				case cui_raw::lefttop:
				case cui_raw::leftbottom:
					rcImage.right = rcBar.left - iMargin;
					rcImage.left = rcImage.right - imageSize.cx;
					break;

				default:
					// default to bottom
					rcImage.top = rcBar.bottom + iMargin;
					rcImage.bottom = rcImage.top + imageSize.cy;
					break;
				}

				if (pControl->bImageOnlyTightFit)
					rcImage = rcClient;

				if (pControl->iPNGResource)
				{
					// load image created from PNG resource
					if (!pControl->m_pDisplaybitmap && pControl->GdiplusBitmap_res)	// load only once
						pControl->m_pDisplaybitmap = ResizeGdiplusBitmap(pControl->GdiplusBitmap_res, rcImage, false, Quality::high, false, true, pControl->rcImage);
				}
				else
				{
					// load image created from file
					if (!pControl->m_pDisplaybitmap && pControl->GdiplusBitmap)	// load only once
						pControl->m_pDisplaybitmap = ResizeGdiplusBitmap(pControl->GdiplusBitmap, rcImage, false, Quality::high, false, true, pControl->rcImage);
				}
			}

			// compute text placement
			RECT rc = rcClient;

			switch (pControl->textPlacement)
			{
			case cui_raw::bottom:
				rc.left += iMargin;
				rc.right -= iMargin;
				rc.bottom = rcClient.bottom - iMargin;
				rc.top = rc.bottom - static_cast<LONG>(text_rect.Height);
				break;

			case cui_raw::top:
				rc.left += iMargin;
				rc.right -= iMargin;
				rc.top = rcClient.top + iMargin;
				rc.bottom = rc.top + static_cast<LONG>(text_rect.Height);
				break;

			case cui_raw::right:
				rc.top += iMargin;
				rc.bottom -= iMargin;
				rc.right = rcClient.right - iMargin;
				rc.left = rc.right - static_cast<LONG>(text_rect.Width);
				break;

			case cui_raw::righttop:
				rc.top += iMargin;
				rc.bottom -= iMargin;
				rc.right = rcClient.right - iMargin;
				rc.left = rc.right - static_cast<LONG>(text_rect.Width);
				break;

			case cui_raw::rightbottom:
				rc.top += iMargin;
				rc.bottom -= iMargin;
				rc.right = rcClient.right - iMargin;
				rc.left = rc.right - static_cast<LONG>(text_rect.Width);
				break;

			case cui_raw::left:
				rc.top += iMargin;
				rc.bottom -= iMargin;
				rc.left = rcClient.left + iMargin;
				rc.right = rc.left + static_cast<LONG>(text_rect.Width);
				break;

			case cui_raw::lefttop:
				rc.top += iMargin;
				rc.bottom -= iMargin;
				rc.left = rcClient.left + iMargin;
				rc.right = rc.left + static_cast<LONG>(text_rect.Width);
				break;

			case cui_raw::leftbottom:
				rc.top += iMargin;
				rc.bottom -= iMargin;
				rc.left = rcClient.left + iMargin;
				rc.right = rc.left + static_cast<LONG>(text_rect.Width);
				break;

			default:
				// default to bottom
				rc.left += iMargin;
				rc.right -= iMargin;
				rc.bottom = rcClient.bottom - iMargin;
				rc.top = rc.bottom - static_cast<LONG>(text_rect.Height);
				break;
			}

			pControl->rcText = rc;

			// capture active rect
			pControl->rcActive = rcClient;

			if (!pControl->bImageOnlyTightFit)
			{
				// if orientation has text on the left or right, make the image width remain unchanged, and force text to fit in the remaining space, and move the text right next to the image
				switch (pControl->textPlacement)
				{
				case cui_raw::bottom:
					pControl->rcText.top = pControl->rcImage.bottom + iMargin;
					pControl->rcText.bottom = rcClient.bottom - iMargin;
					break;

				case cui_raw::top:
					pControl->rcText.bottom = pControl->rcImage.top - iMargin;
					pControl->rcText.top = rcClient.top + iMargin;
					break;

				case cui_raw::right:
				case cui_raw::righttop:
				case cui_raw::rightbottom:
					pControl->rcText.left = pControl->rcImage.right + iMargin;
					pControl->rcText.right = rcClient.right - iMargin;

					pControl->rcActive.right = pControl->rcText.right;
					break;

				case cui_raw::left:
				case cui_raw::lefttop:
				case cui_raw::leftbottom:
					pControl->rcText.right = pControl->rcImage.left - iMargin;
					pControl->rcText.left = rcClient.left + iMargin;

					pControl->rcActive.left = pControl->rcText.left;
					break;
				default:
					break;
				}
			}

			if (((pControl->rcText.right - pControl->rcText.left) > 0) &&
				((pControl->rcText.bottom - pControl->rcText.top) > 0))
			{
				Gdiplus::RectF layoutRect = liblec::cui::gui_raw::cui_rawImpl::convert_rect(pControl->rcText);

				switch (pControl->textPlacement)
				{
				case cui_raw::bottom:
					align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::center);
					break;

				case cui_raw::top:
					align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::bottommiddle);
					break;

				case cui_raw::right:
					align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::middleleft);
					break;

				case cui_raw::righttop:
					align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::topleft);
					break;

				case cui_raw::rightbottom:
					align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::bottomleft);
					break;

				case cui_raw::left:
					align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::middleright);
					break;

				case cui_raw::lefttop:
					align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::topright);
					break;

				case cui_raw::leftbottom:
					align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::bottomright);
					break;

				default:
					// default to bottom
					align_text(text_rect, layoutRect, liblec::cui::gui_raw::cui_raw::textAlignment::center);
					break;
				}

				// draw text
				if (!pControl->bImageOnlyTightFit)
				{
					Gdiplus::StringFormat format;
					format.SetAlignment(Gdiplus::StringAlignment::StringAlignmentNear);
					format.SetTrimming(Gdiplus::StringTrimming::StringTrimmingEllipsisCharacter);
					format.SetFormatFlags(Gdiplus::StringFormatFlags::StringFormatFlagsNoWrap);

					if (design)
					{
						Gdiplus::Color color;
						color.SetFromCOLORREF(RGB(240, 240, 240));
						Gdiplus::SolidBrush brush(color);
						graphics.FillRectangle(&brush, layoutRect);

						color.SetFromCOLORREF(RGB(230, 230, 230));
						brush.SetColor(color);
						graphics.FillRectangle(&brush, text_rect);
					}

					// draw text
					color.SetFromCOLORREF(clrText);
					Gdiplus::SolidBrush text_brush(color);
					graphics.DrawString(pControl->sText.c_str(),
						-1, p_font, text_rect, &format, &text_brush);

					delete p_font;
					p_font = nullptr;
				}
			}

			// draw image
			if (pControl->m_pDisplaybitmap)
			{
				Gdiplus::Bitmap* original = new Gdiplus::Bitmap(pControl->rcImage.right - pControl->rcImage.left, pControl->rcImage.bottom - pControl->rcImage.top, pControl->m_pDisplaybitmap->GetPixelFormat());

				Gdiplus::Graphics graphics(original);

				if (!pControl->bChangeColor)
				{
					if (IsWindowEnabled(hWnd))
						graphics.DrawImage(pControl->m_pDisplaybitmap, 0, 0);
					else
					{
						// change bitmap color to greyscale

						// create a color matrix
						Gdiplus::ColorMatrix clrMatrix = {
							{
								{ .3f, .3f, .3f, 0, 0 },
								{ .6f, .6f, .6f, 0, 0 },
								{ .1f, .1f, .1f, 0, 0 },
								{ 0, 0, 0, 1, 0 },
								{ 0, 0, 0, 0, 1 }
							}
						};

						Gdiplus::ImageAttributes attributes;
						attributes.SetColorMatrix(&clrMatrix,
							Gdiplus::ColorMatrixFlagsDefault, Gdiplus::ColorAdjustTypeBitmap);

						Gdiplus::RectF destRect;
						destRect.X = 0;
						destRect.Y = 0;
						destRect.Width = (Gdiplus::REAL)(pControl->rcImage.right - pControl->rcImage.left);
						destRect.Height = (Gdiplus::REAL)(pControl->rcImage.bottom - pControl->rcImage.top);
						graphics.DrawImage(pControl->m_pDisplaybitmap, destRect, 0, 0, (Gdiplus::REAL)(original->GetWidth()), (Gdiplus::REAL)(original->GetHeight()), Gdiplus::UnitPixel, &attributes);
					}
				}
				else
				{
					// change bitmap color
					Gdiplus::Color c;

					if (!pControl->bHot)
						c.SetFromCOLORREF(pControl->clrImage);
					else
						c.SetFromCOLORREF(pControl->clrImageHot);

					if (!IsWindowEnabled(hWnd))
						c.SetFromCOLORREF(pControl->d->m_clrDisabled);

					// create a color matrix
					Gdiplus::ColorMatrix clrMatrix = {
						{
							{ 0, 0, 0, 0, 0 },
							{ 0, 0, 0, 0, 0 },
							{ 0, 0, 0, 0, 0 },
							{ 0, 0, 0, 1, 0 },
							{ c.GetR() / 255.0f,
							c.GetG() / 255.0f,
							c.GetB() / 255.0f,
							0, 1 }
						}
					};

					Gdiplus::ImageAttributes attributes;
					attributes.SetColorMatrix(&clrMatrix);

					Gdiplus::RectF destRect;
					destRect.X = 0;
					destRect.Y = 0;
					destRect.Width = (Gdiplus::REAL)(pControl->rcImage.right - pControl->rcImage.left);
					destRect.Height = (Gdiplus::REAL)(pControl->rcImage.bottom - pControl->rcImage.top);
					graphics.DrawImage(pControl->m_pDisplaybitmap, destRect, 0, 0, (Gdiplus::REAL)(original->GetWidth()), (Gdiplus::REAL)(original->GetHeight()), Gdiplus::UnitPixel, &attributes);
				}

				Gdiplus::Graphics graphics_out(hdc);
				graphics_out.DrawImage(original, (Gdiplus::REAL)pControl->rcImage.left, (Gdiplus::REAL)pControl->rcImage.top);

				if (original)
				{
					delete original;
					original = NULL;
				}
			}

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
			InvalidateRect(hWnd, NULL, FALSE);
		}
		else
		{
			// control has been disabled
			pControl->bHot = false;
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

		// delete display bitmap, we're done
		if (pControl->m_pDisplaybitmap)
		{
			delete pControl->m_pDisplaybitmap;
			pControl->m_pDisplaybitmap = NULL;
		}
	}
	break;

	case WM_SIZE:
	{
		// delete buffer, we need it recreated
		if (pControl->m_pDisplaybitmap)
		{
			delete pControl->m_pDisplaybitmap;
			pControl->m_pDisplaybitmap = NULL;
		}

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
} // ImageProc
