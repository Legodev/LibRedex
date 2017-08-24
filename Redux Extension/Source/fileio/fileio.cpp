/* fileio.cpp
 *
 * Copyright 2016-2018 Desolation Redux
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

fileio::fileio(EXT_FUNCTIONS &extFunctions) {
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

    boost::property_tree::ptree configtree;
    boost::property_tree::json_parser::read_json(CONFIG_FILE_NAME, configtree);


    for (auto& item : configtree.get_child("fileio.readonce")) {
		readlist.insert(std::make_pair(item.second.get_value<std::string>(), 1));
	}

	for (auto& item : configtree.get_child("fileio.read")) {
		readlist.insert(std::make_pair(item.second.get_value<std::string>(), 0));
	}

	for (auto& item : configtree.get_child("fileio.write")) {
		writelist.insert(std::make_pair(item.second.get_value<std::string>(), 0));
	}

	return;
}

fileio::~fileio() {
	return;
}

#if defined(__linux__)
boost::filesystem::path fileio::ToLowerIfNeeded(boost::filesystem::path Path) {
	boost::filesystem::path loweredPath = boost::algorithm::to_lower_copy(Path.string());

	if (boost::filesystem::exists(loweredPath)) {
		return loweredPath;
	}

	return Path;
}
#endif

boost::filesystem::path fileio::GetConfigPath() {
	boost::filesystem::path configPath = "@DesolationServer";
	configPath /= "Config";

	return configPath;

#if defined(__linux__)
	configPath = ToLowerIfNeeded(configPath);
#endif

	if (boost::filesystem::is_directory(configPath)) {
		return configPath;
	}


	configPath = "Config";
#if defined(__linux__)
	configPath = ToLowerIfNeeded(configPath);
#endif

	if (boost::filesystem::is_directory(configPath)) {
		return configPath;
	}

	throw std::runtime_error("could not find the config path try to lower all names and check the dll location");
}

std::string fileio::GetInitOrder(std::string &extFunction, ext_arguments &extArguments) {
	boost::filesystem::path configPath = GetConfigPath();
	configPath /= "PluginList.cfg";
#if defined(__linux__)
	configPath = ToLowerIfNeeded(configPath);
#endif

	if (boost::filesystem::exists(configPath)) {
		int charpos, linenum;
		std::string returnString = "[";
		std::ifstream infile(configPath.string());
		std::string line;

		linenum = 0;
		while (std::getline(infile, line)) {
			std::istringstream iss(line);

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
	boost::filesystem::path configPath = GetConfigPath();

	std::string returnString = "[";
	int filenum = 0;
	int itemnum = 0;

	for (std::string& filename : extArguments.get_simplelist<std::string>("configfiles")) {

		boost::regex dirupexpression("\\.\\.");
		filename = boost::regex_replace(filename, dirupexpression, "", boost::match_default | boost::format_all);

		boost::filesystem::path filepath = configPath / (filename + ".cfg");

#if defined(__linux__)
		filepath = ToLowerIfNeeded(filepath);
#endif

		if (filenum != 0) {
			returnString += ",";
		}
		returnString += "[";

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
					returnString += ",";
				}

				returnString += "[\"";
				returnString += key;
				returnString += "\",\"";
				returnString += value;
				returnString += "\"]";

				itemnum++;
			}
		}

		returnString += "]";
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
			std::istringstream iss(line);

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
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + "not implemented" + "\"]";
}

std::string fileio::appendFile(std::string &extFunction, ext_arguments &extArguments) {
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\",\"" + "not implemented" + "\"]";
}
