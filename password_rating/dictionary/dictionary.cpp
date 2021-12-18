//
// dictionary.cpp - dictionary implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#define _GNU_SOURCE

#include "../../resources/dictionary_resource.h"
#include "dictionary.h"

#include <Shlwapi.h>				// for StrStrA
#pragma comment(lib, "Shlwapi.lib")

#include <cctype>					// for tolower

const char* dictionary::dict_buffer = NULL;

/// load dictionary file into buffer
/// this function will only load the dictionary if the buffer is found to be empty
void dictionary::LoadDict()
{
	if (dict_buffer == NULL)
	{
		DWORD size = 0;

		// load dictionary text file in resource
		LoadFileInResource(IDTXT_DICT, TEXTFILE, size, dict_buffer);

		if (size == 0 || dict_buffer == NULL) {
			dict_buffer = NULL;
		}
	}
	else {
		// dictionary already in memory
	}

	return;
}

int searchforword(const char* ccSource, const char* ccToFind, bool bCasesensitive) {
	if (bCasesensitive == true) {
		// search for word in dictionary (case sensitive)
		if (strstr(ccSource, ccToFind) != NULL) return 1;
	}
	else {
		// search for word in dictionary (case insensitive)
		std::string m_sToFind(ccToFind);

		// check all lowercase
		for (int i = 0; i < (int)m_sToFind.length(); i++)
			m_sToFind[i] = tolower(m_sToFind[i]);

		if (strstr(ccSource, m_sToFind.c_str()) != NULL)
			return 1;

		// check first letter uppercase
		for (int i = 0; i < (int)m_sToFind.length(); i++)
		{
			m_sToFind[0] = toupper(m_sToFind[0]);
			break;
		}

		if (strstr(ccSource, m_sToFind.c_str()) != NULL)
			return 1;
	}

	return 0;
}

/// search word in dictionary file
/// returns 0 if word is NOT found in the file
/// returns 1 if word is found
/// returns -1 if dictionary has not been loaded into memory
int dictionary::SearchInDict(const std::wstring& sWord, bool bCasesensitive) {
	if (dict_buffer == NULL) return -1;

	std::string m_sWord = std::string(sWord.begin(), sWord.end());

	int iRes = 0;

	// check if last character is an 's' and cut it off to check for plurals
	if (m_sWord.length() > 1 && tolower(m_sWord[m_sWord.length() - 1]) == 's') {
		std::string m_sWordToSearch = std::string(m_sWord.begin(), m_sWord.end() - 1);

		iRes = searchforword(dict_buffer, m_sWordToSearch.c_str(), bCasesensitive);

		if (iRes == 1)
			return 1;
		else
			return 0;
	}

	// check word as-is
	iRes = searchforword(dict_buffer, m_sWord.c_str(), bCasesensitive);

	if (iRes == 1) return 1;

	return 0;
}

void dictionary::LoadFileInResource(int name, int type, DWORD& size, const char*& data) {
	HMODULE handle = NULL;

	GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
		(LPCTSTR)& LoadFileInResource,	/* use the address of the current function to detect the module handle of the current application */
		&handle);

	HRSRC rc = FindResource(handle, MAKEINTRESOURCE(name), MAKEINTRESOURCE(type));

	HGLOBAL rcData = LoadResource(handle, rc);

	size = SizeofResource(handle, rc);

	data = static_cast<const char*>(LockResource(rcData));
}
