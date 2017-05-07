/* fileio.hpp
 *
 * Copyright 2016-2017 Desolation Redux
 *
 * Author: Legodev <legodevgit@mailbox.org>
 *         Kegan <ryancheek@hush.com>
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

private:

	typedef std::map<std::string, int> FILE_IO_MAP;
	FILE_IO_MAP readlist;
	FILE_IO_MAP writelist;

	std::string readFile(std::string &extFunction, ext_arguments &extArguments);
	std::string writeFile(std::string &extFunction, ext_arguments &extArguments);
	std::string appendFile(std::string &extFunction, ext_arguments &extArguments);
	std::string GetInitOrder(std::string &extFunction, ext_arguments &extArguments);
	std::string GetCfgFile(std::string &extFunction, ext_arguments &extArguments);

	boost::filesystem::path GetConfigPath();
	#if defined(__linux__)
	boost::filesystem::path ToLowerIfNeeded(boost::filesystem::path configPath);
	#endif
};


#endif /* SOURCE_FILEIO_HPP_ */
