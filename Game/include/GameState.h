#pragma once

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

#if !defined(__cplusplus) || __cplusplus < 201103L
#error Only C++11 or later supported.
#endif

#include <iostream>
#include <memory>
#include <string>
#include <unordered_map>
#include <Area.h>
#include <QualifiedAreaID.h>
#include "Database.h"
#include "Player.h"

namespace x801 {
  namespace game {
    enum LoginStatus {
      LOGIN_OK = 0,
      LOGIN_INVALID_CREDENTIALS = 1,
      LOGIN_SERVER_FULL = 2,
      LOGIN_BANNED = 3,
      LOGIN_ALREADY_LOGGED_IN = 4,
      LOGIN_FAILED = 5,
      LOGIN_NOT_ENOUGH_DATA = 6,
    };
    class GameState;
    class AreaWithPlayers {
    public:
      AreaWithPlayers();
      AreaWithPlayers(GameState* g, std::istream& fh) :
          g(g), area(new x801::map::Area(fh)) {}
      AreaWithPlayers(const AreaWithPlayers& other) = delete;
      AreaWithPlayers& operator=(const AreaWithPlayers& other) = delete;
      ~AreaWithPlayers();
    private:
      GameState* g = nullptr;
      x801::map::Area* area = nullptr;
    };
    class GameState {
    public:
      LoginStatus login(Credentials& c, uint32_t& id);
      void logout(uint32_t id);
      Player& getPlayer(uint32_t id) {
        return allPlayers[id];
      }
      const std::string& getUsernameByID(uint32_t id) {
        return usernamesByID[id];
      }
      uint32_t getIDByUsername(std::string& name) {
        return idsByUsername[name];
      }
    private:
      Database db;
      std::unordered_map<uint32_t, Player> allPlayers;
      std::unordered_map<uint32_t, std::string> usernamesByID;
      std::unordered_map<std::string, uint32_t> idsByUsername;
      std::unordered_map<
        x801::map::QualifiedAreaID, std::unique_ptr<AreaWithPlayers>,
        x801::map::QualifiedAreaIDHash, x801::map::QualifiedAreaIDEqual
      > areas;
    };
  }
}
