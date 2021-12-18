//
// RichEdit.cpp - rich edit control implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#include "../cui_rawImpl.h"
#include "../../HlpFxs/HlpFxs.h"

LRESULT CALLBACK cui_rawImpl::RichEditControlProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
	cui_rawImpl::RichEditControl* pThis = reinterpret_cast<cui_rawImpl::RichEditControl*>(ptr);

	// Any messages we don't process must be passed onto the original window function
	return CallWindowProc((WNDPROC)pThis->PrevProc, hWnd, msg, wParam, lParam);
} // RichEditControlProc

void cui_rawImpl::OnRichEdit_COMMAND(HWND hWnd, WPARAM wParam, cui_rawImpl* d, cui_raw* pThis)
{
	int iID = LOWORD(wParam);

	bool bFound = false;

	for (auto &page : pThis->d->m_Pages)
	{
		for (auto &ctrl : page.second.m_RichEditControls)
		{
			std::basic_string<TCHAR> sErr;

			if (true)
			{
				// handle font things

				// Set up the CHARFORMAT structure
				CHARFORMAT cfm;
				cfm.cbSize = sizeof(cfm);

				// Get char format
				::SendMessage(ctrl.second.hWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm);

				// Set up the CHARFORMAT2 structure
				CHARFORMAT2 cfm2;
				cfm2.cbSize = sizeof(cfm2);

				// Get char format
				::SendMessage(ctrl.second.hWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm2);

				auto handleCtrl = [&](DWORD dwEffect, int iEffectID)
				{
					if (dwEffect == CFE_SUBSCRIPT || dwEffect == CFE_SUPERSCRIPT)
					{
						// CHARFORMAT2 controls
						if (cfm2.dwEffects & dwEffect)
						{
							pThis->setImageColors(page.first, iEffectID, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
								RGB(0, 0, 150), RGB(0, 0, 150), true, sErr);
						}
						else
						{
							pThis->setImageColors(page.first, iEffectID, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
								RGB(255, 255, 255), RGB(255, 255, 255), true, sErr);
						}
					}
					else
					{
						// CHARFORMAT controls
						if (cfm.dwEffects & dwEffect)
						{
							pThis->setImageColors(page.first, iEffectID, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
								RGB(0, 0, 150), RGB(0, 0, 150), true, sErr);
						}
						else
						{
							pThis->setImageColors(page.first, iEffectID, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
								RGB(255, 255, 255), RGB(255, 255, 255), true, sErr);
						}
					}
				};

				if (iID == ctrl.second.iBold)
				{
					bFound = true;

					// handle bold things
					if (cfm.dwEffects & CFE_BOLD)
					{
						// text is already bold ... make it not
						cfm.dwEffects = NULL;
					}
					else
						cfm.dwEffects = CFE_BOLD;

					// Set the new effects
					cfm.dwMask = CFM_BOLD;

					// Set the new format
					::SendMessage(ctrl.second.hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm);

					// handle bold
					handleCtrl(CFE_BOLD, ctrl.second.iBold);
				}
				else
					if (iID == ctrl.second.iItalic)
					{
						bFound = true;

						// handle italic things
						if (cfm.dwEffects & CFE_ITALIC)
						{
							// text is already italic ... make it not
							cfm.dwEffects = NULL;
						}
						else
							cfm.dwEffects = CFE_ITALIC;

						// Set the new effects
						cfm.dwMask = CFM_ITALIC;

						// Set the new format
						::SendMessage(ctrl.second.hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm);

						// handle italic
						handleCtrl(CFE_ITALIC, ctrl.second.iItalic);
					}
					else
						if (iID == ctrl.second.iUnderline)
						{
							bFound = true;

							// handle underline things
							if (cfm.dwEffects & CFE_UNDERLINE)
							{
								// text is already underline ... make it not
								cfm.dwEffects = NULL;
							}
							else
								cfm.dwEffects = CFE_UNDERLINE;

							// Set the new effects
							cfm.dwMask = CFM_UNDERLINE;

							// Set the new format
							::SendMessage(ctrl.second.hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm);

							// handle underline
							handleCtrl(CFE_UNDERLINE, ctrl.second.iUnderline);
						}
						else
							if (iID == ctrl.second.iStrikethough)
							{
								bFound = true;

								// handle strikethrough things
								if (cfm.dwEffects & CFE_STRIKEOUT)
								{
									// text is already strikethrough ... make it not
									cfm.dwEffects = NULL;
								}
								else
									cfm.dwEffects = CFE_STRIKEOUT;

								// Set the new effects
								cfm.dwMask = CFM_STRIKEOUT;

								// Set the new format
								::SendMessage(ctrl.second.hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm);

								// handle strikethrough
								handleCtrl(CFE_STRIKEOUT, ctrl.second.iStrikethough);
							}
							else
								if (iID == ctrl.second.iSubscript)
								{
									bFound = true;

									// handle subscript things
									if (cfm2.dwEffects & CFE_SUBSCRIPT)
									{
										// text is already subscript ... make it not
										cfm2.dwEffects = NULL;
									}
									else
										cfm2.dwEffects = CFE_SUBSCRIPT;

									// Set the new effects
									cfm2.dwMask = CFM_SUBSCRIPT;

									// Set the new format
									::SendMessage(ctrl.second.hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm2);

									// handle subscript
									handleCtrl(CFE_SUBSCRIPT, ctrl.second.iSubscript);

									// handle superscript
									handleCtrl(CFE_SUPERSCRIPT, ctrl.second.iSuperscript);
								}
								else
									if (iID == ctrl.second.iSuperscript)
									{
										bFound = true;

										// handle superscript things
										if (cfm2.dwEffects & CFE_SUPERSCRIPT)
										{
											// text is already superscript ... make it not
											cfm2.dwEffects = NULL;
										}
										else
											cfm2.dwEffects = CFE_SUPERSCRIPT;

										// Set the new effects
										cfm2.dwMask = CFM_SUPERSCRIPT;

										// Set the new format
										::SendMessage(ctrl.second.hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm2);

										// handle subscript
										handleCtrl(CFE_SUBSCRIPT, ctrl.second.iSubscript);

										// handle superscript
										handleCtrl(CFE_SUPERSCRIPT, ctrl.second.iSuperscript);
									}
									else
										if (iID == ctrl.second.iLarger)
										{
											bFound = true;

											cfm.dwMask = CFM_SIZE;

											if (cfm.yHeight < 12 * 20)
											{
												// increase by 1pt
												cfm.yHeight += 1 * 20;
											}
											else
												if (cfm.yHeight < 28 * 20)
												{
													// increase by 2pt
													cfm.yHeight += 2 * 20;
												}
												else
												{
													// increase by 10pt
													cfm.yHeight += 10 * 20;
												}

											SendMessage(ctrl.second.hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm);
										}
										else
											if (iID == ctrl.second.iSmaller)
											{
												bFound = true;

												cfm.dwMask = CFM_SIZE;

												if (cfm.yHeight <= 1 * 20)	// font size 1pt
												{
													// leave as-is
												}
												else
													if (cfm.yHeight <= 12 * 20)
													{
														// decrease by 1pt
														cfm.yHeight -= 1 * 20;
													}
													else
														if (cfm.yHeight <= 28 * 20)
														{
															// decrease by 2pt
															cfm.yHeight -= 2 * 20;
														}
														else
														{
															// decrease by 10pt
															cfm.yHeight -= 10 * 20;
														}

												SendMessage(ctrl.second.hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm);
											}
											else
												if (iID == ctrl.second.iFontColor)
												{
													bFound = true;

													// Set up the CHARFORMAT structure
													CHARFORMAT cfm;
													cfm.cbSize = sizeof(cfm);

													::SendMessage(ctrl.second.hWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm);

													// Set the new effects
													bool bColorPicked = false;
													COLORREF rgb = RGB(0, 0, 0);
													pThis->pickColor(bColorPicked, rgb);

													if (bColorPicked)
													{
														cfm.crTextColor = rgb;

														if (cfm.dwEffects & CFE_BOLD)
															cfm.dwEffects = CFE_BOLD;
														else
															cfm.dwEffects = NULL;

														// for some reason if CFM_COLOR is standing alone it's not working
														cfm.dwMask = CFM_BOLD | CFM_COLOR;

														// Set the new format
														::SendMessage(ctrl.second.hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm);

														// change the image bar to match the selected color
														pThis->setImageBar(page.first, ctrl.second.iFontColor, rgb, true, sErr);
													}

													SetFocus(ctrl.second.hWnd);	// set focus to rich edit control
												}
												else
													if (iID == ctrl.second.iIDFontSize)
													{
														HWND hWndItem = GetDlgItem(hWnd, ctrl.second.iIDFontSize);

														bFound = true;

														int iMsg = HIWORD(wParam);

														if (iMsg == CBN_SELCHANGE)
														{
															// check index of currently selected item
															int iIndex = (int)SendMessage(hWndItem, CB_GETCURSEL, NULL, NULL);

															if (iIndex == CB_ERR)
															{
																// no item is selected
															}
															else
															{
																/*
																** explicitly command the control to select this item
																** this will enable calls by GetDlgItemText to be successful
																*/
																SendMessage(hWndItem, CB_SETCURSEL, iIndex, NULL);

																// get the combo's text
																auto getWinText = [](HWND hWnd)
																{
																	int i = GetWindowTextLength(hWnd) + 1;

																	std::basic_string<TCHAR> sText;

																	TCHAR *buffer = new TCHAR[i];
																	GetWindowText(hWnd, buffer, i);
																	sText = buffer;
																	delete buffer;

																	return sText;
																}; // getWinText

																std::basic_string<TCHAR> sFontSize = getWinText(hWndItem);

																double iPtSize = 10.0;

																OutputDebugString(_T("-->ComboText: "));
																OutputDebugString(sFontSize.c_str());
																OutputDebugString(_T(" ...\r\n"));

																iPtSize = _ttof(sFontSize.c_str());

																// set font size
																cfm.dwMask = CFM_SIZE;
																cfm.yHeight = LONG((iPtSize * 20.0) + 0.5);

																SendMessage(ctrl.second.hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm);
																SetFocus(ctrl.second.hWnd);	// set focus to rich edit control
															}
														}
													}
													else
														if (iID == ctrl.second.iIDFontList)
														{
															HWND hWndItem = GetDlgItem(hWnd, ctrl.second.iIDFontList);

															bFound = true;

															int iMsg = HIWORD(wParam);

															if (iMsg == CBN_SELCHANGE)
															{
																// check index of currently selected item
																int iIndex = (int)SendMessage(hWndItem, CB_GETCURSEL, NULL, NULL);

																if (iIndex == CB_ERR)
																{
																	// no item is selected
																}
																else
																{
																	/*
																	** explicitly command the control to select this item
																	** this will enable calls by GetDlgItemText to be successful
																	*/
																	SendMessage(hWndItem, CB_SETCURSEL, iIndex, NULL);

																	// get the combo's text
																	auto getWinText = [](HWND hWnd)
																	{
																		int i = GetWindowTextLength(hWnd) + 1;

																		std::basic_string<TCHAR> sText;

																		TCHAR *buffer = new TCHAR[i];
																		GetWindowText(hWnd, buffer, i);
																		sText = buffer;
																		delete buffer;

																		return sText;
																	}; // getWinText

																	std::basic_string<TCHAR> sFontName = getWinText(hWndItem);

																	// set font size
																	cfm.dwMask = CFM_FACE;

																	lstrcpynW(cfm.szFaceName, sFontName.c_str(), (int)sFontName.length() + 1);

																	SendMessage(ctrl.second.hWnd, EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm);
																	SetFocus(ctrl.second.hWnd);	// set focus to rich edit control
																}
															}
														}

				// handle font size
				if (iID == ctrl.second.iLarger ||
					iID == ctrl.second.iSmaller
					)
				{
					std::basic_string<TCHAR> sFontSize = CRoundOff::tostr<TCHAR>((double(cfm.yHeight) / 20.0f), 1);

					if (sFontSize[sFontSize.length() - 1] == '0')
					{
						// remove decimal if it's null
						auto idx = sFontSize.find('.');
						sFontSize = sFontSize.erase(idx, 2);
					}

					pThis->setComboText(page.first, ctrl.second.iIDFontSize, sFontSize, sErr);
				}
			}

			if (!bFound)
			{
				// handle paragraph things

				// Set up the PARAFORMAT structure
				PARAFORMAT pfm;

				pfm.cbSize = sizeof(pfm);
				pfm.dwMask = PFM_ALIGNMENT;

				// Get para format
				::SendMessage(ctrl.second.hWnd, EM_GETPARAFORMAT, SCF_SELECTION, (LPARAM)&pfm);

				auto handleCtrl = [&](WORD wAlignment, int iEffectID)
				{
					if (pfm.wAlignment == wAlignment)
					{
						pThis->setImageColors(page.first, iEffectID, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
							RGB(0, 0, 150), RGB(0, 0, 150), true, sErr);
					}
					else
					{
						pThis->setImageColors(page.first, iEffectID, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
							RGB(255, 255, 255), RGB(255, 255, 255), true, sErr);
					}
				};

				if (iID == ctrl.second.iLeftAlign)
				{
					bFound = true;

					pfm.wAlignment = PFA_LEFT;
					SendMessage(ctrl.second.hWnd, EM_SETPARAFORMAT, SCF_SELECTION, (LPARAM)&pfm);
				}
				else
					if (iID == ctrl.second.iCenterAlign)
					{
						bFound = true;

						pfm.wAlignment = PFA_CENTER;
						SendMessage(ctrl.second.hWnd, EM_SETPARAFORMAT, SCF_SELECTION, (LPARAM)&pfm);
					}
					else
						if (iID == ctrl.second.iRightAlign)
						{
							bFound = true;

							pfm.wAlignment = PFA_RIGHT;
							SendMessage(ctrl.second.hWnd, EM_SETPARAFORMAT, SCF_SELECTION, (LPARAM)&pfm);
						}
						else
							if (iID == ctrl.second.iJustify)
							{
								bFound = true;

								pfm.wAlignment = PFA_JUSTIFY;
								SendMessage(ctrl.second.hWnd, EM_SETPARAFORMAT, SCF_SELECTION, (LPARAM)&pfm);
							}

				if (bFound)
				{
					// handle left
					handleCtrl(PFA_LEFT, ctrl.second.iLeftAlign);

					// handle center
					handleCtrl(PFA_CENTER, ctrl.second.iCenterAlign);

					// handle right
					handleCtrl(PFA_RIGHT, ctrl.second.iRightAlign);

					// handle justify
					handleCtrl(PFA_JUSTIFY, ctrl.second.iJustify);
				}
			}

			if (!bFound)
			{
				// Set up the PARAFORMAT2 structure
				PARAFORMAT2 pfm2;
				pfm2.cbSize = sizeof(pfm2);

				// Get para format
				::SendMessage(ctrl.second.hWnd, EM_GETPARAFORMAT, SCF_SELECTION, (LPARAM)&pfm2);

				// handle paragraph formatting (numbering)
				if (iID == ctrl.second.iList)
				{
					bFound = true;

					pfm2.dwMask = PFM_NUMBERING;

					if (pfm2.wNumbering == NULL)
					{
						pfm2.wNumbering = PFN_BULLET;
					}
					else
						pfm2.wNumbering = NULL;

					::SendMessage(ctrl.second.hWnd, EM_SETPARAFORMAT, SCF_SELECTION, (LPARAM)&pfm2);

					// show numbering state
					switch (pfm2.wNumbering)
					{
					case PFN_BULLET:
					{
						// working
						if (true)
							pThis->selectComboItem(page.first, ctrl.second.iListType, _T("• • •"), sErr);
					}
					break;

					case PFN_ARABIC:
					{
						if (pfm2.wNumberingStyle == PFNS_PERIOD)
							pThis->selectComboItem(page.first, ctrl.second.iListType, _T("1. 2. 3."), sErr);
					}
					break;

					case PFN_LCLETTER:
					{
						pThis->selectComboItem(page.first, ctrl.second.iListType, _T("a. b. c."), sErr);
					}
					break;

					case PFN_UCLETTER:
					{
						pThis->selectComboItem(page.first, ctrl.second.iListType, _T("A. B. C. "), sErr);
					}
					break;

					case PFN_LCROMAN:
					{
						pThis->selectComboItem(page.first, ctrl.second.iListType, _T("i. ii. iii."), sErr);
					}
					break;

					case PFN_UCROMAN:
					{
						pThis->selectComboItem(page.first, ctrl.second.iListType, _T("I. II. III. "), sErr);
					}
					break;

					case NULL:
					default:
						break;
					}

					if (pfm2.wNumbering == NULL)
					{
						pThis->setImageColors(page.first, ctrl.second.iList, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
							RGB(255, 255, 255), RGB(255, 255, 255), true, sErr);
						pThis->hideControl(page.first, ctrl.second.iListType);
					}
					else
					{
						pThis->setImageColors(page.first, ctrl.second.iList, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
							RGB(0, 0, 150), RGB(0, 0, 150), true, sErr);
						pThis->showControl(page.first, ctrl.second.iListType);
					}
				}
				else
					if (iID == ctrl.second.iListType)
					{
						// check list type
						HWND hWndItem = GetDlgItem(hWnd, ctrl.second.iListType);

						bFound = true;

						int iMsg = HIWORD(wParam);

						if (iMsg == CBN_SELCHANGE)
						{
							// check index of currently selected item
							int iIndex = (int)SendMessage(hWndItem, CB_GETCURSEL, NULL, NULL);

							if (iIndex == CB_ERR)
							{
								// no item is selected
							}
							else
							{
								/*
								** explicitly command the control to select this item
								** this will enable calls by GetDlgItemText to be successful
								*/
								SendMessage(hWndItem, CB_SETCURSEL, iIndex, NULL);

								// get the combo's text
								auto getWinText = [](HWND hWnd)
								{
									int i = GetWindowTextLength(hWnd) + 1;

									std::basic_string<TCHAR> sText;

									TCHAR *buffer = new TCHAR[i];
									GetWindowText(hWnd, buffer, i);
									sText = buffer;
									delete buffer;

									return sText;
								}; // getWinText

								std::basic_string<TCHAR> sListType = getWinText(hWndItem);
								OutputDebugString(_T("-->"));
								OutputDebugString(sListType.c_str());
								OutputDebugString(_T("\r\n"));

								pfm2.dwMask = PFM_NUMBERING | PFM_NUMBERINGSTYLE;

								if (sListType == _T("• • •"))
								{
									// change to bullet list
									pfm2.wNumbering = PFN_BULLET;
								}
								else
									if (sListType == _T("1. 2. 3."))
									{
										pfm2.wNumbering = PFN_ARABIC;
										pfm2.wNumberingStyle = PFNS_PERIOD;
									}
									else
										if (sListType == _T("a. b. c."))
										{
											pfm2.wNumbering = PFN_LCLETTER;
											pfm2.wNumberingStyle = PFNS_PERIOD;
										}
										else
											if (sListType == _T("A. B. C. "))
											{
												pfm2.wNumbering = PFN_UCLETTER;
												pfm2.wNumberingStyle = PFNS_PERIOD;
											}
											else
												if (sListType == _T("i. ii. iii."))
												{
													pfm2.wNumbering = PFN_LCROMAN;
													pfm2.wNumberingStyle = PFNS_PERIOD;
												}
												else
													if (sListType == _T("I. II. III. "))
													{
														pfm2.wNumbering = PFN_UCROMAN;
														pfm2.wNumberingStyle = PFNS_PERIOD;
													}

								::SendMessage(ctrl.second.hWnd, EM_SETPARAFORMAT, SCF_SELECTION, (LPARAM)&pfm2);

								SetFocus(ctrl.second.hWnd);	// set focus to rich edit control
							}
						}
					}
			}


		}
	}
} // OnRichEdit_COMMAND

LRESULT cui_rawImpl::OnRichEdit_NOTIFY(HWND hWnd, WPARAM wParam, LPARAM lParam, cui_rawImpl* d, cui_raw* pThis)
{
	int iControlID = (int)wParam;

	bool bFound = false;

	for (auto &page : pThis->d->m_Pages)
	{
		for (auto &ctrl : page.second.m_RichEditControls)
		{
			if (iControlID == ctrl.second.iUniqueID)
			{
				bFound = true;

				std::basic_string<TCHAR> sErr;

				// selection has changed in this rich edit control ... handle it ...

				// get a pointer to the SELCHANGE structure
				SELCHANGE *pSelChange = (SELCHANGE *)lParam;

				if (pSelChange)
				{
					switch (pSelChange->seltyp)
					{
					case SEL_TEXT:
					case SEL_EMPTY:
					{
						OutputDebugString(_T("-->SEL_TEXT/SEL_EMPTY ...\r\n"));

						// handle font formatting
						if (true)
						{
							// Set up the CHARFORMAT structure
							CHARFORMAT cfm;
							cfm.cbSize = sizeof(cfm);

							// Get char format
							::SendMessage(ctrl.second.hWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm);

							// Set up the CHARFORMAT2 structure
							CHARFORMAT2 cfm2;
							cfm2.cbSize = sizeof(cfm2);

							// Get char format
							::SendMessage(ctrl.second.hWnd, EM_GETCHARFORMAT, SCF_SELECTION, (LPARAM)&cfm2);

							auto handleCtrl = [&](DWORD dwEffect, int iEffectID)
							{
								if (dwEffect == CFE_SUBSCRIPT || dwEffect == CFE_SUPERSCRIPT)
								{
									// CHARFORMAT2 controls
									if (cfm2.dwEffects & dwEffect)
									{
										pThis->setImageColors(page.first, iEffectID, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
											RGB(0, 0, 150), RGB(0, 0, 150), true, sErr);
									}
									else
									{
										pThis->setImageColors(page.first, iEffectID, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
											RGB(255, 255, 255), RGB(255, 255, 255), true, sErr);
									}
								}
								else
								{
									// CHARFORMAT controls
									if (cfm.dwEffects & dwEffect)
									{
										pThis->setImageColors(page.first, iEffectID, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
											RGB(0, 0, 150), RGB(0, 0, 150), true, sErr);
									}
									else
									{
										pThis->setImageColors(page.first, iEffectID, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
											RGB(255, 255, 255), RGB(255, 255, 255), true, sErr);
									}
								}
							};

							// handle bold
							handleCtrl(CFE_BOLD, ctrl.second.iBold);

							// handle italic
							handleCtrl(CFE_ITALIC, ctrl.second.iItalic);

							// handle underline
							handleCtrl(CFE_UNDERLINE, ctrl.second.iUnderline);

							// handle strikethrough
							handleCtrl(CFE_STRIKEOUT, ctrl.second.iStrikethough);

							// handle subscript
							handleCtrl(CFE_SUBSCRIPT, ctrl.second.iSubscript);

							// handle superscript
							handleCtrl(CFE_SUPERSCRIPT, ctrl.second.iSuperscript);

							// handle font size
							std::basic_string<TCHAR> sFontSize = CRoundOff::tostr<TCHAR>((double(cfm.yHeight) / 20.0f), 1);

							if (sFontSize[sFontSize.length() - 1] == '0')
							{
								// remove decimal if it's null
								auto idx = sFontSize.find('.');
								sFontSize = sFontSize.erase(idx, 2);
							}

							pThis->setComboText(page.first, ctrl.second.iIDFontSize, sFontSize, sErr);

							// handle font name
							std::basic_string<TCHAR> sFontName(cfm.szFaceName);

							pThis->setComboText(page.first, ctrl.second.iIDFontList, sFontName, sErr);
						}

						// handle paragraph formatting (alignment)
						if (true)
						{
							// Set up the PARAFORMAT structure
							PARAFORMAT pfm;

							pfm.cbSize = sizeof(pfm);
							pfm.dwMask = PFM_ALIGNMENT;
							pfm.wAlignment = PFA_LEFT;

							// Get para format
							::SendMessage(ctrl.second.hWnd, EM_GETPARAFORMAT, SCF_SELECTION, (LPARAM)&pfm);

							auto handleCtrl = [&](WORD wAlignment, int iEffectID)
							{
								if (pfm.wAlignment == wAlignment)
								{
									pThis->setImageColors(page.first, iEffectID, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
										RGB(0, 0, 150), RGB(0, 0, 150), true, sErr);
								}
								else
								{
									pThis->setImageColors(page.first, iEffectID, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
										RGB(255, 255, 255), RGB(255, 255, 255), true, sErr);
								}
							};

							// handle left
							handleCtrl(PFA_LEFT, ctrl.second.iLeftAlign);

							// handle center
							handleCtrl(PFA_CENTER, ctrl.second.iCenterAlign);

							// handle right
							handleCtrl(PFA_RIGHT, ctrl.second.iRightAlign);

							// handle justify
							handleCtrl(PFA_JUSTIFY, ctrl.second.iJustify);

						}

						// handle paragraph formatting (numbering)
						if (true)
						{
							// Set up the PARAFORMAT2 structure
							PARAFORMAT2 pfm2;
							pfm2.dwMask = PFM_NUMBERING | PFM_NUMBERINGSTYLE;
							pfm2.cbSize = sizeof(pfm2);

							// Get para format
							::SendMessage(ctrl.second.hWnd, EM_GETPARAFORMAT, SCF_SELECTION, (LPARAM)&pfm2);

							switch (pfm2.wNumbering)
							{
							case PFN_BULLET:
							{
								// working
								if (true)
									pThis->selectComboItem(page.first, ctrl.second.iListType, _T("• • •"), sErr);
							}
							break;

							case PFN_ARABIC:
							{
								if (pfm2.wNumberingStyle == PFNS_PERIOD)
									pThis->selectComboItem(page.first, ctrl.second.iListType, _T("1. 2. 3."), sErr);
							}
							break;

							case PFN_LCLETTER:
							{
								pThis->selectComboItem(page.first, ctrl.second.iListType, _T("a. b. c."), sErr);
							}
							break;

							case PFN_UCLETTER:
							{
								pThis->selectComboItem(page.first, ctrl.second.iListType, _T("A. B. C. "), sErr);
							}
							break;

							case PFN_LCROMAN:
							{
								pThis->selectComboItem(page.first, ctrl.second.iListType, _T("i. ii. iii."), sErr);
							}
							break;

							case PFN_UCROMAN:
							{
								pThis->selectComboItem(page.first, ctrl.second.iListType, _T("I. II. III. "), sErr);
							}
							break;

							case NULL:
							default:
								break;
							}

							if (pfm2.wNumbering == NULL)
							{
								pThis->setImageColors(page.first, ctrl.second.iList, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
									RGB(255, 255, 255), RGB(255, 255, 255), true, sErr);
								pThis->hideControl(page.first, ctrl.second.iListType);
							}
							else
							{
								pThis->setImageColors(page.first, ctrl.second.iList, RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0), RGB(0, 0, 0),
									RGB(0, 0, 150), RGB(0, 0, 150), true, sErr);
								pThis->showControl(page.first, ctrl.second.iListType);
							}
						}
					}
					break;

					case SEL_OBJECT:
					{
						OutputDebugString(_T("-->SEL_OBJECT ...\r\n"));
					}
					break;

					case SEL_MULTICHAR:
					{
						OutputDebugString(_T("-->SEL_MULTICHAR ...\r\n"));
					}
					break;

					case SEL_MULTIOBJECT:
					{
						OutputDebugString(_T("-->SEL_MULTIOBJECT ...\r\n"));
					}
					break;

					default:
						break;
					}
				}
			}

			if (bFound)
				break;
		}

		if (bFound)
			break;
	}

	return 1;
} // OnRichEdit_NOTIFY
