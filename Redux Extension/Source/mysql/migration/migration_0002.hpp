/* mysql.hpp
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

std::string schema_migration_0002 = R"(
ALTER TABLE `world`
MODIFY name VARCHAR(256),
MODIFY map VARCHAR(128);
ALTER TABLE `clan`
MODIFY name VARCHAR(128);
ALTER TABLE `clan_member`
MODIFY comment VARCHAR(256);
ALTER TABLE `killinfo`
MODIFY weapon VARCHAR(128),
MODIFY type VARCHAR(128);
)";
