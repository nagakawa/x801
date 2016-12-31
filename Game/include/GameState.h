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
#include <string>
#include <unordered_map>
#include <Area.h>
#include "Database.h"

namespace x801 {
  namespace game {
    class GameState;
    class AreaWithPlayers {
    public:
      AreaWithPlayers(GameState* g, std::istream& fh) :
          g(g), area(new x801::map::Area(fh)) {}
      ~AreaWithPlayers();
    private:
      GameState* g;
      x801::map::Area* area = nullptr;
    };
    class GameState {
    public:
      std::string getUsernameByID(int id) { return usernamesByID[id]; }
      int getIDByUsername(std::string& name) { return idsByUsername[name]; }
    private:
      Database db;
      std::unordered_map<int, std::string> usernamesByID;
      std::unordered_map<std::string, int> idsByUsername;
    };
  }
}
