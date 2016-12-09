#include "Layer.h"

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

#include <stdint.h>
#include <utils.h>

x801::map::Layer::~Layer() {
  delete[] map;
}

void x801::map::Layer::allocateBlocks() {
  map = new Block[width * height];
}

Block x801::map::Layer::getMapBlockAt(int x, int y) {
  int xtrans = x - xoff;
  int ytrans = y - yoff;
  int index = ytrans * width + xtrans;
  return map[index];
}

Block x801::map::Layer::getMapBlockAtRaw(int x, int y) {
  int index = y * width + x;
  return map[index];
}

void x801::map::Layer::setMapBlockAt(int x, int y, Block b) {
  int xtrans = x - xoff;
  int ytrans = y - yoff;
  int index = ytrans * width + xtrans;
  map[index] = b;
}

void x801::map::Layer::setMapBlockAtRaw(int x, int y, Block b) {
  int index = y * width + x;
  map[index] = b;
}

x801::map::Layer::Layer(std::istream& handle) {
  width = x801::base::readInt<uint16_t>(handle);
  height = x801::base::readInt<uint16_t>(handle);
  xoff = x801::base::readInt<int16_t>(handle);
  yoff = x801::base::readInt<int16_t>(handle);
  allocateBlocks();
  for (int i = 0; i < width * height; ++i) {
    map[i] = Block(x801::base::readInt<uint32_t>(handle));
  }
}

void x801::map::Layer::write(std::ostream& handle) {
  x801::base::writeInt<uint16_t>(handle, width);
  x801::base::writeInt<uint16_t>(handle, height);
  x801::base::writeInt<int16_t>(handle, xoff);
  x801::base::writeInt<int16_t>(handle, yoff);
  for (int i = 0; i < width * height; ++i) {
    x801::base::writeInt<uint32_t>(handle, map[i].label);
  }
}

void x801::map::Layer::operator=(const Layer& that) {
  width = that.width;
  height = that.height;
  xoff = that.xoff;
  yoff = that.yoff;
  delete[] map;
  allocateBlocks();
  memcpy(map, that.map, width * height * sizeof(Block));
}
