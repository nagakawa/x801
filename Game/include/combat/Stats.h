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
#include <iosfwd>
#include <vector>

#include <gmpxx.h>

#include "combat/School.h"

namespace x801 {
  namespace game {
    class StatsUser {
    public:
      size_t level;
      size_t school;
      size_t xp;
    };
    class Stats {
    public:
      Stats(const StatsUser& su, const std::vector<School>& schools);
      Stats(std::istream& fh);
      Stats() {}
      class SchoolSpecific {
      public:
        SchoolSpecific() : damage(0), resist(0), accuracy(0), pierce(0) {}
        SchoolSpecific(std::istream& fh);
        mpz_class damage;
        mpz_class resist;
        int accuracy; // 0.1%s
        size_t pierce;
      };
      size_t level;
      size_t school;
      mpz_class maxHealth;
      size_t maxMana; // Surely no one will need more than 4000 million mana?
      std::vector<SchoolSpecific> ss;
      size_t powerPipChance; // Expressed in 0.1%s.
    };
  }
}