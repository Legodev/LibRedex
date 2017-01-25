/* ext_base.hpp
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

#ifndef SOURCE_EXTBASE_HPP_
#define SOURCE_EXTBASE_HPP_

#include <string>
#include <map>
#include <boost/property_tree/ptree.hpp>
#include <boost/function.hpp>

#include "constants.hpp"

class ext_base {
public:
	std::string processExtCall(boost::property_tree::ptree &extCall) {
		std::string returnString;

		std::string extFunction = extCall.get<std::string>("extFunction");
		boost::property_tree::ptree &extArguments = extCall.get_child("extArguments");

		EXT_FUNCTIONS::iterator it = extFunctions.find(extFunction);
		if (it != extFunctions.end()) {
			const EXT_FUNCTION &func(it->second);

			try {
				returnString = "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\", " + func(extArguments) + "]";
			} catch (std::exception const& e) {
				std::string error = e.what();
				int i = 0;
				while ((i = error.find("\"", i)) != std::string::npos) {
					error.insert(i, "\"");
					i += 2;
				}
				returnString = "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_ERROR) + "\", \"";
				returnString += error;
				returnString += "\"]";
			}

		} else {
			throw std::runtime_error("Don't know extFunction: " + extFunction);
		}

		return returnString;
	};

protected:
	typedef boost::function<std::string(boost::property_tree::ptree &extArguments)> EXT_FUNCTION;
	typedef std::map<std::string, EXT_FUNCTION> EXT_FUNCTIONS;
	EXT_FUNCTIONS extFunctions;
};


#endif /* SOURCE_EXTBASE_HPP_ */
