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

#include <string.h>
#include <iostream>
#include <utils.h>

namespace x801 {
  namespace map {
    struct Block {
      Block() : label(0) {}
      Block(uint32_t b) : label(b) {}
      uint32_t label;
      bool isSolid() {
        return label >> 31;
      }
      bool getBlockID() {
        return label & 0xffffff;
      }
      bool getElevation() {
        return (label >> 25) & 63;
      }
      bool operator==(Block other) {
        return label == other.label;
      }
    };
    const size_t BLOCKS_IN_CHUNK = 16 * 16 * 16;
    class Chunk {
    public:
      Chunk(int x, int y, int z) :
          x(x), y(y), z(z), empty(true),
          map(nullptr) {}
      Chunk(std::istream& handle) :
          x(x801::base::readInt<int16_t>(handle)),
          y(x801::base::readInt<int16_t>(handle)),
          z(x801::base::readInt<int16_t>(handle)),
          empty(x801::base::readInt<uint16_t>(handle)),
          map(new Block[BLOCKS_IN_CHUNK]) {
        for (size_t i = 0; i < BLOCKS_IN_CHUNK; ++i) {
          map[i] = Block(x801::base::readInt<uint32_t>(handle));
        }
      }
      void write(std::ostream& handle) const;
      ~Chunk();
      int getX() { return x; }
      int getY() { return y; }
      int getZ() { return z; }
      bool isEmpty() { return empty; }
      Block* getMapBlocks() { return map; }
      Block getMapBlockAt(size_t ix, size_t iy, size_t iz);
      void setMapBlockAt(size_t ix, size_t iy, size_t iz, Block b);
      Chunk(const Chunk& that) :
          x(that.x), y(that.y), z(that.z), empty(that.empty),
          map(that.empty ? nullptr : new Block[BLOCKS_IN_CHUNK]) {
        if (!that.empty)
          memcpy(map, that.map, BLOCKS_IN_CHUNK * sizeof(Block));
      }
      Chunk& operator=(const Chunk& that);
    private:
      int x, y, z;
      bool empty;
      // The elements are in row-major order.
      Block* map;
    };
  }
}
