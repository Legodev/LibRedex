/* mysql.hpp
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

#ifndef SOURCE_MYSQL_HPP_
#define SOURCE_MYSQL_HPP_

#include <string>
#include <mysql.h>
#include <map>
#include <queue>
#include <vector>
#include "constants.hpp"
#include "database/base.hpp"

class mysql_db_handler: virtual public base_db_handler {
	// https://dev.mysql.com/doc/connector-cpp/en/connector-cpp-examples-complete-example-1.html
public:
	mysql_db_handler();
	~mysql_db_handler();

	void connect(std::string hostname, std::string user, std::string password, std::string database, unsigned int port,
			bool whitelistonly, bool allowsteamapi, bool vaccheckban, unsigned int vacmaxcount,
			unsigned int vacignoredays, std::string worlduuid);
	void disconnect();
	void rawquery(std::string query);
	void rawquery(std::string query, MYSQL_RES **result);

	std::string querydbversion();

	void checkWorldUUID();
	std::string loadPlayer(std::string nickname, std::string steamid);
	std::string loadAvChars(std::string playeruuid);
	std::string linkChars(std::string playeruuid, std::string variabuuid);
	cache_base* loadChar(std::map<std::string, cache_base*> &charactercache, std::string playeruuid);
	std::string createChar(std::map<std::string, cache_base*> &charactercache, ext_arguments &extArgument);
	std::string updateChar(std::map<std::string, cache_base*> &charactercache, ext_arguments &extArgument);
	std::string killChar(std::string charuuid, std::string attackeruuid, std::string type, std::string weapon, float distance);

	std::string loadObject(std::map<std::string, cache_base*> &objectcache, ext_arguments &extArgument);
	std::string createObject(std::map<std::string, cache_base*> &objectcache, ext_arguments &extArgument);
	std::string updateObject(std::map<std::string, cache_base*> &objectcache, ext_arguments &extArgument);
	std::string killObject(std::string objectuuid, std::string attackeruuid, std::string type, std::string weapon, float distance);
	std::vector<cache_base*> dumpObjects(std::map<std::string, cache_base*> &objectcache);

private:
		MYSQL *connection;
		std::string hostname;
		std::string user;
		std::string password;
		std::string database;
		unsigned int port;
		std::string socket;
		unsigned long int flag;
		my_bool reconnect = 1;

		bool whitelistonly;
		bool allowsteamapi;
		bool vaccheckban;

		unsigned int vacmaxcount;
		unsigned int vacignoredays;

		std::string worlduuid;

		void preparedStatementQuery(std::string query, MYSQL_BIND input_params[]);
		void test_stmt_error(MYSQL_STMT *stmt, int status);

};

#endif /* SOURCE_MYSQL_HPP_ */
