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

#include <BitStream.h>

#include "utils.h"
#include "combat/Battle.h"

#define NEXTQTY quantities.at(q++)
#define NEXTSTEP steps.at(q++) // No pun intended

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
    enum class StepType : uint_fast16_t {
      /*
        Describes damage done to a player.
        [0] school of damage
        [1] amount of damage (quantity)
        [2] target
          0 = enemy target
          1 = ally target
          2 = all enemies
          3 = all allies
          4 = self
      */
      damage = 0,
      /*
        Describes healing done to a player.
        [0] amount of healing (quantity)
        [1] target (see above)
      */
      healing = 1,
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
        m.nSteps = readInt<uint8_t>(fh);
        m.pad = readInt<uint8_t>(fh);
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
    inline bool isDirectQuantity(uint32_t q) {
      // If the 2 most significant bits are 00 or 11,
      // then this is a direct quantity.
      uint32_t top = q & 0xC000'0000;
      return (top == 0x0000'0000 || top == 0xC000'0000);
    }
    mpz_class SpellIndex::evaluateQuantity(
        uint32_t q, std::mt19937& r) const {
      if (isDirectQuantity(q)) {
        return mpz_class((int32_t) q);
      }
      // If not a direct quantity, look up the address.
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
    std::string SpellIndex::quantityToString(uint32_t q) const {
      std::string s;
      quantityToString(q, s);
      return s;
    }
    void SpellIndex::quantityToString(uint32_t q, std::string& s) const {
      if (isDirectQuantity(q)) {
        s += std::to_string((int32_t) q);
        return;
      }
      q -= (1 << 30);
      uint_fast32_t typemeta = NEXTQTY;
      QuantityType type = QuantityType(typemeta & 0xFFFF);
      uint_fast16_t meta = (typemeta >> 16) & 0xFFFF;
      switch (type) {
        // Insert quantity logic here.
        case QuantityType::range: {
          uint_fast32_t min = NEXTQTY;
          uint_fast32_t max = NEXTQTY;
          s += std::to_string(min);
          s += u8" \u2013 ";
          s += std::to_string(max);
          break;
        }
      }
      (void) meta;
      return;
    }
    bool SpellIndex::actuateStep(
        uint32_t& address, Battle& b,
        size_t i, RakNet::BitStream& out,
        std::mt19937& r) const {
      uint32_t q = address;
      uint_fast32_t typemeta = NEXTSTEP;
      StepType type = StepType(typemeta & 0xFFFF);
      uint_fast16_t meta = (typemeta >> 16) & 0xFFFF;
      switch (type) {
        case StepType::damage: {
          //
          break;
        }
        case StepType::healing: {
          //
          break;
        }
      }
      (void) meta; (void) b; (void) i; (void) out; (void) r;
      return true;
    }
    bool SpellIndex::actuate(
        uint32_t sid, Battle& b,
        size_t i, RakNet::BitStream& out,
        std::mt19937& r) const {
      if (sid >= metadata.size()) return false;
      // Information about all charms invoked before casting.
      // (none as of now)
      out.Write((uint32_t) 0);
      const Metadata& md = metadata[sid];
      // Write school of spell
      out.Write((uint16_t) md.school);
      std::uniform_int_distribution<unsigned int> dist(0U, 999U);
      if (dist(r) >= md.accuracy) { // Fizzle
        out.Write((uint8_t) 0);
        return true;
      }
      // By here, the spell succeeded.
      // Information about all charms invoked during casting.
      // (none as of now)
      out.Write((uint32_t) 0);
      uint32_t offset = md.address;
      for (size_t j = 0; j < md.nSteps; ++j) {
        bool res = actuateStep(offset, b, i, out, r);
        if (!res) return false;
      }
      return true;
    }
  }
}