/*
** gui.h - cui framework gui interface
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

#if defined(CUI_EXPORTS)
	#include "cui.h"
	#include "cui_raw/cui_raw.h"
#else
	#include <liblec/cui.h>
	#include <liblec/cui/gui_raw/cui_raw.h>
#endif

#include <string>
#include <vector>
#include <functional>

namespace liblec
{
	namespace cui
	{
		/// <summary>
		/// Color object. Values for each element range from 0 to 255.
		/// </summary>
		struct color
		{
			unsigned short red = 0;
			unsigned short green = 0;
			unsigned short blue = 0;
		};

		/// <summary>
		/// Rectangle object.
		/// </summary>
		class cui_api rect
		{
		public:
			long left = 0;
			long right = 0;
			long top = 0;
			long bottom = 0;

			long width() { return right - left; }
			long height() { return bottom - top; }
			void set_width(long width) { right = left + width; }
			void set_height(long height) { bottom = top + height; }
		}; // rect

		struct point
		{
			long x = 0;
			long y = 0;
		};

		struct size
		{
			long width = 0;
			long height = 0;
		};

		enum class month
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
		}; // month

		struct date
		{
			unsigned short day = 1;
			liblec::cui::month month = liblec::cui::month::january;
			short year = 2019;
		};

		struct time
		{
			unsigned short hour = 0;
			unsigned short minute = 0;
			unsigned short second = 0;
		};

		enum class image_format
		{
			png,
			bmp,
			jpeg,
			none,
		};

		enum class window_position
		{
			// offset members use 10 pixels

			center_to_working_area,
			center_to_parent,
			top_left,
			top_left_offset,
			bottom_left,
			bottom_left_offset,
			top_right,
			top_right_offset,
			bottom_right,
			bottom_right_offset,
		}; // window_position

		namespace tools
		{
			enum class snap
			{
				bottom_left,
				bottom,
				bottom_right,
				top_left,
				top,
				top_right,
				right_top,
				right,
				right_bottom,
				left_top,
				left,
				left_bottom,
			};

			void cui_api snap_to(const liblec::cui::rect &rect_reference,
				snap snap_type,
				const size_t &clearance,
				liblec::cui::rect &rect);

			void cui_api pos_rect(const liblec::cui::rect &rect_reference,
				liblec::cui::rect &rect,
				const size_t &perc_h,
				const size_t &perc_v);

		} // namespace tools

		namespace widgets
		{
			/// <summary>
			/// Action on resize.
			/// </summary>
			struct on_resize
			{
				/// <summary>
				/// The percentage rate for following the parent's right border.
				/// 0 = doesn't move horizontally, 100 = moves same number of pixels horizontally
				/// that the parent's right border has moved.
				/// </summary>
				int perc_h = 0;

				/// <summary>The percentage rate for following the parent's bottom border.
				/// 0 = doesn't move vertically, 100 = moves same number of pixels vertically
				/// that the parent's bottom border has moved.
				/// </summary>
				int perc_v = 0;

				/// <summary>
				/// The percentage rate for following the parent's change in width. 0 = doesn't
				/// follow change in parent's width, 100 = width changes at same rate as that of
				/// parent.
				/// </summary>
				int perc_width = 0;

				/// <summary>
				/// The percentage rate for following the parent's change in height. 0 = doesn't
				/// follow change in parent's height, 100 = height changes at same rate as that of
				/// parent.
				/// </summary>
				int perc_height = 0;
			}; // on_resize

			/// <summary>
			/// Text alignment.
			/// </summary>
			enum class text_alignment
			{
				top_left,
				center,
				top_right,

				/// <summary>
				/// Place text on the middle (vertically) left.
				/// </summary>
				middle_left,

				/// <summary>
				/// Place text in the middle (vertically and horizontally).
				/// </summary>
				middle,

				/// <summary>
				/// Place text on the middle (vertically) right.
				/// </summary>
				middle_right,

				bottom_left,

				/// <summary>
				/// Place text on the bottom middle (vertically).
				/// </summary>
				bottom_middle,
				bottom_right,
			}; // text_alignment

			struct text
			{
				std::string text_value;

				/// <summary>
				/// Alias to uniquely identify the control within the current page. It is needed
				/// to manipulate the control after it has been created. When referring to it
				/// later, use the following format "page name/alias" since it is only required to
				/// be unique in the current page. This allows controls in different pages can
				/// to safely have different "aliases" because each alias is married to a page.
				/// </summary>
				std::string alias;
				liblec::cui::color color = { 0, 0, 0 };
				liblec::cui::color color_hot = { 255, 180, 0 };
				std::string tooltip;
				std::string font;
				double font_size = 9;
				liblec::cui::rect rect;
				liblec::cui::widgets::text_alignment alignment =
					liblec::cui::widgets::text_alignment::top_left;
				liblec::cui::widgets::on_resize on_resize;
				bool multiline = false;
				std::function<void()> on_click = nullptr;
			}; // text

			struct button
			{
				std::string caption;

				std::string alias;
				bool is_default = true;
				std::string tooltip;
				std::string font;
				double font_size = 9;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
				std::function<void()> on_click = nullptr;
			}; // button

			struct combobox
			{
				std::string alias;
				std::vector<std::string> items;
				std::string selected_item;
				std::string font;
				double font_size = 9;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
				bool auto_complete = false;
				bool read_only = true;
				std::function<void()> on_selection = nullptr;
			}; // combobox

			struct editbox
			{
				std::string alias;
				std::string cue_banner;
				std::string font;
				double font_size = 9;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
				bool multiline = false;
				bool scrollbar = false;
				bool password = false;
				bool read_only = false;
				size_t limit = 0;
				std::string allowed_set;
				std::string forbidden_set;
				std::string control_to_invoke_alias;
				std::function<void()> on_type = nullptr;
			}; // editbox

			/// <summary>
			/// Position of text in image control.
			/// </summary>
			enum class image_text_placement
			{
				bottom,
				top,
				right,
				right_top,
				right_bottom,
				left,
				left_top,
				left_bottom,
			};

			/// <summary>
			/// Toggle action when mouse moves over.
			/// </summary>
			enum class on_toggle
			{
				left,
				right,
				up,
				down,
			}; // on_toggle

			struct image_colors
			{
				liblec::cui::color color_text = { 100, 100, 100 };
				liblec::cui::color color_text_hot = { 21, 79, 139 };
				liblec::cui::color color = { 0, 0, 0 };
				liblec::cui::color color_hot = { 21, 79, 139 };
				liblec::cui::color color_border = { 255, 255, 255 };
				liblec::cui::color color_border_hot = { 240, 240, 240 };
			}; // image_colors

			struct image
			{
				std::string alias;
				std::string text;
				std::string tooltip;
				std::string filename;
				size_t png_resource = 0;
				std::string font;
				double font_size = 9;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;

				/// <param name="tight_fit">
				/// Whether to display only the image (no border, no toggling).
				/// </param>
				bool tight_fit = false;

				bool change_color = false;
				image_colors color;
				liblec::cui::color color_background = { 255, 255, 255 };
				liblec::cui::color color_background_hot = { 250, 250, 250 };
				bool bar = false;
				liblec::cui::color color_bar = { 255, 0, 0 };
				liblec::cui::widgets::image_text_placement text_position =
					liblec::cui::widgets::image_text_placement::bottom;
				bool toggle = false;
				liblec::cui::widgets::on_toggle on_toggle =
					liblec::cui::widgets::on_toggle::up;
				std::function<void()> on_click = nullptr;
			}; // image

			struct icon
			{
				std::string alias;
				std::string text;
				std::string description;
				std::string tooltip;
				std::string filename;
				size_t png_resource = 0;
				std::string font;
				double font_size = 9;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
				bool change_color = false;
				image_colors color;
				liblec::cui::color color_background = { 255, 255, 255 };
				liblec::cui::color color_background_hot = { 245, 245, 255 };
				bool bar = false;
				liblec::cui::color color_bar = { 255, 0, 0 };
				liblec::cui::widgets::image_text_placement text_position =
					liblec::cui::widgets::image_text_placement::right_top;
				bool toggle = true;
				liblec::cui::widgets::on_toggle on_toggle =
					liblec::cui::widgets::on_toggle::up;
				liblec::cui::size size = { 48, 48 };
				std::function<void()> on_click = nullptr;
			}; // icon

			struct toggle_button
			{
				std::string alias;
				std::string text_on;
				std::string text_off;
				std::string tooltip;
				std::string font;
				double font_size = 9;
				liblec::cui::color color_text = { 100, 100, 100 };
				liblec::cui::color color_on = { 21, 79, 139 };
				liblec::cui::color color_off = { 200, 200, 200 };
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
				bool on = true;
				std::function<void()> on_toggle = nullptr;
			}; // toggle_button

			struct progress_bar
			{
				std::string alias;
				liblec::cui::color color = { 0, 150, 0 };
				liblec::cui::color color_unfilled = { 250, 250, 250 };
				double initial_percentage = 0;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
			}; // progress_bar

			struct password_strength_bar
			{
				std::string alias;
				liblec::cui::color color_unfilled = { 230, 230, 230 };
				double initial_percentage = 0;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
			}; // password_strength_bar

			struct rectangle
			{
				liblec::cui::color color = { 250, 250, 250 };
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
			}; // rectangle

			struct hairline
			{
				liblec::cui::color color = { 200, 200, 200 };
				liblec::cui::point point;
				size_t length_horizontal = 0;
				size_t length_vertical = 0;
				liblec::cui::widgets::on_resize on_resize;
			}; // hairline

			struct groupbox
			{
				liblec::cui::color color = { 200, 200, 200 };
				std::vector<liblec::cui::rect> rects;
				liblec::cui::widgets::on_resize on_resize;
				size_t clearance_ = 5;
			};

			struct selector_item
			{
				std::string alias;
				std::string label;
				bool default_item = false;
				std::function<void()> on_select = nullptr;
			};

			struct selector
			{
				std::string alias;
				std::string tooltip;
				std::vector<liblec::cui::widgets::selector_item> items;
				std::string font;
				double font_size = 9;
				liblec::cui::color color_text = { 0, 0, 0 };
				liblec::cui::color color_bar = { 21, 79, 139 };
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
			}; // selector

			struct star_rating
			{
				std::string alias;
				std::string tooltip;
				liblec::cui::color color_on = { 21, 79, 139 };
				liblec::cui::color color_off = { 200, 200, 200 };
				liblec::cui::color color_hot = { 255, 180, 0 };
				liblec::cui::color color_border = { 200, 200, 200 };
				size_t highest_rating = 5;
				size_t initial_rating = 0;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
				std::function<void()> on_rating = nullptr;
			}; // star_rating

			struct date
			{
				std::string alias;
				std::string font;
				double font_size = 9;
				bool allow_none = false;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
				std::function<void()> on_date = nullptr;
			}; // date

			struct time
			{
				std::string alias;
				std::string font;
				double font_size = 9;
				bool allow_none = false;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
				std::function<void()> on_time = nullptr;
			}; // time

			struct richedit
			{
				std::string alias;
				std::string cue_banner;
				std::string font_ui_;
				double font_size_ui = 9;
				std::string font = "Georgia";
				double font_size = 11;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
				bool read_only = false;
				bool border = true;
				liblec::cui::color color_border = { 200, 200, 200 };
			}; // richedit

			struct chart_entry
			{
				std::string label;
				double value = 0;
				liblec::cui::color color = { 79, 129, 189 };
			};

			struct barchart_data
			{
				std::string caption;
				std::string x_label;
				std::string y_label;
				int lower_limit = 0;
				int upper_limit = 100;
				bool autoscale = true;
				bool autocolor = false;
				std::vector<liblec::cui::widgets::chart_entry> bars;
			}; // barchart_data

			struct barchart
			{
				std::string alias;
				liblec::cui::widgets::barchart_data data;
				std::string font;
				double font_size = 9;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
			}; // barchart

			struct line_info
			{
				std::string series_name;
				liblec::cui::color color = { 79, 129, 189 };
				std::vector<liblec::cui::widgets::chart_entry> points;
			};

			struct linechart_data
			{
				std::string caption;
				std::string x_label;
				std::string y_label;
				int lower_limit = 0;
				int upper_limit = 100;
				bool autoscale = true;
				bool autocolor = false;
				std::vector<liblec::cui::widgets::line_info> lines;
			}; // linechart_data

			struct linechart
			{
				std::string alias;
				liblec::cui::widgets::linechart_data data;
				std::string font;
				double font_size = 9;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
			}; // linechart

			enum class piechart_hover_effect
			{
				glow,
				glow_and_arc,
				glow_and_grow,
				glow_and_shrink_others,
			};

			struct piechart_data
			{
				std::string caption;
				bool doughnut = true;
				bool autocolor = true;
				liblec::cui::widgets::piechart_hover_effect on_hover =
					liblec::cui::widgets::piechart_hover_effect::glow_and_arc;
				std::vector<liblec::cui::widgets::chart_entry> slices;
			}; // piechart_data

			struct piechart
			{
				std::string alias;
				liblec::cui::widgets::piechart_data data;
				std::string font;
				double font_size = 9;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
			}; // piechart

			enum class listview_column_type
			{
				string_,
				char_,
				int_,
				float_,
			};

			struct listview_column
			{
				std::string name;
				size_t width = 50;
				liblec::cui::widgets::listview_column_type type;
				bool barchart = false;
				size_t barchart_max = 100;
				liblec::cui::color color_bar = { 190, 190, 255 };
				liblec::cui::color color_text = { 0, 0, 0 };
			}; // listview_column

			struct listview_item
			{
				std::string column_name;
				std::string item_data;
				bool custom_text_color = false;
				liblec::cui::color color_text = { 0, 0, 0 };
				size_t row_number = 0;
			}; // listview_item

			struct listview_row
			{
				std::vector<liblec::cui::widgets::listview_item> items;
			};

			struct context_menu_item
			{
				std::string label;
				bool is_default = false;
				std::function<void()> on_click = nullptr;
				int png_resource = 0;
			};

			struct listview
			{
				std::string alias;
				std::vector<liblec::cui::widgets::listview_column> columns;
				std::vector<liblec::cui::widgets::listview_row> data;
				std::vector<liblec::cui::widgets::context_menu_item> context_menu;
				std::string unique_column_name;
				bool border = true;
				bool gridlines = true;
				bool sort_by_clicking_column = true;
				std::string font;
				double font_size = 9;
				liblec::cui::rect rect;
				liblec::cui::widgets::on_resize on_resize;
				std::function<void()> on_selection = nullptr;
			}; // listview

			struct tab_control
			{
				std::string alias;
				liblec::cui::rect rect;
				std::string font;
				double font_size = 9;
				liblec::cui::color color_tab_line = { 200, 200, 200 };
			};

			struct tab
			{
				std::string tab_name;
				std::string tooltip;
				bool default_tab = false;
			};

		} // namespace widgets

		/// Correct usage of the liblec::cui::gui class is as follows:
		/// 
		/// 1. Make a class that inherits from liblec::cui::gui, optionally overwriting the
		/// virtual functions. It is highly recommended to make these private to prevent manual
		/// calls to them outside the class.
		/// 
		/// class my_gui_class : public liblec::cui::gui
		/// {
		/// public:
		///		my_gui_class(){};
		///		my_gui_class(const std::string &app_guid) :
		///			liblec::cui::gui(app_guid) {};
		/// 
		///		void text_1_handler() { // text_1 has been clicked }
		/// 
		/// private:
		///		bool layout(liblec::cui::gui::page &persistent_page,
		///				liblec::cui::gui::page &home_page,
		///				std::string &error) { // layout code here }
		///		void on_run() { // startup code here }
		///		void on_caption() { // caption has been clicked }
		///		void on_stop() { stop(); }
		///		void on_shutdown() { // cleanup code here }
		/// }
		/// 
		/// 2. Add layout code to the layout() member as follows:
		/// 
		/// bool layout(liblec::cui::gui::page &persistent_page,
		///				liblec::cui::gui::page &home_page,
		///				std::string &error)
		/// {
		///		home_page.set_name("Sample App");
		/// 
		///		liblec::cui::widgets::text text_1;
		///		text_1.text_value = "Sample text";
		///		text_1.on_click = [&]() {text_1_handler(); };	// add handler for text 1
		/// 
		///		home_page.add_text(text_1);
		/// 
		///		return true;
		/// }
		/// 
		/// To add controls that persist across all pages use persistent_page. When adding other
		/// pages make sure not to draw over these controls.
		/// 
		/// NOTE: do not call add_page() in layout()
		/// 
		/// 3. Setup the handlers for the various controls. There are two options
		/// 
		/// Option A (using lambda functions, as shown above, and highly recommended)
		/// text_1.on_click = [&]() {text_1_handler(); };
		/// 
		/// Option B (using std::bind)
		/// text_1.on_click = std::bind(&my_gui_class::text_1_handler, this};
		/// 
		/// 4. Run the app
		/// 
		/// my_gui_class gui();
		/// 
		/// std::string error;
		/// gui.run(error);
		/// 
		/// 5. To add another page to the window is simple (we will do this in text_1_handler in
		/// this example
		/// 
		/// void text_1_handler()
		/// {
		///		liblec::cui::gui::page sample_page("Sample Page");
		/// 
		///		// add text control
		///		liblec::cui::widgets::text text_1;
		///		text_1.text_value = "< Back";
		///		text_2.on_click = [&]() {show_previous_page(); };
		///		sample_page.add_text(text_1);
		/// 
		///		// add edit box
		///		liblec::cui::widgets::editbox edit_1;
		///		edit_1.cue_banner = "Enter text here";
		///		sample_page.add_editbox(edit_1);
		/// 
		///		add_page(sample_page);
		/// }
		/// 
		/// 6. In order to interact with a control use the .alias member as follows
		/// 
		/// sample_page.set_name("Sample Page");
		/// text_1.alias = "sample_alias";
		///	sample_page.add_text(text_1);
		/// 
		/// .... then elsewhere when you need to, say read the value of text_1's text ....
		/// 
		/// std::string text_value, error;
		/// get_text("Sample Page/sample_alias", text_value, error);
		/// 
		/// NOTE: when declaring an alias use a simple string (has to be unique in that page).
		/// When calling the alias combine it with the page the control is in using a forward
		/// slash.
		/// 

		/// <summary>
		/// gui class.
		/// </summary>
		/// 
		/// <remarks>
		/// Any app that uses this class should declare itself as DPI-aware through its application
		/// manifest.
		/// </remarks>
		class cui_api gui
		{
		public:
			gui();

			/// <param name="app_guid">
			/// Unique guid used to prevent multiple instances of the app,
			/// e.g. "Global\\{7DBACCA2-B96E-4D0E-B0F7-933EA9A3C064}"
			/// Leave empty to allow multiple instances, or just use the overload that doesn't
			/// take this parameter.
			/// </param>
			gui(const std::string &app_guid);

			~gui();

			void modal(gui& parent);
			void modal(liblec::cui::gui_raw::cui_raw& parent);

			liblec::cui::gui_raw::cui_raw& get_raw();

			class cui_api page
			{
			public:
				page(const std::string &page_name);
				~page();

				void set_name(const std::string &page_name);

				void add_text(const liblec::cui::widgets::text &t);
				void add_button(const liblec::cui::widgets::button &b);
				void add_combobox(const liblec::cui::widgets::combobox &c);
				void add_editbox(const liblec::cui::widgets::editbox &e);
				void add_icon(const liblec::cui::widgets::icon &i);
				void add_image(const liblec::cui::widgets::image &i);
				void add_toggle_button(const liblec::cui::widgets::toggle_button &t);
				void add_progress_bar(const liblec::cui::widgets::progress_bar &p);
				void add_password_strength_bar(
					const liblec::cui::widgets::password_strength_bar &p);
				void add_rectangle(const liblec::cui::widgets::rectangle &r);
				void add_hairline(const liblec::cui::widgets::hairline &h);
				void add_groupbox(const liblec::cui::widgets::groupbox &g);
				void add_selector(const liblec::cui::widgets::selector &s);
				void add_star_rating(const liblec::cui::widgets::star_rating &s);
				void add_date(const liblec::cui::widgets::date &d);
				void add_time(const liblec::cui::widgets::time &t);
				void add_richedit(const liblec::cui::widgets::richedit &r);
				void add_barchart(const liblec::cui::widgets::barchart &b);
				void add_linechart(const liblec::cui::widgets::linechart &l);
				void add_piechart(const liblec::cui::widgets::piechart &p);
				void add_listview(const liblec::cui::widgets::listview &l);
				void add_tabcontrol(const liblec::cui::widgets::tab_control &t);
				void add_tab(const liblec::cui::widgets::tab &t);

			private:
				class page_impl;
				page_impl* d_;

				page();

				friend gui;

				page(const page&);
				page& operator=(const page&);
			}; // page

			bool page_exists(const std::string &page_name);
			void add_page(const page &page_to_add);
			void show_page(const std::string &page_name);
			std::string current_page();
			std::string previous_page();
			void show_previous_page();

			/// <summary>
			/// Run the gui app.
			/// </summary>
			/// 
			/// <param name="error">
			/// Error information.
			/// </param>
			/// 
			/// <returns>
			/// Returns true if successful, else false.
			/// </returns>
			/// 
			/// <remarks>
			/// This is a blocking function and will only return when the gui window is closed.
			/// </remarks>
			bool run(std::string &error);

			/// <summary>
			/// Run the gui app.
			/// </summary>
			/// 
			/// <param name="window_guid">
			/// Unique guid used to enable another instance to find this instance's window and
			/// open it. Leave empty to prevent another instance from finding and opening this
			/// instance's window, or just use the overload that doesn't have this parameter.
			/// </param>
			/// 
			/// <param name="error">
			/// Error information.
			/// </param>
			/// 
			/// <returns>
			/// Returns true if successful, else false.
			/// </returns>
			/// 
			/// <remarks>
			/// This is a blocking function and will only return when <see cref="stop()"/> is
			/// called. A good place to do that is in the <see cref="on_stop()"/> handler.
			/// </remarks>
			bool run(const std::string &window_guid,
				std::string &error);

			/// <summary>
			/// Stop the gui app.
			/// </summary>
			void stop();

			// app

			/// <summary>
			/// Get the name of the home page (this is also the name of the app).
			/// </summary>
			/// 
			/// <returns>
			/// Returns the name of the home page.
			/// </returns>
			std::string home_page();

			// window controls used before run() is called
			
			// Only width(), height(), and title_bar_height() also work after run() is called

			void set_width(const size_t &width);
			void set_min_width(const size_t &width);
			void set_height(const size_t &height);
			void set_min_height(const size_t &height);

			void set_min_width_and_height(const size_t &width,
				const size_t &height);
			
			size_t width();
			size_t min_width();
			size_t height();
			size_t min_height();

			size_t title_bar_height();

			void set_position(const size_t &x,
				const size_t &y,
				const size_t &width,
				const size_t &height);

			void set_position(const window_position &pos,
				const size_t &width,
				const size_t &height);

			void set_resource_dll(const std::string& file_name);

			struct caption_icon_png
			{
				size_t png_resource_16 = 0;
				size_t png_resource_20 = 0;
				size_t png_resource_24 = 0;
				size_t png_resource_28 = 0;
				size_t png_resource_32 = 0;
			};

			void set_icons(const size_t &ico_resource,
				const caption_icon_png &png_resource);

			struct font_file
			{
				std::string fullpath;
				std::string font_name;
			};

			struct font_resource
			{
				int font_resource_id = 0;
				std::string font_name;
			};

			bool load_fonts(const std::vector<font_file> &font_files,
				std::string &error);

			bool load_fonts(const std::vector<font_resource> &font_resources,
				std::string &error);

			void set_ui_font(const std::string &font);

			void set_ui_color(color ui_color);

			// window functions used after run() is called (the resizing functions also work
			// before run() is called)

			void disable();
			void enable();
			bool is_enabled();

			void hide();
			void show();
			void show(const bool &foreground);
			bool is_visible();

			void maximize();
			bool is_maximized();

			bool is_prompt_displayed();

			void exclude_from_title_bar(const std::string &alias);

			void close();
			void close_hard();

			int get_x();
			int get_y();
			void set_xy(const size_t &x,
				const size_t &y);
			void set_xy(const size_t &x,
				const size_t &y,
				const size_t &width,
				const size_t &height);

			void add_to_prevent_quit_list(const std::string &alias);
			void prevent_quit();
			void allow_quit();

			void prevent_resizing();
			void allow_resizing();
			bool is_resizing_allowed();

			void drop_files_accept(std::function<void(const std::string &fullpath)> on_drop_files);
			void drop_files_reject();

			// general

			void get_working_area(liblec::cui::rect &rect);

			// controls

			void disable(const std::string &alias);
			void enable(const std::string &alias);
			bool is_enabled(const std::string &alias);

			void hide(const std::string &alias);
			void show(const std::string &alias);
			bool is_visible(const std::string &alias);

			void set_focus(const std::string &alias);

			// text

			bool set_text(const std::string &alias,
				const std::string &text_value,
				std::string &error);

			bool get_text(const std::string &alias,
				std::string &text,
				std::string &error);

			// combobox

			bool set_combobox_text(const std::string &alias,
				const std::string &text_value,
				std::string &error);

			bool get_combobox_text(const std::string &alias,
				std::string &text,
				std::string &error);

			bool select_combobox_item(const std::string &alias,
				const std::string &item_to_select,
				std::string &error);

			bool repopulate_combobox(const std::string &alias,
				const std::vector<std::string> &items,
				std::string &error);

			// editbox

			bool set_editbox_text(const std::string &alias,
				const std::string &text_value,
				std::string &error);

			bool get_editbox_text(const std::string &alias,
				std::string &text,
				std::string &error);

			bool get_edit_remaining_characters(const std::string &alias,
				int &chars,
				std::string &error);

			// timer

			void set_timer(const std::string &alias,
				const size_t &milliseconds,
				std::function<void()>on_timer);

			bool timer_running(const std::string &alias);

			void stop_timer(const std::string &alias);

			// image

			bool change_image(const std::string &alias,
				const size_t &png_resource,
				const bool &update_now,
				std::string &error);

			bool change_image(const std::string &alias,
				const std::string &filename,
				const bool &update_now,
				std::string &error);

			bool change_image_text(const std::string &alias,
				const std::string &text,
				const std::string &description,
				const bool &update_now,
				std::string &error);

			bool set_image_bar(const std::string &alias,
				const liblec::cui::color &color,
				const bool &update_now,
				std::string &error);

			bool remove_image_bar(const std::string &alias,
				const bool &update_now,
				std::string &error);

			bool set_image_colors(const std::string &alias,
				const liblec::cui::widgets::image_colors &color,
				const bool &update_now,
				std::string &error);

			bool update_image(const std::string &alias,
				std::string &error);

			bool save_image(const std::string& alias,
				liblec::cui::image_format format,
				liblec::cui::size max_size,
				const std::string& full_path,
				std::string& actual_path,
				std::string& error);

			// toggle buttons

			bool set_toggle_button(const std::string &alias,
				const bool &on,
				std::string &error);

			bool get_toggle_button(const std::string &alias,
				bool &on,
				std::string &error);

			// progress bars

			bool set_progress_bar(const std::string &alias,
				const double &percentage,
				std::string &error);

			// password strength bars

			bool set_password_strength_bar(const std::string &alias,
				const double &percentage,
				std::string &error);

			// selector controls

			bool set_selector(const std::string &alias,
				const std::string &item_alias,
				std::string &error);

			bool get_selector(const std::string &alias,
				std::string &item_alias,
				std::string &error);

			// star rating controls

			bool set_star_rating(const std::string &alias,
				const size_t &rating,
				std::string &error);

			bool get_star_rating(const std::string &alias,
				size_t &rating,
				std::string &error);

			// date controls

			bool set_date(const std::string &alias,
				const liblec::cui::date &date,
				std::string &error);

			bool get_date(const std::string &alias,
				liblec::cui::date &date,
				std::string &error);

			// time controls

			bool set_time(const std::string &alias,
				const liblec::cui::time &time,
				std::string &error);

			bool get_time(const std::string &alias,
				liblec::cui::time &time,
				std::string &error);

			// rich edit controls

			bool richedit_load_file(const std::string &alias,
				const std::string &full_path,
				std::string &error);

			bool richedit_load_rtf(const std::string &alias,
				const std::string &rtf,
				std::string &error);

			bool richedit_save(const std::string &alias,
				const std::string &full_path,
				std::string &actual_path,
				std::string &error);

			bool richedit_get_rtf(const std::string &alias,
				std::string &rtf,
				std::string &error);

			// bar charts

			bool set_barchart_scale(const std::string &alias,
				const bool &autoscale,
				std::string &error);

			bool get_barchart_scale(const std::string &alias,
				bool &autoscale,
				std::string &error);

			bool get_barchart_name(const std::string &alias,
				std::string &name,
				std::string &error);

			bool barchart_reload(const std::string &alias,
				const liblec::cui::widgets::barchart_data &data,
				std::string &error);

			bool barchart_save(const std::string &alias,
				liblec::cui::image_format format,
				const std::string &full_path,
				std::string &actual_path,
				std::string &error);

			// line charts

			bool set_linechart_scale(const std::string &alias,
				const bool &autoscale,
				std::string &error);

			bool get_linechart_scale(const std::string &alias,
				bool &autoscale,
				std::string &error);

			bool get_linechart_name(const std::string &alias,
				std::string &name,
				std::string &error);

			bool linechart_reload(const std::string &alias,
				const liblec::cui::widgets::linechart_data &data,
				std::string &error);

			bool linechart_save(const std::string &alias,
				liblec::cui::image_format format,
				const std::string &full_path,
				std::string &actual_path,
				std::string &error);

			// pie charts

			bool piechart_reload(const std::string &alias,
				const liblec::cui::widgets::piechart_data &data,
				std::string &error);

			bool piechart_save(const std::string &alias,
				liblec::cui::image_format format,
				const std::string &full_path,
				std::string &actual_path,
				std::string &error);

			// listview controls

			bool add_listview_row(const std::string &alias,
				liblec::cui::widgets::listview_row &row,
				const bool &scroll_to_bottom,
				std::string &error);

			bool repopulate_listview(const std::string &alias,
				std::vector<liblec::cui::widgets::listview_row> &data,
				std::string &error);

			bool get_listview(const std::string &alias,
				std::vector<liblec::cui::widgets::listview_column> &columns,
				std::vector<liblec::cui::widgets::listview_row> &data,
				std::string &error);

			bool get_listview_selected(const std::string &alias,
				std::vector<liblec::cui::widgets::listview_row> &rows,
				std::string &error);

			bool update_listview_item(const std::string &alias,
				const liblec::cui::widgets::listview_item &item,
				const liblec::cui::widgets::listview_row &row,
				std::string &error);

			bool remove_listview_row(const std::string &alias,
				const size_t &row_number,
				std::string &error);

			// tab controls

			bool get_selected_tab(const std::string &alias,
				std::string &selected_tab,
				std::string &error);

			// open and save files

			struct file_type
			{
				/// <summary>
				/// File extension (without the dot), e.g. rtf.
				/// </summary>
				std::string extension = "rtf";

				/// <summary>
				/// File description, e.g. Rich Text Format.
				/// </summary>
				std::string description = "Rich Text Format";
			};

			struct open_file_params
			{
				std::string title;
				std::vector<file_type> file_types;
				bool small_window = false;
				std::string font;
				double font_size = 9;
				bool include_all_supported_types = true;
			};

			std::string open_file(const open_file_params &params);
			std::string save_file(const open_file_params &params,
				const std::string &file_name);

			// select folder

			struct select_folder_params
			{
				std::string title;
				std::string message;
				bool small_window = false;
				std::string font;
				double font_size = 9;
			};

			std::string select_folder(const select_folder_params &params);

			// pick color

			bool pick_color(liblec::cui::color &color);

			// tray icon

			struct tray_icon_item
			{
				std::string alias;
				std::string label;
				bool is_default = false;
				bool is_enabled = true;
				std::function<void()> on_click = nullptr;
			};

			bool add_tray_icon(const size_t &ico_resource,
				const std::string &title,
				const std::vector<tray_icon_item> &items,
				std::string &error);

			bool change_tray_icon(const size_t &ico_resource,
				const std::string &title,
				std::string &error);

			void remove_tray_icon();

			// prompt and notification

			enum class prompt_type
			{
				ok,
				ok_cancel,
				yes_no
			};

			struct prompt_params
			{
				liblec::cui::gui::prompt_type type = liblec::cui::gui::prompt_type::ok;
				int png_icon_resource = 0;
				liblec::cui::color color = { 0, 0, 0 };
			};

			/// <summary>
			/// Prompt the user in the form of a message box.
			/// </summary>
			/// 
			/// <param name="params">
			/// The prompt parameters.
			/// </param>
			/// 
			/// <param name="message">
			/// The message (title) of the prompt (optional)
			/// </param>
			/// 
			/// <param name="details">
			/// The details of the message.
			/// </param>
			/// 
			/// <returns>
			/// Returns true when either Ok or Yes are pressed, else false.
			/// </returns>
			bool prompt(const prompt_params &params,
				const std::string &message,
				const std::string &details);

			struct notification_params
			{
				int png_icon_resource = 0;
				liblec::cui::color color = { 0, 0, 0 };
			};

			/// <summary>
			/// Display a notification to the user.
			/// </summary>
			/// 
			/// <param name="message">
			/// The message (title) of the prompt (optional)
			/// </param>
			/// 
			/// <param name="details">
			/// The details of the message.
			/// </param>
			/// 
			/// <param name="timeout_seconds">
			/// The time, in seconds, it will take for the popup notification to close itself
			/// automatically. Set to 0 to prevent it from closing automatically.
			/// </param>
			/// 
			/// <remarks>
			/// This functions is non-blocking and returns almost immediately. If there is already
			/// another notification currently displayed the current call will place this
			/// notification into a queue. The notification will be displayed one second after the
			/// last notification is closed. When the notification is displayed, it will remain
			/// displayed until the user moves the mouse, at which point the timeout will begin
			/// to count down. If the user moves the mouse above it, however, it will not close
			/// automatically; in this case the only way to close it is using it's close button or
			/// Alt + F4.
			/// </remarks>
			void notification(const notification_params &params,
				const std::string &message,
				const std::string &details,
				const size_t &timeout_seconds
			);

			// virtual

			/// <summary>
			/// First virtual function called after run, for the layout of the home page.
			/// </summary>
			/// 
			/// <param name="persistent_page">
			/// A reference to the persistent page. Use this object to add controls that remain on
			/// the window no matter what page one is in. The name of this page MUST be empty.
			/// </param>
			/// 
			/// <param name="home_page">
			/// A reference to the home page. Use this object to define the layout of the
			/// home page and add controls to it.
			/// </param>
			/// 
			/// <param name="error">
			/// Write error information here.
			/// </param>
			/// 
			/// <returns>
			/// Return true if successful, else false.
			/// </returns>
			virtual bool layout(liblec::cui::gui::page &persistent_page,
				liblec::cui::gui::page &home_page,
				std::string &error) { return true; }

			/// <summary>
			/// Called during window creation just before it is displayed.
			/// </summary>
			/// 
			/// <remarks>
			/// This is a good place to place all essential initialization code like reading
			/// registry settings.
			/// </remarks>
			virtual void on_run() {}

			/// <summary>
			/// Called when the window caption is clicked.
			/// </summary>
			virtual void on_caption() {}

			/// <summary>
			/// Virtual function called when an application close request is received either
			/// through the close button being clicked or throught Alt+F4.
			/// Call <see cref="stop()"/> to stop the app.
			/// </summary>
			virtual void on_stop() {}

			/// <summary>
			/// Called just before the window is destroyed, at which point <see cref="run()"/>
			/// will return.
			/// </summary>
			/// 
			/// <remarks>
			/// This is a good place to place all essential cleanup code. At this point the
			/// window has been hidden. It will be destroyed soon after this function returns.
			/// </remarks>
			virtual void on_shutdown() {}

		private:
			class gui_impl;
			gui_impl* d_;

			gui(const gui&);
			gui& operator=(const gui&);
		}; // gui

	} // namespace cui

} // namespace liblec
