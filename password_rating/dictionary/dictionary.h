//
// dictionary.h - dictionary interface
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#pragma once

#include <Windows.h>
#include <tchar.h>
#include <string>

// dictionary class - definition
class dictionary {
	static const char* dict_buffer;
	static void LoadFileInResource(int name, int type, DWORD& size, const char*& data);

public:
	dictionary();
	~dictionary();

	static void LoadDict();
	static int SearchInDict(const std::wstring& sWord, bool bCasesensitive);
};
