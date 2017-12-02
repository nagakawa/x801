#include "combat/SpellIndex.h"

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

#include "utils.h"

#define NEXTQTY quantities.at(q++)

namespace x801 {
  namespace game {
    enum class QuantityType : uint_fast16_t {
      /*
        Describes a a range of damage.
        [0] minimum value (direct 32-bit)
        [1] maximum value (direct 32-bit)
      */
      range = 0,
    };
    SpellIndex::SpellIndex(std::istream& fh) {
      using namespace x801::base;
      size_t count = readInt<uint32_t>(fh);
      for (size_t i = 0; i < count; ++i) {
        std::string name = readString<uint16_t>(fh);
        size_t id = readInt<uint32_t>(fh);
        idsByName.insert(
          std::pair<std::string, size_t>(std::move(name), id));
      }
      count = readInt<uint32_t>(fh);
      metadata.resize(count);
      for (size_t i = 0; i < count; ++i) {
        Metadata& m = metadata[i];
        m.address = readInt<uint32_t>(fh);
        m.minSpeed = readInt<int16_t>(fh);
        m.maxSpeed = readInt<int16_t>(fh);
        m.accuracy = readInt<uint16_t>(fh);
        m.school = readInt<uint16_t>(fh);
        m.cost = readInt<uint8_t>(fh);
        m.flags = readInt<uint8_t>(fh);
        m.pad = readInt<uint16_t>(fh);
      }
      count = readInt<uint32_t>(fh);
      quantities.resize(count);
      for (size_t i = 0; i < count; ++i) {
        quantities[i] = readInt<uint32_t>(fh);
      }
      count = readInt<uint32_t>(fh);
      steps.resize(count);
      for (size_t i = 0; i < count; ++i) {
        steps[i] = readInt<uint32_t>(fh);
      }
    }
    const SpellIndex::Metadata& SpellIndex::getMetadata(size_t id) const {
      return metadata[id];
    }
    size_t SpellIndex::getIDByName(const char* s) const {
      auto it = idsByName.find(s);
      return (it != idsByName.end()) ? it->second : -1U;
    }
    size_t SpellIndex::getIDByName(const std::string& s) const {
      auto it = idsByName.find(s);
      return (it != idsByName.end()) ? it->second : -1U;
    }
    mpz_class SpellIndex::evaluateQuantity(
        uint32_t q, std::mt19937& r) const {
      uint32_t top = q & 0xC000'0000;
      // If the 2 most significant bits are 00 or 11,
      // then this is a direct quantity.
      if (top == 0x0000'0000 || top == 0xC000'0000) {
        return mpz_class((int32_t) q);
      }
      // Otherwise, look up the address.
      q -= (1 << 30);
      uint_fast32_t typemeta = NEXTQTY;
      QuantityType type = QuantityType(typemeta & 0xFFFF);
      uint_fast16_t meta = (typemeta >> 16) & 0xFFFF;
      switch (type) {
        // Insert quantity logic here.
        case QuantityType::range: {
          uint_fast32_t min = NEXTQTY;
          uint_fast32_t max = NEXTQTY;
          std::uniform_int_distribution<uint_fast32_t> dist(min, max);
          return dist(r);
        }
      }
      (void) meta;
      return 0;
    }
  }
}