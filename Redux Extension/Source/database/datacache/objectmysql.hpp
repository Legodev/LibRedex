/*
 * objectmysql.hpp
 *
 *  Created on: 11.07.2017
 *      Author: lego
 */

#ifndef SOURCE_DATABASE_DATACACHE_OBJECTMYSQL_HPP_
#define SOURCE_DATABASE_DATACACHE_OBJECTMYSQL_HPP_

#include "database/datacache/cachebase.hpp"
#include <mysql.h>

#define objectCacheMaxElements 27

class object_mysql: virtual public cache_base {
public:
	object_mysql();
	~object_mysql();

	int setData(std::string variableName, std::string variableValue);
	int setData(unsigned int arraypos, std::string variableValue);
	int setData(ext_arguments &extArgument);

	std::string getAsArmaString();

	MYSQL_BIND mysql_bind[objectCacheMaxElements];
	my_bool is_null[objectCacheMaxElements];

private:
	bool dirty = false;

	int priority = 10001;
	signed char type = 3;
	signed char locked 	= 0;
	float damage = 1.0;
	float fuel = 1.0;
	float fuelcargo = 0.0;
	float repaircargo = 0.0;
	float direction = 0.0;
	signed char positiontype = 0;
	float positionx = 0.0;
	float positiony = 0.0;
	float positionz = 0.0;

	void freeStrings();
};

#endif /* SOURCE_DATABASE_DATACACHE_OBJECTMYSQL_HPP_ */
