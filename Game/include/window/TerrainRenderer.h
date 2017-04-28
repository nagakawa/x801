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

#define GLEW_STATIC
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <EBO.h>
#include <FBO.h>
#include <Shader.h>
#include <ShaderProgram.h>
#include <Texture.h>
#include <VAO.h>
#include <VBO.h>

#include <Area.h>
#include <Chunk.h>
#include <Model.h>

namespace x801 {
  namespace game {
    class ClientWindow;
    class TerrainRenderer;
  }
}
#include "Client.h"
#include "GameState.h"
#include "window/Axes.h"
#include "window/ClientWindow.h"
#include "window/Patcher.h"
#include "window/patcher_views/ModelView.h"
#include "window/patcher_views/TextureView.h"

namespace x801 {
  namespace game {
    struct CMVertex {
      // The vertex coordinates are chunk-local and are expressed
      // as signed 9.7's.
      uint32_t w;
      int16_t x, y, z;
      uint8_t u, v;
    };
    static_assert(offsetof(CMVertex, x) + 2 == offsetof(CMVertex, y) && offsetof(CMVertex, y) + 2 == offsetof(CMVertex, z) && offsetof(CMVertex, u) + 1 == offsetof(CMVertex, v), "Basic offset checks");
    class ChunkMeshBuffer {
    public:
      ChunkMeshBuffer(
          const x801::map::ChunkXYZ& xyz,
          TerrainRenderer* tr,
          x801::map::Chunk* chunk) :
          xyz(xyz), tr(tr), chunk(chunk) {}
      void createMesh();
      void setUpRender(bool layer);
      void render(bool layer);
    private:
      void addBlock(size_t lx, size_t ly, size_t lz);
      x801::map::ChunkXYZ xyz;
      TerrainRenderer* tr;
      x801::map::Chunk* chunk;
      std::vector<CMVertex> opaqueVertices;
      std::vector<CMVertex> transparentVertices;
      agl::VBO vbo[2];
      agl::VAO vao[2];
      agl::ShaderProgram program[2];
#ifndef NDEBUG
      bool setup[2] = {false, false};
#endif
      friend class TerrainRenderer;
    };
    class TerrainRenderer {
    public:
      TerrainRenderer(ClientWindow* cw);
      void draw();
      ChunkMeshBuffer* summon(const x801::map::ChunkXYZ& pos);
      ClientWindow* cw;
      Client* c;
      Patcher* p;
      TextureView* tv;
      ClientGameState* gs;
      std::shared_ptr<agl::Texture> tex;
      x801::map::ModelApplicationIndex* mai;
      x801::map::ModelFunctionIndex* mfi;
      std::shared_ptr<agl::FBO> fboMS;
      std::shared_ptr<agl::FBO> fboSS;
      std::shared_ptr<agl::Texture> fboTex;
      std::shared_ptr<agl::Texture> fboTexMS;
#ifndef NDEBUG
      Axes axes;
#endif
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