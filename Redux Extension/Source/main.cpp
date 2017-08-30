/* main.cpp
 *
 * Copyright 2016-2018 Desolation Redux
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

#include "main.hpp"

extern "C"
{
	void RVExtension(char *output, int outputSize, const char *function)
	{
#ifdef DEBUG
			testfile << "\t\t ARMAIO-REQUEST " << function << std::endl;
			testfile.flush();
#endif
			std::string errstr = "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_ERROR) + "\", \"";
			errstr += "Sorry RVExtension is not supported anymore";
			errstr += "\"]";
			strncpy(output, errstr.c_str(), outputSize);
#ifdef DEBUG
			testfile << "\t\t ARMAIO-ERROR " << errstr << std::endl;
			testfile.flush();
#endif
	}
}

extern "C"
{
	void RVExtensionArgs(char *output, int outputSize, const char *function, const char **args, int argsCnt)
	{
		try {
#ifdef DEBUG
			testfile << "\t\t ARMAIO-REQUEST " << function << std::endl;
			testfile.flush();
			for (int i = 0; i < argsCnt; i++) {
				testfile << "\t\t ARMAIO-Argument " << i << ": " << args[i] << std::endl;
				testfile.flush();
			}
#endif
			std::string returnString = extension->processCallExtension(function, args, argsCnt, outputSize);
#ifdef DEBUG
			testfile << "\t\t ARMAIO-RETURN " << returnString << std::endl;
			testfile.flush();
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
			testfile << "\t\t ARMAIO-ERROR " << errstr << std::endl;
			testfile.flush();
#endif
		}
	}
}
