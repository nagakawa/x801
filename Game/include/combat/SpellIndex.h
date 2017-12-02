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
#include <random>
#include <unordered_map>
#include <vector>

#include <gmpxx.h>

#include "combat/School.h"

namespace x801 {
  namespace game {
    class SpellIndex {
    public:
      SpellIndex(std::istream& fh);
      struct Metadata {
        uint32_t address; // TODO should we give up fixed mdentry size?
        int16_t minSpeed;
        int16_t maxSpeed;
        uint16_t accuracy; // 0.1%s
        uint16_t school;
        uint8_t cost;
        /*
          bit 0: should player choose single enemy?
          bit 1: should player choose single ally?
          other bits unused
        */
        uint8_t flags;
        uint16_t pad;
      }; // 16 bytes total
      size_t getIDByName(const char* s) const;
      size_t getIDByName(const std::string& s) const;
      const Metadata& getMetadata(size_t id) const;
      mpz_class evaluateQuantity(
        uint32_t q, std::mt19937& r) const;
    private:
      std::unordered_map<std::string, size_t> idsByName;
      std::vector<Metadata> metadata;
      std::vector<uint32_t> quantities;
      std::vector<uint32_t> steps;
    };
  }
}