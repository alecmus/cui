/*
** HlpFxs.h - helper functions
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

#include <Windows.h>
#include <tchar.h>
#include <string>

/// <summary>
/// Rounding off class.
/// </summary>
class CRoundOff
{
public:
	/// <summary>
	/// Round-off a double to a string.
	/// </summary>
	/// 
	/// <param name="d">
	/// The double to round-off.
	/// </param>
	/// 
	/// <param name="iDecimalPlaces">
	/// The number of decimal places to round it off to.
	/// </param>
	/// 
	/// <returns>
	/// The rounded-off value, as a string.
	/// </returns>
	template <typename T>
	static std::basic_string<T> tostr(const double &d, int iDecimalPlaces)
	{
		std::basic_stringstream<T> ss;
		ss << std::fixed;
		ss.precision(iDecimalPlaces);
		ss << d;
		return ss.str();
	} // tostr

	  /// <summary>
	  /// Round-off a double to another double.
	  /// </summary>
	  /// 
	  /// <param name="d">
	  /// The double to round-off.
	  /// </param>
	  /// 
	  /// <param name="iDecimalPlaces">
	  /// The number of decimal places to round it off to.
	  /// </param>
	  /// 
	  /// <returns>
	  /// The rounded-off value.
	  /// </returns>
	static double tod(const double &d, int iDecimalPlaces)
	{
		int y = (int)d;
		double z = d - (double)y;
		double m = pow(10, iDecimalPlaces);
		double q = z * m;
		double r = round(q);

		return static_cast<double>(y) + (1.0 / m) * r;
	} // tod
}; // CRoundOff

