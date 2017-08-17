/* mysql.cpp
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

#include <exception>
#include <stdexcept>
#include <boost/format.hpp>
#include "database/mysql.hpp"
#include "utils/uuid.hpp"
#include "database/datacache/charactermysql.hpp"
#include "database/datacache/objectmysql.hpp"

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
	}

	return;
}

void mysql_db_handler::disconnect() {
	mysql_close(connection);
	return;
}

void mysql_db_handler::checkWorldUUID() {
	MYSQL_RES *result;
	MYSQL_ROW row;
	bool addUUID = true;
	unsigned long long int rowcount;

	std::string queryworlduuid =
	str(boost::format{"SELECT HEX(`world`.`uuid`) "
			"FROM `world` "
			"WHERE `world`.`uuid` = CAST(0x%s AS BINARY)"} % worlduuid);

	this->rawquery(queryworlduuid, &result);

	rowcount = mysql_num_rows(result);

	if (rowcount > 0) {
		addUUID = false;
	}

	mysql_free_result(result);

	if (addUUID) {
		queryworlduuid =
			str(boost::format{"INSERT INTO `world` (`uuid`, `name`, `map`) "
								"VALUES (CAST(0x%s AS BINARY), \"you need to\", \"eddit this\")"}
								% worlduuid);
		this->rawquery(queryworlduuid);
	}
};

void mysql_db_handler::rawquery(std::string query) {
//	printf("%s\n", query.c_str());

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
//	printf("%s\n", query.c_str());

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

void mysql_db_handler::preparedStatementQuery(std::string query, MYSQL_BIND input_params[]) {
	int        status;

	MYSQL_STMT *stmt;
	stmt = mysql_stmt_init(connection);
	if (!stmt)
	{
		throw std::runtime_error("Could not initialize statement\n");
	}

	status = mysql_stmt_prepare(stmt, query.c_str(), query.size() + 1);
	test_stmt_error(stmt, status);

	status = mysql_stmt_bind_param(stmt, input_params);
	test_stmt_error(stmt, status);

	status = mysql_stmt_execute(stmt);
	test_stmt_error(stmt, status);

	status = mysql_stmt_close(stmt);
	test_stmt_error(stmt, status);

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
	std::string persistent_variables_uuid = "";
	std::string mainclanuuid = "";
	std::string banned = "false";
	std::string banreason = "unknown";

	std::string queryplayerinfo =
	str(boost::format{"SELECT HEX(`player`.`uuid`), "
			"HEX(`player_on_world_has_persistent_variables`.`persistent_variables_uuid`), "
			"HEX(`player`.`mainclan_uuid`), "
		    "(CASE WHEN (NOW() < `player`.`banenddate`) THEN \"true\" ELSE \"false\" END) AS BANNED, "
		    "`player`.`banreason`"
			"FROM `player` "
			"LEFT JOIN `player_on_world_has_persistent_variables` "
			" ON `player`.`uuid` = `player_on_world_has_persistent_variables`.`player_uuid` "
			" AND `player_on_world_has_persistent_variables`.`world_uuid` =  CAST(0x%s AS BINARY) "
			"WHERE `player`.`steamid` = \"%s\" "} % worlduuid % steamid);

	char typearrayplayerinfo[] = {
			1, // HEX(`actualplayer`.`uuid`)
			1, // HEX(`player_on_world_has_persistent_variables`.`persistent_variables_uuid`)
			2, // GROUP_CONCAT(`player`.`steamid` SEPARATOR '\", \"') AS mainclanuuid
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
			persistent_variables_uuid = row[1];
		}

		if (row[2] != NULL) {
			mainclanuuid = row[2];
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

	return "[\"" + playeruuid + "\",\"" + persistent_variables_uuid + "\",\"" + mainclanuuid + "\",[" + banned + ",\"" + banreason +"\"]]";
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
							"SELECT HEX(`persistent_variables`.`uuid`), "
									"`persistent_variables`.`persistentvariables`, `world`.`name`, `world`.`map` "
									"FROM `world_is_linked_to_world` "
									"INNER JOIN `player_on_world_has_persistent_variables` "
									" ON `world_is_linked_to_world`.`world_uuid2` = `player_on_world_has_persistent_variables`.`world_uuid` "
									"INNER JOIN `persistent_variables` "
									" ON `persistent_variables`.`uuid` = `player_on_world_has_persistent_variables`.`persistent_variables_uuid` "
									"INNER JOIN `world` "
									" ON `world`.`uuid` = `world_is_linked_to_world`.`world_uuid2` "
									"WHERE `world_is_linked_to_world`.`world_uuid1` = CAST(0x%s AS BINARY) "
									" AND `player_on_world_has_persistent_variables`.`player_uuid` = CAST(0x%s AS BINARY)" }
							% worlduuid % playeruuid);

	char typearray[] = {
			1, // HEX(`persistent_variables`.`uuid`)
			0, // `persistent_variables`.`persistentvariables`
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
			boost::format { "INSERT INTO `player_on_world_has_persistent_variables` "
					"(`player_uuid`, `world_uuid`, `persistent_variables_uuid`) "
					"VALUES (0x%s AS BINARY), CAST(0x%s AS BINARY), CAST(0x%s AS BINARY))"
					} % playeruuid % worlduuid % variabuuid);


	this->rawquery(query);

	return playeruuid;
}

cache_base* mysql_db_handler::loadChar(std::map<std::string, cache_base*> &charactercache, std::string playeruuid) {
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	bool printcomma = false;
	character_mysql* character = 0;

	std::string querycharinfo = str(boost::format{"SELECT `animationstate`, `direction`, `positiontype`, `positionx`, `positiony`, `positionz`, `character`.`uuid` as `character_uuid`, "
				"`classname`, `hitpoints`, `variables`, `textures`, `gear`, `currentweapon`, `character`.`charactershareables_uuid`, "
				"`persistentvariables`, `charactershareables`.`persistent_variables_uuid`, `object_uuid` "
				"FROM `player_on_world_has_character` "
				"INNER JOIN `character`  "
				" ON `player_on_world_has_character`.`character_uuid` = `character`.`uuid` "
				"INNER JOIN `charactershareables` "
				" ON `character`.`charactershareables_uuid` = `charactershareables`.`uuid` "
				"INNER JOIN `persistent_variables` "
				" ON `charactershareables`.`persistent_variables_uuid` = `persistent_variables`.`uuid` "
				"WHERE `player_on_world_has_character`.`player_uuid` = CAST(0x%s AS BINARY) "
				" AND `player_on_world_has_character`.`world_uuid` =  CAST(0x%s AS BINARY) "
				" AND `player_on_world_has_character`.`killinfo_uuid` IS NULL" } % playeruuid % worlduuid);

	this->rawquery(querycharinfo, &result);

	fieldcount = mysql_num_fields(result);
	rowcount = mysql_num_rows(result);

//	for (int rowpos = 0; rowpos < rowcount && rowpos < 25; rowpos++) {
		row = mysql_fetch_row(result);
		if (row[6] != NULL && row[13] != NULL && row[15] != NULL) {
			std::string charuuid = row[6];

			auto it = charactercache.find(charuuid);
			if (it != charactercache.end()) {
				character = static_cast<character_mysql*>((void*)it->second);
			} else {
				character_mysql* character = new character_mysql;
				charactercache.insert(std::make_pair(charuuid, (cache_base*) character));
			}

			for (int fieldpos = 0; fieldpos < fieldcount; fieldpos++) {
				if (row[fieldpos] != NULL) {
					character->setData(fieldpos, row[fieldpos]);
				} else {
					character->setData(fieldpos, "");
					character->is_null[fieldpos] = (my_bool) 1;
				}
			}
		}
//	}

	mysql_free_result(result);

	return character;
}


std::string mysql_db_handler::createChar(std::map<std::string, cache_base*> &charactercache, ext_arguments &extArgument) {
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;

	std::string playeruuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_PLAYER_UUID);

	// char info
	std::string charuuid = "";
	std::string persistent_variables_uuid = "";
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
		charuuid = orderedUUID();
		character_mysql* character = new character_mysql;
		character->setData(extArgument);
		character->setData(PROTOCOL_DBCALL_ARGUMENT_CHARUUID, charuuid);
		charactercache.insert(std::make_pair(charuuid, (cache_base*) character));

		/* get the uuid for the Death Persistent Variables */
		query = str(boost::format { "SELECT HEX(`persistent_variables_uuid`) "
				"FROM `player_on_world_has_persistent_variables` "
				"WHERE `player_uuid` = CAST(0x%s AS BINARY) "
				"AND `world_uuid` = CAST(0x%s AS BINARY)" } % playeruuid % worlduuid);

		this->rawquery(query, &result);

		rowcount = mysql_num_rows(result);

		// printf("rowcount = %d\n", (int) rowcount);

		if (rowcount > 0) {
			row = mysql_fetch_row(result);

			if (row[0] != NULL) {
				persistent_variables_uuid = row[0];
			}
		}

		mysql_free_result(result);

		/* if there are no Death Persistent Variables create some*/
		if (persistent_variables_uuid == "") {
			persistent_variables_uuid = orderedUUID();
			character->setData(PROTOCOL_DBCALL_ARGUMENT_PERSISTENTVARIABUUID, persistent_variables_uuid);

			query = "INSERT INTO `persistent_variables` (`persistentvariables`, `uuid`) VALUES (?, UNHEX(?))";

			this->preparedStatementQuery(query, character->mysql_bind+14);

			query = str(boost::format { "INSERT INTO `player_on_world_has_persistent_variables` "
					"(`player_uuid`, `world_uuid`, `persistent_variables_uuid`) "
					"VALUES (CAST(0x%s AS BINARY), "
					"CAST(0x%s AS BINARY), "
					"CAST(0x%s AS BINARY));" } % playeruuid % worlduuid % persistent_variables_uuid);

			this->rawquery(query);
		} else {
			character->setData(PROTOCOL_DBCALL_ARGUMENT_PERSISTENTVARIABUUID, persistent_variables_uuid);
			query = str(boost::format { "SELECT `persistentvariables` "
							"FROM `persistent_variables` "
							"WHERE `persistent_variables`.`uuid` = CAST(0x%s AS BINARY)"
							} % persistent_variables_uuid);

			this->rawquery(query, &result);

			rowcount = mysql_num_rows(result);

			if (rowcount > 0) {
				row = mysql_fetch_row(result);

				if (row[0] != NULL) {
					character->setData(PROTOCOL_DBCALL_ARGUMENT_PERSISTENTVARIABLES, row[0]);
				}
			}
			mysql_free_result(result);
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
				"FROM `player_on_world_has_persistent_variables` "
				"WHERE `persistent_variables_uuid` = CAST(0x%s AS BINARY) "
				") LIMIT 1" } % playeruuid % persistent_variables_uuid);

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
			character->setData(PROTOCOL_DBCALL_ARGUMENT_CHARSHAREUUID, shareable_variables_uuid);

			query = str(
					boost::format { "INSERT INTO `charactershareables` (`classname`, `hitpoints`, "
							"`variables`, `textures`, `gear`, "
							"`currentweapon`, `uuid`, `persistent_variables_uuid`) "
							"VALUES (?, ?, ?, ?, ?, ?, UNHEX(?), CAST(0x%s AS BINARY))" } % persistent_variables_uuid);

			this->preparedStatementQuery(query, character->mysql_bind+7);
		} else {
			character->setData(PROTOCOL_DBCALL_ARGUMENT_CHARSHAREUUID, shareable_variables_uuid);

			query = str(boost::format { "SELECT `classname`, `hitpoints`, "
							"`variables`, `textures`, `gear`, `currentweapon`"
							"FROM `charactershareables` "
							"WHERE `charactershareables`.`uuid` = CAST(0x%s AS BINARY)"
							} % shareable_variables_uuid);

			this->rawquery(query, &result);

			rowcount = mysql_num_rows(result);

			if (rowcount > 0) {
				row = mysql_fetch_row(result);

				if (row[0] != NULL) {
					character->setData(PROTOCOL_DBCALL_ARGUMENT_CLASSNAME, row[0]);
				}
				if (row[1] != NULL) {
					character->setData(PROTOCOL_DBCALL_ARGUMENT_HITPOINTS, row[1]);
				}
				if (row[2] != NULL) {
					character->setData(PROTOCOL_DBCALL_ARGUMENT_VARIABLES, row[2]);
				}
				if (row[3] != NULL) {
					character->setData(PROTOCOL_DBCALL_ARGUMENT_TEXTURES, row[3]);
				}
				if (row[4] != NULL) {
					character->setData(PROTOCOL_DBCALL_ARGUMENT_GEAR, row[4]);
				}
				if (row[5] != NULL) {
					character->setData(PROTOCOL_DBCALL_ARGUMENT_CURRENTWEAPON, row[5]);
				}
			}
			mysql_free_result(result);
		}

		query = str(
				boost::format { "INSERT INTO `character` (`animationstate`, `direction`, "
						"`positiontype`, `positionx`, `positiony`, "
						"`positionz`, `uuid`, `charactershareables_uuid`) "
						"VALUES (?, ?, ?, ?, ?, ?, "
						"UNHEX(?), CAST(0x%s AS BINARY))" } % shareable_variables_uuid);

		this->preparedStatementQuery(query, character->mysql_bind);

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

std::string mysql_db_handler::updateChar(std::map<std::string, cache_base*> &charactercache, ext_arguments &extArgument) {
	std::string charuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_CHARUUID);
	character_mysql* character = 0;

	auto it = charactercache.find(charuuid);
	if (it != charactercache.end()) {
		character = static_cast<character_mysql*>((void*)it->second);
	} else {
		throw std::runtime_error("could not find character to update: " + charuuid);
	}

	std::string query = "UPDATE `character` "
					"SET `animationstate` = ?, "
					"`direction` = ?, "
					"`positiontype` = ?, "
					"`positionx` = ?, "
					"`positiony` = ?, "
					"`positionz` = ? "
					" WHERE `character`.`uuid` = UNHEX(?)";

	this->preparedStatementQuery(query, character->mysql_bind);

	query = "UPDATE `charactershareables` "
					"SET `classname` = ?, "
					"`hitpoints` = ?, "
					"`variables` = ?, "
					"`textures` = ?, "
					"`gear` = ?, "
					"`currentweapon` = ? "
					" WHERE `charactershareables`.`uuid` = UNHEX(?)";

	this->preparedStatementQuery(query, character->mysql_bind+7);

	query = "UPDATE `persistent_variables` "
			"SET `persistentvariables` = ? "
			"WHERE `persistent_variables`.`uuid` = UNHEX(?)";

	this->preparedStatementQuery(query, character->mysql_bind+14);

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
					"`object`.`classname`, `object`.`priority`, `object`.`type`, `object`.`accesscode`, "
					"`object`.`locked`, HEX(`object`.`player_uuid`), `object`.`hitpoints`, `object`.`damage`, "
					"`object`.`fuel`, `object`.`fuelcargo`, `object`.`repaircargo`, `object`.`items`, "
					"`object`.`magazinesturret`, "
					"`object`.`variables`, `object`.`animationstate`, `object`.`textures`, `object`.`direction`, "
					"`object`.`positiontype`, `object`.`positionx`, `object`.`positiony`, `object`.`positionz`, "
					"`object`.`positionadvanced`, `object`.`reservedone`, `object`.`reservedtwo`, "
					"GROUP_CONCAT(`player`.`steamid` SEPARATOR '\", \"') AS mainclanuuid "
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
			0, // `object`.`type`
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
			2  // GROUP_CONCAT(`player`.`steamid` SEPARATOR '\", \"') AS mainclanuuid
			};



	//this->rawquery("SELECT HEX(uuid) FROM `world` WHERE uuid = CAST(0x11e66ac83e75277882c510bf48883ace AS BINARY)", &result);
	this->rawquery(query, &result);

	fieldcount = mysql_num_fields(result);
	rowcount = mysql_num_rows(result);

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
#include <unistd.h>
std::string mysql_db_handler::createObject(std::map<std::string, cache_base*> &objectcache, ext_arguments &extArgument) {
	std::string objectuuid;

	object_mysql* object = new object_mysql;
	object->setData(extArgument);

	try {
		objectuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	} catch (std::runtime_error const& e) {
		objectuuid = orderedUUID();
		object->setData(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID, objectuuid);
	}

	objectcache.insert(std::make_pair(objectuuid, (cache_base*) object));

	std::string query = "INSERT INTO `object` (`classname`, `priority`, `timelastused`, "
							"`timecreated`, `type`, `accesscode`, `locked`, `player_uuid`, `hitpoints`, "
							"`damage`, `fuel`, `fuelcargo`, `repaircargo`, `items`, `magazinesturret`, "
							"`variables`, `animationstate`, `textures`, `direction`, `positiontype`, "
							"`positionx`, `positiony`, `positionz`, "
							"`positionadvanced`, `reservedone`, `reservedtwo`, `uuid`) "
							"VALUES (?, ?, now(), "
							"now(), ?, ?, ?, UNHEX(?), ?, "
							"?, ?, ?, ?, ?, ?, ?, "
							"?, ?, ?, ?, ?, ?, ?, "
							"?, ?, ?, UNHEX(?))";

	this->preparedStatementQuery(query, object->mysql_bind);

	query = str(boost::format { "INSERT INTO `world_has_objects` (`world_uuid`, `object_uuid`) "
			"VALUES (CAST(0x%s AS BINARY), CAST(0x%s AS BINARY))" } % worlduuid % objectuuid);

	this->rawquery(query);

	return objectuuid;
}

std::string mysql_db_handler::updateObject(std::map<std::string, cache_base*> &objectcache, ext_arguments &extArgument) {
	std::string objectuuid = extArgument.getUUID(PROTOCOL_DBCALL_ARGUMENT_OBJECTUUID);
	object_mysql* object = 0;

	auto it = objectcache.find(objectuuid);
	if (it != objectcache.end()) {
		object = static_cast<object_mysql*>((void*)it->second);
	} else {
		throw std::runtime_error("could not find object to update: " + objectuuid);
	}

	std::string query = "UPDATE `object` "
						"SET `classname` = ?, "
						"    `priority` = ?, "
						"    `type` = ?,"
						"    `accesscode` = ?, "
						"    `locked` = ?, "
						"    `player_uuid` = UNHEX(?), "
						"    `hitpoints` = ?, "
						"    `damage` = ?, "
						"    `fuel` = ?, "
						"    `fuelcargo` = ?, "
						"    `repaircargo` = ?, "
						"    `items` = ?, "
						"    `magazinesturret` = ?, "
						"    `variables` = ?, "
						"    `animationstate` = ?, "
						"    `textures` = ?, "
						"    `direction` = ?, "
						"    `positiontype` = ?, "
						"    `positionx` = ?, "
						"    `positiony` = ?, "
						"    `positionz` = ?, "
						"    `positionadvanced` = ?, "
						"    `reservedone` = ?, "
						"    `reservedtwo` = ? "
						"WHERE `object`.`uuid` = UNHEX(?);";
	this->preparedStatementQuery(query, object->mysql_bind);

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

//std::vector<cache_base*> mysql_db_handler::dumpObjects(std::map<std::string, cache_base*> &objectcache) {
//	std::vector<cache_base*> objectList;
//	MYSQL_BIND input_params[1];
//	int        status;
//
//	long unsigned int length = 35;
//	char buffer[35] = "0x";
//	memcpy (buffer+2, worlduuid.c_str(), 33);
//	buffer[34] = 0;
//
//	input_params[0].buffer_type = MYSQL_TYPE_STRING;
//	input_params[0].buffer = (void*) buffer;
//	input_params[0].length = &length;
//	input_params[0].is_null = 0;
//
//	MYSQL_STMT *stmt;
//	stmt = mysql_stmt_init(connection);
//	if (!stmt)
//	{
//	  printf("Could not initialize statement\n");
//	  exit(1);
//	}
//
//	std::string query = "SELECT HEX(`object`.`uuid`), "
//							"`object`.`classname`, `object`.`priority`, `object`.`type`, `object`.`accesscode`, "
//							"`object`.`locked`, HEX(`object`.`player_uuid`), `object`.`hitpoints`, `object`.`damage`, "
//							"`object`.`fuel`, `object`.`fuelcargo`, `object`.`repaircargo`, `object`.`items`, "
//							"`object`.`magazinesturret`, "
//							"`object`.`variables`, `object`.`animationstate`, `object`.`textures`, `object`.`direction`, "
//							"`object`.`positiontype`, `object`.`positionx`, `object`.`positiony`, `object`.`positionz`, "
//							"`object`.`positionadvanced`, `object`.`reservedone`, `object`.`reservedtwo`, "
//							"HEX(`world_has_objects`.`parentobject_uuid`), "
//							"HEX(`player`.`uuid`) "
//							"FROM `world_has_objects` "
//							"INNER JOIN `object` "
//							" ON `world_has_objects`.`object_uuid` = `object`.`uuid` "
//							"LEFT JOIN `player` "
//							" ON `object`.`player_uuid` = `player`.`uuid` "
//							"WHERE `world_has_objects`.`world_uuid` = CAST(? AS BINARY) "
//							"OR `world_has_objects`.`killinfo_uuid` IS NULL "
//							"ORDER BY `object`.priority ASC, `world_has_objects`.`parentobject_uuid` ASC";
//
//	status = mysql_stmt_prepare(stmt, query.c_str(), query.size() + 1);
//	test_stmt_error(stmt, status);
//
//	status = mysql_stmt_bind_param(stmt, input_params);
//	test_stmt_error(stmt, status);
//
//	status = mysql_stmt_execute(stmt);
//	test_stmt_error(stmt, status);
//
//	do {
//		int i;
//		int num_fields;       /* number of columns in result */
//		MYSQL_FIELD *fields;  /* for result set metadata */
//
//		/* the column count is > 0 if there is a result set */
//		/* 0 if the result is only the final status packet */
//		num_fields = mysql_stmt_field_count(stmt);
//
//		if (num_fields > objectCacheMaxElements) {
//			throw std::runtime_error("to many fields in dump objects:" + std::to_string(num_fields));
//		}
//
//		if (num_fields > 0)
//		{
//			MYSQL_RES *rs_metadata = mysql_stmt_result_metadata(stmt);
//
//			fields = mysql_fetch_fields(rs_metadata);
//
//			object_mysql* object = new object_mysql;
//
//			for (i = 0; i < num_fields; ++i)
//			{
//				if (object->mysql_bind[i].buffer_type != fields[i].type && fields[i].type != 252) {
//					throw std::runtime_error("unexpected field type in dump objects field " + std::to_string(i) +
//							std::string(" has type ") + std::to_string(object->mysql_bind[i].buffer_type) +
//							std::string(" but got type ") + std::to_string(fields[i].type));
//				}
//			}
//
//			/* fetch and display result set rows */
//			while (1)
//			{
//				for (unsigned int arraypos = 0; arraypos < num_fields; arraypos++) {
//					if (object->mysql_bind[arraypos].buffer_type == MYSQL_TYPE_VAR_STRING) {
//						if (object->mysql_bind[arraypos].buffer == 0) {
//							char * pointer = new char[64];
//							object->mysql_bind[arraypos].buffer = pointer;
//							object->mysql_bind[arraypos].buffer_length = 64;
//						}
//					}
//				}
//
//				status = mysql_stmt_bind_result(stmt, object->mysql_bind);
//				test_stmt_error(stmt, status);
//
//				status = mysql_stmt_fetch(stmt);
//
//				if (status == 1 || status == MYSQL_NO_DATA) {
////					delete object;
//					break;
//				}
//
//				std::string uuid = (char*) object->mysql_bind[0].buffer;
//				objectList.push_back((cache_base*) object);
//				objectcache.insert(std::make_pair(uuid, (cache_base*) object));
//
//				object_mysql* object = new object_mysql;
//			}
//
//			mysql_free_result(rs_metadata); /* free metadata */
//		}
//
//		/* more results? -1 = no, >0 = error, 0 = yes (keep looking) */
//		status = mysql_stmt_next_result(stmt);
//		if (status > 0) {
//			test_stmt_error(stmt, status);
//		}
//
//	} while (status == 0);
//
//	return objectList;
//}

std::vector<cache_base*> mysql_db_handler::dumpObjects(std::map<std::string, cache_base*> &objectcache) {
	std::vector<cache_base*> objectList;
	MYSQL_RES *result;
	MYSQL_ROW row;
	unsigned int fieldcount;
	unsigned long long int rowcount;
	std::string query =
	str(boost::format{"SELECT `object`.`classname`, `object`.`priority`, `object`.`type`, `object`.`accesscode`, "
						"`object`.`locked`, HEX(`object`.`player_uuid`), `object`.`hitpoints`, `object`.`damage`, "
						"`object`.`fuel`, `object`.`fuelcargo`, `object`.`repaircargo`, `object`.`items`, "
						"`object`.`magazinesturret`, "
						"`object`.`variables`, `object`.`animationstate`, `object`.`textures`, `object`.`direction`, "
						"`object`.`positiontype`, `object`.`positionx`, `object`.`positiony`, `object`.`positionz`, "
						"`object`.`positionadvanced`, `object`.`reservedone`, `object`.`reservedtwo`, "
						"HEX(`object`.`uuid`), "
						"HEX(`world_has_objects`.`parentobject_uuid`), "
						"HEX(`player`.`uuid`) "
						"FROM `world_has_objects` "
						"INNER JOIN `object` "
						" ON `world_has_objects`.`object_uuid` = `object`.`uuid` "
						"LEFT JOIN `player` "
						" ON `object`.`player_uuid` = `player`.`uuid` "
						"WHERE `world_has_objects`.`world_uuid` = CAST(0x%s AS BINARY) "
						"AND `world_has_objects`.`killinfo_uuid` IS NULL "
						"ORDER BY `object`.priority ASC, `world_has_objects`.`parentobject_uuid` ASC"} % worlduuid);

	this->rawquery(query, &result);

	fieldcount = mysql_num_fields(result);
	rowcount = mysql_num_rows(result);

	for (int rowpos = 0; rowpos < rowcount; rowpos++) {
		row = mysql_fetch_row(result);
		if (row[24] != NULL) {
			std::string uuid = row[24];

			object_mysql* object = new object_mysql;
			objectList.push_back((cache_base*) object);
			objectcache.insert(std::make_pair(uuid, (cache_base*) object));

			for (int fieldpos = 0; fieldpos < fieldcount && fieldpos < 27; fieldpos++) {
				if (row[fieldpos] != NULL) {
					object->setData(fieldpos, row[fieldpos]);
				} else {
					object->setNull(fieldpos);
				}
			}
		}
	}

	mysql_free_result(result);

	return objectList;
}

void mysql_db_handler::test_stmt_error(MYSQL_STMT *stmt, int status) {
  if (status)
  {
	  throw std::runtime_error(std::string("MYSQLERROR") + std::string(mysql_stmt_error(stmt)) + std::string(" Errornumber: ") + std::to_string(mysql_stmt_errno(stmt)));
  }
}
