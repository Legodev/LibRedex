/* base.hpp
 *
 * Copyright 2016-2017 Desolation Redux
 *
 * Author: Legodev <legodevgit@mailbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef SOURCE_BASE_HPP_
#define SOURCE_BASE_HPP_

#include <string>
#include <map>
#include <queue>
#include <vector>
#include "constants.hpp"

class base_db_handler {
/*
protected:
	base_db_handler() { };
*/

public:
	virtual ~base_db_handler() { };

	virtual void connect(std::string hostname, std::string user, std::string password, std::string database,
			unsigned int port, bool whitelistonly, bool allowsteamapi, bool vaccheckban, unsigned int vacmaxcount,
			unsigned int vacignoredays, std::string worlduuid) {};
	virtual void disconnect() {};

	virtual std::string querydbversion() { return "none"; }
	virtual std::vector<std::vector<std::string> > verbosetest(std::string query) { return {{"none"}}; };

	virtual std::string loadPlayer(std::string nickname, std::string steamid) { return "none"; };
	virtual std::string loadAvChars(std::string playeruuid) { return "none"; }
	virtual std::string linkChars(std::string playeruuid, std::string variabuuid) { return "none"; }
	virtual std::string loadChar(std::string playeruuid) { return "none"; }
	virtual std::string createChar(std::string playeruuid, std::string animationstate, float direction,
			int positiontype, float positionx, float positiony, float positionz, std::string classname,
			std::string hitpoints, std::string variables, std::string persistentvariables, std::string textures,
			std::string inventoryuniform, std::string inventoryvest, std::string inventorybackpack, std::string uniform,
			std::string vest, std::string backpack, std::string headgear, std::string googles,
			std::string primaryweapon, std::string secondaryweapon, std::string handgun, std::string tools,
			std::string currentweapon) { return "none"; }
	virtual std::string updateChar(std::string charuuid, std::string animationstate, float direction, int positiontype,
			float positionx, float positiony, float positionz, std::string classname, std::string hitpoints,
			std::string variables, std::string persistentvariables, std::string textures, std::string inventoryuniform,
			std::string inventoryvest, std::string inventorybackpack, std::string uniform, std::string vest,
			std::string backpack, std::string headgear, std::string googles, std::string primaryweapon,
			std::string secondaryweapon, std::string handgun, std::string tools, std::string currentweapon) { return "none"; }
	virtual std::string locupdateChar(std::string charuuid, std::string animationstate, float direction,
			int positiontype, float positionx, float positiony, float positionz) { return "none"; }
	virtual std::string killChar(std::string charuuid, std::string attackeruuid, std::string type, std::string weapon,
			float distance) { return "none"; }

	virtual std::string loadObject(std::string objectuuid) { return "none"; }
	virtual std::string createObject(std::string classname, int priority, int visible, std::string accesscode,
			int locked, std::string player_uuid, std::string hitpoints, float damage, float fuel, float fuelcargo,
			float repaircargo, std::string items, std::string magazines, std::string weapons, std::string backpacks,
			std::string magazinesturret, std::string variables, std::string animationstate, std::string textures,
			float direction, int positiontype, float positionx, float positiony, float positionz) { return "none"; }
	virtual std::string createObject(std::string objectuuid, std::string classname, int priority, int visible,
			std::string accesscode, int locked, std::string player_uuid, std::string hitpoints, float damage,
			float fuel, float fuelcargo, float repaircargo, std::string items, std::string magazines,
			std::string weapons, std::string backpacks, std::string magazinesturret, std::string variables,
			std::string animationstate, std::string textures, float direction, int positiontype, float positionx,
			float positiony, float positionz) { return "none"; }
	virtual std::string updateObject(std::string objectuuid, std::string classname, int priority, int visible,
			std::string accesscode, int locked, std::string player_uuid, std::string hitpoints, float damage,
			float fuel, float fuelcargo, float repaircargo, std::string items, std::string magazines,
			std::string weapons, std::string backpacks, std::string magazinesturret, std::string variables,
			std::string animationstate, std::string textures, float direction, int positiontype, float positionx,
			float positiony, float positionz) { return "none"; }
	virtual std::string killObject(std::string objectuuid, std::string attackeruuid, std::string type,
			std::string weapon, float distance) { return "none"; }
	virtual std::vector<std::vector<std::string> > dumpObjects() { return {{"none"}}; };
};

#endif /* SOURCE_BASE_HPP_ */
