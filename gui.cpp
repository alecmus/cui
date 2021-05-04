/*
** gui.cpp - cui framework gui implementation
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

#include "gui.h"

#include "limit_single_instance/limit_single_instance.h"
#include "error/win_error.h"
#include "resource.h"

#include "cui_raw/cui_raw.h"	// cui_raw Framework

#include <Windows.h>
#include <tchar.h>

// common controls
#include <CommCtrl.h>
#pragma comment (lib, "Comctl32.lib")

// GDI+
#include <GdiPlus.h>
#pragma comment(lib, "GdiPlus.lib")

// COM
#include <comdef.h>

#include <atomic>
#include <vector>
#include <map>

// for visual styles
#pragma comment(linker, "\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' \
language='*'\"")

HMODULE get_this_module_handle()
{
	HMODULE handle = NULL;
	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCTSTR)get_this_module_handle,	/* use the address of the current function to detect the module handle of the current application */
		&handle);
	return handle;
} // get_this_module_handle

// conversion functions

std::basic_string<TCHAR> convert_string(const std::string &string_)
{
	return std::basic_string<TCHAR>(string_.begin(), string_.end());
}

std::string convert_string(const std::basic_string<TCHAR> &string_)
{
	return std::string(string_.begin(), string_.end());
}

liblec::cui::gui_raw::cui_raw::textAlignment
convert_text_alignment(liblec::cui::widgets::text_alignment alignment)
{
	liblec::cui::gui_raw::cui_raw::textAlignment alignment_;

	switch (alignment)
	{
	case liblec::cui::widgets::text_alignment::top_left:
		alignment_ = liblec::cui::gui_raw::cui_raw::textAlignment::topleft;
		break;

	case liblec::cui::widgets::text_alignment::center:
		alignment_ = liblec::cui::gui_raw::cui_raw::textAlignment::center;
		break;

	case liblec::cui::widgets::text_alignment::top_right:
		alignment_ = liblec::cui::gui_raw::cui_raw::textAlignment::topright;
		break;

	case liblec::cui::widgets::text_alignment::middle_left:
		alignment_ = liblec::cui::gui_raw::cui_raw::textAlignment::middleleft;
		break;

	case liblec::cui::widgets::text_alignment::middle:
		alignment_ = liblec::cui::gui_raw::cui_raw::textAlignment::middle;
		break;

	case liblec::cui::widgets::text_alignment::middle_right:
		alignment_ = liblec::cui::gui_raw::cui_raw::textAlignment::middleright;
		break;

	case liblec::cui::widgets::text_alignment::bottom_left:
		alignment_ = liblec::cui::gui_raw::cui_raw::textAlignment::bottomleft;
		break;

	case liblec::cui::widgets::text_alignment::bottom_middle:
		alignment_ = liblec::cui::gui_raw::cui_raw::textAlignment::bottommiddle;
		break;

	case liblec::cui::widgets::text_alignment::bottom_right:
		alignment_ = liblec::cui::gui_raw::cui_raw::textAlignment::bottomright;
		break;

	default:
		alignment_ = liblec::cui::gui_raw::cui_raw::textAlignment::topleft;
		break;
	}

	return alignment_;
} // convert_text_alignment

liblec::cui::gui_raw::cui_raw::onResize
convert_resize(liblec::cui::widgets::on_resize resize)
{
	liblec::cui::gui_raw::cui_raw::onResize resize_;

	resize_.iPercH = resize.perc_h;
	resize_.iPercV = resize.perc_v;
	resize_.iPercCX = resize.perc_width;
	resize_.iPercCY = resize.perc_height;

	return resize_;
} // convert_resize

RECT convert_rect(const liblec::cui::rect &rc,
	const size_t &top_margin)
{
	RECT rc_;
	rc_.left = rc.left;
	rc_.right = rc.right;
	rc_.top = rc.top + top_margin;			// this is very important
	rc_.bottom = rc.bottom + top_margin;	// this is very important

	return rc_;
} // convert_rect

SIZE convert_size(const liblec::cui::size &sz)
{
	SIZE sz_;
	sz_.cx = sz.width;
	sz_.cy = sz.height;
	return sz_;
} // convert_size

liblec::cui::gui_raw::cui_raw::imageTextPlacement
convert_image_placement(liblec::cui::widgets::image_text_placement placement)
{
	liblec::cui::gui_raw::cui_raw::imageTextPlacement placement_;

	switch (placement)
	{
	case liblec::cui::widgets::image_text_placement::bottom:
		placement_ = liblec::cui::gui_raw::cui_raw::imageTextPlacement::bottom;
		break;
	case liblec::cui::widgets::image_text_placement::top:
		placement_ = liblec::cui::gui_raw::cui_raw::imageTextPlacement::top;
		break;
	case liblec::cui::widgets::image_text_placement::right:
		placement_ = liblec::cui::gui_raw::cui_raw::imageTextPlacement::right;
		break;
	case liblec::cui::widgets::image_text_placement::right_top:
		placement_ = liblec::cui::gui_raw::cui_raw::imageTextPlacement::righttop;
		break;
	case liblec::cui::widgets::image_text_placement::right_bottom:
		placement_ = liblec::cui::gui_raw::cui_raw::imageTextPlacement::rightbottom;
		break;
	case liblec::cui::widgets::image_text_placement::left:
		placement_ = liblec::cui::gui_raw::cui_raw::imageTextPlacement::left;
		break;
	case liblec::cui::widgets::image_text_placement::left_top:
		placement_ = liblec::cui::gui_raw::cui_raw::imageTextPlacement::lefttop;
		break;
	case liblec::cui::widgets::image_text_placement::left_bottom:
		placement_ = liblec::cui::gui_raw::cui_raw::imageTextPlacement::leftbottom;
		break;
	default:
		placement_ = liblec::cui::gui_raw::cui_raw::imageTextPlacement::righttop;
		break;
	}

	return placement_;
} // convert_image_placement

liblec::cui::gui_raw::cui_raw::onToggle
convert_toggle_action(liblec::cui::widgets::on_toggle on_toggle)
{
	liblec::cui::gui_raw::cui_raw::onToggle on_toggle_;

	switch (on_toggle)
	{
	case liblec::cui::widgets::on_toggle::left:
		on_toggle_ = liblec::cui::gui_raw::cui_raw::onToggle::toggleLeft;
		break;
	case liblec::cui::widgets::on_toggle::right:
		on_toggle_ = liblec::cui::gui_raw::cui_raw::onToggle::toggleRight;
		break;
	case liblec::cui::widgets::on_toggle::up:
		on_toggle_ = liblec::cui::gui_raw::cui_raw::onToggle::toggleUp;
		break;
	case liblec::cui::widgets::on_toggle::down:
		on_toggle_ = liblec::cui::gui_raw::cui_raw::onToggle::toggleDown;
		break;
	default:
		on_toggle_ = liblec::cui::gui_raw::cui_raw::onToggle::toggleUp;
		break;
	}

	return on_toggle_;
} // convert_toggle_action

liblec::cui::gui_raw::cui_raw::imgFormat
convert_image_format(liblec::cui::image_format format)
{
	liblec::cui::gui_raw::cui_raw::imgFormat format_;

	switch (format)
	{
	case liblec::cui::image_format::png:
		format_ = liblec::cui::gui_raw::cui_raw::imgFormat::PNG;
		break;
	case liblec::cui::image_format::bmp:
		format_ = liblec::cui::gui_raw::cui_raw::imgFormat::BMP;
		break;
	case liblec::cui::image_format::jpeg:
		format_ = liblec::cui::gui_raw::cui_raw::imgFormat::JPEG;
		break;
	case liblec::cui::image_format::none:
		format_ = liblec::cui::gui_raw::cui_raw::imgFormat::NONE;
		break;
	default:
		format_ = liblec::cui::gui_raw::cui_raw::imgFormat::PNG;
		break;
	}

	return format_;
} // convert_image_format

liblec::cui::gui_raw::cui_raw::pieChartHoverEffect
convert_piechart_hover_effect(liblec::cui::widgets::piechart_hover_effect on_hover)
{
	liblec::cui::gui_raw::cui_raw::pieChartHoverEffect on_hover_;

	switch (on_hover)
	{
	case liblec::cui::widgets::piechart_hover_effect::glow:
		on_hover_ = liblec::cui::gui_raw::cui_raw::pieChartHoverEffect::glow;
		break;
	case liblec::cui::widgets::piechart_hover_effect::glow_and_arc:
		on_hover_ = liblec::cui::gui_raw::cui_raw::pieChartHoverEffect::glowAndArc;
		break;
	case liblec::cui::widgets::piechart_hover_effect::glow_and_grow:
		on_hover_ = liblec::cui::gui_raw::cui_raw::pieChartHoverEffect::glowAndGrow;
		break;
	case liblec::cui::widgets::piechart_hover_effect::glow_and_shrink_others:
		on_hover_ = liblec::cui::gui_raw::cui_raw::pieChartHoverEffect::glowAndShrinkOthers;
		break;
	default:
		on_hover_ = liblec::cui::gui_raw::cui_raw::pieChartHoverEffect::glowAndArc;
		break;
	}

	return on_hover_;
} // convert_piechart_hover_effect

std::vector<liblec::cui::gui_raw::cui_raw::listviewColumn>
convert_listview_columns(const std::vector<liblec::cui::widgets::listview_column> &columns)
{
	std::vector<liblec::cui::gui_raw::cui_raw::listviewColumn> columns_;
	columns_.reserve(columns.size());

	for (size_t i = 0; i < columns.size(); i++)
	{
		liblec::cui::gui_raw::cui_raw::listviewColumn column_;
		column_.iColumnID = i + 1;
		column_.iWidth = columns[i].width;
		column_.sColumnName = convert_string(columns[i].name);

		switch (columns[i].type)
		{
		case liblec::cui::widgets::listview_column_type::string_:
			column_.type = liblec::cui::gui_raw::cui_raw::listviewColumnType::String;
			break;
		case liblec::cui::widgets::listview_column_type::char_:
			column_.type = liblec::cui::gui_raw::cui_raw::listviewColumnType::Char;
			break;
		case liblec::cui::widgets::listview_column_type::int_:
			column_.type = liblec::cui::gui_raw::cui_raw::listviewColumnType::Int;
			break;
		case liblec::cui::widgets::listview_column_type::float_:
			column_.type = liblec::cui::gui_raw::cui_raw::listviewColumnType::Float;
			break;
		default:
			column_.type = liblec::cui::gui_raw::cui_raw::listviewColumnType::String;
			break;
		}

		column_.bBarChart = columns[i].barchart;
		column_.iBarChartMax = columns[i].barchart_max;
		column_.clrBarChart = RGB(columns[i].color_bar.red,
			columns[i].color_bar.green, columns[i].color_bar.blue);
		column_.clrText = RGB(columns[i].color_text.red,
			columns[i].color_text.green, columns[i].color_text.blue);

		columns_.push_back(column_);
	}

	return columns_;
} // convert_listview_columns

std::vector<liblec::cui::widgets::listview_column>
convert_listview_columns(const std::vector<liblec::cui::gui_raw::cui_raw::listviewColumn> &columns)
{
	std::vector<liblec::cui::widgets::listview_column> columns_;
	columns_.reserve(columns.size());

	for (size_t i = 0; i < columns.size(); i++)
	{
		liblec::cui::widgets::listview_column column_;
		column_.width = columns[i].iWidth;
		column_.name = convert_string(columns[i].sColumnName);

		switch (columns[i].type)
		{
		case liblec::cui::gui_raw::cui_raw::String:
			column_.type = liblec::cui::widgets::listview_column_type::string_;
			break;
		case liblec::cui::gui_raw::cui_raw::Char:
			column_.type = liblec::cui::widgets::listview_column_type::char_;
			break;
		case liblec::cui::gui_raw::cui_raw::Int:
			column_.type = liblec::cui::widgets::listview_column_type::int_;
			break;
		case liblec::cui::gui_raw::cui_raw::Float:
			column_.type = liblec::cui::widgets::listview_column_type::float_;
			break;
		default:
			column_.type = liblec::cui::widgets::listview_column_type::string_;
			break;
		}

		column_.barchart = columns[i].bBarChart;
		column_.barchart_max = columns[i].iBarChartMax;
		column_.color_bar.red = (unsigned short)GetRValue(columns[i].clrBarChart);
		column_.color_bar.green = (unsigned short)GetGValue(columns[i].clrBarChart);
		column_.color_bar.blue = (unsigned short)GetBValue(columns[i].clrBarChart);

		column_.color_text.red = (unsigned short)GetRValue(columns[i].clrText);
		column_.color_text.green = (unsigned short)GetGValue(columns[i].clrText);
		column_.color_text.blue = (unsigned short)GetBValue(columns[i].clrText);

		columns_.push_back(column_);
	}

	return columns_;
} // convert_listview_columns

std::vector<liblec::cui::gui_raw::cui_raw::listviewRow>
convert_listview_rows(const std::vector<liblec::cui::widgets::listview_row> &data)
{
	std::vector<liblec::cui::gui_raw::cui_raw::listviewRow> rows_;

	for (size_t i = 0; i < data.size(); i++)
	{
		liblec::cui::gui_raw::cui_raw::listviewRow row_;
		row_.vItems.reserve(data[i].items.size());

		for (size_t j = 0; j < data[i].items.size(); j++)
		{
			liblec::cui::gui_raw::cui_raw::listviewItem item_;
			item_.bCustom = data[i].items[j].custom_text_color;
			item_.clrText = RGB(data[i].items[j].color_text.red,
				data[i].items[j].color_text.green, data[i].items[j].color_text.blue);
			item_.iRowNumber = data[i].items[j].row_number;
			item_.sColumnName = convert_string(data[i].items[j].column_name);
			item_.sItemData = convert_string(data[i].items[j].item_data);

			row_.vItems.push_back(item_);
		}

		rows_.push_back(row_);
	}

	return rows_;
} // convert_listview_rows

std::vector<liblec::cui::widgets::listview_row>
convert_listview_rows(const std::vector<liblec::cui::gui_raw::cui_raw::listviewRow> &data)
{
	std::vector<liblec::cui::widgets::listview_row> rows_;

	for (size_t i = 0; i < data.size(); i++)
	{
		liblec::cui::widgets::listview_row row_;
		row_.items.reserve(data[i].vItems.size());

		for (size_t j = 0; j < data[i].vItems.size(); j++)
		{
			liblec::cui::widgets::listview_item item_;
			item_.custom_text_color = data[i].vItems[j].bCustom;

			item_.color_text.red = (unsigned short)GetRValue(data[i].vItems[j].clrText);
			item_.color_text.green = (unsigned short)GetGValue(data[i].vItems[j].clrText);
			item_.color_text.blue = (unsigned short)GetBValue(data[i].vItems[j].clrText);

			item_.row_number = data[i].vItems[j].iRowNumber;
			item_.column_name = convert_string(data[i].vItems[j].sColumnName);
			item_.item_data = convert_string(data[i].vItems[j].sItemData);

			row_.items.push_back(item_);
		}

		rows_.push_back(row_);
	}

	return rows_;
} // convert_listview_rows

void liblec::cui::tools::snap_to(const liblec::cui::rect & rect_reference,
	liblec::cui::tools::snap snap_type,
	const size_t & clearance,
	liblec::cui::rect & rect)
{
	const long width_reference = rect_reference.right - rect_reference.left;
	const long height_reference = rect_reference.bottom - rect_reference.top;

	const long width = rect.width();
	const long height = rect.height();

	switch (snap_type)
	{
	case liblec::cui::tools::snap::bottom:
		rect.top = rect_reference.bottom + clearance;
		rect.left = rect_reference.left + (width_reference - width) / 2;
		break;

	case liblec::cui::tools::snap::bottom_right:
		rect.top = rect_reference.bottom + clearance;
		rect.left = rect_reference.right - width;
		break;

	case liblec::cui::tools::snap::top_left:
		rect.top = rect_reference.top - (height + clearance);
		rect.left = rect_reference.left;
		break;

	case liblec::cui::tools::snap::top:
		rect.top = rect_reference.top - (height + clearance);
		rect.left = rect_reference.left + (width_reference - width) / 2;
		break;

	case liblec::cui::tools::snap::top_right:
		rect.top = rect_reference.top - (height + clearance);
		rect.left = rect_reference.right - width;
		break;

	case liblec::cui::tools::snap::right_top:
		rect.left = rect_reference.right + clearance;
		rect.top = rect_reference.top;
		break;

	case liblec::cui::tools::snap::right:
		rect.left = rect_reference.right + clearance;
		rect.top = rect_reference.top + (height_reference - height) / 2;
		break;

	case liblec::cui::tools::snap::right_bottom:
		rect.left = rect_reference.right + clearance;
		rect.top = rect_reference.bottom - height;
		break;

	case liblec::cui::tools::snap::left_top:
		rect.left = rect_reference.left - (width + clearance);
		rect.top = rect_reference.top;
		break;

	case liblec::cui::tools::snap::left:
		rect.left = rect_reference.left - (width + clearance);
		rect.top = rect_reference.top + (height_reference - height) / 2;
		break;

	case liblec::cui::tools::snap::left_bottom:
		rect.left = rect_reference.left - (width + clearance);
		rect.top = rect_reference.bottom - height;
		break;

	case liblec::cui::tools::snap::bottom_left:
	default:
		rect.top = rect_reference.bottom + clearance;
		rect.left = rect_reference.left;
		break;
	}

	rect.set_width(width);
	rect.set_height(height);
} // snap_to

void liblec::cui::tools::pos_rect(const liblec::cui::rect &rect_reference,
	liblec::cui::rect &rect,
	const size_t &perc_h,
	const size_t &perc_v)
{
	RECT rcInOut = convert_rect(rect, 0);
	liblec::cui::gui_raw::cui_raw::posRect(rcInOut, convert_rect(rect_reference, 0), perc_h, perc_v);
	rect.left = rcInOut.left;
	rect.top = rcInOut.top;
	rect.right = rcInOut.right;
	rect.bottom = rcInOut.bottom;
} // pos_rect

class liblec::cui::gui::gui_impl
{
public:
	gui_impl(const std::string &app_guid):
		unique_id_(1000),	// start at 1000 just to be safe
		init_id_(0),
		close_id_(0),
		shutdown_id_(0),
		limit_single_instance_(true),
		color_ui_(RGB(21, 79, 139)),
		color_ui_background_(RGB(255, 255, 255)),
		color_ui_hot_(RGB(255, 180, 0)),
		color_ui_disabled_(RGB(240, 240, 240)),
		font_ui_("Segoe UI"),
		font_size_ui_(9),
		color_tooltip_text_(RGB(0, 0, 0)),
		color_tooltip_background_(RGB(255, 255, 255)),
		color_tooltip_border_(RGB(200, 200, 200)),
		color_warning_(RGB(255, 140, 0)),
		color_ok_(RGB(0, 150, 0)),
		caption_("liblec::cui::gui window"),
		caption_tooltip_("Click to view information about this app"),
		caption_id_(0),
		resource_dll_filename_(std::string()),
		width_(780),
		height_(480),
		min_width_(500),
		min_height_(300),
		allow_resizing_(true),
		x_(0),
		y_(0),
		user_pos_(false),
		preset_pos_(false),
		window_pos_(liblec::cui::gui_raw::cui_raw::windowPosition::centerToWorkingArea),
		ico_resource_(0),
		drop_files_id_(0),
		on_drop_files_(nullptr),
		resource_module_handle_(NULL),
		stop_(false),
		p_raw_ui_(nullptr),
		p_parent_(nullptr),
		top_margin_(0)
	{
		++instances_;	// incrememt instances count

		if (!initialized_)
		{
			// initialize common controls (for visual styles)
			InitCommonControls();

			// initialize GDI+
			Gdiplus::GdiplusStartupInput gdiplus_startup_input;
			GdiplusStartup(&gdi_plus_token_, &gdiplus_startup_input, NULL);

			// initialize COM
			HRESULT hres = CoInitializeEx(0, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

			if (FAILED(hres))
			{
				// CoInitialize failed, get detailed error information
				_com_error com_error(hres);

				const size_t max = 256;
				TCHAR error[max];

				_sntprintf_s(error,
					max,
					_T("An error occured while initializing the library.\r\n\r\nDetails: %s"),
					com_error.ErrorMessage());

				// display error message
				MessageBox(NULL, error, _T("liblec::cui::gui"), MB_ICONERROR);

				// initialization has failed. Go no further.
				return;
			}

			// for limiting the number of instances of the app
			p_instance_ = new limit_single_instance(app_guid);

			// set initialized flag to true (only here)
			initialized_ = true;
		}

		// ...
	}

	~gui_impl()
	{
		// ....

		// unload fonts loaded from font files
		unload_font_files(font_files_);

		// unload fonts loaded from font resources
		unload_font_resources(font_resources_);

		if (initialized_ && instances_ == 1)
		{
			// Uninitialize COM
			CoUninitialize();

			if (gdi_plus_token_)
			{
				// shut down GDI+
				Gdiplus::GdiplusShutdown(gdi_plus_token_);
			}

			if (p_instance_)
			{
				delete p_instance_;
				p_instance_ = nullptr;
			}

			// set initialized flag to false (only here)
			initialized_ = false;
		}

		--instances_;	// decremement instances count
	}

	size_t make_unique_id()
	{
		++unique_id_;
		return unique_id_;
	}

	std::string set_font(const std::string &font)
	{
		if (font.empty())
			return font_ui_;
		else
			return font;
	}

	static BOOL CALLBACK enum_fam_callback(LPLOGFONTA lplf,
		LPNEWTEXTMETRIC lpntm,
		DWORD FontType,
		LPVOID pVoid)
	{
		UNREFERENCED_PARAMETER(lplf);
		UNREFERENCED_PARAMETER(lpntm);

		std::vector<std::string> *pvFontNames = (std::vector<std::string>*) pVoid;
		pvFontNames->push_back(lplf->lfFaceName);

		return TRUE;
	} // enum_fam_callback

	/// <summary>
	/// Check if a font is available for use.
	/// </summary>
	/// 
	/// <param name="font_name">
	/// The name of the font.
	/// </param>
	/// 
	/// <returns>
	/// Returns true if the font is available, else false.
	/// </returns>
	static bool font_available(const std::string &font_name)
	{
		auto get_font_names = [](std::vector<std::string> &font_names)
		{
			HDC hdc = GetDC(GetDesktopWindow());

			EnumFontFamiliesA(hdc, (LPCSTR)NULL,
				(FONTENUMPROCA)enum_fam_callback, (LPARAM)&font_names);

			ReleaseDC(GetDesktopWindow(), hdc);
		}; // get_font_names

		std::vector<std::string> font_names;
		get_font_names(font_names);

		for (auto &it : font_names)
		{
			if (it == font_name)
				return true;
		}

		return false;
	} // font_available

	/// <summary>
	/// Load fonts from files.
	/// </summary>
	/// 
	/// <param name="font_files">
	/// The list of files to load fonts from.
	/// </param>
	/// 
	/// <param name="error">
	/// Error information.
	/// </param>
	/// 
	/// <returns>
	/// Returns true if successful, else false.
	/// </returns>
	static bool load_font_files(std::vector<std::string> &font_files,
		std::string &error)
	{
		for (auto &it : font_files)
		{
			if (AddFontResourceExA(it.c_str(), FR_PRIVATE, NULL) == 0)
			{
				error = "Font file '" + it + "' could not be loaded";
				return false;
			}
		}

		return true;
	} // load_font_files

	static void unload_font_files(std::vector<std::string> &font_files)
	{
		for (auto &it : font_files)
			RemoveFontResourceExA(it.c_str(), FR_PRIVATE, NULL);
	}

	/// <summary>
	/// Load fonts from resource.
	/// </summary>
	/// 
	/// <param name="font_resources">
	/// List of resources to load fonts from.
	/// </param>
	/// 
	/// <param name="font_resource_handles">
	/// List of font resource handles.
	/// </param>
	/// 
	/// <param name="font_resource_type">
	/// Type of resource.
	/// </param>
	/// 
	/// <param name="resource_module_handle">
	/// Handle to resource module.
	/// </param>
	/// 
	/// <param name="error">
	/// Error information.
	/// </param>
	/// 
	/// <returns>
	/// Returns true if successful, else false.
	/// </returns>
	static bool load_font_resources(std::vector<int> &font_resources,
		std::vector<HANDLE> &font_resource_handles,
		const std::string &font_resource_type,
		HMODULE resource_module_handle,
		std::string &error)
	{
		for (auto &it : font_resources)
		{
			HRSRC res = FindResourceA(resource_module_handle, MAKEINTRESOURCEA(it), font_resource_type.c_str());

			if (res)
			{
				HGLOBAL mem = LoadResource(resource_module_handle, res);

				void* data = LockResource(mem);

				size_t len = SizeofResource(resource_module_handle, res);

				DWORD nFonts;

				HANDLE hFontResHandle = AddFontMemResourceEx(
					data,	// font resource
					len,	// number of bytes in font resource
					NULL,	// reserved, must be 0
					&nFonts	// number of fonts installed
				);

				if (hFontResHandle == NULL)
				{
					error = "Font resource '" + std::to_string(it) + "' could not be loaded";
					return false;
				}

				font_resource_handles.push_back(hFontResHandle);
			}
			else
			{
				error = get_last_error();
				
				if (error.empty())
					error = "Font resource '" + std::to_string(it) + "' could not be loaded";

				return false;
			}
		}

		return true;
	} // load_font_resources

	static void unload_font_resources(const std::vector<HANDLE> &font_resource_handles)
	{
		for (auto &it : font_resource_handles)
			RemoveFontMemResourceEx(it);
	}

	void add_text_control(const liblec::cui::widgets::text &t,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			size_t unique_id = make_unique_id();

			p_raw_ui_->addText(convert_string(page_name),
				unique_id,
				convert_string(t.text_value),
				t.on_click == nullptr,
				RGB(t.color.red, t.color.green, t.color.blue),
				RGB(t.color_hot.red, t.color_hot.green, t.color_hot.blue),
				convert_string(t.tooltip),
				convert_string(set_font(t.font)),
				t.font_size,
				convert_rect(t.rect, top_margin_),
				convert_text_alignment(t.alignment),
				convert_resize(t.on_resize),
				t.multiline);

			// register text on_click handler
			handler_[unique_id] = t.on_click;

			if (!t.alias.empty())
			{
				// add this text control to the id map
				id_map_[page_path + t.alias] = unique_id;
			}
		}
	} // add_text_control

	void add_button(const liblec::cui::widgets::button &b,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			size_t unique_id = make_unique_id();

			p_raw_ui_->addButton(convert_string(page_name),
				unique_id,
				convert_string(b.tooltip),
				convert_string(b.caption),
				convert_string(set_font(b.font)),
				b.font_size,
				convert_rect(b.rect, top_margin_),
				convert_resize(b.on_resize),
				b.is_default);

			// register button on_click handler
			handler_[unique_id] = b.on_click;

			if (!b.alias.empty())
			{
				// add this button to the id map (really not necessary though but why not?)
				id_map_[page_path + b.alias] = unique_id;
			}
		}
	} // add_button

	void add_combobox(const liblec::cui::widgets::combobox &c,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			size_t unique_id = make_unique_id();

			std::vector<std::basic_string<TCHAR>> items(c.items.size());

			for (size_t i = 0; i < c.items.size(); i++)
				items[i] = convert_string(c.items[i]);

			p_raw_ui_->addComboBox(convert_string(page_name),
				unique_id,
				items,
				convert_string(c.selected_item),
				convert_string(set_font(c.font)),
				c.font_size,
				convert_rect(c.rect, top_margin_),
				convert_resize(c.on_resize),
				c.auto_complete,
				c.read_only);

			// register combobox on_selection handler
			handler_[unique_id] = c.on_selection;

			if (!c.alias.empty())
			{
				// add this combobox to the id map
				id_map_[page_path + c.alias] = unique_id;
			}
		}
	} // add_combobox

	void add_editbox(const liblec::cui::widgets::editbox &e,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			size_t unique_id = make_unique_id();

			size_t unique_id_to_invoke = 0;
			
			if (!e.control_to_invoke_alias.empty())
			{
				try
				{
					unique_id_to_invoke = id_map_.at(e.control_to_invoke_alias);
				}
				catch (std::exception &)
				{

				}
			}

			p_raw_ui_->addEdit(convert_string(page_name),
				unique_id,
				convert_string(set_font(e.font)),
				e.font_size,
				convert_string(e.cue_banner),
				convert_rect(e.rect, top_margin_),
				convert_resize(e.on_resize),
				e.multiline,
				e.scrollbar,
				e.password,
				e.read_only,
				e.limit,
				convert_string(e.allowed_set),
				convert_string(e.forbidden_set),
				unique_id_to_invoke);

			// register editbox on_type handler
			handler_[unique_id] = e.on_type;

			if (!e.alias.empty())
			{
				// add this editbox to the id map
				id_map_[page_path + e.alias] = unique_id;
			}
		}
	} // add_editbox

	void add_icon(const liblec::cui::widgets::icon &i,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			size_t unique_id = make_unique_id();

			// prioritize a PNG resource over a file
			if (i.png_resource)
				p_raw_ui_->addImage(convert_string(page_name),
					unique_id,
					convert_string(i.tooltip),
					false,
					i.change_color,
					RGB(i.color.color.red, i.color.color.green, i.color.color.blue),
					RGB(i.color.color_hot.red, i.color.color_hot.green, i.color.color_hot.blue),
					RGB(i.color.color_border.red,
						i.color.color_border.green, i.color.color_border.blue),
					RGB(i.color.color_border_hot.red,
						i.color.color_border_hot.green, i.color.color_border_hot.blue),
					i.bar,
					RGB(i.color_bar.red, i.color_bar.green, i.color_bar.blue),
					RGB(i.color_background.red, i.color_background.green, i.color_background.blue),
					RGB(i.color_background_hot.red,
						i.color_background_hot.green, i.color_background_hot.blue),
					i.png_resource,
					convert_string(i.text),
					RGB(i.color.color_text.red,
						i.color.color_text.green, i.color.color_text.blue),
					RGB(i.color.color_text_hot.red,
						i.color.color_text_hot.green, i.color.color_text_hot.blue),
					convert_string(set_font(i.font)),
					i.font_size,
					convert_image_placement(i.text_position),
					convert_rect(i.rect, top_margin_),
					convert_resize(i.on_resize),
					i.toggle,
					convert_toggle_action(i.on_toggle),
					true,
					convert_string(i.description),
					convert_size(i.size));
			else
				p_raw_ui_->addImage(convert_string(page_name),
					unique_id,
					convert_string(i.tooltip),
					false,
					i.change_color,
					RGB(i.color.color.red, i.color.color.green, i.color.color.blue),
					RGB(i.color.color_hot.red, i.color.color_hot.green, i.color.color_hot.blue),
					RGB(i.color.color_border.red,
						i.color.color_border.green, i.color.color_border.blue),
					RGB(i.color.color_border_hot.red,
						i.color.color_border_hot.green, i.color.color_border_hot.blue),
					i.bar,
					RGB(i.color_bar.red, i.color_bar.green, i.color_bar.blue),
					RGB(i.color_background.red, i.color_background.green, i.color_background.blue),
					RGB(i.color_background_hot.red,
						i.color_background_hot.green, i.color_background_hot.blue),
					convert_string(i.filename),
					convert_string(i.text),
					RGB(i.color.color_text.red, i.color.color_text.green, i.color.color_text.blue),
					RGB(i.color.color_text_hot.red,
						i.color.color_text_hot.green, i.color.color_text_hot.blue),
					convert_string(set_font(i.font)),
					i.font_size,
					convert_image_placement(i.text_position),
					convert_rect(i.rect, top_margin_),
					convert_resize(i.on_resize),
					i.toggle,
					convert_toggle_action(i.on_toggle),
					true,
					convert_string(i.description),
					convert_size(i.size));

			// register icon on_click handler
			handler_[unique_id] = i.on_click;

			if (!i.alias.empty())
			{
				// add this icon to the id map
				id_map_[page_path + i.alias] = unique_id;
			}
		}
	} // add_icon

	void add_image(const liblec::cui::widgets::image &i,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			size_t unique_id = make_unique_id();

			// prioritize a PNG resource over a file
			if (i.png_resource)
				p_raw_ui_->addImage(convert_string(page_name),
					unique_id,
					convert_string(i.tooltip),
					i.tight_fit,
					i.change_color,
					RGB(i.color.color.red, i.color.color.green, i.color.color.blue),
					RGB(i.color.color_hot.red, i.color.color_hot.green, i.color.color_hot.blue),
					RGB(i.color.color_border.red,
						i.color.color_border.green, i.color.color_border.blue),
					RGB(i.color.color_border_hot.red,
						i.color.color_border_hot.green, i.color.color_border_hot.blue),
					i.bar,
					RGB(i.color_bar.red, i.color_bar.green, i.color_bar.blue),
					RGB(i.color_background.red, i.color_background.green, i.color_background.blue),
					RGB(i.color_background_hot.red,
						i.color_background_hot.green, i.color_background_hot.blue),
					i.png_resource,
					convert_string(i.text),
					RGB(i.color.color_text.red, i.color.color_text.green, i.color.color_text.blue),
					RGB(i.color.color_text_hot.red,
						i.color.color_text_hot.green, i.color.color_text_hot.blue),
					convert_string(set_font(i.font)),
					i.font_size,
					convert_image_placement(i.text_position),
					convert_rect(i.rect, top_margin_),
					convert_resize(i.on_resize),
					i.toggle,
					convert_toggle_action(i.on_toggle),
					false,
					convert_string(""),
					SIZE{ 0, 0 });
			else
				p_raw_ui_->addImage(convert_string(page_name),
					unique_id,
					convert_string(i.tooltip),
					false,
					i.change_color,
					RGB(i.color.color.red, i.color.color.green, i.color.color.blue),
					RGB(i.color.color_hot.red, i.color.color_hot.green, i.color.color_hot.blue),
					RGB(i.color.color_border.red,
						i.color.color_border.green, i.color.color_border.blue),
					RGB(i.color.color_border_hot.red,
						i.color.color_border_hot.green, i.color.color_border_hot.blue),
					i.bar,
					RGB(i.color_bar.red, i.color_bar.green, i.color_bar.blue),
					RGB(i.color_background.red, i.color_background.green, i.color_background.blue),
					RGB(i.color_background_hot.red,
						i.color_background_hot.green, i.color_background_hot.blue),
					convert_string(i.filename),
					convert_string(i.text),
					RGB(i.color.color_text.red, i.color.color_text.green, i.color.color_text.blue),
					RGB(i.color.color_text_hot.red,
						i.color.color_text_hot.green, i.color.color_text_hot.blue),
					convert_string(set_font(i.font)),
					i.font_size,
					convert_image_placement(i.text_position),
					convert_rect(i.rect, top_margin_),
					convert_resize(i.on_resize),
					i.toggle,
					convert_toggle_action(i.on_toggle),
					false,
					convert_string(""),
					SIZE{ 0, 0 });

			// register image on_click handler
			handler_[unique_id] = i.on_click;

			if (!i.alias.empty())
			{
				// add this image to the id map
				id_map_[page_path + i.alias] = unique_id;
			}
		}
	} // add_image

	void add_toggle_button(const liblec::cui::widgets::toggle_button &t,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			size_t unique_id = make_unique_id();

			p_raw_ui_->addToggleButton(convert_string(page_name),
				unique_id,
				convert_string(t.tooltip),
				convert_string(t.text_on),
				convert_string(t.text_off),
				convert_string(set_font(t.font)),
				t.font_size,
				RGB(t.color_text.red, t.color_text.green, t.color_text.blue),
				RGB(t.color_on.red, t.color_on.green, t.color_on.blue),
				RGB(t.color_off.red, t.color_off.green, t.color_off.blue),
				convert_rect(t.rect, top_margin_),
				convert_resize(t.on_resize),
				t.on);

			// register toggle button on_toggle handler
			handler_[unique_id] = t.on_toggle;

			if (!t.alias.empty())
			{
				// add this toggle button to the id map
				id_map_[page_path + t.alias] = unique_id;
			}
		}
	} // add_toggle_button

	void add_progress_bar(const liblec::cui::widgets::progress_bar &p,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			size_t unique_id = make_unique_id();

			p_raw_ui_->addProgressBar(convert_string(page_name),
				unique_id,
				RGB(p.color.red, p.color.green, p.color.blue),
				RGB(p.color_unfilled.red, p.color_unfilled.green, p.color_unfilled.blue),
				convert_rect(p.rect, top_margin_),
				convert_resize(p.on_resize),
				p.initial_percentage);

			if (!p.alias.empty())
			{
				// add this progress bar to the id map
				id_map_[page_path + p.alias] = unique_id;
			}
		}
	} // add_progress_bar

	void add_password_strength_bar(const liblec::cui::widgets::password_strength_bar &p,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			size_t unique_id = make_unique_id();

			p_raw_ui_->addPasswordStrengthBar(convert_string(page_name),
				unique_id,
				RGB(p.color_unfilled.red, p.color_unfilled.green, p.color_unfilled.blue),
				convert_rect(p.rect, top_margin_),
				convert_resize(p.on_resize),
				p.initial_percentage);

			if (!p.alias.empty())
			{
				// add this password strength bar to the id map
				id_map_[page_path + p.alias] = unique_id;
			}
		}
	} // add_password_strength_bar

	void add_rectangle(const liblec::cui::widgets::rectangle &r,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			size_t unique_id = make_unique_id();

			p_raw_ui_->addRect(convert_string(page_name),
				unique_id,
				RGB(r.color.red, r.color.green, r.color.blue),
				convert_rect(r.rect, top_margin_),
				convert_resize(r.on_resize));
		}
	} // add_rectangle

	void add_hairline(const liblec::cui::widgets::hairline &h,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			liblec::cui::rect rect_;
			rect_.left = h.point.x;
			rect_.top = h.point.y;

			// prioritize horizontal line
			if (h.length_horizontal)
			{
				rect_.set_height(1);
				rect_.set_width(h.length_horizontal);
			}
			else
				if (h.length_vertical)
				{
					rect_.set_width(1);
					rect_.set_height(h.length_vertical);
				}
				else
					return;

			size_t unique_id = make_unique_id();

			p_raw_ui_->addRect(convert_string(page_name),
				unique_id,
				RGB(h.color.red, h.color.green, h.color.blue),
				convert_rect(rect_, top_margin_),
				convert_resize(h.on_resize));
		}
	} // add_hairline

	void add_selector(const liblec::cui::widgets::selector &s,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			std::vector<liblec::cui::gui_raw::cui_raw::selectorItem> vItems;
			vItems.reserve(s.items.size());

			int iDefaultItem = 0;

			for (auto &it : s.items)
			{
				size_t unique_id = make_unique_id();

				if (!iDefaultItem)
					iDefaultItem = unique_id;
				else
					if (it.default_item)
						iDefaultItem = unique_id;

				liblec::cui::gui_raw::cui_raw::selectorItem item;

				item.iUniqueID = unique_id;
				item.sDescription = convert_string(it.label);
				vItems.push_back(item);

				// register selector item on_select handler
				handler_[unique_id] = it.on_select;

				if (!it.alias.empty())
				{
					// add this selector item to the id map
					id_map_[page_path + it.alias] = unique_id;
				}
			}

			size_t unique_id = make_unique_id();

			p_raw_ui_->addSelector(convert_string(page_name),
				unique_id,
				convert_string(s.tooltip),
				vItems,
				iDefaultItem,
				convert_string(set_font(s.font)),
				s.font_size,
				RGB(s.color_text.red, s.color_text.green, s.color_text.blue),
				RGB(s.color_bar.red, s.color_bar.green, s.color_bar.blue),
				convert_rect(s.rect, top_margin_),
				convert_resize(s.on_resize));

			if (!s.alias.empty())
			{
				// add this selector control to the id map
				id_map_[page_path + s.alias] = unique_id;
			}
		}
	} // add_selector

	void add_star_rating(const liblec::cui::widgets::star_rating &s,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			size_t unique_id = make_unique_id();

			int highest_rating = s.highest_rating;
			p_raw_ui_->addStarRating(convert_string(page_name),
				unique_id,
				convert_string(s.tooltip),
				RGB(s.color_border.red, s.color_border.green, s.color_border.blue),
				RGB(s.color_on.red, s.color_on.green, s.color_on.blue),
				RGB(s.color_off.red, s.color_off.green, s.color_off.blue),
				RGB(s.color_hot.red, s.color_hot.green, s.color_hot.blue),
				convert_rect(s.rect, top_margin_),
				convert_resize(s.on_resize),
				s.initial_rating,
				highest_rating,
				s.on_rating == nullptr);

			// register star rating on_rating handler
			handler_[unique_id] = s.on_rating;

			if (!s.alias.empty())
			{
				// add this star rating control to the id map
				id_map_[page_path + s.alias] = unique_id;
			}
		}
	} // add_star_rating

	void add_date(const liblec::cui::widgets::date &d,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			size_t unique_id = make_unique_id();

			p_raw_ui_->addDate(convert_string(page_name),
				unique_id,
				convert_string(set_font(d.font)),
				d.font_size,
				convert_rect(d.rect, top_margin_),
				convert_resize(d.on_resize),
				d.allow_none);

			// register date control on_date handler
			handler_[unique_id] = d.on_date;

			if (!d.alias.empty())
			{
				// add this date control to the id map
				id_map_[page_path + d.alias] = unique_id;
			}
		}
	} // add_date

	void add_time(const liblec::cui::widgets::time &t,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			size_t unique_id = make_unique_id();

			p_raw_ui_->addTime(convert_string(page_name),
				unique_id,
				convert_string(set_font(t.font)),
				t.font_size,
				convert_rect(t.rect, top_margin_),
				convert_resize(t.on_resize),
				t.allow_none);

			// register time control on_time handler
			handler_[unique_id] = t.on_time;

			if (!t.alias.empty())
			{
				// add this time control to the id map
				id_map_[page_path + t.alias] = unique_id;
			}
		}
	} // add_time

	void add_richedit(const liblec::cui::widgets::richedit &r,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			// set default icons for RichEdit paragraph formatting
			int left_align = IDP_LEFT_ALIGN_20;
			int center_align = IDP_CENTER_ALIGN_20;
			int right_align = IDP_RIGHT_ALIGN_20;
			int justify_align = IDP_JUSTIFY_20;
			int list = IDP_LIST_20;

			// capture current DPI scale
			double dpi_scale = p_raw_ui_->getDPIScale();

			// decide paragraph formatting icons based on DPI scale
			if (dpi_scale != 1)
			{
				if (dpi_scale <= 1.25)
				{
					left_align = IDP_LEFT_ALIGN_25;
					center_align = IDP_CENTER_ALIGN_25;
					right_align = IDP_RIGHT_ALIGN_25;
					justify_align = IDP_JUSTIFY_25;
					list = IDP_LIST_25;
				}
				else
					if (dpi_scale <= 1.5)
					{
						left_align = IDP_LEFT_ALIGN_30;
						center_align = IDP_CENTER_ALIGN_30;
						right_align = IDP_RIGHT_ALIGN_30;
						justify_align = IDP_JUSTIFY_30;
						list = IDP_LIST_30;
					}
					else
						if (dpi_scale <= 1.75)
						{
							left_align = IDP_LEFT_ALIGN_35;
							center_align = IDP_CENTER_ALIGN_35;
							right_align = IDP_RIGHT_ALIGN_35;
							justify_align = IDP_JUSTIFY_35;
							list = IDP_LIST_35;
						}
						else
						{
							left_align = IDP_LEFT_ALIGN_40;
							center_align = IDP_CENTER_ALIGN_40;
							right_align = IDP_RIGHT_ALIGN_40;
							justify_align = IDP_JUSTIFY_40;
							list = IDP_LIST_40;
						}
			}

			size_t unique_id = make_unique_id();

			HMODULE resource_module = get_this_module_handle();

			p_raw_ui_->addRichEdit(convert_string(page_name),
				unique_id,
				convert_string(set_font(r.font_ui_)),
				r.font_size_ui,
				convert_string(r.font),
				r.font_size,
				convert_rect(r.rect, top_margin_),
				convert_resize(r.on_resize),
				r.border,
				RGB(r.color_border.red, r.color_border.green, r.color_border.blue),
				r.read_only,
				IDP_BOLD,
				IDP_ITALIC,
				IDP_UNDERLINE,
				IDP_STRIKETHROUGH,
				IDP_SUBSCRIPT,
				IDP_SUPERSCRIPT,
				IDP_LARGER,
				IDP_SMALLER,
				IDP_FONTCOLOR,
				left_align,
				center_align,
				right_align,
				justify_align,
				list,
				resource_module);

			if (!r.alias.empty())
			{
				// add this rich edit control to the id map
				id_map_[page_path + r.alias] = unique_id;
			}
		}
	} // add_richedit

	void add_barchart(const liblec::cui::widgets::barchart &b,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			std::vector<liblec::cui::gui_raw::cui_raw::barChartData> vValues;
			vValues.reserve(b.data.bars.size());

			for (size_t i = 0; i < b.data.bars.size(); i++)
			{
				liblec::cui::gui_raw::cui_raw::barChartData data;
				data.iNumber = i + 1;
				data.sLabel = convert_string(b.data.bars[i].label);
				data.clrBar = RGB(b.data.bars[i].color.red,
					b.data.bars[i].color.green, b.data.bars[i].color.blue);
				data.dValue = b.data.bars[i].value;

				vValues.push_back(data);
			}

			size_t unique_id = make_unique_id();

			p_raw_ui_->addBarChart(convert_string(page_name),
				unique_id,
				convert_string(set_font(b.font)),
				b.font_size,
				convert_rect(b.rect, top_margin_),
				convert_resize(b.on_resize),
				RGB(0, 0, 0),
				convert_string(b.data.caption),
				convert_string(b.data.x_label),
				convert_string(b.data.y_label),
				b.data.lower_limit,
				b.data.upper_limit,
				b.data.autoscale,
				vValues,
				b.data.autocolor);

			if (!b.alias.empty())
			{
				// add this bar chart to the id map
				id_map_[page_path + b.alias] = unique_id;
			}
		}
	} // add_barchart

	void add_linechart(const liblec::cui::widgets::linechart &l,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			std::vector<liblec::cui::gui_raw::cui_raw::lineInfo> vLines;
			vLines.reserve(l.data.lines.size());

			for (size_t i = 0; i < l.data.lines.size(); i++)
			{
				liblec::cui::gui_raw::cui_raw::lineInfo line;
				line.sSeriesName = convert_string(l.data.lines[i].series_name);
				line.clrLine = RGB(l.data.lines[i].color.red,
					l.data.lines[i].color.green, l.data.lines[i].color.blue);

				line.vValues.reserve(l.data.lines[i].points.size());

				for (size_t j = 0; j < l.data.lines[i].points.size(); j++)
				{
					liblec::cui::gui_raw::cui_raw::barChartData data;
					data.iNumber = j + 1;
					data.sLabel = convert_string(l.data.lines[i].points[j].label);
					data.clrBar = RGB(l.data.lines[i].points[j].color.red,
						l.data.lines[i].points[j].color.green, l.data.lines[i].points[j].color.blue);
					data.dValue = l.data.lines[i].points[j].value;

					line.vValues.push_back(data);
				}

				vLines.push_back(line);
			}

			size_t unique_id = make_unique_id();

			p_raw_ui_->addLineChart(convert_string(page_name),
				unique_id,
				convert_string(set_font(l.font)),
				l.font_size,
				convert_rect(l.rect, top_margin_),
				convert_resize(l.on_resize),
				convert_string(l.data.caption),
				convert_string(l.data.x_label),
				convert_string(l.data.y_label),
				l.data.lower_limit,
				l.data.upper_limit,
				l.data.autoscale,
				vLines,
				l.data.autocolor);

			if (!l.alias.empty())
			{
				// add this line chart to the id map
				id_map_[page_path + l.alias] = unique_id;
			}
		}
	} // add_linechart

	void add_piechart(const liblec::cui::widgets::piechart &p,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			std::vector<liblec::cui::gui_raw::cui_raw::pieChartData> vValues;
			vValues.reserve(p.data.slices.size());

			for (size_t i = 0; i < p.data.slices.size(); i++)
			{
				liblec::cui::gui_raw::cui_raw::pieChartData data;
				data.iNumber = i + 1;
				data.sItemLabel = convert_string(p.data.slices[i].label);
				data.clrItem = RGB(p.data.slices[i].color.red,
					p.data.slices[i].color.green, p.data.slices[i].color.blue);
				data.dValue = p.data.slices[i].value;

				vValues.push_back(data);
			}

			size_t unique_id = make_unique_id();

			p_raw_ui_->addPieChart(convert_string(page_name),
				unique_id,
				convert_string(set_font(p.font)),
				p.font_size,
				convert_rect(p.rect, top_margin_),
				convert_resize(p.on_resize),
				p.data.autocolor,
				convert_string(p.data.caption),
				vValues,
				convert_piechart_hover_effect(p.data.on_hover),
				p.data.doughnut);

			if (!p.alias.empty())
			{
				// add this pie chart to the id map
				id_map_[page_path + p.alias] = unique_id;
			}
		}
	} // add_piechart

	void add_listview(const liblec::cui::widgets::listview &l,
		const std::string &page_name,
		const std::string &page_path)
	{
		if (p_raw_ui_)
		{
			size_t unique_id = make_unique_id();

			auto convert_context_menu = [&](const std::vector<liblec::cui::widgets::context_menu_item> &context_menu)
			{
				std::vector<liblec::cui::gui_raw::cui_raw::contextMenuItem> context_menu_;
				context_menu_.reserve(context_menu.size());

				for (size_t i = 0; i < context_menu.size(); i++)
				{
					liblec::cui::gui_raw::cui_raw::contextMenuItem item_;
					item_.bDefault = context_menu[i].is_default;
					item_.IDC_PNGICON = context_menu[i].png_resource;
					item_.sLabel = convert_string(context_menu[i].label);

					size_t unique_id = make_unique_id();

					item_.iUniqueID = unique_id;

					context_menu_.push_back(item_);
					
					// register context menu item on_click handler
					handler_[unique_id] = context_menu[i].on_click;
				}

				return context_menu_;
			};

			p_raw_ui_->addListview(convert_string(page_name),
				unique_id,
				convert_listview_columns(l.columns),
				convert_string(l.unique_column_name),
				convert_listview_rows(l.data),
				convert_context_menu(l.context_menu),
				convert_string(set_font(l.font)),
				l.font_size,
				convert_rect(l.rect, top_margin_),
				convert_resize(l.on_resize),
				l.border,
				l.gridlines,
				l.sort_by_clicking_column);

			if (!l.alias.empty())
			{
				// add this listview to the id map
				id_map_[page_path + l.alias] = unique_id;
			}

			// register listview on_selection handler
			handler_[unique_id] = l.on_selection;
		}
	} // add_listview

	// the command procedure
	static void command_procedure(liblec::cui::gui_raw::cui_raw &raw_ui,
		int unique_id,
		void *p_data)
	{
		liblec::cui::gui *p_ui = NULL;
		void* p_void = raw_ui.getState();
		if (p_void)
		{
			// retrieve pointer to this liblec::cui::gui object
			p_ui = reinterpret_cast<liblec::cui::gui*>(p_void);
		}

		if (p_ui)
		{
			if (unique_id == p_ui->d_->init_id_)
			{
				// initializing ... first visit to command procedure

				// exclude caption from title bar
				raw_ui.excludeFromTitleBar(_T(""), p_ui->d_->caption_id_);
			}
			else
				if (unique_id == p_ui->d_->shutdown_id_)
				{
				}
				else
					if (unique_id == p_ui->d_->drop_files_id_)
					{
						liblec::cui::gui_raw::cui_raw::dropFileMsg *p_dropfile_data =
							(liblec::cui::gui_raw::cui_raw::dropFileMsg*)p_data;

						if (p_dropfile_data)
						{
							std::basic_string<TCHAR> fullpath_ = p_dropfile_data->sFullPath;

							// call the on_drop_files handler
							if (p_ui->d_->on_drop_files_)
								p_ui->d_->on_drop_files_(convert_string(fullpath_));
						}
					}

			// check if this control has a handler
			try
			{
				// call the handler. This is where all the magic happens :)
				p_ui->d_->handler_.at(unique_id)();
			}
			catch (std::exception &e)
			{
				// handler does not exist
				std::string error = e.what();
			}
		}

	} // command_procedure

private:
	// static members
	static std::atomic<bool> initialized_;
	static std::atomic<size_t> instances_;
	static ULONG_PTR gdi_plus_token_;
	static limit_single_instance* p_instance_;
	std::atomic<int> unique_id_;
	int init_id_, close_id_, shutdown_id_;

	bool limit_single_instance_;

	// ui colors
	COLORREF color_ui_;
	COLORREF color_ui_background_;
	COLORREF color_ui_hot_;
	COLORREF color_ui_disabled_;
	COLORREF color_tooltip_text_;
	COLORREF color_tooltip_background_;
	COLORREF color_tooltip_border_;
	COLORREF color_warning_;
	COLORREF color_ok_;

	// fonts
	std::string font_ui_;
	int font_size_ui_;

	// caption (also the name of the home page)
	std::string caption_;
	std::string caption_tooltip_;
	int caption_id_;

	// name of dll containing resources like PNGs etc
	std::string resource_dll_filename_;

	// window dimensions
	int width_, height_, min_width_, min_height_;

	bool allow_resizing_;

	// window position
	int x_, y_;

	bool user_pos_;
	bool preset_pos_;
	liblec::cui::gui_raw::cui_raw::windowPosition window_pos_;

	size_t ico_resource_;
	liblec::cui::gui::caption_icon_png png_resource_;

	size_t drop_files_id_;
	std::function<void(const std::string &fullpath)> on_drop_files_;

	HMODULE resource_module_handle_;
	std::vector<std::string> font_files_;
	std::vector<HANDLE> font_resources_;

	bool stop_;

	std::map<int, std::function<void()>> handler_;	// map to store handlers <unique_id, handler>

	liblec::cui::gui_raw::cui_raw* p_raw_ui_;			// pointer to the raw gui object
	liblec::cui::gui_raw::cui_raw* p_parent_;

	std::map<std::string, int> id_map_;				// <page_path, unique_id>

	size_t top_margin_;								// margins

	friend gui;	// so liblec::cui::gui can access the private members of this class
}; // gui_impl

// initialize static variables
std::atomic<bool> liblec::cui::gui::gui_impl::initialized_ = false;
std::atomic<size_t> liblec::cui::gui::gui_impl::instances_ = 0;
ULONG_PTR liblec::cui::gui::gui_impl::gdi_plus_token_ = 0;
limit_single_instance* liblec::cui::gui::gui_impl::p_instance_ = nullptr;

liblec::cui::gui::gui(const std::string &app_guid)
{
	d_ = new gui_impl(app_guid);

	// exclude title bar by default because cui_raw actually allows us to draw in the title bar area
	// with ease. The user of this class shouldn't have to worry about remembering to skip the
	// title bar area each time they are adding a control
	d_->top_margin_ = title_bar_height();
} // gui

liblec::cui::gui::gui() :
	liblec::cui::gui::gui(std::string()) {}

liblec::cui::gui::~gui()
{
	if (d_)
	{
		delete d_;
		d_ = nullptr;
	}
} // ~gui

void liblec::cui::gui::modal(gui& parent) {
	d_->p_parent_ = parent.d_->p_raw_ui_;
}

void liblec::cui::gui::modal(liblec::cui::gui_raw::cui_raw& parent) {
	d_->p_parent_ = &parent;
}

liblec::cui::gui_raw::cui_raw& liblec::cui::gui::get_raw()
{
	return *d_->p_raw_ui_;
}

class liblec::cui::gui::page::page_impl
{
public:
	page_impl(const std::string &page_name) :
		page_name_(page_name) {}
	~page_impl() {}

	std::string page_name_;

	struct text_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::text text;
	};

	struct button_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::button button;
	};

	struct combobox_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::combobox combobox;
	};

	struct editbox_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::editbox editbox;
	};

	struct icon_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::icon icon;
	};

	struct image_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::image image;
	};

	struct toggle_button_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::toggle_button toggle_button;
	};

	struct progress_bar_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::progress_bar progress_bar;
	};

	struct password_strength_bar_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::password_strength_bar password_strength_bar;
	};

	struct rectangle_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::rectangle rectangle;
	};

	struct hairline_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::hairline hairline;
	};

	struct selector_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::selector selector;
	};

	struct star_rating_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::star_rating star_rating;
	};

	struct date_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::date date;
	};

	struct time_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::time time;
	};

	struct richedit_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::richedit richedit;
	};

	struct barchart_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::barchart barchart;
	};

	struct linechart_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::linechart linechart;
	};

	struct piechart_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::piechart piechart;
	};

	struct listview_
	{
		bool in_tab = false;
		liblec::cui::widgets::tab tab;
		liblec::cui::widgets::listview listview;
	};

	std::vector<liblec::cui::gui::page::page_impl::text_> text_controls_;
	std::vector<liblec::cui::gui::page::page_impl::button_> buttons_;
	std::vector<liblec::cui::gui::page::page_impl::combobox_> comboboxes_;
	std::vector<liblec::cui::gui::page::page_impl::editbox_> editboxes_;
	std::vector<liblec::cui::gui::page::page_impl::icon_> icons_;
	std::vector<liblec::cui::gui::page::page_impl::image_> images_;
	std::vector<liblec::cui::gui::page::page_impl::toggle_button_> toggle_buttons_;
	std::vector<liblec::cui::gui::page::page_impl::progress_bar_> progress_bars_;
	std::vector<liblec::cui::gui::page::page_impl::password_strength_bar_> password_strength_bars_;
	std::vector<liblec::cui::gui::page::page_impl::rectangle_> rectangles_;
	std::vector<liblec::cui::gui::page::page_impl::hairline_> hairlines_;
	std::vector<liblec::cui::gui::page::page_impl::selector_> selectors_;
	std::vector<liblec::cui::gui::page::page_impl::star_rating_> star_ratings_;
	std::vector<liblec::cui::gui::page::page_impl::date_> dates_;
	std::vector<liblec::cui::gui::page::page_impl::time_> times_;
	std::vector<liblec::cui::gui::page::page_impl::richedit_> richedits_;
	std::vector<liblec::cui::gui::page::page_impl::barchart_> barcharts_;
	std::vector<liblec::cui::gui::page::page_impl::linechart_> linecharts_;
	std::vector<liblec::cui::gui::page::page_impl::piechart_> piecharts_;
	std::vector<liblec::cui::gui::page::page_impl::listview_> listviews_;

	bool has_tab_control = false;
	liblec::cui::widgets::tab_control tab_control;
	liblec::cui::widgets::tab current_tab;
	std::vector< liblec::cui::widgets::tab> tabs;
};

liblec::cui::gui::page::page(const std::string &page_name)
{
	d_ = new  liblec::cui::gui::page::page_impl(page_name);
}

liblec::cui::gui::page::page() :
	liblec::cui::gui::page(std::string())
{
}

liblec::cui::gui::page::~page()
{
	if (d_)
	{
		delete d_;
		d_ = nullptr;
	}
}

void liblec::cui::gui::page::set_name(const std::string & page_name)
{
	if (d_)
		d_->page_name_ = page_name;
}

void liblec::cui::gui::page::add_text(const liblec::cui::widgets::text &t)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::text_ t_;
		t_.in_tab = d_->has_tab_control;
		t_.tab = d_->current_tab;
		t_.text = t;

		d_->text_controls_.push_back(t_);
	}
}

void liblec::cui::gui::page::add_button(const liblec::cui::widgets::button &b)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::button_ b_;
		b_.in_tab = d_->has_tab_control;
		b_.tab = d_->current_tab;
		b_.button = b;

		d_->buttons_.push_back(b_);
	}
}

void liblec::cui::gui::page::add_combobox(const liblec::cui::widgets::combobox &c)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::combobox_ c_;
		c_.in_tab = d_->has_tab_control;
		c_.tab = d_->current_tab;
		c_.combobox = c;

		d_->comboboxes_.push_back(c_);
	}
}

void liblec::cui::gui::page::add_editbox(const liblec::cui::widgets::editbox &e)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::editbox_ e_;
		e_.in_tab = d_->has_tab_control;
		e_.tab = d_->current_tab;
		e_.editbox = e;

		d_->editboxes_.push_back(e_);
	}
}

void liblec::cui::gui::page::add_icon(const liblec::cui::widgets::icon &i)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::icon_ i_;
		i_.in_tab = d_->has_tab_control;
		i_.tab = d_->current_tab;
		i_.icon = i;

		d_->icons_.push_back(i_);
	}
}

void liblec::cui::gui::page::add_image(const liblec::cui::widgets::image &i)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::image_ i_;
		i_.in_tab = d_->has_tab_control;
		i_.tab = d_->current_tab;
		i_.image = i;

		d_->images_.push_back(i_);
	}
}

void liblec::cui::gui::page::add_toggle_button(const liblec::cui::widgets::toggle_button &t)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::toggle_button_ t_;
		t_.in_tab = d_->has_tab_control;
		t_.tab = d_->current_tab;
		t_.toggle_button = t;

		d_->toggle_buttons_.push_back(t_);
	}
}

void liblec::cui::gui::page::add_progress_bar(const liblec::cui::widgets::progress_bar &p)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::progress_bar_ p_;
		p_.in_tab = d_->has_tab_control;
		p_.tab = d_->current_tab;
		p_.progress_bar = p;

		d_->progress_bars_.push_back(p_);
	}
}

void liblec::cui::gui::page::add_password_strength_bar(
	const liblec::cui::widgets::password_strength_bar &p)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::password_strength_bar_ p_;
		p_.in_tab = d_->has_tab_control;
		p_.tab = d_->current_tab;
		p_.password_strength_bar = p;

		d_->password_strength_bars_.push_back(p_);
	}
}

void liblec::cui::gui::page::add_rectangle(const liblec::cui::widgets::rectangle &r)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::rectangle_ r_;
		r_.in_tab = d_->has_tab_control;
		r_.tab = d_->current_tab;
		r_.rectangle = r;

		d_->rectangles_.push_back(r_);
	}
}

void liblec::cui::gui::page::add_hairline(const liblec::cui::widgets::hairline &h)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::hairline_ h_;
		h_.in_tab = d_->has_tab_control;
		h_.tab = d_->current_tab;
		h_.hairline = h;

		d_->hairlines_.push_back(h_);
	}
}

void liblec::cui::gui::page::add_groupbox(const liblec::cui::widgets::groupbox &g)
{
	if (d_ && !g.rects.empty())
	{
		long left = g.rects[0].left;
		long right = g.rects[0].right;
		long top = g.rects[0].top;
		long bottom = g.rects[0].bottom;

		for (auto &it : g.rects)
		{
			if (it.left < left)
				left = it.left;

			if (it.right > right)
				right = it.right;

			if (it.top < top)
				top = it.top;

			if (it.bottom > bottom)
				bottom = it.bottom;
		}

		{
			liblec::cui::widgets::hairline hairline_left;
			hairline_left.color = g.color;
			hairline_left.point.x = left - g.clearance_;
			hairline_left.point.y = top - g.clearance_;
			hairline_left.length_vertical = bottom - top + 2 * g.clearance_;

			hairline_left.on_resize.perc_h = g.on_resize.perc_h;
			hairline_left.on_resize.perc_v = g.on_resize.perc_v;
			hairline_left.on_resize.perc_height = g.on_resize.perc_height;

			liblec::cui::gui::page::page_impl::hairline_ h_;
			h_.in_tab = d_->has_tab_control;
			h_.tab = d_->current_tab;
			h_.hairline = hairline_left;

			d_->hairlines_.push_back(h_);
		}

		{
			liblec::cui::widgets::hairline hairline_right;
			hairline_right.color = g.color;
			hairline_right.point.x = right + g.clearance_;
			hairline_right.point.y = top - g.clearance_;
			hairline_right.length_vertical = bottom - top + 2 * g.clearance_;

			hairline_right.on_resize.perc_h = g.on_resize.perc_h + g.on_resize.perc_width;
			hairline_right.on_resize.perc_v = g.on_resize.perc_v;
			hairline_right.on_resize.perc_height = g.on_resize.perc_height;

			liblec::cui::gui::page::page_impl::hairline_ h_;
			h_.in_tab = d_->has_tab_control;
			h_.tab = d_->current_tab;
			h_.hairline = hairline_right;

			d_->hairlines_.push_back(h_);
		}

		{
			liblec::cui::widgets::hairline hairline_top;
			hairline_top.color = g.color;
			hairline_top.point.x = left - g.clearance_;
			hairline_top.point.y = top - g.clearance_;
			hairline_top.length_horizontal = right - left + 2 * g.clearance_;

			hairline_top.on_resize.perc_h = g.on_resize.perc_h;
			hairline_top.on_resize.perc_v = g.on_resize.perc_v;
			hairline_top.on_resize.perc_width = g.on_resize.perc_width;

			liblec::cui::gui::page::page_impl::hairline_ h_;
			h_.in_tab = d_->has_tab_control;
			h_.tab = d_->current_tab;
			h_.hairline = hairline_top;

			d_->hairlines_.push_back(h_);
		}

		{
			liblec::cui::widgets::hairline hairline_bottom;
			hairline_bottom.color = g.color;
			hairline_bottom.point.x = left - g.clearance_;
			hairline_bottom.point.y = bottom + g.clearance_;
			hairline_bottom.length_horizontal = right - left + 2 * g.clearance_;

			hairline_bottom.on_resize.perc_h = g.on_resize.perc_h;
			hairline_bottom.on_resize.perc_v = g.on_resize.perc_v + g.on_resize.perc_height;
			hairline_bottom.on_resize.perc_width = g.on_resize.perc_width;

			liblec::cui::gui::page::page_impl::hairline_ h_;
			h_.in_tab = d_->has_tab_control;
			h_.tab = d_->current_tab;
			h_.hairline = hairline_bottom;

			d_->hairlines_.push_back(h_);
		}
	}
} // add_groupbox

void liblec::cui::gui::page::add_selector(const liblec::cui::widgets::selector &s)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::selector_ s_;
		s_.in_tab = d_->has_tab_control;
		s_.tab = d_->current_tab;
		s_.selector = s;

		d_->selectors_.push_back(s_);
	}
}

void liblec::cui::gui::page::add_star_rating(const liblec::cui::widgets::star_rating &s)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::star_rating_ s_;
		s_.in_tab = d_->has_tab_control;
		s_.tab = d_->current_tab;
		s_.star_rating = s;

		d_->star_ratings_.push_back(s_);
	}
}

void liblec::cui::gui::page::add_date(const liblec::cui::widgets::date &d)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::date_ date_;
		date_.in_tab = d_->has_tab_control;
		date_.tab = d_->current_tab;
		date_.date = d;

		d_->dates_.push_back(date_);
	}
}

void liblec::cui::gui::page::add_time(const liblec::cui::widgets::time &t)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::time_ t_;
		t_.in_tab = d_->has_tab_control;
		t_.tab = d_->current_tab;
		t_.time = t;

		d_->times_.push_back(t_);
	}
}

void liblec::cui::gui::page::add_richedit(const liblec::cui::widgets::richedit &r)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::richedit_ r_;
		r_.in_tab = d_->has_tab_control;
		r_.tab = d_->current_tab;
		r_.richedit = r;

		d_->richedits_.push_back(r_);
	}
}

void liblec::cui::gui::page::add_barchart(const liblec::cui::widgets::barchart &b)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::barchart_ b_;
		b_.in_tab = d_->has_tab_control;
		b_.tab = d_->current_tab;
		b_.barchart = b;

		d_->barcharts_.push_back(b_);
	}
}

void liblec::cui::gui::page::add_linechart(const liblec::cui::widgets::linechart &l)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::linechart_ l_;
		l_.in_tab = d_->has_tab_control;
		l_.tab = d_->current_tab;
		l_.linechart = l;

		d_->linecharts_.push_back(l_);
	}
}

void liblec::cui::gui::page::add_piechart(const liblec::cui::widgets::piechart &p)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::piechart_ p_;
		p_.in_tab = d_->has_tab_control;
		p_.tab = d_->current_tab;
		p_.piechart = p;

		d_->piecharts_.push_back(p_);
	}
}

void liblec::cui::gui::page::add_listview(const liblec::cui::widgets::listview &l)
{
	if (d_)
	{
		liblec::cui::gui::page::page_impl::listview_ l_;
		l_.in_tab = d_->has_tab_control;
		l_.tab = d_->current_tab;
		l_.listview = l;

		d_->listviews_.push_back(l_);
	}
}

void liblec::cui::gui::page::add_tabcontrol(const liblec::cui::widgets::tab_control &t)
{
	if (d_)
	{
		if (!d_->has_tab_control)
		{
			d_->tab_control = t;
			d_->has_tab_control = true;
		}
	}
}

void liblec::cui::gui::page::add_tab(const liblec::cui::widgets::tab &t)
{
	if (d_)
	{
		d_->tabs.push_back(t);
		d_->current_tab = t;
	}
}

bool liblec::cui::gui::page_exists(const std::string & page_name)
{
	if (d_->p_raw_ui_)
		return d_->p_raw_ui_->checkPage(convert_string(page_name));
	else
		return false;
}

void liblec::cui::gui::add_page(const page &page_to_add)
{
	bool add_controls = false;

	if (page_to_add.d_->page_name_ == d_->caption_)
	{
		// this is the home page ... do not call cui_raw::addPage()
		add_controls = true;
	}
	else
	{
		if (d_->p_raw_ui_)
			add_controls = d_->p_raw_ui_->addPage(convert_string(page_to_add.d_->page_name_),
				d_->command_procedure);	// use this same same command procedure for every page!
	}

	if (add_controls)
	{
		// add controls to the page

		// make page path
		std::string page_path = page_to_add.d_->page_name_ + "/";

		auto do_add_controls = [&](const std::string &tab_name)
		{
			// add text controls
			for (auto &it : page_to_add.d_->text_controls_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add text control using gui_impl::add_text_control
				d_->add_text_control(it.text,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add buttons
			for (auto &it : page_to_add.d_->buttons_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add button control using gui_impl::add_button
				d_->add_button(it.button,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add comboboxes
			for (auto &it : page_to_add.d_->comboboxes_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add comboboxes using gui_impl::add_combobox
				d_->add_combobox(it.combobox,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add editboxes
			for (auto &it : page_to_add.d_->editboxes_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add editboxes using gui_impl::add_editbox
				d_->add_editbox(it.editbox,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add icons
			for (auto &it : page_to_add.d_->icons_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add icons using gui_impl::add_icon
				d_->add_icon(it.icon,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add images
			for (auto &it : page_to_add.d_->images_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add images using gui_impl::add_image
				d_->add_image(it.image,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add toggle buttons
			for (auto &it : page_to_add.d_->toggle_buttons_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add toggle buttons using gui_impl::add_toggle_button
				d_->add_toggle_button(it.toggle_button,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add progress bars
			for (auto &it : page_to_add.d_->progress_bars_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add progress bars using gui_impl::add_progress_bar
				d_->add_progress_bar(it.progress_bar,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add password strength bars
			for (auto &it : page_to_add.d_->password_strength_bars_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add progress bars using gui_impl::add_password_strength_bar
				d_->add_password_strength_bar(it.password_strength_bar,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add rectangles
			for (auto &it : page_to_add.d_->rectangles_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add rectangles using gui_impl::add_rectangle
				d_->add_rectangle(it.rectangle,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add hairlines
			for (auto &it : page_to_add.d_->hairlines_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add hairlines using gui_impl::add_hairline
				d_->add_hairline(it.hairline,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add selector controls
			for (auto &it : page_to_add.d_->selectors_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add selectors using gui_impl::add_selector
				d_->add_selector(it.selector,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add star rating controls
			for (auto &it : page_to_add.d_->star_ratings_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add star rating controls using gui_impl::add_star_rating
				d_->add_star_rating(it.star_rating,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add date controls
			for (auto &it : page_to_add.d_->dates_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add date controls using gui_impl::add_date
				d_->add_date(it.date,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add time controls
			for (auto &it : page_to_add.d_->times_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add time controls using gui_impl::add_time
				d_->add_time(it.time,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add rich edit controls
			for (auto &it : page_to_add.d_->richedits_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add rich edit controls using gui_impl::add_richedit
				d_->add_richedit(it.richedit,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add bar charts
			for (auto &it : page_to_add.d_->barcharts_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add bar chart using gui_impl::add_barchart
				d_->add_barchart(it.barchart,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add line charts
			for (auto &it : page_to_add.d_->linecharts_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add line chart using gui_impl::add_linechart
				d_->add_linechart(it.linechart,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add pie charts
			for (auto &it : page_to_add.d_->piecharts_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add pie chart using gui_impl::add_piechart
				d_->add_piechart(it.piechart,
					page_to_add.d_->page_name_,
					page_path);
			}

			// add list views
			for (auto &it : page_to_add.d_->listviews_)
			{
				if (it.tab.tab_name != tab_name)
					continue;

				// add list views using gui_impl::add_listview
				d_->add_listview(it.listview,
					page_to_add.d_->page_name_,
					page_path);
			}

			//... add other controls
		}; // do_add_controls

		// add normal controls
		do_add_controls("");

		if (page_to_add.d_->has_tab_control)
		{
			if (!page_to_add.d_->page_name_.empty())
			{
				// add tab control
				size_t unique_id = d_->make_unique_id();

				d_->p_raw_ui_->addTabControl(convert_string(page_to_add.d_->page_name_),
					unique_id,
					convert_rect(page_to_add.d_->tab_control.rect, d_->top_margin_),
					convert_string(d_->set_font(page_to_add.d_->tab_control.font)),
					page_to_add.d_->tab_control.font_size,
					RGB(page_to_add.d_->tab_control.color_tab_line.red,
						page_to_add.d_->tab_control.color_tab_line.green,
						page_to_add.d_->tab_control.color_tab_line.blue));

				if (!page_to_add.d_->tab_control.alias.empty())
				{
					// add this tab control to the id map
					d_->id_map_[page_path + page_to_add.d_->tab_control.alias] = unique_id;
				}

				std::string visible_tab;

				for (auto &it : page_to_add.d_->tabs)
				{
					if (visible_tab.empty() | it.default_tab)
						visible_tab = it.tab_name;

					// add tab
					d_->p_raw_ui_->addTab(convert_string(page_to_add.d_->page_name_),
						convert_string(it.tab_name),
						convert_string(it.tooltip));

					// add controls within this tab
					do_add_controls(it.tab_name);
				}

				// set visible tab
				d_->p_raw_ui_->showTab(convert_string(page_to_add.d_->page_name_),
					convert_string(visible_tab));
			}
		}

		// create the page (if this is neither the homepage nor the persistent page)
		if (page_to_add.d_->page_name_ != d_->caption_ && !page_to_add.d_->page_name_.empty())
		{
			if (d_->p_raw_ui_)
				d_->p_raw_ui_->createPage(convert_string(page_to_add.d_->page_name_));
		}
	}
} // add_page

void liblec::cui::gui::show_page(const std::string &page_name)
{
	if (d_->p_raw_ui_)
			d_->p_raw_ui_->showPage(convert_string(page_name), false);
}

std::string liblec::cui::gui::current_page()
{
	if (d_->p_raw_ui_)
		return convert_string(d_->p_raw_ui_->currentPage());
	else
		return std::string();
}

std::string liblec::cui::gui::previous_page()
{
	if (d_->p_raw_ui_)
		return convert_string(d_->p_raw_ui_->previousPage());
	else
		return std::string();
}

void liblec::cui::gui::show_previous_page()
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->showPage(d_->p_raw_ui_->previousPage(), false);
}

bool liblec::cui::gui::run(std::string &error)
{
	return run(std::string(), error);
} // run

bool liblec::cui::gui::run(const std::string &window_guid,
	std::string &error)
{
	if (d_ && !d_->initialized_)
	{
		error = "Library initialization error: liblec::cui::gui::run";
		return false;
	}

	if (d_->limit_single_instance_ && d_->p_instance_->another_instance_running())
	{
		if (!window_guid.empty())
		{
			// create temporary cui_raw object
			liblec::cui::gui_raw::cui_raw* p_raw_ui = new liblec::cui::gui_raw::cui_raw(
				_T("cui default window"),
				d_->command_procedure, d_->color_ui_background_, d_->color_ui_,
				d_->color_ui_hot_, d_->color_ui_disabled_, convert_string(d_->font_ui_),
				8, d_->color_tooltip_text_, d_->color_tooltip_background_,
				d_->color_tooltip_border_,
				GetModuleHandle(NULL),
				NULL,
				(void*)this);	// can do without passing this pointer

			if (p_raw_ui)
			{
				// try to open existing instance
				if (p_raw_ui->openExistingInstance(convert_string(window_guid)))
				{
					// existing instance opened successfully
				}

				delete p_raw_ui;
				p_raw_ui = nullptr;
			}
		}

		return true;	// close this instance
	}

	// get resource module handle
	if (!d_->resource_dll_filename_.empty())
	{
		d_->resource_module_handle_ = LoadLibrary(convert_string(d_->resource_dll_filename_).c_str());

		if (d_->resource_module_handle_ == NULL)
		{
			// LoadLibrary failed
			error = "Loading " + d_->resource_dll_filename_ + " failed: " + get_last_error();
			return false;
		}
	}
	else
		d_->resource_module_handle_ = GetModuleHandle(NULL);

	if (d_->p_raw_ui_)
	{
		delete d_->p_raw_ui_;
		d_->p_raw_ui_ = nullptr;
	}

	// make the layout
	liblec::cui::gui::page home_page(d_->caption_);
	liblec::cui::gui::page persistent_page("");

	std::string startup_error;
	d_->stop_ = !layout(persistent_page, home_page, startup_error);
	
	// capture caption from the name of the home page
	d_->caption_ = home_page.d_->page_name_;

	// now let's create this instance's cui_raw object
	d_->p_raw_ui_ = new liblec::cui::gui_raw::cui_raw(
		convert_string(d_->caption_),
		d_->command_procedure, d_->color_ui_background_, d_->color_ui_,
		d_->color_ui_hot_, d_->color_ui_disabled_, convert_string(d_->font_ui_),
		8, d_->color_tooltip_text_, d_->color_tooltip_background_,
		d_->color_tooltip_border_,
		d_->resource_module_handle_, d_->p_parent_,
		(void*)this);	// pass pointer to this liblec::cui::gui object

	// register instance
	d_->p_raw_ui_->registerInstance(convert_string(window_guid), 0);

	// load font files for Gdiplus
	for (auto &it : d_->font_files_)
	{
		std::basic_string<TCHAR> sErr;
		d_->p_raw_ui_->addFont(convert_string(it), sErr);
	}

	/////////////////////////////////////////////////////////////////
	// add caption icon
	int iMargin = 10;

	RECT rc;
	liblec::cui::gui_raw::cui_raw::onResize resize;
	resize.iPercH = 0;
	resize.iPercV = 0;
	resize.iPercCY = 0;
	resize.iPercCX = 0;
	
	rc.left = 0;
	rc.right = rc.left + 16;
	rc.top = 0;
	rc.bottom = rc.top + 16;

	RECT rcTitleBar;
	rcTitleBar.top = 1;
	rcTitleBar.bottom = d_->p_raw_ui_->getTitleBarHeight();
	rcTitleBar.left = iMargin;
	rcTitleBar.right = d_->width_ - 1;
	liblec::cui::gui_raw::cui_raw::posRect(rc, rcTitleBar, 0, 50);

	// decide small icon based on DPI scale
	double dpi_scale = d_->p_raw_ui_->getDPIScale();

	int png_icon_small = d_->png_resource_.png_resource_16;

	if (dpi_scale != 1)
	{
		if (dpi_scale <= 1.25)
			png_icon_small = d_->png_resource_.png_resource_20;
		else
			if (dpi_scale <= 1.5)
				png_icon_small = d_->png_resource_.png_resource_24;
			else
				if (dpi_scale <= 1.75)
					png_icon_small = d_->png_resource_.png_resource_28;
				else
					png_icon_small = d_->png_resource_.png_resource_32;
	}

	d_->p_raw_ui_->addImage(_T(""), d_->make_unique_id(), _T(""), true, false, d_->color_ui_,
	d_->color_ui_, d_->color_ui_background_, d_->color_ui_background_, false,
		d_->color_ui_, d_->color_ui_background_, d_->color_ui_background_, png_icon_small, _T(""),
		RGB(0,0,0), RGB(0,0,0), _T(""), 9,
		liblec::cui::gui_raw::cui_raw::imageTextPlacement::right, rc, resize, false,
		liblec::cui::gui_raw::cui_raw::onToggle::toggleRight, false, _T(""), { 0, 0 });

	// set icons
	d_->p_raw_ui_->setIcons(d_->ico_resource_, NULL, png_icon_small);

	auto text_size = [&](const std::basic_string<TCHAR>& text,
		const double& font_size,
		const std::basic_string<TCHAR>& font_name,
		const double& max_text_width)
	{
		HDC hdcScreen = GetDC(NULL);

		Gdiplus::Graphics graphics(hdcScreen);

		Gdiplus::Font* p_font = new Gdiplus::Font(&Gdiplus::FontFamily(font_name.c_str()),
			static_cast<Gdiplus::REAL>(font_size));

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

		auto round_up = [](const Gdiplus::REAL& real)
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

	// calculate width of title string
	SIZE titleSize = text_size(convert_string(d_->caption_),
		9, convert_string(d_->font_ui_), 0);

	/////////////////////////////////////////////////////////////////
	// add window caption (let's do it manually without using gui_impl::add_text_control)
	rc.left = rc.right + iMargin;
	rc.right = rc.left + titleSize.cx;
	rc.top = rcTitleBar.top;
	rc.bottom = rcTitleBar.bottom;

	int iBottom = rc.bottom;

	d_->caption_id_ = d_->make_unique_id();
	d_->handler_[d_->caption_id_] = [&]() {on_caption(); };	// register caption handler

	d_->p_raw_ui_->addText(_T(""),	// make it pageless so it appears on all pages
		d_->caption_id_,
		convert_string(d_->caption_),
		false,
		d_->color_ui_, d_->color_ui_hot_,
		convert_string(d_->caption_tooltip_),
		convert_string(d_->font_ui_),
		9,	// it's the caption, so impose this
		rc,
		liblec::cui::gui_raw::cui_raw::middleleft, resize, false);

	// add home page
	add_page(home_page);

	// add persistent controls
	add_page(persistent_page);

	if (d_->user_pos_)
	{
		if (d_->preset_pos_)
			d_->p_raw_ui_->setPosition(d_->window_pos_,
				d_->width_, d_->height_);
		else
			d_->p_raw_ui_->setPosition(d_->x_,
				d_->y_, d_->width_, d_->height_);
	}
	else
	{
		if (d_->p_parent_)
			d_->p_raw_ui_->setPosition(
				liblec::cui::gui_raw::cui_raw::windowPosition::centerToParent,
				d_->width_, d_->height_);
		else
			d_->p_raw_ui_->setPosition(
				liblec::cui::gui_raw::cui_raw::windowPosition::centerToWorkingArea,
				d_->width_, d_->height_);
	}

	// add close button
	d_->close_id_ = d_->make_unique_id();
	d_->handler_[d_->close_id_] = [&]() {on_stop(); };	// register stop handler

	d_->p_raw_ui_->addCloseBtn(d_->close_id_, true);

	if (d_->allow_resizing_)
	{
		// add maximize button
		d_->p_raw_ui_->addMaxBtn(d_->make_unique_id());

		// allow window resizing
		d_->p_raw_ui_->allowResizing();
	}

	// add minimize button
	d_->p_raw_ui_->addMinBtn(d_->make_unique_id());

	// set minimum width and height
	d_->p_raw_ui_->setMinWidthAndHeight(d_->min_width_, d_->min_height_);

	// hide UI shadow
	d_->p_raw_ui_->hideShadow();

	// create window
	d_->init_id_ = d_->make_unique_id();
	d_->handler_[d_->init_id_] = [&]() {on_run(); };			// register run handler

	d_->shutdown_id_ = d_->make_unique_id();
	d_->handler_[d_->shutdown_id_] = [&]() {on_shutdown(); };	// register shutdown handler

	bool result = true;

	if (!d_->stop_)
	{
		result = d_->p_raw_ui_->create(d_->init_id_, d_->shutdown_id_);

		if (!result)
			error = "Creating window failed";
	}
	else
	{
		error = startup_error;
		return false;
	}

	// cleanup
	if (d_->p_raw_ui_)
	{
		delete d_->p_raw_ui_;
		d_->p_raw_ui_ = nullptr;
	}

	if (!d_->resource_dll_filename_.empty())
	{
		// release the resources DLL
		if (d_->resource_module_handle_)
		{
			FreeLibrary(d_->resource_module_handle_);
			d_->resource_module_handle_ = NULL;
		}
	}

	return result;
} // run

void liblec::cui::gui::stop()
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->close();
	else
		d_->stop_ = true;
}

std::string liblec::cui::gui::home_page()
{
	return d_->caption_;
}

void liblec::cui::gui::set_width(const size_t &width)
{
	d_->width_ = width;
}

void liblec::cui::gui::set_min_width(const size_t &width)
{
	d_->min_width_ = width;
}

void liblec::cui::gui::set_height(const size_t &height)
{
	d_->height_ = height;
}

void liblec::cui::gui::set_min_height(const size_t &height)
{
	d_->min_height_ = height;
}

void liblec::cui::gui::set_min_width_and_height(const size_t &width,
	const size_t &height)
{
	d_->min_width_ = width;
	d_->min_height_ = height;
}

size_t liblec::cui::gui::width()
{
	if (d_->p_raw_ui_)
		return d_->p_raw_ui_->getWidth();
	else
		return d_->width_;
}

size_t liblec::cui::gui::min_width()
{
	return d_->min_width_;
}

size_t liblec::cui::gui::height()
{
	if (d_->p_raw_ui_)
		return d_->p_raw_ui_->getHeight() - title_bar_height();
	else
		return d_->height_ - title_bar_height();
}

size_t liblec::cui::gui::min_height()
{
	return d_->min_height_;
}

size_t liblec::cui::gui::title_bar_height()
{
	size_t title_bar_height = 0;

	if (d_->p_raw_ui_)
		title_bar_height = d_->p_raw_ui_->getTitleBarHeight();
	else
	{
		// todo: this is resource intensive. There has to be a better way. But what can we do for
		// now?

		// create temporary cui_raw object
		liblec::cui::gui_raw::cui_raw* p_raw_ui = new liblec::cui::gui_raw::cui_raw(
			_T("cui default window"),
			d_->command_procedure, d_->color_ui_background_, d_->color_ui_,
			d_->color_ui_hot_, d_->color_ui_disabled_, convert_string(d_->font_ui_),
			8, d_->color_tooltip_text_, d_->color_tooltip_background_,
			d_->color_tooltip_border_,
			GetModuleHandle(NULL),
			NULL,
			(void*)this);	// can do without passing this pointer

		if (p_raw_ui)
		{
			title_bar_height = p_raw_ui->getTitleBarHeight();

			delete p_raw_ui;
			p_raw_ui = nullptr;
		}
	}

	return title_bar_height;
} // title_bar_height

void liblec::cui::gui::set_position(const size_t &x,
	const size_t &y,
	const size_t &width,
	const size_t &height)
{
	d_->user_pos_ = true;

	d_->x_ = x;
	d_->y_ = y;
	d_->width_ = width;
	d_->height_ = height;
}

void liblec::cui::gui::set_position(const window_position &pos,
	const size_t &width,
	const size_t &height)
{
	d_->user_pos_ = true;
	d_->preset_pos_ = true;

	switch (pos)
	{
	case liblec::cui::window_position::center_to_working_area:
		d_->window_pos_ = liblec::cui::gui_raw::cui_raw::windowPosition::centerToWorkingArea;
		break;
	case liblec::cui::window_position::center_to_parent:
		d_->window_pos_ = liblec::cui::gui_raw::cui_raw::windowPosition::centerToParent;
		break;
	case liblec::cui::window_position::top_left:
		d_->window_pos_ = liblec::cui::gui_raw::cui_raw::windowPosition::topLeft;
		break;
	case liblec::cui::window_position::top_left_offset:
		d_->window_pos_ = liblec::cui::gui_raw::cui_raw::windowPosition::topLeftOffset;
		break;
	case liblec::cui::window_position::bottom_left:
		d_->window_pos_ = liblec::cui::gui_raw::cui_raw::windowPosition::bottomLeft;
		break;
	case liblec::cui::window_position::bottom_left_offset:
		d_->window_pos_ = liblec::cui::gui_raw::cui_raw::windowPosition::bottomLeftOffset;
		break;
	case liblec::cui::window_position::top_right:
		d_->window_pos_ = liblec::cui::gui_raw::cui_raw::windowPosition::topRight;
		break;
	case liblec::cui::window_position::top_right_offset:
		d_->window_pos_ = liblec::cui::gui_raw::cui_raw::windowPosition::topRightOffset;
		break;
	case liblec::cui::window_position::bottom_right:
		d_->window_pos_ = liblec::cui::gui_raw::cui_raw::windowPosition::bottomRight;
		break;
	case liblec::cui::window_position::bottom_right_offset:
		d_->window_pos_ = liblec::cui::gui_raw::cui_raw::windowPosition::bottomRightOffset;
		break;
	default:
		d_->window_pos_ = liblec::cui::gui_raw::cui_raw::windowPosition::centerToWorkingArea;
		break;
	}
} // set_position

void liblec::cui::gui::set_resource_dll(const std::string& file_name) {
	d_->resource_dll_filename_ = file_name;
}

void liblec::cui::gui::set_icons(const size_t &ico_resource,
	const caption_icon_png &png_resource)
{
	d_->ico_resource_ = ico_resource;
	d_->png_resource_ = png_resource;
}

bool liblec::cui::gui::load_fonts(const std::vector<font_file> &font_files,
	std::string &error)
{
	// load font files
	for (auto &it : font_files)
		d_->font_files_.push_back(it.fullpath);

	if (!liblec::cui::gui::gui_impl::load_font_files(d_->font_files_, error))
	{
		error = "Some required fonts could not be loaded: " + error;
		return false;
	}

	for (auto &it : font_files)
	{
		if (!liblec::cui::gui::gui_impl::font_available(it.font_name))
		{
			error = "Font '" + it.font_name + "' could not be loaded";
			return false;
		}
	}

	return true;
} // load_fonts

bool liblec::cui::gui::load_fonts(const std::vector<font_resource> &font_resources,
	std::string &error)
{
	// load font resources
	std::vector<int> vFontRes;

	for (auto &it : font_resources)
		vFontRes.push_back(it.font_resource_id);

	if (!liblec::cui::gui::gui_impl::load_font_resources(vFontRes, d_->font_resources_, "BINARY", d_->resource_module_handle_, error))
	{
		error = "Some required fonts could not be loaded: " + error;
		return false;
	}

	for (auto &it : font_resources)
	{
		if (!liblec::cui::gui::gui_impl::font_available(it.font_name))
		{
			error = "Font '" + it.font_name + "' could not be loaded";
			return false;
		}
	}

	return true;
} // load_fonts

void liblec::cui::gui::set_ui_font(const std::string &font)
{
	d_->font_ui_ = font;
}

void liblec::cui::gui::set_ui_color(color ui_color) {
	d_->color_ui_ = RGB(ui_color.red, ui_color.green, ui_color.blue);
}

void liblec::cui::gui::disable()
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->disable();
}

void liblec::cui::gui::enable()
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->enable();
}

bool liblec::cui::gui::is_enabled()
{
	if (d_->p_raw_ui_)
		return d_->p_raw_ui_->isEnabled();
	else
		return false;
}

void liblec::cui::gui::hide()
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->hideWindow();
}

void liblec::cui::gui::show()
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->showWindow();
}

void liblec::cui::gui::show(const bool &foreground)
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->showWindow(foreground);
}

bool liblec::cui::gui::is_visible()
{
	if (d_->p_raw_ui_)
		return d_->p_raw_ui_->isVisible();
	else
		return false;
}

void liblec::cui::gui::maximize()
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->maximize();
}

bool liblec::cui::gui::is_maximized()
{
	if (d_->p_raw_ui_)
		return d_->p_raw_ui_->isMaximized();
	else
		return false;
}

bool liblec::cui::gui::is_prompt_displayed()
{
	if (d_->p_raw_ui_)
		return d_->p_raw_ui_->isMessageBoxDisplayed();
	else
		return false;
}

void liblec::cui::gui::exclude_from_title_bar(const std::string &alias)
{
	try {
		if (d_->p_raw_ui_)
		{
			int unique_id = d_->id_map_.at(alias);

			std::string page_name = alias;

			auto idx = page_name.rfind("/");
			page_name.erase(idx, page_name.length());

			d_->p_raw_ui_->excludeFromTitleBar(convert_string(page_name), unique_id);
		}
	}
	catch (std::exception e) {
	}
}

void liblec::cui::gui::close()
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->close();
}

void liblec::cui::gui::close_hard()
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->closeHard();
}

int liblec::cui::gui::get_x()
{
	if (d_->p_raw_ui_)
		return d_->p_raw_ui_->getX();
	else
		return 0;
}

int liblec::cui::gui::get_y()
{
	if (d_->p_raw_ui_)
		return d_->p_raw_ui_->getY();
	else
		return 0;
}

void liblec::cui::gui::set_xy(const size_t &x, const size_t &y)
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->setXY(x, y);
}

void liblec::cui::gui::set_xy(const size_t &x,
	const size_t &y,
	const size_t &width,
	const size_t &height)
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->setXY(x, y, width, height);
}

void liblec::cui::gui::add_to_prevent_quit_list(const std::string & alias)
{
	if (d_->p_raw_ui_)
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		d_->p_raw_ui_->addToPreventQuitList(convert_string(page_name), unique_id);
	}
}

void liblec::cui::gui::prevent_quit()
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->preventQuit();
}

void liblec::cui::gui::allow_quit()
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->allowQuit();
}

void liblec::cui::gui::prevent_resizing()
{
	d_->allow_resizing_ = false;

	if (d_->p_raw_ui_)
		d_->p_raw_ui_->preventResizing();
}

void liblec::cui::gui::allow_resizing()
{
	d_->allow_resizing_ = true;

	if (d_->p_raw_ui_)
		d_->p_raw_ui_->allowResizing();
}

bool liblec::cui::gui::is_resizing_allowed()
{
	if (d_->p_raw_ui_)
		return d_->p_raw_ui_->resizing();
	else
		return d_->allow_resizing_;
}

void liblec::cui::gui::drop_files_accept(
	std::function<void(const std::string &fullpath)> on_drop_files)
{
	if (d_->p_raw_ui_)
	{
		size_t unique_id = d_->make_unique_id();

		d_->drop_files_id_ = unique_id;
		d_->on_drop_files_ = on_drop_files;

		// do not add this to the id map ... it will be handled custom in the command procedure

		d_->p_raw_ui_->dropFilesAccept(unique_id);
	}
}

void liblec::cui::gui::drop_files_reject()
{
	if (d_->p_raw_ui_)
	{
		d_->drop_files_id_ = 0;
		d_->on_drop_files_ = nullptr;

		d_->p_raw_ui_->dropFilesReject();
	}
}

void liblec::cui::gui::get_working_area(liblec::cui::rect &rect)
{
	if (d_->p_raw_ui_)
	{
		RECT rect_;
		d_->p_raw_ui_->getWorkingArea(rect_);

		rect.left = rect_.left;
		rect.top = rect_.top;
		rect.right = rect_.right;
		rect.bottom = rect_.bottom;
	}
}

void liblec::cui::gui::disable(const std::string &alias)
{
	if (d_->p_raw_ui_)
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		d_->p_raw_ui_->disableControl(convert_string(page_name), unique_id);
	}
}

void liblec::cui::gui::enable(const std::string &alias)
{
	if (d_->p_raw_ui_)
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		d_->p_raw_ui_->enableControl(convert_string(page_name), unique_id);
	}
}

bool liblec::cui::gui::is_enabled(const std::string &alias)
{
	if (d_->p_raw_ui_)
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		return d_->p_raw_ui_->controlEnabled(convert_string(page_name), unique_id);
	}
	else
		return false;
}

void liblec::cui::gui::hide(const std::string &alias)
{
	if (d_->p_raw_ui_)
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		d_->p_raw_ui_->hideControl(convert_string(page_name), unique_id);
	}
}

void liblec::cui::gui::show(const std::string &alias)
{
	if (d_->p_raw_ui_)
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		d_->p_raw_ui_->showControl(convert_string(page_name), unique_id);
	}
}

bool liblec::cui::gui::is_visible(const std::string &alias)
{
	if (d_->p_raw_ui_)
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		return d_->p_raw_ui_->controlVisible(convert_string(page_name), unique_id);
	}
	else
		return false;
}

void liblec::cui::gui::set_focus(const std::string &alias)
{
	if (d_->p_raw_ui_)
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		d_->p_raw_ui_->setFocus(convert_string(page_name), unique_id);
	}
}

bool liblec::cui::gui::set_text(const std::string &alias,
	const std::string &text_value,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::set_text";
		return false;
	}

	try
	{
		bool result = true;

		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		if (d_->p_raw_ui_)
		{
			result = d_->p_raw_ui_->setText(convert_string(page_name),
				unique_id,
				convert_string(text_value),
				error_);

			error = convert_string(error_);
		}

		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // set_text

bool liblec::cui::gui::get_text(const std::string & alias,
	std::string & text,
	std::string & error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_text";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> text_, error_;
		bool result = d_->p_raw_ui_->getText(convert_string(page_name),
			unique_id,
			text_,
			error_);

		text = convert_string(text_);
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_text

bool liblec::cui::gui::set_combobox_text(const std::string &alias,
	const std::string &text_value,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::set_combobox_text";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->setComboText(convert_string(page_name),
			unique_id,
			convert_string(text_value),
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // set_combobox_text

bool liblec::cui::gui::get_combobox_text(const std::string &alias,
	std::string &text,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_combobox_text";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> text_, error_;
		bool result = d_->p_raw_ui_->getComboText(convert_string(page_name),
			unique_id,
			text_,
			error_);

		text = convert_string(text_);
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_combobox_text

bool liblec::cui::gui::select_combobox_item(const std::string &alias,
	const std::string &item_to_select,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::select_combobox_item";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->selectComboItem(convert_string(page_name),
			unique_id,
			convert_string(item_to_select),
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // select_combobox_item

bool liblec::cui::gui::repopulate_combobox(const std::string &alias,
	const std::vector<std::string> &items,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::repopulate_combobox";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::vector<std::basic_string<TCHAR>> items_(items.size());

		for (size_t i = 0; i < items.size(); i++)
			items_[i] = convert_string(items[i]);

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->repopulateCombo(convert_string(page_name),
			unique_id,
			items_,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // repopulate_combobox

bool liblec::cui::gui::set_editbox_text(const std::string &alias,
	const std::string &text_value,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::set_editbox_text";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->setEditText(convert_string(page_name),
			unique_id,
			convert_string(text_value),
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // set_editbox_text

bool liblec::cui::gui::get_editbox_text(const std::string &alias,
	std::string &text,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_editbox_text";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_, text_;
		bool result = d_->p_raw_ui_->getEditText(convert_string(page_name),
			unique_id,
			text_,
			error_);

		text = convert_string(text_);
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_editbox_text

bool liblec::cui::gui::get_edit_remaining_characters(const std::string & alias, int & chars, std::string & error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_edit_remaining_characters";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->getEditCharsLeft(convert_string(page_name),
			unique_id,
			chars,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_edit_remaining_characters

void liblec::cui::gui::set_timer(const std::string &alias,
	const size_t &milliseconds,
	std::function<void()>on_timer)
{
	if (d_->p_raw_ui_)
	{
		size_t unique_id = d_->make_unique_id();

		// make page path
		std::string page_path = d_->caption_ + "/";	// always associate timers to the home page

		// add this timer to the id map
		d_->id_map_[page_path + alias] = unique_id;

		d_->handler_[unique_id] = on_timer;		// register timer handler

		// set the timer
		d_->p_raw_ui_->setTimer(unique_id, milliseconds);
	}
} // set_timer

bool liblec::cui::gui::timer_running(const std::string &alias)
{
	if (d_->p_raw_ui_)
	{
		try
		{
			int unique_id = d_->id_map_.at(alias);
			return d_->p_raw_ui_->timerRunning(unique_id);
		}
		catch (std::exception&)
		{
		}
	}
	
	return false;
} // timer_running

void liblec::cui::gui::stop_timer(const std::string &alias)
{
	if (d_->p_raw_ui_)
	{
		try
		{
			// get timer unique id
			int unique_id = d_->id_map_.at(alias);
			d_->p_raw_ui_->stopTimer(unique_id);
		}
		catch (std::exception &)
		{
		}
	}
} // stop_timer

bool liblec::cui::gui::change_image(const std::string &alias,
	const size_t &png_resource,
	const bool &update_now,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::change_image";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->changeImage(convert_string(page_name),
			unique_id,
			png_resource,
			false, convert_string(""), update_now, error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // change_image

bool liblec::cui::gui::change_image(const std::string &alias,
	const std::string &filename,
	const bool &update_now,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::change_image";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->changeImage(convert_string(page_name),
			unique_id,
			convert_string(filename),
			false, convert_string(""), update_now, error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // change_image

bool liblec::cui::gui::change_image_text(const std::string &alias,
	const std::string &text,
	const std::string &description,
	const bool &update_now,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::change_image_text";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->changeImageText(convert_string(page_name),
			unique_id,
			convert_string(text),
			convert_string(description), update_now, error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // change_image_text

bool liblec::cui::gui::set_image_bar(const std::string &alias,
	const liblec::cui::color &color,
	const bool &update_now,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::set_image_bar";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->setImageBar(convert_string(page_name),
			unique_id,
			RGB(color.red, color.green, color.blue),
			update_now,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // set_image_bar

bool liblec::cui::gui::remove_image_bar(const std::string &alias,
	const bool &update_now,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::remove_image_bar";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->removeImageBar(convert_string(page_name),
			unique_id,
			update_now,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // remove_image_bar

bool liblec::cui::gui::set_image_colors(const std::string &alias,
	const liblec::cui::widgets::image_colors &color,
	const bool &update_now,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::set_image_colors";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->setImageColors(convert_string(page_name),
			unique_id,
			RGB(color.color.red, color.color.green, color.color.blue),
			RGB(color.color_hot.red, color.color_hot.green, color.color_hot.blue),
			RGB(color.color_text.red, color.color_text.green, color.color_text.blue),
			RGB(color.color_text_hot.red,
				color.color_text_hot.green, color.color_text_hot.blue),
			RGB(color.color_border.red,
				color.color_border.green, color.color_border.blue),
			RGB(color.color_border_hot.red,
				color.color_border_hot.green, color.color_border_hot.blue),
			update_now,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // set_image_colors

bool liblec::cui::gui::update_image(const std::string &alias,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::update_image";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->updateImage(convert_string(page_name),
			unique_id,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // update_image

bool liblec::cui::gui::save_image(const std::string& alias,
	liblec::cui::image_format format,
	liblec::cui::size max_size,
	const std::string& full_path,
	std::string& actual_path,
	std::string& error) {
	if (!d_->p_raw_ui_) {
		error = "Library usage error: liblec::cui::gui::save_image";
		return false;
	}

	try {
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_, full_path_ = convert_string(full_path);
		bool result = d_->p_raw_ui_->saveImage(convert_string(page_name),
			unique_id,
			convert_image_format(format),
			convert_size(max_size),
			full_path_,
			error_);

		actual_path = convert_string(full_path_);
		error = convert_string(error_);
		return result;
	}
	catch (std::exception& e) {
		error = e.what();
		return false;
	}
}

bool liblec::cui::gui::set_toggle_button(const std::string &alias,
	const bool &on,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::set_toggle_button";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->setToggleButton(convert_string(page_name),
			unique_id,
			on,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // set_toggle_button

bool liblec::cui::gui::get_toggle_button(const std::string &alias,
	bool &on,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_toggle_button";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->getToggleButton(convert_string(page_name),
			unique_id,
			on,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_toggle_button

bool liblec::cui::gui::set_progress_bar(const std::string &alias,
	const double &percentage,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::set_progress_bar";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->setProgressBar(convert_string(page_name),
			unique_id,
			percentage,
			false,
			RGB(0, 150, 0),
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // set_progress_bar

bool liblec::cui::gui::set_password_strength_bar(const std::string &alias,
	const double &percentage, std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::set_password_strength_bar";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->setPasswordStrengthBar(convert_string(page_name),
			unique_id,
			percentage,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // set_password_strength_bar

bool liblec::cui::gui::set_selector(const std::string &alias,
	const std::string &item_alias,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::set_selector";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);
		int unique_id_item = d_->id_map_.at(item_alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->setSelector(convert_string(page_name),
			unique_id,
			unique_id_item,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // set_selector

bool liblec::cui::gui::get_selector(const std::string &alias,
	std::string &item_alias,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_selector";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		int iSelectorItemID = 0;
		bool result = d_->p_raw_ui_->getSelector(convert_string(page_name),
			unique_id,
			iSelectorItemID,
			error_);

		// get item alias from the map
		for (auto &it : d_->id_map_)
		{
			if (it.second == iSelectorItemID)
			{
				item_alias = it.first;
				break;
			}
		}

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_selector

bool liblec::cui::gui::set_star_rating(const std::string &alias,
	const size_t &rating,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::set_star_rating";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->setStarRating(convert_string(page_name),
			unique_id,
			rating,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // set_star_rating

bool liblec::cui::gui::get_star_rating(const std::string &alias,
	size_t &rating,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_star_rating";
		return false;
	}

	rating = 0;

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		int rating_ = 0;
		bool result = d_->p_raw_ui_->getStarRating(convert_string(page_name),
			unique_id,
			rating_,
			error_);

		rating = rating_;
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_star_rating

bool liblec::cui::gui::set_date(const std::string &alias,
	const liblec::cui::date &date,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::set_date";
		return false;
	}

	try
	{
		liblec::cui::gui_raw::cui_raw::date date_;
		date_.iDay = date.day;
		date_.iYear = date.year;

		switch (date.month)
		{
		case liblec::cui::month::january:
			date_.enMonth = liblec::cui::gui_raw::cui_raw::month::january;
			break;
		case liblec::cui::month::february:
			date_.enMonth = liblec::cui::gui_raw::cui_raw::month::february;
			break;
		case liblec::cui::month::march:
			date_.enMonth = liblec::cui::gui_raw::cui_raw::month::march;
			break;
		case liblec::cui::month::april:
			date_.enMonth = liblec::cui::gui_raw::cui_raw::month::april;
			break;
		case liblec::cui::month::may:
			date_.enMonth = liblec::cui::gui_raw::cui_raw::month::may;
			break;
		case liblec::cui::month::june:
			date_.enMonth = liblec::cui::gui_raw::cui_raw::month::june;
			break;
		case liblec::cui::month::july:
			date_.enMonth = liblec::cui::gui_raw::cui_raw::month::july;
			break;
		case liblec::cui::month::august:
			date_.enMonth = liblec::cui::gui_raw::cui_raw::month::august;
			break;
		case liblec::cui::month::september:
			date_.enMonth = liblec::cui::gui_raw::cui_raw::month::september;
			break;
		case liblec::cui::month::october:
			date_.enMonth = liblec::cui::gui_raw::cui_raw::month::october;
			break;
		case liblec::cui::month::november:
			date_.enMonth = liblec::cui::gui_raw::cui_raw::month::november;
			break;
		case liblec::cui::month::december:
			date_.enMonth = liblec::cui::gui_raw::cui_raw::month::december;
			break;
		default:
			date_.enMonth = liblec::cui::gui_raw::cui_raw::month::january;
			break;
		}

		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->setDate(convert_string(page_name),
			unique_id,
			date_,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // set_date

bool liblec::cui::gui::get_date(const std::string &alias,
	liblec::cui::date &date,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_date";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		liblec::cui::gui_raw::cui_raw::date date_;
		bool result = d_->p_raw_ui_->getDate(convert_string(page_name),
			unique_id,
			date_,
			error_);

		date.day = date_.iDay;
		date.year = date_.iYear;

		switch (date_.enMonth)
		{
		case liblec::cui::gui_raw::cui_raw::january:
			date.month = liblec::cui::month::january;
			break;
		case liblec::cui::gui_raw::cui_raw::february:
			date.month = liblec::cui::month::february;
			break;
		case liblec::cui::gui_raw::cui_raw::march:
			date.month = liblec::cui::month::march;
			break;
		case liblec::cui::gui_raw::cui_raw::april:
			date.month = liblec::cui::month::april;
			break;
		case liblec::cui::gui_raw::cui_raw::may:
			date.month = liblec::cui::month::may;
			break;
		case liblec::cui::gui_raw::cui_raw::june:
			date.month = liblec::cui::month::june;
			break;
		case liblec::cui::gui_raw::cui_raw::july:
			date.month = liblec::cui::month::july;
			break;
		case liblec::cui::gui_raw::cui_raw::august:
			date.month = liblec::cui::month::august;
			break;
		case liblec::cui::gui_raw::cui_raw::september:
			date.month = liblec::cui::month::september;
			break;
		case liblec::cui::gui_raw::cui_raw::october:
			date.month = liblec::cui::month::october;
			break;
		case liblec::cui::gui_raw::cui_raw::november:
			date.month = liblec::cui::month::november;
			break;
		case liblec::cui::gui_raw::cui_raw::december:
			date.month = liblec::cui::month::december;
			break;
		default:
			date.month = liblec::cui::month::january;
			break;
		}

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_date

bool liblec::cui::gui::set_time(const std::string &alias,
	const liblec::cui::time &time,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::set_time";
		return false;
	}

	try
	{
		liblec::cui::gui_raw::cui_raw::time time_;
		time_.iHour = time.hour;
		time_.iMinute = time.minute;
		time_.iSecond = time.second;

		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->setTime(convert_string(page_name),
			unique_id,
			time_,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // set_time

bool liblec::cui::gui::get_time(const std::string &alias,
	liblec::cui::time &time,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_time";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		liblec::cui::gui_raw::cui_raw::time time_;
		bool result = d_->p_raw_ui_->getTime(convert_string(page_name),
			unique_id,
			time_,
			error_);

		time.hour = time_.iHour;
		time.minute = time_.iMinute;
		time.second = time_.iSecond;

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_time

bool liblec::cui::gui::richedit_load_file(const std::string &alias,
	const std::string &full_path,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::richedit_load_file";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->richEditLoad(convert_string(page_name),
			unique_id,
			convert_string(full_path),
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // richedit_load_file

bool liblec::cui::gui::richedit_load_rtf(const std::string &alias,
	const std::string &rtf,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::richedit_load_rtf";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->richEditRTFLoad(convert_string(page_name),
			unique_id,
			rtf,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // richedit_load_rtf

bool liblec::cui::gui::richedit_save(const std::string &alias,
	const std::string &full_path,
	std::string &actual_path,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::richedit_save";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_, full_path_ = convert_string(full_path);
		bool result = d_->p_raw_ui_->richEditSave(convert_string(page_name),
			unique_id,
			full_path_,
			error_);

		actual_path = convert_string(full_path_);
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // richedit_save

bool liblec::cui::gui::richedit_get_rtf(const std::string &alias,
	std::string &rtf,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::richedit_get_rtf";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->richEditRTFGet(convert_string(page_name),
			unique_id,
			rtf,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // richedit_get_rtf

bool liblec::cui::gui::set_barchart_scale(const std::string &alias,
	const bool &autoscale,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::set_barchart_scale";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->barChartScaleSet(convert_string(page_name),
			unique_id,
			autoscale,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // set_barchart_scale

bool liblec::cui::gui::get_barchart_scale(const std::string &alias,
	bool &autoscale,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_barchart_scale";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->barChartScaleGet(convert_string(page_name),
			unique_id,
			autoscale,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_barchart_scale

bool liblec::cui::gui::get_barchart_name(const std::string &alias,
	std::string &name,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_barchart_name";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_, name_;
		bool result = d_->p_raw_ui_->barChartNameGet(convert_string(page_name),
			unique_id,
			name_,
			error_);

		name = convert_string(name_);
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_barchart_name

bool liblec::cui::gui::barchart_reload(const std::string &alias,
	const liblec::cui::widgets::barchart_data &data,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::barchart_reload";
		return false;
	}

	try
	{
		std::vector<liblec::cui::gui_raw::cui_raw::barChartData> vValues;
		vValues.reserve(data.bars.size());

		for (size_t i = 0; i < data.bars.size(); i++)
		{
			liblec::cui::gui_raw::cui_raw::barChartData data_;
			data_.iNumber = i + 1;
			data_.sLabel = convert_string(data.bars[i].label);
			data_.clrBar = RGB(data.bars[i].color.red,
				data.bars[i].color.green, data.bars[i].color.blue);
			data_.dValue = data.bars[i].value;

			vValues.push_back(data_);
		}

		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->barChartReload(convert_string(page_name),
			unique_id,
			convert_string(data.caption),
			convert_string(data.x_label),
			convert_string(data.y_label),
			data.lower_limit,
			data.upper_limit,
			data.autoscale,
			vValues,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // barchart_reload

bool liblec::cui::gui::barchart_save(const std::string &alias,
	liblec::cui::image_format format,
	const std::string &full_path,
	std::string &actual_path,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::barchart_save";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_, full_path_ = convert_string(full_path);
		bool result = d_->p_raw_ui_->barChartSave(convert_string(page_name),
			unique_id,
			convert_image_format(format),
			full_path_,
			error_);

		actual_path = convert_string(full_path_);
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // barchart_save

bool liblec::cui::gui::set_linechart_scale(const std::string &alias,
	const bool &autoscale,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::set_linechart_scale";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->lineChartScaleSet(convert_string(page_name),
			unique_id,
			autoscale,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // set_linechart_scale

bool liblec::cui::gui::get_linechart_scale(const std::string &alias,
	bool &autoscale,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_linechart_scale";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->lineChartScaleGet(convert_string(page_name),
			unique_id,
			autoscale,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_linechart_scale

bool liblec::cui::gui::get_linechart_name(const std::string &alias,
	std::string &name,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_linechart_name";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_, name_;
		bool result = d_->p_raw_ui_->lineChartNameGet(convert_string(page_name),
			unique_id,
			name_,
			error_);

		name = convert_string(name_);
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_linechart_name

bool liblec::cui::gui::linechart_reload(const std::string &alias,
	const liblec::cui::widgets::linechart_data &data,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::linechart_reload";
		return false;
	}

	try
	{
		std::vector<liblec::cui::gui_raw::cui_raw::lineInfo> vLines;
		vLines.reserve(data.lines.size());

		for (size_t i = 0; i < data.lines.size(); i++)
		{
			liblec::cui::gui_raw::cui_raw::lineInfo line;
			line.sSeriesName = convert_string(data.lines[i].series_name);
			line.clrLine = RGB(data.lines[i].color.red,
				data.lines[i].color.green, data.lines[i].color.blue);

			line.vValues.reserve(data.lines[i].points.size());

			for (size_t j = 0; j < data.lines[i].points.size(); j++)
			{
				liblec::cui::gui_raw::cui_raw::barChartData data_;
				data_.iNumber = j + 1;
				data_.sLabel = convert_string(data.lines[i].points[j].label);
				data_.clrBar = RGB(data.lines[i].points[j].color.red,
					data.lines[i].points[j].color.green, data.lines[i].points[j].color.blue);
				data_.dValue = data.lines[i].points[j].value;

				line.vValues.push_back(data_);
			}

			vLines.push_back(line);
		}

		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->lineChartReload(convert_string(page_name),
			unique_id,
			convert_string(data.caption),
			convert_string(data.x_label),
			convert_string(data.y_label),
			data.lower_limit,
			data.upper_limit,
			data.autoscale,
			vLines,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // linechart_reload

bool liblec::cui::gui::linechart_save(const std::string &alias,
	liblec::cui::image_format format,
	const std::string &full_path,
	std::string &actual_path,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::linechart_save";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_, full_path_ = convert_string(full_path);
		bool result = d_->p_raw_ui_->lineChartSave(convert_string(page_name),
			unique_id,
			convert_image_format(format),
			full_path_,
			error_);

		actual_path = convert_string(full_path_);
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // linechart_save

bool liblec::cui::gui::piechart_reload(const std::string &alias,
	const liblec::cui::widgets::piechart_data &data,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::piechart_reload";
		return false;
	}

	try
	{
		std::vector<liblec::cui::gui_raw::cui_raw::pieChartData> vValues;
		vValues.reserve(data.slices.size());

		for (size_t i = 0; i < data.slices.size(); i++)
		{
			liblec::cui::gui_raw::cui_raw::pieChartData data_;
			data_.iNumber = i + 1;
			data_.sItemLabel = convert_string(data.slices[i].label);
			data_.clrItem = RGB(data.slices[i].color.red,
				data.slices[i].color.green, data.slices[i].color.blue);
			data_.dValue = data.slices[i].value;

			vValues.push_back(data_);
		}

		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->pieChartReload(convert_string(page_name),
			unique_id,
			data.autocolor,
			convert_string(data.caption),
			vValues,
			convert_piechart_hover_effect(data.on_hover),
			data.doughnut,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // piechart_reload

bool liblec::cui::gui::piechart_save(const std::string &alias,
	liblec::cui::image_format format,
	const std::string &full_path,
	std::string &actual_path,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::piechart_save";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_, full_path_ = convert_string(full_path);
		bool result = d_->p_raw_ui_->pieChartSave(convert_string(page_name),
			unique_id,
			convert_image_format(format),
			full_path_,
			error_);

		actual_path = convert_string(full_path_);
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // piechart_save

bool liblec::cui::gui::add_listview_row(const std::string &alias,
	liblec::cui::widgets::listview_row &row,
	const bool &scroll_to_bottom,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::add_listview_row";
		return false;
	}

	try
	{
		liblec::cui::gui_raw::cui_raw::listviewRow row_;
		row_.vItems.reserve(row.items.size());

		for (size_t j = 0; j < row.items.size(); j++)
		{
			liblec::cui::gui_raw::cui_raw::listviewItem item_;
			item_.bCustom = row.items[j].custom_text_color;
			item_.clrText = RGB(row.items[j].color_text.red,
				row.items[j].color_text.green, row.items[j].color_text.blue);
			item_.iRowNumber = row.items[j].row_number;
			item_.sColumnName = convert_string(row.items[j].column_name);
			item_.sItemData = convert_string(row.items[j].item_data);

			row_.vItems.push_back(item_);
		}

		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->addListviewRow(convert_string(page_name),
			unique_id,
			row_,
			scroll_to_bottom,
			error_);

		std::vector<liblec::cui::gui_raw::cui_raw::listviewRow> rows_;
		rows_.reserve(1);
		rows_.push_back(row_);

		row = convert_listview_rows(rows_)[0];
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // add_listview_row

bool liblec::cui::gui::repopulate_listview(const std::string &alias,
	std::vector<liblec::cui::widgets::listview_row> &data,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::repopulate_listview";
		return false;
	}

	try
	{
		std::vector<liblec::cui::gui_raw::cui_raw::listviewRow> data_ =
			convert_listview_rows(data);

		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->repopulateListview(convert_string(page_name),
			unique_id,
			data_,
			error_);

		data = convert_listview_rows(data_);
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // repopulate_listview

bool liblec::cui::gui::get_listview(const std::string &alias,
	std::vector<liblec::cui::widgets::listview_column> &columns,
	std::vector<liblec::cui::widgets::listview_row> &data,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_listview";
		return false;
	}

	try
	{
		std::vector<liblec::cui::gui_raw::cui_raw::listviewColumn> columns_;
		std::vector<liblec::cui::gui_raw::cui_raw::listviewRow> data_;

		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->getListview(convert_string(page_name),
			unique_id,
			columns_,
			data_,
			error_);

		columns = convert_listview_columns(columns_);
		data = convert_listview_rows(data_);
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_listview

bool liblec::cui::gui::get_listview_selected(const std::string &alias,
	std::vector<liblec::cui::widgets::listview_row> &rows,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_listview_selected";
		return false;
	}

	try
	{
		std::vector<liblec::cui::gui_raw::cui_raw::listviewRow> rows_;

		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->getListviewSelected(convert_string(page_name),
			unique_id,
			rows_,
			error_);

		rows = convert_listview_rows(rows_);
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_listview_selected

bool liblec::cui::gui::update_listview_item(const std::string &alias,
	const liblec::cui::widgets::listview_item &item,
	const liblec::cui::widgets::listview_row &row,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::update_listview_item";
		return false;
	}

	try
	{
		std::vector<liblec::cui::widgets::listview_row> rows;
		rows.reserve(1);
		rows.push_back(row);

		liblec::cui::gui_raw::cui_raw::listviewItem item_;
		item_.bCustom = item.custom_text_color;
		item_.clrText = RGB(item.color_text.red,
			item.color_text.green, item.color_text.blue);
		item_.iRowNumber = item.row_number;
		item_.sColumnName = convert_string(item.column_name);
		item_.sItemData = convert_string(item.item_data);

		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->updateListViewItem(convert_string(page_name),
			unique_id,
			item_,
			convert_listview_rows(rows)[0],
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // update_listview_item

bool liblec::cui::gui::remove_listview_row(const std::string &alias,
	const size_t &row_number,
	std::string & error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::remove_listview_row";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> error_;
		bool result = d_->p_raw_ui_->removeListViewRow(convert_string(page_name),
			unique_id,
			row_number,
			error_);

		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // remove_listview_row

bool liblec::cui::gui::get_selected_tab(const std::string &alias,
	std::string &selected_tab,
	std::string &error)
{
	if (!d_->p_raw_ui_)
	{
		error = "Library usage error: liblec::cui::gui::get_selected_tab";
		return false;
	}

	try
	{
		int unique_id = d_->id_map_.at(alias);

		std::string page_name = alias;

		auto idx = page_name.rfind("/");
		page_name.erase(idx, page_name.length());

		std::basic_string<TCHAR> selected_tab_, error_;
		bool result = d_->p_raw_ui_->getSelectedTab(convert_string(page_name),
			unique_id,
			selected_tab_,
			error_);

		selected_tab = convert_string(selected_tab_);
		error = convert_string(error_);
		return result;
	}
	catch (std::exception &e)
	{
		error = e.what();
		return false;
	}
} // get_selected_tab

std::string liblec::cui::gui::open_file(const open_file_params &params)
{
	if (d_->p_raw_ui_)
	{
		liblec::cui::gui_raw::cui_raw::opensavefileParams params_;
		params_.bHideNavigationToolbar = true;
		params_.bIncludeAllSupportedTypes = params.include_all_supported_types;
		params_.bSmallWindow = params.small_window;
		params_.iFontSize = params.font_size;
		params_.sFontName = convert_string(d_->set_font(params.font));
		params_.sTitle = convert_string(params.title);

		params_.vFileTypes.resize(params.file_types.size());

		for (size_t i = 0; i < params_.vFileTypes.size(); i++)
		{
			params_.vFileTypes[i].sFileExtension = convert_string(params.file_types[i].extension);
			params_.vFileTypes[i].sDescription = convert_string(params.file_types[i].description);
		}

		std::basic_string<TCHAR> fullpath_;
		d_->p_raw_ui_->openFile(params_, fullpath_);
		return convert_string(fullpath_);
	}
	else
		return std::string();
} // open_file

std::string liblec::cui::gui::save_file(const open_file_params &params,
	const std::string &file_name)
{
	if (d_->p_raw_ui_)
	{
		liblec::cui::gui_raw::cui_raw::opensavefileParams params_;
		params_.bHideNavigationToolbar = true;
		params_.bIncludeAllSupportedTypes = params.include_all_supported_types;
		params_.bSmallWindow = params.small_window;
		params_.iFontSize = params.font_size;
		params_.sFontName = convert_string(d_->set_font(params.font));
		params_.sTitle = convert_string(params.title);

		params_.vFileTypes.resize(params.file_types.size());

		for (size_t i = 0; i < params_.vFileTypes.size(); i++)
		{
			params_.vFileTypes[i].sFileExtension = convert_string(params.file_types[i].extension);
			params_.vFileTypes[i].sDescription = convert_string(params.file_types[i].description);
		}

		std::basic_string<TCHAR> fullpath_;
		d_->p_raw_ui_->saveFile(params_, convert_string(file_name), fullpath_);
		return convert_string(fullpath_);
	}
	else
		return std::string();
} // save_file

std::string liblec::cui::gui::select_folder(const select_folder_params &params)
{
	if (d_->p_raw_ui_)
	{
		liblec::cui::gui_raw::cui_raw::selectFolderParams params_;
		params_.bSmallWindow = params.small_window;
		params_.iFontSize = params.font_size;
		params_.sFontName = convert_string(d_->set_font(params.font));
		params_.sTitle = convert_string(params.title);
		params_.sMessage = convert_string(params.message);

		std::basic_string<TCHAR> path_;
		d_->p_raw_ui_->selectFolder(params_, path_);
		return convert_string(path_);
	}
	else
		return std::string();
} // select_folder

bool liblec::cui::gui::pick_color(liblec::cui::color &color)
{
	if (d_->p_raw_ui_)
	{
		bool result = false;
		COLORREF color_;
		d_->p_raw_ui_->pickColor(result, color_);
		
		color.red = (unsigned short)GetRValue(color_);
		color.green = (unsigned short)GetGValue(color_);
		color.blue = (unsigned short)GetBValue(color_);
		return result;
	}
	else
		return false;
} // pick_color

bool liblec::cui::gui::add_tray_icon(const size_t &ico_resource,
	const std::string &title,
	const std::vector<tray_icon_item> &items,
	std::string &error)
{
	if (d_->p_raw_ui_)
	{
		bool result = false;
		
		std::vector<liblec::cui::gui_raw::cui_raw::trayIconItem> items_;
		items_.resize(items.size());

		for (size_t i = 0; i < items_.size(); i++)
		{
			size_t unique_id = d_->make_unique_id();
				
			items_[i].iUniqueID = unique_id;
			items_[i].bDefault = items[i].is_default;
			items_[i].bEnabled = items[i].is_enabled;
			items_[i].sLabel = convert_string(items[i].label);

			// register tray icon item on_click handler
			d_->handler_[unique_id] = items[i].on_click;
		}

		std::basic_string<TCHAR> error_;
		result = d_->p_raw_ui_->addTrayIcon(ico_resource, convert_string(title), items_, error_);

		error = convert_string(error_);
		return result;
	}
	else
		return false;
} // add_tray_icon

bool liblec::cui::gui::change_tray_icon(const size_t &ico_resource,
	const std::string &title,
	std::string &error)
{
	if (d_->p_raw_ui_)
	{
		bool result = false;

		std::basic_string<TCHAR> error_;
		result = d_->p_raw_ui_->changeTrayIcon(ico_resource, convert_string(title), error_);

		error = convert_string(error_);
		return result;
	}
	else
		return false;
} // change_tray_icon

void liblec::cui::gui::remove_tray_icon()
{
	if (d_->p_raw_ui_)
		d_->p_raw_ui_->removeTrayIcon();
}

bool liblec::cui::gui::prompt(const prompt_params &params,
	const std::string &message,
	const std::string &details)
{
	liblec::cui::gui_raw::cui_raw::msgBoxParams params_;
	params_.sFontName = convert_string(d_->font_ui_);
	params_.iFontSize = d_->font_size_ui_;
	params_.sMessage = convert_string(message);
	params_.sDetails = convert_string(details);

	params_.IDP_ICON = params.png_icon_resource;
	params_.clrImage = RGB(params.color.red, params.color.green, params.color.blue);

	switch (params.type)
	{
	case liblec::cui::gui::prompt_type::ok_cancel:
		params_.type = liblec::cui::gui_raw::cui_raw::msgBoxType::okCancel;
		break;

	case liblec::cui::gui::prompt_type::yes_no:
		params_.type = liblec::cui::gui_raw::cui_raw::msgBoxType::yesNo;
		break;

	case liblec::cui::gui::prompt_type::ok:
	default:
		params_.type = liblec::cui::gui_raw::cui_raw::msgBoxType::okOnly;
		break;
	}

	liblec::cui::gui_raw::cui_raw::msgBoxResult res;
	
	if (d_->p_raw_ui_)
		res = d_->p_raw_ui_->msgBox(params_);
	else
	{
		res = liblec::cui::gui_raw::cui_raw::msgBox(params_,
			convert_string(d_->caption_),
			d_->color_ui_background_,
			d_->color_ui_,
			d_->color_ui_hot_,
			convert_string(d_->font_ui_),
			8,
			d_->color_tooltip_text_,
			d_->color_tooltip_background_,
			d_->color_tooltip_background_,
			0,
			0,
			0,
			GetModuleHandle(NULL),
			nullptr
			);
	}

	switch (res)
	{
	case liblec::cui::gui_raw::cui_raw::noresponse:
		break;

	case liblec::cui::gui_raw::cui_raw::notexecuted:
		break;

	case liblec::cui::gui_raw::cui_raw::cancel:
		break;

	case liblec::cui::gui_raw::cui_raw::no:
		break;

	case liblec::cui::gui_raw::cui_raw::ok:
	case liblec::cui::gui_raw::cui_raw::yes:
	default:
		return true;
		break;
	}

	return false;
} // prompt

void liblec::cui::gui::notification(const notification_params &params,
	const std::string &message,
	const std::string &details,
	const size_t &timeout_seconds)
{
	if (d_->p_raw_ui_)
	{
		liblec::cui::gui_raw::cui_raw::notParams params_;
		params_.sFontName = convert_string(d_->font_ui_);
		params_.iFontSize = d_->font_size_ui_;
		params_.iTimer = timeout_seconds;
		params_.sMessage = convert_string(message);
		params_.sDetails = convert_string(details);
		
		params_.IDP_ICON = params.png_icon_resource;
		params_.clrImage = RGB(params.color.red, params.color.green, params.color.blue);
		
		d_->p_raw_ui_->notX(params_);
	}
} // notification
