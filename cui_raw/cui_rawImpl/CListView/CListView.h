/*
** CListView.h - listview handling - interface
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

#include <Windows.h>
#include <CommCtrl.h>
#include <vector>
#include <string>
#include <map>

/*
** Clistview - listview handling class
*/
class Clistview
{
public:
	typedef void(*CONTEXTFUNC)(HWND, WPARAM, LPARAM);

	enum enType
	{
		String,
		Char,
		Int,
		Float
	};

	Clistview();
	~Clistview();

	/*
	** initialize listview handler
	*/
	void Init(
		HWND hlistview,																					// list view handle
		HBITMAP hBmp = NULL,																			// listview background bitmap
		DWORD_PTR dwExtendedStyle = LVS_EX_FULLROWSELECT | LVS_EX_HEADERDRAGDROP | LVS_EX_GRIDLINES | LVS_EX_DOUBLEBUFFER,	// extended list view style
		UINT iMask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_FMT,									// list view mask
		int iFormat = LVCFMT_LEFT																		// list view format
	);

	/*
	** add column to list view
	*/
	void AddColumn(
		int iColNumber,			// column number
		const std::basic_string<TCHAR> &sColName,	// column name
		int iColWidth,			// column width
		enType enColType		// column type
	);

	/*
	** insert item into list view
	** iRowNumber and iColNumber counted from 0
	*/
	void InsertItem(
		int iRowNumber,			// row number
		int iColNumber,			// column number
		const std::basic_string<TCHAR> &sValue		// value of item to be inserted
	);

	/*
	** update item in list view
	** iRowNumber and iColNumber counted from 0
	*/
	void UpdateItem(
		int iRowNumber,			// row number
		int iColNumber,			// column number
		const std::basic_string<TCHAR> &sValue		// value of item to be inserted
	);

	/*
	** remove item from listview
	*/
	void RemoveRow(
		int iRowNumber
	);

	/*
	** reset index of selected item
	*/
	void ResetIndex();

	/*
	** get index of next selected item
	** returns -1 if there is no item selected or if out of range
	*/
	int GetNextIndex();

	/*
	** get number of selected items
	*/
	int Selected();

	/*
	** get the selected item's text
	*/
	std::basic_string<TCHAR> GetSelected(int iColNumber);

	/*
	** get all list-view text
	** size of vector is rows x columns
	*/
	void GetInfo(
		std::vector<std::basic_string<TCHAR>> &vCols,			// column names
		std::vector<int> &vColWidth,		// column widths
		std::vector<enType> &vColType,	// column type
		std::vector<std::vector<std::basic_string<TCHAR>>> &vInfo		// listview text
	);

	/*
	** clear list view contents
	*/
	void Clear();

	/*
	** clear list view columns
	*/
	void ClearColumns();

	/*
	** resize listview width
	*/
	void ResizeW();

	/*
	** resize listview height
	*/
	void ResizeH();

	/*
	** resize listview width and height
	*/
	void ResizeWH();

	// get number of columns
	int Get_NumOfCols();

	// get number of rows
	int Get_NumOfRows();

private:
	HWND m_hlistview;		// list view handle
	int m_iNumOfCols;		// number of columns
	int m_iPrevRow;			// previous row
	int m_iIndex;			// index of selected item
	std::vector<enType> m_vColTypes;	// types
	int m_parent_w;	// parent window width
	int m_parent_h;	// parent window height
	int m_list_w;	// listview initial width
	int m_list_h;	// listview initial height

	LVCOLUMN lvc;	// list view column
	LVITEM lv;		// list view item

	HWND m_hWndParent;		// handle of parent window
}; // Clistview
