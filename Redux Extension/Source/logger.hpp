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

#ifndef SOURCE_LOGGER_HPP
#define SOURCE_LOGGER_HPP

#include <chrono>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <fstream>

#include "constants.hpp"
#ifndef BUILD_TIME
	#define BUILD_TIME "undefined"
#endif

class Logger {
public:
	explicit Logger(std::ofstream& filestreamhandler)
			:filestreamhandler(filestreamhandler) {
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);
		std::stringstream ss;
		ss <<  "# "<< std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X -- ") << "libredex -- version " << DLLVERSIONSTRING << " -- build time -- " << BUILD_TIME << " #";
		for (int i; i < ss.tellp(); i++)
			filestreamhandler << "#";
		filestreamhandler << std::endl;
		filestreamhandler << ss.rdbuf() << std::endl;
		for (int i; i < ss.tellp(); i++)
			filestreamhandler << "#";
		filestreamhandler << std::endl;
	}
	void flush() {
		filestreamhandler.flush();
	}

	void close() {
		filestreamhandler.close();
	}

private:
	template<typename T> friend std::ofstream& operator<<(Logger&, T);

	std::ofstream& filestreamhandler;
};

template<typename T> std::ofstream& operator<<(Logger& log, T data) {
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	log.filestreamhandler << std::put_time(std::localtime(&in_time_t), "%X  ");

	log.filestreamhandler << data;
	return log.filestreamhandler;
}

#endif /* SOURCE_LOGGER_HPP_ */