/* cachebase.hpp
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

#ifndef SOURCE_DATABASE_DATACACHE_CACHEBASE_HPP_
#define SOURCE_DATABASE_DATACACHE_CACHEBASE_HPP_

#include "extbase.hpp"

class cache_base {

public:
	virtual ~cache_base() { };

	virtual int setData(std::string variableName, std::string variableValue) { return 0; };
	virtual int setData(ext_arguments &extArgument) { return 0; };

	virtual std::string getAsArmaString() { return "none"; };

	virtual bool isDirty() { return false; };
	virtual bool cleanDirty() { return true; };

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
