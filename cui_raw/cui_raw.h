/*
** cui_raw.h - cui_raw framework - interface
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

#pragma once

#ifdef CUI_EXPORTS
#define cui_api __declspec(dllexport)
#include "../cui.h"
#else
#define cui_api __declspec(dllimport)
#include <liblec/cui.h>

#ifdef _WIN64

#ifdef _DEBUG
#pragma comment(lib, "cui64d.lib")
#else
#pragma comment(lib, "cui64.lib")
#endif // _DEBUG

#else

#ifdef _DEBUG
#pragma comment(lib, "cui32d.lib")
#else
#pragma comment(lib, "cui32.lib")
#endif // _DEBUG

#endif // _WIN64

#endif

#include <Windows.h>
#include <tchar.h>
#include <string>
#include <vector>

namespace liblec
{
	namespace cui
	{
		namespace gui_raw
		{
			class cui_raw;
			class cui_rawImpl;
			class CSplashScreenImpl;

			typedef void(*CommandProcedure)(cui_raw &ui, int iUniqueID, void *pData);

			/// <summary>
			/// Window class.
			/// </summary>
			/// 
			/// <remarks>
			/// Do not use IDs that are less than 20 to avoid conflict with built in Windows dialog box command
			/// IDs defined in WinUser.h, like IDOK(1), IDCANCEL(2) ... IDCONTINUE(11) ... etc
			/// 
			/// This class is a wrapper to a WIN32 window for rapid GUI application development.
			///	Any program that uses this class should declare itself as DPI-aware through its application manifest.
			/// Written by Alec T. Musasa, 22 April 2015.
			/// </remarks>
			class cui_api cui_raw
			{
			public:
				/// <summary>
				/// Predefined window position.
				/// </summary>
				enum windowPosition
				{
					/// <summary>
					/// Center the window to the client's working area. The working area is the entire screen minus the 
					/// task-bar.
					/// </summary> 
					centerToWorkingArea = 0,

					/// <summary>
					/// Center the window to the client's working area.
					/// </summary> 
					centerToParent,

					/// <summary>
					/// Place window on the top left of the client's working area.
					/// </summary> 
					topLeft,

					/// <summary>
					/// Place window on the top left of the client's working area but offset by ten pixels.
					/// </summary> 
					topLeftOffset,

					/// <summary>
					/// Place window on the bottom left of the client's working area.
					/// </summary> 
					bottomLeft,

					/// <summary>
					/// Place window on the bottom left of the client's working area but offset by ten pixels.
					/// </summary> 
					bottomLeftOffset,

					/// <summary>
					/// Place window on the top right of the client's working area.
					/// </summary> 
					topRight,

					/// <summary>
					/// Place window on the top right of the client's working area but offset by ten pixels.
					/// </summary> 
					topRightOffset,

					/// <summary>
					/// Place window on the bottom right of the client's working area.
					/// </summary> 
					bottomRight,

					/// <summary>
					/// Place window on the bottom right of the client's working area but offset by ten pixels.
					/// </summary> 
					bottomRightOffset,
				};

				/// <summary>Action on resize.</summary>
				struct onResize
				{
					/// <summary>
					/// The percentage rate for following the parent's right border.
					/// 0 = doesn't move horizontally, 100 = moves same number of pixels horizontally that the parent's 
					/// right border has moved.
					/// </summary>
					int iPercH = 0;

					/// <summary>The percentage rate for following the parent's bottom border.
					/// 0 = doesn't move vertically, 100 = moves same number of pixels vertically that the parent's 
					/// bottom border has moved.
					/// </summary>
					int iPercV = 0;

					/// <summary>
					/// The percentage rate for following the parent's change in width. 0 = doesn't follow change in 
					/// parent's width, 100 = width changes at same rate as that of parent.
					/// </summary>
					int iPercCX = 0;

					/// <summary>
					/// The percentage rate for following the parent's change in height. 0 = doesn't follow change in 
					/// parent's height, 100 = height changes at same rate as that of parent.
					/// </summary>
					int iPercCY = 0;
				};

				/// <summary>
				/// Text alignment.
				/// </summary>
				enum textAlignment
				{
					/// <summary>
					/// Place text on the top left.
					/// </summary>
					topleft,

					/// <summary>
					/// Place text on the top center.
					/// </summary>
					center,

					/// <summary>
					/// Place text on the top right.
					/// </summary>
					topright,

					/// <summary>
					/// Place text on the middle (vertically) left.
					/// </summary>
					middleleft,

					/// <summary>
					/// Place text in the middle (vertically and horizontally).
					/// </summary>
					middle,

					/// <summary>
					/// Place text on the middle (vertically) right.
					/// </summary>
					middleright,

					/// <summary>
					/// Place text on the bottom left.
					/// </summary>
					bottomleft,

					/// <summary>
					/// Place text on the bottom middle (vertically).
					/// </summary>
					bottommiddle,

					/// <summary>
					/// Place text on the bottom right.
					/// </summary>
					bottomright,
				};

				/// <summary>
				/// Type of listview column.
				/// </summary>
				enum listviewColumnType
				{
					/// <summary>
					/// Listview column for strings.
					/// </summary>
					String,

					/// <summary>
					/// Listview column for individual characters.
					/// </summary>
					Char,

					/// <summary>
					/// Listview column for integers.
					/// </summary>
					Int,

					/// <summary>
					/// Listview column for floating point numbers.
					/// </summary>
					Float
				};

				/// <summary>
				/// Toggle action when mouse moves over.
				/// </summary>
				enum onToggle
				{
					/// <summary>
					/// Toggle to the left when mouse moves over.
					/// </summary>
					toggleLeft,

					/// <summary>
					/// Toggle to the right when mouse moves over.
					/// </summary>
					toggleRight,

					/// <summary>
					/// Toggle upwards when mouse moves over.
					/// </summary>
					toggleUp,

					/// <summary>
					/// Toggle downwards when mouse moves over.
					/// </summary>
					toggleDown,
				};

				/// <summary>
				/// Listview column description.
				/// </summary>
				struct listviewColumn
				{
					/// <summary>
					/// Column ID - MUST be unique.
					/// </summary>
					int iColumnID;

					/// <summary>
					/// Listview column width, in pixels.
					/// </summary>
					int iWidth = 0;

					/// <summary>
					/// The name of the column.
					/// </summary>
					std::basic_string<TCHAR> sColumnName;

					/// <summary>
					/// Type of listview column.
					/// </summary>
					listviewColumnType type;

					/// <summary>
					/// Whether to display bar charts in the listview column.
					/// </summary>
					/// <remarks>
					/// TO-DO: find out why this won't work in Windows XP.
					/// </remarks>
					bool bBarChart = false;

					/// <summary>
					/// Upper limit of bar chart.
					/// </summary>
					int iBarChartMax = 100;

					/// <summary>
					/// Color of bar chart.
					/// </summary>
					COLORREF clrBarChart = RGB(190, 190, 255);

					/// <summary>
					/// Color of text in the column.
					/// </summary>
					COLORREF clrText = RGB(0, 0, 0);
				};

				/// <summary>
				/// Listview item description.
				/// </summary>
				struct listviewItem
				{
					/// <summary>
					/// Row number to place item in.
					/// </summary>
					int iRowNumber = 0;

					/// <summary>
					/// Name of column to place item in.
					/// </summary>
					std::basic_string<TCHAR> sColumnName;

					/// <summary>
					/// The data to place in the cell.
					/// </summary>
					std::basic_string<TCHAR> sItemData;

					/// <summary>
					/// Whether to customize this item's text.
					/// </summary>
					bool bCustom = false;

					/// <summary>
					/// The text color of an item.
					/// </summary>
					/// 
					/// <remarks>
					/// Used if listviewItem::bCustom if set to true.
					/// </remarks>
					COLORREF clrText;
				};

				/// <summary>
				/// Listview row description.
				/// </summary>
				struct listviewRow
				{
					/// <summary>
					/// Items in listview's row.
					/// </summary>
					std::vector<listviewItem> vItems;
				};

				/// <summary>
				/// Context menu item description.
				/// </summary>
				struct contextMenuItem
				{
					/// <summary>
					/// Unique ID of the context menu item.
					/// </summary>
					/// <remarks>
					/// This ID will be sent to the command procedure).
					/// </remarks>
					int iUniqueID;

					/// <summary>
					/// The label (text) in the menu item. Leave this empty to make a separator.
					/// </summary>
					std::basic_string<TCHAR> sLabel;

					/// <summary>
					/// Whether this is the default item in the context menu.
					/// </summary>
					/// <remarks>
					/// The default item will be displayed bold.
					/// </remarks>
					bool bDefault;

					/// <summary>
					/// The ID of the PNG resource to use in the context menu item.
					/// </summary>
					int IDC_PNGICON;
				};

				/// <summary>
				/// Selector item.
				/// </summary>
				struct selectorItem
				{
					/// <summary>
					/// Unique ID of the context menu item.
					/// </summary>
					/// 
					/// <remarks>
					/// This ID will be sent to the command procedure).
					/// </remarks>
					int iUniqueID;

					/// <summary>
					/// The label (text) to the right of the item.
					/// </summary>
					std::basic_string<TCHAR> sDescription;

					/// <summary>
					/// Reserved.
					/// </summary>
					int iYPos = 0;

					/// <summary>
					/// Reserved.
					/// </summary>
					int iDiff = 0;
				};

				/// <summary>
				/// Message sent to command procedure by a text control.
				/// </summary>
				/// 
				/// <remarks>
				/// A pointer to the object is sent to the command procedure through the window state parameter.
				/// </remarks>
				struct textMsg
				{
					/// <summary>
					/// Whether text control was double-clicked.
					/// </summary>
					bool bDoubleClick;
				};

				/// <summary>
				/// Message sent to the command procedure by a combobox control.
				/// </summary>
				/// 
				/// <remarks>
				/// A pointer to the object is sent to the command procedure through the window state parameter.
				/// </remarks>
				struct comboboxMsg
				{
					/// <summary>
					/// The currently selected item.
					/// </summary>
					std::basic_string<TCHAR> sSelected;
				};

				/// <summary>
				/// Message sent to the command procedure by a list view.
				/// </summary>
				/// <remarks>
				/// 
				/// A pointer to the object is sent to the command procedure through the window state parameter.
				/// </remarks>
				struct listviewMsg
				{
					/// <summary>
					/// The currently selected row.
					/// </summary>
					listviewRow row;

					/// <summary>
					/// Whether listview row was double clicked.
					/// </summary>
					/// 
					/// <remarks>
					/// TO-DO: add implementation for detecting when listview item is double clicked.
					/// </remarks>
					bool bDoubleClick = false;
				};

				/// <summary>
				/// Message sent to the command procedure by a list view popup menu.
				/// </summary>
				/// 
				/// <remarks>
				/// A pointer to the object is sent to the command procedure through the window state parameter.
				/// </remarks>
				struct listviewPopupMenuItemMsg
				{
					/// <summary>
					/// The currently selected rows.
					/// </summary>
					std::vector<listviewRow> vRows;

					/// <summary>
					/// The unique ID of the context menu item.
					/// </summary>
					int iUniqueID;

					/// <summary>
					/// The label (text) of the context menu item.
					/// </summary>
					std::basic_string<TCHAR> sLabel;
				};

				/// <summary>
				/// Message sent to the command procedure by a toggle button.
				/// </summary>
				/// 
				/// <remarks>
				/// A pointer to the object is sent to the command procedure through the window state parameter.
				/// </remarks>
				struct toggleButtonMsg
				{
					/// <summary>
					/// Whether the toggle button has just been turned on.
					/// </summary>
					bool bOn;
				};

				/// <summary>
				/// Message to be sent to the command procedure when another instance sends data to this instance.
				/// </summary>
				/// 
				/// <remarks>
				/// A pointer to the object is sent to the command procedure through the window state parameter.
				/// </remarks>
				struct copyDataMsg
				{
					/// <summary>
					/// The command line of the instance sending the data.
					/// </summary>
					std::string sCommandLine;
				};

				/// <summary>
				/// Message to be sent to the command procedure when the user drops a file onto the window.
				/// </summary>
				/// 
				/// <remarks>
				/// cui_raw::dropFilesAccept() must be called first in order for the dropFilesMsg to be sent to the
				/// command procedure.
				/// </remarks>
				struct dropFileMsg
				{
					/// <summary>
					/// The full path to the file or folder that the user has dropped onto the window.
					/// </summary>
					std::basic_string<TCHAR> sFullPath;
				};

				/// <summary>
				/// Message to be sent to the command procedure when the user clicks a pie chart.
				/// </summary>
				/// 
				/// <remarks>
				/// A pointer to the object is sent to the command procedure through the window state parameter.
				/// </remarks>
				struct pieChartMsg
				{
					/// <summary>
					/// The number of the pie item, as defined in cui_raw::pieChartData.
					/// </summary>
					int iNumber = 0;

					/// <summary>
					/// The label of the pie item, as defined in cui_raw::pieChartData.
					/// </summary>
					std::basic_string<TCHAR> sItemLabel;
				};

			public:
				/// <summary>
				/// Create a window object.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The name of the home page.
				/// </param>
				/// 
				/// <param name="pCommandProc">
				/// The command procedure.
				/// </param>
				/// 
				/// <param name="clrBackground">
				/// The window's background color.
				/// </param>
				/// 
				/// <param name="clrTheme">
				/// The window's theme color.
				/// </param>
				/// 
				/// <param name="clrThemeHot">
				/// The color to be used when the mouse is hovering over some items; e.g. the close button.
				/// </param>
				/// 
				/// <param name="clrDisabled">
				/// The color to be used for disabled controls.
				/// </param>
				/// 
				/// <param name="sTooltipFont">
				/// The name of the font to be used for displaying tooltips.
				/// </param>
				/// 
				/// <param name="iTooltipFontSize">
				/// The size of the font, in points, to be used for displaying tooltips.
				/// </param>
				/// 
				/// <param name="clrTooltipText">
				/// The color of the text in tooltip controls.
				/// </param>
				/// 
				/// <param name="clrTooltipBackground">
				/// The background color of tooltip controls.
				/// </param>
				/// 
				/// <param name="clrTooltipBorder">
				/// The border color of tooltip controls.
				/// </param>
				/// 
				/// <param name="hResModule">
				/// Handle to the module (DLL or exe) that contains the resources to be used (PNGs etc).
				/// </param>
				/// 
				/// <param name="pParent">
				/// A pointer to the window's parent (set to NULL if this is not a child window).
				/// </param>
				/// 
				/// <param name="pState">
				/// A pointer to the object used for managing the window/application state.
				///	This pointer is passed to the command procedure. (Set to NULL if you do not wish to use this
				/// parameter or if you wish to add it later using setState()).
				/// </param>
				/// 
				/// <returns> 
				/// There is no return value; this is a constructor.
				/// </returns>
				/// 
				/// <remarks>
				/// Make sure to dereference the pState pointer properly or the application will crash during runtime.
				/// </remarks>
				cui_raw(
					const std::basic_string<TCHAR> &sPageName,
					CommandProcedure pCommandProc,
					COLORREF clrBackground,
					COLORREF clrTheme,
					COLORREF clrThemeHot,
					COLORREF clrDisabled,
					const std::basic_string<TCHAR> &sTooltipFont,
					double iTooltipFontSize,
					COLORREF clrTooltipText,
					COLORREF clrTooltipBackground,
					COLORREF clrTooltipBorder,
					HMODULE hResModule,
					cui_raw* pParent,
					void *pState
				);

				/// <summary>
				/// Add font file to use for the user interface. Many fonts can be added using this
				/// function.
				/// </summary>
				/// 
				/// <param name="sFontFullPath">
				/// The full path to the font file, including the extension.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// Fonts need to be added before create() is called.
				/// </remarks>
				bool addFont(const std::basic_string<TCHAR> &sFontFullPath,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Get the list of font files that have been added.
				/// </summary>
				/// 
				/// <returns>
				/// Returns the font list.
				/// </returns>
				std::vector<std::basic_string<TCHAR>> get_font_files();

				/// <summary>
				/// Get version the of this library.
				/// </summary>
				/// 
				/// <returns>
				/// Returns a string containing the library's name and the version.
				/// </returns>
				static std::basic_string<TCHAR> GetVersion();

				~cui_raw();

				/// <summary>
				/// Position a rectangle within another.
				/// </summary>
				/// 
				/// <param name="rcInOut">
				/// The rectangle to position. Positioned rectangle will be written back when the function has 
				/// completed it's operation.
				/// </param>
				/// 
				/// <param name="rcTarget">
				/// The target rectangle in which another is to be positioned.
				/// </param>
				/// 
				/// <param name="iPercH">
				/// The percentage along the target rectangle's width at which to place our rectangle. 0% = left, 
				/// 100% = right.
				/// </param>
				/// 
				/// <param name="iPercV">
				/// The percentage along the target rectangle's height at which to place our rectangle. 0% = top, 
				/// 100% = bottom.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				/// 
				/// <remarks>
				/// If you need to remember the old size of the rectangle you have to create a copy BEFORE calling 
				/// this function.
				/// </remarks>
				static void posRect(RECT &rcInOut, const RECT &rcTarget, int iPercH, int iPercV);

				/// <summary>
				/// Set window's position and size.
				/// </summary>
				/// 
				/// <param name="ix">
				/// x - position on the client's screen, in pixels. 0 = left.
				/// </param>
				/// 
				/// <param name="iy">
				/// y - position on the client's screen, in pixels. 0 = top.
				/// </param>
				/// 
				/// <param name="icx">
				/// The width of the window, in pixels.
				/// </param>
				/// 
				/// <param name="icy">
				/// The height of the window, in pixels.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				/// 
				/// <remarks>
				/// This function only works before create() is called. To change position thereafter call setXY().
				/// </remarks>
				void setPosition(int ix, int iy, int icx, int icy);

				/// <summary>
				/// Set window's position using a predefined placement method.
				/// </summary>
				/// 
				/// <param name="wndPos">
				/// The placement method as defined in the windowPosition enumeration.
				/// </param>
				/// 
				/// <param name="icx">
				/// The width of the window, in pixels.
				/// </param>
				/// 
				/// <param name="icy">
				/// The height of the window, in pixels.
				/// </param>
				/// 
				/// <seealso cref="windowPosition">
				/// windowPosition is an enumeration for window placement.
				/// </seealso>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				/// 
				/// <remarks>
				/// This function only works before create() is called. To change position thereafter call setXY().
				/// </remarks>
				void setPosition(windowPosition wndPos, int icx, int icy);

				/// <summary>
				/// Set the window's minimum permitted width and height.
				/// </summary>
				/// 
				/// <param name="iMinW">
				/// The minimum window width, in pixels.
				/// </param>
				/// 
				/// <param name="iMinH">
				/// The minimum window height, in pixels.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void setMinWidthAndHeight(int iMinW, int iMinH);

				/// <summary>
				/// Enable a control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void enableControl(const std::basic_string<TCHAR> &sPageName, int iUniqueID);

				/// <summary>
				/// Disable a control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void disableControl(const std::basic_string<TCHAR> &sPageName, int iUniqueID);

				/// <summary>
				/// Check if a control is enabled.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if the control is enabled, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// False will also be returned if an error occurs or if the control doesn't exist.
				/// </remarks>
				bool controlEnabled(const std::basic_string<TCHAR> &sPageName, int iUniqueID);

				/// <summary>
				/// Hide a control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				void hideControl(const std::basic_string<TCHAR> &sPageName, int iUniqueID);

				/// <summary>
				/// Show a control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				void showControl(const std::basic_string<TCHAR> &sPageName, int iUniqueID);

				/// <summary>
				/// Check if a controls is visible.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if the control is visible, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// False will also be returned if an error occurs or if the control is hidden.
				/// </remarks>
				bool controlVisible(const std::basic_string<TCHAR> &sPageName, int iUniqueID);

				/// <summary>
				/// Set focus to a particular control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				void setFocus(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID);

				/// <summary>
				/// Enable this window.
				/// </summary>
				void enable();

				/// <summary>
				/// Disable this window.
				/// </summary>
				void disable();

				/// <summary>
				/// Check whether this window is enabled.
				/// </summary>
				/// 
				/// <returns>
				/// Returns true if window is enabled, else false.
				/// </returns>
				bool isEnabled();

				/// <summary>
				/// Check whether the window is visible.
				/// </summary>
				/// 
				/// <returns>
				/// Returns true if window is visible, else false.
				/// </returns>
				bool isVisible();

				/// <summary>
				/// Check whether there is a message box currently displayed.
				/// </summary>
				/// 
				/// <returns>
				/// Returns true if there is at least one message box displayed, else false.
				/// </returns>
				bool isMessageBoxDisplayed();

				/// <summary>
				/// Set a timer.
				/// </summary>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the timer.
				/// </param>
				/// 
				/// <param name="iMilliSeconds">
				/// The time after which iUniqueID will be sent to the command procedure of the home page.
				/// </param>
				void setTimer(int iUniqueID, unsigned int iMilliSeconds);

				/// <summary>
				/// Check whether a timer is running.
				/// </summary>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the timer.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if the timer is running, else false.
				/// </returns>
				bool timerRunning(int iUniqueID);

				/// <summary>
				/// Stop timer.
				/// </summary>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the timer.
				/// </param>
				void stopTimer(int iUniqueID);

				/// <summary>
				/// Add a close button to the window (added to the top right).
				/// </summary>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the close button control.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addCloseBtn(int iUniqueID);

				/// <summary>
				/// Add a close button to the window (added to the top right).
				/// </summary>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the close button control.
				/// </param>
				/// 
				/// <param name="bCustomHandle">
				/// Whether to handle the button action in the command procedure.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addCloseBtn(int iUniqueID, bool bCustomHandle);

				/// <summary>
				/// Exclude an area from the title bar area so it can receive input.
				/// </summary>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>

				/// <summary>
				/// Exclude a control from the title bar area so it can receive input.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The name of the page the control is in.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				void excludeFromTitleBar(const std::basic_string<TCHAR> &sPageName, int iUniqueID);

				/// <summary>
				/// Add a maximize button to the window (added to the top right).
				/// </summary>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the maximize button control.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addMaxBtn(int iUniqueID);

				/// <summary>
				/// Add a minimize button to the window (added to the top right).
				/// </summary>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the minimize button control.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addMinBtn(int iUniqueID);

				/// <summary>
				/// Add a button control to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sTooltip">
				/// The tooltip to be displayed when the mouse is hovered over the control.
				/// </param>
				/// 
				/// <param name="sCaption">
				/// The text to be displayed in the control.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="bDefaultButton">
				/// Flag to indicate this is the default button in the page. If set to yes, the button is called when 
				/// the user presses the Enter key while the keyboard focus is in an edit control.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addButton(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					const std::basic_string<TCHAR> &sTooltip,
					const std::basic_string<TCHAR> &sCaption,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					RECT rc, onResize resize, bool bDefaultButton);

				/// <summary>
				/// Add a text control to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sText">
				/// The text to be displayed in the control.
				/// </param>
				/// 
				/// <param name="bStatic">
				/// Whether this is a static control.
				/// </param>
				/// 
				/// <param name="clrText">
				/// The text color.
				/// </param>
				/// 
				/// <param name="clrTextHot">
				/// The text color when mouse hovers over it. Not used if bStatic is set to true.
				/// </param>
				/// 
				/// <param name="sTooltip">
				/// The tooltip to be displayed when the mouse is hovered over the control.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="align">
				/// The alignment of the text within the specified rectangular coordinates.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="bMultiLine">
				/// Whether to permit multiple lines if text cannot fit into one line.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addText(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID, const std::basic_string<TCHAR> &sText,
					bool bStatic,
					COLORREF clrText,
					COLORREF clrTextHot,
					const std::basic_string<TCHAR> &sTooltip,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					RECT rc,
					textAlignment align,
					onResize resize,
					bool bMultiLine = false
				);

				/// <summary>
				/// Get text in text control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sText">
				/// The text within the control.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool getText(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					std::basic_string<TCHAR> &sText,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Set text control text.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sText">
				/// The text to be displayed in the control.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool setText(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::basic_string<TCHAR> &sText,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Set text control text.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sText">
				/// The text to be displayed in the control.
				/// </param>
				/// 
				/// <param name="clrText">
				/// The new color of the text.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool setText(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::basic_string<TCHAR> &sText,
					COLORREF clrText,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Add a combobox control to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="vItems">
				/// A vector listing the strings to be added to the combobox control.
				/// </param>
				/// 
				/// <param name="sSelectedItem">
				/// The item in the list to be selected by default.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="bAutoComplete">
				/// Whether to autocomplete as the user types.
				/// </param>
				/// 
				/// <param name="bReadOnly">
				/// Whether the combobox is read-only.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addComboBox(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::vector<std::basic_string<TCHAR>> &vItems,
					const std::basic_string<TCHAR> &sSelectedItem,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					RECT rc, onResize resize, bool bAutoComplete, bool bReadOnly);

				/// <summary>
				/// Get combobox text.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sText">
				/// The currently selected text.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool getComboText(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					std::basic_string<TCHAR> &sText,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Select combobox item.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sSelectedItem">
				/// The item to select.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// If item is not found current selection will be cleared and an error will be returned.
				/// If sSelectedItem is an empty string the combobox selection will be cleared.
				/// </remarks>
				bool selectComboItem(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::basic_string<TCHAR> &sSelectedItem,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Set combobox text.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sText">
				/// The text to set to the combobox.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// Can be used to set custom text in non-read-only comboboxes (text
				/// that's not necessarily in the list). Works the same as cui_raw::selectComboItem
				/// for read-only comboboxes (see remarks under cui_raw::selectComboItem).
				/// </remarks>
				bool setComboText(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::basic_string<TCHAR> &sText,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Repopulate combobox.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="vItems">
				/// The list of items.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool repopulateCombo(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::vector<std::basic_string<TCHAR>> &vItems,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Add a listview control to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="vColumns">
				/// The columns to add to the listview control.
				/// </param>
				/// 
				/// <param name="sUniqueColumnName">
				/// The name of the column which is guaranteed to have unique items.
				/// </param>
				/// 
				/// <param name="vData">
				/// The data to add to the listview control.
				/// </param>
				/// 
				/// <param name="vContextMenu">
				/// The context menu to display when user right clicks an entry in the listview control.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="bBorder">
				/// Whether to draw a border around the control.
				/// </param>
				/// 
				/// <param name="bGridLines">
				/// Whether to display gridlines in the listview control.
				/// </param>
				/// 
				/// <param name="bSortByClickingColumn">
				/// Whether to enable sorting of items when column is clicked.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				/// 
				/// <remarks>
				/// Make sure sUniqueColumnName refers to a column whose entries are guaranteed to be unique,
				/// otherwise the results of sorting columns and calls to other listview functions like
				/// updateListViewItem() is undefined.
				/// </remarks>
				void addListview(
					const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					std::vector<listviewColumn> vColumns,
					const std::basic_string<TCHAR> &sUniqueColumnName,
					std::vector<listviewRow> vData,
					std::vector<contextMenuItem> vContextMenu,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					RECT rc,
					onResize resize,
					bool bBorder,
					bool bGridLines,
					bool bSortByClickingColumn
				);

				/// <summary>
				/// Add row to listview.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="vRow">
				/// The row to add to the listview.
				/// </param>
				/// 
				/// <param name="bScrollToBottom">
				/// Whether to scroll to the bottom after adding the row.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// The row number is automatically determined internally and written back to vRow.
				/// </remarks>
				bool addListviewRow(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					listviewRow &vRow,
					bool bScrollToBottom,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Repopulate a listview control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="vData">
				/// The data to add to the listview control.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// Row numbers are automatically calculated internally and written back to vData.
				/// </remarks>
				bool repopulateListview(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					std::vector<listviewRow> &vData,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Get data contained in a listview control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="vColumns">
				/// The column definitions of the listview control.
				/// </param>
				/// 
				/// <param name="vData">
				/// The data in the the listview control.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool getListview(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					std::vector<listviewColumn> &vColumns,
					std::vector<listviewRow> &vData,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Get currently selected rows in listview control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="vRows">
				/// The rows that are currently selected.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool getListviewSelected(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					std::vector<listviewRow> &vRows,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Update a listview item.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="item">
				/// The listview item.
				/// </param>
				/// 
				/// <param name="row">
				/// The row to which the listview item belongs.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// This function is only guaranteed to work properly for listviews with a correctly defined
				/// unique column name.
				/// </remarks>
				bool updateListViewItem(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					listviewItem item,
					const listviewRow &row,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Remove row from listview.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="iRowNumber">
				/// The row number (counted from 0).
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool removeListViewRow(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					int iRowNumber,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Add an edit control to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="sCueBanner">
				/// The placeholder text to display in the edit control when it is empty (useful for describing what 
				/// kind of information is expected).
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="bMultiLine">
				/// Whether to permit multiple lines if text cannot fit into one line.
				/// </param>
				/// 
				/// <param name="bScrollBar">
				/// Whether to add a vertical scroll bar for multiline text.
				/// </param>
				/// 
				/// <param name="bPassword">
				/// Whether this is a password control.
				/// </param>
				/// 
				/// <param name="bReadOnly">
				/// Whether control is readonly.
				/// </param>
				/// 
				/// <param name="iLimit">
				/// The limit to the number of characters this control can hold. 0 means unlimited.
				/// </param>
				/// 
				/// <param name="sAllowedCharacterSet">
				/// The character set to limit to. Empty character set means there is no limit.
				/// </param>
				/// 
				/// <param name="sForbiddenCharacterSet">
				/// The set of characters to prevent from being input. Empty character set means all
				/// characters are permitted.
				/// </param>
				/// 
				/// <param name="iControlToInvoke">
				/// The control to invoke when enter is pressed while in this control.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addEdit(
					const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					const std::basic_string<TCHAR> &sCueBanner, RECT rc, onResize resize,
					bool bMultiLine, bool bScrollBar, bool bPassword, bool bReadOnly,
					int iLimit = 0,
					const std::basic_string<TCHAR> &sAllowedCharacterSet = _T(""),
					const std::basic_string<TCHAR> &sForbiddenCharacterSet = _T(""),
					int iControlToInvoke = 0
				);

				/// <summary>
				/// Get edit control text.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sText">
				/// The text in the control.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool getEditText(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					std::basic_string<TCHAR> &sText,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Set edit control text.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sText">
				/// The text to place into the control.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool setEditText(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::basic_string<TCHAR> &sText,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Get the number of characters left before an edit control reaches it's limit.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="iCharsLeft">
				/// The number of characters left before the limit is reached. A value of -1 is
				/// returned if the edit control has no limit.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool getEditCharsLeft(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					int &iCharsLeft,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Add a toggle button control to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sTooltip">
				/// The tooltip to be displayed when the mouse is hovered over the control.
				/// </param>
				/// 
				/// <param name="sCaptionOn">
				/// The text to be displayed to the right of the control when the button is on.
				/// </param>
				/// 
				/// <param name="sCaptionOff">
				/// The text to be displayed to the right of the control when the button is off.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="clrText">
				/// The text color.
				/// </param>
				/// 
				/// <param name="clrOn">
				/// The on color.
				/// </param>
				/// 
				/// <param name="clrOff">
				/// The off color.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="bOn">
				/// Whether to turn the button on by default.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addToggleButton(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::basic_string<TCHAR> &sTooltip,
					const std::basic_string<TCHAR> &sCaptionOn,
					const std::basic_string<TCHAR> &sCaptionOff,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					COLORREF clrText,
					COLORREF clrOn,
					COLORREF clrOff,
					RECT rc, onResize resize, bool bOn);

				/// <summary>
				/// Set toggle button state.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="bOn">
				/// Whether to turn the button on.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool setToggleButton(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					bool bOn,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Get toggle button state.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="bOn">
				/// The state of the toggle button.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool getToggleButton(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					bool &bOn,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Add a selector control to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sTooltip">
				/// The tooltip to be displayed when the mouse is hovered over the control.
				/// </param>
				/// 
				/// <param name="vItems">
				/// The items in the selector.
				/// </param>
				/// 
				/// <param name="iDefaultItem">
				/// The item to be selected by default in the selector.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="clrText">
				/// The text color.
				/// </param>
				/// 
				/// <param name="clrBar">
				/// The bar color.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addSelector(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::basic_string<TCHAR> &sTooltip,
					std::vector<selectorItem> vItems,
					int iDefaultItem,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					COLORREF clrText, COLORREF clrBar,
					RECT rc, onResize resize);

				/// <summary>
				/// Set selector control state.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="iSelectorItemID">
				/// The unique ID of the selector item to select.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool setSelector(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					int iSelectorItemID,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Get selector control state.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="iSelectorItemID">
				/// The unique ID of the currently selected.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool getSelector(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					int &iSelectorItemID,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Add a progress bar control to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="clrBar">
				/// The color of the progress indicator.
				/// </param>
				/// 
				/// <param name="clrUnfilled">
				/// The color of the unfilled areas.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="iInitialPercentage">
				/// The initial percentage of the progress indicator. Set to -1 to set the progress bar to "busy".
				/// Set to -2 to reverse the progress bar (countdown bar).
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				/// 
				/// <remarks>
				/// The color of the border is a slightly darker version of clrUnfilled. If, however, clrUnfilled is 
				/// the same as the background color of the entire window, the border will be removed.
				/// </remarks>
				void addProgressBar(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					COLORREF clrBar,
					COLORREF clrUnfilled,
					RECT rc, onResize resize, double iInitialPercentage);

				/// <summary>
				/// Set progress bar position.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="iPercentage">
				/// The percentage of the progress indicator. Set to -1 to set the progress bar to "busy".
				/// </param>
				/// 
				/// <param name="bChangeColor">
				/// Whether to change the color of the progress bar.
				/// </param>
				/// 
				/// <param name="clrBar">
				/// The color of the bar.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool setProgressBar(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID, double iPercentage, bool bChangeColor, COLORREF clrBar,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Add a password strength bar control to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="clrUnfilled">
				/// The color of the unfilled areas.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="iInitialPercentage">
				/// The initial percentage of the progress indicator.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addPasswordStrengthBar(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID, COLORREF clrUnfilled,
					RECT rc, onResize resize, double iInitialPercentage);

				/// <summary>
				/// Set password strength meter bar position.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="iPercentage">
				/// The percentage of the password strength.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool setPasswordStrengthBar(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID, double iPercentage, std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Add an up-down control to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="iMin">
				/// The minimum value.
				/// </param>
				/// 
				/// <param name="iMax">
				/// The maximum value.
				/// </param>
				/// 
				/// <param name="iPos">
				/// The initial value to be displayed.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addUpDown(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					RECT rc, onResize resize, int iMin, int iMax, int iPos);

				/// <summary>
				/// Set up-down control position.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="iPos">
				/// The position of the control.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool setUpDown(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					int iPos,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Get up-down control position.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="iPos">
				/// The position of the control.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool getUpDown(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					int &iPos,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Month enumeration.
				/// </summary>
				/// 
				/// <remarks>
				/// See: https://msdn.microsoft.com/en-us/library/windows/desktop/ms724950(v=vs.85).aspx
				/// </remarks>
				enum month
				{
					january = 1,
					february,
					march,
					april,
					may,
					june,
					july,
					august,
					september,
					october,
					november,
					december,
				};

				/// <summary>
				/// Date structure.
				/// </summary>
				/// 
				/// <remarks>
				/// Invalid date marked by a year of -1.
				/// </remarks>
				struct date
				{
					/// <summary>
					/// Day of the month.
					/// </summary>
					int iDay = 1;

					/// <summary>
					/// Month of the year.
					/// </summary>
					month enMonth;

					/// <summary>
					/// Year.
					/// </summary>
					int iYear = 2015;
				};

				/// <summary>
				/// Add a date control to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="bAllowNone">
				/// Allow a NONE selection.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addDate(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					RECT rc, onResize resize, bool bAllowNone);

				/// <summary>
				/// Set date in a date control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="dDate">
				/// The date to select. If the year is set to -1, the date control will be set to NONE.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool setDate(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					date dDate,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Get the date in a date control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="dDate">
				/// The date currently selected in the control. If the date control is set to NONE, the iYear member is 
				/// set to -1.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool getDate(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					date &dDate,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Time structure.
				/// </summary>
				struct time
				{
					/// <summary>
					/// The hour (0 - 23).
					/// </summary>
					int iHour = 0;

					/// <summary>
					/// The minute (0 - 59)
					/// </summary>
					int iMinute = 0;

					/// <summary>
					/// The second (0 - 59)
					/// </summary>
					int iSecond = 0;
				};

				/// <summary>
				/// Add a time control to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="bAllowNone">
				/// Allow a NONE selection.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addTime(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					RECT rc, onResize resize, bool bAllowNone);

				/// <summary>
				/// Set time in a time control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="tTime">
				/// The time to select.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool setTime(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					time tTime,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Get the time in a time control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="tTime">
				/// The time currently selected in the control.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool getTime(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					time &tTime,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Bar chart data structure.
				/// </summary>
				struct barChartData
				{
					/// <summary>
					/// Item number.
					/// </summary>
					int iNumber;

					/// <summary>
					/// The item's label.
					/// </summary>
					std::basic_string<TCHAR> sLabel;

					/// <param name="clrBar">
					/// The color of the bar in the chart.
					/// </param>
					COLORREF clrBar;

					/// <summary>
					/// The item's value.
					/// </summary>
					double dValue = 0;
				};

				/// <summary>
				/// Add a bar chart to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="clrBar">
				/// The color of the bars in the chart.
				/// </param>
				/// 
				/// <param name="sChartName">
				/// The name of the chart.
				/// </param>
				/// 
				/// <param name="sXaxisLabel">
				/// The x-axis label.
				/// </param>
				/// 
				/// <param name="sYaxisLabel">
				/// The y-axis label.
				/// </param>
				/// 
				/// <param name="iLowerLimit">
				/// The lower limit of the range.
				/// </param>
				/// 
				/// <param name="iUpperLimit">
				/// The upper limit of the range.
				/// </param>
				/// 
				/// <param name="bAutoScale">
				/// Whether to automatically adjust the y-axis scale.
				/// </param>
				/// 
				/// <param name="vValues">
				/// Values to plot.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addBarChart(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					RECT rc, onResize resize,
					COLORREF clrBar,
					std::basic_string<TCHAR> sChartName,
					std::basic_string<TCHAR> sXaxisLabel, std::basic_string<TCHAR> sYaxisLabel,
					int iLowerLimit, int iUpperLimit, bool bAutoScale,
					std::vector<barChartData> vValues,
					bool autocolor = false
				);

				/// <summary>
				/// Set bar chart scale.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="bAutoScale">
				/// Whether to automatically adjust the y-axis scale.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool barChartScaleSet(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					bool bAutoScale,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Set bar chart scale.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="bAutoScale">
				/// Whether y-axis scale is set to adjust automatically.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool barChartScaleGet(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					bool &bAutoScale,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Image format.
				/// </summary>
				enum imgFormat
				{
					/// <summary>
					/// PNG Image (Portable Network Graphics).
					/// </summary>
					PNG = 0,

					/// <summary>
					/// BITMAP Image.
					/// </summary>
					BMP,

					/// <summary>
					/// JPEG Image.
					/// </summary>
					JPEG,

					/// <summary>
					/// Unspecified image format.
					/// </summary>
					NONE,
				};

				/// <summary>
				/// Set bar chart name.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sBarChartName">
				/// The name of the bar chart.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool barChartNameGet(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					std::basic_string<TCHAR> &sBarChartName,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Save bar chart to an image file.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="format">
				/// Image format.
				/// </param>
				/// 
				/// <param name="sFullPath">
				/// Full path of image file (with or without a file extension). NOTE: The actual full path that the 
				/// image is saved to will be written back.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// When imgFormat::NONE is used the file will be saved without an extension but the internal format 
				/// used will be PNG.
				/// </remarks>
				bool barChartSave(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					imgFormat format,
					std::basic_string<TCHAR> &sFullPath,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Add a bar chart to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sChartName">
				/// The name of the chart.
				/// </param>
				/// 
				/// <param name="sXaxisLabel">
				/// The x-axis label.
				/// </param>
				/// 
				/// <param name="sYaxisLabel">
				/// The y-axis label.
				/// </param>
				/// 
				/// <param name="iLowerLimit">
				/// The lower limit of the range.
				/// </param>
				/// 
				/// <param name="iUpperLimit">
				/// The upper limit of the range.
				/// </param>
				/// 
				/// <param name="bAutoScale">
				/// Whether to automatically adjust the y-axis scale.
				/// </param>
				/// 
				/// <param name="vValues">
				/// Values to plot.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				bool barChartReload(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					std::basic_string<TCHAR> sChartName,
					std::basic_string<TCHAR> sXaxisLabel, std::basic_string<TCHAR> sYaxisLabel,
					int iLowerLimit, int iUpperLimit, bool bAutoScale,
					std::vector<barChartData> vValues,
					std::basic_string<TCHAR> &sErr
				);

				struct lineInfo
				{
					/// <summary>
					/// The name of the series represented by the line.
					/// </summary>
					std::basic_string<TCHAR> sSeriesName;

					/// <param name="clrLine">
					/// The color of the line in the chart.
					/// </param>
					COLORREF clrLine;

					/// <summary>
					/// The values to plot for this line.
					/// </summary>
					std::vector<barChartData> vValues;
				};

				/// <summary>
				/// Add a line chart to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="sChartName">
				/// The name of the chart.
				/// </param>
				/// 
				/// <param name="sXaxisLabel">
				/// The x-axis label.
				/// </param>
				/// 
				/// <param name="sYaxisLabel">
				/// The y-axis label.
				/// </param>
				/// 
				/// <param name="iLowerLimit">
				/// The lower limit of the range.
				/// </param>
				/// 
				/// <param name="iUpperLimit">
				/// The upper limit of the range.
				/// </param>
				/// 
				/// <param name="bAutoScale">
				/// Whether to automatically adjust the y-axis scale.
				/// </param>
				/// 
				/// <param name="vLines">
				/// Lines to plot.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addLineChart(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					RECT rc, onResize resize,
					std::basic_string<TCHAR> sChartName,
					std::basic_string<TCHAR> sXaxisLabel, std::basic_string<TCHAR> sYaxisLabel,
					int iLowerLimit, int iUpperLimit, bool bAutoScale,
					std::vector<lineInfo> vLines,
					bool autocolor = false
				);

				/// <summary>
				/// Set line chart scale.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="bAutoScale">
				/// Whether to automatically adjust the y-axis scale.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool lineChartScaleSet(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					bool bAutoScale,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Set line chart scale.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="bAutoScale">
				/// Whether y-axis scale is set to adjust automatically.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool lineChartScaleGet(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					bool &bAutoScale,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Set line chart name.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sLineChartName">
				/// The name of the line chart.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool lineChartNameGet(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					std::basic_string<TCHAR> &sLineChartName,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Save line chart to an image file.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="format">
				/// Image format.
				/// </param>
				/// 
				/// <param name="sFullPath">
				/// Full path of image file (with or without a file extension). NOTE: The actual full path that the 
				/// image is saved to will be written back.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// When imgFormat::NONE is used the file will be saved without an extension but the internal format 
				/// used will be PNG.
				/// </remarks>
				bool lineChartSave(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					imgFormat format,
					std::basic_string<TCHAR> &sFullPath,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Add a line chart to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sChartName">
				/// The name of the chart.
				/// </param>
				/// 
				/// <param name="sXaxisLabel">
				/// The x-axis label.
				/// </param>
				/// 
				/// <param name="sYaxisLabel">
				/// The y-axis label.
				/// </param>
				/// 
				/// <param name="iLowerLimit">
				/// The lower limit of the range.
				/// </param>
				/// 
				/// <param name="iUpperLimit">
				/// The upper limit of the range.
				/// </param>
				/// 
				/// <param name="bAutoScale">
				/// Whether to automatically adjust the y-axis scale.
				/// </param>
				/// 
				/// <param name="vLines">
				/// The lines to plot.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				bool lineChartReload(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					std::basic_string<TCHAR> sChartName,
					std::basic_string<TCHAR> sXaxisLabel, std::basic_string<TCHAR> sYaxisLabel,
					int iLowerLimit, int iUpperLimit, bool bAutoScale,
					std::vector<lineInfo> vLines,
					std::basic_string<TCHAR> &sErr
				);

				struct pieChartData
				{
					/// <summary>
					/// The item number.
					/// </summary>
					int iNumber = 0;

					/// <summary>
					/// The value of the item, e.g. 41.3 (this is NOT a percentage, the percentage will be deduced)
					/// </summary>
					double dValue = 0;

					/// <summary>
					/// The label of the item, e.g. Male.
					/// </summary>
					std::basic_string<TCHAR> sItemLabel;

					/// <summary>
					/// The color to assign to the bar chart item. Only used if the pie chart itself is not autocolor.
					/// </summary>
					COLORREF clrItem;
				};

				/// <summary>
				/// Pie chart graphic effects when mouse if hovered over it.
				/// </summary>
				enum pieChartHoverEffect
				{
					/// <summary>
					/// Glow the slice.
					/// </summary>
					glow = 0,

					/// <summary>
					/// Glow the slice and add an arc to it for highlight.
					/// </summary>
					glowAndArc,

					/// <summary>
					/// Glow the slice and grow it for highlight.
					/// </summary>
					glowAndGrow,

					/// <summary>
					/// Glow the slice and shrink the other slices.
					/// </summary>
					glowAndShrinkOthers,
				};

				/// <summary>
				/// Add a pie chart control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The name of the font to use in the control.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The font size, in points.
				/// </param>
				/// 
				/// <param name="rc">
				/// The RECT to place the control in.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="bAutoColor">
				/// Whether to auto-color the chart.
				/// </param>
				/// 
				/// <param name="sChartName">
				/// The name of the chart.
				/// </param>
				/// 
				/// <param name="vData">
				/// The data to display in the chart, as defined in the cui_raw::pieChartData struct.
				/// </param>
				/// 
				/// <param name="hoverEffect">
				/// The visual effect when the mouse is hovered over the pie chart, as defined
				/// in cui_raw::pieChartHoverEffect.
				/// </param>
				/// 
				/// <param name="bDoughnut">
				/// Whether to make this a doughnut pie chart (with an open space in the middle)
				/// </param>
				/// 
				/// <remarks>
				/// Ensure that the width of the pie chart is greater than it's height. The diameter of the pie is
				/// derived from the height.
				/// </remarks>
				void addPieChart(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					RECT rc, onResize resize,
					bool bAutoColor,
					std::basic_string<TCHAR> sChartName,
					std::vector<pieChartData> vData,
					pieChartHoverEffect hoverEffect,
					bool bDoughnut
				);

				/// <summary>
				/// Save pie chart to an image file.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="format">
				/// Image format.
				/// </param>
				/// 
				/// <param name="sFullPath">
				/// Full path of image file (with or without a file extension). NOTE: The actual full path that the 
				/// image is saved to will be written back.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// When imgFormat::NONE is used the file will be saved without an extension but the internal format 
				/// used will be PNG.
				/// </remarks>
				bool pieChartSave(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					imgFormat format,
					std::basic_string<TCHAR> &sFullPath,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Reload a pie chart.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page in which the control is.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="bAutoColor">
				/// Whether to auto-color the chart.
				/// </param>
				/// 
				/// <param name="sChartName">
				/// The name of the chart.
				/// </param>
				/// 
				/// <param name="vData">
				/// The data to display in the chart, as defined in the cui_raw::pieChartData struct.
				/// </param>
				/// 
				/// <param name="hoverEffect">
				/// The visual effect when the mouse is hovered over the pie chart, as defined
				/// in cui_raw::pieChartHoverEffect.
				/// </param>
				/// 
				/// <param name="bDoughnut">
				/// Whether to make this a doughnut pie chart (with an open space in the middle)
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool pieChartReload(const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					bool bAutoColor,
					std::basic_string<TCHAR> sChartName,
					std::vector<pieChartData> vData,
					pieChartHoverEffect hoverEffect,
					bool bDoughnut,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Add a rich edit control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sUIFontName">
				/// The name of the UI font to use in the control.
				/// </param>
				/// 
				/// <param name="iUIFontSize">
				/// The size of the UI font, in points.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The name of the font to use in the control.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The font size, in points.
				/// </param>
				/// 
				/// <param name="rc">
				/// The RECT to place the control in.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="bBorder">
				/// Whether to place a border around the control.
				/// </param>
				/// 
				/// <param name="clrBorder">
				/// The color to use for the control's border. Only used if bBorder is true.
				/// </param>
				/// 
				/// <param name="bReadOnly">
				/// Whether to make the control read-only.
				/// </param>
				/// 
				/// <param name="IDP_BOLD_">
				/// The PNG image for the bold button.
				/// </param>
				/// 
				/// <param name="IDP_ITALIC_">
				/// The PNG image for the italic button.
				/// </param>
				/// 
				/// <param name="IDP_UNDERLINE_">
				/// The PNG image for the underline icon.
				/// </param>
				/// 
				/// <param name="IDP_STRIKETHROUGH_">
				/// The PNG image for the strikeout icon.
				/// </param>
				/// 
				/// <param name="IDP_SUBSCRIPT_">
				/// The PNG image for the subscript icon.
				/// </param>
				/// 
				/// <param name="IDP_SUPERSCRIPT_">
				/// The PNG image for the superscript icon.
				/// </param>
				/// 
				/// <param name="IDP_LARGER_">
				/// The PNG image for the enlarge text icon.
				/// </param>
				/// 
				/// <param name="IDP_SMALLER_">
				/// The PNG image for the shrink text icon.
				/// </param>
				/// 
				/// <param name="IDP_FONTCOLOR_">
				/// The PNG image for the font color icon.
				/// </param>
				/// 
				/// <param name="IDP_LEFT_ALIGN_">
				/// The PNG image for the left align icon.
				/// </param>
				/// 
				/// <param name="IDP_CENTER_ALIGN_">
				/// The PNG image for the center align icon.
				/// </param>
				/// 
				/// <param name="IDP_RIGHT_ALIGN_">
				/// The PNG image for the right align icon.
				/// </param>
				/// 
				/// <param name="IDP_JUSTIFY_">
				/// The PNG image for the justify icon.
				/// </param>
				/// 
				/// <param name="IDP_LIST_">
				/// The PNG image for the list icon.
				/// </param>
				/// 
				/// <param name="resource_module">
				/// Handle to the module containing the PNGs. Set to NULL to use the module
				/// specified in the constructor.
				/// </param>
				/// 
				/// <remarks>
				/// All the PNG resources are ignored if bReadOnly is set to true.
				/// </remarks>
				void addRichEdit(
					const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					const std::basic_string<TCHAR> &sUIFontName, double iUIFontSize,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					RECT rc, onResize resize,
					bool bBorder,
					COLORREF clrBorder,
					bool bReadOnly,
					int IDP_BOLD_,
					int IDP_ITALIC_,
					int IDP_UNDERLINE_,
					int IDP_STRIKETHROUGH_,
					int IDP_SUBSCRIPT_,
					int IDP_SUPERSCRIPT_,
					int IDP_LARGER_,
					int IDP_SMALLER_,
					int IDP_FONTCOLOR_,
					int IDP_LEFT_ALIGN_,
					int IDP_CENTER_ALIGN_,
					int IDP_RIGHT_ALIGN_,
					int IDP_JUSTIFY_,
					int IDP_LIST_,
					HMODULE resource_module = NULL
				);

				/// <summary>
				/// Add a read-only rich edit control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sUIFontName">
				/// The name of the UI font to use in the control.
				/// </param>
				/// 
				/// <param name="iUIFontSize">
				/// The size of the UI font, in points.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The name of the font to use in the control.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The font size, in points.
				/// </param>
				/// 
				/// <param name="rc">
				/// The RECT to place the control in.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="bBorder">
				/// Whether to place a border around the control.
				/// </param>
				/// 
				/// <param name="clrBorder">
				/// The color to use for the control's border. Only used if bBorder is true.
				/// </param>
				/// 
				/// <remarks>
				/// Internally calls it's overload with bReadOnly set to true.
				/// </remarks>
				void addRichEdit(
					const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					const std::basic_string<TCHAR> &sUIFontName, double iUIFontSize,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					RECT rc, onResize resize,
					bool bBorder,
					COLORREF clrBorder
				);

				/// <summary>
				/// Load rich edit file into a rich edit control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sFullPath">
				/// The full path to the file, including the extension.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool richEditLoad(
					const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					const std::basic_string<TCHAR> &sFullPath,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Load RTF text into a rich edit control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sRTF">
				/// The RTF formatted text.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool richEditRTFLoad(
					const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					const std::string &sRTF,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Save the contents of a rich edit into a rich edit file (.rtf).
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page the control is in.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sFullPath">
				/// The full path to the file, including the extension.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// If the extension is messy it will be fixed internally.
				/// </remarks>
				bool richEditSave(
					const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					std::basic_string<TCHAR> &sFullPath,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Write the contents of a rich edit into a string.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page the control is in.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sRTF">
				/// The string to write the RTF data into.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool richEditRTFGet(
					const std::basic_string<TCHAR> &sPageName, int iUniqueID,
					std::string &sRTF,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Position of text in image control.
				/// </summary>
				enum imageTextPlacement
				{
					/// <summary>
					/// Place text below the image.
					/// </summary>
					bottom = 1,

					/// <summary>
					/// Place text above the image.
					/// </summary>
					top,

					/// <summary>
					/// Place text to the right of the image.
					/// </summary>
					right,

					/// <summary>
					/// Place text to the top right of the image.
					/// </summary>
					righttop,

					/// <summary>
					/// Place text to the bottom right of the image.
					/// </summary>
					rightbottom,

					/// <summary>
					/// Place text to the left of the image.
					/// </summary>
					left,

					/// <summary>
					/// Place text to the top left of the image.
					/// </summary>
					lefttop,

					/// <summary>
					/// Place text to the bottom left of the image.
					/// </summary>
					leftbottom,
				};

				/// <summary>
				/// Add an image control (based on a PNG resource) to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sTooltip">
				/// The tooltip to be displayed when the mouse moves over the control.
				/// </param>
				/// 
				/// <param name="bTightFit">
				/// Whether to display only the image (no border, no toggling).
				/// </param>
				/// 
				/// <param name="bChangeColor">
				/// Whether to change the color of the image (works with outline images and clipart).
				/// </param>
				/// 
				/// <param name="clrImage">
				/// The color to use for the image (if bChangeColor is true). Everything except the transparent pixels 
				/// is colored.
				/// </param>
				/// 
				/// <param name="clrImageHot">
				/// The color to use for the image when the mouse moves over the control (if bChangeColor is true). 
				/// Everything except the transparent pixels is colored.
				/// </param>
				/// 
				/// <param name="clrBorder">
				/// The color to use for the border.
				/// </param>
				/// 
				/// <param name="clrBorderHot">
				/// The color to use for the border when the mouse moves over the control.
				/// </param>
				/// 
				/// <param name="bButtonBar">
				/// Whether to place a bar in the control. The position of the bar depends on the textPlacement 
				/// parameter.
				/// </param>
				/// 
				/// <param name="clrBar">
				/// The color of the bar.
				/// </param>
				/// 
				/// <param name="clrBackground">
				/// The color of the background (if bBackground is true).
				/// </param>
				/// 
				/// <param name="clrBackgroundHot">
				/// The color of the background when mouse is hovered over the control (if bBackground is true).
				/// </param>
				/// 
				/// <param name="IDC_PNG">
				/// The ID of the PNG resource.
				/// </param>
				/// 
				/// <param name="sText">
				/// The image's caption text.
				/// </param>
				/// 
				/// <param name="clrText">
				/// The text color.
				/// </param>
				/// 
				/// <param name="clrTextHot">
				/// The text color when the mouse moves over the control.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="textPlacement">
				/// Position of text in image control.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="bToggle">
				/// Whether to toggle the control when the mouse moves over it.
				/// </param>
				/// 
				/// <param name="toggleAction">
				/// Toggle action when mouse moves over the control.
				/// </param>
				/// 
				/// <param name="bDescriptive">
				/// Whether to include a descriptive caption. When set to true the image is set to a size of imageSize 
				/// and placed on the top left.
				/// </param>
				/// 
				/// <param name="sDescription">
				/// Text to use for descriptive caption if bDescriptive is set to true.
				/// </param>
				/// 
				/// <param name="imageSize">
				/// The size of the image if bDescriptive is set to true.
				/// </param>
				/// 
				/// <param name="resource_module">
				/// Handle to the module containing IDC_PNG. Set to NULL to use the module
				/// specified in the constructor.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addImage(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::basic_string<TCHAR> &sTooltip,
					bool bTightFit,
					bool bChangeColor,
					COLORREF clrImage,
					COLORREF clrImageHot,
					COLORREF clrBorder,
					COLORREF clrBorderHot,
					bool bButtonBar,
					COLORREF clrBar,
					COLORREF clrBackground,
					COLORREF clrBackgroundHot,
					int IDC_PNG,
					const std::basic_string<TCHAR> &sText,
					COLORREF clrText,
					COLORREF clrTextHot,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					imageTextPlacement textPlacement,
					RECT rc, onResize resize, bool bToggle, onToggle toggleAction, bool bDescriptive,
					const std::basic_string<TCHAR> &sDescription, SIZE imageSize,
					HMODULE resource_module = NULL
				);

				/// <summary>
				/// Add an image control (based on a PNG resource) to the window, and with custom descriptive text font.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sTooltip">
				/// The tooltip to be displayed when the mouse moves over the control.
				/// </param>
				/// 
				/// <param name="bTightFit">
				/// Whether to display only the image (no border, no toggling).
				/// </param>
				/// 
				/// <param name="bChangeColor">
				/// Whether to change the color of the image (works with outline images and clipart).
				/// </param>
				/// 
				/// <param name="clrImage">
				/// The color to use for the image (if bChangeColor is true). Everything except the transparent pixels 
				/// is colored.
				/// </param>
				/// 
				/// <param name="clrImageHot">
				/// The color to use for the image when the mouse moves over the control (if bChangeColor is true). 
				/// Everything except the transparent pixels is colored.
				/// </param>
				/// 
				/// <param name="clrBorder">
				/// The color to use for the border.
				/// </param>
				/// 
				/// <param name="clrBorderHot">
				/// The color to use for the border when the mouse moves over the control.
				/// </param>
				/// 
				/// <param name="bButtonBar">
				/// Whether to place a bar in the control. The position of the bar depends on the textPlacement 
				/// parameter.
				/// </param>
				/// 
				/// <param name="clrBar">
				/// The color of the bar.
				/// </param>
				/// 
				/// <param name="clrBackground">
				/// The color of the background (if bBackground is true).
				/// </param>
				/// 
				/// <param name="clrBackgroundHot">
				/// The color of the background when mouse is hovered over the control (if bBackground is true).
				/// </param>
				/// 
				/// <param name="IDC_PNG">
				/// The ID of the PNG resource.
				/// </param>
				/// 
				/// <param name="sText">
				/// The image's caption text.
				/// </param>
				/// 
				/// <param name="clrText">
				/// The text color.
				/// </param>
				/// 
				/// <param name="clrTextHot">
				/// The text color when the mouse moves over the control.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="sFontNameDescription">
				/// The font to be used for the description text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="textPlacement">
				/// Position of text in image control.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="bToggle">
				/// Whether to toggle the control when the mouse moves over it.
				/// </param>
				/// 
				/// <param name="toggleAction">
				/// Toggle action when mouse moves over the control.
				/// </param>
				/// 
				/// <param name="bDescriptive">
				/// Whether to include a descriptive caption. When set to true the image is set to a size of imageSize 
				/// and placed on the top left.
				/// </param>
				/// 
				/// <param name="sDescription">
				/// Text to use for descriptive caption if bDescriptive is set to true.
				/// </param>
				/// 
				/// <param name="imageSize">
				/// The size of the image if bDescriptive is set to true.
				/// </param>
				/// 
				/// <param name="resource_module">
				/// Handle to the module containing IDC_PNG. Set to NULL to use the module
				/// specified in the constructor.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addImage(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::basic_string<TCHAR> &sTooltip,
					bool bTightFit,
					bool bChangeColor,
					COLORREF clrImage,
					COLORREF clrImageHot,
					COLORREF clrBorder,
					COLORREF clrBorderHot,
					bool bButtonBar,
					COLORREF clrBar,
					COLORREF clrBackground,
					COLORREF clrBackgroundHot,
					int IDC_PNG,
					const std::basic_string<TCHAR> &sText,
					COLORREF clrText,
					COLORREF clrTextHot,
					const std::basic_string<TCHAR> &sFontName, const std::basic_string<TCHAR> &sFontNameDescription, double iFontSize,
					imageTextPlacement textPlacement,
					RECT rc, onResize resize, bool bToggle, onToggle toggleAction, bool bDescriptive,
					const std::basic_string<TCHAR> &sDescription, SIZE imageSize,
					HMODULE resource_module = NULL
				);

				/// <summary>
				/// Add an image control (based on a file) to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sTooltip">
				/// The tooltip to be displayed when the mouse moves over the control.
				/// </param>
				/// 
				/// <param name="bTightFit">
				/// Whether to display only the image (no border, no toggling).
				/// </param>
				/// 
				/// <param name="bChangeColor">
				/// Whether to change the color of the image (works with outline images and clipart).
				/// </param>
				/// 
				/// <param name="clrImage">
				/// The color to use for the image (if bChangeColor is true). Everything except the transparent pixels 
				/// is colored.
				/// </param>
				/// 
				/// <param name="clrImageHot">
				/// The color to use for the image when the mouse moves over the control (if bChangeColor is true). 
				/// Everything except the transparent pixels is colored.
				/// </param>
				/// 
				/// <param name="clrBorder">
				/// The color to use for the border.
				/// </param>
				/// 
				/// <param name="clrBorderHot">
				/// The color to use for the border when the mouse moves over the control.
				/// </param>
				/// 
				/// <param name="bButtonBar">
				/// Whether to place a bar in the control. The position of the bar depends on the textPlacement 
				/// parameter.
				/// </param>
				/// 
				/// <param name="clrBar">
				/// The color of the bar.
				/// </param>
				/// 
				/// <param name="clrBackground">
				/// The color of the background (if bBackground is true).
				/// </param>
				/// 
				/// <param name="clrBackgroundHot">
				/// The color of the background when mouse is hovered over the control (if bBackground is true).
				/// </param>
				/// 
				/// <param name="sFileName">
				/// The full path to an image.
				/// </param>
				/// 
				/// <param name="sText">
				/// The image's caption text.
				/// </param>
				/// 
				/// <param name="clrText">
				/// The text color.
				/// </param>
				/// 
				/// <param name="clrTextHot">
				/// The text color when the mouse moves over the control.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="textPlacement">
				/// Position of text in image control.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="bToggle">
				/// Whether to toggle the control when the mouse moves over it.
				/// </param>
				/// 
				/// <param name="toggleAction">
				/// Toggle action when mouse moves over the control.
				/// </param>
				/// 
				/// <param name="bDescriptive">
				/// Whether to include a descriptive caption. When set to true the image is set to a size of imageSize 
				/// and placed on the top left.
				/// </param>
				/// 
				/// <param name="sDescription">
				/// Text to use for descriptive caption if bDescriptive is set to true.
				/// </param>
				/// 
				/// <param name="imageSize">
				/// The size of the image if bDescriptive is set to true.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addImage(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::basic_string<TCHAR> &sTooltip,
					bool bTightFit,
					bool bChangeColor,
					COLORREF clrImage,
					COLORREF clrImageHot,
					COLORREF clrBorder,
					COLORREF clrBorderHot,
					bool bButtonBar,
					COLORREF clrBar,
					COLORREF clrBackground,
					COLORREF clrBackgroundHot,
					const std::basic_string<TCHAR> &sFileName,
					const std::basic_string<TCHAR> &sText,
					COLORREF clrText,
					COLORREF clrTextHot,
					const std::basic_string<TCHAR> &sFontName, double iFontSize,
					imageTextPlacement textPlacement,
					RECT rc, onResize resize, bool bToggle, onToggle toggleAction, bool bDescriptive,
					const std::basic_string<TCHAR> &sDescription, SIZE imageSize
				);

				/// <summary>
				/// Change image in image control (using a PNG resource).
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="IDC_PNG">
				/// The ID of the PNG resource. Set to -1 to remove an existing image.
				/// </param>
				/// 
				/// <param name="bChangeText">
				/// Whether to change the image's text.
				/// </param>
				/// 
				/// <param name="sNewText">
				/// The image's caption text.
				/// </param>
				/// 
				/// <param name="bUpdate">
				/// Whether to update the image control immediately.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// If you intend to change various attributes of the same image consecutively set bUpdate to false and 
				/// use a "sweeping" call to updateImage() when done with all the changes.
				/// </remarks>
				bool changeImage(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					int IDC_PNG,
					bool bChangeText,
					const std::basic_string<TCHAR> &sNewText,
					bool bUpdate,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Change image in image control (using an image file).
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sNewFileName">
				/// The full path to an image.
				/// </param>
				/// 
				/// <param name="bChangeText">
				/// Whether to change the image's text.
				/// </param>
				/// 
				/// <param name="sNewText">
				/// The image's caption text.
				/// </param>
				/// 
				/// <param name="bUpdate">
				/// Whether to update the image control immediately.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// If you intend to change various attributes of the same image consecutively set bUpdate to false and 
				/// use a "sweeping" call to updateImage() when done with all the changes.
				/// </remarks>
				bool changeImage(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::basic_string<TCHAR> &sNewFileName,
					bool bChangeText,
					const std::basic_string<TCHAR> &sNewText,
					bool bUpdate,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Change caption of image control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sNewText">
				/// The image's caption text.
				/// </param>
				/// 
				/// <param name="sNewDescription">
				/// The image's descriptive caption text.
				/// </param>
				/// 
				/// <param name="bUpdate">
				/// Whether to update the image control immediately.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// If you intend to change various attributes of the same image consecutively set bUpdate to false and 
				/// use a "sweeping" call to updateImage() when done with all the changes.
				/// </remarks>
				bool changeImageText(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::basic_string<TCHAR> &sNewText,
					const std::basic_string<TCHAR> &sNewDescription,
					bool bUpdate,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Set/change an image control's bar.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="clrBar">
				/// The color of the bar.
				/// </param>
				/// 
				/// <param name="bUpdate">
				/// Whether to update the image control immediately.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// If you intend to change various attributes of the same image consecutively set bUpdate to false and 
				/// use a "sweeping" call to updateImage() when done with all the changes.
				/// </remarks>
				bool setImageBar(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					COLORREF clrBar,
					bool bUpdate,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Remove an image control's bar.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="bUpdate">
				/// Whether to update the image control immediately.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// If you intend to change various attributes of the same image consecutively set bUpdate to false and 
				/// use a "sweeping" call to updateImage() when done with all the changes.
				/// </remarks>
				bool removeImageBar(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					bool bUpdate,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Set/change an image control's colors.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="clrImage">
				/// The color to use for the image (if bChangeColor is true). Everything except the transparent pixels 
				/// is colored.
				/// </param>
				/// 
				/// <param name="clrImageHot">
				/// The color to use for the image when the mouse moves over the control (if bChangeColor is true). 
				/// Everything except the transparent pixels is colored.
				/// </param>
				/// 
				/// <param name="clrText">
				/// The text color.
				/// </param>
				/// 
				/// <param name="clrTextHot">
				/// The text color when the mouse moves over the control.
				/// </param>
				/// 
				/// <param name="clrBorder">
				/// The color to use for the border.
				/// </param>
				/// 
				/// <param name="clrBorderHot">
				/// The color to use for the border when the mouse moves over the control.
				/// </param>
				/// 
				/// <param name="bUpdate">
				/// Whether to update the image control immediately.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// If you intend to change various attributes of the same image consecutively set bUpdate to false and 
				/// use a "sweeping" call to updateImage() when done with all the changes.
				/// </remarks>
				bool setImageColors(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					COLORREF clrImage,
					COLORREF clrImageHot,
					COLORREF clrText,
					COLORREF clrTextHot,
					COLORREF clrBorder,
					COLORREF clrBorderHot,
					bool bUpdate,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Update an image control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// To be called after changeImage(), changeImageText(), setImageBar(), setImageBar() and 
				/// setImageColors(). For performance reasons it is highly recommended that if there are multiple 
				/// consecutive calls to the above mentioned functions you set the bUpdate flag to false in each of 
				/// them and do a "sweeping" update by a single call to this function. NOTE: unlike using the bUpdate 
				/// flag in each of the above mentioned functions, this call updates the ENTIRE image control.
				/// As such, prefer the flag for single function calls and this call for a sweeping redraw.
				/// </remarks>
				bool updateImage(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID, std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Save image contained within an image control to file.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="format">
				/// Image format.
				/// </param>
				/// 
				/// <param name="maxSize">
				/// The maximum size of the image, in pixels. Set to 0x0 to ignore this parameter.
				/// </param>
				/// 
				/// <param name="sFullPath">
				/// Full path of image file (with or without a file extension). NOTE: The actual full path that the 
				/// image is saved to will be written back.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// When imgFormat::NONE is used the file will be saved without an extension but the internal format 
				/// used will be PNG.
				/// </remarks>
				bool saveImage(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					imgFormat format,
					SIZE maxSize,
					std::basic_string<TCHAR> &sFullPath,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Get the caption text of an image control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page containing the control.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sText">
				/// The image's caption text.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool getImageText(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					std::basic_string<TCHAR> &sText,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Add a rectangle control to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="clr">
				/// The fill color of the rectangle.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addRect(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID, COLORREF clr,
					RECT rc, onResize resize);

				/// <summary>
				/// Add a hairline control to the window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="clr">
				/// The color of the line.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void addHairLine(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID, COLORREF clr,
					RECT rc, onResize resize);

				/// <summary>
				/// Add star rating control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="sTooltip">
				/// The tooltip to be displayed when the mouse is hovered over the control.
				/// </param>
				/// 
				/// <param name="clrBorder">
				/// The color of the control's border.
				/// </param>
				/// 
				/// <param name="clrOn">
				/// The color when the control is on.
				/// </param>
				/// 
				/// <param name="clrOff">
				/// The color when the control is off
				/// </param>
				/// 
				/// <param name="clrHot">
				/// The color when the mouse is over the control.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="resize">
				/// The behavior of the control when the window is resized.
				/// </param>
				/// 
				/// <param name="iInitialRating">
				/// The initial rating on a scale indicated by iHighestRating.
				/// </param>
				/// 
				/// <param name="iHighestRating">
				/// The highest rating. Recommended value is 5. The internally set value is
				/// written back to this parameter. A value less than 3 or greater than 50
				/// is not permitted.
				/// </param>
				/// 
				/// <param name="bStatic">
				///  Whether this is a static control.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				/// 
				/// <remarks>
				/// The rating can be set by hovering over the control and clicking. To remove the
				/// rating right click the control.
				/// </remarks>
				void addStarRating(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					const std::basic_string<TCHAR> &sTooltip,
					COLORREF clrBorder,
					COLORREF clrOn,
					COLORREF clrOff,
					COLORREF clrHot,
					RECT rc, onResize resize, int iInitialRating, int &iHighestRating, bool bStatic);

				/// <summary>
				/// Get the star rating of a star rating control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="iRating">
				/// The rating on a scale of 0-5.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool getStarRating(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					int &iRating,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Set the star rating of a star rating control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="iRating">
				/// The rating on a scale of 0 - iHighestRating (defined in cui_raw::addStarRating()).
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information if function fails and returns false.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool setStarRating(
					const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					int iRating,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Set the window's icons.
				/// </summary>
				/// 
				/// <param name="IDI_IconLarge">
				/// Large icon (resource) for use by Windows.
				/// </param>
				/// 
				/// <param name="IDI_IconSmall">
				/// Small icon (resource) for use by Windows.
				/// </param>
				/// 
				/// <param name="IDP_PNGIconSmall">
				/// Small PNG icon (resource). Ideal size is 16x16.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void setIcons(
					int IDI_IconLarge,
					int IDI_IconSmall,
					int IDP_PNGIconSmall
				);

				/// <summary>
				/// Prevent the shadow from being displayed.
				/// </summary>
				/// 
				/// <remarks>
				/// This function must be called before create().
				/// </remarks>
				void hideShadow();

				/// <summary>
				/// Create the window.
				/// </summary>
				/// 
				/// <param name="iInitID">
				/// The ID to send to the command procedure when initialization is complete but before the window 
				/// is displayed.
				/// </param>
				/// 
				/// <param name="iShutdownID">
				/// The ID to send to the command procedure just before the window is destroyed.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// This function will block until the main window is destroyed.
				/// Control events are sent to the command procedure.
				/// 1. Button controls - pData is not used.
				/// 2. Text controls - pData is a pointer to the textMsg struct.
				/// 3. Image controls - pData is not used.
				/// 4. ComboBox controls - pData is a pointer to the comboboxMsg struct.
				/// 5. Listview controls - pData is a pointer to the listviewMsg struct.
				/// 6. Edit controls - pData is not used.
				/// 7. Selector controls - pData is not used.
				/// 8. UpDown controls - pData is not used.
				/// </remarks>
				bool create(int iInitID,
					int iShutdownID
				);

				/// <summary>
				/// Create the window.
				/// </summary>
				/// 
				/// <param name="iInitID">
				/// The ID to send to the command procedure when initialization is complete but before the window 
				/// is displayed.
				/// </param>
				/// 
				/// <param name="iShutdownID">
				/// The ID to send to the command procedure just before the window is destroyed.
				/// </param>
				/// 
				/// <param name="bActivate">
				/// Whether to activate the window.
				/// </param>
				/// 
				/// <param name="bTopMost">
				/// Whether to make this a topmost window.
				/// </param>
				/// 
				/// <param name="bVisible">
				/// Whether the window is initially visible (overrides bActivate if set to false).
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// This function will block until the main window is destroyed.
				/// Control events are sent to the command procedure.
				/// 1. Button controls - pData is not used.
				/// 2. Text controls - pData is a pointer to the textMsg struct.
				/// 3. Image controls - pData is not used.
				/// 4. ComboBox controls - pData is a pointer to the comboboxMsg struct.
				/// 5. Listview controls - pData is a pointer to the listviewMsg struct.
				/// 6. Edit controls - pData is not used.
				/// 7. Selector controls - pData is not used.
				/// 8. UpDown controls - pData is not used.
				/// </remarks>
				bool create(int iInitID,
					int iShutdownID,
					bool bActivate,
					bool bTopMost,
					bool bVisible
				);

				/// <summary>
				/// Register this instance so other instances can find it and open it.
				/// </summary>
				/// 
				/// <param name="sUniqueRegID">
				/// A unique ID that will enable other instances to find this instance. Use a GUID.
				/// </param>
				/// 
				/// <param name="iCopyDataID">
				/// The unique ID to be called when another instance sends data to this instance.
				/// Set to 0 to disable handling of inter-process data.
				/// </param>
				void registerInstance(
					const std::basic_string<TCHAR> &sUniqueRegID,
					int iCopyDataID
				);

				/// <summary>
				/// Open an existing registered instance.
				/// </summary>
				/// 
				/// <param name="sUniqueRegID">
				/// The unique ID of the instance to look for.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if another instance is found and opened, else returns false.
				/// </returns>
				bool openExistingInstance(const std::basic_string<TCHAR> &sUniqueRegID);

				/// <summary>
				/// Close the window (if closing is not blocked by preventQuit()).
				/// </summary>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				/// 
				/// <remarks>
				/// This function returns immediately. The window will not be closed immediately. The window will not be closed if allowQuit() has not been called after a call to preventQuit().
				/// </remarks>
				void close();

				/// <summary>
				/// Close the window (regardless of preventQuit() having been called).
				/// </summary>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				/// 
				/// <remarks>
				/// This function returns immediately. The window will not be closed immediately. NOTE: use this function carefully, as strange behaviour may occur, e.g. if there is a message box displayed.
				/// In such a case the app may disappear and hang, making the only way to close it completely to be via the taskmanager, or by signing out of Windows.
				/// </remarks>
				void closeHard();

				/// <summary>
				/// Hide this window.
				/// </summary>
				void hideWindow();

				/// <summary>
				/// Show this window.
				/// </summary>
				void showWindow();

				/// <summary>
				/// Show this window.
				/// </summary>
				/// 
				/// <param name="bForeground">
				/// Whether to bring it to the foreground.
				/// </param>
				void showWindow(bool bForeground);

				/// <summary>
				/// Get the name of the current page.
				/// </summary>
				/// 
				/// <returns>
				/// Returns the name of the current page.
				/// </returns>
				std::basic_string<TCHAR> currentPage();

				/// <summary>
				/// Add page to window.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The name of the page to be added to the window.
				/// </param>
				/// 
				/// <param name="pCommandProc">
				/// The command procedure to be used by the page's controls.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if the page has been added successfully and false if it already exists.
				/// </returns>
				/// 
				/// <remarks>
				/// After adding a page to the window using this function, controls can be added to that page 
				/// by specifying the page in the sPageName parameter when adding those controls.
				/// The page can be switched to by calling the showPage() function.
				/// </remarks>
				bool addPage(const std::basic_string<TCHAR> &sPageName, CommandProcedure pCommandProc);

				/// <summary>
				/// Check if a page exists.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The name of the page.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if the page exists, else false.
				/// </returns>
				bool checkPage(const std::basic_string<TCHAR> &sPageName);

				/// <summary>
				/// Add tab control (to be called before addTab) ... defines the region to be occupied by the tab 
				/// control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				/// 
				/// <param name="rc">
				/// The rectangular coordinates of the control, in pixels.
				/// </param>
				/// 
				/// <param name="sFontName">
				/// The font to be used for the text.
				/// </param>
				/// 
				/// <param name="iFontSize">
				/// The size of the font, in points.
				/// </param>
				/// 
				/// <param name="clrTabLine">
				/// The color of the line used to seperate the actual tabs from the content area.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool addTabControl(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					RECT rc,
					const std::basic_string<TCHAR> &sFontName, double iFontSize, COLORREF clrTabLine);

				/// <summary>
				/// Add a tab to page. NOTE: all controls that are added after this function is called will be added to 
				/// the tab.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="sTabName">
				/// The name of the tab.
				/// </param>
				/// 
				/// <param name="sTooltip">
				/// The tooltip to be displayed when the mouse moves over the control.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// This function should only be called after the addTabControl() function has been called.
				/// </remarks>
				bool addTab(const std::basic_string<TCHAR> &sPageName,
					const std::basic_string<TCHAR> &sTabName,
					const std::basic_string<TCHAR> &sTooltip);

				/// <summary>
				/// Show a particular tab. NOTE: should follow addTab() after controls have beed added but before 
				/// showPage().
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page in which the tab is.
				/// </param>
				/// 
				/// <param name="sTabName">
				/// The name of the tab.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// This function should only be called after the addTabControl() function has been called.
				/// </remarks>
				bool showTab(const std::basic_string<TCHAR> &sPageName,
					const std::basic_string<TCHAR> &sTabName);

				/// <summary>
				/// Get the selected tab in a tab control.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The name of the page the control is in.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique id of the control.
				/// </param>
				/// 
				/// <param name="sSelectedTab">
				/// The name of the selected tab.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				bool getSelectedTab(const std::basic_string<TCHAR> &sPageName,
					int iUniqueID,
					std::basic_string<TCHAR> &sSelectedTab,
					std::basic_string<TCHAR> &sErr);

				/// <summary>
				/// Create a page.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The name of the page.
				/// </param>
				/// 
				/// <remarks>
				/// This function follow addPage() after controls have been added to a page, but before showPage().
				/// Note: page will only become visible after showPage() is called.
				/// </remarks>
				void createPage(const std::basic_string<TCHAR> &sPageName);

				/// <summary>
				/// Show created page - should follow createPage().
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page to be shown.
				/// </param>
				/// 
				/// <param name="bCreatePage">
				/// Whether to create the page (if it hasn't already been created).
				/// Note: It's not necessary to set this flag to true if createPage() has been called.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void showPage(const std::basic_string<TCHAR> &sPageName, bool bCreatePage);

				/// <summary>
				/// Get the name of the page that was open before the current page.
				/// </summary>
				/// 
				/// <returns>
				/// Returns the name of the previous page.
				/// </returns>
				std::basic_string<TCHAR> previousPage();

				/// <summary>
				/// Get the main window's title (home page).
				/// </summary>
				/// 
				/// <returns>
				/// Returns the name of the main window (home page).
				/// </returns>
				std::basic_string<TCHAR> getTitle();

				/// <summary>
				/// Get main window's width (unscaled for DPI).
				/// </summary>
				/// 
				/// <returns>
				/// Returns the width of the window, in pixels.
				/// </returns>
				int getWidth();

				/// <summary>
				/// Get main window's height (unscaled for DPI).
				/// </summary>
				/// 
				/// <returns>
				/// Returns the height of the window, in pixels.
				/// </returns>
				int getHeight();

				/// <summary>
				/// Get main window's x position.
				/// </summary>
				/// 
				/// <returns>
				/// Returns the x-position of the window, in pixels.
				/// </returns>
				int getX();

				/// <summary>
				/// Get main window's y position.
				/// </summary>
				/// 
				/// <returns>
				/// Returns the y-position of the window, in pixels.
				/// </returns>
				int getY();

				/// <summary>
				/// Set main window position, in pixels.
				/// </summary>
				/// 
				/// <param name="x">
				/// x-coordinate.
				/// </param>
				/// 
				/// <param name="y">
				/// y-coordinate.
				/// </param>
				void setXY(int x,
					int y
				);

				/// <summary>
				/// Set main window position, in pixels.
				/// </summary>
				/// 
				/// <param name="x">
				/// x-coordinate.
				/// </param>
				/// 
				/// <param name="y">
				/// y-coordinate.
				/// </param>
				/// 
				/// <param name="cx">
				/// The width.
				/// </param>
				/// 
				/// <param name="cy">
				/// The height.
				/// </param>
				void setXY(int x,
					int y,
					int cx,
					int cy
				);

				/// <summary>
				/// Get the coordinates of the working area.
				/// </summary>
				/// 
				/// <param name="rcWork">
				/// The RECT defining the working area.
				/// </param>
				void getWorkingArea(RECT &rcWork);

				/// <summary>
				/// Check whether a window is maximized.
				/// </summary>
				/// 
				/// <returns>
				/// Returns true if the window is maximized, else false.
				/// </returns>
				bool isMaximized();

				/// <summary>
				/// Maximize window.
				/// </summary>
				/// 
				/// <remarks>
				/// This function works whether create() has been called or not.
				/// </remarks>
				void maximize();

				/// <summary>
				/// Get main window's height (unscaled for DPI).
				/// </summary>
				/// 
				/// <returns>
				/// Returns the default title bar height, in pixels.
				/// </returns>
				int getTitleBarHeight();

				/// <summary>
				/// Get current DPI scale.
				/// </summary>
				/// 
				/// <returns>
				/// Returns the DPI scale relative to 96dpi.
				/// </returns>
				double getDPIScale();

				/// <summary>
				/// Set user state information.
				/// </summary>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				/// 
				/// <remarks>
				/// In order to use the data, cast/convert the void pointer to a pointer type of the original data.
				/// </remarks>
				void setState(void* pState);

				/// <summary>
				/// Get user supplied state information.
				/// </summary>
				/// 
				/// <returns>
				/// Returns a void pointer to the state information.
				/// </returns>
				/// 
				/// <remarks>
				/// In order to use the data, cast/convert the void pointer to a pointer type of the original data.
				/// </remarks>
				void* getState();

				/// <summary>
				/// Get the window's native handle.
				/// </summary>
				/// 
				/// <returns>
				/// Returns the Window's native handle if successful, else returns NULL.
				/// </returns>
				/// 
				/// <remarks>
				/// Be very careful with the window handle. Don't use it unless you understand the internal mechanics
				/// of cui_raw, or you will mess things up horribly. As far as possible use the public members of cui_raw to
				/// do any GUI stuff.
				/// </remarks>
				HWND getHWND();

				/// <summary>
				/// Add a control to the list that's disabled when preventQuit() is called.
				/// </summary>
				/// 
				/// <param name="sPageName">
				/// The page into which the control is to be placed.
				/// </param>
				/// 
				/// <param name="iUniqueID">
				/// The unique ID of the control.
				/// </param>
				void addToPreventQuitList(const std::basic_string<TCHAR> &sPageName, int iUniqueID);

				/// <summary>
				/// Prevent window from getting closed.
				/// </summary>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				/// 
				/// <remarks>
				/// The client application should remember to call allowQuit() after completing the critical task
				/// otherwise the only way to close the application will be via closeHard(), the Task Manager or similar.
				/// An internal counter keeps track of how many calls to preventQuit() have been called, and balances
				/// that with the number of calls to allowQuit().
				/// </remarks>
				void preventQuit();

				/// <summary>
				/// Allow window to be closed.
				/// </summary>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				/// 
				/// <remarks>
				/// An internal counter keeps track of how many calls to preventQuit() have been called, and balances
				/// that with the number of calls to allowQuit(). As many allowQuit() calls need to be called as
				/// preventQuit() calls in order to reset that counter. Before that counter is reset close() won't work,
				/// but closeHard() will work either way. However, be careful with closeHard(), as indicated in it's 
				/// documentation.
				/// </remarks>
				void allowQuit();

				/// <summary>
				/// Prevent window resizing.
				/// </summary>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void preventResizing();

				/// <summary>
				/// Allow window resizing.
				/// </summary>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void allowResizing();

				/// <summary>
				/// Check whether window resizing is enabled.
				/// </summary>
				/// 
				/// <returns>
				/// Returns true if resizing is enabled, else false.
				/// </returns>
				bool resizing();

				/// <summary>
				/// Set timer for closing this window automatically.
				/// </summary>
				/// 
				/// <param name="iTimer">
				/// The time, in seconds, it will take for the window to close itself automatically.
				/// 0 = never close automatically.
				/// </param>
				/// 
				/// <param name="bStartOnMouseMove">
				/// Whether to stop the timer only after the mouse has moved (whether over the window or not).
				/// </param>
				/// 
				/// <param name="bStopOnMouseOverWindow">
				/// Whether to stop the timer when the mouse moves over the window.
				/// </param>
				/// 
				/// <remarks>
				/// If the window has not been created yet, the timer will start from the moment it is created.
				/// If the window has already been created, the timer will start immediately.
				/// </remarks>
				void setTimer(unsigned int iTimer, bool bStartOnMouseMove, bool bStopOnMouseOverWindow);

				/// <summary>
				/// Save HBITMAP to file.
				/// </summary>
				/// 
				/// <param name="hBmp">
				/// The bitmap handle.
				/// </param>
				/// 
				/// <param name="sFileName">
				/// Full path to file. NOTE: if the supplied extension is wrong it will be adjusted internally. See
				/// remarks.
				/// </param>
				/// 
				/// <param name="format">
				/// The file format as defined in the cui_raw::imgFormat struct.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// GDI+ must be initialized before this function if called. The final path is written back to
				/// sFileName.
				/// </remarks>
				static bool HBITMAPtoFile(
					HBITMAP hBmp,
					std::basic_string<TCHAR> &sFileName,
					imgFormat format,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Cause window to accept drag-drop files.
				/// </summary>
				/// 
				/// <param name="iDropFilesID">
				/// The unique ID to be called by the window procedure when a file is dropped onto the window.
				/// </param>
				/// 
				/// <remarks>
				/// The file that has been dropped onto the window is sent to the command procedure
				/// via the pData parameter as a pointer to a cui_raw::dropFileMsg struct.
				/// </remarks>
				void dropFilesAccept(int iDropFilesID);

				/// <summary>
				/// Prevent window from accepting drag-drop files.
				/// </summary>
				void dropFilesReject();


			public:
				/// <summary>
				/// File type.
				/// </summary>
				struct fileType
				{
					/// <summary>
					/// File extension (without the dot), e.g. bmp.
					/// </summary>
					std::basic_string<TCHAR> sFileExtension = _T("bmp");

					/// <summary>
					/// File description, e.g. Bitmap Files.
					/// </summary>
					std::basic_string<TCHAR> sDescription = _T("Bitmap Files");
				};

				/// <summary>
				/// Parameters for use with openFile() or saveFile().
				/// </summary>
				struct opensavefileParams
				{
					/// <summary>
					/// Title of the dialog.
					/// </summary>
					std::basic_string<TCHAR> sTitle;

					/// <summary>
					/// File type filter.
					/// </summary>
					std::vector<fileType> vFileTypes;

					/// <summary>
					/// Flag for displaying a small window.
					/// </summary>
					bool bSmallWindow = false;

					/// <summary>
					/// Flag to hide navigation toolbar on the left. NOTE: when set to true clrUIbckgnd is ignored.
					/// </summary>
					bool bHideNavigationToolbar = true;

					/// <summary>
					/// the font to use in the dialog.
					/// </summary>
					std::basic_string<TCHAR> sFontName = _T("Droid Sans");

					/// <summary>
					/// The size of the font to use in the dialog.
					/// </summary>
					double iFontSize = 9;

					/// <summary>
					/// Flag to add option to show all supported file types.
					/// </summary>
					bool bIncludeAllSupportedTypes = true;
				};

				/// <summary>
				/// Open file on computer's file system.
				/// </summary>
				/// 
				/// <param name="params">
				/// Parameters as defined in the opensavefileParams struct.
				/// </param>
				/// 
				/// <param name="sFile">
				/// The full path to the file is written back to this parameter.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void openFile(
					const opensavefileParams &params,
					std::basic_string<TCHAR> &sFile
				);

				/// <summary>
				/// Save file to computer's file system.
				/// </summary>
				/// 
				/// <param name="params">
				/// Parameters as defined in the opensavefileParams struct.
				/// </param>
				/// 
				/// <param name="sFile">
				/// The name of the file (without an extension).
				/// </param>
				/// 
				/// <param name="sFullPath">
				/// The full path to the file is written back to this parameter.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void saveFile(
					const opensavefileParams &params,
					const std::basic_string<TCHAR> &sFile,
					std::basic_string<TCHAR> &sFullPath
				);

				/// <summary>
				/// Parameters for use with selectFolder().
				/// </summary>
				struct selectFolderParams
				{
					/// <summary>
					/// The title of the dialog.
					/// </summary>
					std::basic_string<TCHAR> sTitle;

					/// <summary>
					/// The message to display in the dialog.
					/// </summary>
					std::basic_string<TCHAR> sMessage;

					/// <summary>
					/// Flag for displaying a small window.
					/// </summary>
					bool bSmallWindow = false;

					/// <summary>
					/// The font to use in the dialog.
					/// </summary>
					std::basic_string<TCHAR> sFontName = _T("Droid Sans");

					/// <summary>
					/// The size of the font to use in the dialog.
					/// </summary>
					double iFontSize = 9;
				};

				/// <summary>
				/// Select a folder on the computer's file system.
				/// </summary>
				/// 
				/// <param name="params">
				/// Parameters as defined in the selectFolderParams struct.
				/// </param>
				/// 
				/// <param name="sPath">
				/// The full path to the folder is written back to this parameter.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				void selectFolder(
					const selectFolderParams &params,
					std::basic_string<TCHAR> &sPath
				);

				/// <summary>
				/// Message box result.
				/// </summary>
				enum msgBoxResult
				{
					/// <summary>
					/// No response received from the user.
					/// </summary>
					noresponse = 0,

					/// <summary>
					/// Message box was not executed (there is another displayed).
					/// </summary>
					notexecuted,

					/// <summary>
					/// User selected ok.
					/// </summary>
					ok,

					/// <summary>
					/// User selected cancel.
					/// </summary>
					cancel,

					/// <summary>
					/// User selected yes.
					/// </summary>
					yes,

					/// <summary>
					/// User selected no.
					/// </summary>
					no,
				};

				/// <summary>
				/// Message box type.
				/// </summary>
				enum msgBoxType
				{
					/// <summary>
					/// Message box with only an OK button.
					/// </summary>
					okOnly,

					/// <summary>
					/// Message box with an OK and a Cancel button.
					/// </summary>
					okCancel,

					/// <summary>
					/// Message box with an Yes and a No button.
					/// </summary>
					yesNo
				};

				/// <summary>
				/// Parameters for use with msgBox().
				/// </summary>
				struct msgBoxParams
				{
					/// <summary>
					/// The message to display in the dialog.
					/// </summary>
					std::basic_string<TCHAR> sMessage;

					/// <summary>
					/// The details to display in the dialog.
					/// </summary>
					std::basic_string<TCHAR> sDetails;

					/// <summary>
					/// The font to be used in the dialog.
					/// </summary>
					std::basic_string<TCHAR> sFontName = _T("Droid Sans");

					/// <summary>
					/// The size of the font to use in the dialog.
					/// </summary>
					double iFontSize = 9;

					/// <summary>
					/// Message box type.
					/// </summary>
					msgBoxType type;

					/// <summary>
					/// The ID of the PNG resource to use for the icon in the message box.
					/// </summary>
					/// 
					/// <remarks>
					/// The icon will be downsized to a size of 32x32 if it's larger.
					/// </remarks>
					int IDP_ICON = 0;

					/// <summary>
					/// The color of the icon.
					/// </summary>
					COLORREF clrImage = RGB(0, 0, 0);
				};

				/// <summary>
				/// Display a message box.
				/// </summary>
				/// 
				/// <param name="params">
				/// Parameters as defined in the msgBoxParams struct.
				/// </param>
				/// 
				/// <returns>
				/// The user's response as defined in the msgBoxResult enumeration.
				/// </returns>
				/// 
				/// <remarks>
				/// Quicker for message boxes when there's an existing window.
				/// </remarks>
				msgBoxResult msgBox(
					const msgBoxParams &params
				);

				/// <summary>
				/// Display a message box.
				/// </summary>
				/// 
				/// <param name="params">
				/// Parameters as defined in the msgBoxParams struct.
				/// </param>
				/// 
				/// <param name="sTitle">
				/// The title of the dialog.
				/// </param>
				/// 
				/// <param name="clrBackground">
				/// The window's background color.
				/// </param>
				/// 
				/// <param name="clrTheme">
				/// The window's theme color.
				/// </param>
				/// 
				/// <param name="clrThemeHot">
				/// The color to be used when the mouse is hovering over some items; e.g. the close button.
				/// </param>
				/// 
				/// <param name="sTooltipFont">
				/// The name of the font to be used for displaying tooltips.
				/// </param>
				/// 
				/// <param name="iTooltipFontSize">
				/// The size of the font, in points, to be used for displaying tooltips.
				/// </param>
				/// 
				/// <param name="clrTooltipText">
				/// The color of the text in tooltip controls.
				/// </param>
				/// 
				/// <param name="clrTooltipBackground">
				/// The background color of tooltip controls.
				/// </param>
				/// 
				/// <param name="clrTooltipBorder">
				/// The border color of tooltip controls.
				/// </param>
				/// 
				/// <param name="IDI_ICON">
				/// Large icon (resource) for use by Windows.
				/// </param>
				/// 
				/// <param name="IDI_ICONSMALL">
				/// Small icon (resource) for use by Windows.
				/// </param>
				/// 
				/// <param name="IDP_ICONSMALL">
				/// Small PNG icon (resource). Ideal size is 16x16.
				/// </param>
				/// 
				/// <param name="hResModule">
				/// Handle to the module (DLL or exe) that contains the resources to be used (PNGs etc).
				/// </param>
				/// 
				/// <param name="pParent">
				/// A pointer to the window's parent (set to NULL if this is not a child window).
				/// </param>
				/// 
				/// <returns>
				/// The user's response as defined in the msgBoxResult enumeration.
				/// </returns>
				/// 
				/// <remarks>
				/// Useful for displaying message boxes when there is no existing window.
				/// </remarks>
				static msgBoxResult msgBox(
					const msgBoxParams &params,
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
				);

				/// <summary>
				/// Parameters for use with popup notifications.
				/// </summary>
				/// 
				/// <seealso cref="not()">
				/// </seealso>
				struct notParams
				{
					/// <summary>
					/// The message to display in the dialog.
					/// </summary>
					std::basic_string<TCHAR> sMessage;

					/// <summary>
					/// The details to display in the dialog.
					/// </summary>
					std::basic_string<TCHAR> sDetails;

					/// <summary>
					/// The font to be used in the dialog.
					/// </summary>
					std::basic_string<TCHAR> sFontName = _T("Droid Sans");

					/// <summary>
					/// The size of the font to use in the dialog.
					/// </summary>
					double iFontSize = 9;

					/// <summary>
					/// The ID of the PNG resource to use for the icon in the message box.
					/// </summary>
					/// 
					/// <remarks>
					/// The icon will be downsized to a size of 32x32 if it's larger.
					/// </remarks>
					int IDP_ICON = 0;

					/// <summary>
					/// The color of the icon.
					/// </summary>
					COLORREF clrImage = RGB(0, 0, 0);

					/// <summary>
					/// The time, in seconds, it will take for the popup notification to close itself automatically.
					/// Set to 0 to prevent it from closing automatically.
					/// </summary>
					unsigned int iTimer = 0;

					/// <summary>
					/// Whether to add a shadow to the notification window.
					/// </summary>
					bool bShadow = false;
				};

				/// <summary>
				/// Display a popup notification.
				/// </summary>
				/// 
				/// <param name="params">
				/// Parameters as defined in the notParams struct.
				/// </param>
				/// 
				/// <returns>
				/// No return value.
				/// </returns>
				/// 
				/// <remarks>
				/// On Windows versions before 10 the popup appears on the bottom right;
				/// on later versions it appears on the top right.
				/// This functions is non-blocking. It returns almost immediately. If there is already another
				/// notification currently displayed the current call will place this notification into a queue. The
				/// notification will be displayed one second after the last notification is closed.
				/// When the notification is displayed, it will remain displayed until ten seconds after the user 
				/// moves the mouse. If the user moves the mouse above it, however, it will not close automatically;
				/// in this case the only way to close it is using it's close button or Alt + F4.
				/// </remarks>
				void notX(
					const notParams &params
				);

				/// <summary>
				/// Splash screen class.
				/// </summary>
				class cui_api CSplashScreen
				{
				public:
					CSplashScreen();
					~CSplashScreen();

					/// <summary>
					/// Load splash screen.
					/// </summary>
					/// 
					/// <param name="hResModule">
					/// Handle to the module (DLL or exe) that contains the resources to be used (PNGs etc).
					/// </param>
					/// 
					/// <param name="IDI_ICONSMALL">
					/// Small icon (resource) for use by Windows.
					/// </param>
					/// 
					/// <param name="IDP_SPLASH">
					/// Splash screen PNG image resource.
					/// </param>
					/// 
					/// <param name="sErr">
					/// Error information.
					/// </param>
					/// 
					/// <returns>
					/// Returns true if successful, else false.
					/// </returns>
					bool Load(
						HMODULE hResModule,
						int IDI_ICONSMALL,
						int IDP_SPLASH,
						std::basic_string<TCHAR> &sErr
					);

					/// <summary>
					/// Remove splash screen.
					/// </summary>
					/// 
					/// <returns>
					/// No return value.
					/// </returns>
					void Remove();

				private:
					CSplashScreenImpl *d;
				}; // CSplashScreen

				struct trayIconItem
				{
					int iUniqueID;
					std::basic_string<TCHAR> sLabel;
					bool bDefault = false;
					bool bEnabled = true;
				};

				/// <summary>
				/// Add icon to the system tray.
				/// </summary>
				/// 
				/// <param name="IDI_SMALLICON">
				/// The icon resource containing the small icon.
				/// </param>
				/// 
				/// <param name="sTitle">
				/// The title displayed when the mouse is hovered over the icon.
				/// </param>
				/// 
				/// <param name="trayItems">
				/// A list of the tray items in the menu, as defined in the cui_raw::trayIconItem struct.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// The icon is removed automatically on application exit if removeTrayIcon() is not called before that.
				/// This function will only work AFTER the main window has been created.
				/// If there is already a tray icon, this function does nothing.
				/// </remarks>
				bool addTrayIcon(
					int IDI_SMALLICON,
					const std::basic_string<TCHAR> &sTitle,
					const std::vector<trayIconItem> &trayItems,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Change tray icon.
				/// </summary>
				/// 
				/// <param name="IDI_SMALLICON">
				/// The icon resource containing the small icon.
				/// </param>
				/// 
				/// <param name="sTitle">
				/// The title displayed when the mouse is hovered over the icon.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// The icon is removed automatically on application exit if removeTrayIcon() is not called before that.
				/// This function will only work AFTER the main window has been created AND addTrayIcon has been called.
				/// If there is already a tray icon, this function does nothing.
				/// </remarks>
				bool changeTrayIcon(
					int IDI_SMALLICON,
					const std::basic_string<TCHAR> &sTitle,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Remove the tray icon added by a call to cui_raw::addTrayIcon().
				/// </summary>
				/// 
				/// <remarks>
				/// Even if this function is never called, it will be called internally in the cui_raw destructor.
				/// </remarks>
				void removeTrayIcon();

				/// <summary>
				/// Resize image.
				/// </summary>
				/// 
				/// <param name="sSourceFullPath">
				/// The full path to the source file.
				/// </param>
				/// 
				/// <param name="format">
				/// The format to use for the destination file as defined in cui_raw::imgFormat
				/// </param>
				/// 
				/// <param name="sDestinationFullPath">
				/// The full path to the destination.
				/// </param>
				/// 
				/// <param name="maxSize">
				/// The maximum size of the image.
				/// </param>
				/// 
				/// <param name="realSize">
				/// The size of the resized image.
				/// </param>
				/// 
				/// <param name="sErr">
				/// Error information.
				/// </param>
				/// 
				/// <returns>
				/// Returns true if successful, else false.
				/// </returns>
				/// 
				/// <remarks>
				/// Final path is written back to sDestinationFullPath. Actual size if written back to realSize.
				/// </remarks>
				static bool resizeImage(
					const std::basic_string<TCHAR> &sSourceFullPath,
					imgFormat format,
					std::basic_string<TCHAR> &sDestinationFullPath,
					SIZE maxSize,
					SIZE &realSize,
					std::basic_string<TCHAR> &sErr
				);

				/// <summary>
				/// Display a color picker dialog.
				/// </summary>
				/// 
				/// <param name="bColorPicked">
				/// Whether a color was picked. False is written back if the user selects cancel.
				/// </param>
				/// 
				/// <param name="rgb">
				/// The color that the user selected.
				/// </param>
				void pickColor(
					bool &bColorPicked,
					COLORREF &rgb
				);

			private:
				cui_rawImpl *d;
				friend class cui_rawImpl;
			}; // cui_raw

		} // namespace gui_raw

	} // namespace cui

} // namespace liblec
