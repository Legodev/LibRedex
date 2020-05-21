/* mysql_binds.cpp
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

#include "mysql/mysql_binds.hpp"
#include <stdexcept>
#include <cstring>
#include <iostream>

mysql_binds::mysql_binds(unsigned int length) {
	//std::cout << "start binds init" << std::endl;
	this->maxpos = length;
	this->mysql_bind = new MYSQL_BIND[length];
	this->is_null = new my_bool[length];
	this->length = new long unsigned int[length];

	for (unsigned int arraypos = 0; arraypos < this->maxpos; arraypos++) {
		this->is_null[arraypos] = 1;
		this->length[arraypos] = 0;

		this->mysql_bind[arraypos].is_null = &this->is_null[arraypos];
		this->mysql_bind[arraypos].length = &this->length[arraypos];
		this->mysql_bind[arraypos].buffer_type = MYSQL_TYPE_NULL;
		this->mysql_bind[arraypos].buffer = 0;
	}

	//std::cout << "end binds init" << std::endl;
}

mysql_binds::~mysql_binds() {
	//std::cout << "start binds deconstruct" << std::endl;

	//std::cout << "start freeing buffers" << std::endl;
	if (this->mysql_bind != 0) {
		for (unsigned int arraypos = 0; arraypos < this->maxpos; arraypos++) {
			//std::cout << "start freeing buffer " << arraypos << " pointer " <<  this->mysql_bind[arraypos].buffer << std::endl;
			if (this->mysql_bind[arraypos].buffer != 0) {
				free(this->mysql_bind[arraypos].buffer);
				this->mysql_bind[arraypos].buffer = 0;
			}
		}
	}
	//std::cout << "start freeing bind" << std::endl;
	if (this->mysql_bind != 0) {
		delete this->mysql_bind;
		this->mysql_bind = 0;
	}
	//std::cout << "start freeing is_null" << std::endl;
	if (this->is_null != 0) {
		delete this->is_null;
		this->is_null = 0;
	}
	//std::cout << "start freeing length" << std::endl;
	if (this->length != 0) {
		delete this->length;
		this->length = 0;
	}

	//std::cout << "end binds deconstruct" << std::endl;
}

void mysql_binds::checkIncrement() {
	if (this->arraypos >= this->maxpos) {
		throw std::runtime_error("catched buffer overflow");
	}
}

void mysql_binds::setIncrement(unsigned int increment) {
	this->arraypos = increment;
}

void mysql_binds::increment() {
	this->arraypos++;
}

void mysql_binds::addNull() {
	//std::cout << "start addNull" << " arraypos " << this->arraypos << std::endl;

	this->checkIncrement();
	this->length[this->arraypos] = 0;
	this->is_null[this->arraypos] = 1;

	this->increment();

	//std::cout << "end addNull" << std::endl;
}

void mysql_binds::addData(signed char value) {
	//std::cout << "start addData signed char" << " arraypos " << this->arraypos << std::endl;

	this->checkIncrement();
	this->is_null[this->arraypos] = 0;
	this->length[this->arraypos] = 1;

	this->mysql_bind[this->arraypos].buffer_type = MYSQL_TYPE_TINY;
	signed char* pointer = (signed char*) this->mysql_bind[this->arraypos].buffer;

	if (pointer != 0) {
		delete pointer;
	}
	pointer = new signed char;
	*pointer = value;

	this->increment();

	//std::cout << "end addData signed char" << std::endl;
}

void mysql_binds::addData(int value) {
	//std::cout << "start addData int" << " arraypos " << this->arraypos << std::endl;

	this->checkIncrement();
	this->is_null[this->arraypos] = 0;
	this->length[this->arraypos] = 1;

	this->mysql_bind[this->arraypos].buffer_type = MYSQL_TYPE_LONG;

	int* pointer = (int*) this->mysql_bind[this->arraypos].buffer;
	if (pointer != 0) {
		delete pointer;
	}

	this->mysql_bind[this->arraypos].buffer = new int;
	pointer = (int*) this->mysql_bind[this->arraypos].buffer;
	*pointer = value;

	this->increment();

	//std::cout << "end addData int" << std::endl;
}

void mysql_binds::addData(unsigned int value) {
	//std::cout << "start addData unsigned int" << " arraypos " << this->arraypos << std::endl;

	this->checkIncrement();
	this->is_null[this->arraypos] = 0;
	this->length[this->arraypos] = 1;

	this->mysql_bind[this->arraypos].buffer_type = MYSQL_TYPE_LONG;

	unsigned int* pointer = (unsigned int*) this->mysql_bind[this->arraypos].buffer;
	if (pointer != 0) {
		delete pointer;
	}

	this->mysql_bind[this->arraypos].buffer = new unsigned int;
	pointer = (unsigned int*) this->mysql_bind[this->arraypos].buffer;
	*pointer = value;

	this->increment();

	//std::cout << "end addData unsigned int" << std::endl;
}

void mysql_binds::addData(float value) {
	//std::cout << "start addData float" << " arraypos " << this->arraypos << std::endl;

	this->checkIncrement();
	this->is_null[this->arraypos] = 0;
	this->length[this->arraypos] = 1;

	this->mysql_bind[this->arraypos].buffer_type = MYSQL_TYPE_FLOAT;

	float* pointer = (float*) this->mysql_bind[this->arraypos].buffer;
	if (pointer != 0) {
		delete pointer;
	}

	this->mysql_bind[this->arraypos].buffer = new float;
	pointer = (float*) this->mysql_bind[this->arraypos].buffer;
	*pointer = value;

	this->increment();

	//std::cout << "end addData float" << std::endl;
}

void mysql_binds::addData(std::string value) {
	//std::cout << "start addData string" << " arraypos " << this->arraypos << std::endl;

	this->checkIncrement();
	this->is_null[this->arraypos] = 0;

	this->mysql_bind[this->arraypos].buffer_type = MYSQL_TYPE_VAR_STRING;
	char* pointer = (char*) this->mysql_bind[this->arraypos].buffer;
	int size = value.size();

	//std::cout << "if length" << std::endl;
	// the database text fields are of the MySQL datatype TEXT, which can store 64 Kibibytes data
	// if our string is longer then these 64 Kibibytes we discard the data because truncation causes serious issues
	if (size >= 65530) {
		this->addNull();

		if (pointer != 0) {
			this->mysql_bind[this->arraypos].buffer = 0;
			this->mysql_bind[this->arraypos].buffer_length = 0;
			delete pointer;
			pointer = 0;
		}

		return;
	}

	//std::cout << "if previous data" << std::endl;
	if (pointer == 0 || std::string(pointer) != value) {
		if (pointer != 0 && this->mysql_bind[this->arraypos].buffer_length < size) {
			this->mysql_bind[this->arraypos].buffer = 0;
			this->mysql_bind[this->arraypos].buffer_length = 0;
			delete pointer;
			pointer = 0;
		}
		if (pointer == 0) {
			pointer = new char[size * 2];
			this->mysql_bind[this->arraypos].buffer = pointer;
			this->mysql_bind[this->arraypos].buffer_length = size * 2;
		}
	}
	//std::cout << "memcpy" << std::endl;
	memcpy(pointer, value.c_str(), size);
	pointer[size] = 0;
	this->length[this->arraypos] = size;

	this->increment();

	//std::cout << "end addData string" << std::endl;
}