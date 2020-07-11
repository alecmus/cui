/*
** timestamp.cpp - time related operations - implementation
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

#include "../cui.h"
#include <ctime>

namespace liblec::cui::date_gen {
	std::string time_stamp() {
		std::time_t time_temp = time(0);
		std::tm time_out = { };
		localtime_s(&time_out, &time_temp);

		std::string year = std::to_string(time_out.tm_year + 1900);
		std::string month = std::to_string(time_out.tm_mon + 1);
		std::string day = std::to_string(time_out.tm_mday);
		std::string hour = std::to_string(time_out.tm_hour);
		std::string minute = std::to_string(time_out.tm_min);
		std::string second = std::to_string(time_out.tm_sec);

		for (size_t i = year.length(); i < 4; i++) year = "0" + year;
		for (size_t i = month.length(); i < 2; i++) month = "0" + month;
		for (size_t i = day.length(); i < 2; i++) day = "0" + day;
		for (size_t i = hour.length(); i < 2; i++) hour = "0" + hour;
		for (size_t i = minute.length(); i < 2; i++) minute = "0" + minute;
		for (size_t i = second.length(); i < 2; i++) second = "0" + second;

		return std::string(year + month + day + " " + hour + ":" + minute + ":" + second);
	}

	std::string to_string(const date& dt) {
		auto day = std::to_string(dt.day);
		auto month = std::to_string(dt.month);
		auto year = std::to_string(dt.year);

		for (size_t i = day.length(); i < 2; i++) day = "0" + day;
		for (size_t i = month.length(); i < 2; i++) month = "0" + month;
		for (size_t i = year.length(); i < 4; i++) year = "0" + year;

		return (year + month + day);
	}

	date from_string(const std::string& dt) {
		date dt_ = { 1, 1, 2019 };
		try {
			dt_.day = atoi(dt.substr(6, 2).c_str());
			dt_.month = atoi(dt.substr(4, 2).c_str());
			dt_.year = atoi(dt.substr(0, 4).c_str());
		}
		catch (const std::exception&) {}
		return dt_;
	}

	date today() {
		std::time_t time_temp = time(0);
		std::tm time_out = { };
		localtime_s(&time_out, &time_temp);

		date dt;
		dt.day = time_out.tm_mday;
		dt.month = time_out.tm_mon + 1;
		dt.year = time_out.tm_year + 1900;
		return dt;
	}

	date last_day_of_month(const date& dt) {
		struct tm when = { 0, 0, 0, 1 };

		if (dt.month == 12) {
			when.tm_mon = 0;
			when.tm_year = dt.year - 1900 + 1;
		}
		else {
			when.tm_mon = dt.month;
			when.tm_year = dt.year - 1900;
		}

		// get the first day of the next month and subtract one day
		const time_t lastday = mktime(&when) - 86400;
		localtime_s(&when, &lastday);
		return date{ when.tm_mday, when.tm_mon + 1, when.tm_year + 1900 };
	}

	int day_of_week(date dt) {
		std::tm time_in = { 0, 0, 0, dt.day, dt.month - 1, dt.year - 1900 };
		std::time_t time_temp = std::mktime(&time_in);
		std::tm time_out = { };
		localtime_s(&time_out, &time_temp);
		return time_out.tm_wday + 1;
	}

	void add_days(date& dt, int n) {
		std::tm time_in = { 0, 0, 0, dt.day, dt.month - 1, dt.year - 1900 };
		time_in.tm_mday += n;

		std::time_t time_temp = std::mktime(&time_in);
		std::tm time_out = { };
		localtime_s(&time_out, &time_temp);

		dt.day = time_out.tm_mday;
		dt.month = time_out.tm_mon + 1;
		dt.year = time_out.tm_year + 1900;
	}

	void get_week(const date& dt, date& start, date& end) {
		start = end = dt;
		const auto d = day_of_week(dt);
		add_days(start, 0 - d);
		add_days(end, 6 - d);
	}

	void get_month(const date& dt, date& start, date& end) {
		start = end = dt;
		const auto d = dt.day;
		add_days(start, 1 - d);
		end = last_day_of_month(dt);
	}

	void get_year(const date& dt, date& start, date& end) {
		start = { 1, 1, dt.year };
		end = { 31, 12, dt.year };
	}
}
