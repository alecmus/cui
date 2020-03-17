/*
** notX.cpp - pop-up notification implementation
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

#include "../../cui_raw.h"
#include "../cui_rawImpl.h"

enum controls
{
	initID = 20,
	shutdownID,
	button_close,
	button_min,
	button_max,

	captionIcon,
	title,
	icon,
	message,
	details,
};

static void cmdProcNot(cui_raw &ui, int iUniqueID, void *pData)
{
	controls ctrlID = (controls)iUniqueID;

	cui_raw::notParams *pParams = NULL;
	void* pState = ui.getState();
	if (pState)
		pParams = (cui_raw::notParams*)pState;

	switch (ctrlID)
	{
	case message:
		break;

	case details:
		break;

	default:
		break;
	}
} // cmdProcNot

  /// <summary>
  /// Automatically sets the thread's handle to NULL in the destructor.
  /// </summary>
  /// <remarks>
  /// This ensures the thread's handle is set to NULL AFTER the thread has exited.
  /// </remarks>
class autoSetThreadHandleToNULL
{
public:
	autoSetThreadHandleToNULL() {}

	void setThreadHandle(HANDLE *p)
	{
		p_hThread = p;
	}

	~autoSetThreadHandleToNULL()
	{
		if (p_hThread)
			(*p_hThread) = NULL;
	}

private:
	HANDLE *p_hThread = NULL;
}; // autoSetThreadHandleToNULL

UINT ThreadFunc(LPVOID lpvoid)
{
	cui_rawImpl::notStruct* m_not = (cui_rawImpl::notStruct*)lpvoid;

	// declare an object for setting THIS thread's handle to NULL after it is exits.
	autoSetThreadHandleToNULL m_auto;

	if (m_not)
	{
		// loop until the previous notification is closed
		while (true)
		{
			// get a handle to the previous notification's thread
			HANDLE* pHandle = NULL;

			try
			{
				CCriticalSectionLocker lock(m_not->d->m_locker_for_m_nots);

				for (auto &it : m_not->d->m_nots)
				{
					if (it.second.hThread && it.second.hThread != m_not->hThread)
					{
						if (it.second.iNumber == m_not->iNumber - 1)
						{
							// ONLY capture handle of previous notification,
							// whether or not it has been displayed
							pHandle = &it.second.hThread;
							break;
						}
					}
				}
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
			}

			if (pHandle)
			{
				WaitForSingleObject(*pHandle, INFINITE);

				// wait for one second to make it visible to the user that this is a different notification
				Sleep(1000);
			}
			else
				break;
		}

		// get a pointer to this thread's handle
		m_auto.setThreadHandle(&m_not->hThread);

		if (m_not->pcui_raw)
		{
			// prevent UI shadow if setting says so
			if (!m_not->params.bShadow)
				m_not->pcui_raw->hideShadow();

			// create window
			bool bRes = m_not->pcui_raw->create(initID, shutdownID, false, true, true);

			if (!bRes)
			{
				// something went wrong, return with msgBoxResult::noresponse
			}
		}
	}

	return NULL;
} // ThreadFunc

void cui_raw::notX(
	const notParams &in_params
	)
{
	try
	{
		auto text_size = [&](const std::basic_string<TCHAR> &text,
			const double &font_size,
			const std::basic_string<TCHAR> &font_name,
			const double &max_text_width)
		{
			HDC hdcScreen = GetDC(NULL);

			// capture current DPI scale
			const Gdiplus::REAL dpi_scale =
				(Gdiplus::REAL)GetDeviceCaps(hdcScreen, LOGPIXELSY) / (Gdiplus::REAL)96.0f;

			Gdiplus::Graphics graphics(hdcScreen);

			Gdiplus::Font* p_font = new Gdiplus::Font(&Gdiplus::FontFamily(font_name.c_str()),
				static_cast<Gdiplus::REAL>(font_size));

			if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
			{
				delete p_font;
				p_font = nullptr;
				p_font = new Gdiplus::Font(font_name.c_str(),
					static_cast<Gdiplus::REAL>(font_size), Gdiplus::FontStyle::FontStyleRegular,
					Gdiplus::UnitPoint, &d->m_font_collection);
			}

			Gdiplus::RectF text_rect;
			Gdiplus::RectF layoutRect;
			layoutRect.Width = static_cast<Gdiplus::REAL>(max_text_width);

			graphics.MeasureString(text.c_str(), -1, p_font, layoutRect, &text_rect);

			if (true)
			{
				if (text_rect.Width < layoutRect.Width)
					text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
			}

			delete p_font;
			p_font = nullptr;

			auto round_up = [](const Gdiplus::REAL &real)
			{
				LONG long_ = static_cast<LONG>(real);
				Gdiplus::REAL real_ = static_cast<Gdiplus::REAL>(long_);

				if (real_ < real)
					long_++;

				return long_;
			}; // round_up

			SIZE size;
			size.cx = round_up(text_rect.Width / dpi_scale);
			size.cy = round_up(text_rect.Height / dpi_scale);

			ReleaseDC(NULL, hdcScreen);

			return size;
		}; // text_size

		// TO-DO: find a way to delete this item ... it has been created dynamically ...
		cui_raw* pcui_raw = new cui_raw(d->m_sTitle, cmdProcNot, d->m_clrBackground, d->m_clrTheme, d->m_clrThemeHot, d->m_clrDisabled, d->m_sTooltipFont, d->m_iTooltipFontSize, d->m_clrTooltipText, d->m_clrTooltipBackground, d->m_clrTooltipBorder, d->m_hResModule, NULL, NULL);

		// add this structure to the notification structs map
		CCriticalSectionLocker locker(d->m_locker_for_m_nots);

		for (auto &it : get_font_files())
		{
			std::basic_string<TCHAR> sErr;
			pcui_raw->addFont(it, sErr);
		}

		cui_rawImpl::notStruct m_not;
		m_not.iNumber = (int)d->m_nots.size() + 1;
		m_not.pcui_raw = pcui_raw;
		m_not.params = in_params;
		m_not.d = d;
		d->m_nots.insert(std::pair<cui_raw*, cui_rawImpl::notStruct>(pcui_raw, m_not));

		// set state information
		d->m_nots[pcui_raw].pcui_raw->setState((void*)&d->m_nots[pcui_raw].params);

		int iMargin = 10;
		int iMainW = 0;
		int iMainH = 0;

		// calculate width of title string
		SIZE titleSize = text_size(d->m_sTitle,
			d->m_nots[pcui_raw].params.iFontSize, d->m_nots[pcui_raw].params.sFontName, 0);

		// set default window width
		iMainW = 300;

		// calculate width of message text
		SIZE messageSize = text_size(d->m_nots[pcui_raw].params.sMessage,
			11 * d->m_nots[pcui_raw].params.iFontSize / 9, d->m_nots[pcui_raw].params.sFontName, 0);

		// limit message length to 480
		messageSize.cx = min(messageSize.cx, 480);

		int icon_size = 0;

		if (d->m_nots[pcui_raw].params.IDP_ICON)
			icon_size = iMargin + 32;

		// base window width on whichever is greater
		iMainW = max(iMainW, icon_size + iMargin + messageSize.cx + iMargin);

		// determine width of details
		int iDetailsWidth = iMainW - 2 * iMargin - icon_size;

		// determine height of details
		SIZE detailsSize = text_size(d->m_nots[pcui_raw].params.sDetails,
			d->m_nots[pcui_raw].params.iFontSize, d->m_nots[pcui_raw].params.sFontName, 0);

		if (detailsSize.cx > iDetailsWidth)
		{
			// limit details width to 400
			detailsSize.cx = min(detailsSize.cx, 400);

			// base window width on whichever is greater
			iMainW = max(iMainW, icon_size + iMargin + detailsSize.cx + iMargin);

			iDetailsWidth = iMainW - 2 * iMargin - icon_size;

			detailsSize = text_size(d->m_nots[pcui_raw].params.sDetails,
				d->m_nots[pcui_raw].params.iFontSize, d->m_nots[pcui_raw].params.sFontName, iDetailsWidth);
		}

		// calculate height of window
		iMainH = 30 + iMargin + messageSize.cy + iMargin + detailsSize.cy + 2 * iMargin;

		if (!d->m_nots[pcui_raw].params.IDP_ICON)
			iMainW -= (32 + 2 * iMargin);

		// if no, display on top right offset be default
		if (true)
			pcui_raw->setPosition(cui_raw::windowPosition::topRightOffset, iMainW, iMainH);
		else
			pcui_raw->setPosition(cui_raw::windowPosition::bottomRightOffset, iMainW, iMainH);

		int iBtnWidth = 100;
		int iBtnHeight = 25;

		RECT rc;
		cui_raw::onResize resize;

		// add caption icon
		rc.left = 0;
		rc.right = rc.left + 16;
		rc.top = 0;
		rc.bottom = rc.top + 16;

		RECT rcTitleBar;
		rcTitleBar.top = 1;
		rcTitleBar.bottom = 30;	// TO-DO: remove magic number
		rcTitleBar.left = iMargin;
		rcTitleBar.right = iMainW - 1;
		cui_raw::posRect(rc, rcTitleBar, 0, 50);

		resize.iPercH = 0;
		resize.iPercV = 0;
		resize.iPercCY = 0;
		resize.iPercCX = 0;
		pcui_raw->addImage(_T(""),
			controls::captionIcon, _T(""), true, false,
			d->m_clrTheme, d->m_clrTheme, d->m_clrBackground, d->m_clrBackground,
			false,
			d->m_clrTheme, d->m_clrBackground, d->m_clrBackground, d->m_IDP_ICONSMALL, _T(""), RGB(0, 0, 0), RGB(0, 0, 0),
			d->m_nots[pcui_raw].params.sFontName, d->m_nots[pcui_raw].params.sFontName, d->m_nots[pcui_raw].params.iFontSize,
			cui_raw::imageTextPlacement::right, rc, resize, false, cui_raw::onToggle::toggleRight, false, _T(""), { 0, 0 });

		// add window title
		rc.left = rc.right + iMargin;
		rc.right = 200;
		rc.top = 1;	// TO-DO: remove magic number
		rc.bottom = rc.top + 29;	// TO-DO: remove magic number
		resize.iPercH = 0;
		resize.iPercV = 0;
		resize.iPercCY = 0;
		resize.iPercCX = 0;
		pcui_raw->addText(d->m_sTitle, controls::title, d->m_sTitle, true, d->m_clrTheme, d->m_clrTheme, _T(""), d->m_nots[pcui_raw].params.sFontName, d->m_nots[pcui_raw].params.iFontSize, rc, cui_raw::middleleft, resize, false);

		// add icon
		int right = iMargin;
		if (d->m_nots[pcui_raw].params.IDP_ICON)
		{
			// add icon
			RECT rcIcon;
			rcIcon.left = iMargin;
			rcIcon.right = rcIcon.left;
			rcIcon.top = 30;		// TO-DO: remove magic number
			rcIcon.bottom = rcIcon.top;

			rc.left = 0;
			rc.right = rc.left + 32;
			rc.top = 0;
			rc.bottom = rc.top + 32;

			RECT rcTarget;
			rcTarget.left = iMargin;
			rcTarget.top = 30 + iMargin;	// TO-DO: remove magic number
			rcTarget.right = rcTarget.left + 32;
			rcTarget.bottom = iMainH - iMargin;

			resize.iPercCY = 0;
			resize.iPercCX = 0;
			resize.iPercH = 0;
			resize.iPercV = 0;
			cui_raw::posRect(rc, rcTarget, resize.iPercH, resize.iPercV);

			rcIcon = rc;

			right = rcIcon.right + iMargin;

			// do not change color when set to RGB(256, 256, 256)!
			pcui_raw->addImage(d->m_sTitle, controls::icon, _T(""), true,
				d->m_nots[pcui_raw].params.clrImage != RGB(256, 256, 256),
				d->m_nots[pcui_raw].params.clrImage, d->m_nots[pcui_raw].params.clrImage,
				d->m_clrBackground, d->m_clrBackground, false, d->m_clrBackground,
				d->m_clrBackground, d->m_clrBackground, d->m_nots[pcui_raw].params.IDP_ICON, _T(""),
				RGB(0, 0, 0), RGB(0, 0, 0), d->m_nots[pcui_raw].params.sFontName,
				d->m_nots[pcui_raw].params.sFontName, d->m_nots[pcui_raw].params.iFontSize,
				cui_raw::imageTextPlacement::bottom, rc, resize, false, cui_raw::onToggle::toggleUp,
				false, _T(""), { 0, 0 });
		}

		// add buttons
		RECT rcText;
		rcText.top = 30;		// TO-DO: remove magic number
		rcText.left = right;
		rcText.right = iMainW - iMargin;
		rcText.bottom = iMainH - iMargin;

		// add message
		resize.iPercCY = 0;
		resize.iPercCX = 0;
		resize.iPercH = 100;
		resize.iPercV = 100;

		RECT rcMessage = rcText;
		rcMessage.bottom = rcMessage.top + messageSize.cy;

		// add message
		pcui_raw->addText(d->m_sTitle, controls::message, d->m_nots[pcui_raw].params.sMessage, true, RGB(0, 0, 0), RGB(0, 0, 0), _T(""), d->m_nots[pcui_raw].params.sFontName, 11 * d->m_nots[pcui_raw].params.iFontSize / 9, rcMessage, cui_raw::textAlignment::topleft, resize, false);

		rcText = rcMessage;
		rcText.top = rcText.bottom + iMargin;
		rcText.bottom = rcText.top + detailsSize.cy;

		// add details
		pcui_raw->addText(d->m_sTitle, controls::details, d->m_nots[pcui_raw].params.sDetails, true, RGB(0, 0, 0), RGB(0, 0, 0), _T(""), d->m_nots[pcui_raw].params.sFontName, d->m_nots[pcui_raw].params.iFontSize, rcText, cui_raw::textAlignment::topleft, resize, true);

		// add close button
		pcui_raw->addCloseBtn(controls::button_close);

		// set minimum width and height
		pcui_raw->setMinWidthAndHeight(iMainW, iMainH);

		// set window icons
		pcui_raw->setIcons(d->m_IDI_ICON, d->m_IDI_ICONSMALL, d->m_IDP_ICONSMALL);

		// set timer
		pcui_raw->setTimer(d->m_nots[pcui_raw].params.iTimer, true, true);

		// create a thread to run this on
		d->m_nots[pcui_raw].hThread = CreateThread(NULL,		// no security attributes
			0,									// use default stack size 
			(LPTHREAD_START_ROUTINE)ThreadFunc,
			&d->m_nots[pcui_raw],					// param to thread func
			NULL,								// creation flag
			NULL
		);
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	return;
} // not
