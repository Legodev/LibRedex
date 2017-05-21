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

extern "C" {
void RVExtension(char *output, int outputSize, const char *function);
}

int main(int argc, char *argv[])
{
	std::string uuid;
	std::string functionstring;
    char output[size];

    const char function[] = "{ 'extFunction': 'iocall', 'extArguments': {  'extFunction': 'GetInitOrder', 'extArguments': {  } } }";
    RVExtension(output, size, function);
    std::cout << "OUTPUT: " << output << std::endl;

    const char function2[] = "{ 'extFunction': 'iocall', 'extArguments': {  'extFunction': 'GetCfgFile', 'extArguments': { 'configfiles': ['PluginManager','DesoDB','Desolation','Realism','AntiSideChat','Jump','TimeModify','EarPlugs','AdminTools'] } } }";
    RVExtension(output, size, function2);
    uuid = output;
    uuid = uuid.substr(11, 32);
    std::cout << "FOUND UUID: " << uuid << std::endl;
    std::cout << "OUTPUT: " << output << std::endl;

    while (strncmp(output, PROTOCOL_MESSAGE_TRANSMIT_FINISHED_MSG, 20) != 0) {
        	functionstring = "{ 'extFunction': 'rcvmsg', 'extArguments': {  'msguuid': '";
        	functionstring += uuid;
    		functionstring += "' } }";

    		RVExtension(output, size, functionstring.c_str());
    		std::cout << output << std::endl;
    	}

    return 0;
}
