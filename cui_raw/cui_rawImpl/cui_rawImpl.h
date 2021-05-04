/*
** cui_rawImpl.h - cui_rawImpl interface
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

#pragma once

#include <map>
#include <Windows.h>
#include <WindowsX.h>
#include <CommCtrl.h>
#include <GdiPlus.h>
#include <Richedit.h>

#include "../cui_raw.h"
#include "CMouseTrack/CMouseTrack.h"
#include "Clistview/CListView.h"
#include "CShadow/CShadow.h"
#include "../CImage/CImage.h"
#include "../CBrush/CBrush.h"
#include "../CPopupMenu/CPopupMenu.h"
#include "../CResizer/CResizer.h"
#include "../CGdiPlusBitmap/CGdiPlusBitmap.h"
#include "../CCriticalSection/CCriticalSection.h"

using namespace liblec::cui::gui_raw;

class liblec::cui::gui_raw::cui_rawImpl
{
public:
	cui_rawImpl();
	~cui_rawImpl();

	///////////////////////////////////////////////////////////////////////////////////////
	// functions

	// position a Gdiplus rectangle within another
	static void pos_rect(Gdiplus::RectF &rcInOut, const Gdiplus::RectF &rcTarget, float iPercH, float iPercV)
	{
		Gdiplus::RectF rcIn = rcInOut;

		Gdiplus::REAL iDeltaX = (rcTarget.GetRight() - rcTarget.GetLeft()) - (rcIn.GetRight() - rcIn.GetLeft());
		rcInOut.X = rcTarget.GetLeft() + (iPercH * iDeltaX) / 100.0f;

		Gdiplus::REAL iDeltaY = (rcTarget.GetBottom() - rcTarget.GetTop()) - (rcIn.GetBottom() - rcIn.GetTop());
		rcInOut.Y = rcTarget.GetTop() + (iPercV * iDeltaY) / 100.0f;

		Gdiplus::REAL right = rcInOut.GetLeft() + (rcIn.GetRight() - rcIn.GetLeft());
		Gdiplus::REAL bottom = rcInOut.GetTop() + (rcIn.GetBottom() - rcIn.GetTop());

		rcInOut.Width = right - rcInOut.X;
		rcInOut.Height = bottom - rcInOut.Y;
	} // pos_rect

	// align text rectangle to the layout rectangle
	static void align_text(Gdiplus::RectF &text_rect,
		const Gdiplus::RectF &layoutRect,
		cui_raw::textAlignment align)
	{
		// do not permit text_rect to be larger than layoutRect in either dimension
		text_rect.Width = min(text_rect.Width, layoutRect.Width);
		text_rect.Height = min(text_rect.Height, layoutRect.Height);

		// align the text rectangle to the layout rectangle
		switch (align)
		{
		case cui_raw::topleft:
			pos_rect(text_rect, layoutRect, 0, 0);
			break;
		case cui_raw::center:
			pos_rect(text_rect, layoutRect, 50, 0);
			break;
		case cui_raw::topright:
			pos_rect(text_rect, layoutRect, 100, 0);
			break;
		case cui_raw::middleleft:
			pos_rect(text_rect, layoutRect, 0, 50);
			break;
		case cui_raw::middle:
			pos_rect(text_rect, layoutRect, 50, 50);
			break;
		case cui_raw::middleright:
			pos_rect(text_rect, layoutRect, 100, 50);
			break;
		case cui_raw::bottomleft:
			pos_rect(text_rect, layoutRect, 0, 100);
			break;
		case cui_raw::bottommiddle:
			pos_rect(text_rect, layoutRect, 50, 100);
			break;
		case cui_raw::bottomright:
			pos_rect(text_rect, layoutRect, 100, 100);
			break;
		default:
			pos_rect(text_rect, layoutRect, 0, 0);	// default to topleft
			break;
		}
	} // align_text

	static RECT convert_rect(const Gdiplus::RectF &rect)
	{
		RECT rect_;
		rect_.top = static_cast<LONG>(rect.GetTop());
		rect_.left = static_cast<LONG>(rect.GetLeft());
		rect_.right = static_cast<LONG>(rect.GetRight());
		rect_.bottom = static_cast<LONG>(rect.GetBottom());
		return rect_;
	}

	static Gdiplus::RectF convert_rect(const RECT &rect)
	{
		Gdiplus::RectF rect_;
		rect_.X = static_cast<Gdiplus::REAL>(rect.left);
		rect_.Y = static_cast<Gdiplus::REAL>(rect.top);
		rect_.Width = static_cast<Gdiplus::REAL>(rect.right - rect.left);
		rect_.Height = static_cast<Gdiplus::REAL>(rect.bottom - rect.top);
		return rect_;
	}

	/*
	** get coordinates of working area
	** NOTE: working area NOT screen so takes taskbar into account
	*/
	void GetWorkingArea(HWND hWnd, RECT &rectout);

	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

	bool CreateShadow(HWND hWnd);

	static LRESULT OnWM_NCHITTEST(HWND hWnd, LPARAM lParam, cui_rawImpl* d);
	static void OnWM_GETMINMAXINFO(HWND hWnd, WPARAM wParam, LPARAM lParam, cui_rawImpl* d);
	static void OnWM_PAINT(HWND hWnd, cui_rawImpl* d);
	static void OnWM_COMMAND(HWND hWnd, WPARAM wParam, cui_rawImpl* d, cui_raw* pThis);
	static void OnRichEdit_COMMAND(HWND hWnd, WPARAM wParam, cui_rawImpl* d, cui_raw* pThis);
	static void OnWM_LBUTTONUP(HWND hWnd, cui_rawImpl* d);
	static void OnWM_LBUTTONDOWN(HWND hWnd, cui_rawImpl* d);
	static void OnWM_LBUTTONDBLCLK(HWND hWnd, cui_rawImpl* d);
	static void OnWM_RBUTTONUP(HWND hWnd, cui_rawImpl* d);
	static void OnWM_RBUTTONDOWN(HWND hWnd, cui_rawImpl* d);
	static void OnWM_ERASEBKGND(HWND hWnd, WPARAM wParam, cui_rawImpl* d);
	static void OnWM_SIZE(HWND hWnd, WPARAM wParam, cui_rawImpl* d);
	static void OnWM_CREATE(HWND hWnd, cui_raw* pThis);
	static void OnWM_MOUSELEAVE(HWND hWnd, cui_rawImpl* d);
	static void OnWM_MOUSEMOVE(HWND hWnd, LPARAM lParam, cui_rawImpl* d);
	static LRESULT OnWM_NOTIFY(HWND hWnd, LPARAM lParam, cui_rawImpl* d);
	static LRESULT OnRichEdit_NOTIFY(HWND hWnd, WPARAM wParam, LPARAM lParam, cui_rawImpl* d, cui_raw* pThis);

	static LRESULT CALLBACK TextControlProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK ControlBtnProc(HWND, UINT, WPARAM, LPARAM);

	// returns true if (x, y) is within rc
	static bool insideRect(int x, int y, LPRECT lpRect);

	static LRESULT CALLBACK ComboBoxControlProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK ComboBoxEditControlProc(HWND, UINT, WPARAM, LPARAM);
	static BOOL Combo_IsExtended(HWND hWndCtl);
	static int Combo_FindString(HWND hWndCtl, INT indexStart, LPTSTR lpszFind);

	static LRESULT CALLBACK listviewControlProc(HWND, UINT, WPARAM, LPARAM);
	static int CALLBACK CompareListItems(LPARAM lParam1, LPARAM lParam2, LPARAM lParam);
	static LRESULT HandleCustomDraw(NMLVCUSTOMDRAW* pcd, HWND hlistview, std::vector<cui_raw::listviewRow> lvRows, std::vector<cui_raw::listviewColumn> lvColumns);
	static LRESULT CALLBACK EditControlProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK ToggleBtnProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK BtnProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK ProgressProc(HWND, UINT, WPARAM, LPARAM);
	void AddControls(HWND hWnd, std::basic_string<TCHAR> sPage, cui_raw* pThis);
	static LRESULT CALLBACK ImageProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK tabControlProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK RectProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK PasswordStrengthProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK SelectorProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK UpDownControlProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK DateControlProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK TimeControlProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK BarChartControlProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK LineChartControlProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK StarRatingProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK PieChartControlProc(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK RichEditControlProc(HWND, UINT, WPARAM, LPARAM);

	void handleTabControls(const std::basic_string<TCHAR> &sPageName, int iUniqueID);
	void captureControls(const std::basic_string<TCHAR> &sPageName, int iUniqueID, HWND hWnd);
	void capturePagelessControls(int iUniqueID, HWND hWnd);
	void hideControl(const std::basic_string<TCHAR> &sPageName, HWND hWnd);
	void showControl(const std::basic_string<TCHAR> &sPageName, HWND hWnd);

	///////////////////////////////////////////////////////////////////////////////////////
	// structs, enumerations and types

	class coordinates
	{
	public:
		int left;
		int right;
		int top;
		int bottom;

		int width() { return right - left; }
		int height() { return bottom - top; }
		RECT rect() {
			RECT rc;
			rc.left = left;
			rc.right = right;
			rc.top = top;
			rc.bottom = bottom;
			return rc;
		}
	};

	enum enumBtn
	{
		normal = 10,
		hover,
		pressed,
	};

	struct ToolTipControl
	{
		HWND hWndControl = NULL;
		HWND hWndTooltip = NULL;
		std::basic_string<TCHAR> sFontName;
		double iFontSize;
		COLORREF m_clrBackground = 0;
		COLORREF m_clrText = 0;
		COLORREF m_clrBorder = 0;
		std::basic_string<TCHAR> sText;
		bool bAllowToolTip = false;

		CShadow *m_pShadow = nullptr;

		bool bDontShowWithinControl = false;
		cui_rawImpl *d = nullptr;
	};

	struct ButtonControl
	{
		int iUniqueID;
		bool bPageLess = false;
		std::basic_string<TCHAR> sCaption;
		std::basic_string<TCHAR> sFontName;
		double iFontSize;
		coordinates coords;

		HWND hWnd = NULL;		// reserved
		bool bDefaultButton = false;	// reserved
		cui_raw::onResize resize;	// reserved
		LONG_PTR PrevProc = NULL;			// reserved
		cui_rawImpl* d = NULL;		// reserved
		enumBtn state;			// reserved
		bool bPressed = false;	// reserved

		ToolTipControl toolTip;

		HBITMAP hbm_buffer = nullptr;
	};

	struct TextControl
	{
		int iUniqueID;
		bool bPageLess = false;
		std::basic_string<TCHAR> sText, sTextDisplay;
		std::basic_string<TCHAR> sFontName;
		double iFontSize;
		coordinates coords;

		HWND hWnd = NULL;		// reserved
		COLORREF clrText;		// reserved
		COLORREF clrTextHot;	// reserved
		cui_raw::textAlignment align;	// reserved
		bool bMultiLine;		// multiline
		LONG_PTR PrevProc = NULL;			// super reserved
		cui_raw::onResize resize;	// reserved
		bool bDoubleClicked = false;	// reserved
		bool bPressed = false;			// reserved
		COLORREF clrBackground;	// reserved

		HBITMAP hbm_buffer = NULL;
		bool bHot = false;

		ToolTipControl toolTip;
		RECT rcText;

		bool bStatic = true;
		cui_rawImpl* d = NULL;
	};

	struct ComboBoxControl
	{
		int iUniqueID;
		bool bPageLess = false;
		std::basic_string<TCHAR> sSelectedItem;
		std::basic_string<TCHAR> sFontName;
		double iFontSize;
		coordinates coords;

		HWND hWnd = NULL;			// reserved
		bool bReadOnly;				// reserved
		bool bAutoComplete;			// reserved
		LONG_PTR PrevProc = NULL;				// super reserved
		cui_raw* pThis = NULL;			// reserved

		std::vector<std::basic_string<TCHAR>> vData;	// reserved
		HFONT hfont = NULL;								// reserved
		cui_raw::onResize resize;							// reserved

		BOOL fMouseDown = FALSE;
		BOOL fButtonDown = FALSE;

		HBITMAP hbm_buffer = NULL;
		cui_rawImpl* d = NULL;
		bool bFontList = false;

		int m_iMaxNameWidth = 0;
	};

	struct ComboBoxEditControl
	{
		HWND hWnd = NULL;
		LONG_PTR PrevProc = NULL;				// super reserved
	};

	struct listviewControl
	{
		int iUniqueID;
		bool bPageLess = false;
		std::basic_string<TCHAR> sFontName;
		double iFontSize;
		coordinates coords;

		HWND hWnd = NULL;								// reserved
		std::vector<cui_raw::listviewColumn> vColumns;		// reserved
		Clistview* pClistview = NULL;					// super reserved
		std::vector<cui_raw::listviewRow> vData;			// reserved
		std::vector<cui_raw::contextMenuItem> vContextMenu;// reserved
		LONG_PTR PrevProc = NULL;				// super reserved
		bool bSortByClickingColumn;	// reserved
		int nSortColumn = -1;		// reserved
		BOOL bSortAscending = TRUE;	// reserved
		HFONT hfont = NULL;			// reserved
		bool bGridLines;			// reserved
		bool bBorder;
		COLORREF clrBarChart;		// reserved
		cui_raw::onResize resize;		// reserved

		bool bDoubleClicked = false;	// reserved

		cui_rawImpl *d = nullptr;

		bool bBusy = false;

		std::basic_string<TCHAR> sUniqueColumnName;
	};

	struct EditControl
	{
		int iUniqueID;
		bool bPageLess = false;
		std::basic_string<TCHAR> sText;
		std::basic_string<TCHAR> sFontName;
		std::basic_string<TCHAR> sCueBanner;
		double iFontSize;
		coordinates coords;

		HWND hWnd = NULL;			// reserved
		COLORREF clrText;			// reserved
		cui_raw::textAlignment align;	// reserved
		bool bMultiLine = false;	// multiline
		bool bScrollBar = false;	// scroll bar for multiline text
		bool bPassword = false;		// password control
		LONG_PTR PrevProc = NULL;			// super reserved
		cui_raw::onResize resize;		// reserved
		HFONT hfont = NULL;			// reserved
		cui_rawImpl *d = nullptr;		// reserved

		int iMin = 0;
		int iMax = 100;
		int iPos = 50;

		HWND hWndUpDown = NULL;

		bool bUpDown = false;
		LONG_PTR UpDownPrevProc = NULL;	// super reserved

		HBITMAP hbm_buffer = NULL;

		BOOL fMouseDown = FALSE;
		BOOL fButtonDown = FALSE;

		bool bUp = false;
		bool bReadOnly = false;
		int iLimit = 0;
		std::basic_string<TCHAR> sAllowedCharacterSet;
		std::basic_string<TCHAR> sForbiddenCharacterSet;
		int iControlToInvoke = 0;
	};

	struct sortstruct
	{
		LPARAM lParamSort;
		HWND hlistview;
	};

	struct ControlBtn
	{
		int iUniqueID;
		HWND hWnd = NULL;
		bool bCustomHandle = false;
		cui_rawImpl* d = NULL;
		COLORREF clrBtn;
		COLORREF clrBtnHot;
		bool bHot = false;
		LONG_PTR PrevProc = NULL;
		enumBtn state;			// reserved
		bool bPressed = false;	// reserved
		HBITMAP hbm_buffer = nullptr;

		ToolTipControl toolTip;
	};

	struct ToggleButtonControl
	{
		int iUniqueID;
		bool bPageLess = false;
		std::basic_string<TCHAR> sCaptionOn;
		std::basic_string<TCHAR> sCaptionOff;
		std::basic_string<TCHAR> sFontName;
		COLORREF clrText;
		COLORREF clrOn;
		COLORREF clrOff;
		double iFontSize;
		coordinates coords;

		HWND hWnd = NULL;		// reserved
		bool bOn;				// reserved
		cui_raw::onResize resize;	// reserved

		LONG_PTR PrevProc = NULL;			// reserved
		cui_rawImpl* d = NULL;		// reserved
		RECT rcToggler;			// reserved
		enumBtn state;			// reserved
		bool bPressed = false;			// reserved
		bool bHot = false;				// reserved

		ToolTipControl toolTip;
		int iPercH = 0;

		bool bOldState;

		POINT ptStart;

		HBITMAP hbm_buffer = NULL;
	};

	struct ProgressControl
	{
		int iUniqueID;
		bool bPageLess = false;
		coordinates coords;
		RECT rcCurrent;			// reserved
		COLORREF clrBar;		// reserved
		COLORREF clrUnfilled;

		HWND hWnd = NULL;		// reserved
		cui_raw::onResize resize;	// reserved

		LONG_PTR PrevProc = NULL;			// reserved
		cui_rawImpl* d = NULL;		// reserved
		double iPercentage = 0;	// reserved

		bool bReverse = false;

		bool bBusy = false;

		bool bTimerRunning = false;

		bool bBackward = false;

		int iBusyPerc = 0;

		// for double buffering
		HBITMAP hbm_buffer = NULL;
	};

	struct PasswordStrengthControl
	{
		int iUniqueID;
		COLORREF clrUnfilled;
		bool bPageLess = false;
		coordinates coords;
		RECT rcCurrent;			// reserved

		HWND hWnd = NULL;		// reserved
		cui_raw::onResize resize;	// reserved

		LONG_PTR PrevProc = NULL;			// reserved
		cui_rawImpl* d = NULL;		// reserved
		double iPercentage = 0;	// reserved
	};

	struct ImageControl
	{
		int iUniqueID;
		bool bPageLess = false;
		std::basic_string<TCHAR> sText;
		std::basic_string<TCHAR> sFontName;
		std::basic_string<TCHAR> sFontNameDescription;
		double iFontSize;
		coordinates coords;

		HWND hWnd = NULL;		// reserved
		COLORREF clrText;		// reserved
		COLORREF clrTextHot;	// reserved

		bool bButtonBar;		// reserved
		COLORREF clrBorder;		// reserved
		COLORREF clrBorderHot;	// reserved

		COLORREF clrImage;		// reserved
		COLORREF clrImageHot;	// reserved

		COLORREF clrBackground;	// reserved
		COLORREF clrBackgroundHot;	// reserved
		COLORREF clrBar;		// reserved

		LONG_PTR PrevProc = NULL;			// super reserved
		cui_raw::onResize resize;	// reserved
		bool bPressed = false;			// reserved
		int iPNGResource = 0;			// PNG resource
		std::basic_string<TCHAR> sFileName;
		cui_rawImpl* d = NULL;		// reserved
		HWND hWndPic;			// reserved
		HWND hWndText;			// reserved
		WNDCLASSEX wcex;		// reserved
		CGdiPlusBitmapResource GdiplusBitmap_res;	// reserved
		CGdiPlusBitmap GdiplusBitmap;				// reserved
		Gdiplus::Bitmap* m_pDisplaybitmap = NULL;	// reserved

		bool bImageOnlyTightFit = false;

		bool bHot = false;		// reserved
		RECT rcText;			// reserved
		RECT rcBar;				// reserved
		RECT rcImage;			// reserved
		RECT rcActive;			// reserved

		const int iOffset = 2;	// reserved
		bool bOffset = false;	// reserved

		bool bToggle = true;					// reserved
		cui_raw::onToggle toggleAction;				// reserved

		bool bChangeColor = false;				// reserved

		cui_raw::imageTextPlacement textPlacement;	// reserved

		ToolTipControl toolTip;

		HBITMAP hbm_buffer = NULL;

		bool bDescriptive = false;
		std::basic_string<TCHAR> sDescription;
		SIZE imageSize;

		HMODULE resource_module = NULL;
	};

	struct tab
	{
		std::basic_string<TCHAR> sCaption;
		std::basic_string<TCHAR> sTooltip;
		RECT rcTab;						// reserved
		bool bHot = false;				// reserved
		bool bSelected = false;			// reserved
		bool bPressed = false;			// reserved
		std::map<int, HWND> m_Controls;	// reserved
	};

	struct tabControl
	{
		int iUniqueID = -123;		// to-do: remove magic number

		std::basic_string<TCHAR> sFontName;
		double iFontSize;

		COLORREF clrTabLine;

		coordinates coords;

		HWND hWnd = NULL;			// reserved
		cui_raw::onResize resize;		// reserved
		LONG_PTR PrevProc = NULL;	// reserved
		cui_rawImpl* d = NULL;			// reserved

		HBITMAP hbm_buffer = NULL;

		std::vector<tab> vTabs;		// reserved

		ToolTipControl toolTip;
	};

	struct RectControl
	{
		bool bPageLess = false;
		int iUniqueID;

		coordinates coords;

		COLORREF clr;				// reserved

		HWND hWnd = NULL;			// reserved
		cui_raw::onResize resize;		// reserved

		LONG_PTR PrevProc = NULL;	// reserved
		cui_rawImpl* d = NULL;			// reserved
	};

	struct SelectorControl
	{
		int iUniqueID;
		bool bPageLess = false;
		RECT rcCurrent;			// reserved

		enumBtn state;			// reserved

		std::basic_string<TCHAR> sText;
		std::basic_string<TCHAR> sFontName;
		double iFontSize;
		coordinates coords;

		COLORREF clrText;		// reserved
		COLORREF clrBar;		// reserved

		HWND hWnd = NULL;		// reserved
		cui_raw::onResize resize;	// reserved

		LONG_PTR PrevProc = NULL;			// reserved
		cui_rawImpl* d = NULL;		// reserved

		ToolTipControl toolTip;

		std::vector<cui_raw::selectorItem> vItems;

		int iSelectedItem = 0;
		RECT rcSelector;		// reserved

		bool bHot = false;
		int iPercV = 0;
		bool bPressed = false;

		int iOldSelectedItem = 0;

		POINT ptStart;

		int yUpper = 0;
		int yLower = 0;

		HBITMAP hbm_buffer = NULL;
	};

	struct DateControl
	{
		bool bPageLess = false;
		int iUniqueID;

		coordinates coords;

		COLORREF clr;			// reserved

		HWND hWnd = NULL;		// reserved
		cui_raw::onResize resize;	// reserved

		LONG_PTR PrevProc = NULL;			// reserved
		cui_rawImpl* d = NULL;		// reserved

		cui_raw::date dDate;

		std::basic_string<TCHAR> sFontName;
		double iFontSize;

		bool bAllowNone = false;

		HFONT hfont = NULL;
		bool bBusy = false;
	};

	struct TimeControl
	{
		bool bPageLess = false;
		int iUniqueID;

		coordinates coords;

		COLORREF clr;			// reserved

		HWND hWnd = NULL;		// reserved
		cui_raw::onResize resize;	// reserved

		LONG_PTR PrevProc = NULL;			// reserved
		cui_rawImpl* d = NULL;		// reserved

		cui_raw::time tTime;

		std::basic_string<TCHAR> sFontName;
		double iFontSize;

		bool bAllowNone = false;

		HFONT hfont = NULL;
		bool bBusy = false;

		HBITMAP hbm_buffer = NULL;
		BOOL fMouseDown = FALSE;
		BOOL fButtonDown = FALSE;
		bool bUp = false;
	};

	// chart bar info struct
	struct chartBarInfo
	{
		RECT rect;
		std::basic_string<TCHAR> sChartInfo;
		bool bHot = false;
		bool bPressed = false;	// not currently used ... overkill things
		std::basic_string<TCHAR> sTooltip;
		RECT rcLabel;
	};

	struct BarChartControl
	{
		bool bPageLess = false;
		int iUniqueID;

		coordinates coords;

		HWND hWnd = NULL;		// reserved
		cui_raw::onResize resize;	// reserved

		LONG_PTR PrevProc = NULL;			// reserved
		cui_rawImpl* d = NULL;		// reserved

		std::basic_string<TCHAR> sFontName;
		double iFontSize;

		HBITMAP hbm_buffer = NULL;

		ToolTipControl toolTip;

		//////////////////

		std::basic_string<TCHAR> sChartName;						// chart name
		std::basic_string<TCHAR> sXaxisLabel;					// x-axis label
		std::basic_string<TCHAR> sYaxisLabel;					// y-axis label
		int iLowerLimit;					// lower limit
		int iUpperLimit;					// upper limit
		bool bAutoScale;					// autoscale flag
		std::vector<cui_raw::barChartData> vValues;	// values to plot (v[x-value][y-value])

		std::vector<chartBarInfo> chartBarsInfo;		// information about chart bars

		bool bInfoCaptured = false;
	};

	// chart line info struct
	struct chartLineInfo
	{
		RECT rect;
		std::basic_string<TCHAR> sChartInfo;
		bool bHot = false;
	};

	struct lines
	{
		bool bHot = false;
		bool bPressed = false;	// not currently used ... overkill things
		RECT rcLabel;
		std::basic_string<TCHAR> sTooltip;
		std::basic_string<TCHAR> sSeriesName;
		std::vector<chartLineInfo> chartLinesInfo;
	};

	struct LineChartControl
	{
		bool bPageLess = false;
		int iUniqueID;

		coordinates coords;

		HWND hWnd = NULL;		// reserved
		cui_raw::onResize resize;	// reserved

		LONG_PTR PrevProc = NULL;			// reserved
		cui_rawImpl* d = NULL;		// reserved

		std::basic_string<TCHAR> sFontName;
		double iFontSize;

		HBITMAP hbm_buffer = NULL;

		ToolTipControl toolTip;

		//////////////////

		std::basic_string<TCHAR> sChartName;						// chart name
		std::basic_string<TCHAR> sXaxisLabel;					// x-axis label
		std::basic_string<TCHAR> sYaxisLabel;					// y-axis label
		int iLowerLimit;					// lower limit
		int iUpperLimit;					// upper limit
		bool bAutoScale;					// autoscale flag
		std::vector<cui_raw::lineInfo> vLines;	// lines to plot

		std::vector<lines> linesInfo;		// information about chart lines

		bool bInfoCaptured = false;
	};

	// pie chart item info struct
	struct pieChartItemInfo
	{
		int iNumber = 0;
		std::basic_string<TCHAR> sItemLabel;
		std::basic_string<TCHAR> sTooltip;
		Gdiplus::Region *pRegion = NULL;
		bool bHot = false;
		RECT rcLabel;
		bool bPressed = false;
	};

	struct PieChartControl
	{
		bool bPageLess = false;
		int iUniqueID;

		coordinates coords;

		HWND hWnd = NULL;		// reserved
		cui_raw::onResize resize;	// reserved

		LONG_PTR PrevProc = NULL;			// reserved
		cui_rawImpl* d = NULL;		// reserved

		std::basic_string<TCHAR> sFontName;
		double iFontSize;

		HBITMAP hbm_buffer = NULL;

		ToolTipControl toolTip;

		//////////////////

		std::basic_string<TCHAR> sChartName;	// chart name

		std::vector<cui_raw::pieChartData> vData;	// values to chart

		std::vector<pieChartItemInfo> chartBarsInfo;		// information about chart items

		bool bAutoColor = false;

		bool bInfoCaptured = false;

		bool bHot = false;
		RECT rcPieChart;
		cui_raw::pieChartHoverEffect hoverEffect = cui_raw::pieChartHoverEffect::glow;
		bool bDoughnut = false;
	};

	struct RichEditControl
	{
		bool bPageLess = false;
		int iUniqueID;

		coordinates coords;

		HWND hWnd = NULL;		// reserved
		cui_raw::onResize resize;	// reserved

		LONG_PTR PrevProc = NULL;			// reserved
		cui_rawImpl* d = NULL;		// reserved

		std::basic_string<TCHAR> sFontName;
		double iFontSize;

		bool bReadOnly = false;
		bool bBorder = true;

		int iIDLineLeft = -1, iIDLineRight = -1, iIDLineTop = -1, iIDLineBottom = -1;

		int iIDHSeparator = -1, iIDVSeparator = -1;
		int iIDFontList = -1, iIDFontSize = -1, iIDFontLabel = -1, iIDParagraphLabel = -1;
		int iBold = -1, iItalic = -1, iUnderline = -1, iStrikethough = -1, iSubscript = -1, iSuperscript = -1, iLarger = -1, iSmaller = -1, iFontColor = -1;

		int iLeftAlign = -1, iCenterAlign = -1, iRightAlign = -1, iJustify = -1;
		int iList = -1, iListType = -1;
	};

	struct StarRatingControl
	{
		int iUniqueID;
		bool bPageLess = false;
		COLORREF clrBorder;
		COLORREF clrOn;
		COLORREF clrOff;
		COLORREF clrHot;
		coordinates coords;

		RECT rcStarRating;

		struct star
		{
			bool bHot = false;
			RECT rc;
			COLORREF clr;
		};

		std::vector<star> rcStars;

		enumBtn state;			// reserved

		HWND hWnd = NULL;		// reserved
		int iRating = 0;		// reserved
		cui_raw::onResize resize;	// reserved

		LONG_PTR PrevProc = NULL;			// reserved
		cui_rawImpl* d = NULL;		// reserved
		bool bPressed = false;			// reserved
		bool bRightPressed = false;
		bool bHot = false;				// reserved
		int iHighestRating = 5;

		ToolTipControl toolTip;

		HBITMAP hbm_buffer = NULL;
		bool bStatic = false;
	};

	///////////////////////////////////////////////////////////////////////////////////////
	//parameters

	HMODULE m_hResModule;	// handle to resources DLL
	HWND m_hWnd, m_hWndParent;
	bool parent_was_enabled;
	int m_ix, m_iy, m_icx, m_icy;
	COLORREF m_clrBackground, m_clrTheme, m_clrThemeHot, m_clrThemeLight, m_clrThemeDarker, m_clrDisabled;
	HBRUSH m_hbrBackground;
	bool m_bCreated;
	std::basic_string<TCHAR> m_sTitle;
	std::basic_string<TCHAR> m_sCurrentPage;
	WNDCLASSEX wcex;
	bool m_bEnableResize;
	int m_iBoarderWidth;
	bool m_bMinbtn, m_bMaxbtn, m_bClosebtn;
	RECT m_rect_max_bk, m_rect_min_bk, m_rect_close_bk;
	int m_iTitlebarHeight;
	CShadow* m_pShadow;
	bool m_bCentershadow;
	bool m_bShadow;
	int m_iMinWidth, m_iMinHeight, m_iMinWidthCalc;
	CMouseTrack* m_pMouseTrack;

	int m_iIDClose, m_iIDMax, m_iIDMin;
	bool m_bMaximized;
	CResizer* m_pResizer;

	int m_IDI_ICON, m_IDI_ICONSMALL, m_IDP_ICONSMALL;
	int m_iStopQuit;

	HCURSOR m_hNormalCursor;
	HCURSOR m_hHotCursor;

	int m_iLeftBorder, m_iTopBorder, m_iRightBorder, m_iBottomBorder;

	int m_iShutdownID;

	// control buttons (appear on all pages at the top right)
	std::map<int, ControlBtn> m_ControlBtns;

	// borders (appear on all pages)
	std::map<int, RectControl> m_Borders;

	// timers
	std::map<int, int> m_Timers;

	// user supplied state information
	void *pState_user;

	// tooltip font parameters
	std::basic_string<TCHAR> m_sTooltipFont;
	double m_iTooltipFontSize;
	COLORREF m_clrTooltipText;
	COLORREF m_clrTooltipBackground;
	COLORREF m_clrTooltipBorder;

	bool m_bLButtonDown;
	bool m_bRButtonDown;

	int m_iMessageBoxes;

	bool m_bParentClosing;

	// children map
	std::map<cui_raw*, cui_raw*> m_Children;

	cui_raw* m_pcui_rawparent;

	double m_DPIScale;

	// TO-DO: remove magic number or define more formally/properly
	const unsigned int ID_TIMER = 14232238;

	// TO-DO: remove magic number or define more formally/properly
	const unsigned int ID_TIMER_CHECK = 14232239;

	bool m_bStartOnMouseMove;

	bool m_bStopOnMouseOverWindow;

	bool m_bTimerRunning;

	int m_iTimer;

	POINT m_ptStartCheck;

	bool m_bNotification;

	std::vector<HWND> m_rcExclude;

	/// <summary> Notification window structure. </summary>
	struct notStruct
	{
		/// <summary> Number of notification window. </summary>
		int iNumber = 0;

		/// <summary> Pointer to the notification window. </summary>
		cui_raw* pcui_raw = NULL;

		/// <summary> Notification window parameters. </summary>
		cui_raw::notParams params;

		/// <summary> Pointer to the main window's implementation. </summary>
		cui_rawImpl* d = NULL;

		/// <summary> Handle to the notification window thread. </summary>
		HANDLE hThread = NULL;
	};

	// notification window map
	std::map<cui_raw*, notStruct> m_nots;

	// critical section locker to protect access to m_nots (which will definitely be multithreaded)
	CCriticalSection m_locker_for_m_nots;

	// tray items
	UINT uID;

	struct trayIcon
	{
		int IDI_SMALLICON = 0;
		std::basic_string<TCHAR> sTitle;
		std::vector<cui_raw::trayIconItem> m_trayItems;
	}m_trayIcon;

	// page struct
	struct page
	{
		std::map<int, ButtonControl> m_ButtonControls;
		std::map<int, TextControl> m_TextControls;
		std::map<int, ComboBoxControl> m_ComboBoxControls;
		std::map<HWND, ComboBoxEditControl> m_ComboBoxEditControls;
		std::map<int, listviewControl> m_listviewControls;
		std::map<int, EditControl> m_EditControls;
		std::map<int, ToggleButtonControl> m_ToggleButtonControls;
		std::map<int, ProgressControl> m_ProgressControls;
		std::map<int, ImageControl> m_ImageControls;
		std::map<int, RectControl> m_RectControls;
		std::map<int, PasswordStrengthControl> m_PasswordStrengthControls;
		std::map<int, SelectorControl> m_SelectorControls;
		std::map<int, DateControl> m_DateControls;
		std::map<int, TimeControl> m_TimeControls;
		std::map<int, BarChartControl> m_BarChartControls;
		std::map<int, LineChartControl> m_LineChartControls;
		std::map<int, StarRatingControl> m_StarRatingControls;
		std::map<int, PieChartControl> m_PieChartControls;
		std::map<int, RichEditControl> m_RichEditControls;

		std::basic_string<TCHAR> m_sCurrentTab;
		tabControl m_TabControl;	// only one currently supported

		CommandProcedure m_pCommandProc = NULL;
	};

	// page list (for managing pages)
	std::vector<std::basic_string<TCHAR>> m_sPageList;

	// pages
	std::map<std::basic_string<TCHAR>, page> m_Pages;

	// controls [pageName, pageControls]
	std::map<std::basic_string<TCHAR>, std::vector<HWND>> m_vControls;
	std::map<std::basic_string<TCHAR>, std::vector<HWND>> m_vPagelessControls;
	std::map<std::basic_string<TCHAR>, std::vector<HWND>> m_vHiddenControls;

	void hidePage(std::basic_string<TCHAR> sPageName);
	void showPage(std::basic_string<TCHAR> sPageName);

	void hitControlButton(cui_rawImpl::ControlBtn &Control, POINT &pt, std::vector<RECT> &m_vHotRects);
	void hitButtonControl(cui_rawImpl::ButtonControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects);
	void hitToggleButtonControl(cui_rawImpl::ToggleButtonControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects);
	void hitImageControl(cui_rawImpl::ImageControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects);
	void hitSelectorControl(cui_rawImpl::SelectorControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects);
	void hitBarChartControl(cui_rawImpl::BarChartControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects);
	void hitLineChartControl(cui_rawImpl::LineChartControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects);
	void hitPieChartControl(cui_rawImpl::PieChartControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects);
	void hitTextControl(cui_rawImpl::TextControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects);
	void hitStarRatingControl(cui_rawImpl::StarRatingControl &Control, POINT &pt, std::vector<RECT> &m_vHotRects);

	void flagPressControlButton(cui_rawImpl::ControlBtn &Control, POINT &pt);
	void flagPressButtonControl(cui_rawImpl::ButtonControl &Control, POINT &pt);
	void flagPressToggleButtonControl(cui_rawImpl::ToggleButtonControl &Control, POINT &pt);
	void flagPressTextControl(cui_rawImpl::TextControl &Control, POINT &pt);
	void flagPressImageControl(cui_rawImpl::ImageControl &Control, POINT &pt);
	void flagPressSelectorControl(cui_rawImpl::SelectorControl &Control, POINT &pt);
	void flagPressStarRatingControl(cui_rawImpl::StarRatingControl &Control, POINT &pt);
	void flagPressPieChartControl(cui_rawImpl::PieChartControl &Control, POINT &pt);

	void flagRightPressStarRatingControl(cui_rawImpl::StarRatingControl &Control, POINT &pt);

	void checkPressControlButton(cui_rawImpl::ControlBtn &Control, POINT &pt);
	void checkPressButtonControl(cui_rawImpl::ButtonControl &Control, POINT &pt);
	void checkPressToggleButtonControl(cui_rawImpl::ToggleButtonControl &Control, POINT &pt);
	void checkPressTextControl(cui_rawImpl::TextControl &Control, POINT &pt);
	void checkPressImageControl(cui_rawImpl::ImageControl &Control, POINT &pt);
	void checkPressSelectorControl(cui_rawImpl::SelectorControl &Control, POINT &pt);
	void checkPressStarRatingControl(cui_rawImpl::StarRatingControl &Control, POINT &pt);
	void checkPressPieChartControl(cui_rawImpl::PieChartControl &Control, POINT &pt);

	void checkRightPressStarRatingControl(cui_rawImpl::StarRatingControl &Control, POINT &pt);

	void resetImageControl(cui_rawImpl::ImageControl &Control);

	void showTrayPopupMenu();

	UINT m_iRegID;
	int m_iCopyDataID;
	std::string m_sCopyData;
	int m_iDropFilesID;
	std::basic_string<TCHAR> m_sDropFile;
	int m_iWidth;	// to be captured each time the window is about to be minimized
	int m_iHeight;	// to be captured each time the window is about to be minimized

	HWND getControlHWND(const std::basic_string<TCHAR> &sPageName, std::vector<HWND> &vHWNDs, int iUniqueID);
	bool bFirstRun;
	std::vector<HWND> m_vPreventQuitList;
	HINSTANCE hRichEdit, hInstance_ = NULL;
	std::basic_string<TCHAR> sRichEditClass;
	int iAddToWM_APP;

	int GetNewMessageID();
	std::vector<int> vFontCombos;
	std::map<std::basic_string<TCHAR>, Gdiplus::Font*> vFonts;
	Gdiplus::PrivateFontCollection m_font_collection;
	std::vector<std::basic_string<TCHAR>> m_font_collection_files;
}; // cui_rawImpl
