/* dbinittest.cpp
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
#include <queue>
#include "../constants.hpp"
#include <unistd.h>
#define range 11, 32
#define cachesize 1024

void RVExtension(char *output, int outputSize, const char *function);

std::string receivemsg(std::string msguuid) {
	bool loop = true;
	std::string functionstring;
	char output[cachesize];

	std::cout << "receiving msg information of " << msguuid << std::endl;
	functionstring = "{ 'extFunction': 'dbcall', 'extArguments': {  'dbfunction': 'chkasmsg', 'extArgument': { 'msguuid': '";
	functionstring += msguuid + "' } } }";

	while (loop) {
		std::cout << "JSON: " << functionstring.c_str() << std::endl;
		RVExtension(output, cachesize, functionstring.c_str());
		std::cout << output << std::endl;
		if (strncmp(output, PROTOCOL_MESSAGE_EXISTING, 22) == 0) {
			loop = false;
		}
	}

	functionstring = "{ 'extFunction': 'dbcall', 'extArguments': {  'dbfunction': 'rcvasmsg', 'extArgument': { 'msguuid': '";
	functionstring += msguuid + "' } } }";
	std::cout << "receiving msg " << msguuid << std::endl;
	std::cout << "JSON: " << functionstring.c_str() << std::endl;
	RVExtension(output, cachesize, functionstring.c_str());

	return std::string(output);
}

int main(int argc, char *argv[])
{
	std::string uuid;
	std::string functionstring;
	std::queue<std::string> msgqueue;

    char output[cachesize];
    const char function[] = "{ 'extFunction': 'initdb', 'extArguments': {  'poolsize': 4, 'worlduuid': '11e66ac33a4ccd1c82c510bf48883ace' } }";
    const char function2[] = "{ 'extFunction': 'dbcall', 'extArguments': {  'dbfunction': 'dbVersion', 'extArgument': {  } } }";
    const char function3[] = "{ 'extFunction': 'dbcall', 'extArguments': {  'dbfunction': 'dbgCall', 'extArgument': {  } } }";
    const char function4[] = "{ 'extFunction': 'dbcall', 'extArguments': {  'dbfunction': 'dumpObjects', 'extArgument': {  } } }";
    const char function5[] = "{ 'extFunction': 'dbcall', 'extArguments': {  'dbfunction': 'loadPlayer', 'extArgument': { 'nickname': 'Lego', 'steamid': '76561198025362180'  } } }";
    const char function6[] = "{ 'extFunction': 'dbcall', 'extArguments': {  'dbfunction': 'loadAvChars', 'extArgument': { 'playeruuid': '11e66abc1942138d82c510bf48883ace' } } }";
    const char function7[] = "{ 'extFunction': 'dbcall', 'extArguments': {  'dbfunction': 'loadChar', 'extArgument': { 'playeruuid': '11e66abc1942138d82c510bf48883ace' } } }";
    const char function8[] = "{ 'extFunction': 'dbcall', 'extArguments': {  'dbfunction': 'updateChar', 'extArgument': { 'charuuid': '11e66ac3076d214682c510bf48883ace', 'animationstate': 'VAR_ANIMATIONSTATE',  'direction': '23.5', 'positiontype': '0', 'positionx': '21.42', 'positiony': '666.9', 'positionz': '133.7', 'classname': 'sampleclass', 'hitpoints': '[]', 'variables': '[]', 'persistentvariables': '[]', 'textures': '[]', 'inventoryuniform': '[]', 'inventoryvest': '[]', 'inventorybackpack': '[]', 'uniform': 'someuniform', 'vest': 'somevest', 'backpack': 'somebackpack', 'headgear': 'someheadgear', 'googles': 'somegoogles', 'primaryweapon': '[\"someprimaryweapon\", [\"someattachment\"]]', 'secondaryweapon': '[\"somesecondaryweapon\", [\"someattachment\"]]', 'handgun': '[\"somehandgunweapon\", [\"someattachment\"]]', 'tools': '[]', 'currentweapon': 'someprimaryweapon' } } }";
    const char function9[] = "{ 'extFunction': 'dbcall', 'extArguments': {  'dbfunction': 'locupdateChar', 'extArgument': { 'charuuid': '11e66ac3076d25dc82c510bf48883ace', 'animationstate': 'VAR_ANIMATIONSTATE',  'direction': '23.5', 'positiontype': '0', 'positionx': '21.42', 'positiony': '666.9', 'positionz': '133.7'  } } }";
    const char function10[] = "{ 'extFunction': 'dbcall', 'extArguments': {  'dbfunction': 'loadObject', 'extArgument': { 'objectuuid': '11e66b045d8ced8aab0c10bf48883ace' } } }";
    const char function11[] = "{ 'extFunction': 'dbcall', 'extArguments': {  'dbfunction': 'updateObject', 'extArgument': { 'objectuuid': '11e66b045d8ced8aab0c10bf48883ace' } } }";
    std::cout << "trying to spawn 4 threads" << std::endl;
    std::cout << "JSON: " << function << std::endl;
    RVExtension(output, cachesize, function);
    std::cout << output << std::endl  << std::endl << std::endl;

    usleep(500);

    std::cout << "trying to spawn threads again! (should fail)" << std::endl;
    RVExtension(output, cachesize, function);
    std::cout << output << std::endl  << std::endl << std::endl;

    std::cout << "trying to get db version several times" << std::endl;
    std::cout << "JSON: " << function2 << std::endl;
    for (int i = 0; i < 8; i++) {
    	RVExtension(output, cachesize, function2);
    	std::cout << output << std::endl  << std::endl << std::endl;
    }

    usleep(500);

    std::cout << "executing an debug call" << std::endl;
    std::cout << "JSON: " << function3 << std::endl;
	RVExtension(output, cachesize, function3);
	std::cout << output << std::endl  << std::endl << std::endl;
	msgqueue.push(std::string(output).substr(range));

	usleep(500);

	std::cout << "executing dumpObjects" << std::endl;
	std::cout << "JSON: " << function4 << std::endl;
	RVExtension(output, cachesize, function4);
	std::cout << output << std::endl  << std::endl << std::endl;
	msgqueue.push(std::string(output).substr(range));

	usleep(500);

	std::cout << "loading player data" << std::endl;
	std::cout << "JSON: " << function5 << std::endl;
	RVExtension(output, cachesize, function5);
	std::cout << output << std::endl  << std::endl << std::endl;
	msgqueue.push(std::string(output).substr(range));

	usleep(500);

	std::cout << "JSON: " << function6 << std::endl;
	RVExtension(output, cachesize, function6);
	std::cout << output << std::endl  << std::endl << std::endl;
	msgqueue.push(std::string(output).substr(range));

	usleep(500);

	std::cout << "JSON: " << function7 << std::endl;
	RVExtension(output, cachesize, function7);
	std::cout << output << std::endl  << std::endl << std::endl;
	msgqueue.push(std::string(output).substr(range));

	usleep(500);

	std::cout << "JSON: " << function8 << std::endl;
	RVExtension(output, cachesize, function8);
	std::cout << output << std::endl  << std::endl << std::endl;
	msgqueue.push(std::string(output).substr(range));

	usleep(500);

	std::cout << "JSON: " << function9 << std::endl;
	RVExtension(output, cachesize, function9);
	std::cout << output << std::endl  << std::endl << std::endl;
	msgqueue.push(std::string(output).substr(range));

	usleep(500);

	std::cout << "JSON: " << function10 << std::endl;
	RVExtension(output, cachesize, function10);
	std::cout << output << std::endl  << std::endl << std::endl;
	msgqueue.push(std::string(output).substr(range));

	usleep(500);

//	std::cout << "JSON: " << function11 << std::endl;
//	RVExtension(output, cachesize, function11);
//	std::cout << output << std::endl  << std::endl << std::endl;
//
//	usleep(500);

	usleep(100000);
	std::cout << std::endl << std::endl << "sleeping some time before getting async messages" << std::endl << std::endl << std::endl;
	usleep(1000000);
	while (!msgqueue.empty())
	{
		bool loop = true;
	    std::string str = msgqueue.front();

	    std::string returnmsg = receivemsg(str);
		std::cout << returnmsg << std::endl << std::endl << std::endl ;

	    msgqueue.pop();
	}


    return 0;
}
