//
// win_error.h - windows errors interface
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#pragma once

#include <string>

/// <summary>
/// Get Windows last error as a string.
/// </summary>
std::string get_last_error();
