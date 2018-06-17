/* mysql.cpp
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

#include <cassert>
#include <exception>
#include <stdexcept>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/format.hpp>

#include "mysql/mysql.hpp"
#include "utils/uuid.hpp"
#include "mysql/datacache/charactermysql.hpp"
#include "mysql/datacache/objectmysql.hpp"

#ifdef DEBUG
	#include <fstream>
	#include <iostream>
	#include <sstream>
	extern std::ofstream testfile;
#endif

extern std::map<std::string, unsigned int> * objectvariablemap;
std::map<std::string, unsigned int> * objectvariablemap = 0;
extern std::map<std::string, unsigned int> * charactervariablemap;
std::map<std::string, unsigned int> * charactervariablemap = 0;

mysql_db_handler::mysql_db_handler(EXT_FUNCTIONS &extFunctions) {
	this->hostname = "";
	this->user = "";
	this->password = "";
	this->database = "";
	this->port = 0;
	this->socket = "";
	this->flag = 0;

	this->whitelistonly = false;
	this->allowsteamapi = false;
	this->vaccheckban = false;

	this->vacmaxcount = 0;
	this->vacignoredays = 0;

	this->worlduuid = "";


	if (charactervariablemap == 0) {
		charactervariablemap = new std::map<std::string, unsigned int>;
	}

	if (objectvariablemap == 0) {
		objectvariablemap = new std::map<std::string, unsigned int>;
	}

	extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_EXECUTE_INIT_DB),
						std::make_tuple(
								boost::bind(&mysql_db_handler::spawnHandler, this, _1, _2),
								SYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_EXECUTE_TERMINATE_DB),
						std::make_tuple(
								boost::bind(&mysql_db_handler::terminateHandler, this, _1, _2),
								SYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_RETURN_UUID),
						std::make_tuple(
								boost::bind(&mysql_db_handler::intergetUUID, this, _1, _2),
								SYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_RETURN_ECHO_STRING),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interecho, this, _1, _2),
								SYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_RETURN_DB_VERSION),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interdbVersion, this, _1, _2),
								SYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_PLAYER),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interloadPlayer, this, _1, _2),
								ASYNC_MAGIC)));
		extFunctions.insert(
						std::make_pair(
								std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_PLAYER_GROUPS),
								std::make_tuple(
										boost::bind(&mysql_db_handler::interloadPlayerGroups, this, _1, _2),
										ASYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_AV_CHARS),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interloadAvChars, this, _1, _2),
								ASYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_LINK_CHARS),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interlinkChars, this, _1, _2),
								ASYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_CHAR),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interloadChar, this, _1, _2),
								ASYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_CREATE_CHAR),
						std::make_tuple(
								boost::bind(&mysql_db_handler::intercreateChar, this, _1, _2),
								ASYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_UPDATE_CHAR),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interupdateChar, this, _1, _2),
								ASYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_QUIET_UPDATE_CHAR),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interupdateChar, this, _1, _2),
								QUIET_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_DECLARE_CHAR_DEATH),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interkillChar, this, _1, _2),
								ASYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_OBJECT),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interloadObject, this, _1, _2),
								ASYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_CREATE_OBJECT),
						std::make_tuple(
								boost::bind(&mysql_db_handler::intercreateObject, this, _1, _2),
								ASYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_QUIET_CREATE_OBJECT),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interqcreateObject, this, _1, _2),
								QUIET_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_UPDATE_OBJECT),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interupdateObject, this, _1, _2),
								ASYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_QUIET_UPDATE_OBJECT),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interupdateObject, this, _1, _2),
								QUIET_MAGIC)));
		extFunctions.insert(
				std::make_pair(
						std::string(PROTOCOL_DBCALL_FUNCTION_DECLARE_OBJECT_DEATH),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interkillObject, this, _1, _2),
								ASYNC_MAGIC)));
		extFunctions.insert(
				std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_DUMP_OBJECTS),
						std::make_tuple(
								boost::bind(&mysql_db_handler::interdumpObjects, this, _1, _2),
								ASYNC_MAGIC)));
}

mysql_db_handler::~mysql_db_handler() {
	disconnect();

	for (auto it = objectcache.cbegin(); it != objectcache.cend(); ++it)
	{
		object_mysql * object = it->second;
		delete object;
		objectcache.erase(it);
	}

	for (auto it = charactercache.cbegin(); it != charactercache.cend(); ++it)
	{
		character_mysql * character = it->second;
		delete character;
		charactercache.erase(it);
	}

    if (charactervariablemap != 0) {
        delete charactervariablemap;
        charactervariablemap = 0;
    }

    if (objectvariablemap != 0) {
        delete objectvariablemap;
        objectvariablemap = 0;
    }

	return;
}

std::string mysql_db_handler::spawnHandler(std::string &extFunction, ext_arguments &extArgument) {
	std::string worlduuid = extArgument.getUUID("worlduuid");
	int i = 0;

	if (!poolinitialized) {
		std::string type;
		std::string hostname;
		std::string user;
		std::string password;
		std::string database;
		unsigned int port = 0;

		bool whitelistonly;
		bool allowsteamapi;
		bool vaccheckban;
		unsigned int vacmaxcount;
		unsigned int vacignoredays;

		boost::property_tree::ptree configtree;
		boost::property_tree::json_parser::read_json(CONFIG_FILE_NAME, configtree);

		unsigned int poolsize = configtree.get<unsigned int>("gamesettings.poolsize");

		if (poolsize < 1) {
			poolsize = 1;
		}

		type = configtree.get<std::string>("database.type");
		hostname = configtree.get<std::string>("database.hostname");
		user = configtree.get<std::string>("database.user");
		password = configtree.get<std::string>("database.password");
		database = configtree.get<std::string>("database.dbname");
		port = configtree.get<unsigned int>("database.port");

		whitelistonly = configtree.get<bool>("gamesettings.whitelistonly");
		allowsteamapi = configtree.get<bool>("gamesettings.allowsteamapi");

		vaccheckban = configtree.get<bool>("gamesettings.vaccheckban");
		vacmaxcount = configtree.get<unsigned int>("gamesettings.vacmaxcount");
		vacignoredays = configtree.get<unsigned int>("gamesettings.vacignoredays");

		this->hostname = hostname;
		this->user = user;
		this->password = password;
		this->database = database;
		this->port = port;
	//	this->socket = NULL;
	//	this->flag = 0;

		this->whitelistonly = whitelistonly;
		this->allowsteamapi = allowsteamapi;
		this->vaccheckban = vaccheckban;

		this->vacmaxcount = vacmaxcount;
		this->vacignoredays = vacignoredays;

		this->worlduuid = worlduuid;

		if (!connectionpool.is_lock_free()) {
			throw std::runtime_error("connectionpool is not lock free");
		}

		/* for the beginning we want to have some spare pool handlers */
		connectionpool.reserve(poolsize + 3);

		for (i = 0; i <= poolsize; i++) {
			MYSQL * connection = connect();
#ifdef DEBUG
			testfile << "CREATED NEW DB POINTER " << static_cast<void*>(connection) << std::endl;
			testfile.flush();
#endif

			connectionpool.bounded_push((intptr_t) connection);

			if (i == 0) {
				checkWorldUUID();
			}
		}

		poolinitialized = true;
	} else {
		throw std::runtime_error("Threads already spawned");
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\", " + std::to_string(i) + "]";
}

MYSQL * mysql_db_handler::connect() {
	MYSQL * connection = mysql_init(NULL);

	if (connection == NULL) {
		throw std::runtime_error("problem while initializing mysql_db_handler: " + std::string(mysql_error(connection)));
	}

	mysql_options(connection, MYSQL_OPT_RECONNECT, &reconnect);

	if (mysql_real_connect(connection, this->hostname.c_str(),
			this->user.c_str(), this->password.c_str(), this->database.c_str(),
			this->port, this->socket.c_str(), this->flag) == NULL) {
		mysql_close(connection);
		throw std::runtime_error("connection problem while initializing mysql_db_handler: "	+ std::string(mysql_error(connection)));
	}

	return connection;
}

void mysql_db_handler::disconnect() {
	intptr_t connectionpointer;
	MYSQL *connection;

	// prevent new requests
	poolcleanup = true;

	// while there is an handler call disconnect
	while (connectionpool.pop(connectionpointer)) {
		connection = (MYSQL *) connectionpointer;
		mysql_close(connection);
	}

	poolcleanup = false;
	poolinitialized = false;
	return;
}

MYSQL * mysql_db_handler::getconnection() {
	intptr_t connectionpointer;
	MYSQL *connection = 0;

	// try to get an db connection
	while (!connectionpool.pop(connectionpointer));
	connection = (MYSQL *) connectionpointer;

#ifdef DEBUG
			testfile << "GOT DB POINTER " << static_cast<void*>(connection) << std::endl;
			testfile.flush();
#endif

	if (connection == 0) {
		std::runtime_error("connection problem while getting mysql_db_handler: got a NULLPOINTER");
	}

	return connection;
}

void mysql_db_handler::returnconnection(MYSQL * connection) {
	intptr_t connectionpointer = (intptr_t) connection;
	connectionpool.bounded_push(connectionpointer);

#ifdef DEBUG
			testfile << "RETURNED DB POINTER " << static_cast<void*>(connection) << std::endl;
			testfile.flush();
#endif

	return;
}

void mysql_db_handler::rawquery(std::string query) {
	MYSQL *connection = getconnection();

#ifdef DEBUG
			testfile << "QUERY: " << query << std::endl;
			testfile.flush();
#endif

	if (mysql_real_query(connection, query.c_str(), query.size())) {
		throw std::runtime_error(
				"error while executing query: "
						+ query
						+ std::string(" ---- ERROR MESSAGE: ")
						+ std::string(mysql_error(connection)));
	}

	returnconnection(connection);
	return;
}

void mysql_db_handler::rawquery(std::string query, MYSQL_RES **result) {
	MYSQL *connection = getconnection();

#ifdef DEBUG
			testfile << "QUERY " << query << std::endl;
			testfile.flush();
#endif

	if (mysql_real_query(connection, query.c_str(), query.size())) {
		throw std::runtime_error(
				"error while executing query: "
						+ query
						+ std::string(" ---- ERROR MESSAGE: ")
						+ std::string(mysql_error(connection)));
	}

	*result = mysql_store_result(connection);

	if (*result == NULL) {
		throw std::runtime_error(
				"error while getting the result: "
						+ query
						+ std::string(" ---- ERROR MESSAGE: ")
						+ std::string(mysql_error(connection)));
	}

	returnconnection(connection);
	return;
}

void mysql_db_handler::preparedStatementQuery(std::string query, MYSQL_BIND input_params[]) {
	MYSQL *connection = getconnection();
	int        status;

#ifdef DEBUG
			testfile << "QUERY " << query << std::endl;
			testfile.flush();
#endif

	MYSQL_STMT *stmt;
	stmt = mysql_stmt_init(connection);
	if (!stmt)
	{
		throw std::runtime_error("Could not initialize statement\n");
	}

	status = mysql_stmt_prepare(stmt, query.c_str(), query.size() + 1);
	test_stmt_error(stmt, status);

	status = mysql_stmt_bind_param(stmt, input_params);
	test_stmt_error(stmt, status);

	status = mysql_stmt_execute(stmt);
	test_stmt_error(stmt, status);

	status = mysql_stmt_close(stmt);
	test_stmt_error(stmt, status);

	returnconnection(connection);
	return;
}

void mysql_db_handler::checkWorldUUID() {
	MYSQL_RES *result;
	MYSQL_ROW row;
	bool addUUID = true;
	unsigned long long int rowcount;

	std::string queryworlduuid =
	str(boost::format{"SELECT HEX(`world`.`uuid`) "
			"FROM `world` "
			"WHERE `world`.`uuid` = CAST(0x%s AS BINARY)"} % worlduuid);

	this->rawquery(queryworlduuid, &result);

	rowcount = mysql_num_rows(result);

	if (rowcount > 0) {
		addUUID = false;
	}

	mysql_free_result(result);

	if (addUUID) {
		queryworlduuid =
			str(boost::format{"INSERT INTO `world` (`uuid`, `name`, `map`) "
								"VALUES (CAST(0x%s AS BINARY), \"you need to\", \"eddit this\")"}
								% worlduuid);
		this->rawquery(queryworlduuid);
	}
};

std::string mysql_db_handler::terminateHandler(std::string &extFunction, ext_arguments &extArgument) {
	terminateHandler();
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"DONE\"]";
}

void mysql_db_handler::terminateHandler() {
	disconnect();
	return;
}

std::string mysql_db_handler::intergetUUID(std::string &extFunction, ext_arguments &extArgument) {
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + orderedUUID() + "\"]";
}

std::string mysql_db_handler::interecho(std::string &extFunction, ext_arguments &extArgument) {
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + extArgument.get<std::string>("echostring") + "\"]";
}

std::string mysql_db_handler::interdbVersion(std::string &extFunction, ext_arguments &extArgument) {
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + querydbversion() + "\"]";
}


std::string mysql_db_handler::interloadPlayer(std::string &extFunction, ext_arguments &extArgument) {
	std::string nickname = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_NICKNAME);
	std::string steamid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_STEAMID);
	std::string playerinfo = loadPlayer(nickname, steamid);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + playerinfo + "]";
}

std::string mysql_db_handler::interloadPlayerGroups(std::string &extFunction, ext_arguments &extArgument) {
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string result = loadPlayerGroups(playeruuid);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + result + "]";
}

std::string mysql_db_handler::interloadAvChars(std::string &extFunction, ext_arguments &extArgument) {
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string result = loadAvChars(playeruuid);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + result + "]";
}

std::string mysql_db_handler::interlinkChars(std::string &extFunction, ext_arguments &extArgument) {
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string variabuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_VARIABUUID);
	std::string result = linkChars(playeruuid, variabuuid);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + result + "]";
}

std::string mysql_db_handler::interloadChar(std::string &extFunction, ext_arguments &extArgument) {
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string characterdata = loadChar(playeruuid);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + characterdata + "]";
}

std::string mysql_db_handler::intercreateChar(std::string &extFunction, ext_arguments &extArgument) {
	std::string characterdata = createChar(extArgument);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + characterdata + "\"]";
}

std::string mysql_db_handler::interupdateChar(std::string &extFunction, ext_arguments &extArgument) {
	std::string result = updateChar(extArgument);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string mysql_db_handler::interkillChar(std::string &extFunction, ext_arguments &extArgument) {
	std::string charuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_CHARUUID);
	std::string attackeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_ATTACKER);
	std::string type = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_TYPE);
	std::string weapon = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_WEAPON);
	float distance = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DISTANCE);

	std::string result = killChar(charuuid, attackeruuid, type, weapon, distance);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string mysql_db_handler::interloadObject(std::string &extFunction, ext_arguments &extArgument) {
	std::string result = loadObject(extArgument);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + result + "]";
}

std::string mysql_db_handler::intercreateObject(std::string &extFunction, ext_arguments &extArgument) {
	std::string result = createObject(extArgument);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string mysql_db_handler::interqcreateObject(std::string &extFunction, ext_arguments &extArgument) {
	std::string result = createObject(extArgument);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string mysql_db_handler::interupdateObject(std::string &extFunction, ext_arguments &extArgument) {
	std::string result = updateObject(extArgument);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string mysql_db_handler::interkillObject(std::string &extFunction, ext_arguments &extArgument) {
	std::string charuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	std::string attackeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_ATTACKER);
	std::string type = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_TYPE);
	std::string weapon = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_WEAPON);
	float distance = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DISTANCE);

	std::string result = killObject(charuuid, attackeruuid, type, weapon, distance);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string mysql_db_handler::interdumpObjects(std::string &extFunction, ext_arguments &extArgument) {
	std::string matrix;
	bool placecommaone = false;
	bool placecommatwo = false;

	std::vector<object_mysql *> objectList = dumpObjects(extArgument);
	matrix = "[";
	for (object_mysql * object : objectList) {
			if (placecommaone) {
					matrix += ",";
			}

			matrix += object->getAsArmaString();

			placecommaone = true;
	}
	matrix += "]";

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + matrix + "]";
}

/* SQL Querys */

std::string mysql_db_handler::querydbversion() {
	MYSQL_RES *result;
	MYSQL_ROW row;
	std::string version;

	this->rawquery("SELECT VERSION()", &result);

	row = mysql_fetch_row(result);

	version = row[0];

	mysql_free_result(result);

	return version;
}

std::string mysql_db_handler::loadPlayer(std::string nickname, std::string steamid) {
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;

	// player info
	std::string playeruuid = "";
	std::string persistent_variables_uuid = "";
	std::string mainclanuuid = "";
	std::string banned = "false";
	std::string banreason = "unknown";

	std::string queryplayerinfo =
	str(boost::format{"SELECT HEX(`player`.`uuid`), "
			"HEX(`player_on_world_has_persistent_variables`.`persistent_variables_uuid`), "
			"HEX(`player`.`mainclan_uuid`), "
		    "(CASE WHEN (NOW() < `player`.`banenddate`) THEN \"true\" ELSE \"false\" END) AS BANNED, "
		    "`player`.`banreason`"
			"FROM `player` "
			"LEFT JOIN `player_on_world_has_persistent_variables` "
			" ON `player`.`uuid` = `player_on_world_has_persistent_variables`.`player_uuid` "
			" AND `player_on_world_has_persistent_variables`.`world_uuid` =  CAST(0x%s AS BINARY) "
			"WHERE `player`.`steamid` = \"%s\" "} % worlduuid % steamid);

	this->rawquery(queryplayerinfo, &result);

	rowcount = mysql_num_rows(result);

	if (rowcount > 0) {
		row = mysql_fetch_row(result);

		// should be always true, if not we have a huge bug
		if (row[0] != NULL) {
			playeruuid = row[0];
		}

		if (row[1] != NULL) {
			persistent_variables_uuid = row[1];
		}

		if (row[2] != NULL) {
			mainclanuuid = row[2];
		}

		if (row[3] != NULL) {
			banned = row[3];
		}
		if (row[4] != NULL) {
			banreason = row[4];
		}
	}

	mysql_free_result(result);

	if (banned == "false") {
		if (playeruuid == "") {
			playeruuid = orderedUUID();
			queryplayerinfo =
				str(boost::format{"INSERT INTO `player` (`uuid`, `steamid`, `battleyeid`, "
									"`firstlogin`, `firstnick`, `lastlogin`, `lastnick`, "
									"`bancount`, `banreason`, `banbegindate`, `banenddate`) "
									"VALUES (CAST(0x%s AS BINARY), \"%s\", \"unused\", NOW(), "
									"\"%s\", NOW(), \"%s\", '0', NULL, NULL, NULL)"}
									% playeruuid % steamid % nickname % nickname);
		} else {
			queryplayerinfo =
						str(boost::format{"UPDATE `player` SET `lastlogin` = NOW(), `lastnick` = \"%s\" \
											WHERE `player`.`uuid` = CAST(0x%s AS BINARY)"} % nickname % playeruuid);
		}

		this->rawquery(queryplayerinfo);

		if (whitelistonly) {
			std::string querywhitelist =
				str(boost::format{"SELECT * FROM `whitelist` "
						"WHERE `whitelist`.`world_uuid` = CAST(0x%s AS BINARY) "
						" AND `whitelist`.`player_uuid` = CAST(0x%s AS BINARY) "} % worlduuid % playeruuid);

				this->rawquery(querywhitelist, &result);

				rowcount = mysql_num_rows(result);

				if (rowcount < 1) {
					banned = "true";
					banreason = "not whitelisted";
				}

				mysql_free_result(result);
		}
	}

	return "[\"" + playeruuid + "\",\"" + persistent_variables_uuid + "\",\"" + mainclanuuid + "\",[" + banned + ",\"" + banreason +"\"]]";
}

std::string mysql_db_handler::loadPlayerGroups(std::string playeruuid) {
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	bool printcommaone = false;
	bool printcommatwo = false;

	// char info
	std::string groupinfo = "";

	std::string querygroupinfo =
			str(
					boost::format {
		"SELECT HEX(`clan`.`uuid`) AS 'clan_uuid', HEX(`founder`.`uuid`) AS 'founder_uuid', `founder`.`steamid` AS 'founder_steamid', "
		"`founder`.`lastnick` AS 'founder_nick', GROUP_CONCAT("
		  "DISTINCT CONCAT('[\"', HEX(`player`.`uuid`), '\",\"' , `player`.`steamid`, '\",\"' , `player`.`lastnick`, '\",', "
		  	  "`clan_memeber`.`rank`, ',\"', `clan_memeber`.`comment`, '\"]') "
		  "SEPARATOR ','"
		") AS 'clan_member'"
		"FROM `clan` LEFT JOIN `player` AS `founder` ON `clan`.`founder_uuid` = `founder`.`uuid` "
		"LEFT JOIN `clan_memeber` ON `clan`.`uuid` = `clan_memeber`.`clan_uuid` "
		"LEFT JOIN `player` ON `clan_memeber`.`player_uuid` = `player`.`uuid`"
		"WHERE `clan`.`uuid` IN ("
								"SELECT `clan_memeber`.`clan_uuid` "
								"FROM `clan_memeber` "
								"WHERE `clan_memeber`.`player_uuid` = CAST(0x%s AS BINARY)"
								")"
		"GROUP BY `clan`.`uuid`" }
							% playeruuid);

	char typearray[] = {
			1, // HEX(`clan`.`uuid`)
			1, // HEX(`founder`.`uuid`)
			1, // `founder`.`steamid`
			1, // `founder`.`lastnick`
			3, // 'clan_member'
			};

	this->rawquery(querygroupinfo, &result);

	fieldcount = mysql_num_fields(result);
	rowcount = mysql_num_rows(result);

	groupinfo = "[";

	for (int rowpos = 0; rowpos < rowcount; rowpos++) {
		row = mysql_fetch_row(result);

		if (printcommaone) {
			groupinfo += ",\n";
			printcommaone = false;
			printcommatwo = false;
		}

		groupinfo += "[";

		for (int fieldpos = 0; fieldpos < fieldcount; fieldpos++) {
			if (printcommatwo) {
				groupinfo += ",";
				printcommatwo = false;
			}
			switch (typearray[fieldpos]) {
			case 1:
				groupinfo += "\"";
				break;
			case 2:
				groupinfo += "[";
				if (row[fieldpos] != NULL)
					groupinfo += "\"";
				break;
			case 3:
				groupinfo += "[";
				break;
			default:
				groupinfo += "";
			}

			if (row[fieldpos] != NULL) {
				groupinfo += row[fieldpos];
			} else {
				groupinfo += "";
			}

			switch (typearray[fieldpos]) {
			case 1:
				groupinfo += "\"";
				break;
			case 2:
				if (row[fieldpos] != NULL)
					groupinfo += "\"";
				groupinfo += "]";
				break;
			case 3:
				groupinfo += "]";
				break;
			}
			printcommatwo = true;
		}
		groupinfo += "]";
		printcommaone = true;
	}
	groupinfo += "]";

	return groupinfo;
}

std::string mysql_db_handler::loadAvChars(std::string playeruuid) {
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	bool printcommaone = false;
	bool printcommatwo = false;

	// char info
	std::string charinfo = "";

	std::string querycharinfo =
			str(
					boost::format {
							"SELECT HEX(`persistent_variables`.`uuid`), "
									"`persistent_variables`.`persistentvariables`, `world`.`name`, `world`.`map` "
									"FROM `world_is_linked_to_world` "
									"INNER JOIN `player_on_world_has_persistent_variables` "
									" ON `world_is_linked_to_world`.`world_uuid2` = `player_on_world_has_persistent_variables`.`world_uuid` "
									"INNER JOIN `persistent_variables` "
									" ON `persistent_variables`.`uuid` = `player_on_world_has_persistent_variables`.`persistent_variables_uuid` "
									"INNER JOIN `world` "
									" ON `world`.`uuid` = `world_is_linked_to_world`.`world_uuid2` "
									"WHERE `world_is_linked_to_world`.`world_uuid1` = CAST(0x%s AS BINARY) "
									" AND `player_on_world_has_persistent_variables`.`player_uuid` = CAST(0x%s AS BINARY)" }
							% worlduuid % playeruuid);

	char typearray[] = {
			1, // HEX(`persistent_variables`.`uuid`)
			0, // `persistent_variables`.`persistentvariables`
			1, // `world`.`name`
			1  // `world`.`map`
			};

	this->rawquery(querycharinfo, &result);

	fieldcount = mysql_num_fields(result);
	rowcount = mysql_num_rows(result);

	charinfo = "[";

	for (int rowpos = 0; rowpos < rowcount; rowpos++) {
		row = mysql_fetch_row(result);

		if (printcommaone) {
			charinfo += ",";
			printcommaone = false;
			printcommatwo = false;
		}

		charinfo += "[";

		for (int fieldpos = 0; fieldpos < fieldcount; fieldpos++) {
			if (printcommatwo) {
				charinfo += ",";
				printcommatwo = false;
			}
			switch (typearray[fieldpos]) {
			case 1:
				charinfo += "\"";
				break;
			case 2:
				charinfo += "[";
				if (row[fieldpos] != NULL)
					charinfo += "\"";
				break;
			default:
				charinfo += "";
			}

			if (row[fieldpos] != NULL) {
				charinfo += row[fieldpos];
			} else {
				charinfo += "";
			}

			switch (typearray[fieldpos]) {
			case 1:
				charinfo += "\"";
				break;
			case 2:
				if (row[fieldpos] != NULL)
					charinfo += "\"";
				charinfo += "]";
				break;
			}
			printcommatwo = true;
		}
		charinfo += "]";
		printcommaone = true;
	}
	charinfo += "]";

	return charinfo;
}

std::string mysql_db_handler::linkChars(std::string playeruuid, std::string variabuuid) {
	std::string query = str(
			boost::format { "INSERT INTO `player_on_world_has_persistent_variables` "
					"(`player_uuid`, `world_uuid`, `persistent_variables_uuid`) "
					"VALUES (0x%s AS BINARY), CAST(0x%s AS BINARY), CAST(0x%s AS BINARY))"
					} % playeruuid % worlduuid % variabuuid);


	this->rawquery(query);

	return playeruuid;
}

std::string mysql_db_handler::loadChar(std::string playeruuid) {
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	bool printcomma = false;
	character_mysql* character = 0;

	std::string querycharinfo = str(boost::format{"SELECT `animationstate`, `direction`, `positiontype`, `positionx`, `positiony`, `positionz`, HEX(`character`.`uuid`) as `character_uuid`, "
				"`classname`, `hitpoints`, `variables`, `textures`, `gear`, `currentweapon`, HEX(`character`.`charactershareables_uuid`), "
				"`persistentvariables`, HEX(`charactershareables`.`persistent_variables_uuid`), HEX(`object_uuid`) "
				"FROM `player_on_world_has_character` "
				"INNER JOIN `character`  "
				" ON `player_on_world_has_character`.`character_uuid` = `character`.`uuid` "
				"INNER JOIN `charactershareables` "
				" ON `character`.`charactershareables_uuid` = `charactershareables`.`uuid` "
				"INNER JOIN `persistent_variables` "
				" ON `charactershareables`.`persistent_variables_uuid` = `persistent_variables`.`uuid` "
				"WHERE `player_on_world_has_character`.`player_uuid` = CAST(0x%s AS BINARY) "
				" AND `player_on_world_has_character`.`world_uuid` =  CAST(0x%s AS BINARY) "
				" AND `player_on_world_has_character`.`killinfo_uuid` IS NULL" } % playeruuid % worlduuid);

	this->rawquery(querycharinfo, &result);

	fieldcount = mysql_num_fields(result);
	rowcount = mysql_num_rows(result);

	if (rowcount > 0) {
		row = mysql_fetch_row(result);
		if (row[6] != NULL && row[13] != NULL && row[15] != NULL) {
			std::string charuuid = row[6];

			auto it = charactercache.find(charuuid);
			if (it != charactercache.end()) {
				character = it->second;
			} else {
				character = new character_mysql;
				charactercache.insert(std::make_pair(charuuid, character));
			}

			for (int fieldpos = 0; fieldpos < fieldcount; fieldpos++) {
				if (row[fieldpos] != NULL) {
					character->setData(fieldpos, row[fieldpos]);
				} else {
					character->setData(fieldpos, "");
					character->is_null[fieldpos] = (my_bool) 1;
				}
			}
		}
	}

	mysql_free_result(result);

	if (character != 0) {
		return character->getAsArmaString();
	}

	return "[]";
}


std::string mysql_db_handler::createChar(ext_arguments &extArgument) {
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;

	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);

	// char info
	std::string charuuid = "";
	std::string persistent_variables_uuid = "";
	std::string shareable_variables_uuid = "";

	/* for testing purpose, check if there is an existing character, if yes return its uuid */
	std::string query = str(boost::format { "SELECT HEX(`character_uuid`) "
			"FROM `player_on_world_has_character` "
			"WHERE `player_on_world_has_character`.`player_uuid` = CAST(0x%s AS BINARY) "
			"AND `player_on_world_has_character`.`world_uuid` =  CAST(0x%s AS BINARY) "
			"AND `player_on_world_has_character`.`killinfo_uuid` IS NULL" } % playeruuid % worlduuid);

	this->rawquery(query, &result);

	rowcount = mysql_num_rows(result);

	if (rowcount > 0) {
		row = mysql_fetch_row(result);

		if (row[0] != NULL) {
			charuuid = row[0];
		}
	}

	mysql_free_result(result);

	if (charuuid == "") {
		charuuid = orderedUUID();
		character_mysql* character = new character_mysql;
		character->setData(extArgument);
		character->setData(PROTOCOL_DBCALL_ARGUMENT_CHARUUID, charuuid);
		charactercache.insert(std::make_pair(charuuid, character));

		/* get the uuid for the Death Persistent Variables */
		query = str(boost::format { "SELECT HEX(`persistent_variables_uuid`) "
				"FROM `player_on_world_has_persistent_variables` "
				"WHERE `player_uuid` = CAST(0x%s AS BINARY) "
				"AND `world_uuid` = CAST(0x%s AS BINARY)" } % playeruuid % worlduuid);

		this->rawquery(query, &result);

		rowcount = mysql_num_rows(result);

		if (rowcount > 0) {
			row = mysql_fetch_row(result);

			if (row[0] != NULL) {
				persistent_variables_uuid = row[0];
			}
		}

		mysql_free_result(result);

		/* if there are no Death Persistent Variables create some*/
		if (persistent_variables_uuid == "") {
			persistent_variables_uuid = orderedUUID();
			character->setData(PROTOCOL_DBCALL_ARGUMENT_PERSISTENTVARIABUUID, persistent_variables_uuid);

			query = "INSERT INTO `persistent_variables` (`persistentvariables`, `uuid`) VALUES (?, UNHEX(?))";

			this->preparedStatementQuery(query, character->mysql_bind+14);

			query = str(boost::format { "INSERT INTO `player_on_world_has_persistent_variables` "
					"(`player_uuid`, `world_uuid`, `persistent_variables_uuid`) "
					"VALUES (CAST(0x%s AS BINARY), "
					"CAST(0x%s AS BINARY), "
					"CAST(0x%s AS BINARY));" } % playeruuid % worlduuid % persistent_variables_uuid);

			this->rawquery(query);
		} else {
			character->setData(PROTOCOL_DBCALL_ARGUMENT_PERSISTENTVARIABUUID, persistent_variables_uuid);
			query = str(boost::format { "SELECT `persistentvariables` "
							"FROM `persistent_variables` "
							"WHERE `persistent_variables`.`uuid` = CAST(0x%s AS BINARY)"
							} % persistent_variables_uuid);

			this->rawquery(query, &result);

			rowcount = mysql_num_rows(result);

			if (rowcount > 0) {
				row = mysql_fetch_row(result);

				if (row[0] != NULL) {
					character->setData(PROTOCOL_DBCALL_ARGUMENT_PERSISTENTVARIABLES, row[0]);
				}
			}
			mysql_free_result(result);
		}

		/* get alive linked character shareables */
		query = str(boost::format { "SELECT HEX(`charactershareables`.`uuid`) "
				"FROM `player_on_world_has_character` "
				"INNER JOIN `character` "
				"ON `player_on_world_has_character`.`character_uuid` = `character`.`uuid` "
				"INNER JOIN `charactershareables` "
				"ON `character`.`charactershareables_uuid` = `charactershareables`.`uuid` "
				"WHERE `player_on_world_has_character`.`player_uuid` = CAST(0x%s AS BINARY) "
				"AND `player_on_world_has_character`.`killinfo_uuid` IS NULL "
				"AND `player_on_world_has_character`.`world_uuid` IN  ( "
				"SELECT `world_uuid` "
				"FROM `player_on_world_has_persistent_variables` "
				"WHERE `persistent_variables_uuid` = CAST(0x%s AS BINARY) "
				") LIMIT 1" } % playeruuid % persistent_variables_uuid);

		this->rawquery(query, &result);

		rowcount = mysql_num_rows(result);

		if (rowcount > 0) {
			row = mysql_fetch_row(result);

			if (row[0] != NULL) {
				shareable_variables_uuid = row[0];
			}
		}

		mysql_free_result(result);

		/* if there are no Death Persistent Variables create some*/
		if (shareable_variables_uuid == "") {
			shareable_variables_uuid = orderedUUID();
			character->setData(PROTOCOL_DBCALL_ARGUMENT_CHARSHAREUUID, shareable_variables_uuid);

			query = str(
					boost::format { "INSERT INTO `charactershareables` (`classname`, `hitpoints`, "
							"`variables`, `textures`, `gear`, "
							"`currentweapon`, `uuid`, `persistent_variables_uuid`) "
							"VALUES (?, ?, ?, ?, ?, ?, UNHEX(?), CAST(0x%s AS BINARY))" } % persistent_variables_uuid);

			this->preparedStatementQuery(query, character->mysql_bind+7);
		} else {
			character->setData(PROTOCOL_DBCALL_ARGUMENT_CHARSHAREUUID, shareable_variables_uuid);

			query = str(boost::format { "SELECT `classname`, `hitpoints`, "
							"`variables`, `textures`, `gear`, `currentweapon`"
							"FROM `charactershareables` "
							"WHERE `charactershareables`.`uuid` = CAST(0x%s AS BINARY)"
							} % shareable_variables_uuid);

			this->rawquery(query, &result);

			rowcount = mysql_num_rows(result);

			if (rowcount > 0) {
				row = mysql_fetch_row(result);

				if (row[0] != NULL) {
					character->setData(PROTOCOL_DBCALL_ARGUMENT_CLASSNAME, row[0]);
				}
				if (row[1] != NULL) {
					character->setData(PROTOCOL_DBCALL_ARGUMENT_HITPOINTS, row[1]);
				}
				if (row[2] != NULL) {
					character->setData(PROTOCOL_DBCALL_ARGUMENT_VARIABLES, row[2]);
				}
				if (row[3] != NULL) {
					character->setData(PROTOCOL_DBCALL_ARGUMENT_TEXTURES, row[3]);
				}
				if (row[4] != NULL) {
					character->setData(PROTOCOL_DBCALL_ARGUMENT_GEAR, row[4]);
				}
				if (row[5] != NULL) {
					character->setData(PROTOCOL_DBCALL_ARGUMENT_CURRENTWEAPON, row[5]);
				}
			}
			mysql_free_result(result);
		}

		query = str(
				boost::format { "INSERT INTO `character` (`animationstate`, `direction`, "
						"`positiontype`, `positionx`, `positiony`, "
						"`positionz`, `uuid`, `charactershareables_uuid`) "
						"VALUES (?, ?, ?, ?, ?, ?, "
						"UNHEX(?), CAST(0x%s AS BINARY))" } % shareable_variables_uuid);

		this->preparedStatementQuery(query, character->mysql_bind);

		query = str(boost::format { "INSERT INTO `player_on_world_has_character` (`player_uuid`, `world_uuid`, "
				"`character_uuid`, `killinfo_uuid`) "
				"VALUES (CAST(0x%s AS BINARY), "
				"CAST(0x%s AS BINARY), "
				"CAST(0x%s AS BINARY), "
				"NULL)" } % playeruuid % worlduuid % charuuid);

		this->rawquery(query);
	}

	return charuuid;
}

std::string mysql_db_handler::updateChar(ext_arguments &extArgument) {
	std::string charuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_CHARUUID);
	character_mysql* character = 0;

	auto it = charactercache.find(charuuid);
	if (it != charactercache.end()) {
		character = static_cast<character_mysql*>((void*)it->second);
	} else {
		throw std::runtime_error("could not find character to update: " + charuuid);
	}

	character->setData(extArgument);

	std::string query = "UPDATE `character` "
					"SET `animationstate` = ?, "
					"`direction` = ?, "
					"`positiontype` = ?, "
					"`positionx` = ?, "
					"`positiony` = ?, "
					"`positionz` = ? "
					" WHERE `character`.`uuid` = UNHEX(?)";

	this->preparedStatementQuery(query, character->mysql_bind);

	query = "UPDATE `charactershareables` "
					"SET `classname` = ?, "
					"`hitpoints` = ?, "
					"`variables` = ?, "
					"`textures` = ?, "
					"`gear` = ?, "
					"`currentweapon` = ? "
					" WHERE `charactershareables`.`uuid` = UNHEX(?)";

	this->preparedStatementQuery(query, character->mysql_bind+7);

	query = "UPDATE `persistent_variables` "
			"SET `persistentvariables` = ? "
			"WHERE `persistent_variables`.`uuid` = UNHEX(?)";

	this->preparedStatementQuery(query, character->mysql_bind+14);

	return charuuid;
}

std::string mysql_db_handler::killChar(std::string charuuid, std::string attackeruuid, std::string type,
		std::string weapon, float distance) {
	std::string killuuid = orderedUUID();

	if (attackeruuid == "") {
		attackeruuid = "NULL";
	} else {
		attackeruuid = "CAST(0x" + attackeruuid + " AS BINARY)";
	}

	if (type == "") {
		type = "NULL";
	} else {
		type = "'" + type + "'";
	}

	if (weapon == "") {
		weapon = "NULL";
	} else {
		weapon = "'" + weapon + "'";
	}

	std::string query = str(
			boost::format {"INSERT INTO `killinfo` "
							"(`uuid`, `date`, `attacker_uuid`, `type`, `weapon`, `distance`) "
							"VALUES (CAST(0x%s AS BINARY), CURRENT_TIMESTAMP, %s, %s, %s, %s);" }
							% killuuid % attackeruuid % type % weapon % distance);


	this->rawquery(query);

	query = str(
				boost::format {"UPDATE `player_on_world_has_character` "
								"SET `killinfo_uuid` = CAST(0x%s AS BINARY) "
								"WHERE `player_on_world_has_character`.`character_uuid` IN "
								"(SELECT `character`.`uuid` FROM `character` WHERE `charactershareables_uuid` = "
								"(SELECT `character`.`charactershareables_uuid` FROM `character` WHERE `uuid` = CAST(0x%s AS BINARY)));"}
								% killuuid % charuuid);


		this->rawquery(query);

	return killuuid;
}

std::string mysql_db_handler::loadObject(ext_arguments &extArgument) {
	std::string objectuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	object_mysql* object = 0;

	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	std::string query =
	str(boost::format{"SELECT `object`.`classname`, `object`.`priority`, `object`.`type`, `object`.`accesscode`, "
						"`object`.`locked`, HEX(`object`.`player_uuid`), `object`.`hitpoints`, `object`.`damage`, "
						"`object`.`fuel`, `object`.`fuelcargo`, `object`.`repaircargo`, `object`.`items`, "
						"`object`.`magazinesturret`, "
						"`object`.`variables`, `object`.`animationstate`, `object`.`textures`, `object`.`direction`, "
						"`object`.`positiontype`, `object`.`positionx`, `object`.`positiony`, `object`.`positionz`, "
						"`object`.`positionadvanced`, `object`.`reservedone`, `object`.`reservedtwo`, "
						"HEX(`object`.`uuid`), "
						"HEX(`world_has_objects`.`parentobject_uuid`), "
						"HEX(`player`.`uuid`) "
						"FROM `world_has_objects` "
						"INNER JOIN `object` "
						" ON `world_has_objects`.`object_uuid` = `object`.`uuid` "
						"LEFT JOIN `player` "
						" ON `object`.`player_uuid` = `player`.`uuid` "
						"WHERE `world_has_objects`.`object_uuid` = CAST(0x%s AS BINARY) "
						"AND `world_has_objects`.`killinfo_uuid` IS NULL "
						"ORDER BY `object`.priority ASC, `world_has_objects`.`parentobject_uuid` ASC"} % objectuuid);

	this->rawquery(query, &result);

	fieldcount = mysql_num_fields(result);
	rowcount = mysql_num_rows(result);

	if (rowcount > 0) {
		row = mysql_fetch_row(result);
		if (row[24] != NULL) {
			auto it = objectcache.find(objectuuid);
			if (it != objectcache.end()) {
				object = static_cast<object_mysql*>((void*)it->second);
			} else {
				object = new object_mysql;
				objectcache.insert(std::make_pair(objectuuid, object));
			}

			for (int fieldpos = 0; fieldpos < fieldcount && fieldpos < 27; fieldpos++) {
				if (row[fieldpos] != NULL) {
					object->setData(fieldpos, row[fieldpos]);
				} else {
					object->setNull(fieldpos);
				}
			}
		}
	}

	mysql_free_result(result);

	if (object != 0) {
		return object->getAsArmaString();
	}

	return "[]";

}

std::string mysql_db_handler::createObject(ext_arguments &extArgument) {
	std::string objectuuid;

	object_mysql* object = new object_mysql;
	object->setData(extArgument);

	try {
		objectuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	} catch (std::runtime_error const& e) {
		objectuuid = orderedUUID();
		object->setData(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID, objectuuid);
	}

	objectcache.insert(std::make_pair(objectuuid, object));

	std::string query = "INSERT INTO `object` (`classname`, `priority`, `timelastused`, "
							"`timecreated`, `type`, `accesscode`, `locked`, `player_uuid`, `hitpoints`, "
							"`damage`, `fuel`, `fuelcargo`, `repaircargo`, `items`, `magazinesturret`, "
							"`variables`, `animationstate`, `textures`, `direction`, `positiontype`, "
							"`positionx`, `positiony`, `positionz`, "
							"`positionadvanced`, `reservedone`, `reservedtwo`, `uuid`) "
							"VALUES (?, ?, now(), "
							"now(), ?, ?, ?, UNHEX(?), ?, "
							"?, ?, ?, ?, ?, ?, ?, "
							"?, ?, ?, ?, ?, ?, ?, "
							"?, ?, ?, UNHEX(?))";

	this->preparedStatementQuery(query, object->mysql_bind);

	query = str(boost::format { "INSERT INTO `world_has_objects` (`world_uuid`, `object_uuid`) "
			"VALUES (CAST(0x%s AS BINARY), CAST(0x%s AS BINARY))" } % worlduuid % objectuuid);

	this->rawquery(query);

	return objectuuid;
}

std::string mysql_db_handler::updateObject(ext_arguments &extArgument) {
	std::string objectuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	object_mysql* object = 0;

	auto it = objectcache.find(objectuuid);
	if (it != objectcache.end()) {
		object = static_cast<object_mysql*>((void*)it->second);
	} else {
		throw std::runtime_error("could not find object to update: " + objectuuid);
	}

	object->setData(extArgument);

	std::string query = "UPDATE `object` "
						"SET `classname` = ?, "
						"    `priority` = ?, "
						"    `type` = ?,"
						"    `accesscode` = ?, "
						"    `locked` = ?, "
						"    `player_uuid` = UNHEX(?), "
						"    `hitpoints` = ?, "
						"    `damage` = ?, "
						"    `fuel` = ?, "
						"    `fuelcargo` = ?, "
						"    `repaircargo` = ?, "
						"    `items` = ?, "
						"    `magazinesturret` = ?, "
						"    `variables` = ?, "
						"    `animationstate` = ?, "
						"    `textures` = ?, "
						"    `direction` = ?, "
						"    `positiontype` = ?, "
						"    `positionx` = ?, "
						"    `positiony` = ?, "
						"    `positionz` = ?, "
						"    `positionadvanced` = ?, "
						"    `reservedone` = ?, "
						"    `reservedtwo` = ? "
						"WHERE `object`.`uuid` = UNHEX(?);";
	this->preparedStatementQuery(query, object->mysql_bind);

	return objectuuid;
}

std::string mysql_db_handler::killObject(std::string objectuuid, std::string attackeruuid, std::string type,
		std::string weapon, float distance) {
	std::string killuuid = orderedUUID();

	if (attackeruuid == "") {
		attackeruuid = "NULL";
	} else {
		attackeruuid = "CAST(0x" + attackeruuid + " AS BINARY)";
	}

	if (type == "") {
		type = "NULL";
	} else {
		type = "'" + type + "'";
	}

	if (weapon == "") {
		weapon = "NULL";
	} else {
		weapon = "'" + weapon + "'";
	}

	std::string query = str(
			boost::format {"INSERT INTO `killinfo` "
							"(`uuid`, `date`, `attacker_uuid`, `type`, `weapon`, `distance`) "
							"VALUES (CAST(0x%s AS BINARY), CURRENT_TIMESTAMP, %s, %s, %s, %s);" }
							% killuuid % attackeruuid % type % weapon % distance);


	this->rawquery(query);

	query = str(
				boost::format {"UPDATE `world_has_objects` SET `killinfo_uuid` = CAST(0x%s AS BINARY) "
								"WHERE `world_has_objects`.`object_uuid` = CAST(0x%s AS BINARY);"}
								% killuuid % objectuuid);


		this->rawquery(query);

	return killuuid;
}

std::vector<object_mysql *> mysql_db_handler::dumpObjects(ext_arguments &extArgument) {
	std::vector<object_mysql *> objectList;
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	std::string query =
	str(boost::format{"SELECT `object`.`classname`, `object`.`priority`, `object`.`type`, `object`.`accesscode`, "
						"`object`.`locked`, HEX(`object`.`player_uuid`), `object`.`hitpoints`, `object`.`damage`, "
						"`object`.`fuel`, `object`.`fuelcargo`, `object`.`repaircargo`, `object`.`items`, "
						"`object`.`magazinesturret`, "
						"`object`.`variables`, `object`.`animationstate`, `object`.`textures`, `object`.`direction`, "
						"`object`.`positiontype`, `object`.`positionx`, `object`.`positiony`, `object`.`positionz`, "
						"`object`.`positionadvanced`, `object`.`reservedone`, `object`.`reservedtwo`, "
						"HEX(`object`.`uuid`), "
						"HEX(`world_has_objects`.`parentobject_uuid`), "
						"HEX(`player`.`uuid`) "
						"FROM `world_has_objects` "
						"INNER JOIN `object` "
						" ON `world_has_objects`.`object_uuid` = `object`.`uuid` "
						"LEFT JOIN `player` "
						" ON `object`.`player_uuid` = `player`.`uuid` "
						"WHERE `world_has_objects`.`world_uuid` = CAST(0x%s AS BINARY) "
						"AND `world_has_objects`.`killinfo_uuid` IS NULL "
						"ORDER BY `object`.priority ASC, `world_has_objects`.`parentobject_uuid` ASC"} % worlduuid);

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_OFFSET) && extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_LIMIT)) {
		unsigned int offset = extArgument.get<unsigned int>(PROTOCOL_DBCALL_ARGUMENT_OFFSET);
		unsigned int limit = extArgument.get<unsigned int>(PROTOCOL_DBCALL_ARGUMENT_LIMIT);
		query += " LIMIT " + std::to_string(offset) + "," + std::to_string(limit);
	}

	this->rawquery(query, &result);

	fieldcount = mysql_num_fields(result);
	rowcount = mysql_num_rows(result);

	for (int rowpos = 0; rowpos < rowcount; rowpos++) {
		row = mysql_fetch_row(result);
		if (row[24] != NULL) {
			std::string uuid = row[24];

			object_mysql* object = new object_mysql;
			objectList.push_back(object);
			objectcache.insert(std::make_pair(uuid, object));

			for (int fieldpos = 0; fieldpos < fieldcount && fieldpos < 27; fieldpos++) {
				if (row[fieldpos] != NULL) {
					object->setData(fieldpos, row[fieldpos]);
				} else {
					object->setNull(fieldpos);
				}
			}
		}
	}

	mysql_free_result(result);

	return objectList;
}

void mysql_db_handler::test_stmt_error(MYSQL_STMT *stmt, int status) {
  if (status)
  {
	  throw std::runtime_error(std::string("MYSQLERROR") + std::string(mysql_stmt_error(stmt)) + std::string(" Errornumber: ") + std::to_string(mysql_stmt_errno(stmt)));
  }
}
