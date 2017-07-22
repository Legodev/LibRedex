/*
 * cachebase.hpp
 *
 *  Created on: 11.07.2017
 *      Author: lego
 */

#ifndef SOURCE_DATABASE_DATACACHE_CACHEBASE_HPP_
#define SOURCE_DATABASE_DATACACHE_CACHEBASE_HPP_

#include "extbase.hpp"

class cache_base {

public:
	virtual ~cache_base() { };

	virtual int setData(std::string variableName, std::string variableValue) { return 0; };
	virtual int setData(ext_arguments &extArgument) { return 0; };

	virtual std::string getAsArmaString() { return "none"; };

};
/*
	uuid = "0"
	classname =  "uninitializedobject"
	priority = 10001
	type = 3
	accesscode 	= ""
	locked 	= 0
	player_uuid = ""
	hitpoints = "[]"
	damage = 1
	fuel = 1
	fuelcargo = 0
	repaircargo = 0
	items = "[[],[],[],[],[]]"
	magazinesturret = "[]"
	variables = "[]"
	animationstate = "[]"
	textures = "[]"
	direction = 0
	positiontype = 0
	positionx = 0
	positiony = 0
	positionz = 0
	positionadvanced = "[]"
	reservedone = ""
	reservedtwo = ""
 */

#endif /* SOURCE_DATABASE_DATACACHE_CACHEBASE_HPP_ */
