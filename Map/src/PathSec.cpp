#include "PathSec.h"

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

#include <stdint.h>
#include <algorithm>
#include <iostream>
#include <limits>

namespace x801 {
  namespace map {
    Path::Path(std::istream& fh) {
      using namespace x801::base;
      minX = minY = std::numeric_limits<int16_t>::max();
      maxX = maxY = std::numeric_limits<int16_t>::min();
      z = readInt<int8_t>(fh);
      size_t nodeCount = readInt<uint8_t>(fh);
      size_t mobCount = readInt<uint8_t>(fh);
      size_t bossCount = readInt<uint8_t>(fh);
      for (size_t i = 0; i < nodeCount; ++i) {
        int16_t x = readInt<int16_t>(fh);
        int16_t y = readInt<int16_t>(fh);
        vertices.push_back({x, y});
        minX = std::min(minX, x);
        maxX = std::max(maxX, x);
        minY = std::min(minY, y);
        maxY = std::max(maxY, y);
      }
      for (size_t i = 0; i < mobCount; ++i) {
        weights.push_back(readInt<uint8_t>(fh));
        mobNames.push_back(readString<uint16_t>(fh));
      }
      for (size_t i = 0; i < bossCount; ++i) {
        bossNames.push_back(readString<uint16_t>(fh));
      }
    }
    void Path::write(std::ostream& fh) const {
      using namespace x801::base;
      writeInt<int8_t>(fh, z);
      writeInt<uint8_t>(fh, vertices.size());
      writeInt<uint8_t>(fh, mobNames.size());
      writeInt<uint8_t>(fh, bossNames.size());
      for (const Vertex& v : vertices) {
        writeInt<int16_t>(fh, v.x);
        writeInt<int16_t>(fh, v.y);
      }
      for (size_t i = 0; i < mobNames.size(); ++i) {
        writeInt<uint8_t>(fh, weights[i]);
        writeString<uint16_t>(fh, mobNames[i]);
      }
      for (const std::string& n : bossNames) {
        writeString<uint16_t>(fh, n);
      }
    }
    PathSec::PathSec(std::istream& fh) {
      using namespace x801::base;
      size_t count = readInt<uint16_t>(fh);
      for (size_t i = 0; i < count; ++i) {
        paths.emplace_back(fh);
      }
      present = true;
    }
    void PathSec::write(std::ostream& fh) const {
      using namespace x801::base;
      writeInt<uint16_t>(fh, paths.size());
      for (const Path& p : paths) {
        p.write(fh);
      }
    }
  }
}