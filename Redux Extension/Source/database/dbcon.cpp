/* dbcon.cpp
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

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <exception>
#include <stdexcept>

#include "database/dbcon.hpp"
#include "utils/uuid.hpp"

dbcon::dbcon(EXT_FUNCTIONS &extFunctions) {
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_EXECUTE_INIT_DB),
					std::make_tuple(
							boost::bind(&dbcon::spawnHandler, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_EXECUTE_TERMINATE_DB),
					std::make_tuple(
							boost::bind(&dbcon::terminateHandler, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_RETURN_UUID),
					std::make_tuple(
							boost::bind(&dbcon::getUUID, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_RETURN_ECHO_STRING),
					std::make_tuple(
							boost::bind(&dbcon::echo, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_RETURN_DB_VERSION),
					std::make_tuple(
							boost::bind(&dbcon::dbVersion, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_PLAYER),
					std::make_tuple(
							boost::bind(&dbcon::loadPlayer, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_AV_CHARS),
					std::make_tuple(
							boost::bind(&dbcon::loadAvChars, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_LINK_CHARS),
					std::make_tuple(
							boost::bind(&dbcon::linkChars, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_CHAR),
					std::make_tuple(
							boost::bind(&dbcon::loadChar, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_CREATE_CHAR),
					std::make_tuple(
							boost::bind(&dbcon::createChar, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_UPDATE_CHAR),
					std::make_tuple(
							boost::bind(&dbcon::updateChar, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_DECLARE_CHAR_DEATH),
					std::make_tuple(
							boost::bind(&dbcon::killChar, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_OBJECT),
					std::make_tuple(
							boost::bind(&dbcon::loadObject, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_CREATE_OBJECT),
					std::make_tuple(
							boost::bind(&dbcon::createObject, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_QUIET_CREATE_OBJECT),
					std::make_tuple(
							boost::bind(&dbcon::qcreateObject, this, _1, _2),
							QUIET_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_UPDATE_OBJECT),
					std::make_tuple(
							boost::bind(&dbcon::updateObject, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_DECLARE_OBJECT_DEATH),
					std::make_tuple(
							boost::bind(&dbcon::killObject, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_DUMP_OBJECTS),
					std::make_tuple(
							boost::bind(&dbcon::dumpObjects, this, _1, _2),
							ASYNC_MAGIC)));
	return;
}

dbcon::~dbcon() {
	intptr_t dbhandlerpointer;
	base_db_handler *dbhandler;

	// while there is an handler call disconnect
	while (dbhandlerpool.pop(dbhandlerpointer)) {
		dbhandler = (base_db_handler*) dbhandlerpointer;
		dbhandler->disconnect();
	}

	poolinitialized = false;
	return;
}

std::string dbcon::spawnHandler(std::string &extFunction, ext_arguments &extArgument) {
	std::string worlduuid = extArgument.getUUID("worlduuid");

	if (!poolinitialized) {
		int i;
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

		base_db_handler *dbhandler;
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

		if (!dbhandlerpool.is_lock_free()) {
			throw std::runtime_error("dbhandlerpool is not lock free");
		}

		/* for the beginning we want to have some spare pool handlers */
		dbhandlerpool.reserve(poolsize + 3);

		for (i = 0; i < poolsize+1; i++) {
			if (boost::iequals(type, "MYSQL")) {
				std::cout << "creating mysql_db_handler" << std::endl;
				dbhandler = new (mysql_db_handler);
			} else {
				dbhandler = new (base_db_handler);
			}

			dbhandler->connect(hostname, user, password, database, port, whitelistonly, allowsteamapi, vaccheckban,
					vacmaxcount, vacignoredays, worlduuid);

			if (i == 0) {
				dbhandler->checkWorldUUID();
			}

			dbhandlerpool.bounded_push((intptr_t) dbhandler);
		}

		poolinitialized = true;
	} else {
		throw std::runtime_error("Threads already spawned");
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",1337]";
}

std::string dbcon::terminateHandler(std::string &extFunction, ext_arguments &extArgument) {
	this->terminateHandler();
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"DONE\"]";
}

void dbcon::terminateHandler() {
	intptr_t dbhandlerpointer;
	base_db_handler *dbhandler;

	// prevent new requests
	poolcleanup = true;

	// while there is an handler call disconnect
	while (dbhandlerpool.pop(dbhandlerpointer)) {
		dbhandler = (base_db_handler*) dbhandlerpointer;
		dbhandler->disconnect();
	}

	poolcleanup = false;
	poolinitialized = false;
	return;
}

base_db_handler * dbcon::getDBHandler() {
	intptr_t dbhandlerpointer;
	base_db_handler *dbhandler = 0;

	// try to get an db handler
	while (!dbhandlerpool.pop(dbhandlerpointer));

	try {
		dbhandler = (base_db_handler*) dbhandlerpointer;
	} catch (std::exception const& e) {
		// always return the handler
		dbhandlerpool.bounded_push(dbhandlerpointer);
		dbhandler = 0;
		throw std::runtime_error(e.what());
	}

	return dbhandler;
}

void dbcon::returnDBHandler(base_db_handler * dbhandler) {
	intptr_t dbhandlerpointer = (intptr_t) dbhandler;
	dbhandlerpool.bounded_push(dbhandlerpointer);

	return;
}

std::string dbcon::getUUID(std::string &extFunction, ext_arguments &extArgument) {
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + orderedUUID() + "\"]";
}

std::string dbcon::echo(std::string &extFunction, ext_arguments &extArgument) {
	std::string echostring = extArgument.get<std::string>("echostring");
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + echostring + "\"]";
}

std::string dbcon::dbVersion(std::string &extFunction, ext_arguments &extArgument) {
	base_db_handler *dbhandler = getDBHandler();
	std::string version;

	version = dbhandler->querydbversion();

	returnDBHandler(dbhandler);
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + version + "\"]";
}


std::string dbcon::loadPlayer(std::string &extFunction, ext_arguments &extArgument) {
	base_db_handler *dbhandler = getDBHandler();
	std::string nickname = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_NICKNAME);
	std::string steamid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_STEAMID);
	std::string playerinfo = dbhandler->loadPlayer(nickname, steamid);

	returnDBHandler(dbhandler);
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + playerinfo + "]";
}

std::string dbcon::loadAvChars(std::string &extFunction, ext_arguments &extArgument) {
	base_db_handler *dbhandler = getDBHandler();
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);

	std::string result = dbhandler->loadAvChars(playeruuid);

	returnDBHandler(dbhandler);
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + result + "]";
}

std::string dbcon::linkChars(std::string &extFunction, ext_arguments &extArgument) {
	base_db_handler *dbhandler = getDBHandler();
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string variabuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_VARIABUUID);

	std::string result = dbhandler->linkChars(playeruuid, variabuuid);

	returnDBHandler(dbhandler);
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + result + "]";
}

std::string dbcon::loadChar(std::string &extFunction, ext_arguments &extArgument) {
	base_db_handler *dbhandler = getDBHandler();
	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);

	std::string characterdata = dbhandler->loadChar(charactercache, playeruuid);

	returnDBHandler(dbhandler);
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + characterdata + "]";
}

std::string dbcon::createChar(std::string &extFunction, ext_arguments &extArgument) {
	base_db_handler *dbhandler = getDBHandler();
	std::string characterdata = dbhandler->createChar(charactercache, extArgument);

	returnDBHandler(dbhandler);
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + characterdata + "\"]";
}

std::string dbcon::updateChar(std::string &extFunction, ext_arguments &extArgument) {
	base_db_handler *dbhandler = getDBHandler();
	std::string result = dbhandler->updateChar(charactercache, extArgument);

	returnDBHandler(dbhandler);
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string dbcon::killChar(std::string &extFunction, ext_arguments &extArgument) {
	base_db_handler *dbhandler = getDBHandler();
	std::string charuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_CHARUUID);
	std::string attackeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_ATTACKER);
	std::string type = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_TYPE);
	std::string weapon = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_WEAPON);
	float distance = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DISTANCE);

	std::string result = dbhandler->killChar(charuuid, attackeruuid, type, weapon, distance);

	returnDBHandler(dbhandler);
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string dbcon::loadObject(std::string &extFunction, ext_arguments &extArgument) {
	base_db_handler *dbhandler = getDBHandler();

	std::string result = dbhandler->loadObject(objectcache, extArgument);

	returnDBHandler(dbhandler);
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + result + "]";
}

std::string dbcon::createObject(std::string &extFunction, ext_arguments &extArgument) {
	base_db_handler *dbhandler = getDBHandler();
	std::string result = dbhandler->createObject(objectcache, extArgument);

	returnDBHandler(dbhandler);
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string dbcon::qcreateObject(std::string &extFunction, ext_arguments &extArgument) {
	base_db_handler *dbhandler = getDBHandler();
	std::string result = dbhandler->createObject(objectcache, extArgument);

	returnDBHandler(dbhandler);
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string dbcon::updateObject(std::string &extFunction, ext_arguments &extArgument) {
	base_db_handler *dbhandler = getDBHandler();
	std::string result = dbhandler->updateObject(objectcache, extArgument);

	returnDBHandler(dbhandler);
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string dbcon::killObject(std::string &extFunction, ext_arguments &extArgument) {
	base_db_handler *dbhandler = getDBHandler();
	std::string charuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	std::string attackeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_ATTACKER);
	std::string type = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_TYPE);
	std::string weapon = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_WEAPON);
	float distance = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DISTANCE);

	std::string result = dbhandler->killObject(charuuid, attackeruuid, type, weapon, distance);

	returnDBHandler(dbhandler);
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string dbcon::dumpObjects(std::string &extFunction, ext_arguments &extArgument) {
	base_db_handler *dbhandler = getDBHandler();
	std::string matrix;
	bool placecommaone = false;
	bool placecommatwo = false;

	std::vector<cache_base*> objectList = dbhandler->dumpObjects(objectcache);
	matrix = "[";
	for (cache_base* object : objectList) {
			if (placecommaone) {
					matrix += ",";
			}

			matrix += object->getAsArmaString();

			placecommaone = true;
	}
	matrix += "]";

	returnDBHandler(dbhandler);
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + matrix + "]";
}
