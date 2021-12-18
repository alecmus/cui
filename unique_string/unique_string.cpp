//
// unique_string.cpp - unique string implementation
//
// cui framework, part of the liblec library
// Copyright (c) 2016 Alec Musasa (alecmus at live dot com)
//
// Released under the MIT license. For full details see the
// file LICENSE.txt
//

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
