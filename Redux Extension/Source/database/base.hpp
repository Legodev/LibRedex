/* base.hpp
 *
 * Copyright 2016-2018 Desolation Redux
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

#ifndef SOURCE_BASE_HPP_
#define SOURCE_BASE_HPP_

#include <string>
#include <map>
#include <queue>
#include <vector>
#include "constants.hpp"
#include "database/datacache/objectmysql.hpp"

class base_db_handler {
/*
protected:
	base_db_handler() { };
*/

public:
	virtual ~base_db_handler() { };

	virtual void connect(std::string hostname, std::string user, std::string password, std::string database,
			unsigned int port, bool whitelistonly, bool allowsteamapi, bool vaccheckban, unsigned int vacmaxcount,
			unsigned int vacignoredays, std::string worlduuid) {};
	virtual void disconnect() {};

	virtual std::string querydbversion() { return "none"; }
	virtual std::vector<std::vector<std::string> > verbosetest(std::string query) { return {{"none"}}; };

	virtual void checkWorldUUID() {};
	virtual std::string loadPlayer(std::string nickname, std::string steamid) { return "none"; };
	virtual std::string loadAvChars(std::string playeruuid) { return "none"; }
	virtual std::string linkChars(std::string playeruuid, std::string variabuuid) { return "none"; }
	virtual cache_base* loadChar(std::map<std::string, cache_base*> &charactercache, std::string playeruuid) { return NULL; };

	virtual std::string loadChar(std::string playeruuid) { return "none"; }
	virtual std::string createChar(std::map<std::string, cache_base*> &charactercache, ext_arguments &extArgument) { return "none"; }
	virtual std::string updateChar(std::map<std::string, cache_base*> &charactercache, ext_arguments &extArgument) { return "none"; }
	virtual std::string killChar(std::string charuuid, std::string attackeruuid, std::string type, std::string weapon,
			float distance) { return "none"; }

	virtual std::string loadObject(std::string objectuuid) { return "none"; }
	virtual std::string createObject(std::map<std::string, cache_base*> &objectcache, ext_arguments &extArgument) { return "none"; }
	virtual std::string updateObject(std::map<std::string, cache_base*> &objectcache, ext_arguments &extArgument) { return "none"; }
	virtual std::string killObject(std::string objectuuid, std::string attackeruuid, std::string type,
			std::string weapon, float distance) { return "none"; }
	virtual std::vector<cache_base*> dumpObjects(std::map<std::string, cache_base*> &objectcache) { return std::vector<cache_base*>(); };
};

#endif /* SOURCE_BASE_HPP_ */
