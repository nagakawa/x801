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
#include <array>

#include <boost/variant.hpp>

#include <gmpxx.h>

#include "combat/SpellIndex.h"

namespace x801 {
  namespace game {
    enum class PactTypes : uint_fast16_t {
      /*
        Indicates damage.
        [0:uint16_t] the school
        [1:uint8_t] the defender
        [2:mpz_class] amount of damage
      */
      damage = 0,
    };
    struct Pips {
      uint8_t normal;
      uint8_t power;
    };
    static constexpr size_t PLAYERS_PER_SIDE = 8;
    static constexpr size_t ALL_ENEMIES = 16;
    static constexpr size_t ALL_ALLIES = 17;
    class Stats;
    class Battle {
    public:
      class Entity {
      public:
        Pips pips = {0, 0};
        int initiative = 0;
        size_t stunRounds = 0;
        const Stats* stats = nullptr; // if nullptr, no one here
        mpz_class health = 0;
        size_t mana = 0;
        uint32_t playerID = 0; // 0 if enemy
      };
      std::array<Entity, 2 * PLAYERS_PER_SIDE> players;
      void damage(
        size_t attacker, size_t defender,
        size_t school, const mpz_class& amt,
        RakNet::BitStream& out, size_t& nPacts);
    };
  }
}