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

#include <iostream>

namespace x801 {
  namespace map {
    struct Block {
      uint32_t label;
    };
    class Area {
    public:
      Area(int w, int h, int xoff = 0, int yoff = 0) :
          width(w), height(h), xoff(xoff), yoff(yoff) {
        allocateBlocks();
      }
      // Area(std::istream& handle);
      // void write(std::ostream& handle);
      ~Area();
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
      // Don't define these for now; maybe they can be defined later.
      Area(const Area& that) = delete;
      void operator=(const Area& that) = delete;
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
