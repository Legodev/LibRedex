/* mysql_binds.hpp
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

#ifndef SOURCE_MYSQL_BINDS_HPP_
#define SOURCE_MYSQL_BINDS_HPP_

#include <string>
#include <mysql.h>
#if LIBMYSQL_VERSION_ID > 80000
	#define my_bool bool
#endif

class mysql_binds {
public:
	mysql_binds(unsigned int length);
	~mysql_binds();

	void addNull();

	//void addData(bool value);
	void addData(signed char value);
	void addData(int value);
	void addData(unsigned int value);
	void addData(float value);
	void addData(std::string value);

	void setIncrement(unsigned int increment);

	MYSQL_BIND* mysql_bind;

private:
	unsigned int arraypos = 0;
	unsigned int maxpos = 0;
	my_bool* is_null;
	long unsigned int* length;

	void increment();
	void checkIncrement();

};

#endif //SOURCE_MYSQL_BINDS_HPP_
