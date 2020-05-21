/* handle_callback.cpp
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

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>

#include "constants.hpp"

#include "rest/handle_callback.hpp"

extern int (* callbackPtr)(char const* name, char const* function, char const* data);

#ifdef DEBUG
	#include "logger.hpp"
	extern Logger logfile;
#endif
void handle_callback(std::string url, std::string data)
{
	std::vector<std::string> urlsplit;
	boost::split(urlsplit, url, boost::is_any_of("/"));

#ifdef DEBUG
	logfile << "REST REQUEST URL" << std::endl;
	for(int i = 0; i < urlsplit.size(); i++)
	{
		logfile << "\t\t" << i << " " << urlsplit[i] << std::endl;
	}
	logfile << "REST REQUEST DATA" << std::endl;
	logfile << data << std::endl;
	logfile.flush();
#endif

	if((urlsplit.size() == 3 || urlsplit.size() == 4) && urlsplit[1] == "execute")
	{
		callbackPtr(PROTOCOL_CALLBACK_NAME_CALLIN, urlsplit[2].c_str(), data.c_str());
	}
}