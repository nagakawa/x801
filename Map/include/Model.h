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
    // This is used to check collisions.
    enum HitboxType {
      HITBOX_NONE = 0,
      HITBOX_FULL = 1,
      HITBOX_TOP_HALF = 2,
      HITBOX_BOTTOM_HALF = 3,
    };
    // Fun fact:
    // dir ^ 1 gives the direction opposite of the one provided.
    // dir1 ^ dir2 is 1 iff dir1 is the opposite direction as dir2.
    enum Direction {
      UP = 0,
      DOWN,
      NORTH,
      SOUTH,
      EAST,
      WEST
    };
    extern const char* DIRECTION_NAMES[6];
    // XYZ are represented as follows:
    // 0 represents the center of the block.
    // +/- 64 represents 0.5 block units.
    // This gives a precision of 1/128 of a block unit.
    struct VertexXYZ {
      int8_t x, y, z;
    };
    // UV are represented as follows:
    // 0 is the left (top) of a single texture.
    // 128 is the right (bottom) edge.
    // index is the index of the vertex in the `vertices` array.
    struct VertexIUV {
      uint16_t index;
      uint8_t u, v;
    };
    // `vertices` stores the three vertices of a face.
    // `texture` is the index to the `textures` array in a ModelApplication.
    // `occlusionFlags`:
    // for i in 0 to 5:
    // bit i is set iff this face should be occluded if there is a block in
    // direction i that is opaque in the (i ^ 1)-direction
    // bit 6 is set iff this face is partially transparent
    struct Face {
      VertexIUV vertices[3];
      uint8_t texture;
      uint8_t occlusionFlags;
    };
    class ModelFunction {
    public:
      ModelFunction(std::istream& fh);
      void write(std::ostream& fh);
      // hitbox type (see above)
      uint16_t hitboxType;
      // bit i is set iff this block is opaque in direction i
      uint8_t opacityFlags;
      // number of texture parameters this model function takes
      uint8_t textureCount;
      // vertices
      std::vector<VertexXYZ> vertices;
      // faces
      std::vector<Face> faces;
    };
    // Stores some number of models.
    // Think of it as a glorified vector<ModelFunction>.
    class ModelFunctionIndex {
    public:
      ModelFunctionIndex(std::istream& fh);
      void write(std::ostream& fh);
      std::vector<ModelFunction> models;
      int error;
      x801::base::Version version;
    };
    // Describes a block model plus the textures to apply to it.
    class ModelApplication {
    public:
      ModelApplication(std::istream& fh);
      void write(std::ostream& fh);
      // The index of the model function to use from the MFI.
      uint32_t modfnum;
      // The textures to use. These are extracted from
      // blocks.png.
      std::vector<uint32_t> textures;
    };
    // Stores some number of applications.
    // Think of it as a glorified vector<ModelApplication>.
    class ModelApplicationIndex {
    public:
      ModelApplicationIndex(std::istream& fh);
      void write(std::ostream& fh);
      std::vector<ModelApplication> applications;
      int error;
      x801::base::Version version;
    };
  }
}