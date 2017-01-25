/* dbaddchar.cpp
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
#include "utils/uuid.hpp"
#include <unistd.h>
#include <boost/random/random_device.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#define range 11, 32
#define cachesize 1024

void RVExtension(char *output, int outputSize, const char *function);

std::string randomstring(int length) {
	std::stringstream returnString;
	std::string chars("abcdefghijklmnopqrstuvwxyz"
			"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
			"1234567890");
	boost::random::random_device rng;
	boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
	for (int i = 0; i < length; ++i) {
		returnString << chars[index_dist(rng)];
	}

	return returnString.str();
}

std::string randomsteamid() {
	std::stringstream returnString;
	std::string chars("1234567890");
	boost::random::random_device rng;
	boost::random::uniform_int_distribution<> index_dist(0, chars.size() - 1);
	for (int i = 0; i < 17; ++i) {
		returnString << chars[index_dist(rng)];
	}

	return returnString.str();
}

std::string receivemsg(std::string msguuid) {
	bool loop = true;
	std::string functionstring;
	char output[cachesize];

	std::cout << "receiving msg information of " << msguuid << std::endl;
	functionstring = "{ 'dllFunction': 'dbcall', 'dllArguments': {  'dbfunction': 'chkasmsg', 'dbarguments': { 'msguuid': '";
	functionstring += msguuid + "' } } }";

	while (loop) {
		// std::cout << "JSON: " << functionstring.c_str() << std::endl;
		RVExtension(output, cachesize, functionstring.c_str());
		std::cout << output << std::endl;
		if (strncmp(output, PROTOCOL_MESSAGE_EXISTING, 22) == 0) {
			loop = false;
		} else {
			usleep(5000);
		}
	}

	functionstring = "{ 'dllFunction': 'dbcall', 'dllArguments': {  'dbfunction': 'rcvasmsg', 'dbarguments': { 'msguuid': '";
	functionstring += msguuid + "' } } }";
	std::cout << "receiving msg " << msguuid << std::endl;
	// std::cout << "JSON: " << functionstring.c_str() << std::endl;
	RVExtension(output, cachesize, functionstring.c_str());

	return std::string(output);
}

std::string handleoutput(std::string output) {
	std::string returnmsg = output;
	if (output.substr(2,5) == "ASYNC") {
		returnmsg = receivemsg(output.substr(range));
	}

	return returnmsg;
}


int main(int argc, char *argv[])
{
	std::string uuid;
	std::string functionstring;
	std::string playeruuid;
	std::string objectuuid;
    char output[1024];
    std::string stringoutput;
    const char function[] = "{ 'dllFunction': 'initdb', 'dllArguments': {  'poolsize': 4, 'worlduuid': '11e66ac33a4ccd1c82c510bf48883ace' } }";

    std::cout << "trying to spawn 4 threads" << std::endl;
    // std::cout << "JSON: " << function << std::endl;
    RVExtension(output, 1024, function);
    std::cout << output << std::endl  << std::endl << std::endl;

    usleep(500);

    functionstring = "{ 'dllFunction': 'dbcall', 'dllArguments': {  'dbfunction': 'loadPlayer', 'dbarguments': { 'nickname': '";
    functionstring += randomstring(6) + "', 'steamid': '";
    functionstring += randomsteamid() + "'  } } }";

	std::cout << "loading player data" << std::endl;
	// std::cout << "JSON: " << functionstring.c_str() << std::endl;
	RVExtension(output, 1024, functionstring.c_str());
	stringoutput = handleoutput(output);
	std::cout << stringoutput << std::endl;

	playeruuid = stringoutput.substr(10, 32);
	std::cout << "PLAYERUUID: " << playeruuid << std::endl  << std::endl << std::endl;

	usleep(500);

	functionstring = "{ 'dllFunction': 'dbcall', 'dllArguments': {  'dbfunction': 'createChar', 'dbarguments': { 'playeruuid': '";
	functionstring += playeruuid + "', 'animationstate': 'VAR_ANIMATIONSTATE',  'direction': '-23.5', 'positiontype': '0', 'positionx': '21.42', 'positiony': '666.9', 'positionz': '-133.7', 'classname': 'sampleclass', 'hitpoints': '[]', 'variables': '[]', 'persistentvariables': '[]', 'textures': '[]', 'inventoryuniform': '[]', 'inventoryvest': '[]', 'inventorybackpack': '[]', 'uniform': 'someuniform', 'vest': 'somevest', 'backpack': 'somebackpack', 'headgear': 'someheadgear', 'googles': 'somegoogles', 'primaryweapon': '[\"someprimaryweapon\", [\"someattachment\"]]', 'secondaryweapon': '[\"somesecondaryweapon\", [\"someattachment\"]]', 'handgun': '[\"somehandgunweapon\", [\"someattachment\"]]', 'tools': '[]', 'currentweapon': 'someprimaryweapon' } } }";

	std::cout << "creating character data" << std::endl;
	// std::cout << "JSON: " << functionstring.c_str() << std::endl;
	RVExtension(output, 1024, functionstring.c_str());
	stringoutput = handleoutput(output);
	std::cout << stringoutput << std::endl << std::endl << std::endl;

	usleep(500);

	functionstring = "{ 'dllFunction': 'dbcall', 'dllArguments': {  'dbfunction': 'createObject', 'dbarguments': { 'classname': '";
	functionstring += randomstring(8) + "', 'priority': 2, 'visible': 1, 'accesscode': '', 'locked': 0, 'playeruuid': '";
	functionstring += playeruuid + "', 'hitpoints': '[]',  'damage': 0.1, 'fuel': 0.9, 'fuelcargo': 0.0, 'repaircargo': 0.0, ";
	functionstring += "'items': '[]', 'magazines': '[]', 'weapons': '[]', 'backpacks': '[]', 'magazinesturret': '[]', 'variables': '[]', ";
	functionstring += "'animationstate': '[]', 'textures': '[]', 'direction': '23.5', 'positiontype': '0', 'positionx': '21.42', 'positiony': '666.9', 'positionz': '133.7' } } }";

	std::cout << "creating object data" << std::endl;
	// std::cout << "JSON: " << functionstring.c_str() << std::endl;
	RVExtension(output, 1024, functionstring.c_str());
	stringoutput = handleoutput(output);
	std::cout << stringoutput << std::endl;

	objectuuid = stringoutput.substr(9, 32);
	std::cout << "OBJECTUUID: " << objectuuid << std::endl  << std::endl << std::endl;

	usleep(500);

	functionstring = "{ 'dllFunction': 'dbcall', 'dllArguments': {  'dbfunction': 'updateObject', 'dbarguments': { 'objectuuid': '";
	functionstring += objectuuid + "', 'classname': '";
	functionstring += randomstring(8) + "', 'priority': 2, 'visible': 1, 'accesscode': '', 'locked': 0, 'playeruuid': '";
	functionstring += playeruuid + "', 'hitpoints': '[]',  'damage': 0.1, 'fuel': 0.9, 'fuelcargo': 0.0, 'repaircargo': 0.0, ";
	functionstring += "'items': '[]', 'magazines': '[]', 'weapons': '[]', 'backpacks': '[]', 'magazinesturret': '[]', 'variables': '[]', ";
	functionstring += "'animationstate': '[]', 'textures': '[]', 'direction': '23.5', 'positiontype': '0', 'positionx': '21.42', 'positiony': '666.9', 'positionz': '133.7' } } }";

	std::cout << "update object data" << std::endl;
	// std::cout << "JSON: " << functionstring.c_str() << std::endl;
	RVExtension(output, 1024, functionstring.c_str());
	stringoutput = handleoutput(output);
	std::cout << stringoutput << std::endl << std::endl << std::endl;

	usleep(500);

	functionstring = "{ 'dllFunction': 'dbcall', 'dllArguments': {  'dbfunction': 'createObject', 'dbarguments': { 'classname': '";
	functionstring += randomstring(8) + "', 'priority': 2, 'visible': 1, 'accesscode': '', 'locked': 0, 'playeruuid': '";
	functionstring += "', 'hitpoints': '[]',  'damage': 0.1, 'fuel': 0.9, 'fuelcargo': 0.0, 'repaircargo': 0.0, ";
	functionstring += "'items': '[]', 'magazines': '[]', 'weapons': '[]', 'backpacks': '[]', 'magazinesturret': '[]', 'variables': '[]', ";
	functionstring += "'animationstate': '[]', 'textures': '[]', 'direction': '23.5', 'positiontype': '0', 'positionx': '21.42', 'positiony': '666.9', 'positionz': '133.7' } } }";

	std::cout << "creating object data without owner" << std::endl;
	// std::cout << "JSON: " << functionstring.c_str() << std::endl;
	RVExtension(output, 1024, functionstring.c_str());
	stringoutput = handleoutput(output);
	std::cout << stringoutput << std::endl << std::endl << std::endl;

    return 0;
}
