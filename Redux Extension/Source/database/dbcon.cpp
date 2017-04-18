/* dbcon.cpp
 *
 * Copyright 2016-2017 Desolation Redux
 *
 * Author: Legodev <legodevgit@mailbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
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
					boost::bind(&dbcon::spawnHandler, this, _1, _2)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_EXECUTE_TERMINATE_DB),
					boost::bind(&dbcon::terminateHandler, this, _1, _2)));

	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_RETURN_UUID),
					boost::bind(&dbcon::getUUID, this, _1, _2)));

	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_RETURN_ECHO_STRING),
					boost::bind(&dbcon::echo, this, _1, _2)));

	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_RETURN_ASYNC_MSG),
					boost::bind(&dbcon::rcvasmsg, this, _1, _2)));

	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_RETURN_ASYNC_SATE),
					boost::bind(&dbcon::chkasmsg, this, _1, _2)));

	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_RETURN_DB_VERSION),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_RETURN_DB_VERSION),
					std::make_tuple(
							boost::bind(&dbcon::dbVersion, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_DEBUG_CALL),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_PLAYER),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_PLAYER),
					std::make_tuple(
							boost::bind(&dbcon::loadPlayer, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_AV_CHARS),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_AV_CHARS),
					std::make_tuple(
							boost::bind(&dbcon::loadAvChars, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_LINK_CHARS),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_LINK_CHARS),
					std::make_tuple(
							boost::bind(&dbcon::linkChars, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_CHAR),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_CHAR),
					std::make_tuple(boost::bind(&dbcon::loadChar, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_CREATE_CHAR),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_CREATE_CHAR),
					std::make_tuple(
							boost::bind(&dbcon::createChar, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_UPDATE_CHAR),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_UPDATE_CHAR),
					std::make_tuple(
							boost::bind(&dbcon::updateChar, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_LOCATIONUPDATE_CHAR),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_LOCATIONUPDATE_CHAR),
					std::make_tuple(
							boost::bind(&dbcon::locupdateChar, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_DECLARE_CHAR_DEATH),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_DECLARE_CHAR_DEATH),
					std::make_tuple(boost::bind(&dbcon::killChar, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_OBJECT),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_LOAD_OBJECT),
					std::make_tuple(
							boost::bind(&dbcon::loadObject, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_CREATE_OBJECT),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_CREATE_OBJECT),
					std::make_tuple(
							boost::bind(&dbcon::createObject, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_QUIET_CREATE_OBJECT),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_QUIET_CREATE_OBJECT),
					std::make_tuple(
							boost::bind(&dbcon::qcreateObject, this, _1, _2),
							QUIET_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_UPDATE_OBJECT),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_UPDATE_OBJECT),
					std::make_tuple(
							boost::bind(&dbcon::updateObject, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_DECLARE_OBJECT_DEATH),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_DECLARE_OBJECT_DEATH),
					std::make_tuple(
							boost::bind(&dbcon::killObject, this, _1, _2),
							ASYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_DUMP_OBJECTS),
					boost::bind(&dbcon::processDBCall, this, _1, _2)));
	dbfunctions.insert(
			std::make_pair(std::string(PROTOCOL_DBCALL_FUNCTION_DUMP_OBJECTS),
					std::make_tuple(
							boost::bind(&dbcon::dumpObjects, this, _1, _2),
							ASYNC_MAGIC)));

	// spawn some idle work
	DBioServiceWork.reset( new boost::asio::io_service::work(DBioService) );

	return;
}

dbcon::~dbcon() {
	intptr_t syncdbhandlerpointer;
	base_db_handler *syncdbhandler;

	DBioServiceWork.reset(); // stop all idle work!
	DBioService.stop(); // terminate all work
	// commented because of a bug with boost thread pool if join_all gets executed in process detach
	//asyncthreadpool.join_all(); // get rid of all threads

	// while there is an handler call disconnect
	while (syncdbhandlerpool.pop(syncdbhandlerpointer)) {
		syncdbhandler = (base_db_handler*) syncdbhandlerpointer;
		syncdbhandler->disconnect();
	}

	poolinitialized = false;
	return;
}

std::string dbcon::spawnHandler(std::string &extFunction, ext_arguments &extArgument) {
	unsigned int poolsize = extArgument.get<unsigned int>("poolsize");
	std::string worlduuid = extArgument.get<std::string>("worlduuid");

	if (poolsize < 1) {
		poolsize = 1;
	}

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

		base_db_handler *syncdbhandler;
		boost::property_tree::ptree configtree;
		boost::property_tree::json_parser::read_json(CONFIG_FILE_NAME, configtree);

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

		if (!syncdbhandlerpool.is_lock_free()) {
			throw std::runtime_error("syncdbhandlerpool is not lock free");
		}

		/* for the beginning we want to have some spare pool handlers */
		syncdbhandlerpool.reserve(poolsize + 3);

		for (i = 0; i < poolsize+2; i++) {
			if (boost::iequals(type, "MYSQL")) {
				std::cout << "creating mysql_db_handler" << std::endl;
				syncdbhandler = new (mysql_db_handler);
			} else {
				syncdbhandler = new (base_db_handler);
			}

			syncdbhandler->connect(hostname, user, password, database, port, whitelistonly, allowsteamapi, vaccheckban,
					vacmaxcount, vacignoredays, worlduuid);
			syncdbhandlerpool.bounded_push((intptr_t) syncdbhandler);
		}

		// tempsyncdbhandler.connect(hostname, user, password, database, port, worlduuid);

		for (i = 0; i < poolsize; i++) {
			asyncthreadpool.create_thread(
					boost::bind(static_cast<std::size_t (boost::asio::io_service::*)()>(&boost::asio::io_service::run), &DBioService)
			);
		}

		poolinitialized = true;
	} else {
		throw std::runtime_error("Threads already spawned");
	}

	return std::to_string(asyncthreadpool.size());
}

std::string dbcon::terminateHandler(std::string &extFunction, ext_arguments &extArgument) {
	this->terminateHandler();
	return "DONE";
}

void dbcon::terminateHandler() {
	intptr_t syncdbhandlerpointer;
	base_db_handler *syncdbhandler;

	// prevent new requests
	poolcleanup = true;

	DBioServiceWork.reset(); // stop all idle work!

	// wait until all jobs are finished
	while (!DBioService.stopped()) {
		boost::thread::yield();
	}

	// should not  ne needed
	DBioService.stop();

	// get rid of all threads
	asyncthreadpool.join_all();

	// while there is an handler call disconnect
	while (syncdbhandlerpool.pop(syncdbhandlerpointer)) {
		syncdbhandler = (base_db_handler*) syncdbhandlerpointer;
		syncdbhandler->disconnect();
	}

	poolcleanup = false;
	poolinitialized = false;
	return;
}

std::string dbcon::processDBCall(std::string &extFunction, ext_arguments &extArgument) {
	std::string returnString;

	DB_FUNCTIONS::iterator it = dbfunctions.find(extFunction);
	if (it != dbfunctions.end()) {
		DB_FUNCTION_INFO funcinfo = it->second;

		if (!poolinitialized) {
			throw std::runtime_error("db handler pool not initialized");
		}

		if (poolcleanup) {
			throw std::runtime_error("db handler pool is about to be terminated, will not add an new request");
		}

		switch (std::get<1>(funcinfo)) {
		case SYNC_MAGIC:
			returnString = syncCall(funcinfo, extArgument);
			break;

		case ASYNC_MAGIC:
			returnString = asyncCall(funcinfo, extArgument);
			break;

		case QUIET_MAGIC:
			returnString = quietCall(funcinfo, extArgument);
			break;

		default:
			throw std::runtime_error("unknown function class");
		}
	} else {
		throw std::runtime_error("Don't know extFunction: " + extFunction);
	}

	return returnString;
}

std::string dbcon::syncCall(DB_FUNCTION_INFO funcinfo, ext_arguments &extArgument) {
	const DB_FUNCTION &func(std::get<0>(funcinfo));
	std::string returnString;

	intptr_t syncdbhandlerpointer;
	base_db_handler *syncdbhandler;

	// try to get an db handler
	while (!syncdbhandlerpool.pop(syncdbhandlerpointer)) {
		printf("i am in a loop! :( \n");
	}
	try {
		syncdbhandler = (base_db_handler*) syncdbhandlerpointer;

		returnString = func(extArgument, syncdbhandler);

		// return handler to the pool
		syncdbhandlerpool.bounded_push(syncdbhandlerpointer);
	} catch (std::exception const& e) {
		// always return the handler
		syncdbhandlerpool.bounded_push(syncdbhandlerpointer);
		throw std::runtime_error(e.what());
	}

	return returnString;
}

std::string dbcon::asyncCall(DB_FUNCTION_INFO funcinfo, ext_arguments &extArgument) {
	ext_arguments copyextArgument = extArgument;
	PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier = PROTOCOL_IDENTIFIER_GENERATOR;

	//boost::bind(&dbcon::asyncCallProcessor, this, funcinfo, extArgument, messageIdentifier);
	DBioService.post(boost::bind(&dbcon::asyncCallProcessor, this, funcinfo, extArgument, messageIdentifier));

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_ASYNC) + "\",\"" + messageIdentifier + "\"]" ;
}

std::string dbcon::quietCall(DB_FUNCTION_INFO funcinfo, ext_arguments &extArgument) {
	ext_arguments copyextArgument = extArgument;
	PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier = "";

	DBioService.post(boost::bind(&dbcon::asyncCallProcessor, this, funcinfo, extArgument, messageIdentifier));

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_QUIET) + "\",\"\"]" ;
}

void dbcon::asyncCallProcessor(DB_FUNCTION_INFO funcinfo, ext_arguments extArgument, PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier) {
	const DB_FUNCTION &func(std::get<0>(funcinfo));

	std::string returnString;

	try {
		returnString = this->syncCall(funcinfo, extArgument);
		//func(extArgument, syncdbhandler);
	} catch (std::exception const& e) {
		std::string error = e.what();
		int i = 0;
		while ((i = error.find("\"", i)) != std::string::npos) {
			error.insert(i, "\"");
			i += 2;
		}
		returnString = "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_ERROR) + "\",\"";
		returnString += error;
		returnString += "\"]";
	}

	if (messageIdentifier != "") {
		msgmutex.lock();
		msgmap.insert(std::make_pair(messageIdentifier, returnString));
		msgmutex.unlock();
	}

	return;
}

std::string dbcon::getUUID(std::string &extFunction, ext_arguments &extArgument) {
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + orderedUUID() + "\"]";
}

std::string dbcon::echo(std::string &extFunction, ext_arguments &extArgument) {
	std::string echostring = extArgument.get<std::string>("echostring");
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + echostring + "\"]";
}

std::string dbcon::chkasmsg(std::string &extFunction, ext_arguments &extArgument) {
	PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier = extArgument.get<PROTOCOL_IDENTIFIER_DATATYPE>(PROTOCOL_IDENTIFIER_NAME);
	std::string returnString;

	msgmutex.lock();
	SINGLE_MESSAGE_MAP::iterator it = msgmap.find(messageIdentifier);
	msgmutex.unlock();

	// check if message object was found
	if (it == msgmap.end()) {
		returnString = PROTOCOL_MESSAGE_NOT_EXISTING;
	} else {
		returnString = PROTOCOL_MESSAGE_EXISTING;
	}

	return returnString;
}

std::string dbcon::rcvasmsg(std::string &extFunction, ext_arguments &extArgument) {
	PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier = extArgument.get<PROTOCOL_IDENTIFIER_DATATYPE>(PROTOCOL_IDENTIFIER_NAME);
	std::string returnString;

	msgmutex.lock();
	SINGLE_MESSAGE_MAP::iterator it = msgmap.find(messageIdentifier);
	msgmutex.unlock();

	// check if message object was found
	if (it == msgmap.end()) {
		returnString = PROTOCOL_MESSAGE_NOT_EXISTING;
	} else {
		// extract message object
		returnString = it->second;
		msgmutex.lock();
		msgmap.erase (it);
		msgmutex.unlock();
	}

	return returnString;
}

std::string dbcon::dbVersion(ext_arguments &extArgument, base_db_handler *dbhandler) {
	std::string version;

	version = dbhandler->querydbversion();

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + version + "\"]";
}



std::string dbcon::loadPlayer(ext_arguments &extArgument, base_db_handler *dbhandler) {
	std::string nickname = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_NICKNAME);
	std::string steamid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_STEAMID);
	std::string playerinfo = dbhandler->loadPlayer(nickname, steamid);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + playerinfo + "]";
}

std::string dbcon::loadAvChars(ext_arguments &extArgument, base_db_handler *dbhandler) {
	std::string playeruuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);

	std::string result = dbhandler->loadAvChars(playeruuid);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + result + "]";
}

std::string dbcon::linkChars(ext_arguments &extArgument, base_db_handler *dbhandler) {
	std::string playeruuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string variabuuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_VARIABUUID);

	std::string result = dbhandler->linkChars(playeruuid, variabuuid);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + result + "]";
}

std::string dbcon::loadChar(ext_arguments &extArgument, base_db_handler *dbhandler) {
	std::string playeruuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);

	std::string result = dbhandler->loadChar(playeruuid);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + result + "]";
}

std::string dbcon::createChar(ext_arguments &extArgument, base_db_handler *dbhandler) {
	std::string playeruuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string animationstate = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_ANIMATIONSTATE);
	float direction = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DIRECTION);
	int positiontype = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_POSITIONTYPE);
	float positionx = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONX);
	float positiony = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONY);
	float positionz = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONZ);
	std::string classname = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_CLASSNAME);
	std::string hitpoints = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_HITPOINTS);
	std::string variables = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_VARIABLES);
	std::string persistentvariables = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_PERSISTENTVARIABLES);
	std::string textures = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_TEXTURES);
	std::string inventoryuniform = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_INVENTORYUNIFORM);
	std::string inventoryvest = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_INVENTORYVEST);
	std::string inventorybackpack = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_INVENTORYBACKPACK);
	std::string uniform = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_UNIFORM);
	std::string vest = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_VEST);
	std::string backpack = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_BACKPACK);
	std::string headgear = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_HEADGEAR);
	std::string googles = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_GOOGLES);
	std::string primaryweapon = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_PRIMARYWEAPON);
	std::string secondaryweapon = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_SECONDARYWEAPON);
	std::string handgun = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_HANDGUN);
	std::string tools = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_TOOLS);
	std::string currentweapon = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_CURRENTWEAPON);

	std::string result = dbhandler->createChar(playeruuid, animationstate, direction, positiontype, positionx,
			positiony, positionz, classname, hitpoints, variables, persistentvariables, textures, inventoryuniform,
			inventoryvest, inventorybackpack, uniform, vest, backpack, headgear, googles, primaryweapon,
			secondaryweapon, handgun, tools, currentweapon);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string dbcon::updateChar(ext_arguments &extArgument, base_db_handler *dbhandler) {
	std::string charuuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_CHARUUID);
	std::string animationstate = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_ANIMATIONSTATE);
	float direction = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DIRECTION);
	int positiontype = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_POSITIONTYPE);
	float positionx = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONX);
	float positiony = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONY);
	float positionz = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONZ);
	std::string classname = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_CLASSNAME);
	std::string hitpoints = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_HITPOINTS);
	std::string variables = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_VARIABLES);
	std::string persistentvariables = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_PERSISTENTVARIABLES);
	std::string textures = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_TEXTURES);
	std::string inventoryuniform = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_INVENTORYUNIFORM);
	std::string inventoryvest = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_INVENTORYVEST);
	std::string inventorybackpack = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_INVENTORYBACKPACK);
	std::string uniform = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_UNIFORM);
	std::string vest = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_VEST);
	std::string backpack = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_BACKPACK);
	std::string headgear = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_HEADGEAR);
	std::string googles = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_GOOGLES);
	std::string primaryweapon = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_PRIMARYWEAPON);
	std::string secondaryweapon = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_SECONDARYWEAPON);
	std::string handgun = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_HANDGUN);
	std::string tools = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_TOOLS);
	std::string currentweapon = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_CURRENTWEAPON);

	std::string result = dbhandler->updateChar(charuuid, animationstate, direction, positiontype, positionx,
				positiony, positionz, classname, hitpoints, variables, persistentvariables, textures, inventoryuniform,
				inventoryvest, inventorybackpack, uniform, vest, backpack, headgear, googles, primaryweapon,
				secondaryweapon, handgun, tools, currentweapon);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string dbcon::locupdateChar(ext_arguments &extArgument, base_db_handler *dbhandler) {
	std::string charuuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_CHARUUID);
	std::string animationstate = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_ANIMATIONSTATE);
	float direction = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DIRECTION);
	int positiontype = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_POSITIONTYPE);
	float positionx = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONX);
	float positiony = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONY);
	float positionz = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONZ);

	std::string result = dbhandler->locupdateChar(charuuid, animationstate, direction, positiontype, positionx,
					positiony, positionz);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string dbcon::killChar(ext_arguments &extArgument, base_db_handler *dbhandler) {
	std::string charuuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_CHARUUID);
	std::string attackeruuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_ATTACKER);
	std::string type = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_TYPE);
	std::string weapon = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_WEAPON);
	float distance = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DISTANCE);

	std::string result = dbhandler->killChar(charuuid, attackeruuid, type, weapon, distance);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string dbcon::loadObject(ext_arguments &extArgument, base_db_handler *dbhandler) {
	std::string objectuuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);

	std::string result = dbhandler->loadObject(objectuuid);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + result + "]";
}

std::string dbcon::createObject(ext_arguments &extArgument, base_db_handler *dbhandler) {
	std::string classname = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_CLASSNAME);
	int priority = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_PRIORITY);
	int visible = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_VISIBLE);
	std::string accesscode = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_ACCESSCODE);
	int locked = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_LOCKED);
	std::string player_uuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string hitpoints = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_HITPOINTS);
	float damage = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DAMAGE);
	float fuel = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_FUEL);
	float fuelcargo = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_FUELCARGO);
	float repaircargo = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_REPAIRCARGO);
	std::string items = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_ITEMS);
	std::string magazinesturret = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_MAGAZINESTURRET);
	std::string variables = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_VARIABLES);
	std::string animationstate = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_ANIMATIONSTATE);
	std::string textures = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_TEXTURES);
	float direction = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DIRECTION);
	int positiontype = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_POSITIONTYPE);
	float positionx = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONX);
	float positiony = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONY);
	float positionz = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONZ);
	std::string positionadvanced = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_POSITIONADVANCED);
	std::string reservedone = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_RESERVEDONE);
	std::string reservedtwo = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_RESERVEDTWO);

	std::string result = dbhandler->createObject(classname, priority, visible,
			accesscode, locked, player_uuid, hitpoints, damage, fuel, fuelcargo,
			repaircargo, items, magazinesturret, variables, animationstate,
			textures, direction, positiontype, positionx, positiony, positionz,
			positionadvanced, reservedone, reservedtwo);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string dbcon::qcreateObject(ext_arguments &extArgument, base_db_handler *dbhandler) {
	std::string objectuuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	std::string classname = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_CLASSNAME);
	int priority = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_PRIORITY);
	int visible = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_VISIBLE);
	std::string accesscode = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_ACCESSCODE);
	int locked = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_LOCKED);
	std::string player_uuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string hitpoints = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_HITPOINTS);
	float damage = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DAMAGE);
	float fuel = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_FUEL);
	float fuelcargo = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_FUELCARGO);
	float repaircargo = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_REPAIRCARGO);
	std::string items = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_ITEMS);
	std::string magazinesturret = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_MAGAZINESTURRET);
	std::string variables = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_VARIABLES);
	std::string animationstate = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_ANIMATIONSTATE);
	std::string textures = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_TEXTURES);
	float direction = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DIRECTION);
	int positiontype = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_POSITIONTYPE);
	float positionx = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONX);
	float positiony = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONY);
	float positionz = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONZ);
	std::string positionadvanced = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_POSITIONADVANCED);
	std::string reservedone = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_RESERVEDONE);
	std::string reservedtwo = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_RESERVEDTWO);

	std::string result = dbhandler->createObject(objectuuid, classname,
			priority, visible, accesscode, locked, player_uuid, hitpoints,
			damage, fuel, fuelcargo, repaircargo, items, magazinesturret,
			variables, animationstate, textures, direction, positiontype,
			positionx, positiony, positionz, positionadvanced, reservedone,
			reservedtwo);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string dbcon::updateObject(ext_arguments &extArgument, base_db_handler *dbhandler) {
	std::string objectuuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	std::string classname = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_CLASSNAME);
	int priority = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_PRIORITY);
	int visible = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_VISIBLE);
	std::string accesscode = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_ACCESSCODE);
	int locked = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_LOCKED);
	std::string player_uuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);
	std::string hitpoints = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_HITPOINTS);
	float damage = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DAMAGE);
	float fuel = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_FUEL);
	float fuelcargo = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_FUELCARGO);
	float repaircargo = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_REPAIRCARGO);
	std::string items = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_ITEMS);
	std::string magazinesturret = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_MAGAZINESTURRET);
	std::string variables = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_VARIABLES);
	std::string animationstate = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_ANIMATIONSTATE);
	std::string textures = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_TEXTURES);
	float direction = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DIRECTION);
	int positiontype = extArgument.get<int>(PROTOCOL_DBCALL_ARGUMENT_POSITIONTYPE);
	float positionx = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONX);
	float positiony = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONY);
	float positionz = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_POSITIONZ);
	std::string positionadvanced = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_POSITIONADVANCED);
	std::string reservedone = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_RESERVEDONE);
	std::string reservedtwo = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_RESERVEDTWO);

	std::string result = dbhandler->updateObject(objectuuid, classname,
			priority, visible, accesscode, locked, player_uuid, hitpoints,
			damage, fuel, fuelcargo, repaircargo, items, magazinesturret,
			variables, animationstate, textures, direction, positiontype,
			positionx, positiony, positionz, positionadvanced, reservedone,
			reservedtwo);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string dbcon::killObject(ext_arguments &extArgument, base_db_handler *dbhandler) {
	std::string charuuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	std::string attackeruuid = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_ATTACKER);
	std::string type = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_TYPE);
	std::string weapon = extArgument.get<std::string>(PROTOCOL_DBCALL_ARGUMENT_WEAPON);
	float distance = extArgument.get<float>(PROTOCOL_DBCALL_ARGUMENT_DISTANCE);

	std::string result = dbhandler->killObject(charuuid, attackeruuid, type, weapon, distance);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + result + "\"]";
}

std::string dbcon::dumpObjects(ext_arguments &extArgument, base_db_handler *dbhandler) {
        std::string matrix;
        bool placecommaone = false;
        bool placecommatwo = false;

        std::vector< std::vector<std::string> > resultmatrix = dbhandler->dumpObjects();
        matrix = "[";
        for (auto& row : resultmatrix) {
                if (placecommaone) {
                        matrix += ",";
                }
                matrix += "[";
                placecommatwo = false;
                for (auto& value : row) {
                        if (placecommatwo) {
                                matrix += ",";
                        }
                        //matrix += "\"" + value + "\"";
                        matrix += value;
                        placecommatwo = true;
                }
                matrix += "]";
                placecommaone = true;
        }
        matrix += "]";

        return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + matrix + "]";
}
