/* fileio.cpp
 *
 * Copyright 2016-2020 Desolation Redux
 *
 * Author: Legodev <legodevgit@mailbox.org>
 *         Kegan <ryancheek@hush.com>
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
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/case_conv.hpp>
#include <boost/regex.hpp>
#include <cassert>
#include <exception>
#include <stdexcept>
#include <fstream>

#include "fileio/fileio.hpp"

fileio::fileio(EXT_FUNCTIONS &extFunctions, boost::property_tree::ptree configtree) {
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_IOCALL_FUNCTION_READ_FILE),
					std::make_tuple(
							boost::bind(&fileio::readFile, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_IOCALL_FUNCTION_WRITE_FILE),
					std::make_tuple(
							boost::bind(&fileio::writeFile, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_IOCALL_FUNCTION_APPEND_FILE),
					std::make_tuple(
							boost::bind(&fileio::appendFile, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_IOCALL_FUNCTION_PLUGINSYSTEM_GETINITORDER),
					std::make_tuple(
							boost::bind(&fileio::GetInitOrder, this, _1, _2),
							SYNC_MAGIC)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_IOCALL_FUNCTION_PLUGINSYSTEM_GETCFGFILE),
					std::make_tuple(
							boost::bind(&fileio::GetCfgFile, this, _1, _2),
							SYNC_MAGIC)));

	for (auto& item : configtree.get_child("fileio.readonce")) {
		readlist.insert(std::make_pair(item.second.get_value<std::string>(), 1));
	}

	for (auto& item : configtree.get_child("fileio.read")) {
		readlist.insert(std::make_pair(item.second.get_value<std::string>(), 0));
	}

	for (auto& item : configtree.get_child("fileio.write")) {
		//std::ofstream file;
		writelist.insert(std::make_pair(item.second.get_value<std::string>(), new std::ofstream()));
	}

	return;
}

fileio::~fileio() {
	this->terminateHandler();
	return;
}

void fileio::terminateHandler() {
	for (auto it = writelist.begin(); it != writelist.end(); ++it) {
		if (it->second != 0) {
			if (it->second->is_open()) {
				it->second->close();
			}
			delete it->second;

			it->second = 0;
		}
	}
	return;
}

boost::filesystem::path fileio::GetConfigPath(std::string filename, bool useAsStartPath) {
	const std::string toFind = boost::algorithm::to_lower_copy(filename);

	auto begin = boost::filesystem::recursive_directory_iterator(this->configPath);
	const auto end = boost::filesystem::recursive_directory_iterator();

	for (; begin != end; ++begin) {
		const std::string path = boost::algorithm::to_lower_copy(begin->path().string());
		if (path.find(toFind) != std::string::npos) {
			if (!boost::filesystem::is_directory(path)) {
				if (useAsStartPath) {
					// speedup future search by starting on the last searched directory
					this->configPath = begin->path().parent_path();
				}
				return begin->path();
			}
		}
	}

	throw std::runtime_error("could not find the path to config '" + filename + "', i started searching on path: " + this->configPath.string());
}

std::string fileio::GetInitOrder(std::string &extFunction, ext_arguments &extArguments) {
	boost::filesystem::path configPath;

	// try to get special config
	try {
		std::string worlduuid = extArguments.getUUID(PROTOCOL_DBCALL_ARGUMENT_WORLDUUID);
		configPath = GetConfigPath("PluginList_" + worlduuid + ".cfg", true);
	}
	// fallback to default config
	catch (const std::runtime_error&) {
		configPath = GetConfigPath("PluginList.cfg", true);
	}

	if (boost::filesystem::exists(configPath)) {
		int charpos, linenum;
		std::string returnString = "[";
		std::ifstream infile(configPath.string());
		std::string line;

		linenum = 0;
		while (std::getline(infile, line)) {
			line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());

			//std::istringstream iss(line);

			int i = 0;
			while ((i = line.find("\"", i)) != std::string::npos) {
				line.insert(i, "\"");
				i += 2;
			}

			// remove comments
			boost::regex commentexpression("\\s*#.*");
			line = boost::regex_replace(line, commentexpression, "");

			// skip empty lines
			boost::regex EmptyLineRegex("^\\s*$");
			boost::cmatch what;
			if (boost::regex_match(line.c_str(), what, EmptyLineRegex)) {
				continue;
			}

			// add second " to get the string after call compile to concatinate with the remaining parts
			charpos = 0;
			while ((charpos = line.find("\"", charpos)) != std::string::npos) {
				line.insert(charpos, "\"");
				charpos += 2;
			}

			if (linenum != 0) {
				returnString += ",";
			}

			returnString += "\"" + line + "\"";

			linenum++;
		}

		returnString += "]";

		return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + returnString + "]";
	} else {
		throw std::runtime_error("cannot read file " + configPath.string());
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",[]]";
}

std::string fileio::GetCfgFile(std::string &extFunction, ext_arguments &extArguments) {
	std::string returnString = "[";
	int filenum = 0;
	int itemnum = 0;

	for (std::string& filename : extArguments.get_simplelist<std::string>("configfiles")) {
		boost::regex dirupexpression("\\.\\.");
		filename = boost::regex_replace(filename, dirupexpression, "", boost::match_default | boost::format_all);

		if (filenum != 0) {
			returnString += ",";
		}
		
		try {
			boost::filesystem::path filepath = GetConfigPath(filename + ".cfg");
			
			std::string dataString = "[";
	
			if (boost::filesystem::exists(filepath)) {
				boost::property_tree::ptree configtree;
				boost::property_tree::ini_parser::read_ini(filepath.string(), configtree);
	
				itemnum = 0;
				BOOST_FOREACH(boost::property_tree::ptree::value_type &val, configtree) {
					// val.first is the name of the child.
					// val.second is the child tree.
					std::string key = val.first;
					std::string value = val.second.get_value<std::string>();
					int i = 0;
	
					while ((i = key.find("\"", i)) != std::string::npos) {
						key.insert(i, "\"");
						i += 2;
					}
	
					i = 0;
					while ((i = value.find("\"", i)) != std::string::npos) {
						value.insert(i, "\"");
						i += 2;
					}
	
					// remove comments
					boost::regex commentexpression("\\s*#.*");
					value = boost::regex_replace(value, commentexpression, "");
	
					if (itemnum != 0) {
						dataString += ",";
					}
	
					dataString += "[\"";
					dataString += key;
					dataString += "\",\"";
					dataString += value;
					dataString += "\"]";
	
					itemnum++;
				}
			}
			
			dataString += "]";
			
			returnString += dataString;
		} catch (const std::runtime_error& e) {
			// valid data is always of type array, this will make errors of type string
			returnString += "\"";
			returnString += e.what();
			returnString += "\"";
		}
		filenum++;
	}
	returnString += "]";
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + returnString + "]";
}

std::string fileio::readFile(std::string &extFunction, ext_arguments &extArguments) {
	std::string filename = extArguments.get<std::string>("filename");

	FILE_IO_MAP::iterator it = readlist.find(filename);
	if (it != readlist.end()) {
		if (it->second == 1) {
			readlist.erase(it);
		}

		int charpos, linenum;
		std::string returnString = "[";
		std::ifstream infile(filename);
		std::string line;

		linenum = 0;
		while (std::getline(infile, line)) {
			//std::istringstream iss(line);

			// add second " to get the string after call compile to concatinate with the remaining parts
			charpos = 0;
			while ((charpos = line.find("\"", charpos)) != std::string::npos) {
				line.insert(charpos, "\"");
				charpos += 2;
			}

			if (linenum != 0) {
				returnString += ",";
			}

			returnString += "\"" + line + "\"";

			linenum++;
		}

		returnString += "]";
	} else {
		throw std::runtime_error("cannot read file: " + filename);
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + filename + "]";
}

std::string fileio::writeFile(std::string &extFunction, ext_arguments &extArguments) {
	std::string filename = extArguments.get<std::string>("filename");
	std::string message = extArguments.get<std::string>("message");

	FILE_WRITE_MAP::iterator it = writelist.find(filename);
	if (it != writelist.end()) {
		if (!it->second->is_open()) {
			it->second->open(filename, std::ios::out | std::ios::trunc);
		}

		*it->second << message << std::endl;
		it->second->flush();
	} else {
		throw std::runtime_error("cannot open file: " + filename);
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + filename + "]";
}

std::string fileio::appendFile(std::string &extFunction, ext_arguments &extArguments) {
	std::string filename = extArguments.get<std::string>("filename");
	std::string message = extArguments.get<std::string>("message");

	FILE_WRITE_MAP::iterator it = writelist.find(filename);
	if (it != writelist.end()) {
		if (!it->second->is_open()) {
			it->second->open(filename, std::ios::out | std::ios::app);
		}

		*it->second << message << std::endl;
		it->second->flush();
	} else {
		throw std::runtime_error("cannot open file: " + filename);
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + filename + "]";
}
