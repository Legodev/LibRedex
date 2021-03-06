/* mysql.cpp
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
#include "mysql/mysql_binds.hpp"
#include "mysql/datacache/charactermysql.hpp"
#include "mysql/datacache/objectmysql.hpp"
#include "mysql/migration/includeAllSchemas.hpp"

#ifdef DEBUG
#include "logger.hpp"
extern Logger logfile;
#endif

extern int (* callbackPtr)(char const* name, char const* function, char const* data);

extern std::map<std::string, unsigned int>* objectvariablemap;
std::map<std::string, unsigned int>* objectvariablemap = 0;
extern std::map<std::string, unsigned int>* charactervariablemap;
std::map<std::string, unsigned int>* charactervariablemap = 0;

mysql_db_handler::mysql_db_handler(EXT_FUNCTIONS& extFunctions) {
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
					std::string(PROTOCOL_DBCALL_FUNCTION_SET_WORLD_VARIABLES),
					std::make_tuple(
							boost::bind(&mysql_db_handler::setWorldVariables, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_GET_WORLD_VARIABLES),
					std::make_tuple(
							boost::bind(&mysql_db_handler::getWorldVariables, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_SET_WORLD_STATE),
					std::make_tuple(
							boost::bind(&mysql_db_handler::setWorldState, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_GET_LINKED_WORLDS),
					std::make_tuple(
							boost::bind(&mysql_db_handler::getLinkedWorlds, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_PLAYER),
					std::make_tuple(
							boost::bind(&mysql_db_handler::loadPlayer, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_LOGOUT_PLAYER),
					std::make_tuple(
							boost::bind(&mysql_db_handler::logoutPlayer, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_LOGOUT_PLAYER_QUIET),
					std::make_tuple(
							boost::bind(&mysql_db_handler::logoutPlayer, this, _1, _2),
							QUIET_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_INCREMENT_PLAYER_STATS),
					std::make_tuple(
							boost::bind(&mysql_db_handler::incrementPlayerStats, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_INCREMENT_PLAYER_STATS_QUIET),
					std::make_tuple(
							boost::bind(&mysql_db_handler::incrementPlayerStats, this, _1, _2),
							QUIET_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_PLAYER_GROUPS),
					std::make_tuple(
							boost::bind(&mysql_db_handler::loadPlayerGroups, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_WHITELIST_PLAYER),
					std::make_tuple(
							boost::bind(&mysql_db_handler::whitelistPlayer, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_UNWHITELIST_PLAYER),
					std::make_tuple(
							boost::bind(&mysql_db_handler::unwhitelistPlayer, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_AV_CHARS),
					std::make_tuple(
							boost::bind(&mysql_db_handler::loadAvChars, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_LINK_CHARS),
					std::make_tuple(
							boost::bind(&mysql_db_handler::linkChars, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_CHAR),
					std::make_tuple(
							boost::bind(&mysql_db_handler::loadChar, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_CHAR_CALLBACK),
					std::make_tuple(
							boost::bind(&mysql_db_handler::loadChar, this, _1, _2),
							CALLBACK_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_CREATE_CHAR),
					std::make_tuple(
							boost::bind(&mysql_db_handler::createChar, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_UPDATE_CHAR),
					std::make_tuple(
							boost::bind(&mysql_db_handler::updateChar, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_QUIET_UPDATE_CHAR),
					std::make_tuple(
							boost::bind(&mysql_db_handler::updateChar, this, _1, _2),
							QUIET_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_DECLARE_CHAR_DEATH),
					std::make_tuple(
							boost::bind(&mysql_db_handler::killChar, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_OBJECT),
					std::make_tuple(
							boost::bind(&mysql_db_handler::loadObject, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_CREATE_OBJECT),
					std::make_tuple(
							boost::bind(&mysql_db_handler::createObject, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_QUIET_CREATE_OBJECT),
					std::make_tuple(
							boost::bind(&mysql_db_handler::createObject, this, _1, _2),
							QUIET_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_UPDATE_OBJECT),
					std::make_tuple(
							boost::bind(&mysql_db_handler::updateObject, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_QUIET_UPDATE_OBJECT),
					std::make_tuple(
							boost::bind(&mysql_db_handler::updateObject, this, _1, _2),
							QUIET_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_DECLARE_OBJECT_DEATH),
					std::make_tuple(
							boost::bind(&mysql_db_handler::killObject, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_DUMP_OBJECTS),
					std::make_tuple(
							boost::bind(&mysql_db_handler::interdumpObjects, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_DUMP_OBJECTS_CALLBACK),
					std::make_tuple(
							boost::bind(&mysql_db_handler::interdumpObjectsCallback, this, _1, _2),
							CALLBACK_MAGIC)));

	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_CREATE_OBJECT_WORLD_LINK),
					std::make_tuple(
							boost::bind(&mysql_db_handler::createObjectWorldLink, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_UPDATE_OBJECT_WORLD_LINK),
					std::make_tuple(
							boost::bind(&mysql_db_handler::updateObjectWorldLink, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_DELETE_OBJECT_WORLD_LINK),
					std::make_tuple(
							boost::bind(&mysql_db_handler::deleteObjectWorldLink, this, _1, _2),
							ASYNC_MAGIC)));

	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_GET_CLAN),
					std::make_tuple(
							boost::bind(&mysql_db_handler::getClan, this, _1, _2),
							ASYNC_MAGIC)));

	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_CREATE_CLAN),
					std::make_tuple(
							boost::bind(&mysql_db_handler::createClan, this, _1, _2),
							ASYNC_MAGIC)));

	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_UPDATE_CLAN),
					std::make_tuple(
							boost::bind(&mysql_db_handler::updateClan, this, _1, _2),
							ASYNC_MAGIC)));

	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_ADD_CLAN_PLAYER),
					std::make_tuple(
							boost::bind(&mysql_db_handler::addClanPlayer, this, _1, _2),
							ASYNC_MAGIC)));

	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_UPDATE_CLAN_PLAYER),
					std::make_tuple(
							boost::bind(&mysql_db_handler::updateClanPlayer, this, _1, _2),
							ASYNC_MAGIC)));

	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_DELETE_CLAN_PLAYER),
					std::make_tuple(
							boost::bind(&mysql_db_handler::deleteClanPlayer, this, _1, _2),
							ASYNC_MAGIC)));

	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_UPDATE_PLAYER_MAIN_CLAN),
					std::make_tuple(
							boost::bind(&mysql_db_handler::updatePlayerMainClan, this, _1, _2),
							ASYNC_MAGIC)));
}

mysql_db_handler::~mysql_db_handler() {
	disconnect();

	for (auto it = objectcache.cbegin(); it != objectcache.cend(); ++it) {
		object_mysql* object = it->second;
		delete object;
		objectcache.erase(it);
	}

	for (auto it = charactercache.cbegin(); it != charactercache.cend(); ++it) {
		character_mysql* character = it->second;
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

std::string mysql_db_handler::spawnHandler(std::string& extFunction, ext_arguments& extArgument) {
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
			MYSQL* connection = connect();
#ifdef DEBUG
			logfile << "CREATED NEW DB POINTER " << static_cast<void*>(connection) << std::endl;
			logfile.flush();
#endif

			connectionpool.bounded_push((intptr_t) connection);

			if (i == 0) {
				checkMigration(extArgument, database);
				checkWorldUUID(extArgument);
				checkWorldVARIABLES(extArgument);
			}
		}

		poolinitialized = true;
	} else {
		throw std::runtime_error("Threads already spawned");
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\", " + std::to_string(i) + "]";
}

MYSQL* mysql_db_handler::connect() {
	MYSQL* connection = mysql_init(NULL);

	if (connection == NULL) {
		throw std::runtime_error("problem while initializing mysql_db_handler: " + std::string(mysql_error(connection)));
	}

	mysql_options(connection, MYSQL_OPT_RECONNECT, &reconnect);

	if (mysql_real_connect(connection, this->hostname.c_str(),
			this->user.c_str(), this->password.c_str(), this->database.c_str(),
			this->port, this->socket.c_str(), this->flag) == NULL) {
		mysql_close(connection);
		throw std::runtime_error("connection problem while initializing mysql_db_handler: " + std::string(mysql_error(connection)));
	}

	return connection;
}

void mysql_db_handler::disconnect() {
	intptr_t connectionpointer;
	MYSQL* connection;

	// prevent new requests
	poolcleanup = true;

	// while there is an handler call disconnect
	while (connectionpool.pop(connectionpointer)) {
		connection = (MYSQL*) connectionpointer;
		mysql_close(connection);
	}

	poolcleanup = false;
	poolinitialized = false;
	return;
}

MYSQL* mysql_db_handler::getconnection() {
	intptr_t connectionpointer;
	MYSQL* connection = 0;

	// try to get an db connection
	while (!connectionpool.pop(connectionpointer));
	connection = (MYSQL*) connectionpointer;

#ifdef DEBUG
	logfile << "GOT DB POINTER " << static_cast<void*>(connection) << std::endl;
	logfile.flush();
#endif

	if (connection == 0) {
		std::runtime_error("connection problem while getting mysql_db_handler: got a NULLPOINTER");
	}

	return connection;
}

void mysql_db_handler::returnconnection(MYSQL* connection) {
	intptr_t connectionpointer = (intptr_t) connection;
	connectionpool.bounded_push(connectionpointer);

#ifdef DEBUG
	logfile << "RETURNED DB POINTER " << static_cast<void*>(connection) << std::endl;
	logfile.flush();
#endif

	return;
}

void mysql_db_handler::rawmultiquerys(std::string query) {
	MYSQL* connection = getconnection();

	mysql_set_server_option(connection, MYSQL_OPTION_MULTI_STATEMENTS_ON);

#ifdef DEBUG
	logfile << "MULTI QUERY: " << query << std::endl;
	logfile.flush();
#endif

	if (mysql_real_query(connection, query.c_str(), query.size())) {
		throw std::runtime_error(
				"error while executing query: "
						+ query
						+ std::string(" ---- ERROR MESSAGE: ")
						+ std::string(mysql_error(connection)));
	}

	int status = 1;
	while (mysql_next_result(connection) == 0);

	mysql_set_server_option(connection, MYSQL_OPTION_MULTI_STATEMENTS_OFF);

	returnconnection(connection);
	return;
}

void mysql_db_handler::rawquery(std::string query) {
	MYSQL* connection = getconnection();

#ifdef DEBUG
	logfile << "QUERY: " << query << std::endl;
	logfile.flush();
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

void mysql_db_handler::rawquery(std::string query, MYSQL_RES** result) {
	MYSQL* connection = getconnection();

#ifdef DEBUG
	logfile << "QUERY " << query << std::endl;
	logfile.flush();
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
	MYSQL* connection = getconnection();
	int status;

#ifdef DEBUG
	logfile << "QUERY " << query << std::endl;
	logfile.flush();
#endif

	MYSQL_STMT* stmt;
	stmt = mysql_stmt_init(connection);
	if (!stmt) {
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

std::string mysql_db_handler::mysqlResultToArrayString(MYSQL_RES* result, char* typearray) {
	MYSQL_ROW row;

	unsigned int fieldcount;
	unsigned long long int rowcount;
	bool printcommaone = false;
	bool printcommatwo = false;

	fieldcount = mysql_num_fields(result);
	rowcount = mysql_num_rows(result);

	std::string arraystring = "[";

	for (int rowpos = 0; rowpos < rowcount; rowpos++) {
		row = mysql_fetch_row(result);

		if (printcommaone) {
			arraystring += ",";
			printcommaone = false;
			printcommatwo = false;
		}

		arraystring += "[";

		for (int fieldpos = 0; fieldpos < fieldcount; fieldpos++) {
			if (printcommatwo) {
				arraystring += ",";
				printcommatwo = false;
			}
			switch (typearray[fieldpos]) {
				case 1: arraystring += "\"";
					break;
				case 2: arraystring += "[";
					if (row[fieldpos] != NULL) {
						arraystring += "\"";
					}
					break;
				default: arraystring += "";
			}

			if (row[fieldpos] != NULL) {
				arraystring += row[fieldpos];
			} else {
				arraystring += "";
			}

			switch (typearray[fieldpos]) {
				case 1: arraystring += "\"";
					break;
				case 2:
					if (row[fieldpos] != NULL) {
						arraystring += "\"";
					}
					arraystring += "]";
					break;
			}
			printcommatwo = true;
		}
		arraystring += "]";
		printcommaone = true;
	}
	arraystring += "]";

	return arraystring;
}

void mysql_db_handler::checkMigration(ext_arguments& extArgument, std::string database) {
	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned long long int rowcount;

	/* check initial creation */
	std::string tablenames =
			str(boost::format{"SELECT `TABLE_NAME` "
							  "FROM INFORMATION_SCHEMA.TABLES "
							  "WHERE TABLE_SCHEMA = '%s'"} % database);

	this->rawquery(tablenames, &result);

	rowcount = mysql_num_rows(result);
	mysql_free_result(result);

	if (rowcount < 15) {
		this->rawmultiquerys(schema_FullTable);
#ifdef DEBUG
		logfile << "CREATED INITIAL TABLES" << std::endl;
		logfile.flush();
#endif
	}

	/* first change */
	/* first change has no schema_version, checking for existance of schema_version */
	bool firstChangeRequired = true;
	std::string fchangeCheck = "DESCRIBE world";

	this->rawquery(fchangeCheck, &result);
	rowcount = mysql_num_rows(result);

	for (int rowpos = 0; rowpos < rowcount; rowpos++) {
		row = mysql_fetch_row(result);
		if (std::string(row[0]) == "schema_version") {
			firstChangeRequired = false;
		}
	}

	mysql_free_result(result);

	if (firstChangeRequired) {
		this->rawmultiquerys(schema_migration_0001);

#ifdef DEBUG
		logfile << "DONE APPLYING MIGRATION 0001" << std::endl;
		logfile.flush();
#endif
	}
#ifdef DEBUG
	else {
		logfile << "MIGRATION 0001 ALREADY APPLIED" << std::endl;
		logfile.flush();
	}
#endif

	/* follwing changes */
	int current_schema_Version = 1;

	/* catch the case of an freh database */
	try {
		this->rawquery("SELECT MIN(schema_version) FROM `world`", &result);
		row = mysql_fetch_row(result);
		current_schema_Version = std::stoi(row[0]);
		mysql_free_result(result);
	}
	catch (const std::logic_error& e) {
#ifdef DEBUG
		logfile << "IGNORED STRING ERROR FOR EMPTY DATABASE, error was: " << e.what() << std::endl;
		logfile.flush();
#endif
	}

	switch (current_schema_Version) {
		case 1: this->rawmultiquerys(schema_migration_0002);
#ifdef DEBUG
			logfile << "APPLIED MIGRATION 0002" << std::endl;
			logfile.flush();
#endif
			[[fallthrough]];

		default: // do nothing
			break;
	}


	/* update schema version in world */
	std::string updateSchemaLibredex = str(boost::format{"UPDATE `world` SET `schema_version` = %d"} % schema_Version);
	this->rawquery(updateSchemaLibredex);
}

void mysql_db_handler::checkWorldUUID(ext_arguments& extArgument) {
	MYSQL_RES* result;
	MYSQL_ROW row;
	bool addUUID = true;
	unsigned long long int rowcount;

	std::string query =
			str(boost::format{"SELECT HEX(`world`.`uuid`) "
							  "FROM `world` "
							  "WHERE `world`.`uuid` = CAST(0x%s AS BINARY)"} % worlduuid);

	this->rawquery(query, &result);

	rowcount = mysql_num_rows(result);

	if (rowcount > 0) {
		addUUID = false;
	}

	mysql_free_result(result);


	std::string name = "empty";
	std::string map = "empty";

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_NAME) || extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_MAP)) {
		name = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_NAME);
		map = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_MAP);
	}

	mysql_binds mysql_bind = mysql_binds(5);

	mysql_bind.addData(name);
	mysql_bind.addData(map);
	mysql_bind.addData(schema_Version);
	mysql_bind.addData(DLLVERSIONSTRING);
	mysql_bind.addData(worlduuid);

	if (addUUID) {
		query = "INSERT INTO `world` (`name`, `map`, `schema_version`, `libredex_version`, `uuid`, `startuptime`) "
				"VALUES (?, ?, ?, ?, UNHEX(?), NOW())";
	} else if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_NAME) || extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_MAP)) {
		query = "UPDATE `world` "
				"SET `name` = ?, "
				"`map` = ?, "
				"`schema_version` = ?, "
				"`libredex_version` = ?, "
				"`startuptime` = NOW() "
				"WHERE `world`.`uuid` = UNHEX(?)";
	} else {
		query = "UPDATE `world` "
				"SET `schema_version` = ?, "
				"`libredex_version` = ?, "
				"`startuptime` = NOW() "
				"WHERE `world`.`uuid` = UNHEX(?)";
	}

	if (addUUID || extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_NAME) || extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_MAP)) {
		this->preparedStatementQuery(query, mysql_bind.mysql_bind);
	} else {
		this->preparedStatementQuery(query, mysql_bind.mysql_bind + 2);
	}
};

void mysql_db_handler::checkWorldVARIABLES(ext_arguments& extArgument) {
	MYSQL_RES* result;
	MYSQL_ROW row;
	bool addVariables = true;
	unsigned long long int rowcount;

	std::string query = str(boost::format{"SELECT HEX(`world_has_persistent_variables`.`persistent_variables_uuid`) "
										  "FROM `world_has_persistent_variables` "
										  "WHERE `world_has_persistent_variables`.`world_uuid` = CAST(0x%s AS BINARY)"} % worlduuid);

	this->rawquery(query, &result);

	rowcount = mysql_num_rows(result);

	if (rowcount > 0) {
		addVariables = false;
	}

	mysql_free_result(result);

	std::string variables = "[]";
	std::string persistent_variables_uuid = orderedUUID();

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_VARIABLES)) {
		variables = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_VARIABLES);
	}

	mysql_binds mysql_bind = mysql_binds(4);

	mysql_bind.addData(worlduuid);
	mysql_bind.addData(persistent_variables_uuid);
	mysql_bind.addData(variables);
	mysql_bind.addData(worlduuid);

	if (addVariables) {
		query = "INSERT INTO `persistent_variables` (`uuid`, `persistentvariables`) VALUES (UNHEX(?), ?)";
		this->preparedStatementQuery(query, mysql_bind.mysql_bind + 1);

		query = "INSERT INTO `world_has_persistent_variables` (`world_uuid`, `persistent_variables_uuid`) VALUES (UNHEX(?), UNHEX(?));";
		this->preparedStatementQuery(query, mysql_bind.mysql_bind);

	} else if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_VARIABLES)) {
		// update variables
		std::string query = "UPDATE `persistent_variables` SET `persistentvariables` = ? WHERE uuid = ("
							"SELECT `persistent_variables_uuid` FROM `world_has_persistent_variables` WHERE `world_has_persistent_variables`.`world_uuid` = UNHEX(?)"
							");";

		this->preparedStatementQuery(query, mysql_bind.mysql_bind + 2);
	}
}

std::string mysql_db_handler::terminateHandler(std::string& extFunction, ext_arguments& extArgument) {
	this->terminateHandler();
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"DONE\"]";
}

void mysql_db_handler::terminateHandler() {
	//std::cout << "TERMINATE MYSQL" << std::endl;
#ifdef DEBUG
	logfile << "TERMINATE MYSQL" << std::endl;
	logfile.flush();
#endif
	disconnect();
	return;
}

std::string mysql_db_handler::intergetUUID(std::string& extFunction, ext_arguments& extArgument) {
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + orderedUUID() + "\"]";
}

std::string mysql_db_handler::interecho(std::string& extFunction, ext_arguments& extArgument) {
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + extArgument.get<std::string>("echostring") + "\"]";
}

std::string mysql_db_handler::interdbVersion(std::string& extFunction, ext_arguments& extArgument) {
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + querydbversion() + "\"]";
}

std::string mysql_db_handler::interdumpObjects(std::string& extFunction, ext_arguments& extArgument) {
	std::string matrix;
	bool placecommaone = false;

	std::vector<object_mysql*> objectList = dumpObjects(extArgument);
	matrix = "[";
	for (object_mysql* object : objectList) {
		if (placecommaone) {
			matrix += ",";
		}

		matrix += object->getAsArmaString();

		placecommaone = true;
	}
	matrix += "]";

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + matrix + "]";
}

std::string mysql_db_handler::interdumpObjectsCallback(std::string& extFunction, ext_arguments& extArgument) {
	std::string callbackfunction = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_CALLBACK_FUNCTION);

	std::vector<object_mysql*> objectList = dumpObjects(extArgument);

	for (object_mysql* object : objectList) {
		if (callbackPtr != nullptr) {
			callbackPtr(PROTOCOL_CALLBACK_NAME_CALLIN, callbackfunction.c_str(), object->getAsArmaString().c_str());
		}
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",[0,\"DONE DUMPING OBJECTS\", " + std::to_string(objectList.size()) + "]]";
}

/* SQL Querys */

std::string mysql_db_handler::querydbversion() {
	MYSQL_RES* result;
	MYSQL_ROW row;
	std::string version;

	this->rawquery("SELECT VERSION()", &result);

	row = mysql_fetch_row(result);

	version = row[0];

	mysql_free_result(result);

	return version;
}

std::string mysql_db_handler::setWorldVariables(std::string& extFunction, ext_arguments& extArgument) {
	this->checkWorldVARIABLES(extArgument);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"DONE\"]";
}

std::string mysql_db_handler::getWorldVariables(std::string& extFunction, ext_arguments& extArgument) {
	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned long long int rowcount;
	std::string variables = "[]";

	std::string worlduuid = this->worlduuid;

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_WORLDUUID)) {
		worlduuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_WORLDUUID);
	}

	mysql_binds mysql_bind = mysql_binds(1);

	mysql_bind.addData(worlduuid);

	std::string query = str(boost::format{"SELECT `persistentvariables` FROM `persistent_variables` WHERE uuid = ("
						"SELECT `persistent_variables_uuid` FROM `world_has_persistent_variables` WHERE `world_has_persistent_variables`.`world_uuid` = CAST(0x%s AS BINARY)"
						");"} % worlduuid);

	this->rawquery(query, &result);

	rowcount = mysql_num_rows(result);

	if (rowcount > 0) {
		row = mysql_fetch_row(result);
		if (row[0] != NULL) {
			variables = row[0];
		}
	}

	mysql_free_result(result);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + variables + "]";
}

std::string mysql_db_handler::setWorldState(std::string& extFunction, ext_arguments& extArgument) {

	int state = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_STATE_VALUE);

	std::string query = str(boost::format{
			"UPDATE `world` SET `state` = %d "
			"WHERE `uuid` = CAST(0x%s AS BINARY)"} % state % worlduuid);

	this->rawquery(query);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"DONE\"]";
}

std::string mysql_db_handler::getLinkedWorlds(std::string& extFunction, ext_arguments& extArgument) {
	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	bool printcommaone = false;
	bool printcommatwo = false;

	// char info
	std::string charinfo = "";

	std::string querycharinfo =
			str(
					boost::format{
							"SELECT HEX(`world`.`uuid`), `world`.`name`, `world`.`map` , `world`.`latitude`, "
							"`world`.`longitude`, `world`.`state` , `world`.`ip`, "
							"`world`.`port`, `world`.`password` , `world`.`mods` "
							"FROM `world_is_linked_to_world` "
							"INNER JOIN `world` "
							" ON `world`.`uuid` = `world_is_linked_to_world`.`world_uuid2` "
							"WHERE `world_is_linked_to_world`.`world_uuid1` = CAST(0x%s AS BINARY) "}
							% worlduuid);

	char typearray[] = {
			1, // HEX(`world`.`uuid`)
			1, // `world`.`name`
			1, // `world`.`map`
			1, // `world`.`latitude`
			1, // `world`.`longitude`
			1, // `world`.`state`
			1, // `world`.`ip`
			1, // `world`.`port`
			1, // `world`.`password`
			1  // `world`.`mods`
	};

	this->rawquery(querycharinfo, &result);

	charinfo = mysqlResultToArrayString(result, typearray);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + charinfo + "]";
}

std::string mysql_db_handler::loadPlayer(std::string& extFunction, ext_arguments& extArgument) {
	std::string nickname = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_NICKNAME);
	std::string steamid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_STEAMID);

	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;

	// player info
	std::string playeruuid = "";
	std::string persistent_variables_uuid = "";
	std::string mainclanuuid = "";
	std::string banned = "false";
	std::string banreason = "unknown";
	std::string targetworld_uuid = "";

	std::string queryplayerinfo =
			str(boost::format{"SELECT HEX(`player`.`uuid`), "
							  "HEX(`player_on_world_has_persistent_variables`.`persistent_variables_uuid`), "
							  "HEX(`player`.`mainclan_uuid`), "
							  "(CASE WHEN (NOW() < `player`.`banenddate`) THEN \"true\" ELSE \"false\" END) AS BANNED, "
							  "`player`.`banreason`, "
							  "HEX(`player`.`targetworld_uuid`) "
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
		if (row[5] != NULL) {
			targetworld_uuid = row[5];
		}
	}

	mysql_free_result(result);

	if (banned == "false") {
		mysql_binds mysql_bind = mysql_binds(4);
		mysql_bind.addData(steamid);
		mysql_bind.addData(nickname);
		mysql_bind.addData(nickname);

		if (playeruuid == "") {
			playeruuid = orderedUUID();
			mysql_bind.addData(playeruuid);

			queryplayerinfo = "INSERT INTO `player` (`steamid`, `battleyeid`, "
							  "`firstlogin`, `firstnick`, `lastlogin`, `lastnick`, "
							  "`bancount`, `banreason`, `banbegindate`, `banenddate`, `uuid`) "
							  "VALUES (?, \"unused\", NOW(), "
							  "?, NOW(), ?, '0', NULL, NULL, NULL, UNHEX(?))";

			this->preparedStatementQuery(queryplayerinfo, mysql_bind.mysql_bind);
		} else {
			mysql_bind.addData(playeruuid);

			queryplayerinfo ="UPDATE `player` SET `lastlogin` = NOW(), `lastnick` = ? WHERE `player`.`uuid` = UNHEX(?)";
			this->preparedStatementQuery(queryplayerinfo, mysql_bind.mysql_bind+2);
		}



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

	std::string playerinfo = "[\"" + playeruuid + "\",\"" + persistent_variables_uuid + "\",\"" + mainclanuuid + "\",[" + banned + ",\"" + banreason + "\"], \"" + targetworld_uuid + "\"]";
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + playerinfo + "]";
}

std::string mysql_db_handler::logoutPlayer(std::string& extFunction, ext_arguments& extArgument) {
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);

	std::string queryplayerinfo =
			str(boost::format{"UPDATE `player` SET `lastlogout` = NOW(), "
							  "`playtime` = `playtime` + TIME_TO_SEC(TIMEDIFF(NOW(), `lastlogin`)), "
							  "`targetworld_uuid` = NULL "
							  "WHERE `player`.`uuid` = CAST(0x%s AS BINARY)"} % playeruuid);

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_PLAYER_TARGETWORLD_UUID)) {
		std::string targetworld_uuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_TARGETWORLD_UUID);
		queryplayerinfo =
				str(boost::format{"UPDATE `player` SET `lastlogout` = NOW(), "
								  "`playtime` = `playtime` + TIME_TO_SEC(TIMEDIFF(NOW(), `lastlogin`)), "
								  "`targetworld_uuid` = CAST(0x%s AS BINARY) "
								  "WHERE `player`.`uuid` = CAST(0x%s AS BINARY)"} % targetworld_uuid % playeruuid);
	}

	this->rawquery(queryplayerinfo);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"DONE\"]";
}

std::string mysql_db_handler::incrementPlayerStats(std::string& extFunction, ext_arguments& extArgument) {
	bool commit = false;

	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);

	std::string queryplayerinfo = "UPDATE `player` SET ";

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_PLAYER_AIKILLS)) {
		commit = true;
		queryplayerinfo += str(boost::format{" `aikills` = `aikills` + %d,"} % extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_PLAYER_AIKILLS));
	}
	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_PLAYER_NONAIKILLS)) {
		commit = true;
		queryplayerinfo += str(boost::format{" `nonaikills` = `nonaikills` + %d,"} % extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_PLAYER_NONAIKILLS));
	}
	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_PLAYER_DEATHS)) {
		commit = true;
		queryplayerinfo += str(boost::format{" `deaths` = `deaths` + %d,"} % extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_PLAYER_DEATHS));
	}

	if (commit) {
		queryplayerinfo += str(boost::format{" `battleyeid` = `battleyeid` WHERE `player`.`uuid` = CAST(0x%s AS BINARY)"} % playeruuid);
		this->rawquery(queryplayerinfo);
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"DONE\"]";
}
std::string mysql_db_handler::loadPlayerGroups(std::string& extFunction, ext_arguments& extArgument) {
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);

	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	bool printcommaone = false;
	bool printcommatwo = false;

	// char info
	std::string groupinfo = "";

	std::string querygroupinfo =
			str(
					boost::format{
							"SELECT HEX(`clan`.`uuid`) AS 'clan_uuid', HEX(`founder`.`uuid`) AS 'founder_uuid', `founder`.`steamid` AS 'founder_steamid', "
							"`founder`.`lastnick` AS 'founder_nick', GROUP_CONCAT("
							"DISTINCT CONCAT('[\"', HEX(`player`.`uuid`), '\",\"' , `player`.`steamid`, '\",\"' , `player`.`lastnick`, '\",', "
							"`clan_member`.`rank`, ',\"', `clan_member`.`comment`, '\"]') "
							"SEPARATOR ','"
							") AS 'clan_member_list', `persistent_variables`.`persistentvariables` AS 'clan_variables' "
							"FROM `clan` LEFT JOIN `player` AS `founder` ON `clan`.`founder_uuid` = `founder`.`uuid` "
							"LEFT JOIN `clan_member` ON `clan`.`uuid` = `clan_member`.`clan_uuid` "
							"LEFT JOIN `player` ON `clan_member`.`player_uuid` = `player`.`uuid` "
							"LEFT JOIN `clan_has_persistent_variables` ON `clan`.`uuid` = `clan_has_persistent_variables`.`clan_uuid`"
							"LEFT JOIN `persistent_variables` ON `persistent_variables`.`uuid` = `clan_has_persistent_variables`.`persistent_variables_uuid`"
							"WHERE `clan`.`uuid` IN ("
							"SELECT `clan_member`.`clan_uuid` "
							"FROM `clan_member` "
							"WHERE `clan_member`.`player_uuid` = CAST(0x%s AS BINARY)"
							")"
							"GROUP BY `clan`.`uuid`"}
							% playeruuid);

	char typearray[] = {
			1, // HEX(`clan`.`uuid`)
			1, // HEX(`founder`.`uuid`)
			1, // `founder`.`steamid`
			1, // `founder`.`lastnick`
			3, // 'clan_member_list'
			1, // 'clan_variables'
	};

	this->rawquery(querygroupinfo, &result);

	groupinfo = mysqlResultToArrayString(result, typearray);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + groupinfo + "]";
}

std::string mysql_db_handler::whitelistPlayer(std::string& extFunction, ext_arguments& extArgument) {
	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;

	std::string extplayeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string playeruuid = "";

	std::string queryplayerinfo = str(boost::format{"SELECT HEX(`player`.`uuid`), "
													"FROM `player` "
													"WHERE `player`.`uuid` = CAST(0x%s AS BINARY) "} % extplayeruuid);

	this->rawquery(queryplayerinfo, &result);

	rowcount = mysql_num_rows(result);

	if (rowcount > 0) {
		row = mysql_fetch_row(result);
		if (row[0] != NULL) {
			playeruuid = row[0];
		}
	}

	mysql_free_result(result);

	if (playeruuid != "") {
		std::string querywhitelist = str(boost::format{"INSER INTO `whitelist` (`world_uuid`, `player_uuid`)"
													   "VALUES (CAST(0x%s AS BINARY), CAST(0x%s AS BINARY))"} % worlduuid % playeruuid);

		this->rawquery(querywhitelist);

	} else {
		return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"player does not exist!\"]";
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"DONE\"]";
}

std::string mysql_db_handler::unwhitelistPlayer(std::string& extFunction, ext_arguments& extArgument) {
	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;

	std::string extplayeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string playeruuid = "";

	std::string queryplayerinfo = str(boost::format{"SELECT HEX(`player`.`uuid`), "
													"FROM `player` "
													"WHERE `player`.`uuid` = CAST(0x%s AS BINARY) "} % extplayeruuid);

	this->rawquery(queryplayerinfo, &result);

	rowcount = mysql_num_rows(result);

	if (rowcount > 0) {
		row = mysql_fetch_row(result);
		if (row[0] != NULL) {
			playeruuid = row[0];
		}
	}

	mysql_free_result(result);

	if (playeruuid != "") {
		std::string querywhitelist =
				str(boost::format{"DELETE FROM `whitelist` "
								  "WHERE `whitelist`.`world_uuid` = CAST(0x%s AS BINARY) "
								  " AND `whitelist`.`player_uuid` = CAST(0x%s AS BINARY) "} % worlduuid % playeruuid);

		this->rawquery(querywhitelist);

	} else {
		return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"player does not exist!\"]";
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"DONE\"]";
}

std::string mysql_db_handler::loadAvChars(std::string& extFunction, ext_arguments& extArgument) {
	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	bool printcommaone = false;
	bool printcommatwo = false;

	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);

	// char info
	std::string charinfo = "";

	std::string querycharinfo =
			str(
					boost::format{
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
							" AND `player_on_world_has_persistent_variables`.`player_uuid` = CAST(0x%s AS BINARY)"}
							% worlduuid % playeruuid);

	char typearray[] = {
			1, // HEX(`persistent_variables`.`uuid`)
			0, // `persistent_variables`.`persistentvariables`
			1, // `world`.`name`
			1  // `world`.`map`
	};

	this->rawquery(querycharinfo, &result);

	charinfo = mysqlResultToArrayString(result, typearray);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + charinfo + "]";
}

std::string mysql_db_handler::linkChars(std::string& extFunction, ext_arguments& extArgument) {
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string variabuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_VARIABUUID);

	std::string query = str(
			boost::format{"INSERT INTO `player_on_world_has_persistent_variables` "
						  "(`player_uuid`, `world_uuid`, `persistent_variables_uuid`) "
						  "VALUES (CAST(0x%s AS BINARY), CAST(0x%s AS BINARY), CAST(0x%s AS BINARY))"
			} % playeruuid % worlduuid % variabuuid);

	this->rawquery(query);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + playeruuid + "\"]";
}

std::string mysql_db_handler::loadChar(std::string& extFunction, ext_arguments& extArgument) {
	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	bool printcomma = false;
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string characterdata = "[]";
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
												  " AND `player_on_world_has_character`.`killinfo_uuid` IS NULL"} % playeruuid % worlduuid);

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
		characterdata = character->getAsArmaString();
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + characterdata + "]";
}

std::string mysql_db_handler::createChar(std::string& extFunction, ext_arguments& extArgument) {
	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;

	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);

	// char info
	std::string charuuid = "";
	std::string persistent_variables_uuid = "";
	std::string shareable_variables_uuid = "";

	/* for testing purpose, check if there is an existing character, if yes return its uuid */
	std::string query = str(boost::format{"SELECT HEX(`character_uuid`) "
										  "FROM `player_on_world_has_character` "
										  "WHERE `player_on_world_has_character`.`player_uuid` = CAST(0x%s AS BINARY) "
										  "AND `player_on_world_has_character`.`world_uuid` =  CAST(0x%s AS BINARY) "
										  "AND `player_on_world_has_character`.`killinfo_uuid` IS NULL"} % playeruuid % worlduuid);

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
		query = str(boost::format{"SELECT HEX(`persistent_variables_uuid`) "
								  "FROM `player_on_world_has_persistent_variables` "
								  "WHERE `player_uuid` = CAST(0x%s AS BINARY) "
								  "AND `world_uuid` = CAST(0x%s AS BINARY)"} % playeruuid % worlduuid);

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

			this->preparedStatementQuery(query, character->mysql_bind + 14);

			query = str(boost::format{"INSERT INTO `player_on_world_has_persistent_variables` "
									  "(`player_uuid`, `world_uuid`, `persistent_variables_uuid`) "
									  "VALUES (CAST(0x%s AS BINARY), "
									  "CAST(0x%s AS BINARY), "
									  "CAST(0x%s AS BINARY));"} % playeruuid % worlduuid % persistent_variables_uuid);

			this->rawquery(query);
		} else {
			character->setData(PROTOCOL_DBCALL_ARGUMENT_PERSISTENTVARIABUUID, persistent_variables_uuid);
			query = str(boost::format{"SELECT `persistentvariables` "
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
		query = str(boost::format{"SELECT HEX(`charactershareables`.`uuid`) "
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
								  ") LIMIT 1"} % playeruuid % persistent_variables_uuid);

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
					boost::format{"INSERT INTO `charactershareables` (`classname`, `hitpoints`, "
								  "`variables`, `textures`, `gear`, "
								  "`currentweapon`, `uuid`, `persistent_variables_uuid`) "
								  "VALUES (?, ?, ?, ?, ?, ?, UNHEX(?), CAST(0x%s AS BINARY))"} % persistent_variables_uuid);

			this->preparedStatementQuery(query, character->mysql_bind + 7);
		} else {
			character->setData(PROTOCOL_DBCALL_ARGUMENT_CHARSHAREUUID, shareable_variables_uuid);

			query = str(boost::format{"SELECT `classname`, `hitpoints`, "
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
				boost::format{"INSERT INTO `character` (`animationstate`, `direction`, "
							  "`positiontype`, `positionx`, `positiony`, "
							  "`positionz`, `uuid`, `charactershareables_uuid`) "
							  "VALUES (?, ?, ?, ?, ?, ?, "
							  "UNHEX(?), CAST(0x%s AS BINARY))"} % shareable_variables_uuid);

		this->preparedStatementQuery(query, character->mysql_bind);

		query = str(boost::format{"INSERT INTO `player_on_world_has_character` (`player_uuid`, `world_uuid`, "
								  "`character_uuid`, `killinfo_uuid`) "
								  "VALUES (CAST(0x%s AS BINARY), "
								  "CAST(0x%s AS BINARY), "
								  "CAST(0x%s AS BINARY), "
								  "NULL)"} % playeruuid % worlduuid % charuuid);

		this->rawquery(query);
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + charuuid + "\"]";
}

std::string mysql_db_handler::updateChar(std::string& extFunction, ext_arguments& extArgument) {
	std::string charuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_CHARUUID);
	character_mysql* character = 0;

	auto it = charactercache.find(charuuid);
	if (it != charactercache.end()) {
		character = static_cast<character_mysql*>((void*) it->second);
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

	this->preparedStatementQuery(query, character->mysql_bind + 7);

	query = "UPDATE `persistent_variables` "
			"SET `persistentvariables` = ? "
			"WHERE `persistent_variables`.`uuid` = UNHEX(?)";

	this->preparedStatementQuery(query, character->mysql_bind + 14);

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_PARENTUUID)) {
		std::string parentuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PARENTUUID);
		char* charactershareablesuuid = (char*) character->mysql_bind[13].buffer;

		std::string query;
		if (parentuuid != "") {
			query = str(boost::format{"UPDATE `charactershareables` SET `object_uuid` = CAST(0x%s AS BINARY) "
									  "WHERE `charactershareables`.`uuid` = CAST(0x%s AS BINARY);"} % parentuuid % charactershareablesuuid);
		} else {
			query = str(boost::format{"UPDATE `charactershareables` SET `object_uuid` = NULL "
									  "WHERE `charactershareables`.`uuid` = CAST(0x%s AS BINARY);"} % charactershareablesuuid);
		}
		this->rawquery(query);
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + charuuid + "\"]";
}

std::string mysql_db_handler::killChar(std::string& extFunction, ext_arguments& extArgument) {
	mysql_binds mysql_bind = mysql_binds(7);
	std::string killuuid = orderedUUID();
	std::string charuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_CHARUUID);
	std::string attackeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_ATTACKER);
	std::string type = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_TYPE);
	std::string weapon = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_WEAPON);
	float distance = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DISTANCE);

	if (attackeruuid == "") {
		mysql_bind.addNull();
	} else {
		mysql_bind.addData(attackeruuid);
	}

	if (type == "") {
		mysql_bind.addNull();
	} else {
		mysql_bind.addData(type);
	}

	if (weapon == "") {
		mysql_bind.addNull();
	} else {
		mysql_bind.addData(weapon);
	}

	mysql_bind.addData(distance);

	mysql_bind.addData(killuuid);
	mysql_bind.addData(charuuid);

	std::string query = "INSERT INTO `killinfo` "
						"(`date`, `attacker_uuid`, `type`, `weapon`, `distance`, `uuid`) "
						"VALUES (CURRENT_TIMESTAMP, UNHEX(?), ?, ?, ?, UNHEX(?));";
	this->preparedStatementQuery(query, mysql_bind.mysql_bind);

	query = "UPDATE `player_on_world_has_character` "
			"SET `killinfo_uuid` = UNHEX(?) "
			"WHERE `player_on_world_has_character`.`character_uuid` IN "
			"(SELECT `character`.`uuid` FROM `character` WHERE `charactershareables_uuid` = "
			"(SELECT `character`.`charactershareables_uuid` FROM `character` WHERE `uuid` = UNHEX(?)));";

	this->preparedStatementQuery(query, mysql_bind.mysql_bind+4);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + killuuid + "\"]";
}

std::string mysql_db_handler::loadObject(std::string& extFunction, ext_arguments& extArgument) {
	std::string objectuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	object_mysql* object = 0;

	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	std::string objinfo = "[]";

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
				object = static_cast<object_mysql*>((void*) it->second);
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
		objinfo = object->getAsArmaString();
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + objinfo + "]";
}

std::string mysql_db_handler::createObject(std::string& extFunction, ext_arguments& extArgument) {
	std::string objectuuid;

	object_mysql* object = new object_mysql;
	object->setData(extArgument);

	try {
		objectuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	}
	catch (std::runtime_error const& e) {
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

	query = str(boost::format{"INSERT INTO `world_has_objects` (`world_uuid`, `object_uuid`) "
							  "VALUES (CAST(0x%s AS BINARY), CAST(0x%s AS BINARY))"} % worlduuid % objectuuid);

	this->rawquery(query);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + objectuuid + "\"]";
}

std::string mysql_db_handler::updateObject(std::string& extFunction, ext_arguments& extArgument) {
	std::string objectuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	object_mysql* object = 0;

	auto it = objectcache.find(objectuuid);
	if (it != objectcache.end()) {
		object = static_cast<object_mysql*>((void*) it->second);
	} else {
		throw std::runtime_error("could not find object to update: " + objectuuid);
	}

	object->setData(extArgument);

	std::string query = "UPDATE `object` "
						"SET `classname` = ?, "
						"    `priority` = ?, "
						"    `timelastused` = NOW(), "
						"    `type` = ?, "
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

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_PARENTUUID)) {
		std::string parentuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PARENTUUID);
		std::string query;
		if (parentuuid != "") {
			query = str(boost::format{"UPDATE `world_has_objects` SET `parentobject_uuid` = CAST(0x%s AS BINARY) "
									  "WHERE `world_has_objects`.`object_uuid` = CAST(0x%s AS BINARY);"} % parentuuid % objectuuid);
		} else {
			query = str(boost::format{"UPDATE `world_has_objects` SET `parentobject_uuid` = NULL "
									  "WHERE `world_has_objects`.`object_uuid` = CAST(0x%s AS BINARY);"} % objectuuid);
		}
		this->rawquery(query);
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + objectuuid + "\"]";
}

std::string mysql_db_handler::killObject(std::string& extFunction, ext_arguments& extArgument) {
	mysql_binds mysql_bind = mysql_binds(7);

	std::string killuuid = orderedUUID();
	std::string objectuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	std::string attackeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_ATTACKER);
	std::string type = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_TYPE);
	std::string weapon = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_WEAPON);
	float distance = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DISTANCE);

	if (attackeruuid == "") {
		mysql_bind.addNull();
	} else {
		mysql_bind.addData(attackeruuid);
	}

	if (type == "") {
		mysql_bind.addNull();
	} else {
		mysql_bind.addData(type);
	}

	if (weapon == "") {
		mysql_bind.addNull();
	} else {
		mysql_bind.addData(weapon);
	}

	mysql_bind.addData(distance);

	mysql_bind.addData(killuuid);
	mysql_bind.addData(objectuuid);

	std::string query = "INSERT INTO `killinfo` "
						"(`date`, `attacker_uuid`, `type`, `weapon`, `distance`, `uuid`) "
						"VALUES (CURRENT_TIMESTAMP, UNHEX(?), ?, ?, ?, UNHEX(?));";
	this->preparedStatementQuery(query, mysql_bind.mysql_bind);

	query = "UPDATE `world_has_objects` SET `killinfo_uuid` = UNHEX(?) WHERE `world_has_objects`.`object_uuid` = UNHEX(?);";
	this->preparedStatementQuery(query, mysql_bind.mysql_bind+4);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + killuuid + "\"]";
}

std::vector<object_mysql*> mysql_db_handler::dumpObjects(ext_arguments& extArgument) {
	std::vector<object_mysql*> objectList;
	MYSQL_RES* result;
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
							  "HEX(`player`.`mainclan_uuid`) "
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

std::string mysql_db_handler::createObjectWorldLink(std::string& extFunction, ext_arguments& extArgument) {
	std::string objectuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	std::string worlduuid = this->worlduuid;
	MYSQL_RES* result;
	unsigned long long int rowcount;

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_WORLDUUID)) {
		worlduuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_WORLDUUID);
	}

	std::string query = str(boost::format{"SELECT HEX(`object_uuid`) "
										  "FROM `world_has_objects` "
										  "WHERE `world_has_objects`.`object_uuid` = CAST(0x%s AS BINARY) "
										  "AND `world_has_objects`.`world_uuid` =  CAST(0x%s AS BINARY) "
										  "AND `world_has_objects`.`killinfo_uuid` IS NULL"} % objectuuid % worlduuid);

	this->rawquery(query, &result);

	rowcount = mysql_num_rows(result);
	mysql_free_result(result);

	if (rowcount < 1) {

		std::string query = str(boost::format{"INSERT INTO `world_has_objects` (`world_uuid`, `object_uuid`) "
											  "VALUES (CAST(0x%s AS BINARY), CAST(0x%s AS BINARY))"} % worlduuid % objectuuid);

		this->rawquery(query);
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",[\"" + objectuuid + "\",\"" + worlduuid + "\"]]";
}

std::string mysql_db_handler::updateObjectWorldLink(std::string& extFunction, ext_arguments& extArgument) {
	std::string objectuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	std::string worlduuid = this->worlduuid;

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_WORLDUUID)) {
		worlduuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_WORLDUUID);
	}

	std::string query = str(boost::format{"UPDATE `world_has_objects` SET `world_uuid` = CAST(0x%s AS BINARY) "
										  "WHERE `world_has_objects`.`object_uuid` = CAST(0x%s AS BINARY);"} % worlduuid % objectuuid);
	this->rawquery(query);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",[\"" + objectuuid + "\",\"" + worlduuid + "\"]]";
}

std::string mysql_db_handler::deleteObjectWorldLink(std::string& extFunction, ext_arguments& extArgument) {
	std::string objectuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	std::string worlduuid = "*";

	std::string query = str(boost::format{"DELETE FROM `world_has_objects` "
										  "WHERE `world_has_objects`.`object_uuid` = CAST(0x%s AS BINARY);"} % objectuuid);

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_WORLDUUID)) {
		worlduuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_WORLDUUID);
		query = str(boost::format{"DELETE FROM `world_has_objects` SET "
								  "WHERE `world_has_objects`.`object_uuid` = CAST(0x%s AS BINARY)"
								  "AND `world_uuid` = CAST(0x%s AS BINARY);"} % objectuuid % worlduuid);
	}

	this->rawquery(query);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",[\"" + objectuuid + "\",\"" + worlduuid + "\"]]";
}

void mysql_db_handler::failIfClanNotExists(std::string clanuuid) {
	MYSQL_RES* result;
	unsigned long long int rowcount = 0;

	if (!clanuuid.empty()) {
		std::string queryclaninfo =
				str(boost::format{"SELECT HEX(`clan`.`uuid`) "
								  "FROM `clan` "
								  "WHERE `clan`.`uuid` = CAST(0x%s AS BINARY)"} % clanuuid);

		this->rawquery(queryclaninfo, &result);

		rowcount = mysql_num_rows(result);
		mysql_free_result(result);
	}

	if (rowcount < 1) {
		throw std::runtime_error("could not find clan with uuid: " + clanuuid);
	}

	return;
}

void mysql_db_handler::failIfClanNameNotUnique(std::string clanname) {
	MYSQL_RES* result;
	unsigned long long int rowcount;

	std::string queryclaninfo =
			str(boost::format{"SELECT HEX(`clan`.`uuid`) "
							  "FROM `clan` "
							  "WHERE LOWER(`clan`.`name`) = LOWER(\"%s\")"} % clanname);

	this->rawquery(queryclaninfo, &result);

	rowcount = mysql_num_rows(result);
	mysql_free_result(result);

	if (rowcount > 0) {
		throw std::runtime_error("clan name already taken: " + clanname);
	}

	return;
}

bool mysql_db_handler::playerMemberOfClan(std::string clanuuid, std::string playeruuid) {
	MYSQL_RES* result;
	unsigned long long int rowcount = 0;

	if (!clanuuid.empty()) {
		std::string query = str(boost::format{"SELECT HEX(`clan_uuid`) "
											  "FROM `clan_member` "
											  "WHERE `clan_member`.`clan_uuid` = CAST(0x%s AS BINARY) "
											  "AND `clan_member`.`player_uuid` =  CAST(0x%s AS BINARY) "} % clanuuid % playeruuid);

		this->rawquery(query, &result);

		rowcount = mysql_num_rows(result);
		mysql_free_result(result);
	}
	if (rowcount < 1) {
		return false;
	}

	return true;
}

std::string mysql_db_handler::getClan(std::string& extFunction, ext_arguments& extArgument) {
	std::string clanuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_CLAN_UUID);
	std::string clanname = "";
	std::string founderuuid = "";
	std::string foundersteamid = "";
	std::string foundernickname = "";
	std::string clanmemberlist = "";
	std::string clanvariables = "[]";

	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount = 0;

	if (!clanuuid.empty()) {
		std::string queryclaninfo =
				str(boost::format{"SELECT `clan`.`name` AS 'clan_name', HEX(`founder`.`uuid`) AS 'founder_uuid', `founder`.`steamid` AS 'founder_steamid', "
								  "`founder`.`lastnick` AS 'founder_nick', GROUP_CONCAT("
								  "DISTINCT CONCAT('[\"', HEX(`player`.`uuid`), '\",\"' , `player`.`steamid`, '\",\"' , `player`.`lastnick`, '\",', "
								  "`clan_member`.`rank`, ',\"', `clan_member`.`comment`, '\"]') "
								  "SEPARATOR ','"
								  ") AS 'clan_member_list', `persistent_variables`.`persistentvariables` AS 'clan_variables' "
								  "FROM `clan` "
		                          "LEFT JOIN `player` AS `founder` ON `clan`.`founder_uuid` = `founder`.`uuid` "
								  "LEFT JOIN `clan_member` ON `clan`.`uuid` = `clan_member`.`clan_uuid` "
								  "LEFT JOIN `player` ON `clan_member`.`player_uuid` = `player`.`uuid`"
								  "LEFT JOIN `clan_has_persistent_variables` ON `clan`.`uuid` = `clan_has_persistent_variables`.`clan_uuid`"
								  "LEFT JOIN `persistent_variables` ON `persistent_variables`.`uuid` = `clan_has_persistent_variables`.`persistent_variables_uuid`"
								  "WHERE `clan`.`uuid` = CAST(0x%s AS BINARY)"} % clanuuid);

		this->rawquery(queryclaninfo, &result);

		rowcount = mysql_num_rows(result);
	}

	if (rowcount < 1) {
		throw std::runtime_error("could not find clan with uuid: " + clanuuid);
	}

	row = mysql_fetch_row(result);

	if (row[0] != NULL) {
		clanname = row[0];
	}

	if (row[1] != NULL) {
		founderuuid = row[1];
	}

	if (row[2] != NULL) {
		foundersteamid = row[2];
	}

	if (row[3] != NULL) {
		foundernickname = row[3];
	}

	if (row[4] != NULL) {
		clanmemberlist = row[4];
	}

	if (row[5] != NULL) {
		clanvariables = row[5];
	}

	mysql_free_result(result);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",[\"" + clanuuid + "\",\"" + clanname + "\",\"" + founderuuid + "\",\"" + foundersteamid + "\",\"" + foundernickname + "\",[" + clanmemberlist + "]," + clanvariables + "]]";
}

std::string mysql_db_handler::createClan(std::string& extFunction, ext_arguments& extArgument) {
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string clanname = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_CLANNAME);
	std::string clanvariables = "[]";
	std::string clanuuid = orderedUUID();
	std::string persistent_variables_uuid = orderedUUID();

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_CLAN_UUID)) {
		clanuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_CLAN_UUID);
	}

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_VARIABLES)) {
		clanvariables = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_VARIABLES);
	}


	MYSQL_RES* result;
	unsigned long long int rowcount;

	// only create clan if owner actually exists
	std::string queryplayerinfo =
			str(boost::format{"SELECT HEX(`player`.`uuid`) "
							  "FROM `player` "
							  "WHERE `player`.`uuid` = CAST(0x%s AS BINARY)"} % playeruuid);

	this->rawquery(queryplayerinfo, &result);

	rowcount = mysql_num_rows(result);
	mysql_free_result(result);

	if (rowcount < 1) {
		throw std::runtime_error("could not find player with uuid: " + playeruuid);
	}

	// prevent name duplication case insensitive
	this->failIfClanNameNotUnique(clanname);

	mysql_binds mysql_bind = mysql_binds(5);
	mysql_bind.addData(playeruuid);
	mysql_bind.addData(clanname);
	mysql_bind.addData(clanuuid);

	mysql_bind.addData(persistent_variables_uuid);
	mysql_bind.addData(clanvariables);

	// add clan
	std::string query = "INSERT INTO `clan` (`founder_uuid`, `name`, `uuid`) "
						"VALUES (UNHEX(?), ?, UNHEX(?))";
	this->preparedStatementQuery(query, mysql_bind.mysql_bind);

	// add owner as member
	query = "INSERT INTO `clan_member` (`player_uuid`, `rank`, `comment`, `clan_uuid`) "
			"VALUES (UNHEX(?), 0, ?, UNHEX(?))";
	this->preparedStatementQuery(query, mysql_bind.mysql_bind);

	query = "INSERT INTO `persistent_variables` (`uuid`, `persistentvariables`) VALUES (UNHEX(?), ?)";
	this->preparedStatementQuery(query, mysql_bind.mysql_bind+3);

	query = "INSERT INTO `clan_has_persistent_variables` (`clan_uuid`, `persistent_variables_uuid`) VALUES (UNHEX(?), UNHEX(?));";
	this->preparedStatementQuery(query, mysql_bind.mysql_bind+2);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",[\"" + clanuuid + "\"]]";
}

std::string mysql_db_handler::updateClan(std::string& extFunction, ext_arguments& extArgument) {
	std::string clanuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_CLAN_UUID);
	this->failIfClanNotExists(clanuuid);

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_VARIABLES)) {
		std::string clanvariables = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_VARIABLES);
		mysql_binds mysql_bind = mysql_binds(2);
		mysql_bind.addData(clanvariables);
		mysql_bind.addData(clanuuid);

		// change variables
		std::string query = "UPDATE `persistent_variables` SET `persistentvariables` = ? WHERE uuid = ("
							"SELECT `persistent_variables_uuid` FROM `clan_has_persistent_variables` WHERE `clan_has_persistent_variables`.`clan_uuid` = UNHEX(?)"
							");";

		this->preparedStatementQuery(query, mysql_bind.mysql_bind);
	}

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID)) {
		std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
		if (!this->playerMemberOfClan(clanuuid, playeruuid)) {
			throw std::runtime_error("new owner " + playeruuid + " needs to be member of the clan " + clanuuid);
		}

		mysql_binds mysql_bind = mysql_binds(2);
		mysql_bind.addData(playeruuid);
		mysql_bind.addData(clanuuid);

		// change owner
		std::string query ="UPDATE `clan` SET `founder_uuid` = UNHEX(?) WHERE `clan`.`uuid` = UNHEX(?);";

		this->preparedStatementQuery(query, mysql_bind.mysql_bind);
	}

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_CLANNAME)) {
		std::string clanname = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_CLANNAME);
		// prevent name duplication case insensitive
		this->failIfClanNameNotUnique(clanname);

		mysql_binds mysql_bind = mysql_binds(2);
		mysql_bind.addData(clanname);
		mysql_bind.addData(clanuuid);

		// change name
		std::string query = "UPDATE `clan` SET `name` = ? "
							"WHERE `clan`.`uuid` = UNHEX(?);";

		this->preparedStatementQuery(query, mysql_bind.mysql_bind);
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",[\"" + clanuuid + "\"]]";
}

std::string mysql_db_handler::addClanPlayer(std::string& extFunction, ext_arguments& extArgument) {
	std::string clanuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_CLAN_UUID);
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	int rank = 0;
	std::string comment = "";

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_RANK)) {
		rank = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_RANK);
	}

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_COMMENT)) {
		comment = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_COMMENT);
	}

	mysql_binds mysql_bind = mysql_binds(4);

	mysql_bind.addData(clanuuid);
	mysql_bind.addData(playeruuid);
	mysql_bind.addData(rank);
	mysql_bind.addData(comment);

	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;

	this->failIfClanNotExists(clanuuid);

	// only add player to clan, if it was not added before
	if (!this->playerMemberOfClan(clanuuid, playeruuid)) {
		// determine if player has a main clan, if not we will set it
		std::string queryplayerinfo =
				str(boost::format{"SELECT HEX(`player`.`mainclan_uuid`) "
								  "FROM `player` "
								  "WHERE `player`.`uuid` = CAST(0x%s AS BINARY)"} % playeruuid);

		this->rawquery(queryplayerinfo, &result);

		rowcount = mysql_num_rows(result);

		if (rowcount < 1) {
			throw std::runtime_error("could not find player with uuid: " + playeruuid);
		}
		row = mysql_fetch_row(result);


		// add player only if he actually exists
		std::string query = "INSERT INTO `clan_member` (`clan_uuid`, `player_uuid`, `rank`, `comment`) VALUES (UNHEX(?), UNHEX(?), ?, ?)";

		this->preparedStatementQuery(query, mysql_bind.mysql_bind);

		// update players main clan if it was unset
		if (row[0] == NULL) {
			queryplayerinfo =
					str(boost::format{"UPDATE `player` SET `mainclan_uuid` = CAST(0x%s AS BINARY) "
									  "WHERE `player`.`uuid` = CAST(0x%s AS BINARY)"} % clanuuid % playeruuid);

			this->rawquery(queryplayerinfo);
		}

		mysql_free_result(result);
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",[\"" + clanuuid + "\",\"" + playeruuid + "\"]]";
}

std::string mysql_db_handler::updateClanPlayer(std::string& extFunction, ext_arguments& extArgument) {
	std::string clanuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_CLAN_UUID);
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_RANK)) {
		int rank = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_RANK);

		mysql_binds mysql_bind = mysql_binds(3);

		mysql_bind.addData(rank);
		mysql_bind.addData(clanuuid);
		mysql_bind.addData(playeruuid);

		std::string query ="UPDATE `clan_member` SET `rank` = ? WHERE `clan_member`.`clan_uuid` = UNHEX(?) AND `clan_member`.`player_uuid` = UNHEX(?)";

		this->preparedStatementQuery(query, mysql_bind.mysql_bind);
	}

	if (extArgument.keyExists(PROTOCOL_DBCALL_ARGUMENT_COMMENT)) {
		std::string comment = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_COMMENT);

		mysql_binds mysql_bind = mysql_binds(3);

		mysql_bind.addData(comment);
		mysql_bind.addData(clanuuid);
		mysql_bind.addData(playeruuid);

		std::string query ="UPDATE `clan_member` SET `comment` = ? WHERE `clan_member`.`clan_uuid` = UNHEX(?) AND `clan_member`.`player_uuid` = UNHEX(?)";

		this->preparedStatementQuery(query, mysql_bind.mysql_bind);
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",[\"" + clanuuid + "\",\"" + playeruuid + "\"]]";
}

std::string mysql_db_handler::deleteClanPlayer(std::string& extFunction, ext_arguments& extArgument) {
	std::string clanuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_CLAN_UUID);
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	int rank = 0;

	MYSQL_RES* result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;

	this->failIfClanNotExists(clanuuid);

	// only delete player from clan, if he was added before
	if (this->playerMemberOfClan(clanuuid, playeruuid)) {
		std::string query = str(boost::format{"DELETE FROM `clan_member` "
											  "WHERE `clan_member`.`clan_uuid` = CAST(0x%s AS BINARY) "
											  "AND `clan_member`.`player_uuid` = CAST(0x%s AS BINARY)"} % clanuuid % playeruuid);

		this->rawquery(query);

		// determine if player has a main clan, if not we will set it
		std::string queryplayerinfo =
				str(boost::format{"SELECT HEX(`player`.`mainclan_uuid`) "
								  "FROM `player` "
								  "WHERE `player`.`uuid` = CAST(0x%s AS BINARY)"} % playeruuid);

		this->rawquery(queryplayerinfo, &result);

		rowcount = mysql_num_rows(result);

		if (rowcount < 1) {
			throw std::runtime_error("could not find player with uuid: " + playeruuid);
		}
		row = mysql_fetch_row(result);

		// if the main clan of player is the clan he was deleted from, unset the main clan link
		if (row[0] != NULL) {
			if (clanuuid == row[0]) {
				queryplayerinfo =
						str(boost::format{"UPDATE `player` SET `mainclan_uuid` = NULL "
										  "WHERE `player`.`uuid` = CAST(0x%s AS BINARY) "} % playeruuid);

				this->rawquery(queryplayerinfo);
			}
		}

		mysql_free_result(result);
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",[\"" + clanuuid + "\",\"" + playeruuid + "\"]]";
}

std::string mysql_db_handler::updatePlayerMainClan(std::string& extFunction, ext_arguments& extArgument) {
	std::string clanuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_CLAN_UUID);
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);

	if (!clanuuid.empty()) {
		this->failIfClanNotExists(clanuuid);

		// only add player to clan, if it was not added before
		if (!this->playerMemberOfClan(clanuuid, playeruuid)) {
			throw std::runtime_error("player " + playeruuid + " needs to be member of clan " + clanuuid + " to use it as main clan");
		};

		std::string queryplayerinfo =
				str(boost::format{"UPDATE `player` SET `mainclan_uuid` = CAST(0x%s AS BINARY) "
					  "WHERE `player`.`uuid` = CAST(0x%s AS BINARY)"} % clanuuid % playeruuid);

		this->rawquery(queryplayerinfo);
	} else {
		std::string queryplayerinfo =
				str(boost::format{"UPDATE `player` SET `mainclan_uuid` = NULL "
					  "WHERE `player`.`uuid` = CAST(0x%s AS BINARY)"} % playeruuid);

		this->rawquery(queryplayerinfo);
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",[\"" + clanuuid + "\",\"" + playeruuid + "\"]]";
}

void mysql_db_handler::test_stmt_error(MYSQL_STMT* stmt, int status) {
	if (status) {
		throw std::runtime_error(std::string("MYSQLERROR") + std::string(mysql_stmt_error(stmt)) + std::string(" Errornumber: ") + std::to_string(mysql_stmt_errno(stmt)));
	}
}
