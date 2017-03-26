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