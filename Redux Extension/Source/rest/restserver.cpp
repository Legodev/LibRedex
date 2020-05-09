/* restserver.cpp
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

#include <boost/bind.hpp>
#include "rest/http_server.hpp"
#include "rest/restserver.hpp"

#include "constants.hpp"
#ifdef DEBUG
#include <fstream>
#include <iostream>
#include <sstream>
extern std::ofstream testfile;
#endif

restserver::restserver(EXT_FUNCTIONS &extFunctions) {
	extFunctions.insert(
			std::make_pair(
					std::string(PROTOCOL_REST_FUNCTION_STARTSERVER),
					std::make_tuple(
							boost::bind(&restserver::spawnHandler, this, _1, _2),
							SYNC_MAGIC)));

}

restserver::~restserver() {
	this->terminateHandler();
}

void restserver::terminateHandler() {
	std::cout << "TERMINATE RESTSERVER" << std::endl;
#ifdef DEBUG
	testfile << "TERMINATE RESTSERVER" << std::endl;
	testfile.flush();
#endif
	if (this->serverThread != 0) {
		if (this->serverinitialized) {
			this->serverReference.stop();
		}

		this->serverThread->join();
		delete this->serverThread;
	}
}

std::string restserver::spawnHandler(std::string &extFunction, ext_arguments &extArgument) {
	std::string address = extArgument.get<std::string>(PROTOCOL_REST_ARGUMENT_ADDRESS);
	this->address = boost::asio::ip::make_address(address);
	this->port = extArgument.get<unsigned short>(PROTOCOL_REST_ARGUMENT_PORT);

	if (!this->serverinitialized) {
		this->serverThread = new std::thread(&restserver::serverFunction, this);
		this->serverinitialized = true;
	} else {
		throw std::runtime_error("Server already spawned");
	}

	return "[\"" + std::string(PROTOCOL_MESSAGE_TYPE_MESSAGE) + "\", \"Server Started on " + address + ":" + std::to_string(this->port) + "\"]";
}

void restserver::serverFunction() {
	this->address = boost::asio::ip::make_address("0.0.0.0");
	this->port = 8000;

	boost::asio::ip::tcp::acceptor acceptor{this->serverReference, {this->address, this->port}};
	boost::asio::ip::tcp::socket socket{this->serverReference};

	http_server(acceptor, socket);

	this->serverReference.run();
}