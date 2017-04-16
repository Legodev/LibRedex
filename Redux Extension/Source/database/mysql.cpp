/* mysql.cpp
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

#include <exception>
#include <stdexcept>
#include <boost/format.hpp>
#include "database/mysql.hpp"
#include "utils/uuid.hpp"

mysql_db_handler::mysql_db_handler() {
	this->hostname = "";
	this->user = "";
	this->password = "";
	this->database = "";
	this->port = 0;
	this->socket = "";
	this->flag = 0;

	this->whitelistonly = false;
	this->allowsteamapi = false;
	this->vaccheckban = false;

	this->vacmaxcount = 0;
	this->vacignoredays = 0;

	this->worlduuid = "";

	connection = mysql_init(NULL);

	if (connection == NULL) {
		throw std::runtime_error("problem while initializing mysql_db_handler: " + std::string(mysql_error(connection)));
	}
}

mysql_db_handler::~mysql_db_handler() {
	mysql_close(connection);
	return;
}

void mysql_db_handler::connect(std::string hostname, std::string user, std::string password, std::string database,
		unsigned int port, bool whitelistonly, bool allowsteamapi, bool vaccheckban, unsigned int vacmaxcount,
		unsigned int vacignoredays, std::string worlduuid) {
	this->hostname = hostname;
	this->user = user;
	this->password = password;
	this->database = database;
	this->port = port;
//	this->socket = NULL;
//	this->flag = 0;

	this->whitelistonly = whitelistonly;
	this->allowsteamapi = allowsteamapi;
	this->vaccheckban = vaccheckban;

	this->vacmaxcount = vacmaxcount;
	this->vacignoredays = vacignoredays;

	this->worlduuid = worlduuid;

	mysql_options(connection, MYSQL_OPT_RECONNECT, &reconnect);

	if (mysql_real_connect(connection, this->hostname.c_str(),
			this->user.c_str(), this->password.c_str(), this->database.c_str(),
			this->port, this->socket.c_str(), this->flag) == NULL) {
		throw std::runtime_error(
				"connection problem while initializing mysql_db_handler: "
						+ std::string(mysql_error(connection)));
		mysql_close(connection);
	} else {
		printf("created connection \n");
	}

	return;
}

void mysql_db_handler::disconnect() {
	mysql_close(connection);
	return;
}

void mysql_db_handler::rawquery(std::string query) {
	printf("%s\n", query.c_str());

	if (mysql_real_query(connection, query.c_str(), query.size())) {
		throw std::runtime_error(
				"error while executing query: "
						+ query
						+ std::string(" ---- ERROR MESSAGE: ")
						+ std::string(mysql_error(connection)));
	}

	return;
}

void mysql_db_handler::rawquery(std::string query, MYSQL_RES **result) {
	printf("%s\n", query.c_str());

	if (mysql_real_query(connection, query.c_str(), query.size())) {
		throw std::runtime_error(
				"error while executing query: "
						+ query
						+ std::string(" ---- ERROR MESSAGE: ")
						+ std::string(mysql_error(connection)));
	}

	*result = mysql_store_result(connection);

	if (*result == NULL) {
		throw std::runtime_error(
				"error while getting the result: "
						+ query
						+ std::string(" ---- ERROR MESSAGE: ")
						+ std::string(mysql_error(connection)));
	}

	return;
}

std::string mysql_db_handler::querydbversion() {
	MYSQL_RES *result;
	MYSQL_ROW row;
	std::string version;

	this->rawquery("SELECT VERSION()", &result);

	row = mysql_fetch_row(result);
	//fieldcount = mysql_num_fields(*result);
	version = row[0];

	mysql_free_result(result);

	return version;
}

std::string mysql_db_handler::loadPlayer(std::string nickname, std::string steamid) {
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;

	// player info
	std::string playeruuid = "";
	std::string death_persistent_variables_uuid = "";
	std::string friendlist = "";
	std::string banned = "false";
	std::string banreason = "unknown";

	std::string queryplayerinfo =
	str(boost::format{"SELECT HEX(`actualplayer`.`uuid`), "
			"HEX(`player_on_world_has_death_persistent_variables`.`death_persistent_variables_uuid`), "
			"GROUP_CONCAT(`friendplayer`.`steamid` SEPARATOR '\", \"') AS friendlist, "
		    "(CASE WHEN (NOW() < `actualplayer`.`banenddate`) THEN \"true\" ELSE \"false\" END) AS BANNED, `actualplayer`.`banreason`"
			"FROM `player` actualplayer "
			"LEFT JOIN `player_on_world_has_death_persistent_variables` "
			" ON `actualplayer`.`uuid` = `player_on_world_has_death_persistent_variables`.`player_uuid` "
			" AND `player_on_world_has_death_persistent_variables`.`world_uuid` =  CAST(0x%s AS BINARY) "
			"LEFT JOIN `player_is_friend_with_player` "
			" ON `actualplayer`.`uuid` = `player_is_friend_with_player`.`player1_uuid` "
			" LEFT JOIN `player` friendplayer "
			" ON `player_is_friend_with_player`.`player2_uuid` = `friendplayer`.`uuid` "
			"WHERE `actualplayer`.`steamid` = \"%s\" "
			"GROUP BY `actualplayer`.`uuid`"} % worlduuid % steamid);

	char typearrayplayerinfo[] = {
			1, // HEX(`actualplayer`.`uuid`)
			1, // HEX(`player_on_world_has_death_persistent_variables`.`death_persistent_variables_uuid`)
			2, // GROUP_CONCAT(`player`.`steamid` SEPARATOR '\", \"') AS friendlist
			0, // (CASE WHEN (NOW() < `actualplayer`.`banenddate`) THEN "true" ELSE "false" END) AS BANNED`
			0  // `actualplayer`.`banreason
	};

	this->rawquery(queryplayerinfo, &result);

	rowcount = mysql_num_rows(result);

	// printf("rowcount = %d\n", (int)rowcount);

	if (rowcount > 0) {
		row = mysql_fetch_row(result);

		// should be always true, if not we have a huge bug
		if (row[0] != NULL) {
			playeruuid = row[0];
		}

		if (row[1] != NULL) {
			death_persistent_variables_uuid = row[1];
		}

		if (row[2] != NULL) {
			friendlist = "\"" ;
			friendlist += row[2];
			friendlist += "\"";
		}

		if (row[3] != NULL) {
			banned = row[3];
		}
		if (row[4] != NULL) {
			banreason = row[4];
		}
	}

	mysql_free_result(result);

	if (banned == "false") {
		if (playeruuid == "") {
			playeruuid = orderedUUID();
			queryplayerinfo =
				str(boost::format{"INSERT INTO `player` (`uuid`, `steamid`, `battleyeid`, "
									"`firstlogin`, `firstnick`, `lastlogin`, `lastnick`, "
									"`bancount`, `banreason`, `banbegindate`, `banenddate`) "
									"VALUES (CAST(0x%s AS BINARY), \"%s\", \"unused\", NOW(), "
									"\"%s\", NOW(), \"%s\", '0', NULL, NULL, NULL)"}
									% playeruuid % steamid % nickname % nickname);
		} else {
			queryplayerinfo =
						str(boost::format{"UPDATE `player` SET `lastlogin` = NOW(), `lastnick` = \"%s\" \
											WHERE `player`.`uuid` = CAST(0x%s AS BINARY)"} % nickname % playeruuid);
		}

		this->rawquery(queryplayerinfo);

		if (whitelistonly) {
			std::string querywhitelist =
				str(boost::format{"SELECT * FROM `whitelist` "
						"WHERE `whitelist`.`world_uuid` = CAST(0x%s AS BINARY) "
						" AND `whitelist`.`player_uuid` = CAST(0x%s AS BINARY) "} % worlduuid % playeruuid);

				this->rawquery(querywhitelist, &result);

				rowcount = mysql_num_rows(result);

				if (rowcount < 1) {
					banned = "true";
					banreason = "not whitelisted";
				}

				mysql_free_result(result);
		}
	}

	return "[\"" + playeruuid + "\",\"" + death_persistent_variables_uuid + "\",[" + friendlist + "],[" + banned + ",\"" + banreason +"\"]]";
}

std::string mysql_db_handler::loadAvChars(std::string playeruuid) {
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	bool printcommaone = false;
	bool printcommatwo = false;

	// char info
	std::string charinfo = "";

	std::string querycharinfo =
			str(
					boost::format {
							"SELECT HEX(`death_persistent_variables`.`uuid`), "
									"`death_persistent_variables`.`persistentvariables`, `world`.`name`, `world`.`map` "
									"FROM `world_is_linked_to_world` "
									"INNER JOIN `player_on_world_has_death_persistent_variables` "
									" ON `world_is_linked_to_world`.`world_uuid2` = `player_on_world_has_death_persistent_variables`.`world_uuid` "
									"INNER JOIN `death_persistent_variables` "
									" ON `death_persistent_variables`.`uuid` = `player_on_world_has_death_persistent_variables`.`death_persistent_variables_uuid` "
									"INNER JOIN `world` "
									" ON `world`.`uuid` = `world_is_linked_to_world`.`world_uuid2` "
									"WHERE `world_is_linked_to_world`.`world_uuid1` = CAST(0x%s AS BINARY) "
									" AND `player_on_world_has_death_persistent_variables`.`player_uuid` = CAST(0x%s AS BINARY)" }
							% worlduuid % playeruuid);

	char typearray[] = {
			1, // HEX(`death_persistent_variables`.`uuid`)
			0, // `death_persistent_variables`.`persistentvariables`
			1, // `world`.`name`
			1  // `world`.`map`
			};

	this->rawquery(querycharinfo, &result);

	fieldcount = mysql_num_fields(result);
	rowcount = mysql_num_rows(result);

	// printf("fieldcount = %d\n", (int) fieldcount);
	// printf("rowcount = %d\n", (int) rowcount);

	charinfo = "[";

	for (int rowpos = 0; rowpos < rowcount; rowpos++) {
		row = mysql_fetch_row(result);

		charinfo += "[";
		if (printcommaone) {
			charinfo += ",";
		}

		for (int fieldpos = 0; fieldpos < fieldcount; fieldpos++) {
			if (printcommatwo) {
				charinfo += ",";
			}
			switch (typearray[fieldpos]) {
			case 1:
				charinfo += "\"";
				break;
			case 2:
				charinfo += "[";
				if (row[fieldpos] != NULL)
					charinfo += "\"";
				break;
			default:
				charinfo += "";
			}

			if (row[fieldpos] != NULL) {
				charinfo += row[fieldpos];
			} else {
				charinfo += "";
			}

			switch (typearray[fieldpos]) {
			case 1:
				charinfo += "\"";
				break;
			case 2:
				if (row[fieldpos] != NULL)
					charinfo += "\"";
				charinfo += "]";
				break;
			}
			printcommatwo = true;
		}
		charinfo += "]";
		printcommaone = true;
	}
	charinfo += "]";

	return charinfo;
}

std::string mysql_db_handler::linkChars(std::string playeruuid, std::string variabuuid) {
	std::string query = str(
			boost::format { "INSERT INTO `player_on_world_has_death_persistent_variables` "
					"(`player_uuid`, `world_uuid`, `death_persistent_variables_uuid`) "
					"VALUES (0x%s AS BINARY), CAST(0x%s AS BINARY), CAST(0x%s AS BINARY))"
					} % playeruuid % worlduuid % variabuuid);


	this->rawquery(query);

	return playeruuid;
}

std::string mysql_db_handler::loadChar(std::string playeruuid) {
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	bool printcomma = false;

	// char info
	std::string charinfo = "";

	std::string querycharinfo = str(boost::format {
		    "SELECT HEX(`character`.`uuid`), `animationstate`, "
			"`direction`, `positiontype`, `positionx`, `positiony`, "
			"`positionz`, `classname`, `hitpoints`, `variables`, "
			"`persistentvariables`, `textures`, `inventoryuniform`, "
			"`inventoryvest`, `inventorybackpack`, `uniform`, `vest`, "
			"`backpack`, `headgear`, `googles`, `primaryweapon`, "
			"`secondaryweapon`, `handgun`, `tools`, `currentweapon` "
			"FROM `player_on_world_has_character` "
			"INNER JOIN `character`  "
			" ON `player_on_world_has_character`.`character_uuid` = `character`.`uuid` "
			"INNER JOIN `charactershareables` "
			" ON `character`.`charactershareables_uuid` = `charactershareables`.`uuid` "
			"INNER JOIN `death_persistent_variables` "
			" ON `charactershareables`.`death_persistent_variables_uuid` = `death_persistent_variables`.`uuid` "
			"WHERE `player_on_world_has_character`.`player_uuid` = CAST(0x%s AS BINARY) "
			" AND `player_on_world_has_character`.`world_uuid` =  CAST(0x%s AS BINARY) "
			" AND `player_on_world_has_character`.`killinfo_uuid` IS NULL" } % playeruuid % worlduuid);

	char typearray[] = {
			1, // HEX(`character`.`uuid`)
			1, // `animationstate`
			0, // `direction`
			0, // `positiontype`
			0, // `positionx`
			0, // `positiony`
			0, // `positionz`
			1, // `classname`
			0, // `hitpoints`
			0, // `variables`
			0, // `persistentvariables`
			0, // `textures`
			0, // `inventoryuniform`
			0, // `inventoryvest`
			0, // `inventorybackpack`
			1, // `uniform`
			1, // `vest`
			1, // `backpack`
			1, // `headgear`
			1, // `googles`
			0, // `primaryweapon`
			0, // `secondaryweapon`
			0, // `handgun`
			0, // `tools`
			1  // `currentweapon`
	};

	this->rawquery(querycharinfo, &result);

	fieldcount = mysql_num_fields(result);
	rowcount = mysql_num_rows(result);

	// printf("rowcount = %d\n", (int) rowcount);

	charinfo = "[";
	if (rowcount > 0) {
		row = mysql_fetch_row(result);
		for (int fieldpos = 0; fieldpos < fieldcount; fieldpos++) {
			if (printcomma) {
				charinfo += ",";
			}
			switch (typearray[fieldpos]) {
			case 1:
				charinfo += "\"";
				break;
			case 2:
				charinfo += "[";
				if (row[fieldpos] != NULL)
					charinfo += "\"";
				break;
			default:
				charinfo += "";
			}

			if (row[fieldpos] != NULL) {
				charinfo += row[fieldpos];
			} else {
				charinfo += "";
			}

			switch (typearray[fieldpos]) {
			case 1:
				charinfo += "\"";
				break;
			case 2:
				if (row[fieldpos] != NULL)
					charinfo += "\"";
				charinfo += "]";
				break;
			}
			printcomma = true;
		}
	}
	charinfo += "]";

	mysql_free_result(result);

	return charinfo;
}


std::string mysql_db_handler::createChar(std::string playeruuid, std::string animationstate, float direction,
		int positiontype, float positionx, float positiony, float positionz, std::string classname,
		std::string hitpoints, std::string variables, std::string persistentvariables, std::string textures,
		std::string inventoryuniform, std::string inventoryvest, std::string inventorybackpack, std::string uniform,
		std::string vest, std::string backpack, std::string headgear, std::string googles, std::string primaryweapon,
		std::string secondaryweapon, std::string handgun, std::string tools, std::string currentweapon) {
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;

	// char info
	std::string charuuid = "";
	std::string death_persistent_variables_uuid = "";
	std::string shareable_variables_uuid = "";

	/* for testing purpose, check if there is an existing character, if yes return its uuid */
	std::string query = str(boost::format { "SELECT HEX(`character_uuid`) "
			"FROM `player_on_world_has_character` "
			"WHERE `player_on_world_has_character`.`player_uuid` = CAST(0x%s AS BINARY) "
			"AND `player_on_world_has_character`.`world_uuid` =  CAST(0x%s AS BINARY) "
			"AND `player_on_world_has_character`.`killinfo_uuid` IS NULL" } % playeruuid % worlduuid);



	this->rawquery(query, &result);

	rowcount = mysql_num_rows(result);

	// printf("rowcount = %d\n", (int) rowcount);

	if (rowcount > 0) {
		row = mysql_fetch_row(result);

		if (row[0] != NULL) {
			charuuid = row[0];
		}
	}

	mysql_free_result(result);

	if (charuuid == "") {
		/* get the uuid for the Death Persistent Variables */
		query = str(boost::format { "SELECT HEX(`death_persistent_variables_uuid`) "
				"FROM `player_on_world_has_death_persistent_variables` "
				"WHERE `player_uuid` = CAST(0x%s AS BINARY) "
				"AND `world_uuid` = CAST(0x%s AS BINARY)" } % playeruuid % worlduuid);


		this->rawquery(query, &result);

		rowcount = mysql_num_rows(result);

		// printf("rowcount = %d\n", (int) rowcount);

		if (rowcount > 0) {
			row = mysql_fetch_row(result);

			if (row[0] != NULL) {
				death_persistent_variables_uuid = row[0];
			}
		}

		mysql_free_result(result);

		/* if there are no Death Persistent Variables create some*/
		if (death_persistent_variables_uuid == "") {
			death_persistent_variables_uuid = orderedUUID();

			query = str(
					boost::format { "INSERT INTO `death_persistent_variables` (`uuid`, `persistentvariables`) "
							"VALUES (CAST(0x%s AS BINARY), \"%s\"); " } % death_persistent_variables_uuid
							% persistentvariables);


			this->rawquery(query);

			query = str(boost::format { "INSERT INTO `player_on_world_has_death_persistent_variables` "
					"(`player_uuid`, `world_uuid`, `death_persistent_variables_uuid`) "
					"VALUES (CAST(0x%s AS BINARY), "
					"CAST(0x%s AS BINARY), "
					"CAST(0x%s AS BINARY));" } % playeruuid % worlduuid % death_persistent_variables_uuid);


			this->rawquery(query);
		}

		/* get alive linked character shareables */
		query = str(boost::format { "SELECT HEX(`charactershareables`.`uuid`) "
				"FROM `player_on_world_has_character` "
				"INNER JOIN `character` "
				"ON `player_on_world_has_character`.`character_uuid` = `character`.`uuid` "
				"INNER JOIN `charactershareables` "
				"ON `character`.`charactershareables_uuid` = `charactershareables`.`uuid` "
				"WHERE `player_on_world_has_character`.`player_uuid` = CAST(0x%s AS BINARY) "
				"AND `player_on_world_has_character`.`killinfo_uuid` IS NULL "
				"AND `player_on_world_has_character`.`world_uuid` IN  ( "
				"SELECT `world_uuid` "
				"FROM `player_on_world_has_death_persistent_variables` "
				"WHERE `death_persistent_variables_uuid` = CAST(0x%s AS BINARY) "
				") LIMIT 1" } % playeruuid % death_persistent_variables_uuid);



		this->rawquery(query, &result);

		rowcount = mysql_num_rows(result);

		// printf("rowcount = %d\n", (int) rowcount);

		if (rowcount > 0) {
			row = mysql_fetch_row(result);

			if (row[0] != NULL) {
				shareable_variables_uuid = row[0];
			}
		}

		mysql_free_result(result);

		/* if there are no Death Persistent Variables create some*/
		if (shareable_variables_uuid == "") {
			shareable_variables_uuid = orderedUUID();

			query = str(
					boost::format { "INSERT INTO `charactershareables` (`uuid`, `classname`, `hitpoints`, "
							"`variables`, `death_persistent_variables_uuid`, `textures`, "
							"`inventoryuniform`, `inventoryvest`, `inventorybackpack`, "
							" `uniform`, `vest`, `backpack`, "
							"`headgear`, `googles`, `primaryweapon`, "
							"`secondaryweapon`, `handgun`, `tools`, "
							"`currentweapon`) "
							"VALUES (CAST(0x%s AS BINARY), "
							"\"%s\", \"%s\", "
							"\"%s\", CAST(0x%s AS BINARY), "
							"\"%s\", \"%s\", "
							"\"%s\", \"%s\", "
							"\"%s\", \"%s\", \"%s\", "
							"\"%s\", \"%s\", \"%s\", "
							"\"%s\", \"%s\", \"%s\", "
							"\"%s\")" } % shareable_variables_uuid % classname % hitpoints % variables
							% death_persistent_variables_uuid % textures % inventoryuniform % inventoryvest
							% inventorybackpack % uniform % vest % backpack % headgear % googles % primaryweapon
							% secondaryweapon % handgun % tools % currentweapon);



			this->rawquery(query);
		}

		/* add character world specific data */
		/* add character to world */
		charuuid = orderedUUID();

		query = str(
				boost::format { "INSERT INTO `character` (`uuid`, `animationstate`, `direction`, "
						"`positiontype`, `positionx`, `positiony`, "
						"`positionz`, `charactershareables_uuid`) "
						"VALUES (CAST(0x%s AS BINARY), "
						"\"%s\", %s, "
						"%s, %s, %s, "
						"%s, CAST(0x%s AS BINARY)); " } % charuuid % animationstate % direction % positiontype
						% positionx % positiony % positionz % shareable_variables_uuid);



		this->rawquery(query);
		query = str(boost::format { "INSERT INTO `player_on_world_has_character` (`player_uuid`, `world_uuid`, "
				"`character_uuid`, `killinfo_uuid`) "
				"VALUES (CAST(0x%s AS BINARY), "
				"CAST(0x%s AS BINARY), "
				"CAST(0x%s AS BINARY), "
				"NULL)" } % playeruuid % worlduuid % charuuid);



		this->rawquery(query);

	}

	return charuuid;
}

std::string mysql_db_handler::updateChar(std::string charuuid, std::string animationstate, float direction, int positiontype,
		float positionx, float positiony, float positionz, std::string classname, std::string hitpoints,
		std::string variables, std::string persistentvariables, std::string textures, std::string inventoryuniform,
		std::string inventoryvest, std::string inventorybackpack, std::string uniform, std::string vest,
		std::string backpack, std::string headgear, std::string googles, std::string primaryweapon,
		std::string secondaryweapon, std::string handgun, std::string tools, std::string currentweapon) {

	std::string query = str(
			boost::format { "UPDATE `characterview` "
					"SET `animationstate` = \"%s\", "
					"`direction` = %s, "
					"`positiontype` = %s, "
					"`positionx` = %s, "
					"`positiony` = %s, "
					"`positionz` = %s "
					" WHERE `characterview`.`uuid` = CAST(0x%s AS BINARY);" } % animationstate % direction
					% positiontype % positionx % positiony % positionz % charuuid);


	this->rawquery(query);

	query = str(
			boost::format { "UPDATE `characterview` "
					"SET `classname` = \"%s\", "
					"`hitpoints` = \"%s\", "
					"`variables` = \"%s\", "
					"`textures` = \"%s\", "
					"`inventoryuniform` = \"%s\", "
					"`inventoryvest` = \"%s\", "
					"`inventorybackpack` = \"%s\", "
					"`uniform` = \"%s\", "
					"`vest` = \"%s\", "
					"`backpack` = \"%s\", "
					"`headgear` = \"%s\", "
					"`googles` = \"%s\", "
					"`primaryweapon` = \"%s\", "
					"`secondaryweapon` = \"%s\", "
					"`handgun` = \"%s\", "
					"`tools` = \"%s\", "
					"`currentweapon` = \"%s\" "
					" WHERE `characterview`.`uuid` = CAST(0x%s AS BINARY);" } % classname % hitpoints % variables
					% textures % inventoryuniform % inventoryvest % inventorybackpack % uniform % vest % backpack
					% headgear % googles % primaryweapon % secondaryweapon % handgun % tools % currentweapon
					% charuuid);


	this->rawquery(query);

	query = str(boost::format { "UPDATE `characterview` "
			"SET `persistentvariables` = \"%s\" "
			"WHERE `characterview`.`uuid` = CAST(0x%s AS BINARY);" } % persistentvariables % charuuid);


	this->rawquery(query);

	return charuuid;
}

std::string mysql_db_handler::locupdateChar(std::string charuuid, std::string animationstate, float direction,
		int positiontype, float positionx, float positiony, float positionz) {
	std::string query = str(
			boost::format { "UPDATE `characterview` "
					"SET `animationstate` = \"%s\", "
					"`direction` = %s, "
					"`positiontype` = %s, "
					"`positionx` = %s, "
					"`positiony` = %s, "
					"`positionz` = %s "
					" WHERE `characterview`.`uuid` = CAST(0x%s AS BINARY);" } % animationstate % direction
					% positiontype % positionx % positiony % positionz % charuuid);


	this->rawquery(query);

	return charuuid;
}

std::string mysql_db_handler::killChar(std::string charuuid, std::string attackeruuid, std::string type,
		std::string weapon, float distance) {
	std::string killuuid = orderedUUID();

	if (attackeruuid == "") {
		attackeruuid = "NULL";
	} else {
		attackeruuid = "CAST(0x" + attackeruuid + " AS BINARY)";
	}

	if (type == "") {
		type = "NULL";
	} else {
		type = "'" + type + "'";
	}

	if (weapon == "") {
		weapon = "NULL";
	} else {
		weapon = "'" + weapon + "'";
	}

	std::string query = str(
			boost::format {"INSERT INTO `killinfo` "
							"(`uuid`, `date`, `attacker_uuid`, `type`, `weapon`, `distance`) "
							"VALUES (CAST(0x%s AS BINARY), CURRENT_TIMESTAMP, %s, %s, %s, %s);" }
							% killuuid % attackeruuid % type % weapon % distance);


	this->rawquery(query);

	query = str(
				boost::format {"UPDATE `player_on_world_has_character` "
								"SET `killinfo_uuid` = CAST(0x%s AS BINARY) "
								"WHERE `player_on_world_has_character`.`character_uuid` IN "
								"(SELECT `character`.`uuid` FROM `character` WHERE `charactershareables_uuid` = "
								"(SELECT `character`.`charactershareables_uuid` FROM `character` WHERE `uuid` = CAST(0x%s AS BINARY)));"}
								% killuuid % charuuid);


		this->rawquery(query);

	return killuuid;
}

std::string mysql_db_handler::loadObject(std::string objectuuid) {
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	bool printcommatwo = false;

	// char info
	std::string objectinfo = "";

	std::string query = str(boost::format {
			"SELECT HEX(`object`.`uuid`), HEX(`world_has_objects`.`parentobject_uuid`), "
					"`object`.`classname`, `object`.`priority`, `object`.`visible`, `object`.`accesscode`, "
					"`object`.`locked`, HEX(`object`.`player_uuid`), `object`.`hitpoints`, `object`.`damage`, "
					"`object`.`fuel`, `object`.`fuelcargo`, `object`.`repaircargo`, `object`.`items`, "
					"`object`.`magazinesturret`, "
					"`object`.`variables`, `object`.`animationstate`, `object`.`textures`, `object`.`direction`, "
					"`object`.`positiontype`, `object`.`positionx`, `object`.`positiony`, `object`.`positionz`, "
					"`object`.`positionadvanced`, `object`.`reservedone`, `object`.`reservedtwo`, "
					"GROUP_CONCAT(`player`.`steamid` SEPARATOR '\", \"') AS friendlist "
					"FROM `world_has_objects` "
					"INNER JOIN `object` "
					" ON `world_has_objects`.`object_uuid` = `object`.`uuid` "
					"LEFT JOIN `player_is_friend_with_player` "
					" ON `object`.`player_uuid` = `player_is_friend_with_player`.`player1_uuid` "
					"LEFT JOIN `player` "
					" ON `player_is_friend_with_player`.`player2_uuid` = `player`.`uuid` "
					"WHERE `world_has_objects`.`object_uuid` = CAST(0x%s AS BINARY) "
					"GROUP BY `object`.`uuid` "
					"ORDER BY `object`.priority ASC, `world_has_objects`.`parentobject_uuid` ASC" } % objectuuid);

	char typearray[] = { 1, // HEX(`object`.`uuid`)
			1, // HEX(`world_has_objects`.`parentobject_uuid`)
			1, // `object`.`classname`
			0, // `object`.`priority`
			0, // `object`.`visible`
			1, // `object`.`accesscode`
			0, // `object`.`locked`
			1, // HEX(`object`.`player_uuid`)
			0, // `object`.`hitpoints`
			0, // `object`.`damage`
			0, // `object`.`fuel`
			0, // `object`.`fuelcargo`
			0, // `object`.`repaircargo`
			0, // `object`.`items`
			0, // `object`.`magazinesturret`
			0, // `object`.`variables`
			0, // `object`.`animationstate`
			0, // `object`.`textures`
			0, // `object`.`direction`
			0, // `object`.`positiontype`
			0, // `object`.`positionx`
			0, // `object`.`positiony`
			0, // `object`.`positionz`
			0, // `object`.`positionadvanced`
			0, // `object`.`reservedone`
			0, // `object`.`reservedtwo`
			2  // GROUP_CONCAT(`player`.`steamid` SEPARATOR '\", \"') AS friendlist
			};



	//this->rawquery("SELECT HEX(uuid) FROM `world` WHERE uuid = CAST(0x11e66ac83e75277882c510bf48883ace AS BINARY)", &result);
	this->rawquery(query, &result);

	fieldcount = mysql_num_fields(result);
	rowcount = mysql_num_rows(result);

	// printf("fieldcount = %d\n", (int) fieldcount);
	// printf("rowcount = %d\n", (int) rowcount);

	objectinfo = "[";
	if (rowcount > 0) {
		row = mysql_fetch_row(result);

		for (int fieldpos = 0; fieldpos < fieldcount; fieldpos++) {
			if (printcommatwo) {
				objectinfo += ",";
			}
			switch (typearray[fieldpos]) {
			case 1:
				objectinfo += "\"";
				break;
			case 2:
				objectinfo += "[";
				if (row[fieldpos] != NULL)
					objectinfo += "\"";
				break;
			default:
				objectinfo += "";
			}

			if (row[fieldpos] != NULL) {
				objectinfo += row[fieldpos];
			} else {
				objectinfo += "";
			}

			switch (typearray[fieldpos]) {
			case 1:
				objectinfo += "\"";
				break;
			case 2:
				if (row[fieldpos] != NULL)
					objectinfo += "\"";
				objectinfo += "]";
				break;
			}
			printcommatwo = true;
		}
	}
	objectinfo += "]";

	mysql_free_result(result);

	return objectinfo;
}

std::string mysql_db_handler::createObject(std::string classname,
		int priority, int visible, std::string accesscode, int locked,
		std::string player_uuid, std::string hitpoints, float damage,
		float fuel, float fuelcargo, float repaircargo, std::string items,
		std::string magazinesturret, std::string variables,
		std::string animationstate, std::string textures, float direction,
		int positiontype, float positionx, float positiony, float positionz,
		std::string positionadvanced, std::string reservedone, std::string reservedtwo) {

	std::string objectuuid = orderedUUID();

	this->createObject(objectuuid, classname, priority, visible, accesscode,
			locked, player_uuid, hitpoints, damage, fuel, fuelcargo,
			repaircargo, items, magazinesturret, variables, animationstate,
			textures, direction, positiontype, positionx, positiony, positionz,
			positionadvanced, reservedone, reservedtwo);

	return objectuuid;
}

std::string mysql_db_handler::createObject(std::string objectuuid, std::string classname,
		int priority, int visible, std::string accesscode, int locked,
		std::string player_uuid, std::string hitpoints, float damage,
		float fuel, float fuelcargo, float repaircargo, std::string items,
		std::string magazinesturret, std::string variables,
		std::string animationstate, std::string textures, float direction,
		int positiontype, float positionx, float positiony, float positionz,
		std::string positionadvanced, std::string reservedone, std::string reservedtwo) {

	std::string query;

	if (player_uuid == "") {
		query = str(
				boost::format { "INSERT INTO `object` (`uuid`, `classname`, `priority`, `timelastused`, "
						"`timecreated`, `visible`, `accesscode`, `locked`, `player_uuid`, `hitpoints`, "
						"`damage`, `fuel`, `fuelcargo`, `repaircargo`, `items`, `magazinesturret`, "
						"`variables`, `animationstate`, `textures`, `direction`, `positiontype`, "
						"`positionx`, `positiony`, `positionz`, "
						"`positionadvanced`, `reservedone`, `reservedtwo`) "
						"VALUES (CAST(0x%s AS BINARY), \"%s\", %s, now(), "
						"now(), %s, \"%s\", %s, NULL, \"%s\", "
						"%s, %s, %s, %s, \"%s\", \"%s\", \"%s\", "
						"\"%s\", \"%s\", %s, %s, %s, %s, %s, "
						"\"%s\", \"%s\", \"%s\")" } % objectuuid % classname % priority % visible % accesscode % locked
						% hitpoints % damage % fuel % fuelcargo % repaircargo % items
						% magazinesturret % variables % animationstate % textures % direction % positiontype % positionx
						% positiony % positionz % positionadvanced % reservedone % reservedtwo);
	} else {
		query = str(
				boost::format { "INSERT INTO `object` (`uuid`, `classname`, `priority`, `timelastused`, "
						"`timecreated`, `visible`, `accesscode`, `locked`, `player_uuid`, `hitpoints`, "
						"`damage`, `fuel`, `fuelcargo`, `repaircargo`, `items`, `magazinesturret`, "
						"`variables`, `animationstate`, `textures`, `direction`, `positiontype`, "
						"`positionx`, `positiony`, `positionz`, "
						"`positionadvanced`, `reservedone`, `reservedtwo`) "
						"VALUES (CAST(0x%s AS BINARY), \"%s\", %s, now(), "
						"now(), %s, \"%s\", %s, CAST(0x%s AS BINARY), \"%s\", "
						"%s, %s, %s, %s, \"%s\", \"%s\", \"%s\", "
						"\"%s\", \"%s\", %s, %s, %s, %s, %s, "
						"\"%s\", \"%s\", \"%s\")" } % objectuuid % classname % priority % visible % accesscode % locked	% player_uuid
						% hitpoints % damage % fuel % fuelcargo % repaircargo % items
						% magazinesturret % variables % animationstate % textures % direction % positiontype % positionx
						% positiony % positionz	% positionadvanced % reservedone % reservedtwo);
	}



	this->rawquery(query);

	query = str(boost::format { "INSERT INTO `world_has_objects` (`world_uuid`, `object_uuid`) "
			"VALUES (CAST(0x%s AS BINARY), CAST(0x%s AS BINARY))" } % worlduuid % objectuuid);


	this->rawquery(query);

	return objectuuid;
}

std::string mysql_db_handler::updateObject(std::string objectuuid, std::string classname,
		int priority, int visible, std::string accesscode, int locked,
		std::string player_uuid, std::string hitpoints, float damage,
		float fuel, float fuelcargo, float repaircargo, std::string items,
		std::string magazinesturret, std::string variables,
		std::string animationstate, std::string textures, float direction,
		int positiontype, float positionx, float positiony, float positionz,
		std::string positionadvanced, std::string reservedone, std::string reservedtwo) {

	std::string query;

		if (player_uuid == "") {
			query = str(
					boost::format { "UPDATE `object` "
									"SET `classname` = \"%s\", "
									"    `priority` = %s, "
									"    `visible` = \"%s\","
									"    `accesscode` = \"%s\", "
									"    `locked` = \"%s\", "
									"    `player_uuid` = NULL, "
									"    `hitpoints` = \"%s\", "
									"    `damage` = %s, "
									"    `fuel` = %s, "
									"    `fuelcargo` = %s, "
									"    `repaircargo` = %s, "
									"    `items` = \"%s\", "
									"    `magazinesturret` = \"%s\", "
									"    `variables` = \"%s\", "
									"    `animationstate` = \"%s\", "
									"    `textures` = \"%s\", "
									"    `direction` = %s, "
									"    `positiontype` = %s, "
									"    `positionx` = %s, "
									"    `positiony` = %s, "
									"    `positionz` = %s, "
									"    `positionadvanced` = \"%s\", "
									"    `reservedone` = \"%s\", "
									"    `reservedtwo` = \"%s\" "
									"WHERE `object`.`uuid` = CAST(0x%s AS BINARY);" }
									% classname % priority % visible % accesscode % locked
									% hitpoints % damage % fuel % fuelcargo % repaircargo % items
									% magazinesturret % variables % animationstate % textures % direction
									% positiontype % positionx % positiony % positionz
									% positionadvanced % reservedone % reservedtwo % objectuuid);
		} else {
			query = str(
					boost::format { "UPDATE `object` "
									"SET `classname` = \"%s\", "
									"    `priority` = %s, "
									"    `visible` = \"%s\","
									"    `accesscode` = \"%s\", "
									"    `locked` = \"%s\", "
									"    `player_uuid` = CAST(0x%s AS BINARY), "
									"    `hitpoints` = \"%s\", "
									"    `damage` = %s, "
									"    `fuel` = %s, "
									"    `fuelcargo` = %s, "
									"    `repaircargo` = %s, "
									"    `items` = \"%s\", "
									"    `magazinesturret` = \"%s\", "
									"    `variables` = \"%s\", "
									"    `animationstate` = \"%s\", "
									"    `textures` = \"%s\", "
									"    `direction` = %s, "
									"    `positiontype` = %s, "
									"    `positionx` = %s, "
									"    `positiony` = %s, "
									"    `positionz` = %s, "
									"    `positionadvanced` = \"%s\", "
									"    `reservedone` = \"%s\", "
									"    `reservedtwo` = \"%s\" "
									"WHERE `object`.`uuid` = CAST(0x%s AS BINARY);" }
									% classname % priority % visible % accesscode % locked
									% player_uuid % hitpoints % damage % fuel % fuelcargo % repaircargo % items
									% magazinesturret % variables % animationstate % textures % direction
									% positiontype % positionx % positiony % positionz
									% positionadvanced % reservedone % reservedtwo % objectuuid);
		}



		this->rawquery(query);

		return objectuuid;
}

std::string mysql_db_handler::killObject(std::string objectuuid, std::string attackeruuid, std::string type,
		std::string weapon, float distance) {
	std::string killuuid = orderedUUID();

	if (attackeruuid == "") {
		attackeruuid = "NULL";
	} else {
		attackeruuid = "CAST(0x" + attackeruuid + " AS BINARY)";
	}

	if (type == "") {
		type = "NULL";
	} else {
		type = "'" + type + "'";
	}

	if (weapon == "") {
		weapon = "NULL";
	} else {
		weapon = "'" + weapon + "'";
	}

	std::string query = str(
			boost::format {"INSERT INTO `killinfo` "
							"(`uuid`, `date`, `attacker_uuid`, `type`, `weapon`, `distance`) "
							"VALUES (CAST(0x%s AS BINARY), CURRENT_TIMESTAMP, %s, %s, %s, %s);" }
							% killuuid % attackeruuid % type % weapon % distance);


	this->rawquery(query);

	query = str(
				boost::format {"UPDATE `world_has_objects` SET `killinfo_uuid` = CAST(0x%s AS BINARY) "
								"WHERE `world_has_objects`.`object_uuid` = CAST(0x%s AS BINARY);"}
								% killuuid % objectuuid);


		this->rawquery(query);

	return killuuid;
}

std::vector< std::vector<std::string> > mysql_db_handler::dumpObjects() {
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	std::string query =
	str(boost::format{"SELECT HEX(`object`.`uuid`), HEX(`world_has_objects`.`parentobject_uuid`), "
						"`object`.`classname`, `object`.`priority`, `object`.`visible`, `object`.`accesscode`, "
						"`object`.`locked`, HEX(`object`.`player_uuid`), `object`.`hitpoints`, `object`.`damage`, "
						"`object`.`fuel`, `object`.`fuelcargo`, `object`.`repaircargo`, `object`.`items`, "
						"`object`.`magazinesturret`, "
						"`object`.`variables`, `object`.`animationstate`, `object`.`textures`, `object`.`direction`, "
						"`object`.`positiontype`, `object`.`positionx`, `object`.`positiony`, `object`.`positionz`, "
						"`object`.`positionadvanced`, `object`.`reservedone`, `object`.`reservedtwo`, "
						"GROUP_CONCAT(`player`.`steamid` SEPARATOR '\", \"') AS friendlist "
						"FROM `world_has_objects` "
						"INNER JOIN `object` "
						" ON `world_has_objects`.`object_uuid` = `object`.`uuid` "
						"LEFT JOIN `player_is_friend_with_player` "
						" ON `object`.`player_uuid` = `player_is_friend_with_player`.`player1_uuid` "
						"LEFT JOIN `player` "
						" ON `player_is_friend_with_player`.`player2_uuid` = `player`.`uuid` "
						"WHERE `world_has_objects`.`world_uuid` = CAST(0x%s AS BINARY) "
						"AND `world_has_objects`.`killinfo_uuid` IS NULL "
						"GROUP BY `object`.`uuid` "
						"ORDER BY `object`.priority ASC, `world_has_objects`.`parentobject_uuid` ASC"} % worlduuid);

	char typearray[] = {
			1, // HEX(`object`.`uuid`)
			1, // HEX(`world_has_objects`.`parentobject_uuid`)
			1, // `object`.`classname`
			0, // `object`.`priority`
			0, // `object`.`visible`
			1, // `object`.`accesscode`
			0, // `object`.`locked`
			1, // HEX(`object`.`player_uuid`)
			0, // `object`.`hitpoints`
			0, // `object`.`damage`
			0, // `object`.`fuel`
			0, // `object`.`fuelcargo`
			0, // `object`.`repaircargo`
			0, // `object`.`items`
			0, // `object`.`magazinesturret`
			0, // `object`.`variables`
			0, // `object`.`animationstate`
			0, // `object`.`textures`
			0, // `object`.`direction`
			0, // `object`.`positiontype`
			0, // `object`.`positionx`
			0, // `object`.`positiony`
			0, // `object`.`positionz`
			0, // `object`.`positionadvanced`
			0, // `object`.`reservedone`
			0, // `object`.`reservedtwo`
			2  // GROUP_CONCAT(`player`.`steamid` SEPARATOR '\", \"') AS friendlist
	};


	this->rawquery(query, &result);

	fieldcount = mysql_num_fields(result);
	rowcount = mysql_num_rows(result);

	// printf("fieldcount = %d\n", (int)fieldcount);
	// printf("rowcount = %d\n", (int)rowcount);

	std::vector< std::vector<std::string> > resultmatrix (rowcount, std::vector<std::string>(fieldcount));

	for (int rowpos = 0; rowpos < rowcount; rowpos++) {
		row = mysql_fetch_row(result);
		for (int fieldpos = 0; fieldpos < fieldcount; fieldpos++) {
			switch (typearray[fieldpos]) {
			case 1:
				resultmatrix[rowpos][fieldpos] = "\"";
				break;
			case 2:
				resultmatrix[rowpos][fieldpos] = "[";
				if (row[fieldpos] != NULL)
					resultmatrix[rowpos][fieldpos] += "\"";
				break;
			default:
				resultmatrix[rowpos][fieldpos] = "";
			}

			if (row[fieldpos] != NULL) {
				resultmatrix[rowpos][fieldpos] += row[fieldpos];
			} else {
				resultmatrix[rowpos][fieldpos] += "";
			}

			switch (typearray[fieldpos]) {
			case 1:
				resultmatrix[rowpos][fieldpos] += "\"";
				break;
			case 2:
				if (row[fieldpos] != NULL)
					resultmatrix[rowpos][fieldpos] += "\"";
				resultmatrix[rowpos][fieldpos] += "]";
				break;
			}
		}
	}

	mysql_free_result(result);

	return resultmatrix;
}

