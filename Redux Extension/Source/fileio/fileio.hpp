/* fileio.hpp
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

#ifndef SOURCE_FILEIO_HPP_
#define SOURCE_FILEIO_HPP_

#include <algorithm>
#include <string>
#include <map>
#include <tuple>
#include <boost/function.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include "extbase.hpp"
#include "constants.hpp"

class fileio: public ext_base {
public:
	fileio(EXT_FUNCTIONS &extFunctions);
	~fileio();
	void terminateHandler();

private:

	typedef std::map<std::string, int> FILE_IO_MAP;
	FILE_IO_MAP readlist;

	typedef std::map<std::string, std::ofstream *> FILE_WRITE_MAP;
	FILE_WRITE_MAP writelist;
	
	boost::filesystem::path configPath = ".";

	std::string readFile(std::string &extFunction, ext_arguments &extArguments);
	std::string writeFile(std::string &extFunction, ext_arguments &extArguments);
	std::string appendFile(std::string &extFunction, ext_arguments &extArguments);
	std::string GetInitOrder(std::string &extFunction, ext_arguments &extArguments);
	std::string GetCfgFile(std::string &extFunction, ext_arguments &extArguments);

	boost::filesystem::path GetConfigPath(std::string filename, bool useAsStartPath = false);
};


#endif /* SOURCE_FILEIO_HPP_ */
