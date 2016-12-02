#include "Area.h"

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

x801::map::Area::~Area() {
  delete[] map;
}

void x801::map::Area::allocateBlocks() {
  map = new Block[width * height];
}

Block x801::map::Area::getMapBlockAt(int x, int y) {
  int xtrans = x - xoff;
  int ytrans = y - yoff;
  int index = ytrans * width + xtrans;
  return map[index];
}

Block x801::map::Area::getMapBlockAtRaw(int x, int y) {
  int index = y * width + x;
  return map[index];
}

void x801::map::Area::setMapBlockAt(int x, int y, Block b) {
  int xtrans = x - xoff;
  int ytrans = y - yoff;
  int index = ytrans * width + xtrans;
  map[index] = b;
}

void x801::map::Area::setMapBlockAtRaw(int x, int y, Block b) {
  int index = y * width + x;
  map[index] = b;
}
