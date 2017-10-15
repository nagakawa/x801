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

#include <assert.h>
#include <stdint.h>
#include <limits>

namespace x801 {
  namespace map {
    Chunk::~Chunk() {
      delete[] map;
    }
    
    Block Chunk::getMapBlockAt(size_t ix, size_t iy) const {
      if (empty) return Block();
      size_t index = (ix << 4) | iy;
      assert(index < BLOCKS_IN_CHUNK);
      return map[index];
    }
    
    void Chunk::setMapBlockAt(size_t ix, size_t iy, Block b) {
      if (empty) {
        empty = false;
        map = new Block[BLOCKS_IN_CHUNK];
      }
      size_t index = (ix << 4) | iy;
      assert(index < BLOCKS_IN_CHUNK);
      map[index] = b;
    }
    
    Chunk::Chunk(std::istream& handle) {
      xyz.x = x801::base::readInt<int16_t>(handle);
      xyz.y = x801::base::readInt<int16_t>(handle);
      xyz.z = x801::base::readInt<int16_t>(handle);
      empty = x801::base::readInt<uint16_t>(handle);
      map = empty ? nullptr : new Block[BLOCKS_IN_CHUNK];
      if (empty) return;
      // std::cout << '(' << xyz.x << ", " << xyz.y << ", " << xyz.z << ") ";
      for (size_t i = 0; i < BLOCKS_IN_CHUNK; ++i) {
        map[i] = Block(x801::base::readInt<uint32_t>(handle));
        // std::cout << map[i].label << ' ';
      }
      // std::cout << '\n';
    }
    
    void Chunk::write(std::ostream& handle) const {
      x801::base::writeInt<int16_t>(handle, xyz.x);
      x801::base::writeInt<int16_t>(handle, xyz.y);
      x801::base::writeInt<int16_t>(handle, xyz.z);
      x801::base::writeInt<uint16_t>(handle, empty);
      for (size_t i = 0; i < BLOCKS_IN_CHUNK; ++i) {
        x801::base::writeInt<uint32_t>(handle, map[i].label);
      }
    }
    
    Chunk& Chunk::operator=(const Chunk& that) {
      xyz = that.xyz;
      empty = that.empty;
      if (!empty && map == nullptr) map = new Block[BLOCKS_IN_CHUNK];
      else {
        delete[] map;
        map = nullptr;
      }
      if (!empty)
        memcpy(map, that.map, BLOCKS_IN_CHUNK * sizeof(Block));
      return *this;
    }
    
    Chunk& Chunk::operator=(Chunk&& that) {
      xyz = that.xyz;
      empty = that.empty;
      map = that.map;
      that.map = nullptr;
      that.empty = true;
      return *this;
    }
    
    static constexpr size_t BLOCK_ID_LIMIT = 65535;

    BlockTextureBindings::BlockTextureBindings(std::istream& fh) {
      unsigned int key, value;
      while (!fh.eof() && !fh.bad()) {
        fh >> key >> value;
        if (fh.fail()) {
          fh.clear();
          fh.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          continue;
        }
        if (key > BLOCK_ID_LIMIT) {
          std::cerr << "Block ID limit exceeded in BlockTextureBindings!\n";
          std::cerr << "Limit is " << BLOCK_ID_LIMIT;
          std::cerr << " but received " << key << "!\n";
          exit(-1);
        }
        texIDsByBlockID.resize(key + 1);
        texIDsByBlockID[key] = value;
      }
    }
    EntityTextureBindings::EntityTextureBindings(std::istream& fh) {
      std::string key;
      unsigned int value;
      while (!fh.eof() && !fh.bad()) {
        fh >> key >> value;
        if (fh.fail()) {
          fh.clear();
          fh.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
          continue;
        }
        texIDsByEntityID[key] = value;
      }
    }
    std::ostream& operator<<(
        std::ostream& s,
        const BlockTextureBindings& b) {
      s << "[";
      for (size_t i = 0; i < b.count(); ++i) {
        if (i != 0) s << ", ";
        size_t realid = i + 1;
        s << realid << " -> " << b.getTexID(realid);
      }
      s << "]";
      return s;
    }
    std::ostream& operator<<(
        std::ostream& s,
        const EntityTextureBindings& b) {
      s << "[";
      bool first = true;
      for (const auto& p : b.texIDsByEntityID) {
        if (first)
          first = false;
        else
          std::cout << ", ";
        s << p.first << " -> " << p.second;
      }
      s << "]";
      return s;
    }
  }
}
