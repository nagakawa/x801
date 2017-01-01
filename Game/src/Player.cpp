#include "Player.h"

/*
Copyright (C) 2016 AGC.


This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as
published by the Free Software Foundation, either version 3 of the
License, or (at your option) any later version.


This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.


You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

using namespace x801::game;

#include <utils.h>

x801::game::Player::Player(uint32_t id, Database& db) {
  playerID = id;
  x801::base::writeRandomBytes(cookie, COOKIE_LEN);
  db.loadPlayerLocation(id, location);
}

x801::game::Player::~Player() {
  // nothing
}