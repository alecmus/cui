/*
** limit_single_instance.h - limit application to a single instance - interface
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

#include <string>

/// <summary>
/// Limit app to a single instance.
/// </summary>
class limit_single_instance
{
public:
	/// <summary>
	/// Constructor.
	/// </summary>
	/// 
	/// <param name="mutex_name">
	/// Unique guid used to prevent multiple instances of the app,
	/// e.g. "Global\\{14434072-3695-4BD7-8CD5-ECFCDD571FA4}"
	/// </param>
	limit_single_instance(const std::string &mutex_name);
	~limit_single_instance();

	/// <summary>
	/// Check if another instance is running.
	/// </summary>
	/// 
	/// <returns>
	/// Returns true if another instance is running, else false.
	/// </returns>
	bool another_instance_running();

	/// <summary>
	/// Wait for the other instance. This function will block until the other instance is closed.
	/// </summary>
	void wait_for_other_instance();
private:
	limit_single_instance();

	class limit_single_instance_impl;
	limit_single_instance_impl* d_;
}; // limit_single_instance
