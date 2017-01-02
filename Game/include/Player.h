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

#include <stdint.h>
#include "Database.h"
#include "Location.h"

namespace x801 {
  namespace game {
    extern const Location defaultLocation;
    class Player {
    public:
      Player() : playerID(0), location(defaultLocation) {}
#pragma GCC diagnostic push                // fixing this warning will affect
#pragma GCC diagnostic ignored "-Weffc++"  // the signature of the method at hand
      Player(uint32_t id, Database& db) : playerID(id) {
        if (!db.loadPlayerLocation(id, location))
          location = defaultLocation;
      }
#pragma GCC diagnostic pop
      Player(const Player& other) :
        playerID(other.playerID), location(other.location) {}
      inline Player& operator=(const Player& other) {
        playerID = other.playerID;
        location = other.location;
        return *this;
      }
      // Player(const char* username, const uint8_t* passHash);
      Location& getLocation() { return location; }
      const Location& getLocation() const { return location; }
      ~Player();
    private:
      uint32_t playerID;
      Location location;
    };
  }
}
