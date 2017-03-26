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
#include <string.h>
#include <functional>
#include <iostream>
#include <vector>
#include <utils.h>
#include <Version.h>

namespace x801 {
  namespace map {
    enum HitboxType {
      HITBOX_NONE = 0,
      HITBOX_FULL = 1,
      HITBOX_TOP_HALF = 2,
      HITBOX_BOTTOM_HALF = 3,
    };
    enum Direction {
      UP = 0,
      DOWN,
      NORTH,
      SOUTH,
      EAST,
      WEST
    };
    struct VertexXYZ {
      int8_t x, y, z;
    };
    struct VertexIUV {
      uint16_t index;
      uint8_t u, v;
    };
    struct Face {
      VertexIUV vertices[3];
      uint8_t texture;
      uint8_t occlusionFlags;
    };
    class ModelFunction {
    public:
      ModelFunction(std::istream& fh);
      void write(std::ostream& fh);
      uint16_t hitboxType;
      uint8_t opacityFlags;
      uint8_t textureCount;
      std::vector<VertexXYZ> vertices;
      std::vector<Face> faces;
    };
    class ModelFunctionIndex {
    public:
      ModelFunctionIndex(std::istream& fh);
      void write(std::ostream& fh);
      std::vector<ModelFunction> models;
      int error;
      x801::base::Version version;
    };
  }
}