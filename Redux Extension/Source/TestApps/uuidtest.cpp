/* uuidtest.cpp
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

#include "testfileheader.hpp"

int main(int argc, char *argv[])
{
    char output[1024];
    const char function[] = "{ 'extFunction': 'dbcall', 'extArguments': {  'dbfunction': 'getUUID', 'extArgument': {  } } }";
    RVExtension(output, 1024, function);
    std::cout << output << std::endl;

    return 0;
}
