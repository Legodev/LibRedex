/* uuid.cpp
 *
 * Copyright 2016-2020 Desolation Redux
 *
 * Author: Legodev <legodevgit@mailbox.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 */

#include <sstream>
#include "uuid.hpp"

#ifdef __linux__
#include <uuid/uuid.h>

std::string generateUUID() {
	uuid_t id;
	uuid_generate_time(id);

	char uuidstring[37];
	uuid_unparse(id, uuidstring);

	return uuidstring;
}
#else
#include <rpc.h>
#pragma comment(lib, "rpcrt4.lib")

std::string generateUUID() {
	UUID uuid;
	UuidCreateSequential( &uuid );

	unsigned char* wszUuid = NULL;

	UuidToString(&uuid, &wszUuid);

	std::string uuidstring = (const char*)wszUuid;

	RpcStringFree(&wszUuid);
	wszUuid = NULL;

	return uuidstring;
}

#endif

std::string orderedUUID() {
	std::stringstream orderedUUID;
	std::string UUID = generateUUID();
	orderedUUID << UUID.substr(14, 4) << UUID.substr(9, 4) << UUID.substr(0, 8)
			<< UUID.substr(19, 4) << UUID.substr(24);
	return orderedUUID.str();
}
