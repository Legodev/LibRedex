/* randomlist.hpp
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

#ifndef SOURCE_RANDOMLIST_HPP_
#define SOURCE_RANDOMLIST_HPP_

#include <algorithm>
#include <string>
#include <map>
#include <tuple>
#include <boost/property_tree/ptree.hpp>
#include <boost/function.hpp>

#include "extbase.hpp"
#include "constants.hpp"
#include "randomlist/discrete_randomlist.hpp"

class randomlist: public ext_base {
public:
	randomlist();
	~randomlist();

private:
	typedef std::map<std::string, discrete_list> DISCRETE_LIST_MAP;
	DISCRETE_LIST_MAP DiscreteItemList;

	std::string addDiscreteItemList(boost::property_tree::ptree &extArguments);
	std::string getDiscreteItemList(boost::property_tree::ptree &extArguments);
};


#endif /* SOURCE_RANDOMLIST_HPP_ */
