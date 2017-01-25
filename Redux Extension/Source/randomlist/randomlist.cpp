/* randomlist.cpp
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

#include "randomlist/randomlist.hpp"

#include <unistd.h>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/property_tree/ini_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/regex.hpp>
#include <cassert>
#include <exception>
#include <stdexcept>
#include <fstream>


randomlist::randomlist() {
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_RANDOM_FUNCTION_ADD_DISCRETE_LIST),
					boost::bind(&randomlist::addDiscreteItemList, this, _1)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_RANDOM_FUNCTION_GET_DISCRETE_LIST),
					boost::bind(&randomlist::getDiscreteItemList, this, _1)));

    boost::property_tree::ptree configtree;
    boost::property_tree::json_parser::read_json(CONFIG_FILE_NAME, configtree);

	return;
}

randomlist::~randomlist() {
	return;
}

std::string randomlist::addDiscreteItemList(boost::property_tree::ptree &extArguments) {
	std::string returnString = "";
	std::string listName = extArguments.get<std::string>("listName");
	//boost::property_tree::ptree &extArguments = extArguments.get_child("extArguments");

	DISCRETE_LIST_MAP::iterator it = DiscreteItemList.find(listName);
	if (it != DiscreteItemList.end()) {
		throw std::runtime_error("List does already exist: " + listName);
	} else {
		std::list<int> weights;
		std::list<std::string> items;

		for (auto& item : extArguments.get_child("weights")) {
			int itemWeight = item.second.get_value<int>();
			weights.push_back(itemWeight);
		}

		for (auto& item : extArguments.get_child("items")) {
			std::string itemClass = item.second.get_value<std::string>();
			items.push_back(itemClass);
		}

		if (weights.empty()) {
				throw std::runtime_error("The weights array is empty!");
		}

		if (items.empty()) {
				throw std::runtime_error("The items array is empty!");
		}

		if (weights.size() != items.size()) {
				throw std::runtime_error("The weights and items array need to have the same size!");
		}

		DiscreteItemList.insert(std::make_pair(listName, discrete_list(weights, items)));
		returnString = "\"Done\"";
	}

	return returnString;
}

std::string randomlist::getDiscreteItemList(boost::property_tree::ptree &extArguments) {
	std::string returnString = "";
	std::string listName = extArguments.get<std::string>("listName");
	unsigned int itemAmount = extArguments.get<unsigned int>("itemAmount");

	DISCRETE_LIST_MAP::iterator it = DiscreteItemList.find(listName);
	if (it != DiscreteItemList.end()) {
		returnString = it->second.getItemList(itemAmount);
	} else {
		throw std::runtime_error("could not find list: " + listName);
	}

	return returnString;
}
