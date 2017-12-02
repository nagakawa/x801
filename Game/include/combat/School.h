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
#include <string>
#include <vector>

#include <gmpxx.h>

namespace x801 {
  namespace game {
    class School {
    public:
      School(
          std::string name, std::string userName,
          size_t healthAtLevel0, size_t healthPerLevel,
          bool usable = true) :
        name(name), userName(userName),
        healthAtLevel0(healthAtLevel0), healthPerLevel(healthPerLevel),
        usable(usable) {}
      const std::string name; // e. g. "Fire"
      const std::string userName; // e. g. "Pyromancer"
      // Values that affect base health.
      // The following is based on the following table:
      // http://www.wizard101central.com/wiki/Basic:Level_Chart#Health.2C_Mana.2C_Power_Pip_Chance.2C_Energy
      // An important difference: Wizard101 starts at level 1,
      // but Experiment801 starts at level 0. We extrapolate to level 0.
      // Also, Wizard101 doesn't have linear base health increases
      // per level. We simplify here.
      const size_t healthAtLevel0; // e. g. 403
      const size_t healthPerLevel; // e. g. 22
      const bool usable;
      size_t getBaseHealth(size_t level) const {
        return healthAtLevel0 + level * healthPerLevel;
      }
    };
    enum DefaultSchoolIDs : size_t {
      FIRE = 0,
      ICE = 1,
      LIGHTNING = 2,
      WATER = 3,
      EARTH = 4,
      WIND = 5,
      LIGHT = 6,
      DARKNESS = 7,
      TOP = 8,
      BOTTOM = 9,
    };
    // Hard-coded list of schools.
    // The base stats might need rebalancing.
    extern std::vector<School> defaultSchools;
    // Returns true if the school "a" has mastery over school "b".
    bool schoolMasters(size_t a, size_t b);
  }
}