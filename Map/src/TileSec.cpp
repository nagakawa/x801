#include "TileSec.h"

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

#include <utils.h>

#pragma GCC diagnostic push                // we DO want an explicit ctor
#pragma GCC diagnostic ignored "-Weffc++"  // since it has complex behaviour
x801::map::TileSec::TileSec(std::istream& fh) {
  size_t chunkCount = x801::base::readInt<uint16_t>(fh);
  for (size_t i = 0; i < chunkCount; ++i) {
    createChunk(fh);
  }
}
#pragma GCC diagnostic pop

void x801::map::TileSec::write(std::ostream& fh) const {
  x801::base::writeInt<uint16_t>(fh, chunks.size());
  for (auto& ci : chunks) {
    ci.second.write(fh);
  }
}

TileSec& x801::map::TileSec::operator=(const TileSec&& that) {
  chunks = std::move(that.chunks);
  return *this;
}

Block x801::map::TileSec::getBlock(const BlockXYZ& xyz) {
  auto ci = chunks.find(xyz.home());
  if (ci == chunks.end()) return Block();
  Chunk& c = ci->second;
  BlockXYZ clc = xyz.chunkLocal();
  return c.getMapBlockAt(clc.x, clc.y, clc.z);
}

void x801::map::TileSec::setBlock(const BlockXYZ& xyz, Block b) {
  auto ci = chunks.find(xyz.home());
  if (ci == chunks.end()) {
    createChunk(xyz.home());
  }
  Chunk& c = ci->second;
  BlockXYZ clc = xyz.chunkLocal();
  c.setMapBlockAt(clc.x, clc.y, clc.z, b);
}

void x801::map::TileSec::createChunk(const ChunkXYZ& xyz) {
  chunks.emplace(
    std::piecewise_construct,
    std::forward_as_tuple(xyz),
    std::forward_as_tuple(xyz));
}

void x801::map::TileSec::createChunk(std::istream& fh) {
  Chunk c(fh);
  chunks.insert({c.getXYZ(), std::move(c)});
}