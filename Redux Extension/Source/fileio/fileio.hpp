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
#include <boost/property_tree/ptree.hpp>
#include <boost/function.hpp>

#include "extbase.hpp"
#include "constants.hpp"

class fileio: public ext_base {
public:
	fileio();
	~fileio();

private:

	typedef std::map<std::string, int> FILE_IO_MAP;
	FILE_IO_MAP readlist;
	FILE_IO_MAP writelist;

	std::string readFile(boost::property_tree::ptree &extArguments);
	std::string writeFile(boost::property_tree::ptree &extArguments);
	std::string appendFile(boost::property_tree::ptree &extArguments);
	std::string GetInitOrder(boost::property_tree::ptree &extArguments);
	std::string GetCfgFile(boost::property_tree::ptree &extArguments);
};


#endif /* SOURCE_FILEIO_HPP_ */
