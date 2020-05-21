/* charactermysql.cpp
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

#include "constants.hpp"
#include "mysql/datacache/charactermysql.hpp"
#include <string.h>
#include <boost/lexical_cast.hpp>

extern std::map<std::string, unsigned int> * charactervariablemap;

character_mysql::character_mysql() {
	memset(mysql_bind, 0, sizeof (mysql_bind));

	for (unsigned int arraypos = 0; arraypos < characterCacheMaxElements; arraypos++) {
		is_null[arraypos] = (my_bool) 1;
		mysql_bind[arraypos].is_null = &is_null[arraypos];
	}

	if (charactervariablemap->empty()) {
		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_ANIMATIONSTATE, 0));
		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_DIRECTION, 1));
		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_POSITIONTYPE, 2));
		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_POSITIONX, 3));
		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_POSITIONY, 4));
		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_POSITIONZ, 5));
		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_CHARUUID, 6));

		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_CLASSNAME, 7));
		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_HITPOINTS, 8));
		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_VARIABLES, 9));
		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_TEXTURES, 10));
		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_GEAR, 11));
		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_CURRENTWEAPON, 12));
		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_CHARSHAREUUID, 13));

		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_PERSISTENTVARIABLES, 14));
		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_PERSISTENTVARIABUUID, 15));

		charactervariablemap->insert(std::make_pair(PROTOCOL_DBCALL_ARGUMENT_PARENTUUID, 16));
	}


	mysql_bind[0].buffer_type = MYSQL_TYPE_VAR_STRING; // animationstate =  "uninitializedcharacter"

	mysql_bind[1].buffer_type = MYSQL_TYPE_FLOAT;  // direction = 0.0
	mysql_bind[1].buffer = (char *) &direction;

	mysql_bind[2].buffer_type = MYSQL_TYPE_TINY;   // positiontype = 0
	mysql_bind[2].buffer = (char *) &positiontype;

	mysql_bind[3].buffer_type = MYSQL_TYPE_FLOAT;  // positionx = 0.0
	mysql_bind[3].buffer = (char *) &positionx;

	mysql_bind[4].buffer_type = MYSQL_TYPE_FLOAT;  // positiony = 0.0
	mysql_bind[4].buffer = (char *) &positiony;

	mysql_bind[5].buffer_type = MYSQL_TYPE_FLOAT;  // positionz = 0.0
	mysql_bind[5].buffer = (char *) &positionz;

	mysql_bind[6].buffer_type = MYSQL_TYPE_VAR_STRING; // character_uuid = "0"


	mysql_bind[7].buffer_type = MYSQL_TYPE_VAR_STRING; // classname 	= ""
	mysql_bind[8].buffer_type = MYSQL_TYPE_VAR_STRING; // hitpoints 	= ""
	mysql_bind[9].buffer_type = MYSQL_TYPE_VAR_STRING; // variables 	= ""
	mysql_bind[10].buffer_type = MYSQL_TYPE_VAR_STRING; // textures 	= ""
	mysql_bind[11].buffer_type = MYSQL_TYPE_VAR_STRING; // gear 	= ""
	mysql_bind[12].buffer_type = MYSQL_TYPE_VAR_STRING; // currentweapon 	= ""
	mysql_bind[13].buffer_type = MYSQL_TYPE_VAR_STRING; // charactershareables_uuid 	= "0"

	mysql_bind[14].buffer_type = MYSQL_TYPE_VAR_STRING; // persistentvariables 	= ""
	mysql_bind[15].buffer_type = MYSQL_TYPE_VAR_STRING; // persistent_variables_uuid 	= ""

	mysql_bind[16].buffer_type = MYSQL_TYPE_VAR_STRING; // object_uuid 	= "0"
}

character_mysql::~character_mysql() {
	this->freeStrings();
}

void character_mysql::freeStrings() {
	for (unsigned int arraypos = 0; arraypos < characterCacheMaxElements; arraypos++) {
		if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_VAR_STRING && mysql_bind[arraypos].buffer != 0) {
			free(mysql_bind[arraypos].buffer);
			mysql_bind[arraypos].buffer = 0;
			mysql_bind[arraypos].buffer_length = 0;
		}
	}
}

int character_mysql::setNull(unsigned int arraypos) {
	is_null[arraypos] = (my_bool) 1;
	mysql_bind[arraypos].is_null = &is_null[arraypos];

	dirty = true;

	return 1;
}

int character_mysql::setData(std::string variableName, std::string variableValue) {
	auto it = charactervariablemap->find(variableName);
	if (it != charactervariablemap->end()) {
		unsigned int arraypos = it->second;

		setData(arraypos, variableValue);

	} else {
		throw std::runtime_error("unknown variable: " + variableName);
	}

	dirty = true;

	return 1;
}

int character_mysql::setData(ext_arguments &extArgument) {
	std::list<std::string> keyList = extArgument.getKeys();

	for(auto const &key : keyList) {
		auto it = charactervariablemap->find(key);
		if (it != charactervariablemap->end()) {
			unsigned int arraypos = it->second;
			std::string variableValue = extArgument.get<std::string>(key);

			if (variableValue == "" && key.find("uuid") != std::string::npos) {
				setNull(arraypos);
			} else {
				setData(arraypos, variableValue);
			}
		}
	}

	dirty = true;

	return 1;
}

int character_mysql::setData(unsigned int arraypos, std::string variableValue) {
	if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_VAR_STRING) {
		char * pointer = (char *) mysql_bind[arraypos].buffer;
		int size = variableValue.size();

		// the database text fields are of the MySQL datatype TEXT, which can store 64 Kibibytes data
		// if our string is longer then these 64 Kibibytes we discard the data because truncation causes serious issues
		if (size >= 65530) {
			this->setNull(arraypos);

			mysql_bind[arraypos].buffer = 0;
			mysql_bind[arraypos].buffer_length = 0;
			free(pointer);
			pointer = 0;

			return 0;
		}

		if (pointer == 0 || std::string(pointer) != variableValue) {
			if (pointer != 0 && mysql_bind[arraypos].buffer_length < size) {
				mysql_bind[arraypos].buffer = 0;
				mysql_bind[arraypos].buffer_length = 0;
				free(pointer);
				pointer = 0;
			}
			if (pointer == 0) {
				pointer = new char[size*2];
				mysql_bind[arraypos].buffer = pointer;
				mysql_bind[arraypos].buffer_length = size * 2;
			}
		}
		memcpy (mysql_bind[arraypos].buffer, variableValue.c_str(), size);
		pointer[size] = 0;
		length[arraypos] = size;
		mysql_bind[arraypos].length = &length[arraypos];
	}

	if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_FLOAT) {
		float * pointer = (float *) mysql_bind[arraypos].buffer;
		* pointer = boost::lexical_cast<float>(variableValue);
	}

	if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_TINY) {
		signed char * pointer = (signed char *) mysql_bind[arraypos].buffer;
		// lexical_cast casts all 8 bit values as character not as number ( 0 becomes 48 instead of 0)
		// * pointer = boost::lexical_cast<signed char>(variableValue);
		* pointer = (signed char) boost::lexical_cast<int>(variableValue);
	}

	if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_LONG) {
		int * pointer = (int *) mysql_bind[arraypos].buffer;
		* pointer = boost::lexical_cast<int>(variableValue);
	}

	is_null[arraypos] = 0;
	mysql_bind[arraypos].is_null = &is_null[arraypos];

	return 1;
}

std::string character_mysql::getAsArmaString() {
	std::string returnString = "[";
	bool placecommaone = false;

	for (unsigned int arraypos = 0; arraypos < characterCacheMaxElements; arraypos++) {
		if (placecommaone) {
			returnString += ",";
		}

		if (mysql_bind[arraypos].buffer != 0) {

			if (is_null[arraypos] == (my_bool) 0) {
				if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_VAR_STRING) {
					char * pointer = (char *) mysql_bind[arraypos].buffer;
					std::string data = pointer;
					if (data.front() == '[') {
						returnString += data;
					} else {
						returnString += std::string("\"") + data + std::string("\"");
					}
				}

				if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_FLOAT) {
					float * pointer = (float *) mysql_bind[arraypos].buffer;
					returnString += std::to_string(*pointer);
				}

				if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_TINY) {
					signed char * pointer = (signed char *) mysql_bind[arraypos].buffer;
					returnString += std::to_string(*pointer);
				}

				if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_LONG) {
					int * pointer = (int *) mysql_bind[arraypos].buffer;
					returnString += std::to_string(*pointer);
				}
			} else {
				returnString += std::string("\"\"");
			}
		} else {
			returnString += std::string("\"\"");
		}

		placecommaone = true;
	}

	returnString += "]";

	return returnString;
}
