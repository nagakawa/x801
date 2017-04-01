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

#include <FBO.h>

#include <Area.h>
#include <Chunk.h>

#include "Client.h"
#include "GameState.h"
#include "window/ClientWindow.h"
#include "window/Patcher.h"
#include "window/patcher_views/TextureView.h"

namespace x801 {
  namespace game {
    class TerrainRenderer;
    class ChunkMeshBuffer {
    public:
      ChunkMeshBuffer(
          const x801::map::ChunkXYZ& xyz,
          TerrainRenderer* tr) :
        xyz(xyz), tr(tr) {}
    private:
      x801::map::ChunkXYZ xyz;
      TerrainRenderer* tr;
    };
    class TerrainRenderer {
    public:
      TerrainRenderer(ClientWindow* cw);
      ClientWindow* cw;
      Client* c;
      Patcher* p;
      TextureView* tv;
      ClientGameState* gs;
      agl::FBO fbo;
    private:
      std::unordered_map<
          x801::map::ChunkXYZ,
          ChunkMeshBuffer,
          x801::map::ChunkHasher> cmbs;
      x801::map::Chunk* getChunk(const x801::map::ChunkXYZ& pos);
    };
  }
}