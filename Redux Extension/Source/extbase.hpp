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

#include <list>
#include <string>
#include <sstream>
#include <boost/lexical_cast.hpp>

class ext_arguments {
public:
	void add(std::string key, std::string value) {
		argmap.insert(std::make_pair(key,value));
		return;
	}

	int addargs(const char **args, int argsCnt) {
		if (argsCnt % 2 == 0) {
			for (int i = 0; i < argsCnt; i += 2) {
				argmap.insert(std::make_pair(args[i],args[i+1]));
			}
		} else {
			throw std::runtime_error("the amount of items in the array is not even");
		}
		return 0;
	}

	template<typename ReturnType>
	ReturnType get(std::string identifier) {
		std::string Argument;

		ARGUMENT_MAP::iterator it = argmap.find(identifier);
		if (it != argmap.end()) {
			Argument = it->second;
		} else {
			throw std::runtime_error("did not find identifier: " + identifier);
		}

	    return boost::lexical_cast<ReturnType>(Argument);
	}

	template<typename ReturnType>
	std::list<ReturnType> get_simplelist(std::string identifier) {
		int brakedcount = 0;
		int doublequotecount = 0;
		std::list<ReturnType> returnList;
		std::stringstream ArgumentStream;

		std::string Argument;

		ARGUMENT_MAP::iterator it = argmap.find(identifier);
		if (it != argmap.end()) {
			Argument = it->second;
		} else {
			throw std::runtime_error("did not find identifier: " + identifier);
		}

		for (char& c : Argument) {
			switch (c) {
			case '[':
				brakedcount++;
				if (brakedcount > 1) {
					ArgumentStream << c;
				}
				break;
			case ']':
				brakedcount--;
				if (brakedcount < 1) {
					// should not be needed, but just in case
					brakedcount = 0;

					returnList.push_back(boost::lexical_cast<ReturnType>(ArgumentStream.str()));

					// empty the stringstream
					ArgumentStream.str("");
				} else {
					ArgumentStream << c;
				}
				break;
			case ',':
				// keep commas in strings
				if (brakedcount == 1 && doublequotecount % 2 == 0) {
					returnList.push_back(boost::lexical_cast<ReturnType>(ArgumentStream.str()));

					// empty the stringstream
					ArgumentStream.str("");
				} else {
					ArgumentStream << c;
				}
				break;
			case '"':
				doublequotecount++;
				if (brakedcount != 1) {
					ArgumentStream << c;
				}
				break;
			default:
				ArgumentStream << c;
				break;
			}
		}

		return returnList;
	}

protected:
	typedef std::map<std::string, std::string> ARGUMENT_MAP;
	ARGUMENT_MAP argmap;
};

class ext_base {
public:
	int spawnHandler(std::string extFunction, ext_arguments &extArgument) {
		return 0;
	}

	void terminateHandler() {
		return;
	}

	void terminateHandler(std::string extFunction, ext_arguments &extArgument) {
		return this->terminateHandler();
	}

protected:
};

typedef boost::function<std::string(std::string &extFunction, ext_arguments &extArgument)> EXT_FUNCTION;
typedef std::map<std::string, EXT_FUNCTION> EXT_FUNCTIONS;

#endif /* SOURCE_EXTBASE_HPP_ */
