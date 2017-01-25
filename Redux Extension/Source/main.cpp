/* main.cpp
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

#include "main.hpp"

void RVExtension(char *output, int outputSize, const char *function)
{
	try {
#ifdef DEBUG
		testfile << "REQUEST " << function << std::endl;
#endif
		std::string returnString = extension->processCallExtension(function, outputSize);
#ifdef DEBUG
		testfile << "RETURN " << returnString << std::endl;
#endif
		strncpy(output, returnString.c_str(), outputSize);
		return;
	} catch (std::exception const& e) {
		std::string error = e.what();
		int i = 0;
		while ((i = error.find("\"", i)) != std::string::npos) {
			error.insert(i, "\"");
			i += 2;
		}
		std::string errstr = "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_ERROR) + "\", \"";
		errstr += error;
		errstr += "\"]";
		strncpy(output, errstr.c_str(), outputSize);
#ifdef DEBUG
		testfile << "ERROR " << errstr << std::endl;
#endif
	}
}
