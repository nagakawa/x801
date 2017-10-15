#include "POISec.h"

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

namespace x801 {
  namespace map {
    POISec::POISec(std::istream& fh) {
      using namespace x801::base;
      present = true;
      size_t count = readInt<uint16_t>(fh);
      for (size_t i = 0; i < count; ++i) {
        POI p;
        p.x = readInt<int16_t>(fh);
        p.y = readInt<int16_t>(fh);
        p.z = readInt<int8_t>(fh);
        pois.push_back(p);
        bool hasEntity = readInt<uint8_t>(fh);
        if (hasEntity) {
          std::string texname = readString<uint16_t>(fh);
          uint8_t offset = readInt<uint8_t>(fh);
          std::string title = readString<uint16_t>(fh);
          std::string name = readString<uint16_t>(fh);
          entityPOIs.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(i),
            std::forward_as_tuple(texname, offset, title, name)
          );
        }
      }
    }
    void POISec::write(std::ostream& fh) const {
      using namespace x801::base;
      size_t count = pois.size();
      writeInt<uint16_t>(fh, count);
      for (size_t i = 0; i < count; ++i) {
        const POI& p = pois[i];
        writeInt<int16_t>(fh, p.x);
        writeInt<int16_t>(fh, p.y);
        writeInt<int8_t>(fh, p.z);
        const auto it = entityPOIs.find(i);
        bool hasEntity = it != entityPOIs.end();
        writeInt<uint8_t>(fh, hasEntity);
        if (hasEntity) {
          const EntityPOI& ep = it->second;
          writeString<uint16_t>(fh, ep.texname);
          writeInt<uint8_t>(fh, ep.offset);
          writeString<uint16_t>(fh, ep.title);
          writeString<uint16_t>(fh, ep.name);
        }
      }
    }
  }
}