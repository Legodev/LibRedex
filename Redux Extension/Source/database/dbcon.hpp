/* dbcon.hpp
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

#ifndef SOURCE_DBCON_HPP_
#define SOURCE_DBCON_HPP_

#include <string>
#include <map>
#include <tuple>
#include <cstdint>
#include <mutex>
#include <boost/property_tree/ptree.hpp>
#include <boost/function.hpp>
#include <boost/asio.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/tss.hpp>

#include "database/base.hpp"
#include "database/mysql.hpp"
#include "constants.hpp"

#include <stdio.h>

class dbcon {
public:
	dbcon();
	~dbcon();
	int spawnHandler(unsigned int poolsize, std::string worlduuid);
	void terminateHandler();
	std::string processDBCall(boost::property_tree::ptree &dbcall);

private:
	typedef boost::function<std::string(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler)> DB_FUNCTION;
	typedef std::tuple<DB_FUNCTION, int> DB_FUNCTION_INFO;
	typedef std::map<std::string, DB_FUNCTION_INFO> DB_FUNCTIONS;
	DB_FUNCTIONS dbfunctions;

	bool poolinitialized = false;
	bool poolcleanup = false;
	boost::asio::io_service DBioService;
	boost::shared_ptr<boost::asio::io_service::work> DBioServiceWork;

	boost::thread_group asyncthreadpool;
	boost::mutex dbmutex;

	std::mutex msgmutex;
	typedef std::map<PROTOCOL_IDENTIFIER_DATATYPE, std::string> SINGLE_MESSAGE_MAP;
	SINGLE_MESSAGE_MAP msgmap;

	std::string getUUID(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string echo(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string rcvasmsg(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string chkasmsg(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string dbVersion(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string debugCall(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);

	std::string loadPlayer(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string loadAvChars(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string linkChars(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string loadChar(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string createChar(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string updateChar(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string locupdateChar(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string killChar(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);

	std::string loadObject(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string createObject(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string qcreateObject(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string updateObject(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string killObject(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);
	std::string dumpObjects(boost::property_tree::ptree &dbarguments, base_db_handler *dbhandler);


	//base_db_handler tempsyncdbhandler;
	//boost::lockfree::queue<intptr_t, boost::lockfree::capacity<10>> syncdbhandlerpool;
	boost::lockfree::queue<intptr_t, boost::lockfree::fixed_sized<false>> syncdbhandlerpool{1};

	std::string syncCall(DB_FUNCTION_INFO funcinfo, boost::property_tree::ptree &dbarguments);
	std::string asyncCall(DB_FUNCTION_INFO funcinfo, boost::property_tree::ptree &dbarguments);
	std::string quietCall(DB_FUNCTION_INFO funcinfo, boost::property_tree::ptree &dbarguments);
	void asyncCallProcessor(DB_FUNCTION_INFO funcinfo, boost::property_tree::ptree dbarguments, PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier);
	std::string handlelessCall(DB_FUNCTION_INFO funcinfo, boost::property_tree::ptree &dbarguments);
};


#endif /* SOURCE_DBCON_HPP_ */
