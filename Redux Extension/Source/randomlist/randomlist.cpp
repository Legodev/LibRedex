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
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <exception>
#include <stdexcept>
#include <fstream>


randomlist::randomlist(EXT_FUNCTIONS &extFunctions) {
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_RANDOM_FUNCTION_ADD_DISCRETE_LIST),
					boost::bind(&randomlist::addDiscreteItemList, this, _1, _2)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_RANDOM_FUNCTION_GET_DISCRETE_LIST),
					boost::bind(&randomlist::getDiscreteItemList, this, _1, _2)));
	extFunctions.insert(
			std::make_pair(std::string(PROTOCOL_RANDOM_FUNCTION_GET_RANDOMNUMBER_LIST),
					boost::bind(&randomlist::getRandomNumberList, this, _1, _2)));

    boost::property_tree::ptree configtree;
    boost::property_tree::json_parser::read_json(CONFIG_FILE_NAME, configtree);

	return;
}

randomlist::~randomlist() {
	return;
}

std::string randomlist::addDiscreteItemList(std::string &extFunction, ext_arguments &extArguments) {
	std::string returnString = "";
	std::string listName = extArguments.get<std::string>("listName");
	//ext_arguments &extArguments = extArguments.get_child("extArguments");

	DISCRETE_LIST_MAP::iterator it = DiscreteItemList.find(listName);
	if (it != DiscreteItemList.end()) {
		throw std::runtime_error("List does already exist: " + listName);
	} else {
		std::list<int> weights;
		std::list<std::string> items;

		weights = extArguments.get_simplelist<int>("weights");
		items = extArguments.get_simplelist<std::string>("items");

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

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + returnString + "]";
}

std::string randomlist::getDiscreteItemList(std::string &extFunction, ext_arguments &extArguments) {
	std::string returnString = "";
	std::string listName = extArguments.get<std::string>("listName");
	unsigned int itemAmount = extArguments.get<unsigned int>("itemAmount");

	DISCRETE_LIST_MAP::iterator it = DiscreteItemList.find(listName);
	if (it != DiscreteItemList.end()) {
		returnString = it->second.getItemList(itemAmount);
	} else {
		throw std::runtime_error("could not find list: " + listName);
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + returnString + "]";
}

std::string randomlist::getRandomNumberList(std::string &extFunction, ext_arguments &extArguments) {
	std::string returnString = "[";
	bool insertSeperator = false;

	std::string type = extArguments.get<std::string>("type");
	unsigned int amount = extArguments.get<unsigned int>("amount");

	std::default_random_engine generator;
	unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
	generator.seed(seed);

	if (boost::iequals(type, "int")) {
		std::uniform_int_distribution<int> distribution(
				extArguments.get<int>("start"), extArguments.get<int>("end"));

		for (int i = 0; i < amount; ++i) {
			int number = distribution(generator);

			if (insertSeperator) {
				returnString += ",";
			}

			returnString += std::to_string(number);

			insertSeperator = true;
		}
	} else {
		std::uniform_real_distribution<float> distribution(
				extArguments.get<float>("start"), extArguments.get<float>("end"));

		for (int i = 0; i < amount; ++i) {
			float number = distribution(generator);

			if (insertSeperator) {
				returnString += ",";
			}

			returnString += std::to_string(number);

			insertSeperator = true;
		}
	}

	returnString += "]";
	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\"," + returnString + "]";
}
