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

#include <stdint.h>
#include <iostream>
#include <map>
#include <vector>
#include "Chunk.h"

namespace x801 {
  namespace map {
    struct BlockXYZ {
      int x, y, z;
      BlockXYZ(int x, int y, int z) :
        x(x), y(y), z(z) {}
      BlockXYZ() : x(0), y(0), z(0) {}
      ChunkXYZ home() const {
        return ChunkXYZ(x >> 4, y >> 4, z >> 4);
      }
      BlockXYZ chunkLocal() const {
        return BlockXYZ(x & 15, y & 15, z & 15);
      }
      bool operator==(const BlockXYZ& that) const {
        return x == that.x && y == that.y && z == that.z;
      }
      bool operator<(const BlockXYZ& that) const {
        if (x < that.x) return true;
        if (x > that.x) return false;
        if (y < that.y) return true;
        if (y > that.y) return false;
        if (z < that.z) return true;
        if (z > that.z) return false;
        return false;
      }
    };
    struct BlockHasher {
      size_t operator()(const BlockXYZ& xyz) const {
        size_t hx = std::hash<int>{}(xyz.x);
        size_t hy = std::hash<int>{}(xyz.y);
        size_t hz = std::hash<int>{}(xyz.z);
        return hx ^ (hy << 1) ^ (hz << 2);
      }
    };
    class TileSec {
    public:
      TileSec(std::istream& fh);
      void write(std::ostream& fh) const;
      TileSec(const TileSec&& that) :
        chunks(std::move(that.chunks)) {}
      TileSec& operator=(const TileSec&& that);
      Block getBlock(const BlockXYZ& xyz);
      void setBlock(const BlockXYZ& xyz, Block b);
      void createChunk(const ChunkXYZ& xyz);
      void createChunk(std::istream& fh);
    private:
      std::map<ChunkXYZ, Chunk> chunks;
      //int layerCount;
      //std::vector<Layer> layers;
    };
  }
}
