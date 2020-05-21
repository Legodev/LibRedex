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

std::string schema_migration_0001 = R"(
ALTER TABLE `world`
ADD COLUMN `schema_version` INT AFTER `mods`,
ADD COLUMN `libredex_version` VARCHAR(15) AFTER `schema_version`,
ADD COLUMN `startuptime` TIMESTAMP NULL AFTER `libredex_version`;

ALTER TABLE `player`
DROP COLUMN `playtime`; 
ALTER TABLE `player`
ADD COLUMN `playtime` INT NOT NULL DEFAULT 0 AFTER `deaths`;

ALTER TABLE `clan_member`
ADD COLUMN `id` INT UNIQUE AUTO_INCREMENT FIRST;

ALTER TABLE `world_is_linked_to_world`
ADD COLUMN `id` INT UNIQUE AUTO_INCREMENT FIRST;
)";
