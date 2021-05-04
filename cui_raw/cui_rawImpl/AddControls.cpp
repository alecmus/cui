/*
** AddControls.cpp - add controls implementation
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

#include "cui_rawImpl.h"
#include "Combobox/Combobox.h"
#include "../scaleAdjust/scaleAdjust.h"
#include "../XCreateFont/XCreateFont.h"

bool isNumeric(const std::basic_string<TCHAR> &s)
{
	bool bIsNumeric = true;

	for (auto &ch : s)
	{
		if (IsCharAlpha(ch))
			bIsNumeric = false;

		if (!bIsNumeric)
			break;
	}

	return bIsNumeric;
} // isNumeric

bool isNumeric(const std::vector<std::basic_string<TCHAR>> &v)
{
	bool bIsNumeric = true;

	for (auto &it : v)
	{
		if (!isNumeric(it))
			bIsNumeric = false;

		if (!bIsNumeric)
			break;
	}

	return bIsNumeric;
} // isNumeric

BOOL CALLBACK enumFamCallBack(LPLOGFONT lplf,
	LPNEWTEXTMETRIC lpntm,
	DWORD FontType,
	LPVOID pVoid)
{
	UNREFERENCED_PARAMETER(lplf);
	UNREFERENCED_PARAMETER(lpntm);

	/* Windows lists fonts which have a vmtx (vertical metrics) table twice.
	* Once using their normal name, and again preceded by '@'. These appear
	* in font lists in some windows apps, such as wordpad. We don't want
	* these so we skip any font where the first character is '@'
	*/
	if (lplf->lfFaceName[0] == '@') {
		return TRUE;
	}

	std::vector<std::basic_string<TCHAR>> *pvFontNames =
		(std::vector<std::basic_string<TCHAR>> *) pVoid;

	// capture font names
	pvFontNames->push_back(lplf->lfFaceName);

	return TRUE;
} // enumFamCallBack

  /// <summary>
  /// Get list of fonts that are available to the app.
  /// </summary>
  /// 
  /// <param name="vFontNames">
  /// The list of fonts that are available.
  /// </param>
void getFontNames(std::vector<std::basic_string<TCHAR>> &vFontNames)
{
	HDC hdc = GetDC(GetDesktopWindow());

	EnumFontFamilies(hdc, (LPCTSTR)NULL,
		(FONTENUMPROC)enumFamCallBack, (LPARAM)&vFontNames);

	ReleaseDC(GetDesktopWindow(), hdc);
} // getFontNames

void cui_rawImpl::AddControls(HWND hWnd, std::basic_string<TCHAR> sPage, cui_raw* pThis)
{
	if (IsIconic(hWnd))
	{
		// window is minimized ... restore it ...
		ShowWindow(hWnd, SW_RESTORE);
		UpdateWindow(hWnd);
	}

	// create controls and hidden controls containers (TO-DO: improve)
	pThis->d->m_vPagelessControls[pThis->d->m_sTitle];
	pThis->d->m_vControls[sPage];
	pThis->d->m_vHiddenControls[sPage];

	// calculate ultimate minimum window width
	int iControlButtonSize = 2 * pThis->d->m_iTitlebarHeight / 3;

	int iMargin = (pThis->d->m_iTitlebarHeight - iControlButtonSize) / 2;

	int iMin = ((int)pThis->d->m_ControlBtns.size() - 1) * (iControlButtonSize + iMargin);
	pThis->d->m_iMinWidthCalc =
		pThis->d->m_iMinWidthCalc > iMin ? pThis->d->m_iMinWidthCalc : iMin;

	// add tab control
	if (m_Pages.at(sPage).m_TabControl.iUniqueID != -123)	// to-do: remove magic number
	{
		// this page has a tab control ... add it ...
		if (!IsWindow(m_Pages.at(sPage).m_TabControl.hWnd))
		{
			m_Pages.at(sPage).m_TabControl.d = pThis->d;

			DWORD dwStyle = WS_CHILD;

			m_Pages.at(sPage).m_TabControl.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
				m_Pages.at(sPage).m_TabControl.coords.left,
				m_Pages.at(sPage).m_TabControl.coords.top,
				m_Pages.at(sPage).m_TabControl.coords.width() - 1,	// -1 to permit line on right
				m_Pages.at(sPage).m_TabControl.coords.height(),
				hWnd, (HMENU)(INT_PTR)m_Pages.at(sPage).m_TabControl.iUniqueID, pThis->d->hInstance_, NULL);

			try
			{
				pThis->d->m_vControls.at(sPage).push_back(m_Pages.at(sPage).m_TabControl.hWnd);
			}
			catch (std::exception &e)
			{
				std::string m_sErr = e.what();
			}

			// subclass control so we can do custom drawing
			SetWindowLongPtr(m_Pages.at(sPage).m_TabControl.hWnd, GWLP_USERDATA,
				(LONG_PTR)&m_Pages.at(sPage).m_TabControl);
			m_Pages.at(sPage).m_TabControl.PrevProc =
				SetWindowLongPtr(m_Pages.at(sPage).m_TabControl.hWnd, GWLP_WNDPROC,
				(LONG_PTR)tabControlProc);

			// set resizing behaviour
			pThis->d->m_pResizer->OnResize(m_Pages.at(sPage).m_TabControl.hWnd,
				m_Pages.at(sPage).m_TabControl.resize.iPercH,
				m_Pages.at(sPage).m_TabControl.resize.iPercV,
				m_Pages.at(sPage).m_TabControl.resize.iPercCX,
				m_Pages.at(sPage).m_TabControl.resize.iPercCY);

			// add a line to the right of the tab control
			RECT rcTabLine;
			rcTabLine.right = m_Pages.at(sPage).m_TabControl.coords.right;
			rcTabLine.left = rcTabLine.right - 1;
			rcTabLine.top = m_Pages.at(sPage).m_TabControl.coords.top;
			rcTabLine.bottom = m_Pages.at(sPage).m_TabControl.coords.bottom;

			UNscaleRECT(rcTabLine, pThis->d->m_DPIScale);

			pThis->addHairLine(sPage,
				444,	// TO-DO: remove magic number
				m_Pages.at(sPage).m_TabControl.clrTabLine,
				rcTabLine,
				m_Pages.at(sPage).m_TabControl.resize);
		}
	}

	// add rect controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_RectControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				it.second.d = pThis->d;

				DWORD dwStyle = WS_CHILD;

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
					it.second.coords.left, 
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				// capture control only if it's not a border
				if (it.second.iUniqueID != pThis->d->m_iLeftBorder &&
					it.second.iUniqueID != pThis->d->m_iTopBorder &&
					it.second.iUniqueID != pThis->d->m_iRightBorder &&
					it.second.iUniqueID != pThis->d->m_iBottomBorder
					)
				{
					if (!it.second.bPageLess)
						captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
					else
						capturePagelessControls(it.second.iUniqueID, it.second.hWnd);
				}

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)RectProc);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add button controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_ButtonControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				it.second.d = pThis->d;

				DWORD dwStyle = WS_CHILD;

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
					it.second.coords.left,
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)BtnProc);

				if (it.second.bDefaultButton)
					SetFocus(it.second.hWnd);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add text controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_TextControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				DWORD dwStyle = WS_CHILD;

				// set background color
				it.second.clrBackground = pThis->d->m_clrBackground;

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
					it.second.coords.left,
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)TextControlProc);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add window control buttons
	for (auto &it : pThis->d->m_ControlBtns)
	{
		if (!IsWindow(it.second.hWnd))
		{
			RECT rc;
			GetClientRect(hWnd, &rc);

			// add close button
			if (it.first == pThis->d->m_iIDClose)
			{
				DWORD dwStyle = WS_CHILD;

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
					rc.right - iControlButtonSize - iMargin,
					rc.top + iMargin,
					iControlButtonSize,
					iControlButtonSize,
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)ControlBtnProc);

				// set resize behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, 100, 0, false, false);
			}

			// add max button
			if (it.first == pThis->d->m_iIDMax)
			{
				if (pThis->d->m_bClosebtn)
					rc.right -= (iControlButtonSize + iMargin);

				DWORD dwStyle = WS_CHILD;

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
					rc.right - iControlButtonSize - iMargin,
					rc.top + iMargin,
					iControlButtonSize,
					iControlButtonSize,
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)ControlBtnProc);

				// set resize behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, 100, 0, false, false);
			}

			// add min button
			if (it.first == pThis->d->m_iIDMin)
			{
				if (pThis->d->m_bClosebtn)
					rc.right -= (iControlButtonSize + iMargin);

				if (pThis->d->m_bMaxbtn)
					rc.right -= (iControlButtonSize + iMargin);

				DWORD dwStyle = WS_CHILD;

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
					rc.right - iControlButtonSize - iMargin,
					rc.top + iMargin,
					iControlButtonSize,
					iControlButtonSize,
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)ControlBtnProc);

				// set resize behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, 100, 0, false, false);
			}
		}
	}

	// add combobox controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_ComboBoxControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				// check if this is a font list combobox
				for (auto m_it : pThis->d->vFontCombos)
				{
					if (it.second.iUniqueID == m_it)
						it.second.bFontList = true;
				}

				DWORD dwStyle = WS_CHILD | WS_VSCROLL | WS_TABSTOP | CBS_AUTOHSCROLL;

				// check if this is a numeric combobox
				if (!isNumeric(it.second.vData))
					dwStyle |= CBS_SORT;

				if (it.second.bFontList)
					dwStyle |= CBS_OWNERDRAWFIXED | CBS_HASSTRINGS;

				// TO-DO: determine if this is really neccessary with all our custom drawing
				if (it.second.bReadOnly)
					dwStyle |= CBS_DROPDOWNLIST;
				else
					dwStyle |= CBS_DROPDOWN;

				it.second.hWnd = CreateWindow(WC_COMBOBOX, _T(""), dwStyle,
					it.second.coords.left,
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// subclass combobox control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)ComboBoxControlProc);

				if (it.second.bFontList)
				{
					// get list of fonts
					static std::vector<std::basic_string<TCHAR>> m_vFontNames;

					HDC dc = GetDC(it.second.hWnd);

					if (m_vFontNames.empty())
					{
						// do this only once
						getFontNames(m_vFontNames);

						for (auto fontName : m_vFontNames)
						{
							Gdiplus::Font* p_font =
								new Gdiplus::Font(&Gdiplus::FontFamily(fontName.c_str()), 9);

							if (p_font && p_font->GetLastStatus() != Gdiplus::Status::Ok)
							{
								delete p_font;
								p_font = nullptr;
								p_font = new Gdiplus::Font(fontName.c_str(),
									9, Gdiplus::FontStyle::FontStyleRegular,
									Gdiplus::UnitPoint, &pThis->d->m_font_collection);
							}

							if (p_font && p_font->GetLastStatus() == Gdiplus::Status::Ok)
								pThis->d->vFonts[fontName] = p_font;
						}
					}

					// populate combobox with list of fonts

					Gdiplus::Graphics gr(dc);

					RECT rc;
					rc.left = 0;
					rc.top = 0;
					rc.right = 0;
					rc.bottom = 0;

					Gdiplus::RectF layoutRect = convert_rect(rc);

					for (auto font : pThis->d->vFonts)
					{
						LRESULT iIndex = SendMessage(
							it.second.hWnd,
							CB_ADDSTRING,
							NULL,
							LPARAM(font.first.c_str())
						);

						// measure text rectangle
						Gdiplus::RectF text_rect;
						gr.MeasureString(font.first.c_str(),
							-1, font.second, layoutRect, &text_rect);

						if (true)
						{
							if (text_rect.Width < layoutRect.Width)
								text_rect.Width = min(text_rect.Width * 1.01f, layoutRect.Width);
						}

						it.second.m_iMaxNameWidth =
							max(it.second.m_iMaxNameWidth, static_cast<int>(text_rect.Width));

						if (false)
						{
							// not currently used
							SIZE sz = { 0 };
							SendMessage(it.second.hWnd, CB_SETITEMDATA, iIndex,
								(LPARAM)sz.cx);	// save width of item in data
						}
					}

					ReleaseDC(it.second.hWnd, dc);

					// select default font
					std::basic_string<TCHAR> sErr;
					pThis->selectComboItem(sPage, it.second.iUniqueID, it.second.sFontName, sErr);
				}
				else
				{
					// populate combobox
					for (size_t x = 0; x < it.second.vData.size(); x++)
					{
						SendMessage(
							it.second.hWnd,
							CB_ADDSTRING,
							NULL,
							LPARAM(it.second.vData[x].c_str())
						);
					}

					// set combobox font
					HDC hdc = GetDC(it.second.hWnd);

					it.second.hfont = XCreateFont(hdc, it.second.sFontName, it.second.iFontSize);
					SetWindowFont(it.second.hWnd, it.second.hfont, TRUE);

					ReleaseDC(it.second.hWnd, hdc);

					// select item
					int nIndex = (int)::SendMessage(it.second.hWnd, CB_FINDSTRINGEXACT, NULL,
						LPARAM(it.second.sSelectedItem.c_str()));

					SendMessage(it.second.hWnd, CB_SETCURSEL, nIndex, NULL);

					if (!it.second.bReadOnly && it.second.bAutoComplete)
					{
						ComboBoxEditControl comboEdit;

						// get the combobox's edit control
						comboEdit.hWnd = Combo_IsExtended(it.second.hWnd) ?
							ComboBoxEx_GetEditControl(it.second.hWnd) :
							FindWindowEx(it.second.hWnd, NULL, WC_EDIT, NULL);

						try
						{
							pThis->d->m_Pages.at(sPage).m_ComboBoxEditControls.insert(
								std::make_pair(comboEdit.hWnd, comboEdit));

							// subclass combobox's edit control so we can do autocomplete
							SetWindowLongPtr(
								pThis->d->m_Pages.at(sPage).m_ComboBoxEditControls.at(
									comboEdit.hWnd).hWnd, GWLP_USERDATA,
									(LONG_PTR)&pThis->d->m_Pages.at(
										sPage).m_ComboBoxEditControls.at(comboEdit.hWnd));
							pThis->d->m_Pages.at(sPage).m_ComboBoxEditControls.at(
								comboEdit.hWnd).PrevProc = SetWindowLongPtr(pThis->d->m_Pages.at(
									sPage).m_ComboBoxEditControls.at(comboEdit.hWnd).hWnd,
									GWLP_WNDPROC, (LONG_PTR)ComboBoxEditControlProc);

							// Set the text limit to standard size
							ComboBox_LimitText(it.second.hWnd, DEFAULT_TXT_LIM);
						}
						catch (std::exception &e)
						{
							std::string sErr = e.what();
						}
					}
				}

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);

				// select default item
				std::basic_string<TCHAR> sErr;

				if (!it.second.sSelectedItem.empty())
					pThis->selectComboItem(sPage,
						it.second.iUniqueID, it.second.sSelectedItem, sErr);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add listview controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_listviewControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				DWORD dwStyle = WS_CHILD | LVS_REPORT | LVS_SHOWSELALWAYS;

				if (it.second.bBorder)
					dwStyle |= WS_BORDER;

				it.second.hWnd = CreateWindow(WC_LISTVIEW, _T(""), dwStyle,
					it.second.coords.left,
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// set listview font
				HDC hdc = GetDC(it.second.hWnd);

				it.second.hfont = XCreateFont(hdc, it.second.sFontName, it.second.iFontSize);
				SetWindowFont(it.second.hWnd, it.second.hfont, TRUE);

				ReleaseDC(it.second.hWnd, hdc);

				// add listview columns
				it.second.pClistview = new Clistview;

				DWORD dwExStyle = LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP |
					LVS_EX_DOUBLEBUFFER;

				if (it.second.bGridLines)
					dwExStyle |= LVS_EX_GRIDLINES;

				it.second.pClistview->Init(it.second.hWnd, NULL, dwExStyle);

				for (size_t x = 0; x < it.second.vColumns.size(); x++)
				{
					// set column ID
					it.second.vColumns[x].iColumnID = (int)x;

					Clistview::enType type;

					switch (it.second.vColumns[x].type)
					{
					case cui_raw::String:
						type = Clistview::enType::String;
						break;

					case cui_raw::Char:
						type = Clistview::enType::Char;
						break;

					case cui_raw::Int:
						type = Clistview::enType::Int;
						break;

					case cui_raw::Float:
						type = Clistview::enType::Float;
						break;

					default:
						type = Clistview::enType::String;
						break;
					}

					// add columns
					it.second.pClistview->AddColumn((int)x, it.second.vColumns[x].sColumnName,
						int(0.5 + it.second.vColumns[x].iWidth * pThis->d->m_DPIScale), type);
				}

				// populate listview
				int iRowNumber = 0;
				for (auto &row : it.second.vData)
				{
					for (auto &item : row.vItems)
					{
						std::basic_string<TCHAR> sColumnName = item.sColumnName;
						std::basic_string<TCHAR> sItemData = item.sItemData;

						// ignore user supplied row number
						item.iRowNumber = iRowNumber;

						int iColumnNumber = -1;

						// determine column number
						for (size_t i = 0; i < it.second.vColumns.size(); i++)
						{
							if (it.second.vColumns[i].sColumnName == sColumnName)
							{
								iColumnNumber = it.second.vColumns[i].iColumnID;
								break;
							}
						}

						// insert item
						if (iColumnNumber != -1) // failsafe in-case there's a typo in column name
						{
							it.second.pClistview->InsertItem(iRowNumber, iColumnNumber, sItemData);
						}
					}

					iRowNumber++;
				}

				// subclass listview control so we can do some mumbo-jumbo!!!
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)listviewControlProc);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add edit and updown controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_EditControls)
		{
			if (!it.second.bUpDown)
			{
				// this is a standard edit control

				if (!IsWindow(it.second.hWnd))
				{
					DWORD dwStyle = WS_TABSTOP | WS_CHILD | WS_BORDER | ES_LEFT;

					if (it.second.bMultiLine)
					{
						dwStyle |= ES_MULTILINE | ES_AUTOVSCROLL;

						if (it.second.bScrollBar)
							dwStyle |= WS_VSCROLL;
					}
					else
						dwStyle |= ES_AUTOHSCROLL;

					if (it.second.bPassword)
						dwStyle |= ES_PASSWORD;

					if (it.second.bReadOnly)
						dwStyle |= ES_READONLY;

					it.second.hWnd = CreateWindow( WC_EDIT, _T(""), dwStyle,
						it.second.coords.left,
						it.second.coords.top,
						it.second.coords.width(),
						it.second.coords.height(),
						hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

					if (!it.second.bPageLess)
						captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
					else
						capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

					// set cue banner
					if (!it.second.sCueBanner.empty())
					{
						std::basic_string<TCHAR> sText = it.second.sCueBanner;
						SendMessage(it.second.hWnd, EM_SETCUEBANNER, TRUE,
							(LPARAM)(sText.c_str()));
					}

					// limit number of characters
					if (it.second.iLimit > 0)
						SendMessage(it.second.hWnd, EM_SETLIMITTEXT, it.second.iLimit, 0);

					// set edit control font
					HDC hdc = GetDC(it.second.hWnd);

					it.second.hfont = XCreateFont(hdc, it.second.sFontName, it.second.iFontSize);
					SetWindowFont(it.second.hWnd, it.second.hfont, TRUE);

					ReleaseDC(it.second.hWnd, hdc);

					// subclass edit control so we can do custom drawing
					SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
					it.second.PrevProc =
						SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)EditControlProc);

					// set resizing behaviour
					pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
						it.second.resize.iPercV, it.second.resize.iPercCX,
						it.second.resize.iPercCY);
				}
			}
			else
			{
				// this is an upDown control

				if (!IsWindow(it.second.hWnd))
				{
					it.second.d = pThis->d;

					// create a buddy window (edit control)
					DWORD dwStyle = WS_TABSTOP | WS_CHILDWINDOW | WS_BORDER | ES_NUMBER | ES_LEFT;

					it.second.hWnd = CreateWindow(WC_EDIT, _T(""), dwStyle,
						it.second.coords.left,
						it.second.coords.top,
						it.second.coords.width(),
						it.second.coords.height(),
						hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

					if (!it.second.bPageLess)
						captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
					else
						capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

					// set edit control font
					HDC hdc = GetDC(it.second.hWnd);

					it.second.hfont = XCreateFont(hdc, it.second.sFontName, it.second.iFontSize);
					SetWindowFont(it.second.hWnd, it.second.hfont, TRUE);

					ReleaseDC(it.second.hWnd, hdc);

					// subclass control so we can do custom drawing
					SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
					it.second.PrevProc =
						SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)EditControlProc);

					// defer setting resize behavior until after the actual buddy control is added
				}

				if (!IsWindow(it.second.hWndUpDown))
				{
					// create an updown control
					DWORD dwStyle = WS_CHILDWINDOW | UDS_AUTOBUDDY | UDS_SETBUDDYINT |
						UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_HOTTRACK;

					it.second.hWndUpDown = CreateWindow(UPDOWN_CLASS, _T(""), dwStyle, 0, 0, 0, 0,
						hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

					// Sets the controls direction and range
					SendMessage(it.second.hWndUpDown, UDM_SETRANGE, 0,
						MAKELPARAM(it.second.iMax, it.second.iMin));

					// Set the position of the up-down control
					SendMessage(it.second.hWndUpDown, UDM_SETPOS, 0, LPARAM(it.second.iPos));

					// subclass updown control so we can do custom drawing
					SetWindowLongPtr(it.second.hWndUpDown, GWLP_USERDATA, (LONG_PTR)&it.second);
					it.second.UpDownPrevProc = SetWindowLongPtr(it.second.hWndUpDown, GWLP_WNDPROC,
						(LONG_PTR)UpDownControlProc);

					// set resizing behaviour of edit control
					// (after the actual updown control has been added)
					pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
						it.second.resize.iPercV, it.second.resize.iPercCX,
						it.second.resize.iPercCY);

					// set resizing behaviour of updown control
					pThis->d->m_pResizer->OnResize(it.second.hWndUpDown, it.second.resize.iPercH,
						it.second.resize.iPercV, it.second.resize.iPercCX,
						it.second.resize.iPercCY);
				}
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add date controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_DateControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				it.second.d = pThis->d;

				DWORD dwStyle = WS_TABSTOP | WS_CHILD;

				if (it.second.bAllowNone)
					dwStyle |= DTS_SHOWNONE;

				// add date control
				it.second.hWnd = CreateWindow(DATETIMEPICK_CLASS, _T(""), dwStyle,
					it.second.coords.left,
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// set date control font
				HDC hdc = GetDC(it.second.hWnd);

				it.second.hfont = XCreateFont(hdc, it.second.sFontName, it.second.iFontSize);
				SetWindowFont(it.second.hWnd, it.second.hfont, TRUE);

				ReleaseDC(it.second.hWnd, hdc);

				// set date control format, e.g. 13-Jul-2013
				DateTime_SetFormat(it.second.hWnd, _T("dd-MMM-yyyy"));

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)DateControlProc);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add time controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_TimeControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				it.second.d = pThis->d;

				DWORD dwStyle = WS_TABSTOP | WS_CHILD;

				if (it.second.bAllowNone)
					dwStyle |= DTS_SHOWNONE;

				dwStyle |= DTS_TIMEFORMAT;

				it.second.hWnd = CreateWindow(DATETIMEPICK_CLASS, _T(""), dwStyle,
					it.second.coords.left,
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// set date control font
				HDC hdc = GetDC(it.second.hWnd);

				it.second.hfont = XCreateFont(hdc, it.second.sFontName, it.second.iFontSize);
				SetWindowFont(it.second.hWnd, it.second.hfont, TRUE);

				ReleaseDC(it.second.hWnd, hdc);

				// set time format
				DateTime_SetFormat(it.second.hWnd, _T("h:mm tt"));	//example: 07:45 PM

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)TimeControlProc);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add bar chart controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_BarChartControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				it.second.d = pThis->d;

				DWORD dwStyle = WS_CHILD;

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
					it.second.coords.left,
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)BarChartControlProc);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add line chart controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_LineChartControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				it.second.d = pThis->d;

				DWORD dwStyle = WS_CHILD;

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
					it.second.coords.left,
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)LineChartControlProc);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add pie chart controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_PieChartControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				it.second.d = pThis->d;

				DWORD dwStyle = WS_CHILD;

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
					it.second.coords.left,
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)PieChartControlProc);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add rich edit controls
	try
	{
		if (pThis->d->hRichEdit)
		{
			for (auto &it : pThis->d->m_Pages.at(sPage).m_RichEditControls)
			{
				if (!IsWindow(it.second.hWnd))
				{
					it.second.d = pThis->d;

					DWORD dwStyle =
						WS_CHILD | ES_MULTILINE | ES_WANTRETURN | ES_AUTOVSCROLL | WS_VSCROLL |
						ES_NOHIDESEL;	// show selected text even when it's out of focus

					// NOTE: PFA_JUSTIFY doesn't work (or rather apply)
					// when ES_AUTOHSCROLL and WS_HSCROLL are set
					bool bHScroll = false;
					if (bHScroll)
						dwStyle |= ES_AUTOHSCROLL | WS_HSCROLL;

					if (it.second.bReadOnly)
						dwStyle |= ES_READONLY;

					it.second.hWnd = CreateWindow(pThis->d->sRichEditClass.c_str(),
						_T(""), dwStyle,
						it.second.coords.left,
						it.second.coords.top,
						it.second.coords.width(),
						it.second.coords.height(),
						hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

					if (!it.second.bPageLess)
						captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
					else
						capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

					CHARFORMAT cf;
					cf.cbSize = sizeof(cf);

					// set default font
					memset(&cf, 0, sizeof cf);
					cf.cbSize = sizeof cf;
					cf.dwMask = CFM_FACE | CFM_SIZE | CFM_BOLD | CFM_ITALIC | CFM_UNDERLINE;
					cf.dwEffects = 0;
					cf.yHeight = 200;// TO-DO: find a way of translating from it.second.iFontSize
					lstrcpyn(cf.szFaceName, it.second.sFontName.c_str(), _countof(cf.szFaceName));
					SendMessage(it.second.hWnd, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);

					// enable advanced typography options (for justify and such)
					SendMessage(it.second.hWnd, EM_SETTYPOGRAPHYOPTIONS,
						TO_ADVANCEDTYPOGRAPHY, TO_ADVANCEDTYPOGRAPHY);

					// tell the control which notifications to send to the parent
					// (send this message last for performance!)
					SendMessage(it.second.hWnd, EM_SETEVENTMASK, 0, ENM_SELCHANGE);

					// subclass control so we can do custom drawing
					SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
					it.second.PrevProc = SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC,
						(LONG_PTR)RichEditControlProc);

					// disable numbering type combobox
					pThis->hideControl(sPage, it.second.iListType);

					// set resizing behaviour
					pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
						it.second.resize.iPercV, it.second.resize.iPercCX,
						it.second.resize.iPercCY);
				}
			}
		}
		else
		{
			// the rich edit library is not available
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add toggle button controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_ToggleButtonControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				it.second.d = pThis->d;

				DWORD dwStyle = WS_CHILD;

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
					it.second.coords.left,
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)ToggleBtnProc);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add star rating controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_StarRatingControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				it.second.d = pThis->d;

				DWORD dwStyle = WS_CHILD;

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
					it.second.coords.left,
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)StarRatingProc);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add progress controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_ProgressControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				it.second.d = pThis->d;

				DWORD dwStyle = WS_CHILD;

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
					it.second.coords.left,
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)ProgressProc);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add password strength controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_PasswordStrengthControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				it.second.d = pThis->d;

				DWORD dwStyle = WS_CHILD;

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
					it.second.coords.left,
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)PasswordStrengthProc);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add selector controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_SelectorControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				it.second.d = pThis->d;

				DWORD dwStyle = WS_CHILD;

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle,
					it.second.coords.left,
					it.second.coords.top,
					it.second.coords.width(),
					it.second.coords.height(),
					hWnd, (HMENU)(INT_PTR)it.second.iUniqueID, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)SelectorProc);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}

	// add Image button controls
	try
	{
		for (auto &it : pThis->d->m_Pages.at(sPage).m_ImageControls)
		{
			if (!IsWindow(it.second.hWnd))
			{
				it.second.d = pThis->d;

				DWORD dwStyle = WS_CHILD;

				int x, y, cx, cy;

				if (it.second.bToggle)
				{
					switch (it.second.toggleAction)
					{
					case cui_raw::toggleLeft:
					{
						x = it.second.coords.left + it.second.iOffset;
						y = it.second.coords.top;
						cx = it.second.coords.width() - it.second.iOffset;
						cy = it.second.coords.height();
					}
					break;

					case cui_raw::toggleRight:
					{
						x = it.second.coords.left;
						y = it.second.coords.top;
						cx = it.second.coords.width() - it.second.iOffset;
						cy = it.second.coords.height();
					}
					break;

					case cui_raw::toggleUp:
					{
						x = it.second.coords.left;
						y = it.second.coords.top + it.second.iOffset;
						cx = it.second.coords.width();
						cy = it.second.coords.height() - it.second.iOffset;
					}
					break;

					case cui_raw::toggleDown:
					{
						x = it.second.coords.left;
						y = it.second.coords.top;
						cx = it.second.coords.width();
						cy = it.second.coords.height() - it.second.iOffset;
					}
					break;

					default:
					{
						x = it.second.coords.left;
						y = it.second.coords.top + it.second.iOffset;
						cx = it.second.coords.width();
						cy = it.second.coords.height() - it.second.iOffset;
					}
					break;
					}
				}
				else
				{
					x = it.second.coords.left;
					y = it.second.coords.top;
					cx = it.second.coords.width();
					cy = it.second.coords.height();
				}

				it.second.hWnd = CreateWindow(WC_STATIC, _T(""), dwStyle, x, y, cx, cy, hWnd,
					NULL, pThis->d->hInstance_, NULL);

				if (!it.second.bPageLess)
					captureControls(sPage, it.second.iUniqueID, it.second.hWnd);
				else
					capturePagelessControls(it.second.iUniqueID, it.second.hWnd);

				if (it.second.iPNGResource)
				{
					HMODULE resource_module = pThis->d->m_hResModule;

					if (it.second.resource_module)
						resource_module = it.second.resource_module;

					// attempt to load image from PNG resource
					std::basic_string<TCHAR> sErr;
					bool bRes = it.second.GdiplusBitmap_res.Load(it.second.iPNGResource,
						_T("PNG"), sErr, resource_module);
				}
				else
					if (!it.second.sFileName.empty())
					{
						// attempt to load image from file

						// TO-DO: implement error response
						std::basic_string<TCHAR> sErr;
						it.second.GdiplusBitmap.Load(it.second.sFileName.c_str(), sErr);
					}

				// subclass control so we can do custom drawing
				SetWindowLongPtr(it.second.hWnd, GWLP_USERDATA, (LONG_PTR)&it.second);
				it.second.PrevProc =
					SetWindowLongPtr(it.second.hWnd, GWLP_WNDPROC, (LONG_PTR)ImageProc);

				// set resizing behaviour
				pThis->d->m_pResizer->OnResize(it.second.hWnd, it.second.resize.iPercH,
					it.second.resize.iPercV, it.second.resize.iPercCX, it.second.resize.iPercCY);
			}
		}
	}
	catch (std::exception &e)
	{
		std::string m_sErr = e.what();
	}
} // AddControls
