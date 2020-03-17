/*
** CListView.h - listview handling - implementation
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

#include "Clistview.h"
#include <tchar.h>

#ifdef _UNICODE
#define to_tstring	std::to_wstring
#else
#define to_tstring	std::to_string
#endif // _UNICODE

#define WM_CLICKED 1111119	// message to be sent to listview to indicate it's column has been clicked

// constructor
Clistview::Clistview()
{
	m_hWndParent = NULL;
	m_hlistview = NULL;
	m_iPrevRow = -1;
	m_iIndex = -1;
	m_vColTypes.clear();
	m_iNumOfCols = 0;
	m_parent_w = 0;
	m_parent_h = 0;
	m_list_w = 0;
	m_list_h = 0;
};

// destructor
Clistview::~Clistview()
{
}

/*
** initialize listview handler
*/
void Clistview::Init(
	HWND hlistview,				// list view handle
	HBITMAP hBmp,				// listview background bitmap
	DWORD_PTR dwExtendedStyle,	// extended list view style
	UINT iMask,					// list view mask
	int iFormat					// list view format
)
{
	m_hlistview = hlistview;
	m_hWndParent = GetParent(hlistview);

	m_iNumOfCols = 0;
	m_vColTypes.clear();

	// initialize list view column and item
	lvc = { 0 };
	lv = { 0 };

	// set extended list view style
	ListView_SetExtendedListViewStyle(m_hlistview, dwExtendedStyle);

	// set list view mask
	lvc.mask = iMask;

	// set list view format
	lvc.fmt = iFormat;

	// capture parent dimensions
	RECT rect;
	GetClientRect(m_hWndParent, &rect);
	m_parent_w = rect.right - rect.left;
	m_parent_h = rect.bottom - rect.top;

	// capture listview dimensions
	GetClientRect(m_hlistview, &rect);
	m_list_w = rect.right - rect.left;
	m_list_h = rect.bottom - rect.top;

	ResizeWH();

	if (hBmp != NULL)
	{
		LVBKIMAGE lvbki = { 0 };

		lvbki.ulFlags = LVBKIF_SOURCE_HBITMAP | LVBKIF_STYLE_NORMAL;
		lvbki.hbm = hBmp;
		lvbki.xOffsetPercent = 100;
		lvbki.yOffsetPercent = 100;

		SendMessage(m_hlistview, LVM_SETBKIMAGE, 0, reinterpret_cast<LPARAM>(&lvbki));
	}
} // Init

  // get number of columns
int Clistview::Get_NumOfCols()
{
	return m_iNumOfCols;
}

// get number of rows
int Clistview::Get_NumOfRows()
{
	return ListView_GetItemCount(m_hlistview);
}

/*
** add column to list view
*/
void Clistview::AddColumn(
	int iColNumber,			// column number
	const std::basic_string<TCHAR> &sColName,	// column name
	int iColWidth,			// column width
	enType enColType		// column type
)
{
	/*
	** add columns to the list-view
	*/
	lvc.iSubItem = iColNumber;
	lvc.cx = iColWidth;

	TCHAR _colname[MAX_PATH];
	lstrcpyn(_colname, sColName.c_str(), _countof(_colname));
	lvc.pszText = _colname;

	ListView_InsertColumn(m_hlistview, m_iNumOfCols, &lvc);

	m_vColTypes.push_back(enColType);

	// increase number of columns
	m_iNumOfCols++;
} // AddColumn

  /*
  ** resize listview width
  */
void Clistview::ResizeW()
{
	// capture new parent dimensions
	RECT rect;
	GetClientRect(m_hWndParent, &rect);
	int m_newparent_w = rect.right - rect.left;
	int m_newparent_h = rect.bottom - rect.top;

	int iWidthChange = m_newparent_w - m_parent_w;

	SetWindowPos(m_hlistview, 0, 0, 0, m_list_w + iWidthChange, m_list_h, SWP_NOZORDER | SWP_NOMOVE);

	return;
} // ResizeW

  /*
  ** resize listview height
  */
void Clistview::ResizeH()
{
	// capture new parent dimensions
	RECT rect;
	GetClientRect(m_hWndParent, &rect);
	int m_newparent_w = rect.right - rect.left;
	int m_newparent_h = rect.bottom - rect.top;

	int iHeightChange = m_newparent_h - m_parent_h;

	SetWindowPos(m_hlistview, 0, 0, 0, m_list_w, m_list_h + iHeightChange, SWP_NOZORDER | SWP_NOMOVE);

	return;
} // ResizeH

  /*
  ** resize listview width and height
  */
void Clistview::ResizeWH()
{
	// capture new parent dimensions
	RECT rect;
	GetClientRect(m_hWndParent, &rect);
	int m_newparent_w = rect.right - rect.left;
	int m_newparent_h = rect.bottom - rect.top;

	int iWidthChange = m_newparent_w - m_parent_w;
	int iHeightChange = m_newparent_h - m_parent_h;

	SetWindowPos(m_hlistview, 0, 0, 0, m_list_w + iWidthChange, m_list_h + iHeightChange, SWP_NOZORDER | SWP_NOMOVE);

	return;
} // ResizeWH

  /*
  ** insert item into list view
  ** iRowNumber and iColNumber counted from 0
  */
void Clistview::InsertItem(
	int iRowNumber,			// row number
	int iColNumber,			// column number
	const std::basic_string<TCHAR> &sValue		// value of item to be inserted
)
{
	LVITEM lv = { 0 };

	int i = (int)sValue.length() + 1;
	TCHAR *buf = new TCHAR[i];
	lstrcpyn(buf, sValue.c_str(), i);

	// check if this row is after previous row
	if (iRowNumber > m_iPrevRow)
	{
		// check the difference (how many rows ahead this row is)
		int iDiff = iRowNumber - m_iPrevRow;

		// insert as many rows as the difference
		for (int i = 0; i < iDiff; i++)
		{
			lv.iItem = iRowNumber + i;
			ListView_InsertItem(m_hlistview, &lv);
		}
	}

	// set the text of the cell
	ListView_SetItemText(m_hlistview, iRowNumber, iColNumber, buf);

	m_iPrevRow = iRowNumber;

	delete buf;
} //InsertItem

  /*
  ** update item in list view
  ** iRowNumber and iColNumber counted from 0
  */
void Clistview::UpdateItem(
	int iRowNumber,			// row number
	int iColNumber,			// column number
	const std::basic_string<TCHAR> &sValue		// value of item to be inserted
)
{
	LVITEM lv = { 0 };

	int i = (int)sValue.length() + 1;
	TCHAR *buf = new TCHAR[i];
	lstrcpyn(buf, sValue.c_str(), i);

	// check if this row is after previous row
	if (iRowNumber <= m_iPrevRow && iColNumber <= m_iNumOfCols)
	{
		// set the text of the cell
		ListView_SetItemText(m_hlistview, iRowNumber, iColNumber, buf);
	}

	delete buf;
} //UpdateItem

  /*
  ** remove item from listview
  */
void Clistview::RemoveRow(
	int iRowNumber
)
{
	if (iRowNumber <= m_iPrevRow)
	{
		ListView_DeleteItem(m_hlistview, iRowNumber);
		m_iPrevRow--;
	}
} // RemoveRow

  /*
  ** reset index of selected item
  */
void Clistview::ResetIndex()
{
	m_iIndex = -1;
} // ResetIndex

  /*
  ** get index of next selected item
  ** returns -1 if there is no item selected or if out of range
  */
int Clistview::GetNextIndex()
{
	m_iIndex = ListView_GetNextItem(m_hlistview, m_iIndex, LVNI_SELECTED);
	return m_iIndex;
} // GetIndex

std::basic_string<TCHAR> GetItemText(HWND hWndListView, int nItem, int nSubItem)
{
	LV_ITEM item;
	memset(&item, 0, sizeof(LV_ITEM));
	item.iSubItem = nSubItem;

	int nLen = 128; //initial reasonable string length

	int ReturnCode = 0;

	TCHAR *buf = NULL;

	do
	{
		if (buf)
		{
			delete buf;
			nLen *= 2; //resize the string buffer (double)
		}

		// dynamic allocation
		buf = new TCHAR[nLen];

		item.cchTextMax = nLen;
		item.pszText = buf;

		ReturnCode = (int)::SendMessage(hWndListView, LVM_GETITEMTEXT,
			(WPARAM)nItem, (LPARAM)&item);

	} while (ReturnCode == nLen - 1); //if could not get all chars, try again

	std::basic_string<TCHAR> s(buf);

	if (buf)
		delete buf;

	return s;
} // GetItemText

  /*
  ** get the selected item's text
  */
std::basic_string<TCHAR> Clistview::GetSelected(int iColNumber)
{
	return GetItemText(m_hlistview, m_iIndex, iColNumber);
} // GetSelected

  /*
  ** get number of selected items
  */
int Clistview::Selected()
{
	return ListView_GetSelectedCount(m_hlistview);
} // Selected

  /*
  ** clear list view contents
  */
void Clistview::Clear()
{
	m_iPrevRow = -1;
	ResetIndex();

	/*
	** clear list view control
	*/
	ListView_DeleteAllItems(m_hlistview);
} // Clear

  /*
  ** clear list view columns
  */
void Clistview::ClearColumns()
{
	// clear listview contents
	Clear();

	for (int i = 0; i < m_iNumOfCols; i++)
	{
		ListView_DeleteColumn(m_hlistview, 0);
	}

	// reset number of columns
	m_iNumOfCols = 0;

	// clear listview column types
	m_vColTypes.clear();
} // ClearColumns

  /*
  ** get all list-view text
  ** size of vector is rows x columns
  */
void Clistview::GetInfo(
	std::vector<std::basic_string<TCHAR>> &vCols,			// column names
	std::vector<int> &vColWidth,		// column widths
	std::vector<enType> &vColType,	// column type
	std::vector<std::vector<std::basic_string<TCHAR>>> &vInfo		// listview text
)
{
	int iRows = ListView_GetItemCount(m_hlistview);
	int iCols = m_iNumOfCols;

	LVITEM lv = { 0 };

	vInfo.clear();

	// resize vectors
	vCols.resize(iCols);
	vColWidth.resize(iCols);

	// resize matrix
	vInfo.resize(iRows);
	for (int i = 0; i < (int)vInfo.size(); i++)
		vInfo[i].resize(iCols);

	for (int iRow = 0; iRow < iRows; iRow++)
	{
		lv.mask = LVIF_TEXT;
		lv.iItem = iRow;

		for (int iCol = 0; iCol < iCols; iCol++)
		{
			// get column information
			if (iRow == 0)
			{
				LVCOLUMN lc = { 0 };
				lc.mask = LVCF_TEXT;

				// TO-DO: eliminate magic number
				TCHAR buf[256];
				lc.pszText = buf;
				lc.cchTextMax = _countof(buf);

				ListView_GetColumn(m_hlistview, iCol, &lc);

				vCols[iCol] = buf;
				vColWidth[iCol] = ListView_GetColumnWidth(m_hlistview, iCol);
			}

			lv.iSubItem = iCol;

			// insert information into matrix
			vInfo[iRow][iCol] = GetItemText(m_hlistview, iRow, iCol);
		}
	}

	vColType = m_vColTypes;

	return;
} // GetInfo
