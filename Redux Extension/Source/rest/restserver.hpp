/* restserver.hpp
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

#ifndef REDEX_REST_SERVER_HPP
#define REDEX_REST_SERVER_HPP

#include <algorithm>
#include <string>
#include <thread>
#include <map>
#include <tuple>
#include <boost/asio.hpp>

#include "extbase.hpp"
#include "http_connection.hpp"

class restserver: public ext_base {
public:
	restserver(EXT_FUNCTIONS &extFunctions);
	~restserver();

	std::string spawnHandler(std::string &extFunction, ext_arguments &extArgument);
	void terminateHandler();

private:
	bool serverinitialized = false;
	
	std::thread * serverThread = 0;
	//std::string address;
	boost::asio::ip::address address;
	unsigned short port;

	boost::asio::io_context serverReference{1};

	void serverFunction();
};

#endif //REDEX_REST_SERVER_HPP
