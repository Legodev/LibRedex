/* redex.hpp
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

#ifndef SOURCE_REDEX_HPP_
#define SOURCE_REDEX_HPP_

#include <string>
#include <map>
#include <queue>
#include <mutex>
#include <boost/property_tree/ptree.hpp>
#include <boost/function.hpp>
#include "constants.hpp"
#include "database/dbcon.hpp"
#include "fileio/fileio.hpp"
#include "datetime/datetime.hpp"
#include "randomlist/randomlist.hpp"

class redex {
public:
	redex();
	~redex();
	std::string processCallExtension(const char *function, int outputSize);
	void terminate();

private:
	typedef boost::function<
			std::string(boost::property_tree::ptree &dbarguments)> DLL_FUNCTION;
	typedef std::map<std::string, DLL_FUNCTION> DLL_FUNCTIONS;
	DLL_FUNCTIONS dllFunctions;

	dbcon dbconnection;
	fileio fileinputoutput;
	datetime datetimeobj;
	randomlist randomlistobj;

	std::mutex msgmutex;
	typedef std::map<PROTOCOL_IDENTIFIER_DATATYPE, std::queue<std::string>> MESSAGE_MAP;
	MESSAGE_MAP msgmap;

	std::string initdb(boost::property_tree::ptree &dllArguments);
	std::string termdb(boost::property_tree::ptree &dllArguments);
	std::string dbcall(boost::property_tree::ptree &dllArguments);
	std::string iocall(boost::property_tree::ptree &dllArguments);
	std::string dtcall(boost::property_tree::ptree &dllArguments);
	std::string rlcall(boost::property_tree::ptree &dllArguments);
	std::string rcvmsg(boost::property_tree::ptree &dllArguments);
	std::string chkmsg(boost::property_tree::ptree &dllArguments);

	std::string multipartMSGGenerator(std::string returnString, int outputSize);
};

#endif /* SOURCE_REDEX_HPP_ */
