/* datetime.cpp
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

#include <unistd.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <cassert>
#include <exception>
#include <stdexcept>
#include <fstream>
#include <ctime>

#include "datetime/datetime.hpp"

datetime::datetime(EXT_FUNCTIONS &extFunctions) {
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DTCALL_FUNCTION_GET_DATE_TIME_Array),
					boost::bind(&datetime::getDateTimeArray, this, _1, _2)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DTCALL_FUNCTION_GET_EPOCH_TIME),
					boost::bind(&datetime::getEpochTime, this, _1, _2)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_DTCALL_FUNCTION_GET_UNIX_TIME),
					boost::bind(&datetime::getEpochTime, this, _1, _2)));
	return;
}

datetime::~datetime() {
	return;
}

std::string datetime::getDateTimeArray(std::string &extFunction, ext_arguments &extArguments) {
	std::stringstream returnString;
	std::time_t now = std::time(0);

	std::tm *ltm = std::localtime(&now);

	returnString << "[" << 1900 + ltm->tm_year
				<< "," << 1 + ltm->tm_mon
				<< "," << ltm->tm_mday
				<< "," << ltm->tm_hour
				<< "," << ltm->tm_min
				<< "," << ltm->tm_sec << "]";

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + returnString.str() + "]";
}

std::string datetime::getEpochTime(std::string &extFunction, ext_arguments &extArguments) {
	std::stringstream returnString;
	std::time_t now = std::time(0);
	returnString << now;

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + returnString.str() + "]";
}
