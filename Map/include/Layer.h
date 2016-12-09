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
    class Layer {
    public:
      Layer(int w, int h, int xoff = 0, int yoff = 0) :
          width(w), height(h), xoff(xoff), yoff(yoff) {
        allocateBlocks();
      }
      Layer(std::istream& handle);
      void write(std::ostream& handle);
      ~Layer();
      int getWidth() { return width; }
      int getHeight() { return height; }
      int getXOffset() { return xoff; }
      int getYOffset() { return yoff; }
      Block* getMapBlocks() { return map; }
      // NB: (x, y) is the northwest corner of the block
      Block getMapBlockAt(int x, int y);
      Block getMapBlockAtRaw(int x, int y);
      void setMapBlockAt(int x, int y, Block b);
      void setMapBlockAtRaw(int x, int y, Block b);
      Layer(const Layer& that) :
          width(that.width), height(that.height),
          xoff(that.xoff), yoff(that.yoff) {
        allocateBlocks();
        memcpy(map, that.map, width * height * sizeof(Block));
      }
      void operator=(const Layer& that);
    private:
      void allocateBlocks();
      int width, height;
      // These are the coordinates of the northwest corner of the map.
      // This is useful if you later want to expand the map in that direction
      // without having players appear to move.
      int xoff, yoff;
      // The elements are in row-major order.
      Block* map;
    };
  }
}
