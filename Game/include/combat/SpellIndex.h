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
        uint32_t address;
        uint16_t accuracy; // 0.1%s
        // XXX HARD LIMIT of 256 schools for alignment purposes;
        // change if this is exceeded
        uint8_t school;
        uint8_t cost;
      }; // 8 bytes total
      size_t getIDByName(const char* s) const;
      size_t getIDByName(const std::string& s) const;
      const Metadata& getMetadata(size_t id) const;
      mpz_class evaluateQuantity(uint32_t q) const;
    private:
      std::unordered_map<std::string, size_t> idsByName;
      std::vector<Metadata> metadata;
      std::vector<uint32_t> quantities;
      std::vector<uint32_t> steps;
    };
  }
}