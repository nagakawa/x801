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

#include <memory>

#include <FBO.h>
#include <Texture.h>

#include <Area.h>
#include <Chunk.h>
#include <Model.h>

#include "Client.h"
#include "GameState.h"
#include "window/ClientWindow.h"
#include "window/Patcher.h"
#include "window/patcher_views/ModelView.h"
#include "window/patcher_views/TextureView.h"

namespace x801 {
  namespace game {
    class TerrainRenderer;
    struct CMVertex {
      // The vertex coordinates are chunk-local and are expressed
      // as signed 9.7's.
      int16_t x, y, z;
      uint8_t u;
      uint32_t v;
    };
    class ChunkMeshBuffer {
    public:
      ChunkMeshBuffer(
          const x801::map::ChunkXYZ& xyz,
          TerrainRenderer* tr);
      void createMesh();
    private:
      void addBlock(size_t lx, size_t ly, size_t lz);
      x801::map::Chunk* chunk;
      x801::map::ChunkXYZ xyz;
      TerrainRenderer* tr;
      std::vector<CMVertex> vertices;
      friend class TerrainRenderer;
    };
    class TerrainRenderer {
    public:
      TerrainRenderer(ClientWindow* cw);
      ClientWindow* cw;
      Client* c;
      Patcher* p;
      TextureView* tv;
      ClientGameState* gs;
      std::shared_ptr<agl::Texture> tex;
      x801::map::ModelApplicationIndex* mai;
      x801::map::ModelFunctionIndex* mfi;
      agl::FBO fbo;
    private:
      std::unordered_map<
          x801::map::ChunkXYZ,
          ChunkMeshBuffer,
          x801::map::ChunkHasher> cmbs;
      x801::map::Chunk* getChunk(const x801::map::ChunkXYZ& pos);
      friend class ChunkMeshBuffer;
    };
  }
}