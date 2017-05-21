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

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <fstream>
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
	extFunctions.insert(
				std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_TERMINATE_ALL),
						boost::bind(&redex::terminateAll, this, _1, _2)));
	extFunctions.insert(
				std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_RECEIVE_MESSAGE),
						boost::bind(&redex::rcvmsg, this, _1, _2)));
	extFunctions.insert(
					std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_CHECK_MESSAGE_STATE),
							boost::bind(&redex::chkmsg, this, _1, _2)));

	if (access(CONFIG_FILE_NAME, F_OK) == -1) {
		std::ofstream logfile;
		logfile.open("LibRedExErrorLogFile.txt", std::ios::out | std::ios::trunc);
		logfile << "cannot find config file: " << CONFIG_FILE_NAME << std::endl;
		logfile.flush();
		logfile.close();
	} else {
		extModules.emplace_back(new dbcon(extFunctions));
		extModules.emplace_back(new fileio(extFunctions));
		extModules.emplace_back(new datetime(extFunctions));
		extModules.emplace_back(new randomlist(extFunctions));
	}

	return;
}

redex::~redex() {
	return;
}

void redex::terminate() {
	for (auto &module : extModules) {
	    module->terminateHandler();
	}

	return;
}

std::string redex::processCallExtension(const char *function, const char **args, int argsCnt, int outputSize) {
	std::string returnString;
	std::stringstream functionstream;

	// NEEDED to make sure there is room for the last '\0'
	outputSize -= 1;

	std::string extFunction = function;
	ext_arguments extArguments;
	extArguments.addargs(args, argsCnt);

	EXT_FUNCTIONS::iterator it = extFunctions.find(extFunction);
	if (it != extFunctions.end()) {
		const EXT_FUNCTION &func(it->second);

		try {
			returnString = func(extFunction, extArguments);
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

	} else {
		returnString = "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_ERROR) + "\",\"Don't know extFunction: ";
		returnString += extFunction;
		returnString += " - maybe you are missing the config file: ";
		returnString += CONFIG_FILE_NAME;
		returnString += " - see: LibRedExErrorLogFile.txt\"]";
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

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MULTIPART) + "\",\"" + messageIdentifier + "\",\"" + firststring + "\"]";
}

std::string redex::terminateAll(std::string extFunction, ext_arguments &extArguments) {
	this->terminate();

	return "DONE";
}

std::string redex::rcvmsg(std::string extFunction, ext_arguments &extArguments) {
	PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier = extArguments.get<PROTOCOL_IDENTIFIER_DATATYPE>(PROTOCOL_IDENTIFIER_NAME);
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

std::string redex::chkmsg(std::string extFunction, ext_arguments &extArguments) {
	PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier = extArguments.get<PROTOCOL_IDENTIFIER_DATATYPE>("messageIdentifier");
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

