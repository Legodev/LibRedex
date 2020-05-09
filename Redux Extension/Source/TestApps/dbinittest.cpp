/* dbinittest.cpp
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

#include <iostream>
#include <sstream>
#include <string>
#include <string.h>
#include <queue>
#include <unistd.h>
#include "../constants.hpp"

#include "testfileheader.hpp"

#define range 10, 32
#define cachesize 1024

std::string receiveAsyncMsg(std::string msguuid) {
	bool loop = true;

	std::list<std::string> stringList;
	const char ** StringArray;
    char output[cachesize];

    stringList.clear();
    stringList.push_back("msguuid");
    stringList.push_back(msguuid);
    StringArray = UseListOfStringToArrayofCharArray(stringList);

    std::cout << "SENDING stringList: " << std::endl;
    for(std::string stringItem : stringList) {
    	std::cout << "Item: " << stringItem << std::endl;
    }
    std::cout << std::endl << std::endl;

	while (loop) {
		RVExtensionArgs(output, cachesize, "chkasmsg", StringArray, stringList.size());
		std::cout << output << std::endl;
		if (strncmp(output, PROTOCOL_MESSAGE_EXISTING, 22) == 0) {
			loop = false;
		}
	}

	std::cout << "receiving msg " << msguuid << std::endl;
	RVExtensionArgs(output, cachesize, "rcvasmsg", StringArray, stringList.size());

	delete [] StringArray;

	return std::string(output);
}

int main(int argc, char *argv[])
{
	std::string uuid;
	std::string functionstring;
	std::queue<std::string> msgqueue;

	std::list<std::string> stringList;
	const char ** StringArray;
    char output[cachesize];

    /* FUNCTIONCALL BEGIN */
    stringList.clear();
    stringList.push_back("poolsize");
    stringList.push_back("4");
    stringList.push_back("worlduuid");
    stringList.push_back("11e66ac33a4ccd1c82c510bf48883ace");

    StringArray = UseListOfStringToArrayofCharArray(stringList);

    std::cout << "SENDING stringList: " << std::endl;
    for(std::string stringItem : stringList) {
    	std::cout << "Item: " << stringItem << std::endl;
    }
    std::cout << std::endl << std::endl;

	RVExtensionArgs(output, cachesize, "initdb", StringArray, stringList.size());
	std::cout << "Return: " << output << std::endl  << std::endl << std::endl;

	delete [] StringArray;
	/* FUNCTIONCALL END */

	/* FUNCTIONCALL BEGIN */
	stringList.clear();
	StringArray = UseListOfStringToArrayofCharArray(stringList);

	std::cout << "SENDING stringList: " << std::endl;
	for(std::string stringItem : stringList) {
		std::cout << "Item: " << stringItem << std::endl;
	}
	std::cout << std::endl << std::endl;

	RVExtensionArgs(output, cachesize, "dbVersion", StringArray, stringList.size());
	std::cout << "Return: " << output << std::endl  << std::endl << std::endl;

	delete [] StringArray;
	/* FUNCTIONCALL END */

	/* FUNCTIONCALL BEGIN */
	stringList.clear();
	StringArray = UseListOfStringToArrayofCharArray(stringList);

	std::cout << "SENDING stringList: " << std::endl;
	for(std::string stringItem : stringList) {
		std::cout << "Item: " << stringItem << std::endl;
	}
	std::cout << std::endl << std::endl;

	RVExtensionArgs(output, cachesize, "dumpObjects", StringArray, stringList.size());
	std::cout << "Return: " << output << std::endl  << std::endl << std::endl;
	msgqueue.push(std::string(output).substr(range));

	delete [] StringArray;
	/* FUNCTIONCALL END */
//
//	/* FUNCTIONCALL BEGIN */
//	stringList.clear();
//	stringList.push_back("nickname");
//	stringList.push_back("Lego");
//	stringList.push_back("steamid");
//	stringList.push_back("76561198025362180");
//
//	StringArray = UseListOfStringToArrayofCharArray(stringList);
//
//	std::cout << "SENDING stringList: " << std::endl;
//	for(std::string stringItem : stringList) {
//		std::cout << "Item: " << stringItem << std::endl;
//	}
//	std::cout << std::endl << std::endl;
//
//	RVExtensionArgs(output, cachesize, "loadPlayer", StringArray, stringList.size());
//	std::cout << "Return: " << output << std::endl  << std::endl << std::endl;
//	msgqueue.push(std::string(output).substr(range));
//
//	delete [] StringArray;
//	/* FUNCTIONCALL END */
//
//	/* FUNCTIONCALL BEGIN */
//	stringList.clear();
//	stringList.push_back("playeruuid");
//	stringList.push_back("11e66abc1942138d82c510bf48883ace");
//
//	StringArray = UseListOfStringToArrayofCharArray(stringList);
//
//	std::cout << "SENDING stringList: " << std::endl;
//	for(std::string stringItem : stringList) {
//		std::cout << "Item: " << stringItem << std::endl;
//	}
//	std::cout << std::endl << std::endl;
//
//	RVExtensionArgs(output, cachesize, "loadAvChars", StringArray, stringList.size());
//	std::cout << "Return: " << output << std::endl  << std::endl << std::endl;
//	msgqueue.push(std::string(output).substr(range));
//
//	delete [] StringArray;
//	/* FUNCTIONCALL END */
//
//	/* FUNCTIONCALL BEGIN */
//	stringList.clear();
//	stringList.push_back("playeruuid");
//	stringList.push_back("11e66abc1942138d82c510bf48883ace");
//
//	StringArray = UseListOfStringToArrayofCharArray(stringList);
//
//	std::cout << "SENDING stringList: " << std::endl;
//	for(std::string stringItem : stringList) {
//		std::cout << "Item: " << stringItem << std::endl;
//	}
//	std::cout << std::endl << std::endl;
//
//	RVExtensionArgs(output, cachesize, "loadChar", StringArray, stringList.size());
//	std::cout << "Return: " << output << std::endl  << std::endl << std::endl;
//	msgqueue.push(std::string(output).substr(range));
//
//	delete [] StringArray;
//	/* FUNCTIONCALL END */
//
//	/* FUNCTIONCALL BEGIN */
//	stringList.clear();
//	stringList.push_back("charuuid");
//	stringList.push_back("11e66ac3076d214682c510bf48883ace");
//	stringList.push_back("animationstate");
//	stringList.push_back("VAR_ANIMATIONSTATE");
//	stringList.push_back("direction");
//	stringList.push_back("23.5");
//	stringList.push_back("positiontype");
//	stringList.push_back("0");
//	stringList.push_back("positionx");
//	stringList.push_back("21.42");
//	stringList.push_back("positiony");
//	stringList.push_back("666.9");
//	stringList.push_back("positionz");
//	stringList.push_back("133.7");
//	stringList.push_back("classname");
//	stringList.push_back("sampleclass");
//	stringList.push_back("hitpoints");
//	stringList.push_back("[]");
//	stringList.push_back("variables");
//	stringList.push_back("[]");
//	stringList.push_back("persistentvariables");
//	stringList.push_back("[]");
//	stringList.push_back("textures");
//	stringList.push_back("[]");
//	stringList.push_back("gear");
//	stringList.push_back("[]");
//	stringList.push_back("inventoryvest");
//	stringList.push_back("[]");
//	stringList.push_back("inventorybackpack");
//	stringList.push_back("[]");
//	stringList.push_back("uniform");
//	stringList.push_back("someuniform");
//	stringList.push_back("vest");
//	stringList.push_back("somevest");
//	stringList.push_back("backpack");
//	stringList.push_back("somebackpack");
//	stringList.push_back("headgear");
//	stringList.push_back("someheadgear");
//	stringList.push_back("googles");
//	stringList.push_back("somegoogles");
//	stringList.push_back("primaryweapon");
//	stringList.push_back("[\"someprimaryweapon\", [\"someattachment\"]]");
//	stringList.push_back("secondaryweapon");
//	stringList.push_back("[\"somesecondaryweapon\", [\"someattachment\"]]");
//	stringList.push_back("handgun");
//	stringList.push_back("[\"somehandgunweapon\", [\"someattachment\"]]");
//	stringList.push_back("tools");
//	stringList.push_back("[]");
//	stringList.push_back("currentweapon");
//	stringList.push_back("someprimaryweapon");
//
//	StringArray = UseListOfStringToArrayofCharArray(stringList);
//
//	std::cout << "SENDING stringList: " << std::endl;
//	for(std::string stringItem : stringList) {
//		std::cout << "Item: " << stringItem << std::endl;
//	}
//	std::cout << std::endl << std::endl;
//
//	RVExtensionArgs(output, cachesize, "updateChar", StringArray, stringList.size());
//	std::cout << "Return: " << output << std::endl  << std::endl << std::endl;
//	msgqueue.push(std::string(output).substr(range));
//
//	std::cout << "SENDING stringList: " << std::endl;
//	for(std::string stringItem : stringList) {
//		std::cout << "Item: " << stringItem << std::endl;
//	}
//	std::cout << std::endl << std::endl;
//
//	RVExtensionArgs(output, cachesize, "locupdateChar", StringArray, stringList.size());
//	std::cout << "Return: " << output << std::endl  << std::endl << std::endl;
//	msgqueue.push(std::string(output).substr(range));
//
//	delete [] StringArray;
//	/* FUNCTIONCALL END */
//
//	/* FUNCTIONCALL BEGIN */
////	stringList.clear();
////	stringList.push_back("objectuuid");
////	stringList.push_back("11e66b045d8ced8aab0c10bf48883ace");
////
////	StringArray = UseListOfStringToArrayofCharArray(stringList);
////
////	std::cout << "SENDING stringList: " << std::endl;
////	for(std::string stringItem : stringList) {
////		std::cout << "Item: " << stringItem << std::endl;
////	}
////	std::cout << std::endl << std::endl;
////
////	RVExtensionArgs(output, cachesize, "loadObject", StringArray, stringList.size());
////	std::cout << "Return: " << output << std::endl  << std::endl << std::endl;
////	msgqueue.push(std::string(output).substr(range));
////
////	delete [] StringArray;
////	/* FUNCTIONCALL END */

	usleep(100000);
	std::cout << std::endl << std::endl << "sleeping some time before getting async messages" << std::endl << std::endl << std::endl;
	usleep(1000000);
	while (!msgqueue.empty())
	{
		bool loop = true;
	    std::string str = msgqueue.front();

	    std::string returnmsg = receiveAsyncMsg(str);
		std::cout << returnmsg << std::endl << std::endl << std::endl ;

	    msgqueue.pop();
	}


    return 0;
}
