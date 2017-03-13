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
#include <vector>
#include <queue>
#include <mutex>
#include <boost/property_tree/ptree.hpp>
#include <boost/function.hpp>
#include "constants.hpp"
#include "extbase.hpp"
#include "database/dbcon.hpp"
#include "fileio/fileio.hpp"
#include "datetime/datetime.hpp"
#include "randomlist/randomlist.hpp"

class redex {
public:
	redex();
	~redex();
	std::string processCallExtension(const char *function, const char **args, int argsCnt, int outputSize);
	void terminate();

private:
	EXT_FUNCTIONS extFunctions;

	typedef std::vector<std::unique_ptr<ext_base>> extModulesType;
	extModulesType extModules;

	std::mutex msgmutex;
	typedef std::map<PROTOCOL_IDENTIFIER_DATATYPE, std::queue<std::string>> MESSAGE_MAP;
	MESSAGE_MAP msgmap;

	std::string terminateAll(std::string extFunction, ext_arguments &extArguments);
	std::string rcvmsg(std::string extFunction, ext_arguments &extArguments);
	std::string chkmsg(std::string extFunction, ext_arguments &extArguments);
	std::string multipartMSGGenerator(std::string returnString, int outputSize);
};

#endif /* SOURCE_REDEX_HPP_ */
