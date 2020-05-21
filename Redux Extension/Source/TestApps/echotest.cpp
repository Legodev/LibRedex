/* echotest.cpp
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
#include "../constants.hpp"

#include "testfileheader.hpp"

#define range 10, 32
#define cachesize 128

int main(int argc, char *argv[])
{
	std::string uuid;

	std::list<std::string> stringList;
	const char ** StringArray;
    char output[cachesize];

    stringList.clear();
    stringList.push_back("echostring");
    stringList.push_back("The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog. The quick brown fox jumps over the lazy dog.");
    StringArray = UseListOfStringToArrayofCharArray(stringList);

    std::cout << "SENDING stringList: " << std::endl;
    for(std::string stringItem : stringList) {
    	std::cout << "Item: " << stringItem << std::endl;
    }
    std::cout << std::endl << std::endl;

    RVExtensionArgs(output, cachesize, "echo", StringArray, stringList.size());
    delete [] StringArray;


    uuid = output;

    if (uuid.std::string::find("ERROR") != std::string::npos) {
        std::cout << "an error occured:" << uuid << '\n';
        return 1;
    }

    uuid = uuid.substr(10, 32);
    std::cout << "FOUND UUID: " << uuid << std::endl;

    std::cout << output << std::endl;

    while (strncmp(output, PROTOCOL_MESSAGE_TRANSMIT_FINISHED_MSG, 20) != 0) {
    	stringList.clear();
    	stringList.push_back("msguuid");
    	stringList.push_back(uuid);
    	StringArray = UseListOfStringToArrayofCharArray(stringList);

		std::cout << "receiving msg " << uuid << std::endl;
		RVExtensionArgs(output, cachesize, "rcvmsg", StringArray, stringList.size());
		std::cout << output << std::endl;
	}


    delete [] StringArray;
    return 0;
}
