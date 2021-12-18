//
// RichEdit.h - rich edit control interface
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

/*
** Rich edit control things
** Code portions from:
** http://www.sharewareconnection.com/richedit.htm
*/

#pragma once

#include <Windows.h>
#include <richedit.h>
#include <fstream>
#include <string>

#define StreamType_File 1
#define StreamType_Buffer 2
#define StreamType_NA 0

// to-do: seriously? eliminate this global variable!
int StreamType = StreamType_NA;

void SetStreamType(int st)
{
	//1 = file, 2 = buffer
	//StreamType_File,  StreamType_Buffer
	StreamType = st;
}

int GetStreamType(void)
{
	return StreamType;
}

/// <summary>
/// 
/// </summary>
/// 
/// <param name="dwCookie">
/// application-defined value (i use this to pass file handle or TCHAR buffer address)
/// </param>
/// 
/// <param name="pbBuff">
/// pointer to a buffer (you write/read to/from this)
/// </param>
/// 
/// <param name="cb">
/// number of bytes to read or write (# of bytes it will allow you to read/write)
/// </param>
/// 
/// <param name="pcb">
/// pointer to number of bytes transferred
/// </param>
/// 
/// <returns></returns>
static DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb)
{
	if (StreamType == StreamType_NA)
		return 0;
	else
		if (StreamType == StreamType_File)	//were we passed a file object?
		{
			std::fstream* fpntr = (std::fstream*)dwCookie;	//cast it as a file object

			fpntr->read((char*)pbBuff, (unsigned int)cb);	//read data

			//tells windows when to stop calling the callback function
			*pcb = (long)fpntr->gcount();
		}
		else
			if (StreamType == StreamType_Buffer)	//were we passed a string object?
			{
				std::string* writes = (std::string*)dwCookie;	//cast as string

				if (writes->size() < (size_t)cb)	//check if we are allowed to write rest or not
				{//since the size of the string is less then what were allowed
					*pcb = (long)writes->size();	//set size to string size
					memcpy(pbBuff, (void*)writes->data(), *pcb);	//write rest
					writes->erase();	//erase whole string
				}
				else
				{	//string is too big to write
					*pcb = cb;	//set size to allowed transfer size
					memcpy(pbBuff, (void*)writes->data(), *pcb);	//write allowed amount
					writes->erase(0, cb);	//erase only what we wrote
				}
			}

	return 0;
}

static DWORD CALLBACK EditStreamCallbackRead(DWORD_PTR dwCookie,
	LPBYTE pbBuff, LONG cb, LONG* pcb)
{
	std::string* reads = (std::string*)dwCookie;	//here we read into a string
													//try changing this to write to a file

	if (cb > 0)	//as long as theres something to get
	{
		*pcb = cb;	//set to size of what we can get
		reads->append((char*)pbBuff, cb);	//get
	}

	return 0;
}

DWORD CALLBACK EditStreamOutCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb)
{
	HANDLE hFile = (HANDLE)dwCookie;
	DWORD NumberOfBytesWritten;
	if (!WriteFile(hFile, pbBuff, cb, &NumberOfBytesWritten, NULL))
	{
		//handle errors
		return 1;
		// or perhaps return GetLastError();
	}

	*pcb = NumberOfBytesWritten;
	return 0;
}

void SaveRichTextToFile(HWND hWnd, LPCTSTR filename)
{
	HANDLE hFile = CreateFile(filename, GENERIC_WRITE, 0, NULL,
		CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile == INVALID_HANDLE_VALUE) { /* handle errors */ }
	else
	{
		EDITSTREAM es = { 0 };
		es.dwCookie = (DWORD_PTR)hFile;
		es.pfnCallback = EditStreamOutCallback;
		SendMessage(hWnd, EM_STREAMOUT, SF_RTF, (LPARAM)&es);
		CloseHandle(hFile);

		if (es.dwError != 0) { /* handle errors */ }
	}
}
