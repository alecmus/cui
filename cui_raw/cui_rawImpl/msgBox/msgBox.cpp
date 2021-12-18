//
// msgBox.cpp - message box implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#include "../../cui_raw.h"
#include "../cui_rawImpl.h"

// set to 0 to use 0ms in the busy loop (high CPU usage) and set to 1 to set to 1ms and disable shadow
// instead. This macro was made for aesthetic reasons. Each of these two methods ensure the message box is
// drawn properly over the parent window if one exists.
#define DISABLESHADOWFORUIPERFORMANCE	0

enum controls
{
	initID = 20,
	shutdownID,
	ok,
	cancel,
	yes,
	no,

	button_close,
	button_min,
	button_max,

	captionIcon,
	title,
	icon,
	message,
	details,
};

struct msgBoxData
{
	cui_raw::msgBoxParams params;
	std::basic_string<TCHAR> sTitle;
	COLORREF clrBackground;
	COLORREF clrTheme;
	COLORREF clrThemeHot;
	std::basic_string<TCHAR> sTooltipFont;
	double iTooltipFontSize;
	COLORREF clrTooltipText;
	COLORREF clrTooltipBackground;
	COLORREF clrTooltipBorder;
	int IDI_ICON;
	int IDI_ICONSMALL;
	int IDP_ICONSMALL;
	HMODULE hResModule;
	cui_raw* pParent;

	bool bRespondedTo = false;
	cui_raw::msgBoxResult result;
};

static void cmdProcMsg(cui_raw &ui, int iUniqueID, void *pData)
{
	controls ctrlID = (controls)iUniqueID;

	msgBoxData *pParams = NULL;
	void* pState = ui.getState();
	if (pState)
		pParams = (msgBoxData*)pState;

	bool bClose = false;

	switch (ctrlID)
	{
	case initID:
		break;

	case ok:
	{
		// ok button pressed
		if (pParams)
		{
			pParams->result = cui_raw::msgBoxResult::ok;
			pParams->bRespondedTo = true;
			bClose = true;
		}
	}
	break;

	case cancel:
	{
		// cancel button pressed
		if (pParams)
		{
			pParams->result = cui_raw::msgBoxResult::cancel;
			pParams->bRespondedTo = true;
			bClose = true;
		}
	}
	break;

	case yes:
	{
		// yes button pressed
		if (pParams)
		{
			pParams->result = cui_raw::msgBoxResult::yes;
			pParams->bRespondedTo = true;
			bClose = true;
		}
	}
	break;

	case no:
	{
		// no button pressed
		if (pParams)
		{
			pParams->result = cui_raw::msgBoxResult::no;
			pParams->bRespondedTo = true;
			bClose = true;
		}
	}
	break;

	default:
		break;
	}

	if (bClose)
	{
		// close window
		ui.close();
	}
} // cmdProcMsg

UINT msgBoxThreadFunc(LPVOID lpvoid)
{
	msgBoxData *pData = (msgBoxData*)lpvoid;

	Gdiplus::PrivateFontCollection m_font_collection;

	if (pData->pParent)
	{
		for (auto &it : pData->pParent->get_font_files())
			m_font_collection.AddFontFile(it.c_str());
	}

	auto text_size = [&](const std::basic_string<TCHAR> &text,
		const double &font_size,
		const std::basic_string<TCHAR> &font_name,
		const double &max_text_width)
	{
		HDC hdcScreen = GetDC(NULL);

		Gdiplus::Graphics graphics(hdcScreen);

		Gdiplus::FontFamily ffm(font_name.c_str());
		Gdiplus::Font* p_font = new Gdiplus::Font(&ffm,
			static_cast<Gdiplus::REAL>(font_size));

		if (p_font->GetLastStatus() != Gdiplus::Status::Ok)
		{
			delete p_font;
			p_font = nullptr;
			p_font = new Gdiplus::Font(font_name.c_str(),
				static_cast<Gdiplus::REAL>(font_size), Gdiplus::FontStyle::FontStyleRegular,
				Gdiplus::UnitPoint, &m_font_collection);
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
		size.cx = round_up(text_rect.Width);
		size.cy = round_up(text_rect.Height);

		ReleaseDC(NULL, hdcScreen);

		return size;
	}; // text_size

	cui_raw wnd(
		pData->sTitle,
		cmdProcMsg, pData->clrBackground, pData->clrTheme, pData->clrThemeHot,
		RGB(240, 240, 240), pData->sTooltipFont, pData->iTooltipFontSize, pData->clrTooltipText,
		pData->clrTooltipBackground, pData->clrTooltipBorder, pData->hResModule,
		pData->pParent, (void*)pData);

	if (pData->pParent)
	{
		for (auto &it : pData->pParent->get_font_files())
		{
			std::basic_string<TCHAR> sErr;
			wnd.addFont(it, sErr);
		}
	}

	int iMargin = 10;
	int iDetailsButtonsMargin = 2 * iMargin;
	int iMainW = 0;
	int iMainH = 0;

	// calculate width of title string
	SIZE titleSize = text_size(pData->sTitle,
		pData->params.iFontSize, pData->params.sFontName, 0);

	// set default window width
	iMainW = 300;

	// calculate width of message text
	SIZE messageSize = text_size(pData->params.sMessage,
		11 * pData->params.iFontSize / 9, pData->params.sFontName, 0);

	// limit message length to 480
	messageSize.cx = min(messageSize.cx, 480);

	int icon_size = 0;

	if (pData->params.IDP_ICON)
		icon_size = iMargin + 32;

	// base window width on whichever is greater
	iMainW = max(iMainW, icon_size + iMargin + messageSize.cx + iMargin);

	// determine width of details
	int iDetailsWidth = iMainW - 2 * iMargin - icon_size;

	// determine height of details
	SIZE detailsSize = text_size(pData->params.sDetails,
		pData->params.iFontSize, pData->params.sFontName, 0);

	if (detailsSize.cx > iDetailsWidth)
	{
		// limit details width to 400
		detailsSize.cx = min(detailsSize.cx, 400);

		// base window width on whichever is greater
		iMainW = max(iMainW, icon_size + iMargin + detailsSize.cx + iMargin);

		iDetailsWidth = iMainW - 2 * iMargin - icon_size;

		detailsSize = text_size(pData->params.sDetails,
			pData->params.iFontSize, pData->params.sFontName, iDetailsWidth);
	}

	// calculate height of window
	iMainH = 30 + messageSize.cy + iMargin + detailsSize.cy + iDetailsButtonsMargin + 25 + iMargin;

	if (pData->pParent)
		wnd.setPosition(cui_raw::windowPosition::centerToParent, iMainW, iMainH);
	else
		wnd.setPosition(cui_raw::windowPosition::centerToWorkingArea, iMainW, iMainH);

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
	wnd.addImage(_T(""),
		controls::captionIcon, _T(""), true, false,
		pData->clrTheme, pData->clrTheme, pData->clrBackground, pData->clrBackground,
		false,
		pData->clrTheme, pData->clrBackground, pData->clrBackground, pData->IDP_ICONSMALL,
		_T(""), RGB(0, 0, 0), RGB(0, 0, 0),
		pData->params.sFontName, pData->params.sFontName, pData->params.iFontSize,
		cui_raw::imageTextPlacement::right, rc, resize, false,
		cui_raw::onToggle::toggleRight, false, _T(""), { 0, 0 });

	// add window title
	rc.left = rc.right + iMargin;
	rc.right = rc.left + titleSize.cx;
	rc.top = 1;	// TO-DO: remove magic number
	rc.bottom = rc.top + 29;	// TO-DO: remove magic number
	resize.iPercH = 0;
	resize.iPercV = 0;
	resize.iPercCY = 0;
	resize.iPercCX = 0;
	wnd.addText(pData->sTitle, controls::title, pData->sTitle, true, pData->clrTheme,
		pData->clrTheme, _T(""), pData->params.sFontName, pData->params.iFontSize,
		rc, cui_raw::middleleft, resize, false);

	// add icon
	int right = iMargin;
	if (pData->params.IDP_ICON)
	{
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
		wnd.addImage(pData->sTitle, controls::icon, _T(""), true,
			pData->params.clrImage != RGB(256, 256, 256), pData->params.clrImage,
			pData->params.clrImage, pData->clrBackground, pData->clrBackground,
			false, pData->clrBackground, pData->clrBackground, pData->clrBackground,
			pData->params.IDP_ICON, _T(""), RGB(0, 0, 0), RGB(0, 0, 0),
			pData->params.sFontName, pData->params.sFontName, pData->params.iFontSize, cui_raw::imageTextPlacement::bottom, rc, resize,
			false, cui_raw::onToggle::toggleUp, false, _T(""), { 0, 0 });
	}

	// add buttons
	RECT rcText;
	rcText.top = 30;		// TO-DO: remove magic number
	rcText.left = right;
	rcText.right = iMainW - iMargin;
	rcText.bottom = iMainH - iMargin;

	switch (pData->params.type)
	{
	case cui_raw::okOnly:
	{
		// define target rect
		RECT rcTarget;
		rcTarget.left = iMargin;
		rcTarget.right = iMainW - iMargin;
		rcTarget.top = 30;		// TO-DO: remove magic number
		rcTarget.bottom = iMainH - iMargin;

		// define rect to place button in
		rc.left = 0;
		rc.right = rc.left + 90;
		rc.top = 0;
		rc.bottom = rc.top + 25;

		// add OK button
		resize.iPercH = 100;
		resize.iPercV = 100;
		cui_raw::posRect(rc, rcTarget, resize.iPercH, resize.iPercV);
		wnd.addButton(pData->sTitle, controls::ok, _T(""), _T("OK"), pData->params.sFontName,
			pData->params.iFontSize, rc, resize, false);

		rcText.bottom = rc.top - iDetailsButtonsMargin;
	}
	break;

	case cui_raw::okCancel:
	{
		// define target rect
		RECT rcTarget;
		rcTarget.left = iMargin;
		rcTarget.right = iMainW - iMargin;
		rcTarget.top = 30;		// TO-DO: remove magic number
		rcTarget.bottom = iMainH - iMargin;

		// define rect to place button in
		rc.left = 0;
		rc.right = rc.left + 90;
		rc.top = 0;
		rc.bottom = rc.top + 25;

		// add Cancel button
		resize.iPercH = 100;
		resize.iPercV = 100;
		cui_raw::posRect(rc, rcTarget, resize.iPercH, resize.iPercV);
		wnd.addButton(
			pData->sTitle, controls::cancel, _T(""), _T("Cancel"),
			pData->params.sFontName, pData->params.iFontSize, rc, resize, false
		);

		// add OK button
		rc.right -= (90 + iMargin);
		rc.left -= (90 + iMargin);
		wnd.addButton(
			pData->sTitle, controls::ok, _T(""), _T("OK"), pData->params.sFontName,
			pData->params.iFontSize, rc, resize, false
		);

		rcText.bottom = rc.top - iDetailsButtonsMargin;
	}
	break;

	case cui_raw::yesNo:
	{
		// define target rect
		RECT rcTarget;
		rcTarget.left = iMargin;
		rcTarget.right = iMainW - iMargin;
		rcTarget.top = 30;		// TO-DO: remove magic number
		rcTarget.bottom = iMainH - iMargin;

		// define rect to place button in
		rc.left = 0;
		rc.right = rc.left + 90;
		rc.top = 0;
		rc.bottom = rc.top + 25;

		// add Cancel button
		resize.iPercH = 100;
		resize.iPercV = 100;
		cui_raw::posRect(rc, rcTarget, resize.iPercH, resize.iPercV);
		wnd.addButton(
			pData->sTitle, controls::no, _T(""), _T("No"), pData->params.sFontName,
			pData->params.iFontSize, rc, resize, false
		);

		// add OK button
		rc.right -= (90 + iMargin);
		rc.left -= (90 + iMargin);
		wnd.addButton(
			pData->sTitle, controls::yes, _T(""), _T("Yes"), pData->params.sFontName,
			pData->params.iFontSize, rc, resize, false
		);

		rcText.bottom = rc.top - iDetailsButtonsMargin;
	}
	break;

	default:
		break;
	}

	// add message
	resize.iPercCY = 0;
	resize.iPercCX = 0;
	resize.iPercH = 100;
	resize.iPercV = 100;

	RECT rcMessage = rcText;
	rcMessage.bottom = rcMessage.top + messageSize.cy;

	// add message
	wnd.addText(
		pData->sTitle, controls::message, pData->params.sMessage, true, RGB(0, 0, 0), RGB(0, 0, 0),
		_T(""), pData->params.sFontName, 11 * pData->params.iFontSize / 9, rcMessage,
		cui_raw::textAlignment::topleft, resize, false
	);

	rcText = rcMessage;
	rcText.top = rcText.bottom + iMargin;
	rcText.bottom = rcText.top + detailsSize.cy;

	// add details
	wnd.addText(
		pData->sTitle, controls::details, pData->params.sDetails, true, RGB(0, 0, 0), RGB(0, 0, 0),
		_T(""), pData->params.sFontName, pData->params.iFontSize, rcText, cui_raw::textAlignment::topleft,
		resize, true
	);

	// add close button
	wnd.addCloseBtn(controls::button_close);

	// set minimum width and height
	wnd.setMinWidthAndHeight(iMainW, iMainH);

	// set window icons
	wnd.setIcons(pData->IDI_ICON, pData->IDI_ICONSMALL, pData->IDP_ICONSMALL);

#if DISABLESHADOWFORUIPERFORMANCE
	if (pData->pParent)
	{
		if (IsWindowVisible(pData->pParent->getHWND()))
		{
			// hide the shadow
			wnd.hideShadow();	// use this option if cpu usage in the busy function is too much
		}
	}
#endif // DISABLESHADOWFORUIPERFORMANCE

	// create window
	bool bRes = wnd.create(initID, shutdownID);

	cui_raw::msgBoxResult res = cui_raw::msgBoxResult::noresponse;

	if (!bRes)
		pData->result = res;	// something went wrong, return with msgBoxResult::noresponse
	else
	{
		if (pData->bRespondedTo)
		{
			// pData->result = pData->result;
		}
		else
			pData->result = res;
	}

	return 0;
} // msgBoxThreadFunc

  /// <summary>
  /// This function keeps the UI responsive during lengthy calls.
  /// </summary>
  /// 
  /// <returns>
  /// Returns true to permit operation to continue and false to quit immediately.
  /// </returns>
static bool BusyFunction()
{
	//return true;	// this function is causing a recursion problem when it processes messages

	// check if there are Windows messages and dispatch them (keeps UI responsive)
	MSG uMsg = {};

	if (PeekMessage(&uMsg, NULL, 0, 0, PM_REMOVE))
	{
		if (uMsg.message == WM_QUIT)
		{
			PostQuitMessage(0);
			return false;
		}
		else
		{
			TranslateMessage(&uMsg);
			DispatchMessage(&uMsg);
		}
	}

	return true;
} // BusyFunction

cui_raw::msgBoxResult cui_raw::msgBox(
	const msgBoxParams &params
)
{
	cui_raw::msgBoxResult res = msgBox(params, d->m_sTitle, d->m_clrBackground, d->m_clrTheme, d->m_clrThemeHot, d->m_sTooltipFont, d->m_iTooltipFontSize, d->m_clrTooltipText, d->m_clrTooltipBackground, d->m_clrTooltipBorder, d->m_IDI_ICON, d->m_IDI_ICONSMALL, d->m_IDP_ICONSMALL, d->m_hResModule, this);
	return res;
} // msgBox

cui_raw::msgBoxResult cui_raw::msgBox(
	const cui_raw::msgBoxParams &params,
	const std::basic_string<TCHAR> &sTitle,
	COLORREF clrBackground,
	COLORREF clrTheme,
	COLORREF clrThemeHot,
	const std::basic_string<TCHAR> &sTooltipFont,
	double iTooltipFontSize,
	COLORREF clrTooltipText,
	COLORREF clrTooltipBackground,
	COLORREF clrTooltipBorder,
	int IDI_ICON,
	int IDI_ICONSMALL,
	int IDP_ICONSMALL,
	HMODULE hResModule,
	cui_raw* pParent
)
{
	if (pParent)
	{
		if (pParent->d->m_iMessageBoxes > 0)
		{
			// do not allow more than one message box to be displayed
			return cui_raw::msgBoxResult::notexecuted;	// this is super crazy and has to be remembered
		}

		pParent->d->m_iMessageBoxes++;
	}

	if (pParent && IsWindow(pParent->getHWND()) && IsIconic(pParent->getHWND()))
	{
		// parent was minimized when the message box was displayed ... restore it ...
		ShowWindow(pParent->getHWND(), SW_RESTORE);
		UpdateWindow(pParent->getHWND());
	}

	msgBoxData m_data;
	m_data.params = params;
	m_data.sTitle = sTitle;
	m_data.clrBackground = clrBackground;
	m_data.clrTheme = clrTheme;
	m_data.clrThemeHot = clrThemeHot;
	m_data.sTooltipFont = sTooltipFont;
	m_data.iTooltipFontSize = iTooltipFontSize;
	m_data.clrTooltipText = clrTooltipText;
	m_data.clrTooltipBackground = clrTooltipBackground;
	m_data.clrTooltipBorder = clrTooltipBorder;
	m_data.IDI_ICON = IDI_ICON;
	m_data.IDI_ICONSMALL = IDI_ICONSMALL;
	m_data.IDP_ICONSMALL = IDP_ICONSMALL;
	m_data.hResModule = hResModule;
	m_data.pParent = pParent;
	m_data.bRespondedTo = false;
	m_data.result = cui_raw::msgBoxResult::noresponse;

	HANDLE hThread = CreateThread(NULL,		// no security attributes
		0,									// use default stack size 
		(LPTHREAD_START_ROUTINE)msgBoxThreadFunc,
		&m_data,							// param to thread func
		NULL,								// creation flag
		NULL
	);

	while (true)
	{
#if DISABLESHADOWFORUIPERFORMANCE
		if (WaitForSingleObject(hThread, 1) == WAIT_TIMEOUT)
#else
		if (WaitForSingleObject(hThread, 0) == WAIT_TIMEOUT)
#endif // DISABLESHADOWFORUIPERFORMANCE

		{
			if (!BusyFunction())
			{
				TerminateThread(hThread, 1);

				if (pParent)
					pParent->d->m_iMessageBoxes--;

				return cui_raw::msgBoxResult::noresponse;
			}
			else
				continue;
		}
		else
			break;
	}

	// wait for thread to close
	WaitForSingleObject(hThread, INFINITE);

	CloseHandle(hThread);
	hThread = NULL;

	if (pParent)
		pParent->d->m_iMessageBoxes--;

	return m_data.result;
} // msgBox
