#include "Model.h"

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

#include "mapErrors.h"

using namespace x801::map;

x801::map::ModelFunction::ModelFunction(std::istream& fh) {
  hitboxType = x801::base::readInt<uint16_t>(fh);
  char c;
  fh.get(c);
  opacityFlags = (uint8_t) c;
  fh.get(c);
  textureCount = (uint8_t) c;
  uint16_t vertexCount = x801::base::readInt<uint16_t>(fh);
  uint16_t faceCount = x801::base::readInt<uint16_t>(fh);
  for (size_t i = 0; i < vertexCount; ++i) {
    fh.get(c);
    int8_t x = (int8_t) c;
    fh.get(c);
    int8_t y = (int8_t) c;
    fh.get(c);
    int8_t z = (int8_t) c;
    vertices.push_back({ x, y, z });
  }
  for (size_t i = 0; i < faceCount; ++i) {
    Face f;
    for (size_t i = 0; i < 3; ++i) {
      f.vertices[i].index = x801::base::readInt<uint16_t>(fh);
    }
    for (size_t i = 0; i < 3; ++i) {
      fh.get(c);
      f.vertices[i].u = (uint8_t) c;
      fh.get(c);
      f.vertices[i].v = (uint8_t) c;
    }
    fh.get(c);
    f.texture = (uint8_t) c;
    fh.get(c);
    f.occlusionFlags = (uint8_t) c;
    faces.push_back(f);
  }
}

void x801::map::ModelFunction::write(std::ostream& fh) {
  x801::base::writeInt<uint16_t>(fh, hitboxType);
  fh.put((char) opacityFlags);
  fh.put((char) textureCount);
  x801::base::writeInt<uint16_t>(fh, vertices.size());
  x801::base::writeInt<uint16_t>(fh, faces.size());
  for (size_t i = 0; i < vertices.size(); ++i) {
    fh.put((char) vertices[i].x);
    fh.put((char) vertices[i].y);
    fh.put((char) vertices[i].z);
  }
  for (size_t i = 0; i < faces.size(); ++i) {
    Face& f = faces[i];
    for (size_t i = 0; i < 3; ++i) {
      x801::base::writeInt<uint16_t>(fh, f.vertices[i].index);
    }
    for (size_t i = 0; i < 3; ++i) {
      fh.put((char) f.vertices[i].u);
      fh.put((char) f.vertices[i].v);
    }
    fh.put((char) f.texture);
    fh.put((char) f.occlusionFlags);
  }
}

x801::map::ModelFunctionIndex::ModelFunctionIndex(std::istream& fh) {
  int header = x801::base::readInt<uint32_t>(fh);
  if (header != 0x46444d58L) { // XMDF
    error = MODERR_NOT_A_MODEL;
  }
  version = x801::base::Version(fh);
  uint32_t count = x801::base::readInt<uint32_t>(fh);
  for (size_t i = 0; i < count; ++i) {
    models.emplace_back(fh);
  }
}

void x801::map::ModelFunctionIndex::write(std::ostream& fh) {
  x801::base::writeInt<uint32_t>(fh, 0x46444d58L);
  x801::base::engineVersion.write(fh);
  x801::base::writeInt<uint32_t>(fh, models.size());
  for (size_t i = 0; i < models.size(); ++i) {
    models[i].write(fh);
  }
}

x801::map::ModelApplication::ModelApplication(std::istream& fh) {
  modfnum = x801::base::readInt<uint32_t>(fh);
  char c;
  fh.get(c);
  uint8_t numTextures = (uint8_t) c;
  for (size_t i = 0; i < numTextures; ++i)
    textures.push_back(x801::base::readInt<uint32_t>(fh));
}

void x801::map::ModelApplication::write(std::ostream& fh) {
  x801::base::writeInt<uint32_t>(fh, modfnum);
  if (textures.size() >= 256) throw "too many texture parameters";
  fh.put((char) textures.size());
  for (uint32_t t : textures)
    x801::base::writeInt<uint32_t>(fh, t);
}

x801::map::ModelApplicationIndex::ModelApplicationIndex(std::istream& fh) {
  int header = x801::base::readInt<uint32_t>(fh);
  if (header != 0x41444d58L) { // XMDA
    error = MODERR_NOT_A_MODEL;
  }
  version = x801::base::Version(fh);
  uint32_t count = x801::base::readInt<uint32_t>(fh);
  for (size_t i = 0; i < count; ++i) {
    applications.emplace_back(fh);
  }
}

void x801::map::ModelApplicationIndex::write(std::ostream& fh) {
  x801::base::writeInt<uint32_t>(fh, 0x41444d58L);
  x801::base::engineVersion.write(fh);
  x801::base::writeInt<uint32_t>(fh, applications.size());
  for (size_t i = 0; i < applications.size(); ++i) {
    applications[i].write(fh);
  }
}

const char* x801::map::DIRECTION_NAMES[6] = {
  "UP", "DOWN", "NORTH", "SOUTH", "EAST", "WEST"
};