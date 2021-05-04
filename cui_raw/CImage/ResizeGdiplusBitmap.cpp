/*
** ResizeGdiplusBitmap.cpp - implementation of GDI+ bitmap resizing
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

#include <sstream>
#include "GetEncoderClsid/GetEncoderClsid.h"
#include "../Error/Error.h"
#include "../CGdiPlusBitmap/CGdiPlusBitmap.h"

// resize GDIPlus bitmap
Gdiplus::Bitmap * ResizeGdiplusBitmap(
	Gdiplus::Bitmap *pBmpIn,	// source bitmap
	const RECT rectTarget,		// target rect
	bool bStretch,				// stretch bitmap to supplied dimensions
	Quality quality,			// resizing quality
	bool bEnlargeIfSmaller,		// flag to determine whether image is enlarged to (width x height) if it is smaller than that
	bool bCenter,				// center bitmap
	RECT &rectOut				// output rect
)
{
	int iWidth = rectTarget.right - rectTarget.left;
	int iHeight = rectTarget.bottom - rectTarget.top;

	/*
	** deduce old height and ratio
	*/
	UINT old_height = pBmpIn->GetHeight();
	UINT old_width = pBmpIn->GetWidth();
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

	rectOut.left = 0;
	rectOut.top = 0;

	if (bCenter)
	{
		// set x-position for painting image on center
		rectOut.left = ((rectTarget.right - rectTarget.left) - iWidth) / 2;

		// set y-position for painting image on center
		rectOut.top = ((rectTarget.bottom - rectTarget.top) - iHeight) / 2;
	}

	rectOut.left += rectTarget.left;
	rectOut.top += rectTarget.top;

	/*
	** set quality of resizing image
	** by adjusting the Gdiplus interpolation mode
	*/
	Gdiplus::InterpolationMode iInterMode;
	Gdiplus::PixelOffsetMode iPixelMode;

	switch (quality)
	{
	case Quality::low:
		iInterMode = Gdiplus::InterpolationMode::InterpolationModeLowQuality;
		iPixelMode = Gdiplus::PixelOffsetMode::PixelOffsetModeDefault;
		break;

	case Quality::medium:
		iInterMode = Gdiplus::InterpolationMode::InterpolationModeBilinear;
		iPixelMode = Gdiplus::PixelOffsetMode::PixelOffsetModeDefault;
		break;

	case Quality::high:
		iInterMode = Gdiplus::InterpolationMode::InterpolationModeHighQualityBilinear;
		iPixelMode = Gdiplus::PixelOffsetMode::PixelOffsetModeHalf;
		break;

	default:
		iInterMode = Gdiplus::InterpolationMode::InterpolationModeDefault;
		iPixelMode = Gdiplus::PixelOffsetMode::PixelOffsetModeDefault;
		break;
	}

	Gdiplus::Bitmap* bmp_out = new Gdiplus::Bitmap(iWidth, iHeight, pBmpIn->GetPixelFormat());
	Gdiplus::Graphics graphics(bmp_out);
	graphics.SetInterpolationMode(iInterMode);
	graphics.SetPixelOffsetMode(iPixelMode);
	graphics.DrawImage(pBmpIn, 0, 0, iWidth, iHeight);

	rectOut.right = rectOut.left + iWidth;
	rectOut.bottom = rectOut.top + iHeight;

	return bmp_out;
} // ResizeGdiplusbitmap
