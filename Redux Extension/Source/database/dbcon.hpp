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
#include "extbase.hpp"
#include "constants.hpp"
#include "database/datacache/objectmysql.hpp"

#include <stdio.h>

class dbcon: public ext_base {
public:
	dbcon(EXT_FUNCTIONS &extFunctions);
	~dbcon();
	std::string spawnHandler(std::string &extFunction, ext_arguments &extArgument);
	void terminateHandler();
	std::string terminateHandler(std::string &extFunction, ext_arguments &extArgument);
	std::string processDBCall(std::string &extFunction, ext_arguments &extArgument);

private:
	typedef boost::function<std::string(ext_arguments &extArgument, base_db_handler *dbhandler)> DB_FUNCTION;
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

	std::map<std::string, object_base*> objectcache;

	std::string getUUID(std::string &extFunction, ext_arguments &extArgument);
	std::string echo(std::string &extFunction, ext_arguments &extArgument);
	std::string rcvasmsg(std::string &extFunction, ext_arguments &extArgument);
	std::string chkasmsg(std::string &extFunction, ext_arguments &extArgument);

	std::string dbVersion(ext_arguments &extArgument, base_db_handler *dbhandler);

	std::string loadPlayer(ext_arguments &extArgument, base_db_handler *dbhandler);
	std::string loadAvChars(ext_arguments &extArgument, base_db_handler *dbhandler);
	std::string linkChars(ext_arguments &extArgument, base_db_handler *dbhandler);
	std::string loadChar(ext_arguments &extArgument, base_db_handler *dbhandler);
	std::string createChar(ext_arguments &extArgument, base_db_handler *dbhandler);
	std::string updateChar(ext_arguments &extArgument, base_db_handler *dbhandler);
	std::string locupdateChar(ext_arguments &extArgument, base_db_handler *dbhandler);
	std::string killChar(ext_arguments &extArgument, base_db_handler *dbhandler);

	std::string loadObject(ext_arguments &extArgument, base_db_handler *dbhandler);
	std::string createObject(ext_arguments &extArgument, base_db_handler *dbhandler);
	std::string qcreateObject(ext_arguments &extArgument, base_db_handler *dbhandler);
	std::string updateObject(ext_arguments &extArgument, base_db_handler *dbhandler);
	std::string killObject(ext_arguments &extArgument, base_db_handler *dbhandler);
	std::string dumpObjects(ext_arguments &extArgument, base_db_handler *dbhandler);


	//base_db_handler tempsyncdbhandler;
	//boost::lockfree::queue<intptr_t, boost::lockfree::capacity<10>> syncdbhandlerpool;
	boost::lockfree::queue<intptr_t, boost::lockfree::fixed_sized<false>> syncdbhandlerpool{1};

	std::string syncCall(DB_FUNCTION_INFO funcinfo, ext_arguments &extArgument);
	std::string asyncCall(DB_FUNCTION_INFO funcinfo, ext_arguments &extArgument);
	std::string quietCall(DB_FUNCTION_INFO funcinfo, ext_arguments &extArgument);
	void asyncCallProcessor(DB_FUNCTION_INFO funcinfo, ext_arguments extArgument, PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier);
};


#endif /* SOURCE_DBCON_HPP_ */
