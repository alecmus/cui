//
// cui.cpp - cui framework implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

#include "cui.h"
#include "versioninfo.h"

#include <Windows.h>

// DllMain function
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;

	case DLL_THREAD_ATTACH:
		break;

	case DLL_THREAD_DETACH:
		break;

	case DLL_PROCESS_DETACH:
		break;

	default:
		break;
	}
	return TRUE;
} // DllMain

std::string liblec::cui::version()
{
	return cuiname + std::string(" ") + cuiversion + std::string(", ") + cuidate;
} // version

bool cui_api liblec::cui::keep_alive() {
	MSG msg = {};

	if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
		if (msg.message == WM_QUIT) {
			PostQuitMessage(0);
			return false;
		}
		else {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return true;
}
