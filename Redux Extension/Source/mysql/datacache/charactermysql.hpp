/* charactermysql.hpp
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

#ifndef SOURCE_DATABASE_DATACACHE_CHARACTERMYSQL_HPP_
#define SOURCE_DATABASE_DATACACHE_CHARACTERMYSQL_HPP_

#include "extbase.hpp"
#include <mysql.h>
#if LIBMYSQL_VERSION_ID > 80000
	#define my_bool bool
#endif

#define characterCacheMaxElements 17

class character_mysql {
public:
	character_mysql();
	~character_mysql();

	int setNull(unsigned int arraypos);
	int setData(std::string variableName, std::string variableValue);
	int setData(unsigned int arraypos, std::string variableValue);
	int setData(ext_arguments &extArgument);

	bool isDirty() { return dirty; };
	bool cleanDirty() {
		dirty = false;
		dirtytablecharacter = false;
		dirtytableshareables = false;
		dirtytablepersistentvariables = false;
		return true;
	};

	std::string getAsArmaString();

	MYSQL_BIND mysql_bind[characterCacheMaxElements];
	my_bool is_null[characterCacheMaxElements];
	long unsigned int length [characterCacheMaxElements];

	bool dirtytablecharacter = false;
	bool dirtytableshareables = false;
	bool dirtytablepersistentvariables = false;

private:
	bool dirty = false;

	float direction = 0.0;
	signed char positiontype = 0;
	float positionx = 0.0;
	float positiony = 0.0;
	float positionz = 0.0;

	long unsigned int test = 32;

	void freeStrings();
};

#endif /* SOURCE_DATABASE_DATACACHE_CHARACTERMYSQL_HPP_ */
