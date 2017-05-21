/* uuidtest.cpp
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

#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include "../constants.hpp"

#include "testfileheader.hpp"

#define size 1024

int main(int argc, char *argv[])
{
	std::string uuid;
	std::string functionstring;
    char output[size];

    const char function[] = "{ 'extFunction': 'rlcall', 'extArguments': {  'extFunction': 'addDiscreteItemList', 'extArguments': { 'listName': 'testlist', 'weights': [20, 5, 3, 2], 'items': ['Cat', 'Dog', 'Bird', 'Mouse'] } } }";
    std::cout << "SENT: " << function << std::endl;
    RVExtension(output, size, function);
    std::cout << "OUTPUT: " << output << std::endl;

    const char function2[] = "{ 'extFunction': 'rlcall', 'extArguments': {  'extFunction': 'getDiscreteItemList', 'extArguments': { 'listName': 'testlist', 'itemAmount': 1 } } }";
	std::cout << "SENT: " << function2 << std::endl;
	RVExtension(output, size, function2);
	std::cout << "OUTPUT: " << output << std::endl;

	const char function3[] = "{ 'extFunction': 'rlcall', 'extArguments': {  'extFunction': 'getDiscreteItemList', 'extArguments': { 'listName': 'testlist', 'itemAmount': 20 } } }";
	std::cout << "SENT: " << function3 << std::endl;
	RVExtension(output, size, function3);
	std::cout << "OUTPUT: " << output << std::endl;

	const char function4[] = "{ 'extFunction': 'rlcall', 'extArguments': {  'extFunction': 'addDiscreteItemList', 'extArguments': { 'listName': 'testlist2', 'weights': [20, 5, 3, 2], 'items': [] } } }";
	std::cout << "SENT: " << function4 << std::endl;
	RVExtension(output, size, function4);
	std::cout << "OUTPUT: " << output << std::endl;

	const char function5[] = "{ 'extFunction': 'rlcall', 'extArguments': {  'extFunction': 'addDiscreteItemList', 'extArguments': { 'listName': 'testlist3', 'weights': [], 'items': ['Cat', 'Dog', 'Bird', 'Mouse'] } } }";
	std::cout << "SENT: " << function5 << std::endl;
	RVExtension(output, size, function5);
	std::cout << "OUTPUT: " << output << std::endl;

	const char function6[] = "{ 'extFunction': 'rlcall', 'extArguments': {  'extFunction': 'addDiscreteItemList', 'extArguments': { 'listName': 'testlist4', 'weights': [20, 5, 3, 2], 'items': ['Cat', 'Dog', 'Bird'] } } }";
	std::cout << "SENT: " << function6 << std::endl;
	RVExtension(output, size, function6);
	std::cout << "OUTPUT: " << output << std::endl;

    return 0;
}
