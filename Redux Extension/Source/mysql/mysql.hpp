/* mysql.hpp
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

#ifndef SOURCE_MYSQL_HPP_
#define SOURCE_MYSQL_HPP_

#include <string>
#include <map>
#include <tuple>
#include <cstdint>
#include <mutex>
#include <vector>
#include <queue>
#include <boost/property_tree/ptree.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread/mutex.hpp>
#include <mysql.h>
#if LIBMYSQL_VERSION_ID > 80000
	#define my_bool bool
#endif

#include "extbase.hpp"
#include "constants.hpp"
#include "mysql/datacache/objectmysql.hpp"
#include "mysql/datacache/charactermysql.hpp"

#include <stdio.h>


class mysql_db_handler: public ext_base {
	// https://dev.mysql.com/doc/connector-cpp/en/connector-cpp-examples-complete-example-1.html
public:
	mysql_db_handler(EXT_FUNCTIONS &extFunctions);
	~mysql_db_handler();

	std::string spawnHandler(std::string &extFunction, ext_arguments &extArgument);
	void terminateHandler();
	std::string terminateHandler(std::string &extFunction, ext_arguments &extArgument);

private:
		boost::lockfree::queue<intptr_t, boost::lockfree::fixed_sized<false>> connectionpool{1};

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



		bool poolinitialized = false;
		bool poolcleanup = false;

		boost::mutex dbmutex;

		std::map<std::string, object_mysql *> objectcache;
		std::map<std::string, character_mysql *> charactercache;


		MYSQL * connect();
		void disconnect();
		MYSQL * getconnection();
		void returnconnection(MYSQL * connection);

		std::string mysqlResultToArrayString(MYSQL_RES *result, char *typearray);

		std::string intergetUUID(std::string &extFunction, ext_arguments &extArgument);
		std::string interecho(std::string &extFunction, ext_arguments &extArgument);

		std::string interdbVersion(std::string &extFunction, ext_arguments &extArgument);

		void rawmultiquerys(std::string query);
		void rawquery(std::string query);
		void rawquery(std::string query, MYSQL_RES **result);

		std::string querydbversion();

		void checkMigration(ext_arguments &extArgument, std::string database);
		void checkWorldUUID(ext_arguments &extArgument);
		void checkWorldVARIABLES(ext_arguments &extArgument);


		std::string setWorldVariables(std::string &extFunction, ext_arguments &extArgument);
		std::string getWorldVariables(std::string &extFunction, ext_arguments &extArgument);
		std::string setWorldState(std::string &extFunction, ext_arguments &extArgument);
		std::string getLinkedWorlds(std::string &extFunction, ext_arguments &extArgument);
		std::string whitelistPlayer(std::string &extFunction, ext_arguments &extArgument);
		std::string unwhitelistPlayer(std::string &extFunction, ext_arguments &extArgument);

		std::string loadPlayer(std::string &extFunction, ext_arguments &extArgument);
		std::string logoutPlayer(std::string &extFunction, ext_arguments &extArgument);
		std::string incrementPlayerStats(std::string &extFunction, ext_arguments &extArgument);
		std::string loadPlayerGroups(std::string &extFunction, ext_arguments &extArgument);
		std::string updatePlayerMainClan(std::string &extFunction, ext_arguments &extArgument);
		std::string loadAvChars(std::string &extFunction, ext_arguments &extArgument);
		std::string linkChars(std::string &extFunction, ext_arguments &extArgument);
		std::string loadChar(std::string &extFunction, ext_arguments &extArgument);
		std::string createChar(std::string &extFunction, ext_arguments &extArgument);
		std::string updateChar(std::string &extFunction, ext_arguments &extArgument);
		std::string killChar(std::string &extFunction, ext_arguments &extArgument);

		std::string loadObject(std::string &extFunction, ext_arguments &extArgument);
		std::string createObject(std::string &extFunction, ext_arguments &extArgument);
		std::string updateObject(std::string &extFunction, ext_arguments &extArgument);
		std::string killObject(std::string &extFunction, ext_arguments &extArgument);

	    std::string interdumpObjects(std::string &extFunction, ext_arguments &extArgument);
		std::string interdumpObjectsCallback(std::string &extFunction, ext_arguments &extArgument);
		std::vector<object_mysql *> dumpObjects(ext_arguments &extArgument);

		std::string createObjectWorldLink(std::string &extFunction, ext_arguments &extArgument);
		std::string updateObjectWorldLink(std::string &extFunction, ext_arguments &extArgument);
		std::string deleteObjectWorldLink(std::string &extFunction, ext_arguments &extArgument);

		void failIfClanNotExists(std::string clanuuid);
		void failIfClanNameNotUnique(std::string clanname);
		bool playerMemberOfClan(std::string clanuuid, std::string playeruuid);

		std::string getClan(std::string &extFunction, ext_arguments &extArgument);
		std::string createClan(std::string &extFunction, ext_arguments &extArgument);
		std::string updateClan(std::string &extFunction, ext_arguments &extArgument);
		std::string addClanPlayer(std::string &extFunction, ext_arguments &extArgument);
		std::string updateClanPlayer(std::string &extFunction, ext_arguments &extArgument);
		std::string deleteClanPlayer(std::string &extFunction, ext_arguments &extArgument);




};

#endif /* SOURCE_MYSQL_HPP_ */
