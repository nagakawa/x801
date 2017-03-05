#include "Chunk.h"

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

using namespace x801::map;

#include <assert.h>
#include <stdint.h>

x801::map::Chunk::~Chunk() {
  delete[] map;
}

Block x801::map::Chunk::getMapBlockAt(size_t ix, size_t iy, size_t iz) {
  if (empty) return Block();
  size_t index = (iz << 8) | (ix << 4) | iy;
  assert(index < BLOCKS_IN_CHUNK);
  return map[index];
}

void x801::map::Chunk::setMapBlockAt(size_t ix, size_t iy, size_t iz, Block b) {
  if (empty) {
    empty = false;
    map = new Block[BLOCKS_IN_CHUNK];
  }
  size_t index = (iz << 8) | (ix << 4) | iy;
  assert(index < BLOCKS_IN_CHUNK);
  map[index] = b;
}

/*x801::map::Chunk::Chunk(std::istream& handle) {
  width = x801::base::readInt<uint16_t>(handle);
  height = x801::base::readInt<uint16_t>(handle);
  xoff = x801::base::readInt<int16_t>(handle);
  yoff = x801::base::readInt<int16_t>(handle);
  allocateBlocks();
  for (int i = 0; i < width * height; ++i) {
    map[i] = Block(x801::base::readInt<uint32_t>(handle));
  }
}*/

void x801::map::Chunk::write(std::ostream& handle) const {
  x801::base::writeInt<int16_t>(handle, x);
  x801::base::writeInt<int16_t>(handle, y);
  x801::base::writeInt<int16_t>(handle, z);
  x801::base::writeInt<uint16_t>(handle, empty);
  for (size_t i = 0; i < BLOCKS_IN_CHUNK; ++i) {
    x801::base::writeInt<uint32_t>(handle, map[i].label);
  }
}

Chunk& x801::map::Chunk::operator=(const Chunk& that) {
  x = that.x;
  y = that.y;
  z = that.z;
  empty = that.empty;
  if (!empty && map == nullptr) map = new Block[BLOCKS_IN_CHUNK];
  else map = nullptr;
  if (!empty)
    memcpy(map, that.map, BLOCKS_IN_CHUNK * sizeof(Block));
  return *this;
}
