/* dbcon.hpp
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

#ifndef SOURCE_DBCON_HPP_
#define SOURCE_DBCON_HPP_

#include <string>
#include <map>
#include <tuple>
#include <cstdint>
#include <mutex>
#include <boost/property_tree/ptree.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/thread/mutex.hpp>

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
	bool poolinitialized = false;
	bool poolcleanup = false;

	boost::mutex dbmutex;

	std::map<std::string, cache_base*> objectcache;
	std::map<std::string, cache_base*> charactercache;

	std::string getUUID(std::string &extFunction, ext_arguments &extArgument);
	std::string echo(std::string &extFunction, ext_arguments &extArgument);

	std::string dbVersion(std::string &extFunction, ext_arguments &extArgument);

	std::string loadPlayer(std::string &extFunction, ext_arguments &extArgument);
	std::string loadAvChars(std::string &extFunction, ext_arguments &extArgument);
	std::string linkChars(std::string &extFunction, ext_arguments &extArgument);
	std::string loadChar(std::string &extFunction, ext_arguments &extArgument);
	std::string createChar(std::string &extFunction, ext_arguments &extArgument);
	std::string updateChar(std::string &extFunction, ext_arguments &extArgument);
	std::string killChar(std::string &extFunction, ext_arguments &extArgument);

	std::string loadObject(std::string &extFunction, ext_arguments &extArgument);
	std::string createObject(std::string &extFunction, ext_arguments &extArgument);
	std::string qcreateObject(std::string &extFunction, ext_arguments &extArgument);
	std::string updateObject(std::string &extFunction, ext_arguments &extArgument);
	std::string killObject(std::string &extFunction, ext_arguments &extArgument);
	std::string dumpObjects(std::string &extFunction, ext_arguments &extArgument);


	//boost::lockfree::queue<intptr_t, boost::lockfree::capacity<10>> dbhandlerpool;
	boost::lockfree::queue<intptr_t, boost::lockfree::fixed_sized<false>> dbhandlerpool{1};
	base_db_handler * getDBHandler();
	void returnDBHandler(base_db_handler * dbhandler);
};


#endif /* SOURCE_DBCON_HPP_ */
