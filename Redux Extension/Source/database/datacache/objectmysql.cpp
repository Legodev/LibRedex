/*
 * objectmysql.cpp
 *
 *  Created on: 11.07.2017
 *      Author: lego
 */


#include "database/datacache/objectmysql.hpp"
#include <string.h>
#include <boost/lexical_cast.hpp>

object_mysql::object_mysql() {
	memset(mysql_bind, 0, sizeof (mysql_bind));

	for (unsigned int arraypos = 0; arraypos < objectCacheMaxElements; arraypos++) {
		is_null[arraypos] = (my_bool) 1;
		mysql_bind[arraypos].is_null = &is_null[arraypos];
	}

	if (objectvariablemap.empty()) {
		objectvariablemap.insert(std::make_pair("uuid", 0));
		objectvariablemap.insert(std::make_pair("classname", 1));
		objectvariablemap.insert(std::make_pair("priority", 2));
		objectvariablemap.insert(std::make_pair("type", 3));
		objectvariablemap.insert(std::make_pair("accesscode", 4));
		objectvariablemap.insert(std::make_pair("locked", 5));
		objectvariablemap.insert(std::make_pair("player_uuid", 6));
		objectvariablemap.insert(std::make_pair("hitpoints", 7));
		objectvariablemap.insert(std::make_pair("damage", 8));
		objectvariablemap.insert(std::make_pair("fuel", 9));
		objectvariablemap.insert(std::make_pair("fuelcargo", 10));
		objectvariablemap.insert(std::make_pair("repaircargo", 11));
		objectvariablemap.insert(std::make_pair("items", 12));
		objectvariablemap.insert(std::make_pair("magazinesturret", 13));
		objectvariablemap.insert(std::make_pair("variables", 14));
		objectvariablemap.insert(std::make_pair("animationstate", 15));
		objectvariablemap.insert(std::make_pair("textures", 16));
		objectvariablemap.insert(std::make_pair("direction", 17));
		objectvariablemap.insert(std::make_pair("positiontype", 18));
		objectvariablemap.insert(std::make_pair("positionx", 19));
		objectvariablemap.insert(std::make_pair("positiony", 20));
		objectvariablemap.insert(std::make_pair("positionz", 21));
		objectvariablemap.insert(std::make_pair("positionadvanced", 22));
		objectvariablemap.insert(std::make_pair("reservedone", 23));
		objectvariablemap.insert(std::make_pair("reservedtwo", 24));
	}

	mysql_bind[0].buffer_type = MYSQL_TYPE_STRING; // uuid = "0"
	mysql_bind[1].buffer_type = MYSQL_TYPE_STRING; // classname =  "uninitializedobject"
	mysql_bind[2].buffer_type = MYSQL_TYPE_LONG;   // priority = 10001
	mysql_bind[2].buffer = (char *) &priority;

	mysql_bind[3].buffer_type = MYSQL_TYPE_TINY;   // type = 3
	mysql_bind[3].buffer = (char *) &type;

	mysql_bind[4].buffer_type = MYSQL_TYPE_STRING; // accesscode 	= ""
	mysql_bind[5].buffer_type = MYSQL_TYPE_TINY;   // locked 	= 0
	mysql_bind[5].buffer = (char *) &locked;

	mysql_bind[6].buffer_type = MYSQL_TYPE_STRING; // player_uuid = ""
	mysql_bind[7].buffer_type = MYSQL_TYPE_STRING; // hitpoints = "[]"
	mysql_bind[8].buffer_type = MYSQL_TYPE_FLOAT;  // damage = 1.0
	mysql_bind[8].buffer = (char *) &damage;

	mysql_bind[9].buffer_type = MYSQL_TYPE_FLOAT;  // fuel = 1.0
	mysql_bind[9].buffer = (char *) &fuel;

	mysql_bind[10].buffer_type = MYSQL_TYPE_FLOAT;  // fuelcargo = 0.0
	mysql_bind[10].buffer = (char *) &fuelcargo;

	mysql_bind[11].buffer_type = MYSQL_TYPE_FLOAT;  // repaircargo = 0.0
	mysql_bind[11].buffer = (char *) &repaircargo;

	mysql_bind[12].buffer_type = MYSQL_TYPE_STRING; // items = "[[],[],[],[],[]]"
	mysql_bind[13].buffer_type = MYSQL_TYPE_STRING; // magazinesturret = "[]"
	mysql_bind[14].buffer_type = MYSQL_TYPE_STRING; // variables = "[]"
	mysql_bind[15].buffer_type = MYSQL_TYPE_STRING; // animationstate = "[]"
	mysql_bind[16].buffer_type = MYSQL_TYPE_STRING; // textures = "[]"
	mysql_bind[17].buffer_type = MYSQL_TYPE_FLOAT;  // direction = 0.0
	mysql_bind[17].buffer = (char *) &direction;

	mysql_bind[18].buffer_type = MYSQL_TYPE_TINY;   // positiontype = 0
	mysql_bind[18].buffer = (char *) &positiontype;

	mysql_bind[19].buffer_type = MYSQL_TYPE_FLOAT;  // positionx = 0.0
	mysql_bind[19].buffer = (char *) &positionx;

	mysql_bind[20].buffer_type = MYSQL_TYPE_FLOAT;  // positiony = 0.0
	mysql_bind[20].buffer = (char *) &positiony;

	mysql_bind[21].buffer_type = MYSQL_TYPE_FLOAT;  // positionz = 0.0
	mysql_bind[21].buffer = (char *) &positionz;

	mysql_bind[22].buffer_type = MYSQL_TYPE_STRING; // positionadvanced = "[]"
	mysql_bind[23].buffer_type = MYSQL_TYPE_STRING; // reservedone = ""
	mysql_bind[24].buffer_type = MYSQL_TYPE_STRING; // reservedtwo = ""

	mysql_bind[25].buffer_type = MYSQL_TYPE_STRING; // parent_uuid = "0"
	mysql_bind[26].buffer_type = MYSQL_TYPE_STRING; // clan_uuid = "0"

}

object_mysql::~object_mysql() {
	this->freeStrings();
}

void object_mysql::freeStrings() {
	for (unsigned int arraypos = 0; arraypos < 25; arraypos++) {
		if (mysql_bind[arraypos].buffer != 0) {
			free(mysql_bind[arraypos].buffer);
			mysql_bind[arraypos].buffer = 0;
			mysql_bind[arraypos].buffer_length = 0;
		}
	}
}

int object_mysql::setData(std::string variableName, std::string variableValue) {
	auto it = objectvariablemap.find(variableName);
	if (it != objectvariablemap.end()) {
		unsigned int arraypos = it->second;

		setData(arraypos, variableValue);

	} else {
		throw std::runtime_error("unknown variable: " + variableName);
	}

	dirty = true;

	return 1;
}

int object_mysql::setData(unsigned int arraypos, std::string variableValue) {
	if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_STRING) {
		char * pointer = (char *) mysql_bind[arraypos].buffer;
		int size = variableValue.size() + 1;

		if (pointer == 0 || std::string(pointer) != variableValue) {
			if (pointer != 0 && mysql_bind[arraypos].buffer_length < size) {
				mysql_bind[arraypos].buffer = 0;
				mysql_bind[arraypos].buffer_length = 0;
				free(pointer);
			}

			char * pointer = new char[size*2];
			mysql_bind[arraypos].buffer = pointer;
			mysql_bind[arraypos].buffer_length = size * 2;
		}

		memcpy (mysql_bind[arraypos].buffer, variableValue.c_str(), size);
	}

	if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_FLOAT) {
		float * pointer = (float *) mysql_bind[arraypos].buffer;
		* pointer = boost::lexical_cast<float>(variableValue);
	}

	if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_TINY) {
		signed char * pointer = (signed char *) mysql_bind[arraypos].buffer;
		* pointer = boost::lexical_cast<signed char>(variableValue);
	}

	if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_LONG) {
		int * pointer = (int *) mysql_bind[arraypos].buffer;
		* pointer = boost::lexical_cast<int>(variableValue);
	}

	is_null[arraypos] = (my_bool) 0;

	return 1;
}

int object_mysql::setData(ext_arguments &extArgument) {
	std::list<std::string> keyList = extArgument.getKeys();

		for(auto const &key : keyList) {
			auto it = objectvariablemap.find(key);
			if (it != objectvariablemap.end()) {
				unsigned int arraypos = it->second;

				if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_STRING) {
					char * pointer = (char *) mysql_bind[arraypos].buffer;
					std::string value = extArgument.get<std::string>(key);
					int size = value.size() + 1;

					if (pointer == 0 || std::string(pointer) != value) {
						if (pointer != 0 && mysql_bind[arraypos].buffer_length < size) {
							mysql_bind[arraypos].buffer = 0;
							mysql_bind[arraypos].buffer_length = 0;
							free(pointer);

							char * pointer = new char(size*2);
							mysql_bind[arraypos].buffer = pointer;
							mysql_bind[arraypos].buffer_length = size * 2;
						}
						memcpy (mysql_bind[arraypos].buffer, value.c_str(), size);
					}
				}

				if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_FLOAT) {
					float * pointer = (float *) mysql_bind[arraypos].buffer;
					float value = extArgument.get<float>(key);

					if (* pointer != value) {
						* pointer = value;
						dirty = true;
					}
				}

				if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_TINY) {
					signed char * pointer = (signed char *) mysql_bind[arraypos].buffer;
					signed char value = extArgument.get<signed char>(key);

					if (* pointer != value) {
						* pointer = value;
						dirty = true;
					}
				}

				if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_LONG) {
					int * pointer = (int *) mysql_bind[arraypos].buffer;
					int value = extArgument.get<int>(key);

					if (* pointer != value) {
						* pointer = value;
						dirty = true;
					}
				}

				is_null[arraypos] = (my_bool) 0;

			} else {
				throw std::runtime_error("unknown variable: " + key);
			}
		}

	dirty = true;

	return 1;
}

std::string object_mysql::getAsArmaString() {
	std::string returnString = "[";
	bool placecommaone = false;

	for (unsigned int arraypos = 0; arraypos < objectCacheMaxElements; arraypos++) {
		if (placecommaone) {
			returnString += ",";
		}

		if (mysql_bind[arraypos].buffer != 0) {

			if (is_null[arraypos] == (my_bool) 0) {
				if (mysql_bind[arraypos].buffer_type == MYSQL_TYPE_STRING) {
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
