/*
** unique_string.cpp - unique string implementation
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

// for UuidToString, RpcStringFree
#include <rpc.h>
#pragma comment(lib, "rpcrt4.lib")

std::string cui_api liblec::cui::unique_string() {
	std::string sUUID;

	UUID uuid;
	UuidCreate(&uuid);
	RPC_CSTR uuidStr;
	RPC_STATUS status = UuidToStringA(&uuid, &uuidStr);
	sUUID += reinterpret_cast<char*>(uuidStr);
	RpcStringFreeA(&uuidStr);

	return sUUID;
}

std::string cui_api liblec::cui::unique_string_short() {
	std::string uuid = unique_string();

	const auto idx = uuid.find('-');

	if (idx != std::string::npos) {
		return uuid.substr(0, idx);
	}
	else
		return uuid;
}
