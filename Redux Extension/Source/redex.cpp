/* redex.cpp
 *
 * Copyright 2016-2020 Desolation Redux
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
#include <boost/filesystem.hpp>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <cassert>
#include <exception>
#include <stdexcept>

#include "redex.hpp"
#include "utils/uuid.hpp"

extern int(*callbackPtr)(char const *name, char const *function, char const *data);
#ifdef DEBUG
	#include "logger.hpp"
	extern Logger logfile;
#endif

redex::redex(std::string LibRedexFilePath) {
#ifdef DEBUG
	logfile << "initializing libredex core" << std::endl;
	logfile.flush();
#endif
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_LIBARY_FUNCTION_TERMINATE_ALL),
					std::make_tuple(
							boost::bind(&redex::terminateAll, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_RETURN_ASYNC_SATE),
					std::make_tuple(
							boost::bind(&redex::chkmsg, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_DBCALL_FUNCTION_RETURN_ASYNC_MSG),
					std::make_tuple(
							boost::bind(&redex::rcvasmsg, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_RECEIVE_MESSAGE),
					std::make_tuple(
							boost::bind(&redex::rcvmsg, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_CHECK_MESSAGE_STATE),
					std::make_tuple(
							boost::bind(&redex::chkmsg, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_CHECK_VERSION),
					std::make_tuple(
							boost::bind(&redex::version, this, _1, _2),
							SYNC_MAGIC)));

	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_CHECK_VERSION_ASYNC),
					std::make_tuple(
							boost::bind(&redex::version, this, _1, _2),
							ASYNC_MAGIC)));
	
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_LIBARY_FUNCTION_CHECK_VERSION_CALLBACK),
					std::make_tuple(
							boost::bind(&redex::version, this, _1, _2),
							CALLBACK_MAGIC)));

#ifdef DEBUG
	logfile << "creating filepath from " << LibRedexFilePath << std::endl;
	logfile.flush();
#endif
	LibRedexConfigFilePath = LibRedexFilePath;
#ifdef DEBUG
	logfile << "creating config file path using " << CONFIG_FILE_NAME << std::endl;
	logfile.flush();
#endif
	LibRedexConfigFilePath = LibRedexConfigFilePath.parent_path() / CONFIG_FILE_NAME;

#ifdef DEBUG
	logfile << "checking if config file exists using path " << LibRedexConfigFilePath << std::endl;
	logfile.flush();
#endif
	if (!boost::filesystem::exists(LibRedexConfigFilePath)) {
#ifndef DEBUG		
		std::ofstream logfile;
		logfile.open("libredex.log", std::ios::out | std::ios::trunc);
#endif
		logfile << "cannot find config file: " << LibRedexConfigFilePath << std::endl;
		logfile << "LibRedexFilePath: " << LibRedexFilePath << std::endl;
		logfile.flush();
#ifndef DEBUG
		logfile.close();
#endif
	} else {
		// spawn some idle work
		REDEXioServiceWork.reset( new boost::asio::io_service::work(REDEXioService) );

#ifdef DEBUG
		logfile << "trying to read config file using path " << LibRedexConfigFilePath << std::endl;
		logfile.flush();
#endif
		boost::property_tree::ptree configtree;
		boost::property_tree::json_parser::read_json(LibRedexConfigFilePath.string(), configtree);

		unsigned int poolsize = configtree.get<unsigned int>("gamesettings.poolsize");
		std::string type = configtree.get<std::string>("database.type");

		if (poolsize < 1) {
			poolsize = 1;
		}

#ifdef DEBUG
		logfile << "creating threads using the size of " << poolsize << std::endl;
		logfile.flush();
#endif
		for (unsigned int i = 0; i < poolsize; i++) {
			asyncthreadpool.create_thread(
					boost::bind(static_cast<std::size_t (boost::asio::io_service::*)()>(&boost::asio::io_service::run), &REDEXioService)
			);
		}

#ifdef DEBUG
		logfile << "initializing the database module" << std::endl;
		logfile.flush();
#endif
		if (boost::iequals(type, "MYSQL")) {
			extModules.emplace_back(new mysql_db_handler(extFunctions, configtree));
		}

#ifdef DEBUG
		logfile << "initializing the fileio module" << std::endl;
		logfile.flush();
#endif
		extModules.emplace_back(new fileio(extFunctions, configtree));
#ifdef DEBUG
		logfile << "initializing the datetime module" << std::endl;
		logfile.flush();
#endif
		extModules.emplace_back(new datetime(extFunctions, configtree));
#ifdef DEBUG
		logfile << "initializing the randomlist module" << std::endl;
		logfile.flush();
#endif
		extModules.emplace_back(new randomlist(extFunctions, configtree));
#ifdef DEBUG
		logfile << "initializing the restserver module" << std::endl;
		logfile.flush();
#endif
		extModules.emplace_back(new restserver(extFunctions, configtree));
#ifdef DEBUG
		logfile << "done initializing libredex core" << std::endl;
		logfile.flush();
#endif
	}
	return;
}

redex::~redex() {
	REDEXioServiceWork.reset(); // stop all idle work!
	REDEXioService.stop(); // terminate all work
	// commented because of a bug with boost thread pool if join_all gets executed in process detach on windows
#ifdef __linux__
	asyncthreadpool.join_all(); // get rid of all threads
#endif

	for (auto &module : extModules) {
		module->terminateHandler();
	}

	return;
}

void redex::terminate() {
#ifdef DEBUG
	logfile << "TERMINATE REDEX" << std::endl;
	logfile.flush();
#endif

	REDEXioServiceWork.reset(); // stop all idle work!

	// wait until all jobs are finished
	while (!REDEXioService.stopped()) {
		//boost::thread::yield();
	}

	// should not be needed
	REDEXioService.stop();

	// get rid of all threads
	asyncthreadpool.join_all();

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
		EXT_FUNCTION_INFO funcinfo = it->second;

		switch (std::get<1>(funcinfo)) {
		case SYNC_MAGIC:
			returnString = syncCall(funcinfo, extArguments);
			break;

		case ASYNC_MAGIC:
			returnString = asyncCall(funcinfo, extArguments);
			break;
			
		case CALLBACK_MAGIC:
			returnString = callbackCall(funcinfo, extArguments);
			break;

		case QUIET_MAGIC:
			returnString = quietCall(funcinfo, extArguments);
			break;

		default:
			throw std::runtime_error("unknown function class");
		}

	} else {
		returnString = "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_ERROR) + "\",\"Don't know extFunction: ";
		returnString += extFunction;
		returnString += " - maybe you are missing the config file: ";
		returnString += LibRedexConfigFilePath.string();
		returnString += " - see: libredex.log\"]";
	}

	if (returnString.length() > outputSize) {
		returnString = multipartMSGGenerator(returnString, outputSize);
	}

	return returnString;
}

std::string redex::syncCall(EXT_FUNCTION_INFO funcinfo, ext_arguments &extArgument) {
	const EXT_FUNCTION &func(std::get<0>(funcinfo));
	std::string returnString = "code to be changed";

	try {
		returnString = func(returnString, extArgument);
	} catch (std::exception const& e) {
		throw std::runtime_error(e.what());
	}

	return returnString;
}

std::string redex::asyncCall(EXT_FUNCTION_INFO funcinfo, ext_arguments &extArgument) {
	ext_arguments copyextArgument = extArgument;
	PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier = PROTOCOL_IDENTIFIER_GENERATOR;

	REDEXioService.post(boost::bind(&redex::asyncCallProcessor, this, funcinfo, extArgument, messageIdentifier));

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_ASYNC) + "\",\"" + messageIdentifier + "\"]" ;
}

std::string redex::callbackCall(EXT_FUNCTION_INFO funcinfo, ext_arguments &extArgument) {
	ext_arguments copyextArgument = extArgument;
	PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier = PROTOCOL_IDENTIFIER_GENERATOR;

	REDEXioService.post(boost::bind(&redex::callbackCallProcessor, this, funcinfo, extArgument, messageIdentifier));

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_CALLBACK) + "\",\"" + messageIdentifier + "\"]" ;
}

std::string redex::quietCall(EXT_FUNCTION_INFO funcinfo, ext_arguments &extArgument) {
	ext_arguments copyextArgument = extArgument;
	PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier = "";

	REDEXioService.post(boost::bind(&redex::asyncCallProcessor, this, funcinfo, extArgument, messageIdentifier));

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_QUIET) + "\",\"\"]" ;
}

void redex::asyncCallProcessor(EXT_FUNCTION_INFO funcinfo, ext_arguments extArgument, PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier) {
	std::string returnString;
	std::queue<std::string> stringqueue;

	try {
		returnString = this->syncCall(funcinfo, extArgument);
	} catch (std::exception const& e) {
		std::string error = e.what();
#ifdef DEBUG
			logfile << "INTERNAL ERROR " << error << std::endl;
			logfile.flush();
#endif
		int i = 0;
		while ((i = error.find("\"", i)) != std::string::npos) {
			error.insert(i, "\"");
			i += 2;
		}
		returnString = "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_ERROR) + "\",\"";
		returnString += error;
		returnString += "\"]";
	}

	if (messageIdentifier != "") {
		stringqueue.push(returnString);

		msgmutex.lock();
		msgmap.insert(std::make_pair(messageIdentifier, stringqueue));
		msgmutex.unlock();
	}

	return;
}

void redex::callbackCallProcessor(EXT_FUNCTION_INFO funcinfo, ext_arguments extArgument, PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier) {
	std::string returnString;
	
	
	try {
		returnString = this->syncCall(funcinfo, extArgument);
	} catch (std::exception const& e) {
		std::string error = e.what();
#ifdef DEBUG
			logfile << "INTERNAL ERROR " << error << std::endl;
			logfile.flush();
#endif
		int i = 0;
		while ((i = error.find("\"", i)) != std::string::npos) {
			error.insert(i, "\"");
			i += 2;
		}
		returnString = "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_ERROR) + "\",\"";
		returnString += error;
		returnString += "\"]";
	}
	
	if (callbackPtr != nullptr) {
		callbackPtr(PROTOCOL_CALLBACK_NAME_CALLBACK, messageIdentifier.c_str(), returnString.c_str());
	}
		
	return;
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

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"DONE\"]";
}

std::string redex::rcvasmsg(std::string &extFunction, ext_arguments &extArgument) {
	PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier = extArgument.get<PROTOCOL_IDENTIFIER_DATATYPE>(PROTOCOL_IDENTIFIER_NAME);
	std::queue<std::string> *stringqueue;
	std::string returnString;

	msgmutex.lock();
	MESSAGE_MAP::iterator it = msgmap.find(messageIdentifier);
	msgmutex.unlock();

	// check if message object was found
	if (it == msgmap.end()) {
		returnString = PROTOCOL_MESSAGE_NOT_EXISTING;
	} else {
		// extract message object
		stringqueue = &it->second;

		if (!stringqueue->empty()) {
			returnString = stringqueue->front();
		} else {
			returnString = "";
		}

		msgmutex.lock();
		msgmap.erase (it);
		msgmutex.unlock();
	}

	return returnString;
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
	PROTOCOL_IDENTIFIER_DATATYPE messageIdentifier = extArguments.get<PROTOCOL_IDENTIFIER_DATATYPE>(PROTOCOL_IDENTIFIER_NAME);
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

std::string redex::version(std::string extFunction, ext_arguments &extArguments) {
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\", " + std::string(DLLVERSIONSTRING) + "]";
}

