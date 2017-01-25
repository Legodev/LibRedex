/* redex.cpp
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
#include <iostream>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cassert>
#include <exception>
#include <stdexcept>

#include "redex.hpp"
#include "utils/uuid.hpp"

redex::redex() {
	dllFunctions.insert(
			std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_EXECUTE_INIT_DB),
					boost::bind(&redex::initdb, this, _1)));
	dllFunctions.insert(
				std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_EXECUTE_TERMINATE_DB),
						boost::bind(&redex::termdb, this, _1)));
	dllFunctions.insert(
			std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_EXECUTE_DB_CALL),
					boost::bind(&redex::dbcall, this, _1)));
	dllFunctions.insert(
				std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_EXECUTE_IO_CALL),
						boost::bind(&redex::iocall, this, _1)));
	dllFunctions.insert(
				std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_EXECUTE_DT_CALL),
						boost::bind(&redex::dtcall, this, _1)));
	dllFunctions.insert(
					std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_EXECUTE_RL_CALL),
							boost::bind(&redex::rlcall, this, _1)));
	dllFunctions.insert(
				std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_RECEIVE_MESSAGE),
						boost::bind(&redex::rcvmsg, this, _1)));
	dllFunctions.insert(
					std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_CHECK_MESSAGE_STATE),
							boost::bind(&redex::chkmsg, this, _1)));
	return;
}
redex::~redex() {
	return;
}

void redex::terminate() {
	dbconnection.terminateHandler();

	return;
}

std::string redex::processCallExtension(const char *function, int outputSize) {
	std::string returnString;
	std::stringstream functionstream;

	// NEEDED to make sure there is room for the last '\0'
	outputSize -= 1;

	for (unsigned int i = 0; function[i] != 0; i++) {
		switch(function[i]) {
			case '-': if (function[i+1] > '0' && function[i+1] < '9') { functionstream << "-"; }; break;
			case ';': break;
			case '#': break;
			case '"': functionstream << "\\\\\\\""; break;
			case '\'': functionstream << "\""; break;
			case '\\': functionstream << "\\\\\\\\"; break;
			default: functionstream << function[i]; break;
		}
	}

	printf("INPUT: %s\n", functionstream.str().c_str());

	boost::property_tree::ptree pt;
	boost::property_tree::read_json(functionstream, pt);

	std::string dllFunction = pt.get<std::string>("dllFunction");
	boost::property_tree::ptree &dllArguments = pt.get_child("dllArguments");

	DLL_FUNCTIONS::iterator it = dllFunctions.find(dllFunction);
	if (it != dllFunctions.end()) {
		const DLL_FUNCTION &func(it->second);

		try {
			returnString = func(dllArguments);
		} catch (std::exception const& e) {
			std::string error = e.what();
			int i = 0;
			while ((i = error.find("\"", i)) != std::string::npos) {
				error.insert(i, "\"");
				i += 2;
			}
			returnString = "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_ERROR)+ "\", \"";
			returnString += error;
			returnString += "\"]";
		}

	} else {
		throw std::runtime_error("Don't know dllFunction: " + dllFunction);
	}

	if (returnString.length() > outputSize) {
		returnString = multipartMSGGenerator(returnString, outputSize);
	}

	return returnString;
}

std::string redex::multipartMSGGenerator(std::string returnString, int outputSize) {
	PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier = PROTOCOL_IDENTIFIER_GENERATOR;
	std::string firststring;
	std::queue<std::string> stringqueue;
	int firststringlength;
	int i = 0;

	// safe some characters for the protocol overhead
	// int firstoutputSize = outputSize - MAXCHARS_FOR_PROTOCOL_OVERHEAD;
	int firstoutputSize = outputSize - STATIC_MULTIPART_MESSAGE_PROTOCOL_OVERHEAD;

	// BEGIN TODO
	// find better way to find the perfect length for first string because escaping
	// the double quotes causes a problem if there are many double quotes
	firststring = returnString.substr(0, firstoutputSize);
	firststringlength = firststring.length();
	i = 0;
	while ((i = firststring.find("\"", i)) != std::string::npos) {
		firststring.insert(i, "\"");
		i += 2;
	}
	// END TODO

	firstoutputSize = firstoutputSize - firststring.length() + firststringlength;
	firststring = returnString.substr(0, firstoutputSize);
	i = 0;
	// add second " to get the string after call compile to concatinate with the remaining parts
	while ((i = firststring.find("\"", i)) != std::string::npos) {
		firststring.insert(i, "\"");
		i += 2;
	}

	// split the remaining string at outputSize
	for (i = firstoutputSize; i < returnString.length(); i += outputSize) {
		stringqueue.push(returnString.substr(i, outputSize));
	}

	msgmutex.lock();
	msgmap.insert(std::make_pair(messageIdentifier, stringqueue));
	msgmutex.unlock();

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MULTIPART) + "\", \"" + messageIdentifier + "\", \"" + firststring + "\"]";
}


std::string redex::initdb(boost::property_tree::ptree &dbcall) {
	int poolsize = dbcall.get<int>("poolsize");
	std::string worlduuid = dbcall.get<std::string>("worlduuid");

	//poolsize =
	dbconnection.spawnHandler(poolsize, worlduuid);

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\", [\"" + std::to_string(poolsize) + "\", \"Threads spawned\"]]";
}

std::string redex::termdb(boost::property_tree::ptree &dbcall) {
	dbconnection.terminateHandler();

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\", \"DONE\"]";
}

std::string redex::rcvmsg(boost::property_tree::ptree &dllArguments) {
	PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier = dllArguments.get<PROTOCOL_IDENTIFIER_DATATYPE>(PROTOCOL_IDENTIFIER_NAME);
	std::queue<std::string> *stringqueue;
	std::string returnString;

	msgmutex.lock();
	MESSAGE_MAP::iterator it = msgmap.find(messageIdentifier);
	msgmutex.unlock();

	// check if message object was found
	if (it == msgmap.end()) {
		throw std::runtime_error("Message " + messageIdentifier + " does not exist");
	}

	// extract message object
	stringqueue = &it->second;

	if (stringqueue->empty()) {
		// delete message object
		msgmutex.lock();
		msgmap.erase (it);
		msgmutex.unlock();
		// signal arma that it got the last message
		returnString = PROTOCOL_MESSAGE_TRANSMIT_FINISHED_MSG;
	} else {
		// get next message and remove it from queue
		returnString = stringqueue->front();
		stringqueue->pop();
	}

	return returnString;
}

std::string redex::chkmsg(boost::property_tree::ptree &dllArguments) {
	PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier = dllArguments.get<PROTOCOL_IDENTIFIER_DATATYPE>("messageIdentifier");
	std::queue<PROTOCOL_IDENTIFIER_DATATYPE> *stringqueue;
	std::string returnString;

	msgmutex.lock();
	MESSAGE_MAP::iterator it = msgmap.find(messageIdentifier);
	msgmutex.unlock();

	// check if message object was found
	if (it == msgmap.end()) {
		returnString = PROTOCOL_MESSAGE_NOT_EXISTING;
	} else {
		returnString = PROTOCOL_MESSAGE_EXISTING;

		/*
		 * dunno if we want more information
		stringqueue = &it->second;
		if (stringqueue->empty()) {
			returnString = PROTOCOL_MESSAGE_EMPTY;
		} else {
			returnString = PROTOCOL_MESSAGE_EXISTING;
		}
		*/
	}

	return returnString;
}

std::string redex::dbcall(boost::property_tree::ptree &dllArguments) {
	return dbconnection.processDBCall(dllArguments);
}

std::string redex::iocall(boost::property_tree::ptree &dllArguments) {
	return fileinputoutput.processExtCall(dllArguments);
}

std::string redex::dtcall(boost::property_tree::ptree &dllArguments) {
	return datetimeobj.processExtCall(dllArguments);
}

std::string redex::rlcall(boost::property_tree::ptree &dllArguments) {
	return randomlistobj.processExtCall(dllArguments);
}
