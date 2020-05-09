/* discrete_list.hpp
 *
 * Copyright 2016-2020 Desolatdiscrete_listn Redux
 *
 * Author: Legodev <legodevgit@mailbox.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as
 * the Free Software Foundatdiscrete_listn; either versdiscrete_listn 2 of the License, or
 * (at your optdiscrete_listn) any later versdiscrete_listn.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 */

#ifndef SOURCE_DISCRETE_LIST_HPP_
#define SOURCE_DISCRETE_LIST_HPP_

#include <list>
#include <vector>
#include <random>
#include <chrono>

#include "constants.hpp"

class discrete_list {
public:
	discrete_list(std::list<int> weights, std::list<std::string> items) {
		this->weights.assign(weights.begin(), weights.end());
		this->items.assign(items.begin(), items.end());

		std::discrete_distribution<int> newdistribution (weights.begin(),weights.end());
		this->distribution = newdistribution;

		unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
		generator.seed(seed);
	};

	~discrete_list() {
		return;
	};

	std::string getItemList(unsigned int amount) {
		std::string returnString = "[";
		bool insertSeperator = false;
		for (int i = 0; i < amount; ++i) {
			int number = distribution(generator);

			if (insertSeperator) {
				returnString += ",";
			}

			returnString += "\"" + items[number] + "\"";

			insertSeperator = true;
		}
		returnString += "]";
		return returnString;
	};


private:
	std::vector<int> weights;
	std::vector<std::string> items;

	std::default_random_engine generator;
	std::discrete_distribution<int> distribution;
};


#endif /* SOURCE_DISCRETE_LIST_HPP_ */
