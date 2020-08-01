/*
** cui.h - cui interface
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

#if defined(CUI_EXPORTS)
	#define cui_api __declspec(dllexport)
#else
	#define cui_api __declspec(dllimport)

	// for visual styles
	#pragma comment(linker, "\"/manifestdependency:type='win32' \
	name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
	processorArchitecture='*' publicKeyToken='6595b64144ccf1df' \
	language='*'\"")

	#if defined(_WIN64)
		#if defined(_DEBUG)
			#pragma comment(lib, "cui64d.lib")
		#else
			#pragma comment(lib, "cui64.lib")
		#endif
	#else
		#if defined(_DEBUG)
			#pragma comment(lib, "cui32d.lib")
		#else
			#pragma comment(lib, "cui32.lib")
		#endif
	#endif
#endif

#include <string>
#include <vector>

namespace liblec
{
	namespace cui
	{
		/// <summary>
		/// Get the version of the cui framework.
		/// </summary>
		/// 
		/// <returns>
		/// Returns the version number as a string in the form "cui 1.0.0 04 Jan 2019"
		/// </returns>
		std::string cui_api version();

		struct password_quality {
			int strength = 0;
			std::string rating_summary;
			std::string issues_summary;
			std::vector<std::string> issues;
		};

		password_quality cui_api password_rating(const std::string& username,
			const std::string& password);

		std::string cui_api unique_string();
		std::string cui_api unique_string_short();

		namespace date_gen {
			struct date {
				int day = 1;
				int month = 1;
				int year = 2019;
			};

			/// <summary>
			/// Make timestamp from current local time in the form 20190802 20:48:06
			/// </summary>
			std::string cui_api time_stamp();

			std::string cui_api to_string(const date& dt);
			date cui_api from_string(const std::string& dt);
			date cui_api today();
			date cui_api last_day_of_month(const date& dt);
			int cui_api day_of_week(date dt);
			void cui_api add_days(date& dt, int n);
			void cui_api get_week(const date& dt, date& start, date& end);
			void cui_api get_month(const date& dt, date& start, date& end);
			void cui_api get_year(const date& dt, date& start, date& end);
		}

		struct file_type
		{
			std::string extension = "bmp";
			std::string description = "Bitmap Files";
		};

		struct open_file_params
		{
			std::vector<file_type> file_types;
			bool include_all_supported_types = true;
			std::string title = "Open File";
		};

		struct save_file_params
		{
			std::vector<file_type> file_types;
			bool include_all_supported_types = true;
			std::string title = "Save File";
		};

		struct list_column
		{
			std::string name;
			unsigned short width = 80;
		};

		/// <summary>
		/// Keep UI responsive during lengthy calls.
		/// </summary>
		/// 
		/// <returns>
		/// Returns true to continue operation, and false to quit immediately.
		/// </returns>
		/// 
		/// <remarks>
		/// In lengthy operations it is useful to call this method between successive
		/// steps in order to keep the UI responsive.
		/// </remarks>
		bool cui_api keep_alive();
	}
}
