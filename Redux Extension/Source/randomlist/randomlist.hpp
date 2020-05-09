/* randomlist.hpp
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

#ifndef SOURCE_RANDOMLIST_HPP_
#define SOURCE_RANDOMLIST_HPP_

#include <algorithm>
#include <string>
#include <map>
#include <tuple>
#include <boost/function.hpp>

#include "extbase.hpp"
#include "constants.hpp"
#include "randomlist/discrete_randomlist.hpp"

class randomlist: public ext_base {
public:
	randomlist(EXT_FUNCTIONS &extFunctions);
	~randomlist();

private:
	typedef std::map<std::string, discrete_list> DISCRETE_LIST_MAP;
	DISCRETE_LIST_MAP DiscreteItemList;

	std::string addDiscreteItemList(std::string &extFunction, ext_arguments &extArguments);
	std::string getDiscreteItemList(std::string &extFunction, ext_arguments &extArguments);
	std::string getRandomNumberList(std::string &extFunction, ext_arguments &extArguments);
};


#endif /* SOURCE_RANDOMLIST_HPP_ */
