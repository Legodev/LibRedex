/* main.cpp
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

#include "main.hpp"

extern "C"
{
	void RVExtensionVersion(char *output, int outputSize)
	{
		if (outputSize > 0) {
			char version[] = DLLVERSIONSTRING;
			//--- max outputSize is 32 bytes
			strncpy(output, version, outputSize);
			output[outputSize - 1] = '\0';
		}
	}
}

extern "C"
{
	void RVExtension(char *output, int outputSize, const char *function)
	{
		if (outputSize > 0) {
#ifdef DEBUG
			logfile << "### --> ARMAIO-REQ --> ### " << function << std::endl;
			logfile.flush();
#endif
			std::string errstr = "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_ERROR) + "\", \"";
			errstr += "Sorry RVExtension is not supported anymore";
			errstr += "\"]";
			strncpy(output, errstr.c_str(), outputSize);
			output[outputSize - 1] = '\0';
#ifdef DEBUG
			logfile << "### >_< ARMAIO-ERR >_< ### " << errstr << std::endl;
			logfile.flush();
#endif
		}
	}
}

extern "C"
{
	void RVExtensionArgs(char *output, int outputSize, const char *function, const char **args, int argsCnt)
	{
		if (outputSize > 0) {
#ifdef DEBUG
			std::time_t result;
#endif
			try {
#ifdef DEBUG
				logfile << "### --> ARMAIO-REQ --> ### " << function << std::endl;
				logfile.flush();
				for (int i = 0; i < argsCnt; i++) {
					logfile << "### --- ARMAIO-ARG --- ### " << i << ": " << args[i] << std::endl;
					logfile.flush();
				}
#endif
				std::string returnString = extension->processCallExtension(function, args, argsCnt, outputSize);
#ifdef DEBUG
				logfile << "### <-- ARMAIO-RET <-- ### " << returnString << std::endl;
				logfile.flush();
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
				output[outputSize - 1] = '\0';
#ifdef DEBUG
				logfile << "### >_< ARMAIO-ERR >_< ### " << errstr << std::endl;
				logfile.flush();
#endif
			}
		}
	}
}

extern "C"
{
	void RVExtensionRegisterCallback(int(*callbackProc)(char const *name, char const *function, char const *data))
	{
		callbackPtr = callbackProc;
		#ifdef DEBUG
				std::time_t result;
				logfile << "### === ARMAIO-SET === ### " << "callbackPtr = " << callbackPtr << std::endl;
				logfile.flush();
		#endif
	}
}